/*______________________________________________________________________________
 |                                                                              |
 |  1964 - n64rcp.h                                                             |
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

#ifndef __N64RCP_H
#define __N64RCP_H

#include "globals.h"
#include "memory.h"

/*
0x03F0 0000 to 0x03FF FFFF  RDRAM registers:
 --------------------------------------------
        RDRAM_BASE_REG - 0x03F00000

        0x03F0 0000 to 0x03F0 0003  RDRAM_CONFIG_REG or RDRAM_DEVICE_TYPE_REG
        0x03F0 0004 to 0x03F0 0007  RDRAM_DEVICE_ID_REG
        0x03F0 0008 to 0x03F0 000B  RDRAM_DELAY_REG
        0x03F0 000C to 0x03F0 000F  RDRAM_MODE_REG
        0x03F0 0010 to 0x03F0 0013  RDRAM_REF_INTERVAL_REG
        0x03F0 0014 to 0x03F0 0017  RDRAM_REF_ROW_REG
        0x03F0 0018 to 0x03F0 001B  RDRAM_RAS_INTERVAL_REG
        0x03F0 001C to 0x03F0 001F  RDRAM_MIN_INTERVAL_REG
        0x03F0 0020 to 0x03F0 0023  RDRAM_ADDR_SELECT_REG
        0x03F0 0024 to 0x03F0 0027  RDRAM_DEVICE_MANUF_REG
        0x03F0 0028 to 0x03FF FFFF  Unknown
*/
#define NUMBEROFRDRAMREG            10
#define RDRAM_CONFIG_REG            gMS_ramRegs0[0]
#define RDRAM_DEVICE_ID_REG         gMS_ramRegs0[1]
#define RDRAM_DELAY_REG             gMS_ramRegs0[2]
#define RDRAM_MODE_REG              gMS_ramRegs0[3]
#define RDRAM_REF_INTERVAL_REG      gMS_ramRegs0[4]
#define RDRAM_REF_ROW_REG           gMS_ramRegs0[5]
#define RDRAM_RAS_INTERVAL_REG      gMS_ramRegs0[6]
#define RDRAM_MIN_INTERVAL_REG      gMS_ramRegs0[7]
#define RDRAM_ADDR_SELECT_REG       gMS_ramRegs0[8]
#define RDRAM_DEVICE_MANUF_REG      gMS_ramRegs0[9]

/*
0x0400 0000 to 0x0400 FFFF  SP registers:
 -----------------------------------------
        SP_BASE_REG - 0x04040000

        0x0400 0000 to 0x0400 0FFF  SP_DMEM read/write
        0x0400 1000 to 0x0400 1FFF  SP_IMEM read/write
        0x0400 2000 to 0x0403 FFFF  Unused
        0x0404 0000 to 0x0404 0003  SP_MEM_ADDR_REG
        0x0404 0004 to 0x0404 0007  SP_DRAM_ADDR_REG
        0x0404 0008 to 0x0404 000B  SP_RD_LEN_REG
        0x0404 000C to 0x0404 000F  SP_WR_LEN_REG
        0x0404 0010 to 0x0404 0013  SP_STATUS_REG
        0x0404 0014 to 0x0404 0017  SP_DMA_FULL_REG
        0x0404 0018 to 0x0404 001B  SP_DMA_BUSY_REG
        0x0404 001C to 0x0404 001F  SP_SEMAPHORE_REG
        0x0404 0020 to 0x0407 FFFF  Unused
        0x0408 0000 to 0x0408 0003  SP_PC_REG
        0x0408 0004 to 0x0408 0007  SP_IBIST_REG
        0x0408 0008 to 0x040F FFFF  Unused
*/
#define SP_DMEM_START       0x04000000  /* read/write */
#define SP_DMEM_END     0x04000FFF
#define SP_IMEM_START       0x04001000  /* read/write */
#define SP_IMEM_END     0x04001FFF

