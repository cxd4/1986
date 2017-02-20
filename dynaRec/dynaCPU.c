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
*/

//Notes:
//===================================================
// - increment PassIndex for each opcode using multipass (same op can occur multiple times in one block)

//Note: If we don't do MapOneConstantToRegister in MapRegister, we can optimize the MOV step for constants.
//- 3 major steps: 
//      Map   when necessary
//      MOV   when necessary
//      do op when necessary
//- other steps
//      translate 64/32 (vice versa)
//           - first find a demo that doesn't have an interrupt for a bne loop (N64stars?)
//             and patch the map (64/32, 32/64 and regs needing reload) to jump directly
//      flush
int Two_Pass_Compiler = 0;
int PassIndex = 0;
int NextPass[14]; //assign to sthg, or msvc crapola?

//-- Notes --
//Always map the dirty reg first b4 the others

#define DisplayAddress DisplayError("lCodePosition=%08X", &RecompCode[lCodePosition]);
#define CheckForConstMap  if ((ConstMap[xRD->mips_reg].IsMapped) || (ConstMap[xRS->mips_reg].IsMapped) || (ConstMap[xRT->mips_reg].IsMapped)) DisplayError("whui");

#define INTERPRET_LOADSTORE(OPCODE) {                                       \
        int temp;                                                           \
        if (ConstMap[__RS].IsMapped) FlushOneConstant(__RS);                \
        if (ConstMap[__RT].IsMapped) FlushOneConstant(__RT);                \
        if ((temp = CheckWhereIsMipsReg(__RS)) > -1)  FlushRegister(temp);  \
        if ((temp = CheckWhereIsMipsReg(__RT)) > -1)  FlushRegister(temp);  \
        PushMap();                                                          \
        MOV_ImmToReg(1, Reg_ECX, reg->code);                                \
        MOV_ImmToReg(1, Reg_EAX, (uint32)&OPCODE);                          \
        CALL_Reg(Reg_EAX);                                                  \
        PopMap();                                                           \
}

#define INTERPRET(OPCODE)                       \
    FlushAllRegisters();                        \
    MOV_ImmToReg(1, Reg_ECX, reg->code);        \
    MOV_ImmToReg(1, Reg_EAX, (uint32)&OPCODE);  \
    CALL_Reg(Reg_EAX);


#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include "../options.h"
#include "../hardware.h"
#include "../r4300i.h"
#include "../n64rcp.h"
#include "regcache.h"
#include "../interrupt.h"
#include "x86.h"
#include "dynaCPU.h"
#include "dynaCPU_defines.h"
#include "dynaLog.h"
#include "../emulator.h"
extern void RefreshOpList(char *opcode);


#define INTERPRET_STORE_NEW(OPCODE) Interpret_Store_New((uint32)&OPCODE);
void Interpret_Store_New(unsigned _int32 OPCODE)
{
        int temp, temp2;

        if (ConstMap[__dotRS].IsMapped)
        {
            temp = ConstMap[__dotRS].value;
            if (ConstMap[__dotRT].IsMapped) FlushOneConstant(__dotRT);
            if ((temp2 = CheckWhereIsMipsReg(__dotRT)) > -1)  FlushRegister(temp2);
            PUSH_RegIfMapped(Reg_EAX);
            PUSH_RegIfMapped(Reg_ECX);
            PUSH_RegIfMapped(Reg_EDX);

            MOV_ImmToReg(1, Reg_EDX, __dotRT);
            MOV_ImmToReg(1, Reg_ECX, temp+(_s32)(_s16)__dotI);
            MOV_ImmToReg(1, Reg_EAX, (uint32)OPCODE);
            CALL_Reg(Reg_EAX);
            POP_RegIfMapped(Reg_EDX);
            POP_RegIfMapped(Reg_ECX);
            POP_RegIfMapped(Reg_EAX);
        }
        else
        {
        if (ConstMap[__dotRS].IsMapped) FlushOneConstant(__dotRS);
        if (ConstMap[__dotRT].IsMapped) FlushOneConstant(__dotRT);
        if ((temp = CheckWhereIsMipsReg(__dotRS)) > -1)  FlushRegister(temp);
        if ((temp = CheckWhereIsMipsReg(__dotRT)) > -1)  FlushRegister(temp);
            PUSH_RegIfMapped(Reg_EAX);
            PUSH_RegIfMapped(Reg_ECX);
            PUSH_RegIfMapped(Reg_EDX);

            MOV_MemoryToReg(1, Reg_ECX, ModRM_disp32, (_u32)&gHardwareState.GPR[__dotRS]);
            MOV_ImmToReg(1, Reg_EDX, __dotRT);
            ADD_ImmToReg(1, Reg_ECX, (_s32)(_s16)__dotI);
            MOV_ImmToReg(1, Reg_EAX, (uint32)OPCODE);
            CALL_Reg(Reg_EAX);
            POP_RegIfMapped(Reg_EDX);
            POP_RegIfMapped(Reg_ECX);
            POP_RegIfMapped(Reg_EAX);
        }
}

void Interpret_Load_New(unsigned _int32 OPCODE, x86regtyp* xRD, x86regtyp* xRS, x86regtyp* xRT)
{
    if (ConstMap[xRS->mips_reg].IsMapped)
    {
        int temp = ConstMap[xRS->mips_reg].value;

        xRT->IsDirty = 1;
        xRT->NoNeedToLoadTheLo = 1;
        MapRT;
        if (xRT->x86reg != Reg_EAX) PUSH_RegIfMapped(Reg_EAX);
        if (xRT->x86reg != Reg_ECX) PUSH_RegIfMapped(Reg_ECX);
        if (xRT->x86reg != Reg_EDX) PUSH_RegIfMapped(Reg_EDX);

        MOV_ImmToReg(1, Reg_ECX, temp+(_s32)(_s16)__dotI);
        MOV_ImmToReg(1, Reg_EAX, (uint32)OPCODE);
        CALL_Reg(Reg_EAX);  MOV_Reg2ToReg1(1, xRT->x86reg, Reg_EAX);

        if (xRT->x86reg != Reg_EDX) POP_RegIfMapped(Reg_EDX);
        if (xRT->x86reg != Reg_ECX) POP_RegIfMapped(Reg_ECX);
        if (xRT->x86reg != Reg_EAX) POP_RegIfMapped(Reg_EAX);
    }
    else
    {
        xRT->IsDirty = 1;
        if (xRT->mips_reg != xRS->mips_reg) xRT->NoNeedToLoadTheLo = 1;
        MapRT; MapRS;
        if (xRT->x86reg != Reg_EAX) PUSH_RegIfMapped(Reg_EAX);
        if (xRT->x86reg != Reg_ECX) PUSH_RegIfMapped(Reg_ECX);
        if (xRT->x86reg != Reg_EDX) PUSH_RegIfMapped(Reg_EDX);

        MOV_Reg2ToReg1(1, Reg_ECX, xRS->x86reg);
        ADD_ImmToReg(1, Reg_ECX, (_s32)(_s16)__dotI);
        MOV_ImmToReg(1, Reg_EAX, (uint32)OPCODE);
        CALL_Reg(Reg_EAX); MOV_Reg2ToReg1(1, xRT->x86reg, Reg_EAX);

        if (xRT->x86reg != Reg_EDX) POP_RegIfMapped(Reg_EDX);
        if (xRT->x86reg != Reg_ECX) POP_RegIfMapped(Reg_ECX);
        if (xRT->x86reg != Reg_EAX) POP_RegIfMapped(Reg_EAX);
    }
}

#ifdef ENABLE_OPCODE_DEBUGGER
void COMPARE_Run(uint32 Inter_Opcode_Address);
#define _SAFTY_CPU_(x)  COMPARE_Run((uint32)&x);
#else
#define _SAFTY_CPU_(x)
//#define _SAFTY_CPU_(x)  INTERPRET(x); return;
#endif
#define _OPCODE_DEBUG_BRANCH_(x) _SAFTY_CPU_(x)


uint32 PositionAfterMap[0x10000];
uint32 TargetIndex;
#define asm_END PositionAfterMap[(uint16)gHWS_pc] = lCodePosition;  // This indicates where the mapping code of an instruction ends
                                                                              // so if we want to jump to this instruction from within the same page,
                                                                              // we don't need to map all over again.

// prototypes
extern BOOL CPUIsRunning;
extern int __cdecl SetStatusBar(char *debug, ...);
extern void HELP_debug(unsigned long pc);
extern _u32 ThisYear;

extern void FlushConstants();
extern void FlushRegister(int k);
extern void MapConst(x86regtyp* xMAP, int value);
int CheckWhereIsMipsReg(int mips_reg);


#include "dynaHelper.h"     // branches and jumps   interpretive
#include "dynabranch.h"     // branches and jumps

//MapConstant ConstMap[32];
FlushedMap  FlushedRegistersMap[NUM_CONSTS];

_u32 translatepc;
int AlreadyRecompiled;

x86regtyp   xRD[1];
x86regtyp   xRS[1];
x86regtyp   xRT[1];
x86regtyp   xLO[1];
x86regtyp   xHI[1];


dyn_cpu_instr now_do_dyna_instruction[64] =
{
    dyna4300i_special,  dyna4300i_regimm,   dyna4300i_j,    dyna4300i_jal,      dyna4300i_beq,      dyna4300i_bne,      dyna4300i_blez,     dyna4300i_bgtz,
    dyna4300i_addi,     dyna4300i_addiu,    dyna4300i_slti, dyna4300i_sltiu,    dyna4300i_andi,     dyna4300i_ori,      dyna4300i_xori,     dyna4300i_lui,
    dyna4300i_cop0,     dyna4300i_cop1,     dyna4300i_cop2, dyna4300i_reserved, dyna4300i_beql,     dyna4300i_bnel,     dyna4300i_blezl,    dyna4300i_bgtzl,
    dyna4300i_daddi,    dyna4300i_daddiu,   dyna4300i_ldl,  dyna4300i_ldr,      dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved,
    dyna4300i_lb,       dyna4300i_lh,       dyna4300i_lwl,  dyna4300i_lw,       dyna4300i_lbu,      dyna4300i_lhu,      dyna4300i_lwr,      dyna4300i_lwu,
    dyna4300i_sb,       dyna4300i_sh,       dyna4300i_swl,  dyna4300i_sw,       dyna4300i_sdl,      dyna4300i_sdr,      dyna4300i_swr,      dyna4300i_cache,
    dyna4300i_ll,       dyna4300i_lwc1,     dyna4300i_lwc2, dyna4300i_reserved, dyna4300i_lld,      dyna4300i_ldc1,     dyna4300i_ldc2,     dyna4300i_ld,
    dyna4300i_sc,       dyna4300i_swc1,     dyna4300i_swc2, dyna4300i_reserved, dyna4300i_scd,      dyna4300i_sdc1,     dyna4300i_sdc2,     dyna4300i_sd
};


#ifdef ENABLE_OPCODE_DEBUGGER
extern char* DebugPrintInstr(uint32 Instruction);
extern char op_str[0xff];

void COMPARE_SwitchToInterpretive()
{
    //Setup for interpreter
    sDWord_ptr = (uint8**)&sDWORD_R__Debug;
    sDWord2_ptr = (uint8**)&sDWORD_R_2__Debug;
#ifdef DIRECT_TLB_LOOKUP
	//TLB_sDWord_ptr = (uint8**)&TLB_sDWord__Debug;
	TLB_sDWord_ptr = (uint8**)&TLB_sDWord;
#endif

	p_gMemoryState = &gMemoryState_Interpreter_Compare;
    p_gHardwareState = &gHardwareState_Interpreter_Compare;
}

void COMPARE_SwitchToDynarec()
{
    //Setup for dyna
    sDWord_ptr = (uint8**)&sDWord;
    sDWord2_ptr = (uint8**)&sDWord2;
#ifdef DIRECT_TLB_LOOKUP
	TLB_sDWord_ptr = (uint8**)&TLB_sDWord;
#endif

	p_gMemoryState = &gMemoryState;
    p_gHardwareState = &gHardwareState;
}

void CompareStates1(uint32 Instruction)
{
    memcpy(&gHardwareState_Flushed_Dynarec_Compare, &gHardwareState, sizeof(gHardwareState));
}

