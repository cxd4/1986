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
#include "globals.h"
#include "r4300i.h"
#include "interrupt.h"
#include "win32/DLL_Video.h"
#include "win32/DLL_Audio.h"

extern uint32 TranslateTLBAddress(uint32 address);
extern void DMA_AI();

#define VI_INTERRUPTCOUNT			625000
#define VI_INTERRUPTCOUNT_MINUS_ONE 624999

#ifdef DEBUG_COMMON
#include <stdio.h>
#include "dbgprint.h"
extern char* DebugMainCPU();
char* DebugCOP(int n, uint32 Instruction);
char* DebugGPR(uint32 rt);
char* DebugCOPReg(uint32 n, uint32 reg);
char* DebugCOP0Reg(uint8 reg);
char* DebugCOP1Reg(uint8 reg);
extern char* DebugRegimm();
extern char* DebugSpecial();
extern void RefreshOpList(char *opcode);
extern void DebugPrintInstruction(uint32 instruction);
#endif

extern void ResizeVideoWindow(void);
extern BOOL Rom_Loaded;


/* DMA external functions */
extern void DMA_Write_PI();
extern void DMA_Read_PI();
extern void DMA_Write_SP();
extern void DMA_Read_SP();
extern void DMA_Read_SP();
extern void DMA_Write_SI();
extern void DMA_Read_SI();

extern void Handle_MI();
extern void Handle_SP(uint32 value);
extern void Handle_DPC(uint32 value);

//---------------------------------------------------------------------------------------

void r4300i_break(uint32 Instruction){}
void r4300i_sync(uint32 Instruction){}
void r4300i_syscall(uint32 Instruction){}
void UNUSED(uint32 Instruction)			{	DisplayError("%08X: Illegal opcode request.", pc);		}

//---------------------------------------------------------------------------------------

char op_str[0xFF];

//---------------------------------------------------------------------------------------

void r4300i_add(uint32 Instruction)	{sLOGICAL(+);}	void r4300i_dadd(uint32 Instruction) {sDLOGICAL(+);}
void r4300i_addu(uint32 Instruction){sLOGICAL(+);}	void r4300i_daddu(uint32 Instruction){sDLOGICAL(+);}
void r4300i_sub(uint32 Instruction)	{sLOGICAL(-);}	void r4300i_dsub(uint32 Instruction) {sDLOGICAL(-);}
void r4300i_subu(uint32 Instruction){sLOGICAL(-);}	void r4300i_dsubu(uint32 Instruction){sDLOGICAL(-);}

void r4300i_addi(uint32 Instruction) {sLOGICAL_WITH_IMM(+);}	void r4300i_daddi(uint32 Instruction) {	sDLOGICAL_WITH_IMM(+);}
void r4300i_addiu(uint32 Instruction){sLOGICAL_WITH_IMM(+);}	void r4300i_daddiu(uint32 Instruction){	sDLOGICAL_WITH_IMM(+);}

void r4300i_and(uint32 Instruction)	{uDLOGICAL(&);	}	void r4300i_andi(uint32 Instruction){uDLOGICAL_WITH_IMM(&);	}
void r4300i_or(uint32 Instruction)	{uDLOGICAL(|);	}	void r4300i_ori(uint32 Instruction)	{uDLOGICAL_WITH_IMM(|);	}
void r4300i_xor(uint32 Instruction)	{uDLOGICAL(^);	}	void r4300i_xori(uint32 Instruction){uDLOGICAL_WITH_IMM(^);	}
void r4300i_nor(uint32 Instruction)	{uDLOGICAL(|~);	}


#define LOAD_TLB_FUN												\
	uint32 QuerAddr;												\
																	\
	QUER_ADDR;														\
																	\
	if ((QuerAddr & 0xC0000000) != 0x80000000)						\
	{																\
		QuerAddr = TranslateTLBAddress(QuerAddr);					\
	}

//---------------------------------------------------------------------------------------

void r4300i_lb(uint32 Instruction)
{
	LOAD_TLB_FUN
	GPR[RT_FT] = LOAD_SBYTE_PARAM(QuerAddr);
}

//---------------------------------------------------------------------------------------

