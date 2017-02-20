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
#include <memory.h>
#include "options.h"
#include "interrupt.h"
#include "globals.h"
#include "r4300i.h"
#include "n64rcp.h"
#include "dma.h"
#include "debug_option.h"
#include "timer.h"
#include "iPIF.h"
#include "timer.h"
#include "emulator.h"
#include "hardware.h"
#include "win32/Dll_Audio.h"

#ifdef DEBUG_COMMON
#include <stdio.h>
extern void RefreshOpList(char *opcode);
#endif

#ifdef DODMASEGMENT
BOOL  DMAInProgress=FALSE;
enum DMATYPE PIDMAInProgress=NO_DMA_IN_PROGRESS;
enum DMATYPE SIDMAInProgress=NO_DMA_IN_PROGRESS;
enum DMATYPE SPDMAInProgress=NO_DMA_IN_PROGRESS;

uint32 PIDMASourceAddress=0;
uint32 PIDMATargetAddress=0;
uint32 PIDMACurrentPosition=0;
uint32 PIDMALength=0;

uint32 SPDMASourceAddress=0;
uint32 SPDMATargetAddress=0;
uint32 SPDMACurrentPosition=0;
uint32 SPDMALength=0;
uint32 DMA_SP_Transfer_Source_Begin_Address = 0;
uint32 DMA_SP_Transfer_Target_Begin_Address = 0;
int DMA_SP_Transfer_Count = 0;


uint32 SIDMASourceAddress=0;
uint32 SIDMATargetAddress=0;
//uint32 SIDMACurrentPosition=0;
//uint32 SIDMALength=0;

int DMA_SI_Transfer_Count = 0;


#endif

extern uint32 CalculateAddress(uint32 Addr);
extern void dlist_check();
extern void iPifCheck(void);

extern void (__stdcall* DLL_3D_About)(HWND);

void CheckControllers();

/*__________________________________________________________________________
 |                                                                          |
 |               Peripheral Interface (PI) DMA Read/Write                   |
 |__________________________________________________________________________|


  _______________________________________________________
 |                                                       |
 | Peripheral Interface (PI) DMA Read                    |
 |_______________________________________________________|
 |                 |                   |                 |
 |       To        |       From        |      Size       |
 |_________________|___________________|_________________|
 |                 |                   |                 |
 |PI_CART_ADDR_REG | PI_DRAM_ADDR_REG  | PI_RD_LEN_REG+1 |
 |_________________|___________________|_________________|
*/
void DMA_MemCopy_PI_To_DRAM()
{
#ifdef DEBUG_PI_DMA 
	if( debug_pi_dma )
	{
		sprintf(generalmessage, "%08X: RD_PI  %d bytes %08X to %08X", gHardwareState.pc, PI_RD_LEN_REG+1, PI_DRAM_ADDR_REG, PI_CART_ADDR_REG);
		RefreshOpList(generalmessage);
	}
#endif

#ifdef DEBUG_SRAM
	if( debug_sram )
	{
		sprintf(generalmessage, "SRAM or ROM Write");
		RefreshOpList(generalmessage);
	}
#endif

#ifdef DODMASEGMENT
	//Setup DMA transfer in segments
	PIDMAInProgress = DMA_PI_READ;
	DMAInProgress = TRUE;
	CPUNeedToDoOtherTask = TRUE;

	PIDMASourceAddress = PI_DRAM_ADDR_REG & 0x00FFFFFF;
	PIDMATargetAddress = PI_CART_ADDR_REG & 0x1FFFFFFF;
	PIDMACurrentPosition = 0;;
	PIDMALength = (PI_RD_LEN_REG & 0x00FFFFFF) + 1;

	PI_STATUS_REG |= PI_STATUS_DMA_BUSY;	// Set PI status register DMA busy
#else

	if( (PI_CART_ADDR_REG & 0xFF000000) == 0x08000000 )
		memcpy(     &C2A2[PI_CART_ADDR_REG & 0x000FFFFF],
					&RDRAM[PI_DRAM_ADDR_REG & 0x0FFFFFFF], 
					(PI_RD_LEN_REG & 0x00FFFFFF) + 1);

	else
	    memcpy(     &ROM_Image[PI_CART_ADDR_REG & 0x0FFFFFFF],
					&RDRAM[PI_DRAM_ADDR_REG & 0x0FFFFFFF], 
					(PI_RD_LEN_REG & 0x00FFFFFF) + 1);

    //SB 6/6/99
    /* put a PI interrupt on the stack */
    MI_INTR_REG_R |= MI_INTR_PI;
    gHardwareState.COP0Reg[CAUSE] |= 0x400;
#ifdef CPUCHECKINTR
	CPUNeedToCheckInterrupt = TRUE;
	CPUNeedToDoOtherTask = TRUE;
#endif


#endif

}



