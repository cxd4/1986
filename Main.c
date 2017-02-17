//1964: The Open Source Nintendo64 Emulator 
//started 3/10/1999 by Joel M. (schibo)
//First Version Released as Open Source: 4/9/1999
//main.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chipmake.h"
#include "MainRegs.h"
#include "main.h"

#define CODESIZE 6400000 //This is crap now.
//Its just so we can read more bytes to mem.
//We actually need to get the size of the file at runtime.

//function declarations
void ReadRomData(char* RomPath);
void ByteSwap(uint32 beginByte, uint32 endByte);
//void DoMainCPUOpcode();

void Step_CPU();
void RunOpcode();
void CleanUp();

void DoBC1();
void DoCOPx(int COPn);
void DoFPU();
void DoRegimm();
void DoSpecial();
void DoRSPStore();
void DoRSPLoad();
void DoTLB();
void DoVector();

extern void DebuggerUI();
extern uint32 GetPhysicalAddress(uint32 Address);

void ReadRomData(char* RomPath)
{
    FILE* stream;

	//Allocate rom buffer
	if (( buffer = (uint32)calloc( CODESIZE, sizeof( uint32 ))) == NULL ) {
		printf( "Error: Could not allocate rom buffer.\n" );
		exit( 0 );
	}

	if( (stream = fopen( RomPath, "r" )) != NULL )   {
		/* Attempt to read in 25 characters */
		fread( buffer, sizeof( uint32 ), CODESIZE, stream );
		fclose( stream );
	}
	else {
		printf( "File could not be opened.\n" );
		//fclose(stream); //This line was a nono.
		exit(0);
	}
	fclose(stream);
}

void RunOpcode()
{
	Instruction = *InstructionPointer;
	Opcode = Instruction >> 26;

	if (Opcode > 25) 
		switch(Opcode) {
		case LB  : lb();  break;
		case LBU : lbu(); break;
		case LD  : ld();  break;
		case LDL : ldl(); break;
		case LDR : ldr(); break;
		case LH  : lh();  break;
		case LHU : lhu(); break;
		case LL  : ll();  break;
		case LLD : lld(); break;
		case LW  : lw();  break;
		case LWL : lwl(); break;
		case LWR : lwr(); break;
		case LWU : lwu(); break;
		case SB  : sb();  break;
		case SC  : sc();  break;
		case SCD : scd(); break;
		case SD  : sd();  break;
		case SDL : sdl(); break;
		case SDR : sdr(); break;
		case SH  : sh();  break;
		case SW  : sw();  break;
		case SWL : swl(); break;
		case SWR : swr(); break;

		case LDC1 : ldc1(); break;
		case LWC1 : lwc1(); break;
		case SDC1 : sdc1(); break;
		case SWC1 : swc1(); break;

		case LWC2 : DoRSPLoad(); break;
		case SWC2 : DoRSPStore(); break;

		case CACHE : cache(); break;

		default  : ;
#ifdef _DEBUG
			printf("%X: Opcode = %d (working on it)\n", pc, Opcode);
#endif
		}
	else {
		switch (Opcode) {
		case SPECIAL : DoSpecial();     break;
		case REGIMM  : DoRegimm();      break;
		case COP0    : DoCOPx(0);       break;	
		case COP1    : DoCOPx(1);       break;
		case COP2    : DoCOPx(2);       break;
		
		case BEQ     : beq();  break;  //4
		case BEQL    : beql(); break;  //20
		case BNE     : bne();  break;  //5
		case BNEL    : bnel(); break;  //21

		case ADDI    : addi();   break;
		case ADDIU   : addiu();  break;
		case ANDI    : andi();   break;
		case DADDI   : daddi();  break;
		case DADDIU  : daddiu(); break;
		case ORI     : ori();    break;
		case SLTI    : slti();   break;
		case SLTIU   : sltiu();  break;
		case XORI    : xori();   break;
		
		case LUI     : lui();    break;
		case JAL     : jal();    break;
		default      : ;
#ifdef _DEBUG			
			printf("%X: Opcode = %d (working on it)\n", pc, Opcode);
#endif
		}
	}
}

