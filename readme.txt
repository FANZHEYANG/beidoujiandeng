6.9修改
开关机逻辑
开机：
芯片上电/复位
→ main()
→ 初始化时钟、LED、按键、GPIO、电源控制脚
→ 先把 RNSS/RDSS/音频相关电源脚拉低
→ 初始化 BLE、外设任务、GNSS/RDSS
→ 注册 Pwr 任务、Bat 任务
→ 进入 TMOS 主循环
→ Pwr 任务周期执行
→ 当前测试代码强制打开 RNSS 和 RDSS 电源

关机：
按下 KEY2
→ GPIOB 中断触发
→ PWR_SW_Flag = TRUE
→ PWR_SW_cnt 自增，用于 LED 状态切换
→ pwroff_evt 周期检测 KEY2 是否仍然按下
→ 连续检测到 KEY2 按下 3 次
→ 播放“关机”
→ 关闭 RNSS/RDSS 电源
→ 关闭所有 LED
→ 停止 PWR/BAT 相关任务
→ 调用 LowPower_Shutdown(0)


6.11修改

UART2 在这个工程里主要是调试日志串口，不是 GNSS/RDSS 的业务通信串口。

peripheral_main.c (line 63) 里只有在 #ifdef DEBUG 下初始化 UART2_DefInit()
并配置 PA7 为 TXD。

Peripheral.wvproj (line 161) 里定义了 DEBUG=2
编译结果显示 _write() 会把 printf/PRINT 输出到 UART2。

代码里没有看到 UART2_IRQHandler、UART2_RecvString、业务数据收发等使用，所以它基本只负责输出日志。
真正业务串口在 MCU.c (line 258)：UART0 接 DM229/RDSS，UART1 接北斗定位/RNSS，UART3 接语音播放。
UART2 的 RX 注释掉了，PA6 后面又被配置成 +3.3V_DM_EN 电源控制脚
所以当前 UART2 实际上只适合当调试输出 TX 用。调串口助手看日志的话，接 PA7，波特率默认 115200。

在新板中PA6改回RXD2，将DM_EN改到PB18