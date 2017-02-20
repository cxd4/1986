/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Emulator for Nintendo 64 console system                              |
 |  Copyright (C) 2001  Joel Middendorf  schibo@emuhq.com                       |
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
#include <stdio.h>
#include "globals.h"
#include "options.h"
#include "debug_option.h"
#include "dynarec/dynarec.h"
//#include "regcache/regcache.h"
#include "emulator.h"
#include "interrupt.h"
#include "r4300i.h"
#include "n64rcp.h"
#include "dma.h"
#include "timer.h"
#include "memory.h"
#include "win32/DLL_Video.h"
#include "win32/DLL_Audio.h"
#include "win32/windebug.h"
#include "dynarec/regcache.h"
#include "dynarec/x86.h"

/*
#include "dynarec/dynaBranch.h"
#include "dynarec/dynaCOP1.h"
#include "dynarec/dynaCPU.h"
#include "dynarec/dynaCPU_defines.h"
#include "dynarec/dynaLog.h"
#include "dynarec/dynaHelper.h"
#include "dynarec/dynaRec.h"


*/

#ifdef WINDEBUG_1964
#include "win32/windebug.h"
void DebugPrintInstruction(uint32 instruction);
#endif

//void RunTheInterpreter();
void (__cdecl RunTheInterpreter)(void *pVoid);
void (__cdecl RunTheRegCache)(void *pVoid);
uint8* (__cdecl StaticRecompiler)(uint8 *Dest);
void rc_Intr_Common(void);
void Dyna_Exception_Service_Routine(void);

extern BOOL Rom_Loaded;
//---------------------------------------------------------------------------------------

DWORD   CPUThreadID;
HANDLE  CPUThreadHandle = NULL;
uint32	sp_hle_task=0;
int		sp_task_counter=0;
int		si_io_counter = 0;
BOOL    CPUNeedToDoOtherTask = FALSE;
BOOL	CPUNeedToCheckException = FALSE;
BOOL	CPUNeedToCheckInterrupt = FALSE;
uint32	SavedCOUNT=0;
uint32	SavedCOUNT2=0;
uint32	SavedCOUNT3=0;
static  int tocount;
uint32  VIcounter;

uint32  instCount=0;

extern HWND hwnd;
extern void RunSPTask(void);
extern void (*CPU_instruction[64])(uint32 Instruction);
//extern void Trigger_VIInterrupt(void);
extern uint32 SetException_Interrupt(uint32 pc);
extern GFX_INFO Gfx_Info;
extern int AiUpdating;

//------------------------------------------------------------------------------------------------------------------------------

//#define MY_THREAD_PRIORITY  THREAD_PRIORITY_HIGHEST
#ifdef DEBUG_COMMON
	// Use normal priority for easy debug
#define MY_THREAD_PRIORITY THREAD_PRIORITY_NORMAL
#else
//#define MY_THREAD_PRIORITY THREAD_PRIORITY_ABOVE_NORMAL
#define MY_THREAD_PRIORITY THREAD_PRIORITY_NORMAL
#endif

void RunEmulator(uint32 core)
{
	if( !Rom_Loaded )
	{
		return;
	}

    switch (core)
    {
        case 0 : 
			CPUThreadHandle = (HANDLE) _beginthread(RunTheInterpreter, 0, NULL);
			break;
        case 2 : 
			CPUThreadHandle = (HANDLE) _beginthread(RunTheRegCache, 0, NULL); 
			break;
    }
}


