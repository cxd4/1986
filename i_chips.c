/*______________________________________________________________________________
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


This project is a documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar. Many source
code comments are taken from anarko's n64toolkit with consent and are 
the property of anarko.
*/

#include "r4300i.h"
#include "globals.h"

void SPECIAL_instr(uint32 Instruction);
void REGIMM_instr(uint32 Instruction);
void COP0_instr(uint32 Instruction);
void COP1_instr(uint32 Instruction);
void TLB_instr(uint32 Instruction);
void COP1_BC_instr(uint32 Instruction);
void COP1_S_instr(uint32 Instruction);
void COP1_D_instr(uint32 Instruction);
void COP1_W_instr(uint32 Instruction);
void COP1_L_instr(uint32 Instruction);


/*
****************************************************************************
** Main CPU                                                               **
****************************************************************************
  
    CPU: Instructions encoded by opcode field.
    31---------26---------------------------------------------------0
    |  opcode   |                                                   |
    ------6----------------------------------------------------------
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
000 | *1    | *2    | J     | JAL   | BEQ   | BNE   | BLEZ  | BGTZ  |
001 | ADDI  | ADDIU | SLTI  | SLTIU | ANDI  | ORI   | XORI  | LUI   |
010 | *3    | *4    |  ---  |  ---  | BEQL  | BNEL  | BLEZL | BGTZL |
011 | DADDI |DADDIU | LDL   | LDR   |  ---  |  ---  |  ---  |  ---  |
100 | LB    | LH    | LWL   | LW    | LBU   | LHU   | LWR   | LWU   |
101 | SB    | SH    | SWL   | SW    | SDL   | SDR   | SWR   | CACHE |
110 | LL    | LWC1  |  ---  |  ---  | LLD   | LDC1  | (LDC2)| LD    |
111 | SC    | SWC1  |  ---  |  ---  | SCD   | SDC1  | (SDC2)| SD    |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
     *1 = SPECIAL, see SPECIAL list    *2 = REGIMM, see REGIMM list
     *3 = COP0                         *4 = COP1
*/
void (*CPU_instruction[64])(uint32 Instruction)=
{
    SPECIAL_instr,  REGIMM_instr,   r4300i_j,   r4300i_jal,     r4300i_beq, r4300i_bne, r4300i_blez,    r4300i_bgtz,
    r4300i_addi,    r4300i_addiu,   r4300i_slti,r4300i_sltiu,   r4300i_andi,r4300i_ori, r4300i_xori,    r4300i_lui,
    COP0_instr,     COP1_instr,     UNUSED,     UNUSED,         r4300i_beql,r4300i_bnel,r4300i_blezl,   r4300i_bgtzl,
    r4300i_daddi,   r4300i_daddiu,  r4300i_ldl, r4300i_ldr,     UNUSED,     UNUSED,     UNUSED,         UNUSED,
    r4300i_lb,      r4300i_lh,      r4300i_lwl, r4300i_lw,      r4300i_lbu, r4300i_lhu, r4300i_lwr,     r4300i_lwu,
    r4300i_sb,      r4300i_sh,      r4300i_swl, r4300i_sw,      r4300i_sdl, r4300i_sdr, r4300i_swr,     r4300i_cache,
    r4300i_ll,      r4300i_lwc1,    UNUSED,     UNUSED,         r4300i_lld, r4300i_ldc1,UNUSED,         r4300i_ld,
    r4300i_sc,      r4300i_swc1,    UNUSED,     UNUSED,         r4300i_scd, r4300i_sdc1,UNUSED,         r4300i_sd
};

//---------------------------------------------------------------------------------------

