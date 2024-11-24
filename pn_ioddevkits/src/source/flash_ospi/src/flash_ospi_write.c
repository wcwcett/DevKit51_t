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
/*  F i l e               &F: flash_ospi_write.c                        :F&  */
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


//////////////////////////////////////////////////////
// Switch to I/O control
PNIO_VOID flash_ospi_direct_switch_gpio_on(const struct flash_ospi_info * info)
{
	do{} while (REG32(U_OSPI_SPFU__SPI_CTRL) & (PNIO_UINT32)U_OSPI_SPFU__SPI_CTRL__BUSY); // Wait for the prefetch unit to finish
	REG32(U_OSPI_SPFU__SPI_CTRL) = 0;  // Remove prefetch unit
	REG32(U_OSPI__SSIENR) = 0;         // Disable IP

	// Switch off alternate function
	info->gpio_config_func(info->gpio_config_data,
			0x00000000, info->gpio_port_mode, // portmode
			0x00000000, info->gpio_ioctrl, // ioctrl
			0x00000000, 0x00000000, // mux63_32
			0x00000000, 0x00000000); // pull47_32

	OCT_CS_ENABLE;
}

//////////////////////////////////////////////////////
// IP Controls the I/O again
PNIO_VOID flash_ospi_direct_switch_gpio_off(const struct flash_ospi_info * info)
{
	OCT_CS_DISABLE;

	// Switch on alternate functions
	info->gpio_config_func(info->gpio_config_data,
			info->gpio_port_mode, info->gpio_port_mode, // portmode
			0x00000000, 0x00000000, // ioctrl
			0x00000000, 0x00000000, // mux63_32
			0x00000000, 0x00000000); // pull47_32
}

///////////////////////////////////////////////////////
// Issue command
PNIO_VOID flash_ospi_direct_write_cmd(PNIO_UINT32 cmd, PNIO_UINT32 cmd_mode)
{
	PNIO_UINT32 i;
	PNIO_UINT32 scmd;

	switch (cmd_mode)
	{
		case FLASH_MODE_CMD_OCT_DDR16:
		{
			OCT_CLEAR_DATA;
			OCT_SET_DATA((cmd & 0xff00) >> 5);
			OCT_SET_CLK_1;
			OCT_CLEAR_DATA;
			OCT_SET_DATA((cmd & 0xff) << 3);
			OCT_SET_CLK_0;
			break;
		}
		case FLASH_MODE_CMD_OCT_SDR16:
		{
			OCT_CLEAR_DATA;
			OCT_SET_DATA((cmd & 0xff00) >> 5);
			OCT_SET_CLK_1;
			OCT_SET_CLK_0;
			OCT_CLEAR_DATA;
			OCT_SET_DATA((cmd & 0xff) << 3);
			OCT_SET_CLK_1;
			OCT_SET_CLK_0;
			break;
		}
		case FLASH_MODE_CMD_OCT_SDR8:
		{
			OCT_CLEAR_DATA;
			OCT_SET_DATA((cmd & 0xff) << 3);
			OCT_SET_CLK_1;
			OCT_SET_CLK_0;
			break;
		}
		case FLASH_MODE_CMD_QUAD_DDR8:
		{
			QUAD_CLEAR_DATA;
			QUAD_SET_DATA((cmd & 0xf0) >> 1);
			QUAD_SET_CLK_1;
			QUAD_CLEAR_DATA;
			QUAD_SET_DATA((cmd & 0x0f) << 3);
			QUAD_SET_CLK_0;
			break;
		}
		case FLASH_MODE_CMD_QUAD_SDR8:
		{	QUAD_CLEAR_DATA;
			QUAD_SET_DATA((cmd & 0xf0) >> 1);
			QUAD_SET_CLK_1;
			QUAD_SET_CLK_1;
			QUAD_SET_CLK_0;
			QUAD_CLEAR_DATA;
			QUAD_SET_DATA((cmd & 0x0f) << 3);
			QUAD_SET_CLK_1;
			QUAD_SET_CLK_1;
			QUAD_SET_CLK_0;
			QUAD_CLEAR_DATA;
			break;
		}
		case FLASH_MODE_CMD_SINGLE:
		{
			OCT_CS_DISABLE;
			scmd = cmd;
			OCT_SET7_DATA;
			OCT_CS_ENABLE;
			for (i = 0; i < 8; i++)
			{
				if (scmd & 0x80) {
					OCT_SET_DQ0;
				} else {
					OCT_CLEAR_DQ0;
				}
				QUAD_SET_CLK_1;
				scmd = scmd << 1;
				QUAD_SET_CLK_0;
			}
			break;
		}
		default:
		{
			PNIO_ConsolePrintf("Unsupported cmd_mode : %d\n", cmd_mode);
			break;
		}
	}
}

