/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Emulator for Nintendo 64 console system                              |
 |  Copyright (C) 2001  Joel Middendorf  schibo@emulation64.com                 |
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
 |  email      : schibo@emulation64.com                                         |
 |  paper mail :                                                                |
 |______________________________________________________________________________|


The project is a direct documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar. Most source
code comments are taken directly from anarko's n64toolkit with consent and are 
the property of anarko.
*/

#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <shlobj.h>
#include "wingui.h"
#include <shellapi.h>
#include "..\options.h"
#include "..\debug_option.h"
#include "..\globals.h"
#include "..\n64rcp.h"
#include "..\hardware.h"
#include "..\fileio.h"
#include "..\emulator.h"
#include "..\dma.h"
#include "..\interrupt.h"
#include "..\memory.h"
#include "..\iPIF.h"
#include "..\flashram.h"
#include "DLL_Video.h"
#include "DLL_Audio.h"
#include "DLL_Input.h"
#include "registry.h"
#include "..\r4300i.h"
#include "..\controller.h"
#include "..\timer.h"
#include "..\1964ini.h"
#include "..\romlist.h"
#include "..\cheatcode.h"
#include "..\kaillera\kaillera.h"
#include "..\dynarec\dynacpu.h"


#ifdef WINDEBUG_1964
#include "windebug.h"
#endif

int ActiveApp;
char szBaseWindowTitle[] = "1964 0.6.4";
HACCEL hAccelTable = (HACCEL)"WINGUI_ACC";
HWND hwnd=NULL;						/* handle to main window					*/
HWND hwndRomList=NULL;				/* Handle to the rom list child window		*/
HWND hStatusBar=NULL;				/* Window Handle of the status bar			*/
HWND hToolBar=NULL;					/* Window Handle of the toolbar				*/
HWND hClientWindow=NULL;			/* Window handle of the client child window */
HWND hCriticalMsgWnd=NULL;			/* handle to critical message window		*/
HMENU hMenu=NULL;

// Status Bar text fields
char staturbar_field_1[256];
char staturbar_field_2[80];
char staturbar_field_3[80];
char staturbar_field_4[80];
char staturbar_field_5[80];

unsigned int cfmenulist[8] = {
	ID_CF_CF1,
	ID_CF_CF2,
	ID_CF_CF3,
	ID_CF_CF4,
	ID_CF_CF5,
	ID_CF_CF6,
	ID_CF_CF7,
	ID_CF_CF8
};

unsigned int codecheckmenulist[8] = {
	ID_CPU_DYNACODECHECKING_NOCHECK,
	ID_CPU_DYNACODECHECKING_DMA,
	ID_CPU_DYNACODECHECKING_DWORD,
	ID_CPU_DYNACODECHECKING_QWORD,
	ID_CPU_DYNACODECHECKING_QWORDANDDMA,
	ID_CPU_DYNACODECHECKING_BLOCK,
	ID_CPU_DYNACODECHECKING_BLOCKANDDMA
};

char recent_rom_directory_lists[4][260];
char recent_game_lists[8][260];

char game_country_name[10];
int game_country_tvsystem=0;

int AiUpdating;
int timer;
BOOL gamepaused = FALSE;
BOOL pausegame  = FALSE;
int StateFileNumber = 0;
BOOL pausegamebymenu = FALSE;
BOOL auto_apply_cheat_code_enabled = FALSE;
BOOL window_is_moving = FALSE;
BOOL window_is_maximized = FALSE;
BOOL window_is_minimized = FALSE;
BOOL block_menu = TRUE;				// Block all menu command while 1964 is busy
int clientwidth = 800;

RECT window_position;

extern int selected_rom_index;
extern BOOL Is_Reading_Rom_File;;
extern BOOL To_Stop_Reading_Rom_File;

#ifdef DEBUG_COMMON
void ToggleDebugOptions(WPARAM wParam);
#endif
LRESULT APIENTRY OptionsDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam);

void SelectVISpeed(WPARAM wParam);
void SetupAdvancedMenus();
void RegenerateAdvancedUserMenus(void);
void DeleteAdvancedUserMenus(void);
void RegenerateStateSelectorMenus(void);
void DeleteStateSelectorMenus(void);
void RegerateRecentGameMenus(void);
void DeleteRecentGameMenus(void);
void RegerateRecentRomDirectoryMenus(void);
void DeleteRecentRomDirectoryMenus(void);
void RefreshRecentGameMenus(char* newgamefilename);
void RefreshRecentRomDirectoryMenus(char* newromdirectory);
void ChangeToRecentDirectory(int id);
void OpenRecentGame(int id);

//---------------------------------------------------------------------------------------
extern HINSTANCE hInst;
extern HWND hwnd;
extern LRESULT APIENTRY PluginsDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam);

/*	
	type = 0		Load all plugins
	type = 1		Load video plugin
	type = 2		Load audio plugin
	type = 3		Load input plugin
*/

enum {LOAD_ALL_PLUGIN, LOAD_VIDEO_PLUGIN, LOAD_AUDIO_PLUGIN, LOAD_INPUT_PLUGIN};

void LoadPlugins(int type)
{
    char AudioPath[_MAX_PATH];	// _MAX_PATH = 260
    char VideoPath[_MAX_PATH];
    char InputPath[_MAX_PATH];
    char StartPath[_MAX_PATH];
    char NullSndPath[_MAX_PATH];
    // int Crapped = 0;
    int Audio = 0;

    SetDefaultOptions();
    GetPluginDir(StartPath);
    
	if( type == LOAD_ALL_PLUGIN || type == LOAD_VIDEO_PLUGIN )
	{
		strcpy(VideoPath, StartPath);

		SetStatusBarText(0, "Loading Video Plugin ...");
		// Set Video plugin path
		if (strcmp(gRegSettings.VideoPlugin, "") == 0)
		{
			strcpy(gRegSettings.VideoPlugin, "1964ogl.dll");
			strcat(VideoPath, gRegSettings.VideoPlugin);
		}
		else
		{
			strcat(VideoPath, gRegSettings.VideoPlugin);
		}

		// Load Video plugin
		if (LoadVideoPlugin(VideoPath) == FALSE)
		{
			strcpy(gRegSettings.VideoPlugin, "");
			WriteConfiguration();
			CloseVideoPlugin();
		}

	}

	if( type == LOAD_ALL_PLUGIN || type == LOAD_INPUT_PLUGIN )
	{
		SetStatusBarText(0, "Loading Input Plugin ...");
		// Set Input plugin path
		strcpy(InputPath, StartPath);
		if (strcmp(gRegSettings.InputPlugin, "") == 0)
		{
			strcpy(gRegSettings.InputPlugin, "Basic Keyboard Plugin.dll");
			strcat(InputPath, gRegSettings.InputPlugin);
		}
		else
		{
			strcat(InputPath, gRegSettings.InputPlugin);
		}

		// Load Input plugin DLL
		if (LoadControllerPlugin(InputPath) == FALSE)
		{
			strcpy(gRegSettings.InputPlugin, "");
			WriteConfiguration();
			CloseControllerPlugin();
		}
		// Call the CONTROLLER_InitiateControllers function in the input DLL to initiate the controllers
		SetStatusBarText(0, "Init Input Plugin ...");
		CONTROLLER_InitiateControllers(hwnd, Controls);  
	}

	if( type == LOAD_ALL_PLUGIN || type == LOAD_AUDIO_PLUGIN )
	{
		SetStatusBarText(0, "Loading Audio Plugin ...");
		// Set path for the Audio plugin
		strcpy(AudioPath, StartPath);
		if (strcmp(gRegSettings.AudioPlugin, "") == 0)
		{
			strcpy(gRegSettings.AudioPlugin, "tr64_audio.dll");
			strcat(AudioPath, gRegSettings.AudioPlugin);
		}
		else
		{
			strcat(AudioPath, gRegSettings.AudioPlugin);
		}

		if (LoadAudioPlugin(AudioPath) == TRUE)
		{
			Audio = 1;
		}

		if (_AUDIO_Initialize != NULL)
		{
			AiUpdating = 1;
			if (AUDIO_Initialize(Audio_Info) == TRUE )
			{
				Audio = 1;
			}
			else
			{
				Audio = 0;
				AiUpdating = 0;
			}
			//AiUpdating = 0;
		}

		if (Audio == 0)
		{    
			CloseAudioPlugin();
			strcpy(gRegSettings.AudioPlugin, "");
			WriteConfiguration();

			strcpy(NullSndPath, StartPath);
			strcpy(gRegSettings.AudioPlugin, "No Sound.dll");
			strcat(NullSndPath, gRegSettings.AudioPlugin);

		}
	}

	if( type == LOAD_ALL_PLUGIN || type == LOAD_VIDEO_PLUGIN )
	{
		SetStatusBarText(0, "Init Video Plugin ...");
		VIDEO_InitiateGFX(Gfx_Info);
		//VIDEO_RomOpen();
		NewRomList_ListViewChangeWindowRect();
		DockStatusBar();
	}
	SetStatusBarText(0, "Ready");
}

//---------------------------------------------------------------------------------------
void CALLBACK TimerProc( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime )
{
	if( Rom_Loaded )
	{
		if( Emu_Is_Running )
		{
			if( !gamepaused )
			{
				vips = (vips * 0.8f + 0.2f * framecounter);
				if( vips >= 100.0 )
					sprintf(generalmessage, "%3d VI/s", (int)vips);
				else
					sprintf(generalmessage, "%3.1f VI/s", vips);
				framecounter=0;
				QueryPerformanceCounter(&LastSecondTime);
				SetStatusBarText(1, generalmessage);

				if( display_detail_status )
				{
					sprintf(generalmessage, "PC=%08x, DList=%d, AList=%d, PI=%d, Cont=%d", 
						gHWS_pc,
						DListCount,
						AListCount,
						PIDMACount,
						ControllerReadCount
					);
					SetStatusBarText(0, generalmessage);
				}
			}

			// Apply the hack codes
			if( auto_apply_cheat_code && auto_apply_cheat_code_enabled )
				CodeList_ApplyAllCode(INGAME);
		}
	}
}

//---------------------------------------------------------------------------------------


void FreePlugins()
{
	CloseControllerPlugin();
	CloseAudioPlugin();
	CloseVideoPlugin();
}

//---------------------------------------------------------------------------------------

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    MSG msg;
    Rom_Loaded	= FALSE;
	block_menu	= TRUE;			// block all menu commands during starting up	
	whichcore = DYNACOMPILER;
	OSversion = GetVersion();

#ifdef WINDEBUG_1964
    RUN_TO_ADDRESS_ACTIVE = FALSE;
    OpCount = 0;
    NextClearCode = 250;
    BreakAddress = -1;
#endif

#ifdef ENABLE_OPCODE_DEBUGGER
    p_gHardwareState = (HardwareState*)&gHardwareState;
    p_gMemoryState   = (MemoryState*)&gMemoryState;
#endif

    hInst = hInstance;
    LoadString(hInstance, IDS_MAINDISCLAIMER, MainDisclaimer, sizeof(MainDisclaimer));

    if (hPrevInstance)
        return FALSE;

	Set_1964_Directory();

	LoadDllKaillera();
	kailleraInit();
	ReadConfiguration();                //System registry settings

    hwnd = InitWin98UI(hInstance, nCmdShow);
        if (hwnd == NULL)
            DisplayError("Could not get a windows handle.");

	SetupAdvancedMenus();
#ifndef DEBUG_COMMON
    EnableMenuItem(hMenu, ID_RUNTO, MF_GRAYED);
	{
		// Disable the DEBUG menu
		int i,k;
		char str[80];
		i = GetMenuItemCount(hMenu);
		for( k=0; k<i; k++)
		{
			GetMenuString(hMenu,k,str,80,MF_BYPOSITION);
			if( strcmp(str,"Debug" ) == 0 )
				DeleteMenu(hMenu,k,MF_BYPOSITION);
		}
	}
#else
	SetDebugMenu();
#endif

#ifndef ENABLE_OPCODE_DEBUGGER
	DeleteMenu(hMenu,ID_OPCODEDEBUGGER,MF_BYCOMMAND);
	DeleteMenu(hMenu,ID_OPCODEDEBUGGER_BLOCK_ONLY,MF_BYCOMMAND);
	DeleteMenu(hMenu,ID_DIRTYONLY,MF_BYCOMMAND);
#endif
        

	hStatusBar = CreateStatusWindow( WS_CHILD |WS_VISIBLE , staturbar_field_1, hwnd, 0x1122	);
	{
		RECT rc, src;
		GetWindowRect(hwnd, &rc);
		GetWindowRect(hStatusBar, &src);
		//MoveWindow(hwnd,rc.left, rc.top, rc.right-rc.left+1, rc.bottom-rc.top+1+src.bottom-src.top+1, FALSE);
		DockStatusBar();
	}
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

	InitVirtualMemory();

	InitPluginData();

	SetStatusBarText(0,"Load Rom Setting from 1964.ini");
	FileIO_Load1964Ini();

	SetWindowText(hwnd, szBaseWindowTitle);
	whichcore = defaultoptions.Emulator;
	SendMessage(hwnd, WM_COMMAND, whichcore == DYNACOMPILER?ID_DYNAMICCOMPILER:ID_INTERPRETER, 0);

	SendMessage(hwnd, WM_COMMAND, cfmenulist[defaultoptions.Counter_Factor-1],0);

	SetStatusBarText(3, defaultoptions.RDRAM_Size == RDRAMSIZE_4MB ? "4MB":"8MB");
	SetStatusBarText(4, "D");
	SetStatusBarText(2, "CF=1");

	SetStatusBarText(0, "Looking for ROM file in the ROM directory and Generate List");
	RomListReadDirectory( rom_directory_to_use );
	hwndRomList = NewRomList_CreateListViewControl(hwnd);

    SetStatusBarText(0, "Loading plugins");
	LoadPlugins(LOAD_ALL_PLUGIN);

	SetStatusBarText(0, "Initialize emulator and r4300 core");
    r4300i_Init();

	timer = SetTimer(hwnd,1,1000,TimerProc);

	SetStatusBarText(0, "Ready");
	block_menu = FALSE;				// allow menu commands

#ifdef DEBUG_COMMON
    DebuggerActive = FALSE;
    OpenDebugger();
#endif

	if( critical_msg_window )
	{
		if( hCriticalMsgWnd == NULL )
		{
			hCriticalMsgWnd = CreateDialog(hInst, "CRITICAL_MESSAGE", NULL, (DLGPROC)CriticalMessageDialog);
		}
	}
	
    SetActiveWindow(hwnd);


_HOPPITY:

    if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
	{
        if( !GetMessage( &msg, NULL, 0, 0 ) ) 
		{
            return msg.wParam;
		}
        else if (!TranslateAccelerator (hwnd, hAccelTable, &msg))
        {
            TranslateMessage (&msg) ;
            DispatchMessage (&msg) ;
        }
	}
 goto _HOPPITY;
}

//---------------------------------------------------------------------------------------

char critical_msg_buffer[32*1024];	//32KB
void __cdecl DisplayCriticalMessage(char * Message, ...)
{
	if( critical_msg_window )
	{
		char Msg[400];
		va_list ap;

		va_start( ap, Message );
		vsprintf( Msg, Message, ap );
		va_end( ap );

		if( strlen(critical_msg_buffer) + strlen(Msg) + 2 < 32*1024 )
		{
			strcat(critical_msg_buffer, Msg);
			strcat(critical_msg_buffer,"\n");
			SendDlgItemMessage(hCriticalMsgWnd, IDC_CRITICAL_MESSAGE_TEXTBOX, WM_SETTEXT, 0, (LPARAM)critical_msg_buffer);
		}
		else
		{
			DisplayError(Msg);
		}
	}
}

void __cdecl DisplayError (char * Message, ...)
{
#ifdef ENABLE_DISPLAY_ERROR
    char Msg[400];
    va_list ap;

#ifdef WINDEBUG_1964
    RefreshDebugger();
#endif

    va_start( ap, Message );
    vsprintf( Msg, Message, ap );
    va_end( ap );

    MessageBox(NULL,Msg,"Error",MB_OK|MB_ICONINFORMATION);
	DisplayCriticalMessage(Msg);
#else
	//Display this in the log window
#endif
}


BOOL __cdecl DisplayError_AskIfContinue (char * Message, ...)
{
    char Msg[400];
	int val;
    va_list ap;

#ifdef WINDEBUG_1964
    RefreshDebugger();
#endif

    va_start( ap, Message );
    vsprintf( Msg, Message, ap );
    va_end( ap );
	strcat(Msg,"\nDo you want to continue emulation ?");

    val=MessageBox(NULL,Msg,"Error",MB_YESNO|MB_ICONINFORMATION|MB_SYSTEMMODAL);
	if( val == IDYES )
		return TRUE;
	else
		return FALSE;
}

