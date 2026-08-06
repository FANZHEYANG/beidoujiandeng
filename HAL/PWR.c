

/******************************************************************************/
/* 头文件包含 */
#include "HAL.h"
#include "peripheral.h"

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/
signed short RoughCalib_Value = 0; // ADC粗调偏差值
uint8_t Bat_percentage = 0;

BOOL RN_SW_Flag   = FALSE;//默认关闭
BOOL RD_SW_Flag   = FALSE;
BOOL SOS_SW_Flag  = FALSE;
BOOL PWR_SW_Flag  = FALSE;
uint8_t PWR_SW_cnt = 0;

BOOL RD_TEST_Flag = FALSE;
BOOL RD_RST_Flag  = FALSE;   

static uint8_t Pwr_TaskID; // Task ID for PWR processing
static uint8_t Bat_TaskID; // Task ID for BAT processing
#define VOICE_LOOP_TEST 0 // 1:循环测试语音播放， 0:正常逻辑
#define VOICE_QUEUE_MAX 10
#define VOICE_TEXT_MAX 64
#define VOICE_PLAY_GAP MS1_TO_SYSTEM_TIME(3000)
#define VOICE_KEY_DEBOUNCE_GAP MS1_TO_SYSTEM_TIME(80)
#define KEY1_SCAN_PERIOD MS1_TO_SYSTEM_TIME(100)
#define KEY1_LONG_PRESS_COUNT 30
#define SOS_WAIT_FIX_MAX_COUNT (20 * 60)
#define SOS_SEND_MAX_COUNT 20
#define SOS_ALARM_PERIOD MS1_TO_SYSTEM_TIME(1000)
#define SOS_MSG_LF 3
#define SOS_MSG_ENCODE 2
#define SOS_MSG_GENERATION 3
#define LOCATION_MSG_LF SOS_MSG_LF
#define LOCATION_MSG_ENCODE SOS_MSG_ENCODE
#define LOCATION_MSG_GENERATION SOS_MSG_GENERATION
#define WATER_DETECT_PERIOD MS1_TO_SYSTEM_TIME(200)
#define WATER_ADC_CHANNEL 2
#define WATER_ADC_SAMPLE_COUNT 10
#define WATER_SHORT_THRESHOLD_MV 2000
#define WATER_ADC_MV_NUMERATOR 1050
#define WATER_ADC_MV_DENOMINATOR 2048
#define WATER_SHORT_CONFIRM_COUNT 3
#define WATER_CLEAR_CONFIRM_COUNT 5

#define SOS_STATE_IDLE 0
#define SOS_STATE_WAIT_FIX 1
#define SOS_STATE_SENDING 2

#define SOS_EXIT_LONG_PRESS 1
#define SOS_EXIT_REPLY 2
#define SOS_EXIT_SEND_MAX 3
#define SOS_EXIT_NO_FIX 4
#define SOS_EXIT_POWER 5
#define SOS_EXIT_NO_CONTACT 6
#if SOS_TEST_ONLY_PRINT
#define VOICE_DEBUG_PRINT(...)
#else
#define VOICE_DEBUG_PRINT(...) PRINT(__VA_ARGS__)
#endif

static char voice_queue[VOICE_QUEUE_MAX][VOICE_TEXT_MAX];
static volatile uint8_t voice_queue_head = 0;
static volatile uint8_t voice_queue_tail = 0;
static volatile uint8_t voice_queue_count = 0;
static volatile uint8_t voice_playing = 0;
static volatile uint32_t voice_bat_last_req_time = 0;
static uint8_t gnss_last_fixed = 0;

static volatile uint8_t key1_irq_pending = 0;
static uint8_t key1_checking = 0;
static uint8_t key1_long_done = 0;
static uint16_t key1_hold_count = 0;

static uint8_t sos_state = SOS_STATE_IDLE;
static uint8_t sos_send_count = 0;
static uint16_t sos_wait_fix_count = 0;
static volatile uint8_t sos_reply_received = 0;
static uint32_t location_report_interval_sec = 0;

static uint8_t led_flash_enable = 0;
static uint8_t manual_flash_enable = 0;
static uint8_t sos_flash_enable = 0;
static uint8_t water_flash_enable = 0;
static uint8_t water_short_active = 0;
static uint8_t water_short_count = 0;
static uint8_t water_clear_count = 0;
static uint8_t led_index = 0;
static int8_t led_dir = 1;
static uint8_t blink_phase = 0;

static uint8_t key2_checking = 0;
static uint8_t key2_long_done = 0;
static uint16_t key2_hold_count = 0;
static uint8_t key2_wait_release = 0;
static uint8_t key2_release_count = 0;
#define KEY2_LONG_PRESS_COUNT 48
#define KEY2_RELEASE_STABLE_COUNT 10

static uint8_t soft_power_off = 0;  

static void UpdateLedFlash(void);
static void Pwr_UpdateLedFlashEnable(void);
static void Pwr_WaterDetectInit(void);
static uint16_t Pwr_ReadWaterAdc(void);
static uint16_t Pwr_ReadWaterVoltageMv(void);
static void Pwr_HandleWaterDetectEvent(void);
static void Pwr_ClearVoiceQueue(void);
static uint8_t Pwr_QueueVoiceText(const char *text);
static uint8_t Pwr_DequeueVoiceText(char *text);

