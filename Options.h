/*______________________________________________________________________________
 |                                                                              |
 |  1964 - options.h                                                            |
 |  Copyright (C) 2001 Joel Middendorf, <schibo@emulation64.com>                |
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
 |  To contact the author:  Schibo and Rice                                     |
 |  email      : schibo@emulation64.com, dyangchicago@yahoo.com                 |
 |  paper mail :                                                                |
 |______________________________________________________________________________|
*/

#ifndef __OPTIONS_H
#define __OPTIONS_H

/*------------------------------------------------------------------*/
/*                                                                  */
/*   CPU Counter Handling Options                                   */
/*                                                                  */
/*------------------------------------------------------------------*/

//Debugging tools (Disable these for public releases)
//#define ENABLE_DISPLAY_ERROR

#ifdef ENABLE_OPCODE_DEBUGGER
extern unsigned _int8** sDWord_ptr;
extern unsigned _int8** sDWord_ptr2;
extern unsigned _int8** TLB_sDWord_ptr;

#define sDWORD_R    (sDWord_ptr)
#define sDWORD_R_2  (sDWord2_ptr)

#define gHWS_GPR                p_gHardwareState->GPR
#define gHWS_COP0Reg            p_gHardwareState->COP0Reg
#define gHWS_fpr32              p_gHardwareState->fpr32
#define gHWS_COP1Reg            p_gHardwareState->COP1Reg
#define gHWS_RememberFprHi      p_gHardwareState->RememberFprHi
#define gHWS_COP1Con            p_gHardwareState->COP1Con
#define gHWS_COP0Con            p_gHardwareState->COP0Con
#define gHWS_LLbit              p_gHardwareState->LLbit
#define gHWS_pc                 p_gHardwareState->pc
#define gHWS_code               p_gHardwareState->code

#define gMS_ramRegs0            p_gMemoryState->ramRegs0
#define gMS_ramRegs4            p_gMemoryState->ramRegs4
#define gMS_ramRegs8            p_gMemoryState->ramRegs8
#define gMS_SP_MEM              p_gMemoryState->SP_MEM
#define gMS_SP_REG_1            p_gMemoryState->SP_REG_1
#define gMS_SP_REG_2            p_gMemoryState->SP_REG_2
#define gMS_DPC                 p_gMemoryState->DPC
#define gMS_DPS                 p_gMemoryState->DPS
#define gMS_MI                  p_gMemoryState->MI
#define gMS_VI                  p_gMemoryState->VI
#define gMS_AI                  p_gMemoryState->AI
#define gMS_PI                  p_gMemoryState->PI
#define gMS_RI                  p_gMemoryState->RI
#define gMS_SI                  p_gMemoryState->SI
#define gMS_RDRAM               p_gMemoryState->RDRAM
#define gMS_C2A1                p_gMemoryState->C2A1
#define gMS_C1A1                p_gMemoryState->C1A1
#define gMS_C1A3                p_gMemoryState->C1A3
#define gMS_C2A2                p_gMemoryState->C2A2
#define gMS_ROM_Image           p_gMemoryState->ROM_Image
#define gMS_GIO_REG             p_gMemoryState->GIO_REG
#define gMS_PIF                 p_gMemoryState->PIF
#define gMS_ExRDRAM             p_gMemoryState->ExRDRAM
#define gMS_dummySegment        p_gMemoryState->dummySegment
#define gMS_TLB                 p_gMemoryState->TLB

#else

#define sDWORD_R    sDWord
#define sDWORD_R_2  sDWord2

#define gHWS_GPR                gHardwareState.GPR
#define gHWS_COP0Reg            gHardwareState.COP0Reg
#define gHWS_fpr32              gHardwareState.fpr32
#define gHWS_COP1Reg            gHardwareState.COP1Reg
#define gHWS_RememberFprHi      gHardwareState.RememberFprHi
#define gHWS_COP1Con            gHardwareState.COP1Con
#define gHWS_COP0Con            gHardwareState.COP0Con
#define gHWS_LLbit              gHardwareState.LLbit
#define gHWS_pc                 gHardwareState.pc
#define gHWS_code               gHardwareState.code

#define gMS_ramRegs0            gMemoryState.ramRegs0
#define gMS_ramRegs4            gMemoryState.ramRegs4
#define gMS_ramRegs8            gMemoryState.ramRegs8
#define gMS_SP_MEM              gMemoryState.SP_MEM
#define gMS_SP_REG_1            gMemoryState.SP_REG_1
#define gMS_SP_REG_2            gMemoryState.SP_REG_2
#define gMS_DPC                 gMemoryState.DPC
#define gMS_DPS                 gMemoryState.DPS
#define gMS_MI                  gMemoryState.MI
#define gMS_VI                  gMemoryState.VI
#define gMS_AI                  gMemoryState.AI
#define gMS_PI                  gMemoryState.PI
#define gMS_RI                  gMemoryState.RI
#define gMS_SI                  gMemoryState.SI
#define gMS_RDRAM               gMemoryState.RDRAM
#define gMS_C2A1                gMemoryState.C2A1
#define gMS_C1A1                gMemoryState.C1A1
#define gMS_C1A3                gMemoryState.C1A3
#define gMS_C2A2                gMemoryState.C2A2
#define gMS_ROM_Image           gMemoryState.ROM_Image
#define gMS_GIO_REG             gMemoryState.GIO_REG
#define gMS_PIF                 gMemoryState.PIF
#define gMS_ExRDRAM             gMemoryState.ExRDRAM
#define gMS_dummySegment        gMemoryState.dummySegment
#define gMS_TLB                 gMemoryState.TLB

