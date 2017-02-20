/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Emulator for Nintendo 64 console system                              |
 |  Copyright (C) 2001  Joel Middendorf  schibo@emuhq.com                       |
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
by my friend anarko and RSP info has been provided by zilmar. Most source
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
#include "DLL_Video.h"
#include "DLL_Audio.h"
#include "DLL_Input.h"
#include "registry.h"
#include "..\r4300i.h"
#include "..\controller.h"
#include "..\timer.h"


#ifdef WINDEBUG_1964
#include "windebug.h"
#endif

int ActiveApp;
char szBaseWindowTitle[] = "1964 0.6.1";
HACCEL hAccelTable = (HACCEL)"WINGUI_ACC";
int AiUpdating;
int timer;

#ifdef DEBUG_COMMON
void ToggleDebugOptions(WPARAM wParam);
#endif

void SelectVISpeed(WPARAM wParam);

//---------------------------------------------------------------------------------------

void LoadPlugins()
{
    char AudioPath[_MAX_PATH];	// _MAX_PATH = 260
    char VideoPath[_MAX_PATH];
    char InputPath[_MAX_PATH];
    char StartPath[_MAX_PATH];
    char NullSndPath[_MAX_PATH];
    // int Crapped = 0;
    int Audio = 0;

    GetPluginDir(StartPath);
    
    strcpy(VideoPath, StartPath);

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
        exit(1);
    }

	// Call the VIDEO_InitiateGFX function in the Video DLL to initiate GFX as specified
	// by Gfx_info which has been initialized by WinMain()
    (VIDEO_InitiateGFX)(Gfx_Info);


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
        exit(1);
    }

	// Call the CONTROLLER_InitiateControllers function in the input DLL to initiate the controllers
    CONTROLLER_InitiateControllers(hwnd, Controls);  


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

    if (AUDIO_Initialize != NULL)
	{
        AiUpdating = 1;
        if ((AUDIO_Initialize)(Audio_Info) == TRUE ) 
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

		// Try to load the NullSnd audio plugin if failed to load the regular one
		if (LoadAudioPlugin(NullSndPath) != TRUE)
		{
			//Crapped = 1;
			DisplayError("Can not load %s.", NullSndPath);
			exit(1);
		}
    }
}

