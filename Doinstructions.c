//DoInstructions.c

//TODO: optimize:
//Make these functions static inline void.
//Get the Parse routine out of the functions and put it in the 
//RunOpcode loop. Otherwise, inline makes code huuuge!

#include <stdio.h>
#include "chipmake.h"
#include "mainregs.h"

#ifdef _DEBUG
extern char* DebugMainCPU();
extern char* DebugMainCPUReg();
extern char* DebugCOP0(uint8 fmt);
extern char* DebugCOP0Reg(uint8 reg);
extern char* DebugRegimm(uint8 Instruction);
extern char* DebugSpecial(uint8 function);
extern uint8 UpdateViewPort;
#endif _DEBUG

extern void RunOpcode();

extern void Parse6_5_5_16();
extern void Parse6_26();
//void ParseCOPxInstruction();

//////////////////////////////////////////////////////////////////////
//                 Main Processor Opcodes
//         syntax: Op rt, offset(base)         
//////////////////////////////////////////////////////////////////////

void lb() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void lbu() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void ld() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void ldl() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void ldr() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void lh() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void lhu() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void ll() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void lld() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void lw() {
uint32* tempPtr;
Parse6_5_5_16();
tempPtr = &buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
MainCPUReg[rt_ft] = *tempPtr;

#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
//	printf("*tempPtr = %X\n", *tempPtr);
#endif
}

void lwl() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void lwr() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void lwu() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void sb() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void sc() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void scd() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void sd() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void sdl() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void sdr() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void sh() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}


//-----------------------------------------------------------------
//| SW        | Store Word                                        |
//|-----------|---------------------------------------------------|
//|101011 (43)|  base   |   rt    |             offset            |
//------6----------5---------5-------------------16----------------
// Format:  SW rt, offset(base)
// Purpose: To store a word to memory.
// Descrip: word[base+offset] = rt
void sw() {
uint32* tempPtr;
Parse6_5_5_16();
#ifdef _DEBUG
	if (UpdateViewPort)
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif

tempPtr = &buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
*tempPtr = MainCPUReg[rt_ft];
}

void swl() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void swr() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

//////////////////////////////////////////////////////////////////////

//				COP0 Instructions
//syntax        Op rt, fs

//////////////////////////////////////////////////////////////////////

void cfc0() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugCOP0(rs_base_fmt), DebugMainCPUReg(rt_ft), DebugCOP0Reg(rd_fs));
#endif
}

void ctc0() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugCOP0(rs_base_fmt), DebugMainCPUReg(rt_ft), DebugCOP0Reg(rd_fs));
#endif
}

void dmfc0() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugCOP0(rs_base_fmt), DebugMainCPUReg(rt_ft), DebugCOP0Reg(rd_fs));
#endif
}

void dmtc0() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugCOP0(rs_base_fmt), DebugMainCPUReg(rt_ft), DebugCOP0Reg(rd_fs));
#endif
}

//SB 11/4/99
//-----------------------------------------------------------------
//| MFC0      | Move word From CP0                                |
//|-----------|---------------------------------------------------|
//|  010000   |00000 (0)|   rt    |   fs    |    0000 0000 000    |
//------6----------5---------5---------5--------------11-----------
// Format:  MFC0 rt, rd
// Purpose: To copy a word from an FPR to a GPR.
// Descrip: rt = COP rd
void mfc0() {
	Parse6_5_5_5_5_6(); // TODO : FIXME!
	MainCPUReg[rt_ft] = COP0Reg[rd_fs];
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugCOP0(rs_base_fmt),
DebugMainCPUReg(rt_ft), DebugCOP0Reg(rd_fs));
#endif
}

//SB 11/4/99
//-----------------------------------------------------------------
//| MTC0      | Move word to CP0                                  |
//|-----------|---------------------------------------------------|
//|  010000   |00100 (4)|   rt    |   fs    |    0000 0000 000    |
//------6----------5---------5---------5--------------11-----------
// Format:  MTC0 rt, rd
// Purpose: To copy a word from a GPR to an FPR.
// Descrip: COP rd = rt
void mtc0() {
	Parse6_5_5_5_5_6(); // TODO : FIXME!
	COP0Reg[rd_fs] = MainCPUReg[rt_ft];
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugCOP0(rs_base_fmt),
DebugMainCPUReg(rt_ft), DebugCOP0Reg(rd_fs));
#endif
}

//////////////////////////////////////////////////////////////////////

//				COP1 Instructions 
//    syntax:   Op rt, fs

