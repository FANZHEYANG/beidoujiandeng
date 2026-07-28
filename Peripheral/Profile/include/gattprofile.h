/********************************** (C) COPYRIGHT *******************************
 * File Name          : gattprofile.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/11
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef GATTPROFILE_H
#define GATTPROFILE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

// Profile Parameters
#define SIMPLEPROFILE_CHAR1         0           // RW uint8_t - Profile Characteristic 1 value
#define SIMPLEPROFILE_CHAR2         1           // RW uint8_t - Profile Characteristic 2 value
#define SIMPLEPROFILE_CHAR3         2           // RW uint8_t - Profile Characteristic 3 value
#define SIMPLEPROFILE_CHAR4         3           // RW uint8_t - Profile Characteristic 4 value
#define SIMPLEPROFILE_CHAR5         4           // RW uint8_t - Profile Characteristic 4 value

#define CARDPROFILE_CHAR            0
#define BATTERYPROFILE_CHAR         0

#define GNSSPROFILE_CHAR1           0           // RW uint8_t - Profile Characteristic 1 value
#define GNSSPROFILE_CHAR2           1           // RW uint8_t - Profile Characteristic 2 value

#define CONTROLPROFILE_CHAR1         0           // RW uint8_t - Profile Characteristic 1 value
#define CONTROLPROFILE_CHAR2         1           // RW uint8_t - Profile Characteristic 2 value
#define CONTROLPROFILE_CHAR3         2           // RW uint8_t - Profile Characteristic 3 value
#define CONTROLPROFILE_CHAR4         3           // RW uint8_t - Profile Characteristic 4 value
#define CONTROLPROFILE_CHAR5         4           // RW uint8_t - Profile Characteristic 5 value

#define RDSSPROFILE_CHAR1            0           // RW uint8_t - Profile Characteristic 1 value
#define RDSSPROFILE_CHAR2            1           // RW uint8_t - Profile Characteristic 2 value
#define RDSSPROFILE_CHAR3            2           // RW uint8_t - Profile Characteristic 3 value
#define RDSSPROFILE_CHAR4            3           // RW uint8_t - Profile Characteristic 4 value
#define RDSSPROFILE_CHAR5            4           // RW uint8_t - Profile Characteristic 5 value
#define RDSSPROFILE_CHAR6            5           // RW uint8_t - Profile Characteristic 6 value
#define RDSSPROFILE_CHAR7            6           // RW uint8_t - Profile Characteristic 7 value
// Simple Profile Service UUID
#define SIMPLEPROFILE_SERV_UUID     0xFFE0

// Key Pressed UUID
#define SIMPLEPROFILE_CHAR1_UUID    0xFFE1
#define SIMPLEPROFILE_CHAR2_UUID    0xFFE2
#define SIMPLEPROFILE_CHAR3_UUID    0xFFE3
#define SIMPLEPROFILE_CHAR4_UUID    0xFFE4
#define SIMPLEPROFILE_CHAR5_UUID    0xFFE5

// 控制命令Control Profile Service UUID
#define CONTROLPROFILE_SERV_UUID     0x4000

// Key Pressed UUID
#define CONTROLPROFILE_CHAR1_UUID    0x4001 //GNSS开关
#define CONTROLPROFILE_CHAR2_UUID    0x4002 //RDSS开关
#define CONTROLPROFILE_CHAR3_UUID    0x4003 //SOS开关
#define CONTROLPROFILE_CHAR4_UUID    0x4004 //RDSS过检开关
#define CONTROLPROFILE_CHAR5_UUID    0x4005 //RDSS复位开关

//CARD Profile Service UUID
#define CARDPROFILE_SERV_UUID         0x4100
//CARD UUID
#define CARDPROFILE_CHAR_UUID         0x4101

// RDSS命令Rdss Profile Service UUID
#define RDSSPROFILE_SERV_UUID         0x4500

// RDSS UUID
#define RDSSPROFILE_CHAR1_UUID        0x4501 //模块信息
#define RDSSPROFILE_CHAR2_UUID        0x4502 //卡信息
#define RDSSPROFILE_CHAR3_UUID        0x4503 //信噪比前12强数组
#define RDSSPROFILE_CHAR4_UUID        0x4504 //发送数据
#define RDSSPROFILE_CHAR5_UUID        0x4505 //反馈信息
#define RDSSPROFILE_CHAR6_UUID        0x4506 //入站信息
#define RDSSPROFILE_CHAR7_UUID        0x4507 //频度倒计时

// Simple Keys Profile Services bit fields参数是一个位掩码，用于指定要添加的服务类型
#define SIMPLEPROFILE_SERVICE       0x00000001
//自定义
#define CARDPROFILE_SERVICE         0x00000001
#define BATTERYPROFILE_SERVICE      0x00000001
#define CONTROLPROFILE_SERVICE      0x00000001
#define GNSSPROFILE_SERVICE         0x00000001
#define RDSSPROFILE_SERVICE         0x00000001

// Length of characteristic in bytes ( Default MTU is 23 )
#define SIMPLEPROFILE_CHAR1_LEN     1
#define SIMPLEPROFILE_CHAR2_LEN     1
#define SIMPLEPROFILE_CHAR3_LEN     1
#define SIMPLEPROFILE_CHAR4_LEN     1
#define SIMPLEPROFILE_CHAR5_LEN     5
//电池电量
#define BATTERYPROFILE_CHAR_LEN     1
//卡参数配置
#define CARDPROFILE_CHAR_LEN         4
//控制命令
#define CONTROLPROFILE_CHAR1_LEN     1
#define CONTROLPROFILE_CHAR2_LEN     1
#define CONTROLPROFILE_CHAR3_LEN     1
#define CONTROLPROFILE_CHAR4_LEN     1
#define CONTROLPROFILE_CHAR5_LEN     1
//GNSS命令
#define GNSSPROFILE_CHAR1_LEN     12
#define GNSSPROFILE_CHAR2_LEN     40
//RDSS命令
#define RDSSPROFILE_CHAR1_LEN     52
#define RDSSPROFILE_CHAR2_LEN     16
#define RDSSPROFILE_CHAR3_LEN     12
#define RDSSPROFILE_CHAR4_LEN     92
#define RDSSPROFILE_CHAR5_LEN     8
//538
#define RDSSPROFILE_CHAR6_LEN     86
#define RDSSPROFILE_CHAR7_LEN     4

//自定义添加

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef void (*simpleProfileChange_t)(uint8_t paramID, uint8_t *pValue, uint16_t len);

typedef struct
{
    simpleProfileChange_t pfnSimpleProfileChange; // Called when characteristic value changes
} simpleProfileCBs_t;

//电池电量
typedef void (*batteryProfileChange_t)(uint8_t paramID, uint8_t *pValue, uint16_t len);

typedef struct
{
    batteryProfileChange_t pfnBatteryProfileChange; // Called when characteristic value changes
} batteryProfileCBs_t;

//控制命令
typedef void (*controlProfileChange_t)(uint8_t paramID, uint8_t *pValue, uint16_t len);

typedef struct
{
    controlProfileChange_t pfncontrolProfileChange; // Called when characteristic value changes
} controlProfileCBs_t;

//GNSS
typedef void (*gnssProfileChange_t)(uint8_t paramID, uint8_t *pValue, uint16_t len);

typedef struct
{
    gnssProfileChange_t pfnGnssProfileChange; // Called when characteristic value changes
} gnssProfileCBs_t;

//RDSS
typedef void (*rdssProfileChange_t)(uint8_t paramID, uint8_t *pValue, uint16_t len);

typedef struct
{
    rdssProfileChange_t pfnRdssProfileChange; // Called when characteristic value changes
} rdssProfileCBs_t;

// Callback when a characteristic value has changed
typedef void (*cardProfileChange_t)(uint8_t paramID, uint8_t *pValue, uint16_t len);

typedef struct
{
    cardProfileChange_t pfncardProfileChange; // Called when characteristic value changes
} cardProfileCBs_t;
/*********************************************************************
 * API FUNCTIONS
 */

