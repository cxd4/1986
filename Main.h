//Main.h Main's external dependencies

//Parse Externals
extern void ParseHeader();
extern void Parse6_5_5_16();
extern void Parse6_5_5_5_5_6();
extern void Parse6_20_6();
//extern void ParseCOPxInstruction(); //Get CoProssessor instructions

//Debugger externals
#ifdef _DEBUG
extern void RefreshConsole();
extern void DebuggerMain();
extern void freeconsole();
extern uint8 UpdateViewPort; //Whether or not to update the console
#endif

//Main Processor Opcodes
extern void lb();
extern void lbu();
extern void ld();
extern void ldl();
extern void ldr();
extern void lh();
extern void lhu();
extern void ll();
extern void lld();
extern void lw();
extern void lwl();
extern void lwr();
extern void lwu(); 
extern void sb(); 
extern void sc();
extern void scd();
extern void sd();
extern void sdl();
extern void sdr();
extern void sh();
extern void sw();
extern void swl();
extern void swr();

extern void ldc1();
extern void lwc1();
extern void sdc1();
extern void swc1();

extern void lui();

extern void addiu();
extern void addi();
extern void andi();
extern void daddi();
extern void ori();
extern void slti();
extern void sltiu();
extern void xori();

extern void beq();
extern void beql();
extern void bne();
extern void bnel();

extern void jal();

//Cop0 Opcodes
//extern void cfc0();
//extern void ctc0();
//extern dmfc0();
//extern dmtc0();
extern mfc0();
extern mtc0();

//Cop1 Opcodes
extern void cfc1();
extern void ctc1();
extern void dmfc1();
extern void dmtc1();
extern void mfc1();
extern void mtc1();

//Cop2 Opcodes (floats)
//extern void cfc2();
//extern void ctc2();
//extern void dmfc2();
//extern void dmtc2();
//extern void mfc2();
//extern void mtc2();  

//More MainCPU Opcodes (floats)
extern void ldc2();  
extern void lwc2();  
extern void sdc2();  
extern void swc2();

//Regimm Opcodes();
extern void bltz();
extern void bgez();
extern void bltzl();
extern void bgezl();
extern void tgei();
extern void tgeiu();
extern void tlti();
extern void tltiu();
extern void teqi();
extern void tnei();
extern void bltzal();
extern void bgezal();
extern void bltzall();
extern void bgezall();

//Special Opcodes
extern void sll();
extern void srl();
extern void sra();
extern void sllv();
extern void srlv();
extern void srav();
extern void jr();
extern void jalr();
extern void syscall();
extern void Break();
extern void sync();
extern void mfhi();
extern void mthi();
extern void mflo();
extern void mtlo();
extern void dsllv();
extern void dsrlv();
extern void dsrav();
extern void mult();
extern void multu();
extern void Div();
extern void divu();
extern void dmult();
extern void dmultu();
extern void ddiv();
extern void ddivu();
extern void add();
extern void addu();
extern void sub();
extern void subu();
extern void and();
extern void or();
extern void xor();
extern void nor();
extern void slt();
extern void sltu();
extern void dadd();
extern void daddu();
extern void dsub();
extern void dsubu();
extern void tge();
extern void tgeu();
extern void tlt();
extern void tltu();
extern void teq();
extern void tne();
extern void dsll();
extern void dsrl();
extern void dsra();
extern void dsll32();
extern void dsrl32();
extern void dsra32();

//FPU functions
extern void add_f();
extern void sub_f();
extern void mul();
extern void div_f();
extern void Sqrt();
extern void Abs();
extern void mov();
extern void neg();
extern void round(void* W_L);
extern void trunc(void* W_L);
extern void Ceil(void* W_L);
extern void Floor(void* W_L);
extern void Cvt(void* S_D_W_L); 
extern void c_cond();

//BC1 Instructions
extern void bc1f();
extern void bf1t();
extern void bc1fl();
extern void bc1tl();

//TLB
extern void tlbr();
extern void tlbwi();
extern void tlbwr();
extern void tlbp();
extern void eret();

//Vector opcodes
extern void vmulf();
extern void vmulu();
extern void vrndp();
extern void vmulq();
extern void vmudl();
extern void vmudm();
extern void vmudn();
extern void vmudh();
extern void vmacf();
extern void vmacu();
extern void vrndn();
extern void vmacq();
extern void vmadl();
extern void vmadm();
extern void vmadn();
extern void vmadh();
extern void vadd();
extern void vsub();

extern void vsut(); //may not exist

extern void vabs();
extern void vaddc();
extern void vsubc();

//extern void vaddb(); //
//extern void vsubb(); //
//extern void vaccb(); //
//extern void vsucb(); //these 7 may not exist
//extern void vsad();  //
//extern void vsac();  //
//extern void vsum();  //

extern void vsaw(); 
extern void vlt();
extern void veq();
extern void vne();
extern void vge();
extern void vcl();
extern void vch();
extern void vcr();
extern void vmrg();
extern void vand();
extern void vnand();
extern void vor();
extern void vnor();
extern void vxor();
extern void vnxor();
extern void vrcp();
extern void vrcpl();
extern void vrcph();
extern void vmov();
extern void vrsq();
extern void vrsql();
extern void vrsqh();