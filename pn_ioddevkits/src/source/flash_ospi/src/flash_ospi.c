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
/*  F i l e               &F: flash_ospi.c                              :F&  */
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
#include "os.h"

#include "flash_ospi.h"
#include "flash_ospi_devices.h"

#include "arm926.h"
#include "ertec200p_reg.h"


volatile PNIO_UINT32 flash_dummy32;

/* values for flash_ospi_register_struct_t.global_mode */

static PNIO_VOID ospi_command(PNIO_UINT8 cmd)
{
	FLASH_OSPI_CS_ENABLE();
	REG32(U_OSPI__DR0) = cmd;
	FLASH_DUMMY_READ_REG32(U_OSPI__SR);
	do {} while ((REG32(U_OSPI__SR) & (PNIO_UINT32)U_OSPI__SR__TFE) == 0);
	do {} while (REG32(U_OSPI__SR) & (PNIO_UINT32)U_OSPI__SR__BUSY);
	FLASH_DUMMY_READ_REG32(U_OSPI__DR0);
	FLASH_OSPI_CS_DISABLE();
}

static PNIO_VOID ospi_command_2(PNIO_UINT8 cmd1, PNIO_UINT8 cmd2)
{
	FLASH_OSPI_CS_ENABLE();
	REG32(U_OSPI__DR0) = cmd1;
	REG32(U_OSPI__DR0) = cmd2;
	FLASH_DUMMY_READ_REG32(U_OSPI__SR);
	do {} while ((REG32(U_OSPI__SR) & (PNIO_UINT32)U_OSPI__SR__TFE) == 0);
	do {} while (REG32(U_OSPI__SR) & (PNIO_UINT32)U_OSPI__SR__BUSY);
	FLASH_DUMMY_READ_REG32(U_OSPI__DR0);
	FLASH_OSPI_CS_DISABLE();
}

static PNIO_VOID ospi_command_16(PNIO_UINT16 cmd)
{
	FLASH_OSPI_CS_ENABLE();
	REG32(U_OSPI__DR0) = cmd;
	FLASH_DUMMY_READ_REG32(U_OSPI__SR);
	do {} while ((REG32(U_OSPI__SR) & (PNIO_UINT32)U_OSPI__SR__TFE) == 0);
	do {} while (REG32(U_OSPI__SR) & (PNIO_UINT32)U_OSPI__SR__BUSY);
	FLASH_DUMMY_READ_REG32(U_OSPI__DR0);
	FLASH_OSPI_CS_DISABLE();
}

