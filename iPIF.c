/*$T iPIF.c GC 1.136 03/09/02 17:42:34 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Peripheral interface. Controller handled here, including mempak, flashram access, etc.
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


/*
 * 1964 Copyright (C) 1999-2002 Joel Middendorf, <schibo@emulation64.com> This
 * program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version. This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. To contact the
 * authors: email: schibo@emulation64.com, rice1964@yahoo.com
 */
#include <windows.h>
#include <stdio.h>
#include "globals.h"
#include "n64rcp.h"
#include "hardware.h"
#include "memory.h"
#include "iPif.h"
#include "gamesave.h"
#include "fileio.h"
#include "debug_option.h"
#include "emulator.h"
#include "win32/windebug.h"
#include "win32/DLL_Input.h"
#include "kaillera/Kaillera.h"
#include "gamesave.h"
#include "netplay.h"

_u8 EEProm_Status_Byte = 0x00;

_u8 bufin[64];

/*
 =======================================================================================================================
    Init PIF //
 =======================================================================================================================
 */
void Init_iPIF(void)
{
	gamesave.mempak_used[0] = FALSE;
	gamesave.mempak_used[1] = FALSE;
	gamesave.mempak_used[2] = FALSE;
	gamesave.mempak_used[3] = FALSE;
	gamesave.Sram_used = FALSE;
	gamesave.FlashRamUsed = FALSE;
	gamesave.EEprom_used = FALSE;
	gamesave.firstusedsavemedia = 0;

	switch(currentromoptions.Eeprom_size)
	{
	case EEPROMSIZE_16KB:	
		EEProm_Status_Byte = 0xC0; 
		break;
	case EEPROMSIZE_4KB:	
		EEProm_Status_Byte = 0x80; 
		break;
	default:			 /* =EEPROMSIZE_NONE */
		EEProm_Status_Byte = 0x00; 
		break;
	};
}

/*
 =======================================================================================================================
    Close PIF //
 =======================================================================================================================
 */
void Close_iPIF(void)
{
	/*~~*/
	int i;
	/*~~*/

	/* write mempaks to file if it was in use */
	if
	(
		currentromoptions.Save_Type == MEMPAK_SAVETYPE
	||	(currentromoptions.Save_Type == FIRSTUSE_SAVETYPE && gamesave.firstusedsavemedia == MEMPAK_SAVETYPE)
	||	currentromoptions.Save_Type == ANYUSED_SAVETYPE
	)
	{
		for(i = 0; i < 4; i++)
		{
			if(gamesave.mempak_used[i] && gamesave.mempak_written[i])
			{
				FileIO_WriteMemPak(i);
				gamesave.mempak_used[i] = FALSE;
				gamesave.mempak_written[i] = FALSE;
			}
		}
	}

	/* write eeprom to file if it was in use */
	if
	(
		currentromoptions.Save_Type == EEPROM_SAVETYPE
	||	(currentromoptions.Save_Type == FIRSTUSE_SAVETYPE && gamesave.firstusedsavemedia == EEPROM_SAVETYPE)
	||	currentromoptions.Save_Type == ANYUSED_SAVETYPE
	)
	{
		if(gamesave.EEprom_used && gamesave.EEprom_written)
		{
			FileIO_WriteEEprom();
			gamesave.EEprom_used = FALSE;
			gamesave.EEprom_written = FALSE;
		}
	}

	if
	(
		currentromoptions.Save_Type == SRAM_SAVETYPE
	||	(currentromoptions.Save_Type == FIRSTUSE_SAVETYPE && gamesave.firstusedsavemedia == SRAM_SAVETYPE)
	||	currentromoptions.Save_Type == ANYUSED_SAVETYPE
	)
	{
		if(gamesave.Sram_used)
		{
			FileIO_WriteSRAM();
			gamesave.Sram_used = FALSE;
		}
	}

	if
	(
		currentromoptions.Save_Type == FLASHRAM_SAVETYPE
	||	currentromoptions.Save_Type == SRAM_SAVETYPE
	||	(currentromoptions.Save_Type == FIRSTUSE_SAVETYPE && gamesave.firstusedsavemedia == FLASHRAM_SAVETYPE)
	||	currentromoptions.Save_Type == ANYUSED_SAVETYPE
	)
	{
		if(gamesave.FlashRamUsed)
		{
			FileIO_WriteFLASHRAM();
			gamesave.FlashRamUsed = FALSE;
		}
	}

	/* Check #define MEMORY_SIZE_C2A1 0x8000 */
	if(gMS_C2A1)
	{
		/*~~~~~~~~~~~~~~~~*/
		int i, modified = 0;
		/*~~~~~~~~~~~~~~~~*/

		for(i = 0; i < MEMORY_SIZE_C2A1 / 4; i++)
		{
			if(gMS_C2A1[i] != 0) modified++;
		}

		if(modified > 0) TRACE0("C2A1 is used");
	}

	if(gMS_C1A1)
	{
		/*~~~~~~~~~~~~~~~~*/
		int i, modified = 0;
		/*~~~~~~~~~~~~~~~~*/

		for(i = 0; i < MEMORY_SIZE_C1A1 / 4; i++)
			if(gMS_C1A1[i] != 0) modified++;
		if(modified > 0) TRACE0("C1A1 is used");
	}

	if(gMS_C2A2)
	{
		/*~~~~~~~~~~~~~~~~*/
		int i, modified = 0;
		/*~~~~~~~~~~~~~~~~*/

		for(i = 0; i < MEMORY_SIZE_C2A2 / 4; i++)
			if(gMS_C2A2[i] != 0) modified++;
		if(modified > 0) TRACE0("C2A2 is used");
	}

	if(gMS_C1A3)
	{
		/*~~~~~~~~~~~~~~~~*/
		int i, modified = 0;
		/*~~~~~~~~~~~~~~~~*/

		for(i = 0; i < MEMORY_SIZE_C1A3 / 4; i++)
			if(gMS_C1A3[i] != 0) modified++;
		if(modified > 0) TRACE0("C1A3 is used");
	}
}

