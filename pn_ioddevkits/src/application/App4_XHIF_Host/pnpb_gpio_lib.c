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
/*  F i l e               &F: pnpb_gpio_lib.c                           :F&  */
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
* @file     pnpb_gpio_lib.c
* @brief    PNPB library for XHIF - GPIO functions
*/

/* Includes ******************************************************************/

#include "pnpb_lib.h"
#include "pnpb_lib_int.h"
#include "pnpb_lib_mem_int.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "pnpb_gpio_lib.h"

/* Local constants ***********************************************************/

static const PNIO_UINT32 phys_gpio_base[GPIO_BANK_NUM] =
{
    GPIO_BASE_0,
    GPIO_BASE_1,
    GPIO_BASE_2,
    GPIO_BASE_3
};

/* Local variables ***********************************************************/

static void* gpio_base[GPIO_BANK_NUM];

/* Local function prototypes *************************************************/

static PNIO_UINT32 bbb_gpio_check_pin_mux(GPIO_BANK gpio_bank, GPIO_NUM gpio_num,
        bbb_gpio_dir_t dir);
static PNIO_UINT32 bbb_gpio_set_direction(GPIO_BANK gpio_bank, GPIO_NUM gpio_num,
        bbb_gpio_dir_t dir);
static PNIO_UINT32 bbb_gpio_export(GPIO_BANK gpio_bank, GPIO_NUM gpio_num);
static PNIO_UINT32 bbb_gpio_unexport(GPIO_BANK gpio_bank, GPIO_NUM gpio_num);

/* Exported function implementation ******************************************/

/**
 *  @brief Map all necessary GPIO registers to virtual memory.
 *
 *  @param[in]      devmem_fd   file descriptor of /dev/mem.
 *
 *  @return         Return PNPB_OK if success, otherwise PNPB_NOT_OK.
 *
 */
PNIO_UINT32 bbb_gpio_init(PNIO_UINT32 devmem_fd)
{
    PNIO_UINT32 rv = PNPB_OK;
    PNIO_UINT32 base_offset;
    PNIO_INT32 i;

    /* Map all GPIO banks */
    for (i = 0; i < GPIO_BANK_NUM; i++)
    {
        base_offset = phys_gpio_base[i];
        gpio_base[i] = mmap(0, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                devmem_fd, base_offset);

        if (gpio_base[i] == MAP_FAILED)
        {
            /* Trace */
            printf("GPIO registers map failed!\n");
            rv = PNPB_NOT_OK;
            break;
        }
    }

    return rv;
}

/*!****************************************************************************
 *  \brief      Close all used GPIOs.
 *
 *  \param      None.
 *
 *  \return     Return PNPB_OK if success, otherwise PNPB_NOT_OK.
 *****************************************************************************/
PNIO_UINT32 bbb_gpio_deinit(void)
{
    PNIO_INT32 i;

    /* Unmap all GPIO banks */
    for (i = 0; i < GPIO_BANK_NUM; i++)
    {
        munmap(gpio_base[i], MMAP_SIZE);
        gpio_base[i] = NULL;
    }

    return PNPB_OK;
}

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
        bbb_gpio_dir_t dir)
{
    PNIO_UINT32 ret;

    /* Export GPIOs*/
    ret = bbb_gpio_export(gpio_bank, gpio_num);
    if(ret != PNPB_OK)
    {
        printf("GPIO%d_%d export error!\n", gpio_bank, gpio_num);
        return ret;
    }

    /* Set direction */
    ret = bbb_gpio_set_direction(gpio_bank, gpio_num, dir);
    if(ret != PNPB_OK)
    {
        printf("GPIO%d_%d direction configuration error!\n", gpio_bank, gpio_num);
        return ret;
    }

    /* Check pinmux */
    ret = bbb_gpio_check_pin_mux(gpio_bank, gpio_num, dir);
    if(ret != PNPB_OK)
    {
        printf("GPIO%d_%d pinmux error!\n", gpio_bank, gpio_num);
        return ret;
    }

    return ret;
}

/**
 *  @brief Close specified GPIO.
 *
 *  @param[in]      bank, num   Selected GPIO.
 *
 *  @return         Return PNPB_OK if success, otherwise PNPB_NOT_OK.
 *
 */
PNIO_UINT32 bbb_gpio_close(GPIO_BANK gpio_bank, GPIO_NUM gpio_num)
{
    return bbb_gpio_unexport(gpio_bank, gpio_num);
}

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
        PNIO_UINT32 on)
{
    PNIO_UINT32 rv = PNPB_OK;
    volatile PNIO_UINT32* output;
    PNIO_UINT32 pinmask;

    pinmask = 1 << (PNIO_INT32) gpio_num;

    if (on == 0)
    {
        output = (PNIO_UINT32*) (gpio_base[(PNIO_INT32) gpio_bank] + OFFSET_CLEAR);
    }
    else
    {
        output = (PNIO_UINT32*) (gpio_base[(PNIO_INT32) gpio_bank] + OFFSET_SET);
    }

    /* Write value! */
    *output = pinmask;

    return rv;
}

