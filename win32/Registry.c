/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Emulator for Nintendo 64 console system                              |
 |  Copyright (C) 2001  Joel Middendorf  schibo@emulation64.com                 |
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


The project is a direct documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar. Most source
code comments are taken directly from anarko's n64toolkit with consent and are 
the property of anarko.
*/

#include <windows.h>
#include "../globals.h"
#include "resource.h"
#include "registry.h"
#include "../1964ini.h"
#include "../debug_option.h"
#include "wingui.h"
#include "windebug.h"

#define MAIN_1964_KEY       "Software\\1964emu_064\\GUI"
#define KEY_WINDOW_X        "WindowXPos"
#define KEY_WINDOW_Y        "WindowYPos"
#define KEY_MAXIMIZED       "Maximized"
#define KEY_CLIENT_WIDTH    "ClientWidth"
#define KEY_ROM_PATH        "ROMPath"
#define KEY_THREAD_PRIORITY "ThreadPriority"
#define KEY_AUDIO_PLUGIN    "AudioPlugin"
#define KEY_INPUT_PLUGIN    "InputPlugin"
#define KEY_VIDEO_PLUGIN    "VideoPlugin"

char* ReadRegistryStrVal(char* MainKey, char* Field);
uint32 ReadRegistryDwordVal(char* MainKey, char* Field);
BOOL Test1964Registry();
void InitAll1964Options();

