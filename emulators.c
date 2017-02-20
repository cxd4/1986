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

#include <windows.h>
#include <process.h>
#include <stdio.h>
#include "options.h"
#include "globals.h"
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
#include "cheatcode.h"
#include "1964ini.h"
#include "win32/DLL_Video.h"
#include "win32/DLL_Audio.h"
#include "win32/DLL_Input.h"
#include "win32/windebug.h"
#include "win32/wingui.h"
#include "dynarec/regcache.h"
#include "dynarec/x86.h"
#include "dynarec/dynaLog.h"

/*
#include "dynarec/dynaBranch.h"
#include "dynarec/dynaCOP1.h"
#include "dynarec/dynaCPU.h"
#include "dynarec/dynaCPU_defines.h"
#include "dynarec/dynaLog.h"
#include "dynarec/dynaHelper.h"
#include "dynarec/dynaRec.h"


*/

#ifdef DEBUG_COMMON
#include "win32/windebug.h"
extern char* DebugPrintInstruction(uint32 instruction);
extern char* DebugPrintInstructionWithOutRefresh(uint32 Instruction);
#endif
char* DebugPrintInstr(uint32 Instruction);

//void RunTheInterpreter();
void RunTheInterpreter(void);
void RunFasterInterpreter(void);
void RunTheRegCache(void);
void (__cdecl StartCPUThread)(void *pVoid);
uint8* (__cdecl StaticRecompiler)(uint8 *Dest);
void rc_Intr_Common(void);
void InitEmu(void);
__forceinline void Dyna_Get_Block(void);
void Dyna_Compile_Block(void);
__forceinline void Dyna_CPU_Check_Other_Tasks();

void Dyna_Code_Check_None();
void Dyna_Code_Check_QWORD();
void Dyna_Code_Check_DWORD();
void Dyna_Code_Check_BLOCK();
void Dyna_Code_Check_None_Boot();
void (*Dyna_Code_Check[])() = 
	{	Dyna_Code_Check_None, Dyna_Code_Check_None, Dyna_Code_Check_DWORD, Dyna_Code_Check_QWORD,
		Dyna_Code_Check_QWORD, Dyna_Code_Check_BLOCK, Dyna_Code_Check_BLOCK };
void (*Dyna_Check_Codes)() = NULL;
//---------------------------------------------------------------------------------------

DWORD   CPUThreadID;
HANDLE  CPUThreadHandle = NULL;
uint32	sp_hle_task=0;
int		sp_task_counter=0;
int		si_io_counter = 0;
BOOL    CPUNeedToDoOtherTask = FALSE;
BOOL	CPUNeedToCheckException = FALSE;
BOOL	CPUNeedToCheckInterrupt = FALSE;
uint32  FR_reg_offset=1;
uint32	SavedCOUNT=0;
uint32	SavedCOUNT2=0;
uint32	SavedCOUNT3=0;
static  int tocount;
uint32  VIcounter;
uint32 cp0Counter;
uint32 instcounter;
static exception_entry_count = 0;
static	dyna_check_count = 0;

#ifdef FAST_COUNTER
// Optimized new CPU COUNT and VI counter variables
uint64 current_counter;			
uint64 next_vi_counter;			// use 64bit varible, will neve overflow
uint64 next_count_counter;		// value in here is in the unit of VIcounter, not in the half rate
								// speed concerning about using 64bit is not big deal here
								// because these two variable will not be used in emu main loop
								// only when VI/COMPARE interrrupt happens
__int32 counter_leap;
__int32 countdown_counter;
void Set_Countdown_Counter(void);
uint32 Get_COUNT_Register(void);
uint32 Get_VIcounter(void);
void Count_Down_All(void);
void Count_Down(uint32 count);
void Check_VI_and_COMPARE_Interrupt(void);
void Set_COMPARE_Interrupt_Target_Counter(void);
void Init_Count_Down_Counters(void);
#endif
    
int Is_Compiling=0;
extern dyn_cpu_instr dyna_instruction[64];
extern unsigned long	lCodePosition;
static uint32 Instruction;	// variable good in emulator.c only
static uint32 translatepc;	// variable good in emulator.c only

uint32 g_translatepc;		// This global will be set at returning from a block
uint32* g_LookupPtr;		// This global will be set at returning from a block
uint32 g_blocksize;			// This global will be set at returning from a block
uint32 g_pc_is_rdram;		// This global will be set at returning from a block

uint32 TempPC;
static uint32* LookupPtr;

extern HWND hwnd;

enum GAME_STOP_REASON { EMURUNNING=1, EMUSTOP=2, EMUPAUSE=3, EMUSWITCHCORE, EMURESUME };
enum GAME_STOP_REASON reason_to_stop = EMUSTOP;

extern BOOL gamepaused;
extern BOOL pausegame;

BOOL needswitchcore = FALSE;
BOOL needinitemu=FALSE;
extern void RunSPTask(void);
extern void (*CPU_instruction[64])(uint32 Instruction);
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

int Audio_Thread_Keep_Running;
HANDLE AudioThreadHandle = NULL;
void (__cdecl AudioThread)(void *pVoid)
{
 SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL );
_Start:
 while(Audio_Thread_Keep_Running) { AUDIO_AiUpdate(FALSE); }
 goto _Start;
} 
//-------------------------------------------------------------------------------------
// Called by GUI thread to start emulating
//-------------------------------------------------------------------------------------
void RunEmulator(uint32 core)
{
   	if( !Rom_Loaded )
	{
		return;
	}

	Emu_Keep_Running = TRUE;
    Audio_Thread_Keep_Running = FALSE;
	AUDIO_Initialize(Audio_Info);
	AUDIO_AiUpdate(FALSE);
	whichcore = core;
	CPUThreadHandle = (HANDLE) _beginthread(StartCPUThread, 0, NULL);
    AudioThreadHandle = (HANDLE) _beginthread(AudioThread, 0, NULL);
    SuspendThread(AudioThreadHandle);
}

//-------------------------------------------------------------------------------------
// Called by GUI thread to pause emulating
//-------------------------------------------------------------------------------------
void PauseEmulator(void)
{
    Sleep(200);
    reason_to_stop = EMUPAUSE;
	Emu_Keep_Running = FALSE;
	AUDIO_AiUpdate(FALSE);
    ResumeThread(AudioThreadHandle);
  	Audio_Thread_Keep_Running = TRUE;
    
    while( ! gamepaused && Emu_Is_Running )
	{
#ifdef DEBUG_COMMON
		MSG msg;
		if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
		{
			if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
			{
				if( GetMessage( &msg, NULL, 0, 0 ) ) 
					DispatchMessage (&msg) ;
			}
		}
#endif
		Emu_Keep_Running = FALSE;
		reason_to_stop = EMUPAUSE;
		Sleep(50);
	}
	sprintf(generalmessage, "%s - paused", szWindowTitle);
	SetStatusBarText(0, generalmessage);
	SetWindowText(hwnd, generalmessage);
}