#define SP_DMEM                gMS_SP_MEM[0]
#define SP_IMEM                gMS_SP_MEM[0x400]
#define SP_MEM_ADDR_REG        gMS_SP_REG_1[0x0]
#define SP_DRAM_ADDR_REG       gMS_SP_REG_1[0x1]
#define SP_RD_LEN_REG          gMS_SP_REG_1[0x2]
#define SP_WR_LEN_REG          gMS_SP_REG_1[0x3]
#define SP_STATUS_REG          gMS_SP_REG_1[0x4]
#define SP_DMA_FULL_REG        gMS_SP_REG_1[0x5]
#define SP_DMA_BUSY_REG        gMS_SP_REG_1[0x6]
#define SP_SEMAPHORE_REG       gMS_SP_REG_1[0x7]
#define SP_PC_REG              gMS_SP_REG_2[0x0]
#define SP_IBIST_REG           gMS_SP_REG_2[0x1]
#define HLE_DMEM_TASK          gMS_SP_MEM[0x03F0]
#define NUMBEROFSPREG   8

/*
0x0410 0000 to 0x041F FFFF  DP command registers:
 -------------------------------------------------
        DPC_BASE_REG - 0x04100000

        0x0410 0000 to 0x0410 0003  DPC_START_REG
        0x0410 0004 to 0x0410 0007  DPC_END_REG
        0x0410 0008 to 0x0410 000B  DPC_CURRENT_REG
        0x0410 000C to 0x0410 000F  DPC_STATUS_REG
        0x0410 0010 to 0x0410 0013  DPC_CLOCK_REG
        0x0410 0014 to 0x0410 0017  DPC_BUFBUSY_REG
        0x0410 0018 to 0x0410 001B  DPC_PIPEBUSY_REG
        0x0410 001C to 0x0410 001F  DPC_TMEM_REG
        0x0410 0020 to 0x041F FFFF  Unused
*/
#define DPC_START_REG          gMS_DPC[0]
#define DPC_END_REG            gMS_DPC[1]
#define DPC_CURRENT_REG        gMS_DPC[2]
#define DPC_STATUS_REG         gMS_DPC[3]
#define DPC_CLOCK_REG          gMS_DPC[4]
#define DPC_BUFBUSY_REG        gMS_DPC[5]
#define DPC_PIPEBUSY_REG       gMS_DPC[6]
#define DPC_TMEM_REG           gMS_DPC[7]
#define NUMBEROFDPREG   8

/*
0x0420 0000 to 0x042F FFFF  DP span registers:
 ----------------------------------------------
        DPS_BASE_REG - 0x04200000

        0x0420 0000 to 0x0420 0003  DPS_TBIST_REG
        0x0420 0004 to 0x0420 0007  DPS_TEST_MODE_REG
        0x0420 0008 to 0x0420 000B  DPS_BUFTEST_ADDR_REG
        0x0420 000C to 0x0420 000F  DPS_BUFTEST_DATA_REG
        0x0420 0010 to 0x042F FFFF  Unused
*/
#define DPS_TBIST_REG                gMS_DPS[0]
#define DPS_TEST_MODE_REG            gMS_DPS[1]
#define DPS_BUFTEST_ADDR_REG         gMS_DPS[2]
#define DPS_BUFTEST_DATA_REG         gMS_DPS[3]
#define NUMBEROFDPSREG  4

/*
0x0430 0000 to 0x043F FFFF  MIPS interface (MI) registers:
 ----------------------------------------------------------
        MI_BASE_REG - 0x04300000

        0x0430 0000 to 0x0430 0003  MI_INIT_MODE_REG or MI_MODE_REG
        0x0430 0004 to 0x0430 0007  MI_VERSION_REG or MI_NOOP_REG
        0x0430 0008 to 0x0430 000B  MI_INTR_REG
        0x0430 000C to 0x0430 000F  MI_INTR_MASK_REG
        0x0430 0010 to 0x043F FFFF  Unused
*/
#define MI_INIT_MODE_REG_R           gMS_MI[0]
#define MI_VERSION_REG_R             gMS_MI[1]
#define MI_INTR_REG_R                gMS_MI[2]
#define MI_INTR_MASK_REG_R           gMS_MI[3]
#define NUMBEROFMIREG   4