static void Pwr_StartSosAlarm(void);
static void Pwr_StopSosAlarm(uint8_t reason);
static void Pwr_HandleKey1Event(void);
static void Pwr_HandleSosAlarmEvent(void);
static uint8_t Pwr_QueueLocationMessage(void);
static void Pwr_ScheduleLocationReport(void);

uint8_t  key_timer_cnt=0;
/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/

static void Pwr_ClearVoiceQueue(void)
{
    voice_queue_head = 0;
    voice_queue_tail = 0;
    voice_queue_count = 0;
    voice_playing = 0;
    memset(voice_queue, 0, sizeof(voice_queue));
}

static uint8_t Pwr_QueueVoiceText(const char *text)
{
    uint16_t len;

    if((text == NULL) || (text[0] == '\0') || (soft_power_off != 0))
    {
        return 0;
    }

    if(voice_queue_count >= VOICE_QUEUE_MAX)
    {
        VOICE_DEBUG_PRINT("[VOICE] queue full, drop=%s\r\n", text);
        return 0;
    }

    len = strlen(text);
    if(len >= VOICE_TEXT_MAX)
    {
        len = VOICE_TEXT_MAX - 1;
    }

    memset(voice_queue[voice_queue_tail], 0, VOICE_TEXT_MAX);
    tmos_memcpy(voice_queue[voice_queue_tail], text, len);
    voice_queue_tail++;
    if(voice_queue_tail >= VOICE_QUEUE_MAX)
    {
        voice_queue_tail = 0;
    }
    voice_queue_count++;

    if(voice_playing == 0)
    {
        tmos_start_task(Pwr_TaskID, sos_evt, 1);
    }
    return 1;
}

uint8_t Pwr_RequestVoiceText(const char *text)
{
    return Pwr_QueueVoiceText(text);
}

static uint8_t Pwr_DequeueVoiceText(char *text)
{
    if((text == NULL) || (voice_queue_count == 0))
    {
        return 0;
    }

    strcpy(text, voice_queue[voice_queue_head]);
    memset(voice_queue[voice_queue_head], 0, VOICE_TEXT_MAX);
    voice_queue_head++;
    if(voice_queue_head >= VOICE_QUEUE_MAX)
    {
        voice_queue_head = 0;
    }
    voice_queue_count--;

    return 1;
}

void Pwr_RequestBatteryVoice(void)
{
    char data[4] = {0};
    char strBAT[24] = {0};
    uint32_t now = TMOS_GetSystemClock();

    if(soft_power_off != 0)
    {
        return;
    }

    if((voice_bat_last_req_time != 0) &&
       ((uint32_t)(now - voice_bat_last_req_time) < VOICE_KEY_DEBOUNCE_GAP))
    {
        return;
    }

    voice_bat_last_req_time = now;
    sprintf(data, "%d", Bat_percentage);
    strcat(strBAT, "电量百分之");
    strcat(strBAT, data);
    strcat(strBAT, "\r\n");
    Pwr_QueueVoiceText(strBAT);
}

void Pwr_Key1Pressed(void)
{
    if(soft_power_off == 0)
    {
        key1_irq_pending = 1;
    }
}

void Pwr_OnGnssFixUpdate(uint8_t fixed)
{
    if(soft_power_off != 0)
    {
        return;
    }

    if(fixed)
    {
        if(gnss_last_fixed == 0)
        {
            gnss_last_fixed = 1;
            Pwr_QueueVoiceText("已定位");
            VOICE_DEBUG_PRINT("[VOICE GNSS] fixed, queue located voice\r\n");
        }
    }
    else
    {
        gnss_last_fixed = 0;
    }
}

void Pwr_OnRdssMessageReceived(uint32_t sender)
{
    char voice_text[VOICE_TEXT_MAX] = {0};
    uint32_t contact = strtoul((char *)DestIC, NULL, 10);

    if(sender != 0)
    {
        sprintf(voice_text, "收到%lu的消息", (unsigned long)sender);
        Pwr_QueueVoiceText(voice_text);
        VOICE_DEBUG_PRINT("[VOICE RDSS] queue rx message voice sender=%lu\r\n", (unsigned long)sender);
    }

    if((sos_state != SOS_STATE_IDLE) && (contact != 0) && (sender == contact))
    {
        sos_reply_received = 1;
    }
}

static void Pwr_UpdateLedFlashEnable(void)
{
    uint8_t next_enable = (manual_flash_enable || sos_flash_enable || water_flash_enable) ? 1 : 0;

    if((led_flash_enable != 0) && (next_enable == 0))
    {
        led_index = 0;
        blink_phase = 0;
        HalLedOnOff(HAL_LED_ALL, HAL_LED_MODE_OFF);
    }

    led_flash_enable = next_enable;
}

static void Pwr_WaterDetectInit(void)
{
    GPIOA_ModeCfg(GPIO_Pin_13, GPIO_ModeOut_PP_5mA);
    GPIOA_SetBits(GPIO_Pin_13);
    GPIOA_ModeCfg(GPIO_Pin_12, GPIO_ModeIN_Floating);
}

