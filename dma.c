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
#include "1964ini.h"
#include "memory.h"
#include "win32/Dll_Audio.h"
#include "flashram.h"
#include "win32/windebug.h"

#ifdef SAVEOPCOUNTER
#define EXTRA_DMA_TIMING(val)	DMAIncreaseTimer(val);
#else
#define EXTRA_DMA_TIMING(val)
#endif


#define FASTPIDMA

uint32 PIDMASourceAddress=0;
uint32 PIDMATargetAddress=0;
uint32 PIDMACurrentPosition=0;
uint32 PIDMALength=0;
//uint32 PIDMASourceMemory=0;
//uint32 PIDMATargetMemory=0;
uint32 Use_V_Alloc2__source = 0;
uint32 Use_V_Alloc2__target = 0;

BOOL  DMAInProgress=FALSE;
enum DMATYPE PIDMAInProgress=NO_DMA_IN_PROGRESS;
enum DMATYPE SIDMAInProgress=NO_DMA_IN_PROGRESS;
enum DMATYPE SPDMAInProgress=NO_DMA_IN_PROGRESS;

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

void CheckControllers();
void DMAIncreaseTimer(uint32 val);

void InitDMA()
{
	PIDMASourceAddress=0;
	PIDMATargetAddress=0;
	PIDMACurrentPosition=0;
	PIDMALength=0;
	Use_V_Alloc2__source = 0;
	Use_V_Alloc2__target = 0;

	DMAInProgress=FALSE;
	PIDMAInProgress=NO_DMA_IN_PROGRESS;
	SIDMAInProgress=NO_DMA_IN_PROGRESS;
	SPDMAInProgress=NO_DMA_IN_PROGRESS;

	SPDMASourceAddress=0;
	SPDMATargetAddress=0;
	SPDMACurrentPosition=0;
	SPDMALength=0;
	DMA_SP_Transfer_Source_Begin_Address = 0;
	DMA_SP_Transfer_Target_Begin_Address = 0;
	DMA_SP_Transfer_Count = 0;

	SIDMASourceAddress=0;
	SIDMATargetAddress=0;
	DMA_SI_Transfer_Count = 0;
}

/*__________________________________________________________________________
 |                                                                          |
 |               Peripheral Interface (PI) DMA Read/Write                   |
 |__________________________________________________________________________|
*/

