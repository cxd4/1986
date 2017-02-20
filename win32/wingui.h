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
*/

/*
The project is a direct documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar. Most source
code comments are taken directly from anarko's n64toolkit with consent and are 
the property of anarko.
*/

#include "resource.h"

/* Functions in the wingui.c */
int APIENTRY WinMain( HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow);
HWND InitWin98UI(HANDLE hInstance, int nCmdShow);
LRESULT APIENTRY MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY About(HWND hDlg, unsigned message, WORD wParam, LONG lParam);
void __cdecl DisplayError (char * Message, ...);
BOOL WinLoadRom();
BOOL WinLoadRomStep2(char * szFileName);
void Pause();
void Resume();
void Kill();
void Play();
void Stop();
void KailleraPlay();
void OpenROM();
void CloseROM();
void ChangeDirectory();
void SaveState();
void LoadState();
void SaveStateByNumber(WPARAM wparam);
void LoadStateByNumber(WPARAM wparam);
void SaveStateByDialog(void);
void LoadStateByDialog(void);
void EnableStateMenu(void);
void DisableStateMenu(void);
void PrepareBeforePlay(void);
void KillCPUThread();
void SetCounterFactor(int);
void SetCodeCheckMethod(int);
void InitPluginData(void);
void Set_1964_Directory(void);
void ResizeVideoWindow(void);
void CountryCodeToCountryName_and_TVSystem(int countrycode, char *countryname, int * tvsystem);
void CaptureScreenToFile(void);

void StateSetNumber(int number);
void Exit1964();
void GetPluginDir(char* Directory);
extern int LoadGNUDistConditions(char* ConditionsBuf);
LRESULT APIENTRY ConditionsDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam);
LRESULT APIENTRY DefaultOptionsDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam);
LRESULT APIENTRY CheatAndHackDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam);
LRESULT APIENTRY CriticalMessageDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam);

#define MAXFILENAME 256          /* maximum length of file pathname      */

char szWindowTitle[80];

/* the legal stuff */
unsigned char MainDisclaimer[320];
unsigned char WarrantyPart1[700];
unsigned char WarrantyPart2[700];
unsigned char DistConditions[800]; /* GNU Redistribution Conditions */


HINSTANCE hInst;
HANDLE hAccTable;                               /* handle to accelerator table				*/
extern HWND hwnd;								/* handle to main window					*/
extern HWND hwndRomList;						/* Handle to the rom list child window		*/
extern HWND hStatusBar;							/* Window Handle of the status bar			*/
extern HWND hToolBar;							/* Window Handle of the toolbar				*/
extern HWND hClientWindow;						/* Window handle of the client child window */
extern HWND hCriticalMsgWnd;					/* handle to critical message window		*/
extern HMENU hMenu;

extern void DockStatusBar(void);
extern void InitStatusBarParts(void);
extern void SetStatusBarText(int, char *);

/* Status Bar fields */
char staturbar_field_1[];
char staturbar_field_2[];
char staturbar_field_3[];
char staturbar_field_4[];
char staturbar_field_5[];

extern int clientwidth;							/* Client window width				*/
extern RECT window_position;					/* 1964 main window location		*/

char recent_rom_directory_lists[4][260];
char recent_game_lists[8][260];
