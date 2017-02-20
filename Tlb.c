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


This project is a documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar. Many source
code comments are taken from anarko's n64toolkit with consent and are 
the property of anarko.
*/

#include <windows.h>
#include <stdio.h>
#include "globals.h"
#include "options.h"
#include "r4300i.h"
#include "hardware.h"
#include "interrupt.h"
#include "Dbgprint.h"
#include "debug_option.h"
#include "timer.h"
#include "memory.h"

/*
  -----------------------------------------------------------------
  | TLBP      | Translation Lookaside Buffer Probe                |
  |-----------|---------------------------------------------------|
  |  010000   |CO|      0000 0000 0000 0000 000       | 001000 (8)|
  ------6------1-------------------19-----------------------6------
   Format:  TLBP
   Purpose: The Index register is loaded with the address of the TLB entry
   whose contents match the contents of the EntryHi register. If no
   TLB entry matches, the high-order bit of the Index register is set.
   The architecture does not specify the operation of memory references
   associated with the instruction immediately after a TLBP instruction,
   nor is the operation specified if more than one TLB entry matches.
*/

uint32 TLBMapErrorAddress(uint32 address);

void r4300i_COP0_tlbp(uint32 Instruction)
{
    uint32      index;
	//uint32      g;

    gHardwareState.COP0Reg[INDEX] = 0x80000000; // initially set high-order bit

    for (index = 0; index <= NTLBENTRIES; index++) {
        //g = (TLB[index].EntryLo0 & TLB[index].EntryLo1) & 0x01;
        /*if (((TLB[index].EntryHi & (~0x1FFF)) == (gHardwareState.COP0Reg[ENTRYHI] & (~0x1FFF))) &&
                (g || ((TLB[index].EntryHi & 0xFF) == (gHardwareState.COP0Reg[ENTRYHI] & 0xFF)))
				&& TLB[index].valid )*/
		if( ( (TLB[index].EntryHi & TLBHI_VPN2MASK) == (gHardwareState.COP0Reg[ENTRYHI] & TLBHI_VPN2MASK) ) &&
			( (TLB[index].EntryHi & TLBHI_PIDMASK)  == (gHardwareState.COP0Reg[ENTRYHI] & TLBHI_PIDMASK ) ) )
        {

    		gHardwareState.COP0Reg[INDEX] = index;
#ifdef DEBUG_TLB
			if( debug_tlb )
			{
				sprintf(generalmessage, "TLBP - Load INDEX register: %d", index);
				RefreshOpList(generalmessage);
#ifdef DEBUG_TLB_DETAIL
				sprintf(generalmessage, "VPN2 = 0x%08X [bit 31-13]", (uint32)gHardwareState.COP0Reg[ENTRYHI] & TLBHI_VPN2MASK);
				RefreshOpList(generalmessage);
#endif
			}
#endif
            return;
        }
    }
#ifdef DEBUG_TLB
	if( debug_tlb )
	{
		sprintf(generalmessage, "TLBP - no match");
		RefreshOpList(generalmessage);
	}
#endif

}


/*
  -----------------------------------------------------------------
  | TLBR      | Translation Lookaside Buffer Read                 |
  |-----------|---------------------------------------------------|
  |  010000   |CO|      0000 0000 0000 0000 000       | 000001 (1)|
  ------6------1-------------------19-----------------------6------
   Format:  TLBR
   Purpose: The G bit (which controls ASID matching) read from the TLB is
   written into both of the EntryLo0 and EntryLo1 registers.
   The EntryHi and EntryLo registers are loaded with the contents of
   the TLB entry pointed at by the contents of the TLB Index register.
   The operation is invalid (and the results are unspecified) if the
   contents of the TLB Index register are greater than the number of
   TLB entries in the processor.
*/
void r4300i_COP0_tlbr(uint32 Instruction)
{
    uint32      index = gHardwareState.COP0Reg[INDEX] /*& NTLBENTRIES*/;
    uint32      g = (TLB[index].EntryLo0 & TLB[index].EntryLo1 & 0x01);

    if (index > NTLBENTRIES) {
        DisplayError("ERROR: tlbr received an invalid index");
    }
    else
    {
#ifdef DEBUG_TLB
		if( debug_tlb )
		{
			sprintf(generalmessage, "TLBR - Read from TLB[%d]", index);
			RefreshOpList(generalmessage);
		}
#endif
	
		gHardwareState.COP0Reg[PAGEMASK] = (uint32)(TLB[index].PageMask);
        gHardwareState.COP0Reg[ENTRYHI] =  (uint32)(TLB[index].EntryHi & (~(uint32)TLB[index].PageMask));
        gHardwareState.COP0Reg[ENTRYLO1] = (TLB[index].EntryLo1 | g);
        gHardwareState.COP0Reg[ENTRYLO0] = (TLB[index].EntryLo0 | g);
        
        // ?? TLB[index].MyHiMask = ~(uint32)TLB[index].PageMask & 0xFFFFE000;
    }
}