void r4300i_lbu(uint32 Instruction)
{	
	uint32 rt_ft = RT_FT;

	LOAD_TLB_FUN
				 (uint32)GPR[rt_ft] = LOAD_UBYTE_PARAM(QuerAddr);
	*(uint32*)(&(uint32*)GPR[rt_ft]+1) = 0;
}

//---------------------------------------------------------------------------------------

void r4300i_lh(uint32 Instruction)
{	
	LOAD_TLB_FUN
	GPR[RT_FT] = LOAD_SHALF_PARAM(QuerAddr);
}

//---------------------------------------------------------------------------------------

void r4300i_lhu(uint32 Instruction)
{	
	uint32 rt_ft = RT_FT;
	
	LOAD_TLB_FUN

				 (uint32)GPR[rt_ft] = LOAD_UHALF_PARAM(QuerAddr);
	*(uint32*)(&(uint32*)GPR[rt_ft]+1) = 0;
}

//---------------------------------------------------------------------------------------

void r4300i_lwu(uint32 Instruction)
{	
	uint32 rt_ft = RT_FT;

	LOAD_TLB_FUN
				 (uint32)GPR[rt_ft]    = LOAD_UWORD_PARAM(QuerAddr);;
	*(uint32*)(&(uint32*)GPR[rt_ft]+1) = 0;
}

//---------------------------------------------------------------------------------------

void r4300i_ll(uint32 Instruction)
{	
	LOAD_TLB_FUN
	GPR[RT_FT] = LOAD_SWORD_PARAM(QuerAddr);	
	LLbit = 1;
}

//---------------------------------------------------------------------------------------

void r4300i_ld(uint32 Instruction)
{
	uint32 UWORD[2];
	uint32 rt_ft = RT_FT;

	LOAD_TLB_FUN

	*(uint64*)UWORD = ((uint64)*((uint64*)sDWORD_R[((QuerAddr) >> 16)]+(((uint16)QuerAddr) >> 3)));
				 (uint32)GPR[rt_ft]    = UWORD[1];
	*(uint32*)(&(uint32*)GPR[rt_ft]+1) = UWORD[0];
}

//---------------------------------------------------------------------------------------

void r4300i_lld(uint32 Instruction)
{
	uint32 UWORD[2];
	uint32 rt_ft = RT_FT;

	LOAD_TLB_FUN

	*(uint64*)UWORD = ((uint64)*((uint64*)sDWORD_R[((uint16)((QuerAddr) >> 16))]+(((uint16)QuerAddr) >> 3)));
				 (uint32)GPR[rt_ft]    = UWORD[1];
	*(uint32*)(&(uint32*)GPR[rt_ft]+1) = UWORD[0];

	LLbit = 1;
}

//---------------------------------------------------------------------------------------

void r4300i_sb(uint32 Instruction){
	LOAD_TLB_FUN
	LOAD_UBYTE_PARAM(QuerAddr) =  (uint8)GPR[RT_FT];
}

//---------------------------------------------------------------------------------------

void r4300i_sh(uint32 Instruction)
{
	LOAD_TLB_FUN
	LOAD_UHALF_PARAM(QuerAddr) = (uint16)GPR[RT_FT];
}

//---------------------------------------------------------------------------------------

void r4300i_sd(uint32 Instruction)
{
	uint64* StoreDW;
	uint32 rt_ft = RT_FT;

	LOAD_TLB_FUN

	StoreDW = ((uint64*)sDWORD_R[((QuerAddr) >> 16)]+(((uint16)QuerAddr) >> 3));
	*(uint32*)(&(uint32*)StoreDW[0]+1) = (uint32)GPR[rt_ft];
	*(uint32*)StoreDW = *(uint32*)(&(uint32*)GPR[rt_ft]+1);
}

//---------------------------------------------------------------------------------------

void r4300i_sc(uint32 Instruction)
{	
	if (LLbit) 
	{
		LOAD_TLB_FUN
		LOAD_UWORD_PARAM(QuerAddr) = (uint32)GPR[RT_FT];
	}	
	(uint64)GPR[RT_FT] = (uint64)LLbit;
}

//---------------------------------------------------------------------------------------

