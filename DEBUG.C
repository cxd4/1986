//Debug.c for debug output

#ifdef _DEBUG
#include "chipmake.h"
#include "MainRegs.h"
#include <stdio.h>
#include <string.h>

//Function Declarations
char* DebugMainCPU();
char* DebugCOP0( uint8 fmt );
char* DebugSpecial( uint8 function );
char* DebugRegimm(uint8 s);

char* DebugMainCPU() {
	char* OpcodeStr;
	
	switch(Opcode) {
	case SPECIAL : return("SPECIAL"); //Perform Special Op
	case REGIMM : return("REGIMM");	  //Perform REGIMM
	case J      : return("J");
	case JAL    : return("JAL");
	case BEQ    : return("BEQ");
	case BNE    : return("BNE");
	case BLEZ   : return("BLEZ");
	case BGTZ   : return("BGTZ");
	case ADDI   : return("ADDI");
	case ADDIU  : return("ADDIU");
	case SLTI   : return("SLTI");
	case SLTIU  : return("SLTIU");
	case ANDI   : return("ANDI");
	case ORI    : return("ORI");
	case XORI   : return("XORI");
	case LUI    : return("LUI");
	case COP0   : return("COP0"); // <-
	case COP1   : return("COP1"); // <- Go to Coprocessor (COPn)
	case COP2   : return("COP2"); // <-
	case BEQL   : return("BEQL");
	case BNEL   : return("BNEL");
	case BLEZL  : return("BLEZL");
	case BGTZL  : return("BGTZL");
	case DADDI  : return("DADDI");
	case DADDIU : return("DADDIU");
	case LDL    : return("LDL");
	case LDR    : return("LDR");
	case LB     : return("LB");
	case LH     : return("LH");
	case LWL    : return("LWL");
	case LW     : return("LW");
	case LBU    : return("LBU");
	case LHU    : return("LHU");
	case LWR    : return("LWR");
	case LWU    : return("LWU");
	case SB     : return("SB");
	case SH     : return("SH");
	case SWL    : return("SWL");
	case SW     : return("SW");
	case SDL    : return("SDL");
	case SDR    : return("SDR");
	case SWR    : return("SWR");
	case CACHE  : return("CACHE");
	case LL     : return("LL");
	case LWC1   : return("LWC1");
	case LWC2   : return("LWC2");
	//51 unused
	case LLD    : return("LLD");
	case LDC1   : return("LDC1");
	case LDC2   : return("LDC2");
	case LD     : return("LD");
	case SC     : return("SC");
	case SWC1   : return("SWC1");
	case SWC2   : return("SWC2");
	//59 unused
	case SCD    : return("SCD");
	case SDC1   : return("SDC1");
	case SDC2   : return("SDC2");
	case SD     : return("SD");
	default     : return( "!!!!!" ); 
	}
	return( OpcodeStr );
}

char* DebugMainCPUReg(uint8 rt) {
	switch(rt) {
	case R0 : return("r0");
	case AT : return("at");
	case V0 : return("v0");
	case V1 : return("v1");
	case A0 : return("a0");
	case A1 : return("a1");
	case A2 : return("a2");
	case A3 : return("a3");
	case T0 : return("t0");
	case T1 : return("t1");
	case T2 : return("t2");
	case T3 : return("t3");
	case T4 : return("t4");
	case T5 : return("t5");
	case T6 : return("t6");
	case T7 : return("t7");
	case S0 : return("s0");
	case S1 : return("s1");
	case S2 : return("s2");
	case S3 : return("s3");
	case S4 : return("s4");
	case S5 : return("s5");
	case S6 : return("s6");
	case S7 : return("s7");
	case T8 : return("t8");
	case T9 : return("t9");
	case K0 : return("k0");
	case K1 : return("k1");
	case GP : return("gp");
	case SP : return("sp");
	case S8 : return("s8");
	case RA : return("ra");
	default : return("!!!!!");
	}
}

//Debug CoProcessor0 (COP0) Opcodes
char* DebugCOP0( uint8 fmt ) {
	switch( fmt ) {
	case MFC0  : return( "MFC0" );
	case DMFC0 : return( "DMFC0" );
	case CFC0  : return( "CFC0" );
	case MTC0  : return( "MTC0" );
	case DMTC0 : return( "DMTC0" );
	case CTC0  : return( "CTC0" );
	case BC0   : return( "BC0" );
	case TLB   : return( "TLB" );
	default    : return( "!fmt!" );
	}
}