PNIO_VOID flash_ospi_direct_write_addr(PNIO_UINT32 addr, PNIO_UINT32 mode, PNIO_UINT32 fill)
{
	PNIO_UINT32 addr_shift;
	PNIO_UINT32 addr_z;
	PNIO_UINT32 addr_len;
	PNIO_UINT32 addr_mode;
	PNIO_UINT32 i;

	if (mode == FLASH_MODE_ADDR_NULL)
		return;

	// since the MSB is output, an address shift is carried out.
	addr_mode = mode & ~(0xf<<2);
	addr_len = (mode & (0xf<<2));

	if (addr_len == 0) {  // if I previously had MODE_NULL
		return;
	} else if (addr_len == 8) {
		addr_z = 1;
		addr_shift = addr & 0xff;
	} else if (addr_len == 16) {
		addr_z = 2;
		addr_shift = (addr & 0xff00) >> 8 | (addr & 0xff) << 8;
	} else if (addr_len == 24) {
		addr_z = 3;
		if (fill) {
			addr_shift = (addr & 0xff0000) >> 16 | (addr & 0x00ff00) | (addr & 0x0000fe) << 16;
		} else {
			addr_shift = (addr & 0xff0000) >> 16 | (addr & 0x00ff00) | (addr & 0x0000ff) << 16;
		}
	} else if (addr_len == 32) {
		addr_z = 4;
		if (fill) {
			addr_shift = (addr & 0xff000000) >> 24 | (addr & 0x00ff0000) >> 8 | (addr & 0x0000ff00) << 8 | (addr & 0x000000fe) << 24;
		} else {
			addr_shift = (addr & 0xff000000) >> 24 | (addr & 0x00ff0000) >> 8 | (addr & 0x0000ff00) << 8 | (addr & 0x000000ff) << 24;
		}
	} else {
		return;
	}

	switch (addr_mode)
	{
		case FLASH_MODE_ADDR_OCT_DDR:
		{
			for (i=0; i<addr_z / 2; i++)
			{
				OCT_CLEAR_DATA;
				OCT_SET_DATA((addr_shift & 0xff) << 3);
				OCT_SET_CLK_1;
				OCT_CLEAR_DATA;
				OCT_SET_DATA((addr_shift & 0xff00) >> 5);
				OCT_SET_CLK_0;
				addr_shift = addr_shift >> 16;
			}
			break;
		}
		case FLASH_MODE_ADDR_OCT_SDR:
		{
			for (i=0; i<addr_z; i++)
			{
				OCT_CLEAR_DATA;
				OCT_SET_DATA((addr_shift & 0xff) << 3);
				OCT_SET_CLK_1;
				addr_shift = addr_shift >> 8;
				OCT_SET_CLK_0;
			}
			break;
		}
		case FLASH_MODE_ADDR_QUAD_DDR:
		{
			for (i=0; i<addr_z; i++)
			{
				QUAD_CLEAR_DATA;
				QUAD_SET_DATA((addr_shift & 0xf0) >> 1);
				QUAD_SET_CLK_1;
				QUAD_CLEAR_DATA;
				QUAD_SET_DATA((addr_shift & 0x0f) << 3);
				QUAD_SET_CLK_0;
				addr_shift = addr_shift >> 8;
			}
			break;
		}
		case FLASH_MODE_ADDR_QUAD_SDR:
		{
			for (i=0; i<addr_z; i++)
			{
				QUAD_CLEAR_DATA;
				QUAD_SET_DATA((addr_shift & 0xf0) >> 1);
				QUAD_SET_CLK_1;
				QUAD_SET_CLK_1;
				QUAD_SET_CLK_0;
				QUAD_CLEAR_DATA;
				QUAD_SET_DATA((addr_shift & 0x0f) << 3);
				QUAD_SET_CLK_1;
				addr_shift = addr_shift >> 8;
				QUAD_SET_CLK_0;
			}
			break;
		}
		default:
		{
			PNIO_ConsolePrintf("Unsupported addr_mode : %d\n", addr_mode);
			break;
		}
	}
}