// This function is optimized to do faster PI DMA memory copy
// will copy memory from PIDMASourceAddress to PIDMATargetAddress by PIDMALength
// and will use the PIDMASourceMemory and PIDMATargetMemory
void FastPIMemoryCopy(void)
{
	register int i;
	unsigned register __int32 target;// = PIDMATargetMemory+PIDMATargetAddress;
	unsigned register __int32 source;// = PIDMASourceMemory+PIDMASourceAddress;

    if (Use_V_Alloc2__target)
    {
        target = (uint32)pLOAD_UWORD_PARAM_2(PIDMATargetAddress);
    }
    else
    {
        target = (uint32)pLOAD_UWORD_PARAM(PIDMATargetAddress);

    }

    if (Use_V_Alloc2__source)
    {
        source = (uint32)pLOAD_UWORD_PARAM_2(PIDMASourceAddress);
    }
    else
    {
        source = (uint32)pLOAD_UWORD_PARAM(PIDMASourceAddress);
    }

// solution #1
	//for( i = -(__int32)PIDMALength; i<0; i++ )
	//	*(uint8*)(target++^0x3) = *(uint8*)(source++^0x3);

// solution #2
	__try{
	if( (target&3) == 0 && (source&3) == 0 && (PIDMALength&3) == 0 )	// DWORD align
	{
		for( i = -(((__int32)PIDMALength)>>2); i<0; i++ )
		{
			*(uint32*)target = *(uint32*)source;
			target+=4;
			source+=4;
		}
	}
	else if( (target&1)==0 && (source&1)== 0 && (PIDMALength&1) == 0 )	// WORD align
	{
		for( i = -(((__int32)PIDMALength)>>1); i<0; i++ )
		{
			*(uint16*)(target^2) = *(uint16*)(source^2);
			target+=2;
			source+=2;
		}
	}
	else	// not align
	{
		for( i = -(__int32)PIDMALength; i<0; i++ )
			*(uint8*)(target++^0x3) = *(uint8*)(source++^0x3);
	}
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE3("Bad PI DMA: Source=%08X, Target=%08X. Len=%08X", PIDMASourceAddress, PIDMATargetAddress, PIDMALength)
	}
}
/*_______________________________________________________
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
void DMA_PI_MemCopy_From_DRAM_To_Cart()
{
	PIDMACount++;
	PIDMASourceAddress = (PI_DRAM_ADDR_REG & 0x00FFFFFF)|0x80000000;
	PIDMATargetAddress = (PI_CART_ADDR_REG & 0x1FFFFFFF)|0x80000000;
	PIDMACurrentPosition = 0;;
	PIDMALength = (PI_RD_LEN_REG & 0x00FFFFFF) + 1;
//	PIDMASourceMemory = valloc;
//	PIDMATargetMemory  = valloc;
    Use_V_Alloc2__target = 0;
    Use_V_Alloc2__source = 0;

#ifdef DEBUG_PI_DMA 
	if( debug_pi_dma )
	{
		TRACE4( "%08X: PI Copy RDRAM to CART  %d bytes %08X to %08X", gHWS_pc, PIDMALength, PI_DRAM_ADDR_REG, PI_CART_ADDR_REG);

		if( PI_DRAM_ADDR_REG & 0x7 )
		{
			DisplayError("Warning, PI DMA, address does not align as requirement. RDRAM ADDR = %08X", PI_DRAM_ADDR_REG);
		}
		if( (PIDMALength & 0x1) || (PI_CART_ADDR_REG & 0x1) )
		{
			DisplayError("Warning, PI DMA, need half word swap. RDRAM ADDR = %08X, CART ADDR=%08X, Len=%X", PI_DRAM_ADDR_REG, PI_CART_ADDR_REG, PIDMALength);
		}
	}
#endif


#ifdef DEBUG_SRAM
	if( debug_sram )
	{
		TRACE3( "SRAM/FLASHRAM or CART Write, % bytes bytes %08X to %08X", PIDMALength, PI_DRAM_ADDR_REG, PI_CART_ADDR_REG);
	}
#endif


	if( (PI_CART_ADDR_REG & 0x1F000000) == MEMORY_START_C2A2 )	// Flashram PI DMA read
	{
		// Tell flashram about this DMA event
		DMA_RDRAM_To_Flashram(PI_DRAM_ADDR_REG, PI_CART_ADDR_REG, PIDMALength);
        Use_V_Alloc2__target = 1;
	}

#ifndef ENABLE_OPCODE_DEBUGGER
	if( currentromoptions.DMA_Segmentation == USEDMASEG_YES )
	{
		//Setup DMA transfer in segments
		PIDMAInProgress = DMA_PI_READ;
		DMAInProgress = TRUE;
		CPUNeedToDoOtherTask = TRUE;


		//PI_STATUS_REG |= PI_STATUS_DMA_BUSY;	// Set PI status register DMA busy
		PI_STATUS_REG |= PI_STATUS_DMA_IO_BUSY;	// Set PI status register DMA busy
	}
	else
#endif
	{
#ifdef FASTPIDMA
		FastPIMemoryCopy();
#else
		register __int32 i;
		for( i = -(__int32)PIDMALength; i<0; i++ )
			*(uint8*)(PIDMATargetMemory+(PIDMATargetAddress++^0x3)) = *(uint8*)(PIDMASourceMemory+(PIDMASourceAddress++^0x3));
#endif
		EXTRA_DMA_TIMING(PIDMALength);
		Trigger_PIInterrupt();
	}
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
void DMA_PI_MemCopy_From_Cart_To_DRAM() 
{
	uint32 len = (PI_WR_LEN_REG & 0x00FFFFFF) +1;
	uint32 pi_dram_addr_reg = (PI_DRAM_ADDR_REG & 0x00FFFFFF) | 0x80000000;
	uint32 pi_cart_addr_reg = (PI_CART_ADDR_REG & 0x1FFFFFFF) | 0x80000000;

	PIDMACount++;

#ifdef DEBUG_PI_DMA 
	if( debug_pi_dma )
	{
		TRACE4( "%08X: PI Copy CART to RDRAM %db from %08X to %08X", gHWS_pc, len, PI_CART_ADDR_REG, PI_DRAM_ADDR_REG);

		if( pi_dram_addr_reg & 0x7 )
		{
			TRACE1( "Warning, PI DMA, address does not align as requirement. RDRAM ADDR = %08X", pi_dram_addr_reg);
		}

		if( len & 0x1 )
		{
			TRACE0( "Warning, PI DMA, odd length");
			//len++;
			//len--;
		}

		if( pi_cart_addr_reg & 0x1 )
		{
			TRACE0( "Warning, PI DMA, odd CARD address");
		}
	}

	if( debug_sram)
	{
		if( (pi_cart_addr_reg & 0x1F000000) == MEMORY_START_C2A1 )
		{
			TRACE0( "Copy C2A1 (0x05000000) to RDRAM")
		}
		else if( (pi_cart_addr_reg & 0x1F000000) == MEMORY_START_C1A1 )
		{
			TRACE0( "Copy C1A1 (0x06000000) to RDRAM")
		}
		else if( (pi_cart_addr_reg & 0x1F000000) == MEMORY_START_C2A2 )
		{
			TRACE3( "DMA Flashram to RDRAM %d byte from %08X to %08X", len, PI_CART_ADDR_REG, PI_DRAM_ADDR_REG)
		}
		else if( (pi_cart_addr_reg & 0x1FF00000) == MEMORY_START_C1A3 )
		{
			TRACE0( "Copy C1A3 (0x1FD00000) to RDRAM")
		}

	}
#endif

	PIDMASourceAddress = (PI_CART_ADDR_REG & 0x1FFFFFFF) | 0x80000000;
	PIDMATargetAddress = (PI_DRAM_ADDR_REG & 0x00FFFFFF) | 0x80000000;
    Use_V_Alloc2__source = 0;
    Use_V_Alloc2__target = 0;

	if( ((PIDMATargetAddress&0x1FFFFFFF)+len) >= current_rdram_size )
	{
		DisplayError("Bad PI DMA address, PI DMA skipped");
		Trigger_PIInterrupt();
		return;
	}

	PIDMACurrentPosition = 0;;
	PIDMALength = len;


	if( whichcore == DYNACOMPILER && 
		(currentromoptions.Code_Check == CODE_CHECK_MEMORY_QWORD_AND_DMA || currentromoptions.Code_Check == CODE_CHECK_MEMORY_BLOCK_AND_DMA ||
		 currentromoptions.Code_Check == CODE_CHECK_DMA_ONLY) )
	{
		// Clear Dynacomplied code
#ifndef DYNAEXECUTIONMAPMEMORY
		__try
		{
			memset(&DynaRDRAM[PI_DRAM_ADDR_REG & 0x00FFFFFF],0,len);
		}
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
			DEBUG_PI_DMA_TRACE0( "Bad PI DMA address, PI DMA skipped");

			Trigger_PIInterrupt();
			DisplayError("Bad PI DMA task");
			return;
		}
#else
		{
			register uint32 addr = PIDMATargetAddress;
			register int i = -(__int32)PIDMALength;;

			// Align to DWORD boundary
			i += (addr%4);
			addr += (addr%4);

			for( ; i<0; i+=4, addr+=4 )
			{
L1:
				__try
				{
				//Need to clear the Dyna marks in dynarommap, how to do it?
					*(uint32*)((uint8*)sDYN_PC_LOOKUP[((uint16)(addr >> 16))] + (uint16)addr)= 0;
				}
				__except(NULL,EXCEPTION_EXECUTE_HANDLER)
				{
					//exception only happens at 0x10000 boundary or at the beginning
					if( addr%0x10000 == 0 )
					{
						if( i+0x10000<0)
						{
							i+= 0x10000;
							addr += 0x10000;
							goto L1;
						}
						else
							break;
					}
					else if( i+addr%0x10000 < 0 )
					{
						i+= addr%0x10000;
						addr += (addr/0x10000+1)*0x10000;
						goto L1;
					}
					else
						break;
				}
			}
		}
#endif
	}

	if( (pi_cart_addr_reg & 0x1F000000) == MEMORY_START_C2A2 )	// Flashram PI DMA read
	{
		// Tell flashram about this DMA event
		DMA_Flashram_To_RDRAM(PI_DRAM_ADDR_REG, PI_CART_ADDR_REG, PIDMALength);
        Use_V_Alloc2__source = 1;
	}

	/*
	else if( ((PIDMASourceAddress&0x0FFFFFFF)+len) > gAllocationLength )
	{
		len = gAllocationLength - (PIDMASourceAddress&0x0FFFFFFF);
		PIDMALength = len;
		TRACE1("Warning, DMA length is too long, trimmed, len=%d", len);
	}
	*/


