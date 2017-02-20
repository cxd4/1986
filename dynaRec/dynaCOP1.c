#define _SAFTY_COP1_(x)
//#define _SAFTY_COP1_(x)  INTERPRET(x); return;


#include <math.h> 
#include <float.h>

#include "../options.h"
#include "../hardware.h"
#include "../r4300i.h"
#include "regcache.h"

#include "dynaCOP1.h"
#include "dynaCPU.h"
#include "x86.h"

extern void dyna4300i_reserved(OP_PARAMS);
extern void HELP_cop1_bc1f(_u32 pc);
extern void HELP_cop1_bc1t(_u32 pc);
extern void HELP_cop1_bc1fl(_u32 pc);
extern void HELP_cop1_bc1tl(_u32 pc);
extern void HELP_Call(unsigned long function);
extern void StoreMipsCpuRegister(unsigned long iMipsReg,unsigned char iIntelReg1,unsigned char iIntelReg2);
extern void LoadMipsCpuRegister(unsigned long iMipsReg,unsigned char iIntelReg1,unsigned char iIntelReg2);
extern void HELP_CheckExceptionSlot(unsigned long pc);

extern MapConstant ConstMap[32];
extern x86regtyp   xRD[1];
extern x86regtyp   xRS[1];
extern x86regtyp   xRT[1];

void (*dyna4300i_cop1_D_instruction[64])(OP_PARAMS);
void dyna4300i_cop1_mfc1(OP_PARAMS);
void dyna4300i_cop1_dmfc1(OP_PARAMS);
void dyna4300i_cop1_cfc1(OP_PARAMS);
void dyna4300i_cop1_mtc1(OP_PARAMS);
void dyna4300i_cop1_dmtc1(OP_PARAMS);
void dyna4300i_cop1_ctc1(OP_PARAMS);
void dyna4300i_cop1_BC_instr(OP_PARAMS);
void dyna4300i_cop1_S_instr(OP_PARAMS);
void dyna4300i_cop1_D_instr(OP_PARAMS);
void dyna4300i_cop1_W_instr(OP_PARAMS);
void dyna4300i_cop1_L_instr(OP_PARAMS);

void dyna4300i_cop1_bc1f(OP_PARAMS);
void dyna4300i_cop1_bc1t(OP_PARAMS);
void dyna4300i_cop1_bc1fl(OP_PARAMS);
void dyna4300i_cop1_bc1tl(OP_PARAMS);

void dyna4300i_cop1_add_s(OP_PARAMS);
void dyna4300i_cop1_sub_s(OP_PARAMS);
void dyna4300i_cop1_mul_s(OP_PARAMS);
void dyna4300i_cop1_div_s(OP_PARAMS);   
void dyna4300i_cop1_sqrt_s(OP_PARAMS);
void dyna4300i_cop1_abs_s(OP_PARAMS);
void dyna4300i_cop1_mov_s(OP_PARAMS);
void dyna4300i_cop1_neg_s(OP_PARAMS);
void dyna4300i_cop1_roundl_s(OP_PARAMS);
void dyna4300i_cop1_truncl_s(OP_PARAMS);
void dyna4300i_cop1_ceill_s(OP_PARAMS);
void dyna4300i_cop1_floorl_s(OP_PARAMS);
void dyna4300i_cop1_roundw_s(OP_PARAMS);
void dyna4300i_cop1_truncw_s(OP_PARAMS);
void dyna4300i_cop1_ceilw_s(OP_PARAMS);
void dyna4300i_cop1_floorw_s(OP_PARAMS);
void dyna4300i_cop1_cvtd_s(OP_PARAMS);
void dyna4300i_cop1_cvtw_s(OP_PARAMS);
void dyna4300i_cop1_cvtl_s(OP_PARAMS);
void dyna4300i_c_f_s(OP_PARAMS);
void dyna4300i_c_un_s(OP_PARAMS);
void dyna4300i_c_eq_s(OP_PARAMS);
void dyna4300i_c_ueq_s(OP_PARAMS);
void dyna4300i_c_olt_s(OP_PARAMS);
void dyna4300i_c_ult_s(OP_PARAMS);
void dyna4300i_c_ole_s(OP_PARAMS);  
void dyna4300i_c_ule_s(OP_PARAMS);
void dyna4300i_c_sf_s(OP_PARAMS);
void dyna4300i_c_ngle_s(OP_PARAMS);
void dyna4300i_c_seq_s(OP_PARAMS);
void dyna4300i_c_ngl_s(OP_PARAMS);
void dyna4300i_c_lt_s(OP_PARAMS);
void dyna4300i_c_nge_s(OP_PARAMS);
void dyna4300i_c_le_s(OP_PARAMS);
void dyna4300i_c_ngt_s(OP_PARAMS);