//------------------------------------------------------------------------------------------------------------------------------
void (__cdecl RunTheInterpreter)(void *pVoid)
{
    uint32 Instruction;
	uint32 translatepc;


//    SetThreadPriority(CPUThreadHandle, MY_THREAD_PRIORITY);
    DWORD dwVersion = GetVersion();
    
    if (dwVersion < 0x80000000)                // Windows NT
        SetThreadPriority(CPUThreadHandle, THREAD_PRIORITY_NORMAL);
    else
        SetThreadPriority(CPUThreadHandle, THREAD_PRIORITY_HIGHEST);   


	SavedCOUNT = 0;

	
	SavedCOUNT2=0;
	SavedCOUNT3=0;
	VIcounter = 0;

	LocationJumpedFrom = 0;
	Init_Dynarec();				// We can hot switch from Intepreter mode to Dyna mode

	tocount = 0;
	memcpy(&HeaderDllPass[0], &ROM_Image[0], 0x40);
    VIDEO_RomOpen();
    //AUDIO_RomOpen();

_NSAKEY:
	//What if load from 0xbfc00000 ? (example)
    if ((gHardwareState.pc ^ 0x80000000) & 0xC0000000 ) 
	{                                                   
		translatepc = TranslateITLBAddress(gHardwareState.pc);
		
        if( gHardwareState.COP0Reg[CAUSE] & TLBL_Miss )
		{
			gHardwareState.pc = SetException_Interrupt(gHardwareState.pc);

            //get instruction            
            __try
			{
                    Instruction = *(uint32*)(valloc+((gHardwareState.pc)&0x1fffffff));
            }
			__except(NULL,EXCEPTION_EXECUTE_HANDLER)
			{
                __try
				{
                    Instruction = *(uint32*)(valloc2+((gHardwareState.pc)&0x1fffffff));
                }
				__except(NULL,EXCEPTION_EXECUTE_HANDLER)
				{
                    DisplayError("%08X: Instruction2 out of range", gHardwareState.pc);
                }
            }

		}
		else
		{
			//get instruction
			__try
			{
				Instruction = *(uint32*)(valloc+((translatepc)&0x1fffffff));
			}
			__except(NULL,EXCEPTION_EXECUTE_HANDLER)
			{
				__try
				{
	                Instruction = *(uint32*)(valloc2+((translatepc)&0x1fffffff));
		        }
				__except(NULL,EXCEPTION_EXECUTE_HANDLER)
				{
			        DisplayError("%08X: Instruction1 out of range", translatepc);

					// Trigger the Address Error Load Exception
					gHardwareState.COP0Reg[BADVADDR] = gHardwareState.pc;
					gHardwareState.COP0Reg[CAUSE] |= EXC_RADE;
#ifndef CPUCHECKINTR
					CPUNeedToDoOtherTask = TRUE;
					CPUNeedToCheckException = TRUE;
#endif
					
					gHardwareState.pc = SetException_Interrupt(gHardwareState.pc);

					//get instruction            
					__try
					{
							Instruction = *(uint32*)(valloc+((gHardwareState.pc)&0x1fffffff));
					}
					__except(NULL,EXCEPTION_EXECUTE_HANDLER)
					{
						__try
						{
							Instruction = *(uint32*)(valloc2+((gHardwareState.pc)&0x1fffffff));
						}
						__except(NULL,EXCEPTION_EXECUTE_HANDLER)
						{
							DisplayError("%08X: Instruction2 out of range", gHardwareState.pc);
						}
					}

				}
			}
		}
	}
	else
	{
        //get instruction            
        __try
		{
                Instruction = *(uint32*)(valloc+((gHardwareState.pc)&0x1fffffff));
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
            __try
			{
                Instruction = *(uint32*)(valloc2+((gHardwareState.pc)&0x1fffffff));
            }
			__except(NULL,EXCEPTION_EXECUTE_HANDLER)
			{
                DisplayError("%08X: Instruction3 out of range", gHardwareState.pc);

					// Trigger the Address Error Load Exception
					gHardwareState.COP0Reg[BADVADDR] = gHardwareState.pc;
					gHardwareState.COP0Reg[CAUSE] |= EXC_RADE;
#ifndef CPUCHECKINTR
					CPUNeedToDoOtherTask = TRUE;
					CPUNeedToCheckException = TRUE;
#endif
					
					gHardwareState.pc = SetException_Interrupt(gHardwareState.pc);

					//get instruction            
					__try
					{
							Instruction = *(uint32*)(valloc+((gHardwareState.pc)&0x1fffffff));
					}
					__except(NULL,EXCEPTION_EXECUTE_HANDLER)
					{
						__try
						{
							Instruction = *(uint32*)(valloc2+((gHardwareState.pc)&0x1fffffff));
						}
						__except(NULL,EXCEPTION_EXECUTE_HANDLER)
						{
							DisplayError("%08X: Instruction2 out of range", gHardwareState.pc);
						}
					}
			}
        }
	}

    CPU_instruction[_OPCODE_](Instruction);
    gHardwareState.GPR[0] = 0;


#ifdef DEBUG_COMMON
	if( DebuggerActive )
	{
		HandleBreakpoint(Instruction);
		if (DebuggerEnabled)
		{   
			DebugPrintInstruction(Instruction); 
			RefreshDebugger(); 
		}
	}
#endif

    switch (CPUdelay)
    {
        case 0 :    gHardwareState.pc += 4;                         break;
        case 1 :    gHardwareState.pc += 4;         CPUdelay = 2;   break;
        default:    gHardwareState.pc = CPUdelayPC; CPUdelay = 0;   break;
    }

    // check for the vi interrupt
#ifdef SAVEVICOUNTER
	if( VIcounter >= MAGICNUMBER )
	{
		VIcounter=0;
		Trigger_VIInterrupt();
	}
#else
	if( gHardwareState.COP0Reg[COUNT]-SavedCOUNT >= MAGICNUMBERFORCOUNTREG )
	{
		SavedCOUNT = gHardwareState.COP0Reg[COUNT];
        Trigger_VIInterrupt();
	}
#endif
	
	
	instCount++;
	
#ifdef DEBUG_CPU_COUNTER
	if( debug_cpu_counter && gHardwareState.COP0Reg[COUNT] % 500000 == 0 )
	{
		sprintf(generalmessage, "COUNT = %08X, COMPARE= %08X, PC = %08X", gHardwareState.COP0Reg[COUNT], gHardwareState.COP0Reg[COMPARE], gHardwareState.pc);
		RefreshOpList(generalmessage);
	}
#endif

#ifdef SAVECPUCOUNTER
    gHardwareState.COP0Reg[COUNT] += tocount;
	tocount = 1-tocount;
#else
	gHardwareState.COP0Reg[COUNT]++;
#endif

#ifdef SAVEVICOUNTER
	VIcounter++;
#endif

	if (gHardwareState.COP0Reg[COUNT] == gHardwareState.COP0Reg[COMPARE] )
	{
		// Trigger count interrupt
		// Counter interrupt is very important to do
		// It makes a lot of games to work
		Trigger_CompareInterrupt();
	}

	if( CPUNeedToDoOtherTask )	
		CPUDoOtherTasks();

#ifndef CPUCHECKINTR
    // check for pending interrupts
    if ((gHardwareState.COP0Reg[STATUS] & 0x00000006 /*0x00000004*/) == 0 &&	// No in another interrupt routine
		((gHardwareState.COP0Reg[CAUSE] & 0x0000007C)	||		// Is there a exception ?
		((gHardwareState.COP0Reg[STATUS] & 0x00000001) != 0 &&	// Interrupts are enabled
		((gHardwareState.COP0Reg[CAUSE] & gHardwareState.COP0Reg[STATUS] & 0x0000FF00) != 0) )))
	{	// If there are interrupts and interrupts are enabled in the interrupt masks
		//if (((gHardwareState.COP0Reg[STATUS] & 0x00000006)) == 0)
			// If there is not either Error or Exceptions happens already
            //if (((gHardwareState.COP0Reg[STATUS] & 0x00000001)) == 1)
				// if the interrupt processing is enabled
                gHardwareState.pc = SetException_Interrupt(gHardwareState.pc);
    }
#else
	//CPU will check interrupts in the CPUDoOtherTasks() function
#endif

	if( whichcore == 2 ) //switch to Dyna mode
	{
		Dyna_Exception_Service_Routine();
	}

goto _NSAKEY;

    _endthread();
}

