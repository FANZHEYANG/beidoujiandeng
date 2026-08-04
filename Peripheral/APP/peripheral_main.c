/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2020/08/06
 * Description        : 外设从机应用主函数及任务系统初始化
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "CONFIG.h"
#include "HAL.h"
#include "gattprofile.h"
#include "peripheral.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
const uint8_t MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

/*********************************************************************
 * @fn      Main_Circulation
 *
 * @brief   主循环
 *
 * @return  none
 */
__HIGH_CODE
__attribute__((noinline))
void Main_Circulation()
{
    while(1)
    {
        TMOS_SystemProcess();
    }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   主函数
 *
 * @return  none
 */
int main(void)
{
#if(defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    PWR_DCDCCfg(ENABLE);      //使能DCDC电源
#endif
    HSECFG_Capacitance(HSECap_18p);
    SetSysClock(SYSCLK_FREQ); //设置系统时钟
#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    GPIOB_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
#endif

#ifdef DEBUG
    GPIOA_SetBits(GPIO_Pin_7);
    GPIOA_ModeCfg(GPIO_Pin_6, GPIO_ModeIN_PU);      // RXD-配置上拉输入
    GPIOA_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_5mA); // TXD-配置推挽输出，注意先让IO口输出高电平
    UART2_DefInit();
    //PRINT("UART2 debug start\r\n");//调试串口2
#endif
    //PRINT("%s\n", VER_LIB);

    HAL_LedInit();
    HAL_KeyInit();

    //+3.3V_AU_EN（Audio Power）
    GPIOB_ModeCfg( GPIO_Pin_16 , GPIO_ModeOut_PP_5mA );
    //+3.3V_CM_EN(RNSS CM112B Power)
    GPIOB_ModeCfg( GPIO_Pin_1 , GPIO_ModeOut_PP_5mA );
    //AT-EN PA15（RNSS AT2659）
    GPIOA_ModeCfg( GPIO_Pin_15 , GPIO_ModeOut_PP_5mA );
     //+3.7V_DM_EN(RDSS DM229 Power)
    GPIOB_ModeCfg( GPIO_Pin_3 , GPIO_ModeOut_PP_5mA );
    //+3.3V_DM_EN（RNSS DM229 Power）
    GPIOB_ModeCfg( GPIO_Pin_18 , GPIO_ModeOut_PP_5mA );

    GPIOB_SetBits(GPIO_Pin_16);
    GPIOB_ResetBits(GPIO_Pin_1|GPIO_Pin_3|GPIO_Pin_18);
    GPIOA_ResetBits(GPIO_Pin_15);
    
    DelayMs(100);
    
    XT90_Init();

    CH58x_BLEInit();
    HAL_Init();
    GAPRole_PeripheralInit();
    Peripheral_Init();

    Pwr_init();
    Bat_init();

    RNSS_init();
    RDSS_init();

     Main_Circulation();
}

/******************************** endfile @ main ******************************/
