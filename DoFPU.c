//DoFPU,c
//Thanks to anarko for the docs!

#include "chipmake.h"
#include "mainregs.h"


/*
****************************************************************************
** Floating-point Unit, FPU (COP1) instructions                           **
****************************************************************************
 fmt = 10000b (.S, Single)   fmt = 10001b (.D, Double)
 fmt = 10100b (.W, Word)     fmt = 10101b (.L, Long)
*/

/*
-----------------------------------------------------------------
| ABS.fmt   | floating-point ABSolute value                     |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |  00000  |   fs    |   fd    | 000101 (5)|
------6----------5---------5---------5---------5----------6------
 Format:  ABS.S fd, fs
          ABS.D fd, fs
 Purpose: To compute the absolute value of an FP value.
*/
void Abs() {
#ifdef _DEBUG
	//printf("%X: %s", pc, DebugFPU(function);
#endif
}

/*
-----------------------------------------------------------------
| ADD.fmt   | floating-point ADD                                |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |   ft    |   fs    |   fd    | 000000 (0)|
------6----------5---------5---------5---------5----------6------
 Format:  ADD.S fd, fs, ft
          ADD.D fd, fs, ft
 Purpose: To add FP values.
*/
void add_f() {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
| BC1F      | Branch on FP False                                |
|-----------|---------------------------------------------------|
|  010001   |01000 (8)| 000|nd|tf|           offset             |
|           |         |    | 0| 0|                              |
------6----------5-------3---1--1--------------16----------------
 Format:  BC1F offset
 Purpose: To test an FP condition code and do a PC-relative
          conditional branch.
*/
void bc1f() {
#ifdef _DEBUG
#endif
}


/*
-----------------------------------------------------------------
| BC1FL     | Branch on FP False Likley                         |
|-----------|---------------------------------------------------|
|  010001   |01000 (8)| 000|nd|tf|           offset             |
|           |         |    | 1| 0|                              |
------6----------5-------3---1--1--------------16----------------
 Format:  BC1FL offset
 Purpose: To test an FP condition code and do a PC-relative conditional
          branch; execute the delay slot only if the branch is taken.
*/
void bc1fl() {
#ifdef _DEBUG
#endif
}



/*
-----------------------------------------------------------------
| BC1T      | Branch on FP True                                 |
|-----------|---------------------------------------------------|
|  010001   |01000 (8)| 000|nd|tf|           offset             |
|           |         |    | 0| 1|                              |
------6----------5-------3---1--1--------------16----------------
 Format:  BC1T offset
 Purpose: To test an FP condition code and do a PC-relative
          conditional branch.
*/
void bc1t() {
#ifdef _DEBUG
#endif
}


/*
-----------------------------------------------------------------
| BC1TL     | Branch on FP True Likley                          |
|-----------|---------------------------------------------------|
|  010001   |01000 (8)| 000|nd|tf|           offset             |
|           |         |    | 1| 1|                              |
------6----------5-------3---1--1--------------16----------------
 Format:  BC1TL offset
 Purpose: To test an FP condition code and do a PC-relative conditional
          branch; execute the delay slot only if the branch is taken.
*/
void bc1tl() {
#ifdef _DEBUG
#endif
}


/*
-----------------------------------------------------------------
| C.cond.fmt| floating-point Compare                            |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |   ft    |   fs    | 000 |00 |11 | cond  |
------6----------5---------5---------5-------3----2---2-----4----
 cond: 0000 = F (False)
       0001 = UN (Unordered)
       0010 = EQ (Equal)
       0011 = UEQ (Unordered or Equal)
       0100 = OLT (Ordered or Less Than)
       0101 = ULT (Unordered or Less Than)
       0110 = OLE (Ordered or Less than or Equal)
       0111 = ULE (Unordered or Less than or Equal)
       1000 = SF (Signaling False)
       1001 = NGLE (Not Greater than or Less than or Equal)
       1010 = SEQ (Signaling Equal)
       1011 = NGL (Not Greater than or Less than)
       1100 = LT (Less Than)
       1101 = NGE (Not Greater than or Equal)
       1110 = LE (Less than or Equal)
       1111 = NGT (Not Greater Than)
 Format:  C.cond.S fs, ft
          C.cond.D fs, ft
 Purpose: To compare FP values and record the Boolean result in
          a condition code.
*/
void c_cond() {
#ifdef _DEBUG
#endif
}



/*
-----------------------------------------------------------------
| CEIL.W.fmt| floating-point CEILing convert to Word fixed-point|
|-----------|---------------------------------------------------|
|  010001   |   fmt   |  00000  |   fs    |   fd    |001110 (14)|
------6----------5---------5---------5---------5----------6------
 Format:  CEIL.W.S fd, fs
          CEIL.W.D fd, fs
 Purpose: To convert an FP value to 32-bit fixed-point, rounding up.

-----------------------------------------------------------------
| CEIL.L.fmt| floating-point CEILing convert to Long fixed-point|
|-----------|---------------------------------------------------|
|  010001   |   fmt   |  00000  |   fs    |   fd    |001010 (10)|
------6----------5---------5---------5---------5----------6------
 Format:  CEIL.L.S fd, fs
          CEIL.L.D fd, fs
 Purpose: To convert an FP value to 64-bit fixed-point, rounding up.
*/
void Ceil(void* W_L) {
#ifdef _DEBUG
#endif
}


/*
-----------------------------------------------------------------
| CFC1      | Move control word From Floating-Point             |
|-----------|---------------------------------------------------|
|  010001   |00010 (2)|   rt    |   fs    |    0000 0000 000    |
------6----------5---------5---------5--------------11-----------
 Format:  CFC1 rt, fs
 Purpose: To copy a word from an FPU control register to a GPR.
 Descrip: rt = fs
*/
void cfc1() {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
| CTC1      | Move control word To Floating-Point               |
|-----------|---------------------------------------------------|
|  010001   |00110 (6)|   rt    |   fs    |    0000 0000 000    |
------6----------5---------5---------5--------------11-----------
 Format:  CTC1 rt, fs
 Purpose: To copy a word from a GPR to an FPU control register.
 Descrip: fs = rt
*/
void ctc1() {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
| CVT.D.fmt | floating-point ConVerT to Double floating-point   |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |  00000  |   fs    |   fd    |100001 (33)|
------6----------5---------5---------5---------5----------6------
 Format:  CVT.D.S fd, fs
          CVT.D.W fd, fs
          CVT.D.L fd, fs
 Purpose: To convert an FP or fixed-point value to double FP.

-----------------------------------------------------------------
| CVT.L.fmt | floating-point ConVerT to Long fixed-point        |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |  00000  |   fs    |   fd    |100101 (37)|
------6----------5---------5---------5---------5----------6------
 Format:  CVT.L.S fd, fs
          CVT.L.D fd, fs
 Purpose: To convert an FP value to a 64-bit fixed-point.

-----------------------------------------------------------------
| CVT.S.fmt | floating-point ConVerT to Single floating-point   |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |  00000  |   fs    |   fd    |100000 (32)|
------6----------5---------5---------5---------5----------6------
 Format:  CVT.S.D fd, fs
          CVT.S.W fd, fs
          CVT.S.L fd, fs
 Purpose: To convert an FP or fixed-point value to single FP.

-----------------------------------------------------------------
| CVT.W.fmt | floating-point ConVerT to Word fixed-point        |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |  00000  |   fs    |   fd    |100100 (36)|
------6----------5---------5---------5---------5----------6------
 Format:  CVT.W.S fd, fs
          CVT.W.D fd, fs
 Purpose: To convert an FP value to 32-bit fixed-point.
*/
void Cvt(void* S_D_W_L) {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
| DIV.fmt   | floating-point DIVide                             |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |    ft   |   fs    |   fd    | 000011 (3)|
------6----------5---------5---------5---------5----------6------
 Format:  DIV.S fd, fs, ft
          DIV.D fd, fs, ft
 Purpose: To divide FP values.
*/
void div_f() {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
| DMFC1     | Doubleword Move From Floating-Point               |
|-----------|---------------------------------------------------|
|  010001   |00001 (1)|   rt    |   fs    |    0000 0000 000    |
------6----------5---------5---------5--------------11-----------
 Format:  DMFC1 rt, fs
 Purpose: To copy a doubleword from an FPR to a GPR.
*/
void dmfc1() {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
| DMTC1     | Doubleword Move To Floating-Point                 |
|-----------|---------------------------------------------------|
|  010001   |00101 (5)|   rt    |   fs    |    0000 0000 000    |
------6----------5---------5---------5--------------11-----------
 Format:  DMTC1 rt, fs
 Purpose: To copy a doubleword from a GPR to an FPR.
*/
void dmtc1() {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
|FLOOR.L.fmt| floating-point FLOOR convert to Long fixed-point  |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |  00000  |   fs    |   fd    |001011 (11)|
------6----------5---------5---------5---------5----------6------
 Format:  FLOOR.L.S fd, fs
          FLOOR.L.D fd, fs
 Purpose: To convert an FP value to 64-bit fixed-point, rounding down.

-----------------------------------------------------------------
|FLOOR.W.fmt| floating-point FLOOR convert to Word fixed-point  |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |  00000  |   fs    |   fd    |001111 (15)|
------6----------5---------5---------5---------5----------6------
 Format:  FLOOR.W.S fd, fs
          FLOOR.W.D fd, fs
 Purpose: To convert an FP value to 32-bit fixed-point, rounding down.
*/
void Floor(void* W_L) {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
| LDC1      | Load Doubleword to Floating-Point                 |
|-----------|---------------------------------------------------|
|110101 (53)|   base  |   ft    |            offset             |
------6----------5---------5-------------------16----------------
 Format:  LDC1 ft, offset(base)
 Purpose: To load a doubleword from memory to an FPR.
 Descrip: COP ft = doubleword[base+offset]
*/
void ldc1() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

/*
-----------------------------------------------------------------
| LWC1      | Load Word to Floating-Point                       |
|-----------|---------------------------------------------------|
|110001 (49)|   base  |   ft    |            offset             |
------6----------5---------5-------------------16----------------
 Format:  LWC1 ft, offset(base)
 Purpose: To load a word from memory to an FPR.
 Comment: LWC1 ft, offset(base) is equal to a L.S ft, offset(base)
 Descrip: COP ft = word[base+offset]
*/
void lwc1() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

/*
-----------------------------------------------------------------
| MFC1      | Move Word From Floating-Point                     |
|-----------|---------------------------------------------------|
|  010001   |00000 (0)|   rt    |   fs    |    0000 0000 000    |
------6----------5---------5---------5--------------11-----------
 Format:  MFC1 rt, fs
 Purpose: To copy a word from an FPU (CP1) general register to a GPR.
 Descrip: rt = COP fs
*/
void mfc1() {
#ifdef _DEBUG
#endif
}


/*
-----------------------------------------------------------------
| MOV.fmt   | floating-point MOVe                               |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |  00000  |   fs    |   fd    | 000110 (6)|
------6----------5---------5---------5---------5----------6------
 Format:  MOV.S fd, fs
          MOV.D fd, fs
 Purpose: To move an FP value between FPRs.
*/
void mov() {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
| MTC1      | Move Word to Floating-Point                       |
|-----------|---------------------------------------------------|
|  010001   |00100 (4)|   rt    |   fs    |    0000 0000 000    |
------6----------5---------5---------5--------------11-----------
 Format:  MTC1 rt, fs
 Purpose: To copy a word from a GPR to an FPU (CP1) general register.
 Descrip: COP fs = rt
*/
void mtc1() {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
| MUL.fmt   | floating-point MULtiply                           |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |   ft    |   fs    |   fd    | 000010 (2)|
------6----------5---------5---------5---------5----------6------
 Format:  MUL.S fd, fs, ft
          MUL.D fd, fs, ft
 Purpose: To multiply FP values.
*/
void mul() {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
| NEG.fmt   | floating-point NEGate                             |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |  00000  |   fs    |   fd    | 000111 (7)|
------6----------5---------5---------5---------5----------6------
 Format:  NEG.S fd, fs
          NEG.D fd, fs
 Purpose: To negate an FP value.
*/
void neg() {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
|ROUND.L.fmt| floating-point ROUND to Long fixed-point          |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |  00000  |   fs    |   fd    | 001000 (8)|
------6----------5---------5---------5---------5----------6------
 Format:  ROUND.L.S fd, fs
          ROUND.L.D fd, fs
 Purpose: To convert an FP value to 64-bit fixed-point, rounding to nearest.

-----------------------------------------------------------------
|ROUND.W.fmt| floating-point ROUND to Word fixed-point          |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |  00000  |   fs    |   fd    |001100 (12)|
------6----------5---------5---------5---------5----------6------
 Format:  ROUND.W.S fd, fs
          ROUND.W.D fd, fs
 Purpose: To convert an FP value to 32-bit fixed-point, rounding to nearest.
*/
void round(void* W_L) {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
| SDC1      | Store Doubleword from Floating-Point              |
|-----------|---------------------------------------------------|
|111101 (61)|  base   |   ft    |             offset            |
------6----------5---------5-------------------16----------------
 Format:  SDC1 ft, offset(base)
 Purpose: To store a doubleword from an FPR to memory.
 Descrip: doubleword[base+offset] = COP rt
*/
void sdc1() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}


/*
-----------------------------------------------------------------
| SQRT.fmt  | floating-point SQuare RooT                        |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |  00000  |   fs    |   fd    | 000100 (4)|
------6----------5---------5---------5---------5----------6------
 Format:  SQRT.S fd, fs
          SQRT.D fd, fs
 Purpose: To compute the square root of an FP value.
*/
void Sqrt() {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
| SUB.fmt   | floating-point SUBtract                           |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |   ft    |   fs    |   fd    | 000001 (1)|
------6----------5---------5---------5---------5----------6------
 Format:  SUB.S fd, fs, ft
          SUB.D fd, fs, ft
 Purpose: To subtract FP values.
*/
void sub_f() {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
| SWC1      | Store Word from Floating-Point                    |
|-----------|---------------------------------------------------|
|111001 (57)|  base   |   ft    |             offset            |
------6----------5---------5-------------------16----------------
 Format:  SWC1 ft, offset(base)
 Purpose: To store a word from an FPR to memory.
 Comment: SWC1 ft, offset(base) is equal to a S.S ft, offset(base)
 Descrip: word[base+offset] = COP rt
*/
void swc1() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

/*
-----------------------------------------------------------------
|TRUNC.L.fmt| floating-point TRUNCate to Long fixed-point       |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |  00000  |   fs    |   fd    | 001001 (9)|
------6----------5---------5---------5---------5----------6------
 Format:  TRUNC.L.S fd, fs
          TRUNC.L.D fd, fs
 Purpose: To convert an FP value to 64-bit fixed-point, rounding toward zero.

-----------------------------------------------------------------
|TRUNC.W.fmt| floating-point TRUNCate to Word fixed-point       |
|-----------|---------------------------------------------------|
|  010001   |   fmt   |  00000  |   fs    |   fd    |001101 (13)|
------6----------5---------5---------5---------5----------6------
 Format:  TRUNC.W.S fd, fs
          TRUNC.W.D fd, fs
 Purpose: To convert an FP value to 32-bit fixed-point, rounding toward zero.
*/
void trunc(void* W_L) {
#ifdef _DEBUG
#endif
}
