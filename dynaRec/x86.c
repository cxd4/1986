#include    "../options.h"
#include    "../globals.h"
#include    "../debug_option.h"
#include	"x86.h"
#include	"dynaLog.h"
#include    "regcache.h"

extern void __cdecl MBox(char *debug, ...);
extern void __cdecl DisplayError (char * Message, ...);

unsigned long	JumpTargets[100];
unsigned char	*RecompCode;
unsigned long	lCodePosition, lCodeSize;

char *RegNames[] =
{
	"EAX",
	"ECX",
	"EDX",
	"EBX",
	"ESP",
	"EBP",
	"ESI",
	"EDI"
};

void SetTranslator(unsigned char *Code, unsigned long Pos, unsigned long Size)	
{
	RecompCode = Code;
	lCodePosition = Pos;
	lCodeSize = Size;
}

unsigned long GetTranslatorPosition()	
{
	return (lCodePosition);
}

void SetTarget(unsigned char bIndex)
{
  char	bPosition;

#ifdef LOG_DYNA
	LogDyna("j_point %i:\n", bIndex);
#endif LOG_DYNA

  bPosition = (char )((lCodePosition - JumpTargets[bIndex]) & 0xFF);
  RecompCode[JumpTargets[bIndex]-1] = bPosition;
}


void SetNearTarget(unsigned char bIndex)
{
  int	wPosition;

#ifdef LOG_DYNA
	LogDyna("j_point %i:\n", bIndex);
#endif LOG_DYNA

  wPosition = ((lCodePosition - JumpTargets[bIndex]));
  *(unsigned _int32*)&RecompCode[JumpTargets[bIndex]-4] = wPosition;
}


void DynaBufferOverrun()
{
	memset(dyna_CodeTable, 0xFF, CODETABLE_SIZE);
	SetTranslator(dyna_RecompCode, 0, RECOMPCODE_SIZE);
	DynaBufferOverError = TRUE;

	//DisplayError("Dynabuffer overrun");
}


void WC8(unsigned char bValue)	
{
	if (DynaBufferOverError) return;

    if (gMultiPass.WriteCode)
    {
        if ((lCodePosition + 10) > lCodeSize) DynaBufferOverrun();
	RecompCode[lCodePosition] = bValue;
	lCodePosition++;
    }
}

void WC16(unsigned int wValue)
{
	if (DynaBufferOverError) return;

    if (gMultiPass.WriteCode)
    {
        if ((lCodePosition + 10) > lCodeSize) DynaBufferOverrun();
    (*((unsigned _int16*)(&RecompCode[lCodePosition]))=(unsigned _int16)(wValue));
    lCodePosition+=2;
    }
}

void WC32(unsigned long dwValue)
{
	if (DynaBufferOverError) return;

    if (gMultiPass.WriteCode)
    {
        if ((lCodePosition + 10) > lCodeSize) DynaBufferOverrun();
    (*((unsigned _int32*)(&RecompCode[lCodePosition]))=(unsigned _int32)(dwValue));
    lCodePosition+=4;
}
}


// Now some X86 Opcodes ...
void ADC_Reg1ToReg2(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
	WC8((unsigned char)(0x10 | OperandSize));
	WC8((unsigned char)(0xD0 | (Reg1 << 3) | Reg2));

#ifdef LOG_DYNA
	LogDyna("	ADC %s, %s\n", RegNames[Reg1], RegNames[Reg2]);
#endif LOG_DYNA
}

void ADC_RegToMemory(unsigned char OperandSize, unsigned char Reg,unsigned long Addr)
{
	WC8((unsigned char)(0x12 | OperandSize));
	WC8((unsigned char)(0x05 | (Reg<<3)));
	WC32(Addr);

#ifdef LOG_DYNA
	LogDyna("	ADC [0x%08X],%s\n", Addr, RegNames[Reg]);
#endif LOG_DYNA
}

void ADC_ImmToReg(unsigned char OperandSize, unsigned char Reg,unsigned char Data)
{
	WC8((unsigned char)0x83);
	WC8((unsigned char)(0xd0 | Reg)); 
	WC8((unsigned char)Data);

#ifdef LOG_DYNA
	LogDyna("	ADC %s, 0x%08X\n", RegNames[Reg], Data);
#endif LOG_DYNA
}

void ADD_Reg1ToReg2(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
	WC8((unsigned char)(0x00 | OperandSize));
	WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));

#ifdef LOG_DYNA
	LogDyna("	ADD %s, %s\n", RegNames[Reg1], RegNames[Reg2]);
#endif LOG_DYNA
}

void ADD_Reg2ToReg1(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
	WC8((unsigned char)(0x02 | OperandSize));
	WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));

