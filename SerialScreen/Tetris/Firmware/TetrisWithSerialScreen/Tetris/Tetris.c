#include "Tetris.h"
#include "UserHMIHandle.h"
#include "HMI_VariableManager.h"
#include "Timer.h"
#include <stdlib.h>
#include <string.h>


#define TETRIS_COL_CUBE_NUM		10	// 列数
#define TETRIS_ROW_CUBE_NUM		21	// 行数


struct
{
	bool bMoveLeft;
	bool bMoveRight;
	bool bTransForm;
	bool bDown;
	bool bRun;
	uint32_t u32SleepTimer;
	uint32_t u32SleepInterval;
	uint32_t u32RandValue;
	uint8_t u8MinLevelRow;
	uint8_t u8MinLevelRowUpdateStart;
}g_sControlManager;

typedef struct
{
	uint8_t u8ID;
	bool bLeftBoundary;
	bool bRightBoundary;
	bool bTopBoundary;
	bool bBottumBoundary;
	uint8_t u8AxisCol;
	uint8_t u8AxisRow;
}CubeStruct;


typedef enum
{
	CUBE_COLOR_WHITE = 0,
	CUBE_COLOR_RED,
	CUBE_COLOR_ORANGE,
	CUBE_COLOR_YELLOW,
	CUBE_COLOR_GREEN,
	CUBE_COLOR_BLUE,
}CubeColor;


typedef enum
{
	FORM_TYPE_NONE = 0,
	FORM_TYPE_SQUARE,
	FORM_TYPE_BAR,
	FORM_TYPE_Z,
	FORM_TYPE_RZ,
	FORM_TYPE_L,
	FORM_TYPE_RL,
	FORM_TYPE_HILL,
}FormType;


typedef struct
{
	uint8_t u8FormIndex;
	const uint8_t u8FormNum;
	const CubeStruct (*pFormAxis)[4];
}FormStruct;


typedef enum
{
	TYPE_NONE = 0,
	TYPE_MOVE_DOWN,
	TYPE_MOVE_LEFT,
	TYPE_MOVE_RIGHT,
	TYPE_MOVE_ROTATE,
	TYPE_UPDATE_CUBES,
}UpdateType;



const CubeStruct g_sReverseZFormAxisInfo[][4] = 
{
	{
		{.u8ID = 1, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 0, .u8AxisRow = 1},
		{.u8ID = 2, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 1, .u8AxisRow = 1},
		{.u8ID = 3, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 0},
		{.u8ID = 4, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 2, .u8AxisRow = 0},
	},
	{
		{.u8ID = 1, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 1, .u8AxisRow = 2},
		{.u8ID = 2, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 1},
		{.u8ID = 3, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 0, .u8AxisRow = 1},
		{.u8ID = 4, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 0},
	},	
};


const CubeStruct g_sZFormAxisInfo[][4] = 
{
	{
		{.u8ID = 1, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 0},
		{.u8ID = 2, .bLeftBoundary = false, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 0},
		{.u8ID = 3, .bLeftBoundary = false, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 1, .u8AxisRow = 1},
		{.u8ID = 4, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 2, .u8AxisRow = 1},
	},
	{
		{.u8ID = 1, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 0, .u8AxisRow = 2},
		{.u8ID = 2, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 1},
		{.u8ID = 3, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 1},
		{.u8ID = 4, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 0},
	},	
};


const CubeStruct g_sSquareAxisInfo[][4] = 
{
	{
		{.u8ID = 1, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 0},
		{.u8ID = 2, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 0},
		{.u8ID = 3, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 0, .u8AxisRow = 1},
		{.u8ID = 4, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 1, .u8AxisRow = 1},
	},
};


const CubeStruct g_sBarFormAxisInfo[][4] = 
{
	{
		{.u8ID = 1, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = true, .u8AxisCol = 0, .u8AxisRow = 0},
		{.u8ID = 2, .bLeftBoundary = false, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = true, .u8AxisCol = 1, .u8AxisRow = 0},
		{.u8ID = 3, .bLeftBoundary = false, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = true, .u8AxisCol = 2, .u8AxisRow = 0},
		{.u8ID = 4, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = true, .bBottumBoundary = true, .u8AxisCol = 3, .u8AxisRow = 0},
	},
	{
		{.u8ID = 1, .bLeftBoundary = true, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 0, .u8AxisRow = 3},
		{.u8ID = 2, .bLeftBoundary = true, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 2},
		{.u8ID = 3, .bLeftBoundary = true, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 1},
		{.u8ID = 4, .bLeftBoundary = true, .bRightBoundary = true, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 0},
	},	
};


