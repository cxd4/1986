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


This project is a documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar. Many source
code comments are taken from anarko's n64toolkit with consent and are 
the property of anarko.
*/

#include <windows.h>
#include "options.h"
#include "debug_option.h"
#include "globals.h"
#include "r4300i.h"
#include "hardware.h"
#include "n64rcp.h"
#include "memory.h"
#include "iPIF.h"

#include "dma.h"
#include "emulator.h"
#include "timer.h"
#include "interrupt.h"
#include "win32/DLL_Video.h"
#include "win32/DLL_Audio.h"

extern void DMA_AI();

#ifdef DEBUG_COMMON
#include <stdio.h>
#include "dbgprint.h"
extern char* DebugMainCPU();
char* DebugCOP(int n, uint32 Instruction);
char* DebugGPR(uint32 rt);
char* DebugCOPReg(uint32 n, uint32 reg);
char* DebugCOP0Reg(uint8 reg);
char* DebugCOP1Reg(uint8 reg);
extern char* DebugRegimm();
extern char* DebugSpecial();
extern void RefreshOpList(char *opcode);
extern void DebugPrintInstruction(uint32 instruction);
#endif

extern void ResizeVideoWindow(void);
extern BOOL Rom_Loaded;
extern uint32 cp0Counter;
extern uint8* dummySegment;

extern void Handle_MI();
extern void Handle_SP(uint32 value);
extern void Handle_DPC(uint32 value);

extern uint32 VIcounter;
extern BOOL VIupdated;

void DebugIO(uint32, char *, uint32);
//---------------------------------------------------------------------------------------

void r4300i_break(uint32 Instruction)
{
	// Trigger the BREAK Exception
	gHardwareState.COP0Reg[CAUSE] |= EXC_BREAK;
#ifndef CPUCHECKINTR
	CPUNeedToDoOtherTask = TRUE;
	CPUNeedToCheckException = TRUE;
#endif
}

void r4300i_sync(uint32 Instruction)
{
//#ifdef DEBUG_COMMON
//	DisplayError("Intruction SYNC, not implemented yet");
//#endif
}

void r4300i_syscall(uint32 Instruction)
{
#ifdef DEBUG_COMMON
//	DisplayError("Intruction SYSCALL, not implemented yet");
#endif
	//Cause a SYSCALL exception
	gHardwareState.COP0Reg[CAUSE] |= EXC_SYSCALL;
#ifndef CPUCHECKINTR
	CPUNeedToDoOtherTask = TRUE;
	CPUNeedToCheckException = TRUE;
#endif
}

void UNUSED(uint32 Instruction)         
{
	// Trigger the RI Exception
	gHardwareState.COP0Reg[CAUSE] |= EXC_II;
#ifndef CPUCHECKINTR
	CPUNeedToDoOtherTask = TRUE;
	CPUNeedToCheckException = TRUE;
#endif

	/*DisplayError("%08X: Illegal opcode request.", gHardwareState.pc);*/       
}

//---------------------------------------------------------------------------------------

void r4300i_add(uint32 Instruction) {sLOGICAL(+);}  
void r4300i_dadd(uint32 Instruction) {sDLOGICAL(+);}
void r4300i_addu(uint32 Instruction){sLOGICAL(+);}  
void r4300i_daddu(uint32 Instruction){sDLOGICAL(+);}
void r4300i_sub(uint32 Instruction) {sLOGICAL(-);}  
void r4300i_dsub(uint32 Instruction) {uDLOGICAL(-);}
void r4300i_subu(uint32 Instruction){sLOGICAL(-);}  
void r4300i_dsubu(uint32 Instruction){sDLOGICAL(-);}

void r4300i_addi(uint32 Instruction) {sLOGICAL_WITH_IMM(+);}    
void r4300i_daddi(uint32 Instruction) { sDLOGICAL_WITH_IMM(+);}
void r4300i_addiu(uint32 Instruction) { sDLOGICAL_WITH_IMM(+);}
/*
{
	gRT = (_int64)((_int32)gRS + OFFSET_IMMEDIATE);
	if( (Instruction & 0xFFFF) == 0xFFFF )
	{
		gRT = gRS + (_int16)OFFSET_IMMEDIATE;
	}
} 
*/  

void r4300i_daddiu(uint32 Instruction){ uDLOGICAL_WITH_IMM(+);}

void r4300i_and(uint32 Instruction) {uDLOGICAL(&);  }   
void r4300i_andi(uint32 Instruction){uDLOGICAL_WITH_IMM(&); }
void r4300i_or(uint32 Instruction)  {uDLOGICAL(|);  }   
void r4300i_ori(uint32 Instruction) {uDLOGICAL_WITH_IMM(|); }
void r4300i_xor(uint32 Instruction) {uDLOGICAL(^);  }   
void r4300i_xori(uint32 Instruction){uDLOGICAL_WITH_IMM(^); }
void r4300i_nor(uint32 Instruction) {uDLOGICAL(|~); }

//---------------------------------------------------------------------------------------


void r4300i_lb(uint32 Instruction)
{
    LOAD_TLB_FUN

    __try{
        gRT = *(_int8 *)(uint8*)(valloc+((QuerAddr^3)&0x1fffffff));
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            gRT = *(_int8 *)(uint8*)(valloc2+((QuerAddr^3)&0x1fffffff));
#ifdef DEBUG_IO_READ
			DisplayError("Use LB to read registers at 0x%08x", QuerAddr);
#endif
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("%08X: LB out of range", gHardwareState.pc);
			// Use dummy segment
			gRT = *(_int8 *)(uint8*)(valloc+((QuerAddr^3)&0x0000ffff)+0x1FFF0000);
        }
   }
}

//---------------------------------------------------------------------------------------

void r4300i_lbu(uint32 Instruction)
{   
    LOAD_TLB_FUN
    __try{
        gRT = *(uint8*)(valloc+((QuerAddr^3)&0x1fffffff));
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            gRT = *(uint8*)(valloc2+((QuerAddr^3)&0x1fffffff));
#ifdef DEBUG_IO_READ
			DisplayError("Use LBU to read registers at 0x%08x", QuerAddr);
#endif
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			//uint32 virtualaddress = (uint32)((_int32)gBASE + (_int32)OFFSET_IMMEDIATE);
			//DisplayError("%08X: LBU out of range VA=%08X, PA=%08X, Out of range. ", gHardwareState.pc, virtualaddress, QuerAddr);
			gHardwareState.COP0Reg[CAUSE] |= EXC_RADE;	// Trigger Address Error Exception
#ifndef CPUCHECKINTR
			CPUNeedToDoOtherTask = TRUE;
			CPUNeedToCheckException = TRUE;
#endif
			gHardwareState.COP0Reg[BADVADDR] = QuerAddr;

#ifdef DEBUG_TLB
			if( debug_tlb )
			{
				uint32 Instruction = FetchInstruction();
				uint32 virtualaddress = (uint32)((_int32)gBASE + (_int32)OFFSET_IMMEDIATE);
				//DisplayError("%08X: LBU to access VA=%08X, PA=%08X, Out of range. ", gHardwareState.pc, virtualaddress, QuerAddr);
				sprintf(generalmessage, "%08X: LBU to access VA=%08X, PA=%08X, Out of range. ", gHardwareState.pc, virtualaddress, QuerAddr);
				RefreshOpList(generalmessage); 
			}
#endif

			// Use dummy segment
			gRT = *(uint8 *)(valloc+((QuerAddr^3)&0x0000ffff)+0x1FFF0000);

        }
   }
}

//---------------------------------------------------------------------------------------

void r4300i_lh(uint32 Instruction)
{   
    LOAD_TLB_FUN
    __try{
        gRT = *(_int16*)(valloc+((QuerAddr^2)&0x1fffffff));
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            gRT = *(_int16*)(valloc2+((QuerAddr^2)&0x1fffffff));
#ifdef DEBUG_IO_READ
			DisplayError("Use LH to read registers at 0x%08x", QuerAddr);
#endif
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
            //DisplayError("%08X: LH out of range", gHardwareState.pc);
			gHardwareState.COP0Reg[CAUSE] |= EXC_RADE;	// Trigger Address Error Exception
#ifndef CPUCHECKINTR
			CPUNeedToDoOtherTask = TRUE;
			CPUNeedToCheckException = TRUE;
#endif
			gHardwareState.COP0Reg[BADVADDR] = QuerAddr;

#ifdef DEBUG_TLB
			if( debug_tlb )
			{
				uint32 Instruction = FetchInstruction();
				uint32 virtualaddress = (uint32)((_int32)gBASE + (_int32)OFFSET_IMMEDIATE);
				//DisplayError("%08X: LH to access VA=%08X, PA=%08X, Out of range. ", gHardwareState.pc, virtualaddress, QuerAddr);
				sprintf(generalmessage, "%08X: LH to access VA=%08X, PA=%08X, Out of range. ", gHardwareState.pc, virtualaddress, QuerAddr);
				RefreshOpList(generalmessage); 
			}
#endif
			// Use dummy segment
			gRT = *(_int16*)(valloc+((QuerAddr^2)&0x0000ffff)+0x1FFF0000);
        }
   }
}

//---------------------------------------------------------------------------------------

