/*
  ______________________________________________________________________________
 |                                                                              |
 |  1964 - Emulator for Nintendo 64 console system                              |
 |  Copyright (C) 2001  Joel Middendorf  schibo@emulation64.com                 |
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
 |  email      : schibo@emulation64.com                                         |
 |  paper mail :                                                                |
 |______________________________________________________________________________|

*/

#include <windows.h>
#include "options.h"
#include "globals.h"
#include "r4300i.h"
#include "hardware.h"
#include "memory.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include "interrupt.h"
#include "win32/windebug.h"
#include "emulator.h"

#define COP1_CONDITION_BIT 0x00800000
#define NOT_COP1_CONDITION_BIT  0xFF7FFFFF

//#define CHK_64BIT_FPU
#ifndef ENABLE_64BIT_FPU
#define CHK_64BITMODE(Name) if (gHWS_COP0Reg[STATUS] & 0x04000000) { MessageBox(NULL, "64bit mode FPU detected!", Name, 0); }
#else
#define CHK_64BITMODE(Name)
#endif


#define SAVE_C_COND_FMT
extern r4300i_speedhack();
#ifdef DEBUG_COMMON
void DebugPrintPC(uint32 thePC);
#endif

#ifdef ENABLE_64BIT_FPU
__forceinline void write_64bit_fpu_reg(int regno, uint64 val)
{
	gHardwareState.fpr32[regno] = (uint32)(val & 0x00000000FFFFFFFF);
	gHardwareState.fpr32[regno+FR_reg_offset] = (uint32)(val >> 32);
}

__forceinline uint64 read_64bit_fpu_reg(int regno)
{
	uint64 tempval;
	*(((uint32*)&tempval)+1) = gHardwareState.fpr32[regno+FR_reg_offset];
	*(uint32*)&tempval = gHardwareState.fpr32[regno];
	return tempval;
}
#endif

#define ENABLE_CHECK_FPU_USABILITY
#ifdef ENABLE_CHECK_FPU_USABILITY
#define CheckFPU_Usablity(addr)	if( (gHWS_COP0Reg[STATUS] & 0x20000000) == 0 ) TriggerFPUUnusableException();
#else ENABLE_CHECK_FPU_USABILITY
#define CheckFPU_Usablity(addr)
#endif ENABLE_CHECK_FPU_USABILITY

#ifdef ENABLE_OPCODE_DEBUGGER
#define OPCODE_DEBUGGER_BRANCH_SKIP		CPUdelay=0;
#else
#define OPCODE_DEBUGGER_BRANCH_SKIP
#endif

void r4300i_COP1_add_s(uint32 Instruction) {	(*((float*)&cFD)) = (*((float  *)&cFS)) + (*((float*)&cFT)); 	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);}
void r4300i_COP1_sub_s(uint32 Instruction) {    (*((float*)&cFD)) = (*((float  *)&cFS)) - (*((float*)&cFT));	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);}
void r4300i_COP1_mul_s(uint32 Instruction) {    (*((float*)&cFD)) = (*((float  *)&cFS)) * (*((float*)&cFT));	SAVE_OP_COUNTER_INCREASE_INTERPRETER(4);}
void r4300i_COP1_div_s(uint32 Instruction) {    (*((float*)&cFD)) = (*((float  *)&cFS)) / (*((float*)&cFT));	SAVE_OP_COUNTER_INCREASE_INTERPRETER(12);}

#ifdef ENABLE_64BIT_FPU
void r4300i_COP1_add_d(uint32 Instruction) 
{
	uint64 u1 = read_64bit_fpu_reg(RD_FS);
	uint64 u2 = read_64bit_fpu_reg(RT_FT);
	double val3 = (*(double*)&u1) + (*(double*)&u2);
	write_64bit_fpu_reg(SA_FD, *((uint64*)(&val3)));
}

void r4300i_COP1_sub_d(uint32 Instruction) 
{    
	uint64 u1 = read_64bit_fpu_reg(RD_FS);
	uint64 u2 = read_64bit_fpu_reg(RT_FT);
	double val3 = (*(double*)&u1) - (*(double*)&u2);
	write_64bit_fpu_reg(SA_FD, *((uint64*)(&val3)));
}

void r4300i_COP1_mul_d(uint32 Instruction) 
{    
	uint64 u1 = read_64bit_fpu_reg(RD_FS);
	uint64 u2 = read_64bit_fpu_reg(RT_FT);
	double val3 = (*(double*)&u1) * (*(double*)&u2);
	write_64bit_fpu_reg(SA_FD, *((uint64*)(&val3)));
}

void r4300i_COP1_div_d(uint32 Instruction) 
{    
	uint64 u1 = read_64bit_fpu_reg(RD_FS);
	uint64 u2 = read_64bit_fpu_reg(RT_FT);
	if( *(double*)&u2 != 0 )
	{
		double val3 = (*(double*)&u1) / (*(double*)&u2);
		write_64bit_fpu_reg(SA_FD, *((uint64*)(&val3)));
	}
}

#else
void r4300i_COP1_add_d(uint32 Instruction) {    CHK_64BITMODE("add_d") *((double *)&cFD) =  *((double *)&cFS)  + *((double *)&cFT); 	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);}
void r4300i_COP1_sub_d(uint32 Instruction) {    CHK_64BITMODE("sub_d") *((double *)&cFD) =  *((double *)&cFS)  - *((double *)&cFT); 	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);}
void r4300i_COP1_mul_d(uint32 Instruction) {    CHK_64BITMODE("mul_d") *((double *)&cFD) =  *((double *)&cFS)  * *((double *)&cFT); 	SAVE_OP_COUNTER_INCREASE_INTERPRETER(4);}
void r4300i_COP1_div_d(uint32 Instruction) {    CHK_64BITMODE("div_d") *((double *)&cFD) =  *((double *)&cFS)  / *((double *)&cFT); 	SAVE_OP_COUNTER_INCREASE_INTERPRETER(18);}
#endif

void r4300i_COP1_abs_s(uint32 Instruction) {    *((float  *)&cFD) = (float)fabs((double)*((float  *)&cFS)); 	SAVE_OP_COUNTER_INCREASE_INTERPRETER(27);}
void r4300i_COP1_sqrt_s(uint32 Instruction){    *((float  *)&cFD) = (float)sqrt((double)*((float  *)&cFS)); 	SAVE_OP_COUNTER_INCREASE_INTERPRETER(1);}
void r4300i_COP1_neg_s(uint32 Instruction) {    *((float  *)&cFD) = (float)   -(        *((float  *)&cFS)); 	SAVE_OP_COUNTER_INCREASE_INTERPRETER(1);}

//---------------------------------------------------------------------------------------


