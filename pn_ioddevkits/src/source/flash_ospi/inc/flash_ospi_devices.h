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
/*  F i l e               &F: flash_ospi_devices.h                      :F&  */
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
#ifndef FLASH_OSPI_DEVICES_H
#define FLASH_OSPI_DEVICES_H

#ifdef BOARD_TYPE_STEP_3

#define WRITE_ENABLE 0x06
#define WRITE_VOLATILE_CONFIG_REG 0x81
#define WRITE_ENHANCED_VOLATILE_CONFIG_REG 0x61

/**
 * @brief list of defined flash parameter sets
 *
 * @details If one flash device is used in different configurtions on different
 *          PCBs, one ID must be defined for each configuration, e.g.
 *          FLASH_OSPI_FLASH_MT35XU512_OCT_MODE
 *          FLASH_OSPI_FLASH_MT35XU512_QUAD_MODE
 *          FLASH_OSPI_FLASH_MT35XU512_QUAD_MODE
 */
enum flash_ospi_flash_param_id {
	FLASH_OSPI_FLASH_NONE = 0, /** reserved */
	FLASH_OSPI_FLASH_MX25UM512,
	FLASH_OSPI_FLASH_MT25QU128,
	FLASH_OSPI_FLASH_MT35XU512,
	FLASH_OSPI_FLASH_MX25L12845_SDR,
	FLASH_OSPI_FLASH_MX25L12845_DDR,
	FLASH_OSPI_FLASH_MT25QL128,
	FLASH_OSPI_FLASH_W25Q128JVSJM,
	FLASH_OSPI_FLASH_CNT /** must be last entry */
};

#endif // BOARD_TYPE_STEP_3

#endif // FLASH_OSPI_DEVICES_H

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
