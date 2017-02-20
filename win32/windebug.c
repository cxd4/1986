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

#ifdef WINDEBUG_1964

#include <windows.h>
#include <stdio.h>
#include "..\options.h"
#include "..\timer.h"
#include "windebug.h"
#include "wingui.h"
#include "..\globals.h"
#include "..\r4300i.h"
#include "..\hardware.h"
#include "..\n64rcp.h"
#include "..\interrupt.h"
#include "..\DbgPrint.h"
#include "..\emulator.h"
#include "..\memory.h"

#define  MIPS_VIEW 0
#define INTEL_VIEW 1
int EndianView;
extern HANDLE CPUThreadHandle;

#define LOGGING
static FILE* fp=NULL;
static BOOL LoggingEnabled;

static BOOL DisplayCompareReg=0;		// Use by opcode debugger
static BOOL DisplayDynaCompareReg = 0;

//---------------------------------------------------------------------------------------

LRESULT CALLBACK VIREGS(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int i;

    switch(message)
    {
    case WM_CLOSE:
        PostMessage(hDlg, WM_DESTROY,0, 0L);
        break; 
    case WM_DESTROY:
        hVIRegwnd = NULL;
        EndDialog(hDlg,TRUE);
        break;
    case WM_INITDIALOG:
        VIREGEDIT[0]=GetDlgItem(hDlg,IDC_VIWIDTH);
        VIREGEDIT[1]=GetDlgItem(hDlg,IDC_VIHEIGHT);
        VIREGEDIT[2]=GetDlgItem(hDlg,IDC_VIORIGIN);
        VIREGEDIT[3]=GetDlgItem(hDlg,IDC_VIPIXSIZE);
        UpdateVIReg();
        return(TRUE);
        break;
    case WM_COMMAND:
        switch (wParam)
        {
            case IDOK:
                EndDialog(hDlg, TRUE);
                break;
        }
        break;
    }
    return(FALSE);
}

//---------------------------------------------------------------------------------------
LRESULT CALLBACK DEBUGGER(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int i;

    switch(message)
    {
    case WM_CLOSE:
        PostMessage(hDlg, WM_DESTROY,0, 0L);
        break; 
    case WM_DESTROY:
        hRegswnd = NULL;
        EndDialog(hDlg,TRUE);
        break;
    case WM_INITDIALOG:
        for (i=0;i<32;i++)
        {
             GPREDIT[i] =   GetDlgItem(hDlg, IDC_GPR1+i);
            COP0EDIT[i] =   GetDlgItem(hDlg, IDC_COP01+i);
            COP1EDIT[i] =   GetDlgItem(hDlg, IDC_FPR0+i);
        }
        MISCEDIT[0] = GetDlgItem(hDlg,IDC_PC);
        MISCEDIT[1] = GetDlgItem(hDlg,IDC_LLBIT);
        MISCEDIT[2] = GetDlgItem(hDlg,IDC_MULTHI);
        MISCEDIT[3] = GetDlgItem(hDlg,IDC_MULTLO);
        UpdateGPR();
        UpdateCOP0();
        UpdateFPR();
        UpdateMisc();
#ifdef ENABLE_OPCODE_DEBUGGER
		DisplayCompareReg = 0;
		DisplayDynaCompareReg = 0;
#endif
        return(TRUE);
        break;
    case WM_COMMAND:
        switch (wParam)
        {
            case IDOK:
                CloseDebugger();
                break;
            case IDC_STEPCPU: //..TODO
				InterpreterStepCPU();
		        UpdateGPR();
				UpdateCOP0();
				UpdateFPR();
				UpdateMisc();
				UpdateVIReg();
                break;
            case ID_RUNTO:
                Set_Breakpoint();
                break;
#ifdef ENABLE_OPCODE_DEBUGGER
			case IDC_OPCODE_COMPARE:
				DisplayCompareReg = 1 - DisplayCompareReg;
				break;
			case IDC_OPCODE_DYNA_COMPARE:
				DisplayDynaCompareReg = 1 - DisplayDynaCompareReg;
				break;
#endif
            case IDC_CHECK1:
                if (DebuggerEnabled == 0)
				{
                    DebuggerEnabled = 1;
					if( Emu_Is_Running )
					{
						PauseEmulator();
						ResumeEmulator(TRUE);	// Need to init emu
					}
				}
                else
				{
                    DebuggerEnabled = 0;
					if( Emu_Is_Running )
					{
						PauseEmulator();
						ResumeEmulator(TRUE);	// Need to init emu
					}
				}
                break;

#ifdef LOGGING
			case IDC_CHECK_LOGGING:
				if( LoggingEnabled )
					LoggingEnabled = FALSE;
				else
				{
					LoggingEnabled = TRUE;
					if( fp == NULL )	// log file is not opened yet
					{
						// Open log file to write
						if( (fp=fopen("debug.log","w")) == NULL )
							DisplayError("Error to create debug log file");
						LoggingEnabled = FALSE;
					}
				}
				break;
#endif

            case IDC_TRIGGER_SP_INTERRUPT:
				Trigger_SPInterrupt();
                break;
			case IDC_REGS_FLUSH:
		        UpdateGPR();
				UpdateCOP0();
				UpdateFPR();
				UpdateMisc();
				UpdateVIReg();
				break;
        }
        break;
    }
    return(FALSE);
}

