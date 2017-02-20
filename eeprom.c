#include <windows.h>
#include <stdio.h>
#include "globals.h"
#include "n64rcp.h"
#include "iPIF.h"

static HANDLE hEepromFile = NULL;
BYTE EEPROM[0x800];

void CloseEeprom (void)
{
}





void Process_PifRam()
{
    static uint8* PIF_Ram;
    static uint8* PifRom;

	PIF_Ram= (uint8*)&PIF[PIF_RAM_PHYS];
	PifRom = (uint8*)&PIF[0];

    switch (PIF_Ram[0x3f])
	{
	    case 1:
		    if (*(DWORD *)&PIF_Ram[0] != 0 )
			{
			}
			else
			{            
				//check
        
				if(PIF_Ram[4]==0xFF&&PIF_Ram[5]==0x01&&PIF_Ram[6]==0x03)
				{
					PIF_Ram[8]=0x00;
					PIF_Ram[9]=0x80;
	                PIF_Ram[10]=0x00;
		            return;
				}	
			}
			break;

		case 0x08: 
			PIF_Ram[0x3F] = 0; 
			break;

		case 0x10:
			memset(PifRom,0,0x7C0); 
			break;

		case 0x30:
			PIF_Ram[0x3F] = 0x80;   
			break;

		case 0xC0:
			memset(PIF_Ram,0,0x40); 
			break;

		default:
			DisplayError("Process_PIF_Ram failed %X",PIF_Ram[0x3F]);
    };
}