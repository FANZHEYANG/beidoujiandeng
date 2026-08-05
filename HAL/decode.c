#include "CH58x_common.h"
#include "CH58x_uart.h"
#include "decode.h"	
#include "stdio.h"	
#include "string.h"
#include "HAL.h"
#if SOS_TEST_ONLY_PRINT
#undef PRINT
#define PRINT(...)
#define printf(...)
#endif


uint16_t point0 = 0;
uint16_t point1 = 0;
uint16_t gga_point1 = 0;
static volatile uint32_t gnss_rx_byte_count = 0;
static volatile uint32_t gnss_dollar_count = 0;
static volatile uint32_t gnss_lf_count = 0;
static volatile uint32_t gnss_cr_count = 0;
static uint8_t gnss_raw_ring[48];
static uint8_t gnss_raw_idx = 0;
static volatile uint32_t gnss_mute_until = 0;
static uint8_t gnss_echo_warned = 0;

static void gnss_mute_rx_ms(uint32_t ms)
{
    gnss_mute_until = TMOS_GetSystemClock() + MS1_TO_SYSTEM_TIME(ms);
}

static uint8_t gnss_rx_muted(void)
{
    return (TMOS_GetSystemClock() < gnss_mute_until) ? 1 : 0;
}

static uint8_t gnss_ring_has_echo(void)
{
    uint8_t i;

    for(i = 0; i < 46; i++)
    {
        if(gnss_raw_ring[i] == 'p' && gnss_raw_ring[(i + 1) & 47] == '1' &&
           gnss_raw_ring[(i + 2) & 47] == '=')
            return 1;
        if(gnss_raw_ring[i] == 'h' && gnss_raw_ring[(i + 1) & 47] == 'a' &&
           gnss_raw_ring[(i + 2) & 47] == 'd')
            return 1;
    }
    return 0;
}
uint8_t  RD_rxflag = 0;
uint8_t  RD_txflag = 0;
uint8_t  RD_tx_ack_dirty = 0;
uint8_t  RD_msg_rx_dirty = 0;
uint16_t RD_realsize = 0;

_GGA GGA;
_SaveData Save_Data;
_Model_info Model_info;
_Card_info Card_info;

_GSVDataSet GSVDataSet;
gps_data_save_t Save_GSV_Data;
uint16_t point2 = 0;
uint16_t gsv_point2 = 0;
uint8_t  USART_RX_BUF1_GSV[USART_REC_LEN_GSV]; 

_Tx_ack  Tx_ack;
_Msg_tx Msg_tx;
_Msg_rx Msg_rx;

uint8_t snr[12]={0};//倒序排序
unsigned int frequency_count_down=0;

uint8_t LocalIC[16]="0000000";//本设备的北斗卡号
uint8_t DestIC [16]="0000000";//目标设备的北斗卡号
const char strCCVER[]="$CCVER,0,0*41\r\n";//读取版本信息
const char strCCSIM[]="$CCSIM,0,0*57\r\n";//读取北三卡信息
const char strCCPTL[]="$CCPTL,2*56\r\n";//设置为扩展协议
const char strCCTTC[]="$CCTTC,0*5F\r\n";//频度倒计时查询

uint8_t *p1,*RD_result;
const uint8_t delims[]={',', '*'};

