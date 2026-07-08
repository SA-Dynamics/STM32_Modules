#ifndef HMI_OPERATIONLOGIC_H
#define HMI_OPERATIONLOGIC_H

#include "main.h"
#include "HMI_VariableManager.h"


void HMI_ControlInit(void);
void HMI_ControlRun(void);
void HMI_LinkWriteVariable(const HMI_Widget eWidget, void *pVariable);
bool HMI_SwitchPage(const uint16_t u16PageIndex);
bool HMI_PopWindow(const uint16_t u16Index);
bool HMI_CancelPopWindow(void);
bool HMI_UpdateVariable(const HMI_Widget eWidget);
bool HMI_RegisterCurveVar(const uint8_t u8Channel, const HMI_Widget eWidget);
void HMI_GetWidgetValue(const HMI_Widget eWidget, void **pData);
void HMI_RegisterWidgetEvent(const HMI_Widget eWidget, void (*pCbFunc)(const HMI_Widget eWidget, const void *pReturnData));


#endif
