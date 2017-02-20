//#define __SPEED_HACK
#include "..\options.h"
#include "..\timer.h"
#include "..\memory.h"

#define __SPEED_HACK											\
		if(__dotI == -1)											\
			if((READ_CPU_INSTRUCTION((pc + 4))) == 0x00000000)	\
			{													\
				/*gHardwareState.cpr[0][COUNT] = next_interrupt_count; */ 	    \
				X86_CALL((_u32)DoSpeedHack);\
			}													\





void HELP_Call(unsigned long function)
{
    FlushAllRegisters();
    WC8(0xB9);
    WC32(gHardwareState.pc);

	X86_CALL(function);
}

extern uint32 VIcounter;

void DoSpeedHack(void)
{
	if( CPUNeedToDoOtherTask )	// Let CPU to finish all other tasks before SPEEDHACK
		return;

#ifdef SAVEVICOUNTER
	if( ( gHardwareState.COP0Reg[COUNT] + MAGICNUMBERFORCOUNTREG > gHardwareState.COP0Reg[COMPARE] ) &&
	  (gHardwareState.COP0Reg[COUNT] < gHardwareState.COP0Reg[COMPARE] ) )
	{
		Trigger_CompareInterrupt();
		VIcounter += (gHardwareState.COP0Reg[COMPARE] - gHardwareState.COP0Reg[COUNT] )*2;
		gHardwareState.COP0Reg[COUNT] = gHardwareState.COP0Reg[COMPARE];
	}
	else
	{
		gHardwareState.COP0Reg[COUNT] += (MAGICNUMBER - VIcounter ) / 2 ;
		VIcounter = MAGICNUMBER;
	}
#else
	VIcounter += MAGICNUMBER;
	gHardwareState.COP0Reg[COUNT] += MAGICNUMBER;
	if( ( gHardwareState.COP0Reg[COUNT] > gHardwareState.COP0Reg[COMPARE] &&
	  gHardwareState.COP0Reg[COUNT] - MAGICNUMBER < gHardwareState.COP0Reg[COMPARE] ) ||
	( gHardwareState.COP0Reg[COUNT] < gHardwareState.COP0Reg[COUNT] - MAGICNUMBER &&
	  gHardwareState.COP0Reg[COUNT] > gHardwareState.COP0Reg[COMPARE] ))
	{
		Trigger_CompareInterrupt();
	}
#endif
}

void DoBNELSpeedHack(uint32 Instruction)
{
	int grt1, grt2, grs1, grs2, inc, count;

	grt1 = RT_FT;
	grs1 = RS_BASE_FMT;
	Instruction = LOAD_UWORD_PARAM(gHardwareState.pc+4);
	grt2 = RT_FT;
	grs2 = RS_BASE_FMT;
	inc = (_int16)OFFSET_IMMEDIATE;
	count = ((uint32)gHardwareState.GPR[grt1] - (uint32)gHardwareState.GPR[grt2])/inc;


	//if( count < 4 )
	//	return;
	
	if( ( gHardwareState.COP0Reg[COUNT] + count > gHardwareState.COP0Reg[COMPARE] ) &&
	  (gHardwareState.COP0Reg[COUNT] < gHardwareState.COP0Reg[COMPARE] ) )
	{
		Trigger_CompareInterrupt();
		VIcounter += (gHardwareState.COP0Reg[COMPARE] - gHardwareState.COP0Reg[COUNT] )*2;
		gHardwareState.COP0Reg[COUNT] = gHardwareState.COP0Reg[COMPARE];
	}
	else
	{
		gHardwareState.COP0Reg[COUNT] += count ;
		VIcounter += (count*2);
	}
	

	gHardwareState.GPR[grt2] = gHardwareState.GPR[grt1] - inc;

}