/*_______________________________________________________
 |                                                       |
 | Peripheral Interface (PI) DMA Write                   |
 |_______________________________________________________|
 |                 |                   |                 |
 |       To        |       From        |      Size       |
 |_________________|___________________|_________________|
 |                 |                   |                 |
 |PI_DRAM_ADDR_REG | PI_CART_ADDR_REG  | PI_WR_LEN_REG+1 |
 |_________________|___________________|_________________|
*/
void DMA_MemCopy_DRAM_To_PI() 
{

	uint32 len = (PI_WR_LEN_REG & 0x00FFFFFF) +1;
	uint32 pi_dram_addr_reg = PI_DRAM_ADDR_REG & 0x00FFFFFF;
	uint32 pi_cart_addr_reg = PI_CART_ADDR_REG & 0x0FFFFFFF;

#ifdef DEBUG_PI_DMA 
	if( debug_pi_dma )
	{
		sprintf(generalmessage, "%08X: WR_PI %db from %08X to %08X", gHardwareState.pc, len, PI_CART_ADDR_REG, PI_DRAM_ADDR_REG);
		RefreshOpList(generalmessage);
	}
#endif

	// Clear Dynacomplied code
	__try
	{
		memset(&DynaRDRAM[PI_DRAM_ADDR_REG & 0x00FFFFFF],0,len);
	}
	__except(NULL,EXCEPTION_EXECUTE_HANDLER)
	{
#ifdef DEBUG_PI_DMA
		sprintf(generalmessage, "Bad PI DMA address, PI DMA skipped");
		RefreshOpList(generalmessage);
#endif
		MI_INTR_REG_R  |= MI_INTR_PI;
		gHardwareState.COP0Reg[CAUSE] |= 0x400;
		DisplayError("Bad PI DMA task");
		return;
	}


#ifdef DODMASEGMENT
	//Setup DMA transfer in segments
	PIDMAInProgress = DMA_PI_WRITE;
	DMAInProgress = TRUE;
	CPUNeedToDoOtherTask = TRUE;

	PIDMASourceAddress = PI_CART_ADDR_REG & 0x1FFFFFFF;
	PIDMATargetAddress = PI_DRAM_ADDR_REG & 0x00FFFFFF;
	PIDMACurrentPosition = 0;;
	PIDMALength = len;

	PI_STATUS_REG |= PI_STATUS_DMA_BUSY;
#else

	if( (PI_CART_ADDR_REG & 0xFF000000) == 0x08000000 )
	    memcpy(		&RDRAM[PI_DRAM_ADDR_REG & 0x00FFFFFF], 
					&C2A2[PI_CART_ADDR_REG & 0x000FFFFF],
					len);
	else
	{
		if( pi_cart_addr_reg >= gAllocationLength )
		{
			DisplayError("DMA Write PI, out of range");
		}
	    memcpy(		&RDRAM[PI_DRAM_ADDR_REG & 0x00FFFFFF],
					&ROM_Image[pi_cart_addr_reg],
					len);
	}



/*
    _asm {
    mov edi, offset RDRAM
    add edi, pi_dram_addr_reg
    mov ecx, offset ROM_Image
    add ecx, pi_cart_addr_reg
    mov ebx, 0
    }

_Label1:
    _asm {
    cmp ebx, len
    je _Label2
    mov eax, dword ptr [ecx]
    mov dword ptr [edi],eax
    add ebx, 4
    add ecx, 4
    add edi, 4
    jmp _Label1
    }

_Label2:
*/
    //SB 6/6/99
    /* put a PI interrupt on the stack */
    
    MI_INTR_REG_R  |= MI_INTR_PI;
    gHardwareState.COP0Reg[CAUSE] |= 0x400;
#ifdef CPUCHECKINTR
	CPUNeedToCheckInterrupt = TRUE;
	CPUNeedToDoOtherTask = TRUE;
#endif
#endif // DODMASEGMENT
}