/*
 =======================================================================================================================
    Build CRC for PAK
 =======================================================================================================================
 */
void BuildCRC(_u8 *data, _u8 *crc)
{
	/*~~~~~~~~~~*/
	_u8 tmp, tmp2;
	int i, j;
	/*~~~~~~~~~~*/

	tmp = 0;
	tmp2 = 0;
	for(i = 0; i <= 32; i++)
	{
		for(j = 7; j >= 0; j--)
		{
			if(tmp & 0x80)
				tmp2 = 0x85;
			else
				tmp2 = 0x00;
			tmp <<= 1;
			if(i == 32)
				tmp |= 0;
			else if(data[i] & (1 << j))
				tmp |= 1;
			else
				tmp |= 0;
			tmp ^= tmp2;
		}
	}

	*crc = tmp;
}

/*
 =======================================================================================================================
    Read Data from MemPak
 =======================================================================================================================
 */
void ReadControllerPak(int device, char *cmd)
{
	uint8	crc;
	uint16	offset = *(_u16 *) &cmd[1];

	offset = (offset >> 8) | (offset << 8);
	offset = offset >> 5;

	if(offset <= 0x400)
	{
		if(!gamesave.mempak_used[device])
		{
			FileIO_LoadMemPak(device);
			gamesave.mempak_used[device] = TRUE;

			if(gamesave.firstusedsavemedia == 0)
			{
				gamesave.firstusedsavemedia = MEMPAK_SAVETYPE;
			}
		}
		memcpy(&cmd[3], &(gamesave.mempak[device][offset * 32]), 32);
	}

	BuildCRC(&cmd[3], &crc);	/* Build CRC of the Data */
	cmd[35] = crc;
}

/*
 =======================================================================================================================
    Write Data to MemPak
 =======================================================================================================================
 */
void WriteControllerPak(int device, char *cmd)
{
	uint8	crc;
	uint16	offset = *(_u16 *) &cmd[1];

	offset = (offset >> 8) | (offset << 8);
	offset = offset >> 5;

	if(offset <= 0x400)
	{
		if(!gamesave.mempak_used[device])
		{
			FileIO_LoadMemPak(device);
			gamesave.mempak_used[device] = TRUE;

			if(gamesave.firstusedsavemedia == 0)
			{
				gamesave.firstusedsavemedia = MEMPAK_SAVETYPE;
			}
		}

		/* Copy Data to Mempak */
		memcpy(&(gamesave.mempak[device][offset * 32]), &cmd[3], 32);
		gamesave.mempak_written[device] = TRUE;
		FileIO_WriteMemPak(device);		//Save the file to disk
	}

	BuildCRC(&cmd[3], &crc);	/* Build CRC of the Data */
	cmd[35] = crc;
}

/*
 =======================================================================================================================
    Handles all Commands which are sent to a Controller
 =======================================================================================================================
 */
