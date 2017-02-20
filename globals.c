/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Globals                                                              |
 |  Copyright (C) 2001 Joel Middendorf, <schibo@emuhq.com>                      |
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
*/

#include <windows.h>
#include "globals.h"
#include "r4300i.h"
#include "plugins.h"
#include "hardware.h"

char* CURRENT1964VERSION = "1964_052";

CONTROL Controls[4];


HardwareState gHardwareState;
HardwareState gHardwareState2;

uint8* sDWORD_R[0xFFFF];
uint8* sDYN_PC_LOOKUP[0xFFFF];		// This array is the Dynamic Compile Lookup table

uint32* LocationJumpedFrom;
uint32* pcptr;
uint32 KEEP_RECOMPILING;
uint8* Block;

uint32  CPUdelayPC;			// the saved Program Counter at CPU load/branch delay mode
uint32  CPUdelay;			// Describer if the CPU is in load/branch delay mode

int		whichcore;			// Which compiler to use in emulator

tlb_struct      TLB[MAXTLB];
tlb_struct		ITLB[MAXITLB];
tlb_struct		DTLB[MAXDTLB];


uint8* ROM_Image;

uint32 gAllocationLength;
uint8 HeaderDllPass[0x40];

t_EmuData EmuData;

_u8 EEprom[EEPROM_SIZE_4KB];		// 4KB
//_u8 EEprom[EEPROM_SIZE];		// 2KB
int eepromsize= EEPROM_SIZE;

_u8 SRam[SRAM_SIZE];			//0x8000 Bytes
_u8 FlashRAM[FLASHRAM_SIZE];	//0x20000 Bytes


GFX_INFO Gfx_Info;
AUDIO_INFO Audio_Info;
int DebuggerEnabled;			// Flag to toggle debug printing on/off
BOOL Rom_Loaded;

t_rominfo rominfo;				// Rom information
uint8 *dyna_CodeTable;

char  generalmessage[256];				// general purpose buffer to display messages
