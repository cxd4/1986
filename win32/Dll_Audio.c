#include <windows.h>

#include "..\globals.h"

#include "DLL_Audio.h"

HINSTANCE	hinstLibAudio = NULL; 

void   (__cdecl* AUDIO_RomOpen)(void) = NULL;
void   (__cdecl* AUDIO_DllClose)() = NULL;
void   (__cdecl* AUDIO_DllConfig)() = NULL;
void   (__cdecl* AUDIO_GetDllInfo)(PLUGIN_INFO *) = NULL;
int    (__cdecl* AUDIO_Initialize)(HWND) = NULL;
void   (__cdecl* AUDIO_End)() = NULL;
void   (__cdecl* AUDIO_PlaySnd)(unsigned __int8*, unsigned __int32*) = NULL;
_int32 (__cdecl* AUDIO_TimeLeft)(unsigned char*) = NULL;
void   (__cdecl* AUDIO_About)(HWND) = NULL;

BOOL LoadAudioPlugin(char *libname)
{
	if (hinstLibAudio != NULL)
		FreeLibrary(hinstLibAudio);

	hinstLibAudio = LoadLibrary(libname);
	if (hinstLibAudio != NULL)
	{
		AUDIO_GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *))		GetProcAddress(hinstLibAudio, "GetDllInfo");
    if(AUDIO_GetDllInfo)
    {
      PLUGIN_INFO Plugin_Info;
      ZeroMemory(&Plugin_Info, sizeof(Plugin_Info));

      AUDIO_GetDllInfo(&Plugin_Info);
      if(Plugin_Info.Type == PLUGIN_TYPE_AUDIO)
      {
        if(Plugin_Info.Version == 1)
        {
#if 0
          if(Plugin_Info.NormalMemory == FALSE
            && Plugin_Info.MemoryBswaped == TRUE)
          {
#endif
            AUDIO_DllClose	= (void     (__cdecl*)(void))		GetProcAddress(hinstLibAudio, "CloseDLL");
            AUDIO_DllConfig	= (void     (__cdecl*)(HWND))		GetProcAddress(hinstLibAudio, "DllConfig");
            AUDIO_RomOpen   = (void     (__cdecl*)(void))       GetProcAddress(hinstLibAudio, "RomOpen");
            AUDIO_Initialize= (int      (__cdecl*)(HWND))		GetProcAddress(hinstLibAudio, "Initialize");
            AUDIO_End		= (void     (__cdecl*)(void))		GetProcAddress(hinstLibAudio, "End");
            AUDIO_PlaySnd	= (void     (__cdecl*)(unsigned __int8*, unsigned __int32*))GetProcAddress(hinstLibAudio, "PlaySnd");
            AUDIO_TimeLeft	= (__int32  (__cdecl*)(UCHAR*))		GetProcAddress(hinstLibAudio, "TimeLeft");
			AUDIO_About     = (void     (__cdecl*)(HWND))		GetProcAddress(hinstLibAudio, "DllAbout");
#if 0
          }
          else
          {
            DisplayError("%s dll don't have good memory setting !", libname, Plugin_Info.Version);
            CloseAudioPlugin();
            return FALSE;
          }
#endif
        }
        else
        {
          DisplayError("%s dll is a %d version of input plugin. But this emulator need a version 1 !", libname, Plugin_Info.Version);
          CloseAudioPlugin();
          return FALSE;
        }
      }
      else
      {
        DisplayError("%s dll ins't a input plugin !", libname);
        CloseAudioPlugin();
        return FALSE;
      }
    }
    else
    {
      DisplayError("%s dll seem to be a wrong input dll !", libname);
      CloseAudioPlugin();
      return FALSE;
    }
	}
	else
	{
		DisplayError("%s dll not found. Please check your directory for the existence of this file.", libname);
		CloseAudioPlugin();
    return FALSE;
	}

  return TRUE;
}

void CloseAudioPlugin()
{
  AUDIO_End();

  if(hinstLibAudio)
    FreeLibrary(hinstLibAudio);
  hinstLibAudio = NULL;
  
  AUDIO_RomOpen = NULL;
  AUDIO_DllClose = NULL;
  AUDIO_DllConfig = NULL;
  AUDIO_GetDllInfo = NULL;
  AUDIO_Initialize = NULL;
  AUDIO_End	 = NULL;
  AUDIO_PlaySnd	= NULL;
  AUDIO_TimeLeft = NULL;
}