#ifndef ENABLE_OPCODE_DEBUGGER
	if( currentromoptions.DMA_Segmentation == USEDMASEG_YES )
	{
		//Setup DMA transfer in segments
		PIDMAInProgress = DMA_PI_WRITE;
		DMAInProgress = TRUE;
		CPUNeedToDoOtherTask = TRUE;

		//PI_STATUS_REG |= PI_STATUS_DMA_BUSY;
		PI_STATUS_REG |= PI_STATUS_DMA_IO_BUSY;
	}
	else
#endif
	{
#ifdef FASTPIDMA
		FastPIMemoryCopy();
#else
		register __int32 i;
		for( i= -(__int32)PIDMALength; i<0; i++ )
			*(uint8*)(PIDMATargetMemory+(PIDMATargetAddress++^0x3)) = *(uint8*)(PIDMASourceMemory+(PIDMASourceAddress++^0x3));
#endif
		EXTRA_DMA_TIMING(PIDMALength);
		Trigger_PIInterrupt();
	}
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
	uint32 sp_mem_addr_reg = SP_MEM_ADDR_REG;

#ifdef DEBUG_SP_DMA 
	if( debug_sp_dma )
	{
		TRACE3( "SP DMA Read  %d bytes from %08X to %08X", SP_RD_LEN_REG+1, SP_DRAM_ADDR_REG, SP_MEM_ADDR_REG )
	}

	// Check Half Word Alignment
	if( SP_DRAM_ADDR_REG & 0x7 )
	{
		//DisplayError("Warning, SP DMA, address does not align as requirement. RDRAM ADDR = %08X", SPDMASourceAddress);
		TRACE2("Warning, SP DMA, address does not align as requirement, RDRAM ADDR = %08X, SP_MEM_ADDR_REG=%08X", SP_DRAM_ADDR_REG, SP_MEM_ADDR_REG);
	}

	if( SP_MEM_ADDR_REG & 0x3 )
	{
		//DisplayError("Warning, SP DMA, need half word swap. RDRAM ADDR = %08X, SP_MEM_ADDR_REG=%08X", SPDMASourceAddress, SPDMATargetAddress);
		TRACE2("Warning, SP DMA, address does not align as requirement, RDRAM ADDR = %08X, SP_MEM_ADDR_REG=%08X", SP_DRAM_ADDR_REG, SP_MEM_ADDR_REG);
	}