/*
    SPECIAL: Instr. encoded by function field when opcode field = SPECIAL.
    31---------26------------------------------------------5--------0
    | = SPECIAL |                                         | function|
    ------6----------------------------------------------------6-----
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
000 | SLL   |  ---  | SRL   | SRA   | SLLV  |  ---  | SRLV  | SRAV  |
001 | JR    | JALR  |  ---  |  ---  |SYSCALL| BREAK |  ---  | SYNC  |
010 | MFHI  | MTHI  | MFLO  | MTLO  | DSLLV |  ---  | DSRLV | DSRAV |
011 | MULT  | MULTU | DIV   | DIVU  | DMULT | DMULTU| DDIV  | DDIVU |
100 | ADD   | ADDU  | SUB   | SUBU  | AND   | OR    | XOR   | NOR   |
101 |  ---  |  ---  | SLT   | SLTU  | DADD  | DADDU | DSUB  | DSUBU |
110 | TGE   | TGEU  | TLT   | TLTU  | TEQ   |  ---  | TNE   |  ---  |
111 | DSLL  |  ---  | DSRL  | DSRA  |DSLL32 |  ---  |DSRL32 |DSRA32 |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
*/
void (*SPECIAL_Instruction[64])(uint32 Instruction)=
{
    r4300i_sll, UNUSED,         r4300i_srl, r4300i_sra, r4300i_sllv,    UNUSED,         r4300i_srlv,    r4300i_srav,
    r4300i_jr,  r4300i_jalr,    UNUSED,     UNUSED,     r4300i_syscall, r4300i_break,   UNUSED,         r4300i_sync,
    r4300i_mfhi,r4300i_mthi,    r4300i_mflo,r4300i_mtlo,r4300i_dsllv,   UNUSED,         r4300i_dsrlv,   r4300i_dsrav,
    r4300i_mult,r4300i_multu,   r4300i_div, r4300i_divu,r4300i_dmult,   r4300i_dmultu,  r4300i_ddiv,    r4300i_ddivu,   
    r4300i_add, r4300i_addu,    r4300i_sub, r4300i_subu,r4300i_and,     r4300i_or,      r4300i_xor,     r4300i_nor,
    UNUSED,     UNUSED,         r4300i_slt, r4300i_sltu,r4300i_dadd,    r4300i_daddu,   r4300i_dsub,    r4300i_dsubu,
    r4300i_tge, r4300i_tgeu,    r4300i_tlt, r4300i_tltu,r4300i_teq,     UNUSED,         r4300i_tne,     UNUSED,
    r4300i_dsll,UNUSED,         r4300i_dsrl,r4300i_dsra,r4300i_dsll32,  UNUSED,         r4300i_dsrl32,  r4300i_dsra32
};


//---------------------------------------------------------------------------------------

/*
    REGIMM: Instructions encoded by the rt field when opcode field = REGIMM.
    31---------26----------20-------16------------------------------0
    | = REGIMM  |          |   rt    |                              |
    ------6---------------------5------------------------------------
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
 00 | BLTZ  | BGEZ  | BLTZL | BGEZL |  ---  |  ---  |  ---  |  ---  |
 01 | TGEI  | TGEIU | TLTI  | TLTIU | TEQI  |  ---  | TNEI  |  ---  |
 10 | BLTZAL| BGEZAL|BLTZALL|BGEZALL|  ---  |  ---  |  ---  |  ---  |
 11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
*/
void (*REGIMM_Instruction[32])(uint32 Instruction)=
{
    r4300i_bltz,    r4300i_bgez,    r4300i_bltzl,   r4300i_bgezl,   UNUSED,         UNUSED, UNUSED,         UNUSED,
    r4300i_tgei,    r4300i_tgeiu,   r4300i_tlti,    r4300i_tltiu,   r4300i_teqi,    UNUSED, r4300i_tnei,    UNUSED,
    r4300i_bltzal,  r4300i_bgezal,  r4300i_bltzall, r4300i_bgezall, UNUSED,         UNUSED, UNUSED,         UNUSED,
    UNUSED,         UNUSED,         UNUSED,         UNUSED,         UNUSED,         UNUSED, UNUSED,         UNUSED  
};

//---------------------------------------------------------------------------------------

