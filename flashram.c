/*______________________________________________________________________________
 |                                                                              |
 |  1964 - flashram.c                                                           |
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
 |                                                                              |
 |  This implementation of flashram is according to the informatin provided     |
 |  by F|RES and Icepir8, thanks them very much to make this great information. |
 |______________________________________________________________________________|
*/

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "options.h"
#include "globals.h"
#include "r4300i.h"
#include "n64rcp.h"
#include "flashram.h"
#include "hardware.h"
#include "memory.h"
#include "iPIF.h"
#include "1964ini.h"
#include "win32/windebug.h"
#include "debug_option.h"

extern BOOL FlashRamUsed;
extern FileIO_ReadFLASHRAM();


#ifdef DEBUG_COMMON
#define FLASHRAM_HANDLE_ERROR	\
		DisplayError("Flashram error: current state=%d, event=%08x", flashram_state, event);	\
		flashram_state = FLASHRAM_IDLE_STATE;
#else
#define FLASHRAM_HANDLE_ERROR	flashram_state = FLASHRAM_IDLE_STATE;
#endif

// Flashram state definition

enum FLASHRAM_STATE 
{ 
	FLASHRAM_IDLE_STATE,
	FLASHRAM_ERASE_COMMAND_1_STATE,
	FLASHRAM_ERASE_COMMAND_2_STATE,
	FLASHRAM_ERASE_READ_STATUS_1_STATE,
	FLASHRAM_ERASE_READ_STATUS_2_STATE,
	FLASHRAM_WRITE_INIT_STATE,
	FLASHRAM_WRITE_SET_PAGE_STATE,
	FLASHRAM_WRITE_READ_STATUS_1_STATE,
	FLASHRAM_WRITE_READ_STATUS_2_STATE,
	FLASHRAM_GET_STATUS_INIT_STATE,
	FLASHRAM_GET_STATUS_1_STATE,
	FLASHRAM_READ_INIT_STATE,
	FLASHRAM_READ_GET_STATUS_1_STATE,
	FLASHRAM_READ_GET_STATUS_2_STATE
};


#define	FLASHRAM_READ_STATUS_WORD		0
#define	FLASHRAM_ERASE_BLOCK_COMMAND_1	0x4B00
#define	FLASHRAM_ERASE_BLOCK_COMMAND_2	0x7800
#define	FLASHRAM_EXECUTE_COMMAND		0xd200
#define	FLASHRAM_INIT_WRITE_COMMAND		0xb400
#define	FLASHRAM_WRITE_SET_PAGE_COMMAND 0xa500
#define	FLASHRAM_GET_STATUS_COMMAND		0xe100
#define	FLASHRAM_INIT_READ_COMMAND		0xf000

#define	FLASHRAM_STATUS_WORD_VALUE_1_ERASE		0x11118008
#define FLASHRAM_STATUS_WORD_VALUE_2_ERASE		0x00c20000
#define	FLASHRAM_STATUS_WORD_VALUE_1_WRITE		0x11118004
#define FLASHRAM_STATUS_WORD_VALUE_2_WRITE		0x00c20000
#define	FLASHRAM_STATUS_WORD_VALUE_1_GET_STATUS	0x11118001
#define FLASHRAM_STATUS_WORD_VALUE_2_GET_STATUS	0x00c20000
#define	FLASHRAM_STATUS_WORD_VALUE_1_READ		0x11118004
#define	FLASHRAM_STATUS_WORD_VALUE_2_READ		0xf0000000

static enum FLASHRAM_STATE flashram_state;
static uint32 flashram_status_reg_word1;
static uint32 saved_flashram_dword_1;
static BOOL reading_mode;
static int flashram_pageno;

BOOL flashram_state_machine(uint32 event);

void Flashram_Init(void)
{
	flashram_state = FLASHRAM_IDLE_STATE;
	
	flashram_status_reg_word1 = FLASHRAM_STATUS_WORD_VALUE_1_GET_STATUS;
//	FLASHRAM_STATUS_REG_1 = FLASHRAM_STATUS_WORD_VALUE_1_GET_STATUS;
	saved_flashram_dword_1 = 0;

	flashram_pageno = 0;
	reading_mode = FALSE;
}