#ifdef LOG_DYNA
	LogDyna("	ADD %s, %s\n", RegNames[Reg1], RegNames[Reg2]);
#endif LOG_DYNA
}

void ADD_ImmToReg(unsigned char OperandSize, unsigned char Reg, unsigned long Data)
{
    if (Reg == Reg_EAX)
    {
        WC8((unsigned char)(0x04 | OperandSize));
    }
    else
    {
	WC8((unsigned char)(0x80 | OperandSize));
	WC8((unsigned char)(0xC0 | Reg));
    }
	WC32(Data);  

#ifdef LOG_DYNA
	LogDyna("	ADD %s, 0x%08X\n", RegNames[Reg], Data);
#endif LOG_DYNA
}

void ADD_ImmToMemory(unsigned long Address, unsigned long Data)
{
	WC8((unsigned char)0x81);
	WC8((unsigned char)0x05);
	WC32(Address);
	WC32((unsigned long)(Data));

#ifdef LOG_DYNA
	LogDyna(" ADD [0x%08X], 0x%08x\n", Address, Data);
#endif LOG_DYNA
}


void AND_ImmToEAX(unsigned char OperandSize, unsigned long Data)
{
	WC8((unsigned char)(0x24 | OperandSize));
	WC32(Data);

#ifdef LOG_DYNA
	LogDyna("	AND EAX, 0x%08X\n", Data);
#endif LOG_DYNA
}

void AND_ImmToReg(unsigned char OperandSize, unsigned char Reg, unsigned long Data)
{
	WC8((unsigned char)(0x80 | OperandSize));
	WC8((unsigned char)(0xE0 | Reg));
	if(OperandSize!=0)
		WC32(Data);
	else
		WC8((unsigned char)(Data & 0xFF)); 

#ifdef LOG_DYNA
	LogDyna("	AND %s, 0x%08X\n", RegNames[Reg], Data);
#endif LOG_DYNA
}

void AND_Reg1ToReg2(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
	WC8((unsigned char)(0x20 | OperandSize));
	WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));  

#ifdef LOG_DYNA
	LogDyna("	AND %s, %s\n", RegNames[Reg1], RegNames[Reg2]);
#endif LOG_DYNA
}

void AND_ImmToMemory(unsigned long Address, unsigned long Data)
{
	WC8((unsigned char)0x81);
	WC8((unsigned char)0x25);
	WC32(Address);
	WC32(Data);

#ifdef LOG_DYNA
	LogDyna("	AND [0x%08X], 0x%08x\n", Address, Data);
#endif LOG_DYNA
}

void AND_MemoryToReg(unsigned char OperandSize, unsigned char Reg,unsigned long Address)
{
	WC8(0x23);
	WC8((unsigned char)(0x05 | (Reg<<3)));
	WC32(Address);

#ifdef LOG_DYNA
	LogDyna("	AND %s, [0x%08X]\n", RegNames[Reg], Address);
#endif LOG_DYNA
}

void BSWAP(unsigned char Reg)
{
	WC8((unsigned char)0x0F);
	WC8((unsigned char)(0xC8 | Reg));  

#ifdef LOG_DYNA
	LogDyna("	BSWAP %s\n", RegNames[Reg]);
#endif LOG_DYNA
}

void X86_CALL(unsigned long dwAddress)
{
	unsigned	__int32	wTemp;
	wTemp = dwAddress - (unsigned	__int32)(char *)&RecompCode[lCodePosition] - 5;
	WC8(0xe8);
	WC32(wTemp);

#ifdef LOG_DYNA
	LogDyna("	CALL 0x%08X\n", dwAddress);
#endif LOG_DYNA
}

void CALL_Reg(unsigned char Reg)
{
	WC8((unsigned char)0xFF);
	WC8((unsigned char)(0xD0 | Reg));

#ifdef LOG_DYNA
	LogDyna("	CALL %s\n", RegNames[Reg]);
#endif LOG_DYNA
}


void CDQ()
{
	WC8((unsigned char)0x99);  

#ifdef LOG_DYNA
	LogDyna("	CDQ\n");
#endif LOG_DYNA
}

void CMP_Reg1WithReg2(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
	WC8((unsigned char)(0x38 | OperandSize));
	WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));

#ifdef LOG_DYNA
	LogDyna("	CMP %s, %s\n", RegNames[Reg1], RegNames[Reg2]);
#endif LOG_DYNA
}

void CMP_Reg2WithReg1(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
	WC8((unsigned char)(0x3A | OperandSize));
	WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));

#ifdef LOG_DYNA
	LogDyna("	CMP %s, %s\n", RegNames[Reg2], RegNames[Reg1]);
