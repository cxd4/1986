#define uint8  unsigned __int8
#define uint16 unsigned __int16
#define uint32 unsigned __int32
#define uint64 unsigned __int64

//#ifdef _DEBUG
	_int64 Counter;
//#endif

//globals
//The buffer data type is arbitrary. What matters is the 
//data type of the pointer.
uint8* buffer;
uint32* InstructionPointer;
uint32 Instruction;

_int8 Opcode;
uint8 SpecialOp;

uint8 rt_ft;			 //this field is rt or ft
uint8 rd_fs;             //this field is rd or fs
uint8 sa_fd;             //this field is sa or fd
uint8 rs_base_fmt;       //this field is rs or base or format 
_int16 offset_immediate; //this field is offset or immediate
uint32 address;
uint32 pc;               //program counter. (Keeps addresses.)
uint32 MainStartAddr;    //Start Address of Main Code
_int32 instr_index;

uint16 CodeStart;         //Array element location that starts off the code.

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