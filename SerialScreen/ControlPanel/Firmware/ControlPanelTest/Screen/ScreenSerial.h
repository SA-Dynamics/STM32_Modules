#ifndef SCREENSERIAL_H
#define SCREENSERIAL_H


#include "main.h"

// 串口屏事件字节数,在串口屏配置中限制长度
#define EVENT_DATA_MAX_LEN      16


// 串口屏执行逻辑返回结果
typedef enum
{
	STATE_NONE = 0,
    STATE_OK,
	STATE_ERROR,
    STATE_TIMEOUT,
    STATE_BUSY,
}ScreenState;


// 串口屏事件
typedef struct 
{
    uint16_t u16VarAddr;
    uint8_t u8Data[EVENT_DATA_MAX_LEN];
    uint8_t u8Len;
}ScreenEvent;



void ScreenSerialInit(void);
ScreenState SwitchPage(const uint8_t u8TargetIndex);
ScreenState PopWindow(const uint8_t u8TargetIndex);
ScreenState UpdateValueU16(const uint16_t u16Addr, const uint16_t u16Value);
ScreenState RegisterCurveVar(const uint8_t u8Channel, const uint16_t u16Index);
void ScreenSerialHandler(void);

// weak, 由外部实现
void CbRecvEvent(const ScreenEvent *pEvent);

#endif
