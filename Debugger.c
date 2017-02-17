//Debugger.c
#ifdef _DEBUG

#include <windows.h>
#include <wincon.h>
#include <stdio.h>
#include <string.h>
#include "mainregs.h"
#include "chipmake.h"

HANDLE hSaved;
HANDLE hMainScreen;
FILE* returnString;

#define CONSOLE_INIT 0
#define CONSOLE_DONE 1
#define SHOW_MAIN_SCREEN 2
#define SHOW_SCREEN_1 3

#define COLS 80
#define ROWS 25
void handle_console (unsigned char action);
void SetupMainScreen();
void PrintString(int X, int Y, char* theString);
void DumpRegisters();

char* ToHexString(uint32 theValue);

CHAR_INFO* pchar;

//printf buffer viewport settings
//these are global to run faster. No biggie, it's 
//just a debugger! :)
CHAR_INFO *viewport;
int starty;
unsigned char begin_line;
unsigned char end_line;
uint8 UpdateViewPort;
void InitViewPort();
COORD csize, c0 = {0,0}; 
SMALL_RECT rect;

void RefreshConsole() {
	
		ReadConsoleOutput (hSaved, viewport, csize, c0, &rect);
	    WriteConsoleOutput (hMainScreen, viewport, csize, c0, &rect);
		DumpRegisters();
		while(!kbhit()); getch();
}

void DebuggerMain(void) {
	Counter = 0;
	pchar = (CHAR_INFO*)malloc (sizeof(CHAR_INFO) *  80);
	InitViewPort();

	returnString = (char*)malloc(10);

	SetConsoleTitle("1964 Debugger");
	handle_console(CONSOLE_INIT);
	handle_console(SHOW_SCREEN_1);	
	SetupMainScreen();
	handle_console(SHOW_MAIN_SCREEN);
}

