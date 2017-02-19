/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Emulator for Nintendo 64 console system                              |
 |  Copyright (C) 2000  Joel Middendorf  schibo@emuhq.com                       |
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
 |  email      : schibo@emuhq.com                                               |
 |  paper mail :                                                                |
 |______________________________________________________________________________|


This project is a documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar :). Many source
code comments are taken from anarko's n64toolkit with consent and are 
the property of anarko.
*/

#include <windows.h>
#include "r4300i.h"
#include "globals.h"
#include <math.h>

extern uint32 TranslateTLBAddress(uint32 address);

#ifdef SAFE_DYNAREC
#define CHK_64BITMODE(Name) if (COP0Reg[STATUS] & 0x04000000) { MessageBox(NULL, "64bit mode FPU detected!", Name, 0); }
#else
#define CHK_64BITMODE(Name)
#endif

void r4300i_COP1_add_s(uint32 Instruction) {	CHK_64BITMODE("add_s") (*((float*)&COP1Reg[SA_FD])) = (*((float  *)&COP1Reg[RD_FS])) + (*((float*)&COP1Reg[RT_FT]));	}
void r4300i_COP1_sub_s(uint32 Instruction) {	CHK_64BITMODE("sub_s") (*((float*)&COP1Reg[SA_FD])) = (*((float  *)&COP1Reg[RD_FS])) - (*((float*)&COP1Reg[RT_FT]));	}
void r4300i_COP1_mul_s(uint32 Instruction) {	CHK_64BITMODE("mul_s") (*((float*)&COP1Reg[SA_FD])) = (*((float  *)&COP1Reg[RD_FS])) * (*((float*)&COP1Reg[RT_FT]));	}
void r4300i_COP1_div_s(uint32 Instruction) {	CHK_64BITMODE("div_s") (*((float*)&COP1Reg[SA_FD])) = (*((float  *)&COP1Reg[RD_FS])) / (*((float*)&COP1Reg[RT_FT]));	}

void r4300i_COP1_add_d(uint32 Instruction) {	CHK_64BITMODE("add_d") *((double *)&COP1Reg[SA_FD]) =  *((double *)&COP1Reg[RD_FS])  + *((double *)&COP1Reg[RT_FT]);	}
void r4300i_COP1_sub_d(uint32 Instruction) {	CHK_64BITMODE("sub_d") *((double *)&COP1Reg[SA_FD]) =  *((double *)&COP1Reg[RD_FS])  - *((double *)&COP1Reg[RT_FT]);	}
void r4300i_COP1_mul_d(uint32 Instruction) {	CHK_64BITMODE("mul_d") *((double *)&COP1Reg[SA_FD]) =  *((double *)&COP1Reg[RD_FS])  * *((double *)&COP1Reg[RT_FT]);	}
void r4300i_COP1_div_d(uint32 Instruction) {	CHK_64BITMODE("div_d") *((double *)&COP1Reg[SA_FD]) =  *((double *)&COP1Reg[RD_FS])  / *((double *)&COP1Reg[RT_FT]);	}

//---------------------------------------------------------------------------------------

void r4300i_COP1_abs_s(uint32 Instruction) {	CHK_64BITMODE("abs_s")  *((float  *)&COP1Reg[SA_FD]) = (float)fabs((double)*((float  *)&COP1Reg[RD_FS]));	}
void r4300i_COP1_sqrt_s(uint32 Instruction){	CHK_64BITMODE("sqrt_s") *((float  *)&COP1Reg[SA_FD]) = (float)sqrt((double)*((float  *)&COP1Reg[RD_FS]));	}
void r4300i_COP1_neg_s(uint32 Instruction) {	CHK_64BITMODE("neg_s")  *((float  *)&COP1Reg[SA_FD]) = (float)   -(        *((float  *)&COP1Reg[RD_FS]));	}
void r4300i_COP1_abs_d(uint32 Instruction) {	CHK_64BITMODE("abs_d")  *((double *)&COP1Reg[RD_FS]) =        fabs(        *((double *)&COP1Reg[SA_FD]));	}
void r4300i_COP1_sqrt_d(uint32 Instruction){	CHK_64BITMODE("sqrt_d") *((double *)&COP1Reg[SA_FD]) =		  sqrt(        *((double *)&COP1Reg[RD_FS]));	}
void r4300i_COP1_neg_d(uint32 Instruction) {	CHK_64BITMODE("neg_d")  *((double *)&COP1Reg[SA_FD]) =			 -(        *((double *)&COP1Reg[RD_FS]));	}

