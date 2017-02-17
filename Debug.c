//Debug.c for debug output

#ifdef _DEBUG
#include "chipmake.h"
#include "MainRegs.h"
#include <stdio.h>
#include <string.h>

//Function Declarations
char* DebugBC1(uint8 function);
char* DebugCOP0( uint8 fmt );
char* DebugFPU(uint8 function);
char* DebugMainCPU();
char* DebugRegimm(uint8 s);
char* DebugRSPLoad();
char* DebugRSPStore();
char* DebugSpecial( uint8 function );
char* DebugTLB();
char* DebugVector();

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
	case MFC  : return( "MFC0" );
	case DMFC : return( "DMFC0" );
	case CFC  : return( "CFC0" );
	case MTC  : return( "MTC0" );
	case DMTC : return( "DMTC0" );
	case CTC  : return( "CTC0" );
	case BC   : return( "BC0" );
	case TLB  : return( "TLB" );
	default   : return( "!fmt!" );
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

//BC1
char* DebugBC1(uint8 function) {
	switch( function ) {
	case BC1F     : return("BC1F");
	case BC1T     : return("BC1T");
	case BC1FL    : return("BC1FL");
	case BC1TL    : return("BC1TL");
	default       : return("!BC!");
	}
}


//COP1 Functions
char* DebugFPU(uint8 function) {
	switch( function ) {
	case ADD_F   : return("ADD_F");
	case SUB_F   : return("SUB_F");
	case MUL     : return("MUL");
	case DIV_F   : return("DIV_F");
	case SQRT    : return("SQRT");
	case ABS     : return("ABS");
	case MOV     : return("MOV");
	case NEG     : return("NEG");
	case ROUND_L : return("ROUND.L");
	case TRUNC_L : return("TRUNC.L");
	case CEIL_L  : return("CEIL.L");
	case FLOOR_L : return("FLOOR.L");
	case ROUND_W : return("ROUND.W");
	case TRUNC_W : return("TRUNC.W");
	case CEIL_W  : return("CEIL.W");
	case FLOOR_W : return("FLOOR.W");
	case CVT_S   : return("CVT.S");
	case CVT_D   : return("CVT.D");
	case CVT_W   : return("CVT.W");
	case CVT_L   : return("CVT.L");
	case C_F     : return("C.F");
	case C_UN    : return("C.UN");
	case C_EQ    : return("C.EQ");
	case C_UEQ   : return("C.UEQ");
	case C_OLD   : return("C.OLD");
	case C_ULT   : return("C.ULT");
	case C_OLE   : return("C.OLE");
	case C_ULE   : return("C.ULE");
	case C_SF    : return("C.SF");
	case C_NGLE  : return("C.NGLE");
	case C_SEQ   : return("C.SEQ");
	case C_NGL   : return("C.NGL");
	case C_LT    : return("C.LT");
	case C_NGE   : return("C.NGE");
	case C_LE    : return("C.LE");
	case C_NGT   : return("C.NGT");
	default      : return("!FPU!");
	}
}

//RSP Load
char* DebugRSPLoad() {
	switch(rd_fs) {
	case LBV : return("LBV");
	case LSV : return("LSV");
	case LLV : return("LLV");
	case LDV : return("LDV");
	case LQV : return("LQV");
	case LRV : return("LRV");
	case LPV : return("LPV");
	case LUV : return("LUV");
	case LHV : return("LHV");
	case LFV : return("LFV");
	case LWV : return("LWV");
	case LTV : return("LTV");
	default  : return("!RSPLOAD!");
	}
}

//RSP Store
char* DebugRSPStore() {
	switch(rd_fs) {
	case SBV : return("SBV");
	case SSV : return("SSV");
	case SLV : return("SLV");
	case SDV : return("SDV");
	case SQV : return("SQV");
	case SRV : return("SRV");
	case SPV : return("SPV");
	case SUV : return("SUV");
	case SHV : return("SHV");
	case SFV : return("SFV");
	case SWV : return("SWV");
	case STV : return("STV");
	default  : return("!RSPSTORE!");
	}
}

char* DebugVector(uint8 function) {
	switch(function) {
	case VMULF : return("VMULF");
	case VMULU : return("VMULU");
	case VRNDP : return("VRNDP");
	case VMULQ : return("VMULQ");
	case VMUDL : return("VMUDL");
	case VMUDM : return("VMUDM");
	case VMUDN : return("VMUDN");
	case VMUDH : return("VMUDH");
	case VMACF : return("VMACF");
	case VMACU : return("VMACU");
	case VRNDN : return("VMRDN");
	case VMACQ : return("VMACQ");
	case VMADL : return("VMADL");
	case VMADM : return("VMADM");
	case VMADN : return("VMADN");
	case VMADH : return("VMADH");
	case VADD  : return("VADD");
	case VSUB  : return("VSUB");

	case VSUT  : return("VSUT");//may not exist

	case VABS  : return("VABS");
	case VADDC : return("VADDC");
	case VSUBC : return("VSUBC");

	case VADDB : return("VADDB");//
	case VSUBB : return("VSUBB");//
	case VACCB : return("VACCB");//
	case VSUCB : return("VSUCB");//these 7 may not exist
	case VSAD  : return("VSAD");//
	case VSAC  : return("VSAC");//
	case VSUM  : return("VSUM");//

	case VSAW  : return("VSAW");
	case VLT   : return("VLT");
	case VEQ   : return("VEQ");
	case VNE   : return("VNE");
	case VGE   : return("VGE");
	case VCL   : return("VCL");
	case VCH   : return("VCH");
	case VCR   : return("VCR");
	case VMRG  : return("VMRG");
	case VAND  : return("VAND");
	case VNAND : return("VNAND");
	case VOR   : return("VOR");
	case VNOR  : return("VNOR");
	case VXOR  : return("VXOR");
	case VNXOR : return("VXNOR");
	case VRCP  : return("VRCP");
	case VRCPL : return("VRCPL");
	case VRCPH : return("VRCPH");
	case VMOV  : return("VMOV");
	case VRSQ  : return("VRSQ");
	case VRSQL : return("VRSQL");
	case VRSQH : return("VRSQH");
	default    : return("!VECTOR!");
	}
}

char* DebugTLB() {
	switch(rs_base_fmt) {
	case TLBR  : return("TLBR");
	case TLBWI : return("TLBWI");
	case TLBWR : return("TLBWR");
	case TLBP  : return("TLBP");
	case ERET  : return("ERET");
	default    : return("!TLB!");
	}
}

#endif //end #ifdef _DEBUG