#endif


	SPDMALength = (SP_RD_LEN_REG & 0x00000FFF) + 1;	// SP_RD_LEN_REG bit [0-11] is length to transfer


	/*
	if( currentromoptions.DMA_Segmentation == USEDMASEG_YES )
	{
		//Setup DMA transfer in segments
		SPDMAInProgress = DMA_SP_READ;
		DMAInProgress = TRUE;
		CPUNeedToDoOtherTask = TRUE;

		SPDMASourceAddress = (SP_DRAM_ADDR_REG & 0x00FFFFFF) | 0x80000000;
		SPDMATargetAddress = SP_DMEM_START + (SP_MEM_ADDR_REG & 0x00001FFF) + 0x80000000;
		//SPDMATargetAddress = sp_mem_addr_reg;
		SPDMACurrentPosition = 0;;

		SP_DMA_BUSY_REG = 1;
		SP_STATUS_REG |= SP_STATUS_DMA_BUSY;

		DMA_SP_Transfer_Source_Begin_Address = SPDMASourceAddress;
		DMA_SP_Transfer_Target_Begin_Address = SPDMATargetAddress;
		DMA_SP_Transfer_Count = (SP_RD_LEN_REG>>12)&0x000000FF;	// Bit [12-19] is for count
	}
	else
	*/
	{
        __try{
            memcpy(&gMS_SP_MEM[(sp_mem_addr_reg&0x1FFF)>>2],	&gMS_RDRAM[SP_DRAM_ADDR_REG & 0x00FFFFFF],	SPDMALength);
        }
        __except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("Bad SP DMA copy");
        }

		EXTRA_DMA_TIMING(SPDMALength);
    
		SP_DMA_BUSY_REG=0;
		SP_STATUS_REG &= ~SP_STATUS_DMA_BUSY;	// Clear the DMA Busy bit

		// Rice. 2001-08018
		SP_STATUS_REG |= SP_STATUS_HALT;