//---------------------------------------------------------------------------------------

void r4300i_COP1_bc1f(uint32 Instruction) {	CHK_64BITMODE("bc1f")  if((((uint32)COP1Con[31] & 0x00800000)) == 0)	DELAY_SET						}
void r4300i_COP1_bc1t(uint32 Instruction) {	CHK_64BITMODE("bc1t")  if((((uint32)COP1Con[31] & 0x00800000)) != 0)	DELAY_SET						}
void r4300i_COP1_bc1fl(uint32 Instruction){	CHK_64BITMODE("bc1fl") if((((uint32)COP1Con[31] & 0x00800000)) == 0)	DELAY_SET	else	DELAY_SKIP	}
void r4300i_COP1_bc1tl(uint32 Instruction){	CHK_64BITMODE("bc1tl") if((((uint32)COP1Con[31] & 0x00800000)) != 0)	DELAY_SET	else	DELAY_SKIP	}

//---------------------------------------------------------------------------------------

void r4300i_C_EQ_S(uint32 Instruction) {	CHK_64BITMODE("C_EQ_S")  if( *((float  *)&COP1Reg[RD_FS]) == *((float  *)&COP1Reg[RT_FT]))		COP1Con[31] |=  0x00800000;
						else																	COP1Con[31] &=  0xFF7FFFFF;}
void r4300i_C_UEQ_S(uint32 Instruction){	CHK_64BITMODE("C_UEQ_S") if( *((float  *)&COP1Reg[RD_FS]) == *((float  *)&COP1Reg[RT_FT]))		COP1Con[31] |=  0x00800000;
						else																	COP1Con[31] &=  0xFF7FFFFF;}
void r4300i_C_EQ_D(uint32 Instruction) {	CHK_64BITMODE("C_EQ_D")  if( *((double *)&COP1Reg[RD_FS]) == *((double *)&COP1Reg[RT_FT]))		COP1Con[31] |=  0x00800000;
						else																	COP1Con[31] &=  0xFF7FFFFF;}
void r4300i_C_UEQ_D(uint32 Instruction){	CHK_64BITMODE("C_UEQ_D") if( *((double *)&COP1Reg[RD_FS]) == *((double *)&COP1Reg[RT_FT]))		COP1Con[31] |=  0x00800000;
						else																	COP1Con[31] &=  0xFF7FFFFF;}
void r4300i_C_LT_S(uint32 Instruction) {	CHK_64BITMODE("C_LT_S")  if( *((float  *)&COP1Reg[RD_FS]) <  *((float  *)&COP1Reg[RT_FT]))		COP1Con[31] |=  0x00800000;
						else																	COP1Con[31] &=  0xFF7FFFFF;}
void r4300i_C_NGE_S(uint32 Instruction){	CHK_64BITMODE("C_NGE_S") if( *((float  *)&COP1Reg[RD_FS]) <  *((float  *)&COP1Reg[RT_FT]))		COP1Con[31] |=  0x00800000;
						else																	COP1Con[31] &=  0xFF7FFFFF;}
void r4300i_C_LT_D(uint32 Instruction) {	CHK_64BITMODE("C_LT_D")  if( *((double *)&COP1Reg[RD_FS]) <  *((double *)&COP1Reg[RT_FT]))		COP1Con[31] |=  0x00800000;
						else																	COP1Con[31] &=  0xFF7FFFFF;}
void r4300i_C_NGE_D(uint32 Instruction){	CHK_64BITMODE("C_NGE_D") if( *((double *)&COP1Reg[RD_FS]) <  *((double *)&COP1Reg[RT_FT]))		COP1Con[31] |=  0x00800000;
						else																	COP1Con[31] &=  0xFF7FFFFF;}
