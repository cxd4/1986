#define uint8  unsigned __int8
#define uint16 unsigned __int16
#define uint32 unsigned __int32
#define uint64 unsigned __int64

//Lets FPU know data size
long* dummy_long; //same as _int32? (we may not need this)
float* dummy_single;
double* dummy_double;
short* dummy_word;


//#ifdef _DEBUG
	_int64 Counter;
//#endif

#ifdef _DEBUG
	unsigned char UserCommand;
#endif

//globals
//The buffer data type is arbitrary. What matters is the 
//data type of the pointer.
uint8* buffer;
uint32* InstructionPointer;
uint32 Instruction;

_int8 Opcode;
uint8 function;          //Special function or float function
uint8 rt_ft;			 //this field is rt or ft
uint8 rd_fs;             //this field is rd or fs
uint8 sa_fd;             //this field is sa or fd
uint8 rs_base_fmt;       //this field is rs or base or format 
_int16 offset_immediate; //this field is offset or immediate
uint8 nd, tf;            //these are 1-bit fields for bc1 ops

uint32 address;
uint32 pc;               //program counter. (Keeps addresses.)
uint32 MainStartAddr;    //Start Address of Main Code
_int32 instr_index;

//SB 16/4/99
uint32	CPUdelayPC;
int		CPUdelay;

uint32 FastLoopAddr; //Debugger variable. Allows user to goto next instruction outside loop when
                     //loop condition is satisfied without the need to view each instruction.

//char* MainCPU[64];
_int64 MainCPUReg[32];

//char* COP0[16];
_int64 COP0Reg[32];
_int64 COP1Reg[32];
_int64 COP2Reg[32];

//Types of instructions
#define MAINCPU_INSTR 0
#define COP1_INSTR    1
#define COP2_INSTR    2
#define COP3_INSTR    3
#define REGIMM_INSTR  4

//Memory
_int64 RDRAM[4194304]; //RAM
_int64 SP_DMEM[4096];  //SP Register
_int64 SP_IMEM[4096];  //SP Register
_int64 VI[1048576];    //100000h Video Interface
_int64 AI[1048576];    //100000h Audio Interface
_int64 PI[1048576];    //100000h Peripheral Interface
_int64 RI[1048576];    //100000h RDRAM Interface
_int64 SI[1048576];    //100000h Serial Interface