void dyna4300i_cop1_add_d(OP_PARAMS);
void dyna4300i_cop1_sub_d(OP_PARAMS);
void dyna4300i_cop1_mul_d(OP_PARAMS);
void dyna4300i_cop1_div_d(OP_PARAMS);   
void dyna4300i_cop1_sqrt_d(OP_PARAMS);
void dyna4300i_cop1_abs_d(OP_PARAMS);
void dyna4300i_cop1_mov_d(OP_PARAMS);
void dyna4300i_cop1_neg_d(OP_PARAMS);
void dyna4300i_cop1_roundl_d(OP_PARAMS);
void dyna4300i_cop1_truncl_d(OP_PARAMS);
void dyna4300i_cop1_ceill_d(OP_PARAMS); 
void dyna4300i_cop1_floorl_d(OP_PARAMS);    
void dyna4300i_cop1_roundw_d(OP_PARAMS);
void dyna4300i_cop1_truncw_d(OP_PARAMS);
void dyna4300i_cop1_ceilw_d(OP_PARAMS);
void dyna4300i_cop1_floorw_d(OP_PARAMS);
void dyna4300i_cop1_cvts_d(OP_PARAMS);
void dyna4300i_cop1_cvtw_d(OP_PARAMS);
void dyna4300i_cop1_cvtl_d(OP_PARAMS);
void dyna4300i_c_f_d(OP_PARAMS);
void dyna4300i_c_un_d(OP_PARAMS);
void dyna4300i_c_eq_d(OP_PARAMS);
void dyna4300i_c_ueq_d(OP_PARAMS);
void dyna4300i_c_olt_d(OP_PARAMS);  
void dyna4300i_c_ult_d(OP_PARAMS);  
void dyna4300i_c_ole_d(OP_PARAMS);  
void dyna4300i_c_ule_d(OP_PARAMS);
void dyna4300i_c_sf_d(OP_PARAMS);
void dyna4300i_c_ngle_d(OP_PARAMS);
void dyna4300i_c_seq_d(OP_PARAMS);  
void dyna4300i_c_ngl_d(OP_PARAMS);
void dyna4300i_c_lt_d(OP_PARAMS);
void dyna4300i_c_nge_d(OP_PARAMS);  
void dyna4300i_c_le_d(OP_PARAMS);
void dyna4300i_c_ngt_d(OP_PARAMS);

void dyna4300i_cop1_cvts_w(OP_PARAMS);
void dyna4300i_cop1_cvtd_w(OP_PARAMS);
        
void dyna4300i_cop1_cvts_l(OP_PARAMS);
void dyna4300i_cop1_cvtd_l(OP_PARAMS);


void dyna4300i_c_s(OP_PARAMS);
void dyna4300i_c_d(OP_PARAMS);

/******************************************************************************\
*                                                                              *
*  Function Tables                                                             *
*                                                                              *
\******************************************************************************/
void (*dyna4300i_cop1_Instruction[])(OP_PARAMS)=
{
    dyna4300i_cop1_mfc1,     dyna4300i_cop1_dmfc1,   dyna4300i_cop1_cfc1,       dyna4300i_reserved, 
    dyna4300i_cop1_mtc1,     dyna4300i_cop1_dmtc1,   dyna4300i_cop1_ctc1,       dyna4300i_reserved,
    dyna4300i_cop1_BC_instr, dyna4300i_reserved,     dyna4300i_reserved,        dyna4300i_reserved, 
    dyna4300i_reserved,      dyna4300i_reserved,     dyna4300i_reserved,        dyna4300i_reserved,
    dyna4300i_cop1_S_instr,  dyna4300i_cop1_D_instr, dyna4300i_reserved,        dyna4300i_reserved, 
    dyna4300i_cop1_W_instr,  dyna4300i_cop1_L_instr, dyna4300i_reserved,        dyna4300i_reserved,
    dyna4300i_reserved,      dyna4300i_reserved,     dyna4300i_reserved,        dyna4300i_reserved, 
    dyna4300i_reserved,      dyna4300i_reserved,     dyna4300i_reserved,        dyna4300i_reserved
};

void (*dyna4300i_cop1_BC_instruction[])(OP_PARAMS)=
{
    dyna4300i_cop1_bc1f,      dyna4300i_cop1_bc1t,
    dyna4300i_cop1_bc1fl,     dyna4300i_cop1_bc1tl
};