void r4300i_C_LE_S(uint32 Instruction) {	CHK_64BITMODE("C_LE_S")  if( *((float  *)&COP1Reg[RD_FS]) <= *((float  *)&COP1Reg[RT_FT]))		COP1Con[31] |=  0x00800000;
						else																	COP1Con[31] &=  0xFF7FFFFF;}
void r4300i_C_NGT_S(uint32 Instruction){	CHK_64BITMODE("C_NGT_S") if( *((float  *)&COP1Reg[RD_FS]) <= *((float  *)&COP1Reg[RT_FT]))		COP1Con[31] |=  0x00800000;
						else																	COP1Con[31] &=  0xFF7FFFFF;}
void r4300i_C_LE_D(uint32 Instruction) {	CHK_64BITMODE("C_LE_D")  if( *((double *)&COP1Reg[RD_FS]) <= *((double *)&COP1Reg[RT_FT]))		COP1Con[31] |=  0x00800000;
						else																	COP1Con[31] &=  0xFF7FFFFF;}
void r4300i_C_NGT_D(uint32 Instruction){	CHK_64BITMODE("C_NGT_D") if( *((double *)&COP1Reg[RD_FS]) <= *((double *)&COP1Reg[RT_FT]))		COP1Con[31] |=  0x00800000;
						else																	COP1Con[31] &=  0xFF7FFFFF;}

//---------------------------------------------------------------------------------------

//we won't need to confirm mode for control registers...
void r4300i_COP1_cfc1(uint32 Instruction) {	GPR[RT_FT] = (_int64)(_int32)COP1Con[RD_FS];}
void r4300i_COP1_ctc1(uint32 Instruction) {	COP1Con[RD_FS] = (uint32)GPR[RT_FT];}

//---------------------------------------------------------------------------------------

void r4300i_COP1_cvtd_s(uint32 Instruction) {	CHK_64BITMODE("cvtd_s") *((double *)&COP1Reg[SA_FD]) = (double)(*((float  *)&COP1Reg[RD_FS]));}
void r4300i_COP1_cvtd_w(uint32 Instruction) {	CHK_64BITMODE("cvtd_w") *((double *)&COP1Reg[SA_FD]) = (double)(*((_int32 *)&COP1Reg[RD_FS]));}
void r4300i_COP1_cvtd_l(uint32 Instruction) {	CHK_64BITMODE("cvtd_l") *((double *)&COP1Reg[SA_FD]) = (double)(*((_int64 *)&COP1Reg[RD_FS]));}
void r4300i_COP1_cvts_d(uint32 Instruction) {	CHK_64BITMODE("cvtd_d") *((float  *)&COP1Reg[SA_FD]) =  (float)(*((double *)&COP1Reg[RD_FS]));}
void r4300i_COP1_cvts_w(uint32 Instruction) {	CHK_64BITMODE("cvts_w") *((float  *)&COP1Reg[SA_FD]) =  (float)(*((_int32 *)&COP1Reg[RD_FS]));}
void r4300i_COP1_cvts_l(uint32 Instruction) {	CHK_64BITMODE("cvts_l") *((float  *)&COP1Reg[SA_FD]) =  (float)(*((_int64 *)&COP1Reg[RD_FS]));}
void r4300i_COP1_cvtw_s(uint32 Instruction) {	CHK_64BITMODE("cvtw_s") *((uint32 *)&COP1Reg[SA_FD]) = (uint32)(*((float  *)&COP1Reg[RD_FS]));}
void r4300i_COP1_cvtw_d(uint32 Instruction) {	CHK_64BITMODE("cvtw_d") *((uint32 *)&COP1Reg[SA_FD]) = (uint32)(*((double *)&COP1Reg[RD_FS]));}

//---------------------------------------------------------------------------------------

