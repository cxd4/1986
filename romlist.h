/*$T romlist.h GC 1.136 02/28/02 07:51:10 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
#ifndef _ROMLIST_H__1964_
#define _ROMLIST_H__1964_

#include <windows.h>
#include "globals.h"
#include "1964ini.h"

struct ROMLIST_ENTRY_STRUCT
{
	INI_ENTRY	*pinientry;
	char		romfilename[_MAX_FNAME];
	long		size;
};

enum
{
	ROMLIST_GAMENAME,
	ROMLIST_COUNTRY,
	ROMLIST_SIZE,
	ROMLIST_COMMENT,
	ROMLIST_GAMENAME_INVERT,
	ROMLIST_COUNTRY_INVERT,
	ROMLIST_SIZE_INVERT,
	ROMLIST_COMMENT_INVERT
};

typedef struct ROMLIST_ENTRY_STRUCT ROMLIST_ENTRY;

/* Global variabls */
#define MAX_ROMLIST			2000
#define ROMLIST_MAX_COLUMNS 4

extern ROMLIST_ENTRY *romlist[MAX_ROMLIST];
extern int romlist_count;
extern int romlist_sort_method;
extern int RomList_Column_Widths[ROMLIST_MAX_COLUMNS];

/* Functions */
BOOL RomListReadDirectory(const char *path);
void ClearRomList(void);
void InitRomList(void);
int RomListAddEntry(INI_ENTRY *newentry, char *romfilename, long int filesize);
void RomListOpenRom(int index);
void RomListSelectRom(int index);
void RomListRomOptions(int index);
void RomListSaveCurrentPos(void);
void RomListUseSavedPos(void);

HWND NewRomList_CreateListViewControl(HWND hwndParent);
void NewRomList_ListViewHideHeader(HWND hwnd);
void NewRomList_ListViewShowHeader(HWND hwnd);
void NewRomList_ListViewFreshRomList(void);
void NewRomList_ListViewChangeWindowRect(void);
void NewRomList_Sort(void);
void RomListRememberColumnWidth(void);
ROMLIST_ENTRY *RomListGet_Selected_Entry(void);
void RomListSelectLoadedRomEntry(void);

LRESULT APIENTRY RomListDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam);
void ReadRomHeaderInMemory(INI_ENTRY *ini_entry);
ROMLIST_ENTRY *RomListSelectedEntry(void);

void ConvertInvalidInternalName(char *oldname, char *newname);
BOOL InternalNameIsValid(char *name);
#endif /* ROMLIST_H */
