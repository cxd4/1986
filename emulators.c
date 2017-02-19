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
*/

#include <windows.h>
#include <process.h>
#include "globals.h"
#include "dynarec/dynarec.h"
#include "interrupt.h"
#include "r4300i.h"
#include "win32/DLL_Video.h"
#ifdef WINDEBUG_1964
#include "win32/windebug.h"
#endif


void RunTheInterpreter();
void (__cdecl RunTheDynamicRecompiler)(void *pVoid);
void RunTheRegCache();

#ifdef DEBUG_COMMON
void DebugPrintInstruction(uint32 instruction);
#endif

//---------------------------------------------------------------------------------------

DWORD   CPUThreadID;
HANDLE  CPUThreadHandle = NULL;

extern HWND hwnd;
extern void (*CPU_instruction[64])(uint32 Instruction);
extern uint32 SetException_Interrupt(uint32 pc);
extern GFX_INFO	Gfx_Info;

//------------------------------------------------------------------------------------------------------------------------------

#define VI_INTERRUPTCOUNT	625000
#define MY_THREAD_PRIORITY	THREAD_PRIORITY_HIGHEST

void RunEmulator(uint32 WhichCore)
{
	WhichCore = 1;
	switch (WhichCore)
	{
		case 0 : CPUThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunTheInterpreter,       NULL, 0, &CPUThreadID); break;
		case 1 : CPUThreadHandle = (HANDLE) _beginthread(RunTheDynamicRecompiler, 0, NULL); break;
	}
}

//------------------------------------------------------------------------------------------------------------------------------

void RunTheInterpreter()
{
	uint32 Instruction;
	
	SetThreadPriority(CPUThreadHandle, MY_THREAD_PRIORITY);

	VIDEO_RomOpen();

_NSAKEY:

		Instruction = LOAD_UWORD_PARAM(pc);
		CPU_instruction[_OPCODE_](Instruction);


#ifdef DEBUG_COMMON
		HandleBreakpoint(Instruction);	if (DebuggerEnabled){	DebugPrintInstruction(Instruction);	RefreshDebugger(); }
#endif

		switch (CPUdelay)
		{
			case 0 :	pc += 4;							break;
			case 1 :	pc += 4;			CPUdelay = 2;	break;
			default:	pc = CPUdelayPC;	CPUdelay = 0;	break;
		}

		
	COP0Reg[COUNT]++;

	// check for the vi interrupt
	if (COP0Reg[COUNT] >= VI_INTERRUPTCOUNT)
		Trigger_VIInterrupt();

	// check for pending interrupts
	if ((COP0Reg[CAUSE] & COP0Reg[STATUS] & 0x0000FF00) != 0) {
		if (((COP0Reg[STATUS] & 0x00000006)) == 0)
			if (((COP0Reg[STATUS] & 0x00000001)) == 1)
				pc = SetException_Interrupt(pc);	
	}		
goto _NSAKEY;
}

//------------------------------------------------------------------------------------------------------------------------------

extern void DynCheckForTimedEvents(uint32 cnt);
extern void SetTLBMissException();

void (__cdecl RunTheDynamicRecompiler)(void *pVoid)
{
	static uint32 TempPC;
	static uint32 cnt = 0;
	uint32* LookupPtr;
	uint32* Label;
	uint32 Instruction;	
	static uint8* Dest;

	SetThreadPriority(CPUThreadHandle, MY_THREAD_PRIORITY);

	VIDEO_RomOpen();

	Dest = (uint8*)VirtualAlloc(NULL, 16*1024*1024, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	LocationJumpedFrom = 0;


_FDA:
	if (TempPC != pc)
	{
		TempPC = pc;
		LookupPtr = (uint32*)((uint8*)sDYN_PC_LOOKUP[TempPC>>16] + (uint16)TempPC);
		Block = (uint8*)*LookupPtr;
		if (Block == NULL)
		{
			Block = Dest;
			KEEP_RECOMPILING = 1;
			InstructionPointer = (uint32*)((uint8*)sDWORD_R[(TempPC >> 16)] + ((uint16)TempPC));
			*LookupPtr = (uint32)Dest;
			cnt = 0;
			
			INCREMENT_COUNT_VAL(Dest);

			FetchOpcode
			cnt+=1;
			while(KEEP_RECOMPILING)
			{
				pc += 4; cnt += 1;
				InstructionPointer = (uint32*)((uint8*)sDWORD_R[(pc >> 16)] + ((uint16)pc));
				FetchOpcode
			}
		}
		pc = TempPC;
	}
	*Label = cnt;

#ifdef ADDRESS_RESOLUTION
	if (LocationJumpedFrom != 0)	{	*LocationJumpedFrom = (uint32)Block;	LocationJumpedFrom = 0;	}
#endif
	if ((( COP0Reg[STATUS] & IE )) != 0) Dest = Dyn_Intr_Common(Dest);
	if ( COP0Reg[COUNT] >= 625000){ Trigger_VIInterrupt();}

	__asm call Block

goto _FDA;

	_endthread();
}