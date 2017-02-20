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


The project is a direct documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar. Most source
code comments are taken directly from anarko's n64toolkit with consent and are 
the property of anarko.

   This is version 0.5.2
   refer to readme.txt 
   for latest changes
*/

#include <stdio.h>
#include <stdlib.h>
#include "options.h"
#include "zlib/unzip.h"
//#include <zlib.h>
#include <direct.h>
#include "globals.h"
#include "n64rcp.h"
#include "fileio.h"
#include "memory.h"
#include "hardware.h"
#include "r4300i.h"
#include "1964ini.h"
#include "win32/wingui.h"
#include "win32/windebug.h"

BOOL Is_Reading_Rom_File=FALSE;;
BOOL To_Stop_Reading_Rom_File=FALSE;

BYTE gzipped_defaultm0[] = {
	0x1F, 0x8B, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xED, 0xCF, 0xB1, 0x09, 0x80, 0x40, 
	0x0C, 0x05, 0xD0, 0x9C, 0xF6, 0x4E, 0xE5, 0x42, 0x8E, 0xE0, 0x58, 0x0E, 0x65, 0x6B, 0x75, 0xA0, 
	0x16, 0x87, 0x8D, 0x60, 0x2A, 0xAB, 0x7B, 0x2F, 0x4D, 0xE0, 0xF3, 0x03, 0x59, 0xE3, 0xDB, 0x79, 
	0x9B, 0xE6, 0x7D, 0x7B, 0x05, 0xA5, 0x44, 0xAD, 0xC3, 0x91, 0xD4, 0xD3, 0xFE, 0xDF, 0xF7, 0xB3, 
	0x7E, 0x2C, 0xCF, 0x36, 0x76, 0x39, 0xBD, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD0, 0x5C, 0xAE, 0x72, 0x7F, 0x57, 0x00, 0x80, 0x00,
	0x00
};

#define STATE_SAVE_FORMAT_VERSION	(0x02)
#define VERSION_MAGIC_NUMBER		(0x19640064)
//---------------------------------------------------------------------------------------
void SwapRomName( uint8 *name )
{
	int i;
	uint8 c;
	for( i=0; i<20; i+=4 )
	{
		c = name[i];
		name[i]=name[i+3];
		name[i+3]=c;

		c = name[i+2];
		name[i+2]=name[i+1];
		name[i+1]=c;
	}

	for( i=19; i>=0; i-- )
	{
		if( name[i] != ' ' )
			break;
	}
	name[i+1] = '\0';

	//for( ; i>=0; i-- )
	//{
	//	if( name[i] == ':' )
	//		name[i] = '-';
	//}
}

void SwapRomHeader( uint8* romheader)
{
	int i;
	uint8 c;

	for( i=0; i<0x40; i+=4 )
	{
		c = romheader[i];
		romheader[i]=romheader[i+3];
		romheader[i+3]=c;

		c = romheader[i+2];
		romheader[i+2]=romheader[i+1];
		romheader[i+1]=c;
	}
}

// Read ROM Header information into the int_entry, return ROM size
long ReadRomHeader(char *rompath, INI_ENTRY* ini_entry )
{
	uint8 buffer[0x100];
	long int filesize;
	FILE *fp;
	//DisplayError("Read Rom Header: %s",rompath);
	fp = fopen(rompath, "rb");
    if( fp != NULL )   
	{
		// Get file size
		fseek(fp, 0, SEEK_END);
		filesize = ftell(fp);
		/* pad with zeros to fill the displacement */
		if (filesize & 0xFFFF)
			filesize = filesize + (0x10000 - (filesize & 0xFFFF));
		fseek(fp, 0, SEEK_SET); //set pointer to beginning of file

        fread( buffer, sizeof( uint8 ), 0x40, fp );
		if( ByteSwap(0x40, buffer) == TRUE )
		{
			strncpy(ini_entry->Game_Name,buffer+0x20, 0x14);
			SwapRomName(ini_entry->Game_Name);
			//SwapRomHeader(buffer);

			ini_entry->crc1 = *((uint32*)(buffer+0x10));
			ini_entry->crc2 = *((uint32*)(buffer+0x14));
			ini_entry->countrycode = buffer[0x3D];
			//ini_entry->countrycode = buffer[0x3E];
			fclose(fp);
			return filesize;
		}
		else
		{
			// This file is not a ROM file, skipped
			fclose(fp);
			return 0;
		}
	}
	else
	{
		// Can not open this file, skipped it
		return 0;
	}
}

