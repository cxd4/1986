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


This project is a documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar. Many source
code comments are taken from anarko's n64toolkit with consent and are 
the property of anarko.
*/

#include <windows.h>
#include <stdio.h>
#include "options.h"
#include "globals.h"
#include "r4300i.h"
#include "hardware.h"
#include "interrupt.h"
#include "win32/windebug.h"
#include "debug_option.h"
#include "timer.h"
#include "memory.h"
#include "1964ini.h"
#include "emulator.h"

//------------------------Local Definition and Variables---------------------------
#define MAXITLB	  2
#define MAXDTLB   3

int ITLB_Index[MAXITLB];
int DTLB_Index[MAXDTLB];
int last_itlb_index;
int last_dtlb_index;
static BOOL newtlb;

//--------------------------Debug Output-------------------------------------------
#ifdef DEBUG_TLB
#define	TLB_TRACE(macro)	{if(debug_tlb) {macro}}
#else
#define TLB_TRACE(macro)
#endif

#ifdef DEBUG_TLB_DETAIL
#define	TLB_DETAIL_TRACE(macro)	{if(debug_tlb_detail) {macro}}
#else
#define	TLB_DETAIL_TRACE(macro)
#endif
//---------------------------------------------------------------------------------

#ifdef DIRECT_TLB_LOOKUP
#define DUMMYTLBINDEX (-1)
#define DUMMYDIRECTTLBVALUE	0xFFFFFFFF
// Define the TLB Lookup Table
uint32 Direct_TLB_Lookup_Table[0x100000];

tlb_struct dummy_tlb;
void Build_Whole_Direct_TLB_Lookup_Table();
void Build_Direct_TLB_Lookup_Table(int index, BOOL tobuild);
void Refresh_Direct_TLB_Lookup_Table(int index);
uint32 Direct_TLB_Lookup( uint32 address, int operation );

//#define BUILD_DIRECT_TLB_MAP

#endif

//---------------------TLB.c Internal Functions------------------------------------
uint32 TLBMapErrorAddress(uint32 address);
uint32 Trigger_TLB_Refill_Exception(uint32 address, int operation);
uint32 Trigger_TLB_Invalid_Exception(uint32 address, int operation);
//---------------------------------------------------------------------------------
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

void r4300i_COP0_tlbp(uint32 Instruction)
{
    uint32      index;
	//uint32      g;

    gHWS_COP0Reg[INDEX] |= 0x80000000; // initially set high-order bit

    for (index = 0; index <= NTLBENTRIES; index++) {

		// Not sure about here, if we should compare the all TLBHI_VPN2MASK, or need to mask with TLB.MyHiMask
		// I think should just mask with TLBHI_VPN2MASK, not with TLB.MyHiMask according to r4300i manual
		// But doing this, some game will give errors of TLBP lookup fail.
		//if( ( (gMS_TLB[index].EntryHi & TLBHI_VPN2MASK) == (gHWS_COP0Reg[ENTRYHI] & TLBHI_VPN2MASK) ) &&
		if( ( (gMS_TLB[index].EntryHi & gMS_TLB[index].MyHiMask) == (gHWS_COP0Reg[ENTRYHI] & gMS_TLB[index].MyHiMask) ) &&

			( (gMS_TLB[index].EntryLo0 & 0x01 & gMS_TLB[index].EntryLo1 ) ||
			  (gMS_TLB[index].EntryHi & TLBHI_PIDMASK)  == (gHWS_COP0Reg[ENTRYHI] & TLBHI_PIDMASK ) ) )
        {

    		gHWS_COP0Reg[INDEX] = index;
			TLB_DETAIL_TRACE(TRACE2( "TLBP - Load INDEX register:%d, VPN2 = 0x%08X [bit 31-13]", index, ((uint32)gHWS_COP0Reg[ENTRYHI] & TLBHI_VPN2MASK)))
            return;
        }
    }
	TLB_TRACE(TRACE1( "TLBP - no match, VPN2 = 0x%08X [bit 31-13]", ((uint32)gHWS_COP0Reg[ENTRYHI] & TLBHI_VPN2MASK)))
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
    uint32 index = gHWS_COP0Reg[INDEX];

#ifdef DEBUG_TLB
    if (index > NTLBENTRIES) {
        DisplayError("ERROR: tlbr received an invalid index");
    }
#endif

	index &= NTLBENTRIES;

	TLB_DETAIL_TRACE(TRACE1( "TLBR - Read from TLB[%d]", index))
	gHWS_COP0Reg[PAGEMASK] = (uint32)(gMS_TLB[index].PageMask);
	gHWS_COP0Reg[ENTRYHI] =  (uint32)(gMS_TLB[index].EntryHi);
    gHWS_COP0Reg[ENTRYLO1] = (uint32)(gMS_TLB[index].EntryLo1);
    gHWS_COP0Reg[ENTRYLO0] = (uint32)(gMS_TLB[index].EntryLo0);
}

