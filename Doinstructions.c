//DoInstructions.c

//TODO: optimize:
//Make these functions static inline void.
//Get the Parse routine out of the functions and put it in the 
//RunOpcode loop. Otherwise, inline makes code huuuge!

#include <stdio.h>
#include "chipmake.h"
#include "mainregs.h"

#define DOUBLEMASKRIGHT 0x0000ffff
#define DOUBLEMASKLEFT 0xffff0000
#define WORDMASKRIGHT 0x00ff
#define WORDMASKLEFT 0xff00

#ifdef _DEBUG
extern char* DebugMainCPU();
extern char* DebugMainCPUReg();
extern char* DebugCOP0(uint8 fmt);
extern char* DebugCOP0Reg(uint8 reg);
extern char* DebugRegimm(uint8 Instruction);
extern char* DebugSpecial(uint8 function);
extern uint8 UpdateViewPort;
#endif _DEBUG

extern int LoadMemory(_int64* targetReg);
extern int StoreMemory(unsigned long regValue);

extern void RunOpcode();

extern void Parse6_5_5_16();
extern void Parse6_26();
//void ParseCOPxInstruction();

//////////////////////////////////////////////////////////////////////
//                 Main Processor Opcodes
//         syntax: Op rt, offset(base)         
//////////////////////////////////////////////////////////////////////

//Eudaemon Apr 14, 1999
//--------------------------------------------------------------
//| LB            |  Load Byte       |
//--------------------------------------------------------------
//|100000 (32) | Base |          rt        |        offset   |
//|-------6--------|---5----|--------5-------|-----16--------|
// Format:  LB rt, offset(base)
// Purpose: Loads Byte from Memory to rt as a signed value
// rt = byte[base+offset]
void lb() {

_int8 targetByte ; // Byte that will be read from memory (signed)

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

targetByte =
buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
MainCPUReg[rt_ft] = targetByte;

}

//Eudaemon Apr 14, 1999
//--------------------------------------------------------------
//| LBU            |  Load Byte (unsigned)         |
//--------------------------------------------------------------
//|100100 (36) | Base |          rt        |        offset   |
//|-------6--------|---5----|--------5-------|-----16--------|
// Format: LBU rt, offset(base)
// Purpose: Load an unsigned byte from memory to rt
// rt = (unsigned)byte[base+offset]

void lbu() {

uint8 targetByte;  // Byte that will be read from memory

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

targetByte =
buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
MainCPUReg[rt_ft] = targetByte;

}

//15 april 99 Eudaemon
//--------------------------------------------------------------
//| LD            |  Load DoubleWord         |
//--------------------------------------------------------------
//|100100 (36) | Base |          rt        |        offset   |
//|-------6--------|---5----|--------5-------|-----16--------|
// Format: LD rt, offset(base)
// Pupose: Loads a DoubleWord (64 Bits) from memory to rt
// rt = doubleword[offset+base]

void ld() {

_int64 targetDouble; //Double Word to be loaded from memory

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft),
offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif

targetDouble =
buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
MainCPUReg[rt_ft] = targetDouble;

}

//15 april 99 Eudaemon
//--------------------------------------------------------------
//| LDL            |  Load DoubleWord Left         |
//--------------------------------------------------------------
//|011010 (26) | Base |          rt        |        offset   |
//|-------6--------|---5----|--------5-------|-----16--------|
// Format: LDL rt, offset(base)
// Pupose: Loads the most significant bits from a
//         DoubleWord (64 Bits) from memory to the least significant bits
//         of rt
// left[rt] = right[doubleword[offset+base]]

void ldl() {

_int64 targetDouble; //Double Word to be loaded from memory

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

//Mask the least significant bits of rt
MainCPUReg[rt_ft] = MainCPUReg[rt_ft] & DOUBLEMASKLEFT;
//Load and Mask the most significant bits of the doubleWord
targetDouble =
buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
targetDouble = targetDouble & DOUBLEMASKRIGHT;
//Combine the resulting bits
MainCPUReg[rt_ft] = MainCPUReg[rt_ft] | targetDouble;

}