//------------------------------------------------------------------------------------------------------------------------------
void CheckCPUOtherTasks(void)
{
	if(CPUNeedToDoOtherTask)
		CPUDoOtherTasks();
}


uint32 cp0Counter;    
extern dyn_cpu_instr dyna_instruction[64];
extern unsigned long	lCodePosition;


void (__cdecl RunTheRegCache)(void *pVoid)
{
    static uint32 TempPC;
    uint32* LookupPtr;
    uint32 Instruction; 
	uint32 translatepc;

	int i;

//    SetThreadPriority(CPUThreadHandle, MY_THREAD_PRIORITY);
    DWORD dwVersion = GetVersion();
	HardwareState* reg = &gHardwareState;
    
    if (dwVersion < 0x80000000)                // Windows NT
        SetThreadPriority(CPUThreadHandle, THREAD_PRIORITY_NORMAL);
    else
        SetThreadPriority(CPUThreadHandle, THREAD_PRIORITY_HIGHEST);   
    
	SavedCOUNT=0;
	SavedCOUNT2=0;
	SavedCOUNT3=0;
	VIcounter = 0;

	memcpy(&HeaderDllPass[0], &ROM_Image[0], 0x40);
    VIDEO_RomOpen();
//	AUDIO_RomOpen();

    LocationJumpedFrom = 0;
    Init_Dynarec(); 

_FDA:

    if (TempPC != gHardwareState.pc)
    {


		if ((gHardwareState.pc ^ 0x80000000) & 0xC0000000 ) 
		{                                                   
			translatepc = TranslateITLBAddress(gHardwareState.pc);
			if( gHardwareState.COP0Reg[CAUSE] & TLBL_Miss )
				goto INTR_EXP;

			TempPC = translatepc;
		}
		else
	        TempPC = gHardwareState.pc;
        
		LookupPtr = (uint32*)((uint8*)sDYN_PC_LOOKUP[TempPC>>16] + (uint16)TempPC);
		Block = (uint8*)*LookupPtr;

        if (Block == NULL)	// Check if this block of N64 code has been compiled before
        {
			// No, this block has never been compiled, so need to compile it
            Block = &dyna_RecompCode[lCodePosition];
            KEEP_RECOMPILING = 1;

            //get instruction            
            __try{
                pcptr = (uint32 *)(valloc+((TempPC)&0x1fffffff));
                Instruction = *pcptr;
            }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                __try{
                    pcptr = (uint32 *)(valloc2+((TempPC)&0x1fffffff));
                    Instruction = *pcptr;
                }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                    DisplayError("%08X: tempPC out of range", TempPC);
                }
            }

		// Need more work in this part, a lot of game need to use TLB to access instructions

			*LookupPtr = (_u32)Block;
            cp0Counter = 0;

            //RegFetchOpcode
#ifdef DEBUG_COMMON
			if(DebuggerActive)
			{
                rc_DYNDEBUG_UPDATE
                DEBUG_BPT
			}
#endif

            gHardwareState.code = Instruction;
            dyna_instruction[_OPCODE_](&gHardwareState);


            cp0Counter++;
            while(KEEP_RECOMPILING)
            {
				gHardwareState.pc += 4; 
				cp0Counter++;

				// Do TLB for instruction
				if ((gHardwareState.pc ^ 0x80000000) & 0xC0000000 ) 
				{                                                   
                    translatepc = TranslateITLBAddress(gHardwareState.pc);

                    //get instruction            
                    __try{
                        pcptr = (uint32 *)(valloc+((translatepc)&0x1fffffff));
                        Instruction = *pcptr;
                    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                        __try{
                            pcptr = (uint32 *)(valloc2+((translatepc)&0x1fffffff));
                            Instruction = *pcptr;
                        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                            DisplayError("%08X: pcptr1 out of range", translatepc);
                        }
                    }

    			}
				else
				{
                    //get instruction            
                    __try{
                        pcptr = (uint32 *)(valloc+((gHardwareState.pc)&0x1fffffff));
                        Instruction = *pcptr;
                    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                        __try{
                            pcptr = (uint32 *)(valloc2+((gHardwareState.pc)&0x1fffffff));
                            Instruction = *pcptr;
                        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                            DisplayError("%08X: pcptr2 out of range", gHardwareState.pc);
                        }
                    }


				}

				if( gHardwareState.COP0Reg[CAUSE] & TLBL_Miss )
				{
#ifdef DEBUG_COMMON
					if(DebuggerActive)
					{
						rc_DYNDEBUG_UPDATE
						DEBUG_BPT
					}
#endif
					gHardwareState.code = Instruction;
					dyna_instruction[_OPCODE_](&gHardwareState);

#ifdef DYNCPUCHECKINTRAFTEREACHINSTRUCTION
					// This section will check CPUTask at each opcode, this will decrease
					// overall speed of emu, from 60 fps to 54 fps

					//TEST_ImmWithMemory(&CPUNeedToDoOtherTask,1);
					//Jcc_Near_auto(CC_NE, 0x0c); //jmp false
					//INTERPRET_LOADSTORE(CPUDoOtherTasks);
					INTERPRET_LOADSTORE(CheckCPUOtherTasks);
#endif
					
				}
				else
                {
#ifdef DEBUG_COMMON
					if(DebuggerActive)
					{
						rc_DYNDEBUG_UPDATE
						DEBUG_BPT
					}
#endif
					gHardwareState.code = Instruction;
					dyna_instruction[_OPCODE_](&gHardwareState);

					
#ifdef DYNCPUCHECKINTRAFTEREACHINSTRUCTION
					// This section will check CPUTask at each opcode, this will decrease
					// overall speed of emu, from 60 fps to 54 fps
					//TEST_ImmWithMemory(&CPUNeedToDoOtherTask,1);
					//Jcc_Near_auto(CC_NE, 0x0c); //jmp false
					//INTERPRET_LOADSTORE(CPUDoOtherTasks);
					INTERPRET_LOADSTORE(CheckCPUOtherTasks);
#endif
					
                }
            }

#ifdef DEBUG_DYNA
			if( debug_dyna_compiler )
			{
				sprintf(generalmessage, "Dyna compile: memory %08X - %08X", TempPC, gHardwareState.pc);
				RefreshOpList(generalmessage);
			}
#endif
        }
        gHardwareState.pc = TempPC;
    }