void r4300i_scd(uint32 Instruction)
{
	uint64* StoreDW;
	uint32 rt_ft = RT_FT;

	if (LLbit)
	{
		LOAD_TLB_FUN

		StoreDW = ((uint64*)sDWORD_R[((QuerAddr) >> 16)]+(((uint16)QuerAddr) >> 3));
		*(uint32*)(&(uint32*)StoreDW[0]+1) = (uint32)GPR[rt_ft];
		*(uint32*)StoreDW				   = *(uint32*)(&(uint32*)GPR[rt_ft]+1);
	} 
	(uint64)GPR[RT_FT] = (uint64)LLbit;	
}

//---------------------------------------------------------------------------------------

void r4300i_mfhi(uint32 Instruction){GPR[RD_FS] = HI;}	void r4300i_mthi(uint32 Instruction){HI = GPR[RS_BASE_FMT];}
void r4300i_mflo(uint32 Instruction){GPR[RD_FS] = LO;}	void r4300i_mtlo(uint32 Instruction){LO = GPR[RS_BASE_FMT];}

//---------------------------------------------------------------------------------------

void r4300i_sll(uint32 Instruction)	{	 uLOGICAL_SHIFT(<<, SA_FD);	}	void r4300i_sllv(uint32 Instruction){	 uLOGICAL_SHIFT(<<, (GPR[RS_BASE_FMT]&0x1F));}
void r4300i_dsll(uint32 Instruction){	uDLOGICAL_SHIFT(<<, SA_FD);	}	void r4300i_dsllv(uint32 Instruction) {	uDLOGICAL_SHIFT(<<, (GPR[RS_BASE_FMT]&0x3F));}

void r4300i_srl(uint32 Instruction){	uLOGICAL_SHIFT(>>, SA_FD);					 }
void r4300i_srlv(uint32 Instruction){	uLOGICAL_SHIFT(>>, (GPR[RS_BASE_FMT]&0x1F)); }
void r4300i_dsrl(uint32 Instruction){	uDLOGICAL_SHIFT(>>, SA_FD);					 }
void r4300i_dsrlv(uint32 Instruction){	uDLOGICAL_SHIFT(>>, (GPR[RS_BASE_FMT]&0x3F));}
void r4300i_sra(uint32 Instruction){	 sLOGICAL_SHIFT(>>, SA_FD);					 }
void r4300i_srav(uint32 Instruction){	 sLOGICAL_SHIFT(>>, (GPR[RS_BASE_FMT]&0x1F));}

//Questionable: dsra & dsrav are probably not getting sign-extended
//---------------------------------------------------------------------------------------

void r4300i_dsra(uint32 Instruction)	{	
	DisplayError("dsra");
	sDLOGICAL_SHIFT(>>, SA_FD);	}	

//---------------------------------------------------------------------------------------

void r4300i_dsrav(uint32 Instruction) {	
	DisplayError("dsrav");
	sDLOGICAL_SHIFT(>>, (GPR[RS_BASE_FMT]&0x3F));}

//---------------------------------------------------------------------------------------

void r4300i_dsll32(uint32 Instruction){	
	*(uint32*)((uint8*)&GPR[RD_FS]+4) = (uint32)GPR[RT_FT] << SA_FD;
	*(uint32*)((uint8*)&GPR[RD_FS]  ) = 0;
}

//---------------------------------------------------------------------------------------

void r4300i_dsrl32(uint32 Instruction){
	(uint32)GPR[RD_FS] = *(uint32*)((uint8*)&GPR[RT_FT]+4);
	*(uint32*)((uint8*)&GPR[RD_FS]) = *(uint32*)((uint8*)&GPR[RD_FS]) >> SA_FD;
	*(uint32*)((uint8*)&GPR[RD_FS]+4) = 0;
}

//---------------------------------------------------------------------------------------

void r4300i_dsra32(uint32 Instruction){	
	(uint32)GPR[RD_FS] = *(uint32*)((uint8*)&GPR[RT_FT]+4);
	sLOGICAL_SHIFT(>>, SA_FD);
}

//---------------------------------------------------------------------------------------

