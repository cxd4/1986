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


extern BOOL LoadControllerPlugin(char* libname);
extern void CloseControllerPlugin(void);

extern HINSTANCE hinstControllerPlugin;

#endif //_INPUT_H