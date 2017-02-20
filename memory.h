#ifndef __MEMORY_H
#define __MEMORY_H

#include "globals.h"

#define SP_START_ADDR   0x04000000
#define SP_END          0x04080007
#define SP_SIZE			(SP_END - SP_START_ADDR +1)

void Init_R_AND_W(uint8 * MemoryRange, uint32 startAddress, uint32 endAddress);
void DynInit_R_AND_W(uint8 * MemoryRange, uint32 startAddress, uint32 endAddress);

void InitMemoryLookupTables(void);

void InitVirtualMemory(void);
void FreeVirtualMemory(void);
void InitVirtualRomMemory(uint32 filesize);
void FreeVirtualRomMemory(void);
void LockVirtualRomMemory(void);
void UnlockVirtualRomMemory(void);

extern uint32 valloc;
extern uint32 valloc2;

#endif