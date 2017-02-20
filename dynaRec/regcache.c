#include "../hardware.h"
#include "../options.h"
#include "regcache.h"
#include "x86.h"

#define ItIsARegisterNotToUse(k) ((k == Reg_ECX)|| (k == Reg_ESP)) //Don't use these registers for mapping

#define Num_x86regs 8

#define IT_IS_HIGHWORD      -2
#define IT_IS_UNUSED        -1

/* The following macros are for debugging purposes */
//#define SAFE_32_TO_64         1
//#define SAFE_64_TO_32         1
//#define SAFE_FLUSHING         1

//void FlushRegister(_u32 k);
void FlushRegister(int k);
void WriteBackDirty(_int8 k);
void FlushOneConstant(int k);
void MapOneConstantToRegister(x86regtyp* Conditions);


/*****************************************************************/

x86regtyp x86reg[8];
/******************************************************************
  Function: InitRegisterMap()
  Purpose:  This function will set each entry in the MIPS->x86 map 
            to -1 (unused)
  input:    nothing
  output:   nothing
*******************************************************************/

//extern MapConstant ConstMap[32];
extern FlushedMap  FlushedRegistersMap[32];
void InitRegisterMap()
{
    //_s32 k;
	int k;
    
    ThisYear = 2000;
    for (k=0; k<Num_x86regs; k++)
    {
        x86reg[k].mips_reg  = -1;
        x86reg[k].HiWordLoc = -1;
        x86reg[k].x86reg    = k;
        x86reg[k].Is32bit   = 0;    // If 1, this register uses 32bit

        x86reg[k].IsDirty   = 0;    // If 1, this register will eventually need 
                                    // to be unmapped (written back to memory-- 
                                    // "flushed")
       x86reg[k].BirthDate = ThisYear;
    }

        for (k=0; k<32; k++)
        {
            ConstMap[k].IsMapped = 0;
            ConstMap[k].value    = 0;
        }

        for (k=0; k<32; k++)
        {
            FlushedRegistersMap[k].Is32bit = 0;
        }
}