#ifdef ENABLE_64BIT_FPU
void r4300i_COP1_abs_d(uint32 Instruction) 
{    
	uint64 u1 = read_64bit_fpu_reg(RD_FS);
	double val3 = fabs(*(double*)&u1);
	write_64bit_fpu_reg(SA_FD, *((uint64*)(&val3)));
}
void r4300i_COP1_sqrt_d(uint32 Instruction)
{
	uint64 u1 = read_64bit_fpu_reg(RD_FS);
	double val3 = sqrt(*(double*)&u1);
	write_64bit_fpu_reg(SA_FD, *((uint64*)(&val3)));
}
void r4300i_COP1_neg_d(uint32 Instruction) 
{    
	uint64 u1 = read_64bit_fpu_reg(RD_FS);
	double val1 = 0 - *((double*)&u1);
	write_64bit_fpu_reg(SA_FD, *((uint64*)(&val1)));
}
#else
void r4300i_COP1_abs_d(uint32 Instruction) {    CHK_64BITMODE("abs_d")  *((double *)&cFD) =        fabs(        *((double *)&cFS));  	SAVE_OP_COUNTER_INCREASE_INTERPRETER(56);}
void r4300i_COP1_sqrt_d(uint32 Instruction){    CHK_64BITMODE("sqrt_d") *((double *)&cFD) =        sqrt(        *((double *)&cFS));  	SAVE_OP_COUNTER_INCREASE_INTERPRETER(1);}
void r4300i_COP1_neg_d(uint32 Instruction) {    CHK_64BITMODE("neg_d")  *((double *)&cFD) =           -(        *((double *)&cFS));  	SAVE_OP_COUNTER_INCREASE_INTERPRETER(1);}
#endif

//---------------------------------------------------------------------------------------

void r4300i_COP1_bc1f(uint32 Instruction) 
{ 
	CHK_64BITMODE("bc1f")  
	if((((uint32)cCON31 & 0x00800000)) == 0) 
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
	}
             
}

void r4300i_COP1_bc1t(uint32 Instruction) 
{ 
	CHK_64BITMODE("bc1t")  
	if((((uint32)cCON31 & 0x00800000)) != 0) 
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
	}
}

void r4300i_COP1_bc1fl(uint32 Instruction)
{ 
	CHK_64BITMODE("bc1fl") 
	if((((uint32)cCON31 & 0x00800000)) == 0) 
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else    
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
		DELAY_SKIP  
}
}

void r4300i_COP1_bc1tl(uint32 Instruction)
{ 
	CHK_64BITMODE("bc1tl") 
	if((((uint32)cCON31 & 0x00800000)) != 0) 
	{
		R4300I_SPEEDHACK
		DELAY_SET   
	}
	else    
	{
		OPCODE_DEBUGGER_BRANCH_SKIP
		DELAY_SKIP  
}
}

//---------------------------------------------------------------------------------------
void r4300i_C_cond_fmt_s(uint32 Instruction)
{
	float fcFS32, fcFT32;
	BOOL less, equal, unordered, cond, cond0, cond1, cond2, cond3;
	cond0 = (Instruction   ) & 0x1;
	cond1 = (Instruction>>1) & 0x1;
	cond2 = (Instruction>>2) & 0x1;
	cond3 = (Instruction>>3) & 0x1;
	fcFS32 = *((float  *)&cFS);
	fcFT32 = *((float  *)&cFT);
	if (_isnan(fcFS32) || _isnan(fcFT32))
	{
		less = FALSE;
		equal = FALSE;
		unordered = TRUE;
		if (cond3)
		{
			// Fire invalid operation exception
			return;
		}
	}
	else
	{
		less  = (fcFS32 < fcFT32);
		equal = (fcFS32 == fcFT32);
		unordered = FALSE;	
	}
	cond = ((cond0 && unordered) || (cond1 && equal) || (cond2 && less));
	if ( cond )
		cCON31 |= 0x00800000;
	else
		cCON31 &= 0xFF7FFFFF;
}

void r4300i_C_cond_fmt_d(uint32 Instruction)
{
	double fcFS32, fcFT32;

	BOOL less, equal, unordered, cond, cond0, cond1, cond2, cond3;
#ifdef ENABLE_64BIT_FPU
	uint64 val1, val2;
#endif

	cond0 = (Instruction   ) & 0x1;
	cond1 = (Instruction>>1) & 0x1;
	cond2 = (Instruction>>2) & 0x1;
	cond3 = (Instruction>>3) & 0x1;
#ifdef ENABLE_64BIT_FPU
	val1 = read_64bit_fpu_reg(RD_FS);
	val2 = read_64bit_fpu_reg(RT_FT);
	fcFS32 = *((double  *)&val1);
	fcFT32 = *((double  *)&val2);
#else
	fcFS32 = *((double  *)&cFS);
	fcFT32 = *((double  *)&cFT);
#endif

	if (_isnan(fcFS32) || _isnan(fcFT32))
	{
		less = FALSE;
		equal = FALSE;
		unordered = TRUE;

		if (cond3)
		{
			// Fire invalid operation exception
			return;
		}
	}
	else
	{
		less  = (fcFS32 < fcFT32);
		equal = (fcFS32 == fcFT32);
		unordered = FALSE;	
	}

	cond = ((cond0 && unordered) || (cond1 && equal) || (cond2 && less));

	if ( cond )
		cCON31 |= 0x00800000;
	else
		cCON31 &= 0xFF7FFFFF;
}

void r4300i_C_EQ_S(uint32 Instruction) 
{
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_s(Instruction);
#else
	if( *((float  *)&cFS) == *((float  *)&cFT)) 
		cCON31 |=  0x00800000;
	else                                                                    
		cCON31 &=  0xFF7FFFFF;
#endif
}

void r4300i_C_UEQ_S(uint32 Instruction)
{    
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_s(Instruction);
#else
	if( *((float  *)&cFS) != *((float  *)&cFT))        
		cCON31 |=  0x00800000;
	else                                                                    
		cCON31 &=  0xFF7FFFFF;
#endif
}
void r4300i_C_EQ_D(uint32 Instruction) 
{    
#ifdef SAVE_C_COND_FMT
	r4300i_C_cond_fmt_d(Instruction);
#else

#ifdef ENABLE_64BIT_FPU
	uint64 u1 = read_64bit_fpu_reg(RD_FS);
	uint64 u2 = read_64bit_fpu_reg(RT_FT);
	if( *(double*)(&u1) == *(double*)(&u2) )        
#else
	CHK_64BITMODE("C_EQ_D")  
	if( *((double *)&cFS) == *((double *)&cFT))     
#endif
		cCON31 |=  0x00800000;
	else                                                                    
		cCON31 &=  0xFF7FFFFF;
#endif
}

