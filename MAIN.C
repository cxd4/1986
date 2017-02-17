//1964: The Nintendo64 Emulator by Deku Nut
//Started 3/10/1999
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
void RunOpcode();
//void DoMainCPUOpcode();
void DoCOPx(int COPn);
void DoRegimm();
void DoSpecial() ;
void JumpToNextInstruction();
void CleanUp();
void ByteSwap(uint32 beginByte, uint32 endByte);

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
		printf( "File could not be opened\n" );
		fclose(stream);
		exit(0);
	}
	fclose(stream);
}

void RunOpcode()
{
	Instruction = *InstructionPointer++;
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

		case CACHE : cache(); break;

		default  : 
#ifdef _DEBUG
			printf("%X: Opcode = %d (working on it)\n", pc, Opcode);
#endif
			JumpToNextInstruction(); break;
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

		case ADDI    : addi();  break;
		case ADDIU   : addiu(); break;
		case ANDI    : andi();  break;
		case DADDI   : daddi(); break;
		case ORI     : ori();   break;
		case SLTI    : slti();  break;
		case SLTIU   : sltiu(); break;
		case XORI    : xori();  break;
		
		case LUI     : lui();   break;
		case JAL     : jal();   break;
		default      :
#ifdef _DEBUG			
			printf("%X: Opcode = %d (working on it)\n", pc, Opcode);
#endif
	        JumpToNextInstruction(); break;
		}
	}
}

//Jump to the next instruction for unimplemented opcodes.
void JumpToNextInstruction() {
	InstructionPointer++;
	InstructionPointer++;
	InstructionPointer++;
	InstructionPointer++;	
}

void DoCOPx(int COPn) {
	Parse6_5_5_5_5_6();
	switch(COPn) {
	case 0 : switch(rs_base_fmt) {
			case CFC0  : cfc0();  break;
			case CTC0  : ctc0();  break;
			case DMFC0 : dmfc0(); break;
			case DMTC0 : dmtc0(); break;
			case MFC0  : mfc0();  break;
			case MTC0  : mtc0();  break;
			default    : ;
#ifdef _DEBUG				
				printf("!!COPO!!\n");
#endif
			} break;
	case 1 : switch(rs_base_fmt) {
			case CFC1  : cfc1();  break;
			case CTC1  : ctc1();  break;
			case DMFC1 : dmfc1(); break;
			case DMTC1 : dmtc1(); break;
			case MFC1  : mfc1();  break;
			case MTC1  : mtc1();  break;
			default    : ;
#ifdef _DEBUG				
				printf("!!COP1!!\n");
#endif
			} break;
	case 2 : switch(rs_base_fmt) {
			case CFC2  : cfc2();  break;
			case CTC2  : ctc2();  break;
			case DMFC2 : dmfc2(); break;
			case DMTC2 : dmtc2(); break;
			case MFC2  : mfc2();  break;
			case MTC2  : mtc2();  break;
			
			case LDC2  : ldc2();  break;
			case LWC2  : lwc2();  break;
			case SDC2  : sdc2();  break;
			case SWC2  : swc2();  break;
			default    : ;
#ifdef _DEBUG				
				printf("!!COP2!!\n");
#endif
		} break;
	default : ; //No need to Jump to next instr, 'cause Parse already did
#ifdef _DEBUG		
		printf("!!COP %d does not exist!!\n", COPn);
#endif
	}
}

void DoSpecial() {
	//The special op is the last 6 bits of the 32bit instruction
	SpecialOp = Instruction << 2;
	SpecialOp = SpecialOp >> 2;
	//printf("SpecialOp = %d, %s\n", SpecialOp, DebugSpecial(SpecialOp));
	Parse6_5_5_5_5_6(); //Careful: this is only for these cases right now.
	switch (SpecialOp) {
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
		printf("!!!!!\n");
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
		printf("!!!!!\n");
#endif
	}
}

void main(void)
{
	char* RomPath;
	int k;

	RomPath = "c:\\mariok~1.v64";
  	ReadRomData(RomPath);
	InstructionPointer = &buffer[0];

	//CHEEZY PATCHES
	MainCPUReg[RA] = 0x80000520; //Bootcode probably does this 
	//MainCPUReg[RA] = 0x03ff0000; //but boot code bombs right now.
	//MainCPUReg[RA] = 0x800004C4;
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
	CodeStart = 64;
	InstructionPointer = &buffer[CodeStart];

	//Disassemble Boot Code (uncomment below to do it)
//	for (k=0; k<1008; k++) { //1008 instructions in the boot code 
//						     //after the header
//		RunOpcode();
//		pc+=4;
//#ifdef _DEBUG
//	if (UpdateViewPort) RefreshConsole();
//#endif
//	} //end for

	pc = MainStartAddr = 0x80000400;
	//Point InstructionPointer to beginning of Main Code
	CodeStart = 4096;
	InstructionPointer = &buffer[CodeStart];

#ifdef _DEBUG
	printf("\nMain code disassembly:\n");
#endif
	//Disassemble Main Code
	while (!kbhit()); getch();
	for(;;) {
		RunOpcode();
		pc+=4; //increment program counter
#ifdef _DEBUG
	if (UpdateViewPort)	RefreshConsole();
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