char op_Str[0xffff];
void CompareStates(uint32 Instruction)
{
    int reg, errcount=0;
    int OutputDisplayed = 0;
    int GPR_array_size = sizeof(gHardwareState.GPR);

	gHardwareState_Interpreter_Compare.COP0Reg[COUNT] = gHardwareState.COP0Reg[COUNT];

	if( debug_opcode_block == 1 )
	{
		if ( memcmp(&gHardwareState, &gHardwareState_Interpreter_Compare, GPR_array_size-8 ) != 0 )
		{
			errcount = 1;
			memcpy(&gHardwareState_Flushed_Dynarec_Compare, &gHardwareState, GPR_array_size-8 );
		}
	}
	else
	{
		if ( memcmp(&gHardwareState_Flushed_Dynarec_Compare, &gHardwareState_Interpreter_Compare, GPR_array_size-8 ) != 0 )
		{
			errcount = 1;
		}
	}


    // -8 to ignore gHardwareState.code..so code must be last item in struct!
    //memcmp should be faster, but not sure yet.

	if( errcount > 0 )
    {
       //This part needs no optimization because it's only executed when memcmp != 0
/*
       //Is it PC ?
       if (gHardwareState.pc != gHardwareState_Interpreter_Compare.pc)
       {
			if( gHardwareState.pc == CPUdelayPC )
			{
				gHardwareState_Interpreter_Compare.pc = gHardwareState.pc;	//don't compare PC at this moment
				if ( memcmp(&gHardwareState_Flushed_Dynarec_Compare, &gHardwareState_Interpreter_Compare, sizeof(gHardwareState)-8 ) == 0 )
				{
//				    memcpy(&gHardwareState, &gHardwareState_Dynarec_Compare, sizeof(gHardwareState));
					return;
				}
			}
			else
			{
				sprintf(op_Str, "%s\n%08X: Dynarec and Interpretive PCs do not match.\n \
				Inter PC = %08X\n \
				Dyna PC = %08X\n\n%s",
				DebugPrintInstr(Instruction), gHardwareState.pc, 
				gHardwareState_Interpreter_Compare.pc,
				gHardwareState.pc, 
				op_Str
				);
				OutputDisplayed = 1;
		   }
       }
*/
       //Which GPR is it ?
       for (reg = sizeof(gHardwareState.GPR)/8; reg>=0; reg--)
       if ( gHardwareState_Flushed_Dynarec_Compare.GPR[reg] != gHardwareState_Interpreter_Compare.GPR[reg])
       {
            sprintf(op_Str, "%s\n%08X: Dynarec and Interpretive GPR do not match.\n \
			InterGPR[%d] = %016I64X\n \
			DynaGPR[%d] = %016I64X\n\n%s", 
			DebugPrintInstr(Instruction), gHardwareState.pc, 
			reg, gHardwareState_Interpreter_Compare.GPR[reg],
			reg, gHardwareState_Flushed_Dynarec_Compare.GPR[reg],
			op_Str
			);
            OutputDisplayed = 1;
       }
    }

    if ( memcmp(&gHardwareState+GPR_array_size, &gHardwareState_Interpreter_Compare+GPR_array_size, sizeof(gHardwareState)-(GPR_array_size+8) ) != 0 )
    {
       errcount = 1;

       //Is it COP0Reg ?
       for (reg = sizeof(gHardwareState.COP0Reg)/4; reg>=0; reg--)
       if ( gHardwareState.COP0Reg[reg] != gHardwareState_Interpreter_Compare.COP0Reg[reg] )
       {
            sprintf(op_Str, "%s\n%08X: Dynarec and Interpretive COP0Reg do not match.\n \
            InterCOP0Reg[%d] = %08X\n \
            DynaCOP0Reg[%d] = %08X\n\n%s",
			DebugPrintInstr(Instruction), gHardwareState.pc,
			reg, gHardwareState_Interpreter_Compare.COP0Reg[reg],
			reg, gHardwareState.COP0Reg[reg],
			op_Str
			);
            OutputDisplayed = 1;
       }

       //Is it COP0Con ?
       for (reg = sizeof(gHardwareState.COP0Con)/4; reg>=0; reg--)
       if ( gHardwareState.COP0Con[reg] != gHardwareState_Interpreter_Compare.COP0Con[reg])
       {
            sprintf(op_Str, "%s\n%08X: Dynarec and Interpretive COP0Con do not match.\n \
			InterCOP0Con[%d] = %08X\n \
			DynaCOP0Con[%d] = %08X\n\n%s", 
			DebugPrintInstr(Instruction), gHardwareState.pc,
			reg, gHardwareState_Interpreter_Compare.COP0Con[reg],
			reg, gHardwareState.COP0Con[reg],
			op_Str
			);
            OutputDisplayed = 1;
       }

       //Is it COP1Con ?
       for (reg = sizeof(gHardwareState.COP1Con)/4; reg>=0; reg--)
       if ( gHardwareState.COP1Con[reg] != gHardwareState_Interpreter_Compare.COP1Con[reg])
       {
            sprintf(op_Str, "%s\n%08X: Dynarec and Interpretive COP1Con do not match.\n \
			InterCOP1Con[%d] = %08X\n \
			DynaCOP1Con[%d] = %08X\n\n%s", 
			DebugPrintInstr(Instruction), gHardwareState.pc,
			reg, gHardwareState_Interpreter_Compare.COP1Con[reg],
			reg, gHardwareState.COP1Con[reg],
			op_Str
			);
            OutputDisplayed = 1;
       }
    
       //Is it FPR32 ?
		//for (reg = sizeof(gHardwareState.fpr32)/4; reg>=0; reg--)
		for (reg = sizeof(gHardwareState.fpr32)/8; reg>=0; reg--)
       //if ( gHardwareState.fpr32[reg] != *(uint64*)&gHardwareState_Interpreter_Compare.fpr32[reg])
	   if ( *(uint64*)&gHardwareState.fpr32[reg] != *(uint64*)&gHardwareState_Interpreter_Compare.fpr32[reg])
       {
            sprintf(op_Str, "%s\n%08X: Dynarec and Interpretive fpr32 do not match.\n \
			Inter_fpr32[%d][] = %016I64X\n \
			Dyna_fpr32[%d][] = %016I64X\n\n%s", 
			DebugPrintInstr(Instruction), gHardwareState.pc,
			reg, *(uint64*)&gHardwareState_Interpreter_Compare.fpr32[reg],
			reg, *(uint64*)&gHardwareState.fpr32[reg],
			op_Str
			);
            OutputDisplayed = 1;
       }

       //Is it LLbit ?
       if ( gHardwareState.LLbit != gHardwareState_Interpreter_Compare.LLbit)
       {
		    DebugPrintInstr(Instruction);     
            
            sprintf(op_Str, "%s\n%08X: Dynarec and Interpretive LLbit do not match.\n \
			InterLLbit = %08X\n \
			DynaLLbit = %08X\n\n%s", 
			DebugPrintInstr(Instruction), gHardwareState.pc,
			gHardwareState_Interpreter_Compare.LLbit,
			reg, gHardwareState.LLbit,
			op_Str
	        );
            OutputDisplayed = 1;
        }

        //Is it RememberFprHi ?
        for (reg = sizeof(gHardwareState.RememberFprHi)/4; reg>=0; reg--)
        if ( gHardwareState.RememberFprHi[reg] != gHardwareState_Interpreter_Compare.RememberFprHi[reg])
        {
		    DebugPrintInstr(Instruction);     

            sprintf(op_Str, "%s\n%08X: Dynarec and Interpretive RemeberFprHi do not match.\n \
			Inter_RememberFprHi[%d] = %08X\n \
			Dyna_RememberFprHi[%d] = %08X\n\n%s", 
			DebugPrintInstr(Instruction), gHardwareState.pc,
			reg, gHardwareState_Interpreter_Compare.RememberFprHi[reg],
			reg, gHardwareState.RememberFprHi[reg],
			op_Str
			);
            OutputDisplayed = 1;
        }

        //ignore for now. TODO: FixMe!
        if (!OutputDisplayed); else 


      	if( errcount > 0 )
		{
			int val=MessageBox(NULL,op_Str,"Error",MB_YESNOCANCEL|MB_ICONINFORMATION|MB_SYSTEMMODAL);
            
		    strcat(op_Str, "\nYes-Copy Dyna to Interpreter,  No->Copy Interpreter to Dyna, Cancel->Do nothing");
    
            if (!OutputDisplayed)
                DisplayError("Missed a message output.");

            memset(op_Str, 0, sizeof(op_Str));

			if( val == IDYES )
			{
				Debugger_Copy_Memory(&gMemoryState_Interpreter_Compare, &gMemoryState);
				//memcpy(&gMemoryState_Interpreter_Compare, &gMemoryState, sizeof(gHardwareState));
				memcpy(&gHardwareState_Interpreter_Compare, &gHardwareState_Flushed_Dynarec_Compare, sizeof(gHardwareState));
			}
			else if( val == IDNO )
			{
				memcpy(&gHardwareState, &gHardwareState_Interpreter_Compare, sizeof(gHardwareState));
				Debugger_Copy_Memory(&gMemoryState, &gMemoryState_Interpreter_Compare);
                //memcpy(&gMemoryState,   &gMemoryState_Interpreter_Compare, sizeof(gHardwareState));
				memset(op_Str, 0, sizeof(op_Str));
				return;
			}
		}
    }
}

extern void (*CPU_instruction[64])(uint32 Instruction);
void COMPARE_Run_Interpreter_Opcode(uint32 pc)
{
	uint32 Instruction;
	__try{
		Instruction = LOAD_UWORD_PARAM(pc);
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(pc) ) 
		{                                                   
			uint32 translatepc = TranslateITLBAddress(pc);
			__try{
				Instruction = LOAD_UWORD_PARAM(translatepc);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DisplayError("%08X: PC out of range", pc);
			}
		}
		else
			DisplayError("%08X: PC out of range", pc);
	}
    CPU_instruction[_OPCODE_](Instruction);
}

void COMPARE_Run(uint32 Inter_Opcode_Address)
{
	if( debug_opcode == 0 ) return;

    PushMap();

    MOV_ImmToReg(1, Reg_EAX, (_u32)&COMPARE_SwitchToInterpretive);
	CALL_Reg(Reg_EAX);
    //MOV_ImmToReg(1, Reg_ECX, gHardwareState.code);
    //MOV_ImmToReg(1, Reg_EAX, (_u32)Inter_Opcode_Address);
    MOV_ImmToReg(1, Reg_ECX, gHardwareState.pc);
    MOV_ImmToReg(1, Reg_EAX, (_u32)COMPARE_Run_Interpreter_Opcode);
    CALL_Reg(Reg_EAX); //disable this line to test integrity of dyna portion

    MOV_ImmToReg(1, Reg_EAX, (_u32)&COMPARE_SwitchToDynarec);
    CALL_Reg(Reg_EAX);  
    PopMap(); 
}

_int32 RegisterRecall[8];

void WriteBackDirtyToDynaCompare(_int8 k)
{
    //32 bit register
    if ((x86reg[k].HiWordLoc == k))
    {
        if (x86reg[k].Is32bit != 1)
            DisplayError("Bug");

       MOV_RegToMemory(1, k,  ModRM_disp32,   (_u32)&gHardwareState_Flushed_Dynarec_Compare.GPR[0]+(x86reg[k].mips_reg<<3));
       SAR_RegByImm(1, k, 31);
       MOV_RegToMemory(1, x86reg[k].HiWordLoc, ModRM_disp32, 4+(_u32)&gHardwareState_Flushed_Dynarec_Compare.GPR[0]+(x86reg[k].mips_reg<<3));

    }
    else
    //64 bit register
    {
        if (x86reg[k].Is32bit == 1)
            DisplayError("Bug");

        MOV_RegToMemory(1, k,  ModRM_disp32,   (_u32)&gHardwareState_Flushed_Dynarec_Compare.GPR[0]+(x86reg[k].mips_reg<<3));
        MOV_RegToMemory(1, x86reg[k].HiWordLoc, ModRM_disp32, 4+(_u32)&gHardwareState_Flushed_Dynarec_Compare.GPR[0]+(x86reg[k].mips_reg<<3));
    }
}

void x86reg_Delete(int k);
void FlushRegisterToDynaCompare(int k)
{
#ifdef DEBUG_REGCACHE
    //paranoid error check
    if (x86reg[k].HiWordLoc == -1)
        DisplayError("FlushRegister: The HiWord was not set!");
#endif

#ifdef R0_COMPENSATION
    //Until we don't map all r0's, we'll need this check
    //if (ConstMap[0].IsMapped == 0) DisplayError("How did Const[0] get unmapped???");
    if (x86reg[k].mips_reg == 0) x86reg[k].IsDirty = 0;
#endif

    if (x86reg[k].IsDirty == 1)
        WriteBackDirtyToDynaCompare((_s8)k);

    x86reg_Delete(x86reg[k].HiWordLoc);
    x86reg_Delete(k);
}

void FlushRegisterToDynaCompare_NoUnmap(int k)
{
#ifdef DEBUG_REGCACHE
    //paranoid error check
    if (x86reg[k].HiWordLoc == -1)
        DisplayError("FlushRegister: The HiWord was not set!");
#endif

#ifdef R0_COMPENSATION
    //Until we don't map all r0's, we'll need this check
    //if (ConstMap[0].IsMapped == 0) DisplayError("How did Const[0] get unmapped???");
    if (x86reg[k].mips_reg == 0) x86reg[k].IsDirty = 0;
#endif

    if (x86reg[k].IsDirty == 1)
        WriteBackDirtyToDynaCompare((_s8)k);

//    x86reg_Delete(x86reg[k].HiWordLoc);
//    x86reg_Delete(k);
}



void FlushAllRegistersToDynaCompare()
{
    _int8 k;
    const Num_x86regs = 8;

 //   FlushConstants(); //schibo: need to work on this for opcode debugger when using constants!!
    
    for(k=0; k<Num_x86regs; k++)
    {
        if (ItIsARegisterNotToUse(k));
        else if (x86reg[k].mips_reg > -1)
            FlushRegisterToDynaCompare(k);
    }
 //   ThisYear = 2001;   
 //   for (k=1; k<NUM_CONSTS; k++)
   //     FlushedRegistersMap[k].Is32bit = 0;
}


#ifdef _DEBUG
extern void UpdateGPR();
extern void UpdateCOP0();
extern void UpdateFPR();
extern void UpdateMisc();
extern void UpdateVIReg();
#endif

_int32 bad_mips_reg=0;
void COMPARE_ConstErrorMessage(uint32 pc)
{
    DisplayError("Consts do not match.");
    //TODO: call patcher. Put patcher in its own function.
}

void COMPARE_SomeErrorMessage(uint32 pc)
{
    uint32 Instruction;

    __try{
        Instruction = LOAD_UWORD_PARAM(pc);
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
    if( NOT_IN_KO_K1_SEG(pc) )
    {
        uint32 translatepc = TranslateITLBAddress(pc);
        __try{
            Instruction = LOAD_UWORD_PARAM(translatepc);
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("%08X: PC out of range", pc);
        }
    }
    else
        DisplayError("%08X: PC out of range", pc);
    }


#ifdef _DEBUG
    FlushAllRegisters();
    UpdateGPR();
    UpdateCOP0();
    UpdateFPR();
    UpdateMisc();
    UpdateVIReg();
#endif

    //This COP1Con[31] check needs to be one of the first because I set bad_mips_reg to a negative value.
    if (gHardwareState_Interpreter_Compare.LLbit != gHardwareState.LLbit)
    sprintf(op_Str, "%s\n%08X: COP1Con[31] mismatch.\n \
        COP1Con[31] = %08X\n \
        COP1Con[31] = %08X\n\n",
        DebugPrintInstr(Instruction), pc,
        bad_mips_reg, gHardwareState_Interpreter_Compare.LLbit,
        bad_mips_reg, gHardwareState.LLbit);
    //This COP1Con[31] check needs to be one of the first because I set bad_mips_reg to a negative value.
    else if (gHardwareState_Interpreter_Compare.COP1Con[31] != gHardwareState.COP1Con[31])
    sprintf(op_Str, "%s\n%08X: COP1Con[31] mismatch.\n \
        COP1Con[31] = %08X\n \
        COP1Con[31] = %08X\n\n",
        DebugPrintInstr(Instruction), pc,
        bad_mips_reg, gHardwareState_Interpreter_Compare.COP1Con[31],
        bad_mips_reg, gHardwareState.COP1Con[31]);
    else if (gHardwareState_Interpreter_Compare.GPR[bad_mips_reg] != gHardwareState.GPR[bad_mips_reg])
    sprintf(op_Str, "%s\n%08X: Dirty GPR mismatch.\n \
        InterGPR[%d] = %016I64X\n \
        DynaGPR[%d] = %016I64X\n\n",
        DebugPrintInstr(Instruction), pc,
        bad_mips_reg, gHardwareState_Interpreter_Compare.GPR[bad_mips_reg],
        bad_mips_reg, gHardwareState.GPR[bad_mips_reg]);
    else if (gHardwareState_Interpreter_Compare.fpr32[bad_mips_reg] != gHardwareState.fpr32[bad_mips_reg])
    sprintf(op_Str, "%s\n%08X: Dirty FPR mismatch.\n \
        InterFPR[%d] = %016I64X\n \
        DynaFPR[%d] = %016I64X\n\n",
        DebugPrintInstr(Instruction), pc,
        bad_mips_reg, gHardwareState_Interpreter_Compare.fpr32[bad_mips_reg],
        bad_mips_reg, gHardwareState.fpr32[bad_mips_reg]);
    else
    sprintf(op_Str, "%08X: COP0 mismatch.\n \
        InterCOP0[%d] = %08X\n \
        DynaCOP0[%d] = %08X\n\n",
        pc,
        bad_mips_reg, gHardwareState_Interpreter_Compare.COP0Reg[bad_mips_reg],
        bad_mips_reg, gHardwareState.COP0Reg[bad_mips_reg]);

    {
        int val;
		TRACE0(op_Str);
		strcat(op_Str, "\nYes-Copy Dyna to Interpreter,  No->Do nothing");
        val=MessageBox(NULL,op_Str,"Error",MB_YESNO|MB_ICONINFORMATION|MB_SYSTEMMODAL);

        memset(op_Str, 0, sizeof(op_Str));

        if( val == IDYES )
        {
            Debugger_Copy_Memory(&gMemoryState_Interpreter_Compare, &gMemoryState);
            //memcpy(&gMemoryState_Interpreter_Compare, &gMemoryState, sizeof(gHardwareState));
            //we don't know which chip it was, so for now let's just do this:
            memcpy(&gHardwareState_Interpreter_Compare.GPR[bad_mips_reg], &gHardwareState_Flushed_Dynarec_Compare.GPR[bad_mips_reg], 4);
            memcpy(&gHardwareState_Interpreter_Compare.GPR[bad_mips_reg]+4, &gHardwareState.GPR[bad_mips_reg]+4, 4);
            memcpy(&gHardwareState_Interpreter_Compare.fpr32[bad_mips_reg], &gHardwareState.fpr32[bad_mips_reg], 4);
            memcpy(&gHardwareState_Interpreter_Compare.fpr32[bad_mips_reg]+4, &gHardwareState.fpr32[bad_mips_reg]+4, 4);
        }
    }

    //DisplayError("%08X: Dirty reg does not match", pc);
}


void COMPARE_DebugDirtyConst(uint32 mips_reg)
{
    CMP_MemoryWithImm(1, (_u32)&gHardwareState_Interpreter_Compare.GPR[mips_reg],
        ConstMap[mips_reg].value);
    Jcc_auto(CC_NE, 20);

    CMP_MemoryWithImm(1, (_u32)&gHardwareState_Interpreter_Compare.GPR[mips_reg]+4,
        ((_s32)ConstMap[mips_reg].value>>31));
    Jcc_auto(CC_NE, 21);

    //Passed
    JMP_Short_auto(22);

    //Failed
    SetTarget(20);
    SetTarget(21);
    MOV_ImmToMemory(1, (uint32)&bad_mips_reg, (uint32)mips_reg);
    
    //FlushRegisterToDynaCompare_NoUnmap(xCMP->mips_reg);

    PUSHAD();
    MOV_ImmToReg(1, Reg_ECX, gHardwareState.pc);
    MOV_ImmToReg(1, Reg_EAX, (uint32)&COMPARE_ConstErrorMessage);
    CALL_Reg(Reg_EAX);
    POPAD();

    SetTarget(22);
}


