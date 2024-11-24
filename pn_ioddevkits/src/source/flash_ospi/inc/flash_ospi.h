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
/*  F i l e               &F: flash_ospi.h                              :F&  */
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
#ifndef FLASH_OSPI_H
#define FLASH_OSPI_H

#ifdef BOARD_TYPE_STEP_3

#include <stddef.h>

#include "flash_ospi_int.h"
#include "flash_ospi_devices.h"
#include "lsa_cfg.h"
#include "pnio_types.h"

extern volatile PNIO_UINT32 flash_dummy32;

typedef enum 
{
	PNIO_FLASH_PARAM_ERR = 2,
	PNIO_FLASH_ADDR_ERR ,
	PNIO_FLASH_SIZE_ERR,
	PNIO_FLASH_ALIGN_ERR,
	PNIO_FLASH_MEM_ERR,
	PNIO_FLASH_BUSY_ERR,
	PNIO_FLASH_NOTALLOWED_ERR
	
} PNIO_FLASH_OSPI_ERR_ENUM;

/**
 * @brief internal data structure for FLASH
 *
 * @details Must be allocated by the application and passed to most FLASH
 *          functions. The information in the structure must not be
 *          touched by the application!
 */
typedef struct flash_ospi_info {
	flash_ospi_register_struct_s regs;
	PNIO_UINT32 gpio_port_mode;
	PNIO_UINT32 gpio_ioctrl;
	PNIO_UINT32 program_cmd_mode;
	PNIO_UINT32 program_addr_mode;
	PNIO_UINT32 program_data_mode;
	PNIO_UINT32 erasesector_cmd_mode;
	PNIO_UINT32 erasesector_addr_mode;
	PNIO_UINT32 eraseblock_cmd_mode;
	PNIO_UINT32 eraseblock_addr_mode;
	PNIO_UINT32 user_cmd_0_cmd_mode;
	PNIO_UINT32 user_cmd_0_addr_mode;

	PNIO_VOID (*gpio_config_func)(const PNIO_VOID * data,
				PNIO_UINT32 portmode, PNIO_UINT32 portmode_mask,
				PNIO_UINT32 ioctrl, PNIO_UINT32 ioctrl_mask,
				PNIO_UINT32 mux63_32, PNIO_UINT32 mux63_32_mask,
				PNIO_UINT32 pull47_32, PNIO_UINT32 pull47_32_mask);
	PNIO_VOID * gpio_config_data;
	PNIO_VOID (*wait_us_func)(PNIO_VOID * data, PNIO_UINT32 us);
	PNIO_VOID * wait_us_data;
	PNIO_VOID (*watchdog_trigger_func)(PNIO_VOID * data);
	PNIO_VOID * watchdog_trigger_data;

	PNIO_UINT32 erase_in_progress;
} flash_ospi_info_t;

/**
 * @brief parameters for flash_ospi_open
 *
 * @note This structure should be zeroed (e.g. with memset) before filling the fields
 *       to make sure that unused fields are defined 0.
 */
struct flash_ospi_open_param {
	/** pointer to callback function that configures GPIOs
	 *  (see flash_ospi_test.c for an example)
	 *
	 * @param data			callback data
	 * @param portmode		bit values to set in U_GPIOX__GPIO_PORT_MODE_1_L
	 * @param portmode_mask 	bits to change in U_GPIOX__GPIO_PORT_MODE_1_L
	 * @param ioctrl		bit values to set in U_GPIOX__GPIO_IOCTRL_1
	 * @param ioctrl_mask 		bits to change in U_GPIOX__GPIO_IOCTRL_1
	 * @param mux63_32		bit values to set in U_SCRB2__MUX63_32GPIO
	 * @param mux63_32_mask 	bits to change in U_SCRB2__MUX63_32GPIO
	 * @param pull47_32		bit values to set in U_SCRB__PULL47_32GPIO
	 * @param pull47_32_mask 	bits to change in U_SCRB__PULL47_32GPIO
	 */
	PNIO_VOID (*gpio_config_func)(const PNIO_VOID * data,
				PNIO_UINT32 portmode, PNIO_UINT32 portmode_mask,
				PNIO_UINT32 ioctrl, PNIO_UINT32 ioctrl_mask,
				PNIO_UINT32 mux63_32, PNIO_UINT32 mux63_32_mask,
				PNIO_UINT32 pull47_32, PNIO_UINT32 pull47_32_mask);
	/** callback data that will be passed to each call of gpio_config_func() */
	PNIO_VOID * gpio_config_data;

