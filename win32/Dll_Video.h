#ifndef _VIDEO_H
#define _VIDEO_H

#include "../plugins.h"

extern BOOL   (__cdecl* VIDEO_InitiateGFX)(GFX_INFO);
extern void   (__cdecl* VIDEO_ProcessDList)(void);
extern void   (__cdecl* VIDEO_RomOpen)(void);
extern void   (__cdecl* VIDEO_RomClosed)(void);
extern void   (__cdecl* VIDEO_DllClose)(void);
extern void   (__cdecl* _VIDEO_DllConfig)(HWND);
extern void   (__cdecl* VIDEO_GetDllInfo)(PLUGIN_INFO *);
extern void   (__cdecl* VIDEO_UpdateScreen)(void);
extern void   (__cdecl* _VIDEO_Test )(HWND);
extern void   (__cdecl* _VIDEO_About)(HWND);
extern void   (__cdecl* _VIDEO_MoveScreen)(int, int);
extern void   (__cdecl* VIDEO_ChangeWindow)(int);
extern void   (__cdecl* _VIDEO_ExtraChangeResolution)(HWND, long, HWND);

BOOL LoadVideoPlugin(char* libname);
void CloseVideoPlugin(void);
void VIDEO_DllConfig(HWND);	
void VIDEO_About(HWND);
void VIDEO_Test(HWND);
void VIDEO_MoveScreen(int, int);
void VIDEO_ExtraChangeResolution(HWND, long, HWND);

extern HINSTANCE hinstLibVideo;

#endif //_VIDEO_H