/*__________________________________________________________________________
 |                                                                          |
 |                   Signal Processor (SP) DMA Read/Write                   |
 |__________________________________________________________________________|


  _______________________________________________________
 |                                                       |
 | Signal Processor (SP) DMA Read                        |
 |_______________________________________________________|
 |                 |                   |                 |
 |       To        |       From        |      Size       |
 |_________________|___________________|_________________|
 |                 |                   |                 |
 | SP_MEM_ADDR_REG | SP_DRAM_ADDR_REG  | SP_RD_LEN_REG+1 |
 |_________________|___________________|_________________|
*/
void DMA_MemCopy_DRAM_To_SP() 
{
#ifndef DODMASEGMENT
	uint16 segment;
	uint32 Offset;
	uint32 sp_mem_addr_reg = SP_MEM_ADDR_REG;
#endif

#ifdef DEBUG_SP_DMA 
	if( debug_sp_dma )
	{
		sprintf(generalmessage, "SP DMA Read  %d bytes from %08X to %08X", SP_RD_LEN_REG+1, SP_DRAM_ADDR_REG, SP_MEM_ADDR_REG );
		RefreshOpList(generalmessage);
	}
#endif


#ifdef DODMASEGMENT
		//Setup DMA transfer in segments
	SPDMAInProgress = DMA_SP_READ;
	DMAInProgress = TRUE;
	CPUNeedToDoOtherTask = TRUE;

	SPDMASourceAddress = SP_DRAM_ADDR_REG & 0x00FFFFFF;
	SPDMATargetAddress = SP_DMEM_START + (SP_MEM_ADDR_REG & 0x00001FFF);
	SPDMACurrentPosition = 0;;
	SPDMALength = (SP_RD_LEN_REG & 0x00000FFF) + 1;	// SP_RD_LEN_REG bit [0-11] is length to transfer

	SP_DMA_BUSY_REG = 1;
    SP_STATUS_REG |= SP_STATUS_DMA_BUSY;

	DMA_SP_Transfer_Source_Begin_Address = SPDMASourceAddress;
	DMA_SP_Transfer_Target_Begin_Address = SPDMATargetAddress;
	DMA_SP_Transfer_Count = (SP_RD_LEN_REG>>12)&0x000000FF;	// Bit [12-19] is for count

#else

    segment = (uint16)(sp_mem_addr_reg >> 16);
    Offset = (uint16)sp_mem_addr_reg;

    memcpy(((uint32*)((uint8*)sDWORD_R[segment]+(Offset))),
            &RDRAM[SP_DRAM_ADDR_REG & 0x00FFFFFF],
            SP_RD_LEN_REG+1);

    
    //SB 6/6/99
    /* put an SP interrupt on the stack */
    
    SP_DMA_BUSY_REG = 0;
#endif
}