//---------------------------------------------------------------------------------------

extern char op_str[0xff];
extern BOOL pausegame;
extern void AUDIO_AiUpdate(BOOL update);
enum GAME_STOP_REASON { EMURUNNING=1, EMUSTOP=2, EMUPAUSE=3, EMUSWITCHCORE=4, EMURESUME };
extern enum GAME_STOP_REASON reason_to_stop;

void HandleBreakpoint(uint32 Instruction) {
    if (RUN_TO_ADDRESS_ACTIVE)
    {
        if (gHWS_pc == BreakAddress)
        {
            //RUN_TO_ADDRESS_ACTIVE = FALSE;
            //BreakAddress = 0;
            RefreshDebugger();
            
            //DebuggerEnabled = 1;
            WinDynDebugPrintInstruction(Instruction);
			RefreshDebugger();
			TRACE1("breakpoint %08X is hit", BreakAddress);
            DisplayError("%s\nBreakpoint Encountered.", op_str);
            //SuspendThread(CPUThreadHandle);
			reason_to_stop = EMUPAUSE;
			Emu_Keep_Running = FALSE;
			AUDIO_AiUpdate(TRUE);
        }
    }
}

void PrintTLB(BOOL all)
{
	int i;
	uint32 g;

	for(i=0; i<=NTLBENTRIES; i++ )
	{
		if( gMS_TLB[i].valid || all == TRUE )
		{
			g = (gMS_TLB[i].EntryLo0 & gMS_TLB[i].EntryLo1 & 0x01);

			TRACE3( "TLB [%d], G=%d, V=%d", i,g, gMS_TLB[i].valid);
			TRACE2( "PAGEMASK = 0x%08X, ENTRYHI = 0x%08X", (uint32)gMS_TLB[i].PageMask, gMS_TLB[i].EntryHi);
			TRACE2( "ENTRYLO1 = 0x%08X, ENTRYLO0 = 0x%08X", gMS_TLB[i].EntryLo1, gMS_TLB[i].EntryLo0);
			TRACE2( "LoCompare = 0x%08X, MyHiMask = 0x%08X", gMS_TLB[i].LoCompare, gMS_TLB[i].MyHiMask);
		}
    }
}

//---------------------------------------------------------------------------------------

void UpdateMemList()
{
    char final[80];
    LPSTR mem[12];

    uint8* realmemloc;
    uint32 memloc;
    int i;

    /* clear memory list */
    SendMessage(MEMLISTBOX,LB_RESETCONTENT ,0,0);

    /* get address */
    SendMessage(MEMLOCEDIT, WM_GETTEXT, 10, (LPARAM) (LPSTR) mem);

    if (Rom_Loaded == 0) 
    {
        sprintf(final, "--- Please load ROM first ---");
        SendMessage(MEMLISTBOX,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)final);
    }
    else 
    {
        memloc = StrToHex((char*)mem) & 0xFFFFFFF0;
        if (memloc > 0xBFFFFFFF) 
        {
            sprintf(final, "--- Out of memory range ---");  
            SendMessage(MEMLISTBOX,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)final);
        }
        else
        {
            realmemloc = pLOAD_UBYTE_PARAM(memloc);
            if (realmemloc == NULL) 
            {
                sprintf(final, "--- Invalid Segment ---");
                SendMessage(MEMLISTBOX,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)final);
            }
            else 
            {
                realmemloc += ((memloc & 0xFFFF)); 
                if (realmemloc == NULL) 
                {
                    sprintf(final, "--- Invalid Displacement ---");
                    SendMessage(MEMLISTBOX,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)final);
                }
                else for (i=0;i<11;i++)
                {
                    if (EndianView == INTEL_VIEW) /* intel view on intel machine */
                    {
                        sprintf 
                        (
                            final, "%08X:  %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X  %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c",memloc, 
                            realmemloc[0x00], realmemloc[0x01], realmemloc[0x02], realmemloc[0x03],
                            realmemloc[0x04], realmemloc[0x05], realmemloc[0x06], realmemloc[0x07],                     
                            realmemloc[0x08], realmemloc[0x09], realmemloc[0x0A], realmemloc[0x0B],
                            realmemloc[0x0C], realmemloc[0x0D], realmemloc[0x0E], realmemloc[0x0F],
                            
                            realmemloc[0x00], realmemloc[0x01], realmemloc[0x02], realmemloc[0x03],
                            realmemloc[0x04], realmemloc[0x05], realmemloc[0x06], realmemloc[0x07],                     
                            realmemloc[0x08], realmemloc[0x09], realmemloc[0x0A], realmemloc[0x0B],
                            realmemloc[0x0C], realmemloc[0x0D], realmemloc[0x0E], realmemloc[0x0F]
                        );
                    }
                    else /* it is MIPS view (on an intel machine) */
                    {
                        sprintf 
                        (
                            final, "%08X:  %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X  %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c",memloc, 
                            realmemloc[0x03], realmemloc[0x02], realmemloc[0x01], realmemloc[0x00],
                            realmemloc[0x07], realmemloc[0x06], realmemloc[0x05], realmemloc[0x04],                     
                            realmemloc[0x0B], realmemloc[0x0A], realmemloc[0x09], realmemloc[0x08],
                            realmemloc[0x0F], realmemloc[0x0E], realmemloc[0x0D], realmemloc[0x0C],

                            realmemloc[0x03], realmemloc[0x02], realmemloc[0x01], realmemloc[0x00],
                            realmemloc[0x07], realmemloc[0x06], realmemloc[0x05], realmemloc[0x04],                     
                            realmemloc[0x0B], realmemloc[0x0A], realmemloc[0x09], realmemloc[0x08],
                            realmemloc[0x0F], realmemloc[0x0E], realmemloc[0x0D], realmemloc[0x0C]
                        );
                    }
                    SendMessage(MEMLISTBOX,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)final);
                    realmemloc += 0x10;
                    memloc += 0x10;
                }   /* end for */
            }   /* end if (realmemloc == NULL)...else */
        }   /* end if (memloc > 0xBFFFFFFF)...else */
    }   /* end if (Rom_Loaded)...else */
}

