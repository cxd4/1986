//#define LOG_PIF

#include <windows.h>
#include <stdio.h>
#include "globals.h"
#include "options.h"
#include "n64rcp.h"
#include "hardware.h"
#include "iPif.h"
#include "fileio.h"
#include "debug_option.h"
#include "dbgprint.h"
#include "win32\DLL_Input.h"


uint8* PIF=NULL;

_u8 mempak[4][1024*32];		// Define 4 mempak, each one is 32K
BOOL mempak_used[4];		// status of each mempak
BOOL mempak_written[4];		// write status of each mempak. If it is never writen, will not save it

_u8	 EEProm_Status_Byte;
BOOL EEprom_used;
BOOL EEprom_written;
BOOL Sram_used;

_u8 bufin[64];

//////////////////////////////////////////////////////////////////////////////
// Init PIF																	//
//////////////////////////////////////////////////////////////////////////////
void Init_iPIF()
{
	mempak_used[0] = FALSE;
	mempak_used[1] = FALSE;
	mempak_used[2] = FALSE;
	mempak_used[3] = FALSE;
	Sram_used = FALSE;
	EEprom_used = FALSE;

	DisplayError("Emudata has never been initialized.");
	switch(EmuData.CartridgeType)
	{
		case CARTRIDGE_16KBIT_EEP:
			EEProm_Status_Byte = 0xC0;
			break;

		case CARTRIDGE_4KBIT_EEP:
			EEProm_Status_Byte = 0x80;
			break;
		
		default:
			EEProm_Status_Byte = 0x80;
			break;
	};
}



//////////////////////////////////////////////////////////////////////////////
// Close PIF																//
//////////////////////////////////////////////////////////////////////////////
void Close_iPIF()
{
	int i;

	// write mempaks to file if it was in use
	for (i=0; i<4; i++)
	{
		if (mempak_used[i] && mempak_written[i])
		{
			FileIO_WriteMemPak(i);
		}
	}		

	// write eeprom to file if it was in use
	if (EEprom_used && EEprom_written)
		FileIO_WriteEEprom();

	if( Sram_used )
	{
		FileIO_WriteSRAM();
	}
}


//////////////////////////////////////////////////////////////////////////////
// Build CRC for PAK														//
//////////////////////////////////////////////////////////////////////////////
void BuildCRC(_u8 *data, _u8 *crc)
{
	_u8 tmp,tmp2;
	int i,j;
	tmp=0;
	tmp2=0;
	for(i=0;i<=32;i++)
	{
		for(j=7;j>=0;j--)
		{
			if(tmp&0x80)
				tmp2=0x85;
			else
				tmp2=0x00;
			tmp<<=1;
			if(i==32)
				tmp |= 0;
			else
				if(data[i]&(1<<j))
					tmp|=1;
				else
					tmp|=0;
			tmp^=tmp2;
		}
	}
	*crc=tmp;
}



//////////////////////////////////////////////////////////////////////////////
// Read Data from MemPak													//
//////////////////////////////////////////////////////////////////////////////
void ReadControllerPak(int device, char *cmd)
{
	_u8 crc;
	_u16 offset = *(_u16*)&cmd[1];
	offset=(offset>>8)|(offset<<8);
	//_u16 offset = cmd[2]*0x100+cmd[1];

	offset = offset >> 5;


	if (offset > 0x400)
	{
// offset is out of range, dont copy Data
//    Handle Rumble Pack -> Rumble Pack Address: 0xC01B (offset = 0x600)		
	}
	else
	{
// Copy Data to Mempak
#ifdef DEBUG_DUMP_MEMPAK
		int i;
		char *str = generalmessage;
		if( debug_si_mempak )
		{
			sprintf(generalmessage, "Read MEMPAK %d, offset=0x%X",device,offset*32);
			RefreshOpList(generalmessage);
			for( i=0; i<32; i++)
			{
				sprintf(str, "%02x ",mempak[device][offset*32+i]);
				str+=3;
				if( (i+1)%8 == 0 )
				{
					*str++ = '\0';
					RefreshOpList(generalmessage);
					str=generalmessage;
				}
			}
			*str++ = '\0';
			RefreshOpList(generalmessage);
		}
#endif
		memcpy(&cmd[3], &(mempak[device][offset*32]), 32);
	}


// Build CRC of the Data
	BuildCRC(&cmd[3], &crc);

	cmd[35] = crc;
}



