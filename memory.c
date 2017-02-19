/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Emulator for Nintendo 64 console system                              |
 |  Copyright (C) 2000  Joel Middendorf  schibo@emuhq.com                       |
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


The project is a direct documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar :). Most source
code comments are taken directly from anarko's n64toolkit with consent and are 
the property of anarko.
*/

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <windows.h>
#include "globals.h"
#include "r4300i.h"
#include <memory.h>

#include <process.h>
#include <malloc.h>
#include "interrupt.h"

#define SP_START_ADDR	0x04000000
#define SP_END			0x04080007
#define SP_SIZE (SP_END - SP_START_ADDR +1)

//---------------------------------------------------------

uint8 dummySegment[0xFFFF]; //handles crap pointers for now..band-aid'ish
void Init_R_AND_W(uint8* MemoryRange, uint32 startAddress, uint32 endAddress);
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

void DynInit_R_AND_W(uint8* MemoryRange, uint32 startAddress, uint32 endAddress);
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

void InitMemoryLookupTables() 
{//                                    START                  END
    Init_R_AND_W((uint8*)RDRAM,     0x00000000,        0x003FFFFF);
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
    Init_R_AND_W((uint8*)C2A2,      0x08000000,        0x08000000+2047);
    Init_R_AND_W((uint8*)ROM_Image, 0x10000000,        0x10000000+gAllocationLength-1);
    Init_R_AND_W((uint8*)GIO_REG,   0x18000000,        0x18000803);
    Init_R_AND_W((uint8*)PIF,       0x1FC00000,        0x1FC007FF);

    DynInit_R_AND_W((uint8*)DynaRDRAM,    0x00000000,  0x003FFFFF);
    DynInit_R_AND_W((uint8*)DynaSP_REG,   0x04000000,  0x04080007);
}

//---------------------------------------------------------

void r4300i_Reset()
{
    memset(RDRAM,   0, sizeof(RDRAM)  );
    memset(RDREG,   0, sizeof(RDREG)  );
    memset(SP_REG,  0, sizeof(SP_REG) );
    memset(DPC,     0, sizeof(DPC)    );
    memset(DPS,     0, sizeof(DPS)    );
    memset(MI,      0, sizeof(MI)     );
    memset(VI,      0, sizeof(VI)     );
    memset(AI,      0, sizeof(AI)     );
    memset(PI,      0, sizeof(PI)     );
    memset(RI,      0, sizeof(RI)     );
    memset(SI,      0, sizeof(SI)     );
    memset(C2A1,    0, sizeof(C2A1)   );
    memset(C1A1,    0, sizeof(C1A1)   );
    memset(C2A2,    0, sizeof(C2A2)   );
    memset(GIO_REG, 0, sizeof(GIO_REG));
    memset(PIF,     0, sizeof(PIF)    );    
    r4300i_Init();    

    CPUdelayPC = 0;

    /* Copy boot code to SP_DMEM */
    memcpy(SP_DMEM, ROM_Image, 0x1000);
    pc = 0xA4000040;
}

//---------------------------------------------------------
	
uint32 TranslateTLBAddress(uint32 address)
{
	uint32		realAddress = 0x80000000;
	_int32		c;
	tlb_struct	*theTLB;

	uint32		EntryLo;

	// search the tlb entries
	for (c = 31; c >= 0; c--) {
		theTLB = &TLB[c];

		// skip unused entries
		if (theTLB->valid == 0)								continue;

		if ( ((theTLB->EntryLo0 | theTLB->EntryLo1)) == 0)	continue;

		// compare upper bits
		if ((address & theTLB->MyHiMask) == (theTLB->EntryHi & theTLB->MyHiMask)) {
			// check the global bit
			if ((0x01 & theTLB->EntryLo1 & theTLB->EntryLo0) == 1) {
				// select EntryLo depending on if we're in an even or odd page
				if (address & theTLB->LoCompare) 
					EntryLo = theTLB->EntryLo1;
				else
					EntryLo = theTLB->EntryLo0;

				if (EntryLo & 0x02) {
					// calculate the real address from EntryLo
					realAddress |= ((EntryLo << 6) & ((theTLB->MyHiMask) >> 1));
					realAddress |= (address & ((theTLB->PageMask | 0x00001FFF) >> 1));
					return realAddress;
				} else {
					// invalid tlb entry
					goto error;
				}
			} else {
				// check asid - not necessary (?)
				goto error;
			}
		}
	}

error:
//	DisplayError("TLB MISS. Address = %08X\n", address);
	COP0Reg[CAUSE] |= TLBL_Miss;
	return (0xFFFFFFFC);
}
