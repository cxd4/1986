#ifndef __DYNAREC_H
#define __DYNAREC_H

#include "hardware.h"

extern _u8 *dyna_CodeTable;

extern BOOL Init_Dynarec();
extern void Free_Dynarec();
extern void Start_Dynarec();

#endif