/*_______________________________________________________
 |                                                       |
 | Signal Processor (SP) DMA Write                       |
 |_______________________________________________________|
 |                 |                   |                 |
 |       To        |       From        |      Size       |
 |_________________|___________________|_________________|
 |                 |                   |                 |
 |SP_DRAM_ADDR_REG |  SP_MEM_ADDR_REG  | SP_WR_LEN_REG+1 |
 |_________________|___________________|_________________|
*/
void DMA_MemCopy_SP_to_DRAM()
{
#ifndef DODMASEGMENT
	uint16 segment;
#endif

#ifdef DEBUG_SP_DMA 
	if( debug_sp_dma )
	{
		sprintf(generalmessage, "SP DMA Write %d bytes from %08X to %08X", SP_WR_LEN_REG+1, SP_MEM_ADDR_REG, SP_DRAM_ADDR_REG);
		RefreshOpList(generalmessage);
	}
#endif

#ifdef DODMASEGMENT
		//Setup DMA transfer in segments
	DMAInProgress = TRUE;
	SPDMAInProgress = DMA_SP_WRITE;
	CPUNeedToDoOtherTask = TRUE;

	SPDMASourceAddress = SP_DMEM_START + (SP_MEM_ADDR_REG & 0x00001FFF);
	SPDMATargetAddress = SP_DRAM_ADDR_REG & 0x00FFFFFF;
	SPDMACurrentPosition = 0;;
	SPDMALength = (SP_WR_LEN_REG & 0x00000FFF) + 1;	// SP_RD_LEN_REG bit [0-11] is length to transfer

	SP_DMA_BUSY_REG = 1;
    SP_STATUS_REG |= SP_STATUS_DMA_BUSY;

	DMA_SP_Transfer_Source_Begin_Address = SPDMASourceAddress;
	DMA_SP_Transfer_Target_Begin_Address = SPDMATargetAddress;
	DMA_SP_Transfer_Count = (SP_RD_LEN_REG>>12)&0x000000FF;	// Bit [12-19] is for count
#else


    segment = (uint16)(SP_MEM_ADDR_REG >> 16);

    memcpy(   &RDRAM[SP_DRAM_ADDR_REG & 0x00FFFFFF],
          ((uint32*)((uint8*)sDWORD_R[segment]+(((uint16)SP_MEM_ADDR_REG) ))),
          (SP_WR_LEN_REG)+1);


	/* Should an SP interrupt be put on the stack, like in the DMA_MemCopy_DRAM_To_SP() */
	SP_DMA_BUSY_REG=0;
#endif
}




//SB 03/07/99
/*__________________________________________________________________________
 |                                                                          |
 |                   Serial Interface (SI) DMA Read/Write                   |
 |__________________________________________________________________________|


  _______________________________________________________
 |                                                       |
 | Serial Interface (SI) DMA Read                        |
 |_______________________________________________________|
 |                 |                   |                 |
 |       To        |       From        |      Size       |
 |_________________|___________________|_________________|
 |                 |                   |                 |
 |SI_DRAM_ADDR_REG |  PIF_RAM_START    |    64 bytes     |
 |_________________|___________________|_________________|
*/

void Do_DMA_MemCopy_SI_To_DRAM()
{
    uint32 si_dram_addr_reg = SI_DRAM_ADDR_REG;
	uint32 PIF_RAM_PHYS_addr;
	uint32 RDRAM_addr;

#ifdef DEBUG_SI_DMA 
	if( debug_si_dma )
	{
		sprintf(generalmessage, "SI DMA Read");
		RefreshOpList(generalmessage);
	}
#endif

    //CheckControllers();
    iPifCheck();

	PIF_RAM_PHYS_addr = (uint32)(&PIF[PIF_RAM_PHYS]);		// From
	RDRAM_addr =		(uint32)&RDRAM[0];					// To

	_asm {
		mov edi, RDRAM_addr
		add edi, si_dram_addr_reg
		mov ecx, PIF_RAM_PHYS_addr

		mov eax, dword ptr [ecx]
		mov ebx, dword ptr [ecx + 4]
	//    bswap eax
	//    bswap ebx
		mov  dword ptr [edi],eax
		mov  dword ptr [edi + 4],ebx

		mov eax, dword ptr [ecx + 8]
		mov ebx, dword ptr [ecx + 12]
	//    bswap eax
	//    bswap ebx
		mov  dword ptr [edi + 8],eax
		mov  dword ptr [edi + 12],ebx
		add ecx, 16
		add edi, 16


		mov eax, dword ptr [ecx]
		mov ebx, dword ptr [ecx + 4]
	//    bswap eax
	//    bswap ebx
		mov  dword ptr [edi],eax
		mov  dword ptr [edi + 4],ebx
    
		mov eax, dword ptr [ecx + 8]
		mov ebx, dword ptr [ecx + 12]
	//   bswap eax
	//    bswap ebx
		mov  dword ptr [edi + 8],eax
		mov  dword ptr [edi + 12],ebx
		add ecx, 16
		add edi, 16


		mov eax, dword ptr [ecx]
		mov ebx, dword ptr [ecx + 4]
	//    bswap eax
	//   bswap ebx
		mov  dword ptr [edi],eax
		mov  dword ptr [edi + 4],ebx
    
		mov eax, dword ptr [ecx + 8]
		mov ebx, dword ptr [ecx + 12]
	//    bswap eax
	//    bswap ebx
		mov  dword ptr [edi + 8],eax
		mov  dword ptr [edi + 12],ebx
		add ecx, 16
		add edi, 16


		mov eax, dword ptr [ecx]
		mov ebx, dword ptr [ecx + 4]
	//    bswap eax
	//    bswap ebx
		mov  dword ptr [edi],eax
		mov  dword ptr [edi + 4],ebx
    
		mov eax, dword ptr [ecx + 8]
		mov ebx, dword ptr [ecx + 12]
	//    bswap eax
	//    bswap ebx
		mov  dword ptr [edi + 8],eax
		mov  dword ptr [edi + 12],ebx
	}

    MI_INTR_REG_R  |= MI_INTR_SI;
    gHardwareState.COP0Reg[CAUSE] |= 0x400;
#ifdef CPUCHECKINTR
	CPUNeedToCheckInterrupt = TRUE;
	CPUNeedToDoOtherTask = TRUE;
#endif

	// From apollo 0.01d
	SI_STATUS_REG |= SI_STATUS_INTERRUPT;

}

