/*
  ______________________________________________________________________________
 |                                                                              |
 |  1964 - Emulator for Nintendo 64 console system                              |
 |  Copyright (C) 2001  Joel Middendorf  schibo@emuhq.com                       |
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
 |  email      : schibo@emuhq.com                                               |
 |  paper mail :                                                                |
 |______________________________________________________________________________|

*/

#include <windows.h>
#include "globals.h"
#include "r4300i.h"
#include "hardware.h"
#include "memory.h"
#include <math.h>

#ifdef SAFE_DYNAREC
#define CHK_64BITMODE(Name) if (COP0Reg[STATUS] & 0x04000000) { MessageBox(NULL, "64bit mode FPU detected!", Name, 0); }
#else
#define CHK_64BITMODE(Name)
#endif

void r4300i_COP1_add_s(uint32 Instruction) {    CHK_64BITMODE("add_s") (*((float*)&cFD)) = (*((float  *)&cFS)) + (*((float*)&cFT)); }
void r4300i_COP1_sub_s(uint32 Instruction) {    CHK_64BITMODE("sub_s") (*((float*)&cFD)) = (*((float  *)&cFS)) - (*((float*)&cFT)); }
void r4300i_COP1_mul_s(uint32 Instruction) {    CHK_64BITMODE("mul_s") (*((float*)&cFD)) = (*((float  *)&cFS)) * (*((float*)&cFT)); }
void r4300i_COP1_div_s(uint32 Instruction) {    CHK_64BITMODE("div_s") (*((float*)&cFD)) = (*((float  *)&cFS)) / (*((float*)&cFT)); }

void r4300i_COP1_add_d(uint32 Instruction) {    CHK_64BITMODE("add_d") *((double *)&cFD) =  *((double *)&cFS)  + *((double *)&cFT); }
void r4300i_COP1_sub_d(uint32 Instruction) {    CHK_64BITMODE("sub_d") *((double *)&cFD) =  *((double *)&cFS)  - *((double *)&cFT); }
void r4300i_COP1_mul_d(uint32 Instruction) {    CHK_64BITMODE("mul_d") *((double *)&cFD) =  *((double *)&cFS)  * *((double *)&cFT); }
void r4300i_COP1_div_d(uint32 Instruction) {    CHK_64BITMODE("div_d") *((double *)&cFD) =  *((double *)&cFS)  / *((double *)&cFT); }

//---------------------------------------------------------------------------------------

void r4300i_COP1_abs_s(uint32 Instruction) {    CHK_64BITMODE("abs_s")  *((float  *)&cFD) = (float)fabs((double)*((float  *)&cFS)); }
void r4300i_COP1_sqrt_s(uint32 Instruction){    CHK_64BITMODE("sqrt_s") *((float  *)&cFD) = (float)sqrt((double)*((float  *)&cFS)); }
void r4300i_COP1_neg_s(uint32 Instruction) {    CHK_64BITMODE("neg_s")  *((float  *)&cFD) = (float)   -(        *((float  *)&cFS)); }
void r4300i_COP1_abs_d(uint32 Instruction) {    CHK_64BITMODE("abs_d")  *((double *)&cFD) =        fabs(        *((double *)&cFS)); }
void r4300i_COP1_sqrt_d(uint32 Instruction){    CHK_64BITMODE("sqrt_d") *((double *)&cFD) =        sqrt(        *((double *)&cFS)); }
void r4300i_COP1_neg_d(uint32 Instruction) {    CHK_64BITMODE("neg_d")  *((double *)&cFD) =           -(        *((double *)&cFS)); }

//---------------------------------------------------------------------------------------

void r4300i_COP1_bc1f(uint32 Instruction) 
{ 
	CHK_64BITMODE("bc1f")  
	if((((uint32)cCON31 & 0x00800000)) == 0) 
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
		/*else if( LOAD_UWORD_PARAM(gHardwareState.pc) == 0x5443FFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4) == 0x24420004 )
		{
			gHardwareState.GPR[V0] = gHardwareState.GPR[V1]-4;
		}
		*/
        else 
#endif
		DELAY_SET
	}
             
}

void r4300i_COP1_bc1t(uint32 Instruction) 
{ 
	CHK_64BITMODE("bc1t")  
	if((((uint32)cCON31 & 0x00800000)) != 0) 
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
		/*else if( LOAD_UWORD_PARAM(gHardwareState.pc) == 0x5443FFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4) == 0x24420004 )
		{
			gHardwareState.GPR[V0] = gHardwareState.GPR[V1]-4;
		}
		*/
        else 
#endif
		DELAY_SET
	}
}

