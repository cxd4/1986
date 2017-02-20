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
#include "options.h"
#include "debug_option.h"
#include "globals.h"
#include "r4300i.h"
#include "hardware.h"
#include "n64rcp.h"
#include "memory.h"
#include "iPIF.h"
#include "timer.h"
#include "memory.h"
#include "1964ini.h"
#include "interrupt.h"
#include "flashram.h"
#include "dma.h"
#include "emulator.h"
#include "timer.h"
#include "interrupt.h"
#include "win32/DLL_Video.h"
#include "win32/DLL_Audio.h"
#include "win32/windebug.h"
#include "win32/wingui.h"

//---------------------------------------------------------------------------------------
//	r4300i.c internal macro definition
//---------------------------------------------------------------------------------------
#ifdef DEBUG_TLB
#define DISPLAY_ADDRESS_ERROR( addr, opcodestr )	if( debug_tlb )	{Display_Address_Error( addr, opcodestr );}
#else
#define DISPLAY_ADDRESS_ERROR( addr, opcodestr )
#endif

#ifdef ENABLE_OPCODE_DEBUGGER
#define OPCODE_DEBUGGER_BRANCH_SKIP		CPUdelay=0;
#else
#define OPCODE_DEBUGGER_BRANCH_SKIP		{}
#endif
//---------------------------------------------------------------------------------------
//	r4300i.c internal functions
//---------------------------------------------------------------------------------------
extern void DebugIO(uint32, char *, uint32);
__inline void Display_Address_Error( uint32 addr, char *opcode );
void Faster_Check_SW(uint32 QuerAddr, uint32 rt_ft);

//---------------------------------------------------------------------------------------

void r4300i_break(uint32 Instruction)
{
	// Trigger the BREAK Exception
	SET_EXCEPTION(EXC_BREAK);
	HandleExceptions(0x80000180);
}

void r4300i_sync(uint32 Instruction)
{
//#ifdef DEBUG_COMMON
//	DisplayError("Intruction SYNC, not implemented yet");
//#endif
}

void r4300i_syscall(uint32 Instruction)
{
	//Cause a SYSCALL exception
	SET_EXCEPTION(EXC_SYSCALL)
	HandleExceptions(0x80000180);
}

void UNUSED(uint32 Instruction)         
{
	// Trigger the RI Exception
	//SET_EXCEPTION(EXC_II)
	//HandleExceptions(0x80000180);
	/*DisplayError("%08X: Illegal opcode request.", gHWS_pc);*/       
	TRACE1("Invalid Instructin, PC=%08X", gHWS_pc);
}

/*---------------------------------------------------------------------------------------
	-----------------------------------------------------------------
	| ADD       | ADD word                                          |
	|-----------|---------------------------------------------------|
	|  000000   |   rs    |   rt    |   rd    |  00000  |100000 (32)|
	------6----------5---------5---------5---------5----------6------
	 Format:  ADD rd, rs, rt
	 Purpose: To add 32-bit integers. If overflow occurs, then trap.
	 Comment: ADD rd, r0, rs is equal to a MOVE rd, rs
	 Descrip: rd = rs + rt

	-----------------------------------------------------------------
	| ADDU      | ADD Unsigned word                                 |
	|-----------|---------------------------------------------------|
	|  000000   |   rs    |   rt    |    rd   |  00000  |100001 (33)|
	------6----------5---------5---------5---------5----------6------
	 Format:  ADDU rd, rs, rt
	 Purpose: To add 32-bit integers.
	 Descrip: rd = rs + rt

	-----------------------------------------------------------------
	| ADDI      | ADD Immediate word                                |
	|-----------|---------------------------------------------------|
	|001000 (8) |   rs    |   rt    |          immediate            |
	------6----------5---------5-------------------16----------------
	 Format:  ADDI rt, rs, immediate
	 Purpose: To add a constant to a 32-bit integer.
			  If overflow occurs, then trap.
	 Descrip: rt = rs + immediate

	-----------------------------------------------------------------
	| ADDIU     | ADD Immediate Unsigned word                       |
	|-----------|---------------------------------------------------|
	|001001 (9) |   rs    |   rt    |          immediate            |
	------6----------5---------5-------------------16----------------
	 Format:  ADDIU rt, rs, immediate
	 Purpose: To add a constant to a 32-bit integer.
	 Descrip: rt = rs + immediate
/*---------------------------------------------------------------------------------------*/
void r4300i_add(uint32 Instruction)		{CHECK_R0_EQUAL_0(RD_FS, "add");	sLOGICAL(+);			}  
void r4300i_addu(uint32 Instruction)	{CHECK_R0_EQUAL_0(RD_FS, "addu");	sLOGICAL(+);			}  
void r4300i_addi(uint32 Instruction)	{CHECK_R0_EQUAL_0(RT_FT, "addi");	sLOGICAL_WITH_IMM(+);	}    
void r4300i_addiu(uint32 Instruction)	{CHECK_R0_EQUAL_0(RT_FT, "addiu");	sLOGICAL_WITH_IMM(+);	}
//void r4300i_addiu(uint32 Instruction)	{CHECK_R0_EQUAL_0(RT_FT);	sDLOGICAL_WITH_IMM(+);	}

/*---------------------------------------------------------------------------------------
	-----------------------------------------------------------------
	| DADD      | Doubleword ADD                                    |
	|-----------|---------------------------------------------------|
	|  000000   |   rs    |   rt    |    rd   |  00000  |101100 (44)|
	------6----------5---------5---------5---------5----------6------
	 Format:  DADD rd, rs, rt
	 Purpose: To add 64-bit integers. If overflow occurs, then trap.
	 Descrip: rd = rs + rt

	-----------------------------------------------------------------
	| DADDU     | Doubleword ADD Unsigned                           |
	|-----------|---------------------------------------------------|
	|  000000   |   rs    |   rt    |    rd   |  00000  |101101 (45)|
	------6----------5---------5---------5---------5----------6------
	 Format:  DADDU rd, rs, rt
	 Purpose: To add 64-bit integers.
	 Descrip: rd = rs + rt

	-----------------------------------------------------------------
	| DADDI     | Doubleword ADD Immediate                          |
	|-----------|---------------------------------------------------|
	|011000 (24)|   rs    |   rt    |          immediate            |
	------6----------5---------5-------------------16----------------
	 Format:  DADDI rt, rs, immediate
	 Purpose: To add a constant to a 64-bit integer.
			  If overflow occurs, then trap.
	 Descrip: rt = rs + immediate

	-----------------------------------------------------------------
	| DADDIU    | Doubleword ADD Immediate Unsigned                 |
	|-----------|---------------------------------------------------|
	|011001 (25)|   rs    |   rt    |          immediate            |
	------6----------5---------5-------------------16----------------
	 Format:  DADDIU rt, rs, immediate
	 Purpose: To add a constant to a 64-bit integer.
	 Descrip: rt = rs + immediate
/*---------------------------------------------------------------------------------------*/
void r4300i_dadd(uint32 Instruction)	{CHECK_R0_EQUAL_0(RD_FS, "dadd");	sDLOGICAL(+);			}
void r4300i_daddu(uint32 Instruction)	{CHECK_R0_EQUAL_0(RD_FS, "daddu");	sDLOGICAL(+);			}
void r4300i_daddi(uint32 Instruction)	{CHECK_R0_EQUAL_0(RT_FT, "daddi");	sDLOGICAL_WITH_IMM(+);	}
void r4300i_daddiu(uint32 Instruction)	{CHECK_R0_EQUAL_0(RT_FT, "daddiu");	sDLOGICAL_WITH_IMM(+);	}

/*---------------------------------------------------------------------------------------
	-----------------------------------------------------------------
	| SUB       | SUBtract word                                     |
	|-----------|---------------------------------------------------|
	|  000000   |   rs    |   rt    |    rd   |  00000  |100010 (34)|
	------6----------5---------5---------5---------5----------6------
	 Format:  SUB rd, rs, rt
	 Purpose: To subtract 32-bit integers. If overflow occurs, then trap.
	 Comment: SUB rd, r0, rt is equal to a NEG rd, rt
	 Descrip: rd = rs - rt

	-----------------------------------------------------------------
	| SUBU      | SUBtract Unsigned word                            |
	|-----------|---------------------------------------------------|
	|  000000   |   rs    |   rt    |    rd   |  00000  |100011 (35)|
	------6----------5---------5---------5---------5----------6------
	 Format:  SUBU rd, rs, rt
	 Purpose: To subtract 32-bit integers. No trap on overflow.
	 Comment: SUBU rd, r0, rs is equal to a NEGU rd, rs
	 Descrip: rd = rs - rt

	-----------------------------------------------------------------
	| DSUB      | Doubleword SUBtract                               |
	|-----------|---------------------------------------------------|
	|  000000   |   rs    |   rt    |    rd   |  00000  |101110 (46)|
	------6----------5---------5---------5---------5----------6------
	 Format:  DSUB rd, rs, rt
	 Purpose: To subtract 64-bit integers; trap if overflow.
	 Descrip: rd = rs - rt

	-----------------------------------------------------------------
	| DSUBU     | Doubleword SUBtract Unsigned                      |
	|-----------|---------------------------------------------------|
	|  000000   |   rs    |   rt    |   rd    |  00000  |101111 (47)|
	------6----------5---------5---------5---------5----------6------
	 Format:  DSUBU rd, rs, rt
	 Purpose: To subtract 64-bit integers.
	 Descrip: rd = rs - rt
/*---------------------------------------------------------------------------------------*/

void r4300i_sub(uint32 Instruction)		{CHECK_R0_EQUAL_0(RD_FS, "sub");	sLOGICAL(-);	}  
void r4300i_subu(uint32 Instruction)	{CHECK_R0_EQUAL_0(RD_FS, "subu");	sLOGICAL(-);	}  
//void r4300i_dsub(uint32 Instruction)	{CHECK_R0_EQUAL_0(RD_FS, "dsub");	uDLOGICAL(-);	}
void r4300i_dsub(uint32 Instruction)	{CHECK_R0_EQUAL_0(RD_FS, "dsub");	sDLOGICAL(-);	}
void r4300i_dsubu(uint32 Instruction)	{CHECK_R0_EQUAL_0(RD_FS, "dsubu");	sDLOGICAL(-);	}

/*---------------------------------------------------------------------------------------
	-----------------------------------------------------------------
	| AND       | AND                                               |
	|-----------|---------------------------------------------------|
	|  000000   |   rs    |   rt    |    rd   |  00000  |100100 (36)|
	------6----------5---------5---------5---------5----------6------
	 Format:  AND rd, rs, rt
	 Purpose: To do a bitwise logical AND.
	 Descrip: rd = (rs AND rt)

	-----------------------------------------------------------------
	| ANDI      | AND Immediate                                     |
	|-----------|---------------------------------------------------|
	|001100 (12)|   rs    |   rt    |          immediate            |
	------6----------5---------5-------------------16----------------
	 Format:  ANDI rt, rs, immediate
	 Purpose: To do a bitwise logical AND with a constant.
	 Descrip: rt = (rs AND immediate)
/*---------------------------------------------------------------------------------------*/
void r4300i_and(uint32 Instruction) {CHECK_R0_EQUAL_0(RD_FS, "and");	uDLOGICAL(&);			}   
void r4300i_andi(uint32 Instruction){CHECK_R0_EQUAL_0(RT_FT, "andi");	uDLOGICAL_WITH_IMM(&);	}

/*---------------------------------------------------------------------------------------
	-----------------------------------------------------------------
	| OR        | OR                                                |
	|-----------|---------------------------------------------------|
	|  000000   |   rs    |   rt    |    rd   |  00000  |100101 (37)|
	------6----------5---------5---------5---------5----------6------
	 Format:  OR rd, rs, rt
	 Purpose: To do a bitwise logical OR.
	 Descrip: rd = (rs OR rt)

	-----------------------------------------------------------------
	| ORI       | OR Immediate                                      |
	|-----------|---------------------------------------------------|
	|001101 (13)|   rs    |   rt    |            immediate          |
	------6----------5---------5-------------------16----------------
	 Format:  ORI rt, rs, immediate
	 Purpose: To do a bitwise logical OR with a constant.
	 Comment: ORI rt, r0, imm is equal to a LI rt, imm
	 Descrip: rt = (rs OR immediate)
/*---------------------------------------------------------------------------------------*/
void r4300i_or(uint32 Instruction)  {CHECK_R0_EQUAL_0(RD_FS, "or");		uDLOGICAL(|);			}   
void r4300i_ori(uint32 Instruction) {CHECK_R0_EQUAL_0(RT_FT, "ori");	uDLOGICAL_WITH_IMM(|);	}

/*---------------------------------------------------------------------------------------
	-----------------------------------------------------------------
	| XOR       | eXclusive OR                                      |
	|-----------|---------------------------------------------------|
	|  000000   |   rs    |   rt    |    rd   |  00000  |100110 (38)|
	------6----------5---------5---------5---------5----------6------
	 Format:  XOR rd, rs, rt
	 Purpose: To do a bitwise logical EXCLUSIVE OR.
	 Descrip: rd = (rs XOR rt)

	-----------------------------------------------------------------
	| XORI      | eXclusive OR Immediate                            |
	|-----------|---------------------------------------------------|
	|001110 (14)|   rs    |   rt    |           immediate           |
	------6----------5---------5-------------------16----------------
	 Format:  XORI rt, rs, immediate
	 Purpose: To do a bitwise logical EXCLUSIVE OR with a constant.
	 Descrip: rd = (rs XOR immediate)
/*---------------------------------------------------------------------------------------*/
void r4300i_xor(uint32 Instruction) {CHECK_R0_EQUAL_0(RD_FS, "xor");	uDLOGICAL(^);			}   
void r4300i_xori(uint32 Instruction){CHECK_R0_EQUAL_0(RT_FT, "xori");	uDLOGICAL_WITH_IMM(^);	}

/*---------------------------------------------------------------------------------------
-----------------------------------------------------------------
| NOR       | Not OR                                            |
|-----------|---------------------------------------------------|
|  000000   |   rs    |   rt    |    rd   |  00000  |100111 (39)|
------6----------5---------5---------5---------5----------6------
 Format:  NOR rd, rs, rt
 Purpose: To do a bitwise logical NOT OR.
 Descrip: rd = (rs NOR rt)
/*---------------------------------------------------------------------------------------*/
void r4300i_nor(uint32 Instruction)  {CHECK_R0_EQUAL_0(RD_FS, "nor"); gRD = ~(gRS | gRT);}

