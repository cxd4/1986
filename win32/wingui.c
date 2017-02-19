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

int EMU_STATUS;
#define PAUSED  1

#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <direct.h>
#include "wingui.h"
#include "..\globals.h"
#include "plugins.h"
#include "DLL_Video.h"
#include "DLL_Audio.h"
#include "DLL_Input.h"

extern HANDLE CPUThreadHandle;

int ActiveApp;

#ifdef WINDEBUG_1964
#include "windebug.h"
#endif

int repeat;
char szBaseWindowTitle[] = "1964 0.4.9";

extern void r4300i_Reset();
extern void RunEmulator(unsigned _int32 WhichCore);
extern BOOL CloseCPUThread();

void Pause();
void Kill();
void Play();
void Reset();
void OpenROM();
void SwitchVideoMode();


HACCEL hAccelTable = (HACCEL)"WINGUI_ACC";
BOOL fFreeResult;

//---------------------------------------------------------------------------------------

void LoadPlugins()
{
	LoadAudioPlugin("steb_aud");
	LoadVideoPlugin("1964ogl");
//	LoadVideoPlugin("NooTe_D3D");
  LoadInputPlugin("NooTe_DI");

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
			exit(1);
		

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	Gfx_Info.hWnd					= hwnd;
	Gfx_Info.hStatusBar				= NULL;
	Gfx_Info.RDRAM					= (__int8*)&RDRAM[0];
	Gfx_Info.DMEM					= (__int8*)&SP_REG[0];
	Gfx_Info.MI_INTR_RG				= (unsigned __int32*)&MI_INTR_REG_R;
	Gfx_Info.VI_STATUS_RG			= &VI_STATUS_REG;
	Gfx_Info.VI_ORIGIN_RG			= &VI_ORIGIN_REG;
	Gfx_Info.VI_WIDTH_RG			= &VI_WIDTH_REG;
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
	LoadPlugins();

	INPUT_Initialize(hwnd, hInstance);

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
    ofn.lpstrInitialDir		= AppPath;
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

LRESULT APIENTRY PluginsDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
	switch (message) {
        case WM_INITDIALOG:
			return (TRUE);

        case WM_COMMAND:
			switch (wParam)
			{
				case IDOK:
				case IDCANCEL:
                {
					EndDialog(hDlg, TRUE);
					return (TRUE);
				}
				case  IDC_DI_CONFIG: INPUT_DllConfig(hDlg);	break;
				case  IDC_DI_ABOUT : INPUT_About(hDlg);		break;

				case IDC_VID_CONFIG: VIDEO_DllConfig(hDlg);	break;
				case IDC_VID_ABOUT : VIDEO_About(hDlg);		break;

            }
            break;
    }
    return (FALSE);
}

//---------------------------------------------------------------------------------------

long FAR PASCAL MainWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    static PAINTSTRUCT ps;
	static int ok = 0;

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
				case IDM_PLUGINS	:	DialogBox(hInst, "PLUGINS", hWnd, (DLGPROC)PluginsDialog);			break;
				case ID_CHECKWEB	:	system("explorer.exe http://www.emuhq.com/1964"); break;

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
				FreePlugins();
				PostQuitMessage(0);
				break;
			}
            break;

		case WM_CLOSE:
			SuspendThread(CPUThreadHandle);
			CloseHandle(CPUThreadHandle);
			FreePlugins();
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
	EMU_STATUS = PAUSED;
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