void r4300i_lhu(uint32 Instruction)
{   
    LOAD_TLB_FUN
    __try{
        gRT = *(uint16*)(valloc+((QuerAddr^2)&0x1fffffff));
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            gRT = *(uint16*)(valloc2+((QuerAddr^2)&0x1fffffff));
#ifdef DEBUG_IO_READ
			DisplayError("Use LHU to read registers at 0x%08x", QuerAddr);
#endif
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			//uint32 virtualaddress = (uint32)((_int32)gBASE + (_int32)OFFSET_IMMEDIATE);
			//DisplayError("%08X: LHU out of range VA=%08X, PA=%08X, Out of range. ", gHardwareState.pc, virtualaddress, QuerAddr);
			gHardwareState.COP0Reg[CAUSE] |= EXC_RADE;	// Trigger Address Error Exception
#ifndef CPUCHECKINTR
			CPUNeedToDoOtherTask = TRUE;
			CPUNeedToCheckException = TRUE;
#endif
			gHardwareState.COP0Reg[BADVADDR] = QuerAddr;
#ifdef DEBUG_TLB
			if( debug_tlb )
			{
				uint32 Instruction = FetchInstruction();
				uint32 virtualaddress = (uint32)((_int32)gBASE + (_int32)OFFSET_IMMEDIATE);
				sprintf(generalmessage, "%08X: LHU to access VA=%08X, PA=%08X, Out of range. ", gHardwareState.pc, virtualaddress, QuerAddr);
				RefreshOpList(generalmessage); 
			}
#endif
			// Use dummy segment
			gRT = *(uint16*)(valloc+((QuerAddr^2)&0x0000ffff)+0x1FFF0000);
        }
   }
}

//---------------------------------------------------------------------------------------

void r4300i_lwu(uint32 Instruction)
{   
    uint32 rt_ft = RT_FT;

    LOAD_TLB_FUN
    __try{
        gRT = *(uint32*)(valloc+((QuerAddr)&0x1fffffff));
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            gRT = *(uint32*)(valloc2+((QuerAddr)&0x1fffffff));
#ifdef DEBUG_IO_READ
			DisplayError("Use LWU to read registers at 0x%08x", QuerAddr);
#endif

        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("%08X: LWU out of range", gHardwareState.pc);
        }
   }
}

//---------------------------------------------------------------------------------------

void r4300i_ll(uint32 Instruction)
{   
    LOAD_TLB_FUN
    __try{
        gRT = *(uint32*)(valloc+((QuerAddr)&0x1fffffff));
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            gRT = *(uint32*)(valloc2+((QuerAddr)&0x1fffffff));
#ifdef DEBUG_IO_READ
			DisplayError("Use LL to read registers at 0x%08x", QuerAddr);
#endif
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("%08X: LL out of range", gHardwareState.pc);
        }
   }
    gHardwareState.LLbit = 1;
}

//---------------------------------------------------------------------------------------

void r4300i_ld(uint32 Instruction)
{
    uint32 UWORD[2];
    uint32 rt_ft = RT_FT;

    LOAD_TLB_FUN

    *(uint64*)UWORD = ((uint64)*((uint64*)sDWORD_R[((QuerAddr) >> 16)]+(((uint16)QuerAddr) >> 3)));
                 (uint32)gHardwareState.GPR[rt_ft]    = UWORD[1];
    *(uint32*)(&(uint32*)gHardwareState.GPR[rt_ft]+1) = UWORD[0];
}

//---------------------------------------------------------------------------------------

void r4300i_lld(uint32 Instruction)
{
    uint32 UWORD[2];
    uint32 rt_ft = RT_FT;

    LOAD_TLB_FUN

    *(uint64*)UWORD = ((uint64)*((uint64*)sDWORD_R[((uint16)((QuerAddr) >> 16))]+(((uint16)QuerAddr) >> 3)));
                 (uint32)gHardwareState.GPR[rt_ft]    = UWORD[1];
    *(uint32*)(&(uint32*)gHardwareState.GPR[rt_ft]+1) = UWORD[0];

    gHardwareState.LLbit = 1;
}

//---------------------------------------------------------------------------------------

void r4300i_sb(uint32 Instruction){
    //LOAD_TLB_FUN
	STORE_TLB_FUN
    __try {
        *(uint8 *)(uint8*)(valloc+((QuerAddr^3)&0x1fffffff)) =  (uint8)gRT;
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){ 
        __try {
            *(uint8 *)(uint8*)(valloc2+((QuerAddr^3)&0x1fffffff)) =  (uint8)gRT;
#ifdef DEBUG_IO_WRITE
			DisplayError("Use SB to read registers at 0x%08x", QuerAddr);
#endif

        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
#ifdef DEBUG_TLB
			if( debug_tlb )
			{
				uint32 Instruction = FetchInstruction();
				uint32 virtualaddress = (uint32)((_int32)gBASE + (_int32)OFFSET_IMMEDIATE);
				//DisplayError("%08X: SB to access VA=%08X, PA=%08X, Out of range. ", gHardwareState.pc, virtualaddress, QuerAddr);
				sprintf(generalmessage, "%08X: SB to access VA=%08X, PA=%08X, Out of range. ", gHardwareState.pc, virtualaddress, QuerAddr);
				RefreshOpList(generalmessage); 
			}
#endif
			//uint32 virtualaddress = (uint32)((_int32)gBASE + (_int32)OFFSET_IMMEDIATE);
			//DisplayError("%08X: SB out of range VA=%08X, PA=%08X, Out of range. ", gHardwareState.pc, virtualaddress, QuerAddr);
			gHardwareState.COP0Reg[CAUSE] |= EXC_WADE;	// Trigger Address Error Exception
#ifndef CPUCHECKINTR
			CPUNeedToDoOtherTask = TRUE;
			CPUNeedToCheckException = TRUE;
#endif
			gHardwareState.COP0Reg[BADVADDR] = QuerAddr;

			// Use dummy segment
			*(uint8 *)(valloc+((QuerAddr^3)&0x0000ffff)+0x1FFF0000) = (uint8)gRT;
        }
    }
}

//---------------------------------------------------------------------------------------

void r4300i_sh(uint32 Instruction)
{
    //LOAD_TLB_FUN
	STORE_TLB_FUN
    __try {
        *(uint16 *)(uint8*)(valloc+((QuerAddr^2)&0x1fffffff)) =  (uint16)gRT;
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){ 
        __try {
            *(uint16 *)(uint8*)(valloc2+((QuerAddr^2)&0x1fffffff)) =  (uint16)gRT;
#ifdef DEBUG_IO_WRITE
			DisplayError("Use SH to read registers at 0x%08x", QuerAddr);
#endif
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			//uint32 virtualaddress = (uint32)((_int32)gBASE + (_int32)OFFSET_IMMEDIATE);
			//DisplayError("%08X: SH out of range VA=%08X, PA=%08X, Out of range. ", gHardwareState.pc, virtualaddress, QuerAddr);
			gHardwareState.COP0Reg[CAUSE] |= EXC_WADE;	// Trigger Address Error Exception
#ifndef CPUCHECKINTR
			CPUNeedToDoOtherTask = TRUE;
			CPUNeedToCheckException = TRUE;
#endif
			gHardwareState.COP0Reg[BADVADDR] = QuerAddr;
#ifdef DEBUG_TLB
			if( debug_tlb )
			{
				uint32 Instruction = FetchInstruction();
				uint32 virtualaddress = (uint32)((_int32)gBASE + (_int32)OFFSET_IMMEDIATE);
				sprintf(generalmessage, "%08X: SH to access VA=%08X, PA=%08X, Out of range. ", gHardwareState.pc, virtualaddress, QuerAddr);
				RefreshOpList(generalmessage); 
			}
#endif
			// Use dummy segment
			*(uint16 *)(uint8*)(valloc+((QuerAddr^2)&0x0000ffff)+0x1FFF0000) = (uint16)gRT;
        }
    }
}

//---------------------------------------------------------------------------------------

void r4300i_sd(uint32 Instruction)
{
    uint64* StoreDW;
    uint32 rt_ft = RT_FT;

    //LOAD_TLB_FUN
	STORE_TLB_FUN

    StoreDW = ((uint64*)sDWORD_R[((QuerAddr) >> 16)]+(((uint16)QuerAddr) >> 3));
    *(uint32*)(&(uint32*)StoreDW[0]+1) = (uint32)gHardwareState.GPR[rt_ft];
    *(uint32*)StoreDW = *(uint32*)(&(uint32*)gHardwareState.GPR[rt_ft]+1);
}

//---------------------------------------------------------------------------------------

void r4300i_sc(uint32 Instruction)
{   
    if (gHardwareState.LLbit) 
    {
        //LOAD_TLB_FUN
		STORE_TLB_FUN
        __try{
            *(uint32*)(uint8*)(valloc+(QuerAddr & 0x1fffffff)) = (uint32)gRT;
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
            __try{
                *(uint32*)(uint8*)(valloc2+(QuerAddr & 0x1fffffff)) = (uint32)gRT;
#ifdef DEBUG_IO_WRITE
			DisplayError("Use SC to read registers at 0x%08x", QuerAddr);
#endif
            }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
                DisplayError("%08X: SC out of range", gHardwareState.pc);
            }
        }
   }
   (uint64)gRT = (uint64)gHardwareState.LLbit;
}

//---------------------------------------------------------------------------------------

//TODO: needs try/except
void r4300i_scd(uint32 Instruction)
{
    uint64* StoreDW;
    uint32 rt_ft = RT_FT;

    if (gHardwareState.LLbit)
    {
        //LOAD_TLB_FUN
		STORE_TLB_FUN

        StoreDW = ((uint64*)sDWORD_R[((QuerAddr) >> 16)]+(((uint16)QuerAddr) >> 3));
        *(uint32*)(&(uint32*)StoreDW[0]+1) = (uint32)gHardwareState.GPR[rt_ft];
        *(uint32*)StoreDW                  = *(uint32*)(&(uint32*)gHardwareState.GPR[rt_ft]+1);
    } 
    (uint64)gHardwareState.GPR[rt_ft] = (uint64)gHardwareState.LLbit; 
}