PNIO_VOID flash_ospi_direct_write_data(PNIO_UINT8 * data, PNIO_UINT32 data_len, PNIO_UINT32 data_mode,
		PNIO_UINT32 fill)
{
	PNIO_UINT32 i;
	PNIO_UINT8 * p8;
	PNIO_UINT32 data_shift;
	PNIO_UINT32 data_z;

	if (data_mode == FLASH_MODE_DATA_NULL)
		return;

	p8 = (PNIO_UINT8 *) data;
	for (data_z = 0; data_z < data_len; )
	{
		if (fill) {
			fill = 0;
			data_shift = 0xff; // only run once and do not increase the number 
		} else {
			if (data_z++ < data_len) {
				data_shift = (PNIO_UINT32)*p8++;
			} else {
				data_shift = 0xff;
			}
		}

		if (data_z++ < data_len) {
			data_shift |= (PNIO_UINT32)*p8++ << 8;
		} else {
			data_shift |= (PNIO_UINT32) 0xff << 8;
		}

		if (data_z++ < data_len) {
			data_shift |= (PNIO_UINT32)*p8++ << 16;
		} else {
			data_shift |= (PNIO_UINT32) 0xff << 16;
		}

		if (data_z++ < data_len) {
			data_shift |= (PNIO_UINT32)*p8++ << 24;
		} else {
			data_shift |= (PNIO_UINT32) 0xff << 24;
		}

		switch (data_mode)
		{
			case FLASH_MODE_DATA_OCT_DDR:
			{
				for (i=0; i<2; i++)
				{
					OCT_CLEAR_DATA;
					OCT_SET_DATA((data_shift & 0xff) << 3);
					OCT_SET_CLK_1;
					OCT_CLEAR_DATA;
					OCT_SET_DATA((data_shift & 0xff00) >> 5);
					OCT_SET_CLK_0;
					data_shift = data_shift >> 16;
				}
				break;
			}
			case FLASH_MODE_DATA_OCT_SDR:
			{
				for (i=0; i<4; i++)
				{
					OCT_CLEAR_DATA;
					OCT_SET_DATA((data_shift & 0xff) << 3);
					OCT_SET_CLK_1;
					data_shift = data_shift >> 8;
					OCT_SET_CLK_0;
				}
				break;
			}
			case FLASH_MODE_DATA_QUAD_DDR:
			{
				for (i=0; i<4; i++)
				{
					QUAD_CLEAR_DATA;
					QUAD_SET_DATA((data_shift & 0xf0) >> 1);
					QUAD_SET_CLK_1;
					QUAD_CLEAR_DATA;
					QUAD_SET_DATA((data_shift & 0x0f) << 3);
					QUAD_SET_CLK_0;
					data_shift = data_shift >> 8;
				}
				break;
			}
			case FLASH_MODE_DATA_QUAD_SDR:
			{
				for (i=0; i<4; i++)
				{
					QUAD_CLEAR_DATA;
					QUAD_SET_DATA((data_shift & 0xf0) >> 1);
					QUAD_SET_CLK_1;
					QUAD_SET_CLK_0;
					QUAD_CLEAR_DATA;
					QUAD_SET_DATA((data_shift & 0x0f) << 3);
					QUAD_SET_CLK_1;
					data_shift = data_shift >> 8;
					QUAD_SET_CLK_0;
				}
				break;
			}
			default:
			{
				PNIO_ConsolePrintf("Unsupported data_mode : %d\n", data_mode);
				break;
			}
		}
	}
}