#define LINETODASM 0x100
void MemoryDeAssemble(void)
{
    char final[80];
    LPSTR mem[12];

    uint32* realmemloc;
    uint32  memloc;
    int i;

    /* clear memory list */
    SendMessage(MEMLISTBOX,LB_RESETCONTENT ,0,0);

    /* get address */
    SendMessage(MEMLOCEDIT, WM_GETTEXT, 10, (LPARAM) (LPSTR) mem);

    if (Rom_Loaded == 0) 
    {
        sprintf(final, "--- Please load ROM first ---");
        SendMessage(MEMLISTBOX,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)final);
    }
    else 
    {
        memloc = StrToHex((char*)mem) & 0xFFFFFE00;
        //if (memloc > 0xBFFFFFFF) 
        //{
        //    sprintf(final, "--- Out of memory range ---");  
        //    SendMessage(MEMLISTBOX,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)final);
        //}
        //else
        {
			uint32 translatepc;

			if ((memloc ^ 0x80000000) & 0xC0000000 ) //(gHWS_pc & 0xC0000000) != 0x80000000) 
			{                                                   
				translatepc = TranslateITLBAddress(memloc);
				realmemloc = pLOAD_UWORD_PARAM(translatepc);	//TODO: valloc2
			}
			else
			{
	            realmemloc = pLOAD_UWORD_PARAM(memloc);
			}

            if (realmemloc == NULL) 
            {
                sprintf(final, "--- Invalid Segment ---");
                SendMessage(MEMLISTBOX,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)final);
            }
            else 
            {
                realmemloc += ((memloc & 0xFFFF)); 
                if (realmemloc == NULL) 
                {
                    sprintf(final, "--- Invalid Displacement ---");
                    SendMessage(MEMLISTBOX,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)final);
                }
                else
				{
					uint32 temppc = gHWS_pc;
					uint32 Instruction;
					uint32 Addr = StrToHex((char*)mem);


					gHWS_pc = memloc;

					for (i=0;i<LINETODASM;i++)
					{
						if ((gHWS_pc ^ 0x80000000) & 0xC0000000 ) //(gHWS_pc & 0xC0000000) != 0x80000000) 
						{                                                   
							translatepc = TranslateITLBAddress(gHWS_pc);
							Instruction = LOAD_UWORD_PARAM(translatepc);
						}
						else
						{
							Instruction = LOAD_UWORD_PARAM(gHWS_pc);
						}
						
						DebugPrintInstructionWithOutRefresh(Instruction); 
						SendMessage(MEMLISTBOX,LB_INSERTSTRING,(WPARAM)i,(LPARAM)op_str);

						gHWS_pc += 4;
						realmemloc++;
						memloc++;
					}

					gHWS_pc = temppc;
					
                }   /* end for */
            }   /* end if (realmemloc == NULL)...else */
        }   /* end if (memloc > 0xBFFFFFFF)...else */
    }   /* end if (Rom_Loaded)...else */
}

