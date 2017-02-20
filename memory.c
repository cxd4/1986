/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Emulator for Nintendo 64 console system                              |
 |  Copyright (C) 2001  Joel Middendorf  schibo@emuhq.com                       |
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
 |  email      : schibo@emuhq.com                                               |
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
#include "globals.h"
#include "options.h"
#include "r4300i.h"
#include "memory.h"
#include "n64rcp.h"
#include "interrupt.h"
#include "iPIF.h"


//---------------------------------------------------------

uint8* dummySegment = NULL; //handles crap pointers for now..band-aid'ish

void Init_R_AND_W(uint8* MemoryRange, uint32 startAddress, uint32 endAddress)
{
    uint8* pTmp = (uint8*)MemoryRange;
    
    uint32 curSegment = ((startAddress & 0x1FFFFFFF) >> 16);
    uint32 endSegment = ((endAddress   & 0x1FFFFFFF) >> 16);

    while (curSegment <= endSegment) {
        sDWORD_R[curSegment | 0x0000] = pTmp;
        sDWORD_R[curSegment | 0x8000] = pTmp;
        sDWORD_R[curSegment | 0xA000] = pTmp;
        pTmp += 0x10000;
        curSegment++;
    }

    endSegment++;

    //this is a hack..if the memory is unmapped, point the pointers to the dummy segment...
    while (endSegment <= 0x1FFF) {
        sDWORD_R[endSegment | 0x0000] = dummySegment;
        sDWORD_R[endSegment | 0x8000] = dummySegment;
        sDWORD_R[endSegment | 0xA000] = dummySegment;
        endSegment++;
    }
}

//---------------------------------------------------------

void DynInit_R_AND_W(uint8* MemoryRange, uint32 startAddress, uint32 endAddress)
{
    uint8* pTmp = (uint8*)MemoryRange;
    uint32 curSegment = ((startAddress & 0x1FFFFFFF) >> 16);
    uint32 endSegment = ((endAddress   & 0x1FFFFFFF) >> 16);

    while (curSegment <= endSegment) {
        sDYN_PC_LOOKUP[curSegment | 0x0000] = pTmp;
        sDYN_PC_LOOKUP[curSegment | 0x8000] = pTmp;
        sDYN_PC_LOOKUP[curSegment | 0xA000] = pTmp;
        pTmp += 0x10000;
        curSegment++;
    }
}

//---------------------------------------------------------
// Init Memory Lookup Table
// Function is called from fileio.c
// right after a new ROM image is loaded
//---------------------------------------------------------
void InitMemoryLookupTables() 
{
	int i;
	for( i=0; i<0xFFFF; i++ )
	{
		sDWORD_R[i] = 0;
	}
//                                    START                  END
    Init_R_AND_W((uint8*)RDRAM,     0x00000000,        0x007FFFFF);
//	Init_R_AND_W((uint8*)RDRAM,     0x00000000,        0x00FFFFFF);
    Init_R_AND_W((uint8*)RDREG,     0x03F00000,        0x03FFFFFF);
    Init_R_AND_W((uint8*)SP_REG,    0x04000000,        0x04080007);
    Init_R_AND_W((uint8*)DPC,       0x04100000,        0x0410001F);
    Init_R_AND_W((uint8*)DPS,       0x04200000,        0x0420000F);
    Init_R_AND_W((uint8*)MI,        0x04300000,        0x0430000F);
    Init_R_AND_W((uint8*)VI,        0x04400000,        0x04400037);
    Init_R_AND_W((uint8*)AI,        0x04500000,        0x04500017);
    Init_R_AND_W((uint8*)PI,        0x04600000,        0x04600033);
    Init_R_AND_W((uint8*)RI,        0x04700000,        0x0470001F);
    Init_R_AND_W((uint8*)SI,        0x04800000,        0x0480001B);
    Init_R_AND_W((uint8*)C2A1,      0x05000000,        0x05000000+2047);
    Init_R_AND_W((uint8*)C1A1,      0x06000000,        0x06000000+2047);
	Init_R_AND_W((uint8*)C2A2,      0x08000000,        0x08000000+0xFFFFF);
//	Init_R_AND_W((uint8*)C2A2,      0x08000000,        0x08000000+2047);
    Init_R_AND_W((uint8*)ROM_Image, 0x10000000,        0x10000000+gAllocationLength-1);
    Init_R_AND_W((uint8*)GIO_REG,   0x18000000,        0x18000803);
    Init_R_AND_W((uint8*)PIF,       0x1FC00000,        0x1FC007FF);
	Init_R_AND_W((uint8*)C1A3,		0x1FD00000,		   0x1FD00000+0xFFFF);

	for( i=0; i<0xFFFF; i++ )
	{
		sDYN_PC_LOOKUP[i] = 0;
	}

//    DynInit_R_AND_W((uint8*)DynaRDRAM,    0x00000000,  0x003FFFFF);
	DynInit_R_AND_W((uint8*)DynaRDRAM,    0x00000000,  0x007FFFFF);
    DynInit_R_AND_W((uint8*)DynaSP_REG,   0x04000000,  0x04080007);
	DynInit_R_AND_W((uint8*)DynaROM,0x10000000,0x1FFFFFFF);
}