void (*dyna4300i_cop1_S_instruction[])(OP_PARAMS)=
{
    dyna4300i_cop1_add_s,    dyna4300i_cop1_sub_s,    dyna4300i_cop1_mul_s,     dyna4300i_cop1_div_s,   
    dyna4300i_cop1_sqrt_s,   dyna4300i_cop1_abs_s,    dyna4300i_cop1_mov_s,     dyna4300i_cop1_neg_s,
    dyna4300i_cop1_roundl_s, dyna4300i_cop1_truncl_s, dyna4300i_cop1_ceill_s,   dyna4300i_cop1_floorl_s,
    dyna4300i_cop1_roundw_s, dyna4300i_cop1_truncw_s, dyna4300i_cop1_ceilw_s,   dyna4300i_cop1_floorw_s,
    dyna4300i_reserved,      dyna4300i_reserved,      dyna4300i_reserved,       dyna4300i_reserved,
    dyna4300i_reserved,      dyna4300i_reserved,      dyna4300i_reserved,       dyna4300i_reserved,
    dyna4300i_reserved,      dyna4300i_reserved,      dyna4300i_reserved,       dyna4300i_reserved,
    dyna4300i_reserved,      dyna4300i_reserved,      dyna4300i_reserved,       dyna4300i_reserved,
    dyna4300i_reserved,      dyna4300i_cop1_cvtd_s,   dyna4300i_reserved,       dyna4300i_reserved,
    dyna4300i_cop1_cvtw_s,   dyna4300i_cop1_cvtl_s,   dyna4300i_reserved,       dyna4300i_reserved,
    dyna4300i_reserved,      dyna4300i_reserved,      dyna4300i_reserved,       dyna4300i_reserved,
    dyna4300i_reserved,      dyna4300i_reserved,      dyna4300i_reserved,       dyna4300i_reserved,

    dyna4300i_c_f_s,         dyna4300i_c_un_s,        dyna4300i_c_eq_s,         dyna4300i_c_ueq_s,
    dyna4300i_c_olt_s,       dyna4300i_c_ult_s,       dyna4300i_c_ole_s,        dyna4300i_c_ule_s,
    dyna4300i_c_sf_s,        dyna4300i_c_ngle_s,      dyna4300i_c_seq_s,        dyna4300i_c_ngl_s,
    dyna4300i_c_lt_s,        dyna4300i_c_nge_s,       dyna4300i_c_le_s,         dyna4300i_c_ngt_s

/*
	dyna4300i_c_s,			 dyna4300i_c_s,			dyna4300i_c_s,			    dyna4300i_c_s,
	dyna4300i_c_s,			 dyna4300i_c_s,			dyna4300i_c_s,			    dyna4300i_c_s,
	dyna4300i_c_s,			 dyna4300i_c_s,			dyna4300i_c_s,			    dyna4300i_c_s,
	dyna4300i_c_s,			 dyna4300i_c_s,			dyna4300i_c_s,			    dyna4300i_c_s
*/
};

void (*dyna4300i_cop1_D_instruction[64])(OP_PARAMS)=
{
    dyna4300i_cop1_add_d,    dyna4300i_cop1_sub_d,    dyna4300i_cop1_mul_d,     dyna4300i_cop1_div_d,   
    dyna4300i_cop1_sqrt_d,   dyna4300i_cop1_abs_d,    dyna4300i_cop1_mov_d,     dyna4300i_cop1_neg_d,
    dyna4300i_cop1_roundl_d, dyna4300i_cop1_truncl_d, dyna4300i_cop1_ceill_d,   dyna4300i_cop1_floorl_d,
    dyna4300i_cop1_roundw_d, dyna4300i_cop1_truncw_d, dyna4300i_cop1_ceilw_d,   dyna4300i_cop1_floorw_d,
    dyna4300i_reserved,      dyna4300i_reserved,      dyna4300i_reserved,       dyna4300i_reserved,
    dyna4300i_reserved,      dyna4300i_reserved,      dyna4300i_reserved,       dyna4300i_reserved,
    dyna4300i_reserved,      dyna4300i_reserved,      dyna4300i_reserved,       dyna4300i_reserved,
    dyna4300i_reserved,      dyna4300i_reserved,      dyna4300i_reserved,       dyna4300i_reserved,
    dyna4300i_cop1_cvts_d,   dyna4300i_reserved,      dyna4300i_reserved,       dyna4300i_reserved,
    dyna4300i_cop1_cvtw_d,   dyna4300i_cop1_cvtl_d,   dyna4300i_reserved,       dyna4300i_reserved,
    dyna4300i_reserved,      dyna4300i_reserved,      dyna4300i_reserved,       dyna4300i_reserved,
    dyna4300i_reserved,      dyna4300i_reserved,      dyna4300i_reserved,       dyna4300i_reserved,
/*
    dyna4300i_c_f_d,         dyna4300i_c_un_d,        dyna4300i_c_eq_d,         dyna4300i_c_ueq_d,
    dyna4300i_c_olt_d,       dyna4300i_c_ult_d,       dyna4300i_c_ole_d,        dyna4300i_c_ule_d,
    dyna4300i_c_sf_d,        dyna4300i_c_ngle_d,      dyna4300i_c_seq_d,        dyna4300i_c_ngl_d,
    dyna4300i_c_lt_d,        dyna4300i_c_nge_d,       dyna4300i_c_le_d,         dyna4300i_c_ngt_d
*/

    dyna4300i_c_d,          dyna4300i_c_d,            dyna4300i_c_eq_d,         dyna4300i_c_d,
    dyna4300i_c_d,          dyna4300i_c_d,            dyna4300i_c_d,            dyna4300i_c_d,
    dyna4300i_c_sf_d,       dyna4300i_c_ngle_d,       dyna4300i_c_d,            dyna4300i_c_d,
    dyna4300i_c_lt_d,       dyna4300i_c_nge_d,        dyna4300i_c_le_d,         dyna4300i_c_ngt_d
};