char* DebugCOP0Reg(uint8 reg) {
	switch( reg ) {
	case INDEX     : return("Index");
	case RANDOM    : return("Random");
	case ENTRYLO0  : return("EntryLo0");
	case ENTRYLO1  : return("EntryLo1");
	case CONTEXT   : return("Context");
	case PAGEMASK  : return("PageMask");
	case WIRED     : return("Wired");
	case RESERVED0 : return("Reserved");
	case BADVADDR  : return("BadVaddr");
	case COUNT     : return("Count");
	case ENTRYHI   : return("EntryHi");
	case COMPARE   : return("Compare");
	case STATUS    : return("Status");
	case CAUSE     : return("Cause");
	case EPC       : return("EPC");
	case PREVID    : return("PRevID");
	case CONFIG    : return("Config");
	case LLADDR    : return("LLAddr");
	case WATCHLO   : return("WatchLo");
	case WATCHHI   : return("WatchHi");
	case XCONTEXT  : return("XContext");
	case RESERVED1 : return("Reserved");
	case RESERVED2 : return("Reserved");
	case RESERVED3 : return("Reserved");
	case RESERVED4 : return("Reserved");
	case RESERVED5 : return("Reserved");
	case PERR      : return("PErr");
	case CACHEERR  : return("CacheErr");
	case TAGLO     : return("TagLo");
	case TAGHI     : return("TagHi");
	case ERROREPC  : return("ErrorEPC");
	case RESERVED6 : return("Reserved");
	default        : return("!!!!!");
	}
}

//Debug Special when Main Opcode = 0
char* DebugSpecial(uint8 function) {
	switch( function ) {
//SPECIAL Instructions
	case SLL        : return( "SLL" );
	case SRL        : return( "SRL" );
	case SRA        : return( "SRA" );
	case SLLV       : return( "SLLV" );
	case SRLV       : return( "SLRV" );
	case SRAV       : return( "SRAV" );
	case JR         : return( "JR" );
	case JALR       : return( "JALR" );
	case SYSCALL    : return( "SYSCALL" );
	case BREAK      : return( "BREAK" );
	case SYNC       : return( "SYNC" );
	case MFHI       : return( "MFHI" );
	case MTHI       : return( "MTHI" );
	case MFLO       : return( "MFLO" );
	case MTLO       : return( "MTLO" );
	case DSLLV      : return( "DSLLV" );
	case DSRLV      : return( "DSRLV" );
	case DSRAV      : return( "DSRAV" );
	case MULT       : return( "MULT" );
	case MULTU      : return( "MULTU" );
	case DIV        : return( "DIV" );
	case DIVU       : return( "DIVU" );
	case DMULT      : return( "DMULT" );
	case DMULTU     : return( "DMULTU" );
	case DDIV       : return( "DDIV" );
	case DDIVU      : return( "DDIVU" );
	case ADD        : return( "ADD" );
	case ADDU       : return( "ADDU" );
	case SUB        : return( "SUB" );
	case SUBU       : return( "SUBU" );
	case AND        : return( "AND" );
	case OR         : return( "OR" );
	case XOR        : return( "XOR" );
	case NOR        : return( "NOR" );
	case SLT        : return( "SLT" );
	case SLTU       : return( "SLTU" );
	case DADD       : return( "DADD" );
	case DADDU      : return( "DADDU" );
	case DSUB       : return( "DSUB" );
	case DSUBU      : return( "DSUBU" );
	case TGE        : return( "TGE" );
	case TGEU       : return( "TGEU" );
	case TLT        : return( "TLT" );
	case TLTU       : return( "TLTU" );
	case TEQ        : return( "TEQ" );
	case TNE        : return( "TNE" );
	case DSLL       : return( "DSLL" );
	case DSRL       : return( "DSRL" );
	case DSRA       : return( "DSRA" );
	case DSLL32     : return( "DSLL32" );
	case DSRL32     : return( "DSRL32" );
	case DSRA32     : return( "DSRA32" );
	default         : return( "!!!!!" );
	}
}

//REGIMM Opcodes
char* DebugRegimm(uint8 Instruction) {
	switch( Instruction ) {
	case BLTZ    : return("BLTZ");
	case BGEZ    : return("BGEZ");
	case BLTZL   : return("BLTZL");
	case BGEZL   : return("BGEZL");
	case TGEI    : return("TGEI");
	case TGEIU   : return("TGEIU");
	case TLTI    : return("TLTI");
	case TLTIU   : return("TLTIU");
	case TEQI    : return("TEQI");
	case TNEI    : return("TNEI");
	case BLTZAL  : return("BLTZAL");
	case BGEZAL  : return("BGEZAL");
	case BLTZALL : return("BLTZALL");
	case BGEZALL : return("BGEZALL");
	default      : return("!!!!!");
	}
}

#endif //end #ifdef _DEBUG