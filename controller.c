//---------------------------------------------------------------------------------------
// controller.c
//---------------------------------------------------------------------------------------

// 09/24/99 - gerrit - i've completely rewritten the controller code to fix some
// weirdness and deal with eeproms


#include <windows.h>
#include "controller.h"
#include "globals.h"
#include "win32\DLL_Input.h"

//---------------------------------------------------------------------------------------

#define true 1
#define false 0

uint32	controllerStatus = (CONT_ABSOLUTE | CONT_JOYPORT | CONT_NOMEMCARD);
uint32	controllerAvailable[4] = {true, true, false, false};

unsigned char	gSram[1024];
unsigned long	gSramUsed = false;		// flag for eeprom access

//---------------------------------------------------------------------------------------

//FILE * filepointer;
void CheckControllers(void)
{
	uint32	curController = 0;
	uint32	*pifWord = (uint32*)(uint8*)&PIF[PIF_RAM_PHYS];
	uint32	c, code, recMask;
	uint8	type;

	do {
		// read the next pif code
		code = (*pifWord);

		// process the pif code
		if ((code >> 24) == 0xFE) {
			// last command - exit
			break;
		} else if (code == 0x00000000) {

			uint8	size, read, offset;
			code = (*(++pifWord));
			// eeprom command
			size   = (uint8)(code >> 24);
			read   = (uint8)(code >> 16);
			type   = (uint8)(code >> 8);
			offset = (uint8)((code & 0xFF) * 8);
			if (size == 0xFF) {
				// write eeprom status
				if ((read == 1) && (type <= 4)) {
					if (type >= 4)
						recMask = 0;
					else
						recMask = (0xFFFFFFFF >> (type * 8));
					(*(++pifWord)) &= recMask;
//					(*pifWord) |= (kPIF_eepromAvailable & (~recMask)); //if eeprom is working, uncomment me!!
				} else {
					(*pifWord++) |= kPIFErr_badRecValue;
					DisplayError("Bad eeprom");
				}
				goto exit;
			} else if (type == RD_EEPROM) {
				DisplayError("Read from eeprom");
				// read from eeprom
				if (size == 2) {
					uint8	*dstPtr = (uint8*)(pifWord + 1);

					for (c = 0; c < read; c++)
					{
						(*dstPtr++) = gSram[offset + c];
					}

					(uint8*)pifWord += read;
				} else {
					// corrupt command
					(*pifWord++) |= kPIFErr_badRecValue;
				}

				goto exit;
				
			} else if (type == WR_EEPROM) {
				DisplayError("Write to eeprom");
				// write to eeprom
				gSramUsed = true;	// flag that the sram was used
				// copy data to eeprom
				if ((read == 1) && (size >= 2)) {
					uint8		*srcPtr = (uint8*)(pifWord + 1);
					for (c = 0; c < (size - 2); c++)
						gSram[offset + c] = (*srcPtr++);

					(uint8*)pifWord += size;
				} else {
					// corrupt command
					(*pifWord++) |= kPIFErr_badRecValue;
				}
				goto exit;
			} else {
				goto exit;
			}
			pifWord++;			// move to next pif code
		} else {
			uint32	cmd, send, rec;
			// standard controller command
			cmd  = (uint32)(uint8)(code >> 24);
			send = (uint32)(uint8)(code >> 16);
			rec  = (uint32)(uint8)(code >> 8);
			type = (uint32)(uint8)(code & 0xFF);
			// only process available controllers
			if (controllerAvailable[curController] == true) {
				// process by code type
				switch(type) {
					case GET_STATUS:
						// validate the receive size
						if (rec > 3) {
							(*pifWord) |= kPIFErr_badRecValue;
							rec = 3;
						}
						// write controller status
						if (rec >= 4) //x86 shr range is 0-31! if (rec*8)==32, we need this bandaid. ppc does not need this check.
							recMask = 0;	
						else
							recMask = (0xFFFFFFFF >> (rec * 8));
						(*(++pifWord)) &= recMask;
						(*pifWord) |= (controllerStatus & (~recMask));
						break;
					case RD_BUTTONS:
						// validate the receive size
						if (rec > 4) {
							(*pifWord) |= kPIFErr_badRecValue;
							rec = 4;
						}
						// write the button states
						if (rec >= 4)
							recMask = 0;
						else
							recMask = (0xFFFFFFFF >> (rec * 8));
						(*(++pifWord)) &= recMask;
						(*pifWord) |= (INPUT_CheckController(curController) & (~recMask));
						break;
					case RESET_CONT:
						// just ignore controller resets
						pifWord++;
						break;
					case RD_MEMPACK:
						// temp hack to work around unsupported commands
						//printf("PIF: ignoring unsupported mempack command\n");
//						(*pifWord) |= kPIFErr_noMemPack;
						goto exit;
						break;
					case WR_MEMPACK:
						// temp hack to work around unsupported commands
						//printf("PIF: ignoring unsupported mempack command\n");
//						(*pifWord) |= kPIFErr_noMemPack;
						goto exit;
						break;
					default:
//						DisplayError("PIF: error - unsupported controller command");
						(*pifWord) |= kPIFErr_noMemPack;	// not sure about this...
						goto exit;
						break;
				}
			} else {
				// no controller plugged in
				(*pifWord++) |= kPIFErr_noController;
//				DisplayError("No controller in port %d", curController+1);
			}
			pifWord++;			// move to next pif code
		}
		curController++;		// move to next controller
	} while (curController < 8);

exit:
#ifdef BIG_ENDIAN
	gHardwareState.pif[PIF_RAM_PHYS + 63] = 0;
#else
	PIF[PIF_RAM_PHYS + 60] = 0;
#endif
}