//---------------------------------------------------------------------------------------

void UpdateCIC()
{
//** Math CIC
	__int64 CIC_CRC = 0;
    int i;

	for (i=0; i <0xFC0; i++)
	{
		CIC_CRC = CIC_CRC + (uint8)gMemoryState.ROM_Image[0x40+i];
	}

	switch(CIC_CRC)
	{
	// CIC-NUS-6101  (starfox)
	case 0x33a27:
	case 0x3421e:
//		DisplayError("Using CIC-NUS-6101\n");
		rominfo.CIC = (uint64)0x3f;
		rominfo.RDRam_Size_Hack = (uint32)0x318;
		break;

	//CIC-NUS-6102  (mario)	
	case 0x34044:
//		DisplayError("Using CIC-NUS-6102\n");
		rominfo.CIC = (uint64)0x3f;
		rominfo.RDRam_Size_Hack = (uint32)0x318;
		ROM_CheckSumMario();

		break;		

	//CIC-NUS-6103   (Banjo)
	case 0x357d0:
//		DisplayError("Using CIC-NUS-6103\n");
		rominfo.CIC = (uint64)0x78;
		rominfo.RDRam_Size_Hack = (uint32)0x318;
		break;

	//CIC-NUS-6105  (Zelda)
	case 0x47a81:
//		DisplayError("Using CIC-NUS-6105\n");
		rominfo.CIC = 0x91;
		rominfo.RDRam_Size_Hack = (uint32)0x3F0;
		ROM_CheckSumZelda();
		break;

	// CIC-NUS-6106  (F-Zero X)
	case 0x371cc:
//		DisplayError("Using CIC-NUS-6106\n");
		rominfo.CIC = (uint64)0x85;
		rominfo.RDRam_Size_Hack = (uint32)0x318;
		break;

// Using F1 World Grand Prix
	case 0x343c9:
//		LogDirectToFile("Using f1 World Grand Prix\n");
//		DisplayError("F1 World Grand Prix ... i never saw ths BootCode before");
		rominfo.CIC = (uint64)0x85;
		rominfo.RDRam_Size_Hack = (uint32)0x3F0;
		break;

	default:
//		DisplayError("unknown CIC %08x!!!", (uint32)CIC_CRC);
//		SystemFailure(FILEIO_EXIT);

		// Use Mario for unknown boot code
		DisplayError("Unknown boot code, use Mario boot code instead");
		rominfo.CIC = (uint64)0x3f;
		rominfo.RDRam_Size_Hack = (uint32)0x318;
		break;
	}

	TRACE1("Rom CIC=%02X", rominfo.CIC);

	rominfo.countrycode = HeaderDllPass[0x3D];

	CountryCodeToCountryName_and_TVSystem(rominfo.countrycode, game_country_name, &game_country_tvsystem);
	rominfo.TV_System = game_country_tvsystem;
	Init_VI_Counter(game_country_tvsystem);
}

BOOL WinLoadRom()
{
    OPENFILENAME ofn;
    char szFileName[MAXFILENAME];
    char szFileTitle[MAXFILENAME];

    memset(&szFileName,0,sizeof(szFileName));
    memset(&szFileTitle, 0, sizeof(szFileTitle));

    ofn.lStructSize         = sizeof(OPENFILENAME);
    ofn.hwndOwner           = hwnd;
    ofn.lpstrFilter         = "N64 ROMs (*.ZIP, *.V64, *.BIN, *.ROM, *.Z64, *.N64, *.USA, *.PAL, *.J64)\0*.ZIP;*.V64;*.BIN;*.ROM;*.Z64;*.N64;*.USA;*.PAL;*.J64\0All Files (*.*)\0*.*\0";
    ofn.lpstrCustomFilter   = NULL;
    ofn.nMaxCustFilter      = 0;
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = szFileName;
    ofn.nMaxFile            = MAXFILENAME;
	ofn.lpstrInitialDir     = rom_directory_to_use;
    ofn.lpstrFileTitle      = szFileTitle;
    ofn.nMaxFileTitle       = MAXFILENAME;
    ofn.lpstrTitle          = "Open Image";
    ofn.lpstrDefExt         = "TXT";
    ofn.Flags               = OFN_HIDEREADONLY;

    if (!GetOpenFileName ((LPOPENFILENAME)&ofn))
	{
        return FALSE;
	}

	_getcwd( rom_directory_to_use, PATH_LEN );
	strcpy(last_rom_directory, rom_directory_to_use);
    
    WriteConfiguration();

    if( WinLoadRomStep2(szFileName) )
    {
    /*
        INI_ENTRY *pentry;
        char drive[_MAX_DRIVE] ,dir[_MAX_DIR];
        char fname[_MAX_FNAME],ext[_MAX_EXT];
        _splitpath( szFileName, drive, dir, fname, ext );
        strcpy(szFileName, fname);
        strcat(szFileName,ext);
        // Check and create romlist entry for this new loaded rom
        pentry = GetNewIniEntry();
        ReadRomHeaderInMemory(pentry);
        RomListAddEntry(pentry,szFileName, 0);
        DeleteIniEntryByEntry(pentry);
    */

        // Read hack code for this rom
        CodeList_Clear();
        CodeList_ReadCode(rominfo.name);
        RefreshRecentGameMenus(szFileName);
        return TRUE;
    }    
	else
	{
		return FALSE;
	}
}

BOOL WinLoadRomStep2(char * szFileName)
{
    lstrcpy(szWindowTitle, szBaseWindowTitle);
    lstrcat(szWindowTitle, " - ");

    if( ReadRomData(szFileName) == FALSE )
		return FALSE;

    lstrcat(szWindowTitle, rominfo.name);

	memcpy(&HeaderDllPass[0], &gMemoryState.ROM_Image[0], 0x40);
    EnableMenuItem(hMenu, ID_OPENROM, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_PLUGINS, MF_GRAYED);
	EnableMenuItem(hMenu, ID_CLOSEROM, MF_ENABLED);
    SetWindowText(hwnd, szWindowTitle);

    gHardwareState.pc = 0xA4000040; //We do it in r4300i_inithardware

    Rom_Loaded = TRUE;

    UpdateCIC();
	sprintf(generalmessage, "%s - Loaded", szWindowTitle);
	SetWindowText(hwnd, generalmessage);

    return TRUE;    
}

//---------------------------------------------------------------------------------------

HWND InitWin98UI(HANDLE hInstance, int nCmdShow)
{
    WNDCLASS    wc;

    int width, height;
    RECT winrect = {0,0,800,600};

	winrect.right = clientwidth;
	switch( clientwidth )
	{
	case 320:
		winrect.bottom = 200;
		break;
	case 640:
		winrect.bottom = 480;
		break;
	case 1024:
		winrect.bottom = 768;
		break;
	case 1280:
		winrect.bottom = 1024;
		break;
	default: // 800:
		winrect.bottom = 600;
		winrect.right = 800;
		break;
	}

    AdjustWindowRect(   &winrect, WS_OVERLAPPEDWINDOW /*|WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME */, TRUE);

    width = winrect.right - winrect.left;
    height = winrect.bottom - winrect.top;

    wc.style  = CS_SAVEBITS; // | CS_DBLCLKS;
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
        WS_OVERLAPPEDWINDOW, // | WS_VSCROLL,
        window_position.left,
        window_position.top,
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
    hMenu = GetMenu (hwnd);

	if(!IsKailleraDllLoaded())
		EnableMenuItem(hMenu, ID_KAILLERA_MODE, MF_GRAYED);

    return hwnd;
}

//---------------------------------------------------------------------------------------

void GetPluginDir(char* Directory)
{
    //strcpy(Directory,main_directory);
    //strcat(Directory,"Plugin\\");
	strcpy(Directory, plugin_directory_to_use);
}

//---------------------------------------------------------------------------------------

void   (__cdecl* GetDllInfo )( PLUGIN_INFO *) = NULL; 
void   (__cdecl* DLL_About)(HWND) = NULL; 
char temp_video_plugin[256];
char temp_audio_plugin[256];
char temp_input_plugin[256];
LRESULT APIENTRY PluginsDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam) 
{ 
	PLUGIN_INFO Plugin_Info; 
    HINSTANCE       hinstLib = NULL; 
    WIN32_FIND_DATA libaa; 
    int ComboItemNum;

    int h=0, i=0, j=0, bDONE = 0; 

    HANDLE FindFirst; 
    char PluginName[300]; 
    char StartPath[_MAX_PATH]; 
    char SearchPath[_MAX_PATH]; 
    int index; 

    GetPluginDir(StartPath); 
    strcpy(SearchPath, StartPath); 
    strcat(SearchPath, "*.dll"); 

  switch (message) 
  { 
  case WM_INITDIALOG: 
    { 
      int KeepLooping = 1; 
	  strcpy(temp_video_plugin, gRegSettings.VideoPlugin);
	  strcpy(temp_audio_plugin, gRegSettings.AudioPlugin);
	  strcpy(temp_input_plugin, gRegSettings.InputPlugin);

      FindFirst = FindFirstFile(SearchPath, &libaa); 
      
      if (FindFirst == INVALID_HANDLE_VALUE) 
      { 
        return(TRUE); 
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
        
        GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *)) GetProcAddress(hinstLib, "GetDllInfo");

		__try{
			GetDllInfo(&Plugin_Info); 
		}
		__except(NULL,EXCEPTION_EXECUTE_HANDLER)
		{
			goto skipdll;
		};
        
        switch (Plugin_Info.Type) 
        { 
        case PLUGIN_TYPE_GFX            : 
          index = SendDlgItemMessage(hDlg, IDC_COMBO_VIDEO, CB_ADDSTRING, 0, (LPARAM)Plugin_Info.Name); 
          if ( _stricmp(libaa.cFileName, gRegSettings.VideoPlugin) == 0 ) 
            SendDlgItemMessage(hDlg, IDC_COMBO_VIDEO, CB_SETCURSEL, (WPARAM)index, (LPARAM)0); 
          break; 
          
        case PLUGIN_TYPE_CONTROLLER : 
          index = SendDlgItemMessage(hDlg, IDC_COMBO_INPUT, CB_ADDSTRING, 0, (LPARAM)Plugin_Info.Name); 
          if ( _stricmp(libaa.cFileName, gRegSettings.InputPlugin) == 0 ) 
            SendDlgItemMessage(hDlg, IDC_COMBO_INPUT, CB_SETCURSEL, (WPARAM)index, (LPARAM)0); 
          break; 
          
        case PLUGIN_TYPE_AUDIO          : 
          index = SendDlgItemMessage(hDlg, IDC_COMBO_AUDIO, CB_ADDSTRING, 0, (LPARAM)Plugin_Info.Name); 
          if ( _stricmp(libaa.cFileName, gRegSettings.AudioPlugin) == 0 ) 
            SendDlgItemMessage(hDlg, IDC_COMBO_AUDIO, CB_SETCURSEL, (WPARAM)index, (LPARAM)0); 
          break; 
          
        } 
        
skipdll:
        FreeLibrary(hinstLib); 
        hinstLib = NULL; 
        KeepLooping = FindNextFile(FindFirst, &libaa); 
        GetDllInfo = NULL; 
        PluginName[0] = '\0'; 

		_CONTROLLER_Under_Selecting_DllAbout = _CONTROLLER_DllAbout;
		_CONTROLLER_Under_Selecting_DllTest = _CONTROLLER_DllTest;
		_VIDEO_Under_Selecting_About = _VIDEO_About;
		_VIDEO_Under_Selecting_Test = _VIDEO_Test;
		_AUDIO_Under_Selecting_About = _AUDIO_About;
		_AUDIO_Under_Selecting_Test = _AUDIO_Test;
		} 
    } 
    return (TRUE); 
      
  case WM_COMMAND: 
    { 
      switch (wParam) 
      { 
      case IDOK: 
        { 
			BOOL is_changed = FALSE;
			FreeLibrary(hinstLib); 
			EndDialog(hDlg, TRUE); 
			if( strcmp(gRegSettings.VideoPlugin, temp_video_plugin) != 0 )
			{
				strcpy(gRegSettings.VideoPlugin, temp_video_plugin);
				CloseVideoPlugin();
				LoadPlugins(LOAD_VIDEO_PLUGIN);
				NewRomList_ListViewChangeWindowRect();
				DockStatusBar();
				is_changed = TRUE;
			}

			if( strcmp(gRegSettings.AudioPlugin, temp_audio_plugin) != 0 )
			{
				strcpy(gRegSettings.AudioPlugin, temp_audio_plugin);
				CloseAudioPlugin();
				LoadPlugins(LOAD_AUDIO_PLUGIN);
				is_changed = TRUE;
			}

			if( strcmp(gRegSettings.InputPlugin, temp_input_plugin) != 0 )
			{
				strcpy(gRegSettings.InputPlugin, temp_input_plugin);
				CloseControllerPlugin();
				LoadPlugins(LOAD_INPUT_PLUGIN);
				CONTROLLER_InitiateControllers(hwnd, Controls);       //Input DLL Initialization
				is_changed = TRUE;
			}

			if( is_changed )
				WriteConfiguration(); 
			return (TRUE); 
        } 
      case IDCANCEL: 
        { 
          EndDialog(hDlg, TRUE); 
          return (TRUE); 
        } 
      case  IDC_DI_ABOUT : CONTROLLER_Under_Selecting_DllAbout(hDlg);           break; 
      case  IDC_DI_TEST  : CONTROLLER_Under_Selecting_DllTest(hDlg);            break; 
        
      case IDC_VID_ABOUT : VIDEO_Under_Selecting_About(hDlg);           break; 
      case IDC_VID_TEST  : VIDEO_Under_Selecting_Test(hDlg);            break; 
        
      case IDC_AUD_ABOUT : AUDIO_Under_Selecting_About(hDlg);           break; 
      case IDC_AUD_TEST  : AUDIO_Under_Selecting_Test(hDlg);            break; 
        
      } 
      
      case CBN_SELCHANGE:
		  switch ( 	LOWORD(wParam) )
		  {
		  case IDC_COMBO_VIDEO:
			// Video
		__try{
			FreeLibrary(hinstLib); 
			ComboItemNum = SendDlgItemMessage(hDlg, IDC_COMBO_VIDEO, CB_GETCURSEL, 0, 0); 

			bDONE = 0;
			FindFirst = FindFirstFile(SearchPath, &libaa); 
			while (bDONE == 0) 
			{ 
			  strcpy(PluginName, StartPath); 
			  strcat(PluginName, libaa.cFileName); 

 			  FreeLibrary(hinstLib); 
			  hinstLib = LoadLibrary(PluginName); 

			  GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *))       GetProcAddress(hinstLib, "GetDllInfo"); 
              __try{
                GetDllInfo(&Plugin_Info);
              }
              __except(NULL,EXCEPTION_EXECUTE_HANDLER){
                goto _skipPlugin3;
              }

			  switch (Plugin_Info.Type) 
			  { 
				case PLUGIN_TYPE_GFX: 
					_VIDEO_Under_Selecting_Test          = (void (__cdecl*)(HWND))       GetProcAddress(hinstLib, "DllTest"); 
					_VIDEO_Under_Selecting_About         = (void (__cdecl*)(HWND))       GetProcAddress(hinstLib, "DllAbout"); 

					h++; 
					break; 
			  } 
_skipPlugin3:
			  if (h > ComboItemNum) bDONE = 1; 
			  else FindNextFile(FindFirst, &libaa); 
			  GetDllInfo = NULL; 
			  hinstLib = NULL; 
			} 

			bDONE = 0; 
			strcpy(temp_video_plugin, libaa.cFileName); 

			}
			__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DisplayError("Video Plugin Error");
                //No plugins of this type..do nothing
			}


			break;
		  case IDC_COMBO_AUDIO:
			__try{
			// Audio 
			FreeLibrary(hinstLib); 
			ComboItemNum = SendDlgItemMessage(hDlg, IDC_COMBO_AUDIO, CB_GETCURSEL, 0, 0); 
			FindFirst = FindFirstFile(SearchPath, &libaa); 
			bDONE = 0;

			while (bDONE == 0) 
			{ 
			  strcpy(PluginName, StartPath); 
			  strcat(PluginName, libaa.cFileName); 

			  FreeLibrary(hinstLib); 
			  hinstLib = LoadLibrary(PluginName); 
			  GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *))       GetProcAddress(hinstLib, "GetDllInfo"); 
              __try{
                GetDllInfo(&Plugin_Info); 
              }
              __except(NULL,EXCEPTION_EXECUTE_HANDLER){
                goto _skipPlugin0;
              }
			  switch (Plugin_Info.Type) 
			  { 
			  case PLUGIN_TYPE_AUDIO            : 
				_AUDIO_Under_Selecting_Test          = (void (__cdecl*)(HWND))       GetProcAddress(hinstLib, "DllTest"); 
				_AUDIO_Under_Selecting_About         = (void (__cdecl*)(HWND))       GetProcAddress(hinstLib, "DllAbout"); 

				j++; 
				break; 
			  } 
