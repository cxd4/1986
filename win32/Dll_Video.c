#include <windows.h>

#include "..\globals.h"

#include "DLL_Video.h"


HINSTANCE	hinstLibVideo = NULL;

BOOL   (__cdecl* VIDEO_InitiateGFX)(GFX_INFO) = NULL;
void   (__cdecl* VIDEO_ProcessDList)() = NULL;
void   (__cdecl* VIDEO_RomOpen)(void) = NULL;
void   (__cdecl* VIDEO_DllClose)() = NULL;
void   (__cdecl* VIDEO_DllConfig)() = NULL;
void   (__cdecl* VIDEO_UpdateScreen)() = NULL;
void   (__cdecl* VIDEO_GetDllInfo )( PLUGIN_INFO *) = NULL;
void   (__cdecl* VIDEO_About)(HWND) = NULL;

void   (__cdecl* VIDEO_ExtraChangeResolution)(HWND, long, HWND) = NULL;

BOOL LoadVideoPlugin(char* libname)
{
	if (hinstLibVideo != NULL)
		FreeLibrary(hinstLibVideo);

	hinstLibVideo = LoadLibrary(libname);
	if (hinstLibVideo != NULL)
	{
		VIDEO_GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *))		GetProcAddress(hinstLibVideo, "GetDllInfo");
		if(VIDEO_GetDllInfo)
		{
			PLUGIN_INFO Plugin_Info;
			ZeroMemory(&Plugin_Info, sizeof(Plugin_Info));

			VIDEO_GetDllInfo(&Plugin_Info);
			if(Plugin_Info.Type == PLUGIN_TYPE_GFX)
			{
				if(Plugin_Info.Version == 1)
				{
					VIDEO_DllClose			= (void (__cdecl*)(void))		GetProcAddress(hinstLibVideo, "CloseDLL");
					VIDEO_DllConfig			= (void (__cdecl*)(HWND))		GetProcAddress(hinstLibVideo, "DllConfig");
					VIDEO_InitiateGFX		= (BOOL (__cdecl*)(GFX_INFO))	GetProcAddress(hinstLibVideo, "InitiateGFX");
					VIDEO_RomOpen           = (void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "RomOpen");
					VIDEO_ProcessDList		= (void (__cdecl*)(void))		GetProcAddress(hinstLibVideo, "ProcessDList");
					VIDEO_UpdateScreen		= (void (__cdecl*)(void))		GetProcAddress(hinstLibVideo, "UpdateScreen");
					VIDEO_About				= (void (__cdecl*)(HWND))		GetProcAddress(hinstLibVideo, "DllAbout");

					VIDEO_ExtraChangeResolution= (void   (__cdecl*)(HWND, long, HWND)) GetProcAddress(hinstLibVideo, "ChangeWinSize");
				}
				else
				{
					DisplayError("%s dll is a %d version of gfx plugin. This emulator needs version 1!", libname, Plugin_Info.Version);
					CloseVideoPlugin();
					return FALSE;
				}
			} 
			else 
			{
				DisplayError("%s dll ins't a gfx plugin !", libname);
				CloseVideoPlugin();
				return FALSE;
			}
		} 
		else 
		{
			DisplayError("%s dll seem to be a wrong gfx dll !", libname);
			CloseVideoPlugin();
			return FALSE;
		}
	} 
	else 
	{
		DisplayError("%s dll not found. Please check your directory for the existence of this file.", libname);
		CloseVideoPlugin();
		return FALSE;
	}

  return TRUE;
}

void CloseVideoPlugin()
{
  if(VIDEO_DllClose)
    VIDEO_DllClose();

  if(hinstLibVideo)
    FreeLibrary(hinstLibVideo); 
  hinstLibVideo = NULL;
  
  VIDEO_InitiateGFX = NULL;
  VIDEO_ProcessDList = NULL;
  VIDEO_RomOpen = NULL;
  VIDEO_DllClose = NULL;
  VIDEO_DllConfig = NULL;
  VIDEO_GetDllInfo = NULL;
  VIDEO_UpdateScreen = NULL;
  VIDEO_ExtraChangeResolution = NULL;
}