#endif


#ifdef ENABLE_OPCODE_DEBUGGER
void COMPARE_SwitchToInterpretive();
void COMPARE_SwitchToDynarec();
#define OPCODE_DEBUGGER_BEGIN_EPILOGUE  \
{                                       \
    int k;                              \
    for (k=0; k<=1; k++)                 \
    {                                   \
        if( k==0 && debug_opcode != 0) COMPARE_SwitchToInterpretive();	\
        if( k==1 ) COMPARE_SwitchToDynarec();      
#define OPCODE_DEBUGGER_END_EPILOGUE    \
		if( debug_opcode == 0 ) break;	\
    }                                   \
}
#else
#define OPCODE_DEBUGGER_BEGIN_EPILOGUE
#define OPCODE_DEBUGGER_END_EPILOGUE
#endif



//This option is to mark SP busy, and delay SP task for a moment of time
#define DOSPTASKCOUNTER

//This option is to mark SI IO Busy, and delay SI IO task for a moment of time
#define DOSIIOTASKCOUNTER

// This option will precisely emulate CPU PCLOCK counter for all the multi-pclock opcode
// Like integer MUL, DIV, TLBP and so on
#define SAVEOPCOUNTER


/*------------------------------------------------------------------*/
/*                                                                  */
/*   CPU Core  Options                                              */
/*                                                                  */
/*------------------------------------------------------------------*/

// Check and make sure register R0=0
#define CPU_CORE_CHECK_R0

#ifdef DEBUG_COMMON
// Check address alignment for store/load instructions
//#define ADDR_ALIGN_CHECKING
#endif

// If this option is enabled, CPU core will generate ADDRESS ERROR exception when LOAD
// or STORE instructions access unmapped memory range. Otherwise CPU core will not generate
// such exceptions
#ifdef DEBUG_COMMON
#define ADDRESS_ERROR_EXCEPTION
#endif

// If this option is enabled, CPU will continue to check other TLB entries even
// an invalid entry has been matched.
#define ADVANCED_TLB

// This option will enabl the direct TLB lookup, address lookup will be
// carried on directly via TLB_Lookup_Table array
// The TLB_Lookup_Table array should be eventually marge with S_DWORD lookup table
#define DIRECT_TLB_LOOKUP

// If enabled, CPU core will check overflow and divided_by_zero exceptions for integer opcodes
#define DO_INTEGER_MATH_EXCEPTION

// If enabled, CPU core will check math exceptions for FPU
#define DO_FPU_MATH_EXCEPTION
/*------------------------------------------------------------------*/
/*                                                                  */
/*   Interrupt Handling Options                                     */
/*                                                                  */
/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
/*                                                                  */
/*   Self-Modified Code Detection Options                           */
/*                                                                  */
/*------------------------------------------------------------------*/

// This option allows ebp for local stack variables when mapping and flushing registers.
#define USE_STACK

/*------------------------------------------------------------------*/
/*                                                                  */
/*   Memory Access and Unmapped Memory Handling Options             */
/*                                                                  */
/*------------------------------------------------------------------*/

// Dynacompiler need mapped RDRAM and ROM memory to save compiling info, uses a lot of memory
// This option saves memory usage, will dynamatically allocate and map only memory ranges that
// instructions are executed
#define DYNAEXECUTIONMAPMEMORY

// Without this option turned on, 1964 will not load ROM into memory, but mapped rom memory directly
// to the file on disk, this will dramtically save memory usage, will also speed up game boot up
// time. Of course, draw back is the game executing will be slower because operating system will 
// speed time to do the memory-file mapping/caching
// This option is not implemented yet
// #define MAPROMMEMORYWITHOUTLOADROM

// This option enables to use dummy segment for any unmapped memory access
#define USE_DUMMY_SEGMENT


/*------------------------------------------------------------------*/
/*                                                                  */
/*   Optimizations                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
// Will set the next block PC/Block at compiling time, to save execution time
#define DYNA_GET_BLOCK_SPEEDUP

// Will use a faster way to do VI/COMPARE interrupt checking
#define FAST_COUNTER

#ifdef DIRECT_TLB_LOOKUP
#ifdef ENABLE_OPCODE_DEBUGGER
#define TLB_sDWORD_R    (TLB_sDWord_ptr)
#else
#define TLB_sDWORD_R    TLB_sDWord
#endif
#endif

#endif