/*
0x0440 0000 to 0x044F FFFF  Video interface (VI) registers:
 -----------------------------------------------------------
        VI_BASE_REG - 0x04400000

        0x0440 0000 to 0x0440 0003  VI_STATUS_REG or VI_CONTROL_REG
        0x0440 0004 to 0x0440 0007  VI_ORIGIN_REG or VI_DRAM_ADDR_REG
        0x0440 0008 to 0x0440 000B  VI_WIDTH_REG or VI_H_WIDTH_REG
        0x0440 000C to 0x0440 000F  VI_INTR_REG or VI_V_INTR_REG
        0x0440 0010 to 0x0440 0013  VI_CURRENT_REG or VI_V_CURRENT_LINE_REG                                           
        0x0440 0014 to 0x0440 0017  VI_BURST_REG or VI_TIMING_REG
        0x0440 0018 to 0x0440 001B  VI_V_SYNC_REG
        0x0440 001C to 0x0440 001F  VI_H_SYNC_REG
        0x0440 0020 to 0x0440 0023  VI_LEAP_REG or VI_H_SYNC_LEAP_REG
        0x0440 0024 to 0x0440 0027  VI_H_START_REG or VI_H_VIDEO_REG
        0x0440 0028 to 0x0440 002B  VI_V_START_REG or VI_V_VIDEO_REG
        0x0440 002C to 0x0440 002F  VI_V_BURST_REG
        0x0440 0030 to 0x0440 0033  VI_X_SCALE_REG
        0x0440 0034 to 0x0440 0037  VI_Y_SCALE_REG
        0x0440 0038 to 0x044F FFFF  Unused
*/
#define VI_STATUS_REG   gMS_VI[0]
#define VI_ORIGIN_REG   gMS_VI[1]
#define VI_WIDTH_REG    gMS_VI[2]
#define VI_INTR_REG     gMS_VI[3]
#define VI_CURRENT_REG  gMS_VI[4]
#define VI_BURST_REG    gMS_VI[5]
#define VI_V_SYNC_REG   gMS_VI[6]
#define VI_H_SYNC_REG   gMS_VI[7]
#define VI_LEAP_REG     gMS_VI[8]
#define VI_H_START_REG  gMS_VI[9]
#define VI_V_START_REG  gMS_VI[10]
#define VI_V_BURST_REG  gMS_VI[11]
#define VI_X_SCALE_REG  gMS_VI[12]
#define VI_Y_SCALE_REG  gMS_VI[13]
#define NUMBEROFVIREG   14

/*
0x0450 0000 to 0x045F FFFF  Audio interface (AI) registers:
 -----------------------------------------------------------
        AI_BASE_REG - 0x04500000

        0x0450 0000 to 0x0450 0003  AI_DRAM_ADDR_REG
        0x0450 0004 to 0x0450 0007  AI_LEN_REG
        0x0450 0008 to 0x0450 000B  AI_CONTROL_REG
        0x0450 000C to 0x0450 000F  AI_STATUS_REG
        0x0450 0010 to 0x0450 0013  AI_DACRATE_REG
        0x0450 0014 to 0x0450 0017  AI_BITRATE_REG
        0x0450 0018 to 0x045F FFFF  Unused
*/

#define AI_DRAM_ADDR_REG   gMS_AI[0]
#define AI_LEN_REG         gMS_AI[1]
#define AI_CONTROL_REG     gMS_AI[2]
#define AI_STATUS_REG      gMS_AI[3]
#define AI_DACRATE_REG     gMS_AI[4]
#define AI_BITRATE_REG     gMS_AI[5]
#define NUMBEROFAIREG   6

#define AI_STATUS_FIFO_FULL 0x80000000      /* Bit 31: full */
#define AI_STATUS_DMA_BUSY  0x40000000      /* Bit 30: busy */

/*
 0x0460 0000 to 0x046F FFFF  Peripheral interface (PI) registers:
 ----------------------------------------------------------------
        PI_BASE_REG - 0x04600000

        0x0460 0000 to 0x0460 0003  PI_DRAM_ADDR_REG
        0x0460 0004 to 0x0460 0007  PI_CART_ADDR_REG
        0x0460 0008 to 0x0460 000B  PI_RD_LEN_REG
        0x0460 000C to 0x0460 000F  PI_WR_LEN_REG
        0x0460 0010 to 0x0460 0013 PI_STATUS_REG
        0x0460 0014 to 0x0460 0017  PI_BSD_DOM1_LAT_REG or PI_DOMAIN1_REG
        0x0460 0018 to 0x0460 001B  PI_BSD_DOM1_PWD_REG
        0x0460 001C to 0x0460 001F  PI_BSD_DOM1_PGS_REG
        0x0460 0020 to 0x0460 0023  PI_BSD_DOM1_RLS_REG
        0x0460 0024 to 0x0460 0027  PI_BSD_DOM2_LAT_REG or PI_DOMAIN2_REG
        0x0460 0028 to 0x0460 002B  PI_BSD_DOM2_PWD_REG
        0x0460 002C to 0x0460 002F  PI_BSD_DOM2_PGS_REG
        0x0460 0030 to 0x0460 0033  PI_BSD_DOM2_RLS_REG
        0x0460 0034 to 0x046F FFFF  Unused
*/
#define PI_DRAM_ADDR_REG        gMS_PI[0]
#define PI_CART_ADDR_REG        gMS_PI[1]
#define PI_RD_LEN_REG           gMS_PI[2]
#define PI_WR_LEN_REG           gMS_PI[3]
#define PI_STATUS_REG           gMS_PI[4]
#define PI_BSD_DOM1_LAT_REG     gMS_PI[5]
#define PI_BSD_DOM1_PWD_REG     gMS_PI[6]
#define PI_BSD_DOM1_PGS_REG     gMS_PI[7]
#define PI_BSD_DOM1_RLS_REG     gMS_PI[8]
#define PI_BSD_DOM2_LAT_REG     gMS_PI[9]
#define PI_BSD_DOM2_PWD_REG     gMS_PI[10]
#define PI_BSD_DOM2_PGS_REG     gMS_PI[11]
#define PI_BSD_DOM2_RLS_REG     gMS_PI[12]
#define NUMBEROFPIREG   13

