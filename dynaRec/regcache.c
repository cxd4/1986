/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Emulator for Nintendo 64 console system                              |
 |  Copyright (C) 2001  Joel Middendorf  schibo@emulation64.com                 |
 |                                                                              |
 |  This program is free software; you can redistribute it and/or               |
 |  modify it under the terms of the GNU General Public License                 |
 |  as published by the Free Software Foundation; either version 2              |
 |  of the License, or (at your option) any later version.                      |
 |                                                                              |
 |  This program is distributed in the hope that it will be useful,             |
 |  but WITHOUT ANY WARRANTY; without even the implied warranty of              |
 |  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               |
 |  GNU General Public License for more details.                                |
 |                                                                              |
 |  You should have received a copy of the GNU General Public License           |
 |  along with this program; if not, write to the Free Software                 |
 |  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. |
 |                                                                              |
 |  To contact the author:                                                      |
 |  email      : schibo@emulation64.com                                         |
 |  paper mail :                                                                |
 |______________________________________________________________________________|
*/

//#define SAFE_32_TO_64         1
//#define SAFE_64_TO_32         1
//#define SAFE_FLUSHING         1

#include "../hardware.h"
#include "../options.h"
#include "regcache.h"
#include "x86.h"
#define IT_IS_HIGHWORD      -2
#define IT_IS_UNUSED        -1
#define NUM_X86REGS         8
#define DEBUG_REGCACHE
#define R0_COMPENSATION

void FlushRegister(int k);
void WriteBackDirty(_int8 k);
void FlushOneConstant(int k);
void MapOneConstantToRegister(x86regtyp* Conditions, int The2ndOneNotToFlush, int The3rdOneNotToFlush);
extern FlushedMap  FlushedRegistersMap[NUM_CONSTS];
x86regtyp x86reg[8];

////////////////////////////////////////////////////////////////////////

void InitRegisterMap()
{
	int k;
    const Num_x86regs = NUM_X86REGS;

	ThisYear = 2001;
    for (k=0; k<Num_x86regs; k++)
    {
        x86reg[k].mips_reg  = -1;
        x86reg[k].HiWordLoc = -1;
        x86reg[k].x86reg    = k;
        x86reg[k].BirthDate = ThisYear;
        x86reg[k].Is32bit   = 0;    // If 1, this register uses 32bit

        x86reg[k].IsDirty   = 0;    // If 1, this register will eventually need 
                                    // to be unmapped (written back to memory-- 
                                    // "flushed")

       ConstMap[k].IsMapped = 0;
       ConstMap[k].value    = 0;
       FlushedRegistersMap[k].Is32bit = 0;
    }

    for (k=NUM_X86REGS; k<NUM_CONSTS; k++)
    {
        ConstMap[k].IsMapped = 0;
        ConstMap[k].value    = 0;
        FlushedRegistersMap[k].Is32bit = 0;
    }
#ifndef NO_CONSTS
    ConstMap[0].IsMapped = 1;
#endif
    FlushedRegistersMap[0].Is32bit = 1;

    gMultiPass.UseOnePassOnly = 1; //this will be a user option. (zero = multipass)
#ifdef _DEBUG
    gMultiPass.UseOnePassOnly = 1; //not sure how we'll debug multipass yet. We don't do single-pass
                                   //properly in debug yet.
#endif

    gMultiPass.WriteCode = 1;
    gMultiPass.JumpToPhysAddr = 0;
    gMultiPass.WhichPass = COMPILE_MAP_ONLY;
}

////////////////////////////////////////////////////////////////////////

int CheckIs32Bit(int mips_reg)
{
    _s32 k;
    const Num_x86regs = NUM_X86REGS;    
    for (k=0; k < Num_x86regs; k++)
    {
        if (ItIsARegisterNotToUse(k));
        else if (x86reg[k].mips_reg == mips_reg) // if mapped 
        {
            return(x86reg[k].Is32bit);
        }
    }
    return(0);
}

////////////////////////////////////////////////////////////////////////