BOOL ControllerCommand(_u8 *cmd, int device)
{
	emustatus.ControllerReadCount++;

	if(Kaillera_Is_Running == TRUE)
	{
		// Need only the first device for kaillera mode cause this is the only device we really use ;)
		if(!Controls[0].Present)
		{
			cmd[1] |= 0x80;
			cmd[3] = 0xFF;
			cmd[4] = 0xFF;
			cmd[5] = 0xFF;
			return TRUE;
		}
	}
	else
	{
		if(!Controls[device].Present)
		{
			cmd[1] |= 0x80;
			cmd[3] = 0xFF;
			cmd[4] = 0xFF;
			cmd[5] = 0xFF;
			return TRUE;
		}
	}

	switch(cmd[2])
	{
	/* Get Controller status */
	case 0xFF:				/* 0xFF could be something like Reset Controller and return the status */
	case 0x00:				/* 0x00 return the status */
		cmd[3] = 0x05;		/* for Adaptoid only */
		cmd[4] = 0x00;		/* for Adaptoid only */
		if
		(
			(Controls[device].Plugin & PLUGIN_MEMPAK)
		&&	(currentromoptions.Save_Type == MEMPAK_SAVETYPE || currentromoptions.Save_Type == ANYUSED_SAVETYPE)
		) cmd[5] = 0x01;
		else
			cmd[5] = 0x00;	/* no mempack - reversed fir Adaptoid only (Bit 0x01 would be rumble-pack) */
		break;

	/* Read Controller Data ... need a DInput interface first =) */
	case 0x01:
		{
			/*~~~~~~~~~*/
			BUTTONS Keys;
			/*~~~~~~~~~*/

			if(Kaillera_Is_Running)
			{
				/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
				typedef struct
				{
					unsigned int	c;
					BUTTONS			b;
				} kbuffer;
				static kbuffer	kBuffers[8];
				int				reclen;
				int				internal_counter = 0;
				/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

				CONTROLLER_GetKeys(0, &Keys);

label_Jump:
				if(internal_counter > 100)
				{
					MessageBox(NULL, "internal_counter reached !!!", "Error", 0);
					Kaillera_Is_Running = FALSE;
				}

				memcpy(&kBuffers[0].b, &Keys, sizeof(BUTTONS));
				kBuffers[0].c = Kaillera_Counter;
				reclen = kailleraModifyPlayValues((void *) kBuffers, sizeof(kbuffer));

				if(reclen == -1)
				{
					MessageBox(NULL, "Kaillera timeout", "Error", 0);
					Kaillera_Is_Running = FALSE;
				}
				else if(reclen > 0)
				{
					int i;
					for(i = 0; i < Kaillera_Players; i++)
					{
						if(kBuffers[i].c != Kaillera_Counter)	
						{
							/* This synchronizes all players */
							/* but could make game play really slow */
							goto label_Jump;
						}
					}
				}
				else
					goto label_Jump;

				Kaillera_Counter++;
				internal_counter++;

				memcpy(&Keys, &kBuffers[device].b, sizeof(BUTTONS));
			}
			else
			{
				if( NetplayInitialized )
				{
					netplay_get_keys(device, &Keys, emustatus.DListCount);
				}
				else
					CONTROLLER_GetKeys(device, &Keys);
			}

			*(DWORD *) &cmd[3] = *(DWORD *) &Keys;
			DEBUG_CONTROLLER_TRACE(TRACE2("Read controller %d, return %X", device, *(DWORD *) &Keys););
		}
		break;

	/* Read Controller Pak */
	case 0x02:
		switch(Controls[device].Plugin)
		{
		case PLUGIN_MEMPAK:
			ReadControllerPak(device, &cmd[2]);
			break;
		case PLUGIN_NONE:
		case PLUGIN_RUMBLE_PAK:
		case PLUGIN_TANSFER_PAK:
		default:
			break;
		}

		return FALSE;
		break;

	/* Write Controller Pak */
	case 0x03:
		switch(Controls[device].Plugin)
		{
		case PLUGIN_MEMPAK:
			WriteControllerPak(device, &cmd[2]);
			break;
		case PLUGIN_NONE:
		case PLUGIN_RUMBLE_PAK:
		case PLUGIN_TANSFER_PAK:
		default:
			break;
		}

		return FALSE;
		break;

	default:
		TRACE2("Unkown ControllerCommand %X, pc=%08X", cmd[2], gHWS_pc);
#ifdef DEBUG_COMMON
		{
			/*~~~~~~~~~~~~~~~~~~*/
			char	line[100];
			char	*pline = line;
			int		i;
			/*~~~~~~~~~~~~~~~~~~*/

			for(i = 0; i < 64; i++)
			{
				sprintf(pline, "%02X ", cmd[i]);
				pline += 3;
				if(i % 8 == 7)
				{
					pline = line;
					TRACE1("%s", pline);
				}
			}
		}
#endif
		DisplayError("Unknown ControllerCommand %x, pc=%08X, device=%d", cmd[2], gHWS_pc, device);

		/* Treat this as GET_STATUS */
		cmd[3] = 0x05;		/* for Adaptoid only */
		cmd[4] = 0x00;		/* for Adaptoid only */
		if
		(
			(Controls[device].Plugin & PLUGIN_MEMPAK)
		&&	(currentromoptions.Save_Type == MEMPAK_SAVETYPE || currentromoptions.Save_Type == ANYUSED_SAVETYPE)
		) cmd[5] = 0x01;
		else
			cmd[5] = 0x00;	/* no mempack - for Adaptoid only (Bit 0x01 would be rumble-pack) */

		/* exit(IPIF_EXIT); */
		break;
	}

	return TRUE;
}