	/** pointer to callback function that waits a specific amount of us
	 *
	 * @param data			callback data
	 * @param us			number of microseconds that shall be waited
	 */
	PNIO_VOID (*wait_us_func)(PNIO_VOID * data, PNIO_UINT32 us);
	/** callback data that will be passed to each call of wait_us_func() */
	PNIO_VOID * wait_us_data;

	/** pointer to callback function that retriggers the watchdog during long operations
	 *  (may be NULL if no watchdog triggering is wanted)
	 *
	 * @param data			callback data
	 */
	PNIO_VOID (*watchdog_trigger_func)(PNIO_VOID * data);
	/** callback data that will be passed to each call of watchdog_trigger_func() */
	PNIO_VOID * watchdog_trigger_data;

};

#define FLASH_OSPI_MODE_SINGLE	0x00
#define FLASH_OSPI_MODE_DUAL	0x01
#define FLASH_OSPI_MODE_QUAD	0x02
#define FLASH_OSPI_MODE_OCTAL	0x03

#define FLASH_OSPI_SERVICE_REG (U_SCRB2__SERVICE_1_REG)

#define RSC_BYTES 16

#define OCT_CS_ENABLE		(REG32(U_GPIOX__GPIO_OUT_CLEAR_1)= 0x00000001)
#define OCT_CS_DISABLE		(REG32(U_GPIOX__GPIO_OUT_SET_1)  = 0x00000001)

#define OCT_SET_CLK_1		(REG32(U_GPIOX__GPIO_OUT_SET_1) = 0x02)
#define OCT_SET_CLK_0		(REG32(U_GPIOX__GPIO_OUT_CLEAR_1) = 0x02)
#define OCT_CLEAR_DATA		(REG32(U_GPIOX__GPIO_OUT_CLEAR_1) = 0x7f8)
#define OCT_SET7_DATA		(REG32(U_GPIOX__GPIO_OUT_SET_1) = 0x7f0)
#define OCT_SET_DATA(x)		(REG32(U_GPIOX__GPIO_OUT_SET_1) = (x))
#define OCT_SET_DQ0			(REG32(U_GPIOX__GPIO_OUT_SET_1) = 0x008)
#define OCT_CLEAR_DQ0		(REG32(U_GPIOX__GPIO_OUT_CLEAR_1) = 0x008)

#define QUAD_SET_CLK_1  	(REG32(U_GPIOX__GPIO_OUT_SET_1) = 0x02)
#define QUAD_SET_CLK_0  	(REG32(U_GPIOX__GPIO_OUT_CLEAR_1) = 0x02)
#define QUAD_CLEAR_DATA 	(REG32(U_GPIOX__GPIO_OUT_CLEAR_1) = 0x078)
#define QUAD_SET_DATA(x)	(REG32(U_GPIOX__GPIO_OUT_SET_1) = (x))

#define FLASH_WRITE_GLOBAL_MODE_OCTAL  0x80000000
#define FLASH_WRITE_GLOBAL_MODE_QUAD   0x40000000

#define FLASH_MODE_DDR		(1<<16)
#define FLASH_MODE_CMD_DDR	(1<<17)