const CubeStruct g_sLFormAxisInfo[][4] = 
{
	{
		{.u8ID = 1, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 0},
		{.u8ID = 2, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 0, .u8AxisRow = 1},
		{.u8ID = 3, .bLeftBoundary = false, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 1, .u8AxisRow = 1},
		{.u8ID = 4, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 2, .u8AxisRow = 1},
	},
	
	{
		{.u8ID = 1, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 0, .u8AxisRow = 2},
		{.u8ID = 2, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 1, .u8AxisRow = 2},
		{.u8ID = 3, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 1},
		{.u8ID = 4, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 0},
	},
	
	{
		{.u8ID = 1, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 2, .u8AxisRow = 1},
		{.u8ID = 2, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 2, .u8AxisRow = 0},
		{.u8ID = 3, .bLeftBoundary = false, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 0},
		{.u8ID = 4, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 0},
	},	
	
	{
		{.u8ID = 1, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 0},
		{.u8ID = 2, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 0},
		{.u8ID = 3, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 1},
		{.u8ID = 4, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 0, .u8AxisRow = 2},
	},	
};


const CubeStruct g_sReverseLFormAxisInfo[][4] = 
{
	{
		{.u8ID = 1, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 2, .u8AxisRow = 0},
		{.u8ID = 2, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 2, .u8AxisRow = 1},
		{.u8ID = 3, .bLeftBoundary = false, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 1, .u8AxisRow = 1},
		{.u8ID = 4, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 0, .u8AxisRow = 1},
	},
	
	{
		{.u8ID = 1, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 1, .u8AxisRow = 2},
		{.u8ID = 2, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 0, .u8AxisRow = 2},
		{.u8ID = 3, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 1},
		{.u8ID = 4, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 0},
	},
	
	{
		{.u8ID = 1, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 0, .u8AxisRow = 1},
		{.u8ID = 2, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 0},
		{.u8ID = 3, .bLeftBoundary = false, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 0},
		{.u8ID = 4, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 2, .u8AxisRow = 0},
	},	
	
	{
		{.u8ID = 1, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 0},
		{.u8ID = 2, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 0},
		{.u8ID = 3, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 1},
		{.u8ID = 4, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 1, .u8AxisRow = 2},
	},	
};


const CubeStruct g_sHillFormAxisInfo[][4] = 
{
	{
		{.u8ID = 1, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 0, .u8AxisRow = 1},
		{.u8ID = 2, .bLeftBoundary = false, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 0},
		{.u8ID = 3, .bLeftBoundary = false, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 1, .u8AxisRow = 1},
		{.u8ID = 4, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 2, .u8AxisRow = 1},
	},
	
	{
		{.u8ID = 1, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 1, .u8AxisRow = 2},
		{.u8ID = 2, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 1},
		{.u8ID = 3, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 1},
		{.u8ID = 4, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 0},
	},
	
	{
		{.u8ID = 1, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 2, .u8AxisRow = 0},
		{.u8ID = 2, .bLeftBoundary = false, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 1, .u8AxisRow = 1},
		{.u8ID = 3, .bLeftBoundary = false, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 0},
		{.u8ID = 4, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 0},
	},	
	
	{
		{.u8ID = 1, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = true, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 0},
		{.u8ID = 2, .bLeftBoundary = false, .bRightBoundary = true, .bTopBoundary = false, .bBottumBoundary = false, .u8AxisCol = 1, .u8AxisRow = 1},
		{.u8ID = 3, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = false, .u8AxisCol = 0, .u8AxisRow = 1},
		{.u8ID = 4, .bLeftBoundary = true, .bRightBoundary = false, .bTopBoundary = false, .bBottumBoundary = true, .u8AxisCol = 0, .u8AxisRow = 2},
	},	
};