//////////////////////////////////////////////////////////////////////////////
// Write Data to MemPak														//
//////////////////////////////////////////////////////////////////////////////
void WriteControllerPak(int device, char *cmd)
{
	_u8 crc;
	_u16 offset = *(_u16*)&cmd[1];
	offset=(offset>>8)|(offset<<8);
	//_u16 offset = cmd[2]*0x100+cmd[1];

	offset = offset >> 5;

//LogPIFData(bufin, TRUE);

	if (offset > 0x400)
	{
// offset is out of range, dont copy Data
//    Handle Rumble Pack -> Rumble Pack Address: 0xC01B (offset = 0x600)		
	}
	else
	{
// Copy Data to Mempak
#ifdef DEBUG_DUMP_MEMPAK
		int i;
		char *str = generalmessage;
#endif

		memcpy(&(mempak[device][offset*32]),&cmd[3],32);
#ifdef DEBUG_DUMP_MEMPAK
		if( debug_si_mempak )
		{
			sprintf(generalmessage, "Write MEMPAK %d, offset=0x%X",device,offset*32);
			RefreshOpList(generalmessage);
			for( i=0; i<32; i++)
			{
				sprintf(str, "%02X ",mempak[device][offset*32+i]);
				str+=3;
				if( (i+1)%8 == 0 )
				{
					*str++ = '\0';
					RefreshOpList(generalmessage);
					str=generalmessage;
				}
			}
			*str++ = '\0';
			RefreshOpList(generalmessage);
		}
#endif

	}

// Build CRC of the Data
	BuildCRC(&cmd[3], &crc);

	cmd[35] = crc;
}