void r4300i_C_UEQ_D(uint32 Instruction)
{    
#ifdef SAVE_C_COND_FMT
	r4300i_C_cond_fmt_d(Instruction);
#else

#ifdef ENABLE_64BIT_FPU
	uint64 u1 = read_64bit_fpu_reg(RD_FS);
	uint64 u2 = read_64bit_fpu_reg(RT_FT);
	if( *(double*)(&u1) != *(double*)(&u2) )        
#else
	CHK_64BITMODE("C_UEQ_D") 
	if( *((double *)&cFS) != *((double *)&cFT))        
#endif
		cCON31 |=  0x00800000;
	else                                                                    
		cCON31 &=  0xFF7FFFFF;
#endif
}

void r4300i_C_LT_S(uint32 Instruction) 
{    
#ifdef SAVE_C_COND_FMT
	r4300i_C_cond_fmt_s(Instruction);
#else
	if( *((float  *)&cFS) <  *((float  *)&cFT))      
		cCON31 |=  0x00800000;
	else                                                                    
		cCON31 &=  0xFF7FFFFF;
#endif
}

void r4300i_C_NGE_S(uint32 Instruction)
{    
#ifdef SAVE_C_COND_FMT
	r4300i_C_cond_fmt_s(Instruction);
#else
	if( *((float  *)&cFS) <  *((float  *)&cFT))        
		cCON31 |=  0x00800000;
	else                                                                    
		cCON31 &=  0xFF7FFFFF;
#endif
}

void r4300i_C_LT_D(uint32 Instruction) 
{    
#ifdef SAVE_C_COND_FMT
	r4300i_C_cond_fmt_d(Instruction);
#else

#ifdef ENABLE_64BIT_FPU
	uint64 u1 = read_64bit_fpu_reg(RD_FS);
	uint64 u2 = read_64bit_fpu_reg(RT_FT);
	if( *(double*)(&u1) < *(double*)(&u2) )        
#else
	CHK_64BITMODE("C_LT_D")  
	if( *((double *)&cFS) <  *((double *)&cFT))        
#endif
		cCON31 |=  0x00800000;
	else                                                                    
		cCON31 &=  0xFF7FFFFF;
#endif
}

void r4300i_C_NGE_D(uint32 Instruction)
{    
#ifdef SAVE_C_COND_FMT
	r4300i_C_cond_fmt_d(Instruction);
#else

#ifdef ENABLE_64BIT_FPU
	uint64 u1 = read_64bit_fpu_reg(RD_FS);
	uint64 u2 = read_64bit_fpu_reg(RT_FT);
	if( *(double*)(&u1) < *(double*)(&u2) )        
#else
	CHK_64BITMODE("C_NGE_D") 
	if( *((double *)&cFS) <  *((double *)&cFT))        
#endif
		cCON31 |=  0x00800000;
                        
	else                                                                    
		cCON31 &=  0xFF7FFFFF;
#endif
}

void r4300i_C_LE_S(uint32 Instruction) 
{    
#ifdef SAVE_C_COND_FMT
	r4300i_C_cond_fmt_s(Instruction);
#else
	if( *((float  *)&cFS) <= *((float  *)&cFT))        
		cCON31 |=  0x00800000;
                        
	else                                                                    
		cCON31 &=  0xFF7FFFFF;
#endif
}

void r4300i_C_NGT_S(uint32 Instruction)
{    
#ifdef SAVE_C_COND_FMT
	r4300i_C_cond_fmt_s(Instruction);
#else
	if( *((float  *)&cFS) <= *((float  *)&cFT))        
		cCON31 |=  0x00800000;
                        
	else                                                                    
		cCON31 &=  0xFF7FFFFF;
#endif
}

void r4300i_C_LE_D(uint32 Instruction) 
{    
#ifdef SAVE_C_COND_FMT
	r4300i_C_cond_fmt_d(Instruction);
#else

#ifdef ENABLE_64BIT_FPU
	uint64 u1 = read_64bit_fpu_reg(RD_FS);
	uint64 u2 = read_64bit_fpu_reg(RT_FT);
	if( *(double*)(&u1) <= *(double*)(&u2) )        
#else
	CHK_64BITMODE("C_LE_D")  
	if( *((double *)&cFS) <= *((double *)&cFT))        
#endif
		cCON31 |=  0x00800000;
	else                                                                    
		cCON31 &=  0xFF7FFFFF;
#endif
}

void r4300i_C_NGT_D(uint32 Instruction)
{    
#ifdef SAVE_C_COND_FMT
	r4300i_C_cond_fmt_d(Instruction);
#else

#ifdef ENABLE_64BIT_FPU
	uint64 u1 = read_64bit_fpu_reg(RD_FS);
	uint64 u2 = read_64bit_fpu_reg(RT_FT);
	if( *(double*)(&u1) <= *(double*)(&u2) )        
#else
	CHK_64BITMODE("C_NGT_D") 
	if( *((double *)&cFS) <= *((double *)&cFT))      
#endif
		cCON31 |=  0x00800000;
                        
	else                                                                    
		cCON31 &=  0xFF7FFFFF;
#endif
}

//---------------------------------------------------------------------------------------

//we won't need to confirm mode for control registers...
void r4300i_COP1_cfc1(uint32 Instruction) 
{ 
	uint32 rt_ft = RT_FT;
    uint32 rd_fs = RD_FS;

    if ((rt_ft) == 0) return;
    if( rd_fs == 0 || rd_fs == 31 )
	{
		gHWS_GPR[rt_ft]  = (__int64)(__int32)cCONFS;
	}
}

void r4300i_COP1_ctc1(uint32 Instruction) 
{
	uint32 rt_ft = RT_FT;
    uint32 rd_fs = RD_FS;
	
    if( (rd_fs == 31) && (cCON31 != (uint32)gHWS_GPR[rt_ft]) )	// Only Control Register 31 is writeable
	{
		// Check if the automatic round setting changes
		if( ((uint32)gHWS_GPR[rt_ft] ^ cCON31) & 0x00000003 )
		{
		}

		// Check if exceptions are enabled

		// Need to set 80x87 control register to auto round and precision control

		cCON31 = (uint32)gHWS_GPR[rt_ft];
	}
}

