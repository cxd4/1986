#ifndef _REGCACHE_H__1964_
#define _REGCACHE_H__1964_




#define _Xor 0x33



/* These macros are for mapping and unmapping GPRs. */
/* Comments indicate examples of recompiled code    */
#define StoreGPR_LO(k)                                                  \
{                                                                       \
    _u32 mod_rm;                                                        \
    _u32 offset = (_u32)&gHardwareState.GPR[0]+(x86reg[k].mips_reg<<3);             \
                                                                        \
  /*00413F0D 89 0D 80 83 49 00    mov         dword ptr [_GPR],ecx*/    \
    if (k==0)                                                           \
        WC8(0xA3);                                                      \
    else                                                                \
    {                                                                   \
        mod_rm = 0x5|(k<<3);                                            \
        WC8(0x89);                                                      \
        WC8((_u8)mod_rm);                                               \
    }                                                                   \
    WC32(offset);                                                       \
}

#define StoreGPR_HI(k)                                                  \
{                                                                       \
    _u32 mod_rm;                                                        \
    _u32 offset = (_u32)&gHardwareState.GPR[0]+4+(x86reg[k].mips_reg<<3);           \
                                                                        \
  /*00413F0D 89 0D 80 83 49 00    mov         dword ptr [_GPR+4],ecx*/  \
    if (x86reg[k].HiWordLoc==0)                                         \
        WC8(0xA3);                                                      \
    else                                                                \
    {                                                                   \
        mod_rm = 0x5|(x86reg[k].HiWordLoc<<3);                          \
        WC8(0x89);                                                      \
        WC8((_u8)mod_rm);                                               \
    }                                                                   \
    WC32(offset);                                                       \
}


#define LoadGPR_LO(k)                                                   \
{                                                                       \
    _u32 offset = (_u32)&gHardwareState.GPR[0]+(x86reg[k].mips_reg<<3);             \
                                                                        \
    /*0041B3E3 8B 0D 84 61 4A 00    mov         ecx,dword ptr [_GPR] */ \
    if (k==0)                                                           \
        WC8(0xA1);                                                      \
    else                                                                \
    {                                                                   \
        _u32 mod_rm = 0x5|(k<<3);                                       \
        WC8(0x8B);                                                      \
        WC8((_u8)mod_rm);                                               \
    }                                                                   \
    WC32(offset);                                                       \
}

#define LoadGPR_HI(k)                                                   \
{                                                                       \
    _u32 offset = (_u32)&gHardwareState.GPR[0]+4+(x86reg[k].mips_reg<<3);           \
                                                                        \
  /*0041B3E3 8B 0D 84 61 4A 00    mov         ecx,dword ptr [_GPR+4] */ \
    if (x86reg[k].HiWordLoc==0)                                         \
        WC8(0xA1);                                                      \
    else                                                                \
    {                                                                   \
        _u32 mod_rm = 0x5|(x86reg[k].HiWordLoc<<3);                     \
        WC8(0x8B);                                                      \
        WC8((_u8)mod_rm);                                               \
    }                                                                   \
    WC32(offset);                                                       \
}


typedef struct x86regtyp
{
    _u32 BirthDate;
    _s8  HiWordLoc;
    _s8  Is32bit;
    _s8  IsDirty;
    _s8  mips_reg;
    _s8  NoNeedToLoadTheLo;
    _s8  NoNeedToLoadTheHi;
    _s8  x86reg;

} x86regtyp;

//Keeps status of constants
typedef struct MapConstant
{
    _s32 value;
    _s32 IsMapped;
} MapConstant;


//Keeps status of registers stored to memory
typedef struct FlushedMap
{
    _u32 Is32bit;
} FlushedMap;

_u32 ThisYear;
MapConstant ConstMap[32];





#define fpr32 COP1Reg
#define STATUS_CU1  SR_CU1

#define COP1_CONDITION_BIT 0x00800000
#define NOT_COP1_CONDITION_BIT  0xFF7FFFFF


#define __OPCODE        ((_u8)(__CODE >> 26))
#define __RS            (((_u8)(reg->code >> 21)) & 0x1f)
#define __RT            (((_u8)(reg->code >> 16)) & 0x1f)
#define __RD            (((_u8)(reg->code >> 11)) & 0x1f)
#define __SA            (((_u8)(reg->code >>  6)) & 0x1f)
#define __F             ( (_u8)(reg->code)       & 0x3f)
//#define __I             ( (_s32)(_s16)reg->code )
#define __I             ( (_s32)(_s16)(reg->code & 0xFFFF) )
#define __O             ( reg->pc + 4 + (__I << 2) )
#define ____T           (reg->code & 0x3ffffff)
#define __T             ( (reg->pc & 0xf0000000) | (____T << 2) )
#define __FS            __RD
#define __FT            __RT
#define __FD            __SA