long ReadZippedRomHeader(char* rompath, INI_ENTRY* ini_entry )
{
	long int filesize;
    unzFile fp;
	char szFileName[256];
	char ext[_MAX_EXT];
	uint8 buffer[0x100];

	//DisplayError("Read Zipped Rom Header: %s",rompath);
   
	fp = unzOpen(rompath);
	if( fp == NULL )
	{
		return FALSE;	// Can not open this ZIP file
	}

	if(unzGoToFirstFile(fp) == UNZ_OK)
	{
		do
		{
			unz_file_info file_info;
			if(unzGetCurrentFileInfo(fp, &file_info, szFileName, 256, NULL, 0, NULL, 0) == UNZ_OK)
			{
				filesize = file_info.uncompressed_size;
				if (filesize & 0xFFFF)
					filesize = filesize + (0x10000 - (filesize & 0xFFFF));

				strcpy(ext,szFileName+strlen(szFileName) - 4);
				if(	strcmp(ext, ".bin") == 0 || strcmp(ext,".v64") == 0 ||
					strcmp(ext, ".rom") == 0 || strcmp(ext,".usa") == 0 ||
					strcmp(ext, ".z64") == 0 || strcmp(ext,".n64") == 0 )
				{
					if(unzOpenCurrentFile(fp)== UNZ_OK)
					{
						if( unzReadCurrentFile(fp, buffer, 0x40) == 0x40 )
						{
							if( ByteSwap(0x40, buffer) )
							{
								strncpy(ini_entry->Game_Name,buffer+0x20, 0x14);
								SwapRomName(ini_entry->Game_Name);
								//SwapRomHeader(buffer);

								ini_entry->crc1 = *((uint32*)(buffer+0x10));
								ini_entry->crc2 = *((uint32*)(buffer+0x14));
								ini_entry->countrycode = buffer[0x3D];
								//ini_entry->countrycode = buffer[0x3E];
								
								unzClose(fp);
								return filesize;
							}
						}
					}
				}
			}
		}
		while(unzGoToNextFile(fp) == UNZ_OK);
    }
    unzClose(fp);
	return 0;	// Read ZIP file fails for some reason

}

BOOL ReadRomData(char* rompath)
{
    FILE* fp;
    unsigned long gROMLength; //size in bytes of the ROM

    if(stricmp(&rompath[strlen(rompath) - 4], ".zip") == 0)
    {
        ReadZippedRomData(rompath);
        return TRUE;
    }

    fp = fopen(rompath, "rb");
    if ( fp == NULL )
    {
        MessageBox(hwnd, "Error opening this file. Please refresh the ROM list.", "Error", MB_ICONINFORMATION);
        return FALSE;
    }
    rewind(fp);
    fseek(fp, 0, SEEK_END);
    gROMLength = ftell(fp);

    /* pad with zeros to fill the displacement */
    if (((gROMLength & 0xFFFF)) == 0)
        gAllocationLength = gROMLength;
    else
        gAllocationLength = gROMLength + (0x10000 - (gROMLength & 0xFFFF));

	InitVirtualRomMemory(gAllocationLength);

    InitMemoryLookupTables();
	InitTLB();

    fseek(fp, 0, SEEK_SET); //set pointer to beginning of file
    if( fp != NULL )   {
		uint64 i;
		MSG msg;

		Is_Reading_Rom_File=TRUE;;
		To_Stop_Reading_Rom_File=FALSE;

		for( i=0; i<gROMLength && To_Stop_Reading_Rom_File==FALSE; i+=65536)
		{
			if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
			{
				if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
				{
					if( GetMessage( &msg, NULL, 0, 0 ) ) 
						DispatchMessage (&msg) ;
				}
			}

			if( To_Stop_Reading_Rom_File==TRUE )
			{
				CloseROM();
				To_Stop_Reading_Rom_File = FALSE;
				Is_Reading_Rom_File=FALSE;

				fclose( fp );
				return FALSE;

			}

			fread(gMS_ROM_Image+i, sizeof(uint8), 65536, fp);
			sprintf(generalmessage, "Loading %s %d%%", rompath, i*100/gROMLength);
			SetStatusBarText(0, generalmessage);
		}
		SetStatusBarText(0, "Ready");

		Is_Reading_Rom_File=FALSE;

        ByteSwap(gAllocationLength, gMS_ROM_Image);
		memcpy((uint8*)&rominfo.validation, gMS_ROM_Image, 0x40);
		SwapRomHeader((uint8*)&rominfo.validation);

        /* Copy boot code to SP_DMEM */
        memcpy((uint8*)&SP_DMEM, gMS_ROM_Image, 0x1000);
		memcpy(rominfo.name,gMS_ROM_Image+0x20,20);
		SwapRomName(rominfo.name);
    }
	else 
	{
        DisplayError( "File could not be opened." );
        exit(0);
    }

    fclose( fp );

	return TRUE;
}




