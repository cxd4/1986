/*______________________________________________________________________________
 |                                                                              |
 |  1964 - debug_option.h                                                       |
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
 |  To contact the author:                                                      |
 |  email      : dyangchicago@yahoo.com, schibo@emulation64.com                 |
 |  paper mail :                                                                |
 |______________________________________________________________________________|
*/


#ifndef __1964_DEBUG_OPTION
#define __1964_DEBUG_OPTION

#ifdef _DEBUG

#define DEBUG_RANGE_ERROR
#define DEBUG_IO_READ
#define DEBUG_IO_WRITE

#define DEBUG_DYNA_COMPARE_INTERRUPT
#define DEBUG_COMPARE_INTERRUPT
#define DEBUG_CPU_COUNTER

#define DEBUG_SI_DMA
#define DEBUG_SP_DMA
#define DEBUG_PI_DMA
#define DEBUG_SP_TASK


// This option to control if debugger will display debug information for si mempak operation
#define DEBUG_SI_MEMPAK
#ifdef  DEBUG_SI_MEMPAK
//#define DEBUG_DUMP_MEMPAK
#endif

// This option will dump SRAM operation messages
#define DEBUG_SRAM

// This option to control if debugger will display debug information for si eeprom operation
#define DEBUG_SI_EEPROM


// This option to control if debugger will display debug information for si controller operation
#define DEBUG_SI_CONTROLLER

// This option to control if debugger will display debug information for TLB operation
#define DEBUG_TLB

#ifdef  DEBUG_TLB
#define DEBUG_TLB_DETAIL
#endif


#define DEBUG_DYNA_SPMEM

// This option to control if debugger will display debug information for TRAP operation
#define DEBUG_TRAP


// This option to control if debugger will display debug information for IO memory operation
#define DEBUG_IO
#ifdef  DEBUG_IO
#define DEBUG_IO_VI
#define DEBUG_IO_SP
#define DEBUG_IO_PI
#define DEBUG_IO_AI
#define DEBUG_IO_MI
#define DEBUG_IO_SI
#define DEBUG_IO_RI
#define DEBUG_IO_DP
#define DEBUG_IO_DPS
#define DEBUG_IO_RDRAM
#endif

#define DEBUG_DYNA
#define DEBUG_DYNAEXECUTION

extern	int debug_io;
extern	int debug_io_vi;
extern	int debug_io_sp;
extern	int debug_io_pi;
extern	int debug_io_ai;
extern	int	debug_io_mi;
extern	int debug_io_si;
extern	int debug_io_ri;
extern	int debug_io_dp;
extern	int debug_io_dps;
extern	int debug_io_rdram;
extern	int debug_audio;
extern	int debug_trap;
extern	int debug_compare_interrupt;
extern	int debug_cpu_counter;
extern	int debug_vi_interrupt;
extern	int debug_ai_interrupt;
extern	int	debug_si_interrupt;
extern	int	debug_pi_interrupt;
extern	int debug_interrupt;
extern	int debug_sp_task;
extern	int debug_si_task;
extern	int debug_sp_dma;
extern	int	debug_si_dma;
extern	int debug_pi_dma;
extern	int debug_si_mempak;
extern	int debug_si_controller;
extern	int debug_dump_mempak;
extern	int debug_si_eeprom;
extern	int debug_tlb;
extern	int debug_tlb_detail;
extern	int debug_sram;
extern	int debug_dyna_compiler;
extern	int debug_dyna_execution;
extern	int debug_dyna_log;
extern	int debug_64bit_fpu;

// This option to control if debugger will display debug information for audio operation
#define DEBUG_AUDIO
#endif

#ifdef DEBUG_SI_DMA
#define DEBUG_SI_DMA_TRACE0(str)		if( debug_si_dma ) {TRACE0(str);}
#else
#define DEBUG_SI_DMA_TRACE0
#endif

#ifdef DEBUG_PI_DMA
#define DEBUG_PI_DMA_TRACE0(str)		if( debug_pi_dma ) {TRACE0(str);}
#else
#define DEBUG_PI_DMA_TRACE0
#endif

#ifdef DEBUG_COMMON
#define DEBUG_INTERRUPT_TRACE(othermacro)		if( debug_interrupt ) { othermacro };
#define DEBUG_AI_INTERRUPT_TRACE(othermacro)	if( debug_interrupt && debug_ai_interrupt ) { othermacro };
#define DEBUG_VI_INTERRUPT_TRACE(othermacro)	if( debug_interrupt && debug_vi_interrupt ) { othermacro };
#define DEBUG_PI_INTERRUPT_TRACE(othermacro)	if( debug_interrupt && debug_pi_interrupt ) { othermacro };
#define DEBUG_SI_INTERRUPT_TRACE(othermacro)	if( debug_interrupt && debug_si_interrupt ) { othermacro };
#define DEBUG_COMPARE_INTERRUPT_TRACE(othermacro)	if( debug_interrupt && debug_compare_interrupt ) { othermacro };
#define DEBUG_CPU_COUNTER_TRACE(othermacro)		if( debug_cpu_counter ) { othermacro };
#else
#define DEBUG_INTERRUPT_TRACE(othermacro)
#define DEBUG_AI_INTERRUPT_TRACE(othermacro)
#define DEBUG_VI_INTERRUPT_TRACE(othermacro)
#define DEBUG_PI_INTERRUPT_TRACE(othermacro)
#define DEBUG_SI_INTERRUPT_TRACE(othermacro)
#define DEBUG_COMPARE_INTERRUPT_TRACE(othermacro)
#define DEBUG_CPU_COUNTER_TRACE(othermacro)
#endif

extern  int debug_opcode;
extern  int debug_opcode_block;
extern  int debug_dirty_only;


#endif	//__1964_DEBUG_OPTION