_skipPlugin0:
              if (j > ComboItemNum) bDONE = 1; 
			  else FindNextFile(FindFirst, &libaa); 
			  GetDllInfo = NULL; 
			  hinstLib = NULL; 
			} 
			bDONE = 0; 
			strcpy(temp_audio_plugin, libaa.cFileName); 

			}
			__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				DisplayError("Audio Plugin Error");
                //No plugins of this type..do nothing
			}
    
			break;
		  case IDC_COMBO_INPUT:
			// Input 
			__try{

				FreeLibrary(hinstLib); 
				ComboItemNum = SendDlgItemMessage(hDlg, IDC_COMBO_INPUT, CB_GETCURSEL, 0, 0); 
				FindFirst = FindFirstFile(SearchPath, &libaa); 
				bDONE = 0;

				while (bDONE == 0) 
				{ 
					strcpy(PluginName, StartPath); 
					strcat(PluginName, libaa.cFileName); 
					FreeLibrary(hinstLib); 
					hinstLib = LoadLibrary(PluginName); 
					GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *))       GetProcAddress(hinstLib, "GetDllInfo"); 

					__try{
						GetDllInfo(&Plugin_Info); 
					}
					__except(NULL,EXCEPTION_EXECUTE_HANDLER){
						goto _skipPlugin1;
					}
					switch (Plugin_Info.Type) 
					{ 
					case PLUGIN_TYPE_CONTROLLER : 
						_CONTROLLER_Under_Selecting_DllAbout         = (void (__cdecl*)(HWND))       GetProcAddress(hinstLib, "DllAbout"); 
						_CONTROLLER_Under_Selecting_DllTest     = (void (__cdecl*)(HWND))       GetProcAddress(hinstLib, "DllTest"); 
						i++; 
						break; 
					} 

_skipPlugin1:              
					if (i > ComboItemNum) bDONE = 1; 
					else FindNextFile(FindFirst, &libaa); 

					GetDllInfo = NULL; 
					hinstLib = NULL; 
				} 
				strcpy(temp_input_plugin, libaa.cFileName); 
			}
			__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				//No plugins of this type..do nothing
			}
			break;
		}        
    
	}
	} 
  return (FALSE); 
}
//---------------------------------------------------------------------------------------


long FAR PASCAL MainWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    static PAINTSTRUCT ps;
    static int ok = 0;
    static int IsFullScreen = 0;
	static BOOL gamepausebyinactive = FALSE;
    RECT ClientRect;

    switch (message) 
    {
		case WM_ACTIVATE:
			if( pause_at_inactive )
			{
				BOOL minimize = (BOOL) HIWORD(wParam);
				switch (LOWORD(wParam))
				{
				case WA_ACTIVE:
				case WA_CLICKACTIVE:
					if( Emu_Is_Running && gamepaused && gamepausebyinactive )
					{
						Resume();
						gamepausebyinactive = FALSE;
					}
					break;
				case WA_INACTIVE:
					if( minimize && Emu_Is_Running && gamepaused == FALSE )
					{
						Pause();
						gamepausebyinactive = TRUE;
					}
					break;
				}
			}
			break;

        case WM_PAINT:
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            return 0;

        case WM_ACTIVATEAPP:
            ActiveApp = wParam;
//            if(INPUT_ApplicationSyncAcquire)                   damn :)
//                INPUT_ApplicationSyncAcquire(hWnd,ActiveApp);
            break;

        case WM_SETFOCUS: 
            ActiveApp = wParam;
/*            if(INPUT_ApplicationSyncAcquire)
            {
                INPUT_ApplicationSyncAcquire(hWnd,ActiveApp);
            }*/ //damn again :)
            break;

        case WM_MOVE:
			if (ok == 1)
			{
				GetClientRect(hwnd, &ClientRect);
				VIDEO_MoveScreen(ClientRect.left, ClientRect.top);
			}
			ok = 1;
            break;
		case WM_EXITSIZEMOVE:
			if( !Emu_Is_Running && !window_is_moving)
			{
				DefWindowProc(hWnd, message, wParam, lParam);
				NewRomList_ListViewChangeWindowRect();
			}

			if( !window_is_moving )
			{
				DockStatusBar();
			}
			window_is_moving = FALSE;
			break;
		case WM_SIZE:
			if( wParam == SIZE_MAXIMIZED && !window_is_maximized)
			{
				window_is_maximized = TRUE;
				if( !Emu_Is_Running )
				{
					NewRomList_ListViewChangeWindowRect();
				}

				DockStatusBar();
			}
			else if( wParam == SIZE_MINIMIZED && !window_is_minimized )
			{
				window_is_minimized = TRUE;
			}
			else if( window_is_maximized || window_is_minimized )
			{
				window_is_maximized = FALSE;
				window_is_minimized = FALSE;
				if( !Emu_Is_Running )
				{
					NewRomList_ListViewChangeWindowRect();
				}

				DockStatusBar();
			}
			break;
		case WM_MOVING:
			window_is_moving = TRUE;
			break;

		case WM_KEYDOWN:
			CONTROLLER_WM_KeyDown(wParam, lParam);
			break;

		case WM_KEYUP:
	        switch (wParam)
			{
				case VK_ESCAPE: 
					if( Emu_Is_Running || IsFullScreen )
					{
						IsFullScreen ^= 1;
						VIDEO_ChangeWindow(IsFullScreen);
					}
					else if( Is_Reading_Rom_File )
					{
						To_Stop_Reading_Rom_File = TRUE;
					}
					break;
				case VK_F2:
					Play();
					break;
				case VK_F3:
					Pause();
					break;
				case VK_F4:
					Stop();
					break;
				case VK_F5:
					SaveState();
					break;
				case VK_F7:
					LoadState();
					break;
				case VK_F9:
					CodeList_ApplyAllCode( GSBUTTON );
					auto_apply_cheat_code_enabled = TRUE;
					break;
				case VK_F10:
					KillCPUThread();
					break;
				case VK_F12:
					CaptureScreenToFile();
					break;
				case 0x30:
				case 0x31:
				case 0x32:
				case 0x33:
				case 0x34:
				case 0x35:
				case 0x36:
				case 0x37:
				case 0x38:
				case 0x39:
					StateSetNumber(wParam-0x30);
					break;
				default:
					CONTROLLER_WM_KeyUp(wParam, lParam); 
					break;
			}
			break;

		case WM_SYSKEYUP:
			switch (wParam)
			{
				case VK_RETURN:
					if( Emu_Is_Running || IsFullScreen )
					{
						IsFullScreen ^= 1;
						VIDEO_ChangeWindow(IsFullScreen);
					}
				break;
			}
			break;

		case WM_NOTIFY:
			if( ((LPNMHDR) lParam)->hwndFrom == hwndRomList )
			{
				if( ! Emu_Is_Running )
				{
					switch( ((LPNMHDR) lParam)->code )
					{
					case NM_DBLCLK:
						RomListOpenRom(((LPNMLISTVIEW)lParam)->iItem);
						break;
					case NM_RCLICK:
						RomListRomOptions(((LPNMLISTVIEW)lParam)->iItem);
						break;
					case NM_CLICK:
						RomListSelectRom(((LPNMLISTVIEW)lParam)->iItem);
						EnableMenuItem(hMenu, ID_FILE_ROMINFO, MF_ENABLED);
						EnableMenuItem(hMenu, ID_FILE_CHEAT, MF_ENABLED);

					//case LVN_KEYDOWN:
					//	DisplayError("Key Down");
					default:
						return (DefWindowProc(hWnd, message, wParam, lParam));
						break;
					}
				}
			}
			else if( ((LPNMHDR) lParam)->hwndFrom == hStatusBar && ((LPNMHDR) lParam)->code == NM_DBLCLK)
			{
				int fieldno = ((LPNMLISTVIEW)lParam)->iItem;
				switch( fieldno )
				{
				case 2:	// Counter Factor
					// Reset Counter Factor to default value
					SendMessage(hwnd, WM_COMMAND, ID_CF_CF1, 0);
					break;
				case 4:	// CPU core
					//Switch CPU core
					if( Emu_Is_Running )
						SendMessage(hwnd, WM_COMMAND, whichcore == DYNACOMPILER?ID_INTERPRETER:ID_DYNAMICCOMPILER, 0);
					else
						SendMessage(hwnd, WM_COMMAND, defaultoptions.Emulator == DYNACOMPILER?ID_INTERPRETER:ID_DYNAMICCOMPILER, 0);
					break;
				}
			}
			else
				return (DefWindowProc(hWnd, message, wParam, lParam));
			break;

        case WM_COMMAND:
			if( block_menu ) break;		// ok, all menu commands are blocked
            switch (wParam) 
            {
                case ID_ROM_STOP   :    Stop();				break;
                case ID_ROM_START   :   Play();				break;
                case ID_ROM_PAUSE   :	Pause();			break;
				case ID_CPU_KILL:		KillCPUThread();	break;
#ifdef WINDEBUG_1964
                case ID_RUNTO       :   Set_Breakpoint(); 	break;
#endif
                case ID_KAILLERA_MODE:	KailleraPlay();			break;
				case ID_OPENROM		:	OpenROM();			break;
				case ID_CLOSEROM	:	CloseROM();			break;
				case ID_FILE_ROMINFO:
/*					if( Emu_Is_Running )
					{
						Pause();
						RomListRomOptions(selected_rom_index);
						GenerateCurrentRomOptions();
                        Sleep(200);
                        ResumeEmulator(FALSE);	// Refresh dyna
					}
					else*/
					{
						RomListRomOptions(selected_rom_index);
					}
					break;
				case ID_FILE_FRESHROMLIST:
					if( Emu_Is_Running ) break;
					NewRomList_ListViewChangeWindowRect();
					DockStatusBar();
					ClearRomList();
					NewRomList_ListViewFreshRomList();
					SetStatusBarText(0, "Looking for ROM file in the ROM directory and Generate List");
					RomListReadDirectory( rom_directory_to_use );
					NewRomList_ListViewFreshRomList();
					SetStatusBarText(0, "Ready");
					break;
				case ID_DEFAULTOPTIONS:
					DialogBox(hInst, "DEFAULT_OPTIONS", hWnd, (DLGPROC)DefaultOptionsDialog);
					break;
				case ID_PERFERENCE_OPTIONS:
					DialogBox(hInst, "Options", hWnd, (DLGPROC)OptionsDialog);
					break;
				case ID_CHANGEDIRECTORY:
					ChangeDirectory();
					InvalidateRect(hWnd, NULL, TRUE);
					break;
				case ID_FILE_ROMDIRECTORY1:	ChangeToRecentDirectory(0); break;
				case ID_FILE_ROMDIRECTORY2:	ChangeToRecentDirectory(1); break;
				case ID_FILE_ROMDIRECTORY3:	ChangeToRecentDirectory(2); break;
				case ID_FILE_ROMDIRECTORY4:	ChangeToRecentDirectory(3); break;
				case ID_FILE_RECENTGAMES_GAME1: OpenRecentGame(0); break;
				case ID_FILE_RECENTGAMES_GAME2: OpenRecentGame(1); break;
				case ID_FILE_RECENTGAMES_GAME3: OpenRecentGame(2); break;
				case ID_FILE_RECENTGAMES_GAME4: OpenRecentGame(3); break;
				case ID_FILE_RECENTGAMES_GAME5: OpenRecentGame(4); break;
				case ID_FILE_RECENTGAMES_GAME6: OpenRecentGame(5); break;
				case ID_FILE_RECENTGAMES_GAME7: OpenRecentGame(6); break;
				case ID_FILE_RECENTGAMES_GAME8: OpenRecentGame(7); break;
				case ID_FILE_CHEAT:
					if( Emu_Is_Running )
					{
						//SuspendThread(CPUThreadHandle);
						DialogBox(hInst, "CHEAT_HACK", hWnd, (DLGPROC)CheatAndHackDialog);
						//ResumeThread(CPUThreadHandle);
					}
					else
					{
						CodeList_ReadCode(romlist[selected_rom_index]->pinientry->Game_Name);
						DialogBox(hInst, "CHEAT_HACK", hWnd, (DLGPROC)CheatAndHackDialog);
					}
					break;
                case ID_ABOUT:
                    DialogBox(hInst, "ABOUTBOX", hWnd, (DLGPROC)About);
                    break;
                case ID_CHEATS_APPLY:
                    CodeList_ApplyAllCode( GSBUTTON );
					auto_apply_cheat_code_enabled = TRUE;
					break;

                case ID_VIDEO_CONFIG: 
					if( Emu_Is_Running )
					{
						//SuspendThread(CPUThreadHandle);
						VIDEO_DllConfig(hWnd);  
						//ResumeThread(CPUThreadHandle);
					}
					else
					{
						VIDEO_DllConfig(hWnd);  
						NewRomList_ListViewChangeWindowRect();
					}

					DockStatusBar();
					break; 
                case ID_AUD_CONFIG: 
					if( Emu_Is_Running )
					{
						SuspendThread(CPUThreadHandle);
						AUDIO_DllConfig(hWnd);         
						ResumeThread(CPUThreadHandle);
					}
					else
					{
						AUDIO_DllConfig(hWnd);         
					}
					break; 
                case ID_DI_CONFIG: 
					if( Emu_Is_Running )
					{
					//	SuspendThread(CPUThreadHandle);
						CONTROLLER_DllConfig(hWnd);     
					//	ResumeThread(CPUThreadHandle);
					}
					else
					{
						CONTROLLER_DllConfig(hWnd);     
					}
					break; 
				case ID_INTERPRETER:
					CheckMenuItem(hMenu, ID_INTERPRETER, MF_CHECKED);
					CheckMenuItem(hMenu, ID_STATICCOMPILER, MF_UNCHECKED);
					CheckMenuItem(hMenu, ID_DYNAMICCOMPILER, MF_UNCHECKED);
					EmulatorSetCore(INTERPRETER);
					break;
				case ID_STATICCOMPILER:	
					CheckMenuItem(hMenu, ID_INTERPRETER, MF_UNCHECKED);
					CheckMenuItem(hMenu, ID_STATICCOMPILER, MF_CHECKED);
					CheckMenuItem(hMenu, ID_DYNAMICCOMPILER, MF_UNCHECKED);
					EmulatorSetCore(1);
					break;
				case ID_DYNAMICCOMPILER:
					CheckMenuItem(hMenu, ID_INTERPRETER, MF_UNCHECKED);
					CheckMenuItem(hMenu, ID_STATICCOMPILER, MF_UNCHECKED);
					CheckMenuItem(hMenu, ID_DYNAMICCOMPILER, MF_CHECKED);
					EmulatorSetCore(DYNACOMPILER);
					break;

				case ID_4KEEPROM:
					if( eepromsize  == EEPROM_SIZE )
					{
						eepromsize = EEPROM_SIZE_4KB;	// set to 4K
						CheckMenuItem(hMenu, ID_4KEEPROM, MF_CHECKED);
					}
					else
					{
						eepromsize = EEPROM_SIZE;
						CheckMenuItem(hMenu, ID_4KEEPROM, MF_UNCHECKED);
					}
					break;

				case ID_CF_CF1:	SetCounterFactor(1);	break;
				case ID_CF_CF2:	SetCounterFactor(2);	break;
				case ID_CF_CF3:	SetCounterFactor(3);	break;
				case ID_CF_CF4:	SetCounterFactor(4);	break;
				case ID_CF_CF5:	SetCounterFactor(5);	break;
				case ID_CF_CF6:	SetCounterFactor(6);	break;
				case ID_CF_CF7:	SetCounterFactor(7);	break;
				case ID_CF_CF8:	SetCounterFactor(8);	break;
				case ID_CPU_DYNACODECHECKING_NOCHECK:		SetCodeCheckMethod(1);	break;
				case ID_CPU_DYNACODECHECKING_DMA:			SetCodeCheckMethod(2);	break;
				case ID_CPU_DYNACODECHECKING_DWORD:			SetCodeCheckMethod(3);	break;
				case ID_CPU_DYNACODECHECKING_QWORD:			SetCodeCheckMethod(4);	break;
				case ID_CPU_DYNACODECHECKING_QWORDANDDMA:	SetCodeCheckMethod(5);	break;
				case ID_CPU_DYNACODECHECKING_BLOCK:			SetCodeCheckMethod(6);	break;
				case ID_CPU_DYNACODECHECKING_BLOCKANDDMA:	SetCodeCheckMethod(7);	break;
                case IDM_320_240    : VIDEO_ExtraChangeResolution(hWnd, 320, NULL); break;
                case IDM_640_480    : VIDEO_ExtraChangeResolution(hWnd, 640, NULL); break;
                case IDM_800_600    : VIDEO_ExtraChangeResolution(hWnd, 800, NULL); break;
                case IDM_1024_768   : VIDEO_ExtraChangeResolution(hWnd, 1024, NULL);break;
                case IDM_1280_1024  : VIDEO_ExtraChangeResolution(hWnd, 1280, NULL);break;
                case IDM_1600_1200  : VIDEO_ExtraChangeResolution(hWnd, 1600, NULL);break;
                case IDM_1920_1440  : VIDEO_ExtraChangeResolution(hWnd, 1920, NULL);break;
                case IDM_FULLSCREEN : 
					if( Emu_Is_Running || IsFullScreen )
					{
						IsFullScreen ^= 1;
						VIDEO_ChangeWindow(IsFullScreen);
					}
					break;
				case ID_PLUGINS_SCREENSHOTS:
					CaptureScreenToFile();
					break;
                case IDM_PLUGINS    : 
					DialogBox(hInst, "PLUGINS", hWnd, (DLGPROC)PluginsDialog);            
					break;
                case ID_CHECKWEB    : 
					ShellExecute( hwnd, "open", "http://1964.emulation64.com", NULL, NULL, SW_MAXIMIZE); 
					break;

                case ID_ONLINE_HELP : 
					ShellExecute( hwnd, "open", "http://1964.emulation64.com/help.htm", NULL, NULL, SW_MAXIMIZE); 
					break;


                // NooTe 22/07/99
                case ID_CONFIGURE_VIDEO :   VIDEO_DllConfig(hWnd);  break;
                case ID_HELP_FINDER: DisplayError("Help contents"); break;
                case ID_ABOUT_WARRANTY:
                    LoadString(hInst, IDS_WARRANTY_SEC11, WarrantyPart1, 700);
                    LoadString(hInst, IDS_WARRANTY_SEC12, WarrantyPart2, 700);
                    MessageBox(hwnd, WarrantyPart1, "NO WARRANTY", MB_OK);
                    MessageBox(hwnd, WarrantyPart2, "NO WARRANTY", MB_OK);
                    break;

                case ID_REDISTRIBUTE:
                    DialogBox(hInst, "REDISTRIB_DIALOG", hWnd, (DLGPROC)ConditionsDialog);
                    break;
#ifdef DEBUG_COMMON
                case ID_DEBUGGER_OPEN:
					if( Emu_Is_Running )
					{
						PauseEmulator();
	                    OpenDebugger();
						ResumeEmulator(FALSE);	// Need to init emu
					}
					else
					{
						OpenDebugger();
					}
                    break;
                case ID_DEBUGGER_CLOSE:
					if( Emu_Is_Running )
					{
						PauseEmulator();
	                    CloseDebugger();
						ResumeEmulator(FALSE);	// Need to init emu
					}
					else
					{
	                    CloseDebugger();
					}
                    break;
				case ID_DEBUGTLB:
				case ID_DEBUGTLBINDETAIL:

				case ID_DEBUGAUDIOTASK:
				
				case ID_DEBUGIO:
				case ID_DEBUGIOSI:
				case ID_DEBUGIOSP:
				case ID_DEBUGIOVI:
				case ID_DEBUGIOAI:
				case ID_DEBUGIOMI:
				case ID_DEBUGIORI:
				case ID_DEBUGIOPI:
				case ID_DEBUGIODP:
				case ID_DEBUGIODPS:
				case ID_DEBUGIORDRAM:

				case ID_DEBUGSPTASK:
				case ID_DEBUGSITASK:
				case ID_DEBUGPIDMA:
				case ID_DEBUGSIDMA:
				case ID_DEBUGSPDMA:
				
				case ID_DEBUGMEMPAK:
				case ID_DEBUGEEPROM:
				case ID_DEBUG_CONTROLLER:
				case ID_DEBUG_SRAM:

				case ID_INTERRUPTDEBUGGING:
				case ID_DEBUGVIINTERRUPTS:
				case ID_DEBUGPIINTERRUPTS:
				case ID_DEBUGAIINTERRUPTS:
				case ID_DEBUGSIINTERRUPTS:
				case ID_DEBUGCOMPAREINTERRUPTS:
				case ID_DEBUGCPUCOUNTER:
				case ID_DEBUGDYNA:
				case ID_DEBUGDYNAEXECUTION:
				case ID_DYNALOG:
				case ID_DEBUG_64BITFPU:
					ToggleDebugOptions(wParam);
					break;
#endif
#ifdef ENABLE_OPCODE_DEBUGGER
				case ID_OPCODEDEBUGGER:
					debug_opcode = 1 - debug_opcode;
					if( debug_opcode )
						CheckMenuItem(hMenu, ID_OPCODEDEBUGGER, MF_CHECKED);
					else
						CheckMenuItem(hMenu, ID_OPCODEDEBUGGER, MF_UNCHECKED);
					if( Emu_Is_Running )
					{
						PauseEmulator();
						Debugger_Copy_Memory(&gMemoryState_Interpreter_Compare, &gMemoryState);
						memcpy( &gHardwareState_Interpreter_Compare, &gHardwareState,		sizeof(HardwareState));
						ResumeEmulator(FALSE);	// Need to init emu
					}
					break;
				case ID_OPCODEDEBUGGER_BLOCK_ONLY:
					debug_opcode_block = 1 - debug_opcode_block;
					if( debug_opcode_block )
						CheckMenuItem(hMenu, ID_OPCODEDEBUGGER_BLOCK_ONLY, MF_CHECKED);
					else
						CheckMenuItem(hMenu, ID_OPCODEDEBUGGER_BLOCK_ONLY, MF_UNCHECKED);
					if( Emu_Is_Running )
					{
						if( debug_opcode == 0 && debug_opcode_block )
						{
							debug_opcode = 1;
							CheckMenuItem(hMenu, ID_OPCODEDEBUGGER, MF_CHECKED);
						}
						PauseEmulator();
						Debugger_Copy_Memory(&gMemoryState_Interpreter_Compare, &gMemoryState);
						memcpy( &gHardwareState_Interpreter_Compare, &gHardwareState,		sizeof(HardwareState));
						ResumeEmulator(FALSE);	// Need to init emu
					}
					break;
				case ID_DIRTYONLY:
					debug_dirty_only = 1 - debug_dirty_only;
					if( debug_dirty_only && debug_opcode )
						CheckMenuItem(hMenu, ID_DIRTYONLY, MF_CHECKED);
					else
						CheckMenuItem(hMenu, ID_DIRTYONLY, MF_UNCHECKED);
					if( Emu_Is_Running )
					{
						if( debug_opcode == 0 && debug_dirty_only )
						{
							debug_opcode = 1;
							CheckMenuItem(hMenu, ID_OPCODEDEBUGGER, MF_CHECKED);
						}
						PauseEmulator();
						Debugger_Copy_Memory(&gMemoryState_Interpreter_Compare, &gMemoryState);
						memcpy( &gHardwareState_Interpreter_Compare, &gHardwareState,		sizeof(HardwareState));
						ResumeEmulator(FALSE);	// Need to init emu
					}
					break;
#endif
				case ID_SAVE_STATE_1:
				case ID_SAVE_STATE_2:
				case ID_SAVE_STATE_3:
				case ID_SAVE_STATE_4:
				case ID_SAVE_STATE_5:
				case ID_SAVE_STATE_6:
				case ID_SAVE_STATE_7:
				case ID_SAVE_STATE_8:
				case ID_SAVE_STATE_9:
				case ID_SAVE_STATE_0:
					SaveStateByNumber(wParam);
					break;
				case ID_LOAD_STATE_1:
				case ID_LOAD_STATE_2:
				case ID_LOAD_STATE_3:
				case ID_LOAD_STATE_4:
				case ID_LOAD_STATE_5:
				case ID_LOAD_STATE_6:
				case ID_LOAD_STATE_7:
				case ID_LOAD_STATE_8:
				case ID_LOAD_STATE_9:
				case ID_LOAD_STATE_0:
					LoadStateByNumber(wParam);
					break;
				case ID_SAVESTATE:
					SaveStateByDialog();
					break;
				case ID_LOADSTATE:
					LoadStateByDialog();
					break;
#ifdef DEBUG_COMMON
				case ID_SETBREAKPOINT:
					{
						BOOL breakpointsetted = RUN_TO_ADDRESS_ACTIVE;
						Set_Breakpoint();
						if( Emu_Is_Running && breakpointsetted != RUN_TO_ADDRESS_ACTIVE )
						{
							PauseEmulator();
							ResumeEmulator(FALSE);	// Need to init emu
						}
					}
					break;
				case ID_CLEAR_BREAKPOINT:
					if( RUN_TO_ADDRESS_ACTIVE )
					{
						Clear_Breakpoint();
						if( Emu_Is_Running )
						{
							PauseEmulator();
							ResumeEmulator(FALSE);	// Need to init emu
						}
					}
					break;
#endif
                case ID_EXIT:
					KillTimer(hWnd, timer);
					Exit1964();
					break;
            }
            break;

		case WM_ENTERMENULOOP:
            //            SuspendThread(CPUThreadHandle); //Disabled for 0.6.4 - schibo
			// To pause game when user enter the menu bar
			break;
		case WM_EXITMENULOOP:
			// To resume game when user leave the menu bar
			break;

        case WM_CLOSE:
			KillTimer(hWnd, timer);
			Exit1964();
            break;
        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0l);
}