uint8_t USART_RX_BUF0[USART0_REC_LEN];    //接收缓冲,最大USART0_REC_LEN个字节.
uint8_t USART_RX_BUF1[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态

#define RD_FRAME_QUEUE_SIZE 4
static uint8_t RD_FRAME_QUEUE[RD_FRAME_QUEUE_SIZE][USART0_REC_LEN];
static uint16_t RD_FRAME_LEN[RD_FRAME_QUEUE_SIZE];
static volatile uint8_t RD_FRAME_WRITE = 0;
static volatile uint8_t RD_FRAME_READ = 0;
static volatile uint8_t RD_FRAME_COUNT = 0;
static uint8_t RD_PARSE_BUF[USART0_REC_LEN];

static uint8_t RNSS_TaskID; // Task ID for RNSS processing
static uint8_t RDSS_TaskID; // Task ID for RDSS processing

// 计算NMEA校验和
unsigned char calculate_checksum(const char *sentence) {
    unsigned char checksum = 0;
    // 从$后一个字符开始计算到*前一个字符
    for (int i = 1; sentence[i] && sentence[i] != '*'; i++) {
        checksum ^= sentence[i];
    }
    return checksum;
}

static void PrintCardInfoToUART2(void)
{
    PRINT("\r\n[RDSS CardInfo]\r\n");
    PRINT("cardid=%lu\r\n", (unsigned long)Card_info.cardid);
    PRINT("usertype=%d\r\n", Card_info.usertype);
    PRINT("encryped=%d\r\n", Card_info.encryped);
    PRINT("regional_service_freq=%d\r\n", Card_info.regional_service_freq);
    PRINT("regional_comm_grade=%d\r\n", Card_info.regional_comm_grade);
    PRINT("global_service_freq=%d\r\n", Card_info.global_service_freq);
    PRINT("global_comm_grade=%d\r\n", Card_info.global_comm_grade);
    PRINT("subusernumber=%u\r\n", Card_info.subusernumber);
    PRINT("cardtype=%d\r\n", Card_info.cardtype);
    PRINT("[RDSS CardInfo End]\r\n");
}

// 验证NMEA校验和
int validate_checksum(const char *sentence) {
    // 查找校验和开始位置
    const char *checksum_start = strrchr(sentence, '*');
    if (!checksum_start) {
        return 0; // 没有找到校验和
    }
    
    // 提取提供的校验和
    char checksum_str[3];
    strncpy(checksum_str, checksum_start + 1, 2);
    checksum_str[2] = '\0';
    
    unsigned char provided_checksum;
    char *endptr;
    provided_checksum = (unsigned char)strtol(checksum_str, &endptr, 16);
    
    if (*endptr != '\0') {
        return 0; // 校验和不是有效的十六进制数
    }
    
    // 计算实际校验和
    unsigned char actual_checksum = calculate_checksum(sentence);
    
    return (actual_checksum == provided_checksum);
}  

// 检查字段是否为空（仅包含空格或制表符）
int is_empty_field(const char *field) {
    if (!field) return 1;
    while (*field) {
        if (!isspace(*field)) {
            return 0;
        }
        field++;
    }
    return 1;
}

// GSV语句解析函数
int parse_gbgsv(const char* sentence, satellite_info_t* satellites, int max_satellites) {
    // 验证校验和
    if (!validate_checksum(sentence)) {
        printf("GSV checksum warn\r\n");
    }

 // 查找数据部分开始位置（跳过$GBGSV或$GPGSV）
    const char *ptr = strchr(sentence, ',');
    if (!ptr) {
        printf("无效的语句格式\n");
        return -1;
    }
    ptr++; // 跳过第一个逗号
    
    // 手动分割字段
    int field_count = 0;
    const char *field_start = ptr;
    const char *field_end = ptr;
    const char *fields[50]; // 存储字段起始位置
    int field_lengths[50];  // 存储字段长度
    
    while (*ptr && field_count < 50) {
        
        if (*ptr == ',' || *ptr == '*') {
            fields[field_count] = field_start;
            field_lengths[field_count] = (int)(ptr - field_start);
            field_count++;
            
            if (*ptr == '*') break; // 遇到校验和结束
            
            // 准备下一个字段
            ptr++;
            field_start = ptr;
            field_end = ptr;
            continue;
        }
        field_end = ptr;
        ptr++;
     }
    
    if (field_count  < 3) {
        printf("字段数量不足\n");
        return -1;
    }
    
    // 解析基本信息
    char temp[32];
    strncpy(temp, fields[0], field_lengths[0]);
    temp[field_lengths[0]] = '\0';
    GSVDataSet.total_messages = atoi(temp);// 总消息数
    
    strncpy(temp, fields[1], field_lengths[1]);
    temp[field_lengths[1]] = '\0';
    GSVDataSet.current_message = atoi(temp); // 当前消息编号
    
    strncpy(temp, fields[2], field_lengths[2]);
    temp[field_lengths[2]] = '\0';
    GSVDataSet.satellites_in_view = atoi(temp);// 可见卫星总数
    
    // printf("总消息数: %d\n", GSVDataSet.total_messages);
    // printf("当前消息编号: %d\n", GSVDataSet.current_message);
    // printf("可见卫星总数: %d\n", GSVDataSet.satellites_in_view);
    // printf("\r\n ");

    // 验证卫星数量是否合理
    if (GSVDataSet.satellites_in_view < 0 ) {
        printf("卫星数量不合理，放弃解析\n");
        return -1;
    }
    
    // 解析卫星信息
    int satellite_count = 0;
    // 从第3个字段开始，每4个字段描述一颗卫星
    for (int i = 3; (i + 3 < field_count ) && (satellite_count < max_satellites); i =i + 4) {
        // 解析PRN
        strncpy(temp, fields[i], field_lengths[i]);
        temp[field_lengths[i]] = '\0';
        satellites[satellite_count].prn = (temp[0] != '\0') ? atoi(temp) : 0;
        //printf("prn:%d\n",satellites[satellite_count].prn);
        // 解析仰角
        strncpy(temp, fields[i+1], field_lengths[i+1]);
        temp[field_lengths[i+1]] = '\0';
        satellites[satellite_count].elevation = (temp[0] != '\0') ? atoi(temp) : 0;
        //printf("elevation:%d\n",satellites[satellite_count].elevation);
        // 解析方位角
        strncpy(temp, fields[i+2], field_lengths[i+2]);
        temp[field_lengths[i+2]] = '\0';
        satellites[satellite_count].azimuth = (temp[0] != '\0') ? atoi(temp) : 0;
        //printf("azimuth:%d\n",satellites[satellite_count].azimuth);
        // 解析信噪比
        strncpy(temp, fields[i+3], field_lengths[i+3]);
        temp[field_lengths[i+3]] = '\0';
        satellites[satellite_count].snr = (temp[0] != '\0') ? atoi(temp) : 0;
        //printf(" SNR:%d\n",satellites[satellite_count].snr);

        satellite_count++;
        // printf("\r\n ");
    }
   
    return satellite_count;
}

// 字符转换数字
int fast_atoi(const char* str) {
    if (str == NULL) return 0;
    
    int result = 0;
    int sign = 1;
    int i = 0;
    
    // 处理符号
    if (str[0] == '-') {
        sign = -1;
        i = 1;
    } else if (str[0] == '+') {
        i = 1;
    }
    
    // 转换数字
    for (; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0');
        } else {
            break; // 遇到非数字字符停止
        }
    }
    
    return result * sign;
}

// 从GSV语句中提取总消息数
int extract_total_messages(const char* gbgsv_line) {
    const char* ptr = gbgsv_line;
    
    // 跳过$GBGSV前缀
    while (*ptr && *ptr != '$') ptr++;
    if (*ptr == '$') ptr++;
    
    // 跳过语句标识符
    while (*ptr && *ptr != ',') ptr++;
    if (*ptr == ',') ptr++;
    
    // 解析总语句数
    int total_messages = fast_atoi(ptr);
    return total_messages;
}

