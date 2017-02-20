#ifndef _CONTROLLER_H__1964_
#define _CONTROLLER_H__1964_

//command types
#define GET_STATUS      0x00 
#define RD_BUTTONS      0x01
#define RD_MEMPACK      0x02
#define WR_MEMPACK      0x03
#define RD_EEPROM       0x04
#define WR_EEPROM       0x05
#define RESET_CONT      0xff

#define kPIFErr_noError         0x0000
#define kPIFErr_noController    0x8000
#define kPIFErr_noMemPack       0x8000
#define kPIFErr_badRecValue     0x4000

#define kPIF_eepromAvailable    0x00800000

//error codes
#define ERR_NONE        0x00    //no error
#define ERR_NOTPRESENT  0x80    //device not present
#define ERR_TOOBIG      0x40    //cannot send/recieve number of bytes for command

//controller status
#define CONT_MEMCARD    0x00000100
#define CONT_NOMEMCARD  0x00000200
#define CONT_ABSOLUTE   0x01000000
#define CONT_RELATIVE   0x02000000
#define CONT_JOYPORT    0x04000000
#define CONT_EEPROM     0x80000000

extern void CheckControllers(void);
extern void CloseEeprom (void);

#endif // _CONTROLLER_H__1964_