void Flashram_Command(unsigned __int32 val)
{
#ifdef DEBUG_COMMON
	if( debug_sram )
	{
		TRACE1("Execute FlashRAM command %08X", val)
	}
#endif
	flashram_state_machine((uint32)val);
}

unsigned __int32 Flashram_Get_Status(void)
{
	if( reading_mode )
	{
		return FLASHRAM_STATUS_REG_1;
	}

	if( flashram_state_machine(FLASHRAM_READ_STATUS_WORD) )
	{
#ifdef DEBUG_COMMON
		if( debug_sram)
		{
			TRACE1("Read FlashRAM status register = %08X", flashram_status_reg_word1);
		}
#endif
		return flashram_status_reg_word1;
	}
	else
	{
#ifdef DEBUG_COMMON
		if( debug_sram )
		{
			TRACE1("Read FlashRAM status register invalid = %08X", FLASHRAM_STATUS_WORD_VALUE_2_GET_STATUS);
		}
#endif
		return FLASHRAM_STATUS_WORD_VALUE_2_GET_STATUS;
	}
}


// Service routines of each state
BOOL flashram_state_service_idle(uint32 event)		// State 0: FLASHRAM_IDLE_STATE
{
	switch( event>>16 )
	{
	case FLASHRAM_ERASE_BLOCK_COMMAND_1:
		flashram_pageno = (__int16)event;
		if( flashram_pageno >= 1024 )
		{
			DisplayError("Invalid flashram page number %d", flashram_pageno);
			flashram_pageno %= 1024;
		}
		flashram_state = FLASHRAM_ERASE_COMMAND_1_STATE;
		break;
	case FLASHRAM_INIT_WRITE_COMMAND:
		flashram_state = FLASHRAM_WRITE_INIT_STATE;
		break;
	case FLASHRAM_GET_STATUS_COMMAND:
		flashram_state = FLASHRAM_GET_STATUS_INIT_STATE;
		FLASHRAM_STATUS_REG_1 = FLASHRAM_STATUS_WORD_VALUE_1_GET_STATUS;
		FLASHRAM_STATUS_REG_1 = FLASHRAM_STATUS_WORD_VALUE_1_GET_STATUS;
		break;
	case FLASHRAM_INIT_READ_COMMAND:
		flashram_state = FLASHRAM_READ_INIT_STATE;
		break;
	case FLASHRAM_READ_STATUS_WORD:
		flashram_status_reg_word1 = FLASHRAM_STATUS_WORD_VALUE_1_GET_STATUS;
		FLASHRAM_STATUS_REG_1 = FLASHRAM_STATUS_WORD_VALUE_1_ERASE;
		FLASHRAM_STATUS_REG_1 = FLASHRAM_STATUS_WORD_VALUE_2_ERASE;
		flashram_state = FLASHRAM_GET_STATUS_1_STATE;
		break;
	case FLASHRAM_EXECUTE_COMMAND:
		break;
	default:
		FLASHRAM_HANDLE_ERROR
		return FALSE;
	}
	return TRUE;
}

BOOL flashram_state_service_erase_1(uint32 event)	// State 1: FLASHRAM_ERASE_COMMAND_1_STATE
{
	switch( event>>16 )
	{
	case FLASHRAM_ERASE_BLOCK_COMMAND_2:
		flashram_state = FLASHRAM_ERASE_COMMAND_2_STATE;
		break;
	default:
		FLASHRAM_HANDLE_ERROR
		return FALSE;
	}
	return TRUE;
}

