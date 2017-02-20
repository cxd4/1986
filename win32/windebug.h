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

#ifdef WINDEBUG_1964

#ifndef _WINDEBUG_H__1964_
#define _WINDEBUG_H__1964_

#ifndef DEBUG_COMMON
#define DEBUG_COMMON
#endif


#include <windows.h>

extern void RefreshOpList(char *opcode);

HWND hRegswnd;                                  /* handle to GPR/FPU Regs window    */
HWND hVIRegwnd;                                 /* handle to VI Regs window         */
HWND hCodeListwnd;                              /* handle to CodeList window        */
HWND hEnterHexwnd;                              /* handle to "Enter Hex" window     */
HWND hCOP2Vecswnd;                              /* handle to COP2 vectors window    */
HWND hMemorywnd;                                /* handle to Memory window          */

//Control Handles
HWND GPREDIT[32];                               //Cop0 Register editbox handles
HWND COP0EDIT[32];                              //Cop0 Register editbox handles
HWND COP1EDIT[32];                              //Cop1 Register editbox handles
HWND MISCEDIT[4];                               //PC, LLbit,MultHI/LO
HWND VIREGEDIT[4];
HWND CODEEDIT[1];                               /* editbox which holds info on all decoded opcodes */
HWND COP2VECEDIT[32][8];                        /* editboxes which hold all COP2 vec data          */
HWND MEMLISTBOX;                                //Listbox to display memory
HWND MEMLOCEDIT;                                //Editbox to choose mem location

char opBuffer[65535];                           /* buffer to all the decoded opcode text */
unsigned _int32 BreakAddress;                            /* User-defined breakpoint */


//BOOL  RUN_TO_ADDRESS_ACTIVE = FALSE; // if this flag is true, check for pc <= RunToTargetAddress
//uint32    RunToTargetAddress;    // the user-specified target break address
BOOL    RUN_TO_ADDRESS_ACTIVE; // if this flag is true, check for pc <= RunToTargetAddress

BOOL    DebuggerActive;                 //is the debugger on?
unsigned __int32 NextClearCode;
int OpCount;

extern void UpdateGPR();
extern void UpdateCOP0();
extern void UpdateFPR();
extern void UpdateMisc();
extern void RefreshDebugger();

extern void OpenDebugger();
extern void CloseDebugger();

extern void Get_HexInput();
extern unsigned _int32 StrToHex(char* HexStr);

extern void UpdateVIReg();
extern void UpdateCOP2Vec1();
extern void UpdateCOP2Vec2();
extern void UpdateCOP2Vec3();
extern void UpdateCOP2Vec4();

extern void WinDynDebugPrintInstruction(unsigned __int32 Instruction);
extern void HandleBreakpoint(unsigned __int32 Instruction);

#endif// _WINDEBUG_H__1964_

#endif// WINDEBUG_1964