//-------------------------------------------------------------------------------------
// Called by GUI thread to resume emulating from pausing
//
// the global variabl "needinit" is to pass information to the CPU thread
// to do InitEmu() in CPU thread. Reason behind this is that OpenGL is multi-thread
// save, initialization must be done in the CPU thread
//-------------------------------------------------------------------------------------
extern BOOL auto_apply_cheat_code_enabled;
void ResumeEmulator(BOOL needinit)
{
        AUDIO_AiUpdate(FALSE);
        needinitemu = needinit;
        tocount = 0;

        SuspendThread(AudioThreadHandle);
        // Apply the hack codes
        if( auto_apply_cheat_code && auto_apply_cheat_code_enabled )
                CodeList_ApplyAllCode(INGAME);

        Emu_Keep_Running = TRUE;

        sprintf(generalmessage, "%s - Running", szWindowTitle);
        SetStatusBarText(0, generalmessage);
        SetWindowText(hwnd, generalmessage);
}
//-------------------------------------------------------------------------------------
// Called by GUI thread to stop emulating
//-------------------------------------------------------------------------------------
void StopEmulator(void)
{
	Emu_Keep_Running = FALSE;
    Audio_Thread_Keep_Running = FALSE;
	reason_to_stop = EMUSTOP;
	AUDIO_RomClosed();
	CONTROLLER_RomClosed();
	while( Emu_Is_Running )
	{
#ifdef DEBUG_COMMON
		MSG msg;
		if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
		{
			if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
			{
				if( GetMessage( &msg, NULL, 0, 0 ) ) 
					DispatchMessage (&msg) ;
			}
		}
#endif
		Emu_Keep_Running = FALSE;
		reason_to_stop = EMUSTOP;
		Sleep(50);
	}
}

//-------------------------------------------------------------------------------------
// Called by GUI thread to switch CPU core while emulating
//-------------------------------------------------------------------------------------
void EmulatorSetCore(int core)
{
	if( Emu_Is_Running )
	{
		if( whichcore != core )
		{
			PauseEmulator();
			TRACE2("Switch CPU Core to %s, PC=%08X", emulator_type_names[core], gHWS_pc);
			whichcore = core;
			ResumeEmulator(FALSE);
		}
	}
	else	// Emulator is not running, then change default CPU core
	{
		defaultoptions.Emulator = core;
	}

	SetStatusBarText(4, core==DYNACOMPILER ? "D" : "I");
}

// All functions above are called by GUI
// All functions below are used in the CPU thread

//-------------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------------
uint32 RDRamSizeHackSavedDWord1 = 0;
uint32 RDRamSizeHackSavedDWord2 = 0;
void CloseEmulator(void)
{
	Audio_Thread_Keep_Running = FALSE;
    VIDEO_RomClosed();
	Free_Dynarec();
	Emu_Is_Running = FALSE;
	*(uint32*)&gMS_RDRAM[rominfo.RDRam_Size_Hack] = RDRamSizeHackSavedDWord1;
	*(uint32*)&gMS_RDRAM[0x2FE1C0] = RDRamSizeHackSavedDWord2;
}

//-------------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------------
extern void Set_Translate_PC();
extern BOOL FPU_Is_Enabled;
void RefreshDynaDuringGamePlay()
{
	Init_Dynarec();	
#ifdef DYNA_GET_BLOCK_SPEEDUP
	Set_Translate_PC();
#endif
}

void InitEmu()
{
    if (OSversion < 0x80000000)                // Windows NT
        SetThreadPriority(CPUThreadHandle, THREAD_PRIORITY_NORMAL);
    else
        SetThreadPriority(CPUThreadHandle, THREAD_PRIORITY_HIGHEST);   
 
 if( gHWS_COP0Reg[STATUS] & 0x04000000 )
 {
  FR_reg_offset = 32;
 }
 else
 {
  FR_reg_offset = 1;
 }
 
 SavedCOUNT = 0;
 SavedCOUNT2=0;
 SavedCOUNT3=0;
 VIcounter = 0;
 CPUdelay = 0;
 CPUdelayPC = 0;
 sp_task_counter=0;
 si_io_counter = 0;
 CPUNeedToDoOtherTask = FALSE;
 CPUNeedToCheckInterrupt = FALSE;
 cp0Counter=0;
 gamepaused=FALSE;
 pausegame=FALSE;
 needswitchcore = FALSE;
 needinitemu=FALSE;
 lCodePosition = 0;
 Block = 0;
 exception_entry_count = 0;
 FPU_Is_Enabled = FALSE;
 dyna_check_count = 0;
 Is_Compiling = 0;
 vips = vips_speed_limits[currentromoptions.Max_FPS];
 framecounter = 0;
 QueryPerformanceCounter(&LastVITime);
 QueryPerformanceCounter(&LastSecondTime);
 
 tocount = 0;
 
 Init_Count_Down_Counters();
 
 memcpy(&HeaderDllPass[0], &gMS_ROM_Image[0], 0x40);
 VIDEO_RomOpen();
 CONTROLLER_RomOpen();
 
 RefreshDynaDuringGamePlay();
}

//-------------------------------------------------------------------------------------
// This function is called at the beginning of emulating, runs until boot successfully
//-------------------------------------------------------------------------------------
BOOL IsBooting = FALSE;
void N64_Boot()
{
	int RDRam_Hacked = 0;
	uint32 bootaddr = (*(uint32*)(gMS_ROM_Image+8) & 0x007FFFFF) + 0x80000000;

	Emu_Is_Running = TRUE;
	IsBooting = TRUE;

	while( Emu_Keep_Running && gHWS_pc != bootaddr && (gHWS_pc & 0x00FFFFFF) <  0x2000 )
	{
		if( whichcore == INTERPRETER )
			InterpreterStepCPU();
		else
			RunDynaBlock();

	if( RDRam_Hacked == 0 )
		if( (currentromoptions.DMA_Segmentation == USEDMASEG_YES && DMAInProgress ) || (dma_in_segments == FALSE && (MI_INTR_REG_R & MI_INTR_PI)) )
		{
			RDRam_Hacked = 1;

			RDRamSizeHackSavedDWord1 = *(uint32*)&gMS_RDRAM[rominfo.RDRam_Size_Hack];
			//*(uint32*)&gMS_RDRAM[rominfo.RDRam_Size_Hack] = RDRAM_SIZE;
			*(uint32*)&gMS_RDRAM[rominfo.RDRam_Size_Hack] = current_rdram_size;

			
			RDRamSizeHackSavedDWord2 = *(uint32*)&gMS_RDRAM[0x2FE1C0];

			//Azimer - DK64 Hack to break out of infinate loop
			//I believe this memory location is some sort of copywrite protection which
			//is written to using the RSP on bootup.  The only issue I see is if it
			//effects any other roms?

			if( strncmp(currentromoptions.Game_Name, "DONKEY KONG 64", 14) == 0 )
			//if( currentromoptions.crc1 == 0xEC58EABF && currentromoptions.crc2 == 0xAD7C7169 )	//DK64
				*(uint32*)&gMS_RDRAM[0x2FE1C0] = 0xAD170014;
		}
	}

	if( auto_apply_cheat_code )
		CodeList_ApplyAllCode(BOOTUPONCE);

	Emu_Is_Running = FALSE;
	IsBooting = FALSE;

	if( gHWS_pc == bootaddr )
	{
		TRACE1("N64 boot successfully, start run from %08X", bootaddr)
	}
	else
	{
		TRACE1("N64 boot failed, start run from %08X", gHWS_pc)
	}
}