PNIO_VOID flash_ospi_reset_1(const struct flash_ospi_info * info)
{
	PNIO_UINT32 octal;

	/* CONFIG(2) specifies max. allowed I/O width: octal (1) or quad (0) */
	octal = REG32(U_SCRB2__SETUP_REG) & 0x4;

	/* set CSn, SCK and SI to output, values 1, 0 and 0 */
	REG32(U_GPIOX__GPIO_OUT_SET_1) = 0x00000001;
	REG32(U_GPIOX__GPIO_OUT_CLEAR_1) = 0x0000000a;
	info->gpio_config_func(info->gpio_config_data,
			0x00000000, 0x00000000,  // portmode
			0x00000000, 0x0000000b,  // ioctrl
			0x00000000, 0x00000000,  // mux63_32
			0x00000000, 0x00000000); // pull47_32

	/* disable PFU */
	REG32(U_OSPI_SPFU__SPI_CTRL) = 0;

	/* common settings */
	FLASH_OSPI_CS_DISABLE();
	REG32(U_OSPI__SSIENR) = 0;   // Disable IP
	REG32(U_OSPI__BAUDR) = 0x0a; // 12,5 MHz
	REG32(U_OSPI__TXFTLR) = 0x1; // Transmit Fifo Tiefe 1
	REG32(U_OSPI__RXFTLR) = 0x1; // Receive Fifo Tiefe 1
	REG32(U_OSPI__IMR) = 0x0;
	REG32(U_OSPI__CTRLR1) = 0x0000000; // immer -1 keine Dataframes
	REG32(U_OSPI__SER) = 0x1;		   // Enable Slave

	if (octal) {
		/* set GPIOs to octal mode */
		info->gpio_config_func(info->gpio_config_data,
				FLASH_OCT_GPIO_PORT_MODE_SWCS, FLASH_OCT_GPIO_PORT_MODE_MASK,
				0x00000000, 0x00000000, // ioctrl
				FLASH_OCT_GPIO_MUX63_32, FLASH_OCT_GPIO_MUX63_32_MASK,
				FLASH_OCT_GPIO_PULL47_32, FLASH_OCT_GPIO_PULL47_32_MASK);

		/* send four-byte reset command in octal DDR mode (for MX25UM512...) */
		REG32(U_OSPI__SSIENR) = 0;  	// Disable IP
		REG32(U_OSPI__CTRLR0) = 0x007f0100;
		REG32(U_OSPI__SPI_CTRLR0) = 0x00030302; // instruction DDR enable, DDR enable, 16 bit instruction
		REG32(U_OSPI__TXD_DRIVE_EDGE) = 0x00000003;
		REG32(U_OSPI__SSIENR) = 1;  	// Enable IP
		ospi_command_16(0x6699);   		// enable Reset
		ospi_command_16(0x9966);   		// Reset

		/* send four-byte reset command in octal mode (for MX25UM512...) */
		REG32(U_OSPI__SSIENR) = 0;		// Disable IP
		REG32(U_OSPI__CTRLR0) = 0x00677107;  // 8 bit data size, octal
		REG32(U_OSPI__SPI_CTRLR0) = 0x00000202; // 8 bit instruction
		REG32(U_OSPI__TXD_DRIVE_EDGE) = 0x00000001;
		REG32(U_OSPI__SSIENR) = 1;		// Enable IP
		ospi_command_2(0x66, 0x99);		// enable Reset
		ospi_command_2(0x99, 0x66);		// Reset

		/* send reset command in octal DDR mode */
		REG32(U_OSPI__SSIENR) = 0;		// Disable IP
		REG32(U_OSPI__CTRLR0) = 0x007f0100;
		REG32(U_OSPI__SPI_CTRLR0) = 0x00030302;
		REG32(U_OSPI__TXD_DRIVE_EDGE) = 0x00000003;
		REG32(U_OSPI__SSIENR) = 1;		// Enable IP
		ospi_command_16(0x6666);		// enable Reset
		ospi_command_16(0x9999);		// Reset

		/* send reset command in octal mode */
		REG32(U_OSPI__SSIENR) = 0;		// Disable IP
		REG32(U_OSPI__CTRLR0) = 0x00677107;		// 8 bit data size, octal
		REG32(U_OSPI__SPI_CTRLR0) = 0x00000202; // 8 bit instruction
		REG32(U_OSPI__TXD_DRIVE_EDGE) = 0x00000001;
		REG32(U_OSPI__SSIENR) = 1;		// Enable IP
		ospi_command(0x66);				// enable Reset
		ospi_command(0x99);				// Reset
	}

	/* set GPIOs to quad mode */
	info->gpio_config_func(info->gpio_config_data,
			FLASH_QUAD_GPIO_PORT_MODE_SWCS, FLASH_OCT_GPIO_PORT_MODE_MASK,
			0x00000000, 0x00000000, // ioctrl
			FLASH_QUAD_GPIO_MUX63_32, FLASH_OCT_GPIO_MUX63_32_MASK,
			FLASH_QUAD_GPIO_PULL47_32, FLASH_OCT_GPIO_PULL47_32_MASK);

	/* send reset command in quad DDR mode */
	REG32(U_OSPI__SSIENR) = 0; 			// Disable IP
	REG32(U_OSPI__CTRLR0) = 0x00477107;
	REG32(U_OSPI__SPI_CTRLR0) = 0x00030202;
	REG32(U_OSPI__TXD_DRIVE_EDGE) = 0x00000003;
	REG32(U_OSPI__SSIENR) = 1;			// Enable IP
	ospi_command(0x66);					// enable Reset
	ospi_command(0x99);					// Reset

	/* send reset command in quad mode */
	REG32(U_OSPI__SSIENR) = 0;			// Disable IP
	REG32(U_OSPI__CTRLR0) = 0x00477107;		// 8 bit data size, quad
	REG32(U_OSPI__SPI_CTRLR0) = 0x00000202; // 8 bit instruction
	REG32(U_OSPI__TXD_DRIVE_EDGE) = 0x00000000;
	REG32(U_OSPI__SSIENR) = 1;			// Enable IP
	ospi_command(0x66);					// enable Reset
	ospi_command(0x99);					// Reset

	/* set GPIOs to dual/single mode */
	info->gpio_config_func(info->gpio_config_data,
			FLASH_DUAL_GPIO_PORT_MODE_SWCS, FLASH_OCT_GPIO_PORT_MODE_MASK,
			0x00000000, 0x00000000, // ioctrl
			FLASH_DUAL_GPIO_MUX63_32, FLASH_OCT_GPIO_MUX63_32_MASK,
			FLASH_DUAL_GPIO_PULL47_32, FLASH_OCT_GPIO_PULL47_32_MASK);

	/* send reset command in dual mode */
	REG32(U_OSPI__SSIENR) = 0;		// Disable IP
	REG32(U_OSPI__CTRLR0) = 0x00277107;		// 8 bit data size, dual
	REG32(U_OSPI__SPI_CTRLR0) = 0x00000202; // 8 bit instruction
	REG32(U_OSPI__SSIENR) = 1;		// Enable IP
	ospi_command(0x66);				// enable Reset
	ospi_command(0x99);				// Reset

	/* send reset command in single mode */
	REG32(U_OSPI__SSIENR) = 0;		// Disable IP
	REG32(U_OSPI__CTRLR0) = 0x00070000;		// 8 bit data size, single
	REG32(U_OSPI__SPI_CTRLR0) = 0x00000202; // 8 bit instruction
	REG32(U_OSPI__SSIENR) = 1;		// Enable IP
	ospi_command(0x66);				// enable Reset
	ospi_command(0x99);				// Reset

}

