//  ______________________________________________________________________________
// |                                                                              |
// |  1964 - Emulator for Nintendo 64 console system                              |
// |  Copyright (C) 2001  Joel Middendorf  schibo@emuhq.com                       |
// |                                                                              |
// |  This program is free software; you can redistribute it and/or               |
// |  modify it under the terms of the GNU General Public License                 |
// |  as published by the Free Software Foundation; either version 2              |
// |  of the License, or (at your option) any later version.                      |
// |                                                                              |
// |  This program is distributed in the hope that it will be useful,             |
// |  but WITHOUT ANY WARRANTY; without even the implied warranty of              |
// |  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               |
// |  GNU General Public License for more details.                                |
// |                                                                              |
// |  You should have received a copy of the GNU General Public License           |
// |  along with this program; if not, write to the Free Software                 |
// |  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. |
// |                                                                              |
// |  To contact the author:                                                      |
// |  email      : schibo@emuhq.com                                               |
// |  paper mail :                                                                |
// |______________________________________________________________________________|

// The project is a direct documentation-to-code translation of the n64toolkit 
// by my friend anarko and RSP info has been provided by zilmar. Most source
// code comments are taken directly from anarko's n64toolkit with consent and are 
// the property of anarko.

// 9/15/1999 - GERRIT - rewrote opcode debug printing so that it's no longer
// dependant on opcode execution. Makes things easier for dynarec, etc...

#ifdef DEBUG_COMMON

#ifndef _DBGPRINT__1964_
#define _DBGPRINT__1964_

//---------------------------------------------------------------------------------------

extern char     op_str[0xFF];
extern void RefreshOpList(char *string);

void DebugPrintInstruction(uint32 instruction);
void DebugPrintInstructionWithOutRefresh(uint32 Instruction);
void DebugPrintPC(uint32 thePC);

//---------------------------------------------------------------------------------------
// argument printing macros

#define DBGPRINT_RT_IMM(_op_name)                                           \
        sprintf(op_str, "%08X: %s%s,%04Xh  %s=%08X", gHardwareState.pc, _op_name,                    \
        r4300i_RegNames[RT_FT], OFFSET_IMMEDIATE,		\
		r4300i_RegNames[RT_FT], (uint32)gHardwareState.GPR[RT_FT]);

#define DBGPRINT_RT_FS_COP0(_op_name)                                       \
        sprintf(op_str, "%08X: %s%s,%s  %s=%08X", gHardwareState.pc, _op_name,                   \
        r4300i_RegNames[RT_FT], r4300i_COP0_RegNames[RD_FS],	\
		r4300i_RegNames[RT_FT], (uint32)gHardwareState.GPR[RT_FT]);

#define DBGPRINT_RT_FS_COP1(_op_name)                                       \
        sprintf(op_str, "%08X: %s%s,%s  %08X %08X", gHardwareState.pc, _op_name,                   \
        r4300i_RegNames[RT_FT], r4300i_COP1_RegNames[RD_FS],		\
		(uint32)gHardwareState.GPR[RT_FT], (uint32)gHardwareState.COP1Reg[RD_FS]);

#define DBGPRINT_BASE_RT_OFFSET(_op_name)                                   \
        sprintf(op_str, "%08X: %s%2s,%04Xh(%s)  %08X", gHardwareState.pc, _op_name,               \
        r4300i_RegNames[RT_FT], OFFSET_IMMEDIATE,                           \
        r4300i_RegNames[RS_BASE_FMT], (uint32)gHardwareState.GPR[RT_FT]);

#define DBGPRINT_RS_RT_IMM(_op_name)                                        \
        sprintf(op_str, "%08X: %s%2s,%s,%04Xh  %08X  %08X", gHardwareState.pc, _op_name,                \
        r4300i_RegNames[RT_FT], r4300i_RegNames[RS_BASE_FMT],               \
        (signed)OFFSET_IMMEDIATE, ((uint32)gHardwareState.GPR[RT_FT]), ((uint32)gHardwareState.GPR[RS_BASE_FMT]));

#define DBGPRINT_RS_RT_IMMH(_op_name)                                       \
        sprintf(op_str, "%08X: %s%2s,%s,%04Xh  %08X  %08X", gHardwareState.pc, _op_name,                \
        r4300i_RegNames[RT_FT], r4300i_RegNames[RS_BASE_FMT],               \
        (uint16)OFFSET_IMMEDIATE, ((uint32)gHardwareState.GPR[RT_FT]), ((uint32)gHardwareState.GPR[RS_BASE_FMT]));

#define DBGPRINT_RS_OFF(_op_name)                                           \
        sprintf(op_str, "%08X: %s%s,%04Xh  %s=%08X", gHardwareState.pc, _op_name,                    \
        r4300i_RegNames[RS_BASE_FMT], OFFSET_IMMEDIATE,		\
		r4300i_RegNames[RS_BASE_FMT], (uint32)gHardwareState.GPR[RS_BASE_FMT]);

#define DBGPRINT_RS_IMM(_op_name)                                           \
        sprintf(op_str, "%08X: %s%s,%04Xh  %s=%08X", gHardwareState.pc, (uint32)_op_name,            \
        r4300i_RegNames[RS_BASE_FMT], OFFSET_IMMEDIATE,		\
		r4300i_RegNames[RS_BASE_FMT], (uint32)gHardwareState.GPR[RS_BASE_FMT]);

