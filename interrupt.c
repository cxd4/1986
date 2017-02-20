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


The project is a direct documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar. Most source
code comments are taken directly from anarko's n64toolkit with consent and are 
the property of anarko.
*/


#include <windows.h>
#include "interrupt.h"
#include "globals.h"
#include "options.h"
#include "r4300i.h"
#include "n64rcp.h"
#include "timer.h"
#include "hardware.h"
#include "dma.h"
#include "debug_option.h"
#include "dynarec/dynarec.h"
#include "win32/DLL_Video.h"
#include "win32/DLL_Audio.h"
#include "win32/registry.h"
#include <stdio.h>  
#include <stdlib.h>

#ifdef DEBUG_COMMON
char dbgString[80];
#endif


extern void RefreshOpList(char *opcode);
extern uint32 sp_hle_task;
extern int    sp_task_counter;
extern uint32 VIcounter;
int		viframeskip=0;				// This global parameter determines to skip a VI frame
									// after every few frames
int		viframeskipcount=0;
int		framecounter=0;				// To count how many frames are displayed per second


//extern uint32* InstructionPointer;

uint8* InterruptVector;
uint8* IVTStart;
uint8* TLBMissStart;
BOOL VIupdated=FALSE;

//---------------------------------------------------------------------------------------
// This function is called when a Exception Interrupt happens, will set COP0 registers
// EPC and CAUSE, then return a correct interrupt vector address
// 
uint32 SetException_Interrupt(uint32 pc)
{
	uint32 newpc = 0x180;

	// Any Interrupt and Exceptions will result delay 4 PCLOCKs
	VIcounter+=4;
	gHardwareState.COP0Reg[COUNT] +=2;


    if (CPUdelay != 0)                  /* are we in branch delay slot? */
    {                                       /* yes */
        gHardwareState.COP0Reg[CAUSE] |= BD;
        gHardwareState.COP0Reg[EPC] = pc - 4;
        CPUdelay = 0;
    }
    else
    {                                       /* no */            
        gHardwareState.COP0Reg[CAUSE] &= NOT_BD;
        gHardwareState.COP0Reg[EPC] = pc;
    }

    gHardwareState.COP0Reg[STATUS] |= EXL;              // set EXL = 1
														// to disable further interrupts

    if ( (gHardwareState.COP0Reg[CAUSE] & TLBL_Miss) || (gHardwareState.COP0Reg[CAUSE] & TLBS_Miss))
	{
		newpc = 0x80;
    }

    gHardwareState.COP0Reg[CAUSE] &= NOT_EXCCODE;   /* clear EXCCode  */

	if( gHardwareState.COP0Reg[STATUS] & (1 << 22) )
	{
		//newpc += 0xbfc00200;
		newpc += 0x80000000;
	}
	else
	{
		newpc += 0x80000000;
	}

	return newpc;
	
}

//---------------------------------------------------------------------------------------

//extern void Trigger_RSPBreak();
//extern void Trigger_DPInterrupt();
void CheckInterrupts()
{
    if ((MI_INTR_REG_R & MI_INTR_SP) != 0) Trigger_RSPBreak();
    if ((MI_INTR_REG_R & MI_INTR_DP) != 0) Trigger_DPInterrupt();
}

//---------------------------------------------------------------------------------------