//---------------------------------------------------------------------------------------
void Pause() 
{
	if( Emu_Is_Running )
	{
		if( !gamepaused  )
		{
			PauseEmulator();
		}
		else
		{
			ResumeEmulator(FALSE);	// Resume emulator always InitEmu, why not
			sprintf(generalmessage, "%s - Running", szWindowTitle);
			SetWindowText(hwnd, generalmessage);
		}
	}
}

void Resume()
{
	if( Emu_Is_Running && gamepaused )
		ResumeEmulator(FALSE);
}

void AfterStop()
{
	Emu_Is_Running = FALSE;
	EnableMenuItem(hMenu, ID_OPENROM, MF_ENABLED);
	EnableMenuItem(hMenu, IDM_PLUGINS, MF_ENABLED);
	EnableMenuItem(hMenu, ID_CLOSEROM, MF_ENABLED);
	EnableMenuItem(hMenu, ID_ROM_START, MF_ENABLED);
	EnableMenuItem(hMenu, ID_ROM_PAUSE, MF_GRAYED);
	//EnableMenuItem(hMenu, ID_ROM_STOP, MF_GRAYED);
    EnableMenuItem(hMenu, ID_PLUGINS_SCREENSHOTS, MF_GRAYED);
	DisableStateMenu();

	ShowWindow(hwndRomList, SW_SHOW);
	//EnableWindow(hwndRomList, TRUE);
	ListView_SetExtendedListViewStyle(hwndRomList, LVS_EX_FULLROWSELECT );	//| LVS_EX_TRACKSELECT );
	NewRomList_ListViewShowHeader(hwndRomList);

	//refresh the rom list, just to prevent user has changed resolution
	NewRomList_ListViewChangeWindowRect();
	DockStatusBar();

	// Reset some of the default options
	defaultoptions.Emulator = DYNACOMPILER;
	whichcore = defaultoptions.Emulator;
	SendMessage(hwnd, WM_COMMAND, whichcore == DYNACOMPILER?ID_DYNAMICCOMPILER:ID_INTERPRETER, 0);
	SetStatusBarText(4, whichcore == DYNACOMPILER ? "D" : "I");
	SetCounterFactor(defaultoptions.Counter_Factor);
	SetCodeCheckMethod(defaultoptions.Code_Check);

	// Flash the status bar
	ShowWindow(hStatusBar, SW_HIDE);
	ShowWindow(hStatusBar, SW_SHOW);
	SetStatusBarText(3, defaultoptions.RDRAM_Size == RDRAMSIZE_4MB ? "4MB":"8MB");

	sprintf(generalmessage, "%s - Stopped", szWindowTitle);
	SetWindowText(hwnd, generalmessage);
	SetStatusBarText(0,"Ready");
}

//---------------------------------------------------------------------------------------
void KillCPUThread()
{
    if ( Emu_Is_Running )
    {
        SuspendThread(CPUThreadHandle);
        TerminateThread(CPUThreadHandle, 1);
        CloseHandle(CPUThreadHandle);

		AUDIO_RomClosed();
		CONTROLLER_RomClosed();
		VIDEO_RomClosed();
		//Free_Dynarec();

		AfterStop();
    }
}

void Kill() 
{
	StopEmulator();
    CPUThreadHandle = NULL;
}

//---------------------------------------------------------------------------------------

void Play()
{
	int core;
        
    if (Rom_Loaded)
    {
		if( Emu_Is_Running )
			Stop();

		PrepareBeforePlay();

		core = currentromoptions.Emulator;
		if( core == DYNACOMPILER )
		{	// Dynarec
			CheckMenuItem(hMenu, ID_INTERPRETER, MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_DYNAMICCOMPILER, MF_CHECKED);
			whichcore = DYNACOMPILER;
		}
		else
		{	// Interpreter
			CheckMenuItem(hMenu, ID_INTERPRETER, MF_CHECKED);
			CheckMenuItem(hMenu, ID_DYNAMICCOMPILER, MF_UNCHECKED);
			whichcore = INTERPRETER;
		}

		r4300i_Reset();
		RunEmulator(whichcore);

		EnableMenuItem(hMenu, ID_OPENROM, MF_GRAYED);
		EnableMenuItem(hMenu, IDM_PLUGINS, MF_GRAYED);
		if(Kaillera_Is_Running == FALSE)
		{
			EnableMenuItem(hMenu, ID_ROM_PAUSE, MF_ENABLED);
			EnableMenuItem(hMenu, ID_ROM_STOP, MF_ENABLED);
			EnableStateMenu();
		}
		else
		{
			EnableMenuItem(hMenu, ID_FILE_CHEAT, MF_GRAYED);
			EnableMenuItem(hMenu, ID_CLOSEROM, MF_GRAYED);
			EnableMenuItem(hMenu, ID_ROM_START, MF_GRAYED);
			EnableMenuItem(hMenu, ID_ROM_PAUSE, MF_GRAYED);
			//EnableMenuItem(hMenu, ID_ROM_STOP, MF_GRAYED);
		}

		if( GfxPluginVersion == 0x0103 )
		{
		    EnableMenuItem(hMenu, ID_PLUGINS_SCREENSHOTS, MF_ENABLED);
		}
		else
		{
		    EnableMenuItem(hMenu, ID_PLUGINS_SCREENSHOTS, MF_GRAYED);
		}

		sprintf(generalmessage, "%s - Running", szWindowTitle);
		SetWindowText(hwnd, generalmessage);
    }
	else
        DisplayError("Please load a ROM first.");
}

//---------------------------------------------------------------------------------------

void Stop()
{
    if ( Emu_Is_Running )
    {
		if ( Kaillera_Is_Running == TRUE)
		{
			Kaillera_Is_Running = FALSE;
			kailleraEndGame();
		}

		StopEmulator();
		AfterStop();
	}
	else if( Is_Reading_Rom_File )
	{
		To_Stop_Reading_Rom_File = TRUE;
	}
}

//---------------------------------------------------------------------------------------
// Kaillera Stuff
//---------------------------------------------------------------------------------------

int WINAPI kailleraGameCallback(char *game, int player, int numplayers)
{
	int i;

	Kaillera_Is_Running = TRUE;
	Kaillera_Players = numplayers;
	Kaillera_Counter = 0;

	for(i=0; i<romlist_count; i++)
	{
		char szRom[50];
		ROMLIST_ENTRY *entry = romlist[i];
		
		sprintf(szRom, "%s (%X-%X:%c)", entry->pinientry->Game_Name, entry->pinientry->crc1, entry->pinientry->crc2, entry->pinientry->countrycode);

		if(strcmp(szRom, game) == 0)
		{
			RomListOpenRom(i);
		}
	}


	return 0;
}

