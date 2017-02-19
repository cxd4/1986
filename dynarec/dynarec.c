/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Emulator for Nintendo 64 console system                              |
 |  Copyright (C) 2000  Joel Middendorf  schibo@emuhq.com                       |
 |  1964 Dynarec Engine Copyright (C) 2000  Joel Middendorf  schibo@emuhq.com   | 
 |                                                                              |
 |  Do NOT violate the terms and conditions of this copyright                   | 
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
 
	Many thanks to zilmar for his help with the dynarec engine.
*/

//dynarec.c compiler options
/************************************************************
 * Macro Name:  SAFE_DYNAREC								*
 * Description: Will call interpretive opcodes when defined.*
 *              Used for debugging the dynarec.             *
 *															*
 ************************************************************/
//#define SAFE_DYNAREC 1 


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "r4300i.h"
#include "globals.h"
#include "dynarec.h"
#include "interrupt.h"

//---------------------------------------------------------------------------------------

extern void WinDynDebugPrintInstruction(uint32 Instruction);
extern void __cdecl DisplayError (char * Message, ...);
extern void Trigger_VIInterrupt();
extern void CheckEm();

RETURN_TYPE dyn_UNUSED(OP_PARAMS) {	DisplayError("Found an unused");return(Dest);}

//---------------------------------------------------------------------------------------

#ifdef DEBUG_COMMON
extern void (*DebugInstruction[64])(uint32 Instruction);
char TheOpcodeStr[0xFF];
#endif

//---------------------------------------------------------------------------------------

RETURN_TYPE FetchDelay(int StorePC, uint8* Dest)
{
	uint32 Instruction;
	
	Instruction = *InstructionPointer;
	DYNDEBUG_UPDATE
	Dest = dyn_CPU_instruction[_OPCODE_](PASS_PARAMS);
	SetVariable(Dest, &pc, (uint32)(StorePC))
return(Dest);
}

//---------------------------------------------------------------------------------------

