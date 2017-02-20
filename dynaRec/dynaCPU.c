#define _SAFTY_CPU_(x)
//#define _SAFTY_CPU_(x)  INTERPRET(x); return;

//#define SAFE_SLT          1
//#define SAFE_IMM          1
//#define SAFE_GATES        1
//#define SAFE_LOADSTORE    1
//#define SAFE_MATH         1
//#define SAFE_SHIFTS       1

//-- Notes --
// Search for "FIXME!!!"
//Always map the dirty reg first b4 the others

#define DisplayAddress DisplayError("LcodePosition=%08X", &RecompCode[lCodePosition]);
#define CheckForConstMap  if ((ConstMap[xRD->mips_reg].IsMapped) || (ConstMap[xRS->mips_reg].IsMapped) || (ConstMap[xRT->mips_reg].IsMapped)) DisplayError("whui");

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

// prototypes
extern BOOL CPUIsRunning;
extern int __cdecl SetStatusBar(char *debug, ...);
extern void HELP_debug(unsigned long pc);
extern _u32 ThisYear;

void FlushConstants();
//void FlushRegister(_u32 k);
void FlushRegister(int k);
int CheckWhereIsMipsReg(int mips_reg);


#include "dynaHelper.h"     // branches and jumps   interpretive
#include "dynabranch.h"     // branches and jumps

//MapConstant ConstMap[32];
FlushedMap  FlushedRegistersMap[32];

_u32 translatepc;
int AlreadyRecompiled;

x86regtyp   xRD[1];
x86regtyp   xRS[1];
x86regtyp   xRT[1];
x86regtyp   xLO[1];
x86regtyp   xHI[1];

dyn_cpu_instr dyna_instruction[64] =
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

dyn_cpu_instr dyna_special_instruction[64] =
{
    dyna4300i_special_sll,   dyna4300i_reserved,      dyna4300i_special_srl,  dyna4300i_special_sra,  dyna4300i_special_sllv,    dyna4300i_reserved,       dyna4300i_special_srlv,   dyna4300i_special_srav,
    dyna4300i_special_jr,    dyna4300i_special_jalr,  dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_special_syscall, dyna4300i_special_break,  dyna4300i_reserved,       dyna4300i_special_sync,
    dyna4300i_special_mfhi,  dyna4300i_special_mthi,  dyna4300i_special_mflo, dyna4300i_special_mtlo, dyna4300i_special_dsllv,   dyna4300i_reserved,       dyna4300i_special_dsrlv,  dyna4300i_special_dsrav,
    dyna4300i_special_mult,  dyna4300i_special_multu, dyna4300i_special_div,  dyna4300i_special_divu, dyna4300i_special_dmult,   dyna4300i_special_dmultu, dyna4300i_special_ddiv,   dyna4300i_special_ddivu, 
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
    dyna4300i_cop0_rs_co, dyna4300i_cop0_rs_co,  dyna4300i_cop0_rs_co, dyna4300i_cop0_rs_co, dyna4300i_cop0_rs_co, dyna4300i_cop0_rs_co,  dyna4300i_cop0_rs_co, dyna4300i_cop0_rs_co,
    dyna4300i_cop0_rs_co, dyna4300i_cop0_rs_co,  dyna4300i_cop0_rs_co, dyna4300i_cop0_rs_co, dyna4300i_cop0_rs_co, dyna4300i_cop0_rs_co,  dyna4300i_cop0_rs_co, dyna4300i_cop0_rs_co
};

dyn_cpu_instr dyna_cop0_rt_instruction[32] =
{
    dyna4300i_cop0_rt_bcf, dyna4300i_cop0_rt_bct, dyna4300i_cop0_rt_bcfl, dyna4300i_cop0_rt_bctl, dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved,
    dyna4300i_reserved,    dyna4300i_reserved,    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved,
    dyna4300i_reserved,    dyna4300i_reserved,    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved,
    dyna4300i_reserved,    dyna4300i_reserved,    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved, dyna4300i_reserved
};

dyn_cpu_instr dyna_cop0_instruction[64] =
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


//// BAD STUFF, BUT HERE NOW FOR COMPATIBILITY ////

void HELP_CheckExceptionSlot(_u32 pc){}
_u8* GetStaticMemAddr(_u32 Addr){ return(NULL);}


_u8* InterruptVector;
_u8* TLBMissStart;
_u8* IVTStart;
/////////////////////////

