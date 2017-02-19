/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Emulator for Nintendo 64 console system                              |
 |  Copyright (C) 2000  Joel Middendorf  schibo@emuhq.com                       |
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
 |  To contact the author:                                                      |
 |  email      : schibo@emuhq.com                                               |
 |  paper mail :                                                                |
 |______________________________________________________________________________|


The project is a direct documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar :). Most source
code comments are taken directly from anarko's n64toolkit with consent and are 
the property of anarko.
*/

#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include "wingui.h"
#include <shellapi.h>
#include "..\globals.h"
#include "DLL_Video.h"
#include "DLL_Audio.h"
#include "DLL_Input.h"
#include "registry.h"

extern HANDLE CPUThreadHandle;

int ActiveApp;

#ifdef WINDEBUG_1964
#include "windebug.h"
#endif

int repeat;
char szBaseWindowTitle[] = "1964 0.5.0";

extern void r4300i_Reset();
extern void RunEmulator(unsigned _int32 WhichCore);
extern BOOL CloseCPUThread();

extern void ReadConfiguration();
extern void WriteConfiguration();


void Pause();
void Kill();
void Play();
void Reset();
void OpenROM();
void GetPluginDir(char* Directory);

HACCEL hAccelTable = (HACCEL)"WINGUI_ACC";
BOOL fFreeResult;

//---------------------------------------------------------------------------------------

void LoadPlugins()
{
	char AudioPath[_MAX_PATH];
	char VideoPath[_MAX_PATH];
	char InputPath[_MAX_PATH];
	char StartPath[_MAX_PATH];

	GetPluginDir(StartPath);
	
	strcpy(AudioPath, StartPath);
	if (strcmp(gRegSettings.AudioPlugin, "") == 0)
	{
		strcpy(gRegSettings.AudioPlugin, "steb_aud.dll");
		strcat(AudioPath, gRegSettings.AudioPlugin);
	}
	else
		strcat(AudioPath, gRegSettings.AudioPlugin);
	LoadAudioPlugin(AudioPath);

	strcpy(VideoPath, StartPath);
	if (strcmp(gRegSettings.VideoPlugin, "") == 0)
	{
		strcpy(gRegSettings.VideoPlugin, "1964ogl.dll");
		strcat(VideoPath, gRegSettings.VideoPlugin);
	}
	else
		strcat(VideoPath, gRegSettings.VideoPlugin);
	LoadVideoPlugin(VideoPath);
	
	
	strcpy(InputPath, StartPath);
	if (strcmp(gRegSettings.InputPlugin, "") == 0)
	{
		strcpy(gRegSettings.InputPlugin, "NooTe_DI.dll");
		strcat(InputPath, gRegSettings.InputPlugin);
	}
	else											
		strcat(InputPath, gRegSettings.InputPlugin);
	LoadInputPlugin(InputPath);

	if (!(AUDIO_Initialize)(hwnd))	DisplayError("No sound availiable");
}

//---------------------------------------------------------------------------------------


void FreePlugins()
{
  CloseInputPlugin();
  CloseAudioPlugin();
  CloseVideoPlugin();
}

//---------------------------------------------------------------------------------------

extern GFX_INFO Gfx_Info;
extern void  CheckInterrupts(void);
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	MSG msg;
	Rom_Loaded			  = FALSE;

#ifdef WINDEBUG_1964
	RUN_TO_ADDRESS_ACTIVE = TRUE;
	OpCount = 0;
	NextClearCode = 250;
	BreakAddress = -1;
