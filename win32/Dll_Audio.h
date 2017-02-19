#ifndef _DLL_AUDIO_H
#define _DLL_AUDIO_H

#include "plugins.h"

extern void   (__cdecl* AUDIO_RomOpen)(void);
extern void   (__cdecl* AUDIO_DllClose)();
extern void   (__cdecl* AUDIO_DllConfig)();
extern void   (__cdecl* AUDIO_GetDllInfo)(PLUGIN_INFO *);
extern void   (__cdecl* AUDIO_Test )(HWND);
extern void   (__cdecl* AUDIO_About)(HWND);

//SB 2/8/99
extern int    (__cdecl* AUDIO_Initialize)(HWND);
extern void   (__cdecl* AUDIO_End)();
extern void   (__cdecl* AUDIO_PlaySnd)(unsigned __int8*, unsigned __int32*);
extern _int32 (__cdecl* AUDIO_TimeLeft)(unsigned char*);

BOOL LoadAudioPlugin(char* libname);
void CloseAudioPlugin();

#endif //_DLL_AUDIO_H