//		Trigger_SPInterrupt();
		SPDMAInProgress = NO_DMA_IN_PROGRESS;

#ifdef DEBUG_SP_DMA
		if( debug_sp_dma )
		{
			TRACE0( "SP DMA Finished");
		}
#endif
	}
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
	uint16 segment;

#ifdef DEBUG_SP_DMA 
	if( debug_sp_dma )
	{
		TRACE3( "SP DMA Write %d bytes from %08X to %08X", SP_WR_LEN_REG+1, SP_MEM_ADDR_REG, SP_DRAM_ADDR_REG);
	}

	// Check Half Word Alignment
	if( SP_DRAM_ADDR_REG & 0x7 )
	{
		//DisplayError("Warning, SP DMA, address does not align as requirement. RDRAM ADDR = %08X", SP_DRAM_ADDR_REG);
		TRACE2("Warning, SP DMA, address does not align as requirementRDRAM ADDR = %08X, SP_MEM_ADDR_REG=%08X", SP_DRAM_ADDR_REG, SP_MEM_ADDR_REG);
	}

	if( SP_MEM_ADDR_REG & 0x3 )
	{
		//DisplayError("Warning, SP DMA, need half word swap. RDRAM ADDR = %08X, SP_MEM_ADDR_REG=%08X", SP_DRAM_ADDR_REG, SP_MEM_ADDR_REG);
		TRACE2("Warning, SP DMA, address does not align as requirementRDRAM ADDR = %08X, SP_MEM_ADDR_REG=%08X", SP_DRAM_ADDR_REG, SP_MEM_ADDR_REG);
	}
#endif

	SPDMALength = (SP_WR_LEN_REG & 0x00000FFF) + 1;	// SP_RD_LEN_REG bit [0-11] is length to transfer


	/*
	if( currentromoptions.DMA_Segmentation == USEDMASEG_YES )
	{
		//Setup DMA transfer in segments
		DMAInProgress = TRUE;
		SPDMAInProgress = DMA_SP_WRITE;
		CPUNeedToDoOtherTask = TRUE;

		//SPDMASourceAddress = SP_DMEM_START + (SP_MEM_ADDR_REG & 0x00001FFF);
		SPDMASourceAddress = SP_MEM_ADDR_REG;
		SPDMATargetAddress = SP_DRAM_ADDR_REG & 0x00FFFFFF | 0x80000000;
		SPDMACurrentPosition = 0;;


		SP_DMA_BUSY_REG = 1;
		SP_STATUS_REG |= SP_STATUS_DMA_BUSY;

		DMA_SP_Transfer_Source_Begin_Address = SPDMASourceAddress;
		DMA_SP_Transfer_Target_Begin_Address = SPDMATargetAddress;
		DMA_SP_Transfer_Count = (SP_RD_LEN_REG>>12)&0x000000FF;	// Bit [12-19] is for count
	}
	else
	*/
	{
		segment = (uint16)(SP_MEM_ADDR_REG >> 16);

        memcpy(   &gMS_RDRAM[SP_DRAM_ADDR_REG & 0x00FFFFFF], &gMS_SP_MEM[(SP_MEM_ADDR_REG&0x1FFF)>>2], (SP_WR_LEN_REG)+1);

		EXTRA_DMA_TIMING(SPDMALength);

		SP_DMA_BUSY_REG=0;
		SP_STATUS_REG &= ~SP_STATUS_DMA_BUSY;	// Clear the DMA Busy bit

		// Rice. 2001-08018
		SP_STATUS_REG |= SP_STATUS_HALT;

//		Trigger_SPInterrupt();
		SPDMAInProgress = NO_DMA_IN_PROGRESS;

#ifdef DEBUG_SP_DMA
		if( debug_sp_dma )
		{
			TRACE0( "SP DMA Finished");
		}
#endif

	}
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

	// Check Half Word Alignment
	if( SI_DRAM_ADDR_REG & 0x7 )
	{
		TRACE1("Warning, SI DMA is skipped, address does not align. RDRAM ADDR = %08X", SI_DRAM_ADDR_REG);

		//Skip this DMA
		SI_STATUS_REG |= SI_STATUS_INTERRUPT;
		Trigger_SIInterrupt();

		return;
	}

	DEBUG_SI_DMA_TRACE0( "SI DMA Read");

    //CheckControllers();
    iPifCheck();

	PIF_RAM_PHYS_addr = (uint32)(&gMS_PIF[PIF_RAM_PHYS]);		// From
	RDRAM_addr =		(uint32)&gMS_RDRAM[0];					// To

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

	EXTRA_DMA_TIMING(64);
	SI_STATUS_REG |= SI_STATUS_INTERRUPT;
	Trigger_SIInterrupt();
}

