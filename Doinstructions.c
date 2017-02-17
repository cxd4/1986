//DoInstructions.c

//TODO: optimize:
//Make these functions static inline void.
//Get the Parse routine out of the functions and put it in the 
//RunOpcode loop. Otherwise, inline makes code huuuge!

#include <stdio.h>
#include "chipmake.h"
#include "mainregs.h"

#define DOUBLEMASKMOST 0x00000000ffffffff
#define DOUBLEMASKLEAST 0xffffffff00000000
#define WORDMASKMOST 0xffffffff0000ffff
#define WORDMASKLEAST 0xffffffffffff0000

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

//_int8 targetByte ; // Byte that will be read from memory (signed)

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

//targetByte =
//buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
//MainCPUReg[rt_ft] = targetByte;
LoadMemory((_int8)MainCPUReg[rt_ft]);
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

//uint8 targetByte;  // Byte that will be read from memory

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

//targetByte =
//buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
//MainCPUReg[rt_ft] = targetByte;
LoadMemory((uint8)MainCPUReg[rt_ft]);
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

//_int64 targetDouble; //Double Word to be loaded from memory

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft),
offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif

//targetDouble =
//buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
//MainCPUReg[rt_ft] = targetDouble;
LoadMemory((_int64)MainCPUReg[rt_ft]);
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
//MainCPUReg[rt_ft] = MainCPUReg[rt_ft] & DOUBLEMASKLEFT;
//Load and Mask the most significant bits of the doubleWord
//targetDouble =
//buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
//targetDouble = targetDouble & DOUBLEMASKRIGHT;
//Combine the resulting bits
//MainCPUReg[rt_ft] = MainCPUReg[rt_ft] | targetDouble;
LoadMemory((_int64)targetDouble);
MainCPUReg[rt_ft] = (MainCPUReg[rt_ft] & DOUBLEMASKLEAST)|
                    ((targetDouble & DOUBLEMASKLEAST) >> 32);
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
//MainCPUReg[rt_ft] = MainCPUReg[rt_ft] & DOUBLEMASKRIGHT;
//Load and Mask the least significant bits of the doubleWord
//targetDouble =
//buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
//targetDouble = targetDouble & DOUBLEMASKLEFT;
//Combine the resulting bits
//MainCPUReg[rt_ft] = MainCPUReg[rt_ft] | targetDouble;

LoadMemory((_int64)targetDouble);
MainCPUReg[rt_ft] = (MainCPUReg[rt_ft] & DOUBLEMASKMOST)|
                   ((targetDouble & DOUBLEMASKMOST) << 32);
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

//_int16 targetHalf; //Half Word to load to rt

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

//targetHalf =
//buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
//MainCPUReg[rt_ft] = targetHalf;
LoadMemory((_int16)MainCPUReg[rt_ft]);
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

//uint16 targetHalf; //unsigned Halfword to load to rt

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

//targetHalf =
//buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
//MainCPUReg[rt_ft] = targetHalf;
LoadMemory((uint16)MainCPUReg[rt_ft]);
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

//_int32 targetWord; //Word to load to rt

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

//targetWord =
//buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
//MainCPUReg[rt_ft] = targetWord;
LoadMemory((_int32)MainCPUReg[rt_ft]);
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

//_int64 targetDouble; //Double to be read from

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

//targetDouble =
//buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
//MainCPUReg[rt_ft] = targetDouble;
LoadMemory((_int64)MainCPUReg[rt_ft]);
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
LoadMemory((_int32)MainCPUReg[rt_ft]);
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
//MainCPUReg[rt_ft] = MainCPUReg[rt_ft] & WORDMASKLEFT;
//Load and Mask the most significant bits of the Word
//targetWord =
//buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
//targetWord = targetWord & WORDMASKRIGHT;
//Combine the resulting bits
//MainCPUReg[rt_ft] = MainCPUReg[rt_ft] | targetWord;

LoadMemory((_int32)targetWord);
 MainCPUReg[rt_ft] = (MainCPUReg[rt_ft] & WORDMASKLEAST)|
                    ((targetWord & WORDMASKLEAST) >> 16);
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
//MainCPUReg[rt_ft] = MainCPUReg[rt_ft] & WORDMASKRIGHT;
//Load and Mask the least significant bits of the Word
//targetWord =
//buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
//targetWord = targetWord & WORDMASKLEFT;
//Combine the resulting bits
//MainCPUReg[rt_ft] = MainCPUReg[rt_ft] | targetWord;

LoadMemory((_int32)targetWord);
MainCPUReg[rt_ft] = (MainCPUReg[rt_ft] & WORDMASKMOST)|
                     ((targetWord & WORDMASKMOST) << 16);

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

