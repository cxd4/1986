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
#include "memory.h"
#include "timer.h"
#include "hardware.h"
#include "dma.h"
#include "debug_option.h"
#include "emulator.h"
#include "1964ini.h"
#include "dynarec/dynarec.h"
#include "win32/DLL_Video.h"
#include "win32/DLL_Audio.h"
#include "win32/registry.h"
#include "win32/wingui.h"
#include "win32/resource.h"
#include "win32/windebug.h"
#include <stdio.h>  
#include <stdlib.h>

#ifdef DEBUG_COMMON
char * Get_Interrupt_Name()
{
	uint32 cause;
	
	if( gHWS_COP0Reg[CAUSE] & 0x00007B00 )
	{
		DisplayError("Invalid interrupt bits set, CAUSE reg = %08X", gHWS_COP0Reg[CAUSE]);
		return "Invalid";
	}

	cause = (gHWS_COP0Reg[CAUSE] & 0x00008400);
	switch( cause )
	{
	case 0x00008000:
		return "Compare";
		break;
	case 0x00000400:
		switch ( MI_INTR_REG_R & 0x0000003F )
		{
		case MI_INTR_SP:
			return "SP";
			break;
		case MI_INTR_SI:
			return "SI";
			break;
		case MI_INTR_AI:
			return "AI";
			break;
		case MI_INTR_VI:
			return "VI";
			break;
		case MI_INTR_PI:
			return "PI";
			break;
		case MI_INTR_DP:
			return "DP";
			break;
		case MI_INTR_DP|MI_INTR_SP:
			return "DP&SP";
			break;
		default:
			if( (MI_INTR_REG_R & 0x0000003F) == 0 )
			{
				DisplayError("No MI interrupt as interrupt is triggered, MI_INTR_REG = %08X", MI_INTR_REG_R);
				return "No MI";
			}
			else
			{
				DisplayError("Warning: Multiple MI interrupt is triggered at the same time, MI_INTR_REG = %08X", MI_INTR_REG_R);
				return "Invalid MI";
			}
			break;
		}
		break;
	default:
		if( cause == 0x00008400 )
		{
			//DisplayError("Warning, both COMPARE and MI interrupt happens together, could lose one");
			return "COMPARE&MI";
			break;
		}
		else
		{
			DisplayError("Warning, invalid interrupts, CAUSE=%08X", gHWS_COP0Reg[CAUSE]);
			return "Invalid";
		}
		break;
	}
}
#endif
//---------------------------------------------------------------------------------------
// This function is called when a Exception Interrupt happens, will set COP0 registers
// EPC and CAUSE, then return a correct interrupt vector address
// 
uint32 SetException_Interrupt(uint32 pc)
{
	uint32 newpc = 0x180;

#ifdef FAST_COUNTER
	Count_Down(4*VICounterFactors[CounterFactor]);
#else
	// Any Interrupt and Exceptions will result delay 4 PCLOCKs
	VIcounter+= 4*VICounterFactors[CounterFactor];
	gHWS_COP0Reg[COUNT] += 2*CounterFactors[CounterFactor];
#endif


    if (CPUdelay != 0)                  /* are we in branch delay slot? */
    {                                       /* yes */
        gHWS_COP0Reg[CAUSE] |= BD;
        gHWS_COP0Reg[EPC] = pc - 4;
        CPUdelay = 0;
    }
    else
    {                                       /* no */            
        gHWS_COP0Reg[CAUSE] &= NOT_BD;
        gHWS_COP0Reg[EPC] = pc;
    }

    gHWS_COP0Reg[STATUS] |= EXL;              // set EXL = 1
														// to disable further interrupts

    if ( (gHWS_COP0Reg[CAUSE] & TLBL_Miss) || (gHWS_COP0Reg[CAUSE] & TLBS_Miss))
	{
		newpc = 0x80;
    }

    //gHWS_COP0Reg[CAUSE] &= NOT_EXCCODE;   /* clear EXCCode  */

	if( gHWS_COP0Reg[STATUS] & (1 << 22) )
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

void CheckInterrupts()
{
	if ((MI_INTR_REG_R & MI_INTR_SP) != 0) 
	{
		OPCODE_DEBUGGER_BEGIN_EPILOGUE
		Trigger_RSPBreak();
		OPCODE_DEBUGGER_END_EPILOGUE
	}

    if ((MI_INTR_REG_R & MI_INTR_DP) != 0)
	{
		OPCODE_DEBUGGER_BEGIN_EPILOGUE
		Trigger_DPInterrupt();
		OPCODE_DEBUGGER_END_EPILOGUE
	}

	/*
    if ((MI_INTR_REG_R & MI_INTR_VI) != 0) 
	{
		OPCODE_DEBUGGER_BEGIN_EPILOGUE
		Trigger_VIInterrupt();
		OPCODE_DEBUGGER_END_EPILOGUE
	}
	*/

    if ((MI_INTR_REG_R & MI_INTR_AI) != 0) 
	{
		OPCODE_DEBUGGER_BEGIN_EPILOGUE
		Trigger_AIInterrupt();
		OPCODE_DEBUGGER_END_EPILOGUE
	}
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

	// Check MI interrupt again, this is important, otherwise we will lost interrupts
	if( MI_INTR_MASK_REG_R & 0x0000003F & MI_INTR_REG_R ) 
	{
		// Trigger an MI interrupt, don't know what it is
		SET_EXCEPTION(EXC_INT)
        gHWS_COP0Reg[CAUSE] |= CAUSE_IP3;
		HandleInterrupts(0x80000180);
	}
}

//---------------------------------------------------------------------------------------

void RunSPTask(void)
{
	sp_task_counter = 0;

	switch( sp_hle_task )
	{
	case GFX_TASK:
		__try{
			VIDEO_ProcessDList();
			DListCount++;
			DPC_STATUS_REG = 0x801;		// Makes Banjo Kazooie work - Azimer
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
		}

		if ((MI_INTR_REG_R & MI_INTR_DP) != 0) Trigger_DPInterrupt();
#ifdef DEBUG_SP_TASK
		if( debug_sp_task )
		{
			TRACE0( "SP GRX Task finished")
		}
#endif
		break;
	case SND_TASK:
		__try{
            AUDIO_ProcessAList();
			AListCount++;
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
			//DisplayError("Memory exception fires to process AUDIO DList");
		}
			if ((MI_INTR_REG_R & MI_INTR_DP) != 0) Trigger_DPInterrupt();
#ifdef DEBUG_SP_TASK 
			if( debug_sp_task )
			{
				TRACE0( "SP SND Task finished");
			}
#endif
			break;
	default:
		__try{
			VIDEO_ProcessRDPList();
			//VIDEO_ProcessDList();

        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
			//DisplayError("Memory exception fires to process RDP List");
		}

		if ((MI_INTR_REG_R & MI_INTR_DP) != 0) Trigger_DPInterrupt();

#ifdef DEBUG_SP_TASK 
		if( debug_sp_task )
		{
			//TRACE0( "SP BAD Task skipped");
			TRACE0( "Unknown SP Taks");
		}
#endif
		break;
	}

	Trigger_RSPBreak();

}



//---------------------------------------------------------------------------------------

void Handle_SP(uint32 value) 
{
    if (value & SP_SET_HALT)        (SP_STATUS_REG) |=  SP_STATUS_HALT; 
    if (value & SP_CLR_BROKE)       (SP_STATUS_REG) &= ~SP_STATUS_BROKE;

    if (value & SP_CLR_INTR)
	{	
		(MI_INTR_REG_R) &= ~MI_INTR_SP;
		if( (MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0)   
		{
			gHWS_COP0Reg[CAUSE] &=  0xFFFFFBFF;
			//CPUNeedToCheckInterrupt = FALSE;
			//CPUNeedToDoOtherTask = TRUE;	// Set CPU to do other tasks
		}
	}

    if (value & SP_SET_INTR)
	{
		(MI_INTR_REG_R) |= MI_INTR_SP;
		if( (MI_INTR_REG_R & MI_INTR_MASK_REG_R) != 0)   
		{
			SET_EXCEPTION(EXC_INT)
			gHWS_COP0Reg[CAUSE] |=  CAUSE_IP3;
			HandleInterrupts(0x80000180);
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

    if (value & SP_CLR_HALT)		
	{
		(SP_STATUS_REG) &= ~SP_STATUS_HALT;

#ifdef DEBUG_SP_TASK 
		if( debug_sp_task )
		{
			TRACE0( "SP Task is triggered");
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

	// Add by Rice, 2001.08.10
	SP_STATUS_REG |= SP_STATUS_HALT;
}

//---------------------------------------------------------------------------------------

void Handle_DPC(uint32 value)
{
    if (value & DPC_CLR_XBUS_DMEM_DMA) (DPC_STATUS_REG) &= ~DPC_STATUS_XBUS_DMEM_DMA;
    if (value & DPC_SET_XBUS_DMEM_DMA) (DPC_STATUS_REG) |= DPC_STATUS_XBUS_DMEM_DMA;

    if (value & DPC_CLR_FREEZE) (DPC_STATUS_REG) &= ~DPC_STATUS_FREEZE;

	// Modified by Rice. 2001.08.10
    //if (value & DPC_SET_FREEZE) (DPC_STATUS_REG) |= DPC_STATUS_FREEZE;

    if (value & DPC_CLR_FLUSH) (DPC_STATUS_REG) &= ~DPC_STATUS_FLUSH;
    if (value & DPC_SET_FLUSH) (DPC_STATUS_REG) |= DPC_STATUS_FLUSH;

    if (value & DPC_CLR_TMEM_REG) (DPC_TMEM_REG) = 0;
    if (value & DPC_CLR_PIPEBUSY_REG) (DPC_PIPEBUSY_REG) = 0;
    if (value & DPC_CLR_BUFBUSY_REG) (DPC_BUFBUSY_REG) = 0;
    if (value & DPC_CLR_CLOCK_REG) (DPC_CLOCK_REG) = 0;
}

//---------------------------------------------------------------------------------------

void Handle_PI() 
{
    TRACE1("%08X: Clearing PI Interrupt", gHWS_pc);

    if ((PI_STATUS_REG & 0x2))  MI_INTR_REG_R &= ~MI_INTR_PI;
    
    if (MI_INTR_REG_R != 0)   
	{
		SET_EXCEPTION(EXC_INT)
		gHWS_COP0Reg[CAUSE] |=  CAUSE_IP3;
		CPUNeedToCheckInterrupt = TRUE;
		CPUNeedToDoOtherTask = TRUE;
	}
    else                        
	{
		gHWS_COP0Reg[CAUSE] &=  0xFFFFFBFF;
		//CPUNeedToCheckInterrupt = FALSE;
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
		gHWS_COP0Reg[CAUSE] &=  0xFFFFFBFF;
		//CPUNeedToCheckInterrupt = FALSE;
	}
}


//---------------------------------------------------------------------------------------

void Trigger_AIInterrupt(void)
{
	DEBUG_AI_INTERRUPT_TRACE(TRACE0("AI Interrupt is triggered"););

    // set the interrupt to fire
    (MI_INTR_REG_R) |= MI_INTR_AI;
    if ((MI_INTR_MASK_REG_R) & MI_INTR_AI)
	{
		SET_EXCEPTION(EXC_INT)
		gHWS_COP0Reg[CAUSE] |= CAUSE_IP3;
		HandleInterrupts(0x80000180);
	}

}

void Trigger_CompareInterrupt(void)
{
	DEBUG_COMPARE_INTERRUPT_TRACE(
		TRACE0("COUNT Interrupt is triggered");
		TRACE3("COUNT = %08X, COMPARE= %08X, PC = %08X", gHWS_COP0Reg[COUNT], gHWS_COP0Reg[COMPARE], gHWS_pc);
	);
    // set the compare interrupt flag (ip7)
	SET_EXCEPTION(EXC_INT)
    gHWS_COP0Reg[CAUSE] |= CAUSE_IP8;

	HandleInterrupts(0x80000180);
}

//---------------------------------------------------------------------------------------

void Trigger_DPInterrupt(void)
{
    // set the interrupt to fire
    (MI_INTR_REG_R) |= MI_INTR_DP;
    if ((MI_INTR_MASK_REG_R) & MI_INTR_DP)
	{
		SET_EXCEPTION(EXC_INT)
        gHWS_COP0Reg[CAUSE] |= CAUSE_IP3;
		HandleInterrupts(0x80000180);
	}
}

//---------------------------------------------------------------------------------------

LARGE_INTEGER LastVITime = { 0,0 };
LARGE_INTEGER LastSecondTime = { 0,0 };
LARGE_INTEGER Freq;
LARGE_INTEGER CurrentTime;
LARGE_INTEGER Elapsed;
double sleeptime;
double tempvips;

void Trigger_VIInterrupt(void)
{
#ifdef ENABLE_OPCODE_DEBUGGER
	if( p_gHardwareState == &gHardwareState )	
		//only do the VI updatescreen for dyna, not for interpreter compare
#endif
	{
		DEBUG_VI_INTERRUPT_TRACE(TRACE0( "VI Interrupt is triggered"););
		//DEBUG_VI_INTERRUPT_TRACE(TRACE1("VI counter = %08X", Get_VIcounter());
		VIDEO_UpdateScreen();
		framecounter++;
		if (AiUpdating==1)
			AUDIO_AiUpdate(FALSE);
	}

	// Speed Sync
	if( currentromoptions.Max_FPS != MAXFPS_NONE )
	{
		QueryPerformanceCounter(&CurrentTime);
		Elapsed.QuadPart = CurrentTime.QuadPart - LastSecondTime.QuadPart;
		tempvips = framecounter / ((double)Elapsed.QuadPart / (double)Freq.QuadPart);

		if( tempvips > vips_speed_limits[currentromoptions.Max_FPS]+1.0)
		{
			//TRACE1("tempvips = %f", (float)tempvips);
			Elapsed.QuadPart = CurrentTime.QuadPart - LastVITime.QuadPart;

			sleeptime = 1000.00f/(vips_speed_limits[currentromoptions.Max_FPS]) - (double)Elapsed.QuadPart / (double)Freq.QuadPart * 1000.00;
			do{
				if( sleeptime > 0 )
				{
					if( sleeptime > 1.5 )
						Sleep(1);
					else
					{
						uint32 i;
						for( i= 0; i< 1000*sleeptime; i++ )	{ ; }	//busy wait
					}
				}

				QueryPerformanceCounter(&CurrentTime);
				Elapsed.QuadPart = CurrentTime.QuadPart - LastVITime.QuadPart;
				sleeptime = 1000.00f/(vips_speed_limits[currentromoptions.Max_FPS]) - (float)Elapsed.QuadPart / (float)Freq.QuadPart * 1000.00;
			}while(sleeptime>0.1);
		}
		LastVITime = CurrentTime;
	}

    // set the interrupt to fire
    (MI_INTR_REG_R) |= MI_INTR_VI;
    if ((MI_INTR_MASK_REG_R) & MI_INTR_MASK_VI)
	{
		SET_EXCEPTION(EXC_INT)
        gHWS_COP0Reg[CAUSE] |= CAUSE_IP3;
		HandleInterrupts(0x80000180);
	}
}

void Trigger_SIInterrupt(void)
{
	MI_INTR_REG_R  |= MI_INTR_SI;

	if ((MI_INTR_MASK_REG_R) & MI_INTR_MASK_SI)
	{
		SET_EXCEPTION(EXC_INT)
		gHWS_COP0Reg[CAUSE] |= CAUSE_IP3;
		HandleInterrupts(0x80000180);
	}
}

void Trigger_PIInterrupt(void)
{
	MI_INTR_REG_R |= MI_INTR_PI;	// Set PI Interrupt

    if ((MI_INTR_MASK_REG_R) & MI_INTR_MASK_PI)
	{
		SET_EXCEPTION(EXC_INT)
		gHWS_COP0Reg[CAUSE] |= CAUSE_IP3;
		HandleInterrupts(0x80000180);
	}
}
//---------------------------------------------------------------------------------------
void Trigger_SPInterrupt(void)
{
	MI_INTR_REG_R  |= MI_INTR_SP;

	if ((MI_INTR_MASK_REG_R) & MI_INTR_MASK_SP)
	{
		SET_EXCEPTION(EXC_INT)
		gHWS_COP0Reg[CAUSE] |= CAUSE_IP3;
		HandleInterrupts(0x80000180);
	}
}

void Trigger_RSPBreak(void)
{
    // set the status flags
    (SP_STATUS_REG) |= 0x00000203;
    // set the sp interrupt when wanted
    if ((SP_STATUS_REG) & 0x0040) 
	{
		Trigger_SPInterrupt();
    }


	// Add by Rice 2001.08.10
	SP_STATUS_REG |= SP_STATUS_HALT;
}

void Trigger_Address_Error_Exception(uint32 addr)
{
	SET_EXCEPTION(EXC_RADE)
	gHWS_COP0Reg[BADVADDR] = addr;

	//TRACE0("Should fire Address Error Exception, but we skipped here");
	TRACE0("Fires Address Error Exception");

	//HandleExceptions(0x80000180);
}

void TriggerFPUUnusableException()
{
	if( (gHWS_COP0Reg[STATUS] & SR_CU1) )
		return;

	SET_EXCEPTION(EXC_CPU)
	gHWS_COP0Reg[CAUSE] &= 0xCFFFFFFF;
	gHWS_COP0Reg[CAUSE] |= CAUSE_CE1;

	// Should test the CPU Delay slot bit
	//gHWS_COP0Reg[EPC] = gHWS_pc;

	TRACE0("FPU Unusable Exception");
	HandleExceptions(0x80000180);
}

// This exception is triggered when integer overflow or divided by zero condition happens
void TriggerIntegerOverflowException()
{
	SET_EXCEPTION(EXC_OV)
	TRACE0("Integer Overflow Exception is triggered");
	HandleExceptions(0x80000180);
}

void TriggerGeneralFPUException()
{
	SET_EXCEPTION(EXC_FPE)
	TRACE0("FPU Exception is triggered");
	HandleExceptions(0x80000180);
}

void HandleInterrupts(uint32 vt)
{
	// Set flag for interrupt service
	CPUNeedToCheckInterrupt = TRUE;
	CPUNeedToDoOtherTask = TRUE;
}

void HandleExceptions(uint32 evt)
{
	if( gHWS_COP0Reg[STATUS] & EXL_OR_ERL )	// Exception in exception
	{
		TRACE1("Warning, Exception happens in exception, the new exception is %d", (0x7C&gHWS_COP0Reg[CAUSE])>>2);
	}

	Dyna_Exception_Service_Routine(evt);
}

// This function is for debug purpose
void Trigger_Interrupt_Without_Mask(uint32 interrupt)
{
    // set the interrupt to fire
    (MI_INTR_REG_R) |= interrupt;
	SET_EXCEPTION(EXC_INT)
    gHWS_COP0Reg[CAUSE] |= CAUSE_IP3;
	HandleInterrupts(0x80000180);
}




