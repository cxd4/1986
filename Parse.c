//Parse.c

#include <stdio.h>
#include "chipmake.h"
#include "mainregs.h" //we're only using this once. lose it.

//function declarations
//The Parse() names denote the bit length of each field.
//ex: Parse6_20_6(): field0=6, field1=20, field2=6.
void Parse6_26(); //for J and JAL opcodes.
void Parse6_5_5_16();
void Parse6_5_5_10_6();
void Parse6_5_5_5_5_6();
//void ParseCOPxInstruction();
void ParseHeader();

#ifdef _DEBUG
extern char* DebugMainCPU();
extern char* DebugMainCPUReg(uint8 rt);
extern char* DebugSpecial(uint8 function);
extern char* DebugRegimm(uint8 s);
extern char* DebugCOP0(uint8 fmt);
extern char* DebugCOP0Reg(uint8 reg);
#endif _DEBUG

void ParseHeader()
{ 
	uint32 BootMem;
	uint16 validation;
	uint8 IsCompressed;
	uint8 Unknown;
	char cartridgeID[3];
	
	uint32 clockRate;
	uint32 release;
	uint32 CRC1, CRC2;
	//uint32 programCounter;
	//char* imageName;
//	char imageName[10];
	char imageName[21];
	
	//Pointers are slow cause they have to go to memory. 
	//Use pointers as infrequently as possible.
	BootMem = *InstructionPointer++; 

	validation = BootMem >> 16;
	IsCompressed = BootMem >> 8;
	Unknown = BootMem;
	clockRate = *InstructionPointer++;
	MainStartAddr = *InstructionPointer++;
	release = *InstructionPointer++;
	CRC1 = *InstructionPointer++;
	CRC2 = *InstructionPointer++;
	*InstructionPointer++;
	*InstructionPointer++;
	
	strcpy(imageName, &buffer[32]);
	imageName[20] = '\0';
	strcpy(cartridgeID, &buffer[60]);
	cartridgeID[2] = '\0';

#ifdef _DEBUG
	printf("[] = expected\n");
	printf("----------------------------------------------------\n");
	printf("0000h - 0001h   Validation word [8037h]= %xh\n", validation);
	printf("0002h           Is compressed? [12h or 13h] = %xh\n", IsCompressed);
	printf("0003h           Unknown [40h] = %Xh\n", Unknown);
	printf("0004h - 0007h   ClockRate = %08Xh\n", clockRate);
	printf("0008h - 000Bh   ProgramCounter = %08Xh\n", MainStartAddr);
	printf("000Ch - 000Fh   Release = %Xh\n", release);
	printf("0010h - 0013h   CRC1 = %08Xh\n", CRC1);
	printf("0014h - 0017h   CRC2 = %08Xh\n", CRC2);
	printf("0020h - 0033h   Image name = %s\n", imageName);
	printf("003Bh           Manufacturer [N = Nintendo] = %c\n", buffer[59]);
	printf("003Ch - 003Dh   Cartridge ID = %s\n", cartridgeID);
	printf("003Eh           Country Code = %c\n", buffer[62]);
	printf("----------------------------------------------------\n");
#endif
}

void Parse6_26() {
//Now THIS is interesting! :)
//The first 4 bits of target are the first4bits of pc.
//The next 26 bits are instr_index.
//The last 2 bits are 00.
	
//For conservation, we'll reuse the instr_index variable to 
//represent target.
uint32 first4bits;

	instr_index  = Instruction << 6;
	instr_index = instr_index >> 4;
	
	first4bits = pc >> 28;
	first4bits = first4bits << 28;
	instr_index += first4bits;
//	printf("instr_index = %X\n", instr_index);
	//while(!kbhit());
}

//REGIMM: Instr. encoded by function field when opcode field = REGIMM
//example:
//-----------------------------------------------------------------
//| BGEZ      | Branch on Greater than or Equal to Zero           |
//|-----------|---------------------------------------------------|
//|  opcode   |  field1 | field2  |       offset/immediate        |
//------6----------5---------5-------------------16----------------
void Parse6_5_5_16() {
	rs_base_fmt = Instruction >> 21;
	rs_base_fmt = rs_base_fmt << 3;
	rs_base_fmt = rs_base_fmt >> 3;
	rt_ft = Instruction >> 16;
	rt_ft = rt_ft << 3;
	rt_ft = rt_ft >> 3;
	offset_immediate = Instruction;
}

//Parse6_5_5_5_5_6 example:
//-----------------------------------------------------------------
//| ABS.fmt   | floating-point ABSolute value                     |
//|-----------|---------------------------------------------------|
//|  010001   |   fmt   |{rt}00000|   fs    |   fd    | 000101 (5)|
//------6----------5---------5---------5---------5----------6------
//Assume we already know what the first & last fields are.
//Note: I'm cheezily using this as Parse6_5_5_11 also for now.
//Hey, it works. :)
void Parse6_5_5_5_5_6() {
	rs_base_fmt = Instruction >> 21;
	rs_base_fmt = rs_base_fmt << 3;
	rs_base_fmt = rs_base_fmt >> 3;

	rt_ft = Instruction >> 16;
	rt_ft = rt_ft << 3;
	rt_ft = rt_ft >> 3;
	
	rd_fs = Instruction >> 11;
	rd_fs = rd_fs << 3;
	rd_fs = rd_fs >> 3;

	sa_fd = Instruction >> 6;
	sa_fd = sa_fd << 3;
	sa_fd = sa_fd >> 3;
}
