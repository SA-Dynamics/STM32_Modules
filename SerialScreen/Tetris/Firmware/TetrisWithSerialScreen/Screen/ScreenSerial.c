#include "ScreenSerial.h"
#include "ScreenSerialDefines.h"
#include <string.h>
#include "Timer.h"


#define UART_DATA_BUFFER_SIZE       512         // 串口数据buffer大小
#define PARSE_FUNC_TYPE_NUM         8           // 接收数据校验函数指针数组大小

// 串口发送完成和接受完成标志
static bool g_bSendFinish = false;
static bool g_bRecvFrameFinish = false;


// 接收数据判断函数
void IsCmdRespond(void);
void IsDataReturn(void);

// 串口数据管理
static struct 
{
    uint8_t u8UartBuffer[UART_DATA_BUFFER_SIZE];
    uint16_t u16WriteIndex;
    uint16_t u16ReadIndex;
    uint8_t u8Len;
    void (*pParseFunc[PARSE_FUNC_TYPE_NUM])(void);
	uint32_t u32CmdRespond;
}g_sUartDataManager = 
{
	.pParseFunc = {IsCmdRespond, IsDataReturn},
};


/**
  * @brief  接收数据响应后的事件处理回调, 由外部实现
  * @param  None
  * @retval None
  */
__weak void CbRecvEvent(const ScreenEvent *pEvent)
{
	
}


/**
  * @brief  判断接收到的数据是否是指令响应
  * @param  None
  * @retval None
  */
void IsCmdRespond(void)
{
    uint8_t u8DataTemp[3] = {0x82, 0x4F, 0x4B};

    // 校验数据长度和数据内容
    if (3 == g_sUartDataManager.u8Len && 
        0x82 == g_sUartDataManager.u8UartBuffer[g_sUartDataManager.u16ReadIndex] &&
        0x4F == g_sUartDataManager.u8UartBuffer[(g_sUartDataManager.u16ReadIndex + 1) % UART_DATA_BUFFER_SIZE] && 
        0x4B == g_sUartDataManager.u8UartBuffer[(g_sUartDataManager.u16ReadIndex + 2) % UART_DATA_BUFFER_SIZE])
    {
        // 数据响应
        g_sUartDataManager.u32CmdRespond++;
    }
}


/**
  * @brief  判断接收到的数据是否是数据返回
  * @param  None
  * @retval None
  */
void IsDataReturn(void)
{
    bool bRet = false;

    if (g_sUartDataManager.u8Len > 4)
    {
        if (0x83 == g_sUartDataManager.u8UartBuffer[g_sUartDataManager.u16ReadIndex])
        {
            ScreenEvent sEvent;
            sEvent.u16VarAddr = 
                ((uint16_t)g_sUartDataManager.u8UartBuffer[(g_sUartDataManager.u16ReadIndex + 1) % UART_DATA_BUFFER_SIZE] << 8) | 
                g_sUartDataManager.u8UartBuffer[(g_sUartDataManager.u16ReadIndex + 2) % UART_DATA_BUFFER_SIZE];
            sEvent.u8Len = g_sUartDataManager.u8Len - 4; // 减去传输方向和地址字节

            if (sEvent.u8Len <= EVENT_DATA_MAX_LEN)
            {
                for (uint8_t i = 0; i < sEvent.u8Len; i++)
                {
                    sEvent.u8Data[i] = g_sUartDataManager.u8UartBuffer[(g_sUartDataManager.u16ReadIndex + 4 + i) % UART_DATA_BUFFER_SIZE];
                }
                
                CbRecvEvent(&sEvent);       
            }
        }
    }
}


/**
  * @brief  调用接收数据判断函数, 解析串口接收到的数据
  * @param  None
  * @retval None
  */
void ParseUartData(void)
{
    if (g_sUartDataManager.u8Len > 2)
    {
        // 在解析函数列表中解析数据
        for (uint8_t i = 0; i < PARSE_FUNC_TYPE_NUM; i++)
        {
            if (g_sUartDataManager.pParseFunc[i])
            {
                g_sUartDataManager.pParseFunc[i]();
            }
            else
            {
                break;
            }
        }
    }
}


/**
  * @brief  判断接收buffer中是否有数据, 并提取出有效数据
  * @param  None
  * @retval None
  */