uint32 saved_si_dram_addr_reg=0;

void DMA_MemCopy_SI_To_DRAM()
{
#ifndef ENABLE_OPCODE_DEBUGGER
	if( currentromoptions.DMA_Segmentation == USEDMASEG_YES )
	{
		DEBUG_SI_DMA_TRACE0( "SI DMA Read Start");

		DMAInProgress = TRUE;
		SIDMAInProgress = DMA_SI_WRITE;
		CPUNeedToDoOtherTask = TRUE;

		DMA_SI_Transfer_Count = 64;

		saved_si_dram_addr_reg = SI_DRAM_ADDR_REG;

		//Set SI DMA Busy
		SI_STATUS_REG |= SI_STATUS_DMA_BUSY;
	}
	else
#endif
	{
		Do_DMA_MemCopy_SI_To_DRAM();
	}
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

	// Check Half Word Alignment
	if( SI_DRAM_ADDR_REG & 0x7 )
	{
		TRACE1("Warning, SI DMA is skipped, address does not align. RDRAM ADDR = %08X", SI_DRAM_ADDR_REG);
		
		//Skip this DMA
		SI_STATUS_REG |= SI_STATUS_INTERRUPT;
		Trigger_SIInterrupt();
		return;
	}

	DEBUG_SI_DMA_TRACE0( "SI DMA Write");

	PIF_RAM_PHYS_addr = (uint32)(&gMS_PIF[PIF_RAM_PHYS]);
	RDRAM_addr =		(uint32)&gMS_RDRAM[0];
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

	EXTRA_DMA_TIMING(64);

	SI_STATUS_REG |= SI_STATUS_INTERRUPT;
	Trigger_SIInterrupt();
}

void DMA_MemCopy_DRAM_to_SI()
{
#ifndef ENABLE_OPCODE_DEBUGGER
	if( currentromoptions.DMA_Segmentation == USEDMASEG_YES )
	{
		DEBUG_SI_DMA_TRACE0( "SI DMA Write Start");

		//Setup DMA transfer in segments
		DMAInProgress = TRUE;
		SIDMAInProgress = DMA_SI_READ;
		CPUNeedToDoOtherTask = TRUE;

		DMA_SI_Transfer_Count = 64;
		saved_si_dram_addr_reg = SI_DRAM_ADDR_REG;

		//Set SI DMA Busy
		SI_STATUS_REG |= SI_STATUS_DMA_BUSY;
	}
	else
#endif
	{
		Do_DMA_MemCopy_DRAM_to_SI();
	}
}

void DMA_AI()
{
	//AUDIO_AiLenChanged();

	//DMAInProgress = DMA_AI_WRITE;
	//CPUNeedTodoOtherTask = TRUE;
	//DMACurrentPosition = 0;
	//DMALength = AI_LEN_REG * 8;
	AI_STATUS_REG |= AI_STATUS_DMA_BUSY;
//	SetDMABusy();

  //MI_INTR_REG_R |= MI_INTR_AI;
  //SET_EXCEPTION(EXC_INT)
  //gHWS_COP0Reg[CAUSE] |= CAUSE_IP3;
//	CPUNeedToCheckInterrupt = TRUE;
//	CPUNeedToDoOtherTask = TRUE;
    
  //Trigger_AIInterrupt();
}

void Do_SI_IO_Task()
{
}

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
#ifdef FASTPIDMA
	if( PIDMACurrentPosition+PI_DMA_SEGMENT >= PIDMALength)
	{
		FastPIMemoryCopy();

		PI_STATUS_REG &= ~PI_STATUS_DMA_IO_BUSY;	// Clear the PI DMA BUSY 
		Trigger_PIInterrupt();
		PIDMAInProgress = NO_DMA_IN_PROGRESS;

		DEBUG_PI_DMA_TRACE0("PI DMA Finished");

		EXTRA_DMA_TIMING(PIDMALength-PIDMACurrentPosition)
	}
	else
	{
		PIDMACurrentPosition+=PI_DMA_SEGMENT;
		EXTRA_DMA_TIMING(PI_DMA_SEGMENT);
	}
