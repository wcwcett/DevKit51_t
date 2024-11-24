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
/*  F i l e               &F: flash_vectors.c                           :F&  */
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

#include "compiler.h"
#include <pnioerrx.h>
#include <os.h>
#include <pniousrd.h>
#include "ertec200p_reg.h"
#include "flash_vectors.h"

#include <stdio.h>

volatile PNIO_UINT32 flash_dummy32_vectors;


/******************************************************************************
 *
 * Pallas Hardware Abstraction Layer OSPI
 *
 * Copyright (C) Siemens AG 2020. All Rights Reserved. Confidential.
 *
 *****************************************************************************/

__attribute__ ((section (".atext_i_tcm"))) static PNIO_VOID gpio_config(const PNIO_VOID * data,
        PNIO_UINT32 portmode,   
        PNIO_UINT32 portmode_mask,
        PNIO_UINT32 ioctrl,     
        PNIO_UINT32 ioctrl_mask,
        PNIO_UINT32 mux63_32,   
        PNIO_UINT32 mux63_32_mask,
        PNIO_UINT32 pull47_32,  
        PNIO_UINT32 pull47_32_mask)
{
    //lint --e{737} Loss of sign(int to unsigned int)

    PNIO_UINT32 tmp;
    tmp = (PNIO_UINT32)REG32(PN_SUB_GPIOX__GPIO_IOCTRL_1);
    tmp &= ~ioctrl_mask;
    tmp |= ioctrl;
    REG32(PN_SUB_GPIOX__GPIO_IOCTRL_1) = tmp;

    tmp = (PNIO_UINT32)REG32(PN_SUB_GPIOX__GPIO_PORT_MODE_1_L);
    tmp &= ~portmode_mask;
    tmp |= portmode;
    REG32(PN_SUB_GPIOX__GPIO_PORT_MODE_1_L) = tmp;

    tmp = (PNIO_UINT32)REG32(PN_SUB_PN_SCRB__PULL47_32GPIO);
    tmp &= ~pull47_32_mask;
    tmp |= pull47_32;
    REG32(PN_SUB_PN_SCRB__PULL47_32GPIO) = tmp;

    tmp = (PNIO_UINT32)REG32(PN_SUB_PN_SCRB2__MUX63_32GPIO);
    tmp &= ~mux63_32_mask;
    tmp |= mux63_32;
    REG32(PN_SUB_PN_SCRB2__MUX63_32GPIO) = tmp;
}

__attribute__ ((section (".atext_i_tcm"))) PNIO_UINT32 flash_ospi_spi_write_register_vectors(PNIO_UINT8 cmd, PNIO_UINT32 addr,
		PNIO_UINT8 addrlen, PNIO_UINT32 data32, PNIO_UINT8 datalen)
{
    //lint --e{732,737} Loss of sign(int to unsigned int)
    //lint --e{701} --e{702} Shift left/right of signed quantity

	PNIO_UINT32 rb;
	PNIO_UINT32 i;
	PNIO_UINT32 qd;
	PNIO_UINT32 ret;

	FLASH_OSPI_CS_ENABLE();
	REG32(U_OSPI__DR0) = cmd;
	do {} while ((REG32(U_OSPI__SR) & 0x4) == 0x0);
	do {} while ((REG32(U_OSPI__SR) & U_OSPI__SR__BUSY) == U_OSPI__SR__BUSY);
	FLASH_DUMMY_READ_REG32(U_OSPI__DR0);

	if (datalen == 0)
	{
		FLASH_OSPI_CS_DISABLE();
		return (0);
	}

	// Prepare address for output must be rotated
	switch (addrlen)
	{
		case 4:
			qd = ((addr & 0xff) << 24) | ((addr & 0x0000ff00) << 8) | ((addr & 0x00ff0000) >> 8) | ((addr & 0xff000000) >> 24);
			break;
		case 3:
			qd = ((addr & 0xff) << 16) | ((addr & 0x0000ff00) << 8) | ((addr & 0x00ff0000) >> 16);
			break;
		case 2:
			qd = ((addr & 0xff) << 8) | ((addr & 0x0000ff00) >> 8);
			break;
		case 1:
			qd = addr & 0xff;
			break;
		default:
			qd = 0xaffedead;
			addrlen = 0;
			break;
	}

	ret = 0;
	if (addrlen != 0)
	{
		for (i = 0; i < addrlen;i++)
		{
			REG32(U_OSPI__DR0) = qd & 0xff; //     Arg
			do {} while ((REG32(U_OSPI__SR) & 0x4) == 0x0);
			do {} while ((REG32(U_OSPI__SR) & U_OSPI__SR__BUSY) == U_OSPI__SR__BUSY);
			rb = REG32(U_OSPI__DR0);
			qd = qd >> 8;
			ret |= (rb << (i*8));
		}
	}


	qd = data32;
	ret = 0;
	if (datalen > 4)
		datalen = 4;  // only write 4 bytes max
	if (datalen != 0)
	{
		for (i = 0; i < datalen;i++)
		{
			REG32(U_OSPI__DR0) = qd & 0xff; //     Arg
			do {} while ((REG32(U_OSPI__SR) & 0x4) == 0x0);
			do {} while ((REG32(U_OSPI__SR) & U_OSPI__SR__BUSY) == U_OSPI__SR__BUSY);
			rb = REG32(U_OSPI__DR0);
			qd = qd >> 8;
			ret |= (rb << (i*8));
		}
	}
	FLASH_OSPI_CS_DISABLE();
	return (ret);
}

