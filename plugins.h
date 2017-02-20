/*______________________________________________________________________________
 |                                                                              |
 |  1964 - plugins.h                                                            |
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
 |  To contact the author:  Schibo and Rice                                     |
 |  email      : schibo@emulation64.com, dyangchicago@yahoo.com                 |
 |  paper mail :                                                                |
 |______________________________________________________________________________|
*/

#ifndef _PLUGINS_H
#define _PLUGINS_H

#define PLUGIN_TYPE_RSP             1
#define PLUGIN_TYPE_GFX             2
#define PLUGIN_TYPE_AUDIO           3
#define PLUGIN_TYPE_CONTROLLER      4

typedef struct {
    uint16 Version;
    uint16 Type;
    char Name[100];

    int NormalMemory;
    int MemoryBswaped;
} PLUGIN_INFO;


typedef struct {
    HWND hWnd;
    HWND hStatusBar;
    int MemoryBswaped;
    _int8 * HEADER;
    _int8 * RDRAM;
    _int8 * DMEM;
    _int8 * IMEM;
    uint32 * MI_INTR_RG;
    uint32 * DPC_START_RG;
    uint32 * DPC_END_RG;
    uint32 * DPC_CURRENT_RG;
    uint32 * DPC_STATUS_RG;
    uint32 * DPC_CLOCK_RG;
    uint32 * DPC_BUFBUSY_RG;
    uint32 * DPC_PIPEBUSY_RG;
    uint32 * DPC_TMEM_RG;

    uint32 * VI_STATUS_RG;
    uint32 * VI_ORIGIN_RG;
    uint32 * VI_WIDTH_RG;
    uint32 * VI_INTR_RG;
    uint32 * VI_V_CURRENT_LINE_RG;
    uint32 * VI_TIMING_RG;
    uint32 * VI_V_SYNC_RG;
    uint32 * VI_H_SYNC_RG;
    uint32 * VI_LEAP_RG;
    uint32 * VI_H_START_RG;
    uint32 * VI_V_START_RG;
    uint32 * VI_V_BURST_RG;
    uint32 * VI_X_SCALE_RG;
    uint32 * VI_Y_SCALE_RG;
    void (*CheckInterrupts)( void );
} GFX_INFO;


/* Note: BOOL, BYTE, WORD, DWORD, TRUE, FALSE are defined in windows.h */

#define PLUGIN_TYPE_AUDIO			3

#define EXPORT						__declspec(dllexport)
#define CALL						_cdecl

#define TV_SYSTEM_NTSC				1
#define TV_SYSTEM_PAL				0
#define TV_SYSTEM_MPAL				0

typedef struct {
	HWND hwnd;
	HINSTANCE hinst;

	BOOL MemoryBswaped;    // If this is set to TRUE, then the memory has been pre
	                       //   bswap on a dword (32 bits) boundry 
						   //	eg. the first 8 bytes are stored like this:
	                       //        4 3 2 1   8 7 6 5
	BYTE * HEADER;	// This is the rom header (first 40h bytes of the rom
					// This will be in the same memory format as the rest of the memory.
	BYTE * __RDRAM;
	BYTE * __DMEM;
	BYTE * __IMEM;

	DWORD * __MI_INTR_REG;

	DWORD * __AI_DRAM_ADDR_REG;
	DWORD * __AI_LEN_REG;
	DWORD * __AI_CONTROL_REG;
	DWORD * __AI_STATUS_REG;
	DWORD * __AI_DACRATE_REG;
	DWORD * __AI_BITRATE_REG;

	void (*CheckInterrupts)( void );
} AUDIO_INFO;


#endif // _PLUGINS_H