uint32 saved_si_dram_addr_reg=0;

void DMA_MemCopy_SI_To_DRAM()
{
#ifdef DOSIIODMASEGMENT
#ifdef DEBUG_SI_DMA 
	if( debug_si_dma )
	{
		sprintf(generalmessage, "SI DMA Read Start");
		RefreshOpList(generalmessage);
	}
#endif		//Setup DMA transfer in segments
	DMAInProgress = TRUE;
	SIDMAInProgress = DMA_SI_WRITE;
	CPUNeedToDoOtherTask = TRUE;

	DMA_SI_Transfer_Count = 64;

	saved_si_dram_addr_reg = SI_DRAM_ADDR_REG;

	//Set SI DMA Busy
	SI_STATUS_REG |= SI_STATUS_DMA_BUSY;
#else
	Do_DMA_MemCopy_SI_To_DRAM();
#endif
}

/*_______________________________________________________
 |                                                       |
 | Serial Interface (SI) DMA Write                       |
 |_______________________________________________________|
 |                 |                   |                 |
 |       To        |       From        |      Size       |
 |_________________|___________________|_________________|
 |                 |                   |                 |
 |  PIF_RAM_START  | SI_DRAM_ADDR_REG  |    64 bytes     |
 |_________________|___________________|_________________|
*/
void Do_DMA_MemCopy_DRAM_to_SI()
{
    uint32 si_dram_addr_reg = SI_DRAM_ADDR_REG;
	uint32 PIF_RAM_PHYS_addr;
	uint32 RDRAM_addr;

//  memcpy(&RDRAM[SI_DRAM_ADDR_REG & 0x0FFFFFFF], &PIF[PIF_RAM_PHYS], 64);

#ifdef DEBUG_SI_DMA 
	if( debug_si_dma )
	{
		sprintf(generalmessage, "SI DMA Write");
		RefreshOpList(generalmessage);
	}
#endif

	PIF_RAM_PHYS_addr = (uint32)(&PIF[PIF_RAM_PHYS]);
	RDRAM_addr =		(uint32)&RDRAM[0];
	_asm {
		mov edi, PIF_RAM_PHYS_addr//PifRamPos
		mov ecx, RDRAM_addr
		add ecx, si_dram_addr_reg

		mov eax, dword ptr [ecx]
		mov ebx, dword ptr [ecx + 4]
	//    bswap eax
	//    bswap ebx
		mov  dword ptr [edi],eax
		mov  dword ptr [edi + 4],ebx

		mov eax, dword ptr [ecx + 8]
		mov ebx, dword ptr [ecx + 12]
	//    bswap eax
	//    bswap ebx
		mov  dword ptr [edi + 8],eax
		mov  dword ptr [edi + 12],ebx
		add ecx, 16
		add edi, 16


		mov eax, dword ptr [ecx]
		mov ebx, dword ptr [ecx + 4]
	//    bswap eax
	//    bswap ebx
		mov  dword ptr [edi],eax
		mov  dword ptr [edi + 4],ebx
    
		mov eax, dword ptr [ecx + 8]
		mov ebx, dword ptr [ecx + 12]
	//    bswap eax
	//    bswap ebx
		mov  dword ptr [edi + 8],eax
		mov  dword ptr [edi + 12],ebx
		add ecx, 16
		add edi, 16


		mov eax, dword ptr [ecx]
		mov ebx, dword ptr [ecx + 4]
	//    bswap eax
	//    bswap ebx
		mov  dword ptr [edi],eax
		mov  dword ptr [edi + 4],ebx
    
		mov eax, dword ptr [ecx + 8]
		mov ebx, dword ptr [ecx + 12]
	//    bswap eax
	//   bswap ebx
		mov  dword ptr [edi + 8],eax
		mov  dword ptr [edi + 12],ebx
		add ecx, 16
		add edi, 16


		mov eax, dword ptr [ecx]
		mov ebx, dword ptr [ecx + 4]
	//    bswap eax
	//   bswap ebx
		mov  dword ptr [edi],eax
		mov  dword ptr [edi + 4],ebx
    
		mov eax, dword ptr [ecx + 8]
		mov ebx, dword ptr [ecx + 12]
	//    bswap eax
	//   bswap ebx
		mov  dword ptr [edi + 8],eax
		mov  dword ptr [edi + 12],ebx
	}

    MI_INTR_REG_R  |= MI_INTR_SI;
    gHardwareState.COP0Reg[CAUSE] |= 0x400;
#ifdef CPUCHECKINTR
	CPUNeedToCheckInterrupt = TRUE;
	CPUNeedToDoOtherTask = TRUE;
#endif
	
	// From apollo 0.01d
	SI_STATUS_REG |= SI_STATUS_INTERRUPT;

}