//---------------------------------------------------------------------------------------
//-----------------------------------------------------------------
//| LB        | Load Byte                                         |
//|-----------|---------------------------------------------------|
//|100000 (32)|  base   |   rt    |             offset            |
//------6----------5---------5-------------------16----------------
// Format:  LB rt, offset(base)
// Purpose: To load a byte from memory as a signed value.
// Descrip: rt = byte[base+offset]
//---------------------------------------------------------------------------------------

_int32 r4300i_lb_faster(uint32 QuerAddr) 
{
	__try{
        return(LOAD_SBYTE_PARAM(QuerAddr));
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(QuerAddr) )
		{
			QuerAddr = TranslateTLBAddressForLoad(QuerAddr);
		}
		__try{
            return(LOAD_SBYTE_PARAM(QuerAddr));
		}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			__try{
                return(LOAD_SBYTE_PARAM_2(QuerAddr));
				TRACE2("%08X: Use LB to read registers at 0x%08x", gHWS_pc, QuerAddr);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DISPLAY_ADDRESS_ERROR( QuerAddr, "LB" );
				return 0;
			}
		}
   }
}

void r4300i_lb(uint32 Instruction)
{
    uint32 QuerAddr;
    uint32 rt_ft = RT_FT;
    
	CHECK_R0_EQUAL_0(rt_ft, "lb");

    QUER_ADDR;

    gHWS_GPR[rt_ft] = r4300i_lb_faster(QuerAddr);
}



//---------------------------------------------------------------------------------------
//	-----------------------------------------------------------------
//	| LBU       | Load Byte Unsigned                                |
//	|-----------|---------------------------------------------------|
//	|100100 (36)|  base   |   rt    |             offset            |
//	------6----------5---------5-------------------16----------------
//	 Format:  LBU rt, offset(base)
//	 Purpose: To load a byte from memory as an unsigned value.
//	 Descrip: rt = byte[base+offset]
//---------------------------------------------------------------------------------------
uint32 r4300i_lbu_faster(uint32 QuerAddr) 
{   
	__try{
        return(LOAD_UBYTE_PARAM(QuerAddr));
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(QuerAddr) )
		{
			QuerAddr = TranslateTLBAddressForLoad(QuerAddr);
		}
		__try{
            return(LOAD_UBYTE_PARAM(QuerAddr));
		}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			__try{
                return(LOAD_UBYTE_PARAM_2(QuerAddr));
				TRACE2("%08X: Use LBU to read registers at 0x%08x", gHWS_pc, QuerAddr);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DISPLAY_ADDRESS_ERROR( QuerAddr, "LBU" );
				return 0;
			}
		}
   }
}

void r4300i_lbu(uint32 Instruction)
{
    uint32 QuerAddr;
    uint32 rt_ft = RT_FT;
    
	CHECK_R0_EQUAL_0(rt_ft, "lbu");
    QUER_ADDR;

    gHWS_GPR[rt_ft] = r4300i_lbu_faster(QuerAddr);
}



//---------------------------------------------------------------------------------------
//	-----------------------------------------------------------------
//	| LH        | Load Halfword                                     |
//	|-----------|---------------------------------------------------|
//	|100001 (33)|  base   |   rt    |             offset            |
//	------6----------5---------5-------------------16----------------
//	 Format:  LH rt, offset(base)
//	 Purpose: To load a halfword from memory as a signed value.
//	 Descrip: rt = halfword[base+offset]
//---------------------------------------------------------------------------------------
_int32 r4300i_lh_faster(uint32 QuerAddr) 
{   
	ADDR_HALFWORD_ALIGN_CHECKING(QuerAddr)

	__try{
        return(LOAD_SHALF_PARAM(QuerAddr));
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(QuerAddr) )
		{
			QuerAddr = TranslateTLBAddressForLoad(QuerAddr);
		}
		__try{
            return(LOAD_SHALF_PARAM(QuerAddr));
		}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			__try{
                return(LOAD_SHALF_PARAM_2(QuerAddr));
				TRACE2( "%08X: Use LH to read registers at 0x%08x", gHWS_pc, QuerAddr);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DISPLAY_ADDRESS_ERROR( QuerAddr, "LH" );
				return 0;
			}
		}
   }
}

void r4300i_lh(uint32 Instruction)
{
    uint32 QuerAddr;
    uint32 rt_ft = RT_FT;
    
	CHECK_R0_EQUAL_0(rt_ft, "lh");
    
    QUER_ADDR;

    gHWS_GPR[rt_ft] = r4300i_lh_faster(QuerAddr);
}


//---------------------------------------------------------------------------------------
//	-----------------------------------------------------------------
//	| LHU       | Load Halfword Unsigned                            |
//	|-----------|---------------------------------------------------|
//	|100101 (37)|  base   |   rt    |             offset            |
//	------6----------5---------5-------------------16----------------
//	 Format:  LHU rt, offset(base)
//	 Purpose: To load a halfword from memory as an unsigned value.
//	 Descrip: rt = halfword[base+offset]
//---------------------------------------------------------------------------------------
uint32 r4300i_lhu_faster(uint32 QuerAddr) 
{   
	ADDR_HALFWORD_ALIGN_CHECKING(QuerAddr)

	__try{
        return(LOAD_UHALF_PARAM(QuerAddr));
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(QuerAddr) )
		{
			QuerAddr = TranslateTLBAddressForLoad(QuerAddr);
		}
		__try{
            return(LOAD_UHALF_PARAM(QuerAddr));
		}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			__try{
                return(LOAD_UHALF_PARAM_2(QuerAddr));
				TRACE2( "%08X: Use LHU to read registers at 0x%08x", gHWS_pc, QuerAddr);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DISPLAY_ADDRESS_ERROR( QuerAddr, "LHU" );
				return 0;
			}
		}
   }
}

void r4300i_lhu(uint32 Instruction)
{
    uint32 QuerAddr;
    uint32 rt_ft = RT_FT;
    
    CHECK_R0_EQUAL_0(rt_ft, "lhu");
    QUER_ADDR;

    gHWS_GPR[rt_ft] = r4300i_lhu_faster(QuerAddr);
}


//---------------------------------------------------------------------------------------
//	-----------------------------------------------------------------
//	| LWU       | Load Word Unsigned                                |
//	|-----------|---------------------------------------------------|
//	|100111 (39)|  base   |   rt    |             offset            |
//	------6----------5---------5-------------------16----------------
//	 Format:  LWU rt, offset(base)
//	 Purpose: To load a word from memory as an unsigned value.
//	 Descrip: rt = word[base+offset]
//---------------------------------------------------------------------------------------
void r4300i_lwu(uint32 Instruction)
{
    uint32 QuerAddr;
	CHECK_R0_EQUAL_0(RT_FT, "lwu");

    QUER_ADDR;
	ADDR_WORD_ALIGN_CHECKING(QuerAddr)

	__try{
        gRT = LOAD_UWORD_PARAM(QuerAddr);
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(QuerAddr) )
		{
			QuerAddr = TranslateTLBAddressForLoad(QuerAddr);
		}
		__try{
			gRT = LOAD_UWORD_PARAM(QuerAddr);
		}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			__try{
				gRT = LOAD_UWORD_PARAM_2(QuerAddr);
				TRACE2( "%08X: Use LWU to read registers at 0x%08x", gHWS_pc, QuerAddr);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DISPLAY_ADDRESS_ERROR( QuerAddr, "LWU" );
			}
		}
   }
}

//---------------------------------------------------------------------------------------
//	-----------------------------------------------------------------
//	| LL        | Load Linked Word                                  |
//	|-----------|---------------------------------------------------|
//	|110000 (48)|  base   |   rt    |             offset            |
//	------6----------5---------5-------------------16----------------
//	 Format:  LL rt, offset(base)
//	 Purpose: To load a word from memory for an atomic read-modify-write.
//---------------------------------------------------------------------------------------
void r4300i_ll(uint32 Instruction)
{   
    uint32 QuerAddr;
	CHECK_R0_EQUAL_0(RT_FT, "ll");

    QUER_ADDR;
	ADDR_WORD_ALIGN_CHECKING(QuerAddr)

	__try{
        gRT = LOAD_SWORD_PARAM(QuerAddr);
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(QuerAddr) )
		{
			QuerAddr = TranslateTLBAddressForLoad(QuerAddr);
		}
		__try{
			gRT = LOAD_SWORD_PARAM(QuerAddr);
		}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			__try{
				gRT = LOAD_SWORD_PARAM_2(QuerAddr);
				TRACE2( "%08X: Use LL to read registers at 0x%08x", gHWS_pc, QuerAddr);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DISPLAY_ADDRESS_ERROR( QuerAddr, "LL" );
			}
		}
   }
   gHWS_LLbit = 1; //Unconditionally ??
}

//---------------------------------------------------------------------------------------
//	-----------------------------------------------------------------
//	| LD        | Load Doubleword                                   |
//	|-----------|---------------------------------------------------|
//	|110111 (55)|  base   |   rt    |             offset            |
//	------6----------5---------5-------------------16----------------
//	 Format:  LD rt, offset(base)
//	 Purpose: To load a doubleword from memory.
//	 Descrip: rt = doubleword[base+offset]
//---------------------------------------------------------------------------------------

void r4300i_ld_faster(uint32 QuerAddr, uint32 rt_ft)
{
	ADDR_DWORD_ALIGN_CHECKING(QuerAddr)

    // Note: If we add Check_LW() to this, Don't forget to set GPR[r0] = 0
    // at the end of this function and remove "CHECK_R0_EQUAL_0(rt_ft)" 
    // in r4300i_ld(). We won't use CHECK_R0_EQUAL_0(rt_ft) for ld
    // because we'll need to use Check_LW() even when loading into GPR[r0].
    // Also, the dyna will need to remove the Check for 
    // "if (xRT->mips_reg == 0) return;" as well.
        
    __try{
		uint32* pQuerMask = pLOAD_UWORD_PARAM(QuerAddr);
		gHWS_GPR[rt_ft]                          = pQuerMask[1];
		*(uint32*)(&(uint32*)gHWS_GPR[rt_ft]+1)  = pQuerMask[0];
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(QuerAddr) )
		{
			QuerAddr = TranslateTLBAddressForLoad(QuerAddr);
		}
		__try{
			uint32* pQuerMask = pLOAD_UWORD_PARAM(QuerAddr);
			gHWS_GPR[rt_ft]                          = pQuerMask[1];
			*(uint32*)(&(uint32*)gHWS_GPR[rt_ft]+1)  = pQuerMask[0];
		}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			__try{
				uint32* pQuerMask = pLOAD_UWORD_PARAM_2(QuerAddr);
				gHWS_GPR[rt_ft]                          = pQuerMask[1];
				*(uint32*)(&(uint32*)gHWS_GPR[rt_ft]+1)  = pQuerMask[0];
				TRACE2( "%08X: Use LD to read registers at 0x%08x", gHWS_pc, QuerAddr);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DISPLAY_ADDRESS_ERROR( QuerAddr, "LD" );
			}
		}
   }
}

void r4300i_ld(uint32 Instruction)
{
    uint32 QuerAddr;
    uint32 rt_ft = RT_FT;
    
	CHECK_R0_EQUAL_0(rt_ft, "ld");
    QUER_ADDR;

    r4300i_ld_faster(QuerAddr, rt_ft);
}



//---------------------------------------------------------------------------------------
//	-----------------------------------------------------------------
//	| LLD       | Load Linked Doubleword                            |
//	|-----------|---------------------------------------------------|
//	|110100 (52)|  base   |   rt    |             offset            |
//	------6----------5---------5-------------------16----------------
//	 Format:  LLD rt, offset(base)
//	 Purpose: To load a doubleword from memory for an atomic read-modify-write.
//---------------------------------------------------------------------------------------

void r4300i_lld(uint32 Instruction)
{
	uint32 rt_ft = RT_FT;
    uint32 QuerAddr;
	CHECK_R0_EQUAL_0(rt_ft, "lld");
    QUER_ADDR;
	ADDR_DWORD_ALIGN_CHECKING(QuerAddr)

	__try{
		uint32* pQuerMask = pLOAD_UWORD_PARAM(QuerAddr);
		gHWS_GPR[rt_ft]                          = pQuerMask[1];
		*(uint32*)(&(uint32*)gHWS_GPR[rt_ft]+1)  = pQuerMask[0];
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(QuerAddr) )
		{
			QuerAddr = TranslateTLBAddressForLoad(QuerAddr);
		}
		__try{
			uint32* pQuerMask = pLOAD_UWORD_PARAM(QuerAddr);
			gHWS_GPR[rt_ft]                          = pQuerMask[1];
			*(uint32*)(&(uint32*)gHWS_GPR[rt_ft]+1)  = pQuerMask[0];
		}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			__try{
				uint32* pQuerMask = pLOAD_UWORD_PARAM_2(QuerAddr);
				gHWS_GPR[rt_ft]                          = pQuerMask[1];
				*(uint32*)(&(uint32*)gHWS_GPR[rt_ft]+1)  = pQuerMask[0];
				TRACE2( "%08X: Use LLD to read registers at 0x%08x", gHWS_pc, QuerAddr);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DISPLAY_ADDRESS_ERROR( QuerAddr, "LLD" );
			}
		}
	}
	gHWS_LLbit = 1;
}

/*---------------------------------------------------------------------------------------
-----------------------------------------------------------------
| SB        | Store Byte                                        |
|-----------|---------------------------------------------------|
|101000 (40)|  base   |   rt    |             offset            |
------6----------5---------5-------------------16----------------
 Format:  SB rt, offset(base)
 Purpose: To store a byte to memory.
 Descrip: byte[base+offset] = rt
//-------------------------------------------------------------------------------------*/

void r4300i_sb_faster(uint32 QuerAddr, uint32 rt_ft)
{
	__try{
		LOAD_UBYTE_PARAM(QuerAddr) =  (uint8)gHWS_GPR[rt_ft];
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(QuerAddr) )
		{
			QuerAddr = TranslateTLBAddressForStore(QuerAddr);
		}
		__try{
			LOAD_UBYTE_PARAM(QuerAddr) =  (uint8)gHWS_GPR[rt_ft];
		}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			__try{
				LOAD_UBYTE_PARAM_2(QuerAddr) =  (uint8)gHWS_GPR[rt_ft];
				TRACE2("%08X: Use SB to store registers at 0x%08x", gHWS_pc, QuerAddr);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DISPLAY_ADDRESS_ERROR( QuerAddr, "SB" );
			}
		}
   }
}

void r4300i_sb(uint32 Instruction)
{
    uint32 QuerAddr;
    uint32 rt_ft = RT_FT;
    
    QUER_ADDR;

    r4300i_sb_faster(QuerAddr, rt_ft);
}


