#include "ScreenMessageHandle.h"
#include "ScreenSerial.h"
#include "ScreenSerialDefines.h"
#include "Timer.h"
#include <string.h>



#define DYNAMIC_DATA_MAX_NUM        32
#define ACTION_NUM_MAX				4

typedef void (*pPageHandler)(void);

// 页面管理
struct
{           
    uint16_t u16PopPageIndex;		// 弹窗页面索引
    uint16_t u16SwitchPageIndex;	// 切换页面索引
}g_sPageManager;


// 指令队列管理
struct
{
    ScrCmdInfo *sCmdQueue[MAX_CMD_NUM];
    uint8_t u8WriteIndex;
    uint8_t u8ReadIndex;
    uint8_t u8QueueLen;
}g_sScrCmdManager = {0};


// 事件管理
struct
{
    ScreenEvent sEventQueue[MAX_EVENT_NUM];
    uint8_t u8WriteIndex;
    uint8_t u8ReadIndex;
    uint8_t u8QueueLen;
}g_sEventQueueManager = {0};



/**
  * @brief  事件接收回调, 重写ScreenSerial.c中的定义
  * @param  pEvent 事件变量地址
  * @retval None
  */
void CbRecvEvent(const ScreenEvent *pEvent)
{
	// 将事件拷贝到队列中
    if (g_sEventQueueManager.u8QueueLen < MAX_EVENT_NUM)
    {      
        memcpy(&g_sEventQueueManager.sEventQueue[g_sEventQueueManager.u8ReadIndex], pEvent, sizeof(ScreenEvent));
        g_sEventQueueManager.u8WriteIndex = (g_sEventQueueManager.u8WriteIndex + 1) % MAX_EVENT_NUM;
        g_sEventQueueManager.u8QueueLen++;      
    }       
}


/**
  * @brief  事件读取
  * @param  pEvent 事件变量地址
  * @retval None
  */
static bool ReadEvent(ScreenEvent *pEvent)
{
    bool bRet = false;

	// 从事件队列中读取一个事件
    if (g_sEventQueueManager.u8QueueLen && pEvent)
    {
        memcpy(pEvent, &g_sEventQueueManager.sEventQueue[g_sEventQueueManager.u8ReadIndex], sizeof(ScreenEvent));

        g_sEventQueueManager.u8ReadIndex = (g_sEventQueueManager.u8ReadIndex + 1) % MAX_EVENT_NUM;
        g_sEventQueueManager.u8QueueLen--;

        bRet = true;
    }

    return bRet;
}


/**
  * @brief  读取指令, 只在内部使用
  * @param  pInfo 指令地址的指针
  * @retval None
  */
static bool ReadCmd(ScrCmdInfo **pInfo)
{
    bool bRet = false;
	
	// 将指令从队列中取出
    if (g_sScrCmdManager.u8QueueLen)
    {
        *pInfo = g_sScrCmdManager.sCmdQueue[g_sScrCmdManager.u8ReadIndex];

        g_sScrCmdManager.u8ReadIndex = (g_sScrCmdManager.u8ReadIndex + 1) % MAX_CMD_NUM;
        g_sScrCmdManager.u8QueueLen--;

        bRet = true;
    }

    return bRet;
}


/**
  * @brief  发送指令, 只在外部使用
  * @param  pInfo 指令变量的指针
  * @retval None
  */
bool SendCmd(ScrCmdInfo *pInfo)
{
	bool bRet = false;
	
	// 将指令放入到队列中
    if (g_sScrCmdManager.u8QueueLen < MAX_CMD_NUM)
    {      
        g_sScrCmdManager.sCmdQueue[g_sScrCmdManager.u8ReadIndex] = pInfo;

        g_sScrCmdManager.u8WriteIndex = (g_sScrCmdManager.u8WriteIndex + 1) % MAX_CMD_NUM;
        g_sScrCmdManager.u8QueueLen++;  
		bRet = true;
    }  

	return bRet;
}


/**
  * @brief  执行指令对应的操作
  * @param  pInfo 指令变量的指针
  * @param  pState 执行函数返回的结果
  * @retval None
  */
void ActionHandler(ScrCmdInfo *pInfo, ScreenState *pState)
{
	if (!pInfo)
	{
		return;
	}
	
	switch (pInfo->eCmd)
	{
		case SYS_CMD_SWITCH_PAGE:
			*pState = SwitchPage(*(uint8_t *)pInfo->pData);
			if (*pState == STATE_OK)
			{
			}
			break;
		
		case SYS_CMD_POP_WIN:
			*pState = PopWindow(*(uint8_t *)pInfo->pData);
			break;
		
		case CMD_UPDATE_VAR_U16:
			*pState = UpdateValueU16(pInfo->u16VarAddr, *(uint16_t *)pInfo->pData);
			break;
		
		default:
			break;
	}
}


/**
  * @brief  指令执行流程
  * @param  None
  * @retval None
  */
void CmdManagerRun(void)
{
	static enum
	{
		CHECK_BUFFER = 0,
		CHECK_ACTION,
		ACTION_DELAY,
	}ItemHandleStep = CHECK_BUFFER;
	
	static uint32_t u32HandleTimer = 0;
	ScreenState eState = STATE_NONE;
	static ScrCmdInfo *pInfo = NULL;
	
	switch (ItemHandleStep)
	{
		case CHECK_BUFFER:
			// 获取指令变量并检查变量是否有效
			if (ReadCmd(&pInfo))
			{
				if (pInfo)
				{
					pInfo->bSuccess = false;
					ItemHandleStep = CHECK_ACTION;
				}
			}
			break;
			
		case CHECK_ACTION:
			// 执行指令对应的操作并检查执行结果
			ActionHandler(pInfo, &eState);
			if (STATE_OK == eState || STATE_TIMEOUT == eState || STATE_ERROR == eState)
			{	
				if (STATE_OK == eState)
				{
					pInfo->bSuccess = true;
				}
				ResetTimerCount(&u32HandleTimer);
				ItemHandleStep = ACTION_DELAY;
			}			
			break;
			
		case ACTION_DELAY:
			// 进行一段时间的延时
			if (GetTimerTickDelta(u32HandleTimer, GetCurTimerCount()) >= 2)
			{
				ItemHandleStep = CHECK_BUFFER;
			}			
			break;
		
		default:
			break;
	}	
}


/**
  * @brief  事件处理
  * @param  u16Addr 事件控件地址
  * @param  pReturnData 事件数据
  * @retval None
  */
__weak void CbEventHandler(const uint16_t u16Addr, const void *pReturnData)
{

}


/**
  * @brief  执行读变量管理流程
  * @param  None
  * @retval None
  */
void ItemsReadManagerRun(void)
{
	ScreenEvent sEvent;
	
	if (ReadEvent(&sEvent))
	{
		CbEventHandler(sEvent.u16VarAddr, &sEvent.u8Data);
	}
}


/**
  * @brief  串口屏消息处理初始化
  * @param  None
  * @retval None
  */
void ScreenMessageHandleInit(void)
{
	ScreenSerialInit();
}


/**
  * @brief  串口屏消息处理流程
  * @param  None
  * @retval None
  */
void ScreenMessageHandleRun(void)
{	
	ItemsReadManagerRun();
	CmdManagerRun();
	ScreenSerialHandler();
}