#endif LOG_DYNA
}

void CMP_EAXWithImm(unsigned char OperandSize, unsigned long Data)
{
	WC8((unsigned char)(0x3C | OperandSize));
	WC32(Data);
}

void CMP_RegWithImm(unsigned char OperandSize, unsigned char Reg, unsigned long Data)
{
	WC8((unsigned char)(0x80 | OperandSize));
	WC8((unsigned char)(0xF8 | Reg));
	WC32(Data);
}

void CMP_RegWithShort(unsigned char OperandSize, unsigned char Reg, unsigned char Data)
{
	WC8((unsigned char)(0x82 | OperandSize)); //assumed for now..verify
	WC8((unsigned char)(0xF8 | Reg));
	WC8(Data);
}


void CMP_MemoryWithImm(unsigned char OperandSize, unsigned long dwAddress,unsigned long Data)
{
	WC8((unsigned char)(0x80 | OperandSize));
	WC8((unsigned char)0x3D);
	WC32(dwAddress);
	WC32(Data);
}

void CMP_RegWithMemory(unsigned char Reg,unsigned long dwAddress)
{
	WC8(0x3B);
	WC8((unsigned char)(0x05 | (Reg<<3)));
	WC32(dwAddress);
}

void FABS()
{
	WC8(0xd9);
	WC8(0xe1);

#ifdef LOG_DYNA
	LogDyna("	FABS\n");
#endif LOG_DYNA
}

void FADD_Memory(unsigned char OperandSize,unsigned long dwAddress)
{
	WC8((unsigned char)(0xD8 + OperandSize));
	WC8(0x05);
	WC32(dwAddress);
}

void FCOMP(unsigned char OperandSize,unsigned long dwAddress)
{
	WC8((unsigned char)(0xD8 + OperandSize));
	WC8(0x1D);
	WC32(dwAddress);
}

void FNSTSW()
{
//	WC8(0x9B); why this ? - schibo
	WC8(0xDF);
	WC8(0xE0);

#ifdef LOG_DYNA
	LogDyna("	FNSTSW\n");
#endif LOG_DYNA
}

void FDIV_Memory(unsigned char OperandSize,unsigned long dwAddress)
{
	WC8((unsigned char)(0xD8 + OperandSize));
	WC8(0x35);
	WC32(dwAddress);
}

void FLD_Memory(unsigned char OperandSize,unsigned long dwAddress)
{
	WC8((unsigned char)(0xD9 + OperandSize));
	WC8(0x05);
	WC32(dwAddress);
}

void FLDCW_Memory(unsigned long dwAddress)
{
	WC8(0xd9);
	WC8(0x2d);
	WC32(dwAddress);
}

void FISTP_Memory(unsigned char OperandSize,unsigned long dwAddress)
{
	WC8(0xDB );
	WC8(0x1D);
	WC32(dwAddress);
}

void FILD_Memory(unsigned char OperandSize,unsigned long dwAddress)
{
	WC8((unsigned char)(0xDB + OperandSize));
	WC8(0x05);
	WC32(dwAddress);
}

void FMUL_Memory(unsigned char OperandSize,unsigned long dwAddress)
{
	WC8((unsigned char)(0xD8 + OperandSize));
	WC8(0x0d);
	WC32(dwAddress);
}

void FNEG()
{
	WC8(0xd9);
	WC8(0xe0);

#ifdef LOG_DYNA
	LogDyna("	FNEG\n");
#endif LOG_DYNA
}

void FRNDINT()
{
	WC8(0xD9);
	WC8(0xFC);

#ifdef LOG_DYNA
	LogDyna("	FRNDINT\n");
#endif LOG_DYNA
}

void FSQRT()
{
	WC8(0xd9);
	WC8(0xfa);

#ifdef LOG_DYNA
	LogDyna("	FSQRT\n");
#endif LOG_DYNA
}

void FSTP_Memory(unsigned char OperandSize,unsigned long dwAddress)
{
	WC8((unsigned char)(0xd9 + OperandSize));
	WC8(0x1d);
	WC32(dwAddress);
}

void FSUB_Memory(unsigned char OperandSize,unsigned long dwAddress)
{
	WC8((unsigned char)(0xD8 + OperandSize));
	WC8(0x25);
	WC32(dwAddress);
}

void IMUL_EAXWithReg(unsigned char OperandSize, unsigned char Reg)
{
	WC8((unsigned char)(0xF6 | OperandSize));
	WC8((unsigned char)(0xE8 | Reg));   
}