void r4300i_COP1_bc1fl(uint32 Instruction)
{ 
	CHK_64BITMODE("bc1fl") 
	if((((uint32)cCON31 & 0x00800000)) == 0) 
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
		/*else if( LOAD_UWORD_PARAM(gHardwareState.pc) == 0x5443FFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4) == 0x24420004 )
		{
			gHardwareState.GPR[V0] = gHardwareState.GPR[V1]-4;
		}
		*/
        else 
#endif
		DELAY_SET
	}
	else    
		DELAY_SKIP  
}

void r4300i_COP1_bc1tl(uint32 Instruction)
{ 
	CHK_64BITMODE("bc1tl") 
	if((((uint32)cCON31 & 0x00800000)) != 0) 
	{
#ifdef DOSPEEDHACK
        if ((uint16)OFFSET_IMMEDIATE == (uint16)0xFFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4)==0) 
        {
            CPUdelayPC = gHardwareState.pc;
            CPUdelay = 1;
			r4300i_speedhack();
        }
		/*else if( LOAD_UWORD_PARAM(gHardwareState.pc) == 0x5443FFFF && LOAD_UWORD_PARAM(gHardwareState.pc+4) == 0x24420004 )
		{
			gHardwareState.GPR[V0] = gHardwareState.GPR[V1]-4;
		}
		*/
        else 
#endif
		DELAY_SET
	}
	else    
		DELAY_SKIP  
}

//---------------------------------------------------------------------------------------

void r4300i_C_EQ_S(uint32 Instruction) {    CHK_64BITMODE("C_EQ_S")  if( *((float  *)&cFS) == *((float  *)&cFT))        cCON31 |=  0x00800000;
                        else                                                                    cCON31 &=  0xFF7FFFFF;}
void r4300i_C_UEQ_S(uint32 Instruction){    CHK_64BITMODE("C_UEQ_S") if( *((float  *)&cFS) == *((float  *)&cFT))        cCON31 |=  0x00800000;
                        else                                                                    cCON31 &=  0xFF7FFFFF;}
void r4300i_C_EQ_D(uint32 Instruction) {    CHK_64BITMODE("C_EQ_D")  if( *((double *)&cFS) == *((double *)&cFT))        cCON31 |=  0x00800000;
                        else                                                                    cCON31 &=  0xFF7FFFFF;}
void r4300i_C_UEQ_D(uint32 Instruction){    CHK_64BITMODE("C_UEQ_D") if( *((double *)&cFS) == *((double *)&cFT))        cCON31 |=  0x00800000;
                        else                                                                    cCON31 &=  0xFF7FFFFF;}
void r4300i_C_LT_S(uint32 Instruction) {    CHK_64BITMODE("C_LT_S")  if( *((float  *)&cFS) <  *((float  *)&cFT))        cCON31 |=  0x00800000;
                        else                                                                    cCON31 &=  0xFF7FFFFF;}
void r4300i_C_NGE_S(uint32 Instruction){    CHK_64BITMODE("C_NGE_S") if( *((float  *)&cFS) <  *((float  *)&cFT))        cCON31 |=  0x00800000;
                        else                                                                    cCON31 &=  0xFF7FFFFF;}
void r4300i_C_LT_D(uint32 Instruction) {    CHK_64BITMODE("C_LT_D")  if( *((double *)&cFS) <  *((double *)&cFT))        cCON31 |=  0x00800000;
                        else                                                                    cCON31 &=  0xFF7FFFFF;}
void r4300i_C_NGE_D(uint32 Instruction){    CHK_64BITMODE("C_NGE_D") if( *((double *)&cFS) <  *((double *)&cFT))        cCON31 |=  0x00800000;
                        else                                                                    cCON31 &=  0xFF7FFFFF;}
void r4300i_C_LE_S(uint32 Instruction) {    CHK_64BITMODE("C_LE_S")  if( *((float  *)&cFS) <= *((float  *)&cFT))        cCON31 |=  0x00800000;
                        else                                                                    cCON31 &=  0xFF7FFFFF;}
void r4300i_C_NGT_S(uint32 Instruction){    CHK_64BITMODE("C_NGT_S") if( *((float  *)&cFS) <= *((float  *)&cFT))        cCON31 |=  0x00800000;
                        else                                                                    cCON31 &=  0xFF7FFFFF;}
