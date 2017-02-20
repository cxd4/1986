#ifndef _REGCACHE_H__1964_
#define _REGCACHE_H__1964_


//#define NO_CONSTS         1
//#define SAFE_SLT          1
//#define SAFE_GATES        1
//#define SAFE_MATH         1
//#define SAFE_IMM          1
//#define SAFE_LOADSTORE    1
//#define SAFE_LOADSTORE_FPU 1
//#define SAFE_SHIFTS       1

#define _Xor 0x33
#define NUM_CONSTS 34

extern unsigned long	lCodePosition;
extern unsigned char	*RecompCode;

#ifdef USE_STACK
#define ItIsARegisterNotToUse(k) ((k == Reg_EBP) || (k == Reg_ESP)) //Don't use these registers for mapping
#define  StoreGPR_LO(k) \
{                       \
    if (x86reg[k].mips_reg == 0);    \
    else if ( x86reg[k].mips_reg < 16) \
        { /* MOV_RegToRegDisp8()*/WC8(0x89); WC8((_u8)(0x45+(k<<3))); WC8((_u8)(-128+(x86reg[k].mips_reg<<3)));}/*__asm { mov eax, dword ptr [ebp] }*/\
    else if (x86reg[k].mips_reg < 32)    \
        { /* MOV_RegToRegDisp8()*/WC8(0x89); WC8((_u8)(0x45+(k<<3))); WC8((_u8)((x86reg[k].mips_reg-16)<<3));}/*__asm { mov eax, dword ptr [ebp] }*/\
    else    \
        MOV_RegToMemory(1, (_u8)k, ModRM_disp32, (_u32)&gHWS_GPR[0]+(x86reg[k].mips_reg<<3));  \
}

#define  LoadGPR_LO(k) \
{                       \
    if (x86reg[k].mips_reg == 0)    \
        XOR_Reg2ToReg1(1, (_u8)k, (_u8)k);    \
    else if ( x86reg[k].mips_reg < 16) \
        { /* MOV_RegToRegDisp8()*/WC8(0x8b); WC8((_u8)(0x45+(k<<3))); WC8((_u8)(-128+(x86reg[k].mips_reg<<3)));}/*__asm { mov eax, dword ptr [ebp] }*/\
    else if (x86reg[k].mips_reg < 32) \
    { \
        /* MOV_RegDisp8ToReg()*/WC8(0x8b); WC8((_u8)(0x45+(k<<3))); WC8((_u8)((x86reg[k].mips_reg-16)<<3));}/*__asm { mov eax, dword ptr [ebp] }*/\
    else    \
        MOV_MemoryToReg(1, (_u8)k, ModRM_disp32, (_u32)&gHWS_GPR[0]+(x86reg[k].mips_reg<<3));  \
}

#define  LoadGPR_HI(k) \
{                       \
    if (x86reg[k].mips_reg == 0)    \
        XOR_Reg2ToReg1(1, (_u8)x86reg[k].HiWordLoc, (_u8)x86reg[k].HiWordLoc);    \
    else if ( x86reg[k].mips_reg < 16) \
        { /* MOV_RegToRegDisp8()*/WC8(0x8b); WC8((_u8)(0x45+(x86reg[k].HiWordLoc<<3))); WC8((_u8)(-124+(x86reg[k].mips_reg<<3)));}/*__asm { mov eax, dword ptr [ebp] }*/\
    else if (x86reg[k].mips_reg < 32) \
    { \
        /* MOV_RegDisp8ToReg()*/WC8(0x8b); WC8((_u8)(0x45+(x86reg[k].HiWordLoc<<3))); WC8((_u8)(4+((x86reg[k].mips_reg-16)<<3)));}/*__asm { mov eax, dword ptr [ebp] }*/\
    else    \
        MOV_MemoryToReg(1, (_u8)x86reg[k].HiWordLoc, ModRM_disp32, 4+(_u32)&gHWS_GPR[0]+(x86reg[k].mips_reg<<3));\
}

#define  StoreGPR_HI(k) \
{                       \
    if (x86reg[k].mips_reg == 0);    \
    else if ( x86reg[k].mips_reg < 16) \
        { /* MOV_RegToRegDisp8()*/WC8(0x89); WC8((_u8)(0x45+(x86reg[k].HiWordLoc<<3))); WC8((_u8)(-124+(x86reg[k].mips_reg<<3)));}/*__asm { mov eax, dword ptr [ebp] }*/\
    else if (x86reg[k].mips_reg < 32) \
    { \
        /* MOV_RegDisp8ToReg()*/WC8(0x89); WC8((_u8)(0x45+(x86reg[k].HiWordLoc<<3))); WC8((_u8)(4+((x86reg[k].mips_reg-16)<<3)));}/*__asm { mov eax, dword ptr [ebp] }*/\
    else    \
        MOV_RegToMemory(1, x86reg[k].HiWordLoc, ModRM_disp32, 4+(_u32)&gHWS_GPR[0]+(x86reg[k].mips_reg<<3));\
}
#else
#define ItIsARegisterNotToUse(k) ((k == Reg_ESP)) //Don't use these registers for mapping
#define  LoadGPR_LO(k) MOV_MemoryToReg(1, k, ModRM_disp32, (_u32)&gHWS_GPR[0]+(x86reg[k].mips_reg<<3));
#define  LoadGPR_HI(k) MOV_MemoryToReg(1, x86reg[k].HiWordLoc, ModRM_disp32, 4+(_u32)&gHWS_GPR[0]+(x86reg[k].mips_reg<<3));
#define StoreGPR_LO(k) MOV_RegToMemory(1, k,                   ModRM_disp32,   (_u32)&gHWS_GPR[0]+(x86reg[k].mips_reg<<3));
#define StoreGPR_HI(k) MOV_RegToMemory(1, x86reg[k].HiWordLoc, ModRM_disp32, 4+(_u32)&gHWS_GPR[0]+(x86reg[k].mips_reg<<3));

