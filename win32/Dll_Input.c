#include <windows.h>
#include "../globals.h"
#include "registry.h"
#include "../dllsrc/controller.h"
#include "DLL_Input.h"


//////////////////////////////////////////////////////////////////////
// Input plugin
//////////////////////////////////////////////////////////////////////
HINSTANCE	hinstControllerPlugin = NULL;

void   (__cdecl* _CONTROLLER_CloseDLL)(void) = NULL;
void   (__cdecl* _CONTROLLER_ControllerCommand) ( int Control, BYTE * Command) = NULL;
void   (__cdecl* _CONTROLLER_DllAbout)(HWND) = NULL;
void   (__cdecl* _CONTROLLER_DllConfig)(HWND) = NULL;
void   (__cdecl* _CONTROLLER_DllTest)(HWND) = NULL;
void   (__cdecl* _CONTROLLER_GetDllInfo )( PLUGIN_INFO *) = NULL;
void   (__cdecl* _CONTROLLER_GetKeys)(int Control, BUTTONS * Keys ) = NULL;
void   (__cdecl* _CONTROLLER_InitiateControllers)(HWND hMainWindow, CONTROL Controls[4]) = NULL;
void   (__cdecl* _CONTROLLER_ReadController)( int Control, BYTE * Command ) = NULL;
void   (__cdecl* _CONTROLLER_RomClosed)(void) = NULL;
void   (__cdecl* _CONTROLLER_RomOpen)(void) = NULL;
void   (__cdecl* _CONTROLLER_WM_KeyDown)( WPARAM wParam, LPARAM lParam ) = NULL;
void   (__cdecl* _CONTROLLER_WM_KeyUp)( WPARAM wParam, LPARAM lParam ) = NULL;

BOOL LoadControllerPlugin(char* libname)
{
	if (hinstControllerPlugin != NULL)
	{
		CloseControllerPlugin();
	}

	// Load the input plugin DLL
	hinstControllerPlugin = LoadLibrary(libname);

	if (hinstControllerPlugin != NULL)	// Check if DLL is loaded successfully
	{
		// Get function address _CONTROLLER_GetDllInfo
		_CONTROLLER_GetDllInfo = (void (__cdecl*)(PLUGIN_INFO *)) GetProcAddress(hinstControllerPlugin, "GetDllInfo");

		if(_CONTROLLER_GetDllInfo)	// Is there the function _CONTROLLER_GetDllInfo in the Input plugin DLL
		{
			PLUGIN_INFO Plugin_Info;
			ZeroMemory(&Plugin_Info, sizeof(Plugin_Info));

			_CONTROLLER_GetDllInfo(&Plugin_Info);

			if(Plugin_Info.Type == PLUGIN_TYPE_CONTROLLER)
			{
				if(Plugin_Info.Version == CONTROLLER_VERSION)
				{
 					_CONTROLLER_CloseDLL			= (void (__cdecl*)(void)) GetProcAddress(hinstControllerPlugin, "CloseDLL");
					_CONTROLLER_ControllerCommand	= (void (__cdecl*)(int Control, BYTE* Command))	GetProcAddress(hinstControllerPlugin, "ControllerCommand");
					_CONTROLLER_DllAbout			= (void (__cdecl*)(HWND)) GetProcAddress(hinstControllerPlugin, "DllAbout");
					_CONTROLLER_DllConfig			= (void (__cdecl*)(HWND)) GetProcAddress(hinstControllerPlugin, "DllConfig");
					_CONTROLLER_DllTest				= (void (__cdecl*)(HWND)) GetProcAddress(hinstControllerPlugin, "DllTest");
					_CONTROLLER_GetDllInfo			= (void (__cdecl*)( PLUGIN_INFO *))	GetProcAddress(hinstControllerPlugin, "GetDllInfo");
					_CONTROLLER_GetKeys				= (void (__cdecl*)(int Control, BUTTONS * Keys )) GetProcAddress(hinstControllerPlugin, "GetKeys");
					_CONTROLLER_InitiateControllers	= (void (__cdecl*)(HWND hMainWindow, CONTROL Controls[4])) GetProcAddress(hinstControllerPlugin, "InitiateControllers");
					_CONTROLLER_ReadController		= (void (__cdecl*)( int Control, BYTE * Command )) GetProcAddress(hinstControllerPlugin, "ReadController");
					_CONTROLLER_RomClosed			= (void (__cdecl*)(void)) GetProcAddress(hinstControllerPlugin, "RomClosed");
					_CONTROLLER_RomOpen				= (void (__cdecl*)(void)) GetProcAddress(hinstControllerPlugin, "RomOpen");
					_CONTROLLER_WM_KeyDown			= (void (__cdecl*)( WPARAM wParam, LPARAM lParam )) GetProcAddress(hinstControllerPlugin, "WM_KeyDown");
					_CONTROLLER_WM_KeyUp			= (void (__cdecl*)( WPARAM wParam, LPARAM lParam )) GetProcAddress(hinstControllerPlugin, "WM_KeyUp");
				}
//				else
//				{
//					DisplayError("%s dll is a %d version of input plugin. This emulator needs version 1!", libname, Plugin_Info.Version);
//					return FALSE;
//				}
			} 
			else 
			{
				DisplayError("%s dll ins't a input plugin !", libname);
				return FALSE;
			}
		} 
		else 
		{
			DisplayError("%s dll seem to be a wrong input dll !", libname);
			return FALSE;
		}
	} 
	else 
	{
		DisplayError("LoadControllerPlugin(): %s error. Please install DirectX8 and verify that this file is in Plugin directory.", libname);
		return FALSE;
	}

  return TRUE;
}

