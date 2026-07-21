#include "ApplicationDemo.h"
#include "Timer.h"
#include "main.h"
#include "usart.h"
#include "../../UpdateProcess/UpdateProcess.h"


/**
  * @brief  串口打印
  * @param  None
  * @retval None
  */
static void App_Control(void)
{
	static uint32_t u32TimerCount = 0;
	
	// 如果IAP流程当前正在运行, 就停止此任务的运行
	if (UpdateProcessBusy())
	{
		return;
	}
	
	if (GetTimerTickDelta(u32TimerCount, GetCurTimerCount()) >= 1000)
	{
		ResetTimerCount(&u32TimerCount);
		HAL_GPIO_TogglePin(RunningLED_GPIO_Port, RunningLED_Pin);
		
		printf("this is a test, stm32f103c8t6\r\n");	
	}
}


void ApplicationDemoInit(void)
{
	UpdateProcessInit();
}


/**
  * @brief  程序示例, 不建议在内部使用任何阻塞延时, 比如HAL_Delay, 以及
  *			慎用任何可能引起长时间等待的逻辑, 比如while
  * @param  None
  * @retval None
  */
void ApplicationDemoHandle(void)
{
	App_Control();
	UpdateProcess();
}