//---------------------------------------------------------------------------------------
#ifdef ENABLE_64BIT_FPU
void r4300i_COP1_cvtd_s(uint32 Instruction) 
{
	float f1 = *((float  *)&cFS);
	double val2 = (double)(f1);;
	write_64bit_fpu_reg(SA_FD, *((uint64*)&val2));
}
void r4300i_COP1_cvtd_w(uint32 Instruction) 
{
	__int32 i1 = *((_int32  *)&cFS);
	double val2 = (double)i1;
	write_64bit_fpu_reg(SA_FD, *(uint64*)&val2);
}
void r4300i_COP1_cvtd_l(uint32 Instruction) 
{   
	uint64 val = read_64bit_fpu_reg(RD_FS);
	double val2 = (double)(*((_int64  *)&val));
	write_64bit_fpu_reg(SA_FD, *(uint64*)&val2);
}
void r4300i_COP1_cvts_d(uint32 Instruction) 
{
	uint64 val = read_64bit_fpu_reg(RD_FS);
	double d1 = *((double *)&val);
	*((float  *)&cFD) = (float)d1;
}
void r4300i_COP1_cvts_l(uint32 Instruction) 
{   
	uint64 val = read_64bit_fpu_reg(RD_FS);
	*((float  *)&cFD) = (float)(*((_int64 *)&val));
}
void r4300i_COP1_cvtw_d(uint32 Instruction) 
{	
	uint64 val  = read_64bit_fpu_reg(RD_FS);
	double d1 = *((double *)&val);
	__int32 i1 = (__int32)d1;
	//*((__int32 *)&cFD) = (__int32)(*((double *)&val));
	*((__int32 *)&cFD) = (__int32)d1;
}
void r4300i_COP1_cvtl_s(uint32 Instruction) 
{
	float val = *((float  *)&cFS);
	__int64 val2 = (__int64)val;
	write_64bit_fpu_reg(SA_FD, *(uint64*)&val2);
}
void r4300i_COP1_cvtl_d(uint32 Instruction) 
{   
	uint64 val  = read_64bit_fpu_reg(RD_FS);
	__int64 val2 = (__int64)val;
	write_64bit_fpu_reg(SA_FD, *(uint64*)&val2);
}
#else
void r4300i_COP1_cvtd_s(uint32 Instruction) {   CHK_64BITMODE("cvtd_s") *((double *)&cFD) = (double)(*((float  *)&cFS));	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);}
void r4300i_COP1_cvtd_w(uint32 Instruction) {   CHK_64BITMODE("cvtd_w") *((double *)&cFD) = (double)(*((_int32 *)&cFS));	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);}
void r4300i_COP1_cvtd_l(uint32 Instruction) {   CHK_64BITMODE("cvtd_l") *((double *)&cFD) = (double)(*((_int64 *)&cFS));	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);}
void r4300i_COP1_cvts_d(uint32 Instruction) {   CHK_64BITMODE("cvtd_d") *((float  *)&cFD) =  (float)(*((double *)&cFS));	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);}
void r4300i_COP1_cvts_l(uint32 Instruction) {   CHK_64BITMODE("cvts_l") *((float  *)&cFD) =  (float)(*((_int64 *)&cFS));	SAVE_OP_COUNTER_INCREASE_INTERPRETER(3);}
void r4300i_COP1_cvtw_d(uint32 Instruction) {	CHK_64BITMODE("cvtw_d") *((__int32 *)&cFD) = (__int32)(*((double *)&cFS));	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);}
void r4300i_COP1_cvtl_s(uint32 Instruction) {   CHK_64BITMODE("cvtl_s") *((__int64 *)&cFD) = (__int64)(*((float *)&cFS));	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);}
void r4300i_COP1_cvtl_d(uint32 Instruction) {   CHK_64BITMODE("cvtl_d") *((__int64 *)&cFD) = (__int64)(*((double *)&cFS));	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);}
#endif
void r4300i_COP1_cvts_w(uint32 Instruction) {   CHK_64BITMODE("cvts_w") *((float  *)&cFD) =  (float)(*((_int32 *)&cFS));	SAVE_OP_COUNTER_INCREASE_INTERPRETER(3);}
//void r4300i_COP1_cvtw_s(uint32 Instruction) {	CHK_64BITMODE("cvtw_s") *((uint32 *)&cFD) = (uint32)(*((float  *)&cFS));	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);}
void r4300i_COP1_cvtw_s(uint32 Instruction) {	CHK_64BITMODE("cvtw_s") *((__int32 *)&cFD) = (__int32)(*((float  *)&cFS));	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);}


//---------------------------------------------------------------------------------------
void r4300i_COP1_mtc1(uint32 Instruction) 
{ 
	CHK_64BITMODE("mtc1")              
	*(uint32*)&cFS  = (uint32)gRT;
}
void r4300i_COP1_mfc1(uint32 Instruction) 
{ 
	CHK_64BITMODE("mfc1")   
	CHECK_R0_EQUAL_0(RT_FT, "mfc1")

	(*(_int64*)&gRT) =     (__int64)(*((_int32 *)&cFS)); 
	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);
}


#ifdef ENABLE_64BIT_FPU
void r4300i_COP1_dmtc1(uint32 Instruction)
{
	write_64bit_fpu_reg(RD_FS, gRT);
	//*((_int64*)&cFS) =         gRT;
}
void r4300i_COP1_dmfc1(uint32 Instruction)
{
	CHK_64BITMODE("dmfc1") 
	CHECK_R0_EQUAL_0(RT_FT, "dmfc1")
	gRT = read_64bit_fpu_reg(RD_FS);
	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);
}     

void r4300i_COP1_mov_d(uint32 Instruction)
{ 
	write_64bit_fpu_reg(SA_FD, read_64bit_fpu_reg(RD_FS));
	//CHK_64BITMODE("mov_d")  *((double *)&cFD) = *((double*)&cFS);
}

void r4300i_COP1_mov_s(uint32 Instruction)
{ 
	*(uint32*)&cFD  = *(uint32*)&cFS;
}

#else
void r4300i_COP1_dmtc1(uint32 Instruction){ CHK_64BITMODE("dmtc1") *((_int64*)&cFS) =         gRT;}
void r4300i_COP1_dmfc1(uint32 Instruction){ CHK_64BITMODE("dmfc1") if ((RT_FT) != 0) *((double*)&gRT) = *((double*)&cFS);}
void r4300i_COP1_mov_d(uint32 Instruction){ CHK_64BITMODE("mov_d")  *((double *)&cFD) = *((double*)&cFS);}
void r4300i_COP1_mov_s(uint32 Instruction)
{ 
	CHK_64BITMODE("mov_s")
	*(uint32*)&cFD  = *(uint32*)&cFS;
}
#endif


//-------------------------------------------------------------------------------------------------------

void r4300i_lwc1_faster(uint32 QuerAddr, uint32 rt_ft)
{   
	ADDR_WORD_ALIGN_CHECKING(QuerAddr)
    __try{
        *((uint32 *)&FPU_Reg(rt_ft)) = LOAD_UWORD_PARAM(QuerAddr);
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        if( NOT_IN_KO_K1_SEG(QuerAddr) )
        {
            QuerAddr = TranslateTLBAddressForLoad(QuerAddr);
        }

        __try{
			*((uint32 *)&FPU_Reg(rt_ft)) = LOAD_UWORD_PARAM(QuerAddr);
		}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			__try{
				*((uint32 *)&FPU_Reg(rt_ft)) = LOAD_UWORD_PARAM_2(QuerAddr);
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			TRACE2("%08X: LWC1 to access VA=%08X, Out of range. ", gHWS_pc, QuerAddr);
        }
   }
   }
    CHK_64BITMODE("lwc1")
	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);
}