#ifdef ADDRESS_RESOLUTION
    if (LocationJumpedFrom != 0)    
	{   
		*LocationJumpedFrom = (uint32)Block;    
		LocationJumpedFrom = 0; 
	}
#endif

	
#ifdef DEBUG_CPU_COUNTER
	if( debug_cpu_counter && gHardwareState.COP0Reg[COUNT] / 500000 > SavedCOUNT3 / 500000 )
	{
		sprintf(generalmessage, "COUNT = %08X, COMPARE= %08X, PC = %08X", gHardwareState.COP0Reg[COUNT], gHardwareState.COP0Reg[COMPARE], gHardwareState.pc);
		RefreshOpList(generalmessage);
		SavedCOUNT3 = gHardwareState.COP0Reg[COUNT];
	}
#endif

#ifdef FASTERCOMPARE
	if ( SavedCOUNT < gHardwareState.COP0Reg[COMPARE] && gHardwareState.COP0Reg[COUNT] > gHardwareState.COP0Reg[COMPARE]  )
#else
	if ( (gHardwareState.COP0Reg[COUNT] > gHardwareState.COP0Reg[COMPARE] && SavedCOUNT < gHardwareState.COP0Reg[COMPARE] ) ||
		 (gHardwareState.COP0Reg[COUNT] > gHardwareState.COP0Reg[COMPARE] && gHardwareState.COP0Reg[COUNT] < SavedCOUNT ))
#endif
	{
		Trigger_CompareInterrupt();
		SavedCOUNT = gHardwareState.COP0Reg[COUNT];
	}

	
#ifdef SAVEVICOUNTER
	if( VIcounter >= MAGICNUMBER )
	{
        Trigger_VIInterrupt();
		VIcounter = 0;
	}
#else
	if( gHardwareState.COP0Reg[COUNT]-SavedCOUNT >= MAGICNUMBER )
	{
		SavedCOUNT = gHardwareState.COP0Reg[COUNT];
        Trigger_VIInterrupt();
	}
#endif
    
INTR_EXP:
#ifndef DYNCPUCHECKINTR
    if ((gHardwareState.COP0Reg[STATUS] & 0x00000006) == 0 &&	// No in another interrupt routine
		((gHardwareState.COP0Reg[CAUSE] & 0x0000007C)	||		// Is there a exception ?
		((gHardwareState.COP0Reg[STATUS] & 0x00000001) != 0 &&	// Interrupts are enabled
		((gHardwareState.COP0Reg[CAUSE] & gHardwareState.COP0Reg[STATUS] & 0x0000FF00) != 0) )))

	{
        rc_Intr_Common();
	}
#endif

	if( CPUNeedToDoOtherTask )	
	{
		for( i=0; i<abs(gHardwareState.COP0Reg[COUNT] - SavedCOUNT2); i+=10)
		{
			if( CPUNeedToDoOtherTask )
				CPUDoOtherTasks();
			else
				break;
		}
	}

	SavedCOUNT2 = gHardwareState.COP0Reg[COUNT];

#ifdef DEBUG_DYNAEXECUTION
			if( debug_dyna_execution )
			{
				sprintf(generalmessage, "Dyna execution: PC = %08X", gHardwareState.pc);
				RefreshOpList(generalmessage);
			}
#endif

	// Run the compiled code in the Block
	__asm pushad
	__asm call Block
	__asm popad