void __cdecl LogDyna(char *debug, ...);

//-------------------------------------------------------------
// This is the entry point for CPU emulating thread
//-------------------------------------------------------------
void (__cdecl StartCPUThread)(void *pVoid)
{
	TRACE0("");
	TRACE0("");
	TRACE0("");
	TRACE1("Starting ROM %s", rominfo.name)
	TRACE0("");
	TRACE0("");
	TRACE0("");

#ifdef ENABLE_OPCODE_DEBUGGER
    p_gHardwareState = (HardwareState*)&gHardwareState;
    p_gMemoryState   = (MemoryState*)&gMemoryState;
#endif
	InitEmu();
	N64_Boot();

	reason_to_stop = EMURUNNING;

	__try{
START_CPU_THREAD:
		Emu_Is_Running = TRUE;
		switch (whichcore)
		{
			case INTERPRETER : 
				RunTheInterpreter();
				//RunFasterInterpreter();
				break;
			case DYNACOMPILER : 
				RunTheRegCache(); 
				break;
		}

		if( reason_to_stop == EMUPAUSE )
		{
#ifdef FAST_COUNTER
			// If user is saving state after pausing, need to update the COUNT register here
			// so we can return back to the original timer value
			// This make Donkey Kong can be saved state
			gHWS_COP0Reg[COUNT] = Get_COUNT_Register();
#endif

			PauseEmulating();
			goto START_CPU_THREAD;
		}
	}
	__except(NULL,EXCEPTION_EXECUTE_HANDLER)
	{
		MessageBox(hwnd, "Unknown Error happens, Exit from emulator", "Error", MB_ICONINFORMATION);
		AUDIO_RomClosed();
		CONTROLLER_RomClosed();
		Emu_Is_Running = FALSE;
		Emu_Keep_Running = FALSE;
		reason_to_stop = EMUSTOP;
		SendMessage(hwnd, WM_COMMAND, ID_FILE_FRESHROMLIST, 0);
	}

	CloseEmulator();
    _endthreadex(0);
}

//-------------------------------------------------------------
// Use in CPU thread, will pausing emu and wait for resume
//-------------------------------------------------------------
void PauseEmulating(void)
{
	gamepaused = TRUE;
    
    Audio_Thread_Keep_Running = TRUE;
    ResumeThread(AudioThreadHandle);
    
    while( Emu_Keep_Running == FALSE && reason_to_stop == EMUPAUSE )
	{
		gamepaused = TRUE;
		Sleep(200);
		VIDEO_UpdateScreen();
	}

	gamepaused = FALSE;

    if( Emu_Keep_Running )
	{
		AUDIO_AiUpdate(FALSE);
		Free_Dynarec();

		if( needinitemu )
			InitEmu();
		else
			RefreshDynaDuringGamePlay();
	}
}

//-------------------------------------------------------------
// This is a main loop for emulating in interpreter
//-------------------------------------------------------------
void RunTheInterpreter(void)
{
	while( Emu_Keep_Running || CPUdelay!= 0 || CPUNeedToDoOtherTask )
	{
		Instruction = FetchInstruction();		// Fetch instruction at PC
		CPU_instruction[_OPCODE_](Instruction);

		//gHWS_GPR[0] = 0;
		INTERPRETER_DEBUG_INSTRUCTION

		switch (CPUdelay)
		{
			case 0 :    
				gHWS_pc += 4; break;
			case 1 :    
				gHWS_pc += 4;         CPUdelay = 2;   
				break;
			default:    
				gHWS_pc = CPUdelayPC; CPUdelay = 0;   
				break;
		}

#ifdef FAST_COUNTER
		countdown_counter -= VICounterFactors[CounterFactor];
		if( countdown_counter <= 0 )
			Check_VI_and_COMPARE_Interrupt();
#else
		VIcounter+=VICounterFactors[CounterFactor];
		INTERPRETER_CHECK_VI

		// CPU COUNT register is increased at the half speed as the PCLOCK
		gHWS_COP0Reg[COUNT] += tocount;
		tocount = CounterFactors[CounterFactor] - tocount;
		INTERPRETER_CHECK_COMPARE
#endif

		if( CPUNeedToDoOtherTask )	
			CPUDoOtherTasks();
	}
}

//------------------------------------------------------------------------------
// This is a main loop for emulating in optimized interpreter
// In optimized interpreter, interrupt is not checked in branch delay slots,
// like in dynarec core.
//------------------------------------------------------------------------------
void RunFasterInterpreter(void)
{
	while( Emu_Keep_Running || CPUdelay!= 0 || CPUNeedToDoOtherTask )
	{
		Instruction = FetchInstruction();		// Fetch instruction at PC
		CPU_instruction[_OPCODE_](Instruction);
		//gHWS_GPR[0] = 0;
		INTERPRETER_DEBUG_INSTRUCTION

		switch (CPUdelay)
		{
			case 0 :    gHWS_pc += 4;                         break;
			case 1 :    gHWS_pc += 4;         CPUdelay = 2;   break;
			default:    
				gHWS_pc = CPUdelayPC; 
				CPUdelay = 0;   

#ifdef FAST_COUNTER
				if( countdown_counter <= 0 )
					Check_VI_and_COMPARE_Interrupt();

				g_blocksize = SavedCOUNT2 - countdown_counter;
				Dyna_CPU_Check_Other_Tasks();
				SavedCOUNT2 = countdown_counter;
#else
				INTERPRETER_CHECK_VI
				INTERPRETER_CHECK_COMPARE
				g_blocksize = gHWS_COP0Reg[COUNT] - SavedCOUNT2;
				Dyna_CPU_Check_Other_Tasks();
				SavedCOUNT2 = gHWS_COP0Reg[COUNT];
#endif

				break;
		}


		// check for the vi interrupt
#ifdef FAST_COUNTER
		countdown_counter -= VICounterFactors[CounterFactor];
#else
		VIcounter+=VICounterFactors[CounterFactor];
		gHWS_COP0Reg[COUNT] += tocount;
		tocount = CounterFactors[CounterFactor]-tocount;
#endif
	}
}