#define FLASH_MODE_CMD_STD        0x01
#define FLASH_MODE_CMD_MASK       (0|FLASH_MODE_CMD_DDR|FLASH_MODE_DDR|(0x3<<8))
#define FLASH_MODE_CMD_NULL   0
#define FLASH_MODE_CMD_OCT_DDR16  (FLASH_WRITE_GLOBAL_MODE_OCTAL|FLASH_MODE_CMD_DDR|FLASH_MODE_DDR|(0x3<<8))
#define FLASH_MODE_CMD_OCT_SDR16  (FLASH_WRITE_GLOBAL_MODE_OCTAL|FLASH_MODE_DDR|(0x3<<8))
#define FLASH_MODE_CMD_OCT_SDR8   (FLASH_WRITE_GLOBAL_MODE_OCTAL|(0x2<<8))
#define FLASH_MODE_CMD_QUAD_DDR8  (FLASH_WRITE_GLOBAL_MODE_QUAD|FLASH_MODE_CMD_DDR|FLASH_MODE_DDR|(0x2<<8))
#define FLASH_MODE_CMD_QUAD_SDR8  (FLASH_WRITE_GLOBAL_MODE_QUAD|(0x2<<8))
#define FLASH_MODE_CMD_SINGLE     0x00000200

#define FLASH_MODE_ADDR_MASK      (0|FLASH_MODE_DDR|(0xf<<2))
#define FLASH_MODE_ADDR_NULL      0x00000000
#define FLASH_MODE_ADDR_OCT_DDR   (FLASH_WRITE_GLOBAL_MODE_OCTAL|FLASH_MODE_DDR)
#define FLASH_MODE_ADDR_OCT_SDR   (FLASH_WRITE_GLOBAL_MODE_OCTAL)
#define FLASH_MODE_ADDR_QUAD_DDR  (FLASH_WRITE_GLOBAL_MODE_QUAD|FLASH_MODE_DDR)
#define FLASH_MODE_ADDR_QUAD_SDR  (FLASH_WRITE_GLOBAL_MODE_QUAD)

#define FLASH_MODE_DATA_MASK      (0|FLASH_MODE_DDR)
#define FLASH_MODE_DATA_NULL      0x00000000
#define FLASH_MODE_DATA_OCT_DDR   (FLASH_WRITE_GLOBAL_MODE_OCTAL|FLASH_MODE_DDR)
#define FLASH_MODE_DATA_OCT_SDR   FLASH_WRITE_GLOBAL_MODE_OCTAL
#define FLASH_MODE_DATA_QUAD_DDR  (FLASH_WRITE_GLOBAL_MODE_QUAD|FLASH_MODE_DDR)
#define FLASH_MODE_DATA_QUAD_SDR  FLASH_WRITE_GLOBAL_MODE_QUAD


#define FLASH_OSPI_CS_ENABLE() do { REG32(U_GPIOX__GPIO_OUT_CLEAR_1) = 0x00000001; } while (0)
#define FLASH_OSPI_CS_DISABLE() do { REG32(U_GPIOX__GPIO_OUT_SET_1)  = 0x00000001; } while (0)

/* values for GPIO registers
 * Alternate A
 * GPIO 32    CS
 * GPIO 33    CLK
 * GPIO 34    DQS
 * GPIO 35    DQ0
 * GPIO 36    DQ1
 * GPIO 37    DQ2
 * GPIO 38    DQ3
 * GPIO 39    DQ4
 * GPIO 40    DQ5
 * GPIO 41    DQ6
 * GPIO 42    DQ7
 */
#define FLASH_OCT_GPIO_PORT_MODE_MASK  0x003fffff
#define FLASH_OCT_GPIO_PORT_MODE       0x00155555
#define FLASH_OCT_GPIO_PORT_MODE_SWCS  0x00155554
#define FLASH_OCT_GPIO_IOCTRL_MASK     0x000007ff
#define FLASH_OCT_GPIO_IOCTRL          0x000007fb
#define FLASH_OCT_GPIO_MUX63_32_MASK   0x000007ff
#define FLASH_OCT_GPIO_MUX63_32        0x000007ff
#define FLASH_OCT_GPIO_PULL47_32_MASK  0x003fffff
#define FLASH_OCT_GPIO_PULL47_32       0x00155575 // DQS on Pulldown, Rest on Pullup