/*---------------------------------------------------------------------------------------
-----------------------------------------------------------------
| SH        | Store Halfword                                    |
|-----------|---------------------------------------------------|
|101001 (41)|  base   |   rt    |             offset            |
------6----------5---------5-------------------16----------------
 Format:  SH rt, offset(base)
 Purpose: To store a halfword to memory.
 Descrip: halfword[base+offset] = rt
//---------------------------------------------------------------------------------------*/

void r4300i_sh_faster(uint32 QuerAddr, uint32 rt_ft)
{
	ADDR_HALFWORD_ALIGN_CHECKING(QuerAddr)

	__try{
        LOAD_UHALF_PARAM(QuerAddr) =  (uint16)gHWS_GPR[rt_ft];
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(QuerAddr) )
		{
			QuerAddr = TranslateTLBAddressForStore(QuerAddr);
		}
		__try{
			LOAD_UHALF_PARAM(QuerAddr) =  (uint16)gHWS_GPR[rt_ft];
		}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			__try{
				LOAD_UHALF_PARAM_2(QuerAddr) =  (uint16)gHWS_GPR[rt_ft];
				TRACE2("%08X: Use SH to store registers at 0x%08x", gHWS_pc, QuerAddr);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DISPLAY_ADDRESS_ERROR( QuerAddr, "SH" );
			}
		}
   }
}

void r4300i_sh(uint32 Instruction)
{
    uint32 QuerAddr;
    uint32 rt_ft = RT_FT;
    
    QUER_ADDR;

    r4300i_sh_faster(QuerAddr, rt_ft);
}


//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

void r4300i_sd_faster(uint32 QuerAddr, uint32 rt_ft)
{
	ADDR_DWORD_ALIGN_CHECKING(QuerAddr)

	__try{
		uint32* pQuerMask = pLOAD_UWORD_PARAM(QuerAddr);
		pQuerMask[1] = (uint32)gHWS_GPR[rt_ft];
		pQuerMask[0] = *(uint32*)(&(uint32*)gHWS_GPR[rt_ft]+1);
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(QuerAddr) )
		{
			QuerAddr = TranslateTLBAddressForStore(QuerAddr);
		}
		__try{
			uint32* pQuerMask = pLOAD_UWORD_PARAM(QuerAddr);
			pQuerMask[1] = (uint32)gHWS_GPR[rt_ft];
			pQuerMask[0] = *(uint32*)(&(uint32*)gHWS_GPR[rt_ft]+1);
		}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			__try{
				uint32* pQuerMask = pLOAD_UWORD_PARAM_2(QuerAddr);
				pQuerMask[1] = (uint32)gHWS_GPR[rt_ft];
				pQuerMask[0] = *(uint32*)(&(uint32*)gHWS_GPR[rt_ft]+1);
				TRACE2("%08X: Use SD to store registers at 0x%08x", gHWS_pc, QuerAddr);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DISPLAY_ADDRESS_ERROR( QuerAddr, "SD" );
			}
		}
   }
}

void r4300i_sd(uint32 Instruction)
{
    uint32 QuerAddr;
    uint32 rt_ft = RT_FT;
    
    QUER_ADDR;

    r4300i_sd_faster(QuerAddr, rt_ft);
}


//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

void r4300i_sc(uint32 Instruction)
{   
    uint32 rt_ft = RT_FT;

    if (gHWS_LLbit) 
    {
		uint32 QuerAddr;
		QUER_ADDR;
		ADDR_WORD_ALIGN_CHECKING(QuerAddr)

        __try{
            LOAD_UWORD_PARAM(QuerAddr) = (uint32)gHWS_GPR[rt_ft];
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			if( NOT_IN_KO_K1_SEG(QuerAddr) )
			{
				QuerAddr = TranslateTLBAddressForStore(QuerAddr);
			}
			__try{
				LOAD_UWORD_PARAM(QuerAddr) = (uint32)gHWS_GPR[rt_ft];
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				__try{
					LOAD_UWORD_PARAM_2(QuerAddr) = (uint32)gHWS_GPR[rt_ft];
					TRACE1("Use SC to read registers at 0x%08x", QuerAddr);
				}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
					DISPLAY_ADDRESS_ERROR( QuerAddr, "SC" )
				}
			}
        }
   }

   (uint64)gRT = (uint64)gHWS_LLbit;
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

void r4300i_scd(uint32 Instruction)
{
	uint32 rt_ft = RT_FT;
    if (gHWS_LLbit) 
    {
		uint32 QuerAddr;
		QUER_ADDR;
		ADDR_DWORD_ALIGN_CHECKING(QuerAddr)

        __try{
			uint32* pQuerMask = pLOAD_UWORD_PARAM(QuerAddr);
			pQuerMask[1] = (uint32)gHWS_GPR[rt_ft];
			pQuerMask[0] = *(uint32*)(&(uint32*)gHWS_GPR[rt_ft]+1);
			*(uint32*)(&(uint32*)gHWS_GPR[rt_ft]+1)  = pQuerMask[0];
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			if( NOT_IN_KO_K1_SEG(QuerAddr) )
			{
				QuerAddr = TranslateTLBAddressForStore(QuerAddr);
			}
			__try{
				uint32* pQuerMask = pLOAD_UWORD_PARAM(QuerAddr);
				pQuerMask[1] = (uint32)gHWS_GPR[rt_ft];
				pQuerMask[0] = *(uint32*)(&(uint32*)gHWS_GPR[rt_ft]+1);
				*(uint32*)(&(uint32*)gHWS_GPR[rt_ft]+1)  = pQuerMask[0];
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				__try{
					uint32* pQuerMask = pLOAD_UWORD_PARAM_2(QuerAddr);
					pQuerMask[1] = (uint32)gHWS_GPR[rt_ft];
					pQuerMask[0] = *(uint32*)(&(uint32*)gHWS_GPR[rt_ft]+1);
					*(uint32*)(&(uint32*)gHWS_GPR[rt_ft]+1)  = pQuerMask[0];
					TRACE1("Use SCD to read registers at 0x%08x", QuerAddr);
				}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
					DISPLAY_ADDRESS_ERROR( QuerAddr, "SCD" )
				}
			}
        }
   }

    (uint64)gHWS_GPR[rt_ft] = (uint64)gHWS_LLbit; 
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

void r4300i_mfhi(uint32 Instruction){gRD = gHWS_GPR[gHI];}  
void r4300i_mthi(uint32 Instruction){gHWS_GPR[gHI] = gRS;}
void r4300i_mflo(uint32 Instruction){gRD = gHWS_GPR[gLO];}  
void r4300i_mtlo(uint32 Instruction){gHWS_GPR[gLO] = gRS;}

/*---------------------------------------------------------------------------------------
	-----------------------------------------------------------------
	| SLL       | Shift word Left Logical                           |
	|-----------|---------------------------------------------------|
	|  000000   |  00000  |   rt    |    rd   |   sa    |000000 (0) |
	------6----------5---------5---------5---------5----------6------
	 Format:  SLL rd, rt, sa
	 Purpose: To left shift a word by a fixed number of bits.
	 Comment: SLL r0, r0, 0 is equal to a NOP (No OPeration)
	 Descrip: rd = rt << sa

	-----------------------------------------------------------------
	| SLLV      | Shift word Left Logical Variable                  |
	|-----------|---------------------------------------------------|
	|  000000   |   rs    |   rt    |    rd   |  00000  |000100 (4) |
	------6----------5---------5---------5---------5----------6------
	 Format:  SLLV rd, rt, rs
	 Purpose: To left shift a word by a variable number of bits.
	 Descrip: rd = rt << sa

	-----------------------------------------------------------------
	| DSLL      | Doubleword Shift Left Logical                     |
	|-----------|---------------------------------------------------|
	|  000000   |  00000  |   rt    |    rd   |   sa    |111000 (56)|
	------6----------5---------5---------5---------5----------6------
	 Format:  DSLL rd, rt, sa
	 Purpose: To left shift a doubleword by a fixed amount -- 0 to 31 bits.
	 Descrip: rd = rt << sa

	-----------------------------------------------------------------
	| DSLL32    | Doubleword Shift Left Logical +32                 |
	|-----------|---------------------------------------------------|
	|  000000   |  00000  |   rt    |    rd   |   sa    |111100 (60)|
	------6----------5---------5---------5---------5----------6------
	 Format:  DSLL32 rd, rt, sa
	 Purpose: To left shift a doubleword by a fixed amount -- 32 to 63 bits.
	 Descrip: rd = rt << (sa+32)

	-----------------------------------------------------------------
	| DSLLV     | Doubleword Shift Left Logical Variable            |
	|-----------|---------------------------------------------------|
	|  000000   |   rs    |   rt    |    rd   |  00000  |010100 (20)|
	------6----------5---------5---------5---------5----------6------
	 Format:  DSLLV rd, rt, rs
	 Purpose: To left shift a doubleword by a variable number of bits.
	 Descrip: rd = rt << rs
/*---------------------------------------------------------------------------------------*/
void r4300i_sll(uint32 Instruction)		
{
	if( RD_FS == 0 && RT_FT == 0 ) return;	//NOP
	CHECK_R0_EQUAL_0(RD_FS, "sll");	
	uLOGICAL_SHIFT(<<, SA_FD);		
}   
void r4300i_sllv(uint32 Instruction)	{CHECK_R0_EQUAL_0(RD_FS, "sllv");	uLOGICAL_SHIFT(<<, (gRS&0x1F));	}
void r4300i_dsll(uint32 Instruction)	{CHECK_R0_EQUAL_0(RD_FS, "dsll");	(uint64)gRD = (uint64)gRT << SA_FD; }   
void r4300i_dsllv(uint32 Instruction)	{CHECK_R0_EQUAL_0(RD_FS, "dsllv");	(uint64)gRD = (uint64)gRT << (gRS&0x3F);}
void r4300i_dsll32(uint32 Instruction)
{ 
	CHECK_R0_EQUAL_0(RD_FS, "dsll32");
    *(uint32*)((uint8*)&gRD+4) = (uint32)gRT << SA_FD;
    *(uint32*)((uint8*)&gRD  ) = 0;
}

/*---------------------------------------------------------------------------------------
-----------------------------------------------------------------
| SRL       | Shift word Right Logical                          |
|-----------|---------------------------------------------------|
|  000000   |  00000  |   rt    |    rd   |   sa    |000010 (2) |
------6----------5---------5---------5---------5----------6------
 Format:  SRL rd, rt, sa
 Purpose: To logical right shift a word by a fixed number of bits.
 Descrip: rd = rt >> sa

-----------------------------------------------------------------
| SRLV      | Shift word Right Logical Variable                 |
|-----------|---------------------------------------------------|
|  000000   |   rs    |   rt    |    rd   |  00000  |000110 (6) |
------6----------5---------5---------5---------5----------6------
 Format:  SRLV rd, rt, rs
 Purpose: To logical right shift a word by a variable number of bits.
 Descrip: rd = rt >> rs

-----------------------------------------------------------------
| DSRL      | Doubleword Shift Right Logical                    |
|-----------|---------------------------------------------------|
|  000000   |  00000  |   rt    |    rd   |   sa    |111010 (58)|
------6----------5---------5---------5---------5----------6------
 Format:  DSRL rd, rt, sa
 Purpose: To logical right shift a doubleword by a fixed amount
          -- 0 to 31 bits.
 Descrip: rd = rt >> sa

-----------------------------------------------------------------
| DSRL32    | Doubleword Shift Right Logical +32                |
|-----------|---------------------------------------------------|
|  000000   |  00000  |   rt    |    rd   |   sa    |111110 (62)|
------6----------5---------5---------5---------5----------6------
 Format:  DSRL32 rd, rt, sa
 Purpose: To logical right shift a doubleword by a fixed amount
          -- 32 to 63 bits.
 Descrip: rd = rt >> (sa+32)

-----------------------------------------------------------------
| DSRLV     | Doubleword Shift Right Logical Variable           |
|-----------|---------------------------------------------------|
|  000000   |  00000  |   rt    |    rd   |   rs    |010110 (22)|
------6----------5---------5---------5---------5----------6------
 Format:  DSRLV rd, rt, rs
 Purpose: To logical right shift a doubleword by a variable number of bits.
 Descrip: rd = rt >> rs

/*---------------------------------------------------------------------------------------*/
void r4300i_srl(uint32 Instruction)		{CHECK_R0_EQUAL_0(RD_FS, "srl");	uLOGICAL_SHIFT(>>, SA_FD);		}
void r4300i_srlv(uint32 Instruction)	{CHECK_R0_EQUAL_0(RD_FS, "srlv");	uLOGICAL_SHIFT(>>, (gRS&0x1F)); }
void r4300i_dsrl(uint32 Instruction)	{CHECK_R0_EQUAL_0(RD_FS, "dsrl");	(uint64)gRD = (uint64)gRT >> SA_FD; }
void r4300i_dsrlv(uint32 Instruction)	{CHECK_R0_EQUAL_0(RD_FS, "dsrlv");	(uint64)gRD = (uint64)gRT >> (gRS&0x3F);}
void r4300i_dsrl32(uint32 Instruction)
{
	CHECK_R0_EQUAL_0(RD_FS, "dsrl32");	
    (uint32)gRD = *(uint32*)((uint8*)&gRT+4);
    *(uint32*)((uint8*)&gRD) = *(uint32*)((uint8*)&gRD) >> SA_FD;
    *(uint32*)((uint8*)&gRD+4) = 0;
}

/*---------------------------------------------------------------------------------------
-----------------------------------------------------------------
| SRA       | Shift word Right Arithmetic                       |
|-----------|---------------------------------------------------|
|  000000   |  00000  |   rt    |    rd   |   sa    |000011 (3) |
------6----------5---------5---------5---------5----------6------
 Format:  SRA rd, rt, sa
 Purpose: To arithmetic right shift a word by a fixed number of bits.
 Descrip: rd = rt >> sa

-----------------------------------------------------------------
| SRAV      | Shift word Right Arithmetic Variable              |
|-----------|---------------------------------------------------|
|  000000   |   rs    |   rt    |    rd   |  00000  |000111 (7) |
------6----------5---------5---------5---------5----------6------
 Format:  SRAV rd, rt, rs
 Purpose: To arithmetic right shift a word by a variable number of bits.
 Descrip: rd = rt >> rs

-----------------------------------------------------------------
| DSRA      | Doubleword Shift Right Arithmetic                 |
|-----------|---------------------------------------------------|
|  000000   |  00000  |   rt    |    rd   |   sa    |111011 (59)|
------6----------5---------5---------5---------5----------6------
 Format:  DSRA rd, rt, sa
 Purpose: To arithmetic right shift a doubleword by a fixed
          amount -- 0 to 31 bits.
 Descrip: rd = rt >> sa

-----------------------------------------------------------------
| DSRA32    | Doubleword Shift Right Arithmetic +32             |
|-----------|---------------------------------------------------|
|  000000   |  00000  |   rt    |    rd   |   sa    |111111 (63)|
------6----------5---------5---------5---------5----------6------
 Format:  DSRA32 rd, rt, sa  
 Purpose: To arithmetic right shift a doubleword by a fixed
          amount -- 32-63 bits.
 Descrip: rd = rt >> (sa+32)

-----------------------------------------------------------------
| DSRAV     | Doubleword Shift Right Arithmetic Variable        |
|-----------|---------------------------------------------------|
|  000000   |  00000  |   rt    |    rd   |   rs    |010111 (23)|
------6----------5---------5---------5---------5----------6------
 Format:  DSRAV rd, rt, rs
 Purpose: To arithmetic right shift a doubleword by a variable
          number of bits.
 Descrip: rd = rt >> rs
/*---------------------------------------------------------------------------------------*/
void r4300i_sra(uint32 Instruction)		{CHECK_R0_EQUAL_0(RD_FS, "sra");	sLOGICAL_SHIFT(>>, SA_FD);	}
void r4300i_srav(uint32 Instruction)	{CHECK_R0_EQUAL_0(RD_FS, "srav");	sLOGICAL_SHIFT(>>, (gRS&0x1F));}

