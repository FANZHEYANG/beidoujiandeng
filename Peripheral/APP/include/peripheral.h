/*
 * @Author: FANZHEYANG zheyangfan793@gmail.com
 * @Date: 2026-01-01 22:10:46
 * @LastEditors: FANZHEYANG zheyangfan793@gmail.com
 * @LastEditTime: 2026-06-24 14:03:07
 * @FilePath: \BLE_all0307 (gnss和rdss实现）\Peripheral\APP\include\peripheral.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/********************************** (C) COPYRIGHT *******************************
 * File Name          : peripheral.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/11
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef PERIPHERAL_H
#define PERIPHERAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */


// Peripheral Task Events
#define SBP_START_DEVICE_EVT    0x0001 //初始化GAPRole任务
#define SBP_PERIODIC_EVT        0x0002 //周期事件，5000ms
#define SBP_READ_RSSI_EVT       0x0004
#define SBP_PARAM_UPDATE_EVT    0x0008
#define SBP_PHY_UPDATE_EVT      0x0010
#define SBP_BLE_DISCONNECT_VOICE_EVT 0x0020


/*********************************************************************
 * MACROS
 */
typedef struct
{
    uint16_t connHandle; // Connection handle of current connection
    uint16_t connInterval;
    uint16_t connSlaveLatency;
    uint16_t connTimeout;
} peripheralConnItem_t;

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the BLE Application
 */
extern void Peripheral_Init(void);
extern void Peripheral_BleOff(void);
extern void Peripheral_BleOn(void);

/*
 * Task Event Processor for the BLE Application
 */
extern uint16_t Peripheral_ProcessEvent(uint8_t task_id, uint16_t events);



/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
