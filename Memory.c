/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Emulator for Nintendo 64 console system                              |
 |  Copyright (C) 2001  Joel Middendorf  schibo@emulation64.com                 |
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
 |  email      : schibo@emulation64.com                                         |
 |  paper mail :                                                                |
 |______________________________________________________________________________|
/*

  The project is a direct documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar. Most source
code comments are taken directly from anarko's n64toolkit with consent and are 
the property of anarko.
*/

#include <windows.h>
#include <windowsx.h>
#include <memory.h>
#include <process.h>
#include <malloc.h>
#include "options.h"
#include "globals.h"
#include "r4300i.h"
#include "memory.h"
#include "n64rcp.h"
#include "interrupt.h"
#include "iPIF.h"
#include "1964ini.h"
#include "Dynarec\x86.h"
#include "hardware.h"
#include "win32/windebug.h"
#include "win32\wingui.h"

void __cdecl LogDyna(char *debug, ...);

//---------------------------------------------------------
// setsize = 0      Default size
// setsize = RDRAMSIZE_4MB      4MB
// setsize = RDRAMSIZE_8MB      8MB
uint32 rdram_sizes[3] = { 0x400000, 0x400000, 0x800000 };
//uint32 current_rdram_size = 0x800000;
uint32 current_rdram_size = 0x400000;
uint32 RDRAM_End_Address = 0x807FFFFF;
uint32 RDRAM_Begin_Address = 0x80000000;
uint32 ROM_End_Address;
uint32 ROM_Begin_Address = 0xA0000000;

#include "globals.h"

#ifdef DYNAEXECUTIONMAPMEMORY
uint8* dynarommap[0x20000];
#else
//uint8 DynaRDRAM[0x00400000];  // Size = 4MB
//uint8 RDRAM[0x00400000];      // Size = 4MB
uint8 DynaRDRAM[0x00800000];    // Size = 8MB

uint32 DynaSP_REG[0x800];
uint8* DynaROM = NULL;
#endif

uint8 *dummymap;

//---------------------------------------------------------

void Init_R_AND_W(uint8** sDWORD_R, uint8* MemoryRange, uint32 startAddress, uint32 size)
{
    uint8* pTmp = (uint8*)MemoryRange;
    
    uint32 curSegment = ((startAddress ) >> 16);
    uint32 endSegment = ((startAddress+size-1) >> 16);

    while (curSegment <= endSegment) {
        //sDWORD_R[curSegment | 0x0000] = pTmp;
        sDWORD_R[curSegment | 0x8000] = pTmp;
        sDWORD_R[curSegment | 0xA000] = pTmp;
        pTmp += 0x10000;
        curSegment++;
    }
}

//---------------------------------------------------------
// Init DynaMemory Lookup Table
// Function is called from fileio.c
// right after a new ROM image is loaded
//---------------------------------------------------------
void DynInit_R_AND_W(uint8* MemoryRange, uint32 startAddress, uint32 endAddress)
{
    uint8* pTmp = (uint8*)MemoryRange;
    uint32 curSegment = ((startAddress & 0x1FFFFFFF) >> 16); //?????????
    uint32 endSegment = ((endAddress   & 0x1FFFFFFF) >> 16); //?????????

    while (curSegment <= endSegment) {
        //sDYN_PC_LOOKUP[curSegment | 0x0000] = pTmp;
        sDYN_PC_LOOKUP[curSegment | 0x8000] = pTmp;
        sDYN_PC_LOOKUP[curSegment | 0xA000] = pTmp;
        pTmp += 0x10000;
        curSegment++;
    }
}

