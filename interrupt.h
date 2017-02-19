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
*/

#ifndef _INTERRUPT_H__1964_
#define _INTERRUPT_H__1964_

#define MI_INTR_SP			0x00000001	
#define MI_INTR_SI			0x00000002	
#define MI_INTR_AI			0x00000004	
#define MI_INTR_VI			0x00000008
#define MI_INTR_PI			0x00000010	
#define MI_INTR_DP			0x00000020	

/* These are defined for speed */
#define NOT_MI_INTR_SP		0xFFFFFFFE	
#define NOT_MI_INTR_SI		0xFFFFFFFD	
#define NOT_MI_INTR_AI		0xFFFFFFFB	
#define NOT_MI_INTR_VI		0xFFFFFFF7
#define NOT_MI_INTR_PI		0xFFFFFFEF
#define NOT_MI_INTR_DP		0xFFFFFFDF	



#define MI_INTR_MASK_SP_CLR		0x01	
#define MI_INTR_MASK_SI_CLR		0x04		
#define MI_INTR_MASK_AI_CLR		0x10	
#define MI_INTR_MASK_VI_CLR		0x40		
#define MI_INTR_MASK_PI_CLR		0x100		
#define MI_INTR_MASK_DP_CLR		0x400

#define MI_INTR_MASK_SP_SET		0x02	
#define MI_INTR_MASK_SI_SET		0x08		
#define MI_INTR_MASK_AI_SET		0x20	
#define MI_INTR_MASK_VI_SET		0x80		
#define MI_INTR_MASK_PI_SET		0x200		
#define MI_INTR_MASK_DP_SET		0x800

// values read from the mi mode register
#define MI_MODE_INIT			0x0080
#define MI_MODE_EBUS			0x0100
#define MI_MODE_RDRAM			0x0200

// values written to the mi mode register
#define MI_CLR_INIT				0x0080
#define MI_SET_INIT				0x0100
#define MI_CLR_EBUS				0x0200
#define MI_SET_EBUS				0x0400
#define MI_CLR_DP_INTR			0x0800
#define MI_CLR_RDRAM			0x1000
#define MI_SET_RDRAM			0x2000


/* SP_STATUS_REG read bits */
#define SP_STATUS_HALT			0x0001
#define SP_STATUS_BROKE			0x0002
#define SP_STATUS_DMA_BUSY		0x0004
#define SP_STATUS_DMA_FULL		0x0008
#define SP_STATUS_IO_FULL		0x0010
#define SP_STATUS_SSTEP			0x0020
#define SP_STATUS_INTR_BREAK	0x0040
#define SP_STATUS_YIELD			0x0080
#define SP_STATUS_YIELDED		0x0100
#define SP_STATUS_TASKDONE		0x0200
#define SP_STATUS_SIG3			0x0400
#define SP_STATUS_SIG4			0x0800
#define SP_STATUS_SIG5			0x1000
#define SP_STATUS_SIG6			0x2000
#define SP_STATUS_SIG7			0x4000

/* SP_STATUS_REG write bits */
#define SP_CLR_HALT				0x0000001
#define SP_SET_HALT				0x0000002
#define SP_CLR_BROKE			0x0000004

#define SP_CLR_INTR				0x0000008
#define SP_SET_INTR				0x0000010
#define SP_CLR_SSTEP			0x0000020
#define SP_SET_SSTEP			0x0000040
#define	SP_CLR_INTR_BREAK		0x0000080
#define SP_SET_INTR_BREAK		0x0000100
#define SP_CLR_YIELD			0x0000200
#define SP_SET_YIELD			0x0000400
#define SP_CLR_YIELDED			0x0000800
#define SP_SET_YIELDED			0x0001000
#define SP_CLR_TASKDONE			0x0002000
#define SP_SET_TASKDONE			0x0004000
#define SP_CLR_SIG3				0x0008000
#define SP_SET_SIG3				0x0010000
#define SP_CLR_SIG4				0x0020000
#define SP_SET_SIG4				0x0040000
#define SP_CLR_SIG5				0x0080000
#define SP_SET_SIG5				0x0100000
#define SP_CLR_SIG6				0x0200000
#define SP_SET_SIG6				0x0400000
#define SP_CLR_SIG7				0x0800000
#define SP_SET_SIG7				0x1000000

/* DPC_STATUS_REG read bits */
#define DPC_STATUS_XBUS_DMEM_DMA	0x0000001
#define DPC_STATUS_FREEZE			0x0000002
#define DPC_STATUS_FLUSH			0x0000004

/* DPC_STATUS_REG write bits */
#define DPC_CLR_XBUS_DMEM_DMA	0x0000001
#define DPC_SET_XBUS_DMEM_DMA 	0x0000002
#define DPC_CLR_FREEZE			0x0000004
#define DPC_SET_FREEZE 			0x0000008
#define DPC_CLR_FLUSH			0x0000010
#define DPC_SET_FLUSH			0x0000020
#define DPC_CLR_TMEM_REG		0x0000040
#define DPC_CLR_PIPEBUSY_REG	0x0000080
#define DPC_CLR_BUFBUSY_REG		0x0000100
#define DPC_CLR_CLOCK_REG		0x0000200

extern void Intr_PI();
extern void Intr_SI();
extern void Intr_AI();
extern void Intr_DP();
extern void Intr_SP();
extern void Intr_VI();

// MACFIX - globals moved to mac_globals
//unsigned _int32 iCurrentInterruptMask;
extern unsigned _int32 iCurrentInterruptMask;

#define IE			0x00000001
#define EXL			0x00000002
#define ERL			0x00000004

//For speed...
#define EXL_OR_ERL	0x00000006

#define BD			0x80000000
#define NOT_BD		0x7FFFFFFF
#define BEV			0x00400000
#define TLBL_Miss	0x00000008
#define TLBS_Miss	0x00000009	


#define EXCCODE		0x7C
#define NOT_EXCCODE	0xFFFFFF83

extern void Intr_Common();
extern void __cdecl printlist (char * Message, ...);
extern void Dbg_Handle_SP(unsigned __int32 value);
extern void Handle_DPC();
extern void Handle_PI();

void WriteMI_ModeReg(unsigned __int32 value);
void Handle_MI(unsigned __int32 value);

#endif //_INTERRUPT_H__1964_