static uint16_t Pwr_ReadWaterAdc(void)
{
    uint8_t i;
    uint32_t total = 0;

    ADC_ChannelCfg(WATER_ADC_CHANNEL);
    for(i = 0; i < WATER_ADC_SAMPLE_COUNT; i++)
    {
        int32_t sample = (int32_t)ADC_ExcutSingleConver() + RoughCalib_Value;
        if(sample < 0)
        {
            sample = 0;
        }
        else if(sample > 4095)
        {
            sample = 4095;
        }
        total += (uint16_t)sample;
    }

    return (uint16_t)(total / WATER_ADC_SAMPLE_COUNT);
}

static uint16_t Pwr_ReadWaterVoltageMv(void)
{
    uint32_t water_raw = Pwr_ReadWaterAdc();

    return (uint16_t)((water_raw * WATER_ADC_MV_NUMERATOR) / WATER_ADC_MV_DENOMINATOR);
}

static void Pwr_HandleWaterDetectEvent(void)
{
    uint16_t water_mv = Pwr_ReadWaterVoltageMv();

    PRINT("[WATER] PA12=%u mV\r\n", water_mv);

    if(water_mv < WATER_SHORT_THRESHOLD_MV)
    {
        water_clear_count = 0;
        if(water_short_count < WATER_SHORT_CONFIRM_COUNT)
        {
            water_short_count++;
        }

        if((water_short_count >= WATER_SHORT_CONFIRM_COUNT) && (water_short_active == 0))
        {
            water_short_active = 1;
            water_flash_enable = 1;
            Pwr_UpdateLedFlashEnable();
        }
    }
    else
    {
        water_short_count = 0;
        if(water_clear_count < WATER_CLEAR_CONFIRM_COUNT)
        {
            water_clear_count++;
        }

        if((water_clear_count >= WATER_CLEAR_CONFIRM_COUNT) && (water_short_active != 0))
        {
            water_short_active = 0;
            water_flash_enable = 0;
            Pwr_UpdateLedFlashEnable();
        }
    }
}
static uint8_t Pwr_IsGnssFixed(void)
{
    return (GGA.fix_quality != 0) ? 1 : 0;
}

static void Pwr_FormatCoord(char *buf, float value)
{
    uint8_t negative = 0;
    uint32_t scaled;
    uint32_t whole;
    uint32_t frac;

    if(value < 0)
    {
        negative = 1;
        value = -value;
    }

    scaled = (uint32_t)(value * 1000000.0f + 0.5f);
    whole = scaled / 1000000;
    frac = scaled % 1000000;

    if(negative)
    {
        sprintf(buf, "-%lu.%06lu", (unsigned long)whole, (unsigned long)frac);
    }
    else
    {
        sprintf(buf, "%lu.%06lu", (unsigned long)whole, (unsigned long)frac);
    }
}

static uint8_t Pwr_QueueSosMessage(void)
{
    char lat[16] = {0};
    char lon[16] = {0};
    char payload[RDSS_MSG_PAYLOAD_MAX + 1] = {0};
    uint32_t dest_card;
    uint16_t len;

    if(RD_txflag != 0)
    {
        return 0;
    }

    dest_card = strtoul((char *)DestIC, NULL, 10);
    if(dest_card == 0)
    {
        Pwr_StopSosAlarm(SOS_EXIT_NO_CONTACT);
        return 0;
    }

    Pwr_FormatCoord(lat, GGA.latitude);
    Pwr_FormatCoord(lon, GGA.longitude);
    sprintf(payload, "SOS,LAT=%s,LON=%s,HR=0,O2=0,STEP=0,KCAL=0", lat, lon);
    len = Rdss_SanitizePayloadLen((uint8_t *)payload, strlen(payload));

    Msg_tx.lf = SOS_MSG_LF;
    Msg_tx.encode = SOS_MSG_ENCODE;
    Msg_tx.generation = SOS_MSG_GENERATION;
    Msg_tx.reservied = 0;
    Msg_tx.heart_rate = 0;
    Msg_tx.blood_oxygen = 0;
    Msg_tx.foot_step = 0;
    Msg_tx.kcal = 0;
    Msg_tx.dest_card = dest_card;
    Msg_tx.payload_len = len;
    memset(Msg_tx.payload, 0, sizeof(Msg_tx.payload));
    tmos_memcpy(Msg_tx.payload, payload, len);

    Pwr_EnableRdssForSend();
    RD_txflag = TRUE;
    sos_send_count++;
    PRINT("[SOS] queue send %d/%d dest=%lu payload=%s\r\n",
          sos_send_count, SOS_SEND_MAX_COUNT, (unsigned long)dest_card, payload);

    if(sos_send_count >= SOS_SEND_MAX_COUNT)
    {
        Pwr_StopSosAlarm(SOS_EXIT_SEND_MAX);
    }

    return 1;
}

static void Pwr_ScheduleLocationReport(void)
{
    if(location_report_interval_sec != 0)
    {
        tmos_start_task(Pwr_TaskID, location_report_evt, MS1_TO_SYSTEM_TIME(location_report_interval_sec * 1000UL));
    }
}

