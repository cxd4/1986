//#define SAFE_BRANCHES 1
//#define DANGEROUS_BRANCHES 1
//////////////////////////////////////////////////////////////////////////////////
// small and dirty helper, hacks and cheats =)				                    //
//////////////////////////////////////////////////////////////////////////////////
//#define SPEED_HACK
#include "..\options.h"

#define SPEED_HACK                                                                      \
    if ( __I == -1 )	\
	{	\
		if( (pcptr[1] == 0))                                                \
		{                                                                                   \
			/*MOV_ImmToMemory(1, (_u32)&gHardwareState.COP0Reg[COUNT], MAGICNUMBER);*/      \
			PUSHAD();                                                                       \
			X86_CALL((_u32)DoSpeedHack);                                                    \
			POPAD();                                                                        \
		}																					\
		else	\
			;/*DisplayError("Missing SpeedHack");*/	\
	}	


//#define J_SPEED_HACK    

#define J_SPEED_HACK                                                                            \
                                                                                                \
if ( (reg->pc == aValue) && (pcptr[1] == 0))                                                    \
    {                                                                                           \
		/*MOV_ImmToMemory(1, (_u32)&gHardwareState.COP0Reg[COUNT], MAGICNUMBER);*/              \
		PUSHAD();                                                                               \
        X86_CALL((_u32)DoSpeedHack);                                                            \
        POPAD();                                                                                \
    }

extern MapConstant ConstMap[32];
extern x86regtyp   x86reg[8];
MapConstant TempConstMap[32];
x86regtyp   Tempx86reg[8];
extern _u32 cp0Counter;

static int rerere = 0;
void Compile_Slot(_u32 pc)
{
	_u8 op;

#ifdef LOG_DYNA
	LogDyna("** Compile Delay Slot\n", pc);
#endif LOG_DYNA

#ifdef _USE_DEBUGGER_
	HELP_Call((unsigned long)HELP_debug);
#endif _USE_DEBUGGER_

	gHardwareState.code = pcptr[1];
//    if (rerere++ > 40)
  //      DisplayError("%08X: gHardwareState.code = %08X", gHardwareState.pc, gHardwareState.code);
	op = (_u8)(gHardwareState.code >> 26);

    memcpy(TempConstMap, ConstMap, sizeof(MapConstant)<<5);
    memcpy(Tempx86reg, x86reg, sizeof(x86regtyp)<<3);
    dyna_instruction[op](&gHardwareState);
//    if (dyna_branch == TRUE) DisplayError("Branch in delay slot!?");
    FlushAllRegisters();
    memcpy(ConstMap, TempConstMap, sizeof(MapConstant)<<5);
    memcpy(x86reg, Tempx86reg, sizeof(x86regtyp)<<3);
}


void Compile_Slot_Jump(_u32 pc)
{
	_u8 op;

#ifdef LOG_DYNA
	LogDyna("** Compile Delay Slot\n", pc);
#endif LOG_DYNA

#ifdef _USE_DEBUGGER_
	HELP_Call((unsigned long)HELP_debug);
#endif _USE_DEBUGGER_

	gHardwareState.code = pcptr[1];
	op = (_u8)(gHardwareState.code >> 26);

    dyna_instruction[op](&gHardwareState);
    FlushAllRegisters();
}




extern void CheckTheInterrupts(_u32 count);
extern uint32 VIcounter;
void Interrupts()
{
        WC8(0xB9);
        WC32(counter);
#ifdef SAVECPUCOUNTER
		// CPU counter should increase at the half speed of PCLOCK
		//ADD_ImmToMemory((_u32)&gHardwareState.COP0Reg[COUNT], (cp0Counter+1)/2 );
		//ADD_ImmToMemory((_u32)&gHardwareState.COP0Reg[COUNT], cp0Counter/2+1 );
		ADD_ImmToMemory((_u32)&gHardwareState.COP0Reg[COUNT], (cp0Counter&0xFF)/2+1 );
#else
        ADD_ImmToMemory((_u32)&gHardwareState.COP0Reg[COUNT], cp0Counter+1);
#endif
		ADD_ImmToMemory((_u32)&VIcounter,cp0Counter+1);
        //X86_CALL((_u32)CheckTheInterrupts);

		if( cp0Counter+1 > 0xFF )	// cp0counter is larger than 0xFF

		{
			unsigned int i;
			for(i=0; i<2*(cp0Counter+1)/0x100; i++ )
			{
#ifdef SAVECPUCOUNTER
				// CPU counter should increase at the half speed of PCLOCK
				ADD_ImmToMemory((_u32)&gHardwareState.COP0Reg[COUNT], 0x40 );
#else
				ADD_ImmToMemory((_u32)&gHardwareState.COP0Reg[COUNT], 0x80);
#endif
				ADD_ImmToMemory((_u32)&VIcounter,0x80);
			}
		}

// check if the next block is compiled (i hope we can handle the addresses of that static =)
// *** i will add that later

// The Block is ready ... back to the main-loop ... i have to rewrite that too =)
	
    RET();
}