PNIO_UINT32 flash_ospi_cmd_read_statusreg(const struct flash_ospi_info * info)
{
	PNIO_UINT32 retval;

	flash_ospi_setup_register(info, FLASH_OSPI_READ_STATUS_CMD);  // Set to read status
	if (info->regs.cmd[FLASH_OSPI_READ_STATUS_CMD].ospi_spi_ctrlr0 & 0x3c) {
		REG32(U_OSPI__DR0) = info->regs.cmd[FLASH_OSPI_READ_STATUS_CMD].ospi_cmd; // Still need to send address
		REG32(U_OSPI__DR0) = 0;
	} else {
		REG32(U_OSPI__DR0) = info->regs.cmd[FLASH_OSPI_READ_STATUS_CMD].ospi_cmd;
	}
	do {} while ((REG32(U_OSPI__SR) & (PNIO_UINT32)U_OSPI__SR__TFE) == 0x0);
	do {} while ((REG32(U_OSPI__SR) & (PNIO_UINT32)U_OSPI__SR__BUSY) == (PNIO_UINT32)U_OSPI__SR__BUSY); // Finished

	retval = REG32(U_OSPI__DR0);
	return (retval);
}

PNIO_VOID flash_ospi_cmd_write_enable(const struct flash_ospi_info * info)
{
	flash_ospi_setup_register(info, FLASH_OSPI_WRITE_ENABLE_CMD);
	REG32(U_OSPI__DR0) = info->regs.cmd[FLASH_OSPI_WRITE_ENABLE_CMD].ospi_cmd;
	do {} while ((REG32(U_OSPI__SR) & (PNIO_UINT32)U_OSPI__SR__TFE) == 0x0);
	do {} while ((REG32(U_OSPI__SR) & (PNIO_UINT32)U_OSPI__SR__BUSY) == (PNIO_UINT32)U_OSPI__SR__BUSY); // Finished
}

PNIO_VOID flash_ospi_cmd_erase_sector(const struct flash_ospi_info * info, PNIO_UINT32 addr)
{
	// With data length=0, OSPI-IP outputs one more byte after the address. 
	// This causes some memories to ignore the command. So we use GPIO instead of IP.
	flash_ospi_direct_switch_gpio_on(info);
	flash_ospi_direct_write_cmd(info->regs.cmd[FLASH_OSPI_ERASE_SECTOR_CMD].ospi_cmd, info->erasesector_cmd_mode);
	flash_ospi_direct_write_addr(addr, info->erasesector_addr_mode, 0);
	flash_ospi_direct_switch_gpio_off(info);
}

PNIO_VOID flash_ospi_cmd_erase_block(const struct flash_ospi_info * info, PNIO_UINT32 addr)
{
	// With data length=0, OSPI-IP outputs one more byte after the address. 
	// This causes some memories to ignore the command. So we use GPIO instead of IP.
	flash_ospi_direct_switch_gpio_on(info);
	flash_ospi_direct_write_cmd(info->regs.cmd[FLASH_OSPI_ERASE_BLOCK_CMD].ospi_cmd, info->eraseblock_cmd_mode);
	flash_ospi_direct_write_addr(addr, info->eraseblock_addr_mode, 0);
	flash_ospi_direct_switch_gpio_off(info);
}

PNIO_UINT32 flash_ospi_cmd_get_erase_status(const struct flash_ospi_info * info)
{
	PNIO_UINT32 status;

	// now ask if done
	status = flash_ospi_cmd_read_statusreg(info);
	if ((status & info->regs.erase_done_mask) != info->regs.erase_done_pol) {
		// still active
		return 0;
	}

	// done, return error status
	if (info->regs.erase_error_mask != 0
			&& (status & info->regs.erase_error_mask) == info->regs.erase_error_pol) {
		// With some flashes, the error status must be acknowledged
		flash_ospi_cmd_clear_status(info);

		// mistake
		return 2;
	} else {
		// OK
		return 1;
	}
}