#define DBGPRINT_RS_RD(_op_name)                                            \
        sprintf(op_str, "%08X: %s%s,%s  %08X  %08X", gHardwareState.pc, (uint32)_op_name,               \
        r4300i_RegNames[RS_BASE_FMT], r4300i_RegNames[RD_FS],		\
		(uint32)gHardwareState.GPR[RS_BASE_FMT], (uint32)gHardwareState.GPR[RD_FS]);


#define DBGPRINT_RS_RT_RD(_op_name)                                         \
        sprintf(op_str, "%08X: %s%2s,%s,%s %08X %08X %08X", gHardwareState.pc, (uint32)_op_name,        \
        r4300i_RegNames[RD_FS], r4300i_RegNames[RS_BASE_FMT],               \
        r4300i_RegNames[RT_FT],			\
		(uint32)gHardwareState.GPR[RD_FS], (uint32)gHardwareState.GPR[RS_BASE_FMT], (uint32)gHardwareState.GPR[RT_FT]);

#define DBGPRINT_RT_RD_SA(_op_name)                                         \
        if ((RT_FT | SA_FD | RD_FS) == 0)                                   \
            sprintf(op_str, "%08X: NOP", gHardwareState.pc);                                \
        else                                                                \
        {                                                                   \
        sprintf(op_str, "%08X: %s%2s,%s,%04Xh", gHardwareState.pc, (uint32)_op_name,        \
        (uint32)r4300i_RegNames[RD_FS], (uint32)r4300i_RegNames[RT_FT], SA_FD);             \
        }

#define DBGPRINT_RS_RT(_op_name)                                            \
        sprintf(op_str, "%08X: %s%s,%s  %08X  %08X", gHardwareState.pc, (uint32)_op_name,               \
        r4300i_RegNames[RS_BASE_FMT], r4300i_RegNames[RT_FT],	\
		(uint32)gHardwareState.GPR[RS_BASE_FMT], (uint32)gHardwareState.GPR[RT_FT]);


#define DBGPRINT_RD(_op_name)                                               \
        sprintf(op_str, "%08X: %s%s  %s=%08X", gHardwareState.pc, (uint32)_op_name,                  \
        r4300i_RegNames[RD_FS],		\
		r4300i_RegNames[RD_FS], (uint32)gHardwareState.GPR[RD_FS]);


#define DBGPRINT_RS(_op_name)                                               \
        sprintf(op_str, "%08X: %s%s  %s=%08X", gHardwareState.pc, (uint32)_op_name,                  \
        r4300i_RegNames[RS_BASE_FMT],		\
		r4300i_RegNames[RS_BASE_FMT], (uint32)gHardwareState.GPR[RS_BASE_FMT]);

#define DBGPRINT_RS_RT_OFF(_op_name)        DBGPRINT_RS(_op_name)

#define DBGPRINT_RS_RT_OFF_BRANCH(_op_name)                                 \
        sprintf(op_str, "%08X: %s%s,%s,%04Xh  %08X  %08X", gHardwareState.pc, (uint32)_op_name,     \
        r4300i_RegNames[RS_BASE_FMT], r4300i_RegNames[RT_FT],               \
        ((OFFSET_IMMEDIATE * 4) + gHardwareState.pc + 4),	\
		(uint32)gHardwareState.GPR[RS_BASE_FMT], (uint32)gHardwareState.GPR[RT_FT]);

#define DBGPRINT_INSTR_INDEX(_op_name)                                      \
        sprintf(op_str, "%08X: %s%08X", gHardwareState.pc, (uint32)_op_name, instr_index);

#define DBGPRINT_RT_RD_RS(_op_name)                                         \
        sprintf(op_str, "%08X: %s%2s,%s,%s  %08X  %08X  %08X", gHardwareState.pc, (uint32)_op_name,        \
        r4300i_RegNames[RD_FS], r4300i_RegNames[RT_FT],                     \
        r4300i_RegNames[RS_BASE_FMT],			\
		(uint32)gHardwareState.GPR[RD_FS], (uint32)gHardwareState.GPR[RT_FT], (uint32)gHardwareState.GPR[RS_BASE_FMT]);


#define DBGPRINT_OPCODE(_op_name)                                           \
        sprintf(op_str, "%08X: %s", gHardwareState.pc, (uint32)_op_name);

#define DBGPRINT_FD_FS(_op_name)                                            \
        sprintf(op_str, "%08X: %s%s,%s", gHardwareState.pc, (uint32)_op_name,               \
        r4300i_COP1_RegNames[SA_FD], r4300i_COP1_RegNames[RD_FS]);

#define DBGPRINT_FD_FS_FT(_op_name)                                         \
        sprintf(op_str, "%08X: %s%s,%s,%s  %08X  %08X  %08X", gHardwareState.pc, (uint32)_op_name,            \
        r4300i_RegNames[SA_FD], r4300i_RegNames[RD_FS],                     \
        r4300i_RegNames[RT_FT],			\
		(uint32)gHardwareState.GPR[SA_FD], (uint32)gHardwareState.GPR[RD_FS], (uint32)gHardwareState.GPR[RT_FT]);


#endif      //_DBGPRINT__1964_
#endif      // DEBUG_COMMON