void r4300i_COP1_mtc1(uint32 Instruction) { CHK_64BITMODE("mtc1")  COP1Reg[RD_FS]  = (uint32)GPR[RT_FT];}
void r4300i_COP1_dmtc1(uint32 Instruction){ CHK_64BITMODE("dmtc1") *((_int64*)&COP1Reg[SA_FD]) =         GPR[RT_FT];}

void r4300i_COP1_mfc1(uint32 Instruction) {	CHK_64BITMODE("mfc1") 	(*(_int64*)&GPR[RT_FT]) =     (_int32)COP1Reg[RD_FS]; }
void r4300i_COP1_dmfc1(uint32 Instruction){ CHK_64BITMODE("dmfc1") 	*((double*)&GPR[RT_FT]) = *((double*)&COP1Reg[RD_FS]);}

void r4300i_COP1_mov_s(uint32 Instruction){ CHK_64BITMODE("mov_s") 	COP1Reg[SA_FD]  =             COP1Reg[RD_FS];}
void r4300i_COP1_mov_d(uint32 Instruction){ CHK_64BITMODE("mov_d")  *((double *)&COP1Reg[SA_FD]) = *((double*)&COP1Reg[RD_FS]);}

//-------------------------------------------------------------------------------------------------------

void r4300i_lwc1(uint32 Instruction)		
{	
	uint32 QuerAddr;

	QUER_ADDR;

	CHK_64BITMODE("lwc1")
	if ((QuerAddr & 0xC0000000) != 0x80000000)
	{
		QuerAddr = TranslateTLBAddress(QuerAddr);
	}
	COP1Reg[RT_FT] = LOAD_UWORD_PARAM(QuerAddr);	
}

//-------------------------------------------------------------------------------------------------------

void r4300i_swc1(uint32 Instruction){	
	
	uint32 QuerAddr;

	QUER_ADDR;
	
	CHK_64BITMODE("swc1")
	if ((QuerAddr & 0xC0000000) != 0x80000000)
	{
		QuerAddr = TranslateTLBAddress(QuerAddr);
	}
	LOAD_UWORD_PARAM(QuerAddr) = (uint32)COP1Reg[RT_FT];
}

//-------------------------------------------------------------------------------------------------------

void r4300i_ldc1(uint32 Instruction)
{	
	uint32 QuerAddr;
	uint32 UWORD[2];
	uint32 rt_ft = RT_FT;

	QUER_ADDR;

	CHK_64BITMODE("ldc1")

	if ((QuerAddr & 0xC0000000) != 0x80000000)
	{
		QuerAddr = TranslateTLBAddress(QuerAddr);
	}

	*(uint64*)UWORD = ((uint64)*((uint64*)sDWORD_R[((QuerAddr) >> 16)]+(((uint16)QuerAddr) >> 3)));
						 COP1Reg[rt_ft]		= UWORD[1];
	*(uint32*)(&(uint32*)COP1Reg[rt_ft]+1)	= UWORD[0];
}
//---------------------------------------------------------------------------------------
//This sdc1 is for 32bit mode only!!
void r4300i_sdc1(uint32 Instruction)	{	
	uint32 QuerAddr;
	uint32 rt_ft = RT_FT;
	uint64 *TempStore;

	QUER_ADDR;
	
	CHK_64BITMODE("sdc1")
	if ((QuerAddr & 0xC0000000) != 0x80000000)
	{
		QuerAddr = TranslateTLBAddress(QuerAddr);
	}

	TempStore = ((uint64*)sDWORD_R[((QuerAddr) >> 16)] + (((uint16)QuerAddr) >> 3 ));
	*(uint32*)(&(uint32*)TempStore[0]+1)  = COP1Reg[rt_ft];
	*(uint32*)(&(uint32*)TempStore[0])    = *(uint32*)(&(uint32*)COP1Reg[rt_ft]+1);
}

//---------------------------------------------------------------------------------------

void r4300i_COP1_truncw_s(uint32 Instruction) {CHK_64BITMODE("truncw_s") *((uint32 *)&COP1Reg[SA_FD]) = (uint32)(*((float  *)&COP1Reg[RD_FS]));	}
void r4300i_COP1_truncw_d(uint32 Instruction) {CHK_64BITMODE("truncw_d") *((_int32 *)&COP1Reg[SA_FD]) = (_int32)(*((double *)&COP1Reg[RD_FS]));	}