//uint32 targetWord; //Word to be loaded to rt

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

//targetWord =
//buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
//MainCPUReg[rt_ft] = targetWord;
LoadMemory((uint32)MainCPUReg[rt_ft]);
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

//_int8* ptrByte; //Pointer to byte to store

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif
//ptrByte =
//&buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
//*ptrByte = MainCPUReg[rt_ft];
StoreMemory((_int8)MainCPUReg[rt_ft]);
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

//_int32* ptrWord;

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

//ptrWord =
//&buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
//*ptrWord = MainCPUReg[rt_ft];
StoreMemory((_int32)MainCPUReg[rt_ft]);
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

//_int32* ptrdoubleWord;

Parse6_5_5_16();
#ifdef _DEBUG
 printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(),
DebugMainCPUReg(rt_ft), offset_immediate,
DebugMainCPUReg(rs_base_fmt));

#endif

//ptrdoubleWord =
//&buffer[MainCPUReg[rs_base_fmt]+offset_immediate-MainStartAddr];
//*ptrdoubleWord = MainCPUReg[rt_ft];
StoreMemory((_int64)MainCPUReg[rt_ft]);
}

void sd() {
//_int64* ptrDouble;
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
StoreMemory((_int64)MainCPUReg[rt_ft]);
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
//_int16* ptrHword;
Parse6_5_5_16();
#ifdef _DEBUG
	printf("%X: %s\t%2s,%04Xh(%s)\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), offset_immediate, DebugMainCPUReg(rs_base_fmt));
#endif
StoreMemory(MainCPUReg[rt_ft]);
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
	StoreMemory((_int32)MainCPUReg[rt_ft]);
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

//-----------------------------------------------------------------
//| DADDI     | Doubleword ADD Immediate                          |
//|-----------|---------------------------------------------------|
//|011000 (24)|   rs    |   rt    |          immediate            |
//------6----------5---------5-------------------16----------------
//Format:  DADDI rt, rs, immediate
// Purpose: To add a constant to a 64-bit integer.
//         If overflow occurs, then trap.
// Descrip: rt = rs + immediate

//Themedes 18/4/99
void daddi() {
Parse6_5_5_16();
	MainCPUReg[rd_fs] = (uint64)MainCPUReg[rs_base_fmt] + offset_immediate;
#ifdef _DEBUG
	printf("%X: %s\t%2s,%s,%04Xh\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

//-----------------------------------------------------------------
//| DADDIU    | Doubleword ADD Immediate Unsigned                 |
//|-----------|---------------------------------------------------|
//|011001 (25)|   rs    |   rt    |          immediate            |
//------6----------5---------5-------------------16----------------
// Format:  DADDIU rt, rs, immediate
// Purpose: To add a constant to a 64-bit integer.
// Descrip: rt = rs + immediate

//Themedes 18/4/99
void daddiu() {
Parse6_5_5_16();
	MainCPUReg[rt_ft] = MainCPUReg[rs_base_fmt] + (uint64)offset_immediate;
#ifdef _DEBUG
	printf("%X: %s\t%2s,%s,%04Xh\n", pc, DebugMainCPU(), DebugMainCPUReg(rt_ft), DebugMainCPUReg(rs_base_fmt), (unsigned)offset_immediate);
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

//-----------------------------------------------------------------
//| BGEZ      | Branch on Greater than or Equal to Zero           |
//|-----------|---------------------------------------------------|
//|  000001   |   rs    |00001 (1)|            offset             |
//------6----------5---------5-------------------16----------------
// Format:  BGEZ rs, offset
// Purpose: To test a GPR then do a PC-relative conditional branch.
// Descrip: branch if rs >= 0 (signed)
void bgez() {
	Parse6_5_5_16();
	if (MainCPUReg[rs_base_fmt] >= 0)
	{

		CPUdelayPC = pc + 4 + offset_immediate*4;
		CPUdelay = 1;
	}
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft),
DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

//-----------------------------------------------------------------
//| BGEZALL   | Branch on Greater than or Equal to Zero And Link Likley
//|-----------|---------------------------------------------------|
//|  000001   |   rs    |10011(19)|            offset             |
//------6----------5---------5-------------------16----------------
// Format:  BGEZALL rs, offset
// Purpose: To test a GPR then do a PC-relative conditional procedure
call;
//          execute the delay slot only if the branch is taken.
// Descrip: branch if rs >= 0 (signed) (return address in ra)
void bgezall() {
	Parse6_5_5_16();
	if (MainCPUReg[rs_base_fmt] >= 0)
	{
		MainCPUReg[RA] = pc + 8;
		CPUdelayPC = pc + 4 + offset_immediate*4;
		CPUdelay = 1;
	}
	else
	{
		pc += 4; //skip delay instruction
	}
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft),
DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

//-----------------------------------------------------------------
//| BGEZL     | Branch on Greater than or Equal to Zero Likley    |
//|-----------|---------------------------------------------------|
//|  000001   |   rs    |00011 (3)|            offset             |
//------6----------5---------5-------------------16----------------
// Format:  BGEZL rs, offset
// Purpose: To test a GPR then do a PC-relative conditional branch;
//          execute the delay slot only if the branch is taken.
// Descrip: branch if rs >= 0 (signed)
void bgezl() {
	Parse6_5_5_16();
	if (MainCPUReg[rs_base_fmt] >= 0)
	{
		MainCPUReg[RA] = pc + 8;
		CPUdelayPC = pc + 4 + offset_immediate*4;
		CPUdelay = 1;
	}
	else
	{
		pc += 4; //skip delay instruction
	}
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft),
DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

//-----------------------------------------------------------------
//| BLTZ      | Branch on Less Than Zero                          |
//|-----------|---------------------------------------------------|
//|  000001   |   rs    |  00000  |            offset             |
//------6----------5---------5-------------------16----------------
// Format:  BLTZ rs, offset
// Purpose: To test a GPR then do a PC-relative conditional branch.
// Descrip: branch if rs < 0 (signed)
void bltz() {
	Parse6_5_5_16();
	if (MainCPUReg[rs_base_fmt] < 0)
	{

		CPUdelayPC = pc + 4 + offset_immediate*4;
		CPUdelay = 1;
	}
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft),
DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

//-----------------------------------------------------------------
//| BLTZAL    | Branch on Less Than Zero And Link                 |
//|-----------|---------------------------------------------------|
//|  000001   |   rs    |  10000  |            offset             |
//------6----------5---------5-------------------16----------------
// Format:  BLTZAL rs, offset
// Purpose: To test a GPR then do a PC-relative conditional procedure call.
// Descrip: branch if rs < 0 (signed) (return address in ra)
void bltzal() {
	Parse6_5_5_16();
	if (MainCPUReg[rs_base_fmt] < 0)
	{
		MainCPUReg[RA] = pc + 8;
		CPUdelayPC = pc + 4 + offset_immediate*4;
		CPUdelay = 1;
	}
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft),
DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

//-----------------------------------------------------------------
//| BLTZALL   | Branch on Less Than Zero And Link Likley          |
//|-----------|---------------------------------------------------|
//|  000001   |   rs    |  10010  |            offset             |
//------6----------5---------5-------------------16----------------
// Format:  BLTZALL rs, offset
// Purpose: To test a GPR then do a PC-relative conditional procedure
//          call; execute the delay slot only if the branch is taken.
// Descrip: branch if rs < 0 (signed) (return address in ra)
void bltzall() {
	Parse6_5_5_16();
	if (MainCPUReg[rs_base_fmt] < 0)
	{
		MainCPUReg[RA] = pc + 8;
		CPUdelayPC = pc + 4 + offset_immediate*4;
		CPUdelay = 1;
	}
	else
	{
		pc += 4; //skip delay instruction
	}
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft),
DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

//-----------------------------------------------------------------
//| BLTZL     | Branch on Less Than Zero Likley                   |
//|-----------|---------------------------------------------------|
//|  000001   |   rs    |00010 (2)|            offset             |
//------6----------5---------5-------------------16----------------
// Format:  BLTZL rs, offset
// Purpose: To test a GPR then do a PC-relative conditional branch;
//            execute the delay slot only if the branch is taken.
// Descrip: branch if rs < 0 (signed)
void bltzl() {
	Parse6_5_5_16();
	if (MainCPUReg[rs_base_fmt] < 0)
	{
		CPUdelayPC = pc + 4 + offset_immediate*4;
		CPUdelay = 1;
	}
	else
	{
		pc += 4; //skip delay instruction
	}
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft),
DebugMainCPUReg(rs_base_fmt), offset_immediate);
#endif
}

//-----------------------------------------------------------------
//| BGEZAL    | Branch on Greater than or Equal to Zero And Link  |
//|-----------|---------------------------------------------------|
//|  000001   |   rs    |10001(17)|            offset             |
//------6----------5---------5-------------------16----------------
// Format:  BGEZAL rs, offset
// Purpose: To test a GPR then do a PC-relative conditional procedure call.
// Comment: BGEZAL r0, offset is equal to a BAL offset
// Descrip: branch if rs >= 0 (signed) (return address in ra)
void bgezal() {
	Parse6_5_5_16();
	if (MainCPUReg[rs_base_fmt] >= 0)
	{
		MainCPUReg[RA] = pc + 8;
		CPUdelayPC = pc + 4 + offset_immediate*4;
		CPUdelay = 1;
	}
#ifdef _DEBUG
	printf("%X: %s\t%s,%04Xh\n", pc, DebugRegimm(rt_ft),
DebugMainCPUReg(rs_base_fmt), offset_immediate);
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

//steb Apr 18, 1999
//-----------------------------------------------------------------
//| JALR      | Jump And Link Register                            |
//|-----------|---------------------------------------------------|
//|  000000   |   rs    |  00000  |   rd    |  00000  |001001 (9) |
//------6----------5---------5---------5---------5----------6------
// Format:  JALR rs, rd
// Purpose: To procedure call to an instruction address in a register.
// Descrip: return address in rd
void jalr() {
	Parse6_5_5_5_5_6();
	CPUdelayPC = rs_base_fmt;
	CPUdelay = 1;
	MainCPUReg[rd_fs] = pc + 8;
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(function),
DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rd_fs));
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
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
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
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function),
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
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function),
DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt),
DebugMainCPUReg(rt_ft));
#endif
}