//Questionable: dsra & dsrav are probably not getting sign-extended
//---------------------------------------------------------------------------------------
void r4300i_dsrav(uint32 Instruction)	{CHECK_R0_EQUAL_0(RD_FS, "dsrav");	gRD =         gRT >> (gRS&0x3F);}
void r4300i_dsra(uint32 Instruction)	{CHECK_R0_EQUAL_0(RD_FS, "dsra");	gRD =         gRT >> SA_FD; }   
void r4300i_dsra32(uint32 Instruction)
{
	CHECK_R0_EQUAL_0(RD_FS, "dsra32");	
    (uint32)gRD = *(uint32*)((uint8*)&gRT+4);
    sLOGICAL_SHIFT(>>, SA_FD);
}

/*---------------------------------------------------------------------------------------
-----------------------------------------------------------------
| SLT       | Set on Less Than                                  |
|-----------|---------------------------------------------------|
|  000000   |   rs    |   rt    |    rd   |  00000  |101010 (42)|
------6----------5---------5---------5---------5----------6------
 Format:  SLT rd, rs, rt
 Purpose: To record the result of a less-than comparison.
 Descrip: if rs < rt then rd = 1 else rd = 0

-----------------------------------------------------------------
| SLTI      | Set on Less Than Immediate                        |
|-----------|---------------------------------------------------|
|001010 (10)|    rs   |   rt    |            immediate          |
------6----------5---------5-------------------16----------------
 Format:  SLTI rt, rs, immediate
 Purpose: To record the result of a less-than comparison with a constant.
 Descrip: if rs < immediate then rd = 1 else rd = 0

-----------------------------------------------------------------
| SLTIU     | Set on Less Than Immediate Unsigned               |
|-----------|---------------------------------------------------|
|001011 (11)|   rs    |   rt    |            immediate          |
------6----------5---------5-------------------16----------------
 Format:  SLTIU rt, rs, immediate
 Purpose: To record the result of an unsigned less-than
          comparison with a constant.
 Descrip: if rs < immediate then rd = 1 else rd = 0

-----------------------------------------------------------------
| SLTU      | Set on Less Than Unsigned                         |
|-----------|---------------------------------------------------|
|  000000   |   rs    |   rt    |    rd   |  00000  |101011 (43)|
------6----------5---------5---------5---------5----------6------
 Format:  SLTU rd, rs, rt
 Purpose: To record the result of an unsigned less-than comparison.
 Descrip: if rs < rt then rd = 1 else rd = 0
/*---------------------------------------------------------------------------------------*/
void r4300i_slt(uint32 Instruction)		{if ( gRS <gRT )   gRD = 1;    else gRD = 0;}
void r4300i_sltu(uint32 Instruction)	{if ((uint64)gRS < (uint64)gRT )   gRD = 1;    else gRD = 0;}
void r4300i_slti(uint32 Instruction)	{if ( gRS < OFFSET_IMMEDIATE)   gRT = 1;    else gRT = 0;}
void r4300i_sltiu(uint32 Instruction)	
{
	if ((uint64)gRS < (uint64)(_int64)(_int32)(_int16)(uint16)OFFSET_IMMEDIATE)
		gRT = 1;    
	else 
		gRT = 0;
}


void r4300i_COP0_mfc0(uint32 Instruction) 
{
	CHECK_R0_EQUAL_0(RT_FT, "mfc0");
	switch (RD_FS)
	{
	case RANDOM:
#ifdef FAST_COUNTER
		gHWS_COP0Reg[RANDOM] = Get_COUNT_Register() % (0x40-(gHWS_COP0Reg[WIRED]&0x3f))+gHWS_COP0Reg[WIRED];
#else
		gHWS_COP0Reg[RANDOM] = gHWS_COP0Reg[COUNT] % (0x40-(gHWS_COP0Reg[WIRED]&0x3f))+gHWS_COP0Reg[WIRED];
#endif
		break;
	case COUNT:
#ifdef FAST_COUNTER
		gHWS_COP0Reg[COUNT] = Get_COUNT_Register();
#endif
#ifdef DEBUG_COMMON
		DEBUG_CPU_COUNTER_TRACE(TRACE3( "COUNT register is read = %8X, COMPARE=%8X , PC=0x%08X", c0FS, gHWS_COP0Reg[COMPARE], gHWS_pc););
#endif
		break;

	}
	
	gRT = (_int64)(_int32)c0FS;
}

void Convert32fpuTo64fpu()
{
    int k;
	uint32 val;
    //for (k=31; k>=0; k--) gHWS_fpr32[k<<1] = gHWS_fpr32[k];
	for( k=0; k<31; k+=2 )
	{
		val = gHWS_fpr32[k*2+1];
		gHWS_fpr32[k*2+1] = gHWS_fpr32[(k+1)*2];
		gHWS_fpr32[(k+1)*2] = val;
	}
}

void Convert64fpuTo32fpu()
{
    int k;
	uint32 val;
    //for (k=0; k<32; k++) gHWS_fpr32[k] = gHWS_fpr32[k<<1];
	for( k=0; k<31; k+=2 )
	{
		val = gHWS_fpr32[k*2+1];
		gHWS_fpr32[k*2+1] = gHWS_fpr32[(k+1)*2];
		gHWS_fpr32[(k+1)*2] = val;
	}
}

BOOL FPU_Is_Enabled = FALSE;
void Init_Count_Down_Counters();
void r4300i_COP0_mtc0(uint32 Instruction) 
{
    switch( RD_FS )
	{
	case INDEX:			// The INDEX Register
		//gHWS_COP0Reg[INDEX] = (uint32)gRT & 0x1F;
		//gHWS_COP0Reg[INDEX] = (uint32)gRT & 0x3F;
		gHWS_COP0Reg[INDEX] = gHWS_COP0Reg[INDEX] & 0x80000000 | ((uint32)gRT & 0x3F);
		return;
	case RANDOM:			// The RANDOM register, read only
		return;
	case CAUSE: 	// The CAUSE register
		gHWS_COP0Reg[CAUSE] = ((uint32)gRT & 0x300) | (gHWS_COP0Reg[CAUSE] & ~(0x300));
		return;
	case ENTRYLO0:
	case ENTRYLO1:
		c0FS = (uint32)gRT & 0x3FFFFFFF;
		return;
	case PAGEMASK:		// The PAGEMASK register
		gHWS_COP0Reg[PAGEMASK] = (uint32)gRT & 0x01ffe000;
		return;
	case WIRED:			// The WIRED register
		gHWS_COP0Reg[WIRED] = (uint32)gRT & 0x3f;
		//gHWS_COP0Reg[WIRED] = (uint32)gRT & 0x1f;
		gHWS_COP0Reg[RANDOM] = 0x1f;
		return;
	case BADVADDR:			// BADVADDR register, Read only registers
		return;
		/*
	case EPC:
		gHWS_COP0Reg[EPC] = (uint32)gRT;
		TRACE2( "PC=%08X, EPC register is written = %8X", gHWS_pc, (uint32)gRT);
		return;
		*/

	case COUNT:
		DEBUG_COMPARE_INTERRUPT_TRACE(TRACE2( "COUNT register is written = %8X, COMPARE=%8X ", (uint32)gRT, gHWS_COP0Reg[COMPARE]););
		gHWS_COP0Reg[COUNT] = (uint32)gRT;	//COUNT register is writable
		// This is risky
		TRACE1("Set COUNT register = %08X", gHWS_COP0Reg[COUNT]);
#ifdef FAST_COUNTER
		Init_Count_Down_Counters();
#endif
		break;

	case ENTRYHI:		// The EntryHi register
		gHWS_COP0Reg[ENTRYHI] = (uint32)gRT & 0xFFFFE0FF;
		return;
	case COMPARE:		// The Compare register
		gHWS_COP0Reg[CAUSE] &= 0xffff7fff;	// Clear current COMPARE interrupt
		DEBUG_COMPARE_INTERRUPT_TRACE(TRACE2( "COMPARE register is rewritten = %8X, COUNT=%8X ", (uint32)gRT, gHWS_COP0Reg[COUNT]););
		gHWS_COP0Reg[COMPARE]  = (uint32)gRT;
#ifdef FAST_COUNTER
		Set_COMPARE_Interrupt_Target_Counter();
#endif
		return;
	case STATUS:		// The status register
		//TRACE2( "Write STATUS register: PC = %08X, new value=%08X", gHWS_pc, (uint32)gRT);

#ifdef ENABLE_64BIT_FPU
        // Has FR bit changed ?
        if( (gHWS_COP0Reg[STATUS]& 0x04000000) != (gRT & 0x04000000))
        {
			FR_reg_offset = (gRT & 0x04000000)?32:1;
        }
#endif

		// Test the exception bit
		if( (gRT & EXL) == 0 && (gHWS_COP0Reg[STATUS] & EXL == 1))
		{
			TRACE3( "EXL Bit is cleared at PC = %8X, COMPARE=%8X , PC=0x%08X", gRT, gHWS_COP0Reg[COMPARE], gHWS_pc);

			// CPU will check interrupts at the next cycle
			if( (gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) )
			{
				CPUNeedToCheckInterrupt = TRUE;
				CPUNeedToDoOtherTask = TRUE;
			}
		}

		// Test the IE bit
		if( (gRT & IE == 1) &&  (gHWS_COP0Reg[STATUS] & IE == 0) )		// If enable interrupt
		{
			// CPU will check interrupts at the next cycle
			TRACE3( "Interrupt is enabled at PC = %8X, COMPARE=%8X , PC=0x%08X", gRT, gHWS_COP0Reg[COMPARE], gHWS_pc);
			if( (gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) )
			{
				CPUNeedToCheckInterrupt = TRUE;
				CPUNeedToDoOtherTask = TRUE;
			}
		}

		// Check FPU usage bit
		if( currentromoptions.FPU_Hack == USEFPUHACK_YES && (gRT & SR_CU1) != (gHWS_COP0Reg[STATUS] & SR_CU1)  )
		{
			if( gRT & SR_CU1 )
			{
				dyna_instruction[0x11] = dyna4300i_cop1;					//this is for dyna
				CPU_instruction[0x11] = COP1_instr;							//this is for interpreter
			}
			else
			{
				dyna_instruction[0x11] = dyna4300i_cop1_with_exception;		//this is for dyna
				CPU_instruction[0x11] = COP1_NotAvailable_instr;			//this is for interpreter
			}
		}

		gHWS_COP0Reg[STATUS] = (uint32)gRT;
		return;
	case PREVID:		// PRID register, Read only registers
		return;		// This makes BomberMan Hero not to freeze after [START]
	}
	
	c0FS = (uint32)gRT;
}

//---------------------------------------------------------------------------------------
//	-----------------------------------------------------------------
//	| LUI       | Load Upper Immediate                              |
//	|-----------|---------------------------------------------------|
//	|001111 (15)|  00000  |   rt    |           immediate           |
//	------6----------5---------5-------------------16----------------
//	 Format:  LUI rt, immediate
//	 Purpose: To load a constant into the upper half of a word.
//	 Descrip: rt = immediate * 10000h
//---------------------------------------------------------------------------------------

void r4300i_lui(uint32 Instruction)     
{
	CHECK_R0_EQUAL_0(RT_FT, "lui");
    gRT = OFFSET_IMMEDIATE << (uint32)16;
}

//---------------------------------------------------------------------------------------
void r4300i_speedhack()
{
	// Before SPEEDHACK, let CPU to finish all other tasks, let DMA, SP Task and so on
	if( CPUNeedToDoOtherTask )
		return;

#ifdef FAST_COUNTER
		Count_Down_All();
#else
	if( (gHWS_COP0Reg[COUNT] < gHWS_COP0Reg[COMPARE] &&
			gHWS_COP0Reg[COUNT] + MAGICNUMBERFORCOUNTREG >= gHWS_COP0Reg[COMPARE] ) ||
		( gHWS_COP0Reg[COUNT] > gHWS_COP0Reg[COMPARE] && 
		(uint32)(gHWS_COP0Reg[COUNT] + MAGICNUMBERFORCOUNTREG) <= gHWS_COP0Reg[COMPARE] ))
	{
		Trigger_CompareInterrupt();
		VIcounter += (gHWS_COP0Reg[COMPARE] - gHWS_COP0Reg[COUNT] )*VICounterFactors[CounterFactor]/CounterFactors[CounterFactor];
		gHWS_COP0Reg[COUNT] = gHWS_COP0Reg[COMPARE];
	}
	else
	{
		gHWS_COP0Reg[COUNT] += (MAGICNUMBER - VIcounter )*CounterFactors[CounterFactor]/VICounterFactors[CounterFactor];
		VIcounter = MAGICNUMBER;
	}
#endif
}

void r4300i_bgez(uint32 Instruction)    
{                       
	if (gRS >= 0)   
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
	}
}
void r4300i_bgezal(uint32 Instruction)  
{
	INTERPRETIVE_LINK(RA)  
	if (gRS >= 0)   
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
	}
}

void r4300i_bgezall(uint32 Instruction) 
{
	INTERPRETIVE_LINK(RA)  
	if (gRS >= 0)   
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else 
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
		DELAY_SKIP 
	}
}
void r4300i_bgezl(uint32 Instruction)   
{                       
	if (gRS >= 0)   
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
		DELAY_SKIP 
	}
}

void r4300i_bltz(uint32 Instruction)    
{                       
	if (gRS <  0)   
	{
		DELAY_SET   
	}
	else
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
	}
}
void r4300i_bltzal(uint32 Instruction)  
{
	INTERPRETIVE_LINK(RA)  
	if (gRS <  0)   
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
	}
}
void r4300i_bltzall(uint32 Instruction) 
{
	INTERPRETIVE_LINK(RA)  
	if (gRS <  0)   
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
		DELAY_SKIP 
	}
}