#endif

	hInst = hInstance;
	LoadString(hInstance, IDS_MAINDISCLAIMER, MainDisclaimer, sizeof(MainDisclaimer));

	_getcwd( AppPath, PATH_LEN );

	if (hPrevInstance)
		return FALSE;

	hwnd = InitWin98UI(hInstance, nCmdShow);
		if (hwnd == NULL)
			DisplayError("Could not get a windows handle.");
		

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	Gfx_Info.hWnd					= hwnd;
	Gfx_Info.hStatusBar				= NULL;
	Gfx_Info.MemoryBswaped			= TRUE;
	Gfx_Info.RDRAM					= (__int8*)&RDRAM[0];
	Gfx_Info.DMEM					= (__int8*)&SP_DMEM;
	Gfx_Info.IMEM					= (__int8*)&SP_IMEM;
	Gfx_Info.MI_INTR_RG				= &MI_INTR_REG_R;
	Gfx_Info.DPC_START_RG			= &DPC_START_REG;
	Gfx_Info.DPC_END_RG				= &DPC_END_REG;
	Gfx_Info.DPC_CURRENT_RG			= &DPC_CURRENT_REG;
	Gfx_Info.DPC_STATUS_RG			= &DPC_STATUS_REG;
	Gfx_Info.DPC_CLOCK_RG			= &DPC_CLOCK_REG;
	Gfx_Info.DPC_BUFBUSY_RG			= &DPC_BUFBUSY_REG;
	Gfx_Info.DPC_PIPEBUSY_RG		= &DPC_PIPEBUSY_REG;
	Gfx_Info.DPC_TMEM_RG			= &DPC_TMEM_REG;

	Gfx_Info.VI_STATUS_RG			= &VI_STATUS_REG;
	Gfx_Info.VI_ORIGIN_RG			= &VI_ORIGIN_REG;
	Gfx_Info.VI_WIDTH_RG			= &VI_WIDTH_REG;
	Gfx_Info.VI_INTR_RG				= &VI_INTR_REG;
	Gfx_Info.VI_V_CURRENT_LINE_RG	= &VI_CURRENT_REG;
	Gfx_Info.VI_TIMING_RG			= &VI_BURST_REG;
	Gfx_Info.VI_V_SYNC_RG			= &VI_V_SYNC_REG;
	Gfx_Info.VI_H_SYNC_RG			= &VI_H_SYNC_REG;
	Gfx_Info.VI_LEAP_RG				= &VI_LEAP_REG;
	Gfx_Info.VI_H_START_RG			= &VI_H_START_REG;
	Gfx_Info.VI_V_START_RG			= &VI_V_START_REG;
	Gfx_Info.VI_V_BURST_RG			= &VI_V_BURST_REG;
	Gfx_Info.VI_X_SCALE_RG			= &VI_X_SCALE_REG;
	Gfx_Info.VI_Y_SCALE_RG			= &VI_Y_SCALE_REG;
	Gfx_Info.CheckInterrupts		= CheckInterrupts;

	r4300i_Init();
	ReadConfiguration();				//System registry settings

	LoadPlugins();
	INPUT_Initialize(hwnd, hInst);	//Input DLL Initialization
	VIDEO_InitiateGFX(Gfx_Info);		//GFX DLL Initialization

	Rom_Loaded = TRUE;

#ifdef WINDEBUG_1964 	
	OpenDebugger();
#endif

_HOPPITY:

	if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
		if( !GetMessage( &msg, NULL, 0, 0 ) ) 
			return msg.wParam;
		else if (!TranslateAccelerator (hwnd, hAccelTable, &msg))
		{
			TranslateMessage (&msg) ;
			 DispatchMessage (&msg) ;
		}

goto _HOPPITY;
}

//---------------------------------------------------------------------------------------

void __cdecl DisplayError (char * Message, ...) {
	char Msg[400];
	va_list ap;

#ifdef WINDEBUG_1964
	RefreshDebugger();
#endif

	va_start( ap, Message );
	vsprintf( Msg, Message, ap );
	va_end( ap );

	MessageBox(NULL,Msg,"Error",MB_OK|MB_ICONERROR);
}

//---------------------------------------------------------------------------------------