goto _FDA;

    Free_Dynarec();
    _endthread();
}




extern void DynCheckForTimedEvents(uint32 cnt);
extern void SetTLBMissException();
extern int WasFlushed;
//------------------------------------------------------------------------------------------------------------------------------

void CPUDoOtherTasks(void)
{
	if( sp_task_counter > 0 )
	{
		sp_task_counter -= SPCOUNTERTOINCREASE;		// Transfer one byte per instruction
		if( sp_task_counter <= 0 )
			RunSPTask();
		//else
			//return;		// SP Task is not finished yet, CPU will check it after next instruction
	}
	// to get here, SP task in finished


#ifdef DODMASEGMENT
	// DMA segmentation is very important, it is for timing issues.
	// DMA segmentation makes a lot of games boot and work
	// DMA transfer has to be in front of SP, otherwise Zelda game stop working
	if(DMAInProgress)
	{
		DoDMASegment();
		//return;
	}
	// To get here, DMA task is finished
#endif

#ifdef DOSIIODMASEGMENT
	if( si_io_counter > 0 )
	{
		si_io_counter -= 4;
		if( si_io_counter <= 0 )
		{
			si_io_counter = 0;
			SI_STATUS_REG &= ~SI_STATUS_RD_BUSY;
		}
	}
#endif

#ifdef CPUCHECKINTR
//#ifdef DYNCPUCHECKINTR
	if( whichcore == 0 )	// intepreter mode
//#endif
	{
		if( CPUNeedToCheckException)
		{
			if ((gHardwareState.COP0Reg[STATUS] & 0x00000006 ) == 0 &&	// No in another interrupt routine
				(gHardwareState.COP0Reg[CAUSE] & 0x0000007C) )	// Is there a exception ?
			{
				gHardwareState.pc = SetException_Interrupt(gHardwareState.pc);
				CPUNeedToCheckException = FALSE;
			}
		}
		else if( CPUNeedToCheckInterrupt )
		{
			if ((gHardwareState.COP0Reg[STATUS] & 0x00000006 /*0x00000004*/) == 0 &&	// No in another interrupt routine
				((gHardwareState.COP0Reg[STATUS] & 0x00000001) != 0 &&	// Interrupts are enabled
				((gHardwareState.COP0Reg[CAUSE] & gHardwareState.COP0Reg[STATUS] & 0x0000FF00) != 0) ))
			{	// If there are interrupts and interrupts are enabled in the interrupt masks
				//if (((gHardwareState.COP0Reg[STATUS] & 0x00000006)) == 0)
					// If there is not either Error or Exceptions happens already
					//if (((gHardwareState.COP0Reg[STATUS] & 0x00000001)) == 1)
						// if the interrupt processing is enabled
				gHardwareState.pc = SetException_Interrupt(gHardwareState.pc);
				CPUNeedToCheckInterrupt = FALSE;
			}
		}
	}
	else	// Dyna mode
#ifdef DYNCPUCHECKINTR
	{
		if( CPUNeedToCheckException )
		{
			if ((gHardwareState.COP0Reg[STATUS] & 0x00000006 ) == 0 &&	// No in another interrupt routine
				(gHardwareState.COP0Reg[CAUSE] & 0x0000007C) )	// Is there a exception ?
			{
				Dyna_Exception_Service_Routine();
				//rc_Intr_Common();
				//CPUNeedToCheckException = FALSE;
			}
		}
		else if( CPUNeedToCheckInterrupt )
		{
			if ((gHardwareState.COP0Reg[STATUS] & 0x00000006 ) == 0 &&	// No in another interrupt routine
				((gHardwareState.COP0Reg[STATUS] & 0x00000001) != 0 &&	// Interrupts are enabled
				((gHardwareState.COP0Reg[CAUSE] & gHardwareState.COP0Reg[STATUS] & 0x0000FF00) != 0) ))
			{
				//Dyna_Exception_Service_Routine();
				rc_Intr_Common();
				CPUNeedToCheckInterrupt = FALSE;
			}
		}
	}
#else
	{
		CPUNeedToCheckException = FALSE;
		CPUNeedToCheckInterrupt = FALSE;
	}
#endif

#endif

	// up to here, no CPU task left.
	if( 
		sp_task_counter>0 
#ifdef 	DODMASEGMENT	
		|| DMAInProgress
#endif

#ifdef DOSIIODMASEGMENT
		|| si_io_counter > 0 
#endif

#ifdef CPUCHECKINTR
		|| CPUNeedToCheckException || CPUNeedToCheckInterrupt
#endif
		)
		return;
	CPUNeedToDoOtherTask = FALSE;
}

void ClearCPUTasks(void)
{
	CPUNeedToDoOtherTask = FALSE;
#ifdef DODMASEGMENT
	DMAInProgress = FALSE;
	PIDMAInProgress = NO_DMA_IN_PROGRESS;
	SIDMAInProgress = NO_DMA_IN_PROGRESS;
	SPDMAInProgress = NO_DMA_IN_PROGRESS;
#endif
//#ifdef DOSPTASKCOUNTER
	sp_task_counter = 0;
//#endif
//#ifdef DOSIIODMASEGMENT
	si_io_counter = 0;
//#endif
//#ifdef CPUCHECKINTR
	CPUNeedToCheckException = FALSE;
	CPUNeedToCheckInterrupt = FALSE;
//#endif
}

