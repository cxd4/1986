/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Macintosh Global Variables                                           |
 |  Copyright (C) 2000 Gerrit Goossen, <gerrit@aol.com>                         |
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
 |  email      : gerrit@aol.com                                                 |
 |  paper mail :                                                                |
 |______________________________________________________________________________|
*/


#ifndef _GLOBALS_H__1964_
#define _GLOBALS_H__1964_

#define uint64 unsigned __int64
#define uint32 unsigned __int32
#define uint16 unsigned __int16
#define uint8  unsigned __int8

extern void __cdecl DisplayError (char * Message, ...);

// little-endian memory operations
#define QUER_ADDR QuerAddr = (uint32)((_int32)GPR[RS_BASE_FMT] + (_int32)OFFSET_IMMEDIATE)
#define DISPLACE(X)	(((uint16)QuerAddr) >> X )

#define LOAD_SBYTE_PARAM(param)		*         ((_int8*)sDWORD_R[((uint16)((param) >> 16))] + ((uint16)param ^ 3))
#define LOAD_UBYTE_PARAM(param)		*         ((uint8*)sDWORD_R[((uint16)((param) >> 16))] + ((uint16)param ^ 3))
#define LOAD_SHALF_PARAM(param)		*(_int16*)((uint8*)sDWORD_R[((uint16)((param) >> 16))] + ((uint16)param ^ 2))
#define LOAD_UHALF_PARAM(param)		*(uint16*)((uint8*)sDWORD_R[((uint16)((param) >> 16))] + ((uint16)param ^ 2))
#define LOAD_SWORD_PARAM(param)		*(_int32*)((uint8*)sDWORD_R[((uint16)((param) >> 16))] + ((uint16)param    ))
#define LOAD_UWORD_PARAM(param)		*(uint32*)((uint8*)sDWORD_R[((uint16)((param) >> 16))] + ((uint16)param    ))

#define BAD_TASK	0
#define GFX_TASK	1
#define SND_TASK	2

#define MAGICNUMBER 625000

extern uint8* sDWORD_R[0xFFFF];
extern uint8* sDYN_PC_LOOKUP[0xFFFF];

extern uint32 gAllocationLength;
extern unsigned long gROMLength; //size in bytes of the ROM
extern uint32 RESET_CPU;
extern int DebuggerEnabled; /* Flag to toggle debug printing on/off */
extern uint8 ND, TF;            /* these are 1-bit fields for bc1 ops                */
extern uint32 pc;               /* program counter. (Keeps addresses.) */
extern uint32    CPUdelayPC;
extern uint32    CPUdelay;
#define PATH_LEN 80     //max characters for path
extern char AppPath[PATH_LEN]; //used for storing application path
extern uint32 FoundHLE[1]; //Flag for HLE detection...Disables cfb when found


//dynarec globals
extern uint32* LocationJumpedFrom;
extern uint32* InstructionPointer;
extern uint32 KEEP_RECOMPILING;
extern uint8* Block;
#define PIF_RAM_PHYS 0x7C0


#ifdef WINDEBUG_1964

#define DYNDEBUG_UPDATE					\
	SetVariable(Dest, &pc, pc)			\
	PUTDST8KNOWN(Dest, 0xB9)			\
	PUTDST32(Dest, Instruction)			\
	C_CALL(Dest,(uint32)&WinDynDebugPrintInstruction)

#define DEBUG_BPT					\
	PUTDST8KNOWN(Dest, 0xB9)		\
	PUTDST32(Dest, Instruction)		\
	LOADIMM32(&HandleBreakpoint)	\
	PUTDST16KNOWN(Dest,0xd0ff)

#else

#define DYNDEBUG_UPDATE	
#define DEBUG_BPT

#endif


#define FetchOpcode						\
	Instruction = *InstructionPointer;	\
	DYNDEBUG_UPDATE						\
	DEBUG_BPT							\
	Dest = dyn_CPU_instruction[_OPCODE_](PASS_PARAMS);


#define RegFetchOpcode()				\
	Instruction = *InstructionPointer;	\
	DYNDEBUG_UPDATE						\
	DEBUG_BPT							\
	rc_CPU_instruction[_OPCODE_]();