void r4300i_slt(uint32 Instruction)		{if (		 GPR[RS_BASE_FMT] <		    GPR[RT_FT]		)	GPR[RD_FS] = 1;	else GPR[RD_FS] = 0;}
void r4300i_sltu(uint32 Instruction)	{if ((uint64)GPR[RS_BASE_FMT] < (uint64)GPR[RT_FT]		)	GPR[RD_FS] = 1;	else GPR[RD_FS] = 0;}
void r4300i_slti(uint32 Instruction)	{if (        GPR[RS_BASE_FMT] <		    OFFSET_IMMEDIATE)	GPR[RT_FT] = 1;	else GPR[RT_FT] = 0;}
void r4300i_sltiu(uint32 Instruction)	{if ((uint64)GPR[RS_BASE_FMT] < (uint64)OFFSET_IMMEDIATE)	GPR[RT_FT] = 1;	else GPR[RT_FT] = 0;}

void r4300i_COP0_mfc0(uint32 Instruction) {	GPR[RT_FT] = (_int64)(_int32)COP0Reg[RD_FS];	}
void r4300i_COP0_mtc0(uint32 Instruction) {	COP0Reg[RD_FS] = (uint32)GPR[RT_FT];			}

//---------------------------------------------------------------------------------------

void r4300i_lui(uint32 Instruction)		{GPR[RT_FT] = OFFSET_IMMEDIATE << (uint32)16;}

//---------------------------------------------------------------------------------------

void r4300i_bgez(uint32 Instruction)	{						if (GPR[RS_BASE_FMT] >= 0)	DELAY_SET					}
void r4300i_bgezal(uint32 Instruction)	{INTERPRETIVE_LINK(RA)	if (GPR[RS_BASE_FMT] >= 0)	DELAY_SET					}
void r4300i_bgezall(uint32 Instruction)	{INTERPRETIVE_LINK(RA)	if (GPR[RS_BASE_FMT] >= 0)	DELAY_SET	else DELAY_SKIP	}
void r4300i_bgezl(uint32 Instruction)	{						if (GPR[RS_BASE_FMT] >= 0)	DELAY_SET	else DELAY_SKIP	}

void r4300i_bltz(uint32 Instruction)	{						if (GPR[RS_BASE_FMT] <  0)	DELAY_SET					}
void r4300i_bltzal(uint32 Instruction)	{INTERPRETIVE_LINK(RA)	if (GPR[RS_BASE_FMT] <  0)	DELAY_SET					}
void r4300i_bltzall(uint32 Instruction)	{INTERPRETIVE_LINK(RA)	if (GPR[RS_BASE_FMT] <  0)	DELAY_SET	else DELAY_SKIP }
void r4300i_bltzl(uint32 Instruction)	{						if (GPR[RS_BASE_FMT] <  0)	DELAY_SET	else DELAY_SKIP }

void r4300i_bgtz(uint32 Instruction)	{						if (GPR[RS_BASE_FMT] >  0)	DELAY_SET					}
void r4300i_bgtzl(uint32 Instruction)	{						if (GPR[RS_BASE_FMT] >  0)	DELAY_SET	else DELAY_SKIP	}
void r4300i_blez(uint32 Instruction)	{						if (GPR[RS_BASE_FMT] <= 0)	DELAY_SET					}
void r4300i_blezl(uint32 Instruction)	{						if (GPR[RS_BASE_FMT] <= 0)	DELAY_SET	else DELAY_SKIP	}

void r4300i_bne(uint32 Instruction)		{if ((uint64)GPR[RS_BASE_FMT] != (uint64)GPR[RT_FT])	DELAY_SET					}
void r4300i_bnel(uint32 Instruction)	{if ((uint64)GPR[RS_BASE_FMT] != (uint64)GPR[RT_FT])	DELAY_SET	else DELAY_SKIP	}
void r4300i_beql(uint32 Instruction)	{if ((uint64)GPR[RS_BASE_FMT] == (uint64)GPR[RT_FT])	DELAY_SET	else DELAY_SKIP	}
void r4300i_beq(uint32 Instruction)		{
	if ((uint64)GPR[RS_BASE_FMT] == (uint64)GPR[RT_FT])
	{
		/* Speed hack */
		if (((uint16)OFFSET_IMMEDIATE) == (uint16)0xFFFF) 
		{
			{
				COP0Reg[COUNT] += VI_INTERRUPTCOUNT_MINUS_ONE;
				CPUdelayPC = pc;
				CPUdelay = 1;
			}
		}
		else DELAY_SET
	}
}

//---------------------------------------------------------------------------------------