__attribute__ ((section (".atext_i_tcm"))) static PNIO_VOID setup_mx25um512(const struct flash_ospi_info * info)
{
	(PNIO_VOID) info;
	flash_ospi_spi_write_register_vectors(WRITE_ENABLE, 0x00, 0,0x0, 0); // so registers can be written
	flash_ospi_spi_write_register_vectors(0x72, 0x0300, 4, 0x5, 1); // 10 Dummy Cycles
	flash_ospi_spi_write_register_vectors(WRITE_ENABLE, 0x00,0, 0x0, 0); // so registers can be written
	flash_ospi_spi_write_register_vectors(0x72, 0x0200, 4, 0x2, 1); // DQS enable
	flash_ospi_spi_write_register_vectors(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so registers can be written
	flash_ospi_spi_write_register_vectors(0x72, 0x0, 4, 0x2, 1);    // switch to DDR write ospi byte
}

__attribute__ ((section (".atext_i_tcm"))) static PNIO_VOID setup_mt25qu128(const struct flash_ospi_info * info)
{
	(PNIO_VOID) info;
	flash_ospi_spi_write_register_vectors(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so registers can be written
	flash_ospi_spi_write_register_vectors(WRITE_VOLATILE_CONFIG_REG,0x0, 0, 0x8b, 1);  // Config Register 8 Dummy 
	flash_ospi_spi_write_register_vectors(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so registers can be written
	// flash_ospi_spi_write_register_vectors(WRITE_ENHANCED_VOLATILE_CONFIG_REG, 0x0, 0, 0x0f,1);  // Building block is on quad, 30ohm
	//flash_ospi_spi_write_register_vectors(WRITE_ENHANCED_VOLATILE_CONFIG_REG, 0x0, 0, 0x09,1);  // Building block is on quad, 90ohm
	// flash_ospi_spi_write_register_vectors(WRITE_ENHANCED_VOLATILE_CONFIG_REG, 0x0, 0, 0x0b,1);  // Building block is on quad, 45ohm
	flash_ospi_spi_write_register_vectors(WRITE_ENHANCED_VOLATILE_CONFIG_REG, 0x0, 0, 0x0d, 1);  // Building block is on quad, ODS=20ohm
}	

__attribute__ ((section (".atext_i_tcm"))) static PNIO_VOID setup_mt25ql128(const struct flash_ospi_info * info)
{
	(PNIO_VOID) info;
	flash_ospi_spi_write_register_vectors(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so registers can be written
	flash_ospi_spi_write_register_vectors(WRITE_VOLATILE_CONFIG_REG,0x0, 0, 0x8b, 1);  // Config Register 8 Dummy 
	flash_ospi_spi_write_register_vectors(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so registers can be written
	flash_ospi_spi_write_register_vectors(WRITE_ENHANCED_VOLATILE_CONFIG_REG, 0x0, 0, 0x0d, 1);  // Building block is on quad, Treiberstaerke 20 Ohm
}	

__attribute__ ((section (".atext_i_tcm"))) static PNIO_VOID setup_mt35xu512(const struct flash_ospi_info * info)
{
	(PNIO_VOID) info;
	flash_ospi_spi_write_register_vectors(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so registers can be written
	flash_ospi_spi_write_register_vectors(0x81, 0x01, 3, 0x10, 1); // 16 Dummy Cycles
	flash_ospi_spi_write_register_vectors(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so registers can be written
	flash_ospi_spi_write_register_vectors(0x81, 0x05, 3, 0xfe, 1); // 32 Bit
	flash_ospi_spi_write_register_vectors(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so registers can be written
	flash_ospi_spi_write_register_vectors(0x81, 0x0, 4, 0xe7, 1);    // switch to DDR with DQs write
}

__attribute__ ((section (".atext_i_tcm"))) static PNIO_VOID setup_mx25l12845(const struct flash_ospi_info * info)
{
	(PNIO_VOID) info;
	flash_ospi_spi_write_register_vectors(0x35, 0x00, 0, 0x0, 0); // enable QPI mode
}

__attribute__ ((section (".atext_i_tcm"))) static PNIO_VOID setup_mx25l12845_ddr(const struct flash_ospi_info * info)
{
    //lint --e{732} Loss of sign(int to unsigned int) 

	// increase dummy cycles for 62.5 MHz DDR (RMW status+configuration register)
	PNIO_UINT32 sr = flash_ospi_spi_write_register_vectors(0x05, 0x00, 0, 0x0, 1); // read status register
	PNIO_UINT32 cr = flash_ospi_spi_write_register_vectors(0x15, 0x00, 0, 0x0, 1); // read configuration register
	cr |= 0x2 << 6; // DC=2 -> 8 dummy cycles for DDR read
	PNIO_UINT32 sr_cr = sr | (cr << 8);
	flash_ospi_spi_write_register_vectors(WRITE_ENABLE, 0x00, 0, 0x0, 0); // so registers can be written
	flash_ospi_spi_write_register_vectors(0x01, 0x00, 0, sr_cr, 2); // write status+configuration register
	// takes up to 40 ms
	do {
        /*flash_ospi_trigger_watchdog(info);*/
        {
            if (info->watchdog_trigger_func != NULL) {
                info->watchdog_trigger_func(info->watchdog_trigger_data);
            }
        }

		sr = flash_ospi_spi_write_register_vectors(0x05, 0x00, 0, 0x0, 1); // read status register
	} while (sr & 0x01); // busy

	flash_ospi_spi_write_register_vectors(0x35, 0x00, 0, 0x0, 0); // enable QPI mode
}

__attribute__ ((section (".atext_i_tcm"))) static PNIO_VOID setup_w25q128jv(const struct flash_ospi_info * info)
{
	(PNIO_VOID) info;

	/* Write Enable for Volatile Status Register */
	flash_ospi_spi_write_register_vectors(0x50, 0x00, 0, 0x00, 0);

	/* Write Status Register-2: QE=1 */
	flash_ospi_spi_write_register_vectors(0x31, 0x00, 0, 0x02, 1);

	/* Write Enable for Volatile Status Register */
	flash_ospi_spi_write_register_vectors(0x50, 0x00, 0, 0x00, 0);

	/* Write Status Register-3: HOLD/RST=0 (factory default) DRV[1:0]=01 (75%) WPS=0 (factory default) */
	flash_ospi_spi_write_register_vectors(0x11, 0x00, 0, 0x20, 1);

	/* Enter QPI Mode */
	flash_ospi_spi_write_register_vectors(0x38, 0x00, 0, 0x00, 0);
}

__attribute__ ((section(".mdata")))  flash_ospi_register_struct_s flash_ospi_flash_param_vectors[FLASH_OSPI_FLASH_CNT] = {
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

__attribute__ ((section (".atext_i_tcm"))) static PNIO_VOID ospi_command_vectors(PNIO_UINT8 cmd)
{
    //lint --e{732,737} Loss of sign(int to unsigned int) 

	FLASH_OSPI_CS_ENABLE();
	REG32(U_OSPI__DR0) = cmd;
	FLASH_DUMMY_READ_REG32(U_OSPI__SR);
	do {} while ((REG32(U_OSPI__SR) & U_OSPI__SR__TFE) == 0);
	do {} while (REG32(U_OSPI__SR) & U_OSPI__SR__BUSY);
	FLASH_DUMMY_READ_REG32(U_OSPI__DR0);
	FLASH_OSPI_CS_DISABLE();
}

__attribute__ ((section (".atext_i_tcm"))) static PNIO_VOID ospi_command_2_vectors(PNIO_UINT8 cmd1, PNIO_UINT8 cmd2)
{
    //lint --e{732,737} Loss of sign(int to unsigned int) 

	FLASH_OSPI_CS_ENABLE();
	REG32(U_OSPI__DR0) = cmd1;
	REG32(U_OSPI__DR0) = cmd2;
	FLASH_DUMMY_READ_REG32(U_OSPI__SR);
	do {} while ((REG32(U_OSPI__SR) & U_OSPI__SR__TFE) == 0);
	do {} while (REG32(U_OSPI__SR) & U_OSPI__SR__BUSY);
	FLASH_DUMMY_READ_REG32(U_OSPI__DR0);
	FLASH_OSPI_CS_DISABLE();
}

__attribute__ ((section (".atext_i_tcm"))) static PNIO_VOID ospi_command_16_vectors(PNIO_UINT16 cmd)
{
    //lint --e{732,737} Loss of sign(int to unsigned int) 

	FLASH_OSPI_CS_ENABLE();
	REG32(U_OSPI__DR0) = cmd;
	FLASH_DUMMY_READ_REG32(U_OSPI__SR);
	do {} while ((REG32(U_OSPI__SR) & U_OSPI__SR__TFE) == 0);
	do {} while (REG32(U_OSPI__SR) & U_OSPI__SR__BUSY);
	FLASH_DUMMY_READ_REG32(U_OSPI__DR0);
	FLASH_OSPI_CS_DISABLE();
}


__attribute__ ((section (".atext_i_tcm"))) PNIO_VOID flash_ospi_reset_1_vectors(struct flash_ospi_info * info)
{
	int octal;

	/* CONFIG(2) specifies max. allowed I/O width: octal (1) or quad (0) */
	octal = REG32(U_SCRB2__SETUP_REG) & 0x4;

	/* set CSn, SCK and SI to output, values 1, 0 and 0 */
	REG32(U_GPIOX__GPIO_OUT_SET_1) = 0x00000001;
	REG32(U_GPIOX__GPIO_OUT_CLEAR_1) = 0x0000000a;
	info->gpio_config_func(info->gpio_config_data,
			0x00000000, 0x00000000, // portmode
			0x00000000, 0x0000000b, // ioctrl
			0x00000000, 0x00000000, // mux63_32
			0x00000000, 0x00000000); // pull47_32

	/* disable PFU */
	REG32(U_OSPI_SPFU__SPI_CTRL) = 0;

	/* common settings */
	FLASH_OSPI_CS_DISABLE();
	REG32(U_OSPI__SSIENR) = 0;  // Disable IP
	REG32(U_OSPI__BAUDR) = 0x0a; // 12,5 MHz
	REG32(U_OSPI__TXFTLR) = 0x1; // Transmit Fifo Tiefe 1
	REG32(U_OSPI__RXFTLR) = 0x1; // Receive Fifo Tiefe 1
	REG32(U_OSPI__IMR) = 0x0;
	REG32(U_OSPI__CTRLR1) = 0x0000000;  // immer -1 keine Dataframes
	REG32(U_OSPI__SER) = 0x1;  // Enable Slave

	if (octal) {
		/* set GPIOs to octal mode */
		info->gpio_config_func(info->gpio_config_data,
				FLASH_OCT_GPIO_PORT_MODE_SWCS, FLASH_OCT_GPIO_PORT_MODE_MASK,
				0x00000000, 0x00000000, // ioctrl
				FLASH_OCT_GPIO_MUX63_32, FLASH_OCT_GPIO_MUX63_32_MASK,
				FLASH_OCT_GPIO_PULL47_32, FLASH_OCT_GPIO_PULL47_32_MASK);

		/* send four-byte reset command in octal DDR mode (for MX25UM512...) */
		REG32(U_OSPI__SSIENR) = 0;  // Disable IP
		REG32(U_OSPI__CTRLR0) = 0x007f0100;
		REG32(U_OSPI__SPI_CTRLR0) = 0x00030302; // instruction DDR enable, DDR enable, 16 bit instruction
		REG32(U_OSPI__TXD_DRIVE_EDGE) = 0x00000003;
		REG32(U_OSPI__SSIENR) = 1;  // Enable IP
		ospi_command_16_vectors(0x6699);   // enable Reset
		ospi_command_16_vectors(0x9966);   // Reset

		/* send four-byte reset command in octal mode (for MX25UM512...) */
		REG32(U_OSPI__SSIENR) = 0;  // Disable IP
		REG32(U_OSPI__CTRLR0) = 0x00677107;  // 8 bit data size, octal
		REG32(U_OSPI__SPI_CTRLR0) = 0x00000202; // 8 bit instruction
		REG32(U_OSPI__TXD_DRIVE_EDGE) = 0x00000001;
		REG32(U_OSPI__SSIENR) = 1;  // Enable IP
		ospi_command_2_vectors(0x66, 0x99);   // enable Reset
		ospi_command_2_vectors(0x99, 0x66);   // Reset

		/* send reset command in octal DDR mode */
		REG32(U_OSPI__SSIENR) = 0;  // Disable IP
		REG32(U_OSPI__CTRLR0) = 0x007f0100;
		REG32(U_OSPI__SPI_CTRLR0) = 0x00030302;
		REG32(U_OSPI__TXD_DRIVE_EDGE) = 0x00000003;
		REG32(U_OSPI__SSIENR) = 1;  // Enable IP
		ospi_command_16_vectors(0x6666);   // enable Reset
		ospi_command_16_vectors(0x9999);   // Reset

		/* send reset command in octal mode */
		REG32(U_OSPI__SSIENR) = 0;  // Disable IP
		REG32(U_OSPI__CTRLR0) = 0x00677107;  // 8 bit data size, octal
		REG32(U_OSPI__SPI_CTRLR0) = 0x00000202; // 8 bit instruction
		REG32(U_OSPI__TXD_DRIVE_EDGE) = 0x00000001;
		REG32(U_OSPI__SSIENR) = 1;  // Enable IP
		ospi_command_vectors(0x66);   // enable Reset
		ospi_command_vectors(0x99);   // Reset
	}

	/* set GPIOs to quad mode */
	info->gpio_config_func(info->gpio_config_data,
			FLASH_QUAD_GPIO_PORT_MODE_SWCS, FLASH_OCT_GPIO_PORT_MODE_MASK,
			0x00000000, 0x00000000, // ioctrl
			FLASH_QUAD_GPIO_MUX63_32, FLASH_OCT_GPIO_MUX63_32_MASK,
			FLASH_QUAD_GPIO_PULL47_32, FLASH_OCT_GPIO_PULL47_32_MASK);

	/* send reset command in quad DDR mode */
	REG32(U_OSPI__SSIENR) = 0;  // Disable IP
	REG32(U_OSPI__CTRLR0) = 0x00477107;
	REG32(U_OSPI__SPI_CTRLR0) = 0x00030202;
	REG32(U_OSPI__TXD_DRIVE_EDGE) = 0x00000003;
	REG32(U_OSPI__SSIENR) = 1;  // Enable IP
	ospi_command_vectors(0x66);   // enable Reset
	ospi_command_vectors(0x99);   // Reset

	/* send reset command in quad mode */
	REG32(U_OSPI__SSIENR) = 0;  // Disable IP
	REG32(U_OSPI__CTRLR0) = 0x00477107;  // 8 bit data size, quad
	REG32(U_OSPI__SPI_CTRLR0) = 0x00000202; // 8 bit instruction
	REG32(U_OSPI__TXD_DRIVE_EDGE) = 0x00000000;
	REG32(U_OSPI__SSIENR) = 1;  // Enable IP
	ospi_command_vectors(0x66);   // enable Reset
	ospi_command_vectors(0x99);   // Reset

	/* set GPIOs to dual/single mode */
	info->gpio_config_func(info->gpio_config_data,
			FLASH_DUAL_GPIO_PORT_MODE_SWCS, FLASH_OCT_GPIO_PORT_MODE_MASK,
			0x00000000, 0x00000000, // ioctrl
			FLASH_DUAL_GPIO_MUX63_32, FLASH_OCT_GPIO_MUX63_32_MASK,
			FLASH_DUAL_GPIO_PULL47_32, FLASH_OCT_GPIO_PULL47_32_MASK);

	/* send reset command in dual mode */
	REG32(U_OSPI__SSIENR) = 0;  // Disable IP
	REG32(U_OSPI__CTRLR0) = 0x00277107;  // 8 bit data size, dual
	REG32(U_OSPI__SPI_CTRLR0) = 0x00000202; // 8 bit instruction
	REG32(U_OSPI__SSIENR) = 1;  // Enable IP
	ospi_command_vectors(0x66);   // enable Reset
	ospi_command_vectors(0x99);   // Reset

	/* send reset command in single mode */
	REG32(U_OSPI__SSIENR) = 0;  // Disable IP
	REG32(U_OSPI__CTRLR0) = 0x00070000;  // 8 bit data size, single
	REG32(U_OSPI__SPI_CTRLR0) = 0x00000202; // 8 bit instruction
	REG32(U_OSPI__SSIENR) = 1;  // Enable IP
	ospi_command_vectors(0x66);   // enable Reset
	ospi_command_vectors(0x99);   // Reset

}




__attribute__ ((section (".atext_i_tcm"))) PNIO_VOID flash_ospi_setup_register_vectors(struct flash_ospi_info * info, PNIO_UINT32 cmd_index)
{
	do{} while (REG32(U_OSPI_SPFU__SPI_CTRL) & U_OSPI_SPFU__SPI_CTRL__BUSY);

	REG32(U_OSPI_SPFU__SPI_CTRL) = 0; // Disable PFU
	REG32(U_OSPI__SSIENR) = 0;  // Disable IP
	REG32(U_OSPI__CTRLR0) = info->regs.cmd[cmd_index].ospi_ctrlr0;
	REG32(U_OSPI__CTRLR1) = info->regs.cmd[cmd_index].ospi_ctrlr1;
	REG32(U_OSPI__SPI_CTRLR0) = info->regs.cmd[cmd_index].ospi_spi_ctrlr0;
	REG32(U_OSPI__BAUDR) = info->regs.cmd[cmd_index]. ospi_baudr;
	REG32(U_SCRB2__OCTALSPI_STROBE_REG) = info->regs.cmd[cmd_index].ospi_strobe_reg;
	REG32(U_OSPI__TXD_DRIVE_EDGE) = info->regs.cmd[cmd_index].ospi_txd_drive_edge;
	REG32(U_OSPI__SSIENR) = 1;  // Enable IP
	REG32(U_OSPI_SPFU__SPI_CTRL) = info->regs.cmd[cmd_index].spfu_spi_ctrl;
}

__attribute__ ((section (".atext_i_tcm"))) PNIO_UINT32 flash_ospi_startup_quick_vectors(struct flash_ospi_info * info,
		const flash_ospi_register_struct_s * regs)
{
    do{} while (REG32(U_OSPI_SPFU__SPI_CTRL) & U_OSPI_SPFU__SPI_CTRL__BUSY);

	REG32(U_OSPI_SPFU__SPI_CTRL) = 0; // Disable PFU
	REG32(U_OSPI__SSIENR) = 0;  // Disable IP
	REG32(U_OSPI__TXFTLR) = 0x10; // Transmit Fifo Tiefe 32
	REG32(U_OSPI__RXFTLR) = 0x10; // Receive Fifo Tiefe  32
	REG32(U_OSPI__IMR) = 0x0;
	REG32(U_OSPI__SER) = 0x1;  // Enable Slave
	REG32(U_OSPI__DMACR) = U_OSPI__DMACR__RDMAE | U_OSPI__DMACR__TDMAE;
	REG32(U_OSPI__IMR) = U_OSPI__IMR__RXFIM;
	REG32(U_OSPI__TXD_DRIVE_EDGE) = 0;
	REG32(U_OSPI__SSIENR) = 1;  // Enable IP

    //	//flash_ospi_init_ip_default();//
    //lint -e{419} Apparent data overrun for function
	memcpy(&info->regs, regs, sizeof(flash_ospi_register_struct_s));

	if (info->regs.global_mode == FLASH_OSPI_MODE_OCTAL) {
		info->gpio_port_mode = FLASH_OCT_GPIO_PORT_MODE;
		info->gpio_ioctrl = FLASH_OCT_GPIO_IOCTRL;
		info->program_cmd_mode = FLASH_WRITE_GLOBAL_MODE_OCTAL;
		info->program_addr_mode = FLASH_WRITE_GLOBAL_MODE_OCTAL;
		info->program_data_mode = FLASH_WRITE_GLOBAL_MODE_OCTAL;
		info->erasesector_cmd_mode = FLASH_WRITE_GLOBAL_MODE_OCTAL;
		info->erasesector_addr_mode = FLASH_WRITE_GLOBAL_MODE_OCTAL;
		info->eraseblock_cmd_mode = FLASH_WRITE_GLOBAL_MODE_OCTAL;
		info->eraseblock_addr_mode = FLASH_WRITE_GLOBAL_MODE_OCTAL;
		info->user_cmd_0_cmd_mode = FLASH_WRITE_GLOBAL_MODE_OCTAL;
		info->user_cmd_0_addr_mode = FLASH_WRITE_GLOBAL_MODE_OCTAL;

		// GPIOs auf Octal stellen
		info->gpio_config_func(info->gpio_config_data,
				FLASH_OCT_GPIO_PORT_MODE, FLASH_OCT_GPIO_PORT_MODE_MASK,
				0x00000000, 0x00000000, // ioctrl
				FLASH_OCT_GPIO_MUX63_32, FLASH_OCT_GPIO_MUX63_32_MASK,
				FLASH_OCT_GPIO_PULL47_32, FLASH_OCT_GPIO_PULL47_32_MASK);
	} else if (info->regs.global_mode == FLASH_OSPI_MODE_QUAD) {
		info->gpio_port_mode = FLASH_QUAD_GPIO_PORT_MODE;
		info->gpio_ioctrl = FLASH_QUAD_GPIO_IOCTRL;
		info->program_cmd_mode = FLASH_WRITE_GLOBAL_MODE_QUAD;
		info->program_addr_mode = FLASH_WRITE_GLOBAL_MODE_QUAD;
		info->program_data_mode = FLASH_WRITE_GLOBAL_MODE_QUAD;
		info->erasesector_cmd_mode = FLASH_WRITE_GLOBAL_MODE_QUAD;
		info->erasesector_addr_mode = FLASH_WRITE_GLOBAL_MODE_QUAD;
		info->eraseblock_cmd_mode = FLASH_WRITE_GLOBAL_MODE_QUAD;
		info->eraseblock_addr_mode = FLASH_WRITE_GLOBAL_MODE_QUAD;
		info->user_cmd_0_cmd_mode = FLASH_WRITE_GLOBAL_MODE_QUAD;
		info->user_cmd_0_addr_mode = FLASH_WRITE_GLOBAL_MODE_QUAD;

		// GPIOs auf Quad stellen
		info->gpio_config_func(info->gpio_config_data,
				FLASH_QUAD_GPIO_PORT_MODE, FLASH_QUAD_GPIO_PORT_MODE_MASK,
				0x00000000, 0x00000000, // ioctrl
				FLASH_QUAD_GPIO_MUX63_32, FLASH_QUAD_GPIO_MUX63_32_MASK,
				FLASH_QUAD_GPIO_PULL47_32, FLASH_QUAD_GPIO_PULL47_32_MASK);
	} else {
		return 1;
	}

	// PROGRAM wird per Software ueber GPIO gemacht. Die notwendigen Einstellungen
	// holen wir uns aber aus den Vorgaben fuer die Parameterregister vom IP.
	if ((info->regs.cmd[FLASH_OSPI_PROGRAM_CMD].ospi_spi_ctrlr0 & FLASH_MODE_CMD_STD) == FLASH_MODE_CMD_STD) {
		info->program_cmd_mode = FLASH_MODE_CMD_SINGLE | (info->regs.cmd[FLASH_OSPI_PROGRAM_CMD].ospi_spi_ctrlr0 & (0x3<<8));
	} else {
		info->program_cmd_mode |= info->regs.cmd[FLASH_OSPI_PROGRAM_CMD].ospi_spi_ctrlr0 & FLASH_MODE_CMD_MASK;
	}
	info->program_addr_mode |= info->regs.cmd[FLASH_OSPI_PROGRAM_CMD].ospi_spi_ctrlr0 & FLASH_MODE_ADDR_MASK;
	info->program_data_mode |= info->regs.cmd[FLASH_OSPI_PROGRAM_CMD].ospi_spi_ctrlr0 & FLASH_MODE_DATA_MASK;

	// gleiches fuer ERASE_SECTOR
	if ((info->regs.cmd[FLASH_OSPI_ERASE_SECTOR_CMD].ospi_spi_ctrlr0 & FLASH_MODE_CMD_STD) == FLASH_MODE_CMD_STD) {
		info->erasesector_cmd_mode = FLASH_MODE_CMD_SINGLE | (info->regs.cmd[FLASH_OSPI_ERASE_SECTOR_CMD].ospi_spi_ctrlr0 & (0x3<<8));
	} else {
		info->erasesector_cmd_mode |= info->regs.cmd[FLASH_OSPI_ERASE_SECTOR_CMD].ospi_spi_ctrlr0 & FLASH_MODE_CMD_MASK;
	}
	info->erasesector_addr_mode |= info->regs.cmd[FLASH_OSPI_ERASE_SECTOR_CMD].ospi_spi_ctrlr0 & FLASH_MODE_ADDR_MASK;

	// gleiches fuer ERASE_BLOCK
	if ((info->regs.cmd[FLASH_OSPI_ERASE_BLOCK_CMD].ospi_spi_ctrlr0 & FLASH_MODE_CMD_STD) == FLASH_MODE_CMD_STD) {
		info->eraseblock_cmd_mode = FLASH_MODE_CMD_SINGLE | (info->regs.cmd[FLASH_OSPI_ERASE_BLOCK_CMD].ospi_spi_ctrlr0 & (0x3<<8));
	} else {
		info->eraseblock_cmd_mode |= info->regs.cmd[FLASH_OSPI_ERASE_BLOCK_CMD].ospi_spi_ctrlr0 & FLASH_MODE_CMD_MASK;
	}
	info->eraseblock_addr_mode |= info->regs.cmd[FLASH_OSPI_ERASE_BLOCK_CMD].ospi_spi_ctrlr0 & FLASH_MODE_ADDR_MASK;

	// gleiches fuer USER_CMD_0
	if ((info->regs.cmd[FLASH_OSPI_USER_CMD_0].ospi_spi_ctrlr0 & FLASH_MODE_CMD_STD) == FLASH_MODE_CMD_STD) {
		info->user_cmd_0_cmd_mode = FLASH_MODE_CMD_SINGLE | (info->regs.cmd[FLASH_OSPI_USER_CMD_0].ospi_spi_ctrlr0 & (0x3<<8));
	} else {
		info->user_cmd_0_cmd_mode |= info->regs.cmd[FLASH_OSPI_USER_CMD_0].ospi_spi_ctrlr0 & FLASH_MODE_CMD_MASK;
	}
	info->user_cmd_0_addr_mode |= info->regs.cmd[FLASH_OSPI_USER_CMD_0].ospi_spi_ctrlr0 & FLASH_MODE_ADDR_MASK;

	flash_ospi_setup_register_vectors(info, FLASH_OSPI_READ_CMD);  // immer mit Prefetch enable und zum Lesen starten
	return 0;
}

__attribute__ ((section (".atext_i_tcm"))) PNIO_UINT32 flash_ospi_setup_vectors(struct flash_ospi_info * info, PNIO_UINT32 flash_param_id)
{
	if (flash_param_id >= FLASH_OSPI_FLASH_CNT) {
		return PNIO_FLASH_PARAM_ERR;
	}

	if (flash_param_id == 0) {
		/* use flash_param_id from service register (for reinitialization during FW startup
		 * ofter OSPI boot SBL) */
		flash_param_id = REG32(FLASH_OSPI_SERVICE_REG);
		if (flash_param_id == 0 || flash_param_id >= FLASH_OSPI_FLASH_CNT) {
			return PNIO_FLASH_PARAM_ERR;
		}
	} else {
		if (REG32(FLASH_OSPI_SERVICE_REG) != 0) {
			return PNIO_FLASH_PARAM_ERR;
		}

		/* use specified flash_param_id (for OSPI boot SBL or standalone mode) */
		flash_ospi_flash_param_vectors[flash_param_id].setup_func(info);
		REG32(FLASH_OSPI_SERVICE_REG) = flash_param_id;
	}

	if (flash_ospi_startup_quick_vectors(info, &flash_ospi_flash_param_vectors[flash_param_id]) != 0) {
		return PNIO_FLASH_PARAM_ERR;
	}

	return 0;
}


__attribute__ ((section (".atext_i_tcm"))) PNIO_UINT32 init_ospi_vectors()
{
    //lint --e{732,737} Loss of sign(int to unsigned int) 

    PNIO_UINT32 ospi_id = 0;
    PNIO_UINT32 flash_id = 0;
    PNIO_UINT32 ret_code = 0;
    flash_ospi_info_t ospi_info;
    /* init all variables */
	ospi_info.gpio_port_mode = 0;
	ospi_info.gpio_ioctrl = 0;
	ospi_info.program_cmd_mode = 0;
	ospi_info.program_addr_mode = 0;
	ospi_info.program_data_mode = 0;
	ospi_info.erasesector_cmd_mode = 0;
	ospi_info.erasesector_addr_mode = 0;
	ospi_info.eraseblock_cmd_mode = 0;
	ospi_info.eraseblock_addr_mode = 0;
	ospi_info.gpio_config_func = gpio_config;
	ospi_info.gpio_config_data = (PNIO_VOID *)0xa9a8a7a6;
	ospi_info.wait_us_func = NULL;
	ospi_info.wait_us_data = NULL;
	ospi_info.erase_in_progress = 0;

    flash_ospi_reset_1_vectors(&ospi_info);

    REG32(FLASH_OSPI_SERVICE_REG) = 0;

    ospi_id = flash_ospi_spi_write_register_vectors(0x9f, 0x0, 0, 0x0, 4) & SYS_OSPI_ID_MASK;

    if (ospi_id == SYS_OSPI_ID_MASK)
    {
        //error in read. return error code
    }
    switch (ospi_id) {
		case 0x1870ef:
		case 0x1840ef:
			flash_id = FLASH_OSPI_FLASH_W25Q128JVSJM;
			break;
		case 0x18ba20:
			flash_id = FLASH_OSPI_FLASH_MT25QL128;
			break;
		case 0x1820c2:
			flash_id = FLASH_OSPI_FLASH_MX25L12845_DDR;
			break;
		case SYS_OSPI_ID_MASK:
			flash_id = FLASH_OSPI_FLASH_NONE;
			break;
		default:
			break;
	}


    if(flash_id == 0)
    {
        return PNIO_FLASH_PARAM_ERR;
    }

    ret_code = flash_ospi_setup_vectors(&ospi_info, flash_id);

    if(ret_code != PNIO_OK)
    {
        return PNIO_FLASH_PARAM_ERR; 
    }

    return PNIO_OK;
}
#endif // BOARD_TYPE_STEP_3

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