extern _int64 HI;
extern _int64 LO;
extern uint32 LLbit;
extern _int64 GPR[32];
extern uint32 COP0Reg[32];
extern uint32 COP1Reg[64];
extern uint32 COP1Con[64];

#define MAXTLB    32

typedef struct
{
	uint32	valid;
	uint32	EntryHi;
	uint32	EntryLo1;
	uint32	EntryLo0;
	uint64	PageMask;
	uint32	LoCompare;
	uint32	MyHiMask;
} tlb_struct;

extern tlb_struct        TLB[MAXTLB];

extern uint8  RDRAM[0x00400000];
extern uint32 RDREG[262144];
extern uint32 SP_REG[131074];
extern uint32 DPC[8];
extern uint32 DPS[4];
extern uint32 MI[4];
extern uint32 VI[14];
extern uint32 AI[6];
extern uint32 PI[13];
extern uint32 RI[8];
extern uint32 SI[7];
extern uint32 C2A1[512];
extern uint32 C1A1[512];
extern uint32 C2A2[512];
extern uint32 GIO_REG[513];
extern uint8  PIF[2048];
extern uint8* ROM_Image;

extern uint8  DynaRDRAM[0x00400000];
extern uint32 DynaSP_REG[131074];


/*
****************************************************************************
** Registers                                                              **
****************************************************************************
  Main CPU registers:
  -------------------
    00h = r0/reg0     08h = t0/reg8     10h = s0/reg16    18h = t8/reg24
    01h = at/reg1     09h = t1/reg9     11h = s1/reg17    19h = t9/reg25
    02h = v0/reg2     0Ah = t2/reg10    12h = s2/reg18    1Ah = k0/reg26
    03h = v1/reg3     0Bh = t3/reg11    13h = s3/reg19    1Bh = k1/reg27
    04h = a0/reg4     0Ch = t4/reg12    14h = s4/reg20    1Ch = gp/reg28
    05h = a1/reg5     0Dh = t5/reg13    15h = s5/reg21    1Dh = sp/reg29
    06h = a2/reg6     0Eh = t6/reg14    16h = s6/reg22    1Eh = s8/reg30
    07h = a3/reg7     0Fh = t7/reg15    17h = s7/reg23    1Fh = ra/reg31
*/
  
#define R0 0x00000000
#define AT 0x00000001
#define V0 0x00000002
#define V1 0x00000003
#define A0 0x00000004
#define A1 0x00000005
#define A2 0x00000006
#define A3 0x00000007
#define T0 0x00000008
#define T1 0x00000009
#define T2 0x0000000A
#define T3 0x0000000B
#define T4 0x0000000C
#define T5 0x0000000D
#define T6 0x0000000E
#define T7 0x0000000F
#define S0 0x00000010
#define S1 0x00000011
#define S2 0x00000012
#define S3 0x00000013
#define S4 0x00000014
#define S5 0x00000015
#define S6 0x00000016
#define S7 0x00000017
#define T8 0x00000018
#define T9 0x00000019
#define K0 0x0000001A
#define K1 0x0000001B
#define GP 0x0000001C
#define SP 0x0000001D
#define S8 0x0000001E
#define RA 0x0000001F