void r4300i_bltzl(uint32 Instruction)   
{                       
	if (gRS <  0)   
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else 
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
		DELAY_SKIP 
	}
}

void r4300i_bgtz(uint32 Instruction)    
{
	if (gRS >  0)
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
	}
}

void r4300i_bgtzl(uint32 Instruction)   
{
	if (gRS >  0)   
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
		DELAY_SKIP 
	}
}

void r4300i_blez(uint32 Instruction)    
{
	if (gRS <= 0)   
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
	}
}

void r4300i_blezl(uint32 Instruction)   
{
	if (gRS <= 0)   
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else 
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
		DELAY_SKIP 
	}
}

void r4300i_bne(uint32 Instruction)     
{
	if ((uint64)gRS != (uint64)gRT)
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else
		OPCODE_DEBUGGER_BRANCH_SKIP
}

void r4300i_bnel(uint32 Instruction)
{
	if ((uint64)gRS != (uint64)gRT)  
	{
		R4300I_SPEEDHACK
		DELAY_SET
	}
	else
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
		DELAY_SKIP 
	}
}

void r4300i_beql(uint32 Instruction)    
{
	if ((uint64)gRS == (uint64)gRT)    
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else 
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
		DELAY_SKIP 
	}
}

void r4300i_beq(uint32 Instruction)     
{
    if ((uint64)gRS == (uint64)gRT)
    {
		R4300I_SPEEDHACK
		DELAY_SET   
    }
	else
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
	}
}

//---------------------------------------------------------------------------------------

void r4300i_div(uint32 Instruction) {

    if ((uint32)gRT != 0) //Divide by zero in rom!!! (really should generate exception)
    {   
        sLOGIC(gHWS_GPR[gLO], gRS, /, gRT);
        sLOGIC(gHWS_GPR[gHI], gRS, %, gRT);
    }

	SAVE_OP_COUNTER_INCREASE_INTERPRETER(PCLOCKDIV);
}

//---------------------------------------------------------------------------------------

void r4300i_divu(uint32 Instruction) {

    uint32 RSReg = (uint32)gRS;
    uint32 RTReg = (uint32)gRT;

    if ((uint32)RTReg != 0) //Divide by zero in rom!!!
    {
        uLOGIC(gHWS_GPR[gLO], RSReg, /, RTReg);
        uLOGIC(gHWS_GPR[gHI], RSReg, %, RTReg);
    }

	SAVE_OP_COUNTER_INCREASE_INTERPRETER(PCLOCKDIVU);
}

//---------------------------------------------------------------------------------------

void r4300i_ddiv(uint32 Instruction) {
    _int64 TempRS;
    _int64 RTReg = (_int64)gRT;

    if ((uint32)RTReg != 0) //Divide by zero in rom!!!
    {   
        gHWS_GPR[gLO] = (_int64)(TempRS=(uint64)gRS)    / RTReg;
        gHWS_GPR[gHI] = TempRS                          % RTReg;
    }

	SAVE_OP_COUNTER_INCREASE_INTERPRETER(PCLOCKDDIV);
}

//---------------------------------------------------------------------------------------

void r4300i_ddivu(uint32 Instruction) {
    _int64 TempRS;
    uint32 RTReg = (uint32)gRT;

    if ((uint32)RTReg != 0) //Divide by zero in rom!!!
    {   
        gHWS_GPR[gLO] = ((uint32)(TempRS=(uint64)gRS)) / RTReg;
        gHWS_GPR[gHI] =  (uint32)TempRS                % RTReg;
    }

	SAVE_OP_COUNTER_INCREASE_INTERPRETER(PCLOCKDDIVU);
}

//---------------------------------------------------------------------------------------

void r4300i_mult(uint32 Instruction) {
    _int64 result;

    result = (_int64)(_int32)gRS * (_int64)(_int32)gRT;
    gHWS_GPR[gLO] = (_int32)result;
    gHWS_GPR[gHI] = (_int32)(result >> 32);

	SAVE_OP_COUNTER_INCREASE_INTERPRETER(PCLOCKMULT);
}

//---------------------------------------------------------------------------------------

void r4300i_multu(uint32 Instruction) {
    uint64 result;

    result = (uint64)(uint32)gRS * (uint64)(uint32)gRT;
    gHWS_GPR[gLO] = (_int32)result;
    gHWS_GPR[gHI] = (_int32)(result >> 32);

	SAVE_OP_COUNTER_INCREASE_INTERPRETER(PCLOCKMULTU);
}

//---------------------------------------------------------------------------------------

void r4300i_dmultu(uint32 Instruction) {
     unsigned __int64 hh,hl,lh,ll,b;
     __int64 t1,t2;

     t1 = gRS;
     t2 = gRT;

     hh = ((__int64)(t1 >> 32)  & 0x0ffffffff)  * ((__int64)(t2 >> 32)  &   0x0ffffffff);
     hl =  (__int64)(t1         & 0x0ffffffff)  * ((__int64)(t2 >> 32)  &   0x0ffffffff);
     lh = ((__int64)(t1 >> 32)  & 0x0ffffffff)  *  (__int64)(t2         &   0x0ffffffff);
     ll = ((__int64)(t1         & 0x0ffffffff)  *  (__int64)(t2         &   0x0ffffffff));

     gHWS_GPR[gLO] = ((hl + lh) << 32) + ll;

     b=(((hl + lh) + (ll >> 32)) & 0x0100000000)>>32;

     gHWS_GPR[gHI] = (unsigned __int64)hh + ((signed __int64)(unsigned __int32)(hl >> 32) + (signed __int64)(unsigned __int32)(lh >> 32) + b);

	SAVE_OP_COUNTER_INCREASE_INTERPRETER(PCLOCKDMULTU);
}

//---------------------------------------------------------------------------------------

void r4300i_dmult(uint32 Instruction) {
     unsigned __int64 hh,hl,lh,ll,b;
     __int64 t1,t2;
     int sgn = 0;

     t1 = gRS;
     t2 = gRT;
     if (t1 < 0) {sgn ^= 1; t1 = -t1;}
     if (t2 < 0) {sgn ^= 1; t2 = -t2;}

     hh = ((__int64)(t1 >> 32) & 0x0ffffffff)   * ((__int64)(t2 >> 32)  &   0x0ffffffff);
     hl = (__int64)(t1 & 0x0ffffffff)           * ((__int64)(t2 >> 32)  &   0x0ffffffff);
     lh = ((__int64)(t1 >> 32) & 0x0ffffffff)   *  (__int64)(t2         &   0x0ffffffff);
     ll = ((__int64)(t1 & 0x0ffffffff)          *  (__int64)(t2         &   0x0ffffffff));

     gHWS_GPR[gLO] = ((hl + lh) << 32) + ll;

     b=(((hl + lh) + (ll >> 32)) & 0x0100000000)>>32;

     gHWS_GPR[gHI] = (unsigned __int64)hh + ((signed __int64)(unsigned __int32)(hl  >> 32) + (signed __int64)(unsigned __int32)(lh >> 32) + b);

     b = (gHWS_GPR[gLO] >= 0) ? 1 : 0;

     if (sgn != 0)
     {
          gHWS_GPR[gLO] = -gHWS_GPR[gLO];
          gHWS_GPR[gHI] = -gHWS_GPR[gHI] + b;
     }

	SAVE_OP_COUNTER_INCREASE_INTERPRETER(PCLOCKDMULT);
}

//---------------------------------------------------------------------------------------

void r4300i_jal(uint32 Instruction) 
{   
	INTERPRETIVE_LINK(RA)       
	CPUdelay = 1;   
	CPUdelayPC = INSTR_INDEX;   
}

void r4300i_jalr(uint32 Instruction)
{   
	INTERPRETIVE_LINK(RD_FS)    
	CPUdelay = 1;   
	CPUdelayPC = (uint32)gRS;   
}

void r4300i_jr(uint32 Instruction)  
{                               
	CPUdelay = 1;   
	CPUdelayPC = (uint32)gRS;   
}

void r4300i_j(uint32 Instruction)   
{
	CPUdelay = 1;   
	CPUdelayPC = INSTR_INDEX;
	R4300I_J_SPEEDHACK
}

//---------------------------------------------------------------------------------------


_int32 r4300i_lw_faster(uint32 QuerAddr)
{
//	CHECK_R0_EQUAL_0(rt_ft)

	ADDR_WORD_ALIGN_CHECKING(QuerAddr)

   __try{
        return(LOAD_SWORD_PARAM(QuerAddr));
   }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(QuerAddr) )
		{
			QuerAddr = TranslateTLBAddressForLoad(QuerAddr);
			__try{
                return(LOAD_SWORD_PARAM(QuerAddr));
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                return(Check_LW(QuerAddr));
			}
		}
		else
		{
            return(Check_LW(QuerAddr));
		}
//      DisplayError("Interpretive Exception");
   }
}

_int32 r4300i_lw_faster2(uint32 QuerAddr)
{
//	CHECK_R0_EQUAL_0(rt_ft)

	ADDR_WORD_ALIGN_CHECKING(QuerAddr)

	if( QuerAddr >= 0x80000000 && QuerAddr <= 0x9FFFFFFF )
	{
		uint32 addr = QuerAddr&0x1FFFFFFF;
		if( addr < current_rdram_size )
			return *(uint32*)(gMS_RDRAM+addr);
	}

	return r4300i_lw_faster(QuerAddr);
}


void r4300i_lw(uint32 Instruction)
{
    uint32 QuerAddr;
    uint32 rt_ft = RT_FT;
    
    QUER_ADDR;

    if (rt_ft == 0)
        r4300i_lw_faster(QuerAddr);
    else
        gHWS_GPR[rt_ft] = r4300i_lw_faster(QuerAddr);
}



//---------------------------------------------------------------------------------------
void r4300i_sw_faster(uint32 QuerAddr, uint32 rt_ft) 
{   
//	uint32 rt_ft = RT_FT;
//    uint32 QuerAddr;

//    QUER_ADDR;
	ADDR_WORD_ALIGN_CHECKING(QuerAddr)

	__try{
		LOAD_UWORD_PARAM(QuerAddr) = (uint32)gHWS_GPR[rt_ft];
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(QuerAddr) )
		{
			QuerAddr = TranslateTLBAddressForStore(QuerAddr);
		   __try{
			  LOAD_UWORD_PARAM(QuerAddr) = (uint32)gHWS_GPR[rt_ft];
		   }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				Check_SW(QuerAddr, rt_ft);
				//Faster_Check_SW(QuerAddr, rt_ft);
		   }
		}
		else
		{
			Check_SW(QuerAddr, rt_ft);
			//Faster_Check_SW(QuerAddr, rt_ft);
		}
	}

#ifdef CPU_CORE_CHECK_R0
//	if (rt_ft == 0) gHWS_GPR[0] = 0; //needed only in lw, sw. Don't need.
#endif

}

void r4300i_sw(uint32 Instruction)
{
    uint32 QuerAddr;
    uint32 rt_ft = RT_FT;
    
    QUER_ADDR;

    r4300i_sw_faster(QuerAddr, rt_ft);
}


//---------------------------------------------------------------------------------------

void r4300i_lwl(uint32 Instruction)
{
    uint32 LoadWord1 = 0;
	uint32 LoadWord2 = 0;
    uint32 rt_ft = RT_FT;
    uint32 vAddr;
    uint32 vAddr2;
    uint32 QuerAddr;
	CHECK_R0_EQUAL_0(RT_FT, "lwl");

    QUER_ADDR;

	vAddr = QuerAddr & 0xfffffffc;

    __try{
        LoadWord1 = LOAD_UWORD_PARAM(vAddr);
        vAddr2 = vAddr+4;
        LoadWord2 = LOAD_UWORD_PARAM(vAddr2);
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(QuerAddr) )
		{
			QuerAddr = TranslateTLBAddressForLoad(QuerAddr);
			__try{
				vAddr = QuerAddr & 0xfffffffc;
				LoadWord1 = LOAD_UWORD_PARAM(vAddr);
				vAddr2 = vAddr+4;
				LoadWord2 = LOAD_UWORD_PARAM(vAddr2);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				__try{
					LoadWord1 = LOAD_UWORD_PARAM_2(vAddr);
					vAddr2 = vAddr+4;
					LoadWord2 = LOAD_UWORD_PARAM_2(vAddr2);
					TRACE1("Use LWL to read registers at 0x%08x", QuerAddr);
				}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
					DISPLAY_ADDRESS_ERROR( QuerAddr, "LWL" )
				}
			}
		}
		else
		{
			__try{
				LoadWord1 = LOAD_UWORD_PARAM_2(vAddr);
				vAddr2 = vAddr+4;
				LoadWord2 = LOAD_UWORD_PARAM_2(vAddr2);
				TRACE1("Use LWL to read registers at 0x%08x", QuerAddr);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DISPLAY_ADDRESS_ERROR( QuerAddr, "LWL" )
			}
		}
	}

    switch (QuerAddr & 3) 
    {
        case 0: gRT = (_int64)(_int32)LoadWord1;                               break;
        case 1: gRT = (_int64)(_int32)((gRT & 0x000000ff) | (LoadWord1 << 8)); break;
        case 2: gRT = (_int64)(_int32)((gRT & 0x0000ffff) | (LoadWord1 << 16));break;
        case 3: gRT = (_int64)(_int32)((gRT & 0x00ffffff) | (LoadWord1 << 24));break;
    }
}

//---------------------------------------------------------------------------------------

