/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Emulator for Nintendo 64 console system                              |
 |  Copyright (C) 2000  Joel Middendorf  schibo@emuhq.com                       |
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

/*
The project is a direct documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar :). Most source
code comments are taken directly from anarko's n64toolkit with consent and are 
the property of anarko.
*/

#include "resource.h"

int APIENTRY WinMain( HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow);

HWND InitWin98UI(HANDLE hInstance, int nCmdShow);

LRESULT APIENTRY MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY About(HWND hDlg, unsigned message, WORD wParam, LONG lParam);

#define MAXFILENAME 256 	     /* maximum length of file pathname      */

char szWindowTitle[80];

void r4300i_Init();
void CleanUp();
void ReadRomData(char* RomPath);
void __cdecl DisplayError (char * Message, ...);

BOOL Rom_Loaded;
BOOL WinLoadRom();

/* the legal stuff */
char MainDisclaimer[320];
char WarrantyPart1[700];
char WarrantyPart2[700];
char DistConditions[800]; /* GNU Redistribution Conditions */
extern int LoadGNUDistConditions(char* ConditionsBuf);
LRESULT APIENTRY ConditionsDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam);


HINSTANCE hInst;
HANDLE hAccTable;								/* handle to accelerator table		*/
HWND hwnd;                                      /* handle to main window			*/
HMENU hMenu;
