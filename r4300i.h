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


The project is a direct documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar :). Most source
code comments are taken directly from anarko's n64toolkit with consent and are 
the property of anarko.
*/

#ifndef _R4300I_H__1964_
#define _R4300I_H__1964_

extern void dyn_r4300i_Step(void);
extern void RunTheInterpreter();
extern void r4300i_Init(void);
extern void r4300i_Reset(void);

#define FPU COP1Reg


#define _GET_RSP_INSTRUCTION_	Instruction = *RSP_InstructionPointer;
#define _OPCODE_			(unsigned)(Instruction >> 26)
#define _FUNCTION_			(unsigned)(Instruction & 0x3F)
#define SA_FD               (unsigned)((Instruction >> 6)  & 0x1F)
#define RD_FS				(uint32)((Instruction >> 11) & 0x1F)
#define RT_FT				(uint32)((Instruction >> 16) & 0x1F)
#define RS_BASE_FMT			(uint32)((Instruction >> 21) & 0x1F)
#define OFFSET_IMMEDIATE	((_int16)Instruction)
#define BITWISE_IMMEDIATE	(uint16)(Instruction & 0xFFFF)

#define _RS_	RS_BASE_FMT
#define _RD_	RD_FS
#define _RT_	RT_FT
#define _F_                 ((uint8)(Instruction) & 0x3f)
#define _ND_				((uint8)((Instruction >> 17) & 0x1))
#define _TF_				((uint8)((Instruction >> 16) & 0x1))

/* The first 4 bits of instr_index = the 1st   4 bits of pc
   The next 26 bits of instr_index = the last 26 bits of instruction
   The last  2 bits of instr_index = 00
   instr_index is a misnomer (should be called target, but easier to understand this way)
*/
#define INSTR_INDEX			( (pc & 0xF0000000) | ((Instruction & 0x03FFFFFF) << 2) )

#define DELAY_SET 				{	CPUdelayPC = pc + 4 + (OFFSET_IMMEDIATE << 2); CPUdelay = 1;	}
#define DELAY_SKIP				{	__asm { add pc, 4 }												}
#define INTERPRETIVE_LINK(X)	{	GPR[X] = (_int32)(pc + 8);										}


//Opcode Logic Macros 
//---------------------
#define   sLOGIC(Sum, Operand1, OPERATOR, Operand2)	Sum =		  (_int64)((_int32)Operand1 OPERATOR (_int32)Operand2)
#define   uLOGIC(Sum, Operand1, OPERATOR, Operand2)	Sum = (_int64)(_int32)((uint32)Operand1 OPERATOR (uint32)Operand2)

#define  sDLOGIC(Sum, Operand1, OPERATOR, Operand2)	Sum = (_int64)Operand1 OPERATOR (_int64)Operand2
#define  uDLOGIC(Sum, Operand1, OPERATOR, Operand2)	Sum = (uint64)Operand1 OPERATOR (uint64)Operand2
//---------------------

#define  sLOGICAL(OPERATOR)				 sLOGIC(GPR[_RD_], GPR[_RS_], OPERATOR, GPR[_RT_])
#define sDLOGICAL(OPERATOR)				sDLOGIC(GPR[_RD_], GPR[_RS_], OPERATOR, GPR[_RT_])
#define uDLOGICAL(OPERATOR)				uDLOGIC(GPR[_RD_], GPR[_RS_], OPERATOR, GPR[_RT_])

#define  sLOGICAL_WITH_IMM(OPERATOR)	 sLOGIC(GPR[_RT_], GPR[_RS_], OPERATOR,	(_int16)OFFSET_IMMEDIATE)
#define sDLOGICAL_WITH_IMM(OPERATOR)    sDLOGIC(GPR[_RT_], GPR[_RS_], OPERATOR, (_int16)OFFSET_IMMEDIATE)
#define uDLOGICAL_WITH_IMM(OPERATOR)	uDLOGIC(GPR[_RT_], GPR[_RS_], OPERATOR, (uint16)OFFSET_IMMEDIATE)

