#ifndef _DYNAREC_H__1964_
#define _DYNAREC_H__1964_

//#define USE_MMX				1
//#define ADDRESS_RESOLUTION	1

#ifdef USE_MMX
#define CDQ_STORE(Dest, TO_MEM)																\
																							\
/*		 99					  cdq												*/			\
/*0040FC99 0F 6E CA             movd        mm1,edx								*/			\
/*0040FC9C 0F 6E D0             movd        mm2,eax								*/			\
/*0040FC9F 0F 73 F1 20          psllq       mm1,20h (32)						*/			\
/*0040FCA3 0F EB CA             por         mm1,mm2								*/			\
/*0040FCA6 0F 7F 0D 80 8A 4A 00 movq        mmword ptr [_GPR (004a8a80)],mm1	*/			\
/*0040FCAD 0F 77                emms											*/			\
																							\
		PUTDST32KNOWN(Dest, 0xCA6E0F99)		/* CDQ */										\
		PUTDST32KNOWN(Dest, 0x0FD06E0F)														\
		PUTDST32KNOWN(Dest, 0x0F20F173)														\
		PUTDST32KNOWN(Dest, 0x7F0FCAEB)	PUTDST8KNOWN(Dest, 0x0D)	PUTDST32(Dest, TO_MEM)	\
		PUTDST16KNOWN(Dest, 0x770F)
#else
#define CDQ_STORE(Dest, TO_MEM)																\
		/*0040F916 99                   cdq												*/	\
		/*0040F917 A3 E0 01 4B 00       mov         [_GPR (004b01e0)],eax				*/	\
		/*0040F91C 89 15 E4 01 4B 00    mov         dword ptr [_GPR+4 (004b01e4)],edx	*/	\
																							\
		PUTDST16KNOWN(Dest, 0xA399)			PUTDST32(Dest, TO_MEM)							\
		PUTDST16KNOWN(Dest, 0x1589)			PUTDST32(Dest, TO_MEM+4)
#endif

//Intel near jump instructions
#define JNE_NEAR	0x75
#define  JE_NEAR	0x74
#define  JG_NEAR	0x7F
#define  JL_NEAR	0x7C
#define	 JA_NEAR	0x77
#define  JB_NEAR	0x72
#define JAE_NEAR	0x73
#define JBE_NEAR	0x76

#define BLEZ_MACRO		BRANCH_MACRO(JG_NEAR, JL_NEAR, JA_NEAR)
#define BLTZ_MACRO		BRANCH_MACRO(JG_NEAR, JL_NEAR, JAE_NEAR)
#define BGEZ_MACRO		BRANCH_MACRO(JL_NEAR, JG_NEAR, JB_NEAR)
#define BGTZ_MACRO		BRANCH_MACRO(JL_NEAR, JG_NEAR, JBE_NEAR)

#define YES_LINK		SetRAToPC(Dest, pc+4)
#define  NO_LINK

#define YES_LIKELY		SetVariable(Dest, &pc, pc+4)
#define NOT_LIKELY		Dest = FetchDelay((pc + 4), Dest);

#define YES_TWO_FALSES	*LabelFalse2 = (uint8)(Dest - PrevLabelFalse2);
#define ONE_FALSE_ONLY

#define NO_SPEED_HACK
#define DO_SPEEDHACK	{										\
	if ((uint16)OFFSET_IMMEDIATE == 0xFFFF)						\
	{															\
	/*	DisplayError("Count = %d", COP0Reg[COUNT]);			*/	\
		SetVariable(Dest, &COP0Reg[COUNT], MAGICNUMBER);		\
	}															\
}


#define sADD_DYN	0xC103
#define sSUB_DYN	0xC12B
//Opcode Logic Macros 
//---------------------

