#include "HMI_Control.h"
#include "Timer.h"

#define HMI_EVENT_QUEUE_LEN		4
	
struct
{
	bool bInit;
	uint8_t u8SwitchPageStep;
	uint8_t u8PopWindowStep;
	uint8_t u8WriteVarStep;
}g_sHMIControlManager;



/**
  * @brief  从控件管理中查找cmd指针
  * @param  eWidget 控件枚举
  * @retval cmd 指令地址
  */
static ScrCmdInfo *FindCmdPointer(const HMI_Widget eWidget)
{
	for (uint8_t i = 0; i < sizeof(g_sWidgetManager) / sizeof(g_sWidgetManager[0]); i++)
	{
		if (eWidget == g_sWidgetManager[i].eWidget)
		{
			return &g_sWidgetManager[i].sCmdInfo;
		}
	}	
	
	return NULL;
}


/**
  * @brief  将外部变量和串口屏变量进行绑定
  * @param  eWidget 控件枚举
  * @param  pVariable 外部变量地址
  * @retval None
  */
void HMI_LinkWriteVariable(const HMI_Widget eWidget, void *pVariable)
{
	for (uint8_t i = 0; i < sizeof(g_sWidgetManager) / sizeof(g_sWidgetManager[0]); i++)
	{
		if (eWidget == g_sWidgetManager[i].eWidget)
		{
			g_sWidgetManager[i].sCmdInfo.pData = pVariable;
			break;
		}
	}
}


/**
  * @brief  更新串口屏变量
  * @param  eWidget 控件枚举
  * @retval true 成功, false 失败
  */
bool HMI_UpdateVariable(const HMI_Widget eWidget)
{
	bool bRet = false;
	
	enum
	{
		WR_PREPARE = 0,
		WR_SEND_CMD,
		WR_WAIT_RESULT,
	};
	
	static ScrCmdInfo *pCmd = NULL;
	
	switch (g_sHMIControlManager.u8WriteVarStep)
	{
		case WR_PREPARE:
			pCmd = FindCmdPointer(eWidget);
			if (pCmd)
			{
				pCmd->bSuccess = false;
				g_sHMIControlManager.u8WriteVarStep = WR_SEND_CMD;
			}
			break;
			
		case WR_SEND_CMD:
			SendCmd(pCmd);
			g_sHMIControlManager.u8WriteVarStep = WR_WAIT_RESULT;
			break;
			
		case WR_WAIT_RESULT:
			if (pCmd->bSuccess)
			{
				pCmd->bSuccess = false;
				g_sHMIControlManager.u8WriteVarStep = WR_PREPARE;
				bRet = true;				
			}
			break;
	}
	
	return bRet;	
}


/**
  * @brief  更新串口屏变量
  * @param  eWidget 控件枚举
  * @retval true 成功, false 失败
  */
static bool HMI_SwitchPage(const uint16_t u16PageIndex)
{
	bool bRet = false;
	
	enum
	{
		SW_PREPARE = 0,
		SW_SEND_CMD,
		SW_WAIT_RESULT,
	};
	static uint16_t u16SwIndex = 0;
	static ScrCmdInfo sCmd = {0};
	
	switch (g_sHMIControlManager.u8SwitchPageStep)
	{
		case SW_PREPARE:
			u16SwIndex = u16PageIndex;
			sCmd.eCmd = SYS_CMD_SWITCH_PAGE;
			sCmd.u16VarAddr = 0x0084;
			sCmd.eDataType = TYPE_UINT16;
			sCmd.u8DataLen = 1;
			sCmd.pData = &u16SwIndex;
			sCmd.bSuccess = false;
			g_sHMIControlManager.u8SwitchPageStep = SW_SEND_CMD;
			break;
			
		case SW_SEND_CMD:
			SendCmd(&sCmd);
			g_sHMIControlManager.u8SwitchPageStep = SW_WAIT_RESULT;
			break;
			
		case SW_WAIT_RESULT:
			if (sCmd.bSuccess)
			{
				sCmd.bSuccess = false;
				g_sHMIControlManager.u8SwitchPageStep = 0;
				bRet = true;				
			}
			break;
	}
	
	return bRet;
}


/**
  * @brief  更新串口屏变量
  * @param  eWidget 控件枚举
  * @retval true 成功, false 失败
  */
