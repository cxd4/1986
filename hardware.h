#ifndef _HARDWARE_H
#define _HARDWARE_H

#include <windows.h>
#include <process.h>
#include "dllsrc/controller.h"

#include "globals.h"		// loads the rom and handle endian stuff
#include "options.h"

//////////////////////////////////////////////////////////////////////
// Data Types
//////////////////////////////////////////////////////////////////////

// unsigned types
#define _u64   unsigned __int64   /* 64 bit */
#define _u32   unsigned __int32   /* 32 bit */
#define _u16   unsigned __int16   /* 16 bit */
#define _u8	   unsigned __int8    /*  8 bit */

// signed types
#define _s64   __int64   /* 64 bit */
#define _s32   __int32   /* 32 bit */
#define _s16   __int16   /* 16 bit */
#define _s8	   __int8    /*  8 bit */

#define __LO 32
#define __HI 33


//////////////////////////////////////////////////////////////////////
// Emulation Data
//////////////////////////////////////////////////////////////////////
#define CARTRIDGE_AUTODETECT	0
#define CARTRIDGE_4KBIT_EEP		1
#define CARTRIDGE_16KBIT_EEP	2
#define CARTRIDGE_SRAM			3
#define CARTRIDGE_FLASHRAM		4 

typedef struct
{
	char video_plugin[128];
	char audio_plugin[128];
	char input_plugin[128];
	char rom_name[1024];
	int  CartridgeType;

	HINSTANCE hInstance;
	HWND hWnd;

	HINSTANCE DLL_hInstance;
} t_EmuData;

//////////////////////////////////////////////////////////////////////
// includes for all Hardware parts
//////////////////////////////////////////////////////////////////////

#define RDRAM_SIZE    0x800000
#define RDRAM_MASK    0x7FFFFF

// FlashRam
#define FLASHRAM_SIZE 0x20000
#define FLASHRAM_MASK 0x1FFFF
extern _u8 FlashRAM[FLASHRAM_SIZE];

// SRam
#define SRAM_SIZE 0x8000
#define SRAM_MASK 0x7FFF
extern _u8 SRam[SRAM_SIZE];  //SRam = 256 kbit (32kbyte)

// EEprom
#define EEPROM_SIZE 0x800	// EEprom Sie = 16 kbit (2kbyte) - old EEprom 512 bytes
#define EEPROM_SIZE_4KB	0x1000
#define EEPROM_MASK 0x7FF	
//extern _u8 EEprom[EEPROM_SIZE];
extern _u8 EEprom[EEPROM_SIZE_4KB];

// Generell ...
extern t_EmuData		EmuData;
extern CONTROL			Controls[4];
extern t_rominfo		rominfo;

typedef struct sHardwareState {
    _int64 GPR[34];         /* General Purpose Registers    GPR[32] = lo, GPR[33] = hi */
    uint32 COP0Reg[32];     /* Coprocessor0 Registers       */
    uint32 fpr32[64];       /* 32bit 64 items needed!		*/
    uint32 LLbit;           /* LoadLinked Bit               */
    uint32 pc;              /* program counter              */
    uint32 COP1Con[32];     /* FPControl Registers, only 0 and 31 is used   */
    uint32 COP0Con[64];     /* FPControl Registers          */
    uint32 RememberFprHi[32];
    uint32 code;            /* The instruction              */
} HardwareState;

extern HardwareState gHardwareState;
extern HardwareState* p_gHardwareState;

#define HARDWARESTATE_SIZE			(sizeof(HardwareState))


#define MAXTLB    32

typedef struct
{
    uint32  valid;
    uint32  EntryHi;
    uint32  EntryLo1;
    uint32  EntryLo0;
    uint64  PageMask;
    uint32  LoCompare;
    uint32  MyHiMask;
} tlb_struct;

typedef struct sMemorySTATE
{
    //memory
//    uint32* RDREG;
    uint32*	ramRegs0;
    uint32*	ramRegs4;
    uint32*	ramRegs8;
    uint32* SP_MEM;
    uint32* SP_REG_1;
    uint32* SP_REG_2;
    uint32* DPC;
    uint32* DPS;
    uint32* MI;
    uint32* VI;
    uint32* AI;
    uint32* PI;
    uint32* RI;
    uint32* SI;
    uint8* RDRAM;		// Size = 4MB
    uint32* C2A1;
    uint32* C1A1;
    uint32* C1A3;
    uint32* C2A2;
    uint8* ROM_Image;
    uint32* GIO_REG;
    uint8*  PIF;
    uint8* ExRDRAM;		// Size = 4MB
    uint8* dummySegment; //handles crap pointers for now..band-aid'ish

    tlb_struct      TLB[MAXTLB];

} MemoryState;

uint8* RDRAM_Copy;

extern MemoryState gMemoryState;
extern MemoryState * p_gMemoryState;
extern MemoryState gVirtMemory;

#ifdef ENABLE_OPCODE_DEBUGGER
extern HardwareState gHardwareState_Interpreter_Compare;
extern HardwareState gHardwareState_Flushed_Dynarec_Compare;
extern MemoryState   gMemoryState_Interpreter_Compare;
#endif



#define gLO 32
#define gHI 33

#define OP_PARAMS   HardwareState* reg
#define PASS_PARAMS reg

typedef void (*dyn_cpu_instr)(OP_PARAMS);

//////////////////////////////////////////////////////////////////////
// New macros
//////////////////////////////////////////////////////////////////////
#define DLL_Func(x) if (x!=NULL) x


#ifndef _USE_DEBUGGER_
_u32 debugger_pc;
#else
extern _u32 debugger_pc;
#endif

_s32 exception_slot;
_u32 next_interrupt_count;
_u32 next_vi_interrupt;
_u32 mem_todo;
_u32 counter;

_u32 *p_dyna_CodeTable;
_u8 *dyna_CodeTable;     // entspricht der Memory_Size ... ein 0xFFFFFFFF eintrag heist
                                // das fuer den Code noch nix precompilt wurde
_u8 *dyna_RecompCode;           // Buffer fuer unseren recompilierten Code
BOOL COP1_exception_speedup;
BOOL DynaBufferOverError;







#endif _HARDWARE_H