#define __dotRS         (((_u8)(gHardwareState.code >> 21)) & 0x1f)
#define __dotRT         (((_u8)(gHardwareState.code >> 16)) & 0x1f)
#define __dotRD         (((_u8)(gHardwareState.code >> 11)) & 0x1f)
#define __dotSA         (((_u8)(gHardwareState.code >>  6)) & 0x1f)

#define __dotI         ( (_s32)(_s16)gHardwareState.code )
#define __dotO             ( gHardwareState.pc + 4 + (__dotI << 2) )
#define ____dotT        (gHardwareState.code & 0x3ffffff)
#define __dotT          ( (gHardwareState.pc & 0xf0000000) | (____dotT << 2) )
#define __dotF          ( (_u8)(gHardwareState.code)       & 0x3f)





/*****************************************************************
                   Function Declarations
/*****************************************************************/



extern int CheckIs32Bit(int mips_reg);
extern void FlushAllRegisters();
extern void FlushOneButNotThese4(int The1stOneNotToFlush, int The2ndOneNotToFlush, int The3rdOneNotToFlush, int The4thOneNotToFlush);
extern void MapRegister(x86regtyp* Conditions, int The2ndOneNotToFlush, int The3rdOneNotToFlush, int The4thOneNotToFlush);
extern int CheckRegStatus(x86regtyp* Query);
extern void InitRegisterMap();
extern _u32 InterruptWasExecuted;

//---------------------------------------------------------------------------------------
#define recPUSHEAX  WC8(0x50);
#define recPOPEAX   WC8(0x58);



#define r32_imm8(opBase, reg, imm)  \
{                                   \
    _u8 op = opBase | (_u8)reg;     \
    WC8(0xC1);                      \
    WC8(op);                        \
    WC8(imm);                       \
}

#define r32_const(op, Const)        \
{                                   \
    WC16(op);                       \
    WC32(Const);                    \
}


#define _jne(Label)     NearJumpConditionLabel8(Label, JNE_NEAR)
#define _je(Label)      NearJumpConditionLabel8(Label,  JE_NEAR)
#define _jmp(Label)     JumpLocal8(Label)
#define _sar(i, Const)  r32_imm8(0xF8, i, Const)

#define _andi(k, Const) r32_const( (0xE081|(k<<8)), Const)
#define _xori(k, Const) r32_const( (0xF081|(k<<8)), Const)
#define  _ori(k, Const) r32_const( (0xC881|(k<<8)), Const)

#define MapRD MapRegister(xRD, xRS->mips_reg, xRT->mips_reg, 99);
#define MapRS MapRegister(xRS, xRD->mips_reg, xRT->mips_reg, 99);
#define MapRT MapRegister(xRT, xRD->mips_reg, xRS->mips_reg, 99);

#define MapLO MapRegister(xLO, xRS->mips_reg, xRT->mips_reg, __HI);
#define MapHI MapRegister(xHI, xRS->mips_reg, xRT->mips_reg, __LO);

#define MapRS__ProtectLoHi MapRegister(xRS, __LO, xRT->mips_reg, __HI);
#define MapRT__ProtectLoHi MapRegister(xRT, __LO, xRS->mips_reg, __HI);

#define r32_const(op, Const)        \
{                                   \
    WC16(op);                       \
    WC32(Const);                    \
}