FormStruct g_sReverseZForm = 
{
	.u8FormIndex = 0,
	.u8FormNum = 2,
	.pFormAxis = g_sReverseZFormAxisInfo,
};


FormStruct g_sZForm = 
{
	.u8FormIndex = 0,
	.u8FormNum = 2,
	.pFormAxis = g_sZFormAxisInfo,
};


FormStruct g_sSquareForm = 
{
	.u8FormIndex = 0,
	.u8FormNum = 1,
	.pFormAxis = g_sSquareAxisInfo,
};


FormStruct g_sBarForm = 
{
	.u8FormIndex = 0,
	.u8FormNum = 2,
	.pFormAxis = g_sBarFormAxisInfo,
};


FormStruct g_sLForm = 
{
	.u8FormIndex = 0,
	.u8FormNum = 4,
	.pFormAxis = g_sLFormAxisInfo,
};

FormStruct g_sReverseLForm = 
{
	.u8FormIndex = 0,
	.u8FormNum = 4,
	.pFormAxis = g_sReverseLFormAxisInfo,
};

FormStruct g_sHillForm = 
{
	.u8FormIndex = 0,
	.u8FormNum = 4,
	.pFormAxis = g_sHillFormAxisInfo,
};


typedef struct
{
	CubeColor eColor;
	bool bActive;
}SingleCubeStruct;

struct
{
	SingleCubeStruct sSingleCubeTrace[TETRIS_ROW_CUBE_NUM];	
}g_sTetrisTable[TETRIS_COL_CUBE_NUM];


// 当前方块管理
struct
{
	CubeColor eColor;
	CubeColor eColorLast;
	uint8_t u8StartX;
	uint8_t u8StartY;
	uint8_t u8EndX;
	uint8_t u8EndY;
	FormStruct *pForm;
	FormType eForm;
	FormType eFormLast;
}g_sCurForm;



void UpdateForm(const UpdateType eType)
{
	for (uint8_t i = 0; i < 4; i++)
	{
		uint8_t u8ColIndex = g_sCurForm.u8StartX + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisCol;
		uint8_t u8RowIndex = g_sCurForm.u8StartY + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisRow;
		g_sTetrisTable[u8ColIndex].sSingleCubeTrace[u8RowIndex].eColor = CUBE_COLOR_WHITE;
		g_sTetrisTable[u8ColIndex].sSingleCubeTrace[u8RowIndex].bActive = false;
		
		if (u8RowIndex)
		{
			SendCubeCmd(u8RowIndex - 1, u8ColIndex, CUBE_COLOR_WHITE);
		}
	}
	
	switch (eType)
	{
		case TYPE_MOVE_DOWN:
			g_sCurForm.u8StartY++;
			break;
		
		case TYPE_MOVE_LEFT:
			g_sCurForm.u8StartX--;
			break;
		
		case TYPE_MOVE_RIGHT:
			g_sCurForm.u8StartX++;
			break;
		
		case TYPE_MOVE_ROTATE:
			g_sCurForm.pForm->u8FormIndex = (g_sCurForm.pForm->u8FormIndex + 1) % g_sCurForm.pForm->u8FormNum;
			break;
		
		
		default:
			break;
	}
	
	for (uint8_t i = 0; i < 4; i++)
	{
		uint8_t u8ColIndex = g_sCurForm.u8StartX + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisCol;
		uint8_t u8RowIndex = g_sCurForm.u8StartY + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisRow;
		g_sTetrisTable[u8ColIndex].sSingleCubeTrace[u8RowIndex].eColor = g_sCurForm.eColor;
		g_sTetrisTable[u8ColIndex].sSingleCubeTrace[u8RowIndex].bActive = true;
		if (u8RowIndex)
		{
			SendCubeCmd(u8RowIndex - 1, u8ColIndex, g_sCurForm.eColor);
		}
	}

}