extern x86regtyp   xRD[1];
extern x86regtyp   xRS[1];
extern x86regtyp   xRT[1];

extern unsigned long lCodePosition;
unsigned long templCodePosition;
extern unsigned long JumpTargets[100];
unsigned long wPosition;
extern void SetRdRsRt64bit(OP_PARAMS);
extern FlushedMap  FlushedRegistersMap[32];

//////////////////////////////////////////////////////////////////////////////////
// branch instructions
//////////////////////////////////////////////////////////////////////////////////
void dyna4300i_bne(OP_PARAMS)
{
    _u32 aValue;
    int tempPC = reg->pc;
    int IsNear = 1; //If short, range= -128 to +127
    int Is32bit = 0;

    int tempRSIs32bit = 0;
    int tempRTIs32bit = 0;
    SetRdRsRt64bit(PASS_PARAMS);

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BNE\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
     HELP_Call((unsigned long)HELP_bne); KEEP_RECOMPILING = FALSE; return;
#endif

    aValue = (reg->pc + 4 + (__I << 2));

    tempRSIs32bit = CheckIs32Bit(xRS->mips_reg);
    tempRTIs32bit = CheckIs32Bit(xRT->mips_reg);
    if (tempRSIs32bit && tempRTIs32bit)
    {
        Is32bit = 1;
        xRS->Is32bit = 1;
        xRT->Is32bit = 1;
    }

    MapRS;
    MapRT;

	CMP_Reg1WithReg2(1, xRS->x86reg, xRT->x86reg);
_Redo:    
    templCodePosition = lCodePosition;

    if (!Is32bit)
    {
        Jcc_auto(CC_NE, 90); //jmp true
        CMP_Reg1WithReg2(1, xRS->HiWordLoc, xRT->HiWordLoc);
    }

        if (IsNear == 1)
            Jcc_Near_auto(CC_E, 91); //jmp false
        else
            Jcc_auto(CC_E, 91); //jmp false

// true
	if (!Is32bit)
        SetTarget(90);

        SPEED_HACK
        reg->pc += 4;

        Compile_Slot(reg->pc);
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

//false
	wPosition = lCodePosition - JumpTargets[91];
    if ((wPosition > 127) && (IsNear == 0))
    {
        lCodePosition = templCodePosition;
        reg->pc -= 4;
        //Rewrite the code as a near jump. Short jump won't cut it.
        IsNear = 1;
        goto _Redo;
    }

    if (IsNear == 0)
    {
        SetTarget(91);
    }
    else
    {
        SetNearTarget(91);
    }

// end of compiled block
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
#else

    MOV_ImmToMemory(1, (unsigned long)&reg->pc, tempPC+4);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_beq(OP_PARAMS)
{
	int Is32bit = 0;
    _u32 aValue;
    int tempPC = reg->pc;
    SetRdRsRt64bit(PASS_PARAMS);

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BNE\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
     HELP_Call((unsigned long)HELP_beq); KEEP_RECOMPILING = FALSE; return;
#endif

/*
    if (CheckIs32Bit(xRS->mips_reg) && CheckIs32Bit(xRT->mips_reg))
    {
        Is32bit = 1;
        xRS->Is32bit = 1;
        xRT->Is32bit = 1;
    }
*/     
     
     aValue = (reg->pc + 4 + (__I << 2));

    MapRS;
    MapRT;
	CMP_Reg1WithReg2(1,xRS->x86reg, xRT->x86reg);
	Jcc_Near_auto(CC_NE, 91); //jmp false

    CMP_Reg1WithReg2(1,xRS->HiWordLoc, xRT->HiWordLoc);
    Jcc_Near_auto(CC_NE, 93); //jmp false*/

// true
        SPEED_HACK

        reg->pc += 4;
		Compile_Slot(reg->pc);

        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

//false
	wPosition = lCodePosition - JumpTargets[91];
//    if (wPosition > 127)
//        DisplayError("%08X: beq: Code %d bytes is too large for a short jump. Must be <= 127 bytes.", reg->pc-4, wPosition);
	SetNearTarget(91);
    SetNearTarget(93);

#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
#else

    MOV_ImmToMemory(1, (unsigned long)&reg->pc, tempPC+4);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_beql(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc+4;
    SetRdRsRt64bit(PASS_PARAMS);

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BNE\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
     HELP_Call((unsigned long)HELP_beq); KEEP_RECOMPILING = FALSE; return;
#endif

    aValue = (reg->pc + 4 + (__I << 2));

    MapRS;
    MapRT;
	CMP_Reg1WithReg2(1,xRS->x86reg, xRT->x86reg);
	Jcc_Near_auto(CC_NE, 91); //jmp false

    CMP_Reg1WithReg2(1,xRS->HiWordLoc, xRT->HiWordLoc);
    Jcc_Near_auto(CC_NE, 93); //jmp false*/

// true
        SPEED_HACK

        reg->pc += 4;
		Compile_Slot(reg->pc);

        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

//false
	wPosition = lCodePosition - JumpTargets[91];
//    if (wPosition > 127)
//        DisplayError("%08X: beq: Code %d bytes is too large for a short jump. Must be <= 127 bytes.", reg->pc-4, wPosition);
	SetNearTarget(91);
    SetNearTarget(93);

#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
    *pcptr++;
#else

    MOV_ImmToMemory(1, (unsigned long)&reg->pc, tempPC+4);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_bnel(OP_PARAMS)
{
    _u32 aValue;
    int tempPC = reg->pc+4;
    int IsNear = 1; //If short, range= -128 to +127

    SetRdRsRt64bit(PASS_PARAMS);
    
#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BNE\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
     HELP_Call((unsigned long)HELP_bne); KEEP_RECOMPILING = FALSE; return;
#endif

    aValue = (reg->pc + 4 + (__I << 2));

    MapRS;
    MapRT;

	CMP_Reg1WithReg2(1,xRS->x86reg, xRT->x86reg);
_Redo:    
    templCodePosition = lCodePosition;
    Jcc_auto(CC_NE, 90); //jmp true
    CMP_Reg1WithReg2(1,xRS->HiWordLoc, xRT->HiWordLoc);

    if (IsNear == 1)
        Jcc_Near_auto(CC_E, 91); //jmp false
    else
        Jcc_auto(CC_E, 91); //jmp false

// true
	SetTarget(90);

        //SPEED_HACK
		if ( (__I == -1)  )																		
		{
			if ( pcptr[1] == 0 ) // The next instruction is NOP
			{
				PUSHAD();
				X86_CALL((_u32)DoSpeedHack);                                                
				POPAD();                                                                        
			}
			else if( ( pcptr[1] & 0xFC000000) == 0x24000000 && strcmp(rominfo.name,"Mario Kart 64") != 0)
			{
				// This speed hack works for Banjo, but will crash Mario Kart
				INTERPRET(DoBNELSpeedHack);
			}
		}	

        reg->pc += 4;
		Compile_Slot(reg->pc);

        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

//false
	wPosition = lCodePosition - JumpTargets[91];
    if ((wPosition > 127) && (IsNear == 0))
    {
        lCodePosition = templCodePosition;
        reg->pc -= 4;
        //Rewrite the code as a near jump. Short jump won't cut it.
        IsNear = 1;
        goto _Redo;
    }

    if (IsNear == 0)
    {
        SetTarget(91);
    }
    else
    {
        SetNearTarget(91);
    }

#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
    *pcptr++;
#else
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
#endif
}


//////////////////////////////////////////////////////////////////////////////////
#define BLEZ_MACRO      BRANCH_MACRO(CC_G, CC_L, CC_A)
#define BLTZ_MACRO      BRANCH_MACRO(CC_G, CC_L, CC_AE)
#define BGEZ_MACRO      BRANCH_MACRO(CC_L, CC_G, CC_B)
#define BGTZ_MACRO      BRANCH_MACRO(CC_L, CC_G, CC_BE)

#define BRANCH_MACRO(JMP1, JMP2, JMP3)      \
	                                        \
    aValue = (reg->pc + 4 + (__I << 2));    \
                                            \
    MapRS;                                  \
	CMP_RegWithShort(1, xRS->HiWordLoc, 0); \
	Jcc_Near_auto(JMP1, 91); /* jmp false */\
	Jcc_auto(JMP2, 90); /* jmp true */      \
                                            \
	CMP_RegWithShort(1, xRS->x86reg, 0);    \
	Jcc_Near_auto(JMP3, 93); /* jmp false2 */


void dyna4300i_blez(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc;
    SetRdRsRt64bit(PASS_PARAMS);
#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BLEZ\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_blez); KEEP_RECOMPILING = FALSE; return;
#endif

//    BLEZ_MACRO

//
    aValue = (reg->pc + 4 + (__I << 2));

    MapRS;
	CMP_RegWithShort(1, xRS->HiWordLoc, 0);
	Jcc_Near_auto(CC_G, 91); /* jmp false */
	Jcc_auto(CC_L, 90); /* jmp true */

	CMP_RegWithShort(1, xRS->x86reg, 0);
	Jcc_Near_auto(CC_A, 93); /* jmp false2 */
//


// true
	SetTarget(90);
		SPEED_HACK
		reg->pc += 4;

        Compile_Slot(reg->pc);
        
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

//false
	SetNearTarget(91);
    SetNearTarget(93);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
#else
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
#endif
}

//////////////////////////////////////////////////////////////////////////////////
void dyna4300i_blezl(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc+4;
    SetRdRsRt64bit(PASS_PARAMS);
#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BLEZ\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_blez); KEEP_RECOMPILING = FALSE; return;
#endif

//    BLEZ_MACRO

//
    aValue = (reg->pc + 4 + (__I << 2));

    MapRS;
	CMP_RegWithShort(1, xRS->HiWordLoc, 0);
	Jcc_Near_auto(CC_G, 91); /* jmp false */
	Jcc_auto(CC_L, 90); /* jmp true */

	CMP_RegWithShort(1, xRS->x86reg, 0);
	Jcc_Near_auto(CC_A, 93); /* jmp false2 */
//


// true
	SetTarget(90);
		SPEED_HACK
		reg->pc += 4;

        Compile_Slot(reg->pc);
        
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

//false
	SetNearTarget(91);
    SetNearTarget(93);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
    *pcptr++;
#else
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_bgtz(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc;
    SetRdRsRt64bit(PASS_PARAMS);

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BGTZ\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bgtz); KEEP_RECOMPILING = FALSE; return;
#endif

//    BGTZ_MACRO
//#define BGTZ_MACRO      BRANCH_MACRO(CC_L, CC_G, CC_BE)
//
    aValue = (reg->pc + 4 + (__I << 2));

    MapRS;
	CMP_RegWithShort(1, xRS->HiWordLoc, 0);
	Jcc_Near_auto(CC_L, 91); /* jmp false */
	Jcc_auto(CC_G, 90); /* jmp true */

	CMP_RegWithShort(1, xRS->x86reg, 0);
	Jcc_Near_auto(CC_BE, 93); /* jmp false2 */
//
// delay Slot
	SetTarget(90);
		SPEED_HACK
		reg->pc += 4;

        Compile_Slot(reg->pc);
        
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

	SetNearTarget(91);
    SetNearTarget(93);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
#else
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_bgtzl(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc+4;
    SetRdRsRt64bit(PASS_PARAMS);

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BGTZ\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bgtz); KEEP_RECOMPILING = FALSE; return;
#endif

//    BGTZ_MACRO
//#define BGTZ_MACRO      BRANCH_MACRO(CC_L, CC_G, CC_BE)
//
    aValue = (reg->pc + 4 + (__I << 2));

    MapRS;
	CMP_RegWithShort(1, xRS->HiWordLoc, 0);
	Jcc_Near_auto(CC_L, 91); /* jmp false */
	Jcc_auto(CC_G, 90); /* jmp true */

	CMP_RegWithShort(1, xRS->x86reg, 0);
	Jcc_Near_auto(CC_BE, 93); /* jmp false2 */
//
// delay Slot
	SetTarget(90);
		SPEED_HACK
		reg->pc += 4;

        Compile_Slot(reg->pc);
        
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

	SetNearTarget(91);
    SetNearTarget(93);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
    *pcptr++;
#else
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_bltz(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc;
    SetRdRsRt64bit(PASS_PARAMS);

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BLTZ\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bltz); KEEP_RECOMPILING = FALSE; return;
#endif

    aValue = (reg->pc + 4 + (__I << 2));

    MapRS;
	CMP_RegWithShort(1, xRS->HiWordLoc, 0);
	Jcc_Near_auto(CC_GE, 91); /* jmp false */

// delay Slot
		SPEED_HACK
		reg->pc += 4;

        Compile_Slot(reg->pc);
        
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

	SetNearTarget(91);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
#else
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_bltzl(OP_PARAMS) 
{
	_u32 aValue;
    SetRdRsRt64bit(PASS_PARAMS);

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BLTZL\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bltzl); KEEP_RECOMPILING = FALSE; return;
#endif

    aValue = (reg->pc + 4 + (__I << 2));

    MapRS;
	CMP_RegWithShort(1, xRS->HiWordLoc, 0);
	Jcc_Near_auto(CC_GE, 91); /* jmp false */

// delay Slot
		SPEED_HACK
		reg->pc += 4;
		Compile_Slot(reg->pc);
		MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

	SetNearTarget(91);
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_bgez(OP_PARAMS) 
{
	_u32 aValue;
    SetRdRsRt64bit(PASS_PARAMS);

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BGEZ\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bgez); KEEP_RECOMPILING = FALSE; return;
#endif

    BGEZ_MACRO

// delay Slot
	SetTarget(90);
		SPEED_HACK
		reg->pc += 4;
		Compile_Slot(reg->pc);
		MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

	SetNearTarget(91);
	SetNearTarget(93);
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_bgezl(OP_PARAMS) 
{
	_u32 aValue;
    SetRdRsRt64bit(PASS_PARAMS);

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BGEZL\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bgezl); KEEP_RECOMPILING = FALSE; return;
#endif

    BGEZ_MACRO

// delay Slot
	SetTarget(90);
		SPEED_HACK
		reg->pc += 4;
		Compile_Slot(reg->pc);
		MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

	SetNearTarget(91);
	SetNearTarget(93);
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_bgezal(OP_PARAMS)
{
	_u32 aValue;
    _s64 LinkVal = (_s64)(_s32)reg->pc+8;
    SetRdRsRt64bit(PASS_PARAMS);

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BGEZAL\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bgezal); KEEP_RECOMPILING = FALSE; return;
#endif

    FlushAllRegisters();
    MOV_ImmToMemory(1, (unsigned long)&reg->GPR[31],   (_s32)LinkVal);
    MOV_ImmToMemory(1, (unsigned long)&reg->GPR[31]+4, (_s32)(LinkVal>>31));
//    FlushedRegistersMap[31].Is32bit = 1;

    BGEZ_MACRO

// delay Slot
	SetTarget(90);
		SPEED_HACK
		reg->pc += 4;
		Compile_Slot(reg->pc);
		MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

	SetNearTarget(91);
	SetNearTarget(93);
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_bgezall(OP_PARAMS)
{
	_u32 aValue;
    _s64 LinkVal = (_s64)(_s32)reg->pc+8;
    SetRdRsRt64bit(PASS_PARAMS);

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BGEZALL\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bgezall); KEEP_RECOMPILING = FALSE; return;
#endif

    FlushAllRegisters();
    MOV_ImmToMemory(1, (unsigned long)&reg->GPR[31],   (_s32)LinkVal);
    MOV_ImmToMemory(1, (unsigned long)&reg->GPR[31]+4, (_s32)(LinkVal>>31));
//    FlushedRegistersMap[31].Is32bit = 1;

    BGEZ_MACRO

// delay Slot
	SetTarget(90);
		SPEED_HACK
		reg->pc += 4;
		Compile_Slot(reg->pc);
		MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

	SetNearTarget(91);
	SetNearTarget(93);
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_bltzal(OP_PARAMS)
{
	_u32 aValue;
    _s64 LinkVal = (_s64)(_s32)reg->pc+8;
    SetRdRsRt64bit(PASS_PARAMS);

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BLTZAL\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
   HELP_Call((unsigned long)HELP_bltzal); KEEP_RECOMPILING = FALSE; return;
#endif

    
    FlushAllRegisters();
    MOV_ImmToMemory(1, (unsigned long)&reg->GPR[31],   (_s32)LinkVal);
    MOV_ImmToMemory(1, (unsigned long)&reg->GPR[31]+4, (_s32)(LinkVal>>31));
//    FlushedRegistersMap[31].Is32bit = 1;

    aValue = (reg->pc + 4 + (__I << 2));

    MapRS;
	CMP_RegWithShort(1, xRS->HiWordLoc, 0);
	Jcc_Near_auto(CC_GE, 91); /* jmp false */

// delay Slot
		SPEED_HACK
		reg->pc += 4;
		Compile_Slot(reg->pc);
		MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

	SetNearTarget(91);
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_bltzall(OP_PARAMS)
{
	_u32 aValue;
    _s64 LinkVal = (_s64)(_s32)reg->pc+8;
    SetRdRsRt64bit(PASS_PARAMS);

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BLTZALL\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bltzall); KEEP_RECOMPILING = FALSE; return;
#endif

    FlushAllRegisters();
    MOV_ImmToMemory(1, (unsigned long)&reg->GPR[31],   (_s32)LinkVal);
    MOV_ImmToMemory(1, (unsigned long)&reg->GPR[31]+4, (_s32)(LinkVal>>31));
//    FlushedRegistersMap[31].Is32bit = 1;

    aValue = (reg->pc + 4 + (__I << 2));

    MapRS;
	CMP_RegWithShort(1, xRS->HiWordLoc, 0);
	Jcc_Near_auto(CC_GE, 91); /* jmp false */

// delay Slot
		SPEED_HACK
		reg->pc += 4;
		Compile_Slot(reg->pc);
		MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

	SetNearTarget(91);
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_jal(OP_PARAMS)
{
	_u32 aValue;
#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	JAL\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_jal); KEEP_RECOMPILING = FALSE; return;
#endif

	
    aValue = ( (reg->pc & 0xf0000000) | (____T << 2) );

	FlushAllRegisters();
    MOV_ImmToMemory(1, (unsigned long)&reg->GPR[31], (reg->pc + 8));
    MOV_ImmToMemory(1, (unsigned long)&reg->GPR[31]+4, (reg->pc + 8)>>31);
//    FlushedRegistersMap[31].Is32bit = 1;

	reg->pc += 4;
	Compile_Slot_Jump(reg->pc);
	MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_j(OP_PARAMS)
{
	_u32 aValue;
#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	J\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_j); KEEP_RECOMPILING = FALSE; return; 
#endif

	
    aValue = ( (reg->pc & 0xf0000000) | (____T << 2) );

	J_SPEED_HACK
    reg->pc += 4;
    Compile_Slot_Jump(reg->pc);
	MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_special_jr(OP_PARAMS)
{
#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	JR\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_jr); KEEP_RECOMPILING = FALSE; return;
#endif
	
    FlushAllRegisters();
    LoadLowMipsCpuRegister(__RS,Reg_EAX);
	MOV_EAXToMemory(1, (unsigned long)&reg->pc);

	reg->pc += 4;
	Compile_Slot_Jump(reg->pc);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
}


//////////////////////////////////////////////////////////////////////////////
// void (*dyna4300i_cop1_BC_instruction[])(OP_PARAMS)=                      //
//////////////////////////////////////////////////////////////////////////////
void dyna4300i_cop1_bc1f(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc;

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BC1F\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bc1f); KEEP_RECOMPILING = FALSE; return;
#endif

    aValue = (reg->pc + 4 + (__I << 2));

    MOV_MemoryToReg(1, Reg_ECX, ModRM_disp32, (_u32)&reg->COP1Con[31]);
    AND_ImmToReg(1, Reg_ECX, COP1_CONDITION_BIT);

    TEST_Reg2WithReg1(1, Reg_ECX, Reg_ECX);
	Jcc_Near_auto(CC_NZ, 91); /* jmp false */

// delay Slot
		SPEED_HACK
		reg->pc += 4;

        Compile_Slot(reg->pc);

        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

    SetNearTarget(91);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
#else
    MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_cop1_bc1fl(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc+4;

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BC1FL\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bc1fl); KEEP_RECOMPILING = FALSE; return;
#endif

    aValue = (reg->pc + 4 + (__I << 2));

    MOV_MemoryToReg(1, Reg_ECX, ModRM_disp32, (unsigned long)(&reg->COP1Con[31]));
    AND_ImmToReg(1, Reg_ECX, COP1_CONDITION_BIT);

    TEST_Reg2WithReg1(1, Reg_ECX, Reg_ECX);
	Jcc_Near_auto(CC_NZ, 91); /* jmp false */

// delay Slot
		SPEED_HACK
		reg->pc += 4;

        Compile_Slot(reg->pc);

        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

    SetNearTarget(91);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
    *pcptr++;
#else
    MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_cop1_bc1t(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc;

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BC1T\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bc1t); KEEP_RECOMPILING = FALSE; return;
#endif

    aValue = (reg->pc + 4 + (__I << 2));

    MOV_MemoryToReg(1, Reg_ECX, ModRM_disp32, (_u32)&reg->COP1Con[31]);
    AND_ImmToReg(1, Reg_ECX, COP1_CONDITION_BIT);

    TEST_Reg2WithReg1(1, Reg_ECX, Reg_ECX);
	Jcc_Near_auto(CC_Z, 91); /* jmp false */

// delay Slot
		SPEED_HACK
		reg->pc += 4;

        Compile_Slot(reg->pc);

        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

    SetNearTarget(91);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
#else
    MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_cop1_bc1tl(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc+4;

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	BC1TL\n", reg->pc);
#endif LOG_DYNA
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bc1tl); KEEP_RECOMPILING = FALSE; return;
#endif

    aValue = (reg->pc + 4 + (__I << 2));

    MOV_MemoryToReg(1, Reg_ECX, ModRM_disp32, (unsigned long)(&reg->COP1Con[31]));
    AND_ImmToReg(1, Reg_ECX, COP1_CONDITION_BIT);

    TEST_Reg2WithReg1(1, Reg_ECX, Reg_ECX);
	Jcc_Near_auto(CC_Z, 91); /* jmp false */

// delay Slot
		SPEED_HACK
		reg->pc += 4;

        Compile_Slot(reg->pc);

        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        Interrupts();

    SetNearTarget(91);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
    *pcptr++;
#else
    MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_special_jalr(OP_PARAMS)
{
	_u32 aValue;
    
#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_jalr); KEEP_RECOMPILING = FALSE; return;
#endif
#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	JALR\n", reg->pc);
#endif LOG_DYNA

	
    aValue = ( (reg->pc & 0xf0000000) | (____T << 2) );

	FlushAllRegisters();
    MOV_ImmToMemory(1, (unsigned long)&reg->GPR[__RD], (reg->pc + 8));
    MOV_ImmToMemory(1, (unsigned long)&reg->GPR[__RD]+4, (reg->pc + 8)>>31);
//	FlushedRegistersMap[__RD].Is32bit = 1;

    LoadLowMipsCpuRegister(__RS,Reg_EAX);
	MOV_EAXToMemory(1, (unsigned long)&reg->pc);

	reg->pc += 4;
	Compile_Slot_Jump(reg->pc);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
}

//////////////////////////////////////////////////////////////////////////////////
extern BOOL finish_dyna_exception;
extern void RefreshOpList(char *str);
void dyna4300i_cop0_eret(OP_PARAMS)
{
#ifdef DEBUG_DYNA
		sprintf(generalmessage, "ERET");
		RefreshOpList(generalmessage); 
#endif

#ifdef LOG_DYNA
	LogDyna(" 0x%08X:	ERET\n", reg->pc);
#endif LOG_DYNA

#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_eret); 
	KEEP_RECOMPILING = FALSE; 
	return;
#endif

    MOV_MemoryToReg(1,Reg_ECX,ModRM_disp32,(_u32)&reg->COP0Reg[STATUS]);
	AND_ImmToReg(1, Reg_ECX, 0x0004);
	CMP_RegWithShort(1, Reg_ECX, 0);
	Jcc_auto(CC_E,0);
		MOV_MemoryToReg(1,Reg_ECX,ModRM_disp32,(_u32)&reg->COP0Reg[ERROREPC]);
		AND_ImmToMemory((unsigned long)&reg->COP0Reg[STATUS], 0xFFFFFFFB);
		JMP_Short_auto(1);
	SetTarget(0);
    	MOV_MemoryToReg(1,Reg_ECX,ModRM_disp32,(_u32)&reg->COP0Reg[EPC]);
		AND_ImmToMemory((unsigned long)&reg->COP0Reg[STATUS], 0xFFFFFFFD);
	SetTarget(1);	

	MOV_ImmToMemory(1, (unsigned long)&reg->LLbit, 0);
	MOV_RegToMemory(1, Reg_ECX, ModRM_disp32, (unsigned long)&reg->pc);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts();
	finish_dyna_exception = TRUE;
}