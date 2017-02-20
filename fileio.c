/*______________________________________________________________________________
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
#include "dbgprint.h"

//---------------------------------------------------------------------------------------
void SwapRomName( char name[21] )
{
	int i;
	char c;
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
}

void ReadRomData(char* RomPath)
{
    FILE* fp;
    unsigned long gROMLength; //size in bytes of the ROM

    if(stricmp(&RomPath[strlen(RomPath) - 4], ".zip") == 0)
    {
        ReadZippedRomData(RomPath);
        return;
    }

    fp = fopen(RomPath, "rb");
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
        fread( ROM_Image, sizeof( uint8 ), gROMLength, fp );

        ByteSwap(gAllocationLength, ROM_Image);

#ifdef LOCKROMMEMORY
		LockVirtualRomMemory();
#endif

        /* Copy boot code to SP_DMEM */
        memcpy((uint8*)&SP_DMEM, ROM_Image, 0x1000);

		memcpy(rominfo.name,ROM_Image+0x20,20);
		SwapRomName(rominfo.name);
    }
	else 
	{
        DisplayError( "File could not be opened." );
        exit(0);
    }

    fclose( fp );
}

//---------------------------------------------------------------------------------------

void ReadZippedRomData(char* RomPath)
{
    unzFile fp;
    unsigned long gROMLength; //size in bytes of the ROM
   
  if(fp = unzOpen(RomPath))
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
                if(unzReadCurrentFile(fp, ROM_Image, sizeof( uint8 ) * gROMLength) == (int)(sizeof( uint8 ) * gROMLength))
                {
                    ByteSwap(gAllocationLength, ROM_Image);

#ifdef LOCKROMMEMORY
		LockVirtualRomMemory();
#endif
                
                    /* Copy boot code to SP_DMEM */
                    memcpy((uint8*)&SP_DMEM, ROM_Image, 0x1000);
			
					memcpy(rominfo.name,ROM_Image+0x20,20);
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

void ByteSwap(uint32 Size, uint8* Image) {
    uint32  k;

    if (Image[0] == 0x80 && Image[1] == 0x37)
        for (k=0; k < Size; k+=4) {
            //Converts 40123780 to 0x80371240
            _asm {
                mov edx, Image
                add edx, k
                mov eax, dword ptr [edx]
                bswap eax
                mov dword ptr [edx], eax
            }
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
    }
_LABELEXIT: ;
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

// unknown character ... print a "_"
		temp[i] = 0x5F;
	}
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
	char path_buffer[_MAX_PATH], drive[_MAX_DRIVE] ,dir[_MAX_DIR];
	char fname[_MAX_FNAME],ext[_MAX_EXT];
	char temp[1024], filename[64];
	char CRC[8];
	int i;

 	GetModuleFileName(NULL,path_buffer,sizeof(path_buffer));
	_splitpath( path_buffer, drive, dir, fname, ext );

	strcpy(Directory,drive);
	strcat(Directory,dir);

	for (i=0; i<8; i++)
		CRC[i] = ((char*)&rominfo.crc1)[i^3];

	sprintf(filename, "%s-%X%X",rominfo.name, ((_u32*)&CRC)[0], ((_u32*)&CRC)[1]);
	AnalyzeString(filename);
	sprintf(temp,"%ssaves\\%s.", Directory, filename);

	strcpy(Directory, temp);
	strcat(Directory, Ext);	
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

#ifdef DEBUG_COMMON
	sprintf(generalmessage, "Write MEMPAK to file: %s", temp);
	RefreshOpList(generalmessage);
#endif

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
		if( !FileIO_CreateFile(temp, 1024*32*4) )
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
#ifdef DEBUG_COMMON
	sprintf(generalmessage, "Load MEMPAK from file: %s", temp);
	RefreshOpList(generalmessage);
#endif

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
#ifdef DEBUG_COMMON
		sprintf(generalmessage, "Write EEPROM to file: %s", temp);
		RefreshOpList(generalmessage);
#endif

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

#ifdef DEBUG_COMMON
	sprintf(generalmessage, "Load EEPROM from file: %s", temp);
	RefreshOpList(generalmessage);
#endif

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

#ifdef DEBUG_COMMON
		sprintf(generalmessage, "Write SRAM into file: %s", temp);
		RefreshOpList(generalmessage);
#endif

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

#ifdef DEBUG_COMMON
		sprintf(generalmessage,"Load SRAM from file: %s", temp);
		RefreshOpList(generalmessage);
#endif

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