void Handle_MI(uint32 value)
{
    if ((value & MI_INTR_MASK_SP_CLR))  MI_INTR_MASK_REG_R &= ~MI_INTR_SP; 
    if ((value & MI_INTR_MASK_SI_CLR))  MI_INTR_MASK_REG_R &= ~MI_INTR_SI;
    if ((value & MI_INTR_MASK_AI_CLR))  MI_INTR_MASK_REG_R &= ~MI_INTR_AI;
    if ((value & MI_INTR_MASK_VI_CLR))  MI_INTR_MASK_REG_R &= ~MI_INTR_VI;
    if ((value & MI_INTR_MASK_PI_CLR))  MI_INTR_MASK_REG_R &= ~MI_INTR_PI;
    if ((value & MI_INTR_MASK_DP_CLR))  MI_INTR_MASK_REG_R &= ~MI_INTR_DP;

    if ((value & MI_INTR_MASK_SI_SET))  MI_INTR_MASK_REG_R |=  MI_INTR_SI;
    if ((value & MI_INTR_MASK_VI_SET))  MI_INTR_MASK_REG_R |=  MI_INTR_VI;
    if ((value & MI_INTR_MASK_AI_SET))  MI_INTR_MASK_REG_R |=  MI_INTR_AI;
    if ((value & MI_INTR_MASK_PI_SET))  MI_INTR_MASK_REG_R |=  MI_INTR_PI;
    if ((value & MI_INTR_MASK_DP_SET))  MI_INTR_MASK_REG_R |=  MI_INTR_DP;
    if ((value & MI_INTR_MASK_SP_SET))  MI_INTR_MASK_REG_R |=  MI_INTR_SP;
}

//---------------------------------------------------------------------------------------

void Trigger_AIInterrupt(void)
{
#ifdef DEBUG_COMMON
	if( debug_interrupt && debug_ai_interrupt )
	{
		sprintf(generalmessage,"AI Interrupt is triggered");
		RefreshOpList(generalmessage);
	}
#endif
    // set the interrupt to fire
    (MI_INTR_REG_R) |= MI_INTR_AI;
    if ((MI_INTR_MASK_REG_R) & MI_INTR_AI)
	{
		gHardwareState.COP0Reg[CAUSE] |= 0x00000400;
#ifdef CPUCHECKINTR
		CPUNeedToCheckInterrupt = TRUE;
		CPUNeedToDoOtherTask = TRUE;
#endif
	}

}

extern void GetPluginDir(char* Directory);
void RunSPTask(void)
{
	sp_task_counter = 0;
	switch( sp_hle_task )
	{
	case GFX_TASK:
		__try{
            VIDEO_ProcessDList();
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
			char VideoPath[_MAX_PATH];
			char StartPath[_MAX_PATH];

			DisplayError("Memory exception fires to process VIDEO DList");
			
			CloseVideoPlugin();

			// Need to reload the VIDEO plugin

			GetPluginDir(StartPath);
			strcpy(VideoPath, StartPath);
			strcat(VideoPath, gRegSettings.VideoPlugin);

			LoadVideoPlugin(VideoPath);
			VIDEO_RomOpen();

		}
		VIupdated = TRUE;

			if ((MI_INTR_REG_R & MI_INTR_DP) != 0) Trigger_DPInterrupt();
#ifdef DEBUG_SP_TASK
			if( debug_sp_task )
			{
				sprintf(generalmessage, "SP GRX Task finished");
				RefreshOpList(generalmessage);
			}
#endif
		break;
	case SND_TASK:
		__try{
            AUDIO_ProcessAList();
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
			DisplayError("Memory exception fires to process AUDIO DList");
		}
			if ((MI_INTR_REG_R & MI_INTR_DP) != 0) Trigger_DPInterrupt();
#ifdef DEBUG_SP_TASK 
			if( debug_sp_task )
			{
				sprintf(generalmessage, "SP SND Task finished");
				RefreshOpList(generalmessage);
			}
#endif
			break;
	default:
#ifdef DEBUG_SP_TASK 
		if( debug_sp_task )
		{
			//sprintf(TempStr, "SP BAD Task finished");
			sprintf(generalmessage, "SP BAD Task skipped");
			RefreshOpList(generalmessage);
		}
#endif
		/*
		__try{
            VIDEO_ProcessDList();
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
			DisplayError("Memory exception fires to process Unknown DList");
		}
		

        CheckInterrupts();
		if ((MI_INTR_REG_R & MI_INTR_DP) != 0) Trigger_DPInterrupt();
		*/
		break;
	}

	Trigger_RSPBreak();

}



