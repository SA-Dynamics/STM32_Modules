#include "HMI_VariableManager.h"


// 事件管理
EventManagerStruct g_sEventManager[3] = 
{
	{
		.eWidget = HMI_BUTTON_POP_RD,
		.u16Addr = RD_HMI_BUTTON_POP_ADDR,
		.pCbFunc = {0},
	},
	
	{
		.eWidget = HMI_BUTTON_RELEASE_RD,
		.u16Addr = RD_HMI_BUTTON_RELEASE_ADDR,
		.pCbFunc = {0},
	},
	
};




// 控件管理
WidgetManager g_sWidgetManager[] = 
{
	// 这里加入用户自定义写变量

	{
		.eWidget = HMI_ROTATE_ICON_WR,
		.sCmdInfo.eCmd = CMD_UPDATE_VAR_U16,
		.sCmdInfo.u16VarAddr = WR_ROTATE_ICON_ADDR,
		.sCmdInfo.eDataType = TYPE_UINT16,
		.sCmdInfo.u8DataLen = 1,
		.sCmdInfo.pData = NULL,
		.sCmdInfo.bSuccess = false,
	},
	
	{
		.eWidget = HMI_DATA_VAR_WR,
		.sCmdInfo.eCmd = CMD_UPDATE_VAR_U16,
		.sCmdInfo.u16VarAddr = WR_DATA_VAR_ADDR,
		.sCmdInfo.eDataType = TYPE_UINT16,
		.sCmdInfo.u8DataLen = 1,
		.sCmdInfo.pData = NULL,
		.sCmdInfo.bSuccess = false,
	},
	
	{
		.eWidget = HMI_PROG_BAR1_WR,
		.sCmdInfo.eCmd = CMD_UPDATE_VAR_U16,
		.sCmdInfo.u16VarAddr = WR_PROG_BAR1_ADDR,
		.sCmdInfo.eDataType = TYPE_UINT16,
		.sCmdInfo.u8DataLen = 1,
		.sCmdInfo.pData = NULL,
		.sCmdInfo.bSuccess = false,
	},
		
	{
		.eWidget = HMI_PROG_BAR2_WR,
		.sCmdInfo.eCmd = CMD_UPDATE_VAR_U16,
		.sCmdInfo.u16VarAddr = WR_PROG_BAR2_ADDR,
		.sCmdInfo.eDataType = TYPE_UINT16,
		.sCmdInfo.u8DataLen = 1,
		.sCmdInfo.pData = NULL,
		.sCmdInfo.bSuccess = false,
	},
	
	{
		.eWidget = HMI_VAR_ICON1_WR,
		.sCmdInfo.eCmd = CMD_UPDATE_VAR_U16,
		.sCmdInfo.u16VarAddr = WR_VAR_ICON1_ADDR,
		.sCmdInfo.eDataType = TYPE_UINT16,
		.sCmdInfo.u8DataLen = 1,
		.sCmdInfo.pData = NULL,
		.sCmdInfo.bSuccess = false,
	},
	
	{
		.eWidget = HMI_VAR_ICON2_WR,
		.sCmdInfo.eCmd = CMD_UPDATE_VAR_U16,
		.sCmdInfo.u16VarAddr = WR_VAR_ICON2_ADDR,
		.sCmdInfo.eDataType = TYPE_UINT16,
		.sCmdInfo.u8DataLen = 1,
		.sCmdInfo.pData = NULL,
		.sCmdInfo.bSuccess = false,
	},
	
	{
		.eWidget = HMI_BIT_ICON_WR,
		.sCmdInfo.eCmd = CMD_UPDATE_VAR_U16,
		.sCmdInfo.u16VarAddr = WR_BIT_ICON_ADDR,
		.sCmdInfo.eDataType = TYPE_UINT16,
		.sCmdInfo.u8DataLen = 1,
		.sCmdInfo.pData = NULL,
		.sCmdInfo.bSuccess = false,
	},
};