void IMUL_EAXWithMemory(unsigned char OperandSize, unsigned long Address)
{
	WC8((unsigned char)(0xF6 | OperandSize));
	WC8((unsigned char)(0x2D));
	WC32(Address); 
}

void IMUL_Reg2ToReg1(unsigned char Reg1, unsigned char Reg2)
{
    WC16(0xAF0F);
    WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));


#ifdef LOG_DYNA
	LogDyna("	IMUL %s, %s\n", RegNames[Reg2], RegNames[Reg1]);
#endif LOG_DYNA
}



void INC_Memory(unsigned long Address)
{
	WC8((unsigned char)0xFF);
	WC8((unsigned char)0x05);
	WC32(Address);
}

//Incomplete..assumed: operandsize = 1
void DEC_Reg(unsigned char OperandSize, unsigned char Reg)
{
	if(OperandSize==1)
	{
		WC8((unsigned char)(0x48 | Reg));
	}
	else
	{
        DisplayError("DEC: Incomplete");
    }  
}

void INC_Reg(unsigned char OperandSize, unsigned char Reg)
{
	if(OperandSize==1)
	{
		WC8((unsigned char)(0x40 | Reg));
	}
	else
	{
		WC8(0xfe);
		WC8((unsigned char)(0xc0 | Reg));
	}  
}

void DIV_EAXWithReg(unsigned char OperandSize, unsigned char Reg)
{
	WC8((unsigned char)(0xF6 | OperandSize));
	WC8((unsigned char)(0xF0 | Reg));
}

void IDIV_EAXWithReg(unsigned char OperandSize, unsigned char Reg)
{
	WC8((unsigned char)(0xF6 | OperandSize));
	WC8((unsigned char)(0xF8 | Reg));
}

void Jcc(unsigned char ConditionCode,unsigned char Offset)
{
	WC8((unsigned char)(0x70 | ConditionCode));
	WC8((unsigned char)Offset);
}

void Jcc_auto(unsigned char ConditionCode,unsigned long Index)
{
#ifdef LOG_DYNA
	LogDyna("	Jcc () to %i\n", Index);
#endif LOG_DYNA

  WC8((unsigned char)(0x70 | ConditionCode));
  WC8((unsigned char)0x00);
  JumpTargets[Index] = lCodePosition;
}

__inline void Jcc_Near_auto(unsigned char ConditionCode, unsigned long Index)
{
#ifdef LOG_DYNA
	LogDyna("	Jcc () to %i\n", Index);
#endif LOG_DYNA

  WC8((unsigned char)(0x0F));
  WC8((unsigned char)(0x80 | ConditionCode));
  WC32(0x00000000);
  JumpTargets[Index] = lCodePosition;
}

void JMP_Short(unsigned char Offset)
{
	WC8((unsigned char)0xEB);
	WC8((unsigned char)Offset);
}

void JMP_Short_auto(unsigned long Index)
{
#ifdef LOG_DYNA
	LogDyna("	JMP_SHORT () to %i\n", Index);
#endif LOG_DYNA

  WC8((unsigned char)0xEB);
  WC8((unsigned char)0x00);
  JumpTargets[Index] = lCodePosition;
}

void JMP_FAR(unsigned long dwAddress)
{
	WC8(0xFF);
	WC8(0x25);
	WC32(dwAddress);
}

void LEA(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
    WC8(0x8D);
    WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));
}

void Reg2ToReg1(unsigned char OperandSize, unsigned char op, unsigned char Reg1,unsigned char Reg2)
{
	WC8((unsigned char)(op | OperandSize));
	WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));

#ifdef LOG_DYNA
	LogDyna("	TODO %s, %s\n", RegNames[Reg1], RegNames[Reg2]);
#endif LOG_DYNA
}

void MOV_Reg1ToReg2(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
	if (Reg1 == Reg2) return;
	WC8((unsigned char)(0x88 | OperandSize));
	WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2)); 

#ifdef LOG_DYNA
	LogDyna("	MOV %s, %s\n", RegNames[Reg1], RegNames[Reg2]);
#endif LOG_DYNA
}

void MOV_Reg2ToReg1(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
	if (Reg1 == Reg2) return;
	WC8((unsigned char)(0x8A | OperandSize));
	WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));  

#ifdef LOG_DYNA
	LogDyna("	MOV %s, %s\n", RegNames[Reg1], RegNames[Reg2]);
#endif LOG_DYNA
}

void MOV_MemoryToReg(unsigned char OperandSize, unsigned char Reg, unsigned char ModRM, unsigned long Address)
{
	WC8((unsigned char)(0x8A | OperandSize));
	WC8((unsigned char)(ModRM | (Reg << 3)));
	if(Address!=0)
		WC32(Address);  

#ifdef LOG_DYNA
	LogDyna("	MOV %s, [0x%08X]\n", RegNames[Reg], Address);
#endif LOG_DYNA
}