//---------------------------------------------------------------------------------------

void r4300i_COP1_cvtl_s(uint32 Instruction)	{	DisplayError("Unhandled Opcode CVT.L.S");		}
void r4300i_COP1_cvtl_d(uint32 Instruction)	{	DisplayError("Unhandled Opcode CVT.L.D");		}
void r4300i_COP1_floorl(uint32 Instruction)	{	DisplayError("Unhandled Opcode FLOOR.L.fmt");	}
void r4300i_COP1_floorw(uint32 Instruction)	{	DisplayError("Unhandled Opcode FLOOR.W.fmt");	}
void r4300i_COP1_roundl(uint32 Instruction)	{	DisplayError("Unhandled Opcode ROUND.L.fmt");	}
void r4300i_COP1_roundw(uint32 Instruction)	{	DisplayError("Unhandled Opcode ROUND.W.fmt");	}
void r4300i_COP1_truncl(uint32 Instruction)	{	DisplayError("Unhandled Opcode TRUNC.L.fmt");	}
void r4300i_COP1_ceilw(uint32 Instruction)	{	DisplayError("Unhandled Opcode CEIL.W.fmt");	}
void r4300i_COP1_ceill(uint32 Instruction)	{	DisplayError("Unhandled Opcode CEIL.L.fmt");	}
void r4300i_C_UN_S(uint32 Instruction)		{	DisplayError("Unhandled Opcode C.UN.S");		}
void r4300i_C_UN_D(uint32 Instruction)		{	DisplayError("Unhandled Opcode C.UN.D");		}
void r4300i_C_OLT_S(uint32 Instruction)		{	DisplayError("Unhandled opcode C.OLT.S");		}
void r4300i_C_OLT_D(uint32 Instruction)		{	DisplayError("Unhandled opcode C.OLT.D");		}
void r4300i_C_ULT_S(uint32 Instruction)		{	DisplayError("Unhandled opcode C.ULT.S");		}
void r4300i_C_ULT_D(uint32 Instruction)		{	DisplayError("Unhandled opcode C.ULT.D");		}
void r4300i_C_OLE_S(uint32 Instruction)		{	DisplayError("Unhandled opcode C.OLE.S");		}
void r4300i_C_OLE_D(uint32 Instruction)		{	DisplayError("Unhandled opcode C.OLE.D");		}
void r4300i_C_ULE_S(uint32 Instruction)		{	DisplayError("Unhandled opcode C.ULE.S");		}
void r4300i_C_ULE_D(uint32 Instruction)		{	DisplayError("Unhandled opcode C.ULE.D");		}
void r4300i_C_SF_S(uint32 Instruction)		{	DisplayError("Unhandled opcode C.SF.S");		}
void r4300i_C_SF_D(uint32 Instruction)		{	DisplayError("Unhandled opcode C.SF.D");		}
void r4300i_C_NGLE_S(uint32 Instruction)	{	DisplayError("Unhandled opcode C.NGLE.S");		}
void r4300i_C_NGLE_D(uint32 Instruction)	{	DisplayError("Unhandled opcode C.NGLE.D");		}
void r4300i_C_SEQ_S(uint32 Instruction)		{	DisplayError("Unhandled opcode C.SEQ.S");		}
void r4300i_C_SEQ_D(uint32 Instruction)		{	DisplayError("Unhandled opcode C.SEQ.D");		}
void r4300i_C_NGL_S(uint32 Instruction)		{	DisplayError("Unhandled opcode C.NGL.S");		}
void r4300i_C_NGL_D(uint32 Instruction)		{	DisplayError("Unhandled opcode C.NGL.D");		}
void r4300i_C_F_S(uint32 Instruction)		{	DisplayError("Unhandled opcode C.F.S");			}
void r4300i_C_F_D(uint32 Instruction)		{	DisplayError("Unhnadled Opcode C.F.D");			}