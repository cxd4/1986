#ifndef __FLASHRAM_H
/*______________________________________________________________________________
 |                                                                              |
 |  1964 - flashram.h                                                             |
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

#define __FLASHRAM_H

#define FLASHRAM_STATUS_REG_WORD1_ADDR	0xA8000000
#define FLASHRAM_STATUS_REG_WORD2_ADDR	0xA8000004
#define FLASHRAM_COMMAND_REG_ADDR	0xA8010000

#define FLASHRAM_STATUS_REG_1	(LOAD_UWORD_PARAM_2(FLASHRAM_STATUS_REG_WORD1_ADDR))
#define FLASHRAM_STATUS_REG_2	(LOAD_UWORD_PARAM_2(FLASHRAM_STATUS_REG_WORD2_ADDR))

void Flashram_Command(unsigned __int32 val);
unsigned __int32 Flashram_Get_Status(void);

// Trigger from r4300i opcode LW and SW
#define CHECK_FLASHRAM_SW(addr,val)	if( addr == FLASHRAM_COMMAND_REG_ADDR ) Flashram_Command(val);
#define CHECK_FLASHRAM_LW(addr)	if( addr == FLASHRAM_STATUS_REG_WORD1_ADDR ) {return(Flashram_Get_Status()); }
/*
	if( addr == FLASHRAM_STATUS_REG_WORD1_ADDR || addr == FLASHRAM_STATUS_REG_WORD2_ADDR)	\
	{													\
		if( addr == FLASHRAM_STATUS_REG_ADDR_WORD2 ) DisplayError("Reading flashram statue word 2");	\
		reg = Flashram_Get_Status();					\
	}
	*/

void DMA_Flashram_To_RDRAM(unsigned __int32 rdramaddr, unsigned __int32 flashramaddr, unsigned __int32 len);
void DMA_RDRAM_To_Flashram(unsigned __int32 rdramaddr, unsigned __int32 flashramaddr, unsigned __int32 len);

void Flashram_Init(void);
#endif