void MOVSX_MemoryToReg(unsigned char OperandSize, unsigned char Reg, unsigned char ModRM, unsigned long Address)
{
	WC8(0x0F);
    WC8((unsigned char)(0xBE | OperandSize));
	WC8((unsigned char)(ModRM | (Reg << 3)));
	if(Address!=0)
		WC32(Address);  

#ifdef LOG_DYNA
	LogDyna("	MOVSX %s, [0x%08X]\n", RegNames[Reg], Address);
#endif LOG_DYNA
}

void MOVZX_MemoryToReg(unsigned char OperandSize, unsigned char Reg, unsigned char ModRM, unsigned long Address)
{
	WC8(0x0F);
    WC8((unsigned char)(0xB6 | OperandSize));
	WC8((unsigned char)(ModRM | (Reg << 3)));
	if(Address!=0)
		WC32(Address);  

#ifdef LOG_DYNA
	LogDyna("	MOVSX %s, [0x%08X]\n", RegNames[Reg], Address);
#endif LOG_DYNA
}

void MOV_MemoryToEAX(unsigned char OperandSize, unsigned long Address)
{
	WC8((unsigned char)(0xA0 | OperandSize));
	WC32(Address);
}

void MOV_RegToMemory(unsigned char OperandSize, unsigned char Reg, unsigned char ModRM, unsigned long Address)
{
	WC8((unsigned char)(0x88 | OperandSize));
	WC8((unsigned char)(ModRM | (Reg << 3)));
	if(Address!=0)
		WC32(Address);

#ifdef LOG_DYNA
	LogDyna("	MOV [0x%08X],%s\n", Address, RegNames[Reg]);
#endif LOG_DYNA
}

void MOV_RegToMemory2(unsigned char OperandSize, unsigned char Reg, unsigned char ModRM, unsigned long Address)
{
	WC8((unsigned char)(0x88 | OperandSize));
	WC8((unsigned char)(0x80 | ModRM | (Reg << 3)));
	WC32(Address);

#ifdef LOG_DYNA
	LogDyna("	MOV [0x%08X],%s\n", Address, RegNames[Reg]);
#endif LOG_DYNA
}

void MOV_MemoryToReg2(unsigned char OperandSize, unsigned char Reg, unsigned char ModRM, unsigned long Address)
{
	WC8((unsigned char)(0x8a | OperandSize));
	WC8((unsigned char)(0x80 | ModRM | (Reg << 3)));
	WC32(Address);

#ifdef LOG_DYNA
	LogDyna("	MOV %s, [0x%08X]\n", RegNames[Reg], Address);
#endif LOG_DYNA
}

void MOV_Reg16ToMemory(unsigned char Reg,unsigned char ModRM, unsigned long Address)
{
	WC8((unsigned char)0x66);
	WC8((unsigned char)(0x88 | 1));  // use 16 bit
	WC8((unsigned char)(ModRM | (Reg << 3)));
	if(Address!=0)
		WC32(Address);
}

void MOV_MemoryToReg16(unsigned char Reg,unsigned char ModRM, unsigned long Address)
{
	WC8((unsigned char)0x66);
	WC8((unsigned char)(0x8a | 1));  // use 16 bit
	WC8((unsigned char)(ModRM | (Reg << 3)));
	if(Address!=0)
		WC32(Address);

#ifdef LOG_DYNA
	LogDyna("	MOV16 %s, [0x%08X]\n", RegNames[Reg], Address);
#endif LOG_DYNA
}

void MOV_EAXToMemory(unsigned char OperandSize, unsigned long Address)
{
	WC8((unsigned char)(0xA2 | OperandSize));
	WC32(Address);
}

void MOV_ImmToReg(unsigned char OperandSize, unsigned char Reg, unsigned long Data)
{
	WC8((unsigned char)(0xB0 | (OperandSize << 3) | Reg));
	if(OperandSize==0)
	{
		WC8((unsigned char)Data);
	}
	else
	{
		WC32(Data);
	}

#ifdef LOG_DYNA
	LogDyna("	MOV %s, 0x%08X\n", RegNames[Reg], Data);
#endif LOG_DYNA
}

void MOV_ImmToMemory(unsigned char OperandSize, unsigned long Address, unsigned long Data)
{
	WC8((unsigned char)(0xC6 | OperandSize));
	WC8((unsigned char)0x05);  // Always use disp32-address
	WC32(Address);
	if(OperandSize==1)
		WC32(Data);
	else
		WC8((unsigned __int8)(Data));

#ifdef LOG_DYNA
	LogDyna("	MOV [0x%08X], 0x%08x\n", Address, Data);
#endif LOG_DYNA
}