//-------------------------------------------------------------
// This is a main loop for emulating in Dynarec
//-------------------------------------------------------------
uint32 HardwareStart = (uint32)&gHardwareState+128;
uint32 savedpc=0;
uint32 Inst;
void RunTheRegCache(void)
{
	register uint32 i;

    __asm pushad

#ifdef USE_STACK
        __asm push ebp
        __asm mov ebp, HardwareStart
#endif

    while( Emu_Keep_Running )
        {
_DoOtherTask:

#ifdef DYNA_GET_BLOCK_SPEEDUP
    Block = (uint8*)*g_LookupPtr;
    if( Block != NULL && g_pc_is_rdram ) Dyna_Check_Codes();
#else
    __asm { push ebp } Dyna_Get_Block(); __asm { pop ebp }
#endif
    if (Block == NULL)  { __asm { push ebp } Dyna_Compile_Block(); __asm { pop ebp } }

                // Run the compiled code in the Block
#ifdef _DEBUG
                DEBUG_PRINT_DYNA_EXECUTION_INFO
        __asm pushad
#endif
                __asm call Block
#ifdef _DEBUG
        __asm popad
#endif

#ifdef ENABLE_OPCODE_DEBUGGER
                if (CPUdelay == 1)
            gHardwareState_Interpreter_Compare.pc += 4;


        if( gHardwareState.pc != CPUdelayPC && CPUdelay == 1 )
                {
                        savedpc = gHardwareState.pc;
                        gHardwareState.pc = gHardwareState_Interpreter_Compare.pc;
                        Inst = FetchInstruction();

                        sprintf(generalmessage, "Compare Target PC error: pc=%08X, DelayPC=%08X,
Before Jump PC=%08X\nDyna:%s\nInterpreter:",
                                savedpc, CPUdelayPC, gHardwareState_Interpreter_Compare.pc,
DebugPrintInstr(Inst) );
                        gHardwareState.pc = savedpc;
                        COMPARE_SwitchToInterpretive();
                        strcat(generalmessage, DebugPrintInstr(Inst));
                        TRACE0(generalmessage);
                        DisplayError(generalmessage);
                        COMPARE_SwitchToDynarec();
                }

        CPUdelay = 0;
                gHardwareState_Interpreter_Compare.pc = gHardwareState.pc;
#endif

#ifdef FAST_COUNTER
                if( countdown_counter <= 0 )
        { __asm { push ebp}  Check_VI_and_COMPARE_Interrupt();  __asm { pop ebp } }// in the function Check_VI_and_COMPARE_Interrupt()
#else
        __asm push ebp;
        OPCODE_DEBUGGER_BEGIN_EPILOGUE
                DYNA_CHECK_COMPARE      // Check COMPARE interrupts
                DYNA_CHECK_VI   // Check VI counter / generate VI interrupts
        OPCODE_DEBUGGER_END_EPILOGUE
        __asm pop ebp;
#endif

    	//Dyna_CPU_Check_Other_Tasks() replaced in this function by this:
        if( CPUNeedToDoOtherTask )	
	    {
            __asm { push ebp }
            for( i=0; CPUNeedToDoOtherTask && i<g_blocksize; i+=10)
		    	CPUDoOtherTasks();
            __asm { pop ebp }
	    }
        
        

        }
    if (CPUNeedToDoOtherTask)
        goto _DoOtherTask;
#ifdef USE_STACK
        __asm pop ebp;
#endif
        __asm popad
}

extern void DynCheckForTimedEvents(uint32 cnt);
extern void SetTLBMissException();
extern int WasFlushed;

//------------------------------------------------------------------------------------------------------------------------------
// Check and execute all other tasks, called by emulating main loop
// Will do DMA, interrupt checking and so on.
//------------------------------------------------------------------------------------------------------------------------------
void CPUDoOtherTasks(void)
{
	if( sp_task_counter > 0 )
	{
		sp_task_counter -= SPCOUNTERTOINCREASE;		// Transfer one byte per instruction
		if( sp_task_counter <= 0 )
		{
			OPCODE_DEBUGGER_BEGIN_EPILOGUE
			RunSPTask();
			OPCODE_DEBUGGER_END_EPILOGUE
		}
		//else
			//return;		// SP Task is not finished yet, CPU will check it after next instruction
	}
	// to get here, SP task in finished


#ifndef ENABLE_OPCODE_DEBUGGER
	// DMA segmentation is very important, it is for timing issues.
	// DMA segmentation makes a lot of games boot and work
	// DMA transfer has to be in front of SP, otherwise Zelda game stop working
	if(currentromoptions.DMA_Segmentation == USEDMASEG_YES && DMAInProgress)
	{
		DoDMASegment();
		//return;
	}
	// To get here, DMA task is finished

	if( currentromoptions.DMA_Segmentation == USEDMASEG_YES && si_io_counter > 0 )
	{
		si_io_counter -= 4;
		if( si_io_counter <= 0 )
		{
			si_io_counter = 0;
			SI_STATUS_REG &= ~SI_STATUS_RD_BUSY;
		}
	}
#endif

	if( whichcore == INTERPRETER )	// intepreter mode
	{
		if( CPUNeedToCheckException)
		{
			if ((gHWS_COP0Reg[STATUS] & EXL_OR_ERL ) == 0 &&	// No in another interrupt routine
				(gHWS_COP0Reg[CAUSE] & 0x0000007C) )	// Is there a exception ?
			{
				gHWS_pc = SetException_Interrupt(gHWS_pc);
				CPUNeedToCheckException = FALSE;
			}
		}
		else if( CPUNeedToCheckInterrupt )
		{
			if ((gHWS_COP0Reg[STATUS] & EXL_OR_ERL /*0x00000004*/) == 0 &&	// No in another interrupt routine
				((gHWS_COP0Reg[STATUS] & 0x00000001) != 0 &&	// Interrupts are enabled
				((gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) != 0) ))
			{	// If there are interrupts and interrupts are enabled in the interrupt masks
				//if (((gHWS_COP0Reg[STATUS] & 0x00000006)) == 0)
					// If there is not either Error or Exceptions happens already
					//if (((gHWS_COP0Reg[STATUS] & 0x00000001)) == 1)
						// if the interrupt processing is enabled
				gHWS_pc = SetException_Interrupt(gHWS_pc);
				CPUNeedToCheckInterrupt = FALSE;
			}
		}
	}
	else	// Dyna mode
	{
		if( CPUNeedToCheckInterrupt )
		{
#ifdef ENABLE_OPCODE_DEBUGGER
			COMPARE_SwitchToInterpretive();
			if ((gHWS_COP0Reg[STATUS] & EXL_OR_ERL ) == 0 &&	// No in another interrupt routine
				((gHWS_COP0Reg[STATUS] & 0x00000001) != 0 &&	// Interrupts are enabled
				((gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) != 0) ))
			{
				gHWS_COP0Reg[EPC] = gHWS_pc;
				gHWS_COP0Reg[STATUS] |= EXL;          // set EXL = 1 
				gHWS_pc = 0x80000180;
				gHWS_COP0Reg[CAUSE] &= NOT_BD;        // clear BD
				DEBUG_INTERRUPT_TRACE(TRACE1("Interrupt is being served, Interrupt=%s", Get_Interrupt_Name()));
			}

			COMPARE_SwitchToDynarec();
			if ((gHWS_COP0Reg[STATUS] & EXL_OR_ERL ) == 0 &&	// No in another interrupt routine
				((gHWS_COP0Reg[STATUS] & 0x00000001) != 0 &&	// Interrupts are enabled
				((gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) != 0) ))
			{
				DEBUG_INTERRUPT_TRACE(TRACE1("Interrupt is being served, Interrupt=%s", Get_Interrupt_Name()));
				rc_Intr_Common();
#ifdef DYNA_GET_BLOCK_SPEEDUP
				Set_Translate_PC();
#endif
				CPUNeedToCheckInterrupt = FALSE;
			}
#else
			if ((gHWS_COP0Reg[STATUS] & EXL_OR_ERL ) == 0 &&	// No in another interrupt routine
				((gHWS_COP0Reg[STATUS] & 0x00000001) != 0 &&	// Interrupts are enabled
				((gHWS_COP0Reg[CAUSE] & gHWS_COP0Reg[STATUS] & 0x0000FF00) != 0) ))
			{
				//DEBUG_INTERRUPT_TRACE(TRACE1("Interrupt is being served, Interrupt=%s", Get_Interrupt_Name()));
				rc_Intr_Common();
#ifdef DYNA_GET_BLOCK_SPEEDUP
				Set_Translate_PC();
#endif
				//gHWS_pc = SetException_Interrupt(gHWS_pc);
				CPUNeedToCheckInterrupt = FALSE;
			}
#endif
		}
	}

	// up to here, no CPU task left.
	if( sp_task_counter>0 || DMAInProgress || si_io_counter > 0 
		|| CPUNeedToCheckException || CPUNeedToCheckInterrupt
		)
		return;
	CPUNeedToDoOtherTask = FALSE;
}

