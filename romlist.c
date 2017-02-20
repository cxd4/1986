/*$T romlist.c GC 1.136 03/09/02 17:32:23 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Routines for populating and sorting the romlist browser
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


/*
 * 1964 Copyright (C) 1999-2002 Joel Middendorf, <schibo@emulation64.com> This
 * program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version. This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. To contact the
 * authors: email: schibo@emulation64.com, rice1964@yahoo.com
 */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "hardware.h"
#include "globals.h"
#include "1964ini.h"
#include "romlist.h"
#include "fileio.h"
#include "cheatcode.h"
#include "win32/registry.h"
#include "win32/wingui.h"
#include "win32/windebug.h"
#include "debug_option.h"
#include "commctrl.h"
#include "memory.h"
#include "kaillera/kaillera.h"
#include "emulator.h"

/* Global variabls */
ROMLIST_ENTRY	*romlist[MAX_ROMLIST];
int				romlist_count = 0;
int				romlist_sort_method = ROMLIST_GAMENAME;
int				selected_rom_index;
static char		savedrompath[_MAX_PATH];
void			RomListGetGoodRomNameToDisplay(char *buf, int index);

/* Four column to display */
int				RomList_Column_Widths[ROMLIST_MAX_COLUMNS] = { 180, 50, 50, 360 };

/*
 =======================================================================================================================
    Functions
 =======================================================================================================================
 */
