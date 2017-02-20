#ifndef __1964_DYNACPU_H
#define __1964_DYNACPU_H

void dyna4300i_reserved(OP_PARAMS);		// ready :)
void dyna4300i_reserved1(OP_PARAMS);	// ready :) ..for normal instr reserved
void dyna4300i_invalid(OP_PARAMS);		// ready :)
void dyna4300i_special(OP_PARAMS);		// Table
void dyna4300i_regimm(OP_PARAMS);		// Table
void dyna4300i_j(OP_PARAMS);
void dyna4300i_jal(OP_PARAMS);
void dyna4300i_beq(OP_PARAMS);
void dyna4300i_bne(OP_PARAMS);
void dyna4300i_blez(OP_PARAMS);
void dyna4300i_bgtz(OP_PARAMS);
void dyna4300i_addi(OP_PARAMS);			// lionel
void dyna4300i_addiu(OP_PARAMS);			// lionel
void dyna4300i_slti(OP_PARAMS);			// lionel
void dyna4300i_sltiu(OP_PARAMS);			// lionel
void dyna4300i_andi(OP_PARAMS);			// lionel
void dyna4300i_ori(OP_PARAMS);			// lionel
void dyna4300i_xori(OP_PARAMS);			// lionel
void dyna4300i_lui(OP_PARAMS);			// lionel
void dyna4300i_cop0(OP_PARAMS);			// Table
void dyna4300i_cop1(OP_PARAMS);			// Table
void dyna4300i_cop2(OP_PARAMS);			// Table
void dyna4300i_beql(OP_PARAMS);
void dyna4300i_bnel(OP_PARAMS);
void dyna4300i_blezl(OP_PARAMS);
void dyna4300i_bgtzl(OP_PARAMS);
void dyna4300i_daddi(OP_PARAMS);
void dyna4300i_daddiu(OP_PARAMS);
void dyna4300i_ldl(OP_PARAMS);
void dyna4300i_ldr(OP_PARAMS);
void dyna4300i_lb(OP_PARAMS);
void dyna4300i_lh(OP_PARAMS);
void dyna4300i_lwl(OP_PARAMS);
void dyna4300i_lw(OP_PARAMS);			
void dyna4300i_lbu(OP_PARAMS);
void dyna4300i_lhu(OP_PARAMS);
void dyna4300i_lwr(OP_PARAMS);
void dyna4300i_lwu(OP_PARAMS);
void dyna4300i_sb(OP_PARAMS);
void dyna4300i_sh(OP_PARAMS);
void dyna4300i_swl(OP_PARAMS);
void dyna4300i_sw(OP_PARAMS);			
void dyna4300i_sdl(OP_PARAMS);
void dyna4300i_sdr(OP_PARAMS);
void dyna4300i_swr(OP_PARAMS);
void dyna4300i_cache(OP_PARAMS);
void dyna4300i_ll(OP_PARAMS);
void dyna4300i_lwc1(OP_PARAMS);
void dyna4300i_lwc2(OP_PARAMS);
void dyna4300i_lld(OP_PARAMS);
void dyna4300i_ldc1(OP_PARAMS);
void dyna4300i_ldc2(OP_PARAMS);
void dyna4300i_ld(OP_PARAMS);
void dyna4300i_sc(OP_PARAMS);
void dyna4300i_swc1(OP_PARAMS);
void dyna4300i_swc2(OP_PARAMS);
void dyna4300i_scd(OP_PARAMS);
void dyna4300i_sdc1(OP_PARAMS);
void dyna4300i_sdc2(OP_PARAMS);
void dyna4300i_sd(OP_PARAMS);
void dyna4300i_special_sll(OP_PARAMS);
void dyna4300i_special_srl(OP_PARAMS);
void dyna4300i_special_sra(OP_PARAMS);
void dyna4300i_special_sllv(OP_PARAMS);
void dyna4300i_special_srlv(OP_PARAMS);
void dyna4300i_special_srav(OP_PARAMS);
void dyna4300i_special_jr(OP_PARAMS);
void dyna4300i_special_jalr(OP_PARAMS);
void dyna4300i_special_syscall(OP_PARAMS);
void dyna4300i_special_break(OP_PARAMS);
void dyna4300i_special_sync(OP_PARAMS);
void dyna4300i_special_mfhi(OP_PARAMS);		// lionel
void dyna4300i_special_mthi(OP_PARAMS);		// lionel
void dyna4300i_special_mflo(OP_PARAMS);		// lionel
void dyna4300i_special_mtlo(OP_PARAMS);		// lionel
void dyna4300i_special_dsllv(OP_PARAMS);
void dyna4300i_special_dsrlv(OP_PARAMS);
void dyna4300i_special_dsrav(OP_PARAMS);
void dyna4300i_special_mult(OP_PARAMS);
void dyna4300i_special_multu(OP_PARAMS);		// lionel
void dyna4300i_special_div(OP_PARAMS);		// lionel
void dyna4300i_special_divu(OP_PARAMS);		// lionel
void dyna4300i_special_dmult(OP_PARAMS);
void dyna4300i_special_dmultu(OP_PARAMS);
void dyna4300i_special_ddiv(OP_PARAMS);
void dyna4300i_special_ddivu(OP_PARAMS);
void dyna4300i_special_add(OP_PARAMS);		
void dyna4300i_special_addu(OP_PARAMS);		// lionel
void dyna4300i_special_sub(OP_PARAMS);
void dyna4300i_special_subu(OP_PARAMS);		// lionel
void dyna4300i_special_and(OP_PARAMS);		// lionel
void dyna4300i_special_or(OP_PARAMS);		// lionel
void dyna4300i_special_xor(OP_PARAMS);		// lionel
void dyna4300i_special_nor(OP_PARAMS);		// lionel
void dyna4300i_special_slt(OP_PARAMS);		
void dyna4300i_special_sltu(OP_PARAMS);
void dyna4300i_special_dadd(OP_PARAMS);
void dyna4300i_special_daddu(OP_PARAMS);
void dyna4300i_special_dsub(OP_PARAMS);
void dyna4300i_special_dsubu(OP_PARAMS);
void dyna4300i_special_tge(OP_PARAMS);
void dyna4300i_special_tgeu(OP_PARAMS);
void dyna4300i_special_tlt(OP_PARAMS);
void dyna4300i_special_tltu(OP_PARAMS);
void dyna4300i_special_teq(OP_PARAMS);
void dyna4300i_special_tne(OP_PARAMS);
void dyna4300i_special_dsll(OP_PARAMS);
void dyna4300i_special_dsrl(OP_PARAMS);
void dyna4300i_special_dsra(OP_PARAMS);
void dyna4300i_special_dsll32(OP_PARAMS);	//lionel
void dyna4300i_special_dsrl32(OP_PARAMS);	//lionel
void dyna4300i_special_dsra32(OP_PARAMS);
void dyna4300i_regimm_bltz(OP_PARAMS);
void dyna4300i_regimm_bgez(OP_PARAMS);
void dyna4300i_regimm_bltzl(OP_PARAMS);
void dyna4300i_regimm_bgezl(OP_PARAMS);
void dyna4300i_regimm_tgei(OP_PARAMS);
void dyna4300i_regimm_tgeiu(OP_PARAMS);
void dyna4300i_regimm_tlti(OP_PARAMS);
void dyna4300i_regimm_tltiu(OP_PARAMS);
void dyna4300i_regimm_teqi(OP_PARAMS);
void dyna4300i_regimm_tnei(OP_PARAMS);
void dyna4300i_regimm_bltzal(OP_PARAMS);
void dyna4300i_regimm_bgezal(OP_PARAMS);
void dyna4300i_regimm_bltzall(OP_PARAMS);
void dyna4300i_regimm_bgezall(OP_PARAMS);
void dyna4300i_cop0_rs_mf(OP_PARAMS);
void dyna4300i_cop0_rs_dmf(OP_PARAMS);
void dyna4300i_cop0_rs_cf(OP_PARAMS);
void dyna4300i_cop0_rs_mt(OP_PARAMS);
void dyna4300i_cop0_rs_dmt(OP_PARAMS);
void dyna4300i_cop0_rs_ct(OP_PARAMS);
void dyna4300i_cop0_rs_bc(OP_PARAMS);
void dyna4300i_cop0_rs_co(OP_PARAMS);
void dyna4300i_cop0_rt_bcf(OP_PARAMS);
void dyna4300i_cop0_rt_bct(OP_PARAMS);
void dyna4300i_cop0_rt_bcfl(OP_PARAMS);
void dyna4300i_cop0_rt_bctl(OP_PARAMS);
void dyna4300i_cop0_tlbr(OP_PARAMS);
void dyna4300i_cop0_tlbwi(OP_PARAMS);
void dyna4300i_cop0_tlbwr(OP_PARAMS);
void dyna4300i_cop0_tlbp(OP_PARAMS);
void dyna4300i_cop0_eret(OP_PARAMS);
void dyna4300i_cop2_rs_not_implemented(OP_PARAMS);