/*
  COP0 registers:
  ---------------
    00h = Index       08h = BadVAddr    10h = Config      18h = *RESERVED* 
    01h = Random      09h = Count       11h = LLAddr      19h = *RESERVED*
    02h = EntryLo0    0Ah = EntryHi     12h = WatchLo     1Ah = PErr
    03h = EntryLo1    0Bh = Compare     13h = WatchHi     1Bh = CacheErr
    04h = Context     0Ch = Status      14h = XContext    1Ch = TagLo
    05h = PageMask    0Dh = Cause       15h = *RESERVED*  1Dh = TagHi
    06h = Wired       0Eh = EPC         16h = *RESERVED*  1Eh = ErrorEPC
    07h = *RESERVED*  0Fh = PRevID      17h = *RESERVED*  1Fh = *RESERVED*
*/
#define INDEX     0x00000000
#define RANDOM    0x00000001
#define ENTRYLO0  0x00000002
#define ENTRYLO1  0x00000003
#define CONTEXT   0x00000004
#define PAGEMASK  0x00000005
#define WIRED     0x00000006
#define RESERVED0 0x00000007
#define BADVADDR  0x00000008
#define COUNT     0x00000009
#define ENTRYHI   0x0000000A
#define COMPARE   0x0000000B
#define STATUS    0x0000000C
#define CAUSE     0x0000000D
#define EPC       0x0000000E
#define PREVID    0x0000000F
#define CONFIG    0x00000010
#define LLADDR    0x00000011
#define WATCHLO   0x00000012
#define WATCHHI   0x00000013
#define XCONTEXT  0x00000014
#define RESERVED1 0x00000015
#define RESERVED2 0x00000016
#define RESERVED3 0x00000017
#define RESERVED4 0x00000018
#define RESERVED5 0x00000019
#define PERR      0x0000001A
#define CACHEERR  0x0000001B
#define TAGLO     0x0000001C
#define TAGHI     0x0000001D
#define ERROREPC  0x0000001E
#define RESERVED6 0x0000001F

/*
0x03F0 0000 to 0x03FF FFFF  RDRAM registers:
 --------------------------------------------
        RDRAM_BASE_REG - 0x03F00000

        0x03F0 0000 to 0x03F0 0003  RDRAM_CONFIG_REG or RDRAM_DEVICE_TYPE_REG
        0x03F0 0004 to 0x03F0 0007  RDRAM_DEVICE_ID_REG
        0x03F0 0008 to 0x03F0 000B  RDRAM_DELAY_REG
        0x03F0 000C to 0x03F0 000F  RDRAM_MODE_REG
        0x03F0 0010 to 0x03F0 0013  RDRAM_REF_INTERVAL_REG
        0x03F0 0014 to 0x03F0 0017  RDRAM_REF_ROW_REG
        0x03F0 0018 to 0x03F0 001B  RDRAM_RAS_INTERVAL_REG
        0x03F0 001C to 0x03F0 001F  RDRAM_MIN_INTERVAL_REG
        0x03F0 0020 to 0x03F0 0023  RDRAM_ADDR_SELECT_REG
        0x03F0 0024 to 0x03F0 0027  RDRAM_DEVICE_MANUF_REG
        0x03F0 0028 to 0x03FF FFFF  Unknown
*/
#define RDRAM_CONFIG_REG            RDREG[0]
#define RDRAM_DEVICE_ID_REG         RDREG[1]
#define RDRAM_DELAY_REG             RDREG[2]
#define RDRAM_MODE_REG              RDREG[3]
#define RDRAM_REF_INTERVAL_REG      RDREG[4]
#define RDRAM_REF_ROW_REG           RDREG[5]
#define RDRAM_RAS_INTERVAL_REG      RDREG[6]
#define RDRAM_MIN_INTERVAL_REG      RDREG[7]
#define RDRAM_ADDR_SELECT_REG       RDREG[8]
#define RDRAM_DEVICE_MANUF_REG      RDREG[9]