void r4300i_lwc1(uint32 Instruction)        
{   
    LOAD_TLB_FUN
	ADDR_WORD_ALIGN_CHECKING(QuerAddr)
    __try{
        *(uint32*)&cFT = LOAD_UWORD_PARAM(QuerAddr);
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
			*(uint32*)&cFT = LOAD_UWORD_PARAM_2(QuerAddr);
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			TRACE2("%08X: LWC1 to access VA=%08X, Out of range. ", gHardwareState.pc, QuerAddr);
        }
   }
    //CHK_64BITMODE("lwc1")
}

//-------------------------------------------------------------------------------------------------------

void r4300i_swc1_faster(uint32 QuerAddr, uint32 rt_ft)
{
    
	ADDR_WORD_ALIGN_CHECKING(QuerAddr)
    __try {
        LOAD_UWORD_PARAM(QuerAddr) =  *((uint32 *)&FPU_Reg(rt_ft));
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){ 
        if( NOT_IN_KO_K1_SEG(QuerAddr) )
        {
            QuerAddr = TranslateTLBAddressForStore(QuerAddr);
        }

		__try {
			LOAD_UWORD_PARAM(QuerAddr) =  *((uint32 *)&FPU_Reg(rt_ft));
		}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try {
				LOAD_UWORD_PARAM_2(QuerAddr) =  *((uint32 *)&FPU_Reg(rt_ft));
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			TRACE2("%08X: SWC1 to access VA=%08X, Out of range. ", gHWS_pc, QuerAddr);
			}
        }
    }
    CHK_64BITMODE("swc1")
	SAVE_OP_COUNTER_INCREASE_INTERPRETER(1);
}

void r4300i_swc1(uint32 Instruction)
{   
	STORE_TLB_FUN
	ADDR_WORD_ALIGN_CHECKING(QuerAddr)
    __try {
        LOAD_UWORD_PARAM(QuerAddr) =  (uint32)cFT;
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){ 
        __try {
            LOAD_UWORD_PARAM_2(QuerAddr) =  (uint32)cFT;
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			TRACE2("%08X: SWC1 to access VA=%08X, Out of range. ", gHardwareState.pc, QuerAddr);
        }
    }
    //CHK_64BITMODE("swc1")
}


//-------------------------------------------------------------------------------------------------------

void r4300i_ldc1_faster(uint32 QuerAddr, uint32 rt_ft)
{   
	uint64 *TempStore;

    CHK_64BITMODE("ldc1")

	ADDR_DWORD_ALIGN_CHECKING(QuerAddr)

    __try{
        TempStore = pLOAD_DOUBLE_PARAM(QuerAddr);
                              *((uint32 *)&FPU_Reg(rt_ft))      = *(uint32*)(&(uint32*)TempStore[0]+1);
        *(uint32*)(&(uint32*)(*((uint32 *)&FPU_Reg(rt_ft)))+1)	= *(uint32*)(&(uint32*)TempStore[0]);
    }
    __except(NULL,EXCEPTION_EXECUTE_HANDLER){
        if ((QuerAddr & 0xC0000000) != 0x80000000)
        {
            //QuerAddr = TranslateTLBAddress(QuerAddr);
		    QuerAddr = TranslateTLBAddressForLoad(QuerAddr);
        }
        __try{
			TempStore = pLOAD_DOUBLE_PARAM(QuerAddr);
								  *((uint32 *)&FPU_Reg(rt_ft))      = *(uint32*)(&(uint32*)TempStore[0]+1);
			*(uint32*)(&(uint32*)(*((uint32 *)&FPU_Reg(rt_ft)))+1)	= *(uint32*)(&(uint32*)TempStore[0]);
		}
		__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			__try{
            TempStore = pLOAD_DOUBLE_PARAM_2(QuerAddr);
									  *((uint32 *)&FPU_Reg(rt_ft))      = *(uint32*)(&(uint32*)TempStore[0]+1);
				*(uint32*)(&(uint32*)(*((uint32 *)&FPU_Reg(rt_ft)))+1)	= *(uint32*)(&(uint32*)TempStore[0]);
        }
        __except(NULL,EXCEPTION_EXECUTE_HANDLER){
				TRACE2("%08X: LDC1 to access VA=%08X, Out of range. ", gHWS_pc, QuerAddr);
			}
        }
    }

	SAVE_OP_COUNTER_INCREASE_INTERPRETER(2);
}

void r4300i_ldc1(uint32 Instruction)
{   
    uint32 QuerAddr;
    uint32 rt_ft = RT_FT;
#ifdef ENABLE_64BIT_FPU
    uint64 *TempStore;
	union {
		uint32 reg[2];
		uint64 u1;
	} u;
#endif

	if( (RT_FT & 0x1) && (gHWS_COP0Reg[STATUS] & 0x04000000) == 0 )	return;

#ifdef ENABLE_64BIT_FPU
	QuerAddr = (uint32)((_int32)gBASE + (_int32)OFFSET_IMMEDIATE);
	ADDR_DWORD_ALIGN_CHECKING(QuerAddr)
    if ((QuerAddr ^ 0x80000000) & 0xC0000000)
    {
        //QuerAddr = TranslateTLBAddress(QuerAddr);
		QuerAddr = TranslateTLBAddressForStore(QuerAddr);
    }

    __try{
		TempStore = pLOAD_DOUBLE_PARAM(QuerAddr);;
		u.reg[0] = *(uint32*)(&(uint32*)TempStore[0]+1);
		u.reg[1] = *(uint32*)(&(uint32*)TempStore[0]);
		//write_64bit_fpu_reg(RT_FT, *(uint64*)&reg[0]);
		write_64bit_fpu_reg(RT_FT, u.u1);
	}
    __except(NULL,EXCEPTION_EXECUTE_HANDLER){
		__try{
			TempStore = pLOAD_DOUBLE_PARAM_2(QuerAddr);;
			u.reg[0] = *(uint32*)(&(uint32*)TempStore[0]+1);
			u.reg[1] = *(uint32*)(&(uint32*)TempStore[0]);
			//write_64bit_fpu_reg(RT_FT, *(uint64*)&reg[0]);
			write_64bit_fpu_reg(RT_FT, u.u1);
		}
		__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			TRACE2("%08X: LDC1 to access VA=%08X, Out of range. ", gHWS_pc, QuerAddr);
		}
	}
#else
    QUER_ADDR;

    r4300i_ldc1_faster(QuerAddr, rt_ft);
#endif
}