//---------------------------------------------------------------------------------------
// This function is called only when 1964 starts
//---------------------------------------------------------------------------------------
void ReadConfiguration()
{
	char Directory[_MAX_PATH], str[260];
	int i;

	if( Test1964Registry() == FALSE )
	{
		InitAll1964Options();
		WriteConfiguration();
		return;
	}

    strcpy(Directory,main_directory);

	strcpy(default_plugin_directory, Directory);
	strcat(default_plugin_directory,"Plugin\\");

	strcpy(default_save_directory, Directory);
	strcat(default_save_directory,"Save\\");

    strcpy(gRegSettings.ROMPath,        ReadRegistryStrVal(MAIN_1964_KEY, "ROMPath")    );
    strcpy(gRegSettings.AudioPlugin,    ReadRegistryStrVal(MAIN_1964_KEY, "AudioPlugin"));
    strcpy(gRegSettings.VideoPlugin,    ReadRegistryStrVal(MAIN_1964_KEY, "VideoPlugin"));
    strcpy(gRegSettings.InputPlugin,    ReadRegistryStrVal(MAIN_1964_KEY, "InputPlugin"));

    strcpy(user_set_rom_directory,			ReadRegistryStrVal(MAIN_1964_KEY, "ROMDirectory")    );
	if( strlen(user_set_rom_directory) == 0 ) strcpy(user_set_rom_directory, gRegSettings.ROMPath);

    strcpy(last_rom_directory,			ReadRegistryStrVal(MAIN_1964_KEY, "LastROMDirectory")    );
    strcpy(user_set_save_directory,				ReadRegistryStrVal(MAIN_1964_KEY, "SaveDirectory"));

    strcpy(state_save_directory,			ReadRegistryStrVal(MAIN_1964_KEY, "StateSaveDirectory"));
	if( strlen(state_save_directory) == 0 ) strcpy(state_save_directory, default_state_save_directory);

    strcpy(user_set_plugin_directory, ReadRegistryStrVal(MAIN_1964_KEY, "PluginDirectory"));
	if( strlen(user_set_plugin_directory) == 0 ) strcpy(user_set_plugin_directory, default_plugin_directory);

	auto_run_rom =					ReadRegistryDwordVal(MAIN_1964_KEY, "AutoRunRom");
	auto_apply_cheat_code =			ReadRegistryDwordVal(MAIN_1964_KEY, "AutoApplyCheat");
	pause_at_inactive =				ReadRegistryDwordVal(MAIN_1964_KEY, "PauseWhenInactive");
	pause_at_menu =					ReadRegistryDwordVal(MAIN_1964_KEY, "PauseAtMenu");
	expert_user_mode =				ReadRegistryDwordVal(MAIN_1964_KEY, "ExpertUserMode");
	recent_rom_directory_list =		ReadRegistryDwordVal(MAIN_1964_KEY, "RomDirectoryListMenu");
	recent_game_list =				ReadRegistryDwordVal(MAIN_1964_KEY, "GameListMenu");
	display_detail_status =			ReadRegistryDwordVal(MAIN_1964_KEY, "DisplayDetailStatus");
	state_selector_menu =			ReadRegistryDwordVal(MAIN_1964_KEY, "StateSelectorMenu");
	critical_msg_window =			ReadRegistryDwordVal(MAIN_1964_KEY, "DisplayCriticalMessageWindow");
	display_romlist =				ReadRegistryDwordVal(MAIN_1964_KEY, "DisplayRomList");

	dma_in_segments =				ReadRegistryDwordVal(MAIN_1964_KEY, "DmaInSegments");

	use_default_save_directory =		ReadRegistryDwordVal(MAIN_1964_KEY, "UseDefaultSaveDiectory");
	use_default_state_save_directory =	ReadRegistryDwordVal(MAIN_1964_KEY, "UseDefaultStateSaveDiectory");
	use_default_plugin_directory =		ReadRegistryDwordVal(MAIN_1964_KEY, "UseDefaultPluginDiectory");
	use_last_rom_directory =			ReadRegistryDwordVal(MAIN_1964_KEY, "UseLastRomDiectory");


	// Set the save directory to use
	if( use_default_save_directory )
		strcpy(save_directory_to_use, default_save_directory);
	else
		strcpy(save_directory_to_use, user_set_save_directory);

	// Set the ROM directory to use
	if( use_last_rom_directory )
    { 
		strcpy(rom_directory_to_use, last_rom_directory);
    }
	else
    {
        strcpy(rom_directory_to_use, user_set_rom_directory);
    }

	// Set the plugin directory to use
	if( use_default_plugin_directory )
		strcpy(plugin_directory_to_use, default_plugin_directory);
	else
		strcpy(plugin_directory_to_use, user_set_plugin_directory);


#ifdef DEBUG_COMMON
	defaultoptions.Eeprom_size =	ReadRegistryDwordVal(MAIN_1964_KEY, "DefaultEepromSize");
	if( defaultoptions.Eeprom_size == 0 || defaultoptions.Eeprom_size > 2 )
		defaultoptions.Eeprom_size = 1;

	defaultoptions.RDRAM_Size =		ReadRegistryDwordVal(MAIN_1964_KEY, "DefaultRdramSize");
	if( defaultoptions.RDRAM_Size == 0 || defaultoptions.RDRAM_Size > 2 )
		defaultoptions.RDRAM_Size = 2;

	defaultoptions.Emulator = DYNACOMPILER;

	defaultoptions.Save_Type =		ReadRegistryDwordVal(MAIN_1964_KEY, "DefaultSaveType");
	if( defaultoptions.Save_Type == DEFAULT_SAVETYPE )
		defaultoptions.Save_Type = ANYUSED_SAVETYPE;

	defaultoptions.Code_Check =		ReadRegistryDwordVal(MAIN_1964_KEY, "DefaultCodeCheck");
	defaultoptions.Max_FPS =		ReadRegistryDwordVal(MAIN_1964_KEY, "DefaultMaxFPS");
	defaultoptions.Use_TLB =		ReadRegistryDwordVal(MAIN_1964_KEY, "DefaultUseTLB");
	defaultoptions.FPU_Hack =		ReadRegistryDwordVal(MAIN_1964_KEY, "DefaultUseFPUHack");
	defaultoptions.DMA_Segmentation=dma_in_segments;
	defaultoptions.Use_Register_Caching =		ReadRegistryDwordVal(MAIN_1964_KEY, "RegisterCaching");
	if( defaultoptions.Use_Register_Caching == USEREGC_DEFAULT || defaultoptions.Use_Register_Caching > USEREGC_NO ) 
		defaultoptions.Use_Register_Caching = USEREGC_YES;

	defaultoptions.Counter_Factor = 1;
#else
	defaultoptions.Eeprom_size = EEPROMSIZE_2KB;
	defaultoptions.RDRAM_Size = RDRAMSIZE_4MB;
	defaultoptions.Emulator = DYNACOMPILER;
	defaultoptions.Save_Type = ANYUSED_SAVETYPE;
	defaultoptions.Code_Check =		CODE_CHECK_MEMORY_QWORD;
	defaultoptions.Max_FPS =		MAXFPS_AUTO_SYNC;
	defaultoptions.Use_TLB =		USETLB_YES;
	defaultoptions.FPU_Hack =		USEFPUHACK_YES;
	defaultoptions.DMA_Segmentation=USEDMASEG_YES;
	defaultoptions.Use_Register_Caching = USEREGC_YES;
	defaultoptions.Counter_Factor = 1;
#endif


	clientwidth = ReadRegistryDwordVal(MAIN_1964_KEY, "ClientWindowWidth");
	window_position.top = ReadRegistryDwordVal(MAIN_1964_KEY, "1964WindowTOP");
	if( window_position.top < 0 ) window_position.top = 100;
	window_position.left = ReadRegistryDwordVal(MAIN_1964_KEY, "1964WindowLeft");
	if( window_position.left < 0 ) window_position.left = 100;

#ifdef DEBUG_COMMON
	debug_trap = ReadRegistryDwordVal(MAIN_1964_KEY, "DebugTrap");
	debug_si_controller = ReadRegistryDwordVal(MAIN_1964_KEY, "DebugSIController");
	debug_sp_task = ReadRegistryDwordVal(MAIN_1964_KEY, "DebugSPTask");
	debug_si_task = ReadRegistryDwordVal(MAIN_1964_KEY, "DebugSITask");
	debug_sp_dma = ReadRegistryDwordVal(MAIN_1964_KEY, "DebugSPDMA");
	debug_si_dma = ReadRegistryDwordVal(MAIN_1964_KEY, "DebugSIDMA");
	debug_pi_dma = ReadRegistryDwordVal(MAIN_1964_KEY, "DebugPIDMA");
	debug_si_mempak = ReadRegistryDwordVal(MAIN_1964_KEY, "DebugMempak");
	debug_tlb = ReadRegistryDwordVal(MAIN_1964_KEY, "DebugTLB");
	debug_si_eeprom = ReadRegistryDwordVal(MAIN_1964_KEY, "DebugEEPROM");
	debug_sram = ReadRegistryDwordVal(MAIN_1964_KEY, "DebugSRAM");
#endif

	for(i=0; i<4; i++)
	{
		sprintf(str, "RecentRomDirectory%d", i);
		strcpy(recent_rom_directory_lists[i], ReadRegistryStrVal(MAIN_1964_KEY, str));
		if( strlen(recent_rom_directory_lists[i]) == 0 )
			strcpy(recent_rom_directory_lists[i], "Empty Rom Directory Slot");
	}

	for( i=0; i<8; i++ )
	{
		sprintf(str, "RecentGame%d", i);
		strcpy(recent_game_lists[i], ReadRegistryStrVal(MAIN_1964_KEY, str));
		if( strlen(recent_game_lists[i]) == 0 )
			strcpy(recent_game_lists[i], "Empty Game Slot");
	}
}


