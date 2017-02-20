/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Dll_Video.c                                                          |
 |  Copyright (C) 2001 Joel Middendorf, <schibo@emulation64.com>                |
 |                                                                              |
 |  This program is free software; you can redistribute it and/or               |
 |  modify it under the terms of the GNU General Public License                 |
 |  as published by the Free Software Foundation; either version 2              |
 |  of the License, or (at your option) any later version.                      |
 |                                                                              |
 |  This program is distributed in the hope that it will be useful,             |
 |  but WITHOUT ANY WARRANTY; without even the implied warranty of              |
 |  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               |
 |  GNU General Public License for more details.                                |
 |                                                                              |
 |  You should have received a copy of the GNU General Public License           |
 |  along with this program; if not, write to the Free Software                 |
 |  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. |
 |                                                                              |
 |  To contact the author:  Schibo and Rice                                     |
 |  email      : schibo@emulation64.com, dyangchicago@yahoo.com                 |
 |  paper mail :                                                                |
 |______________________________________________________________________________|
*/

#include <windows.h>
#include "..\globals.h"
#include "..\memory.h"
#include "registry.h"
#include "DLL_Video.h"
#include "..\romlist.h"
#include "wingui.h"

uint16 GfxPluginVersion;
HINSTANCE   hinstLibVideo = NULL;

BOOL   (__cdecl* _VIDEO_InitiateGFX)(GFX_INFO) = NULL;
void   (__cdecl* _VIDEO_ProcessDList)(void) = NULL;
void   (__cdecl* _VIDEO_RomOpen)(void) = NULL;
void   (__cdecl* _VIDEO_RomClosed)(void) = NULL;
void   (__cdecl* _VIDEO_DllClose)() = NULL;
void   (__cdecl* _VIDEO_UpdateScreen)() = NULL;
void   (__cdecl* _VIDEO_GetDllInfo )( PLUGIN_INFO *) = NULL;
void   (__cdecl* _VIDEO_ExtraChangeResolution)(HWND, long, HWND) = NULL;
void   (__cdecl* _VIDEO_DllConfig)(HWND hParent) = NULL;
void   (__cdecl* _VIDEO_Test )(HWND) = NULL;
void   (__cdecl* _VIDEO_About)(HWND) = NULL;
void   (__cdecl* _VIDEO_MoveScreen)(int, int) = NULL;
void   (__cdecl* _VIDEO_DrawScreen)(void) = NULL;
void   (__cdecl* _VIDEO_ViStatusChanged)(void) = NULL;
void   (__cdecl* _VIDEO_ViWidthChanged)(void) = NULL;
void   (__cdecl* _VIDEO_ChangeWindow)(int) = NULL;

// For spec 1.3
void   (__cdecl* _VIDEO_ChangeWindow_1_3)(void) = NULL;
void	  (__cdecl* _VIDEO_CaptureScreen)(char * Directory) = NULL;
void	  (__cdecl* _VIDEO_ProcessRDPList)(void) = NULL;
void	  (__cdecl* _VIDEO_ShowCFB)(void) = NULL;