//---------------------------------------------------------------------------------------

void r4300i_mfhi(uint32 Instruction){gRD = gHardwareState.GPR[gHI];}  
void r4300i_mthi(uint32 Instruction){gHardwareState.GPR[gHI] = gRS;}
void r4300i_mflo(uint32 Instruction){gRD = gHardwareState.GPR[gLO];}  
void r4300i_mtlo(uint32 Instruction){gHardwareState.GPR[gLO] = gRS;}

//---------------------------------------------------------------------------------------

void r4300i_sll(uint32 Instruction) {    uLOGICAL_SHIFT(<<, SA_FD); }   
void r4300i_sllv(uint32 Instruction){    uLOGICAL_SHIFT(<<, (gRS&0x1F));}


void r4300i_srl(uint32 Instruction){    uLOGICAL_SHIFT(>>, SA_FD);                   }
void r4300i_srlv(uint32 Instruction){   uLOGICAL_SHIFT(>>, (gRS&0x1F)); }


void r4300i_sra(uint32 Instruction){     sLOGICAL_SHIFT(>>, SA_FD);                  }
void r4300i_srav(uint32 Instruction){    sLOGICAL_SHIFT(>>, (gRS&0x1F));}

//Questionable: dsra & dsrav are probably not getting sign-extended
//---------------------------------------------------------------------------------------

void r4300i_dsrlv(uint32 Instruction){ (uint64)gRD = (uint64)gRT >> (gRS&0x3F);}
void r4300i_dsrav(uint32 Instruction) {        gRD =         gRT >> (gRS&0x3F);}
void r4300i_dsllv(uint32 Instruction){ (uint64)gRD = (uint64)gRT << (gRS&0x3F);}
void r4300i_dsll(uint32 Instruction){  (uint64)gRD = (uint64)gRT << SA_FD; }   
void r4300i_dsrl(uint32 Instruction){  (uint64)gRD = (uint64)gRT >> SA_FD; }
void r4300i_dsra(uint32 Instruction){          gRD =         gRT >> SA_FD; }   

//---------------------------------------------------------------------------------------

void r4300i_dsll32(uint32 Instruction){ 
    *(uint32*)((uint8*)&gRD+4) = (uint32)gRT << SA_FD;
    *(uint32*)((uint8*)&gRD  ) = 0;
}

//---------------------------------------------------------------------------------------

void r4300i_dsrl32(uint32 Instruction){
    (uint32)gRD = *(uint32*)((uint8*)&gRT+4);
    *(uint32*)((uint8*)&gRD) = *(uint32*)((uint8*)&gRD) >> SA_FD;
    *(uint32*)((uint8*)&gRD+4) = 0;
}

//---------------------------------------------------------------------------------------

void r4300i_dsra32(uint32 Instruction){ 
    (uint32)gRD = *(uint32*)((uint8*)&gRT+4);
    sLOGICAL_SHIFT(>>, SA_FD);
}

//---------------------------------------------------------------------------------------

void r4300i_slt(uint32 Instruction)     
{if (        gRS <          gRT     )   gRD = 1;    else gRD = 0;}

void r4300i_sltu(uint32 Instruction)    
{if ((uint64)gRS < (uint64)gRT      )   gRD = 1;    else gRD = 0;}

void r4300i_slti(uint32 Instruction)    
{if (        gRS <          OFFSET_IMMEDIATE)   gRT = 1;    else gRT = 0;}

void r4300i_sltiu(uint32 Instruction)
{if ((uint64)gRS < (uint64)(_int64)(_int32)(_int16)(uint16)OFFSET_IMMEDIATE)
gRT = 1;    else gRT = 0;}


extern int whichcore;
extern uint32 VIcounter;
extern uint32 cp0Counter;
//extern BOOL finish_dyna_exception;
void r4300i_COP0_mfc0(uint32 Instruction) 
{
	switch (RD_FS)
	{
	case RANDOM:
		gHardwareState.COP0Reg[RANDOM] = gHardwareState.COP0Reg[COUNT] % (0x40-(gHardwareState.COP0Reg[WIRED]&0x3f))+gHardwareState.COP0Reg[WIRED];
		break;
#ifdef DEBUG_COMPARE_INTERRUPT
	case COUNT:
		if( debug_interrupt && debug_compare_interrupt )
		{
			sprintf(generalmessage, "COUNT register is read = %8X, COMPARE=%8X , PC=0x%08X", c0FS,gHardwareState.COP0Reg[COMPARE],gHardwareState.pc);
			RefreshOpList(generalmessage); 
		}
		break;
#endif
	//case EPC:
		//finish_dyna_exception = TRUE;
//#ifdef DEBUG_COMMON
//		sprintf(generalmessage, "EPC register is read = %8X, COMPARE=%8X , PC=0x%08X", c0FS,gHardwareState.COP0Reg[COMPARE],gHardwareState.pc);
//		RefreshOpList(generalmessage); 
//#endif
//		break;
	}
	gRT = (_int64)(_int32)c0FS; 
}

void r4300i_COP0_mtc0(uint32 Instruction) 
{
	switch( RD_FS )
	{
	case INDEX:			// The INDEX Register
		//gHardwareState.COP0Reg[INDEX] = (uint32)gRT & 0x1F;
		//gHardwareState.COP0Reg[INDEX] = (uint32)gRT & 0x3F;
		gHardwareState.COP0Reg[INDEX] = gHardwareState.COP0Reg[INDEX] & 0x80000000 | ((uint32)gRT & 0x3F);
		return;
	case RANDOM:			// The RANDOM register, read only
		return;
	//case 13: 	// The CAUSE register
	//	c0FS = ((uint32)gRT & 0x300) | (c0FS & ~(0x300));
	//	return;
	case ENTRYLO0:
	case ENTRYLO1:
		c0FS = (uint32)gRT & 0x3FFFFFFF;
		return;
	case PAGEMASK:		// The PAGEMASK register
		gHardwareState.COP0Reg[PAGEMASK] = (uint32)gRT & 0x01ffe000;
		return;
	case WIRED:			// The WIRED register
		gHardwareState.COP0Reg[WIRED] = (uint32)gRT & 0x3f;
		//gHardwareState.COP0Reg[WIRED] = (uint32)gRT & 0x1f;
		gHardwareState.COP0Reg[RANDOM] = 0x1f;
		return;
	case BADVADDR:			// BADVADDR register, Read only registers
		return;
#ifdef DEBUG_COMPARE_INTERRUPT
	case COUNT:
		if( debug_interrupt && debug_compare_interrupt )
		{
			sprintf(generalmessage, "COUNT register is written = %8X, COMPARE=%8X ", (uint32)gRT,gHardwareState.COP0Reg[COMPARE]);
			RefreshOpList(generalmessage); 
		}
		break;
#endif
	case ENTRYHI:		// The EntryHi register
		gHardwareState.COP0Reg[ENTRYHI] = (uint32)gRT & 0xFFFFE0FF;
		return;
	case COMPARE:		// The Compare register
		gHardwareState.COP0Reg[CAUSE] &= 0xffff7fff;	// Clear current COMPARE interrupt
#ifdef DEBUG_COMPARE_INTERRUPT
		if( debug_interrupt && debug_compare_interrupt )
		{
			sprintf(generalmessage, "COMPARE register is rewritten = %8X, COUNT=%8X ", (uint32)gRT,gHardwareState.COP0Reg[COUNT]);
			RefreshOpList(generalmessage); 
		}
#endif
		gHardwareState.COP0Reg[COMPARE]  = (uint32)gRT;
		break;
		/*
#ifdef CPUCHECKINTR
	case STATUS:		// The status register
		if( gRT & EXL == 0 && gHardwareState.COP0Reg[STATUS] & EXL == 1)
		{
#ifdef DEBUG_COMMON
			sprintf(generalmessage, "EXL Bit is cleared at PC = %8X, COMPARE=%8X , PC=0x%08X", gRT,gHardwareState.COP0Reg[COMPARE],gHardwareState.pc);
			RefreshOpList(generalmessage); 
#endif
			// CPU will check interrupts at the next cycle
			CPUNeedToCheckException = TRUE;
			CPUNeedToDoOtherTask = TRUE;
		}

		if( gRT & IE == 1 &&  gHardwareState.COP0Reg[STATUS] & IE == 0 )		// If enable interrupt
		{
			// CPU will check interrupts at the next cycle
			CPUNeedToDoOtherTask = TRUE;
			CPUNeedToCheckInterrupt = TRUE;
		}
		gHardwareState.COP0Reg[STATUS] = (uint32)gRT;
		return;
#endif
		*/
	//case EPC:
	//	finish_dyna_exception = TRUE;
	//	break;
	case PREVID:		// PRID register, Read only registers
		return;		// This makes BomberMan Hero not to freeze after [START]
	}
	
	c0FS = (uint32)gRT;
}

//---------------------------------------------------------------------------------------

void r4300i_lui(uint32 Instruction)     
{
	gRT = OFFSET_IMMEDIATE << (uint32)16;
}

