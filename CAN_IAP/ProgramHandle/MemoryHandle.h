#ifndef MEMORYHANDLE_H
#define MEMORYHANDLE_H

#include "main.h"

#define MEMORY_DATA_NUM		6

typedef enum
{
	PROGRAM_FLAG_INDEX = 0,
	APP1_FLAG_INDEX,
	APP2_FLAG_INDEX,
	APP_ACTIVE_INDEX,
	LAST_APP_INDEX,
}ProgramInfoIndex;

typedef struct
{
	ProgramInfoIndex eIndex;
	uint8_t u8Data;
}ProgramInfoData;

bool ReadSingleMemoryInfo(const ProgramInfoIndex eInfo, uint8_t *pData);
void ReadAllMemoryInfo(uint8_t **pData);
bool WriteDiscontinuousMemoryInfo(const ProgramInfoData *pData, const uint8_t u8Len);
bool WriteAllMemoryInfo(const uint8_t *pData);

#endif
