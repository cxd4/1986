/*______________________________________________________________________________
 |                                                                              |
 |  1964 - cheatcode.c                                                          |
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

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "options.h"
#include "globals.h"
#include "r4300i.h"
#include "1964ini.h"
#include "cheatcode.h"
#include "hardware.h"
#include "win32\wingui.h"

int  codegroupcount=0;
int  currentgroupindex = -1;
BOOL codemodified = FALSE;

static char current_rom_internal_name[30];

CODEGROUP *codegrouplist;

void CodeList_Clear(void)
{
	int i;
	
	for( i=0; i<codegroupcount; i++ )
		codegrouplist[i].codecount = 0;

	codegroupcount=0;
	codemodified = FALSE;

	if( codegrouplist != NULL )
	{
		VirtualFree(codegrouplist, 0, MEM_RELEASE);
	}
}


// According cheat code from file for current loaded rom
// Cheat Code file name: 1964cheatcode.dat
// File layout:
//   File contents entries for different rom
// Entry Layout:
//   [Rom Name]
//   Name1=b,12345678-xxxx,12345678-xxxx
//   Name2=b,12345678-xxxx,12345678-xxxx
//
//   b=0, 1
//     b=0   -> this group is not active
//     b=1	 -> this group is active
//
// Limition:
//   Support upto 30 games per rom
//   Support upto 10 code per group
//
// Activation:
//   Codes in a group will be activated/deactivated together
BOOL CodeList_ReadCode(char * intername_rom_name)
{
	char codefilepath[_MAX_PATH];
	char line[256], romname[256];
	BOOL found;
	unsigned int c1,c2;
	FILE *stream;

	strcpy(current_rom_internal_name, intername_rom_name);

    strcpy(codefilepath,main_directory);
    strcat(codefilepath,"1964cheatcode.dat");

	stream = fopen(codefilepath, "rt");
	if (stream == NULL)
	{
		// File does not exist, create a new empty one
		stream=fopen(codefilepath, "wt");

		if( stream == NULL )
		{
			DisplayError("Can not found 1964cheatcode.dat file and can not create it.");
			return FALSE;
		}
		
		fclose(stream);
		return TRUE;
	}

	// Locate the entry for current rom by searching for rominfo.name

	sprintf(romname,"[%s]", current_rom_internal_name);
	found = FALSE;

	while( fgets(line, 256, stream) )
	{
		chopm(line);
		if( strcmp(line,romname) != 0 )
			continue;
		else
		{
			found = TRUE;
			break;
		}
	}

	if( found )
	{
		int numberofgroups;
		// Read all code for current rom only

		// Step 1, read the number of groups
		if( fgets(line, 256, stream) )
		{
			chopm(line);
			if( strncmp(line, "NumberOfGroups=", 15) == 0 )
			{
				numberofgroups = atoi(line+15);
			}
			else
			{
				numberofgroups = MAX_CHEATCODE_GROUP_PER_ROM;
			}
		}
		else
		{
			return FALSE;
		}

		// Allocate memory for groups
		codegrouplist = VirtualAlloc(NULL, numberofgroups*sizeof(CODEGROUP), MEM_COMMIT, PAGE_READWRITE);
		if( codegrouplist == NULL )
		{
			DisplayError("Can not allocate memory to load cheat codes");
			return FALSE;
		}

		codegroupcount = 0;
		while(	codegroupcount < numberofgroups && fgets(line, 256, stream) && strlen(line) > 8 )
		{
			// Codes for the group are in the string line[]
			for( c1=0; line[c1] != '=' && line[c1] != '\0'; c1++ )
				codegrouplist[codegroupcount].name[c1] = line[c1];
			codegrouplist[codegroupcount].name[c1] = '\0';
 			codegrouplist[codegroupcount].active = line[c1+1]-'0';
			c1+=2;

			for( c2=0; c2 < (strlen(line)-c1-1)/14; c2++, codegrouplist[codegroupcount].codecount++)
			{
				codegrouplist[codegroupcount].codelist[c2].addr = ConvertHexStringToInt(line+c1+1+c2*14, 8);
				codegrouplist[codegroupcount].codelist[c2].val  = (uint16)ConvertHexStringToInt(line+c1+1+c2*14+9, 4);
			}
			codegroupcount++;
		}

		//DisplayError("Load %d groups of cheat code", codegroupcount);
	}
	else
	{
		// Can not found entry for the current rom
	}

	fclose(stream);
	return TRUE;
}

// Save cheat codes in file
BOOL CodeList_SaveCode(void)
{
	char codefilepath[_MAX_PATH], bakfilepath[_MAX_PATH];
	char line[256], romname[256];
	BOOL found;
	int c1,c2;
	FILE *stream, *stream2;

	if( !codemodified )return TRUE;		// Do nothing if no changes

    strcpy(codefilepath,main_directory);
    strcat(codefilepath,"1964cheatcode.dat");

    strcpy(bakfilepath,main_directory);
    strcat(bakfilepath,"1964cheatcode.dat.bak");

	sprintf(romname,"[%s]", current_rom_internal_name);

	stream2 = fopen(bakfilepath, "wt");
	if( stream2 == NULL )
	{
		DisplayError("Can not open 1964cheatcode.dat file to write.");
		return FALSE;
	}

	stream = fopen(codefilepath, "rt");
	if ( stream != NULL )
	{
		while( fgets(line, 256, stream) )
		{
			chopm(line);
			if( strcmp(line,romname) != 0 && found == FALSE)
			{
				fprintf(stream2,"%s\n", line);
			}
			else if( strcmp(line,romname) != 0 && found && line[0] == '[' )
			{
				fprintf(stream2,"%s\n", line);
				found = FALSE;
			}
			else
			{
				found = TRUE;
			}
		}
		fclose(stream);
	}
	else
	{
		DisplayError("1964cheatcode.dat does not exist, create a new one");
	}

	fprintf(stream2, "[%s]\n", current_rom_internal_name);
	fprintf(stream2, "NumberOfGroups=%d\n", codegroupcount);
	for( c1=0; c1<codegroupcount; c1++)
	{
		fprintf(stream2, "%s=%d,", codegrouplist[c1].name,codegrouplist[c1].active);
		for( c2=0; c2<codegrouplist[c1].codecount; c2++ )
		{
			fprintf(stream2, "%08X-%04X,", codegrouplist[c1].codelist[c2].addr, codegrouplist[c1].codelist[c2].val);
		}
		fprintf(stream2,"\n");
	}
	fprintf(stream2,"\n");
	fclose(stream2);

	remove(codefilepath);
	rename(bakfilepath,codefilepath);
	return TRUE;
}

/*  Apply game shark code. 

    Supports N64 game shark code types:
	Code Type Format	Code Type Description 
	----------------    ---------------------
	80-XXXXXX 00YY		8-Bit Constant Write 
	81-XXXXXX YYYY		16-Bit Constant Write 
	50-00AABB CCCC		Serial Repeater 
	88-XXXXXX 00YY		8-Bit GS Button Write 
	89-XXXXXX YYYY		16-Bit GS Button Write 
	A0-XXXXXX 00YY		8-Bit Constant Write (Uncached) 
	A1-XXXXXX YYYY		16-Bit Constant Write (Uncached) 
	D0-XXXXXX 00YY		8-Bit If Equal To 
	D1-XXXXXX YYYY		16-Bit If Equal To 
	D2-XXXXXX 00YY		8-Bit If Not Equal To 
	D3-XXXXXX YYYY		16-Bit If Not Equal To 
	DE-XXXXXX 0000		Download & Execute 
	F0-XXXXXX 00YY		8-Bit Bootup Write Once 
	F1-XXXXXX YYYY		16-Bit Bootup Write Once 


    Support 1964 only cheat/hack code types:
	    These two 1964 only code type could be used to hack ROM data without modifying
	    the real ROM file
	Code Type Format	Code Type Description 
	----------------    ---------------------
	0-XXXXXXX 00YY		8-Bit Constant ROM Write, to modify the ROM data after loading 
	1-XXXXXXX YYYY		16-Bit Constant ROM Write, to modify the ROM data after loading 

    Not supports N64 game shark code types:
	Code Type Format	Code Type Description 
	----------------    ---------------------
	CC-000000 0000
	DD-000000 0000
	EE-000000 0000		Disable Expansion Pak 
	FF-XXXXXX 0000		Store Activated Cheat Codes 

   1964 uses [F9] key as the Game Shark button
   */

