/*______________________________________________________________________________
 |                                                                              |
 |  1964 - controller.h                                                         |
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
 |  To contact the author:  Schibo and Rice                                     |
 |  email      : schibo@emulation64.com, dyangchicago@yahoo.com                 |
 |  paper mail :                                                                |
 |______________________________________________________________________________|
*/

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