void tlb_write_entry(int index)
{
	tlb_struct  *theTLB;

    int g = (gHWS_COP0Reg[ENTRYLO0] & gHWS_COP0Reg[ENTRYLO1]) & 0x01;	// The g bit

#ifdef DIRECT_TLB_LOOKUP
	if( index == DUMMYTLBINDEX )	// please write to dummy tlb
	{
		theTLB = &dummy_tlb;
	}
	else
	    theTLB = &gMS_TLB[index];
#else
    theTLB = &gMS_TLB[index];
#endif

    if (index > NTLBENTRIES) { DisplayError("ERROR: tlbwi called with invalid index"); return;}

#ifdef DEBUG_TLB_DETAIL
	if( debug_tlb_detail ) {
		TRACE1( "TLBWI - Load TLB[%d]", index);
		TRACE2( "PAGEMASK = 0x%08X, ENTRYHI = 0x%08X", (uint32)gHWS_COP0Reg[PAGEMASK],(uint32)gHWS_COP0Reg[ENTRYHI]);
		TRACE2( "ENTRYLO1 = 0x%08X, ENTRYLO0 = 0x%08X", (uint32)gHWS_COP0Reg[ENTRYLO1],(uint32)gHWS_COP0Reg[ENTRYLO0]);
	}
#endif

	theTLB->valid = 0;	// Not sure if we should use this valid bit any more
						// according to TLB comparasion, it is OK to have only EntryLo0 or EntryLo1 valid, not both of them
						// yes, still need in Direct TLB lookup

	if( (gHWS_COP0Reg[ENTRYLO1] & TLBLO_V ) || (gHWS_COP0Reg[ENTRYLO0] & TLBLO_V ) )
		theTLB->valid = 1;
    
	theTLB->PageMask = gHWS_COP0Reg[PAGEMASK];
    theTLB->EntryLo1 = (gHWS_COP0Reg[ENTRYLO1] | g);
    theTLB->EntryLo0 = (gHWS_COP0Reg[ENTRYLO0] | g);
    theTLB->MyHiMask = ~(uint32)theTLB->PageMask & TLBHI_VPN2MASK;
    theTLB->EntryHi = gHWS_COP0Reg[ENTRYHI];	// This EntryHi should just copy from the ENTRYHI register
														// we will not store the g bit in the EntryHi, but keep them in EntryLo0 and EntryLo1
	//theTLB->EntryHi = (gHWS_COP0Reg[ENTRYHI] & (~(uint32)gHWS_COP0Reg[PAGEMASK])) | (gHWS_COP0Reg[PAGEMASK]&TLBHI_PIDMASK);


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

	newtlb = TRUE;
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
#ifdef DIRECT_TLB_LOOKUP
	Refresh_Direct_TLB_Lookup_Table(gHWS_COP0Reg[INDEX]&NTLBENTRIES);
#else
	tlb_write_entry(gHWS_COP0Reg[INDEX]&NTLBENTRIES);
#endif
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
    int index;

#ifdef FAST_COUNTER
	gHWS_COP0Reg[RANDOM] = Get_COUNT_Register() % (0x40-(gHWS_COP0Reg[WIRED]&0x3f))+gHWS_COP0Reg[WIRED];
#else
	gHWS_COP0Reg[RANDOM] = gHWS_COP0Reg[COUNT] % (0x40-(gHWS_COP0Reg[WIRED]&0x3f))+gHWS_COP0Reg[WIRED];
#endif
	//gHWS_COP0Reg[RANDOM] = gHWS_COP0Reg[COUNT] % (0x40-(gHWS_COP0Reg[WIRED]&0x3f))+gHWS_COP0Reg[WIRED];

	index = gHWS_COP0Reg[RANDOM] & NTLBENTRIES;
#ifdef DIRECT_TLB_LOOKUP
	Refresh_Direct_TLB_Lookup_Table(index);
#else
	tlb_write_entry(index);
#endif
}	

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
void r4300i_COP0_eret(uint32 Instruction) 
{
	/*
#ifdef DEBUG_DYNA
		TRACE3( "ERET: PC = %08X, ERROREPC=%08X, EPC=%08X", gHWS_pc, gHWS_COP0Reg[ERROREPC], gHWS_COP0Reg[EPC]);
#endif
		*/
    if ((gHWS_COP0Reg[STATUS] & 0x00000004)) 
    {
        CPUdelayPC = gHWS_COP0Reg[ERROREPC];  //dynarec use: pc = COP0Reg[ERROREPC];
        /* clear the ERL bit to zero */
        gHWS_COP0Reg[STATUS] &= 0xFFFFFFFB; // 0xFFFFFFFB same as ~0x00000004;
    }
    else
    {
        CPUdelayPC = gHWS_COP0Reg[EPC]; //dynarec use: pc = COP0Reg[EPC];
        /* clear the EXL bit of the status register to zero */
        gHWS_COP0Reg[STATUS] &= 0xFFFFFFFD; //0xFFFFFFFD same as ~0x00000002
    }
    CPUdelay = 2;//not used in dynarec
    gHWS_LLbit    =  0;
	//gHWS_COP0Reg[CAUSE] &= NOT_EXCCODE;
	/*
#ifdef DEBUG_COMMON
	if( CPUdelayPC == 0x00000000 )
		DisplayError("ERET: return PC=00000000, ERROREPC=%08X, EPC=%08X", gHWS_COP0Reg[ERROREPC], gHWS_COP0Reg[EPC]);
#endif
	*/
}
uint32 TranslateITLBAddress(uint32 address)
{
	if( currentromoptions.Use_TLB != USETLB_YES )
	{
		TLB_TRACE(TRACE1("Warning, Need to use TLB, Vaddr=%08X", address))
		return ((address&0x1FFFFFFF)|0x80000000);
	}
	else
	{
#ifdef DIRECT_TLB_LOOKUP
		return Direct_TLB_Lookup(address, 2);
#else
		return TranslateTLBAddress(address,2);
#endif
	}
}