//---------------------------------------------------------

uint32 valloc = 0;
uint8* Crapola;
// Special Memory regions that generate exception
uint32 valloc2=0;

//---------------------------------------------------------
void InitVirtualMemory2(void)
{
	if ((valloc2 = (uint32)VirtualAlloc(0,512*1024*1024,MEM_RESERVE,PAGE_READWRITE))) 
	{
#ifdef DEBUG_COMMON
//		DisplayError("Allocation virtual memory 512MB successfully");
#endif 
	} 
	else
	{
		DisplayError("Failed to allocate 512MB virtual memory");
		return;
	}

	RDREG	= (uint32*)VirtualAlloc((void*)(valloc2+0x03F00000),	0x100000,	MEM_COMMIT,PAGE_READWRITE);

	ramRegs0= (uint8*)VirtualAlloc((void*)(valloc2+0x03F00000),	0x30,		MEM_COMMIT,PAGE_READWRITE);
	ramRegs4= (uint8*)VirtualAlloc((void*)(valloc2+0x03F04000),	0x30,		MEM_COMMIT,PAGE_READWRITE);
	ramRegs8= (uint8*)VirtualAlloc((void*)(valloc2+0x03F80000),	0x30,		MEM_COMMIT,PAGE_READWRITE);

    SP_REG	= (uint32*)VirtualAlloc((void*)(valloc2+0x04000000),	0x80008,	MEM_COMMIT,PAGE_READWRITE);
	DPC		= (uint32*)VirtualAlloc((void*)(valloc2+0x04100000),	0x20,		MEM_COMMIT,PAGE_READWRITE);
	DPS		= (uint32*)VirtualAlloc((void*)(valloc2+0x04200000),	0x10,		MEM_COMMIT,PAGE_READWRITE);
	MI		= (uint32*)VirtualAlloc((void*)(valloc2+0x04300000),	0x10,		MEM_COMMIT,PAGE_READWRITE);
	VI		= (uint32*)VirtualAlloc((void*)(valloc2+0x04400000),	0x50,		MEM_COMMIT,PAGE_READWRITE);
	AI		= (uint32*)VirtualAlloc((void*)(valloc2+0x04500000),	0x18,		MEM_COMMIT,PAGE_READWRITE);
	PI		= (uint32*)VirtualAlloc((void*)(valloc2+0x04600000),	0x4C,		MEM_COMMIT,PAGE_READWRITE);
	RI		= (uint32*)VirtualAlloc((void*)(valloc2+0x04700000),	0x20,		MEM_COMMIT,PAGE_READWRITE);
	SI		= (uint32*)VirtualAlloc((void*)(valloc2+0x04800000),	0x1C,		MEM_COMMIT,PAGE_READWRITE);
}

void InitVirtualMemory(void)
{
	if ((valloc = (uint32)VirtualAlloc(0,512*1024*1024,MEM_RESERVE,PAGE_READWRITE))) 
	{
#ifdef DEBUG_COMMON
//		DisplayError("Allocation virtual memory 512MB successfully");
#endif 
	} 
	else
	{
		DisplayError("Failed to allocate 512MB virtual memory");
		return;
	}

	//RDRAM	= (uint8*)VirtualAlloc((void*)valloc,				16*1024*1024,MEM_COMMIT,PAGE_READWRITE);
	RDRAM	= (uint8*)VirtualAlloc((void*)valloc,				8*1024*1024,MEM_COMMIT,PAGE_READWRITE);
	C2A1= (uint32*)VirtualAlloc((void*)(valloc+0x05000000),	0x10000,	MEM_COMMIT,PAGE_READWRITE);
	C1A1= (uint32*)VirtualAlloc((void*)(valloc+0x06000000),	0x10000,	MEM_COMMIT,PAGE_READWRITE);
	C2A2= (uint32*)VirtualAlloc((void*)(valloc+0x08000000),	0x400000,	MEM_COMMIT,PAGE_READWRITE);
	GIO_REG	= (uint32*)VirtualAlloc((void*)(valloc+0x18000000),	0x804,	MEM_COMMIT,PAGE_READWRITE);
	C1A3= (uint32*)VirtualAlloc((void*)(valloc+0x1FD00000),	0x10000,	MEM_COMMIT,PAGE_READWRITE);
	PIF		= (uint8*)VirtualAlloc((void*)(valloc+0x1FC00000),	0x800,		MEM_COMMIT,PAGE_READWRITE);
    dummySegment = (uint8*)VirtualAlloc((void*)(valloc+0x1FFF0000),	0x10000,	MEM_COMMIT,PAGE_READWRITE);

    InitVirtualMemory2();
}