BOOL WinLoadRom()
{
	OPENFILENAME ofn;
	char szFileName[MAXFILENAME];
	char szFileTitle[MAXFILENAME];

	memset(&szFileName,0,sizeof(szFileName));
	memset(&szFileTitle, 0, sizeof(szFileTitle));

    ofn.lStructSize			= sizeof(OPENFILENAME);
    ofn.hwndOwner			= hwnd;
    ofn.lpstrFilter			= "N64 ROMs (*.ZIP, *.V64, *.BIN, *.ROM)\0*.ZIP;*.V64;*.BIN;*.ROM\0All Files (*.*)\0*.*\0";
	ofn.lpstrCustomFilter	= NULL;
    ofn.nMaxCustFilter		= 0;
    ofn.nFilterIndex		= 1;
    ofn.lpstrFile			= szFileName;
    ofn.nMaxFile			= MAXFILENAME;
    ofn.lpstrInitialDir		= gRegSettings.ROMPath;
    ofn.lpstrFileTitle		= szFileTitle;
    ofn.nMaxFileTitle		= MAXFILENAME;
    ofn.lpstrTitle			= "Open Image";
    ofn.lpstrDefExt			= "TXT";
    ofn.Flags				= OFN_HIDEREADONLY;

    if (!GetOpenFileName ((LPOPENFILENAME)&ofn)) return FALSE;

	lstrcpy(szWindowTitle, szBaseWindowTitle);
	lstrcat(szWindowTitle, " - ");
	lstrcat(szWindowTitle, szFileTitle);

	ReadRomData(szFileName);
	EnableMenuItem(hMenu, ID_OPENROM, MF_GRAYED);
	SetWindowText(hwnd, szWindowTitle);

	pc = 0xA4000040;

	Rom_Loaded = TRUE;

#ifdef WINDEBUG_1964
	MessageBox(hwnd, "Rom Loaded. Click the Rom->Play menu to begin\n", "OK!",MB_OK);
#endif

	_getcwd( gRegSettings.ROMPath, PATH_LEN );
	WriteConfiguration();
	return TRUE;	
}

//---------------------------------------------------------------------------------------

HWND InitWin98UI(HANDLE hInstance, int nCmdShow)
{
	WNDCLASS	wc;

	RECT winrect = {0,0,640,480};
	int width, height;

	AdjustWindowRect(	&winrect, WS_OVERLAPPEDWINDOW /*WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME */, TRUE);

	width = winrect.right - winrect.left;
	height = winrect.bottom - winrect.top;

    wc.style  = CS_SAVEBITS;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = (HINSTANCE)hInstance;
    wc.hIcon = LoadIcon((HINSTANCE)hInstance, MAKEINTRESOURCE(IDI_ICON2));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject (BLACK_BRUSH); //hBrush;
    wc.lpszMenuName =  "WINGUI_MENU";

    wc.lpszClassName = "WinGui";

	RegisterClass(&wc);

    hwnd = CreateWindow(
        "WinGui",
        szBaseWindowTitle,
		WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        NULL,
        NULL,
        (HINSTANCE)hInstance,
        NULL
    );

	if (hwnd == NULL) {
		MessageBox(NULL, "CreateWindow() failed: Cannot create a window.", "Error", MB_OK);
		return (NULL);
	}

    hAccTable = LoadAccelerators(hInst, "WINGUI_ACC");

#ifndef WINDEBUG_1964
	hMenu = GetMenu (hwnd);
	EnableMenuItem(hMenu, ID_RUNTO, MF_GRAYED);
	EnableMenuItem(hMenu, ID_DEBUGGER_OPEN,	MF_GRAYED);
	EnableMenuItem(hMenu, ID_DEBUGGER_CLOSE,MF_GRAYED);
#endif

	return hwnd;
}

//---------------------------------------------------------------------------------------

void GetPluginDir(char* Directory) {
	char path_buffer[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR];
	char fname[_MAX_FNAME],ext[_MAX_EXT];
	GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));
	_splitpath(path_buffer,drive,dir,fname,ext);
	strcpy(Directory,drive);
	strcat(Directory,dir);
	strcat(Directory,"Plugin\\");
}

//---------------------------------------------------------------------------------------

