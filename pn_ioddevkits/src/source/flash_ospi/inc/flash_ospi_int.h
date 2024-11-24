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
/*  F i l e               &F: flash_ospi_int.h                          :F&  */
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
#ifndef FLASH_OSPI_INT_H
#define FLASH_OSPI_INT_H

#ifdef BOARD_TYPE_STEP_3

#include "flash_ospi_devices.h"

# define FLASH_OSPI_REGISTER_MAX_CMD 8

typedef struct flash_ospi_register_cmd_struct_t {
	PNIO_UINT16 ospi_cmd;
	PNIO_UINT32 ospi_ctrlr0;           // Value Register U_OSPI__CTRLR0
	PNIO_UINT32 ospi_ctrlr1;           // Value Register U_OSPI__CTRLR1
	PNIO_UINT32 ospi_spi_ctrlr0;       // Value Register U_OSPI__SPI_CTRLR0
	PNIO_UINT32 spfu_spi_ctrl;         // Value Register U_OSPI__CTRLR0
	PNIO_UINT32 ospi_strobe_reg;       // Value Register U_SCRB2__OCTALSPI_STROBE_REG
	PNIO_UINT32 ospi_baudr;            // Value Register U_OSPI__BAUDR
	PNIO_UINT32 ospi_txd_drive_edge;   // Value Register U_OSPI__TXD_DRIVE_EDGE
} flash_ospi_register_cmd_struct_s;

struct flash_ospi_info; // forward declaration

typedef struct flash_ospi_register_struct_t {
	PNIO_UINT8 global_mode;                // Mode SPI,DUAL,QUAD,OCTAL
	PNIO_VOID (*setup_func)(const struct flash_ospi_info *);
	PNIO_UINT8 prog_done_mask;
	PNIO_UINT8 prog_done_pol;
	PNIO_UINT8 prog_error_mask;
	PNIO_UINT8 prog_error_pol;
	PNIO_UINT8 erase_done_mask;
	PNIO_UINT8 erase_done_pol;
	PNIO_UINT8 erase_error_mask;
	PNIO_UINT8 erase_error_pol;
	flash_ospi_register_cmd_struct_s cmd[FLASH_OSPI_REGISTER_MAX_CMD];
} flash_ospi_register_struct_s;

/* indices for commands in flash_ospi_register_struct_t.cmd */
#define FLASH_OSPI_READ_CMD          0
#define FLASH_OSPI_PROGRAM_CMD       1
#define FLASH_OSPI_WRITE_ENABLE_CMD  2
#define FLASH_OSPI_READ_STATUS_CMD   3
#define FLASH_OSPI_ERASE_SECTOR_CMD  4
#define FLASH_OSPI_ERASE_BLOCK_CMD   5
#define FLASH_OSPI_USER_CMD_0        6
#define FLASH_OSPI_USER_CMD_1        7

/* implemented in flash_ospi.c */
extern PNIO_VOID flash_ospi_trigger_watchdog(const struct flash_ospi_info * info);

/* defined in flash_ospi_devices.c */
extern const flash_ospi_register_struct_s flash_ospi_flash_param[FLASH_OSPI_FLASH_CNT];

/* construct OCTALSPI_STROBE_REG value */
#define FLASH_OSPI_STROBE_REG(mode, sck_delay, rxds_setup_cycles) (((mode) ? (PNIO_UINT32) 1 << 31 : 0) | ((PNIO_UINT32) (sck_delay) << 26) | ((PNIO_UINT32) (rxds_setup_cycles) << 16))

#endif // BOARD_TYPE_STEP_3

#endif // FLASH_OSPI_INT_H

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/