static uint8_t Pwr_QueueLocationMessage(void)
{
    char lat[16] = {0};
    char lon[16] = {0};
    char payload[RDSS_MSG_PAYLOAD_MAX + 1] = {0};
    uint32_t dest_card;
    uint16_t len;

    if(RD_txflag != 0)
    {
        PRINT("[LOC] wait previous RDSS tx\r\n");
        return 0;
    }

    if(!Pwr_IsGnssFixed())
    {
        PRINT("[LOC] skip, not fixed\r\n");
        return 0;
    }

    if(frequency_count_down != 0)
    {
        PRINT("[LOC] wait frequency countdown=%u\r\n", frequency_count_down);
        return 0;
    }

    dest_card = strtoul((char *)DestIC, NULL, 10);
    if(dest_card == 0)
    {
        PRINT("[LOC] skip, no dest card\r\n");
        return 0;
    }

    Pwr_FormatCoord(lat, GGA.latitude);
    Pwr_FormatCoord(lon, GGA.longitude);
    sprintf(payload, "LOC,LAT=%s,LON=%s,HR=0,O2=0,STEP=0,KCAL=0", lat, lon);
    len = Rdss_SanitizePayloadLen((uint8_t *)payload, strlen(payload));

    Msg_tx.lf = LOCATION_MSG_LF;
    Msg_tx.encode = LOCATION_MSG_ENCODE;
    Msg_tx.generation = LOCATION_MSG_GENERATION;
    Msg_tx.reservied = 0;
    Msg_tx.heart_rate = 0;
    Msg_tx.blood_oxygen = 0;
    Msg_tx.foot_step = 0;
    Msg_tx.kcal = 0;
    Msg_tx.dest_card = dest_card;
    Msg_tx.payload_len = len;
    memset(Msg_tx.payload, 0, sizeof(Msg_tx.payload));
    tmos_memcpy(Msg_tx.payload, payload, len);

    Pwr_EnableRdssForSend();
    RD_txflag = TRUE;
    PRINT("[LOC] queue send dest=%lu payload=%s\r\n", (unsigned long)dest_card, payload);
    return 1;
}

void Pwr_SetLocationReportInterval(uint32_t interval_sec)
{
    location_report_interval_sec = interval_sec;
    tmos_stop_task(Pwr_TaskID, location_report_evt);

    PRINT("[LOC] timer interval=%lu sec\r\n", (unsigned long)location_report_interval_sec);
    Pwr_ScheduleLocationReport();
}

void Pwr_RequestLocationReport(void)
{
    Pwr_QueueLocationMessage();
}
static void Pwr_StartSosAlarm(void)
{
    uint32_t sos_contact = strtoul((char *)DestIC, NULL, 10);

    if((soft_power_off != 0) || (sos_state != SOS_STATE_IDLE))
    {
        return;
    }

    PRINT("[SOS] start check DestIC=%s contact=%lu local=%lu\r\n",
          DestIC, (unsigned long)sos_contact, (unsigned long)Card_info.cardid);

    if(sos_contact == 0)
    {
        SOS_SW_Flag = FALSE;
        Pwr_ClearVoiceQueue();
        Pwr_QueueVoiceText("无法发出SOS，请设置联系人");
        PRINT("[SOS] start fail, no contact card\r\n");
        return;
    }

    Pwr_ClearVoiceQueue();
    sos_send_count = 0;
    sos_wait_fix_count = 0;
    sos_reply_received = 0;
    sos_state = SOS_STATE_WAIT_FIX;
    SOS_SW_Flag = TRUE;
    sos_flash_enable = 1;
    Pwr_UpdateLedFlashEnable();

    Pwr_QueueVoiceText("SOS报警");
    PRINT("[SOS] start, wait fix\r\n");
    tmos_start_task(Pwr_TaskID, sos_alarm_evt, 1);
}

static void Pwr_StopSosAlarm(uint8_t reason)
{
    if(sos_state == SOS_STATE_IDLE)
    {
        return;
    }

    sos_state = SOS_STATE_IDLE;
    sos_send_count = 0;
    sos_wait_fix_count = 0;
    sos_reply_received = 0;
    SOS_SW_Flag = FALSE;
    sos_flash_enable = 0;
    Pwr_UpdateLedFlashEnable();
    tmos_stop_task(Pwr_TaskID, sos_alarm_evt);
    if(reason != SOS_EXIT_REPLY)
    {
        Pwr_ClearVoiceQueue();
    }

    if(reason == SOS_EXIT_NO_FIX)
    {
        Pwr_QueueVoiceText("未定位，无法发出SOS，请更换位置");
    }
    else if(reason == SOS_EXIT_SEND_MAX)
    {
        Pwr_QueueVoiceText("无法发出SOS，请更换位置");
    }
    else if(reason == SOS_EXIT_NO_CONTACT)
    {
        Pwr_QueueVoiceText("无法发出SOS，请设置联系人");
    }
    else if((reason == SOS_EXIT_LONG_PRESS) || (reason == SOS_EXIT_REPLY))
    {
        Pwr_QueueVoiceText("退出SOS");
    }

    PRINT("[SOS] stop reason=%d\r\n", reason);
}