void DoCOPx(int COPn) {
	Parse6_5_5_5_5_6();
	switch(COPn) {
	case 0 : switch(rs_base_fmt) {
			//case CFC  : cfc0();  break; 
			//case CTC  : ctc0();  break;
			//case DMFC : dmfc0(); break;
			//case DMTC : dmtc0(); break;
			case MFC  : mfc0();  break;
			case MTC  : mtc0();  break;
			case TLB  : DoTLB(); break;
			default    : ;
#ifdef _DEBUG				
				printf("!!COPO!!\n");
#endif
			} break;
	case 1 : switch(rs_base_fmt) {
			case CFC     : cfc1();  break;
			case CTC     : ctc1();  break;
			case DMFC    : dmfc1(); break;
			case DMTC    : dmtc1(); break;
			case MFC     : mfc1();  break;
			case MTC     : mtc1();  break;
			case BC      : DoBC1();  break;
			case S_INSTR : DoFPU(dummy_single);  break;
			case D_INSTR : DoFPU(dummy_double); break;
			case W_INSTR : DoFPU(dummy_word);   break;
			case L_INSTR : DoFPU(dummy_long);   break;

			default    : ;
#ifdef _DEBUG				
				printf("!!COP1!!\n");
#endif
			} break;
	case 2 : if (rs_base_fmt > 15) DoVector();
		    else switch(rs_base_fmt) {
//			case CFC  : cfc2();  break;        //according to anarko's docs,
//			case CTC  : ctc2();  break;        //none of these exist in r4300i
//			case DMFC : dmfc2(); break;        //(i think)
//			case DMTC : dmtc2(); break;
//			case MFC  : mfc2();  break;
//			case MTC  : mtc2();  break;
			
//			case LDC2  : ldc2();       break;
//			case LWC2  : DoRSPLoad();  break;
//			case SDC2  : DoRSPStore(); break;
//			case SWC2  : swc2();       break;
			default    : ;
#ifdef _DEBUG				
				printf("!!COP2!!\n");
#endif
		} break;
	default : ; //No need to Jump to next instr, 'cause Parse already did.
		        //Above comment makes no sense anymore, but i need it. :)
#ifdef _DEBUG		
		printf("!!COP %d does not exist!!\n", COPn);
#endif
	}
}

void DoFPU() {
	Parse6_5_5_5_5_6();
	//The FPU op (function) is the last 6 bits of the 32bit instruction
	if ( rs_base_fmt < 47) switch(rs_base_fmt) {
		case ADD_F   : add_f(); //0
		case SUB_F   : sub_f(); //1
		case MUL     : mul();   //2
		case DIV_F   : div_f(); //3
		case SQRT    : Sqrt();  //4
		case ABS     : Abs();   //5
		case MOV     : mov();   //6
		case NEG     : neg();   //7
		case ROUND_L : round(dummy_long); //8
		case TRUNC_L : trunc(dummy_long); //9
		case CEIL_L  : Ceil(dummy_long);  //10
		case FLOOR_L : Floor(dummy_long); //11
		case ROUND_W : round(dummy_word); //12
		case TRUNC_W : trunc(dummy_word); //13
		case CEIL_W  : Ceil(dummy_word);  //14
		case FLOOR_W : Floor(dummy_word); //15
		case CVT_S   : Cvt(dummy_single); //32
		case CVT_D   : Cvt(dummy_double); //33
		case CVT_W   : Cvt(dummy_word);   //36
		case CVT_L   : Cvt(dummy_long);   //37
		default      : ;
#ifdef _DEBUG
		printf("!!FPU!!\n");
#endif
	}
	else 
		c_cond();         //48-63
	
}

void DoBC1() {
	/*Parse6_5_3_1_1_16(); TODO: Would you create this parser? I madeglobal ND, TF*/
	switch(rs_base_fmt) {
	case BC1F  : bc1f();  //0
	case BC1T  : bc1t();  //1
	case BC1FL : bc1fl(); //2
	case BC1TL : bc1tl(); //3
	default    : ;
#ifdef _DEBUG
		printf("!!BC1!!!\n");
#endif
	}
}

