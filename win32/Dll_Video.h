#ifndef _VIDEO_H
#define _VIDEO_H

#include "plugins.h"

extern BOOL   (__cdecl* VIDEO_InitiateGFX)(GFX_INFO);
extern void   (__cdecl* VIDEO_ProcessDList)();
extern void   (__cdecl* VIDEO_RomOpen)(void);
extern void   (__cdecl* VIDEO_DllClose)();
extern void   (__cdecl* VIDEO_DllConfig)();
extern void   (__cdecl* VIDEO_GetDllInfo)(PLUGIN_INFO *);
extern void   (__cdecl* VIDEO_UpdateScreen)();
extern void   (__cdecl* VIDEO_Test )(HWND);
extern void   (__cdecl* VIDEO_About)(HWND);
extern void   (__cdecl* VIDEO_MoveScreen)(int, int);

extern void   (__cdecl* VIDEO_ExtraChangeResolution)(HWND, long, HWND);

BOOL LoadVideoPlugin(char* libname);
void CloseVideoPlugin();

#endif //_VIDEO_H