//---------------------------------------------------------------------------------------

void ReadZippedRomData(char* rompath)
{
    unzFile fp;
    unsigned long gROMLength; //size in bytes of the ROM
   
  if(fp = unzOpen(rompath))
  {
    char szFileName[256];
    if(unzGoToFirstFile(fp) == UNZ_OK)
    {
      do
      {
        unz_file_info file_info;
        if(unzGetCurrentFileInfo(fp,
                         &file_info,
                         szFileName,
                         256,
                         NULL,
                         0,
                         NULL,
                         0) == UNZ_OK)
        {
          if(stricmp(&szFileName[strlen(szFileName) - 4], ".bin") == 0
            || stricmp(&szFileName[strlen(szFileName) - 4], ".v64") == 0
            || stricmp(&szFileName[strlen(szFileName) - 4], ".rom") == 0)
          {
            gROMLength = file_info.uncompressed_size; //get size of ROM

            /* pad with zeros to fill the displacement */
            if (((gROMLength & 0xFFFF)) == 0)
                gAllocationLength = gROMLength;
            else
                gAllocationLength = gROMLength + (0x10000 - (gROMLength & 0xFFFF));

			InitVirtualRomMemory(gAllocationLength);

            InitMemoryLookupTables();
			InitTLB();

            if(unzOpenCurrentFile(fp)== UNZ_OK)
            {
                if(unzReadCurrentFile(fp, gMS_ROM_Image, sizeof( uint8 ) * gROMLength) == (int)(sizeof( uint8 ) * gROMLength))
                {
                    ByteSwap(gAllocationLength, gMS_ROM_Image);
					memcpy((uint8*)&rominfo.validation, gMS_ROM_Image, 0x40);
					SwapRomHeader((uint8*)&rominfo.validation);

                    /* Copy boot code to SP_DMEM */
                    memcpy((uint8*)&SP_DMEM, gMS_ROM_Image, 0x1000);
					memcpy(rominfo.name,gMS_ROM_Image+0x20,20);
					SwapRomName(rominfo.name);

                    unzClose(fp);
                    return;
                }
                else
                {
                    DisplayError("File could not be read. gROMLength = %08X\n", gROMLength );
                    unzClose(fp);
                    exit(0);
                }
                unzCloseCurrentFile(fp);
            }
            else
            {
                DisplayError("File could not be read: CRC Error in zip." );
                unzClose(fp);
                exit(0);
            }
          }
        }
        else
        {
              DisplayError("File could not unzipped." );
              unzClose(fp);
              exit(0);
        }
      }
      while(unzGoToNextFile(fp) == UNZ_OK);
    }
    unzClose(fp);
  }
}

//---------------------------------------------------------------------------------------

