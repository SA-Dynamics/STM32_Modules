#ifndef TETRIS_H
#define TETRIS_H

#include "main.h"


typedef enum
{
	CONTROL_EVENT_NONE = 0,
	CONTROL_EVENT_MOVE_RIGHT,
	CONTROL_EVENT_MOVE_LEFT,
	CONTROL_EVENT_ROTATE,
	CONTROL_EVENT_DROP,
}ControlEvent;


void TetrisInit(void);
void TetrisHandle(void);


#endif
