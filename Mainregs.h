//MainRegs.h
//Thanks to anarko for the docs!

/*     CPU: Instructions encoded by opcode field.
     31---------26---------------------------------------------------0
     |  opcode   |                                                   |
     ------6----------------------------------------------------------
     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
 000 | *1    | *2    | J     | JAL   | BEQ   | BNE   | BLEZ  | BGTZ  |
 001 | ADDI  | ADDIU | SLTI  | SLTIU | ANDI  | ORI   | XORI  | LUI   |
 010 | *3    | *4    | *5    |  ---  | BEQL  | BNEL  | BLEZL | BGTZL |
 011 | DADDI |DADDIU | LDL   | LDR   |  ---  |  ---  |  ---  |  ---  |
 100 | LB    | LH    | LWL   | LW    | LBU   | LHU   | LWR   | LWU   |
 101 | SB    | SH    | SWL   | SW    | SDL   | SDR   | SWR   | CACHE |
 110 | LL    | LWC1  | *LWC2 |  ---  | LLD   | LDC1  | LDC2  | LD    |
 111 | SC    | SWC1  | *SWC2 |  ---  | SCD   | SDC1  | SDC2  | SD    |
  hi |-------|-------|-------|-------|-------|-------|-------|-------|
     *1 = SPECIAL, see SPECIAL list    *2 = REGIMM, see REGIMM list
     *3 = COP0                  *4 = COP1                   *5 = COP2
     *LWC2 = RSP Load instructions     *SWC2 = RSP Store instructions
*/

//Main CPU Opcodes
#define SPECIAL 0
#define REGIMM 1
#define J 2
#define JAL 3
#define BEQ 4
#define BNE 5
#define BLEZ 6
#define BGTZ 7
#define ADDI 8
#define ADDIU 9
#define SLTI 10
#define SLTIU 11
#define ANDI 12
#define ORI 13
#define XORI 14
#define LUI 15
#define COP0 16
#define COP1 17
#define COP2 18
#define BEQL 20
#define BNEL 21
#define BLEZL 22
#define BGTZL 23
#define DADDI 24
#define DADDIU 25
#define LDL 26
#define LDR 27
#define LB 32
#define LH 33
#define LWL 34
#define LW 35
#define LBU 36
#define LHU 37
#define LWR 38
#define LWU 39
#define SB 40
#define SH 41
#define SWL 42
#define SW 43
#define SDL 44
#define SDR 45
#define SWR 46
#define CACHE 47
#define LL 48
#define LWC1 49
#define LWC2 50
//51 unused
#define LLD 52
#define LDC1 53
#define LDC2 54
#define LD 55
#define SC 56
#define SWC1 57
#define SWC2 58
//59 unused
#define SCD 60
#define SDC1 61
#define SDC2 62
#define SD 63


/*
****************************************************************************
** Registers                                                              **
****************************************************************************
  Main CPU registers:
  -------------------
    00h = r0/reg0     08h = t0/reg8     10h = s0/reg16    18h = t8/reg24
    01h = at/reg1     09h = t1/reg9     11h = s1/reg17    19h = t9/reg25
    02h = v0/reg2     0Ah = t2/reg10    12h = s2/reg18    1Ah = k0/reg26
    03h = v1/reg3     0Bh = t3/reg11    13h = s3/reg19    1Bh = k1/reg27
    04h = a0/reg4     0Ch = t4/reg12    14h = s4/reg20    1Ch = gp/reg28
    05h = a1/reg5     0Dh = t5/reg13    15h = s5/reg21    1Dh = sp/reg29
    06h = a2/reg6     0Eh = t6/reg14    16h = s6/reg22    1Eh = s8/reg30
    07h = a3/reg7     0Fh = t7/reg15    17h = s7/reg23    1Fh = ra/reg31
*/
  
#define R0 0x00
#define AT 0x01
#define V0 0x02
#define V1 0x03
#define A0 0x04
#define A1 0x05
#define A2 0x06
#define A3 0x07
#define T0 0x08
#define T1 0x09
#define T2 0x0A
#define T3 0x0B
#define T4 0x0C
#define T5 0x0D
#define T6 0x0E
#define T7 0x0F
#define S0 0x10
#define S1 0x11
#define S2 0x12
#define S3 0x13
#define S4 0x14
#define S5 0x15
#define S6 0x16
#define S7 0x17
#define T8 0x18
#define T9 0x19
#define K0 0x1A
#define K1 0x1B
#define GP 0x1C
#define SP 0x1D
#define S8 0x1E
#define RA 0x1F