uint32 TranslateTLBAddressForLoad(uint32 address)
{
	if( currentromoptions.Use_TLB != USETLB_YES )
	{
		TLB_TRACE(TRACE1("Warning, Need to use TLB-Load, Vaddr=%08X", address))
		return ((address&0x1FFFFFFF)|0x80000000);
	}
	else
	{
#ifdef DIRECT_TLB_LOOKUP
		return Direct_TLB_Lookup(address, 0);
#else
		return TranslateTLBAddress(address,0);
#endif
	}
}

uint32 TranslateTLBAddressForStore(uint32 address)
{
	if( currentromoptions.Use_TLB != USETLB_YES )
	{
		TLB_TRACE(TRACE1("Warning, Need to use TLB-Store, Vaddr=%08X", address))
		return ((address&0x1FFFFFFF)|0x80000000);
	}
	else
	{
#ifdef DIRECT_TLB_LOOKUP
		return Direct_TLB_Lookup(address, 1);
#else
		return TranslateTLBAddress(address,1);
#endif
	}
}


uint32 RedoTLBLookupAfterException(uint32 address, int operation)
{
    uint32      realAddress = 0x80000000;
    _int32      c;
    tlb_struct  *theTLB;
    uint32      EntryLo;

    // search the tlb entries
    for (c = 0; c < MAXTLB; c++) 
	{
        theTLB = &gMS_TLB[c];

        // skip unused entries
        //if (theTLB->valid == 0)                             continue;
        //if ( ((theTLB->EntryLo0 | theTLB->EntryLo1)) == 0)  continue;

        // compare upper bits
		//if ((address & theTLB->MyHiMask & 0x1FFFFFFF) == (theTLB->EntryHi & theTLB->MyHiMask & 0x1FFFFFFF)) 
		if ((address & theTLB->MyHiMask) == (theTLB->EntryHi & theTLB->MyHiMask)) 
		{
            // check the global bit
			if( ((0x01 & theTLB->EntryLo1 & theTLB->EntryLo0) == 1) ||	// g bit match
				((theTLB->EntryHi & TLBHI_PIDMASK) == (gHWS_COP0Reg[ENTRYHI] & TLBHI_PIDMASK)) )	// ASID match
			{
                // select EntryLo depending on if we're in an even or odd page
                if (address & theTLB->LoCompare)	EntryLo = theTLB->EntryLo1;
                else			                    EntryLo = theTLB->EntryLo0;

                if (EntryLo & 0x02) 
				{
                    // calculate the real address from EntryLo
                    realAddress |= ((EntryLo << 6) & ((theTLB->MyHiMask) >> 1));
                    realAddress |= (address & ((theTLB->PageMask | 0x00001FFF) >> 1));
                    return realAddress;
                } 
				else 
				{
#ifdef ADVANCED_TLB
					continue;
#else
                    // invalid tlb entry
                    goto error;
#endif
                }
            } 
        }
    }

#ifndef ADVANCED_TLB
error:
#endif
	TLB_TRACE(TRACE1("Redo TLB check still fail, mapped to dummy memory section, addr=%08x", address))

	if( operation == 2 )
	{
		DisplayError("TLBL Missing Exception for Instruction Access, Bad VPN2 = 0x%8X, Bad Virtual Address = 0x%08X", address>>13, address);
	}

	return TLBMapErrorAddress(address);
}