void r4300i_lwr(uint32 Instruction) 
{
    uint32 LoadWord1 = 0;
	uint32 LoadWord2 = 0;
    uint32 rt_ft = RT_FT;
    uint32 vAddr, vAddr2;
    uint32 QuerAddr;

	CHECK_R0_EQUAL_0(RT_FT, "lwr");
    QUER_ADDR;

	vAddr = QuerAddr & 0xfffffffc;

    __try{
        LoadWord1 = LOAD_UWORD_PARAM(vAddr);
        vAddr2 = vAddr+4;
        LoadWord2 = LOAD_UWORD_PARAM(vAddr2);
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(QuerAddr) )
		{
			QuerAddr = TranslateTLBAddressForStore(QuerAddr);
			__try{
				vAddr = QuerAddr & 0xfffffffc;
				LoadWord1 = LOAD_UWORD_PARAM(vAddr);
				vAddr2 = vAddr+4;
				LoadWord2 = LOAD_UWORD_PARAM(vAddr2);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				__try{
					LoadWord1 = LOAD_UWORD_PARAM_2(vAddr);
					vAddr2 = vAddr+4;
					LoadWord2 = LOAD_UWORD_PARAM_2(vAddr2);
					TRACE1("Use LWR to read registers at 0x%08x", QuerAddr);
				}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
					DISPLAY_ADDRESS_ERROR( QuerAddr, "LWR" )
				}
			}
		}
		else
		{
			__try{
				LoadWord1 = LOAD_UWORD_PARAM_2(vAddr);
				vAddr2 = vAddr+4;
				LoadWord2 = LOAD_UWORD_PARAM_2(vAddr2);
				TRACE1("Use LWR to read registers at 0x%08x", QuerAddr);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DISPLAY_ADDRESS_ERROR( QuerAddr, "LWR" )
			}
		}
	}

    switch (QuerAddr & 3) 
    {
        case 3: gRT = (_int64)(_int32)(gRT & 0x00000000) |  LoadWord1;           break;
        case 2: gRT = (_int64)(_int32)(gRT & 0xff000000) | (LoadWord1 >> 8);     break;
        case 1: gRT = (_int64)(_int32)(gRT & 0xffff0000) | (LoadWord1 >> 16);    break;
        case 0: gRT = (_int64)(_int32)(gRT & 0xffffff00) | (LoadWord1 >> 24);    break;
    }
}

//---------------------------------------------------------------------------------------

void r4300i_swl(uint32 Instruction)
{
    uint32 LoadWord1 = 0;
	uint32 LoadWord2 = 0;
    uint32 rt_ft = RT_FT;
    uint32 vAddr, vAddr2;

	STORE_TLB_FUN
    vAddr = QuerAddr & 0xfffffffc;
    __try{
        LoadWord1 = LOAD_UWORD_PARAM(vAddr);
        vAddr2 = vAddr+4;
        LoadWord2 = LOAD_UWORD_PARAM(vAddr2);
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            LoadWord1 = LOAD_UWORD_PARAM_2(vAddr);
            vAddr2 = vAddr+4;
            LoadWord2 = LOAD_UWORD_PARAM_2(vAddr2);
#ifdef DEBUG_IO_WRITE
			DisplayError("Use SWL to read registers at 0x%08x", QuerAddr);
#endif
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			DISPLAY_ADDRESS_ERROR( QuerAddr, "SWL" )
        }
    }

    __try{
        switch (QuerAddr & 3)
        {
            case 0: LOAD_UWORD_PARAM(vAddr) = (uint32)gRT;                                              break;
            case 1: LOAD_UWORD_PARAM(vAddr) = (uint32)(LoadWord1 & 0xff000000) | ((uint32)gRT >> 8);    break;
            case 2: LOAD_UWORD_PARAM(vAddr) = (uint32)(LoadWord1 & 0xffff0000) | ((uint32)gRT >> 16);   break;
            case 3: LOAD_UWORD_PARAM(vAddr) = (uint32)(LoadWord1 & 0xffffff00) | ((uint32)gRT >> 24);   break;
	    }
    }
    __except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            switch (QuerAddr & 3)
            {
                case 0: LOAD_UWORD_PARAM_2(vAddr) = (uint32)gRT;                                              break;
                case 1: LOAD_UWORD_PARAM_2(vAddr) = (uint32)(LoadWord1 & 0xff000000) | ((uint32)gRT >> 8);    break;
                case 2: LOAD_UWORD_PARAM_2(vAddr) = (uint32)(LoadWord1 & 0xffff0000) | ((uint32)gRT >> 16);   break;
                case 3: LOAD_UWORD_PARAM_2(vAddr) = (uint32)(LoadWord1 & 0xffffff00) | ((uint32)gRT >> 24);   break;
	        }
        }
        __except(NULL,EXCEPTION_EXECUTE_HANDLER){
        
        }
    }
}

//---------------------------------------------------------------------------------------

void r4300i_swr(uint32 Instruction) 
{
    uint32 LoadWord1 = 0;
    uint32 vAddr;

	STORE_TLB_FUN
    vAddr = QuerAddr & 0xfffffffc;
    __try{
        LoadWord1 = LOAD_UWORD_PARAM(vAddr);
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            LoadWord1 = LOAD_UWORD_PARAM_2(vAddr);
#ifdef DEBUG_IO_WRITE
			DisplayError("Use SWR to read registers at 0x%08x", QuerAddr);
#endif
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			DISPLAY_ADDRESS_ERROR( QuerAddr, "SWR" )
        }
    }

    __try{
        switch (QuerAddr & 3)
        {
        case 3: LOAD_UWORD_PARAM(vAddr) = (uint32)gRT;                                              break;
        case 2: LOAD_UWORD_PARAM(vAddr) = (uint32)((LoadWord1 & 0x000000FF) | ((uint32)gRT << 8));  break;
        case 1: LOAD_UWORD_PARAM(vAddr) = (uint32)((LoadWord1 & 0x0000FFFF) | ((uint32)gRT << 16)); break;
        case 0: LOAD_UWORD_PARAM(vAddr) = (uint32)((LoadWord1 & 0x00FFFFFF) | ((uint32)gRT << 24)); break;
        }
    }
    __except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            switch (QuerAddr & 3)
            {
            case 3: LOAD_UWORD_PARAM_2(vAddr) = (uint32)gRT;                                              break;
            case 2: LOAD_UWORD_PARAM_2(vAddr) = (uint32)((LoadWord1 & 0x000000FF) | ((uint32)gRT << 8));  break;
            case 1: LOAD_UWORD_PARAM_2(vAddr) = (uint32)((LoadWord1 & 0x0000FFFF) | ((uint32)gRT << 16)); break;
            case 0: LOAD_UWORD_PARAM_2(vAddr)= (uint32)((LoadWord1 & 0x00FFFFFF) | ((uint32)gRT << 24)); break;
	        }
        }
        __except(NULL,EXCEPTION_EXECUTE_HANDLER){
        
        }
    }
}

/*---------------------------------------------------------------------------------------
-----------------------------------------------------------------
| LDL       | Load Doubleword Left                              |
|-----------|---------------------------------------------------|
|011010 (26)|  base   |   rt    |             offset            |
------6----------5---------5-------------------16----------------
 Format:  LDL rt, offset(base)
 Purpose: To load the most-significant part of a doubleword from
          an unaligned memory address.
 Descrip: left(rt) = right[base+offset]
/*---------------------------------------------------------------------------------------*/
void r4300i_ldl(uint32 Instruction) 
{   
	uint64 tempdword;
    uint32 rt_ft = RT_FT;

    LOAD_TLB_FUN
	CHECK_R0_EQUAL_0(RT_FT, "ldl");

	QuerAddr &= ~0x7;

    __try{
        tempdword = LOAD_DOUBLE_PARAM(QuerAddr);
    }
    __except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            tempdword = LOAD_DOUBLE_PARAM_2(QuerAddr);
        }
        __except(NULL,EXCEPTION_EXECUTE_HANDLER){
            return;
        }
    }

	switch (QuerAddr % 8)
	{
    case 0: gRT = tempdword; break;
    case 1: gRT = ((gRT & 0x00000000000000FF) | (tempdword << 8));  break;
    case 2: gRT = ((gRT & 0x000000000000FFFF) | (tempdword << 16)); break;
    case 3: gRT = ((gRT & 0x0000000000FFFFFF) | (tempdword << 24)); break;
    case 4: gRT = ((gRT & 0x00000000FFFFFFFF) | (tempdword << 32)); break;
    case 5: gRT = ((gRT & 0x000000FFFFFFFFFF) | (tempdword << 40)); break;
    case 6: gRT = ((gRT & 0x0000FFFFFFFFFFFF) | (tempdword << 48)); break;
    case 7: gRT = ((gRT & 0x00FFFFFFFFFFFFFF) | (tempdword << 56)); break;
   }
}

/*---------------------------------------------------------------------------------------
-----------------------------------------------------------------
| LDR       | Load Doubleword Right                             |
|-----------|---------------------------------------------------|
|011011 (27)|  base   |   rt    |             offset            |
------6----------5---------5-------------------16----------------
 Format:  LDR rt, offset(base)
 Purpose: To load the least-significant part of a doubleword from
          an unaligned memory address.
 Descrip: right(rt) = left[base+offset]
/*---------------------------------------------------------------------------------------*/
void r4300i_ldr(uint32 Instruction) 
{   
	uint64 tempdword;
    uint32 rt_ft = RT_FT;

    LOAD_TLB_FUN
	CHECK_R0_EQUAL_0(RT_FT, "ldr");

	QuerAddr &= ~0x7;

    __try{
        tempdword = LOAD_DOUBLE_PARAM(QuerAddr);
    }
    __except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            tempdword = LOAD_DOUBLE_PARAM_2(QuerAddr);
        }
        __except(NULL,EXCEPTION_EXECUTE_HANDLER){
            return;
        }
    }

	switch (QuerAddr % 8)
	{
	case 0: gRT = (gRT & 0xFFFFFFFFFFFFFF00) | (tempdword >> 56); break;
    case 1: gRT = (gRT & 0xFFFFFFFFFFFF0000) | (tempdword >> 48); break;
    case 2: gRT = (gRT & 0xFFFFFFFFFF000000) | (tempdword >> 40); break;
    case 3: gRT = (gRT & 0xFFFFFFFF00000000) | (tempdword >> 32); break;
    case 4: gRT = (gRT & 0xFFFFFF0000000000) | (tempdword >> 24); break;
    case 5: gRT = (gRT & 0xFFFF000000000000) | (tempdword >> 16); break;
    case 6: gRT = (gRT & 0xFF00000000000000) | (tempdword >>  8); break;
    case 7: gRT = tempdword; break;
   }
}

/*---------------------------------------------------------------------------------------
-----------------------------------------------------------------
| SDL       | Store Doubleword Left                             |
|-----------|---------------------------------------------------|
|101100 (44)|  base   |   rt    |             offset            |
------6----------5---------5-------------------16----------------
 Format:  SDL rt, offset(base)
 Purpose: To store the most-significant part of a doubleword to
          an unaligned memory address.
 Descrip: right[base+offset] = left(rt)
/*---------------------------------------------------------------------------------------*/
void r4300i_sdl(uint32 Instruction) 
{   
	uint64 * ptempdword;
    uint32 rt_ft = RT_FT;

    STORE_TLB_FUN

	QuerAddr &= ~0x7;

    __try{
        ptempdword = pLOAD_DOUBLE_PARAM(QuerAddr);
    }
    __except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            ptempdword = pLOAD_DOUBLE_PARAM_2(QuerAddr);
        }
        __except(NULL,EXCEPTION_EXECUTE_HANDLER){
            return;
        }
    }

	switch (QuerAddr % 8)
	{
	case 0:	*ptempdword = gRT;												break;	
	case 1:	*ptempdword = (*ptempdword & 0xFF00000000000000) | (gRT >> 8);	break;
	case 2:	*ptempdword = (*ptempdword & 0xFFFF000000000000) | (gRT >> 16);	break;
	case 3:	*ptempdword = (*ptempdword & 0xFFFFFF0000000000) | (gRT >> 24);	break;
	case 4:	*ptempdword = (*ptempdword & 0xFFFFFFFF00000000) | (gRT >> 32);	break;
	case 5:	*ptempdword = (*ptempdword & 0xFFFFFFFFFF000000) | (gRT >> 40);	break;
	case 6:	*ptempdword = (*ptempdword & 0xFFFFFFFFFFFF0000) | (gRT >> 48);	break;
	case 7: *ptempdword = (*ptempdword & 0xFFFFFFFFFFFFFF00) | (gRT >> 56);	break;
	}
}

/*---------------------------------------------------------------------------------------
-----------------------------------------------------------------
| SDR       | Store Doubleword Right                            |
|-----------|---------------------------------------------------|
|101101 (45)|  base   |   rt    |             offset            |
------6----------5---------5-------------------16----------------
 Format:  SDR rt, offset(base)
 Purpose: To store the least-significant part of a doubleword to
          an unaligned memory address.
 Descrip: left[base+offset] = right(rt)
/*---------------------------------------------------------------------------------------*/

void r4300i_sdr(uint32 Instruction) 
{   
	uint64 * ptempdword;
    uint32 rt_ft = RT_FT;

    STORE_TLB_FUN

	QuerAddr &= ~0x7;

    __try{
        ptempdword = pLOAD_DOUBLE_PARAM(QuerAddr);
    }
    __except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            ptempdword = pLOAD_DOUBLE_PARAM_2(QuerAddr);
        }
        __except(NULL,EXCEPTION_EXECUTE_HANDLER){
            return;
        }
    }

	switch(QuerAddr % 8)
	{
	case 0:	*ptempdword = (*ptempdword & 0x00FFFFFFFFFFFFFF) | (gRT << 56);	break;
	case 1:	*ptempdword = (*ptempdword & 0x0000FFFFFFFFFFFF) | (gRT << 48);	break;
	case 2:	*ptempdword = (*ptempdword & 0x000000FFFFFFFFFF) | (gRT << 40);	break;
	case 3:	*ptempdword = (*ptempdword & 0x00000000FFFFFFFF) | (gRT << 32);	break;
	case 4:	*ptempdword = (*ptempdword & 0x0000000000FFFFFF) | (gRT << 24);	break;
	case 5:	*ptempdword = (*ptempdword & 0x000000000000FFFF) | (gRT << 16);	break;
	case 6:	*ptempdword = (*ptempdword & 0x00000000000000FF) | (gRT << 8);	break;
	case 7: *ptempdword = gRT;												break;
	}
}

//---------------------------------------------------------------------------------------



void r4300i_InitHardware(HardwareState* gHardwareState);
void r4300i_ResetMemory(MemoryState* gMemoryState);

void r4300i_Init()
{
    r4300i_InitHardware(&gHardwareState);

#ifdef ENABLE_OPCODE_DEBUGGER
    r4300i_InitHardware(&gHardwareState_Interpreter_Compare);
    r4300i_InitHardware(&gHardwareState_Flushed_Dynarec_Compare);
#endif
}

void r4300i_Reset()
{
    r4300i_ResetMemory(&gMemoryState);

#ifdef ENABLE_OPCODE_DEBUGGER
    r4300i_ResetMemory(&gMemoryState_Interpreter_Compare);
#endif
}