void DoSpecial() {
	//The special op (function) is the last 6 bits of the 32bit instruction
	function = Instruction << 2;
	function = function >> 2;
	//printf("SpecialOp = %d, %s\n", function, DebugSpecial(function));
	Parse6_5_5_5_5_6(); //Careful: this is only for these cases right now.
	switch (function) {
		case JALR  : jalr();  break;
		case ADD   : add();   break;
		case ADDU  : addu();  break;
		case AND   : and();   break;
		case DADD  : dadd();  break;
		case DADDU : daddu(); break;
		case DSLLV : dsllv(); break;
		case DSUB  : dsub();  break;
		case DSUBU : dsubu(); break;
		case NOR   : nor();   break;
		case OR    : or();    break;
		case SLT   : slt();   break;
		case SLLV  : sllv();  break;
		case SLTU  : sltu();  break;
		case SRAV  : srav();  break;
		case SRLV  : srlv();  break;
		case SUB   : sub();   break;
		case SUBU  : subu();  break;
		case XOR   : xor();   break;

		case DSLL   : dsll();   break;
		case DSLL32 : dsll32(); break;
		case DSRA   : dsra();   break;
		case DSRA32 : dsra32(); break;
		case DSRAV  : dsrav();  break;
		case DSRL   : dsrl();   break;
		case DSRL32 : dsrl32(); break;
		case DSRLV  : dsrlv();  break;
		case SLL    : sll();    break;
		case SRA    : sra();    break;
		case SRL    : srl();    break;

		case DDIV   : ddiv();   break;
		case DDIVU  : ddivu();  break;
		case DIV    : Div();    break; //("D" in Div() is not a typo.)
		case DIVU   : divu();   break;
		case DMULT  : dmult();  break;
		case DMULTU : dmultu(); break;
		case MULT   : mult();   break;
		case MULTU  : multu();  break;

		case TEQ  : teq();  break;
		case TGE  : tge();  break;
		case TGEU : tgeu(); break;
		case TLT  : tlt();  break;
		case TLTU : tltu(); break;
		case TNE  : tne();  break;

		case MFHI : mfhi(); break;
		case MFLO : mflo(); break;

		case MTHI : mthi(); break;
		case MTLO : mtlo(); break;
		case JR   : jr();   break;

		default   : ;
#ifdef _DEBUG			
		printf("!!SPECIAL!!\n");
#endif
	}
}

void DoRegimm() {
	Parse6_5_5_16();
	switch(rt_ft) {
	case BGEZ    : bgez();    break;
	case BGEZALL : bgezall(); break;
	case BGEZL   : bgezl();   break;
	case BLTZ    : bltz();    break;
	case BLTZAL  : bltzal();  break;
	case BLTZALL : bltzall(); break;
 	case BLTZL   : bltzl();   break;
	case BGEZAL  : bgezal();  break;
	
	case TEQI    : teqi();    break;
	case TGEI    : tgei();    break;
	case TGEIU   : tgeiu();   break;
	case TLTI    : tlti();    break;
	case TLTIU   : tltiu();   break;
	case TNEI    : tnei();    break;
	default      : ;
#ifdef _DEBUG		
		printf("!!REGIMM!!\n");
#endif
	}
}

void DoRSPLoad() {
	Parse6_5_5_5_5_6(); //TODO: Make Parse 6_5_15_6
	switch(rd_fs) {
	case LBV : lbv(); break;
	case LSV : lsv(); break;
	case LLV : llv(); break;
	case LDV : ldv(); break;
	case LQV : lqv(); break;
	case LRV : lrv(); break;
	case LPV : lpv(); break;
	case LUV : luv(); break;
	case LHV : lhv(); break;
	case LFV : lfv(); break;
	case LWV : lwv(); break;
	case LTV : ltv(); break;
	default  : ;
#ifdef _DEBUG
		printf("!!RSP LOAD!!\n");
#endif
	}
}