void   (__cdecl* GetDllInfo )( PLUGIN_INFO *) = NULL;
void   (__cdecl* DLL_About)(HWND) = NULL;
LRESULT APIENTRY PluginsDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
	PLUGIN_INFO Plugin_Info;
	HINSTANCE	hinstLib = NULL;
	WIN32_FIND_DATA libaa;
	int ComboItemNum;

	int h=0, i=0, j=0, bDONE = 0;

	HANDLE FindFirst;
	char libname[300];
	char PluginName[300];
	char StartPath[_MAX_PATH];
	char SearchPath[_MAX_PATH];
	int index;

	GetPluginDir(StartPath);
	strcpy(SearchPath, StartPath);
	strcat(SearchPath, "*.dll");

	switch (message) {
        int KeepLooping = 1;
	
		case WM_INITDIALOG:
			FindFirst = FindFirstFile(SearchPath, &libaa);

			if (FindFirst == INVALID_HANDLE_VALUE)
			{ 
				DisplayError("No plugins detected. Plugins belong in Plugin subfolder of 1964 folder."); 
				return(FALSE); 
			}
			/* Reset combo boxes content */
			SendDlgItemMessage(hDlg, IDC_COMBO_VIDEO, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(hDlg, IDC_COMBO_INPUT, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(hDlg, IDC_COMBO_AUDIO, CB_RESETCONTENT, 0, 0);

			/* Populate combo boxes with Plugin Info */
			while (KeepLooping)
			{
				strcpy(PluginName, StartPath);
				strcat(PluginName, libaa.cFileName);
				hinstLib = LoadLibrary(PluginName);
				
				GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *))	GetProcAddress(hinstLib, "GetDllInfo");
				GetDllInfo(&Plugin_Info);

				switch (Plugin_Info.Type)
				{
					case PLUGIN_TYPE_GFX		: 
						index = SendDlgItemMessage(hDlg, IDC_COMBO_VIDEO, CB_ADDSTRING, 0, Plugin_Info.Name);
						if ( _stricmp(libaa.cFileName, gRegSettings.VideoPlugin) == 0 )
							SendDlgItemMessage(hDlg, IDC_COMBO_VIDEO, CB_SETCURSEL, (WPARAM)index, (LPARAM)0);
						break;

					case PLUGIN_TYPE_CONTROLLER :
						index = SendDlgItemMessage(hDlg, IDC_COMBO_INPUT, CB_ADDSTRING, 0, Plugin_Info.Name);
						if ( _stricmp(libaa.cFileName, gRegSettings.InputPlugin) == 0 )
							SendDlgItemMessage(hDlg, IDC_COMBO_INPUT, CB_SETCURSEL, (WPARAM)index, (LPARAM)0);
						break;

					case PLUGIN_TYPE_AUDIO		:
						index = SendDlgItemMessage(hDlg, IDC_COMBO_AUDIO, CB_ADDSTRING, 0, Plugin_Info.Name);
						if ( _stricmp(libaa.cFileName, gRegSettings.AudioPlugin) == 0 )
							SendDlgItemMessage(hDlg, IDC_COMBO_AUDIO, CB_SETCURSEL, (WPARAM)index, (LPARAM)0);
						break;
				
				}

			
				FreeLibrary(libname);
				hinstLib = NULL;
				KeepLooping = FindNextFile(FindFirst, &libaa);
				GetDllInfo = NULL;
				*PluginName = NULL;
			}

			return (TRUE);

        case WM_COMMAND:
			switch (wParam)
			{
				case IDOK:
				{
					WriteConfiguration(); 
					EndDialog(hDlg, TRUE);
					MessageBox(NULL, "If you have switched plugins, you will need to close 1964 now and open in again. Otherwise, the emu will wig out.", "Note: Please Restart 1964", MB_ICONINFORMATION);
					LoadPlugins();
					return (TRUE);
				}
				case IDCANCEL:
                {
					EndDialog(hDlg, TRUE);
					return (TRUE);
				}
				case  IDC_DI_CONFIG: INPUT_DllConfig(hDlg);	break;
				case  IDC_DI_ABOUT : INPUT_About(hDlg);		break;
				case  IDC_DI_TEST  : INPUT_Test(hDlg);		break;

				case IDC_VID_CONFIG: VIDEO_DllConfig(hDlg);	break;
				case IDC_VID_ABOUT : VIDEO_About(hDlg);		break;
				case IDC_VID_TEST  : VIDEO_Test(hDlg);		break;

				case IDC_AUD_CONFIG: AUDIO_DllConfig(hDlg);	break;
				case IDC_AUD_ABOUT : AUDIO_About(hDlg);		break;
				case IDC_AUD_TEST  : AUDIO_Test(hDlg);		break;

            }
		
		case CBN_SELCHANGE: 
			FreeLibrary(libname);
			ComboItemNum = SendDlgItemMessage(hDlg, IDC_COMBO_VIDEO, CB_GETCURSEL, 0, 0);
			FindFirst = FindFirstFile(SearchPath, &libaa);
			while (bDONE == 0)
			{
				strcpy(PluginName, StartPath);
				strcat(PluginName, libaa.cFileName);
				hinstLib = LoadLibrary(PluginName);
				GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *))	GetProcAddress(hinstLib, "GetDllInfo");
				GetDllInfo(&Plugin_Info);
				switch (Plugin_Info.Type)
				{
				case PLUGIN_TYPE_GFX		: 
					VIDEO_Test		= (void (__cdecl*)(HWND))	GetProcAddress(hinstLib, "DllTest"); 
					VIDEO_About		= (void (__cdecl*)(HWND))	GetProcAddress(hinstLib, "DllAbout"); 
					VIDEO_DllConfig	= (void (__cdecl*)(HWND))	GetProcAddress(hinstLib, "DllConfig");
					h++;
					break;
				}
			
				if (h > ComboItemNum) bDONE = 1;
				else FindNextFile(FindFirst, &libaa);
				GetDllInfo = NULL;
				hinstLib = NULL;
			}
			bDONE = 0;
			strcpy(gRegSettings.VideoPlugin, libaa.cFileName);



			FreeLibrary(libname);
			ComboItemNum = SendDlgItemMessage(hDlg, IDC_COMBO_AUDIO, CB_GETCURSEL, 0, 0);
			FindFirst = FindFirstFile(SearchPath, &libaa);
			while (bDONE == 0)
			{
				strcpy(PluginName, StartPath);
				strcat(PluginName, libaa.cFileName);
				hinstLib = LoadLibrary(PluginName);
				GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *))	GetProcAddress(hinstLib, "GetDllInfo");
				GetDllInfo(&Plugin_Info);
				switch (Plugin_Info.Type)
				{
				case PLUGIN_TYPE_AUDIO	    : 
					AUDIO_Test		= (void (__cdecl*)(HWND))	GetProcAddress(hinstLib, "DllTest"); 
					AUDIO_About		= (void (__cdecl*)(HWND))	GetProcAddress(hinstLib, "DllAbout"); 
					AUDIO_DllConfig	= (void (__cdecl*)(HWND))	GetProcAddress(hinstLib, "DllConfig");
					j++;
					break;
				}
				if (j > ComboItemNum) bDONE = 1;
				else FindNextFile(FindFirst, &libaa);
				GetDllInfo = NULL;
				hinstLib = NULL;
			}
			bDONE = 0;
			strcpy(gRegSettings.AudioPlugin, libaa.cFileName);


			
			FreeLibrary(libname);
			ComboItemNum = SendDlgItemMessage(hDlg, IDC_COMBO_INPUT, CB_GETCURSEL, 0, 0);
			FindFirst = FindFirstFile(SearchPath, &libaa);
			while (bDONE == 0)
			{
				strcpy(PluginName, StartPath);
				strcat(PluginName, libaa.cFileName);
				hinstLib = LoadLibrary(PluginName);
				GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *))	GetProcAddress(hinstLib, "GetDllInfo");
				GetDllInfo(&Plugin_Info);
				switch (Plugin_Info.Type)
				{
				case PLUGIN_TYPE_CONTROLLER : 
					INPUT_Test		= (void (__cdecl*)(HWND))	GetProcAddress(hinstLib, "DllTest"); 
					INPUT_About		= (void (__cdecl*)(HWND))	GetProcAddress(hinstLib, "DllAbout");
					INPUT_DllConfig	= (void (__cdecl*)(HWND))	GetProcAddress(hinstLib, "DllConfig");
					i++;
					break;
				}
				if (i > ComboItemNum) bDONE = 1;
				else FindNextFile(FindFirst, &libaa);
				GetDllInfo = NULL;
				hinstLib = NULL;
			}
			strcpy(gRegSettings.InputPlugin, libaa.cFileName);

			
    }
    return (FALSE);
}