void WINAPI kailleraChatReceivedCallback(char *nick, char *text)
{
	// Do what you want with this :)
}

void WINAPI kailleraClientDroppedCallback(char *nick, int playernb)
{
	// Do what you want with this :)
}

void WINAPI kailleraMoreInfosCallback(char *gamename)
{
	// Do what you want with this :)
}

char szKailleraNamedRoms[50*MAX_ROMLIST];

void KailleraPlay()
{
	int i;
	kailleraInfos kInfos;
	char *pszKailleraNamedRoms = szKailleraNamedRoms;
	
	// build roms list :)
	*pszKailleraNamedRoms = '\0';
	for(i=0; i<romlist_count; i++)
	{
		char szRom[50];
		ROMLIST_ENTRY *entry = romlist[i];
		
		sprintf(szRom, "%s (%X-%X:%c)", entry->pinientry->Game_Name, entry->pinientry->crc1, entry->pinientry->crc2, entry->pinientry->countrycode);
		strncpy(pszKailleraNamedRoms, szRom, strlen(szRom)+1);
		pszKailleraNamedRoms += strlen(szRom)+1;
	}
	*(++pszKailleraNamedRoms) = '\0';
	
	kInfos.appName = CURRENT1964VERSION;
	kInfos.gameList = szKailleraNamedRoms;
	kInfos.gameCallback = kailleraGameCallback;
	kInfos.chatReceivedCallback = kailleraChatReceivedCallback;
	kInfos.clientDroppedCallback = kailleraClientDroppedCallback;
	kInfos.moreInfosCallback = kailleraMoreInfosCallback;
	
	// Lock some menu items
	EnableMenuItem(hMenu, ID_KAILLERA_MODE, MF_GRAYED);
	
	kailleraSetInfos(&kInfos);
	kailleraSelectServerDialog(NULL);

	// Stop emulator if running
	Stop();
	
	// Unlock menu items
	EnableMenuItem(hMenu, ID_KAILLERA_MODE, MF_ENABLED);
}

//---------------------------------------------------------------------------------------

void CloseROM()
{
	if( Rom_Loaded )
	{
		Stop();

		Close_iPIF();
		FreeVirtualRomMemory();
		r4300i_Init();
		//sp_reset();

		Rom_Loaded = FALSE;

		EnableMenuItem(hMenu, ID_OPENROM, MF_ENABLED);
		EnableMenuItem(hMenu, IDM_PLUGINS, MF_ENABLED);
		EnableMenuItem(hMenu, ID_ROM_START, MF_GRAYED);
		EnableMenuItem(hMenu, ID_ROM_PAUSE, MF_GRAYED);
		//EnableMenuItem(hMenu, ID_ROM_STOP, MF_GRAYED);
		EnableMenuItem(hMenu, ID_CLOSEROM, MF_GRAYED);
		EnableMenuItem(hMenu, ID_FILE_ROMINFO, MF_GRAYED);
		EnableMenuItem(hMenu, ID_FILE_CHEAT, MF_GRAYED);

		SetWindowText(hwnd, szWindowTitle);
	}
//	else
//		DisplayError("Please load a ROM first.");
}
//---------------------------------------------------------------------------------------

void OpenROM() 
{
	if( Rom_Loaded )
		CloseROM();

    if (WinLoadRom() == TRUE) //If the user opened a rom,
    {
        EnableMenuItem(hMenu, ID_ROM_START, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CLOSEROM, MF_ENABLED);
		EnableMenuItem(hMenu, ID_FILE_ROMINFO, MF_ENABLED);
		EnableMenuItem(hMenu, ID_FILE_CHEAT, MF_ENABLED);

		if( auto_run_rom )
	        Play(); //autoplay
    }
}