void UartBufferHandler(void)
{
    static enum
    {
		CHECK_FRAME_RECV = 0,
        UPDATE_WRITE_INDEX,
        FIND_FRAME_HEAD1,
        FIND_FRAME_HEAD2,
        GET_DATA_LEN,
        PARSE_DATA,
    }FrameHandlerStep = CHECK_FRAME_RECV;
    static uint8_t u8DataLen = 0;

    switch (FrameHandlerStep)
    {
		case CHECK_FRAME_RECV:
            // 首先判断接收完成标志
			if (g_bRecvFrameFinish)
			{
				FrameHandlerStep = UPDATE_WRITE_INDEX;
				g_bRecvFrameFinish = false;
			}
			break;

        case UPDATE_WRITE_INDEX:
            g_sUartDataManager.u16WriteIndex = 
                UART_DATA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&UART_DMA_RX_PORT);
            
            if (g_sUartDataManager.u16WriteIndex != g_sUartDataManager.u16ReadIndex)
            {
                FrameHandlerStep = FIND_FRAME_HEAD1;
            }
            else
            {
                FrameHandlerStep = CHECK_FRAME_RECV;
            }
            break;
		
        case FIND_FRAME_HEAD1:
            // 判断帧头1
            if (0x5A == g_sUartDataManager.u8UartBuffer[g_sUartDataManager.u16ReadIndex])
            {                
                FrameHandlerStep = FIND_FRAME_HEAD2;
            }
            else
            {
                // 如果没有找到帧头1, 就重新等待接收
                if (g_sUartDataManager.u16WriteIndex == g_sUartDataManager.u16ReadIndex)
                {
                    FrameHandlerStep = CHECK_FRAME_RECV;
                    break;
                }
            }
            g_sUartDataManager.u16ReadIndex = (g_sUartDataManager.u16ReadIndex + 1) % UART_DATA_BUFFER_SIZE;
            break;

        case FIND_FRAME_HEAD2:
            // 判断帧头2           
            if (g_sUartDataManager.u16WriteIndex == g_sUartDataManager.u16ReadIndex)
            {
                // 如果没有数据了, 就重新等待接收
                FrameHandlerStep = CHECK_FRAME_RECV;
                break;
            }
            if (0xA5 == g_sUartDataManager.u8UartBuffer[g_sUartDataManager.u16ReadIndex])
            {                
                FrameHandlerStep = GET_DATA_LEN;
            }
            else
            {
                // 帧头错误,返回重新查找
                FrameHandlerStep = FIND_FRAME_HEAD1;
            }
            g_sUartDataManager.u16ReadIndex = (g_sUartDataManager.u16ReadIndex + 1) % UART_DATA_BUFFER_SIZE;
            break;

        case GET_DATA_LEN:
            if (g_sUartDataManager.u16WriteIndex == g_sUartDataManager.u16ReadIndex)
            {
                // 如果没有数据了, 就重新等待接收
                FrameHandlerStep = CHECK_FRAME_RECV;
                break;
            }
            // 获取接收数据的长度
            u8DataLen = g_sUartDataManager.u8UartBuffer[g_sUartDataManager.u16ReadIndex];
            if (g_sUartDataManager.u16WriteIndex > g_sUartDataManager.u16ReadIndex)
            {
                if ((g_sUartDataManager.u16WriteIndex - g_sUartDataManager.u16ReadIndex) < u8DataLen)
                {
                    g_sUartDataManager.u16ReadIndex = g_sUartDataManager.u16WriteIndex;
                    FrameHandlerStep = CHECK_FRAME_RECV;
                    break;
                }
            }
            else
            {
                if ((UART_DATA_BUFFER_SIZE - g_sUartDataManager.u16ReadIndex - 1 + g_sUartDataManager.u16WriteIndex) < u8DataLen)
                {
                    g_sUartDataManager.u16ReadIndex = g_sUartDataManager.u16WriteIndex;
                    FrameHandlerStep = CHECK_FRAME_RECV;
                    break;
                }
            }

            g_sUartDataManager.u16ReadIndex = (g_sUartDataManager.u16ReadIndex + 1) % UART_DATA_BUFFER_SIZE;
            FrameHandlerStep = PARSE_DATA;
            break;

        case PARSE_DATA:
            // 解析数据
			g_sUartDataManager.u8Len = u8DataLen;
            ParseUartData();			
            g_sUartDataManager.u16ReadIndex = (g_sUartDataManager.u16ReadIndex + u8DataLen) % UART_DATA_BUFFER_SIZE;

            if (g_sUartDataManager.u16ReadIndex == g_sUartDataManager.u16WriteIndex)
            {
                // 当前接收到的数据已经读完
                FrameHandlerStep = CHECK_FRAME_RECV;
            }
            else
            {
                // 查找下一个帧头
                FrameHandlerStep = FIND_FRAME_HEAD1;
            }           
            break;
    }

}


/**
  * @brief  将数据写入到串口屏
  * @param  pData 指向需要写入的数据指针
  * @param  u8Len 数据字节数
  * @retval None
  */