int CheckIs32Bit(int mips_reg)
{
    _s32 k;
    
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

int CheckIsDirty(int mips_reg)
{
    _s32 k;
    
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

int CheckWhereIsMipsReg(int mips_reg)
{
    _s32 k;
    
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


/******************************************************************
  Function: MapRegister()
  Purpose:  This function is called when you want to map a MIPS 
            General Purpose Register (GPR) to a native x86 register.
  input:    x86regtyp Conditions: The MIPS register to be mapped (with 
                                  its conditions--Is it Dirty? Is it
                                  32bit or 64bit?).
  output:   Conditions->x86reg  : Which x86 register it has been mapped to.
                                  Conditions->x86reg = -1: reg failed to map.
*******************************************************************/
void MapRegister(x86regtyp* Conditions, int The2ndOneNotToFlush, int The3rdOneNotToFlush, int The4thOneNotToFlush)
{
    //_s32 k;
	int k;
    //_s32 i;
	int i;
    int MapSuccess;

    /* If the MIPS register is already mapped, 
    find out which x86 register it is mapped to */

    ThisYear++;

    MapOneConstantToRegister(Conditions);
    for (k=0; k < Num_x86regs; k++)
    {
        if (ItIsARegisterNotToUse(k));

        //if the reg is already mapped
        else if (x86reg[k].mips_reg == Conditions->mips_reg)
        {
            //Converts 64bit reg to 32bit reg:

            // check if the register wants 32bit
            if (Conditions->Is32bit == 1)
            {
                // check if the HiWord is mapped to some other x86 register than where we are now...k.
                if ((x86reg[k].HiWordLoc != k)  && (Conditions->IsDirty == 1))
                {  
                    // we know the register mapped was 64bit.
#ifdef SAFE_64_TO_32                    
                FlushRegister(k);
                goto _Do32bit;
#endif

                if (x86reg[k].HiWordLoc == -1)
                        DisplayError("MapRegister() bug: HiWordLoc should not be -1");

                    //free that HiWord
                    {
                        x86reg[x86reg[k].HiWordLoc].mips_reg = IT_IS_UNUSED;
                        x86reg[x86reg[k].HiWordLoc].Is32bit   = 0;
                        x86reg[x86reg[k].HiWordLoc].IsDirty   = 0;
                        x86reg[x86reg[k].HiWordLoc].HiWordLoc = -1;
                        x86reg[x86reg[k].HiWordLoc].BirthDate = ThisYear;

                        x86reg[k].HiWordLoc = k;
                        x86reg[k].Is32bit   = 1;
                        x86reg[k].BirthDate = ThisYear;
                    }
                }
                else if (x86reg[k].HiWordLoc != k)
                {    
                    Conditions->Is32bit = 0; //means do not convert.
                }
            }

            // Converts 32bit reg to 64bit:

            // we know the register wants 64 bit:
            else
            {
                // check if the HiWord is mapped to the same register where we are at now...k.
                if ((x86reg[k].HiWordLoc == k))
                {
                    int HiMapped = 0;
                    int couldntmap = 0;

#ifdef SAFE_32_TO_64                    
                FlushRegister(k);
                goto _Do64bit;
#endif

                    //find a spot for the highword
                    while (HiMapped == 0)
                    {
                        if (couldntmap++ >= 100)
                        {
                            DisplayError("I can't find a place to map.");
                        }

                        for (i=0; i<8; i++)
                        {
                            if (ItIsARegisterNotToUse(i));
                            else if ((x86reg[i].mips_reg == IT_IS_UNUSED))
                            {
                                HiMapped = 1;

                                x86reg[k].HiWordLoc = i;
                                x86reg[k].Is32bit = 0; //Heheh I forgot this in wip4 ;).
                                
                                x86reg[i].mips_reg  = IT_IS_HIGHWORD;
                                x86reg[i].Is32bit  = 0;
                                x86reg[i].IsDirty  = 0;
                                x86reg[i].HiWordLoc = -1;

                                if ((x86reg[k].IsDirty == 0))
                                {
                                    if (FlushedRegistersMap[x86reg[k].mips_reg].Is32bit == 1)
                                    {
                                        MOV_Reg2ToReg1(1, x86reg[k].HiWordLoc, x86reg[k].x86reg);
                                        XOR_Reg1ToReg2(1, Reg_ECX, Reg_ECX);
                                        DEC_Reg(1, Reg_ECX);
                                        SAR_RegByCL(1, x86reg[k].HiWordLoc);
                                    }
                                    else
                                    {
                                        LoadGPR_HI(k);
                                    }                                    
                                }
                                else if (Conditions->NoNeedToLoadTheHi == 0)
                                {
                                    MOV_Reg2ToReg1(1, (_u8)i, (_u8)k);
                                    XOR_Reg1ToReg2(1, Reg_ECX, Reg_ECX);
                                    DEC_Reg(1, Reg_ECX);
                                    SAR_RegByCL(1, (_u8)i);
                                }
                                FlushedRegistersMap[x86reg[k].mips_reg].Is32bit = 0;
                                i = 99;
                            }
                        }
                        if (HiMapped == 0)
                        {
                            FlushOneButNotThese4(x86reg[k].mips_reg, The2ndOneNotToFlush, The3rdOneNotToFlush, The4thOneNotToFlush);
                        }
                    }
                }
            }

            //set and return the map info
            Conditions->x86reg = k; 
            Conditions->HiWordLoc = x86reg[k].HiWordLoc;
            
            if (Conditions->IsDirty == 1)   x86reg[k].IsDirty = 1;
            x86reg[k].Is32bit = Conditions->Is32bit;
            

            if (x86reg[k].HiWordLoc == -1)          DisplayError("Set&return map info: HiWord is -1!!!");
            if ((k == Reg_ESP))                       DisplayError("Writing Lo to esp..bad!");
            if ((x86reg[k].HiWordLoc == Reg_ESP))     DisplayError("Writing Hi to esp..bad!");

            x86reg[k].BirthDate = ThisYear;
            x86reg[x86reg[k].HiWordLoc].BirthDate = ThisYear;
            return;
        }
    }



    /* If the MIPS register is unmapped, map it to the next
    available x86 register(s) */


    //32bit
    if (Conditions->Is32bit == 1)
//_Do32bit:
    {
        MapSuccess = 0;
        while (MapSuccess == 0)
        {
            for (k=0; k < Num_x86regs; k++)
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

                    if (Conditions->IsDirty == 1)   x86reg[k].IsDirty = 1;
                    Conditions->x86reg  = k;
                    Conditions->HiWordLoc = k; //ok..
                
                    if ((Conditions->NoNeedToLoadTheLo == 0))
                        LoadGPR_LO(k);

                    if (x86reg[k].HiWordLoc == -1) DisplayError("Do32bit: Hiword = -1 Not expected.");

                    x86reg[k].BirthDate = ThisYear;
                    MapSuccess = 1; return;
                }
            }
            if (MapSuccess == 0)
            {
                FlushOneButNotThese4(99, The2ndOneNotToFlush, The3rdOneNotToFlush, The4thOneNotToFlush);
            }
        }
    }

    else

//_Do64bit:
    //64bit
    {
        int TheFirstAvailableReg = 0;
        int NumRegsAvailable     = 0;
        //make sure we have 2 registers available for mapping

        MapSuccess = 0;
        while (MapSuccess == 0)
        {
            TheFirstAvailableReg = 0;
            NumRegsAvailable     = 0;
            for (k=0; k<8; k++)
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
                                XOR_Reg1ToReg2(1, Reg_ECX, Reg_ECX);
                                DEC_Reg(1, Reg_ECX);
                                SAR_RegByCL(1, x86reg[k].HiWordLoc);
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
                FlushOneButNotThese4(99, The2ndOneNotToFlush, The3rdOneNotToFlush, The4thOneNotToFlush);
            }
        }
    }

    /* If there are no registers left to map, x86reg = -1 (failed) */
    DisplayError("Failed to Map!!!");
    Conditions->x86reg  = -1;
}

