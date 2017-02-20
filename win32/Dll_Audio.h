#ifndef _DLL_AUDIO_H
#define _DLL_AUDIO_H

#include "../plugins.h"

extern void   (__cdecl* AUDIO_DllClose)(void);
extern void   (__cdecl* AUDIO_RomClosed)(void);
extern void   (__cdecl* AUDIO_GetDllInfo)(PLUGIN_INFO *);

extern void   (__cdecl* _AUDIO_DllConfig)(HWND);
extern void   (__cdecl* _AUDIO_Test )(HWND);
extern void   (__cdecl* _AUDIO_About)(HWND);

extern int    (__cdecl* AUDIO_Initialize)(AUDIO_INFO);
extern void   (__cdecl* AUDIO_End)(void);
extern void   (__cdecl* AUDIO_PlaySnd)(unsigned __int8*, unsigned __int32*);
extern _int32 (__cdecl* AUDIO_TimeLeft)(unsigned char*);
extern void   (__cdecl* AUDIO_ProcessAList)(void);

extern void   (__cdecl* _AUDIO_AiDacrateChanged)(int);
extern void   (__cdecl* _AUDIO_AiLenChanged)(void);
extern DWORD  (__cdecl* _AUDIO_AiReadLength)(void);
extern void   (__cdecl* _AUDIO_AiUpdate)(BOOL);

BOOL LoadAudioPlugin(char* libname);
void CloseAudioPlugin(void);
void AUDIO_DllConfig(HWND);
void AUDIO_Test(HWND);
void AUDIO_About(HWND);
void AUDIO_AiDacrateChanged(int);
void AUDIO_AiLenChanged(void);
DWORD AUDIO_AiReadLength(void);
void AUDIO_AiUpdate(BOOL);

extern HINSTANCE   hinstLibAudio;

#endif //_DLL_AUDIO_H