// This function will apply codes in different modes
// mode = INGAME		Apply code in game play
// mode = BOOTUPONCE	Apply code after bootup once only
// mode = GSBUTTON		Apply code at GS botton pushed
// mode = ONLYIN1964	1964 only, Apply code after rom is loaded and before boot
BOOL CodeList_ApplyCode(int index, int mode)
{
	int i, codetype;
	uint32 addr;
	uint16 valword;
	uint8  valbyte;
	BOOL executenext = TRUE;

	if( Rom_Loaded && Emu_Is_Running && index < codegroupcount)
	{
		for( i=0; i<codegrouplist[index].codecount; i++)
		{
			if( executenext == FALSE ) // OK, skip this code
			{
				executenext = TRUE;
				continue;
			}

			codetype = codegrouplist[index].codelist[i].addr / 0x1000000;
			addr = (codegrouplist[index].codelist[i].addr & 0x00FFFFFF | 0x80000000);
			valword = codegrouplist[index].codelist[i].val;
			valbyte = (uint8)valword;


			__try{
				switch( mode )
				{
				case GSBUTTON:	// mode = 3		Apply code at GS botton pushed
					switch (codetype)
					{
					case 0x88:	//88-XXXXXX 00YY		8-Bit GS Button Write
						LOAD_UBYTE_PARAM(addr) = valbyte;
						break;
					case 0x89:	//89-XXXXXX YYYY		16-Bit GS Button Write
						LOAD_UHALF_PARAM(addr) = valword;
						break;
					}
				case INGAME:	// mode = 1		Apply code in game play
					switch (codetype)
					{
					case 0x80:	//80-XXXXXX 00YY		8-Bit Constant Write
						LOAD_UBYTE_PARAM(addr) = valbyte;
						break;
					case 0x81:	//81-XXXXXX YYYY		16-Bit Constant Write
						LOAD_UHALF_PARAM(addr) = valword;
						break;
					case 0x50:	//50-00AABB CCCC		Serial Repeater
						{
							int repeatcount = (addr & 0x0000FF00) >>2;
							uint32 addroffset = (addr & 0x000000FF);
							uint8 valinc = valbyte;

							if( i+1 < codegrouplist[index].codecount )
							{
								codetype = codegrouplist[index].codelist[i+1].addr / 0x1000000;
								addr = (codegrouplist[index].codelist[i+1].addr & 0x00FFFFFF | 0x80000000);
								valword = codegrouplist[index].codelist[i+1].val;
								valbyte = (uint8)valword;

								if( codetype == 0x80 ) // Only works if the next code is 0x80
								{
									do {
										LOAD_UBYTE_PARAM(addr) = valbyte;
										addr += addroffset;
										valbyte += valinc;
										repeatcount--;
									} while (repeatcount > 0);
								}
								else
								{
								}

							}
							else
							{
							}

						}
						break;
					case 0xA0:	//A0-XXXXXX 00YY		8-Bit Constant Write (Uncached)
						LOAD_UBYTE_PARAM(addr) = valbyte;
						break;
					case 0xA1:	//A1-XXXXXX YYYY		16-Bit Constant Write (Uncached)
						LOAD_UHALF_PARAM(addr) = valword;
						break;
					case 0xD0:	//D0-XXXXXX 00YY		8-Bit If Equal To
						if( LOAD_UBYTE_PARAM(addr) != valbyte )
							executenext = FALSE;
						break;
					case 0xD1:	//D1-XXXXXX YYYY		16-Bit If Equal To
						if( LOAD_UHALF_PARAM(addr) != valword )
							executenext = FALSE;
						break;
					case 0xD2:	//D2-XXXXXX 00YY		8-Bit If Not Equal To
						if( LOAD_UBYTE_PARAM(addr) == valbyte )
							executenext = FALSE;
						break;
					case 0xD3:	//D3-XXXXXX YYYY		16-Bit If Not Equal To
						if( LOAD_UHALF_PARAM(addr) == valword )
							executenext = FALSE;
						break;
					}
					break;
				case BOOTUPONCE:	// mode = 2		Apply code after bootup once only
					switch (codetype)
					{
					case 0xDE:	//DE-XXXXXX 0000		Download & Execute
						gHWS_pc = addr;
						break;
					case 0xF0:	//F0-XXXXXX 00YY		8-Bit Bootup Write Once
						LOAD_UBYTE_PARAM(addr) = valbyte;
							break;
					case 0xF1:	//F1-XXXXXX YYYY		16-Bit Bootup Write Once
						LOAD_UHALF_PARAM(addr) = valword;
						break;
					}
					break;
				case ONLYIN1964:	// Works in 1964 only
					switch (codetype)
					{
						if( (codetype & 0xE0) == 0x00 )		// 1964 only code type
						{
							addr = (codegrouplist[index].codelist[i].addr & 0x0FFFFFFF| 0x80000000);
							if( (codetype & 0xF0) == 0 )
							{
								//	0-XXXXXXX 00YY		8-Bit Constant ROM Write, to modify the ROM data after loading 
								LOAD_UBYTE_PARAM(addr) = valbyte;
							}
							else
							{
								//	1-XXXXXXX YYYY		16-Bit Constant ROM Write, to modify the ROM data after loading 
								LOAD_UHALF_PARAM(addr) = valword;
							}
						}
					}
					break;
				}
			}
			__except(NULL,EXCEPTION_EXECUTE_HANDLER)
			{
				DisplayError("Error to apply code: %08X, address %08X = %04X", codegrouplist[index].codelist[i].addr, addr, valword);
			}

		}
		return TRUE;
	}
	else
	{
		DisplayError("Please start the game first before apply the code");
		return FALSE;
	}

}