// Used when selecting plugins
void   (__cdecl* _VIDEO_Under_Selecting_Test )(HWND) = NULL;
void   (__cdecl* _VIDEO_Under_Selecting_About)(HWND) = NULL;

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
        _VIDEO_GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *))       GetProcAddress(hinstLibVideo, "GetDllInfo");

        if(_VIDEO_GetDllInfo != NULL)
        {
            PLUGIN_INFO Plugin_Info;
            ZeroMemory(&Plugin_Info, sizeof(Plugin_Info));

            VIDEO_GetDllInfo(&Plugin_Info);
			GfxPluginVersion = Plugin_Info.Version;

            if(Plugin_Info.Type == PLUGIN_TYPE_GFX)	// Check if this is a video plugin
            {
                _VIDEO_DllClose          = (void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "CloseDLL");
                _VIDEO_ExtraChangeResolution= (void   (__cdecl*)(HWND, long, HWND)) GetProcAddress(hinstLibVideo, "ChangeWinSize");
                _VIDEO_Test             =	(void (__cdecl*)(HWND))       GetProcAddress(hinstLibVideo, "DllTest");
                _VIDEO_About			=	(void (__cdecl*)(HWND))       GetProcAddress(hinstLibVideo, "DllAbout");
                _VIDEO_DllConfig        =	(void (__cdecl*)(HWND))       GetProcAddress(hinstLibVideo, "DllConfig");
                _VIDEO_MoveScreen       =	(void (__cdecl*)(int, int))   GetProcAddress(hinstLibVideo, "MoveScreen");
                _VIDEO_DrawScreen		=	(void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "DrawScreen");
				_VIDEO_ViStatusChanged	=	(void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "ViStatusChanged");
				_VIDEO_ViWidthChanged	=	(void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "ViWidthChanged");
                _VIDEO_InitiateGFX      =   (BOOL (__cdecl*)(GFX_INFO))   GetProcAddress(hinstLibVideo, "InitiateGFX");
                _VIDEO_RomOpen          =   (void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "RomOpen");
				_VIDEO_RomClosed        =   (void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "RomClosed");
                _VIDEO_ProcessDList     =   (void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "ProcessDList");
                _VIDEO_UpdateScreen     =   (void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "UpdateScreen");
                _VIDEO_ChangeWindow     =   (void (__cdecl*)(int))        GetProcAddress(hinstLibVideo, "ChangeWindow");

				// for spec 1.3
				_VIDEO_ChangeWindow_1_3 =   (void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "ChangeWindow");
                _VIDEO_CaptureScreen	=   (void (__cdecl*)(char *))     GetProcAddress(hinstLibVideo, "CaptureScreen");
                _VIDEO_ProcessRDPList	=   (void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "ProcessRDPList");
                _VIDEO_ShowCFB			=   (void (__cdecl*)(void))       GetProcAddress(hinstLibVideo, "ShowCFB");

                return(TRUE);
            }
        }
    }
    return FALSE;
}


void VIDEO_GetDllInfo(PLUGIN_INFO * Plugin_Info)
{
    if (_VIDEO_GetDllInfo != NULL)
    {
        __try{
            _VIDEO_GetDllInfo(Plugin_Info);
        }
        __except(NULL, EXCEPTION_EXECUTE_HANDLER){
            DisplayError("GettDllInfo Failed.");
        }
    }
}

BOOL VIDEO_InitiateGFX(GFX_INFO Gfx_Info)
{
    __try{
        _VIDEO_InitiateGFX(Gfx_Info);
    }
    __except(NULL,EXCEPTION_EXECUTE_HANDLER){
//        DisplayError("Cannot Initialize Graphics");
    }
    return(1); //why not for now..
}

void VIDEO_ProcessDList(void)
{
    // try/except is handled from the call
    if (_VIDEO_ProcessDList != NULL)
        _VIDEO_ProcessDList();
}

void VIDEO_RomOpen(void)
{
    if (_VIDEO_RomOpen != NULL)
    {
        __try{
            _VIDEO_RomOpen();
        }
        __except(NULL, EXCEPTION_EXECUTE_HANDLER){
            DisplayError("Video RomOpen Failed.");
        }
    }
}

void VIDEO_RomClosed(void)
{
    if (_VIDEO_RomClosed != NULL)
    {
        __try{
            _VIDEO_RomClosed();
        }
        __except(NULL, EXCEPTION_EXECUTE_HANDLER){
            DisplayError("Video RomClosed Failed.");
        }
    }
}

void VIDEO_ChangeWindow(int window)
{
	if ( GfxPluginVersion == 0x0103 )
	{
		if (_VIDEO_ChangeWindow_1_3 != NULL)
		{
			__try{
				_VIDEO_ChangeWindow_1_3();
			}
			__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DisplayError("VIDEO ChangeWindow failed");
			}
		}
	}
	else
	{
		if (_VIDEO_ChangeWindow != NULL)
		{
			__try{
				_VIDEO_ChangeWindow(window);
			}
			__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DisplayError("VIDEO ChangeWindow failed");
			}
		}
	}
}

void VIDEO_DllClose()
{
    if (_VIDEO_DllClose != NULL)
    {
        __try{
            _VIDEO_DllClose();
        }
        __except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("VIDEO DllClose failed");
        }
    }
}

void CloseVideoPlugin()
{
	VIDEO_DllClose();

	if(hinstLibVideo)
		FreeLibrary(hinstLibVideo); 
	
	hinstLibVideo = NULL;
  
	_VIDEO_InitiateGFX = NULL;
	_VIDEO_ProcessDList = NULL;
	_VIDEO_RomOpen = NULL;
	_VIDEO_DllClose = NULL;
	_VIDEO_DllConfig = NULL;
	_VIDEO_GetDllInfo = NULL;
	_VIDEO_UpdateScreen = NULL;
	_VIDEO_ExtraChangeResolution = NULL;

	_VIDEO_ChangeWindow = NULL;
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

void VIDEO_UpdateScreen(void)
{
	if (_VIDEO_UpdateScreen != NULL)
        __try{
            _VIDEO_UpdateScreen();
        }
        __except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("Video UpdateScreen failed.");
        }
}

