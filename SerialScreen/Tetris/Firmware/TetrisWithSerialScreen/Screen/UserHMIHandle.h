#ifndef USERHMIHANDLE_H
#define USERHMIHANDLE_H

#include "main.h"

#define CMD_BUFFER_LEN		12

void UserHMIHandleInit(void);
void UserHMIHandleTask(void);
bool ScreenBusy(void);
bool SendCubeCmd(const uint8_t u8Row, const uint8_t u8Col, const uint8_t u8ColorIndex);
void ResetHMI(void);

#endif