extern uint8* InterruptVector;
extern uint8* IVTStart;
extern uint8* TLBMissStart;
void InitDynarec();
void InitDynarec() 
{
	InterruptVector = (uint8*)VirtualAlloc(NULL, 8192, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	TLBMissStart = InterruptVector;
}

//---------------------------------------------------------------------------------------

RETURN_TYPE RecompileIVT(uint8* Dest)
{
	uint32 Instruction;

	KEEP_RECOMPILING = 1;

	InstructionPointer = (uint32*)((uint8*)sDWORD_R[0x8000] + (0x0080));
	(*(uint32*)((uint8*)sDYN_PC_LOOKUP[0x8000>>16] + 0x0080)) = (uint32)Dest;
	FetchOpcode

_BEAVIS:	
	while(KEEP_RECOMPILING)
	{
		__asm add pc, 4
		*InstructionPointer++;
		if (pc == 0x80000180)
			IVTStart = Dest;
		FetchOpcode
	}

	if (pc < 0x80000180)
	{
		KEEP_RECOMPILING = 1;
		goto _BEAVIS;
	}
	return(Dest);
}

//---------------------------------------------------------------------------------------

void SetTLBMissException() {
	COP0Reg[EPC] = pc;
	Block = TLBMissStart;
	pc = 0x80000080;
	COP0Reg[CAUSE] &= ~0x400;
}

//---------------------------------------------------------------------------------------

RETURN_TYPE Dyn_Intr_Common(uint8* Dest)
{
	uint8* TempDest;
	static int AlreadyRecompiled;

	if ((( COP0Reg[STATUS] & IE   ))		== 0 )				{return(Dest);}
	if ((( COP0Reg[STATUS] & EXL_OR_ERL  )) != 0 )				{return(Dest);}
	if (( COP0Reg[CAUSE] & 0x0000FF00) == 0)					{return(Dest);}
	if (( COP0Reg[CAUSE] & COP0Reg[STATUS] & 0x0000FF00) == 0)	{return(Dest);}

	COP0Reg[EPC] = pc;
	COP0Reg[STATUS] |= EXL;			// set EXL = 1 

	if (!AlreadyRecompiled)
	{
		TempDest = Dest;	
		Dest = TLBMissStart;
		
		pc = 0x80000080;
		Dest = RecompileIVT(Dest);

		Dest = TempDest;
		AlreadyRecompiled = 1;
	}

	COP0Reg[CAUSE] &= NOT_BD;		/* clear BD		  */
	COP0Reg[CAUSE] &= NOT_EXCCODE;	/* clear EXCCode  */
	pc = 0x80000180;
	Block = IVTStart;
	
	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_blez(OP_PARAMS)	{	BLEZ_MACRO	SETUP_DELAY_SLOTS_UNLIKELY_TFF			return(Dest);}
RETURN_TYPE dyn_r4300i_blezl(OP_PARAMS)	{	BLEZ_MACRO	SETUP_DELAY_SLOTS_LIKELY_TFF			return(Dest);}
RETURN_TYPE dyn_r4300i_bgez(OP_PARAMS)	{	BGEZ_MACRO	SETUP_DELAY_SLOTS_UNLIKELY_TFF			return(Dest);}
RETURN_TYPE dyn_r4300i_bgezall(OP_PARAMS){	BGEZ_MACRO	SETUP_DELAY_SLOTS_LIKELY_AND_LINK_TFF	return(Dest);}
RETURN_TYPE dyn_r4300i_bgezl(OP_PARAMS)	{	BGEZ_MACRO	SETUP_DELAY_SLOTS_LIKELY_TFF			return(Dest);}
RETURN_TYPE dyn_r4300i_bltz(OP_PARAMS)	{	BLTZ_MACRO	SETUP_DELAY_SLOTS_UNLIKELY_TFF			return(Dest);}
RETURN_TYPE dyn_r4300i_bltzal(OP_PARAMS){	BLTZ_MACRO	SETUP_DELAY_SLOTS_UNLIKELY_AND_LINK_TFF	return(Dest);}
RETURN_TYPE dyn_r4300i_bltzall(OP_PARAMS){	BLTZ_MACRO	SETUP_DELAY_SLOTS_LIKELY_AND_LINK_TFF	return(Dest);}
RETURN_TYPE dyn_r4300i_bltzl(OP_PARAMS)	{	BLTZ_MACRO	SETUP_DELAY_SLOTS_LIKELY_TFF			return(Dest);}
RETURN_TYPE dyn_r4300i_bgtz(OP_PARAMS)	{	BGTZ_MACRO	SETUP_DELAY_SLOTS_UNLIKELY_TFF			return(Dest);}
RETURN_TYPE dyn_r4300i_bgtzl(OP_PARAMS)	{	BGTZ_MACRO	SETUP_DELAY_SLOTS_LIKELY_TFF			return(Dest);}
RETURN_TYPE dyn_r4300i_bgezal(OP_PARAMS){	BGEZ_MACRO	SETUP_DELAY_SLOTS_UNLIKELY_AND_LINK_TFF	return(Dest);}

//---------------------------------------------------------------------------------------

/*
-----------------------------------------------------------------
| JR        | Jump Register                                     |
|-----------|---------------------------------------------------|
|  000000   |   rs    |     0000 0000 0000 000      |001000 (8) |
------6----------5------------------15--------------------6------
 Format:  JR rs
 Purpose: To branch to an instruction address in a register.
*/
RETURN_TYPE dyn_r4300i_jr(OP_PARAMS) 
{
	uint32 TheGPRAddr = (uint32)&GPR[RS_BASE_FMT];

	/* Get delay */
	_asm add pc, 4
	*InstructionPointer++;
	Instruction = *InstructionPointer;
	INCREMENT_COUNT(Dest)
	DYNDEBUG_UPDATE
	Dest = dyn_CPU_instruction[_OPCODE_](PASS_PARAMS);

//	Dynarec equivalent of: pc = GPR[RS_BASE_FMT];
//0040BE2B 8B 0D 60 61 49 00    mov         ecx,dword ptr [_GPR+40h (00496160)]
//0040BE31 89 0D 68 62 49 00    mov         dword ptr [_pc (00496268)],ecx
	PUTDST16KNOWN(Dest, 0x0D8B)
	PUTDST32(Dest, TheGPRAddr)

	PUTDST16KNOWN(Dest, 0x0D89)
	PUTDST32(Dest, (uint32)&pc)

	AsmReturn

	KEEP_RECOMPILING = 0;
	return(Dest);
}

//---------------------------------------------------------------------------------------

/*
-----------------------------------------------------------------
| BEQ       | Branch on EQual                                   |
|-----------|---------------------------------------------------|
|000100 (4) |   rs    |   rt    |            offset             |
------6----------5---------5-------------------16----------------
 Format:  BEQ rs, rt, offset
 Purpose: To compare GPRs then do a PC-relative conditional branch.
 Comment: BEQ rs, r0, offset is equal to a BEQZ rs, offset
          BEQ r0, r0, offset is equal to a B offset
 Descrip: branch if rs = rt
*/
RETURN_TYPE dyn_r4300i_beq(OP_PARAMS) 
{
	uint32 PrevLabelTrue,	PrevLabelFalse;
	uint8 *LabelTrue,		*LabelFalse;
#ifdef ADDRESS_RESOLUTION
	uint32* BranchToJump;	uint32* BranchToContinue;
#endif

	uint32 RSArg = RS_BASE_FMT;
	uint32 TempRS = (uint32)&GPR[RSArg];
	uint32 TempRT = RT_FT;

	if (TempRT == 0)
	{
		if (RSArg == 0)	/* B */
		{
			JumpLocal8(Dest, LabelTrue)
			PrevLabelTrue = (uint32)Dest;
		}
		else			/* BEQZ */
		{
			CompMIPS64RegTo0(Dest, TempRS)
			NearJumpConditionLabel8(Dest, LabelTrue, JE_NEAR)
			PrevLabelTrue = (uint32)Dest;
		}
			JumpLocal8(Dest, LabelFalse)
			PrevLabelFalse = (uint32)Dest;	
	}
	else				/* BEQ */
	{
		TempRT = (uint32)&GPR[TempRT];

		CompMIPS64RegToMIPS64Reg_LO(Dest, TempRS, TempRT)
		NearJumpConditionLabel8(Dest, LabelFalse, JNE_NEAR)
		PrevLabelFalse = (uint32)Dest; //Mark

		CompMIPS64RegToMIPS64Reg_HI(Dest, TempRS, TempRT)
		NearJumpConditionLabel8(Dest, LabelTrue, JE_NEAR)
		PrevLabelTrue = (uint32)Dest; //Mark

	}
	SETUP_DELAY_SLOTS_UNLIKELY_TF
	return(Dest);
}

//---------------------------------------------------------------------------------------

/*
-----------------------------------------------------------------
| BEQL      | Branch on EQual Likley                            |
|-----------|---------------------------------------------------|
|010100 (20)|   rs    |   rt    |            offset             |
------6----------5---------5-------------------16----------------
 Format:  BEQL rs, rt, offset
 Purpose: To compare GPRs then do a PC-relative conditional branch;
          execute the delay slot only if the branch is taken.
 Comment: BEQL rs, r0, offset is equal to a BEQZL rs, offset
 Descrip: branch if rs = rt
*/
RETURN_TYPE dyn_r4300i_beql(OP_PARAMS) 
{
	uint32 PrevLabelTrue,	PrevLabelFalse;
	uint8 *LabelTrue,		*LabelFalse;
#ifdef ADDRESS_RESOLUTION
	uint32* BranchToJump;	uint32* BranchToContinue;
#endif

	uint32 RSArg = RS_BASE_FMT;
	uint32 TempRS = (uint32)&GPR[RSArg];
	uint32 TempRT = RT_FT;

	if (TempRT == 0) /* BEQZL */
	{
		if (RSArg == 0)	/* BL */
		{
			JumpLocal8(Dest, LabelTrue)
			PrevLabelTrue = (uint32)Dest;
		}
		else			/* BEQZL */
		{
			CompMIPS64RegTo0(Dest, TempRS)
			NearJumpConditionLabel8(Dest, LabelTrue, JE_NEAR)
			PrevLabelTrue = (uint32)Dest;
		}
			JumpLocal8(Dest, LabelFalse)
			PrevLabelFalse = (uint32)Dest;	
	}
	else			/* BEQL */
	{
		TempRT = (uint32)&GPR[TempRT];

		CompMIPS64RegToMIPS64Reg_LO(Dest, TempRS, TempRT)
		NearJumpConditionLabel8(Dest, LabelFalse, JNE_NEAR)
		PrevLabelFalse = (uint32)Dest; //Mark

		CompMIPS64RegToMIPS64Reg_HI(Dest, TempRS, TempRT)
		NearJumpConditionLabel8(Dest, LabelTrue, JE_NEAR)
		PrevLabelTrue = (uint32)Dest; //Mark

	}

	SETUP_DELAY_SLOTS_LIKELY_TF
	return(Dest);
}

//---------------------------------------------------------------------------------------

/*
-----------------------------------------------------------------
| BNE       | Branch on Not Equal                               |
|-----------|---------------------------------------------------|
|000101 (5) |   rs    |   rt    |            offset             |
------6----------5---------5-------------------16----------------
 Format:  BNE rs, rt, offset
 Purpose: To compare GPRs then do a PC-relative conditional branch.
 Comment: BNE rs, r0, offset is equal to a BNEZ rs, offset
 Descrip: branch if rs <> rt
*/
RETURN_TYPE dyn_r4300i_bne(OP_PARAMS)
{
	uint32 PrevLabelTrue,	PrevLabelFalse;
	uint8 *LabelTrue,		*LabelFalse;
#ifdef ADDRESS_RESOLUTION
	uint32* BranchToJump;	uint32* BranchToContinue;
#endif

	uint32 TempRS = (uint32)&GPR[RS_BASE_FMT];
	uint32 TempRT = RT_FT;
		
	if (TempRT == 0) /* BNEZ */
	{
		CompMIPS64RegTo0(Dest, TempRS)
		NearJumpConditionLabel8(Dest, LabelTrue, JNE_NEAR)
		PrevLabelTrue = (uint32)Dest;

		JumpLocal8(Dest, LabelFalse)
		PrevLabelFalse = (uint32)Dest;
	}
	else			/* BNE */
	{
		TempRT = (uint32)&GPR[TempRT];
		
		CompMIPS64RegToMIPS64Reg_LO(Dest, TempRS, TempRT)
		NearJumpConditionLabel8(Dest, LabelTrue, JNE_NEAR)
		PrevLabelTrue = (uint32)Dest; //Mark

		CompMIPS64RegToMIPS64Reg_HI(Dest, TempRS, TempRT)
		NearJumpConditionLabel8(Dest, LabelFalse, JE_NEAR)
		PrevLabelFalse = (uint32)Dest; //Mark
	}

	SETUP_DELAY_SLOTS_UNLIKELY_TF
	return(Dest);
}

//---------------------------------------------------------------------------------------

/*
-----------------------------------------------------------------
| BNEL      | Branch on Not Equal Likley                        |
|-----------|---------------------------------------------------|
|010101 (21)|   rs    |   rt    |            offset             |
------6----------5---------5-------------------16----------------
 Format:  BNEL rs, rt, offset
 Purpose: To compare GPRs then do a PC-relative conditional branch;
          execute the delay slot only if the branch is taken.
 Comment: BNEL rs, r0, offset is equal to a BNEZL rs, offset
 Descrip: branch if rs <> rt
*/
RETURN_TYPE dyn_r4300i_bnel(OP_PARAMS) 
{
	uint32 PrevLabelTrue,	PrevLabelFalse;
	uint8 *LabelTrue,	*LabelFalse;
#ifdef ADDRESS_RESOLUTION
	uint32* BranchToJump;	uint32* BranchToContinue;
#endif

	uint32 TempRS = (uint32)&GPR[RS_BASE_FMT];
	uint32 TempRT = RT_FT;

	if (TempRT == 0) /* BNEZL */
	{
		CompMIPS64RegTo0(Dest, TempRS)
		NearJumpConditionLabel8(Dest, LabelTrue, JNE_NEAR)
		PrevLabelTrue = (uint32)Dest;

		JumpLocal8(Dest, LabelFalse)
		PrevLabelFalse = (uint32)Dest;	
	}
	else			/* BNEL */
	{
		TempRT = (uint32)&GPR[TempRT];
		
		CompMIPS64RegToMIPS64Reg_LO(Dest, TempRS, TempRT)
		NearJumpConditionLabel8(Dest, LabelTrue, JNE_NEAR)
		PrevLabelTrue = (uint32)Dest; //Mark

		CompMIPS64RegToMIPS64Reg_HI(Dest, TempRS, TempRT)
		NearJumpConditionLabel8(Dest, LabelFalse, JE_NEAR)
		PrevLabelFalse = (uint32)Dest; //Mark
	}

	SETUP_DELAY_SLOTS_LIKELY_TF
	return(Dest);
}

//---------------------------------------------------------------------------------------

/*
-----------------------------------------------------------------
| J         | Jump                                              |
|-----------|---------------------------------------------------|
|000010 (2) |                    INSTR_INDEX                    |
------6-------------------------------26-------------------------
 Format:  J target
 Purpose: To branch within the current 256 MB aligned region.
*/
RETURN_TYPE dyn_r4300i_j(OP_PARAMS) 
{
	uint32 Target = INSTR_INDEX;

#ifdef USE_J_SPEEDHACK
	if (pc == Target)	{
		SetVariable(Dest, &COP0Reg[COUNT], MAGICNUMBER); /* Speed hack */
	}
#endif

	_asm add pc, 4
	InstructionPointer++;
	Instruction = *InstructionPointer;
	INCREMENT_COUNT(Dest)
	
	DYNDEBUG_UPDATE
	Dest = dyn_CPU_instruction[_OPCODE_](PASS_PARAMS);
	SetVariable(Dest, &pc, Target)

	AsmReturn

	KEEP_RECOMPILING = 0;
	return(Dest);
}

//---------------------------------------------------------------------------------------

/*
-----------------------------------------------------------------
| JAL       | Jump And Link                                     |
|-----------|---------------------------------------------------|
|000011 (3) |                    INSTR_INDEX                    |
------6-------------------------------26-------------------------
 Format:  JAL target
 Purpose: To procedure call within the current 256 MB aligned region.
 Descrip: return address in ra
*/
RETURN_TYPE dyn_r4300i_jal(OP_PARAMS) 
{
#ifdef ADDRESS_RESOLUTION
	uint32* BranchToJump;
#endif
	uint32 Target = INSTR_INDEX;
	uint32 RA_Addr = (uint32)&GPR[RA];

	_asm add pc, 4
	*InstructionPointer++;
	Instruction = *InstructionPointer;

	//Similar to FetchDelay but without setting LOOKUP label and not setting pc

	DYNDEBUG_UPDATE
	Dest = dyn_CPU_instruction[_OPCODE_](PASS_PARAMS);
	
	SetRAToPC(Dest, pc+4)	/* Return Address in RA (Link) */

	//NOW we can set the pc
	SetVariable(Dest, &pc, Target)

//jmp $BranchToJump
#ifdef ADDRESS_RESOLUTION
	JumpEAX32(Dest, BranchToJump)											

//$BranchToJump:
*BranchToJump = (uint32)Dest;
	SetVariable(Dest, &LocationJumpedFrom, BranchToJump)
#endif
	AsmReturn

	KEEP_RECOMPILING = 0;

	return(Dest);
}

//---------------------------------------------------------------------------------------

/*
-----------------------------------------------------------------
| JALR      | Jump And Link Register                            |
|-----------|---------------------------------------------------|
|  000000   |   rs    |  00000  |   rd    |  00000  |001001 (9) |
------6----------5---------5---------5---------5----------6------
 Format:  JALR rs, rd
 Purpose: To procedure call to an instruction address in a register.
 Descrip: return address in rd
*/
RETURN_TYPE dyn_r4300i_jalr(OP_PARAMS) 
{
	uint32 RS_Addr  = (uint32)&GPR[RS_BASE_FMT];
	uint32 RD_Addr  = (uint32)&GPR[RD_FS];

	/* Get delay */
	_asm add pc, 4
	*InstructionPointer++;
	Instruction = *InstructionPointer;

	INCREMENT_COUNT(Dest)
	DYNDEBUG_UPDATE
	Dest = dyn_CPU_instruction[_OPCODE_](PASS_PARAMS);

	SetVariable(Dest, RD_Addr, pc+4);
	SetVariable(Dest, RD_Addr+4, 0xFFFFFFFF);

//	Dynarec equivalent of: pc = (uint32)GPR[RS_BASE_FMT];
//0040BE2B 8B 0D 60 61 49 00    mov         ecx,dword ptr [_GPR+40h (00496160)]
//0040BE31 89 0D 68 62 49 00    mov         dword ptr [_pc (00496268)],ecx
	PUTDST16KNOWN(Dest, 0x0D8B)
	PUTDST32(Dest, RS_Addr)

	PUTDST16KNOWN(Dest, 0x0D89)
	PUTDST32(Dest, (uint32)&pc)

	AsmReturn

	KEEP_RECOMPILING = 0;

	return(Dest);
}

//---------------------------------------------------------------------------------------

#ifdef ADDRESS_RESOLUTION
#define BC1(OPERATION)										\
	uint32 PrevLabelTrue,	PrevLabelFalse;					\
	uint8 *LabelTrue,		*LabelFalse;					\
	uint32* BranchToJump;	uint32* BranchToContinue;		\
															\
	/* mov eax, COP1Con[31] */								\
	PUTDST8KNOWN(Dest, 0xA1)								\
	PUTDST32(Dest, (uint32)&COP1Con[31])					\
															\
	/* and eax, 0x00800000h */								\
	PUTDST8KNOWN(Dest, 0x25)								\
	PUTDST32KNOWN(Dest, 0x00800000)							\
															\
	/* test eax, eax */										\
	PUTDST16KNOWN(Dest, 0xC085)								\
															\
	NearJumpConditionLabel8(Dest, LabelFalse, OPERATION)	\
	PrevLabelFalse = (uint32)Dest; /*Mark*/					\
															\
	JumpLocal8(Dest, LabelTrue)								\
	PrevLabelTrue = (uint32)Dest; /*Mark*/
#else
#define BC1(OPERATION)										\
	uint32 PrevLabelTrue,	PrevLabelFalse;					\
	uint8 *LabelTrue,		*LabelFalse;					\
															\
	/* mov eax, COP1Con[31] */								\
	PUTDST8KNOWN(Dest, 0xA1)								\
	PUTDST32(Dest, (uint32)&COP1Con[31])					\
															\
	/* and eax, 0x00800000h */								\
	PUTDST8KNOWN(Dest, 0x25)								\
	PUTDST32KNOWN(Dest, 0x00800000)							\
															\
	/* test eax, eax */										\
	PUTDST16KNOWN(Dest, 0xC085)								\
															\
	NearJumpConditionLabel8(Dest, LabelFalse, OPERATION)	\
	PrevLabelFalse = (uint32)Dest; /*Mark*/					\
															\
	JumpLocal8(Dest, LabelTrue)								\
	PrevLabelTrue = (uint32)Dest; /*Mark*/
#endif

RETURN_TYPE dyn_r4300i_COP1_bc1f(OP_PARAMS) {	BC1(JNE_NEAR)	SETUP_DELAY_SLOTS_UNLIKELY_TF	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_bc1fl(OP_PARAMS){	BC1(JNE_NEAR)	SETUP_DELAY_SLOTS_LIKELY_TF		return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_bc1t(OP_PARAMS) {	BC1(JE_NEAR)	SETUP_DELAY_SLOTS_UNLIKELY_TF	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_bc1tl(OP_PARAMS){	BC1(JE_NEAR)	SETUP_DELAY_SLOTS_LIKELY_TF		return(Dest);}


RETURN_TYPE dyn_r4300i_COP0_eret(OP_PARAMS)
{
//	C_CALL(Dest, r4300i_COP0_eret); AsmReturn KEEP_RECOMPILING = 0;

/*
1656:     if ((COP0Reg[STATUS] & 0x00000004))
00416E33 8B 15 B0 AA 85 00    mov         edx,dword ptr [_COP0Reg+30h (0085aab0)]
00416E39 83 E2 04             and         edx,4
00416E3C 85 D2                test        edx,edx
00416E3E 74 1B                je          dyn_r4300i_COP0_eret+6Bh (00416e5b)
*/
PUTDST16KNOWN(Dest, 0x158b)		PUTDST32(Dest, (uint32)&COP0Reg[STATUS])
PUTDST32KNOWN(Dest, 0x8504e283)	PUTDST16KNOWN(Dest, 0x74d2)
PUTDST8KNOWN(Dest, 0x1b)
/*
1657:     {
1658:         pc = COP0Reg[ERROREPC];
00416E40 A1 F8 AA 85 00       mov         eax,[_COP0Reg+78h (0085aaf8)]
00416E45 A3 24 AF C5 00       mov         [_pc (00c5af24)],eax
*/
PUTDST8KNOWN(Dest, 0xa1)		PUTDST32(Dest, (uint32)&COP0Reg[ERROREPC])
PUTDST8KNOWN(Dest, 0xa3)		PUTDST32(Dest, (uint32)&pc)
/*
1661:         COP0Reg[STATUS] &= 0xFFFFFFFB; // 0xFFFFFFFB same as ~0x00000004;
00416E4A 8B 0D B0 AA 85 00    mov         ecx,dword ptr [_COP0Reg+30h (0085aab0)]
00416E50 83 E1 FB             and         ecx,0FBh
00416E53 89 0D B0 AA 85 00    mov         dword ptr [_COP0Reg+30h (0085aab0)],ecx
*/
PUTDST16KNOWN(Dest, 0x0d8b)		PUTDST32(Dest, (uint32)&COP0Reg[STATUS])
PUTDST16KNOWN(Dest, 0xe183)		PUTDST8KNOWN(Dest, 0xfb)
PUTDST16KNOWN(Dest, 0x0d89)		PUTDST32(Dest, (uint32)&COP0Reg[STATUS])
/*
1662:     }
1663:     else
00416E59 EB 18                jmp         dyn_r4300i_COP0_eret+83h (00416e73)
1664:     {
1665:         pc = COP0Reg[EPC];
00416E5B 8B 15 B8 AA 85 00    mov         edx,dword ptr [_COP0Reg+38h (0085aab8)]
00416E61 89 15 24 AF C5 00    mov         dword ptr [_pc (00c5af24)],edx
*/
PUTDST32KNOWN(Dest, 0x158b18eb)	PUTDST32(Dest, (uint32)&COP0Reg[EPC])
PUTDST16KNOWN(Dest, 0x1589)		PUTDST32(Dest, (uint32)&pc)
/*
1668:         COP0Reg[STATUS] &= 0xFFFFFFFD; //0xFFFFFFFD same as ~0x00000002
00416E67 A1 B0 AA 85 00       mov         eax,[_COP0Reg+30h (0085aab0)]
00416E6C 24 FD                and         al,0FDh
00416E6E A3 B0 AA 85 00       mov         [_COP0Reg+30h (0085aab0)],eax
1669:     }
1670:     LLbit   = 0;
00416E73 C7 05 68 AA 85 00 00 mov         dword ptr [_LLbit (0085aa68)],0
1671:
*/
PUTDST8KNOWN(Dest, 0xa1)		PUTDST32(Dest, (uint32)&COP0Reg[STATUS])
PUTDST16KNOWN(Dest, 0xfd24)		PUTDST8KNOWN(Dest, 0xa3)
PUTDST32(Dest, (uint32)&COP0Reg[STATUS])
PUTDST16KNOWN(Dest, 0x05c7)		PUTDST32(Dest, (uint32)&LLbit)	PUTDST32KNOWN(Dest, 0)

	AsmReturn
	KEEP_RECOMPILING = 0;
	return(Dest);
}

#ifndef SAFE_DYNAREC

RETURN_TYPE dyn_r4300i_add(OP_PARAMS)	{sLOGICAL_DYN(sADD_DYN); return(Dest);}
RETURN_TYPE dyn_r4300i_addu(OP_PARAMS)	{sLOGICAL_DYN(sADD_DYN); return(Dest);}
RETURN_TYPE dyn_r4300i_sub(OP_PARAMS)	{sLOGICAL_DYN(sSUB_DYN); return(Dest);}
RETURN_TYPE dyn_r4300i_subu(OP_PARAMS)	{sLOGICAL_DYN(sSUB_DYN); return(Dest);}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_and(OP_PARAMS)		{uDLOGICAL_DYN(uDAND_DYN, uDAND_DYN2); return(Dest);}
RETURN_TYPE dyn_r4300i_or(OP_PARAMS)		{uDLOGICAL_DYN(uDOR_DYN,  uDOR_DYN2);  return(Dest);}
RETURN_TYPE dyn_r4300i_xor(OP_PARAMS)		{uDLOGICAL_DYN(uDXOR_DYN, uDXOR_DYN2); return(Dest);}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_nor(OP_PARAMS)		{

	uint32 RTAddr = (uint32)&GPR[RT_FT];
	uint32 RDAddr = (uint32)&GPR[RD_FS];
	uint32 RSAddr = (uint32)&GPR[RS_BASE_FMT];

//00416DC9 A1 A0 7A 4A 00       mov         eax,[_GPR+20h (004a7aa0)]
//00416DCE F7 D0                not         eax
//00416DD0 8B 0D A4 7A 4A 00    mov         ecx,dword ptr [_GPR+24h (004a7aa4)]
//00416DD6 F7 D1                not         ecx
//00416DD8 8B 15 90 7A 4A 00    mov         edx,dword ptr [_GPR+10h (004a7a90)]
//00416DDE 0B D0                or          edx,eax
//00416DE0 A1 94 7A 4A 00       mov         eax,[_GPR+14h (004a7a94)]
//00416DE5 0B C1                or          eax,ecx
//00416DE7 89 15 80 7A 4A 00    mov         dword ptr [_GPR (004a7a80)],edx
//00416DED A3 84 7A 4A 00       mov         [_GPR+4 (004a7a84)],eax

	PUTDST8KNOWN(Dest, 0xA1)			PUTDST32(Dest, RTAddr)
	PUTDST32KNOWN(Dest, 0x0D8BD0F7)		PUTDST32(Dest, RTAddr+4)
	PUTDST32KNOWN(Dest, 0x158BD1F7)		PUTDST32(Dest, RSAddr)
	PUTDST16KNOWN(Dest, 0xD00B)			PUTDST8KNOWN(Dest, 0xA1)	PUTDST32(Dest, RSAddr+4)
	PUTDST32KNOWN(Dest, 0x1589C10B)		PUTDST32(Dest, RDAddr)
	PUTDST8KNOWN(Dest, 0xA3)			PUTDST32(Dest, RDAddr+4)

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_addi(OP_PARAMS)		{sLOGICAL_WITH_IMM_DYN(sADDI_DYN) return(Dest);}
RETURN_TYPE dyn_r4300i_addiu(OP_PARAMS)		{sLOGICAL_WITH_IMM_DYN(sADDI_DYN) return(Dest);}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_lb(OP_PARAMS){	INTERPRET(r4300i_lb)	return(Dest);}
RETURN_TYPE dyn_r4300i_lh(OP_PARAMS){	INTERPRET(r4300i_lh)	return(Dest);}
RETURN_TYPE dyn_r4300i_lbu(OP_PARAMS){	INTERPRET(r4300i_lbu)	return(Dest);}
RETURN_TYPE dyn_r4300i_lhu(OP_PARAMS){	INTERPRET(r4300i_lhu)	return(Dest);}
RETURN_TYPE dyn_r4300i_ld(OP_PARAMS){	INTERPRET(r4300i_ld)	return(Dest);}
RETURN_TYPE dyn_r4300i_ldc1(OP_PARAMS){	INTERPRET(r4300i_ldc1)	return(Dest);}
RETURN_TYPE dyn_r4300i_sd(OP_PARAMS){	INTERPRET(r4300i_sd)	return(Dest);}
RETURN_TYPE dyn_r4300i_sdc1(OP_PARAMS){	INTERPRET(r4300i_sdc1)	return(Dest);}
RETURN_TYPE dyn_r4300i_lw(OP_PARAMS){	INTERPRET(r4300i_lw)	return(Dest);}
RETURN_TYPE dyn_r4300i_sw(OP_PARAMS){	INTERPRET(r4300i_sw)	return(Dest);}
RETURN_TYPE dyn_r4300i_swc1(OP_PARAMS){	INTERPRET(r4300i_swc1)	return(Dest);}
RETURN_TYPE dyn_r4300i_sb(OP_PARAMS){	INTERPRET(r4300i_sb)	return(Dest);}
RETURN_TYPE dyn_r4300i_sh(OP_PARAMS){	INTERPRET(r4300i_sh)	return(Dest);}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_slti(OP_PARAMS)
{
#ifdef WINDEBUG_1964
	INTERPRET(r4300i_slti)
#else
	
	_int64 ConstInt		= (_int64)OFFSET_IMMEDIATE;
	uint32 RTAddr		= (uint32)&GPR[RT_FT];
	uint32 RTAddrPlus4	= RTAddr+4;
	uint32 RSAddr		= (uint32)&GPR[RS_BASE_FMT];

//004119E7 81 3D E4 01 4B 00 11 cmp         dword ptr [_GPR+4 (004b01e4)],11111111h
//004119F1 7F 1A                jg          dyn_r4300i_slti+47h (00411a17)
//004119F3 7C 0C                jl          dyn_r4300i_slti+31h (00411a01)
//004119F5 81 3D E0 01 4B 00 88 cmp         dword ptr [_GPR (004b01e0)],88888888h
//004119FF 73 0C                jae         dyn_r4300i_slti+47h (00411a17)
//00411A01 C7 05 E8 01 4B 00 01 mov         dword ptr [_GPR+8 (004b01e8)],1
//00411A15 EB 0A                jmp         dyn_r4300i_slti+5Bh (00411a2b)
//00411A17 C7 05 E8 01 4B 00 00 mov         dword ptr [_GPR+8 (004b01e8)],0
//00411A21 C7 05 EC 01 4B 00 00 mov         dword ptr [_GPR+0Ch (004b01ec)],0
	 PUTDST16KNOWN(Dest, 0x3D81)			PUTDST32(Dest, RSAddr+4)	PUTDST32(Dest, (ConstInt >> 32))
	PUTDST32KNOWN(Dest, 0x0C7C1A7F)

	PUTDST16KNOWN(Dest, 0x3D81)				PUTDST32(Dest, RSAddr)		PUTDST32(Dest, (uint32)ConstInt)
	PUTDST32KNOWN(Dest, 0x05C70C73)			PUTDST32(Dest, RTAddr)		PUTDST32KNOWN(Dest, 0x1)

	PUTDST32KNOWN(Dest, 0x05C70AEB)			PUTDST32(Dest, RTAddr)		PUTDST32KNOWN(Dest, 0x0)
	PUTDST16KNOWN(Dest, 0x05C7)				PUTDST32(Dest, RTAddrPlus4)	PUTDST32KNOWN(Dest, 0x0)

  #endif
	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_sltiu(OP_PARAMS)
{
#ifdef WINDEBUG_1964
  INTERPRET(r4300i_sltiu)
#else
	uint64 Const64		= (uint64)(_int64)OFFSET_IMMEDIATE;
	uint32 RSAddr		= (uint32)&GPR[RS_BASE_FMT];
	uint32 RTAddr		= (uint32)&GPR[RT_FT];
	uint32 RTAddrPlus4	= RTAddr+4;

//0041A797 81 3D 84 7A 4A 00 11 cmp         dword ptr [_GPR+4 (004a7a84)],11111111h
//0041A7A1 77 1A                ja          dyn_r4300i_sltiu+47h (0041a7c7)
//0041A7A3 72 0C                jb          dyn_r4300i_sltiu+31h (0041a7b1)
//0041A7A5 81 3D 80 7A 4A 00 88 cmp         dword ptr [_GPR (004a7a80)],88888888h
//0041A7AF 73 0C                jae         dyn_r4300i_sltiu+47h (0041a7c7)
//0041A7B1 C7 05 90 7A 4A 00 01 mov         dword ptr [_GPR+10h (004a7a90)],1
//0041A7C5 EB 0A                jmp         dyn_r4300i_sltiu+5Bh (0041a7db)
//0041A7C7 C7 05 90 7A 4A 00 00 mov         dword ptr [_GPR+10h (004a7a90)],0
//0041A7D1 C7 05 94 7A 4A 00 00 mov         dword ptr [_GPR+14h (004a7a94)],0

	PUTDST16KNOWN(Dest, 0x3D81)			PUTDST32(Dest, RSAddr+4)	PUTDST32(Dest, (uint32)(Const64 >> 32))
	PUTDST32KNOWN(Dest, 0x0C721A77)	
	PUTDST16KNOWN(Dest, 0x3D81)			PUTDST32(Dest, RSAddr)		PUTDST32(Dest, (uint32)(Const64))
	
	PUTDST32KNOWN(Dest, 0x05C70C73)		PUTDST32(Dest, RTAddr)		PUTDST32KNOWN(Dest, 0x1)

	PUTDST32KNOWN(Dest, 0x05C70AEB)		PUTDST32(Dest, RTAddr)		PUTDST32KNOWN(Dest, 0x0)
	PUTDST16KNOWN(Dest, 0x05C7)			PUTDST32(Dest, RTAddrPlus4)	PUTDST32KNOWN(Dest, 0x0)
#endif
	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_sltu(OP_PARAMS)
{
#ifdef WINDEBUG_1964
	INTERPRET(r4300i_sltu)
#else
	uint32 RSAddr		= (uint32)&GPR[RS_BASE_FMT];
	uint32 RTAddr		= (uint32)&GPR[RT_FT];
	uint32 RDAddr		= (uint32)&GPR[RD_FS];
	uint32 RDAddrPlus4	= RDAddr+4;
	
//0041A6C9 A1 84 7A 4A 00       mov         eax,[_GPR+4 (004a7a84)]
//0041A6CE 3B 05 94 7A 4A 00    cmp         eax,dword ptr [_GPR+14h (004a7a94)]
//0041A6D4 77 1C                ja          dyn_r4300i_sltu+3Ch (0041a6fc)
//0041A6D6 72 0E                jb          dyn_r4300i_sltu+26h (0041a6e6)
//0041A6D8 8B 0D 80 7A 4A 00    mov         ecx,dword ptr [_GPR (004a7a80)]
//0041A6DE 3B 0D 90 7A 4A 00    cmp         ecx,dword ptr [_GPR+10h (004a7a90)]
//0041A6E4 73 16                jae         dyn_r4300i_sltu+3Ch (0041a6fc)
//0041A6E6 C7 05 80 7B 4A 00 01 mov         dword ptr [GPR+XX (004a7b80)],1
//0041A6FA EB 14                jmp         dyn_r4300i_sltu+50h (0041a710)
//0041A6FC C7 05 80 7B 4A 00 00 mov         dword ptr [GPR+XX (004a7b80)],0
//0041A706 C7 05 84 7B 4A 00 00 mov         dword ptr [GPR+XX+4 (004a7b84)],0

	PUTDST8KNOWN(Dest, 0xA1)			PUTDST32(Dest, RSAddr+4)
	PUTDST16KNOWN(Dest, 0x053B)			PUTDST32(Dest, RTAddr+4)
	PUTDST32KNOWN(Dest, 0x0E721C77)		

	PUTDST16KNOWN(Dest, 0x0D8B)			PUTDST32(Dest, RSAddr)
	PUTDST16KNOWN(Dest, 0x0D3B)			PUTDST32(Dest, RTAddr)

	PUTDST32KNOWN(Dest, 0x05C70C73)		PUTDST32(Dest, RDAddr)			PUTDST32KNOWN(Dest, 0x1)

	PUTDST32KNOWN(Dest, 0x05C70AEB)		PUTDST32(Dest, RDAddr)			PUTDST32KNOWN(Dest, 0x0)
	PUTDST16KNOWN(Dest, 0x05C7)			PUTDST32(Dest, RDAddrPlus4)		PUTDST32KNOWN(Dest, 0x0)
#endif
	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_srav(OP_PARAMS)
{	
	uint32 RDAddr = (uint32)&GPR[RD_FS];

//0041211F A1 90 7A 4A 00       mov         eax,[_GPR+10h (004a7a90)]
//00412124 8A 0D A0 7A 4A 00    mov         cl,byte ptr [_GPR+20h (004a7aa0)]
//0041212A D3 F8                sar         eax,cl
//0041212C 99                   cdq
//0041212D A3 80 7A 4A 00       mov         [_GPR (004a7a80)],eax
//00412132 89 15 84 7A 4A 00    mov         dword ptr [_GPR+4 (004a7a84)],edx

	 PUTDST8KNOWN(Dest, 0xA1)			PUTDST32(Dest, (uint32)&GPR[RT_FT])
	PUTDST16KNOWN(Dest, 0x0D8A)			PUTDST32(Dest, (uint32)&GPR[RS_BASE_FMT])
	
	PUTDST16KNOWN(Dest, 0xF8D3)
	CDQ_STORE(Dest, RDAddr)
	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_slt(OP_PARAMS)		{

	uint32 RSAddr = (uint32)&GPR[RS_BASE_FMT];
	uint32 RTAddr = (uint32)&GPR[RT_FT];
	uint32 RDAddr = (uint32)&GPR[RD_FS];
	uint32 RDAddrPlus4 = RDAddr + 4;

//	004260A9 A1 E4 51 4B 00       mov         eax,[_GPR+4 (004b51e4)]
//	004260AE 3B 05 EC 51 4B 00    cmp         eax,dword ptr [_GPR+0Ch (004b51ec)]
//	004260B4 7F 1C                jg          r4300i_slt+3Ch (004260dc)
//	004260B6 7C 0E                jl          r4300i_slt+26h (004260c6)
//	004260B8 8B 0D E0 51 4B 00    mov         ecx,dword ptr [_GPR (004b51e0)]
//	004260BE 3B 0D E8 51 4B 00    cmp         ecx,dword ptr [_GPR+8 (004b51e8)]
//	004260C4 73 0C                jae         r4300i_slt+3Ch (004260dc)
//	004260C6 C7 05 F0 51 4B 00 01 mov         dword ptr [_GPR+10h (004b51f0)],1
//	004260DA EB 0A                jmp         r4300i_slt+50h (004260f0)
//	004260DC C7 05 F8 51 4B 00 00 mov         dword ptr [_GPR+18h (004b51f8)],0
//	004260E6 C7 05 FC 51 4B 00 00 mov         dword ptr [_GPR+1Ch (004b51fc)],0

	PUTDST8KNOWN(Dest, 0xA1)				PUTDST32(Dest, RSAddr+4)
	PUTDST16KNOWN(Dest, 0x053B)				PUTDST32(Dest, RTAddr+4)
	PUTDST32KNOWN(Dest, 0x0E7C1C7F)

	PUTDST16KNOWN(Dest, 0x0D8B)				PUTDST32(Dest, RSAddr)
	PUTDST16KNOWN(Dest, 0x0D3B)				PUTDST32(Dest, RTAddr)

	PUTDST32KNOWN(Dest, 0x05C70C73)			PUTDST32(Dest, RDAddr)		PUTDST32KNOWN(Dest, 0x01)

	PUTDST32KNOWN(Dest, 0x05C70AEB)			PUTDST32(Dest, RDAddr)		PUTDST32KNOWN(Dest, 0x00)
	PUTDST16KNOWN(Dest, 0x05C7)				PUTDST32(Dest, RDAddrPlus4)	PUTDST32KNOWN(Dest, 0x00)
	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_sll(OP_PARAMS)
{
	if (Instruction != 0)
	{
//0040A0C4 A1 E0 01 4B 00       mov         eax,[_GPR (004b01e0)]
//0040A0C9 C1 E0 1F             shl         eax,XXh<-(SA_FD)
//0040A0CC 99                   cdq
//0040A0CD A3 E8 01 4B 00       mov         [_GPR+8 (004b01e8)],eax
//0040A0D2 89 15 EC 01 4B 00    mov         dword ptr [_GPR+0Ch (004b01ec)],edx

	PUTDST8KNOWN(Dest, 0xA1)			PUTDST32(Dest, (uint32)&GPR[RT_FT])
	PUTDST16KNOWN(Dest, 0xE0C1)			PUTDST8(Dest, SA_FD)

	CDQ_STORE(Dest, (uint32)&GPR[RD_FS])
	}
	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_sra(OP_PARAMS)
{	
	uint8 ShiftAmt = (uint8)(SA_FD);
	uint32 RDAddr = (uint32)&GPR[RD_FS];
	
//00412DE7 A1 E8 01 4B 00       mov         eax,[_GPR+8 (004b01e8)]
//00411D5D C1 F8 20             sar         eax,20h
//00412DEF 99                   cdq
//00412DF0 A3 E0 01 4B 00       mov         [_GPR (004b01e0)],eax
//00412DF5 89 15 E4 01 4B 00    mov         dword ptr [_GPR+4 (004b01e4)],edx

	 PUTDST8KNOWN(Dest, 0xA1)			PUTDST32(Dest, (uint32)&GPR[RT_FT])
	PUTDST16KNOWN(Dest, 0xF8C1)			PUTDST8(Dest, ShiftAmt)
	
	CDQ_STORE(Dest, RDAddr)
	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_srl(OP_PARAMS)
{

	uint32 RDAddr = (uint32)&GPR[RD_FS];

//0040A0C4 A1 E0 01 4B 00       mov         eax,[_GPR (004b01e0)]
//00411B89 C1 E8 1F             shr         eax,1Fh
//0040A0CC 99                   cdq
//0040A0CD A3 E8 01 4B 00       mov         [_GPR+8 (004b01e8)],eax
//0040A0D2 89 15 EC 01 4B 00    mov         dword ptr [_GPR+0Ch (004b01ec)],edx

	PUTDST8KNOWN(Dest, 0xA1)			PUTDST32(Dest, (uint32)&GPR[RT_FT])
	PUTDST16KNOWN(Dest, 0xE8C1)			PUTDST8(Dest, SA_FD)

	CDQ_STORE(Dest, RDAddr)
	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_sllv(OP_PARAMS)		{	

	uint32 RDAddr = (uint32)&GPR[RD_FS];

//0040C87E A1 E8 01 4B 00       mov         eax,[_GPR+8 (004b01e8)]
//0040C883 8A 0D F0 01 4B 00    mov         cl,byte ptr [_GPR+10h (004b01f0)]
//0040C889 D3 E0                shl         eax,cl
//0040C88B 99                   cdq
//0040C88C A3 E0 01 4B 00       mov         [_GPR (004b01e0)],eax
//0040C891 89 15 E4 01 4B 00    mov         dword ptr [_GPR+4 (004b01e4)],edx


	 PUTDST8KNOWN(Dest, 0xA1)			PUTDST32(Dest, (uint32)&GPR[RT_FT])
	PUTDST16KNOWN(Dest, 0x0D8A)			PUTDST32(Dest, (uint32)&GPR[RS_BASE_FMT])
	
	PUTDST16KNOWN(Dest, 0xE0D3)
	CDQ_STORE(Dest, RDAddr)
	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_srlv(OP_PARAMS)		
{
	uint32 RDAddr = (uint32)&GPR[RD_FS];

//0040B869 A1 E8 01 4B 00       mov         eax,[_GPR+8 (004b01e8)]
//0040B86E 8A 0D F0 01 4B 00    mov         cl,byte ptr [_GPR+10h (004b01f0)]
//0040B874 D3 E8                shr         eax,cl
//0040B876 99                   cdq
//0040B877 A3 E0 01 4B 00       mov         [_GPR (004b01e0)],eax
//0040B87C 89 15 E4 01 4B 00    mov         dword ptr [_GPR+4 (004b01e4)],edx

	PUTDST8KNOWN(Dest, 0xA1)			PUTDST32(Dest, (uint32)&GPR[RT_FT])
	PUTDST16KNOWN(Dest, 0x0D8A)			PUTDST32(Dest, (uint32)&GPR[RS_BASE_FMT])
	
	PUTDST16KNOWN(Dest, 0xE8D3)
	CDQ_STORE(Dest, RDAddr)
	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_div(OP_PARAMS)
{	
	uint32 PrevLabelTrue;
	uint8 *LabelTrue;
	uint32 RSAddr = (uint32)&GPR[RS_BASE_FMT];
	uint32 RTAddr = (uint32)&GPR[RT_FT];
//00407E26 83 3D 80 E1 A8 00 00 cmp         dword ptr [_GPR (00a8e180)],0
//00407E2D 74 48                je          r4300i_div+77h (00407e77)

//004487C4 A1 E0 01 4B 00       mov         eax,[_GPR (004b01e0)]
//004487C9 8B 0D E8 01 4B 00    mov         ecx,dword ptr [_GPR+8 (004b01e8)]
//004487CF 99                   cdq
//00412F5C F7 F9                idiv        eax,ecx
//004487C4 8B DA                mov         ebx,edx
//004487D2 99                   cdq
//004487D3 A3 50 01 4B 00       mov         [_LO (004b0150)],eax
//004487D8 89 15 54 01 4B 00    mov         dword ptr [_LO+4 (004b0154)],edx
//004487DE 8B C3                mov         eax,ebx
//004487E0 99                   cdq
//004487E1 A3 30 01 4B 00       mov         [_HI (004b0130)],eax
//004487E6 89 15 34 01 4B 00    mov         dword ptr [_HI+4 (004b0134)],edx

	PUTDST16KNOWN(Dest, 0x3D83)		PUTDST32(Dest, RTAddr)		PUTDST8KNOWN(Dest, 0x00)
	NearJumpConditionLabel8(Dest, LabelTrue, JE_NEAR)
	PrevLabelTrue = (uint32)Dest;

	PUTDST8KNOWN(Dest, 0xA1)			PUTDST32(Dest, RSAddr)
	PUTDST16KNOWN(Dest, 0x0D8B)			PUTDST32(Dest, RTAddr)
	PUTDST32KNOWN(Dest, 0x8BF9F799)		
	PUTDST16KNOWN(Dest, 0x99DA)
	CDQ_STORE(Dest, (uint32)&LO)
	PUTDST16KNOWN(Dest, 0xC38B)
	CDQ_STORE(Dest, (uint32)&HI)

*LabelTrue  = (uint8)(Dest - PrevLabelTrue);
	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_divu(OP_PARAMS)
{
	uint32 PrevLabelTrue;
	uint8 *LabelTrue;
	uint32 RSAddr = (uint32)&GPR[RS_BASE_FMT];
	uint32 RTAddr = (uint32)&GPR[RT_FT];

//00407E26 83 3D 80 E1 A8 00 00 cmp         dword ptr [_GPR (00a8e180)],0
//00407E2D 74 48                je          r4300i_div+77h (00407e77)

//004487C4 A1 E0 01 4B 00       mov         eax,[_GPR (004b01e0)]
//004487C9 8B 0D E8 01 4B 00    mov         ecx,dword ptr [_GPR+8 (004b01e8)]
//00418FED 33 D2                xor         edx,edx
//00412F5C F7 F1                div         eax,ecx
//004487C4 8B DA                mov         ebx,edx
//004487D2 99                   cdq
//004487D3 A3 50 01 4B 00       mov         [_LO (004b0150)],eax
//004487D8 89 15 54 01 4B 00    mov         dword ptr [_LO+4 (004b0154)],edx
//004487DE 8B C3                mov         eax,ebx
//004487E0 99                   cdq
//004487E1 A3 30 01 4B 00       mov         [_HI (004b0130)],eax
//004487E6 89 15 34 01 4B 00    mov         dword ptr [_HI+4 (004b0134)],edx

	PUTDST16KNOWN(Dest, 0x3D83)		PUTDST32(Dest, RTAddr)		PUTDST8KNOWN(Dest, 0x00)
	NearJumpConditionLabel8(Dest, LabelTrue, JE_NEAR)
	PrevLabelTrue = (uint32)Dest;

	PUTDST8KNOWN(Dest, 0xA1)		PUTDST32(Dest, RSAddr)
	PUTDST16KNOWN(Dest, 0x0D8B)		PUTDST32(Dest, RTAddr)

	PUTDST16KNOWN(Dest, 0xD233)
	PUTDST16KNOWN(Dest, 0xF1F7)		PUTDST8KNOWN(Dest, 0x8B)
	PUTDST16KNOWN(Dest, 0x99DA)
	CDQ_STORE(Dest, (uint32)&LO)
	PUTDST16KNOWN(Dest, 0xC38B)
	CDQ_STORE(Dest, (uint32)&HI)

*LabelTrue  = (uint8)(Dest - PrevLabelTrue);
	return(Dest);
}

//---------------------------------------------------------------------------------------

//004223A9 A1 30 01 4B 00       mov         eax,[_HI (004b0130)]
//004223AE A3 E0 01 4B 00       mov         [_GPR (004b01e0)],eax
//004223B3 8B 0D 34 01 4B 00    mov         ecx,dword ptr [_HI+4 (004b0134)]
//004223B9 89 0D E4 01 4B 00    mov         dword ptr [_GPR+4 (004b01e4)],ecx
#define MOVE_TO_FROM_HI_LO(Dest, HILO, REG) {					\
	uint32 RegAddr = (uint32)&REG;								\
	uint32 HILOAddr = (uint32)&HILO;							\
	PUTDST8KNOWN(Dest, 0xA1)	PUTDST32(Dest, HILOAddr)		\
	PUTDST8KNOWN(Dest, 0xA3)	PUTDST32(Dest, RegAddr)			\
	PUTDST16KNOWN(Dest, 0x0D8B)	PUTDST32(Dest, HILOAddr+4)		\
	PUTDST16KNOWN(Dest, 0x0D89)	PUTDST32(Dest, RegAddr+4)		\
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_mfhi(OP_PARAMS)	{MOVE_TO_FROM_HI_LO(Dest, HI, GPR[RD_FS]) return(Dest);}	
RETURN_TYPE dyn_r4300i_mflo(OP_PARAMS)	{MOVE_TO_FROM_HI_LO(Dest, LO, GPR[RD_FS]) return(Dest);}
RETURN_TYPE dyn_r4300i_mthi(OP_PARAMS)	{MOVE_TO_FROM_HI_LO(Dest, HI, GPR[RS_BASE_FMT]) return(Dest);}
RETURN_TYPE dyn_r4300i_mtlo(OP_PARAMS)	{MOVE_TO_FROM_HI_LO(Dest, LO, GPR[RS_BASE_FMT]) return(Dest);}


//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_COP1_add_s(OP_PARAMS){	fsingleLOGICAL(0x05D8) return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_sub_s(OP_PARAMS){	fsingleLOGICAL(0x25D8) return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_mul_s(OP_PARAMS){	fsingleLOGICAL(0x0DD8) return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_div_s(OP_PARAMS){	fsingleLOGICAL(0x35D8) return(Dest);}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_COP1_add_d(OP_PARAMS){	fdoubleLOGICAL(0x05DC) return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_sub_d(OP_PARAMS){	fdoubleLOGICAL(0x25DC) return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_mul_d(OP_PARAMS){	fdoubleLOGICAL(0x0DDC) return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_div_d(OP_PARAMS){	fdoubleLOGICAL(0x35DC) return(Dest);}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_COP0_mfc0(OP_PARAMS) 
{
	uint32 COP_RDAddr = (uint32)&COP0Reg[RD_FS];
	uint32 RTAddr     = (uint32)&GPR[RT_FT];

//0041CCC9 A1 E0 5E 4A 00       mov         eax,[_COP0Reg (004a5ee0)]
//0041CCCE 99                   cdq
//0041CCCF A3 80 61 4A 00       mov         [_GPR (004a6180)],eax
//0041CCD4 89 15 84 61 4A 00    mov         dword ptr [_GPR+4 (004a6184)],edx

	PUTDST8KNOWN(Dest, 0xA1)			PUTDST32(Dest, COP_RDAddr)
	CDQ_STORE(Dest, RTAddr)
	
	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_COP0_mtc0(OP_PARAMS)
{
	uint32 rt_ft = RT_FT;
	uint32 RTAddr     = (uint32)&GPR[rt_ft];

//0041CDE9 A1 80 61 4A 00       mov         eax,[_GPR (004a6180)]
//0041CDEE A3 E0 5E 4A 00       mov         [_COP0Reg (004a5ee0)],eax

	if (rt_ft == 0) {
		PUTDST16KNOWN(Dest, 0x05C7)
		PUTDST32(Dest, RTAddr)
		PUTDST32(Dest, 0x00000000)
	}
	else
	{
		PUTDST8KNOWN(Dest, 0xA1)		PUTDST32(Dest, RTAddr)
		PUTDST8KNOWN(Dest, 0xA3)		PUTDST32(Dest, 	(uint32)&COP0Reg[RD_FS])
	}
	return(Dest);
}

//---------------------------------------------------------------------------------------

//-----------------------------------
//  uDLOGICAL_WITH_IMMEDIATE Ops
//-----------------------------------
RETURN_TYPE dyn_r4300i_andi(OP_PARAMS)		
{	
	uint32 RSAddr = (uint32)&GPR[RS_BASE_FMT];
	uint32 RTAddr = (uint32)&GPR[RT_FT];
//See uDLOGICAL_WITH_IMM Macro

//0041B139 A1 80 51 4A 00       mov         eax,[_GPR (004a5180)]
//0041B13E 25 78 56 00 00       and         eax,5678h

//00412B24 33 C9                xor         ecx,ecx
//0041B14C A3 80 51 4A 00       mov         [_GPR (004a5180)],eax
//0041B151 89 0D 84 51 4A 00    mov         dword ptr [_GPR+4 (004a5184)],ecx

	PUTDST8KNOWN(Dest, 0xA1)			PUTDST32(Dest, RSAddr)
	PUTDST8KNOWN(Dest, 0x25)			PUTDST32(Dest, (uint32)(uint16)OFFSET_IMMEDIATE)

	PUTDST16KNOWN(Dest, 0xC933)
	PUTDST8KNOWN(Dest, 0xA3)			PUTDST32(Dest, RTAddr)
	PUTDST16KNOWN(Dest, 0x0D89)			PUTDST32(Dest, RTAddr+4)

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_xori(OP_PARAMS)
{

	uint32 RSAddr = (uint32)&GPR[RS_BASE_FMT];
	uint32 RTAddr = (uint32)&GPR[RT_FT];

//0041B639 A1 80 61 4A 00       mov         eax,[_GPR (004a6180)]
//0041B63E 35 78 56 00 00       xor         eax,5678h

//0041B643 8B 0D 84 61 4A 00    mov         ecx,dword ptr [_GPR+4 (004a6184)]
//0041B64C A3 80 61 4A 00       mov         [_GPR (004a6180)],eax
//0041B651 89 0D 84 61 4A 00    mov         dword ptr [_GPR+4 (004a6184)],ecx

	 PUTDST8KNOWN(Dest, 0xA1)			PUTDST32(Dest, RSAddr)
	 PUTDST8KNOWN(Dest, 0x35)			PUTDST32(Dest, (uint32)(uint16)OFFSET_IMMEDIATE)
//                 ----

	PUTDST16KNOWN(Dest, 0x0D8B)			PUTDST32(Dest, RSAddr+4)
	PUTDST8KNOWN(Dest, 0xA3)			PUTDST32(Dest, RTAddr)
	PUTDST16KNOWN(Dest, 0x0D89)			PUTDST32(Dest, RTAddr+4)

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_ori(OP_PARAMS)		
{	
	uint32 RSAddr = (uint32)&GPR[RS_BASE_FMT];
	uint32 RTAddr = (uint32)&GPR[RT_FT];
	
//See uDLOGICAL_WITH_IMM Macro

//0041B3D9 A1 80 61 4A 00       mov         eax,[_GPR (004a6180)]
//0041B3DE 0D 78 56 00 00       or          eax,5678h

//0041B3E3 8B 0D 84 61 4A 00    mov         ecx,dword ptr [_GPR+4 (004a6184)]

//0041B3E9 A3 80 61 4A 00       mov         [_GPR (004a6180)],eax
//0041B3EE 89 0D 84 61 4A 00    mov         dword ptr [_GPR+4 (004a6184)],ecx

	 PUTDST8KNOWN(Dest, 0xA1)			PUTDST32(Dest, RSAddr)
	 PUTDST8KNOWN(Dest, 0x0D)			PUTDST32(Dest, (uint32)(uint16)OFFSET_IMMEDIATE)
//                 ----

	PUTDST16KNOWN(Dest, 0x0D8B)		PUTDST32(Dest, RSAddr+4)
	 PUTDST8KNOWN(Dest, 0xA3)		PUTDST32(Dest, RTAddr)
	PUTDST16KNOWN(Dest, 0x0D89)		PUTDST32(Dest, RTAddr+4)

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_lui(OP_PARAMS) 
{

	uint32 RTAddr = (uint32)&GPR[RT_FT];
	
	uint32 Imm[2];

	*(_int64*)Imm = OFFSET_IMMEDIATE << 16;

//00402F89 C7 05 80 61 49 00 00 00 00 00 mov         dword ptr [_GPR (00496180)],0
//00402F93 C7 05 84 61 49 00 11 11 11 11 mov         dword ptr [_GPR+4 (00496184)],11111111h

	PUTDST16KNOWN(Dest, 0x05C7)	PUTDST32(Dest, RTAddr)		PUTDST32(Dest, Imm[0])
	PUTDST16KNOWN(Dest, 0x05C7)	PUTDST32(Dest, RTAddr+4)	PUTDST32(Dest, Imm[1])

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_COP1_mtc1(OP_PARAMS)
{

//0041D1D9 A1 E8 51 4B 00       mov         eax,[_GPR+8 (004b51e8)]
//0041D1DE A3 20 50 4B 00       mov         [_COP1Reg (004b5020)],eax
	PUTDST8KNOWN(Dest, 0xA1)		PUTDST32(Dest, (uint32)&GPR[RT_FT])
	PUTDST8KNOWN(Dest, 0xA3)		PUTDST32(Dest, (uint32)&COP1Reg[RD_FS])

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_COP1_neg_s(OP_PARAMS)
{

//00415419 D9 05 28 80 4A 00    fld         dword ptr [_COP1Reg+8 (004a8028)]
//0041541F D9 E0                fchs
//00415421 D9 1D 20 80 4A 00    fstp        dword ptr [_COP1Reg (004a8020)]

	PUTDST16KNOWN(Dest, 0x05D9)		PUTDST32(Dest, (uint32)&COP1Reg[RD_FS])
	PUTDST32KNOWN(Dest, 0x1DD9E0D9)	PUTDST32(Dest, (uint32)&COP1Reg[SA_FD])

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_COP1_mfc1(OP_PARAMS)
{
	uint32 RTAddr = (uint32)&GPR[RT_FT];

//00417A99 A1 24 00 4B 00       mov         eax,[_COP1Reg+4 (004b0024)]
//00417A9E 99                   cdq
//00417A9F A3 E0 01 4B 00       mov         [_GPR (004b01e0)],eax
//00417AA4 89 15 E4 01 4B 00    mov         dword ptr [_GPR+4 (004b01e4)],edx

	PUTDST8KNOWN(Dest, 0xA1)		PUTDST32(Dest, (uint32)&COP1Reg[RD_FS])
	CDQ_STORE(Dest, RTAddr)

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_C_LE_S(OP_PARAMS)
{	
	uint32 COP1ConAddr = (uint32)&COP1Con[31];
	
//0042E6A9 D9 05 88 DE 68 00    fld         dword ptr [_COP1Reg+8 (0068de88)]
//0042E6AF D8 1D 80 DE 68 00    fcomp       dword ptr [_COP1Reg (0068de80)]
//0042E6B5 DF E0                fnstsw      ax
//0042E6B7 F6 C4 41             test        ah,41h
//0042E6BA 74 11                je          dyn_r4300i_C_LE_S+2Dh (0042e6cd)
//0042E6BC A1 BC DC 68 00       mov         eax,[_COP1Con+7Ch (0068dcbc)]
//0042E6C1 0D 00 00 80 00       or          eax,offset _DynaRDRAM+171F80h (00800000)
//0042E6C6 A3 BC DC 68 00       mov         [_COP1Con+7Ch (0068dcbc)],eax
//1297:     else                                                            COP1Con[31] &=  0xFF7FFFFF;
//0042E6CB EB 12                jmp         dyn_r4300i_C_LE_S+3Fh (0042e6df)
//0042E6CD 8B 0D BC DC 68 00    mov         ecx,dword ptr [_COP1Con+7Ch (0068dcbc)]
//0042E6D3 81 E1 FF FF 7F FF    and         ecx,0FF7FFFFFh
//0042E6D9 89 0D BC DC 68 00    mov         dword ptr [_COP1Con+7Ch (0068dcbc)],ecx
	PUTDST16KNOWN(Dest, 0x05D9)			PUTDST32(Dest, (uint32)&COP1Reg[RD_FS])
	PUTDST16KNOWN(Dest, 0x1DD8)			PUTDST32(Dest, (uint32)&COP1Reg[RT_FT])
	PUTDST32KNOWN(Dest, 0xC4F6E0DF)		PUTDST32KNOWN(Dest, 0xA1117441)		PUTDST32(Dest, COP1ConAddr)
	PUTDST8KNOWN(Dest, 0x0D)			PUTDST32KNOWN(Dest, 0x00800000)
	PUTDST8KNOWN(Dest, 0xA3)			PUTDST32(Dest, COP1ConAddr)
	PUTDST32KNOWN(Dest, 0x0D8B12EB)		PUTDST32(Dest, COP1ConAddr)
	PUTDST16KNOWN(Dest, 0xE181)			PUTDST32KNOWN(Dest, 0xFF7FFFFF)
	PUTDST16KNOWN(Dest, 0x0D89)			PUTDST32(Dest, COP1ConAddr)

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_C_LE_D(OP_PARAMS)
{
	uint32 COP1ConAddr = (uint32)&COP1Con[31];
	
//0042E6A9 DD 05 88 DE 68 00    fld         qword ptr [_COP1Reg+8 (0068de88)]
//0042E6AF DC 1D 80 DE 68 00    fcomp       qword ptr [_COP1Reg (0068de80)]
//0042E6B5 DF E0                fnstsw      ax
//0042E6B7 F6 C4 41             test        ah,41h
//0042E6BA 74 11                je          dyn_r4300i_C_LE_D+2Dh (0042e6cd)
//0042E6BC A1 BC DC 68 00       mov         eax,[_COP1Con+7Ch (0068dcbc)]
//0042E6C1 0D 00 00 80 00       or          eax,offset _DynaRDRAM+171F80h (00800000)
//0042E6C6 A3 BC DC 68 00       mov         [_COP1Con+7Ch (0068dcbc)],eax
//1296:     else                                                            COP1Con[31] &=  0xFF7FFFFF;
//0042E6CB EB 12                jmp         dyn_r4300i_C_LE_D+3Fh (0042e6df)
//0042E6CD 8B 0D BC DC 68 00    mov         ecx,dword ptr [_COP1Con+7Ch (0068dcbc)]
//0042E6D3 81 E1 FF FF 7F FF    and         ecx,0FF7FFFFFh
//0042E6D9 89 0D BC DC 68 00    mov         dword ptr [_COP1Con+7Ch (0068dcbc)],ecx
	PUTDST16KNOWN(Dest, 0x05DD)			PUTDST32(Dest, (uint32)&COP1Reg[RD_FS])
	PUTDST16KNOWN(Dest, 0x1DDC)			PUTDST32(Dest, (uint32)&COP1Reg[RT_FT])
	PUTDST32KNOWN(Dest, 0xC4F6E0DF)		PUTDST32KNOWN(Dest, 0xA1117441)		PUTDST32(Dest, COP1ConAddr)
	PUTDST8KNOWN(Dest, 0x0D)			PUTDST32KNOWN(Dest, 0x00800000)
	PUTDST8KNOWN(Dest, 0xA3)			PUTDST32(Dest, COP1ConAddr)
	PUTDST32KNOWN(Dest, 0x0D8B12EB)		PUTDST32(Dest, COP1ConAddr)
	PUTDST16KNOWN(Dest, 0xE181)			PUTDST32KNOWN(Dest, 0xFF7FFFFF)
	PUTDST16KNOWN(Dest, 0x0D89)			PUTDST32(Dest, COP1ConAddr)

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_C_LT_D(OP_PARAMS)
{
	uint32 COP1ConAddr = (uint32)&COP1Con[31];
	
//00416AA9 DD 05 88 DE 68 00    fld         qword ptr [_COP1Reg+8 (0068de88)]
//00416AAF DC 1D 80 DE 68 00    fcomp       qword ptr [_COP1Reg (0068de80)]
//00416AB5 DF E0                fnstsw      ax
//00416AB7 F6 C4 01             test        ah,1
//00416ABA 74 11                je          dyn_r4300i_C_LT_D+2Dh (00416acd)
//00416ABC A1 BC DC 68 00       mov         eax,[_COP1Con+7Ch (0068dcbc)]
//00416AC1 0D 00 00 80 00       or          eax,offset _DynaRDRAM+171F80h (00800000)
//00416AC6 A3 BC DC 68 00       mov         [_COP1Con+7Ch (0068dcbc)],eax
//1296:                         else                                    COP1Con[31] &=  0xFF7FFFFF;
//00416ACB EB 12                jmp         dyn_r4300i_C_LT_D+3Fh (00416adf)
//00416ACD 8B 0D BC DC 68 00    mov         ecx,dword ptr [_COP1Con+7Ch (0068dcbc)]
//00416AD3 81 E1 FF FF 7F FF    and         ecx,0FF7FFFFFh
//00416AD9 89 0D BC DC 68 00    mov         dword ptr [_COP1Con+7Ch (0068dcbc)],ecx
	PUTDST16KNOWN(Dest, 0x05DD)			PUTDST32(Dest, (uint32)&COP1Reg[RD_FS])
	PUTDST16KNOWN(Dest, 0x1DDC)			PUTDST32(Dest, (uint32)&COP1Reg[RT_FT])
	PUTDST32KNOWN(Dest, 0xC4F6E0DF)		PUTDST32KNOWN(Dest, 0xA1117401)		PUTDST32(Dest, COP1ConAddr)
	PUTDST8KNOWN(Dest, 0x0D)			PUTDST32KNOWN(Dest, 0x00800000)
	PUTDST8KNOWN(Dest, 0xA3)			PUTDST32(Dest, COP1ConAddr)
	PUTDST32KNOWN(Dest, 0x0D8B12EB)		PUTDST32(Dest, COP1ConAddr)
	PUTDST16KNOWN(Dest, 0xE181)			PUTDST32KNOWN(Dest, 0xFF7FFFFF)
	PUTDST16KNOWN(Dest, 0x0D89)			PUTDST32(Dest, COP1ConAddr)

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_C_LT_S(OP_PARAMS)
{
	uint32 COP1ConAddr = (uint32)&COP1Con[31];
//00431C99 D9 05 88 DE 68 00    fld         dword ptr [_COP1Reg+8 (0068de88)]
//00431C9F D8 1D 80 DE 68 00    fcomp       dword ptr [_COP1Reg (0068de80)]
//00431CA5 DF E0                fnstsw      ax
//00431CA7 F6 C4 01             test        ah,1
//00431CAA 74 11                je          dyn_r4300i_C_LT_S+2Dh (00431cbd)
//00431CAC A1 BC DC 68 00       mov         eax,[_COP1Con+7Ch (0068dcbc)]
//00431CB1 0D 00 00 80 00       or          eax,offset _DynaRDRAM+171F80h (00800000)
//00431CB6 A3 BC DC 68 00       mov         [_COP1Con+7Ch (0068dcbc)],eax
//1327:     else                                                            COP1Con[31] &=  0xFF7FFFFF;
//00431CBB EB 12                jmp         dyn_r4300i_C_LT_S+3Fh (00431ccf)
//00431CBD 8B 0D BC DC 68 00    mov         ecx,dword ptr [_COP1Con+7Ch (0068dcbc)]
//00431CC3 81 E1 FF FF 7F FF    and         ecx,0FF7FFFFFh
//00431CC9 89 0D BC DC 68 00    mov         dword ptr [_COP1Con+7Ch (0068dcbc)],ecx
	PUTDST16KNOWN(Dest, 0x05D9)			PUTDST32(Dest, (uint32)&COP1Reg[RD_FS])
	PUTDST16KNOWN(Dest, 0x1DD8)			PUTDST32(Dest, (uint32)&COP1Reg[RT_FT])
	PUTDST32KNOWN(Dest, 0xC4F6E0DF)		PUTDST32KNOWN(Dest, 0xA1117401)		PUTDST32(Dest, COP1ConAddr)
	PUTDST8KNOWN(Dest, 0x0D)			PUTDST32KNOWN(Dest, 0x00800000)
	PUTDST8KNOWN(Dest, 0xA3)			PUTDST32(Dest, COP1ConAddr)
	PUTDST32KNOWN(Dest, 0x0D8B12EB)		PUTDST32(Dest, COP1ConAddr)
	PUTDST16KNOWN(Dest, 0xE181)			PUTDST32KNOWN(Dest, 0xFF7FFFFF)
	PUTDST16KNOWN(Dest, 0x0D89)			PUTDST32(Dest, COP1ConAddr)

	return(Dest);
}


//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_C_EQ_S(OP_PARAMS)
{
	uint32 COP1ConAddr = (uint32)&COP1Con[31];

//00419729 D9 05 88 DE 68 00    fld         dword ptr [_COP1Reg+8 (0068de88)]
//0041972F D8 1D 80 DE 68 00    fcomp       dword ptr [_COP1Reg (0068de80)]
//00419735 DF E0                fnstsw      ax
//00419737 F6 C4 40             test        ah,40h
//0041973A 74 11                je          dyn_r4300i_C_EQ_S+2Dh (0041974d)
//0041973C A1 BC DC 68 00       mov         eax,[_COP1Con+7Ch (0068dcbc)]
//00419741 0D 00 00 80 00       or          eax,offset _DynaRDRAM+171F80h (00800000)
//00419746 A3 BC DC 68 00       mov         [_COP1Con+7Ch (0068dcbc)],eax
//1547:     else                                                            COP1Con[31] &=  0xFF7FFFFF;
//0041974B EB 12                jmp         dyn_r4300i_C_EQ_S+3Fh (0041975f)
//0041974D 8B 0D BC DC 68 00    mov         ecx,dword ptr [_COP1Con+7Ch (0068dcbc)]
//00419753 81 E1 FF FF 7F FF    and         ecx,0FF7FFFFFh
//00419759 89 0D BC DC 68 00    mov         dword ptr [_COP1Con+7Ch (0068dcbc)],ecx
	PUTDST16KNOWN(Dest, 0x05D9)			PUTDST32(Dest, (uint32)&COP1Reg[RD_FS])
	PUTDST16KNOWN(Dest, 0x1DD8)			PUTDST32(Dest, (uint32)&COP1Reg[RT_FT])
	PUTDST32KNOWN(Dest, 0xC4F6E0DF)		PUTDST32KNOWN(Dest, 0xA1117440)		PUTDST32(Dest, COP1ConAddr)
	PUTDST8KNOWN(Dest, 0x0D)			PUTDST32KNOWN(Dest, 0x00800000)
	PUTDST8KNOWN(Dest, 0xA3)			PUTDST32(Dest, COP1ConAddr)
	PUTDST32KNOWN(Dest, 0x0D8B12EB)		PUTDST32(Dest, COP1ConAddr)
	PUTDST16KNOWN(Dest, 0xE181)			PUTDST32KNOWN(Dest, 0xFF7FFFFF)
	PUTDST16KNOWN(Dest, 0x0D89)			PUTDST32(Dest, COP1ConAddr)

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_C_EQ_D(OP_PARAMS)
{
	uint32 COP1ConAddr = (uint32)&COP1Con[31];

//00417599 DD 05 88 DE 68 00    fld         qword ptr [_COP1Reg+8 (0068de88)]
//0041759F DC 1D 80 DE 68 00    fcomp       qword ptr [_COP1Reg (0068de80)]
//004175A5 DF E0                fnstsw      ax
//004175A7 F6 C4 40             test        ah,40h
//004175AA 74 11                je          dyn_r4300i_C_EQ_D+2Dh (004175bd)
//004175AC A1 BC DC 68 00       mov         eax,[_COP1Con+7Ch (0068dcbc)]
//004175B1 0D 00 00 80 00       or          eax,offset _DynaRDRAM+171F80h (00800000)
//004175B6 A3 BC DC 68 00       mov         [_COP1Con+7Ch (0068dcbc)],eax
//1438:         else                                                            COP1Con[31] &=  0xFF7FFFFF;
//004175BB EB 12                jmp         dyn_r4300i_C_EQ_D+3Fh (004175cf)
//004175BD 8B 0D BC DC 68 00    mov         ecx,dword ptr [_COP1Con+7Ch (0068dcbc)]
//004175C3 81 E1 FF FF 7F FF    and         ecx,0FF7FFFFFh
//004175C9 89 0D BC DC 68 00    mov         dword ptr [_COP1Con+7Ch (0068dcbc)],ecx
	PUTDST16KNOWN(Dest, 0x05DD)			PUTDST32(Dest, (uint32)&COP1Reg[RD_FS])
	PUTDST16KNOWN(Dest, 0x1DDC)			PUTDST32(Dest, (uint32)&COP1Reg[RT_FT])
	PUTDST32KNOWN(Dest, 0xC4F6E0DF)		PUTDST32KNOWN(Dest, 0xA1117440)		PUTDST32(Dest, COP1ConAddr)
	PUTDST8KNOWN(Dest, 0x0D)			PUTDST32KNOWN(Dest, 0x00800000)
	PUTDST8KNOWN(Dest, 0xA3)			PUTDST32(Dest, COP1ConAddr)
	PUTDST32KNOWN(Dest, 0x0D8B12EB)		PUTDST32(Dest, COP1ConAddr)
	PUTDST16KNOWN(Dest, 0xE181)			PUTDST32KNOWN(Dest, 0xFF7FFFFF)
	PUTDST16KNOWN(Dest, 0x0D89)			PUTDST32(Dest, COP1ConAddr)

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_C_NGE_D(OP_PARAMS)	{dyn_r4300i_C_LT_D(PASS_PARAMS); return(Dest);}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_COP1_cfc1(OP_PARAMS)	{

//00416089 A1 A0 FD 4A 00       mov         eax,[_COP1Con (004afda0)]
//0041608E A3 E0 01 4B 00       mov         [_GPR (004b01e0)],eax
//00416093 8B 0D A4 FD 4A 00    mov         ecx,dword ptr [_COP1Con+4 (004afda4)]
//00416099 89 0D E4 01 4B 00    mov         dword ptr [_GPR+4 (004b01e4)],ecx

	uint32 FSAddr = (uint32)&COP1Con[RD_FS];
	uint32 RTAddr = (uint32)&GPR[RT_FT];

	PUTDST8KNOWN(Dest, 0xA1)	PUTDST32(Dest, FSAddr)
	PUTDST8KNOWN(Dest, 0xA3)	PUTDST32(Dest, RTAddr)
	PUTDST16KNOWN(Dest, 0x0d8B)	PUTDST32(Dest, FSAddr+4)
	PUTDST16KNOWN(Dest, 0x0D89)	PUTDST32(Dest, RTAddr+4)

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_COP1_ctc1(OP_PARAMS)	{
	uint32 FSAddr = (uint32)&COP1Con[RD_FS];
	uint32 RTAddr = (uint32)&GPR[RT_FT];

	PUTDST8KNOWN(Dest, 0xA1)	PUTDST32(Dest, RTAddr)
	PUTDST8KNOWN(Dest, 0xA3)	PUTDST32(Dest, FSAddr)
	PUTDST16KNOWN(Dest, 0x0d8B)	PUTDST32(Dest, RTAddr+4)
	PUTDST16KNOWN(Dest, 0x0D89)	PUTDST32(Dest, FSAddr+4)

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_mult(OP_PARAMS)		{	

//00409850 A1 48 13 42 00       mov         eax,[00421348]
//00409855 F7 2D 40 13 42 00    imul        dword ptr ds:[421340h]
//0040985B 8B CA                mov         ecx,edx
//0040985D 99                   cdq

//0040985E A3 C0 12 42 00       mov         [004212C0],eax
//00409863 8B C1                mov         eax,ecx
//00409865 C1 F8 1F             sar         eax,1Fh

//00409868 8B C1                mov         eax,ecx
//0040986A 89 15 C4 12 42 00    mov         dword ptr ds:[4212C4h],edx
//00409870 99                   cdq
//00409871 A3 B0 12 42 00       mov         [004212B0],eax
//00409876 89 15 B4 12 42 00    mov         dword ptr ds:[4212B4h],edx

	uint32 LOAddr = (uint32)&LO;
	uint32 HIAddr = (uint32)&HI;

	PUTDST8KNOWN(Dest, 0xA1)			PUTDST32(Dest, (uint32)&GPR[RS_BASE_FMT])
	PUTDST16KNOWN(Dest, 0x2DF7)			PUTDST32(Dest, (uint32)&GPR[RT_FT])
	PUTDST16KNOWN(Dest, 0xCA8B)			PUTDST8KNOWN(Dest, 0x99)

	PUTDST8KNOWN(Dest, 0xA3)			PUTDST32(Dest, LOAddr)
	PUTDST32KNOWN(Dest, 0xF8C1C18B)		PUTDST8KNOWN(Dest, 0x1F)
	
	PUTDST32KNOWN(Dest, 0x1589C18B)		PUTDST32(Dest, LOAddr+4)
	CDQ_STORE(Dest, HIAddr)

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_multu(OP_PARAMS)
{
	uint32 LOAddr = (uint32)&LO;
	uint32 HIAddr = (uint32)&HI;
	
//00409990 A1 48 13 42 00       mov         eax,[00421348]
//00409995 F7 25 40 13 42 00    mul         eax,dword ptr ds:[421340h]
//0040999B 8B CA                mov         ecx,edx
//0040999D 99                   cdq
//0040999E A3 C0 12 42 00       mov         [004212C0],eax
//004099A3 8B C1                mov         eax,ecx
//004099A5 89 15 C4 12 42 00    mov         dword ptr ds:[4212C4h],edx
//004099AB 99                   cdq
//004099AC A3 B0 12 42 00       mov         [004212B0],eax
//004099B1 89 15 B4 12 42 00    mov         dword ptr ds:[4212B4h],edx

	PUTDST8KNOWN(Dest, 0xA1)			PUTDST32(Dest, (uint32)&GPR[RS_BASE_FMT])
	PUTDST16KNOWN(Dest, 0x25F7)			PUTDST32(Dest, (uint32)&GPR[RT_FT])
	PUTDST16KNOWN(Dest, 0xCA8B)			PUTDST8KNOWN(Dest, 0x99)

	PUTDST8KNOWN(Dest, 0xA3)			PUTDST32(Dest, LOAddr)
	PUTDST16KNOWN(Dest, 0xC18B)		
	
	PUTDST32KNOWN(Dest, 0x1589C18B)		PUTDST32(Dest, LOAddr+4)
	CDQ_STORE(Dest, HIAddr)

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_lwc1(OP_PARAMS){INTERPRET(r4300i_lwc1) return(Dest);}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_dsll32(OP_PARAMS)
{
//0040C679 A1 20 21 A9 00       mov         eax,[_GPR (00a92120)]
//0040C67E C1 E0 1F             shl         eax,1Fh
//0040C681 A3 34 21 A9 00       mov         [_GPR+14h (00a92134)],eax
//0040C686 C7 05 30 21 A9 00 00 mov         dword ptr [_GPR+10h (00a92130)],0

	PUTDST8KNOWN(Dest, 0xA1)	PUTDST32(Dest, (uint32)&GPR[RT_FT])
	PUTDST16KNOWN(Dest, 0xE0C1)	PUTDST8(Dest, SA_FD)
	PUTDST8KNOWN(Dest, 0xA3)	PUTDST32(Dest, (uint32)&GPR[RD_FS]+4)
	PUTDST16KNOWN(Dest, 0x05C7)	PUTDST32(Dest, (uint32)&GPR[RD_FS]  )	PUTDST32KNOWN(Dest, 0x00000000)

	return(Dest);
}

RETURN_TYPE dyn_r4300i_dsrl32(OP_PARAMS){INTERPRET(r4300i_dsrl32) return(Dest);}
RETURN_TYPE dyn_r4300i_dsra32(OP_PARAMS)
{
	uint32 RDAddr = (uint32)&GPR[RD_FS];

//00418569 A1 24 21 A9 00       mov         eax,[_GPR+4 (00a92124)]
//0041856E A3 30 21 A9 00       mov         [_GPR+10h (00a92130)],eax
//00418573 A1 20 21 A9 00       mov         eax,[_GPR (00a92120)]
//00418578 C1 F8 1F             sar         eax,XXh (<- Shift Amount)
//0041857B 99                   cdq
//0041857C A3 30 21 A9 00       mov         [_GPR+10h (00a92130)],eax
//00418581 89 15 34 21 A9 00    mov         dword ptr [_GPR+14h (00a92134)],edx

	PUTDST8KNOWN(Dest, 0xA1)	PUTDST32(Dest, (uint32)&GPR[RT_FT]+4)
	PUTDST8KNOWN(Dest, 0xA3)	PUTDST32(Dest, RDAddr)
	PUTDST8KNOWN(Dest, 0xA1)	PUTDST32(Dest, (uint32)&GPR[RT_FT]  )
	PUTDST16KNOWN(Dest, 0xF8C1)	PUTDST8(Dest, SA_FD)
	CDQ_STORE(Dest, RDAddr)

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_COP1_cvtd_s(OP_PARAMS){	
//00417609 D9 05 C8 78 4A 00    fld         dword ptr [_COP1Reg+8 (004a78c8)]
//0041760F DD 1D C0 78 4A 00    fstp        qword ptr [_COP1Reg (004a78c0)]
	PUTDST16KNOWN(Dest, 0x05D9)		PUTDST32(Dest, (uint32)&COP1Reg[RD_FS])
	PUTDST16KNOWN(Dest, 0x1DDD)		PUTDST32(Dest, (uint32)&COP1Reg[SA_FD])

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_COP1_cvtd_w(OP_PARAMS){	
//004176C9 DB 05 C8 78 4A 00    fild        dword ptr [_COP1Reg+8 (004a78c8)]
//004176CF DD 1D C0 78 4A 00    fstp        qword ptr [_COP1Reg (004a78c0)]
	PUTDST16KNOWN(Dest, 0x05DB)		PUTDST32(Dest, (uint32)&COP1Reg[RD_FS])
	PUTDST16KNOWN(Dest, 0x1DDD)		PUTDST32(Dest, (uint32)&COP1Reg[SA_FD])

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_COP1_cvts_d(OP_PARAMS){	
//00417A4A DD 05 C8 78 4A 00    fld         qword ptr [_COP1Reg+8 (004a78c8)]
//00417A50 D9 1D C0 78 4A 00    fstp        dword ptr [_COP1Reg (004a78c0)]
	PUTDST16KNOWN(Dest, 0x05DD)		PUTDST32(Dest, (uint32)&COP1Reg[RD_FS])
	PUTDST16KNOWN(Dest, 0x1DD9)		PUTDST32(Dest, (uint32)&COP1Reg[SA_FD])

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_COP1_cvts_w(OP_PARAMS){	
//00417A89 DB 05 C8 78 4A 00    fild        dword ptr [_COP1Reg+8 (004a78c8)]
//00417A8F D9 1D C0 78 4A 00    fstp        dword ptr [_COP1Reg (004a78c0)]
	PUTDST16KNOWN(Dest, 0x05DB)		PUTDST32(Dest, (uint32)&COP1Reg[RD_FS])
	PUTDST16KNOWN(Dest, 0x1DD9)		PUTDST32(Dest, (uint32)&COP1Reg[SA_FD])

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_COP1_mov_s(OP_PARAMS){	
//00417FC9 A1 C8 78 4A 00       mov         eax,[_COP1Reg+8 (004a78c8)]
//00417FCE A3 C0 78 4A 00       mov         [_COP1Reg (004a78c0)],eax
	PUTDST8KNOWN(Dest, 0xA1)		PUTDST32(Dest, (uint32)&COP1Reg[RD_FS])
	PUTDST8KNOWN(Dest, 0xA3)		PUTDST32(Dest, (uint32)&COP1Reg[SA_FD])

	return(Dest);
}

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_r4300i_COP1_mov_d(OP_PARAMS)
{
	uint32 FDAddr = (uint32)&COP1Reg[SA_FD];
	uint32 FSAddr = (uint32)&COP1Reg[RD_FS];
//004197F9 A1 88 DE 68 00       mov         eax,[_COP1Reg+8 (0068de88)]
//004197FE A3 80 DE 68 00       mov         [_COP1Reg (0068de80)],eax
//00419803 8B 0D 8C DE 68 00    mov         ecx,dword ptr [_COP1Reg+0Ch (0068de8c)]
//00419809 89 0D 84 DE 68 00    mov         dword ptr [_COP1Reg+4 (0068de84)],ecx
	PUTDST8KNOWN(Dest, 0xA1)	PUTDST32(Dest, FSAddr)
	PUTDST8KNOWN(Dest, 0xA3)	PUTDST32(Dest, FDAddr)
	PUTDST16KNOWN(Dest, 0x0D8B)	PUTDST32(Dest, FSAddr+4)
	PUTDST16KNOWN(Dest, 0x0D89)	PUTDST32(Dest, FDAddr+4)

	return(Dest);
}

//---------------------------------------------------------------------------------------
//Not recompiled yet--still using interpretive opcode functions:
//--------------------------------------------------------------
RETURN_TYPE dyn_r4300i_lwu(OP_PARAMS)		{		INTERPRET(r4300i_lwu)			return(Dest);}
RETURN_TYPE dyn_r4300i_ddiv(OP_PARAMS)		{		INTERPRET(r4300i_ddiv)			return(Dest);}
RETURN_TYPE dyn_r4300i_ddivu(OP_PARAMS)		{		INTERPRET(r4300i_ddivu)			return(Dest);}
RETURN_TYPE dyn_r4300i_ll(OP_PARAMS)		{		INTERPRET(r4300i_ll)			return(Dest);}
RETURN_TYPE dyn_r4300i_lld(OP_PARAMS)		{		INTERPRET(r4300i_lld)			return(Dest);}
RETURN_TYPE dyn_r4300i_dadd(OP_PARAMS)		{		INTERPRET(r4300i_dadd)			return(Dest);}
RETURN_TYPE dyn_r4300i_daddu(OP_PARAMS)		{		INTERPRET(r4300i_daddu)			return(Dest);}
RETURN_TYPE dyn_r4300i_dsub(OP_PARAMS)		{		INTERPRET(r4300i_dsub)			return(Dest);}
RETURN_TYPE dyn_r4300i_dsubu(OP_PARAMS)		{		INTERPRET(r4300i_dsubu)			return(Dest);}
RETURN_TYPE dyn_r4300i_dmult(OP_PARAMS)		{		INTERPRET(r4300i_dmult)			return(Dest);}
RETURN_TYPE dyn_r4300i_dmultu(OP_PARAMS)	{		INTERPRET(r4300i_dmultu)		return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_ceilw(OP_PARAMS){		INTERPRET(r4300i_COP1_ceilw)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_ceill(OP_PARAMS){		INTERPRET(r4300i_COP1_ceill)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cvtd_l(OP_PARAMS){		INTERPRET(r4300i_COP1_cvtd_l)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cvtl_s(OP_PARAMS){		INTERPRET(r4300i_COP1_cvtl_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cvtl_d(OP_PARAMS){		INTERPRET(r4300i_COP1_cvtl_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cvts_l(OP_PARAMS){		INTERPRET(r4300i_COP1_cvts_l)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cvtw_s(OP_PARAMS){		INTERPRET(r4300i_COP1_cvtw_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cvtw_d(OP_PARAMS){		INTERPRET(r4300i_COP1_cvtw_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_dmfc1(OP_PARAMS){		INTERPRET(r4300i_COP1_dmfc1)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_dmtc1(OP_PARAMS){		INTERPRET(r4300i_COP1_dmtc1)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_floorl(OP_PARAMS){		INTERPRET(r4300i_COP1_floorl)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_floorw(OP_PARAMS){		INTERPRET(r4300i_COP1_floorw)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_roundl(OP_PARAMS){		INTERPRET(r4300i_COP1_roundl)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_roundw(OP_PARAMS){		INTERPRET(r4300i_COP1_roundw)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_sqrt_s(OP_PARAMS){		INTERPRET(r4300i_COP1_sqrt_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_sqrt_d(OP_PARAMS){		INTERPRET(r4300i_COP1_sqrt_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_neg_d(OP_PARAMS){		INTERPRET(r4300i_COP1_neg_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_truncl(OP_PARAMS){		INTERPRET(r4300i_COP1_truncl)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_truncw_s(OP_PARAMS){	INTERPRET(r4300i_COP1_truncw_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_truncw_d(OP_PARAMS){	INTERPRET(r4300i_COP1_truncw_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_C_F_S(OP_PARAMS)		{		INTERPRET(r4300i_C_F_S)			return(Dest);}
RETURN_TYPE dyn_r4300i_C_F_D(OP_PARAMS)		{		INTERPRET(r4300i_C_F_D)			return(Dest);}
RETURN_TYPE dyn_r4300i_C_UN_S(OP_PARAMS)	{		INTERPRET(r4300i_C_UN_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_UN_D(OP_PARAMS)	{		INTERPRET(r4300i_C_UN_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_UEQ_S(OP_PARAMS)	{		INTERPRET(r4300i_C_UEQ_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_UEQ_D(OP_PARAMS)	{		INTERPRET(r4300i_C_UEQ_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_OLT_S(OP_PARAMS)	{		INTERPRET(r4300i_C_OLT_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_OLT_D(OP_PARAMS)	{		INTERPRET(r4300i_C_OLT_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_ULT_S(OP_PARAMS)	{		INTERPRET(r4300i_C_ULT_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_ULT_D(OP_PARAMS)	{		INTERPRET(r4300i_C_ULT_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_OLE_S(OP_PARAMS)	{		INTERPRET(r4300i_C_OLE_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_OLE_D(OP_PARAMS)	{		INTERPRET(r4300i_C_OLE_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_ULE_S(OP_PARAMS)	{		INTERPRET(r4300i_C_ULE_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_ULE_D(OP_PARAMS)	{		INTERPRET(r4300i_C_ULE_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_SF_S(OP_PARAMS)	{		INTERPRET(r4300i_C_SF_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_SF_D(OP_PARAMS)	{		INTERPRET(r4300i_C_SF_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_NGLE_S(OP_PARAMS)	{		INTERPRET(r4300i_C_NGLE_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_NGLE_D(OP_PARAMS)	{		INTERPRET(r4300i_C_NGLE_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_SEQ_S(OP_PARAMS)	{		INTERPRET(r4300i_C_SEQ_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_SEQ_D(OP_PARAMS)	{		INTERPRET(r4300i_C_SEQ_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_NGL_S(OP_PARAMS)	{		INTERPRET(r4300i_C_NGL_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_NGL_D(OP_PARAMS)	{		INTERPRET(r4300i_C_NGL_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_NGE_S(OP_PARAMS)	{		INTERPRET(r4300i_C_NGE_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_NGT_S(OP_PARAMS)	{		INTERPRET(r4300i_C_NGT_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_NGT_D(OP_PARAMS)	{		INTERPRET(r4300i_C_NGT_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_COP0_tlbp(OP_PARAMS) {		INTERPRET(r4300i_COP0_tlbp)		return(Dest);}
RETURN_TYPE dyn_r4300i_COP0_tlbr(OP_PARAMS) {		INTERPRET(r4300i_COP0_tlbr)		return(Dest);}
RETURN_TYPE dyn_r4300i_COP0_tlbwi(OP_PARAMS){		INTERPRET(r4300i_COP0_tlbwi)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP0_tlbwr(OP_PARAMS){		INTERPRET(r4300i_COP0_tlbwr)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_abs_s(OP_PARAMS){		INTERPRET(r4300i_COP1_abs_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_abs_d(OP_PARAMS){		INTERPRET(r4300i_COP1_abs_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_dsll(OP_PARAMS)		{		INTERPRET(r4300i_dsll)			return(Dest);}
RETURN_TYPE dyn_r4300i_dsllv(OP_PARAMS)		{		INTERPRET(r4300i_dsllv)			return(Dest);}
RETURN_TYPE dyn_r4300i_dsra(OP_PARAMS)		{		INTERPRET(r4300i_dsra)			return(Dest);}
RETURN_TYPE dyn_r4300i_dsrav(OP_PARAMS)		{		INTERPRET(r4300i_dsrav)			return(Dest);}
RETURN_TYPE dyn_r4300i_dsrl(OP_PARAMS)		{		INTERPRET(r4300i_dsrl)			return(Dest);}
RETURN_TYPE dyn_r4300i_dsrlv(OP_PARAMS)		{		INTERPRET(r4300i_dsrlv)			return(Dest);}
RETURN_TYPE dyn_r4300i_sc(OP_PARAMS)		{		INTERPRET(r4300i_sc)			return(Dest);}
RETURN_TYPE dyn_r4300i_scd(OP_PARAMS)		{		INTERPRET(r4300i_scd)			return(Dest);}
RETURN_TYPE dyn_r4300i_lwl(OP_PARAMS)		{		INTERPRET(r4300i_lwl)			return(Dest);}
RETURN_TYPE dyn_r4300i_lwr(OP_PARAMS)		{		INTERPRET(r4300i_lwr)			return(Dest);}
RETURN_TYPE dyn_r4300i_ldl(OP_PARAMS)		{		INTERPRET(r4300i_ldl)			return(Dest);}
RETURN_TYPE dyn_r4300i_ldr(OP_PARAMS)		{		INTERPRET(r4300i_ldr)			return(Dest);}
RETURN_TYPE dyn_r4300i_sdl(OP_PARAMS)		{		INTERPRET(r4300i_sdl)			return(Dest);}
RETURN_TYPE dyn_r4300i_sdr(OP_PARAMS)		{		INTERPRET(r4300i_sdr)			return(Dest);}
RETURN_TYPE dyn_r4300i_swl(OP_PARAMS)		{		INTERPRET(r4300i_swl)			return(Dest);}
RETURN_TYPE dyn_r4300i_swr(OP_PARAMS)		{		INTERPRET(r4300i_swr)			return(Dest);}
RETURN_TYPE dyn_r4300i_daddi(OP_PARAMS)		{		INTERPRET(r4300i_daddi)			return(Dest);}
RETURN_TYPE dyn_r4300i_daddiu(OP_PARAMS)	{		INTERPRET(r4300i_daddiu)		return(Dest);}

//---------------------------------------------------------------------------------------
// Ignored instructions

RETURN_TYPE dyn_r4300i_teq(OP_PARAMS)		{return(Dest);}
RETURN_TYPE dyn_r4300i_tge(OP_PARAMS)		{return(Dest);}
RETURN_TYPE dyn_r4300i_tgeu(OP_PARAMS)		{return(Dest);}
RETURN_TYPE dyn_r4300i_tlt(OP_PARAMS)		{return(Dest);}
RETURN_TYPE dyn_r4300i_tltu(OP_PARAMS)		{return(Dest);}
RETURN_TYPE dyn_r4300i_tne(OP_PARAMS)		{return(Dest);}
RETURN_TYPE dyn_r4300i_cache(OP_PARAMS)		{return(Dest);}
RETURN_TYPE dyn_r4300i_teqi(OP_PARAMS)		{return(Dest);}
RETURN_TYPE dyn_r4300i_tgei(OP_PARAMS)		{return(Dest);}
RETURN_TYPE dyn_r4300i_tgeiu(OP_PARAMS)		{return(Dest);}
RETURN_TYPE dyn_r4300i_tlti(OP_PARAMS)		{return(Dest);}
RETURN_TYPE dyn_r4300i_tltiu(OP_PARAMS)		{return(Dest);}
RETURN_TYPE dyn_r4300i_tnei(OP_PARAMS)		{return(Dest);}
RETURN_TYPE dyn_r4300i_break(OP_PARAMS)		{return(Dest);}
RETURN_TYPE dyn_r4300i_sync(OP_PARAMS)		{return(Dest);}
RETURN_TYPE dyn_r4300i_syscall(OP_PARAMS)	{return(Dest);}

#else //#ifdef SAFE_DYNAREC

RETURN_TYPE dyn_r4300i_add(OP_PARAMS)		{		INTERPRET(r4300i_add)			return(Dest);}
RETURN_TYPE dyn_r4300i_addu(OP_PARAMS)		{		INTERPRET(r4300i_addu)			return(Dest);}
RETURN_TYPE dyn_r4300i_sub(OP_PARAMS)		{		INTERPRET(r4300i_sub)			return(Dest);}
RETURN_TYPE dyn_r4300i_subu(OP_PARAMS)		{		INTERPRET(r4300i_subu)			return(Dest);}
RETURN_TYPE dyn_r4300i_and(OP_PARAMS)		{		INTERPRET(r4300i_and)			return(Dest);}
RETURN_TYPE dyn_r4300i_or(OP_PARAMS)		{		INTERPRET(r4300i_or)			return(Dest);}
RETURN_TYPE dyn_r4300i_xor(OP_PARAMS)		{		INTERPRET(r4300i_xor)			return(Dest);}
RETURN_TYPE dyn_r4300i_nor(OP_PARAMS)		{		INTERPRET(r4300i_nor)			return(Dest);}
RETURN_TYPE dyn_r4300i_addi(OP_PARAMS)		{		INTERPRET(r4300i_addi)			return(Dest);}
RETURN_TYPE dyn_r4300i_addiu(OP_PARAMS)		{		INTERPRET(r4300i_addiu)			return(Dest);}
RETURN_TYPE dyn_r4300i_lb(OP_PARAMS)		{		INTERPRET(r4300i_lb)			return(Dest);}
RETURN_TYPE dyn_r4300i_lh(OP_PARAMS)		{		INTERPRET(r4300i_lh)			return(Dest);}
RETURN_TYPE dyn_r4300i_lbu(OP_PARAMS)		{		INTERPRET(r4300i_lbu)			return(Dest);}
RETURN_TYPE dyn_r4300i_lhu(OP_PARAMS)		{		INTERPRET(r4300i_lhu)			return(Dest);}
RETURN_TYPE dyn_r4300i_ld(OP_PARAMS)		{		INTERPRET(r4300i_ld)			return(Dest);}
RETURN_TYPE dyn_r4300i_ldc1(OP_PARAMS)		{		INTERPRET(r4300i_ldc1)			return(Dest);}
RETURN_TYPE dyn_r4300i_sd(OP_PARAMS)		{		INTERPRET(r4300i_sd)			return(Dest);}
RETURN_TYPE dyn_r4300i_sdc1(OP_PARAMS)		{		INTERPRET(r4300i_sdc1)			return(Dest);}
RETURN_TYPE dyn_r4300i_lw(OP_PARAMS)		{		INTERPRET(r4300i_lw)			return(Dest);}
RETURN_TYPE dyn_r4300i_sw(OP_PARAMS)		{		INTERPRET(r4300i_sw)			return(Dest);}
RETURN_TYPE dyn_r4300i_swc1(OP_PARAMS)		{		INTERPRET(r4300i_swc1)			return(Dest);}
RETURN_TYPE dyn_r4300i_sb(OP_PARAMS)		{		INTERPRET(r4300i_sb)			return(Dest);}
RETURN_TYPE dyn_r4300i_sh(OP_PARAMS)		{		INTERPRET(r4300i_sh)			return(Dest);}
RETURN_TYPE dyn_r4300i_slti(OP_PARAMS)		{		INTERPRET(r4300i_slti)			return(Dest);}
RETURN_TYPE dyn_r4300i_sltiu(OP_PARAMS)		{		INTERPRET(r4300i_sltiu)			return(Dest);}
RETURN_TYPE dyn_r4300i_sltu(OP_PARAMS)		{		INTERPRET(r4300i_sltu)			return(Dest);}
RETURN_TYPE dyn_r4300i_srav(OP_PARAMS)		{		INTERPRET(r4300i_srav)			return(Dest);}
RETURN_TYPE dyn_r4300i_slt(OP_PARAMS)		{		INTERPRET(r4300i_slt)			return(Dest);}
RETURN_TYPE dyn_r4300i_sll(OP_PARAMS)		{		INTERPRET(r4300i_sll)			return(Dest);}
RETURN_TYPE dyn_r4300i_srl(OP_PARAMS)		{		INTERPRET(r4300i_srl)			return(Dest);}
RETURN_TYPE dyn_r4300i_sllv(OP_PARAMS)		{		INTERPRET(r4300i_sllv)			return(Dest);}
RETURN_TYPE dyn_r4300i_srlv(OP_PARAMS)		{		INTERPRET(r4300i_srlv)			return(Dest);}
RETURN_TYPE dyn_r4300i_sra(OP_PARAMS)		{		INTERPRET(r4300i_sra)			return(Dest);}
RETURN_TYPE dyn_r4300i_div(OP_PARAMS)		{		INTERPRET(r4300i_div)			return(Dest);}
RETURN_TYPE dyn_r4300i_mfhi(OP_PARAMS)		{		INTERPRET(r4300i_mfhi)			return(Dest);}
RETURN_TYPE dyn_r4300i_mflo(OP_PARAMS)		{		INTERPRET(r4300i_mflo)			return(Dest);}
RETURN_TYPE dyn_r4300i_mthi(OP_PARAMS)		{		INTERPRET(r4300i_mthi)			return(Dest);}
RETURN_TYPE dyn_r4300i_mtlo(OP_PARAMS)		{		INTERPRET(r4300i_mtlo)			return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_add_s(OP_PARAMS){		INTERPRET(r4300i_COP1_add_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_sub_s(OP_PARAMS){		INTERPRET(r4300i_COP1_sub_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_mul_s(OP_PARAMS){		INTERPRET(r4300i_COP1_mul_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_div_s(OP_PARAMS){		INTERPRET(r4300i_COP1_div_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_add_d(OP_PARAMS){		INTERPRET(r4300i_COP1_add_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_sub_d(OP_PARAMS){		INTERPRET(r4300i_COP1_sub_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_mul_d(OP_PARAMS){		INTERPRET(r4300i_COP1_mul_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_div_d(OP_PARAMS){		INTERPRET(r4300i_COP1_div_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP0_mfc0(OP_PARAMS)	{		INTERPRET(r4300i_COP0_mfc0)		return(Dest);}	 
RETURN_TYPE dyn_r4300i_COP0_mtc0(OP_PARAMS)	{		INTERPRET(r4300i_COP0_mtc0)		return(Dest);}
RETURN_TYPE dyn_r4300i_andi(OP_PARAMS)		{		INTERPRET(r4300i_andi)			return(Dest);}
RETURN_TYPE dyn_r4300i_xori(OP_PARAMS)		{		INTERPRET(r4300i_xori)			return(Dest);}
RETURN_TYPE dyn_r4300i_ori(OP_PARAMS)		{		INTERPRET(r4300i_ori)			return(Dest);}
RETURN_TYPE dyn_r4300i_lui(OP_PARAMS)		{		INTERPRET(r4300i_lui)			return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_mtc1(OP_PARAMS)	{		INTERPRET(r4300i_COP1_mtc1)		return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_neg_s(OP_PARAMS){		INTERPRET(r4300i_COP1_neg_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_mfc1(OP_PARAMS)	{		INTERPRET(r4300i_COP1_mfc1)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_LT_D(OP_PARAMS)	{		INTERPRET(r4300i_C_LT_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_NGE_D(OP_PARAMS)	{		INTERPRET(r4300i_C_NGE_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cfc1(OP_PARAMS)	{		INTERPRET(r4300i_COP1_cfc1)		return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_ctc1(OP_PARAMS)	{		INTERPRET(r4300i_COP1_ctc1)		return(Dest);}
RETURN_TYPE dyn_r4300i_mult(OP_PARAMS)		{		INTERPRET(r4300i_mult)			return(Dest);}
RETURN_TYPE dyn_r4300i_multu(OP_PARAMS)		{		INTERPRET(r4300i_multu)			return(Dest);}
RETURN_TYPE dyn_r4300i_lwc1(OP_PARAMS)		{		INTERPRET(r4300i_lwc1)			return(Dest);}
RETURN_TYPE dyn_r4300i_dsll32(OP_PARAMS)	{		INTERPRET(r4300i_dsll32)		return(Dest);}
RETURN_TYPE dyn_r4300i_dsra32(OP_PARAMS)	{		INTERPRET(r4300i_dsra32)		return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cvtd_s(OP_PARAMS){		INTERPRET(r4300i_COP1_cvtd_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cvtd_w(OP_PARAMS){		INTERPRET(r4300i_COP1_cvtd_w)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cvts_d(OP_PARAMS){		INTERPRET(r4300i_COP1_cvts_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cvts_w(OP_PARAMS){		INTERPRET(r4300i_COP1_cvts_w)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_mov_s(OP_PARAMS){		INTERPRET(r4300i_COP1_mov_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_lwu(OP_PARAMS)		{		INTERPRET(r4300i_lwu)			return(Dest);}
RETURN_TYPE dyn_r4300i_ddiv(OP_PARAMS)		{		INTERPRET(r4300i_ddiv)			return(Dest);}
RETURN_TYPE dyn_r4300i_ddivu(OP_PARAMS)		{		INTERPRET(r4300i_ddivu)			return(Dest);}
RETURN_TYPE dyn_r4300i_ll(OP_PARAMS)		{		INTERPRET(r4300i_ll)			return(Dest);}
RETURN_TYPE dyn_r4300i_lld(OP_PARAMS)		{		INTERPRET(r4300i_lld)			return(Dest);}
RETURN_TYPE dyn_r4300i_break(OP_PARAMS)		{		INTERPRET(r4300i_break)			return(Dest);}
RETURN_TYPE dyn_r4300i_sync(OP_PARAMS)		{		INTERPRET(r4300i_sync)			return(Dest);}
RETURN_TYPE dyn_r4300i_syscall(OP_PARAMS)	{		INTERPRET(r4300i_syscall)		return(Dest);}
RETURN_TYPE dyn_r4300i_teqi(OP_PARAMS)		{		INTERPRET(r4300i_teqi)			return(Dest);}
RETURN_TYPE dyn_r4300i_tgei(OP_PARAMS)		{		INTERPRET(r4300i_tgei)			return(Dest);}
RETURN_TYPE dyn_r4300i_tgeiu(OP_PARAMS)		{		INTERPRET(r4300i_tgeiu)			return(Dest);}
RETURN_TYPE dyn_r4300i_tlti(OP_PARAMS)		{		INTERPRET(r4300i_tlti)			return(Dest);}
RETURN_TYPE dyn_r4300i_tltiu(OP_PARAMS)		{		INTERPRET(r4300i_tltiu)			return(Dest);}
RETURN_TYPE dyn_r4300i_tnei(OP_PARAMS)		{		INTERPRET(r4300i_tnei)			return(Dest);}
RETURN_TYPE dyn_r4300i_dadd(OP_PARAMS)		{		INTERPRET(r4300i_dadd)			return(Dest);}
RETURN_TYPE dyn_r4300i_daddu(OP_PARAMS)		{		INTERPRET(r4300i_daddu)			return(Dest);}
RETURN_TYPE dyn_r4300i_dsub(OP_PARAMS)		{		INTERPRET(r4300i_dsub)			return(Dest);}
RETURN_TYPE dyn_r4300i_dsubu(OP_PARAMS)		{		INTERPRET(r4300i_dsubu)			return(Dest);}
RETURN_TYPE dyn_r4300i_divu(OP_PARAMS)		{		INTERPRET(r4300i_divu)			return(Dest);}
RETURN_TYPE dyn_r4300i_dmult(OP_PARAMS)		{		INTERPRET(r4300i_dmult)			return(Dest);}
RETURN_TYPE dyn_r4300i_dmultu(OP_PARAMS)	{		INTERPRET(r4300i_dmultu)		return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_ceilw(OP_PARAMS){		INTERPRET(r4300i_COP1_ceilw)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_ceill(OP_PARAMS){		INTERPRET(r4300i_COP1_ceill)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cvtd_l(OP_PARAMS){		INTERPRET(r4300i_COP1_cvtd_l)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cvtl_s(OP_PARAMS){		INTERPRET(r4300i_COP1_cvtl_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cvtl_d(OP_PARAMS){		INTERPRET(r4300i_COP1_cvtl_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cvts_l(OP_PARAMS){		INTERPRET(r4300i_COP1_cvts_l)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cvtw_s(OP_PARAMS){		INTERPRET(r4300i_COP1_cvtw_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_cvtw_d(OP_PARAMS){		INTERPRET(r4300i_COP1_cvtw_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_dmfc1(OP_PARAMS){		INTERPRET(r4300i_COP1_dmfc1)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_dmtc1(OP_PARAMS){		INTERPRET(r4300i_COP1_dmtc1)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_floorl(OP_PARAMS){		INTERPRET(r4300i_COP1_floorl)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_floorw(OP_PARAMS){		INTERPRET(r4300i_COP1_floorw)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_mov_d(OP_PARAMS){		INTERPRET(r4300i_COP1_mov_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_roundl(OP_PARAMS){		INTERPRET(r4300i_COP1_roundl)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_roundw(OP_PARAMS){		INTERPRET(r4300i_COP1_roundw)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_sqrt_s(OP_PARAMS){		INTERPRET(r4300i_COP1_sqrt_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_sqrt_d(OP_PARAMS){		INTERPRET(r4300i_COP1_sqrt_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_neg_d(OP_PARAMS){		INTERPRET(r4300i_COP1_neg_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_truncl(OP_PARAMS){		INTERPRET(r4300i_COP1_truncl)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_truncw_s(OP_PARAMS){	INTERPRET(r4300i_COP1_truncw_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_truncw_d(OP_PARAMS){	INTERPRET(r4300i_COP1_truncw_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_C_F_S(OP_PARAMS)		{		INTERPRET(r4300i_C_F_S)			return(Dest);}
RETURN_TYPE dyn_r4300i_C_F_D(OP_PARAMS)		{		INTERPRET(r4300i_C_F_D)			return(Dest);}
RETURN_TYPE dyn_r4300i_C_UN_S(OP_PARAMS)	{		INTERPRET(r4300i_C_UN_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_UN_D(OP_PARAMS)	{		INTERPRET(r4300i_C_UN_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_EQ_S(OP_PARAMS)	{		INTERPRET(r4300i_C_EQ_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_EQ_D(OP_PARAMS)	{		INTERPRET(r4300i_C_EQ_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_UEQ_S(OP_PARAMS)	{		INTERPRET(r4300i_C_UEQ_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_UEQ_D(OP_PARAMS)	{		INTERPRET(r4300i_C_UEQ_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_OLT_S(OP_PARAMS)	{		INTERPRET(r4300i_C_OLT_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_OLT_D(OP_PARAMS)	{		INTERPRET(r4300i_C_OLT_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_ULT_S(OP_PARAMS)	{		INTERPRET(r4300i_C_ULT_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_ULT_D(OP_PARAMS)	{		INTERPRET(r4300i_C_ULT_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_OLE_S(OP_PARAMS)	{		INTERPRET(r4300i_C_OLE_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_OLE_D(OP_PARAMS)	{		INTERPRET(r4300i_C_OLE_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_ULE_S(OP_PARAMS)	{		INTERPRET(r4300i_C_ULE_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_ULE_D(OP_PARAMS)	{		INTERPRET(r4300i_C_ULE_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_SF_S(OP_PARAMS)	{		INTERPRET(r4300i_C_SF_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_SF_D(OP_PARAMS)	{		INTERPRET(r4300i_C_SF_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_NGLE_S(OP_PARAMS)	{		INTERPRET(r4300i_C_NGLE_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_NGLE_D(OP_PARAMS)	{		INTERPRET(r4300i_C_NGLE_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_SEQ_S(OP_PARAMS)	{		INTERPRET(r4300i_C_SEQ_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_SEQ_D(OP_PARAMS)	{		INTERPRET(r4300i_C_SEQ_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_NGL_S(OP_PARAMS)	{		INTERPRET(r4300i_C_NGL_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_NGL_D(OP_PARAMS)	{		INTERPRET(r4300i_C_NGL_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_LT_S(OP_PARAMS)	{		INTERPRET(r4300i_C_LT_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_NGE_S(OP_PARAMS)	{		INTERPRET(r4300i_C_NGE_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_LE_S(OP_PARAMS)	{		INTERPRET(r4300i_C_LE_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_LE_D(OP_PARAMS)	{		INTERPRET(r4300i_C_LE_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_NGT_S(OP_PARAMS)	{		INTERPRET(r4300i_C_NGT_S)		return(Dest);}
RETURN_TYPE dyn_r4300i_C_NGT_D(OP_PARAMS)	{		INTERPRET(r4300i_C_NGT_D)		return(Dest);}
RETURN_TYPE dyn_r4300i_COP0_tlbp(OP_PARAMS)	{		INTERPRET(r4300i_COP0_tlbp)		return(Dest);}
RETURN_TYPE dyn_r4300i_COP0_tlbr(OP_PARAMS)	{		INTERPRET(r4300i_COP0_tlbr)		return(Dest);}
RETURN_TYPE dyn_r4300i_COP0_tlbwi(OP_PARAMS){		INTERPRET(r4300i_COP0_tlbwi)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP0_tlbwr(OP_PARAMS){		INTERPRET(r4300i_COP0_tlbwr)	return(Dest);}
RETURN_TYPE dyn_r4300i_teq(OP_PARAMS)		{		INTERPRET(r4300i_teq)			return(Dest);}
RETURN_TYPE dyn_r4300i_tge(OP_PARAMS)		{		INTERPRET(r4300i_tge)			return(Dest);}
RETURN_TYPE dyn_r4300i_tgeu(OP_PARAMS)		{		INTERPRET(r4300i_tgeu)			return(Dest);}
RETURN_TYPE dyn_r4300i_tlt(OP_PARAMS)		{		INTERPRET(r4300i_tlt)			return(Dest);}
RETURN_TYPE dyn_r4300i_tltu(OP_PARAMS)		{		INTERPRET(r4300i_tltu)			return(Dest);}
RETURN_TYPE dyn_r4300i_tne(OP_PARAMS)		{		INTERPRET(r4300i_tne)			return(Dest);}
RETURN_TYPE dyn_r4300i_cache(OP_PARAMS)		{		INTERPRET(r4300i_cache)			return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_abs_s(OP_PARAMS){		INTERPRET(r4300i_COP1_abs_s)	return(Dest);}
RETURN_TYPE dyn_r4300i_COP1_abs_d(OP_PARAMS){		INTERPRET(r4300i_COP1_abs_d)	return(Dest);}
RETURN_TYPE dyn_r4300i_dsll(OP_PARAMS)		{		INTERPRET(r4300i_dsll)			return(Dest);}
RETURN_TYPE dyn_r4300i_dsllv(OP_PARAMS)		{		INTERPRET(r4300i_dsllv)			return(Dest);}
RETURN_TYPE dyn_r4300i_dsra(OP_PARAMS)		{		INTERPRET(r4300i_dsra)			return(Dest);}
RETURN_TYPE dyn_r4300i_dsrav(OP_PARAMS)		{		INTERPRET(r4300i_dsrav)			return(Dest);}
RETURN_TYPE dyn_r4300i_dsrl(OP_PARAMS)		{		INTERPRET(r4300i_dsrl)			return(Dest);}
RETURN_TYPE dyn_r4300i_dsrl32(OP_PARAMS)	{		INTERPRET(r4300i_dsrl32)		return(Dest);}
RETURN_TYPE dyn_r4300i_dsrlv(OP_PARAMS)		{		INTERPRET(r4300i_dsrlv)			return(Dest);}
RETURN_TYPE dyn_r4300i_sc(OP_PARAMS)		{		INTERPRET(r4300i_sc)			return(Dest);}
RETURN_TYPE dyn_r4300i_scd(OP_PARAMS)		{		INTERPRET(r4300i_scd)			return(Dest);}
RETURN_TYPE dyn_r4300i_lwl(OP_PARAMS)		{		INTERPRET(r4300i_lwl)			return(Dest);}
RETURN_TYPE dyn_r4300i_lwr(OP_PARAMS)		{		INTERPRET(r4300i_lwr)			return(Dest);}
RETURN_TYPE dyn_r4300i_ldl(OP_PARAMS)		{		INTERPRET(r4300i_ldl)			return(Dest);}
RETURN_TYPE dyn_r4300i_ldr(OP_PARAMS)		{		INTERPRET(r4300i_ldr)			return(Dest);}
RETURN_TYPE dyn_r4300i_sdl(OP_PARAMS)		{		INTERPRET(r4300i_sdl)			return(Dest);}
RETURN_TYPE dyn_r4300i_sdr(OP_PARAMS)		{		INTERPRET(r4300i_sdr)			return(Dest);}
RETURN_TYPE dyn_r4300i_swl(OP_PARAMS)		{		INTERPRET(r4300i_swl)			return(Dest);}
RETURN_TYPE dyn_r4300i_swr(OP_PARAMS)		{		INTERPRET(r4300i_swr)			return(Dest);}
RETURN_TYPE dyn_r4300i_daddi(OP_PARAMS)		{		INTERPRET(r4300i_daddi)			return(Dest);}
RETURN_TYPE dyn_r4300i_daddiu(OP_PARAMS)	{		INTERPRET(r4300i_daddiu)		return(Dest);}

#endif //#ifndef SAFE_DYNAREC