void RotateForm(void)
{
	bool bRotateAllow = false;
	
	if (g_sCurForm.pForm)
	{
		
		// 预判
		uint8_t u8PreTransIndex = (g_sCurForm.pForm->u8FormIndex + 1) % g_sCurForm.pForm->u8FormNum;
		uint8_t u8TabelRow = 0;
		uint8_t u8TableCol = 0;
		
		for (uint8_t i = 0; i < 4; i++)
		{
			uint8_t u8ColIndex = g_sCurForm.u8StartX + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisCol;
			uint8_t u8RowIndex = g_sCurForm.u8StartY + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisRow;
			g_sTetrisTable[u8ColIndex].sSingleCubeTrace[u8RowIndex].bActive = false;
		}
		
		for (uint8_t i = 0; i < 4; i++)
		{
			u8TabelRow = g_sCurForm.u8StartY + g_sCurForm.pForm->pFormAxis[u8PreTransIndex][i].u8AxisRow;
			u8TableCol = g_sCurForm.u8StartX + g_sCurForm.pForm->pFormAxis[u8PreTransIndex][i].u8AxisCol;
			
			if (u8TableCol >= (TETRIS_COL_CUBE_NUM - 1))
			{
				bRotateAllow = false;
				break;
			}
			
			if (!g_sTetrisTable[u8TableCol].sSingleCubeTrace[u8TabelRow].bActive)
			{
				bRotateAllow = true;
			}
			else
			{
				bRotateAllow = false;
				break;
			}
		}
	}	

	if (bRotateAllow)
	{
		UpdateForm(TYPE_MOVE_ROTATE);
	}
	else
	{
		for (uint8_t i = 0; i < 4; i++)
		{
			uint8_t u8ColIndex = g_sCurForm.u8StartX + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisCol;
			uint8_t u8RowIndex = g_sCurForm.u8StartY + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisRow;
			g_sTetrisTable[u8ColIndex].sSingleCubeTrace[u8RowIndex].bActive = true;
		}
	}
}


bool LeftMoveAllow(void)
{
	bool bRet = false;
	
	for (uint8_t i = 0; i < 4; i++)
	{
		if (g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].bLeftBoundary)
		{
			uint8_t u8TabelRow = g_sCurForm.u8StartX + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisRow;
			uint8_t u8TableCol = g_sCurForm.u8StartX + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisCol;
			if (g_sTetrisTable[u8TableCol - 1].sSingleCubeTrace[u8TabelRow].bActive)
			{
				bRet = false;
				break;
			}
			else if (u8TableCol == 0)
			{
				bRet = false;
				break;
			}
			else
			{
				bRet = true;
			}
		}
	}
	
	return bRet;
}


void MoveLeft(void)
{
	if (g_sCurForm.pForm)
	{
		// 检测触边
		if (LeftMoveAllow())
		{
			UpdateForm(TYPE_MOVE_LEFT);
		}
	}
}


bool RightMoveAllow(void)
{
	bool bRet = false;
	
	for (uint8_t i = 0; i < 4; i++)
	{
		if (g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].bRightBoundary)
		{
			uint8_t u8TabelRow = g_sCurForm.u8StartX + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisRow;
			uint8_t u8TableCol = g_sCurForm.u8StartX + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisCol;
			if (g_sTetrisTable[u8TableCol + 1].sSingleCubeTrace[u8TabelRow].bActive)
			{
				bRet = false;
				break;
			}
			else if (u8TableCol == (TETRIS_COL_CUBE_NUM - 1))
			{
				bRet = false;
				break;
			}
			else
			{
				bRet = true;
			}
		}
	}
	
	return bRet;
}

void MoveRight(void)
{
	if (g_sCurForm.pForm)
	{
		// 检测触边
		if (RightMoveAllow())
		{
			UpdateForm(TYPE_MOVE_RIGHT);
		}
	}
}


bool MoveDown(void)
{
	bool bRet = true;
	
	if (g_sCurForm.pForm)
	{
		uint8_t u8TabelRow = 0;
		uint8_t u8TableCol = 0;
		for (uint8_t i = 0; i < 4; i++)
		{
			if (g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].bBottumBoundary)
			{
				u8TabelRow = g_sCurForm.u8StartY + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisRow;
				u8TableCol = g_sCurForm.u8StartX + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisCol;
				if (u8TabelRow >= (TETRIS_ROW_CUBE_NUM - 1))
				{
					bRet = false;
					break;
				}
				else if (g_sTetrisTable[u8TableCol].sSingleCubeTrace[u8TabelRow + 1].bActive)
				{
					bRet = false;
					break;
				}
			}
		}
	}
	else
	{
		bRet = false;
	}
	
	if (bRet)
	{
		// 可成功下落
		UpdateForm(TYPE_MOVE_DOWN);
	}
	else
	{
		if (g_sCurForm.u8StartY <= g_sControlManager.u8MinLevelRow)
		{
			g_sControlManager.u8MinLevelRow = g_sCurForm.u8StartY;
		}
	}
	
	return bRet;
}


