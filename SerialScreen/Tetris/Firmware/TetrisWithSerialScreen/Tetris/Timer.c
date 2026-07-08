#include "Timer.h"

uint32_t GetCurTimerCount(void)
{
	return HAL_GetTick();
}


void ResetTimerCount(uint32_t *pCount)
{
	*pCount = HAL_GetTick();
}


uint32_t GetTimerTickDelta(const uint32_t u32LastCount, const uint32_t u32CurCount)
{
    uint32_t u32Delta = 0;
    u32Delta = (u32CurCount >= u32LastCount) ? (u32CurCount - u32LastCount) : (0XFFFFFFFF - u32LastCount + u32CurCount + 1);
    
    return u32Delta;
}