// 处理完整的GSV数据集
static void sort_gnss_snr_desc(void)
{
    uint8_t i, j;
    satellite_info_t temp;

    for(i = 0; i < Save_GSV_Data.satellite_count; i++)
    {
        for(j = 0; j < Save_GSV_Data.satellite_count - i - 1; j++)
        {
            if(Save_GSV_Data.satellites[j].snr < Save_GSV_Data.satellites[j + 1].snr)
            {
                temp = Save_GSV_Data.satellites[j];
                Save_GSV_Data.satellites[j] = Save_GSV_Data.satellites[j + 1];
                Save_GSV_Data.satellites[j + 1] = temp;
            }
        }
    }
}

void process_complete_gsv_data() {
    int i, j;
    satellite_info_t temp_satellites[MAX_SATELLITES];
    int new_count = 0;

    for (i = 0; i < Save_GSV_Data.gsv_count; i++) {
        satellite_info_t parsed[4];
        int count = parse_gbgsv((char *)Save_GSV_Data.GSV_Buffers[i], parsed, 4);

        if(count > 0)
        {
            for (j = 0; j < count && new_count < MAX_SATELLITES; j++) {
                temp_satellites[new_count] = parsed[j];
                new_count++;
            }
        }
    }

    if(new_count > 0)
    {
        Save_GSV_Data.satellite_count = new_count;
        for(i = 0; i < new_count; i++)
        {
            Save_GSV_Data.satellites[i] = temp_satellites[i];
        }
        sort_gnss_snr_desc();

        PRINT("\r\n[GNSS GSV] count=%d", Save_GSV_Data.satellite_count);
        for(i = 0; i < 12 && i < Save_GSV_Data.satellite_count; i++)
        {
            PRINT(" %d", Save_GSV_Data.satellites[i].snr);
        }
        PRINT("\r\n");
    }
    else
    {
        PRINT("\r\n[GNSS GSV] parse failed, keep count=%d\r\n", Save_GSV_Data.satellite_count);
    }
}

static void gnss_raw_push(uint8_t b)
{
    if(gnss_rx_muted())
        return;

    gnss_raw_ring[gnss_raw_idx++ & 47] = b;
    if(b == '$')
        gnss_dollar_count++;
    else if(b == '\n')
        gnss_lf_count++;
    else if(b == '\r')
        gnss_cr_count++;
}

static void gnss_save_gga_frame(uint16_t len)
{
    uint16_t i;

    if(len < 8 || USART_RX_BUF1[0] != '$')
        return;
    USART_RX_BUF1[len] = '\0';
    if(strstr((char *)USART_RX_BUF1, "GGA") == NULL)
        return;

    if(len >= GPS_Buffer_Length)
        len = GPS_Buffer_Length - 1;

    memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);
    for(i = 0; i < len; i++)
        Save_Data.GPS_Buffer[i] = USART_RX_BUF1[i];
    Save_Data.isGetData = true;
    gga_point1 = len;
}

static void gnss_save_gsv_frame(uint16_t len)
{
    int total_gsv_messages;
    uint16_t i;

    if(len < 8 || USART_RX_BUF1_GSV[0] != '$')
        return;
    USART_RX_BUF1_GSV[len] = '\0';
    if(strstr((char *)USART_RX_BUF1_GSV, "GSV") == NULL)
        return;

    if(Save_GSV_Data.gsv_count >= 10)
        return;

    if(len >= USART_REC_LEN_GSV)
        len = USART_REC_LEN_GSV - 1;

    memset(Save_GSV_Data.GSV_Buffers[Save_GSV_Data.gsv_count], 0, USART_REC_LEN_GSV);
    for(i = 0; i < len; i++)
        Save_GSV_Data.GSV_Buffers[Save_GSV_Data.gsv_count][i] = USART_RX_BUF1_GSV[i];

    if(Save_GSV_Data.gsv_count == 0)
    {
        total_gsv_messages = extract_total_messages((char *)Save_GSV_Data.GSV_Buffers[Save_GSV_Data.gsv_count]);
        if(total_gsv_messages > 0)
            Save_GSV_Data.total_gsv_messages = total_gsv_messages;
    }

    Save_GSV_Data.gsv_count++;
    if(Save_GSV_Data.total_gsv_messages > 0 &&
       Save_GSV_Data.gsv_count >= Save_GSV_Data.total_gsv_messages)
        Save_GSV_Data.isGSVComplete = true;
}

static void gnss_uart_rx_byte(uint8_t Res)
{
    if(gnss_rx_muted())
        return;

    gnss_raw_push(Res);
    gnss_rx_byte_count++;

    if(Res == '$')
        point1 = 0;
    if(point1 < (USART_REC_LEN - 1))
        USART_RX_BUF1[point1++] = Res;

    if((Res == '\n' || Res == '\r') && point1 >= 8)
    {
        uint16_t save_len = point1;
        while(save_len > 0 &&
              (USART_RX_BUF1[save_len - 1] == '\r' || USART_RX_BUF1[save_len - 1] == '\n'))
            save_len--;

        gnss_save_gga_frame(save_len);
        point1 = 0;
    }

    if(point1 >= USART_REC_LEN)
        point1 = USART_REC_LEN - 1;

    if(Res == '$')
        point2 = 0;
    if(point2 < (USART_REC_LEN_GSV - 1))
        USART_RX_BUF1_GSV[point2++] = Res;

    if((Res == '\n' || Res == '\r') && point2 >= 8)
    {
        uint16_t save_len = point2;
        while(save_len > 0 &&
              (USART_RX_BUF1_GSV[save_len - 1] == '\r' ||
               USART_RX_BUF1_GSV[save_len - 1] == '\n'))
            save_len--;

        gnss_save_gsv_frame(save_len);
        point2 = 0;
    }

    if(point2 >= USART_REC_LEN_GSV)
        point2 = USART_REC_LEN_GSV - 1;
}

static void gnss_poll_uart1(void)
{
    while(R8_UART1_RFC)
    {
        gnss_uart_rx_byte(UART1_RecvByte());
    }
}