BOOL ByteSwap(uint32 Size, uint8* Image) 
{
    uint32  k;

    if (Image[0] == 0x80 && Image[1] == 0x37)
	{
        for (k=0; k < Size; k+=4) 
		{
            //Converts 40123780 to 0x80371240
            _asm {
                mov edx, Image
                add edx, k
                mov eax, dword ptr [edx]
                bswap eax
                mov dword ptr [edx], eax
            }
        }
		return TRUE;
	}
    else if (Image[0] == 0x37 && Image[1] == 0x80)
	//for (k=Size;k != 0; k-=4) {
    {
            //Converts 0x12408037 to 0x80371240
            k = Size;
            _asm {
                mov         edx,Image
                mov         ebx,dword ptr [k]
                jmp         _LABEL3
            }
_LABEL2:
            _asm {                              
                sub         ebx,8
                test        ebx, ebx
                jz          _LABELEXIT
            }
_LABEL3:
            _asm {
                //Yup i copied this asm routine twice..to cut down on the looping by 50%
                mov         eax,dword ptr [edx]
                mov         ecx,eax
                shr         ecx,10h
                shl         eax,10h
                or          ecx,eax
                mov         dword ptr [edx],ecx
                add         edx,4

                mov         eax,dword ptr [edx]
                mov         ecx,eax
                shr         ecx,10h
                shl         eax,10h
                or          ecx,eax
                mov         dword ptr [edx],ecx
                add         edx,4
                jmp         _LABEL2
            }
_LABELEXIT: ;
			return TRUE;
    }
	else
	{
		if( Image[2] == 0x37 && Image[3] == 0x80 )
		{
			//OK, this rom has already been swapped
			return TRUE;
		}
		else
		{
			// This is not a ROM file
			return FALSE;
		}
	}
}

//---------------------------------------------------------------------------------------

/* Load GNU Redistribution Conditions from file */
int LoadGNUDistConditions(char* ConditionsBuf)
{
    long filesize;
    FILE* fp;
    char temp_path[PATH_LEN]; //used for storing application path
    _getcwd( temp_path, PATH_LEN );
    strcat(temp_path, "\\dist.txt");
    
    if ((fp = fopen(temp_path, "rb")) == NULL) {
        sprintf(ConditionsBuf, "Error: %s not found.", temp_path);
        return( 0 );
    }
    rewind(fp);
    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if( fp != NULL )
    {
        fread( ConditionsBuf, sizeof( uint8 ), 11201, fp );
        ConditionsBuf[11201] = '\0';
    }
    else 
    {
        sprintf(ConditionsBuf, "Error getting fp");
        return( 0 );
    }
    fclose( fp );
    return( 1 );
}


//////////////////////////////////////////////////////////////////////////////
// Analyze String															//
//////////////////////////////////////////////////////////////////////////////
void AnalyzeString(char *temp)
{
	int i=-1;
	while(1)
	{
		i++;
		if (temp[i] == 0) break;
		// A-Z
		if ((temp[i] >= 0x41) && (temp[i] <= 0x5a)) continue;
		// a-z
		if ((temp[i] >= 0x61) && (temp[i] <= 0x7a)) continue;
		// 0-9
		if ((temp[i] >= 0x30) && (temp[i] <= 0x39)) continue;
		// "."
		if (temp[i] == 0x2E) continue;
		// "_"
		if (temp[i] == 0x5F) continue;
		// " "
		if (temp[i] == 0x20) continue;
		// "("
		if (temp[i] == 0x28) continue;
		// ")"
		if (temp[i] == 0x29) continue;
		// "-"
		if (temp[i] == 0x2D) continue;
		// "'"
		if (temp[i] == 0x27) continue;

		if( temp[i] == '\\' || temp[i] == ':' || temp[i] == '/' ) continue;

// unknown character ... print a "_"
		temp[i] = 0x5F;
	}
}

BOOL FileIO_CreateMempakFile( char *filename)
{
	FILE *dest;
	char temp[1024*32];
	gzFile *gztempfile;

	dest = fopen(filename, "wb");
	if( dest )
	{
		fwrite(&gzipped_defaultm0[0], 113, 1, dest);
		fclose(dest);
	}
	else
	{
		DisplayError("Unable to create new mempak file, please check your save directory setting.");
		return FALSE;
	}

	gztempfile = gzopen(filename, "rb");
	gzread(	gztempfile, temp, 1024*32 );
	gzclose(gztempfile);

	dest = fopen(filename, "wb");
	if( dest )
	{
		fwrite(temp, 1024*32, 1, dest);
		fclose(dest);
	}
	else
	{
		DisplayError("Unable to create new mempak file, please check your save directory setting.");
		return FALSE;
	}
	return TRUE;
}