//---------------------------------------------------------------------------------------
void r4300i_sdc1_faster(uint32 QuerAddr, uint32 rt_ft)
{   
    uint64 *TempStore;

    CHK_64BITMODE("sdc1");
	

	ADDR_DWORD_ALIGN_CHECKING(QuerAddr)

    __try{
        TempStore = pLOAD_DOUBLE_PARAM(QuerAddr);
        *(uint32*)(&(uint32*)TempStore[0]+1)  = *((uint32 *)&FPU_Reg(rt_ft));
        *(uint32*)(&(uint32*)TempStore[0])    = *(uint32*)(&(uint32*)(*((uint32 *)&FPU_Reg(rt_ft)))+1);
    }
    __except(NULL,EXCEPTION_EXECUTE_HANDLER){
        if ((QuerAddr & 0xC0000000) != 0x80000000)
        {
		    QuerAddr = TranslateTLBAddressForStore(QuerAddr);
        }
		__try{
			TempStore = pLOAD_DOUBLE_PARAM(QuerAddr);
			*(uint32*)(&(uint32*)TempStore[0]+1)  = *((uint32 *)&FPU_Reg(rt_ft));
			*(uint32*)(&(uint32*)TempStore[0])    = *(uint32*)(&(uint32*)(*((uint32 *)&FPU_Reg(rt_ft)))+1);
		}
		__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			__try{
				TempStore = pLOAD_DOUBLE_PARAM_2(QuerAddr);
				*(uint32*)(&(uint32*)TempStore[0]+1)  = *((uint32 *)&FPU_Reg(rt_ft));
				*(uint32*)(&(uint32*)TempStore[0])    = *(uint32*)(&(uint32*)(*((uint32 *)&FPU_Reg(rt_ft)))+1);
			}
			__except(NULL,EXCEPTION_EXECUTE_HANDLER){
				TRACE2("%08X: SDC1 to access VA=%08X, Out of range. ", gHWS_pc, QuerAddr);
			}
		}
    }

	SAVE_OP_COUNTER_INCREASE_INTERPRETER(1);
}

void r4300i_sdc1(uint32 Instruction)    
{   
    uint32 QuerAddr;
    uint32 rt_ft = RT_FT;
    uint64 *TempStore;

    CHK_64BITMODE("sdc1");
	
	if( (RT_FT & 0x1) && (gHWS_COP0Reg[STATUS] & 0x04000000) == 0 ) return;

#ifdef ENABLE_64BIT_FPU
	QuerAddr = (uint32)((_int32)gBASE + (_int32)OFFSET_IMMEDIATE);
	ADDR_DWORD_ALIGN_CHECKING(QuerAddr)
    if ((QuerAddr ^ 0x80000000) & 0xC0000000)
    {
		QuerAddr = TranslateTLBAddressForStore(QuerAddr);
    }

    __try{
		uint32 reg[2];
        TempStore = pLOAD_DOUBLE_PARAM(QuerAddr);
		*(uint64*)&reg[0] = read_64bit_fpu_reg(RT_FT);
		*(uint32*)(&(uint32*)TempStore[0]+1)  = reg[0];
		*(uint32*)(&(uint32*)TempStore[0])    = reg[1];
	}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
		__try{
			uint32 reg[2];
			TempStore = pLOAD_DOUBLE_PARAM_2(QuerAddr);
			*(uint64*)&reg[0] = read_64bit_fpu_reg(RT_FT);
			*(uint32*)(&(uint32*)TempStore[0]+1)  = reg[0];
			*(uint32*)(&(uint32*)TempStore[0])    = reg[1];
		}__except(NULL,EXCEPTION_EXECUTE_HANDLER){
			TRACE2("%08X: SDC1 to access VA=%08X, Out of range. ", gHWS_pc, QuerAddr);
			return;
		}
	}
#else
    QUER_ADDR;

    r4300i_sdc1_faster(QuerAddr, rt_ft);
#endif
}


//---------------------------------------------------------------------------------------
/*
 Format:  TRUNC.W.S fd, fs
 Purpose: To convert an FP value to 32-bit fixed-point, rounding toward zero.
*/
void r4300i_COP1_truncw_s(uint32 Instruction) 
{
	float tempf = *((float  *)&cFS);
	CHK_64BITMODE("truncw_s")

	
	if( tempf >= 0 )
		*((__int32 *)&cFD) = (__int32)(tempf);
	else
		*((__int32 *)&cFD) = -((__int32)(-tempf));
	

	//*((uint32 *)&cFD) = (uint32)(*((float  *)&cFS));   //I do not see a reason why this 
	//													 operation is done in unsigned
}

/*
 Format:  TRUNC.W.D fd, fs
 Purpose: To convert an FP value to 32-bit fixed-point, rounding toward zero.
*/
void r4300i_COP1_truncw_d(uint32 Instruction) 
{
#ifdef ENABLE_64BIT_FPU
	uint64 val = read_64bit_fpu_reg(RD_FS);
	double tempd = *((double *)&val);
#else
	double tempd = *((double *)&cFS);

	CHK_64BITMODE("truncw_d")
#endif

	if( tempd >= 0 )
		*((__int32 *)&cFD) = (__int32)(tempd);
	else
		*((__int32 *)&cFD) = -((__int32)(-tempd));;   
}

/*
 Format:  TRUNC.L.S fd, fs
 Purpose: To convert an FP float value to 64-bit fixed-point, rounding toward zero.
*/
void r4300i_COP1_truncl_s(uint32 Instruction) 
{
	float tempf = *((float  *)&cFS);

#ifdef ENABLE_64BIT_FPU
	__int64 templ;

	if( tempf >= 0 )
	{
		templ = (__int64)(tempf);
		write_64bit_fpu_reg(RD_FS, *(uint64*)&templ);
	}
	else
	{
		templ = -((__int64)(-tempf));
		write_64bit_fpu_reg(RD_FS, *(uint64*)&templ);
	}
#else
	CHK_64BITMODE("truncl_s")

	if( tempf >= 0 )
		*((__int64 *)&cFD) = (__int64)(tempf);
	else
		*((__int64 *)&cFD) = -((__int64)(-tempf));
#endif
}
/*
 Format:  TRUNC.L.D fd, fs
 Purpose: To convert an FP double float value to 64-bit fixed-point, rounding toward zero.
*/

void r4300i_COP1_truncl_d(uint32 Instruction) 
{   
#ifdef ENABLE_64BIT_FPU
	uint64 val = read_64bit_fpu_reg(RD_FS);
	double tempf = *(double*)&val;
	__int64 templ = (__int64)tempf;

	if( tempf >= 0 )
		write_64bit_fpu_reg(RD_FS, *(uint64*)&templ);
	else
		write_64bit_fpu_reg(RD_FS, *(uint64*)&templ);
#else
	double tempf = *((double  *)&cFS);
	CHK_64BITMODE("truncl_d")
	if( tempf >= 0 )
		*((__int64 *)&cFD) = (__int64)(tempf);
	else
		*((__int64 *)&cFD) = -((__int64)(-tempf));
#endif
}

//---------------------------------------------------------------------------------------

/*
 Format:  FLOOR.L.S fd, fs
 Purpose: To convert an FP float value to 64-bit fixed-point, rounding down.
*/
void r4300i_COP1_floorl_s(uint32 Instruction) 
{
#ifdef ENABLE_64BIT_FPU
	__int64 templ = (__int64)(*((float  *)&cFS));
	write_64bit_fpu_reg(RD_FS, *(uint64*)&templ);
#else
	CHK_64BITMODE("floorl_S");
	*((__int64 *)&cFD) = (__int64)(*((float  *)&cFS));
#endif
}