/******************************************************************/

void MapOneConstantToRegister(x86regtyp* Conditions)
{
    x86regtyp   xRT[1];
    xRT->Is32bit = Conditions->Is32bit;

        if (ConstMap[Conditions->mips_reg].IsMapped == 1)
        {
            ConstMap[Conditions->mips_reg].IsMapped = 0;
            xRT->IsDirty = 1;
            xRT->mips_reg = Conditions->mips_reg;
            xRT->NoNeedToLoadTheLo = 1;
            xRT->NoNeedToLoadTheHi = 1;
            MapRegister(xRT, __dotRD, __dotRS, __dotRT);

            if (ConstMap[Conditions->mips_reg].value == 0)
            {
                XOR_Reg1ToReg2(1, xRT->x86reg, xRT->x86reg);
                if (x86reg[xRT->x86reg].Is32bit == 0)
                    XOR_Reg1ToReg2(1, xRT->HiWordLoc, xRT->HiWordLoc);
            }
            else if ((_u32)ConstMap[Conditions->mips_reg].value < 32)
            {
                WC8(0x6A); // push
                WC8(ConstMap[Conditions->mips_reg].value);
                POP_RegFromStack(xRT->x86reg);
                if (x86reg[xRT->x86reg].Is32bit == 0)
                    XOR_Reg1ToReg2(1, xRT->HiWordLoc, xRT->HiWordLoc);
            }
            else
            {    
                int k = (_s32)ConstMap[Conditions->mips_reg].value;
                MOV_ImmToReg(1, xRT->x86reg, ConstMap[Conditions->mips_reg].value);
                if (x86reg[xRT->x86reg].Is32bit == 0)
                {
                    _asm { sar k, 31 }
                    XOR_Reg1ToReg2(1, xRT->HiWordLoc, xRT->HiWordLoc);
                    if (k != 0)
                        DEC_Reg(1, xRT->HiWordLoc);
                }
            }
        }
}

/******************************************************************/
void FlushOneConstant(int k)
{
    int where;
    if (ConstMap[k].IsMapped == 1)
    {
            ConstMap[k].IsMapped = 0;
            where = CheckWhereIsMipsReg(k);
            if (where > -1)
            {
                x86reg[where].IsDirty = 0;
                FlushRegister(where);
            }

            if (ConstMap[k].value == 0)
            {
                _u32 off = (_u32)&gHardwareState.GPR[0]+(k<<3);
                XOR_Reg1ToReg2(1, Reg_ECX, Reg_ECX);
                MOV_RegToMemory(1, Reg_ECX, ModRM_disp32, off);
                MOV_RegToMemory(1, Reg_ECX, ModRM_disp32, 4+off);
            }
            else
            {
                _u32 off = (_u32)&gHardwareState.GPR[0]+(k<<3);
                _s32 i = (_s32)ConstMap[k].value;

                MOV_ImmToMemory(1, off, i);
                _asm { sar i, 31}
                MOV_ImmToMemory(1, 4+off, i);
            }

            FlushedRegistersMap[k].Is32bit = 1;
    }
}

