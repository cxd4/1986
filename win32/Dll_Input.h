/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Dll_Input.h                                                          |
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

#ifndef _INPUT_H
#define _INPUT_H

#include "../plugins.h"

#define GFX_VERSION					0x0102
#define CONTROLLER_VERSION			0x0100

extern void CONTROLLER_CloseDLL(void);
extern void CONTROLLER_ControllerCommand( int _Control, BYTE *_Command);
extern void CONTROLLER_DllAbout(HWND _hWnd);
extern void CONTROLLER_DllConfig(HWND _hWnd);
extern void CONTROLLER_DllTest(HWND _hWnd);
extern void CONTROLLER_GetDllInfo( PLUGIN_INFO *_plugin);
extern void CONTROLLER_GetKeys(int _Control, BUTTONS *_Keys );
extern void CONTROLLER_InitiateControllers(HWND _hMainWindow, CONTROL _Controls[4]);
extern void CONTROLLER_ReadController( int _Control, BYTE * _Command );
extern void CONTROLLER_RomClosed(void);
extern void CONTROLLER_RomOpen(void);
extern void CONTROLLER_WM_KeyDown( WPARAM _wParam, LPARAM _lParam );
extern void CONTROLLER_WM_KeyUp( WPARAM _wParam, LPARAM _lParam );

extern void (__cdecl* _CONTROLLER_DllAbout)(HWND _hWnd);
extern void (__cdecl* _CONTROLLER_DllConfig)(HWND _hWnd);
extern void (__cdecl* _CONTROLLER_DllTest)(HWND _hWnd);

// Used when selecting plugins
extern void CONTROLLER_Under_Selecting_DllAbout(HWND _hWnd);
extern void CONTROLLER_Under_Selecting_DllTest(HWND _hWnd);
extern void (__cdecl* _CONTROLLER_Under_Selecting_DllAbout)(HWND _hWnd);
extern void (__cdecl* _CONTROLLER_Under_Selecting_DllTest)(HWND _hWnd);


extern BOOL LoadControllerPlugin(char* libname);
extern void CloseControllerPlugin(void);

extern HINSTANCE hinstControllerPlugin;

#endif //_INPUT_H