void DoRSPStore() {
	Parse6_5_5_5_5_6(); //TODO: Make Parse 6_5_15_6
	switch(rd_fs) {
	case SBV : sbv(); break; //0
	case SSV : ssv(); break; //1
	case SLV : slv(); break; //2
	case SDV : sdv(); break; //3
	case SQV : sqv(); break; //4
	case SRV : srv(); break; //5
	case SPV : spv(); break; //6
	case SUV : suv(); break; //7
	case SHV : shv(); break; //8
	case SFV : sfv(); break; //9
	case SWV : swv(); break; //10
	case STV : stv(); break; //11
	default  : ;
#ifdef _DEBUG
		printf("!!RSP STORE!!\n");
#endif
	}
}

void DoTLB() {
	switch(rs_base_fmt) {
	case TLBR  : tlbr();  break;
	case TLBWI : tlbwi(); break;
	case TLBWR : tlbwr(); break;
	case TLBP  : tlbp();  break;
	case ERET  : eret();  break;
	default  : ;
#ifdef _DEBUG
		printf("!!TLB!!\n");
#endif
	}
}

void DoVector() {
	//The Vector opcode (function) is the last 6 bits of the 32bit instruction
	function = Instruction << 2;
	function = function >> 2;	
	if (function < 33)
		switch(function) {
		case VMULF : vmulf(); //0
		case VMULU : vmulu(); //1
		case VRNDP : vrndp(); //2
		case VMULQ : vmulq(); //3
		case VMUDL : vmudl(); //4
		case VMUDM : vmudm(); //5
		case VMUDN : vmudn(); //6
		case VMUDH : vmudh(); //7
		case VMACF : vmacf(); //8
		case VMACU : vmacu(); //9
		case VRNDN : vrndn(); //10
		case VMACQ : vmacq(); //11
		case VMADL : vmadl(); //12
		case VMADM : vmadm(); //13
		case VMADN : vmadn(); //14
		case VMADH : vmadh(); //15
		case VADD  : vadd();  //16
		case VSUB  : vsub();  //17

		//case VSUT  : vsut();  //18 may not exist

		case VABS  : vabs();  //19
		case VADDC : vaddc(); //20
		case VSUBC : vsubc(); //21

		//case VADDB : vaddb(); //22
		//case VSUBB : vsubb(); //23
		//case VACCB : vaccb(); //24
		//case VSUCB : vsucb(); //25 these 7 may not exist
		//case VSAD  : vsad();  //26
		//case VSAC  : vsac();  //27
		//case VSUM  : vsum();  //28

		case VSAW  : vsaw();  //29 
		case VLT   : vlt();   //32
		default  : ;
#ifdef _DEBUG
		printf("!!VECTOR!!\n");
#endif
	} 
	else
		switch (function) {
		case VEQ   : veq();   //33
		case VNE   : vne();   //34
		case VGE   : vge();   //35
		case VCL   : vcl();   //36
		case VCH   : vch();   //37
		case VCR   : vcr();   //38
		case VMRG  : vmrg();  //39
		case VAND  : vand();  //40
		case VNAND : vnand(); //41
		case VOR   : vor();   //42
		case VNOR  : vnor();  //43
		case VXOR  : vxor();  //44
		case VNXOR : vxnor(); //45
		case VRCP  : vrcp();  //48
		case VRCPL : vrcpl(); //49
		case VRCPH : vrcph(); //50
		case VMOV  : vmov();  //51
		case VRSQ  : vrsq();  //52
		case VRSQL : vrsql(); //53
		case VRSQH : vrsqh(); //54
		default    : ;
#ifdef _DEBUG
		printf("!!VECTOR!!\n");
#endif
	}
}

