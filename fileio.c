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


The project is a direct documentation-to-code translation of the n64toolkit 
by my friend anarko and RSP info has been provided by zilmar :). Most source
code comments are taken directly from anarko's n64toolkit with consent and are 
the property of anarko.

   This is version 0.5.0
   refer to readme.txt 
   for latest changes
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "zlib/unzip.h"
#include <direct.h>
#include "globals.h"

/* function declarations */
void ReadRomData(char* RomPath);
void ReadZippedRomData(char* RomPath);
void ByteSwap(uint32 Size, uint8* Image);
int LoadGNUDistConditions(char* ConditionsBuf);

/* External functions */
extern void DebuggerUI();
extern void InitMemoryLookupTables();

//---------------------------------------------------------------------------------------

void ReadRomData(char* RomPath)
{
	FILE* fp;
	unsigned long gROMLength; //size in bytes of the ROM

	if(stricmp(&RomPath[strlen(RomPath) - 4], ".zip") == 0)
	{
		ReadZippedRomData(RomPath);
		return;
	}

	free(ROM_Image);

	fp = fopen(RomPath, "rb");
	rewind(fp);
	fseek(fp, 0, SEEK_END);
	gROMLength = ftell(fp);

    /* pad with zeros to fill the displacement */
	if (((gROMLength & 0xFFFF)) == 0)
		gAllocationLength = gROMLength;
	else
		gAllocationLength = gROMLength + (0x10000 - (gROMLength & 0xFFFF));

	if (( ROM_Image = (uint8*)calloc( gAllocationLength, sizeof( uint8 ))) == NULL ) 
    {
		DisplayError( NULL, "ReadRomData() Error: Could not allocate rom image buffer." );
        unzClose(fp);
        exit( 0 );
    }

	InitMemoryLookupTables();

	fseek(fp, 0, SEEK_SET); //set pointer to beginning of file
	if( fp != NULL )   {
		fread( ROM_Image, sizeof( uint8 ), gROMLength, fp );

		ByteSwap(gAllocationLength, ROM_Image);

		/* Copy boot code to SP_DMEM */
		memcpy((uint8*)&SP_DMEM, ROM_Image, 0x1000);
	} else {
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
	free(ROM_Image);
	
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

            if (( ROM_Image = (uint8*)calloc( gAllocationLength, sizeof( uint8 ))) == NULL ) 
            {
				DisplayError("Error: Could not allocate rom image buffer.\n" );
				unzClose(fp);
				exit( 0 );
            }

            InitMemoryLookupTables();

            if(unzOpenCurrentFile(fp)== UNZ_OK)
            {
				if(unzReadCurrentFile(fp, ROM_Image, sizeof( uint8 ) * gROMLength) == (int)(sizeof( uint8 ) * gROMLength))
				{
					ByteSwap(gAllocationLength, ROM_Image);
                
					/* Copy boot code to SP_DMEM */
					memcpy((uint8*)&SP_DMEM, ROM_Image, 0x1000);
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
	uint32	k;

	if (Image[0] == 0x80 && Image[1] == 0x37)
		for (k=Size; k != 0; k-=4) {
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
			}								

			_asm {								
				cmp         ebx,0
				je          _LABELEXIT
			}
_LABEL3:
			_asm {
				//Yup i copied this asm routine twice..to cut down on the looping by 50%
				add         edx,4
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
				jmp			_LABEL2
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
	char temp_path[PATH_LEN+5];

	strcpy(temp_path, AppPath);
	strcat(temp_path, "\\dist.txt");
	
	if ((fp = fopen(temp_path, "rb")) == NULL) {
		sprintf(ConditionsBuf, "Error: %s not found.", temp_path);
		return( 1 );
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
		return( 1 );
	}
	fclose( fp );
	return( 1 );
}