static void Pwr_HandleKey1Event(void)
{
    uint8_t keys = HalKeyRead();

    if(key1_irq_pending)
    {
        key1_irq_pending = 0;
        if(key1_checking == 0)
        {
            key1_checking = 1;
            key1_long_done = 0;
            key1_hold_count = 0;
        }
    }

    if((key1_checking == 0) && (keys & HAL_KEY_SW_1))
    {
        key1_checking = 1;
        key1_long_done = 0;
        key1_hold_count = 0;
    }

    if(key1_checking)
    {
        if(keys & HAL_KEY_SW_1)
        {
            if(key1_hold_count < KEY1_LONG_PRESS_COUNT)
            {
                key1_hold_count++;
            }

            if((key1_hold_count >= KEY1_LONG_PRESS_COUNT) && (key1_long_done == 0))
            {
                key1_long_done = 1;
                if(sos_state == SOS_STATE_IDLE)
                {
                    Pwr_StartSosAlarm();
                }
                else
                {
                    Pwr_StopSosAlarm(SOS_EXIT_LONG_PRESS);
                }
            }
        }
        else
        {
            if(key1_long_done == 0)
            {
                Pwr_RequestBatteryVoice();
            }
            key1_checking = 0;
            key1_long_done = 0;
            key1_hold_count = 0;
        }
    }

    tmos_start_task(Pwr_TaskID, key1_evt, KEY1_SCAN_PERIOD);
}

static void Pwr_HandleSosAlarmEvent(void)
{
    char lat[16] = {0};
    char lon[16] = {0};

    if(sos_state == SOS_STATE_IDLE)
    {
        return;
    }

    if(sos_reply_received)
    {
        Pwr_StopSosAlarm(SOS_EXIT_REPLY);
        return;
    }

    if(SOS_SW_Flag == FALSE)
    {
        Pwr_StopSosAlarm(SOS_EXIT_LONG_PRESS);
        return;
    }

    if(sos_state == SOS_STATE_WAIT_FIX)
    {
        if(Pwr_IsGnssFixed())
        {
            sos_state = SOS_STATE_SENDING;
            sos_wait_fix_count = 0;
            Pwr_FormatCoord(lat, GGA.latitude);
            Pwr_FormatCoord(lon, GGA.longitude);
            PRINT("[SOS] fix ok lat=%s lon=%s\r\n", lat, lon);
        }
        else
        {
            if(sos_wait_fix_count < SOS_WAIT_FIX_MAX_COUNT)
            {
                sos_wait_fix_count++;
            }

            if(sos_wait_fix_count >= SOS_WAIT_FIX_MAX_COUNT)
            {
                Pwr_StopSosAlarm(SOS_EXIT_NO_FIX);
                return;
            }

            tmos_start_task(Pwr_TaskID, sos_alarm_evt, SOS_ALARM_PERIOD);
            return;
        }
    }

    if(sos_state == SOS_STATE_SENDING)
    {
        if(frequency_count_down == 0)
        {
            Pwr_QueueSosMessage();
        }
        else
        {
            PRINT("[SOS] wait frequency countdown=%u\r\n", frequency_count_down);
        }
    }

    if(sos_state != SOS_STATE_IDLE)
    {
        tmos_start_task(Pwr_TaskID, sos_alarm_evt, SOS_ALARM_PERIOD);
    }
}

void OPENAUDIO(void)
{
	GPIOB_SetBits(GPIO_Pin_16);
}

void CLOSEAUDIO(void)
{
	GPIOB_ResetBits(GPIO_Pin_16);
}

void OPENRD(void)
{
	GPIOB_SetBits(GPIO_Pin_3);// +3.7V_DM_EN
    GPIOB_SetBits(GPIO_Pin_18);// +3.3V_DM_EN
}

void Pwr_EnableRdssForSend(void)
{
    RD_SW_Flag = TRUE;
    OPENRD();
}

void CLOSERD(void)
{
	GPIOB_ResetBits(GPIO_Pin_3);
    GPIOB_ResetBits(GPIO_Pin_18);
}

void OPENRN(void)
{
	GPIOB_SetBits(GPIO_Pin_1);
    GPIOA_SetBits(GPIO_Pin_15);
}

void CLOSERN(void)
{
	GPIOB_ResetBits(GPIO_Pin_1);
    GPIOA_ResetBits(GPIO_Pin_15);
}


void BQ25601_Init(void)
{
    GPIOB_ModeCfg(GPIO_Pin_12 | GPIO_Pin_13, GPIO_ModeIN_PU);
    //PRINT("IIC Host mode\r\n");
    I2C_Init(I2C_Mode_I2C, 400000, I2C_DutyCycle_16_9, I2C_Ack_Enable, I2C_AckAddr_7bit, MASTER_ADDR);
    DelayMs(100);

    bq25601_drv->Init();
}

void BATTERY_ADC_Init(void)
{
    /* 单通道采样：选择adc通道4做采样，对应 PA14引脚， 带数据校准功能 */
    GPIOA_ModeCfg( GPIO_Pin_14, GPIO_ModeIN_Floating);
    DelayMs(300);
    ADC_ExtSingleChSampInit(SampleFreq_4_or_2, ADC_PGA_0);
 
    RoughCalib_Value = ADC_DataCalib_Rough(); // 用于计算ADC内部偏差，记录到全局变量 RoughCalib_Value中
    //PRINT("RoughCalib_Value =%d \n", RoughCalib_Value);
     
    ADC_ChannelCfg(4);
}