/*
  COP0 registers:
  ---------------
    00h = Index       08h = BadVAddr    10h = Config      18h = *RESERVED* 
    01h = Random      09h = Count       11h = LLAddr      19h = *RESERVED*
    02h = EntryLo0    0Ah = EntryHi     12h = WatchLo     1Ah = PErr
    03h = EntryLo1    0Bh = Compare     13h = WatchHi     1Bh = CacheErr
    04h = Context     0Ch = Status      14h = XContext    1Ch = TagLo
    05h = PageMask    0Dh = Cause       15h = *RESERVED*  1Dh = TagHi
    06h = Wired       0Eh = EPC         16h = *RESERVED*  1Eh = ErrorEPC
    07h = *RESERVED*  0Fh = PRevID      17h = *RESERVED*  1Fh = *RESERVED*
*/
#define INDEX     0x00
#define RANDOM    0x01
#define ENTRYLO0  0x02
#define ENTRYLO1  0x03
#define CONTEXT   0x04
#define PAGEMASK  0x05
#define WIRED     0x06
#define RESERVED0 0x07
#define BADVADDR  0x08
#define COUNT     0x09
#define ENTRYHI   0x0A
#define COMPARE   0x0B
#define STATUS    0x0C
#define CAUSE     0x0D
#define EPC       0x0E
#define PREVID    0x0F
#define CONFIG    0x10
#define LLADDR    0x11
#define WATCHLO   0x12
#define WATCHHI   0x13
#define XCONTEXT  0x14
#define RESERVED1 0x15
#define RESERVED2 0x16
#define RESERVED3 0x17
#define RESERVED4 0x18
#define RESERVED5 0x19
#define PERR      0x1A
#define CACHEERR  0x1B
#define TAGLO     0x1C
#define TAGHI     0x1D
#define ERROREPC  0x1E
#define RESERVED6 0x1F


/*
****************************************************************************
** COPROCESSOR INSTRUCTIONS                                               **
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

    COP1 - Floating Point Unit (FPU)
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

	Reality Coprocessor: (RCP)
    COP2: Instructions encoded by the fmt field when opcode = COP2.
    31--------26-25------21 ----------------------------------------0
    |  = COP2   |   fmt   |                                         |
    ------6----------5-----------------------------------------------
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
 00 | MFC2  |  ---  | CFC2  |  ---  | MTC2  |  ---  | CTC2  |  ---  |
 01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 10 |  *1   |  *1   |  *1   |  *1   |  *1   |  *1   |  *1   |  *1   |
 11 |  *1   |  *1   |  *1   |  *1   |  *1   |  *1   |  *1   |  *1   |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
     *1 = Vector opcode
*/
#define MFC      0
#define DMFC     1
#define CFC      2
#define MTC      4
#define DMTC     5
#define CTC      6
#define BC       8
#define TLB     16
#define S_INSTR 16
#define D_INSTR 17
#define W_INSTR 18
#define L_INSTR 19

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
#define TLBR  1
#define TLBWI 2
#define TLBWR 6
#define TLBP  8
#define ERET  24


/*
****************************************************************************
** COP1 - Floating Point Unit (FPU)                                       **
****************************************************************************

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
//BC1 Instructions
#define BC1F  0
#define BC1T  1
#define BC1FL 2
#define BC1TL 3
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
#define ADD_F   0
#define SUB_F   1
#define MUL     2
#define DIV_F   3
#define SQRT    4
#define ABS     5
#define MOV     6
#define NEG     7
#define ROUND_L 8
#define TRUNC_L 9
#define CEIL_L  10
#define FLOOR_L 11
#define ROUND_W 12
#define TRUNC_W 13
#define CEIL_W  14
#define FLOOR_W 15
#define CVT_S   32
#define CVT_D   33
#define CVT_W   36
#define CVT_L   37
#define C_F     48
#define C_UN    49
#define C_EQ    50
#define C_UEQ   51
#define C_OLD   52
#define C_ULT   53
#define C_OLE   54
#define C_ULE   55
#define C_SF    56
#define C_NGLE  57
#define C_SEQ   58
#define C_NGL   59
#define C_LT    60
#define C_NGE   61
#define C_LE    62
#define C_NGT   63


/*
****************************************************************************
** COP2 - Reality Coprocessor (RCP)                                       **
****************************************************************************

    RSP Load: Instr. encoded by rd field when opcode field = LWC2
    31---------26-------------------15-------11---------------------0
    |  110010   |                   |   rd   |                      |
    ------6-----------------------------5----------------------------
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
 00 |  LBV  |  LSV  |  LLV  |  LDV  |  LQV  |  LRV  |  LPV  |  LUV  |
 01 |  LHV  |  LFV  |  LWV  |  LTV  |  ---  |  ---  |  ---  |  ---  |
 10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
*/
#define LBV 0
#define LSV 1
#define LLV 2
#define LDV 3
#define LQV 4
#define LRV 5
#define LPV 6
#define LUV 7
#define LHV 8
#define LFV 9
#define LWV 10
#define LTV 11

/*
   RSP Store: Instr. encoded by rd field when opcode field = SWC2
    31---------26-------------------15-------11---------------------0
    |  111010   |                   |   rd   |                      |
    ------6-----------------------------5----------------------------
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
 00 |  SBV  |  SSV  |  SLV  |  SDV  |  SQV  |  SRV  |  SPV  |  SUV  |
 01 |  SHV  |  SFV  |  SWV  |  STV  |  ---  |  ---  |  ---  |  ---  |
 10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
*/
#define SBV 0
#define SSV 1
#define SLV 2
#define SDV 3
#define SQV 4
#define SRV 5
#define SPV 6
#define SUV 7
#define SHV 8
#define SFV 9
#define SWV 10
#define STV 11