/*
 =======================================================================================================================
    Read EEprom (loads EEprom from file if it is present)
 =======================================================================================================================
 */
void ReadEEprom(char *dest, long offset)
{
#ifdef DEBUG_SI_EEPROM
	if(debugoptions.debug_si_eeprom)
	{
		TRACE0("Read from EEPROM");
	}
#endif

	if(!gamesave.EEprom_used)
	{
		FileIO_LoadEEprom();
		gamesave.EEprom_used = TRUE;

		if(gamesave.firstusedsavemedia == 0)
		{
			gamesave.firstusedsavemedia = EEPROM_SAVETYPE;
		}
	}

	memcpy(dest, &gamesave.EEprom[offset], 8);
}

/*
 =======================================================================================================================
    Writes to EEprom
 =======================================================================================================================
 */
void WriteEEprom(char *src, long offset)
{
#ifdef DEBUG_SI_EEPROM
	if(debugoptions.debug_si_eeprom)
	{
		TRACE0("Write to EEPROM");
	}
#endif

	if(!gamesave.EEprom_used)
	{
		FileIO_LoadEEprom();
		gamesave.EEprom_used = TRUE;

		if(gamesave.firstusedsavemedia == 0)
		{
			gamesave.firstusedsavemedia = EEPROM_SAVETYPE;
		}
	}

	memcpy(&gamesave.EEprom[offset], src, 8);
	FileIO_WriteEEprom();	//Write the changes to disk
	gamesave.EEprom_written = TRUE;
}

/*
 =======================================================================================================================
    Handles all Commands which are sent to the EEprom
 =======================================================================================================================
 */
BOOL EEpromCommand(_u8 *cmd, int device)
{
	switch(cmd[2])
	{
	/* reporting eeprom state ... hmmm */
	case 0xFF:
	case 0x00:
#ifdef DEBUG_SI_EEPROM
		if(debugoptions.debug_si_eeprom)
		{
			TRACE0("Execute EEPROM GetStatis Commands");
		}
#endif
		cmd[3] = 0x00;
		cmd[4] = EEProm_Status_Byte;
		cmd[5] = 0x00;
		break;

	/* Read from Eeprom */
	case 0x04:
		ReadEEprom(&cmd[4], cmd[3] * 8);
		break;

	/* Write to Eeprom */
	case 0x05:
		WriteEEprom(&cmd[4], cmd[3] * 8);
		break;

	default:
		break;
	}

	return FALSE;
}

/*
 =======================================================================================================================
    Decode the PIF Rom Data
 =======================================================================================================================
 */
