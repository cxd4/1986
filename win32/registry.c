#include <windows.h>
#include "../globals.h"
#include "resource.h"
#include "registry.h"

#define MAIN_1964_KEY       "Software\\1964emu\\GUI"
#define KEY_WINDOW_X        "WindowXPos"
#define KEY_WINDOW_Y        "WindowYPos"
#define KEY_MAXIMIZED       "Maximized"
#define KEY_CLIENT_WIDTH    "ClientWidth"
#define KEY_ROM_PATH        "ROMPath"
#define KEY_THREAD_PRIORITY "ThreadPriority"
#define KEY_AUDIO_PLUGIN    "AudioPlugin"
#define KEY_INPUT_PLUGIN    "InputPlugin"
#define KEY_VIDEO_PLUGIN    "VideoPlugin"

extern void __cdecl DisplayError (char * Message, ...);
char* ReadRegistryStrVal(char* MainKey, char* Field);

//---------------------------------------------------------------------------------------

void ReadConfiguration()
{
    strcpy(gRegSettings.ROMPath,        ReadRegistryStrVal(MAIN_1964_KEY, "ROMPath")    );
    strcpy(gRegSettings.AudioPlugin,    ReadRegistryStrVal(MAIN_1964_KEY, "AudioPlugin"));
    strcpy(gRegSettings.VideoPlugin,    ReadRegistryStrVal(MAIN_1964_KEY, "VideoPlugin"));
    strcpy(gRegSettings.InputPlugin,    ReadRegistryStrVal(MAIN_1964_KEY, "InputPlugin"));
}


char szData[MAX_PATH];
char* ReadRegistryStrVal(char* MainKey, char* Field)
{
	HKEY  hKey1, hKey2;
	DWORD rc;

	DWORD cbData, dwType;
  
	if(RegConnectRegistry(NULL,HKEY_CURRENT_USER, &hKey1) == ERROR_SUCCESS)
	{
		char    szBuffer[260];
    
		strcpy(szBuffer, MainKey);
    
		rc = RegOpenKey(hKey1, szBuffer, &hKey2);
		if(rc == ERROR_SUCCESS)
		{
			cbData = sizeof(szData);
			rc = RegQueryValueEx( hKey2, Field, NULL, &dwType, (LPBYTE)szData, &cbData);
      
			RegCloseKey(hKey2);
		}
		RegCloseKey(hKey1);
	}

	if (rc == ERROR_SUCCESS && cbData != 0)
	{
		return(szData);
	}
	else
	{
		return("");
	}
}

//---------------------------------------------------------------------------------------

void WriteConfiguration()
{
    HKEY    hKey1, hKey2;
        DWORD   rc;
        DWORD cbData;

        // Save current configuration
        if (RegConnectRegistry(NULL,
          HKEY_CURRENT_USER, &hKey1) == ERROR_SUCCESS)
        {
          char  szBuffer[260];
          
          strcpy(szBuffer, MAIN_1964_KEY);
          rc = RegOpenKey(hKey1, szBuffer, &hKey2);
          if(rc != ERROR_SUCCESS)
            rc = RegCreateKey(hKey1, szBuffer, &hKey2);
          if(rc == ERROR_SUCCESS)
          {
            strcpy(szData, gRegSettings.ROMPath);
            cbData = strlen(szData) + 1;            
            RegSetValueEx( hKey2, KEY_ROM_PATH, 0, REG_SZ, (LPBYTE)szData, cbData);
            RegCloseKey(hKey2);
          }
          RegCloseKey(hKey1);
        }


        {
          char  szBuffer[260];
          
          strcpy(szBuffer, MAIN_1964_KEY);
          rc = RegOpenKey(hKey1, szBuffer, &hKey2);
          if(rc != ERROR_SUCCESS)
            rc = RegCreateKey(hKey1, szBuffer, &hKey2);
          if(rc == ERROR_SUCCESS)
          {
            strcpy(szData, gRegSettings.VideoPlugin);
            cbData = strlen(szData) + 1;            
            RegSetValueEx( hKey2, KEY_VIDEO_PLUGIN, 0, REG_SZ, (LPBYTE)szData, cbData);
            RegCloseKey(hKey2);
          }
          RegCloseKey(hKey1);
        }


        {
          char  szBuffer[260];
          
          strcpy(szBuffer, MAIN_1964_KEY);
          rc = RegOpenKey(hKey1, szBuffer, &hKey2);
          if(rc != ERROR_SUCCESS)
            rc = RegCreateKey(hKey1, szBuffer, &hKey2);
          if(rc == ERROR_SUCCESS)
          {
            strcpy(szData, gRegSettings.InputPlugin);
            cbData = strlen(szData) + 1;            
            RegSetValueEx( hKey2, KEY_INPUT_PLUGIN, 0, REG_SZ, (LPBYTE)szData, cbData);
            RegCloseKey(hKey2);
          }
          RegCloseKey(hKey1);
        }


        {
          char  szBuffer[260];
          
          strcpy(szBuffer, MAIN_1964_KEY);
          rc = RegOpenKey(hKey1, szBuffer, &hKey2);
          if(rc != ERROR_SUCCESS)
            rc = RegCreateKey(hKey1, szBuffer, &hKey2);
          if(rc == ERROR_SUCCESS)
          {
            strcpy(szData, gRegSettings.AudioPlugin);
            cbData = strlen(szData) + 1;            
            RegSetValueEx( hKey2, KEY_AUDIO_PLUGIN, 0, REG_SZ, (LPBYTE)szData, cbData);
            RegCloseKey(hKey2);
          }
          RegCloseKey(hKey1);
        }
}