/*
  -----------------------------------------------------------------
  | TLBWI     | Translation Lookaside Buffer Write Index          |
  |-----------|---------------------------------------------------|
  |  010000   |CO|      0000 0000 0000 0000 000       | 000010 (2)|
  ------6------1-------------------19-----------------------6------
   Format:  TLBWI
   Purpose: The G bit of the TLB is written with the logical AND of the G bits
   in the EntryLo0 and EntryLo1 registers. The TLB entry pointed at by
   the contents of the TLB Index register is loaded with the contents
   of the EntryHi and EntryLo registers. The operation is invalid (and
   the results are unspecified) if the contents of the TLB Index
   register are greater than the number of TLB entries in the processor.
*/
void r4300i_COP0_tlbwi(uint32 Instruction)
{
    uint32      index = gHardwareState.COP0Reg[INDEX] /*& 0x1F*/;
    tlb_struct  *theTLB = &TLB[index];
    int         g = (gHardwareState.COP0Reg[ENTRYLO0] & gHardwareState.COP0Reg[ENTRYLO1]) & 0x01;


    if (index > NTLBENTRIES) {
        DisplayError("ERROR: tlbwi called with invalid index");
    }
    else
    {
#ifdef DEBUG_TLB
		if( debug_tlb ) {
			sprintf(generalmessage, "TLBWI - Load TLB[%d]", index);
			RefreshOpList(generalmessage);
#ifdef DEBUG_TLB_DETAIL
			sprintf(generalmessage, "PAGEMASK = 0x%08X, ENTRYHI = 0x%08X", (uint32)gHardwareState.COP0Reg[PAGEMASK],(uint32)gHardwareState.COP0Reg[ENTRYHI]);
			RefreshOpList(generalmessage);
			sprintf(generalmessage, "ENTRYLO1 = 0x%08X, ENTRYLO0 = 0x%08X", (uint32)gHardwareState.COP0Reg[ENTRYLO1],(uint32)gHardwareState.COP0Reg[ENTRYLO0]);
			RefreshOpList(generalmessage);
#endif
		}
#endif
	    theTLB->valid = 0;
		if( (gHardwareState.COP0Reg[ENTRYLO1] & TLBLO_V ) || (gHardwareState.COP0Reg[ENTRYLO0] & TLBLO_V ) )
		theTLB->valid = 1;
        
		theTLB->PageMask = gHardwareState.COP0Reg[PAGEMASK];
        //theTLB->EntryHi = gHardwareState.COP0Reg[ENTRYHI] ;
		theTLB->EntryHi = gHardwareState.COP0Reg[ENTRYHI] & (~(uint32)gHardwareState.COP0Reg[PAGEMASK]);
        theTLB->EntryLo1 = (gHardwareState.COP0Reg[ENTRYLO1] | g);
        theTLB->EntryLo0 = (gHardwareState.COP0Reg[ENTRYLO0] | g);

        theTLB->MyHiMask = ~(uint32)theTLB->PageMask & TLBHI_VPN2MASK;

        switch (theTLB->PageMask) {
            case 0x00000000:    // 4k
                theTLB->LoCompare = 0x00001000;
                break;
            case 0x00006000:    // 16k
                theTLB->LoCompare = 0x00004000;
                break;
            case 0x0001E000:    // 64k
                theTLB->LoCompare = 0x00010000;
                break;
            case 0x0007E000:    // 256k
                theTLB->LoCompare = 0x00040000;
                break;
            case 0x001FE000:    // 1M
                theTLB->LoCompare = 0x00100000;
                break;
            case 0x007FE000:    // 4M
                theTLB->LoCompare = 0x00400000;
                break;
            case 0x01FFE000:    // 16M
                theTLB->LoCompare = 0x01000000;
                break;
            default:
                DisplayError("ERROR: tlbwi - invalid page size");
                break;
        }
    }
}