void r4300i_div(uint32 Instruction) {
	uint32 TempRS = RS_BASE_FMT;
	uint32 TempRT = RT_FT;
	
	if ((uint32)GPR[TempRT] != 0) //Divide by zero in rom!!! (really should generate exception)
	{	
		sLOGIC(LO, GPR[TempRS], /, GPR[TempRT]);
		sLOGIC(HI, GPR[TempRS], %, GPR[TempRT]);
	}
}

//---------------------------------------------------------------------------------------

void r4300i_divu(uint32 Instruction) {
	uint32 TempRS = RS_BASE_FMT;
	uint32 TempRT = RT_FT;

	uint32 RSReg = (uint32)GPR[TempRS];
	uint32 RTReg = (uint32)GPR[TempRT];

	if ((uint32)RTReg != 0) //Divide by zero in rom!!!
	{
		uLOGIC(LO, RSReg, /, RTReg);
		uLOGIC(HI, RSReg, %, RTReg);
	}
}

//---------------------------------------------------------------------------------------

void r4300i_ddiv(uint32 Instruction) {
	_int64 TempRS;
	uint32 TempRT = RT_FT;
	_int64 RTReg = (_int64)GPR[TempRT];

	if ((uint32)RTReg != 0) //Divide by zero in rom!!!
	{	
		LO = (_int64)(TempRS=(uint64)GPR[RS_BASE_FMT])	/ RTReg;
		HI = TempRS										% RTReg;
	}
}

//---------------------------------------------------------------------------------------

void r4300i_ddivu(uint32 Instruction) {
	_int64 TempRS;
	uint32 TempRT = RT_FT;
	uint32 RTReg = (uint32)GPR[TempRT];

	if ((uint32)RTReg != 0) //Divide by zero in rom!!!
	{	
		LO = ((uint32)(TempRS=(uint64)GPR[RS_BASE_FMT])) / RTReg;
		HI =  (uint32)TempRS							 % RTReg;
	}
}

//---------------------------------------------------------------------------------------

void r4300i_mult(uint32 Instruction) {
	_int64 result;

	result = (_int64)(_int32)GPR[RS_BASE_FMT] * (_int64)(_int32)GPR[RT_FT];
	LO = (_int32)result;
	HI = (_int32)(result >> 32);
}

//---------------------------------------------------------------------------------------

void r4300i_multu(uint32 Instruction) {
	uint64 result;

	result = (uint64)(uint32)GPR[RS_BASE_FMT] * (uint64)(uint32)GPR[RT_FT];
	LO = (_int32)result;
	HI = (_int32)(result >> 32);
}

//---------------------------------------------------------------------------------------

void r4300i_dmultu(uint32 Instruction) {
     unsigned __int64 hh,hl,lh,ll,b;
     __int64 t1,t2;

     t1 = GPR[RS_BASE_FMT];
     t2 = GPR[RT_FT];

     hh = ((__int64)(t1 >> 32)	& 0x0ffffffff)	* ((__int64)(t2 >> 32)	&	0x0ffffffff);
     hl =  (__int64)(t1			& 0x0ffffffff)	* ((__int64)(t2 >> 32)	&	0x0ffffffff);
     lh = ((__int64)(t1 >> 32)	& 0x0ffffffff)	*  (__int64)(t2			&	0x0ffffffff);
     ll = ((__int64)(t1			& 0x0ffffffff)	*  (__int64)(t2			&	0x0ffffffff));

     LO = ((hl + lh) << 32) + ll;

     b=(((hl + lh) + (ll >> 32)) & 0x0100000000)>>32;

     HI = (unsigned __int64)hh + ((signed __int64)(unsigned __int32)(hl
>> 32) + (signed __int64)(unsigned __int32)(lh >> 32) + b);

}

//---------------------------------------------------------------------------------------