//////////////////////////////////////////////////////////////////////////////
// Handles all Commands which are sended to a Controller					//
//////////////////////////////////////////////////////////////////////////////
BOOL ControllerCommand(_u8 *cmd, int device)
{
	if (!Controls[device].Present)
	{
		cmd[1] |= 0x80;
		cmd[3] = 0xFF;
		cmd[4] = 0xFF;
		cmd[5] = 0xFF;
		return TRUE;
	}

	switch (cmd[2]) 
	{
// Get Controller status
	case 0xFF:  // 0xFF could something like Reset Controller and return the status
	case 0x00:	// 0x00 return the status
		cmd[3] = 0x05;  // reenginerd per Adaptoid only
		cmd[4] = 0x00;	// reenginerd per Adaptoid only
		if (Controls[device].Plugin & PLUGIN_MEMPAK)
			cmd[5] = 0x01;
		else
			cmd[5] = 0x00; // no mempack - reenginerd per Adaptoid only (Bit 0x01 would be rumble-pack)
		break;

// Read Controller Data ... need a DInput interface first =)
	case 0x01:
		{
			BUTTONS Keys;
			CONTROLLER_GetKeys(device, &Keys);
			*(DWORD*)&cmd[3] = *(DWORD*)&Keys;
		
#ifdef DEBUG_SI_CONTROLLER
			if( debug_si_controller )
			{
				sprintf(generalmessage, "Read controller %d, return %X",device,*(DWORD*)&Keys);
				RefreshOpList(generalmessage);
			}
#endif
		}
		break;

// Read Controller Pak
	case 0x02:
		switch(Controls[device].Plugin)
		{
		case PLUGIN_MEMPAK:
			if (!mempak_used[device])
			{
				FileIO_LoadMemPak(device);
				mempak_used[device] = TRUE;
			}	
			ReadControllerPak(device, &cmd[2]);
			break;

		case PLUGIN_NONE:
		case PLUGIN_RUMBLE_PAK:
		case PLUGIN_TANSFER_PAK:
			break;

		default:
			break;
		}
		return FALSE;
		break;

	// Write Controller Pak
	case 0x03:
		switch(Controls[device].Plugin)
		{
		case PLUGIN_MEMPAK:
			if (!mempak_used[device])
			{
				if( !FileIO_LoadMemPak(device) )
					mempak_used[device] = TRUE;
			}
			WriteControllerPak(device, &cmd[2]);
			mempak_written[device] = TRUE;
			break;

		case PLUGIN_NONE:
		case PLUGIN_RUMBLE_PAK:
		case PLUGIN_TANSFER_PAK:
			break;

		default:
			break;
		}
		return FALSE;
		break;

	default:
		DisplayError("Unkown ControllerCommand %x", cmd[2]);
		exit(IPIF_EXIT);
		break;
	}

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////////
// Read EEprom (loads EEprom from file if it is present)					//
//////////////////////////////////////////////////////////////////////////////
void ReadEEprom(char *dest, long offset)
{
#ifdef DEBUG_SI_EEPROM
	if( debug_si_eeprom )
	{
		sprintf(generalmessage, "Read from EEPROM");
		RefreshOpList(generalmessage);
	}
#endif
	if (!EEprom_used)
	{
		FileIO_LoadEEprom();
		EEprom_used = TRUE;
	}

	memcpy(dest, &EEprom[offset], 8);
}



//////////////////////////////////////////////////////////////////////////////
// Writes to EEprom															//
//////////////////////////////////////////////////////////////////////////////
void WriteEEprom(char *src, long offset)
{
#ifdef DEBUG_SI_EEPROM
	if( debug_si_eeprom )
	{
		sprintf(generalmessage, "Write to EEPROM");
		RefreshOpList(generalmessage);
	}
#endif
	if (!EEprom_used)
	{
		FileIO_LoadEEprom();
		EEprom_used = TRUE;
	}
	
	memcpy(&EEprom[offset], src, 8);	
	EEprom_written = TRUE;
}


		
/////////////////////////////////////////////////////////////////////////////
// Handles all Commands which are sended to the EEprom						//
//////////////////////////////////////////////////////////////////////////////
BOOL EEpromCommand(_u8 *cmd, int device)
{
	switch (cmd[2]) 
	{
	// reporting eeprom state ... hmmm
	case 0xFF:
	case 0x00: 
#ifdef DEBUG_SI_EEPROM
	if( debug_si_eeprom )
	{
		sprintf(generalmessage, "Execute EEPROM GetStatis Commands");
		RefreshOpList(generalmessage);
	}
#endif
		cmd[3] = 0x00;


//		cmd[4] = EEProm_Status_Byte;
		if( eepromsize == EEPROM_SIZE_4KB )
			cmd[4] = 0xc0;
		else
			cmd[4] = 0x80;


		cmd[5] = 0x00;
		break;

	// Read from Eeprom
	case 0x04: 
//        DisplayError("Read eeprom");
        ReadEEprom(&cmd[4], cmd[3] * 8);
		break;

	// Write to Eeprom
	case 0x05:
//        DisplayError("Write eeprom");
		WriteEEprom(&cmd[4], cmd[3] * 8);
		break;

	default:
		//DisplayError("Unknown EepromCommand %x", cmd[2]);
		//exit(IPIF_EXIT);
		break;
	}

	return FALSE;
}



//////////////////////////////////////////////////////////////////////////////
// Decode the PIF Rom Data													//
//////////////////////////////////////////////////////////////////////////////
void iPifCheck()
{
	int i, count, device;
	for(i=0;i<64;i++)
	{
//		bufin[i]=mem.pi_ram[i^3];
	    bufin[i]=PIF[(PIF_RAM_PHYS+i)^3];
    }

#ifdef LOG_PIF
	LogPIFData(bufin, TRUE);
#endif

	count = 0;
	device = 0;
	while(count < 64)
	{
		_u8 *cmd = &bufin[count];

// Command Block is ready
		if (cmd[0] == 0xFE) 
			break;

// no-op Commands
		// FD is from Command and Conquer
		if ((cmd[0] == 0xFF) ||
			(cmd[0] == 0xFD))
		{ 
			count++; 
			continue; 
		}

// Next Device
		if (cmd[0] == 0x00){ 
			count++;
			device++;
			continue; 
		}

// Device Channel to large (0-3 = Controller, 4 = EEprom)
		if (device > 4) 
			break;

// We get a Real Command now =)
		switch (device)
		{
// Controler Command
		case 0:
		case 1:
		case 2:
		case 3:
			if (Controls[device].RawData)
			{
				CONTROLLER_ControllerCommand(device, cmd);
				break;
			}
			else if (!ControllerCommand(cmd, device))
			{
				count = 64;
				break;
			}
			break;

// EEprom Command
		case 4:
			if (!EEpromCommand(cmd, device))
				count = 64;
			break;

		default:
			DisplayError("Unknown Command for unknwon Device %x", device);
			exit(IPIF_EXIT);
		}

// increase count
// i think that the maximal command size is 12 bytes long
// if it is longer than 12 bytes i cut it 12 Bytes
// i think so because normally the formular 
// (size of Command-Bytes + size of Answer-Bytes + 2 for the 2 size Bytes) for the
// command size works pretty fine, but it dont work together with the EEprom Write CMD

		device++;	// only one command per controller =)
		count += cmd[0] + (cmd[1] & 0x3f) + 2;	// size of Command-Bytes + size of Answer-Bytes + 2 for the 2 size Bytes
	}

// write answer packet to pi_ram
	//bufin[63] = 1;
	bufin[63] = 0;		// Set the last bit is 0 as successfully return
	for(i=0;i<64;i++)
	{
		//mem.pi_ram[i^3] = bufin[i];
        PIF[(PIF_RAM_PHYS+i)^3] = bufin[i];

	}

#ifdef LOG_PIF
	LogPIFData(bufin, FALSE);
#endif
}



//////////////////////////////////////////////////////////////////////////////
// Log PIF Data																//
//////////////////////////////////////////////////////////////////////////////
void LogPIFData(char *data, BOOL input)
{
	FILE *stream;
	stream = fopen("d:/pif_data.txt", "at");
	if (stream != NULL)
	{
		int i,j;
		unsigned char *p = data;
		if (input) fprintf(stream,"\nIncoming\n");
		else fprintf(stream,"\nOutgoing\n");
		for (i=0; i<8; i++)
		{
			for (j=0; j<8; j++)
			{
				fprintf(stream,"%02x ",*p);
				p++;
			}
			fprintf(stream,"\n");
		}
		fclose(stream);
	}
}