void OpenRecentGame(int id)
{
	if( Emu_Is_Running ) return;

	if( id >= 0 && id<8 )
	{
		if( WinLoadRomStep2(recent_game_lists[id]) )
		{
			// Check and create romlist entry for this new loaded rom
			INI_ENTRY *pentry;
			pentry = GetNewIniEntry();
			ReadRomHeaderInMemory(pentry);
			RomListAddEntry(pentry,recent_game_lists[id], 0);
			DeleteIniEntryByEntry(pentry);

			// Read hack code for this rom
			CodeList_Clear();
			CodeList_ReadCode(rominfo.name);

			EnableMenuItem(hMenu, ID_ROM_START, MF_ENABLED);
			EnableMenuItem(hMenu, ID_CLOSEROM, MF_ENABLED);
			EnableMenuItem(hMenu, ID_FILE_ROMINFO, MF_ENABLED);
			EnableMenuItem(hMenu, ID_FILE_CHEAT, MF_ENABLED);

			strcpy(generalmessage, recent_game_lists[id]);
			RefreshRecentGameMenus(generalmessage);

			if( auto_run_rom )
				Play(); //autoplay
		}
	}
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

LRESULT APIENTRY CriticalMessageDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
    switch (message) {
        case WM_INITDIALOG:
            return (TRUE);
        case WM_COMMAND:
        if (wParam == IDOK ) 
		{
			critical_msg_window = 0;
            EndDialog(hDlg, TRUE);
			hCriticalMsgWnd = NULL;
	        SetActiveWindow(hwnd);
            return (TRUE);
        }
		else if( wParam == ID_CLEAR_MESSAGE )
		{
			SendDlgItemMessage(hDlg, IDC_CRITICAL_MESSAGE_TEXTBOX, WM_SETTEXT, 0, (LPARAM)"");
			critical_msg_buffer[0] = '\0';		//clear the critical message buffer
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

    
    if (OLD_WINDOW_WIDTH == VI_WIDTH_REG) return;
    
    if (!IsZoomed(hwnd))
    {
        OLD_WINDOW_WIDTH = VI_WIDTH_REG;

        AdjustWindowRect( &winrect, WS_OVERLAPPEDWINDOW, TRUE);

        width = winrect.right - winrect.left;
        height = winrect.bottom - winrect.top;

        SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW);
    }
}

#ifdef DEBUG_COMMON
// Toggle the debug options according to menu commands
void ToggleDebugOptions(WPARAM wParam)
{
	switch( wParam )
	{
	case ID_DEBUGTLB:
		debug_tlb = 1 - debug_tlb;
		if( debug_tlb )
			CheckMenuItem(hMenu, ID_DEBUGTLB, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGTLB, MF_UNCHECKED);
		break;
	case ID_DEBUGTLBINDETAIL:
		debug_tlb_detail = 1 - debug_tlb_detail;
		if( debug_tlb_detail )
			CheckMenuItem(hMenu, ID_DEBUGTLBINDETAIL, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGTLBINDETAIL, MF_UNCHECKED);
		break;
	case ID_DEBUGAUDIOTASK:
		debug_audio = 1- debug_audio;
		if( debug_audio )
			CheckMenuItem(hMenu, ID_DEBUGAUDIOTASK, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGAUDIOTASK, MF_UNCHECKED);
		break;
	case ID_DEBUGCOMPAREINTERRUPTS:
		debug_compare_interrupt = 1 - debug_compare_interrupt;
		if( debug_compare_interrupt )
			CheckMenuItem(hMenu, ID_DEBUGCOMPAREINTERRUPTS, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGCOMPAREINTERRUPTS, MF_UNCHECKED);
		break;
	case ID_DEBUGCPUCOUNTER:
		debug_cpu_counter = 1-debug_cpu_counter;
		if( debug_cpu_counter )
			CheckMenuItem(hMenu, ID_DEBUGCPUCOUNTER, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGCPUCOUNTER, MF_UNCHECKED);
		break;
	case ID_DEBUGIO:
		debug_io = 1 - debug_io;
		if( debug_io )
			CheckMenuItem(hMenu, ID_DEBUGIO, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGIO, MF_UNCHECKED);
		break;
	case ID_DEBUGIOSI:
		debug_io_si = 1-debug_io_si;
		if( debug_io_si )
			CheckMenuItem(hMenu, ID_DEBUGIOSI, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGIOSI, MF_UNCHECKED);
		break;
	case ID_DEBUGIOSP:
		debug_io_sp = 1-debug_io_sp;
		if( debug_io_sp )
			CheckMenuItem(hMenu, ID_DEBUGIOSP, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGIOSP, MF_UNCHECKED);
		break;
	case ID_DEBUGIOMI:
		debug_io_mi = 1-debug_io_mi;
		if( debug_io_mi )
			CheckMenuItem(hMenu, ID_DEBUGIOMI, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGIOMI, MF_UNCHECKED);
		break;
	case ID_DEBUGIOVI:
		debug_io_vi = 1-debug_io_vi;
		if( debug_io_vi )
			CheckMenuItem(hMenu, ID_DEBUGIOVI, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGIOVI, MF_UNCHECKED);
		break;
	case ID_DEBUGIOAI:
		debug_io_ai = 1-debug_io_ai;
		if( debug_io_ai )
			CheckMenuItem(hMenu, ID_DEBUGIOAI, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGIOAI, MF_UNCHECKED);
		break;
	case ID_DEBUGIORI:
		debug_io_ri = 1-debug_io_ri;
		if( debug_io_ri )
			CheckMenuItem(hMenu, ID_DEBUGIORI, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGIORI, MF_UNCHECKED);
		break;
	case ID_DEBUGIOPI:
		debug_io_pi = 1-debug_io_pi;
		if( debug_io_pi )
			CheckMenuItem(hMenu, ID_DEBUGIOPI, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGIOPI, MF_UNCHECKED);
		break;
	case ID_DEBUGIODP:
		debug_io_dp = 1-debug_io_dp;
		if( debug_io_dp )
			CheckMenuItem(hMenu, ID_DEBUGIODP, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGIODP, MF_UNCHECKED);
		break;
	case ID_DEBUGIODPS:
		debug_io_dps = 1-debug_io_dps;
		if( debug_io_dps )
			CheckMenuItem(hMenu, ID_DEBUGIODPS, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGIODPS, MF_UNCHECKED);
		break;
	case ID_DEBUGIORDRAM:
		debug_io_rdram = 1-debug_io_rdram;
		if( debug_io_rdram )
			CheckMenuItem(hMenu, ID_DEBUGIORDRAM, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGIORDRAM, MF_UNCHECKED);
		break;
	case ID_DEBUGSITASK:
		debug_si_task = 1 - debug_si_task;
		if( debug_si_task )
			CheckMenuItem(hMenu, ID_DEBUGSITASK, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGSITASK, MF_UNCHECKED);
		break;
	case ID_DEBUGSPTASK:
		debug_sp_task = 1 - debug_sp_task;
		if( debug_sp_task )
			CheckMenuItem(hMenu, ID_DEBUGSPTASK, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGSPTASK, MF_UNCHECKED);
		break;
	case ID_DEBUGPIDMA:
		debug_pi_dma = 1 - debug_pi_dma;
		if( debug_pi_dma )
			CheckMenuItem(hMenu, ID_DEBUGPIDMA, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGPIDMA, MF_UNCHECKED);
		break;
	case ID_DEBUGSIDMA:
		debug_si_dma = 1 - debug_si_dma;
		if( debug_si_dma )
			CheckMenuItem(hMenu, ID_DEBUGSIDMA, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGSIDMA, MF_UNCHECKED);
		break;
	case ID_DEBUGSPDMA:
		 debug_sp_dma = 1 - debug_sp_dma;
		if( debug_sp_dma )
			CheckMenuItem(hMenu, ID_DEBUGSPDMA, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGSPDMA, MF_UNCHECKED);
		break;
	case ID_DEBUGMEMPAK:
		debug_si_mempak = 1 - debug_si_mempak;
		if( debug_si_mempak )
			CheckMenuItem(hMenu, ID_DEBUGMEMPAK, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGMEMPAK, MF_UNCHECKED);
		break;
	case ID_DEBUGEEPROM:
		debug_si_eeprom = 1- debug_si_eeprom;
		if( debug_si_eeprom )
			CheckMenuItem(hMenu, ID_DEBUGEEPROM, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGEEPROM, MF_UNCHECKED);
		break;
	case ID_DEBUG_CONTROLLER:
		debug_si_controller = 1-debug_si_controller;
		if( debug_si_controller )
			CheckMenuItem(hMenu, ID_DEBUG_CONTROLLER, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUG_CONTROLLER, MF_UNCHECKED);
		break;
	case ID_DEBUG_SRAM:
		debug_sram = 1-debug_sram;
		if( debug_sram )
			CheckMenuItem(hMenu, ID_DEBUG_SRAM, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUG_SRAM, MF_UNCHECKED);
		break;
	case ID_INTERRUPTDEBUGGING:
		debug_interrupt = 1-debug_interrupt;
		if( debug_interrupt )
			CheckMenuItem(hMenu, ID_INTERRUPTDEBUGGING, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_INTERRUPTDEBUGGING, MF_UNCHECKED);
		break;
	case ID_DEBUGVIINTERRUPTS:
		debug_vi_interrupt = 1-debug_vi_interrupt;
		if( debug_vi_interrupt )
			CheckMenuItem(hMenu, ID_DEBUGVIINTERRUPTS, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGVIINTERRUPTS, MF_UNCHECKED);
		break;
	case ID_DEBUGPIINTERRUPTS:
		debug_pi_interrupt = 1-debug_pi_interrupt;
		if( debug_pi_interrupt )
			CheckMenuItem(hMenu, ID_DEBUGPIINTERRUPTS, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGPIINTERRUPTS, MF_UNCHECKED);
		break;
	case ID_DEBUGAIINTERRUPTS:
		debug_ai_interrupt = 1-debug_ai_interrupt;
		if( debug_ai_interrupt )
			CheckMenuItem(hMenu, ID_DEBUGAIINTERRUPTS, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGAIINTERRUPTS, MF_UNCHECKED);
		break;
	case ID_DEBUGSIINTERRUPTS:
		debug_si_interrupt = 1-debug_si_interrupt;
		if( debug_si_interrupt )
			CheckMenuItem(hMenu, ID_DEBUGSIINTERRUPTS, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGSIINTERRUPTS, MF_UNCHECKED);
		break;
	case ID_DEBUGDYNA:
		debug_dyna_compiler = 1 - debug_dyna_compiler;
		if( debug_dyna_compiler )
			CheckMenuItem(hMenu, ID_DEBUGDYNA, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGDYNA, MF_UNCHECKED);
		break;
	case ID_DEBUGDYNAEXECUTION:
		debug_dyna_execution = 1 - debug_dyna_execution;
		if( debug_dyna_execution )
			CheckMenuItem(hMenu, ID_DEBUGDYNAEXECUTION, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUGDYNAEXECUTION, MF_UNCHECKED);
		break;
	case ID_DYNALOG:
		debug_dyna_log = 1 - debug_dyna_log;
		if( debug_dyna_log )
			CheckMenuItem(hMenu, ID_DYNALOG, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DYNALOG, MF_UNCHECKED);
		break;
	case ID_DEBUG_64BITFPU:
		debug_64bit_fpu = 1 - debug_64bit_fpu;
		if( debug_64bit_fpu )
			CheckMenuItem(hMenu, ID_DEBUG_64BITFPU, MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_DEBUG_64BITFPU, MF_UNCHECKED);
		break;
	}
}
#endif


void SaveState()
{
	int was_running = FALSE;

	if (Rom_Loaded)
	{
		if( Emu_Is_Running )
		{
			PauseEmulator();
			was_running = TRUE;
		}

		// Wait another 2 second for audio and video plugin thread to sync.
		// 2 seconds seem to be enough
		//Sleep(2000);
		sprintf(generalmessage, "%s - Saving State %d", szWindowTitle, StateFileNumber);
		SetStatusBarText(0, generalmessage);

		FileIO_gzSaveState();

		if( was_running )
		{
			ResumeEmulator(FALSE);
		}
	}
}

void LoadState()
{
	int was_running = FALSE;

	if (Rom_Loaded)
	{
		if( Emu_Is_Running )
		{
			PauseEmulator();
			was_running = TRUE;
		}

		sprintf(generalmessage, "%s - Loading State %d", szWindowTitle, StateFileNumber);
		SetStatusBarText(0, generalmessage);

		FileIO_gzLoadState();

		if( was_running )
		{
			ResumeEmulator(TRUE);	// Need to init emu
		}
	}
}

unsigned int statesavemenulist[10] = {
	ID_SAVE_STATE_0,
	ID_SAVE_STATE_1,
	ID_SAVE_STATE_2,
	ID_SAVE_STATE_3,
	ID_SAVE_STATE_4,
	ID_SAVE_STATE_5,
	ID_SAVE_STATE_6,
	ID_SAVE_STATE_7,
	ID_SAVE_STATE_8,
	ID_SAVE_STATE_9
};
unsigned int stateloadmenulist[10] = {
	ID_LOAD_STATE_0,
	ID_LOAD_STATE_1,
	ID_LOAD_STATE_2,
	ID_LOAD_STATE_3,
	ID_LOAD_STATE_4,
	ID_LOAD_STATE_5,
	ID_LOAD_STATE_6,
	ID_LOAD_STATE_7,
	ID_LOAD_STATE_8,
	ID_LOAD_STATE_9
};

void StateSetNumber(int number)
{
	CheckMenuItem(hMenu, statesavemenulist[StateFileNumber], MF_UNCHECKED);
	CheckMenuItem(hMenu, stateloadmenulist[StateFileNumber], MF_UNCHECKED);
	StateFileNumber = number;
	CheckMenuItem(hMenu, statesavemenulist[StateFileNumber], MF_CHECKED);
	CheckMenuItem(hMenu, stateloadmenulist[StateFileNumber], MF_CHECKED);
}

void EnableStateMenu(void)
{
	int i;

	EnableMenuItem(hMenu, ID_SAVESTATE, MF_ENABLED);
	EnableMenuItem(hMenu, ID_LOADSTATE, MF_ENABLED);

	for( i=0; i<10; i++ )
	{
		EnableMenuItem(hMenu, statesavemenulist[i], MF_ENABLED);
		EnableMenuItem(hMenu, stateloadmenulist[i], MF_ENABLED);
	}
}

void DisableStateMenu(void)
{
	int i;

	EnableMenuItem(hMenu, ID_SAVESTATE, MF_GRAYED);
	EnableMenuItem(hMenu, ID_LOADSTATE, MF_GRAYED);

	for( i=0; i<10; i++ )
	{
		EnableMenuItem(hMenu, statesavemenulist[i], MF_GRAYED);
		EnableMenuItem(hMenu, stateloadmenulist[i], MF_GRAYED);
	}
}

void SaveStateByNumber(WPARAM wparam)
{
	int i;
	for( i=0; i<10; i++ )
	{
		CheckMenuItem(hMenu, statesavemenulist[i], MF_UNCHECKED);
		CheckMenuItem(hMenu, stateloadmenulist[i], MF_UNCHECKED);
		if( statesavemenulist[i] == wparam )
		{
			StateFileNumber = i;
			CheckMenuItem(hMenu, statesavemenulist[i], MF_CHECKED);
			CheckMenuItem(hMenu, stateloadmenulist[i], MF_CHECKED);
		}
	}

	SaveState();
}

void LoadStateByNumber(WPARAM wparam)
{
	int i;
	for( i=0; i<10; i++ )
	{
		CheckMenuItem(hMenu, statesavemenulist[i], MF_UNCHECKED);
		CheckMenuItem(hMenu, stateloadmenulist[i], MF_UNCHECKED);
		if( stateloadmenulist[i] == wparam )
		{
			StateFileNumber = i;
			CheckMenuItem(hMenu, statesavemenulist[i], MF_CHECKED);
			CheckMenuItem(hMenu, stateloadmenulist[i], MF_CHECKED);
		}
	}

	LoadState();
}

void SaveStateByDialog(void)
{
    OPENFILENAME ofn;
    char szFileName[MAXFILENAME];
    char szFileTitle[MAXFILENAME];
	char szPath[_MAX_PATH];

    memset(&szFileName,0,sizeof(szFileName));
    memset(&szFileTitle, 0, sizeof(szFileTitle));
	memset(szPath, 0 , _MAX_PATH);

	strcpy(szPath, save_directory_to_use);

    ofn.lStructSize         = sizeof(OPENFILENAME);
    ofn.hwndOwner           = hwnd;
    ofn.lpstrFilter         = "1964 State File (*.sav?)\0*.SAV?;*.ROM\0All Files (*.*)\0*.*\0";
    ofn.lpstrCustomFilter   = NULL;
    ofn.nMaxCustFilter      = 0;
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = szFileName;
    ofn.nMaxFile            = MAXFILENAME;
    ofn.lpstrInitialDir     = szPath;
    ofn.lpstrFileTitle      = szFileTitle;
    ofn.nMaxFileTitle       = MAXFILENAME;
    ofn.lpstrTitle          = "Save State";
    ofn.lpstrDefExt         = "";
    ofn.Flags               = OFN_ENABLESIZING | OFN_HIDEREADONLY;

    if (!GetOpenFileName ((LPOPENFILENAME)&ofn))
	{
		return;
	}
	else
	{
		if (Rom_Loaded)
		{
			BOOL was_running = FALSE;
			if( Emu_Is_Running )
			{
				PauseEmulator();
				was_running = TRUE;
			}

			FileIO_gzSaveStateFile(szFileName);

			if( was_running )
			{
				ResumeEmulator(TRUE);	// Need to init emu
			}
		}
	}

}

void LoadStateByDialog(void)
{
    OPENFILENAME ofn;
    char szFileName[MAXFILENAME];
    char szFileTitle[MAXFILENAME];
	char szPath[_MAX_PATH];

    memset(&szFileName,0,sizeof(szFileName));
    memset(&szFileTitle, 0, sizeof(szFileTitle));
	memset(szPath, 0 , _MAX_PATH);

	strcpy(szPath, save_directory_to_use);

    ofn.lStructSize         = sizeof(OPENFILENAME);
    ofn.hwndOwner           = hwnd;
    ofn.lpstrFilter         = "1964 State File (*.sav?)\0*.SAV?;*.ROM\0All Files (*.*)\0*.*\0";
    ofn.lpstrCustomFilter   = NULL;
    ofn.nMaxCustFilter      = 0;
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = szFileName;
    ofn.nMaxFile            = MAXFILENAME;
    ofn.lpstrInitialDir     = szPath;
    ofn.lpstrFileTitle      = szFileTitle;
    ofn.nMaxFileTitle       = MAXFILENAME;
    ofn.lpstrTitle          = "Load State";
    ofn.lpstrDefExt         = "";
    ofn.Flags               = OFN_ENABLESIZING | OFN_HIDEREADONLY;

    if (!GetOpenFileName ((LPOPENFILENAME)&ofn))
	{
		return;
	}
	else
	{
		if (Rom_Loaded)
		{
			BOOL was_running=FALSE;
			if( Emu_Is_Running )
			{
				PauseEmulator();
				was_running = TRUE;
			}

		FileIO_gzLoadStateFile(szFileName);

			if( was_running )
			{
				ResumeEmulator(TRUE);	// Need to init emu
			}
		}
	}
}

void Free_Dynarec();

static BOOL exiting_1964 = FALSE;
void Exit1964()
{
	WINDOWPLACEMENT placement;

	if( exiting_1964 )
		exit(0);

	exiting_1964 = TRUE;

	SetStatusBarText(0, "Exiting 1964");

	placement.length = sizeof( WINDOWPLACEMENT );
	GetWindowPlacement(hwnd, &placement);
	if( placement.rcNormalPosition.right - placement.rcNormalPosition.left > (1024+1280)/2 )
		clientwidth = 1280;
	else if( placement.rcNormalPosition.right - placement.rcNormalPosition.left > (800+1024)/2 )
		clientwidth = 1024;
	else if( placement.rcNormalPosition.right - placement.rcNormalPosition.left > (800+640)/2 )
		clientwidth = 800;
	else if( placement.rcNormalPosition.right - placement.rcNormalPosition.left > (640+320)/2 )
		clientwidth = 640;
	else
		clientwidth = 320;

	window_position.left = placement.rcNormalPosition.left;
	window_position.top = placement.rcNormalPosition.top;

	if( Emu_Is_Running )
		Stop();

	Close_iPIF();	//save mempak and eeprom
	
	FreeVirtualMemory();

	FileIO_Write1964Ini();	// Save 1964.ini
	WriteConfiguration();
	DeleteAllIniEntries();		// Release all ini entries
	ClearRomList();			// Clean the Rom List

	FreePlugins();

	kailleraShutdown();
	UnloadDllKaillera();

	// Here is the fix for the problem that 1964 crash when exiting if using opengl plugins.
	// I don't know why 1964 crash, looks like crash is not happen in 1964, but dll related.
	// just doing exit(0) will not crash,(maybe we have left some resource not released, donno)
	exit(0);
	//PostQuitMessage(0);
}

BOOL SelectDirectory(char *title, char buffer[MAX_PATH])
{
    BROWSEINFO bi;
    char pszBuffer[MAX_PATH];
    LPITEMIDLIST pidl;

    bi.hwndOwner = hwnd;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = pszBuffer;
    bi.lpszTitle = title;
    bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
    bi.lpfn = NULL;
    bi.lParam = 0;

    if ((pidl = SHBrowseForFolder(&bi)) != NULL)
    {
        if (SHGetPathFromIDList(pidl, buffer))
        {
			return TRUE;
		}
	}
	return FALSE;
}

void ChangeDirectory()
{
	char path[MAX_PATH];

	if( Emu_Is_Running ) return;

	if( SelectDirectory("Selecting a ROM directory", path) )
	{
		strcpy(rom_directory_to_use, path);
		strcpy(last_rom_directory, path);
		WriteConfiguration();
		RefreshRecentRomDirectoryMenus(path);

		ClearRomList();
		SetStatusBarText(0, "Looking for ROM file(s) in the ROM directory and Generating List");
		RomListReadDirectory( rom_directory_to_use );
		NewRomList_ListViewFreshRomList();
		SetStatusBarText(0, "Ready");
    }
}

void ChangeToRecentDirectory(int id)
{
	if( Emu_Is_Running ) return;

	if( id >= 0 && id<4 )
	{
		strcpy(generalmessage, recent_rom_directory_lists[id]);
		strcpy(rom_directory_to_use, generalmessage);
		strcpy(last_rom_directory, generalmessage);
		WriteConfiguration();
		RefreshRecentRomDirectoryMenus(generalmessage);

		ClearRomList();
		SetStatusBarText(0, "Looking for ROM file in the ROM directory and Generate List");
		RomListReadDirectory( rom_directory_to_use );
		NewRomList_ListViewFreshRomList();
		SetStatusBarText(0, "Ready");
	}
}

LRESULT APIENTRY DefaultOptionsDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam) 
{ 
    int i; 
	switch (message) 
	{ 
	case WM_INITDIALOG:
		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_EXPANSIONPAK,		BM_SETCHECK, defaultoptions.RDRAM_Size==RDRAMSIZE_8MB?BST_CHECKED:BST_UNCHECKED,			0);
		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_USETLB,				BM_SETCHECK, defaultoptions.Use_TLB==USETLB_YES?BST_CHECKED:BST_UNCHECKED,		0);
		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_DMASEGMENTATION,	BM_SETCHECK, dma_in_segments==USEDMASEG_YES?BST_CHECKED:BST_UNCHECKED,			0);
		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_REGC,				BM_SETCHECK, defaultoptions.Use_Register_Caching==USEREGC_YES?BST_CHECKED:BST_UNCHECKED,		0);
		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_FPUHACK,			BM_SETCHECK, defaultoptions.FPU_Hack==USEFPUHACK_YES?BST_CHECKED:BST_UNCHECKED,		0);

		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_SAVETYPE, CB_RESETCONTENT, 0, 0);
		for( i=1; i< 7; i++)
		{
			SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_SAVETYPE, CB_INSERTSTRING, i-1, (LPARAM)save_type_names[i]);
			if( i == defaultoptions.Save_Type )
				SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_SAVETYPE, CB_SETCURSEL, i-1, 0);
		}

		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_CODECHECK, CB_RESETCONTENT, 0, 0);
		for( i=1; i< 8; i++)
		{
			SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_CODECHECK, CB_INSERTSTRING, i-1, (LPARAM)codecheck_type_names[i]);
			if( i == defaultoptions.Code_Check )
				SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_CODECHECK, CB_SETCURSEL, i-1, 0);
		}

		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_MAXVISPEED, CB_RESETCONTENT, 0, 0);
		for( i=1; i< 5; i++)
		{
			SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_MAXVISPEED, CB_INSERTSTRING, i-1, (LPARAM)maxfps_type_names[i]);
			if( i == defaultoptions.Max_FPS )
				SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_MAXVISPEED, CB_SETCURSEL, i-1, 0);
		}

		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_EEPROMSIZE, CB_RESETCONTENT, 0, 0);
		for( i=1; i< 4; i++)
		{
			SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_EEPROMSIZE, CB_INSERTSTRING, i-1, (LPARAM)eepromsize_type_names[i]);
			if( i == defaultoptions.Eeprom_size )
				SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_EEPROMSIZE, CB_SETCURSEL, i-1, 0);
		}

		return (TRUE); 
      
	case WM_COMMAND:
		switch (wParam) 
		{
		case IDOK: 
			{ 
				// Read option setting from dialog
				defaultoptions.RDRAM_Size =		(SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_EXPANSIONPAK,	BM_GETCHECK, 0, 0) == BST_CHECKED )+1;
				if( ! Emu_Is_Running ) SetStatusBarText(3, defaultoptions.RDRAM_Size == RDRAMSIZE_4MB ? "4MB":"8MB");
				defaultoptions.Save_Type =		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_SAVETYPE,		CB_GETCURSEL, 0, 0)+1;
				defaultoptions.Code_Check =		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_CODECHECK,		CB_GETCURSEL, 0, 0)+1;
				defaultoptions.Max_FPS =		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_MAXVISPEED,		CB_GETCURSEL, 0, 0)+1;
				defaultoptions.Use_TLB =		2 - (SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_USETLB,	BM_GETCHECK, 0, 0) == BST_CHECKED );
				defaultoptions.Eeprom_size =	SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_EEPROMSIZE,		CB_GETCURSEL, 0, 0)+1;
				defaultoptions.Use_Register_Caching = (SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_REGC,	BM_GETCHECK, 0, 0) == BST_CHECKED );
				defaultoptions.Use_Register_Caching = (defaultoptions.Use_Register_Caching?USEREGC_YES:USEREGC_NO);
				defaultoptions.FPU_Hack =		(SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_FPUHACK,	BM_GETCHECK, 0, 0) == BST_CHECKED );
				defaultoptions.FPU_Hack =		(defaultoptions.FPU_Hack?USEFPUHACK_YES:USEFPUHACK_NO);
				dma_in_segments = (SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_DMASEGMENTATION, BM_GETCHECK, 0, 0) == BST_CHECKED );
				dma_in_segments = (dma_in_segments == 1 ? USEDMASEG_YES : USEDMASEG_NO );
				defaultoptions.DMA_Segmentation = dma_in_segments;

				EndDialog(hDlg, TRUE); 
				return (TRUE); 
			} 
		case IDCANCEL: 
			{ 
				EndDialog(hDlg, TRUE); 
				return (TRUE); 
			} 
		} 
    } 
	return (FALSE); 
}