#endif

typedef struct x86regtyp
{
    uint32 BirthDate;
    _int8  HiWordLoc;
    _int8  Is32bit;
    _int8  IsDirty;
    _int8  mips_reg;
    _int8  NoNeedToLoadTheLo;
    _int8  NoNeedToLoadTheHi;
    _int8  x86reg;

} x86regtyp;

//Keeps status of constants
typedef struct MapConstant
{
    _int32 value;
    _int32 IsMapped;
} MapConstant;


//Keeps status of registers stored to memory
typedef struct FlushedMap
{
    uint32 Is32bit;
} FlushedMap;

uint32 ThisYear;
MapConstant ConstMap[NUM_CONSTS];

////////////////////////////////////////////////
//
// Multi-Pass definitions
//
////////////////////////////////////////////////

#define CHECK_OPCODE_PASS   if (gMultiPass.WhichPass == COMPILE_MAP_ONLY) { SwitchToOpcodePass(); return; }

enum PASSTYPE 
{
    COMPILE_MAP_ONLY,
    COMPILE_OPCODES_ONLY,
    COMPILE_ALL
};

typedef struct MultiPass
{
    int UseOnePassOnly;   // Make this a rom option. default option will be yes (1).
    int WhichPass;        // Mapping pass or opcode pass
    int WriteCode;        // Whether or not we write code on this pass
    int PhysAddrAfterMap; // Phyical Start address of 1st instruction after the block's first mappings.
    int VirtAddrAfterMap; // Virtual ""
    int JumpToPhysAddr;
    int pc;
    int MapPass_Count;
    int OpcodePass_Count;

} MultiPass;
MultiPass gMultiPass;


////////////////////////////////////////////////
//
// Mnemonics
//
////////////////////////////////////////////////

#define __OPCODE        ((_u8)(reg->code >> 26))
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

#define __dotRS         (((_u8)(gHWS_code >> 21)) & 0x1f)
#define __dotRT         (((_u8)(gHWS_code >> 16)) & 0x1f)
#define __dotRD         (((_u8)(gHWS_code >> 11)) & 0x1f)
#define __dotSA         (((_u8)(gHWS_code >>  6)) & 0x1f)

#define __dotI         ( (_s32)(_s16)gHWS_code )
#define __dotO             ( gHWS_pc + 4 + (__dotI << 2) )
#define ____dotT        (gHWS_code & 0x3ffffff)
#define __dotT          ( (gHWS_pc & 0xf0000000) | (____dotT << 2) )
#define __dotF          ( (_u8)(gHWS_code)       & 0x3f)

#define __ND                ((uint8)((reg->code >> 17) & 0x1))
#define __TF                ((uint8)((reg->code >> 16) & 0x1))


////////////////////////////////////////////////
//
// Function Declarations
//
////////////////////////////////////////////////


extern int CheckIs32Bit(int mips_reg);
extern void FlushAllRegisters();
extern void FlushOneButNotThese3(int The1stOneNotToFlush, int The2ndOneNotToFlush, int The3rdOneNotToFlush);
extern void MapRegister(x86regtyp* Conditions, int The2ndOneNotToFlush, int The3rdOneNotToFlush);
extern int CheckRegStatus(x86regtyp* Query);
extern void InitRegisterMap();
extern void FlushRegister(int k);
extern void WriteBackDirty(_int8 k);
extern void FlushOneConstant(int k);
extern void MapOneConstantToRegister(x86regtyp* Conditions, int The2ndOneNotToFlush, int The3rdOneNotToFlush);
extern int CheckWhereIsMipsReg(int mips_reg);
void PUSH_RegIfMapped(int k);
void POP_RegIfMapped(int k);

#define MapRD MapRegister(xRD, xRS->mips_reg, xRT->mips_reg);
#define MapRS MapRegister(xRS, xRD->mips_reg, xRT->mips_reg);
#define MapRT MapRegister(xRT, xRD->mips_reg, xRS->mips_reg);