static PNIO_VOID flash_ospi_init_ip_default(PNIO_VOID)
{
	do{} while (REG32(U_OSPI_SPFU__SPI_CTRL) & (PNIO_UINT32)U_OSPI_SPFU__SPI_CTRL__BUSY);

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
}

//////////////////////////////////////////////////
// Set the registers to previously determined values
// Setting is per command
PNIO_VOID flash_ospi_setup_register(const struct flash_ospi_info * info, PNIO_UINT32 cmd_index)
{
	do{} while (REG32(U_OSPI_SPFU__SPI_CTRL) & (PNIO_UINT32)U_OSPI_SPFU__SPI_CTRL__BUSY);

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

PNIO_UINT32 flash_ospi_startup_quick(struct flash_ospi_info * info,
		const flash_ospi_register_struct_s * regs)
{
	// now set the parameters in the IP first default
	flash_ospi_init_ip_default();

	OsMemCpy(&info->regs, regs, sizeof(flash_ospi_register_struct_s));

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

		// Set GPIOs to Octal
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

		// Set GPIOs to Quad
		info->gpio_config_func(info->gpio_config_data,
				FLASH_QUAD_GPIO_PORT_MODE, FLASH_QUAD_GPIO_PORT_MODE_MASK,
				0x00000000, 0x00000000, // ioctrl
				FLASH_QUAD_GPIO_MUX63_32, FLASH_QUAD_GPIO_MUX63_32_MASK,
				FLASH_QUAD_GPIO_PULL47_32, FLASH_QUAD_GPIO_PULL47_32_MASK);
	} else {
		return 1;
	}

	// PROGRAM is done by software over GPIO. 
	// However, we get the necessary settings from the specifications for the parameter register of the IP.
	if ((info->regs.cmd[FLASH_OSPI_PROGRAM_CMD].ospi_spi_ctrlr0 & FLASH_MODE_CMD_STD) == FLASH_MODE_CMD_STD) {
		info->program_cmd_mode = FLASH_MODE_CMD_SINGLE | (info->regs.cmd[FLASH_OSPI_PROGRAM_CMD].ospi_spi_ctrlr0 & (0x3<<8));
	} else {
		info->program_cmd_mode |= info->regs.cmd[FLASH_OSPI_PROGRAM_CMD].ospi_spi_ctrlr0 & FLASH_MODE_CMD_MASK;
	}
	info->program_addr_mode |= info->regs.cmd[FLASH_OSPI_PROGRAM_CMD].ospi_spi_ctrlr0 & FLASH_MODE_ADDR_MASK;
	info->program_data_mode |= info->regs.cmd[FLASH_OSPI_PROGRAM_CMD].ospi_spi_ctrlr0 & FLASH_MODE_DATA_MASK;

	// same for ERASE_SECTOR
	if ((info->regs.cmd[FLASH_OSPI_ERASE_SECTOR_CMD].ospi_spi_ctrlr0 & FLASH_MODE_CMD_STD) == FLASH_MODE_CMD_STD) {
		info->erasesector_cmd_mode = FLASH_MODE_CMD_SINGLE | (info->regs.cmd[FLASH_OSPI_ERASE_SECTOR_CMD].ospi_spi_ctrlr0 & (0x3<<8));
	} else {
		info->erasesector_cmd_mode |= info->regs.cmd[FLASH_OSPI_ERASE_SECTOR_CMD].ospi_spi_ctrlr0 & FLASH_MODE_CMD_MASK;
	}
	info->erasesector_addr_mode |= info->regs.cmd[FLASH_OSPI_ERASE_SECTOR_CMD].ospi_spi_ctrlr0 & FLASH_MODE_ADDR_MASK;

	// same for ERASE_BLOCK
	if ((info->regs.cmd[FLASH_OSPI_ERASE_BLOCK_CMD].ospi_spi_ctrlr0 & FLASH_MODE_CMD_STD) == FLASH_MODE_CMD_STD) {
		info->eraseblock_cmd_mode = FLASH_MODE_CMD_SINGLE | (info->regs.cmd[FLASH_OSPI_ERASE_BLOCK_CMD].ospi_spi_ctrlr0 & (0x3<<8));
	} else {
		info->eraseblock_cmd_mode |= info->regs.cmd[FLASH_OSPI_ERASE_BLOCK_CMD].ospi_spi_ctrlr0 & FLASH_MODE_CMD_MASK;
	}
	info->eraseblock_addr_mode |= info->regs.cmd[FLASH_OSPI_ERASE_BLOCK_CMD].ospi_spi_ctrlr0 & FLASH_MODE_ADDR_MASK;

	// same for USER_CMD_0
	if ((info->regs.cmd[FLASH_OSPI_USER_CMD_0].ospi_spi_ctrlr0 & FLASH_MODE_CMD_STD) == FLASH_MODE_CMD_STD) {
		info->user_cmd_0_cmd_mode = FLASH_MODE_CMD_SINGLE | (info->regs.cmd[FLASH_OSPI_USER_CMD_0].ospi_spi_ctrlr0 & (0x3<<8));
	} else {
		info->user_cmd_0_cmd_mode |= info->regs.cmd[FLASH_OSPI_USER_CMD_0].ospi_spi_ctrlr0 & FLASH_MODE_CMD_MASK;
	}
	info->user_cmd_0_addr_mode |= info->regs.cmd[FLASH_OSPI_USER_CMD_0].ospi_spi_ctrlr0 & FLASH_MODE_ADDR_MASK;

	flash_ospi_setup_register(info, FLASH_OSPI_READ_CMD);  // always start with prefetch enable and for reading
	return 0;
}