//////////////////////////////////////////////////////////////////////

void cfc1() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugCOP0(rs_base_fmt), DebugMainCPUReg(rt_ft), DebugCOP0Reg(rd_fs));
#endif
}

void ctc1() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugCOP0(rs_base_fmt), DebugMainCPUReg(rt_ft), DebugCOP0Reg(rd_fs));
#endif
}

void dmfc1() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugCOP0(rs_base_fmt), DebugMainCPUReg(rt_ft), DebugCOP0Reg(rd_fs));
#endif
}

void dmtc1() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugCOP0(rs_base_fmt), DebugMainCPUReg(rt_ft), DebugCOP0Reg(rd_fs));
#endif
}

void mfc1() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugCOP0(rs_base_fmt), DebugMainCPUReg(rt_ft), DebugCOP0Reg(rd_fs));
#endif
}

void mtc1() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugCOP0(rs_base_fmt), DebugMainCPUReg(rt_ft), DebugCOP0Reg(rd_fs));
#endif
}

//////////////////////////////////////////////////////////////////////

//			COP2 Instructions (floats) 
//  syntax: Op rt, rd

//////////////////////////////////////////////////////////////////////

void cfc2() {
#ifdef _DEBUG
	printf("%X: %s\t%Xh,%Xh %s\n", pc, DebugCOP0(rs_base_fmt), rt_ft, rd_fs);
#endif
}

void ctc2() {
#ifdef _DEBUG
	printf("%X: %s\t%Xh,%Xh %s\n", pc, DebugCOP0(rs_base_fmt), rt_ft, rd_fs);
#endif
}

void dmfc2() {
#ifdef _DEBUG
	printf("%X: %s\t%Xh,%Xh %s\n", pc, DebugCOP0(rs_base_fmt), rt_ft, rd_fs);
#endif
}

void dmtc2() {
#ifdef _DEBUG
	printf("%X: %s\t%Xh,%Xh %s\n", pc, DebugCOP0(rs_base_fmt), rt_ft, rd_fs);
#endif
}

void mfc2() {
#ifdef _DEBUG
	printf("%X: %s\t%Xh,%Xh %s\n", pc, DebugCOP0(rs_base_fmt), rt_ft, rd_fs);
#endif
}

void mtc2() {
#ifdef _DEBUG
	printf("%X: %s\t%Xh,%Xh %s\n", pc, DebugCOP0(rs_base_fmt), rt_ft, rd_fs);
#endif
}

//////////////////////////////////////////////////////////////////////

//			MainCPU Instructions (floats) 
//  syntax: Op rt, rd

//////////////////////////////////////////////////////////////////////

void ldc2() {
#ifdef _DEBUG
	printf("%X: %s\t%Xh,%Xh %s\n", pc, DebugCOP0(rs_base_fmt), rt_ft, rd_fs);
#endif
}

void lwc2() {
#ifdef _DEBUG
	printf("%X: %s\t%Xh,%Xh %s\n", pc, DebugCOP0(rs_base_fmt), rt_ft, rd_fs);
#endif
}

void sdc2() {
#ifdef _DEBUG
	printf("%X: %s\t%Xh,%Xh %s\n", pc, DebugCOP0(rs_base_fmt), rt_ft, rd_fs);
#endif
}

void swc2() {
#ifdef _DEBUG
	printf("%X: %s\t%Xh,%Xh %s\n", pc, DebugCOP0(rs_base_fmt), rt_ft, rd_fs);
#endif
}

//////////////////////////////////////////////////////////////////////
//
//			LUI (MainCPU Instruction)  
//  syntax: Op rt, immediate
//
//////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------
//| LUI       | Load Upper Immediate                              |
//|-----------|---------------------------------------------------|
//|001111 (15)|  00000  |   rt    |           immediate           |
//------6----------5---------5-------------------16----------------
// Format:  LUI rt, immediate
// Purpose: To load a constant into the upper half of a word.
// Descrip: rt = immediate * 10000h
void lui() {
Parse6_5_5_16();
	MainCPUReg[rt_ft] = offset_immediate << 16;
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate);
#endif
}

