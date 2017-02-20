#ifndef __MEMORY_H
/*______________________________________________________________________________
 |                                                                              |
 |  1964 - memory.h                                                             |
 |  Copyright (C) 2001 Joel Middendorf, <schibo@emulation64.com>                |
 |                                                                              |
 |  This program is free software; you can redistribute it and/or               |
 |  modify it under the terms of the GNU General Public License                 |
 |  as published by the Free Software Foundation; either version 2              |
 |  of the License, or (at your option) any later version.                      |
 |                                                                              |
 |  This program is distributed in the hope that it will be useful,             |
 |  but WITHOUT ANY WARRANTY; without even the implied warranty of              |
 |  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               |
 |  GNU General Public License for more details.                                |
 |                                                                              |
 |  You should have received a copy of the GNU General Public License           |
 |  along with this program; if not, write to the Free Software                 |
 |  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. |
 |                                                                              |
 |  To contact the author:                                                      |
 |  email      : dyangchicago@yahoo.com, schibo@emulation64.com                 |
 |  paper mail :                                                                |
 |______________________________________________________________________________|
*/

#define __MEMORY_H

#include "globals.h"
#include "options.h"
#include "hardware.h"

#ifdef DYNAEXECUTIONMAPMEMORY
uint8* dynarommap[0x20000];
#else
//extern uint8  DynaRDRAM[0x00400000];
extern uint8  DynaRDRAM[0x00800000];
extern uint32 DynaSP_REG[0x800];

extern uint8* DynaROM;
#endif


#define SP_START_ADDR   0x04000000
#define SP_END          0x04080007
#define SP_SIZE			(SP_END - SP_START_ADDR +1)

void Init_R_AND_W(uint8** sDWORD_R, uint8 * MemoryRange, uint32 startAddress, uint32 endAddress);
void DynInit_R_AND_W(uint8 * MemoryRange, uint32 startAddress, uint32 endAddress);

void InitMemoryLookupTables(void);

void InitVirtualMemory(void);
void FreeVirtualMemory(void);
void InitVirtualRomMemory(uint32 filesize);
void FreeVirtualRomMemory(void);
void ResetRdramSize(int setsize);
BOOL UnmappedMemoryExceptionHelper(uint32 addr);

#ifdef ENABLE_OPCODE_DEBUGGER
void Debugger_Copy_Memory(MemoryState *target, MemoryState *source);
#endif

void ROM_CheckSumZelda();
void ROM_CheckSumMario();

extern uint32 current_rdram_size;
extern uint32 RDRAM_End_Address;
extern uint32 RDRAM_Begin_Address;
extern uint32 ROM_End_Address;
extern uint32 ROM_Begin_Address;
extern uint32 rdram_sizes[3];

// Memory segment size definition
#define MEMORY_SIZE_RDRAM       0x400000
#define MEMORY_SIZE_EXRDRAM     0x400000
//#define MEMORY_SIZE_RDREG       0x100000
#define MEMORY_SIZE_RAMREGS0    0x30
#define MEMORY_SIZE_RAMREGS4    0x30
#define MEMORY_SIZE_RAMREGS8    0x30
#define MEMORY_SIZE_SPMEM		0x2000
#define MEMORY_SIZE_SPREG_1     0x24
#define MEMORY_SIZE_SPREG_2		0x8
#define MEMORY_SIZE_DPC         0x20
#define MEMORY_SIZE_DPS         0x10
#define MEMORY_SIZE_MI          0x10
#define MEMORY_SIZE_VI          0x50
#define MEMORY_SIZE_AI          0x18
#define MEMORY_SIZE_PI          0x4C
#define MEMORY_SIZE_RI          0x20
#define MEMORY_SIZE_SI          0x1C
#define MEMORY_SIZE_C2A1		0x8000
#define MEMORY_SIZE_C1A1		0x8000
#define MEMORY_SIZE_C2A2		0x20000
#define MEMORY_SIZE_GIO_REG		0x804
#define MEMORY_SIZE_C1A3		0x8000
#define MEMORY_SIZE_PIF			0x800
#define MEMORY_SIZE_DUMMY		0x10000


// Memory masks
#define ADDRESS_MASK_C2A1		0x00007FFF	// 32KB SRAM start at 0x05000000
#define ADDRESS_MASK_C1A1		0x00007FFF	// 32KB SRAM start at 0x06000000
#define ADDRESS_MASK_C2A2		0x0001FFFF	// 128KB FLASHRAM/SRAM start at 0x08000000
//#define ADDRESS_MASK_C2A2		0x00007FFF	// 128KB FLASHRAM/SRAM start at 0x08000000
#define ADDRESS_MASK_C1A3		0x00007FFF	// 32KB SRAM start at 0x1FD00000

// Memory segment start address / end address definition
#define MEMORY_START_RDRAM      0x00000000
#define MEMORY_START_EXRDRAM    0x00400000
//#define MEMORY_START_RDREG      0x03F00000
#define MEMORY_START_RAMREGS0   0x03F00000
#define MEMORY_START_RAMREGS4   0x03F04000
#define MEMORY_START_RAMREGS8   0x03F80000
#define MEMORY_START_SPMEM      0x04000000
#define MEMORY_START_SPREG_1	0x04040000
#define MEMORY_START_SPREG_2	0x04080000
#define MEMORY_START_DPC        0x04100000
#define MEMORY_START_DPS        0x04200000    
#define MEMORY_START_MI         0x04300000
#define MEMORY_START_VI         0x04400000
#define MEMORY_START_AI         0x04500000
#define MEMORY_START_PI         0x04600000
#define MEMORY_START_RI         0x04700000
#define MEMORY_START_SI         0x04800000
#define MEMORY_START_C2A1		0x05000000
#define MEMORY_START_C1A1		0x06000000
#define MEMORY_START_C2A2		0x08000000
#define MEMORY_START_ROM_IMAGE  0x10000000
#define MEMORY_START_GIO		0x18000000
#define MEMORY_START_PIF		0x1FC00000
#define MEMORY_START_C1A3		0x1FD00000
#define MEMORY_START_DUMMY      0x1FFF0000

#endif