int CheckIsDirty(int mips_reg)
{
    _s32 k;
    const Num_x86regs = NUM_X86REGS;

    for (k=0; k < Num_x86regs; k++)
    {
        if (ItIsARegisterNotToUse(k));
        else if (x86reg[k].mips_reg == mips_reg) // if mapped 
        {
            return(x86reg[k].IsDirty);
        }
    }
    return(0);
}

////////////////////////////////////////////////////////////////////////

int CheckWhereIsMipsReg(int mips_reg)
{
    _s32 k;
    const Num_x86regs = NUM_X86REGS;
    
    for (k=0; k < Num_x86regs; k++)
    {
        if (ItIsARegisterNotToUse(k));
        else if (x86reg[k].mips_reg == mips_reg) // if mapped 
        {
            return(k);
        }
    }
    return(-1);
}

////////////////////////////////////////////////////////////////////////

void x86reg_Delete(int k)
{
    x86reg[k].mips_reg = IT_IS_UNUSED;
    x86reg[k].Is32bit   = 0;
    x86reg[k].IsDirty   = 0;
    x86reg[k].HiWordLoc = -1;
    x86reg[k].BirthDate = ThisYear;
}

////////////////////////////////////////////////////////////////////////

void ConvertRegTo32bit(int k)
{
#ifdef SAFE_64_TO_32                    
    FlushRegister(k);
    goto _Do32bit;
#endif

    if (x86reg[k].HiWordLoc == -1)
        DisplayError("MapRegister() bug: HiWordLoc should not be -1");

    x86reg_Delete(x86reg[k].HiWordLoc);

    x86reg[k].HiWordLoc = k;
    x86reg[k].Is32bit   = 1;
    x86reg[k].BirthDate = ThisYear;
}

////////////////////////////////////////////////////////////////////////