#define FLASH_QUAD_GPIO_PORT_MODE_MASK 0x00003fff
#define FLASH_QUAD_GPIO_PORT_MODE      0x00001555
#define FLASH_QUAD_GPIO_PORT_MODE_SWCS 0x00001554
#define FLASH_QUAD_GPIO_IOCTRL_MASK    0x0000007f
#define FLASH_QUAD_GPIO_IOCTRL         0x0000007b
#define FLASH_QUAD_GPIO_MUX63_32_MASK  0x0000007f
#define FLASH_QUAD_GPIO_MUX63_32       0x0000007f
#define FLASH_QUAD_GPIO_PULL47_32_MASK 0x00003fff
#define FLASH_QUAD_GPIO_PULL47_32      0x00001575 // DQS on Pulldown, Rest on Pullup

#define FLASH_DUAL_GPIO_PORT_MODE_MASK 0x000003ff
#define FLASH_DUAL_GPIO_PORT_MODE      0x00000155
#define FLASH_DUAL_GPIO_PORT_MODE_SWCS 0x00000154
#define FLASH_DUAL_GPIO_IOCTRL_MASK    0x0000001f
#define FLASH_DUAL_GPIO_IOCTRL         0x0000001b
#define FLASH_DUAL_GPIO_MUX63_32_MASK  0x0000001f
#define FLASH_DUAL_GPIO_MUX63_32       0x0000001f
#define FLASH_DUAL_GPIO_PULL47_32_MASK 0x000003ff
#define FLASH_DUAL_GPIO_PULL47_32      0x00000175 // DQS on Pulldown, Rest on Pullup

#define FLASH_SINGLE_GPIO_PORT_MODE_MASK 0x000003ff
#define FLASH_SINGLE_GPIO_PORT_MODE      0x00000155
#define FLASH_SINGLE_GPIO_PORT_MODE_SWCS 0x00000154
#define FLASH_SINGLE_GPIO_IOCTRL_MASK    0x0000001f
#define FLASH_SINGLE_GPIO_IOCTRL         0x0000001b
#define FLASH_SINGLE_GPIO_IOCTRL_SWCS    0x0000001a
#define FLASH_SINGLE_GPIO_MUX63_32_MASK  0x0000001f
#define FLASH_SINGLE_GPIO_MUX63_32       0x0000001f
#define FLASH_SINGLE_GPIO_PULL47_32_MASK 0x000003ff
#define FLASH_SINGLE_GPIO_PULL47_32      0x00000175 // DQS on Pulldown, Rest on Pullup

/** do a dummy read on a register, implemented in a way that even buggy
 *  compilers that do not treat volatile correctly will not optimize it away
 *
 * @param addr	address to read from
 */
#define FLASH_DUMMY_READ_REG32(addr) do { \
	flash_dummy32 = REG32((addr)); \
} while (0)


extern PNIO_VOID flash_ospi_reset_1(const struct flash_ospi_info * info);
extern PNIO_UINT32 flash_ospi_spi_write_register(PNIO_UINT8 cmd, PNIO_UINT32 addr,
		PNIO_UINT8 addrlen, PNIO_UINT32 data32, PNIO_UINT8 datalen);
extern PNIO_VOID flash_ospi_setup_register(const struct flash_ospi_info * info, PNIO_UINT32 cmd_index);
extern PNIO_UINT32 flash_ospi_startup_quick(struct flash_ospi_info * info, const flash_ospi_register_struct_s * regs);

extern PNIO_VOID flash_ospi_set_read_mode(const struct flash_ospi_info * info);

