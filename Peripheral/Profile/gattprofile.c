/********************************** (C) COPYRIGHT *******************************
 * File Name          : gattprofile.C
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        : 自定义包含五种不同属性的服务，包含可读、可写、通知、可读可写、安全可读
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "gattprofile.h"
#include "HAL.h"
#if SOS_TEST_ONLY_PRINT
#define PROFILE_DEBUG_PRINTF(...)
#else
#define PROFILE_DEBUG_PRINTF(...) printf(__VA_ARGS__)
#endif
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// Position of simpleProfilechar4 value in attribute array
#define SIMPLEPROFILE_CHAR4_VALUE_POS    11

// Position of batteryProfilechar value in attribute array
#define BATTERYPROFILE_CHAR_VALUE_POS    2

// Position of gnssProfilechar value in attribute array
#define GNSSPROFILE_CHAR1_VALUE_POS    2
#define GNSSPROFILE_CHAR2_VALUE_POS    6

// Position of rdssProfilechar value in attribute array
#define RDSSPROFILE_CHAR2_VALUE_POS    5
#define RDSSPROFILE_CHAR3_VALUE_POS    9
#define RDSSPROFILE_CHAR4_VALUE_POS    13
#define RDSSPROFILE_CHAR5_VALUE_POS    16
#define RDSSPROFILE_CHAR6_VALUE_POS    20
#define RDSSPROFILE_CHAR7_VALUE_POS    24
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Simple GATT Profile Service UUID: 0xFFF0
const uint8_t simpleProfileServUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(SIMPLEPROFILE_SERV_UUID), HI_UINT16(SIMPLEPROFILE_SERV_UUID)};

// Characteristic 1 UUID: 0xFFF1
const uint8_t simpleProfilechar1UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(SIMPLEPROFILE_CHAR1_UUID), HI_UINT16(SIMPLEPROFILE_CHAR1_UUID)};

// Characteristic 2 UUID: 0xFFF2
const uint8_t simpleProfilechar2UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(SIMPLEPROFILE_CHAR2_UUID), HI_UINT16(SIMPLEPROFILE_CHAR2_UUID)};

// Characteristic 3 UUID: 0xFFF3
const uint8_t simpleProfilechar3UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(SIMPLEPROFILE_CHAR3_UUID), HI_UINT16(SIMPLEPROFILE_CHAR3_UUID)};

// Characteristic 4 UUID: 0xFFF4
const uint8_t simpleProfilechar4UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(SIMPLEPROFILE_CHAR4_UUID), HI_UINT16(SIMPLEPROFILE_CHAR4_UUID)};

// Characteristic 5 UUID: 0xFFF5
const uint8_t simpleProfilechar5UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(SIMPLEPROFILE_CHAR5_UUID), HI_UINT16(SIMPLEPROFILE_CHAR5_UUID)};

//自定义添加
// Battery GATT Profile Service UUID: 0x180F
const uint8_t batteryProfileServUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(BATT_SERV_UUID), HI_UINT16(BATT_SERV_UUID)};
//  Battery level UUID: 0x2A19
const uint8_t batteryProfilecharUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(BATT_LEVEL_UUID), HI_UINT16(BATT_LEVEL_UUID)};

    // CARD GATT Profile Service UUID: 0x4100
const uint8_t cardProfileServUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(CARDPROFILE_SERV_UUID), HI_UINT16(CARDPROFILE_SERV_UUID)};
//  CARD UUID: 0x4101
const uint8_t cardProfilecharUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(CARDPROFILE_CHAR_UUID), HI_UINT16(CARDPROFILE_CHAR_UUID)};

//控制命令
    // Control GATT Profile Service UUID: 0x4000
const uint8_t controlProfileServUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(CONTROLPROFILE_SERV_UUID), HI_UINT16(CONTROLPROFILE_SERV_UUID)};

// Characteristic 1 UUID: 0x4001
const uint8_t controlProfilechar1UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(CONTROLPROFILE_CHAR1_UUID), HI_UINT16(CONTROLPROFILE_CHAR1_UUID)};

// Characteristic 2 UUID: 0x4002
const uint8_t controlProfilechar2UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(CONTROLPROFILE_CHAR2_UUID), HI_UINT16(CONTROLPROFILE_CHAR2_UUID)};

// Characteristic 3 UUID: 0x4003
const uint8_t controlProfilechar3UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(CONTROLPROFILE_CHAR3_UUID), HI_UINT16(CONTROLPROFILE_CHAR3_UUID)};

// Characteristic 4 UUID: 0x4004
const uint8_t controlProfilechar4UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(CONTROLPROFILE_CHAR4_UUID), HI_UINT16(CONTROLPROFILE_CHAR4_UUID)};

// Characteristic 5 UUID: 0x4005
const uint8_t controlProfilechar5UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(CONTROLPROFILE_CHAR5_UUID), HI_UINT16(CONTROLPROFILE_CHAR5_UUID)};

// Characteristic 6 UUID: 0x4006
const uint8_t controlProfilechar6UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(CONTROLPROFILE_CHAR6_UUID), HI_UINT16(CONTROLPROFILE_CHAR6_UUID)};

// Characteristic 7 UUID: 0x4007
const uint8_t controlProfilechar7UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(CONTROLPROFILE_CHAR7_UUID), HI_UINT16(CONTROLPROFILE_CHAR7_UUID)};

//自定义添加 GNSS服务
// GNSS GATT Profile Service UUID: 0x1819
const uint8_t gnssProfileServUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(LOC_NAV_SERV_UUID), HI_UINT16(LOC_NAV_SERV_UUID)};
//  GNSS 信噪比 UUID: 0x2A6A
const uint8_t gnssProfilechar1UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(LN_FEATURE_UUID), HI_UINT16(LN_FEATURE_UUID)};
//  GNSS 位置信息 UUID: 0x2A67
const uint8_t gnssProfilechar2UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(LOC_SPEED_UUID), HI_UINT16(LOC_SPEED_UUID)};

//自定义添加 RDSS服务
// RDSS GATT Profile Service UUID: 0x4500
const uint8_t rdssProfileServUUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(RDSSPROFILE_SERV_UUID), HI_UINT16(RDSSPROFILE_SERV_UUID)};
//  RDSS 模块信息 UUID: 0x4501
const uint8_t rdssProfilechar1UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(RDSSPROFILE_CHAR1_UUID), HI_UINT16(RDSSPROFILE_CHAR1_UUID)};
//  RDSS 卡信息 UUID: 0x4502
const uint8_t rdssProfilechar2UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(RDSSPROFILE_CHAR2_UUID), HI_UINT16(RDSSPROFILE_CHAR2_UUID)};
//  RDSS 信噪比前12强数组 UUID: 0x4503
const uint8_t rdssProfilechar3UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(RDSSPROFILE_CHAR3_UUID), HI_UINT16(RDSSPROFILE_CHAR3_UUID)};
//  RDSS 卡发送数据 UUID: 0x4504
const uint8_t rdssProfilechar4UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(RDSSPROFILE_CHAR4_UUID), HI_UINT16(RDSSPROFILE_CHAR4_UUID)};
//  RDSS 反馈信息 UUID: 0x4505
const uint8_t rdssProfilechar5UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(RDSSPROFILE_CHAR5_UUID), HI_UINT16(RDSSPROFILE_CHAR5_UUID)};
//  RDSS 入站信息 UUID: 0x4506
const uint8_t rdssProfilechar6UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(RDSSPROFILE_CHAR6_UUID), HI_UINT16(RDSSPROFILE_CHAR6_UUID)};
//  RDSS 频度倒计时 UUID: 0x4507
const uint8_t rdssProfilechar7UUID[ATT_BT_UUID_SIZE] = {
    LO_UINT16(RDSSPROFILE_CHAR7_UUID), HI_UINT16(RDSSPROFILE_CHAR7_UUID)};
/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static simpleProfileCBs_t *simpleProfile_AppCBs = NULL;
static cardProfileCBs_t *cardProfile_AppCBs = NULL;
static batteryProfileCBs_t *batteryProfile_AppCBs = NULL;
static controlProfileCBs_t *controlProfile_AppCBs = NULL;
static gnssProfileCBs_t *gnssProfile_AppCBs = NULL;
static rdssProfileCBs_t *rdssProfile_AppCBs = NULL;
/*********************************************************************
 * Profile Attributes - variables
 */

// Simple Profile Service attribute
static const gattAttrType_t simpleProfileService = {ATT_BT_UUID_SIZE, simpleProfileServUUID};

// Simple Profile Characteristic 1 Properties
static uint8_t simpleProfileChar1Props = GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic 1 Value
static uint8_t simpleProfileChar1[SIMPLEPROFILE_CHAR1_LEN] = {0};

// Simple Profile Characteristic 1 User Description
static uint8_t simpleProfileChar1UserDesp[] = "Char1 service\0";

// Simple Profile Characteristic 2 Properties
static uint8_t simpleProfileChar2Props = GATT_PROP_READ;

// Characteristic 2 Value
static uint8_t simpleProfileChar2[SIMPLEPROFILE_CHAR2_LEN] = {0};

// Simple Profile Characteristic 2 User Description
static uint8_t simpleProfileChar2UserDesp[] = "Char2 service\0";

// Simple Profile Characteristic 3 Properties
static uint8_t simpleProfileChar3Props = GATT_PROP_WRITE;

// Characteristic 3 Value
static uint8_t simpleProfileChar3[SIMPLEPROFILE_CHAR3_LEN] = {0};

// Simple Profile Characteristic 3 User Description
static uint8_t simpleProfileChar3UserDesp[] = "Char3 service\0";

// Simple Profile Characteristic 4 Properties
static uint8_t simpleProfileChar4Props = GATT_PROP_NOTIFY;

// Characteristic 4 Value
static uint8_t simpleProfileChar4[SIMPLEPROFILE_CHAR4_LEN] = {0};

// Simple Profile Characteristic 4 Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t simpleProfileChar4Config[PERIPHERAL_MAX_CONNECTION];

// Simple Profile Characteristic 4 User Description
static uint8_t simpleProfileChar4UserDesp[] = "Char4 service\0";

// Simple Profile Characteristic 5 Properties
static uint8_t simpleProfileChar5Props = GATT_PROP_READ;

// Characteristic 5 Value
static uint8_t simpleProfileChar5[SIMPLEPROFILE_CHAR5_LEN] = {0};

// Simple Profile Characteristic 5 User Description
static uint8_t simpleProfileChar5UserDesp[] = "Char5 service\0";

//定义一个电池电量服务
// Battery Service attribute
static const gattAttrType_t batteryProfileService = {ATT_BT_UUID_SIZE, batteryProfileServUUID};
 //特征值初始化
// 特征值属性，读或者写
// Battery Level Properties
static uint8_t batteryProfileCharProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
// 值，可以向其写入数据，也可以读出数据。这里是一个字符数组
// Battery Level Value
static uint8_t batteryProfileChar[BATTERYPROFILE_CHAR_LEN] = {0};
// 用户描述，展现给用户的名字
// Battery Level User Description
static uint8_t batteryProfileCharUserDesp[] = "Battery Level\0";
static gattCharCfg_t batteryProfileCharConfig[PERIPHERAL_MAX_CONNECTION];

//定义一个服务
static const gattAttrType_t cardProfileService = {ATT_BT_UUID_SIZE, cardProfileServUUID};
 //特征值初始化
// 特征值属性，读或者写
static uint8_t cardProfileCharProps = GATT_PROP_READ | GATT_PROP_WRITE;
// 值，可以向其写入数据，也可以读出数据。这里是一个字符数组
static uint8_t cardProfileChar[CARDPROFILE_CHAR_LEN] = {0};
// 用户描述，展现给用户的名字
static uint8_t cardProfileCharUserDesp[] = "card info\0";

//定义一个控制服务
static const gattAttrType_t controlProfileService = {ATT_BT_UUID_SIZE, controlProfileServUUID};

// Control Profile Characteristic 1 Properties
static uint8_t controlProfileChar1Props = GATT_PROP_READ | GATT_PROP_WRITE;
// Characteristic 1 Value
static uint8_t controlProfileChar1[CONTROLPROFILE_CHAR1_LEN] = {0};
// Control Profile Characteristic 1 User Description
static uint8_t controlProfileChar1UserDesp[] = "GNSS switch\0";

// Control Profile Characteristic 2 Properties
static uint8_t controlProfileChar2Props = GATT_PROP_READ | GATT_PROP_WRITE;
// Characteristic 2 Value
static uint8_t controlProfileChar2[CONTROLPROFILE_CHAR2_LEN] = {0};
// Control Profile Characteristic 2 User Description
static uint8_t controlProfileChar2UserDesp[] = "RDSS switch\0";

// Control Profile Characteristic 3 Properties
static uint8_t controlProfileChar3Props = GATT_PROP_READ | GATT_PROP_WRITE;
// Characteristic 3 Value
static uint8_t controlProfileChar3[CONTROLPROFILE_CHAR3_LEN] = {0};
// Controle Profile Characteristic 3 User Description
static uint8_t controlProfileChar3UserDesp[] = "SOS switch\0";