void InitMemoryLookupTables1(uint8** LUT, uint8** LUT_2, MemoryState* gMemoryState) 
{
    Init_R_AND_W(LUT,(uint8*)gMemoryState->RDRAM,    MEMORY_START_RDRAM,      MEMORY_SIZE_RDRAM);
    //Init_R_AND_W(LUT,(uint8*)gMemoryState->ExRDRAM,  MEMORY_START_EXRDRAM,    MEMORY_SIZE_EXRDRAM);
//    Init_R_AND_W(LUT,(uint8*)gMemoryState->RDREG,    MEMORY_START_RDREG,      MEMORY_SIZE_RDREG);
    Init_R_AND_W(LUT,(uint8*)gMemoryState->ramRegs0, MEMORY_START_RAMREGS0,   MEMORY_SIZE_RAMREGS0);
    //Init_R_AND_W(LUT,(uint8*)gMemoryState->ramRegs4, MEMORY_START_RAMREGS4,   MEMORY_SIZE_RAMREGS4);
    Init_R_AND_W(LUT,(uint8*)gMemoryState->ramRegs8, MEMORY_START_RAMREGS8,   MEMORY_SIZE_RAMREGS8);
    Init_R_AND_W(LUT, (uint8*)gMemoryState->SP_MEM,   MEMORY_START_SPMEM,      MEMORY_SIZE_SPMEM);


    Init_R_AND_W(LUT_2, (uint8*)gMemoryState->SP_REG_1, MEMORY_START_SPREG_1,   MEMORY_SIZE_SPREG_1);
    Init_R_AND_W(LUT_2, (uint8*)gMemoryState->SP_REG_2, MEMORY_START_SPREG_2,   MEMORY_SIZE_SPREG_2);
    Init_R_AND_W(LUT_2, (uint8*)gMemoryState->DPC,      MEMORY_START_DPC,       MEMORY_SIZE_DPC);
    Init_R_AND_W(LUT_2, (uint8*)gMemoryState->DPS,      MEMORY_START_DPS,       MEMORY_SIZE_DPS);
    Init_R_AND_W(LUT_2, (uint8*)gMemoryState->MI,       MEMORY_START_MI,        MEMORY_SIZE_MI);
    Init_R_AND_W(LUT_2, (uint8*)gMemoryState->VI,       MEMORY_START_VI,        MEMORY_SIZE_VI);
    Init_R_AND_W(LUT_2, (uint8*)gMemoryState->AI,       MEMORY_START_AI,        MEMORY_SIZE_AI);
    Init_R_AND_W(LUT_2, (uint8*)gMemoryState->PI,       MEMORY_START_PI,        MEMORY_SIZE_PI);
    Init_R_AND_W(LUT_2, (uint8*)gMemoryState->RI,       MEMORY_START_RI,        MEMORY_SIZE_RI);
    Init_R_AND_W(LUT_2, (uint8*)gMemoryState->SI,       MEMORY_START_SI,        MEMORY_SIZE_SI);
    Init_R_AND_W(LUT_2, (uint8*)gMemoryState->C2A1,      MEMORY_START_C2A1,      MEMORY_SIZE_C2A1);
    Init_R_AND_W(LUT, (uint8*)gMemoryState->C1A1,      MEMORY_START_C1A1,      MEMORY_SIZE_C1A1);
    Init_R_AND_W(LUT_2, (uint8*)gMemoryState->C2A2,     MEMORY_START_C2A2,      MEMORY_SIZE_C2A2);

    Init_R_AND_W(LUT, (uint8*)gMemoryState->ROM_Image, MEMORY_START_ROM_IMAGE, gAllocationLength);
    Init_R_AND_W(LUT, (uint8*)gMemoryState->GIO_REG,   MEMORY_START_GIO,       MEMORY_SIZE_GIO_REG);
    Init_R_AND_W(LUT, (uint8*)gMemoryState->PIF,       MEMORY_START_PIF,       MEMORY_SIZE_PIF);
    Init_R_AND_W(LUT, (uint8*)gMemoryState->C1A3,      MEMORY_START_C1A3,      MEMORY_SIZE_C1A3);

#ifndef DYNAEXECUTIONMAPMEMORY
    DynInit_R_AND_W((uint8*)DynaRDRAM,    0x00000000,  rdram_sizes[1]-1);
    DynInit_R_AND_W((uint8*)DynaSP_REG,   0x04000000,  0x04080007);
    DynInit_R_AND_W((uint8*)DynaROM,      0x10000000,   0x1FFFFFFF);
#endif
}

void InitMemoryLookupTables()
{
    int i;

#ifdef ENABLE_OPCODE_DEBUGGER
    sDWord_ptr = (uint8**)&sDWord;
    sDWord2_ptr = (uint8**)&sDWord2;
#endif

    for( i=0; i<=0xFFFF; i++ )
    {
        sDWord[i] = gMemoryState.dummySegment;
        sDWord2[i] = gMemoryState.dummySegment;
        sDYN_PC_LOOKUP[i] = gMemoryState.dummySegment;
#ifdef ENABLE_OPCODE_DEBUGGER
        sDWORD_R__Debug[i] = gMemoryState_Interpreter_Compare.dummySegment;
        sDWORD_R_2__Debug[i] = gMemoryState_Interpreter_Compare.dummySegment;
#endif
    }

    InitMemoryLookupTables1(sDWord, sDWord2, &gMemoryState);

#ifdef ENABLE_OPCODE_DEBUGGER
    InitMemoryLookupTables1(sDWORD_R__Debug, sDWORD_R_2__Debug, &gMemoryState_Interpreter_Compare);
#ifdef DIRECT_TLB_LOOKUP
	TLB_sDWord_ptr = (uint8**)&TLB_sDWord;
#endif
#endif

#ifdef DIRECT_TLB_LOOKUP
	for( i=0; i< 0x100000; i++ )
	{
		TLB_sDWord[i] = sDWord[i>>4]+0x1000*(i&0xf);
//#ifdef ENABLE_OPCODE_DEBUGGER
//		TLB_sDWord__debug[i] = sDWORD_R_2__Debug[i>>4]+0x1000*(i&0xf);
//#endif
	}
#endif

}



