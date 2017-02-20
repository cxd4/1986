/*______________________________________________________________________________
 |                                                                              |
 |  1964 - emulator.h                                                           |
 |  Copyright (C) 2001 Joel Middendorf, <schibo@emulation64.com>                |
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
 |  email      : dyangchicago@yahoo.com, schibo@emulation64.com                 |
 |  paper mail :                                                                |
 |______________________________________________________________________________|
*/

#ifndef __1964_EMULATIONOR_H

#define __1964_EMULATIONOR_H

#include <windows.h>
#include "options.h"
#include "debug_option.h"

extern DWORD	CPUThreadID;
extern HANDLE	CPUThreadHandle;
extern uint32	sp_hle_task;
extern int		sp_task_counter;
extern int		si_io_counter;
extern uint32	cp0Counter;
extern uint32	VIcounter;
extern int		reason_to_stop;
extern int		AiUpdating;


void RunEmulator(unsigned _int32 WhichCore);
void ClearCPUTasks(void);
void InterpreterStepCPU(void);
__forceinline uint32 FetchInstruction(void);
__forceinline void DynaFetchInstruction(void);
__forceinline void RunDynaBlock(void);
void PauseEmulating(void);
void PauseEmulator(void);
void ResumeEmulator(BOOL needinit);
void StopEmulator(void);
void EmulatorSetCore(int core);
void (*Dyna_Code_Check[])();
void (*Dyna_Check_Codes)();
void Dyna_Code_Check_None_Boot(void);
void Dyna_Code_Check_QWORD(void);
void Dyna_Exception_Service_Routine(uint32 vector);
void Dyna_Invalidate_Compiled_Block(uint32 pc);

#ifdef FAST_COUNTER
void Count_Down(uint32 count);
void Count_Down_All(void);
uint32 Get_COUNT_Register(void);
uint32 Get_VIcounter(void);
void Count_Down_All(void);
void Count_Down(uint32 count);
void Check_VI_and_COMPARE_Interrupt(void);
void Set_COMPARE_Interrupt_Target_Counter(void);
#endif

#define INTERPRETER_CHECK_VI		\
	if( VIcounter >= MAGICNUMBER )	\
	{								\
		VIcounter=0;				\
		Trigger_VIInterrupt();		\
	}

#define DYNA_CHECK_VI	INTERPRETER_CHECK_VI

#define INTERPRETER_CHECK_COMPARE												\
	if ( gHWS_COP0Reg[COUNT] == gHWS_COP0Reg[COMPARE] )		\
		Trigger_CompareInterrupt();												


#ifdef DEBUG_COMMON
#define INTERPRETER_DEBUG_INSTRUCTION			\
	if( DebuggerActive )						\
	{											\
		HandleBreakpoint(Instruction);			\
		if (DebuggerEnabled)					\
		{										\
			DebugPrintInstruction(Instruction);	\
			RefreshDebugger();					\
		}										\
	}
#else
#define INTERPRETER_DEBUG_INSTRUCTION
#endif

//#define DYNA_CHECK_COMPARE		\
//	if ( (gHWS_COP0Reg[COUNT] > gHWS_COP0Reg[COMPARE] && SavedCOUNT < gHWS_COP0Reg[COMPARE] ) ||	\
//		 (gHWS_COP0Reg[COUNT] > gHWS_COP0Reg[COMPARE] && gHWS_COP0Reg[COUNT] < SavedCOUNT ))		\
//	{																															\
//		Trigger_CompareInterrupt();																								\
//		SavedCOUNT = gHWS_COP0Reg[COUNT];																				\
//	}

#define DYNA_CHECK_COMPARE		                                            \
	if (gHWS_COP0Reg[COUNT] > gHWS_COP0Reg[COMPARE])    \
        if ( (SavedCOUNT < gHWS_COP0Reg[COMPARE]) ||	            \
		 (gHWS_COP0Reg[COUNT] < SavedCOUNT) )		                \
	{																		\
		SavedCOUNT = gHWS_COP0Reg[COUNT];						    \
		Trigger_CompareInterrupt();											\
	}


#ifdef DEBUG_COMMON
#define DYNA_DEBUG_INSTRUCTION		\
    FlushAllRegisters();            \
	if(DebuggerActive && (DebuggerEnabled || RUN_TO_ADDRESS_ACTIVE) )				\
	{								\
		rc_DYNDEBUG_UPDATE			\
		DEBUG_BPT					\
	}
#else
#define DYNA_DEBUG_INSTRUCTION
#endif

#ifdef LOG_DYNA
#ifdef DEBUG_COMMON
#define DYNA_LOG_INSTRUCTION	\
	if( debug_dyna_log )		\
		LogDyna("\n%s\n", DebugPrintInstructionWithOutRefresh(gHWS_code));
#else
#define DYNA_LOG_INSTRUCTION	\
		LogDyna("\n%s\n", DebugPrintInstructionWithOutRefresh(gHWS_code));
#endif
#else
#define DYNA_LOG_INSTRUCTION
#endif


#ifdef DEBUG_DYNAEXECUTION
#define DEBUG_PRINT_DYNA_EXECUTION_INFO												\
	if( debug_dyna_execution )														\
	{																				\
		sprintf(generalmessage, "Dyna execution: PC = %08X", gHWS_pc);	\
		RefreshOpList(generalmessage);												\
	}
#else
#define DEBUG_PRINT_DYNA_EXECUTION_INFO
#endif

#ifdef DEBUG_DYNA
#define DEBUG_PRINT_DYNA_COMPILE_INFO												\
	if( debug_dyna_compiler )	\
	{							\
		sprintf(generalmessage, "Dyna compile: memory %08X - %08X", TempPC, gHWS_pc);	\
		RefreshOpList(generalmessage);	\
	}
#else
#define DEBUG_PRINT_DYNA_COMPILE_INFO
#endif

extern __int32 countdown_counter;
#ifdef SAVEOPCOUNTER
#ifdef FAST_COUNTER
#define SAVE_OP_COUNTER_INCREASE(val)	SUB_ImmToMemory((_u32)&countdown_counter, val);
#define SAVE_OP_COUNTER_INCREASE_INTERPRETER(val)	{countdown_counter -= val*CounterFactors[CounterFactor]*2;}
#else
#define SAVE_OP_COUNTER_INCREASE(val)
#define SAVE_OP_COUNTER_INCREASE_INTERPRETER(val)	{VIcounter+=val*VICounterFactors[CounterFactor]*2;gHWS_COP0Reg[COUNT]+=val*CounterFactors[CounterFactor];}
#endif
#else
#define SAVE_OP_COUNTER_INCREASE(val)
#define SAVE_OP_COUNTER_INCREASE_INTERPRETER(val)
#endif

#endif //__1964_EMULATIONOR_H