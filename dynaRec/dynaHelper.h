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

//#define __SPEED_HACK
#include "..\options.h"
#include "..\timer.h"
#include "..\memory.h"
#include "..\1964ini.h"

#define __SPEED_HACK											\
		if(__dotI == -1)											\
			if((READ_CPU_INSTRUCTION((pc + 4))) == 0x00000000)	\
			{													\
				/*gHWS_cpr[0][COUNT] = next_interrupt_count; */ 	    \
				X86_CALL((_u32)DoSpeedHack);\
			}													\





void HELP_Call(unsigned long function)
{
    FlushAllRegisters();
    MOV_ImmToReg(1, Reg_ECX, gHWS_pc);
		X86_CALL(function);
}

extern uint32 VIcounter;

void DoSpeedHack(void)
{
	if( CPUNeedToDoOtherTask )	// Let CPU to finish all other tasks before SPEEDHACK
		return;

#ifdef FAST_COUNTER
		Count_Down_All();
#else
	if( (( gHWS_COP0Reg[COUNT] + MAGICNUMBERFORCOUNTREG > gHWS_COP0Reg[COMPARE] ) &&
		(gHWS_COP0Reg[COUNT] < gHWS_COP0Reg[COMPARE] ) ) ||
		((gHWS_COP0Reg[COUNT] > gHWS_COP0Reg[COMPARE] ) &&
		((uint32)(gHWS_COP0Reg[COUNT] + MAGICNUMBERFORCOUNTREG) <= gHWS_COP0Reg[COMPARE])))
	{
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

extern void TriggerFPUUnusableException(void);
void COP1_instr(uint32 Instruction);
void Do_COP1_with_exception(uint32 Instruction)
{
	if( (gHWS_COP0Reg[STATUS] & STATUS_CU1) == 0 )	// CPU1 is not usable
	{
		TriggerFPUUnusableException();
		COP1_instr(Instruction);
	}
	else
	{
		COP1_instr(Instruction);
		//dyna_instruction[0x11] = dyna4300i_cop1;
	}
}


#ifdef DYNA_GET_BLOCK_SPEEDUP
extern uint32 g_translatepc;
extern uint32* g_LookupPtr;
extern uint32 g_pc_is_rdram;
void Try_Lookup_Ptr(void)
{
	uint8* tempblock;
	__try{
		tempblock = (uint8*)*g_LookupPtr;
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER)
	{
#ifdef DYNAEXECUTIONMAPMEMORY
		if( UnmappedMemoryExceptionHelper(g_translatepc) )
		{
			g_LookupPtr = (uint32*)((uint8*)sDYN_PC_LOOKUP[g_translatepc>>16] + (uint16)g_translatepc);
		}
		else
#endif
			DisplayError("Execution from unmapped address, PC=%08X", gHWS_pc);
	}
}

void Set_Translate_PC()
{
	if( NOT_IN_KO_K1_SEG(gHWS_pc))
		g_translatepc = TranslateITLBAddress(gHWS_pc);
	else
		g_translatepc = gHWS_pc;

	if( (g_translatepc & 0x1FFFFFFF) < current_rdram_size )	
		g_pc_is_rdram = g_translatepc&0x007FFFFF;	
	else
		g_pc_is_rdram = 0;

	g_LookupPtr = (uint32*)((uint8*)sDYN_PC_LOOKUP[g_translatepc>>16] + (uint16)g_translatepc);
	Try_Lookup_Ptr();
}
#endif