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
	HMI_BUTTON_POP_RD,
	HMI_BUTTON_RELEASE_RD,

    HMI_ROTATE_ICON_WR,
	HMI_DATA_VAR_WR,
	HMI_PROG_BAR1_WR,
	HMI_PROG_BAR2_WR,
	HMI_VAR_ICON1_WR,
	HMI_VAR_ICON2_WR,
	HMI_BIT_ICON_WR,
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