/**
 *  @brief Read value of GPIO.
 *
 *  @param[in]      bank, num   Selected GPIO.
 *
 *  @return         Return value of GPIO (0 or 1).
 *
 */
PNIO_UINT32 bbb_gpio_get(GPIO_BANK gpio_bank, GPIO_NUM gpio_num)
{
    PNIO_UINT32 rv;
    volatile PNIO_UINT32* input;
    PNIO_UINT32 pinmask;

    pinmask = 1 << (PNIO_INT32) gpio_num;

    input = (PNIO_UINT32*) (gpio_base[(PNIO_INT32) gpio_bank] + OFFSET_DATAIN);

    /* Check value! */
    if((*input & pinmask) > 0)
    {
        rv = 1;
    }
    else
    {
        rv = 0;
    }

    return rv;
}

/* Local function implementation *********************************************/

static PNIO_UINT32 bbb_gpio_export(GPIO_BANK gpio_bank, GPIO_NUM gpio_num)
{
    PNIO_UINT32 rv = PNPB_OK;
    PNIO_INT32 fd;
    PNIO_INT8 gpio_buf[10];
    PNIO_INT32 gpio_number = gpio_bank * 32 + gpio_num;

    /* Open export file */
    fd = open(FILE_GPIO_EXPORT_DIR, O_WRONLY);
    if (fd < 0)
    {
        printf("File %s cannot be opened!\n", FILE_GPIO_EXPORT_DIR);
        return PNPB_NOT_OK;
    }

    /* Write GPIO number to file */
    sprintf(gpio_buf, "%i", gpio_number);
    write(fd, gpio_buf, strlen(gpio_buf) + 1);

    /* Close export file */
    close(fd);

    return rv;
}

static PNIO_UINT32 bbb_gpio_unexport(GPIO_BANK gpio_bank, GPIO_NUM gpio_num)
{
    PNIO_UINT32 rv = PNPB_OK;
    PNIO_INT32 fd, str_len;
    PNIO_INT8 gpio_buf[10];
    PNIO_INT32 gpio_number = gpio_bank * 32 + gpio_num;

    /* Open unexport file */
    fd = open(FILE_GPIO_UNEXPORT_DIR, O_WRONLY);
    if (fd < 0)
    {
        printf("File %s cannot be opened!\n", FILE_GPIO_UNEXPORT_DIR);
        return PNPB_NOT_OK;
    }

    /* Write GPIO number to file */
    sprintf(gpio_buf, "%i", gpio_number);
    str_len = strlen(gpio_buf) + 1;
    if (str_len != write(fd, gpio_buf, strlen(gpio_buf) + 1))
    {
        rv = PNPB_NOT_OK;
    }

    /* Close unexport file */
    close(fd);

    return rv;
}

static PNIO_UINT32 bbb_gpio_set_direction(GPIO_BANK gpio_bank, GPIO_NUM gpio_num,
        bbb_gpio_dir_t dir)
{
    PNIO_UINT32 rv = PNPB_OK;
    PNIO_INT32 fd, str_len;
    PNIO_INT8 dir_buf[SPRINTF_BUFFER_LEN];
    PNIO_INT32 gpio_number = gpio_bank * 32 + gpio_num;

    /* Open direction file */
    sprintf(dir_buf, "%s%i/direction", FILE_GPIO_DIR, gpio_number);
    fd = open(dir_buf, O_WRONLY);
    if (fd < 0)
    {
        printf("File %s cannot be opened!\n", dir_buf);
        return PNPB_NOT_OK;
    }

    /* Write direction to file */
    if (dir == bbb_gpio_input)
    {
        /* Write direction */
        str_len = write(fd, "in", 3);
        /* Check is write was successfull */
        if (str_len != 3)
        {
            rv = PNPB_NOT_OK;
        }
    }
    else
    {
        /* Write direction */
        str_len = write(fd, "out", 4);
        /* Check is write was successfull */
        if (str_len != 4)
        {
            rv = PNPB_NOT_OK;
        }
    }

    /* Close export file */
    close(fd);

    return rv;
}

static PNIO_UINT32 bbb_gpio_check_pin_mux(GPIO_BANK gpio_bank, GPIO_NUM gpio_num,
        bbb_gpio_dir_t dir)
{
    PNIO_UINT32 rv = PNPB_OK;
    PNIO_UINT32 pinmask;
    volatile PNIO_UINT32 *oe;

    pinmask = 1 << (PNIO_INT32) gpio_num;

    /* Check if the pin is properly muxed */
    oe = (PNIO_UINT32*) ((uintptr_t) gpio_base[(PNIO_INT32) gpio_bank]
            + (PNIO_UINT32) OFFSET_OUTPUT_ENABLE);

    if (dir == bbb_gpio_input)
    {
        /* 1 = input */
        if ((*oe & pinmask) != pinmask)
        {
            rv = PNPB_NOT_OK;
        }
    }
    else
    {
        /* 0 = output*/
        if ((*oe & pinmask) != 0)
        {
            rv = PNPB_NOT_OK;
        }
    }

    return rv;
}

/* End of file ***************************************************************/
      
/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