void FreeVirtualMemory(void)
{
	//VirtualFree((void*)valloc,				16*1024*1024, MEM_DECOMMIT);
	VirtualFree((void*)valloc,				8*1024*1024, MEM_DECOMMIT);
	VirtualFree((void*)(valloc2+0x04000000),	0x2000,		 MEM_DECOMMIT);
	VirtualFree((void*)(valloc2+0x04040000),	0x20,		 MEM_DECOMMIT);
	VirtualFree((void*)(valloc2+0x04080000),	0x08,		 MEM_DECOMMIT);
	VirtualFree((void*)(valloc2+0x04100000),	0x10,		 MEM_DECOMMIT);
	VirtualFree((void*)(valloc2+0x04200000),	0x10,		 MEM_DECOMMIT);
	VirtualFree((void*)(valloc2+0x04300000),	0x10,		 MEM_DECOMMIT);
	VirtualFree((void*)(valloc2+0x04400000),	0x38,		 MEM_DECOMMIT);
	VirtualFree((void*)(valloc2+0x04500000),	0x18,		 MEM_DECOMMIT);
	VirtualFree((void*)(valloc2+0x04600000),	0x34,		 MEM_DECOMMIT);
	VirtualFree((void*)(valloc2+0x04700000),	0x20,		 MEM_DECOMMIT);
	VirtualFree((void*)(valloc2+0x04800000),	0x1C,		 MEM_DECOMMIT);
	VirtualFree((void*)(valloc2+0x1FC00000),	0x800,		 MEM_DECOMMIT);
	VirtualFree((void*)(valloc2+0x03F00000),	0x30,		 MEM_DECOMMIT);
	VirtualFree((void*)(valloc2+0x03F04000),	0x30,		 MEM_DECOMMIT);
	VirtualFree((void*)(valloc2+0x03F80000),	0x30,		 MEM_DECOMMIT);
	VirtualFree((void*)(valloc+0x05000000),		1024*1024,	 MEM_DECOMMIT);
	VirtualFree((void*)(valloc+0x06000000),		1024*1024,	 MEM_DECOMMIT);
	VirtualFree((void*)(valloc+0x08000000),		1024*1024*4, MEM_DECOMMIT);
	//VirtualFree((void*)(valloc+0x10000000),		64*1024*1024,MEM_DECOMMIT);
	FreeVirtualRomMemory();
	VirtualFree((void*)(valloc+0x1FC00000),		0x800,		 MEM_DECOMMIT);
	VirtualFree((void*)(valloc+0x1FD00000),		1024*1024,	 MEM_DECOMMIT);
	VirtualFree((void*)(valloc+0x1FFF0000),		0x10000,	 MEM_DECOMMIT);
}

void InitVirtualRomMemory(uint32 memsize) {
	memsize = ((memsize + 0x1fffff)/0x200000)*0x200000;//bring it up to a even 2MB.
	VirtualFree((void*)(valloc+0x10000000),	64*1024*1024,MEM_DECOMMIT);
	ROM_Image= (uint8*)VirtualAlloc((void*)(valloc+0x10000000),	memsize,	MEM_COMMIT,PAGE_READWRITE);
	DynaROM = (uint8*)VirtualAlloc(NULL, memsize, MEM_COMMIT,PAGE_READWRITE);
}


void FreeVirtualRomMemory(void)
{
	VirtualFree((void*)(valloc+0x10000000),	64*1024*1024,MEM_DECOMMIT);
	VirtualFree((void*)DynaROM, 64*1024*1024,MEM_DECOMMIT);
}

void LockVirtualRomMemory()
{
	uint32 memsize;
	memsize = ((gAllocationLength + 0x1fffff)/0x200000)*0x200000;//bring it up to a even 2MB.

	VirtualProtect(ROM_Image, memsize, PAGE_READONLY, (unsigned long *)(&ROM_Image[40]));
	//VirtualProtect(C2A1, 0x10000, PAGE_READONLY, (unsigned long *)(&C2A1[40]));
	//VirtualProtect(C1A1, 0x10000, PAGE_READONLY, (unsigned long *)(&C1A1[40]));
	// Zelda will write to C2A2
	//VirtualProtect(C2A2, 0x400000, PAGE_READONLY, (unsigned long *)(&C2A2[40]));
	//VirtualProtect(C1A3, 0x10000, PAGE_READONLY, (unsigned long *)(&C1A3[40]));
}

void UnlockVirtualRomMemory()
{
	uint32 memsize;
	memsize = ((gAllocationLength + 0x1fffff)/0x200000)*0x200000;//bring it up to a even 2MB.

	VirtualProtect(ROM_Image, memsize, PAGE_READWRITE, (unsigned long *)(&ROM_Image[40]));
	//VirtualProtect(C2A1, 0x10000, PAGE_READWRITE, (unsigned long *)(&C2A1[40]));
	//VirtualProtect(C1A1, 0x10000, PAGE_READWRITE, (unsigned long *)(&C1A1[40]));
	//VirtualProtect(C2A2, 0x400000, PAGE_READWRITE, (unsigned long *)(&C2A2[40]));
	//VirtualProtect(C1A3, 0x10000, PAGE_READWRITE, (unsigned long *)(&C1A3[40]));
}