void ClearCPUTasks(void)
{
	CPUNeedToDoOtherTask = FALSE;
	if( currentromoptions.DMA_Segmentation == USEDMASEG_YES )
	{
		DMAInProgress = FALSE;
		PIDMAInProgress = NO_DMA_IN_PROGRESS;
		SIDMAInProgress = NO_DMA_IN_PROGRESS;
		SPDMAInProgress = NO_DMA_IN_PROGRESS;
	}
	sp_task_counter = 0;
	si_io_counter = 0;
	CPUNeedToCheckException = FALSE;
	CPUNeedToCheckInterrupt = FALSE;
}


//------------------------------------------------------------------------------------------------------------------------------
// Step and run one opcode in interpreter mode
//------------------------------------------------------------------------------------------------------------------------------
void InterpreterStepCPU(void)
{
    uint32 Instruction;

	Instruction = FetchInstruction();
    CPU_instruction[_OPCODE_](Instruction);
    //gHWS_GPR[0] = 0;

	INTERPRETER_DEBUG_INSTRUCTION

    switch (CPUdelay)
    {
        case 0 :    gHWS_pc += 4;                         break;
        case 1 :    gHWS_pc += 4;         CPUdelay = 2;   break;
        default:    gHWS_pc = CPUdelayPC; CPUdelay = 0;   break;
    }

#ifdef FAST_COUNTER
	countdown_counter -= VICounterFactors[CounterFactor];
	if( countdown_counter <= 0 )
		Check_VI_and_COMPARE_Interrupt();
#else
    // check for the vi interrupt
	VIcounter += VICounterFactors[CounterFactor];

    gHWS_COP0Reg[COUNT] += tocount;
	tocount = CounterFactors[CounterFactor]-tocount;
	
	INTERPRETER_CHECK_VI
	INTERPRETER_CHECK_COMPARE
#endif

	if( CPUNeedToDoOtherTask )	
		CPUDoOtherTasks();
}

//------------------------------------------------------------------------------------------------------------------------------
// Return the instruction at current PC
// This is a utility function, called by some debug function
//------------------------------------------------------------------------------------------------------------------------------
__forceinline uint32 FetchInstruction(void)
{
	uint32 Instruction=0;	//Return 0 (NOP) if any error occurs

	__try{
		Instruction = LOAD_UWORD_PARAM(gHWS_pc);
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(gHWS_pc) ) 
		{                                                   
			uint32 translatepc = TranslateITLBAddress(gHWS_pc);
			__try{
				Instruction = LOAD_UWORD_PARAM(translatepc);
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DisplayError("%08X: PC out of range", gHWS_pc);
			}
		}
		else
			DisplayError("%08X: PC out of range", gHWS_pc);
	}
	return Instruction;
}

