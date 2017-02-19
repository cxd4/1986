#include "globals.h"
#include "dynarec.h"
#include "r4300i.h"

RETURN_TYPE dyn_SPECIAL_instr(OP_PARAMS);
RETURN_TYPE dyn_REGIMM_instr(OP_PARAMS);
RETURN_TYPE dyn_COP0_instr(OP_PARAMS);
RETURN_TYPE dyn_COP1_instr(OP_PARAMS);
RETURN_TYPE dyn_TLB_instr(OP_PARAMS);
RETURN_TYPE dyn_COP1_BC_instr(OP_PARAMS);
RETURN_TYPE dyn_COP1_S_instr(OP_PARAMS);
RETURN_TYPE dyn_COP1_D_instr(OP_PARAMS);
RETURN_TYPE dyn_COP1_W_instr(OP_PARAMS);
RETURN_TYPE dyn_COP1_L_instr(OP_PARAMS);
RETURN_TYPE dyn_UNUSED(OP_PARAMS);

//---------------------------------------------------------------------------------------

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
RETURN_TYPE (*dyn_CPU_instruction[64])(OP_PARAMS)=
{
	dyn_SPECIAL_instr,	dyn_REGIMM_instr,	dyn_r4300i_j,	dyn_r4300i_jal,		dyn_r4300i_beq,	dyn_r4300i_bne,	dyn_r4300i_blez,	dyn_r4300i_bgtz,
	dyn_r4300i_addi,	dyn_r4300i_addiu,	dyn_r4300i_slti,dyn_r4300i_sltiu,	dyn_r4300i_andi,dyn_r4300i_ori,	dyn_r4300i_xori,	dyn_r4300i_lui,
	dyn_COP0_instr,		dyn_COP1_instr,		dyn_UNUSED,		dyn_UNUSED,			dyn_r4300i_beql,dyn_r4300i_bnel,dyn_r4300i_blezl,	dyn_r4300i_bgtzl,
	dyn_r4300i_daddi,	dyn_r4300i_daddiu,	dyn_r4300i_ldl,	dyn_r4300i_ldr,		dyn_UNUSED,		dyn_UNUSED,		dyn_UNUSED,			dyn_UNUSED,
	dyn_r4300i_lb,		dyn_r4300i_lh,		dyn_r4300i_lwl,	dyn_r4300i_lw,		dyn_r4300i_lbu,	dyn_r4300i_lhu,	dyn_r4300i_lwr,		dyn_r4300i_lwu,
	dyn_r4300i_sb,		dyn_r4300i_sh,		dyn_r4300i_swl,	dyn_r4300i_sw,		dyn_r4300i_sdl,	dyn_r4300i_sdr,	dyn_r4300i_swr,		dyn_r4300i_cache,
	dyn_r4300i_ll,		dyn_r4300i_lwc1,	dyn_UNUSED,		dyn_UNUSED,			dyn_r4300i_lld,	dyn_r4300i_ldc1,dyn_UNUSED,			dyn_r4300i_ld,
	dyn_r4300i_sc,		dyn_r4300i_swc1,	dyn_UNUSED,		dyn_UNUSED,			dyn_r4300i_scd,	dyn_r4300i_sdc1,dyn_UNUSED,			dyn_r4300i_sd
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
RETURN_TYPE (*dyn_SPECIAL_Instruction[64])(OP_PARAMS)=
{
	dyn_r4300i_sll,	dyn_UNUSED,			dyn_r4300i_srl,	dyn_r4300i_sra,	dyn_r4300i_sllv,	dyn_UNUSED,			dyn_r4300i_srlv,	dyn_r4300i_srav,
	dyn_r4300i_jr,	dyn_r4300i_jalr,	dyn_UNUSED,		dyn_UNUSED,		dyn_r4300i_syscall,	dyn_r4300i_break,	dyn_UNUSED,			dyn_r4300i_sync,
	dyn_r4300i_mfhi,dyn_r4300i_mthi,	dyn_r4300i_mflo,dyn_r4300i_mtlo,dyn_r4300i_dsllv,	dyn_UNUSED,			dyn_r4300i_dsrlv,	dyn_r4300i_dsrav,
	dyn_r4300i_mult,dyn_r4300i_multu,	dyn_r4300i_div,	dyn_r4300i_divu,dyn_r4300i_dmult,	dyn_r4300i_dmultu,	dyn_r4300i_ddiv,	dyn_r4300i_ddivu,	
	dyn_r4300i_add,	dyn_r4300i_addu,	dyn_r4300i_sub,	dyn_r4300i_subu,dyn_r4300i_and,		dyn_r4300i_or,		dyn_r4300i_xor,		dyn_r4300i_nor,
	dyn_UNUSED,		dyn_UNUSED,			dyn_r4300i_slt,	dyn_r4300i_sltu,dyn_r4300i_dadd,	dyn_r4300i_daddu,	dyn_r4300i_dsub,	dyn_r4300i_dsubu,
	dyn_r4300i_tge,	dyn_r4300i_tgeu,	dyn_r4300i_tlt,	dyn_r4300i_tltu,dyn_r4300i_teq,		dyn_UNUSED,			dyn_r4300i_tne,		dyn_UNUSED,
	dyn_r4300i_dsll,dyn_UNUSED,			dyn_r4300i_dsrl,dyn_r4300i_dsra,dyn_r4300i_dsll32,	dyn_UNUSED,			dyn_r4300i_dsrl32,	dyn_r4300i_dsra32
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
RETURN_TYPE (*dyn_REGIMM_Instruction[32])(OP_PARAMS)=
{
	dyn_r4300i_bltz,	dyn_r4300i_bgez,	dyn_r4300i_bltzl,	dyn_r4300i_bgezl,	dyn_UNUSED,			dyn_UNUSED,	dyn_UNUSED,			dyn_UNUSED,
	dyn_r4300i_tgei,	dyn_r4300i_tgeiu,	dyn_r4300i_tlti,	dyn_r4300i_tltiu,	dyn_r4300i_teqi,	dyn_UNUSED,	dyn_r4300i_tnei,	dyn_UNUSED,
	dyn_r4300i_bltzal,	dyn_r4300i_bgezal,	dyn_r4300i_bltzall,	dyn_r4300i_bgezall,	dyn_UNUSED,			dyn_UNUSED,	dyn_UNUSED,			dyn_UNUSED,
	dyn_UNUSED,			dyn_UNUSED,			dyn_UNUSED,			dyn_UNUSED,			dyn_UNUSED,			dyn_UNUSED,	dyn_UNUSED,			dyn_UNUSED	
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
RETURN_TYPE (*dyn_COP0_Instruction[32])(OP_PARAMS)=
{
	dyn_r4300i_COP0_mfc0,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_r4300i_COP0_mtc0,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	
	dyn_TLB_instr,			dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED
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
RETURN_TYPE (*dyn_TLB_Instruction[64])(OP_PARAMS)=
{
	dyn_UNUSED,						dyn_r4300i_COP0_tlbr,	dyn_r4300i_COP0_tlbwi,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_r4300i_COP0_tlbwr,	dyn_UNUSED,
	dyn_r4300i_COP0_tlbp,			dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,				dyn_UNUSED,
	dyn_UNUSED,						dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,				dyn_UNUSED,
	dyn_r4300i_COP0_eret,			dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,				dyn_UNUSED,
	dyn_UNUSED,						dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,				dyn_UNUSED,
	dyn_UNUSED,						dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,				dyn_UNUSED,
	dyn_UNUSED,						dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,				dyn_UNUSED,
	dyn_UNUSED,						dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,				dyn_UNUSED
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
RETURN_TYPE (*dyn_COP1_Instruction[32])(OP_PARAMS)=
{
	dyn_r4300i_COP1_mfc1,	dyn_r4300i_COP1_dmfc1,	dyn_r4300i_COP1_cfc1,	dyn_UNUSED,	dyn_r4300i_COP1_mtc1,	dyn_r4300i_COP1_dmtc1,	dyn_r4300i_COP1_ctc1,	dyn_UNUSED,
	dyn_COP1_BC_instr,		dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,
	dyn_COP1_S_instr,		dyn_COP1_D_instr,		dyn_UNUSED,				dyn_UNUSED,	dyn_COP1_W_instr,		dyn_COP1_L_instr,		dyn_UNUSED,				dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED
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
RETURN_TYPE (*dyn_COP1_BC_Instruction[4])(OP_PARAMS)=
{
	dyn_r4300i_COP1_bc1f,	dyn_r4300i_COP1_bc1t,
	dyn_r4300i_COP1_bc1fl,	dyn_r4300i_COP1_bc1tl
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
RETURN_TYPE (*dyn_COP1_S_Instruction[64])(OP_PARAMS)=
{
	dyn_r4300i_COP1_add_s,	dyn_r4300i_COP1_sub_s,	dyn_r4300i_COP1_mul_s,	dyn_r4300i_COP1_div_s,	dyn_r4300i_COP1_sqrt_s,	dyn_r4300i_COP1_abs_s,		dyn_r4300i_COP1_mov_s,	dyn_r4300i_COP1_neg_s,
	dyn_r4300i_COP1_roundl,	dyn_r4300i_COP1_truncl,	dyn_r4300i_COP1_ceill,	dyn_r4300i_COP1_floorl,	dyn_r4300i_COP1_roundw,	dyn_r4300i_COP1_truncw_s,	dyn_r4300i_COP1_ceilw,	dyn_r4300i_COP1_floorw,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,					dyn_UNUSED,				dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,					dyn_UNUSED,				dyn_UNUSED,
	dyn_UNUSED,				dyn_r4300i_COP1_cvtd_s,	dyn_UNUSED,				dyn_UNUSED,				dyn_r4300i_COP1_cvtw_s,	dyn_r4300i_COP1_cvtl_s,		dyn_UNUSED,				dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,					dyn_UNUSED,				dyn_UNUSED,
	dyn_r4300i_C_F_S,		dyn_r4300i_C_UN_S,		dyn_r4300i_C_EQ_S,		dyn_r4300i_C_UEQ_S,		dyn_r4300i_C_OLT_S,		dyn_r4300i_C_ULT_S,			dyn_r4300i_C_OLE_S,		dyn_r4300i_C_ULE_S,
	dyn_r4300i_C_SF_S,		dyn_r4300i_C_NGLE_S,	dyn_r4300i_C_SEQ_S,		dyn_r4300i_C_NGL_S,		dyn_r4300i_C_LT_S,		dyn_r4300i_C_NGE_S,			dyn_r4300i_C_LE_S,		dyn_r4300i_C_NGT_S
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
RETURN_TYPE (*dyn_COP1_D_Instruction[64])(OP_PARAMS)=
{
	dyn_r4300i_COP1_add_d,	dyn_r4300i_COP1_sub_d,	dyn_r4300i_COP1_mul_d,	dyn_r4300i_COP1_div_d,	dyn_r4300i_COP1_sqrt_d,	dyn_r4300i_COP1_abs_d,		dyn_r4300i_COP1_mov_d,	dyn_r4300i_COP1_neg_d,
	dyn_r4300i_COP1_roundl,	dyn_r4300i_COP1_truncl,	dyn_r4300i_COP1_ceill,	dyn_r4300i_COP1_floorl,	dyn_r4300i_COP1_roundw,	dyn_r4300i_COP1_truncw_d,	dyn_r4300i_COP1_ceilw,	dyn_r4300i_COP1_floorw,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,					dyn_UNUSED,				dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,					dyn_UNUSED,				dyn_UNUSED,
	dyn_r4300i_COP1_cvts_d,	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_r4300i_COP1_cvtw_d,	dyn_r4300i_COP1_cvtl_d,		dyn_UNUSED,				dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,					dyn_UNUSED,				dyn_UNUSED,
	dyn_r4300i_C_F_D,		dyn_r4300i_C_UN_D,		dyn_r4300i_C_EQ_D,		dyn_r4300i_C_UEQ_D,		dyn_r4300i_C_OLT_D,		dyn_r4300i_C_ULT_D,			dyn_r4300i_C_OLE_D,		dyn_r4300i_C_ULE_D,
	dyn_r4300i_C_SF_D,		dyn_r4300i_C_NGLE_D,	dyn_r4300i_C_SEQ_D,		dyn_r4300i_C_NGL_D,		dyn_r4300i_C_LT_D,		dyn_r4300i_C_NGE_D,			dyn_r4300i_C_LE_D,		dyn_r4300i_C_NGT_D
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
RETURN_TYPE (*dyn_COP1_W_Instruction[64])(OP_PARAMS)=
{
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,
	dyn_r4300i_COP1_cvts_w,	dyn_r4300i_COP1_cvtd_w,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED
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
RETURN_TYPE (*dyn_COP1_L_Instruction[64])(OP_PARAMS)=
{
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,
	dyn_r4300i_COP1_cvts_l,	dyn_r4300i_COP1_cvtd_l,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,
	dyn_UNUSED,				dyn_UNUSED,				dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED,	dyn_UNUSED
};

//---------------------------------------------------------------------------------------

RETURN_TYPE dyn_SPECIAL_instr(OP_PARAMS){	Dest = dyn_SPECIAL_Instruction[_FUNCTION_](PASS_PARAMS);		return(Dest);}
RETURN_TYPE dyn_REGIMM_instr(OP_PARAMS)	{	Dest = dyn_REGIMM_Instruction[RT_FT](PASS_PARAMS);				return(Dest);}
RETURN_TYPE dyn_COP0_instr(OP_PARAMS)	{	Dest = dyn_COP0_Instruction[RS_BASE_FMT](PASS_PARAMS);			return(Dest);}
RETURN_TYPE dyn_COP1_instr(OP_PARAMS)	{	Dest = dyn_COP1_Instruction[RS_BASE_FMT](PASS_PARAMS);			return(Dest);}
RETURN_TYPE dyn_TLB_instr(OP_PARAMS)	{	Dest = dyn_TLB_Instruction[_FUNCTION_](PASS_PARAMS);			return(Dest);}
RETURN_TYPE dyn_COP1_BC_instr(OP_PARAMS){	Dest = dyn_COP1_BC_Instruction[(_ND_ << 1) | _TF_](PASS_PARAMS);return(Dest);}
RETURN_TYPE dyn_COP1_S_instr(OP_PARAMS)	{	Dest = dyn_COP1_S_Instruction[_FUNCTION_](PASS_PARAMS);			return(Dest);}
RETURN_TYPE dyn_COP1_D_instr(OP_PARAMS)	{	Dest = dyn_COP1_D_Instruction[_FUNCTION_](PASS_PARAMS);			return(Dest);}
RETURN_TYPE dyn_COP1_W_instr(OP_PARAMS)	{	Dest = dyn_COP1_W_Instruction[_FUNCTION_](PASS_PARAMS);			return(Dest);}
RETURN_TYPE dyn_COP1_L_instr(OP_PARAMS)	{	Dest = dyn_COP1_L_Instruction[_FUNCTION_](PASS_PARAMS);			return(Dest);}