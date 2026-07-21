#ifndef CAN_PROCESS_H
#define CAN_PROCESS_H

#include "can.h"

#define DEFAULT_KEY_INDEX		0
#define CB_PROCESS_MAX_NUM		4


typedef void (*pCbProcess)(CAN_RxHeaderTypeDef *pRxHeader, uint8_t *pMessage);

void RegisterRecvProcess(pCbProcess pFunc);
void SetSendLock(const uint8_t u8KeyForLock);
void FreeSendLock(void);
void SendMessageHandler(CAN_TxHeaderTypeDef *pTxHeader, uint8_t *pMessage);


#endif