void r4300i_dmult(uint32 Instruction) {
     unsigned __int64 hh,hl,lh,ll,b;
     __int64 t1,t2;
     int sgn = 0;

     t1 = GPR[RS_BASE_FMT];
     t2 = GPR[RT_FT];
     if (t1 < 0) {sgn ^= 1; t1 = -t1;}
     if (t2 < 0) {sgn ^= 1; t2 = -t2;}

     hh = ((__int64)(t1 >> 32) & 0x0ffffffff)	* ((__int64)(t2 >> 32)	&	0x0ffffffff);
     hl = (__int64)(t1 & 0x0ffffffff)			* ((__int64)(t2 >> 32)	&	0x0ffffffff);
     lh = ((__int64)(t1 >> 32) & 0x0ffffffff)	*  (__int64)(t2			&	0x0ffffffff);
     ll = ((__int64)(t1 & 0x0ffffffff)			*  (__int64)(t2			&	0x0ffffffff));

     LO = ((hl + lh) << 32) + ll;

     b=(((hl + lh) + (ll >> 32)) & 0x0100000000)>>32;

     HI = (unsigned __int64)hh + ((signed __int64)(unsigned __int32)(hl	>> 32) + (signed __int64)(unsigned __int32)(lh >> 32) + b);

     b = (LO >= 0) ? 1 : 0;

     if (sgn != 0)
     {
          LO = -LO;
          HI = -HI + b;
     }
}

//---------------------------------------------------------------------------------------

void r4300i_jal(uint32 Instruction)	{	INTERPRETIVE_LINK(RA)		CPUdelay = 1;	CPUdelayPC = INSTR_INDEX;				}
void r4300i_jalr(uint32 Instruction){	INTERPRETIVE_LINK(RD_FS)	CPUdelay = 1;	CPUdelayPC = (uint32)GPR[RS_BASE_FMT];	}
void r4300i_jr(uint32 Instruction)	{								CPUdelay = 1;	CPUdelayPC = (uint32)GPR[RS_BASE_FMT];	}
void r4300i_j(uint32 Instruction)	{								CPUdelay = 1;	CPUdelayPC = INSTR_INDEX;
	
	if (pc == CPUdelayPC) { COP0Reg[COUNT] = VI_INTERRUPTCOUNT_MINUS_ONE;} /* Speed hack */
}

//---------------------------------------------------------------------------------------

void r4300i_lw(uint32 Instruction)
{
uint32 rt_ft = RT_FT;

	LOAD_TLB_FUN

	if (((QuerAddr & 0x04000000) != 0))
		Check_LW(QuerAddr, rt_ft);	
	else 
		GPR[rt_ft] = LOAD_SWORD_PARAM(QuerAddr);
}

//---------------------------------------------------------------------------------------

void r4300i_sw(uint32 Instruction) 
{	
uint32 rt_ft = RT_FT;

	LOAD_TLB_FUN

	if (((QuerAddr & 0x04000000) != 0)) 
		Check_SW(QuerAddr, (uint32)GPR[rt_ft]);
	else
		LOAD_UWORD_PARAM(QuerAddr) = (uint32)GPR[rt_ft];
}

//---------------------------------------------------------------------------------------

void r4300i_lwl(uint32 Instruction)
{
	uint32 LoadWord;
	uint32 rt_ft = RT_FT;
	uint32 vAddr;

	LOAD_TLB_FUN
	vAddr = QuerAddr & 0xfffffffc;
	LoadWord = (uint32)LOAD_UWORD_PARAM(vAddr);
	switch (QuerAddr & 3) 
	{
		case 0: GPR[rt_ft] = (_int64)(_int32)LoadWord;										break;
		case 1: GPR[rt_ft] = (_int64)(_int32)((GPR[rt_ft] & 0x000000ff) | (LoadWord << 8));	break;
		case 2: GPR[rt_ft] = (_int64)(_int32)((GPR[rt_ft] & 0x0000ffff) | (LoadWord << 16));break;
		case 3: GPR[rt_ft] = (_int64)(_int32)((GPR[rt_ft] & 0x00ffffff) | (LoadWord << 24));break;
	}
}

//---------------------------------------------------------------------------------------

void r4300i_lwr(uint32 Instruction) 
{
	uint32 LoadWord;
	uint32 rt_ft = RT_FT;
	uint32 vAddr;

	LOAD_TLB_FUN
	
	vAddr = QuerAddr & 0xfffffffc;
	LoadWord = (uint32)LOAD_UWORD_PARAM(vAddr);
	switch (QuerAddr & 3) 
	{
		case 3: GPR[rt_ft] = (_int64)(GPR[rt_ft] & 0xffffffff00000000) |  LoadWord;			break;
		case 2: GPR[rt_ft] = (_int64)(GPR[rt_ft] & 0xffffffffff000000) | (LoadWord >> 8);	break;
		case 1: GPR[rt_ft] = (_int64)(GPR[rt_ft] & 0xffffffffffff0000) | (LoadWord >> 16);	break;
		case 0: GPR[rt_ft] = (_int64)(GPR[rt_ft] & 0xffffffffffffff00) | (LoadWord >> 24);	break;
	}
}