#define rcAddConst(rs, rt, Constant, NEG)                                       \
{                                                                               \
    if (rs==rt)                                                                 \
    {                                                                           \
        {                                                                       \
            if (ConstMap[xRT->mips_reg].IsMapped == 1)                          \
            {                                                                   \
                ConstMap[xRT->mips_reg].value += Constant;                      \
            }                                                                   \
            else                                                                \
            {                                                                   \
                xRT->IsDirty = 1;                                               \
                MapRT;                                                          \
                if ((Constant) != 0)                                            \
                    ADD_ImmToReg(1, xRT->x86reg, Constant);                     \
            }                                                                   \
        }                                                                       \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        if (xRS->mips_reg == 0)                                                 \
        {                                                                       \
            int tempy = CheckWhereIsMipsReg(xRT->mips_reg);                     \
            ConstMap[xRT->mips_reg].IsMapped = 1;                               \
            ConstMap[xRT->mips_reg].value = Constant;                           \
                                                                                \
            if (tempy > -1)                                                     \
            {                                                                   \
                x86reg[tempy].IsDirty = 0;                                      \
                FlushRegister(tempy);                                           \
            }                                                                   \
        }                                                                                   \
        else                                                                                \
        {                                                                                   \
            if (ConstMap[xRS->mips_reg].IsMapped && ConstMap[xRT->mips_reg].IsMapped)       \
            {                                                                               \
                ConstMap[xRT->mips_reg].value = ConstMap[xRS->mips_reg].value + Constant;   \
            }                                                                               \
            else if (ConstMap[xRS->mips_reg].IsMapped)                                      \
            {                                                                               \
                xRT->IsDirty = 1;                                                           \
                xRT->NoNeedToLoadTheLo = 1;                                                 \
                MapRT                                                                       \
                MOV_ImmToReg(1, (_u8)xRT->x86reg, ConstMap[xRS->mips_reg].value+(Constant));\
            }                                                                               \
            else                                                                            \
            {                                                                               \
                ConstMap[xRT->mips_reg].IsMapped = 0;                                       \
                xRT->IsDirty = 1;                                                           \
                xRT->NoNeedToLoadTheLo = 1;                                                 \
                MapRT                                                                       \
                MapRS;                                                                      \
                                                                                            \
                MOV_Reg2ToReg1(1, (_u8)xRT->x86reg, (_u8)xRS->x86reg); /* mov   rt,rs    */ \
                ADD_ImmToReg(1, (_u8)xRT->x86reg, Constant);           /* add   rt,Const */ \
            }                                                                               \
        }                                                                                   \
    }                                                                                       \
}
#define DoConstAdd(theConstant, NEG)   rcAddConst(xRS->mips_reg, xRT->mips_reg, theConstant, NEG)


#define NEGATE(REG)   {XOR_ShortToReg(1, (_u8)REG->x86reg,  0xFF); INC_Reg(1, (_u8)REG->x86reg);}

#define NONNEG {}