void (*dyna4300i_cop1_W_instruction[])(OP_PARAMS)=
{
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_cop1_cvts_w,  dyna4300i_cop1_cvtd_w,  dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved
};
void (*dyna4300i_cop1_L_instruction[])(OP_PARAMS)=
{
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_cop1_cvts_l,  dyna4300i_cop1_cvtd_l,  dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,
    dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved,     dyna4300i_reserved
};


/******************************************************************************\
*                                                                              *
*   CoProcessor1 (COP1)                                                        *
*                                                                              *
\******************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// void (*dyna4300i_cop1_Instruction[])(OP_PARAMS)=                         //
//////////////////////////////////////////////////////////////////////////////
void dyna4300i_cop1_mfc1(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

_SAFTY_COP1_(r4300i_COP1_mfc1)
    
    ConstMap[xRT->mips_reg].IsMapped = 0;
    xRT->IsDirty = 1;
    xRT->NoNeedToLoadTheLo = 1;
    MapRT;
    MOV_MemoryToReg(1,(_u8)xRT->x86reg,ModRM_disp32,(unsigned long)&reg->fpr32[__RD]);
}

void dyna4300i_cop1_dmfc1(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS);

_SAFTY_COP1_(r4300i_COP1_dmfc1)
    
    ConstMap[xRT->mips_reg].IsMapped = 0;
    xRT->IsDirty = 1;
    xRT->NoNeedToLoadTheLo = 1;
    xRT->NoNeedToLoadTheHi = 1;
    MapRT;
    MOV_MemoryToReg(1,(_u8)xRT->x86reg,    ModRM_disp32,  (unsigned long)&reg->fpr32[__RD]);
    MOV_MemoryToReg(1,(_u8)xRT->HiWordLoc, ModRM_disp32,4+(unsigned long)&reg->fpr32[__RD]);
}

void dyna4300i_cop1_cfc1(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

    _SAFTY_COP1_(r4300i_COP1_cfc1)
    
    ConstMap[xRT->mips_reg].IsMapped = 0;
    xRT->IsDirty = 1;
    MapRT;
    MOV_MemoryToReg(1, (_u8)xRT->x86reg, ModRM_disp32, (unsigned long )&reg->COP1Con[__FS]);
}

void dyna4300i_cop1_ctc1(OP_PARAMS)
{
    SetRdRsRt64bit(PASS_PARAMS);

    _SAFTY_COP1_(r4300i_COP1_ctc1)

    
    MapRT;
    MOV_RegToMemory(1, (_u8)xRT->x86reg,    ModRM_disp32,  (unsigned long)&reg->COP1Con[__FS]);
    MOV_RegToMemory(1, (_u8)xRT->HiWordLoc, ModRM_disp32,4+(unsigned long)&reg->COP1Con[__FS]);
}

void dyna4300i_cop1_mtc1(OP_PARAMS)
{
    SetRdRsRt32bit(PASS_PARAMS);

    _SAFTY_COP1_(r4300i_COP1_mtc1)

    if (ConstMap[xRT->mips_reg].IsMapped == 0)
    {
        MapRT;
        MOV_RegToMemory(1, (_u8)xRT->x86reg,ModRM_disp32,(unsigned long)&reg->fpr32[__FS]);
    }
    else
        MOV_ImmToMemory(1, (unsigned long)&reg->fpr32[__FS], ConstMap[xRT->mips_reg].value);
}

void dyna4300i_cop1_dmtc1(OP_PARAMS)
{
_SAFTY_COP1_(r4300i_COP1_dmtc1)
    
    INTERPRET(r4300i_COP1_dmtc1);
}

void dyna4300i_cop1_BC_instr(OP_PARAMS)
{
    dyna4300i_cop1_BC_instruction[__RT](PASS_PARAMS);
}

void dyna4300i_cop1_S_instr(OP_PARAMS)      
{
    dyna4300i_cop1_S_instruction[__F](PASS_PARAMS);
}

void dyna4300i_cop1_D_instr(OP_PARAMS)      
{   
    dyna4300i_cop1_D_instruction[__F](PASS_PARAMS);
}

void dyna4300i_cop1_W_instr(OP_PARAMS)      
{
    dyna4300i_cop1_W_instruction[__F](PASS_PARAMS);
}

void dyna4300i_cop1_L_instr(OP_PARAMS)      
{   
    dyna4300i_cop1_L_instruction[__F](PASS_PARAMS);
}

//////////////////////////////////////////////////////////////////////////////
// void (*dyna4300i_cop1_S_instruction[])(OP_PARAMS)=                       //
//////////////////////////////////////////////////////////////////////////////
void dyna4300i_cop1_add_s(OP_PARAMS)
{
_SAFTY_COP1_(r4300i_COP1_add_s)

    FLD_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FS]);
    FADD_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FT]);
    FSTP_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FD]);
}

void dyna4300i_cop1_sub_s(OP_PARAMS)
{
_SAFTY_COP1_(r4300i_COP1_sub_s)    
    
    FLD_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FS]);
    FSUB_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FT]);
    FSTP_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FD]);
}

void dyna4300i_cop1_mul_s(OP_PARAMS)
{
_SAFTY_COP1_(r4300i_COP1_mul_s)

    FLD_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FS]);
    FMUL_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FT]);
    FSTP_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FD]);
}

void dyna4300i_cop1_div_s(OP_PARAMS)
{
_SAFTY_COP1_(r4300i_COP1_div_s)

    FLD_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FS]);
    FDIV_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FT]);
    FSTP_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FD]);
}   

void dyna4300i_cop1_sqrt_s(OP_PARAMS)
{
_SAFTY_COP1_(r4300i_COP1_sqrt_s)
    
    FLD_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FS]);
    FSQRT();
    FSTP_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FD]);
}

void dyna4300i_cop1_abs_s(OP_PARAMS)
{
_SAFTY_COP1_(r4300i_COP1_abs_s)    
    
    FLD_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FS]);
    FABS();
    FSTP_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FD]);
}

void dyna4300i_cop1_mov_s(OP_PARAMS)
{
_SAFTY_COP1_(r4300i_COP1_mov_s)
    
    MOV_MemoryToReg(1,Reg_ECX,ModRM_disp32,(unsigned long)&reg->fpr32[__FS]);
    MOV_RegToMemory(1,Reg_ECX,ModRM_disp32,(unsigned long)&reg->fpr32[__FD]);
}

void dyna4300i_cop1_neg_s(OP_PARAMS)
{
_SAFTY_COP1_(r4300i_COP1_neg_s)
    
//    DisplayError("Fneg");
    FLD_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FS]);
    FNEG();
    FSTP_Memory(FORMAT_SINGLE,(unsigned long)&reg->fpr32[__FD]);
}

void dyna4300i_cop1_roundl_s(OP_PARAMS)
{
        
    INTERPRET(r4300i_COP1_roundl_s);
}

void dyna4300i_cop1_truncl_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_COP1_truncl_s);
}

void dyna4300i_cop1_ceill_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_COP1_ceill_s);
}   

void dyna4300i_cop1_floorl_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_COP1_floorl_s);
}

void dyna4300i_cop1_roundw_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_COP1_roundw_s);
}

void dyna4300i_cop1_truncw_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_COP1_truncw_s);
}

void dyna4300i_cop1_ceilw_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_COP1_ceilw_s);
}

void dyna4300i_cop1_floorw_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_COP1_floorw_s);
}

void dyna4300i_cop1_cvtd_s(OP_PARAMS)
{
_SAFTY_COP1_(r4300i_COP1_cvtd_s) 

    FLD_Memory(FORMAT_SINGLE,  (_u32)&reg->fpr32[__FS]); 
    FSTP_Memory(FORMAT_QUAD, (_u32)&reg->fpr32[__FD]);
}

void dyna4300i_cop1_cvtw_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_COP1_cvtw_s);
}

void dyna4300i_cop1_cvtl_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_COP1_cvtl_s);
}

void dyna4300i_c_f_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_C_F_S);
}

void dyna4300i_c_un_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_C_UN_S);
}

void dyna4300i_c_ueq_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_C_UEQ_S);
}

void dyna4300i_c_olt_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_C_OLT_S);
}

void dyna4300i_c_ult_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_C_ULT_S);
}

void dyna4300i_c_ole_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_C_OLE_S);
}
    
void dyna4300i_c_ule_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_C_ULE_S);
}

void dyna4300i_c_sf_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_C_SF_S);
}

void dyna4300i_c_ngle_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_C_NGLE_S);
}

void dyna4300i_c_seq_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_C_SEQ_S);
}

void dyna4300i_c_ngl_s(OP_PARAMS)
{
    
    INTERPRET(r4300i_C_NGL_S);
}

//00431C99 D9 05 88 DE 68 00    fld         dword ptr [_COP1Reg+8 (0068de88)]
//00431C9F D8 1D 80 DE 68 00    fcomp       dword ptr [_COP1Reg (0068de80)]
//00431CA5 DF E0                fnstsw      ax
//00431CA7 F6 C4 01             test        ah,1
//00431CAA 74 11                je          rc_r4300i_C_LT_S+2Dh (00431cbd)
//00431CAC A1 BC DC 68 00       mov         eax,[_COP1Con+7Ch (0068dcbc)]
//00431CB1 0D 00 00 80 00       or          eax,offset _DynaRDRAM+171F80h (00800000)
//00431CB6 A3 BC DC 68 00       mov         [_COP1Con+7Ch (0068dcbc)],eax
//1327:     else                                                            COP1Con[31] &=  0xFF7FFFFF;
//00431CBB EB 12                jmp         rc_r4300i_C_LT_S+3Fh (00431ccf)
//00431CBD 8B 0D BC DC 68 00    mov         ecx,dword ptr [_COP1Con+7Ch (0068dcbc)]
//00431CC3 81 E1 FF FF 7F FF    and         ecx,0FF7FFFFFh
//00431CC9 89 0D BC DC 68 00    mov         dword ptr [_COP1Con+7Ch (0068dcbc)],ecx

void _do_c_(HardwareState* reg, _u8 format, _u8 testval)
{
    MOV_Reg2ToReg1(1, Reg_ECX, Reg_EAX);
    FLD_Memory(format,(unsigned long)&reg->fpr32[__FS]);
    FCOMP(format, (unsigned long)&reg->fpr32[__FT]);
    
/*    __asm {
        fnstsw ax
        test ah, 1
    }
*/
    WC32(0xC4F6E0DF); WC8(testval);
    Jcc_auto(CC_E, 0);

    MOV_Reg2ToReg1(1, Reg_EAX, Reg_ECX);
    MOV_MemoryToReg(1, Reg_ECX, ModRM_disp32, (_u32)&reg->COP1Con[31]);
    OR_ImmToReg(1, Reg_ECX, COP1_CONDITION_BIT);
    
    JMP_Short_auto(1);

    SetTarget(0);

    MOV_Reg2ToReg1(1, Reg_EAX, Reg_ECX);
    MOV_MemoryToReg(1, Reg_ECX, ModRM_disp32, (_u32)&reg->COP1Con[31]);
    AND_ImmToReg(1, Reg_ECX, ~COP1_CONDITION_BIT);
    
    SetTarget(1);
    MOV_RegToMemory(1, Reg_ECX, ModRM_disp32, (_u32)&reg->COP1Con[31]);
}