void ConvertRegTo64bit(int k, x86regtyp* Conditions, int keep2, int keep3)
{
    int i;

    // check if the HiWord is mapped to the same register where we are at now...k.
    if ((x86reg[k].HiWordLoc == k))
    {
        int HiMapped = 0;
        int couldntmap = 0;

#ifdef SAFE_32_TO_64                    
        FlushRegister(k);
        goto _Do64bit;
#endif
        //find a spot for the HiWord
        while (HiMapped == 0)
        {
            if (couldntmap++ >= 100) DisplayError("I can't find a place to map.");

            for (i=0; i<NUM_X86REGS; i++)
            {
                if (ItIsARegisterNotToUse(i));
                else if ((x86reg[i].mips_reg == IT_IS_UNUSED))
                {
                    HiMapped = 1;

                    x86reg[k].HiWordLoc = i;
                    x86reg[k].Is32bit = 0;

                    x86reg[i].mips_reg  = IT_IS_HIGHWORD;
                    x86reg[i].Is32bit  = 0;
                    x86reg[i].IsDirty  = 0;
                    x86reg[i].HiWordLoc = -1;

                    if ((x86reg[k].IsDirty == 0))
                    {
                        if (FlushedRegistersMap[x86reg[k].mips_reg].Is32bit == 1)
                        {
                            MOV_Reg2ToReg1(1, x86reg[k].HiWordLoc, x86reg[k].x86reg);
                            SAR_RegByImm(1, x86reg[k].HiWordLoc, 31);
                        }
                        else
                        {
                            LoadGPR_HI(k);
                        }                                    
                    }
                    else if (Conditions->NoNeedToLoadTheHi == 0)
                    {
                        MOV_Reg2ToReg1(1, (_u8)i, (_u8)k);
                        SAR_RegByImm(1, (_u8)i, 31);
                    }
                    FlushedRegistersMap[x86reg[k].mips_reg].Is32bit = 0;
                    i = 99;
                }
            }
            if (HiMapped == 0)
            {
                FlushOneButNotThese3(x86reg[k].mips_reg, keep2, keep3);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////

void Map32bit(int k, x86regtyp* Conditions, int keep2, int keep3)
{
    int MapSuccess = 0;
    while (MapSuccess == 0)
    {
        for (k=0; k < NUM_X86REGS; k++)
        {
            if (ItIsARegisterNotToUse(k));
            else if (x86reg[k].mips_reg == IT_IS_UNUSED)
            {
                /* map it */
                x86reg[k].mips_reg  = Conditions->mips_reg;
                x86reg[k].Is32bit   = 1;
                x86reg[k].HiWordLoc = k;

                if (x86reg[k].HiWordLoc == Reg_ESP)
                    DisplayError("2: x86reg[%d]: Write to esp: cannot do that!", k);

                if (Conditions->IsDirty == 1)
                    x86reg[k].IsDirty = 1;

                Conditions->x86reg  = k;
                Conditions->HiWordLoc = k; //ok..

                if ((Conditions->NoNeedToLoadTheLo == 0))
                {
                    LoadGPR_LO(k);
                }

                if (x86reg[k].HiWordLoc == -1) DisplayError("Do32bit: Hiword = -1 Not expected.");

                x86reg[k].BirthDate = ThisYear;
                MapSuccess = 1; return;
            }
        }
        if (MapSuccess == 0)
        {
            FlushOneButNotThese3(99, keep2, keep3);
        }
    }
}

////////////////////////////////////////////////////////////////////////

void Map64bit(int k, x86regtyp* Conditions, int The2ndOneNotToFlush, int The3rdOneNotToFlush)
{
    int TheFirstAvailableReg = 0;
    int NumRegsAvailable     = 0;
    //make sure we have 2 registers available for mapping

    int MapSuccess = 0;
    while (MapSuccess == 0)
    {
        TheFirstAvailableReg = 0;
        NumRegsAvailable     = 0;
        for (k=0; k<NUM_X86REGS; k++)
        {
            if (x86reg[k].HiWordLoc == Reg_ESP)
                DisplayError("4: x86reg[%d]: Write to esp: cannot do that!", k);

            if (ItIsARegisterNotToUse(k));
            else if (x86reg[k].mips_reg == IT_IS_UNUSED )
            {
                NumRegsAvailable += 1;
                if (NumRegsAvailable == 1)
                    TheFirstAvailableReg = k; //This will represent the hiword

                if (NumRegsAvailable == 2)
                {
                    //map lo
                    x86reg[k].mips_reg  = Conditions->mips_reg;
                    x86reg[k].Is32bit   = 0;

                    if (Conditions->IsDirty == 1)   
                        x86reg[k].IsDirty = 1;

                    Conditions->x86reg  = k;

                    if ((Conditions->NoNeedToLoadTheLo == 0) )
                        LoadGPR_LO(k);

                    //map hi
                    x86reg[k].HiWordLoc   = TheFirstAvailableReg;
                    Conditions->HiWordLoc = TheFirstAvailableReg;

                    if (x86reg[k].HiWordLoc == Reg_ESP)
                        DisplayError("3: x86reg[%d]: Write to esp: cannot do that!", k);

                    x86reg[TheFirstAvailableReg].mips_reg = IT_IS_HIGHWORD;

                    if ((Conditions->NoNeedToLoadTheHi == 0))
                    {
                        if (FlushedRegistersMap[x86reg[k].mips_reg].Is32bit == 1)
                        {
                            MOV_Reg2ToReg1(1, x86reg[k].HiWordLoc, x86reg[k].x86reg);
                            SAR_RegByImm(1, x86reg[k].HiWordLoc, 31);
                        }
                        else
                        {
                            LoadGPR_HI(k);
                        }
                    }
                    FlushedRegistersMap[x86reg[k].mips_reg].Is32bit = 0;

                    if (x86reg[k].HiWordLoc == -1) DisplayError("Do64bit: Hiword = -1 Not expected.");
                    
                    x86reg[k].BirthDate = ThisYear;
                    x86reg[TheFirstAvailableReg].BirthDate = ThisYear;
                    MapSuccess = 1; return;
                }
            }        
        }
        if (MapSuccess == 0)
        {
            FlushOneButNotThese3(99, The2ndOneNotToFlush, The3rdOneNotToFlush);
        }
    }
}

///////////////////////////////////////////////////////////////////////

extern uint32 TempPC;
extern uint32 KEEP_RECOMPILING;
extern uint32 cp0Counter;
void SwitchToOpcodePass()
{
    int k;
    const Num_x86regs = NUM_X86REGS;

    DisplayError("Switching to Opcode pass");
    gMultiPass.VirtAddrAfterMap = gHWS_pc;
    gMultiPass.PhysAddrAfterMap = lCodePosition;
    gMultiPass.JumpToPhysAddr = 1;
    gMultiPass.pc = TempPC-4;
    cp0Counter = 0;

    gMultiPass.OpcodePass_Count = 0;
    gMultiPass.WhichPass = COMPILE_OPCODES_ONLY;
}

////////////////////////////////////////////////////////////////////////

void ConvertReg(int k, x86regtyp* Conditions, int The2ndOneNotToFlush, int The3rdOneNotToFlush)
{
    // check if the register wants 32bit
    if (Conditions->Is32bit == 1)
    {
        // check if the HiWord is mapped to some other x86 register than where we are now...k.
        if ((x86reg[k].HiWordLoc != k)  && (Conditions->IsDirty == 1))
        {
            CHECK_OPCODE_PASS;
            ConvertRegTo32bit(k);
        }
        else if (x86reg[k].HiWordLoc != k)
            Conditions->Is32bit = 0; //means do not convert.
    }
    else
    {
        CHECK_OPCODE_PASS;
        ConvertRegTo64bit(k, Conditions, The2ndOneNotToFlush, The3rdOneNotToFlush);
    }

    //set and return the map info
    Conditions->x86reg = k; 
    Conditions->HiWordLoc = x86reg[k].HiWordLoc;
            
    if (Conditions->IsDirty == 1)   x86reg[k].IsDirty = 1;
    x86reg[k].Is32bit = Conditions->Is32bit;
    x86reg[k].BirthDate = ThisYear;
    x86reg[x86reg[k].HiWordLoc].BirthDate = ThisYear;

    if (x86reg[k].HiWordLoc == -1)          DisplayError("Set&return map info: HiWord is -1!!!");
}

///////////////////////////////////////////////////////////////////////

void CheckMultiPass()
{
    if ( (gMultiPass.WhichPass == COMPILE_OPCODES_ONLY) &&
        (gHWS_pc == gMultiPass.VirtAddrAfterMap) )
    {
        gMultiPass.WhichPass = COMPILE_ALL;
    }
}

////////////////////////////////////////////////////////////////////////

void MapRegister(x86regtyp* Conditions, int keep2, int keep3)
{
	int k;
    const Num_x86regs = NUM_X86REGS;

    ThisYear++;

    gMultiPass.WriteCode = ((gMultiPass.WhichPass != COMPILE_OPCODES_ONLY) ? 1 : 0) | gMultiPass.UseOnePassOnly;
   
    //we still haven't optimized consts fully, so do this:
    if (Conditions->NoNeedToLoadTheLo == 0) { MapOneConstantToRegister(Conditions, keep2, keep3); }
    if (Conditions->mips_reg > 0)  ConstMap[Conditions->mips_reg].IsMapped = 0;

    if ((k = CheckWhereIsMipsReg(Conditions->mips_reg)) > -1)
        ConvertReg(k, Conditions, keep2, keep3);

    else if (Conditions->Is32bit == 1)
        Map32bit(k, Conditions, keep2, keep3);
    
    else
        Map64bit(k, Conditions, keep2, keep3);

    if ((k == Reg_ESP))                     DisplayError("Writing Lo to esp..bad!");
    if ((x86reg[k].HiWordLoc == Reg_ESP))   DisplayError("Writing Hi to esp..bad!");

    gMultiPass.WriteCode = ((gMultiPass.WhichPass == COMPILE_OPCODES_ONLY) || (gMultiPass.WhichPass == COMPILE_ALL) ? 1 : 0) | gMultiPass.UseOnePassOnly;
}

////////////////////////////////////////////////////////////////////////

void MapConst(x86regtyp* xMAP, int value)
{
    int where = CheckWhereIsMipsReg(xMAP->mips_reg);
    ConstMap[xMAP->mips_reg].IsMapped = 1;
    ConstMap[xMAP->mips_reg].value    = value;
    if (where > -1) 
    {
        x86reg[where].IsDirty = 0;
        FlushRegister(where);
    }
    ConstMap[0].value = 0;
}

////////////////////////////////////////////////////////////////////////

void MapOneConstantToRegister(x86regtyp* Conditions, int The2ndOneNotToFlush, int The3rdOneNotToFlush)
{
    x86regtyp xRJ[1];

    xRJ->Is32bit = Conditions->Is32bit;

    if (ConstMap[Conditions->mips_reg].IsMapped == 1)
    {
        ConstMap[Conditions->mips_reg].IsMapped = 0;
        if (Conditions->mips_reg != 0)
        {
            xRJ->IsDirty = 1;
            Conditions->IsDirty = 1;
        }
        xRJ->mips_reg = Conditions->mips_reg;
        xRJ->NoNeedToLoadTheLo = 1;
        xRJ->NoNeedToLoadTheHi = 1;
        Conditions->NoNeedToLoadTheLo = 1;
        Conditions->NoNeedToLoadTheHi = 1;
#ifndef NO_CONSTS
        ConstMap[0].IsMapped = 1;
#endif
        MapRegister(xRJ, The2ndOneNotToFlush, The3rdOneNotToFlush);
#ifndef NO_CONSTS        
        ConstMap[0].IsMapped = 1;
#endif
        ConstMap[0].value = 0;

        if (ConstMap[Conditions->mips_reg].value == 0)
        {
            XOR_Reg1ToReg2(1, xRJ->x86reg, xRJ->x86reg);
            if (x86reg[xRJ->x86reg].Is32bit == 0)
                XOR_Reg1ToReg2(1, xRJ->HiWordLoc, xRJ->HiWordLoc);
        }
        else if ((_u32)ConstMap[Conditions->mips_reg].value < 32)
        {
            PUSH_Imm8ToStack((_u8)ConstMap[Conditions->mips_reg].value);
            POP_RegFromStack(xRJ->x86reg);
            if (x86reg[xRJ->x86reg].Is32bit == 0)
                XOR_Reg1ToReg2(1, xRJ->HiWordLoc, xRJ->HiWordLoc);
        }
        else
        {    
            int k = (_s32)ConstMap[Conditions->mips_reg].value;
            MOV_ImmToReg(1, xRJ->x86reg, ConstMap[Conditions->mips_reg].value);
            if (x86reg[xRJ->x86reg].Is32bit == 0)
            {
                _asm { sar k, 31 }
                XOR_Reg1ToReg2(1, xRJ->HiWordLoc, xRJ->HiWordLoc);
                if (k != 0)
                    DEC_Reg(1, xRJ->HiWordLoc);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////

void FlushOneConstant(int k)
{
    int where;

    CHECK_OPCODE_PASS;

    if (ConstMap[k].IsMapped == 1)
    {
        FlushedRegistersMap[k].Is32bit = 1;
        if (k==0) return;
        ConstMap[k].IsMapped = 0;            
        where = CheckWhereIsMipsReg(k);
        if (where > -1)
        {
#ifdef DEBUG_REGCACHE
            DisplayError("Odd");
#endif
            x86reg[where].IsDirty = 0;
            FlushRegister(where);
        }

        if (ConstMap[k].value == 0)
        {
            _u32 off = (_u32)&gHWS_GPR[0]+(k<<3);
            PUSH_RegIfMapped(Reg_EDI);
            XOR_Reg1ToReg2(1, Reg_EDI, Reg_EDI);
            MOV_RegToMemory(1, Reg_EDI, ModRM_disp32, off);
            MOV_RegToMemory(1, Reg_EDI, ModRM_disp32, 4+off);
            POP_RegIfMapped(Reg_EDI);
        }
        else
        {
            _u32 off = (_u32)&gHWS_GPR[0]+(k<<3);
            _s32 i = (_s32)ConstMap[k].value;

            MOV_ImmToMemory(1, off, i);
            _asm { sar i, 31}
            MOV_ImmToMemory(1, 4+off, i);
        }
    }
}

////////////////////////////////////////////////////////////////////////

void FlushConstants()
{
    int k;

    for (k=1; k<NUM_CONSTS; k++)
    {
        FlushOneConstant(k);
    }
#ifndef NO_CONSTS
    ConstMap[0].IsMapped = 1;
#endif
    ConstMap[0].value    = 0;
}

////////////////////////////////////////////////////////////////////////

void WriteBackDirty(_int8 k)
{
    //32 bit register
    if ((x86reg[k].HiWordLoc == k))
    {
        if (x86reg[k].Is32bit != 1)
            DisplayError("Bug");

       StoreGPR_LO(k);
       SAR_RegByImm(1, k, 31);
       StoreGPR_HI(k);

       FlushedRegistersMap[x86reg[k].mips_reg].Is32bit = 1;
    }
    else
    //64 bit register
    {
        if (x86reg[k].Is32bit == 1)
            DisplayError("Bug");

        StoreGPR_LO(k);
        StoreGPR_HI(k);
        FlushedRegistersMap[x86reg[k].mips_reg].Is32bit = 0;
    }
}

////////////////////////////////////////////////////////////////////////

void FlushRegister(int k)
{
#ifdef DEBUG_REGCACHE
    //paranoid error check
    if (x86reg[k].HiWordLoc == -1)
        DisplayError("FlushRegister: The HiWord was not set!");
#endif

#ifdef R0_COMPENSATION
    //Until we don't map all r0's, we'll need this check
    //if (ConstMap[0].IsMapped == 0) DisplayError("How did Const[0] get unmapped???");
    if (x86reg[k].mips_reg == 0) x86reg[k].IsDirty = 0;
#endif

    if (x86reg[k].IsDirty == 1)
    {
        CHECK_OPCODE_PASS
        WriteBackDirty((_s8)k);
    }

    x86reg_Delete(x86reg[k].HiWordLoc);
    x86reg_Delete(k);
}

/*//////////////////////////////////////////////////////////////////////
  Function: FlushAllRegisters()
  Purpose:  This function is used when you want to "flush" all the 
            General Purpose Registers (GPR) that are mapped so that
            no registers are mapped. An unmapped array entry = -1.
  input:    -
  output:   -
//////////////////////////////////////////////////////////////////////*/

void FlushAllRegisters()
{
    _int8 k;
    const Num_x86regs = NUM_X86REGS;

    CHECK_OPCODE_PASS
    
    FlushConstants();
    
    ThisYear = 2001;   
    for (k=1; k<NUM_CONSTS; k++)
        FlushedRegistersMap[k].Is32bit = 0;

    for(k=0; k<Num_x86regs; k++)
    {
        if (ItIsARegisterNotToUse(k));
        else if (x86reg[k].mips_reg > -1)
            FlushRegister(k);
    }
}

////////////////////////////////////////////////////////////////////////

void PushMap() // Alternative to PUSHAD
{
    _int8 k;
    const Num_x86regs = NUM_X86REGS;
    
    for(k=0; k<Num_x86regs; k++)
    {
        if (ItIsARegisterNotToUse(k));
        else if ((x86reg[k].mips_reg > -1) && (x86reg[k].mips_reg != IT_IS_HIGHWORD))
        {
            PUSH_RegToStack(k);
            if (x86reg[k].HiWordLoc != k)
                PUSH_RegToStack(x86reg[k].HiWordLoc);
        }
    }
}

////////////////////////////////////////////////////////////////////////

void PushMapButNot(int x86reg1) // Alternative to PUSHAD
{
    _int8 k;
    const Num_x86regs = NUM_X86REGS;
    
    for(k=0; k<Num_x86regs; k++)
    {
        if (ItIsARegisterNotToUse(k));
        else if ( (x86reg[k].mips_reg > -1) && (x86reg[k].mips_reg != IT_IS_HIGHWORD) 
            && (k != x86reg1) )
        {
            PUSH_RegToStack(k);
            if (x86reg[k].HiWordLoc != k)
                PUSH_RegToStack(x86reg[k].HiWordLoc);
        }
    }
}

////////////////////////////////////////////////////////////////////////

void PopMapButNot(int x86reg1) // Alternative to POPAD
{
    _int8 k;
    const Num_x86regs = NUM_X86REGS;
    
    for(k=Num_x86regs-1; k>=0; k--)
    {
        if (ItIsARegisterNotToUse(k));
        else if ((x86reg[k].mips_reg > -1) && (x86reg[k].mips_reg != IT_IS_HIGHWORD)
                        && (k != x86reg1) )
        {
            if (x86reg[k].HiWordLoc != k)
                POP_RegFromStack(x86reg[k].HiWordLoc);
            POP_RegFromStack(k);
        }
    }
}

////////////////////////////////////////////////////////////////////////

void PUSH_RegIfMapped(int k)
{
    if ((x86reg[k].mips_reg > -1) || (x86reg[k].mips_reg == IT_IS_HIGHWORD))
        PUSH_RegToStack((uint8)k);
}

////////////////////////////////////////////////////////////////////////

void POP_RegIfMapped(int k)
{
    if ((x86reg[k].mips_reg > -1) || (x86reg[k].mips_reg == IT_IS_HIGHWORD))
        POP_RegFromStack((uint8)k);
}

////////////////////////////////////////////////////////////////////////

void PopMap() // Alternative to POPAD
{
    _int8 k;
    const Num_x86regs = NUM_X86REGS;
    
    for(k=Num_x86regs-1; k>=0; k--)
    {
        if (ItIsARegisterNotToUse(k));
        else if ((x86reg[k].mips_reg > -1) && (x86reg[k].mips_reg != IT_IS_HIGHWORD))
        {
            if (x86reg[k].HiWordLoc != k)
                POP_RegFromStack(x86reg[k].HiWordLoc);
            POP_RegFromStack(k);
        }
    }
}

////////////////////////////////////////////////////////////////////////

void FlushOneButNotThese3(int The1stOneNotToFlush, int The2ndOneNotToFlush, int The3rdOneNotToFlush)
{
    const _u8 Num_x86regs = NUM_X86REGS;

    _u32 paranoid = 0;
    _u32 init = 0;
    _u32 EarliestYear;
    _u32 k;
    _u32 MarkedForDeletion;

    CHECK_OPCODE_PASS

//Flush by least recently used

    for(k=0; k<Num_x86regs; k++)
    {
        if (ItIsARegisterNotToUse(k));
        else if ((x86reg[k].mips_reg > -1) && 
            (x86reg[k].mips_reg != The1stOneNotToFlush) && 
            (x86reg[k].mips_reg != The2ndOneNotToFlush) && 
            (x86reg[k].mips_reg != The3rdOneNotToFlush))
        {
#ifdef DEBUG_REGCACHE
            if (x86reg[k].HiWordLoc == -1) DisplayError("FlushOne: The HiWord was not set!");
#endif
            if (init == 0)
            {
                init = 1;
                MarkedForDeletion = k;
                EarliestYear = x86reg[k].BirthDate;
            }
            else if (x86reg[k].mips_reg == 0)
            {
                    MarkedForDeletion = k;
                    EarliestYear = x86reg[k].BirthDate;
                    k=9; //exits
            }
            else if (x86reg[k].BirthDate <= EarliestYear)
            {
                // If they have same birth year, a nondirty has 
                // priority over a dirty for flushing
                if (x86reg[k].BirthDate == EarliestYear)
                {
                    if ((x86reg[MarkedForDeletion].IsDirty == 1) && (x86reg[k].IsDirty == 0))
                        MarkedForDeletion = k;
                }
                else
                {
                    MarkedForDeletion = k;
                    EarliestYear = x86reg[k].BirthDate;
                }
            }
            paranoid = 1;
        }
    }

    if (ConstMap[x86reg[MarkedForDeletion].mips_reg].IsMapped == 1)
    {
#ifdef DEBUG_REGCACHE
        if (x86reg[MarkedForDeletion].mips_reg != 0) DisplayError("This should not happen. Mips = %d", x86reg[MarkedForDeletion].mips_reg);
#endif
        FlushOneConstant(x86reg[MarkedForDeletion].mips_reg); //Unmaps corr. reg also
    }

    //until we don't map all r0's, we'll need this
#ifdef R0_COMPENSATION
    if (x86reg[MarkedForDeletion].mips_reg == 0) x86reg[MarkedForDeletion].IsDirty = 0;
    FlushRegister(MarkedForDeletion);
#endif

    if (paranoid == 0)
        DisplayError("Could not flush a register!!");
}