void DMA_MemCopy_DRAM_to_SI()
{
#ifdef DOSIIODMASEGMENT
#ifdef DEBUG_SI_DMA 
	if( debug_si_dma )
	{
		sprintf(generalmessage, "SI DMA Write Start");
		RefreshOpList(generalmessage);
	}
#endif
	//Setup DMA transfer in segments
	DMAInProgress = TRUE;
	SIDMAInProgress = DMA_SI_READ;
	CPUNeedToDoOtherTask = TRUE;

	DMA_SI_Transfer_Count = 64;
	saved_si_dram_addr_reg = SI_DRAM_ADDR_REG;

	//Set SI DMA Busy
	SI_STATUS_REG |= SI_STATUS_DMA_BUSY;

#else
	Do_DMA_MemCopy_DRAM_to_SI();
#endif
}

void DMA_AI()
{
	AUDIO_AiLenChanged();

	//DMAInProgress = DMA_AI_WRITE;
	//CPUNeedTodoOtherTask = TRUE;
	//DMACurrentPosition = 0;
	//DMALength = AI_LEN_REG * 8;
	AI_STATUS_REG |= AI_STATUS_DMA_BUSY;
//	SetDMABusy();

  //MI_INTR_REG_R |= MI_INTR_AI;
  //gHardwareState.COP0Reg[CAUSE] |= 0x400;
//#ifdef CPUCHECKINTR
//	CPUNeedToCheckInterrupt = TRUE;
//	CPUNeedToDoOtherTask = TRUE;
//#endif
    
  //Trigger_AIInterrupt();
}

void Do_SI_IO_Task()
{
}

#ifdef DODMASEGMENT
// This function will be called in main loop of emulator
void DoDMASegment(void)
{
	if( PIDMAInProgress )
	{
		DoPIDMASegment();
	}

	if( SPDMAInProgress )
	{
		DoSPDMASegment();
	}

	if( SIDMAInProgress )
	{
		DoSIDMASegment();
	}

	if( PIDMAInProgress || SPDMAInProgress || SIDMAInProgress )
		return;
	
	DMAInProgress = NO_DMA_IN_PROGRESS;
}

