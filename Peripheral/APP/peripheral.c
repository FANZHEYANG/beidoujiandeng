/********************************** (C) COPYRIGHT *******************************
 * File Name          : peripheral.C
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        : 外设从机多连接应用程序，初始化广播连接参数，然后广播，连接主机后，
 *                      请求更新连接参数，通过自定义服务传输数据
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "devinfoservice.h"
#include "gattprofile.h"
#include "peripheral.h"
#include "HAL.h"
#include "decode.h"
#if SOS_TEST_ONLY_PRINT
#undef PRINT
#define PRINT(...)
#define printf(...)
#endif

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD              1600

// How often to perform read rssi event
#define SBP_READ_RSSI_EVT_PERIOD             3200

// Parameter update delay
#define SBP_PARAM_UPDATE_DELAY               6400

// PHY update delay
#define SBP_PHY_UPDATE_DELAY                 2400

#define BLE_RECONNECT_SUPPRESS_PERIOD       MS1_TO_SYSTEM_TIME(2000)
#define BLE_AUTO_POWEROFF_PERIOD           MS1_TO_SYSTEM_TIME(1 * 60 * 1000)//待机时间

// What is the advertising interval when device is discoverable (units of 625us, 80=50ms)
#define DEFAULT_ADVERTISING_INTERVAL         80

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE            GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 6=7.5ms)
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL    6

// Maximum connection interval (units of 1.25ms, 100=125ms)
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL    100

// Slave latency to use parameter update
#define DEFAULT_DESIRED_SLAVE_LATENCY        0

// Supervision timeout value (units of 10ms, 100=1s)
#define DEFAULT_DESIRED_CONN_TIMEOUT         100

// Company Identifier: WCH
#define WCH_COMPANY_ID                       0x07D7

static uint8_t ble_soft_off = FALSE;
static uint8_t ble_disconnect_voice_pending = FALSE;



/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8_t Peripheral_TaskID = INVALID_TASK_ID; // Task ID for internal task/event processing

// GAP - SCAN RSP data (max size = 31 bytes)
//详细设置了广播数据格式还有广播扫描响应的数据
static uint8_t scanRspData[] = {
    // complete name完整设备名称（最多248字节，但受限于31字节）
    0x12, // length of this data数据段长度（20字节 = 1字节类型 + 19字节名称）
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,// 类型：完整本地名称
    'X',
    'T',
    '9',
    '0',
    '_',
    '0',
    '0',
    '0',
    '0',
    '0',
    '0',
    '0',
    '0',
    '0',
    '0',
    '0',
    '0',
    // connection interval range连接间隔范围（告诉中央设备自己支持的连接参数）
    0x05, // length of this data数据段长度（5字节 = 1字节类型 + 4字节数据）
    GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,// 类型：从机扫描连接间隔范围
    LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL), // 100ms，最小间隔低字节 (0x64)
    HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL), //最小间隔高字节 (0x00)
    LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL), // 1s,最大间隔低字节 (0xE8)
    HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL), //最大间隔高字节 (0x03)

    // Tx power level 发射功率（用于估算距离）
    0x02, // length of this data,数据段长度（2字节 = 1字节类型 + 1字节功率）
    GAP_ADTYPE_POWER_LEVEL,//类型：发射功率#define GAP_ADTYPE_POWER_LEVEL  0x0A
    //!< TX Power Level: -127 to +127 dBm
    0 // 功率值0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertising)
//广播设置
static uint8_t advertData[] = {
    // Flags; this sets the device to use limited discoverable
    // mode (advertises for 30 seconds at a time) instead of general
    // discoverable mode (advertises indefinitely)
    // 设备标志（有限可发现模式 + 不支持经典蓝牙）
    0x02, // length of this data数据段长度（2字节 = 1字节类型 + 1字节标志）
    GAP_ADTYPE_FLAGS,// 类型：设备标志
    DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,//无限期广播，仅支持 BLE

    // service UUID, to notify central devices what services are included
    // in this peripheral
    //服务UUID（告知中央设备自己支持的服务）
    0x03,                  // length of this data,数据段长度（3字节 = 1字节类型 + 2字节UUID）
    GAP_ADTYPE_16BIT_MORE, // some of the UUID's, but not all,16位UUID（部分）
    LO_UINT16(SIMPLEPROFILE_SERV_UUID),// UUID低字节
    HI_UINT16(SIMPLEPROFILE_SERV_UUID), // UUID高字节

    LO_UINT16(CONTROLPROFILE_SERV_UUID),// UUID低字节
    HI_UINT16(CONTROLPROFILE_SERV_UUID), // UUID高字节

    LO_UINT16(CARDPROFILE_SERV_UUID),// UUID低字节
    HI_UINT16(CARDPROFILE_SERV_UUID), // UUID高字节

    LO_UINT16(RDSSPROFILE_SERV_UUID),// UUID低字节
    HI_UINT16(RDSSPROFILE_SERV_UUID) // UUID高字节

};

// GAP GATT Attributes
static uint8_t attDeviceName[GAP_DEVICE_NAME_LEN] = "arm blinker";

// Connection item list
static peripheralConnItem_t peripheralConnList;

static uint16_t peripheralMTU = 247;
/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void Peripheral_ProcessTMOSMsg(tmos_event_hdr_t *pMsg);
static void peripheralStateNotificationCB(gapRole_States_t newState, gapRoleEvent_t *pEvent);
static void peripheralScheduleDisconnectVoice(void);
static void peripheralHandleConnectedVoice(void);
static void peripheralScheduleAutoPowerOff(void);
static void peripheralCancelAutoPowerOff(void);
static void peripheralSetSatelliteModules(BOOL enabled);
static void performPeriodicTask(void);
static void simpleProfileChangeCB(uint8_t paramID, uint8_t *pValue, uint16_t len);
static void peripheralParamUpdateCB(uint16_t connHandle, uint16_t connInterval,
                                    uint16_t connSlaveLatency, uint16_t connTimeout);
static void peripheralInitConnItem(peripheralConnItem_t *peripheralConnList);
static void peripheralRssiCB(uint16_t connHandle, int8_t rssi);
static void peripheralChar4Notify(uint8_t *pValue, uint16_t len);

static void batteryProfileChangeCB(uint8_t paramID, uint8_t *pValue, uint16_t len);
static void peripheralbatCharNotify(uint8_t *pValue, uint16_t len);

//gnss上传
static void peripheralGnssChar1Notify(uint8_t *pValue, uint16_t len);
static void peripheralGnssChar2Notify(uint8_t *pValue, uint16_t len);

static void peripheralRdssCardNotify(uint8_t *pValue, uint16_t len);
static void peripheralRdssSnrNotify(uint8_t *pValue, uint16_t len);
static uint8_t peripheralRdssTxAckNotify(uint8_t *pValue, uint16_t len);
static void peripheralRdssMsgRxNotify(uint8_t *pValue, uint16_t len);
static void peripheralRdssFreqNotify(uint8_t *pValue, uint16_t len);
static void sendRdssNotifications(void);

static void controlProfileChangeCB(uint8_t paramID, uint8_t *pValue, uint16_t len);
static void gnssProfileChangeCB(uint8_t paramID, uint8_t *pValue, uint16_t len);
static void rdssProfileChangeCB(uint8_t paramID, uint8_t *pValue, uint16_t len);
/*********************************************************************
 * PROFILE CALLBACKS回调函数
 */

// GAP Role Callbacks处理外设角色的核心事件
static gapRolesCBs_t Peripheral_PeripheralCBs = {
    peripheralStateNotificationCB, // Profile State Change Callbacks,状态变化回调,当外设状态（如广播、连接、断开）改变时触发
    peripheralRssiCB,              // When a valid RSSI is read from controller,RSSI读取回调 (not used by application)
    peripheralParamUpdateCB
};

// Broadcast Callbacks
static gapRolesBroadcasterCBs_t Broadcaster_BroadcasterCBs = {
    NULL, // Not used in peripheral role
    NULL  // Receive scan request callback
};