/*
0x0400 0000 to 0x0400 FFFF  SP registers:
 -----------------------------------------
        SP_BASE_REG - 0x04040000

        0x0400 0000 to 0x0400 0FFF  SP_DMEM read/write
        0x0400 1000 to 0x0400 1FFF  SP_IMEM read/write
        0x0400 2000 to 0x0403 FFFF  Unused
        0x0404 0000 to 0x0404 0003  SP_MEM_ADDR_REG
        0x0404 0004 to 0x0404 0007  SP_DRAM_ADDR_REG
        0x0404 0008 to 0x0404 000B  SP_RD_LEN_REG
        0x0404 000C to 0x0404 000F  SP_WR_LEN_REG
        0x0404 0010 to 0x0404 0013  SP_STATUS_REG
        0x0404 0014 to 0x0404 0017  SP_DMA_FULL_REG
        0x0404 0018 to 0x0404 001B  SP_DMA_BUSY_REG
        0x0404 001C to 0x0404 001F  SP_SEMAPHORE_REG
        0x0404 0020 to 0x0407 FFFF  Unused
        0x0408 0000 to 0x0408 0003  SP_PC_REG
        0x0408 0004 to 0x0408 0007  SP_IBIST_REG
        0x0408 0008 to 0x040F FFFF  Unused
*/
#define SP_DMEM                (uint8*)&SP_REG[0]
#define SP_MEM_ADDR_REG        SP_REG[0x10000]
#define SP_DRAM_ADDR_REG       SP_REG[0x10001]
#define SP_RD_LEN_REG          SP_REG[0x10002]
#define SP_WR_LEN_REG          SP_REG[0x10003]
#define SP_STATUS_REG          SP_REG[0x10004]
#define SP_DMA_FULL_REG        SP_REG[0x10005]
#define SP_DMA_BUSY_REG        SP_REG[0x10006]
#define SP_SEMAPHORE_REG       SP_REG[0x10007]

#define SP_PC_REG              SP_REG[0x20000]
#define SP_IBIST_REG           SP_REG[0x20001]


#define HLE_DMEM_TASK          SP_REG[0x03F0]


/*
0x0410 0000 to 0x041F FFFF  DP command registers:
 -------------------------------------------------
        DPC_BASE_REG - 0x04100000

        0x0410 0000 to 0x0410 0003  DPC_START_REG
        0x0410 0004 to 0x0410 0007  DPC_END_REG
        0x0410 0008 to 0x0410 000B  DPC_CURRENT_REG
        0x0410 000C to 0x0410 000F  DPC_STATUS_REG
        0x0410 0010 to 0x0410 0013  DPC_CLOCK_REG
        0x0410 0014 to 0x0410 0017  DPC_BUFBUSY_REG
        0x0410 0018 to 0x0410 001B  DPC_PIPEBUSY_REG
        0x0410 001C to 0x0410 001F  DPC_TMEM_REG
        0x0410 0020 to 0x041F FFFF  Unused
*/
#define DPC_START_REG          DPC[0]
#define DPC_END_REG            DPC[1]
#define DPC_CURRENT_REG        DPC[2]
#define DPC_STATUS_REG         DPC[3]
#define DPC_CLOCK_REG          DPC[4]
#define DPC_BUFBUSY_REG        DPC[5]
#define DPC_PIPEBUSY_REG       DPC[6]
#define DPC_TMEM_REG           DPC[7]


/*
0x0420 0000 to 0x042F FFFF  DP span registers:
 ----------------------------------------------
        DPS_BASE_REG - 0x04200000

        0x0420 0000 to 0x0420 0003  DPS_TBIST_REG
        0x0420 0004 to 0x0420 0007  DPS_TEST_MODE_REG
        0x0420 0008 to 0x0420 000B  DPS_BUFTEST_ADDR_REG
        0x0420 000C to 0x0420 000F  DPS_BUFTEST_DATA_REG
        0x0420 0010 to 0x042F FFFF  Unused
*/
#define DPS_TBIST_REG                DPS[0]
#define DPS_TEST_MODE_REG            DPS[1]
#define DPS_BUFTEST_ADDR_REG         DPS[2]
#define DPS_BUFTEST_DATA_REG         DPS[3]


/*
0x0430 0000 to 0x043F FFFF  MIPS interface (MI) registers:
 ----------------------------------------------------------
        MI_BASE_REG - 0x04300000

        0x0430 0000 to 0x0430 0003  MI_INIT_MODE_REG or MI_MODE_REG
        0x0430 0004 to 0x0430 0007  MI_VERSION_REG or MI_NOOP_REG
        0x0430 0008 to 0x0430 000B  MI_INTR_REG
        0x0430 000C to 0x0430 000F  MI_INTR_MASK_REG
        0x0430 0010 to 0x043F FFFF  Unused
*/
#define MI_INIT_MODE_REG_R           MI[0]
#define MI_VERSION_REG_R             MI[1]
#define MI_INTR_REG_R                MI[2]
#define MI_INTR_MASK_REG_R           MI[3]

