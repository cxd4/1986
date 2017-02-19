/*______________________________________________________________________________
 |                                                                              |
 |  1964 - Emulator for Nintendo 64 console system                              |
 |  Copyright (C) 2000  Joel Middendorf  schibo@emuhq.com                       |
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

// The project is a direct documentation-to-code translation of the n64toolkit 
// by my friend anarko and RSP info has been provided by zilmar :). Most source
// code comments are taken directly from anarko's n64toolkit with consent and are 
// the property of anarko.

// 9/15/1999 - GERRIT - rewrote opcode debug printing so that it's no longer
// dependant on opcode execution. Makes things easier for dynarec, etc...

#ifdef DEBUG_COMMON

#include <stdio.h>
#include <stdarg.h>
#include "r4300i.h"
#include "globals.h"
#include "interrupt.h"
#include "DbgPrint.h"
#include "controller.h"

void RefreshDebugger();

//---------------------------------------------------------------------------------------

char	op_str[0xFF];

char*	r4300i_RegNames[32] = {
	"r0",	"at",	"v0",	"v1",	"a0",	"a1",	"a2",	"a3",
	"t0",	"t1",	"t2",	"t3",	"t4",	"t5",	"t6",	"t7",
	"s0",	"s1",	"s2",	"s3",	"s4",	"s5",	"s6",	"s7",
	"t8",	"t9",	"k0",	"k1",	"gp",	"sp",	"s8",	"ra"
};

char*	r4300i_COP0_RegNames[32] = {
	"Index",	"Random",	"EntryLo0",	"EntryLo1",	"Context",	"PageMask",	"Wired",	"Reserved",
	"BadVaddr",	"Count",	"EntryHi",	"Compare",	"Status",	"Cause",	"EPC",		"PRevID",
	"Config",	"LLAddr",	"WatchLo",	"WatchHi",	"XContext",	"Reserved",	"Reserved",	"Reserved",
	"Reserved",	"Reserved",	"PErr",		"CacheErr",	"TagLo",	"TagHi",	"ErrorEPC",	"Reserved"

};

char*	r4300i_COP1_RegNames[32] = {
	"fp1",	"fp2",	"fp3",	"fp4",	"fp5",	"fp6",	"fp7",	"fp8",
	"fp9",	"fp10",	"fp11",	"fp12",	"fp13",	"fp14",	"fp15",	"fp16",
	"fp17",	"fp18",	"fp19",	"fp20",	"fp21",	"fp22",	"fp23",	"fp24",
	"fp25",	"fp26",	"fp27",	"fp28",	"fp29",	"fp30",	"fp31",	"fp32"
};

//---------------------------------------------------------------------------------------
// opcode debug print functions

void debug_r4300i_unknown(uint32 Instruction) { DBGPRINT_OPCODE("[UNKNOWN OPCODE]") };

void debug_r4300i_add(uint32 Instruction)			{         DBGPRINT_RS_RT_RD("ADD     ") };
void debug_r4300i_addi(uint32 Instruction)			{        DBGPRINT_RS_RT_IMM("ADDI    ") };
void debug_r4300i_addiu(uint32 Instruction)			{        DBGPRINT_RS_RT_IMM("ADDIU   ") };
void debug_r4300i_addu(uint32 Instruction)			{        DBGPRINT_RS_RT_RD("ADDU     ") };
void debug_r4300i_and(uint32 Instruction)			{        DBGPRINT_RS_RT_RD("AND      ") };
void debug_r4300i_andi(uint32 Instruction)			{       DBGPRINT_RS_RT_IMM("ANDI     ") };
void debug_r4300i_beq(uint32 Instruction)			{ DBGPRINT_RS_RT_OFF_BRANCH("BEQ     ") };
void debug_r4300i_beql(uint32 Instruction)			{ DBGPRINT_RS_RT_OFF_BRANCH("BEQL    ") };
void debug_r4300i_bgtz(uint32 Instruction)			{ DBGPRINT_RS_RT_OFF_BRANCH("BGTZ    ") };
void debug_r4300i_bgtzl(uint32 Instruction)			{ DBGPRINT_RS_RT_OFF_BRANCH("BGTZL   ") };
void debug_r4300i_blez(uint32 Instruction)			{ DBGPRINT_RS_RT_OFF_BRANCH("BLEZ    ") };
void debug_r4300i_blezl(uint32 Instruction)			{ DBGPRINT_RS_RT_OFF_BRANCH("BLEZL   ") };
void debug_r4300i_bne(uint32 Instruction)			{ DBGPRINT_RS_RT_OFF_BRANCH("BNE     ") };
void debug_r4300i_bnel(uint32 Instruction)			{ DBGPRINT_RS_RT_OFF_BRANCH("BNEL    ") };
void debug_r4300i_break(uint32 Instruction)			{           DBGPRINT_OPCODE("BREAK   ") };
void debug_r4300i_cache(uint32 Instruction)			{           DBGPRINT_OPCODE("CACHE   ") };
void debug_r4300i_dadd(uint32 Instruction)			{         DBGPRINT_RS_RT_RD("DADD    ") };
void debug_r4300i_daddi(uint32 Instruction)			{        DBGPRINT_RS_RT_IMM("DADDI   ") };
void debug_r4300i_daddiu(uint32 Instruction)		{        DBGPRINT_RS_RT_IMM("DADDIU  ") };
void debug_r4300i_daddu(uint32 Instruction)			{         DBGPRINT_RS_RT_RD("DADDU   ") };
void debug_r4300i_ddiv(uint32 Instruction)			{            DBGPRINT_RS_RT("DDIV    ") };
void debug_r4300i_ddivu(uint32 Instruction)			{            DBGPRINT_RS_RT("DDIVU   ") };
void debug_r4300i_div(uint32 Instruction)			{            DBGPRINT_RS_RT("DIV     ") };
void debug_r4300i_divu(uint32 Instruction)			{            DBGPRINT_RS_RT("DIVU    ") };
void debug_r4300i_dmult(uint32 Instruction)			{            DBGPRINT_RS_RT("DMULT   ") };
void debug_r4300i_dmultu(uint32 Instruction)		{            DBGPRINT_RS_RT("DMULTU  ") };
void debug_r4300i_dsll(uint32 Instruction)			{         DBGPRINT_RT_RD_SA("DSLL    ") };
void debug_r4300i_dsll32(uint32 Instruction)		{        DBGPRINT_RT_RD_SA("DSLL32   ") };
void debug_r4300i_dsllv(uint32 Instruction)			{        DBGPRINT_RS_RT_RD("DSLLV    ") };
void debug_r4300i_dsra(uint32 Instruction)			{         DBGPRINT_RT_RD_SA("DSRA    ") };
void debug_r4300i_dsra32(uint32 Instruction)		{         DBGPRINT_RT_RD_SA("DSRA32  ") };
void debug_r4300i_dsrav(uint32 Instruction)			{         DBGPRINT_RT_RD_SA("DSRAV   ") };
void debug_r4300i_dsrl(uint32 Instruction)			{         DBGPRINT_RT_RD_SA("DSRL    ") };
void debug_r4300i_dsrl32(uint32 Instruction)		{         DBGPRINT_RT_RD_SA("DSRL32  ") };
void debug_r4300i_dsrlv(uint32 Instruction)			{         DBGPRINT_RT_RD_RS("DSRLV   ") };
void debug_r4300i_dsub(uint32 Instruction)			{         DBGPRINT_RS_RT_RD("DSUB    ") };
void debug_r4300i_dsubu(uint32 Instruction)			{         DBGPRINT_RS_RT_RD("DSUBU   ") };
void debug_r4300i_j(uint32 Instruction)				{                   sprintf(op_str,"%X: J       %08X", pc, INSTR_INDEX); };
void debug_r4300i_jal(uint32 Instruction)			{                   sprintf(op_str,"%08X: JAL     %08X", pc, INSTR_INDEX); };
void debug_r4300i_jalr(uint32 Instruction)			{            DBGPRINT_RS_RD("JALR    ") };
void debug_r4300i_jr(uint32 Instruction)			{               DBGPRINT_RS("JR      ") };
void debug_r4300i_lb(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("LB      ") };
void debug_r4300i_lbu(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("LBU     ") };
void debug_r4300i_ld(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("LD      ") };
void debug_r4300i_ldc1(uint32 Instruction)			{           DBGPRINT_OPCODE("LDC1 [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_ldl(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("LDL     ") };
void debug_r4300i_ldr(uint32 Instruction)			{   DBGPRINT_OPCODE("LDR [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_lh(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("LH      ") };
void debug_r4300i_lhu(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("LHU     ") };
void debug_r4300i_ll(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("LL      ") };
void debug_r4300i_lld(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("LLD     ") };
void debug_r4300i_lui(uint32 Instruction)			{           DBGPRINT_RT_IMM("LUI     ") };
void debug_r4300i_lw(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("LW      ") };
void debug_r4300i_lwc1(uint32 Instruction)			{           DBGPRINT_OPCODE("LWC1 [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_lwl(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("LWL     ") };
void debug_r4300i_lwr(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("LWR     ") };
void debug_r4300i_lwu(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("LWU     ") };
void debug_r4300i_mfhi(uint32 Instruction)			{               DBGPRINT_RD("MFHI    ") };
void debug_r4300i_mflo(uint32 Instruction)			{               DBGPRINT_RD("MFLO    ") };
void debug_r4300i_mthi(uint32 Instruction)			{               DBGPRINT_RS("MTHI    ") };
void debug_r4300i_mtlo(uint32 Instruction)			{               DBGPRINT_RS("MTLO    ") };
void debug_r4300i_mult(uint32 Instruction)			{            DBGPRINT_RS_RT("MULT    ") };
void debug_r4300i_multu(uint32 Instruction)			{            DBGPRINT_RS_RT("MULTU   ") };
void debug_r4300i_nor(uint32 Instruction)			{         DBGPRINT_RS_RT_RD("NOR     ") };
void debug_r4300i_or(uint32 Instruction)			{         DBGPRINT_RS_RT_RD("OR      ") };
void debug_r4300i_ori(uint32 Instruction)			{       DBGPRINT_RS_RT_IMMH("ORI     ") };
void debug_r4300i_sb(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("SB      ") };
void debug_r4300i_sc(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("SC      ") };
void debug_r4300i_scd(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("SCD     ") };
void debug_r4300i_sd(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("SD      ") };
void debug_r4300i_sdc1(uint32 Instruction)			{           DBGPRINT_OPCODE("SDC1 [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_sdl(uint32 Instruction)			{           DBGPRINT_OPCODE("SDL [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_sdr(uint32 Instruction)			{           DBGPRINT_OPCODE("SDR [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_sh(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("SH      ") };
void debug_r4300i_sll(uint32 Instruction)			{         DBGPRINT_RT_RD_SA("SLL     ") };
void debug_r4300i_sllv(uint32 Instruction)			{         DBGPRINT_RS_RT_RD("SLLV    ") };
void debug_r4300i_slt(uint32 Instruction)			{         DBGPRINT_RS_RT_RD("SLT     ") };
void debug_r4300i_slti(uint32 Instruction)			{        DBGPRINT_RS_RT_IMM("SLTI    ") };
void debug_r4300i_sltiu(uint32 Instruction)			{        DBGPRINT_RS_RT_IMM("SLTIU   ") };
void debug_r4300i_sltu(uint32 Instruction)			{         DBGPRINT_RS_RT_RD("SLTU    ") };
void debug_r4300i_sra(uint32 Instruction)			{         DBGPRINT_RT_RD_SA("SRA     ") };
void debug_r4300i_srav(uint32 Instruction)			{         DBGPRINT_RS_RT_RD("SRAV    ") };
void debug_r4300i_srl(uint32 Instruction)			{         DBGPRINT_RT_RD_SA("SRL     ") };
void debug_r4300i_srlv(uint32 Instruction)			{         DBGPRINT_RS_RT_RD("SRLV    ") };
void debug_r4300i_sub(uint32 Instruction)			{         DBGPRINT_RS_RT_RD("SUB     ") };
void debug_r4300i_subu(uint32 Instruction)			{         DBGPRINT_RS_RT_RD("SUBU    ") };
void debug_r4300i_sw(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("SW      ") };
void debug_r4300i_swc1(uint32 Instruction)			{           DBGPRINT_OPCODE("SWC1 [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_swl(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("SWL     ") };
void debug_r4300i_swr(uint32 Instruction)			{   DBGPRINT_BASE_RT_OFFSET("SWR     ") };
void debug_r4300i_sync(uint32 Instruction)			{           DBGPRINT_OPCODE("SYNC    ") };
void debug_r4300i_syscall(uint32 Instruction)		{           DBGPRINT_OPCODE("SYSCALL ") };
void debug_r4300i_teq(uint32 Instruction)			{            DBGPRINT_RS_RT("TEQ     ") };
void debug_r4300i_tge(uint32 Instruction)			{            DBGPRINT_RS_RT("TGE     ") };
void debug_r4300i_tgeu(uint32 Instruction)			{            DBGPRINT_RS_RT("TGEU    ") };
void debug_r4300i_tlt(uint32 Instruction)			{            DBGPRINT_RS_RT("TLT     ") };
void debug_r4300i_tltu(uint32 Instruction)			{            DBGPRINT_RS_RT("TLTU    ") };
void debug_r4300i_tne(uint32 Instruction)			{            DBGPRINT_RS_RT("TNE     ") };
void debug_r4300i_xor(uint32 Instruction)			{         DBGPRINT_RS_RT_RD("XOR     ") };
void debug_r4300i_xori(uint32 Instruction)			{       DBGPRINT_RS_RT_IMMH("XORI    ") };
void debug_r4300i_REGIMM_bgez(uint32 Instruction)	{           DBGPRINT_RS_OFF("BGEZ    ") };
void debug_r4300i_REGIMM_bgezall(uint32 Instruction){           DBGPRINT_RS_OFF("BGEZALL ") };
void debug_r4300i_REGIMM_bgezl(uint32 Instruction)	{           DBGPRINT_RS_OFF("BGEZL   ") };
void debug_r4300i_REGIMM_bltz(uint32 Instruction)	{           DBGPRINT_RS_OFF("BLTZ    ") };
void debug_r4300i_REGIMM_bltzal(uint32 Instruction)	{           DBGPRINT_RS_OFF("BLTZAL  ") };
void debug_r4300i_REGIMM_bltzall(uint32 Instruction){           DBGPRINT_RS_OFF("BLTZALL ") };
void debug_r4300i_REGIMM_bltzl(uint32 Instruction)	{           DBGPRINT_RS_OFF("BLTZL   ") };
void debug_r4300i_REGIMM_bgezal(uint32 Instruction)	{           DBGPRINT_RS_OFF("BGEZAL  ") };
void debug_r4300i_REGIMM_teqi(uint32 Instruction)	{           DBGPRINT_RS_IMM("TEQI    ") };
void debug_r4300i_REGIMM_tgei(uint32 Instruction)	{           DBGPRINT_RS_IMM("TGEI    ") };
void debug_r4300i_REGIMM_tgeiu(uint32 Instruction)	{           DBGPRINT_RS_IMM("TGEIU   ") };
void debug_r4300i_REGIMM_tlti(uint32 Instruction)	{           DBGPRINT_RS_IMM("TLTI    ") };
void debug_r4300i_REGIMM_tltiu(uint32 Instruction)	{           DBGPRINT_RS_IMM("TLTIU   ") };
void debug_r4300i_REGIMM_tnei(uint32 Instruction)	{           DBGPRINT_RS_IMM("TNEI    ") };
void debug_r4300i_COP0_eret(uint32 Instruction)		{           DBGPRINT_OPCODE("ERET    ") };
void debug_r4300i_COP0_mfc0(uint32 Instruction)		{       DBGPRINT_RT_FS_COP0("MFC0    ") };
void debug_r4300i_COP0_mtc0(uint32 Instruction)		{       DBGPRINT_RT_FS_COP0("MTC0    ") };
void debug_r4300i_COP0_tlbp(uint32 Instruction)		{           DBGPRINT_OPCODE("TLBP    ") };
void debug_r4300i_COP0_tlbr(uint32 Instruction)		{                   sprintf(op_str, "%08X: TLBR   (TLB[%d])", pc, 89); };
void debug_r4300i_COP0_tlbwi(uint32 Instruction)	{                   sprintf(op_str, "%08X: TLBWI   (TLB[%d])", pc, (COP0Reg[INDEX] & 0x1F)); };
void debug_r4300i_COP0_tlbwr(uint32 Instruction)	{                   sprintf(op_str, "%08X: TLBWR   (TLB[%d])", pc, (COP0Reg[INDEX] & 0x1F)); };
void debug_r4300i_COP1_abs_s(uint32 Instruction)	{            DBGPRINT_FD_FS("ABS.S   ") };
void debug_r4300i_COP1_abs_d(uint32 Instruction)	{            DBGPRINT_FD_FS("ABS.D   ") };
void debug_r4300i_COP1_add_s(uint32 Instruction)	{         DBGPRINT_FD_FS_FT("ADD.S   ") };
void debug_r4300i_COP1_add_d(uint32 Instruction)	{         DBGPRINT_FD_FS_FT("ADD.D   ") };
void debug_r4300i_COP1_bc1f(uint32 Instruction)		{           DBGPRINT_OPCODE("BC1F [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_bc1fl(uint32 Instruction)	{           DBGPRINT_OPCODE("BC1FL [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_bc1t(uint32 Instruction)		{           DBGPRINT_OPCODE("BC1T [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_bc1tl(uint32 Instruction)	{           DBGPRINT_OPCODE("BC1TL [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_F_S(uint32 Instruction)			{           DBGPRINT_OPCODE("C.F.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_F_D(uint32 Instruction)			{           DBGPRINT_OPCODE("C.F.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_UN_S(uint32 Instruction)		{           DBGPRINT_OPCODE("C.UN.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_UN_D(uint32 Instruction)		{           DBGPRINT_OPCODE("C.UN.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_EQ_S(uint32 Instruction)		{           DBGPRINT_OPCODE("C.EQ.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_EQ_D(uint32 Instruction)		{           DBGPRINT_OPCODE("C.EQ.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_UEQ_S(uint32 Instruction)		{           DBGPRINT_OPCODE("C.UEQ.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_UEQ_D(uint32 Instruction)		{           DBGPRINT_OPCODE("C.UEQ.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_OLT_S(uint32 Instruction)		{           DBGPRINT_OPCODE("C.OLT.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_OLT_D(uint32 Instruction)		{           DBGPRINT_OPCODE("C.OLT.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_ULT_S(uint32 Instruction)		{           DBGPRINT_OPCODE("C.ULT.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_ULT_D(uint32 Instruction)		{           DBGPRINT_OPCODE("C.ULT.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_OLE_S(uint32 Instruction)		{           DBGPRINT_OPCODE("C.OLE.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_OLE_D(uint32 Instruction)		{           DBGPRINT_OPCODE("C.OLE.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_ULE_S(uint32 Instruction)		{           DBGPRINT_OPCODE("C.ULE.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_ULE_D(uint32 Instruction)		{           DBGPRINT_OPCODE("C.ULE.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_SF_S(uint32 Instruction)		{           DBGPRINT_OPCODE("C.SF.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_SF_D(uint32 Instruction)		{           DBGPRINT_OPCODE("C.SF.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_NGLE_S(uint32 Instruction)		{           DBGPRINT_OPCODE("C.NGLE.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_NGLE_D(uint32 Instruction)		{           DBGPRINT_OPCODE("C.NGLE.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_SEQ_S(uint32 Instruction)		{           DBGPRINT_OPCODE("C.SEQ.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_SEQ_D(uint32 Instruction)		{           DBGPRINT_OPCODE("C.SEQ.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_NGL_S(uint32 Instruction)		{           DBGPRINT_OPCODE("C.NGL.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_NGL_D(uint32 Instruction)		{           DBGPRINT_OPCODE("C.NGL.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_LT_S(uint32 Instruction)		{           DBGPRINT_OPCODE("C.LT.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_LT_D(uint32 Instruction)		{           DBGPRINT_OPCODE("C.LT.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_NGE_S(uint32 Instruction)		{           DBGPRINT_OPCODE("C.NGE.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_NGE_D(uint32 Instruction)		{           DBGPRINT_OPCODE("C.NGE.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_LE_S(uint32 Instruction)		{           DBGPRINT_OPCODE("C.LE.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_LE_D(uint32 Instruction)		{           DBGPRINT_OPCODE("C.LE.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_NGT_S(uint32 Instruction)		{           DBGPRINT_OPCODE("C.NGT.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_C_NGT_D(uint32 Instruction)		{           DBGPRINT_OPCODE("C.NGT.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_ceilw(uint32 Instruction)	{           DBGPRINT_OPCODE("CEIL.W.fmt [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_ceill(uint32 Instruction)	{           DBGPRINT_OPCODE("CEIL.L.fmt [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_cfc1(uint32 Instruction)		{       DBGPRINT_RT_FS_COP1("CFC1    ") };
void debug_r4300i_COP1_ctc1(uint32 Instruction)		{       DBGPRINT_RT_FS_COP1("CTC1    ") };
void debug_r4300i_COP1_cvtd_s(uint32 Instruction)	{           DBGPRINT_OPCODE("CVT.D.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_cvtd_w(uint32 Instruction)	{           DBGPRINT_OPCODE("CVT.D.W [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_cvtd_l(uint32 Instruction)	{           DBGPRINT_OPCODE("CVT.D.L [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_cvtl_s(uint32 Instruction)	{           DBGPRINT_OPCODE("CVT.L.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_cvtl_d(uint32 Instruction)	{           DBGPRINT_OPCODE("CVT.L.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_cvts_d(uint32 Instruction)	{           DBGPRINT_OPCODE("CVT.S.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_cvts_w(uint32 Instruction)	{           DBGPRINT_OPCODE("CVT.S.W [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_cvts_l(uint32 Instruction)	{           DBGPRINT_OPCODE("CVT.S.L [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_cvtw_s(uint32 Instruction)	{           DBGPRINT_OPCODE("CVT.W.S [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_cvtw_d(uint32 Instruction)	{           DBGPRINT_OPCODE("CVT.W.D [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_div_s(uint32 Instruction)	{         DBGPRINT_FD_FS_FT("DIV.S   ") };
void debug_r4300i_COP1_div_d(uint32 Instruction)	{         DBGPRINT_FD_FS_FT("DIV.D   ") };
void debug_r4300i_COP1_dmfc1(uint32 Instruction)	{           DBGPRINT_OPCODE("DMFC1 [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_dmtc1(uint32 Instruction)	{           DBGPRINT_OPCODE("DMTC1 [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_floorl(uint32 Instruction)	{            DBGPRINT_FD_FS("FLOOR.L ") };
void debug_r4300i_COP1_floorw(uint32 Instruction)	{            DBGPRINT_FD_FS("FLOOR.W ") };
void debug_r4300i_COP1_mfc1(uint32 Instruction)		{           DBGPRINT_OPCODE("MFC1 [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_mov_s(uint32 Instruction)	{            DBGPRINT_FD_FS("MOV.S   ") };
void debug_r4300i_COP1_mov_d(uint32 Instruction)	{            DBGPRINT_FD_FS("MOV.D   ") };
void debug_r4300i_COP1_mtc1(uint32 Instruction)		{           DBGPRINT_OPCODE("MTC1 [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_mul_s(uint32 Instruction)	{         DBGPRINT_FD_FS_FT("MUL.S   ") };
void debug_r4300i_COP1_mul_d(uint32 Instruction)	{         DBGPRINT_FD_FS_FT("MUL.D   ") };
void debug_r4300i_COP1_neg_s(uint32 Instruction)	{            DBGPRINT_FD_FS("NEG.S   ") };
void debug_r4300i_COP1_neg_d(uint32 Instruction)	{            DBGPRINT_FD_FS("NEG.D   ") };
void debug_r4300i_COP1_roundl(uint32 Instruction)	{           DBGPRINT_OPCODE("ROUND.L.fmt [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_roundw(uint32 Instruction)	{           DBGPRINT_OPCODE("ROUND.W.fmt [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_sqrt_s(uint32 Instruction)	{            DBGPRINT_FD_FS("SQRT.S  ") };
void debug_r4300i_COP1_sqrt_d(uint32 Instruction)	{            DBGPRINT_FD_FS("SQRT.D  ") };
void debug_r4300i_COP1_sub_s(uint32 Instruction)	{         DBGPRINT_FD_FS_FT("SUB.S   ") };
void debug_r4300i_COP1_sub_d(uint32 Instruction)	{         DBGPRINT_FD_FS_FT("SUB.D   ") };
void debug_r4300i_COP1_truncl(uint32 Instruction)	{           DBGPRINT_OPCODE("TRUNC.L.fmt [UNIMPLEMENTED DBGPRINT]") };
void debug_r4300i_COP1_truncw_s(uint32 Instruction)	{            DBGPRINT_FD_FS("TRUNC.W.S") };
void debug_r4300i_COP1_truncw_d(uint32 Instruction)	{            DBGPRINT_FD_FS("TRUNC.W.D") };

//---------------------------------------------------------------------------------------
// table decoding function prototypes

static void debug_SPECIAL(uint32 Instruction);
static void debug_REGIMM(uint32 Instruction);
static void debug_COP0(uint32 Instruction);
static void debug_COP1(uint32 Instruction);
static void debug_TLB(uint32 Instruction);
static void debug_COP1_BC(uint32 Instruction);
static void debug_COP1_S(uint32 Instruction);
static void debug_COP1_D(uint32 Instruction);
static void debug_COP1_W(uint32 Instruction);
static void debug_COP1_L(uint32 Instruction);


//---------------------------------------------------------------------------------------
// opcode debug print function tables

void (*DebugInstruction[64])(uint32 Instruction) = {
	debug_SPECIAL,		debug_REGIMM,		debug_r4300i_j,			debug_r4300i_jal,		debug_r4300i_beq,		debug_r4300i_bne,		debug_r4300i_blez,		debug_r4300i_bgtz,
	debug_r4300i_addi,	debug_r4300i_addiu,	debug_r4300i_slti,		debug_r4300i_sltiu,		debug_r4300i_andi,		debug_r4300i_ori,		debug_r4300i_xori,		debug_r4300i_lui,
	debug_COP0,			debug_COP1,			debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_beql,		debug_r4300i_bnel,		debug_r4300i_blezl,		debug_r4300i_bgtzl,
	debug_r4300i_daddi,	debug_r4300i_daddiu,debug_r4300i_ldl,		debug_r4300i_ldr,		debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_lb,	debug_r4300i_lh,	debug_r4300i_lwl,		debug_r4300i_lw,		debug_r4300i_lbu,		debug_r4300i_lhu,		debug_r4300i_lwr,		debug_r4300i_lwu,
	debug_r4300i_sb,	debug_r4300i_sh,	debug_r4300i_swl,		debug_r4300i_sw,		debug_r4300i_sdl,		debug_r4300i_sdr,		debug_r4300i_swr,		debug_r4300i_cache,
	debug_r4300i_ll,	debug_r4300i_lwc1,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_lld,		debug_r4300i_ldc1,		debug_r4300i_unknown,	debug_r4300i_ld,
	debug_r4300i_sc,	debug_r4300i_swc1,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_scd,		debug_r4300i_sdc1,		debug_r4300i_unknown,	debug_r4300i_sd
};

void (*DebugREGIMMInstruction[32])(uint32 Instruction) = {
	debug_r4300i_REGIMM_bltz,	debug_r4300i_REGIMM_bgez,	debug_r4300i_REGIMM_bltzl,	debug_r4300i_REGIMM_bgezl,	debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,		debug_r4300i_unknown,
	debug_r4300i_REGIMM_tgei,	debug_r4300i_REGIMM_tgeiu,	debug_r4300i_REGIMM_tlti,	debug_r4300i_REGIMM_tltiu,	debug_r4300i_REGIMM_teqi,	debug_r4300i_unknown,	debug_r4300i_REGIMM_tnei,	debug_r4300i_unknown,
	debug_r4300i_REGIMM_bltzal,	debug_r4300i_REGIMM_bgezal,	debug_r4300i_REGIMM_bltzall,debug_r4300i_REGIMM_bgezall,debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,		debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,		debug_r4300i_unknown
};

void (*DebugSPECIALInstruction[64])(uint32 Instruction) = {
	debug_r4300i_sll,		debug_r4300i_unknown,	debug_r4300i_srl,		debug_r4300i_sra,		debug_r4300i_sllv,		debug_r4300i_unknown,	debug_r4300i_srlv,		debug_r4300i_srav,
	debug_r4300i_jr,		debug_r4300i_jalr,		debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_syscall,	debug_r4300i_break,		debug_r4300i_unknown,	debug_r4300i_sync,
	debug_r4300i_mfhi,		debug_r4300i_mthi,		debug_r4300i_mflo,		debug_r4300i_mtlo,		debug_r4300i_dsllv,		debug_r4300i_unknown,	debug_r4300i_dsrlv,		debug_r4300i_dsrav,
	debug_r4300i_mult,		debug_r4300i_multu,		debug_r4300i_div,		debug_r4300i_divu,		debug_r4300i_dmult,		debug_r4300i_dmultu,	debug_r4300i_ddiv,		debug_r4300i_ddivu,
	debug_r4300i_add,		debug_r4300i_addu,		debug_r4300i_sub,		debug_r4300i_subu,		debug_r4300i_and,		debug_r4300i_or,		debug_r4300i_xor,		debug_r4300i_nor,
	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_slt,		debug_r4300i_sltu,		debug_r4300i_dadd,		debug_r4300i_daddu,		debug_r4300i_dsub,		debug_r4300i_dsubu,
	debug_r4300i_tge,		debug_r4300i_tgeu,		debug_r4300i_tlt,		debug_r4300i_tltu,		debug_r4300i_teq,		debug_r4300i_unknown,	debug_r4300i_tne,		debug_r4300i_unknown,
	debug_r4300i_dsll,		debug_r4300i_unknown,	debug_r4300i_dsrl,		debug_r4300i_dsra,		debug_r4300i_dsll32,	debug_r4300i_unknown,	debug_r4300i_dsrl32,	debug_r4300i_dsra32
};

void (*DebugCOP0Instruction[32])(uint32 Instruction) = {
	debug_r4300i_COP0_mfc0,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_COP0_mtc0,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_TLB,				debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown
};

void (*DebugTLBInstruction[64])(uint32 Instruction) = {
	debug_r4300i_unknown,	debug_r4300i_COP0_tlbr,	debug_r4300i_COP0_tlbwi,debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_COP0_tlbwr,debug_r4300i_unknown,
	debug_r4300i_COP0_tlbp,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_COP0_eret,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown
};

void (*DebugCOP1Instruction[32])(uint32 Instruction) = {
	debug_r4300i_COP1_mfc1,	debug_r4300i_COP1_dmfc1,debug_r4300i_COP1_cfc1,	debug_r4300i_unknown,	debug_r4300i_COP1_mtc1,	debug_r4300i_COP1_dmtc1,debug_r4300i_COP1_ctc1,	debug_r4300i_unknown,
	debug_COP1_BC,			debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_COP1_S,			debug_COP1_D,			debug_r4300i_unknown,	debug_r4300i_unknown,	debug_COP1_W,			debug_COP1_L,			debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown
};

void (*DebugCOP1BCInstruction[4])(uint32 Instruction) = {
	debug_r4300i_COP1_bc1f,		debug_r4300i_COP1_bc1t,
	debug_r4300i_COP1_bc1fl,	debug_r4300i_COP1_bc1tl
};

void (*DebugCOP1SInstruction[64])(uint32 Instruction) = {
	debug_r4300i_COP1_add_s,	debug_r4300i_COP1_sub_s,	debug_r4300i_COP1_mul_s,debug_r4300i_COP1_div_s,	debug_r4300i_COP1_sqrt_s,	debug_r4300i_COP1_abs_s,	debug_r4300i_COP1_mov_s,	debug_r4300i_COP1_neg_s,
	debug_r4300i_COP1_roundl,	debug_r4300i_COP1_truncl,	debug_r4300i_COP1_ceill,debug_r4300i_COP1_floorl,	debug_r4300i_COP1_roundw,	debug_r4300i_COP1_truncw_s,	debug_r4300i_COP1_ceilw,	debug_r4300i_COP1_floorw,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_COP1_cvtd_s,	debug_r4300i_unknown,	debug_r4300i_unknown,		debug_r4300i_COP1_cvtw_s,	debug_r4300i_COP1_cvtl_s,	debug_r4300i_unknown,		debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,
	debug_r4300i_C_F_S,			debug_r4300i_C_UN_S,		debug_r4300i_C_EQ_S,	debug_r4300i_C_UEQ_S,		debug_r4300i_C_OLT_S,		debug_r4300i_C_ULT_S,		debug_r4300i_C_OLE_S,		debug_r4300i_C_ULE_S,
	debug_r4300i_C_SF_S,		debug_r4300i_C_NGLE_S,		debug_r4300i_C_SEQ_S,	debug_r4300i_C_NGL_S,		debug_r4300i_C_LT_S,		debug_r4300i_C_NGE_S,		debug_r4300i_C_LE_S,		debug_r4300i_C_NGT_S
};

void (*DebugCOP1DInstruction[64])(uint32 Instruction) = {
	debug_r4300i_COP1_add_d,	debug_r4300i_COP1_sub_d,	debug_r4300i_COP1_mul_d,	debug_r4300i_COP1_div_d,	debug_r4300i_COP1_sqrt_d,	debug_r4300i_COP1_abs_d,	debug_r4300i_COP1_mov_d,debug_r4300i_COP1_neg_d,
	debug_r4300i_COP1_roundl,	debug_r4300i_COP1_truncl,	debug_r4300i_COP1_ceill,	debug_r4300i_COP1_floorl,	debug_r4300i_COP1_roundw,	debug_r4300i_COP1_truncw_d,	debug_r4300i_COP1_ceilw,debug_r4300i_COP1_floorw,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_COP1_cvts_d,	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_COP1_cvtw_d,	debug_r4300i_COP1_cvtl_d,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_C_F_D,			debug_r4300i_C_UN_D,		debug_r4300i_C_EQ_D,		debug_r4300i_C_UEQ_D,		debug_r4300i_C_OLT_D,		debug_r4300i_C_ULT_D,		debug_r4300i_C_OLE_D,	debug_r4300i_C_ULE_D,
	debug_r4300i_C_SF_D,		debug_r4300i_C_NGLE_D,		debug_r4300i_C_SEQ_D,		debug_r4300i_C_NGL_D,		debug_r4300i_C_LT_D,		debug_r4300i_C_NGE_D,		debug_r4300i_C_LE_D,	debug_r4300i_C_NGT_D
};

void (*DebugCOP1WInstruction[64])(uint32 Instruction) = {
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_COP1_cvts_w,	debug_r4300i_COP1_cvtd_w,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown
};

void (*DebugCOP1LInstruction[64])(uint32 Instruction) = {
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_COP1_cvts_l,	debug_r4300i_COP1_cvtd_l,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,
	debug_r4300i_unknown,		debug_r4300i_unknown,		debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown,	debug_r4300i_unknown
};


//---------------------------------------------------------------------------------------
// table decoding functions

void debug_SPECIAL(uint32 Instruction)	{	DebugSPECIALInstruction[_FUNCTION_](Instruction);					}
void debug_REGIMM(uint32 Instruction)	{	DebugREGIMMInstruction[RT_FT](Instruction);							}
void debug_COP0(uint32 Instruction)		{	DebugCOP0Instruction[RS_BASE_FMT](Instruction);						}
void debug_COP1(uint32 Instruction)		{	DebugCOP1Instruction[RS_BASE_FMT](Instruction);						}
void debug_TLB(uint32 Instruction)		{	DebugTLBInstruction[_FUNCTION_](Instruction);						}
void debug_COP1_BC(uint32 Instruction)	{	DebugCOP1BCInstruction[((Instruction >> 16) & 0x03)](Instruction);	}
void debug_COP1_S(uint32 Instruction)	{	DebugCOP1SInstruction[_FUNCTION_](Instruction);						}
void debug_COP1_D(uint32 Instruction)	{	DebugCOP1DInstruction[_FUNCTION_](Instruction);						}
void debug_COP1_W(uint32 Instruction)	{	DebugCOP1WInstruction[_FUNCTION_](Instruction);						}
void debug_COP1_L(uint32 Instruction)	{	DebugCOP1LInstruction[_FUNCTION_](Instruction);						}

//---------------------------------------------------------------------------------------
// main decoding function



void DebugPrintInstruction(uint32 Instruction)
{	
	// generate the debug string function
	DebugInstruction[(Instruction >> 26)](Instruction);
	// refresh the opcode list
	RefreshOpList(op_str);
}

//---------------------------------------------------------------------------------------

void DebugPrintPC(uint32 thePC)
{
	uint32		instruction;
	// load the instruction
	instruction = (*((uint32*)(sDWORD_R[(thePC >> 16)] + (uint16)thePC)));
	// generate the debug string function
	DebugInstruction[(instruction >> 26)](instruction);
	// refresh the opcode list
	RefreshOpList(op_str);
}

//---------------------------------------------------------------------------------------

//Use this one if Instruction is global and if you want to test for DebuggerEnabled
void WinDynDebugPrintInstruction(uint32 Instruction)
{
	if (DebuggerEnabled)
		DebugPrintInstruction(Instruction);
}

//---------------------------------------------------------------------------------------

void DebugControllers(uint8 type, uint32 ctrlnum)
{
    char string[50];

	switch(type)
    {
        case GET_STATUS: sprintf(string, "Check Controller %i : GET_STATUS"		 , ctrlnum);	break;
        case RD_BUTTONS: sprintf(string, "Check Controller %i : RD_BUTTONS"		 , ctrlnum);	break;
        case RD_MEMPACK: sprintf(string, "Check Controller %i : RD_MEMPACK **"	 , ctrlnum);	break;
        case WR_MEMPACK: sprintf(string, "Check Controller %i : WD_MEMPACK **"	 , ctrlnum);	break;
        case RD_EEPROM:  sprintf(string, "Check Controller %i : RD_EEPROM **"	 , ctrlnum);	break;
        case WR_EEPROM:  sprintf(string, "Check Controller %i : WR_EEPROM **"	 , ctrlnum);	break;
        case RESET_CONT: sprintf(string, "Check Controller %i : RESET_CONT **"	 , ctrlnum);	break;
        default:         sprintf(string, "Check Controller %i : UNKNOWN COMMAND!", ctrlnum);	break;
    }
    RefreshOpList(string); 
}

//---------------------------------------------------------------------------------------

void __cdecl printlist (char * Message, ...)
{
	char Msg[400];
	va_list ap;
	
	va_start( ap, Message );
	vsprintf( Msg, Message, ap );
	va_end( ap );
	RefreshOpList(Msg);
}

//---------------------------------------------------------------------------------------

void Dbg_Handle_SP(uint32 value)
{
	if ((value & SP_CLR_HALT ))		{ printlist("SP_CLR_HALT");	}
	if ((value & SP_SET_HALT ))		{ printlist("SP_SET_HALT");	}
	if ((value & SP_CLR_BROKE))		{ printlist("SP_CLR_BROKE");}
	if ((value & SP_CLR_INTR ))		{ printlist("SP_CLR_INTR");	}
	if ((value & SP_SET_INTR ))		{ printlist("SP_SET_INTR");	}
	if ((value & SP_CLR_SSTEP ))	{ printlist("SP_CLR_SSTEP");}
	if ((value & SP_SET_SSTEP ))	{ printlist("SP_SET_SSTEP");}
	if ((value & SP_CLR_INTR_BREAK )){printlist("SP_CLR_INTR_BREAK");}
	if ((value & SP_SET_INTR_BREAK )){printlist("SP_SET_INTR_BREAK");}
	if ((value & SP_CLR_YIELD ))	{ printlist("SP_CLR_YIELD");	}
	if ((value & SP_SET_YIELD ))	{ printlist("SP_SET_YIELD");	}
	if ((value & SP_CLR_YIELDED ))	{ printlist("SP_CLR_YIELDED");	}
	if ((value & SP_SET_YIELDED ))	{ printlist("SP_SET_YIELDED");	}
	if ((value & SP_CLR_TASKDONE ))	{ printlist("SP_CLR_TASKDONE");	}
	if ((value & SP_SET_TASKDONE ))	{ printlist("SP_SET_TASKDONE");	}
	if ((value & SP_CLR_SIG3 ))		{ printlist("SP_CLR_SIG3");	}
	if ((value & SP_SET_SIG3 ))		{ printlist("SP_SET_SIG3");	}
	if ((value & SP_CLR_SIG4 ))		{ printlist("SP_CLR_SIG4");	}
	if ((value & SP_SET_SIG4 ))		{ printlist("SP_SET_SIG4"); }
	if ((value & SP_CLR_SIG5 ))		{ printlist("SP_CLR_SIG5"); }
	if ((value & SP_SET_SIG5 ))		{ printlist("SP_SET_SIG5"); }
	if ((value & SP_CLR_SIG6 ))		{ printlist("SP_CLR_SIG6"); }
	if ((value & SP_SET_SIG6 ))		{ printlist("SP_SET_SIG6"); }
	if ((value & SP_CLR_SIG7 ))		{ printlist("SP_CLR_SIG7"); }
	if ((value & SP_SET_SIG7 ))		{ printlist("SP_SET_SIG7");	}
}

#endif //DEBUG_COMMON