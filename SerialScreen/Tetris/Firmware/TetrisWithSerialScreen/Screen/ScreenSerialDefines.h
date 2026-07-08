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
#define	RD_HMI_BUTTON_DIR_ADDR			0x2000
#define	RD_HMI_BUTTON_DOWN_ADDR			0x2004
#define RD_HMI_BUTTON_STATE_ADDR		0x2002

// 用户定义的串口屏写变量地址
#define WR_CUBE_START_ADDR           	0x1100





#endif
