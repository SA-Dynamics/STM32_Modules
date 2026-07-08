#ifndef HMI_OPERATIONLOGIC_H
#define HMI_OPERATIONLOGIC_H

#include "main.h"
#include "HMI_VariableManager.h"


void HMI_ControlInit(void);
void HMI_ControlRun(void);
void HMI_LinkWriteVariable(const HMI_Widget eWidget, void *pVariable);
bool HMI_UpdateVariable(const HMI_Widget eWidget);
void HMI_GetWidgetValue(const HMI_Widget eWidget, void **pData);
void HMI_RegisterWidgetEvent(const HMI_Widget eWidget, void (*pCbFunc)(const HMI_Widget eWidget, const void *pReturnData));


#endif