BOOL Test1964Registry()
{
	HKEY  hKey1, hKey2;
	DWORD rc;

	if(RegConnectRegistry(NULL,HKEY_CURRENT_USER, &hKey1) == ERROR_SUCCESS)
	{
		char    szBuffer[260];
		strcpy(szBuffer, MAIN_1964_KEY);
    
		rc = RegOpenKey(hKey1, MAIN_1964_KEY, &hKey2);
		RegCloseKey(hKey1);

		if(rc == ERROR_SUCCESS)
			return TRUE;
		else
			return FALSE;
	}

	DisplayError("Error to read Windows registry database");
	return FALSE;
}

char szData[MAX_PATH];
char* ReadRegistryStrVal(char* MainKey, char* Field)
{
	HKEY  hKey1, hKey2;
	DWORD rc;

	DWORD cbData, dwType;
  
	if(RegConnectRegistry(NULL,HKEY_CURRENT_USER, &hKey1) == ERROR_SUCCESS)
	{
		char    szBuffer[260];
    
		strcpy(szBuffer, MainKey);
    
		rc = RegOpenKey(hKey1, szBuffer, &hKey2);
		if(rc == ERROR_SUCCESS)
		{
			cbData = sizeof(szData);
			rc = RegQueryValueEx( hKey2, Field, NULL, &dwType, (LPBYTE)szData, &cbData);
      
			RegCloseKey(hKey2);
		}
		RegCloseKey(hKey1);
	}

	if (rc == ERROR_SUCCESS && cbData != 0)
	{
		return(szData);
	}
	else
	{
		return("");
	}
}