///////////////////////////////////////////////////////
// Write or read block in std. SPI format
//
// Call		cmd			Command
// 			addr 		address
// 			addrlen 	number of address bytes (0,1,2,3)
// 			data32 		Data
// 			datalen 	number of data in bytes (0,1,2,3)
//
// single command
// 		rmw = ospi_write_register(WRITE_ENABLE, 0x00, 0, 0x0, 0);
// Command with data half word
// 		rmw = ospi_write_register(WRITE_STATUS_CONFIG, 0x00, 0, 0xca02, 2);
// Command with 4 address bytes and 4 data bytes
//		rmw = ospi_write_register(0x72, 0x80123456, 4, 0x11223352, 4)
PNIO_UINT32 flash_ospi_spi_write_register(PNIO_UINT8 cmd, PNIO_UINT32 addr,
		PNIO_UINT8 addrlen, PNIO_UINT32 data32, PNIO_UINT8 datalen)
{
	PNIO_UINT32 rb;
	PNIO_UINT32 i;
	PNIO_UINT32 qd;
	PNIO_UINT32 ret;

	FLASH_OSPI_CS_ENABLE();
	REG32(U_OSPI__DR0) = cmd;
	do {} while ((REG32(U_OSPI__SR) & 0x4) == 0x0);
	do {} while ((REG32(U_OSPI__SR) & (PNIO_UINT32)U_OSPI__SR__BUSY) == (PNIO_UINT32)U_OSPI__SR__BUSY);

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
			do {} while ((REG32(U_OSPI__SR) & (PNIO_UINT32)U_OSPI__SR__BUSY) == (PNIO_UINT32)U_OSPI__SR__BUSY);
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
			do {} while ((REG32(U_OSPI__SR) & (PNIO_UINT32)U_OSPI__SR__BUSY) == (PNIO_UINT32)U_OSPI__SR__BUSY);
			rb = REG32(U_OSPI__DR0);
			qd = qd >> 8;
			ret |= (rb << (i*8));
		}
	}
	FLASH_OSPI_CS_DISABLE();
	return (ret);
}


