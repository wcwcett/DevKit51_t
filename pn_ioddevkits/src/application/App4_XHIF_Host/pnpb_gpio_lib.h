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
/*  F i l e               &F: pnpb_gpio_lib.h                           :F&  */
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


/**
* @file     pnpb_gpio_lib.h
* @brief    PNPB library for XHIF - GPIO functions
*/

#ifndef LIB_GPIO_H
#define LIB_GPIO_H

/* Includes ******************************************************************/

#include "pnio_types.h"
#include <stdbool.h>
#include <stdio.h>

/* Defines *******************************************************************/

#define USE_GPIO_REGISTER_POLL_AC
#define USE_GPIO_REGISTER_POLL_CYC

#define MMAP_SIZE               0x1000
#define SPRINTF_BUFFER_LEN      256

/* Sitara general GPIO registers */
#define GPIO_BANK_NUM			(4)
#define GPIO_BASE_0				(0x44E07000)
#define GPIO_BASE_1				(0x4804C000)
#define GPIO_BASE_2				(0x481AC000)
#define GPIO_BASE_3				(0x481AE000)

/* AM335x-specific */
#define OFFSET_OUTPUT_ENABLE	0x134
#define OFFSET_DATAIN			0x138
#define OFFSET_CLEAR			0x190
#define OFFSET_SET				0x194
#define OFFSET_FALLINGDETECT    0x14C
#define OFFSET_RISINGDETECT     0x148
#define OFFSET_IRQSTATUS        0x2C
#define OFFSET_IRQRAWSTATUS     0x24
#define OFFSET_IRQSTATUS2       0x30
#define OFFSET_IRQENABLE        0x34

typedef enum
{
    GPIO_BANK_0 = 0,
    GPIO_BANK_1 = 1,
    GPIO_BANK_2 = 2,
    GPIO_BANK_3 = 3
} GPIO_BANK;

typedef enum
{
    GPIO_NUM_0 = 0,
    GPIO_NUM_1 = 1,
    GPIO_NUM_2 = 2,
    GPIO_NUM_3 = 3,
    GPIO_NUM_4 = 4,
    GPIO_NUM_5 = 5,
    GPIO_NUM_6 = 6,
    GPIO_NUM_7 = 7,
    GPIO_NUM_8 = 8,
    GPIO_NUM_9 = 9,
    GPIO_NUM_10 = 10,
    GPIO_NUM_11 = 11,
    GPIO_NUM_12 = 12,
    GPIO_NUM_13 = 13,
    GPIO_NUM_14 = 14,
    GPIO_NUM_15 = 15,
    GPIO_NUM_16 = 16,
    GPIO_NUM_17 = 17,
    GPIO_NUM_18 = 18,
    GPIO_NUM_19 = 19,
    GPIO_NUM_20 = 20,
    GPIO_NUM_21 = 21,
    GPIO_NUM_22 = 22,
    GPIO_NUM_23 = 23,
    GPIO_NUM_24 = 24,
    GPIO_NUM_25 = 25,
    GPIO_NUM_26 = 26,
    GPIO_NUM_27 = 27,
    GPIO_NUM_28 = 28,
    GPIO_NUM_29 = 29,
    GPIO_NUM_30 = 30,
    GPIO_NUM_31 = 31
} GPIO_NUM;

/* Directories */
#define FILE_GPIO_EXPORT_DIR		"/sys/class/gpio/export"
#define FILE_GPIO_UNEXPORT_DIR		"/sys/class/gpio/unexport"
/* Usage of GPIO_DIR*/
#define FILE_GPIO_DIR				"/sys/class/gpio/gpio"

/* Data types ****************************************************************/

typedef enum
{
    bbb_gpio_input,
    bbb_gpio_output
} bbb_gpio_dir_t;

/* Function prototypes *******************************************************/

/**
 *  @brief Map all necessary GPIO registers to virtual memory.
 *
 *  @param[in]      devmem_fd   file descriptor of /dev/mem.
 *
 *  @return         Return PNPB_OK if success, otherwise PNPB_NOT_OK.
 *
 */
PNIO_UINT32 bbb_gpio_init(PNIO_UINT32 devmem_fd);

/**
 *  @brief Unmap all  GPIO registers from virtual memory.
 *
 *  @param          None.
 *
 *  @return         Return PNPB_OK if success, otherwise PNPB_NOT_OK.
 *
 */
PNIO_UINT32 bbb_gpio_deinit(void);

/**
 *  @brief Open and configure specified GPIO.
 *
 *  @param[in]      bank, num   Selected GPIO.
 *  @param[in]      dir         bbb_gpio_input / bbb_gpio_output
 *
 *  @return         Return PNPB_OK if success, otherwise PNPB_NOT_OK.
 *
 */
PNIO_UINT32 bbb_gpio_open(GPIO_BANK gpio_bank, GPIO_NUM gpio_num,
        bbb_gpio_dir_t dir);

/**
 *  @brief Close specified GPIO.
 *
 *  @param[in]      bank, num   Selected GPIO.
 *
 *  @return         Return PNPB_OK if success, otherwise PNPB_NOT_OK.
 *
 */
PNIO_UINT32 bbb_gpio_close(GPIO_BANK gpio_bank, GPIO_NUM gpio_num);

/**
 *  @brief Set output of GPIO.
 *
 *  @param[in]      bank, num   Selected GPIO.
 *  @param[in]      on          1 = set, 0 = clear
 *
 *  @return         Return PNPB_OK if success, otherwise PNPB_NOT_OK.
 *
 */
PNIO_UINT32 bbb_gpio_set(GPIO_BANK gpio_bank, GPIO_NUM gpio_num,
        PNIO_UINT32 on);

/**
 *  @brief Read value of GPIO.
 *
 *  @param[in]      bank, num   Selected GPIO.
 *
 *  @return         Return value of GPIO (0 or 1).
 *
 */
PNIO_UINT32 bbb_gpio_get(GPIO_BANK gpio_bank, GPIO_NUM gpio_num);

#endif /* LIB_GPIO_H */

/* End of file ***************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