//---------------------------------------------------------------------------------------
__inline void r4300i_speedhack()
{
	// Before SPEEDHACK, let CPU to finish all other tasks, let DMA, SP Task and so on
	if( CPUNeedToDoOtherTask )
		return;

#ifdef SAVEVICOUNTER
	if( gHardwareState.COP0Reg[COUNT] < gHardwareState.COP0Reg[COMPARE] &&
		gHardwareState.COP0Reg[COUNT] + MAGICNUMBERFORCOUNTREG >= gHardwareState.COP0Reg[COMPARE] )
	{
		Trigger_CompareInterrupt();
		VIcounter += (gHardwareState.COP0Reg[COMPARE] - gHardwareState.COP0Reg[COUNT] )*2;
		gHardwareState.COP0Reg[COUNT] = gHardwareState.COP0Reg[COMPARE];
	}
	else
	{
		gHardwareState.COP0Reg[COUNT] += (MAGICNUMBER-VIcounter)/2;
		VIcounter = MAGICNUMBER;
	}
#else
	gHardwareState.COP0Reg[COUNT] += MAGICNUMBER;
	VIcounter += MAGICNUMBER;
	if( ( gHardwareState.COP0Reg[COUNT] > gHardwareState.COP0Reg[COMPARE] &&
		  gHardwareState.COP0Reg[COUNT] - MAGICNUMBER < gHardwareState.COP0Reg[COMPARE] ) ||
		( gHardwareState.COP0Reg[COUNT] < gHardwareState.COP0Reg[COUNT] - MAGICNUMBER &&
		  gHardwareState.COP0Reg[COUNT] > gHardwareState.COP0Reg[COMPARE] ))
	{
		Trigger_CompareInterrupt();
	}
#endif
}

void r4300i_bgez(uint32 Instruction)    
{                       
	if (gRS >= 0)   
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
        else 
#endif
		DELAY_SET                   
	}
}
void r4300i_bgezal(uint32 Instruction)  
{
	INTERPRETIVE_LINK(RA)  
	if (gRS >= 0)   
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
        else 
#endif
		DELAY_SET   
	}
}

void r4300i_bgezall(uint32 Instruction) 
{
	INTERPRETIVE_LINK(RA)  
	if (gRS >= 0)   
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
        else 
#endif
		DELAY_SET   
	}
	else 
		DELAY_SKIP 
}
void r4300i_bgezl(uint32 Instruction)   
{                       
	if (gRS >= 0)   
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
        else 
#endif
		DELAY_SET   
	}
	else 
		DELAY_SKIP 
}

void r4300i_bltz(uint32 Instruction)    
{                       
	if (gRS <  0)   
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
        else 
#endif
		DELAY_SET                   
	}
}
void r4300i_bltzal(uint32 Instruction)  
{
	INTERPRETIVE_LINK(RA)  
	if (gRS <  0)   
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
        else 
#endif
		DELAY_SET   
	}
}
void r4300i_bltzall(uint32 Instruction) 
{
	INTERPRETIVE_LINK(RA)  
	if (gRS <  0)   
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
        else 
#endif
		DELAY_SET   
	}
	else 
		DELAY_SKIP 
}

void r4300i_bltzl(uint32 Instruction)   
{                       
	if (gRS <  0)   
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
        else 
#endif
		DELAY_SET   
	}
	else 
		DELAY_SKIP 
}

void r4300i_bgtz(uint32 Instruction)    
{
	if (gRS >  0)
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
        else 
#endif
		DELAY_SET                   
	}
}

void r4300i_bgtzl(uint32 Instruction)   
{
	if (gRS >  0)   
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
        else 
#endif
		DELAY_SET   
	}
	else DELAY_SKIP 
}

void r4300i_blez(uint32 Instruction)    
{
	if (gRS <= 0)   
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
        else 
#endif
		DELAY_SET                   
	}
}

void r4300i_blezl(uint32 Instruction)   
{
	if (gRS <= 0)   
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
        else 
#endif
		DELAY_SET   
	}
	else 
		DELAY_SKIP 
}

void r4300i_bne(uint32 Instruction)     
{
	if ((uint64)gRS != (uint64)gRT)
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
        else 
#endif
		DELAY_SET                   
	}
}

void r4300i_bnel(uint32 Instruction)
{
	if ((uint64)gRS != (uint64)gRT)  
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF )
		{
			if ( LOAD_UWORD_PARAM(gHardwareState.pc+4)==0 ) 
			{
				CPUdelayPC = gHardwareState.pc;
				CPUdelay = 1;
				r4300i_speedhack();
			}
		// speed hack, like in Banjo Game
		// 802483F4:	BNEL	v0,r0, 802483F4,   where r0 = 0
		// 802483f*:	ADDIU	v0,v0, FFFFFFFC    v0=v0-4
		//
		//-----------------------------------------------------------------
		//| BNEL      | Branch on Not Equal Likley                        |
		//|-----------|---------------------------------------------------|
		//|010101 (21)|   rs    |   rt    |            offset             |
		//------6----------5---------5-------------------16----------------
		//
		//
		//-----------------------------------------------------------------
		//| ADDIU     | ADD Immediate Unsigned word                       |
		//|-----------|---------------------------------------------------|
		//|001001 (9) |   rs    |   rt    |          immediate            |
		//------6----------5---------5-------------------16----------------
		//

		else if( (LOAD_UWORD_PARAM(gHardwareState.pc+4) & 0xFC000000 ) == 0x24000000 )
			{
				int grt1, grt2, grs1, grs2, inc, count;
				uint32 Instruction = LOAD_UWORD_PARAM(gHardwareState.pc);

				grt1 = RT_FT;
				grs1 = RS_BASE_FMT;
				Instruction = LOAD_UWORD_PARAM(gHardwareState.pc+4);
				grt2 = RT_FT;
				grs2 = RS_BASE_FMT;
				inc = (_int16)OFFSET_IMMEDIATE;

				count = ((uint32)gHardwareState.GPR[grt1] - (uint32)gHardwareState.GPR[grt2])/inc;


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
		}
		
        else 
#endif
		DELAY_SET
	}
	else
		DELAY_SKIP 
}

void r4300i_beql(uint32 Instruction)    
{
	if ((uint64)gRS == (uint64)gRT)    
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
        else 
#endif
		DELAY_SET   
	}
	else 
		DELAY_SKIP 
}

void r4300i_beq(uint32 Instruction)     
{
    if ((uint64)gRS == (uint64)gRT)
    {
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
        else 
#endif
			DELAY_SET
    }
}

//---------------------------------------------------------------------------------------

void r4300i_div(uint32 Instruction) {

    if ((uint32)gRT != 0) //Divide by zero in rom!!! (really should generate exception)
    {   
        sLOGIC(gHardwareState.GPR[gLO], gRS, /, gRT);
        sLOGIC(gHardwareState.GPR[gHI], gRS, %, gRT);
    }
#ifdef SAVEOPCOUNTER
	VIcounter += PCLOCKDIV;
	VIcounter += PCLOCKDIV;
	gHardwareState.COP0Reg[COUNT] +=  PCLOCKDIV;
#endif
}

//---------------------------------------------------------------------------------------

void r4300i_divu(uint32 Instruction) {

    uint32 RSReg = (uint32)gRS;
    uint32 RTReg = (uint32)gRT;

    if ((uint32)RTReg != 0) //Divide by zero in rom!!!
    {
        uLOGIC(gHardwareState.GPR[gLO], RSReg, /, RTReg);
        uLOGIC(gHardwareState.GPR[gHI], RSReg, %, RTReg);
    }
#ifdef SAVEOPCOUNTER
	VIcounter += PCLOCKDIVU;
	VIcounter += PCLOCKDIVU;
	gHardwareState.COP0Reg[COUNT] +=  PCLOCKDIVU;
#endif
}

//---------------------------------------------------------------------------------------

void r4300i_ddiv(uint32 Instruction) {
    _int64 TempRS;
    _int64 RTReg = (_int64)gRT;

    if ((uint32)RTReg != 0) //Divide by zero in rom!!!
    {   
        gHardwareState.GPR[gLO] = (_int64)(TempRS=(uint64)gRS)    / RTReg;
        gHardwareState.GPR[gHI] = TempRS                          % RTReg;
    }
#ifdef SAVEOPCOUNTER
	VIcounter += PCLOCKDDIV;
	VIcounter += PCLOCKDDIV;
	gHardwareState.COP0Reg[COUNT] +=  PCLOCKDDIV;
#endif
}

//---------------------------------------------------------------------------------------

void r4300i_ddivu(uint32 Instruction) {
    _int64 TempRS;
    uint32 RTReg = (uint32)gRT;

    if ((uint32)RTReg != 0) //Divide by zero in rom!!!
    {   
        gHardwareState.GPR[gLO] = ((uint32)(TempRS=(uint64)gRS)) / RTReg;
        gHardwareState.GPR[gHI] =  (uint32)TempRS                % RTReg;
    }
#ifdef SAVEOPCOUNTER
	VIcounter += PCLOCKDDIVU;
	VIcounter += PCLOCKDDIVU;
	gHardwareState.COP0Reg[COUNT] +=  PCLOCKDDIVU;
#endif
}

//---------------------------------------------------------------------------------------

void r4300i_mult(uint32 Instruction) {
    _int64 result;

    result = (_int64)(_int32)gRS * (_int64)(_int32)gRT;
    gHardwareState.GPR[gLO] = (_int32)result;
    gHardwareState.GPR[gHI] = (_int32)(result >> 32);
#ifdef SAVEOPCOUNTER
	VIcounter += PCLOCKMULT;
	VIcounter += PCLOCKMULT;
	gHardwareState.COP0Reg[COUNT] +=  PCLOCKMULT;
#endif
}

//---------------------------------------------------------------------------------------