void r4300i_C_LE_D(uint32 Instruction) {    CHK_64BITMODE("C_LE_D")  if( *((double *)&cFS) <= *((double *)&cFT))        cCON31 |=  0x00800000;
                        else                                                                    cCON31 &=  0xFF7FFFFF;}
void r4300i_C_NGT_D(uint32 Instruction){    CHK_64BITMODE("C_NGT_D") if( *((double *)&cFS) <= *((double *)&cFT))        cCON31 |=  0x00800000;
                        else                                                                    cCON31 &=  0xFF7FFFFF;}

//---------------------------------------------------------------------------------------

//we won't need to confirm mode for control registers...
void r4300i_COP1_cfc1(uint32 Instruction) { gRT    = (_int64)(_int32)cCONFS;}
void r4300i_COP1_ctc1(uint32 Instruction) { cCONFS = (uint32)gRT;}

//---------------------------------------------------------------------------------------

void r4300i_COP1_cvtd_s(uint32 Instruction) {   CHK_64BITMODE("cvtd_s") *((double *)&cFD) = (double)(*((float  *)&cFS));}
void r4300i_COP1_cvtd_w(uint32 Instruction) {   CHK_64BITMODE("cvtd_w") *((double *)&cFD) = (double)(*((_int32 *)&cFS));}
void r4300i_COP1_cvtd_l(uint32 Instruction) {   CHK_64BITMODE("cvtd_l") *((double *)&cFD) = (double)(*((_int64 *)&cFS));}
void r4300i_COP1_cvts_d(uint32 Instruction) {   CHK_64BITMODE("cvtd_d") *((float  *)&cFD) =  (float)(*((double *)&cFS));}
void r4300i_COP1_cvts_w(uint32 Instruction) {   CHK_64BITMODE("cvts_w") *((float  *)&cFD) =  (float)(*((_int32 *)&cFS));}
void r4300i_COP1_cvts_l(uint32 Instruction) {   CHK_64BITMODE("cvts_l") *((float  *)&cFD) =  (float)(*((_int64 *)&cFS));}
//void r4300i_COP1_cvtw_s(uint32 Instruction) {	CHK_64BITMODE("cvtw_s") *((uint32 *)&cFD) = (uint32)(*((float  *)&cFS));}
void r4300i_COP1_cvtw_s(uint32 Instruction) {	CHK_64BITMODE("cvtw_s") *((__int32 *)&cFD) = (__int32)(*((float  *)&cFS));}
//void r4300i_COP1_cvtw_d(uint32 Instruction) {	CHK_64BITMODE("cvtw_d") *((uint32 *)&cFD) = (uint32)(*((double *)&cFS));}
void r4300i_COP1_cvtw_d(uint32 Instruction) {	CHK_64BITMODE("cvtw_d") *((__int32 *)&cFD) = (__int32)(*((double *)&cFS));}
//void r4300i_COP1_cvtl_s(uint32 Instruction) {   DisplayError("Unhandled Opcode CVT.L.S");       }
void r4300i_COP1_cvtl_s(uint32 Instruction) {   CHK_64BITMODE("cvtl_s") *((__int64 *)&cFD) = (__int64)(*((float *)&cFS));}
//void r4300i_COP1_cvtl_d(uint32 Instruction) {   DisplayError("Unhandled Opcode CVT.L.D");       }
void r4300i_COP1_cvtl_d(uint32 Instruction) {   CHK_64BITMODE("cvtl_d") *((__int64 *)&cFD) = (__int64)(*((double *)&cFS));}


//---------------------------------------------------------------------------------------

void r4300i_COP1_mtc1(uint32 Instruction) { CHK_64BITMODE("mtc1")              cFS  = (uint32)gRT;}
void r4300i_COP1_dmtc1(uint32 Instruction){ CHK_64BITMODE("dmtc1") *((_int64*)&cFS) =         gRT;}

void r4300i_COP1_mfc1(uint32 Instruction) { CHK_64BITMODE("mfc1")   (*(_int64*)&gRT) =     (_int32)cFS; }
void r4300i_COP1_dmfc1(uint32 Instruction){ CHK_64BITMODE("dmfc1")  *((double*)&gRT) = *((double*)&cFS);}