void r4300i_InitHardware(HardwareState* gHWState)
{
    //set all registers to 0
    memset( gHWState->COP0Con,       0,  sizeof(gHWState->COP0Con)); //not sure if we even need the COP0Con array
	memset( gHWState->COP0Reg,       0,  sizeof(gHWState->COP0Reg));
    memset( gHWState->COP1Con,       0,  sizeof(gHWState->COP1Con));
    memset( gHWState->fpr32        , 0,  sizeof(gHWState->fpr32  ));
    memset( gHWState->RememberFprHi, 0,  sizeof(gHWState->RememberFprHi));

    gHWState->GPR[gHI] = 0;
    gHWState->GPR[gLO] = 0;
    gHWState->LLbit    = 0;


		gHWState->GPR[0x00] = 0;
		gHWState->GPR[0x01] = 0;
		gHWState->GPR[0x02] = 0xffffffffd1731be9;
		gHWState->GPR[0x03] = 0xffffffffd1731be9;
		gHWState->GPR[0x04] = 0x01be9;
		gHWState->GPR[0x05] = 0xfffffffff45231e5;
		gHWState->GPR[0x06] = 0xffffffffa4001f0c;
		gHWState->GPR[0x07] = 0xffffffffa4001f08;
		gHWState->GPR[0x08] = 0x070;
		gHWState->GPR[0x09] = 0;
		gHWState->GPR[0x0a] = 0x040;
		gHWState->GPR[0x0b] = 0xffffffffa4000040;
		gHWState->GPR[0x0c] = 0xffffffffd1330bc3;
		gHWState->GPR[0x0d] = 0xffffffffd1330bc3;
		gHWState->GPR[0x0e] = 0x025613a26;
		gHWState->GPR[0x0f] = 0x02ea04317;
		gHWState->GPR[0x10] = 0;
		gHWState->GPR[0x11] = 0;
		gHWState->GPR[0x12] = 0;
		gHWState->GPR[0x13] = 0;
		gHWState->GPR[0x14] = rominfo.TV_System;
		gHWState->GPR[0x15] = 0;
		gHWState->GPR[0x16] = rominfo.CIC;
		gHWState->GPR[0x17] = 0x06;
		gHWState->GPR[0x18] = 0;
		gHWState->GPR[0x19] = 0xffffffffd73f2993;
		gHWState->GPR[0x1a] = 0;
		gHWState->GPR[0x1b] = 0;
		gHWState->GPR[0x1c] = 0;
		gHWState->GPR[0x1d] = 0xffffffffa4001ff0;
		gHWState->GPR[0x1e] = 0;
		gHWState->GPR[0x1f] = 0xffffffffa4001554;

        
    gHWState->COP0Reg[STATUS] = 0x70400004;
	gHWState->COP0Reg[RANDOM] = 0x0000001f;
	//gHWState->COP0Reg[RANDOM] = 0x0000002f;
	gHWState->COP0Reg[CONFIG] = 0x0006e463;  // zils boot doc ...
	gHWState->COP0Reg[PREVID]   = 0x00000b00;
	gHWState->COP1Con[0]      = 0x00000511;
    
	CPUdelayPC = 0;
	CPUdelay = 0;
	gHWState->pc = 0xA4000040;

    gMemoryState.MI[1] = 0x01010101; //MI_VERSION_REG (odd place for this)
}

//---------------------------------------------------------------------------------------
void r4300i_BootHelper(HardwareState* gHWState, MemoryState* gMemoryState)
{
	uint32 bootaddr;
	bootaddr = *(uint32*)(gMemoryState->ROM_Image+8) & 0x007FFFFF;
	memcpy(gMemoryState->RDRAM+bootaddr, gMemoryState->ROM_Image+0x1000, 0x400000 - bootaddr );
	gHWState->pc = 0x80000000+bootaddr;
}


void r4300i_ResetMemory(MemoryState* gMemoryState)
{
	memset(gMemoryState->RDRAM,   0, MEMORY_SIZE_RDRAM  );
	if( current_rdram_size == 0x800000 )
		memset(gMemoryState->ExRDRAM,   0, MEMORY_SIZE_EXRDRAM  );
//	memset(gMemoryState->RDREG,     0,      MEMORY_SIZE_RDREG    );
	memset(gMemoryState->ramRegs0,	0,		MEMORY_SIZE_RAMREGS0 );
	memset(gMemoryState->ramRegs4,	0,		MEMORY_SIZE_RAMREGS4 );
	memset(gMemoryState->ramRegs8,	0,		MEMORY_SIZE_RAMREGS8 );

	memset(gMemoryState->SP_MEM,  0, MEMORY_SIZE_SPMEM);
	memset(gMemoryState->SP_REG_1,0, MEMORY_SIZE_SPREG_1);
	memset(gMemoryState->SP_REG_2,0, MEMORY_SIZE_SPREG_2);
	gMemoryState->SP_REG_1[0x4] = SP_STATUS_HALT; // SP_STATUS_REG

    memset(gMemoryState->DPC,     0, MEMORY_SIZE_DPC    );
    memset(gMemoryState->DPS,     0, MEMORY_SIZE_DPS    );
    
	memset(gMemoryState->MI,      0, MEMORY_SIZE_MI     );
	gMemoryState->MI[1] = 0x01010101; //MI_VERSION_REG

    memset(gMemoryState->VI,      0, MEMORY_SIZE_VI     );
    memset(gMemoryState->AI,      0, MEMORY_SIZE_AI     );
    memset(gMemoryState->PI,      0, MEMORY_SIZE_PI     );
    
	memset(gMemoryState->RI,      0, MEMORY_SIZE_RI     );
	gMemoryState->RI[1] = 1; //RI_CONFIG_REG

    memset(gMemoryState->SI,      0, MEMORY_SIZE_SI     );
    memset(gMemoryState->C2A1,    0, MEMORY_SIZE_C2A1		);
    memset(gMemoryState->C1A1,    0, MEMORY_SIZE_C1A1		);

    memset(gMemoryState->C2A2,    0, MEMORY_SIZE_C2A2		);
	
    memset(gMemoryState->C1A3,	0, MEMORY_SIZE_C1A3		);
    memset(gMemoryState->GIO_REG, 0, MEMORY_SIZE_GIO_REG	);
    memset(gMemoryState->PIF,     0, MEMORY_SIZE_PIF		);
	
    r4300i_Init();

    CPUdelayPC = 0;
	CPUdelay = 0;
	InitTLB();
	InitDMA();
	ClearCPUTasks();
	RCP_Reset();

    /* Copy boot code to SP_DMEM */
    memcpy((uint8*)&gMemoryState->SP_MEM[0], gMemoryState->ROM_Image, 0x1000);
}


//---------------------------------------------------------------------------------------

_int32 Check_LW(uint32 QuerAddr)
{
	static uint32 Saved_AI_STATUS_REG=0;
    _int32 tempGPR;
	
#ifdef ENABLE_OPCODE_DEBUGGER
	if( p_gMemoryState == &gMemoryState  )
	{
        CHECK_FLASHRAM_LW(QuerAddr);
	}
#else
    CHECK_FLASHRAM_LW(QuerAddr);
#endif

	if( (QuerAddr & 0xFF000000) == 0x84000000 )
		QuerAddr |= 0xA0000000;

    switch (QuerAddr)
    {
	/* AI_LEN_REG       */		
	case 0xA4500004: 
		AI_LEN_REG = AUDIO_AiReadLength(); 
        tempGPR = AI_LEN_REG;
		break;

	/* SP_SEMAPHORE_REG */      
	case 0xA404001C: 
        tempGPR = SP_SEMAPHORE_REG; 
		SP_SEMAPHORE_REG = 1;    
		break;

	/* VI_CURRENT_REG   */      
	case 0xA4400010:
#ifdef FAST_COUNTER
#ifdef ENABLE_OPCODE_DEBUGGER
		if( p_gMemoryState != &gMemoryState || debug_opcode == 0 )
		{
			Count_Down(VI_COUNTER_INC_PER_LINE);
		}
#else
		Count_Down(VI_COUNTER_INC_PER_LINE);
#endif
		VIcounter = Get_VIcounter();
#else
		VIcounter += VI_COUNTER_INC_PER_LINE;	// This is important, many game read VI_CURRENT_REG to do video retrace
		gHWS_COP0Reg[COUNT] += VI_COUNTER_INC_PER_LINE*CounterFactors[CounterFactor]/VICounterFactors[CounterFactor];
#endif
		VI_CURRENT_REG = ( VIcounter / VI_COUNTER_INC_PER_LINE + VI_INTR_REG ) % (max_vi_lines+1);
        tempGPR = VI_CURRENT_REG;

		//TRACE1("VI CURRENT_REG is read, val=%08X", VI_CURRENT_REG);
        break;

	/* SI_STATUS_REG */
	case 0xA4800018:
		if( MI_INTR_REG_R & MI_INTR_SI )	// This is necessary, but not very necessary
			SI_STATUS_REG |= SI_STATUS_INTERRUPT;
		else
			SI_STATUS_REG &= ~SI_STATUS_INTERRUPT;
        tempGPR = SI_STATUS_REG;
		break;

	default: 
		if( QuerAddr >= MEMORY_START_C2A2 && QuerAddr <= MEMORY_START_C2A2+0x0001FFFF )
		{
			DisplayError("Read to C2A2, FLASHRAM");
			QuerAddr = (QuerAddr & ADDRESS_MASK_C2A2)+MEMORY_START_C2A2;
		}

		__try
		{
            tempGPR = LOAD_SWORD_PARAM_2(QuerAddr);
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
			DISPLAY_ADDRESS_ERROR( QuerAddr, "LW" );
        }
		break;
    }
#ifdef DEBUG_IO
	DebugIO(QuerAddr,"Read", tempGPR);
#endif

/* 
 * AI status (R): [31]/[0] ai_full (addr & len buffer full), [30] ai_busy 
 *		  Note that a 1->0 transition in ai_full will set interrupt
 *           (W): clear audio interrupt 
 */
	/*
	if( (AI_STATUS_REG & 0x80000001) != (Saved_AI_STATUS_REG & 0x80000001) )
	{
		if( ((Saved_AI_STATUS_REG & 0x80000000) && (AI_STATUS_REG & 0x8000000) == 0 ) ||
			((Saved_AI_STATUS_REG & 0x00000001) && (AI_STATUS_REG & 0x0000001) == 0 ) )
		{
			//DisplayError("AI_STATUS_REG Trigger AI interrupt");
			Trigger_AIInterrupt();
		}
		Saved_AI_STATUS_REG = AI_STATUS_REG;
	}
	*/

    return(tempGPR);
}    


//---------------------------------------------------------------------------------------
uint32 max_vi_lines;
uint32 max_vi_count;
uint32 vi_count_per_line;

uint32 RTVal;
uint32 SW_QuerAddr;

void Check_SW(uint32 QuerAddr, uint32 rt_ft) 
{
	CHECK_FLASHRAM_SW(QuerAddr,(uint32)gHWS_GPR[rt_ft]);

    RTVal = (uint32)gHWS_GPR[rt_ft];

	if( (QuerAddr & 0xFF000000) == 0x84000000 )
		QuerAddr |= 0xA0000000;

#ifdef DEBUG_IO
	DebugIO(QuerAddr,"Write", RTVal);
#endif

    switch (QuerAddr) 
    {
/* MI_MODE_REG_ADDR      */ case 0xA4300000: 
								WriteMI_ModeReg(RTVal)     ;   
								break;  
/* MI_INTR_MASK_REG      */ case 0xA430000C: 
								Handle_MI(RTVal)           ;   
								break;
/* VI_STATUS_REG		*/	case 0xA4400000:
								if( VI_STATUS_REG != RTVal)
								{
									VI_STATUS_REG = RTVal;
									VIDEO_ViStatusChanged();
								}
								break;

/* VI_CURRENT_REG        */ case 0xA4400010: 
								// Clear VI interrupt
								MI_INTR_REG_R &= NOT_MI_INTR_VI;   
								if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0)
								{
									gHWS_COP0Reg[CAUSE] &= 0xFFFFFBFF;
									if( (gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) == 0 )
										CPUNeedToCheckInterrupt = FALSE;
								}

								//VI_CURRENT_REG = RTVal;
								//VIcounter = RTVal * VI_COUNTER_INC_PER_LINE;
								break;

								
/* VI_V_SYNC_REG */			case 0xA4400018:
								VI_V_SYNC_REG = RTVal;
								break;
/* VI_ORIGIN_REG */			case 0xA4400004:
								if( VI_ORIGIN_REG != RTVal )
								{
									VI_ORIGIN_REG = RTVal;
									VIDEO_UpdateScreen();
								}
								break;
								

/* SP_STATUS_REG         */ case 0xA4040010:
								MI_INTR_REG_R &= NOT_MI_INTR_SP;   
								if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0)
								{
									gHWS_COP0Reg[CAUSE] &= 0xFFFFFBFF;
									if( (gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) == 0 )
										CPUNeedToCheckInterrupt = FALSE;
								}
								Handle_SP(RTVal);
								break;
/* SI_STATUS_REG         */ case 0xA4800018: 
								MI_INTR_REG_R &= NOT_MI_INTR_SI;   
								if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
								{
									gHWS_COP0Reg[CAUSE] &= 0xFFFFFBFF;
									if( (gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) == 0 )
										CPUNeedToCheckInterrupt = FALSE;
								}
								SI_STATUS_REG &= ~SI_STATUS_INTERRUPT;	// Clear the interrupt bit
								break;
								//break;
/* AI_STATUS_REG         */ case 0xA450000C: 
								MI_INTR_REG_R &= NOT_MI_INTR_AI;   
								if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
								{
									gHWS_COP0Reg[CAUSE] &= 0xFFFFFBFF; 
									if( (gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) == 0 )
										CPUNeedToCheckInterrupt = FALSE;
								}
								break;

/* AI_DACRATE_REG		 */ case 0xA4500010:
//        			            QuerAddr = (QuerAddr & 0x000fffff) | 0x1fb00000;
                                AI_DACRATE_REG = RTVal;
                                //LOAD_UWORD_PARAM_2(QuerAddr) = 48681812 / 22050       ;//RTVal;
								AUDIO_AiDacrateChanged(0);

								break;

/* PI_STATUS_REG         */ case 0xA4600010: 
								if( RTVal & PI_STATUS_CLR_INTR )
								{
									MI_INTR_REG_R &= NOT_MI_INTR_PI;   
									if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
									{
										gHWS_COP0Reg[CAUSE] &= 0xFFFFFBFF; 
										if( (gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) == 0 )
											CPUNeedToCheckInterrupt = FALSE;
									}
								}
								
								if( RTVal & PI_STATUS_RESET )
								{
					/*
					*	- When PIC is reset, if PIC happens to be busy, an interrupt will
					*	be generated as PIC returns to idle.  Otherwise, no interrupt will
					*	be generated and PIC remains idle.
					*/

									if( PI_STATUS_REG & 3)	// is PI busy
									{
										// Reset the PIC
										PI_STATUS_REG = 0;
										// Reset finished, set PI Interrupt
										Trigger_PIInterrupt();
									}
									else
									{
										// Reset the PIC
										PI_STATUS_REG = 0;
									}
								}

								break;		// Does not actually write into the PI_STATUS_REG
								//break;