//15 april 99 Eudaemon
//--------------------------------------------------------------
//| LDR            |  Load DoubleWord Right         |
//--------------------------------------------------------------
//|011011 (27) | Base |          rt        |        offset   |
//|-------6--------|---5----|--------5-------|-----16--------|
// Format: LDR rt, offset(base)
// Pupose: Loads the Least significant bits from a
//         DoubleWord (64 Bits) from memory to the Most significant bits
//         of rt
// right[rt] = left[doubleword[offset+base]]

void ldr() {

_int64 targetDouble; //Double Word to be loaded from memory

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

//Mask the most significant bits of rt
MainCPUReg[rt_ft] = MainCPUReg[rt_ft] & DOUBLEMASKRIGHT;
//Load and Mask the least significant bits of the doubleWord
targetDouble =
buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
targetDouble = targetDouble & DOUBLEMASKLEFT;
//Combine the resulting bits
MainCPUReg[rt_ft] = MainCPUReg[rt_ft] | targetDouble;

}

//15 april 99 Eudaemon
//--------------------------------------------------------------
//| LH            |  Load Halfword         |
//--------------------------------------------------------------
//|100001 (33) | Base |          rt        |        offset   |
//|-------6--------|---5----|--------5-------|-----16--------|
// Format: LH rt, offset(base)
// Pupose: Loads a halfword to rt
// rt = halfword[offset+base]

void lh() {

_int16 targetHalf; //Half Word to load to rt

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

targetHalf =
buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
MainCPUReg[rt_ft] = targetHalf;

}

//15 april 99 Eudaemon
//--------------------------------------------------------------
//| LHU            |  Load Halfword Unsigned        |
//--------------------------------------------------------------
//|100101 (37) | Base |          rt        |        offset   |
//|-------6--------|---5----|--------5-------|-----16--------|
// Format: LHU rt, offset(base)
// Pupose: Loads a unsigned halfword to rt
// rt = (unsigned)halfword[offset+base]

void lhu() {

uint16 targetHalf; //unsigned Halfword to load to rt

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

targetHalf =
buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
MainCPUReg[rt_ft] = targetHalf;

}

//15 april 99 Eudaemon
//Is this correct?
//--------------------------------------------------------------
//| LL            |  Load Linked Word       |
//--------------------------------------------------------------
//|110000 (48) | Base |          rt        |        offset   |
//|-------6--------|---5----|--------5-------|-----16--------|
// Format: LL rt, offset(base)
// Pupose: Loads a word to rt for load-modify-write
// rt = word[offset+base]

void ll() {

_int32 targetWord; //Word to load to rt

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

targetWord =
buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
MainCPUReg[rt_ft] = targetWord;

}

//15 april 99 Eudaemon
//Is this correct?
//--------------------------------------------------------------
//| LLD            |  Load Linked DoubleWord       |
//--------------------------------------------------------------
//|110100 (52) | Base |          rt        |        offset   |
//|-------6--------|---5----|--------5-------|-----16--------|
// Format: LLD rt, offset(base)
// Pupose: Loads a doubleword to rt for atomic read-modify-write
// rt = Doubleword[offset+base]

void lld() {

_int64 targetDouble; //Double to be read from

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

targetDouble =
buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
MainCPUReg[rt_ft] = targetDouble;

}

//15 april 99 Eudaemon
//--------------------------------------------------------------
//| LW            |  Load Word         |
//--------------------------------------------------------------
//|100011 (35) | Base |          rt        |        offset   |
//|-------6--------|---5----|--------5-------|-----16--------|
// Format: LW rt, offset(base)
// Pupose: Loads a Word to rt
// rt = word[offset+base]