void DoPIDMASegment()
{
	register int i;

	for( i=0; i<PI_DMA_SEGMENT; i++ )
	{
		if( PIDMACurrentPosition >= PIDMALength )	// PI DMA Transfer finished
		{
#ifdef CPUCHECKINTR
			CPUNeedToCheckInterrupt = TRUE;
			CPUNeedToDoOtherTask = TRUE;
#endif
			MI_INTR_REG_R  |= MI_INTR_PI;
			PI_STATUS_REG &= ~PI_STATUS_DMA_BUSY;	// Clear the PI DMA BUSY 
			gHardwareState.COP0Reg[CAUSE] |= 0x400;
			PIDMAInProgress = NO_DMA_IN_PROGRESS;

#ifdef DEBUG_PI_DMA
			if( debug_pi_dma )
			{
				sprintf(generalmessage, "PI DMA Finished");
				RefreshOpList(generalmessage);
			}
#endif

			return;
		}
	
		// No finished
		*(uint8*)((uint8*)sDWORD_R[((uint16)((PIDMATargetAddress) >> 16))] + ((uint16)PIDMATargetAddress++))=
			*(uint8*)((uint8*)sDWORD_R[((uint16)((PIDMASourceAddress) >> 16))] + ((uint16)PIDMASourceAddress++));
		PIDMACurrentPosition++;
	}
}

void DoSPDMASegment()
{
	register int i;

	for( i=0; i<SP_DMA_SEGMENT; i++ )
	{
		if( SPDMACurrentPosition >= SPDMALength )	// PI DMA Transfer finished
		{
			DMA_SP_Transfer_Count--;
			if( DMA_SP_Transfer_Count > 0 )
			{
				// SP DMA Transfer is not finished yet, continue to do the next transfer
				SPDMASourceAddress = DMA_SP_Transfer_Source_Begin_Address;
				SPDMATargetAddress = DMA_SP_Transfer_Target_Begin_Address;
				SPDMACurrentPosition = 0;
				return;
			}
			else
			{
				SP_DMA_BUSY_REG = 0;
				SP_STATUS_REG &= ~SP_STATUS_DMA_BUSY;	// Clear the DMA Busy bit
				MI_INTR_REG_R  |= MI_INTR_SP;
				gHardwareState.COP0Reg[CAUSE] |= 0x400;
#ifdef CPUCHECKINTR
				CPUNeedToCheckInterrupt = TRUE;
				CPUNeedToDoOtherTask = TRUE;
#endif
				SPDMAInProgress = NO_DMA_IN_PROGRESS;

#ifdef DEBUG_SP_DMA
				if( debug_sp_dma )
				{
					sprintf(generalmessage, "SP DMA Finished");
					RefreshOpList(generalmessage);
				}
#endif
				return;
			}
		}
	
		// No finished
		*(uint8*)((uint8*)sDWORD_R[((uint16)((SPDMATargetAddress) >> 16))] + ((uint16)SPDMATargetAddress++))=
			*(uint8*)((uint8*)sDWORD_R[((uint16)((SPDMASourceAddress) >> 16))] + ((uint16)SPDMASourceAddress++));
		SPDMACurrentPosition++;
	}
}
		
void DoSIDMASegment()
{
	DMA_SI_Transfer_Count -= 2;

	if( DMA_SI_Transfer_Count == 0 )
	{
		 SI_DRAM_ADDR_REG = saved_si_dram_addr_reg;
		if( SIDMAInProgress == DMA_SI_READ )
		{
			Do_DMA_MemCopy_DRAM_to_SI();
			si_io_counter = 200;
			SI_STATUS_REG |= SI_STATUS_RD_BUSY;		// Set SI is busy doing IO 
		}
		else	// SIDMAInProgress == DMA_SI_WRITE
		{
			Do_DMA_MemCopy_SI_To_DRAM();
		}
		SI_STATUS_REG &= ~SI_STATUS_DMA_BUSY;	// Clear the SI DMA Busy signal
		SIDMAInProgress = NO_DMA_IN_PROGRESS;	// Next step should set SI IO busy

#ifdef DEBUG_SI_DMA
		if( debug_si_dma )
		{
			sprintf(generalmessage, "SI DMA Finished");
			RefreshOpList(generalmessage);
		}
#endif	

	}
}


void DynDoDMASegment(void)
{
	if( DMAInProgress )
		DoDMASegment();
}


#endif