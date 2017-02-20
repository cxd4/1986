#ifndef __1964_TIMER_H
#define __1964_TIMER_H

#define SPCOUNTERTOINCREASE	10				// Number of PCLOCK to execute each instruction in r4300i
#define SPTASKPCLOCKS		260				// Number of PCLOCK to execute each SP Task

#define PI_DMA_SEGMENT		40				// Bytes to transfer per CPU instruction cycle
#define SP_DMA_SEGMENT		20				// Bytes to transfer per CPU instruction cycle
#define SI_DMA_SEGMENT		4				// Bytes to transfer per CPU instruction cycle


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

void	CPUDoOtherTasks(void);

#define VI_COUNTER_INC_PER_LINE		1220
#define DEFAULT_MAX_VI_LINE			512
extern int max_vi_lines;

#define VIHACKMAGICNUMBER	256

#endif	//__1964_TIMER_H