void dyna4300i_c_lt_s(OP_PARAMS) {   _SAFTY_COP1_(r4300i_C_LT_S)  _do_c_(reg, FORMAT_SINGLE, 0x01);}
void dyna4300i_c_nge_s(OP_PARAMS){   _SAFTY_COP1_(r4300i_C_NGE_S) _do_c_(reg, FORMAT_SINGLE, 0x01);}
void dyna4300i_c_eq_s(OP_PARAMS) {   _SAFTY_COP1_(r4300i_C_EQ_S)  _do_c_(reg, FORMAT_SINGLE, 0x40);}
void dyna4300i_c_le_s(OP_PARAMS) {   _SAFTY_COP1_(r4300i_C_LE_S)  _do_c_(reg, FORMAT_SINGLE, 0x41);}
void dyna4300i_c_ngt_s(OP_PARAMS){   _SAFTY_COP1_(r4300i_C_NGT_S) _do_c_(reg, FORMAT_SINGLE, 0x41);}


void dyna4300i_c_lt_d(OP_PARAMS) {   _SAFTY_COP1_(r4300i_C_LT_D)  _do_c_(reg, FORMAT_QUAD,   0x01);}
void dyna4300i_c_nge_d(OP_PARAMS){   _SAFTY_COP1_(r4300i_C_NGE_D) _do_c_(reg, FORMAT_QUAD,   0x01);}
void dyna4300i_c_eq_d(OP_PARAMS) {   _SAFTY_COP1_(r4300i_C_EQ_D)  _do_c_(reg, FORMAT_QUAD,   0x40);}
void dyna4300i_c_le_d(OP_PARAMS) {   _SAFTY_COP1_(r4300i_C_LE_D)  _do_c_(reg, FORMAT_QUAD,   0x41);}
void dyna4300i_c_ngt_d(OP_PARAMS){   _SAFTY_COP1_(r4300i_C_NGT_D) _do_c_(reg, FORMAT_QUAD,   0x41);}