void MemoryDeAssembleNextPage()
{
	LPSTR mem[12];
    uint32  memloc;

    /* get address */
    SendMessage(MEMLOCEDIT, WM_GETTEXT, 10, (LPARAM) (LPSTR) mem);

    memloc = StrToHex((char*)mem) & 0xFFFFFC00;
	memloc += 4*LINETODASM;

	sprintf((char*)mem,"%X",memloc);

	SendMessage(MEMLOCEDIT, WM_SETTEXT, 10, (LPARAM) (LPSTR) mem);

	MemoryDeAssemble();
}

void MemoryDeAssemblePrevPage()
{
	LPSTR mem[12];
    uint32  memloc;

    /* get address */
    SendMessage(MEMLOCEDIT, WM_GETTEXT, 10, (LPARAM) (LPSTR) mem);

    memloc = StrToHex((char*)mem) & 0xFFFFFC00;
	memloc -= 4*LINETODASM;

	sprintf((char*)mem,"%X",memloc);

	SendMessage(MEMLOCEDIT, WM_SETTEXT, 10, (LPARAM) (LPSTR) mem);

	MemoryDeAssemble();
}

//---------------------------------------------------------------------------------------

LRESULT CALLBACK MEMORYPROC(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
    case WM_CLOSE:
        PostMessage(hDlg, WM_DESTROY,0, 0L);
        break; 
    case WM_DESTROY:
        hMemorywnd = NULL;
        EndDialog(hDlg,TRUE);
        break;
    case WM_INITDIALOG:
        MEMLISTBOX=GetDlgItem(hDlg,IDC_MEMLIST);
        MEMLOCEDIT=GetDlgItem(hDlg,IDC_MEMEDIT);
        break;
    case WM_COMMAND:
        switch (wParam)
        {   
            case IDC_UPDATE: 
				UpdateMemList(); 
				break;
            case IDC_VIEWMIPS: 
                EndianView = MIPS_VIEW; 
                UpdateMemList();
                break;
            case IDC_VIEWINTEL:
                EndianView = INTEL_VIEW; 
                UpdateMemList();
                break;
			case IDC_DEASSEMBLE:
				MemoryDeAssemble();
				break;
			case IDC_DASM_NEXTPAGE:
				MemoryDeAssembleNextPage();
				break;
			case IDC_DASM_PREVPAGE:
				MemoryDeAssemblePrevPage();
				break;
        }
        break;
    }
    return (FALSE);
}

//---------------------------------------------------------------------------------------

LRESULT APIENTRY CODELISTPROC(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
    switch (message) 
    {
    case WM_CLOSE:
        PostMessage(hDlg, WM_DESTROY,0, 0L);
        break; 
    case WM_DESTROY:
        hCodeListwnd = NULL;
        EndDialog(hDlg,TRUE);
        break;
        case WM_INITDIALOG:
            CODEEDIT[0]=GetDlgItem(hDlg,IDC_CODELIST);
            return (TRUE);

        case WM_COMMAND:
            switch (wParam)
            {
			case IDC_CLEAR_CODELIST:
				SendMessage(CODEEDIT[0],LB_RESETCONTENT ,0,0);
				OpCount=0;
				break;
			case IDC_PAUSE_CODELIST:
				Pause();
				break;
			case IDC_RUN_CODELIST:
				Play();
				break;
			case IDC_PRINT_TLB:
				PrintTLB(FALSE);
				break;
			case IDC_PRINT_TLBALL:
				PrintTLB(TRUE);
				break;
			case IDC_STEPCPU_CODELIST:
				InterpreterStepCPU();
		        UpdateGPR();
				UpdateCOP0();
				UpdateFPR();
				UpdateMisc();
				UpdateVIReg();
				break;
			case IDC_MANUAL_SI:
				Trigger_Interrupt_Without_Mask(MI_INTR_SI);
				break;
			case IDC_MANUAL_AI:
				Trigger_Interrupt_Without_Mask(MI_INTR_AI);
				break;
			case IDC_MANUAL_VI:
				Trigger_Interrupt_Without_Mask(MI_INTR_VI);
				break;
			case IDC_MANUAL_SP:
				Trigger_Interrupt_Without_Mask(MI_INTR_SP);
				break;
			case IDC_MANUAL_DP:
				Trigger_Interrupt_Without_Mask(MI_INTR_DP);
				break;

            }
        break;
    }
    return (FALSE);
}

//---------------------------------------------------------------------------------------

