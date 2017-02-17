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
void PrintString(int X, int Y, char* theString);
void DumpRegisters(int VisibleChip);
void InitMainScreen();
void PrintDebugScreen(int WhichView);

//these #defines control which processor is shown in the debugger window
#define MAINCPU_IN_VIEW 0
#define COP0_IN_VIEW 1
#define COP1_IN_VIEW 2
#define COP2_IN_VIEW 3

int Chip_In_View;

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
void DebuggerUI();
void InitViewPort();
void RefreshConsole();

COORD csize, c0 = {0,0}; 
SMALL_RECT rect;

extern void Step_CPU();

void DebuggerUI() 
{
	COORD CursorXY = {9, 8};

	if (UserCommand != 'z') {
		SetConsoleCursorPosition(hMainScreen, CursorXY);
		while (!kbhit());
		UserCommand = getche();
	}
	switch (UserCommand) 
	{
		case 'z' : //Zip through loop and see the end result. (Very useful)
			//My idea here is to be able to quickly run loops that repeat for a long time.
			//When the loop exits, control is given back to the user.
			//I'd like to see this improved. :)
			UpdateViewPort = 0;
			if (MainCPUReg[T1] == 0) {
				UpdateViewPort = 1;
				UserCommand = "";
				RefreshConsole();
			}
			else			
				Step_CPU();
			break;
		case 'n' : Step_CPU(); break;
		case 'w' : //forward one screen 
		{
			Chip_In_View += 1; 
			if (Chip_In_View > 3) Chip_In_View = 3;
			else 
			{
				PrintDebugScreen(Chip_In_View);
				DumpRegisters(Chip_In_View);
			} 
		} break;
		case 'q' : //back one screen
		{ 
			Chip_In_View -= 1; 
			if (Chip_In_View < 0) Chip_In_View = 0;
			else 
			{
				PrintDebugScreen(Chip_In_View);
				DumpRegisters(Chip_In_View);
			} 
		} break;
	} 
}

void RefreshConsole() {
	ReadConsoleOutput (hSaved, viewport, csize, c0, &rect);
	WriteConsoleOutput (hMainScreen, viewport, csize, c0, &rect);
	PrintDebugScreen(Chip_In_View);
	DumpRegisters(Chip_In_View);
}

void DebuggerMain(void) {
	Chip_In_View = MAINCPU_IN_VIEW; //specify which chip is to be displayed in debugger
	Counter = 0;
	pchar = (CHAR_INFO*)malloc (sizeof(CHAR_INFO) *  80);
	InitViewPort();

	returnString = (char*)malloc(10);

	SetConsoleTitle("1964 Debug Console - [r4300i View]");
	handle_console(CONSOLE_INIT);
	handle_console(SHOW_SCREEN_1);	
	InitMainScreen();
	handle_console(SHOW_MAIN_SCREEN);
	RefreshConsole();
}

void InitMainScreen() {
	char* strbuf = (char*)malloc(80);
	DWORD numwritten;
	
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
	strbuf = "Command:                                                                        ";
	WriteConsole(hMainScreen, strbuf, strlen(strbuf), &numwritten, NULL);
}