/*
0x0440 0000 to 0x044F FFFF  Video interface (VI) registers:
 -----------------------------------------------------------
        VI_BASE_REG - 0x04400000

        0x0440 0000 to 0x0440 0003  VI_STATUS_REG or VI_CONTROL_REG
        0x0440 0004 to 0x0440 0007  VI_ORIGIN_REG or VI_DRAM_ADDR_REG
        0x0440 0008 to 0x0440 000B  VI_WIDTH_REG or VI_H_WIDTH_REG
        0x0440 000C to 0x0440 000F  VI_INTR_REG or VI_V_INTR_REG
        0x0440 0010 to 0x0440 0013  VI_CURRENT_REG or VI_V_CURRENT_LINE_REG                                           
        0x0440 0014 to 0x0440 0017  VI_BURST_REG or VI_TIMING_REG
        0x0440 0018 to 0x0440 001B  VI_V_SYNC_REG
        0x0440 001C to 0x0440 001F  VI_H_SYNC_REG
        0x0440 0020 to 0x0440 0023  VI_LEAP_REG or VI_H_SYNC_LEAP_REG
        0x0440 0024 to 0x0440 0027  VI_H_START_REG or VI_H_VIDEO_REG
        0x0440 0028 to 0x0440 002B  VI_V_START_REG or VI_V_VIDEO_REG
        0x0440 002C to 0x0440 002F  VI_V_BURST_REG
        0x0440 0030 to 0x0440 0033  VI_X_SCALE_REG
        0x0440 0034 to 0x0440 0037  VI_Y_SCALE_REG
        0x0440 0038 to 0x044F FFFF  Unused
*/
#define VI_STATUS_REG   VI[0]
#define VI_ORIGIN_REG   VI[1]
#define VI_WIDTH_REG    VI[2]
#define VI_INTR_REG     VI[3]
#define VI_CURRENT_REG  VI[4]
#define VI_BURST_REG    VI[5]
#define VI_V_SYNC_REG   VI[6]
#define VI_H_SYNC_REG   VI[7]
#define VI_LEAP_REG     VI[8]
#define VI_H_START_REG  VI[9]
#define VI_V_START_REG  VI[10]
#define VI_V_BURST_REG  VI[11]
#define VI_X_SCALE_REG  VI[12]
#define VI_Y_SCALE_REG  VI[13]


/*
0x0450 0000 to 0x045F FFFF  Audio interface (AI) registers:
 -----------------------------------------------------------
        AI_BASE_REG - 0x04500000

        0x0450 0000 to 0x0450 0003  AI_DRAM_ADDR_REG
        0x0450 0004 to 0x0450 0007  AI_LEN_REG
        0x0450 0008 to 0x0450 000B  AI_CONTROL_REG
        0x0450 000C to 0x0450 000F  AI_STATUS_REG
        0x0450 0010 to 0x0450 0013  AI_DACRATE_REG
        0x0450 0014 to 0x0450 0017  AI_BITRATE_REG
        0x0450 0018 to 0x045F FFFF  Unused
*/

#define AI_DRAM_ADDR_REG   AI[0]
#define AI_LEN_REG         AI[1]
#define AI_CONTROL_REG     AI[2]
#define AI_STATUS_REG      AI[3]
#define AI_DACRATE_REG     AI[4]
#define AI_BITRATE_REG     AI[5]

