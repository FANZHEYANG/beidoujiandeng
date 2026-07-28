/********************************** (C) COPYRIGHT *******************************
 * File Name          : KEY.c
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2022/01/18
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "HAL.h"

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/

static uint8_t halKeySavedKeys; /* 保留按键最后的状态，用于查询是否有键值变化 */

/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
static halKeyCBack_t pHalKeyProcessFunction; /* callback function */

/**************************************************************************************************
 * @fn      HAL_KeyInit
 *
 * @brief   Initilize Key Service
 *
 * @param   none
 *
 * @return  None
 **************************************************************************************************/
void HAL_KeyInit(void)
{
    // /* Initialize previous key to 0 */
    // halKeySavedKeys = 0;
    // /* Initialize callback function */
    // pHalKeyProcessFunction = NULL;

    //初始化位输入
    GPIOB_ModeCfg(GPIO_Pin_2, GPIO_ModeIN_Floating);//KEY1
    GPIOB_ModeCfg(GPIO_Pin_5, GPIO_ModeIN_Floating);//KEY2
    // //开启下降沿中断
     GPIOB_ITModeCfg(GPIO_Pin_2,GPIO_ITMode_FallEdge);
     GPIOB_ITModeCfg(GPIO_Pin_5,GPIO_ITMode_FallEdge);
     GPIOB_ClearITFlagBit(GPIO_Pin_2);
     GPIOB_ClearITFlagBit(GPIO_Pin_5);
    // //开启GPIO的睡眠唤醒,如果需要的话
    // //PWR_PeriphWakeUpCfg( ENABLE, RB_SLP_GPIO_WAKE,Long_Delay ); 
    // //开启GPIO中断
     PFIC_EnableIRQ( GPIO_B_IRQn );
}

/**************************************************************************************************
 * @fn      HalKeyConfig
 *
 * @brief   Configure the Key serivce
 *
 * @param   cback - pointer to the CallBack function
 *
 * @return  None
 **************************************************************************************************/
void HalKeyConfig(halKeyCBack_t cback)
{
    /* Register the callback fucntion */
    pHalKeyProcessFunction = cback;
    tmos_start_task(halTaskID, HAL_KEY_EVENT, HAL_KEY_POLLING_VALUE); /* Kick off polling */
}

/**************************************************************************************************
 * @fn      HalKeyRead
 *
 * @brief   Read the current value of a key
 *
 * @param   None
 *
 * @return  keys - current keys status
 **************************************************************************************************/
uint8_t HalKeyRead(void)
{
    uint8_t keys = 0;

    if(HAL_PUSH_BUTTON1())
    { //读按键1
        keys |= HAL_KEY_SW_1;
    }
    if(HAL_PUSH_BUTTON2())
    { //读按键2
        keys |= HAL_KEY_SW_2;
    }
    return keys;
}

/**************************************************************************************************
 * @fn      HAL_KeyPoll
 *
 * @brief   Called by hal_driver to poll the keys
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HAL_KeyPoll(void)
{
    uint8_t keys = 0;
    if(HAL_PUSH_BUTTON1())
    {
        keys |= HAL_KEY_SW_1;
    }
    if(HAL_PUSH_BUTTON2())
    {
        keys |= HAL_KEY_SW_2;
    }
    if(HAL_PUSH_BUTTON3())
    {
        keys |= HAL_KEY_SW_3;
    }
    if(HAL_PUSH_BUTTON4())
    {
        keys |= HAL_KEY_SW_4;
    }
    if(keys == halKeySavedKeys)
    { /* Exit - since no keys have changed */
        return;
    }
    halKeySavedKeys = keys; /* Store the current keys for comparation next time */
    /* Invoke Callback if new keys were depressed */
    if(keys && (pHalKeyProcessFunction))
    {
        (pHalKeyProcessFunction)(keys);
    }
}

//KEY1(SOS按键)中断服务程序
__INTERRUPT
__HIGH_CODE
void GPIOB_IRQHandler(void) {
    if(GPIOB_ReadITFlagBit(GPIO_Pin_2))//KEY1
    {
        Pwr_Key1Pressed();
    }
    if(GPIOB_ReadITFlagBit(GPIO_Pin_5))//KEY2
    {
        PWR_SW_Flag = TRUE;
        // if(PWR_SW_cnt < 4)
        //     PWR_SW_cnt++;
        //     else
        //     PWR_SW_cnt = 0;
    }
    GPIOB_ClearITFlagBit(GPIO_Pin_2);
    GPIOB_ClearITFlagBit(GPIO_Pin_5);
}

/******************************** endfile @ key ******************************/