void iPifCheck(void)
{
	/*~~~~~~~~~~~~~~~~~*/
	int i, count, device;
	/*~~~~~~~~~~~~~~~~~*/

	for(i = 0; i < 64; i++)
	{
		/* bufin[i]=mem.pi_ram[i^3]; */
		bufin[i] = gMS_PIF[(PIF_RAM_PHYS + i) ^ 3];
	}

#ifdef LOG_PIF
	LogPIFData(bufin, TRUE);
#endif
	count = 0;
	device = 0;
	while(count < 64)
	{
		/*~~~~~~~~~~~~~~~~~~~~~*/
		_u8 *cmd = &bufin[count];
		/*~~~~~~~~~~~~~~~~~~~~~*/

		/* Command Block is ready */
		if(cmd[0] == 0xFE) break;

		/*
		 * no-op Commands £
		 * FD is from Command and Conquer
		 */
		if((cmd[0] == 0xFF) || (cmd[0] == 0xFD))
		{
			count++;
			continue;
		}

		/* Next Device */
		if(cmd[0] == 0x00)
		{
			count++;
			device++;
			continue;
		}

		/* Device Channel to large (0-3 = Controller, 4 = EEprom) */
		if(device > 4) break;

		/* We get a Real Command now =) */
		switch(device)
		{
		/* Controler Command */
		case 0:
		case 1:
		case 2:
		case 3:
			if(Controls[device].RawData)
			{
				CONTROLLER_ControllerCommand(device, cmd);
				CONTROLLER_ReadController(device, cmd);
				break;
			}
			else
			{
				if(!ControllerCommand(cmd, device))
				{
					count = 64;
				}
			}
			break;

		/* EEprom Command */
		case 4:
			if(!EEpromCommand(cmd, device))
			{
				count = 64;
			}
			break;

		default:
			DisplayError("Unknown Command for unknwon Device %x", device);
			exit(IPIF_EXIT);
		}

#ifdef DEBUG_COMMON
		switch(cmd[2])
		{
		case 0x00:
		case 0xFF:
			if(debugoptions.debug_si_controller)
			{
				sprintf(tracemessage, "Get Status: %02X %02X %02X %02X %02X %02X %02X",
						cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6]); 
				RefreshOpList(tracemessage);
			}
			break;
		case 0x01:
			if(debugoptions.debug_si_controller)
			{
				sprintf(tracemessage, "Read Controller: %02X %02X %02X %02X %02X %02X %02X",
						cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6]); 
				RefreshOpList(tracemessage);
			}
			break;
		case 0x02:
			if(debugoptions.debug_si_mempak)
			{
				sprintf(tracemessage, "Read Mempak: %02X %02X %02X %02X %02X %02X %02X",
						cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6]); 
				RefreshOpList(tracemessage);
			}
			break;
		case 0x03:
			if(debugoptions.debug_si_mempak)
			{
				sprintf(tracemessage, "Write Mempak: %02X %02X %02X %02X %02X %02X %02X",
						cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6]); 
				RefreshOpList(tracemessage);
			}
			break;
		}
#endif

		/*
		 * increase count £
		 * i think that the maximum command size is 12 bytes long £
		 * if it is longer than 12 bytes i cut it 12 Bytes £
		 * i think so because normally the formula £
		 * (size of Command-Bytes + size of Answer-Bytes + 2 for the 2 size Bytes) for the £
		 * command size works pretty fine, but it doesn't work together with the EEprom
		 * Write CMD
		 */
		device++;	/* only one command per controller =) */
		count += cmd[0] +
		(cmd[1] & 0x3f) +
		2;			/* size of Command-Bytes + size of Answer-Bytes + 2 for the 2 size Bytes */
	}

	if(Controls[0].RawData)
	{
		CONTROLLER_ControllerCommand(-1, bufin);	/* 1 signalling end of processing the pif ram. */
	}

	/*
	 * write answer packet to pi_ram £
	 * bufin[63] = 1;
	 */
	bufin[63] = 0;	/* Set the last bit is 0 as successfully return */
	for(i = 0; i < 64; i++)
	{
		/* mem.pi_ram[i^3] = bufin[i]; */
		gMS_PIF[(PIF_RAM_PHYS + i) ^ 3] = bufin[i];
	}

#ifdef LOG_PIF
	LogPIFData(bufin, FALSE);
#endif
}

/*
 =======================================================================================================================
    Log PIF Data
 =======================================================================================================================
 */
void LogPIFData(char *data, BOOL input)
{
	FILE *stream = fopen("c:/pif_data.txt", "at");
	if(stream != NULL)
	{
		int				i, j;
		unsigned char	*p = data;

		if(input)
		{
			fprintf(stream, "\nIncoming\n");
		}
		else
		{
			fprintf(stream, "\nOutgoing\n");
		}

		for(i = 0; i < 8; i++)
		{
			for(j = 0; j < 8; j++)
			{
				fprintf(stream, "%02x ", *p);
				p++;
			}

			fprintf(stream, "\n");
		}

		fclose(stream);
	}
}