// GAP Bond Manager Callbacks
static gapBondCBs_t Peripheral_BondMgrCBs = {
    NULL, // Passcode callback (not used by application)
    NULL,  // Pairing / Bonding state Callback (not used by application)
    NULL  // oob callback
};

// Simple GATT Profile Callbacks，Simple GATT 服务回调,用途：处理自定义服务（如温度、开关）的读写事件
//paramID：标识被修改的特征值（Characteristic）的 ID例如：若服务中定义了温度（ID=1）、湿度（ID=2）等特征值，
//当客户端读写某个特征值时，paramID会对应其唯一 ID，用于区分不同特征值的变化
static simpleProfileCBs_t Peripheral_SimpleProfileCBs = {
    simpleProfileChangeCB // Characteristic value change callback，当客户端读取或写入服务中的特征值时触发
};
static batteryProfileCBs_t Peripheral_BatteryProfileCBs = {
    batteryProfileChangeCB // Characteristic value change callback，当客户端读取或写入服务中的特征值时触发
};
static controlProfileCBs_t Peripheral_ControlProfileCBs = {
    controlProfileChangeCB // Characteristic value change callback，当客户端读取或写入服务中的特征值时触发
};
static gnssProfileCBs_t Peripheral_GnssProfileCBs = {
    gnssProfileChangeCB // Characteristic value change callback，当客户端读取或写入服务中的特征值时触发
};
static rdssProfileCBs_t Peripheral_RdssProfileCBs = {
    rdssProfileChangeCB // Characteristic value change callback，当客户端读取或写入服务中的特征值时触发
};
/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      Peripheral_Init
 *
 * @brief   Initialization function for the Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by TMOS.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void Peripheral_Init()//外设从机的初始化入口
{
    Peripheral_TaskID = TMOS_ProcessEventRegister(Peripheral_ProcessEvent);//注册任务处理函数，初始化蓝牙协议栈

    // Setup the GAP Peripheral Role Profile
    //配置GAP层参数（广播数据、连接参数）
    {
        uint8_t  initial_advertising_enable = TRUE;
        uint16_t desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
        uint16_t desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;

        const char* hexDigits = "0123456789abcdef";
        uint8_t MacAddr[6];
        GetMACAddress(MacAddr);
         for(uint8_t i = 6; i > 0; i--)
        {
            scanRspData[19-2*i]   = hexDigits[((MacAddr[i-1] & 0xF0) >> 4)];
            scanRspData[19-2*i+1] = hexDigits[(MacAddr[i-1] & 0x0F)];
        }

        // Set the GAP Role Parameters
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);
        //设置广播数据和扫描响应数据
        GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(scanRspData), scanRspData);
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);
         //设置广播间隔（160个单位=100ms）控制设备广播的频率，影响功耗和发现速度
        GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL, sizeof(uint16_t), &desired_min_interval);
        GAPRole_SetParameter(GAPROLE_MAX_CONN_INTERVAL, sizeof(uint16_t), &desired_max_interval);
    }
    //设备发送广播包的频率
    //广播间隔，将最大 / 最小广播间隔都设为 160，本质是让设备以 固定的 100ms 间隔持续广播，适用于需要快速发现、调试简单或单设备的场景
    
    {
        uint16_t advInt = DEFAULT_ADVERTISING_INTERVAL;

        // Set advertising interval
        GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, advInt);
        GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, advInt);

        // Enable scan req notify
        GAP_SetParamValue(TGAP_ADV_SCAN_REQ_NOTIFY, ENABLE);
    }

    // Setup the GAP Bond Manager
    //配对与绑定配置，配置设备配对方式（等待请求）、密码、安全等级和 I/O 能力
    {
        uint32_t passkey = 0; // passkey "000000"，配对密码,配置蓝牙配对和绑定参数，控制设备如何与其他蓝牙设备建立安全连接
        // 配对模式：等待来自中央设备(如手机)的配对请求，不主动发起配对
        // 避免意外配对，需用户明确操作(如按下设备按钮)后才响应配对
        uint8_t  pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
        uint8_t  mitm = TRUE;//启用中间人保护（防止中间人攻击，增强配对安全性）
        uint8_t  bonding = TRUE;//启用绑定,允许设备存储配对密钥，实现未来无需重新配对的快速连接
        uint8_t  ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;//仅显示能力,表示设备可显示配对码但无法输入，配对时依赖外部设备(如手机)输入确认
        // 向蓝牙协议栈注册上述配对参数
        GAPBondMgr_SetParameter(GAPBOND_PERI_DEFAULT_PASSCODE, sizeof(uint32_t), &passkey);
        GAPBondMgr_SetParameter(GAPBOND_PERI_PAIRING_MODE, sizeof(uint8_t), &pairMode);
        GAPBondMgr_SetParameter(GAPBOND_PERI_MITM_PROTECTION, sizeof(uint8_t), &mitm);
        GAPBondMgr_SetParameter(GAPBOND_PERI_IO_CAPABILITIES, sizeof(uint8_t), &ioCap);
        GAPBondMgr_SetParameter(GAPBOND_PERI_BONDING_ENABLED, sizeof(uint8_t), &bonding);
    }

    // Initialize GATT attributes
    //GATT服务初始化，注册多个 GATT 服务，包括：GAP：通用访问配置文件，定义设备角色和发现方式
    GGS_AddService(GATT_ALL_SERVICES);           // GAP，添加GAP服务
    GATTServApp_AddService(GATT_ALL_SERVICES);   // GATT attributes，添加GATT服务，GATT通用属性配置文件，管理服务和特征值
    DevInfo_AddService();                        // Device Information Service，添加设备信息服务，包含制造商、型号等
    SimpleProfile_AddService(GATT_ALL_SERVICES); // Simple GATT Profile，添加自定义服务，用于应用数据传输
    Battery_Addservice(GATT_ALL_SERVICES);       // 电池电量服务
    Control_Addservice(GATT_ALL_SERVICES);       // 控制命令服务
    CardProfile_Addservice(GATT_ALL_SERVICES);   // SOS目标卡参数服务

    Gnss_Addservice(GATT_ALL_SERVICES);       // GNSS服务
    Rdss_Addservice(GATT_ALL_SERVICES);       // RDSS服务
    //OTAProfile_AddService(GATT_ALL_SERVICES);    // 添加OTA服务，固件升级服务，实现空中下载功能


    // Set the GAP Characteristics
    GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName);

    // Setup the SimpleProfile Characteristic Values
    //需要实现IOT控制的函数切口，通过蓝牙发送数据来控制相应的操作
    //设置特征值初始值，为 SimpleProfile 服务的 5 个特征值设置初始值，这些值可被主机读取或修改
    {
        //特征值初始化
        uint8_t charValue1[SIMPLEPROFILE_CHAR1_LEN] = {1};
        uint8_t charValue2[SIMPLEPROFILE_CHAR2_LEN] = {2};
        uint8_t charValue3[SIMPLEPROFILE_CHAR3_LEN] = {3};
        uint8_t charValue4[SIMPLEPROFILE_CHAR4_LEN] = {4};
        uint8_t charValue5[SIMPLEPROFILE_CHAR5_LEN] = {1, 2, 3, 4, 5};

        uint8_t batteryValue[BATTERYPROFILE_CHAR_LEN] = {0};

        //特征值初始化
        uint8_t controlValue1[CONTROLPROFILE_CHAR1_LEN] = {0};
        uint8_t controlValue2[CONTROLPROFILE_CHAR2_LEN] = {0};
        uint8_t controlValue3[CONTROLPROFILE_CHAR3_LEN] = {0};
        uint8_t controlValue4[CONTROLPROFILE_CHAR4_LEN] = {0};
        uint8_t controlValue5[CONTROLPROFILE_CHAR5_LEN] = {0};

        //特征值设置
        SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR1, SIMPLEPROFILE_CHAR1_LEN, charValue1);
        SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR2, SIMPLEPROFILE_CHAR2_LEN, charValue2);
        SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR3, SIMPLEPROFILE_CHAR3_LEN, charValue3);
        SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR4, SIMPLEPROFILE_CHAR4_LEN, charValue4);
        SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR5, SIMPLEPROFILE_CHAR5_LEN, charValue5);

        BatteryProfile_SetParameter(BATTERYPROFILE_CHAR, BATTERYPROFILE_CHAR_LEN, batteryValue);

        ControlProfile_SetParameter(CONTROLPROFILE_CHAR1, CONTROLPROFILE_CHAR1_LEN, controlValue1);
        ControlProfile_SetParameter(CONTROLPROFILE_CHAR2, CONTROLPROFILE_CHAR2_LEN, controlValue2);
        ControlProfile_SetParameter(CONTROLPROFILE_CHAR3, CONTROLPROFILE_CHAR3_LEN, controlValue3);
        ControlProfile_SetParameter(CONTROLPROFILE_CHAR4, CONTROLPROFILE_CHAR4_LEN, controlValue4);
        ControlProfile_SetParameter(CONTROLPROFILE_CHAR5, CONTROLPROFILE_CHAR5_LEN, controlValue5);
    }

    // Init Connection Item
    peripheralInitConnItem(&peripheralConnList);

    // Register callback with SimpleGATTprofile
    //注册回调函数
    SimpleProfile_RegisterAppCBs(&Peripheral_SimpleProfileCBs);
    BatteryProfile_RegisterAppCBs(&Peripheral_BatteryProfileCBs);
    ControlProfile_RegisterAppCBs(&Peripheral_ControlProfileCBs);
    GnssProfile_RegisterAppCBs(&Peripheral_GnssProfileCBs);
    RdssProfile_RegisterAppCBs(&Peripheral_RdssProfileCBs);
    //OTAProfile_RegisterAppCBs(&Peripheral_OTA_IAPProfileCBs);//处理 OTA 固件升级的数据传输和操作
    //在peripheral.c 注册回调函数

    // Register receive scan request callback
    GAPRole_BroadcasterSetCB(&Broadcaster_BroadcasterCBs);

    //启动蓝牙广播，设置延迟事件 SBP_START_DEVICE_EVT，触发 Peripheral_ProcessEvent() 中的广播启动逻辑
    tmos_set_event(Peripheral_TaskID, SBP_START_DEVICE_EVT);
}

