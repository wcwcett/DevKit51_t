/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
/*  This program is protected by German copyright law and international      */
/*  treaties. The use of this software including but not limited to its      */
/*  Source Code is subject to restrictions as agreed in the license          */
/*  agreement between you and Siemens.                                       */
/*  Copying or distribution is not allowed unless expressly permitted        */
/*  according to your license agreement with Siemens.                        */
/*****************************************************************************/
/*                                                                           */
/*  P r o j e c t         &P: PROFINET IO Runtime Software              :P&  */
/*                                                                           */
/*  P a c k a g e         &W: PROFINET IO Runtime Software              :W&  */
/*                                                                           */
/*  C o m p o n e n t     &C: PnIODDevkits                              :C&  */
/*                                                                           */
/*  F i l e               &F: flash_ospi_devices.c                      :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
#ifdef BOARD_TYPE_STEP_3

#include "lsa_cfg.h"
#include "pnio_types.h"

#include "flash_ospi.h"

#include "arm926.h"
#include "ertec200p_reg.h"

#define WRITE_ENABLE 0x06
#define WRITE_VOLATILE_CONFIG_REG 0x81
#define WRITE_ENHANCED_VOLATILE_CONFIG_REG 0x61

static PNIO_VOID setup_mx25um512(const struct flash_ospi_info * info)
{
	(PNIO_VOID) info;
	flash_ospi_spi_write_register(WRITE_ENABLE, 0x00, 0,0x0, 0); // so I can write the registers
	flash_ospi_spi_write_register(0x72, 0x0300, 4, 0x5, 1); // 10 Dummy Cycles
	flash_ospi_spi_write_register(WRITE_ENABLE, 0x00,0, 0x0, 0); // so I can write the registers
	flash_ospi_spi_write_register(0x72, 0x0200, 4, 0x2, 1); // DQS enable
	flash_ospi_spi_write_register(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so I can write the registers
	flash_ospi_spi_write_register(0x72, 0x0, 4, 0x2, 1);    // uMSCHALTEN IN DDR opi Byte schreiben
}

static PNIO_VOID setup_mt25qu128(const struct flash_ospi_info * info)
{
	(PNIO_VOID) info;
	flash_ospi_spi_write_register(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so I can write the registers
	flash_ospi_spi_write_register(WRITE_VOLATILE_CONFIG_REG,0x0, 0, 0x8b, 1);  // Config registers describe 8 dummies 
	flash_ospi_spi_write_register(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so I can write the registers
	flash_ospi_spi_write_register(WRITE_ENHANCED_VOLATILE_CONFIG_REG, 0x0, 0, 0x0f, 1);  // Module is on quad, driver strength 30 ohms (default)
}	

static PNIO_VOID setup_mt25ql128(const struct flash_ospi_info * info)
{
	(PNIO_VOID) info;
	flash_ospi_spi_write_register(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so I can write the registers
	flash_ospi_spi_write_register(WRITE_VOLATILE_CONFIG_REG,0x0, 0, 0x8b, 1);  // Config registers describe 8 dummies 
	flash_ospi_spi_write_register(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so I can write the registers
	flash_ospi_spi_write_register(WRITE_ENHANCED_VOLATILE_CONFIG_REG, 0x0, 0, 0x0d, 1);  // Module is on quad, driver strength 20 ohms
}	

static PNIO_VOID setup_mt35xu512(const struct flash_ospi_info * info)
{
	(PNIO_VOID) info;
	flash_ospi_spi_write_register(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so I can write the registers
	flash_ospi_spi_write_register(0x81, 0x01, 3, 0x10, 1); // 16 Dummy Cycles
	flash_ospi_spi_write_register(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so I can write the registers
	flash_ospi_spi_write_register(0x81, 0x05, 3, 0xfe, 1); // 32 Bit
	flash_ospi_spi_write_register(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so I can write the registers
	flash_ospi_spi_write_register(0x81, 0x0, 4, 0xe7, 1);    // uMSCHALTEN IN DDR  mit DQs schreiben
}

static PNIO_VOID setup_mx25l12845(const struct flash_ospi_info * info)
{
	(PNIO_VOID) info;
	flash_ospi_spi_write_register(0x35, 0x00, 0, 0x0, 0); // enable QPI mode
}

static PNIO_VOID setup_mx25l12845_ddr(const struct flash_ospi_info * info)
{
	// increase dummy cycles for 62.5 MHz DDR (RMW status+configuration register)
	PNIO_UINT32 sr = flash_ospi_spi_write_register(0x05, 0x00, 0, 0x0, 1); // read status register
	PNIO_UINT32 cr = flash_ospi_spi_write_register(0x15, 0x00, 0, 0x0, 1); // read configuration register
	cr |= 0x2 << 6; // DC=2 -> 8 dummy cycles for DDR read
	PNIO_UINT32 sr_cr = sr | (cr << 8);
	flash_ospi_spi_write_register(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so I can write the registers
	flash_ospi_spi_write_register(0x01, 0x00, 0, sr_cr, 2); // write status+configuration register
	// takes up to 40 ms
	do {
		flash_ospi_trigger_watchdog(info);
		sr = flash_ospi_spi_write_register(0x05, 0x00, 0, 0x0, 1); // read status register
	} while (sr & 0x01); // busy

	flash_ospi_spi_write_register(0x35, 0x00, 0, 0x0, 0); // enable QPI mode
}

static PNIO_VOID setup_w25q128jv(const struct flash_ospi_info * info)
{
	(PNIO_VOID) info;

	/* Write Enable for Volatile Status Register */
	flash_ospi_spi_write_register(0x50, 0x00, 0, 0x00, 0);

	/* Write Status Register-2: QE=1 */
	flash_ospi_spi_write_register(0x31, 0x00, 0, 0x02, 1);

	/* Write Enable for Volatile Status Register */
	flash_ospi_spi_write_register(0x50, 0x00, 0, 0x00, 0);

	/* Write Status Register-3: HOLD/RST=0 (factory default) DRV[1:0]=01 (75%) WPS=0 (factory default) */
	flash_ospi_spi_write_register(0x11, 0x00, 0, 0x20, 1);

	/* Enter QPI Mode */
	flash_ospi_spi_write_register(0x38, 0x00, 0, 0x00, 0);
}