void StepCPU(void)
{
    uint32 Instruction;
	uint32 translatepc;

	if( whichcore != 0 )
	{
		return;		// StepCPU ony support Interpreter mode
	}


    if ((gHardwareState.pc ^ 0x80000000) & 0xC0000000 ) 
	{                                                   
		translatepc = TranslateITLBAddress(gHardwareState.pc);
		
        if( gHardwareState.COP0Reg[CAUSE] & TLBL_Miss )
		{
			gHardwareState.pc = SetException_Interrupt(gHardwareState.pc);
			

            //get instruction            
            __try{
                    Instruction = *(uint32*)(valloc+((gHardwareState.pc)&0x1fffffff));
            }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                __try{
                    Instruction = *(uint32*)(valloc2+((gHardwareState.pc)&0x1fffffff));
                }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                    DisplayError("%08X: Instruction2 out of range", gHardwareState.pc);
                }
            }

		}
		else
		{
			//get instruction
			__try{
				Instruction = *(uint32*)(valloc+((translatepc)&0x1fffffff));
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				__try
				{
	                Instruction = *(uint32*)(valloc2+((translatepc)&0x1fffffff));
		        }__except(NULL,EXCEPTION_EXECUTE_HANDLER)
				{
			        DisplayError("%08X: Instruction1 out of range", translatepc);
				}
			}
		}
	}
	else
	{
        //get instruction            
        __try
		{
                Instruction = *(uint32*)(valloc+((gHardwareState.pc)&0x1fffffff));
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
            __try
			{
                Instruction = *(uint32*)(valloc2+((gHardwareState.pc)&0x1fffffff));
            }
			__except(NULL,EXCEPTION_EXECUTE_HANDLER)
			{
                DisplayError("%08X: Instruction3 out of range", gHardwareState.pc);

				translatepc = TranslateITLBAddress(gHardwareState.pc);

		        if( gHardwareState.COP0Reg[CAUSE] & TLBL_Miss )
				{
					gHardwareState.pc = SetException_Interrupt(gHardwareState.pc);
				

		            //get instruction            
		           __try
				   {
	                   Instruction = *(uint32*)(valloc+((gHardwareState.pc)&0x1fffffff));
		           }
				   __except(NULL,EXCEPTION_EXECUTE_HANDLER)
				   {
		                __try
						{
		                   Instruction = *(uint32*)(valloc2+((gHardwareState.pc)&0x1fffffff));
		                }
						__except(NULL,EXCEPTION_EXECUTE_HANDLER)
						{
		                   DisplayError("%08X: Instruction5 out of range", gHardwareState.pc);
		                }
					}

				}
				else
				{
					__try
					{
						Instruction = *(uint32*)(valloc+((translatepc)&0x1fffffff));
					}
					__except(NULL,EXCEPTION_EXECUTE_HANDLER)
					{
						__try{
							Instruction = *(uint32*)(valloc2+((translatepc)&0x1fffffff));
						}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
							DisplayError("%08X: Instruction4 out of range", translatepc);
						}
					}
		        }
			}
        }
	}

    CPU_instruction[_OPCODE_](Instruction);
    gHardwareState.GPR[0] = 0;


#ifdef DEBUG_COMMON
	if( DebuggerActive )
	{
		HandleBreakpoint(Instruction);
		DebugPrintInstruction(Instruction); 
		RefreshDebugger(); 
	}
#endif

    switch (CPUdelay)
    {
        case 0 :    gHardwareState.pc += 4;                         break;
        case 1 :    gHardwareState.pc += 4;         CPUdelay = 2;   break;
        default:    gHardwareState.pc = CPUdelayPC; CPUdelay = 0;   break;
    }

    // check for the vi interrupt
	if( gHardwareState.COP0Reg[COUNT]-SavedCOUNT >= /*6250000*/312500 )
	{
		SavedCOUNT = gHardwareState.COP0Reg[COUNT];
        Trigger_VIInterrupt();
	}

#ifdef DEBUG_CPU_COUNTER
	if( gHardwareState.COP0Reg[COUNT] % 500000 == 0)
	{
		sprintf(generalmessage, "COUNT = %08X, COMPARE= %08X, PC = %08X", gHardwareState.COP0Reg[COUNT], gHardwareState.COP0Reg[COMPARE], gHardwareState.pc);
		RefreshOpList(generalmessage);
	}
#endif

    gHardwareState.COP0Reg[COUNT] += tocount;
	tocount = 1-tocount;
	if (abs(gHardwareState.COP0Reg[COUNT] == gHardwareState.COP0Reg[COMPARE]) )
	{
		// Trigger count interrupt
		// Counter interrupt is very important to do
		// It makes a lot of games to work
		Trigger_CompareInterrupt();
	}

	if( CPUNeedToDoOtherTask )	
		CPUDoOtherTasks();

#ifndef CPUCHECKINTR
    // check for pending interrupts
    if ((gHardwareState.COP0Reg[STATUS] & 0x00000006) == 0 &&	// No in another interrupt routine
		((gHardwareState.COP0Reg[CAUSE] & 0x0000007C)	||		// Is there a exception ?
		((gHardwareState.COP0Reg[STATUS] & 0x00000001) != 0 &&	// Interrupts are enabled
		((gHardwareState.COP0Reg[CAUSE] & gHardwareState.COP0Reg[STATUS] & 0x0000FF00) != 0) )))
	{	// If there are interrupts and interrupts are enabled in the interrupt masks
		//if (((gHardwareState.COP0Reg[STATUS] & 0x00000006)) == 0)
			// If there is not either Error or Exceptions happens already
            //if (((gHardwareState.COP0Reg[STATUS] & 0x00000001)) == 1)
				// if the interrupt processing is enabled
                gHardwareState.pc = SetException_Interrupt(gHardwareState.pc);
    }       