BOOL flashram_state_service_erase_2(uint32 event)	// State 2: FLASHRAM_ERASE_COMMAND_2_STATE
{
	switch( event>>16 )
	{
	case FLASHRAM_READ_STATUS_WORD:
		flashram_status_reg_word1 = FLASHRAM_STATUS_WORD_VALUE_1_ERASE;
		//FLASHRAM_STATUS_REG_1 = FLASHRAM_STATUS_WORD_VALUE_1_ERASE;
		flashram_state = FLASHRAM_ERASE_READ_STATUS_1_STATE;
		break;
	case FLASHRAM_EXECUTE_COMMAND:
		// Here to erase a page
		//DisplayError("To erase a flashram page %d", flashram_pageno);
		memset(FlashRAM+128*flashram_pageno, 0, 128);
		flashram_state = FLASHRAM_IDLE_STATE;
		break;
	default:
		FLASHRAM_HANDLE_ERROR
		return FALSE;
	}
	return TRUE;
}

BOOL flashram_state_service_erase_read_reg_1(uint32 event)	// State 3: FLASHRAM_ERASE_READ_STATUS_1_STATE
{
	switch( event>>16 )
	{
	case FLASHRAM_READ_STATUS_WORD:
		flashram_status_reg_word1 = FLASHRAM_STATUS_WORD_VALUE_2_ERASE;
		//FLASHRAM_STATUS_REG_1 = FLASHRAM_STATUS_WORD_VALUE_2_ERASE;
		flashram_state = FLASHRAM_ERASE_READ_STATUS_2_STATE;
		break;
	default:
		FLASHRAM_HANDLE_ERROR
		return FALSE;
	}
	return TRUE;
}

BOOL flashram_state_service_erase_read_reg_2(uint32 event)	// State 4: FLASHRAM_ERASE_READ_STATUS_2_STATE
{
	switch( event>>16 )
	{
	case FLASHRAM_EXECUTE_COMMAND:
		// Here to erase a page
		//DisplayError("To erase a flashram page %d", flashram_pageno);
		memset(FlashRAM+128*flashram_pageno, 0, 128);
		flashram_state = FLASHRAM_IDLE_STATE;
		break;
	default:
		FLASHRAM_HANDLE_ERROR
		return FALSE;
	}
	return TRUE;
}

BOOL flashram_state_service_write_init(uint32 event)	// State 5: FLASHRAM_WRITE_INIT_STATE
{
	switch( event>>16 )
	{
	case FLASHRAM_WRITE_SET_PAGE_COMMAND:
		flashram_pageno = (__int16)event;
		if( flashram_pageno >= 1024 )
		{
			DisplayError("Invalid flashram page number %d", flashram_pageno);
			flashram_pageno %= 1024;
		}
		flashram_state = FLASHRAM_WRITE_SET_PAGE_STATE;
		break;
	case FLASHRAM_INIT_WRITE_COMMAND:
		break;
	default:
		FLASHRAM_HANDLE_ERROR
		return FALSE;
	}
	return TRUE;
}

BOOL flashram_state_service_write_setpage(uint32 event)		// State 6: FLASHRAM_WRITE_SET_PAGE_STATE
{
	switch( event>>16 )
	{
	case FLASHRAM_READ_STATUS_WORD:
		flashram_status_reg_word1 = FLASHRAM_STATUS_WORD_VALUE_1_WRITE;
		//FLASHRAM_STATUS_REG_1 = FLASHRAM_STATUS_WORD_VALUE_1_WRITE;
		flashram_state = FLASHRAM_WRITE_READ_STATUS_1_STATE;
		break;
	case FLASHRAM_INIT_WRITE_COMMAND:
		break;
	default:
		FLASHRAM_HANDLE_ERROR
		return FALSE;
	}
	return TRUE;
}

BOOL flashram_state_service_write_read_reg_1(uint32 event)	// State 7: FLASHRAM_WRITE_READ_STATUS_1_STATE
{
	switch( event>>16 )
	{
	case FLASHRAM_READ_STATUS_WORD:
		flashram_status_reg_word1 = FLASHRAM_STATUS_WORD_VALUE_2_WRITE;
		//FLASHRAM_STATUS_REG_1 = FLASHRAM_STATUS_WORD_VALUE_2_WRITE;
		flashram_state = FLASHRAM_WRITE_READ_STATUS_2_STATE;
		break;
	default:
		FLASHRAM_HANDLE_ERROR
		return FALSE;
	}
	return TRUE;
}