extern void rs4300i_reserved(_u32 Instruction);
extern void rs4300i_reserved1(_u32 Instruction, _u32 pc); //for normal reserved instr
extern void rs4300i_invalid(_u32 Instruction);
extern void rs4300i_special(_u32 Instruction, _u32 pc);
extern void rs4300i_regimm(_u32 Instruction, _u32 pc);
extern void rs4300i_j(_u32 Instruction, _u32 pc);
extern void rs4300i_jal(_u32 Instruction, _u32 pc);
extern void rs4300i_beq(_u32 Instruction, _u32 pc);
extern void rs4300i_bne(_u32 Instruction, _u32 pc);
extern void rs4300i_blez(_u32 Instruction, _u32 pc);
extern void rs4300i_bgtz(_u32 Instruction, _u32 pc);
extern void rs4300i_addi(_u32 Instruction, _u32 pc);
extern void rs4300i_addiu(_u32 Instruction, _u32 pc);
extern void rs4300i_slti(_u32 Instruction, _u32 pc);
extern void rs4300i_sltiu(_u32 Instruction, _u32 pc);
extern void rs4300i_andi(_u32 Instruction, _u32 pc);
extern void rs4300i_ori(_u32 Instruction, _u32 pc);
extern void rs4300i_xori(_u32 Instruction, _u32 pc);
extern void rs4300i_lui(_u32 Instruction, _u32 pc);
extern void rs4300i_cop0(_u32 Instruction, _u32 pc);
extern void rs4300i_cop1(_u32 Instruction, _u32 pc);
extern void rs4300i_cop2(_u32 Instruction, _u32 pc);
extern void rs4300i_beql(_u32 Instruction, _u32 pc);
extern void rs4300i_bnel(_u32 Instruction, _u32 pc);
extern void rs4300i_blezl(_u32 Instruction, _u32 pc);
extern void rs4300i_bgtzl(_u32 Instruction, _u32 pc);
extern void rs4300i_daddi(_u32 Instruction, _u32 pc);
extern void rs4300i_daddiu(_u32 Instruction, _u32 pc);
extern void rs4300i_ldl(_u32 Instruction, _u32 pc);
extern void rs4300i_ldr(_u32 Instruction, _u32 pc);
extern void rs4300i_lb(_u32 Instruction, _u32 pc);
extern void rs4300i_lh(_u32 Instruction, _u32 pc);
extern void rs4300i_lwl(_u32 Instruction, _u32 pc);
extern void rs4300i_lw(_u32 Instruction, _u32 pc);
extern void rs4300i_lbu(_u32 Instruction, _u32 pc);
extern void rs4300i_lhu(_u32 Instruction, _u32 pc);
extern void rs4300i_lwr(_u32 Instruction, _u32 pc);
extern void rs4300i_lwu(_u32 Instruction, _u32 pc);
extern void rs4300i_sb(_u32 Instruction, _u32 pc);
extern void rs4300i_sh(_u32 Instruction, _u32 pc);
extern void rs4300i_swl(_u32 Instruction, _u32 pc);
extern void rs4300i_sw(_u32 Instruction, _u32 pc);
extern void rs4300i_sdl(_u32 Instruction, _u32 pc);
extern void rs4300i_sdr(_u32 Instruction, _u32 pc);
extern void rs4300i_swr(_u32 Instruction, _u32 pc);
extern void rs4300i_cache(_u32 Instruction, _u32 pc);
extern void rs4300i_ll(_u32 Instruction, _u32 pc);
extern void rs4300i_lwc1(_u32 Instruction, _u32 pc);
extern void rs4300i_lwc2(_u32 Instruction, _u32 pc);
extern void rs4300i_lld(_u32 Instruction, _u32 pc);
extern void rs4300i_ldc1(_u32 Instruction, _u32 pc);
extern void rs4300i_ldc2(_u32 Instruction, _u32 pc);
extern void rs4300i_ld(_u32 Instruction, _u32 pc);
extern void rs4300i_sc(_u32 Instruction, _u32 pc);
extern void rs4300i_swc1(_u32 Instruction, _u32 pc);
extern void rs4300i_swc2(_u32 Instruction, _u32 pc);
extern void rs4300i_scd(_u32 Instruction, _u32 pc);
extern void rs4300i_sdc1(_u32 Instruction, _u32 pc);
extern void rs4300i_sdc2(_u32 Instruction, _u32 pc);
extern void rs4300i_sd(_u32 Instruction, _u32 pc);
//extern void rs4300i_special_sll(_u32 Instruction);
extern void rs4300i_special_srl(_u32 Instruction);
extern void rs4300i_special_sra(_u32 Instruction);
extern void rs4300i_special_sllv(_u32 Instruction);
extern void rs4300i_special_srlv(_u32 Instruction);
extern void rs4300i_special_srav(_u32 Instruction);
extern void rs4300i_special_jr(_u32 Instruction);
extern void rs4300i_special_jalr(_u32 Instruction);
extern void rs4300i_special_syscall(_u32 Instruction);
extern void rs4300i_special_break(_u32 Instruction);
extern void rs4300i_special_sync(_u32 Instruction);
extern void rs4300i_special_mfhi(_u32 Instruction);
extern void rs4300i_special_mthi(_u32 Instruction);
extern void rs4300i_special_mflo(_u32 Instruction);
extern void rs4300i_special_mtlo(_u32 Instruction);
extern void rs4300i_special_dsllv(_u32 Instruction);
extern void rs4300i_special_dsrlv(_u32 Instruction);
extern void rs4300i_special_dsrav(_u32 Instruction);
extern void rs4300i_special_mult(_u32 Instruction);
extern void rs4300i_special_multu(_u32 Instruction);
extern void rs4300i_special_div(_u32 Instruction);
extern void rs4300i_special_divu(_u32 Instruction);
extern void rs4300i_special_dmult(_u32 Instruction);
extern void rs4300i_special_dmultu(_u32 Instruction);
extern void rs4300i_special_ddiv(_u32 Instruction);
extern void rs4300i_special_ddivu(_u32 Instruction);
extern void rs4300i_special_add(_u32 Instruction);
extern void rs4300i_special_addu(_u32 Instruction);
extern void rs4300i_special_sub(_u32 Instruction);
extern void rs4300i_special_subu(_u32 Instruction);
extern void rs4300i_special_and(_u32 Instruction);
extern void rs4300i_special_or(_u32 Instruction);
extern void rs4300i_special_xor(_u32 Instruction);
extern void rs4300i_special_nor(_u32 Instruction);
extern void rs4300i_special_slt(_u32 Instruction);
extern void rs4300i_special_sltu(_u32 Instruction);
extern void rs4300i_special_dadd(_u32 Instruction);
extern void rs4300i_special_daddu(_u32 Instruction);
extern void rs4300i_special_dsub(_u32 Instruction);
extern void rs4300i_special_dsubu(_u32 Instruction);
extern void rs4300i_special_tge(_u32 Instruction);
extern void rs4300i_special_tgeu(_u32 Instruction);
extern void rs4300i_special_tlt(_u32 Instruction);
extern void rs4300i_special_tltu(_u32 Instruction);
extern void rs4300i_special_teq(_u32 Instruction);
extern void rs4300i_special_tne(_u32 Instruction);
extern void rs4300i_special_dsll(_u32 Instruction);
extern void rs4300i_special_dsrl(_u32 Instruction);
extern void rs4300i_special_dsra(_u32 Instruction);
extern void rs4300i_special_dsll32(_u32 Instruction);
extern void rs4300i_special_dsrl32(_u32 Instruction);
extern void rs4300i_special_dsra32(_u32 Instruction);
extern void rs4300i_regimm_bltz(_u32 Instruction);
extern void rs4300i_regimm_bgez(_u32 Instruction);
extern void rs4300i_regimm_bltzl(_u32 Instruction);
extern void rs4300i_regimm_bgezl(_u32 Instruction);
extern void rs4300i_regimm_tgei(_u32 Instruction);
extern void rs4300i_regimm_tgeiu(_u32 Instruction);
extern void rs4300i_regimm_tlti(_u32 Instruction);
extern void rs4300i_regimm_tltiu(_u32 Instruction);
extern void rs4300i_regimm_teqi(_u32 Instruction);
extern void rs4300i_regimm_tnei(_u32 Instruction);
extern void rs4300i_regimm_bltzal(_u32 Instruction);
extern void rs4300i_regimm_bgezal(_u32 Instruction);
extern void rs4300i_regimm_bltzall(_u32 Instruction);
extern void rs4300i_regimm_bgezall(_u32 Instruction);
extern void rs4300i_cop0_rs_mf(_u32 Instruction);
extern void rs4300i_cop0_rs_dmf(_u32 Instruction);
extern void rs4300i_cop0_rs_cf(_u32 Instruction);
extern void rs4300i_cop0_rs_mt(_u32 Instruction);
extern void rs4300i_cop0_rs_dmt(_u32 Instruction);
extern void rs4300i_cop0_rs_ct(_u32 Instruction);
extern void rs4300i_cop0_rs_bc(_u32 Instruction);
extern void rs4300i_cop0_rs_co(_u32 Instruction);
extern void rs4300i_cop0_rt_bcf(_u32 Instruction);
extern void rs4300i_cop0_rt_bct(_u32 Instruction);
extern void rs4300i_cop0_rt_bcfl(_u32 Instruction);
extern void rs4300i_cop0_rt_bctl(_u32 Instruction);
extern void rs4300i_cop0_tlbr(_u32 Instruction);
extern void rs4300i_cop0_tlbwi(_u32 Instruction);
extern void rs4300i_cop0_tlbwr(_u32 Instruction);
extern void rs4300i_cop0_tlbp(_u32 Instruction);
extern void rs4300i_cop0_eret(_u32 Instruction);
extern void rs4300i_cop2_rs_not_implemented(_u32 Instruction);

       
extern dyn_cpu_instr dyna_instruction[64];
extern dyn_cpu_instr dyna_special_instruction[64];
extern dyn_cpu_instr dyna_regimm_instruction[32];
extern dyn_cpu_instr dyna_cop0_rs_instruction[32];
extern dyn_cpu_instr dyna_cop0_rt_instruction[32];
extern dyn_cpu_instr dyna_cop0_instruction[64];
extern dyn_cpu_instr dyna_cop2_rs_instruction[32];

extern void SetRdRsRt32bit(HardwareState* reg);
extern void SetRdRsRt64bit(HardwareState* reg);

#endif
