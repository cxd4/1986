#include <windows.h>
#include "..\globals.h"
#include "registry.h"
#include "DLL_Video.h"

HINSTANCE   hinstLibVideo = NULL;

BOOL   (__cdecl* VIDEO_InitiateGFX)(GFX_INFO) = NULL;
void   (__cdecl* VIDEO_ProcessDList)(void) = NULL;
void   (__cdecl* VIDEO_RomOpen)(void) = NULL;
void   (__cdecl* VIDEO_RomClosed)(void) = NULL;
void   (__cdecl* VIDEO_DllClose)() = NULL;
void   (__cdecl* VIDEO_UpdateScreen)() = NULL;
void   (__cdecl* VIDEO_GetDllInfo )( PLUGIN_INFO *) = NULL;

void   (__cdecl* _VIDEO_ExtraChangeResolution)(HWND, long, HWND) = NULL;
void   (__cdecl* _VIDEO_DllConfig)() = NULL;
void   (__cdecl* _VIDEO_Test )(HWND) = NULL;
void   (__cdecl* _VIDEO_About)(HWND) = NULL;
void   (__cdecl* _VIDEO_MoveScreen)(int, int) = NULL;

void   (__cdecl* VIDEO_ChangeWindow)(int) = NULL;

BOOL LoadVideoPlugin(char* libname)
{
	// Release the video plug-in if it has already been loaded
    if (hinstLibVideo != NULL)
	{
        FreeLibrary(hinstLibVideo);
	}

    hinstLibVideo = LoadLibrary(libname);

    if (hinstLibVideo != NULL)	// Here the library is loaded successfully
    {
		// Get the VIDEO_GetDllInfo function address in the loaded DLL file
        VIDEO_GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *))       GetProcAddress(hinstLibVideo, "GetDllInfo");

        if(VIDEO_GetDllInfo)
        {
            PLUGIN_INFO Plugin_Info;
            ZeroMemory(&Plugin_Info, sizeof(Plugin_Info));

            VIDEO_GetDllInfo(&Plugin_Info);

            if(Plugin_Info.Type == PLUGIN_TYPE_GFX)	// Check if this is a video plugin
            {
                {
                    VIDEO_DllClose          = (void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "CloseDLL");
                    
                    _VIDEO_ExtraChangeResolution= (void   (__cdecl*)(HWND, long, HWND)) GetProcAddress(hinstLibVideo, "ChangeWinSize");
                    _VIDEO_Test              = (void (__cdecl*)(HWND))       GetProcAddress(hinstLibVideo, "DllTest");
                    _VIDEO_About             = (void (__cdecl*)(HWND))       GetProcAddress(hinstLibVideo, "DllAbout");
                    _VIDEO_DllConfig         = (void (__cdecl*)(HWND))       GetProcAddress(hinstLibVideo, "DllConfig");
                    _VIDEO_MoveScreen        = (void (__cdecl*)(int, int))   GetProcAddress(hinstLibVideo, "MoveScreen");
                    
					VIDEO_InitiateGFX       = (BOOL (__cdecl*)(GFX_INFO))   GetProcAddress(hinstLibVideo, "InitiateGFX");
                    VIDEO_RomOpen           = (void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "RomOpen");
					VIDEO_RomClosed           = (void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "RomClosed");
                    VIDEO_ProcessDList      = (void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "ProcessDList");
                    VIDEO_UpdateScreen      = (void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "UpdateScreen");

                    VIDEO_ChangeWindow      = (void (__cdecl*)(int))       GetProcAddress(hinstLibVideo, "ChangeWindow");
                }
            } 
            else 
            {
                DisplayError("%s dll ins't a gfx plugin !", libname);
                return FALSE;
            }
        } 
        else // The video plugin DLL does not has a VIDEO_GetDllInfo function, could be incorrect DLL
        {
            DisplayError("%s dll seem to be a wrong gfx dll !", libname);
            return FALSE;
        }
    } 
    else // Failed to load the DLL
    {
        DisplayError("LoadVideoPlugin(): %s not found. Please check your plugin directory for the existence of this file and use DirectX7 or better.", libname);
        return FALSE;
    }

  return TRUE;
}

void CloseVideoPlugin()
{
	if(VIDEO_DllClose != NULL)
		VIDEO_DllClose();

	if(hinstLibVideo)
		FreeLibrary(hinstLibVideo); 
	
	hinstLibVideo = NULL;
  
	VIDEO_InitiateGFX = NULL;
	VIDEO_ProcessDList = NULL;
	VIDEO_RomOpen = NULL;
	VIDEO_DllClose = NULL;
	_VIDEO_DllConfig = NULL;
	VIDEO_GetDllInfo = NULL;
	VIDEO_UpdateScreen = NULL;
	_VIDEO_ExtraChangeResolution = NULL;

	VIDEO_ChangeWindow = NULL;
	_VIDEO_Test = NULL;
	_VIDEO_About = NULL;
	_VIDEO_MoveScreen = NULL;
}

void VIDEO_DllConfig(HWND hParent)
{
	if (_VIDEO_DllConfig != NULL)
	{
		_VIDEO_DllConfig(hParent);
	}
	else
	{
		DisplayError("%s can not be configed.", "Video Plugin");
	}
}

void VIDEO_About(HWND hParent)
{
	if (_VIDEO_About != NULL)
	{
		_VIDEO_About(hParent);
	}
	else
	{
		DisplayError("%s: About information is not available for this plug-in.", "Video Plugin");
	}
}

void VIDEO_Test(HWND hParent)
{
	if (_VIDEO_Test != NULL)
	{
		_VIDEO_Test(hParent);
	}
	else
	{
		DisplayError("%s: Test function is not available for this plug-in.", "Video Plugin");
	}
}

void VIDEO_MoveScreen(int x, int y)
{
	if (_VIDEO_MoveScreen != NULL)
	{
		_VIDEO_MoveScreen(x,y);
	}
}

void VIDEO_ExtraChangeResolution(HWND hParent, long res, HWND hChild)
{
	if (_VIDEO_ExtraChangeResolution != NULL)
	{
		_VIDEO_ExtraChangeResolution(hParent,res,hChild);
	}
	else
	{
		DisplayError("%s: does not support changing resolution.", "Video Plugin");
	}
}