/*
00419C99 A1 80 41 4A 00       mov         eax,[_GPR (004a4180)]
00419C9E 8B 0D 80 41 4A 00    mov         ecx,dword ptr [_GPR (004a4180)]
00419CA4 03 C1                add         eax,ecx
00419CA6 99                   cdq
00419CA7 A3 80 41 4A 00       mov         [_GPR (004a4180)],eax
00419CAC 89 15 84 41 4A 00    mov         dword ptr [_GPR+4 (004a4184)],edx
*/
//Purpose: to perform an operation on 2 (_int32)GPRs, return Sum in a GPR
//Syntax: Sum =	(_int64)((_int32)Operand1 OPERATOR (_int32)Operand2)
#define   sLOGIC_DYN(Sum, Operand1, OPERATOR, Operand2)				\
	 uint32 SumAddr = (uint32)&Sum;									\
	PUTDST8KNOWN(Dest, 0xA1)	PUTDST32(Dest, (uint32)&Operand1)	\
	PUTDST16KNOWN(Dest, 0x0D8B)	PUTDST32(Dest, (uint32)&Operand2)	\
																	\
	PUTDST16KNOWN(Dest, OPERATOR)									\
	CDQ_STORE(Dest, SumAddr)


#define  sLOGICAL_DYN(OPERATOR)				sLOGIC_DYN(GPR[_RD_],  GPR[_RS_], OPERATOR, GPR[_RT_])




//00415782 D9 05 28 70 4A 00    fld         dword ptr [_COP1Reg+8 (004a7028)]
//00415788 D8 05 30 70 4A 00    fadd        dword ptr [_COP1Reg+10h (004a7030)]
//0041578E D9 1D 20 70 4A 00    fstp        dword ptr [_COP1Reg (004a7020)]
#define  fsLOGIC(Sum, Operand1, OPERATOR16, Operand2)						\
	PUTDST16KNOWN(Dest, 0x05D9)			PUTDST32(Dest, (uint32)&Operand1)	\
	PUTDST16KNOWN(Dest, OPERATOR16)		PUTDST32(Dest, (uint32)&Operand2)	\
	PUTDST16KNOWN(Dest, 0x1DD9)			PUTDST32(Dest, (uint32)&Sum)			

#define fsingleLOGICAL(OPERATOR16)		fsLOGIC(COP1Reg[SA_FD], COP1Reg[RD_FS], OPERATOR16, COP1Reg[RT_FT])


//	*((double *)&COP1Reg[SA_FD]) =  *((double *)&COP1Reg[RD_FS])  / *((double *)&COP1Reg[RT_FT]);	}
//	0041A8A9 DD 05 24 50 4B 00    fld         qword ptr [_COP1Reg+4 (004b5024)]
//	0041A8AF DC 35 28 50 4B 00    fdiv        qword ptr [_COP1Reg+8 (004b5028)]
//	0041A8B5 DD 1D 20 50 4B 00    fstp        qword ptr [_COP1Reg (004b5020)]
#define  fdLOGIC(Sum, Operand1, OPERATOR16, Operand2)						\
	PUTDST16KNOWN(Dest, 0x05DD)		PUTDST32(Dest, (uint32)&Operand1)		\
	PUTDST16KNOWN(Dest, OPERATOR16)	PUTDST32(Dest, (uint32)&Operand2)		\
	PUTDST16KNOWN(Dest, 0x1DDD)		PUTDST32(Dest, (uint32)&Sum)

#define fdoubleLOGICAL(OPERATOR16)		fdLOGIC(COP1Reg[SA_FD], COP1Reg[RD_FS], OPERATOR16, COP1Reg[RT_FT])

#define uDAND_DYN	0x0523
#define uDOR_DYN	0x050B
#define uDXOR_DYN	0x0533

#define uDAND_DYN2	0x0D23
#define uDOR_DYN2	0x0D0B
#define uDXOR_DYN2	0x0D33