BOOL CodeList_ApplyAllCode(enum APPLYCHEATMODE mode)
{
	int i;
	for( i=0; i<codegroupcount; i++ )
	{
		if( codegrouplist[i].active )
			CodeList_ApplyCode(i,mode);
	}

	return TRUE;
}

BOOL CodeList_AddGroup()
{
	return TRUE;
}

BOOL CodeList_InputVerify()
{
	return TRUE;
}

// Is this string a hex number
BOOL IsHex(char *str)
{
	int i;
	int len=strlen(str);
	for( i=0; i<len; i++)
	{
		if( (str[i] >= '0' && str[i] <= '9') ||
			(str[i] >= 'a' && str[i] <= 'f') ||
			(str[i] >= 'A' && str[i] <= 'F') || str[i] == 0x0d)
			continue;
		else
			return FALSE;
	}
	return TRUE;
}

void StringDelete0x0a(char *str)
{
	int i,j;
	int len=strlen(str);
	if( len == 0 ) return;
	for( i=0; i<len; i++ )
	{
		if( str[i] == 0x0a )
		{
			// Delete this character
			for( j=i; j<len-1; j++)
				str[j] = str[j+1];
			len--;
		}
	}
}

extern uint32 ConvertHexStringToInt(const char *str, int nchars);
LRESULT APIENTRY CheatAndHackDialog(HWND hDlg, unsigned message, WORD wParam, LONG lParam) 
{
	int index;
	char codename[256];
	char codelines[256*MAX_CHEATCODE_GROUP_PER_ROM];
	BOOL  codeerror;
	CODEGROUP newgroup;

	switch (message) 
	{ 
	case WM_INITDIALOG:
		SendDlgItemMessage(hDlg, IDC_GAMESHARK_LIST, LB_RESETCONTENT, 0, 0);
		SetDlgItemText(hDlg, IDC_GAMESHARK_NAME, "");
		SetDlgItemText(hDlg, IDC_GAMESHARK_CODE, "");
		strcpy(codename, "Game shark code for: ");
		strcat(codename, current_rom_internal_name);
		SetWindowText(hDlg,codename);

		if( codegroupcount > 0 )
		{
			int i;
			for( i=0; i<codegroupcount; i++ )
			{
				strcpy(generalmessage, codegrouplist[i].name);
				if( codegrouplist[i].active )
				{
					strcat(generalmessage, " - V");
				}
				SendDlgItemMessage(hDlg, IDC_GAMESHARK_LIST, LB_INSERTSTRING, i, (LPARAM)generalmessage);
			}
		}

		return (TRUE); 
      
	case WM_COMMAND:
		switch (wParam) 
		{ 
		case IDC_GAMESHARK_ADD:
			codeerror = FALSE;

			// Check name
			GetDlgItemText(hDlg, IDC_GAMESHARK_NAME, codename, 255);
			if( strlen(codename) == 0 )
			{
				DisplayError("Please enter valid name for the codes");
				codeerror = TRUE;
			}
			else
			{
				int len;
				char *token;
				newgroup.codecount=0;

				strcpy(newgroup.name, codename);
				// Check code line by line
				GetDlgItemText(hDlg, IDC_GAMESHARK_CODE, codelines, 256*MAX_CHEATCODE_GROUP_PER_ROM);
				//StringDelete0x0a(codelines);
				len = strlen(codelines);
				token = strtok(codelines,"\x0a");
				while( token != NULL )
				{
					if( strlen(token) < 12 || IsHex(token)==FALSE )
					{
						codeerror = TRUE;
						break;
					}
					else
					{
						newgroup.codelist[newgroup.codecount].addr = ConvertHexStringToInt(token,8);
						newgroup.codelist[newgroup.codecount].val = (uint16)ConvertHexStringToInt(token+8,4);
						newgroup.codecount++;
						token = strtok(NULL, "\n");
					}
				}

				if( codeerror )
				{
					DisplayError("Codes must be all in HEX numbers, and each code must be 12 characters in length");
				}
			}

			if( codeerror == FALSE )
			{
				// To add the code into groups
				int i,j;
				BOOL found=FALSE;

				codemodified=TRUE; // Codes are modified, we will save it when quit from the dialog

				// step 1, check if there is a same name in current group list
				for( i=0; i<codegroupcount; i++ )
				{
					if( strcmp(codegrouplist[i].name, newgroup.name) == 0 )
					{
						found = TRUE;
						break;
					}
				}

				if( found == FALSE )
				{
					// Need to release and reallocate the memory blocks to add a new entry
					CODEGROUP* newgrouplist;
					newgrouplist = VirtualAlloc(NULL, (codegroupcount+1)*sizeof(CODEGROUP), MEM_COMMIT, PAGE_READWRITE);
					if( newgrouplist == NULL )
					{
						DisplayError("Can not allocate memory to add a new code group");
						break;
					}
					else
					{
						memcpy(newgrouplist, codegrouplist, codegroupcount*sizeof(CODEGROUP));
						VirtualFree(codegrouplist, 0, MEM_RELEASE);
						codegrouplist = newgrouplist;
					}
				}
				// Yes, replace the group and No, add as a new group
				for( j=0; j<newgroup.codecount; j++)
				{
					codegrouplist[i].codelist[j].addr = newgroup.codelist[j].addr;
					codegrouplist[i].codelist[j].val = newgroup.codelist[j].val;
				}
				codegrouplist[i].codecount = newgroup.codecount;
				strcpy(codegrouplist[i].name, newgroup.name);
				if( found == FALSE )
				{
					codegroupcount++;
					codegrouplist[i].active = FALSE;
					// Add the new group to the display list
					SendDlgItemMessage(hDlg, IDC_GAMESHARK_LIST, LB_INSERTSTRING, i, (LPARAM)codegrouplist[i].name);
				}
			}

			// Check memory range
			break;
		case IDC_GAMESHARK_DELETE:
			index = SendDlgItemMessage(hDlg, IDC_GAMESHARK_LIST, LB_GETCARETINDEX, 0, 0);
			if( index < codegroupcount)
			{
				if( index < codegroupcount-1 )
				{
					int j;
					for( j=index; j<codegroupcount-1; j++ )
					{
						memcpy(&codegrouplist[j], &codegrouplist[j+1], sizeof(CODEGROUP));
					}
				}
				codegroupcount--;
				SendDlgItemMessage(hDlg, IDC_GAMESHARK_LIST, LB_DELETESTRING, index, 0);
				codemodified=TRUE; // Codes are modified, we will save it when quit from the dialog
			}
			break;
		case IDC_GAMESHARK_ACTIVATE:
			index = SendDlgItemMessage(hDlg, IDC_GAMESHARK_LIST, LB_GETCARETINDEX, 0, 0);
			if( index < codegroupcount && !codegrouplist[index].active)
			{
				codegrouplist[index].active = TRUE;
				SendDlgItemMessage(hDlg, IDC_GAMESHARK_LIST, LB_DELETESTRING, index, 0);
				strcpy(generalmessage,codegrouplist[index].name);
				strcat(generalmessage," - V");
				SendDlgItemMessage(hDlg, IDC_GAMESHARK_LIST, LB_INSERTSTRING, index, (LPARAM)generalmessage);
				SendDlgItemMessage(hDlg, IDC_GAMESHARK_LIST, LB_SETCURSEL, index, 0);
			}
			codemodified=TRUE; // Codes are modified, we will save it when quit from the dialog
			break;
		case IDC_GAMESHARK_DEACTIVATE:
			index = SendDlgItemMessage(hDlg, IDC_GAMESHARK_LIST, LB_GETCARETINDEX, 0, 0);
			if( index < codegroupcount && codegrouplist[index].active)
			{
				codegrouplist[index].active = FALSE;
				SendDlgItemMessage(hDlg, IDC_GAMESHARK_LIST, LB_DELETESTRING, index, 0);
				SendDlgItemMessage(hDlg, IDC_GAMESHARK_LIST, LB_INSERTSTRING, index, (LPARAM)codegrouplist[index].name);
				SendDlgItemMessage(hDlg, IDC_GAMESHARK_LIST, LB_SETCURSEL, index, 0);
			}
			codemodified=TRUE; // Codes are modified, we will save it when quit from the dialog
			break;
		case IDOK: 
			CodeList_SaveCode();
		case IDCANCEL: 
				EndDialog(hDlg, TRUE); 
				return (TRUE); 
				break;

		} 
	case LBN_DBLCLK:
		{
			int index, i;
			if( (int)LOWORD(wParam) != IDC_GAMESHARK_LIST )
				return TRUE;
			index=SendDlgItemMessage(hDlg, IDC_GAMESHARK_LIST, LB_GETCARETINDEX, 0, 0);
			if( index < codegroupcount )
			{
				SetDlgItemText(hDlg, IDC_GAMESHARK_NAME, codegrouplist[index].name);
				codelines[0]='\0';
				for( i=0; i<codegrouplist[index].codecount; i++ )
				{
					sprintf(generalmessage, "%08X%04X\x0d\x0a", codegrouplist[index].codelist[i].addr, codegrouplist[index].codelist[i].val);
					strcat(codelines,generalmessage);
				}
				SetDlgItemText(hDlg, IDC_GAMESHARK_CODE, codelines);
			}
		}
		return TRUE;
    } 
	return (FALSE); 
}