BOOL Init_Dynarec()
{
    if (dyna_CodeTable != NULL) VirtualFree(dyna_CodeTable,		CODETABLE_SIZE,		MEM_DECOMMIT);
    if (dyna_RecompCode != NULL) VirtualFree(dyna_RecompCode,	RECOMPCODE_SIZE,	MEM_DECOMMIT);
	if (InterruptVector != NULL) VirtualFree(InterruptVector,	8192,	MEM_DECOMMIT);

    dyna_CodeTable = (_u8*)VirtualAlloc(NULL, CODETABLE_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE); // 8mb ist die RDRam Size
    dyna_RecompCode = (_u8*)VirtualAlloc(NULL, RECOMPCODE_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	lCodePosition = 0;

    InterruptVector = (uint8*)VirtualAlloc(NULL, 8192, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	AlreadyRecompiled = FALSE;
    TLBMissStart = InterruptVector;

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

	memset(dyna_RecompCode, 0x00, RECOMPCODE_SIZE);
    memset(dyna_CodeTable, 0xFF, CODETABLE_SIZE);       // bis jetzt gibt es noch
                                                        // keinen compilierten Code

    SetTranslator(dyna_RecompCode, 0, RECOMPCODE_SIZE);

    InitLogDyna();
    InitRegisterMap();

    memset(xRD, 0, sizeof(xRD));
    memset(xRS, 0, sizeof(xRS));
    memset(xRT, 0, sizeof(xRT));
    memset(xLO, 0, sizeof(xLO));
    memset(xHI, 0, sizeof(xHI));

	memset(DynaSP_REG,0x00,131074);
	memset(DynaRDRAM,0x00,0x00800000);
	memset(DynaROM,0x00,gAllocationLength);
    return(TRUE);
}

void Free_Dynarec()
{
    free(dyna_CodeTable);
    free(dyna_RecompCode);
}


#ifdef WINDEBUG_1964
#include "../win32/windebug.h"
extern void HandleBreakpoint(uint32 Instruction);
#endif

//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Compile the IVT (Interrupt Vector table)

void rc_RecompileIVT()
{
    uint32 Instruction;
    uint32 templCodePosition = lCodePosition;
	cp0Counter=0;

    SetTranslator(InterruptVector, 0, 8192);

    KEEP_RECOMPILING = 1;

    pcptr = (uint32*)((uint8*)sDWORD_R[0x8000] + (0x0080));
    (*(uint32*)((uint8*)sDYN_PC_LOOKUP[0x8000>>16] + 0x0080)) = (uint32)(&InterruptVector[0]);
    RegFetchOpcode

_BEAVIS:    
    while(KEEP_RECOMPILING)
    {
        __asm add gHardwareState.pc, 4
		cp0Counter++;
        *pcptr++;
        if (gHardwareState.pc == 0x80000180)
		{
            IVTStart = &InterruptVector[0];
			cp0Counter=0;
		}
        RegFetchOpcode
    }

    if (gHardwareState.pc < 0x80000180)
    {
        KEEP_RECOMPILING = 1;
		cp0Counter=0;
        goto _BEAVIS;
    }

    SetTranslator(dyna_RecompCode, templCodePosition, RECOMPCODE_SIZE);
}

//---------------------------------------------------------------------------------------
// This function performs routine interrupt checking

void rc_Intr_Common()
{

	uint32 Instruction;
    

	/*
    if ((( gHardwareState.COP0Reg[STATUS] & IE   )) == 0 )
	{
		// Do nothing if interrupt is disabled.
		return;
	}
	*/

    if ((( gHardwareState.COP0Reg[STATUS] & EXL_OR_ERL  )) != 0 )
	{
		// Do nothing if ERL(Error) and EXL(Exception) happens together
		// This should never happen
		return;
	}

	/*
    if (( gHardwareState.COP0Reg[CAUSE] & 0x0000FF00) == 0)
	{
		// If there is any interrupts pending, do nothing if no interrupts
		// Do not process Exceptions
		return;
	}
	*/

	/*
    if (( gHardwareState.COP0Reg[CAUSE] & gHardwareState.COP0Reg[STATUS] & 0x0000FF00) == 0	// Check interrupts
		&& gHardwareState.COP0Reg[CAUSE] & 0x0000007c != 0 )    
	{
		return;
	}
	*/

    gHardwareState.COP0Reg[EPC] = gHardwareState.pc;
    gHardwareState.COP0Reg[STATUS] |= EXL;          // set EXL = 1 

	/*
	if( gHardwareState.COP0Reg[STATUS] & 0x04000000 == 0 )
			gHardwareState.pc = 0x80000180;
	else
			gHardwareState.pc = 0xBFC00380;

	do
	{
		Instruction = LOAD_UWORD_PARAM(gHardwareState.pc);
		CPU_instruction[_OPCODE_](Instruction);
		gHardwareState.pc+=4;
	}
	while( _OPCODE_ != 16 );
	*/

    if (!AlreadyRecompiled)
    {
		// Compile the Interrupt service routines.
		// 1964 does not provide interrupt service, the emulated program has the service routines.
               
        gHardwareState.pc = 0x80000080;
        rc_RecompileIVT();

        AlreadyRecompiled = 1;
    }

    if ( (gHardwareState.COP0Reg[CAUSE] & TLBL_Miss) || (gHardwareState.COP0Reg[CAUSE] & TLBS_Miss))
	{
		gHardwareState.pc = 0x80000080;
    }
	else
	{
		gHardwareState.pc = 0x80000180;
	}

    gHardwareState.COP0Reg[CAUSE] &= NOT_BD;        // clear BD 
    gHardwareState.COP0Reg[CAUSE] &= NOT_EXCCODE;   // clear EXCCode
    
    Block = IVTStart;
}


////////////////////////


extern _u32 next_vi_interrupt;
extern void TimingThread();
void CheckTheInterrupts(_u32 count)
{
}

extern double fps;
extern long DListCounter, AListCounter;
extern unsigned __int32 CounterFactor;
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

_u32 CompilePC()
{
    static int InitialCondition = 0;
// Header of a Block 
#ifdef LOG_DYNA
    LogDyna("\n");
    LogDyna("Compile PC at 0x%08X\n", reg->pc);
#endif LOG_DYNA

// DynaRec a Block
    _u32 dynacodeentry = lCodePosition;
    _u32    tmpPC = gHardwareState.pc;
    _u32 initialcondition = 0;


    counter = 0;
    KEEP_RECOMPILING = 1;
    DynaBufferOverError = FALSE;

    COP1_exception_speedup = TRUE;

    do
    {
        _u8 op;

//if (reg->pc == 0x800805C0)
//{
//  int t = 0;
//}
        counter++;      // used for the COUNT-Reg

        if (initialcondition == 1) *pcptr++;
        gHardwareState.code = *pcptr;
        initialcondition = 1;

        op = (_u8)(gHardwareState.code >> 26);

#ifdef _DEBUG
//        T_RS = __dotRS; T_RT = __dotRT; T_RD = __dotRD;
#endif

#ifdef _USE_DEBUGGER_
        HELP_Call((unsigned long)HELP_debug);
#endif _USE_DEBUGGER_

        dyna_instruction[op](&gHardwareState);

        if (DynaBufferOverError)
        {
            gHardwareState.pc = tmpPC;
            return CompilePC();
        }

        gHardwareState.pc += 4;
    } while(KEEP_RECOMPILING);

// Feet of a Block
#ifdef LOG_DYNA
    LogDyna("  *** increase Count-Reg and do common things\n");
#endif LOG_DYNA

    if (DynaBufferOverError)
    {
        gHardwareState.pc = tmpPC;
        return CompilePC();
    }

    return dynacodeentry;
}

_u32 c = 0;
void store_regs()
{
    int i;
    FILE *stream = fopen("c:/reg1.txt", "at");
    fprintf(stream, "\n\npc: %08X c: %x\n", gHardwareState.pc, c);
    for (i=0; i<32; i++)
        fprintf(stream, "reg%02i 0x%08X\n", i, gHardwareState.GPR[i]);

    fclose(stream);
}



extern unsigned char* RecompCode;
extern unsigned long lCodePosition;

/******************************************************************************\
*                                                                              *
*   Helper                                                                     *
*                                                                              *
\******************************************************************************/


///////////////////////////////////////////////////////////////////////////////
// Test functions
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************\
*                                                                              *
*   OpCodes                                                                    *
*                                                                              *
\******************************************************************************/

void dyna4300i_reserved(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	RESERVED INSTRCUTION\n", reg->pc);
#endif LOG_DYNA

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
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	ADDI r%02i = r%02i + 0x%08X\n", reg->pc, __RT, __RS, (_u64)(_u16)__I);
#endif LOG_DYNA

    
#ifdef SAFE_IMM
     INTERPRET(r4300i_addi); return;
#endif

     DoConstAdd((_s32)(_s16)__I, NONNEG)
     //TODO: Check 32bit overflow
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_addiu(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);
    if (xRT->mips_reg == 0) return;

_SAFTY_CPU_(r4300i_addiu)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	ADDIU r%02i = r%02i + 0x%08X\n", reg->pc, __RT, __RS, (_u64)(_u16)__I);
#endif LOG_DYNA
#ifdef SAFE_IMM
     INTERPRET(r4300i_addiu); return;
#endif

    DoConstAdd((_s32)(_s16)__I, NONNEG)
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_slt(OP_PARAMS)
{

#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SLT\n", reg->pc);
#endif LOG_DYNA

    SetRdRsRt64bit(PASS_PARAMS);

_SAFTY_CPU_(r4300i_slt)

    if (xRD->mips_reg == 0) return;
#ifdef SAFE_SLT
     INTERPRET(r4300i_slt); return;
#endif

    if (xRS->mips_reg == xRT->mips_reg)
    {
        int where = CheckWhereIsMipsReg(xRD->mips_reg);
        ConstMap[xRD->mips_reg].IsMapped = 1;
        ConstMap[xRD->mips_reg].value    = 0;
        if (where > -1) 
        {
            x86reg[where].IsDirty = 0;
            FlushRegister(where);
        }
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

        WC16(0x016A); // push 1
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
        WC16(0x016A); // push 1
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

void dyna4300i_special_sltu(OP_PARAMS)
{

#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SLTU\n", reg->pc);
#endif LOG_DYNA

    SetRdRsRt64bit(PASS_PARAMS);

_SAFTY_CPU_(r4300i_sltu)
#ifdef SAFE_SLT
    INTERPRET(r4300i_sltu); return;
#endif

    if (xRD->mips_reg == 0) return;    

    if (xRS->mips_reg == xRT->mips_reg)
    {
        int where = CheckWhereIsMipsReg(xRD->mips_reg);
        ConstMap[xRD->mips_reg].IsMapped = 1;
        ConstMap[xRD->mips_reg].value    = 0;
        if (where > -1) 
        {
            x86reg[where].IsDirty = 0;
            FlushRegister(where);
        }
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

        WC16(0x016A); // push 1
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
        WC16(0x016A); // push 1
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
    _s64 ConstInt     = (_s64)(_s32)__I;
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
            CMP_RegWithImm(1, xRT->HiWordLoc, (_u32)(ConstInt >> 32));
            Jcc_auto(CC_G,2);
            Jcc_auto(CC_L,1);
        }

        CMP_RegWithImm(1, xRT->x86reg, (_u32)ConstInt);
        Jcc_auto(CC_AE, 3);

        if (!Is32bit)
        {
            SetTarget(1);
        }
        WC16(0x016A); // push 1
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
            CMP_RegWithImm(1, xRS->HiWordLoc, (_u32)(ConstInt >> 32));
            Jcc_auto(CC_G,2);
            Jcc_auto(CC_L,1);
        }

        CMP_RegWithImm(1, xRS->x86reg, (_u32)ConstInt);
        Jcc_auto(CC_AE, 4);

        if (!Is32bit)
        {
            SetTarget(1);
        }
        WC16(0x016A); // push 1
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
        WC16(0x016A); // push 1
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
        WC16(0x016A); // push 1
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

#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	ANDI r%02i = r%02i + 0x%08X\n", reg->pc, __RT, __RS, __I);
#endif LOG_DYNA

    if (xRT->mips_reg == 0) return;
#ifdef SAFE_IMM
     INTERPRET(r4300i_andi); return;
#endif

    if (xRS->mips_reg == xRT->mips_reg)
    {
        xRT->IsDirty = 1;
        if (((_u32)(_u16)__I) == 0)
            xRT->NoNeedToLoadTheLo = 1;
        MapRT;
    }
    else
    {
        xRT->IsDirty = 1;
        xRT->NoNeedToLoadTheLo = 1;
        MapRT;

        MapRS;

        MOV_Reg2ToReg1(1, xRT->x86reg, xRS->x86reg); /* mov rt, rs (lo) */
    }

    if (((_u32)(_u16)__I) != 0)
        AND_ImmToReg(1, xRT->x86reg, (_u32)(_u16)__I);
    else
    {
        int where = CheckWhereIsMipsReg(xRT->mips_reg);

        ConstMap[xRT->mips_reg].IsMapped = 1;
        ConstMap[xRT->mips_reg].value    = 0;

        if (where > -1) 
        {
            x86reg[where].IsDirty = 0;
            FlushRegister(where);
        }
    }
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_ori(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_ori)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	ORI r%02i = r%02i | 0x%08X\n", reg.pc, __RT, __RS, (_u64)(_u16)__I);
#endif LOG_DYNA

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
                ConstMap[xRT->mips_reg].value |= (_u32)(_u16)__I;
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

void dyna4300i_xori(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS);

     _SAFTY_CPU_(r4300i_xori)
#ifdef LOG_DYNA
        LogDyna(" 0x%08X:	XORI r%02i = r%02i + 0x%08X\n", reg->pc, __RT, __RS, __I);
#endif LOG_DYNA

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
            XOR_ImmToReg(1, xRS->x86reg, (_u32)(_u16)__I);
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
        Reg2ToReg1(1, 0x8B, xRT->x86reg, xRS->x86reg);        /* mov rt, rs (lo) */
        if (((_u32)(_u16)__I) != 0)
            XOR_ImmToReg(1, xRT->x86reg, (_u32)(_u16)__I);
        MOV_Reg2ToReg1(1, xRT->HiWordLoc, xRS->HiWordLoc);
    }
}

////////////////////////////////////////////////////////////////////////

extern x86regtyp x86reg[];
void dyna4300i_lui(OP_PARAMS)
{
    _u8 rt;
    int wherex86;

_SAFTY_CPU_(r4300i_lui)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	LUI r%02i = 0x%08X\n", reg->pc, __RT, (_u32)(__I << 16));
#endif LOG_DYNA
#ifdef SAFE_IMM
    INTERPRET(r4300i_lui); return;
#endif SAFE_IMM

    rt = __RT;
    if (rt != 0)
    {
        ConstMap[rt].value = (_s32)((__I) << 16);
        ConstMap[rt].IsMapped = 1;

        wherex86 = CheckWhereIsMipsReg(rt);
        
        if (wherex86 > -1)
        {
            x86reg[wherex86].IsDirty = 0;
            FlushRegister(wherex86);
        }
    }
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0(OP_PARAMS)
{
    dyna_cop0_rs_instruction[__RS](PASS_PARAMS);
}

////////////////////////////////////////////////////////////////////////
// Cop1 stuff
////////////////////////////////////////////////////////////////////////

extern void (* dyna4300i_cop1_Instruction[])(OP_PARAMS);
void dyna4300i_cop1(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	COP1\n", reg->pc);
#endif LOG_DYNA

    dyna4300i_cop1_Instruction[__RS](PASS_PARAMS);

// COP1 exception ....
// This stuff is a little bit tricky but i hope it will work =)
// The COPROCESSER_UNUSABLE exception is used in OS for setting 
// a special flag in the Thread Info. Only if the Flag is set the 
// COP1 context (the registers) are stored. 
// So I think it is enough to compile the exception-checking
// for the first COP1 calls in a thread only ... lets pray =)
// same at lwc1, ldc1, swc1, sdc1

    if ((((_u32)reg->COP0Reg[STATUS] & STATUS_CU1) == 0) 
        && (COP1_exception_speedup == TRUE))
    {
//        COP1_exception_speedup = FALSE;
//        HELP_Call((unsigned long)HELP_Cop1);
//        HELP_CheckExceptionSlot(reg->pc);
    }
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop2(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	COP2\n", reg->pc);
#endif LOG_DYNA

    dyna_cop2_rs_instruction[__RS](PASS_PARAMS);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_daddi(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_daddi)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DADDI\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_MATH
    INTERPRET(r4300i_daddi); return;
#endif SAFE_MATH

    INTERPRET(r4300i_daddi);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_daddiu(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_daddiu)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DADDIU\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_MATH
    INTERPRET(r4300i_daddiu); return;
#endif SAFE_MATH
    
    INTERPRET(r4300i_daddiu);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_ldl(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	LDL\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_LOADSTORE
     INTERPRET_LOADSTORE(r4300i_ldl); return;
#endif
    
    _SAFTY_CPU_(r4300i_ldl)

    INTERPRET_LOADSTORE(r4300i_ldl);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_ldr(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	LDR\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_LOADSTORE
     INTERPRET_LOADSTORE(r4300i_ldr); return;
#endif

    _SAFTY_CPU_(r4300i_ldr)
    
    INTERPRET_LOADSTORE(r4300i_ldr);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_lb(OP_PARAMS)
{
    _s32 VAddr;
    _s8* pointer;
    SetRdRsRt32bit(PASS_PARAMS);

#ifdef SAFE_LOADSTORE
     INTERPRET_LOADSTORE(r4300i_lb); return;
#endif

    _SAFTY_CPU_(r4300i_lb);

    VAddr = (ConstMap[xRS->mips_reg].value + (_s32)(_s16)__I) ^ 3;
    pointer = (_s8*)(GetStaticMemAddr(VAddr));

    if ((ConstMap[xRS->mips_reg].IsMapped == 1) && (pointer != NULL))
    {
        if (xRT->mips_reg != 0)
        {
            if (ConstMap[xRT->mips_reg].IsMapped == 1) ConstMap[xRT->mips_reg].IsMapped = 0;
            xRT->IsDirty = 1;
            xRT->NoNeedToLoadTheLo = 1;
            MapRT;
            MOVSX_MemoryToReg(0, xRT->x86reg, ModRM_disp32, (_u32)pointer);
        }
    }
    else
    {
       INTERPRET_LOADSTORE(r4300i_lb)  //Interpreting is faster due to smaller code
    }
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_lh(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	LH r%02i = [r%02i + 0x%08X]\n", reg->pc, __RT, __RS, __I);
#endif LOG_DYNA
#ifdef SAFE_LOADSTORE
     INTERPRET_LOADSTORE(r4300i_lh); return;
#endif

    INTERPRET_LOADSTORE(r4300i_lh);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_lwl(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	LWL\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_lwl); return;
#endif SAFE_LOADSTORE

    INTERPRET_LOADSTORE(r4300i_lwl);
}

////////////////////////////////////////////////////////////////////////

extern char * ((**phys_read_fast)(_u32 addr));
extern char * ((**phys_write_fast)(_u32 addr));

extern x86regtyp x86reg[8];
void dyna4300i_lw(OP_PARAMS)
{
    _s32 VAddr;
    _s32* pointer;
    SetRdRsRt32bit(PASS_PARAMS);

#ifdef SAFE_LOADSTORE
     INTERPRET_LOADSTORE(r4300i_lw); return;
#endif

    _SAFTY_CPU_(r4300i_lw);

  //  FlushAllRegisters(); //FIXME!!! (If broken, 3D MK4 characters will be black while playing (not lit))
    VAddr = ConstMap[xRS->mips_reg].value + (_s32)(_s16)__I;
    pointer = (_s32*)(GetStaticMemAddr(VAddr));
    if ((ConstMap[xRS->mips_reg].IsMapped == 1) && (pointer != NULL))
    {
        if (xRT->mips_reg != 0)
        {
            xRT->IsDirty = 1;
            xRT->NoNeedToLoadTheLo = 1;
            MapRT;
            MOV_MemoryToReg(1, xRT->x86reg, ModRM_disp32, (_u32)pointer);
        }
    }
    else
    {
        INTERPRET_LOADSTORE(r4300i_lw)
    }
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_lbu(OP_PARAMS)
{
    _s32 VAddr;
    _u8* pointer;
    SetRdRsRt32bit(PASS_PARAMS);

#ifdef SAFE_LOADSTORE
     INTERPRET_LOADSTORE(r4300i_lbu); return;
#endif

    _SAFTY_CPU_(r4300i_lbu);

    VAddr = (ConstMap[xRS->mips_reg].value + (_s32)(_s16)__I) ^ 3;
    pointer = (_u8*)(GetStaticMemAddr(VAddr));

    if ((ConstMap[xRS->mips_reg].IsMapped == 1) && (pointer != NULL))
    {
        if (xRT->mips_reg != 0)
        {
            if (ConstMap[xRT->mips_reg].IsMapped == 1) ConstMap[xRT->mips_reg].IsMapped = 0;
            xRT->IsDirty = 1;
            xRT->NoNeedToLoadTheLo = 1;
            MapRT;
            MOVZX_MemoryToReg(0, xRT->x86reg, ModRM_disp32, (_u32)pointer);
        }
    }
    else
    {
        INTERPRET_LOADSTORE(r4300i_lbu)  //Interpreting is faster due to smaller code
    }
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_lhu(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	LHU\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_lhu); return;
#endif

    INTERPRET_LOADSTORE(r4300i_lhu);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_lwr(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	LWR\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_lwr); return;
#endif

    INTERPRET_LOADSTORE(r4300i_lwr);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_lwu(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	LWU\n", reg->pc);
#endif
#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_lwr); return;
#endif


    INTERPRET_LOADSTORE(r4300i_lwu);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_sb(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SB\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_sb); return;
#endif SAFE_LOADSTORE

    INTERPRET_LOADSTORE(r4300i_sb);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_sh(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SH\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_sh); return;
#endif SAFE_LOADSTORE

    INTERPRET_LOADSTORE(r4300i_sh);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_swl(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SWL\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_swl); return;
#endif SAFE_LOADSTORE

    INTERPRET_LOADSTORE(r4300i_swl);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_sw(OP_PARAMS)
{
    _s32 VAddr;
    _s32* pointer;
    SetRdRsRt32bit(PASS_PARAMS);

#ifdef SAFE_LOADSTORE
     INTERPRET_LOADSTORE(r4300i_sw); return;
#endif

    _SAFTY_CPU_(r4300i_sw);

    VAddr = ConstMap[xRS->mips_reg].value + (_s32)(_s16)__I;
    pointer = (_s32*)(GetStaticMemAddr(VAddr));
    if ((ConstMap[xRS->mips_reg].IsMapped == 1) && (pointer != NULL))
    {
        if (xRT->mips_reg != 0)
        {
            if (ConstMap[xRT->mips_reg].IsMapped == 0)
            {
                MapRT;
                MOV_RegToMemory(1, xRT->x86reg, ModRM_disp32, (_u32)pointer);
            }
            else
                MOV_ImmToMemory(1, (_u32)pointer, ConstMap[xRT->mips_reg].value);
        }
        else
        {
            XOR_Reg1ToReg2(1, Reg_ECX, Reg_ECX);
            MOV_RegToMemory(1, Reg_ECX, ModRM_disp32, (_u32)pointer);
        }

    }
    else
    {
       INTERPRET_LOADSTORE(r4300i_sw)  //Interpreting is faster due to smaller code
    }
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_sdl(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SDL\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_ldl); return;
#endif

    INTERPRET_LOADSTORE(r4300i_sdl);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_sdr(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SDR\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_sdr); return;
#endif

    INTERPRET_LOADSTORE(r4300i_sdr);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_swr(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SWR\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_swr); return;
#endif

    INTERPRET_LOADSTORE(r4300i_swr);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cache(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	CACHE - Testing Self-Mod code detection \n", reg->pc);
#endif LOG_DYNA
    
    INTERPRET(r4300i_cache);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_ll(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	LW r%02i = [r%02i + 0x%08X]\n", reg->pc, __RT, __RS, __I);
#endif LOG_DYNA
#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_ll); return;
#endif

    INTERPRET_LOADSTORE(r4300i_ll) 
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_lwc2(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	LWC2\n", reg->pc);
#endif LOG_DYNA
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
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	LLD\n", reg->pc);
#endif LOG_DYNA

    SetRdRsRt32bit(PASS_PARAMS);

#ifdef SAFE_LOADSTORE
    INTERPRET_LOADSTORE(r4300i_lld); return;
#endif

    INTERPRET_LOADSTORE(r4300i_lld); 
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_ldc2(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	LDC2\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_LOADSTORE
    DisplayError("Unhandled ldc2"); return;
    //INTERPRET_LOADSTORE(r4300i_ldc2); return;
#endif

    DisplayError("Unhandled ldc2");
//    INTERPRET_LOADSTORE(r4300i_ldc2);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_ld(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	LD\n", reg->pc);
#endif LOG_DYNA

    SetRdRsRt64bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_ld);

#ifdef SAFE_LOADSTORE
     INTERPRET_LOADSTORE(r4300i_ld); return;
#endif

    INTERPRET_LOADSTORE(r4300i_ld)  //Interpreting is faster due to smaller code
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_sc(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SC\n", reg->pc);
#endif LOG_DYNA

    INTERPRET_LOADSTORE(r4300i_sc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_swc2(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SWC2\n", reg->pc);
#endif LOG_DYNA
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
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SCD\n", reg->pc);
#endif LOG_DYNA

    INTERPRET_LOADSTORE(r4300i_scd);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_sdc2(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SDC2\n", reg->pc);
#endif LOG_DYNA

//    INTERPRET_LOADSTORE(r4300i_sdc2);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_sd(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SD\n", reg->pc);
#endif LOG_DYNA

    INTERPRET_LOADSTORE(r4300i_sd);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_sll(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

_SAFTY_CPU_(r4300i_sll)

#ifdef SAFE_SHIFTS
    INTERPRET(r4300i_sll); return;
#endif

// catching NOP 
    if (reg->code == 0x00000000)
    {
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	NOP\n", reg->pc);
#endif LOG_DYNA 
        return;
    }

// the real instruction
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SLL\n", reg->pc);
#endif LOG_DYNA

    if (xRD->mips_reg == 0) return;
    rcShift(0xE0, xRD->mips_reg, xRT->mips_reg);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_srl(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

_SAFTY_CPU_(r4300i_srl)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SRL\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_SHIFTS
    INTERPRET(r4300i_srl); return;
#endif

    if (xRD->mips_reg == 0) return;
    rcShift(0xE8, xRD->mips_reg, xRT->mips_reg);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_sra(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

_SAFTY_CPU_(r4300i_sra)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SRA\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_SHIFTS
    INTERPRET(r4300i_sra); return;
#endif

    if (xRD->mips_reg == 0) return;
    rcShift(0xF8, xRD->mips_reg, xRT->mips_reg);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_sllv(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_sllv)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SLLV\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_SHIFTS
    INTERPRET(r4300i_sllv); return;
#endif

    if (xRD->mips_reg == 0) return;
        rcShift3reg(SHL_RegByCL, xRD->mips_reg, xRS->mips_reg, xRT->mips_reg);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_srlv(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_srlv)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SRLV\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_SHIFTS
    INTERPRET(r4300i_srlv); return;
#endif

    if (xRD->mips_reg == 0) return;
        rcShift3reg(SHR_RegByCL, xRD->mips_reg, xRS->mips_reg, xRT->mips_reg);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_srav(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_srav)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SRAV\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_SHIFTS
    INTERPRET(r4300i_srav); return;
#endif

    if (xRD->mips_reg == 0) return;
        rcShift3reg(SAR_RegByCL, xRD->mips_reg, xRS->mips_reg, xRT->mips_reg);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_syscall(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SYSCALL\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_syscall);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_break(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	BREAK\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_break);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_sync(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SYNC - IGNORED\n", reg->pc);
#endif LOG_DYNA
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_mtlo(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_mtlo)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	MTLO\n", reg->pc);
#endif LOG_DYNA

// 64bit
    //note: RD is a misnomer. It really corresponds to LO, but it works the same way in our Mapping macros
    //as long as you assign xRD->mips_reg = __LO;

    xRD->IsDirty = 1;
    xRD->NoNeedToLoadTheLo = 1;
    xRD->NoNeedToLoadTheHi = 1;
    xRD->mips_reg = __LO;
    MapRD;
    MapRS;

    MOV_Reg2ToReg1(1, xRD->x86reg   , xRS->x86reg);
    MOV_Reg2ToReg1(1, xRD->HiWordLoc, xRS->HiWordLoc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_mthi(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS);

_SAFTY_CPU_(r4300i_mthi)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	MTHI\n", reg->pc);
#endif LOG_DYNA

    //note: RD is a misnomer. It really corresponds to HI, but it works the same way in our Mapping macros
    //as long as you assign xRD->mips_reg = __HI;

    xRD->IsDirty = 1;
    xRD->NoNeedToLoadTheLo = 1;
    xRD->NoNeedToLoadTheHi = 1;
    xRD->mips_reg = __HI;
    MapRD;
    MapRS;

    MOV_Reg2ToReg1(1, xRD->x86reg   , xRS->x86reg);
    MOV_Reg2ToReg1(1, xRD->HiWordLoc, xRS->HiWordLoc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_mflo(OP_PARAMS)
{

#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	MFLO\n", reg->pc);
#endif LOG_DYNA

// 64bit
    SetRdRsRt64bit(PASS_PARAMS);

_SAFTY_CPU_(r4300i_mflo)
    if (xRD->mips_reg == 0) return;

    //note: RS is a misnomer. It really corresponds to LO, but it works the same way in our Mapping macros
    //as long as you assign xRS->mips_reg = __LO;
    xRD->IsDirty = 1;
    xRD->NoNeedToLoadTheLo = 1;
    xRD->NoNeedToLoadTheHi = 1;
    xRS->mips_reg = __LO;
    MapRD;
    MapRS;

    MOV_Reg2ToReg1(1, xRD->x86reg   , xRS->x86reg);
    MOV_Reg2ToReg1(1, xRD->HiWordLoc, xRS->HiWordLoc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_mfhi(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_mfhi)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	MFHI\n", reg->pc);
#endif LOG_DYNA

// 64bit
    //note: RS is a misnomer. It really corresponds to HI, but it works the same way in our Mapping macros
    //as long as you assign xRS->mips_reg = __HI;

    if (xRD->mips_reg == 0) return;

    xRD->IsDirty = 1;
    xRD->NoNeedToLoadTheLo = 1;
    xRD->NoNeedToLoadTheHi = 1;
    xRS->mips_reg = __HI;
    MapRD;
    MapRS;

    MOV_Reg2ToReg1(1, xRD->x86reg   , xRS->x86reg);
    MOV_Reg2ToReg1(1, xRD->HiWordLoc, xRS->HiWordLoc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsllv(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_dsllv)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DSLLV\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_dsllv);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsrlv(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_dsrlv)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DSRLV\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_dsrlv);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsrav(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_dsrav)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DSRAV\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_dsrav);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_mult(OP_PARAMS)
{   
    _SAFTY_CPU_(r4300i_mult)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	MULT\n", reg->pc);
#endif LOG_DYNA

    FlushAllRegisters();
    LoadLowMipsCpuRegister(__RS,Reg_EAX);
    IMUL_EAXWithMemory(1,(unsigned long)&reg->GPR[__RT]);

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
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_multu(OP_PARAMS)
{
    _SAFTY_CPU_(r4300i_multu)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	MULTU\n", reg->pc);
#endif LOG_DYNA

    FlushAllRegisters();
    LoadLowMipsCpuRegister(__RS,Reg_EAX);
    MUL_EAXWithMemory(1,(unsigned long)&reg->GPR[__RT]);

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

}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_div(OP_PARAMS)
{
    _SAFTY_CPU_(r4300i_div)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DIV\n", reg->pc);
#endif LOG_DYNA

    FlushAllRegisters();
    LoadLowMipsCpuRegister(__RT,Reg_ECX);
    WC8(0x85);WC8(0xc9);                            //test   ecx, ecx
    Jcc_auto(CC_E,0);
        LoadLowMipsCpuRegister(__RS,Reg_EAX);
        MOV_Reg2ToReg1(1, Reg_EDX, Reg_EAX);
        SAR_RegByImm(1, Reg_EDX, 0x1F);

        IDIV_EAXWithReg(1, Reg_ECX);

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

#ifdef SAVEOPCOUNTER
	cp0Counter += PCLOCKDIV*2;
#endif

}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_divu(OP_PARAMS)
{
    _SAFTY_CPU_(r4300i_divu)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DIVU\n", reg->pc);
#endif LOG_DYNA

    FlushAllRegisters();
    LoadLowMipsCpuRegister(__RT,Reg_ECX);
    WC8(0x85);WC8(0xc9);                            //test   ecx, ecx
    Jcc_auto(CC_E,0);
        LoadLowMipsCpuRegister(__RS,Reg_EAX);
        LoadLowMipsCpuRegister(__RT,Reg_ECX);
        XOR_Reg1ToReg2(1, Reg_EDX, Reg_EDX);
        DIV_EAXWithReg(1, Reg_ECX);

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

#ifdef SAVEOPCOUNTER
	cp0Counter += PCLOCKDIVU*2;
#endif
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dmult(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_dmult)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DMULT\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_dmult);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dmultu(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_dmultu)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DMULTU\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_dmultu);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_ddiv(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_ddiv)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DDIV\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_ddiv);

#ifdef SAVEOPCOUNTER
	cp0Counter += PCLOCKDDIV*2;
	gHardwareState.COP0Reg[COUNT] -=  PCLOCKDDIV;	
#endif
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_ddivu(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_ddivu)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DDIVU\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_ddivu);

#ifdef SAVEOPCOUNTER
	cp0Counter += PCLOCKDDIVU*2;
	gHardwareState.COP0Reg[COUNT] -=  PCLOCKDDIVU;	
#endif
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_add(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS); 

_SAFTY_CPU_(r4300i_add)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	ADD r%02i = r%02i + r%02i\n", reg->pc, __RD, __RS, __RT);
#endif LOG_DYNA

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
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	ADDU r%02i = r%02i + r%02i\n", reg->pc, __RD, __RS, __RT);
#endif LOG_DYNA

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
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SUB\n", reg->pc);
#endif LOG_DYNA

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
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	SUBU\n", reg->pc);
#endif LOG_DYNA

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
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	AND r%02i = r%02i & r%02i\n", reg->pc, __RD, __RS, __RT);
#endif LOG_DYNA

#define _Op 0x23
#ifdef SAFE_GATES
     INTERPRET(r4300i_and); return;
#endif

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
                Reg2ToReg1(1, _Op , xRD->x86reg,    xRS->x86reg);       /* op    rd,rs (lo) */
                Reg2ToReg1(1, _Op , xRD->HiWordLoc, xRS->HiWordLoc);    /* op    rd,rs (hi) */
            }
        }
        else
        {
            if (xRD->mips_reg==xRS->mips_reg)
            {
                xRD->IsDirty = 1;
                MapRD;
                MapRT;

                Reg2ToReg1(1, _Op , xRD->x86reg,    xRT->x86reg);       /* op    rd,rt (lo) */
                Reg2ToReg1(1, _Op , xRD->HiWordLoc, xRT->HiWordLoc);    /* op    rd,rt (hi) */
            }
            else
            {
                xRD->NoNeedToLoadTheHi = 1;
                xRD->NoNeedToLoadTheLo = 1;
                xRD->IsDirty = 1;
                MapRD;
                MapRS;
                MapRT;

                Reg2ToReg1(1, 0x8B, xRD->x86reg,    xRS->x86reg);     /* mov      rd,rs (lo) */
                Reg2ToReg1(1, _Op , xRD->x86reg,    xRT->x86reg);     /* op       rd,rt (lo) */
                Reg2ToReg1(1, 0x8B, xRD->HiWordLoc, xRS->HiWordLoc);  /* mov      rd,rs (hi) */
                Reg2ToReg1(1, _Op , xRD->HiWordLoc, xRT->HiWordLoc);  /* op       rd,rt (hi) */
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_or(OP_PARAMS)
{
    static int temp = 0;
    SetRdRsRt64bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_or)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	OR\n", reg->pc);
#endif LOG_DYNA
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
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	XOR\n", reg->pc);
#endif LOG_DYNA
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
            ConstMap[xRD->mips_reg].IsMapped = 0;
            xRD->IsDirty = 1;
            xRD->NoNeedToLoadTheLo = 1;
            xRD->NoNeedToLoadTheHi = 1;
            MapRD;

            XOR_Reg1ToReg2(1, xRD->x86reg,    xRD->x86reg);         /* xor    rd,rd  (lo) */
            XOR_Reg1ToReg2(1, xRD->HiWordLoc, xRD->HiWordLoc);      /* xor    rd,rd  (hi) */
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
    
    _SAFTY_CPU_(r4300i_nor)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	NOR\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_GATES
     INTERPRET(r4300i_nor); return;
#endif

    if (xRD->mips_reg != 0)
    {
        if (xRD->mips_reg==xRT->mips_reg)
        {
            if (xRS->mips_reg==xRT->mips_reg)
            {
                xRD->IsDirty = 1;
                MapRD;

                 OR_ShortToReg(1, Reg_ECX,        0xFF);
                XOR_Reg2ToReg1(1, xRD->x86reg,    Reg_ECX);
                XOR_Reg2ToReg1(1, xRD->HiWordLoc, Reg_ECX);
            }
            else
            {
                xRD->IsDirty = 1;
                MapRD;
                MapRS;

                OR_ShortToReg(1, Reg_ECX, 0xFF);
                Reg2ToReg1(1, _Or , xRD->x86reg,    xRS->x86reg);       /* or    rd,rs (lo) */
                XOR_Reg2ToReg1(1, xRD->x86reg, Reg_ECX);
                Reg2ToReg1(1, _Or , xRD->HiWordLoc, xRS->HiWordLoc);    /* or    rd,rs (hi) */
                XOR_Reg2ToReg1(1, xRD->HiWordLoc, Reg_ECX);
            }
        }
        else
        {
            if (xRD->mips_reg==xRS->mips_reg)
            {
                xRD->IsDirty = 1;
                MapRD;
                MapRT;

                OR_ShortToReg(1, Reg_ECX, 0xFF);
                Reg2ToReg1(1, _Or , xRD->x86reg,    xRT->x86reg);       /* or    rd,rt (lo) */
                XOR_Reg2ToReg1(1, xRD->x86reg, Reg_ECX);
                Reg2ToReg1(1, _Or , xRD->HiWordLoc, xRT->HiWordLoc);    /* or    rd,rt (hi) */
                XOR_Reg2ToReg1(1, xRD->HiWordLoc, Reg_ECX);
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

                     OR_ShortToReg(1, Reg_ECX,        0xFF);
                    MOV_Reg2ToReg1(1, xRD->x86reg,    xRS->x86reg);                     /* mov      rd,rs (lo) */
                        Reg2ToReg1(1, _Or ,           xRD->x86reg,    xRT->x86reg);     /* or       rd,rt (lo) */
                    XOR_Reg2ToReg1(1, xRD->x86reg,    Reg_ECX);
                    MOV_Reg2ToReg1(1, xRD->HiWordLoc, xRS->HiWordLoc);                  /* mov      rd,rs (hi) */
                        Reg2ToReg1(1, _Or,            xRD->HiWordLoc, xRT->HiWordLoc);  /* or       rd,rt (hi) */
                    XOR_Reg2ToReg1(1, xRD->HiWordLoc, Reg_ECX);
                }
                else /* rd!=rt, rd!=rs, rs=rt */
                {
                     OR_ShortToReg(1, Reg_ECX,        0xFF);
                    MOV_Reg2ToReg1(1, xRD->x86reg,    xRS->x86reg);     /* mov      rd,rs (lo) */
                    XOR_Reg2ToReg1(1, xRD->x86reg,    Reg_ECX);
                    MOV_Reg2ToReg1(1, xRD->HiWordLoc, xRS->HiWordLoc);  /* mov      rd,rs (hi) */
                    XOR_Reg2ToReg1(1, xRD->HiWordLoc, Reg_ECX);
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dadd(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_dadd)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DADD\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_dadd);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_daddu(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_daddu)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DADDU\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_daddu);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsub(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_dsub)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DSUB\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_dsub);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsubu(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_dsubu)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DSUBU\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_dsubu);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_tge(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	TGE\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_tge);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_tgeu(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	TGEU\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_tgeu);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_tlt(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	TLT\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_tlt);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_tltu(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	TLTU\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_tltu);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_teq(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	TEQ\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_teq);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_tne(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	TNE\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_tne);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsll(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_dsll)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DSLL\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_dsll);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsrl(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_dsrl)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DSRL\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_dsrl);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsra(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_dsra)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DSRA\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_dsra);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsll32(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_dsll32)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DSLL32\n", reg->pc);
#endif LOG_DYNA

    if (xRD->mips_reg == 0) return;
    if (xRT->mips_reg == xRD->mips_reg)
    {
        xRD->NoNeedToLoadTheHi = 1;
        xRD->IsDirty=1;
        MapRD;
        MOV_Reg2ToReg1(1, xRD->HiWordLoc, xRD->x86reg);
          SHL_RegByImm(1, xRD->HiWordLoc, (_u8)__SA);
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
          SHL_RegByImm(1, xRD->HiWordLoc, (_u8)__SA);
        XOR_Reg1ToReg2(1, xRD->x86reg,    xRD->x86reg);
    }
}

////////////////////////////////////////////////////////////////////////
extern void StoreMipsCpuRegister(unsigned long iMipsReg,unsigned char iIntelReg1,unsigned char iIntelReg2);

void dyna4300i_special_dsrl32(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_dsrl32)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DSRL32\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_SHIFTS
    INTERPRET(r4300i_dsrl32);
#endif

    if (xRD->mips_reg == 0) return;
    INTERPRET(r4300i_dsrl32); return;

    FlushAllRegisters();    
    LoadHighMipsCpuRegister(__RT,Reg_EDX);
    SHR_RegByImm(1, Reg_EDX, (_u8)__SA);
    XOR_Reg1ToReg2(1,Reg_EAX,Reg_EAX);
    StoreMipsCpuRegister(__RD,Reg_EDX,Reg_EAX);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_special_dsra32(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

    _SAFTY_CPU_(r4300i_dsra32)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DSRA32\n", reg->pc);
#endif LOG_DYNA

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
    FlushAllRegisters(); 

    WC8(0xA1);    WC32((_u32)&reg->GPR[xRT->mips_reg]+4);
    WC8(0xA3);    WC32((_u32)&reg->GPR[xRD->mips_reg]);
    WC8(0xA1);    WC32((_u32)&reg->GPR[xRT->mips_reg]);
    WC16(0xF8C1);  WC8(__SA);
    
    xRD->IsDirty = 1;
    xRD->NoNeedToLoadTheLo = 1;
    MapRD;
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_tgei(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	TGEI\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_tgei);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_tgeiu(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	TGEIU\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_tgeiu);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_tlti(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	TLTI\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_tlti);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_tltiu(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	TLTIU\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_tltiu);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_teqi(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	TEQI\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_teqi);
    HELP_CheckExceptionSlot(reg->pc);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_tnei(OP_PARAMS)
{
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	TNEI\n", reg->pc);
#endif LOG_DYNA

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
	/*
   SetRdRsRt32bit(PASS_PARAMS);

_SAFTY_CPU_(r4300i_cop0_rs_mf)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	MF\n", reg->pc);
#endif LOG_DYNA

if (xRT->mips_reg == 0)  return;

    xRT->NoNeedToLoadTheLo = 1;
    xRT->IsDirty = 1;
    MapRT;
    MOV_MemoryToReg(1, xRT->x86reg,ModRM_disp32, (unsigned long)&(_u32)reg->COP0Reg[__RD]);
	*/
	INTERPRET(r4300i_COP0_mfc0);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_rs_dmf(OP_PARAMS)
{
    _u32 rd = (unsigned long)&(_u32)reg->COP0Reg[__RD];
    SetRdRsRt32bit(PASS_PARAMS);

_SAFTY_CPU_(r4300i_cop0_rs_dmf)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DMF\n", reg->pc);
#endif LOG_DYNA

    if (xRT->mips_reg == 0)  return;

    xRT->NoNeedToLoadTheLo = 1;
    xRT->IsDirty = 1;
    MapRT;
    MOV_MemoryToReg(1, xRT->x86reg, ModRM_disp32, rd);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_rs_cf(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);
    
    _SAFTY_CPU_(r4300i_cop0_rs_cf)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	CF\n", reg->pc);
#endif LOG_DYNA

    if (xRT->mips_reg == 0)  return;

    
    xRT->IsDirty = 1;
    xRT->NoNeedToLoadTheLo = 1;
    MapRT;
    MOV_MemoryToReg(1, xRT->x86reg, ModRM_disp32, (unsigned long )&reg->COP0Con[__FS]);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_rs_mt(OP_PARAMS)
{
    INTERPRET(r4300i_COP0_mtc0);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_rs_dmt(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_cop0_rs_dmt)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	DMT\n", reg->pc);
#endif LOG_DYNA

    DisplayError("Unhandled dmtc0");
//    INTERPRET(r4300i_COP0_dmtc0);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_rs_ct(OP_PARAMS)
{
    _u32 fs = (unsigned long)&reg->COP0Con[__FS];
    SetRdRsRt64bit(PASS_PARAMS);

_SAFTY_CPU_(r4300i_cop0_rs_ct)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	CT\n", reg->pc);
#endif LOG_DYNA

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

void dyna4300i_cop0_rs_co(OP_PARAMS)
{
    dyna_cop0_instruction[__F](PASS_PARAMS);   
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_rt_bcf(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_cop0_rt_bcf)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	BCF\n", reg->pc);
#endif LOG_DYNA

    DisplayError("Unhandled BCFC0");
//    INTERPRET(r4300i_cop0_rt_bcf);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_rt_bct(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_cop0_rt_bct)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	BCT\n", reg->pc);
#endif LOG_DYNA

    DisplayError("Unhandled BCTC0");
//    INTERPRET(r4300i_cop0_rt_bct);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_rt_bcfl(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_cop0_rt_bcfl)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	BCFL\n", reg->pc);
#endif LOG_DYNA

    DisplayError("Unhandled BCFL0");
//    INTERPRET(r4300i_cop0_rt_bcfl);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_rt_bctl(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_cop0_rt_bctl)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	BCTL\n", reg->pc);
#endif LOG_DYNA

    DisplayError("Unhandled BCTL0");
//    INTERPRET(r4300i_cop0_rt_bctl);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_tlbr(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_cop0_tlbr)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	TLBR\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_COP0_tlbr);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_tlbwi(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_cop0_tlbwi)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	TLBWI\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_COP0_tlbwi);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_tlbwr(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_cop0_tlbwr)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	TLBWR\n", reg->pc);
#endif LOG_DYNA

    INTERPRET(r4300i_COP0_tlbwr);
}

////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_tlbp(OP_PARAMS)
{
_SAFTY_CPU_(r4300i_cop0_tlbp)
#ifdef LOG_DYNA
    LogDyna(" 0x%08X:	TLBP\n", reg->pc);
#endif LOG_DYNA

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
    DisplayError("There isnt a COP2\n");
}