//////////////////////////////////////////////////////////////////////
//
//			More MainCPU Instructions (with immediate)  
//  syntax: Op rt, rs, immediate
//
//////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------
//| ADDIU     | ADD Immediate Unsigned word                       |
//|-----------|---------------------------------------------------|
//|001001 (9) |   rs    |   rt    |          immediate            |
//------6----------5---------5-------------------16----------------
// Format:  ADDIU rt, rs, immediate
// Purpose: To add a constant to a 32-bit integer.
// Descrip: rt = rs + immediate
void addiu() {
Parse6_5_5_16();
MainCPUReg[rt_ft] = MainCPUReg[rs_base_fmt] + (unsigned)offset_immediate;
#ifdef _DEBUG
	printf("%X: %s\t%2s,%s,%04Xh\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), DebugMainCPUReg(rs_base_fmt), (unsigned)offset_immediate);
#endif
}

//-----------------------------------------------------------------
//| ADDI      | ADD Immediate word                                |
//|-----------|---------------------------------------------------|
//|001000 (8) |   rs    |   rt    |          immediate            |
//------6----------5---------5-------------------16----------------
// Format:  ADDI rt, rs, immediate
// Purpose: To add a constant to a 32-bit integer.
//          If overflow occurs, then trap.
// Descrip: rt = rs + immediate
void addi() {
Parse6_5_5_16();
	MainCPUReg[rt_ft] = MainCPUReg[rs_base_fmt] + offset_immediate;
#ifdef _DEBUG
	if (UpdateViewPort)
	printf("%X: %s\t%2s,%s,%04Xh\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

void andi() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%s,%04Xh\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

void daddi() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%s,%04Xh\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}


//-----------------------------------------------------------------
//| ORI       | OR Immediate                                      |
//|-----------|---------------------------------------------------|
//|001101 (13)|   rs    |   rt    |            immediate          |
//------6----------5---------5-------------------16----------------
// Format:  ORI rt, rs, immediate
// Purpose: To do a bitwise logical OR with a constant.
// Comment: ORI rt, r0, imm is equal to a LI rt, imm
// Descrip: rt = (rs OR immediate)
void ori() {
Parse6_5_5_16();
	MainCPUReg[rt_ft] = MainCPUReg[rs_base_fmt] | offset_immediate;
#ifdef _DEBUG
	printf("%X: %s\t%2s,%s,%04Xh\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

void slti() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%s,%04Xh\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

void sltiu() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%s,%04Xh\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

//SB 11/4/99
//-----------------------------------------------------------------
//| XORI      | eXclusive OR Immediate                            |
//|-----------|---------------------------------------------------|
//|001110 (14)|   rs    |   rt    |           immediate           |
//------6----------5---------5-------------------16----------------
// Format:  XORI rt, rs, immediate
// Purpose: To do a bitwise logical EXCLUSIVE OR with a constant.
// Descrip: rd = (rs XOR immediate)
void xori() {
	Parse6_5_5_16();
	MainCPUReg[rd_fs] = MainCPUReg[rs_base_fmt] ^ offset_immediate;
#ifdef _DEBUG
	printf("%X: %s\t%2s,%s,%04Xh\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), DebugMainCPUReg(rs_base_fmt),
offset_immediate);
#endif
}

//////////////////////////////////////////////////////////////////////
//
//			Regimm Instructions 0-3, 16-19
//  syntax: Op rs, offset
//
//////////////////////////////////////////////////////////////////////

void bgez() {
//Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

void bgezall() {
//Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

void bgezl() {
//Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

void bltz() {
//Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

void bltzal() {
//Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

void bltzall() {
//Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

void bltzl() {
//Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

void bgezal() {
//Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

//////////////////////////////////////////////////////////////////////
//
//			More Regimms (with immediate); Instructions 8-12, 14
//  syntax: Op rs, immediate
//
//////////////////////////////////////////////////////////////////////

void teqi() {
//Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

void tgei() {
//Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

void tgeiu() {
//Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

void tlti() {
//Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

void tltiu() {
//Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

void tnei() {
//Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

//Special Instruction JALR (syntax: JALR rs, rd)
void jalr() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rd_fs));
#endif
}

//////////////////////////////////////////////////////////////////////
//
//			Special Instructions
//  syntax: Op rd, rs, rt
//
//////////////////////////////////////////////////////////////////////

void add() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

//SB 11/4/99
//-----------------------------------------------------------------
//| ADDU      | ADD Unsigned word                                 |
//|-----------|---------------------------------------------------|
//|  000000   |   rs    |   rt    |    rd   |  00000  |100001 (33)|
//------6----------5---------5---------5---------5----------6------
// Format:  ADDU rd, rs, rt
// Purpose: To add 32-bit integers.
// Descrip: rd = rs + rt
void addu() {
	Parse6_5_5_5_5_6();
	MainCPUReg[rd_fs] = MainCPUReg[rs_base_fmt] +
(unsigned)MainCPUReg[rt_ft];

#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp),
DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt),
DebugMainCPUReg(rt_ft));
#endif
}

//SB 11/4/99
//-----------------------------------------------------------------
//| AND       | AND                                               |
//|-----------|---------------------------------------------------|
//|  000000   |   rs    |   rt    |    rd   |  00000  |100100 (36)|
//------6----------5---------5---------5---------5----------6------
// Format:  AND rd, rs, rt
// Purpose: To do a bitwise logical AND.
// Descrip: rd = (rs AND rt)
void and() {
	Parse6_5_5_5_5_6();
	MainCPUReg[rd_fs] = MainCPUReg[rs_base_fmt] & MainCPUReg[rt_ft];
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp),
DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt),
DebugMainCPUReg(rt_ft));
#endif
}

void dadd() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void daddu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsllv() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsub() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsubu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void nor() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

//SB 11/4/99
//-----------------------------------------------------------------
//| OR        | OR                                                |
//|-----------|---------------------------------------------------|
//|  000000   |   rs    |   rt    |    rd   |  00000  |100101 (37)|
//------6----------5---------5---------5---------5----------6------
// Format:  OR rd, rs, rt
// Purpose: To do a bitwise logical OR.
// Descrip: rd = (rs OR rt)
void or() {
	Parse6_5_5_5_5_6();
	MainCPUReg[rd_fs] = MainCPUReg[rs_base_fmt] | MainCPUReg[rt_ft];
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp),
DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt),
DebugMainCPUReg(rt_ft));
#endif
}

//-----------------------------------------------------------------
//| SLT       | Set on Less Than                                  |
//|-----------|---------------------------------------------------|
//|  000000   |   rs    |   rt    |    rd   |  00000  |101010 (42)|
//------6----------5---------5---------5---------5----------6------
// Format:  SLT rd, rs, rt
// Purpose: To record the result of a less-than comparison.
// Descrip: if rs < rt then rd = 1 else rd = 0

void slt() {
	Parse6_5_5_5_5_6();
	if (MainCPUReg[rs_base_fmt] < MainCPUReg[rt_ft])
		MainCPUReg[rd_fs] = 1;
	else
		MainCPUReg[rd_fs] = 0;
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp),
DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt),
DebugMainCPUReg(rt_ft));
#endif
}

//SB 11/4/99
//-----------------------------------------------------------------
//| SLTU      | Set on Less Than Unsigned                         |
//|-----------|---------------------------------------------------|
//|  000000   |   rs    |   rt    |    rd   |  00000  |101011 (43)|
//------6----------5---------5---------5---------5----------6------
// Format:  SLTU rd, rs, rt
// Purpose: To record the result of an unsigned less-than comparison.
// Descrip: if rs < rt then rd = 1 else rd = 0
void sltu() {
	Parse6_5_5_5_5_6();
	if ((unsigned)MainCPUReg[rs_base_fmt] < (unsigned)MainCPUReg[rt_ft])
		MainCPUReg[rd_fs] = 1;
	else
		MainCPUReg[rd_fs] = 0;
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp),
DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt),
DebugMainCPUReg(rt_ft));
#endif
}

void sllv() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void srav() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void srlv() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void sub() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void subu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

//SB 11/4/99
//-----------------------------------------------------------------
//| XOR       | eXclusive OR                                      |
//|-----------|---------------------------------------------------|
//|  000000   |   rs    |   rt    |    rd   |  00000  |100110 (38)|
//------6----------5---------5---------5---------5----------6------
// Format:  XOR rd, rs, rt
// Purpose: To do a bitwise logical EXCLUSIVE OR.
// Descrip: rd = (rs XOR rt)
void xor() {
	Parse6_5_5_5_5_6();
	MainCPUReg[rd_fs] = MainCPUReg[rs_base_fmt] ^ MainCPUReg[rt_ft];
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp),
DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt),
DebugMainCPUReg(rt_ft));
#endif
}

