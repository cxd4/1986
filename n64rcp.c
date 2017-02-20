/*______________________________________________________________________________
 |                                                                              |
 |  1964 - n64rcp.c                                                             |
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

#include "globals.h"
#include "n64rcp.h"
#include "options.h"
#include "timer.h"
#include "hardware.h"

void SP_Reset(void)
{
	//Clear all STATUS Registers
	SP_STATUS_REG = 0;
}

void SI_Reset(void)
{
	SI_STATUS_REG = 0;
}

void PI_Reset(void)
{
	PI_STATUS_REG = 0;
}

void RCP_Reset(void)
{
	SP_Reset();
	SI_Reset();
	PI_Reset();
	//VI_Reset();
	//AI_Reset();
	MI_INTR_REG_R = 0;
}


void Init_VI_Counter(int tv_type)
{
	if( tv_type == 0 )	//PAL
	{
		vi_count_per_line = PAL_VI_COUNT_PER_LINE;
		max_vi_count =		PAL_VI_MAGIC_NUMBER;
		max_vi_lines =		PAL_MAX_VI_LINE;
	}
	else if( tv_type == 1 )	//NTSC
	{
		vi_count_per_line = NTSC_VI_COUNT_PER_LINE;
		max_vi_count =		NTSC_VI_MAGIC_NUMBER;
		max_vi_lines =		NTSC_MAX_VI_LINE;
	}
}