/*
  -----------------------------------------------------------------
  | TLBWR     | Translation Lookaside Buffer Write Random         |
  |-----------|---------------------------------------------------|
  |  010000   |CO|      0000 0000 0000 0000 000       | 000110 (6)|
  ------6------1-------------------19-----------------------6------
   Format:  TLBWR
   Purpose: The G bit of the TLB is written with the logical AND of the G bits
   in the EntryLo0 and EntryLo1 registers. The TLB entry pointed at by
   the contents of the TLB Random register is loaded with the contents
   of the EntryHi and EntryLo registers
*/
void r4300i_COP0_tlbwr(uint32 Instruction)
{
    int         index;
    tlb_struct  *theTLB;

    int         g = (gHardwareState.COP0Reg[ENTRYLO0] & gHardwareState.COP0Reg[ENTRYLO1]) & 0x01;

	gHardwareState.COP0Reg[RANDOM] = gHardwareState.COP0Reg[COUNT] % (0x40-(gHardwareState.COP0Reg[WIRED]&0x3f))+gHardwareState.COP0Reg[WIRED];
	index = gHardwareState.COP0Reg[RANDOM] & NTLBENTRIES;
	theTLB = &TLB[index];
	/*
    if (index > MAXTLB) {
        DisplayError("ERROR: tlbwr called with invalid index");
		return;
    }
	*/

#ifdef DEBUG_TLB
	if( debug_tlb ) {
		sprintf(generalmessage, "TLBWR - Load TLB[%d]", index);
		RefreshOpList(generalmessage);
#ifdef DEBUG_TLB_DETAIL
		sprintf(generalmessage, "PAGEMASK = 0x%08X, ENTRYHI = 0x%08X", (uint32)gHardwareState.COP0Reg[PAGEMASK],(uint32)gHardwareState.COP0Reg[ENTRYHI]);
		RefreshOpList(generalmessage);
		sprintf(generalmessage, "ENTRYLO1 = 0x%08X, ENTRYLO0 = 0x%08X", (uint32)gHardwareState.COP0Reg[ENTRYLO1],(uint32)gHardwareState.COP0Reg[ENTRYLO0]);
		RefreshOpList(generalmessage);
#endif
	}
#endif

    // gerrit - uhh.. do we deal with the random reg at all?
    // DisplayError("ERROR: opcode tlbwr is not working");

    theTLB->valid = 0;
	if( (gHardwareState.COP0Reg[ENTRYLO1] & TLBLO_V ) || (gHardwareState.COP0Reg[ENTRYLO0] & TLBLO_V ) )
		theTLB->valid = 1;

    theTLB->PageMask = gHardwareState.COP0Reg[PAGEMASK];
    theTLB->EntryHi = gHardwareState.COP0Reg[ENTRYHI] & (~(uint32)gHardwareState.COP0Reg[PAGEMASK]);
    theTLB->EntryLo1 = (gHardwareState.COP0Reg[ENTRYLO1] | g);
    theTLB->EntryLo0 = (gHardwareState.COP0Reg[ENTRYLO0] | g);

    theTLB->MyHiMask = ~(uint32)theTLB->PageMask & 0xFFFFE000;
    
    switch (theTLB->PageMask) {
        case 0x00000000:    // 4k
            theTLB->LoCompare = 0x00001000;
            break;
        case 0x00006000:    // 16k
            theTLB->LoCompare = 0x00004000;
            break;
        case 0x0001E000:    // 64k
            theTLB->LoCompare = 0x00010000;
            break;
        case 0x0007E000:    // 256k
            theTLB->LoCompare = 0x00040000;
            break;
        case 0x001FE000:    // 1M
            theTLB->LoCompare = 0x00100000;
            break;
        case 0x007FE000:    // 4M
            theTLB->LoCompare = 0x00400000;
            break;
        case 0x01FFE000:    // 16M
            theTLB->LoCompare = 0x01000000;
            break;
        default:
            DisplayError("ERROR: tlbwr - invalid page size");
            break;
    }
}