/* shift values for OCTALSPI_STROBE_REG according to STA */
#define SHIFT_DQS_INT_3V3 0
#define SHIFT_DQS_INT_1V8 1

const flash_ospi_register_struct_s flash_ospi_flash_param[FLASH_OSPI_FLASH_CNT] = {
	// FLASH_OSPI_FLASH_NONE
	{
		0x00,
		NULL,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		{
		  {0x0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
		  {0x0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
		  {0x0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
		  {0x0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
		  {0x0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
		  {0x0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
		  {0x0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
		  {0x0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 }
		}
	},
	// FLASH_OSPI_FLASH_MX25UM512
	{
0x03, // global Mode
setup_mx25um512,
0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, // Programm Done Mask & Pol, Programm Error Mask & Pol, Erase Done Mask & Pol, Erase Error Mask & Pol 
{
//  Cmd      Ctrlr0      Ctrlr1      Spi_ctrr0   spfu_spi     strobe      baudr       drive_edge
  {0xee11, 0x007f0240, 0x0000000f, 0x00070322, 0x00ee1111 , 0x00000000, 0x00000002 , 0x00000000}, // Read Command
  {0x12ed, 0x007f0140, 0x00000000, 0x00070322, 0x00000000 , 0x00000000, 0x00000010 , 0x00000000}, // Program Command
  {0x06f9, 0x007f0100, 0x00000000, 0x00030302, 0x00000000 , 0x00000000, 0x00000010 , 0x00000001}, // Write Enable Command
  {0x05fa, 0x007f0240, 0x00000003, 0x00070322, 0x00000000 , 0x00000000, 0x00000002 , 0x00000000}, // Read Status Command
  {0x21de, 0x007f0140, 0x00000000, 0x00030322, 0x00000000 , 0x00000000, 0x00000010 , 0x00000000}, // Erase Sector Command
  {0xdc23, 0x007f0140, 0x00000000, 0x00030322, 0x00000000 , 0x00000000, 0x00000010 , 0x00000000}, // Erase Block Command
  {0x0000, 0x007f0140, 0x00000000, 0x00030222, 0x00000000 , 0x00000000, 0x00000010 , 0x00000000}, // User Command 0
  {0x0000, 0x007f0140, 0x00000000, 0x00030222, 0x00000000 , 0x00000000, 0x00000010 , 0x00000000}  // User Command 1
}
	},
	// FLASH_OSPI_FLASH_MT25QU128
	{
0x02, // global Mode
setup_mt25qu128,
0x80, 0x80, 0x10, 0x10, 0x80, 0x80, 0x20, 0x20, // Programm Done Mask & Pol, Programm Error Mask & Pol, Erase Done Mask & Pol, Erase Error Mask & Pol 
{
//  Cmd      Ctrlr0      Ctrlr1      Spi_ctrr0   spfu_spi     strobe      baudr       drive_edge
  {0x00ed, 0x005f0240, 0x0000000f, 0x0007421a, 0x0000ed11, FLASH_OSPI_STROBE_REG(1, 8 + SHIFT_DQS_INT_1V8, 11), 0x00000002, 0x00000000}, // Read Command
  {0x0002, 0x005f0140, 0x00000000, 0x0003021a, 0x00000000, 0x00000000,                                         0x00000010, 0x00000000}, // Program Command
  {0x0006, 0x005f0100, 0x00000000, 0x00030202, 0x00000000, 0x00000000,                                         0x00000010, 0x00000001}, // Write Enable Command
  {0x0070, 0x005f0240, 0x00000001, 0x00070202, 0x00000000, FLASH_OSPI_STROBE_REG(1, 8 + SHIFT_DQS_INT_1V8, 0),  0x00000002, 0x00000000}, // Read Status Command
  {0x0020, 0x005f0140, 0x00000000, 0x0003021a, 0x00000000, 0x00000000,                                         0x00000010, 0x00000000}, // Erase Sector Command
  {0x00d8, 0x005f0140, 0x00000000, 0x0003021a, 0x00000000, 0x00000000,                                         0x00000010, 0x00000000}, // Erase Block Command
  {0x0050, 0x005f0100, 0x00000000, 0x00030202, 0x00000000, 0x00000000,                                         0x00000010, 0x00000001}, // User Command 0
  {0x0000, 0x005f0140, 0x00000000, 0x00010221, 0x00000000, 0x00000000,                                         0x00000010, 0x00000000}  // User Command 1
}
	},
	// FLASH_OSPI_FLASH_MT35XU512
	{
0x03, // global Mode
setup_mt35xu512,
0x80, 0x80, 0x10, 0x10, 0x80, 0x80, 0x20, 0x20, // Programm Done Mask & Pol, Programm Error Mask & Pol, Erase Done Mask & Pol, Erase Error Mask & Pol 
{
//  Cmd      Ctrlr0      Ctrlr1      Spi_ctrr0   spfu_spi     strobe      baudr       drive_edge
  {0x0c0c, 0x007f0240, 0x0000000f, 0x00070322, 0x000c0c11 , 0x00000000, 0x00000002 , 0x00000000}, // Read Command
  {0x1212, 0x007f0140, 0x00000000, 0x00030322, 0x00000000 , 0x00000000, 0x00000010 , 0x00000000}, // Program Command
  {0x0606, 0x007f0100, 0x00000000, 0x00030302, 0x00000000 , 0x00000000, 0x00000010 , 0x00000001}, // Write Enable Command
  {0x7070, 0x007f0240, 0x00000003, 0x00070302, 0x00000000 , 0x00000000, 0x00000002 , 0x00000000}, // Read Status Command
  {0x2020, 0x007f0140, 0x00000000, 0x00030322, 0x00000000 , 0x00000000, 0x00000010 , 0x00000000}, // Erase Sector Command
  {0xd8d8, 0x007f0140, 0x00000000, 0x00030322, 0x00000000 , 0x00000000, 0x00000010 , 0x00000000}, // Erase Block Command
  {0x5050, 0x007f0100, 0x00000000, 0x00030302, 0x00000000 , 0x00000000, 0x00000010 , 0x00000001}, // User Command 0
  {0x0000, 0x007f0140, 0x00000000, 0x00030222, 0x00000000 , 0x00000000, 0x00000010 , 0x00000000}  // User Command 1
}
	},
	// FLASH_OSPI_FLASH_MX25L12845_SDR
	{
0x02, // global Mode
setup_mx25l12845,
0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, // Programm Done Mask & Pol, Programm Error Mask & Pol, Erase Done Mask & Pol, Erase Error Mask & Pol 
{
//  Cmd      Ctrlr0      Ctrlr1      Spi_ctrr0   spfu_spi     strobe      baudr       drive_edge
  {0x00eb, 0x005f0240, 0x0000000f, 0x0004321a, 0x0000eb11, FLASH_OSPI_STROBE_REG(1, 8 + SHIFT_DQS_INT_3V3 - 1, 13), 0x00000002, 0x00000000}, // Read Command
  {0x0002, 0x005f0140, 0x00000000, 0x0004021a, 0x00000000, 0x00000000,                                             0x00000010, 0x00000000}, // Program Command
  {0x0006, 0x005f0100, 0x00000000, 0x00000202, 0x00000000, 0x00000000,                                             0x00000010, 0x00000001}, // Write Enable Command
  {0x0005, 0x005f0240, 0x00000000, 0x00040202, 0x00000000, FLASH_OSPI_STROBE_REG(1, 8 + SHIFT_DQS_INT_3V3 - 1, 1),  0x00000002, 0x00000000}, // Read Status Command
  {0x0020, 0x005f0140, 0x00000000, 0x0000021a, 0x00000000, 0x00000000,                                             0x00000010, 0x00000000}, // Erase Sector Command
  {0x00d8, 0x005f0140, 0x00000000, 0x0000021a, 0x00000000, 0x00000000,                                             0x00000010, 0x00000000}, // Erase Block Command
  {0x0000, 0x005f0140, 0x00000000, 0x00000222, 0x00000000, 0x00000000,                                             0x00000010, 0x00000000}, // User Command 0
  {0x0000, 0x005f0140, 0x00000000, 0x00000222, 0x00000000, 0x00000000,                                             0x00000010, 0x00000000}  // User Command 1
}
	},
	// FLASH_OSPI_FLASH_MX25L12845_DDR
	{
0x02, // global Mode
setup_mx25l12845_ddr,
0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, // Programm Done Mask & Pol, Programm Error Mask & Pol, Erase Done Mask & Pol, Erase Error Mask & Pol 
{
//  Cmd      Ctrlr0      Ctrlr1      Spi_ctrr0   spfu_spi     strobe      baudr       drive_edge
  {0xeedd, 0x005f0240, 0x0000000f, 0x0007431a, 0x00eedd11, FLASH_OSPI_STROBE_REG(1, 8 + SHIFT_DQS_INT_3V3 - 1, 12), 0x00000002, 0x00000000}, // Read Command
  {0x0002, 0x005f0140, 0x00000000, 0x0004021a, 0x00000000, 0x00000000,                                             0x00000010, 0x00000000}, // Program Command
  {0x0006, 0x005f0100, 0x00000000, 0x00000202, 0x00000000, 0x00000000,                                             0x00000010, 0x00000001}, // Write Enable Command
  {0x0005, 0x005f0240, 0x00000000, 0x00040202, 0x00000000, FLASH_OSPI_STROBE_REG(1, 8 + SHIFT_DQS_INT_3V3 - 1, 1),  0x00000002, 0x00000000}, // Read Status Command
  {0x0020, 0x005f0140, 0x00000000, 0x0000021a, 0x00000000, 0x00000000,                                             0x00000010, 0x00000000}, // Erase Sector Command
  {0x00d8, 0x005f0140, 0x00000000, 0x0000021a, 0x00000000, 0x00000000,                                             0x00000010, 0x00000000}, // Erase Block Command
  {0x0000, 0x005f0140, 0x00000000, 0x00000222, 0x00000000, 0x00000000,                                             0x00000010, 0x00000000}, // User Command 0
  {0x0000, 0x005f0140, 0x00000000, 0x00000222, 0x00000000, 0x00000000,                                             0x00000010, 0x00000000}  // User Command 1
}
	},
	// FLASH_OSPI_FLASH_MT25QL128
	{
0x02, // global Mode
setup_mt25ql128,
0x80, 0x80, 0x10, 0x10, 0x80, 0x80, 0x20, 0x20, // Programm Done Mask & Pol, Programm Error Mask & Pol, Erase Done Mask & Pol, Erase Error Mask & Pol 
{
//  Cmd      Ctrlr0      Ctrlr1      Spi_ctrr0   spfu_spi     strobe      baudr       drive_edge
  {0x00ed, 0x005f0240, 0x0000000f, 0x0007421a, 0x0000ed11, FLASH_OSPI_STROBE_REG(1, 8 + SHIFT_DQS_INT_3V3, 11), 0x00000002, 0x00000000}, // Read Command
  {0x0002, 0x005f0140, 0x00000000, 0x0003021a, 0x00000000, 0x00000000,                                         0x00000010, 0x00000000}, // Program Command
  {0x0006, 0x005f0100, 0x00000000, 0x00030202, 0x00000000, 0x00000000,                                         0x00000010, 0x00000001}, // Write Enable Command
  {0x0070, 0x005f0240, 0x00000001, 0x00070202, 0x00000000, FLASH_OSPI_STROBE_REG(1, 8 + SHIFT_DQS_INT_3V3, 0),  0x00000002, 0x00000000}, // Read Status Command
  {0x0020, 0x005f0140, 0x00000000, 0x0003021a, 0x00000000, 0x00000000,                                         0x00000010, 0x00000000}, // Erase Sector Command
  {0x00d8, 0x005f0140, 0x00000000, 0x0003021a, 0x00000000, 0x00000000,                                         0x00000010, 0x00000000}, // Erase Block Command
  {0x0050, 0x005f0100, 0x00000000, 0x00030202, 0x00000000, 0x00000000,                                         0x00000010, 0x00000001}, // User Command 0
  {0x0000, 0x005f0140, 0x00000000, 0x00010221, 0x00000000, 0x00000000,                                         0x00000010, 0x00000000}  // User Command 1
}
	},
	// FLASH_OSPI_FLASH_W25Q128JVSJM
	{
0x02, // global Mode
setup_w25q128jv,
0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, // Programm Done Mask & Pol, Programm Error Mask & Pol, Erase Done Mask & Pol, Erase Error Mask & Pol 
{
//  Cmd      Ctrlr0      Ctrlr1      Spi_ctrr0   spfu_spi     strobe      baudr       drive_edge
  {0xeedd, 0x005f0240, 0x0000000f, 0x0007431a, 0x00eedd11 , FLASH_OSPI_STROBE_REG(1, 8 + SHIFT_DQS_INT_3V3, 12), 0x00000002 , 0x00000000}, // Read Command
  {0x0002, 0x005f0140, 0x00000000, 0x0004021a, 0x00000000 , 0x00000000,                                         0x00000010 , 0x00000000}, // Program Command
  {0x0006, 0x005f0100, 0x00000000, 0x00000202, 0x00000000 , 0x00000000,                                         0x00000010 , 0x00000001}, // Write Enable Command
  {0x0005, 0x005f0240, 0x00000000, 0x00040202, 0x00000000 , FLASH_OSPI_STROBE_REG(1, 8 + SHIFT_DQS_INT_3V3, 1),  0x00000002 , 0x00000000}, // Read Status Command
  {0x0020, 0x005f0140, 0x00000000, 0x0000021a, 0x00000000 , 0x00000000,                                         0x00000010 , 0x00000000}, // Erase Sector Command
  {0x00d8, 0x005f0140, 0x00000000, 0x0000021a, 0x00000000 , 0x00000000,                                         0x00000010 , 0x00000000}, // Erase Block Command
  {0x0000, 0x005f0140, 0x00000000, 0x00000222, 0x00000000 , 0x00000000,                                         0x00000010 , 0x00000000}, // User Command 0
  {0x0000, 0x005f0140, 0x00000000, 0x00000222, 0x00000000 , 0x00000000,                                         0x00000010 , 0x00000000}  // User Command 1
}
	},
};

#endif // BOARD_TYPE_STEP_3

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/