PNIO_UINT32 flash_ospi_cmd_program(const struct flash_ospi_info * info, PNIO_UINT8 * source, PNIO_UINT32 anz,
		PNIO_UINT32 oct_addr)
{
	PNIO_UINT32 status;
	PNIO_UINT32 i;
	PNIO_UINT8 *p8s;
	PNIO_UINT32 canz;
	PNIO_UINT32 storeaddr;
	PNIO_UINT32 storeanz;
	PNIO_UINT32 fill;
	PNIO_UINT32 errors = 0;

	storeaddr = oct_addr;
	storeanz = anz;
	p8s = source;
	for (i = 0; i < storeanz; ) {
		// here the 256 cache size must be taken into account.
		// Top up if necessary
		// If bigger, then in 256 bites
		// spend the rest
		if (storeaddr & 0xff) { // OK I have to fill up the page
			canz = 0x100 - (storeaddr & 0xff);
			if (storeanz < canz)
				canz = storeanz;
			i += canz;
		} else {
			canz = 0x100;  // always on 256 granular
			if (i + canz > storeanz)
				canz = storeanz - i; // rest must be written
			i += canz;
		}

		flash_ospi_cmd_write_enable(info);

		//With DDR and an odd address, 1 dummy data byte (0xff) must be written 
		if ((storeaddr & 0x1) && (info->program_addr_mode & FLASH_MODE_DDR)) {
			fill = 1; // I have to insert 1 byte
		} else {
			fill = 0;
		}

		// write command
		flash_ospi_direct_switch_gpio_on(info);
		flash_ospi_direct_write_cmd(info->regs.cmd[FLASH_OSPI_PROGRAM_CMD].ospi_cmd, info->program_cmd_mode);
		flash_ospi_direct_write_addr(storeaddr, info->program_addr_mode, fill);
		flash_ospi_direct_write_data(p8s, canz, info->program_data_mode, fill);
		flash_ospi_direct_switch_gpio_off(info);

		storeaddr += canz; // Add addresses
		p8s = source + i; // Src Ptr continue

		// now ask if done
		do {
			status = flash_ospi_cmd_read_statusreg(info);
		} while ((status & info->regs.prog_done_mask) != info->regs.prog_done_pol);

		// Return error status but continue
		if (info->regs.prog_error_mask != 0
				&& (status & info->regs.prog_error_mask) == info->regs.prog_error_pol) {
			// With some flashes, the error status must be acknowledged
			flash_ospi_cmd_clear_status(info);

			// mistake
			errors++;
		}
	}

	return (errors);
}

PNIO_VOID flash_ospi_cmd_clear_status(const struct flash_ospi_info * info)
{
	// not present in all flashes
	if (info->regs.cmd[FLASH_OSPI_USER_CMD_0].ospi_cmd == 0) {
		return;
	}

	// With data length=0, OSPI-IP outputs one more byte after the address. 
	// This causes some memories to ignore the command. So we use GPIO instead of IP.
	flash_ospi_direct_switch_gpio_on(info);
	flash_ospi_direct_write_cmd(info->regs.cmd[FLASH_OSPI_USER_CMD_0].ospi_cmd, info->user_cmd_0_cmd_mode);
	flash_ospi_direct_switch_gpio_off(info);
}

/**
 * @brief erases one block of the flash
 *
 * @param info		pointer to FLASH info structure
 * @param block_addr	address of the block (device specific, most devices allow
 * 			any address within the block)
 *
 * @retval 0		OK
 * @retval PNIO_FLASH_BUSY_ERR	erase in progress
 *
 * @note This function returns immediately after sending the erase command
 *       to the flash. get_erase_status must be used to poll for completion.
 *
 * @note Block and sector sizes are device specific. The FLASH does not have
 *       knowledge about the memory organization. What a 'block' is depends
 *       on the definition of erase command in the active parameter set.
 */
PNIO_UINT32 flash_ospi_erase_block(struct flash_ospi_info * info, PNIO_UINT32 block_addr)
{
	/* fail if erase is in progress */
	if (info->erase_in_progress) {
		PNIO_ConsolePrintf("ERROR: erase in progress.\n");
		return PNIO_FLASH_BUSY_ERR;
	}

	flash_ospi_cmd_write_enable(info);
	flash_ospi_cmd_erase_block(info, block_addr);
	info->erase_in_progress = 1;
	return 0;
}