//////////////////////////////////////////////////////////////////////////////
// void (*dyna4300i_cop1_D_instruction[])(OP_PARAMS)=                       //
//////////////////////////////////////////////////////////////////////////////


//  *((double *)&COP1Reg[SA_FD]) =  *((double *)&COP1Reg[RD_FS])  / *((double *)&COP1Reg[RT_FT]);   }
//  0041A8A9 DD 05 24 50 4B 00    fld         qword ptr [_COP1Reg+4 (004b5024)]
//  0041A8AF DC 35 28 50 4B 00    fdiv        qword ptr [_COP1Reg+8 (004b5028)]
//  0041A8B5 DD 1D 20 50 4B 00    fstp        qword ptr [_COP1Reg (004b5020)]
#define  fdLOGIC(Sum, Operand1, OPERATOR16, Operand2)       \
    WC16(0x05DD);     WC32((_u32)&Operand1);                \
    WC16(OPERATOR16); WC32((_u32)&Operand2);                \
    WC16(0x1DDD);     WC32((_u32)&Sum);

//#define cFD     reg->fpr32[__FD]
//#define cFS     reg->fpr32[__FS]
//#define cFT     reg->fpr32[__FT]
//#define fdoubleLOGICAL(OPERATOR16)      fdLOGIC(cFD, cFS, OPERATOR16, cFT)
#define fdoubleLOGICAL(OPERATOR16)      fdLOGIC(reg->fpr32[__FD], reg->fpr32[__FS], OPERATOR16, reg->fpr32[__FT])