#define PI_STATUS_RESET     0x01
#define PI_STATUS_CLR_INTR  0x02

#define PI_STATUS_ERROR     0x04
#define PI_STATUS_IO_BUSY   0x02
#define PI_STATUS_DMA_BUSY  0x01
#define PI_STATUS_DMA_IO_BUSY   0x03
/*
0x0470 0000 to 0x047F FFFF  RDRAM interface (RI) registers:
 -----------------------------------------------------------
        RI_BASE_REG - 0x04700000

        0x0470 0000 to 0x0470 0003  RI_MODE_REG
        0x0470 0004 to 0x0470 0007  RI_CONFIG_REG
        0x0470 0008 to 0x0470 000B  RI_CURRENT_LOAD_REG
        0x0470 000C to 0x0470 000F  RI_SELECT_REG
        0x0470 0010 to 0x0470 0013  RI_REFRESH_REG or RI_COUNT_REG
        0x0470 0014 to 0x0470 0017  RI_LATENCY_REG
        0x0470 0018 to 0x0470 001B  RI_RERROR_REG
        0x0470 001C to 0x0470 001F  RI_WERROR_REG
        0x0470 0020 to 0x047F FFFF  Unused
*/
#define RI_MODE_REG         gMS_RI[0]
#define RI_CONFIG_REG       gMS_RI[1]
#define RI_CURRENT_LOAD_REG gMS_RI[2]
#define RI_SELECT_REG       gMS_RI[3]
#define RI_REFRESH_REG      gMS_RI[4]
#define RI_LATENCY_REG      gMS_RI[5]
#define RI_RERROR_REG       gMS_RI[6]
#define RI_WERROR_REG       gMS_RI[7]
#define NUMBEROFRIREG   8

/*
0x0480 0000 to 0x048F FFFF  Serial interface (SI) registers:
 ------------------------------------------------------------
        SI_BASE_REG - 0x04800000

        0x0480 0000 to 0x0480 0003  SI_DRAM_ADDR_REG
        0x0480 0004 to 0x0480 0007  SI_PIF_ADDR_RD64B_REG
        0x0480 0008 to 0x0480 000B  Reserved
        0x0480 000C to 0x0480 000F  Reserved
        0x0480 0010 to 0x0480 0013  SI_PIF_ADDR_WR64B_REG
        0x0480 0014 to 0x0480 0017  Reserved
        0x0480 0018 to 0x0480 001B  SI_STATUS_REG
        0x0480 001C to 0x048F FFFF  Unused

  Unused:
 -------
        0x0490 0000 to 0x04FF FFFF  Unused
*/
#define SI_DRAM_ADDR_REG         gMS_SI[0]
#define SI_PIF_ADDR_RD64B_REG    gMS_SI[1]
#define SI_PIF_ADDR_WR64B_REG    gMS_SI[4]
#define SI_STATUS_REG            gMS_SI[6]
#define NUMBEROFSIREG   7

#define SI_STATUS_DMA_BUSY  0x0001
#define SI_STATUS_RD_BUSY   0x0002
#define SI_STATUS_DMA_ERROR 0x0008
#define SI_STATUS_INTERRUPT 0x1000

void RCP_Reset(void);

#endif