// Control Profile Characteristic 4 Properties
static uint8_t controlProfileChar4Props = GATT_PROP_READ | GATT_PROP_WRITE;
// Characteristic 4 Value
static uint8_t controlProfileChar4[CONTROLPROFILE_CHAR4_LEN] = {0};
// Control Profile Characteristic 4 User Description
static uint8_t controlProfileChar4UserDesp[] = "RDSS test\0";

// Control Profile Characteristic 5 Properties
static uint8_t controlProfileChar5Props = GATT_PROP_WRITE;
// Characteristic 5 Value
static uint8_t controlProfileChar5[CONTROLPROFILE_CHAR5_LEN] = {0};
// Control Profile Characteristic 5 User Description
static uint8_t controlProfileChar5UserDesp[] = "RDSS reset\0";

// Control Profile Characteristic 6 Properties
static uint8_t controlProfileChar6Props = GATT_PROP_READ | GATT_PROP_WRITE;
// Characteristic 6 Value
static uint8_t controlProfileChar6[CONTROLPROFILE_CHAR6_LEN] = {0};
// Control Profile Characteristic 6 User Description
static uint8_t controlProfileChar6UserDesp[] = "Location timer\0";

// Control Profile Characteristic 7 Properties
static uint8_t controlProfileChar7Props = GATT_PROP_WRITE;
// Characteristic 7 Value
static uint8_t controlProfileChar7[CONTROLPROFILE_CHAR7_LEN] = {0};
// Control Profile Characteristic 7 User Description
static uint8_t controlProfileChar7UserDesp[] = "Location send\0";

//定义一个GNSS定位服务
//GNSS Service attribute
static const gattAttrType_t gnssProfileService = {ATT_BT_UUID_SIZE, gnssProfileServUUID};
 //特征值初始化
// 特征值属性，读或者写
static uint8_t gnssProfileChar1Props = GATT_PROP_READ | GATT_PROP_NOTIFY;
// 值，可以向其写入数据，也可以读出数据。这里是一个字符数组
static uint8_t gnssProfileChar1[GNSSPROFILE_CHAR1_LEN] = {0};
// 用户描述，展现给用户的名字
static uint8_t gnssProfileChar1UserDesp[] = "LN Feature\0";
static gattCharCfg_t gnssProfileChar1Config[PERIPHERAL_MAX_CONNECTION];

// 特征值属性，读或者写
static uint8_t gnssProfileChar2Props = GATT_PROP_READ | GATT_PROP_NOTIFY;
// 值，可以向其写入数据，也可以读出数据。这里是一个字符数组
static uint8_t gnssProfileChar2[GNSSPROFILE_CHAR2_LEN] = {0};
// 用户描述，展现给用户的名字
static uint8_t gnssProfileChar2UserDesp[] = "Location and Speed\0";
static gattCharCfg_t gnssProfileChar2Config[PERIPHERAL_MAX_CONNECTION];

//定义一个RDSS短报文服务
//RDSS Service attribute
static const gattAttrType_t rdssProfileService = {ATT_BT_UUID_SIZE, rdssProfileServUUID};
 //特征值初始化
// 特征值属性，读或者写
static uint8_t rdssProfileChar1Props = GATT_PROP_READ;
// 值，可以向其写入数据，也可以读出数据。这里是一个字符数组
static uint8_t rdssProfileChar1[RDSSPROFILE_CHAR1_LEN] = {0};
// 用户描述，展现给用户的名字
static uint8_t rdssProfileChar1UserDesp[] = "Model Info\0";

// 特征值属性，读或者写
static uint8_t rdssProfileChar2Props = GATT_PROP_READ | GATT_PROP_NOTIFY;
// 值，可以向其写入数据，也可以读出数据。这里是一个字符数组
static uint8_t rdssProfileChar2[RDSSPROFILE_CHAR2_LEN] = {0};
// 用户描述，展现给用户的名字
static uint8_t rdssProfileChar2UserDesp[] = "Card Info\0";
static gattCharCfg_t rdssProfileChar2Config[PERIPHERAL_MAX_CONNECTION];

// 特征值属性，读或者写
static uint8_t rdssProfileChar3Props = GATT_PROP_READ | GATT_PROP_NOTIFY;
// 值，可以向其写入数据，也可以读出数据。这里是一个字符数组
static uint8_t rdssProfileChar3[RDSSPROFILE_CHAR3_LEN] = {0};
// 用户描述，展现给用户的名字
static uint8_t rdssProfileChar3UserDesp[] = "SNR\0";
static gattCharCfg_t rdssProfileChar3Config[PERIPHERAL_MAX_CONNECTION];

// 特征值属性，读或者写
static uint8_t rdssProfileChar4Props = GATT_PROP_WRITE;
// 值，可以向其写入数据，也可以读出数据。这里是一个字符数组
static uint8_t rdssProfileChar4[RDSSPROFILE_CHAR4_LEN] = {0};
// 用户描述，展现给用户的名字
static uint8_t rdssProfileChar4UserDesp[] = "MSG TX\0";

// 特征值属性，读或者写
static uint8_t rdssProfileChar5Props = GATT_PROP_READ | GATT_PROP_NOTIFY;
// 值，可以向其写入数据，也可以读出数据。这里是一个字符数组
static uint8_t rdssProfileChar5[RDSSPROFILE_CHAR5_LEN] = {0};
// 用户描述，展现给用户的名字
static uint8_t rdssProfileChar5UserDesp[] = "TX ACK\0";
static gattCharCfg_t rdssProfileChar5Config[PERIPHERAL_MAX_CONNECTION];

// 特征值属性，读或者写
static uint8_t rdssProfileChar6Props = GATT_PROP_READ | GATT_PROP_NOTIFY;
// 值，可以向其写入数据，也可以读出数据。这里是一个字符数组
static uint8_t rdssProfileChar6[RDSSPROFILE_CHAR6_LEN] = {0};
// 用户描述，展现给用户的名字
static uint8_t rdssProfileChar6UserDesp[] = "MSG RX\0";
static gattCharCfg_t rdssProfileChar6Config[PERIPHERAL_MAX_CONNECTION];

