#ifndef __FILEIO_H
#define __FILEIO_H

#include <windows.h>

/* function declarations */
void ReadRomData(char* RomPath);
void ReadZippedRomData(char* RomPath);
void ByteSwap(uint32 Size, uint8* Image);
int	LoadGNUDistConditions(char* ConditionsBuf);
BOOL FileIO_WriteMemPak(int pak_no);
BOOL FileIO_LoadMemPak(int pak_no);
BOOL FileIO_WriteEEprom();
BOOL FileIO_LoadEEprom();
BOOL FileIO_WriteSRAM();
BOOL FileIO_ReadSRAM();

#endif