//---------------------------------------------------------------------------------------

void r4300i_swl(uint32 Instruction)
{
	uint32 LoadWord;
	uint32 rt_ft = RT_FT;
	uint32 vAddr;

	LOAD_TLB_FUN
	vAddr = QuerAddr & 0xfffffffc;
	LoadWord = (uint32)LOAD_UWORD_PARAM(vAddr);
	switch (QuerAddr & 3)
	{
		case 0: LOAD_UWORD_PARAM(vAddr) = (uint32)GPR[rt_ft];											break;
		case 1: LOAD_UWORD_PARAM(vAddr) = (uint32)(LoadWord & 0xff000000) | ((uint32)GPR[rt_ft] >> 8);	break;
		case 2: LOAD_UWORD_PARAM(vAddr) = (uint32)(LoadWord & 0xffff0000) | ((uint32)GPR[rt_ft] >> 16);	break;
		case 3: LOAD_UWORD_PARAM(vAddr) = (uint32)(LoadWord & 0xffffff00) | ((uint32)GPR[rt_ft] >> 24);	break;
	}
}

//---------------------------------------------------------------------------------------

void r4300i_swr(uint32 Instruction) 
{
	uint32 LoadWord;
	uint32 rt_ft = RT_FT;
	uint32 vAddr;

	LOAD_TLB_FUN
	vAddr = QuerAddr & 0xfffffffc;
	LoadWord = (uint32)LOAD_UWORD_PARAM(vAddr);
	switch (QuerAddr & 3)
	{
		case 3:	LOAD_UWORD_PARAM(vAddr) = (uint32)GPR[rt_ft];												break;
		case 2:	LOAD_UWORD_PARAM(vAddr) = (uint32)((LoadWord & 0x000000FF) | ((uint32)GPR[rt_ft] << 8));	break;
		case 1:	LOAD_UWORD_PARAM(vAddr) = (uint32)((LoadWord & 0x0000FFFF) | ((uint32)GPR[rt_ft] << 16));	break;
		case 0:	LOAD_UWORD_PARAM(vAddr) = (uint32)((LoadWord & 0x00FFFFFF) | ((uint32)GPR[rt_ft] << 24));	break;
	}
}

//---------------------------------------------------------------------------------------

void r4300i_ldl(uint32 Instruction) {	DisplayError("LDL opcode not implemented");	}
void r4300i_ldr(uint32 Instruction) {	DisplayError("LDR opcode not implemented");	}
void r4300i_sdl(uint32 Instruction) {	DisplayError("SDL opcode not implemented");	}
void r4300i_sdr(uint32 Instruction) {	DisplayError("SDR opcode not implemented");	}

//---------------------------------------------------------------------------------------

extern void InitDynarec();

void r4300i_Init()
{
	//set all registers to 0
	memset( GPR,		0,	sizeof(GPR));
	memset( COP0Reg,	0,	sizeof(COP0Reg));
	memset( COP1Reg,	0,	sizeof(COP1Reg));
	HI = 0;
	LO = 0;
	LLbit = 0;

	/* SB 16/4/99 */
	COP0Reg[CONFIG]	= 0x0006E463;
	COP0Reg[STATUS]	= 0x34000000;
	COP0Reg[RANDOM]	= 0x0000002F;
	COP0Reg[PREVID]	= 0x00000b00;
	COP0Reg[RANDOM]	= MAXTLB;		//SB 6/8/99

	GPR[S4]	= 0x00000001;
	GPR[S6]	= 0x0000003F;
	GPR[SP]	= 0xA4001FF0;

	MI_VERSION_REG_R = 0x01010101;

	InitDynarec();
}

//---------------------------------------------------------------------------------------