uint8_t BATTERY_ADC(void)
{
    uint32_t countadc = 0;
    uint16_t min_number = 0;
    uint16_t max_number = 0;
    uint32_t voltage = 0;
    uint16_t adcBuff[20];
    uint8_t percentage;
    uint16_t i = 0;

    ADC_ChannelCfg(4);

    for(i = 0; i < 20; i++)
    {
        adcBuff[i] = ADC_ExcutSingleConver() + RoughCalib_Value; // 连续采样20次
    }
    //printf("original: ");
    // for(i = 0; i < 20; i++)
    // {
    //     PRINT("%d ", adcBuff[i]); // 注意：由于ADC内部偏差的存在，当采样电压在所选增益范围极限附近的时候，可能会出现数据溢出的现象
    // }
    //printf("\r\n");
    countadc = 0;
    for(i = 0; i < 20; i++)
    {
        countadc += adcBuff[i];
        if(i == 0){
            min_number = adcBuff[i];
            max_number = adcBuff[i];
        }
        min_number = ((min_number > adcBuff[i]) ? adcBuff[i] : min_number);    //软件滤波
        max_number = ((max_number < adcBuff[i]) ? adcBuff[i] : max_number);
    }
    //printf("min=%d, max=%d, diff=%d\r\n", min_number, max_number, (max_number-min_number));
    //printf("countaveradc = %d\r\n", countadc);
    countadc = (countadc - min_number - max_number) / 18;                        //删除最小与最大值
    //printf("countaveradc = %d\r\n", countadc);
    voltage = (uint32_t)(countadc/(2.048)*1.05*3);//VREF (1.05f),分压系数3
    //printf("voltage=%d mv\r\n", voltage);

    percentage = (uint8_t)(((float)voltage/1000 - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE )*100);
    //printf("percentage=%d  percent\r\n", percentage);

    return percentage;
}

static void SoftPowerOff(void)
{
    if(soft_power_off != 0)
    {
        return;
    }

    soft_power_off = 1;
    Pwr_ClearVoiceQueue();
    voice_bat_last_req_time = 0;
    key1_irq_pending = 0;
    key1_checking = 0;
    key1_long_done = 0;
    key1_hold_count = 0;
    key2_checking = 0;
    key2_long_done = 0;
    key2_hold_count = 0;
    key2_wait_release = 0;
    key2_release_count = 0;
    gnss_last_fixed = 0;
    Pwr_StopSosAlarm(SOS_EXIT_POWER);
    location_report_interval_sec = 0;
    tmos_stop_task(Pwr_TaskID, location_report_evt);
    tmos_stop_task(Pwr_TaskID, water_detect_evt);
    SOS_SW_Flag = FALSE;

    Peripheral_BleOff();

    manual_flash_enable = 0;
    sos_flash_enable = 0;
    water_flash_enable = 0;
    water_short_active = 0;
    water_short_count = 0;
    water_clear_count = 0;
    Pwr_UpdateLedFlashEnable();
    GPIOA_ResetBits(GPIO_Pin_13);

    OPENAUDIO();
    DelayMs(200);
    Audio_play("关机", sizeof"关机");
    DelayMs(1500);

    CLOSERN();
    CLOSERD();

    tmos_stop_task(Pwr_TaskID, pwr_evt);
    tmos_stop_task(Pwr_TaskID, sos_evt);
    tmos_stop_task(Bat_TaskID, bat_evt);
}

void Pwr_RequestAutoPowerOff(void)
{
    if(soft_power_off == 0)
    {
        tmos_start_task(Pwr_TaskID, auto_poweroff_evt, 1);
    }
}
static void SoftPowerOn(void)
{
    if(soft_power_off == 0)
    {
        return;
    }

    soft_power_off = 0;
    Pwr_ClearVoiceQueue();
    voice_bat_last_req_time = 0;
    key1_irq_pending = 0;
    key1_checking = 0;
    key1_long_done = 0;
    key1_hold_count = 0;
    key2_checking = 0;
    key2_long_done = 0;
    key2_hold_count = 0;
    key2_wait_release = 0;
    key2_release_count = 0;
    gnss_last_fixed = 0;
    location_report_interval_sec = 0;
    tmos_stop_task(Pwr_TaskID, location_report_evt);
    tmos_stop_task(Pwr_TaskID, water_detect_evt);
    SOS_SW_Flag = FALSE;
    manual_flash_enable = 0;
    sos_flash_enable = 0;
    water_flash_enable = 0;
    water_short_active = 0;
    water_short_count = 0;
    water_clear_count = 0;
    Pwr_UpdateLedFlashEnable();

    RN_SW_Flag = FALSE;
    RD_SW_Flag = FALSE;
    CLOSERN();
    CLOSERD();
    OPENAUDIO();
    DelayMs(500);
    Pwr_RequestVoiceText("开机");

    Peripheral_BleOn();

    Pwr_WaterDetectInit();
    tmos_start_task(Pwr_TaskID, pwr_evt, 1600);
    tmos_start_task(Pwr_TaskID, key1_evt, KEY1_SCAN_PERIOD);
    tmos_start_task(Pwr_TaskID, water_detect_evt, WATER_DETECT_PERIOD);
    tmos_start_task(Bat_TaskID, bat_evt, 1600);
}