/*
 Format:  FLOOR.L.D fd, fs
 Purpose: To convert an FP double float value to 64-bit fixed-point, rounding down.
*/
void r4300i_COP1_floorl_d(uint32 Instruction) 
{   
#ifdef ENABLE_64BIT_FPU
	uint64 val = read_64bit_fpu_reg(RD_FS);
	__int64 templ = (__int64)(*((double  *)&val));
	write_64bit_fpu_reg(RD_FS, *(uint64*)&templ);
#else
	CHK_64BITMODE("floorl_d");
	*((__int64 *)&cFD) = (__int64)(*((double  *)&cFS));
#endif
}


/*
 Format:  FLOOR.W.S fd, fs
 Purpose: To convert an FP float value to 32-bit fixed-point, rounding down.
*/
void r4300i_COP1_floorw_s(uint32 Instruction) 
{   
	*((__int32 *)&cFD) = (__int32)(*((float  *)&cFS));
}

/*
 Format:  FLOOR.W.D fd, fs
 Purpose: To convert an FP doule float value to 32-bit fixed-point, rounding down.
*/

void r4300i_COP1_floorw_d(uint32 Instruction) 
{   
#ifdef ENABLE_64BIT_FPU
	uint64 val = read_64bit_fpu_reg(RD_FS);
	*((__int32 *)&cFD) = (__int32)(*((double  *)&val));
#else
	CHK_64BITMODE("floorw_d");
	*((__int32 *)&cFD) = (__int32)(*((double  *)&cFS));
#endif
}




//---------------------------------------------------------------------------------------

/*
 Format:  ROUND.L.S fd, fs
 Purpose: To convert an FP float value to 64-bit fixed-point, rounding to nearest.
*/
void r4300i_COP1_roundl_s(uint32 Instruction) 
{
#ifdef ENABLE_64BIT_FPU
	__int64 templ = (__int64)(*((float  *)&cFS)+0.5);
	write_64bit_fpu_reg(RD_FS, *(uint64*)&templ);
#else
	CHK_64BITMODE("roundw_s");
	*((__int64 *)&cFD) = (__int64)(*((float  *)&cFS)+0.5);
#endif
}

/*
 Format:  ROUND.L.D fd, fs
 Purpose: To convert an FP double float value to 64-bit fixed-point, rounding to nearest.
*/

void r4300i_COP1_roundl_d(uint32 Instruction) 
{   
#ifdef ENABLE_64BIT_FPU
	uint64 val = read_64bit_fpu_reg(RD_FS);
	__int64 templ = (__int64)(*((double  *)&val)+0.5);
	write_64bit_fpu_reg(RD_FS, *(uint64*)&templ);
#else
	CHK_64BITMODE("roundw_s");
	*((__int64 *)&cFD) = (__int64)(*((double  *)&cFS)+0.5);
#endif
}

/*
 Format:  ROUND.W.S fd, fs
 Purpose: To convert an FP float value to 32-bit fixed-point, rounding to nearest.
*/
void r4300i_COP1_roundw_s(uint32 Instruction) 
{
	*((__int32 *)&cFD) = (__int32)(*((float  *)&cFS)+0.5);
}

/*
 Format:  ROUND.W.D fd, fs
 Purpose: To convert an FP double float value to 32-bit fixed-point, rounding to nearest.
*/
void r4300i_COP1_roundw_d(uint32 Instruction) 
{
#ifdef ENABLE_64BIT_FPU
	uint64 val = read_64bit_fpu_reg(RD_FS);
	*((__int32 *)&cFD) = (__int32)(*((double *)&val)+0.5);
#else

	CHK_64BITMODE("roundw_d");
	*((__int32 *)&cFD) = (__int32)(*((double *)&cFS)+0.5); 
#endif
}


//---------------------------------------------------------------------------------------

/*
 Format:  CEIL.W.S fd, fs
 Purpose: To convert an FP single float value to 32-bit fixed-point, rounding up.
*/
void r4300i_COP1_ceilw_s(uint32 Instruction)
{
	*((__int32 *)&cFD) = (__int32)ceil(((double)(*((float  *)&cFS))));
}

/*
 Format:  CEIL.W.D fd, fs
 Purpose: To convert an FP double float value to 32-bit fixed-point, rounding up.
*/
void r4300i_COP1_ceilw_d(uint32 Instruction)  
{   
#ifdef ENABLE_64BIT_FPU
	uint64 val = read_64bit_fpu_reg(RD_FS);
	*((__int32 *)&cFD) = (__int32)ceil(*((double  *)&val));
#else
	CHK_64BITMODE("ceilw_d");
	*((__int32 *)&cFD) = (__int32)ceil(*((double  *)&cFS));
#endif
}

/*
 Format:  CEIL.L.S fd, fs
 Purpose: To convert an FP single float value to 64-bit fixed-point, rounding up.
*/
void r4300i_COP1_ceill_s(uint32 Instruction)
{   
#ifdef ENABLE_64BIT_FPU
	__int64 val2 = (__int64)ceil(((double)(*((float  *)&cFS))));
	write_64bit_fpu_reg(SA_FD, *(uint64*)&val2);
#else
	CHK_64BITMODE("ceill_S");
	*((__int64 *)&cFD) = (__int64)ceil(((double)(*((float  *)&cFS))));
#endif
}

/*
 Format:  CEIL.L.D fd, fs
 Purpose: To convert an FP double float value to 64-bit fixed-point, rounding up.
*/
void r4300i_COP1_ceill_d(uint32 Instruction)  
{
#ifdef ENABLE_64BIT_FPU
	uint64 val = read_64bit_fpu_reg(RD_FS);
	__int64 val2 = (__int64)ceil(*((double  *)&cFS));
	write_64bit_fpu_reg(SA_FD,  *(uint64*)&val2);
#else
	CHK_64BITMODE("ceill_S");
	*((__int64 *)&cFD) = (__int64)ceil(*((double  *)&cFS));
#endif
}

//---------------------------------------------------------------------------------------

void r4300i_C_UN_S(uint32 Instruction)      
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_s(Instruction);
#else
	float fs, ft;
    
	fs = *(float*)&cFS;
	ft = *(float*)&cFT;

	if (_isnan((double)fs) || _isnan((double)ft) ) 
		cCON31 |= COP1_CONDITION_BIT;
	else
		cCON31 &= NOT_COP1_CONDITION_BIT;
#endif
}