//---------------------------------------------------------------------------------------

long FAR PASCAL MainWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    static PAINTSTRUCT ps;
	static int ok = 0;
	RECT ClientRect;
	switch (message) 
	{
		case WM_PAINT:
			BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			return 0;

		case WM_ACTIVATEAPP:
			ActiveApp = wParam;
			if(INPUT_ApplicationSyncAcquire)
				INPUT_ApplicationSyncAcquire(hWnd,ActiveApp);
			break;

		case WM_SETFOCUS: 
			ActiveApp = wParam;
			if(INPUT_ApplicationSyncAcquire)
				INPUT_ApplicationSyncAcquire(hWnd,ActiveApp);
			break;

		case WM_MOVE:
			if (VIDEO_MoveScreen != NULL)
			if (ok == 1)
			{
				GetClientRect(hwnd, &ClientRect);
				VIDEO_MoveScreen(ClientRect.left, ClientRect.top);
			}
			ok = 1;


		case WM_COMMAND:
            
			switch (wParam) 
			{
				case ID_ROM_RESET	:	Reset();		break;
				case ID_ROM_START	:	Play();			break;
				case ID_ROM_STOP	:	Pause();		break;
#ifdef WINDEBUG_1964
				case ID_RUNTO		:	Get_HexInput();	break;
#endif
                case ID_OPENROM: OpenROM(); break;
				case ID_ABOUT:
                    DialogBox(hInst, "ABOUTBOX", hWnd, (DLGPROC)About);
                    break;
		
				case IDM_320_240	: if(VIDEO_ExtraChangeResolution) VIDEO_ExtraChangeResolution(hWnd, 320, NULL); break;
				case IDM_640_480	: if(VIDEO_ExtraChangeResolution) VIDEO_ExtraChangeResolution(hWnd, 640, NULL); break;
				case IDM_800_600	: if(VIDEO_ExtraChangeResolution) VIDEO_ExtraChangeResolution(hWnd, 800, NULL); break;
				case IDM_1024_768	: if(VIDEO_ExtraChangeResolution) VIDEO_ExtraChangeResolution(hWnd, 1024, NULL);break;
				case IDM_1280_1024	: if(VIDEO_ExtraChangeResolution) VIDEO_ExtraChangeResolution(hWnd, 1280, NULL);break;
				case IDM_PLUGINS	: DialogBox(hInst, "PLUGINS", hWnd, (DLGPROC)PluginsDialog);			break;
				case ID_CHECKWEB	: ShellExecute( hwnd, "open", "http://www.emuhq.com/1964", NULL, NULL, SW_MAXIMIZE); break;

				// NooTe 22/07/99
				case ID_CONFIGURE_VIDEO	:	VIDEO_DllConfig(hWnd);	break;
				case ID_HELP_FINDER: DisplayError("Help contents");	break;
				case ID_ABOUT_WARRANTY:
					LoadString(hInst, IDS_WARRANTY_SEC11, WarrantyPart1, 700);
					LoadString(hInst, IDS_WARRANTY_SEC12, WarrantyPart2, 700);
					MessageBox(hwnd, WarrantyPart1, "NO WARRANTY", MB_OK);
					MessageBox(hwnd, WarrantyPart2, "NO WARRANTY", MB_OK);
					break;

				case ID_REDISTRIBUTE:
					DialogBox(hInst, "REDISTRIB_DIALOG", hWnd, (DLGPROC)ConditionsDialog);
					break;
#ifdef WINDEBUG_1964
				case ID_DEBUGGER_OPEN:
					OpenDebugger();
					break;
				case ID_DEBUGGER_CLOSE:
					CloseDebugger();
					break;
#endif
                case ID_EXIT:
				
#ifdef WINDEBUG_1964
					CloseDebugger();
#endif

				SuspendThread(CPUThreadHandle);
				CloseHandle(CPUThreadHandle);
				//FreePlugins();
				PostQuitMessage(0);
				break;
			}
            break;

		case WM_CLOSE:
			SuspendThread(CPUThreadHandle);
			CloseHandle(CPUThreadHandle);
			//FreePlugins();
			PostQuitMessage(0);
			break;
        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0l);
}

