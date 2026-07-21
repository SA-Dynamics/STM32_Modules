#include "CAN_Process.h"


CAN_TxHeaderTypeDef	g_sTxHeader;
CAN_RxHeaderTypeDef	g_sRxHeader;

static pCbProcess g_pFuncList[CB_PROCESS_MAX_NUM] = {NULL};
	

void RegisterRecvProcess(pCbProcess pFunc)
{
	if (pFunc)
	{
		for (uint8_t i = 0; i < CB_PROCESS_MAX_NUM; i++)
		{
			if (pFunc == g_pFuncList[i])
			{
				return;
			}
		}
		
		for (uint8_t i = 0; i < CB_PROCESS_MAX_NUM; i++)
		{
			if (!g_pFuncList[i])
			{
				g_pFuncList[i] = pFunc;
				break;
			}
		}
	}
}



void SendMessageHandler(CAN_TxHeaderTypeDef *pTxHeader, uint8_t *pMessage)
{
	uint32_t u32TxMailbox;
	uint32_t u32Count = 0;


	while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0)
	{
		if (u32Count++ > 10)
		{
			return;
		}
	}

	if(HAL_CAN_AddTxMessage(&hcan, pTxHeader, pMessage, &u32TxMailbox) != HAL_OK)
	{
		
	}
	else
	{
		
	}
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *pcan)
{
	CAN_RxHeaderTypeDef	RxHeader;
	uint8_t u8RecvData[8] = {0};
	
	if (&hcan == pcan)
	{
		HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &RxHeader, u8RecvData);
		
		for (uint8_t i = 0; i < CB_PROCESS_MAX_NUM; i++)
		{
			if (g_pFuncList[i])
			{
				g_pFuncList[i](&RxHeader, u8RecvData);
			}
		}
	}
}

