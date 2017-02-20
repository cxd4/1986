/*______________________________________________________________________________
 |                                                                              |
 |  1964 - timer.h                                                              |
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


#ifndef __1964_TIMER_H
#define __1964_TIMER_H
#include "options.h"
#include "windows.h"


#define SPCOUNTERTOINCREASE	10				// Number of PCLOCK to execute each instruction in r4300i
#define SPTASKPCLOCKS		260				// Number of PCLOCK to execute each SP Task

#define PI_DMA_SEGMENT		0x10			// Bytes to transfer per CPU instruction cycle
#define SP_DMA_SEGMENT		0x10			// Bytes to transfer per CPU instruction cycle
#define SI_DMA_SEGMENT		0x02			// Bytes to transfer per CPU instruction cycle
											// SI DMA Segment must be less than 4, otherwise Cruise USA will not work


BOOL	CPUNeedToDoOtherTask;	// This parameter is set when CPU need to do tasks
								// such as DMA memory transfer, SP task timer count down
								//S SI DMA transfer count down

BOOL	CPUNeedToCheckException;	// This global parameter is set when any exceptions is fired
BOOL	CPUNeedToCheckInterrupt;	// This global parameter is set when any interrupt is fired
									// and the CPU is waiting for a COMPARE interrupts

int		viframeskip;				// This global parameter determines to skip a VI frame
									// after every few frames
int		viframeskipcount;
int		framecounter;				// To count how many frames are displayed per second
float	vips;						// VI/s
extern	LARGE_INTEGER Freq;
extern	LARGE_INTEGER LastVITime;
extern	LARGE_INTEGER LastSecondTime;

void	CPUDoOtherTasks(void);

#define VI_COUNTER_INC_PER_LINE		vi_count_per_line

#define NTSC_MAX_VI_LINE		0x20D
#define NTSC_VI_COUNT_PER_LINE	1190
#define PAL_MAX_VI_LINE			625
#define PAL_VI_COUNT_PER_LINE	1200

extern unsigned __int32 max_vi_lines;
extern unsigned __int32 max_vi_count;
extern unsigned __int32 vi_count_per_line;

#define NTSC_VI_MAGIC_NUMBER	625000
#define PAL_VI_MAGIC_NUMBER		777809 //750000	//777809

#define COUNTER_RATE	CounterFactors[CounterFactor]/VICounterFactors[CounterFactor]
//#define MAGICNUMBER (625000)
#define MAGICNUMBER max_vi_count
//#define MAGICNUMBERFORCOUNTREG (312500)
#define MAGICNUMBERFORCOUNTREG (max_vi_count*CounterFactors[CounterFactor]/VICounterFactors[CounterFactor])


void Init_VI_Counter(int tv_type);

#endif	//__1964_TIMER_H