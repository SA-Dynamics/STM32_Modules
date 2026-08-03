#include "MemoryHandle.h"
#include <string.h>
#include <stdio.h>

#if defined(STM32F103xB)
#include "../FLASH_STM32F1/flash_if.h"
#define MEMORY_HANDLE_ADDR_START		0x08004000
#define BACKUP_ADDR_START				0x0800F400
#elif defined(STM32F407xx)
#include "../FLASH_STM32F4/flash_if.h"
#define MEMORY_HANDLE_ADDR_START		0x08004000
#define BACKUP_ADDR_START				0x0800F400
#endif



bool ReadSingleMemoryInfo(const ProgramInfoIndex eInfo, uint8_t *pData)
{
	bool bRet = false;
	
	__IO uint8_t *pDataMem = (__IO uint8_t*) MEMORY_HANDLE_ADDR_START;
	
	// 校验数据
	uint8_t u8CheckSum = 0;
	for (uint8_t i = 0; i < MEMORY_DATA_NUM - 1; i++)
	{
		u8CheckSum ^= pDataMem[i];
	}
	
	if (u8CheckSum == pDataMem[MEMORY_DATA_NUM - 1]) 
	{
		*pData = pDataMem[eInfo];
		bRet = true;
	}
	
	return bRet;
}


void ReadAllMemoryInfo(uint8_t **pData)
{
	bool bRet = false;
	
	__IO uint8_t *pDataMem = (__IO uint8_t*) MEMORY_HANDLE_ADDR_START;
	
	memcpy(pData, (void *)pDataMem, MEMORY_DATA_NUM);
}


bool WriteDiscontinuousMemoryInfo(const ProgramInfoData *pData, const uint8_t u8Len)
{
	uint8_t u8MemData[8] = {0xFF};	
	ReadAllMemoryInfo((uint8_t **)&u8MemData);
	
	for (uint8_t i = 0; i < u8Len; i++)
	{
		u8MemData[pData[i].eIndex] = pData[i].u8Data;
	}
		
	if (FLASH_If_Erase(MEMORY_HANDLE_ADDR_START, 1))
	{
		printf("%s,erase failed:", __FUNCTION__);
		return false;
	}
	
	
	if (FLASH_If_Write(MEMORY_HANDLE_ADDR_START, (uint32_t *)u8MemData, 2))
	{
		printf("%s,write failed:", __FUNCTION__);
		return false;
	}
	
	if (FLASH_If_Erase(BACKUP_ADDR_START, 1))
	{
		printf("%s,erase backup failed:", __FUNCTION__);
		return false;
	}
	
	if (FLASH_If_Write(BACKUP_ADDR_START, (uint32_t *)u8MemData, 2))
	{
		printf("%s,write backup failed:", __FUNCTION__);
		return false;
	}
	
	return true;
}


bool WriteAllMemoryInfo(const uint8_t *pData)
{
	uint8_t u8MemData[8] = {0xFF};	
	memcpy(u8MemData, pData, 8);
		
	if (FLASH_If_Erase(MEMORY_HANDLE_ADDR_START, 1))
	{
		return false;
	}
	
	
	if (FLASH_If_Write(MEMORY_HANDLE_ADDR_START, (uint32_t *)u8MemData, 2))
	{
		return false;
	}
	
	if (FLASH_If_Erase(BACKUP_ADDR_START, 1))
	{
		return false;
	}
	
	if (FLASH_If_Write(BACKUP_ADDR_START, (uint32_t *)u8MemData, 2))
	{
		return false;
	}
	
	return true;
}