//PWR ADC采集事件
uint16_t Pwr_ProcessEvent(uint8_t task_id, uint16_t events)
{
    //  VOID task_id; // TMOS required parameter that isn't used in this function
    char voice_text[VOICE_TEXT_MAX] = {0};


    if(events & pwr_evt)
    {
        //增加蓝牙控制RNSS电源、GNSS电源控制开关
        if(RN_SW_Flag == TRUE)
            OPENRN();
        else
            CLOSERN();

        if(RD_SW_Flag == TRUE)
            OPENRD();
        else
            CLOSERD();

        //测试用
        // OPENRN();
        // OPENRD();
        //DelayMs(500);
        UART0_SendString((uint8_t *)strCCPTL, strlen((char *)strCCPTL));

        // if(RD_TEST_Flag == TRUE)//RDSS过检开关
        // 

        // if(RD_RST_Flag == TRUE)//RDSS复位开关
        // 

        tmos_start_task(Pwr_TaskID,pwr_evt,1600);
        // return unprocessed events
        return (events ^ pwr_evt);
    }

    if(events & sos_evt)
    {
        voice_playing = 0;

        #if VOICE_LOOP_TEST
        Audio_play("测试语音", sizeof"测试语音");
        #else
        if((SOS_SW_Flag == TRUE) && (sos_state == SOS_STATE_IDLE))
        {
            Pwr_StartSosAlarm();
        }
        else if((SOS_SW_Flag == FALSE) && (sos_state != SOS_STATE_IDLE))
        {
            Pwr_StopSosAlarm(SOS_EXIT_LONG_PRESS);
        }

        if(Pwr_DequeueVoiceText(voice_text))
        {
            OPENAUDIO();
            VOICE_DEBUG_PRINT("[VOICE] play %s\r\n", voice_text);
            Audio_play((uint8_t *)voice_text, strlen(voice_text));
            voice_playing = 1;

            tmos_start_task(Pwr_TaskID,sos_evt,VOICE_PLAY_GAP);
            return (events ^ sos_evt);
        }
        #endif

        tmos_start_task(Pwr_TaskID,sos_evt,1600);//tmosTimer具体是 1600 = 1s
        return (events ^ sos_evt);
    }

    if(events & key1_evt)
    {
        Pwr_HandleKey1Event();
        return (events ^ key1_evt);
    }

    if(events & sos_alarm_evt)
    {
        Pwr_HandleSosAlarmEvent();
        return (events ^ sos_alarm_evt);
    }

    if(events & water_detect_evt)
    {
        if(soft_power_off == 0)
        {
            Pwr_HandleWaterDetectEvent();
            tmos_start_task(Pwr_TaskID,water_detect_evt,WATER_DETECT_PERIOD);
        }
        return (events ^ water_detect_evt);
    }

    if(events & location_report_evt)
    {
        if((soft_power_off == 0) && (location_report_interval_sec != 0))
        {
            Pwr_QueueLocationMessage();
            Pwr_ScheduleLocationReport();
        }
        return (events ^ location_report_evt);
    }

    if(events & auto_poweroff_evt)
    {
        if(soft_power_off == 0)
        {
            SoftPowerOff();
        }
        return (events ^ auto_poweroff_evt);
    }
    if(events & pwroff_evt)
    {
        if(key2_wait_release)
        {
            PWR_SW_Flag = FALSE;
            key2_checking = 0;
            key2_long_done = 0;
            key2_hold_count = 0;

            if(HalKeyRead() == HAL_KEY_SW_2)
            {
                key2_release_count = 0;
            }
            else
            {
                if(key2_release_count < KEY2_RELEASE_STABLE_COUNT)
                {
                    key2_release_count++;
                }

                if(key2_release_count >= KEY2_RELEASE_STABLE_COUNT)
                {
                    key2_wait_release = 0;
                    key2_release_count = 0;
                }
            }
        }
        else
        {
            if(PWR_SW_Flag == TRUE)
            {
                PWR_SW_Flag = FALSE;
                key2_checking = 1;
                key2_long_done = 0;
                key2_hold_count = 0;
            }

            if(key2_checking)
            {
                if(HalKeyRead() == HAL_KEY_SW_2)
                {
                    if(key2_hold_count < KEY2_LONG_PRESS_COUNT)
                    {
                        key2_hold_count++;
                    }

                    if((key2_hold_count >= KEY2_LONG_PRESS_COUNT) && (key2_long_done == 0))
                    {
                        key2_long_done = 1;
                        key2_checking = 0;
                        key2_hold_count = 0;

                        if(soft_power_off == 0)
                        {
                            SoftPowerOff();
                        }
                        else
                        {
                            SoftPowerOn();
                        }

                        key2_wait_release = 1;
                        key2_release_count = 0;
                        PWR_SW_Flag = FALSE;
                    }
                }
                else
                {
                    if(key2_long_done == 0)
                    {
                        if(soft_power_off == 0)
                        {
                            manual_flash_enable = !manual_flash_enable;
                            Pwr_UpdateLedFlashEnable();
                        }
                    }

                    key2_checking = 0;
                    key2_hold_count = 0;
                    key2_long_done = 0;
                }
            }
        }

        if(led_flash_enable)
        {
            UpdateLedFlash();
        }
        // switch(PWR_SW_cnt)
        // {
        //     case 0x01: 
        //                 HalLedOnOff(HAL_LED_1, HAL_LED_MODE_ON);
        //                 HalLedOnOff(HAL_LED_2|HAL_LED_3|HAL_LED_4, HAL_LED_MODE_OFF);
        //                 break;
        //     case 0x02: 
        //                 HalLedOnOff(HAL_LED_4, HAL_LED_MODE_ON);
        //                 HalLedOnOff(HAL_LED_1|HAL_LED_2|HAL_LED_3, HAL_LED_MODE_OFF);
        //                 break;
        //     case 0x03: 
        //                 HalLedOnOff(HAL_LED_2, HAL_LED_MODE_ON);
        //                 HalLedOnOff(HAL_LED_1|HAL_LED_3|HAL_LED_4, HAL_LED_MODE_OFF);
        //                 break;
        //     case 0x04: 
        //                 HalLedOnOff(HAL_LED_3, HAL_LED_MODE_ON);
        //                 HalLedOnOff(HAL_LED_1|HAL_LED_2|HAL_LED_4, HAL_LED_MODE_OFF);
        //                 break;
        //     default:
        //                 //HalLedOnOff(HAL_LED_1|HAL_LED_2|HAL_LED_3|HAL_LED_4, HAL_LED_MODE_OFF);
        //                 break;
        // }       

//             if(HalKeyRead() == HAL_KEY_SW_2)
//             {
//                 key_timer_cnt ++ ;//稳定后
//                  DelayMs(100) ;//稳定后播放
//                 if((HalKeyRead() == HAL_KEY_SW_2) && (key_timer_cnt >= 3))
//                 {
//                     key_timer_cnt = 0;
//                     Audio_play("关机", sizeof"关机");
//                     // 执行关机操作
// //                    Audio_play("关机", sizeof"关机");

//                     // 执行关机操作
//                     CLOSERN();  // 关闭RNSS电源
//                     CLOSERD();  // 关闭GNSS电源
//                     HalLedOnOff(HAL_LED_1|HAL_LED_2|HAL_LED_3|HAL_LED_4, HAL_LED_MODE_OFF);  // 关闭所有LED

//                     // 停止所有任务
//                     tmos_stop_task(Pwr_TaskID, pwr_evt);
//                     tmos_stop_task(Pwr_TaskID, sos_evt);
//                     tmos_stop_task(Pwr_TaskID, pwroff_evt);
//                     tmos_stop_task(Bat_TaskID, bat_evt);

//                     // 进入低功耗模式
//                     LowPower_Shutdown(0);
//                 }
//             }
            // else
            // {
            //     // 按键松开时清零计数器
            //     key_timer_cnt = 0;

            // }

        tmos_start_task(Pwr_TaskID,pwroff_evt,100);//tmosTimer具体是 1600 = 1s
        return (events ^ pwroff_evt);
    }
    // Discard unknown events
    return 0;
}