//extern uint32* InstructionPointer;
extern uint32* pcptr;
/*
-----------------------------------------------------------------
| ERET      | Return from Exception                             |
|-----------|---------------------------------------------------|
|  010000   |CO|      0000 0000 0000 0000 000       |011000 (24)|
------6------1-------------------19-----------------------6------
 Format:  ERET
 Purpose: ERET is the R4300 instruction for returning from an interrupt,
          exception, or error trap. Unlike a branch or jump instruction,
          ERET does not execute the next instruction.
*/
void r4300i_COP0_eret(uint32 Instruction) {

    if ((gHardwareState.COP0Reg[STATUS] & 0x00000004)) 
    {
        CPUdelayPC = gHardwareState.COP0Reg[ERROREPC];  //dynarec use: pc = COP0Reg[ERROREPC];
        /* clear the ERL bit to zero */
        gHardwareState.COP0Reg[STATUS] &= 0xFFFFFFFB; // 0xFFFFFFFB same as ~0x00000004;
    }
    else
    {
        CPUdelayPC = gHardwareState.COP0Reg[EPC]; //dynarec use: pc = COP0Reg[EPC];
        /* clear the EXL bit of the status register to zero */
        gHardwareState.COP0Reg[STATUS] &= 0xFFFFFFFD; //0xFFFFFFFD same as ~0x00000002
    }
    CPUdelay = 2;//not used in dynarec
    gHardwareState.LLbit    =  0;
}