/*
****************************************************************************
** COP0                                                                   **
****************************************************************************

    COP0: Instructions encoded by the fmt field when opcode = COP0.
    31--------26-25------21 ----------------------------------------0
    |  = COP0   |   fmt   |                                         |
    ------6----------5-----------------------------------------------
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
 00 | MFC0  |  ---  |  ---  |  ---  | MTC0  |  ---  |  ---  |  ---  |
 01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 10 | *1    |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
     *1 = TLB instr, see TLB list
*/
void (*COP0_Instruction[32])(uint32 Instruction)=
{
    r4300i_COP0_mfc0,   UNUSED, UNUSED, UNUSED, r4300i_COP0_mtc0,   UNUSED, UNUSED, UNUSED,
    UNUSED,             UNUSED, UNUSED, UNUSED, UNUSED,             UNUSED, UNUSED, UNUSED, 
    TLB_instr,          UNUSED, UNUSED, UNUSED, UNUSED,             UNUSED, UNUSED, UNUSED,
    UNUSED,             UNUSED, UNUSED, UNUSED, UNUSED,             UNUSED, UNUSED, UNUSED
};

//---------------------------------------------------------------------------------------

/*
    TLB: Instructions encoded by the function field when opcode
         = COP0 and fmt = TLB.
    31--------26-25------21 -------------------------------5--------0
    |  = COP0   |  = TLB  |                               | function|
    ------6----------5-----------------------------------------6-----
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
000 |  ---  | TLBR  | TLBWI |  ---  |  ---  |  ---  | TLBWR |  ---  |
001 | TLBP  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
011 | ERET  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
100 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
101 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
111 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
*/
void (*TLB_Instruction[64])(uint32 Instruction)=
{
    UNUSED,                     r4300i_COP0_tlbr,   r4300i_COP0_tlbwi,  UNUSED, UNUSED, UNUSED, r4300i_COP0_tlbwr,  UNUSED,
    r4300i_COP0_tlbp,           UNUSED,             UNUSED,             UNUSED, UNUSED, UNUSED, UNUSED,             UNUSED,
    UNUSED,                     UNUSED,             UNUSED,             UNUSED, UNUSED, UNUSED, UNUSED,             UNUSED,
    r4300i_COP0_eret,           UNUSED,             UNUSED,             UNUSED, UNUSED, UNUSED, UNUSED,             UNUSED,
    UNUSED,                     UNUSED,             UNUSED,             UNUSED, UNUSED, UNUSED, UNUSED,             UNUSED,
    UNUSED,                     UNUSED,             UNUSED,             UNUSED, UNUSED, UNUSED, UNUSED,             UNUSED,
    UNUSED,                     UNUSED,             UNUSED,             UNUSED, UNUSED, UNUSED, UNUSED,             UNUSED,
    UNUSED,                     UNUSED,             UNUSED,             UNUSED, UNUSED, UNUSED, UNUSED,             UNUSED
};

//---------------------------------------------------------------------------------------

/*
****************************************************************************
** COP1 - Floating Point Unit (FPU)                                       **
****************************************************************************

    COP1: Instructions encoded by the fmt field when opcode = COP1.
    31--------26-25------21 ----------------------------------------0
    |  = COP1   |   fmt   |                                         |
    ------6----------5-----------------------------------------------
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
 00 | MFC1  | DMFC1 | CFC1  |  ---  | MTC1  | DMTC1 | CTC1  |  ---  |
 01 | *1    |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 10 | *2    | *3    |  ---  |  ---  | *4    | *5    |  ---  |  ---  |
 11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
     *1 = BC instructions, see BC1 list
     *2 = S instr, see FPU list            *3 = D instr, see FPU list
     *4 = W instr, see FPU list            *5 = L instr, see FPU list
*/
void (*COP1_Instruction[32])(uint32 Instruction)=
{
    r4300i_COP1_mfc1,   r4300i_COP1_dmfc1,  r4300i_COP1_cfc1,   UNUSED, 
    r4300i_COP1_mtc1,   r4300i_COP1_dmtc1,  r4300i_COP1_ctc1,   UNUSED,
    COP1_BC_instr,      UNUSED,             UNUSED,             UNUSED, 
    UNUSED,             UNUSED,             UNUSED,             UNUSED,
    COP1_S_instr,       COP1_D_instr,       UNUSED,             UNUSED,
    COP1_W_instr,       COP1_L_instr,       UNUSED,             UNUSED,
    UNUSED,             UNUSED,             UNUSED,             UNUSED,
    UNUSED,             UNUSED,             UNUSED,             UNUSED
};