//---------------------------------------------------------------------------------------

void Pause() 
{
	SuspendThread(CPUThreadHandle);
}

//---------------------------------------------------------------------------------------

void Kill() 
{
	DWORD ExitCode = 0;
	
	TerminateThread((HANDLE)CPUThreadHandle, ExitCode);
	while (ExitCode == STILL_ACTIVE)
		GetExitCodeThread(CPUThreadHandle, &ExitCode);
	CPUThreadHandle = NULL;
}

//---------------------------------------------------------------------------------------

void Play()
{
	DWORD ExitCode;
	GetExitCodeThread(CPUThreadHandle, &ExitCode);
	if (ExitCode != STILL_ACTIVE)
	{
		r4300i_Reset();
		RunEmulator(1);
	}
	else
		ResumeThread(CPUThreadHandle);
}

//---------------------------------------------------------------------------------------

void OpenROM() 
{
	int WasRunning = 0;
	DWORD ExitCode;

	GetExitCodeThread(CPUThreadHandle, &ExitCode);
	if (ExitCode == STILL_ACTIVE)
	{
		WasRunning = TRUE;
		Pause();
	}
	if (WinLoadRom() == TRUE) //If the user opened a rom,
	{
#ifndef WINDEBUG_1964
		Kill();
		Play(); //autoplay
#endif
	}
	else if (WasRunning == TRUE)
		Play();
}

