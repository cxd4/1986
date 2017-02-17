//Memory.c (schibo)
//Thanks to anarko & zilmar 4 assistance.

#include "chipmake.h"

int StoreMemory(void* regValue);
int LoadMemory(void* targetReg);
uint32 GetPhysicalAddress(uint32 Address);

int StoreMemory(void* regValue) {
	uint32 Address;
	uint32* tempPtr;
	
	Address = MainCPUReg[rs_base_fmt]+offset_immediate;
	if (Address >= 0x80000000 && Address <= 0x9FFFFFFF)      //kseg0
		Address -= 0x80000000;
	else if (Address >= 0xA0000000 && Address <= 0xBFFFFFFF) //kseg1
		Address -= 0xA0000000;
	
	{
		if (Address < 0x00400000)                          RDRAM[Address] = regValue; return(1); //RAM
		if (Address < 0x04001000) Address -= 0x04000000; SP_DMEM[Address] = regValue; return(1); //
	    if (Address < 0x04002000) Address -= 0x40001000; SP_IMEM[Address] = regValue; return(1); //
	    if (Address < 0x04400000); return(0); //TODO: Fill this memory if necessary
		if (Address < 0x04500000) Address -= 0x04400000;      VI[Address] = regValue; return(1); //Video interface
	    if (Address < 0x04600000) Address -= 0x04500000;      AI[Address] = regValue; return(1); //Audio interface
	    if (Address < 0x04700000) Address -= 0x04600000;      PI[Address] = regValue; return(1); //Peripheral interface
		if (Address < 0x04800000) Address -= 0x04700000;      RI[Address] = regValue; return(1); //RDRAM interface
	    if (Address < 0x04900000) Address -= 0x04800000;      SI[Address] = regValue; return(1); //Serial interface	
	    //if (Address < 0x01000000); return(1); //TODO: Fill this memory if necessary
	}
}

int LoadMemory(void* targetReg) {
	uint32 Address;
	
	//Remap mirrored memory
	Address = MainCPUReg[rs_base_fmt]+offset_immediate;
	if (Address >= 0x80000000 && Address <= 0x9FFFFFFF)      //kseg0
		Address -= 0x80000000;
	else if (Address >= 0xA0000000 && Address <= 0xBFFFFFFF) //kseg1
		Address -= 0xA0000000;

	{
		if (Address < 0x00400000)                        targetReg = RDRAM[Address];   return(1); //RAM
		if (Address < 0x04001000) Address -= 0x04000000; targetReg = SP_DMEM[Address]; return(1); //
		if (Address < 0x04002000) Address -= 0x40001000; targetReg = SP_IMEM[Address]; return(1); //
		if (Address < 0x04400000); return(1); //TODO: Fill this memory if necessary
		if (Address < 0x04500000) Address -= 0x04400000; targetReg = VI[Address];      return(1); //Video interface
		if (Address < 0x04600000) Address -= 0x04500000; targetReg = AI[Address];      return(1); //Audio interface
		if (Address < 0x04700000) Address -= 0x04600000; targetReg = PI[Address];      return(1); //Peripheral interface
		if (Address < 0x04800000) Address -= 0x04700000; targetReg = RI[Address];      return(1); //RDRAM interface
		if (Address < 0x04900000) Address -= 0x04800000; targetReg = SI[Address];      return(1); //Serial interface	
		//if (Address < 0x01000000); return(1);//TODO: Fill this memory if necessary
	}
}

uint32 GetPhysicalAddress(uint32 Address) {
	if (Address >= 0x80000400 && Address <= 0x9FFFFFFF)      //kseg0
		return(Address - 0x80000400);
	else if (Address >= 0xA0000000 && Address <= 0xBFFFFFFF) //kseg1
		return(Address - 0xA0000000);
}