/*______________________________________________________________________________
 |                                                                              |
 |  1964 - 1964ini.h                                                            |
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

#ifndef __1964INI_H
#define __1964INI_H

#include <windows.h>
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>

enum GAMESAVETYPE 
{	
	DEFAULT_SAVETYPE,
	EEPROM_SAVETYPE, 
	MEMPAK_SAVETYPE, 
	SRAM_SAVETYPE, 
	FLASHRAM_SAVETYPE, 
	FIRSTUSE_SAVETYPE,
	ANYUSED_SAVETYPE
};

enum EMULATORTYPE
{
	DEFAULT_EMULATORTYPE,
	DYNACOMPILER,
	INTERPRETER
};

enum EEPROMSIZE
{
	EEPROMSIZE_DEFAULT,
	EEPROMSIZE_NONE,
	EEPROMSIZE_2KB,
	EEPROMSIZE_4KB
};

enum CODECHECKTYPE
{
	CODE_CHECK_DEFAULT,
	CODE_CHECK_NONE,
	CODE_CHECK_DMA_ONLY,
	CODE_CHECK_MEMORY_DWORD,
	CODE_CHECK_MEMORY_QWORD,
	CODE_CHECK_MEMORY_QWORD_AND_DMA,
	CODE_CHECK_MEMORY_BLOCK,
	CODE_CHECK_MEMORY_BLOCK_AND_DMA
};

enum USETLBTYPE
{
	USETLB_DEFAULT,
	USETLB_YES,
	USETLB_NO
};

enum MAXFPSTYPE
{
	MAXFPS_DEFAULT,
	MAXFPS_NONE,
	MAXFPS_NTSC_60,
	MAXFPS_PAL_50,
	MAXFPS_AUTO_SYNC
};

enum RDRAMSIZETYPE
{
	RDRAMSIZE_DEFAULT,
	RDRAMSIZE_4MB,
	RDRAMSIZE_8MB
};

enum USEREGISTERCACHING
{
	USEREGC_DEFAULT,
	USEREGC_YES,
	USEREGC_NO
};

enum COUNTERFACTOR
{
	COUTERFACTOR_DEFAULT = 0,
	COUTERFACTOR_1,
	COUTERFACTOR_2,
	COUTERFACTOR_3,
	COUTERFACTOR_4,
	COUTERFACTOR_5,
	COUTERFACTOR_6,
	COUTERFACTOR_7,
	COUTERFACTOR_8
};

enum USEFPUHACK
{
	USEFPUHACK_DEFAULT,
	USEFPUHACK_YES,
	USEFPUHACK_NO
};

enum USEDMASEGMENTATION
{
	USEDMASEG_DEFAULT,
	USEDMASEG_YES,
	USEDMASEG_NO
};

struct INI_ENTRY_STRUCT {
	char	Game_Name[40];
	char	Comments[80];
	char	Alt_Title[51];
	uint32	crc1;
	uint32	crc2;
	uint8	countrycode;
	int		RDRAM_Size;
	enum	EMULATORTYPE	Emulator;
	enum	GAMESAVETYPE	Save_Type;
	enum	CODECHECKTYPE	Code_Check;
	int		Max_FPS;
	int		Use_TLB;
	int		Eeprom_size;
	int		Counter_Factor;
	int		Use_Register_Caching;
	int		FPU_Hack;
	int		DMA_Segmentation;
};
typedef struct INI_ENTRY_STRUCT INI_ENTRY;


// Support update to 3000 entries, should be enough for all the N64 Games
#define MAX_INI_ENTRIES		3000

// Globals definition

char * rdram_size_names[];
char * save_type_names[];
char * emulator_type_names[];
char * codecheck_type_names[];
char * maxfps_type_names[];
char * usetlb_type_names[];
char * eepromsize_type_names[];
char * counter_factor_names[];
char * register_caching_names[];
char * use_fpu_hack_names[];
char * use_dma_segmentation[];
float vips_speed_limits[];

extern INI_ENTRY currentromoptions;
extern INI_ENTRY * ini_entries[MAX_INI_ENTRIES];	// Only allocate memory for entry pointers
													// entries will be dynamically allocated
extern int ini_entry_count;

// Function definition
void InitIniEntries(void);
INI_ENTRY * GetNewIniEntry(void);
int  AddIniEntry(const INI_ENTRY *);
void DeleteIniEntry(const int index);
void DeleteAllIniEntries(void);
int	 FindIniEntry( const char * gamename, const uint32 crc1, const uint32 crc2, const uint8 country);
int  FindIniEntry2( const INI_ENTRY * );
BOOL ReadIniEntry(FILE *, INI_ENTRY *);
BOOL WriteIniEntry(FILE *, const INI_ENTRY *);
BOOL ReadAllIniEntries(FILE *);
BOOL WriteAllIniEntries(FILE *);
void CopyIniEntry(INI_ENTRY *, const INI_ENTRY *);
void DeleteIniEntryByEntry(INI_ENTRY *pentry);
void SetDefaultOptions(void);
void GenerateCurrentRomOptions(void);
BOOL Write1964DefaultOptionsEntry(FILE *pstream);
BOOL Read1964DefaultOptionsEntry(FILE *pstream);
void chopm(char * str);
uint32 ConvertHexStringToInt(const char *str, int nchars);

// 1964 default options
extern INI_ENTRY defaultoptions;
extern BOOL auto_run_rom;
extern BOOL auto_apply_cheat_code;
extern BOOL pause_at_inactive;
extern BOOL pause_at_menu;
extern BOOL use_default_save_directory;
extern BOOL use_default_state_save_directory;
extern BOOL use_default_plugin_directory;
extern BOOL use_last_rom_directory;
extern BOOL dma_in_segments;
extern BOOL expert_user_mode;
extern BOOL recent_rom_directory_list;
extern BOOL recent_game_list;
extern BOOL display_detail_status;
extern BOOL state_selector_menu;
extern BOOL critical_msg_window;
extern BOOL display_romlist;

extern char default_rom_directory[_MAX_PATH];
extern char default_save_directory[_MAX_PATH];
extern char default_state_save_directory[_MAX_PATH];
extern char default_plugin_directory[_MAX_PATH];
extern char user_set_rom_directory[_MAX_PATH];
extern char user_set_save_directory[_MAX_PATH];
extern char state_save_directory[_MAX_PATH];
extern char user_set_plugin_directory[_MAX_PATH];

#endif