#define rcAdd3reg(op, rd, rs, rt, NEG, subtraction)                                             \
{                                                                                               \
    rd = (_u8)__RD;                                                                             \
    rs = (_u8)__RS;                                                                             \
    rt = (_u8)__RT;                                                                             \
                                                                                                \
                                                                                                \
        if (rd==rt)                                                                             \
        {                                                                                       \
            if (rs==rt)  /*rd=rs=rt*/                                                           \
            {                                                                                   \
                if (ConstMap[xRT->mips_reg].IsMapped == 1)                                      \
                {                                                                               \
                    ConstMap[xRT->mips_reg].value += ConstMap[xRD->mips_reg].value;             \
                }                                                                               \
                else                                                                                \
                {                                                                                   \
                    xRD->IsDirty = 1;                                                               \
                                                                                                    \
                    if (subtraction)                                                                \
                    {                                                                               \
                        xRD->NoNeedToLoadTheLo = 1;                                                 \
                        MapRD;                                                                      \
                        Reg2ToReg1(1, _Xor, (_u8)xRD->x86reg, (_u8)xRD->x86reg);   /* xor    rd,rd*/\
                    }                                                                               \
                    else                                                                            \
                    {                                                                               \
                        MapRD;                                                                      \
                        ADD_Reg1ToReg2(1, (_u8)xRD->x86reg, (_u8)xRD->x86reg);   /* shl rd,1 uses 1 cycle also, but only pairing for the u pipe */\
                    }                                                                           \
                }                                                                               \
            }                                                                                   \
            else /*rd=rt, rs!=rt*/                                                              \
            {                                                                                   \
                if (rs == 0)                                                                    \
                    if (subtraction)                                                            \
                    {                                                                           \
                        xRD->IsDirty = 1;                                                       \
                        MapRD;                                                                  \
                        NEG                                                                     \
                        return;                                                                 \
                    }                                                                           \
                    else                                                                        \
                        return;                                                                 \
                                                                                                \
                xRD->IsDirty = 1;                                                               \
                MapRD;                                                                          \
                MapRS;                                                                          \
                NEG                                                                             \
                Reg2ToReg1(1, 0x02, (_u8)xRD->x86reg, (_u8)xRS->x86reg); /* add    rd,rs*/      \
            }                                                                                   \
        }                                                                                       \
        else /*rd != rt */                                                                      \
        {                                                                                       \
            if (rd==rs)                                                                         \
            {                                                                                   \
                if (rt == 0) return;                                                            \
                xRD->IsDirty = 1;                                                               \
                MapRD;                                                                          \
                MapRT;                                                                          \
                Reg2ToReg1(1, op, (_u8)xRD->x86reg, (_u8)xRT->x86reg);    /* add    rd,rt*/     \
            }                                                                                   \
            else  /* rd != rs */                                                                \
            {                                                                                   \
                xRD->IsDirty = 1;                                                               \
                xRD->NoNeedToLoadTheLo = 1;                                                     \
                MapRD;                                                                          \
                if (rs == rt)                                                                   \
                {                                                                               \
                    if (rt == 0) /* rd = 0+0 */                                                 \
                        Reg2ToReg1(1, _Xor, (_u8)xRD->x86reg, (_u8)xRD->x86reg); /* xor rd, rd */\
                    else                                                                        \
                        if (subtraction) /* rd = rs-rs */                                       \
                            Reg2ToReg1(1, _Xor, (_u8)xRD->x86reg, (_u8)xRD->x86reg);   /* xor    rd,rd*/\
                        else  /* rd = rs+rs */                                                  \
                        {                                                                       \
                            MapRT;                                                              \
                            MOV_Reg2ToReg1(1, (_u8)xRD->x86reg, (_u8)xRT->x86reg);    /* mov rd, rt */ \
                            ADD_Reg1ToReg2(1, (_u8)xRD->x86reg, (_u8)xRD->x86reg);   /* shl rd,1 uses 1 cycle also, but only pairing for the u pipe */\
                        }                                                                       \
                }                                                                               \
                else  /* rs != rt */                                                            \
                {                                                                               \
                    if (rs != 0)                                                                \
                    {                                                                           \
                        MapRS;                                                                  \
                        MOV_Reg2ToReg1(1, (_u8)xRD->x86reg, (_u8)xRS->x86reg);/* mov  rd,rs*/   \
                                                                                                \
                        if (rt==0) return;                                                      \
                                                                                                \
                        /* rs!=0, rt!=0 , rs!=rt */                                             \
                        MapRT;                                                                  \
                        Reg2ToReg1(1, op, (_u8)xRD->x86reg, (_u8)xRT->x86reg); /* add rd,rt*/   \
                                                                                                \
                    }                                                                           \
                    else   /*rs=0, rt!=0 */                                                     \
                    {                                                                           \
                        MapRT;                                                                  \
                        MOV_Reg2ToReg1(1, (_u8)xRD->x86reg, (_u8)xRT->x86reg); /* mov  rd,rt*/  \
                        NEG                                                                     \
                    }                                                                           \
                }                                                                               \
            }                                                                                   \
        }                                                                                       \
}

//rd = rt >> rs
#define rcShift3reg(op, rd, rs, rt)                                                             \
{                                                                                               \
        xRD->IsDirty = 1;                                                                       \
        if (rd==rt)                                                                             \
        {                                                                                       \
            MapRD;                                                                              \
                                                                                                \
            if (rs==rt)  /*rd=rs=rt*/                                                           \
            {                                                                                   \
                MOV_Reg2ToReg1(1, Reg_ECX, (_u8)xRD->x86reg);                                   \
                op(1, (_u8)xRD->x86reg);                                                        \
            }                                                                                   \
            else /*rd=rt, rs!=rt*/                                                              \
            {                                                                                   \
                MapRS;                                                                          \
                MOV_Reg2ToReg1(1, Reg_ECX, (_u8)xRS->x86reg);                                   \
                op(1, (_u8)xRD->x86reg);                                                        \
            }                                                                                   \
        }                                                                                       \
        else /*rd != rt */                                                                      \
        {                                                                                       \
            if (rd==rs)                                                                         \
            {                                                                                   \
                MapRD;                                                                          \
                MapRT;                                                                          \
                MOV_Reg2ToReg1(1, Reg_ECX,          (_u8)xRD->x86reg);                          \
                MOV_Reg2ToReg1(1, (_u8)xRD->x86reg, (_u8)xRT->x86reg);                          \
                op(1, (_u8)xRD->x86reg);                                                        \
            }                                                                                   \
            else  /* rd != rs */                                                                \
            {                                                                                   \
                xRD->NoNeedToLoadTheLo = 1;                                                     \
                MapRD;                                                                          \
                if (rs == rt)                                                                   \
                {                                                                               \
                    if (rt == 0) /* rd = (rt=0) >> whatever */                                  \
                        Reg2ToReg1(1, _Xor, (_u8)xRD->x86reg, (_u8)xRD->x86reg); /* xor rd, rd */\
                    else                                                                        \
                    {                                                                       \
                        MapRT;                                                              \
                        MOV_Reg2ToReg1(1, Reg_ECX,          (_u8)xRT->x86reg);    /* mov cx, rt */ \
                        MOV_Reg2ToReg1(1, (_u8)xRD->x86reg, (_u8)xRT->x86reg);    /* mov rd, rt */ \
                        op(1, (_u8)xRD->x86reg);                         /* sar rd, cl */ \
                    }                                                                       \
                }                                                                               \
                else  /* rs != rt */                                                            \
                {                                                                               \
                    if (rs != 0)                                                                \
                    {                                                                           \
                        MapRS;                                                                  \
                        /* schibo:- if rt == 0..what here ? */                                  \
                        /* rs!=0, rt!=0 , rs!=rt */                                             \
                        MapRT;                                                                  \
                        MOV_Reg2ToReg1(1, Reg_ECX,          (_u8)xRS->x86reg); /* mov cx, rs */ \
                        MOV_Reg2ToReg1(1, (_u8)xRD->x86reg, (_u8)xRT->x86reg); /* mov rd, rt */ \
                        op(1,    (_u8)xRD->x86reg);                   /* sar rd, cl */ \
                    }                                                                           \
                    else   /*rs=0, rt!=0 */                                                     \
                    {                                                                           \
                        MapRT;                                                                  \
                        MOV_Reg2ToReg1(1, (_u8)xRD->x86reg, (_u8)xRT->x86reg); /* mov  rd,rt*/  \
                    }                                                                           \
                }                                                                               \
            }                                                                                   \
        }                                                                                       \
}