static uint16_t GetU16LittleEndian(const uint16_t u16Value)
{
	uint8_t u8Hi = u16Value >> 8;
	uint8_t u8Lo = u16Value & 0xFF;
	
	return ((uint16_t)u8Lo << 8) | u8Hi;
}


void CbButtonFunc(const HMI_Widget eWidget, const void *pReturnData)
{
	if (HMI_BUTTON_DIR_ADDR_RD == eWidget)
	{
		if (g_sControlManager.bRun)
		{
			switch (GetU16LittleEndian(*(uint16_t *)pReturnData))
			{
				case 0x5575:
					g_sControlManager.bTransForm = true;
					break;
				
				case 0x4464:
					g_sControlManager.bDown = true;
					break;
				
				case 0x4C6C:
					g_sControlManager.bMoveLeft = true;
					break;
				
				case 0x5272:
					g_sControlManager.bMoveRight = true;
					break;
			}
		}
	}
	else if (HMI_BUTTON_STATE_ADDR_RD == eWidget)
	{
		if (0x0D0D == GetU16LittleEndian(*(uint16_t *)pReturnData))
		{
			g_sControlManager.bRun = !g_sControlManager.bRun;
		}
	}
	else if (HMI_BUTTON_DOWN_RD == eWidget)
	{
		if (0x0100 == GetU16LittleEndian(*(uint16_t *)pReturnData))
		{
			g_sControlManager.u32SleepInterval = 200;
		}
		else if (0x0300 == GetU16LittleEndian(*(uint16_t *)pReturnData))
		{
			g_sControlManager.u32SleepInterval = 500;
		}
	}
}


void HMI_RegisterWidgetEvent(const HMI_Widget eWidget, void (*pCbFunc)(const HMI_Widget eWidget, const void *pReturnData));


void TetrisInit(void)
{
	memset(g_sTetrisTable, 0, sizeof(g_sTetrisTable));
	FormStruct sForms[] = {g_sReverseZForm, g_sZForm, g_sSquareForm, g_sBarForm, g_sLForm, g_sReverseLForm, g_sHillForm};
	
	for (uint8_t i = 0; i < sizeof(sForms) / sizeof(sForms[0]); i++)
	{
		sForms[i].u8FormIndex = 0;
	}
	
	g_sControlManager.u8MinLevelRow = TETRIS_ROW_CUBE_NUM - 1;
	srand(g_sControlManager.u32RandValue);
}



uint8_t GetRandomNum(const uint8_t u8LowerLimit, const uint8_t u8UpperLimit)
{
	uint32_t u32CurRand = 0;
	
	srand(g_sControlManager.u32RandValue + rand());
	u32CurRand = rand();

	return u32CurRand % u8UpperLimit + u8LowerLimit;
}


