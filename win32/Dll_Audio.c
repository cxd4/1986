#include <windows.h>
#include "..\globals.h"
#include "..\n64rcp.h"
#include "registry.h"
#include "DLL_Audio.h"

HINSTANCE   hinstLibAudio = NULL; 

void   (__cdecl* AUDIO_RomClosed)(void) = NULL;
void   (__cdecl* AUDIO_DllClose)() = NULL;

void   (__cdecl* _AUDIO_DllConfig)(HWND) = NULL;
void   (__cdecl* _AUDIO_About)(HWND) = NULL;
void   (__cdecl* _AUDIO_Test )(HWND) = NULL;

void   (__cdecl* AUDIO_GetDllInfo)(PLUGIN_INFO *) = NULL;
BOOL   (__cdecl* AUDIO_Initialize)(AUDIO_INFO) = NULL;
void   (__cdecl* AUDIO_End)() = NULL;
void   (__cdecl* AUDIO_PlaySnd)(unsigned __int8*, unsigned __int32*) = NULL;
_int32 (__cdecl* AUDIO_TimeLeft)(unsigned char*) = NULL;
void   (__cdecl* AUDIO_ProcessAList)(void) = NULL;

void   (__cdecl* _AUDIO_AiDacrateChanged)(int) = NULL;
void   (__cdecl* _AUDIO_AiLenChanged)(void) = NULL;
DWORD  (__cdecl* _AUDIO_AiReadLength)(void) = NULL;
void   (__cdecl* _AUDIO_AiUpdate)(BOOL) = NULL;


BOOL LoadAudioPlugin(char *libname)
{
    if (hinstLibAudio != NULL)
	{
        FreeLibrary(hinstLibAudio);
	}

	// Load the Audio DLL
    hinstLibAudio = LoadLibrary(libname);

    if (hinstLibAudio != NULL)	// Check if load DLL successfully
    {
		// Get the function address AUDIO_GetDllInfo in the audio DLL file
        AUDIO_GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *)) GetProcAddress(hinstLibAudio, "GetDllInfo");

		if(AUDIO_GetDllInfo)
		{
			PLUGIN_INFO Plugin_Info;
			ZeroMemory(&Plugin_Info, sizeof(Plugin_Info));

			AUDIO_GetDllInfo(&Plugin_Info);

			if(Plugin_Info.Type == PLUGIN_TYPE_AUDIO)
			{
				/*if(Plugin_Info.Version == 1)*/
				if(1)
				{

#if 0
					if(Plugin_Info.NormalMemory == FALSE && Plugin_Info.MemoryBswaped == TRUE)
					{
#endif

						_AUDIO_AiDacrateChanged = (void      (__cdecl*)(   int    ))	GetProcAddress(hinstLibAudio, "AiDacrateChanged"      );
						_AUDIO_AiLenChanged     = (void      (__cdecl*)(   void   ))	GetProcAddress(hinstLibAudio, "AiLenChanged"      );
						_AUDIO_AiReadLength     = (DWORD     (__cdecl*)(   void   ))	GetProcAddress(hinstLibAudio, "AiReadLength"      );
						_AUDIO_AiUpdate         = (void      (__cdecl*)(   BOOL   ))	GetProcAddress(hinstLibAudio, "AiUpdate"      );
						AUDIO_DllClose          = (void      (__cdecl*)(   void   ))     GetProcAddress(hinstLibAudio, "CloseDLL"      );

                        _AUDIO_About            = (void      (__cdecl*)(   HWND   ))     GetProcAddress(hinstLibAudio, "Test"      );
						_AUDIO_DllConfig         = (void (__cdecl*)(HWND))       GetProcAddress(hinstLibAudio, "DllConfig");
						_AUDIO_Test             = (void      (__cdecl*)(   HWND   ))     GetProcAddress(hinstLibAudio, "About"      );
                        AUDIO_Initialize        = (BOOL      (__cdecl*)(AUDIO_INFO))     GetProcAddress(hinstLibAudio, "InitiateAudio" );
                        AUDIO_ProcessAList      = (void      (__cdecl*)(   void   ))     GetProcAddress(hinstLibAudio, "ProcessAList"  );
						AUDIO_RomClosed         = (void      (__cdecl*)(   void   ))     GetProcAddress(hinstLibAudio, "RomClosed"     );

#if 0
					}
					else
					{
						DisplayError("%s has an incompatible memory config (Needs dword swapped).", libname, Plugin_Info.Version);
						return FALSE;
					}
#endif
				}
				else
				{
					DisplayError("%s dll is a %d version of audio plugin spec. 1964 requires version 1.", libname, Plugin_Info.Version);
					return FALSE;
				}
			}
			else
			{
				DisplayError("%s dll is not an audio plugin.", libname);
				CloseAudioPlugin();
				return FALSE;
			}
		}
		else
		{
			DisplayError("%s is a bad audio dll.", libname);
			return FALSE;
		}
    }
    else
    {
        if (strcmp(libname, "steb_aud") != 0)
		{
            DisplayError("%s not found. Please check your directory for the existence of this file and use DirectX 8.", libname);
		}
        strcpy(gRegSettings.AudioPlugin, "");
        WriteConfiguration();
        return FALSE;
    }

  return TRUE;
}