// 特征值属性，读或者写
static uint8_t rdssProfileChar7Props = GATT_PROP_READ | GATT_PROP_NOTIFY;
// 值，可以向其写入数据，也可以读出数据。这里是一个字符数组
static uint8_t rdssProfileChar7[RDSSPROFILE_CHAR7_LEN] = {0};
// 用户描述，展现给用户的名字
static uint8_t rdssProfileChar7UserDesp[] = "Freq Count Down\0";
static gattCharCfg_t rdssProfileChar7Config[PERIPHERAL_MAX_CONNECTION];
/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t simpleProfileAttrTbl[] = {
    // Simple Profile Service
    {
        {ATT_BT_UUID_SIZE, primaryServiceUUID}, /* type */
        GATT_PERMIT_READ,                       /* permissions */
        0,                                      /* handle */
        (uint8_t *)&simpleProfileService        /* pValue */
    },

    // Characteristic 1 Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &simpleProfileChar1Props},

    // Characteristic Value 1
    {
        {ATT_BT_UUID_SIZE, simpleProfilechar1UUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        simpleProfileChar1},

    // Characteristic 1 User Description
    {
        {ATT_BT_UUID_SIZE, charUserDescUUID},
        GATT_PERMIT_READ,
        0,
        simpleProfileChar1UserDesp},

    // Characteristic 2 Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &simpleProfileChar2Props},

    // Characteristic Value 2
    {
        {ATT_BT_UUID_SIZE, simpleProfilechar2UUID},
        GATT_PERMIT_READ,
        0,
        simpleProfileChar2},

    // Characteristic 2 User Description
    {
        {ATT_BT_UUID_SIZE, charUserDescUUID},
        GATT_PERMIT_READ,
        0,
        simpleProfileChar2UserDesp},

    // Characteristic 3 Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &simpleProfileChar3Props},

    // Characteristic Value 3
    {
        {ATT_BT_UUID_SIZE, simpleProfilechar3UUID},
        GATT_PERMIT_WRITE,
        0,
        simpleProfileChar3},

    // Characteristic 3 User Description
    {
        {ATT_BT_UUID_SIZE, charUserDescUUID},
        GATT_PERMIT_READ,
        0,
        simpleProfileChar3UserDesp},

    // Characteristic 4 Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &simpleProfileChar4Props},

    // Characteristic Value 4
    {
        {ATT_BT_UUID_SIZE, simpleProfilechar4UUID},
        0,
        0,
        simpleProfileChar4},

    // Characteristic 4 configuration
    {
        {ATT_BT_UUID_SIZE, clientCharCfgUUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8_t *)simpleProfileChar4Config},

    // Characteristic 4 User Description
    {
        {ATT_BT_UUID_SIZE, charUserDescUUID},
        GATT_PERMIT_READ,
        0,
        simpleProfileChar4UserDesp},

    // Characteristic 5 Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &simpleProfileChar5Props},

    // Characteristic Value 5
    {
        {ATT_BT_UUID_SIZE, simpleProfilechar5UUID},
        GATT_PERMIT_AUTHEN_READ,
        0,
        simpleProfileChar5},

    // Characteristic 5 User Description
    {
        {ATT_BT_UUID_SIZE, charUserDescUUID},
        GATT_PERMIT_READ,
        0,
        simpleProfileChar5UserDesp},
};

//属性表
static gattAttribute_t cardProfileAttrTb[]=
{
        //led priofile service
        {
            {ATT_BT_UUID_SIZE,primaryServiceUUID}, //type(属性类型)   属性类型：主服务primaryServiceUUIDd为0x2800即主服务类型。
            GATT_PERMIT_READ,                      //permissions(GATT客户端对于该属性的权限)    即客户端对于该服务的权限，GATT_PERMIT_READ客户端可以发现这个服务。
            0,                                     //handle(表中属性的索引)    即在这个属性表中的位置指明这是在这个数组中的第几个数组元素,因为这个句柄是协议栈自动分配的，因此默认初始化为0
            (uint8_t *)&cardProfileService            //pValue(指向属性值的指针)    用自己定义的UUID值0xFFF0来指向这个服务
        },
        //characteristic Declaration
        {
            {ATT_BT_UUID_SIZE,characterUUID},    //属性类型：特征值  characterUUID为0x2803即特征值类型
            GATT_PERMIT_READ,                    //数组这个元素权限为可读
            0,
            &cardProfileCharProps                 //ledProfileCharProps描述的是特征值具有的属性(指读写属性)，一个是权限一个是属性。
        },
        //characteristic Value
        {
            {ATT_BT_UUID_SIZE,cardProfilecharUUID},    //特征值的自定义UUID
            GATT_PERMIT_READ | GATT_PERMIT_WRITE,     //权限为可读可写，要与特征值类型描述里的ledProfileCharProps特征值属性一致。
            0,
            cardProfileChar                            //特征值存放地方，就是修改里面的内容实现特征值的读写
        },
        //charateristic User Description
        {
            {ATT_BT_UUID_SIZE,charUserDescUUID},     //特征描述的UUID
            GATT_PERMIT_READ,
            0,
            cardProfileCharUserDesp                   //描述的内容
        },

};

//属性表
static gattAttribute_t batteryProfileAttrTb[]=
{
        //battery priofile service
        {
            {ATT_BT_UUID_SIZE,primaryServiceUUID}, //type(属性类型)   属性类型：主服务primaryServiceUUIDd为0x2800即主服务类型。
            GATT_PERMIT_READ,                      //permissions(GATT客户端对于该属性的权限)    即客户端对于该服务的权限，GATT_PERMIT_READ客户端可以发现这个服务。
            0,                                     //handle(表中属性的索引)    即在这个属性表中的位置指明这是在这个数组中的第几个数组元素,因为这个句柄是协议栈自动分配的，因此默认初始化为0
            (uint8_t *)&batteryProfileService            //pValue(指向属性值的指针)    用自己定义的UUID值0x180F来指向这个服务
        },
        //characteristic Declaration
        {
            {ATT_BT_UUID_SIZE,characterUUID},    //属性类型：特征值  characterUUID为0x2803即特征值类型
            GATT_PERMIT_READ,                    //数组这个元素权限为可读
            0,
            &batteryProfileCharProps                 //batteryProfileCharProps描述的是特征值具有的属性(指读写属性)，一个是权限一个是属性。
        },
        //characteristic Value
        {
            {ATT_BT_UUID_SIZE,batteryProfilecharUUID},    //特征值的自定义UUID
            GATT_PERMIT_READ ,                          //权限为可读，要与特征值类型描述里的ledProfileCharProps特征值属性一致。
            0,
            batteryProfileChar                            //特征值存放地方，就是修改里面的内容实现特征值的读写
        },
        // Characteristic  configuration Notify的配置
        {
            {ATT_BT_UUID_SIZE, clientCharCfgUUID},
            GATT_PERMIT_READ | GATT_PERMIT_WRITE,
            0,
            (uint8_t *)batteryProfileCharConfig
        },
        //charateristic User Description
        {
            {ATT_BT_UUID_SIZE,charUserDescUUID},     //特征描述的UUID
            GATT_PERMIT_READ,
            0,
            batteryProfileCharUserDesp                   //描述的内容
        },

};
//控制命令属性表
static gattAttribute_t controlProfileAttrTb[] = {
    // control Profile Service
    {
        {ATT_BT_UUID_SIZE, primaryServiceUUID}, /* type */
        GATT_PERMIT_READ,                       /* permissions */
        0,                                      /* handle */
        (uint8_t *)&controlProfileService        /* pValue */
    },

    // Characteristic 1 Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &controlProfileChar1Props},

    // Characteristic Value 1
    {
        {ATT_BT_UUID_SIZE, controlProfilechar1UUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        controlProfileChar1},

    // Characteristic 1 User Description
    {
        {ATT_BT_UUID_SIZE, charUserDescUUID},
        GATT_PERMIT_READ,
        0,
        controlProfileChar1UserDesp},

    // Characteristic 2 Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &controlProfileChar2Props},

    // Characteristic Value 2
    {
        {ATT_BT_UUID_SIZE, controlProfilechar2UUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        controlProfileChar2},

    // Characteristic 2 User Description
    {
        {ATT_BT_UUID_SIZE, charUserDescUUID},
        GATT_PERMIT_READ,
        0,
        controlProfileChar2UserDesp},

    // Characteristic 3 Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &controlProfileChar3Props},

    // Characteristic Value 3
    {
        {ATT_BT_UUID_SIZE, controlProfilechar3UUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        controlProfileChar3},

    // Characteristic 3 User Description
    {
        {ATT_BT_UUID_SIZE, charUserDescUUID},
        GATT_PERMIT_READ,
        0,
        controlProfileChar3UserDesp},

    // Characteristic 4 Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &controlProfileChar4Props},

    // Characteristic Value 4
    {
        {ATT_BT_UUID_SIZE, controlProfilechar4UUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        controlProfileChar4},

     // Characteristic 4 User Description
    {
        {ATT_BT_UUID_SIZE, charUserDescUUID},
        GATT_PERMIT_READ,
        0,
        controlProfileChar4UserDesp},

    // Characteristic 5 Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &controlProfileChar5Props},

    // Characteristic Value 5
    {
        {ATT_BT_UUID_SIZE, controlProfilechar5UUID},
        GATT_PERMIT_WRITE,
        0,
        controlProfileChar5},

    // Characteristic 5 User Description
    {
        {ATT_BT_UUID_SIZE, charUserDescUUID},
        GATT_PERMIT_READ,
        0,
        controlProfileChar5UserDesp},

    // Characteristic 6 Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &controlProfileChar6Props},

    // Characteristic Value 6
    {
        {ATT_BT_UUID_SIZE, controlProfilechar6UUID},
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        controlProfileChar6},

    // Characteristic 6 User Description
    {
        {ATT_BT_UUID_SIZE, charUserDescUUID},
        GATT_PERMIT_READ,
        0,
        controlProfileChar6UserDesp},

    // Characteristic 7 Declaration
    {
        {ATT_BT_UUID_SIZE, characterUUID},
        GATT_PERMIT_READ,
        0,
        &controlProfileChar7Props},

    // Characteristic Value 7
    {
        {ATT_BT_UUID_SIZE, controlProfilechar7UUID},
        GATT_PERMIT_WRITE,
        0,
        controlProfileChar7},

    // Characteristic 7 User Description
    {
        {ATT_BT_UUID_SIZE, charUserDescUUID},
        GATT_PERMIT_READ,
        0,
        controlProfileChar7UserDesp},
};

//属性表
static gattAttribute_t gnssProfileAttrTb[]=
{
        //gnss priofile service
        {
            {ATT_BT_UUID_SIZE,primaryServiceUUID}, //type(属性类型)   属性类型：主服务primaryServiceUUIDd为0x2800即主服务类型。
            GATT_PERMIT_READ,                      //permissions(GATT客户端对于该属性的权限)    即客户端对于该服务的权限，GATT_PERMIT_READ客户端可以发现这个服务。
            0,                                     //handle(表中属性的索引)    即在这个属性表中的位置指明这是在这个数组中的第几个数组元素,因为这个句柄是协议栈自动分配的，因此默认初始化为0
            (uint8_t *)&gnssProfileService            //pValue(指向属性值的指针)    用自己定义的UUID值0x180F来指向这个服务
        },
        //characteristic Declaration
        {
            {ATT_BT_UUID_SIZE,characterUUID},    //属性类型：特征值  characterUUID为0x2803即特征值类型
            GATT_PERMIT_READ,                    //数组这个元素权限为可读
            0,
            &gnssProfileChar1Props                 //batteryProfileCharProps描述的是特征值具有的属性(指读写属性)，一个是权限一个是属性。
        },
        //characteristic Value
        {
            {ATT_BT_UUID_SIZE,gnssProfilechar1UUID},    //特征值的自定义UUID
            GATT_PERMIT_READ ,                          //权限为可读，要与特征值类型描述里的ledProfileCharProps特征值属性一致。
            0,
            gnssProfileChar1                            //特征值存放地方，就是修改里面的内容实现特征值的读写
        },
        // Characteristic  configuration Notify的配置
        {
            {ATT_BT_UUID_SIZE, clientCharCfgUUID},
            GATT_PERMIT_READ | GATT_PERMIT_WRITE,
            0,
            (uint8_t *)gnssProfileChar1Config
        },
        //charateristic User Description
        {
            {ATT_BT_UUID_SIZE,charUserDescUUID},     //特征描述的UUID
            GATT_PERMIT_READ,
            0,
            gnssProfileChar1UserDesp                   //描述的内容
        },
        //characteristic Declaration
        {
            {ATT_BT_UUID_SIZE,characterUUID},    //属性类型：特征值  characterUUID为0x2803即特征值类型
            GATT_PERMIT_READ,                    //数组这个元素权限为可读
            0,
            &gnssProfileChar2Props                 //batteryProfileCharProps描述的是特征值具有的属性(指读写属性)，一个是权限一个是属性。
        },
        //characteristic Value
        {
            {ATT_BT_UUID_SIZE,gnssProfilechar2UUID},    //特征值的自定义UUID
            GATT_PERMIT_READ ,                          //权限为可读，要与特征值类型描述里的ledProfileCharProps特征值属性一致。
            0,
            gnssProfileChar2                            //特征值存放地方，就是修改里面的内容实现特征值的读写
        },
        // Characteristic  configuration Notify的配置
        {
            {ATT_BT_UUID_SIZE, clientCharCfgUUID},
            GATT_PERMIT_READ | GATT_PERMIT_WRITE,
            0,
            (uint8_t *)gnssProfileChar2Config
        },
        //charateristic User Description
        {
            {ATT_BT_UUID_SIZE,charUserDescUUID},     //特征描述的UUID
            GATT_PERMIT_READ,
            0,
            gnssProfileChar2UserDesp                   //描述的内容
        },
};

//属性表
static gattAttribute_t rdssProfileAttrTb[]=
{
        //rdss priofile service
        {
            {ATT_BT_UUID_SIZE,primaryServiceUUID}, //type(属性类型)   属性类型：主服务primaryServiceUUIDd为0x2800即主服务类型。
            GATT_PERMIT_READ,                      //permissions(GATT客户端对于该属性的权限)    即客户端对于该服务的权限，GATT_PERMIT_READ客户端可以发现这个服务。
            0,                                     //handle(表中属性的索引)    即在这个属性表中的位置指明这是在这个数组中的第几个数组元素,因为这个句柄是协议栈自动分配的，因此默认初始化为0
            (uint8_t *)&rdssProfileService            //pValue(指向属性值的指针)    用自己定义的UUID值0x180F来指向这个服务
        },
        //characteristic Declaration
        {
            {ATT_BT_UUID_SIZE,characterUUID},    //属性类型：特征值  characterUUID为0x2803即特征值类型
            GATT_PERMIT_READ,                    //数组这个元素权限为可读
            0,
            &rdssProfileChar1Props                 //batteryProfileCharProps描述的是特征值具有的属性(指读写属性)，一个是权限一个是属性。
        },
        //characteristic Value
        {
            {ATT_BT_UUID_SIZE,rdssProfilechar1UUID},    //特征值的自定义UUID
            GATT_PERMIT_READ ,                          //权限为可读，要与特征值类型描述里的ledProfileCharProps特征值属性一致。
            0,
            rdssProfileChar1                            //特征值存放地方，就是修改里面的内容实现特征值的读写
        },
        //charateristic User Description
        {
            {ATT_BT_UUID_SIZE,charUserDescUUID},     //特征描述的UUID
            GATT_PERMIT_READ,
            0,
            rdssProfileChar1UserDesp                   //描述的内容
        },
        //characteristic Declaration
        {
            {ATT_BT_UUID_SIZE,characterUUID},    //属性类型：特征值  characterUUID为0x2803即特征值类型
            GATT_PERMIT_READ,                    //数组这个元素权限为可读
            0,
            &rdssProfileChar2Props                 //batteryProfileCharProps描述的是特征值具有的属性(指读写属性)，一个是权限一个是属性。
        },
        //characteristic Value
        {
            {ATT_BT_UUID_SIZE,rdssProfilechar2UUID},    //特征值的自定义UUID
            GATT_PERMIT_READ ,                          //权限为可读，要与特征值类型描述里的ledProfileCharProps特征值属性一致。
            0,
            rdssProfileChar2                            //特征值存放地方，就是修改里面的内容实现特征值的读写
        },
        // Characteristic  configuration Notify的配置
        {
            {ATT_BT_UUID_SIZE, clientCharCfgUUID},
            GATT_PERMIT_READ | GATT_PERMIT_WRITE,
            0,
            (uint8_t *)rdssProfileChar2Config
        },
        //charateristic User Description
        {
            {ATT_BT_UUID_SIZE,charUserDescUUID},     //特征描述的UUID
            GATT_PERMIT_READ,
            0,
            rdssProfileChar2UserDesp                   //描述的内容
        },
        //characteristic Declaration
        {
            {ATT_BT_UUID_SIZE,characterUUID},    //属性类型：特征值  characterUUID为0x2803即特征值类型
            GATT_PERMIT_READ,                    //数组这个元素权限为可读
            0,
            &rdssProfileChar3Props                 //batteryProfileCharProps描述的是特征值具有的属性(指读写属性)，一个是权限一个是属性。
        },
        //characteristic Value
        {
            {ATT_BT_UUID_SIZE,rdssProfilechar3UUID},    //特征值的自定义UUID
            GATT_PERMIT_READ ,                          //权限为可读，要与特征值类型描述里的ledProfileCharProps特征值属性一致。
            0,
            rdssProfileChar3                            //特征值存放地方，就是修改里面的内容实现特征值的读写
        },
        // Characteristic  configuration Notify的配置
        {
            {ATT_BT_UUID_SIZE, clientCharCfgUUID},
            GATT_PERMIT_READ | GATT_PERMIT_WRITE,
            0,
            (uint8_t *)rdssProfileChar3Config
        },
        //charateristic User Description
        {
            {ATT_BT_UUID_SIZE,charUserDescUUID},     //特征描述的UUID
            GATT_PERMIT_READ,
            0,
            rdssProfileChar3UserDesp                   //描述的内容
        },
        //characteristic Declaration
        {
            {ATT_BT_UUID_SIZE,characterUUID},    //属性类型：特征值  characterUUID为0x2803即特征值类型
            GATT_PERMIT_READ,                    //数组这个元素权限为可读
            0,
            &rdssProfileChar4Props                 //batteryProfileCharProps描述的是特征值具有的属性(指读写属性)，一个是权限一个是属性。
        },
        //characteristic Value
        {
            {ATT_BT_UUID_SIZE,rdssProfilechar4UUID},    //特征值的自定义UUID
            GATT_PERMIT_WRITE,                          //权限为可读，要与特征值类型描述里的ledProfileCharProps特征值属性一致。
            0,
            rdssProfileChar4                            //特征值存放地方，就是修改里面的内容实现特征值的读写
        },
        //charateristic User Description
        {
            {ATT_BT_UUID_SIZE,charUserDescUUID},     //特征描述的UUID
            GATT_PERMIT_READ,
            0,
            rdssProfileChar4UserDesp                   //描述的内容
        },
        //characteristic Declaration
        {
            {ATT_BT_UUID_SIZE,characterUUID},    //属性类型：特征值  characterUUID为0x2803即特征值类型
            GATT_PERMIT_READ,                    //数组这个元素权限为可读
            0,
            &rdssProfileChar5Props                 //batteryProfileCharProps描述的是特征值具有的属性(指读写属性)，一个是权限一个是属性。
        },
        //characteristic Value
        {
            {ATT_BT_UUID_SIZE,rdssProfilechar5UUID},    //特征值的自定义UUID
            GATT_PERMIT_READ ,                          //权限为可读，要与特征值类型描述里的ledProfileCharProps特征值属性一致。
            0,
            rdssProfileChar5                            //特征值存放地方，就是修改里面的内容实现特征值的读写
        },
        // Characteristic  configuration Notify的配置
        {
            {ATT_BT_UUID_SIZE, clientCharCfgUUID},
            GATT_PERMIT_READ | GATT_PERMIT_WRITE,
            0,
            (uint8_t *)rdssProfileChar5Config
        },
        //charateristic User Description
        {
            {ATT_BT_UUID_SIZE,charUserDescUUID},     //特征描述的UUID
            GATT_PERMIT_READ,
            0,
            rdssProfileChar5UserDesp                   //描述的内容
        },
        //characteristic Declaration
        {
            {ATT_BT_UUID_SIZE,characterUUID},    //属性类型：特征值  characterUUID为0x2803即特征值类型
            GATT_PERMIT_READ,                    //数组这个元素权限为可读
            0,
            &rdssProfileChar6Props                 //batteryProfileCharProps描述的是特征值具有的属性(指读写属性)，一个是权限一个是属性。
        },
        //characteristic Value
        {
            {ATT_BT_UUID_SIZE,rdssProfilechar6UUID},    //特征值的自定义UUID
            GATT_PERMIT_READ ,                          //权限为可读，要与特征值类型描述里的ledProfileCharProps特征值属性一致。
            0,
            rdssProfileChar6                            //特征值存放地方，就是修改里面的内容实现特征值的读写
        },
        // Characteristic  configuration Notify的配置
        {
            {ATT_BT_UUID_SIZE, clientCharCfgUUID},
            GATT_PERMIT_READ | GATT_PERMIT_WRITE,
            0,
            (uint8_t *)rdssProfileChar6Config
        },
        //charateristic User Description
        {
            {ATT_BT_UUID_SIZE,charUserDescUUID},     //特征描述的UUID
            GATT_PERMIT_READ,
            0,
            rdssProfileChar6UserDesp                   //描述的内容
        },
        //characteristic Declaration
        {
            {ATT_BT_UUID_SIZE,characterUUID},    //属性类型：特征值  characterUUID为0x2803即特征值类型
            GATT_PERMIT_READ,                    //数组这个元素权限为可读
            0,
            &rdssProfileChar7Props                 //batteryProfileCharProps描述的是特征值具有的属性(指读写属性)，一个是权限一个是属性。
        },
        //characteristic Value
        {
            {ATT_BT_UUID_SIZE,rdssProfilechar7UUID},    //特征值的自定义UUID
            GATT_PERMIT_READ ,                          //权限为可读，要与特征值类型描述里的ledProfileCharProps特征值属性一致。
            0,
            rdssProfileChar7                            //特征值存放地方，就是修改里面的内容实现特征值的读写
        },
        // Characteristic  configuration Notify的配置
        {
            {ATT_BT_UUID_SIZE, clientCharCfgUUID},
            GATT_PERMIT_READ | GATT_PERMIT_WRITE,
            0,
            (uint8_t *)rdssProfileChar7Config
        },
        //charateristic User Description
        {
            {ATT_BT_UUID_SIZE,charUserDescUUID},     //特征描述的UUID
            GATT_PERMIT_READ,
            0,
            rdssProfileChar7UserDesp                   //描述的内容
        },
};
/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t simpleProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method);
static bStatus_t simpleProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method);

static void simpleProfile_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType);


static bStatus_t batteryProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method);
static bStatus_t batteryProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method);

