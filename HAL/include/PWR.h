

/******************************************************************************/
#ifndef __PWR_H
#define __PWR_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 *                                              MACROS
 **************************************************************************************************/
/* I2C Mode Definition BQ25601 */
#define IIC_HOST_MODE     1
#define IIC_SLAVE_MODE    0

/* Global define BQ25601*/
#define SIZE            7
#define MASTER_ADDR     0x42

/* Global define Battery*/
#define VOLTAGE_DIVIDER_RATIO 2 // 电压分压比
#define BATTERY_MAX_VOLTAGE 4.2 // 电池满电电压
#define BATTERY_MIN_VOLTAGE 3.0 // 电池极低电压

/**************************************************************************************************
 * TYPEDEFS
 **************************************************************************************************/

/**************************************************************************************************
 *                                             GLOBAL VARIABLES
 **************************************************************************************************/
extern uint8_t Bat_percentage;

extern BOOL RN_SW_Flag;
extern BOOL RD_SW_Flag;
extern BOOL SOS_SW_Flag;
extern BOOL PWR_SW_Flag;
extern uint8_t PWR_SW_cnt;

extern BOOL RD_TEST_Flag;
extern BOOL RD_RST_Flag;
/*********************************************************************
 * FUNCTIONS
 */
//AUDIO
extern void OPENAUDIO(void);
extern void CLOSEAUDIO(void);
//RDSS
extern void OPENRD(void);
extern void Pwr_EnableRdssForSend(void);
extern void CLOSERD(void);
//CM112B RNSS
extern void OPENRN(void);
extern void CLOSERN(void);
//BQ25601
extern void BQ25601_Init(void);
//Battery
extern void BATTERY_ADC_Init(void);
extern uint8_t BATTERY_ADC(void);
extern void Pwr_RequestBatteryVoice(void);
extern uint8_t Pwr_RequestVoiceText(const char *text);
extern void Pwr_Key1Pressed(void);
extern void Pwr_OnGnssFixUpdate(uint8_t fixed);
extern void Pwr_OnRdssMessageReceived(uint32_t sender);
extern void Pwr_RequestAutoPowerOff(void);
extern void Pwr_SetLocationReportInterval(uint32_t interval_sec);
extern void Pwr_RequestLocationReport(void);



 //event
#define pwr_evt                  0x0001  
#define sos_evt                  0x0002  
#define bat_evt                  0x0004  //电池电量采集
#define pwroff_evt               0x0008  //电池电量采集
#define key1_evt                 0x0010  //KEY1短按/长按检测
#define sos_alarm_evt            0x0020  //SOS报警状态机
#define auto_poweroff_evt        0x0040  //蓝牙断开15分钟自动关机
#define location_report_evt      0x0080  //定时发送位置
#define water_detect_evt         0x0100  //落水检测

//初始化，注册task
extern void Pwr_init(void);
extern void Bat_init(void);

/**************************************************************************************************
**************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
