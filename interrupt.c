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


The project is a direct documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar :). Most source
code comments are taken directly from anarko's n64toolkit with consent and are 
the property of anarko.
*/


#include <windows.h>
#include <stdio.h>
#include "interrupt.h"
#include "globals.h"
#include "win32/DLL_Video.h"

#ifdef DEBUG_COMMON
	char dbgString[80];
#endif

void Handle_SP(uint32 value);
void Handle_MI(uint32 value);
void Handle_DPC(uint32 value);
void Handle_PI();
void Init_Bootstrap_Pointers();
extern void RefreshOpList(char *opcode);


extern uint32* InstructionPointer;

uint8* InterruptVector;
uint8* IVTStart;
uint8* TLBMissStart;

//---------------------------------------------------------------------------------------

uint32 SetException_Interrupt(uint32 pc)
{
	if (CPUdelay != 0)					/* are we in branch delay slot? */
	{										/* yes */
		COP0Reg[CAUSE] |= BD;
		COP0Reg[EPC] = pc - 4;
		CPUdelay = 0;
	}
	else
	{										/* no */			
		COP0Reg[CAUSE] &= NOT_BD;
		COP0Reg[EPC] = pc;
	}

	COP0Reg[STATUS] |= EXL;				/* set EXL = 1 */

	if ((COP0Reg[CAUSE] & TLBL_Miss) != 0)
	{
		COP0Reg[CAUSE] &= NOT_EXCCODE;	/* clear EXCCode  */
		return(0x80000080);
	}
	else
	{
		COP0Reg[CAUSE] &= NOT_EXCCODE;	/* clear EXCCode  */
		return(0x80000180);
	}
}

//---------------------------------------------------------------------------------------

extern void Trigger_RSPBreak();
extern void Trigger_DPInterrupt();
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

extern void Trigger_AIInterrupt();

//---------------------------------------------------------------------------------------