void MOVSX_Reg2ToReg1(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
	WC8((unsigned char)0x0F);
	WC8((unsigned char)(0xBE | OperandSize));
	WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));

#ifdef LOG_DYNA
	LogDyna("	MOVSX %s, %s\n", RegNames[Reg1], RegNames[Reg2]);
#endif LOG_DYNA
}

void MOVZX_Reg2ToReg1(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
    WC8((unsigned char)0x0F);
    WC8((unsigned char)(0xB6 | OperandSize));
	WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));
}

void MUL_EAXWithReg(unsigned char OperandSize, unsigned char Reg)
{
	WC8((unsigned char)(0xF6 | OperandSize));
	WC8((unsigned char)(0xE0 | Reg));  
}

void MUL_EAXWithMemory(unsigned char OperandSize, unsigned long Address)
{
	WC8((unsigned char)(0xF6 | OperandSize));
	WC8((unsigned char)(0x25));
	WC32(Address);    
}


void NEG_Reg(unsigned char OperandSize, unsigned char Reg)
{
	WC8((unsigned char)(0xF6 | OperandSize));
	WC8((unsigned char)(0xD8 | Reg));  
}

void NOT_Reg(unsigned char OperandSize, unsigned char Reg)
{
	WC8((unsigned char)(0xF6 | OperandSize));
	WC8((unsigned char)(0xD0 | Reg));
}

void NOP()
{
	WC8((unsigned char)0x90);

#ifdef LOG_DYNA
	LogDyna("	NOP\n");
#endif LOG_DYNA
}

void OR_ImmToEAX(unsigned char OperandSize, unsigned long Data)
{
	WC8((unsigned char)(0x0C | OperandSize));
	WC32(Data);  

#ifdef LOG_DYNA
	LogDyna("	OR EAX, 0x%08X\n", Data);
#endif LOG_DYNA
}

void OR_ImmToReg(unsigned char OperandSize, unsigned char Reg, unsigned long Data)
{
	WC8((unsigned char)(0x80 | OperandSize));
	WC8((unsigned char)(0xC8 | Reg));
	if(OperandSize!=0)
		WC32(Data);
	else
		WC8((unsigned char)(Data & 0xFF)); 

#ifdef LOG_DYNA
	LogDyna("	OR %s, 0x%08X\n", RegNames[Reg], Data);
#endif LOG_DYNA
}

void OR_Reg1ToReg2(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
	WC8((unsigned char)(0x08 | OperandSize));
	WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));  

#ifdef LOG_DYNA
	LogDyna("	OR %s, %s\n", RegNames[Reg1], RegNames[Reg2]);
#endif LOG_DYNA
}

void OR_ImmToMemory(unsigned char OperandSize, unsigned long Immediate,unsigned long Address)
{
	WC8((unsigned char)0x83);
	WC8((unsigned char)0x0D);
	WC32(Address);
	WC8((unsigned char)(Immediate & 0xFF));

#ifdef LOG_DYNA
	LogDyna("	OR [0x%08X], 0x%08x\n", Address, Immediate);
#endif LOG_DYNA
}

void OR_RegToMemory(unsigned char OperandSize, unsigned char Reg,unsigned long Address)
{
	WC8((unsigned char)(0x0A | OperandSize));
	WC8((unsigned char)(0x05 | (Reg<<3)));
	WC32(Address);

#ifdef LOG_DYNA
	LogDyna("	OR [0x%08X], %s\n", Address, RegNames[Reg]);
#endif LOG_DYNA
}

void POP_RegFromStack(unsigned char Reg)
{
	WC8((unsigned char)(0x58 | Reg));

#ifdef LOG_DYNA
	LogDyna("	POP %s\n", RegNames[Reg]);
#endif LOG_DYNA
}

void POPA()
{
	WC8((unsigned char)0x66);
	WC8((unsigned char)0x61);

#ifdef LOG_DYNA
	LogDyna("	POPA\n");
#endif LOG_DYNA
}

void POPAD()
{
	WC8((unsigned char)0x61);

#ifdef LOG_DYNA
	LogDyna("	POPAD\n");
#endif LOG_DYNA
}


void PUSH_Imm8ToStack(unsigned char Imm8) 
{
	WC8(0x6A);
	WC8(Imm8);

#ifdef LOG_DYNA
	LogDyna("	PUSH 0x%02X\n", Imm8);
#endif LOG_DYNA
}