#endif
}

// Return the instruction at current PC
// This is a utility function, called by some debug function
uint32 FetchInstruction(void)
{
	uint32 Instruction;
	uint32 translatepc;

    if ((gHardwareState.pc ^ 0x80000000) & 0xC0000000 ) 
	{                                                   
		translatepc = TranslateITLBAddress(gHardwareState.pc);
		
        if( gHardwareState.COP0Reg[CAUSE] & TLBL_Miss )
		{
			gHardwareState.pc = SetException_Interrupt(gHardwareState.pc);
			

            //get instruction            
            __try{
                    Instruction = *(uint32*)(valloc+((gHardwareState.pc)&0x1fffffff));
            }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                __try{
                    Instruction = *(uint32*)(valloc2+((gHardwareState.pc)&0x1fffffff));
                }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                    DisplayError("%08X: Instruction2 out of range", gHardwareState.pc);
                }
            }

		}
		else
		{
			//get instruction
			__try{
				Instruction = *(uint32*)(valloc+((translatepc)&0x1fffffff));
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				__try
				{
	                Instruction = *(uint32*)(valloc2+((translatepc)&0x1fffffff));
		        }__except(NULL,EXCEPTION_EXECUTE_HANDLER)
				{
			        DisplayError("%08X: Instruction1 out of range", translatepc);
				}
			}
		}
	}
	else
	{
        //get instruction            
        __try
		{
                Instruction = *(uint32*)(valloc+((gHardwareState.pc)&0x1fffffff));
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
            __try
			{
                Instruction = *(uint32*)(valloc2+((gHardwareState.pc)&0x1fffffff));
            }
			__except(NULL,EXCEPTION_EXECUTE_HANDLER)
			{
                DisplayError("%08X: Instruction3 out of range", gHardwareState.pc);

				translatepc = TranslateITLBAddress(gHardwareState.pc);

		        if( gHardwareState.COP0Reg[CAUSE] & TLBL_Miss )
				{
					gHardwareState.pc = SetException_Interrupt(gHardwareState.pc);
				

		            //get instruction            
		           __try
				   {
	                   Instruction = *(uint32*)(valloc+((gHardwareState.pc)&0x1fffffff));
		           }
				   __except(NULL,EXCEPTION_EXECUTE_HANDLER)
				   {
		                __try
						{
		                   Instruction = *(uint32*)(valloc2+((gHardwareState.pc)&0x1fffffff));
		                }
						__except(NULL,EXCEPTION_EXECUTE_HANDLER)
						{
		                   DisplayError("%08X: Instruction5 out of range", gHardwareState.pc);
		                }
					}

				}
				else
				{
					__try
					{
						Instruction = *(uint32*)(valloc+((translatepc)&0x1fffffff));
					}
					__except(NULL,EXCEPTION_EXECUTE_HANDLER)
					{
						__try{
							Instruction = *(uint32*)(valloc2+((translatepc)&0x1fffffff));
						}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
							DisplayError("%08X: Instruction4 out of range", translatepc);
						}
					}
		        }
			}
        }
	}

	return Instruction;

}