void lw() {

//_int32 targetWord; //Word to load to rt

Parse6_5_5_16();

#ifdef _DEBUG
printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

// printf("*tempPtr = %X\n", *tempPtr);
#endif

//targetWord =
//buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
//MainCPUReg[rt_ft] = targetWord;
LoadMemory(MainCPUReg[rt_ft]);
}

//15 april 99 Eudaemon
//--------------------------------------------------------------
//| LWL            |  Load Word Left        |
//--------------------------------------------------------------
//|100010 (34) | Base |          rt        |        offset   |
//|-------6--------|---5----|--------5-------|-----16--------|
// Format: LWL rt, offset(base)
// Pupose: Loads the most significant bits of a Word to
//         the least significant bits rt
// left[rt] = right[word[offset+base]]

void lwl() {

_int32 targetWord; //Word to load to rt

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

//Mask the least significant bits of rt
MainCPUReg[rt_ft] = MainCPUReg[rt_ft] & WORDMASKLEFT;
//Load and Mask the most significant bits of the Word
targetWord =
buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
targetWord = targetWord & WORDMASKRIGHT;
//Combine the resulting bits
MainCPUReg[rt_ft] = MainCPUReg[rt_ft] | targetWord;

}

//15 april 99 Eudaemon
//--------------------------------------------------------------
//| LWR            |  Load Word Right        |
//--------------------------------------------------------------
//|100110 (38) | Base |          rt        |        offset   |
//|-------6--------|---5----|--------5-------|-----16--------|
// Format: LWR rt, offset(base)
// Pupose: Loads the Least significant bits of a Word to
//         the most significant bits rt
// right[rt] = left[word[offset+base]]

void lwr() {

_int32 targetWord; //Word to load to rt

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

//Mask the most significant bits of rt
MainCPUReg[rt_ft] = MainCPUReg[rt_ft] & WORDMASKRIGHT;
//Load and Mask the least significant bits of the Word
targetWord =
buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
targetWord = targetWord & WORDMASKLEFT;
//Combine the resulting bits
MainCPUReg[rt_ft] = MainCPUReg[rt_ft] | targetWord;

}

//15 april 99 Eudaemon
//--------------------------------------------------------------
//| LWU            |  Load Word Unsigned        |
//--------------------------------------------------------------
//|100111 (39) | Base |          rt        |        offset   |
//|-------6--------|---5----|--------5-------|-----16--------|
// Format: LWU rt, offset(base)
// Pupose: Loads an unsigned Word to rt
// rt = (unsigned)word[offset+base]

void lwu() {

uint32 targetWord; //Word to be loaded to rt

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

targetWord =
buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
MainCPUReg[rt_ft] = targetWord;

}

//15 april 99 Eudaemon
//--------------------------------------------------------------
//| SB            |  Store Byte       |
//--------------------------------------------------------------
//|101000 (40) | Base |          rt        |        offset   |
//|-------6--------|---5----|--------5-------|-----16--------|
// Format: SB rt, offset(base)
// Pupose: Store a byte to memory
// byte[base+offset]=rt

void sb() {

_int8* ptrByte; //Pointer to byte to store

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif
ptrByte =
&buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
*ptrByte = MainCPUReg[rt_ft];

}

//15 april 99 Eudaemon
//--------------------------------------------------------------
//| SC            |  Store Conditional Word       |
//--------------------------------------------------------------
//|111000 (56) | Base |          rt        |        offset   |
//|-------6--------|---5----|--------5-------|-----16--------|
// Format: SC rt, offset(base)
// Pupose: Store a word to memory to complete an atomic read-modify-write
// word[base+offset]=rt

void sc() {

_int32* ptrWord;

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

ptrWord =
&buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
*ptrWord = MainCPUReg[rt_ft];

}

//15 april 99 Eudaemon
//--------------------------------------------------------------
//| SCD            |  Store Conditional DoubleWord       |
//--------------------------------------------------------------
//|111100 (60) | Base |          rt        |        offset   |
//|-------6--------|---5----|--------5-------|-----16--------|
// Format: SC rt, offset(base)
// Pupose: Store a doubleword to memory to complete an atomic read-modify-write
// doubleword[base+offset]=rt