/*********************************************************************
 * @fn      peripheralInitConnItem
 *
 * @brief   Init Connection Item
 *
 * @param   peripheralConnList -
 *
 * @return  NULL
 */
static void peripheralInitConnItem(peripheralConnItem_t *peripheralConnList)
{
    peripheralConnList->connHandle = GAP_CONNHANDLE_INIT;
    peripheralConnList->connInterval = 0;
    peripheralConnList->connSlaveLatency = 0;
    peripheralConnList->connTimeout = 0;
}

static void peripheralScheduleAutoPowerOff(void)
{
    if(ble_soft_off != FALSE)
    {
        return;
    }

    tmos_start_task(Peripheral_TaskID, SBP_BLE_AUTO_POWEROFF_EVT, BLE_AUTO_POWEROFF_PERIOD);
}

static void peripheralCancelAutoPowerOff(void)
{
    tmos_stop_task(Peripheral_TaskID, SBP_BLE_AUTO_POWEROFF_EVT);
}

static void peripheralScheduleDisconnectVoice(void)
{
    if(ble_soft_off != FALSE)
    {
        return;
    }

    ble_disconnect_voice_pending = TRUE;
    peripheralScheduleAutoPowerOff();
    tmos_start_task(Peripheral_TaskID, SBP_BLE_DISCONNECT_VOICE_EVT, BLE_RECONNECT_SUPPRESS_PERIOD);
}

static void peripheralHandleConnectedVoice(void)
{
    if(ble_soft_off != FALSE)
    {
        return;
    }

    peripheralCancelAutoPowerOff();

    if(ble_disconnect_voice_pending != FALSE)
    {
        ble_disconnect_voice_pending = FALSE;
        tmos_stop_task(Peripheral_TaskID, SBP_BLE_DISCONNECT_VOICE_EVT);
        return;
    }

    Pwr_RequestVoiceText("蓝牙已连接");
}
/*********************************************************************
 * @fn      Peripheral_ProcessEvent
 *
 * @brief   Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
  * 功能：任务事件处理器，处理系统消息、定时器事件和自定义事件。
 * 事件处理逻辑：
 * SYS_EVENT_MSG：处理 TMOS 消息队列中的消息（如连接状态变化）。
 * SBP_START_DEVICE_EVT：启动 BLE 外设设备并开始广播，设置周期性任务定时器。
 * SBP_PERIODIC_EVT：周期性任务（默认 1 秒），调用 performPeriodicTask 更新特征值。
 * OTA_FLASH_ERASE_EVT：分块擦除 Flash 存储器，用于 OTA 升级。每次擦除后检查是否完成，未完成则继续触发事件
 *
 */