//---------------------------------------------------------------------------------------
void CALLBACK TimerProc( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime )
{
	if( Rom_Loaded )
	{
		sprintf(generalmessage, "%s - %d fps", szWindowTitle, framecounter);
		framecounter=0;
		SetWindowText(hwnd, generalmessage);
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
    Rom_Loaded            = FALSE;
	whichcore = 2;

#ifdef WINDEBUG_1964
    RUN_TO_ADDRESS_ACTIVE = FALSE;
    OpCount = 0;
    NextClearCode = 250;
    BreakAddress = -1;
#endif

    hInst = hInstance;
    LoadString(hInstance, IDS_MAINDISCLAIMER, MainDisclaimer, sizeof(MainDisclaimer));

    if (hPrevInstance)
        return FALSE;

    hwnd = InitWin98UI(hInstance, nCmdShow);
        if (hwnd == NULL)
            DisplayError("Could not get a windows handle.");
        

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

	InitVirtualMemory();

    Gfx_Info.hWnd                   = hwnd;
    Gfx_Info.hStatusBar             = NULL;
    Gfx_Info.MemoryBswaped          = TRUE;
    Gfx_Info.HEADER                 = (__int8*)&HeaderDllPass[0];
    Gfx_Info.RDRAM                  = (__int8*)&RDRAM[0];
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
    Audio_Info.__RDRAM                  = (__int8*)&RDRAM[0];
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


    r4300i_Init();
    ReadConfiguration();                //System registry settings

    LoadPlugins();

	timer = SetTimer(hwnd,1,1000,TimerProc);

#ifdef WINDEBUG_1964
    DebuggerActive = FALSE;
    OpenDebugger();
#endif

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
        //if (AiUpdating)
            //AUDIO_AiUpdate(FALSE);
        //else
            //AUDIO_AiUpdate(FALSE);
 goto _HOPPITY;
}

//---------------------------------------------------------------------------------------

void __cdecl DisplayError (char * Message, ...)
{
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

void UpdateCIC()
{
//** Math CIC
	__int64 CIC_CRC = 0;
    int i;

	for (i=0; i <0xFC0; i++)
	{
		CIC_CRC = CIC_CRC + (uint8)ROM_Image[0x40+i];
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
		break;
	}

	rominfo.countrycode = HeaderDllPass[0x3D];

//** TV_System		
	switch (rominfo.countrycode)
	{
	// Germany - PAL
	case 0x44:
		rominfo.TV_System = 0;
		break;

	// USA - NTSC
	case 0x45:
		rominfo.TV_System = 1;
		break;

	// Japan 
	case 0x4A:
		rominfo.TV_System = 1;
		break;

	// Europe - PAL
	case 0x50:
		rominfo.TV_System = 0;
		break;

	// Australia 
	case 0x55:
		rominfo.TV_System = 0;
		break;

	// Australia 
	case 0x59:
		rominfo.TV_System = 0;
		break;

	// ???
	case 0x58:
		rominfo.TV_System = 0;
		break;

	default: 
//		ErrorToFile("Cant find TV version %x - (default PAL)", rominfo.countrycode);
//		DisplayError("Cant find TV version %x - (default PAL)", rominfo.countrycode);
		rominfo.TV_System = 0;
		break;
	}
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
    ofn.lpstrFilter         = "N64 ROMs (*.ZIP, *.V64, *.BIN, *.ROM)\0*.ZIP;*.V64;*.BIN;*.ROM\0All Files (*.*)\0*.*\0";
    ofn.lpstrCustomFilter   = NULL;
    ofn.nMaxCustFilter      = 0;
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = szFileName;
    ofn.nMaxFile            = MAXFILENAME;
    ofn.lpstrInitialDir     = gRegSettings.ROMPath;
    ofn.lpstrFileTitle      = szFileTitle;
    ofn.nMaxFileTitle       = MAXFILENAME;
    ofn.lpstrTitle          = "Open Image";
    ofn.lpstrDefExt         = "TXT";
    ofn.Flags               = OFN_HIDEREADONLY;

    if (!GetOpenFileName ((LPOPENFILENAME)&ofn))
	{
		return FALSE;
	}

    lstrcpy(szWindowTitle, szBaseWindowTitle);
    lstrcat(szWindowTitle, " - ");

    ReadRomData(szFileName);
    lstrcat(szWindowTitle, rominfo.name);

	memcpy(&HeaderDllPass[0], &ROM_Image[0], 0x40);
    EnableMenuItem(hMenu, ID_OPENROM, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_PLUGINS, MF_GRAYED);
	EnableMenuItem(hMenu, ID_CLOSEROM, MF_ENABLED);
    SetWindowText(hwnd, szWindowTitle);

    gHardwareState.pc = 0xA4000040;

    Rom_Loaded = TRUE;

#ifdef WINDEBUG_1964
    MessageBox(hwnd, "Rom Loaded. Click the Rom->Play menu to begin\n", "OK!",MB_OK);
#endif

    _getcwd( gRegSettings.ROMPath, PATH_LEN );
    WriteConfiguration();

    UpdateCIC();

    return TRUE;    
}

//---------------------------------------------------------------------------------------

HWND InitWin98UI(HANDLE hInstance, int nCmdShow)
{
    WNDCLASS    wc;

    RECT winrect = {0,0,640,480};
    int width, height;

    AdjustWindowRect(   &winrect, WS_OVERLAPPEDWINDOW /*WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME */, TRUE);

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
    hMenu = GetMenu (hwnd);

#ifndef WINDEBUG_1964
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
#endif

    return hwnd;
}

//---------------------------------------------------------------------------------------

void GetPluginDir(char* Directory)
{
    char path_buffer[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR];
    char filename[_MAX_FNAME], ext[_MAX_EXT];

    GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));	// Get 1964.exe path and filename
    _splitpath(path_buffer,drive,dir,filename,ext);

    strcpy(Directory,drive);
    strcat(Directory,dir);
    strcat(Directory,"Plugin\\");
}

//---------------------------------------------------------------------------------------

void GetSaveDir(char* Directory)
{
    char path_buffer[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR];
    char fname[_MAX_FNAME],ext[_MAX_EXT];

    GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));
    _splitpath(path_buffer,drive,dir,fname,ext);

    strcpy(Directory,drive);
    strcat(Directory,dir);
    strcat(Directory,"Save\\");
}

//---------------------------------------------------------------------------------------