LRESULT APIENTRY OptionsDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam) 
{ 
	char path[MAX_PATH];

	switch (message) 
	{ 
	case WM_INITDIALOG:
		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_AUTORUN,			BM_SETCHECK, auto_run_rom?BST_CHECKED:BST_UNCHECKED,			0);
		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_AUTOCHEAT,			BM_SETCHECK, auto_apply_cheat_code?BST_CHECKED:BST_UNCHECKED,	0);
		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIOS_PAUSEONMENU,			BM_SETCHECK, pause_at_menu?BST_CHECKED:BST_UNCHECKED,			0);
		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_PAUSEWHENINACTIVE,	BM_SETCHECK, pause_at_inactive?BST_CHECKED:BST_UNCHECKED,		0);
		SendDlgItemMessage(hDlg, IDC_EXPERT_MODE,			BM_SETCHECK, expert_user_mode?BST_CHECKED:BST_UNCHECKED,			0);
		SendDlgItemMessage(hDlg, IDC_ENABLE_DIRECTORY_LIST,	BM_SETCHECK, recent_rom_directory_list?BST_CHECKED:BST_UNCHECKED,	0);
		SendDlgItemMessage(hDlg, IDC_ENABLE_GAME_LIST,		BM_SETCHECK, recent_game_list?BST_CHECKED:BST_UNCHECKED,			0);
		SendDlgItemMessage(hDlg, IDC_ENABLE_DETAIL_STATUS,	BM_SETCHECK, display_detail_status?BST_CHECKED:BST_UNCHECKED,		0);
		SendDlgItemMessage(hDlg, IDC_ENABLE_STATE_MENU,		BM_SETCHECK, state_selector_menu?BST_CHECKED:BST_UNCHECKED,		0);
		SendDlgItemMessage(hDlg, IDC_OPTION_ERROR_WINDOW,	BM_SETCHECK, critical_msg_window?BST_CHECKED:BST_UNCHECKED,		0);
		SendDlgItemMessage(hDlg, IDC_OPTION_ROMBROWSER,		BM_SETCHECK, display_romlist?BST_CHECKED:BST_UNCHECKED,		0);

		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_USE1964SAVEDIRECTORY,		BM_SETCHECK, use_default_save_directory?BST_CHECKED:BST_UNCHECKED,			0);
		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_USE1964STATESAVEDIRECTORY,	BM_SETCHECK, use_default_state_save_directory?BST_CHECKED:BST_UNCHECKED,	0);
		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_USE1964PLUGINDIRECTORY,		BM_SETCHECK, use_default_plugin_directory?BST_CHECKED:BST_UNCHECKED,		0);
		SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_USELASTROMDIRECTORY,		BM_SETCHECK, use_last_rom_directory?BST_CHECKED:BST_UNCHECKED,				0);

		SetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_STATESAVEDIRECTORY, state_save_directory	);
		SetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_ROMDIRECTORY,		user_set_rom_directory	);
		SetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_SAVEDIRECTORY,		user_set_save_directory		);
		SetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_PLUGINDIRECTORY,	user_set_plugin_directory	);

		return (TRUE); 
      
	case WM_COMMAND:
		switch (wParam) 
		{
		case IDC_DEFAULTOPTIONS_BUTTON_SAVEDIR:
			if( SelectDirectory("Selecting Game Save Directory", path) )
			{
				strcat(path,"\\");
				SetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_SAVEDIRECTORY,		path		);
			}
			break;
		case IDC_DEFAULTOPTIONS_BUTTON_PLUGINDIR:
			if( SelectDirectory("Selecting Plugin Directory", path) )
			{
				strcat(path,"\\");
				SetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_PLUGINDIRECTORY,		path		);
			}
			break;
		case IDC_DEFAULTOPTIONS_BUTTON_ROMDIR:
			if( SelectDirectory("Selecting Default ROM Directory", path) )
			{
				strcat(path,"\\");
				SetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_ROMDIRECTORY,		path		);
			}
			break;

		case IDOK: 
			{ 
				// Read option setting from dialog
				auto_run_rom =			(SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_AUTORUN,			BM_GETCHECK, 0, 0) == BST_CHECKED );
				auto_apply_cheat_code = (SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_AUTOCHEAT,			BM_GETCHECK, 0, 0) == BST_CHECKED );
				pause_at_menu =			(SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIOS_PAUSEONMENU,		BM_GETCHECK, 0, 0) == BST_CHECKED );
				pause_at_inactive =		(SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_PAUSEWHENINACTIVE, BM_GETCHECK, 0, 0) == BST_CHECKED );

				if( expert_user_mode !=	(SendDlgItemMessage(hDlg, IDC_EXPERT_MODE,			BM_GETCHECK, 0, 0) == BST_CHECKED ) )
				{
					expert_user_mode = 1 - expert_user_mode;
					if( expert_user_mode )
						RegenerateAdvancedUserMenus();
					else
					{
						DeleteAdvancedUserMenus();
					}
				}

				if( recent_rom_directory_list != (SendDlgItemMessage(hDlg, IDC_ENABLE_DIRECTORY_LIST,BM_GETCHECK, 0, 0) == BST_CHECKED ) )
				{
					recent_rom_directory_list = 1 - recent_rom_directory_list;
					if( recent_rom_directory_list )
						RegerateRecentRomDirectoryMenus();
					else
					{
						DeleteRecentRomDirectoryMenus();
					}
				}

				if (recent_game_list !=	(SendDlgItemMessage(hDlg, IDC_ENABLE_GAME_LIST,		BM_GETCHECK, 0, 0) == BST_CHECKED ) )
				{
					recent_game_list = 1 - recent_game_list;
					if( recent_game_list )
						RegerateRecentGameMenus();
					else
					{
						DeleteRecentGameMenus();
					}
				}

				if( state_selector_menu !=	(SendDlgItemMessage(hDlg, IDC_ENABLE_STATE_MENU,	BM_GETCHECK, 0, 0) == BST_CHECKED ) )
				{
					state_selector_menu = 1 - state_selector_menu;
					if( state_selector_menu )
						RegenerateStateSelectorMenus();
					else
					{
						DeleteStateSelectorMenus();
					}
				}

				if( critical_msg_window != (SendDlgItemMessage(hDlg, IDC_OPTION_ERROR_WINDOW, BM_GETCHECK, 0, 0) == BST_CHECKED ) )
				{
					critical_msg_window = 1 - critical_msg_window;
					if( critical_msg_window )
					{
						if( hCriticalMsgWnd == NULL )
						{
							hCriticalMsgWnd = CreateDialog(hInst, "CRITICAL_MESSAGE", NULL, (DLGPROC)CriticalMessageDialog);
					        SetActiveWindow(hwnd);
						}
					}
					else
					{
						if( hCriticalMsgWnd != NULL )
						{
					        DestroyWindow(hCriticalMsgWnd);
							hCriticalMsgWnd = NULL;
						}
					}
				}
				
				if( display_romlist != (SendDlgItemMessage(hDlg, IDC_OPTION_ROMBROWSER,	BM_GETCHECK, 0, 0) == BST_CHECKED ) )
				{
					display_romlist = 1 - display_romlist;
					EndDialog(hDlg, TRUE); 
					SendMessage(hwnd, WM_COMMAND, ID_FILE_FRESHROMLIST, 0);
					return (TRUE);
				}

				display_detail_status =		(SendDlgItemMessage(hDlg, IDC_ENABLE_DETAIL_STATUS, BM_GETCHECK, 0, 0) == BST_CHECKED );
				use_default_save_directory =		(SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_USE1964SAVEDIRECTORY,		BM_GETCHECK, 0, 0) == BST_CHECKED );
				use_default_state_save_directory =	(SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_USE1964STATESAVEDIRECTORY, BM_GETCHECK, 0, 0) == BST_CHECKED );
				use_default_plugin_directory =		(SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_USE1964PLUGINDIRECTORY,	BM_GETCHECK, 0, 0) == BST_CHECKED );
				use_last_rom_directory =			(SendDlgItemMessage(hDlg, IDC_DEFAULTOPTIONS_USELASTROMDIRECTORY,		BM_GETCHECK, 0, 0) == BST_CHECKED );

				GetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_ROMDIRECTORY,		user_set_rom_directory,		_MAX_PATH);
				GetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_SAVEDIRECTORY,		user_set_save_directory,	_MAX_PATH);
				GetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_STATESAVEDIRECTORY, state_save_directory,		_MAX_PATH);
				GetDlgItemText(hDlg, IDC_DEFAULTOPTIONS_PLUGINDIRECTORY,	user_set_plugin_directory,	_MAX_PATH);

				// Set the save directory to use
				if( use_default_save_directory )
					strcpy(save_directory_to_use, default_save_directory);
				else
					strcpy(save_directory_to_use, user_set_save_directory);

				// Set the ROM directory to use
				if( use_last_rom_directory )
					strcpy(rom_directory_to_use, last_rom_directory);
				else
					strcpy(rom_directory_to_use, user_set_rom_directory);

				// Set the plugin directory to use
				if( use_default_plugin_directory )
					strcpy(plugin_directory_to_use, default_plugin_directory);
				else
					strcpy(plugin_directory_to_use, user_set_plugin_directory);

				EndDialog(hDlg, TRUE); 
				return (TRUE); 
			} 
		case IDCANCEL: 
			{ 
				EndDialog(hDlg, TRUE); 
				return (TRUE); 
			} 
		} 
    } 
	return (FALSE); 
}

// How to set Counter Factor for both VI and COUNT
//
//
//		Factor		VI Factor		COUNT Factor
//		----------------------------------------
//		1			1				1/2
//		2			1				1
//		3			2				1
//		4			2				2
//		5			4				2
//		6			4				4
//		7			8				4
//		8			8				8


void SetCounterFactor(int factor)
{
	if( CounterFactor != factor )
	{
		CheckMenuItem(hMenu, cfmenulist[CounterFactor-1], MF_UNCHECKED);
		if( Emu_Is_Running )
		{
			PauseEmulator();
			CounterFactor = factor;
			ResumeEmulator(FALSE);	// Need to init emu
		}
		CounterFactor = factor;
		CheckMenuItem(hMenu, cfmenulist[CounterFactor-1], MF_CHECKED);
		sprintf(generalmessage,"CF=%d",factor);
		SetStatusBarText(2,generalmessage);
	}
}



void SetCodeCheckMethod(int method)
{
	if( Emu_Is_Running )
	{
		CheckMenuItem(hMenu, codecheckmenulist[CodeCheckMethod-1], MF_UNCHECKED);
		PauseEmulator();
		Dyna_Check_Codes = Dyna_Code_Check[method-1];
		CodeCheckMethod = method;
		ResumeEmulator(FALSE);	// Need to init emu
		CheckMenuItem(hMenu, codecheckmenulist[method-1], MF_CHECKED);
	}
	else
	{
		CheckMenuItem(hMenu, codecheckmenulist[CodeCheckMethod-1], MF_UNCHECKED);
		CodeCheckMethod = method;
		defaultoptions.Code_Check = method;
		CheckMenuItem(hMenu, codecheckmenulist[method-1], MF_CHECKED);
	}
}

void PrepareBeforePlay(void)
{
	critical_msg_buffer[0] = '\0';		//clear the critical message buffer

	// Hide romlist
	ListView_SetExtendedListViewStyle(hwndRomList, LVS_EX_FULLROWSELECT);
	NewRomList_ListViewHideHeader(hwndRomList);
	ShowWindow(hwndRomList, SW_HIDE);


	// Setting options
	GenerateCurrentRomOptions();
	ResetRdramSize(currentromoptions.RDRAM_Size);

	CheckMenuItem(hMenu, cfmenulist[CounterFactor-1], MF_UNCHECKED);
	CounterFactor = currentromoptions.Counter_Factor;
	CheckMenuItem(hMenu, cfmenulist[CounterFactor-1], MF_CHECKED);
	sprintf(generalmessage, "CF=%d", currentromoptions.Counter_Factor);
	SetStatusBarText(2, generalmessage);

	CheckMenuItem(hMenu, codecheckmenulist[CodeCheckMethod-1], MF_UNCHECKED);
	CodeCheckMethod = currentromoptions.Code_Check;
	CheckMenuItem(hMenu, codecheckmenulist[CodeCheckMethod-1], MF_CHECKED);

	// Using the Check_QWORD to boot, will switch to ROM specified CodeCheckMethod
	// at first FPU exception. I don't know why use NoCheck method will not boot
	// Game like SuperMario should not need to do DynaCodeCheck but how the ROM does not boot
	// with DynaCodeCheck, need debug
	if( CodeCheckMethod == CODE_CHECK_NONE || CodeCheckMethod == CODE_CHECK_DMA_ONLY)
	{
		Dyna_Check_Codes = Dyna_Code_Check_None_Boot;
		TRACE0("Set code check method = Dyna_Code_Check_None_Boot / Check_DMA_only");
	}
	else
		Dyna_Check_Codes = Dyna_Code_Check[CodeCheckMethod-1];

	whichcore = currentromoptions.Emulator;
	SendMessage(hwnd, WM_COMMAND, whichcore == DYNACOMPILER?ID_DYNAMICCOMPILER:ID_INTERPRETER, 0);
	SetStatusBarText(4, whichcore == DYNACOMPILER ? "D" : "I");

	auto_apply_cheat_code_enabled = FALSE;

	//About FPU usage exceptions
	if( currentromoptions.FPU_Hack == USEFPUHACK_YES ) 
	{
		CPU_instruction[0x11] = COP1_NotAvailable_instr;			//this is for interpreter
		dyna_instruction[0x11] = dyna4300i_cop1_with_exception;		//this is for dyna
	}
	else
	{
		CPU_instruction[0x11] = COP1_instr;							//this is for interpreter
		dyna_instruction[0x11] = dyna4300i_cop1;					//this is for dyna
	}

	Flashram_Init();
	Init_iPIF();

	DListCount=0;
	AListCount=0;
	PIDMACount=0;
	ControllerReadCount=0;
	TLBCount=0;

	if( !QueryPerformanceFrequency(&Freq) )
	{
		currentromoptions.Max_FPS = MAXFPS_NONE;	//ok, this computer does not support
													//accurate timer, don't use speed limiter
	}
	else
	{
		if( rominfo.TV_System == 0 )	//PAL
		{
			vips_speed_limits[MAXFPS_AUTO_SYNC] = vips_speed_limits[MAXFPS_PAL_50];
		}
		else	//NTSC
		{
			vips_speed_limits[MAXFPS_AUTO_SYNC] = vips_speed_limits[MAXFPS_NTSC_60];
		}
	}

	//TRACE1("Freq = %ld", Freq.QuadPart);
}

// Move the status bar to the bottom of the main window. 
void DockStatusBar(void)
{
	RECT rc, rcstatusbar;
	if( hStatusBar == NULL )
		return;
	
	GetClientRect(hwnd, &rc);
	GetWindowRect(hStatusBar, &rcstatusbar);
	MoveWindow(hStatusBar, 0, rc.bottom-(rcstatusbar.bottom-rcstatusbar.top+1),
				rcstatusbar.right-rcstatusbar.left+1, rcstatusbar.bottom-rcstatusbar.top+1, TRUE);
	ShowWindow(hStatusBar, SW_HIDE);
	ShowWindow(hStatusBar, SW_SHOW);

	InitStatusBarParts();
}

void InitStatusBarParts(void)
{
	RECT rc;
	if( hStatusBar == NULL )
		return;
	else
	{
		int sizes[6];

		GetWindowRect(hStatusBar, &rc);

		//sizes[5] = rc.right-rc.left-25;
		//sizes[4] = sizes[5]-40;
		sizes[4] = rc.right-rc.left-25;
		sizes[3] = sizes[4]-15;
		sizes[2] = sizes[3]-30;
		sizes[1] = sizes[2]-40;
		sizes[0] = sizes[1]-60;

		SendMessage(hStatusBar, SB_SETPARTS, 5, (LPARAM)sizes);
	}
}

void SetStatusBarText(int fieldno, char *text)
{
	SendMessage(hStatusBar, SB_SETTEXT, fieldno, (LPARAM)text);
}

void InitPluginData(void)
{
    Gfx_Info.hWnd                   = hwnd;
	Gfx_Info.hStatusBar             = hStatusBar;
    Gfx_Info.MemoryBswaped          = TRUE;
    Gfx_Info.HEADER                 = (__int8*)&HeaderDllPass[0];
    Gfx_Info.RDRAM                  = (__int8*)&gMemoryState.RDRAM[0];
    Gfx_Info.DMEM                   = (__int8*)&SP_DMEM;
    Gfx_Info.IMEM                   = (__int8*)&SP_IMEM;
    Gfx_Info.MI_INTR_RG             = &MI_INTR_REG_R;
    Gfx_Info.DPC_START_RG           = &DPC_START_REG;
    Gfx_Info.DPC_END_RG             = &DPC_END_REG;
    Gfx_Info.DPC_CURRENT_RG         = &DPC_CURRENT_REG;
    Gfx_Info.DPC_STATUS_RG          = &DPC_STATUS_REG;
    Gfx_Info.DPC_CLOCK_RG           = &DPC_CLOCK_REG;
    Gfx_Info.DPC_BUFBUSY_RG         = &DPC_BUFBUSY_REG;
    Gfx_Info.DPC_PIPEBUSY_RG        = &DPC_PIPEBUSY_REG;
    Gfx_Info.DPC_TMEM_RG            = &DPC_TMEM_REG;

    Gfx_Info.VI_STATUS_RG           = &VI_STATUS_REG;
    Gfx_Info.VI_ORIGIN_RG           = &VI_ORIGIN_REG;
    Gfx_Info.VI_WIDTH_RG            = &VI_WIDTH_REG;
    Gfx_Info.VI_INTR_RG             = &VI_INTR_REG;
    Gfx_Info.VI_V_CURRENT_LINE_RG   = &VI_CURRENT_REG;
    Gfx_Info.VI_TIMING_RG           = &VI_BURST_REG;
    Gfx_Info.VI_V_SYNC_RG           = &VI_V_SYNC_REG;
    Gfx_Info.VI_H_SYNC_RG           = &VI_H_SYNC_REG;
    Gfx_Info.VI_LEAP_RG             = &VI_LEAP_REG;
    Gfx_Info.VI_H_START_RG          = &VI_H_START_REG;
    Gfx_Info.VI_V_START_RG          = &VI_V_START_REG;
    Gfx_Info.VI_V_BURST_RG          = &VI_V_BURST_REG;
    Gfx_Info.VI_X_SCALE_RG          = &VI_X_SCALE_REG;
    Gfx_Info.VI_Y_SCALE_RG          = &VI_Y_SCALE_REG;
    Gfx_Info.CheckInterrupts        = CheckInterrupts;

	Audio_Info.hwnd = hwnd;
	Audio_Info.hinst = hinstLibAudio;

	Audio_Info.MemoryBswaped = 1;    // If this is set to TRUE, then the memory has been pre
	                       //   bswap on a dword (32 bits) boundry 
						   //	eg. the first 8 bytes are stored like this:
	                       //        4 3 2 1   8 7 6 5
    Audio_Info.HEADER                 = (__int8*)&HeaderDllPass[0];
    Audio_Info.__RDRAM                  = (__int8*)&gMemoryState.RDRAM[0];
    Audio_Info.__DMEM                   = (__int8*)&SP_DMEM;
	Audio_Info.__IMEM					= (__int8*)&SP_IMEM;

	Audio_Info.__MI_INTR_REG             = &MI_INTR_REG_R;

	Audio_Info.__AI_DRAM_ADDR_REG     = &AI_DRAM_ADDR_REG;;
	Audio_Info.__AI_LEN_REG           = &AI_LEN_REG;
	Audio_Info.__AI_CONTROL_REG       = &AI_CONTROL_REG;
	Audio_Info.__AI_STATUS_REG        = &AI_STATUS_REG;
	Audio_Info.__AI_DACRATE_REG       = &AI_DACRATE_REG;
	Audio_Info.__AI_BITRATE_REG       = &AI_BITRATE_REG;
	Audio_Info.CheckInterrupts		  = CheckInterrupts;
}