void SetupMainScreen() {
	DWORD numwritten;

	char* strbuf = (char*)malloc(80);

	SetConsoleTextAttribute(hMainScreen, BACKGROUND_BLUE | FOREGROUND_RED |
		FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	strbuf = "   r0:00000000:........  t0:........  s0:........  t8:........   pc:........    \n";
	WriteConsole(hMainScreen, strbuf, strlen(strbuf), &numwritten, NULL);
	strbuf = "   at:00000000:........  t1:........  s1:........  t9:........  mlo:........    \n";
	WriteConsole(hMainScreen, strbuf, strlen(strbuf), &numwritten, NULL);
	strbuf = "   v0:00000000:........  t2:........  s2:........  k0:........  mhi:........    \n";
	WriteConsole(hMainScreen, strbuf, strlen(strbuf), &numwritten, NULL);
	strbuf = "   v1:00000000:........  t3:........  s3:........  k1:........  thr:0           \n";
	WriteConsole(hMainScreen, strbuf, strlen(strbuf), &numwritten, NULL);
	strbuf = "   a0:00000000:........  t4:........  s4:........  gp:........  fra:0           \n";
	WriteConsole(hMainScreen, strbuf, strlen(strbuf), &numwritten, NULL);
	strbuf = "   a1:00000000:........  t5:........  s5:........  sp:........  mio:........    \n";
	WriteConsole(hMainScreen, strbuf, strlen(strbuf), &numwritten, NULL);
	strbuf = "   a2:00000000:........  t6:........  s6:........  s8:........                  \n";
	WriteConsole(hMainScreen, strbuf, strlen(strbuf), &numwritten, NULL);
	strbuf = "   a3:00000000:........  t7:........  s7:........  ra:........                  \n";
	WriteConsole(hMainScreen, strbuf, strlen(strbuf), &numwritten, NULL);
	
	SetConsoleTextAttribute(hMainScreen, BACKGROUND_BLUE | BACKGROUND_GREEN);
	strbuf = "                                                                                \n";
	WriteConsole(hMainScreen, strbuf, strlen(strbuf), &numwritten, NULL);
}

void handle_console (unsigned char action)
{
    static SECURITY_ATTRIBUTES sa;
//    COORD c;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    long    lMode;
// 	SMALL_RECT* NewConsoleSize;
    
	switch (action){
    case CONSOLE_INIT:                  // Init
 		hSaved = GetStdHandle (STD_OUTPUT_HANDLE);              // Save Standard handle
		SetConsoleTextAttribute(hSaved, FOREGROUND_RED | FOREGROUND_GREEN 
			| FOREGROUND_BLUE | BACKGROUND_INTENSITY);
  	    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;                               // Create a new console buffer
        hMainScreen = CreateConsoleScreenBuffer (GENERIC_WRITE | GENERIC_READ, 
                                FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, 
                                CONSOLE_TEXTMODE_BUFFER, NULL);

		GetConsoleScreenBufferInfo(hSaved, &csbi);        // ... with same size 
		SetConsoleScreenBufferSize(hMainScreen, csbi.dwSize);//csbi.dwSize);

		SetConsoleActiveScreenBuffer(hMainScreen);               // ... that becomes standard handle
		SetConsoleMode(hMainScreen, ENABLE_PROCESSED_INPUT);
        break;
		
    case CONSOLE_DONE:                  // Clean Up
        CloseHandle (hSaved);
		CloseHandle (hMainScreen);
        break;

    case SHOW_MAIN_SCREEN:                  // Save
        SetConsoleActiveScreenBuffer (hMainScreen);              // Current = our standard handle
        SetStdHandle(STD_INPUT_HANDLE, hMainScreen);
        break;
		
    case SHOW_SCREEN_1:               // Restore
        SetConsoleActiveScreenBuffer (hSaved);            // Put saved (shell) screen buffer 
        SetStdHandle(STD_INPUT_HANDLE, hSaved);
        break;
    default: ;
       // win32Trace(("Invalid action code %d received in handle_console", action));
    }
}

void DumpRegisters() {
	PrintString(15, 0, ToHexString(MainCPUReg[R0]));
	PrintString(15, 1, ToHexString(MainCPUReg[AT]));
	PrintString(15, 2, ToHexString(MainCPUReg[V0]));
	PrintString(15, 3, ToHexString(MainCPUReg[V1]));
	PrintString(15, 4, ToHexString(MainCPUReg[A0]));
	PrintString(15, 5, ToHexString(MainCPUReg[A1]));
	PrintString(15, 6, ToHexString(MainCPUReg[A2]));
	PrintString(15, 7, ToHexString(MainCPUReg[A3]));

	//MainCPUReg[T0] = "H";
	PrintString(28, 0, ToHexString(MainCPUReg[T0]));
	PrintString(28, 1, ToHexString(MainCPUReg[T1]));
	PrintString(28, 2, ToHexString(MainCPUReg[T2]));
	PrintString(28, 3, ToHexString(MainCPUReg[T3]));
	PrintString(28, 4, ToHexString(MainCPUReg[T4]));
	PrintString(28, 5, ToHexString(MainCPUReg[T5]));
	PrintString(28, 6, ToHexString(MainCPUReg[T6]));
	PrintString(28, 7, ToHexString(MainCPUReg[T7]));
	
	PrintString(41, 0, ToHexString(MainCPUReg[S0]));
	PrintString(41, 1, ToHexString(MainCPUReg[S1]));
	PrintString(41, 2, ToHexString(MainCPUReg[S2]));
	PrintString(41, 3, ToHexString(MainCPUReg[S3]));
	PrintString(41, 4, ToHexString(MainCPUReg[S4]));
	PrintString(41, 5, ToHexString(MainCPUReg[S5]));
	PrintString(41, 6, ToHexString(MainCPUReg[S6]));
	PrintString(41, 7, ToHexString(MainCPUReg[S7]));
	
	PrintString(54, 0, ToHexString(MainCPUReg[T8]));
	PrintString(54, 1, ToHexString(MainCPUReg[T9]));
	PrintString(54, 2, ToHexString(MainCPUReg[K0]));
	PrintString(54, 3, ToHexString(MainCPUReg[K1]));
	PrintString(54, 4, ToHexString(MainCPUReg[GP]));
	PrintString(54, 5, ToHexString(MainCPUReg[SP]));
	PrintString(54, 6, ToHexString(MainCPUReg[S8]));
	PrintString(54, 7, ToHexString(MainCPUReg[RA]));
	
	PrintString(68, 0, ToHexString(pc));
	PrintString(68, 1, "mlo");
	PrintString(68, 2, "mhi");
	PrintString(68, 3, "thr");
	PrintString(68, 4, "fra");
	PrintString(68, 5, "mio");
}

void PrintString(int X, int Y, char* theString) {
	DWORD dw;
	COORD c0 = { 0, 0 };
	COORD csize; 
	SMALL_RECT writeregion, readregion;
//    CHAR_INFO* pchar;
	WORD Attributes;
	int k;

	if (theString != NULL) {
		csize.Y = 1; 
		writeregion.Left = X;
		writeregion.Top = Y;
		writeregion.Bottom = Y;
		writeregion.Right = strlen(theString) + writeregion.Left - 1;
 	

		Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE \
			| BACKGROUND_BLUE | FOREGROUND_INTENSITY;
		for(k=0; k<strlen(theString); k++) {
			pchar[k].Char.UnicodeChar = theString[k];
			pchar[k].Attributes = Attributes;
		}

 		csize.X = strlen(theString); //length of string
		WriteConsoleOutput(hMainScreen, pchar, csize, c0, &writeregion);
	}
}

char* ToHexString(uint32 theValue) {

	if (theValue == 0)
		return("........");
	else {
		sprintf(returnString, "%08X", theValue);
		return((char*)returnString);
	}
}

void freeconsole() {
	handle_console(CONSOLE_DONE);
	free(returnString);
	free(pchar);
	free(viewport);
}

void InitViewPort() {
	//c0 = {0,0};
	end_line = 25;
	begin_line = 9;	
	viewport = malloc (sizeof(CHAR_INFO) *  (end_line-begin_line) * COLS);
	
	csize.X = COLS;
 	csize.Y = end_line-begin_line;
    rect.Left = 0;
    rect.Top = begin_line;
    rect.Right = COLS;
    rect.Bottom = end_line;
}
#endif //end #ifdef _DEBUG