void r4300i_multu(uint32 Instruction) {
    uint64 result;

    result = (uint64)(uint32)gRS * (uint64)(uint32)gRT;
    gHardwareState.GPR[gLO] = (_int32)result;
    gHardwareState.GPR[gHI] = (_int32)(result >> 32);
#ifdef SAVEOPCOUNTER
	VIcounter += PCLOCKMULTU;
	VIcounter += PCLOCKMULTU;
	gHardwareState.COP0Reg[COUNT] +=  PCLOCKMULTU;
#endif
}

//---------------------------------------------------------------------------------------

void r4300i_dmultu(uint32 Instruction) {
     unsigned __int64 hh,hl,lh,ll,b;
     __int64 t1,t2;

     t1 = gRS;
     t2 = gRT;

     hh = ((__int64)(t1 >> 32)  & 0x0ffffffff)  * ((__int64)(t2 >> 32)  &   0x0ffffffff);
     hl =  (__int64)(t1         & 0x0ffffffff)  * ((__int64)(t2 >> 32)  &   0x0ffffffff);
     lh = ((__int64)(t1 >> 32)  & 0x0ffffffff)  *  (__int64)(t2         &   0x0ffffffff);
     ll = ((__int64)(t1         & 0x0ffffffff)  *  (__int64)(t2         &   0x0ffffffff));

     gHardwareState.GPR[gLO] = ((hl + lh) << 32) + ll;

     b=(((hl + lh) + (ll >> 32)) & 0x0100000000)>>32;

     gHardwareState.GPR[gHI] = (unsigned __int64)hh + ((signed __int64)(unsigned __int32)(hl
>> 32) + (signed __int64)(unsigned __int32)(lh >> 32) + b);

#ifdef SAVEOPCOUNTER
	VIcounter += PCLOCKDMULTU;
	VIcounter += PCLOCKDMULTU;
	gHardwareState.COP0Reg[COUNT] +=  PCLOCKDMULTU;
#endif

}

//---------------------------------------------------------------------------------------

void r4300i_dmult(uint32 Instruction) {
     unsigned __int64 hh,hl,lh,ll,b;
     __int64 t1,t2;
     int sgn = 0;

     t1 = gRS;
     t2 = gRT;
     if (t1 < 0) {sgn ^= 1; t1 = -t1;}
     if (t2 < 0) {sgn ^= 1; t2 = -t2;}

     hh = ((__int64)(t1 >> 32) & 0x0ffffffff)   * ((__int64)(t2 >> 32)  &   0x0ffffffff);
     hl = (__int64)(t1 & 0x0ffffffff)           * ((__int64)(t2 >> 32)  &   0x0ffffffff);
     lh = ((__int64)(t1 >> 32) & 0x0ffffffff)   *  (__int64)(t2         &   0x0ffffffff);
     ll = ((__int64)(t1 & 0x0ffffffff)          *  (__int64)(t2         &   0x0ffffffff));

     gHardwareState.GPR[gLO] = ((hl + lh) << 32) + ll;

     b=(((hl + lh) + (ll >> 32)) & 0x0100000000)>>32;

     gHardwareState.GPR[gHI] = (unsigned __int64)hh + ((signed __int64)(unsigned __int32)(hl  >> 32) + (signed __int64)(unsigned __int32)(lh >> 32) + b);

     b = (gHardwareState.GPR[gLO] >= 0) ? 1 : 0;

     if (sgn != 0)
     {
          gHardwareState.GPR[gLO] = -gHardwareState.GPR[gLO];
          gHardwareState.GPR[gHI] = -gHardwareState.GPR[gHI] + b;
     }
#ifdef SAVEOPCOUNTER
	VIcounter += PCLOCKDMULT;
	VIcounter += PCLOCKDMULT;
	gHardwareState.COP0Reg[COUNT] +=  PCLOCKDMULT;
#endif
}

//---------------------------------------------------------------------------------------

void r4300i_jal(uint32 Instruction) 
{   
	INTERPRETIVE_LINK(RA)       
	CPUdelay = 1;   
	CPUdelayPC = INSTR_INDEX;   
}

void r4300i_jalr(uint32 Instruction)
{   
	INTERPRETIVE_LINK(RD_FS)    
	CPUdelay = 1;   
	CPUdelayPC = (uint32)gRS;   
}

void r4300i_jr(uint32 Instruction)  
{                               
	CPUdelay = 1;   
	CPUdelayPC = (uint32)gRS;   
}

void r4300i_j(uint32 Instruction)   
{
	CPUdelay = 1;   
	CPUdelayPC = INSTR_INDEX;
#ifdef DOSPEEDHACK    
    if (gHardwareState.pc == CPUdelayPC && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0)
	{
		r4300i_speedhack();
	}
#endif
}

//---------------------------------------------------------------------------------------

void r4300i_lw(uint32 Instruction)
{
	uint32 rt_ft = RT_FT;

    LOAD_TLB_FUN

   __try{
      gHardwareState.GPR[rt_ft] = *(_int32*)(uint8*)(valloc+(QuerAddr & 0x1fffffff));
   }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
      Check_LW(QuerAddr, rt_ft);
   }
}

//---------------------------------------------------------------------------------------
extern void Faster_Check_SW(uint32 QuerAddr, uint32 rt_ft);
void r4300i_sw(uint32 Instruction) 
{   
	uint32 rt_ft = RT_FT;

    //LOAD_TLB_FUN
	STORE_TLB_FUN

   __try{
      *(uint32*)(uint8*)(valloc+(QuerAddr & 0x1fffffff)) = (uint32)gHardwareState.GPR[rt_ft];
   }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        Check_SW(QuerAddr, rt_ft);
		//Faster_Check_SW(QuerAddr, rt_ft);
   }
}

//---------------------------------------------------------------------------------------

void r4300i_lwl(uint32 Instruction)
{
    uint32 LoadWord;
    uint32 rt_ft = RT_FT;
    uint32 vAddr;

    LOAD_TLB_FUN
    vAddr = QuerAddr & 0xfffffffc;
    __try{
        LoadWord = *(uint32*)(valloc+((vAddr)&0x1fffffff));
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            LoadWord = *(uint32*)(valloc2+((vAddr)&0x1fffffff));
#ifdef DEBUG_IO_READ
			DisplayError("Use LWL to read registers at 0x%08x", QuerAddr);
#endif
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("%08X: LWL out of range", gHardwareState.pc);
        }
    }

    switch (QuerAddr & 3) 
    {
        case 0: gRT = (_int64)(_int32)LoadWord;                               break;
        case 1: gRT = (_int64)(_int32)((gRT & 0x000000ff) | (LoadWord << 8)); break;
        case 2: gRT = (_int64)(_int32)((gRT & 0x0000ffff) | (LoadWord << 16));break;
        case 3: gRT = (_int64)(_int32)((gRT & 0x00ffffff) | (LoadWord << 24));break;
    }
}

//---------------------------------------------------------------------------------------

void r4300i_lwr(uint32 Instruction) 
{
    uint32 LoadWord;
    uint32 rt_ft = RT_FT;
    uint32 vAddr;

    LOAD_TLB_FUN
    
    vAddr = QuerAddr & 0xfffffffc;
    __try{
        LoadWord = *(uint32*)(valloc+((vAddr)&0x1fffffff));
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            LoadWord = *(uint32*)(valloc2+((vAddr)&0x1fffffff));
#ifdef DEBUG_IO_READ
			DisplayError("Use LWR to read registers at 0x%08x", QuerAddr);
#endif
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("%08X: LWR out of range", gHardwareState.pc);
        }
    }

    switch (QuerAddr & 3) 
    {
        case 3: gRT = (_int64)(gRT & 0xffffffff00000000) |  LoadWord;           break;
        case 2: gRT = (_int64)(gRT & 0xffffffffff000000) | (LoadWord >> 8);     break;
        case 1: gRT = (_int64)(gRT & 0xffffffffffff0000) | (LoadWord >> 16);    break;
        case 0: gRT = (_int64)(gRT & 0xffffffffffffff00) | (LoadWord >> 24);    break;
    }
}

//---------------------------------------------------------------------------------------

void r4300i_swl(uint32 Instruction)
{
    uint32 LoadWord;
    uint32 rt_ft = RT_FT;
    uint32 vAddr;

    //LOAD_TLB_FUN
	STORE_TLB_FUN
    vAddr = QuerAddr & 0xfffffffc;
    __try{
        LoadWord = *(uint32*)(valloc+((vAddr)&0x1fffffff));
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            LoadWord = *(uint32*)(valloc2+((vAddr)&0x1fffffff));
#ifdef DEBUG_IO_WRITE
			DisplayError("Use SWL to read registers at 0x%08x", QuerAddr);
#endif
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("%08X: SWL out of range", gHardwareState.pc);
        }
    }

    switch (QuerAddr & 3)
    {
        case 0: LOAD_UWORD_PARAM(vAddr) = (uint32)gRT;                                          break;
        case 1: LOAD_UWORD_PARAM(vAddr) = (uint32)(LoadWord & 0xff000000) | ((uint32)gRT >> 8); break;
        case 2: LOAD_UWORD_PARAM(vAddr) = (uint32)(LoadWord & 0xffff0000) | ((uint32)gRT >> 16);    break;
        case 3: LOAD_UWORD_PARAM(vAddr) = (uint32)(LoadWord & 0xffffff00) | ((uint32)gRT >> 24);    break;
    }
}

//---------------------------------------------------------------------------------------