static void batteryProfile_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType);


static bStatus_t controlProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method);
static bStatus_t controlProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method);

static bStatus_t cardProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method);
static bStatus_t cardProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method);

static bStatus_t gnssProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method);
static bStatus_t gnssProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method);

static void gnssProfile_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType);

static bStatus_t rdssProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method);
static bStatus_t rdssProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method);

static void rdssProfile_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType);
/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
gattServiceCBs_t simpleProfileCBs = {
    simpleProfile_ReadAttrCB,  // Read callback function pointer
    simpleProfile_WriteAttrCB, // Write callback function pointer
    NULL                       // Authorization callback function pointer
};

//读写回调设置
gattServiceCBs_t batteryProfileCBs = {
    batteryProfile_ReadAttrCB,  // Read callback function pointer
    batteryProfile_WriteAttrCB, // Write callback function pointer

    NULL                       // Authorization callback function pointer
};

//读写回调设置
gattServiceCBs_t controlProfileCBs = {
    controlProfile_ReadAttrCB,  // Read callback function pointer
    controlProfile_WriteAttrCB, // Write callback function pointer

    NULL                       // Authorization callback function pointer
};

//读写回调设置
gattServiceCBs_t cardProfileCBs = {
    cardProfile_ReadAttrCB,  // Read callback function pointer
    cardProfile_WriteAttrCB, // Write callback function pointer

    NULL                       // Authorization callback function pointer
};

//读写回调设置
gattServiceCBs_t gnssProfileCBs = {
    gnssProfile_ReadAttrCB,  // Read callback function pointer
    gnssProfile_WriteAttrCB, // Write callback function pointer

    NULL                       // Authorization callback function pointer
};

//读写回调设置
gattServiceCBs_t rdssProfileCBs = {
    rdssProfile_ReadAttrCB,  // Read callback function pointer
    rdssProfile_WriteAttrCB, // Write callback function pointer

    NULL                       // Authorization callback function pointer
};
/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleProfile_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
//注册服务函数
bStatus_t Battery_Addservice(uint32_t services)
{
    uint8_t status = SUCCESS;

    // 注册notify特征
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, batteryProfileCharConfig);

    // 连接状态回调
    linkDB_Register(batteryProfile_HandleConnStatusCB);

    if(services & BATTERYPROFILE_SERVICE)
    {
        status = GATTServApp_RegisterService(batteryProfileAttrTb,             //属性表
                                        GATT_NUM_ATTRS(batteryProfileAttrTb),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &batteryProfileCBs);                    //读写回调函数有数据读写时调到读写函数中
    }
    return (status);
}

//注册服务函数
bStatus_t Control_Addservice(uint32_t services)
{
    uint8_t status = SUCCESS;

    if(services & CONTROLPROFILE_SERVICE)
    {
        status = GATTServApp_RegisterService(controlProfileAttrTb,             //属性表
                                        GATT_NUM_ATTRS(controlProfileAttrTb),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &controlProfileCBs);                    //读写回调函数有数据读写时调到读写函数中
    }
    return (status);

}

//注册服务函数
bStatus_t CardProfile_Addservice(uint32_t services)
{
    uint8_t status = SUCCESS;

    if(services & CARDPROFILE_SERVICE)
    {
        status = GATTServApp_RegisterService(cardProfileAttrTb,             //属性表
                                        GATT_NUM_ATTRS(cardProfileAttrTb),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &cardProfileCBs);                    //读写回调函数有数据读写时调到读写函数中
    }
    return (status);

}

bStatus_t SimpleProfile_AddService(uint32_t services)
{
    uint8_t status = SUCCESS;

    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, simpleProfileChar4Config);

    // Register with Link DB to receive link status change callback
    linkDB_Register(simpleProfile_HandleConnStatusCB);

    if(services & SIMPLEPROFILE_SERVICE)
    {
        // Register GATT attribute list and CBs with GATT Server App
        status = GATTServApp_RegisterService(simpleProfileAttrTbl,
                                             GATT_NUM_ATTRS(simpleProfileAttrTbl),
                                             GATT_MAX_ENCRYPT_KEY_SIZE,
                                             &simpleProfileCBs);
    }

    return (status);
}

//注册服务函数
bStatus_t Gnss_Addservice(uint32_t services)
{
    uint8_t status = SUCCESS;

    // 注册notify特征
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, gnssProfileChar1Config);
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, gnssProfileChar2Config);

    // 连接状态回调
    linkDB_Register(gnssProfile_HandleConnStatusCB);

    if(services & GNSSPROFILE_SERVICE)
    {
        status = GATTServApp_RegisterService(gnssProfileAttrTb,             //属性表
                                        GATT_NUM_ATTRS(gnssProfileAttrTb),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &gnssProfileCBs);                    //读写回调函数有数据读写时调到读写函数中
    }
    return (status);
}

//注册服务函数
bStatus_t Rdss_Addservice(uint32_t services)
{
    uint8_t status = SUCCESS;

    // 注册notify特征
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, rdssProfileChar2Config);
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, rdssProfileChar3Config);
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, rdssProfileChar5Config);
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, rdssProfileChar6Config);
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, rdssProfileChar7Config);

    // 连接状态回调
    linkDB_Register(rdssProfile_HandleConnStatusCB);

    if(services & RDSSPROFILE_SERVICE)
    {
        status = GATTServApp_RegisterService(rdssProfileAttrTb,             //属性表
                                        GATT_NUM_ATTRS(rdssProfileAttrTb),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &rdssProfileCBs);                    //读写回调函数有数据读写时调到读写函数中
    }
    return (status);
}

/*********************************************************************
 * @fn      SimpleProfile_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t SimpleProfile_RegisterAppCBs(simpleProfileCBs_t *appCallbacks)
{
    if(appCallbacks)
    {
        simpleProfile_AppCBs = appCallbacks;

        return (SUCCESS);
    }
    else
    {
        return (bleAlreadyInRequestedMode);
    }
}

bStatus_t BatteryProfile_RegisterAppCBs(batteryProfileCBs_t *appCallbacks)
{
    if(appCallbacks)
    {
        batteryProfile_AppCBs = appCallbacks;

        return (SUCCESS);
    }
    else
    {
        return (bleAlreadyInRequestedMode);
    }
}

bStatus_t ControlProfile_RegisterAppCBs(controlProfileCBs_t *appCallbacks)
{
    if(appCallbacks)
    {
        controlProfile_AppCBs = appCallbacks;

        return (SUCCESS);
    }
    else
    {
        return (bleAlreadyInRequestedMode);
    }
}

bStatus_t GnssProfile_RegisterAppCBs(gnssProfileCBs_t *appCallbacks)
{
    if(appCallbacks)
    {
        gnssProfile_AppCBs = appCallbacks;

        return (SUCCESS);
    }
    else
    {
        return (bleAlreadyInRequestedMode);
    }
}

bStatus_t RdssProfile_RegisterAppCBs(rdssProfileCBs_t *appCallbacks)
{
    if(appCallbacks)
    {
        rdssProfile_AppCBs = appCallbacks;

        return (SUCCESS);
    }
    else
    {
        return (bleAlreadyInRequestedMode);
    }
}
/*********************************************************************
 * @fn      SimpleProfile_SetParameter
 *
 * @brief   Set a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to right
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16_t will be cast to
 *          uint16_t pointer).
 *
 * @return  bStatus_t
 */
