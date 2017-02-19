#include <windows.h>

#include "..\globals.h"

#include "DLL_Input.h"


HINSTANCE	hinstLibInput = NULL;

void   (__cdecl* INPUT_RomOpen)(void) = NULL;
void   (__cdecl* INPUT_DllClose)() = NULL;
void   (__cdecl* INPUT_DllConfig)() = NULL;
void   (__cdecl* INPUT_GetDllInfo)(PLUGIN_INFO *) = NULL;
unsigned __int32 (__cdecl* INPUT_CheckController)(int controller) = NULL;
BOOL   (__cdecl* INPUT_Initialize)(HWND,HINSTANCE) = NULL;
void   (__cdecl* INPUT_ApplicationSyncAcquire)(HWND, int) = NULL;
void   (__cdecl* INPUT_About)(HWND) = NULL;

BOOL LoadInputPlugin(char* libname)
{
	if (hinstLibInput != NULL)
		FreeLibrary(hinstLibInput);

	hinstLibInput = LoadLibrary(libname);
	if (hinstLibInput != NULL)
	{
		INPUT_GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *))		GetProcAddress(hinstLibInput, "GetDllInfo");
    if(INPUT_GetDllInfo)
    {
      PLUGIN_INFO Plugin_Info;
      ZeroMemory(&Plugin_Info, sizeof(Plugin_Info));

      INPUT_GetDllInfo(&Plugin_Info);
      if(Plugin_Info.Type == PLUGIN_TYPE_CONTROLLER)
      {
        if(Plugin_Info.Version == 1)
        {
#if 0
          if(Plugin_Info.NormalMemory == FALSE
            && Plugin_Info.MemoryBswaped == TRUE)
          {
#endif
            INPUT_DllClose					= (void (__cdecl*)(void))				GetProcAddress(hinstLibInput, "CloseDLL");
            INPUT_DllConfig					= (void (__cdecl*)(HWND))				GetProcAddress(hinstLibInput, "DllConfig");
            INPUT_RomOpen					= (void (__cdecl*)(void))				GetProcAddress(hinstLibInput, "RomOpen");
            INPUT_Initialize				= (BOOL (__cdecl*)(HWND,HINSTANCE))		GetProcAddress(hinstLibInput, "Initialize");
            INPUT_ApplicationSyncAcquire    = (void (__cdecl*)(HWND,int))			GetProcAddress(hinstLibInput, "ApplicationSyncAcquire");
            INPUT_CheckController			= (unsigned __int32   (__cdecl*)(int))	GetProcAddress(hinstLibInput, "CheckController");
			INPUT_About						= (void (__cdecl*)(HWND))				GetProcAddress(hinstLibInput, "DllAbout");
#if 0
          }
          else
          {
            DisplayError("%s dll don't have good memory setting !", libname, Plugin_Info.Version);
            CloseInputPlugin();
            return FALSE;
          }
#endif
        }
        else
        {
          DisplayError("%s dll is a %d version of input plugin. But this emulator needs version 1 !", libname, Plugin_Info.Version);
          CloseInputPlugin();
          return FALSE;
        }
      }
      else
      {
        DisplayError("%s dll ins't a input plugin !", libname);
        CloseInputPlugin();
        return FALSE;
      }
    }
    else
    {
      DisplayError("%s dll seem to be a wrong input dll !", libname);
      CloseInputPlugin();
      return FALSE;
    }
	}
	else
	{
		DisplayError("%s dll missing or invalid. Please check your directory for the existence of this file and use DirectX 7.", libname);
		CloseInputPlugin();
    return FALSE;
	}

  return TRUE;
}

void CloseInputPlugin()
{
  if(INPUT_DllClose)
    INPUT_DllClose();

  if(hinstLibInput)
    FreeLibrary(hinstLibInput); 
  hinstLibInput = NULL;
  
  INPUT_RomOpen = NULL;
  INPUT_DllClose = NULL;
  INPUT_DllConfig = NULL;
  INPUT_GetDllInfo = NULL;
  INPUT_Initialize    = NULL;
  INPUT_ApplicationSyncAcquire = NULL;
  INPUT_CheckController = NULL;
}