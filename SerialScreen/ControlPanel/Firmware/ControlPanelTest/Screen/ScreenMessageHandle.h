#ifndef SCREENMESSAGEHANDLE_H
#define SCREENMESSAGEHANDLE_H

#include "main.h"


#define MAX_EVENT_NUM		16
#define MAX_CMD_NUM			32

// 串口屏指令类型
typedef enum
{
	CMD_NONE = 0,
	SYS_CMD_SWITCH_PAGE,	// 系统, 切换页面
	SYS_CMD_POP_WIN,		// 系统, 弹窗
	SYS_CMD_CANCEL_POP_WIN,	// 系统，取消弹窗
	CMD_UPDATE_VAR_U16,		// 更新无符号16位值
	SYS_CMD_REG_CURVE,		// 注册曲线变量
}CmdType;


// 串口屏数据类型
typedef	enum
{
	TYPE_VOID = 0,
	TYPE_UINT8,
	TYPE_UINT16,
}DataType;


// 指令信息块
typedef struct
{
	CmdType eCmd;
	uint16_t u16VarAddr;	// 地址
	void *pData;			// 数据指针
	DataType eDataType;		 
	uint8_t u8DataLen;		// 数据长度
	bool bSuccess;			// 指令执行结果
}ScrCmdInfo;


void LinkWriteVariable(const uint16_t u16Addr, void *pVariable);
void ScreenMessageHandleInit(void);
void ScreenMessageHandleRun(void);
bool SendCmd(ScrCmdInfo *pInfo);
void CbEventHandler(const uint16_t u16Addr, const void *pReturnData);

#endif