uint16_t Peripheral_ProcessEvent(uint8_t task_id, uint16_t events)
{
    //  VOID task_id; // TMOS required parameter that isn't used in this function

    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        if((pMsg = tmos_msg_receive(Peripheral_TaskID)) != NULL)
        {
            Peripheral_ProcessTMOSMsg((tmos_event_hdr_t *)pMsg);
            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if(events & SBP_START_DEVICE_EVT)
    {
        // Start the Device
        GAPRole_PeripheralStartDevice(Peripheral_TaskID, &Peripheral_BondMgrCBs, &Peripheral_PeripheralCBs);
        return (events ^ SBP_START_DEVICE_EVT);
    }

    if(events & SBP_PERIODIC_EVT)
    {
        // Restart timer
        if(SBP_PERIODIC_EVT_PERIOD)
        {
            tmos_start_task(Peripheral_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD);
        }
        // Perform periodic application task
        performPeriodicTask();
        return (events ^ SBP_PERIODIC_EVT);
    }

    if(events & SBP_PARAM_UPDATE_EVT)
    {
        // Send connect param update request
        // When the current connection parameters already meet the requirements for update, return 0x18(InvalidRange)
        GAPRole_PeripheralConnParamUpdateReq(peripheralConnList.connHandle,
                                             DEFAULT_DESIRED_MIN_CONN_INTERVAL,
                                             DEFAULT_DESIRED_MAX_CONN_INTERVAL,
                                             DEFAULT_DESIRED_SLAVE_LATENCY,
                                             DEFAULT_DESIRED_CONN_TIMEOUT,
                                             Peripheral_TaskID);

        return (events ^ SBP_PARAM_UPDATE_EVT);
    }

    if(events & SBP_PHY_UPDATE_EVT)
    {
        // start phy update
        PRINT("PHY Update %x...\n", GAPRole_UpdatePHY(peripheralConnList.connHandle, 0, 
                    GAP_PHY_BIT_LE_2M, GAP_PHY_BIT_LE_2M, 0));

        return (events ^ SBP_PHY_UPDATE_EVT);
    }

    if(events & SBP_READ_RSSI_EVT)
    {
        GAPRole_ReadRssiCmd(peripheralConnList.connHandle);
        tmos_start_task(Peripheral_TaskID, SBP_READ_RSSI_EVT, SBP_READ_RSSI_EVT_PERIOD);
        return (events ^ SBP_READ_RSSI_EVT);
    }

    if(events & SBP_BLE_AUTO_POWEROFF_EVT)
    {
        if((ble_soft_off == FALSE) && (peripheralConnList.connHandle == GAP_CONNHANDLE_INIT))
        {
            Pwr_RequestAutoPowerOff();
        }

        return (events ^ SBP_BLE_AUTO_POWEROFF_EVT);
    }
    if(events & SBP_BLE_DISCONNECT_VOICE_EVT)
    {
        if(ble_disconnect_voice_pending != FALSE)
        {
            ble_disconnect_voice_pending = FALSE;
            if((ble_soft_off == FALSE) && (peripheralConnList.connHandle == GAP_CONNHANDLE_INIT))
            {
                Pwr_RequestVoiceText("蓝牙断开");
            }
        }

        return (events ^ SBP_BLE_DISCONNECT_VOICE_EVT);
    }
    // Discard unknown events
    return 0;
}



/*********************************************************************
 * @fn      Peripheral_ProcessGAPMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void Peripheral_ProcessGAPMsg(gapRoleEvent_t *pEvent)
{
    switch(pEvent->gap.opcode)
    {
        case GAP_SCAN_REQUEST_EVENT:
        {
//            PRINT("Receive scan req from %x %x %x %x %x %x  ..\n", pEvent->scanReqEvt.scannerAddr[0],
//                  pEvent->scanReqEvt.scannerAddr[1], pEvent->scanReqEvt.scannerAddr[2], pEvent->scanReqEvt.scannerAddr[3],
//                  pEvent->scanReqEvt.scannerAddr[4], pEvent->scanReqEvt.scannerAddr[5]);
            break;
        }

        case GAP_PHY_UPDATE_EVENT:
        {
            //PRINT("Phy update Rx:%x Tx:%x ..\n", pEvent->linkPhyUpdate.connRxPHYS, pEvent->linkPhyUpdate.connTxPHYS);
            break;
        }

        default:
            break;
    }
}

/*********************************************************************
 * @fn      Peripheral_ProcessTMOSMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
  * 功能：处理 TMOS 操作系统消息队列中的消息。
 * 用途：在 BLE 应用里，所有异步事件（像连接状态改变、特征值更新等）都会通过消息队列传递，此函数负责解析并处理这些消息。
 * 参数：
 * pMsg：指向 tmos_event_hdr_t 结构体的指针，表示消息头部。
 * 该结构体通常包含：
 * uint8_t event：事件类型（如定时器到期、GATT 事件）。
 * uint8_t status：事件状态（如成功、失败）。
 * 可能包含额外数据字段（取决于具体事件类型）
 */
static void Peripheral_ProcessTMOSMsg(tmos_event_hdr_t *pMsg)
{
    switch(pMsg->event)
    {
        case GAP_MSG_EVENT:
        {
            Peripheral_ProcessGAPMsg((gapRoleEvent_t *)pMsg);
            break;
        }

        case GATT_MSG_EVENT:
        {
            gattMsgEvent_t *pMsgEvent;

            pMsgEvent = (gattMsgEvent_t *)pMsg;
            if(pMsgEvent->method == ATT_MTU_UPDATED_EVENT)
            {
                peripheralMTU = pMsgEvent->msg.exchangeMTUReq.clientRxMTU;
                PRINT("mtu exchange: %d\n", pMsgEvent->msg.exchangeMTUReq.clientRxMTU);
            }
            break;
        }

        default:
            break;
    }
}

static void peripheralSetSatelliteModules(BOOL enabled)
{
    RN_SW_Flag = enabled;
    RD_SW_Flag = enabled;

    if(enabled == TRUE)
    {
        OPENRN();
        OPENRD();
    }
    else
    {
        CLOSERN();
        CLOSERD();
    }
}

/*********************************************************************
 * @fn      Peripheral_LinkEstablished
 * 当连接建立时，中断会触发Peripheral_LinkEstablished函数
 * @brief   Process link established.
 *
 * @param   pEvent - event to process
 *
 * @return  none
 */
static void Peripheral_LinkEstablished(gapRoleEvent_t *pEvent)
{
    gapEstLinkReqEvent_t *event = (gapEstLinkReqEvent_t *)pEvent;
        if(ble_soft_off)
    {
        GAPRole_TerminateLink(event->connectionHandle);
        return;
    }

    // See if already connected
    if(peripheralConnList.connHandle != GAP_CONNHANDLE_INIT)
    {
        GAPRole_TerminateLink(pEvent->linkCmpl.connectionHandle);
        PRINT("Connection max...\n");
    }
    else
    {
        peripheralConnList.connHandle = event->connectionHandle;
        peripheralConnList.connInterval = event->connInterval;
        peripheralConnList.connSlaveLatency = event->connLatency;
        peripheralConnList.connTimeout = event->connTimeout;
        peripheralSetSatelliteModules(TRUE);
        peripheralMTU = 247;
        // Set timer for periodic event
        tmos_start_task(Peripheral_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD);

        // Set timer for param update event
        tmos_start_task(Peripheral_TaskID, SBP_PARAM_UPDATE_EVT, SBP_PARAM_UPDATE_DELAY);

        // Start read rssi
        tmos_start_task(Peripheral_TaskID, SBP_READ_RSSI_EVT, SBP_READ_RSSI_EVT_PERIOD);

        PRINT("Conn %x - Int %x \n", event->connectionHandle, event->connInterval);
    }
}

/*********************************************************************
 * @fn      Peripheral_LinkTerminated
 *
 * @brief   Process link terminated.
 *
 * @param   pEvent - event to process
 *
 * @return  none
 */
static void Peripheral_LinkTerminated(gapRoleEvent_t *pEvent)
{
    gapTerminateLinkEvent_t *event = (gapTerminateLinkEvent_t *)pEvent;

    if(event->connectionHandle == peripheralConnList.connHandle)
    {
        peripheralConnList.connHandle = GAP_CONNHANDLE_INIT;
        peripheralConnList.connInterval = 0;
        peripheralConnList.connSlaveLatency = 0;
        peripheralConnList.connTimeout = 0;
        tmos_stop_task(Peripheral_TaskID, SBP_PERIODIC_EVT);
        tmos_stop_task(Peripheral_TaskID, SBP_READ_RSSI_EVT);
        peripheralSetSatelliteModules(FALSE);

        // Restart advertising
        
            if(ble_soft_off == FALSE)
{
    uint8_t advertising_enable = TRUE;
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &advertising_enable);
}
        
    }
    else
    {
        PRINT("ERR..\n");
    }
}

/*********************************************************************
 * @fn      peripheralRssiCB
 *
 * @brief   RSSI callback.
 *
 * @param   connHandle - connection handle
 * @param   rssi - RSSI
 *
 * @return  none
 */
static void peripheralRssiCB(uint16_t connHandle, int8_t rssi)
{
    PRINT("RSSI -%d dB Conn  %x \n", -rssi, connHandle);
}

/*********************************************************************
 * @fn      peripheralParamUpdateCB
 *
 * @brief   Parameter update complete callback处理连接参数更新的响应
 *
 * @param   connHandle - connect handle
 *          connInterval - connect interval
 *          connSlaveLatency - connect slave latency
 *          connTimeout - connect timeout
 *
 * @return  none
 */
static void peripheralParamUpdateCB(uint16_t connHandle, uint16_t connInterval,
                                    uint16_t connSlaveLatency, uint16_t connTimeout)
{
    if(connHandle == peripheralConnList.connHandle)
    {
        peripheralConnList.connInterval = connInterval;
        peripheralConnList.connSlaveLatency = connSlaveLatency;
        peripheralConnList.connTimeout = connTimeout;

        PRINT("Update %x - Int %x \n", connHandle, connInterval);
    }
    else
    {
        PRINT("ERR..\n");
    }
}

/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.状态（如启动、广播、连接等）改变会触发此函数
 *
 * @param   newState - new state
 *
 * @return  none
  * 功能：GAP外设角色状态变化的回调函数。
 * 用途：当外设的状态（如待机、广播、已连接等）发生改变时，会调用该函数，可用于更新 UI 或者记录日志。
 * 参数：
 * newState：gapRole_States_t 枚举类型，表示新的外设状态。
 * 常见值包括：
 * GAPROLE_STARTED：外设已启动。设备初始化完成
 * GAPROLE_ADVERTISING：正在广播。开始广播，等待主机连接
 * GAPROLE_CONNECTED：已连接。与主机建立连接，若连接间隔大于预设最大值（DEFAULT_DESIRED_MAX_CONN_INTERVAL），则请求更新连接参数
 * GAPROLE_DISCONNECTED：已断开连接。
 * GAPROLE_WAITING：连接断开，重新启用广播。
 * GAPROLE_ERROR：发生错误
 * pEvent：指向 gapRoleEvent_t 结构体的指针，包含状态变化的详细信息
 */