BOOL RomListReadDirectory(const char *spath)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	MSG				msg;
	char			romfilename[_MAX_PATH];
	char			drive[_MAX_DRIVE], dir[_MAX_DIR];
	char			filename[_MAX_FNAME], ext[_MAX_EXT];
	char			searchpath[_MAX_PATH];
	char			path[_MAX_PATH];
	HANDLE			findfirst;
	WIN32_FIND_DATA libaa;
	long			filesize;
	INI_ENTRY		entry;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	entry.Code_Check = 0;
	entry.Comments[0] = '\0';
	entry.Alt_Title[0] = '\0';
	entry.countrycode = 0;
	entry.crc1 = 0;
	entry.crc2 = 0;
	entry.Emulator = 0;
	entry.Game_Name[0] = '\0';
	entry.Max_FPS = 0;
	entry.RDRAM_Size = 0;
	entry.Save_Type = 0;
	entry.Use_TLB = 0;
	entry.Eeprom_size = 0;
	entry.Counter_Factor = 0;
	entry.Use_Register_Caching = 0;
	entry.FPU_Hack = 0;
	entry.DMA_Segmentation = 0;
	entry.Link_4KB_Blocks = 0;
	entry.Assume_32bit = 0;
	entry.Use_HLE = 0;
	entry.Advanced_Block_Analysis = 0;

	strcpy(directories.last_rom_directory, spath);

	strcpy(savedrompath, spath);
	strcpy(path, spath);
	if(path[strlen(path) - 1] != '\\') strcat(path, "\\");

	strcpy(searchpath, path);
	strcat(searchpath, "*.*");

	findfirst = FindFirstFile(searchpath, &libaa);
	if(findfirst == INVALID_HANDLE_VALUE)
	{
		/* No file in the rom directory */
		return(FALSE);
	}

	SetStatusBarText(0, "Looking for ROM file in the ROM directory and Generate List");
	do
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if(GetMessage(&msg, NULL, 0, 0))
			{
				if(!TranslateAccelerator(gui.hwnd1964main, gui.hAccTable, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}

		strcpy(romfilename, path);
		strcat(romfilename, libaa.cFileName);
		_splitpath(romfilename, drive, dir, filename, ext);
		_strlwr(ext);	/* Convert file extension to lower case */

		/*
		 * DisplayError("Fullname=%s, drive=%s, dir=%s, filename=%s, ext=%s", romfilename,
		 * drive, dir, filename, ext);
		 */
		if
		(
			stricmp(ext, ".rom") == 0
		||	stricmp(ext, ".v64") == 0
		||	stricmp(ext, ".z64") == 0
		||	stricmp(ext, ".usa") == 0
		||	stricmp(ext, ".n64") == 0
		||	stricmp(ext, ".bin") == 0
		||	stricmp(ext, ".zip") == 0
		||	stricmp(ext, ".j64") == 0
		||	stricmp(ext, ".pal") == 0
		)
		{
			if(strcmp(ext, ".zip") == 0)
			{
				/* Open and read this zip file */
				if((filesize = ReadZippedRomHeader(romfilename, &entry)) == 0)
				{
					/* This is not a ROM zip file, skipped it */
					continue;
				}
			}
			else
			{
				/* Open and read this rom file */
				if((filesize = ReadRomHeader(romfilename, &entry)) == 0)
				{
					/* This is not a ROM file, skipped it */
					continue;
				}
			}

			/* Add the header information to our romlist */
			{
				/*~~~~~~~~~~~~~~~~~~*/
				int ini_entries_index;
				/*~~~~~~~~~~~~~~~~~~*/

				if((ini_entries_index = FindIniEntry2(&entry)) >= 0 || (ini_entries_index = AddIniEntry(&entry)) >= 0)
				{
					/* Add the romlist */
					strcat(filename, ext);
					RomListAddEntry(&entry, romfilename, filesize);
				}
				else
				{
					/*
					 * Can not add to ini_entries list for some reason �
					 * Skipped
					 */
					continue;
				}
			}
		}
		else
			continue;	/* Skip this file */
	} while(FindNextFile(findfirst, &libaa));
	selected_rom_index = 0;
	NewRomList_Sort();
	return TRUE;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void ClearRomList(void)
{
	/*~~~~~~~~~~~~~~*/
	register int	i;
	/*~~~~~~~~~~~~~~*/

	for(i = 0; i < romlist_count; i++)
	{
		/* VirtualFree((void*)romlist[i], sizeof(ROMLIST_ENTRY), MEM_DECOMMIT); */
		VirtualFree((void *) romlist[i], 0, MEM_RELEASE);
		romlist[i] = NULL;
	}

	romlist_count = 0;
}

/*
 =======================================================================================================================
    Init the whole list, this function must be call before any other ROMLIST functions
 =======================================================================================================================
 */
void InitRomList(void)
{
	/*~~~~~~~~~~~~~~*/
	register int	i;
	/*~~~~~~~~~~~~~~*/

	for(i = 0; i < MAX_ROMLIST; i++) romlist[i] = NULL;
	romlist_count = 0;
}

/*
 =======================================================================================================================
    Return value is the index of the new entry that is added into the list
 =======================================================================================================================
 */
int RomListAddEntry(INI_ENTRY *newentry, char *romfilename, long filesize)
{
	/*~~~~~~*/
	int index;
	/*~~~~~~*/

	if(romlist_count == MAX_ROMLIST)
	{
		DisplayError("Your directory contains too many roms, I can not display it");
		return -1;
	}

	if((index = FindIniEntry2(newentry)) >= 0 || (index = AddIniEntry(newentry)) >= 0)
	{
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		/*
		 * We can either locate the entry in the ini_entries list �
		 * or this is a new entry, never in the ini_entries list, but we have �
		 * successfully add it into the ini_entries list �
		 * Allocate memory for a new ROMLIST_ENTRY
		 */
		ROMLIST_ENTRY	*pnewentry = NULL;
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

		pnewentry = (ROMLIST_ENTRY *) VirtualAlloc(NULL, sizeof(ROMLIST_ENTRY), MEM_COMMIT, PAGE_READWRITE);
		if(pnewentry == NULL)	/* fail to allocate memory */
			return -1;

		pnewentry->pinientry = ini_entries[index];
		strcpy(pnewentry->romfilename, romfilename);
		pnewentry->size = filesize;

		/* Insert the new entry sorted */
		if(romlist_count == 0)
		{
			selected_rom_index = romlist_count;
			romlist[romlist_count++] = pnewentry;
			return selected_rom_index;
		}
		else
		{
			/*~~*/
			int i;
			/*~~*/

			for(i = 0; i < romlist_count; i++)
			{
				if(stricmp(romlist[i]->pinientry->Game_Name, pnewentry->pinientry->Game_Name) >= 0) break;
			}

			selected_rom_index = i;
			if(selected_rom_index < romlist_count)
			{
				for(i = romlist_count; i > selected_rom_index; i--) romlist[i] = romlist[i - 1];
			}

			romlist[selected_rom_index] = pnewentry;
			romlist_count++;
			return selected_rom_index;
		}
	}
	else
	{
		return -1;
	}
}

extern void RefreshRecentGameMenus(char *filename);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void RomListOpenRom(int index)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~*/
	char	filename[_MAX_PATH];
	/*~~~~~~~~~~~~~~~~~~~~~~~~*/

	if(index >= 0 && index < romlist_count)
	{
		selected_rom_index = index;
		strcpy(filename, romlist[index]->romfilename);

		if(WinLoadRomStep2(filename) == TRUE)
		{
			RefreshRecentGameMenus(filename);

			/* Read hack code for this rom */
			CodeList_ReadCode(rominfo.name);

			EnableMenuItem(gui.hMenu1964main, ID_ROM_START, MF_ENABLED);
			EnableMenuItem(gui.hMenu1964main, ID_ROM_PAUSE, MF_GRAYED);
			EnableMenuItem(gui.hMenu1964main, ID_FILE_ROMINFO, MF_ENABLED);
			EnableMenuItem(gui.hMenu1964main, ID_FILE_CHEAT, MF_ENABLED);

			if(emuoptions.auto_run_rom || Kaillera_Is_Running == TRUE)
			{
				Kill();
				Play(); /* autoplay */
			}
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void RomListSelectRom(int index)
{
	if(index >= 0 && index < romlist_count)
	{
		selected_rom_index = index;
		ListView_SetSelectionMark(gui.hwndRomList, index);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void RomListRomOptions(int index)
{
	if(index >= 0 && index < romlist_count)
	{
		RomListSelectRom(index);
		DialogBox(gui.hInst, "ROM_OPTIONS", gui.hwnd1964main, (DLGPROC) RomListDialog);
		ListView_SetItemText(gui.hwndRomList, index, 3, romlist[index]->pinientry->Comments);

		RomListGetGoodRomNameToDisplay(generalmessage, index);
		ListView_SetItemText(gui.hwndRomList, index, 0, generalmessage);
	}
}

#define ROM_OPTION_SET_LISTBOX(CONTROLID, SIZE, DEFAULTVALUE, VALUE, NAMES) \
	SendDlgItemMessage \
	( \
		hDlg, \
		CONTROLID, \
		CB_RESETCONTENT, \
		0, \
		0 \
	); \
	for(i = 1; i < SIZE; i++) \
	{ \
		if(DEFAULTVALUE == i) \
		{ \
			strcpy(generalmessage, NAMES[i]); \
			strcat(generalmessage, " (default)"); \
			SendDlgItemMessage(hDlg, CONTROLID, CB_INSERTSTRING, i - 1, (LPARAM) (generalmessage)); \
		} \
		else \
			SendDlgItemMessage(hDlg, CONTROLID, CB_INSERTSTRING, i - 1, (LPARAM) (NAMES[i])); \
		if(i == VALUE) SendDlgItemMessage(hDlg, CONTROLID, CB_SETCURSEL, i - 1, 0); \
	} \
	if(VALUE == 0) SendDlgItemMessage(hDlg, CONTROLID, CB_SETCURSEL, DEFAULTVALUE - 1, 0);

/*
 =======================================================================================================================
 =======================================================================================================================
 */

LRESULT APIENTRY RomListDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~*/
	int		i;
	char	tempstr[_MAX_PATH];
	char	countryname[80];
	int		tvsystem;
	/*~~~~~~~~~~~~~~~~~~~~~~~*/

	switch(message)
	{
	case WM_INITDIALOG:
		ROM_OPTION_SET_LISTBOX
		(
			IDC_ROMOPTION_RDRAMSIZE,
			3,
			defaultoptions.RDRAM_Size,
			romlist[selected_rom_index]->pinientry->RDRAM_Size,
			rdram_size_names
		) ROM_OPTION_SET_LISTBOX
			(
				IDC_ROMOPTION_SAVETYPE,
				7,
				defaultoptions.Save_Type,
				romlist[selected_rom_index]->pinientry->Save_Type,
				save_type_names
			) ROM_OPTION_SET_LISTBOX
			(
				IDC_ROMOPTION_CPUEMULATOR,
				3,
				defaultoptions.Emulator,
				romlist[selected_rom_index]->pinientry->Emulator,
				emulator_type_names
			) ROM_OPTION_SET_LISTBOX
			(
				IDC_ROMOPTION_CODECHECK,
				9,
				defaultoptions.Code_Check,
				romlist[selected_rom_index]->pinientry->Code_Check,
				codecheck_type_names
			) ROM_OPTION_SET_LISTBOX
			(
				IDC_ROMOPTION_MAXVISPEED,
				5,
				defaultoptions.Max_FPS,
				romlist[selected_rom_index]->pinientry->Max_FPS,
				maxfps_type_names
			) ROM_OPTION_SET_LISTBOX
			(
				IDC_ROMOPTION_USETLB,
				3,
				defaultoptions.Use_TLB,
				romlist[selected_rom_index]->pinientry->Use_TLB,
				usetlb_type_names
			) ROM_OPTION_SET_LISTBOX
			(
				IDC_ROMOPTION_EEPROMSIZE,
				4,
				defaultoptions.Eeprom_size,
				romlist[selected_rom_index]->pinientry->Eeprom_size,
				eepromsize_type_names
			) ROM_OPTION_SET_LISTBOX
			(
				IDC_ROMOPTION_USEREGC,
				3,
				defaultoptions.Use_Register_Caching,
				romlist[selected_rom_index]->pinientry->Use_Register_Caching,
				register_caching_names
			) ROM_OPTION_SET_LISTBOX
			(
				IDC_ROMOPTION_CF,
				9,
				defaultoptions.Counter_Factor,
				romlist[selected_rom_index]->pinientry->Counter_Factor,
				counter_factor_names
			) ROM_OPTION_SET_LISTBOX
			(
				IDC_ROMOPTION_FPUHACK,
				3,
				defaultoptions.FPU_Hack,
				romlist[selected_rom_index]->pinientry->FPU_Hack,
				use_fpu_hack_names
			) ROM_OPTION_SET_LISTBOX
			(
				IDC_ROMOPTION_DMASEG,
				3,
				defaultoptions.DMA_Segmentation,
				romlist[selected_rom_index]->pinientry->DMA_Segmentation,
				use_dma_segmentation
			) ROM_OPTION_SET_LISTBOX
			(
				IDC_ROMOPTION_LINK4KB,
				3,
				defaultoptions.Link_4KB_Blocks,
				romlist[selected_rom_index]->pinientry->Link_4KB_Blocks,
				use_4kb_link_block_names
			) ROM_OPTION_SET_LISTBOX
			(
				IDC_ROMOPTION_ANALYSIS,
				3,
				defaultoptions.Advanced_Block_Analysis,
				romlist[selected_rom_index]->pinientry->Advanced_Block_Analysis,
				use_block_analysis_names
			) ROM_OPTION_SET_LISTBOX
			(
				IDC_ROMOPTION_ASSUME_32BIT,
				3,
				defaultoptions.Assume_32bit,
				romlist[selected_rom_index]->pinientry->Assume_32bit,
				assume_32bit_names
			) ROM_OPTION_SET_LISTBOX
			(
				IDC_ROMOPTION_HLE,
				3,
				defaultoptions.Use_HLE,
				romlist[selected_rom_index]->pinientry->Use_HLE,
				use_HLE_names
			)
		{
			/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
			char	drive[_MAX_DIR], dir[_MAX_DIR];
			char	fname[_MAX_FNAME], ext[_MAX_EXT];
			/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

			_splitpath(romlist[selected_rom_index]->romfilename, drive, dir, fname, ext);
			strcat(drive, dir);
			strcat(fname, ext);

			SetDlgItemText(hDlg, IDC_ROMOPTION_FILENAME, fname);
			SetDlgItemText(hDlg, IDC_ROMOPTION_FILELOCATION, drive);
		}

		SetDlgItemText(hDlg, IDC_ROMOPTION_GAMENAME, romlist[selected_rom_index]->pinientry->Game_Name);
		SetDlgItemText(hDlg, IDC_ROMOPTION_COMMENTS, romlist[selected_rom_index]->pinientry->Comments);
		SetDlgItemText(hDlg, IDC_ROMOPTION_ALTTITLE, romlist[selected_rom_index]->pinientry->Alt_Title);

		sprintf(tempstr, "%08X", romlist[selected_rom_index]->pinientry->crc1);
		SetDlgItemText(hDlg, IDC_ROMOPTION_CRC1, tempstr);

		sprintf(tempstr, "%08X", romlist[selected_rom_index]->pinientry->crc2);
		SetDlgItemText(hDlg, IDC_ROMOPTION_CRC2, tempstr);

		CountryCodeToCountryName_and_TVSystem
		(
			romlist[selected_rom_index]->pinientry->countrycode,
			countryname,
			&tvsystem
		);
		sprintf(tempstr, "%s (code=0x%02X)", countryname, romlist[selected_rom_index]->pinientry->countrycode);
		SetDlgItemText(hDlg, IDC_ROMOPTION_COUNTRYCODE, tempstr);

		return(TRUE);

	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
			{
				/* Read option setting from dialog */
				romlist[selected_rom_index]->pinientry->RDRAM_Size = SendDlgItemMessage
					(
						hDlg,
						IDC_ROMOPTION_RDRAMSIZE,
						CB_GETCURSEL,
						0,
						0
					) + 1;
				romlist[selected_rom_index]->pinientry->Save_Type = SendDlgItemMessage
					(
						hDlg,
						IDC_ROMOPTION_SAVETYPE,
						CB_GETCURSEL,
						0,
						0
					) + 1;
				romlist[selected_rom_index]->pinientry->Emulator = SendDlgItemMessage
					(
						hDlg,
						IDC_ROMOPTION_CPUEMULATOR,
						CB_GETCURSEL,
						0,
						0
					) + 1;
				romlist[selected_rom_index]->pinientry->Code_Check = SendDlgItemMessage
					(
						hDlg,
						IDC_ROMOPTION_CODECHECK,
						CB_GETCURSEL,
						0,
						0
					) + 1;
				romlist[selected_rom_index]->pinientry->Max_FPS = SendDlgItemMessage
					(
						hDlg,
						IDC_ROMOPTION_MAXVISPEED,
						CB_GETCURSEL,
						0,
						0
					) + 1;
				romlist[selected_rom_index]->pinientry->Use_TLB = SendDlgItemMessage
					(
						hDlg,
						IDC_ROMOPTION_USETLB,
						CB_GETCURSEL,
						0,
						0
					) + 1;
				romlist[selected_rom_index]->pinientry->Eeprom_size = SendDlgItemMessage
					(
						hDlg,
						IDC_ROMOPTION_EEPROMSIZE,
						CB_GETCURSEL,
						0,
						0
					) + 1;
				romlist[selected_rom_index]->pinientry->Use_Register_Caching = SendDlgItemMessage
					(
						hDlg,
						IDC_ROMOPTION_USEREGC,
						CB_GETCURSEL,
						0,
						0
					) + 1;
				romlist[selected_rom_index]->pinientry->Counter_Factor = SendDlgItemMessage
					(
						hDlg,
						IDC_ROMOPTION_CF,
						CB_GETCURSEL,
						0,
						0
					) + 1;
				romlist[selected_rom_index]->pinientry->FPU_Hack = SendDlgItemMessage
					(
						hDlg,
						IDC_ROMOPTION_FPUHACK,
						CB_GETCURSEL,
						0,
						0
					) + 1;
				romlist[selected_rom_index]->pinientry->DMA_Segmentation = SendDlgItemMessage
					(
						hDlg,
						IDC_ROMOPTION_DMASEG,
						CB_GETCURSEL,
						0,
						0
					) + 1;
				romlist[selected_rom_index]->pinientry->Link_4KB_Blocks = SendDlgItemMessage
					(
						hDlg,
						IDC_ROMOPTION_LINK4KB,
						CB_GETCURSEL,
						0,
						0
					) + 1;
				romlist[selected_rom_index]->pinientry->Advanced_Block_Analysis = SendDlgItemMessage
					(
						hDlg,
						IDC_ROMOPTION_ANALYSIS,
						CB_GETCURSEL,
						0,
						0
					) + 1;
				romlist[selected_rom_index]->pinientry->Assume_32bit = SendDlgItemMessage
					(
						hDlg,
						IDC_ROMOPTION_ASSUME_32BIT,
						CB_GETCURSEL,
						0,
						0
					) + 1;
				romlist[selected_rom_index]->pinientry->Use_HLE = SendDlgItemMessage
					(
						hDlg,
						IDC_ROMOPTION_HLE,
						CB_GETCURSEL,
						0,
						0
					) + 1;
				if
				(
					romlist[selected_rom_index]->pinientry->Code_Check != CODE_CHECK_PROTECT_MEMORY
				&&	romlist[selected_rom_index]->pinientry->Code_Check != CODE_CHECK_NONE
				&&	romlist[selected_rom_index]->pinientry->Link_4KB_Blocks == USE4KBLINKBLOCK_YES
				)
				{
					DisplayError("Link 4KB cannot be used when the self-mod code checking method is not PROTECT_MEMORY or No_Check, go to Game Options and set Link_4KB to No.");
					romlist[selected_rom_index]->pinientry->Link_4KB_Blocks = USE4KBLINKBLOCK_NO;
				}

				GetDlgItemText(hDlg, IDC_ROMOPTION_COMMENTS, romlist[selected_rom_index]->pinientry->Comments, 79);
				GetDlgItemText(hDlg, IDC_ROMOPTION_ALTTITLE, romlist[selected_rom_index]->pinientry->Alt_Title, 50);
				FileIO_Write1964Ini();

				EndDialog(hDlg, TRUE);
				return(TRUE);
			}

		case IDCANCEL:
			{
				EndDialog(hDlg, TRUE);
				return(TRUE);
			}
		}
	}

	return(FALSE);
}

/*
 =======================================================================================================================
    Create a rom list entry for the rom loaded in memory
 =======================================================================================================================
 */
void ReadRomHeaderInMemory(INI_ENTRY *ini_entry)
{
	/*~~~~~~~~~~~~~~~~~~*/
	uint8	buffer[0x100];
	/*~~~~~~~~~~~~~~~~~~*/

	memcpy(buffer, gMS_ROM_Image, 0x40);

	strncpy(ini_entry->Game_Name, buffer + 0x20, 0x14);
	SwapRomName(ini_entry->Game_Name);

	ini_entry->crc1 = *((uint32 *) (buffer + 0x10));
	ini_entry->crc2 = *((uint32 *) (buffer + 0x14));
	ini_entry->countrycode = buffer[0x3D];

	/* ini_entry->countrycode = buffer[0x3E]; */
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void RomListSelectLoadedRomEntry(void)
{
	/*~~~~~~~~~~~~~~*/
	int			i;
	INI_ENTRY	entry;
	/*~~~~~~~~~~~~~~*/

	ReadRomHeaderInMemory(&entry);

	for(i = 0; i < romlist_count; i++)
	{
		if(stricmp(romlist[i]->pinientry->Game_Name, entry.Game_Name) == 0) break;
	}

	selected_rom_index = i;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
ROMLIST_ENTRY *RomListSelectedEntry(void)
{
	return romlist[selected_rom_index];
}

/*
 =======================================================================================================================
    DupString - allocates a copy of a string. �
    lpsz - address of the null-terminated string to copy.
 =======================================================================================================================
 */
LPSTR DupString(LPSTR lpsz)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	int		cb = lstrlen(lpsz) + 1;
	LPSTR	lpszNew = LocalAlloc(LMEM_FIXED, cb);
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	if(lpszNew != NULL) CopyMemory(lpszNew, lpsz, cb);
	return lpszNew;
}

#define C_COLUMNS	6

typedef struct	myitem_tag
{
	LPSTR	aCols[C_COLUMNS];
} MYITEM;

/*
 =======================================================================================================================
    InitListViewItems - adds items and subitems to a list view. �
    Returns TRUE if successful, or FALSE otherwise. �
    hwndLV - handle to the list view control. �
    pfData - text file containing list view items with columns �
    separated by semicolons.
 =======================================================================================================================
 */
BOOL WINAPI InitListViewItems(HWND hwndLV)
{
	/*~~~~~~~~~~~~~~~~~~~~*/
	int		index;
	char	size[20];
	char	countryname[40];
	int		tvsystem;
	LVITEM	lvi;
	/*~~~~~~~~~~~~~~~~~~~~*/

	/* Initialize LVITEM members that are common to all items. */
	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.state = 0;
	lvi.stateMask = 0;
	lvi.pszText = LPSTR_TEXTCALLBACK;	/* app. maintains text */
	lvi.iImage = 0;						/* image list index */

	/* Read each line in the specified file. */
	for(index = 0; index < romlist_count; index++)
	{
		/* Initialize item-specific LVITEM members. */
		lvi.iItem = index;
		lvi.iSubItem = 0;

		RomListGetGoodRomNameToDisplay(generalmessage, index);
		lvi.pszText = generalmessage;
		ListView_InsertItem(hwndLV, &lvi);

		CountryCodeToCountryName_and_TVSystem(romlist[index]->pinientry->countrycode, countryname, &tvsystem);

		ListView_SetItemText(hwndLV, index, 1, countryname);
		sprintf(size, "%3dM", romlist[index]->size * 8 / 0x100000);
		ListView_SetItemText(hwndLV, index, 2, size);
		ListView_SetItemText(hwndLV, index, 3, romlist[index]->pinientry->Comments);
	}

	if(romlist_count > 0) ListView_SetSelectionMark(hwndLV, 0);
	return TRUE;
}

/*
 =======================================================================================================================
    InitListViewColumns - adds columns to a list view control. �
    Returns TRUE if successful, or FALSE otherwise. �
    hwndLV - handle to the list view control.
 =======================================================================================================================
 */
BOOL WINAPI InitListViewColumns(HWND hwndLV, int windowwidth)
{
	/*~~~~~~~~~~~~*/
	LVCOLUMNA	lvc;
	/*~~~~~~~~~~~~*/

	/* Initialize the LVCOLUMN structure. */
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = RomList_Column_Widths[0];

	/* lvc.cx = 180; */
	lvc.pszText = "ROM Name";
	lvc.iSubItem = 0;
	ListView_InsertColumn(hwndLV, 0, &lvc);

	lvc.pszText = "Country";

	/* lvc.cx = 50; */
	lvc.cx = RomList_Column_Widths[1];
	lvc.iSubItem = 1;
	ListView_InsertColumn(hwndLV, 1, &lvc);

	lvc.pszText = "Size";

	/* lvc.cx = 50; */
	lvc.cx = RomList_Column_Widths[2];
	lvc.iSubItem = 2;
	ListView_InsertColumn(hwndLV, 2, &lvc);

	lvc.pszText = "Comments";

	/* lvc.cx = windowwidth-305; */
	lvc.cx = RomList_Column_Widths[3];
	lvc.iSubItem = 3;
	ListView_InsertColumn(hwndLV, 3, &lvc);

	return TRUE;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
HWND NewRomList_CreateListViewControl(HWND hwndParent)
{
	/*~~~~~~~~~~~~~*/
	HWND	hwndLV;
	RECT	rcParent;
	/*~~~~~~~~~~~~~*/

	if(!guioptions.display_romlist) return NULL;

	/*
	 * Ensure that the common control DLL is loaded, and then create �
	 * the header control.
	 */
	InitCommonControls();
	GetClientRect(hwndParent, &rcParent);
	if(gui.hStatusBar != NULL)
	{
		/*~~~~~~~~~~~~~~~~*/
		RECT	rcStatusBar;
		/*~~~~~~~~~~~~~~~~*/

		GetWindowRect(gui.hStatusBar, &rcStatusBar);
		rcParent.bottom -= (rcStatusBar.bottom - rcStatusBar.top - 1);
	}

	/* Create the list view window. */
	hwndLV = CreateWindow
		(
			WC_LISTVIEW,
			"",
			WS_CHILD | LVS_REPORT | LVS_SINGLESEL,
			0,
			0,
			rcParent.right,
			rcParent.bottom,
			hwndParent,
			NULL,
			gui.hInst,
			NULL
		);
	if(hwndLV == NULL)
	{
		DisplayError("Error to create listview");
		return NULL;
	}

	InitListViewColumns(hwndLV, rcParent.right);
	InitListViewItems(hwndLV);

	ListView_SetExtendedListViewStyle(hwndLV, LVS_EX_FULLROWSELECT);	/* | LVS_EX_TRACKSELECT ); */

	/* ListView_SetHoverTime(hwndLV, 3000); */
	ShowWindow(hwndLV, SW_SHOW);
	UpdateWindow(hwndLV);
	if(romlist_count > 0) ListView_SetSelectionMark(hwndLV, 0);

	return hwndLV;	/* return the control's handle */
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void NewRomList_ListViewHideHeader(HWND hwnd)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	DWORD	dwStyle = GetWindowLong(hwnd, GWL_STYLE);
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	SetWindowLong(hwnd, GWL_STYLE, dwStyle | LVS_NOCOLUMNHEADER);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void NewRomList_ListViewShowHeader(HWND hwnd)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	DWORD	dwStyle = GetWindowLong(hwnd, GWL_STYLE);
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	SetWindowLong(hwnd, GWL_STYLE, dwStyle &~LVS_NOCOLUMNHEADER);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void NewRomList_ListViewFreshRomList(void)
{
	ListView_DeleteAllItems(gui.hwndRomList);
	NewRomList_Sort();
	InitListViewItems(gui.hwndRomList);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void NewRomList_ListViewChangeWindowRect(void)
{
	RomListRememberColumnWidth();
	ListView_DeleteAllItems(gui.hwndRomList);
	DestroyWindow(gui.hwndRomList);
	gui.hwndRomList = NewRomList_CreateListViewControl(gui.hwnd1964main);
}

/*
 =======================================================================================================================
    Sort the rom list according to parameter romlist_sort_method
 =======================================================================================================================
 */
BOOL InternalNameIsValid(char *name)
{
	/*~~~~~~~~~~~~~~~~~~~~~*/
	BOOL	valid = FALSE;
	int		i;
	int		n = strlen(name);
	/*~~~~~~~~~~~~~~~~~~~~~*/

	if(n > 20) n = 20;
	for(i = 0; i < n; i++)
	{
		if((unsigned char) (name[i]) > 0x7F)
		{
			valid = FALSE;
			break;
		}
		else if(name[i] != ' ')
			valid = TRUE;
	}

	return valid;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void NewRomList_Sort(void)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	struct ROMLIST_ENTRY_STRUCT *temp;
	int							i, j;
	BOOL						needswap;
	char						cname1[10], cname2[10];
	char						gamename1[200], gamename2[200];
	int							tv1, tv2;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	TRACE1("Sort rom list: %d", romlist_count);
	for(i = 0; i < romlist_count - 1; i++)
	{
		RomListGetGoodRomNameToDisplay(gamename1, i);
		for(j = i + 1; j < romlist_count; j++)
		{
			needswap = FALSE;
			switch(romlist_sort_method % 4)
			{
			case ROMLIST_GAMENAME:
				RomListGetGoodRomNameToDisplay(gamename2, j);
				if(stricmp(gamename1, gamename2) >= 0) needswap = TRUE;
				break;
			case ROMLIST_COUNTRY:
				CountryCodeToCountryName_and_TVSystem(romlist[i]->pinientry->countrycode, cname1, &tv1);
				CountryCodeToCountryName_and_TVSystem(romlist[j]->pinientry->countrycode, cname2, &tv2);

				/* if( romlist[i]->pinientry->countrycode > romlist[j]->pinientry->countrycode ) */
				if(stricmp(cname1, cname2) > 0) needswap = TRUE;
				break;
			case ROMLIST_SIZE:
				if(romlist[i]->size > romlist[j]->size) needswap = TRUE;
				break;
			case ROMLIST_COMMENT:
				if(stricmp(romlist[i]->pinientry->Comments, romlist[j]->pinientry->Comments) >= 0) needswap = TRUE;
				break;
			}

			if(romlist_sort_method >= 4)
			{
				needswap = 1 - needswap;
			}

			if(needswap)
			{
				temp = romlist[i];
				romlist[i] = romlist[j];
				romlist[j] = temp;
			}
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void RomListSaveCurrentPos(void)
{
	RomListRememberColumnWidth();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void RomListUseSavedPos(void)
{
	/*~~~~~~~~~*/
	RECT	rect;
	/*~~~~~~~~~*/

	/* int perpage = ListView_GetCountPerPage(); */
	ListView_GetItemRect(gui.hwndRomList, selected_rom_index - 1, &rect, LVIR_LABEL);
	ListView_Scroll(gui.hwndRomList, 0, rect.top);

	/*
	 * ListView_SetItemState(gui.hwndRomList, selected_rom_index,
	 * LVIS_FOCUSED|LVIS_SELECTED, LVIS_STATEIMAGEMASK );
	 */
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void RomListRememberColumnWidth(void)
{
	/*~~*/
	int i;
	/*~~*/

	for(i = 0; i < ROMLIST_MAX_COLUMNS; i++)
	{
		RomList_Column_Widths[i] = ListView_GetColumnWidth(gui.hwndRomList, i);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void RomListGetGoodRomNameToDisplay(char *buf, int index)
{
	if(strlen(romlist[index]->pinientry->Alt_Title) > 1)
		strcpy(buf, romlist[index]->pinientry->Alt_Title);
	else if(InternalNameIsValid(romlist[index]->pinientry->Game_Name))
		strcpy(buf, romlist[index]->pinientry->Game_Name);
	else
	{
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		char	drive[_MAX_DIR], dir[_MAX_DIR];
		char	fname[_MAX_DIR], ext[_MAX_EXT];
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

		_splitpath(romlist[index]->romfilename, drive, dir, fname, ext);
		strcat(fname, ext);
		strcpy(buf, fname);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
ROMLIST_ENTRY *RomListGet_Selected_Entry(void)
{
	return romlist[selected_rom_index];
}

/*
 =======================================================================================================================
    Sort the rom list according to parameter romlist_sort_method
 =======================================================================================================================
 */
void ConvertInvalidInternalName(char *oldname, char *newname)
{
	/*~~~~~~~~~~~~~~~~~~~~*/
	int i;
	int n = strlen(oldname);
	/*~~~~~~~~~~~~~~~~~~~~*/

	if(n > 20) n = 20;
	for(i = 0; i < n; i++)
	{
		if((unsigned char) (oldname[i]) > 0x7F)
		{
			newname[i] = oldname[i] - 0x7F;
			if(newname[i] < 0x20) newname[i] += 0x20;
		}
		else
		{
			newname[i] = oldname[i];
		}
	}

	newname[i] = '\0';
}