bStatus_t SimpleProfile_SetParameter(uint8_t param, uint8_t len, void *value)
{
    bStatus_t ret = SUCCESS;
    switch(param)
    {
        case SIMPLEPROFILE_CHAR1:
            if(len == SIMPLEPROFILE_CHAR1_LEN)
            {
                tmos_memcpy(simpleProfileChar1, value, SIMPLEPROFILE_CHAR1_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        case SIMPLEPROFILE_CHAR2:
            if(len == SIMPLEPROFILE_CHAR2_LEN)
            {
                tmos_memcpy(simpleProfileChar2, value, SIMPLEPROFILE_CHAR2_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        case SIMPLEPROFILE_CHAR3:
            if(len == SIMPLEPROFILE_CHAR3_LEN)
            {
                tmos_memcpy(simpleProfileChar3, value, SIMPLEPROFILE_CHAR3_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        case SIMPLEPROFILE_CHAR4:
            if(len == SIMPLEPROFILE_CHAR4_LEN)
            {
                tmos_memcpy(simpleProfileChar4, value, SIMPLEPROFILE_CHAR4_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        case SIMPLEPROFILE_CHAR5:
            if(len == SIMPLEPROFILE_CHAR5_LEN)
            {
                tmos_memcpy(simpleProfileChar5, value, SIMPLEPROFILE_CHAR5_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return (ret);
}

bStatus_t BatteryProfile_SetParameter(uint8_t param, uint8_t len, void *value)
{
    bStatus_t ret = SUCCESS;
    switch(param)
    {
        case BATTERYPROFILE_CHAR:
            if(len == BATTERYPROFILE_CHAR_LEN)
            {
                tmos_memcpy(batteryProfileChar, value, BATTERYPROFILE_CHAR_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return (ret);
}

bStatus_t ControlProfile_SetParameter(uint8_t param, uint8_t len, void *value)
{
    bStatus_t ret = SUCCESS;
    switch(param)
    {
        case CONTROLPROFILE_CHAR1:
            if(len == CONTROLPROFILE_CHAR1_LEN)
            {
                tmos_memcpy(controlProfileChar1, value, CONTROLPROFILE_CHAR1_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        case CONTROLPROFILE_CHAR2:
            if(len == CONTROLPROFILE_CHAR2_LEN)
            {
                tmos_memcpy(controlProfileChar2, value, CONTROLPROFILE_CHAR2_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        case CONTROLPROFILE_CHAR3:
            if(len == CONTROLPROFILE_CHAR3_LEN)
            {
                tmos_memcpy(controlProfileChar3, value, CONTROLPROFILE_CHAR3_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        case CONTROLPROFILE_CHAR4:
            if(len == CONTROLPROFILE_CHAR4_LEN)
            {
                tmos_memcpy(controlProfileChar4, value, CONTROLPROFILE_CHAR4_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        case CONTROLPROFILE_CHAR5:
            if(len == CONTROLPROFILE_CHAR5_LEN)
            {
                tmos_memcpy(controlProfileChar5, value, CONTROLPROFILE_CHAR5_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        case CONTROLPROFILE_CHAR6:
            if(len == CONTROLPROFILE_CHAR6_LEN)
            {
                tmos_memcpy(controlProfileChar6, value, CONTROLPROFILE_CHAR6_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        case CONTROLPROFILE_CHAR7:
            if(len == CONTROLPROFILE_CHAR7_LEN)
            {
                tmos_memcpy(controlProfileChar7, value, CONTROLPROFILE_CHAR7_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return (ret);
}

bStatus_t GnssProfile_SetParameter(uint8_t param, uint8_t len, void *value)
{
    bStatus_t ret = SUCCESS;
    switch(param)
    {
        case GNSSPROFILE_CHAR1:
            if(len == GNSSPROFILE_CHAR1_LEN)
            {
                tmos_memcpy(gnssProfileChar1, value, GNSSPROFILE_CHAR1_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;
        case GNSSPROFILE_CHAR2:
            if(len == GNSSPROFILE_CHAR2_LEN)
            {
                tmos_memcpy(gnssProfileChar2, value, GNSSPROFILE_CHAR2_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return (ret);
}

bStatus_t RdssProfile_SetParameter(uint8_t param, uint8_t len, void *value)
{
    bStatus_t ret = SUCCESS;
    switch(param)
    {
        case RDSSPROFILE_CHAR1:
            if(len == RDSSPROFILE_CHAR1_LEN)
            {
                tmos_memcpy(rdssProfileChar1, value, RDSSPROFILE_CHAR1_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;
        case RDSSPROFILE_CHAR2:
            if(len == RDSSPROFILE_CHAR2_LEN)
            {
                tmos_memcpy(rdssProfileChar2, value, RDSSPROFILE_CHAR2_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;
        case RDSSPROFILE_CHAR3:
            if(len == RDSSPROFILE_CHAR3_LEN)
            {
                tmos_memcpy(rdssProfileChar3, value, RDSSPROFILE_CHAR3_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;
        case RDSSPROFILE_CHAR4:
            if(len == RDSSPROFILE_CHAR4_LEN)
            {
                tmos_memcpy(rdssProfileChar4, value, RDSSPROFILE_CHAR4_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;
        case RDSSPROFILE_CHAR5:
            if(len == RDSSPROFILE_CHAR5_LEN)
            {
                tmos_memcpy(rdssProfileChar5, value, RDSSPROFILE_CHAR5_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;
        case RDSSPROFILE_CHAR6:
            if(len == RDSSPROFILE_CHAR6_LEN)
            {
                tmos_memcpy(rdssProfileChar6, value, RDSSPROFILE_CHAR6_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;
        case RDSSPROFILE_CHAR7:
            if(len == RDSSPROFILE_CHAR7_LEN)
            {
                tmos_memcpy(rdssProfileChar7, value, RDSSPROFILE_CHAR7_LEN);
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;

        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return (ret);
}
/*********************************************************************
 * @fn      SimpleProfile_GetParameter
 *
 * @brief   Get a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16_t will be cast to
 *          uint16_t pointer).
 *
 * @return  bStatus_t
 */
bStatus_t SimpleProfile_GetParameter(uint8_t param, void *value)
{
    bStatus_t ret = SUCCESS;
    switch(param)
    {
        case SIMPLEPROFILE_CHAR1:
            tmos_memcpy(value, simpleProfileChar1, SIMPLEPROFILE_CHAR1_LEN);
            break;

        case SIMPLEPROFILE_CHAR2:
            tmos_memcpy(value, simpleProfileChar2, SIMPLEPROFILE_CHAR2_LEN);
            break;

        case SIMPLEPROFILE_CHAR3:
            tmos_memcpy(value, simpleProfileChar3, SIMPLEPROFILE_CHAR3_LEN);
            break;

        case SIMPLEPROFILE_CHAR4:
            tmos_memcpy(value, simpleProfileChar4, SIMPLEPROFILE_CHAR4_LEN);
            break;

        case SIMPLEPROFILE_CHAR5:
            tmos_memcpy(value, simpleProfileChar5, SIMPLEPROFILE_CHAR5_LEN);
            break;

        default:
            ret = INVALIDPARAMETER;
            break;
    }

    return (ret);
}

/*********************************************************************
 * @fn          simpleProfile_Notify
 *
 * @brief       Send a notification containing a heart rate
 *              measurement.
 *
 * @param       connHandle - connection handle
 * @param       pNoti - pointer to notification structure
 *
 * @return      Success or Failure
 */
bStatus_t simpleProfile_Notify(uint16_t connHandle, attHandleValueNoti_t *pNoti)
{
    uint16_t value = GATTServApp_ReadCharCfg(connHandle, simpleProfileChar4Config);

    // If notifications enabled
    if(value & GATT_CLIENT_CFG_NOTIFY)
    {
        // Set the handle
        pNoti->handle = simpleProfileAttrTbl[SIMPLEPROFILE_CHAR4_VALUE_POS].handle;

        // Send the notification
        return GATT_Notification(connHandle, pNoti, FALSE);
    }
    return bleIncorrectMode;
}

//底层数据发送接口
bStatus_t batteryProfile_Notify(uint16_t connHandle, attHandleValueNoti_t *pNoti)
{
    uint16_t value = GATTServApp_ReadCharCfg(connHandle, batteryProfileCharConfig);

    // If notifications enabled
    if(value & GATT_CLIENT_CFG_NOTIFY)
    {
        // Set the handle
        pNoti->handle = batteryProfileAttrTb[BATTERYPROFILE_CHAR_VALUE_POS].handle;

        // Send the notification
        return GATT_Notification(connHandle, pNoti, FALSE);
    }
    return bleIncorrectMode;
}

//底层数据发送接口
bStatus_t gnssProfile_Notify1(uint16_t connHandle, attHandleValueNoti_t *pNoti)
{
    uint16_t value = GATTServApp_ReadCharCfg(connHandle, gnssProfileChar1Config);

    // If notifications enabled
    if(value & GATT_CLIENT_CFG_NOTIFY)
    {
        // Set the handle
        pNoti->handle = gnssProfileAttrTb[GNSSPROFILE_CHAR1_VALUE_POS].handle;

        // Send the notification
        return GATT_Notification(connHandle, pNoti, FALSE);
    }
    return bleIncorrectMode;
}
bStatus_t gnssProfile_Notify2(uint16_t connHandle, attHandleValueNoti_t *pNoti)
{
    uint16_t value = GATTServApp_ReadCharCfg(connHandle, gnssProfileChar2Config);

    // If notifications enabled
    if(value & GATT_CLIENT_CFG_NOTIFY)
    {
        // Set the handle
        pNoti->handle = gnssProfileAttrTb[GNSSPROFILE_CHAR2_VALUE_POS].handle;

        // Send the notification
        return GATT_Notification(connHandle, pNoti, FALSE);
    }
    return bleIncorrectMode;
}

// RDSS notify functions
bStatus_t rdssProfile_Notify2(uint16_t connHandle, attHandleValueNoti_t *pNoti)
{
    uint16_t value = GATTServApp_ReadCharCfg(connHandle, rdssProfileChar2Config);

    if(value & GATT_CLIENT_CFG_NOTIFY)
    {
        pNoti->handle = rdssProfileAttrTb[RDSSPROFILE_CHAR2_VALUE_POS].handle;
        return GATT_Notification(connHandle, pNoti, FALSE);
    }
    return bleIncorrectMode;
}

bStatus_t rdssProfile_Notify3(uint16_t connHandle, attHandleValueNoti_t *pNoti)
{
    uint16_t value = GATTServApp_ReadCharCfg(connHandle, rdssProfileChar3Config);

    if(value & GATT_CLIENT_CFG_NOTIFY)
    {
        pNoti->handle = rdssProfileAttrTb[RDSSPROFILE_CHAR3_VALUE_POS].handle;
        return GATT_Notification(connHandle, pNoti, FALSE);
    }
    return bleIncorrectMode;
}

bStatus_t rdssProfile_Notify5(uint16_t connHandle, attHandleValueNoti_t *pNoti)
{
    uint16_t value = GATTServApp_ReadCharCfg(connHandle, rdssProfileChar5Config);

    if(value & GATT_CLIENT_CFG_NOTIFY)
    {
        pNoti->handle = rdssProfileAttrTb[RDSSPROFILE_CHAR5_VALUE_POS].handle;
        return GATT_Notification(connHandle, pNoti, FALSE);
    }
    return bleIncorrectMode;
}

bStatus_t rdssProfile_Notify6(uint16_t connHandle, attHandleValueNoti_t *pNoti)
{
    uint16_t value = GATTServApp_ReadCharCfg(connHandle, rdssProfileChar6Config);

    if(value & GATT_CLIENT_CFG_NOTIFY)
    {
        pNoti->handle = rdssProfileAttrTb[RDSSPROFILE_CHAR6_VALUE_POS].handle;
        return GATT_Notification(connHandle, pNoti, FALSE);
    }
    return bleIncorrectMode;
}

bStatus_t rdssProfile_Notify7(uint16_t connHandle, attHandleValueNoti_t *pNoti)
{
    uint16_t value = GATTServApp_ReadCharCfg(connHandle, rdssProfileChar7Config);

    if(value & GATT_CLIENT_CFG_NOTIFY)
    {
        pNoti->handle = rdssProfileAttrTb[RDSSPROFILE_CHAR7_VALUE_POS].handle;
        return GATT_Notification(connHandle, pNoti, FALSE);
    }
    return bleIncorrectMode;
}
/*********************************************************************
 * @fn          simpleProfile_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 *
 * @return      Success or Failure
 */
static bStatus_t simpleProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method)
{
    bStatus_t status = SUCCESS;


    // Make sure it's not a blob operation (no attributes in the profile are long)
    if(offset > 0)
    {
        return (ATT_ERR_ATTR_NOT_LONG);
    }

    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        switch(uuid)
        {
            // No need for "GATT_SERVICE_UUID" or "GATT_CLIENT_CHAR_CFG_UUID" cases;
            // gattserverapp handles those reads

            // characteristics 1 and 2 have read permissions
            // characteritisc 3 does not have read permissions; therefore it is not
            //   included here
            // characteristic 4 does not have read permissions, but because it
            //   can be sent as a notification, it is included here
            case SIMPLEPROFILE_CHAR1_UUID:
                if(maxLen > SIMPLEPROFILE_CHAR1_LEN)
                {
                    *pLen = SIMPLEPROFILE_CHAR1_LEN;
                }
                else
                {
                    *pLen = maxLen;
                }
                tmos_memcpy(pValue, pAttr->pValue, *pLen);
                break;

            case SIMPLEPROFILE_CHAR2_UUID:
                if(maxLen > SIMPLEPROFILE_CHAR2_LEN)
                {
                    *pLen = SIMPLEPROFILE_CHAR2_LEN;
                }
                else
                {
                    *pLen = maxLen;
                }
                tmos_memcpy(pValue, pAttr->pValue, *pLen);
                break;

            case SIMPLEPROFILE_CHAR4_UUID:
                if(maxLen > SIMPLEPROFILE_CHAR4_LEN)
                {
                    *pLen = SIMPLEPROFILE_CHAR4_LEN;
                }
                else
                {
                    *pLen = maxLen;
                }
                tmos_memcpy(pValue, pAttr->pValue, *pLen);
                break;

            case SIMPLEPROFILE_CHAR5_UUID:
                if(maxLen > SIMPLEPROFILE_CHAR5_LEN)
                {
                    *pLen = SIMPLEPROFILE_CHAR5_LEN;
                }
                else
                {
                    *pLen = maxLen;
                }
                tmos_memcpy(pValue, pAttr->pValue, *pLen);
                break;

            default:
                // Should never get here! (characteristics 3 and 4 do not have read permissions)
                *pLen = 0;
                status = ATT_ERR_ATTR_NOT_FOUND;
                break;
        }
    }
    else
    {
        // 128-bit UUID
        *pLen = 0;
        status = ATT_ERR_INVALID_HANDLE;
    }

    return (status);
}

/*********************************************************************
 * @fn      simpleProfile_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 *
 * @return  Success or Failure
 */
static bStatus_t simpleProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method)
{
    bStatus_t status = SUCCESS;
    uint8_t   notifyApp = 0xFF;

    // If attribute permissions require authorization to write, return error
    if(gattPermitAuthorWrite(pAttr->permissions))
    {
        // Insufficient authorization
        return (ATT_ERR_INSUFFICIENT_AUTHOR);
    }

    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        switch(uuid)
        {
            case SIMPLEPROFILE_CHAR1_UUID:
                //Validate the value
                // Make sure it's not a blob oper
                if(offset == 0)
                {
                    if(len > SIMPLEPROFILE_CHAR1_LEN)
                    {
                        status = ATT_ERR_INVALID_VALUE_SIZE;
                    }
                }
                else
                {
                    status = ATT_ERR_ATTR_NOT_LONG;
                }

                //Write the value
                if(status == SUCCESS)
                {
                    tmos_memcpy(pAttr->pValue, pValue, SIMPLEPROFILE_CHAR1_LEN);
                    notifyApp = SIMPLEPROFILE_CHAR1;
                }
                break;

            case SIMPLEPROFILE_CHAR3_UUID:
                //Validate the value
                // Make sure it's not a blob oper
                if(offset == 0)
                {
                    if(len > SIMPLEPROFILE_CHAR3_LEN)
                    {
                        status = ATT_ERR_INVALID_VALUE_SIZE;
                    }
                }
                else
                {
                    status = ATT_ERR_ATTR_NOT_LONG;
                }

                //Write the value
                if(status == SUCCESS)
                {
                    tmos_memcpy(pAttr->pValue, pValue, SIMPLEPROFILE_CHAR3_LEN);
                    notifyApp = SIMPLEPROFILE_CHAR3;
                }
                break;

            case GATT_CLIENT_CHAR_CFG_UUID:
                status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len,
                                                        offset, GATT_CLIENT_CFG_NOTIFY);
                break;

            default:
                // Should never get here! (characteristics 2 and 4 do not have write permissions)
                status = ATT_ERR_ATTR_NOT_FOUND;
                break;
        }
    }
    else
    {
        // 128-bit UUID
        status = ATT_ERR_INVALID_HANDLE;
    }

    // If a charactersitic value changed then callback function to notify application of change
    if((notifyApp != 0xFF) && simpleProfile_AppCBs && simpleProfile_AppCBs->pfnSimpleProfileChange)
    {
        simpleProfile_AppCBs->pfnSimpleProfileChange(notifyApp, pValue, len);
    }

    return (status);
}

/*********************************************************************
 * @fn          simpleProfile_HandleConnStatusCB
 *
 * @brief       Simple Profile link status change handler function.
 *
 * @param       connHandle - connection handle
 * @param       changeType - type of change
 *
 * @return      none
 */
static void simpleProfile_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType)
{
    // Make sure this is not loopback connection
    if(connHandle != LOOPBACK_CONNHANDLE)
    {
        // Reset Client Char Config if connection has dropped
        if((changeType == LINKDB_STATUS_UPDATE_REMOVED) ||
           ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS) &&
            (!linkDB_Up(connHandle))))
        {
            GATTServApp_InitCharCfg(connHandle, simpleProfileChar4Config);
        }
    }
}

//连接状态回调
static void batteryProfile_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType)
{
    // Make sure this is not loopback connection
    if(connHandle != LOOPBACK_CONNHANDLE)
    {
        // Reset Client Char Config if connection has dropped
        if((changeType == LINKDB_STATUS_UPDATE_REMOVED) ||
           ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS) &&
            (!linkDB_Up(connHandle))))
        {
            GATTServApp_InitCharCfg(connHandle, batteryProfileCharConfig);
        }
    }
}

//连接状态回调
static void gnssProfile_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType)
{
    // Make sure this is not loopback connection
    if(connHandle != LOOPBACK_CONNHANDLE)
    {
        // Reset Client Char Config if connection has dropped
        if((changeType == LINKDB_STATUS_UPDATE_REMOVED) ||
           ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS) &&
            (!linkDB_Up(connHandle))))
        {
            GATTServApp_InitCharCfg(connHandle, gnssProfileChar1Config);
            GATTServApp_InitCharCfg(connHandle, gnssProfileChar2Config);
        }
    }
}

//连接状态回调
static void rdssProfile_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType)
{
    // Make sure this is not loopback connection
    if(connHandle != LOOPBACK_CONNHANDLE)
    {
        // Reset Client Char Config if connection has dropped
        if((changeType == LINKDB_STATUS_UPDATE_REMOVED) ||
           ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS) &&
            (!linkDB_Up(connHandle))))
        {
            GATTServApp_InitCharCfg(connHandle, rdssProfileChar2Config);
            GATTServApp_InitCharCfg(connHandle, rdssProfileChar3Config);
            GATTServApp_InitCharCfg(connHandle, rdssProfileChar5Config);
            GATTServApp_InitCharCfg(connHandle, rdssProfileChar6Config);
            GATTServApp_InitCharCfg(connHandle, rdssProfileChar7Config);
        }
    }
}

//读写回调
static bStatus_t batteryProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method)
{
    bStatus_t status =SUCCESS;
    static uint8_t read_data[BATTERYPROFILE_CHAR_LEN] = {0};

    if(gattPermitAuthenRead(pAttr->permissions))
    {
        return(ATT_ERR_INSUFFICIENT_AUTHOR);
    }
    if( offset > 0)
    {
        return(ATT_ERR_ATTR_NOT_LONG);
    }
    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0],pAttr->type.uuid[1]);

        switch(uuid)
        {
        case BATT_LEVEL_UUID:
                if(maxLen > BATTERYPROFILE_CHAR_LEN)
                {
                    *pLen = BATTERYPROFILE_CHAR_LEN;
                }
                else
                {
                    *pLen = maxLen;
                }
                read_data[0] = Bat_percentage;

                tmos_memcpy(pValue, read_data, *pLen);

        break;

        default:
            *pLen = 0;
            status = ATT_ERR_ATTR_NOT_FOUND;
        break;
        }
    }
    else {
        *pLen = 0;
        status = ATT_ERR_INVALID_HANDLE;
    }
    return(status);
}

static bStatus_t batteryProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method)
{
    bStatus_t status = SUCCESS;
    uint8_t   notifyApp = 0xFF;

    // If attribute permissions require authorization to write, return error
    if(gattPermitAuthorWrite(pAttr->permissions))
    {
        // Insufficient authorization
        return (ATT_ERR_INSUFFICIENT_AUTHOR);
    }

    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        switch(uuid)
        {
             case GATT_CLIENT_CHAR_CFG_UUID:
                status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len,
                                                        offset, GATT_CLIENT_CFG_NOTIFY);
                break;

            default:
                // Should never get here! (characteristics 2 and 4 do not have write permissions)
                status = ATT_ERR_ATTR_NOT_FOUND;
                break;
        }
    }
    else
    {
        // 128-bit UUID
        status = ATT_ERR_INVALID_HANDLE;
    }

    // If a charactersitic value changed then callback function to notify application of change
    if((notifyApp != 0xFF) && batteryProfile_AppCBs && batteryProfile_AppCBs->pfnBatteryProfileChange)
    {
        batteryProfile_AppCBs->pfnBatteryProfileChange(notifyApp, pValue, len);
    }

    return (status);
}


//读写回调
static bStatus_t controlProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method)
{
    bStatus_t status =SUCCESS;

    if(gattPermitAuthenRead(pAttr->permissions))
    {
        return(ATT_ERR_INSUFFICIENT_AUTHOR);
    }
    if( offset > 0)
    {
        return(ATT_ERR_ATTR_NOT_LONG);
    }
    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0],pAttr->type.uuid[1]);
        switch(uuid)
        {
        case CONTROLPROFILE_CHAR1_UUID://GNSS开关
             *pLen = CONTROLPROFILE_CHAR1_LEN;
             static uint8_t read_data1[CONTROLPROFILE_CHAR1_LEN] = {0};
             read_data1[0] = RN_SW_Flag;
             tmos_memcpy(pValue, read_data1, *pLen);

        break;
        case CONTROLPROFILE_CHAR2_UUID://RDSS开关
             *pLen = CONTROLPROFILE_CHAR2_LEN;
             static uint8_t read_data2[CONTROLPROFILE_CHAR2_LEN] = {0};
             read_data2[0] = RD_SW_Flag;
             tmos_memcpy(pValue, read_data2, *pLen);

        break;
        case CONTROLPROFILE_CHAR3_UUID://SOS开关
             *pLen = CONTROLPROFILE_CHAR3_LEN;
             static uint8_t read_data3[CONTROLPROFILE_CHAR3_LEN] = {0};
             read_data3[0] = SOS_SW_Flag;
             tmos_memcpy(pValue, read_data3, *pLen);

        break;
        case CONTROLPROFILE_CHAR4_UUID://RDSS过检开关
             *pLen = CONTROLPROFILE_CHAR4_LEN;
             static uint8_t read_data4[CONTROLPROFILE_CHAR4_LEN] = {0};
             read_data4[0] = RD_TEST_Flag;
             tmos_memcpy(pValue, read_data4, *pLen);

        break;
        case CONTROLPROFILE_CHAR6_UUID://Location timer
             *pLen = CONTROLPROFILE_CHAR6_LEN;
             tmos_memcpy(pValue, controlProfileChar6, *pLen);

        break;
        default:
            *pLen = 0;
            status = ATT_ERR_ATTR_NOT_FOUND;
        break;
        }
    }
    else {
        *pLen = 0;
        status = ATT_ERR_INVALID_HANDLE;
    }
    return(status);
}

static bStatus_t controlProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method)
{
    bStatus_t status =SUCCESS;
    uint8_t notifyApp = 0xFF;

    if(gattPermitAuthenWrite(pAttr->permissions))
    {
        return(ATT_ERR_INSUFFICIENT_AUTHOR);
    }

    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0],pAttr->type.uuid[1]);
        switch(uuid)
        {
        case CONTROLPROFILE_CHAR1_UUID://GNSS开关
            if(offset == 0)
            {
                if(len > CONTROLPROFILE_CHAR1_LEN)
                {
                    status = ATT_ERR_INVALID_VALUE_SIZE;
                }
            }
            else {
                status = ATT_ERR_ATTR_NOT_LONG;
            }

            if(status == SUCCESS) //Write the value
            {
                tmos_memcpy(pAttr->pValue, pValue, CONTROLPROFILE_CHAR1_LEN);
                if(pValue[0] == 0x00)
                    RN_SW_Flag = FALSE;
                else
                    RN_SW_Flag = TRUE;
                notifyApp = CONTROLPROFILE_CHAR1;
            }

        break;
        case CONTROLPROFILE_CHAR2_UUID://RDSS开关
            if(offset == 0)
            {
                if(len > CONTROLPROFILE_CHAR2_LEN)
                {
                    status = ATT_ERR_INVALID_VALUE_SIZE;
                }
            }
            else {
                status = ATT_ERR_ATTR_NOT_LONG;
            }

            if(status == SUCCESS) //Write the value
            {
                tmos_memcpy(pAttr->pValue, pValue, CONTROLPROFILE_CHAR2_LEN);
                if(pValue[0] == 0x00)
                    RD_SW_Flag = FALSE;
                else
                    RD_SW_Flag = TRUE;
                notifyApp = CONTROLPROFILE_CHAR2;
            }

        break;
        case CONTROLPROFILE_CHAR3_UUID://SOS开关
            if(offset == 0)
            {
                if(len > CONTROLPROFILE_CHAR3_LEN)
                {
                    status = ATT_ERR_INVALID_VALUE_SIZE;
                }
            }
            else {
                status = ATT_ERR_ATTR_NOT_LONG;
            }

            if(status == SUCCESS) //Write the value
            {
                tmos_memcpy(pAttr->pValue, pValue, CONTROLPROFILE_CHAR3_LEN);
                if(pValue[0] == 0x00)
                    SOS_SW_Flag = FALSE;
                else
                    SOS_SW_Flag = TRUE;
                notifyApp = CONTROLPROFILE_CHAR3;
            }

        break;
        case CONTROLPROFILE_CHAR4_UUID://RDSS过检开关
            if(offset == 0)
            {
                if(len > CONTROLPROFILE_CHAR4_LEN)
                {
                    status = ATT_ERR_INVALID_VALUE_SIZE;
                }
            }
            else {
                status = ATT_ERR_ATTR_NOT_LONG;
            }

            if(status == SUCCESS) //Write the value
            {
                tmos_memcpy(pAttr->pValue, pValue, CONTROLPROFILE_CHAR4_LEN);
                if(pValue[0] == 0x00)
                    RD_TEST_Flag = FALSE;
                else
                    RD_TEST_Flag = TRUE;
                notifyApp = CONTROLPROFILE_CHAR4;
            }

        break;
        case CONTROLPROFILE_CHAR5_UUID://RDSS复位开关
            if(offset == 0)
            {
                if(len > CONTROLPROFILE_CHAR5_LEN)
                {
                    status = ATT_ERR_INVALID_VALUE_SIZE;
                }
            }
            else {
                status = ATT_ERR_ATTR_NOT_LONG;
            }

            if(status == SUCCESS) //Write the value
            {
                tmos_memcpy(pAttr->pValue, pValue, CONTROLPROFILE_CHAR5_LEN);
                if(pValue[0] == 0x00)
                    RD_RST_Flag = FALSE;
                else
                    RD_RST_Flag = TRUE;
                notifyApp = CONTROLPROFILE_CHAR5;
            }

        break;
        case CONTROLPROFILE_CHAR6_UUID://Location timer
            if(offset == 0)
            {
                if(len != CONTROLPROFILE_CHAR6_LEN)
                {
                    status = ATT_ERR_INVALID_VALUE_SIZE;
                }
            }
            else {
                status = ATT_ERR_ATTR_NOT_LONG;
            }

            if(status == SUCCESS) //Write the value
            {
                tmos_memcpy(pAttr->pValue, pValue, CONTROLPROFILE_CHAR6_LEN);
                notifyApp = CONTROLPROFILE_CHAR6;
            }

        break;
        case CONTROLPROFILE_CHAR7_UUID://Location send
            if(offset == 0)
            {
                if(len != CONTROLPROFILE_CHAR7_LEN)
                {
                    status = ATT_ERR_INVALID_VALUE_SIZE;
                }
            }
            else {
                status = ATT_ERR_ATTR_NOT_LONG;
            }

            if(status == SUCCESS) //Write the value
            {
                tmos_memcpy(pAttr->pValue, pValue, CONTROLPROFILE_CHAR7_LEN);
                notifyApp = CONTROLPROFILE_CHAR7;
            }

        break;
        default:

            status = ATT_ERR_ATTR_NOT_FOUND;
            break;
        }
    }
    else {
        status = ATT_ERR_INVALID_HANDLE;
    }
    // If a charactersitic value changed then callback function to notify application of change
    if ( (notifyApp != 0xFF ) && controlProfile_AppCBs && controlProfile_AppCBs->pfncontrolProfileChange )
      {
        controlProfile_AppCBs->pfncontrolProfileChange( notifyApp, pValue, len );
      }
      return ( status );
}


//读写回调
static bStatus_t cardProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method)
{
    bStatus_t status =SUCCESS;
    static uint8_t read_data[CARDPROFILE_CHAR_LEN] = {0};
    unsigned int temp_id = 0;

    if(gattPermitAuthenRead(pAttr->permissions))
    {
        return(ATT_ERR_INSUFFICIENT_AUTHOR);
    }
    if( offset > 0)
    {
        return(ATT_ERR_ATTR_NOT_LONG);
    }
    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0],pAttr->type.uuid[1]);
        switch(uuid)
        {
        case CARDPROFILE_CHAR_UUID:
             *pLen = CARDPROFILE_CHAR_LEN;

             temp_id = atoi(DestIC); //把字符串格式化为整数
             read_data[0] =(uint8_t) (temp_id  & 0x000000FF);
             read_data[1] =(uint8_t) ((temp_id & 0x0000FF00)>>8);
             read_data[2] =(uint8_t) ((temp_id & 0x00FF0000)>>16);
             read_data[3] =(uint8_t) ((temp_id & 0xFF000000)>>24);

             tmos_memcpy(pValue, read_data, *pLen);
        break;
        default:
            *pLen = 0;
            status = ATT_ERR_ATTR_NOT_FOUND;
        break;
        }
    }
    else {
        *pLen = 0;
        status = ATT_ERR_INVALID_HANDLE;
    }
    return(status);
}

static bStatus_t cardProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method)
{
    bStatus_t status =SUCCESS;
    uint8_t notifyApp = 0xFF;
    uint32_t temp_id = 0;
    uint8_t i = 0;

    if(gattPermitAuthenWrite(pAttr->permissions))
    {
        return(ATT_ERR_INSUFFICIENT_AUTHOR);
    }

    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0],pAttr->type.uuid[1]);
        switch(uuid)
        {
        case CARDPROFILE_CHAR_UUID:
            if(offset == 0)
            {
                if((len == 0) || (len > 10))
                {
                    status = ATT_ERR_INVALID_VALUE_SIZE;
                    PROFILE_DEBUG_PRINTF("\r\n[CARD 4101 WRITE FAIL] len=%d\r\n", len);
                }
            }
            else {
                status = ATT_ERR_ATTR_NOT_LONG;
                PROFILE_DEBUG_PRINTF("\r\n[CARD 4101 WRITE FAIL] offset=%d not supported\r\n", offset);
            }

            if(status == SUCCESS)
            {
                uint8_t ascii_card = TRUE;
                uint32_t le_id = 0;
                uint32_t be_id = 0;

                PROFILE_DEBUG_PRINTF("\r\n[CARD 4101 WRITE] len=%d raw=", len);
                for(i = 0; i < len; i++)
                {
                    PROFILE_DEBUG_PRINTF("%02X ", pValue[i]);
                    if((pValue[i] < '0') || (pValue[i] > '9'))
                    {
                        ascii_card = FALSE;
                    }
                }
                PROFILE_DEBUG_PRINTF("\r\n");

                if(ascii_card)
                {
                    for(i = 0; i < len; i++)
                    {
                        temp_id = temp_id * 10 + (pValue[i] - '0');
                    }
                    PROFILE_DEBUG_PRINTF("[CARD 4101 WRITE] parse ascii card=%lu\r\n", (unsigned long)temp_id);
                }
                else if(len == CARDPROFILE_CHAR_LEN)
                {
                    le_id = ((uint32_t)pValue[0])
                          | (((uint32_t)pValue[1]) << 8)
                          | (((uint32_t)pValue[2]) << 16)
                          | (((uint32_t)pValue[3]) << 24);
                    be_id = ((uint32_t)pValue[3])
                          | (((uint32_t)pValue[2]) << 8)
                          | (((uint32_t)pValue[1]) << 16)
                          | (((uint32_t)pValue[0]) << 24);

                    if((le_id > 99999999UL) && (be_id <= 99999999UL))
                    {
                        temp_id = be_id;
                        PROFILE_DEBUG_PRINTF("[CARD 4101 WRITE] parse big-endian card=%lu\r\n", (unsigned long)temp_id);
                    }
                    else
                    {
                        temp_id = le_id;
                        PROFILE_DEBUG_PRINTF("[CARD 4101 WRITE] parse little-endian card=%lu\r\n", (unsigned long)temp_id);
                    }
                }
                else
                {
                    status = ATT_ERR_INVALID_VALUE_SIZE;
                    PROFILE_DEBUG_PRINTF("[CARD 4101 WRITE FAIL] non-ascii len=%d, expect 4 bytes\r\n", len);
                }

                if(status == SUCCESS)
                {
                    pAttr->pValue[0] = (uint8_t)(temp_id & 0x000000FF);
                    pAttr->pValue[1] = (uint8_t)((temp_id & 0x0000FF00) >> 8);
                    pAttr->pValue[2] = (uint8_t)((temp_id & 0x00FF0000) >> 16);
                    pAttr->pValue[3] = (uint8_t)((temp_id & 0xFF000000) >> 24);
                    sprintf(DestIC, "%lu", (unsigned long)temp_id);
                    PROFILE_DEBUG_PRINTF("[CARD 4101 WRITE OK] DestIC=%s\r\n", DestIC);
                    notifyApp = CARDPROFILE_CHAR;
                }
            }

        break;
        default:

            status = ATT_ERR_ATTR_NOT_FOUND;
            break;
        }
    }
    else {
        status = ATT_ERR_INVALID_HANDLE;
    }
    // If a charactersitic value changed then callback function to notify application of change
    if ( (notifyApp != 0xFF ) && cardProfile_AppCBs && cardProfile_AppCBs->pfncardProfileChange )
      {
        cardProfile_AppCBs->pfncardProfileChange( notifyApp, pAttr->pValue, CARDPROFILE_CHAR_LEN );
      }
      return ( status );
}

void float_to_bytes_memcpy(float value, uint8_t* bytes) {
    // 先将float转换为uint32_t
    uint32_t temp;
    memcpy(&temp, &value, sizeof(float));

    // 转换为大端序
    bytes[0] = (temp >> 24) & 0xFF;
    bytes[1] = (temp >> 16) & 0xFF;
    bytes[2] = (temp >> 8) & 0xFF;
    bytes[3] = temp & 0xFF;
}

//读写回调
static bStatus_t gnssProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method)
{
    bStatus_t status =SUCCESS;
    static uint8_t read_data1[GNSSPROFILE_CHAR1_LEN] = {0};
    static uint8_t read_data2[GNSSPROFILE_CHAR2_LEN] = {0};
    uint8_t temp_bytes[4] = {0};
    uint8_t i;

    if(gattPermitAuthenRead(pAttr->permissions))
    {
        return(ATT_ERR_INSUFFICIENT_AUTHOR);
    }
    if( offset > 0)
    {
        return(ATT_ERR_ATTR_NOT_LONG);
    }
    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0],pAttr->type.uuid[1]);

        switch(uuid)
        {
        case LN_FEATURE_UUID:
                if(maxLen > GNSSPROFILE_CHAR1_LEN)
                {
                    *pLen = GNSSPROFILE_CHAR1_LEN;
                }
                else
                {
                    *pLen = maxLen;
                }
                for(i=0; i<12; i++)//考虑增加排序，将最大的12个SNR数据上传
                {
                    read_data1[i] = Save_GSV_Data.satellites[i].snr;
                }

                tmos_memcpy(pValue, read_data1, *pLen);

        break;
        case LOC_SPEED_UUID:
                if(maxLen > GNSSPROFILE_CHAR2_LEN)
                {
                    *pLen = GNSSPROFILE_CHAR2_LEN;
                }
                else
                {
                    *pLen = maxLen;
                }
                //40
                read_data2[0] = GGA.hour;
                read_data2[1] = GGA.minute;
                read_data2[2] = GGA.second;
                read_data2[3] =(uint8_t) (GGA.microseconds  & 0x000000FF);
                read_data2[4] =(uint8_t) ((GGA.microseconds & 0x0000FF00)>>8);
                read_data2[5] =(uint8_t) ((GGA.microseconds & 0x00FF0000)>>16);
                read_data2[6] =(uint8_t) ((GGA.microseconds & 0xFF000000)>>24);
                float_to_bytes_memcpy(GGA.latitude, temp_bytes);
                for(i=0; i<4; i++)
                {
                    read_data2[7+i] = temp_bytes[i];
                }
                float_to_bytes_memcpy(GGA.longitude, temp_bytes);
                for(i=0; i<4; i++)
                {
                    read_data2[11+i] = temp_bytes[i];
                }
                read_data2[15] =(uint8_t) (GGA.fix_quality  & 0x000000FF);
                read_data2[16] =(uint8_t) ((GGA.fix_quality & 0x0000FF00)>>8);
                read_data2[17] =(uint8_t) ((GGA.fix_quality & 0x00FF0000)>>16);
                read_data2[18] =(uint8_t) ((GGA.fix_quality & 0xFF000000)>>24);
                read_data2[19] =(uint8_t) (GGA.satellites_tracked  & 0x000000FF);
                read_data2[20] =(uint8_t) ((GGA.satellites_tracked & 0x0000FF00)>>8);
                read_data2[21] =(uint8_t) ((GGA.satellites_tracked & 0x00FF0000)>>16);
                read_data2[22] =(uint8_t) ((GGA.satellites_tracked & 0xFF000000)>>24);
                float_to_bytes_memcpy(GGA.hdop, temp_bytes);
                for(i=0; i<4; i++)
                {
                    read_data2[23+i] = temp_bytes[i];
                }
                float_to_bytes_memcpy(GGA.altitude, temp_bytes);
                for(i=0; i<4; i++)
                {
                    read_data2[27+i] = temp_bytes[i];
                }
                float_to_bytes_memcpy(GGA.height, temp_bytes);
                for(i=0; i<4; i++)
                {
                    read_data2[31+i] = temp_bytes[i];
                }
                float_to_bytes_memcpy(GGA.dgps_age, temp_bytes);
                for(i=0; i<4; i++)
                {
                    read_data2[35+i] = temp_bytes[i];
                }

                tmos_memcpy(pValue, read_data2, *pLen);

        break;
        default:
            *pLen = 0;
            status = ATT_ERR_ATTR_NOT_FOUND;
        break;
        }
    }
    else {
        *pLen = 0;
        status = ATT_ERR_INVALID_HANDLE;
    }
    return(status);
}

static bStatus_t gnssProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method)
{
    bStatus_t status = SUCCESS;
    uint8_t   notifyApp = 0xFF;

    // If attribute permissions require authorization to write, return error
    if(gattPermitAuthorWrite(pAttr->permissions))
    {
        // Insufficient authorization
        return (ATT_ERR_INSUFFICIENT_AUTHOR);
    }

    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        switch(uuid)
        {
             case GATT_CLIENT_CHAR_CFG_UUID:
                status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len,
                                                        offset, GATT_CLIENT_CFG_NOTIFY);
                break;

            default:
                // Should never get here! (characteristics 2 and 4 do not have write permissions)
                status = ATT_ERR_ATTR_NOT_FOUND;
                break;
        }
    }
    else
    {
        // 128-bit UUID
        status = ATT_ERR_INVALID_HANDLE;
    }

    // If a charactersitic value changed then callback function to notify application of change
    if((notifyApp != 0xFF) && gnssProfile_AppCBs && gnssProfile_AppCBs->pfnGnssProfileChange)
    {
        gnssProfile_AppCBs->pfnGnssProfileChange(notifyApp, pValue, len);
    }

    return (status);
}



//读写回调
static bStatus_t rdssProfile_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method)
{
    bStatus_t status =SUCCESS;
    static uint8_t read_data1[RDSSPROFILE_CHAR1_LEN] = {0};
    static uint8_t read_data2[RDSSPROFILE_CHAR2_LEN] = {0};
    static uint8_t read_data3[RDSSPROFILE_CHAR3_LEN] = {0};
    static uint8_t read_data5[RDSSPROFILE_CHAR5_LEN] = {0};
    static uint8_t read_data6[RDSSPROFILE_CHAR6_LEN] = {0};
    static uint8_t read_data7[RDSSPROFILE_CHAR7_LEN] = {0};
    uint16_t i;

    if(gattPermitAuthenRead(pAttr->permissions))
    {
        return(ATT_ERR_INSUFFICIENT_AUTHOR);
    }
    if( offset > 0)
    {
        return(ATT_ERR_ATTR_NOT_LONG);
    }
    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0],pAttr->type.uuid[1]);

        switch(uuid)
        {
        case RDSSPROFILE_CHAR1_UUID://0x4501模块信息
                if(maxLen > RDSSPROFILE_CHAR1_LEN)
                {
                    *pLen = RDSSPROFILE_CHAR1_LEN;
                }
                else
                {
                    *pLen = maxLen;
                }
                // strcpy(Model_info.vendor, "CSJW");
                // strcpy(Model_info.type, "DM229");
                // strcpy(Model_info.version, "V1.5.98.6-Q");
                for(i=0; i<10; i++)
			    {
                    sscanf(Model_info.vendor + i,"%c",&read_data1[i]); //把字符串格式化为字符
                }
                for(i=0; i<10; i++)
			    {
                    sscanf(Model_info.type + i,"%c",&read_data1[10+i]); //把字符串格式化为字符
                }
                for(i=0; i<32; i++)
			    {
                    sscanf(Model_info.version + i,"%c",&read_data1[20+i]); //把字符串格式化为字符
                }

                tmos_memcpy(pValue, read_data1, *pLen);

        break;
        case RDSSPROFILE_CHAR2_UUID://0x4502卡信息，C6 3B 40 00 02 00 3C 02 00 00 00 00 03 00 00 00
                if(maxLen > RDSSPROFILE_CHAR2_LEN)
                {
                    *pLen = RDSSPROFILE_CHAR2_LEN;
                }
                else
                {
                    *pLen = maxLen;
                }
                read_data2[0] =(uint8_t) (Card_info.cardid  & 0x000000FF);
                read_data2[1] =(uint8_t) ((Card_info.cardid & 0x0000FF00)>>8);
                read_data2[2] =(uint8_t) ((Card_info.cardid & 0x00FF0000)>>16);
                read_data2[3] =(uint8_t) ((Card_info.cardid & 0xFF000000)>>24);
                read_data2[4] =Card_info.usertype;
                read_data2[5] =Card_info.encryped;
                read_data2[6] =Card_info.regional_service_freq;
                read_data2[7] =Card_info.regional_comm_grade;
                read_data2[8] =Card_info.global_service_freq;
                read_data2[9] =Card_info.global_comm_grade;
                read_data2[10] =(uint8_t)  (Card_info.subusernumber & 0x00FF);
                read_data2[11] =(uint8_t) ((Card_info.subusernumber & 0xFF00)>>8);
                read_data2[12] =Card_info.cardtype;
                read_data2[13] = 0;  // 保留字节
                read_data2[14] = 0;  // 保留字节
                read_data2[15] = 0;  // 保留字节

                tmos_memcpy(pValue, read_data2, *pLen);

        break;
        case RDSSPROFILE_CHAR3_UUID://0x4503信噪比前12强数组，00 00 00 00 00 00 00 00 00 00 00 00
                if(maxLen > RDSSPROFILE_CHAR3_LEN)
                {
                    *pLen = RDSSPROFILE_CHAR3_LEN;
                }
                else
                {
                    *pLen = maxLen;
                }

                for(i=0; i<12; i++)
			    {
                    read_data3[i] = snr[i];
                }

                tmos_memcpy(pValue, read_data3, *pLen);

        break;
        case RDSSPROFILE_CHAR5_UUID://0x4505反馈信息，00 00 00 00 00 00 00 00
                if(maxLen > RDSSPROFILE_CHAR5_LEN)
                {
                    *pLen = RDSSPROFILE_CHAR5_LEN;
                }
                else
                {
                    *pLen = maxLen;
                }
                read_data5[0] = Tx_ack.ack;
                read_data5[1] = Tx_ack.reason;
                read_data5[2] = 0;  // 保留字节
                read_data5[3] = 0;  // 保留字节
                read_data5[4] = 0;  // 保留字节
                read_data5[5] = 0;  // 保留字节
                read_data5[6] = 0;  // 保留字节
                read_data5[7] = 0;  // 保留字节
                tmos_memcpy(pValue, read_data5, *pLen);

        break;
        case RDSSPROFILE_CHAR6_UUID://0x4506入站信息
                if(maxLen > RDSSPROFILE_CHAR6_LEN)
                {
                    *pLen = RDSSPROFILE_CHAR6_LEN;
                }
                else
                {
                    *pLen = maxLen;
                }
                memset(read_data6, 0, RDSSPROFILE_CHAR6_LEN);
                read_data6[0] =(uint8_t) (Msg_rx.sender  & 0x000000FF);
                read_data6[1] =(uint8_t) ((Msg_rx.sender & 0x0000FF00)>>8);
                read_data6[2] =(uint8_t) ((Msg_rx.sender & 0x00FF0000)>>16);
                read_data6[3] =(uint8_t) ((Msg_rx.sender & 0xFF000000)>>24);
                read_data6[4] = Msg_rx.hour;
                read_data6[5] = Msg_rx.minute;
                read_data6[6] = Msg_rx.second;
                read_data6[7] = Msg_rx.encode;
                read_data6[8] =(uint8_t) (Msg_rx.mailtype  & 0x000000FF);
                read_data6[9] =(uint8_t) ((Msg_rx.mailtype & 0x0000FF00)>>8);
                read_data6[10] =(uint8_t) ((Msg_rx.mailtype & 0x00FF0000)>>16);
                read_data6[11] =(uint8_t) ((Msg_rx.mailtype & 0xFF000000)>>24);
                {
                    uint16_t payload_copy_len;

                    payload_copy_len = Rdss_SanitizePayloadLen(Msg_rx.payload, (uint16_t)Msg_rx.payload_len);
                    read_data6[12] =(uint8_t) (payload_copy_len  & 0x000000FF);
                    read_data6[13] =(uint8_t) ((payload_copy_len & 0x0000FF00)>>8);
                    read_data6[14] =(uint8_t) ((payload_copy_len & 0x00FF0000)>>16);
                    read_data6[15] =(uint8_t) ((payload_copy_len & 0xFF000000)>>24);
                    for(i=0; i<payload_copy_len; i++)
                    {
                        read_data6[16+i] = Msg_rx.payload[i];
                    }
                }

                tmos_memcpy(pValue, read_data6, *pLen);

        break;
        case RDSSPROFILE_CHAR7_UUID://0x4507频度倒计时，00 00 00 00
                if(maxLen > RDSSPROFILE_CHAR7_LEN)
                {
                    *pLen = RDSSPROFILE_CHAR7_LEN;
                }
                else
                {
                    *pLen = maxLen;
                }
                read_data7[0] =(uint8_t) (frequency_count_down  & 0x000000FF);
                read_data7[1] =(uint8_t) ((frequency_count_down & 0x0000FF00)>>8);
                read_data7[2] =(uint8_t) ((frequency_count_down & 0x00FF0000)>>16);
                read_data7[3] =(uint8_t) ((frequency_count_down & 0xFF000000)>>24);

                tmos_memcpy(pValue, read_data7, *pLen);

        break;

        default:
            *pLen = 0;
            status = ATT_ERR_ATTR_NOT_FOUND;
        break;
        }
    }
    else {
        *pLen = 0;
        status = ATT_ERR_INVALID_HANDLE;
    }
    return(status);
}