//---------------------------------------------------------

uint8* Crapola;

//---------------------------------------------------------
void InitVirtualDynaMemory(void)
{
    //if (dyna_CodeTable != NULL) VirtualFree(dyna_CodeTable,       CODETABLE_SIZE,     MEM_DECOMMIT);
    //if (dyna_RecompCode != NULL) VirtualFree(dyna_RecompCode, RECOMPCODE_SIZE,    MEM_DECOMMIT);
    if (dyna_CodeTable != NULL) VirtualFree(dyna_CodeTable,     0,  MEM_RELEASE);
    if (dyna_RecompCode != NULL) VirtualFree(dyna_RecompCode,   0,  MEM_RELEASE);
	if (RDRAM_Copy != NULL ) VirtualFree(RDRAM_Copy, 0, MEM_RELEASE);

    dyna_CodeTable = (uint8*)VirtualAlloc(NULL, CODETABLE_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE); // 8mb ist die RDRam Size
    dyna_RecompCode = (uint8*)VirtualAlloc(NULL, RECOMPCODE_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	RDRAM_Copy = (uint8*)VirtualAlloc(NULL, 0x800000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    if (dyna_CodeTable == NULL) 
    {
        DisplayError("Cant alloc Mem for dyna_CodeTable");
        exit(1);
    }
    if (dyna_RecompCode == NULL) 
    {
        DisplayError("Cant alloc Mem for dyna_RecompCode");
        exit(1);
    }

    if( RDRAM_Copy == NULL )
    {
        DisplayError("Cant alloc RDRAM_Copy");
        exit(1);
    }
}

void FreeVirtualDynaMemory(void)
{
    //if( dyna_CodeTable != NULL ) VirtualFree(dyna_CodeTable, CODETABLE_SIZE, MEM_DECOMMIT);
    //if( dyna_RecompCode != NULL ) VirtualFree(dyna_RecompCode, RECOMPCODE_SIZE, MEM_DECOMMIT);
    if( dyna_CodeTable != NULL )    VirtualFree(dyna_CodeTable,     0, MEM_RELEASE);
    if( dyna_RecompCode != NULL )   VirtualFree(dyna_RecompCode,    0, MEM_RELEASE);
}


void InitVirtualMemory1(MemoryState* gMemoryState)
{
    gMemoryState->ramRegs0   = (uint32 *)VirtualAlloc(NULL,     MEMORY_SIZE_DUMMY,    MEM_RESERVE, PAGE_NOACCESS);
    gMemoryState->ramRegs0   = (uint32 *)VirtualAlloc((void*)(gMemoryState->ramRegs0),  MEMORY_SIZE_RAMREGS0, MEM_COMMIT, PAGE_READWRITE);

    gMemoryState->ramRegs4   = (uint32 *)VirtualAlloc((void*)(((uint8*)gMemoryState->ramRegs0)+0x4000),  MEMORY_SIZE_RAMREGS4, MEM_COMMIT, PAGE_READWRITE);

    gMemoryState->ramRegs8   = (uint32 *)VirtualAlloc(NULL,     MEMORY_SIZE_RAMREGS8,    MEM_RESERVE, PAGE_NOACCESS);    
    gMemoryState->ramRegs8   = (uint32 *)VirtualAlloc((void*)(gMemoryState->ramRegs8),  MEMORY_SIZE_RAMREGS8, MEM_COMMIT, PAGE_READWRITE);

    gMemoryState->SP_MEM     =  (uint32*)VirtualAlloc(NULL,          MEMORY_SIZE_DUMMY,    MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->SP_MEM     =  (uint32*)VirtualAlloc((void*)(gMemoryState->SP_MEM), MEMORY_SIZE_SPMEM,    MEM_COMMIT, PAGE_READWRITE);

    gMemoryState->SP_REG_1   =  (uint32*)VirtualAlloc(NULL,          MEMORY_SIZE_DUMMY,    MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->SP_REG_1   =  (uint32*)VirtualAlloc((void*)(gMemoryState->SP_REG_1), MEMORY_SIZE_SPREG_1,  MEM_COMMIT, PAGE_READWRITE);

    gMemoryState->SP_REG_2   = (uint32*)VirtualAlloc(NULL,   MEMORY_SIZE_DUMMY,    MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->SP_REG_2   = (uint32*)VirtualAlloc((void*)(gMemoryState->SP_REG_2),   MEMORY_SIZE_SPREG_2,  MEM_COMMIT, PAGE_READWRITE);

    gMemoryState->DPC        = (uint32*)VirtualAlloc(NULL,       MEMORY_SIZE_DUMMY,    MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->DPC        = (uint32*)VirtualAlloc((void*)(gMemoryState->DPC),       MEMORY_SIZE_DPC,      MEM_COMMIT, PAGE_READWRITE);

    gMemoryState->DPS        = (uint32*)VirtualAlloc(NULL,       MEMORY_SIZE_DUMMY,    MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->DPS        = (uint32*)VirtualAlloc((void*)(gMemoryState->DPS),       MEMORY_SIZE_DPS,      MEM_COMMIT, PAGE_READWRITE);

    gMemoryState->MI         = (uint32*)VirtualAlloc(NULL,        MEMORY_SIZE_DUMMY,    MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->MI         = (uint32*)VirtualAlloc((void*)(gMemoryState->MI),        MEMORY_SIZE_MI,       MEM_COMMIT, PAGE_READWRITE);

    gMemoryState->VI         = (uint32*)VirtualAlloc(NULL,        MEMORY_SIZE_DUMMY,    MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->VI         = (uint32*)VirtualAlloc((void*)(gMemoryState->VI),        MEMORY_SIZE_VI,       MEM_COMMIT,PAGE_READWRITE);

    gMemoryState->AI         = (uint32*)VirtualAlloc(NULL,        MEMORY_SIZE_DUMMY,    MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->AI         = (uint32*)VirtualAlloc((void*)(gMemoryState->AI),        MEMORY_SIZE_AI,       MEM_COMMIT,PAGE_READWRITE);

    gMemoryState->PI         = (uint32*)VirtualAlloc(NULL,        MEMORY_SIZE_DUMMY,    MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->PI         = (uint32*)VirtualAlloc((void*)(gMemoryState->PI),        MEMORY_SIZE_PI,       MEM_COMMIT,PAGE_READWRITE);

    gMemoryState->RI         = (uint32*)VirtualAlloc(NULL,        MEMORY_SIZE_DUMMY,    MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->RI         = (uint32*)VirtualAlloc((void*)(gMemoryState->RI),        MEMORY_SIZE_RI,       MEM_COMMIT,PAGE_READWRITE);

    gMemoryState->SI         = (uint32*)VirtualAlloc(NULL,        MEMORY_SIZE_DUMMY,    MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->SI         = (uint32*)VirtualAlloc((void*)(gMemoryState->SI),        MEMORY_SIZE_SI,       MEM_COMMIT,PAGE_READWRITE);

    gMemoryState->PIF        = (uint8*)VirtualAlloc(NULL,    MEMORY_SIZE_DUMMY,  MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->PIF        = (uint8 *)VirtualAlloc((void*)(gMemoryState->PIF),    MEMORY_SIZE_PIF,    MEM_COMMIT, PAGE_READWRITE);

    gMemoryState->RDRAM      = (uint8 *)VirtualAlloc(NULL,     MEMORY_SIZE_RDRAM+MEMORY_SIZE_EXRDRAM,    MEM_RESERVE, PAGE_NOACCESS);
    gMemoryState->RDRAM      = (uint8 *)VirtualAlloc((void*)(gMemoryState->RDRAM),     MEMORY_SIZE_RDRAM,    MEM_COMMIT, PAGE_READWRITE);

#ifdef ENABLE_OPCODE_DEBUGGER
    gMemoryState->C1A1       = (uint32*)VirtualAlloc(NULL,   MEMORY_SIZE_DUMMY,  MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->C1A1       = (uint32*)VirtualAlloc((void*)(gMemoryState->C1A1),   MEMORY_SIZE_C1A1,   MEM_COMMIT, PAGE_READWRITE);

	gMemoryState->C1A3       = (uint32*)VirtualAlloc(NULL,    MEMORY_SIZE_DUMMY,  MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->C1A3       = (uint32*)VirtualAlloc((void*)(gMemoryState->C1A3),   MEMORY_SIZE_C1A3,   MEM_COMMIT, PAGE_READWRITE);
    
    gMemoryState->C2A1       = (uint32*)VirtualAlloc(NULL,   MEMORY_SIZE_DUMMY,  MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->C2A1       = (uint32*)VirtualAlloc((void*)(gMemoryState->C2A1),   MEMORY_SIZE_C2A1,   MEM_COMMIT, PAGE_READWRITE);
#else
	gMemoryState->C1A1       =	(uint32*)SRam;
	gMemoryState->C1A3		 =	(uint32*)SRam;
	gMemoryState->C2A1		 =	(uint32*)SRam;
#endif

    gMemoryState->C2A2       = (uint32*)VirtualAlloc(NULL,      MEMORY_SIZE_C2A2,    MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->C2A2       = (uint32*)VirtualAlloc((void*)(gMemoryState->C2A2),      MEMORY_SIZE_C2A2,     MEM_COMMIT,PAGE_READWRITE);
	//gMemoryState->C2A2		 =  (uint32*)FlashRAM;	//well, can not do this

    gMemoryState->GIO_REG    = (uint32*)VirtualAlloc(NULL,    MEMORY_SIZE_DUMMY,  MEM_RESERVE,PAGE_NOACCESS);    
    gMemoryState->GIO_REG    = (uint32*)VirtualAlloc((void*)(gMemoryState->GIO_REG),    MEMORY_SIZE_GIO_REG,MEM_COMMIT, PAGE_READWRITE);

    gMemoryState->dummySegment = (uint8*)VirtualAlloc(NULL,    MEMORY_SIZE_DUMMY,  MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->dummySegment = (uint8 *)VirtualAlloc((void*)(gMemoryState->dummySegment),  MEMORY_SIZE_DUMMY,  MEM_COMMIT,PAGE_NOACCESS);
}

extern unsigned _int32 sync_valloc, sync_valloc2;
void InitVirtualMemory(void)
{
    InitVirtualMemory1(&gMemoryState);

#ifdef ENABLE_OPCODE_DEBUGGER
    InitVirtualMemory1(&gMemoryState_Interpreter_Compare);
#endif

    InitVirtualDynaMemory();
}


void FreeVirtualMemory1(MemoryState* gMemoryState)
{
    //rewrite me!!! free virtual memory.

    VirtualFree(gMemoryState->RDRAM,    MEMORY_START_RDRAM,     MEMORY_SIZE_RDRAM);
    VirtualFree(gMemoryState->ExRDRAM,  MEMORY_START_EXRDRAM,   MEMORY_SIZE_EXRDRAM);
//    VirtualFree(gMemoryState->RDREG,    MEMORY_START_RDREG,     MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->ramRegs0, MEMORY_START_RAMREGS0,  MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->ramRegs4, MEMORY_START_RAMREGS4,  MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->ramRegs8, MEMORY_START_RAMREGS8,  MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->SP_MEM,   MEMORY_START_SPMEM,     MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->SP_REG_1, MEMORY_START_SPREG_1,   MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->SP_REG_2, MEMORY_START_SPREG_2,   MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->DPC,      MEMORY_START_DPC,       MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->DPS,      MEMORY_START_DPS,       MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->MI,       MEMORY_START_MI,        MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->VI,       MEMORY_START_VI,        MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->AI,       MEMORY_START_AI,        MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->PI,       MEMORY_START_PI,        MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->RI,       MEMORY_START_RI,        MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->SI,       MEMORY_START_SI,        MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->C2A1,     MEMORY_START_C2A1,      MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->C1A1,     MEMORY_START_C1A1,      MEMORY_SIZE_DUMMY);
    VirtualFree(gMemoryState->C2A2,     MEMORY_START_C2A2,      MEMORY_SIZE_C2A2);
    VirtualFree(gMemoryState->ROM_Image,MEMORY_START_ROM_IMAGE, gAllocationLength);
    VirtualFree(gMemoryState->GIO_REG,  MEMORY_START_GIO,       MEMORY_SIZE_GIO_REG);
    VirtualFree(gMemoryState->PIF,      MEMORY_START_PIF,       MEMORY_SIZE_PIF);
    VirtualFree(gMemoryState->C1A3,     MEMORY_START_C1A3,      MEMORY_SIZE_C1A3);

    gMemoryState->RDRAM        = NULL;
    gMemoryState->ExRDRAM      = NULL;
    gMemoryState->C2A1         = NULL;
    gMemoryState->C1A1         = NULL;
    gMemoryState->C2A2         = NULL;
    gMemoryState->GIO_REG      = NULL;
    gMemoryState->C1A3         = NULL;
    gMemoryState->PIF          = NULL;
    gMemoryState->dummySegment = NULL;
//    gMemoryState->RDREG        = NULL;

    gMemoryState->ramRegs0 = NULL;
    gMemoryState->ramRegs4 = NULL;
    gMemoryState->ramRegs8 = NULL;
    gMemoryState->SP_MEM   = NULL;
    gMemoryState->SP_REG_1 = NULL;
    gMemoryState->SP_REG_2 = NULL;
    gMemoryState->DPC      = NULL;
    gMemoryState->DPS      = NULL;
    gMemoryState->MI       = NULL;
    gMemoryState->VI       = NULL;
    gMemoryState->AI       = NULL;
    gMemoryState->PI       = NULL;
    gMemoryState->RI       = NULL;
    gMemoryState->SI       = NULL;
}

void FreeVirtualMemory()
{
    FreeVirtualMemory1(&gMemoryState);

#ifdef ENABLE_OPCODE_DEBUGGER
    FreeVirtualMemory1(&gMemoryState_Interpreter_Compare);
#endif

    FreeVirtualDynaMemory();
}

void InitVirtualRomMemory1(MemoryState* gMemoryState, uint32 memsize) 
{
    FreeVirtualRomMemory();

    memsize = ((memsize + 0x1fffff)/0x200000)*0x200000;//bring it up to a even 2MB.

	gMemoryState->ROM_Image = (uint8*)VirtualAlloc(NULL, memsize,    MEM_RESERVE,PAGE_NOACCESS);
    gMemoryState->ROM_Image= (uint8*)VirtualAlloc((void*)(gMemoryState->ROM_Image), memsize,    MEM_COMMIT,PAGE_READWRITE);
    TRACE1("Allocated memory for ROM image = %08X", (uint32)gMemoryState->ROM_Image);
    
}

void InitVirtualRomMemory(uint32 memsize) 
{
	ROM_End_Address = 0xA0000000+memsize-1;
    InitVirtualRomMemory1(&gMemoryState, memsize);

#ifdef ENABLE_OPCODE_DEBUGGER
	gMemoryState_Interpreter_Compare.ROM_Image = gMemoryState.ROM_Image;
    //InitVirtualRomMemory1(&gMemoryState_Debug_Copy, memsize);
#endif

#ifndef DYNAEXECUTIONMAPMEMORY
    DynaROM = (uint8*)VirtualAlloc(NULL, memsize, MEM_COMMIT,PAGE_READWRITE);
    if ( DynaROM == NULL ) 
    {
        DisplayError("Failed to allocate virtual memory for ROM Image");
        exit(1);
        return;
    }
    TRACE1("Allocated memory for ROM image mirror = %08X", (uint32)DynaROM);
#endif
}


void FreeVirtualRomMemory(void)
{
#ifdef DYNAEXECUTIONMAPMEMORY
    int i;

    for( i=0; i<0x20000; i++ )
    {
        if( dynarommap[i] != NULL )
        {
            //VirtualFree(dynarommap[i], 0x10000, MEM_DECOMMIT);
            VirtualFree(dynarommap[i], 0, MEM_RELEASE);
            dynarommap[i] = NULL;
        }
    }
#else
    if( DynaROM != NULL )
    {
        if( !(VirtualFree((void*)DynaROM, 0, MEM_RELEASE)) )
        {
            DisplayError("Failed to release virtual memory for DynaROM, error code is %ld", GetLastError());
        }
        else
        {
            DynaROM = NULL;
        }
    }
#endif

    if( gMemoryState.ROM_Image != NULL )
    {
        //if ( !(VirtualFree((void*)gMemoryState.ROM_Image,    64*1024*1024,MEM_DECOMMIT)) )
        if ( !(VirtualFree((void*)gMemoryState.ROM_Image,  0,MEM_RELEASE)) )
        {
            DisplayError("Failed to release virtual memory for ROM Image, error code is %ld", GetLastError());
        }
        else
        {
            gMemoryState.ROM_Image = NULL;
        }
    }

    for( i=0x1000; i<=0x1FFF; i++ )
    {
        sDWord[i+0x8000] = gMemoryState.dummySegment;
		sDWord[i+0xA000] = gMemoryState.dummySegment;
	}
}

#ifdef DYNAEXECUTIONMAPMEMORY
BOOL UnmappedMemoryExceptionHelper(uint32 addr)
{
    if( NOT_IN_KO_K1_SEG(addr) )
        addr = TranslateITLBAddress(addr);

    addr &= 0x1FFFFFFF;

    //if( addr >= 0x10000000 && addr-0x10000000 <= gAllocationLength )  // In ROM range
    {
        int index;

        // Allocate the memory
        addr &= 0x1FFF0000;
        //index = (addr-0x10000000) / 0x10000;
        index = addr / 0x10000;

        dynarommap[index] = VirtualAlloc(NULL, 0x10000, MEM_COMMIT,PAGE_READWRITE);
        if( dynarommap[index] == NULL )
        {
            DisplayError("Unable to allocate memory to support dyna rom mapping");
            return FALSE;
        }
        else
        {
            memset(dynarommap[index], 0, 0x10000);
            // Mapped the memory

            DynInit_R_AND_W(dynarommap[index],addr,addr+0x0000FFFF);
            //TRACE3("Dyna memory mapped %08X, PC=%08X, Address=%08X", addr, gHWS_pc, (uint32)dynarommap[index]);
            return TRUE;
        }
    }
    //else
        //return FALSE;
}
#endif

// Set RDRAM size
void ResetRdramSize(int setsize)
{
	int retval;

    if( rdram_sizes[setsize] != current_rdram_size )
    {
        if( setsize == RDRAMSIZE_4MB )  // Need to turn off the expansion pack
        {
			int i;
			for( i=0x40; i< 0x80; i++ )
			{
		        sDWord[i|0x8000] = gMemoryState.dummySegment;
		        sDWord[i|0xA000] = gMemoryState.dummySegment;
#ifdef ENABLE_OPCODE_DEBUGGER
		        sDWORD_R__Debug[i|0x8000] = gMemoryState_Interpreter_Compare.dummySegment;
		        sDWORD_R__Debug[i|0xA000] = gMemoryState_Interpreter_Compare.dummySegment;
#endif
			}
			retval = VirtualFree(gMemoryState.ExRDRAM, MEMORY_SIZE_EXRDRAM, MEM_DECOMMIT);
			if( retval == 0 ) DisplayError("Error to release the ExRDRAM");

        }
        else    // Need to turn on the expansion pack
        {
			gMemoryState.ExRDRAM    = (uint8 *)VirtualAlloc((((uint8*)gMemoryState.RDRAM)+0x400000),   MEMORY_SIZE_EXRDRAM,  MEM_COMMIT, PAGE_READWRITE);
			if( (uint32)gMemoryState.ExRDRAM != (uint32)gMemoryState.RDRAM + 0x400000 )
				DisplayError("Fix me in ResetRdramSize()!, RDRAM and ExRDRAM is not in continue memory address: RDRAM=%08X, ExRDRAM=%08X",
								(uint32)gMemoryState.RDRAM, (uint32)gMemoryState.ExRDRAM	);

		    Init_R_AND_W(sDWord,(uint8*)gMemoryState.ExRDRAM,  MEMORY_START_EXRDRAM,    MEMORY_SIZE_EXRDRAM);
#ifdef ENABLE_OPCODE_DEBUGGER
		    Init_R_AND_W(sDWORD_R__Debug,(uint8*)gMemoryState_Interpreter_Compare.ExRDRAM,  MEMORY_START_EXRDRAM,    MEMORY_SIZE_EXRDRAM);
#endif
        }
        current_rdram_size = rdram_sizes[setsize];
    }

	RDRAM_End_Address = 0x80000000 + current_rdram_size -1;

    SetStatusBarText(3, setsize==RDRAMSIZE_4MB?"4MB":"8MB");
}


// These two CheckSum Checking routine is borrowed from Deadalus
void ROM_CheckSumMario()
{
    DWORD * rom;
    DWORD addr1;
    DWORD a1;
    DWORD t7;
    DWORD v1 = 0;
    DWORD t0 = 0;
    DWORD v0 = 0xF8CA4DDC; //(MARIO_BOOT_CIC * 0x5d588b65) + 1;
    DWORD a3 = 0xF8CA4DDC;
    DWORD t2 = 0xF8CA4DDC;
    DWORD t3 = 0xF8CA4DDC;
    DWORD s0 = 0xF8CA4DDC;
    DWORD a2 = 0xF8CA4DDC;
    DWORD t4 = 0xF8CA4DDC;
    DWORD t8, t6, a0;


    rom = (DWORD *)gMemoryState.ROM_Image;

    TRACE0("");
    TRACE0("");
    TRACE0("");
    TRACE0("Checking CRC for this ROM");

    for (addr1 = 0; addr1 < 0x00100000; addr1+=4)
    {
        v0 = rom[(addr1 + 0x1000)>>2];
        v1 = a3 + v0;
        a1 = v1;
        if (v1 < a3) 
            t2++;
    
        v1 = v0 & 0x001f;
        t7 = 0x20 - v1;
        t8 = (v0 >> (t7&0x1f));
        t6 = (v0 << (v1&0x1f));
        a0 = t6 | t8;

        a3 = a1;
        t3 ^= v0;
        s0 += a0;
        if (a2 < v0)
            a2 ^= a3 ^ v0;
        else
            a2 ^= a0;

        t0 += 4;
        t7 = v0 ^ s0;
        t4 += t7;
    }
    TRACE0("Finish CRC Checking");


    a3 ^= t2 ^ t3;  // CRC1
    s0 ^= a2 ^ t4;  // CRC2

    if (a3 != rom[0x10>>2] || s0 != rom[0x14>>2])
    {
        //DisplayError("Warning, CRC values don't match, fixed");

        rom[0x10>>2] = a3;
        rom[0x14>>2] = s0;
    }
}

void ROM_CheckSumZelda()
{
    DWORD * rom;
    DWORD addr1;
    DWORD addr2;
    DWORD t5 = 0x00000020;
    DWORD a3 = 0xDF26F436;
    DWORD t2 = 0xDF26F436;
    DWORD t3 = 0xDF26F436;
    DWORD s0 = 0xDF26F436;
    DWORD a2 = 0xDF26F436;
    DWORD t4 = 0xDF26F436;
    DWORD v0, v1, a1, a0;


    rom = (DWORD *)gMemoryState.ROM_Image;

    addr2 = 0;

    TRACE0("");
    TRACE0("");
    TRACE0("");
    TRACE0("Checking CRC for this ROM");

    for (addr1 = 0; addr1 < 0x00100000; addr1 += 4)
    {
        v0 = rom[(addr1 + 0x1000)>>2];
        v1 = a3 + v0;
        a1 = v1;
        
        if (v1 < a3)
            t2++;

        v1 = v0 & 0x1f;
        a0 = (v0 >> (t5-v1)) | (v0 << v1);
        a3 = a1;
        t3 = t3 ^ v0;
        s0 += a0;
        if (a2 < v0)
            a2 ^= a3 ^ v0;
        else
            a2 ^= a0;

        t4 += rom[(addr2 + 0x750)>>2] ^ v0;
        addr2 = (addr2 + 4) & 0xFF;

    }
    TRACE0("Finish CRC Checking");


    a3 ^= t2 ^ t3;
    s0 ^= a2 ^ t4;

    if (a3 != rom[0x10>>2] || s0 != rom[0x14>>2])
    {
        //DisplayError("Warning, CRC values don't match, fixed");

        rom[0x10>>2] = a3;
        rom[0x14>>2] = s0;
    }
    TRACE2("Generating CRC [M%d / %d]", 0x00100000, 0x00100000);
}

#ifdef ENABLE_OPCODE_DEBUGGER
void Debugger_Copy_Memory(MemoryState *target, MemoryState *source)
{
	memcpy( target->RDRAM,		source->RDRAM,		current_rdram_size);

//	memcpy( target->RDREG,		source->RDREG,		MEMORY_SIZE_RDREG);
	memcpy( target->ramRegs0,	source->ramRegs0,	MEMORY_SIZE_RAMREGS0);
	memcpy( target->ramRegs4,	source->ramRegs4,	MEMORY_SIZE_RAMREGS4);
	memcpy( target->ramRegs8,	source->ramRegs8,	MEMORY_SIZE_RAMREGS8);
	memcpy( target->SP_MEM,		source->SP_MEM,		MEMORY_SIZE_SPMEM);
	memcpy( target->SP_REG_1,	source->SP_REG_1,	MEMORY_SIZE_SPREG_1);
	memcpy( target->SP_REG_2,	source->SP_REG_2,	MEMORY_SIZE_SPREG_2);
	memcpy( target->DPC,		source->DPC,		MEMORY_SIZE_DPC);
	memcpy( target->DPS,		source->DPS,		MEMORY_SIZE_DPS);
	memcpy( target->MI,			source->MI,			MEMORY_SIZE_MI);
	memcpy( target->VI,			source->VI,			MEMORY_SIZE_VI);
	memcpy( target->AI,			source->AI,			MEMORY_SIZE_AI);
	memcpy( target->PI,			source->PI,			MEMORY_SIZE_PI);
	memcpy( target->RI,			source->RI,			MEMORY_SIZE_RI);
	memcpy( target->SI,			source->SI,			MEMORY_SIZE_SI);
	memcpy( target->GIO_REG,	source->GIO_REG,	MEMORY_SIZE_GIO_REG);
	memcpy( target->PIF,		source->PIF,		MEMORY_SIZE_PIF);
	memcpy( target->TLB,		source->TLB,		sizeof(tlb_struct)*MAXTLB);
}
#endif