// This is the main function to do mapped virtual address to physical address translation
uint32 TranslateTLBAddress(uint32 address, int operation)
{
	BOOL invalidmatched;

    uint32      realAddress = 0x80000000;
    _int32      c;
    tlb_struct  *theTLB;
    uint32      EntryLo;

	int maxtlb;
	int *tlb_index_array;
	int *lastused;
	
	if( operation == 2 ) 	{ maxtlb = MAXITLB; tlb_index_array = ITLB_Index; lastused = &last_itlb_index; }
	else					{ maxtlb = MAXDTLB; tlb_index_array = DTLB_Index; lastused = &last_dtlb_index; }

	invalidmatched = FALSE;

    // search the tlb entries
    for (c = 0; c < MAXTLB+maxtlb; c++) 
	//for (c = 0; c < MAXTLB; c++) 
	{
		if( c < maxtlb )
			theTLB = &gMS_TLB[*(tlb_index_array+(maxtlb+*lastused-c)%maxtlb)];	//Check micro TLB first
		else
			theTLB = &gMS_TLB[c-maxtlb];

        // skip unused entries
        //if (theTLB->valid == 0)                             continue;
        //if ( ((theTLB->EntryLo0 | theTLB->EntryLo1) & TLBLO_V ) == 0)  continue;	// Skip this entry if both EntroLo0 and EntryLo1 are invalid

        // compare upper bits
		// This is a unreasonable hack, I should not use this 0x1FFFFFFF mask, but
		// I can not find the bugs in dyna, without this mask, many games will not work well
		// games like Snowboard Kids 2
		//if ((address & theTLB->MyHiMask &0x1FFFFFFF ) == (theTLB->EntryHi & theTLB->MyHiMask & 0x1FFFFFFF)) 

		if ((address & theTLB->MyHiMask ) == (theTLB->EntryHi & theTLB->MyHiMask )) //match, this line is correct
		{
            // check the global bit
            if( ((0x01 & theTLB->EntryLo1 & theTLB->EntryLo0) == 1) ||	// g bit match
				((theTLB->EntryHi & TLBHI_PIDMASK) == (gHWS_COP0Reg[ENTRYHI] & TLBHI_PIDMASK)) )	// ASID match
			{
                // select EntryLo depending on if we're in an even or odd page
                if (address & theTLB->LoCompare)	EntryLo = theTLB->EntryLo1;
                else			                    EntryLo = theTLB->EntryLo0;

                if (EntryLo & 0x02) // Is the entry valid ??
				{
                    // calculate the real address from EntryLo
                    realAddress |= ((EntryLo << 6) & ((theTLB->MyHiMask) >> 1));
					realAddress |= (address & ((theTLB->PageMask | 0x00001FFF) >> 1));
#ifdef DIRECT_TLB_LOOKUP
#ifndef BUILD_DIRECT_TLB_MAP
					Direct_TLB_Lookup_Table[address/0x1000] = (realAddress&0xFFFFF000);

//					TLB_sDWORD_R[address/0x1000] = sDWord[realAddress>>16]+(realAddress&0x0000F000);
					TLB_sDWORD_R[address/0x1000] = TLB_sDWORD_R[realAddress/0x1000];

					TLB_TRACE(TRACE2("Direct TLB Map: VA=%08X, PA=%08X", (address&0xFFFFF000), (realAddress&0xFFFFF000)))
#endif
#endif


					if( c >= maxtlb )
					{
						// Set the microTLB
						*lastused = (*lastused+1)%maxtlb;
						*(tlb_index_array+*lastused) = c-maxtlb;
						TLB_TRACE(TRACE3( "%s remapped to TLB at %d, vaddr=%08x", operation==2?"ITLB":"DTLB", c-maxtlb, address))
					}
					else
					{
						*lastused = c;
					}
					return realAddress;
                } 
				else 
				{
					invalidmatched = TRUE;
#ifdef ADVANCED_TLB
					continue;
#else
					// Trigger TLB Invalid exception
					return(Trigger_TLB_Invalid_Exception(address, operation));
#endif
                }
            } 
        }
    }
#ifdef ADVANCED_TLB
	if( invalidmatched )
		return(Trigger_TLB_Invalid_Exception(address, operation));
	else
#endif
	// Fire TLB refill exception here
	return (Trigger_TLB_Refill_Exception(address, operation));
}