void r4300i_COP1_mov_s(uint32 Instruction){ CHK_64BITMODE("mov_s")               cFD  =             cFS;}
void r4300i_COP1_mov_d(uint32 Instruction){ CHK_64BITMODE("mov_d")  *((double *)&cFD) = *((double*)&cFS);}

//-------------------------------------------------------------------------------------------------------

void r4300i_lwc1(uint32 Instruction)        
{   
    LOAD_TLB_FUN
    __try{
        cFT = *(uint32*)(valloc+((QuerAddr)&0x1fffffff));
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
        __try{
            cFT = *(uint32*)(valloc2+((QuerAddr)&0x1fffffff));
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("%08X: LWC1 out of range", gHardwareState.pc);
        }
   }
    CHK_64BITMODE("lwc1")
}

//-------------------------------------------------------------------------------------------------------

void r4300i_swc1(uint32 Instruction){   
    
    //LOAD_TLB_FUN
	STORE_TLB_FUN
    __try {
        *(uint32*)(valloc+((QuerAddr)&0x1fffffff)) =  (uint32)cFT;
    }__except(NULL,EXCEPTION_EXECUTE_HANDLER){ 
        __try {
            *(uint32*)(valloc2+((QuerAddr)&0x1fffffff)) =  (uint32)cFT;
        }__except(NULL,EXCEPTION_EXECUTE_HANDLER){
            DisplayError("%08X: SWC1 out of range", gHardwareState.pc);
        }
    }
    CHK_64BITMODE("swc1")
}

//-------------------------------------------------------------------------------------------------------

void r4300i_ldc1(uint32 Instruction)
{   
    uint32 UWORD[2];

    LOAD_TLB_FUN

    *(uint64*)UWORD = ((uint64)*((uint64*)sDWORD_R[((QuerAddr) >> 16)]+(((uint16)QuerAddr) >> 3)));
                         cFT        = UWORD[1];
    *(uint32*)(&(uint32*)cFT+1) = UWORD[0];
    CHK_64BITMODE("ldc1")

}
//---------------------------------------------------------------------------------------
//This sdc1 is for 32bit mode only!!
void r4300i_sdc1(uint32 Instruction)    {   
    uint32 QuerAddr;
    uint64 *TempStore;

    QUER_ADDR;

    CHK_64BITMODE("sdc1")
    if ((QuerAddr & 0xC0000000) != 0x80000000)
    {
        //QuerAddr = TranslateTLBAddress(QuerAddr);
		QuerAddr = TranslateTLBAddressForStore(QuerAddr);
    }

    TempStore = ((uint64*)sDWORD_R[((QuerAddr) >> 16)] + (((uint16)QuerAddr) >> 3 ));
    *(uint32*)(&(uint32*)TempStore[0]+1)  = cFT;
    *(uint32*)(&(uint32*)TempStore[0])    = *(uint32*)(&(uint32*)cFT+1);
}

//---------------------------------------------------------------------------------------
/*
 Format:  TRUNC.W.S fd, fs
 Purpose: To convert an FP value to 32-bit fixed-point, rounding toward zero.
*/
void r4300i_COP1_truncw_s(uint32 Instruction) 
{
	float tempf = *((float  *)&cFS);
	CHK_64BITMODE("truncw_s")

	
	if( tempf >= 0 )
		*((__int32 *)&cFD) = (__int32)(tempf);
	else
		*((__int32 *)&cFD) = -((__int32)(-tempf));
	

	//*((uint32 *)&cFD) = (uint32)(*((float  *)&cFS));   //I do not see a reason why this 
	//													 operation is done in unsigned
}

/*
 Format:  TRUNC.W.D fd, fs
 Purpose: To convert an FP value to 32-bit fixed-point, rounding toward zero.
*/
void r4300i_COP1_truncw_d(uint32 Instruction) 
{
	double tempd = *((double *)&cFS);

	CHK_64BITMODE("truncw_d")

	if( tempd >= 0 )
		*((__int32 *)&cFD) = (__int32)(tempd);
	else
		*((__int32 *)&cFD) = -((__int32)(-tempd));;   
}