uint32 TranslateITLBAddress(uint32 address)
{
	static int	lastusedITLB;
    uint32      realAddress = 0x80000000;
    _int32      c;
    tlb_struct  *theTLB;
    uint32      EntryLo;

	for( c=0; c<MAXITLB; c++)
	{
        theTLB = &ITLB[c];

        // skip unused entries
        if (theTLB->valid == 0)                             continue;
        if ( ((theTLB->EntryLo0 | theTLB->EntryLo1)) == 0)  continue;

        // compare upper bits
        if ((address & theTLB->MyHiMask) == (theTLB->EntryHi & theTLB->MyHiMask)) 
		{
            // check the global bit
            if ((0x01 & theTLB->EntryLo1 & theTLB->EntryLo0) == 1) 
			{
                // select EntryLo depending on if we're in an even or odd page
                if (address & theTLB->LoCompare) 
                    EntryLo = theTLB->EntryLo1;
                else
                    EntryLo = theTLB->EntryLo0;

                if (EntryLo & 0x02) 
				{
                    // calculate the real address from EntryLo
                    realAddress |= ((EntryLo << 6) & ((theTLB->MyHiMask) >> 1));
                    realAddress |= (address & ((theTLB->PageMask | 0x00001FFF) >> 1));
					lastusedITLB = c;
                    return realAddress;
                } 
            } 
        }
	}

	// Here the TLB address can not be translated in ITLB table, look it up in TLB table

    // search the tlb entries
    for (c = 0; c < MAXTLB; c++) {
        theTLB = &TLB[c];

        // skip unused entries
        if (theTLB->valid == 0)                             continue;
        if ( ((theTLB->EntryLo0 | theTLB->EntryLo1)) == 0)  continue;

        // compare upper bits
        if ((address & theTLB->MyHiMask) == (theTLB->EntryHi & theTLB->MyHiMask)) 
		{
            // check the global bit
            if ((0x01 & theTLB->EntryLo1 & theTLB->EntryLo0) == 1) 
			{
                // select EntryLo depending on if we're in an even or odd page
                if (address & theTLB->LoCompare) 
                    EntryLo = theTLB->EntryLo1;
                else
                    EntryLo = theTLB->EntryLo0;

                if (EntryLo & 0x02) 
				{
                    // calculate the real address from EntryLo
                    realAddress |= ((EntryLo << 6) & ((theTLB->MyHiMask) >> 1));
                    realAddress |= (address & ((theTLB->PageMask | 0x00001FFF) >> 1));

					// Copy the TLB to microTLB
					lastusedITLB = 1 - lastusedITLB;

					ITLB[lastusedITLB].valid=theTLB->valid;
					ITLB[lastusedITLB].PageMask=theTLB->PageMask;
					ITLB[lastusedITLB].EntryHi=theTLB->EntryHi;
					ITLB[lastusedITLB].EntryLo1=theTLB->EntryLo1;
					ITLB[lastusedITLB].EntryLo0=theTLB->EntryLo0;
			        ITLB[lastusedITLB].MyHiMask=theTLB->MyHiMask;
	                ITLB[lastusedITLB].LoCompare=theTLB->LoCompare;

                    return realAddress;
                } 
				else 
				{
                    // invalid tlb entry
                    goto error;
                }
            } 
			else 
			{
                // check asid - not necessary (?)
                goto error;
            }
        }
    }
	
	// TEMP - HACK (!) + VERY STUPID for golden eye
	//if ((address & 0xFF000000) == 0x7F000000)
	//{
	//	return (0x10034B30 + (address - 0x7F000000));
	//}

error:
	gHardwareState.COP0Reg[BADVADDR] = address;

	gHardwareState.COP0Reg[CONTEXT] &= 0xFF800000;	// Mask off bottom 23 bits
	gHardwareState.COP0Reg[CONTEXT] |= ((address>>13)<<4);

	gHardwareState.COP0Reg[ENTRYHI] &= 0x00001FFF;	// Mask off the top bit 13-31
	gHardwareState.COP0Reg[ENTRYHI] |= (address&0xFFFFE000);

    gHardwareState.COP0Reg[CAUSE] |= TLBL_Miss;
#ifndef CPUCHECKINTR
	CPUNeedToDoOtherTask = TRUE;
	CPUNeedToCheckException = TRUE;
#endif

#ifdef DEBUG_TLB
	if( debug_tlb ) {
#ifdef DEBUG_TLB_DETAIL
		sprintf(generalmessage,"TLBL Missing Exception for Instruction Access");
		RefreshOpList(generalmessage);
		sprintf(generalmessage,"Bad VPN2 = 0x%8X", address>>13);
		RefreshOpList(generalmessage);
		sprintf(generalmessage,"Bad Virtual Address = 0x%08X", address);
		RefreshOpList(generalmessage);
#endif
		//DisplayError("TLBL Missing Exception Fired in ITLB, PC=%08X",gHardwareState.pc);
	}
#endif
    //return (0xFFFFFFFC);
	//return (0x80000000);
	return TLBMapErrorAddress(address);;
	//return (address&0x0000FFFF+0x1FFF0000);	// mapped to dummy segment
}

uint32 TranslateTLBAddressForLoad(uint32 address)
{
	return TranslateTLBAddress(address,0);
}

uint32 TranslateTLBAddressForStore(uint32 address)
{
	return TranslateTLBAddress(address,1);
}

