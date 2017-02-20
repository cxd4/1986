/*______________________________________________________________________________
 |                                                                              |
 |  1964 - fileio.h                                                             |
 |  Copyright (C) 2001 Joel Middendorf, <schibo@emulation64.com>                |
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
 |  email      : dyangchicago@yahoo.com, schibo@emulation64.com                 |
 |  paper mail :                                                                |
 |______________________________________________________________________________|
*/

#ifndef __FILEIO_H
#define __FILEIO_H

#include <windows.h>
#include "1964ini.h"

/* function declarations */
BOOL ReadRomData(char* RomPath);
long ReadRomHeader(char *RomPath, INI_ENTRY* ini_entry);
long ReadZippedRomHeader(char* RomPath, INI_ENTRY* ini_entry);
void ReadZippedRomData(char* RomPath);
BOOL ByteSwap(uint32 Size, uint8* Image);
int	LoadGNUDistConditions(char* ConditionsBuf);
BOOL FileIO_WriteMemPak(int pak_no);
BOOL FileIO_LoadMemPak(int pak_no);
BOOL FileIO_WriteEEprom();
BOOL FileIO_LoadEEprom();
BOOL FileIO_WriteSRAM();
BOOL FileIO_ReadSRAM();
BOOL FileIO_WriteFLASHRAM();
BOOL FileIO_ReadFLASHRAM();
void FileIO_SaveState();
void FileIO_LoadState();
void FileIO_gzSaveState();
void FileIO_gzLoadState();
void FileIO_gzSaveStateFile(const char* filename);
void FileIO_gzLoadStateFile(const char* filename);

BOOL FileIO_Load1964Ini();
BOOL FileIO_Write1964Ini();

extern BOOL Is_Reading_Rom_File;
extern BOOL To_Stop_Reading_Rom_File;

void SwapRomName( uint8 *name );
#endif