void r4300i_swr(uint32 Instruction) 
{
    uint32 LoadWord;
    uint32 rt_ft = RT_FT;
    uint32 vAddr;

    //LOAD_TLB_FUN
	STORE_TLB_FUN
    vAddr = QuerAddr & 0xfffffffc;
    __try{
        LoadWord = *(uint32*)(valloc+((vAddr)&0x1fffffff));
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            LoadWord = *(uint32*)(valloc2+((vAddr)&0x1fffffff));
#ifdef DEBUG_IO_WRITE
			DisplayError("Use SWR to read registers at 0x%08x", QuerAddr);
#endif
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("%08X: SWR out of range", gHardwareState.pc);
        }
    }

    switch (QuerAddr & 3)
    {
        case 3: LOAD_UWORD_PARAM(vAddr) = (uint32)gRT;                                              break;
        case 2: LOAD_UWORD_PARAM(vAddr) = (uint32)((LoadWord & 0x000000FF) | ((uint32)gRT << 8));   break;
        case 1: LOAD_UWORD_PARAM(vAddr) = (uint32)((LoadWord & 0x0000FFFF) | ((uint32)gRT << 16));  break;
        case 0: LOAD_UWORD_PARAM(vAddr) = (uint32)((LoadWord & 0x00FFFFFF) | ((uint32)gRT << 24));  break;
    }
}

//---------------------------------------------------------------------------------------

void r4300i_ldl(uint32 Instruction) {   DisplayError("LDL opcode not implemented"); }
void r4300i_ldr(uint32 Instruction) {   DisplayError("LDR opcode not implemented"); }
void r4300i_sdl(uint32 Instruction) {   DisplayError("SDL opcode not implemented"); }
void r4300i_sdr(uint32 Instruction) {   DisplayError("SDR opcode not implemented"); }

//---------------------------------------------------------------------------------------

extern void InitDynarec(void);

extern t_rominfo rominfo;
void r4300i_Init()
{
    //set all registers to 0
    memset( gHardwareState.COP0Reg, 0,  sizeof(gHardwareState.COP0Reg));
    memset( gHardwareState.COP1Reg, 0,  sizeof(gHardwareState.COP1Reg));
    memset( gHardwareState.COP1Con, 0,  sizeof(gHardwareState.COP1Con));

    gHardwareState.GPR[gHI] = 0;
    gHardwareState.GPR[gLO] = 0;
    gHardwareState.LLbit = 0;


		gHardwareState.GPR[0x00] = 0;
		gHardwareState.GPR[0x01] = 0;
		gHardwareState.GPR[0x02] = 0xffffffffd1731be9;
		gHardwareState.GPR[0x03] = 0xffffffffd1731be9;
		gHardwareState.GPR[0x04] = 0x01be9;
		gHardwareState.GPR[0x05] = 0xfffffffff45231e5;
		gHardwareState.GPR[0x06] = 0xffffffffa4001f0c;
		gHardwareState.GPR[0x07] = 0xffffffffa4001f08;
		gHardwareState.GPR[0x08] = 0x070;
		gHardwareState.GPR[0x09] = 0;
		gHardwareState.GPR[0x0a] = 0x040;
		gHardwareState.GPR[0x0b] = 0xffffffffa4000040;
		gHardwareState.GPR[0x0c] = 0xffffffffd1330bc3;
		gHardwareState.GPR[0x0d] = 0xffffffffd1330bc3;
		gHardwareState.GPR[0x0e] = 0x025613a26;
		gHardwareState.GPR[0x0f] = 0x02ea04317;
		gHardwareState.GPR[0x10] = 0;
		gHardwareState.GPR[0x11] = 0;
		gHardwareState.GPR[0x12] = 0;
		gHardwareState.GPR[0x13] = 0;
		gHardwareState.GPR[0x14] = rominfo.TV_System;
		gHardwareState.GPR[0x15] = 0;
		gHardwareState.GPR[0x16] = rominfo.CIC;
		gHardwareState.GPR[0x17] = 0x06;
		gHardwareState.GPR[0x18] = 0;
		gHardwareState.GPR[0x19] = 0xffffffffd73f2993;
		gHardwareState.GPR[0x1a] = 0;
		gHardwareState.GPR[0x1b] = 0;
		gHardwareState.GPR[0x1c] = 0;
		gHardwareState.GPR[0x1d] = 0xffffffffa4001ff0;
		gHardwareState.GPR[0x1e] = 0;
		gHardwareState.GPR[0x1f] = 0xffffffffa4001554;

        
    gHardwareState.COP0Reg[STATUS] = 0x70400004;
	gHardwareState.COP0Reg[RANDOM] = 0x0000001f;
	//gHardwareState.COP0Reg[RANDOM] = 0x0000002f;
	gHardwareState.COP0Reg[CONFIG] = 0x0006e463;  // zils boot doc ...
	gHardwareState.COP0Reg[PREVID]   = 0x00000b00;
	gHardwareState.COP1Con[0]      = 0x00000511;
    
	CPUdelayPC = 0;
	CPUdelay = 0;
	gHardwareState.pc = 0xA4000040;

    MI_VERSION_REG_R = 0x01010101;
}

//---------------------------------------------------------------------------------------


void r4300i_Reset()
{
	/*
    memset(RDRAM,   0, sizeof(RDRAM)  );
    memset(RDREG,   0, sizeof(RDREG)  );
    memset(SP_REG,  0, sizeof(SP_REG) );
    memset(DPC,     0, sizeof(DPC)    );
    memset(DPS,     0, sizeof(DPS)    );
    memset(MI,      0, sizeof(MI)     );
    memset(VI,      0, sizeof(VI)     );
    memset(AI,      0, sizeof(AI)     );
    memset(PI,      0, sizeof(PI)     );
    memset(RI,      0, sizeof(RI)     );
    memset(SI,      0, sizeof(SI)     );
    memset(C2A1,    0, sizeof(C2A1)   );
    memset(C1A1,    0, sizeof(C1A1)   );
    memset(C2A2,    0, sizeof(C2A2)   );
    memset(GIO_REG, 0, sizeof(GIO_REG));
	*/
    memset(PIF,     0, sizeof(PIF)    );
    r4300i_Init();

    CPUdelayPC = 0;
	CPUdelay = 0;
//	InitDynarec();
	InitTLB();
	ClearCPUTasks();
	RCP_Reset();

    /* Copy boot code to SP_DMEM */
    memcpy((uint8*)&SP_DMEM, ROM_Image, 0x1000);
    gHardwareState.pc = 0xA4000040;
}

//---------------------------------------------------------------------------------------




void Check_LW(uint32 QuerAddr, uint32 rt_ft)
{
	static int VLine;
	
	// a stupid hack to use in Dyna mode
	//if( (QuerAddr & 0x1FFF0000) == 0x04300000 && (QuerAddr &0x1FFFFFFF) > 0x04300020) 
	//	QuerAddr = QuerAddr - 0x128;

    switch (QuerAddr)
    {
	/* AI_LEN_REG       */		
	case 0xA4500004: 
		AI_LEN_REG = AUDIO_AiReadLength();   
		break;

	/* SP_SEMAPHORE_REG */      
	case 0xA404001C: 
		gHardwareState.GPR[rt_ft] = SP_SEMAPHORE_REG; 
		SP_SEMAPHORE_REG = 1;    
		break;

	/* VI_CURRENT_REG   */      
	case 0xA4400010:
#ifdef SAVEVICOUNTER
		VI_CURRENT_REG = ( VIcounter / VI_COUNTER_INC_PER_LINE + VI_INTR_REG ) % max_vi_lines;
#else
		VI_CURRENT_REG = (VI_CURRENT_REG + 1 ) % max_vi_lines;
#endif
		gHardwareState.GPR[rt_ft] = VI_CURRENT_REG;
        break;

	default: 
		__try
		{
            gHardwareState.GPR[rt_ft] = *(_int32*)(uint8*)(valloc2+(QuerAddr & 0x1fffffff));                               
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
#ifdef DEBUG_TLB
		   if( debug_tlb )
		   {
				uint32 Instruction = FetchInstruction();
				uint32 virtualaddress = (uint32)((_int32)gBASE + (_int32)OFFSET_IMMEDIATE);
				sprintf(generalmessage, "%08X: LW to access VA=%08X, PA=%08X, Out of range. ", gHardwareState.pc, virtualaddress, QuerAddr);
				RefreshOpList(generalmessage); 
		   }
#endif
			gHardwareState.COP0Reg[CAUSE] |= EXC_RADE;	// Trigger Address Error Exception
#ifndef CPUCHECKINTR
			CPUNeedToDoOtherTask = TRUE;
			CPUNeedToCheckException = TRUE;
#endif
			gHardwareState.COP0Reg[BADVADDR] = QuerAddr;

			// Use dummy segment
			gHardwareState.GPR[rt_ft] = *(uint32*)(dummySegment+(QuerAddr&0x00007fff));

        }
		break;
    }
#ifdef DEBUG_IO
	DebugIO(QuerAddr,"Read", (uint32)gHardwareState.GPR[rt_ft]);
#endif

}    


//---------------------------------------------------------------------------------------
int max_vi_lines = DEFAULT_MAX_VI_LINE;
uint32 RTVal;
uint32 SW_QuerAddr;