void Check_LW(uint32 QuerAddr, uint32 rt_ft)
{
static int VLine;

	switch (QuerAddr)
	{
/* AI_LEN_REG		*///	case 0xA4500004: GPR[rt_ft] = (DLL_AI_TimeLeft)((uint8*)AI);	break;
/* AI_LEN_REG		*/		case 0xA4500004: GPR[rt_ft] = 0;	break;
/* SP_SEMAPHORE_REG */		case 0xA404001C: GPR[rt_ft] = SP_SEMAPHORE_REG; SP_SEMAPHORE_REG = 1;	break;
/* VI_CURRENT_REG   */		case 0xA4400010:	
								if (VLine++ == 512) 
									VLine = 0;
								GPR[rt_ft] = VLine;
								break;
/* SI_STATUS_REG			case 0xA4800018: SI_STATUS_REG &= 0xFFFFEFFF;
								if ((MI_INTR_REG_R) & MI_INTR_SI)
									SI_STATUS_REG |= 0x1000;
								break; */
		default: GPR[rt_ft] = LOAD_SWORD_PARAM(QuerAddr); break;
	}
}

//---------------------------------------------------------------------------------------

void Check_SW(uint32 QuerAddr, uint32 RTVal) {
	switch (QuerAddr) 
	{
/* MI_MODE_REG_ADDR      */	case 0xA4300000: WriteMI_ModeReg(RTVal)		;	break;	
/* MI_INTR_MASK_REG      */	case 0xA430000C: Handle_MI(RTVal)			;	break;
/* VI_CURRENT_REG        */	case 0xA4400010: MI_INTR_REG_R &= NOT_MI_INTR_VI;	if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) COP0Reg[CAUSE] &= 0xFFFFFBFF; break;
/* SP_STATUS_REG         */	case 0xA4040010: MI_INTR_REG_R &= NOT_MI_INTR_SP;	if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) COP0Reg[CAUSE] &= 0xFFFFFBFF; Handle_SP(RTVal);	break;
/* SI_STATUS_REG         */	case 0xA4800018: MI_INTR_REG_R &= NOT_MI_INTR_SI;	if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) COP0Reg[CAUSE] &= 0xFFFFFBFF; break;
/* AI_STATUS_REG         */	case 0xA450000C: MI_INTR_REG_R &= NOT_MI_INTR_AI;	if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) COP0Reg[CAUSE] &= 0xFFFFFBFF; break;
/* PI_STATUS_REG         */	case 0xA4600010: MI_INTR_REG_R &= NOT_MI_INTR_PI;	if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) COP0Reg[CAUSE] &= 0xFFFFFBFF; break;
/* DPC_STATUS_REG        */ case 0xA410000C: MI_INTR_REG_R &= NOT_MI_INTR_DP;	if ((MI_INTR_REG_R & MI_INTR_MASK_REG_R) == 0) COP0Reg[CAUSE] &= 0xFFFFFBFF; Handle_DPC(RTVal);break;
/* SP_SEMAPHORE_REG      */	case 0xA404001C: SP_SEMAPHORE_REG		= 0	;	break;
/* PI_WR_LEN_REG         */	case 0xA460000C: PI_WR_LEN_REG			= RTVal; DMA_Write_PI() ;		break;
/* SP_WR_LEN_REG         */ case 0xA404000C: SP_WR_LEN_REG			= RTVal; DMA_Write_SP() ;		break;
/* SP_RD_LEN_REG         */	case 0xA4040008: SP_RD_LEN_REG			= RTVal; DMA_Read_SP()  ;		break;
/* SI_PIF_ADDR_WR64B_REG */ case 0xA4800010: SI_PIF_ADDR_WR64B_REG	= RTVal; DMA_Write_SI() ;		break;
/* SI_PIF_ADDR_RD64B_REG */ case 0xA4800004: SI_PIF_ADDR_RD64B_REG	= RTVal; DMA_Read_SI()  ;		break;
/* VI_WIDTH_REG          	case 0xA4400008: VI_WIDTH_REG			= RTVal; ResizeVideoWindow() ;  break;*/
/* AI_LEN_REG            */	case 0xA4500004: AI_LEN_REG				= RTVal; DMA_AI(); (AUDIO_PlaySnd)(RDRAM,AI);

#ifdef DEBUG_COMMON
			sprintf(op_str, "%08X: Play %d bytes of audio at %08X", pc, AI_LEN_REG, AI_DRAM_ADDR_REG);
			RefreshOpList(op_str); 
#endif
		break; 			
		default: LOAD_UWORD_PARAM(QuerAddr) = RTVal; break;
	}
}