/* DPC_STATUS_REG        */ case 0xA410000C: 
								MI_INTR_REG_R &= NOT_MI_INTR_DP;   
								if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
								{
									gHWS_COP0Reg[CAUSE] &= 0xFFFFFBFF; 
									if( (gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) == 0 )
										CPUNeedToCheckInterrupt = FALSE;
								}
								Handle_DPC(RTVal);
								break;
/* DPC_END_REG */			case 0xA4100004:
								Trigger_DPInterrupt();
								break;

							case 0xA4100010:	//DPC_CLOCK_REG
							case 0xA4100014:	//DPC_BUFBUSY_REG
							case 0xA4100018:	//DPC_PIPEBUSY_REG
							case 0xA410001C:	//DPC_TMEM_REG
							case 0xA4300004:	//MI_VERSION_REG or MI_NOOP_REG
							case 0xA4300008:	//MI_INTR_REG
								break;	//read only registers

/* SP_SEMAPHORE_REG      */ case 0xA404001C: 
								SP_SEMAPHORE_REG       = 0 ;   
								break;
/* PI_WR_LEN_REG         */ case 0xA460000C: 
								PI_WR_LEN_REG          = RTVal; 
								DMA_PI_MemCopy_From_Cart_To_DRAM() ;   	
								break;
/* PI_RD_LEN_REG         */ case 0xA4600008: 
								PI_RD_LEN_REG          = RTVal; 
								DMA_PI_MemCopy_From_DRAM_To_Cart() ;        
								break;
/* SP_WR_LEN_REG         */ case 0xA404000C: 
								SP_WR_LEN_REG          = RTVal; 
								DMA_MemCopy_SP_to_DRAM() ;       
								break;
/* SP_RD_LEN_REG         */ case 0xA4040008: 
								SP_RD_LEN_REG          = RTVal; 
								DMA_MemCopy_DRAM_To_SP()  ;       
								break;
/* SI_PIF_ADDR_WR64B_REG */ case 0xA4800010: 
								SI_PIF_ADDR_WR64B_REG  = RTVal; 
								DMA_MemCopy_DRAM_to_SI() ;       
								break;
/* SI_PIF_ADDR_RD64B_REG */ case 0xA4800004: 
								SI_PIF_ADDR_RD64B_REG  = RTVal; 
								DMA_MemCopy_SI_To_DRAM()  ;       
								break;
/* VI_WIDTH_REG */          case 0xA4400008: 
								if( VI_WIDTH_REG != RTVal )
								{
									VI_WIDTH_REG = RTVal; 
									//ResizeVideoWindow() ;  
									VIDEO_ViWidthChanged();
								}
								break;
/* AI_LEN_REG            */ case 0xA4500004: 
								DMA_AI();
                                AI_LEN_REG             = RTVal;
#ifdef DEBUG_AUDIO
								if(debug_audio)
									TRACE3( "%08X: Play %d bytes of audio at %08X", gHWS_pc, AI_LEN_REG, AI_DRAM_ADDR_REG);
#endif
                                AUDIO_AiLenChanged();
								AUDIO_AiUpdate(FALSE);

        break;          
        
    default:
		if( QuerAddr >= MEMORY_START_C2A2 && QuerAddr <= (MEMORY_START_C2A2+MEMORY_SIZE_C2A2) )
		{
#ifdef DEBUG_FLASHRAM
			DisplayError("Write to C2A2, FLASHRAM");
#endif
			QuerAddr = (QuerAddr & ADDRESS_MASK_C2A2)+MEMORY_START_C2A2;
		}
		__try { 
            LOAD_UWORD_PARAM_2(QuerAddr) = RTVal;
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			DISPLAY_ADDRESS_ERROR( QuerAddr, "SW" );
        }
        break;
                       
    }
}

// This function will display error message and set exception when address error happens when SW
void SW_AddrError(uint32 QuerAddr, uint32 val)
{
	DISPLAY_ADDRESS_ERROR( QuerAddr, "SW" );
	//TRIGGER_ADDR_ERROR_EXCEPTION(EXC_WADE,QuerAddr);
	// Use dummy segment
	// *(uint32*)(dummySegment+(QuerAddr&0x00007fff)) = val;
}

static int regindex;


void rdram_reg_sw(void)
{
	__try 
	{
        LOAD_UWORD_PARAM_2(SW_QuerAddr) = RTVal;
    }
	__except(NULL,EXCEPTION_EXECUTE_HANDLER)
	{
		SW_AddrError(SW_QuerAddr, RTVal);
	}
}

void sp_reg_sw(void)
{
	if( SW_QuerAddr < 0xA4002000 )
	{
        __try
		{
            LOAD_UWORD_PARAM_2(SW_QuerAddr) = RTVal;
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
			SW_AddrError(SW_QuerAddr, RTVal);
        }
	}
	else
	{
	switch( SW_QuerAddr )
	{
	/* SP_STATUS_REG         */ 
	case 0xA4040010:
		MI_INTR_REG_R &= NOT_MI_INTR_SP;   
		if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0)
		{
			gHWS_COP0Reg[CAUSE] &= 0xFFFFFBFF;
			if( (gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) == 0 )
				CPUNeedToCheckInterrupt = FALSE;
		}
		Handle_SP(RTVal);
		break;
	
	/* SP_SEMAPHORE_REG      */ 
	case 0xA404001C: 
		SP_SEMAPHORE_REG       = 0 ;   
		break;

	/* SP_WR_LEN_REG         */ 
	case 0xA404000C: 
		SP_WR_LEN_REG          = RTVal; 
		DMA_MemCopy_SP_to_DRAM() ;       
		break;

	/* SP_RD_LEN_REG         */ 
	case 0xA4040008: 
		SP_RD_LEN_REG          = RTVal; 
		DMA_MemCopy_DRAM_To_SP()  ;       
		break;
	default:
        __try
		{
            LOAD_UWORD_PARAM_2(SW_QuerAddr) = RTVal;
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
			SW_AddrError(SW_QuerAddr, RTVal);
        }
		break;
	}
	}
}

void dp_reg_sw(void)
{
	/* DPC_STATUS_REG        */ 
	switch( regindex )
	{
	case 3: 
		MI_INTR_REG_R &= NOT_MI_INTR_DP;   
		if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
		{
			gHWS_COP0Reg[CAUSE] &= 0xFFFFFBFF; 
			if( (gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) == 0 )
				CPUNeedToCheckInterrupt = FALSE;
		}
		Handle_DPC(RTVal);
		break;
	
	/* DPC_END_REG */			
	case 1:
		Trigger_DPInterrupt();
		break;
	default:
		if( regindex < NUMBEROFDPREG )
			gMS_DPC[regindex] = RTVal;
		else
			DisplayError("Error DPC Register write, index = %d", regindex);
		break;
	}
}

void dpspan_rag_sw(void)
{
	if( regindex < NUMBEROFDPSREG )
		gMS_DPS[regindex] = RTVal;
	else
		DisplayError("Error DPS Register write, index = %d", regindex);
}

void mi_reg_sw(void)
{
	switch( regindex )
	{
	/* MI_MODE_REG_ADDR      */ 
	case 0: 
		WriteMI_ModeReg(RTVal)     ;   
		break;  

	/* MI_INTR_MASK_REG      */ 
	case 3: 
		Handle_MI(RTVal)           ;   
		break;

	default:
		if( regindex < NUMBEROFMIREG )
			gMS_MI[regindex] = RTVal;
		else
			DisplayError("Error MI Register write, index = %d", regindex);
		break;
	}
}

void vi_reg_sw(void)
{
	switch( regindex )
	{
	case 0:	/* VI_STATUS_REG */
		VI_STATUS_REG = RTVal;
		VIDEO_ViStatusChanged();
		break;

	/* VI_CURRENT_REG        */ 
	case 4: 
		// Clear VI interrupt
		MI_INTR_REG_R &= NOT_MI_INTR_VI;   
		if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0)
		{
			gHWS_COP0Reg[CAUSE] &= 0xFFFFFBFF;
			if( (gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) == 0 )
				CPUNeedToCheckInterrupt = FALSE;
		}

		//VI_CURRENT_REG = RTVal;
		//VIcounter = RTVal * VI_COUNTER_INC_PER_LINE;
		break;

								
	/* VI_V_SYNC_REG */			
	case 6:
		VI_V_SYNC_REG = RTVal;
		break;

	/* VI_ORIGIN_REG */			
	case 1:
		if( VI_ORIGIN_REG != RTVal )
		{
			VI_ORIGIN_REG = RTVal;
			VIDEO_UpdateScreen();
		}
		break;

	/* VI_WIDTH_REG */      
	case 2: 
		if( VI_WIDTH_REG != RTVal )
		{
			VI_WIDTH_REG = RTVal; 
			//ResizeVideoWindow() ;  
			VIDEO_ViWidthChanged();
		}
		break;

	default:
		if( regindex < NUMBEROFVIREG )
			gMS_VI[regindex] = RTVal;
		else
			DisplayError("Error VI Register write, index = %d", regindex);
		break;
	}
}

void ai_reg_sw(void)
{
	switch( regindex )
	{
	/* AI_STATUS_REG         */ 
	case 3: 
		MI_INTR_REG_R &= NOT_MI_INTR_AI;   
		if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
		{
			gHWS_COP0Reg[CAUSE] &= 0xFFFFFBFF; 
			if( (gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) == 0 )
				CPUNeedToCheckInterrupt = FALSE;
		}
		break;

	/* AI_DACRATE_REG		 */ 
	case 4:
//      QuerAddr = (QuerAddr & 0x000fffff) | 0x1fb00000;
        AI_DACRATE_REG = RTVal;
        //LOAD_UWORD_PARAM_2(QuerAddr) = 48681812 / 22050       ;//RTVal;
		AUDIO_AiDacrateChanged(0);

		break;

	/* AI_LEN_REG            */ 
	case 1: 
		DMA_AI();
        AI_LEN_REG             = RTVal;
        AUDIO_AiLenChanged();
		AUDIO_AiUpdate(FALSE);
		// (AUDIO_PlaySnd)(RDRAM,AI);

#ifdef DEBUG_AUDIO
		if(debug_audio)
			TRACE3( "%08X: Play %d bytes of audio at %08X", gHWS_pc, AI_LEN_REG, AI_DRAM_ADDR_REG);
#endif
        break;          
        
	default:
		if( regindex < NUMBEROFAIREG )
			gMS_AI[regindex] = RTVal;
		else
			DisplayError("Error AI Register write, index = %d", regindex);
		break;
	}
}

void pi_reg_sw(void)
{
	switch( regindex )
	{
	/* PI_STATUS_REG         */ 
	case 4: 
		if( RTVal & PI_STATUS_CLR_INTR )
		{
			MI_INTR_REG_R &= NOT_MI_INTR_PI;   
			if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
			{
				gHWS_COP0Reg[CAUSE] &= 0xFFFFFBFF; 
				if( (gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) == 0 )
					CPUNeedToCheckInterrupt = FALSE;
			}
		}
		
		if( RTVal & PI_STATUS_RESET )
		{
			if( PI_STATUS_REG & 3)	// is PI busy
			{
				// Reset finished, set PI Interrupt
				Trigger_PIInterrupt();
			}
			// Reset the PIC
			PI_STATUS_REG = 0;
		}

		break;		// Does not actually write into the PI_STATUS_REG

	/* PI_WR_LEN_REG         */ 
	case 3: 
		PI_WR_LEN_REG          = RTVal; 
		DMA_PI_MemCopy_From_Cart_To_DRAM() ;   	
		break;

	/* PI_RD_LEN_REG         */ 
	case 2: 
		PI_RD_LEN_REG          = RTVal; 
		DMA_PI_MemCopy_From_DRAM_To_Cart() ;        
		break;

	default:
		if( regindex < NUMBEROFPIREG )
			gMS_PI[regindex] = RTVal;
		else
			DisplayError("Error PI Register write, index = %d", regindex);
		break;
	}
}

void si_reg_sw(void)
{
	switch( regindex )
	{
	/* SI_STATUS_REG         */ 
	case 6: 
		MI_INTR_REG_R &= NOT_MI_INTR_SI;   
		if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
		{
			gHWS_COP0Reg[CAUSE] &= 0xFFFFFBFF;
			if( (gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) == 0 )
				CPUNeedToCheckInterrupt = FALSE;
		}
		SI_STATUS_REG &= ~SI_STATUS_INTERRUPT;	// Clear the interrupt bit
		break;

	/* SI_PIF_ADDR_WR64B_REG */ 
	case 4: 
		SI_PIF_ADDR_WR64B_REG  = RTVal; 
		DMA_MemCopy_DRAM_to_SI() ;       
		break;
	
	/* SI_PIF_ADDR_RD64B_REG */ 
	case 1: 
		SI_PIF_ADDR_RD64B_REG  = RTVal; 
		DMA_MemCopy_SI_To_DRAM()  ;       
		break;

	default:
		if( regindex < NUMBEROFSIREG )
			gMS_SI[regindex] = RTVal;
		else
			DisplayError("Error SI Register write, index = %d", regindex);
		break;
	}
}

void ri_reg_sw(void)
{
	switch( regindex )
	{
		default:
		if( regindex < NUMBEROFRIREG )
			gMS_RI[regindex] = RTVal;
		else
			DisplayError("Error RI Register write, index = %d", regindex);
		break;
	}

}

void (*IO_SW[10])(void)=
{
	rdram_reg_sw,	sp_reg_sw,		dp_reg_sw,		dpspan_rag_sw,	mi_reg_sw,	
	vi_reg_sw,		ai_reg_sw,		pi_reg_sw,		ri_reg_sw,		si_reg_sw
};

void Faster_Check_SW(uint32 QuerAddr, uint32 rt_ft)
{
	//int i;
	RTVal = (uint32)gHWS_GPR[rt_ft];
	SW_QuerAddr = QuerAddr;
	regindex=(SW_QuerAddr&0xFF)/4;

	//i = ((SW_QuerAddr&0x1FFFFFFF)>>20) - 0x3F;
	IO_SW[((SW_QuerAddr&0x1FFFFFFF)>>20) - 0x3F]();
	//IO_SW[i]();
}

__inline void Display_Address_Error( uint32 addr, char *opcode )
{
#ifdef DEBUG_COMMON
	uint32 Instruction = FetchInstruction();
	uint32 virtualaddress = (uint32)((_int32)gBASE + (_int32)OFFSET_IMMEDIATE);
	TRACE4( "%08X: %s to access VA=%08X, PA=%08X, Out of range. ", gHWS_pc, opcode, virtualaddress, addr);
#endif
}