uint32 Trigger_TLB_Refill_Exception(uint32 address, int operation)
{
#ifdef DEBUG_TLB
	if( debug_tlb ) 
	{
		if( operation == 0 )
			TRACE2("0x%08X: TLBL Refill Exception, Bad Virtual Address = 0x%08X", gHWS_pc, address)
		else if( operation == 1 )
			TRACE2("0x%08X: TLBS Refill Exception, Bad Virtual Address = 0x%08X", gHWS_pc, address)
		else
			TRACE2("0x%08X: TLB Refill Exception for Instruction, VA=%08X", gHWS_pc, address)
	}
#endif

	gHWS_COP0Reg[BADVADDR] = address;

	gHWS_COP0Reg[CONTEXT] &= 0xFF800000;	// Mask off bottom 23 bits
	gHWS_COP0Reg[CONTEXT] |= ((address>>13)<<4);

	gHWS_COP0Reg[ENTRYHI] &= 0x00001FFF;	// Mask off the top bit 13-31
	gHWS_COP0Reg[ENTRYHI] |= (address&0xFFFFE000);

	if( operation == 1 )
		SET_EXCEPTION(TLBS_Miss)
	else
		SET_EXCEPTION(TLBL_Miss)

	newtlb = FALSE;

	HandleExceptions(0x80000080);

	if( newtlb )	// if a new TLB entry is written in the TLB exception service routine
	{
		// Could make infinite loop, be careful here
		return( TranslateTLBAddress( address, operation) );
	}
	else
	{
		// Redo TLB lookup, of course, redo will still fail, need a little optimize here
		return( RedoTLBLookupAfterException(address, operation) );
	}
}
uint32 Trigger_TLB_Invalid_Exception(uint32 address, int operation)
{
#ifdef DEBUG_TLB
	if( debug_tlb ) 
	{
		if( operation == 0 )
			TRACE2("0x%08X: TLBL Invalid Exception, Bad Virtual Address = 0x%08X", gHWS_pc, address)
		else if( operation == 1 )
			TRACE2("0x%08X: TLBS Invalid Exception, Bad Virtual Address = 0x%08X", gHWS_pc, address)
		else
			TRACE2("0x%08X: TLB Invalid Exception for Instruction, VA=%08X", gHWS_pc, address)

		TRACE1("Bad Virtual Address = 0x%08X", address);
	}
#endif

	gHWS_COP0Reg[BADVADDR] = address;

	gHWS_COP0Reg[CONTEXT] &= 0xFF800000;	// Mask off bottom 23 bits
	gHWS_COP0Reg[CONTEXT] |= ((address>>13)<<4);

	gHWS_COP0Reg[ENTRYHI] &= 0x00001FFF;	// Mask off the top bit 13-31
	gHWS_COP0Reg[ENTRYHI] |= (address&0xFFFFE000);

	if( operation == 1 )
		SET_EXCEPTION(TLBS_Miss)
	else
		SET_EXCEPTION(TLBL_Miss)

	newtlb = FALSE;

	HandleExceptions(0x80000180);

	if( newtlb )	// if a new TLB entry is written in the TLB exception service routine
	{
		// Could make infinite loop, be careful here
		return( TranslateTLBAddress( address, operation) );
	}
	else
	{
		// Redo TLB lookup, of course, redo will still fail, need a little optimize here
		return( RedoTLBLookupAfterException(address, operation) );
	}
}