extern PNIO_UINT32 flash_ospi_cmd_read_statusreg(const struct flash_ospi_info * info);
extern PNIO_VOID flash_ospi_cmd_write_enable(const struct flash_ospi_info * info);
extern PNIO_VOID flash_ospi_cmd_erase_sector(const struct flash_ospi_info * info, PNIO_UINT32 addr);
extern PNIO_VOID flash_ospi_cmd_erase_block(const struct flash_ospi_info * info, PNIO_UINT32 addr);
extern PNIO_UINT32 flash_ospi_cmd_get_erase_status(const struct flash_ospi_info * info);
extern PNIO_UINT32 flash_ospi_cmd_program(const struct flash_ospi_info * info, PNIO_UINT8 * source,
		PNIO_UINT32 anz, PNIO_UINT32 oct_addr);
extern PNIO_VOID flash_ospi_cmd_clear_status(const struct flash_ospi_info * info);

extern PNIO_VOID flash_ospi_direct_switch_gpio_on(const struct flash_ospi_info * info);
extern PNIO_VOID flash_ospi_direct_switch_gpio_off(const struct flash_ospi_info * info);
extern PNIO_VOID flash_ospi_direct_write_cmd(PNIO_UINT32 cmd, PNIO_UINT32 cmd_mode);
extern PNIO_VOID flash_ospi_direct_write_addr(PNIO_UINT32 addr, PNIO_UINT32 mode, PNIO_UINT32 fill);
extern PNIO_VOID flash_ospi_direct_write_data(PNIO_UINT8 * data, PNIO_UINT32 data_len, PNIO_UINT32 data_mode,
		PNIO_UINT32 fill);

extern PNIO_VOID flash_ospi_init(flash_ospi_info_t * info);
extern PNIO_UINT32 flash_ospi_open(flash_ospi_info_t * info, const struct flash_ospi_open_param * params,
		PNIO_UINT32 reset);
extern PNIO_UINT32 flash_ospi_close(flash_ospi_info_t * info);
extern PNIO_UINT32 flash_ospi_read_idreg(PNIO_VOID);
extern PNIO_UINT32 flash_ospi_setup(flash_ospi_info_t * info, PNIO_UINT32 flash_param_id);
extern PNIO_UINT32 flash_ospi_get_flash_param_id(flash_ospi_info_t * info);
extern PNIO_UINT32 flash_ospi_erase_block(flash_ospi_info_t * info, PNIO_UINT32 block_addr);
extern PNIO_UINT32 flash_ospi_erase_sector(flash_ospi_info_t * info, PNIO_UINT32 sector_addr);
extern PNIO_UINT32 flash_ospi_get_erase_status(flash_ospi_info_t * info);
extern PNIO_UINT32 flash_ospi_read(const flash_ospi_info_t * info, PNIO_UINT32 mem_addr, PNIO_VOID * dest, size_t len);
extern PNIO_UINT32 flash_ospi_read_rsc(flash_ospi_info_t * info, PNIO_UINT32 mem_addr, PNIO_VOID * dest,
		PNIO_UINT32 rsc_page_size, size_t npages);
extern PNIO_UINT32 flash_ospi_read_rsc_dma(flash_ospi_info_t * info, PNIO_UINT32 mem_addr, PNIO_UINT32 dest_addr,
		PNIO_UINT32 rsc_page_size, size_t npages);
extern PNIO_UINT32 flash_ospi_write(const flash_ospi_info_t * info, PNIO_UINT32 mem_addr, PNIO_VOID * src, size_t len);
extern PNIO_UINT32 flash_ospi_write_rsc(const flash_ospi_info_t * info, PNIO_UINT32 mem_addr, PNIO_VOID * src,
		PNIO_UINT32 rsc_page_size, size_t npages);
extern PNIO_UINT32 flash_ospi_abort(flash_ospi_info_t * info);
#endif // BOARD_TYPE_STEP_3

#endif // FLASH_OSPI_H

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/