/*
149:      GPR[0] = (uint64)GPR[1] & (uint64)GPR[2];
0041A7B9 A1 88 51 4A 00       mov         eax,[_GPR+8 (004a5188)]
0041A7BE 23 05 90 51 4A 00    and         eax,dword ptr [_GPR+10h (004a5190)]

0041A7C4 8B 0D 8C 51 4A 00    mov         ecx,dword ptr [_GPR+0Ch (004a518c)]
0041A7CA 23 0D 94 51 4A 00    and         ecx,dword ptr [_GPR+14h (004a5194)]

0041A7D0 A3 80 51 4A 00       mov         [_GPR (004a5180)],eax
0041A7D5 89 0D 84 51 4A 00    mov         dword ptr [_GPR+4 (004a5184)],ecx
*/
//Purpose: to perform an operation on 2 (uint64)GPRs, return in a GPR
//Syntax: Sum = (uint64)Operand1 OPERATOR (uint64)Operand2
#define uDLOGIC_DYN(Sum, Operand1, OPERATOR1, OPERATOR2, Operand2)						\
	uint32 SumAddr = (uint32)&Sum;														\
	uint32 Operand1Addr = (uint32)&Operand1;											\
	uint32 Operand2Addr = (uint32)&Operand2;											\
																						\
	PUTDST8KNOWN(Dest, 0xA1)							PUTDST32(Dest, Operand1Addr)	\
	PUTDST16KNOWN(Dest, OPERATOR1)						PUTDST32(Dest, Operand2Addr)	\
																						\
	PUTDST16KNOWN(Dest, 0x0D8B)							PUTDST32(Dest, Operand1Addr+4)	\
	PUTDST16KNOWN(Dest, OPERATOR2)						PUTDST32(Dest, Operand2Addr+4)	\
																						\
	 PUTDST8KNOWN(Dest, 0xA3)							PUTDST32(Dest, SumAddr)			\
	PUTDST16KNOWN(Dest, 0x0D89)							PUTDST32(Dest, SumAddr+4)

#define uDLOGICAL_DYN(OPERATOR1, OPERATOR2)		uDLOGIC_DYN(GPR[_RD_], GPR[_RS_], OPERATOR1, OPERATOR2, GPR[_RT_])




#define sADDI_DYN	0x05
/*
0041AC49 A1 88 51 4A 00       mov         eax,[_GPR+8 (004a5188)]
0041AC4E 05 78 56 34 12       add         eax,12345678h

0041AC53 99                   cdq

0041AC54 A3 80 51 4A 00       mov         [_GPR (004a5180)],eax
0041AC59 89 15 84 51 4A 00    mov         dword ptr [_GPR+4 (004a5184)],edx
*/
#define sLOGICAL_WITH_CONST32(Sum, Operand1, OPERATOR, int32ConstOperand2)	\
	uint32 SumAddr = (uint32)&Sum;											\
	PUTDST8KNOWN(Dest, 0xA1)		PUTDST32(Dest, (uint32)&Operand1)		\
	PUTDST8KNOWN(Dest, OPERATOR)	PUTDST32(Dest, int32ConstOperand2)		\
																			\
	CDQ_STORE(Dest, SumAddr)
	
#define sLOGICAL_WITH_IMM_DYN(OPERATOR)	 sLOGICAL_WITH_CONST32(GPR[_RT_], GPR[_RS_], OPERATOR, (_int32)(_int16)OFFSET_IMMEDIATE)

#ifdef ADDRESS_RESOLUTION
#define JUMP_CONTINUE_DEFS		uint32* BranchToJump;	uint32* BranchToContinue;
#else
#define JUMP_CONTINUE_DEFS
#endif

#define BRANCH_MACRO(NEAR1, NEAR2, NEAR3)								\
	uint32 PrevLabelTrue,	PrevLabelFalse, PrevLabelFalse2;			\
	uint8 *LabelTrue,		*LabelFalse,	*LabelFalse2;				\
	JUMP_CONTINUE_DEFS													\
																		\
	uint32 RS_Addr = (uint32)&GPR[RS_BASE_FMT];							\
	uint32 RA_Addr = (uint32)&GPR[RA];									\
																		\
	CompConst8ToVariable(Dest, 0, RS_Addr+4)							\
	NearJumpConditionLabel8(Dest, LabelFalse, NEAR1)					\
	PrevLabelFalse = (uint32)Dest; /* Mark */							\
																		\
	NearJumpConditionLabel8(Dest, LabelTrue, NEAR2)						\
	PrevLabelTrue = (uint32)Dest; /* Mark */							\
																		\
	CompConst8ToVariable(Dest, 0, RS_Addr)								\
	NearJumpConditionLabel8(Dest, LabelFalse2, NEAR3)					\
	PrevLabelFalse2 = (uint32)Dest;