uint32 TranslateTLBAddress(uint32 address, int operation)
{
	static int	lastusedDTLB=1;
    uint32      realAddress = 0x80000000;
    _int32      c;
    tlb_struct  *theTLB;
    uint32      EntryLo;

	for( c=0; c<MAXDTLB; c++)
	{
        theTLB = &DTLB[c];

        // skip unused entries
        if (theTLB->valid == 0)                             continue;
        if ( ((theTLB->EntryLo0 | theTLB->EntryLo1)) == 0)  continue;

        // compare upper bits
        //if ((address & theTLB->MyHiMask) == (theTLB->EntryHi & theTLB->MyHiMask)) 
		if ((address & theTLB->MyHiMask & 0x1FFFFFFF) == (theTLB->EntryHi & theTLB->MyHiMask & 0x1FFFFFFF)) 
		{
            // check the global bit
            if ((0x01 & theTLB->EntryLo1 & theTLB->EntryLo0) == 1) 
			{
                // select EntryLo depending on if we're in an even or odd page
                if (address & theTLB->LoCompare) 
                    EntryLo = theTLB->EntryLo1;
                else
                    EntryLo = theTLB->EntryLo0;

                if (EntryLo & 0x02) 
				{
                    // calculate the real address from EntryLo
                    realAddress |= ((EntryLo << 6) & ((theTLB->MyHiMask) >> 1));
                    realAddress |= (address & ((theTLB->PageMask | 0x00001FFF) >> 1));
					lastusedDTLB = c;

//#ifdef DEBUG_COMMON 
//    sprintf(generalmessage, "DTLB matched at %d",c);
//    RefreshOpList(generalmessage);
//#endif
                    return realAddress;
                } 
            }
			else
			{
#ifdef DEBUG_TLB 
				if( debug_tlb ) {
					sprintf(generalmessage, "Should check TLB's ASID field");
					RefreshOpList(generalmessage);
				}
#endif
				
			}
        }
	}

	// Here the TLB address can not be translated in DTLB table, look it up in TLB table

    // search the tlb entries
    for (c = 0; c < MAXTLB; c++) {
        theTLB = &TLB[c];

        // skip unused entries
        if (theTLB->valid == 0)                             continue;
        if ( ((theTLB->EntryLo0 | theTLB->EntryLo1)) == 0)  continue;

        // compare upper bits
        //if ((address & theTLB->MyHiMask) == (theTLB->EntryHi & theTLB->MyHiMask)) 
		if ((address & theTLB->MyHiMask & 0x1FFFFFFF) == (theTLB->EntryHi & theTLB->MyHiMask & 0x1FFFFFFF)) 
		{
            // check the global bit
            if ((0x01 & theTLB->EntryLo1 & theTLB->EntryLo0) == 1) 
			{
                // select EntryLo depending on if we're in an even or odd page
                if (address & theTLB->LoCompare) 
                    EntryLo = theTLB->EntryLo1;
                else
                    EntryLo = theTLB->EntryLo0;

                if (EntryLo & 0x02) 
				{
                    // calculate the real address from EntryLo
                    realAddress |= ((EntryLo << 6) & ((theTLB->MyHiMask) >> 1));
                    realAddress |= (address & ((theTLB->PageMask | 0x00001FFF) >> 1));

					// Copy the TLB to microTLB
					lastusedDTLB = 1 - lastusedDTLB;

					DTLB[lastusedDTLB].valid=theTLB->valid;
					DTLB[lastusedDTLB].PageMask=theTLB->PageMask;
					DTLB[lastusedDTLB].EntryHi=theTLB->EntryHi;
					DTLB[lastusedDTLB].EntryLo1=theTLB->EntryLo1;
					DTLB[lastusedDTLB].EntryLo0=theTLB->EntryLo0;
			        DTLB[lastusedDTLB].MyHiMask=theTLB->MyHiMask;
	                DTLB[lastusedDTLB].LoCompare=theTLB->LoCompare;

#ifdef DEBUG_TLB 
					if( debug_tlb ) {
					sprintf(generalmessage, "DTLB remapped to TLB at %d",c);
					RefreshOpList(generalmessage);
					}
#endif

                    return realAddress;
                } 
				else 
				{
                    // invalid tlb entry
                    goto error;
                }
            } 
			else 
			{
#ifdef DEBUG_TLB
				if( debug_tlb ) {
					sprintf(generalmessage, "Should check TLB's ASID field");
					RefreshOpList(generalmessage);
				}
#endif
                // check asid - not necessary (?)
                goto error;
            }
        }
    }

error:
	gHardwareState.COP0Reg[BADVADDR] = address;

	gHardwareState.COP0Reg[CONTEXT] &= 0xFF800000;	// Mask off bottom 23 bits
	gHardwareState.COP0Reg[CONTEXT] |= ((address>>13)<<4);

	gHardwareState.COP0Reg[ENTRYHI] &= 0x00001FFF;	// Mask off the top bit 13-31
	gHardwareState.COP0Reg[ENTRYHI] |= (address&0xFFFFE000);

	if( operation == 0 )
		gHardwareState.COP0Reg[CAUSE] |= TLBL_Miss;
	else
		gHardwareState.COP0Reg[CAUSE] |= TLBS_Miss;
#ifndef CPUCHECKINTR
		CPUNeedToDoOtherTask = TRUE;
		CPUNeedToCheckException = TRUE;
#endif

#ifdef DEBUG_TLB
	if( debug_tlb ) {
		if( operation == 0 )
			sprintf(generalmessage,"0x%08X: TLBL Missing Exception", gHardwareState.pc);
		else
			sprintf(generalmessage,"0x%08X: TLBS Missing Exception", gHardwareState.pc);

		RefreshOpList(generalmessage);
		
		sprintf(generalmessage,"Bad Virtual Address = 0x%08X", address);
		RefreshOpList(generalmessage);
	}
#endif
	return TLBMapErrorAddress(address);	
	//return (address&0x0000FFFF+0x1FFF0000);	// mapped to dummy segment
}