void CloseAudioPlugin()
{
	//  AUDIO_End();
	if(AUDIO_RomClosed)
	{
		AUDIO_RomClosed();
	}

	if(AUDIO_DllClose)
	{
		AUDIO_DllClose();
	}

  
	if(hinstLibAudio)
	{
		FreeLibrary(hinstLibAudio);
		hinstLibAudio = NULL;
	}
  
	AUDIO_DllClose = NULL;
	_AUDIO_DllConfig = NULL;
	_AUDIO_About = NULL;
	_AUDIO_Test = NULL;
	AUDIO_GetDllInfo = NULL;
	AUDIO_Initialize = NULL;
	AUDIO_End  = NULL;
	AUDIO_PlaySnd = NULL;
	AUDIO_TimeLeft = NULL;
}

void AUDIO_DllConfig(HWND hParent)
{
	if (_AUDIO_DllConfig != NULL)
	{
		_AUDIO_DllConfig(hParent);
	}
	else
	{
		DisplayError("%s can not be configed.", "Audio Plugin");
	}
}

void AUDIO_About(HWND hParent)
{
	if (_AUDIO_About != NULL)
	{
		_AUDIO_About(hParent);
	}
	else
	{
//		DisplayError("%s: About information is not available for this plug-in.", "Audio Plugin");
	}
}

void AUDIO_Test(HWND hParent)
{
	if (_AUDIO_Test != NULL)
	{
		_AUDIO_Test(hParent);
	}
	else
	{
//		DisplayError("%s: Test box is not available for this plug-in.", "Audio Plugin");
	}
}

void AUDIO_AiDacrateChanged(int SystemType)
{
	if (_AUDIO_AiDacrateChanged != NULL)
	{
		_AUDIO_AiDacrateChanged(SystemType);
	}
	else
	{
//		DisplayError("%s: Test box is not available for this plug-in.", "Audio Plugin");
	}
}

void AUDIO_AiLenChanged(void)
{
	if (_AUDIO_AiLenChanged != NULL)
	{
		_AUDIO_AiLenChanged();
	}
	else
	{
//		DisplayError("%s: Test box is not available for this plug-in.", "Audio Plugin");
	}

}

DWORD AUDIO_AiReadLength(void)
{
	if (_AUDIO_AiReadLength != NULL)
	{
		return _AUDIO_AiReadLength();
	}
	else
	{
//		DisplayError("%s: Test box is not available for this plug-in.", "Audio Plugin");
		//return AI_LEN_REG;
		return 0;
	}
}

void AUDIO_AiUpdate(BOOL update)
{
	if (_AUDIO_AiUpdate != NULL)
	{
		_AUDIO_AiUpdate(update);
	}
	else
	{
//		DisplayError("%s: Test box is not available for this plug-in.", "Audio Plugin");
	}
}