static void peripheralStateNotificationCB(gapRole_States_t newState, gapRoleEvent_t *pEvent)
{
    switch(newState & GAPROLE_STATE_ADV_MASK)
    {
        case GAPROLE_STARTED://设备已启动但未开始广播，通常用于初始化设备地址或系统ID
            PRINT("Initialized..\n");
            break;

        case GAPROLE_ADVERTISING://设备正在广播，可被中心设备发现
            if(pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
            {
                Peripheral_LinkTerminated(pEvent);
                peripheralScheduleDisconnectVoice();

                PRINT("Disconnected.. Reason:%x\n", pEvent->linkTerminate.reason);
                PRINT("Advertising..\n");
            }
            else if(pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
            {
                PRINT("Advertising..\n");
            }
            break;

        case GAPROLE_CONNECTED://设备已连接到中心设备，可进行数据传输。
            if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                Peripheral_LinkEstablished(pEvent);
                peripheralHandleConnectedVoice();

                PRINT("Connected..\n");
            }
            break;

        case GAPROLE_CONNECTED_ADV://表示设备当前处于已连接并同时进行广播的模式
            if(pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
            {
                PRINT("Connected Advertising..\n");
            }
            break;

        case GAPROLE_WAITING://表示设备已启动但当前?不处于广播状态?，正处于?等待期?，准备?重新开始广播
            if(pEvent->gap.opcode == GAP_END_DISCOVERABLE_DONE_EVENT)
            {
                PRINT("Waiting for advertising..\n");
            }
            else if(pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
            {
                Peripheral_LinkTerminated(pEvent);
                peripheralScheduleDisconnectVoice();

                PRINT("Disconnected.. Reason:%x\n", pEvent->linkTerminate.reason);
            }
            else if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                if(pEvent->gap.hdr.status != SUCCESS)
                {
                    PRINT("Waiting for advertising..\n");
                }
                else
                {
                    PRINT("Error..\n");
                }
            }
            else
            {
                PRINT("Error..%x\n", pEvent->gap.opcode);
            }
            break;

        case GAPROLE_ERROR://设备进入错误状态，需处理异常情况
            PRINT("Error..\n");
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      performPeriodicTask
 *
 * @brief   Perform a periodic application task. This function gets
 *          called every five seconds as a result of the SBP_PERIODIC_EVT
 *          TMOS event. In this example, the value of the third
 *          characteristic in the SimpleGATTProfile service is retrieved
 *          from the profile, and then copied into the value of the
 *          the fourth characteristic.
 *
 * @param   none
 *
 * @return  none
 * 功能：执行周期性任务。默认每秒执行一次。周期事件，5000ms
 * 用途：按照预设的时间间隔（由SBP_PERIODIC_EVT_PERIOD定义）执行特定操作，例如更新传感器数据、检查电池电量等。
 * 参数：无
 * 实现逻辑：读取 SIMPLEPROFILE_CHAR4 特征值。
 * 若读取成功，将该值写回 SIMPLEPROFILE_CHAR4（实际为自我复制，可能用于测试或保持连接活跃）
 */
static void performPeriodicTask(void)
{
    uint8_t notiData[SIMPLEPROFILE_CHAR4_LEN] = {0xab};
    uint8_t notiData_bat[BATTERYPROFILE_CHAR_LEN] = {0x00};

    peripheralChar4Notify(notiData, SIMPLEPROFILE_CHAR4_LEN);

    notiData_bat[0] = Bat_percentage;
    peripheralbatCharNotify(notiData_bat, BATTERYPROFILE_CHAR_LEN);
//   // GNSS测试数据1: 信噪比 (12个卫星的SNR值)
//       uint8_t gnss_snr_test[GNSSPROFILE_CHAR1_LEN] = {
//           45, 42, 38, 35, 33, 30, 28, 25, 22, 20, 18, 15
//       };
//       peripheralGnssChar1Notify(gnss_snr_test, GNSSPROFILE_CHAR1_LEN);
//
//       // GNSS测试数据2: 位置信息 (北京天安门: 39.9042°N, 116.4074°E)
//       uint8_t gnss_location_test[GNSSPROFILE_CHAR2_LEN] = {
//           12, 30, 45,                 // 时:分:秒
//           0x40, 0xE2, 0x01, 0x00,     // 微秒: 123456
//           0x52, 0xB8, 0x1F, 0x42,     // 纬度: 39.9042 (float)
//           0x3D, 0x0A, 0xE9, 0x42,     // 经度: 116.4074 (float)
//           0x01, 0x00, 0x00, 0x00,     // 定位质量: 1
//           0x08, 0x00, 0x00, 0x00,     // 卫星数: 8
//           0x9A, 0x99, 0x99, 0x3F,     // hdop: 1.2 (float)
//           0x00, 0x00, 0x4A, 0x42,     // 海拔: 50.5 (float)
//           0x33, 0x33, 0x51, 0x42,     // 高度: 52.3 (float)
//           0x00, 0x00, 0x00, 0x00      // dgps_age: 0.0 (float)
//       };
//       peripheralGnssChar2Notify(gnss_location_test, GNSSPROFILE_CHAR2_LEN);
//    // RDSS SNR 固定测试数据 (0x4503)
//    uint8_t rdssSnrData[RDSSPROFILE_CHAR3_LEN] = {
//    45, 42, 38, 35, 40, 43,   // 前 6 个
//    37, 41, 39, 36, 44, 46    // 后 6 个
//    };
//    peripheralRdssSnrNotify(rdssSnrData, RDSSPROFILE_CHAR3_LEN);
//-------------------------------------------------------------
     uint8_t gnssData1[GNSSPROFILE_CHAR1_LEN] = {0};
     uint8_t gnssData2[GNSSPROFILE_CHAR2_LEN] = {0};
     uint8_t temp_bytes[4] = {0};
     uint8_t i;


     for(i = 0; i < 12; i++)
     {
         gnssData1[i] = Save_GSV_Data.satellites[i].snr;
     }


     gnssData2[0] = GGA.hour;
     gnssData2[1] = GGA.minute;
     gnssData2[2] = GGA.second;
     gnssData2[3] = (uint8_t)(GGA.microseconds & 0x000000FF);
     gnssData2[4] = (uint8_t)((GGA.microseconds & 0x0000FF00) >> 8);
     gnssData2[5] = (uint8_t)((GGA.microseconds & 0x00FF0000) >> 16);
     gnssData2[6] = (uint8_t)((GGA.microseconds & 0xFF000000) >> 24);


     float_to_bytes_memcpy(GGA.latitude, temp_bytes);
     for(i = 0; i < 4; i++)
     {
         gnssData2[7 + i] = temp_bytes[i];
     }


     float_to_bytes_memcpy(GGA.longitude, temp_bytes);
     for(i = 0; i < 4; i++)
     {
         gnssData2[11 + i] = temp_bytes[i];
     }

     gnssData2[15] = (uint8_t)(GGA.fix_quality & 0x000000FF);
     gnssData2[16] = (uint8_t)((GGA.fix_quality & 0x0000FF00) >> 8);
     gnssData2[17] = (uint8_t)((GGA.fix_quality & 0x00FF0000) >> 16);
     gnssData2[18] = (uint8_t)((GGA.fix_quality & 0xFF000000) >> 24);


     gnssData2[19] = (uint8_t)(GGA.satellites_tracked & 0x000000FF);
     gnssData2[20] = (uint8_t)((GGA.satellites_tracked & 0x0000FF00) >> 8);
     gnssData2[21] = (uint8_t)((GGA.satellites_tracked & 0x00FF0000) >> 16);
     gnssData2[22] = (uint8_t)((GGA.satellites_tracked & 0xFF000000) >> 24);


     float_to_bytes_memcpy(GGA.hdop, temp_bytes);
     for(i = 0; i < 4; i++)
     {
         gnssData2[23 + i] = temp_bytes[i];
     }


     float_to_bytes_memcpy(GGA.altitude, temp_bytes);
     for(i = 0; i < 4; i++)
     {
         gnssData2[27 + i] = temp_bytes[i];
     }


     float_to_bytes_memcpy(GGA.height, temp_bytes);
     for(i = 0; i < 4; i++)
     {
         gnssData2[31 + i] = temp_bytes[i];
     }


     float_to_bytes_memcpy(GGA.dgps_age, temp_bytes);
     for(i = 0; i < 4; i++)
     {
         gnssData2[35 + i] = temp_bytes[i];
     }


     peripheralGnssChar1Notify(gnssData1, GNSSPROFILE_CHAR1_LEN);
     peripheralGnssChar2Notify(gnssData2, GNSSPROFILE_CHAR2_LEN);
     //RDSS全部notify
    sendRdssNotifications();

}

/*********************************************************************
 * @fn      peripheralChar4Notify
 *
 * @brief   Prepare and send simpleProfileChar4 notification
 *
 * @param   pValue - data to notify
 *          len - length of data
 *
 * @return  none
 */
static void peripheralChar4Notify(uint8_t *pValue, uint16_t len)
{
    attHandleValueNoti_t noti;
    if(len > (peripheralMTU - 3))
    {
        PRINT("Too large noti\n");
        return;
    }
    noti.len = len;
    noti.pValue = GATT_bm_alloc(peripheralConnList.connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL, 0);
    if(noti.pValue)
    {
        tmos_memcpy(noti.pValue, pValue, noti.len);
        if(simpleProfile_Notify(peripheralConnList.connHandle, &noti) != SUCCESS)
        {
            GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
        }
    }
}

//notify的数据发送接口
static void peripheralbatCharNotify(uint8_t *pValue, uint16_t len)
{
    attHandleValueNoti_t noti;
    if(len > (peripheralMTU - 3))
    {
        PRINT("Too large noti\n");
        return;
    }
    noti.len = len;
    noti.pValue = GATT_bm_alloc(peripheralConnList.connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL, 0);
    if(noti.pValue)
    {
        tmos_memcpy(noti.pValue, pValue, noti.len);
        if(batteryProfile_Notify(peripheralConnList.connHandle, &noti) != SUCCESS)
        {
            GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
        }
    }
}

//GNSS Char1 notify
static void peripheralGnssChar1Notify(uint8_t *pValue, uint16_t len)
{
    attHandleValueNoti_t noti;
    if(len > (peripheralMTU - 3))
    {
        PRINT("Too large noti\n");
        return;
    }
    noti.len = len;
    noti.pValue = GATT_bm_alloc(peripheralConnList.connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL, 0);
    if(noti.pValue)
    {
        tmos_memcpy(noti.pValue, pValue, noti.len);
        if(gnssProfile_Notify1(peripheralConnList.connHandle, &noti) != SUCCESS)
        {
            GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
        }
    }
}

//GNSS Char2 notify
static void peripheralGnssChar2Notify(uint8_t *pValue, uint16_t len)
{
    attHandleValueNoti_t noti;
    if(len > (peripheralMTU - 3))
    {
        PRINT("Too large noti\n");
        return;
    }
    noti.len = len;
    noti.pValue = GATT_bm_alloc(peripheralConnList.connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL, 0);
    if(noti.pValue)
    {
        tmos_memcpy(noti.pValue, pValue, noti.len);
        if(gnssProfile_Notify2(peripheralConnList.connHandle, &noti) != SUCCESS)
        {
            GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
        }
    }
}

/*********************************************************************
 * @fn      simpleProfileChangeCB
 *
 * @brief   Callback from SimpleBLEProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *          pValue - pointer to data that was changed
 *          len - length of data
 *
 * @return  none
  * 功能：SimpleProfile 服务参数变化的回调函数。
 * 用途：当 SimpleProfile 服务中的某个特征值（如温度、湿度）被远程设备读取或写入时，会触发该回调。
 * 参数：
 * paramID：uint8_t 类型，表示发生变化的特征值 ID。
 * 例如：
 * SIMPLEPROFILE_CHAR1：第一个特征值。
 * SIMPLEPROFILE_CHAR2：第二个特征值
 * 实现逻辑：当 SIMPLEPROFILE_CHAR1 或 SIMPLEPROFILE_CHAR3 变化时，打印日志并读取新值。
 * 其他特征值变化时，进入 default 分支（未处理）。
 */
static void simpleProfileChangeCB(uint8_t paramID, uint8_t *pValue, uint16_t len)
{
    switch(paramID)
    {
        case SIMPLEPROFILE_CHAR1:
        {
            uint8_t newValue[SIMPLEPROFILE_CHAR1_LEN];
            tmos_memcpy(newValue, pValue, len);
            PRINT("profile ChangeCB CHAR1.. \n");
            break;
        }

        case SIMPLEPROFILE_CHAR3:
        {
            uint8_t newValue[SIMPLEPROFILE_CHAR3_LEN];
            tmos_memcpy(newValue, pValue, len);
            PRINT("profile ChangeCB CHAR3..\n");
            break;
        }

        default:
            // should not reach here!
            break;
    }
}

//接收主机数据进行处理
static void batteryProfileChangeCB(uint8_t paramID,uint8_t *pValue,uint16_t len)
{
    switch(paramID)
    {
    case BATTERYPROFILE_CHAR:
        {
            uint8_t newValue[BATTERYPROFILE_CHAR_LEN];
            tmos_memcpy(newValue, pValue, len);//接收到的数据
            //if(newValue[0])

            break;
        }
    default:
        break;
    }
}

//接收主机数据进行处理
static void gnssProfileChangeCB(uint8_t paramID,uint8_t *pValue,uint16_t len)
{
    switch(paramID)
    {
    case GNSSPROFILE_CHAR1:
        {
            uint8_t newValue[GNSSPROFILE_CHAR1_LEN];
            tmos_memcpy(newValue, pValue, len);//接收到的数据
            //if(newValue[0])

            break;
        }
    case GNSSPROFILE_CHAR2:
        {
            uint8_t newValue[GNSSPROFILE_CHAR2_LEN];
            tmos_memcpy(newValue, pValue, len);//接收到的数据
            //if(newValue[0])

            break;
        }
    default:
        break;
    }
}

//接收主机数据进行处理
static void rdssProfileChangeCB(uint8_t paramID,uint8_t *pValue,uint16_t len)
{
    switch(paramID)
    {
    case RDSSPROFILE_CHAR1:
        {
            uint8_t newValue[RDSSPROFILE_CHAR1_LEN];
            tmos_memcpy(newValue, pValue, len);//接收到的数据
            //if(newValue[0])

            break;
        }
    case RDSSPROFILE_CHAR2:
        {
            uint8_t newValue[RDSSPROFILE_CHAR2_LEN];
            tmos_memcpy(newValue, pValue, len);//接收到的数据
            //if(newValue[0])

            break;
        }
    case RDSSPROFILE_CHAR3:
        {
            uint8_t newValue[RDSSPROFILE_CHAR3_LEN];
            tmos_memcpy(newValue, pValue, len);//接收到的数据
            //if(newValue[0])

            break;
        }
    case RDSSPROFILE_CHAR5:
        {
            uint8_t newValue[RDSSPROFILE_CHAR5_LEN];
            tmos_memcpy(newValue, pValue, len);//接收到的数据
            //if(newValue[0])

            break;
        }
    case RDSSPROFILE_CHAR6:
        {
            uint8_t newValue[RDSSPROFILE_CHAR6_LEN];
            tmos_memcpy(newValue, pValue, len);//接收到的数据
            //if(newValue[0])

            break;
        }
    case RDSSPROFILE_CHAR7:
        {
            uint8_t newValue[RDSSPROFILE_CHAR7_LEN];
            tmos_memcpy(newValue, pValue, len);//接收到的数据
            //if(newValue[0])

            break;
        }
    default:
        break;
    }
}


static void controlProfileChangeCB(uint8_t paramID, uint8_t *pValue, uint16_t len)
{
    switch(paramID)
    {
        case CONTROLPROFILE_CHAR1:
        {
            uint8_t newValue[CONTROLPROFILE_CHAR1_LEN];
            tmos_memcpy(newValue, pValue, len);//接收到的数据
            PRINT("profile ChangeCB CHAR1.. \n");
            break;
        }
        case CONTROLPROFILE_CHAR2:
        {
            uint8_t newValue[CONTROLPROFILE_CHAR2_LEN];
            tmos_memcpy(newValue, pValue, len);//接收到的数据
            PRINT("profile ChangeCB CHAR1.. \n");
            break;
        }
        case CONTROLPROFILE_CHAR3:
        {
            uint8_t newValue[CONTROLPROFILE_CHAR3_LEN];
            tmos_memcpy(newValue, pValue, len);//接收到的数据
            PRINT("profile ChangeCB CHAR1.. \n");
            break;
        }
        case CONTROLPROFILE_CHAR4:
        {
            uint8_t newValue[CONTROLPROFILE_CHAR4_LEN];
            tmos_memcpy(newValue, pValue, len);//接收到的数据
            PRINT("profile ChangeCB CHAR1.. \n");
            break;
        }
        case CONTROLPROFILE_CHAR5:
        {
            uint8_t newValue[CONTROLPROFILE_CHAR5_LEN];
            tmos_memcpy(newValue, pValue, len);//接收到的数据
            PRINT("profile ChangeCB CHAR1.. \n");
            break;
        }
        default:
            // should not reach here!
            break;
    }
}

//RDSS Card Info notify
static void peripheralRdssCardNotify(uint8_t *pValue, uint16_t len)
{
    attHandleValueNoti_t noti;
    if(len > (peripheralMTU - 3))
    {
        PRINT("Too large noti\n");
        return;
    }
    noti.len = len;
    noti.pValue = GATT_bm_alloc(peripheralConnList.connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL, 0);
    if(noti.pValue)
    {
        tmos_memcpy(noti.pValue, pValue, noti.len);
        if(rdssProfile_Notify2(peripheralConnList.connHandle, &noti) != SUCCESS)
        {
            GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
        }
    }
}

//RDSS SNR notify
static void peripheralRdssSnrNotify(uint8_t *pValue, uint16_t len)
{
    attHandleValueNoti_t noti;
    if(len > (peripheralMTU - 3))
    {
        PRINT("Too large noti\n");
        return;
    }
    noti.len = len;
    noti.pValue = GATT_bm_alloc(peripheralConnList.connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL, 0);
    if(noti.pValue)
    {
        tmos_memcpy(noti.pValue, pValue, noti.len);
        if(rdssProfile_Notify3(peripheralConnList.connHandle, &noti) != SUCCESS)
        {
            GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
        }
    }
}

//RDSS TX ACK notify
static uint8_t peripheralRdssTxAckNotify(uint8_t *pValue, uint16_t len)
{
    attHandleValueNoti_t noti;
    bStatus_t status;
    if(len > (peripheralMTU - 3))
    {
        PRINT("[4505] notify too large len=%d mtu=%d\r\n", len, peripheralMTU);
        return bleInvalidRange;
    }
    noti.len = len;
    noti.pValue = GATT_bm_alloc(peripheralConnList.connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL, 0);
    if(noti.pValue)
    {
        tmos_memcpy(noti.pValue, pValue, noti.len);
        status = rdssProfile_Notify5(peripheralConnList.connHandle, &noti);
        if(status != SUCCESS)
        {
            GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
            PRINT("[4505] notify fail status=%d ack=%d reason=%d\r\n", status, pValue[0], pValue[1]);
        }
        else
        {
            PRINT("[4505] notify ok ack=%d reason=%d\r\n", pValue[0], pValue[1]);
        }
        return status;
    }
    PRINT("[4505] notify alloc fail ack=%d reason=%d\r\n", pValue[0], pValue[1]);
    return bleMemAllocError;
}

//RDSS MSG RX notify
static void peripheralRdssMsgRxNotify(uint8_t *pValue, uint16_t len)
{
    attHandleValueNoti_t noti;
    if(len > (peripheralMTU - 3))
    {
        PRINT("Too large noti\n");
        return;
    }
    noti.len = len;
    noti.pValue = GATT_bm_alloc(peripheralConnList.connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL, 0);
    if(noti.pValue)
    {
        tmos_memcpy(noti.pValue, pValue, noti.len);
        if(rdssProfile_Notify6(peripheralConnList.connHandle, &noti) != SUCCESS)
        {
            GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
        }
    }
}

//RDSS Frequency Countdown notify
static void peripheralRdssFreqNotify(uint8_t *pValue, uint16_t len)
{
    attHandleValueNoti_t noti;
    if(len > (peripheralMTU - 3))
    {
        PRINT("Too large noti\n");
        return;
    }
    noti.len = len;
    noti.pValue = GATT_bm_alloc(peripheralConnList.connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL, 0);
    if(noti.pValue)
    {
        tmos_memcpy(noti.pValue, pValue, noti.len);
        if(rdssProfile_Notify7(peripheralConnList.connHandle, &noti) != SUCCESS)
        {
            GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
        }
    }
}

//static void sendRdssNotifications(void)
//{
//    // RDSS Char2 - 卡信息 (16字节) - 固定测试数据
//    uint8_t rdssCardData[RDSSPROFILE_CHAR2_LEN] = {
//        0x78, 0x56, 0x34, 0x12,  // cardid = 0x12345678
//        0x01,                     // usertype = 1 (一类)
//        0x01,                     // encryped = true (加密)
//        0x05,                     // regional_service_freq = 5
//        0x03,                     // regional_comm_grade = 3
//        0x08,                     // global_service_freq = 8
//        0x04,                     // global_comm_grade = 4
//        0x0A, 0x00,              // subusernumber = 10
//        0x02,                     // cardtype = 2
//        0x00, 0x00, 0x00         // 保留字节
//    };
//    peripheralRdssCardNotify(rdssCardData, RDSSPROFILE_CHAR2_LEN);
//
//    // RDSS Char3 - 信噪比 (12字节) - 固定测试数据
//    uint8_t rdssSnrData[RDSSPROFILE_CHAR3_LEN] = {
//        45, 42, 38, 35, 40, 43,  // 前6颗卫星SNR
//        37, 41, 39, 36, 44, 46   // 后6颗卫星SNR
//    };
//    peripheralRdssSnrNotify(rdssSnrData, RDSSPROFILE_CHAR3_LEN);
//
//    // RDSS Char5 - 发送确认 (8字节) - 固定测试数据
//    uint8_t rdssTxAckData[RDSSPROFILE_CHAR5_LEN] = {
//        0x01,                     // ack = true (成功)
//        0x00,                     // reason = 0 (成功)
//        0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // 保留
//    };
//    peripheralRdssTxAckNotify(rdssTxAckData, RDSSPROFILE_CHAR5_LEN);
//
//    // RDSS Char6 - 接收消息 (16字节) - 固定测试数据
//    uint8_t rdssMsgRxData[RDSSPROFILE_CHAR6_LEN] = {
//        0x00, 0x00, 0xBC, 0x9A,  // sender = 0x9ABC0000
//        0x0E,                     // hour = 14
//        0x1E,                     // minute = 30
//        0x2D,                     // second = 45
//        0x03,                     // encode = 3 (混合编码)
//        0x01, 0x00, 0x00, 0x00,  // mailtype = 1
//        0x20, 0x00, 0x00, 0x00   // payload_len = 32
//    };
//    peripheralRdssMsgRxNotify(rdssMsgRxData, RDSSPROFILE_CHAR6_LEN);
//
//    // RDSS Char7 - 频率倒计时 (4字节) - 固定测试数据
//    uint8_t rdssFreqData[RDSSPROFILE_CHAR7_LEN] = {
//        0x13, 0x00, 0x00, 0x00   // frequency_count_down = 19秒
//    };
//    peripheralRdssFreqNotify(rdssFreqData, RDSSPROFILE_CHAR7_LEN);
//}
// RDSS全部notify
static void sendRdssNotifications(void)
{
    uint8_t rdssCardData[RDSSPROFILE_CHAR2_LEN] = {0};
    uint8_t rdssTxAckData[RDSSPROFILE_CHAR5_LEN] = {0};
    uint8_t rdssMsgRxData[RDSSPROFILE_CHAR6_LEN] = {0};
    uint8_t rdssFreqData[RDSSPROFILE_CHAR7_LEN] = {0};
    uint16_t payload_copy_len;
    uint16_t i;
    // RDSS Char2
    rdssCardData[0] = (uint8_t)(Card_info.cardid & 0x000000FF);
    rdssCardData[1] = (uint8_t)((Card_info.cardid & 0x0000FF00) >> 8);
    rdssCardData[2] = (uint8_t)((Card_info.cardid & 0x00FF0000) >> 16);
    rdssCardData[3] = (uint8_t)((Card_info.cardid & 0xFF000000) >> 24);
    rdssCardData[4] = Card_info.usertype;
    rdssCardData[5] = Card_info.encryped;
    rdssCardData[6] = Card_info.regional_service_freq;
    rdssCardData[7] = Card_info.regional_comm_grade;
    rdssCardData[8] = Card_info.global_service_freq;
    rdssCardData[9] = Card_info.global_comm_grade;
    rdssCardData[10] = (uint8_t)(Card_info.subusernumber & 0x00FF);
    // RDSS Char5: only notify when a new DM229 $BDFKI ack is parsed.
    if(RD_tx_ack_dirty)
    {
        rdssTxAckData[0] = Tx_ack.ack;
        rdssTxAckData[1] = Tx_ack.reason;
        if(peripheralRdssTxAckNotify(rdssTxAckData, RDSSPROFILE_CHAR5_LEN) == SUCCESS)
        {
            RD_tx_ack_dirty = 0;
        }
    }

    // RDSS Char3
    peripheralRdssSnrNotify(snr, RDSSPROFILE_CHAR3_LEN);


    // RDSS Char6
    rdssMsgRxData[0] = (uint8_t)(Msg_rx.sender & 0x000000FF);
    rdssMsgRxData[1] = (uint8_t)((Msg_rx.sender & 0x0000FF00) >> 8);
    rdssMsgRxData[2] = (uint8_t)((Msg_rx.sender & 0x00FF0000) >> 16);
    rdssMsgRxData[3] = (uint8_t)((Msg_rx.sender & 0xFF000000) >> 24);
    rdssMsgRxData[4] = Msg_rx.hour;
    rdssMsgRxData[5] = Msg_rx.minute;
    rdssMsgRxData[6] = Msg_rx.second;
    rdssMsgRxData[7] = Msg_rx.encode;
    rdssMsgRxData[8] = (uint8_t)(Msg_rx.mailtype & 0x000000FF);
    rdssMsgRxData[9] = (uint8_t)((Msg_rx.mailtype & 0x0000FF00) >> 8);
    rdssMsgRxData[10] = (uint8_t)((Msg_rx.mailtype & 0x00FF0000) >> 16);
    rdssMsgRxData[11] = (uint8_t)((Msg_rx.mailtype & 0xFF000000) >> 24);
    rdssMsgRxData[12] = (uint8_t)(Msg_rx.payload_len & 0x000000FF);
    rdssMsgRxData[13] = (uint8_t)((Msg_rx.payload_len & 0x0000FF00) >> 8);
    rdssMsgRxData[14] = (uint8_t)((Msg_rx.payload_len & 0x00FF0000) >> 16);
    rdssMsgRxData[15] = (uint8_t)((Msg_rx.payload_len & 0xFF000000) >> 24);
    payload_copy_len = Msg_rx.payload_len;
    if(payload_copy_len > 70)
    {
        payload_copy_len = 70;
    }
    for(i = 0; i < payload_copy_len; i++)
    {
        rdssMsgRxData[16 + i] = Msg_rx.payload[i];
    }
    peripheralRdssMsgRxNotify(rdssMsgRxData, RDSSPROFILE_CHAR6_LEN);

    // RDSS Char7
    rdssFreqData[0] = (uint8_t)(frequency_count_down & 0x000000FF);
    rdssFreqData[1] = (uint8_t)((frequency_count_down & 0x0000FF00) >> 8);
    rdssFreqData[2] = (uint8_t)((frequency_count_down & 0x00FF0000) >> 16);
    rdssFreqData[3] = (uint8_t)((frequency_count_down & 0xFF000000) >> 24);
    peripheralRdssFreqNotify(rdssFreqData, RDSSPROFILE_CHAR7_LEN);
}

void Peripheral_BleOff(void)
{
    uint8_t advertising_enable = FALSE;

    ble_soft_off = TRUE;
    ble_disconnect_voice_pending = FALSE;
    tmos_stop_task(Peripheral_TaskID, SBP_BLE_DISCONNECT_VOICE_EVT);
    peripheralCancelAutoPowerOff();
    peripheralSetSatelliteModules(FALSE);

    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &advertising_enable);

    if(peripheralConnList.connHandle != GAP_CONNHANDLE_INIT)
    {
        GAPRole_TerminateLink(peripheralConnList.connHandle);
    }

    tmos_stop_task(Peripheral_TaskID, SBP_PERIODIC_EVT);
    tmos_stop_task(Peripheral_TaskID, SBP_READ_RSSI_EVT);
    tmos_stop_task(Peripheral_TaskID, SBP_PARAM_UPDATE_EVT);
}

void Peripheral_BleOn(void)
{
    uint8_t advertising_enable = TRUE;

    ble_soft_off = FALSE;
    peripheralSetSatelliteModules(FALSE);
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &advertising_enable);
}
/*********************************************************************
 *
 * void OTA_IAPReadDataComplete(unsigned char index);
 * 功能：空中下载（OTA）固件升级中，读取数据完成的回调。
 * 用途：在从主机接收完一帧固件数据后调用，用于处理数据或者请求下一帧。
 * 参数：
 * index：unsigned char 类型，表示当前处理的数据包索引。
 * 用于标识固件数据的分段位置（如第 0 帧、第 1 帧）。
*/

/*********************************************************************
 *
 * void OTA_IAPWriteData(unsigned char index, unsigned char *p_data, unsigned char w_len);
 * 功能：向 OTA 缓冲区写入数据。
 * 用途：将接收到的固件数据写入临时存储区，后续用于更新设备固件。
 * 参数：
 * index：数据包索引（同 OTA_IAPReadDataComplete）。
 * p_data：指向 unsigned char 数组的指针，表示待写入的数据缓冲区。
 * w_len：unsigned char 类型，表示数据长度（单位：字节）。
*/

/*********************************************************************
 *
 * void Rec_OTA_IAP_DataDeal(void);
 * 功能：处理接收到的 OTA 数据。
 * 用途：解析接收到的固件数据包，校验数据完整性，以及管理升级流程。
 * 参数：无。
*/

/*********************************************************************
 *
 * void OTA_IAP_SendCMDDealSta(uint8_t deal_status);
 * 功能：发送 OTA 命令处理状态。
 * 用途：向主机反馈固件升级的状态（如成功、失败、进度）。
 * 参数：
 * deal_status：uint8_t 类型，表示处理状态。
 * 常见值包括：
 * 0x00：处理成功。
 * 0x01：处理失败。
 * 0x02：正在处理（进度）。
*/



/*********************************************************************
*********************************************************************/
