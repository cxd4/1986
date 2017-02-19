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

#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <memory.h>
#include "interrupt.h"
#include "globals.h"

#ifdef DEBUG_COMMON
extern void RefreshOpList(char *opcode);
#endif
extern uint32 CalculateAddress(uint32 Addr);
extern void dlist_check();

extern void (APIENTRY* DLL_3D_About)(HWND);

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


void DMA_Read_PI() {
#ifdef DEBUG_COMMON	
	char TempStr[80];
	sprintf(TempStr, "%08X: RD_PI  %d bytes %08X to %08X", pc, PI_RD_LEN_REG+1, PI_DRAM_ADDR_REG, PI_CART_ADDR_REG);
	RefreshOpList(TempStr);
#endif
	
	memcpy(		&ROM_Image[PI_CART_ADDR_REG & 0x0FFFFFFF],
				   &RDRAM[PI_DRAM_ADDR_REG & 0x0FFFFFFF], 
		   PI_RD_LEN_REG + 1);

	//SB 6/6/99
	/* put a PI interrupt on the stack */
	MI_INTR_REG_R |= MI_INTR_PI;
	COP0Reg[CAUSE] |= 0x400;
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
void DMA_Write_PI() {
//uint32 length;
uint16 cart_segment;
//uint32 i = 0;
#ifdef DEBUG_COMMON	
	char TempStr[80];
	sprintf(TempStr, "%08X: WR_PI %d bytes from %08X to %08X", pc, PI_WR_LEN_REG+1, PI_CART_ADDR_REG, PI_DRAM_ADDR_REG);
	RefreshOpList(TempStr);
#endif


	cart_segment = (uint16)(PI_CART_ADDR_REG >> 16);

	memcpy(   &RDRAM[PI_DRAM_ADDR_REG & 0x00FFFFFF],
	      ((uint32*)((uint8*)sDWORD_R[cart_segment]+(((uint16)PI_CART_ADDR_REG) ))),
		PI_WR_LEN_REG+1);

	//SB 6/6/99
	/* put a PI interrupt on the stack */
	
	MI_INTR_REG_R  |= MI_INTR_PI;
	COP0Reg[CAUSE] |= 0x400;
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
void DMA_Read_SP() {
uint16 segment;

#ifdef DEBUG_COMMON	
	char TempStr[80];
//	sprintf(TempStr, "SP DMA Read  %d bytes from %08X to %08X", SP_RD_LEN_REG+1, SP_DRAM_ADDR_REG, SP_MEM_ADDR_REG );
//	RefreshOpList(TempStr);
#endif
	
	segment = (uint16)(SP_MEM_ADDR_REG >> 16);

	if (((segment & 0xC000)) == 0xC000); /* Ignore TLB until we handle it. */
	else {
	
		memcpy(((uint32*)((uint8*)sDWORD_R[segment]+(((uint16)SP_MEM_ADDR_REG) ))),
				&RDRAM[SP_DRAM_ADDR_REG & 0x00FFFFFF],
			SP_RD_LEN_REG+1);

	//SB 6/6/99
	/* put an SP interrupt on the stack */
	
	}

	SP_DMA_BUSY_REG = 0;
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
void DMA_Write_SP() {
uint16 segment;

#ifdef DEBUG_COMMON	
	char TempStr[80];
//	sprintf(TempStr, "SP DMA Write %d bytes from %08X to %08X", SP_WR_LEN_REG+1, SP_MEM_ADDR_REG, SP_DRAM_ADDR_REG);
//	RefreshOpList(TempStr);
#endif

	segment = (uint16)(SP_MEM_ADDR_REG >> 16);

	if (((segment & 0xC000)) == 0xC000); /* Ignore TLB until we handle it. */
	else {
	
		memcpy(   &RDRAM[SP_DRAM_ADDR_REG & 0x00FFFFFF],
		      ((uint32*)((uint8*)sDWORD_R[segment]+(((uint16)SP_MEM_ADDR_REG) ))),
	
			(SP_WR_LEN_REG)+1);

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
void DMA_Read_SI() {
#ifdef DEBUG_COMMON	
	char TempStr[80];
	sprintf(TempStr, "SI DMA Read");
	RefreshOpList(TempStr);
#endif
	CheckControllers();
	memcpy(&RDRAM[SI_DRAM_ADDR_REG & 0x0FFFFFFF], &PIF[PIF_RAM_PHYS], 64);

	MI_INTR_REG_R  |= MI_INTR_SI;
	COP0Reg[CAUSE] |= 0x400;

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
void DMA_Write_SI() {
#ifdef DEBUG_COMMON	
	char TempStr[80];
	sprintf(TempStr, "SI DMA Write");
	RefreshOpList(TempStr);
#endif

	memcpy(&PIF[PIF_RAM_PHYS], &RDRAM[SI_DRAM_ADDR_REG & 0x0FFFFFFF], 64);
	MI_INTR_REG_R  |= MI_INTR_SI;
	COP0Reg[CAUSE] |= 0x400;
}

void DMA_AI();
void DMA_AI() {
	
//	MI_INTR_REG_R |= MI_INTR_AI;
//	COP0Reg[CAUSE] |= 0x400;
	
//	Trigger_AIInterrupt();
}