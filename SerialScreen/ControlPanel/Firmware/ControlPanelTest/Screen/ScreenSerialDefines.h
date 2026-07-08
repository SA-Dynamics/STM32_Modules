#ifndef SCREENSERIALDEFINES_H
#define SCREENSERIALDEFINES_H

#include "usart.h"

#define UART_PORT					huart1      // 用户串口
#define UART_DMA_RX_PORT			hdma_usart1_rx
#define UART_DMA_TX_PORT			hdma_usart1_tx
#define SEND_DATA_MAX_LEN           32          // 发送串口数据最大长度


#define EVENT_HANDLER_NUM_PER_WIDGET    4   // 每个widget事件可以绑定的回调数量
extern void CbScreenUartTxCpltHandler(void);
extern void CbUart1IdleHandler(void);


// 用户定义的串口屏读变量地址
#define	RD_HMI_BUTTON_POP_ADDR			0x220A
#define	RD_HMI_BUTTON_RELEASE_ADDR		0x3202


// 用户定义的串口屏写变量地址
#define WR_ROTATE_ICON_ADDR           	0x2100
#define WR_DATA_VAR_ADDR           		0x2102
#define WR_PROG_BAR1_ADDR           	0x2200
#define WR_PROG_BAR2_ADDR           	0x2202
#define WR_VAR_ICON1_ADDR           	0x2204
#define WR_VAR_ICON2_ADDR           	0x2206
#define WR_BIT_ICON_ADDR           		0x2208

#endif