void PrintDebugScreen(int WhichView) {
	switch(WhichView) {
	case MAINCPU_IN_VIEW : {
		PrintString(0, 0, "   r0:00000000:........  t0:........  s0:........  t8:........   pc:........    ");
		PrintString(0, 1, "   at:00000000:........  t1:........  s1:........  t9:........  mlo:........    ");
		PrintString(0, 2, "   v0:00000000:........  t2:........  s2:........  k0:........  mhi:........    ");
		PrintString(0, 3, "   v1:00000000:........  t3:........  s3:........  k1:........  thr:0           ");
		PrintString(0, 4, "   a0:00000000:........  t4:........  s4:........  gp:........  fra:0           ");
		PrintString(0, 5, "   a1:00000000:........  t5:........  s5:........  sp:........  mio:........    ");
		PrintString(0, 6, "   a2:00000000:........  t6:........  s6:........  s8:........                  ");
		PrintString(0, 7, "   a3:00000000:........  t7:........  s7:........  ra:........                  ");
   } break;
	default : {
		PrintString(0, 0, "  00h:00000000:........ 08h:........ 10h:........ 18h:........   pc:........    ");
		PrintString(0, 1, "  01h:00000000:........ 09h:........ 11h:........ 19h:........  mlo:........    ");
		PrintString(0, 2, "  02h:00000000:........ 0Ah:........ 12h:........ 1Ah:........  mhi:........    ");
		PrintString(0, 3, "  03h:00000000:........ 0Bh:........ 13h:........ 1Bh:........  thr:0           ");
		PrintString(0, 4, "  04h:00000000:........ 0Ch:........ 14h:........ 1Ch:........  fra:0           ");
		PrintString(0, 5, "  05h:00000000:........ 0Dh:........ 15h:........ 1Dh:........  mio:........    ");
		PrintString(0, 6, "  06h:00000000:........ 0Eh:........ 16h:........ 1Eh:........                  ");
		PrintString(0, 7, "  07h:00000000:........ 0Fh:........ 17h:........ 1Fh:........                  ");
		} break;
	} //end switch
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

//Print the registers to the screen of the chip we are viewing.
//This is huge code, but just a debugger. Low priority for optimization ;)
void DumpRegisters(int VisibleChip) {
	char* fetchedConsoleTitle, NewConsoleTitle;
//	DWORD dummy;
	static int prevTitleFlag; //So we don't keep refreshing the title cause that's visibly ugly.

	switch( VisibleChip ) {
	case MAINCPU_IN_VIEW : 
	{
		if ( prevTitleFlag != VisibleChip )	SetConsoleTitle("1964 Debug Console - [r4300i View]");
		PrintString(15, 0, ToHexString(MainCPUReg[R0]));
		PrintString(15, 1, ToHexString(MainCPUReg[AT]));
		PrintString(15, 2, ToHexString(MainCPUReg[V0]));
		PrintString(15, 3, ToHexString(MainCPUReg[V1]));
		PrintString(15, 4, ToHexString(MainCPUReg[A0]));
		PrintString(15, 5, ToHexString(MainCPUReg[A1]));
		PrintString(15, 6, ToHexString(MainCPUReg[A2]));
		PrintString(15, 7, ToHexString(MainCPUReg[A3]));

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
	} break;
	case COP0_IN_VIEW : 
	{	
		if ( prevTitleFlag != VisibleChip )	SetConsoleTitle("1964 Debug Console - [COP0 View]");
		PrintString(15, 0, ToHexString(COP0Reg[INDEX]));     // 0x00
		PrintString(15, 1, ToHexString(COP0Reg[RANDOM]));    // 0x01
		PrintString(15, 2, ToHexString(COP0Reg[ENTRYLO0]));  // 0x02
		PrintString(15, 3, ToHexString(COP0Reg[ENTRYLO1]));  // 0x03
		PrintString(15, 4, ToHexString(COP0Reg[CONTEXT]));   // 0x04
		PrintString(15, 5, ToHexString(COP0Reg[PAGEMASK]));  // 0x05
		PrintString(15, 6, ToHexString(COP0Reg[WIRED]));     // 0x06
		PrintString(15, 7, ToHexString(COP0Reg[RESERVED0])); // 0x07
		
		PrintString(28, 0, ToHexString(COP0Reg[BADVADDR]));  // 0x08
		PrintString(28, 1, ToHexString(COP0Reg[COUNT]));     // 0x09
		PrintString(28, 2, ToHexString(COP0Reg[ENTRYHI]));   // 0x0A
		PrintString(28, 3, ToHexString(COP0Reg[COMPARE]));   // 0x0B
		PrintString(28, 4, ToHexString(COP0Reg[STATUS]));    // 0x0C
		PrintString(28, 5, ToHexString(COP0Reg[CAUSE]));     // 0x0D
		PrintString(28, 6, ToHexString(COP0Reg[EPC]));       // 0x0E
		PrintString(28, 7, ToHexString(COP0Reg[PREVID]));    // 0x0F
		
		PrintString(41, 0, ToHexString(COP0Reg[CONFIG]));    // 0x10
		PrintString(41, 1, ToHexString(COP0Reg[LLADDR]));    // 0x11
		PrintString(41, 2, ToHexString(COP0Reg[WATCHLO]));   // 0x12
		PrintString(41, 3, ToHexString(COP0Reg[WATCHHI]));   // 0x13
		PrintString(41, 4, ToHexString(COP0Reg[XCONTEXT]));  // 0x14
		PrintString(41, 5, ToHexString(COP0Reg[RESERVED1])); // 0x15
		PrintString(41, 6, ToHexString(COP0Reg[RESERVED2])); // 0x16
		PrintString(41, 7, ToHexString(COP0Reg[RESERVED3])); // 0x17

		PrintString(54, 0, ToHexString(COP0Reg[RESERVED4])); // 0x18
		PrintString(54, 1, ToHexString(COP0Reg[RESERVED5])); // 0x19
		PrintString(54, 2, ToHexString(COP0Reg[PERR]));      // 0x1A
		PrintString(54, 3, ToHexString(COP0Reg[CACHEERR]));  // 0x1B
		PrintString(54, 4, ToHexString(COP0Reg[TAGLO]));     // 0x1C
		PrintString(54, 5, ToHexString(COP0Reg[TAGHI]));     // 0x1D
		PrintString(54, 6, ToHexString(COP0Reg[ERROREPC]));  // 0x1E
		PrintString(54, 7, ToHexString(COP0Reg[RESERVED6])); // 0x1F

		PrintString(68, 0, ToHexString(pc));
		PrintString(68, 1, "mlo");
		PrintString(68, 2, "mhi");
		PrintString(68, 3, "thr");
		PrintString(68, 4, "fra");
		PrintString(68, 5, "mio");
	} break;
	case COP1_IN_VIEW : 
	{	
		if ( prevTitleFlag != VisibleChip )	SetConsoleTitle("1964 Debug Console - [COP1 View]");
		PrintString(15, 0, ToHexString(COP1Reg[INDEX]));     // 0x00
		PrintString(15, 1, ToHexString(COP1Reg[RANDOM]));    // 0x01
		PrintString(15, 2, ToHexString(COP1Reg[ENTRYLO0]));  // 0x02
		PrintString(15, 3, ToHexString(COP1Reg[ENTRYLO1]));  // 0x03
		PrintString(15, 4, ToHexString(COP1Reg[CONTEXT]));   // 0x04
		PrintString(15, 5, ToHexString(COP1Reg[PAGEMASK]));  // 0x05
		PrintString(15, 6, ToHexString(COP1Reg[WIRED]));     // 0x06
		PrintString(15, 7, ToHexString(COP1Reg[RESERVED0])); // 0x07
		
		PrintString(28, 0, ToHexString(COP1Reg[BADVADDR]));  // 0x08
		PrintString(28, 1, ToHexString(COP1Reg[COUNT]));     // 0x09
		PrintString(28, 2, ToHexString(COP1Reg[ENTRYHI]));   // 0x0A
		PrintString(28, 3, ToHexString(COP1Reg[COMPARE]));   // 0x0B
		PrintString(28, 4, ToHexString(COP1Reg[STATUS]));    // 0x0C
		PrintString(28, 5, ToHexString(COP1Reg[CAUSE]));     // 0x0D
		PrintString(28, 6, ToHexString(COP1Reg[EPC]));       // 0x0E
		PrintString(28, 7, ToHexString(COP1Reg[PREVID]));    // 0x0F
		
		PrintString(41, 0, ToHexString(COP1Reg[CONFIG]));    // 0x10
		PrintString(41, 1, ToHexString(COP1Reg[LLADDR]));    // 0x11
		PrintString(41, 2, ToHexString(COP1Reg[WATCHLO]));   // 0x12
		PrintString(41, 3, ToHexString(COP1Reg[WATCHHI]));   // 0x13
		PrintString(41, 4, ToHexString(COP1Reg[XCONTEXT]));  // 0x14
		PrintString(41, 5, ToHexString(COP1Reg[RESERVED1])); // 0x15
		PrintString(41, 6, ToHexString(COP1Reg[RESERVED2])); // 0x16
		PrintString(41, 7, ToHexString(COP1Reg[RESERVED3])); // 0x17

		PrintString(54, 0, ToHexString(COP1Reg[RESERVED4])); // 0x18
		PrintString(54, 1, ToHexString(COP1Reg[RESERVED5])); // 0x19
		PrintString(54, 2, ToHexString(COP1Reg[PERR]));      // 0x1A
		PrintString(54, 3, ToHexString(COP1Reg[CACHEERR]));  // 0x1B
		PrintString(54, 4, ToHexString(COP1Reg[TAGLO]));     // 0x1C
		PrintString(54, 5, ToHexString(COP1Reg[TAGHI]));     // 0x1D
		PrintString(54, 6, ToHexString(COP1Reg[ERROREPC]));  // 0x1E
		PrintString(54, 7, ToHexString(COP1Reg[RESERVED6])); // 0x1F

		PrintString(68, 0, ToHexString(pc));
		PrintString(68, 1, "mlo");
		PrintString(68, 2, "mhi");
		PrintString(68, 3, "thr");
		PrintString(68, 4, "fra");
		PrintString(68, 5, "mio");
	} break;
	case COP2_IN_VIEW : 
	{	
		if ( prevTitleFlag != VisibleChip )	SetConsoleTitle("1964 Debug Console - [COP2 View]");
		PrintString(15, 0, ToHexString(COP1Reg[INDEX]));     // 0x00
		PrintString(15, 1, ToHexString(COP1Reg[RANDOM]));    // 0x01
		PrintString(15, 2, ToHexString(COP1Reg[ENTRYLO0]));  // 0x02
		PrintString(15, 3, ToHexString(COP1Reg[ENTRYLO1]));  // 0x03
		PrintString(15, 4, ToHexString(COP1Reg[CONTEXT]));   // 0x04
		PrintString(15, 5, ToHexString(COP1Reg[PAGEMASK]));  // 0x05
		PrintString(15, 6, ToHexString(COP1Reg[WIRED]));     // 0x06
		PrintString(15, 7, ToHexString(COP1Reg[RESERVED0])); // 0x07
		
		PrintString(28, 0, ToHexString(COP1Reg[BADVADDR]));  // 0x08
		PrintString(28, 1, ToHexString(COP1Reg[COUNT]));     // 0x09
		PrintString(28, 2, ToHexString(COP1Reg[ENTRYHI]));   // 0x0A
		PrintString(28, 3, ToHexString(COP1Reg[COMPARE]));   // 0x0B
		PrintString(28, 4, ToHexString(COP1Reg[STATUS]));    // 0x0C
		PrintString(28, 5, ToHexString(COP1Reg[CAUSE]));     // 0x0D
		PrintString(28, 6, ToHexString(COP1Reg[EPC]));       // 0x0E
		PrintString(28, 7, ToHexString(COP1Reg[PREVID]));    // 0x0F
		
		PrintString(41, 0, ToHexString(COP1Reg[CONFIG]));    // 0x10
		PrintString(41, 1, ToHexString(COP1Reg[LLADDR]));    // 0x11
		PrintString(41, 2, ToHexString(COP1Reg[WATCHLO]));   // 0x12
		PrintString(41, 3, ToHexString(COP1Reg[WATCHHI]));   // 0x13
		PrintString(41, 4, ToHexString(COP1Reg[XCONTEXT]));  // 0x14
		PrintString(41, 5, ToHexString(COP1Reg[RESERVED1])); // 0x15
		PrintString(41, 6, ToHexString(COP1Reg[RESERVED2])); // 0x16
		PrintString(41, 7, ToHexString(COP1Reg[RESERVED3])); // 0x17

		PrintString(54, 0, ToHexString(COP1Reg[RESERVED4])); // 0x18
		PrintString(54, 1, ToHexString(COP1Reg[RESERVED5])); // 0x19
		PrintString(54, 2, ToHexString(COP1Reg[PERR]));      // 0x1A
		PrintString(54, 3, ToHexString(COP1Reg[CACHEERR]));  // 0x1B
		PrintString(54, 4, ToHexString(COP1Reg[TAGLO]));     // 0x1C
		PrintString(54, 5, ToHexString(COP1Reg[TAGHI]));     // 0x1D
		PrintString(54, 6, ToHexString(COP1Reg[ERROREPC]));  // 0x1E
		PrintString(54, 7, ToHexString(COP1Reg[RESERVED6])); // 0x1F

		PrintString(68, 0, ToHexString(pc));
		PrintString(68, 1, "mlo");
		PrintString(68, 2, "mhi");
		PrintString(68, 3, "thr");
		PrintString(68, 4, "fra");
		PrintString(68, 5, "mio");
	} //end case
	} //end switch
	prevTitleFlag = VisibleChip;
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