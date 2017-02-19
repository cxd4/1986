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

#include <windows.h>
#include "globals.h"
#include "plugins.h"

uint8* sDWORD_R[0xFFFF];
uint8* sDYN_PC_LOOKUP[0xFFFF];

uint32 RESET_CPU;

unsigned long gROMLength; //size in bytes of the ROM

int DebuggerEnabled; /* Flag to toggle debug printing on/off */


uint8 ND, TF;            /* these are 1-bit fields for bc1 ops				*/
uint32 pc;               /* program counter. (Keeps addresses.) */


uint32* LocationJumpedFrom;
uint32* InstructionPointer;
uint32 KEEP_RECOMPILING;
uint8* Block;


uint32	CPUdelayPC;
uint32	CPUdelay;


char AppPath[PATH_LEN]; //used for storing application path

_int64 GPR[32];
uint32 COP0Reg[32];
uint32 COP1Reg[64]; /* FPU General Registers */
uint32 COP1Con[64]; /* FPU Control Registers */
_int64 HI;         /* MultHI reg */
_int64 LO;         /* MultLO reg */
uint32 LLbit;      /* LLbit for load link instructions */

uint32 FoundHLE[1]; //Flag for HLE detection...Disables cfb when found

tlb_struct		TLB[MAXTLB];

uint8 RDRAM[0x00400000];
uint8  PIF[2048];
uint32 RDREG[262144];
uint32 SP_REG[131074];
uint32 DPC[8];
uint32 DPS[4];
uint32 MI[4];
uint32 VI[14];
uint32 AI[6];
uint32 PI[13];
uint32 RI[8];
uint32 SI[7];
uint32 C2A1[512];
uint32 C1A1[512];
uint32 C2A2[512];
uint32 GIO_REG[513];
uint8* ROM_Image;

uint8 DynaRDRAM[0x00400000];
uint32 DynaSP_REG[131074];

uint32 gAllocationLength;

GFX_INFO Gfx_Info;