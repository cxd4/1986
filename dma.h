#ifndef __DMA_H
#define __DMA_H

#include "windows.h"
#include "globals.h"
//#include "options.h"

//#ifdef DODMASEGMENT

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

//#endif

/* DMA external functions */
extern void DMA_MemCopy_DRAM_To_PI();
extern void DMA_MemCopy_PI_To_DRAM();
extern void DMA_MemCopy_SP_to_DRAM();
extern void DMA_MemCopy_DRAM_To_SP();
extern void DMA_MemCopy_DRAM_to_SI();
extern void DMA_MemCopy_SI_To_DRAM();

#endif	// __DMA_H