#define Do3Add(op, NEG, subtraction)    rcAdd3reg(op, xRD->mips_reg, xRS->mips_reg, xRT->mips_reg, NEG, subtraction)

#define rcShift(modbase, rd, rt)                                   \
{                                                                  \
    _u8 mod_rm;                                                    \
    rd = (_u8)__RD;                                                \
    rt = (_u8)__RT;                                                \
                                                                   \
    xRD->mips_reg = (_u8)__RD;                                     \
                                                                   \
    if (rd==rt)                                                    \
    {                                                              \
        xRD->IsDirty = 1;                                          \
        MapRD;                                                     \
        mod_rm = modbase|((_u8)xRD->x86reg);                       \
                                                                   \
        WC8(0xC1);                                                 \
        WC8((_u8)mod_rm);                                          \
        WC8((_u8)__SA);                                            \
    }                                                              \
    else                                                           \
    {                                                              \
        xRD->IsDirty = 1;                                          \
        xRD->NoNeedToLoadTheLo = 1;                                \
        MapRD;                                                     \
        mod_rm = modbase|((_u8)xRD->x86reg);                       \
        if (rt == 0)                                                \
        {                                                           \
            XOR_Reg1ToReg2(1, (_u8)xRD[0].x86reg, (_u8)xRD[0].x86reg); \
        }                                                              \
        else                                                           \
        {                                                              \
            MapRT;                                                     \
                                                                       \
            /* mov      rd,rt*/                                        \
            MOV_Reg2ToReg1(1, (_u8)xRD[0].x86reg, (_u8)xRT[0].x86reg); \
            WC8(0xC1);                                                 \
            WC8((_u8)mod_rm);                                          \
            WC8((_u8)__SA);                                            \
        }                                                              \
    }                                                                  \
}





    



#define SetLoHiRsRt32bit                                            \
    memset(xLO, 0, sizeof(xLO));                                    \
    memset(xHI, 0, sizeof(xHI));                                    \
    memset(xRS, 0, sizeof(xRS));                                    \
    memset(xRT, 0, sizeof(xRT));                                    \
                                                                    \
    xLO->mips_reg = __LO; xRT->mips_reg = __RT; xRS->mips_reg = __RS;    xHI->mips_reg = __HI; \
    xLO->Is32bit  =   1;  xRT->Is32bit = 1;     xRS->Is32bit = 1;        xHI->Is32bit  = 1;




#define SetRsRt64bit                                                \
    memset(xRD, 0, sizeof(xRD));                                    \
    memset(xRS, 0, sizeof(xRS));                                    \
    memset(xRT, 0, sizeof(xRT));                                    \
                                                                    \
    xRD->mips_reg = __RD; xRT->mips_reg = __RT; xRS->mips_reg = __RS;


#endif //_REGCACHE_H__1964_