/**
 * @brief erases one sector of the flash
 *
 * @param info		pointer to FLASH info structure
 * @param sector_addr	address of the sector (device specific, most devices allow
 * 			any address within the sector)
 *
 * @retval 0		OK
 * @retval PNIO_FLASH_BUSY_ERR	erase in progress
 *
 * @note This function returns immediately after sending the erase command
 *       to the flash. get_erase_status must be used to poll for completion.
 *
 * @note Block and sector sizes are device specific. The FLASH does not have
 *       knowledge about the memory organization. What a 'sector' is depends
 *       on the definition of erase command in the active parameter set.
 */
PNIO_UINT32 flash_ospi_erase_sector(struct flash_ospi_info * info, PNIO_UINT32 sector_addr)
{
	/* fail if erase is in progress */
	if (info->erase_in_progress) {
		PNIO_ConsolePrintf("ERROR: erase in progress.\n");
		return PNIO_FLASH_BUSY_ERR;
	}

	flash_ospi_cmd_write_enable(info);
	flash_ospi_cmd_erase_sector(info, sector_addr);
	info->erase_in_progress = 1;
	return 0;
}

/**
 * @brief gets the status of a running erase command
 *
 * @param info		pointer to FLASH info structure
 *
 * @retval 0			OK, erase finished
 * @retval >0			erase still in progress
 * @retval PNIO_FLASH_MEM_ERR		flash erase operation failed
 * @retval PNIO_FLASH_NOTALLOWED_ERR	no erase command running
 */
PNIO_UINT32 flash_ospi_get_erase_status(struct flash_ospi_info * info)
{
	PNIO_UINT32 status;
	PNIO_UINT32 ret;

	if (!info->erase_in_progress) {
		PNIO_ConsolePrintf("ERROR: no erase command running.\n");
		return PNIO_FLASH_NOTALLOWED_ERR;
	}

	status = flash_ospi_cmd_get_erase_status(info);
	if (status == 0) {
		/* still running */
		ret = 1;
	} else if (status == 1) {
		/* finished, OK */
		ret = 0;
		info->erase_in_progress = 0;
	} else {
		/* finished, error */
		PNIO_ConsolePrintf("ERROR: flash erase operation failed.\n");
		ret = PNIO_FLASH_MEM_ERR;
		info->erase_in_progress = 0;
	}

	/* switch back to read mode */
	flash_ospi_set_read_mode(info);

	return ret;
}

/**
 * @brief write data to flash
 *
 * @param info		pointer to FLASH info structure
 * @param mem_addr	byte address in flash memory
 * @param src		virtual source address
 * @param len		number of bytes to write
 *
 * @retval 0		OK
 * @retval PNIO_FLASH_MEM_ERR	flash program operation failed
 * @retval PNIO_FLASH_BUSY_ERR	erase in progress
 */
PNIO_UINT32 flash_ospi_write(const struct flash_ospi_info * info, PNIO_UINT32 mem_addr, PNIO_VOID * src,
		size_t len)
{
	PNIO_UINT32 ret = 0;

	/* fail if erase is in progress */
	if (info->erase_in_progress) {
		PNIO_ConsolePrintf("ERROR: erase in progress.\n");
		return PNIO_FLASH_BUSY_ERR;
	}

	/* flash_ospi_cmd_write_enable is called internally by flash_ospi_cmd_program */

	if (flash_ospi_cmd_program(info, src, len, mem_addr) != 0) {
		PNIO_ConsolePrintf("ERROR: flash program operation failed.\n");
		ret = PNIO_FLASH_MEM_ERR;
	}

	/* switch back to read mode */
	flash_ospi_set_read_mode(info);

	return ret;
}