void scd() {

_int32* ptrdoubleWord;

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

ptrdoubleWord =
&buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
*ptrdoubleWord = MainCPUReg[rt_ft];


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
//uint32* tempPtr;
Parse6_5_5_16();
#ifdef _DEBUG
	if (UpdateViewPort)
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
	StoreMemory(MainCPUReg[rt_ft]);
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

//Themedes
void andi() {
Parse6_5_5_16();
	MainCPUReg[rt_ft] = MainCPUReg[rs_base_fmt] & offset_immediate;
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

//SB 11/4/99
//-----------------------------------------------------------------
//| SLTI      | Set on Less Than Immediate                        |
//|-----------|---------------------------------------------------|
//|001010 (10)|    rs   |   rt    |            immediate          |
//------6----------5---------5-------------------16----------------
// Format:  SLTI rt, rs, immediate
// Purpose: To record the result of a less-than comparison with a constant.
// Descrip: if rs < immediate then rd = 1 else rd = 0

void slti() {
Parse6_5_5_16();
	if (MainCPUReg[rs_base_fmt] < offset_immediate)
		MainCPUReg[rd_fs] = 1;
	else
		MainCPUReg[rd_fs] = 0;
#ifdef _DEBUG
	printf("%X: %s\t%2s,%s,%04Xh\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

//SB 11/4/99
//-----------------------------------------------------------------
//| SLTIU     | Set on Less Than Immediate Unsigned               |
//|-----------|---------------------------------------------------|
//|001011 (11)|   rs    |   rt    |            immediate          |
//------6----------5---------5-------------------16----------------
// Format:  SLTIU rt, rs, immediate
// Purpose: To record the result of an unsigned less-than
//          comparison with a constant.
// Descrip: if rs < immediate then rd = 1 else rd = 0
void sltiu() {
Parse6_5_5_16();
	if ((unsigned)MainCPUReg[rs_base_fmt] < (unsigned)offset_immediate)
		MainCPUReg[rd_fs] = 1;
	else
		MainCPUReg[rd_fs] = 0;
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

//SB 11/4/99
//-----------------------------------------------------------------
//| ADD       | ADD word                                          |
//|-----------|---------------------------------------------------|
//|  000000   |   rs    |   rt    |   rd    |  00000  |100000 (32)|
//------6----------5---------5---------5---------5----------6------
// Format:  ADD rd, rs, rt
// Purpose: To add 32-bit integers. If overflow occurs, then trap.
// Comment: ADD rd, r0, rs is equal to a MOVE rd, rs
// Descrip: rd = rs + rt

void add() {
	Parse6_5_5_5_5_6();
	MainCPUReg[rd_fs] = MainCPUReg[rs_base_fmt] + MainCPUReg[rt_ft];

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

//SB 11/4/99
//-----------------------------------------------------------------
//| NOR       | Not OR                                            |
//|-----------|---------------------------------------------------|
//|  000000   |   rs    |   rt    |    rd   |  00000  |100111 (39)|
//------6----------5---------5---------5---------5----------6------
// Format:  NOR rd, rs, rt
// Purpose: To do a bitwise logical NOT OR.
// Descrip: rd = (rs NOR rt)
void nor() {
	Parse6_5_5_5_5_6();
	MainCPUReg[rd_fs] = ~(MainCPUReg[rs_base_fmt] | MainCPUReg[rt_ft]);
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

//Themedes 
void sub() {
Parse6_5_5_5_5_6();
	MainCPUReg[rd_fs] = MainCPUReg[rs_base_fmt] - MainCPUReg[rt_ft];
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(SpecialOp), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

//Themedes 
void subu() {
Parse6_5_5_5_5_6();
	MainCPUReg[rd_fs] = MainCPUReg[rs_base_fmt] - (unsigned)MainCPUReg[rt_ft];
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