void GenerateNewForm(void)
{
	g_sCurForm.eColor = GetRandomNum(CUBE_COLOR_RED, CUBE_COLOR_BLUE);
	if (g_sCurForm.eColor == g_sCurForm.eColorLast)
	{
		g_sCurForm.eColor = (g_sCurForm.eColor != CUBE_COLOR_BLUE) ? (g_sCurForm.eColor + CUBE_COLOR_RED) : CUBE_COLOR_RED;
	}
	g_sCurForm.eColorLast = g_sCurForm.eColor;
	
	g_sCurForm.eForm = GetRandomNum(FORM_TYPE_SQUARE, FORM_TYPE_HILL);
	if (g_sCurForm.eForm == g_sCurForm.eFormLast)
	{
		g_sCurForm.eForm = (g_sCurForm.eForm != FORM_TYPE_HILL) ? (g_sCurForm.eForm + FORM_TYPE_SQUARE) : FORM_TYPE_SQUARE;
	}
	g_sCurForm.eFormLast = g_sCurForm.eForm;
	
	switch (g_sCurForm.eForm)
	{
		case FORM_TYPE_SQUARE:
			g_sCurForm.pForm = &g_sSquareForm;
			break;
		
		case FORM_TYPE_BAR:
			g_sCurForm.pForm = &g_sBarForm;
			break;
		
		case FORM_TYPE_Z:
			g_sCurForm.pForm = &g_sZForm;
			break;
		
		case FORM_TYPE_RZ:
			g_sCurForm.pForm = &g_sReverseZForm;
			break;
		
		case FORM_TYPE_L:
			g_sCurForm.pForm = &g_sLForm;
			break;
		
		case FORM_TYPE_RL:
			g_sCurForm.pForm = &g_sReverseLForm;
			break;
		
		case FORM_TYPE_HILL:	
			g_sCurForm.pForm = &g_sHillForm;
			break;
		
		default:
			g_sCurForm.pForm = NULL;
			break;
	}
	
	g_sCurForm.u8StartX = TETRIS_COL_CUBE_NUM / 2 - 1;
	g_sCurForm.u8StartY = 0;
	g_sCurForm.pForm->u8FormIndex = 0;
	
	for (uint8_t i = 0; i < 4; i++)
	{
		uint8_t u8ColIndex = g_sCurForm.u8StartX + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisCol;
		uint8_t u8RowIndex = g_sCurForm.u8StartY + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisRow;
		g_sTetrisTable[u8ColIndex].sSingleCubeTrace[u8RowIndex].bActive = true;
		
		if (u8RowIndex)
		{
			SendCubeCmd(u8RowIndex - 1, u8ColIndex, g_sCurForm.eColor);
		}		
	}
}


bool EliminateRow(void)
{
	uint8_t u8EliminateRow = 0;
	
	uint8_t u8MinLevelRow = g_sControlManager.u8MinLevelRow;
	g_sControlManager.u8MinLevelRowUpdateStart = g_sControlManager.u8MinLevelRow;
	
	for (uint8_t i = u8MinLevelRow; i < TETRIS_ROW_CUBE_NUM; i++)
	{
		bool bActive = true;
		for (uint8_t j = 0; j < TETRIS_COL_CUBE_NUM; j++)
		{
			bActive = bActive & g_sTetrisTable[j].sSingleCubeTrace[i].bActive;
			if (!bActive)
			{
				break;
			}
		}
		
		if (bActive)
		{
			
			// 拷贝行
			for (uint8_t j = 0; j < TETRIS_COL_CUBE_NUM; j++)
			{
				for (uint8_t u8Row = i; u8Row > u8MinLevelRow; u8Row--)
				{
					g_sTetrisTable[j].sSingleCubeTrace[u8Row] = g_sTetrisTable[j].sSingleCubeTrace[u8Row - 1];
				}
			}
			
			
			for (uint8_t j = 0; j < TETRIS_COL_CUBE_NUM; j++)
			{
				g_sTetrisTable[j].sSingleCubeTrace[u8MinLevelRow].eColor = CUBE_COLOR_WHITE;
				g_sTetrisTable[j].sSingleCubeTrace[u8MinLevelRow].bActive = false;
			}
			
			u8EliminateRow++;
			g_sControlManager.u8MinLevelRow++;
		}
	}
	
	return (u8EliminateRow != 0);
}


bool CheckEliminateResult(void)
{
	static enum
	{
		SET_UPDATE_PARAM = 0,
		SEND_CMD,
		WAIT_RESULT,
	}EliminateStep = SET_UPDATE_PARAM;
	
	bool bRet = false;
	static uint8_t u8RowIndex = 0;
	static uint8_t u8ColIndex = 0;
	
	switch (EliminateStep)
	{
		case SET_UPDATE_PARAM:
			u8RowIndex = g_sControlManager.u8MinLevelRowUpdateStart;
			u8ColIndex = 0;
			EliminateStep = SEND_CMD;
			break;
		
		case SEND_CMD:
			SendCubeCmd(u8RowIndex - 1, u8ColIndex, g_sTetrisTable[u8ColIndex].sSingleCubeTrace[u8RowIndex].eColor);
			u8ColIndex++;
		
			EliminateStep = WAIT_RESULT;
			if (u8ColIndex >= TETRIS_COL_CUBE_NUM)
			{
				u8ColIndex = 0;
				u8RowIndex++;
				
				if (u8RowIndex >= TETRIS_ROW_CUBE_NUM)
				{
					EliminateStep = SET_UPDATE_PARAM;
					bRet = true;
				}
			}
			break;
			
		case WAIT_RESULT:
			if (!ScreenBusy())
			{
				EliminateStep = SEND_CMD;
			}
			break;
	}
	
	
	return bRet;
}


