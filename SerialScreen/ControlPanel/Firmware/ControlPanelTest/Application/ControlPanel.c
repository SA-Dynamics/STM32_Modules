#include "ControlPanel.h"
#include "Timer.h"
#include "main.h"
#include "HMI_Control.h"


struct
{
	uint16_t u16RotateIcon;
	uint16_t u16DataVar;
	uint16_t u16ProgBar1;
	uint16_t u16ProgBar2;
	uint16_t u16VarIcon1;
	uint16_t u16VarIcon2;
	uint16_t u16BitIcon;
	uint8_t u8PopValue;
}g_sPanelManager;


void UpdateMeterRotate(void)
{
	static uint32_t u32TimerCount = 0;
	static int16_t s16Value = 0;
	static int8_t s8Direction = 1;
	
	if (GetTimerTickDelta(u32TimerCount, GetCurTimerCount()) >= 20)
	{
		ResetTimerCount(&u32TimerCount);
		
		s16Value = s16Value + s8Direction;
		
		// 旋转值做映射
		g_sPanelManager.u16RotateIcon = ((s16Value - 70) >= 0) ? ((s16Value - 70) * 2) : ((s16Value - 70) * 2 + 360);
		g_sPanelManager.u16DataVar = s16Value;
		
		if (s16Value >= 140)
		{
			s8Direction = -1;
		}
		else if (s16Value <= 0)
		{
			s8Direction = 1;
		}
		

	}
}


void UpdateProgressBar(void)
{
	static uint32_t u32TimerCount1 = 0;
	static int8_t s8Direction1 = 1;
	static uint32_t u32TimerCount2 = 0;
	static int8_t s8Direction2 = 1;
	
	static int16_t s16ProgBar1 = 0;
	static int16_t s16ProgBar2 = 0;
	
	if (GetTimerTickDelta(u32TimerCount1, GetCurTimerCount()) >= 50)
	{
		ResetTimerCount(&u32TimerCount1);
		
		s16ProgBar1 += s8Direction1;
		g_sPanelManager.u16ProgBar1 = s16ProgBar1;
		
		if (s16ProgBar1 >= 100)
		{
			s8Direction1 = -1;
		}
		else if (s16ProgBar1 <= 0)
		{
			s8Direction1 = 1;
		}	
	}
	
	if (GetTimerTickDelta(u32TimerCount2, GetCurTimerCount()) >= 20)
	{
		ResetTimerCount(&u32TimerCount2);
		
		s16ProgBar2 += s8Direction2;
		g_sPanelManager.u16ProgBar2 = s16ProgBar2;
		
		if (s16ProgBar2 >= 100)
		{
			s8Direction2 = -1;
		}
		else if (s16ProgBar2 <= 0)
		{
			s8Direction2 = 1;
		}	
	}
}

// 更新变量图标
void UpdateIconValue(void)
{
	static uint32_t u32TimerCount1 = 0;
	static uint32_t u32TimerCount2 = 0;
	static uint16_t u16Icon1Value = 0;
	static uint16_t u16Icon2Value = 0;
	
	if (GetTimerTickDelta(u32TimerCount1, GetCurTimerCount()) >= 800)
	{
		ResetTimerCount(&u32TimerCount1);
		
		u16Icon1Value = (u16Icon1Value + 1) % 3;
		g_sPanelManager.u16VarIcon1 = u16Icon1Value + 2;
	}
	
	if (GetTimerTickDelta(u32TimerCount2, GetCurTimerCount()) >= 1000)
	{
		ResetTimerCount(&u32TimerCount2);	
		
		u16Icon2Value = (u16Icon2Value + 1) % 2;
		g_sPanelManager.u16VarIcon2 = u16Icon2Value + 5;
	}
}


// 更新位变量图标
void UpdateBitIconValue(void)
{
	static uint32_t u32TimerCount1 = 0;
	static uint32_t u32TimerCount2 = 0;
	static uint32_t u32TimerCount3 = 0;
	
	static struct
	{
		uint16_t bBit0 : 1;
		uint16_t bBit1 : 1;
		uint16_t bBit2 : 1;
		uint16_t : 13;
	}sData;
	
	// 以不同频率更新数据
	if (GetTimerTickDelta(u32TimerCount1, GetCurTimerCount()) >= 1000)
	{
		ResetTimerCount(&u32TimerCount1);
		
		sData.bBit0 = ~sData.bBit0;
	}	
	
	if (GetTimerTickDelta(u32TimerCount2, GetCurTimerCount()) >= 800)
	{
		ResetTimerCount(&u32TimerCount2);
		
		sData.bBit1 = ~sData.bBit1;
	}	
	
	if (GetTimerTickDelta(u32TimerCount3, GetCurTimerCount()) >= 500)
	{
		ResetTimerCount(&u32TimerCount3);
		
		sData.bBit2 = ~sData.bBit2;
	}
	
	g_sPanelManager.u16BitIcon = *(uint16_t *)&sData;
}