void InitTLB()
{
	int i;
	for( i=0; i<MAXTLB; i++ )
		memset(&gMemoryState.TLB[i], 0, sizeof( tlb_struct ));

	last_itlb_index=0;
	for( i=0; i<MAXITLB; i++ )
		ITLB_Index[i]=0;

	last_dtlb_index=0;
	for( i=0; i<MAXDTLB; i++ )
		DTLB_Index[i]=0;

#ifdef DIRECT_TLB_LOOKUP
	// Initialize the TLB Lookup Table
	// For KSEG0 and KSEG1 memory addresses, we still point to dummysegment here
	// because this memory will never use TLB for translation ??
	// Of course, this is true for most of games, Conker's Bad Fur Day is an extra.
	// we do not worry about it now

	//memset(&Direct_TLB_Lookup_Table[0], 0xFF, sizeof(Direct_TLB_Lookup_Table));

	for( i = 0; i < 0x100000; i++ )
		Direct_TLB_Lookup_Table[i] = 0xFFFFFFFF;	// This is a magic number, to represent invalid TLB address
											// we probably need to review a little bit here laterly

	for( i=0; i<0x100000; i++)
	{
		TLB_sDWORD_R[i] = sDWord[i>>4]+0x1000*(i&0xf);
	}

#endif
}

