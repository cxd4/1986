void	LoadLowMipsCpuRegister(unsigned long iMipsReg,unsigned char iIntelReg)
{
	MOV_MemoryToReg(1,iIntelReg,ModRM_disp32,(unsigned long)&gHWS_GPR[iMipsReg]);		
}

void	LoadHighMipsCpuRegister(unsigned long iMipsReg,unsigned char iIntelReg)
{
	MOV_MemoryToReg(1,iIntelReg,ModRM_disp32,(unsigned long)4+(unsigned long)&gHWS_GPR[iMipsReg]);
}
void	StoreLowMipsCpuRegister(unsigned long iMipsReg,unsigned char iIntelReg)
{
	
	MOV_RegToMemory(1,iIntelReg,ModRM_disp32,(unsigned long)&gHWS_GPR[iMipsReg]);
}