/*********************************************************************
 * @fn      UART0_IRQHandler
 *
 * @brief   UART0中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART0_IRQHandler(void)
{
    uint8_t Res;
	//uint8_t Read_flag = UART0_GetITFlag();   // 只读一次，保存在局部变量

    switch(UART0_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART0_GetLinSTA();
            break;
        }

        case UART_II_RECV_RDY: // 数据达到设置触发点
            Res =UART0_RecvByte();	//读取接收到的数据
            //UART2_SendByte(Res);

            if(Res == '$')
            {
                point0 = 0;
                memset(USART_RX_BUF0, 0, USART0_REC_LEN);
            }
            if (point0 < USART0_REC_LEN-1) // 防止超界
            {
                USART_RX_BUF0[point0 ++] = Res;
            }
            else
            {
                point0 = 0;   // 超了丢掉
            }


            // if(Res == '\n')
            // {
            //     USART_RX_BUF0[point0] = '\0';
            //     RD_realsize = point0;
            //     point0 = 0;
            //     RD_rxflag = TRUE;
            // }
            if(Res == '\r' || Res == '\n')
            {
                if(point0 > 1 && USART_RX_BUF0[0] == '$')
                {
                    USART_RX_BUF0[point0] = '\0';
                    RD_realsize = point0;

                    if(RD_FRAME_COUNT < RD_FRAME_QUEUE_SIZE)
                    {
                        tmos_memcpy(RD_FRAME_QUEUE[RD_FRAME_WRITE], USART_RX_BUF0, point0 + 1);
                        RD_FRAME_LEN[RD_FRAME_WRITE] = point0;
                        RD_FRAME_WRITE++;
                        if(RD_FRAME_WRITE >= RD_FRAME_QUEUE_SIZE)
                        {
                            RD_FRAME_WRITE = 0;
                        }
                        RD_FRAME_COUNT++;
                        RD_rxflag = TRUE;
                    }
                }

                point0 = 0;
            }

//            if (point0 > 630)//读取RDSS数据包
//             {
//                 RD_realsize = point0;
//				 point0 =0;
//                 RD_rxflag = TRUE;     // 通知主循环
//             }

            break;

        case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成
            Res = UART0_RecvString(USART_RX_BUF0);//不能删除，清中断


	        break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }
}
  

/*********************************************************************
 * @fn      UART1_IRQHandler
 *
 * @brief   UART1中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART1_IRQHandler(void)
{
    switch(UART1_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART1_GetLinSTA();
            break;
        }

        case UART_II_RECV_RDY: // 数据达到设置触发点
        case UART_II_RECV_TOUT: // 接收超时：剩余字节也送入拼帧逻辑，勿写入 USART_RX_BUF1
            while(R8_UART1_RFC)
            {
                gnss_uart_rx_byte(UART1_RecvByte());
            }
            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            break;

        default:
            break;
    }
}


uint8_t Hand(uint8_t *a)                   // 串口命令识别函数
{ 
    if(strstr(USART_RX_BUF1,a)!=NULL)
	    return 1;
	else
		return 0;
}

void CLR_Buf(void)                           // 串口缓存清理
{
	memset(USART_RX_BUF1, 0, USART_REC_LEN);      //清空
  point1 = 0;                    
}

void CLR_Buf_GSV(void)                           // 串口缓存清理
{
	memset(USART_RX_BUF1_GSV, 0, USART_REC_LEN_GSV);      //清空                
}

static void clearGpsParsedFields(void)
{
	memset(Save_Data.UTCTime, 0, UTCTime_Length);
	memset(Save_Data.latitude, 0, latitude_Length);
	memset(Save_Data.N_S, 0, N_S_Length);
	memset(Save_Data.longitude, 0, longitude_Length);
	memset(Save_Data.E_W, 0, E_W_Length);
	memset(Save_Data.satellites_tracked, 0, satellites_tracked_Length);
	memset(Save_Data.hdop, 0, hdop_Length);
	memset(Save_Data.altitude, 0, altitude_Length);
	memset(Save_Data.height, 0, height_Length);
	memset(Save_Data.dgps_age, 0, dgps_age_Length);
}

static void clearGgaPositionFields(void)
{
	GGA.latitude = 0.0f;
	GGA.longitude = 0.0f;
	GGA.satellites_tracked = 0;
	GGA.hdop = 0.0f;
	GGA.altitude = 0.0f;
	GGA.height = 0.0f;
	GGA.dgps_age = 0.0f;
}

static void gnssCopyField(uint8_t *dst, uint8_t dst_len, const char *start, const char *end)
{
	uint16_t len;

	if(dst == NULL || start == NULL || end == NULL || dst_len == 0 || end < start)
	{
		return;
	}

	len = (uint16_t)(end - start);
	if(len >= dst_len)
	{
		len = dst_len - 1;
	}

	memset(dst, 0, dst_len);
	if(len > 0)
	{
		memcpy(dst, start, len);
	}
}

void clrStruct()
{
	Save_Data.isGetData = false;
	Save_Data.isParseData = false;
	Save_Data.isUsefull = false;
	memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //清空
	clearGpsParsedFields();
}

// 解析 GGA → GGA 结构体
void parseGpsBuffer()
{
	char *subString;
	char *subStringNext;
	char i = 0;

	if (Save_Data.isGetData)
	{
		Save_Data.isGetData = false;
		Save_Data.isParseData = false;
		Save_Data.isUsefull = false;
		GGA.fix_quality = 0;
        gga_point1 = 0;
		clearGpsParsedFields();
		
		for (i = 0 ; i <= 13 ; i++)
		{
			if (i == 0)
			{
				if ((subString = strstr((char *)Save_Data.GPS_Buffer, ",")) == NULL)
					printf("ERROR\r\n");	//解析错误
			}
			else
			{
				subString++;
				subStringNext = strstr(subString, ",");
				if (subStringNext != NULL || i == 13)
				{
					char *field_end = subStringNext;

					if(i == 13 && field_end == NULL)
					{
						field_end = strchr(subString, '*');
					}
					if(field_end == NULL)
					{
						printf("ERROR\r\n");
						continue;
					}
					switch(i)
					{
						case 1:gnssCopyField(Save_Data.UTCTime, UTCTime_Length, subString, field_end);break;
						case 2:gnssCopyField(Save_Data.latitude, latitude_Length, subString, field_end);break;
						case 3:gnssCopyField(Save_Data.N_S, N_S_Length, subString, field_end);break;
						case 4:gnssCopyField(Save_Data.longitude, longitude_Length, subString, field_end);break;
						case 5:gnssCopyField(Save_Data.E_W, E_W_Length, subString, field_end);break;
						case 6:
						{
							uint8_t usefullBuffer[4] = {0};

							gnssCopyField(usefullBuffer, sizeof(usefullBuffer), subString, field_end);
							GGA.fix_quality = atoi((char *)usefullBuffer);
							Save_Data.isUsefull = (GGA.fix_quality > 0) ? true : false;
							break;
						}
						case 7:gnssCopyField(Save_Data.satellites_tracked, satellites_tracked_Length, subString, field_end);break;
						case 8:gnssCopyField(Save_Data.hdop, hdop_Length, subString, field_end);break;
						case 9:gnssCopyField(Save_Data.altitude, altitude_Length, subString, field_end);break;
						case 10:break;
						case 11:gnssCopyField(Save_Data.height, height_Length, subString, field_end);break;
						case 12:break;
						case 13:gnssCopyField(Save_Data.dgps_age, dgps_age_Length, subString, field_end);break;
						default:break;
					}

					if(subStringNext != NULL)
					{
						subString = subStringNext;
					}
					Save_Data.isParseData = true;
				}
				else
				{
					printf("ERROR\r\n");	//解析错误
				}
			}

		}
	}
}

// 字符串转 float/int，填充 GGA
void printGpsBuffer()
{
	if (Save_Data.isParseData)
	{
		Save_Data.isParseData = false;

		GGA.hour = atoi(Save_Data.UTCTime) / 10000;
		GGA.minute = (atoi(Save_Data.UTCTime) % 10000) / 100;
		GGA.second = atoi(Save_Data.UTCTime) % 100;
		GGA.microseconds = (uint32_t)(atof(Save_Data.UTCTime) * 1000) % 1000;
        // printf("%d\r\n", GGA.hour);
		// printf("%d\r\n", GGA.minute);
		// printf("%d\r\n", GGA.second);
		// printf("%d\r\n", GGA.microseconds);

		if(Save_Data.isUsefull)
		{
			if(Save_Data.N_S[0] == 'N')
				GGA.latitude = atoi(Save_Data.latitude) / 100 + (atof(Save_Data.latitude) / 100 - atoi(Save_Data.latitude) / 100) * 100.0 / 60.0;
			else if(Save_Data.N_S[0] == 'S')
				GGA.latitude = -(atoi(Save_Data.latitude) / 100 + (atof(Save_Data.latitude) / 100 - atoi(Save_Data.latitude) / 100) * 100.0 / 60.0);
			if(Save_Data.E_W[0] == 'E')	
				GGA.longitude = atoi(Save_Data.longitude) / 100 + (atof(Save_Data.longitude) / 100 - atoi(Save_Data.longitude) / 100) * 100.0 / 60.0;
			else if(Save_Data.E_W[0] == 'W')
				GGA.longitude = -(atoi(Save_Data.longitude) / 100 + (atof(Save_Data.longitude) / 100 - atoi(Save_Data.longitude) / 100) * 100.0 / 60.0);
			GGA.satellites_tracked = atoi(Save_Data.satellites_tracked);
			GGA.hdop = atof(Save_Data.hdop);
			GGA.altitude = atof(Save_Data.altitude);
			GGA.height = atof(Save_Data.height);
			GGA.dgps_age = atof(Save_Data.dgps_age);
		}
		else
		{
			clearGgaPositionFields();
		}

		PRINT("\r\n[GNSS GGA] fix=%d lat=%f lon=%f sats=%d\r\n",
		       GGA.fix_quality, GGA.latitude, GGA.longitude, GGA.satellites_tracked);
        Pwr_OnGnssFixUpdate((uint8_t)GGA.fix_quality);
	}
}

//RNSS定位信息采集事件
uint16_t RNSS_ProcessEvent(uint8_t task_id, uint16_t events)
{
    //  VOID task_id; // TMOS required parameter that isn't used in this function
    
    if(events & rnss_evt)
    {
        uint8_t had_gga;

        if(RN_SW_Flag == TRUE)
        {
            OPENRN();
        }

        gnss_poll_uart1();
        had_gga = Save_Data.isGetData;
        parseGpsBuffer();
		printGpsBuffer();

        gnss_mute_rx_ms(80);
        PRINT("[GNSS] rx=%lu had_gga=%d gsv=%d/%d sat=%d fix=%d buf=%u\r\n",
              (unsigned long)gnss_rx_byte_count,
              (int)had_gga,
              Save_GSV_Data.gsv_count,
              Save_GSV_Data.total_gsv_messages,
              Save_GSV_Data.satellite_count,
              GGA.fix_quality,
              (unsigned int)point1);

        if(had_gga == 0 && gnss_ring_has_echo() && !gnss_echo_warned)
        {
            gnss_echo_warned = 1;
            PRINT("[GNSS WARN] UART1 RX got debug echo, not CM112B NMEA. Check PA8 wiring.\r\n");
        }
        else if(had_gga == 0 && !gnss_ring_has_echo())
        {
            uint8_t i;
            uint8_t base = gnss_raw_idx;
            PRINT("[GNSS RAW] ");
            for(i = 0; i < 24; i++)
                PRINT("%02X ", gnss_raw_ring[(base - 24 + i) & 47]);
            PRINT("\r\n");
        }
        gnss_mute_rx_ms(80);

        //GSV解析
		if(Save_GSV_Data.isGSVComplete == true)
		{
			Save_GSV_Data.isGSVComplete =false;
         
            // printf("\r\n***** \r\n");
            // for(int i=0;i<Save_GSV_Data.total_gsv_messages;i++)
            // {
            //     for(int k=0;k<USART_REC_LEN_GSV;k++)
            //         printf("%c",Save_GSV_Data.GSV_Buffers[i][k]);
            // }
            // printf("*****\r\n");

            process_complete_gsv_data();
            CLR_Buf_GSV();
            Save_GSV_Data.total_gsv_messages = 0;  // 重置总消息数
            Save_GSV_Data.gsv_count = 0;  // 重置计数器

            // for(int i=0;i<GSVDataSet.satellites_in_view;i++)
            // {
            //     printf("SNR(%d):%d  \r\n",i,Save_GSV_Data.satellites[i].snr);
            // }

		}

        tmos_start_task(RNSS_TaskID,rnss_evt,1600);//tmosTimer具体是 1600 = 1s，1s采集间隔
        return (events ^ rnss_evt);
    }
   
    // Discard unknown events
    return 0;
}

uint16_t Rdss_SanitizePayloadLen(const uint8_t *payload, uint16_t len)
{
    uint16_t high_count = 0;
    int16_t index;

    if(len > RDSS_MSG_PAYLOAD_MAX)
    {
        len = RDSS_MSG_PAYLOAD_MAX;
    }

    if(payload == NULL || len == 0)
    {
        return 0;
    }

    index = (int16_t)(len - 1);
    while(index >= 0 && (payload[index] & 0x80))
    {
        high_count++;
        index--;
    }

    if((high_count & 0x01) != 0)
    {
        len--;
    }

    return len;
}

static void Rdss_ClearMsgRx(void)
{
    memset(&Msg_rx, 0, sizeof(Msg_rx));
}
//RDSS短报文信息采集事件
uint16_t RDSS_ProcessEvent(uint8_t task_id, uint16_t events)
{
    //  VOID task_id; // TMOS required parameter that isn't used in this function
    uint8_t strCCMSG[200]={0};// 用于组装发送短报文的命令字符串
    uint8_t byte[3]={0},str_ic[8]={0},check[3]={0};//临时存储2字节数据，存储IC卡号，存储校验和
    uint8_t strstr_show[200]={0}, j=0, normal_flag, coding_flag;// 解码后的报文内容，循环变量和标志位
    uint8_t ICcard[16]={0};// 目标IC卡号

    unsigned int temp=0;// 临时变量
    uint16_t i=0;// 循环计数器
    
    if(events & rdss_evt)
    {
    
    //把COM口输入的数据通过北斗短报文发送出去，接收方默认是LocalIC
    //发送数据$CCMSG,1850000,3,2,78516D23313233*1F
	if (RD_txflag==1)
	{
		//RD_txflag = false;

		//组织CCMSG的语句内容
		strcpy(ICcard, DestIC);
		//sprintf(strCCTXA,"$CCTXA,%s,1,2,A4", ICcard);//混合编码，最大发送77字节的用户内容，因为A4占用了一个字节
		sprintf(strCCMSG,"$CCMSG,%s,%d,%d,", ICcard,Msg_tx.lf,Msg_tx.encode);//代码编码，最大发送78字节的用户内容
												
        // 只发送App写入0x4504的正文，不追加心率/血氧/步数/卡路里。
        Msg_tx.payload_len = Rdss_SanitizePayloadLen(Msg_tx.payload, (uint16_t)Msg_tx.payload_len);

        for(i = 0; i < Msg_tx.payload_len; i++)
        {
            sprintf(check, "%02X", Msg_tx.payload[i]);
            strcat(strCCMSG, check);
        }

        PRINT("\r\n[RDSS TX 4504] dest=%s lf=%d encode=%d payload_len=%lu\r\n",
              ICcard, Msg_tx.lf, Msg_tx.encode, (unsigned long)Msg_tx.payload_len);
        
		//生成异或校验字节，将$到*之间的所有字符串(不包括$和*)
		temp=0;
		for(i=1; i<strlen(strCCMSG);i++)
		{
			temp = temp ^ (int)strCCMSG[i];
		}
		sprintf(check,"%02X",(int)temp);//
				
		//组织CCTXA语句的最后部分
		strcat(strCCMSG,"*");
		strcat(strCCMSG,check);
		strcat(strCCMSG,"\r\n");
        PRINT("[RDSS TX 4504] cmd=%s", strCCMSG);

        RN_SW_Flag = FALSE;
        CLOSERN();


        HalLedOnOff(HAL_LED_ALL, HAL_LED_MODE_OFF);

        CLOSEAUDIO();
        
        DelayMs(2000);

		UART0_SendString((uint8_t *)strCCMSG, strlen((char *)strCCMSG));
		RD_txflag = false;
        RN_SW_Flag = TRUE;
        OPENRN();
				
    }


     	while (RD_FRAME_COUNT > 0)
	{
        uint16_t rd_frame_len;

        PFIC_DisableIRQ(UART0_IRQn);
        rd_frame_len = RD_FRAME_LEN[RD_FRAME_READ];
        tmos_memcpy(RD_PARSE_BUF, RD_FRAME_QUEUE[RD_FRAME_READ], rd_frame_len + 1);
        RD_FRAME_READ++;
        if(RD_FRAME_READ >= RD_FRAME_QUEUE_SIZE)
        {
            RD_FRAME_READ = 0;
        }
        RD_FRAME_COUNT--;
        if(RD_FRAME_COUNT == 0)
        {
            RD_rxflag = false;
        }
        PFIC_EnableIRQ(UART0_IRQn);

		//printf("[UART0 RX] %s\r\n", RD_PARSE_BUF);

		//设备版本号
		if(strstr((char *)RD_PARSE_BUF,"$BDVXX"))
		{
			p1=strstr((char *)RD_PARSE_BUF,"$BDVXX");
			RD_result=strtok(p1, delims);//$BDVXX
            RD_result=strtok(NULL, delims);//设备供应商
            strcpy(Model_info.vendor, RD_result);
			RD_result=strtok(NULL, delims);//设备类型
            strcpy(Model_info.type, RD_result);
			RD_result=strtok(NULL, delims);//硬件版本
            RD_result=strtok(NULL, delims);//软件版本
            strcpy(Model_info.version, RD_result);
		}

		//入站信息 	
        //接收到短报文，比如:$BDMXX,1850000,054650,2,1234567890ABCDEF*41\r\n 		
        else if(strstr((char *)RD_PARSE_BUF,"$BDMXX"))
        {
            Rdss_ClearMsgRx();
            p1=strstr((char *)RD_PARSE_BUF,"$BDMXX");
            RD_result=strtok(p1, delims);//$BDMXX
            RD_result=strtok(NULL, delims);//发信人地址ID
            if(RD_result != NULL)
            {
                strcpy(str_ic, RD_result);
                Msg_rx.sender = atoi(RD_result);
            }

            RD_result=strtok(NULL, delims);//发信时间，北京时间
            if(RD_result != NULL)
            {
                memset(byte, 0, sizeof(byte));
                strncpy(byte, RD_result, 2);
                Msg_rx.hour = atoi(byte);
                memset(byte, 0, sizeof(byte));
                strncpy(byte, RD_result + 2, 2);
                Msg_rx.minute = atoi(byte);
                memset(byte, 0, sizeof(byte));
                strncpy(byte, RD_result + 4, 2);
                Msg_rx.second = atoi(byte);
            }

            normal_flag=REALNORMAL;
            coding_flag=MIXCODING;

            RD_result=strtok(NULL, delims);//电文形式
            if(RD_result != NULL)
            {
                Msg_rx.encode = atoi(RD_result);
                if(strcmp(RD_result, "3")==0)
                    coding_flag=MIXCODING;
                else if((strcmp(RD_result, "2")==0) || (strcmp(RD_result, "1")==0))
                    coding_flag=OTHERCODING;
                else
                    normal_flag=REALERROR;
            }

            Msg_rx.mailtype = 0;

            if(normal_flag==REALNORMAL)
            {
                RD_result=strtok(NULL, delims);//电文内容
                if(RD_result != NULL)
                {
                    memset(strstr_show, 0, sizeof(strstr_show));
                    memset(Msg_rx.payload, 0, sizeof(Msg_rx.payload));
                    j=0;

                    for(i=coding_flag; (i + 1)<strlen(RD_result) && j<RDSS_MSG_PAYLOAD_MAX; i=i+2)
                    {
                        memset(byte, 0, sizeof(byte));
                        strncpy(byte, RD_result+i, 2);
                        sscanf(byte,"%02X",&temp);
                        strstr_show[j]=temp;
                        Msg_rx.payload[j]=strstr_show[j];
                        j++;
                    }
                    Msg_rx.payload_len = Rdss_SanitizePayloadLen(Msg_rx.payload, j);
                    RD_msg_rx_dirty = 1;

                    PRINT("\r\n[RDSS RX BDMXX] sender=%lu time=%02d:%02d:%02d encode=%d payload_len=%lu\r\n",
                          (unsigned long)Msg_rx.sender, Msg_rx.hour, Msg_rx.minute, Msg_rx.second,
                          Msg_rx.encode, (unsigned long)Msg_rx.payload_len);
                    PRINT("[RDSS RX BDMXX] payload_hex=");
                    for(i=0; i<Msg_rx.payload_len; i++)
                    {
                        PRINT("%02X ", Msg_rx.payload[i]);
                    }
                    PRINT("\r\n");
                    Pwr_OnRdssMessageReceived(Msg_rx.sender);
                }
            }
        }

        //卡信息
        else if(strstr((char *)RD_PARSE_BUF,"$BDSXX"))
        {
            p1=strstr((char *)RD_PARSE_BUF,"$BDSXX");
            RD_result=strtok(p1, delims);//$BDSXX

            RD_result = strtok(NULL, delims); // 用户地址ID
            if(RD_result != NULL)
            {
                strncpy((char *)LocalIC, (char *)RD_result, sizeof(LocalIC) - 1);
                LocalIC[sizeof(LocalIC) - 1] = '\0';
                Card_info.cardid = (uint32_t)strtoul((char *)RD_result, NULL, 10);
            }

            RD_result=strtok(NULL, delims);//通播地址
            RD_result=strtok(NULL, delims);//用户标识
            if(RD_result != NULL) Card_info.usertype = atoi(RD_result);
            RD_result=strtok(NULL, delims);//保密标识
            if(RD_result != NULL)
            {
                if(strcmp(RD_result,"E")==0 || strcmp(RD_result,"0E")==0)
                    Card_info.encryped = true;
                if(strcmp(RD_result,"N")==0 || strcmp(RD_result,"0N")==0)
                    Card_info.encryped = false;
            }
            RD_result=strtok(NULL, delims);//区域服务频度
            if(RD_result != NULL) Card_info.regional_service_freq = atoi(RD_result);
            RD_result=strtok(NULL, delims);//区域通信等级
            if(RD_result != NULL) Card_info.regional_comm_grade = atoi(RD_result);
            RD_result=strtok(NULL, delims);//全球服务频度
            if(RD_result != NULL) Card_info.global_service_freq = atoi(RD_result);
            RD_result=strtok(NULL, delims);//全球通信等级
            if(RD_result != NULL) Card_info.global_comm_grade = atoi(RD_result);
            RD_result=strtok(NULL, delims);//下属用户数
            if(RD_result != NULL) Card_info.subusernumber = atoi(RD_result);
            RD_result=strtok(NULL, delims);//卡类型
            if(RD_result != NULL) Card_info.cardtype = atoi(RD_result);
        }
		//FKI回复,反馈信息
		//$BDFKI,111626,LOC,Y,0,0000*44
		else if(strstr((char *)RD_PARSE_BUF,"$BDFKI"))//入站信息成功与否反馈
		{
			p1=strstr((char *)RD_PARSE_BUF,"$BDFKI");
			RD_result=strtok(p1, delims);//$BDFKI
			RD_result=strtok(NULL, delims);//指令执行时间：北京时
			RD_result=strtok(NULL, delims);//入站发射类型
			RD_result=strtok(NULL, delims);//发射情况 Y发射成功 N发射失败
			if(strcmp(RD_result,"Y")==0 ) 
			{
				Tx_ack.ack = true;
				Tx_ack.reason = 0;
			}
            if(strcmp(RD_result,"N")==0 ) 
			    Tx_ack.ack = false;
			RD_result=strtok(NULL, delims);//失败原因
            Tx_ack.reason = atoi(RD_result);
            RD_tx_ack_dirty = 1;
            PRINT("\r\n[RDSS ACK BDFKI] ack=%d reason=%d\r\n", Tx_ack.ack, Tx_ack.reason);
			RD_result=strtok(NULL, delims);//剩余时间
            if(RD_result != NULL)
            {
                frequency_count_down = atoi(RD_result);
            }
		}
		//查询模块发送频度倒计时,CCTTC
		//$BDTTC,19*61
		else if(strstr((char *)RD_PARSE_BUF,"$BDTTC"))//入站信息成功与否反馈
		{
			p1=strstr((char *)RD_PARSE_BUF,"$BDTTC");
			RD_result=strtok(p1, delims);//$BDTTC
			RD_result=strtok(NULL, delims);//区域频度倒计时
			
            frequency_count_down = atoi(RD_result);
			
		}

        //信噪比数组
		//$BDSNR,00,00,47,00,43,00,00,00,00,00,00,00,51,00,00,00,44,40,00,00,00*61
		else if(strstr((char *)RD_PARSE_BUF,"$BDSNR"))//默认1s收到1次
		{
			p1=strstr((char *)RD_PARSE_BUF,"$BDSNR");
			RD_result=strtok(p1, delims);//$BDSNR
            
			for(i=0; i<21; i++)
			{				
				RD_result=strtok(NULL, delims);
                
				sscanf(RD_result,"%d",&temp); //把字符串格式化为整数
				strstr_show[i]=temp;
			}

            //排序
//            for (i = 0; i < 20; i++) {
//             for (j = 0; j < 20-i; j++) {
//                if (strstr_show[j] > strstr_show[j+1]) {// 交换 arr[j] 和 arr[j+1]
//                    temp = strstr_show[j];
//                    strstr_show[j] = strstr_show[j+1];
//                    strstr_show[j+1] = temp;
//                }
//             }
//			}
			// 冒泡排序 (从大到小)
			for (i = 0; i < 20; i++) {
			    for (j = 0; j < 20-i; j++) {
			        if (strstr_show[j] < strstr_show[j+1]) {  // 改为 < 号
			            temp = strstr_show[j];
			            strstr_show[j] = strstr_show[j+1];
			            strstr_show[j+1] = temp;
			        }
			    }
			}
			for(i=0; i<12; i++)
			{
				snr[i] = strstr_show[i];
			}
//			printf("\r\n[RDSS SNR] ");
//			for(i=0; i<12; i++)
//			{
//				printf("%d ", snr[i]);
//			}
//			printf("\r\n");
        }
    }


        tmos_start_task(RDSS_TaskID,rdss_evt,1600);//tmosTimer具体是 1600 = 1s，1s采集间隔
        return (events ^ rdss_evt);
    }

	if(events & rdss_tx_evt)
    {
	    //PRINT("\r\n[UART0 TX] send CCSIM\r\n");
	    //PRINT("%s", strCCSIM);
	    UART0_SendString((uint8_t *)strCCSIM, strlen((char *)strCCSIM));//读取卡信息
//		UART0_SendString((uint8_t *)strCCSIM, sizeof(strCCSIM));
		//PRINT("[UART0 TX] CCSIM done\r\n");
		//PrintCardInfoToUART2();

        //？频度倒计时查询啥时候进行的条件需要明确一下，发送短报文时？
		UART0_SendString((uint8_t *)strCCTTC, strlen(strCCTTC));//频度倒计时查询

	    tmos_start_task(RDSS_TaskID,rdss_tx_evt,1600*15);//tmosTimer具体是 1600 = 1s，1s采集间隔
        return (events ^ rdss_tx_evt);
	}
   
    // Discard unknown events
    return 0;
}

void RNSS_init(void)
{
    RNSS_TaskID = TMOS_ProcessEventRegister(RNSS_ProcessEvent);//注册任务，返回task_id
    CLR_Buf();//清空缓存
    clrStruct();

	tmos_start_task(RNSS_TaskID,rnss_evt,1600);//开始任务
}

void RDSS_init(void)
{
    RDSS_TaskID = TMOS_ProcessEventRegister(RDSS_ProcessEvent);//注册任务，返回task_id

	tmos_start_task(RDSS_TaskID,rdss_tx_evt,1600*15);//开始任务
    tmos_start_task(RDSS_TaskID,rdss_evt,1600);//开始任务
}