/*
    Vector opcodes: Instr. encoded by the function field when opcode = COP2.
    31---------26---25------------------------------------5---------0
    |  = COP2   | 1 |                                     | function|
    ------6-------1--------------------------------------------6-----
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
000 | VMULF | VMULU | VRNDP | VMULQ | VMUDL | VMUDM | VMUDN | VMUDH |
001 | VMACF | VMACU | VRNDN | VMACQ | VMADL | VMADM | VMADN | VMADH |
010 | VADD  | VSUB  | VSUT? | VABS  | VADDC | VSUBC | VADDB?| VSUBB?|
011 | VACCB?| VSUCB?| VSAD? | VSAC? | VSUM? | VSAW  |  ---  |  ---  |
100 |  VLT  |  VEQ  |  VNE  |  VGE  |  VCL  |  VCH  |  VCR  | VMRG  |
101 | VAND  | VNAND |  VOR  | VNOR  | VXOR  | VNXOR |  ---  |  ---  |
110 | VRCP  | VRCPL | VRCPH | VMOV  | VRSQ  | VRSQL | VRSQH |  ---  |
110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
    Comment: Those with a ? in the end of them may not exist
*/
#define VMULF 0
#define VMULU 1
#define VRNDP 2
#define VMULQ 3
#define VMUDL 4
#define VMUDM 5
#define VMUDN 6
#define VMUDH 7
#define VMACF 8
#define VMACU 9
#define VRNDN 10
#define VMACQ 11
#define VMADL 12
#define VMADM 13
#define VMADN 14
#define VMADH 15
#define VADD  16
#define VSUB  17

#define VSUT  18 //may not exist

#define VABS  19
#define VADDC 20
#define VSUBC 21

#define VADDB 22 //
#define VSUBB 23 //
#define VACCB 24 //
#define VSUCB 25 //these 7 may not exist
#define VSAD  26 //
#define VSAC  27 //
#define VSUM  28 //

#define VSAW  29 
#define VLT   32
#define VEQ   33
#define VNE   34
#define VGE   35
#define VCL   36
#define VCH   37
#define VCR   38
#define VMRG  39
#define VAND  40
#define VNAND 41
#define VOR   42
#define VNOR  43
#define VXOR  44
#define VNXOR 45
#define VRCP  48
#define VRCPL 49
#define VRCPH 50
#define VMOV  51
#define VRSQ  52
#define VRSQL 53
#define VRSQH 54

//     TLB: Instructions encoded by the function field when opcode
//          = COP0 and fmt = TLB.
//     31--------26-25------21 -------------------------------5--------0
//     |  = COP0   |  = TLB  |                               | function|
//     ------6----------5-----------------------------------------6-----
//     |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
// 000 |  ---  | TLBR  | TLBWI |  ---  |  ---  |  ---  | TLBWR |  ---  |
// 001 | TLBP  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 011 | ERET  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 100 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 101 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
// 111 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
//  hi |-------|-------|-------|-------|-------|-------|-------|-------|
//TLB Instructions
#define TLBR  1
#define TLBWI 2
#define TLBWR 6
#define TLBP  8
#define ERET  24


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
  
//SPECIAL Instructions:
#define SLL 0
#define SRL 2
#define SRA 3
#define SLLV 4
#define SRLV 6
#define SRAV 7
#define JR 8
#define JALR 9
#define SYSCALL 12
#define BREAK 13
#define SYNC 15
#define MFHI 16
#define MTHI 17
#define MFLO 18
#define MTLO 19
#define DSLLV 20
#define DSRLV 22
#define DSRAV 23
#define MULT 24
#define MULTU 25
#define DIV 26
#define DIVU 27
#define DMULT 28
#define DMULTU 29
#define DDIV 30
#define DDIVU 31
#define ADD 32
#define ADDU 33
#define SUB 34
#define SUBU 35
#define AND 36
#define OR 37
#define XOR 38
#define NOR 39
#define SLT 42
#define SLTU 43
#define DADD 44
#define DADDU 45
#define DSUB 46
#define DSUBU 47
#define TGE 48
#define TGEU 49
#define TLT 50
#define TLTU 51
#define TEQ 52
#define TNE 54
#define DSLL 56
#define DSRL 58
#define DSRA 59
#define DSLL32 60
#define DSRL32 62
#define DSRA32 63


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
//REGIMM Opcodes
#define BLTZ 0
#define BGEZ 1
#define BLTZL 2
#define BGEZL 3
#define TGEI 8
#define TGEIU 9
#define TLTI 10
#define TLTIU 11
#define TEQI 12
#define TNEI 14
#define BLTZAL 16
#define BGEZAL 17
#define BLTZALL 18
#define BGEZALL 19