#ifdef ADDRESS_RESOLUTION
#define SETUP_BRANCH_DELAYS(LINK_NOLINK, LIKELY_UNLIKELY, SECOND_FALSE_POINTER, SPEED_HACK)		\
{																					\
																					\
/* $true: */																		\
*LabelTrue  = (uint8)(Dest - PrevLabelTrue);										\
																					\
	SPEED_HACK																		\
																					\
	INCREMENT_COUNT(Dest)															\
	_asm {add pc, 4}																\
	*InstructionPointer++;															\
																					\
	LINK_NOLINK																		\
																					\
	/* Get Delay */																	\
	Dest = FetchDelay((pc + (OFFSET_IMMEDIATE << 2)), Dest);						\
																					\
																					\
	/* jmp $BranchToJump */															\
	JumpEAX32(Dest, BranchToJump)													\
																					\
/* $false: */																		\
*LabelFalse  = (uint8)(Dest - PrevLabelFalse);										\
SECOND_FALSE_POINTER																\
																					\
	INCREMENT_COUNT(Dest)															\
																					\
	LINK_NOLINK																		\
																					\
	LIKELY_UNLIKELY																	\
																					\
																					\
	/* jmp $BranchToContinue */														\
	JumpEAX32(Dest, BranchToContinue)												\
																					\
																					\
/* $BranchToJump: */																\
*BranchToJump = (uint32)Dest;														\
	SetVariable(Dest, &LocationJumpedFrom, BranchToJump)							\
	AsmReturn																		\
																					\
/* $BranchToContinue */																\
*BranchToContinue = (uint32)Dest;													\
	SetVariable(Dest, &LocationJumpedFrom, BranchToContinue)						\
	AsmReturn																		\
																					\
	KEEP_RECOMPILING = 0;															\
}
#else
#define SETUP_BRANCH_DELAYS(LINK_NOLINK, LIKELY_UNLIKELY, SECOND_FALSE_POINTER, SPEED_HACK)		\
{																					\
																					\
/* $true: */																		\
*LabelTrue  = (uint8)(Dest - PrevLabelTrue);										\
																					\
	SPEED_HACK																		\
																					\
	INCREMENT_COUNT(Dest)															\
	_asm {add pc, 4}																\
	*InstructionPointer++;															\
																					\
	LINK_NOLINK																		\
																					\
	/* Get Delay */																	\
	Dest = FetchDelay((pc + (OFFSET_IMMEDIATE << 2)), Dest);						\
																					\
	AsmReturn																		\
																					\
/* $false: */																		\
*LabelFalse  = (uint8)(Dest - PrevLabelFalse);										\
SECOND_FALSE_POINTER																\
																					\
	INCREMENT_COUNT(Dest)															\
																					\
	LINK_NOLINK																		\
																					\
	LIKELY_UNLIKELY																	\
																					\
																					\
	AsmReturn																		\
																					\
	KEEP_RECOMPILING = 0;															\
}	
#endif

#define USE_BRANCH_SPEEDHACKS	1
#define USE_J_SPEEDHACK		1

#ifdef  USE_BRANCH_SPEEDHACKS
#define SETUP_DELAY_SLOTS_UNLIKELY_TF				SETUP_BRANCH_DELAYS(NO_LINK,  NOT_LIKELY, ONE_FALSE_ONLY, DO_SPEEDHACK)
#define SETUP_DELAY_SLOTS_UNLIKELY_TFF				SETUP_BRANCH_DELAYS(NO_LINK,  NOT_LIKELY, YES_TWO_FALSES, DO_SPEEDHACK)
#define SETUP_DELAY_SLOTS_UNLIKELY_AND_LINK_TFF		SETUP_BRANCH_DELAYS(YES_LINK, NOT_LIKELY, YES_TWO_FALSES, DO_SPEEDHACK)
#define SETUP_DELAY_SLOTS_LIKELY_TF					SETUP_BRANCH_DELAYS(NO_LINK,  YES_LIKELY, ONE_FALSE_ONLY, DO_SPEEDHACK)
#define SETUP_DELAY_SLOTS_LIKELY_TFF				SETUP_BRANCH_DELAYS(NO_LINK,  YES_LIKELY, YES_TWO_FALSES, DO_SPEEDHACK)
#define SETUP_DELAY_SLOTS_LIKELY_AND_LINK_TFF		SETUP_BRANCH_DELAYS(YES_LINK, YES_LIKELY, YES_TWO_FALSES, DO_SPEEDHACK)
#else
#define SETUP_DELAY_SLOTS_UNLIKELY_TF				SETUP_BRANCH_DELAYS(NO_LINK,  NOT_LIKELY, ONE_FALSE_ONLY, NO_SPEED_HACK)
#define SETUP_DELAY_SLOTS_UNLIKELY_TFF				SETUP_BRANCH_DELAYS(NO_LINK,  NOT_LIKELY, YES_TWO_FALSES, NO_SPEED_HACK)
#define SETUP_DELAY_SLOTS_UNLIKELY_AND_LINK_TFF		SETUP_BRANCH_DELAYS(YES_LINK, NOT_LIKELY, YES_TWO_FALSES, NO_SPEED_HACK)
#define SETUP_DELAY_SLOTS_LIKELY_TF					SETUP_BRANCH_DELAYS(NO_LINK,  YES_LIKELY, ONE_FALSE_ONLY, NO_SPEED_HACK)
#define SETUP_DELAY_SLOTS_LIKELY_TFF				SETUP_BRANCH_DELAYS(NO_LINK,  YES_LIKELY, YES_TWO_FALSES, NO_SPEED_HACK)
#define SETUP_DELAY_SLOTS_LIKELY_AND_LINK_TFF		SETUP_BRANCH_DELAYS(YES_LINK, YES_LIKELY, YES_TWO_FALSES, NO_SPEED_HACK)
#endif