/****************************************************************************
 *
 * Low-level functions, directly correspond to one flash command
 *
 ****************************************************************************/

PNIO_VOID flash_ospi_set_read_mode(const struct flash_ospi_info * info)
{
	flash_ospi_setup_register(info, FLASH_OSPI_READ_CMD);
}

/*#########################*/


/**
 * @brief initializes FLASH data structures
 *
 * @details Initializes info to default values, but does not initialize
 *          any hardware.
 *
 * @param info	pointer to FLASH info structure
 */
PNIO_VOID flash_ospi_init(flash_ospi_info_t * info)
{
	info->gpio_port_mode = 0;
	info->gpio_ioctrl = 0;
	info->program_cmd_mode = 0;
	info->program_addr_mode = 0;
	info->program_data_mode = 0;
	info->erasesector_cmd_mode = 0;
	info->erasesector_addr_mode = 0;
	info->eraseblock_cmd_mode = 0;
	info->eraseblock_addr_mode = 0;

	info->gpio_config_func = NULL;
	info->gpio_config_data = NULL;
	info->wait_us_func = NULL;
	info->wait_us_data = NULL;
	info->erase_in_progress = 0;
}

/**
 * @brief initializes FLASH and OSPI IP, optionally configures GPIOs and reset flash device
 *
 * @param info		pointer to FLASH info structure
 * @param params	pointer to open parameter structure (to keep argument list short)
 * @param reset		If true, initializes the OSPI IP, switches on GPIOs
 *			and sends a reset sequence to bring the flash device
 *			into standard SPI mode; if false, all this is expected
 *			to be done in advance, e.g. by the PBL during OSPI boot.
 *
 */
PNIO_UINT32 flash_ospi_open(flash_ospi_info_t * info, const struct flash_ospi_open_param * params, PNIO_UINT32 reset)
{
	info->gpio_config_func = params->gpio_config_func;
	info->gpio_config_data = params->gpio_config_data;
	info->wait_us_func = params->wait_us_func;
	info->wait_us_data = params->wait_us_data;
	info->watchdog_trigger_func = params->watchdog_trigger_func;
	info->watchdog_trigger_data = params->watchdog_trigger_data;

	info->gpio_config_func(info->gpio_config_data,
			FLASH_SINGLE_GPIO_PORT_MODE_SWCS, FLASH_SINGLE_GPIO_PORT_MODE_MASK,
			FLASH_SINGLE_GPIO_IOCTRL_SWCS,    FLASH_SINGLE_GPIO_IOCTRL_MASK,
			FLASH_SINGLE_GPIO_MUX63_32,       FLASH_SINGLE_GPIO_MUX63_32_MASK,
			FLASH_SINGLE_GPIO_PULL47_32,      FLASH_SINGLE_GPIO_PULL47_32_MASK);

	if (reset) {
		flash_ospi_reset_1(info);

		REG32(FLASH_OSPI_SERVICE_REG) = 0;
	}

	return 0;
}

