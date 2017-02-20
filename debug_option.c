/*______________________________________________________________________________
 |                                                                              |
 |  1964 - debug_option.c                                                       |
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
#include "debug_option.h"

#ifdef _DEBUG
int debug_io=0;
int debug_io_vi=0;
int debug_io_sp=0;
int debug_io_pi=0;
int debug_io_ai=0;
int	debug_io_mi=0;
int debug_io_si=0;
int debug_io_ri=0;
int debug_io_dp=0;
int debug_io_dps=0;
int debug_io_rdram=0;
int debug_audio=0;
int debug_trap=1;
int debug_si_controller=1;
int debug_compare_interrupt=0;
int debug_cpu_counter=0;
int debug_sp_task=1;
int debug_si_task=0;
int debug_sp_dma=0;
int	debug_si_dma=0;
int debug_pi_dma=1;
int debug_si_mempak=1;
int debug_dump_mempak=0;
int debug_tlb=1;
int debug_tlb_detail=0;
int debug_si_eeprom=1;
int debug_vi_interrupt=0;
int debug_ai_interrupt=0;
int	debug_si_interrupt=0;
int	debug_pi_interrupt=0;
int debug_interrupt=0;
int debug_sram=1;
int debug_dyna_compiler=0;
int debug_dyna_execution=0;
int debug_dyna_log=0;
int debug_64bit_fpu=0;

#endif

int debug_opcode=1;
int debug_opcode_block=0;
int debug_dirty_only = 1;