//-----------------------------------------------------------------
//| DADD      | Doubleword ADD                                    |
//|-----------|---------------------------------------------------|
//|  000000   |   rs    |   rt    |    rd   |  00000  |101100 (44)|
//------6----------5---------5---------5---------5----------6------
// Format:  DADD rd, rs, rt
// Purpose: To add 64-bit integers. If overflow occurs, then trap.
// Descrip: rd = rs + rt

//Themedes 17/4/99
void dadd() {
Parse6_5_5_5_5_6();
	MainCPUReg[rd_fs] = (_int64)MainCPUReg[rs_base_fmt] + (_int64)MainCPUReg[rt_ft];
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void daddu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsllv() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsub() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsubu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
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
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
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
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function),
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
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function),
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
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function),
DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt),
DebugMainCPUReg(rt_ft));
#endif
}

void sllv() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void srav() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void srlv() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

//Themedes 
void sub() {
Parse6_5_5_5_5_6();
	MainCPUReg[rd_fs] = MainCPUReg[rs_base_fmt] - MainCPUReg[rt_ft];
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

//Themedes 
void subu() {
Parse6_5_5_5_5_6();
	MainCPUReg[rd_fs] = MainCPUReg[rs_base_fmt] - (unsigned)MainCPUReg[rt_ft];
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
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
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function),
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
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsll32() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsra() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsra32() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsrav() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsrl() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsrl32() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dsrlv() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

//-----------------------------------------------------------------
//| SLL       | Shift word Left Logical                           |
//|-----------|---------------------------------------------------|
//|  000000   |  00000  |   rt    |    rd   |   sa    |000000 (0) |
//------6----------5---------5---------5---------5----------6------
// Format:  SLL rd, rt, sa
// Purpose: To left shift a word by a fixed number of bits.
// Comment: SLL r0, r0, 0 is equal to a NOP (No OPeration)
// Descrip: rd = rt << sa


//The shift is a logical shift if the left operand is 
//an unsigned quantity;otherwise, it is an arithmetic shift.

//Themedes 19/4/99
void sll() {
Parse6_5_5_5_5_6();
	MainCPUReg[rd_fs] = (unsigned)MainCPUReg[rt_ft] << MainCPUReg[sa_fd];
#ifdef _DEBUG
	if (rd_fs + rs_base_fmt + rt_ft == 0)
		printf("%X: NOP\n", pc);
	else
		printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

//-----------------------------------------------------------------
//| SRA       | Shift word Right Arithmetic                       |
//|-----------|---------------------------------------------------|
//|  000000   |  00000  |   rt    |    rd   |   sa    |000011 (3) |
//------6----------5---------5---------5---------5----------6------
// Format:  SRA rd, rt, sa
// Purpose: To arithmetic right shift a word by a fixed number of bits.
// Descrip: rd = rt >> sa

//Themedes 19/4/99
void sra() {
Parse6_5_5_5_5_6();
	MainCPUReg[rd_fs] = MainCPUReg[rt_ft] >> MainCPUReg[sa_fd];
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

//-----------------------------------------------------------------
//| SRL       | Shift word Right Logical                          |
//|-----------|---------------------------------------------------|
//|  000000   |  00000  |   rt    |    rd   |   sa    |000010 (2) |
//------6----------5---------5---------5---------5----------6------
// Format:  SRL rd, rt, sa
// Purpose: To logical right shift a word by a fixed number of bits.
// Descrip: rd = rt >> sa

//Themedes 19/4/99
void srl() {
Parse6_5_5_5_5_6();
	MainCPUReg[rd_fs] = (unsigned)MainCPUReg[rt_ft] >> MainCPUReg[sa_fd];
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
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
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void ddivu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void Div() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void divu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dmult() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void dmultu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void mult() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void multu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void teq() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void tge() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void tgeu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void tlt() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void tltu() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
#endif
}

void tne() {
#ifdef _DEBUG
	printf("%X: %s\t%s,%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft));
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
	printf("%X: %s\t%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs));
#endif
}

void mflo() {
#ifdef _DEBUG
	printf("%X: %s\t%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rd_fs));
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
	printf("%X: %s\t%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rs_base_fmt));
#endif
}

void mtlo() {
#ifdef _DEBUG
	printf("%X: %s\t%s\n", pc, DebugSpecial(function), DebugMainCPUReg(rs_base_fmt));
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
	Parse6_26();

	CPUdelayPC = MainCPUReg[rs_base_fmt];
	CPUdelay = 1;

#ifdef _DEBUG
	printf("%X: %s\t%s\n", pc, DebugSpecial(function),
DebugMainCPUReg(rs_base_fmt));
#endif
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
	Parse6_5_5_16();
	if (MainCPUReg[rs_base_fmt] == MainCPUReg[rt_ft])
	{
		CPUdelayPC = pc + 4 + offset_immediate*4;
		CPUdelay = 1;
	}
#ifdef _DEBUG
	if (UpdateViewPort)
	printf("%X: %s\t%s,%s,%04Xh\n", pc, DebugMainCPU(),
DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft),
offset_immediate);
#endif
}

//-----------------------------------------------------------------
//| BEQL      | Branch on EQual Likley                            |
//|-----------|---------------------------------------------------|
//|010100 (20)|   rs    |   rt    |            offset             |
//------6----------5---------5-------------------16----------------
// Format:  BEQL rs, rt, offset
// Purpose: To compare GPRs then do a PC-relative conditional branch;
//          execute the delay slot only if the branch is taken.
// Comment: BEQL rs, r0, offset is equal to a BEQZL rs, offset
// Descrip: branch if rs = rt
void beql() {
	Parse6_5_5_16();
	if (MainCPUReg[rs_base_fmt] == MainCPUReg[rt_ft])
	{
		CPUdelayPC = pc + 4 + offset_immediate*4;
		CPUdelay = 1;
	}
	else
	{
		pc += 4;
	}
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%04Xh\n", pc, DebugMainCPU(),
DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft),
offset_immediate);
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
	Parse6_5_5_16();
	if (MainCPUReg[rs_base_fmt] != MainCPUReg[rt_ft])
	{
		CPUdelayPC = pc + 4 + offset_immediate*4;
		CPUdelay = 1;
	}
#ifdef _DEBUG
	FastLoopAddr = pc;
	if (UpdateViewPort)
	printf("%X: %s\t%s,%s,%04Xh\n", pc, DebugMainCPU(),
DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft),
offset_immediate*4 +
pc+4);
#endif
}

//-----------------------------------------------------------------
//| BNEL      | Branch on Not Equal Likley                        |
//|-----------|---------------------------------------------------|
//|010101 (21)|   rs    |   rt    |            offset             |
//------6----------5---------5-------------------16----------------
// Format:  BNEL rs, rt, offset
// Purpose: To compare GPRs then do a PC-relative conditional branch;
//          execute the delay slot only if the branch is taken.
// Comment: BNEL rs, r0, offset is equal to a BNEZL rs, offset
// Descrip: branch if rs <> rt
void bnel() {
	Parse6_5_5_16();
	if (MainCPUReg[rs_base_fmt] != MainCPUReg[rt_ft])
	{
		CPUdelayPC = pc + 4 + offset_immediate*4;
		CPUdelay = 1;
	}
	else
	{
		pc += 4;
	}
#ifdef _DEBUG
	printf("%X: %s\t%s,%s,%04Xh\n", pc, DebugMainCPU(),
DebugMainCPUReg(rs_base_fmt), DebugMainCPUReg(rt_ft),
offset_immediate);
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

void jal() {
	Parse6_26();

	CPUdelayPC = instr_index;
	CPUdelay = 1;
	MainCPUReg[RA] = pc + 8;

#ifdef _DEBUG
	printf("%X: %s\t%08X\n", pc, DebugMainCPU(), instr_index);
#endif
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