//#define SAFE_BRANCHES 1
//#define DANGEROUS_BRANCHES 1
//////////////////////////////////////////////////////////////////////////////////
// small and dirty helper, hacks and cheats =)				                    //
//////////////////////////////////////////////////////////////////////////////////
//#define SPEED_HACK
#include "..\options.h"
//#include "..\dbgprint.h"
#include "..\debug_option.h"
#include "..\1964ini.h"

#define SPEEDHACK_MISSING	DisplayError("Missing SpeedHack");

extern uint32 g_translatepc;
extern uint32* g_LookupPtr;
extern uint32 g_pc_is_rdram;
static uint8* tempblock;

#ifdef DYNA_GET_BLOCK_SPEEDUP
#define TLB_TRANSLATE_PC(pc)						\
	{												\
		if( NOT_IN_KO_K1_SEG(pc))					\
		{											\
				PushMap();							\
				X86_CALL((_u32)Set_Translate_PC);	\
				PopMap();							\
		}											\
		else										\
		{											\
			g_translatepc = pc;						\
			if( (g_translatepc & 0x1FFFFFFF) < current_rdram_size )				\
				g_pc_is_rdram = g_translatepc&0x007FFFFF;						\
			else																\
				g_pc_is_rdram = 0;												\
			MOV_ImmToMemory(1, (unsigned long)&g_pc_is_rdram, g_pc_is_rdram);	\
			MOV_ImmToMemory(1, (unsigned long)&g_translatepc, g_translatepc);	\
			g_LookupPtr = (uint32*)((uint8*)sDYN_PC_LOOKUP[g_translatepc>>16] + (uint16)g_translatepc);	\
			Try_Lookup_Ptr();																	\
			MOV_ImmToMemory(1, (unsigned long)&g_LookupPtr, (uint32)g_LookupPtr);				\
		}																						\
	}

#define TLB_TRANSLATE_PC_INDIRECT		\
		PushMap();						\
        X86_CALL((_u32)Set_Translate_PC);	\
        PopMap();
#else
#define TLB_TRANSLATE_PC(pc)
#define TLB_TRANSLATE_PC_INDIRECT
#endif

#define SPEED_HACK								\
    if ( __I == -1 )							\
	{											\
		if( pcptr[1] == 0 )						\
		{										\
			PushMap();							\
			X86_CALL((_u32)DoSpeedHack);		\
			PopMap();							\
		}										\
	}	


//#define J_SPEED_HACK    

#define J_SPEED_HACK                                                                            \
                                                                                                \
if ( (reg->pc == aValue) && (pcptr[1] == 0))                                                    \
    {                                                                                           \
		/*MOV_ImmToMemory(1, (_u32)&gHWS_COP0Reg[COUNT], MAGICNUMBER);*/              \
		PushMap();                                                                               \
        X86_CALL((_u32)DoSpeedHack);                                                            \
        PopMap();                                                                                \
    }

extern MapConstant ConstMap[NUM_CONSTS];
extern x86regtyp   x86reg[8];
MapConstant TempConstMap[NUM_CONSTS];
x86regtyp   Tempx86reg[8];
extern _u32 cp0Counter;

#ifdef ENABLE_OPCODE_DEBUGGER
MapConstant TempConstMap_Debug[NUM_CONSTS];
x86regtyp Tempx86reg_Debug[8];
#endif

BOOL CompilingSlot = FALSE;
void Compile_Slot(_u32 pc)
{
	_u8 op;

	if( CompilingSlot )
	{
		DisplayError("Branch in branch delay slot PC=%08X", gHWS_pc);
		return;
	}

	// Add by Rice. 2001-08-11
	if( currentromoptions.Use_Register_Caching == USEREGC_NO )
    {
	    FlushAllRegisters();
    }

#ifdef DEBUG_COMMON
	//MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);
    FlushAllRegisters();
	//OR_ImmToMemory(1, BD, (unsigned long)&gHWS_COP0Reg[CAUSE]);
#endif

	CompilingSlot = TRUE;

#ifdef LOG_DYNA
    LogDyna("** Compile Delay Slot\n", pc);
#endif LOG_DYNA

#ifdef _USE_DEBUGGER_
    HELP_Call((unsigned long)HELP_debug);
#endif _USE_DEBUGGER_

    gHWS_code = pcptr[1];
    op = (_u8)(gHWS_code >> 26);

    memcpy(TempConstMap, ConstMap, sizeof(ConstMap));
    memcpy(Tempx86reg, x86reg, sizeof(x86reg));

	DYNA_LOG_INSTRUCTION;
    
	dyna_instruction[op](&gHardwareState);

//    if (dyna_branch == TRUE) DisplayError("Branch in delay slot!?");
    FlushAllRegisters();

#ifdef DEBUG_COMMON
	//AND_ImmToMemory((unsigned long)&gHWS_COP0Reg[CAUSE], NOT_BD);
#endif

    memcpy(ConstMap, TempConstMap, sizeof(ConstMap));
    memcpy(x86reg, Tempx86reg, sizeof(x86reg));

	CompilingSlot = FALSE;
}