// Many thanks to zilmar for help with macros..
#define  PUTDST8(x,y)	{ (*((uint8* )(x))=(uint8 )(y)); x++;  }
#define PUTDST16(x,y)	{ (*((uint16*)(x))=(uint16)(y)); x+=2; } 
#define PUTDST32(x,y)	{ (*((uint32*)(x))=(uint32)(y)); x+=4; }


//If y is a known hexadecimal constant, use these...they're a bit faster
#define PUTDST32KNOWN(x,y) __asm		\
/* Port output */						\
{										\
__asm	mov			eax, dword ptr [x]	\
__asm	mov			dword ptr [eax], y	\
__asm	add			eax, 4				\
__asm	mov			dword ptr [x], eax	\
}
#define PUTDST16KNOWN(x,y) __asm		\
{										\
__asm	mov         eax,dword ptr [x]	\
__asm	mov         word ptr [eax],y	\
__asm	add         eax,2				\
__asm	mov         dword ptr [x],eax	\
}


#define PUTDST8KNOWN(x,y) __asm			\
{										\
__asm   mov			eax,dword ptr [x]	\
__asm   mov         byte ptr [eax],y	\
__asm   inc         eax					\
__asm   mov         dword ptr [x],eax	\
}


#define INTERPRET(OPCODE)				\
	PUTDST8KNOWN(Dest, 0xB9)			\
	PUTDST32(Dest, Instruction)			\
	C_CALL(Dest,(uint32)&OPCODE)


#define AsmReturn	PUTDST8(Dest, 0xC3)

#define CompConst8ToVariable(Dest, Const, Variable)	\
{													\
	PUTDST16KNOWN(Dest, 0x3D83)  /* cmp 8*/			\
	PUTDST32(Dest, Variable)						\
	 PUTDST8(Dest, Const)							\
}


#define CompMIPS64RegTo0(Dest, Register)	\
{											\
	/* mov, eax [Register] */				\
	 PUTDST8KNOWN(Dest, 0xA1)				\
	PUTDST32(Dest, Register)				\
	/* or eax, dword ptr [Register+4] */	\
	PUTDST16KNOWN(Dest, 0x050B)				\
	PUTDST32(Dest, (uint32*)Register+1)		\
	/* test eax, eax */						\
	PUTDST16KNOWN(Dest, 0xC085)				\
}

#define CompMIPS64RegToMIPS64Reg_LO(Dest, Register1, Register2)	\
{																\
	/* mov eax, [Register1(lo)] */								\
	 PUTDST8KNOWN(Dest, 0xA1)									\
	PUTDST32(Dest, Register1)									\
	/* cmp eax, dword ptr [Register2(lo)] */					\
	 PUTDST16KNOWN(Dest, 0x053B)								\
	 PUTDST32(Dest, Register2)									\
}