/*
 Format:  TRUNC.L.S fd, fs
 Purpose: To convert an FP float value to 64-bit fixed-point, rounding toward zero.
*/
void r4300i_COP1_truncl_s(uint32 Instruction) 
{   
	//DisplayError("Unhandled Opcode TRUNC.L.d");   
	float tempf = *((float  *)&cFS);
	CHK_64BITMODE("truncl_s")

	if( tempf >= 0 )
		*((__int64 *)&cFD) = (__int64)(tempf);
	else
		*((__int64 *)&cFD) = -((__int64)(-tempf));
}
/*
 Format:  TRUNC.L.D fd, fs
 Purpose: To convert an FP double float value to 64-bit fixed-point, rounding toward zero.
*/

void r4300i_COP1_truncl_d(uint32 Instruction) 
{   
	//DisplayError("Unhandled Opcode TRUNC.L.d");   
	double tempf = *((double  *)&cFS);
	CHK_64BITMODE("truncl_d")

	if( tempf >= 0 )
		*((__int64 *)&cFD) = (__int64)(tempf);
	else
		*((__int64 *)&cFD) = -((__int64)(-tempf));
}

//---------------------------------------------------------------------------------------

/*
 Format:  FLOOR.L.S fd, fs
 Purpose: To convert an FP float value to 64-bit fixed-point, rounding down.
*/
void r4300i_COP1_floorl_s(uint32 Instruction) 
{
	//DisplayError("Unhandled Opcode FLOOR.L.fmt");   
	CHK_64BITMODE("floorl_S");
	*((__int64 *)&cFD) = (__int64)(*((float  *)&cFS));
}

/*
 Format:  FLOOR.L.D fd, fs
 Purpose: To convert an FP double float value to 64-bit fixed-point, rounding down.
*/
void r4300i_COP1_floorl_d(uint32 Instruction) 
{   
	//DisplayError("Unhandled Opcode FLOOR.L.fmt");   
	CHK_64BITMODE("floorl_d");
	*((__int64 *)&cFD) = (__int64)(*((double  *)&cFS));
}


/*
 Format:  FLOOR.W.S fd, fs
 Purpose: To convert an FP float value to 32-bit fixed-point, rounding down.
*/
void r4300i_COP1_floorw_s(uint32 Instruction) 
{   
	//DisplayError("Unhandled Opcode FLOOR.W.fmt");   
	CHK_64BITMODE("floorw_S");
	*((__int32 *)&cFD) = (__int32)(*((float  *)&cFS));
}

/*
 Format:  FLOOR.W.D fd, fs
 Purpose: To convert an FP doule float value to 32-bit fixed-point, rounding down.
*/

void r4300i_COP1_floorw_d(uint32 Instruction) 
{   
	//DisplayError("Unhandled Opcode FLOOR.W.fmt");   
	CHK_64BITMODE("floorw_d");
	*((__int32 *)&cFD) = (__int32)(*((double  *)&cFS));
}




//---------------------------------------------------------------------------------------

/*
 Format:  ROUND.L.S fd, fs
 Purpose: To convert an FP float value to 64-bit fixed-point, rounding to nearest.
*/
void r4300i_COP1_roundl_s(uint32 Instruction) 
{
	//DisplayError("Unhandled Opcode ROUND.L.S");   
	CHK_64BITMODE("roundw_s");
	*((__int64 *)&cFD) = (__int64)(*((float  *)&cFS)+0.5);
}

/*
 Format:  ROUND.L.D fd, fs
 Purpose: To convert an FP double float value to 64-bit fixed-point, rounding to nearest.
*/

void r4300i_COP1_roundl_d(uint32 Instruction) 
{   
	//DisplayError("Unhandled Opcode ROUND.L.D");   
	CHK_64BITMODE("roundw_s");
	*((__int64 *)&cFD) = (__int64)(*((double  *)&cFS)+0.5);
}

/*
 Format:  ROUND.W.S fd, fs
 Purpose: To convert an FP float value to 32-bit fixed-point, rounding to nearest.
*/
void r4300i_COP1_roundw_s(uint32 Instruction) 
{
	//DisplayError("Unhandled Opcode ROUND.W.fmt");
	CHK_64BITMODE("roundw_s");
	*((__int32 *)&cFD) = (__int32)(*((float  *)&cFS)+0.5);
}

/*
 Format:  ROUND.W.D fd, fs
 Purpose: To convert an FP double float value to 32-bit fixed-point, rounding to nearest.
*/
void r4300i_COP1_roundw_d(uint32 Instruction) 
{
	//DisplayError("Unhandled Opcode ROUND.W.fmt");  
	CHK_64BITMODE("roundw_d");
	*((__int32 *)&cFD) = (__int32)(*((double *)&cFS)+0.5); 
}