#else
	register int i;

	for( i=-PI_DMA_SEGMENT; i<0; i++ )
	{
		if( PIDMACurrentPosition >= PIDMALength )	// PI DMA Transfer finished
		{
			PI_STATUS_REG &= ~PI_STATUS_DMA_IO_BUSY;	// Clear the PI DMA BUSY 
			Trigger_PIInterrupt();
			PIDMAInProgress = NO_DMA_IN_PROGRESS;

			DEBUG_PI_DMA_TRACE0("PI DMA Finished");

			EXTRA_DMA_TIMING(PI_DMA_SEGMENT+i);
			return;
		}
	
		__try{
        *(uint8*)(PIDMATargetMemory+(PIDMATargetAddress++^0x3)) = *(uint8*)(PIDMASourceMemory+(PIDMASourceAddress++^0x3)); 
		PIDMACurrentPosition++;
		}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			PI_STATUS_REG &= ~PI_STATUS_DMA_IO_BUSY;	// Clear the PI DMA BUSY 
			Trigger_PIInterrupt();
			PIDMAInProgress = NO_DMA_IN_PROGRESS;

			DEBUG_PI_DMA_TRACE0("Bad PI DMA Finished");
		}

	}
	EXTRA_DMA_TIMING(PI_DMA_SEGMENT);
#endif
}

void DoSPDMASegment()
{
	register int i;
    uint8* pSPDMATargetAddress;
    uint8* pSPDMASourceAddress;

	for( i=0; i<SP_DMA_SEGMENT; i++ )
	{
		if( SPDMACurrentPosition >= SPDMALength )	// PI DMA Transfer finished
		{
			//DMA_SP_Transfer_Count--;
			//if( DMA_SP_Transfer_Count > 0 )
			//{
				// SP DMA Transfer is not finished yet, continue to do the next transfer
			//	SPDMASourceAddress = DMA_SP_Transfer_Source_Begin_Address;
			//	SPDMATargetAddress = DMA_SP_Transfer_Target_Begin_Address;
			//	SPDMACurrentPosition = 0;
			//	return;
			//}
			//else
			//{
				SP_DMA_BUSY_REG = 0;
				SP_STATUS_REG &= ~SP_STATUS_DMA_BUSY;	// Clear the DMA Busy bit

				// Rice. 2001-08018
				SP_STATUS_REG |= SP_STATUS_HALT;
				//Trigger_SPInterrupt();
				SPDMAInProgress = NO_DMA_IN_PROGRESS;

#ifdef DEBUG_SP_DMA
				if( debug_sp_dma )
				{
					TRACE0( "SP DMA Finished");
				}
#endif
				EXTRA_DMA_TIMING(PI_DMA_SEGMENT-i);
				return;
			//}
		}
	
		// No finished
        pSPDMATargetAddress = pLOAD_UBYTE_PARAM(SPDMATargetAddress++);
        pSPDMASourceAddress = pLOAD_UBYTE_PARAM(SPDMASourceAddress++);

        *pSPDMATargetAddress = *pSPDMASourceAddress;
        SPDMACurrentPosition++;
	}
	EXTRA_DMA_TIMING(PI_DMA_SEGMENT);
}
		
void DoSIDMASegment()
{
	DMA_SI_Transfer_Count -= SI_DMA_SEGMENT;	

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

		DEBUG_SI_DMA_TRACE0( "SI DMA Finished");
	}
}


void DynDoDMASegment(void)
{
	if( DMAInProgress )
		DoDMASegment();
}

// Adjust Timing for VIcounter and COUNT register
void DMAIncreaseTimer(uint32 val)
{
#ifdef FAST_COUNTER
	//Count_Down(val*VICounterFactors[CounterFactor]);
	Count_Down(val*VICounterFactors[CounterFactor]/2);	//assume each pclock will transfer 4 bytes
#else
	//gHWS_COP0Reg[COUNT] += val*CounterFactors[CounterFactor]/VICounterFactors[CounterFactor];
	//VIcounter += val*VICounterFactors[CounterFactor];
	gHWS_COP0Reg[COUNT] += val*CounterFactors[CounterFactor]/VICounterFactors[CounterFactor]/4;
	VIcounter += val*VICounterFactors[CounterFactor]/2;
#endif
}