void PUSH_RegToStack(unsigned char Reg)
{
	WC8((unsigned char)(0x50 | Reg));

#ifdef LOG_DYNA
	LogDyna("	PUSH %s\n", RegNames[Reg]);
#endif LOG_DYNA
}

void PUSH_WordToStack(unsigned	__int32 wWord)
{
	WC8(0x68);
	WC32(wWord);

#ifdef LOG_DYNA
	LogDyna("	PUSH 0x%08X\n", wWord);
#endif LOG_DYNA
}

void PUSHA()
{
	WC8((unsigned char)0x66);
	WC8((unsigned char)0x60);

#ifdef LOG_DYNA
	LogDyna("	PUSHAA\n");
#endif LOG_DYNA
}

void PUSHAD()
{
	WC8((unsigned char)0x60);

#ifdef LOG_DYNA
	LogDyna("	PUSHADA\n");
#endif LOG_DYNA
}

void RET()
{
	WC8((unsigned char)0xC3);

#ifdef LOG_DYNA
	LogDyna("	RET\n");
#endif LOG_DYNA
}

void SAHF()
{
	WC8(0x9e);

#ifdef LOG_DYNA
	LogDyna("	SAHF\n");
#endif LOG_DYNA
}

void SETcc_Reg(unsigned char ConditionCode, unsigned char Reg)
{
	WC8((unsigned char)0x0F);
	WC8((unsigned char)(0x90 | ConditionCode));
	WC8((unsigned char)(0xC0 | Reg));
}

void SAR_RegByCL(unsigned char OperandSize, unsigned char Reg)
{
	WC8((unsigned char)(0xD2 | OperandSize));
	WC8((unsigned char)(0xF8 | Reg));
}

void SHL_RegByImm(unsigned char OperandSize, unsigned char Reg,unsigned char Data)
{
    WC8((unsigned char)(0xC0 | OperandSize));
	WC8((unsigned char)(0xE0 | Reg));
	WC8((unsigned char)Data);
}

void SHR_RegByImm(unsigned char OperandSize, unsigned char Reg,unsigned char Data)
{
	WC8((unsigned char)(0xC0 | OperandSize));
	WC8((unsigned char)(0xE8 | Reg));
	WC8((unsigned char)Data);
}

void SAR_RegByImm(unsigned char OperandSize, unsigned char Reg,unsigned char Data)
{
    WC8((unsigned char)(0xC0 | OperandSize));
	WC8((unsigned char)(0xF8 | Reg));
	WC8((unsigned char)Data);
}

void SHL_MemoryByCL(unsigned char OperandSize, unsigned char Reg, unsigned long Address)
{
	WC8((unsigned char)(0xD2 | OperandSize));
	WC8((unsigned char)(0xA4));
	if(Address!=0)
		WC32(Address);

#ifdef LOG_DYNA
	LogDyna("	SHL %s, [0x%08X]\n", RegNames[Reg], Address);
#endif LOG_DYNA
}

void SHR_MemoryByCL(unsigned char OperandSize, unsigned char Reg, unsigned long Address)
{
	WC8((unsigned char)(0xD2 | OperandSize));
	WC8((unsigned char)(0xAC));
	if(Address!=0)
		WC32(Address);

#ifdef LOG_DYNA
	LogDyna("	SHR %s, [0x%08X]\n", RegNames[Reg], Address);
#endif LOG_DYNA
}

void SAR_MemoryByCL(unsigned char OperandSize, unsigned char Reg, unsigned long Address)
{
	WC8((unsigned char)(0xD2 | OperandSize));
	WC8((unsigned char)(0xBC));
	if(Address!=0)
		WC32(Address);

#ifdef LOG_DYNA
	LogDyna("	SAR %s, [0x%08X]\n", RegNames[Reg], Address);
#endif LOG_DYNA
}


void SHL_RegBy1(unsigned char OperandSize, unsigned char Reg)
{
	WC8((unsigned char)(0xD0 | OperandSize));
	WC8((unsigned char)(0xE0 | Reg));
}


void SHL_RegByCL(unsigned char OperandSize, unsigned char Reg)
{
	WC8((unsigned char)(0xD2 | OperandSize));
	WC8((unsigned char)(0xE0 | Reg));
}

void SHR_RegByCL(unsigned char OperandSize, unsigned char Reg)
{
	WC8((unsigned char)(0xD2 | OperandSize));
	WC8((unsigned char)(0xE8 | Reg));
}

void SUB_Reg1OfReg2(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
	WC8((unsigned char)(0x28 | OperandSize));
	WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));

#ifdef LOG_DYNA
	LogDyna("	SUB %s, %s\n", RegNames[Reg1], RegNames[Reg2]);