void CloseControllerPlugin()
{
	if(_CONTROLLER_RomClosed)
	{
		_CONTROLLER_RomClosed();
	}

	if(_CONTROLLER_CloseDLL)
	{
		_CONTROLLER_CloseDLL();
	}

	/*
	if(_CONTROLLER_RomClosed)
	{
		_CONTROLLER_RomClosed();
	}
	*/

	if(hinstControllerPlugin)
	{
		FreeLibrary(hinstControllerPlugin); 
	}
	
	hinstControllerPlugin = NULL;
 
	_CONTROLLER_CloseDLL			= NULL;
	_CONTROLLER_ControllerCommand	= NULL;
	_CONTROLLER_DllAbout			= NULL;
	_CONTROLLER_DllConfig			= NULL;
	_CONTROLLER_DllTest				= NULL;
	_CONTROLLER_GetDllInfo			= NULL;
	_CONTROLLER_GetKeys				= NULL;
	_CONTROLLER_InitiateControllers	= NULL;
	_CONTROLLER_ReadController		= NULL;
	_CONTROLLER_RomClosed			= NULL;
	_CONTROLLER_RomOpen				= NULL;
	_CONTROLLER_WM_KeyDown			= NULL;
	_CONTROLLER_WM_KeyUp			= NULL;
}

void CONTROLLER_CloseDLL(void)
{
	if (_CONTROLLER_CloseDLL != NULL)
	{
		_CONTROLLER_CloseDLL();
	}
}

void CONTROLLER_ControllerCommand ( int _Control, BYTE *_Command)
{
	if (_CONTROLLER_ControllerCommand != NULL)
	{
		_CONTROLLER_ControllerCommand(_Control, _Command);
	}
}

void CONTROLLER_DllAbout(HWND _hWnd)
{
	if (_CONTROLLER_DllAbout != NULL)
	{
		_CONTROLLER_DllAbout(_hWnd);
	}
}

void CONTROLLER_DllConfig(HWND _hWnd)
{
	if (_CONTROLLER_DllConfig != NULL)
	{
		_CONTROLLER_DllConfig(_hWnd);
	}
}

void CONTROLLER_DllTest(HWND _hWnd)
{
	if (_CONTROLLER_DllTest != NULL)
	{
		_CONTROLLER_DllTest(_hWnd);
	}
}

void CONTROLLER_GetDllInfo( PLUGIN_INFO *_plugin)
{
	if (_CONTROLLER_GetDllInfo != NULL)
	{
		_CONTROLLER_GetDllInfo(_plugin);
	}
}

void CONTROLLER_GetKeys(int _Control, BUTTONS * _Keys )
{
	if (_CONTROLLER_GetKeys != NULL)
	{
		_CONTROLLER_GetKeys(_Control, _Keys);
	}
}

void CONTROLLER_InitiateControllers(HWND _hMainWindow,
CONTROL _Controls[4])
{
        if (_CONTROLLER_InitiateControllers != NULL)
        {
                _CONTROLLER_InitiateControllers(_hMainWindow,
_Controls);
        }

        
        // Add mempak support in 1964 by configure the control 4 as mempak
        // no matter if the control plugin support it or not
        _Controls[0].Plugin = PLUGIN_MEMPAK;
        _Controls[1].Present = FALSE;
        _Controls[2].Present = FALSE;
        _Controls[3].Present = FALSE;
}
void CONTROLLER_ReadController( int _Control, BYTE * _Command ) 
{
	if (_CONTROLLER_ReadController != NULL)
	{
		_CONTROLLER_ReadController(_Control, _Command);
	}
}

void CONTROLLER_RomClosed(void) 
{
	if (_CONTROLLER_RomClosed != NULL)
	{
		_CONTROLLER_RomClosed();
	}
}

void CONTROLLER_RomOpen(void) 
{
	if (_CONTROLLER_RomOpen != NULL)
	{
		_CONTROLLER_RomOpen();
	}
}

void CONTROLLER_WM_KeyDown( WPARAM _wParam, LPARAM _lParam ) 
{
	if (_CONTROLLER_WM_KeyDown != NULL)
	{
		_CONTROLLER_WM_KeyDown(_wParam, _lParam);
	}
}

void CONTROLLER_WM_KeyUp( WPARAM _wParam, LPARAM _lParam )
{
	if (_CONTROLLER_WM_KeyUp != NULL)
	{
		_CONTROLLER_WM_KeyUp(_wParam, _lParam);
	}
}