/**
  * @brief  判断游戏是否失败, 必须用在图形已经落定后
  * @param  None 
  * @retval None
  */
bool GameFailed(void)
{
	bool bRet = false;
	for (uint8_t i = 0; i < 4; i++)
	{
		uint8_t u8RowIndex = g_sCurForm.u8StartY + g_sCurForm.pForm->pFormAxis[g_sCurForm.pForm->u8FormIndex][i].u8AxisRow;
		
		if (1 == u8RowIndex)
		{
			ResetHMI();
			bRet = true;
			break;
		}		
	}
	
	return bRet;
}


/**
  * @brief  逻辑处理流程
  * @param  None 
  * @retval None
  */
void TetrisHandle(void)
{
	static enum
	{
		REGISTER_HMI = 0,
		RESET_ALL,
		WAIT_START,
		GENERATE_NEW_FORM,
		FORM_HANDLE,
		UPDATE_RESULT,
		ELIMINATE_ROW,
	}FsmStep = REGISTER_HMI;
	
	static uint32_t u32TetrisTimer = 0;
	
	switch (FsmStep)
	{
		case REGISTER_HMI:
			// 注册HMI相关变量的回调函数
			HMI_RegisterWidgetEvent(HMI_BUTTON_DIR_ADDR_RD, CbButtonFunc);
			HMI_RegisterWidgetEvent(HMI_BUTTON_STATE_ADDR_RD, CbButtonFunc);
			HMI_RegisterWidgetEvent(HMI_BUTTON_DOWN_RD, CbButtonFunc);
			FsmStep = RESET_ALL;
			break;
		
		case RESET_ALL:
			TetrisInit();			
			FsmStep = WAIT_START;
			break;
			
		case WAIT_START:
			if (g_sControlManager.bRun)
			{
				FsmStep = GENERATE_NEW_FORM;
			}
			break;
		
		case GENERATE_NEW_FORM:
			GenerateNewForm();
			FsmStep = FORM_HANDLE;
			ResetTimerCount(&u32TetrisTimer);
			g_sControlManager.u32SleepInterval = 500;
			break;
		
		case FORM_HANDLE:
			if (GetTimerTickDelta(u32TetrisTimer, GetCurTimerCount()) >= g_sControlManager.u32SleepInterval)
			{
				if (g_sControlManager.bRun && !ScreenBusy())
				{
					if (g_sControlManager.bMoveLeft)
					{
						g_sControlManager.bMoveLeft = false;
						MoveLeft();						
						ResetTimerCount(&u32TetrisTimer);
					}
					else if (g_sControlManager.bMoveRight)
					{
						g_sControlManager.bMoveRight = false;
						MoveRight();						
						ResetTimerCount(&u32TetrisTimer);
					}
					else if (g_sControlManager.bTransForm)
					{
						g_sControlManager.bTransForm = false;
						RotateForm();
						ResetTimerCount(&u32TetrisTimer);
					}
					else
					{
						if (!MoveDown())
						{
							FsmStep = UPDATE_RESULT;
							
						}
						ResetTimerCount(&u32TetrisTimer);
					}
				}
			}
			break;
		
		case UPDATE_RESULT:
			if(EliminateRow())
			{
				FsmStep = ELIMINATE_ROW;
			}
			else if (GameFailed())
			{
				FsmStep = RESET_ALL;
			}
			else
			{
				FsmStep = GENERATE_NEW_FORM;
			}
			break;
			
		case ELIMINATE_ROW:
			if (CheckEliminateResult())
			{
				FsmStep = GENERATE_NEW_FORM;
			}
			break;
	}
	
	g_sControlManager.u32RandValue++;
}


