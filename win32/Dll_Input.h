#ifndef _INPUT_H
#define _INPUT_H

#include "plugins.h"

extern void   (__cdecl* INPUT_RomOpen)(void);
extern void   (__cdecl* INPUT_DllClose)();
extern void   (__cdecl* INPUT_DllConfig)();
extern void   (__cdecl* INPUT_GetDllInfo)(PLUGIN_INFO *);
extern void   (__cdecl* INPUT_About)(HWND);

extern unsigned __int32 (__cdecl* INPUT_CheckController)(int controller);
extern BOOL (__cdecl* INPUT_Initialize)(HWND, HINSTANCE);
extern void (__cdecl* INPUT_ApplicationSyncAcquire)(HWND, int);

BOOL LoadInputPlugin(char* libname);
void CloseInputPlugin();

#endif //_INPUT_H