uint32 GetMemoryOffsetPointer(uint32 param)
{
         return((uint32)((_int8*)sDWORD_R[((uint16)((param) >> 16))] + ((uint16)param ^ 3)));
}


void InitTLB(void)
{
	int i;
	for( i=0; i<MAXTLB; i++ )
	{
	    TLB[i].EntryHi=0;
		TLB[i].EntryLo0=0;
		TLB[i].EntryLo1=0;
		TLB[i].LoCompare=0;
		TLB[i].MyHiMask=0;
		TLB[i].PageMask=0;
		TLB[i].valid=0;
	}

	for( i=0; i<MAXITLB; i++ )
	{
	    ITLB[i].EntryHi=0;
		ITLB[i].EntryLo0=0;
		ITLB[i].EntryLo1=0;
		ITLB[i].LoCompare=0;
		ITLB[i].MyHiMask=0;
		ITLB[i].PageMask=0;
		ITLB[i].valid=0;
	}

	for( i=0; i<MAXDTLB; i++ )
	{
	    DTLB[i].EntryHi=0;
		DTLB[i].EntryLo0=0;
		DTLB[i].EntryLo1=0;
		DTLB[i].LoCompare=0;
		DTLB[i].MyHiMask=0;
		DTLB[i].PageMask=0;
		DTLB[i].valid=0;
	}

}

// This function will try best to map an TLB error virtual address to its real address
uint32 TLBMapErrorAddress( uint32 address )
{
	// Check if the address is in any readable memory region
	uint32 dummyword;

	__try
	{
		dummyword = *((uint32*)(valloc + (address&0x1fffffff) ));
		return (address&0x1fffffff);
	}
	__except(NULL,EXCEPTION_EXECUTE_HANDLER)
	{
        __try
		{
            dummyword = *((uint32*)(valloc2 + (address&0x1fffffff) ));
			return (address&0x1fffffff);
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
			return (address&0x0000FFFF+0x1FFF0000);	// mapped to dummy segment
        }
	}
}


//TRAPS
void r4300i_teqi(uint32 Instruction){DisplayError("In Trap TEQI");}
void r4300i_tgei(uint32 Instruction){DisplayError("In Trap r4300i_tgei");}
void r4300i_tgeiu(uint32 Instruction){DisplayError("In Trap r4300i_tgeiu");}
void r4300i_tlti(uint32 Instruction){DisplayError("In Trap r4300i_tlti");}
void r4300i_tltiu(uint32 Instruction){DisplayError("In Trap r4300i_tltiu");}
void r4300i_tnei(uint32 Instruction){DisplayError("In Trap r4300i_tnei");}
void r4300i_teq(uint32 Instruction)
{
#ifdef DEBUG_TRAP
	if(debug_trap)
	{
	    sprintf(generalmessage, "Trap r4300i_teq");
		RefreshOpList(generalmessage);
	}
#endif
	gHardwareState.COP0Reg[CAUSE] |= EXC_TRAP;
#ifndef CPUCHECKINTR
	CPUNeedToDoOtherTask = TRUE;
	CPUNeedToCheckException = TRUE;
#endif
}
void r4300i_tge(uint32 Instruction){DisplayError("In Trap r4300i_tge");}
void r4300i_tgeu(uint32 Instruction){DisplayError("In Trap r4300i_tgeu");}
void r4300i_tlt(uint32 Instruction){DisplayError("In Trap r4300i_tlt");}
void r4300i_tltu(uint32 Instruction){DisplayError("In Trap r4300i_tltu");}
void r4300i_tne(uint32 Instruction){DisplayError("In Trap r4300i_tne");}
// instruction cache size 16kb 
void r4300i_cache(uint32 Instruction)
{
	//DisplayError("In CACHE intrunction");
}