#define DoConstAdd(Constant)                                                    \
{                                                                               \
    if (xRS->mips_reg==xRT->mips_reg)                                           \
    {                                                                           \
        if (ConstMap[xRS->mips_reg].IsMapped == 1)                              \
        {                                                                       \
            (_s32)ConstMap[xRT->mips_reg].value += (_s32)Constant;              \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            xRT->IsDirty = 1; /*bugfix: it will convert to 32bit */             \
            MapRT;                                                              \
                                                                                \
            if ((Constant) != 0)                                                \
            {                                                                   \
                ADD_ImmToReg(1, xRT->x86reg, Constant);                         \
            }                                                                   \
        }                                                                       \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        if (xRS->mips_reg == 0)                                                 \
        {                                                                       \
/*#ifndef NO_CONSTS                                                          */ \
/*           int tempy = CheckWhereIsMipsReg(xRT->mips_reg);                 */ \
/*            ConstMap[xRT->mips_reg].IsMapped = 1;                          */ \
/*            ConstMap[xRT->mips_reg].value = Constant;                      */ \
/*                                                                           */ \
/*            if (tempy > -1)                                                */ \
/*            {                                                              */ \
/*                x86reg[tempy].IsDirty = 0;                                 */ \
/*                FlushRegister(tempy);                                      */ \
/*            }                                                              */ \
/*#else                                                                      */ \
            xRT->IsDirty = 1;                                                   \
            xRT->Is32bit = 1;                                                   \
            xRT->NoNeedToLoadTheLo = 1;                                         \
            MapRT;                                                              \
            MOV_ImmToReg(1, xRT->x86reg, Constant);                             \
/*#endif                                                                     */ \
        }                                                                                   \
        else                                                                                \
        {                                                                                   \
            if (ConstMap[xRS->mips_reg].IsMapped && ConstMap[xRT->mips_reg].IsMapped)       \
            {                                                                               \
                (_s32)ConstMap[xRT->mips_reg].value = (_s32)ConstMap[xRS->mips_reg].value + (_s32)Constant;   \
            }                                                                               \
            else if (ConstMap[xRS->mips_reg].IsMapped)                                      \
            {                                                                               \
                xRT->IsDirty = 1;                                                           \
                xRT->NoNeedToLoadTheLo = 1;                                                 \
                MapRT                                                                       \
                MOV_ImmToReg(1, (_u8)xRT->x86reg, (_s32)ConstMap[xRS->mips_reg].value+(_s32)(Constant));\
            }                                                                               \
            else  /* (rs != rt) && (rs != 0) */                                             \
            {                                                                               \
                ConstMap[xRT->mips_reg].IsMapped = 0;                                       \
                xRT->IsDirty = 1;                                                           \
                xRT->NoNeedToLoadTheLo = 1;                                                 \
                MapRT                                                                       \
                MapRS;                                                                      \
                                                                                            \
                MOV_Reg2ToReg1(1, (_u8)xRT->x86reg, (_u8)xRS->x86reg); /* mov   rt,rs    */ \
                ADD_ImmToReg(1, (_u8)xRT->x86reg, (_s32)(Constant));   /* add   rt,Const */ \
            }                                                                               \
        }                                                                                   \
    }                                                                                       \
}


#define NEGATE(REG)   {XOR_ShortToReg(1, (_u8)REG->x86reg,  0xFF); INC_Reg(1, (_u8)REG->x86reg);}

#define NONNEG {}

#define rcAdd3reg(op, rd, rs, rt, NEG, subtraction)                                             \
{                                                                                               \
    rd = (_u8)__RD;                                                                             \
    rs = (_u8)__RS;                                                                             \
    rt = (_u8)__RT;                                                                             \
                                                                                                \
    if (rd==rt)                                                                                 \
    {                                                                                           \
        if (rs==rt)  /*rd=rs=rt*/                                                               \
        {                                                                                       \
            if (ConstMap[xRT->mips_reg].IsMapped == 1)                                          \
            {                                                                                   \
                if (subtraction)                                                                \
                    (_s32)ConstMap[xRT->mips_reg].value -= (_s32)ConstMap[xRT->mips_reg].value; \
                else                                                                            \
                    (_s32)ConstMap[xRT->mips_reg].value += (_s32)ConstMap[xRT->mips_reg].value; \
            }                                                                                   \
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
            ADD_Reg2ToReg1(1, xRD->x86reg, xRS->x86reg);                                    \
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
                    XOR_Reg2ToReg1(1, xRD->x86reg, xRD->x86reg); /* xor rd, rd */           \
                else                                                                        \
                    if (subtraction) /* rd = rs-rs */                                       \
                        XOR_Reg2ToReg1(1, xRD->x86reg, xRD->x86reg);                        \
                    else  /* rd = rs+rs */                                                  \
                    {                                                                       \
                        MapRT;                                                              \
                        MOV_Reg2ToReg1(1, xRD->x86reg, xRT->x86reg);                        \
                        ADD_Reg2ToReg1(1, xRD->x86reg, xRD->x86reg);   /* shl rd,1 uses 1 cycle also, but only pairing for the u pipe */\
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

#define Do3Add(op, NEG, subtraction)    rcAdd3reg(op, xRD->mips_reg, xRS->mips_reg, xRT->mips_reg, NEG, subtraction)

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