void Compile_Slot_Jump(_u32 pc)
{
	_u8 op;

	if( CompilingSlot )
	{
		DisplayError("Branch in branch delay slot PC=%08X", gHWS_pc);
		return;
	}

	// Add by Rice. 2001-08-11
	if( currentromoptions.Use_Register_Caching == USEREGC_NO )
	    FlushAllRegisters();

#ifdef DEBUG_COMMON
	//MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);
    FlushAllRegisters();
	//OR_ImmToMemory(1, BD, (unsigned long)&gHWS_COP0Reg[CAUSE]);
#endif

	CompilingSlot = TRUE;

#ifdef LOG_DYNA
	LogDyna("** Compile Delay Slot\n", pc);
#endif LOG_DYNA

#ifdef _USE_DEBUGGER_
	HELP_Call((unsigned long)HELP_debug);
#endif _USE_DEBUGGER_

	gHWS_code = pcptr[1];
	op = (_u8)(gHWS_code >> 26);

	DYNA_LOG_INSTRUCTION;

    dyna_instruction[op](&gHardwareState);
    FlushAllRegisters();
#ifdef DEBUG_COMMON
	//AND_ImmToMemory((unsigned long)&gHWS_COP0Reg[CAUSE], NOT_BD);
#endif

	CompilingSlot = FALSE;
}




extern void CheckTheInterrupts(_u32 count);
extern uint32 VIcounter;
extern uint32 g_blocksize;
extern __int32 countdown_counter;
extern uint32 TempPC;
extern uint8* Block;
uint32 BlockStartAddress;
extern uint32 BlockStart;