/*
 * SimpleProfile_AddService- Initializes the Simple GATT Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 */
extern bStatus_t Battery_Addservice(uint32_t services);
extern bStatus_t Control_Addservice(uint32_t services);
extern bStatus_t Gnss_Addservice(uint32_t services);
extern bStatus_t CardProfile_Addservice(uint32_t services);
extern bStatus_t SimpleProfile_AddService(uint32_t services);
extern bStatus_t Rdss_Addservice(uint32_t services);

/*
 * SimpleProfile_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t SimpleProfile_RegisterAppCBs(simpleProfileCBs_t *appCallbacks);
extern bStatus_t CardProfile_RegisterAppCBs(cardProfileCBs_t *appCallbacks);
extern bStatus_t BatteryProfile_RegisterAppCBs(batteryProfileCBs_t *appCallbacks);
extern bStatus_t ControlProfile_RegisterAppCBs(controlProfileCBs_t *appCallbacks);
extern bStatus_t GnssProfile_RegisterAppCBs(gnssProfileCBs_t *appCallbacks);
extern bStatus_t RdssProfile_RegisterAppCBs(rdssProfileCBs_t *appCallbacks);
/*
 * SimpleProfile_SetParameter - Set a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16_t will be cast to
 *          uint16_t pointer).
 */
extern bStatus_t SimpleProfile_SetParameter(uint8_t param, uint8_t len, void *value);
extern bStatus_t BatteryProfile_SetParameter(uint8_t param, uint8_t len, void *value);
extern bStatus_t ControlProfile_SetParameter(uint8_t param, uint8_t len, void *value);
extern bStatus_t GnssProfile_SetParameter(uint8_t param, uint8_t len, void *value);
extern bStatus_t RdssProfile_SetParameter(uint8_t param, uint8_t len, void *value);
/*
 * SimpleProfile_GetParameter - Get a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16_t will be cast to
 *          uint16_t pointer).
 */
extern bStatus_t SimpleProfile_GetParameter(uint8_t param, void *value);

/*
 * simpleProfile_Notify - Send notification.
 *
 *    connHandle - connect handle
 *    pNoti - pointer to structure to notify.
 */
extern bStatus_t simpleProfile_Notify(uint16_t connHandle, attHandleValueNoti_t *pNoti);
extern bStatus_t batteryProfile_Notify(uint16_t connHandle, attHandleValueNoti_t *pNoti);
extern bStatus_t gnssProfile_Notify1(uint16_t connHandle, attHandleValueNoti_t *pNoti);
extern bStatus_t gnssProfile_Notify2(uint16_t connHandle, attHandleValueNoti_t *pNoti);
extern void float_to_bytes_memcpy(float value, uint8_t* bytes);

extern bStatus_t rdssProfile_Notify2(uint16_t connHandle, attHandleValueNoti_t *pNoti);
extern bStatus_t rdssProfile_Notify3(uint16_t connHandle, attHandleValueNoti_t *pNoti);
extern bStatus_t rdssProfile_Notify5(uint16_t connHandle, attHandleValueNoti_t *pNoti);
extern bStatus_t rdssProfile_Notify6(uint16_t connHandle, attHandleValueNoti_t *pNoti);
extern bStatus_t rdssProfile_Notify7(uint16_t connHandle, attHandleValueNoti_t *pNoti);
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
