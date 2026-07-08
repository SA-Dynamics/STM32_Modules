#include "HMI_VariableManager.h"


// 事件管理
EventManagerStruct g_sEventManager[3] = 
{
	{
		.eWidget = HMI_BUTTON_DIR_ADDR_RD,
		.u16Addr = RD_HMI_BUTTON_DIR_ADDR,
		.pCbFunc = {0},
	},
	
	{
		.eWidget = HMI_BUTTON_DOWN_RD,
		.u16Addr = RD_HMI_BUTTON_DOWN_ADDR,
		.pCbFunc = {0},
	},
	
	{
		.eWidget = HMI_BUTTON_STATE_ADDR_RD,
		.u16Addr = RD_HMI_BUTTON_STATE_ADDR,
		.pCbFunc = {0},
	},
};


// 控件管理
WidgetManager g_sWidgetManager[CUBE_ROW_NUM * CUBE_COL_NUM] = {0};
//{
//	// 这里加入用户自定义写变量

//	{
//		.sCmdInfo.eCmd = CMD_UPDATE_VAR_U16,
//		.sCmdInfo.u16VarAddr = WR_VARIABLE1_ADDR,
//		.sCmdInfo.eDataType = TYPE_UINT16,
//		.sCmdInfo.u8DataLen = 1,
//		.sCmdInfo.pData = NULL,
//		.sCmdInfo.bSuccess = false,
//	},
//	

//};