BOOL flashram_state_service_write_read_reg_2(uint32 event)	// State 8: FLASHRAM_WRITE_READ_STATUS_2_STATE
{
	switch( event>>16 )
	{
	case FLASHRAM_READ_STATUS_WORD:
		flashram_status_reg_word1 = FLASHRAM_STATUS_WORD_VALUE_1_WRITE;
		//FLASHRAM_STATUS_REG_1 = FLASHRAM_STATUS_WORD_VALUE_1_WRITE;
		flashram_state = FLASHRAM_WRITE_READ_STATUS_1_STATE;
		break;
	case FLASHRAM_EXECUTE_COMMAND:
		// Here to write a page by copying the buffer page to the selected page
		//DisplayError("Write a flashram page %d", flashram_pageno);
		memcpy(FlashRAM+128*flashram_pageno, pLOAD_UBYTE_PARAM_2(0xA8000000), 128);
		flashram_state = FLASHRAM_IDLE_STATE;
		break;
	default:
		FLASHRAM_HANDLE_ERROR
		return FALSE;
	}
	return TRUE;
}

BOOL flashram_state_service_get_status_reg_init(uint32 event)	// State 9: FLASHRAM_GET_STATUS_INIT_STATE
{
	switch( event>>16 )
	{
	case FLASHRAM_READ_STATUS_WORD:
		flashram_status_reg_word1 = FLASHRAM_STATUS_WORD_VALUE_1_GET_STATUS;
//		FLASHRAM_STATUS_REG_1 = FLASHRAM_STATUS_WORD_VALUE_1_GET_STATUS;
		flashram_state = FLASHRAM_GET_STATUS_1_STATE;
		break;
	case FLASHRAM_EXECUTE_COMMAND:
		flashram_state = FLASHRAM_IDLE_STATE;
		break;
	case FLASHRAM_INIT_READ_COMMAND:
		flashram_state = FLASHRAM_READ_INIT_STATE;
		break;
	default:
		FLASHRAM_HANDLE_ERROR
		return FALSE;
	}
	return TRUE;
}

BOOL flashram_state_service_get_status_reg_1(uint32 event)		// State 10
{
	switch( event>>16 )
	{
	case FLASHRAM_READ_STATUS_WORD:
		flashram_status_reg_word1 = FLASHRAM_STATUS_WORD_VALUE_2_GET_STATUS;
		//FLASHRAM_STATUS_REG_1 = FLASHRAM_STATUS_WORD_VALUE_2_GET_STATUS;
		flashram_state = FLASHRAM_IDLE_STATE;
		break;
	case FLASHRAM_EXECUTE_COMMAND:
		flashram_state = FLASHRAM_IDLE_STATE;
		break;
	case FLASHRAM_INIT_READ_COMMAND:
		flashram_state = FLASHRAM_READ_INIT_STATE;
	case FLASHRAM_GET_STATUS_COMMAND:
		// Zelda 2 and Pokemon wants this, but not Paper Mario
		//FLASHRAM_STATUS_REG_1 = FLASHRAM_STATUS_WORD_VALUE_1_GET_STATUS;
		//FLASHRAM_STATUS_REG_2 = FLASHRAM_STATUS_WORD_VALUE_2_GET_STATUS;
		flashram_state = FLASHRAM_GET_STATUS_INIT_STATE;
		break;

	default:
		FLASHRAM_HANDLE_ERROR
		return FALSE;
	}
	return TRUE;
}