void   (__cdecl* GetDllInfo )( PLUGIN_INFO *) = NULL; 
void   (__cdecl* DLL_About)(HWND) = NULL; 
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
      } 
    } 
    return (TRUE); 
      
  case WM_COMMAND: 
    { 
      switch (wParam) 
      { 
      case IDOK: 
        { 
          WriteConfiguration(); 
          EndDialog(hDlg, TRUE); 
          FreePlugins();
          LoadPlugins(); 
          CONTROLLER_InitiateControllers(hwnd, Controls);       //Input DLL Initialization
          VIDEO_InitiateGFX(Gfx_Info);        //GFX DLL Initialization

          return (TRUE); 
        } 
      case IDCANCEL: 
        { 
          EndDialog(hDlg, TRUE); 
          return (TRUE); 
        } 
      case  IDC_DI_CONFIG: CONTROLLER_DllConfig(hDlg);       break; 
      case  IDC_DI_ABOUT : CONTROLLER_DllAbout(hDlg);           break; 
      case  IDC_DI_TEST  : CONTROLLER_DllTest(hDlg);            break; 
        
      case IDC_VID_CONFIG: VIDEO_DllConfig(hDlg);       break; 
      case IDC_VID_ABOUT : VIDEO_About(hDlg);           break; 
      case IDC_VID_TEST  : VIDEO_Test(hDlg);            break; 
        
      case IDC_AUD_CONFIG: AUDIO_DllConfig(hDlg);       break; 
      case IDC_AUD_ABOUT : AUDIO_About(hDlg);           break; 
      case IDC_AUD_TEST  : AUDIO_Test(hDlg);            break; 
        
      } 
      
      case CBN_SELCHANGE: 
        // Video 
        FreeLibrary(hinstLib); 
        ComboItemNum = SendDlgItemMessage(hDlg, IDC_COMBO_VIDEO, CB_GETCURSEL, 0, 0); 
        FindFirst = FindFirstFile(SearchPath, &libaa); 
        while (bDONE == 0) 
        { 
          strcpy(PluginName, StartPath); 
          strcat(PluginName, libaa.cFileName); 
          hinstLib = LoadLibrary(PluginName); 
          GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *))       GetProcAddress(hinstLib, "GetDllInfo"); 
          GetDllInfo(&Plugin_Info); 
          switch (Plugin_Info.Type) 
          { 
			case PLUGIN_TYPE_GFX: 
				_VIDEO_DllConfig     = (void (__cdecl*)(HWND))       GetProcAddress(hinstLib, "DllConfig"); 
				_VIDEO_Test          = (void (__cdecl*)(HWND))       GetProcAddress(hinstLib, "DllTest"); 
				_VIDEO_About         = (void (__cdecl*)(HWND))       GetProcAddress(hinstLib, "DllAbout"); 

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
        
        // Audio 
        FreeLibrary(hinstLib); 
        ComboItemNum = SendDlgItemMessage(hDlg, IDC_COMBO_AUDIO, CB_GETCURSEL, 0, 0); 
        FindFirst = FindFirstFile(SearchPath, &libaa); 
        while (bDONE == 0) 
        { 
          strcpy(PluginName, StartPath); 
          strcat(PluginName, libaa.cFileName); 
          hinstLib = LoadLibrary(PluginName); 
          GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *))       GetProcAddress(hinstLib, "GetDllInfo"); 
          GetDllInfo(&Plugin_Info); 
          switch (Plugin_Info.Type) 
          { 
          case PLUGIN_TYPE_AUDIO            : 
            _AUDIO_DllConfig     = (void (__cdecl*)(HWND))       GetProcAddress(hinstLib, "DllConfig"); 
            _AUDIO_Test          = (void (__cdecl*)(HWND))       GetProcAddress(hinstLib, "DllTest"); 
            _AUDIO_About         = (void (__cdecl*)(HWND))       GetProcAddress(hinstLib, "DllAbout"); 

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
        
        // Input 
        FreeLibrary(hinstLib); 
        ComboItemNum = SendDlgItemMessage(hDlg, IDC_COMBO_INPUT, CB_GETCURSEL, 0, 0); 
        FindFirst = FindFirstFile(SearchPath, &libaa); 
        while (bDONE == 0) 
        { 
          strcpy(PluginName, StartPath); 
          strcat(PluginName, libaa.cFileName); 
          hinstLib = LoadLibrary(PluginName); 
          GetDllInfo = (void   (__cdecl*)(PLUGIN_INFO *))       GetProcAddress(hinstLib, "GetDllInfo"); 
          GetDllInfo(&Plugin_Info); 
          switch (Plugin_Info.Type) 
          { 
          case PLUGIN_TYPE_CONTROLLER : 
            _CONTROLLER_DllTest          = (void (__cdecl*)(HWND))       GetProcAddress(hinstLib, "DllTest"); 
            _CONTROLLER_DllAbout         = (void (__cdecl*)(HWND))       GetProcAddress(hinstLib, "DllAbout"); 
            _CONTROLLER_DllConfig     = (void (__cdecl*)(HWND))       GetProcAddress(hinstLib, "DllConfig"); 
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
    } 
  return (FALSE); 
}
//---------------------------------------------------------------------------------------


long FAR PASCAL MainWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    static PAINTSTRUCT ps;
    static int ok = 0;
    static int IsFullScreen = 0;
    RECT ClientRect;

    switch (message) 
    {
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

		case WM_KEYDOWN:
			CONTROLLER_WM_KeyDown(wParam, lParam);
			break;

		case WM_KEYUP:
	        switch (wParam)
			{
				case VK_ESCAPE: 
//					if (IsFullScreen)
					{
						IsFullScreen ^= 1;	// Add by 1964m.53
						VIDEO_ChangeWindow(IsFullScreen);
					}
                
//					if (Rom_Loaded)
//						CloseROM();
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
				default:
					CONTROLLER_WM_KeyUp(wParam, lParam); 
					break;
			}
			break;

		case WM_SYSKEYUP:
			switch (wParam)
			{
				case VK_RETURN:
				IsFullScreen ^= 1;
				VIDEO_ChangeWindow(IsFullScreen);
				break;
			}
			break;

        case WM_COMMAND:
            switch (wParam) 
            {
                case ID_ROM_STOP   :    Stop();        break;
                case ID_ROM_START   :   Play();         break;
                case ID_ROM_PAUSE   :   Pause();        break;
#ifdef WINDEBUG_1964
                case ID_RUNTO       :   Get_HexInput(); break;
#endif
                case ID_OPENROM		:	OpenROM();		break;
				case ID_CLOSEROM	:	CloseROM();		break;
                case ID_ABOUT:
                    DialogBox(hInst, "ABOUTBOX", hWnd, (DLGPROC)About);
                    break;
        
                case ID_DI_CONFIG: CONTROLLER_DllConfig(hWnd);     break; 
                case ID_VIDEO_CONFIG: VIDEO_DllConfig(hWnd);       break; 
                case ID_AUD_CONFIG: AUDIO_DllConfig(hWnd);         break; 
				case ID_INTERPRETER:
					CheckMenuItem(hMenu, ID_INTERPRETER, MF_CHECKED);
					CheckMenuItem(hMenu, ID_STATICCOMPILER, MF_UNCHECKED);
					CheckMenuItem(hMenu, ID_DYNAMICCOMPILER, MF_UNCHECKED);
					whichcore=0;	
					break;
				case ID_STATICCOMPILER:	
					CheckMenuItem(hMenu, ID_INTERPRETER, MF_UNCHECKED);
					CheckMenuItem(hMenu, ID_STATICCOMPILER, MF_CHECKED);
					CheckMenuItem(hMenu, ID_DYNAMICCOMPILER, MF_UNCHECKED);
					whichcore=1;	
					break;
				case ID_DYNAMICCOMPILER:
					CheckMenuItem(hMenu, ID_INTERPRETER, MF_UNCHECKED);
					CheckMenuItem(hMenu, ID_STATICCOMPILER, MF_UNCHECKED);
					CheckMenuItem(hMenu, ID_DYNAMICCOMPILER, MF_CHECKED);
					whichcore=2;	
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

                case IDM_320_240    : VIDEO_ExtraChangeResolution(hWnd, 320, NULL); break;
                case IDM_640_480    : VIDEO_ExtraChangeResolution(hWnd, 640, NULL); break;
                case IDM_800_600    : VIDEO_ExtraChangeResolution(hWnd, 800, NULL); break;
                case IDM_1024_768   : VIDEO_ExtraChangeResolution(hWnd, 1024, NULL);break;
                case IDM_1280_1024  : VIDEO_ExtraChangeResolution(hWnd, 1280, NULL);break;
                case IDM_1600_1200  : VIDEO_ExtraChangeResolution(hWnd, 1600, NULL);break;
                case IDM_1920_1440  : VIDEO_ExtraChangeResolution(hWnd, 1920, NULL);break;
                case IDM_FULLSCREEN : VIDEO_ChangeWindow(1);break;
                case IDM_PLUGINS    : 
					DialogBox(hInst, "PLUGINS", hWnd, (DLGPROC)PluginsDialog);            
					break;
                case ID_CHECKWEB    : 
					ShellExecute( hwnd, "open", "http://www.emuhq.com/1964", NULL, NULL, SW_MAXIMIZE); 
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
                    OpenDebugger();
                    break;
                case ID_DEBUGGER_CLOSE:
                    CloseDebugger();
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
					ToggleDebugOptions(wParam);
					break;
#endif
				case ID_DISABLESOUND:
					AiUpdating = 1-AiUpdating;
					if( AiUpdating )
						CheckMenuItem(hMenu, ID_DISABLESOUND, MF_UNCHECKED);
					else
					{
						CheckMenuItem(hMenu, ID_DISABLESOUND, MF_CHECKED);
						AUDIO_AiUpdate(TRUE);
					}
					break;

                case ID_EXIT:
					SuspendThread(CPUThreadHandle);
					CloseHandle(CPUThreadHandle);
					Close_iPIF();	//save mempak and eeprom
					FreeVirtualMemory();
					KillTimer(hWnd, timer);
					//FreePlugins();
					PostQuitMessage(0);
					break;
            }
            break;

        case WM_CLOSE:
            SuspendThread(CPUThreadHandle);
            CloseHandle(CPUThreadHandle);
            Close_iPIF();	//save mempak and eeprom
            Rom_Loaded = FALSE;
			FreeVirtualMemory();
			KillTimer(hWnd,timer);
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
	EnableMenuItem(hMenu, ID_CLOSEROM, MF_ENABLED);
	AUDIO_AiUpdate(TRUE);
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
        
    if (Rom_Loaded)
    {
        GetExitCodeThread(CPUThreadHandle, &ExitCode);
        if (ExitCode != STILL_ACTIVE)
        {
            r4300i_Reset();
            RunEmulator(whichcore);
        }
        else
		{
            ResumeThread(CPUThreadHandle);
		}
	    EnableMenuItem(hMenu, ID_OPENROM, MF_GRAYED);
//		EnableMenuItem(hMenu, ID_CLOSEROM, MF_GRAYED);
		EnableMenuItem(hMenu, IDM_PLUGINS, MF_GRAYED);
		AUDIO_Initialize(Audio_Info);
		AUDIO_AiUpdate(TRUE);
    }
	else
        DisplayError("Please load a ROM first.");
}

void CloseROM()
{
	if( Rom_Loaded )
	{
#ifdef WINDEBUG_1964
//		CloseDebugger();
#endif

        SuspendThread(CPUThreadHandle);
        CloseHandle(CPUThreadHandle);
		TerminateThread(CPUThreadHandle, 1);
				
		FreeVirtualRomMemory();
		r4300i_Init();
		//sp_reset();
		//CPUdelayPC = 0;
		//CPUdelay = 0;
		//gHardwareState.pc = 0xA4000040;

		//ClearCPUTasks();

		/*
		sp_task_counter = 0;

#ifdef DODMASEGMENT
		DMAInProgress = NO_DMA_IN_PROGRESS;
#endif
		*/
		
		InitTLB();
		Rom_Loaded = FALSE;
		VIDEO_RomClosed();
		AUDIO_AiUpdate(TRUE);
		AUDIO_RomClosed();
		Close_iPIF();

		EnableMenuItem(hMenu, ID_OPENROM, MF_ENABLED);
//		EnableMenuItem(hMenu, ID_CLOSEROM, MF_GRAYED);
		EnableMenuItem(hMenu, IDM_PLUGINS, MF_ENABLED);
	}
	else
		DisplayError("Please load a ROM first.");
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

	if( Rom_Loaded )
		CloseROM();

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

void Stop()
{
    if (Rom_Loaded)
    {
        SuspendThread(CPUThreadHandle);
        TerminateThread(CPUThreadHandle, 1);
        CloseHandle(CPUThreadHandle);
        r4300i_Reset();
		//sp_reset() need to be implemented
		//sp_reset();
		CPUdelayPC = 0;
		CPUdelay = 0;
		EnableMenuItem(hMenu, ID_OPENROM, MF_ENABLED);
		EnableMenuItem(hMenu, IDM_PLUGINS, MF_ENABLED);
		EnableMenuItem(hMenu, ID_CLOSEROM, MF_ENABLED);
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
	}
}
#endif