#endif LOG_DYNA
}

void SUB_ImmToMemory(unsigned long Address, unsigned long Data)
{
	WC8((unsigned char)0x81);
	WC8((unsigned char)0x2D);
	WC32(Address);
	WC32((unsigned long)(Data));

#ifdef LOG_DYNA
	LogDyna(" SUB [0x%08X], 0x%08x\n", Address, Data);
#endif LOG_DYNA
}

__inline void TEST_ImmWithMemory(unsigned long dwAddress,unsigned long Value)
{
	WC8(0xf7); WC8(0x05);
	WC32(dwAddress);
	WC32(Value);
}

void TEST_EAXWithImm(unsigned char OperandSize,unsigned long dwAddress)
{
	WC8((unsigned char)(0xa8 | OperandSize));
	WC32(dwAddress);
}

void TEST_EAXWithEAX()
{
	WC8(0x85); WC8(0xc0);

#ifdef LOG_DYNA
	LogDyna("	TEST EAX, EAX\n");
#endif LOG_DYNA
}

void TEST_Reg2WithReg1(unsigned char OperandSize,unsigned char Reg1, unsigned char Reg2)
{
    WC8((unsigned char)(0x84 | OperandSize));
    WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));
}

void TEST_RegWithImm(unsigned char OperandSize,unsigned char iReg,unsigned long dwAddress)
{
	WC8((unsigned char)(0xf6 | OperandSize));
	WC8((unsigned char)(0xc0 | iReg));
	WC32(dwAddress);
}

void XCHG_Reg1WithReg2(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
	WC8((unsigned char)(0x86 | OperandSize));
	WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));

#ifdef LOG_DYNA
	LogDyna("	XCHG %s, %s\n", RegNames[Reg1], RegNames[Reg2]);
#endif LOG_DYNA
}

void XOR_ImmToEAX(unsigned char OperandSize, unsigned long Data)
{
	WC8((unsigned char)(0x34 | OperandSize));
	WC32(Data);	

#ifdef LOG_DYNA
	LogDyna("	OR EAX, 0x%08X\n", Data);
#endif LOG_DYNA
}

void XOR_Reg1ToReg2(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
	WC8((unsigned char)(0x30 | OperandSize));
	WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));

#ifdef LOG_DYNA
	LogDyna("	XOR %s, %s\n", RegNames[Reg1], RegNames[Reg2]);
#endif LOG_DYNA
}


void XOR_Reg2ToReg1(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
	WC8((unsigned char)(0x32 | OperandSize));
	WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));

#ifdef LOG_DYNA
	LogDyna("	XOR %s, %s\n", RegNames[Reg1], RegNames[Reg2]);
#endif LOG_DYNA
}

void XOR_ImmToReg(unsigned char OperandSize, unsigned char Reg,unsigned long Data)
{
	WC8((unsigned char)(0x80 | OperandSize));
	WC8((unsigned char)(0xF0 | Reg));
	if(OperandSize==0)
	{
		WC8((unsigned char)(unsigned char)Data);
	}
	else
	{
		WC32(Data);
	}

#ifdef LOG_DYNA
	LogDyna("	XOR %s, 0x%08X\n", RegNames[Reg], Data);
#endif LOG_DYNA
}


//Incomplete!: Assumed: OperandSize = 1
void OR_ShortToReg(unsigned char OperandSize, unsigned char Reg, unsigned char Data)
{
	WC8((unsigned char)(0x82 | OperandSize));
	WC8((unsigned char)(0xC8 | Reg));
    WC8((unsigned char)(unsigned char)Data);

#ifdef LOG_DYNA
	LogDyna("	XOR %s, 0x%08X\n", RegNames[Reg], Data);
#endif LOG_DYNA
}


//Incomplete!: Assumed: OperandSize = 1
void XOR_ShortToReg(unsigned char OperandSize, unsigned char Reg, unsigned char Data)
{
	WC8((unsigned char)(0x82 | OperandSize));
	WC8((unsigned char)(0xF0 | Reg));
    WC8((unsigned char)(unsigned char)Data);

#ifdef LOG_DYNA
	LogDyna("	XOR %s, 0x%08X\n", RegNames[Reg], Data);
#endif LOG_DYNA
}


void AND_Reg2ToReg1(unsigned char OperandSize, unsigned char Reg1,unsigned char Reg2)
{
 WC8((unsigned char)(0x22 | OperandSize));
 WC8((unsigned char)(0xC0 | (Reg1 << 3) | Reg2));

#ifdef LOG_DYNA
 LogDyna(" AND %s, %s\n", RegNames[Reg1], RegNames[Reg2]);
#endif LOG_DYNA
}