//---------------------------------------------------------------------------------------

void Handle_SP(uint32 value) 
{
    if (value & SP_CLR_HALT)
    {   
        (SP_STATUS_REG) &= ~SP_STATUS_HALT; 

#ifdef DEBUG_SP_TASK 
		if( debug_sp_task )
		{
			sprintf(generalmessage, "SP Task is triggered");
			RefreshOpList(generalmessage);
		}
#endif

		sp_hle_task = HLE_DMEM_TASK;

#ifdef DOSPTASKCOUNTER		
		sp_task_counter = SPTASKPCLOCKS;
		CPUNeedToDoOtherTask = TRUE;	// Set CPU to do other tasks
										// it should be OK if CPU is already doing some other tasks
#else
		RunSPTask();
#endif

    }
    if (value & SP_SET_HALT)        (SP_STATUS_REG) |=  SP_STATUS_HALT; 
    if (value & SP_CLR_BROKE)       (SP_STATUS_REG) &= ~SP_STATUS_BROKE;

    if (value & SP_CLR_INTR)
	{	
		(MI_INTR_REG_R) &= ~MI_INTR_SP;
		if( (MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0)   
		{
			gHardwareState.COP0Reg[CAUSE] &=  0xFFFFFBFF;
#ifdef CPUCHECKINTR
			CPUNeedToCheckInterrupt = FALSE;
#endif
		}
	}

    if (value & SP_SET_INTR)
	{
		(MI_INTR_REG_R) |= MI_INTR_SP;
		if( (MI_INTR_REG_R & MI_INTR_MASK_REG_R) != 0)   
		{
			gHardwareState.COP0Reg[CAUSE] |=  0x0400;
#ifdef CPUCHECKINTR
			CPUNeedToCheckInterrupt = TRUE;
#endif
		}
	}

    if (value & SP_CLR_SSTEP)       (SP_STATUS_REG) &= ~SP_STATUS_SSTEP;
    if (value & SP_SET_SSTEP)       (SP_STATUS_REG) |= SP_STATUS_SSTEP;
    
    if (value & SP_CLR_INTR_BREAK)  (SP_STATUS_REG) &= ~SP_STATUS_INTR_BREAK;
    if (value & SP_SET_INTR_BREAK)  (SP_STATUS_REG) |= SP_STATUS_INTR_BREAK;
    
    if (value & SP_CLR_YIELD)       (SP_STATUS_REG) &= ~SP_STATUS_YIELD;
    if (value & SP_SET_YIELD)       (SP_STATUS_REG) |= SP_STATUS_YIELD;
    
    if (value & SP_CLR_YIELDED)     (SP_STATUS_REG) &= ~SP_STATUS_YIELDED;
    if (value & SP_SET_YIELDED)     (SP_STATUS_REG) |= SP_STATUS_YIELDED;
    
    if (value & SP_CLR_TASKDONE)    (SP_STATUS_REG) &= ~SP_STATUS_TASKDONE;
    if (value & SP_SET_TASKDONE)    (SP_STATUS_REG) |= SP_STATUS_TASKDONE;
    
    if (value & SP_CLR_SIG3)        (SP_STATUS_REG) &= ~SP_STATUS_SIG3;
    if (value & SP_SET_SIG3)        (SP_STATUS_REG) |= SP_STATUS_SIG3;
    
    if (value & SP_CLR_SIG4)        (SP_STATUS_REG) &= ~SP_STATUS_SIG4;
    if (value & SP_SET_SIG4)        (SP_STATUS_REG) |= SP_STATUS_SIG4;
    
    if (value & SP_CLR_SIG5)        (SP_STATUS_REG) &= ~SP_STATUS_SIG5;
    if (value & SP_SET_SIG5)        (SP_STATUS_REG) |= SP_STATUS_SIG5;
    
    if (value & SP_CLR_SIG6)        (SP_STATUS_REG) &= ~SP_STATUS_SIG6;
    if (value & SP_SET_SIG6)        (SP_STATUS_REG) |= SP_STATUS_SIG6;
    
    if (value & SP_CLR_SIG7)        (SP_STATUS_REG) &= ~SP_STATUS_SIG7;
    if (value & SP_SET_SIG7)        (SP_STATUS_REG) |= SP_STATUS_SIG7;
}

//---------------------------------------------------------------------------------------

void Handle_DPC(uint32 value)
{
    if (value & DPC_CLR_XBUS_DMEM_DMA) (DPC_STATUS_REG) &= ~DPC_STATUS_XBUS_DMEM_DMA;
    if (value & DPC_SET_XBUS_DMEM_DMA) (DPC_STATUS_REG) |= DPC_STATUS_XBUS_DMEM_DMA;

    if (value & DPC_CLR_FREEZE) (DPC_STATUS_REG) &= ~DPC_STATUS_FREEZE;
    if (value & DPC_SET_FREEZE) (DPC_STATUS_REG) |= DPC_STATUS_FREEZE;

    if (value & DPC_CLR_FLUSH) (DPC_STATUS_REG) &= ~DPC_STATUS_FLUSH;
    if (value & DPC_SET_FLUSH) (DPC_STATUS_REG) |= DPC_STATUS_FLUSH;

    if (value & DPC_CLR_TMEM_REG) (DPC_TMEM_REG) = 0;
    if (value & DPC_CLR_PIPEBUSY_REG) (DPC_PIPEBUSY_REG) = 0;
    if (value & DPC_CLR_BUFBUSY_REG) (DPC_BUFBUSY_REG) = 0;
    if (value & DPC_CLR_CLOCK_REG) (DPC_CLOCK_REG) = 0;
}

//---------------------------------------------------------------------------------------

void Handle_PI() {
#ifdef DEBUG_COMMON
    sprintf(dbgString, "%08X: Clearing PI Interrupt", gHardwareState.pc);
    RefreshOpList(dbgString);
#endif

    if ((PI_STATUS_REG & 0x2))  MI_INTR_REG_R &= ~MI_INTR_PI;
    
    if (MI_INTR_REG_R != 0)   
	{
		gHardwareState.COP0Reg[CAUSE] |=  0x0400;
#ifdef CPUCHECKINTR
		CPUNeedToCheckInterrupt = TRUE;
		CPUNeedToDoOtherTask = TRUE;
#endif
	}
    else                        
	{
		gHardwareState.COP0Reg[CAUSE] &=  0xFFFFFBFF;
#ifdef CPUCHECKINTR
		CPUNeedToCheckInterrupt = FALSE;
#endif
	}
}

//---------------------------------------------------------------------------------------

void WriteMI_ModeReg(uint32 value)
{
    if (value & MI_CLR_RDRAM)    MI_INIT_MODE_REG_R  &= ~MI_MODE_RDRAM;
    if (value & MI_CLR_INIT)     MI_INIT_MODE_REG_R  &= ~MI_MODE_INIT;
    if (value & MI_CLR_EBUS)     MI_INIT_MODE_REG_R  &= ~MI_MODE_EBUS;
    if (value & MI_CLR_DP_INTR)  MI_INTR_REG_R       &= ~MI_INTR_DP;

    if (value & MI_SET_INIT)     MI_INIT_MODE_REG_R  |= MI_MODE_INIT;
    if (value & MI_SET_EBUS)     MI_INIT_MODE_REG_R  |= MI_MODE_EBUS;
    if (value & MI_SET_RDRAM)    MI_INIT_MODE_REG_R  |= MI_MODE_RDRAM;

    if ( (MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
	{
		gHardwareState.COP0Reg[CAUSE] &=  0xFFFFFBFF;
#ifdef CPUCHECKINTR
		CPUNeedToCheckInterrupt = FALSE;
#endif
	}
}


//---------------------------------------------------------------------------------------

void Trigger_CompareInterrupt(void)
{
#ifdef DEBUG_COMPARE_INTERRUPT
	if( debug_interrupt && debug_compare_interrupt )
	{
		sprintf(generalmessage, "COUNT Interrupt is triggered");
		RefreshOpList(generalmessage);
		sprintf(generalmessage, "COUNT = %08X, COMPARE= %08X, PC = %08X", gHardwareState.COP0Reg[COUNT], gHardwareState.COP0Reg[COMPARE], gHardwareState.pc);
		RefreshOpList(generalmessage);
	}
#endif
    // set the compare interrupt flag (ip7)
    gHardwareState.COP0Reg[CAUSE] |= 0x00008000;
#ifdef CPUCHECKINTR
	CPUNeedToCheckInterrupt = TRUE;
	CPUNeedToDoOtherTask = TRUE;
#endif
}

//---------------------------------------------------------------------------------------

void Trigger_DPInterrupt(void)
{
    // set the interrupt to fire
    (MI_INTR_REG_R) |= MI_INTR_DP;
    if ((MI_INTR_MASK_REG_R) & MI_INTR_DP)
	{
            gHardwareState.COP0Reg[CAUSE] |= 0x00000400;
#ifdef CPUCHECKINTR
			CPUNeedToCheckInterrupt = TRUE;
			CPUNeedToDoOtherTask = TRUE;
#endif
	}

}

//---------------------------------------------------------------------------------------
extern int AiUpdating;
void Trigger_VIInterrupt(void)
{
	// Speed Sync
	// If enabled, will slow down too much
	/*
	unsigned long currenttime;
	static unsigned long time_of_last_vi=0;

	currenttime = GetTickCount();
	if( currenttime - time_of_last_vi < 12 )
	{
		Sleep(12+time_of_last_vi-currenttime);
	}
	time_of_last_vi = currenttime;
	*/
	
#ifdef DEBUG_COMMON
	if( debug_interrupt && debug_vi_interrupt )
	{
		sprintf(generalmessage, "VI Interrupt is triggered");
		RefreshOpList(generalmessage);
	}
#endif

    // set the interrupt to fire
    (MI_INTR_REG_R) |= MI_INTR_VI;
    if ((MI_INTR_MASK_REG_R) & MI_INTR_VI)
	{
        gHardwareState.COP0Reg[CAUSE] |= 0x00000400;
#ifdef CPUCHECKINTR
		CPUNeedToCheckInterrupt = TRUE;
		CPUNeedToDoOtherTask = TRUE;
#endif
	}

	
	
	if( VIupdated )
	{
		VIDEO_UpdateScreen();
		VIupdated = FALSE;
	}
	else
	{
#ifdef DEBUG_COMMON
		if( debug_interrupt && debug_vi_interrupt )
		{
			sprintf(generalmessage, "No VI Update, Screen Update is skipped");
			RefreshOpList(generalmessage);
		}
#endif
	}

	framecounter++;

	if (AiUpdating)
		AUDIO_AiUpdate(FALSE);

}

//---------------------------------------------------------------------------------------


void Trigger_RSPBreak(void)
{
    // set the status flags
    (SP_STATUS_REG) |= 0x00000203;
    // set the sp interrupt when wanted
    if ((SP_STATUS_REG) & 0x0040) 
	{
        (MI_INTR_REG_R) |= MI_INTR_SP;
        if ((MI_INTR_MASK_REG_R) & MI_INTR_SP)
		{
            gHardwareState.COP0Reg[CAUSE] |= 0x00000400;
#ifdef CPUCHECKINTR
			CPUNeedToCheckInterrupt = TRUE;
			CPUNeedToDoOtherTask = TRUE;
#endif
		}
    }
}