// This function is not usable yet
BOOL finish_dyna_exception = TRUE;
extern int AlreadyRecompiled;
extern void rc_RecompileIVT();
extern uint8* IVTStart;
// This function is called when there is an interrupt or exception fires.
// This is good try, but to put the interrupt or exception service into a function does not work
// because we do not know when the service will finish.
// It turns out that service usually does not finish with ERET opcode
void Dyna_Exception_Service_Routine()
{
    uint32 TempPC=0;
    uint32* LookupPtr;
    uint32 Instruction; 
	uint32 translatepc;
	int i;
	
	HardwareState* reg = &gHardwareState;
	uint8* SavedBlock = Block;

	if ((( gHardwareState.COP0Reg[STATUS] & EXL_OR_ERL  )) != 0 )
	{
		// Do nothing if ERL(Error) and EXL(Exception) happens together
		// This should never happen
		return;
	}

    gHardwareState.COP0Reg[EPC] = gHardwareState.pc;
    gHardwareState.COP0Reg[STATUS] |= EXL;          // set EXL = 1 

    if (!AlreadyRecompiled)
    {
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
	finish_dyna_exception = FALSE;

	__asm pushad
	__asm call Block
	__asm popad

#ifdef DEBUG_COMMON
	sprintf(generalmessage, "Start Exception Service in Dyna");
	RefreshOpList(generalmessage);
#endif

	while( whichcore ==2 /*finish_dyna_exception == FALSE */)
	{

    if (TempPC != gHardwareState.pc)
    {
        TempPC = gHardwareState.pc;
        LookupPtr = (uint32*)((uint8*)sDYN_PC_LOOKUP[TempPC>>16] + (uint16)TempPC);
		Block = (uint8*)*LookupPtr;

        if (Block == NULL)	// Check if this block of N64 code has been compiled before
        {
			// No, this block has never been compiled, so need to compile it
            Block = &dyna_RecompCode[lCodePosition];
            KEEP_RECOMPILING = 1;
                    //get instruction            
                    __try{
                        pcptr = (uint32 *)(valloc+((TempPC)&0x1fffffff));
                        Instruction = *pcptr;
                    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                        __try{
                            pcptr = (uint32 *)(valloc2+((TempPC)&0x1fffffff));
                            Instruction = *pcptr;
                        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                            DisplayError("%08X: tempPC out of range", TempPC);
                        }
                    }

			*LookupPtr = (_u32)Block;
            cp0Counter = 0;

            //RegFetchOpcode
#ifdef DEBUG_COMMON
			if(DebuggerActive)
			{
                rc_DYNDEBUG_UPDATE
                DEBUG_BPT
			}
#endif
            gHardwareState.code = Instruction;
            dyna_instruction[_OPCODE_](&gHardwareState);

            cp0Counter++;
            while(KEEP_RECOMPILING)
            {
				gHardwareState.pc += 4; 
				cp0Counter++;

				// Do TLB for instruction
				if ((gHardwareState.pc ^ 0x80000000) & 0xC0000000 ) 
				{                                                   
                    translatepc = TranslateITLBAddress(gHardwareState.pc);

                    //get instruction            
                    __try{
                        pcptr = (uint32 *)(valloc+((translatepc)&0x1fffffff));
                        Instruction = *pcptr;
                    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                        __try{
                            pcptr = (uint32 *)(valloc2+((translatepc)&0x1fffffff));
                            Instruction = *pcptr;
                        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                            DisplayError("%08X: pcptr1 out of range", translatepc);
                        }
                    }

    			}
				else
				{
                    //get instruction            
                    __try{
                        pcptr = (uint32 *)(valloc+((gHardwareState.pc)&0x1fffffff));
                        Instruction = *pcptr;
                    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                        __try{
                            pcptr = (uint32 *)(valloc2+((gHardwareState.pc)&0x1fffffff));
                            Instruction = *pcptr;
                        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                            DisplayError("%08X: pcptr2 out of range", gHardwareState.pc);
                        }
                    }


				}

				if( gHardwareState.COP0Reg[CAUSE] & TLBL_Miss )
				{
#ifdef DEBUG_COMMON
					if(DebuggerActive)
					{
						rc_DYNDEBUG_UPDATE
						DEBUG_BPT
					}
#endif
					gHardwareState.code = Instruction;
					dyna_instruction[_OPCODE_](&gHardwareState);
				}
				else
                {
#ifdef DEBUG_COMMON
					if(DebuggerActive)
					{
						rc_DYNDEBUG_UPDATE
						DEBUG_BPT
					}
#endif
					gHardwareState.code = Instruction;
					dyna_instruction[_OPCODE_](&gHardwareState);
                }
            }
        }
        gHardwareState.pc = TempPC;
    }

#ifdef ADDRESS_RESOLUTION
    if (LocationJumpedFrom != 0)    
	{   
		*LocationJumpedFrom = (uint32)Block;    
		LocationJumpedFrom = 0; 
	}
#endif

	
#ifdef DEBUG_CPU_COUNTER
	if( debug_cpu_counter && gHardwareState.COP0Reg[COUNT] / 500000 > SavedCOUNT3 / 500000 )
	{
		sprintf(generalmessage, "COUNT = %08X, COMPARE= %08X, PC = %08X", gHardwareState.COP0Reg[COUNT], gHardwareState.COP0Reg[COMPARE], gHardwareState.pc);
		RefreshOpList(generalmessage);
		SavedCOUNT3 = gHardwareState.COP0Reg[COUNT];
	}
#endif

	if ( (gHardwareState.COP0Reg[COUNT] > gHardwareState.COP0Reg[COMPARE] && SavedCOUNT < gHardwareState.COP0Reg[COMPARE] ) ||
		 (gHardwareState.COP0Reg[COUNT] > gHardwareState.COP0Reg[COMPARE] && gHardwareState.COP0Reg[COUNT] < SavedCOUNT ))
	{
		Trigger_CompareInterrupt();
		// Adjust COUNT to COMPARE, does this helps?
		//gHardwareState.COP0Reg[COUNT] = gHardwareState.COP0Reg[COMPARE];
		SavedCOUNT = gHardwareState.COP0Reg[COUNT];
	}
	
#ifdef SAVEVICOUNTER
	if( VIcounter >= MAGICNUMBER )
	{
        Trigger_VIInterrupt();
		VIcounter = 0;
	}
#else
	if( gHardwareState.COP0Reg[COUNT]-SavedCOUNT >= MAGICNUMBER )
	{
		SavedCOUNT = gHardwareState.COP0Reg[COUNT];
        Trigger_VIInterrupt();
	}
#endif

	if( CPUNeedToDoOtherTask )	
	{
		for( i=0; i<abs(gHardwareState.COP0Reg[COUNT] - SavedCOUNT2); i+=10)
		{
			if( CPUNeedToDoOtherTask )
				CPUDoOtherTasks();
			else
				break;
		}
	}

	SavedCOUNT2 = gHardwareState.COP0Reg[COUNT];
	// Run the compiled code in the Block
	__asm pushad
	__asm call Block
	__asm popad

	}
  	
	Block = SavedBlock;

#ifdef DEBUG_COMMON
	sprintf(generalmessage, "Finish Exception Service in Dyna");
	RefreshOpList(generalmessage);
#endif

}