// Debug the Dirty only
void COMPARE_DebugDirty(x86regtyp* xCMP)
{
    //Compare the lo
    CMP_RegWithMemory(xCMP->x86reg, (_u32)&gHardwareState_Interpreter_Compare.GPR[xCMP->mips_reg]);
    Jcc_auto(CC_NE, 41);


    //Compare the hi
    if (xCMP->Is32bit)
    {
        PUSH_RegToStack(xCMP->x86reg);
        SAR_RegByImm(1, xCMP->x86reg, 31);
        CMP_RegWithMemory(xCMP->x86reg, (_u32)&gHardwareState_Interpreter_Compare.GPR[xCMP->mips_reg]+4);
        POP_RegFromStack(xCMP->x86reg);
        Jcc_auto(CC_NE, 43);
    }
    else
    {
        CMP_RegWithMemory(xCMP->HiWordLoc, (_u32)&gHardwareState_Interpreter_Compare.GPR[xCMP->mips_reg]+4);
        Jcc_auto(CC_NE, 43);
    }

    //Passed
    JMP_Short_auto(42);

    //Failed
    SetTarget(41);
    SetTarget(43);
    MOV_ImmToMemory(1, (uint32)&bad_mips_reg, (uint32)xCMP->mips_reg);
    
    FlushRegisterToDynaCompare_NoUnmap(xCMP->mips_reg);

    PUSHAD();
    MOV_ImmToReg(1, Reg_ECX, gHardwareState.pc);
    MOV_ImmToReg(1, Reg_EAX, (uint32)&COMPARE_SomeErrorMessage);
    CALL_Reg(Reg_EAX);
    POPAD();


    SetTarget(42);
}


//Runtime test
int COMPARE_DebugDirty_TheRest(uint32 fd, uint32 fs, uint32 ft)
{
        if (gHardwareState_Interpreter_Compare.fpr32[ft]   != gHardwareState.fpr32[ft])
        return(ft);
    else if (gHardwareState_Interpreter_Compare.fpr32[ft+1] != gHardwareState.fpr32[ft+1])
        return(ft);
    else if (gHardwareState_Interpreter_Compare.fpr32[fs]   != gHardwareState.fpr32[fs])
        return(fs);
    else if (gHardwareState_Interpreter_Compare.fpr32[fs+1] != gHardwareState.fpr32[fs+1])
        return(fs);
    else if (gHardwareState_Interpreter_Compare.fpr32[fd]   != gHardwareState.fpr32[fd])
        return(fd);
    else if (gHardwareState_Interpreter_Compare.fpr32[fd+1] != gHardwareState.fpr32[fd+1])
        return(fd);
    else if (gHardwareState_Interpreter_Compare.COP1Con[31] != gHardwareState.COP1Con[31])
        return(-1);
    else if (gHardwareState_Interpreter_Compare.LLbit != gHardwareState.LLbit)
        return(-2);


    // patches for CP0 discrepancy
    else if (gHardwareState_Interpreter_Compare.COP0Reg[COUNT] != gHardwareState.COP0Reg[COUNT])
        memcpy(&gHardwareState_Interpreter_Compare.COP0Reg[COUNT], &gHardwareState.COP0Reg[COUNT], 4);
    else if (gHardwareState_Interpreter_Compare.COP0Reg[STATUS] != gHardwareState.COP0Reg[STATUS])
        memcpy(&gHardwareState_Interpreter_Compare.COP0Reg[STATUS], &gHardwareState.COP0Reg[STATUS], 4);
    
    
    return(0);
}

void COMPARE_DebugDirtyFPU(OP_PARAMS)
{
    PUSHAD();
    
    //set parameters to pass
    PUSH_WordToStack(__FT);
    MOV_ImmToReg(1, Reg_ECX, (uint32)__FS);
    MOV_ImmToReg(1, Reg_EDX, (uint32)__FD);

    //now call the function
    MOV_ImmToReg(1, Reg_EAX, (uint32)&COMPARE_DebugDirty_TheRest);
    CALL_Reg(Reg_EAX);

    //get return value in eax
    MOV_RegToMemory(1, Reg_EAX, ModRM_disp32, (uint32)&bad_mips_reg);
    TEST_Reg2WithReg1(1, Reg_EAX, Reg_EAX);
    POPAD();
    Jcc_Near_auto(CC_NZ, 41); /* jmp false */

    //Passed
    JMP_Short_auto(42);

    //Failed
    SetTarget(41);
        
    PUSHAD();
    MOV_ImmToReg(1, Reg_ECX, gHardwareState.pc);
    MOV_ImmToReg(1, Reg_EAX, (uint32)&COMPARE_SomeErrorMessage);
    CALL_Reg(Reg_EAX);
    POPAD();


    SetTarget(42);
}

void OpcodeDebugger(OP_PARAMS)
{    
    int k;

    if( debug_opcode == 0 || debug_opcode_block == 1 )
	{
		if( debug_opcode == 1 )
			if( !CompilingSlot )
				MOV_ImmToMemory(1, (unsigned long)&gHardwareState_Interpreter_Compare.pc, gHWS_pc);
		now_do_dyna_instruction[__OPCODE](PASS_PARAMS);
		return;
	}
	
	if( !CompilingSlot )
		MOV_ImmToMemory(1, (unsigned long)&gHardwareState_Interpreter_Compare.pc, gHWS_pc);

    if (debug_dirty_only)
    {
        memset(xRD, 0, sizeof(xRD));
        memset(xRS, 0, sizeof(xRS));
        memset(xRT, 0, sizeof(xRT));
    }

    PUSH_RegToStack(Reg_EBP);
    MOV_MemoryToReg(1, Reg_EBP, ModRM_disp32, (uint32)&gHardwareState.COP0Reg[COUNT]);
    MOV_RegToMemory(1, Reg_EBP, ModRM_disp32, (uint32)&gHardwareState_Interpreter_Compare.COP0Reg[COUNT]);
    POP_RegFromStack(Reg_EBP);



    now_do_dyna_instruction[__OPCODE](PASS_PARAMS);

    if( currentromoptions.Use_Register_Caching == USEREGC_NO )
	    FlushAllRegisters();


    //Store the dyna map
    if (debug_dirty_only)
    {
         if (ConstMap[xRD->mips_reg].IsMapped == 1) COMPARE_DebugDirtyConst(xRD->mips_reg);
         if (ConstMap[xRT->mips_reg].IsMapped == 1) COMPARE_DebugDirtyConst(xRT->mips_reg);
         if (ConstMap[xRS->mips_reg].IsMapped == 1) COMPARE_DebugDirtyConst(xRS->mips_reg);

              if (xRD->IsDirty) COMPARE_DebugDirty(xRD);
         else if (xRT->IsDirty) COMPARE_DebugDirty(xRT);
         else if (xRS->IsDirty) COMPARE_DebugDirty(xRS);
         
         else COMPARE_DebugDirtyFPU(PASS_PARAMS);
    }
    else
    {
    PushMap();
    MOV_ImmToReg(1, Reg_EAX, (uint32)&CompareStates1);
    CALL_Reg(Reg_EAX);

    memcpy(TempConstMap_Debug, ConstMap, sizeof(ConstMap));
    memcpy(Tempx86reg_Debug, x86reg, sizeof(x86reg));
    PopMap();

    for (k=0; k<8; k++)
        if (k==Reg_ESP);
        else MOV_RegToMemory(1,  (unsigned char)k, ModRM_disp32, (uint32)&RegisterRecall[k]);

    FlushAllRegistersToDynaCompare();
    
    for (k=0; k<8; k++)
        if (k==Reg_ESP);
        else MOV_MemoryToReg(1,  (unsigned char)k, ModRM_disp32, (uint32)&RegisterRecall[k]);
    
    memcpy(ConstMap, TempConstMap_Debug, sizeof(ConstMap));
    memcpy(x86reg, Tempx86reg_Debug, sizeof(x86reg));
    
	PushMap();
	MOV_ImmToReg(1, Reg_ECX, reg->code);
	MOV_ImmToReg(1, Reg_EAX, (uint32)&CompareStates);
	CALL_Reg(Reg_EAX);
	PopMap();
    }
}

dyn_cpu_instr dyna_instruction[64] = { 
    OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,
    OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,
    OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,
    OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,
    OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,
    OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,
    OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,
    OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,OpcodeDebugger,
 };

#else

// Never call this function directly. Always use dyna_instruction[]
void CheckMultiPass();
void CompileOpcode(OP_PARAMS)
{
    CheckMultiPass();

    now_do_dyna_instruction[__OPCODE](PASS_PARAMS);

	if( currentromoptions.Use_Register_Caching == USEREGC_NO )
	    FlushAllRegisters();
}

dyn_cpu_instr dyna_instruction[64] =
{
    CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,
    CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,
    CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,
    CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,
    CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,
    CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,
    CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,
    CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,CompileOpcode,
};
#endif

dyn_cpu_instr dyna_special_instruction[64] =
{
    dyna4300i_special_shift, dyna4300i_reserved,      dyna4300i_special_shift,dyna4300i_special_shift,dyna4300i_shift_var,       dyna4300i_reserved,       dyna4300i_shift_var,      dyna4300i_shift_var,
    dyna4300i_special_jr,    dyna4300i_special_jalr,  dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_special_syscall, dyna4300i_special_break,  dyna4300i_reserved,       dyna4300i_special_sync,
    dyna4300i_mf_mt,         dyna4300i_mf_mt,         dyna4300i_mf_mt,        dyna4300i_mf_mt,        dyna4300i_special_dsllv,   dyna4300i_reserved,       dyna4300i_special_dsrlv,  dyna4300i_special_dsrav,
    dyna4300i_special_mul,   dyna4300i_special_mul,   dyna4300i_special_div,  dyna4300i_special_divu, dyna4300i_special_dmult,   dyna4300i_special_dmultu, dyna4300i_special_ddiv,   dyna4300i_special_ddivu, 
    dyna4300i_special_add,   dyna4300i_special_addu,  dyna4300i_special_sub,  dyna4300i_special_subu, dyna4300i_special_and,     dyna4300i_special_or,     dyna4300i_special_xor,    dyna4300i_special_nor,
    dyna4300i_reserved,      dyna4300i_reserved,      dyna4300i_special_slt,  dyna4300i_special_sltu, dyna4300i_special_dadd,    dyna4300i_special_daddu,  dyna4300i_special_dsub,   dyna4300i_special_dsubu,
    dyna4300i_special_tge,   dyna4300i_special_tgeu,  dyna4300i_special_tlt,  dyna4300i_special_tltu, dyna4300i_special_teq,     dyna4300i_reserved,       dyna4300i_special_tne,    dyna4300i_reserved,
    dyna4300i_special_dsll,  dyna4300i_reserved,      dyna4300i_special_dsrl, dyna4300i_special_dsra, dyna4300i_special_dsll32,  dyna4300i_reserved,       dyna4300i_special_dsrl32, dyna4300i_special_dsra32
};

dyn_cpu_instr dyna_regimm_instruction[32] =
{
    dyna4300i_regimm_bltz,   dyna4300i_regimm_bgez,   dyna4300i_regimm_bltzl,   dyna4300i_regimm_bgezl,   dyna4300i_reserved,    dyna4300i_reserved, dyna4300i_reserved,    dyna4300i_reserved,
    dyna4300i_regimm_tgei,   dyna4300i_regimm_tgeiu,  dyna4300i_regimm_tlti,    dyna4300i_regimm_tltiu,   dyna4300i_regimm_teqi, dyna4300i_reserved, dyna4300i_regimm_tnei, dyna4300i_reserved,
    dyna4300i_regimm_bltzal, dyna4300i_regimm_bgezal, dyna4300i_regimm_bltzall, dyna4300i_regimm_bgezall, dyna4300i_reserved,    dyna4300i_reserved, dyna4300i_reserved,    dyna4300i_reserved,
    dyna4300i_reserved,      dyna4300i_reserved,      dyna4300i_reserved,       dyna4300i_reserved,       dyna4300i_reserved,    dyna4300i_reserved, dyna4300i_reserved,    dyna4300i_reserved
};

dyn_cpu_instr dyna_cop0_rs_instruction[32] =
{
    dyna4300i_cop0_rs_mf, dyna4300i_cop0_rs_dmf, dyna4300i_cop0_rs_cf, dyna4300i_reserved,   dyna4300i_cop0_rs_mt, dyna4300i_cop0_rs_dmt, dyna4300i_cop0_rs_ct, dyna4300i_reserved,
    dyna4300i_cop0_rs_bc, dyna4300i_reserved,    dyna4300i_reserved,   dyna4300i_reserved,   dyna4300i_reserved,   dyna4300i_reserved,    dyna4300i_reserved,   dyna4300i_reserved,
    dyna4300i_cop0_tlb, dyna4300i_reserved,    dyna4300i_reserved,   dyna4300i_reserved,   dyna4300i_reserved,   dyna4300i_reserved,    dyna4300i_reserved,   dyna4300i_reserved,
    dyna4300i_reserved,   dyna4300i_reserved,    dyna4300i_reserved,   dyna4300i_reserved,   dyna4300i_reserved,   dyna4300i_reserved,    dyna4300i_reserved,   dyna4300i_reserved
};

dyn_cpu_instr dyna_cop0_rt_instruction[32] =
{
    dyna4300i_cop0_rt_bcf, dyna4300i_cop0_rt_bct, dyna4300i_cop0_rt_bcfl, dyna4300i_cop0_rt_bctl, dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved,
    dyna4300i_reserved,    dyna4300i_reserved,    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved,
    dyna4300i_reserved,    dyna4300i_reserved,    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved,
    dyna4300i_reserved,    dyna4300i_reserved,    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved
};

dyn_cpu_instr dyna_tlb_instruction[64] =
{
    dyna4300i_invalid,   dyna4300i_cop0_tlbr, dyna4300i_cop0_tlbwi, dyna4300i_invalid, dyna4300i_invalid, dyna4300i_invalid, dyna4300i_cop0_tlbwr, dyna4300i_invalid,
    dyna4300i_cop0_tlbp, dyna4300i_invalid,   dyna4300i_invalid,    dyna4300i_invalid, dyna4300i_invalid, dyna4300i_invalid, dyna4300i_invalid,    dyna4300i_invalid,
    dyna4300i_reserved,  dyna4300i_invalid,   dyna4300i_invalid,    dyna4300i_invalid, dyna4300i_invalid, dyna4300i_invalid, dyna4300i_invalid,    dyna4300i_invalid,
    dyna4300i_cop0_eret, dyna4300i_invalid,   dyna4300i_invalid,    dyna4300i_invalid, dyna4300i_invalid, dyna4300i_invalid, dyna4300i_invalid,    dyna4300i_invalid,
    dyna4300i_invalid,   dyna4300i_invalid,   dyna4300i_invalid,    dyna4300i_invalid, dyna4300i_invalid, dyna4300i_invalid, dyna4300i_invalid,    dyna4300i_invalid,
    dyna4300i_invalid,   dyna4300i_invalid,   dyna4300i_invalid,    dyna4300i_invalid, dyna4300i_invalid, dyna4300i_invalid, dyna4300i_invalid,    dyna4300i_invalid,
    dyna4300i_invalid,   dyna4300i_invalid,   dyna4300i_invalid,    dyna4300i_invalid, dyna4300i_invalid, dyna4300i_invalid, dyna4300i_invalid,    dyna4300i_invalid,
    dyna4300i_invalid,   dyna4300i_invalid,   dyna4300i_invalid,    dyna4300i_invalid, dyna4300i_invalid, dyna4300i_invalid, dyna4300i_invalid,    dyna4300i_invalid
};

dyn_cpu_instr dyna_cop2_rs_instruction[32] =
{
    dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented,
    dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented,
    dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented,
    dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented, dyna4300i_cop2_rs_not_implemented
};


////////////////////////////////////////////////////////////////////////
extern void COP0_instr(uint32 Instruction);

void dyna4300i_cop0(OP_PARAMS)
{
    dyna_cop0_rs_instruction[__RS](PASS_PARAMS);
    //if (/* eret */(__RS != 16) && (__F != 24))  INTERPRET(COP0_instr);
}

////////////////////////////////////////////////////////////////////////
// Cop1 stuff
////////////////////////////////////////////////////////////////////////

extern void (* dyna4300i_cop1_Instruction[])(OP_PARAMS);
void dyna4300i_cop1(OP_PARAMS)
{
    dyna4300i_cop1_Instruction[__RS](PASS_PARAMS);
    //INTERPRET(COP1_instr);
}


//// BAD STUFF, BUT HERE NOW FOR COMPATIBILITY ////

void HELP_CheckExceptionSlot(_u32 pc){}
_u8* GetStaticMemAddr(_u32 Addr){ return(NULL);}


/////////////////////////

BOOL Init_Dynarec()
{
    lCodePosition = 0;
    AlreadyRecompiled = FALSE;

	CompilingSlot = FALSE;
	DynaBufferOverError = FALSE;

    memset(dyna_RecompCode, 0x00, RECOMPCODE_SIZE);
    memset(dyna_CodeTable, 0xFF, CODETABLE_SIZE);
    memset(dyna_RecompCode, 0x00, RECOMPCODE_SIZE);
    memset(PositionAfterMap, 0x00, sizeof(PositionAfterMap));

    SetTranslator(dyna_RecompCode, 0, RECOMPCODE_SIZE);

    //InitLogDyna();
    InitRegisterMap();

    memset(xRD, 0, sizeof(xRD));
    memset(xRS, 0, sizeof(xRS));
    memset(xRT, 0, sizeof(xRT));
    memset(xLO, 0, sizeof(xLO));
    memset(xHI, 0, sizeof(xHI));

#ifdef DYNAEXECUTIONMAPMEMORY
    {
        int i;
        for( i=0; i<0x20000; i++ )
        {
            if( dynarommap[i] != NULL )
            {
                memset(dynarommap[i], 0, 0x10000);
            }
        }
    }
#else
    memset(DynaSP_REG,0x00,0x2000);
    memset(DynaRDRAM,0x00,0x00800000);
    memset(DynaROM,0x00,gAllocationLength);
#endif

	memset(RDRAM_Copy, 0, 0x00800000);
    return(TRUE);
}