//---------------------------------------------------------------------------------------

/*
    BC1: Instructions encoded by the nd and tf fields when opcode
         = COP1 and fmt = BC
    31--------26-25------21 ---17--16-------------------------------0
    |  = COP1   |  = BC   |    |nd|tf|                              |
    ------6----------5-----------1--1--------------------------------
    |---0---|---1---| tf
  0 | BC1F  | BC1T  |
  1 | BC1FL | BC1TL |
 nd |-------|-------|
*/
void (*COP1_BC_Instruction[4])(uint32 Instruction)=
{
    r4300i_COP1_bc1f,   r4300i_COP1_bc1t,
    r4300i_COP1_bc1fl,  r4300i_COP1_bc1tl
};

//---------------------------------------------------------------------------------------

/*
    FPU: Instructions encoded by the function field when opcode = COP1
         and fmt = S
    31--------26-25------21 -------------------------------5--------0
    |  = COP1   |  = S    |                               | function|
    ------6----------5-----------------------------------------6-----
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
000 | ADD   | SUB   | MUL   | DIV   | SQRT  | ABS   | MOV   | NEG   |
001 |ROUND.L|TRUNC.L| CEIL.L|FLOOR.L|ROUND.W|TRUNC.W| CEIL.W|FLOOR.W|
010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
011 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
100 |  ---  | CVT.D |  ---  |  ---  | CVT.W | CVT.L |  ---  |  ---  |
101 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
110 | C.F   | C.UN  | C.EQ  | C.UEQ | C.OLT | C.ULT | C.OLE | C.ULE |
111 | C.SF  | C.NGLE| C.SEQ | C.NGL | C.LT  | C.NGE | C.LE  | C.NGT |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
*/
void (*COP1_S_Instruction[64])(uint32 Instruction)=
{
    r4300i_COP1_add_s,    r4300i_COP1_sub_s,    r4300i_COP1_mul_s,    r4300i_COP1_div_s,    
    r4300i_COP1_sqrt_s,   r4300i_COP1_abs_s,    r4300i_COP1_mov_s,    r4300i_COP1_neg_s,
    r4300i_COP1_roundl_s, r4300i_COP1_truncl_s, r4300i_COP1_ceill_s,  r4300i_COP1_floorl_s,
    r4300i_COP1_roundw_s, r4300i_COP1_truncw_s, r4300i_COP1_ceilw_s,  r4300i_COP1_floorw_s,
    UNUSED,               UNUSED,               UNUSED,               UNUSED,
    UNUSED,               UNUSED,               UNUSED,               UNUSED,
    UNUSED,               UNUSED,               UNUSED,               UNUSED,
    UNUSED,               UNUSED,               UNUSED,               UNUSED,
    UNUSED,               r4300i_COP1_cvtd_s,   UNUSED,               UNUSED,
    r4300i_COP1_cvtw_s,   r4300i_COP1_cvtl_s,   UNUSED,               UNUSED,
    UNUSED,               UNUSED,               UNUSED,               UNUSED,
    UNUSED,               UNUSED,               UNUSED,               UNUSED,
    
    r4300i_C_F_S,         r4300i_C_UN_S,        r4300i_C_EQ_S,        r4300i_C_UEQ_S,
    r4300i_C_OLT_S,       r4300i_C_ULT_S,       r4300i_C_OLE_S,       r4300i_C_ULE_S,
    r4300i_C_SF_S,        r4300i_C_NGLE_S,      r4300i_C_SEQ_S,       r4300i_C_NGL_S,
    r4300i_C_LT_S,        r4300i_C_NGE_S,       r4300i_C_LE_S,        r4300i_C_NGT_S
};