#define  sLOGICAL_SHIFT(OPERATOR, ShiftAmount)	 sLOGIC(GPR[_RD_], GPR[_RT_], OPERATOR, (ShiftAmount))
#define sDLOGICAL_SHIFT(OPERATOR, ShiftAmount)	sDLOGIC(GPR[_RD_], GPR[_RT_], OPERATOR, (ShiftAmount))
#define  uLOGICAL_SHIFT(OPERATOR, ShiftAmount)	 uLOGIC(GPR[_RD_], GPR[_RT_], OPERATOR, (ShiftAmount))
#define uDLOGICAL_SHIFT(OPERATOR, ShiftAmount)	uDLOGIC(GPR[_RD_], GPR[_RT_], OPERATOR, (ShiftAmount))



extern void Check_LW(unsigned __int32 rt_ft, unsigned __int32 QuerAddr);
extern void Check_SW(unsigned __int32 rt_ft, unsigned __int32 QuerAddr);
extern void UNUSED(					unsigned __int32 Instruction);
extern void r4300i_cache(			unsigned __int32 Instruction);
extern void r4300i_daddiu(			unsigned __int32 Instruction);
extern void r4300i_lb(				unsigned __int32 Instruction);
extern void r4300i_lbu(				unsigned __int32 Instruction);
extern void r4300i_ld(				unsigned __int32 Instruction);
extern void r4300i_ldl(				unsigned __int32 Instruction);
extern void r4300i_ldr(				unsigned __int32 Instruction);
extern void r4300i_lh(				unsigned __int32 Instruction);
extern void r4300i_lhu(				unsigned __int32 Instruction);
extern void r4300i_ll(				unsigned __int32 Instruction);
extern void r4300i_lld(				unsigned __int32 Instruction);
extern void r4300i_lw(				unsigned __int32 Instruction);
extern void r4300i_lwl(				unsigned __int32 Instruction);
extern void r4300i_lwr(				unsigned __int32 Instruction);
extern void r4300i_lwu(				unsigned __int32 Instruction); 
extern void r4300i_sb(				unsigned __int32 Instruction); 
extern void r4300i_sc(				unsigned __int32 Instruction);
extern void r4300i_scd(				unsigned __int32 Instruction);
extern void r4300i_sd(				unsigned __int32 Instruction);
extern void r4300i_sdl(				unsigned __int32 Instruction);
extern void r4300i_sdr(				unsigned __int32 Instruction);
extern void r4300i_sh(				unsigned __int32 Instruction);
extern void r4300i_sw(				unsigned __int32 Instruction);
extern void r4300i_swl(				unsigned __int32 Instruction);
extern void r4300i_swr(				unsigned __int32 Instruction);
extern void r4300i_ldc1(			unsigned __int32 Instruction);
extern void r4300i_lwc1(			unsigned __int32 Instruction);
extern void r4300i_sdc1(			unsigned __int32 Instruction);
extern void r4300i_swc1(			unsigned __int32 Instruction);
extern void r4300i_lui(				unsigned __int32 Instruction);
extern void r4300i_addiu(			unsigned __int32 Instruction);
extern void r4300i_addi(			unsigned __int32 Instruction);
extern void r4300i_andi(			unsigned __int32 Instruction);
extern void r4300i_daddi(			unsigned __int32 Instruction);
extern void r4300i_ori(				unsigned __int32 Instruction);
extern void r4300i_slti(			unsigned __int32 Instruction);
extern void r4300i_sltiu(			unsigned __int32 Instruction);
extern void r4300i_xori(			unsigned __int32 Instruction);
extern void r4300i_beq(				unsigned __int32 Instruction);
extern void r4300i_beql(			unsigned __int32 Instruction);
extern void r4300i_bne(				unsigned __int32 Instruction);
extern void r4300i_bnel(			unsigned __int32 Instruction);
extern void r4300i_jal(				unsigned __int32 Instruction);
extern void r4300i_ldc2(			unsigned __int32 Instruction);  
extern void r4300i_swc2(			unsigned __int32 Instruction);
extern void r4300i_bltz(			unsigned __int32 Instruction);
extern void r4300i_bgez(			unsigned __int32 Instruction);
extern void r4300i_bltzl(			unsigned __int32 Instruction);
extern void r4300i_bgezl(			unsigned __int32 Instruction);
extern void r4300i_tgei(			unsigned __int32 Instruction);
extern void r4300i_tgeiu(			unsigned __int32 Instruction);
extern void r4300i_tlti(			unsigned __int32 Instruction);
extern void r4300i_tltiu(			unsigned __int32 Instruction);
extern void r4300i_teqi(			unsigned __int32 Instruction);
extern void r4300i_tnei(			unsigned __int32 Instruction);
extern void r4300i_bltzal(			unsigned __int32 Instruction);
extern void r4300i_bgezal(			unsigned __int32 Instruction);
extern void r4300i_bltzall(			unsigned __int32 Instruction);
extern void r4300i_bgezall(			unsigned __int32 Instruction);
extern void r4300i_sll(				unsigned __int32 Instruction);
extern void r4300i_srl(				unsigned __int32 Instruction);
extern void r4300i_sra(				unsigned __int32 Instruction);
extern void r4300i_sllv(			unsigned __int32 Instruction);
extern void r4300i_srlv(			unsigned __int32 Instruction);
extern void r4300i_srav(			unsigned __int32 Instruction);
extern void r4300i_jr(				unsigned __int32 Instruction);
extern void r4300i_jalr(			unsigned __int32 Instruction);
extern void r4300i_syscall(			unsigned __int32 Instruction);
extern void r4300i_Break(			unsigned __int32 Instruction);
extern void r4300i_sync(			unsigned __int32 Instruction);
extern void r4300i_mfhi(			unsigned __int32 Instruction);
extern void r4300i_mthi(			unsigned __int32 Instruction);
extern void r4300i_mflo(			unsigned __int32 Instruction);
extern void r4300i_mtlo(			unsigned __int32 Instruction);
extern void r4300i_dsllv(			unsigned __int32 Instruction);
extern void r4300i_dsrlv(			unsigned __int32 Instruction);
extern void r4300i_dsrav(			unsigned __int32 Instruction);
extern void r4300i_mult(			unsigned __int32 Instruction);
extern void r4300i_multu(			unsigned __int32 Instruction);
extern void r4300i_div(				unsigned __int32 Instruction);
extern void r4300i_divu(			unsigned __int32 Instruction);
extern void r4300i_dmult(			unsigned __int32 Instruction);
extern void r4300i_dmultu(			unsigned __int32 Instruction);
extern void r4300i_ddiv(			unsigned __int32 Instruction);
extern void r4300i_ddivu(			unsigned __int32 Instruction);
extern void r4300i_add(				unsigned __int32 Instruction);
extern void r4300i_addu(			unsigned __int32 Instruction);
extern void r4300i_sub(				unsigned __int32 Instruction);
extern void r4300i_subu(			unsigned __int32 Instruction);
extern void r4300i_and(				unsigned __int32 Instruction);
extern void r4300i_or(				unsigned __int32 Instruction);
extern void r4300i_xor(				unsigned __int32 Instruction);
extern void r4300i_nor(				unsigned __int32 Instruction);
extern void r4300i_slt(				unsigned __int32 Instruction);
extern void r4300i_sltu(			unsigned __int32 Instruction);
extern void r4300i_dadd(			unsigned __int32 Instruction);
extern void r4300i_daddu(			unsigned __int32 Instruction);
extern void r4300i_dsub(			unsigned __int32 Instruction);
extern void r4300i_dsubu(			unsigned __int32 Instruction);
extern void r4300i_tge(				unsigned __int32 Instruction);
extern void r4300i_tgeu(			unsigned __int32 Instruction);
extern void r4300i_tlt(				unsigned __int32 Instruction);
extern void r4300i_tltu(			unsigned __int32 Instruction);
extern void r4300i_teq(				unsigned __int32 Instruction);
extern void r4300i_tne(				unsigned __int32 Instruction);
extern void r4300i_dsll(			unsigned __int32 Instruction);
extern void r4300i_dsrl(			unsigned __int32 Instruction);
extern void r4300i_dsra(			unsigned __int32 Instruction);
extern void r4300i_dsll32(			unsigned __int32 Instruction);
extern void r4300i_dsrl32(			unsigned __int32 Instruction);
extern void r4300i_dsra32(			unsigned __int32 Instruction);
extern void r4300i_COP0_mfc0(		unsigned __int32 Instruction);
extern void r4300i_COP0_mtc0(		unsigned __int32 Instruction);
extern void r4300i_COP1_cfc1(		unsigned __int32 Instruction);
extern void r4300i_COP1_ctc1(		unsigned __int32 Instruction);
extern void r4300i_COP1_dmfc1(		unsigned __int32 Instruction);
extern void r4300i_COP1_dmtc1(		unsigned __int32 Instruction);
extern void r4300i_COP1_mfc1(		unsigned __int32 Instruction);
extern void r4300i_COP1_mtc1(		unsigned __int32 Instruction);
extern void r4300i_COP1_add_s(		unsigned __int32 Instruction);
extern void r4300i_COP1_add_d(		unsigned __int32 Instruction);
extern void r4300i_COP1_sub_s(		unsigned __int32 Instruction);
extern void r4300i_COP1_sub_d(		unsigned __int32 Instruction);
extern void r4300i_COP1_mul_s(		unsigned __int32 Instruction);
extern void r4300i_COP1_mul_d(		unsigned __int32 Instruction);
extern void r4300i_COP1_div_s(		unsigned __int32 Instruction);
extern void r4300i_COP1_div_d(		unsigned __int32 Instruction);
extern void r4300i_COP1_sqrt_s(		unsigned __int32 Instruction);
extern void r4300i_COP1_sqrt_d(		unsigned __int32 Instruction);
extern void r4300i_COP1_abs_s(		unsigned __int32 Instruction);
extern void r4300i_COP1_abs_d(		unsigned __int32 Instruction);
extern void r4300i_COP1_mov_s(		unsigned __int32 Instruction);
extern void r4300i_COP1_mov_d(		unsigned __int32 Instruction);
extern void r4300i_COP1_neg_s(		unsigned __int32 Instruction);
extern void r4300i_COP1_neg_d(		unsigned __int32 Instruction);
extern void r4300i_COP1_roundl(		unsigned __int32 Instruction);
extern void r4300i_COP1_truncl(		unsigned __int32 Instruction);
extern void r4300i_COP1_ceill(		unsigned __int32 Instruction);
extern void r4300i_COP1_floorl(		unsigned __int32 Instruction);
extern void r4300i_COP1_roundw(		unsigned __int32 Instruction);
extern void r4300i_COP1_truncw_s(	unsigned __int32 Instruction);
extern void r4300i_COP1_truncw_d(	unsigned __int32 Instruction);
extern void r4300i_COP1_ceilw(		unsigned __int32 Instruction);
extern void r4300i_COP1_floorw(		unsigned __int32 Instruction);
extern void r4300i_COP1_cvts_d(		unsigned __int32 Instruction);
extern void r4300i_COP1_cvts_w(		unsigned __int32 Instruction);
extern void r4300i_COP1_cvts_l(		unsigned __int32 Instruction);
extern void r4300i_COP1_cvtd_s(		unsigned __int32 Instruction);
extern void r4300i_COP1_cvtd_w(		unsigned __int32 Instruction);
extern void r4300i_COP1_cvtd_l(		unsigned __int32 Instruction);
extern void r4300i_COP1_cvtw_s(		unsigned __int32 Instruction);
extern void r4300i_COP1_cvtw_d(		unsigned __int32 Instruction); 
extern void r4300i_COP1_cvtw_l(		unsigned __int32 Instruction);
extern void r4300i_COP1_cvtl_s(		unsigned __int32 Instruction); 
extern void r4300i_COP1_cvtl_d(		unsigned __int32 Instruction);
extern void r4300i_C_F_S(			unsigned __int32 Instruction);
extern void r4300i_C_F_D(			unsigned __int32 Instruction);
extern void r4300i_C_UN_S(			unsigned __int32 Instruction);
extern void r4300i_C_UN_D(			unsigned __int32 Instruction);
extern void r4300i_C_EQ_S(			unsigned __int32 Instruction);
extern void r4300i_C_EQ_D(			unsigned __int32 Instruction);
extern void r4300i_C_UEQ_S(			unsigned __int32 Instruction);
extern void r4300i_C_UEQ_D(			unsigned __int32 Instruction);
extern void r4300i_C_OLT_S(			unsigned __int32 Instruction);
extern void r4300i_C_OLT_D(			unsigned __int32 Instruction);
extern void r4300i_C_ULT_S(			unsigned __int32 Instruction);
extern void r4300i_C_ULT_D(			unsigned __int32 Instruction);
extern void r4300i_C_OLE_S(			unsigned __int32 Instruction);
extern void r4300i_C_OLE_D(			unsigned __int32 Instruction);
extern void r4300i_C_ULE_S(			unsigned __int32 Instruction);
extern void r4300i_C_ULE_D(			unsigned __int32 Instruction);
extern void r4300i_C_SF_S(			unsigned __int32 Instruction);
extern void r4300i_C_SF_D(			unsigned __int32 Instruction);
extern void r4300i_C_NGLE_S(		unsigned __int32 Instruction);
extern void r4300i_C_NGLE_D(		unsigned __int32 Instruction);
extern void r4300i_C_SEQ_S(			unsigned __int32 Instruction);
extern void r4300i_C_SEQ_D(			unsigned __int32 Instruction);
extern void r4300i_C_NGL_S(			unsigned __int32 Instruction);
extern void r4300i_C_NGL_D(			unsigned __int32 Instruction);
extern void r4300i_C_LT_S(			unsigned __int32 Instruction);
extern void r4300i_C_LT_D(			unsigned __int32 Instruction);
extern void r4300i_C_NGE_S(			unsigned __int32 Instruction);
extern void r4300i_C_NGE_D(			unsigned __int32 Instruction);
extern void r4300i_C_LE_S(			unsigned __int32 Instruction);
extern void r4300i_C_LE_D(			unsigned __int32 Instruction);
extern void r4300i_C_NGT_S(			unsigned __int32 Instruction);
extern void r4300i_C_NGT_D(			unsigned __int32 Instruction);
extern void r4300i_COP1_bc1f(		unsigned __int32 Instruction);
extern void r4300i_COP1_bc1t(		unsigned __int32 Instruction);
extern void r4300i_COP1_bc1fl(		unsigned __int32 Instruction);
extern void r4300i_COP1_bc1tl(		unsigned __int32 Instruction);
extern void r4300i_COP0_tlbr(		unsigned __int32 Instruction);
extern void r4300i_COP0_tlbwi(		unsigned __int32 Instruction);
extern void r4300i_COP0_tlbwr(		unsigned __int32 Instruction);
extern void r4300i_COP0_tlbp(		unsigned __int32 Instruction);
extern void r4300i_COP0_eret(		unsigned __int32 Instruction);
extern void r4300i_j(				unsigned __int32 Instruction);
extern void r4300i_blez(			unsigned __int32 Instruction);
extern void r4300i_bgtz(			unsigned __int32 Instruction);
extern void r4300i_blezl(			unsigned __int32 Instruction);
extern void r4300i_bgtzl(			unsigned __int32 Instruction);
extern void r4300i_break(			unsigned __int32 Instruction);
extern void r4300i_sync(			unsigned __int32 Instruction);
extern void r4300i_syscall(			unsigned __int32 Instruction);

#endif // _R4300I_H__1964_