#define CompMIPS64RegToMIPS64Reg_HI(Dest, Register1, Register2)	\
{																\
	/* mov eax, [Register1(hi)] */								\
	PUTDST8KNOWN(Dest, 0xA1)									\
	PUTDST32(Dest, (uint32*)Register1+1)						\
	/* cmp eax, dword ptr [Register2(hi)] */					\
	PUTDST16KNOWN(Dest, 0x053B)									\
	PUTDST32(Dest, (uint32*)Register2+1)						\
}



#define INCREMENT_COUNT(Dest)									\
	/* inc COP0Reg[COUNT] */									\
	PUTDST16KNOWN(Dest, 0x05FF)									\
	PUTDST32(Dest, (uint32)&COP0Reg[COUNT])

#define INCREMENT_COUNT_VAL(Dest)										\
	PUTDST8KNOWN(Dest, 0xA1)	PUTDST32(Dest, (uint32)&COP0Reg[COUNT])	\
	PUTDST8KNOWN(Dest, 0x05)											\
																		\
Label = (uint32*)&Dest[0];												\
	PUTDST32KNOWN(Dest, 0x00000000)										\
	PUTDST8KNOWN(Dest, 0xA3)	PUTDST32(Dest, (uint32)&COP0Reg[COUNT])	




extern void HandleBreakpoint(uint32 Instruction);

#define C_CALL(Dest, FunctAddress)  \
{									\
	LOADIMM32(FunctAddress)			\
	PUTDST16KNOWN(Dest,0xd0ff)		\
}

#define JumpLocal8(Dest, Label)						\
{													\
	PUTDST8KNOWN(Dest, 0xEB)						\
	Label = (uint8*)&Dest[0];						\
	PUTDST8KNOWN(Dest, 0)							\
}

#define JumpEAX32(Dest, Label)						\
  /* mov eax, Label */								\
	PUTDST8KNOWN(Dest, 0xB8)						\
	Label = (uint32*)&Dest[0];						\
	PUTDST32(Dest, Label)							\
	/* jmp eax	*/									\
	PUTDST16KNOWN(Dest, 0xE0FF)


#define LOADIMM32(x)				\
{									\
  /* mov eax, 32bit value */		\
	PUTDST8KNOWN(Dest,0xB8)			\
	PUTDST32(Dest,(uint32)x)		\
}

#define NearJumpConditionLabel8(Dest, Label, Condition)		\
{															\
	PUTDST8(Dest, Condition)	/* jne, jl, etc */			\
	Label = Dest;											\
	PUTDST8KNOWN(Dest, 0)									\
}

#define SetRAToPC(Dest, pcLoc)								\
	SetVariable(Dest, RA_Addr, pcLoc)						\
	SetVariable(Dest, RA_Addr+4, 0xFFFFFFFF)



#define SetVariable(Dest, Variable, NewValue)		\
{													\
	PUTDST16KNOWN(Dest, 0x05C7)						\
	PUTDST32(Dest, (uint32)Variable)				\
	PUTDST32(Dest, (uint32)NewValue)				\
}



#define OP_PARAMS	uint32 Instruction, uint8* Dest
#define PASS_PARAMS	Instruction, Dest
#define RETURN_TYPE	uint8*		

extern RETURN_TYPE Dyn_Intr_Common(uint8* Dest);