//初始化，注册Task
void Pwr_init(void)
{
    Pwr_TaskID = TMOS_ProcessEventRegister(Pwr_ProcessEvent);//注册任务，返回task_id

    if(HalKeyRead() == HAL_KEY_SW_2)
    {
        PWR_SW_Flag = FALSE;
        key2_checking = 0;
        key2_long_done = 0;
        key2_hold_count = 0;
        key2_wait_release = 1;
        key2_release_count = 0;
    }

    Pwr_WaterDetectInit();
    Pwr_RequestVoiceText("开机");
    tmos_start_task(Pwr_TaskID,pwr_evt,1600);//开始任务
    tmos_start_task(Pwr_TaskID,key1_evt,KEY1_SCAN_PERIOD);//KEY1短按/长按检测
    tmos_start_task(Pwr_TaskID,water_detect_evt,WATER_DETECT_PERIOD);
    tmos_start_task(Pwr_TaskID,pwroff_evt,1600);//开始任务
}

//BAT ADC采集事件
uint16_t Bat_ProcessEvent(uint8_t task_id, uint16_t events)
{
    //  VOID task_id; // TMOS required parameter that isn't used in this function
    

    if(events & bat_evt)
    {
        Bat_percentage = BATTERY_ADC();

        tmos_start_task(Bat_TaskID,bat_evt,1600*45);//tmosTimer具体是 1600 = 1s，45s采集间隔
        return (events ^ bat_evt);
    }
   
    // Discard unknown events
    return 0;
}

void Bat_init(void)
{
    Bat_TaskID = TMOS_ProcessEventRegister(Bat_ProcessEvent);//注册任务，返回task_id
    tmos_start_task(Bat_TaskID,bat_evt,1600);//开始任务
}

//状态机控制LED闪烁
static void UpdateLedFlash(void)
{
    HalLedOnOff(HAL_LED_ALL, HAL_LED_MODE_OFF);

    if(led_index == 0)
    {
        HalLedOnOff(HAL_LED_1, HAL_LED_MODE_ON);
        led_index = 1;
    }
    else
    {
        HalLedOnOff(HAL_LED_4, HAL_LED_MODE_ON);
        led_index = 0;
    }
}

/******************************** endfile @ key ******************************/
