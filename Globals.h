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


#ifndef _GLOBALS_H__1964_
#define _GLOBALS_H__1964_

#include <windows.h>

#define uint64 unsigned __int64
#define uint32 unsigned __int32
#define uint16 unsigned __int16
#define uint8  unsigned __int8


#define PATH_LEN 300     //max characters for path
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif


extern void __cdecl DisplayError (char * Message, ...);


typedef struct 
{       
        char  *real_name;
        char  *dmem_name;
        char  *imem_name;
        char  rom_name[256];

        uint16  validation;       // 0x00 
        uint8   compression;      // 0x02 
        uint8   unknown1;         // 0x03 
        uint32  clockrate;        // 0x04 
        uint32  programcounter;   // 0x08 
        uint32  release;          // 0x0c 
        
        uint32  crc1;             // 0x10 
        uint32  crc2;             // 0x14 
        uint64  unknown2;         // 0x18 
        
        uint8   name[20];         // 0x20 - 0x33 
        
        uint8   unknown3;         // 0x34 
        uint8   unknown4;         // 0x35 
        uint8   unknown5;         // 0x36 
        uint8   unknown6;         // 0x37 
        uint8   unknown7;         // 0x38 
        uint8   unknown8;         // 0x39 
        uint8   unknown9;         // 0x3a 
        uint8   manufacturerid;   // 0x3b 
        uint16  cartridgeid;      // 0x3c 
        uint8   countrycode;      // 0x3e 
        uint8   unknown10;        // 0x3f 

		uint64 CIC;
		uint64 TV_System;
		uint32 RDRam_Size_Hack;
} t_rominfo;

extern int DebuggerEnabled; /* Flag to toggle debug printing on/off */
extern uint32 gAllocationLength;
extern int eepromsize;

extern t_rominfo rominfo;
extern volatile BOOL Rom_Loaded;
extern volatile BOOL Emu_Keep_Running;
extern volatile BOOL Emu_Is_Running;
extern DWORD OSversion;

extern uint8* sDWord[0x10000];
extern uint8* sDWord2[0x10000];
#ifdef DIRECT_TLB_LOOKUP
extern uint8* TLB_sDWord[0x100000];
#endif

#ifdef ENABLE_OPCODE_DEBUGGER
extern uint8* sDWORD_R__Debug[0x10000];
extern uint8* sDWORD_R_2__Debug[0x10000];
extern uint8** sDWord_ptr;
extern uint8** sDWord2_ptr;
#ifdef DIRECT_TLB_LOOKUP
extern uint8** TLB_sDWord_ptr;
//extern uint8* TLB_sDWord__Debug[0x100000];
#endif
#endif


#include "plugins.h"
extern GFX_INFO Gfx_Info;
extern AUDIO_INFO Audio_Info;


extern char* CURRENT1964VERSION;
extern char  generalmessage[256];
extern char  main_directory[256];
extern char  plugin_directory_to_use[256];
extern char  save_directory_to_use[256];
extern char  rom_directory_to_use[256];
extern char  last_rom_directory[256];

extern char  tracemessage[256];
extern char  errormessage[256];

// TRACE macros
#ifdef DEBUG_COMMON
#define TRACE0(str)			{RefreshOpList(str);}
#define TRACE1(str, arg1)	{sprintf(tracemessage, str, arg1);RefreshOpList(tracemessage);}
#define TRACE2(str, arg1, arg2)		{sprintf(tracemessage, str, arg1, arg2);RefreshOpList(tracemessage);}
#define TRACE3(str, arg1, arg2, arg3)	{sprintf(tracemessage, str, arg1, arg2, arg3);RefreshOpList(tracemessage);}
#define TRACE4(str, arg1, arg2, arg3, arg4)		{sprintf(tracemessage, str, arg1, arg2, arg3, arg4);RefreshOpList(tracemessage);}
#else
#define TRACE0(str)
#define TRACE1(str, arg1)
#define TRACE2(str, arg1, arg2)
#define TRACE3(str, arg1, arg2, arg3)
#define TRACE4(str, arg1, arg2, arg3, arg4)
#endif

extern int CounterFactor;
extern int CodeCheckMethod;
extern int VICounterFactors[9];
extern int CounterFactors[9];

extern int DListCount;
extern int AListCount;
extern int PIDMACount;
extern int  ControllerReadCount;
extern unsigned long TLBCount;

extern char game_country_name[10];
extern int  game_country_tvsystem;

#endif /* _GLOBALS_H__1964_ */