// This function will try best to map an TLB error virtual address to its real address
uint32 TLBMapErrorAddress( uint32 address )
{
	// Check if the address is in any readable memory region
	uint32 dummyword;

	__try
	{
		dummyword = LOAD_UWORD_PARAM(address);
		return ((address&0x1fffffff)|0x80000000);
	}
	__except(NULL,EXCEPTION_EXECUTE_HANDLER)
	{
        __try
		{
            dummyword = LOAD_UWORD_PARAM_2(address);
			return (address&0x1fffffff);
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
			return ((address&0x0000FFFF+0x1FFF0000)|0x80000000);	// mapped to dummy segment
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
	if( gRS == gRT )
	{
#ifdef DEBUG_TRAP
		if(debug_trap)
			TRACE0( "Trap r4300i_teq");
#endif
		SET_EXCEPTION(EXC_TRAP)
		HandleExceptions(0x80000180);
	}
}
void r4300i_tge(uint32 Instruction)
{
	//DisplayError("In Trap r4300i_tge");
	if( gRS >= gRT )
	{
#ifdef DEBUG_TRAP
		if(debug_trap)
			TRACE0( "Trap r4300i_tge");
#endif
		SET_EXCEPTION(EXC_TRAP)
		HandleExceptions(0x80000180);
	}
}
void r4300i_tgeu(uint32 Instruction){DisplayError("In Trap r4300i_tgeu");}
void r4300i_tlt(uint32 Instruction){DisplayError("In Trap r4300i_tlt");}
void r4300i_tltu(uint32 Instruction){DisplayError("In Trap r4300i_tltu");}
void r4300i_tne(uint32 Instruction)
{
	//DisplayError("In Trap r4300i_tne");
	if( gRS != gRT )
	{
#ifdef DEBUG_TRAP
		if(debug_trap)
			TRACE0( "Trap r4300i_tne");
#endif
		SET_EXCEPTION(EXC_TRAP)
		HandleExceptions(0x80000180);
	}
}

// instruction cache size 16kb 
void r4300i_cache(uint32 Instruction)
{
	//DisplayError("In CACHE intrunction");
}

#ifdef DIRECT_TLB_LOOKUP
// This function will translate a virtual address to physical address by looking up in the TLB table
uint32 Direct_TLB_Lookup( uint32 address, int operation )
{
	uint32 realAddress = Direct_TLB_Lookup_Table[address >> 12] ;

	if( realAddress == 0xFFFFFFFF )
	{
		return TranslateTLBAddress(address, operation);
	}
	else
	{
		/*
		uint32 realAddr, realDirectAddr;
		realDirectAddr = Direct_TLB_Lookup_Table[address >> 12] + (address&0x00000FFF);
		realAddr = TranslateTLBAddress(address, operation);
		if( realDirectAddr != realAddr )
		{
			DisplayError("Direct TLB fails: VA=%08X, Direct=%08X, Indirect=%08X", address, realDirectAddr, realAddr);
			realAddr = TranslateTLBAddress(address, operation);

		}
		return realAddr;
		*/
		return( realAddress + (address&0x00000FFF) );
	}
}

// This function is called when an new TLB entry is written
// by careful, this function should be called before the new TLB entry is written from register into
// selected TLB entry table, not after the TLB entry is written.
// Reason is that we need to old TLB entry and compare to the new TLB entry content which is still
// in the registers

// Input parameter "index" is the TLB entry which is to be written
void Refresh_Direct_TLB_Lookup_Table(int index)
{
	// This function need optimized otherwise it will be slow, I hope games are using TLB, but not
	// change TLB all the time. At least games should not change the TLB in used all the time while 
	// leave the TLB entries not in used untouched forever.
	// But I know some games are just constantly used only a few TLB

	//TLB_TRACE(TRACE0("Refresh Direct TLB Lookup Table"))

    if (index > NTLBENTRIES) {
        DisplayError("ERROR: tlbwi called with invalid index");	// This should never happen
		return;
    }
	else
	{
		tlb_struct  *theTLB = &gMS_TLB[index];
		int old_g = (gHWS_COP0Reg[ENTRYLO0] & gHWS_COP0Reg[ENTRYLO1]) & 0x01;
		int old_valid = theTLB->valid;
		int new_valid = ((gHWS_COP0Reg[ENTRYLO1] & TLBLO_V ) || (gHWS_COP0Reg[ENTRYLO0] & TLBLO_V ));
        
	// case1: If the new TLB is invalid and the old TLB is also invalid, do nothing
		if( old_valid == 0 && new_valid == 0 )
		{
			tlb_write_entry(index);
			return;
		}

	// case2: Compare the new TLB and the old TLB content, if they are the same, do nothing
		tlb_write_entry(DUMMYTLBINDEX);
		if( memcmp(&dummy_tlb, theTLB, sizeof(tlb_struct)) == 0 )
		{
			tlb_write_entry(index);
			return;
		}

	// case3: If the old TLB is invalid, and the new TLB is valid, do something, without checking
	//        other TLB entries
		if( old_valid == 0 /*And the new is valid*/ )
		{
			tlb_write_entry(index);	//write the new entry

			// Build the direct TLB lookup table, only for the new entry
#ifdef BUILD_DIRECT_TLB_MAP
			Build_Direct_TLB_Lookup_Table(index, TRUE);
#endif
			return;
		}

	// case4: If the old TLB is valid and new TLB is invalid, do something, need to be careful
		if( new_valid == 0 )
		{
			Build_Direct_TLB_Lookup_Table(index, FALSE);	// Clear the old map
			tlb_write_entry(index);	//write the new entry
			return;
		}
	// case5; If the old TLB is valid and new TLB is also valid, do something, need to be careful
		Build_Direct_TLB_Lookup_Table(index, FALSE);	// Clear the old map
		tlb_write_entry(index);	//write the new entry

		// Build the direct TLB lookup table, only for the new entry
#ifdef BUILD_DIRECT_TLB_MAP
		Build_Direct_TLB_Lookup_Table(index, TRUE);
#endif
		return;

	// For the case 4 and case 5, the easiest way is to write the new content and refresh the whole
	// TLB lookup table, this will be a little slower
	// If I can do better, I prefer to exam the conflict TLB entries, and modify the TLB Lookup
	// table only if needed

	// here is the easiest solution
	// 1. Write new TLB into TLB table (not Direct TLB Lookup Table)
		//tlb_write_entry(index);
	// 2. Refresh the whole Direct TLB Lookup Table
		//Build_Whole_Direct_TLB_Lookup_Table();
	}
}

// Need to be very careful in this function, this function is not good yet
// Two parameter:	index -> this TLB entry index to be build
//					tobuild = yes/no	yes=to build		no=to clear
void Build_Direct_TLB_Lookup_Table(int index, BOOL tobuild)
{
	uint32 lowest, highest, middle, realAddress, i;
	tlb_struct  *theTLB = &gMS_TLB[index];

	if( theTLB->valid == 0 ) return;

	if( (theTLB->EntryLo0 & theTLB->EntryLo1 & 1) == 0 ) // g bit == 0
	{
		return;	// should check teh ASID field, but we can not support ASID in direct TLB lookup
	}

	//Step 1: To calculate the mapped address range that this TLB entry is mapping
	lowest = (theTLB->EntryHi & 0xFFFFFF00 );	// Don't support ASID field
	middle = lowest + theTLB->LoCompare;
	highest = lowest+theTLB->LoCompare*2;

	//Step 2: Check the EntryLo1
	if( theTLB->EntryLo0 & 0x2 )	// is the TLB EntryLo is valid?
	{
		realAddress = 0x80000000;

		
                    //realAddress |= ((EntryLo << 6) & ((theTLB->MyHiMask) >> 1));
                    //realAddress |= (address & ((theTLB->PageMask | 0x00001FFF) >> 1));
		
		
		
		
		realAddress |= ((theTLB->EntryLo0 << 6) & ((theTLB->MyHiMask) >> 1));
#ifdef DEBUG_COMMON
		if( debug_tlb )
		{
			if( tobuild )
			{
				sprintf(tracemessage, "Direct TLB Map: entry=%d-0, VA=%08X-%08X, PA=%08X-%08X", index, lowest, lowest+theTLB->LoCompare-1, realAddress, realAddress+theTLB->LoCompare-1);
			}
			else
			{
				sprintf(tracemessage, "Direct TLB UnMap: entry=%d-0, VA=%08X-%08X, PA=%08X-%08X", index, lowest, lowest+theTLB->LoCompare-1, realAddress, realAddress+theTLB->LoCompare-1);
			}
			RefreshOpList(tracemessage);
		}
#endif
		for( i=lowest/0x1000; i<middle/0x1000; i++ )
		{
			if( tobuild )
			{
				//TLB_TRACE(TRACE2("Direct TLB Map: VA=%08X, PA=%08X", i*0x1000, realAddress+i*0x1000-lowest))
				Direct_TLB_Lookup_Table[i] = (realAddress+i*0x1000-lowest);

				{
					uint32 real = realAddress+i*0x1000-lowest;
					//TLB_sDWORD_R[i] = sDWord[real>>16]+(real&0x0000F000);
					TLB_sDWORD_R[i] = TLB_sDWORD_R[real/0x1000];
				}
			}
			else
			{
				//TLB_TRACE(TRACE2("Direct TLB UnMap: VA=%08X, OldPA=%08X", i*0x1000, realAddress+i*0x1000-lowest))
				Direct_TLB_Lookup_Table[i] = DUMMYDIRECTTLBVALUE;
				{
					TLB_sDWORD_R[i] = sDWord[i>>4]+0x1000*(i&0xf);
				}
			}
		}

	}

	//Step 3: Check the EntryLo0
	if( theTLB->EntryLo1 & 0x2 )	// is the TLB EntryLo is valid?
	{
		realAddress = 0x80000000;
		realAddress |= ((theTLB->EntryLo1 << 6) & ((theTLB->MyHiMask) >> 1));
#ifdef DEBUG_COMMON
		if( debug_tlb )
		{
			if( tobuild )
			{
				sprintf(tracemessage, "Direct TLB Map: entry=%d-1, VA=%08X-%08X, PA=%08X-%08X", index, middle, middle+theTLB->LoCompare-1, realAddress, realAddress+theTLB->LoCompare-1);
			}
			else
			{
				sprintf(tracemessage, "Direct TLB UnMap: entry=%d-1, VA=%08X-%08X, PA=%08X-%08X", index, middle, middle+theTLB->LoCompare-1, realAddress, realAddress+theTLB->LoCompare-1);
			}
			RefreshOpList(tracemessage);
		}
#endif
		for( i=middle/0x1000; i<highest/0x1000; i++ )
		{
			if( tobuild )
			{
				//TLB_TRACE(TRACE2("Direct TLB Map: VA=%08X, PA=%08X", i*0x1000, realAddress+i*0x1000-middle))
				Direct_TLB_Lookup_Table[i] = (realAddress+i*0x1000-middle)|0x80000000;

				{
					uint32 real = realAddress+i*0x1000-middle;
					//TLB_sDWORD_R[i] = sDWord[real>>16]+(real&0x0000F000);
					TLB_sDWORD_R[i] = TLB_sDWORD_R[real/0x1000];
				}
			}
			else
			{
				//TLB_TRACE(TRACE2("Direct TLB UnMap: VA=%08X, OldPA=%08X", i*0x1000, realAddress+i*0x1000-middle))
				Direct_TLB_Lookup_Table[i] = DUMMYDIRECTTLBVALUE;
				{
					TLB_sDWORD_R[i] = sDWord[i>>4]+0x1000*(i&0xf);
				}
			}
		}
	}

	// Remember, step 2 and step 3 is the most important part, if there are error, must be in here
}

void Build_Whole_Direct_TLB_Lookup_Table()
{
	int i;

	// Clean the whole table first
	//memset(&Direct_TLB_Lookup_Table[0], 0xFF, sizeof(Direct_TLB_Lookup_Table));
	for( i = 0; i < 0x100000; i++ )
	Direct_TLB_Lookup_Table[i] = 0xFFFFFFFF;	// This is a magic number, to represent invalid TLB address

	for( i=0; i<0x100000; i++)
	{
		TLB_sDWORD_R[i] = sDWord[i>>4]+0x1000*(i&0xf);
	}

	for( i=0; i<= NTLBENTRIES; i++ )
		Build_Direct_TLB_Lookup_Table(i, TRUE);
}
#else
void Build_Whole_Direct_TLB_Lookup_Table()
{
}
#endif