void Check_SW(uint32 QuerAddr, uint32 rt_ft) 
{
    RTVal = (uint32)gHardwareState.GPR[rt_ft];


#ifdef DEBUG_IO
	DebugIO(QuerAddr,"Write", RTVal);
#endif

    switch (QuerAddr) 
    {
/* MI_MODE_REG_ADDR      */ case 0xA4300000: 
								WriteMI_ModeReg(RTVal)     ;   
								break;  
/* MI_INTR_MASK_REG      */ case 0xA430000C: 
								Handle_MI(RTVal)           ;   
								break;
/* VI_CURRENT_REG        */ case 0xA4400010: 
								// Clear VI interrupt
								MI_INTR_REG_R &= NOT_MI_INTR_VI;   
								if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0)
								{
									gHardwareState.COP0Reg[CAUSE] &= 0xFFFFFBFF;
#ifdef CPUCHECKINTR
									CPUNeedToCheckInterrupt = FALSE;
#endif
								}

								//VI_CURRENT_REG = RTVal;
								//VIcounter = RTVal * VI_COUNTER_INC_PER_LINE;
								break;

								
/* VI_V_SYNC_REG */			case 0xA4400018:
								VI_V_SYNC_REG = RTVal;
								if( RTVal != 0 )
									max_vi_lines = RTVal;
								else
									max_vi_lines = DEFAULT_MAX_VI_LINE;	// default VI_V_SYNC value
								break;

/* VI_ORIGIN_REG */			case 0xA4400004:
								if( VI_ORIGIN_REG != RTVal )
								{
									VI_ORIGIN_REG = RTVal;
									VIDEO_UpdateScreen();
								}
								break;
								

/* SP_STATUS_REG         */ case 0xA4040010:
								MI_INTR_REG_R &= NOT_MI_INTR_SP;   
								if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0)
								{
									gHardwareState.COP0Reg[CAUSE] &= 0xFFFFFBFF;
#ifdef CPUCHECKINTR
									CPUNeedToCheckInterrupt = FALSE;
#endif
								}
								Handle_SP(RTVal);
								break;
/* SI_STATUS_REG         */ case 0xA4800018: 
								MI_INTR_REG_R &= NOT_MI_INTR_SI;   
								if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
								{
									gHardwareState.COP0Reg[CAUSE] &= 0xFFFFFBFF;
#ifdef CPUCHECKINTR
									CPUNeedToCheckInterrupt = FALSE;
#endif
								}
								SI_STATUS_REG &= ~SI_STATUS_INTERRUPT;	// Clear the interrupt bit
								break;
								//break;
/* AI_STATUS_REG         */ case 0xA450000C: 
								MI_INTR_REG_R &= NOT_MI_INTR_AI;   
								if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
								{
									gHardwareState.COP0Reg[CAUSE] &= 0xFFFFFBFF; 
#ifdef CPUCHECKINTR
									CPUNeedToCheckInterrupt = FALSE;
#endif
								}
								break;

/* AI_DACRATE_REG		 */ case 0xA4500010:
//        			            QuerAddr = (QuerAddr & 0x000fffff) | 0x1fb00000;
                                AI_DACRATE_REG = RTVal;
                                //LOAD_UWORD_PARAM(QuerAddr) = 48681812 / 22050       ;//RTVal;
								AUDIO_AiDacrateChanged(0);

								break;

/* PI_STATUS_REG         */ case 0xA4600010: 
								if( RTVal & PI_STATUS_CLR_INTR )
								{
									MI_INTR_REG_R &= NOT_MI_INTR_PI;   
									if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
									{
										gHardwareState.COP0Reg[CAUSE] &= 0xFFFFFBFF; 
#ifdef CPUCHECKINTR
										CPUNeedToCheckInterrupt = FALSE;
#endif
									}
								}
								else if( RTVal & PI_STATUS_RESET )
								{
									// Reset the PIC
									PI_STATUS_REG = 0;

									// Reset finished, set PI Interrupt
									MI_INTR_REG_R |= MI_INTR_PI;	// Set PI Interrupt
									gHardwareState.COP0Reg[CAUSE] |= 0x400;
#ifdef CPUCHECKINTR
									CPUNeedToCheckInterrupt = TRUE;
									CPUNeedToDoOtherTask   = TRUE;
#endif
								}

								break;		// Does not actually write into the PI_STATUS_REG
								//break;
/* DPC_STATUS_REG        */ case 0xA410000C: 
								MI_INTR_REG_R &= NOT_MI_INTR_DP;   
								if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
								{
									gHardwareState.COP0Reg[CAUSE] &= 0xFFFFFBFF; 
#ifdef CPUCHECKINTR
									CPUNeedToCheckInterrupt = FALSE;
#endif
								}
								Handle_DPC(RTVal);
								break;
/* SP_SEMAPHORE_REG      */ case 0xA404001C: 
								SP_SEMAPHORE_REG       = 0 ;   
								break;
/* PI_WR_LEN_REG         */ case 0xA460000C: 
								PI_WR_LEN_REG          = RTVal; 
								DMA_MemCopy_DRAM_To_PI() ;   	
								break;
/* PI_RD_LEN_REG         */ case 0xA4600008: 
								PI_RD_LEN_REG          = RTVal; 
								DMA_MemCopy_PI_To_DRAM() ;        
								break;
/* SP_WR_LEN_REG         */ case 0xA404000C: 
								SP_WR_LEN_REG          = RTVal; 
								DMA_MemCopy_SP_to_DRAM() ;       
								break;
/* SP_RD_LEN_REG         */ case 0xA4040008: 
								SP_RD_LEN_REG          = RTVal; 
								DMA_MemCopy_DRAM_To_SP()  ;       
								break;
/* SI_PIF_ADDR_WR64B_REG */ case 0xA4800010: 
								SI_PIF_ADDR_WR64B_REG  = RTVal; 
								DMA_MemCopy_DRAM_to_SI() ;       
								break;
/* SI_PIF_ADDR_RD64B_REG */ case 0xA4800004: 
								SI_PIF_ADDR_RD64B_REG  = RTVal; 
								DMA_MemCopy_SI_To_DRAM()  ;       
								break;
/* VI_WIDTH_REG             case 0xA4400008: VI_WIDTH_REG           = RTVal; ResizeVideoWindow() ;  break;*/
/* AI_LEN_REG            */ case 0xA4500004: 
								DMA_AI();
                                AI_LEN_REG             = RTVal;
                                AUDIO_AiLenChanged();
								AUDIO_AiUpdate(FALSE);
								// (AUDIO_PlaySnd)(RDRAM,AI);

#ifdef DEBUG_AUDIO
								if(debug_audio)
								{
									sprintf(generalmessage, "%08X: Play %d bytes of audio at %08X", gHardwareState.pc, AI_LEN_REG, AI_DRAM_ADDR_REG);
									RefreshOpList(generalmessage); 
								}
#endif
        break;          
        
    default: 
        __try {
            *(uint32*)(uint8*)(valloc2+(QuerAddr & 0x1fffffff)) = RTVal;
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
#ifdef DEBUG_TLB
			if(debug_tlb)
			{
				uint32 Instruction = FetchInstruction();
				uint32 virtualaddress = (uint32)((_int32)gBASE + (_int32)OFFSET_IMMEDIATE);
				//DisplayError("%08X: SW to access VA=%08X, PA=%08X, Out of range. ", gHardwareState.pc, virtualaddress, QuerAddr);
				sprintf(generalmessage, "%08X: SW to access VA=%08X, PA=%08X, Out of range. ", gHardwareState.pc, virtualaddress, QuerAddr);
				RefreshOpList(generalmessage); 
			}
#endif
			gHardwareState.COP0Reg[CAUSE] |= EXC_WADE;	// Trigger Address Error Exception
#ifndef CPUCHECKINTR
			CPUNeedToDoOtherTask = TRUE;
			CPUNeedToCheckException = TRUE;
#endif
			gHardwareState.COP0Reg[BADVADDR] = QuerAddr;

			// Use dummy segment
			*(uint32*)(dummySegment+(QuerAddr&0x00007fff)) = RTVal;
	
        }
        break;
                       
    }
}

// This function will display error message and set exception when address error happens when SW
void SW_AddrError(uint32 QuerAddr, uint32 val)
{
#ifdef DEBUG_TLB
	if(debug_tlb)
	{
		uint32 Instruction = FetchInstruction();
		uint32 virtualaddress = (uint32)((_int32)gBASE + (_int32)OFFSET_IMMEDIATE);
		sprintf(generalmessage, "%08X: SW address error VA=%08X, PA=%08X, Out of range. ", gHardwareState.pc, virtualaddress, QuerAddr);
		RefreshOpList(generalmessage); 
	}
#endif
	gHardwareState.COP0Reg[CAUSE] |= EXC_WADE;	// Trigger Address Error Exception
#ifndef CPUCHECKINTR
	CPUNeedToDoOtherTask = TRUE;
	CPUNeedToCheckException = TRUE;
#endif
	gHardwareState.COP0Reg[BADVADDR] = QuerAddr;

	// Use dummy segment
	*(uint32*)(dummySegment+(QuerAddr&0x00007fff)) = val;
}

static int regindex;


void rdram_reg_sw(void)
{
	__try 
	{
        *(uint32*)(uint8*)(valloc2+(SW_QuerAddr & 0x1fffffff)) = RTVal;
    }
	__except(NULL,EXCEPTION_EXECUTE_HANDLER)
	{
		SW_AddrError(SW_QuerAddr, RTVal);
	}
}

