/*______________________________________________________________________________
 |                                                                              |
 |  1964 - dma.h                                                                |
 |  Copyright (C) 2001 Joel Middendorf, <schibo@emulation64.com>                |
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
 |  email      : dyangchicago@yahoo.com, schibo@emulation64.com                 |
 |  paper mail :                                                                |
 |______________________________________________________________________________|
*/

#ifndef __DMA_H
#define __DMA_H

#include "windows.h"
#include "globals.h"
//#include "options.h"

enum DMATYPE { 
	NO_DMA_IN_PROGRESS, DMA_PI_READ, DMA_PI_WRITE, 
	DMA_SP_READ, DMA_SP_WRITE, DMA_SI_READ, DMA_SI_WRITE,
	DMA_AI_WRITE };

extern BOOL DMAInProgress;
extern enum DMATYPE PIDMAInProgress;
extern enum DMATYPE SIDMAInProgress;
extern enum DMATYPE SPDMAInProgress;

extern uint32 DMASourceAddress;
extern uint32 DMATargetAddress;
extern uint32 DMACurrentPosition;
extern uint32 DMALength;

extern uint32 DMA_SP_Transfer_Source_Begin_Address;
extern uint32 DMA_SP_Transfer_Target_Begin_Address;
extern int DMA_SP_Transfer_Count;
extern int DMA_SI_Transfer_Count;

void DoDMASegment(void);
void DynDoDMASegment(void);
void DoPIDMASegment(void);
void DoSPDMASegment(void);
void DoSIDMASegment(void);
void DMA_AI();

/* DMA external functions */
extern void DMA_PI_MemCopy_From_Cart_To_DRAM();
extern void DMA_PI_MemCopy_From_DRAM_To_Cart();
extern void DMA_MemCopy_SP_to_DRAM();
extern void DMA_MemCopy_DRAM_To_SP();
extern void DMA_MemCopy_DRAM_to_SI();
extern void DMA_MemCopy_SI_To_DRAM();

void InitDMA(void);

#endif	// __DMA_H