/*
 0x0460 0000 to 0x046F FFFF  Peripheral interface (PI) registers:
 ----------------------------------------------------------------
        PI_BASE_REG - 0x04600000

        0x0460 0000 to 0x0460 0003  PI_DRAM_ADDR_REG
        0x0460 0004 to 0x0460 0007  PI_CART_ADDR_REG
        0x0460 0008 to 0x0460 000B  PI_RD_LEN_REG
        0x0460 000C to 0x0460 000F  PI_WR_LEN_REG
        0x0460 0010 to 0x0460 0013 PI_STATUS_REG
        0x0460 0014 to 0x0460 0017  PI_BSD_DOM1_LAT_REG or PI_DOMAIN1_REG
        0x0460 0018 to 0x0460 001B  PI_BSD_DOM1_PWD_REG
        0x0460 001C to 0x0460 001F  PI_BSD_DOM1_PGS_REG
        0x0460 0020 to 0x0460 0023  PI_BSD_DOM1_RLS_REG
        0x0460 0024 to 0x0460 0027  PI_BSD_DOM2_LAT_REG or PI_DOMAIN2_REG
        0x0460 0028 to 0x0460 002B  PI_BSD_DOM2_PWD_REG
        0x0460 002C to 0x0460 002F  PI_BSD_DOM2_PGS_REG
        0x0460 0030 to 0x0460 0033  PI_BSD_DOM2_RLS_REG
        0x0460 0034 to 0x046F FFFF  Unused
*/
#define PI_DRAM_ADDR_REG        PI[0]
#define PI_CART_ADDR_REG        PI[1]
#define PI_RD_LEN_REG           PI[2]
#define PI_WR_LEN_REG           PI[3]
#define PI_STATUS_REG           PI[4]
#define PI_BSD_DOM1_LAT_REG     PI[5]
#define PI_BSD_DOM1_PWD_REG     PI[6]
#define PI_BSD_DOM1_PGS_REG     PI[7]
#define PI_BSD_DOM1_RLS_REG     PI[8]
#define PI_BSD_DOM2_LAT_REG     PI[9]
#define PI_BSD_DOM2_PWD_REG     PI[10]
#define PI_BSD_DOM2_PGS_REG     PI[11]
#define PI_BSD_DOM2_RLS_REG     PI[12]


/*
0x0470 0000 to 0x047F FFFF  RDRAM interface (RI) registers:
 -----------------------------------------------------------
        RI_BASE_REG - 0x04700000

        0x0470 0000 to 0x0470 0003  RI_MODE_REG
        0x0470 0004 to 0x0470 0007  RI_CONFIG_REG
        0x0470 0008 to 0x0470 000B  RI_CURRENT_LOAD_REG
        0x0470 000C to 0x0470 000F  RI_SELECT_REG
        0x0470 0010 to 0x0470 0013  RI_REFRESH_REG or RI_COUNT_REG
        0x0470 0014 to 0x0470 0017  RI_LATENCY_REG
        0x0470 0018 to 0x0470 001B  RI_RERROR_REG
        0x0470 001C to 0x0470 001F  RI_WERROR_REG
        0x0470 0020 to 0x047F FFFF  Unused
*/
#define RI_MODE_REG         RI[0]
#define RI_CONFIG_REG       RI[1]
#define RI_CURRENT_LOAD_REG RI[2]
#define RI_SELECT_REG       RI[3]
#define RI_REFRESH_REG      RI[4]
#define RI_LATENCY_REG      RI[5]
#define RI_RERROR_REG       RI[6]
#define RI_WERROR_REG       RI[7]


/*
0x0480 0000 to 0x048F FFFF  Serial interface (SI) registers:
 ------------------------------------------------------------
        SI_BASE_REG - 0x04800000

        0x0480 0000 to 0x0480 0003  SI_DRAM_ADDR_REG
        0x0480 0004 to 0x0480 0007  SI_PIF_ADDR_RD64B_REG
        0x0480 0008 to 0x0480 000B  Reserved
        0x0480 000C to 0x0480 000F  Reserved
        0x0480 0010 to 0x0480 0013  SI_PIF_ADDR_WR64B_REG
        0x0480 0014 to 0x0480 0017  Reserved
        0x0480 0018 to 0x0480 001B  SI_STATUS_REG
        0x0480 001C to 0x048F FFFF  Unused

  Unused:
 -------
        0x0490 0000 to 0x04FF FFFF  Unused
*/
#define SI_DRAM_ADDR_REG         SI[0]
#define SI_PIF_ADDR_RD64B_REG    SI[1]
#define SI_PIF_ADDR_WR64B_REG    SI[4]
#define SI_STATUS_REG            SI[6]

#endif /* _GLOBALS_H__1964_ */