BOOL FileIO_CreateFile( char * filename, int size )
{
	FILE *stream;
	_u8 tmp = 0;

	stream = fopen(filename, "wb");
	if( stream != NULL )
	{
		fwrite(&tmp, 1, size, stream);
		fclose(stream);
		return TRUE;
	}
	else
		return FALSE;
}


//////////////////////////////////////////////////////////////////////////////
// Get Directory															//
// To create a complete filename to load/save mempak/sram/eeprom			//
//////////////////////////////////////////////////////////////////////////////
void GetFileName( char* Directory, char *Ext) 
{
	char CRC[8];
	char romname[260];
	int i;

	for (i=0; i<8; i++)
		CRC[i] = ((char*)&rominfo.crc1)[i^3];

	strcpy(romname, rominfo.name);
	for( i=0; i<(int)strlen(romname); i++ )
		if( romname[i] == ':' )
			romname[i] = '-';

	sprintf(Directory, "%s%s-%X%X.%s",save_directory_to_use, romname, ((_u32*)&CRC)[0], ((_u32*)&CRC)[1], Ext);
	AnalyzeString(Directory);
}


//////////////////////////////////////////////////////////////////////////////
// Write MemPak Data to File												//
//////////////////////////////////////////////////////////////////////////////
extern _u8 mempak[4][1024*32];
BOOL FileIO_WriteMemPak(int pak_no)
{
	char temp[1024], ext[5];
	FILE *stream;
	
	sprintf(ext, "m%i", pak_no);
	GetFileName(temp, ext);

	stream = fopen(temp, "wb");
	if (stream == NULL)
	{
		DisplayError("Can not Write MEMPAK to file %s", temp);
		return TRUE;
	}

	TRACE1("Write MEMPAK to file: %s", temp)

	fwrite(mempak[0], 1024*32, 1, stream);
	fclose(stream);
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////////
// Load MemPak Data from File												//
//////////////////////////////////////////////////////////////////////////////
BOOL FileIO_LoadMemPak(int pak_no)
{
	char temp[1024], ext[5];
	FILE *stream;

	sprintf(ext, "m%i", pak_no);
	GetFileName(temp, ext);
	stream = fopen(temp, "rb");

	if (stream == NULL)
	{
		if( !FileIO_CreateMempakFile(temp) )
		//if( !FileIO_CreateFile(temp, 1024*32*4) )
		{
			DisplayError("Can not Load MEMPAK from file %s", temp);
			return TRUE;
		}
		else
		{
			stream = fopen(temp, "rb");
			if( stream == NULL )
			{
				DisplayError("Can not Load MEMPAK from file %s", temp);
				return TRUE;
			}
		}
	}
	TRACE1("Load MEMPAK from file: %s", temp)

	fread(mempak[0], 1024*32, 1, stream);
	fclose(stream);
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
// Write EEprom Data to File												//
//////////////////////////////////////////////////////////////////////////////
BOOL FileIO_WriteEEprom()
{
	char temp[1024];
	FILE *stream;

	GetFileName(temp, "eep");
	stream = fopen(temp, "wb");

	if (stream != NULL)
	{
		TRACE1("Write EEPROM to file: %s", temp)

		fwrite(EEprom, eepromsize, 1, stream);
		fclose(stream);
		return FALSE;
	}
	DisplayError("Can not write EEPROM to file");
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////////
// Load EEprom Data from File												//
//////////////////////////////////////////////////////////////////////////////
BOOL FileIO_LoadEEprom()
{
	char temp[1024];
	FILE *stream;

	GetFileName(temp, "eep");
	stream = fopen(temp, "rb");

	if (stream == NULL)
	{
		if( !FileIO_CreateFile(temp, eepromsize) )
		{
			DisplayError("Can not Load EEPROM from file %s", temp);
			return TRUE;
		}
		else
		{
			stream = fopen(temp, "rb");
			if( stream == NULL )
			{
				DisplayError("Can not Load EEPROM from file %s", temp);
				return TRUE;
			}
		}
	}

	TRACE1("Load EEPROM from file: %s", temp);

	fread(EEprom, eepromsize, 1, stream);
	fclose(stream);
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////////
// Write SRam Data to File													//
//////////////////////////////////////////////////////////////////////////////
BOOL FileIO_WriteSRAM()
{
	char temp[1024];
	FILE *stream;

	GetFileName(temp, "sra");
	stream = fopen(temp, "wb");

	if (stream != NULL)
	{

		TRACE1("Write SRAM into file: %s", temp);
		fwrite(SRam, SRAM_SIZE, 1, stream);
		fclose(stream);
		return FALSE;
	}
	else
	{
		DisplayError("Can not create file to save SRAM");
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// Load SRam Data from File													//
//////////////////////////////////////////////////////////////////////////////
BOOL FileIO_ReadSRAM()
{
	char temp[1024];
	FILE *stream;

	GetFileName(temp, "sra");
	stream = fopen(temp, "rb");

	if (stream != NULL)
	{
		TRACE1("Load SRAM from file: %s", temp);
		fread(SRam, SRAM_SIZE, 1, stream);
		fclose(stream);
		return FALSE;
	}
	else
	{
		memset(SRam, 0x00, SRAM_SIZE);
		DisplayError("Can not load SRAM from file: ",temp);
	}
	return TRUE;
}

BOOL FileIO_CreateFLASHRAM(char *filename)
{
	FILE *stream;

	memset(FlashRAM, 0, 128*1024);

	stream = fopen(filename, "wb");
	if( stream == NULL )
	{
		DisplayError("Error to create a new flashram file: %s, please check your directory setting", filename);
		return FALSE;
	}

	fwrite(FlashRAM, 128*1024, 1, stream);
	fclose(stream);
	return TRUE;
}

BOOL FileIO_WriteFLASHRAM()
{
	char temp[256];
	FILE *stream;

	GetFileName(temp, "fla");
	stream = fopen(temp, "wb");

	TRACE1("Save Flashram to file: %s", temp)

	fwrite(FlashRAM, 1024*128, 1, stream);
	fclose(stream);
	return TRUE;
}

BOOL FileIO_ReadFLASHRAM()
{
	char temp[256];
	FILE *stream;

	GetFileName(temp, "fla");
	stream = fopen(temp, "rb");

	if (stream == NULL)
	{
		return FileIO_CreateFLASHRAM(temp);
	}

	TRACE1("Load Flashram from file: %s", temp)

	fread(FlashRAM, 1024*128, 1, stream);
	fclose(stream);
	return TRUE;
}

extern int StateFileNumber;
void FileIO_gzSaveState()
{
	// Open File to write, file should be named as rom name
	char temp[1024];
	char ext[10];
	sprintf(ext,"sav%d", StateFileNumber);
	GetFileName(temp, ext);
	FileIO_gzSaveStateFile(temp);
}

void FileIO_gzReadHardwareState(gzFile *stream)
{
	memset((uint8*)(&gHardwareState),0,sizeof(HardwareState));
	gzread(stream, (uint8*)(&gHardwareState), (sizeof(HardwareState) - sizeof(gHardwareState.COP0Con) - sizeof(gHardwareState.RememberFprHi) - sizeof(gHardwareState.code)));
}
void FileIO_gzWriteHardwareState(gzFile *stream)
{
	gzwrite(stream, (uint8*)(&gHardwareState), (sizeof(HardwareState) - sizeof(gHardwareState.COP0Con) - sizeof(gHardwareState.RememberFprHi) - sizeof(gHardwareState.code)));
}

void FileIO_gzSaveStateFile(const char* filename)
{
	gzFile *stream;
	DWORD magic1964 = VERSION_MAGIC_NUMBER;

	stream = gzopen(filename, "wb");
	if (stream == NULL)
	{
		DisplayError("Can not create/open gzip file to save state");
		return;
	}

	TRACE1("Save state into gzip file: %s", filename);

	gzwrite(stream, (uint8*)(&magic1964), sizeof(DWORD));
	gzwrite(stream, (uint8*)(&current_rdram_size), sizeof(uint32));

	// All CPU registers
	FileIO_gzWriteHardwareState(stream);

	// All IO registers should be saved
	gzwrite(stream, (uint8*)gMS_ramRegs0, 0x30);	//ramRegs0
	gzwrite(stream, (uint8*)gMS_ramRegs4, 0x30);	//ramRegs4
	gzwrite(stream, (uint8*)gMS_ramRegs8, 0x30);	//ramRegs8
	gzwrite(stream, (uint8*)gMS_SP_MEM, MEMORY_SIZE_SPMEM);
	gzwrite(stream, (uint8*)gMS_SP_REG_1, MEMORY_SIZE_SPREG_1);
	gzwrite(stream, (uint8*)gMS_SP_REG_2, MEMORY_SIZE_SPREG_2);
	gzwrite(stream, (uint8*)gMS_DPC, 0x20);	//DPC
	gzwrite(stream, (uint8*)gMS_DPS, 0x10);	//DPS
	gzwrite(stream, (uint8*)gMS_MI, 0x10);	//MI
	gzwrite(stream, (uint8*)gMS_VI, 0x50);	//VI
	gzwrite(stream, (uint8*)gMS_AI, 0x18);	//AI
	gzwrite(stream, (uint8*)gMS_PI, 0x4C);	//PI
	gzwrite(stream, (uint8*)gMS_RI, 0x20);	//RI
	gzwrite(stream, (uint8*)gMS_SI, 0x1C);	//SI

	gzwrite(stream, (uint8*)gMS_RDRAM, current_rdram_size);	//RDRAM
	gzwrite(stream, (uint8*)gMS_GIO_REG, 0x804);	//GIO_REG
	gzwrite(stream, (uint8*)gMS_PIF, 0x800);	//PIF

	gzwrite(stream, (uint8*)gMS_TLB, sizeof(tlb_struct)*MAXTLB);

	
	gzwrite(stream, (uint8*)gMS_C2A1, MEMORY_SIZE_C2A1);	//C2A1
	gzwrite(stream, (uint8*)gMS_C1A1, MEMORY_SIZE_C1A1);	//C1A1
	gzwrite(stream, (uint8*)gMS_C2A2, MEMORY_SIZE_C2A2);	//C2A2
	gzwrite(stream, (uint8*)gMS_C1A3, MEMORY_SIZE_C1A3);	//C1A3

	gzclose(stream);

}

void FileIO_gzLoadState()
{
	// Open File to write, file should be named as rom name
	char temp[1024];
	char ext[10];

	sprintf(ext,"sav%d", StateFileNumber);

	GetFileName(temp, ext);
	FileIO_gzLoadStateFile(temp);
}

void FileIO_gzLoadStateFile(const char* filename)
{
	gzFile *stream;
	DWORD magic1964;
	uint32 rdram_size;

	stream = gzopen(filename, "rb");
	if (stream == NULL)
	{
		DisplayError("Can not open gzip file to load state");
		return;
	}

	TRACE1("Load state from file: %s", filename);

	gzread(stream, (uint8*)(&magic1964), sizeof(DWORD));
	if( magic1964 != VERSION_MAGIC_NUMBER )
		DisplayError("This state file is at different version or it is not a 1964 state save format, it may not work");

	gzread(stream, (uint8*)(&rdram_size), sizeof(uint32));
	ResetRdramSize(rdram_size == 0x400000?RDRAMSIZE_4MB:RDRAMSIZE_8MB);

	FileIO_gzReadHardwareState(stream);

	// All IO registers should be saved
	gzread(stream, (uint8*)gMS_ramRegs0, 0x30);	//ramRegs0
	gzread(stream, (uint8*)gMS_ramRegs4, 0x30);	//ramRegs4
	gzread(stream, (uint8*)gMS_ramRegs8, 0x30);	//ramRegs8
	gzread(stream, (uint8*)gMS_SP_MEM, MEMORY_SIZE_SPMEM);	//SPMEM and SP_REG, size??
	gzread(stream, (uint8*)gMS_SP_REG_1, MEMORY_SIZE_SPREG_1);	//SPMEM and SP_REG, size??
	gzread(stream, (uint8*)gMS_SP_REG_2, MEMORY_SIZE_SPREG_2);	//SPMEM and SP_REG, size??
	gzread(stream, (uint8*)gMS_DPC, 0x20);	//DPC
	gzread(stream, (uint8*)gMS_DPS, 0x10);	//DPS
	gzread(stream, (uint8*)gMS_MI, 0x10);	//MI
	gzread(stream, (uint8*)gMS_VI, 0x50);	//VI
	gzread(stream, (uint8*)gMS_AI, 0x18);	//AI
	gzread(stream, (uint8*)gMS_PI, 0x4C);	//PI
	gzread(stream, (uint8*)gMS_RI, 0x20);	//RI
	gzread(stream, (uint8*)gMS_SI, 0x1C);	//SI

	gzread(stream, (uint8*)gMS_RDRAM, rdram_size);	//RDRAM
	gzread(stream, (uint8*)gMS_GIO_REG, 0x804);	//GIO_REG
	gzread(stream, (uint8*)gMS_PIF, 0x800);	//PIF

	gzread(stream, (uint8*)gMS_TLB, sizeof(tlb_struct)*MAXTLB);
	Build_Whole_Direct_TLB_Lookup_Table();

	gzread(stream, (uint8*)gMS_C2A1, MEMORY_SIZE_C2A1);	//C2A1
	gzread(stream, (uint8*)gMS_C1A1, MEMORY_SIZE_C1A1);	//C1A1
	gzread(stream, (uint8*)gMS_C2A2, MEMORY_SIZE_C2A2);	//C2A2
	gzread(stream, (uint8*)gMS_C1A3, MEMORY_SIZE_C1A3);	//C1A3

	gzclose(stream);

#ifdef ENABLE_OPCODE_DEBUGGER
	Debugger_Copy_Memory(&gMemoryState_Interpreter_Compare, &gMemoryState);
	memcpy( &gHardwareState_Interpreter_Compare, &gHardwareState,		sizeof(HardwareState));
#endif

}

BOOL FileIO_Load1964Ini()
{
	char inifilepath[_MAX_PATH];
	FILE *stream;

    strcpy(inifilepath,main_directory);
    strcat(inifilepath,"1964.ini");

	stream = fopen(inifilepath, "rt");
	if (stream == NULL)
	{
		DisplayError("Can not found 1964.ini file.");
		return FALSE;
	}

	if( ReadAllIniEntries(stream) == FALSE )
	{
		DisplayError("Error to read information from 1964.ini");
		fclose(stream);
		return FALSE;
	}

	fclose(stream);
	return TRUE;
}

BOOL FileIO_Write1964Ini()
{
	char inifilepath[_MAX_PATH], bakfilepath[_MAX_PATH];
	char line[1000];
	FILE *stream, *stream2;

    strcpy(inifilepath,main_directory);
    strcat(inifilepath,"1964.ini");

    strcpy(bakfilepath,main_directory);
    strcat(bakfilepath,"1964.ini.bak");

	stream2 = fopen(bakfilepath, "wt");
	if( stream2 == NULL )
	{
		DisplayError("Can not open 1964.ini file to write.");
		return FALSE;
	}

	stream = fopen(inifilepath, "rt");
	if ( stream != NULL )
	{
		do {
			if( (fgets(line,255,stream) != NULL) && strncmp(line,"[",1)!=0 )
				fprintf(stream2, "%s", line);
			else
				break;
		} while( feof(stream) != 0 );
		fclose(stream);
	}
	else
	{
		DisplayError("1964.ini does not exist, create a new one");
	}

	if( WriteAllIniEntries(stream2) == FALSE )
	{
		DisplayError("Error to write information from 1964.ini");
		fclose(stream2);
		return FALSE;
	}

	fclose(stream2);

	remove(inifilepath);
	rename(bakfilepath,inifilepath);
	return TRUE;
}