void sp_reg_sw(void)
{
	if( SW_QuerAddr < 0xA4002000 )
	{
        __try
		{
            *(uint32*)(uint8*)(valloc2+(SW_QuerAddr & 0x1fffffff)) = RTVal;
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
			SW_AddrError(SW_QuerAddr, RTVal);
        }
	}
	else
	{
	switch( SW_QuerAddr )
	{
	/* SP_STATUS_REG         */ 
	case 0xA4040010:
		MI_INTR_REG_R &= NOT_MI_INTR_SP;   
		if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0)
		{
			gHardwareState.COP0Reg[CAUSE] &= 0xFFFFFBFF;
#ifdef CPUCHECKINTR
			CPUNeedToCheckInterrupt = FALSE;
#endif
		}
		Handle_SP(RTVal);
		break;
	
	/* SP_SEMAPHORE_REG      */ 
	case 0xA404001C: 
		SP_SEMAPHORE_REG       = 0 ;   
		break;

	/* SP_WR_LEN_REG         */ 
	case 0xA404000C: 
		SP_WR_LEN_REG          = RTVal; 
		DMA_MemCopy_SP_to_DRAM() ;       
		break;

	/* SP_RD_LEN_REG         */ 
	case 0xA4040008: 
		SP_RD_LEN_REG          = RTVal; 
		DMA_MemCopy_DRAM_To_SP()  ;       
		break;
	default:
        __try
		{
            *(uint32*)(uint8*)(valloc2+(SW_QuerAddr & 0x1fffffff)) = RTVal;
        }
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
			SW_AddrError(SW_QuerAddr, RTVal);
        }
		break;
	}
	}
}

void dp_reg_sw(void)
{
	/* DPC_STATUS_REG        */ 
	switch( regindex )
	{
	case 3: 
		MI_INTR_REG_R &= NOT_MI_INTR_DP;   
		if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
		{
			gHardwareState.COP0Reg[CAUSE] &= 0xFFFFFBFF; 
#ifdef CPUCHECKINTR
			CPUNeedToCheckInterrupt = FALSE;
#endif
		}
		Handle_DPC(RTVal);
		break;

	default:
		if( regindex < NUMBEROFDPREG )
			DPC[regindex] = RTVal;
		else
			DisplayError("Error DPC Register write, index = %d", regindex);
		break;
	}
}

void dpspan_rag_sw(void)
{
	if( regindex < NUMBEROFDPSREG )
		DPS[regindex] = RTVal;
	else
		DisplayError("Error DPS Register write, index = %d", regindex);
}

void mi_reg_sw(void)
{
	switch( regindex )
	{
	/* MI_MODE_REG_ADDR      */ 
	case 0: 
		WriteMI_ModeReg(RTVal)     ;   
		break;  

	/* MI_INTR_MASK_REG      */ 
	case 3: 
		Handle_MI(RTVal)           ;   
		break;

	default:
		if( regindex < NUMBEROFMIREG )
			MI[regindex] = RTVal;
		else
			DisplayError("Error MI Register write, index = %d", regindex);
		break;
	}
}

void vi_reg_sw(void)
{
	switch( regindex )
	{
	/* VI_CURRENT_REG        */ 
	case 4: 
		// Clear VI interrupt
		MI_INTR_REG_R &= NOT_MI_INTR_VI;   
		if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0)
		{
			gHardwareState.COP0Reg[CAUSE] &= 0xFFFFFBFF;
#ifdef CPUCHECKINTR
			CPUNeedToCheckInterrupt = FALSE;
#endif
		}

		//VI_CURRENT_REG = RTVal;
		//VIcounter = RTVal * VI_COUNTER_INC_PER_LINE;
		break;

								
	/* VI_V_SYNC_REG */			
	case 6:
		VI_V_SYNC_REG = RTVal;
		if( RTVal != 0 )
			max_vi_lines = RTVal;
		else
			max_vi_lines = DEFAULT_MAX_VI_LINE;	// default VI_V_SYNC value
		break;

	/* VI_ORIGIN_REG */			
	case 1:
		if( VI_ORIGIN_REG != RTVal )
		{
			VI_ORIGIN_REG = RTVal;
			VIDEO_UpdateScreen();
		}
		break;

	/* VI_WIDTH_REG             
	case 2: VI_WIDTH_REG           = RTVal; ResizeVideoWindow() ;  break;*/

	default:
		if( regindex < NUMBEROFVIREG )
			VI[regindex] = RTVal;
		else
			DisplayError("Error VI Register write, index = %d", regindex);
		break;
	}
}

void ai_reg_sw(void)
{
	switch( regindex )
	{
	/* AI_STATUS_REG         */ 
	case 3: 
		MI_INTR_REG_R &= NOT_MI_INTR_AI;   
		if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
		{
			gHardwareState.COP0Reg[CAUSE] &= 0xFFFFFBFF; 
#ifdef CPUCHECKINTR
			CPUNeedToCheckInterrupt = FALSE;
#endif
		}
		break;

	/* AI_DACRATE_REG		 */ 
	case 4:
//      QuerAddr = (QuerAddr & 0x000fffff) | 0x1fb00000;
        AI_DACRATE_REG = RTVal;
        //LOAD_UWORD_PARAM(QuerAddr) = 48681812 / 22050       ;//RTVal;
		AUDIO_AiDacrateChanged(0);

		break;

	/* AI_LEN_REG            */ 
	case 1: 
		DMA_AI();
        AI_LEN_REG             = RTVal;
        AUDIO_AiLenChanged();
		AUDIO_AiUpdate(FALSE);
		// (AUDIO_PlaySnd)(RDRAM,AI);

#ifdef DEBUG_AUDIO
		if(debug_audio)
		{
			sprintf(generalmessage, "%08X: Play %d bytes of audio at %08X", gHardwareState.pc, AI_LEN_REG, AI_DRAM_ADDR_REG);
			RefreshOpList(generalmessage); 
		}
#endif
        break;          
        
	default:
		if( regindex < NUMBEROFAIREG )
			AI[regindex] = RTVal;
		else
			DisplayError("Error AI Register write, index = %d", regindex);
		break;
	}
}

void pi_reg_sw(void)
{
	switch( regindex )
	{
	/* PI_STATUS_REG         */ 
	case 4: 
		if( RTVal & PI_STATUS_CLR_INTR )
		{
			MI_INTR_REG_R &= NOT_MI_INTR_PI;   
			if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
			{
				gHardwareState.COP0Reg[CAUSE] &= 0xFFFFFBFF; 
#ifdef CPUCHECKINTR
				CPUNeedToCheckInterrupt = FALSE;
#endif
			}
		}
		else if( RTVal & PI_STATUS_RESET )
		{
			// Reset the PIC
			PI_STATUS_REG = 0;

			// Reset finished, set PI Interrupt
			MI_INTR_REG_R |= MI_INTR_PI;	// Set PI Interrupt
			gHardwareState.COP0Reg[CAUSE] |= 0x400;
#ifdef CPUCHECKINTR
			CPUNeedToCheckInterrupt = TRUE;
			CPUNeedToDoOtherTask   = TRUE;
#endif
		}

		break;		// Does not actually write into the PI_STATUS_REG

	/* PI_WR_LEN_REG         */ 
	case 3: 
		PI_WR_LEN_REG          = RTVal; 
		DMA_MemCopy_DRAM_To_PI() ;   	
		break;

	/* PI_RD_LEN_REG         */ 
	case 2: 
		PI_RD_LEN_REG          = RTVal; 
		DMA_MemCopy_PI_To_DRAM() ;        
		break;

	default:
		if( regindex < NUMBEROFPIREG )
			PI[regindex] = RTVal;
		else
			DisplayError("Error PI Register write, index = %d", regindex);
		break;
	}
}

void si_reg_sw(void)
{
	switch( regindex )
	{
	/* SI_STATUS_REG         */ 
	case 6: 
		MI_INTR_REG_R &= NOT_MI_INTR_SI;   
		if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) 
		{
			gHardwareState.COP0Reg[CAUSE] &= 0xFFFFFBFF;
#ifdef CPUCHECKINTR
			CPUNeedToCheckInterrupt = FALSE;
#endif
		}
		SI_STATUS_REG &= ~SI_STATUS_INTERRUPT;	// Clear the interrupt bit
		break;

	/* SI_PIF_ADDR_WR64B_REG */ 
	case 4: 
		SI_PIF_ADDR_WR64B_REG  = RTVal; 
		DMA_MemCopy_DRAM_to_SI() ;       
		break;
	
	/* SI_PIF_ADDR_RD64B_REG */ 
	case 1: 
		SI_PIF_ADDR_RD64B_REG  = RTVal; 
		DMA_MemCopy_SI_To_DRAM()  ;       
		break;

	default:
		if( regindex < NUMBEROFSIREG )
			SI[regindex] = RTVal;
		else
			DisplayError("Error SI Register write, index = %d", regindex);
		break;
	}
}

void ri_reg_sw(void)
{
	switch( regindex )
	{
		default:
		if( regindex < NUMBEROFRIREG )
			RI[regindex] = RTVal;
		else
			DisplayError("Error RI Register write, index = %d", regindex);
		break;
	}

}

void (*IO_SW[10])(void)=
{
	rdram_reg_sw,	sp_reg_sw,		dp_reg_sw,		dpspan_rag_sw,	mi_reg_sw,	
	vi_reg_sw,		ai_reg_sw,		pi_reg_sw,		ri_reg_sw,		si_reg_sw
};

void Faster_Check_SW(uint32 QuerAddr, uint32 rt_ft)
{
	//int i;
	RTVal = (uint32)gHardwareState.GPR[rt_ft];
	SW_QuerAddr = QuerAddr;
	regindex=(SW_QuerAddr&0xFF)/4;

	//i = ((SW_QuerAddr&0x1FFFFFFF)>>20) - 0x3F;
	IO_SW[((SW_QuerAddr&0x1FFFFFFF)>>20) - 0x3F]();
	//IO_SW[i]();
}