static PNIO_VOID flash_ospi_copy_to_rsc_encoder(const PNIO_VOID * src, size_t len)
{
	PNIO_UINT32 i;

	if (((PNIO_UINT32) src & 0x3) == 0 && (len & 0x3) == 0) {
		/* copy 4 byte wise */
		volatile PNIO_UINT32 * src32 = (PNIO_UINT32 *) src;
		for (i=0; i<len; i+=4) {
			REG32(U_RS_CONTROLLER1__ENC_Data) = *src32++;
		}
	} else if (((PNIO_UINT32) src & 0x1) == 0 && (len & 0x1) == 0) {
		/* copy 2 byte wise */
		volatile PNIO_UINT16 * src16 = (PNIO_UINT16 *) src;
		for (i=0; i<len; i+=2) {
			REG16(U_RS_CONTROLLER1__ENC_Data) = *src16++;
		}
	} else {
		/* copy byte wise */
		volatile PNIO_UINT8 * src8 = (PNIO_UINT8 *) src;
		for (i=0; i<len; i++) {
			REG8(U_RS_CONTROLLER1__ENC_Data) = *src8++;
		}
	}
}

/**
 * @brief encode data with RSC and write to flash
 *
 * @param info		pointer to FLASH info structure
 * @param mem_addr	byte address in flash memory
 * @param src		virtual source address
 * @param rsc_page_size	size of RSC page (max. 238)
 * @param npages	number of RSC pages to write
 *
 * @retval 0		OK
 * @retval PNIO_FLASH_PARAM_ERR	illegal value for rsc_page_size
 * @retval PNIO_FLASH_MEM_ERR	flash program operation failed
 * @retval PNIO_FLASH_BUSY_ERR	erase in progress
 *
 * @note When writing multiple pages, they are written to the flash without gap, i.e.
 *       with rsc_page_size=236, the start address of the second page is 236+16=252 bytes
 *       after the start address of the first page. (16 bytes checksum)
 */
PNIO_UINT32 flash_ospi_write_rsc(const flash_ospi_info_t * info, PNIO_UINT32 mem_addr, PNIO_VOID * src,
		PNIO_UINT32 rsc_page_size, size_t npages)
{
	PNIO_UINT32 prog_errors = 0;

	/* fail if erase is in progress */
	if (info->erase_in_progress) {
		PNIO_ConsolePrintf("ERROR: erase in progress.\n");
		return PNIO_FLASH_BUSY_ERR;
	}

	if (rsc_page_size == 0 || rsc_page_size > 238) {
		PNIO_ConsolePrintf("ERROR: illegal value for rsc_page_size.\n");
		return PNIO_FLASH_PARAM_ERR ;
	}

	/* just in case a previous job is still running */
	if (REG32(U_RS_CONTROLLER1__ENC_Status)
			& (PNIO_UINT32)U_RS_CONTROLLER1__ENC_Status__run) {
		PNIO_ConsolePrintf("ERROR: erase in progress.\n");
		return PNIO_FLASH_BUSY_ERR;
	}

	REG32(U_RS_CONTROLLER1__ENC_DSTmode)
		= U_RS_CONTROLLER1__ENC_DSTmode__bypass;
	REG32(U_RS_CONTROLLER1__ENC_Length) = rsc_page_size;

	while (npages--) {
		/* copy data to encoder */
		flash_ospi_copy_to_rsc_encoder(src, rsc_page_size);
		src = (PNIO_VOID *) ((PNIO_UINT8 *) src + rsc_page_size);

		/* wait until encoder is finished */
		while (REG32(U_RS_CONTROLLER1__ENC_Status)
				& (PNIO_UINT32)U_RS_CONTROLLER1__ENC_Status__run)
			;

		/* now write to flash */
		if (flash_ospi_cmd_program(info,
				(PNIO_UINT8 *) (U_RS_CONTROLLER1_RSC_RAM__start),
				rsc_page_size + RSC_BYTES, mem_addr) != 0) {
			prog_errors++;
		}

		mem_addr += rsc_page_size + RSC_BYTES;
	}

	/* switch back to read mode */
	flash_ospi_set_read_mode(info);

	if (prog_errors) {
		PNIO_ConsolePrintf("ERROR: flash program operation failed.\n");
		return PNIO_FLASH_MEM_ERR;
	} else {
		return 0;
	}
}

#endif // BOARD_TYPE_STEP_3

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/