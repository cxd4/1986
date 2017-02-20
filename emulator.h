#ifndef __1964_EMULATIONOR_H

#define __1964_EMULATIONOR_H

#include <windows.h>

extern DWORD	CPUThreadID;
extern HANDLE	CPUThreadHandle;
extern uint32	sp_hle_task;
extern int		sp_task_counter;
extern int		si_io_counter;


void RunEmulator(unsigned _int32 WhichCore);
void ClearCPUTasks(void);
void StepCPU(void);
uint32 FetchInstruction(void);

#endif //__1964_EMULATIONOR_H