#define JUMP_TYPE_INDIRECT 0
#define JUMP_TYPE_DIRECT   1
#define JUMP_TYPE_BREAKOUT 2
void CompareStates(uint32 Instruction);
void Interrupts(uint32 JumpType)
{
	uint32 viCounter, COUNTCounter;
	if( JumpType == JUMP_TYPE_BREAKOUT )
	{
		viCounter = cp0Counter*VICounterFactors[CounterFactor];
		COUNTCounter = cp0Counter*CounterFactors[CounterFactor]/VICounterFactors[CounterFactor];
	}
	else	//need to execute the delay slot instruction, so need to increase cp0Counter by 1
	{
		viCounter = (cp0Counter+1)*VICounterFactors[CounterFactor];
		COUNTCounter = (cp0Counter+1)*CounterFactors[CounterFactor]/VICounterFactors[CounterFactor];
	}

	MOV_ImmToMemory(1, (_u32)&g_blocksize, cp0Counter+1);
#ifdef FAST_COUNTER
	SUB_ImmToMemory((_u32)&countdown_counter, viCounter);

#ifdef ENABLE_OPCODE_DEBUGGER
	if( debug_opcode == 1 && debug_opcode_block == 1 )
	{
		MOV_ImmToReg(1, Reg_ECX, 0);
		MOV_ImmToReg(1, Reg_EAX, (uint32)&CompareStates);
		CALL_Reg(Reg_EAX);
	}
#endif

    if ((TempPC == g_translatepc) && (JumpType == JUMP_TYPE_DIRECT)) //it's a loop
    {
        CMP_MemoryWithImm(1, (uint32)&countdown_counter, 0);
        Jcc_auto(CC_LE, 97); //jmp true

        CMP_MemoryWithImm(1, (uint32)&CPUNeedToDoOtherTask, 0);
        Jcc_auto(CC_NE, 98); //jmp true
/*
        Ok, just make sure our stuff is flushed..
        memcpy(TempConstMap, ConstMap, sizeof(ConstMap));
        memcpy(Tempx86reg, x86reg, sizeof(x86reg));

        FlushAllRegisters();

        memcpy(ConstMap, TempConstMap, sizeof(ConstMap));
        memcpy(x86reg, Tempx86reg, sizeof(x86reg));
*/
//        DisplayError("Block = %08X", (uint32)Block);
        MOV_ImmToMemory(1, (uint32)&BlockStartAddress, (uint32)BlockStart);

        JMP_FAR((uint32)&BlockStartAddress);

        SetTarget(97);
        SetTarget(98);
    }
#else
	ADD_ImmToMemory((_u32)&gHWS_COP0Reg[COUNT], COUNTCounter);
	ADD_ImmToMemory((_u32)&VIcounter,viCounter);
#endif

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
extern FlushedMap  FlushedRegistersMap[NUM_CONSTS];
extern void SwitchToOpcodePass();

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

    CHECK_OPCODE_PASS
    
    SetRdRsRt64bit(PASS_PARAMS);

	_OPCODE_DEBUG_BRANCH_(r4300i_bne);

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
		TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

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
	TLB_TRANSLATE_PC(tempPC+4)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_beq(OP_PARAMS)
{
	int Is32bit = 0;
    _u32 aValue;
    int tempPC = reg->pc;

    CHECK_OPCODE_PASS

    SetRdRsRt64bit(PASS_PARAMS);
	_OPCODE_DEBUG_BRANCH_(r4300i_beq);

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
		TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

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
	TLB_TRANSLATE_PC(tempPC+4)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_beql(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc+4;

    CHECK_OPCODE_PASS

    SetRdRsRt64bit(PASS_PARAMS);
	_OPCODE_DEBUG_BRANCH_(r4300i_beql);

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
		TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

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
	TLB_TRANSLATE_PC(tempPC+4)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_bnel(OP_PARAMS)
{
    _u32 aValue;
    int tempPC = reg->pc+4;
    int IsNear = 1; //If short, range= -128 to +127

    CHECK_OPCODE_PASS

    SetRdRsRt64bit(PASS_PARAMS);
	_OPCODE_DEBUG_BRANCH_(r4300i_bnel);
    
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

        SPEED_HACK

        reg->pc += 4;
		Compile_Slot(reg->pc);

        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
		TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

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
		TLB_TRANSLATE_PC(reg->pc+4)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
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

    CHECK_OPCODE_PASS    

    SetRdRsRt64bit(PASS_PARAMS);
	_OPCODE_DEBUG_BRANCH_(r4300i_blez);

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
		TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

//false
	SetNearTarget(91);
    SetNearTarget(93);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
#else
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc);
		TLB_TRANSLATE_PC(reg->pc)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
#endif
}

//////////////////////////////////////////////////////////////////////////////////
void dyna4300i_blezl(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc+4;

    CHECK_OPCODE_PASS

	if( CompilingSlot )
	{
		DisplayError("BLEZL in branch delay slot PC=%08X", gHWS_pc);
		return;
	}

    SetRdRsRt64bit(PASS_PARAMS);
	_OPCODE_DEBUG_BRANCH_(r4300i_blezl);

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
		TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

//false
	SetNearTarget(91);
    SetNearTarget(93);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
    *pcptr++;
#else
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);
		TLB_TRANSLATE_PC(reg->pc+4)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_bgtz(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc;

    CHECK_OPCODE_PASS

    SetRdRsRt64bit(PASS_PARAMS);
	_OPCODE_DEBUG_BRANCH_(r4300i_bgtz);

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
		TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

	SetNearTarget(91);
    SetNearTarget(93);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
#else
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc);
		TLB_TRANSLATE_PC(reg->pc)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_bgtzl(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc+4;

    CHECK_OPCODE_PASS    
    
    SetRdRsRt64bit(PASS_PARAMS);
	_OPCODE_DEBUG_BRANCH_(r4300i_bgtzl);

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
        TLB_TRANSLATE_PC(aValue)
		Interrupts(JUMP_TYPE_DIRECT);

	SetNearTarget(91);
    SetNearTarget(93);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
    *pcptr++;
#else
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);
		TLB_TRANSLATE_PC(reg->pc+4)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_bltz(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc;

    CHECK_OPCODE_PASS    

    SetRdRsRt64bit(PASS_PARAMS);
	_OPCODE_DEBUG_BRANCH_(r4300i_bltz);

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
        TLB_TRANSLATE_PC(aValue)
		Interrupts(JUMP_TYPE_DIRECT);

	SetNearTarget(91);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
#else
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc);
		TLB_TRANSLATE_PC(reg->pc)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_bltzl(OP_PARAMS) 
{
	_u32 aValue;

    CHECK_OPCODE_PASS    
    
    SetRdRsRt64bit(PASS_PARAMS);
	_OPCODE_DEBUG_BRANCH_(r4300i_bltzl);

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
        TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

	SetNearTarget(91);
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);
		TLB_TRANSLATE_PC(reg->pc+4)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_bgez(OP_PARAMS) 
{
	_u32 aValue;
    CHECK_OPCODE_PASS    
    
    SetRdRsRt64bit(PASS_PARAMS);
	_OPCODE_DEBUG_BRANCH_(r4300i_bgez);

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
        TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

	SetNearTarget(91);
	SetNearTarget(93);
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc);
		TLB_TRANSLATE_PC(reg->pc)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_bgezl(OP_PARAMS) 
{
	_u32 aValue;

    CHECK_OPCODE_PASS    
    
    SetRdRsRt64bit(PASS_PARAMS);
	_OPCODE_DEBUG_BRANCH_(r4300i_bgezl);

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
        TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

	SetNearTarget(91);
	SetNearTarget(93);
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);
		TLB_TRANSLATE_PC(reg->pc+4)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_bgezal(OP_PARAMS)
{
	_u32 aValue;
    _s64 LinkVal = (_s64)(_s32)reg->pc+8;

    CHECK_OPCODE_PASS    
    
    SetRdRsRt64bit(PASS_PARAMS);
	_OPCODE_DEBUG_BRANCH_(r4300i_bgezal);

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
        TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

	SetNearTarget(91);
	SetNearTarget(93);
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc);
		TLB_TRANSLATE_PC(reg->pc)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_bgezall(OP_PARAMS)
{
	_u32 aValue;
    _s64 LinkVal = (_s64)(_s32)reg->pc+8;
 
    CHECK_OPCODE_PASS    
    
    SetRdRsRt64bit(PASS_PARAMS);
	_OPCODE_DEBUG_BRANCH_(r4300i_bgezall);

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
        TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

	SetNearTarget(91);
	SetNearTarget(93);
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);
		TLB_TRANSLATE_PC(reg->pc+4)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_bltzal(OP_PARAMS)
{
	_u32 aValue;
    _s64 LinkVal = (_s64)(_s32)reg->pc+8;

    CHECK_OPCODE_PASS    
    
    SetRdRsRt64bit(PASS_PARAMS);
	_OPCODE_DEBUG_BRANCH_(r4300i_bltzal);

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
        TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

	SetNearTarget(91);
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc);
		TLB_TRANSLATE_PC(reg->pc)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_regimm_bltzall(OP_PARAMS)
{
	_u32 aValue;
    _s64 LinkVal = (_s64)(_s32)reg->pc+8;

    CHECK_OPCODE_PASS    
    
    SetRdRsRt64bit(PASS_PARAMS);
	_OPCODE_DEBUG_BRANCH_(r4300i_bltzall);

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
        TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

	SetNearTarget(91);
        MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);
		TLB_TRANSLATE_PC(reg->pc+4)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_jal(OP_PARAMS)
{
	_u32 aValue;

    CHECK_OPCODE_PASS    
    
	_OPCODE_DEBUG_BRANCH_(r4300i_jal)

#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_jal); KEEP_RECOMPILING = FALSE; return;
#endif

	
    aValue = ( (reg->pc & 0xf0000000) | (____T << 2) );

	FlushAllRegisters();
    MOV_ImmToMemory(1, (unsigned long)&reg->GPR[31], (reg->pc + 8));
    MOV_ImmToMemory(1, (unsigned long)&reg->GPR[31]+4, ((_int32)(reg->pc + 8))>>31);
//    FlushedRegistersMap[31].Is32bit = 1;

	reg->pc += 4;
	Compile_Slot_Jump(reg->pc);
	MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
    TLB_TRANSLATE_PC(aValue)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_DIRECT);
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_j(OP_PARAMS)
{
	_u32 aValue;

    CHECK_OPCODE_PASS    
    
	_OPCODE_DEBUG_BRANCH_(r4300i_j);

#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_j); KEEP_RECOMPILING = FALSE; return; 
#endif

	
    aValue = ( (reg->pc & 0xf0000000) | (____T << 2) );

	J_SPEED_HACK
    reg->pc += 4;
    Compile_Slot_Jump(reg->pc);
	MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
    TLB_TRANSLATE_PC(aValue)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_DIRECT);
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_special_jr(OP_PARAMS)
{
    CHECK_OPCODE_PASS    
    
	_OPCODE_DEBUG_BRANCH_(r4300i_jr)

#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_jr); KEEP_RECOMPILING = FALSE; return;
#endif
	
    FlushAllRegisters();
    LoadLowMipsCpuRegister(__RS,Reg_EAX);
	MOV_EAXToMemory(1, (unsigned long)&reg->pc);
	TLB_TRANSLATE_PC_INDIRECT

	reg->pc += 4;
	Compile_Slot_Jump(reg->pc);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_INDIRECT);
}


//////////////////////////////////////////////////////////////////////////////
// void (*dyna4300i_cop1_BC_instruction[])(OP_PARAMS)=                      //
//////////////////////////////////////////////////////////////////////////////
void dyna4300i_cop1_bc1f(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc;

    CHECK_OPCODE_PASS    

#ifdef ENABLE_OPCODE_DEBUGGER
	MOV_ImmToMemory(1, (uint32)&gHardwareState_Interpreter_Compare.pc, reg->pc);
	_OPCODE_DEBUG_BRANCH_(r4300i_COP1_bc1f);
#endif

#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bc1f); KEEP_RECOMPILING = FALSE; return;
#endif

    aValue = (reg->pc + 4 + (__I << 2));

    PUSH_RegIfMapped(Reg_EDI);
    MOV_MemoryToReg(1, Reg_EDI, ModRM_disp32, (_u32)&reg->COP1Con[31]);
    AND_ImmToReg(1, Reg_EDI, COP1_CONDITION_BIT);
    TEST_Reg2WithReg1(1, Reg_EDI, Reg_EDI);
    POP_RegIfMapped(Reg_EDI);
    
    Jcc_Near_auto(CC_NZ, 91); /* jmp false */

// delay Slot
		SPEED_HACK
		reg->pc += 4;

        Compile_Slot(reg->pc);

        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

    SetNearTarget(91);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
#else
    MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc);
	TLB_TRANSLATE_PC(reg->pc)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_cop1_bc1fl(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc+4;

    CHECK_OPCODE_PASS    

#ifdef ENABLE_OPCODE_DEBUGGER
	MOV_ImmToMemory(1, (uint32)&gHardwareState_Interpreter_Compare.pc, reg->pc);
	_OPCODE_DEBUG_BRANCH_(r4300i_COP1_bc1fl);
#endif

#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bc1fl); KEEP_RECOMPILING = FALSE; return;
#endif

    aValue = (reg->pc + 4 + (__I << 2));

    PUSH_RegIfMapped(Reg_EDI);
    MOV_MemoryToReg(1, Reg_EDI, ModRM_disp32, (unsigned long)(&reg->COP1Con[31]));
    AND_ImmToReg(1, Reg_EDI, COP1_CONDITION_BIT);
    TEST_Reg2WithReg1(1, Reg_EDI, Reg_EDI);
    POP_RegIfMapped(Reg_EDI);

	Jcc_Near_auto(CC_NZ, 91); /* jmp false */

// delay Slot
		SPEED_HACK
		reg->pc += 4;

        Compile_Slot(reg->pc);

        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

    SetNearTarget(91);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
    *pcptr++;
#else
    MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);
	TLB_TRANSLATE_PC(reg->pc+4)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_cop1_bc1t(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc;

    CHECK_OPCODE_PASS    

#ifdef ENABLE_OPCODE_DEBUGGER
	MOV_ImmToMemory(1, (uint32)&gHardwareState_Interpreter_Compare.pc, reg->pc);
	_OPCODE_DEBUG_BRANCH_(r4300i_COP1_bc1t);
#endif

#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bc1t); KEEP_RECOMPILING = FALSE; return;
#endif

    aValue = (reg->pc + 4 + (__I << 2));

    PUSH_RegIfMapped(Reg_EDI);
    MOV_MemoryToReg(1, Reg_EDI, ModRM_disp32, (_u32)&reg->COP1Con[31]);
    AND_ImmToReg(1, Reg_EDI, COP1_CONDITION_BIT);
    TEST_Reg2WithReg1(1, Reg_EDI, Reg_EDI);
    POP_RegIfMapped(Reg_EDI);

	Jcc_Near_auto(CC_Z, 91); /* jmp false */

// delay Slot
		SPEED_HACK
		reg->pc += 4;

        Compile_Slot(reg->pc);

        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

    SetNearTarget(91);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
#else
    MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc);
	TLB_TRANSLATE_PC(reg->pc)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_cop1_bc1tl(OP_PARAMS)
{
	_u32 aValue;
    int tempPC = reg->pc+4;

    CHECK_OPCODE_PASS    

#ifdef ENABLE_OPCODE_DEBUGGER
	MOV_ImmToMemory(1, (uint32)&gHardwareState_Interpreter_Compare.pc, reg->pc);
	_OPCODE_DEBUG_BRANCH_(r4300i_COP1_bc1tl);
#endif

#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_bc1tl); KEEP_RECOMPILING = FALSE; return;
#endif

    aValue = (reg->pc + 4 + (__I << 2));

    PUSH_RegIfMapped(Reg_EDI);
    MOV_MemoryToReg(1, Reg_EDI, ModRM_disp32, (unsigned long)(&reg->COP1Con[31]));
    AND_ImmToReg(1, Reg_EDI, COP1_CONDITION_BIT);
    TEST_Reg2WithReg1(1, Reg_EDI, Reg_EDI);
    POP_RegIfMapped(Reg_EDI);

    Jcc_Near_auto(CC_Z, 91); /* jmp false */

// delay Slot
		SPEED_HACK
		reg->pc += 4;

        Compile_Slot(reg->pc);

        MOV_ImmToMemory(1, (unsigned long)&reg->pc, aValue);
        TLB_TRANSLATE_PC(aValue)
        Interrupts(JUMP_TYPE_DIRECT);

    SetNearTarget(91);
#ifdef DANGEROUS_BRANCHES
    reg->pc = tempPC;
    *pcptr++;
#else
    MOV_ImmToMemory(1, (unsigned long)&reg->pc, reg->pc+4);
	TLB_TRANSLATE_PC(reg->pc+4)

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_BREAKOUT);
#endif
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_special_jalr(OP_PARAMS)
{
	_u32 aValue;
    
    CHECK_OPCODE_PASS    
    
	_OPCODE_DEBUG_BRANCH_(r4300i_jalr)

#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_jalr); KEEP_RECOMPILING = FALSE; return;
#endif

	
    aValue = ( (reg->pc & 0xf0000000) | (____T << 2) );

	FlushAllRegisters();
    MOV_ImmToMemory(1, (unsigned long)&reg->GPR[__RD], (reg->pc + 8));
    MOV_ImmToMemory(1, (unsigned long)&reg->GPR[__RD]+4, ((_int32)(reg->pc + 8))>>31);
//	FlushedRegistersMap[__RD].Is32bit = 1;

    LoadLowMipsCpuRegister(__RS,Reg_EAX);
	MOV_EAXToMemory(1, (unsigned long)&reg->pc);
	TLB_TRANSLATE_PC_INDIRECT

	reg->pc += 4;
	Compile_Slot_Jump(reg->pc);

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();
    Interrupts(JUMP_TYPE_INDIRECT);
}

//////////////////////////////////////////////////////////////////////////////////

void dyna4300i_cop0_eret(OP_PARAMS)
{
    CHECK_OPCODE_PASS    
    
    _SAFTY_CPU_(r4300i_COP0_eret) //questionable.

#ifdef SAFE_BRANCHES
    HELP_Call((unsigned long)HELP_eret); 
	KEEP_RECOMPILING = FALSE; 
	return;
#endif

    PUSH_RegIfMapped(Reg_EDI);
    MOV_MemoryToReg(1,Reg_EDI,ModRM_disp32,(_u32)&reg->COP0Reg[STATUS]);
	AND_ImmToReg(1, Reg_EDI, 0x0004);
	CMP_RegWithShort(1, Reg_EDI, 0);
	Jcc_auto(CC_E,0);
		MOV_MemoryToReg(1,Reg_EDI,ModRM_disp32,(_u32)&reg->COP0Reg[ERROREPC]);
		AND_ImmToMemory((unsigned long)&reg->COP0Reg[STATUS], 0xFFFFFFFB);
		JMP_Short_auto(1);
	SetTarget(0);
    	MOV_MemoryToReg(1,Reg_EDI,ModRM_disp32,(_u32)&reg->COP0Reg[EPC]);
		AND_ImmToMemory((unsigned long)&reg->COP0Reg[STATUS], 0xFFFFFFFD);
	SetTarget(1);	

	MOV_ImmToMemory(1, (unsigned long)&reg->LLbit, 0);
	MOV_RegToMemory(1, Reg_EDI, ModRM_disp32, (unsigned long)&reg->pc);
    POP_RegIfMapped(Reg_EDI);
	TLB_TRANSLATE_PC_INDIRECT

// end of compiled block
	KEEP_RECOMPILING = FALSE;
    FlushAllRegisters();

    Interrupts(JUMP_TYPE_INDIRECT);
}