LRESULT APIENTRY COP2VEC1(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
    int el;

    switch (message) 
    {
        case WM_INITDIALOG:
            for (el=0;el<8;el++)    {   COP2VECEDIT[0][el]=GetDlgItem(hDlg,IDC_VEC11+el);   }
            for (el=0;el<8;el++)    {   COP2VECEDIT[1][el]=GetDlgItem(hDlg,IDC_VEC21+el);   }
            for (el=0;el<8;el++)    {   COP2VECEDIT[2][el]=GetDlgItem(hDlg,IDC_VEC31+el);   }
            for (el=0;el<8;el++)    {   COP2VECEDIT[3][el]=GetDlgItem(hDlg,IDC_VEC41+el);   }
            for (el=0;el<8;el++)    {   COP2VECEDIT[4][el]=GetDlgItem(hDlg,IDC_VEC51+el);   }
            for (el=0;el<8;el++)    {   COP2VECEDIT[5][el]=GetDlgItem(hDlg,IDC_VEC61+el);   }
            for (el=0;el<8;el++)    {   COP2VECEDIT[6][el]=GetDlgItem(hDlg,IDC_VEC71+el);   }
            for (el=0;el<8;el++)    {   COP2VECEDIT[7][el]=GetDlgItem(hDlg,IDC_VEC81+el);   }
            
            UpdateCOP2Vec1();

            return (TRUE);
    }
    return (FALSE);
}

//---------------------------------------------------------------------------------------

void UpdateCOP2Vec1()
{
//    char String[30];
    int el;
    int vec;

    for (vec=0;vec<8;vec++)
    {
        for (el=0;el<8;el++)
        {
//          sprintf(String, "%08X", RSPVec[vec][el]);
//          SendMessage(COP2VECEDIT[vec][el],WM_SETTEXT,0,(LPARAM)(LPCTSTR)String);
        }
    }
}

//---------------------------------------------------------------------------------------