static bool HMI_PopWindow(const uint16_t u16Index)
{
	bool bRet = false;
	
	enum
	{
		SW_PREPARE = 0,
		SW_SEND_CMD,
		SW_WAIT_RESULT,
	};
	static uint16_t u16PopIndex = 0;
	static ScrCmdInfo sCmd = {0};
	
	switch (g_sHMIControlManager.u8PopWindowStep)
	{
		case SW_PREPARE:
			u16PopIndex = u16Index;
			sCmd.eCmd = SYS_CMD_POP_WIN;
			sCmd.u16VarAddr = 0x00E8;
			sCmd.eDataType = TYPE_UINT16;
			sCmd.u8DataLen = 1;
			sCmd.pData = &u16PopIndex;
			sCmd.bSuccess = false;
			g_sHMIControlManager.u8PopWindowStep = SW_SEND_CMD;
			break;
			
		case SW_SEND_CMD:
			SendCmd(&sCmd);
			g_sHMIControlManager.u8PopWindowStep = SW_WAIT_RESULT;
			break;
			
		case SW_WAIT_RESULT:
			if (sCmd.bSuccess)
			{
				sCmd.bSuccess = false;
				g_sHMIControlManager.u8PopWindowStep = 0;
				bRet = true;				
			}
			break;
	}
	
	return bRet;
}


/**
  * @brief  取消弹窗
  * @param  None
  * @retval true 成功, false 失败
  */
static bool HMI_CancelPopWindow(void)
{
	bool bRet = false;
	
	enum
	{
		SW_PREPARE = 0,
		SW_SEND_CMD,
		SW_WAIT_RESULT,
	};
	static uint16_t u16PopIndex = 0;
	static ScrCmdInfo sCmd = {0};
	
	switch (g_sHMIControlManager.u8PopWindowStep)
	{
		case SW_PREPARE:
			u16PopIndex = 255;
			sCmd.eCmd = SYS_CMD_CANCEL_POP_WIN;
			sCmd.u16VarAddr = 0x00E8;
			sCmd.eDataType = TYPE_UINT16;
			sCmd.u8DataLen = 1;
			sCmd.pData = &u16PopIndex;
			sCmd.bSuccess = false;
			g_sHMIControlManager.u8PopWindowStep = SW_SEND_CMD;
			break;
			
		case SW_SEND_CMD:
			SendCmd(&sCmd);
			g_sHMIControlManager.u8PopWindowStep = SW_WAIT_RESULT;
			break;
			
		case SW_WAIT_RESULT:
			if (sCmd.bSuccess)
			{
				sCmd.bSuccess = false;
				g_sHMIControlManager.u8PopWindowStep = 0;
				bRet = true;				
			}
			break;
	}
	
	return bRet;
}


/**
  * @brief  获取控件变量值
  * @param  eWidget 控件枚举
  * @param  pData 变量地址
  * @retval None
  */
void HMI_GetWidgetValue(const HMI_Widget eWidget, void **pData)
{
	for (uint8_t i = 0; i < sizeof(g_sWidgetManager) / sizeof(g_sWidgetManager[0]); i++)
	{
		if (eWidget == g_sWidgetManager[i].eWidget)
		{
			*pData = g_sWidgetManager[i].sCmdInfo.pData;
			break;
		}
	}
}


/**
  * @brief  实现ScreenMessageHandle.c中的事件处理回调
  * @param  u16Addr 事件地址
  * @param  pReturnData 事件数据地址
  * @retval None
  */
void CbEventHandler(const uint16_t u16Addr, const void *pReturnData)
{
	for (uint8_t i = 0; i < sizeof(g_sEventManager) / sizeof(g_sEventManager[0]); i++)
	{
		if (u16Addr == g_sEventManager[i].u16Addr)
		{
			for (uint8_t j = 0; j < EVENT_HANDLER_NUM_PER_WIDGET; j++)
			{
				if (g_sEventManager[i].pCbFunc[j])
				{
					g_sEventManager[i].pCbFunc[j](g_sEventManager[i].eWidget, pReturnData);
				}
			}
			break;
		}
	}
}



/**
  * @brief  实现ScreenMessageHandle.c中的事件处理回调
  * @param  u16Addr 事件地址
  * @param  pReturnData 事件数据地址
  * @retval None
  */
void HMI_RegisterWidgetEvent(const HMI_Widget eWidget, void (*pCbFunc)(const HMI_Widget eWidget, const void *pReturnData))
{
	for (uint8_t i = 0; i < sizeof(g_sEventManager) / sizeof(g_sEventManager[0]); i++)
	{
		if (eWidget == g_sEventManager[i].eWidget)
		{
			for (uint8_t j = 0; j < EVENT_HANDLER_NUM_PER_WIDGET; j++)
			{
				if (!g_sEventManager[i].pCbFunc[j])
				{
					g_sEventManager[i].pCbFunc[j] = pCbFunc;
					return;
				}
			}
			
		}
	}
}



void HMI_ControlInit(void)
{
	ScreenMessageHandleInit();
	
	g_sHMIControlManager.bInit = true;
}


void HMI_ControlRun(void)
{
	static uint32_t u32OpTimer = 0;
	
	if (!g_sHMIControlManager.bInit)
	{
		return;
	}
	
	ScreenMessageHandleRun();
}