static bStatus_t rdssProfile_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method)
{
    bStatus_t status = SUCCESS;
    uint8_t   notifyApp = 0xFF;
    uint8_t i;

    // If attribute permissions require authorization to write, return error
    if(gattPermitAuthorWrite(pAttr->permissions))
    {
        // Insufficient authorization
        return (ATT_ERR_INSUFFICIENT_AUTHOR);
    }

    if(pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

        switch(uuid)
        {
            case RDSSPROFILE_CHAR4_UUID://发送数据
                if(offset == 0)
                {
                    if(len < 12)
                    {
                        status = ATT_ERR_INVALID_VALUE_SIZE;
                        PROFILE_DEBUG_PRINTF("\r\n[RDSS 4504 WRITE FAIL] len=%d, expect >= 12\r\n", len);
                    }
                }
                else {
                    status = ATT_ERR_ATTR_NOT_LONG;
                    PROFILE_DEBUG_PRINTF("\r\n[RDSS 4504 WRITE FAIL] offset=%d not supported\r\n", offset);
                }

                if(status == SUCCESS)
                {
                    uint16_t copy_len = len;
                    uint32_t requested_payload_len;
                    uint32_t app_payload_len;
                    uint8_t payload_rejected = FALSE;

                    if(copy_len > RDSSPROFILE_CHAR4_LEN)
                    {
                        copy_len = RDSSPROFILE_CHAR4_LEN;
                    }
                    memset(rdssProfileChar4, 0, RDSSPROFILE_CHAR4_LEN);
                    tmos_memcpy(pAttr->pValue, pValue, copy_len);

                    Msg_tx.lf = pValue[0];//频点
                    Msg_tx.encode = pValue[1];//编码类型
                    Msg_tx.generation = pValue[2];//北二或北三
                    Msg_tx.reservied = pValue[3];

                    // App端MsgTx.java格式不包含心率、血氧、步数、卡路里，这里清零保留。
                    Msg_tx.heart_rate = 0;
                    Msg_tx.blood_oxygen = 0;
                    Msg_tx.foot_step = 0;
                    Msg_tx.kcal = 0;

                    Msg_tx.dest_card = ((uint32_t)pValue[4])//目标卡号，小端
                                    |((uint32_t)pValue[5] << 8)
                                    |((uint32_t)pValue[6] << 16)
                                    |((uint32_t)pValue[7] << 24);
                    if(Msg_tx.dest_card != 0)
                    {
                        sprintf(DestIC, "%lu", (unsigned long)Msg_tx.dest_card);
                    }

                    requested_payload_len = ((uint32_t)pValue[8])//payload_len，小端
                                    |((uint32_t)pValue[9] << 8)
                                    |((uint32_t)pValue[10] << 16)
                                    |((uint32_t)pValue[11] << 24);
                    app_payload_len = requested_payload_len;
                    if(app_payload_len > (uint32_t)(len - 12))
                    {
                        app_payload_len = len - 12;
                        payload_rejected = TRUE;
                    }
                    Msg_tx.payload_len = Rdss_SanitizePayloadLen(&pValue[12], (uint16_t)app_payload_len);
                    if(requested_payload_len != Msg_tx.payload_len)
                    {
                        payload_rejected = TRUE;
                    }

                    memset(Msg_tx.payload, 0, sizeof(Msg_tx.payload));
                    for(i=0; i<Msg_tx.payload_len; i++)
                    {
                        Msg_tx.payload[i] = pValue[12+i];
                    }

                    if(payload_rejected)
                    {
                        Tx_ack.ack = false;
                        Tx_ack.reason = RDSS_ACK_REASON_PAYLOAD_TOO_LONG;
                        memset(Tx_ack._unused, 0, sizeof(Tx_ack._unused));
                        RD_tx_ack_dirty = 1;
                        RD_txflag = false;
                        PROFILE_DEBUG_PRINTF("\r\n[RDSS 4504 WRITE FAIL] payload_len=%lu accepted=%lu max=%d\r\n",
                                             (unsigned long)requested_payload_len,
                                             (unsigned long)Msg_tx.payload_len,
                                             RDSS_MSG_PAYLOAD_MAX);
                        notifyApp = RDSSPROFILE_CHAR5;
                    }
                    else
                    {
                        PROFILE_DEBUG_PRINTF("\r\n[RDSS 4504 WRITE OK]\r\n");
                        PROFILE_DEBUG_PRINTF("len=%d\r\n", len);
                        PROFILE_DEBUG_PRINTF("lf=%d\r\n", Msg_tx.lf);
                        PROFILE_DEBUG_PRINTF("encode=%d\r\n", Msg_tx.encode);
                        PROFILE_DEBUG_PRINTF("generation=%d\r\n", Msg_tx.generation);
                        PROFILE_DEBUG_PRINTF("reserved=%d\r\n", Msg_tx.reservied);
                        PROFILE_DEBUG_PRINTF("dest_card=%lu\r\n", (unsigned long)Msg_tx.dest_card);
                        PROFILE_DEBUG_PRINTF("payload_len=%lu\r\n", (unsigned long)Msg_tx.payload_len);
                        PROFILE_DEBUG_PRINTF("payload_hex=");
                        for(i = 0; i < Msg_tx.payload_len; i++)
                        {
                            PROFILE_DEBUG_PRINTF("%02X ", Msg_tx.payload[i]);
                        }
                        PROFILE_DEBUG_PRINTF("\r\n[RDSS 4504 WRITE END]\r\n");

                        Pwr_EnableRdssForSend();
                        RD_txflag = true;
                        PROFILE_DEBUG_PRINTF("[RDSS 4504 WRITE] RD_txflag=%d, wait RDSS task send\r\n", RD_txflag);
                        notifyApp = RDSSPROFILE_CHAR4;
                    }
                }
                PROFILE_DEBUG_PRINTF("rdss_write\r\n");
            break;

             case GATT_CLIENT_CHAR_CFG_UUID:
                status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len,
                                                        offset, GATT_CLIENT_CFG_NOTIFY);
                break;

            default:
                // Should never get here! (characteristics 2 and 4 do not have write permissions)
                status = ATT_ERR_ATTR_NOT_FOUND;
                break;
        }
    }
    else
    {
        // 128-bit UUID
        status = ATT_ERR_INVALID_HANDLE;
    }

    // If a charactersitic value changed then callback function to notify application of change
    if((notifyApp != 0xFF) && rdssProfile_AppCBs && rdssProfile_AppCBs->pfnRdssProfileChange)
    {
        rdssProfile_AppCBs->pfnRdssProfileChange(notifyApp, pValue, len);
    }

    return (status);
}


/*********************************************************************
*********************************************************************/