//------------------------------------------------------------------------------------------------------------------------------
// This inline function will set variable Instruction and pcptr according to current PC regsiter
//------------------------------------------------------------------------------------------------------------------------------
__forceinline void DynaFetchInstruction()
{
	__try{
		pcptr = pLOAD_UWORD_PARAM(gHWS_pc);
		Instruction = *pcptr;

		if( IN_K0_SEG(gHWS_pc) && (gHWS_pc & 0x1FFFFFFF) < current_rdram_size )
		{
			*(uint32*)&RDRAM_Copy[gHWS_pc & 0x1FFFFFFF] = Instruction;
			__try{
				*(uint32*)((uint8*)sDYN_PC_LOOKUP[gHWS_pc>>16] + (uint16)gHWS_pc) = 0;
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER)
			{
			}
		}
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		if( NOT_IN_KO_K1_SEG(gHWS_pc) ) 
		{                                                   
			translatepc = TranslateITLBAddress(gHWS_pc);
			__try{
				pcptr = pLOAD_UWORD_PARAM(translatepc);
				Instruction = *pcptr;
				if( IN_K0_SEG(translatepc) && (translatepc & 0x1FFFFFFF) < current_rdram_size )
				{
					*(uint32*)&RDRAM_Copy[translatepc & 0x1FFFFFFF] = Instruction;
					__try{
						*(uint32*)((uint8*)sDYN_PC_LOOKUP[translatepc>>16] + (uint16)translatepc) = 0;
					}__except(NULL,EXCEPTION_EXECUTE_HANDLER)
					{
					}
				}
			}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DisplayError("%08X: Dyna 2 PC out of range", gHWS_pc);
			}
		}
		else
			DisplayError("%08X: Dyna 1 PC out of range", gHWS_pc);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
// This inline function will run a block of code in Dyna, will compile it first if needed
//------------------------------------------------------------------------------------------------------------------------------
void RunDynaBlock(void)
{
	uint32 HardwareStart = (uint32)&gHardwareState+128;
	Dyna_Get_Block();

	// Check if this block of N64 code has been compiled before
	if (Block == NULL)	Dyna_Compile_Block();

	DEBUG_PRINT_DYNA_EXECUTION_INFO

	// Run the compiled code in the Block
	__asm pushad
#ifdef USE_STACK
	__asm push ebp
	__asm mov ebp, HardwareStart
	__asm call Block
	__asm pop ebp
#else
	__asm call Block
#endif
	__asm popad

#ifdef ENABLE_OPCODE_DEBUGGER
	if (CPUdelay == 1)
        gHardwareState_Interpreter_Compare.pc += 4;
        
	if( gHardwareState.pc != CPUdelayPC && CPUdelay == 1 )
	{
		uint32 savepc = gHardwareState.pc;
		uint32 Inst;
		gHardwareState.pc = gHardwareState_Interpreter_Compare.pc;
		Inst = FetchInstruction();

		sprintf(generalmessage, "Compare PC error: pc=%08X, DelayPC=%08X, oldPC=%08X\nDyna:%s\nInterpreter:", 
			gHardwareState.pc, CPUdelayPC, gHardwareState_Interpreter_Compare.pc, DebugPrintInstr(Inst) );
		gHardwareState.pc = savepc;
		COMPARE_SwitchToInterpretive();
		strcat(generalmessage, DebugPrintInstr(Inst));
		DisplayError(generalmessage);
		COMPARE_SwitchToDynarec();

	}
	CPUdelay = 0;
	gHardwareState_Interpreter_Compare.pc = gHardwareState.pc;
#endif



#ifdef FAST_COUNTER
	if( countdown_counter <= 0 )
		Check_VI_and_COMPARE_Interrupt();
#else
	OPCODE_DEBUGGER_BEGIN_EPILOGUE
	DYNA_CHECK_COMPARE	// Check COMPARE interrupts
	DYNA_CHECK_VI	// Check VI counter / generate VI interrupts
    OPCODE_DEBUGGER_END_EPILOGUE
#endif

	Dyna_CPU_Check_Other_Tasks();
}

//------------------------------------------------------------------------------------------------------------------------------
// Called by dyanrec emu main loop to check other tasks, like DMA and interrupt checking
//------------------------------------------------------------------------------------------------------------------------------
__forceinline void Dyna_CPU_Check_Other_Tasks()
{
	if( CPUNeedToDoOtherTask )	
	{
		register uint32 i;
		for( i=0; CPUNeedToDoOtherTask && i<g_blocksize; i+=10)
			CPUDoOtherTasks();
	}
}

//------------------------------------------------------------------------------------------------------------------------------
// Validate the compiled block, and doing dyna code checking
//------------------------------------------------------------------------------------------------------------------------------
void Dyna_Code_Check_None()
{
}

//------------------------------------------------------------------------------------------------------------------------------
// Validate the compiled block, and doing dyna code checking by checking QWORD method
//------------------------------------------------------------------------------------------------------------------------------
void Dyna_Code_Check_QWORD()
{
	register uint32 pc = g_pc_is_rdram;

	if( *(uint32*)(RDRAM_Copy+pc) != *(uint32*)(gMS_RDRAM+pc) )
	{
		Block = NULL;
		*(uint32*)((uint8*)sDYN_PC_LOOKUP[(pc|0x80000000)>>16] + (uint16)pc) = 0;
		*(uint32*)(RDRAM_Copy+pc) = *(uint32*)(gMS_RDRAM+pc);
	}
	else if( *(uint32*)(RDRAM_Copy+pc+4) != *(uint32*)(gMS_RDRAM+pc+4) )
	{
		Block = NULL;
		*(uint32*)((uint8*)sDYN_PC_LOOKUP[((pc+4)|0x80000000)>>16] + (uint16)(pc+4)) = 0;
		*(uint32*)(RDRAM_Copy+pc+4) = *(uint32*)(gMS_RDRAM+pc+4);
	}
}


//------------------------------------------------------------------------------------------------------------------------------
// Validate the compiled block, and doing dyna code checking by checking DWORD method
//------------------------------------------------------------------------------------------------------------------------------
void Dyna_Code_Check_DWORD()
{
	if( *(uint32*)(RDRAM_Copy+g_pc_is_rdram) != *(uint32*)(gMS_RDRAM+g_pc_is_rdram) )
	{
		Block = NULL;
		*(uint32*)(RDRAM_Copy+g_pc_is_rdram) = *(uint32*)(gMS_RDRAM+g_pc_is_rdram);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
// Validate the compiled block, and doing dyna code checking by checking whole block method
//------------------------------------------------------------------------------------------------------------------------------
void Dyna_Code_Check_BLOCK()
{
	register int length;	// How to get the length of the block
	uint32 pc = g_pc_is_rdram;
	
	for( length = *(uint16*)(Block-2)-1; length >= 0; length--, pc+=4 )
	{
		if( *(uint32*)(RDRAM_Copy+pc) != *(uint32*)(gMS_RDRAM+pc) )
		{
			Block = NULL;
			while( length >= 0 )
			{
				*(uint32*)((uint8*)sDYN_PC_LOOKUP[(pc|0x80000000)>>16] + (uint16)pc) = 0;
				*(uint32*)(RDRAM_Copy+pc) = *(uint32*)(gMS_RDRAM+pc);
				length--;
				pc+=4;
			}
			break;
		}
	}
	//if( g_pc_is_rdram%0x80 == 4 ) Block = NULL;	// this is to test dynabuffer overrun
}

//------------------------------------------------------------------------------------------------------------------------------
// Validate the compiled block, and doing dyna code checking by checking QWORD method
//------------------------------------------------------------------------------------------------------------------------------
void Dyna_Code_Check_None_Boot()
{
	register uint32 pc = g_pc_is_rdram;

	if( *(uint32*)(RDRAM_Copy+pc) != *(uint32*)(gMS_RDRAM+pc) )
	{
		Block = NULL;
		*(uint32*)((uint8*)sDYN_PC_LOOKUP[(pc|0x80000000)>>16] + (uint16)pc) = 0;
		*(uint32*)(RDRAM_Copy+pc) = *(uint32*)(gMS_RDRAM+pc);
	}
	else if( *(uint32*)(RDRAM_Copy+pc+4) != *(uint32*)(gMS_RDRAM+pc+4) )
	{
		Block = NULL;
		*(uint32*)((uint8*)sDYN_PC_LOOKUP[((pc+4)|0x80000000)>>16] + (uint16)(pc+4)) = 0;
		*(uint32*)(RDRAM_Copy+pc+4) = *(uint32*)(gMS_RDRAM+pc+4);
	}

	if( DListCount > 100 )	//yes, we have detect the first self-modify code
	{
		Dyna_Check_Codes = Dyna_Code_Check[CodeCheckMethod-1];
		TRACE0("Reset the Dyna Code Check Method => None");
	}
}


uint32 BlockStart;
//--------------------------------------------------------------------------
// This inline function will compile a block at the current PC
//--------------------------------------------------------------------------
void Dyna_Compile_Block(void)
{
	Is_Compiling++;
	if( Is_Compiling>1 )
	{
		DisplayError("Compiler is re-entered, can not support it.");
	}

#ifdef DYNA_GET_BLOCK_SPEEDUP
	 translatepc = g_translatepc;
	 LookupPtr = g_LookupPtr;
#endif

    gMultiPass.MapPass_Count = 0;
    gMultiPass.OpcodePass_Count = 0;
    gMultiPass.WriteCode = 1;
    gMultiPass.WhichPass = COMPILE_MAP_ONLY;

	lCodePosition += 2;	//increase the lCodePosition by 2 leave two bytes in front of the block
						//to store block size
	BlockStart = (uint32)(&dyna_RecompCode[lCodePosition]);	

redo_compile:
	TempPC = gHWS_pc;
	Block = (uint8*)BlockStart;
	*(uint16*)(Block-2) = 0;	//store block size
    if (gMultiPass.UseOnePassOnly == 1)
        gMultiPass.WhichPass = COMPILE_ALL;

	KEEP_RECOMPILING = 1;
	//*LookupPtr = (_u32)Block;
	cp0Counter = 0;

	//get instruction
	DynaFetchInstruction();


	if( currentromoptions.Use_Register_Caching == USEREGC_NO )
		FlushAllRegisters();

	gHWS_code = Instruction;
	DYNA_DEBUG_INSTRUCTION
	DYNA_LOG_INSTRUCTION

#ifdef DEBUG_COMMON
	MOV_ImmToMemory(1, (unsigned long)&gHWS_pc, gHWS_pc);
#endif

	dyna_instruction[_OPCODE_](&gHardwareState);
	cp0Counter++;

	while(KEEP_RECOMPILING)
	{
		gHWS_pc += 4;
		cp0Counter++;

		DynaFetchInstruction();

		if( currentromoptions.Use_Register_Caching == USEREGC_NO )
			FlushAllRegisters();

		gHWS_code = Instruction;
		DYNA_DEBUG_INSTRUCTION
		DYNA_LOG_INSTRUCTION

		#ifdef DEBUG_COMMON
		MOV_ImmToMemory(1, (unsigned long)&gHWS_pc, gHWS_pc);
		#endif
		dyna_instruction[_OPCODE_](&gHardwareState);
    }

#ifdef DEBUG_COMMON
	if( cp0Counter > 255 ) //DisplayError("Compiled Block is too large, size=%d, pc=%08X, end at %08X", cp0Counter+1, TempPC, TempPC+(cp0Counter+1)*4);
		TRACE3("Compiled Block is too large, size=%d, pc=%08X, end at %08X", cp0Counter+1, TempPC, TempPC+(cp0Counter+1)*4);
#endif

	// Save info for dyna code check/check block to use
	*LookupPtr = (_u32)Block;	// Need to assign the value after compiling, otherwise the value will
								// be set to 0 when doing DynaFetchInstruction()
	*(uint16*)(Block-2) = (uint16)cp0Counter+1;	//store block size

	DEBUG_PRINT_DYNA_COMPILE_INFO
	
	gHWS_pc = TempPC;

	if( DynaBufferOverError )	//ok, we have a buffer error, need to fresh the dyna buffer and recompile this
								//block again.
	{
		//Init_Dynarec();
		if( DynaBufferOverError )
		{
#ifdef DYNAEXECUTIONMAPMEMORY
			int i;
			for( i=0; i<0x20000; i++ )
			{
				if( dynarommap[i] != NULL )
				{
					memset(dynarommap[i], 0, 0x10000);
				}
			}
#else
			memset(DynaSP_REG,0x00,0x2000);
			memset(DynaRDRAM,0x00,0x00800000);
			memset(DynaROM,0x00,gAllocationLength);
#endif
			lCodePosition = 0;
			DynaBufferOverError = FALSE;
			TRACE0("Dyna Buffer Overrun, refresh dyna");
		}
        gMultiPass.MapPass_Count = 0;
        gMultiPass.OpcodePass_Count = 0;
        gMultiPass.WriteCode = 1;
        gMultiPass.WhichPass = COMPILE_MAP_ONLY;
		goto redo_compile;
	}
	Is_Compiling--;
}

//------------------------------------------------------------------------------------------------------------------------------
// This inline function will return a compiled block for the current PC
//------------------------------------------------------------------------------------------------------------------------------
__forceinline void Dyna_Get_Block(void)
{
#ifdef DYNA_GET_BLOCK_SPEEDUP
	Block = (uint8*)*g_LookupPtr;
	if( Block != NULL && g_pc_is_rdram ) Dyna_Check_Codes();
#else
	register unsigned __int32 translatepc = gHWS_pc;

	if( NOT_IN_KO_K1_SEG(gHWS_pc) ) 
	{
		translatepc = TranslateITLBAddress(gHWS_pc);
	}

	if( (translatepc & 0x1FFFFFFF) < current_rdram_size )
		g_pc_is_rdram = (translatepc&0x7FFFFF);
	else
		g_pc_is_rdram = 0;

	LookupPtr = (uint32*)((uint8*)sDYN_PC_LOOKUP[translatepc>>16] + (uint16)translatepc);

	__try{
		Block = (uint8*)*LookupPtr;
		if( Block != NULL && g_pc_is_rdram )	Dyna_Check_Codes();
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER)
	{
#ifdef DYNAEXECUTIONMAPMEMORY
		if( UnmappedMemoryExceptionHelper(translatepc) )
		{
			LookupPtr = (uint32*)((uint8*)sDYN_PC_LOOKUP[translatepc>>16] + (uint16)translatepc);
			Block = (uint8*)*LookupPtr;
			if( Block != NULL && g_pc_is_rdram )	Dyna_Check_Codes();
		}
		else
#endif
			DisplayError("Execution from unmapped address, PC=%08X", gHWS_pc);
	}
#endif
}



#define EXCEPTION_MAX_ENTRY 10
/////////////////////////////////////////////////////////////////////////////////////////////
// This routine serves exceptions in dynarec
// This service routine could be re-entered
/////////////////////////////////////////////////////////////////////////////////////////////
void Dyna_Exception_Service_Routine(uint32 vector)
{
	exception_entry_count++;

	if( exception_entry_count < EXCEPTION_MAX_ENTRY )
	{
		uint8* SavedBlock = Block;
		uint32 TempPC=gHWS_pc;

		SetStatusBarText(4, "E");


		if( (gHWS_COP0Reg[STATUS] & EXL) == 0 )	// Exception not in exception
		{
			gHWS_COP0Reg[EPC] = gHWS_pc;
			gHWS_COP0Reg[STATUS] |= EXL;          // set EXL = 1 
		}
		else
		{
			//If exception is within another exception, service the exception
			// without setting EPC
			TempPC = gHWS_COP0Reg[EPC];
		}

		if( gHWS_COP0Reg[STATUS] & BEV )
		{
			DisplayError("Exception happens during boot, 1964 usually does not handle it");
			//if( vector == 0x80000080) vector = 0x80000000;
			//vector = vector - 0x80000000 + 0xbfc00200;
		}

		// How about branch delay ??

		gHWS_pc = vector;
#ifdef DYNA_GET_BLOCK_SPEEDUP
		Set_Translate_PC();
#endif

		gHWS_COP0Reg[CAUSE] &= NOT_BD;        // clear BD 
    
		TRACE2("Start Exception Service in Dyna, exception=%d, EPC=%08X", (gHWS_COP0Reg[CAUSE]&EXCCODE)>>2, gHWS_COP0Reg[EPC])

		while( TempPC != gHWS_pc && (Emu_Keep_Running || reason_to_stop == EMUPAUSE) )
		{
			if( whichcore == INTERPRETER || Is_Compiling > 0 )
				InterpreterStepCPU();
			else
				RunDynaBlock();
		}
  		
		if( !Emu_Keep_Running )	// User has stopped emulating, will quit emu thread
		{
			AUDIO_RomClosed();
			CONTROLLER_RomClosed();
			Emu_Is_Running = FALSE;
			CloseEmulator();
			_endthreadex(0);
		}

		Block = SavedBlock;

		TRACE0("Finish Exception Service in Dyna")
		SetStatusBarText(4, whichcore==DYNACOMPILER ? "D" : "I");
	}
	else 
	{
		if( exception_entry_count >= EXCEPTION_MAX_ENTRY )
		{
			DisplayError("Exception service routine re-enter exceed maxmium 10 times, skipped");
		}
		// We will not worry about this new exception
	}
	exception_entry_count--;
}

// This function is called to mark a compiled block as invalid so the block will be compiled
// again in the next time of execution.
// This function is designed to be called from anywhere during dyanrec emu execution, for example
// from exception service routine
void Dyna_Invalidate_Compiled_Block(uint32 pc)
{
#define MAX_PC_BACK		400
	// Step 1, back search the beginning of the block
	register uint32 translatepc = pc;
	register int count=0;

	TRACE1("Block at pc=%08X is mad invalid", pc);

	if( NOT_IN_KO_K1_SEG(pc) ) 
	{
		translatepc = TranslateITLBAddress(pc);
	}

	if( (translatepc & 0x1FFFFFFF) >= current_rdram_size )
	{
		return;	// pc is not in RDRAM range
	}

	*(uint32*)(RDRAM_Copy+(translatepc&0x1FFFFFFF)) = 0;	
	// we are not sure if we can find the beginning of the block,
	// so we marked the point of PC as NULL in RDRAM_Copy any way, then we can
	// use dyan_code_check_block method to do code checking

Step1_invalidate_block:
	LookupPtr = (uint32*)((uint8*)sDYN_PC_LOOKUP[translatepc>>16] + (uint16)translatepc);

	__try{
		Block = (uint8*)*LookupPtr;
		if( Block == NULL )
		{
			if( count < MAX_PC_BACK )
			{
				count++;
				translatepc -= 4;
				goto Step1_invalidate_block;
			}
			else
				return;	// Can not go back to beginning of the block
		}
		else
		{
			*LookupPtr = 0; // Here we found the beginning of the block, so mark it NUL
			*(uint32*)(RDRAM_Copy+(translatepc&0x1FFFFFFF)) = 0;	
		}
	}__except(EXCEPTION_EXECUTE_HANDLER)
	{
		if( count < MAX_PC_BACK )
		{
			count++;
			translatepc -= 4;
			goto Step1_invalidate_block;
		}
		else
			return;	// Can not go back to beginning of the block
	}
}

#ifdef FAST_COUNTER
void Set_Countdown_Counter(void)
{
	current_counter = current_counter+counter_leap - countdown_counter;
	//current_counter = current_counter + counter_leap - countdown_counter;
	if( next_vi_counter < next_count_counter )
	{
		countdown_counter = (__int32)(next_vi_counter - current_counter);
	}
	else
	{
		countdown_counter = (__int32)(next_count_counter - current_counter);
	}
	counter_leap = countdown_counter;
}

uint32 Get_VIcounter(void)
{
	return (uint32)((MAGICNUMBER  + (current_counter + counter_leap - countdown_counter) - next_vi_counter ))%MAGICNUMBER;
}

void Count_Down_All(void)
{
	countdown_counter = 0;
	//Check_VI_and_COMPARE_Interrupt();
}

void Count_Down(uint32 count)
{
	countdown_counter -= count;
}

void Check_VI_and_COMPARE_Interrupt(void)
{
	gHWS_COP0Reg[COUNT] = Get_COUNT_Register();
	if( next_vi_counter <= current_counter + counter_leap - countdown_counter )
	{	//ok, should trigger a VI interrupt
		OPCODE_DEBUGGER_BEGIN_EPILOGUE
		Trigger_VIInterrupt();
		OPCODE_DEBUGGER_END_EPILOGUE
		next_vi_counter += MAGICNUMBER;
	}
	
	if( next_count_counter <= current_counter + counter_leap - countdown_counter )
	{	//ok, should trigger an COMPARE interrupt
		OPCODE_DEBUGGER_BEGIN_EPILOGUE
		Trigger_CompareInterrupt();
		OPCODE_DEBUGGER_END_EPILOGUE
		next_count_counter += 0x1000000000;
	}
	Set_Countdown_Counter();
}

uint32 Get_COUNT_Register(void)
{
	return (uint32)((current_counter+counter_leap - countdown_counter)*CounterFactors[CounterFactor]/VICounterFactors[CounterFactor]/2);
}

void Set_COMPARE_Interrupt_Target_Counter(void)
{
	uint32 compare_reg = gHWS_COP0Reg[COMPARE];
	uint32 count_reg = Get_COUNT_Register();

	if( compare_reg > count_reg )
	{
		next_count_counter = current_counter+counter_leap - countdown_counter + 
			((uint64)(compare_reg - count_reg))*VICounterFactors[CounterFactor]/CounterFactors[CounterFactor]*2;
	}
	else
	{
		next_count_counter = current_counter+(counter_leap - countdown_counter) + 
			((uint64)(0x100000000 + compare_reg - count_reg))*2*VICounterFactors[CounterFactor]/CounterFactors[CounterFactor];;
	}
	
	Set_Countdown_Counter();	//reset the Countdown counter
}

void Init_Count_Down_Counters(void)
{
	current_counter = gHWS_COP0Reg[COUNT]*VICounterFactors[CounterFactor]/CounterFactors[CounterFactor]*2;

	if( gHWS_COP0Reg[COMPARE] != 0 )
	{
		if( gHWS_COP0Reg[COMPARE] > gHWS_COP0Reg[COUNT] )
		{
			next_count_counter = current_counter + (gHWS_COP0Reg[COMPARE]-gHWS_COP0Reg[COUNT])*VICounterFactors[CounterFactor]/CounterFactors[CounterFactor]*2;
		}
		else
		{
			next_count_counter = current_counter + (0x100000000+gHWS_COP0Reg[COMPARE]-gHWS_COP0Reg[COUNT])*VICounterFactors[CounterFactor]/CounterFactors[CounterFactor]*2;
		}
		next_count_counter = 0x100000000+current_counter;
	}
	else
	{
		next_count_counter = current_counter + (0x100000000-gHWS_COP0Reg[COUNT])*VICounterFactors[CounterFactor]/CounterFactors[CounterFactor]*2;
	}
	next_vi_counter = current_counter+current_counter%MAGICNUMBER;
	counter_leap = 0;
	countdown_counter = 0;
	Set_Countdown_Counter();	//reset the Countdown counter
}

#endif