void dyna4300i_cop1_add_d(OP_PARAMS){  _SAFTY_COP1_(r4300i_COP1_add_d) /*FlushAllRegisters();*/  fdoubleLOGICAL(0x05DC)}
void dyna4300i_cop1_sub_d(OP_PARAMS){  _SAFTY_COP1_(r4300i_COP1_sub_d) /*FlushAllRegisters();*/  fdoubleLOGICAL(0x25DC)}
void dyna4300i_cop1_mul_d(OP_PARAMS){  _SAFTY_COP1_(r4300i_COP1_mul_d) /*FlushAllRegisters();*/  fdoubleLOGICAL(0x0DDC)}
void dyna4300i_cop1_div_d(OP_PARAMS){  _SAFTY_COP1_(r4300i_COP1_div_d) /*FlushAllRegisters();*/  fdoubleLOGICAL(0x35DC)}

void dyna4300i_cop1_sqrt_d(OP_PARAMS){       INTERPRET(r4300i_COP1_sqrt_d);}
void dyna4300i_cop1_abs_d(OP_PARAMS){        INTERPRET(r4300i_COP1_abs_d);}
void dyna4300i_cop1_mov_d(OP_PARAMS){        INTERPRET(r4300i_COP1_mov_d);}
void dyna4300i_cop1_neg_d(OP_PARAMS){        INTERPRET(r4300i_COP1_neg_d);}
void dyna4300i_cop1_roundl_d(OP_PARAMS){     INTERPRET(r4300i_COP1_roundl_d);}
void dyna4300i_cop1_truncl_d(OP_PARAMS){     INTERPRET(r4300i_COP1_truncl_d);}
void dyna4300i_cop1_ceill_d(OP_PARAMS){      INTERPRET(r4300i_COP1_ceill_d);}
void dyna4300i_cop1_floorl_d(OP_PARAMS){     INTERPRET(r4300i_COP1_floorl_d);}
void dyna4300i_cop1_roundw_d(OP_PARAMS){     INTERPRET(r4300i_COP1_roundw_d);}
void dyna4300i_cop1_truncw_d(OP_PARAMS){     INTERPRET(r4300i_COP1_truncw_d);}
void dyna4300i_cop1_ceilw_d(OP_PARAMS){      INTERPRET(r4300i_COP1_ceilw_d);}
void dyna4300i_cop1_floorw_d(OP_PARAMS){     INTERPRET(r4300i_COP1_floorw_d);}


void dyna4300i_cop1_cvts_d(OP_PARAMS)
{
_SAFTY_COP1_(r4300i_COP1_cvts_d);
    //00417A4A DD 05 C8 78 4A 00    fld         qword ptr [_COP1Reg+8 (004a78c8)]
    //00417A50 D9 1D C0 78 4A 00    fstp        dword ptr [_COP1Reg (004a78c0)]
    
    WC16(0x05DD);     WC32((_u32)&reg->fpr32[__FS]);
    WC16(0x1DD9);     WC32((_u32)&reg->fpr32[__FD]);
}