static uint16_t GetU16LittleEndian(const uint16_t u16Value)
{
	uint8_t u8Hi = u16Value >> 8;
	uint8_t u8Lo = u16Value & 0xFF;
	
	return ((uint16_t)u8Lo << 8) | u8Hi;
}


void CbButtonFunc(const HMI_Widget eWidget, const void *pReturnData)
{
	if (HMI_BUTTON_POP_RD == eWidget)
	{
		if (0x0D0D == GetU16LittleEndian(*(uint16_t *)pReturnData))
		{
			g_sPanelManager.u8PopValue = 1;
		}
	}
	else if (HMI_BUTTON_RELEASE_RD == eWidget)
	{
		if (0x00F0 == GetU16LittleEndian(*(uint16_t *)pReturnData))
		{
			g_sPanelManager.u8PopValue = 2;
		}
	}
}


// 串口中断回调
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	CbScreenUartTxCpltHandler();
}

volatile uint8_t u8IndexTest = 0;

static void PanelCmdHandler(void)
{
	static enum
	{
		REGISTER_CURVE_VAR = 0,
		WRITE_VAR,
		UPDATE_DATA,
		POP_WIN_SHOW,
	}HandleStep = REGISTER_CURVE_VAR;
	
	static uint32_t u32Timer = 0;
	static uint8_t u8Index = 0;
	const HMI_Widget eWidget[] = 
	{
		HMI_ROTATE_ICON_WR, 
		HMI_DATA_VAR_WR, 
		HMI_PROG_BAR1_WR,
		HMI_PROG_BAR2_WR,
		HMI_VAR_ICON1_WR,
		HMI_VAR_ICON2_WR,
		HMI_BIT_ICON_WR
	};
	
	switch (HandleStep)
	{
		case REGISTER_CURVE_VAR:
			if (HMI_RegisterCurveVar(1, HMI_DATA_VAR_WR))
			{
				HandleStep = WRITE_VAR;				
			}
			break;
		
		case WRITE_VAR:
			{
				if (HMI_UpdateVariable(eWidget[u8Index]))
				{
					ResetTimerCount(&u32Timer);
					u8Index = (u8Index + 1) % (sizeof(eWidget) / sizeof(eWidget[0]));
					HandleStep = UPDATE_DATA;
				}
			}
			break;
		
		case UPDATE_DATA:
			if (GetTimerTickDelta(u32Timer, GetCurTimerCount()) >= 2)
			{
				HandleStep = WRITE_VAR;
			}
			else 
			{
				if (1 == g_sPanelManager.u8PopValue)
				{
					HandleStep = POP_WIN_SHOW;
				}
			}
			break;
			
		case POP_WIN_SHOW:
			if (1 == g_sPanelManager.u8PopValue)
			{
				if (HMI_PopWindow(1))
				{
					g_sPanelManager.u8PopValue = 0;
				}
			}
			else if (2 == g_sPanelManager.u8PopValue)
			{
				if (HMI_CancelPopWindow())
				{
					g_sPanelManager.u8PopValue = 0;
					HandleStep = WRITE_VAR;
				}
			}
			break;
	}
	
	UpdateMeterRotate();
	UpdateProgressBar();
	UpdateBitIconValue();
	UpdateIconValue();
	
	u8IndexTest = u8Index;
}



void ControlPanelInit(void)
{
	HMI_RegisterWidgetEvent(HMI_BUTTON_POP_RD, CbButtonFunc);
	HMI_RegisterWidgetEvent(HMI_BUTTON_RELEASE_RD, CbButtonFunc);
	HMI_LinkWriteVariable(HMI_ROTATE_ICON_WR, &g_sPanelManager.u16RotateIcon);
	HMI_LinkWriteVariable(HMI_DATA_VAR_WR, &g_sPanelManager.u16DataVar);
	HMI_LinkWriteVariable(HMI_PROG_BAR1_WR, &g_sPanelManager.u16ProgBar1);
	HMI_LinkWriteVariable(HMI_PROG_BAR2_WR, &g_sPanelManager.u16ProgBar2);
	HMI_LinkWriteVariable(HMI_VAR_ICON1_WR, &g_sPanelManager.u16VarIcon1);
	HMI_LinkWriteVariable(HMI_VAR_ICON2_WR, &g_sPanelManager.u16VarIcon2);
	HMI_LinkWriteVariable(HMI_BIT_ICON_WR, &g_sPanelManager.u16BitIcon);
	
	HMI_ControlInit();
}


void ControlPanelHandle(void)
{
	PanelCmdHandler();
	HMI_ControlRun();
}
