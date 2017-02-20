/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Globals                                                              |
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
 |  email      : schibo@emulation64.com                                         |
 |  paper mail :                                                                |
 |______________________________________________________________________________|
*/

#include <windows.h>
#include "options.h"
#include "globals.h"
#include "r4300i.h"
#include "plugins.h"
#include "hardware.h"
#include "1964ini.h"

char* CURRENT1964VERSION = "1964_064";

CONTROL Controls[4];


HardwareState  gHardwareState,  *p_gHardwareState;
MemoryState    gMemoryState,    *p_gMemoryState;

uint8* sDYN_PC_LOOKUP[0x10000];
uint8* sDWord[0x10000];
uint8* sDWord2[0x10000];

#ifdef DIRECT_TLB_LOOKUP
uint8* TLB_sDWord[0x100000];
#endif


#ifdef ENABLE_OPCODE_DEBUGGER
HardwareState gHardwareState_Interpreter_Compare;
MemoryState   gMemoryState_Interpreter_Compare;
HardwareState gHardwareState_Flushed_Dynarec_Compare;

uint8* sDWORD_R__Debug[0x10000];
uint8* sDWORD_R_2__Debug[0x10000];

#ifdef DIRECT_TLB_LOOKUP
//uint8* TLB_sDWord__Debug[0x100000];
uint8** TLB_sDWord_ptr;
#endif

uint8** sDWord_ptr;
uint8** sDWord2_ptr;
#endif





uint32* pcptr;
uint32 KEEP_RECOMPILING;
uint8* Block;

uint32  CPUdelayPC;			// the saved Program Counter at CPU load/branch delay mode
uint32  CPUdelay;			// Describer if the CPU is in load/branch delay mode

int		whichcore;			// Which compiler to use in emulator

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
volatile BOOL Rom_Loaded=FALSE;
volatile BOOL Emu_Keep_Running=FALSE;
volatile BOOL Emu_Is_Running=FALSE;
DWORD OSversion = 0;

t_rominfo rominfo;				// Rom information
uint8 *dyna_CodeTable = NULL;
uint8 *dyna_RecompCode = NULL;

char  generalmessage[256];				// general purpose buffer to display messages
char  main_directory[256];				// The file directory of 1964.exe
char  plugin_directory_to_use[256];		// The directory of plugin dlls
char  save_directory_to_use[256];		// The directory for all the save files
char  rom_directory_to_use[256];		// The directory to look for rom files
char  last_rom_directory[256];			// The rom directory of the last time

char  tracemessage[256];				// message buffer to display message into debug box
char  errormessage[256];				// message buffer to display message into error console window

int CounterFactor=COUTERFACTOR_1;
int CodeCheckMethod = CODE_CHECK_MEMORY_QWORD;
int VICounterFactors[9] =	{1,1,1,2,2,4,4,8,8};
int CounterFactors[9] =		{1,1,2,2,4,4,8,8,16};	// 1 = half rate, 2 = full rate


int DListCount=0;
int AListCount=0;
int PIDMACount=0;
int ControllerReadCount=0;
unsigned long TLBCount=0;