BOOL flashram_state_service_read_init(uint32 event)
{
	switch( event>>16 )
	{
	case FLASHRAM_READ_STATUS_WORD:
		flashram_status_reg_word1 = FLASHRAM_STATUS_WORD_VALUE_1_READ;
		//FLASHRAM_STATUS_REG_1 = FLASHRAM_STATUS_WORD_VALUE_1_READ;
		flashram_state = FLASHRAM_READ_GET_STATUS_1_STATE;
		reading_mode = TRUE;

		// Copy the FlashRAM to memory for DMA to read
		memcpy((pLOAD_UBYTE_PARAM_2(0xA8000000)), FlashRAM, 1024*128);
		
		break;
	case FLASHRAM_INIT_READ_COMMAND:
		break;
	default:
		reading_mode = FALSE;
		FLASHRAM_HANDLE_ERROR
		return FALSE;
	}
	return TRUE;
}

BOOL flashram_state_service_read_read_reg_1(uint32 event)
{
	switch( event>>16 )
	{
	case FLASHRAM_READ_STATUS_WORD:
		flashram_status_reg_word1 = FLASHRAM_STATUS_WORD_VALUE_2_READ;
		//FLASHRAM_STATUS_REG_1 = FLASHRAM_STATUS_WORD_VALUE_2_READ;

		// Copy the FlashRAM to memory for DMA to read
		memcpy((pLOAD_UBYTE_PARAM_2(0xA8000000)), FlashRAM, 1024*128);

		//flashram_state = FLASHRAM_READ_GET_STATUS_2_STATE;
		flashram_state = FLASHRAM_IDLE_STATE;
		break;
	case FLASHRAM_INIT_READ_COMMAND:
		break;
	case FLASHRAM_ERASE_BLOCK_COMMAND_1:
		reading_mode = FALSE;
		flashram_pageno = (__int16)event;
		if( flashram_pageno >= 1024 )
		{
			DisplayError("Invalid flashram page number %d", flashram_pageno);
			flashram_pageno %= 1024;
		}
		flashram_state = FLASHRAM_ERASE_COMMAND_1_STATE;
		break;
	default:
		reading_mode = FALSE;
		FLASHRAM_HANDLE_ERROR
		return FALSE;
	}
	return TRUE;
}

BOOL flashram_state_service_read_read_reg_2(uint32 event)
{
	switch( event>>16 )
	{
	default:
		FLASHRAM_HANDLE_ERROR
		return FALSE;
	}
	return TRUE;
}

BOOL (*flash_state_services[])(uint32 event) =
{
	flashram_state_service_idle,
	flashram_state_service_erase_1,
	flashram_state_service_erase_2,
	flashram_state_service_erase_read_reg_1,
	flashram_state_service_erase_read_reg_2,
	flashram_state_service_write_init,
	flashram_state_service_write_setpage,
	flashram_state_service_write_read_reg_1,
	flashram_state_service_write_read_reg_2,
	flashram_state_service_get_status_reg_init,
	flashram_state_service_get_status_reg_1,
	flashram_state_service_read_init,
	flashram_state_service_read_read_reg_1,
	flashram_state_service_read_read_reg_2
};

// The finite state machine
BOOL flashram_state_machine(uint32 event)
{
	return( flash_state_services[flashram_state](event) );
}

void DMA_Flashram_To_RDRAM(unsigned __int32 rdramaddr, unsigned __int32 flashramaddr, unsigned __int32 len)
{
	if( firstusedsavemedia == 0 )
		firstusedsavemedia = FLASHRAM_SAVETYPE;

	if( FlashRamUsed == FALSE )
	{
		FlashRamUsed = TRUE;
		FileIO_ReadFLASHRAM();
	}

	if( flashramaddr == 0xa8000000 )
	{
		flashram_state_service_idle(FLASHRAM_READ_STATUS_WORD);
	}
}

void DMA_RDRAM_To_Flashram(unsigned __int32 rdramaddr, unsigned __int32 flashramaddr, unsigned __int32 len)
{
	if( firstusedsavemedia == 0 )
		firstusedsavemedia = FLASHRAM_SAVETYPE;

	if( FlashRamUsed == FALSE )
	{
		FlashRamUsed = TRUE;
		FileIO_ReadFLASHRAM();
	}
}