void Set_1964_Directory(void)
{
	char path_buffer[_MAX_PATH], drive[_MAX_DRIVE] ,dir[_MAX_DIR];
    char fname[_MAX_FNAME],ext[_MAX_EXT];

 	GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));
	_splitpath( path_buffer, drive, dir, fname, ext );

	// Set the main 1964.exe directory
	strcpy(main_directory,drive);
	strcat(main_directory,dir);

}

HMENU file_submenu;
HMENU CPU_submenu;
HMENU recent_rom_directory_submenu;
HMENU recent_game_submenu;
HMENU switch_compiler_submenu;
MENUITEMINFO switch_compiler_submenu_info;
HMENU dyna_code_checking_submenu;
HMENU counter_hack_submenu;
HMENU state_save_submenu;
HMENU state_load_submenu;
UINT recent_rom_directory_submenu_pos;
UINT recent_game_submenu_pos;
UINT switch_compiler_submenu_pos;
UINT dyna_code_checking_submenu_pos;
UINT counter_hack_submenu_pos;
UINT state_save_submenu_pos;
UINT state_load_submenu_pos;

MENUITEMINFO advanced_options_menuitem;
MENUITEMINFO seperator_menuitem;
UINT advanced_options_menuitem_pos;
UINT seperator_menuitem_pos;

UINT recent_game_menu_ids[8] = 
{
	ID_FILE_RECENTGAMES_GAME1,
	ID_FILE_RECENTGAMES_GAME2,
	ID_FILE_RECENTGAMES_GAME3,
	ID_FILE_RECENTGAMES_GAME4,
	ID_FILE_RECENTGAMES_GAME5,
	ID_FILE_RECENTGAMES_GAME6,
	ID_FILE_RECENTGAMES_GAME7,
	ID_FILE_RECENTGAMES_GAME8
};
UINT recent_rom_directory_menu_ids[4] = 
{
	ID_FILE_ROMDIRECTORY1,
	ID_FILE_ROMDIRECTORY2,
	ID_FILE_ROMDIRECTORY3,
	ID_FILE_ROMDIRECTORY4
};

void ModifyMenuText(UINT menuid, char* newtext)
{
	ModifyMenu(hMenu, menuid, MF_BYCOMMAND, menuid, newtext);
}

void RegerateRecentGameMenus(void)
{
	if( recent_rom_directory_list )
		InsertMenu(file_submenu, recent_game_submenu_pos, MF_BYPOSITION|MF_POPUP, (UINT)recent_game_submenu, "Recent Games");
	else
		InsertMenu(file_submenu, recent_rom_directory_submenu_pos, MF_BYPOSITION|MF_POPUP, (UINT)recent_game_submenu, "Recent Games");
}

void DeleteRecentGameMenus(void)
{
	int k,i,j,n;
	char str[100];
	i = GetMenuItemCount(hMenu);
	for( k=0; k<i; k++)
	{
		GetMenuString(hMenu,k,str,80,MF_BYPOSITION);
		if( strcmp(str, "&File") == 0 )
		{
			file_submenu = GetSubMenu(hMenu, k);
			j = GetMenuItemCount(file_submenu);
			for( n=j-1; n>=0; n--)			// I have to delete the menu in reverse order
			{
				GetMenuString(file_submenu,n,str,80,MF_BYPOSITION);
				if( strcmp(str,"Recent Games" ) == 0 )
				{
					recent_game_submenu = GetSubMenu(file_submenu, n);
					recent_game_submenu_pos = n;
					RemoveMenu(file_submenu,n,MF_BYPOSITION);
				}
			}
		}
	}

}

//char recent_game_lists[8][260];
void RefreshRecentGameMenus(char* newgamefilename)
{
	int i;
	for( i=0; i<8; i++ )
	{
		if( strcmp(recent_game_lists[i], newgamefilename) == 0 )
			break;
	}

	if( i != 0 )
	{
		if( i==8 ) i=7;	//if not found

		// need to move the most recent file to the 1st position
		for( ; i>0; i-- )
		{
			strcpy(recent_game_lists[i], recent_game_lists[i-1]);
			ModifyMenuText(recent_game_menu_ids[i], recent_game_lists[i]);
		}
		strcpy(recent_game_lists[0], newgamefilename);
		ModifyMenuText(recent_game_menu_ids[0], newgamefilename);
	}
	return;
}

//char recent_rom_directory_lists[4][260];
void RefreshRecentRomDirectoryMenus(char* newromdirectory)
{
	int i;
	for( i=0; i<4; i++ )
	{
		if( strcmp(recent_rom_directory_lists[i], newromdirectory) == 0 )
			break;
	}

	if( i != 0 )
	{
		if( i==4 ) i=3;	//if not found

		// need to move the most recent file to the 1st position
		for( ; i>0; i-- )
		{
			strcpy(recent_rom_directory_lists[i], recent_rom_directory_lists[i-1]);
			ModifyMenuText(recent_rom_directory_menu_ids[i], recent_rom_directory_lists[i]);
		}
		strcpy(recent_rom_directory_lists[0], newromdirectory);
		ModifyMenuText(recent_rom_directory_menu_ids[0], newromdirectory);
	}
	return;
}

void RegerateRecentRomDirectoryMenus(void)
{
	InsertMenu(file_submenu, recent_rom_directory_submenu_pos, MF_BYPOSITION|MF_POPUP, (UINT)recent_rom_directory_submenu, "Recent ROM Directories");
}

void DeleteRecentRomDirectoryMenus(void)
{
	int k,i,j,n;
	char str[100];
	i = GetMenuItemCount(hMenu);
	for( k=0; k<i; k++)
	{
		GetMenuString(hMenu,k,str,80,MF_BYPOSITION);
		if( strcmp(str, "&File") == 0 )
		{
			file_submenu = GetSubMenu(hMenu, k);
			j = GetMenuItemCount(file_submenu);
			for( n=j-1; n>=0; n--)			// I have to delete the menu in reverse order
			{
				GetMenuString(file_submenu,n,str,80,MF_BYPOSITION);
				if( strcmp(str,"Recent ROM Directories" ) == 0 )
				{
					recent_rom_directory_submenu = GetSubMenu(file_submenu, n);
					recent_rom_directory_submenu_pos = n;
					RemoveMenu(file_submenu,n,MF_BYPOSITION);
				}
			}
		}
	}

}

void RegenerateStateSelectorMenus(void)
{
	if( expert_user_mode )
	{
		InsertMenu(CPU_submenu, state_save_submenu_pos, MF_BYPOSITION|MF_POPUP, (UINT)state_save_submenu, "Save State\tF5");
		InsertMenu(CPU_submenu, state_save_submenu_pos, MF_BYPOSITION|MF_POPUP, (UINT)state_load_submenu, "Load State\tF7");
	}
	else
	{
		AppendMenu(CPU_submenu, MF_POPUP, (UINT)state_save_submenu, "Save State\tF5");
		AppendMenu(CPU_submenu, MF_POPUP, (UINT)state_load_submenu, "Load State\tF7");
	}
}

void DeleteStateSelectorMenus(void)
{

	int k,i,j,n;
	char str[100];
	i = GetMenuItemCount(hMenu);
	for( k=0; k<i; k++)
	{
		GetMenuString(hMenu,k,str,80,MF_BYPOSITION);
		if( strcmp(str,"CPU" ) == 0 )
		{
			CPU_submenu = GetSubMenu(hMenu, k);
			j = GetMenuItemCount(CPU_submenu);
			for( n=j-1; n>=0; n--)			// I have to delete the menu in reverse order
			{
				GetMenuString(CPU_submenu,n,str,80,MF_BYPOSITION);
				// Delete all cpu core switch menus
				if( strcmp(str, "Load State\tF7") == 0 )
				{
					state_load_submenu = GetSubMenu(CPU_submenu, n);
					state_load_submenu_pos = n;
					RemoveMenu(CPU_submenu,n,MF_BYPOSITION);
				}
				else if( strcmp(str, "Save State\tF5") == 0 )
				{
					state_save_submenu = GetSubMenu(CPU_submenu, n);
					state_save_submenu_pos = n;
					RemoveMenu(CPU_submenu,n,MF_BYPOSITION);
				}
			}
		}
	}
}

void RegenerateAdvancedUserMenus(void)
{
	AppendMenu(CPU_submenu, MF_SEPARATOR, (UINT)NULL, NULL);
	AppendMenu(CPU_submenu, MF_POPUP, (UINT)switch_compiler_submenu, "Switch Compiler");
	AppendMenu(CPU_submenu, MF_POPUP, (UINT)dyna_code_checking_submenu, "Dyna Code Checking");
	AppendMenu(CPU_submenu, MF_POPUP, (UINT)counter_hack_submenu, "Counter Hack");
#ifdef DEBUG_COMMON
	InsertMenu(file_submenu, ID_PERFERENCE_OPTIONS, MF_UNCHECKED, ID_DEFAULTOPTIONS, "Advanced Options ...");
#endif
}

void DeleteAdvancedUserMenus(void)
{

	int k,i,j,n;
	char str[100];
	i = GetMenuItemCount(hMenu);
	for( k=0; k<i; k++)
	{
		GetMenuString(hMenu,k,str,80,MF_BYPOSITION);
		if( strcmp(str,"CPU" ) == 0 )
		{
			CPU_submenu = GetSubMenu(hMenu, k);
			j = GetMenuItemCount(CPU_submenu);
			for( n=j-1; n>=0; n--)			// I have to delete the menu in reverse order
			{
				GetMenuString(CPU_submenu,n,str,80,MF_BYPOSITION);
				// Delete all cpu core switch menus
				if( strcmp(str,"Switch Compiler" ) == 0 )
				{
					switch_compiler_submenu = GetSubMenu(CPU_submenu, n);
					GetMenuItemInfo(CPU_submenu, n, MF_BYPOSITION, &switch_compiler_submenu_info);
					switch_compiler_submenu_pos = n;
					RemoveMenu(CPU_submenu,n,MF_BYPOSITION);
#ifndef ENABLE_OPCODE_DEBUGGER
					GetMenuItemInfo(CPU_submenu,n-1,MF_BYPOSITION, &seperator_menuitem);
					seperator_menuitem_pos = n-1;
					RemoveMenu(CPU_submenu,n-1,MF_BYPOSITION);		// Delete the seperator
#endif
				}
				// Delete all code check method switch menus
				else if( strcmp(str,"Dyna Code Checking" ) == 0 )
				{
					dyna_code_checking_submenu = GetSubMenu(CPU_submenu, n);
					dyna_code_checking_submenu_pos = n;
					RemoveMenu(CPU_submenu,n,MF_BYPOSITION);
				}
				// Delete all Counter Hack menus
				else if( strcmp(str,"Counter Hack" ) == 0 )
				{
					counter_hack_submenu = GetSubMenu(CPU_submenu, n);
					counter_hack_submenu_pos = n;
					RemoveMenu(CPU_submenu,n,MF_BYPOSITION);
				}
			}
		}
	}
	GetMenuItemInfo(hMenu,ID_DEFAULTOPTIONS,MF_BYCOMMAND, &advanced_options_menuitem);
	RemoveMenu(hMenu,ID_DEFAULTOPTIONS,MF_BYCOMMAND);
}

void SetupAdvancedMenus()
{
	int i;
	for( i=0; i<4; i++ )
	{
		ModifyMenuText(recent_rom_directory_menu_ids[i], recent_rom_directory_lists[i]);
	}

	for( i=0; i<8; i++ )
	{
		ModifyMenuText(recent_game_menu_ids[i], recent_game_lists[i]);
	}

	if( expert_user_mode == FALSE )
		DeleteAdvancedUserMenus();
#ifndef DEBUG_COMMON
	else
	{	//hide the default option menu from RELEASE MODE
		RemoveMenu(hMenu,ID_DEFAULTOPTIONS,MF_BYCOMMAND);
	}
#endif

	if( recent_rom_directory_list == FALSE )
		DeleteRecentRomDirectoryMenus();

	if( recent_game_list == FALSE )
		DeleteRecentGameMenus();

	if( state_selector_menu == FALSE )
		DeleteStateSelectorMenus();

	//RegenerateAdvancedUserMenus();
}

void CountryCodeToCountryName_and_TVSystem(int countrycode, char *countryname, int * tvsystem)
{
	switch(countrycode)
	{
	// Demo
	case 0:
		*tvsystem = TV_SYSTEM_NTSC;
		strcpy(countryname, "Demo");
		break;

	case '7':
		*tvsystem = TV_SYSTEM_NTSC;
		strcpy(countryname, "Beta");
		break;

	case 'A':
		*tvsystem = TV_SYSTEM_NTSC;
		strcpy(countryname, "NTSC");
		break;

	// Germany
	case 0x44:
		*tvsystem = TV_SYSTEM_PAL;
		strcpy(countryname,"GER");
		break;

	// USA
	case 0x45:
		*tvsystem = TV_SYSTEM_NTSC;
		strcpy(countryname,"USA");
		break;

	// France
	case 0x46:
		*tvsystem = TV_SYSTEM_PAL;
		strcpy(countryname,"FRA");
		break;

	// Italy
	case 'I':
		*tvsystem = TV_SYSTEM_PAL;
		strcpy(countryname, "ITA");
		break;
	// Japan 
	case 0x4A:
		*tvsystem = TV_SYSTEM_NTSC;
		strcpy(countryname,"JAP");
		break;

	// Europe - PAL
	case 0x50:
		*tvsystem = TV_SYSTEM_PAL;
		strcpy(countryname,"EUR");
		break;

	case 'S':	//Spain
		*tvsystem = TV_SYSTEM_PAL;
		strcpy(countryname, "SPA");
		break;

	// Australia 
	case 0x55:
		*tvsystem = TV_SYSTEM_PAL;
		strcpy(countryname,"AUS");
		break;

	case 0x58:
		*tvsystem = TV_SYSTEM_PAL;
		strcpy(countryname,"PAL");
		break;

	// Australia 
	case 0x59:
		*tvsystem = TV_SYSTEM_PAL;
		strcpy(countryname,"AUS");
		break;

	// ???
	default:
		*tvsystem = TV_SYSTEM_PAL;
		sprintf(countryname,"PAL", countrycode);
		break;
	}
}

void CaptureScreenToFile(void)
{
	if( Emu_Is_Running )
	{
		if( GfxPluginVersion != 0x0103 )
			DisplayError("Current video plugin does not support screen capture");
		else
		{
			char directory[_MAX_PATH];
			strcpy(directory, main_directory);
			strcat(directory, "Screens\\");
			VIDEO_CaptureScreen(directory);
		}
	}
}