void VIDEO_DrawScreen(void)
{
	if (_VIDEO_DrawScreen != NULL)
        __try{
            _VIDEO_DrawScreen();
    }
    __except(NULL,EXCEPTION_EXECUTE_HANDLER){
        DisplayError("Video DrawScreen failed.");
    }
    
    VIDEO_UpdateScreen();
}

void VIDEO_ExtraChangeResolution(HWND hParent, long res, HWND hChild)
{
	int statusbarheight;
	if (_VIDEO_ExtraChangeResolution != NULL)
	{
		_VIDEO_ExtraChangeResolution(hParent,res,hChild);
	}
	else
	{
		RECT rect, rectstatusbar;
		GetWindowRect(hParent, &rect);
		if( hStatusBar != NULL )
		{
			GetWindowRect(hStatusBar, &rectstatusbar);
			statusbarheight = rectstatusbar.bottom - rectstatusbar.top+1;
		}
		else
		{
			statusbarheight = 0;
		}
		rect.right=res+rect.left;
		switch( res )
		{
		case 320:
			rect.bottom = 240+rect.top+statusbarheight;
			break;
		case 640:
			rect.bottom = 480+rect.top+statusbarheight;
			break;
		case 800:
			rect.bottom = 600+rect.top+statusbarheight;
			break;
		case 1024:
			rect.bottom = 768+rect.top+statusbarheight;
			break;
		case 1280:
			rect.bottom = 1024+rect.top+statusbarheight;
			break;
		case 1600:
			rect.bottom = 1200+rect.top+statusbarheight;
			break;
		case 1920:
			rect.bottom = 1440+rect.top+statusbarheight;
			break;
		default:
			rect.bottom = 480+rect.top+statusbarheight;
			break;
		}
		AdjustWindowRect( &rect, WS_OVERLAPPEDWINDOW , TRUE);
		SetWindowPos(hParent, HWND_TOP, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW);

		//DisplayError("%s: does not support changing resolution.", "Video Plugin");
	}

	DockStatusBar();
	NewRomList_ListViewChangeWindowRect();
	InvalidateRect(hwnd, NULL, TRUE);

	if( Emu_Is_Running )
	{
		NewRomList_ListViewHideHeader(hwndRomList);
		ShowWindow(hwndRomList, SW_HIDE);
	}
}

void VIDEO_ViStatusChanged(void)
{
	if (_VIDEO_ViStatusChanged != NULL)
	{
        __try{
            _VIDEO_ViStatusChanged();
        }
        __except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("Exception in ViStatusChanged");
        }
	}
}

void VIDEO_ViWidthChanged(void)
{
	if( _VIDEO_ViWidthChanged != NULL )
	{
		__try{
            _VIDEO_ViWidthChanged();
        }
        __except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("Exception in ViWidthChanged");
        }
	}
}


// changes for spec 1.3
void VIDEO_CaptureScreen(char * Directory)
{
	if( _VIDEO_CaptureScreen != NULL )
	{
		__try{
            _VIDEO_CaptureScreen(Directory);
        }
        __except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("Exception in Capture Screen");
        }
	}
}

void VIDEO_ProcessRDPList(void)
{
	if( _VIDEO_ProcessRDPList != NULL )
	{
		__try{
            _VIDEO_ProcessRDPList();
        }
        __except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("Exception in Processing RDP List");
        }
	}
}

void VIDEO_ShowCFB(void)
{
	if( _VIDEO_ShowCFB != NULL )
	{
		__try{
            _VIDEO_ShowCFB();
        }
        __except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("Exception in VIDEO_ShowCFB");
        }
	}
}

// Used when selecting plugins
void VIDEO_Under_Selecting_About(HWND hParent)
{
	if (_VIDEO_Under_Selecting_About != NULL)
	{
		_VIDEO_Under_Selecting_About(hParent);
	}
	else
	{
		DisplayError("%s: About information is not available for this plug-in.", "Video Plugin");
	}
}

void VIDEO_Under_Selecting_Test(HWND hParent)
{
	if (_VIDEO_Under_Selecting_Test != NULL)
	{
		_VIDEO_Under_Selecting_Test(hParent);
	}
	else
	{
		DisplayError("%s: Test function is not available for this plug-in.", "Video Plugin");
	}
}