void Free_Dynarec()
{
}


#ifdef WINDEBUG_1964
#include "../win32/windebug.h"
extern void HandleBreakpoint(uint32 Instruction);
#endif

//---------------------------------------------------------------------------------------
// This function performs routine interrupt checking
// Only interrupt will enter here, exceptions will be served otherwhere
void rc_Intr_Common()
{
    gHWS_COP0Reg[EPC] = gHWS_pc;
    gHWS_COP0Reg[STATUS] |= EXL;          // set EXL = 1 
	gHWS_pc = 0x80000180;
    gHWS_COP0Reg[CAUSE] &= NOT_BD;        // clear BD
}


////////////////////////

void CheckTheInterrupts(_u32 count)
{
}

extern unsigned long    lCodePosition;

void SetRdRsRt32bit(OP_PARAMS)
{
    memset(xRD, 0, sizeof(xRD));
    memset(xRS, 0, sizeof(xRS));
    memset(xRT, 0, sizeof(xRT));
    xRD->mips_reg = __RD; xRT->mips_reg = __RT; xRS->mips_reg = __RS;
    xRD->Is32bit = 1; xRS->Is32bit = 1; xRT->Is32bit = 1;
}

void SetRdRsRt64bit(OP_PARAMS)
{
    memset(xRD, 0, sizeof(xRD));
    memset(xRS, 0, sizeof(xRS));
    memset(xRT, 0, sizeof(xRT));
    xRD->mips_reg = __RD; xRT->mips_reg = __RT; xRS->mips_reg = __RS;
}

_u32 c = 0;
void store_regs()
{
    int i;
	char filename[_MAX_PATH];
	FILE *stream;

	strcpy(filename, main_directory);
	strcat(filename, "reg1.txt");

    stream = fopen(filename, "at");
    fprintf(stream, "\n\npc: %08X c: %x\n", gHWS_pc, c);
    for (i=0; i<32; i++)
        fprintf(stream, "reg%02i 0x%08X\n", i, gHWS_GPR[i]);

    fclose(stream);
}

extern unsigned char* RecompCode;
extern unsigned long lCodePosition;

/******************************************************************************\
*                                                                              *
*   Helper                                                                     *
*                                                                              *
\******************************************************************************/

/******************************************************************************\
*                                                                              *
*   OpCodes                                                                    *
*                                                                              *
\******************************************************************************/

void dyna4300i_reserved(OP_PARAMS)
{
    INTERPRET(UNUSED);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_invalid(OP_PARAMS)
{   
    DisplayError("invalid instruction");
}

/******************************************************************************\
*                                                                              *
*   Central Processing Unit (CPU)                                              *
*                                                                              *
\******************************************************************************/

void dyna4300i_special(OP_PARAMS)
{
    dyna_special_instruction[__F](PASS_PARAMS);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm(OP_PARAMS)
{
    dyna_regimm_instruction[__RT](PASS_PARAMS);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_addi(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);
    if (xRT->mips_reg == 0) return;

_SAFTY_CPU_(r4300i_addi)
    
#ifdef SAFE_IMM
     INTERPRET(r4300i_addi); return;
#endif

    DoConstAdd((_s32)(_s16)__I)
     //TODO: Check 32bit overflow
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_addiu(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);
    if (xRT->mips_reg == 0) return;

_SAFTY_CPU_(r4300i_addiu)
#ifdef SAFE_IMM
     INTERPRET(r4300i_addiu); return;
#endif

    DoConstAdd((_s32)(_s16)__I)
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_slt(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS); 

_SAFTY_CPU_(r4300i_slt)
    if (xRD->mips_reg == 0) return;
#ifdef SAFE_SLT
     INTERPRET(r4300i_slt); return;
#endif 

    if (xRS->mips_reg == xRT->mips_reg)
    {
#ifndef NO_CONSTS
        MapConst(xRD, 0);
#else
        xRD->IsDirty = 1; //bug fix
        xRD->Is32bit = 1;
        xRD->NoNeedToLoadTheLo = 1;
        MapRD;
        XOR_Reg1ToReg2(1, xRD->x86reg, xRD->x86reg);
#endif
    } 

     //32bit
    else if (CheckIs32Bit(xRS->mips_reg) && CheckIs32Bit(xRT->mips_reg))
    {    
        xRD->IsDirty = 1;
        xRD->Is32bit = 1;
        xRS->Is32bit = 1;
        xRT->Is32bit = 1;
        if ((xRD->mips_reg != xRS->mips_reg) && (xRD->mips_reg != xRT->mips_reg))
        {
            xRD->NoNeedToLoadTheLo = 1;
        }
        MapRD;
        MapRS;
        MapRT; 

        CMP_Reg2WithReg1(1, xRS->x86reg, xRT->x86reg);
        Jcc_auto(CC_GE, 5); 

        PUSH_Imm8ToStack(0x01);
        POP_RegFromStack(xRD->x86reg);
        JMP_Short_auto(6); 

SetTarget(5); 

        XOR_Reg1ToReg2(1, xRD->x86reg, xRD->x86reg); 

SetTarget(6);
    }
    //64bit
    else
    {
        xRD->IsDirty = 1; 

        if ((xRD->mips_reg != xRS->mips_reg) && (xRD->mips_reg != xRT->mips_reg))
        {
            xRD->Is32bit = 1;   
            xRD->NoNeedToLoadTheLo = 1;
        }
        MapRD;
        MapRS;
        MapRT; 

        CMP_Reg2WithReg1(1, xRS->HiWordLoc, xRT->HiWordLoc); 

        Jcc_auto(CC_G,2);
        Jcc_auto(CC_L,1); 

        CMP_Reg2WithReg1(1, xRS->x86reg, xRT->x86reg);
        Jcc_auto(CC_AE, 3); 

SetTarget(1);
        PUSH_Imm8ToStack(0x01);
        POP_RegFromStack(xRD->x86reg);
        JMP_Short_auto(4); 

SetTarget(2);
SetTarget(3); 

        XOR_Reg1ToReg2(1, xRD->x86reg, xRD->x86reg); 

SetTarget(4); 

        if (xRD->Is32bit == 0)
        {
            xRD->Is32bit = 1;
            MapRD;//converts to 32bit
        }
    }
} 

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_sltu(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS); 

_SAFTY_CPU_(r4300i_sltu)
#ifdef SAFE_SLT
    INTERPRET(r4300i_sltu); return;
#endif 

    if (xRD->mips_reg == 0) return;    

    if (xRS->mips_reg == xRT->mips_reg)
    {
#ifndef NO_CONSTS
        MapConst(xRD, 0);
#else
        xRD->IsDirty = 1; //bug fix
        xRD->Is32bit = 1;
        xRD->NoNeedToLoadTheLo = 1;
        MapRD;
        XOR_Reg1ToReg2(1, xRD->x86reg, xRD->x86reg);
#endif
    }
     //32bit
    else if (CheckIs32Bit(xRS->mips_reg) && CheckIs32Bit(xRT->mips_reg))
    {    
        xRD->IsDirty = 1;
        xRD->Is32bit = 1;
        xRS->Is32bit = 1;
        xRT->Is32bit = 1;
        if ((xRD->mips_reg != xRS->mips_reg) && (xRD->mips_reg != xRT->mips_reg))
        {
            xRD->NoNeedToLoadTheLo = 1;
        }
        MapRD;
        MapRS;
        MapRT; 

        CMP_Reg2WithReg1(1, xRS->x86reg, xRT->x86reg);
        Jcc_auto(CC_AE, 5); 

        PUSH_Imm8ToStack(0x01);
        POP_RegFromStack(xRD->x86reg);
        JMP_Short_auto(6); 

SetTarget(5); 

        XOR_Reg1ToReg2(1, xRD->x86reg, xRD->x86reg); 

SetTarget(6);
    }
    //64bit
    else
    {
        if ((xRD->mips_reg != xRS->mips_reg) && (xRD->mips_reg != xRT->mips_reg))
        {
            xRD->Is32bit = 1;   
            xRD->NoNeedToLoadTheLo = 1;
        }
        xRD->IsDirty = 1;
        MapRD;
        MapRS;
        MapRT; 

        CMP_Reg2WithReg1(1, xRS->HiWordLoc, xRT->HiWordLoc); 

        Jcc_auto(CC_A,2);
        Jcc_auto(CC_B,1); 

        CMP_Reg2WithReg1(1, xRS->x86reg, xRT->x86reg);
        Jcc_auto(CC_AE, 3); 

SetTarget(1);
        PUSH_Imm8ToStack(0x01);
        POP_RegFromStack(xRD->x86reg);
        JMP_Short_auto(4); 

SetTarget(2);
SetTarget(3); 

        XOR_Reg1ToReg2(1, xRD->x86reg, xRD->x86reg); 

SetTarget(4); 

        if (xRD->Is32bit == 0)
        {
            xRD->Is32bit = 1;
            MapRD;
        }
    }
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_slti(OP_PARAMS)
{
    _int32 ConstInt     = (_int32)__I;
    int Is32bit = 0;
    SetRdRsRt64bit(PASS_PARAMS);

_SAFTY_CPU_(r4300i_slti)
#ifdef SAFE_SLT
     INTERPRET(r4300i_slti); return;
#endif
    if (xRT->mips_reg == 0) return;

    Is32bit = (CheckIs32Bit(xRS->mips_reg) & CheckIs32Bit(xRT->mips_reg));
    if (Is32bit)
    {
        xRT->Is32bit = 1;
        xRS->Is32bit = 1;
    }

    if (xRT->mips_reg == xRS->mips_reg)
    {
        xRT->IsDirty = 1;
        MapRT;

        if (!Is32bit)
        {
            CMP_RegWithImm(1, xRT->HiWordLoc, (uint32)((_int32)ConstInt >> 31));
            Jcc_auto(CC_G,2);
            Jcc_auto(CC_L,1);
        }

        if (!Is32bit)
        {
            CMP_RegWithImm(1, xRT->x86reg, (_u32)ConstInt);
            Jcc_auto(CC_AE, 3);
        }
        else
        {
            CMP_RegWithImm(1, xRT->x86reg, (_u32)ConstInt);
            Jcc_auto(CC_GE, 3);
        }

        if (!Is32bit)
        {
            SetTarget(1);
        }
        PUSH_Imm8ToStack(0x01);
        POP_RegFromStack(xRT->x86reg);

        JMP_Short_auto(4);

        if (!Is32bit)
        {
            SetTarget(2);
        }

        SetTarget(3);

        XOR_Reg1ToReg2(1, xRT->x86reg, xRT->x86reg);

    SetTarget(4);
        if (!Is32bit)
        {
            XOR_Reg1ToReg2(1, xRT->HiWordLoc, xRT->HiWordLoc);
        }
    }
    else //rt != rs :)
    {
        xRT->NoNeedToLoadTheLo = 1;
        xRT->NoNeedToLoadTheHi = 1;
        xRT->IsDirty = 1;
        MapRT;
        MapRS;

        XOR_Reg1ToReg2(1, xRT->x86reg, xRT->x86reg);
        if (!Is32bit)
        {
            CMP_RegWithImm(1, xRS->HiWordLoc, (uint32)((_int32)ConstInt >> 31));
            Jcc_auto(CC_G,2);
            Jcc_auto(CC_L,1);
        }

        if (!Is32bit)
        {
            CMP_RegWithImm(1, xRS->x86reg, (_u32)ConstInt);
            Jcc_auto(CC_AE, 4);
        }
        else
        {
            CMP_RegWithImm(1, xRS->x86reg, (_u32)ConstInt);
            Jcc_auto(CC_GE, 4);
        }


        if (!Is32bit)
        {
            SetTarget(1);
        }
        PUSH_Imm8ToStack(0x01);
        POP_RegFromStack(xRT->x86reg);

        if (!Is32bit)
        {
            SetTarget(2);
        }

        SetTarget(4);
        if (!Is32bit)
        {
            xRT->Is32bit = 1;
            MapRT;
        }
    }
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_sltiu(OP_PARAMS)
{
    _s64 ConstInt     = (_s64)(_s32)__I;
    int Is32bit = 0;
    SetRdRsRt64bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_sltiu)
#ifdef SAFE_SLT
     INTERPRET(r4300i_sltiu); return;
#endif
    if (xRT->mips_reg == 0) return;

    Is32bit = (CheckIs32Bit(xRS->mips_reg) & CheckIs32Bit(xRT->mips_reg));
    if (Is32bit)
    {
        xRT->Is32bit = 1;
        xRS->Is32bit = 1;
    }

    if (xRT->mips_reg == xRS->mips_reg)
    {
        
        xRT->IsDirty = 1;
        MapRT;
        
        if (!Is32bit)
        {
            CMP_RegWithImm(1, xRT->HiWordLoc, (_u32)(ConstInt >> 32));
            Jcc_auto(CC_A,2);
            Jcc_auto(CC_B,1);
        }

        CMP_RegWithImm(1, xRT->x86reg, (_u32)ConstInt);
        Jcc_auto(CC_AE, 3);

        if (!Is32bit)
        {
            SetTarget(1);
        }
        PUSH_Imm8ToStack(0x01);
        POP_RegFromStack(xRT->x86reg);

        JMP_Short_auto(4);

        if (!Is32bit)
        {
            SetTarget(2);
        }

        SetTarget(3);

        XOR_Reg1ToReg2(1, xRT->x86reg, xRT->x86reg);

    SetTarget(4);
        if (!Is32bit)
        {
            XOR_Reg1ToReg2(1, xRT->HiWordLoc, xRT->HiWordLoc);
        }
    }
    else //rt != rs :)
    {
        xRT->IsDirty = 1;
        xRT->NoNeedToLoadTheLo = 1;
        xRT->NoNeedToLoadTheHi = 1;
        MapRT;
        MapRS;

        XOR_Reg1ToReg2(1, xRT->x86reg, xRT->x86reg);
        if (!Is32bit)
        {
            CMP_RegWithImm(1, xRS->HiWordLoc, (_u32)(ConstInt >> 32));
            Jcc_auto(CC_A,2);
            Jcc_auto(CC_B,1);
        }

        CMP_RegWithImm(1, xRS->x86reg, (_u32)ConstInt);
        Jcc_auto(CC_AE, 4);

        if (!Is32bit)
        {
            SetTarget(1);
        }
        PUSH_Imm8ToStack(0x01);
        POP_RegFromStack(xRT->x86reg);

        if (!Is32bit)
        {
            SetTarget(2);
        }

        SetTarget(4);
        if (!Is32bit)
        {
            xRT->Is32bit = 1;
            MapRT;
        }
    }
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_andi(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_andi)

    if (xRT->mips_reg == 0) return;
#ifdef SAFE_IMM
     INTERPRET(r4300i_andi); return;
#endif

    if ((_u32)(_u16)__I == 0)
    {
#ifndef NO_CONSTS
        MapConst(xRT, 0);
#else
        xRT->IsDirty = 1;
        xRT->Is32bit = 1;
        xRT->NoNeedToLoadTheLo = 1;
        MapRT;
        XOR_Reg1ToReg2(1, xRT->x86reg, xRT->x86reg);
#endif
    }
    else if (xRS->mips_reg == xRT->mips_reg)
    {
        xRT->IsDirty = 1;
        MapRT;

        AND_ImmToReg(1, xRT->x86reg, (_u32)(_u16)__I);
    }
    else
    {
        xRT->IsDirty = 1;
        xRT->NoNeedToLoadTheLo = 1;
        MapRT;
        MapRS;

        MOV_Reg2ToReg1(1, xRT->x86reg, xRS->x86reg); /* mov rt, rs (lo) */
        AND_ImmToReg(1, xRT->x86reg, (_u32)(_u16)__I);
    }
}


////////////////////////////////////////////////////////////////////////

void dyna4300i_ori(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_ori)

// 64bit
#ifdef SAFE_IMM
     INTERPRET(r4300i_ori); return;
#endif

    if (xRT->mips_reg == 0) return;
    if (xRS->mips_reg == xRT->mips_reg)
    {
        if ((_u32)(_u16)__I != 0)
        {
            if (ConstMap[xRT->mips_reg].IsMapped == 1)
            {
                (_u32)ConstMap[xRT->mips_reg].value |= (_u32)(_u16)__I;
            }
            else
            {
                xRS->IsDirty = 1;
                
                if (CheckIs32Bit(xRS->mips_reg))
                {
                    xRS->Is32bit = 1;
                }
                MapRS;
                
                OR_ImmToReg(1, xRS->x86reg, (_u32)(_u16)__I);
            }
        }
    }
    else
    {
        xRT->IsDirty = 1;
        xRT->NoNeedToLoadTheLo = 1;
        xRT->NoNeedToLoadTheHi = 1;
        MapRT;

        if (xRS->mips_reg != 0)
        {
            MapRS;
            Reg2ToReg1(1, 0x8B, xRT->x86reg, xRS->x86reg);        /* mov rt, rs (lo) */
            Reg2ToReg1(1, 0x8B, xRT->HiWordLoc, xRS->HiWordLoc);  /* mov rt, rs (hi) */
            if ((_u32)(_u16)__I != 0)
                OR_ImmToReg(1, xRT->x86reg, (_u32)(_u16)__I);
        }
        else /* rs = 0 */
        {
            if ((_u32)(_u16)__I != 0)
                MOV_ImmToReg(1, xRT->x86reg, (_u32)(_u16)__I);
            else
                XOR_Reg1ToReg2(1, xRT->x86reg, xRT->x86reg);

            XOR_Reg1ToReg2(1, xRT->HiWordLoc, xRT->HiWordLoc);
        }
    }
}

////////////////////////////////////////////////////////////////////////                                    

void asm_xori(OP_PARAMS)
{
    switch (NextPass[PassIndex])
    {
    case 1: XOR_ImmToReg(1, xRS->x86reg, (_u32)(_u16)__I); break;
    case 2: MOV_Reg2ToReg1(1, xRT->x86reg, xRS->x86reg);
            if (((_u32)(_u16)__I) != 0)
                XOR_ImmToReg(1, xRT->x86reg, (_u32)(_u16)__I);
            MOV_Reg2ToReg1(1, xRT->HiWordLoc, xRS->HiWordLoc);
            break;
    }
}

////////////////////////////////////////////////////////////////////////                                    

void dyna4300i_xori(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS);

     _SAFTY_CPU_(r4300i_xori)

// 64bit 
    if (xRT->mips_reg == 0) return;    
#ifdef SAFE_IMM
     INTERPRET(r4300i_xori); return;
#endif

    if (xRS->mips_reg == xRT->mips_reg)
    {
        if (((_u32)(_u16)__I) != 0)
        {
            xRS->IsDirty = 1;
            MapRS;
            NextPass[PassIndex] = 1;
        }
    }
    else
    {
        //if (xRS->mips_reg == 0){TODO: this optimization..doesn't seem to be used much tho. }
        xRT->IsDirty = 1;
        xRT->NoNeedToLoadTheLo = 1;
        xRT->NoNeedToLoadTheHi = 1;
        MapRT;
        MapRS;
        NextPass[PassIndex] = 2;
    }

#ifndef TWO_PASS_COMPILER
    asm_xori(PASS_PARAMS);
#endif
}