uint32 DwordData;
uint32 ReadRegistryDwordVal(char* MainKey, char* Field)
{
	HKEY  hKey1, hKey2;
	DWORD rc;

	DWORD cbData;
	DWORD dwType = REG_DWORD;
  
	if(RegConnectRegistry(NULL,HKEY_CURRENT_USER, &hKey1) == ERROR_SUCCESS)
	{
		char    szBuffer[260];
    
		strcpy(szBuffer, MainKey);
    
		rc = RegOpenKey(hKey1, szBuffer, &hKey2);
		if(rc == ERROR_SUCCESS)
		{
			cbData = sizeof(DwordData);
			rc = RegQueryValueEx( hKey2, Field, NULL, &dwType, (LPBYTE)&DwordData, &cbData);
      
			RegCloseKey(hKey2);
		}
		RegCloseKey(hKey1);
	}

	if (rc == ERROR_SUCCESS && cbData != 0)
	{
		return(DwordData);
	}
	else
	{
		return(0);
	}
}

//---------------------------------------------------------------------------------------

void WriteConfiguration()
{
	HKEY    hKey1, hKey2;
	DWORD   rc;
	DWORD cbData;
	char  szBuffer[260], str[260];
	int i;

	// Save current configuration
	if (RegConnectRegistry(NULL, HKEY_CURRENT_USER, &hKey1) != ERROR_SUCCESS)
	{
		DisplayError("Error to write registry");
		return;
	}

	strcpy(szBuffer, MAIN_1964_KEY);
	rc = RegOpenKey(hKey1, szBuffer, &hKey2);
	if(rc != ERROR_SUCCESS)
	{
		rc = RegCreateKey(hKey1, szBuffer, &hKey2);
		if(rc != ERROR_SUCCESS)
		{
			DisplayError("Error to create MAIN_1964_KEY in the registry");
			return;
		}
	}

	strcpy(szData, gRegSettings.ROMPath);
	cbData = strlen(szData) + 1;            
	RegSetValueEx( hKey2, KEY_ROM_PATH, 0, REG_SZ, (LPBYTE)szData, cbData);


	strcpy(szData, gRegSettings.VideoPlugin);
	cbData = strlen(szData) + 1;            
	RegSetValueEx( hKey2, KEY_VIDEO_PLUGIN, 0, REG_SZ, (LPBYTE)szData, cbData);


	strcpy(szData, gRegSettings.InputPlugin);
	cbData = strlen(szData) + 1;            
	RegSetValueEx( hKey2, KEY_INPUT_PLUGIN, 0, REG_SZ, (LPBYTE)szData, cbData);


	strcpy(szData, gRegSettings.AudioPlugin);
	cbData = strlen(szData) + 1;            
	RegSetValueEx( hKey2, KEY_AUDIO_PLUGIN, 0, REG_SZ, (LPBYTE)szData, cbData);

	strcpy(szData, user_set_rom_directory);
	cbData = strlen(szData) + 1;            
	RegSetValueEx( hKey2, "ROMDirectory", 0, REG_SZ, (LPBYTE)szData, cbData);

	strcpy(szData, last_rom_directory);
	cbData = strlen(szData) + 1;            
	RegSetValueEx( hKey2, "LastROMDirectory", 0, REG_SZ, (LPBYTE)szData, cbData);

	strcpy(szData, user_set_save_directory);
	cbData = strlen(szData) + 1;            
	RegSetValueEx( hKey2, "SaveDirectory", 0, REG_SZ, (LPBYTE)szData, cbData);

	strcpy(szData, state_save_directory);
	cbData = strlen(szData) + 1;            
	RegSetValueEx( hKey2, "StateSaveDirectory", 0, REG_SZ, (LPBYTE)szData, cbData);

	strcpy(szData, user_set_plugin_directory);
	cbData = strlen(szData) + 1;            
	RegSetValueEx( hKey2, "PluginDirectory", 0, REG_SZ, (LPBYTE)szData, cbData);

	cbData = sizeof(DwordData);

	DwordData = auto_run_rom;
	RegSetValueEx( hKey2, "AutoRunRom", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = auto_apply_cheat_code;
	RegSetValueEx( hKey2, "AutoApplyCheat", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = pause_at_inactive;
	RegSetValueEx( hKey2, "PauseWhenInactive", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = pause_at_menu;
	RegSetValueEx( hKey2, "PauseAtMenu", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = expert_user_mode;
	RegSetValueEx( hKey2, "ExpertUserMode", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = recent_rom_directory_list;
	RegSetValueEx( hKey2, "RomDirectoryListMenu", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = recent_game_list;
	RegSetValueEx( hKey2, "GameListMenu", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = display_detail_status;
	RegSetValueEx( hKey2, "DisplayDetailStatus", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = state_selector_menu;
	RegSetValueEx( hKey2, "StateSelectorMenu", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = critical_msg_window;
	RegSetValueEx( hKey2, "DisplayCriticalMessageWindow", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);
	
	DwordData = display_romlist;
	RegSetValueEx( hKey2, "DisplayRomList", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = use_default_save_directory;
	RegSetValueEx( hKey2, "UseDefaultSaveDiectory", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = use_default_state_save_directory;
	RegSetValueEx( hKey2, "UseDefaultStateSaveDiectory", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = use_default_plugin_directory;
	RegSetValueEx( hKey2, "UseDefaultPluginDiectory", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = use_last_rom_directory;
	RegSetValueEx( hKey2, "UseLastRomDiectory", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

#ifdef DEBUG_COMMON
	DwordData = dma_in_segments;
	RegSetValueEx( hKey2, "DmaInSegments", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = defaultoptions.Eeprom_size;
	RegSetValueEx( hKey2, "DefaultEepromSize", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = defaultoptions.RDRAM_Size;
	RegSetValueEx( hKey2, "DefaultRdramSize", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = defaultoptions.Emulator;
	RegSetValueEx( hKey2, "DefaultEmulator", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = defaultoptions.Save_Type;
	RegSetValueEx( hKey2, "DefaultSaveType", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = defaultoptions.Code_Check;
	RegSetValueEx( hKey2, "DefaultCodeCheck", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = defaultoptions.Max_FPS;
	RegSetValueEx( hKey2, "DefaultMaxFPS", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = defaultoptions.Use_TLB;
	RegSetValueEx( hKey2, "DefaultUseTLB", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = defaultoptions.Counter_Factor;
	RegSetValueEx( hKey2, "CounterFactor", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = defaultoptions.Use_Register_Caching;
	RegSetValueEx( hKey2, "RegisterCaching", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = defaultoptions.FPU_Hack;
	RegSetValueEx( hKey2, "DefaultUseFPUHack", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);
#endif

	DwordData = clientwidth;
	RegSetValueEx( hKey2, "ClientWindowWidth", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = window_position.top;
	RegSetValueEx( hKey2, "1964WindowTOP", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = window_position.left;
	RegSetValueEx( hKey2, "1964WindowLeft", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

#ifdef DEBUG_COMMON
	DwordData = debug_trap;
	RegSetValueEx( hKey2, "DebugTrap", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = debug_si_controller;
	RegSetValueEx( hKey2, "DebugSIController", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = debug_sp_task;
	RegSetValueEx( hKey2, "DebugSPTask", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = debug_si_task;
	RegSetValueEx( hKey2, "DebugSITask", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = debug_sp_dma;
	RegSetValueEx( hKey2, "DebugSPDMA", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = debug_si_dma;
	RegSetValueEx( hKey2, "DebugSIDMA", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = debug_pi_dma;
	RegSetValueEx( hKey2, "DebugPIDMA", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = debug_si_mempak;
	RegSetValueEx( hKey2, "DebugMempak", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = debug_tlb;
	RegSetValueEx( hKey2, "DebugTLB", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = debug_si_eeprom;
	RegSetValueEx( hKey2, "DebugEEPROM", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);

	DwordData = debug_sram;
	RegSetValueEx( hKey2, "DebugSRAM", 0, REG_DWORD, (LPBYTE)&DwordData, cbData);
#endif

	for(i=0; i<4; i++)
	{
		strcpy(szData, recent_rom_directory_lists[i]);
		sprintf(str, "RecentRomDirectory%d", i);
		cbData = strlen(szData) + 1;            
		RegSetValueEx( hKey2, str, 0, REG_SZ, (LPBYTE)szData, cbData);
	}

	for( i=0; i<8; i++ )
	{
		strcpy(szData, recent_game_lists[i]);
		sprintf(str, "RecentGame%d", i);
		cbData = strlen(szData) + 1;            
		RegSetValueEx( hKey2, str, 0, REG_SZ, (LPBYTE)szData, cbData);
	}

	RegCloseKey(hKey2);
}

void InitAll1964Options()
{
	int i;
	strcpy(default_plugin_directory, main_directory);
	strcat(default_plugin_directory,"Plugin\\");

	strcpy(default_save_directory, main_directory);
	strcat(default_save_directory,"Save\\");

    strcpy(gRegSettings.ROMPath, "");
    strcpy(gRegSettings.AudioPlugin, "tr64_audio.dll");
    strcpy(gRegSettings.VideoPlugin, "1964ogl.dll");
    strcpy(gRegSettings.InputPlugin, "Basic Keyboard Plugin.dll");
	strcpy(user_set_rom_directory, "");
    strcpy(last_rom_directory, "");
    strcpy(user_set_save_directory,	default_save_directory);
    strcpy(state_save_directory, default_save_directory);
	strcpy(user_set_plugin_directory, default_plugin_directory);
	strcpy(save_directory_to_use, default_save_directory);
	strcpy(rom_directory_to_use, last_rom_directory);
	strcpy(plugin_directory_to_use, default_plugin_directory);

	auto_run_rom = 1;
	auto_apply_cheat_code =	0;
	pause_at_inactive =	1;
	pause_at_menu =	0;
	dma_in_segments = 1;

	expert_user_mode = 0;
	recent_rom_directory_list =	0;
	recent_game_list = 0;
	display_detail_status =	1;
	state_selector_menu = 0;
	critical_msg_window = 0;
	display_romlist = 1;

	use_default_save_directory = 1;
	use_default_state_save_directory = 1;
	use_default_plugin_directory = 1;
	use_last_rom_directory = 1;

	defaultoptions.Eeprom_size = EEPROMSIZE_2KB;

	defaultoptions.RDRAM_Size = RDRAMSIZE_4MB;

	defaultoptions.Emulator = DYNACOMPILER;

	defaultoptions.Save_Type = ANYUSED_SAVETYPE;

	defaultoptions.Code_Check =	CODE_CHECK_MEMORY_QWORD;
	defaultoptions.Max_FPS = MAXFPS_AUTO_SYNC;
	defaultoptions.Use_TLB = USETLB_YES;
	defaultoptions.FPU_Hack = USEFPUHACK_NO;
	defaultoptions.DMA_Segmentation = USEDMASEG_YES;

	defaultoptions.Counter_Factor = COUTERFACTOR_1;

	defaultoptions.Use_Register_Caching = USEREGC_YES;

	clientwidth = 640;
	window_position.top = 100;
	window_position.left = 100;

#ifdef DEBUG_COMMON
	debug_trap = 1;
	debug_si_controller = 1;
	debug_sp_task = 0;
	debug_si_task = 0;
	debug_sp_dma = 0;
	debug_si_dma = 0;
	debug_pi_dma = 1;
	debug_si_mempak = 1;
	debug_tlb = 1;
	debug_si_eeprom = 1;
	debug_sram = 1;
#endif

	for(i=0; i<4; i++)
	{
		strcpy(recent_rom_directory_lists[i],"Empty Rom Directory Slot");
	}

	for( i=0; i<8; i++ )
	{
		strcpy(recent_game_lists[i],"Empty Game Slot");
	}
}