extern RETURN_TYPE (*dyn_CPU_instruction[64])(		OP_PARAMS);
extern RETURN_TYPE (*dyn_SPECIAL_Instruction[64])(	OP_PARAMS);
extern RETURN_TYPE (*dyn_REGIMM_Instruction[32])(	OP_PARAMS);
extern RETURN_TYPE (*dyn_COP0_Instruction[32])(		OP_PARAMS);
extern RETURN_TYPE (*dyn_TLB_Instruction[64])(		OP_PARAMS);
extern RETURN_TYPE (*dyn_COP1_Instruction[32])(		OP_PARAMS);
extern RETURN_TYPE (*dyn_COP1_BC_Instruction[4])(	OP_PARAMS);
extern RETURN_TYPE (*dyn_COP1_S_Instruction[64])(	OP_PARAMS);
extern RETURN_TYPE (*dyn_COP1_D_Instruction[64])(	OP_PARAMS);
extern RETURN_TYPE (*dyn_COP1_W_Instruction[64])(	OP_PARAMS);
extern RETURN_TYPE (*dyn_COP1_L_Instruction[64])(	OP_PARAMS);
extern RETURN_TYPE dyn_UNUSED(						OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_cache(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_daddiu(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_lb(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_lbu(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_ld(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_ldl(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_ldr(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_lh(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_lhu(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_ll(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_lld(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_lw(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_lwl(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_lwr(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_lwu(					OP_PARAMS); 
extern RETURN_TYPE dyn_r4300i_sb(					OP_PARAMS); 
extern RETURN_TYPE dyn_r4300i_sc(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_scd(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_sd(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_sdl(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_sdr(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_sh(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_sw(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_swl(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_swr(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_ldc1(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_lwc1(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_sdc1(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_swc1(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_lui(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_addiu(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_addi(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_andi(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_daddi(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_ori(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_slti(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_sltiu(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_xori(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_beq(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_beql(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_bne(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_bnel(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_jal(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_ldc2(					OP_PARAMS);  
extern RETURN_TYPE dyn_r4300i_swc2(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_bltz(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_bgez(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_bltzl(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_bgezl(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_tgei(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_tgeiu(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_tlti(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_tltiu(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_teqi(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_tnei(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_bltzal(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_bgezal(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_bltzall(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_bgezall(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_sll(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_srl(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_sra(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_sllv(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_srlv(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_srav(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_jr(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_jr_runtime(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_jalr(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_syscall(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_Break(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_sync(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_mfhi(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_mthi(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_mflo(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_mtlo(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_dsllv(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_dsrlv(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_dsrav(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_mult(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_multu(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_div(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_divu(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_dmult(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_dmultu(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_ddiv(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_ddivu(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_add(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_addu(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_sub(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_subu(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_and(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_or(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_xor(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_nor(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_slt(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_sltu(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_dadd(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_daddu(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_dsub(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_dsubu(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_tge(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_tgeu(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_tlt(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_tltu(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_teq(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_tne(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_dsll(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_dsrl(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_dsra(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_dsll32(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_dsrl32(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_dsra32(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP0_mfc0(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP0_mtc0(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_cfc1(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_ctc1(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_dmfc1(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_dmtc1(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_mfc1(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_mtc1(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_add_s(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_add_d(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_sub_s(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_sub_d(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_mul_s(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_mul_d(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_div_s(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_div_d(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_sqrt_s(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_sqrt_d(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_abs_s(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_abs_d(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_mov_s(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_mov_d(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_neg_s(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_neg_d(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_roundl(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_truncl(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_ceill(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_floorl(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_roundw(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_truncw_s(		OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_truncw_d(		OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_ceilw(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_floorw(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_cvts_d(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_cvts_w(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_cvts_l(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_cvtd_s(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_cvtd_w(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_cvtd_l(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_cvtw_s(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_cvtw_d(			OP_PARAMS); 
extern RETURN_TYPE dyn_r4300i_COP1_cvtw_l(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_cvtl_s(			OP_PARAMS); 
extern RETURN_TYPE dyn_r4300i_COP1_cvtl_d(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_F_S(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_F_D(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_UN_S(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_UN_D(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_EQ_S(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_EQ_D(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_UEQ_S(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_UEQ_D(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_OLT_S(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_OLT_D(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_ULT_S(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_ULT_D(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_OLE_S(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_OLE_D(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_ULE_S(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_ULE_D(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_SF_S(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_SF_D(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_NGLE_S(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_NGLE_D(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_SEQ_S(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_SEQ_D(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_NGL_S(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_NGL_D(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_LT_S(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_LT_D(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_NGE_S(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_NGE_D(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_LE_S(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_LE_D(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_NGT_S(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_C_NGT_D(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_bc1f(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_bc1t(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_bc1fl(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP1_bc1tl(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP0_tlbr(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP0_tlbwi(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP0_tlbwr(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP0_tlbp(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_COP0_eret(			OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_j(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_blez(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_bgtz(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_blezl(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_bgtzl(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_break(				OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_sync(					OP_PARAMS);
extern RETURN_TYPE dyn_r4300i_syscall(				OP_PARAMS);
extern RETURN_TYPE dyn_temp_NOP(					OP_PARAMS);

#endif //_DYNAREC_H__1964_
