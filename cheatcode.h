/*______________________________________________________________________________
 |                                                                              |
 |  1964 - cheatcode.h                                                          |
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

#ifndef __CHEATCODE_H
#define __CHEATCODE_H

#include <windows.h>
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_CHEATCODE_PER_GROUP	10
#define MAX_CHEATCODE_GROUP_PER_ROM	150

enum APPLYCHEATMODE { INGAME, BOOTUPONCE, GSBUTTON, ONLYIN1964 };

struct CODENODE_STRUCT {
	uint32 addr;
	uint16 val;
};

typedef struct CODENODE_STRUCT CHEATCODENODE;

struct CODEGROUP_STRUCT {
	int  codecount;
	BOOL active;
	char name[80];
	CHEATCODENODE codelist[MAX_CHEATCODE_PER_GROUP];
};
typedef struct CODEGROUP_STRUCT CODEGROUP;

extern int  codegroupcount;
extern CODEGROUP * codegrouplist;

extern void InitCodeListForCurrentGame(void);
extern void CodeList_Clear(void);
extern void CodeList_GotoBeginning(void);
extern void CodeList_ClearGroup(int index);
extern BOOL CodeList_ApplyAllCode(enum APPLYCHEATMODE mode);
extern BOOL CodeList_ReadCode(char * intername_rom_name);
#endif