//////////////////////////////////////////////////////////////////////
//
//			Special Instructions
//  syntax: Op rd, rt, sa
//
//////////////////////////////////////////////////////////////////////

void dsll() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsll32() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsra() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsra32() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsrav() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsrl() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsrl32() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsrlv() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void sll() {
#ifdef _DEBUG
	if (rd_fs + rs_base_fmt + rt_ft == 0)
		printf("%X: NOP\n", pc);
	else
		printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void sra() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void srl() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

//////////////////////////////////////////////////////////////////////
//
//			More Special Instructions
//  syntax: Op rs, rt
//
//////////////////////////////////////////////////////////////////////
void ddiv() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void ddivu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void Div() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void divu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dmult() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dmultu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void mult() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void multu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void teq() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void tge() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void tgeu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void tlt() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void tltu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void tne() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

//////////////////////////////////////////////////////////////////////
//
//			More Special Instructions
//  syntax: Op rd
//
//////////////////////////////////////////////////////////////////////

void mfhi() {
#ifdef _DEBUG
	printf("%X: %s\t%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs));
#endif
}

void mflo() {
#ifdef _DEBUG
	printf("%X: %s\t%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs));
#endif
}

//////////////////////////////////////////////////////////////////////
//
//			More Special Instructions
//  syntax: Op rs
//
//////////////////////////////////////////////////////////////////////

void mthi() {
#ifdef _DEBUG
	printf("%X: %s\t%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt));
#endif
}

void mtlo() {
#ifdef _DEBUG
	printf("%X: %s\t%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt));
#endif
}


//-----------------------------------------------------------------
//| JR        | Jump Register                                     |
//|-----------|---------------------------------------------------|
//|  000000   |   rs    |     0000 0000 0000 000      |001000 (8) |
//------6----------5------------------15--------------------6------
// Format:  JR rs
// Purpose: To branch to an instruction address in a register.
void jr() {
	uint8 rs;
#ifdef _DEBUG
	printf("%X: %s\t%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rs_base_fmt));
#endif
#ifndef DISASM 
//  I know this is strange, but we need to do the NEXT opcode 
//	before actually jumping. 
	rs = rs_base_fmt;
	pc+=4;
	//Opcode = *InstructionPointer >> 2;
	RunOpcode(); //Do the next Opcode.

	pc = MainCPUReg[rs];
	//Pointer goes to where we are now.
	InstructionPointer = &buffer[pc-MainStartAddr+CodeStart];
	pc-=4;
#endif
//By default, pc increases by 4, which explains the -4 here.
//This makes for less code than conditionally adding 4 at each opcode
//and may be quicker. (We'll see.)
//Be sure to do -4 at each jump and branch instruction.
}


//////////////////////////////////////////////////////////////////////
//
//			More MainCPU Instructions
//  syntax: Op rs, rt, offset
//
//////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
//| BEQ       | Branch on EQual                                   |
//|-----------|---------------------------------------------------|
//|000100 (4) |   rs    |   rt    |            offset             |
//------6----------5---------5-------------------16----------------
// Format:  BEQ rs, rt, offset
// Purpose: To compare GPRs then do a PC-relative conditional branch.
// Comment: BEQ rs, r0, offset is equal to a BEQZ rs, offset
//          BEQ r0, r0, offset is equal to a B offset
// Descrip: branch if rs = rt
void beq() {
//_int16 Offset;

Parse6_5_5_16();

#ifdef _DEBUG
	if (UpdateViewPort)
	printf("%X: %s\t%s,%s,%04Xh\n", pc, DebugMainCPU(), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft), offset_immediate);
#endif

#ifndef DISASM 
//  I know this is strange, but we need to do the NEXT opcode 
//	before actually jumping. 
	if (MainCPUReg[rs_base_fmt] == MainCPUReg[rt_ft]) {
		pc+=4;
		Counter = offset_immediate*4 + pc;
#ifdef _DEBUG
		if (Counter < 0) UpdateViewPort = 0;
#endif
		Opcode = *InstructionPointer >> 2;
		RunOpcode(); //Do the next Opcode.

		InstructionPointer = &buffer[Counter-MainStartAddr+CodeStart];
		pc = Counter - 4;
	}
#ifdef _DEBUG
	else
		UpdateViewPort = 1;
#endif // end #ifdef _DEBUG
#endif //end #ifdef DISASM
//By default, pc increases by 4, which explains the -4 here.
//This makes for less code than conditionally adding 4 at each opcode
//and may be quicker. (We'll see.)
//Be sure to do -4 at each jump and branch instruction.
}

void beql() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%04Xh\n", pc, DebugMainCPU(), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft), offset_immediate);
#endif
}


//-----------------------------------------------------------------
//| BNE       | Branch on Not Equal                               |
//|-----------|---------------------------------------------------|
//|000101 (5) |   rs    |   rt    |            offset             |
//------6----------5---------5-------------------16----------------
// Format:  BNE rs, rt, offset
// Purpose: To compare GPRs then do a PC-relative conditional branch.
// Comment: BNE rs, r0, offset is equal to a BNEZ rs, offset
// Descrip: branch if rs <> rt
void bne() {
//_int16 Offset;

Parse6_5_5_16();

#ifdef _DEBUG
	if (UpdateViewPort)
	printf("%X: %s\t%s,%s,%04Xh\n", pc, DebugMainCPU(), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft), offset_immediate*4 + pc+4);
#endif

#ifndef DISASM 
//  I know this is strange, but we need to do the NEXT opcode 
//	before actually jumping. 
	if (MainCPUReg[rs_base_fmt] != MainCPUReg[rt_ft]) {
		pc+=4;
		Counter = offset_immediate*4 + pc;
#ifdef _DEBUG
		if (Counter < pc) UpdateViewPort = 0;
		//printf("Counter = %X\n", Counter);
		//printf("pc =      %X\n", Counter);
#endif
		//Instruction = *InstructionPointer++;
		//Opcode = Instruction >> 26;
		RunOpcode(); //Do the next Opcode.

		InstructionPointer = &buffer[Counter-MainStartAddr+CodeStart];
		pc = Counter - 4;
	}
#ifdef _DEBUG
	else
		UpdateViewPort = 1;
#endif // end #ifdef _DEBUG
#endif //end #ifdef DISASM
//By default, pc increases by 4, which explains the -4 here.
//This makes for less code than conditionally adding 4 at each opcode
//and may be quicker. (We'll see.)
//Be sure to do -4 at each jump and branch instruction.
}

void bnel() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%04Xh\n", pc, DebugMainCPU(), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft), offset_immediate);
#endif
}

//////////////////////////////////////////////////////////////////////
//
//			More MainCPU Instructions
//  syntax: Op ft, offset(bsae)
//
//////////////////////////////////////////////////////////////////////

void ldc1() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void lwc1() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void sdc1() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

void swc1() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}

//////////////////////////////////////////////////////////////////////
//
//			More MainCPU Instructions
//  syntax: Op instr_index
//
//////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
//| JAL       | Jump And Link                                     |
//|-----------|---------------------------------------------------|
//|000011 (3) |                    instr_index                    |
//------6-------------------------------26-------------------------
// Format:  JAL target
// Purpose: To procedure call within the current 256 MB aligned region.
// Descrip: return address in ra

//This baby is the heart of the high-level emulation.
//We perform functions based on the instr_index.

void jal() {
Parse6_26();

#ifdef _DEBUG
	printf("%X: %s\t%08X\n", pc, DebugMainCPU(), instr_index);
#endif

#ifndef DISASM 
//  I know this is strange, but we need to do the NEXT opcode 
//	before actually jumping. 
	//rs = rs_base_fmt;
	
	//Set RA register to return address.
//	MainCPUReg[RA] = pc + 8;
	
	pc+=4;
	Opcode = *InstructionPointer >> 2;
	RunOpcode(); //Do the next Opcode.
	
	//pc = MainCPUReg[rs];
	//Pointer goes to where we are now.
	//here's the heart of the high level stuff.
	switch(instr_index) {
	case 0x800CC0C0 : ; break; // osInitialize.
//	case 0x800CBF70 : ; break; // osCreateThread #10.
	case 0x800CC360 : ; break; // osStartThread #11.
	case 0x80000450 : ; break; //unknown. Ignore.
	default :
		pc = instr_index;
		InstructionPointer = &buffer[instr_index-MainStartAddr+CodeStart];
		pc-=4;
	}
	
#endif
//By default, pc increases by 4, which explains the -4 here.
//This makes for less code than conditionally adding 4 at each opcode
//and may be quicker. (We'll see.)
//Be sure to do -4 at each jump and branch instruction.
}

//////////////////////////////////////////////////////////////////////
//                 Main Processor Opcode - CACHE
//         syntax: CACHE op, offset(base)         
//////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------
//| CACHE     | CACHE                                             |
//|-----------|---------------------------------------------------|
//|101111 (47)|  base   |   op    |             offset            |
//------6----------5---------5-------------------16----------------
// Format:  CACHE op, offset(base)
// Purpose: The 16-bit offset is sign-extended and added to the contents of
//         general register base to form a virtual address. The virtual
//          address is translated to a physical address using the TLB, and the
//          5-bit sub-opcode specifies a cache operation for that address.

void cache() {
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s", pc, DebugMainCPU());
	Opcode = rt_ft;
	printf("\t%2s,%04Xh(%s)\n", DebugMainCPU(),  offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
}