//---------------------------------------------------------------------------------------
// controller.c
//---------------------------------------------------------------------------------------

// 09/24/99 - gerrit - i've completely rewritten the controller code to fix some
// weirdness and deal with eeproms

#include <windows.h>
#include "controller.h"
#include "globals.h"
#include "iPIF.h"
#include "hardware.h"
#include "win32\DLL_Input.h"

//---------------------------------------------------------------------------------------

#define true 1
#define false 0
#define NUM_CONTROLLERS 1

uint32  controllerStatus = (CONT_ABSOLUTE | CONT_JOYPORT | CONT_NOMEMCARD);
uint32  controllerAvailable[4] = {true, false, false, false};


extern void Process_PifRam();

void CheckControllers(void)
{
    uint32  curController = 0;
    uint32  *pifWord = (uint32*)(uint8*)&PIF[PIF_RAM_PHYS];
    uint32  code, recMask;
    uint32 type;

    do {
        // read the next pif code
        code = (*pifWord);

        // process the pif code
        if ((uint8)(code) == 0xFE)
		{
            // last command - exit
            break;
        }
		else if (code == 0x00000000)
		{
            Process_PifRam();
            ++pifWord;
        }
		else 
		{
            // standard controller command
            uint32  cmd, send, rec;

            cmd  = (uint32)(uint8)(code);
            send = (uint32)(uint8)(code >> 8);
            rec  = (uint32)(uint8)(code >> 16);
            type = (uint32)(uint8)(code >> 24);

            // only process available controllers
            if (controllerAvailable[curController] == true)
			{
                // process by code type
                switch(type) {
                    case GET_STATUS:
                        // validate the receive size
                        if (rec > 3)
						{
                            (*pifWord) |= kPIFErr_badRecValue;
                            rec = 3;
                        }
                        // write controller status
                        if (rec >= 4) //x86 shr range is 0-31! if (rec*8)==32, we need this bandaid. ppc does not need this check.
                            recMask = 0;    
                        else
                            recMask = (0xFFFFFFFF << (rec * 8));

                        (*(++pifWord)) &= recMask;
                        (*pifWord) |= (controllerStatus & (~recMask)); // represents a non-word32 swapped
                        break;

                    case RD_BUTTONS:
                        // validate the receive size
                        if (rec > 4)
						{
                            (*pifWord) |= kPIFErr_badRecValue;
                            rec = 4;
                        }

                        // write the button states
                        if (rec >= 4)
                            recMask = 0;
                        else
                            recMask = (0xFFFFFFFF >> (rec * 8));

                        (*(++pifWord)) &= recMask;
                        _asm {
                            mov edx, pifWord
                            mov eax, dword ptr [edx]
                            bswap eax
                            mov dword ptr [edx], eax
                        }

                        //(*pifWord) |= (INPUT_CheckController(curController) & (~recMask)); //represents a word32 swapped
                        _asm {
                            mov edx, pifWord
                            mov eax, dword ptr [edx]
                            bswap eax
                            mov dword ptr [edx], eax
                        }
                        break;

                    case RESET_CONT:
                        pifWord++;
                        break;

                    case RD_MEMPACK:
                        (*pifWord) |= kPIFErr_noMemPack;
                        goto exit;
                        break;

                    case WR_MEMPACK:
                        (*pifWord) |= kPIFErr_noMemPack;
                        goto exit;
                        break;

                    default:
                        (*pifWord) |= kPIFErr_noMemPack;    // not sure about this...
                        goto exit;
                        break;
                }
            }
			else 
			{
                // no controller plugged in
                (*pifWord++) |= kPIFErr_noController;
            }
            pifWord++;          // move to next pif code
        }
        curController++;        // move to next controller
    } while (curController < 2);

exit:
    PIF[PIF_RAM_PHYS + 63] = 0;
}