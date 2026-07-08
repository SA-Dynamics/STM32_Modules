#include "UserHMIHandle.h"
#include "HMI_Control.h"
#include "Timer.h"
#include <string.h>


typedef struct
{
	uint8_t u8Row;
	uint8_t u8Col;
	uint8_t u8ColorIndex;
}CubeInfo;

struct
{
	CubeInfo sCubeInfo[CMD_BUFFER_LEN];
	uint8_t u8ReadIndex;
	uint8_t u8WriteIndex;
	uint8_t u8QueueLen;
	bool bBusy;
	bool bReset;
}g_sCubeCmdManager;


void UserHMIHandleInit(void)
{
	for (uint8_t i = 0; i < CUBE_ROW_NUM * CUBE_COL_NUM; i++)
	{
		g_sWidgetManager[i].eWidget = HMI_CUBE_START_WR + i;
		g_sWidgetManager[i].sCmdInfo.eCmd = CMD_UPDATE_VAR_U16;
		g_sWidgetManager[i].sCmdInfo.u16VarAddr = WR_CUBE_START_ADDR + i * 2;
		g_sWidgetManager[i].sCmdInfo.eDataType = TYPE_UINT16;
		g_sWidgetManager[i].sCmdInfo.u8DataLen = 1;
		g_sWidgetManager[i].sCmdInfo.pData = NULL;
		g_sWidgetManager[i].sCmdInfo.bSuccess = false;
	}
	
	HMI_ControlInit();
}


bool SendCubeCmd(const uint8_t u8Row, const uint8_t u8Col, const uint8_t u8ColorIndex)
{
	bool bRet = false;
	if (g_sCubeCmdManager.u8QueueLen < CMD_BUFFER_LEN)
    {      
		g_sCubeCmdManager.bBusy = true;
        g_sCubeCmdManager.sCubeInfo[g_sCubeCmdManager.u8WriteIndex].u8Row = u8Row;
		g_sCubeCmdManager.sCubeInfo[g_sCubeCmdManager.u8WriteIndex].u8Col = u8Col;
		g_sCubeCmdManager.sCubeInfo[g_sCubeCmdManager.u8WriteIndex].u8ColorIndex = u8ColorIndex;
		
        g_sCubeCmdManager.u8WriteIndex = (g_sCubeCmdManager.u8WriteIndex + 1) % CMD_BUFFER_LEN;
        g_sCubeCmdManager.u8QueueLen++;   

		bRet = true;		
    }  

	return bRet;
}


static bool ReadCubeCmd(CubeInfo *pInfo)
{
	bool bRet = false;

	// 从事件队列中读取一个事件
    if (g_sCubeCmdManager.u8QueueLen && pInfo)
    {
        memcpy(pInfo, &g_sCubeCmdManager.sCubeInfo[g_sCubeCmdManager.u8ReadIndex], sizeof(CubeInfo));

        g_sCubeCmdManager.u8ReadIndex = (g_sCubeCmdManager.u8ReadIndex + 1) % CMD_BUFFER_LEN;
        g_sCubeCmdManager.u8QueueLen--;

        bRet = true;
    }

    return bRet;
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	CbScreenUartTxCpltHandler();
}

bool ScreenBusy(void)
{
	return g_sCubeCmdManager.bBusy;
}


static bool ClearScreen(void)
{
	bool bRet = false;
	
	static enum
	{
		WRITE_DEFAULT_VALUE = 0,
		WRITE_CUBE,
		CHECK_INDEX,
	}ClearStep = WRITE_DEFAULT_VALUE;
	
	static uint32_t u32Timer = 0;
	static uint16_t u16Color = 0;
	static uint16_t u16CubeIndex = 0;
	
	switch (ClearStep)
	{
		case WRITE_DEFAULT_VALUE:
			g_sWidgetManager[u16CubeIndex].sCmdInfo.pData = &u16Color;
			ClearStep = WRITE_CUBE;
			break;
		
		case WRITE_CUBE:
			if (HMI_UpdateVariable(u16CubeIndex + HMI_CUBE_START_WR))
			{
				ResetTimerCount(&u32Timer);
				ClearStep = CHECK_INDEX;
			}
			break;
			
		case CHECK_INDEX:
			if (GetTimerTickDelta(u32Timer, GetCurTimerCount()) >= 5)
			{
				ClearStep = WRITE_DEFAULT_VALUE;
				
				u16CubeIndex = (u16CubeIndex + 1) % (CUBE_ROW_NUM * CUBE_COL_NUM);
				if (u16CubeIndex == 0)
				{
					bRet = true;
				}
			}
			break;
	}
	
	return bRet;
}


void ResetHMI(void)
{
	g_sCubeCmdManager.bReset = true;
}


static void CubeCmdHandler(void)
{
	static enum
	{
		CLEAR_SCREEN = 0,
		READ_CMD,
		WRITE_VAR,
		SEND_DELAY,
	}HandleStep = CLEAR_SCREEN;
	
	static uint32_t u32Timer = 0;
	static CubeInfo sInfo;
	static uint16_t u16Color;
	static uint16_t u16CubeIndex;
	
	switch (HandleStep)
	{
		case CLEAR_SCREEN:
			if (ClearScreen())
			{
				HandleStep = READ_CMD;				
			}
			break;
		
		case READ_CMD:
			if (ReadCubeCmd(&sInfo))
			{
				u16Color = sInfo.u8ColorIndex;
				u16CubeIndex = sInfo.u8Row * CUBE_COL_NUM + sInfo.u8Col;
				g_sWidgetManager[u16CubeIndex].sCmdInfo.pData = &u16Color;
				HandleStep = WRITE_VAR;
			}
			else
			{
				g_sCubeCmdManager.bBusy = false;
			}
			break;
		
		case WRITE_VAR:
		{
			if (HMI_UpdateVariable(u16CubeIndex + HMI_CUBE_START_WR))
			{
				ResetTimerCount(&u32Timer);
				HandleStep = SEND_DELAY;
			}
		}
			break;
		
		case SEND_DELAY:
			if (GetTimerTickDelta(u32Timer, GetCurTimerCount()) >= 2)
			{
				HandleStep = READ_CMD;
			}
			break;
	}
	
	if (g_sCubeCmdManager.bReset)
	{
		g_sCubeCmdManager.bReset = false;
		g_sCubeCmdManager.u8QueueLen = 0;
		HandleStep = CLEAR_SCREEN;
	}
}


void UserHMIHandleTask(void)
{
	CubeCmdHandler();	
	HMI_ControlRun();
}
