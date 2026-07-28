#ifndef __DECODE_H
#define __DECODE_H


//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式


#define USART_REC_LEN  			200  	//定义RNSS最大接收字节数 200
#define USART0_REC_LEN  		1000  	//定义RDSS最大接收字节数 1000
// 定义最大卫星数量和缓冲区长度
#define MAX_SATELLITES 20
#define USART_REC_LEN_GSV 100

extern uint8_t  USART_RX_BUF0[USART0_REC_LEN]; //接收缓冲,最大USART0_REC_LEN个字节 	  	
extern uint8_t  USART_RX_BUF1[USART_REC_LEN];  //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern uint8_t  USART_RX_BUF1_GSV[USART_REC_LEN_GSV];  //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
 //event
#define rnss_evt                  0x0001  
//#define rnss_rx_evt               0x0002  
#define rdss_evt                  0x0002  
#define rdss_tx_evt               0x0004

#define false 0
#define true 1

//定义数组长度
#define GPS_Buffer_Length 80
#define UTCTime_Length 11
#define latitude_Length 11
#define N_S_Length 2
#define longitude_Length 12
#define E_W_Length 2 
#define satellites_tracked_Length 3 
#define hdop_Length 5 
#define altitude_Length 8 
#define height_Length 8 
#define dgps_age_Length 4 

typedef struct SaveData 
{
	uint8_t GPS_Buffer[GPS_Buffer_Length];
	uint8_t isGetData;		//是否获取到GPS数据
	uint8_t isParseData;	//是否解析完成
	uint8_t UTCTime[UTCTime_Length];		//UTC时间
	uint8_t latitude[latitude_Length];		//纬度
	uint8_t N_S[N_S_Length];		//N/S
	uint8_t longitude[longitude_Length];		//经度
	uint8_t E_W[E_W_Length];		//E/W
	uint8_t isUsefull;		//定位信息是否有效
	uint8_t satellites_tracked[satellites_tracked_Length];
	uint8_t hdop[hdop_Length];
	uint8_t altitude[altitude_Length];
	uint8_t height[height_Length];
	uint8_t dgps_age[dgps_age_Length];
} _SaveData;

typedef struct gga{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	unsigned int microseconds;
	float latitude;//纬度,负数为南纬
	float longitude;//经度,负数为西经
	int fix_quality;//定位质量,1 表示定位成功
	int satellites_tracked;
	float hdop;
	float altitude; //海拔高度
	float height;
	float dgps_age;
}_GGA;

extern _GGA GGA;

typedef struct satellite_info{
    int prn;        // 卫星编号
    int elevation;  // 卫星仰角
    int azimuth;    // 卫星方位角
    uint8_t snr;        // 信噪比
} satellite_info_t;

// GSV 数据集管理结构
typedef struct {
    int total_messages;         // 总消息数
    int current_message;        // 当前接收的消息号
    int satellites_in_view;     // 可见卫星总数
} _GSVDataSet;

extern _GSVDataSet GSVDataSet;

// 保存数据结构体
typedef struct {
    char GPS_Buffer[USART_REC_LEN_GSV];
    char GSV_Buffers[10][USART_REC_LEN_GSV];  // 存储5个GBGSV语句
    int gsv_count;                       // 已接收的GSV语句数量
    uint8_t isGSVComplete;                  // GSV数据是否接收完整
	int total_gsv_messages;              // GSV语句总数
    satellite_info_t satellites[MAX_SATELLITES];
    int satellite_count;
} gps_data_save_t;

// 全局变量声明
extern uint16_t point2;
extern gps_data_save_t Save_GSV_Data;

//定义数组长度
#define RD_Buffer_Length 100
#define vendor_Length 10
#define type_Length 10
#define version_Length 32

//模块信息
typedef struct Model_info
{
	uint8_t vendor[vendor_Length];//供应商
	uint8_t type[type_Length];//设备类型
	uint8_t version[version_Length];//版本
} _Model_info;

//卡信息
typedef struct card_info {
	uint32_t cardid; // 用户地址
	uint8_t usertype; // 用户标识 1 北一,2 北二,3 北三
	uint8_t encryped; // 保密标识 E 加密，N非密  
	uint8_t regional_service_freq;//区域服务频度
	uint8_t regional_comm_grade;//区域通信等级
	uint8_t global_service_freq;//全球服务频度
	uint8_t global_comm_grade;//全球通信等级
	unsigned short subusernumber;//下属用户数
	uint8_t cardtype; //卡类型
}_Card_info;

#define REALNORMAL     0
#define REALERROR      1
#define MIXCODING      2
#define OTHERCODING    0
//发送数据
typedef struct msg_tx{
	unsigned char lf;//入站频点
	unsigned char encode;//编码类型
	unsigned char generation;//北二或北三
	unsigned char reservied;
	unsigned char heart_rate;//心率
	unsigned char blood_oxygen;//血氧
	unsigned int foot_step;//步数
	unsigned int kcal;//卡路里
	unsigned int dest_card;//收信方卡号
	unsigned int payload_len;//数据长度
	unsigned char payload[70];//通信数据，发送报文信息
}_Msg_tx;

//接收数据，入站消息
typedef struct msg_rx{
	unsigned int sender;//发信人ID
	unsigned char hour;//发信时间
	unsigned char minute;
	unsigned char second;
	unsigned char encode;//编码方式
	unsigned int mailtype;//邮件类型，信息类别
	unsigned int payload_len;//数据长度
	unsigned char payload[512];//通信数据，收到报文信息
}_Msg_rx;

//反馈信息
typedef struct tx_ack{
	uint8_t ack;
	unsigned char reason;//0-正常, 1-频率未到,2-发射抑制,3-发射静默,4-功率未锁定,5-未检测到IC模块信息,6-权限不足
	unsigned char _unused[6];//保留
}_Tx_ack;

extern uint8_t DestIC [16];//目标设备的北斗卡号
extern const char strCCVER[16];//读取版本信息
extern const char strCCSIM[16];//读取北三卡信息
extern const char strCCPTL[14];//设置为扩展协议

extern uint8_t RD_rxflag,RD_txflag,RD_tx_ack_dirty;
extern uint16_t RD_realsize;

extern uint8_t rxdatabufer;
extern uint16_t point0,point1;

extern _SaveData Save_Data;
extern _Model_info Model_info;
extern _Card_info Card_info;
extern _Tx_ack Tx_ack;
extern _Msg_tx Msg_tx;
extern _Msg_rx Msg_rx;

extern uint8_t snr[12];//倒序排序

extern unsigned int frequency_count_down;

void CLR_Buf(void);
uint8_t Hand(uint8_t *a);
void clrStruct(void);

//初始化，注册task
extern void RNSS_init(void);
extern void RDSS_init(void);

#endif