//---------------------------------------------------------------------------------------

/*
    FPU: Instructions encoded by the function field when opcode = COP1
         and fmt = D
    31--------26-25------21 -------------------------------5--------0
    |  = COP1   |  = D    |                               | function|
    ------6----------5-----------------------------------------6-----
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
000 | ADD   | SUB   | MUL   | DIV   | SQRT  | ABS   | MOV   | NEG   |
001 |ROUND.L|TRUNC.L| CEIL.L|FLOOR.L|ROUND.W|TRUNC.W| CEIL.W|FLOOR.W|
010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
011 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
100 | CVT.S |  ---  |  ---  |  ---  | CVT.W | CVT.L |  ---  |  ---  |
101 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
110 | C.F   | C.UN  | C.EQ  | C.UEQ | C.OLT | C.ULT | C.OLE | C.ULE |
111 | C.SF  | C.NGLE| C.SEQ | C.NGL | C.LT  | C.NGE | C.LE  | C.NGT |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
*/
void (*COP1_D_Instruction[64])(uint32 Instruction)=
{
    r4300i_COP1_add_d,    r4300i_COP1_sub_d,    r4300i_COP1_mul_d,    r4300i_COP1_div_d,
    r4300i_COP1_sqrt_d,   r4300i_COP1_abs_d,    r4300i_COP1_mov_d,    r4300i_COP1_neg_d,
    r4300i_COP1_roundl_d, r4300i_COP1_truncl_d, r4300i_COP1_ceill_d,  r4300i_COP1_floorl_d, 
    r4300i_COP1_roundw_d, r4300i_COP1_truncw_d, r4300i_COP1_ceilw_d,  r4300i_COP1_floorw_d,
    UNUSED,               UNUSED,               UNUSED,               UNUSED,
    UNUSED,               UNUSED,               UNUSED,               UNUSED,
    UNUSED,               UNUSED,               UNUSED,               UNUSED,
    UNUSED,               UNUSED,               UNUSED,               UNUSED,
    r4300i_COP1_cvts_d,   UNUSED,               UNUSED,               UNUSED,
    r4300i_COP1_cvtw_d,   r4300i_COP1_cvtl_d,   UNUSED,               UNUSED,
    UNUSED,               UNUSED,               UNUSED,               UNUSED,
    UNUSED,               UNUSED,               UNUSED,               UNUSED,
    
    r4300i_C_F_D,         r4300i_C_UN_D,        r4300i_C_EQ_D,        r4300i_C_UEQ_D,
    r4300i_C_OLT_D,       r4300i_C_ULT_D,       r4300i_C_OLE_D,       r4300i_C_ULE_D,
    r4300i_C_SF_D,        r4300i_C_NGLE_D,      r4300i_C_SEQ_D,       r4300i_C_NGL_D,
    r4300i_C_LT_D,        r4300i_C_NGE_D,       r4300i_C_LE_D,        r4300i_C_NGT_D
};

//---------------------------------------------------------------------------------------

