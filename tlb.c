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


This project is a documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar :). Many source
code comments are taken from anarko's n64toolkit with consent and are 
the property of anarko.
*/

#include "r4300i.h"
#include "globals.h"

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
	uint32		index, g;

	COP0Reg[INDEX] = 0x80000000;	// initially set high-order bit

	for (index = 0; index < MAXTLB; index++) {
		g = (TLB[index].EntryLo0 & TLB[index].EntryLo1) & 0x01;
		if (((TLB[index].EntryHi & (~0x1FFF)) == (COP0Reg[ENTRYHI] & (~0x1FFF))) &&
				(g || ((TLB[index].EntryHi & 0xFF) == (COP0Reg[ENTRYHI] & 0xFF)))) {

			COP0Reg[INDEX] = index;
			break;
		}
	}
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
	uint32		index = COP0Reg[INDEX] & 0x1F;
	uint32		g = (TLB[index].EntryLo0 & TLB[index].EntryLo1 & 0x01);

	if (index > MAXTLB) {
		DisplayError("ERROR: tlbr received an invalid index");
	}
	else
	{
		COP0Reg[PAGEMASK] = (uint32)(TLB[index].PageMask);
		COP0Reg[ENTRYHI] =  (uint32)(TLB[index].EntryHi & (~(uint32)TLB[index].PageMask));
		COP0Reg[ENTRYLO1] = (TLB[index].EntryLo1 | g);
		COP0Reg[ENTRYLO0] = (TLB[index].EntryLo0 | g);
		
		TLB[index].MyHiMask = ~(uint32)TLB[index].PageMask & 0xFFFFE000;
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
	uint32		index = COP0Reg[INDEX] & 0x1F;
	tlb_struct	*theTLB = &TLB[index];

	if (index > MAXTLB) {
		DisplayError("ERROR: tlbwi called with invalid index");
	}
	else
	{
		theTLB->valid = 1;
		theTLB->PageMask = COP0Reg[PAGEMASK];
		theTLB->EntryHi = COP0Reg[ENTRYHI];
		theTLB->EntryLo1 = COP0Reg[ENTRYLO1];
		theTLB->EntryLo0 = COP0Reg[ENTRYLO0];

		theTLB->MyHiMask = ~(uint32)theTLB->PageMask & 0xFFFFE000;

		switch (theTLB->PageMask) {
			case 0x00000000:	// 4k
				theTLB->LoCompare = 0x00001000;
				break;
			case 0x00006000:	// 16k
				theTLB->LoCompare = 0x00004000;
				break;
			case 0x0001E000:	// 64k
				theTLB->LoCompare = 0x00010000;
				break;
			case 0x0007E000:	// 256k
				theTLB->LoCompare = 0x00040000;
				break;
			case 0x001FE000:	// 1M
				theTLB->LoCompare = 0x00100000;
				break;
			case 0x007FE000:	// 4M
				theTLB->LoCompare = 0x00400000;
				break;
			case 0x01FFE000:	// 16M
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
	int			index = (COP0Reg[RANDOM] & 0x1F);
	tlb_struct	*theTLB = &TLB[index];
	int			g = (COP0Reg[ENTRYLO0] & COP0Reg[ENTRYLO1]) & 0x01;

	if (index > MAXTLB) {
		DisplayError("ERROR: tlbwr called with invalid index");
	}

	// gerrit - uhh.. do we deal with the random reg at all?
	DisplayError("ERROR: opcode tlbwr is not working");

	theTLB->valid = 1;
	theTLB->PageMask = COP0Reg[PAGEMASK];
	theTLB->EntryHi = COP0Reg[ENTRYHI] & (~(uint32)COP0Reg[PAGEMASK]);
	theTLB->EntryLo1 = (COP0Reg[ENTRYLO1] | g);
	theTLB->EntryLo0 = (COP0Reg[ENTRYLO0] | g);

	theTLB->MyHiMask = ~(uint32)theTLB->PageMask & 0xFFFFE000;
	
	switch (theTLB->PageMask) {
		case 0x00000000:	// 4k
			theTLB->LoCompare = 0x00001000;
			break;
		case 0x00006000:	// 16k
			theTLB->LoCompare = 0x00004000;
			break;
		case 0x0001E000:	// 64k
			theTLB->LoCompare = 0x00010000;
			break;
		case 0x0007E000:	// 256k
			theTLB->LoCompare = 0x00040000;
			break;
		case 0x001FE000:	// 1M
			theTLB->LoCompare = 0x00100000;
			break;
		case 0x007FE000:	// 4M
			theTLB->LoCompare = 0x00400000;
			break;
		case 0x01FFE000:	// 16M
			theTLB->LoCompare = 0x01000000;
			break;
		default:
			DisplayError("ERROR: tlbwr - invalid page size");
			break;
	}
}

extern uint32* InstructionPointer;
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

	if ((COP0Reg[STATUS] & 0x00000004)) 
	{
		CPUdelayPC = COP0Reg[ERROREPC];	//dynarec use: pc = COP0Reg[ERROREPC];
		/* clear the ERL bit to zero */
		COP0Reg[STATUS] &= 0xFFFFFFFB; // 0xFFFFFFFB same as ~0x00000004;
	}
	else
	{
		CPUdelayPC = COP0Reg[EPC]; //dynarec use: pc = COP0Reg[EPC];
		/* clear the EXL bit of the status register to zero */
		COP0Reg[STATUS] &= 0xFFFFFFFD; //0xFFFFFFFD same as ~0x00000002
	}
	CPUdelay = 2;//not used in dynarec
	LLbit	=  0;
}


//TRAPS
void r4300i_teqi(uint32 Instruction){}
void r4300i_tgei(uint32 Instruction){}
void r4300i_tgeiu(uint32 Instruction){}
void r4300i_tlti(uint32 Instruction){}
void r4300i_tltiu(uint32 Instruction){}
void r4300i_tnei(uint32 Instruction){}
void r4300i_teq(uint32 Instruction){}
void r4300i_tge(uint32 Instruction){}
void r4300i_tgeu(uint32 Instruction){}
void r4300i_tlt(uint32 Instruction){}
void r4300i_tltu(uint32 Instruction){}
void r4300i_tne(uint32 Instruction){}
void r4300i_cache(uint32 Instruction){}