PNIO_UINT32 flash_ospi_close(flash_ospi_info_t * info)
{
	(PNIO_VOID) info;
	return 0;
}

/**
 * @brief reads the ID register using standard command 0x9f and returns its first 4 bytes
 *
 * @note This functions uses standard SPI mode and must be called before switching to operating
 *       mode with flash_ospi_setup().
 *
 */
PNIO_UINT32 flash_ospi_read_idreg(PNIO_VOID)
{
	PNIO_UINT32 rdata;
	rdata = flash_ospi_spi_write_register(0x9f, 0x0, 0, 0x0, 4);
	return rdata;
}

/**
 * @brief configures FLASH and OSPI IP to the specified flash device
 *
 * @param info		pointer to FLASH info structure
 * @param flash_param_id	ID of flash parameter set as defined PNIO_UINT32 enum flash_ospi_flash_param_id
 *
 * @retval 0		OK
 * @retval PNIO_FLASH_PARAM_ERR 	illegal flash_param_id
 * @retval PNIO_FLASH_PARAM_ERR 	flash_param_id = 0 and service register does not contain flash_param_id
 * @retval PNIO_FLASH_PARAM_ERR 	flash_param_id != 0 and service register does contain flash_param_id
 * @retval PNIO_FLASH_PARAM_ERR 	illegal flash parameters
 *
 * @note	If flash_param_id != 0, the FLASH and the flash device are initialized with the 
 * 		parameters specified by flash_param_id. flash_param_id is stored in the service register.
 * 		If flash_param_id = 0, the flash_param_id is expected in the service register, and the flash
 * 		device is not initialized again.
 */
PNIO_UINT32 flash_ospi_setup(struct flash_ospi_info * info, PNIO_UINT32 flash_param_id)
{
	if (flash_param_id >= FLASH_OSPI_FLASH_CNT) {
		PNIO_ConsolePrintf("ERROR: illegal flash_param_id.\n");
		return PNIO_FLASH_PARAM_ERR;
	}

	if (flash_param_id == 0) {
		/* use flash_param_id from service register (for reinitialization during FW startup
		 * ofter OSPI boot SBL) */
		flash_param_id = REG32(FLASH_OSPI_SERVICE_REG);
		if (flash_param_id == 0 || flash_param_id >= FLASH_OSPI_FLASH_CNT) {
			PNIO_ConsolePrintf("ERROR: service register does not contain flash_param_id.\n");
			return PNIO_FLASH_PARAM_ERR;
		}
	} else {
		if (REG32(FLASH_OSPI_SERVICE_REG) != 0) {
			PNIO_ConsolePrintf("ERROR: illegal flash_param_id.\n");
			return PNIO_FLASH_PARAM_ERR;
		}

		/* use specified flash_param_id (for OSPI boot SBL or standalone mode) */
		flash_ospi_flash_param[flash_param_id].setup_func(info);
		REG32(FLASH_OSPI_SERVICE_REG) = flash_param_id;
	}

	if (flash_ospi_startup_quick(info, &flash_ospi_flash_param[flash_param_id]) != 0) {
		PNIO_ConsolePrintf("ERROR: illegal flash_param_id.\n");
		return PNIO_FLASH_PARAM_ERR;
	}

	return PNIO_OK;
}

/**
 * @brief returns the ID of the active flash parameter set
 *
 * @param info		pointer to FLASH info structure
 *
 * @returns flash_param_id, 0 if uninitialized
 *
 * @note This is the ID of the parameter set that has been given to
 *       flash_ospi_setup() during initialization and is stored in
 *       the FLASH service register.
 */
PNIO_UINT32 flash_ospi_get_flash_param_id(flash_ospi_info_t * info)
{
	(PNIO_VOID) info;

	return REG32(FLASH_OSPI_SERVICE_REG);
}