/*
    FPU: Instructions encoded by the function field when opcode = COP1
         and fmt = W
    31--------26-25------21 -------------------------------5--------0
    |  = COP1   |  = W    |                               | function|
    ------6----------5-----------------------------------------6-----
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
000 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
001 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
011 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
100 | CVT.S | CVT.D |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
101 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
111 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
*/
void (*COP1_W_Instruction[64])(uint32 Instruction)=
{
    UNUSED,             UNUSED,             UNUSED, UNUSED,
    UNUSED,             UNUSED,             UNUSED, UNUSED,
    UNUSED,             UNUSED,             UNUSED, UNUSED, 
    UNUSED,             UNUSED,             UNUSED, UNUSED,
    UNUSED,             UNUSED,             UNUSED, UNUSED, 
    UNUSED,             UNUSED,             UNUSED, UNUSED,
    UNUSED,             UNUSED,             UNUSED, UNUSED, 
    UNUSED,             UNUSED,             UNUSED, UNUSED,
    r4300i_COP1_cvts_w, r4300i_COP1_cvtd_w, UNUSED, UNUSED, 
    UNUSED,             UNUSED,             UNUSED, UNUSED,
    UNUSED,             UNUSED,             UNUSED, UNUSED, 
    UNUSED,             UNUSED,             UNUSED, UNUSED,
    UNUSED,             UNUSED,             UNUSED, UNUSED, 
    UNUSED,             UNUSED,             UNUSED, UNUSED,
    UNUSED,             UNUSED,             UNUSED, UNUSED, 
    UNUSED,             UNUSED,             UNUSED, UNUSED
};

//---------------------------------------------------------------------------------------

/*
    FPU: Instructions encoded by the function field when opcode = COP1
         and fmt = L
    31--------26-25------21 -------------------------------5--------0
    |  = COP1   |  = L    |                               | function|
    ------6----------5-----------------------------------------6-----
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
000 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
001 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
011 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
100 | CVT.S | CVT.D |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
101 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
111 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
*/
void (*COP1_L_Instruction[64])(uint32 Instruction)=
{
    UNUSED,             UNUSED,             UNUSED, UNUSED,
    UNUSED,             UNUSED,             UNUSED, UNUSED,
    UNUSED,             UNUSED,             UNUSED, UNUSED, 
    UNUSED,             UNUSED,             UNUSED, UNUSED,
    UNUSED,             UNUSED,             UNUSED, UNUSED, 
    UNUSED,             UNUSED,             UNUSED, UNUSED,
    UNUSED,             UNUSED,             UNUSED, UNUSED, 
    UNUSED,             UNUSED,             UNUSED, UNUSED,
    r4300i_COP1_cvts_l, r4300i_COP1_cvtd_l, UNUSED, UNUSED, 
    UNUSED,             UNUSED,             UNUSED, UNUSED,
    UNUSED,             UNUSED,             UNUSED, UNUSED, 
    UNUSED,             UNUSED,             UNUSED, UNUSED,
    UNUSED,             UNUSED,             UNUSED, UNUSED, 
    UNUSED,             UNUSED,             UNUSED, UNUSED,
    UNUSED,             UNUSED,             UNUSED, UNUSED, 
    UNUSED,             UNUSED,             UNUSED, UNUSED
};

//---------------------------------------------------------------------------------------

void SPECIAL_instr(uint32 Instruction)  {   SPECIAL_Instruction[_FUNCTION_](Instruction);           }
void REGIMM_instr(uint32 Instruction)   {   REGIMM_Instruction[RT_FT](Instruction);                 }
void COP0_instr(uint32 Instruction)     {   COP0_Instruction[RS_BASE_FMT](Instruction);             }
void COP1_instr(uint32 Instruction)     {   COP1_Instruction[RS_BASE_FMT](Instruction);             }
void TLB_instr(uint32 Instruction)      {   TLB_Instruction[_FUNCTION_](Instruction);               }
void COP1_BC_instr(uint32 Instruction)  {   COP1_BC_Instruction[(_ND_ << 1) | _TF_](Instruction);   }
void COP1_S_instr(uint32 Instruction)   {   COP1_S_Instruction[_FUNCTION_](Instruction);            }
void COP1_D_instr(uint32 Instruction)   {   COP1_D_Instruction[_FUNCTION_](Instruction);            }
void COP1_W_instr(uint32 Instruction)   {   COP1_W_Instruction[_FUNCTION_](Instruction);            }
void COP1_L_instr(uint32 Instruction)   {   COP1_L_Instruction[_FUNCTION_](Instruction);            }