void dyna4300i_cop1_cvtw_d(OP_PARAMS){   INTERPRET(r4300i_COP1_cvtw_d);}
void dyna4300i_cop1_cvtl_d(OP_PARAMS){   INTERPRET(r4300i_COP1_cvtl_d);}
void dyna4300i_c_f_d(OP_PARAMS){     INTERPRET(r4300i_C_F_D);}
void dyna4300i_c_un_d(OP_PARAMS){    INTERPRET(r4300i_C_UN_D);}
void dyna4300i_c_ueq_d(OP_PARAMS){   INTERPRET(r4300i_C_UEQ_D);}
void dyna4300i_c_olt_d(OP_PARAMS){   INTERPRET(r4300i_C_OLT_D);}
void dyna4300i_c_ult_d(OP_PARAMS){   INTERPRET(r4300i_C_ULT_D);}
void dyna4300i_c_ole_d(OP_PARAMS){   INTERPRET(r4300i_C_OLE_D);}
void dyna4300i_c_ule_d(OP_PARAMS){   INTERPRET(r4300i_C_ULE_D);}
void dyna4300i_c_sf_d(OP_PARAMS){    INTERPRET(r4300i_C_SF_D);}
void dyna4300i_c_ngle_d(OP_PARAMS){  INTERPRET(r4300i_C_NGLE_D);}
void dyna4300i_c_seq_d(OP_PARAMS){   INTERPRET(r4300i_C_SEQ_D);}
void dyna4300i_c_ngl_d(OP_PARAMS){   INTERPRET(r4300i_C_NGL_D);}


//////////////////////////////////////////////////////////////////////////////
// void (*dyna4300i_cop1_W_instruction[])(OP_PARAMS)=                       //
//////////////////////////////////////////////////////////////////////////////
void dyna4300i_cop1_cvts_w(OP_PARAMS)
{
    //INTERPRET(r4300i_COP1_cvts_w); return;

//00417A89 DB 05 C8 78 4A 00    fild        dword ptr [_COP1Reg+8 (004a78c8)]
//00417A8F D9 1D C0 78 4A 00    fstp        dword ptr [_COP1Reg (004a78c0)]

_SAFTY_COP1_(r4300i_COP1_cvts_w)

    FILD_Memory(FORMAT_SINGLE, (_u32)&reg->fpr32[__FS]); 
    FSTP_Memory(FORMAT_SINGLE, (_u32)&reg->fpr32[__FD]);
}


void dyna4300i_cop1_cvtd_w(OP_PARAMS)
{
_SAFTY_COP1_(r4300i_COP1_cvtd_w)
    
    FILD_Memory(FORMAT_SINGLE,  (_u32)&reg->fpr32[__FS]); 
    FSTP_Memory(FORMAT_QUAD, (_u32)&reg->fpr32[__FD]);
}

//////////////////////////////////////////////////////////////////////////////
// void (*dyna4300i_cop1_L_instruction[])(OP_PARAMS)=                       //
//////////////////////////////////////////////////////////////////////////////
void dyna4300i_cop1_cvts_l(OP_PARAMS)
{
    
    INTERPRET(r4300i_COP1_cvts_l);
}

void dyna4300i_cop1_cvtd_l(OP_PARAMS)
{
    
    INTERPRET(r4300i_COP1_cvtd_l);
}

//////////////////////////////////////////////////////////////////////////////
// Load From / Store To CPR[Reg]                                            //
//////////////////////////////////////////////////////////////////////////////

extern void HELP_Cop1(unsigned long pc);
extern BOOL COP1_exception_speedup;
void dyna4300i_lwc1(OP_PARAMS)
{
    
    INTERPRET(r4300i_lwc1);

    if ((((_u32)reg->COP0Reg[STATUS] & STATUS_CU1) == 0) 
        && (COP1_exception_speedup == TRUE))
    {
//        COP1_exception_speedup = FALSE;
//        HELP_Call((unsigned long)HELP_Cop1);
//        HELP_CheckExceptionSlot(reg->pc);
    }
}

void dyna4300i_ldc1(OP_PARAMS)
{
    
    INTERPRET(r4300i_ldc1);
    if ((((_u32)reg->COP0Reg[STATUS] & STATUS_CU1) == 0) 
        && (COP1_exception_speedup == TRUE))
    {
//        COP1_exception_speedup = FALSE;
//        HELP_Call((unsigned long)HELP_Cop1);
//        HELP_CheckExceptionSlot(reg->pc);
    }
}

void dyna4300i_swc1(OP_PARAMS)
{
    
    INTERPRET(r4300i_swc1);

    if ((((_u32)reg->COP0Reg[STATUS] & STATUS_CU1) == 0) 
        && (COP1_exception_speedup == TRUE))
    {
//        COP1_exception_speedup = FALSE;
//        HELP_Call((unsigned long)HELP_Cop1);
//        HELP_CheckExceptionSlot(reg->pc);
    }
}

void dyna4300i_sdc1(OP_PARAMS)
{
    
    INTERPRET(r4300i_sdc1);

    if ((((_u32)reg->COP0Reg[STATUS] & STATUS_CU1) == 0) 
        && (COP1_exception_speedup == TRUE))
    {
//        COP1_exception_speedup = FALSE;
//        HELP_Call((unsigned long)HELP_Cop1);
//        HELP_CheckExceptionSlot(reg->pc);
    }
}

void dyna4300i_c_d(OP_PARAMS)
{
//_SAFTY_COP1_(r4300i_c_d);
    
    
//    INTERPRET(r4300i_c_d);
}