/**
 * @brief retrigger watchdog using user provided callback
 *
 * @param info		pointer to FLASH info structure
 */
PNIO_VOID flash_ospi_trigger_watchdog(const struct flash_ospi_info * info)
{
	if (info->watchdog_trigger_func != NULL) {
		info->watchdog_trigger_func(info->watchdog_trigger_data);
	}
}


/**
 * @brief abort the currently running FLASH action from another context
 *
 * @param info		pointer to FLASH info structure
 *
 * @details	This function is supposed to be called from another context,
 * 		e.g. an interrupt handler that is triggered on fatal errors.
 * 		It will abort all running actions and stop the corresponding IPs.
 * 		Afterwards, use of the FLASH is possible again.
 *		IMPORTANT: It is assumed that execution never returns to the
 *		original context!
 */
PNIO_UINT32 flash_ospi_abort(flash_ospi_info_t * info)
{
	PNIO_UINT32 status;


	/* terminate RSC (only possible though SW reset)
	 *
	 * (There is no reset done status available. But reset should
	 * happen immediately and the only problem seems to be that no
	 * writes to the ENC_DATA register may be pending, which is
	 * not the case here. Therefore we just drain the AHB by
	 * polling the encoder/decoder status bits.)
	 */
	REG32(U_SCRB__SYN_RES_CTRL_REG)
		|= (PNIO_UINT32)U_SCRB__SYN_RES_CTRL_REG__SYN_RES_RS_CONTROLLER;
	while (REG32(U_RS_CONTROLLER1__DEC_Status)
			& (PNIO_UINT32)U_RS_CONTROLLER1__DEC_Status__run)
		;
	while (REG32(U_RS_CONTROLLER1__ENC_Status)
			& (PNIO_UINT32)U_RS_CONTROLLER1__ENC_Status__run)
		;
	while (REG32(U_RS_CONTROLLER2__DEC_Status)
			& (PNIO_UINT32)U_RS_CONTROLLER2__DEC_Status__run)
		;
	while (REG32(U_RS_CONTROLLER2__ENC_Status)
			& (PNIO_UINT32)U_RS_CONTROLLER2__ENC_Status__run)
		;
	REG32(U_SCRB__SYN_RES_CTRL_REG)
		&= (PNIO_UINT32)~U_SCRB__SYN_RES_CTRL_REG__SYN_RES_RS_CONTROLLER;

	/* turn off PFU (poll busy=0 twice, ADO 183080) */
	while (REG32(U_OSPI_SPFU__SPI_CTRL) & (PNIO_UINT32)U_OSPI_SPFU__SPI_CTRL__BUSY) ;
	while (REG32(U_OSPI_SPFU__SPI_CTRL) & (PNIO_UINT32)U_OSPI_SPFU__SPI_CTRL__BUSY) ;
	REG32(U_OSPI_SPFU__SPI_CTRL) = 0;
	
	/* disable OSPI IP */
	REG32(U_OSPI__SSIENR) = 0;

	/* in case we were interrupted during a write via GPIO, clear CLK and CSn outputs */
	REG32(U_GPIOX__GPIO_OUT_CLEAR_1) = 0x2; // CLK = 0
	REG32(U_GPIOX__GPIO_OUT_SET_1) = 0x1; // CSn = 1
	flash_ospi_direct_switch_gpio_off(info); // IP mode

	/* in case there was a running program/erase command, wait until the flash is ready */
	do {
		status = flash_ospi_cmd_read_statusreg(info);
	} while ((status & info->regs.prog_done_mask) != info->regs.prog_done_pol
			&& (status & info->regs.erase_done_mask) != info->regs.erase_done_pol);

	/* some flash need the error status be acklowledged */
	if ((info->regs.prog_error_mask != 0
			&& (status & info->regs.prog_error_mask)
				== info->regs.prog_error_pol)
		|| (info->regs.erase_error_mask != 0
			&& (status & info->regs.erase_error_mask)
				== info->regs.erase_error_pol)) {
		flash_ospi_cmd_clear_status(info);
	}

	/* reset SW state */
	info->erase_in_progress = 0;

	return 0;
}
#endif // BOARD_TYPE_STEP_3

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/