void FlushConstants()
{
    int k;

    for (k=1; k<32; k++) //start at 1 to ignore r0
    {
        FlushOneConstant(k);
    }
    ConstMap[0].IsMapped = 0;
}

/******************************************************************/

void WriteBackDirty(_int8 k)
{
    //32 bit register
    if ((x86reg[k].HiWordLoc == k))
    {
        if (x86reg[k].Is32bit != 1)
            DisplayError("Bug");
        
        StoreGPR_LO(k);

       // sar ecx, 31
       XOR_Reg1ToReg2(1, Reg_ECX, Reg_ECX);
       DEC_Reg(1, Reg_ECX);
       SAR_RegByCL(1, k);

       StoreGPR_HI(k);

       FlushedRegistersMap[x86reg[k].mips_reg].Is32bit = 0;
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

/******************************************************************/

//void FlushRegister(_u32 k)
void FlushRegister(int k)
{
    //paranoid error check
    if (x86reg[k].HiWordLoc == -1)
        DisplayError("FlushRegister: The HiWord was not set!");

    if (x86reg[k].IsDirty == 1)
    {
        WriteBackDirty(k);
        x86reg[k].IsDirty      = 0;
    }

    //clear its hiword
    if (x86reg[k].HiWordLoc != k)
    {
        x86reg[x86reg[k].HiWordLoc].mips_reg = IT_IS_UNUSED;
        x86reg[x86reg[k].HiWordLoc].Is32bit  = 0;
        x86reg[x86reg[k].HiWordLoc].IsDirty  = 0;
        x86reg[x86reg[k].HiWordLoc].HiWordLoc = -1;
        x86reg[k].BirthDate = ThisYear;
    }

    x86reg[k].mips_reg = IT_IS_UNUSED;
    x86reg[k].HiWordLoc = -1;
    x86reg[k].Is32bit = 0;
    x86reg[k].BirthDate = ThisYear;
}


/******************************************************************
  Function: FlushAllRegisters()
  Purpose:  This function is used when you want to "flush" all the 
            General Purpose Registers (GPR) that are mapped so that
            no registers are mapped. An unmapped array entry = -1.
  input:    -
  output:   -
*******************************************************************/
void FlushAllRegisters()
{
    _int8 k;

    FlushConstants();
    for(k=0; k<Num_x86regs; k++)
    {
        if (ItIsARegisterNotToUse(k));
        else if (x86reg[k].mips_reg > -1)
            FlushRegister(k);
    }
}

void FlushOneButNotThese4(int The1stOneNotToFlush, int The2ndOneNotToFlush, int The3rdOneNotToFlush, int The4thOneNotToFlush)
{
    _u32 paranoid = 0;
    _u32 init = 0;
    _u32 EarliestYear;
    _u32 k;
    _u32 MarkedForDeletion;
    int Mips;
//Flush by least recently used

    for(k=0; k<Num_x86regs; k++)
    {
        if (ItIsARegisterNotToUse(k));
        else if ((x86reg[k].mips_reg > -1) && (x86reg[k].mips_reg != The1stOneNotToFlush) && (x86reg[k].mips_reg != The2ndOneNotToFlush) && (x86reg[k].mips_reg != The3rdOneNotToFlush) && (x86reg[k].mips_reg != The4thOneNotToFlush))
        {
            //paranoid error check
            if (x86reg[k].HiWordLoc == -1)
                DisplayError("FlushOne: The HiWord was not set!");

            if (init == 0)
            {
                init = 1;
                MarkedForDeletion = k;
                EarliestYear = x86reg[k].BirthDate;
            }
            else if (x86reg[k].BirthDate <= EarliestYear)
            {
                // If they have same birth year, a nondirty has 
                // priority over a dirty for flushing :)
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

            Mips = x86reg[k].mips_reg;
            paranoid = 1;
        }
    }

    if (ConstMap[Mips].IsMapped == 1)
    {
        DisplayError("This should not happen");
        FlushOneConstant(Mips); //Unmaps corr. reg also
    }

    FlushRegister(MarkedForDeletion);

    if (paranoid == 0)
        DisplayError("Could not flush a register!!");
}