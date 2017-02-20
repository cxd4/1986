#include "globals.h"
#include "n64rcp.h"

uint32* DPC=NULL;
uint32* DPS=NULL;
uint32* MI=NULL;
uint32* VI=NULL;
uint32* AI=NULL;
uint32* PI=NULL;
uint32* RI=NULL;
uint32* SI=NULL;
uint32* RDREG=NULL;
uint32* SP_REG=NULL;
uint32* C2A1=NULL;
uint32* C1A1=NULL;
uint32* C1A3=NULL;
uint32* C2A2=NULL;
uint32* GIO_REG=NULL;
uint8* RDRAM;		// Size = 8MB

uint8*	ramRegs0= NULL;
uint8*	ramRegs4= NULL;
uint8*	ramRegs8= NULL;


//uint8 DynaRDRAM[0x00400000];	// Size = 4MB
//uint8 RDRAM[0x00400000];		// Size = 4MB
uint8 DynaRDRAM[0x00800000];	// Size = 8MB
uint32 DynaSP_REG[131074];
uint8* DynaROM = NULL;


void SP_Reset(void)
{
	//Clear all STATUS Registers
	SP_STATUS_REG = 0;
}

void SI_Reset(void)
{
	SI_STATUS_REG = 0;
}

void PI_Reset(void)
{
	PI_STATUS_REG = 0;
}

void RCP_Reset(void)
{
	SP_Reset();
	SI_Reset();
	PI_Reset();
	//VI_Reset();
	//AI_Reset();
	MI_INTR_REG_R = 0;
}