//---------------------------------------------------------------------------------------

void Reset()
{
	if (Rom_Loaded)
	{
		SuspendThread(CPUThreadHandle);
		TerminateThread(CPUThreadHandle, 1);
		CloseHandle(CPUThreadHandle);
		r4300i_Reset();
		Play();
	}
	else
		DisplayError("Please load a ROM first.", 1);
}

//---------------------------------------------------------------------------------------

LRESULT APIENTRY About(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
    switch (message) {
        case WM_INITDIALOG:
            return (TRUE);

        case WM_COMMAND:
	    if (wParam == IDOK
                || wParam == IDCANCEL) {
                EndDialog(hDlg, TRUE);
                return (TRUE);
            }
            break;
    }
    return (FALSE);
}

//---------------------------------------------------------------------------------------
/* Redistribution Conditions Window */
LRESULT APIENTRY ConditionsDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
	char Conditions[11201];

	switch (message) {
        case WM_INITDIALOG:
         //   LoadString(hInst, IDS_REDIST0, temp_buf, 4096);
			LoadGNUDistConditions(Conditions); 
			SetDlgItemText(hDlg, IDC_EDIT0, Conditions);
			return (TRUE);

        case WM_COMMAND:
	    if (wParam == IDOK
                || wParam == IDCANCEL) {
                EndDialog(hDlg, TRUE);
                return (TRUE);
            }
            break;
    }
    return (FALSE);
}

//---------------------------------------------------------------------------------------

void ResizeVideoWindow() 
{	
	static uint32 OLD_WINDOW_WIDTH;
	RECT winrect;
	
	int width, height;

	winrect.top = 0;
	winrect.left = 0;
	winrect.right = VI_WIDTH_REG;
	winrect.bottom = ((VI_WIDTH_REG * 3) >> 2);

	
	if (OLD_WINDOW_WIDTH ==	VI_WIDTH_REG) return;
	
	if (!IsZoomed(hwnd))
	{
		OLD_WINDOW_WIDTH = VI_WIDTH_REG;

		AdjustWindowRect( &winrect, WS_OVERLAPPEDWINDOW, TRUE);

		width = winrect.right - winrect.left;
		height = winrect.bottom - winrect.top;

		SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW);
	}
}
