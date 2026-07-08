#ifndef HMI_VARIABLEMANAGER_H
#define HMI_VARIABLEMANAGER_H

#include "main.h"
#include "ScreenSerialDefines.h"
#include "ScreenMessageHandle.h"

#define CUBE_ROW_NUM	20
#define CUBE_COL_NUM	10

typedef enum
{
    HMI_WIDGET_NONE = 0,

    // 用户自定义widget枚举
	HMI_BUTTON_DIR_ADDR_RD,
	HMI_BUTTON_DOWN_RD,
	HMI_BUTTON_STATE_ADDR_RD,
    HMI_CUBE_START_WR,

}HMI_Widget;


typedef struct
{
	HMI_Widget eWidget;
	uint16_t u16Addr;
	void (*pCbFunc[EVENT_HANDLER_NUM_PER_WIDGET])(const HMI_Widget eWidget, const void *pReturnData);
}EventManagerStruct;


typedef struct
{
	HMI_Widget eWidget;
	ScrCmdInfo sCmdInfo;	
}WidgetManager;


extern EventManagerStruct g_sEventManager[3];
extern WidgetManager g_sWidgetManager[CUBE_ROW_NUM * CUBE_COL_NUM];

#endif