void r4300i_C_UN_D(uint32 Instruction)      
{
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_d(Instruction);
#else
	uint64 ufs, uft;
	double dfs, dft;

	ufs = read_64bit_fpu_reg(RD_FS);
	uft = read_64bit_fpu_reg(RT_FT);

	dfs = *(double*)&ufs;
	dft = *(double*)&uft;

	if (_isnan(dfs) || _isnan(dft) ) 
		cCON31 |= COP1_CONDITION_BIT;
	else
		cCON31 &= NOT_COP1_CONDITION_BIT;
#endif
}

void r4300i_C_OLT_S(uint32 Instruction)
{
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_s(Instruction);
#else
	float fs, ft;

	fs = *(float*)&cFS;
	ft = *(float*)&cFT;

	if( (_isnan((double)fs) == 0 ) && (_isnan((double)ft) == 0 ) && ( fs < ft ) )
		cCON31 |= COP1_CONDITION_BIT;
	else
		cCON31 &= NOT_COP1_CONDITION_BIT;
#endif
}

void r4300i_C_OLT_D(uint32 Instruction)
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_d(Instruction);
#else
	uint64 ufs, uft;
	double dfs, dft;

	ufs = read_64bit_fpu_reg(RD_FS);
	uft = read_64bit_fpu_reg(RT_FT);

	dfs = *(double*)&ufs;
	dft = *(double*)&uft;

	if( _isnan(dfs)==0 && _isnan(dft) == 0 && ( dfs < dft ) )
		cCON31 |= COP1_CONDITION_BIT;
	else
		cCON31 &= NOT_COP1_CONDITION_BIT;
#endif
}

void r4300i_C_ULT_S(uint32 Instruction)     
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_s(Instruction);
#else
	float fs, ft;

	fs = *(float*)&cFS;
	ft = *(float*)&cFT;

	if( _isnan((double)fs) || _isnan((double)ft) || ( fs < ft ) )
		cCON31 |= COP1_CONDITION_BIT;
	else
		cCON31 &= NOT_COP1_CONDITION_BIT;
#endif
}

void r4300i_C_ULT_D(uint32 Instruction)     
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_d(Instruction);
#else
	uint64 ufs, uft;
	double dfs, dft;

	ufs = read_64bit_fpu_reg(RD_FS);
	uft = read_64bit_fpu_reg(RT_FT);

	dfs = *(double*)&ufs;
	dft = *(double*)&uft;

	if( _isnan(dfs) || _isnan(dft) || ( dfs < dft ) )
		cCON31 |= COP1_CONDITION_BIT;
	else
		cCON31 &= NOT_COP1_CONDITION_BIT;
#endif
}

void r4300i_C_OLE_S(uint32 Instruction)     
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_s(Instruction);
#else
	float fs, ft;

	fs = *(float*)&cFS;
	ft = *(float*)&cFT;

	if( (_isnan((double)fs) == 0 ) && (_isnan((double)ft) == 0 ) && ( fs <= ft ) )
		cCON31 |= COP1_CONDITION_BIT;
	else
		cCON31 &= NOT_COP1_CONDITION_BIT;
#endif
}

void r4300i_C_OLE_D(uint32 Instruction)     
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_d(Instruction);
#else
	uint64 ufs, uft;
	double dfs, dft;

	ufs = read_64bit_fpu_reg(RD_FS);
	uft = read_64bit_fpu_reg(RT_FT);

	dfs = *(double*)&ufs;
	dft = *(double*)&uft;

	if( _isnan(dfs)==0 && _isnan(dft) == 0 && ( dfs <= dft ) )
		cCON31 |= COP1_CONDITION_BIT;
	else
		cCON31 &= NOT_COP1_CONDITION_BIT;
#endif
}

void r4300i_C_ULE_S(uint32 Instruction)     
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_s(Instruction);
#else
	float fs, ft;

	fs = *(float*)&cFS;
	ft = *(float*)&cFT;

	if( _isnan((double)fs) || _isnan((double)ft) || ( fs <= ft ) )
		cCON31 |= COP1_CONDITION_BIT;
	else
		cCON31 &= NOT_COP1_CONDITION_BIT;
#endif
}

void r4300i_C_ULE_D(uint32 Instruction)     
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_d(Instruction);
#else
	uint64 ufs, uft;
	double dfs, dft;

	ufs = read_64bit_fpu_reg(RD_FS);
	uft = read_64bit_fpu_reg(RT_FT);

	dfs = *(double*)&ufs;
	dft = *(double*)&uft;

	if( _isnan(dfs) || _isnan(dft) || ( dfs <= dft ) )
		cCON31 |= COP1_CONDITION_BIT;
	else
		cCON31 &= NOT_COP1_CONDITION_BIT;
#endif
}

void r4300i_C_SF_S(uint32 Instruction)      
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_s(Instruction);
#else
	TRACE1("PC=%08X, Unhandled opcode C.SF.S", gHWS_pc);
#endif
}

void r4300i_C_SF_D(uint32 Instruction)      
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_d(Instruction);
#else
	TRACE1("PC=%08X, Unhandled opcode C.SF.D", gHWS_pc);     
#endif
}

void r4300i_C_NGLE_S(uint32 Instruction)    
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_s(Instruction);
#else
	TRACE1("PC=%08X, Unhandled opcode C.NGLE.S", gHWS_pc);
#endif
}

void r4300i_C_NGLE_D(uint32 Instruction)    
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_d(Instruction);
#else
	TRACE1("PC=%08X, Unhandled opcode C.NGLE.D", gHWS_pc);  
#endif
}

void r4300i_C_SEQ_S(uint32 Instruction)     
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_s(Instruction);
#else
	TRACE1("PC=%08X, Unhandled opcode C.SEQ.S", gHWS_pc);
#endif
}

void r4300i_C_SEQ_D(uint32 Instruction)     
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_d(Instruction);
#else
	TRACE1("PC=%08X, Unhandled opcode C.SEQ.D", gHWS_pc);    
#endif
}

void r4300i_C_NGL_S(uint32 Instruction)     
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_s(Instruction);
#else
	TRACE1("PC=%08X, Unhandled opcode C.NGL.S", gHWS_pc);   
#endif
}

void r4300i_C_NGL_D(uint32 Instruction)     
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_d(Instruction);
#else
	TRACE1("PC=%08X, Unhandled opcode C.NGL.D", gHWS_pc);  
#endif
}

void r4300i_C_F_S(uint32 Instruction)       
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_s(Instruction);
#else
	TRACE1("PC=%08X, Unhandled opcode C.F.S", gHWS_pc);   
#endif
}

void r4300i_C_F_D(uint32 Instruction)       
{   
#ifdef SAVE_C_COND_FMT
      r4300i_C_cond_fmt_d(Instruction);
#else
	TRACE1("PC=%08X, Unhnadled Opcode C.F.D", gHWS_pc);    
#endif
}

extern void TriggerFPUUnusableException();
extern void COP1_instr(uint32);
void COP1_NotAvailable_instr(uint32 Instruction)
{   
	TriggerFPUUnusableException();
	COP1_instr(Instruction);
}