ScreenState WriteItem(const uint8_t *pData, const uint8_t u8Len)
{
    static enum
    {
        WRITE_SERIAL_FUNC = 0,
		WAIT_TRANSMIT_SUCCESS,
        WAIT_RESULT,
    }WriteStep = WRITE_SERIAL_FUNC;
    
    static uint32_t u32WaitTimer = 0;
    static uint8_t u8SendData[SEND_DATA_MAX_LEN] = {0};
	
	ScreenState eState = STATE_BUSY;

    switch (WriteStep)
    {
        case WRITE_SERIAL_FUNC:
            // 发送数据
            memset(u8SendData, 0, sizeof(u8SendData));
            memcpy(u8SendData, pData, u8Len);
			HAL_UART_Transmit_DMA(&UART_PORT, u8SendData, u8Len);
			ResetTimerCount(&u32WaitTimer);
			WriteStep = WAIT_TRANSMIT_SUCCESS;
            break;
		
		case WAIT_TRANSMIT_SUCCESS:
            // 等待发送完成
			if (g_bSendFinish)
			{
				ResetTimerCount(&u32WaitTimer);
				g_bSendFinish = false;
				WriteStep = WAIT_RESULT;
			}
			else
			{
				if (GetTimerTickDelta(u32WaitTimer, GetCurTimerCount()) >= 2000)
				{
                    /*
                        这里进行用户定义错误处理
                        错误类型: 发送超时
                    */
					eState = STATE_ERROR;
					WriteStep = WRITE_SERIAL_FUNC;
				}
			}
			break;

        case WAIT_RESULT:
            // 等待串口屏响应
            if (g_sUartDataManager.u32CmdRespond)
            {
				eState = STATE_OK;
                g_sUartDataManager.u32CmdRespond--;
                WriteStep = WRITE_SERIAL_FUNC;
            }
			else
			{
				if (GetTimerTickDelta(u32WaitTimer, GetCurTimerCount()) >= 2000)
				{
                    /*
                        这里进行用户定义错误处理
                        错误类型: 接收指令响应超时
                    */
					eState = STATE_TIMEOUT;
					WriteStep = WRITE_SERIAL_FUNC;
				}
			}
            break;
    }
	
	return eState;
}


/**
  * @brief  给一个串口屏地址发送一个无符号十六位数据
  * @param  u16Addr 串口屏变量地址
  * @param  u16Value 变量值
  * @retval ScreenState数据
  */
ScreenState UpdateValueU16(const uint16_t u16Addr, const uint16_t u16Value)
{
	uint8_t u8Cmd[] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x00, 0x00, 0x00};
	u8Cmd[4] = u16Addr >> 8;
	u8Cmd[5] = u16Addr;
	u8Cmd[6] = u16Value >> 8;
	u8Cmd[7] = u16Value;
	
	return WriteItem((const uint8_t *)&u8Cmd, sizeof(u8Cmd));
}


/**
  * @brief  将串口屏页面切换到指定的页面上
  * @param  u8TargetIndex 切换页面索引
  * @retval ScreenState数据
  */
ScreenState SwitchPage(const uint8_t u8TargetIndex)
{
	uint8_t u8Cmd[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x00};
	u8Cmd[sizeof(u8Cmd) - 1] = u8TargetIndex;
	
	return WriteItem((const uint8_t *)&u8Cmd, sizeof(u8Cmd));
}


/**
  * @brief  在当前页面的基础上弹窗或取消弹窗
  * @param  u8TargetIndex 弹窗所在页面索引, 255为无效数据
  * @retval ScreenState数据
  */
ScreenState PopWindow(const uint8_t u8TargetIndex)
{
	uint8_t u8Cmd[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0xE8, 0x5A, 0x01, 0x00, 0x00};
	if (255 == u8TargetIndex)
	{
        // 无效数据时取消弹窗
		u8Cmd[6] = 0;
		u8Cmd[7] = 0;
	}
	else
	{
		u8Cmd[sizeof(u8Cmd) - 1] = u8TargetIndex;
	}
	
	return WriteItem((const uint8_t *)&u8Cmd, sizeof(u8Cmd));
}


/**
  * @brief  串口数据发送完成回调
  * @param  None
  * @retval None
  */
void CbScreenUartTxCpltHandler(void)
{
	g_bSendFinish = true;
}


/**
  * @brief  串口屏通信层初始化
  * @param  None
  * @retval None
  */
void ScreenSerialInit(void)
{
    /*
        Demo, 注册串口数据发送完成回调函数, 在HAL_UART_TxCpltCallback调用时执行该函数
        RegisterUartTxCpltHandler(&UART_PORT, CbScreenUartTxCpltHandler);
    */
	
	
	// 定义中断和DMA
	__HAL_UART_ENABLE_IT(&UART_PORT, UART_IT_IDLE);
	HAL_UART_Receive_DMA(&UART_PORT, g_sUartDataManager.u8UartBuffer, sizeof(g_sUartDataManager.u8UartBuffer));
}


/**
  * @brief  串口空闲中断回调, 用于判断数据接收完成, 
  * 在 void USARTx_IRQHandler(void) 中判断空闲标志位并调用此函数 
  * @param  None
  * @retval None
  */
void CbUart1IdleHandler(void)
{
	g_bRecvFrameFinish = true;
}


/**
  * @brief  串口屏通信层流程执行
  * @param  None
  * @retval None
  */
void ScreenSerialHandler(void)
{		
	UartBufferHandler();
}