////////////////////////////////////////////////////////////////////////

extern x86regtyp x86reg[];

// Send To Rice
void dyna4300i_lui(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

_SAFTY_CPU_(r4300i_lui)
//#ifdef SAFE_IMM
//    INTERPRET(r4300i_lui); return;
//#endif SAFE_IMM

#ifndef NO_CONSTS
    MapConst(xRT, (_s32)((__I) << 16));
#else
    
    if (__RT != 0)
    {
        xRT->IsDirty = 1;
        MapRT;
        MOV_ImmToReg(1, xRT->x86reg, (_s32)((__I) << 16));
    }
#endif
}

extern void TriggerFPUUnusableException();
void (*Dyna_Code_Check[])(uint32 pc);
void (*Dyna_Check_Codes)(uint32 pc);
void Dyna_Invalidate_Compiled_Block(uint32 pc);

void dyna4300i_cop1_with_exception(OP_PARAMS)
{
	MapConstant LocalTempConstMap[NUM_CONSTS];
	x86regtyp LocalTempx86reg[8];

    if( currentromoptions.FPU_Hack == USEFPUHACK_YES )
	{
		int temp = reg->pc;

		TEST_ImmWithMemory((uint32)&gHWS_COP0Reg[STATUS], SR_CU1);
		Jcc_Near_auto(CC_NE, 8);

		MOV_ImmToMemory(1, (uint32)&reg->pc, temp);
		memcpy(LocalTempConstMap, ConstMap, sizeof(ConstMap));
		memcpy(LocalTempx86reg, x86reg, sizeof(x86reg));
		FlushAllRegisters();
		X86_CALL((uint32)TriggerFPUUnusableException);
		MOV_ImmToMemory(1, (uint32)&reg->pc, temp);
		RET();

		SetNearTarget(8);
		memcpy(ConstMap, LocalTempConstMap, sizeof(ConstMap));
		memcpy(x86reg, LocalTempx86reg, sizeof(x86reg));
	}

	DYNA_LOG_INSTRUCTION;
	dyna4300i_cop1_Instruction[__RS](PASS_PARAMS);
}



////////////////////////////////////////////////////////////////////////

void dyna4300i_cop2(OP_PARAMS)
{
    dyna_cop2_rs_instruction[__RS](PASS_PARAMS);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_daddi(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_daddi)
#ifdef SAFE_MATH
    INTERPRET(r4300i_daddi); return;
#endif SAFE_MATH

    INTERPRET(r4300i_daddi);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_daddiu(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_daddiu)

#ifdef SAFE_MATH
    INTERPRET(r4300i_daddiu); return;
#endif SAFE_MATH
    
    INTERPRET(r4300i_daddiu);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_ldl(OP_PARAMS)
{
    _SAFTY_CPU_(r4300i_ldl)

#ifdef SAFE_LOADSTORE
     INTERPRET_LOADSTORE(r4300i_ldl); return;
#endif
    

    INTERPRET_LOADSTORE(r4300i_ldl);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_ldr(OP_PARAMS)
{
    _SAFTY_CPU_(r4300i_ldr)

#ifdef SAFE_LOADSTORE
     INTERPRET_LOADSTORE(r4300i_ldr); return;
#endif
    
    INTERPRET_LOADSTORE(r4300i_ldr);
}

////////////////////////////////////////////////////////////////////////

extern _int32 r4300i_lh_faster(uint32 QuerAddr);
void dyna4300i_lh(OP_PARAMS)
{
SetRdRsRt32bit(PASS_PARAMS);

	_SAFTY_CPU_(r4300i_lh);

    if (xRT->mips_reg == 0) return;    

#ifdef SAFE_LOADSTORE
    // INTERPRET_LOADSTORE(r4300i_lh); return;
#endif

    Interpret_Load_New((uint32)&r4300i_lh_faster, xRD, xRS, xRT);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_lwl(OP_PARAMS)
{
    _SAFTY_CPU_(r4300i_lwl)

#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_lwl); return;
#endif SAFE_LOADSTORE

    INTERPRET_LOADSTORE(r4300i_lwl);
}

////////////////////////////////////////////////////////////////////////

extern char * ((**phys_read_fast)(_u32 addr));
extern char * ((**phys_write_fast)(_u32 addr));

extern x86regtyp x86reg[8];
extern _int32 r4300i_lw_faster(uint32 QuerAddr);
void dyna4300i_lw(OP_PARAMS)
{
    _s32 value;
    
    _SAFTY_CPU_(r4300i_lw)
    SetRdRsRt32bit(PASS_PARAMS);

#ifdef SAFE_LOADSTORE
    goto _Default;
#endif

    if (ConstMap[xRS->mips_reg].IsMapped == 1)
    {
        uint32 QuerAddr;

        QuerAddr = (_u32)((_s32)ConstMap[xRS->mips_reg].value + (_s32)(_s16)__I);

        if (NOT_IN_KO_K1_SEG(QuerAddr))
            goto _Default;

        __try {value = LOAD_SWORD_PARAM(QuerAddr);}
        __except(NULL,EXCEPTION_EXECUTE_HANDLER) {goto _Default;}

        if (xRT->mips_reg != 0) // mandatory
        {
            ConstMap[xRT->mips_reg].IsMapped = 0;
            xRT->IsDirty = 1;
            xRT->NoNeedToLoadTheLo = 1;
            MapRegister(xRT, 99, 99);

            MOV_MemoryToReg(1, xRT->x86reg, ModRM_disp32, (_u32)pLOAD_SWORD_PARAM(QuerAddr));
        }
    }
    else

_Default:
    {
        Interpret_Load_New((uint32)&r4300i_lw_faster, xRD, xRS, xRT);
        if (__RT == 0)
        {
            XOR_Reg2ToReg1(1, xRT->x86reg, xRT->x86reg);
        }
    }
}

////////////////////////////////////////////////////////////////////////
extern void r4300i_sw_faster(uint32 QuerAddr, uint32 rt_ft);
void dyna4300i_sw(OP_PARAMS)
{
    _u32 QuerAddr;
    _s32 value;
    SetRdRsRt32bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_sw);

#ifdef SAFE_LOADSTORE
    goto _Default;
#endif

    if (ConstMap[xRS->mips_reg].IsMapped == 1)
        __try{
            QuerAddr = (_u32)((_s32)ConstMap[xRS->mips_reg].value + (_s32)(_s16)__I);
            if (NOT_IN_KO_K1_SEG(QuerAddr))
                goto _Default;
                        
            value = LOAD_SWORD_PARAM(QuerAddr);
            if (xRT->mips_reg != 0)
            {
                MapRT;
                MOV_RegToMemory(1, xRT->x86reg, ModRM_disp32, (_u32)pLOAD_SWORD_PARAM(QuerAddr));
            }
            else
            {
                PUSH_RegIfMapped(Reg_EDI);
                XOR_Reg1ToReg2(1, Reg_EDI, Reg_EDI);
                MOV_RegToMemory(1, Reg_EDI, ModRM_disp32, (_u32)pLOAD_SWORD_PARAM(QuerAddr));
                POP_RegIfMapped(Reg_EDI);
            }
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                goto _Default;
    }
    else

_Default:
    INTERPRET_STORE_NEW(r4300i_sw_faster);
}

////////////////////////////////////////////////////////////////////////

extern void r4300i_ld_faster(uint32 QuerAddr, uint32 rt_ft);
void dyna4300i_ld(OP_PARAMS)
{
    _s32 value;
    SetRdRsRt64bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_ld);

#ifdef SAFE_LOADSTORE
     INTERPRET_LOADSTORE(r4300i_ld); return;
#endif

    if (xRT->mips_reg == 0) return;

    if (ConstMap[xRS->mips_reg].IsMapped == 1)
        __try{
            _u32 QuerAddr;
            QuerAddr = (_u32)((_s32)ConstMap[xRS->mips_reg].value + (_s32)(_s16)__I);
        
            if (NOT_IN_KO_K1_SEG(QuerAddr))
                goto _Default;
            
            
            value = LOAD_SWORD_PARAM(QuerAddr);
            value = LOAD_SWORD_PARAM((QuerAddr+4));
            if (xRT->mips_reg != 0) //mandatory
            {
                if (ConstMap[xRT->mips_reg].IsMapped == 1) { ConstMap[xRT->mips_reg].IsMapped = 0;  }
                xRT->IsDirty = 1;
                xRT->NoNeedToLoadTheLo = 1;
                xRT->NoNeedToLoadTheHi = 1;
                MapRT;
                MOV_MemoryToReg(1, xRT->x86reg,    ModRM_disp32, (_u32)pLOAD_UWORD_PARAM( (QuerAddr+4)) );
                MOV_MemoryToReg(1, xRT->HiWordLoc, ModRM_disp32, (_u32)pLOAD_UWORD_PARAM(  QuerAddr   ) );
            }
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                goto _Default;
    }
    else

_Default:
    INTERPRET_LOADSTORE(r4300i_ld);
}

////////////////////////////////////////////////////////////////////////

extern void r4300i_sd_faster(uint32 QuerAddr, uint32 rt_ft);
void dyna4300i_sd(OP_PARAMS)
{
    _s32 value;
    SetRdRsRt64bit(PASS_PARAMS);

	_SAFTY_CPU_(r4300i_sd);

#ifdef SAFE_LOADSTORE
     INTERPRET_LOADSTORE(r4300i_sd); return;
#endif

    if (ConstMap[xRS->mips_reg].IsMapped == 1)
        __try{
            _u32 QuerAddr;

            //TLB range
            QuerAddr = (_u32)((_s32)ConstMap[xRS->mips_reg].value + (_s32)(_s16)__I);

            value = LOAD_SWORD_PARAM(QuerAddr);
            value = LOAD_SWORD_PARAM((QuerAddr+4));
            if (xRT->mips_reg != 0)
            {
                MapRT;
                MOV_RegToMemory(1, xRT->x86reg,    ModRM_disp32, (_u32)pLOAD_UWORD_PARAM( (QuerAddr+4)) );
                MOV_RegToMemory(1, xRT->HiWordLoc, ModRM_disp32, (_u32)pLOAD_UWORD_PARAM(  QuerAddr   ) );
            }
            else
            {
                PUSH_RegIfMapped(Reg_EDI);
                XOR_Reg1ToReg2(1, Reg_EDI, Reg_EDI);
                MOV_RegToMemory(1, Reg_EDI, ModRM_disp32, (_u32)pLOAD_UWORD_PARAM( (QuerAddr+4)) );
                MOV_RegToMemory(1, Reg_EDI, ModRM_disp32, (_u32)pLOAD_UWORD_PARAM(  QuerAddr   ) );
                POP_RegIfMapped(Reg_EDI);
            }
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                goto _Default;
    }
    else

_Default:
    INTERPRET_LOADSTORE(r4300i_sd);
}

////////////////////////////////////////////////////////////////////////
extern _int32 r4300i_lb_faster(uint32 QuerAddr);
void dyna4300i_lb(OP_PARAMS)
{
    _s8 value;
    SetRdRsRt32bit(PASS_PARAMS);

	_SAFTY_CPU_(r4300i_lb);

    if (xRT->mips_reg == 0) return;    

#ifdef SAFE_LOADSTORE
    goto _Default;
#endif

    if (ConstMap[xRS->mips_reg].IsMapped == 1)
    {
        __try{
            _u32 QuerAddr;
            //TLB range
            QuerAddr = (_u32)(((_s32)ConstMap[xRS->mips_reg].value + (_s32)(_s16)__I));
            if (NOT_IN_KO_K1_SEG(QuerAddr))
                goto _Default;

            value = LOAD_SBYTE_PARAM( QuerAddr );
            if (xRT->mips_reg != 0) //mandatory
            {
                if (ConstMap[xRT->mips_reg].IsMapped == 1) { ConstMap[xRT->mips_reg].IsMapped = 0;  }

                xRT->IsDirty = 1;
                xRT->NoNeedToLoadTheLo = 1;
                MapRT;
                MOVSX_MemoryToReg(0, xRT->x86reg, ModRM_disp32, (_u32)pLOAD_SBYTE_PARAM(QuerAddr) );
            }
            else
                goto _Default;
   
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
            goto _Default;
        }
    }
    else

_Default:
    Interpret_Load_New((uint32)&r4300i_lb_faster, xRD, xRS, xRT);
}

////////////////////////////////////////////////////////////////////////
extern uint32 r4300i_lbu_faster(uint32 QuerAddr);
void dyna4300i_lbu(OP_PARAMS)
{
    _u32 QuerAddr;
    _u8 value;
    SetRdRsRt32bit(PASS_PARAMS);

	_SAFTY_CPU_(r4300i_lbu);

    if (xRT->mips_reg == 0) return;    

#ifdef SAFE_LOADSTORE
    goto _Default;
#endif

    if (ConstMap[xRS->mips_reg].IsMapped == 1)
        __try{
            QuerAddr = (_u32)((((_s32)ConstMap[xRS->mips_reg].value + (_s32)(_s16)__I)));
            if (NOT_IN_KO_K1_SEG(QuerAddr))
                goto _Default;
            
            value = LOAD_UBYTE_PARAM( QuerAddr );
            if (xRT->mips_reg != 0) //mandatory
            {
                if (ConstMap[xRT->mips_reg].IsMapped == 1) { ConstMap[xRT->mips_reg].IsMapped = 0; }
                xRT->IsDirty = 1;
                xRT->NoNeedToLoadTheLo = 1;
                MapRT;
                MOVZX_MemoryToReg(0, xRT->x86reg, ModRM_disp32, (_u32)pLOAD_UBYTE_PARAM( QuerAddr ) );
            }
            else
                goto _Default;
   
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                goto _Default;
    }
    else

_Default:
    Interpret_Load_New((uint32)&r4300i_lbu_faster, xRD, xRS, xRT);
}

////////////////////////////////////////////////////////////////////////

extern uint32 r4300i_lhu_faster(uint32 QuerAddr);
void dyna4300i_lhu(OP_PARAMS)
{
SetRdRsRt32bit(PASS_PARAMS);

	_SAFTY_CPU_(r4300i_lhu);

    if (xRT->mips_reg == 0) return;    


#ifdef SAFE_LOADSTORE
    //INTERPRET_LOADSTORE(r4300i_lhu); return;
#endif

    Interpret_Load_New((uint32)&r4300i_lhu_faster, xRD, xRS, xRT);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_lwr(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_lwr)

#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_lwr); return;
#endif

    INTERPRET_LOADSTORE(r4300i_lwr);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_lwu(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_lwu)

#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_lwu); return;
#endif

    INTERPRET_LOADSTORE(r4300i_lwu);
}

////////////////////////////////////////////////////////////////////////

extern void r4300i_sb_faster(uint32 QuerAddr, uint32 rt_ft);
void dyna4300i_sb(OP_PARAMS)
{
    _u8 value;

    SetRdRsRt32bit(PASS_PARAMS);

	_SAFTY_CPU_(r4300i_sb);

#ifdef SAFE_LOADSTORE
//     INTERPRET_LOADSTORE(r4300i_sb); return;
    goto _Default;
#endif
   INTERPRET_STORE_NEW(r4300i_sb_faster); return;
    if (ConstMap[xRS->mips_reg].IsMapped == 1)
    {
        uint32 QuerAddr;

        QuerAddr = (_u32)(((_s32)ConstMap[xRS->mips_reg].value + (_s32)(_s16)__I));

        if (NOT_IN_KO_K1_SEG(QuerAddr))
            goto _Default;

        __try{ value = LOAD_UBYTE_PARAM( QuerAddr ); }
        __except(NULL,EXCEPTION_EXECUTE_HANDLER){ goto _Default; }

        if (xRT->mips_reg != 0)
        {
            //if (gHWS_pc == 0x8031855C)
            //    FlushAllRegisters(); // why do i need to do this ? Debug in Mario, pc 0x8031855c
            MapRT;
            MOV_RegToMemory(0, xRT->x86reg, ModRM_disp32, (_u32)pLOAD_UBYTE_PARAM( QuerAddr ) );
        }
        else
        {
            PUSH_RegIfMapped(Reg_EDI);
            XOR_Reg2ToReg1(0, Reg_EDI, Reg_EDI);
            MOV_RegToMemory(0, Reg_EDI, ModRM_disp32, (_u32)pLOAD_UBYTE_PARAM( QuerAddr ) );
            POP_RegIfMapped(Reg_EDI);
        }
    }
    else
    {
_Default:
        INTERPRET_STORE_NEW(r4300i_sb_faster);
    }
}

////////////////////////////////////////////////////////////////////////

extern void r4300i_sh_faster(uint32 QuerAddr, uint32 rt_ft);
void dyna4300i_sh(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_sh)

#ifdef SAFE_LOADSTORE
    //INTERPRET_LOADSTORE(r4300i_sh); return;
#endif SAFE_LOADSTORE

    INTERPRET_STORE_NEW(r4300i_sh_faster);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_swl(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_swl)

#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_swl); return;
#endif SAFE_LOADSTORE

    INTERPRET_LOADSTORE(r4300i_swl);
}