//---------------------------------------------------------------------------------------

/*
 Format:  CEIL.W.S fd, fs
 Purpose: To convert an FP single float value to 32-bit fixed-point, rounding up.
*/
void r4300i_COP1_ceilw_s(uint32 Instruction)
{   
	//DisplayError("Unhandled Opcode CEIL.W.S");    
	CHK_64BITMODE("ceilw_S");
	*((__int32 *)&cFD) = (__int32)ceil(((double)(*((float  *)&cFS))));
}

/*
 Format:  CEIL.W.D fd, fs
 Purpose: To convert an FP double float value to 32-bit fixed-point, rounding up.
*/
void r4300i_COP1_ceilw_d(uint32 Instruction)  
{   
	DisplayError("Unhandled Opcode CEIL.W.D");    
	*((__int32 *)&cFD) = (__int32)ceil(*((double  *)&cFS));
}

/*
 Format:  CEIL.L.S fd, fs
 Purpose: To convert an FP single float value to 64-bit fixed-point, rounding up.
*/
void r4300i_COP1_ceill_s(uint32 Instruction)
{   
	//DisplayError("Unhandled Opcode CEIL.L.S");    
	CHK_64BITMODE("ceill_S");
	*((__int64 *)&cFD) = (__int64)ceil(((double)(*((float  *)&cFS))));
}

/*
 Format:  CEIL.L.D fd, fs
 Purpose: To convert an FP double float value to 64-bit fixed-point, rounding up.
*/
void r4300i_COP1_ceill_d(uint32 Instruction)  
{   
	//DisplayError("Unhandled Opcode CEIL.L.D");    
	CHK_64BITMODE("ceill_S");
	*((__int64 *)&cFD) = (__int64)ceil(*((double  *)&cFS));
}

//---------------------------------------------------------------------------------------

void r4300i_C_UN_S(uint32 Instruction)      {   DisplayError("Unhandled Opcode C.UN.S");        }
void r4300i_C_UN_D(uint32 Instruction)      {   DisplayError("Unhandled Opcode C.UN.D");        }
void r4300i_C_OLT_S(uint32 Instruction)     {   DisplayError("Unhandled opcode C.OLT.S");       }
void r4300i_C_OLT_D(uint32 Instruction)     {   DisplayError("Unhandled opcode C.OLT.D");       }
void r4300i_C_ULT_S(uint32 Instruction)     {   DisplayError("Unhandled opcode C.ULT.S");       }
void r4300i_C_ULT_D(uint32 Instruction)     {   DisplayError("Unhandled opcode C.ULT.D");       }
void r4300i_C_OLE_S(uint32 Instruction)     {   DisplayError("Unhandled opcode C.OLE.S");       }
void r4300i_C_OLE_D(uint32 Instruction)     {   DisplayError("Unhandled opcode C.OLE.D");       }
void r4300i_C_ULE_S(uint32 Instruction)     {   DisplayError("Unhandled opcode C.ULE.S");       }
void r4300i_C_ULE_D(uint32 Instruction)     {   DisplayError("Unhandled opcode C.ULE.D");       }
void r4300i_C_SF_S(uint32 Instruction)      {   DisplayError("Unhandled opcode C.SF.S");        }
void r4300i_C_SF_D(uint32 Instruction)      {   DisplayError("Unhandled opcode C.SF.D");        }
void r4300i_C_NGLE_S(uint32 Instruction)    {   DisplayError("Unhandled opcode C.NGLE.S");      }
void r4300i_C_NGLE_D(uint32 Instruction)    {   DisplayError("Unhandled opcode C.NGLE.D");      }
void r4300i_C_SEQ_S(uint32 Instruction)     {   DisplayError("Unhandled opcode C.SEQ.S");       }
void r4300i_C_SEQ_D(uint32 Instruction)     {   DisplayError("Unhandled opcode C.SEQ.D");       }
void r4300i_C_NGL_S(uint32 Instruction)     {   DisplayError("Unhandled opcode C.NGL.S");       }
void r4300i_C_NGL_D(uint32 Instruction)     {   DisplayError("Unhandled opcode C.NGL.D");       }
void r4300i_C_F_S(uint32 Instruction)       {   DisplayError("Unhandled opcode C.F.S");         }
void r4300i_C_F_D(uint32 Instruction)       {   DisplayError("Unhnadled Opcode C.F.D");         }