LRESULT APIENTRY COP2VEC2(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
    int el;

    switch (message) 
    {
        case WM_INITDIALOG:
            for (el=0;el<8;el++)    {   COP2VECEDIT[ 8][el]=GetDlgItem(hDlg,IDC_VEC91 +el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[ 9][el]=GetDlgItem(hDlg,IDC_VEC101+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[10][el]=GetDlgItem(hDlg,IDC_VEC111+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[11][el]=GetDlgItem(hDlg,IDC_VEC121+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[12][el]=GetDlgItem(hDlg,IDC_VEC131+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[13][el]=GetDlgItem(hDlg,IDC_VEC141+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[14][el]=GetDlgItem(hDlg,IDC_VEC151+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[15][el]=GetDlgItem(hDlg,IDC_VEC161+el); }

            UpdateCOP2Vec2();

            return (TRUE);
    }
    return (FALSE);
}

//---------------------------------------------------------------------------------------

void UpdateCOP2Vec2()
{
//    char String[30];
    int el;
    int vec;

    for (vec=8;vec<16;vec++)
    {
        for (el=0;el<8;el++)
        {
//          sprintf(String, "%08X", RSPVec[vec][el]);
//          SendMessage(COP2VECEDIT[vec][el],WM_SETTEXT,0,(LPARAM)(LPCTSTR)String);
        }
    }
}

//---------------------------------------------------------------------------------------

LRESULT APIENTRY COP2VEC3(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
    int el;

    switch (message) 
    {
        case WM_INITDIALOG:
            for (el=0;el<8;el++)    {   COP2VECEDIT[16][el]=GetDlgItem(hDlg,IDC_VEC171+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[17][el]=GetDlgItem(hDlg,IDC_VEC181+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[18][el]=GetDlgItem(hDlg,IDC_VEC191+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[19][el]=GetDlgItem(hDlg,IDC_VEC201+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[20][el]=GetDlgItem(hDlg,IDC_VEC211+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[21][el]=GetDlgItem(hDlg,IDC_VEC221+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[22][el]=GetDlgItem(hDlg,IDC_VEC231+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[23][el]=GetDlgItem(hDlg,IDC_VEC241+el); }

            UpdateCOP2Vec3();

            return (TRUE);
    }
    return (FALSE);
}

//---------------------------------------------------------------------------------------

void UpdateCOP2Vec3()
{
//    char String[30];
    int el;
    int vec;

    for (vec=16;vec<24;vec++)
    {
        for (el=0;el<8;el++)
        {
//          sprintf(String, "%08X", RSPVec[vec][el]);
//          SendMessage(COP2VECEDIT[vec][el],WM_SETTEXT,0,(LPARAM)(LPCTSTR)String);
        }
    }
}

//---------------------------------------------------------------------------------------

LRESULT APIENTRY COP2VEC4(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
    int el;

    switch (message) 
    {
        case WM_INITDIALOG:
            for (el=0;el<8;el++)    {   COP2VECEDIT[24][el]=GetDlgItem(hDlg,IDC_VEC251+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[25][el]=GetDlgItem(hDlg,IDC_VEC261+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[26][el]=GetDlgItem(hDlg,IDC_VEC271+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[27][el]=GetDlgItem(hDlg,IDC_VEC281+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[28][el]=GetDlgItem(hDlg,IDC_VEC291+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[29][el]=GetDlgItem(hDlg,IDC_VEC301+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[30][el]=GetDlgItem(hDlg,IDC_VEC311+el); }
            for (el=0;el<8;el++)    {   COP2VECEDIT[31][el]=GetDlgItem(hDlg,IDC_VEC321+el); }

            UpdateCOP2Vec4();

            return (TRUE);
    }
    return (FALSE);
}

//---------------------------------------------------------------------------------------

void UpdateCOP2Vec4()
{
//    char String[30];
    int el;
    int vec;

    for (vec=24;vec<32;vec++)
    {
        for (el=0;el<8;el++)
        {
//          sprintf(String, "%08X", RSPVec[vec][el]);
//          SendMessage(COP2VECEDIT[vec][el],WM_SETTEXT,0,(LPARAM)(LPCTSTR)String);
        }
    }
}

//---------------------------------------------------------------------------------------
/* This is a very specific StrToHex(). We assume that the length of HexStr is 8 bytes.
   This works 4 now. */
uint32 StrToHex(char* HexStr) {
    int k;
    _int32 temp;
    
    for (k=0; k<8; k++)
	{
        if (HexStr[k] <= '9' && HexStr[k] >= '0')
		{
            temp = temp << 4;
            temp += HexStr[k] - '0'; 
        }
		else if (HexStr[k] <= 'F' && HexStr[k] >= 'A')
		{
            temp = temp << 4;
            temp += HexStr[k] - 'A' + 10;
        }
		else
		{
			return(0);
		}
	}

    return(temp);
}

//---------------------------------------------------------------------------------------

LRESULT APIENTRY GETHEX(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
    char no[50];
    _int32 tempint;

    switch (message) 
    {
        case WM_INITDIALOG:
			sprintf(generalmessage, "%08X", BreakAddress);
			SetDlgItemText(hDlg, IDC_VALUE, generalmessage);
            return (TRUE);
            break;

        case WM_COMMAND:
            switch (wParam) 
            {
                case IDOK:
                    SendDlgItemMessage(hDlg, IDC_VALUE, WM_GETTEXT, 10, (LPARAM) (LPSTR) no);
                    /* Set The break address */
                    if (tempint = StrToHex(no)) {
                        RUN_TO_ADDRESS_ACTIVE = TRUE;
                        BreakAddress = tempint;
                    }
                    else
					{
						RUN_TO_ADDRESS_ACTIVE = FALSE;
                        //MessageBox(hwnd, "Illegal Break Address", "Error:", MB_OK);
						MessageBox(hwnd, "Break Point is disabled.", "Note:", MB_OK);
					}
                    
                    EndDialog(hDlg, TRUE);
                    hEnterHexwnd = NULL;
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, TRUE);
                    hEnterHexwnd = NULL;
                    break;
            }
            break;
    }
    return (FALSE);
}

//---------------------------------------------------------------------------------------

void UpdateVIReg()
{
    char String[30];
    
    uint32 height, status;

    sprintf(String, "%i", VI_WIDTH_REG);
    SendMessage(VIREGEDIT[0],WM_SETTEXT,0,(LPARAM)(LPCTSTR)String);

    height = (VI_WIDTH_REG * 3) >> 2;
    sprintf(String, "%i", height);
    SendMessage(VIREGEDIT[1],WM_SETTEXT,0,(LPARAM)(LPCTSTR)String);

    sprintf(String, "%08X", VI_ORIGIN_REG);
    SendMessage(VIREGEDIT[2],WM_SETTEXT,0,(LPARAM)(LPCTSTR)String);

    status = VI_STATUS_REG;
    status &= 0x0003; 
    switch (status) 
    {
        case 0:
            sprintf(String, "No Data / Sync");
            break;
        case 1:
            sprintf(String, "Reserved!");
            break;
        case 2:
            sprintf(String, "5/5/5/1 : 16 bit");
            break;
        case 3:
            sprintf(String, "8/8/8/8 : 32 bit");
            break;
    } 
    SendMessage(VIREGEDIT[3],WM_SETTEXT,0,(LPARAM)(LPCTSTR)String);
}

//---------------------------------------------------------------------------------------

void Set_Breakpoint()
{
    if (!hEnterHexwnd)
        hEnterHexwnd = CreateDialog(hInst, "ENTERHEX", hwnd, (DLGPROC)GETHEX);
}

void Clear_Breakpoint()
{
	RUN_TO_ADDRESS_ACTIVE = FALSE;
}
//---------------------------------------------------------------------------------------

void RefreshOpList(char *opcode)
{
	static count = 0;

	if( DebuggerActive == FALSE ) return;
    strcpy(opBuffer, opcode);

#ifdef LOGGING
	if( LoggingEnabled && count <800000)	// Not to create too big a log file
	{
		fprintf(fp, opcode);
		fputc(13, fp);
		fputc(10, fp);
		count++;
	}
	else
	{
		SendMessage(CODEEDIT[0],LB_INSERTSTRING,(WPARAM)-1,(LPARAM)opcode);
		OpCount++;
		SendMessage(CODEEDIT[0],LB_SETCURSEL,(WPARAM)OpCount-1,(LPARAM)0);

		if (gHWS_COP0Reg[COUNT] == NextClearCode)
		{
			SendMessage(CODEEDIT[0],LB_RESETCONTENT ,0,0);
	        NextClearCode += 250;
		    OpCount = 0;
		}
	}
#endif
}

//---------------------------------------------------------------------------------------

void UpdateMisc()
{
    char String[17];

	HardwareState * pstate;
	pstate = &gHardwareState;

#ifdef ENABLE_OPCODE_DEBUGGER
	if( DisplayCompareReg )
	{
		pstate = &gHardwareState_Interpreter_Compare;
	}
	else if ( DisplayDynaCompareReg )
	{
		pstate = &gHardwareState_Flushed_Dynarec_Compare;
	}
#endif

    /* PC */
    sprintf(String, "%08X", (uint32)pstate->pc);
    SendMessage(MISCEDIT[0],WM_SETTEXT,0,(LPARAM)(LPCTSTR)String);

    /* LLBit */
    sprintf(String, "%08X", (uint32)pstate->LLbit);
    SendMessage(MISCEDIT[1],WM_SETTEXT,0,(LPARAM)(LPCTSTR)String);

    /* MultHI */
    sprintf(String, "%016I64X", pstate->GPR[gHI]);
    SendMessage(MISCEDIT[2],WM_SETTEXT,0,(LPARAM)(LPCTSTR)String);
    
    /* MultLO */
    sprintf(String, "%016I64X", pstate->GPR[gLO]);
    SendMessage(MISCEDIT[3],WM_SETTEXT,0,(LPARAM)(LPCTSTR)String);
}

//---------------------------------------------------------------------------------------

void UpdateGPR()
{
    int i;
    char String[17];

	HardwareState * pstate;
	pstate = &gHardwareState;

#ifdef ENABLE_OPCODE_DEBUGGER
	if( DisplayCompareReg )
	{
		pstate = &gHardwareState_Interpreter_Compare;
	}
	else if ( DisplayDynaCompareReg )
	{
		pstate = &gHardwareState_Flushed_Dynarec_Compare;
	}
#endif

    for (i=0;i<64;i++)
    {
//        if (gHWS_GPR[i] == 0)
//            sprintf(String, "........");
//        else
            sprintf(String, "%016I64X", pstate->GPR[i]);

        SendMessage(GPREDIT[i],WM_SETTEXT,0,(LPARAM)(LPCTSTR)String);
    }
}

//---------------------------------------------------------------------------------------

void UpdateCOP0()
{
    char String[10];
    int i;

	HardwareState * pstate;
	pstate = &gHardwareState;

#ifdef ENABLE_OPCODE_DEBUGGER
	if( DisplayCompareReg )
	{
		pstate = &gHardwareState_Interpreter_Compare;
	}
	else if ( DisplayDynaCompareReg )
	{
		pstate = &gHardwareState_Flushed_Dynarec_Compare;
	}
#endif

    for (i=0;i<32;i++)
    {
//        if (gHWS_COP0Reg[i] == 0)
//            sprintf(String, "........");
//        else
            sprintf(String, "%08X", (uint32)pstate->COP0Reg[i]);

        SendMessage(COP0EDIT[i],WM_SETTEXT,0,(LPARAM)(LPCTSTR)String);
    }
}

//---------------------------------------------------------------------------------------

void UpdateFPR()
{
    char String[17];
    int i;

	HardwareState * pstate;
	pstate = &gHardwareState;

#ifdef ENABLE_OPCODE_DEBUGGER
	if( DisplayCompareReg )
	{
		pstate = &gHardwareState_Interpreter_Compare;
	}
	else if ( DisplayDynaCompareReg )
	{
		pstate = &gHardwareState_Flushed_Dynarec_Compare;
	}
#endif

    for (i=0;i<32;i++)
    {
//        if (gHWS_COP1Reg[i] == 0)
//            sprintf(String, "........");
//        else
        sprintf(String, "%016I64X", (_int64)pstate->fpr32[i]);
        SendMessage(COP1EDIT[i],WM_SETTEXT,0,(LPARAM)(LPCTSTR)String);
    }
}   

//---------------------------------------------------------------------------------------

void RefreshDebugger()
{
#ifndef LOGGING
    if (hRegswnd != NULL)
    {
        UpdateGPR();
        UpdateCOP0();
        UpdateFPR();
        UpdateMisc();
        UpdateVIReg();
    }
#endif
}

//---------------------------------------------------------------------------------------

void CreateCOP2Page()
{
    PROPSHEETPAGE psp[6];
    PROPSHEETHEADER psh;

    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USETITLE;
    psp[0].hInstance = hInst;
    psp[0].pszTemplate = "CODELIST";
    psp[0].pszIcon = NULL;
    psp[0].pfnDlgProc = (DLGPROC)CODELISTPROC;
    psp[0].pszTitle = "CodeList";
    psp[0].lParam = 0;
    
    psp[1].dwSize = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags = PSP_USETITLE;
    psp[1].hInstance = hInst;
    psp[1].pszTemplate = "MEMORY";
    psp[1].pszIcon = NULL;
    psp[1].pfnDlgProc = (DLGPROC)MEMORYPROC;
    psp[1].pszTitle = "Memory";
    psp[1].lParam = 0;
    
    psp[2].dwSize = sizeof(PROPSHEETPAGE);
    psp[2].dwFlags = PSP_USETITLE;
    psp[2].hInstance = hInst;
    psp[2].pszTemplate = "COP2REGS1";
    psp[2].pszIcon = NULL;
    psp[2].pfnDlgProc = (DLGPROC)COP2VEC1;
    psp[2].pszTitle = "Vectors 1-8";
    psp[2].lParam = 0;

    psp[3].dwSize = sizeof(PROPSHEETPAGE);
    psp[3].dwFlags = PSP_USETITLE;
    psp[3].hInstance = hInst;
    psp[3].pszTemplate = "COP2REGS2";
    psp[3].pszIcon = NULL;
    psp[3].pfnDlgProc = (DLGPROC)COP2VEC2;
    psp[3].pszTitle = "Vectors 9-16";
    psp[3].lParam = 0;

    psp[4].dwSize = sizeof(PROPSHEETPAGE);
    psp[4].dwFlags = PSP_USETITLE;
    psp[4].hInstance = hInst;
    psp[4].pszTemplate = "COP2REGS3";
    psp[4].pszIcon = NULL;
    psp[4].pfnDlgProc = (DLGPROC)COP2VEC3;
    psp[4].pszTitle = "Vectors 17-24";
    psp[4].lParam = 0;

    psp[5].dwSize = sizeof(PROPSHEETPAGE);
    psp[5].dwFlags = PSP_USETITLE;
    psp[5].hInstance = hInst;
    psp[5].pszTemplate = "COP2REGS4";
    psp[5].pszIcon = NULL;
    psp[5].pfnDlgProc = (DLGPROC)COP2VEC4;
    psp[5].pszTitle = "Vectors 25-32";
    psp[5].lParam = 0;

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_MODELESS | PSH_NOAPPLYNOW;
    psh.hwndParent = NULL;
    psh.hInstance = hInst;
    psh.pszIcon = NULL;
    psh.pszCaption = (LPSTR) "Debugger";
    psh.nStartPage = 0;
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;

    hCOP2Vecswnd = (HWND)PropertySheet(&psh);
}

//---------------------------------------------------------------------------------------

void OpenDebugger()
{
    if (!DebuggerActive)
    {
        if (!hRegswnd)
		{
			// Create the debugger dialog box
            hRegswnd = CreateDialog(hInst, "REGS", NULL, (DLGPROC)DEBUGGER);
		}

        CreateCOP2Page();
        SetActiveWindow(hwnd);

        DebuggerActive = TRUE;
		OpCount=0;


    }
}

//---------------------------------------------------------------------------------------

void CloseDebugger()
{
    if (DebuggerActive)
    {
        DestroyWindow(hCodeListwnd);
        DestroyWindow(hRegswnd);
        DestroyWindow(hVIRegwnd);
        DestroyWindow(hCOP2Vecswnd);

        DebuggerActive = FALSE;
		if( fp != NULL )
		{
			fclose(fp);
			LoggingEnabled = FALSE;
		}
    }
}


void SetDebugMenu(void)
{
	CheckMenuItem(hMenu, ID_DEBUG_CONTROLLER,	debug_si_controller?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_DEBUGSPTASK,		debug_sp_task?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_DEBUGSITASK,		debug_si_task?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_DEBUGSPDMA,			debug_sp_dma?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_DEBUGSIDMA,			debug_si_dma?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_DEBUGPIDMA,			debug_pi_dma?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_DEBUGMEMPAK,		debug_si_mempak?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_DEBUGTLB,			debug_tlb?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_DEBUGEEPROM,		debug_si_eeprom?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_DEBUG_SRAM,			debug_sram?MF_CHECKED:MF_UNCHECKED);
}

#endif //#ifdef WINDEBUG_1964