void Step_CPU()
{
	//TODO : Exceptions + interrupts here
	RunOpcode();

	//Check Delay slot here...
	switch (CPUdelay)
	{
		case 0 :	//No delay
			pc+=4;
			InstructionPointer++;
			break;
		case 1 :	//execute delay instruction
			pc+=4;
			InstructionPointer++;
			CPUdelay = 2;
			break;
		case 2 :	//Do the jump now
			pc = CPUdelayPC;
			if (MainStartAddr == 0xA4000040)
				InstructionPointer = &buffer[pc-0xA4000000];  //boot code
			else
				InstructionPointer = &buffer[pc-MainStartAddr+4096]; //game code
			
			//printf("GetPhysicalAddress = %x\n", InstructionPointer);
			CPUdelay = 0;
			break;
	}

#ifdef _DEBUG
	if (UpdateViewPort) RefreshConsole();
	//printf("\nCPUdelay = %d. pc = %08X\n\n", CPUdelay, pc);
#endif
}

void main(int argc, char** argv[])
{
	uint32 endbootPC;
	char* RomPath;
	int k;

	//This is the default ROM path. You can set it to whatever you
	//need it to be.
	RomPath = "c:\\fire.bin";
	if (argc < 2) {
		printf("No ROM specified.\n");
		printf("Using %s as default for now.\n", RomPath);
		printf("If you do not have %s in c:\\, I will crash now. Thank you :)\n", RomPath);
	}
	else
		RomPath = argv[1];

	printf("Loading... Please wait.\n");
  	ReadRomData(RomPath);
	InstructionPointer = &buffer[0];
	//CHEEZY PATCHES
	MainCPUReg[RA] = 0x80000520; //Bootcode probably does this 
	//MainCPUReg[RA] = 0x03ff0000; //but boot code bombs right now.
	//MainCPUReg[RA] = 0x800004C4;

	//SB 16/4/99
	COP0Reg[CONFIG]	= 0x00066463;
	COP0Reg[STATUS]	= 0x70400004;
	COP0Reg[RANDOM]	= 0x0000002F;
	COP0Reg[PREVID]	= 0x00000b00;

	MainCPUReg[S4]	= 0x00000001;
	MainCPUReg[S6]	= 0x0000003F;
	MainCPUReg[SP]	= 0x00400000;

#ifdef _DEBUG
	UpdateViewPort = 1; //Allow Console Refresh
	DebuggerMain();
#endif	
	//Check for byte swap. Unflipped ROMs SUCK!
	if (buffer[0] == 0x80 & buffer[1] == 0x37) {
		printf("Swapping bytes. Please use fliprom for faster access!\n");
		ByteSwap(0, CODESIZE);
	}
	else if (!(buffer[0] == 0x37 & buffer[1] == 0x80))
		printf("Not a valid ROM.\n");

	ByteSwap(32, 64); //This swaps the rom text info so it can be read properly.
	ParseHeader();

	//This code does the boot code.
	pc = MainStartAddr = 0xA4000040;
	//Point InstructionPointer to beginning of Boot Code
	InstructionPointer = &buffer[64];

//Execute boot code. Uncomment the /* */ below to do it.

#ifdef _DEBUG
		printf("\nBoot code disassembly:\n");
#endif
	//Disassemble Boot Code

endbootPC = MainStartAddr+1008*4; //1008 instructions in the boot code 
	while (pc != endbootPC) { 
#ifdef _DEBUG
	DebuggerUI();
#else	
	Step_CPU();
#endif
	}

	pc = MainStartAddr = 0x80000400;
	//Point InstructionPointer to beginning of Main Code
	InstructionPointer = &buffer[4096];

#ifdef _DEBUG
	printf("\nMain code disassembly:\n");
#endif

	for(;;)	{
#ifdef _DEBUG
	DebuggerUI();
#else	
	Step_CPU();
#endif
	}
	
	CleanUp();
}

void CleanUp() {
	free(buffer);
#ifdef _DEBUG
	freeconsole();
#endif
}

void ByteSwap(uint32 beginByte, uint32 endByte) {
	uint8 temp; //Used for the byte swapping.
	int k;

	for (k=beginByte;k<endByte; k+=4) {
		temp = buffer[k];
		buffer[k] = buffer[k+3];
		buffer[k+3] = temp;
	
		temp = buffer[k+1];
		buffer[k+1] = buffer[k+2];
		buffer[k+2] = temp;
	}
}