void Handle_SP(uint32 value) 
{
	if (value & SP_CLR_HALT)
	{	
		(SP_STATUS_REG) &= ~SP_STATUS_HALT; 
		if (HLE_DMEM_TASK == SND_TASK)
		{
			Trigger_RSPBreak();
			Trigger_AIInterrupt();
		}
		else if (HLE_DMEM_TASK == GFX_TASK)
		{
			VIDEO_ProcessDList();
			CheckInterrupts();
			Trigger_RSPBreak();
		}
		else
			DisplayError("Bad Task!!");
	}
	if (value & SP_SET_HALT)		(SP_STATUS_REG) |=  SP_STATUS_HALT;	
	if (value & SP_CLR_BROKE)		(SP_STATUS_REG) &= ~SP_STATUS_BROKE;

	if (value & SP_CLR_INTR)		(MI_INTR_REG_R) &= ~MI_INTR_SP;
	if (value & SP_SET_INTR)		(MI_INTR_REG_R) |= MI_INTR_SP;
	
	if (value & SP_CLR_SSTEP)		(SP_STATUS_REG) &= ~SP_STATUS_SSTEP;
	if (value & SP_SET_SSTEP)		(SP_STATUS_REG) |= SP_STATUS_SSTEP;
	
	if (value & SP_CLR_INTR_BREAK)	(SP_STATUS_REG) &= ~SP_STATUS_INTR_BREAK;
	if (value & SP_SET_INTR_BREAK)	(SP_STATUS_REG) |= SP_STATUS_INTR_BREAK;
	
	if (value & SP_CLR_YIELD)		(SP_STATUS_REG) &= ~SP_STATUS_YIELD;
	if (value & SP_SET_YIELD)		(SP_STATUS_REG) |= SP_STATUS_YIELD;
	
	if (value & SP_CLR_YIELDED)		(SP_STATUS_REG) &= ~SP_STATUS_YIELDED;
	if (value & SP_SET_YIELDED)		(SP_STATUS_REG) |= SP_STATUS_YIELDED;
	
	if (value & SP_CLR_TASKDONE)	(SP_STATUS_REG) &= ~SP_STATUS_TASKDONE;
	if (value & SP_SET_TASKDONE)	(SP_STATUS_REG) |= SP_STATUS_TASKDONE;
	
	if (value & SP_CLR_SIG3)		(SP_STATUS_REG) &= ~SP_STATUS_SIG3;
	if (value & SP_SET_SIG3)		(SP_STATUS_REG) |= SP_STATUS_SIG3;
	
	if (value & SP_CLR_SIG4)		(SP_STATUS_REG) &= ~SP_STATUS_SIG4;
	if (value & SP_SET_SIG4)		(SP_STATUS_REG) |= SP_STATUS_SIG4;
	
	if (value & SP_CLR_SIG5)		(SP_STATUS_REG) &= ~SP_STATUS_SIG5;
	if (value & SP_SET_SIG5)		(SP_STATUS_REG) |= SP_STATUS_SIG5;
	
	if (value & SP_CLR_SIG6)		(SP_STATUS_REG) &= ~SP_STATUS_SIG6;
	if (value & SP_SET_SIG6)		(SP_STATUS_REG) |= SP_STATUS_SIG6;
	
	if (value & SP_CLR_SIG7)		(SP_STATUS_REG) &= ~SP_STATUS_SIG7;
	if (value & SP_SET_SIG7)		(SP_STATUS_REG) |= SP_STATUS_SIG7;
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
	sprintf(dbgString, "%08X: Clearing PI Interrupt", pc);
	RefreshOpList(dbgString);
#endif

	if ((PI_STATUS_REG & 0x2))	MI_INTR_REG_R &= ~MI_INTR_PI;
	
	if (MI_INTR_REG_R != 0)		COP0Reg[CAUSE] |=  0x0400;
	else						COP0Reg[CAUSE] &=  0xFFFFFBFF;
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

	if ( (MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0)	COP0Reg[CAUSE] &=  0xFFFFFBFF;
}


//---------------------------------------------------------------------------------------

void Trigger_CompareInterrupt(void)
{
	// set the compare interrupt flag (ip7)
	COP0Reg[CAUSE] |= 0x8000;
}

//---------------------------------------------------------------------------------------

void Trigger_DPInterrupt(void)
{
	// set the interrupt to fire
	(MI_INTR_REG_R) |= MI_INTR_DP;
	if ((MI_INTR_MASK_REG_R) & MI_INTR_DP)
		COP0Reg[CAUSE] |= 0x00000400;
}

//---------------------------------------------------------------------------------------

void Trigger_VIInterrupt(void)
{

	// set the interrupt to fire
	(MI_INTR_REG_R) |= MI_INTR_VI;
	if ((MI_INTR_MASK_REG_R) & MI_INTR_VI)
		COP0Reg[CAUSE] |= 0x00000400;
	if (FoundHLE == 0)
	{
    VIDEO_UpdateScreen();
	}
	COP0Reg[COUNT] = 0;
}

//---------------------------------------------------------------------------------------

void Trigger_AIInterrupt(void)
{

	// set the interrupt to fire
	(MI_INTR_REG_R) |= MI_INTR_AI;
	if ((MI_INTR_MASK_REG_R) & MI_INTR_AI)
		COP0Reg[CAUSE] |= 0x00000400;
}

//---------------------------------------------------------------------------------------

void Trigger_RSPBreak(void)
{
	// set the status flags
	(SP_STATUS_REG) |= 0x00000203;
	// set the sp interrupt when wanted
	if ((SP_STATUS_REG) & 0x0040) {
		(MI_INTR_REG_R) |= MI_INTR_SP;
		if ((MI_INTR_MASK_REG_R) & MI_INTR_SP)
			COP0Reg[CAUSE] |= 0x00000400;
	}
}