////////////////////////////////////////////////////////////////////////

void dyna4300i_sdl(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_sdl)

#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_sdl); return;
#endif

    INTERPRET_LOADSTORE(r4300i_sdl);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_sdr(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_sdr)

#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_sdr); return;
#endif

    INTERPRET_LOADSTORE(r4300i_sdr);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_swr(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_swr)

#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_swr); return;
#endif

    INTERPRET_LOADSTORE(r4300i_swr);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cache(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_cache)

    INTERPRET(r4300i_cache);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_ll(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

	_SAFTY_CPU_(r4300i_ll)

#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_ll); return;
#endif

    INTERPRET_LOADSTORE(r4300i_ll) 
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_lwc2(OP_PARAMS)
{
#ifdef SAFE_LOADSTORE
    DisplayError("Unimplemented LWC2"); return;
    //INTERPRET_LOADSTORE(r4300i_lwc2); return;
#endif

    DisplayError("Unimplemented LWC2");
//    INTERPRET_LOADSTORE(r4300i_lwc2);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_lld(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

	_SAFTY_CPU_(r4300i_lld)

#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_lld); return;
#endif

    INTERPRET_LOADSTORE(r4300i_lld); 
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_ldc2(OP_PARAMS)
{
#ifdef SAFE_LOADSTORE
    DisplayError("Unhandled ldc2"); return;
    //INTERPRET_LOADSTORE(r4300i_ldc2); return;
#endif

    DisplayError("Unhandled ldc2");
//    INTERPRET_LOADSTORE(r4300i_ldc2);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_sc(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_sc)

    INTERPRET_LOADSTORE(r4300i_sc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_swc2(OP_PARAMS)
{
#ifdef SAFE_LOADSTORE
    DisplayError("Unhandled swc2"); return;
    //    INTERPRET_LOADSTORE(r4300i_swc2); return;
#endif

    DisplayError("Unhandled swc2"); return;
    //    INTERPRET_LOADSTORE(r4300i_swc2);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_scd(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_scd)

    INTERPRET_LOADSTORE(r4300i_scd);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_sdc2(OP_PARAMS)
{
//    INTERPRET_LOADSTORE(r4300i_sdc2);
}

////////////////////////////////////////////////////////////////////////

/*
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
    | SRL       | Shift word Right Logical                          |
    |-----------|---------------------------------------------------|
    |  000000   |  00000  |   rt    |    rd   |   sa    |000010 (2) |
    ------6----------5---------5---------5---------5----------6------
     Format:  SRL rd, rt, sa
     Purpose: To logical right shift a word by a fixed number of bits.
     Descrip: rd = rt >> sa

    -----------------------------------------------------------------
    | SRA       | Shift word Right Arithmetic                       |
    |-----------|---------------------------------------------------|
    |  000000   |  00000  |   rt    |    rd   |   sa    |000011 (3) |
    ------6----------5---------5---------5---------5----------6------
     Format:  SRA rd, rt, sa
     Purpose: To arithmetic right shift a word by a fixed number of bits.
     Descrip: rd = rt >> sa
*/
void r4300i_shift(uint32 Instruction)
{
    //Interpret the shift. (For debugging purposes.)
    switch (Instruction & 0x1f)
    { 
        case 0: r4300i_sll(Instruction); break;
        case 2: r4300i_srl(Instruction); break;
        case 3: r4300i_sra(Instruction); break;
    }
}

////////////////////////////////////////////////////////////////////////

void asm_special_shift(OP_PARAMS)
{
    int rd, rt;
    _u8 sa;

    rd = CheckWhereIsMipsReg(__RD);
    rt = CheckWhereIsMipsReg(__RT);

    if (rd > -1)
    switch (NextPass[PassIndex])
    {
    case 3: XOR_Reg1ToReg2(1, (_u8)rd, (_u8)rd); break;
    case 4: MOV_Reg2ToReg1(1, (_u8)rd, (_u8)rt); //intentionally no break here.
    case 2: 
        sa = (_u8)__SA;
        
        if ((sa | rt) != 0)
        switch (reg->code & 0x1F)
        {
            case 0: SHL_RegByImm(1, (_u8)rd, (_u8)sa); break;
            case 2: SHR_RegByImm(1, (_u8)rd, (_u8)sa); break;
            case 3: SAR_RegByImm(1, (_u8)rd, (_u8)sa); break;
        }
    }

    if (Two_Pass_Compiler)
        PassIndex++;
}

////////////////////////////////////////////////////////////////////////
#define _SLL_ 0
#define _SRL_ 2
#define _SRA_ 3

void dyna4300i_special_shift(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

	_SAFTY_CPU_(r4300i_shift)

#ifdef SAFE_SHIFTS
    INTERPRET(r4300i_shift); return;
#endif

    if (xRD->mips_reg == 0)     

//==================================
//  RD is r0
//==================================

        return;

    if (ConstMap[xRD->mips_reg].IsMapped == 1)
    {

//==================================
//  RD is a constant 
//==================================

        if (ConstMap[xRT->mips_reg].IsMapped == 1)

        //==================================
        //  RT is a constant (includes r0)
        //==================================
        {
            _u8 sa = (_u8)__SA;

            switch (reg->code & 0x3)
            {
                case _SLL_: (_u32)ConstMap[xRD->mips_reg].value = (_u32)ConstMap[xRT->mips_reg].value << (_u32)(_u8)sa; break;
                case _SRL_: (_u32)ConstMap[xRD->mips_reg].value = (_u32)ConstMap[xRT->mips_reg].value >> (_u32)(_u8)sa; break;
                case _SRA_: (_s32)ConstMap[xRD->mips_reg].value = (_s32)ConstMap[xRT->mips_reg].value >> (_s32)(_u8)sa; break;
            }
        }
        else
        {            
            _u8 sa = (_u8)__SA;

        //==================================
        //  RT is a register
        //==================================

            goto _whatever;
        }
    }
    else
    {
_whatever:     
        if (xRD->mips_reg==xRT->mips_reg)
        {
            xRD->IsDirty = 1;
            MapRD;
            NextPass[PassIndex] = 2;
        }
        else
        {
            xRD->IsDirty = 1;
            xRD->NoNeedToLoadTheLo = 1;
            MapRD;

            if (xRT->mips_reg == 0)
            {
                NextPass[PassIndex] = 3;
            }
            else
            {
                MapRT;
                NextPass[PassIndex] = 4;
            }
        }

        if (Two_Pass_Compiler)
            PassIndex++;
        else
            asm_special_shift(PASS_PARAMS);
    }
}

////////////////////////////////////////////////////////////////////////

/*
    -----------------------------------------------------------------
    | SLLV      | Shift word Left Logical Variable                  |
    |-----------|---------------------------------------------------|
    |  000000   |   rs    |   rt    |    rd   |  00000  |000100 (4) |
    ------6----------5---------5---------5---------5----------6------
     Format:  SLLV rd, rt, rs
     Purpose: To left shift a word by a variable number of bits.
     Descrip: rd = rt << sa

    -----------------------------------------------------------------
    | SRLV      | Shift word Right Logical Variable                 |
    |-----------|---------------------------------------------------|
    |  000000   |   rs    |   rt    |    rd   |  00000  |000110 (6) |
    ------6----------5---------5---------5---------5----------6------
     Format:  SRLV rd, rt, rs
     Purpose: To logical right shift a word by a variable number of bits.
     Descrip: rd = rt >> rs

    -----------------------------------------------------------------
    | SRAV      | Shift word Right Arithmetic Variable              |
    |-----------|---------------------------------------------------|
    |  000000   |   rs    |   rt    |    rd   |  00000  |000111 (7) |
    ------6----------5---------5---------5---------5----------6------
     Format:  SRAV rd, rt, rs
     Purpose: To arithmetic right shift a word by a variable number of bits.
     Descrip: rd = rt >> rs
*/
void r4300i_shift_var(uint32 Instruction)
{
    //Interpret the shift. (For debugging purposes.)
    switch (Instruction & 0x1f)
    { 
        case 4: r4300i_sllv(Instruction); break;
        case 6: r4300i_srlv(Instruction); break;
        case 7: r4300i_srav(Instruction); break;
    }
}

////////////////////////////////////////////////////////////////////////

void asm_shift_var(OP_PARAMS)
{
    // Instruction pass
    int rd, rs, rt;

    rd = CheckWhereIsMipsReg(__RD);
    rs = CheckWhereIsMipsReg(__RS);
    rt = CheckWhereIsMipsReg(__RT);
    
    if (rd > -1) //will this method screw up which array index ?
    switch (NextPass[PassIndex])
    {
    case 2: MOV_Reg2ToReg1(1, Reg_ECX, (_u8)rd); goto _two;
    case 3: MOV_Reg2ToReg1(1, Reg_ECX, (_u8)rs); goto _two;
    case 4: MOV_Reg2ToReg1(1, Reg_ECX, (_u8)rd); goto _one;
    case 7: MOV_Reg2ToReg1(1, Reg_ECX, (_u8)rs); goto _one;

    case 5: Reg2ToReg1(1, _Xor, (_u8)rd, (_u8)rd); break;

    case 8: MOV_Reg2ToReg1(1, (_u8)rd, (_u8)rt); break;

    case 6: MOV_Reg2ToReg1(1, Reg_ECX, (_u8)rt);
_one:       MOV_Reg2ToReg1(1, (_u8)rd, (_u8)rt);
_two:       switch (reg->code & 0x1f)
            {
                case 4: SHL_RegByCL(1, (_u8)rd); break;
                case 6: SHR_RegByCL(1, (_u8)rd); break;
                case 7: SAR_RegByCL(1, (_u8)rd); break;
            }
    }
    if (Two_Pass_Compiler)
        PassIndex++;
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_shift_var(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

	_SAFTY_CPU_(r4300i_shift_var)

#ifdef SAFE_SHIFTS
    INTERPRET(r4300i_shift_var); return;
#endif
    FlushAllRegisters();
    x86reg[1].mips_reg = 33;
    if (xRD->mips_reg == 0);
    else
    {
        xRD->IsDirty = 1;
        if (xRD->mips_reg==xRT->mips_reg)
        {
            MapRD;

            if (xRS->mips_reg==xRT->mips_reg)  /*rd=rs=rt*/
            {
                NextPass[PassIndex] = 2;
            }
            else /*rd=rt, rs!=rt*/
            {
                MapRS;
                NextPass[PassIndex] = 3;
            }
        }
        else /*rd != rt */
        {
            if (xRD->mips_reg==xRS->mips_reg)
            {
                MapRD; MapRT;
                NextPass[PassIndex] = 4;
            }
            else  /* rd != rs */
            {
                xRD->NoNeedToLoadTheLo = 1;
                MapRD;
                if (xRS->mips_reg == xRT->mips_reg)
                {
                    if (xRT->mips_reg == 0) /* rd = (rt=0) >> whatever */
                        NextPass[PassIndex] = 5;
                    else
                    {
                        MapRT;
                        NextPass[PassIndex] = 6;
                    }
                }
                else  /* rs != rt */
                {
                    if (xRS->mips_reg != 0)
                    {
                        MapRS; MapRT;
                        /* if rt == 0..what here ? */
                        /* rs!=0, rt!=0 , rs!=rt */
                        NextPass[PassIndex] = 7;
                    }
                    else   /*rs=0, rt!=0 */
                    {
                        MapRT;
                        NextPass[PassIndex] = 8;
                    }
                }
            }
        }
    }   
    x86reg[1].mips_reg = -1;

if (Two_Pass_Compiler)
    PassIndex++;
else 
    asm_shift_var(PASS_PARAMS);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_syscall(OP_PARAMS)
{
    INTERPRET(r4300i_syscall);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_break(OP_PARAMS)
{
    INTERPRET(r4300i_break);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_sync(OP_PARAMS)
{
}

////////////////////////////////////////////////////////////////////////

void r4300i_mf_mt(uint32 Instruction)
{
    switch (Instruction & 0x1f)
    {
        case 16: r4300i_mfhi(Instruction); break;
        case 18: r4300i_mflo(Instruction); break;
        case 17: r4300i_mthi(Instruction); break;
        case 19: r4300i_mtlo(Instruction); break;
    }
}

////////////////////////////////////////////////////////////////////////

void asm_mf_mt(OP_PARAMS)
{
    //int where;
    
    switch (NextPass[PassIndex])
    {
    case 0: ; break;
    case 1:
        MOV_Reg2ToReg1(1, xRD->x86reg, xRS->x86reg);
        //where = CheckWhereIsMipsReg(xRD->mips_reg);
        //if (x86reg[where].Is32bit == 0)
            MOV_Reg2ToReg1(1, xRD->HiWordLoc, xRS->HiWordLoc);
        break;
    }
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_mf_mt(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_mf_mt)

    //note: RD (or RS) is a misnomer. It really corresponds to LO (or HI), 
    //but it works in our Mapping macros

	//MFHI(16)  | MTHI(17)  | MFLO(18)  | MTLO(19)

    NextPass[PassIndex] = 1;
    switch (reg->code & 0x1f)
    {
        case 16: if (xRD->mips_reg == 0) NextPass[PassIndex] = 0; else xRS->mips_reg = __HI; break; //mfhi
        case 18: if (xRD->mips_reg == 0) NextPass[PassIndex] = 0; else xRS->mips_reg = __LO; break; //mflo
        case 17: xRD->mips_reg = __HI; break; //mthi
        case 19: xRD->mips_reg = __LO; break; //mtlo
    }

    if (NextPass[PassIndex] == 1)
    {
        xRD->IsDirty = 1;
        xRD->NoNeedToLoadTheLo = 1;
        xRD->NoNeedToLoadTheHi = 1;

        MapRD;
        MapRS;
    }

#ifndef TWO_PASS_COMPILER
    asm_mf_mt(PASS_PARAMS);
#endif
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsllv(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_dsllv)

    INTERPRET(r4300i_dsllv);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsrlv(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_dsrlv)

    INTERPRET(r4300i_dsrlv);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsrav(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_dsrav)

    INTERPRET(r4300i_dsrav);
}

////////////////////////////////////////////////////////////////////////

void r4300i_mul(uint32 Instruction)
{
    switch (Instruction & 0x3f)
    {
        case 24: r4300i_mult(Instruction);  break;
        case 25: r4300i_multu(Instruction); break;
    }
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_mul(OP_PARAMS)
{
    int temp;
    int High = __HI;
    int Low  = __LO;

    _SAFTY_CPU_(r4300i_mul)
    
    SetRdRsRt32bit(PASS_PARAMS);

    if (ConstMap[__LO].IsMapped == 1) ConstMap[__LO].IsMapped = 0;
    if (ConstMap[__HI].IsMapped == 1) ConstMap[__HI].IsMapped = 0;
    if ((temp = CheckWhereIsMipsReg(__LO)) > -1) { x86reg[temp].IsDirty = 0; FlushRegister(temp); }
    if ((temp = CheckWhereIsMipsReg(__HI)) > -1) { x86reg[temp].IsDirty = 0; FlushRegister(temp); }

    xRD->IsDirty = 1;
    xRD->mips_reg = __LO;
    xRD->Is32bit = 0;
    xRD->NoNeedToLoadTheLo = 1;
    xRD->NoNeedToLoadTheHi = 1;
    MapRD;
    MapRS; MapRT;

    if ((xRD->x86reg != Reg_EAX) && (xRD->HiWordLoc != Reg_EAX)) PUSH_RegIfMapped(Reg_EAX);
    if ((xRD->x86reg != Reg_EDX) && (xRD->HiWordLoc != Reg_EDX)) PUSH_RegIfMapped(Reg_EDX);

    if (xRT->x86reg == Reg_EAX)
    {
        switch (__F)
        {
            case 24: IMUL_EAXWithReg(1, xRS->x86reg); break;
            case 25:  MUL_EAXWithReg(1, xRS->x86reg); break;
        }
    }
    else
    {
        MOV_Reg2ToReg1(1, Reg_EAX, xRS->x86reg);
        switch (__F)
        {
            case 24: IMUL_EAXWithReg(1, xRT->x86reg); break;
            case 25:  MUL_EAXWithReg(1, xRT->x86reg); break;
        }
    }

    if ((xRD->HiWordLoc == Reg_EAX) || (xRD->x86reg == Reg_EDX))
    {
        High = __LO;
        Low  = __HI;
        MOV_Reg2ToReg1(1, xRD->HiWordLoc, Reg_EAX);
        MOV_Reg2ToReg1(1, xRD->x86reg,    Reg_EDX);
    }
    else
    {
        MOV_Reg2ToReg1(1, xRD->x86reg,    Reg_EAX);
        MOV_Reg2ToReg1(1, xRD->HiWordLoc, Reg_EDX);
    }

    if ((xRD->x86reg != Reg_EDX) && (xRD->HiWordLoc != Reg_EDX)) POP_RegIfMapped(Reg_EDX);
    if ((xRD->x86reg != Reg_EAX) && (xRD->HiWordLoc != Reg_EAX)) POP_RegIfMapped(Reg_EAX);

    // split up the Lo and Hi into 2 registers.

    x86reg[xRD->HiWordLoc].IsDirty  = 1;
    x86reg[xRD->HiWordLoc].Is32bit  = 1;
    x86reg[xRD->HiWordLoc].mips_reg = High;
    x86reg[xRD->HiWordLoc].HiWordLoc = xRD->HiWordLoc;

    x86reg[xRD->x86reg].IsDirty   = 1;
    x86reg[xRD->x86reg].Is32bit   = 1;
    x86reg[xRD->x86reg].mips_reg  = Low;
    x86reg[xRD->x86reg].HiWordLoc = xRD->x86reg;
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_div(OP_PARAMS)
{
    int temp, k;

    _SAFTY_CPU_(r4300i_div)


//    FlushAllRegisters();
//Cheesy alternative to FlushAllRegisters() but more efficient.
    if (ConstMap[__LO].IsMapped == 1) ConstMap[__LO].IsMapped = 0;
    if (ConstMap[__HI].IsMapped == 1) ConstMap[__HI].IsMapped = 0;
    if ((temp = CheckWhereIsMipsReg(__LO)) > -1) { x86reg[temp].IsDirty = 0; FlushRegister(temp); }
    if ((temp = CheckWhereIsMipsReg(__HI)) > -1) { x86reg[temp].IsDirty = 0; FlushRegister(temp); }

    if (ConstMap[__RS].IsMapped) FlushOneConstant(__RS);
    if (ConstMap[__RT].IsMapped) FlushOneConstant(__RT);
    if ((temp = CheckWhereIsMipsReg(__RS)) > -1)  FlushRegister(temp);
    if ((temp = CheckWhereIsMipsReg(__RT)) > -1)  FlushRegister(temp);
    if (x86reg[0].mips_reg > -1) FlushRegister(0);
    else if (x86reg[0].mips_reg == -2)
        for (k=0; k<8; k++)
        {
            if (ItIsARegisterNotToUse(k)); //Don't use these registers for mapping
            else if (x86reg[k].HiWordLoc == 0)
            {
                FlushRegister(k);
                k=9;
            }
        }
    if (x86reg[2].mips_reg > -1) FlushRegister(2);
    else if (x86reg[2].mips_reg == -2)
        for (k=0; k<8; k++)
        {
            if (ItIsARegisterNotToUse(k)); //Don't use these registers for mapping
            else if (x86reg[k].HiWordLoc == 2)
            {
                FlushRegister(k);
                k = 9;
            }
        }

    PUSH_RegIfMapped(Reg_EDI);
    LoadLowMipsCpuRegister(__RT,Reg_EDI);
    TEST_Reg2WithReg1(1, Reg_EDI, Reg_EDI);
 
    Jcc_auto(CC_E,0);
        LoadLowMipsCpuRegister(__RS,Reg_EAX);
        MOV_Reg2ToReg1(1, Reg_EDX, Reg_EAX);
        SAR_RegByImm(1, Reg_EDX, 0x1F);

        IDIV_EAXWithReg(1, Reg_EDI);

        x86reg[0].IsDirty = 1;
        x86reg[0].Is32bit = 1;
        x86reg[0].mips_reg = __LO;
        x86reg[0].BirthDate = ThisYear;
        x86reg[0].HiWordLoc = 0;

        x86reg[2].IsDirty = 1;
        x86reg[2].Is32bit = 1;
        x86reg[2].mips_reg = __HI;
        x86reg[2].BirthDate = ThisYear;
        x86reg[2].HiWordLoc = 2;

    SetTarget(0);
    POP_RegIfMapped(Reg_EDI);
 
	SAVE_OP_COUNTER_INCREASE(PCLOCKDIV*2*VICounterFactors[CounterFactor]);	
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_divu(OP_PARAMS)
{
    int temp, k;
    
    _SAFTY_CPU_(r4300i_divu)

//    FlushAllRegisters();
//Cheesy alternative to FlushAllRegisters() but more efficient.
    if (ConstMap[__LO].IsMapped == 1) ConstMap[__LO].IsMapped = 0;
    if (ConstMap[__HI].IsMapped == 1) ConstMap[__HI].IsMapped = 0;
    if ((temp = CheckWhereIsMipsReg(__LO)) > -1) { x86reg[temp].IsDirty = 0; FlushRegister(temp); }
    if ((temp = CheckWhereIsMipsReg(__HI)) > -1) { x86reg[temp].IsDirty = 0; FlushRegister(temp); }

    if (ConstMap[__RS].IsMapped) FlushOneConstant(__RS);
    if (ConstMap[__RT].IsMapped) FlushOneConstant(__RT);
    if ((temp = CheckWhereIsMipsReg(__RS)) > -1)  FlushRegister(temp);
    if ((temp = CheckWhereIsMipsReg(__RT)) > -1)  FlushRegister(temp);
    if (x86reg[0].mips_reg > -1) FlushRegister(0);
    else if (x86reg[0].mips_reg == -2)
        for (k=0; k<8; k++)
        {
            if (ItIsARegisterNotToUse(k)); //Don't use these registers for mapping
            else if (x86reg[k].HiWordLoc == 0)
            {
                FlushRegister(k);
                k=9;
            }
        }
    if (x86reg[2].mips_reg > -1) FlushRegister(2);
    else if (x86reg[2].mips_reg == -2)
        for (k=0; k<8; k++)
        {
            if (ItIsARegisterNotToUse(k)); //Don't use these registers for mapping
            else if (x86reg[k].HiWordLoc == 2)
            {
                FlushRegister(k);
                k = 9;
            }
        }

    PUSH_RegIfMapped(Reg_EDI);
    LoadLowMipsCpuRegister(__RT,Reg_EDI);
    TEST_Reg2WithReg1(1, Reg_EDI, Reg_EDI);

    Jcc_auto(CC_E,0);
        LoadLowMipsCpuRegister(__RS,Reg_EAX);
        XOR_Reg1ToReg2(1, Reg_EDX, Reg_EDX);
        DIV_EAXWithReg(1, Reg_EDI);

        x86reg[0].IsDirty = 1;
        x86reg[0].Is32bit = 1;
        x86reg[0].mips_reg = __LO;
        x86reg[0].BirthDate = ThisYear;
        x86reg[0].HiWordLoc = 0;

        x86reg[2].IsDirty = 1;
        x86reg[2].Is32bit = 1;
        x86reg[2].mips_reg = __HI;
        x86reg[2].BirthDate = ThisYear;
        x86reg[2].HiWordLoc = 2; 

    SetTarget(0);
    POP_RegIfMapped(Reg_EDI);

	SAVE_OP_COUNTER_INCREASE(PCLOCKDIV*2*VICounterFactors[CounterFactor]);
}
////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dmult(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_dmult)

    INTERPRET(r4300i_dmult);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dmultu(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_dmultu)

    INTERPRET(r4300i_dmultu);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_ddiv(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_ddiv)

    INTERPRET(r4300i_ddiv);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_ddivu(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_ddivu)

    INTERPRET(r4300i_ddivu);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_add(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS); 

	_SAFTY_CPU_(r4300i_add)

    if (xRD->mips_reg == 0) return;
#ifdef SAFE_MATH
     INTERPRET(r4300i_add); return;
#endif

    Do3Add(0x02, NONNEG, 0) //TODO: overflow exception
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_addu(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

	_SAFTY_CPU_(r4300i_addu)

    if (xRD->mips_reg == 0) return;
#ifdef SAFE_MATH
     INTERPRET(r4300i_addu); return;
#endif

    Do3Add(0x02, NONNEG, 0)
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_sub(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_sub)

    if (xRD->mips_reg == 0) return;
#ifdef SAFE_MATH
     INTERPRET(r4300i_sub); return;
#endif

     Do3Add(0x2A, NEGATE(xRD), 1) //TODO: overflow exception
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_subu(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_subu)

    if (xRD->mips_reg == 0) return;
#ifdef SAFE_MATH
     INTERPRET(r4300i_subu); return;
#endif

    Do3Add(0x2A, NEGATE(xRD), 1)
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_and(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_and)

    if (xRD->mips_reg == 0) return;

#ifdef SAFE_GATES
     INTERPRET(r4300i_and); return;
#endif

        if (xRD->mips_reg==xRT->mips_reg)
        {
            if (xRS->mips_reg==xRT->mips_reg)
            {
            }
        else if (xRD->mips_reg != xRS->mips_reg)
            {
                xRD->IsDirty = 1;
                MapRD;
                MapRS;
                AND_Reg2ToReg1(1, xRD->x86reg,    xRS->x86reg);
                AND_Reg2ToReg1(1, xRD->HiWordLoc, xRS->HiWordLoc);
            }
        }
    else if (xRD->mips_reg==xRS->mips_reg)
            {
        //FlushAllRegisters();
        //INTERPRET(r4300i_and); return;
                xRD->IsDirty = 1;
                MapRD;
                MapRT;

                AND_Reg2ToReg1(1, xRD->x86reg,    xRT->x86reg);
                AND_Reg2ToReg1(1, xRD->HiWordLoc, xRT->HiWordLoc);
            }
            else
            {
                xRD->NoNeedToLoadTheHi = 1;
                xRD->NoNeedToLoadTheLo = 1;
                xRD->IsDirty = 1;
                MapRD;
                MapRS;
                MapRT;

                MOV_Reg2ToReg1(1, xRD->x86reg,    xRS->x86reg);
                AND_Reg2ToReg1(1, xRD->x86reg,    xRT->x86reg);
                MOV_Reg2ToReg1(1, xRD->HiWordLoc, xRS->HiWordLoc);
                AND_Reg2ToReg1(1, xRD->HiWordLoc, xRT->HiWordLoc);
            }
        }


////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------------
	-----------------------------------------------------------------
	| OR        | OR                                                |
	|-----------|---------------------------------------------------|
	|  000000   |   rs    |   rt    |    rd   |  00000  |100101 (37)|
	------6----------5---------5---------5---------5----------6------
	 Format:  OR rd, rs, rt
	 Purpose: To do a bitwise logical OR.
	 Descrip: rd = (rs OR rt)
---------------------------------------------------------------------------------------*/
void dyna4300i_special_or(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS);
	if( xRD->mips_reg == 0) return;

    _SAFTY_CPU_(r4300i_or)

#ifdef SAFE_GATES
     INTERPRET(r4300i_or); return;
#endif

#define _Or 0x0B
    
    if (CheckIs32Bit(xRS->mips_reg) && CheckIs32Bit(xRT->mips_reg))
    {
        xRD->Is32bit = 1;
        xRS->Is32bit = 1;
        xRT->Is32bit = 1;
        xRD->IsDirty = 1;

    //32bit
    if (xRD->mips_reg != 0)
    {
        if (xRD->mips_reg==xRT->mips_reg)
        {
            if (xRS->mips_reg==xRT->mips_reg)
            {
            }
            else
            {
                xRD->IsDirty = 1;
                MapRD;
                MapRS;
                Reg2ToReg1(1, _Or , xRD->x86reg,    xRS->x86reg);       /* or    rd,rs (lo) */
            }
        }
        else
        {
            if (xRD->mips_reg==xRS->mips_reg)
            {
                xRD->IsDirty = 1;
                MapRD;
                MapRT;

                Reg2ToReg1(1, _Or , xRD->x86reg,    xRT->x86reg);       /* or    rd,rt (lo) */
            }
            else
            {
                if (xRS->mips_reg!=xRT->mips_reg) // rs!=rt, rd!=rt, rd!=rs
                {
                    xRD->NoNeedToLoadTheHi = 1;
                    xRD->IsDirty = 1;
                    MapRD;
                    if ((xRT->mips_reg == 0))
                    {
                        MapRS;

                        MOV_Reg2ToReg1(1,      xRD->x86reg,    xRS->x86reg);     /* mov      rd,rs (lo) */
                    }
                    else
                    {
                        MapRS;
                        MapRT;

                        MOV_Reg2ToReg1(1,      xRD->x86reg,    xRS->x86reg);     /* mov      rd,rs (lo) */
                            Reg2ToReg1(1, _Or, xRD->x86reg,    xRT->x86reg);     /* or       rd,rt (lo) */
                    }
                }
                else
                {
                    xRD->NoNeedToLoadTheLo = 1;
                    xRD->IsDirty = 1;
                    MapRD;
                    MapRS;

                    MOV_Reg2ToReg1(1, xRD->x86reg,    xRS->x86reg);     /* mov      rd,rs (lo) */
                }
            }
        }
    }


    }
    
    //64bit
    else 
    if (xRD->mips_reg != 0)
    {
        if (xRD->mips_reg==xRT->mips_reg)
        {
            if (xRS->mips_reg==xRT->mips_reg)
            {
            }
            else
            {
                xRD->IsDirty = 1;
                MapRD;
                MapRS;
                Reg2ToReg1(1, _Or , xRD->x86reg,    xRS->x86reg);       /* or    rd,rs (lo) */
                Reg2ToReg1(1, _Or , xRD->HiWordLoc, xRS->HiWordLoc);    /* or    rd,rs (hi) */
            }
        }
        else
        {
            if (xRD->mips_reg==xRS->mips_reg)
            {
                xRD->IsDirty = 1;
                MapRD;
                MapRT;

                Reg2ToReg1(1, _Or , xRD->x86reg,    xRT->x86reg);       /* or    rd,rt (lo) */
                Reg2ToReg1(1, _Or , xRD->HiWordLoc, xRT->HiWordLoc);    /* or    rd,rt (hi) */
            }
            else
            {
                if (xRS->mips_reg!=xRT->mips_reg) // rs!=rt, rd!=rt, rd!=rs
                {
                    xRD->NoNeedToLoadTheHi = 1;
                    xRD->NoNeedToLoadTheLo = 1;
                    xRD->IsDirty = 1;
                    MapRD;
                    if ((xRT->mips_reg == 0))
                    {
                        MapRS;

                        MOV_Reg2ToReg1(1,      xRD->x86reg,    xRS->x86reg);     /* mov      rd,rs (lo) */
                        MOV_Reg2ToReg1(1,      xRD->HiWordLoc, xRS->HiWordLoc);  /* mov      rd,rs (hi) */
                    }
                    else
                    {
                        MapRS;
                        MapRT;

                        MOV_Reg2ToReg1(1,      xRD->x86reg,    xRS->x86reg);     /* mov      rd,rs (lo) */
                            Reg2ToReg1(1, _Or, xRD->x86reg,    xRT->x86reg);     /* or       rd,rt (lo) */
                        MOV_Reg2ToReg1(1,      xRD->HiWordLoc, xRS->HiWordLoc);  /* mov      rd,rs (hi) */
                            Reg2ToReg1(1, _Or, xRD->HiWordLoc, xRT->HiWordLoc);  /* or       rd,rt (hi) */
                    }
                }
                else
                {
                    xRD->NoNeedToLoadTheHi = 1;
                    xRD->NoNeedToLoadTheLo = 1;
                    xRD->IsDirty = 1;
                    MapRD;
                    MapRS;

                    MOV_Reg2ToReg1(1, xRD->x86reg,    xRS->x86reg);     /* mov      rd,rs (lo) */
                    MOV_Reg2ToReg1(1, xRD->HiWordLoc, xRS->HiWordLoc);  /* mov      rd,rs (hi) */
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_xor(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS); 

    _SAFTY_CPU_(r4300i_xor) 

#ifdef SAFE_GATES
     INTERPRET(r4300i_xor); return;
#endif 

#ifndef _Xor
#define _Xor 0x33
#endif 

    if (xRD->mips_reg == 0) return;
    if (xRD->mips_reg==xRT->mips_reg)
    {
        if (xRS->mips_reg==xRT->mips_reg)
        {
#ifndef NO_CONSTS
        int where = CheckWhereIsMipsReg(xRD->mips_reg);
        ConstMap[xRD->mips_reg].IsMapped = 1;
        ConstMap[xRD->mips_reg].value    = 0;
        
        if (where > -1) 
        {
            x86reg[where].IsDirty = 0;
            FlushRegister(where);
        }
#else
        xRD->IsDirty = 1;
        xRD->Is32bit = 1;
        xRD->NoNeedToLoadTheLo = 1;
        MapRD;
        XOR_Reg1ToReg2(1, xRD->x86reg, xRD->x86reg);
#endif
        }
        else
        {
            xRD->IsDirty = 1; 
            MapRD;
            MapRS; 

            Reg2ToReg1(1, _Xor , xRD->x86reg,    xRS->x86reg);       /* xor    rd,rs (lo) */
            Reg2ToReg1(1, _Xor , xRD->HiWordLoc, xRS->HiWordLoc);    /* xor    rd,rs (hi) */
        }
    }
    else if (xRD->mips_reg==xRS->mips_reg)
    {
        xRD->IsDirty = 1;
        MapRD;
        MapRT; 

        Reg2ToReg1(1, _Xor , xRD->x86reg,    xRT->x86reg);        /* xor    rd,rt (lo) */
        Reg2ToReg1(1, _Xor , xRD->HiWordLoc, xRT->HiWordLoc);     /* xor    rd,rt (hi) */
    }
    else // rd!=rt, rd!=rs, 
    {
        xRD->NoNeedToLoadTheHi = 1;
        xRD->NoNeedToLoadTheLo = 1;
        xRD->IsDirty = 1; 

        MapRD;
        MapRS;
        MapRT; 

        MOV_Reg2ToReg1(1, xRD->x86reg,    xRS->x86reg);      /* mov     rd,rs (lo) */
        MOV_Reg2ToReg1(1, xRD->HiWordLoc, xRS->HiWordLoc);   /* mov     rd,rs (hi) */
        XOR_Reg2ToReg1(1, xRD->x86reg,    xRT->x86reg);      /* xor     rd,rt (lo) */
        XOR_Reg2ToReg1(1, xRD->HiWordLoc, xRT->HiWordLoc);   /* xor     rd,rt (hi) */
    }
} 

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_nor(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS);
	if( xRD->mips_reg == 0) return;

    _SAFTY_CPU_(r4300i_nor)

#ifdef SAFE_GATES
     INTERPRET(r4300i_nor); return;
#endif

    if (xRD->mips_reg != 0 )
    {
        if (xRD->mips_reg==xRT->mips_reg)
        {
            if (xRS->mips_reg==xRT->mips_reg)
            {
                xRD->IsDirty = 1;
                MapRD;

                XOR_ImmToReg(1, xRD->x86reg,    0xffffffff);
                XOR_ImmToReg(1, xRD->HiWordLoc, 0xffffffff);
            }
            else
            {
                xRD->IsDirty = 1;
                MapRD;
                MapRS;

                Reg2ToReg1(1, _Or , xRD->x86reg,    xRS->x86reg);       /* or    rd,rs (lo) */
                XOR_ImmToReg(1, xRD->x86reg, 0xffffffff);
                Reg2ToReg1(1, _Or , xRD->HiWordLoc, xRS->HiWordLoc);    /* or    rd,rs (hi) */
                XOR_ImmToReg(1, xRD->HiWordLoc, 0xffffffff);
            }
        }
        else
        {
            if (xRD->mips_reg==xRS->mips_reg)
            {
                xRD->IsDirty = 1;
                MapRD;
                MapRT;

                Reg2ToReg1(1, _Or , xRD->x86reg,    xRT->x86reg);       /* or    rd,rt (lo) */
                XOR_ImmToReg(1, xRD->x86reg, 0xffffffff);
                Reg2ToReg1(1, _Or , xRD->HiWordLoc, xRT->HiWordLoc);    /* or    rd,rt (hi) */
                XOR_ImmToReg(1, xRD->HiWordLoc, 0xffffffff);
            }
            else /* rd!=rt, rd!=rs */
            {
                xRD->NoNeedToLoadTheHi = 1;
                xRD->NoNeedToLoadTheLo = 1;
                xRD->IsDirty = 1;
                MapRD;
                MapRS;

                if (xRS->mips_reg!=xRT->mips_reg) /* rd!=rt, rd!=rs, rs!=rt */
                {
                    MapRT;

                    MOV_Reg2ToReg1(1, xRD->x86reg,    xRS->x86reg);                     /* mov      rd,rs (lo) */
                        Reg2ToReg1(1, _Or ,           xRD->x86reg,    xRT->x86reg);     /* or       rd,rt (lo) */
                    XOR_ImmToReg(1, xRD->x86reg,    0xffffffff);
                    MOV_Reg2ToReg1(1, xRD->HiWordLoc, xRS->HiWordLoc);                  /* mov      rd,rs (hi) */
                        Reg2ToReg1(1, _Or,            xRD->HiWordLoc, xRT->HiWordLoc);  /* or       rd,rt (hi) */
                    XOR_ImmToReg(1, xRD->HiWordLoc, 0xffffffff);
                }
                else /* rd!=rt, rd!=rs, rs=rt */
                {
                    MOV_Reg2ToReg1(1, xRD->x86reg,    xRS->x86reg);     /* mov      rd,rs (lo) */
                    XOR_ImmToReg(1, xRD->x86reg,   0xffffffff);
                    MOV_Reg2ToReg1(1, xRD->HiWordLoc, xRS->HiWordLoc);  /* mov      rd,rs (hi) */
                    XOR_ImmToReg(1, xRD->HiWordLoc, 0xffffffff);
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dadd(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_dadd)

    INTERPRET(r4300i_dadd);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_daddu(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_daddu)

    INTERPRET(r4300i_daddu);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsub(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_dsub)

    INTERPRET(r4300i_dsub);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsubu(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_dsubu)

    INTERPRET(r4300i_dsubu);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_tge(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_tge)

    INTERPRET(r4300i_tge);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_tgeu(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_tgeu)

    INTERPRET(r4300i_tgeu);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_tlt(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_tlt)

    INTERPRET(r4300i_tlt);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_tltu(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_tltu)

    INTERPRET(r4300i_tltu);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_teq(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_teq)

    INTERPRET(r4300i_teq);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_tne(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_tne)

    INTERPRET(r4300i_tne);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsll(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_dsll)

    INTERPRET(r4300i_dsll);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsrl(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_dsrl)

    INTERPRET(r4300i_dsrl);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsra(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_dsra)

    INTERPRET(r4300i_dsra);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsll32(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_dsll32)

    if (xRD->mips_reg == 0) return;
    if (xRT->mips_reg == xRD->mips_reg)
    {
        xRD->NoNeedToLoadTheHi = 1;
        xRD->IsDirty=1;
        MapRD;
        MOV_Reg2ToReg1(1, xRD->HiWordLoc, xRD->x86reg);
          SHL_RegByImm(1, xRD->HiWordLoc, (_s8)__SA);
        XOR_Reg1ToReg2(1, xRD->x86reg,    xRD->x86reg);
    }
    else
    {
        xRD->IsDirty = 1;
        xRD->NoNeedToLoadTheLo = 1;
        xRD->NoNeedToLoadTheHi = 1;
        MapRD;

        xRT->Is32bit = 1;
        MapRT;

        MOV_Reg2ToReg1(1, xRD->HiWordLoc, xRT->x86reg);
          SHL_RegByImm(1, xRD->HiWordLoc, (_s8)__SA);
        XOR_Reg1ToReg2(1, xRD->x86reg,    xRD->x86reg);
    }
}

////////////////////////////////////////////////////////////////////////
extern void StoreMipsCpuRegister(unsigned long iMipsReg,unsigned char iIntelReg1,unsigned char iIntelReg2);

void dyna4300i_special_dsrl32(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_dsrl32)
#ifdef SAFE_SHIFTS
    INTERPRET(r4300i_dsrl32);
#endif

    if (xRD->mips_reg == 0) return;
    INTERPRET(r4300i_dsrl32); return;
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsra32(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_dsra32)

//INTERPRET(r4300i_dsra32);
//00418569 A1 24 21 A9 00       mov         eax,[_GPR+4 (00a92124)]
//0041856E A3 30 21 A9 00       mov         [_GPR+10h (00a92130)],eax
//00418573 A1 20 21 A9 00       mov         eax,[_GPR (00a92120)]
//00418578 C1 F8 1F             sar         eax,XXh (<- Shift Amount)
//0041857B 99                   cdq
//0041857C A3 30 21 A9 00       mov         [_GPR+10h (00a92130)],eax
//00418581 89 15 34 21 A9 00    mov         dword ptr [_GPR+14h (00a92134)],edx

    if (xRD->mips_reg == 0) return;

    
    INTERPRET(r4300i_dsra32); return;    //need to rewrite because we are not necessarily flushing the hiword to mem
                                                  //when the register was proven to be used as 32bit.
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_tgei(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_tgei)

   INTERPRET(r4300i_tgei);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_tgeiu(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_tgeiu)

    INTERPRET(r4300i_tgeiu);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_tlti(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_tlti)

    INTERPRET(r4300i_tlti);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_tltiu(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_tltiu)

    INTERPRET(r4300i_tltiu);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_teqi(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_teqi)

    INTERPRET(r4300i_teqi);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_tnei(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_tnei)

    INTERPRET(r4300i_tnei);
    HELP_CheckExceptionSlot(reg->pc);
}


/******************************************************************************\
*                                                                              *
*   CoProcessor0 (COP0)                                                        *
*                                                                              *
\******************************************************************************/

void dyna4300i_cop0_rs_mf(OP_PARAMS)
{
    _SAFTY_CPU_(r4300i_COP0_mfc0)    
    /*
   SetRdRsRt32bit(PASS_PARAMS);

_SAFTY_CPU_(r4300i_cop0_rs_mf)

if (xRT->mips_reg == 0)  return;

    xRT->NoNeedToLoadTheLo = 1;
    xRT->IsDirty = 1;
    MapRT;
    MOV_MemoryToReg(1, xRT->x86reg,ModRM_disp32, (unsigned long)&(_u32)reg->COP0Reg[__RD]);
    */
    INTERPRET(r4300i_COP0_mfc0);
}

////////////////////////////////////////////////////////////////////////

//Is this an op???????????????????????????????????
void dyna4300i_cop0_rs_dmf(OP_PARAMS)
{
    _u32 rd = (unsigned long)&(_u32)reg->COP0Reg[__RD];
    SetRdRsRt32bit(PASS_PARAMS);

//_SAFTY_CPU_(r4300i_cop0_rs_dmf)
    if (xRT->mips_reg == 0)  return;

    xRT->NoNeedToLoadTheLo = 1;
    xRT->IsDirty = 1;
    MapRT;
    MOV_MemoryToReg(1, xRT->x86reg, ModRM_disp32, rd);
}

////////////////////////////////////////////////////////////////////////

//Is this an op???????????????????????????????????
void dyna4300i_cop0_rs_cf(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);
    
//    _SAFTY_CPU_(r4300i_cop0_rs_cf)

    if (xRT->mips_reg == 0)  return;

    
    xRT->IsDirty = 1;
    xRT->NoNeedToLoadTheLo = 1;
    MapRT;
    MOV_MemoryToReg(1, xRT->x86reg, ModRM_disp32, (unsigned long )&reg->COP0Con[__FS]);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_rs_mt(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_COP0_mtc0)

    INTERPRET(r4300i_COP0_mtc0);
}

////////////////////////////////////////////////////////////////////////

//Is this an op???????????????????????????????????
void dyna4300i_cop0_rs_dmt(OP_PARAMS)
{
//_SAFTY_CPU_(r4300i_cop0_rs_dmt)

    DisplayError("%08X: Unhandled dmtc0", reg->pc);
//    INTERPRET(r4300i_COP0_dmtc0);
}

////////////////////////////////////////////////////////////////////////

//Is this an op???????????????????????????????????
void dyna4300i_cop0_rs_ct(OP_PARAMS)
{
    _u32 fs = (unsigned long)&reg->COP0Con[__FS];
    SetRdRsRt64bit(PASS_PARAMS);

//_SAFTY_CPU_(r4300i_COP0_rs_ct)

    MapRT;
    MOV_RegToMemory(1,xRT->x86reg,    ModRM_disp32, fs  );
    MOV_RegToMemory(1,xRT->HiWordLoc, ModRM_disp32, fs+4);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_rs_bc(OP_PARAMS)
{
    dyna_cop0_rt_instruction[__RT](PASS_PARAMS);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_tlb(OP_PARAMS)
{
    dyna_tlb_instruction[__F](PASS_PARAMS);   
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_rt_bcf(OP_PARAMS)
{
    DisplayError("%08X: Unhandled BCFC0", reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_rt_bct(OP_PARAMS)
{
    DisplayError("%08X: Unhandled BCTC0", reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_rt_bcfl(OP_PARAMS)
{
    DisplayError("%08X: Unhandled BCFL0", reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_rt_bctl(OP_PARAMS)
{
    DisplayError("%08X: Unhandled BCTL0", reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_tlbr(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_COP0_tlbr)

    INTERPRET(r4300i_COP0_tlbr);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_tlbwi(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_COP0_tlbwi)

    INTERPRET(r4300i_COP0_tlbwi);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_tlbwr(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_COP0_tlbwr)

    INTERPRET(r4300i_COP0_tlbwr);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_tlbp(OP_PARAMS)
{
	_SAFTY_CPU_(r4300i_COP0_tlbp)

    INTERPRET(r4300i_COP0_tlbp);
}

////////////////////////////////////////////////////////////////////////

/******************************************************************************\
*                                                                              *
*   CoProcessor2 (COP2)                                                        *
*                                                                              *
\******************************************************************************/

void dyna4300i_cop2_rs_not_implemented(OP_PARAMS)
{
    DisplayError("%08X: There isnt a COP2\n", reg->pc);
}


///////////////////////////////////

void Dyna_Set_FPU_Usability(BOOL usable)
{
	if( usable )
	{
		dyna_instruction[0x11] = dyna4300i_cop1;
		//TRACE0("FPU is enabled");
	}
	else
	{
		dyna_instruction[0x11] = dyna4300i_cop1_with_exception;
		//TRACE0("FPU is Disabled");
	}
}
