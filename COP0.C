#include "chipmake.h"
//#include "MainRegs.h"

void InitCOP0Registers();

void CreateCOP0() {
	InitCOP0Registers();
}

//CoProcessor Instructions
void InitCOP0Registers() {
	COP0Reg[0x00]= "Index";
	COP0Reg[0x01]= "Random";
	COP0Reg[0x02]= "EntryLo0";
	COP0Reg[0x03]= "EntryLo1";
	COP0Reg[0x04]= "Context";
	COP0Reg[0x05]= "PageMask";
	COP0Reg[0x06]= "Wired";
	COP0Reg[0x07]= "*RESERVED*";
	COP0Reg[0x08]= "BadVAddr";
	COP0Reg[0x09]= "Count";
	COP0Reg[0x0A]= "EntryHi";
	COP0Reg[0x0B]= "Compare";
	COP0Reg[0x0C]= "Status";
	COP0Reg[0x0D]= "Cause";
	COP0Reg[0x0E]= "EPC";
	COP0Reg[0x0F]= "PrevID";
	COP0Reg[0x10]= "Config";
	COP0Reg[0x11]= "LLAddr";
	COP0Reg[0x12]= "WatchLo";
	COP0Reg[0x13]= "WatchHi";
	COP0Reg[0x14]= "XContext";
	COP0Reg[0x15]= "*RESERVED*";
	COP0Reg[0x16]= "*RESERVED*";
	COP0Reg[0x17]= "*RESERVED*";
	COP0Reg[0x18]= "*RESERVED*";
	COP0Reg[0x19]= "*RESERVED*";
	COP0Reg[0x1A]= "PErr";
	COP0Reg[0x1B]= "CacheErr";
	COP0Reg[0x1C]= "TagLo";
	COP0Reg[0x1D]= "TagHi";
	COP0Reg[0x1E]= "ErrorEPC";
	COP0Reg[0x1F]= "*RESERVED*";
}

