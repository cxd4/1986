/*______________________________________________________________________________
 |                                                                              |
 |  1964 - romlist.h                                                            |
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

#ifndef __ROMLIST_H
#define __ROMLIST_H

#include <windows.h>
#include "globals.h"
#include "1964ini.h"

struct ROMLIST_ENTRY_STRUCT
{
	INI_ENTRY * pinientry;
	char romfilename[_MAX_FNAME];
	long size;
};

typedef struct ROMLIST_ENTRY_STRUCT ROMLIST_ENTRY;

// Global variabls
#define MAX_ROMLIST 1000
extern ROMLIST_ENTRY *romlist[MAX_ROMLIST];
extern int romlist_count;

// Functions
BOOL RomListReadDirectory( const char *path );
void ClearRomList(void);
void InitRomList(void);
int  RomListAddEntry( INI_ENTRY * newentry, char *romfilename, long int filesize );
void RomListOpenRom(int index);
void RomListSelectRom(int index);
void RomListRomOptions(int index);

HWND NewRomList_CreateListViewControl(HWND hwndParent);
void NewRomList_ListViewHideHeader(HWND hwnd);
void NewRomList_ListViewShowHeader(HWND hwnd);
void NewRomList_ListViewFreshRomList(void);
void NewRomList_ListViewChangeWindowRect();

LRESULT APIENTRY RomListDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam);
void ReadRomHeaderInMemory(INI_ENTRY* ini_entry );
ROMLIST_ENTRY * RomListSelectedEntry();

#endif	//__ROMLIST_H
