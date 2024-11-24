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
/*  F i l e               &F: flash_vectors.h                           :F&  */
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

#define FLASH_OSPI_REGISTER_MAX_CMD 8

#define FLASH_OSPI_STROBE_REG(mode, sck_delay, rxds_setup_cycles) (((mode) ? (PNIO_UINT32) 1 << 31 : 0) | ((PNIO_UINT32) (sck_delay) << 26) | ((PNIO_UINT32) (rxds_setup_cycles) << 16))

#define PN_SUB_GPIOX__GPIO_PORT_MODE_0_H_RESET__VALUE 0x0
#define PN_SUB_GPIOX__GPIO_IOCTRL_1                                                                             (0x40026020 + ERTEC200P_TOP_BASE)
#define PN_SUB_GPIOX__GPIO_IOCTRL_1__GPIO_IOCTRL_1     

#define PN_SUB_GPIOX__GPIO_IN_1_RESET__VALUE 0x0
#define PN_SUB_GPIOX__GPIO_PORT_MODE_1_L                                                                        (0x40026038 + ERTEC200P_TOP_BASE)
#define PN_SUB_GPIOX__GPIO_PORT_MODE_1_L__GPIO_32_MODE_1_L     

#define PN_SUB_PN_SCRB__PULL47_32GPIO                                                                           (0x4000F09C + ERTEC200P_TOP_BASE)
#define PN_SUB_PN_SCRB__PULL47_32GPIO__PR_GPIO32   

#define PN_SUB_PN_SCRB2__MUX63_32GPIO                                                                           (0x4000F120 + ERTEC200P_TOP_BASE)

#define WRITE_ENABLE 0x06
#define WRITE_VOLATILE_CONFIG_REG 0x81
#define WRITE_ENHANCED_VOLATILE_CONFIG_REG 0x61

/* shift values for OCTALSPI_STROBE_REG according to STA */
#define SHIFT_DQS_INT_3V3 0
#define SHIFT_DQS_INT_1V8 1

struct flash_ospi_info;

#define FLASH_DUMMY_READ_REG32(addr) do { \
	flash_dummy32_vectors = REG32((addr)); \
} while (0)


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

	int erase_in_progress;
} flash_ospi_info_t;

/* values for flash_ospi_register_struct_t.global_mode */
#define FLASH_OSPI_MODE_SINGLE   0x00
#define FLASH_OSPI_MODE_DUAL     0x01
#define FLASH_OSPI_MODE_QUAD     0x02
#define FLASH_OSPI_MODE_OCTAL    0x03

#define FLASH_OSPI_READ_CMD          0
#define FLASH_OSPI_PROGRAM_CMD       1
#define FLASH_OSPI_WRITE_ENABLE_CMD  2
#define FLASH_OSPI_READ_STATUS_CMD   3
#define FLASH_OSPI_ERASE_SECTOR_CMD  4
#define FLASH_OSPI_ERASE_BLOCK_CMD   5
#define FLASH_OSPI_USER_CMD_0        6
#define FLASH_OSPI_USER_CMD_1        7

#define PNIO_FLASH_PARAM_ERR 2

enum flash_ospi_flash_param_id_vectors {
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


#define SYS_OSPI_ID_MASK                0xffffff

#define FLASH_OSPI_SERVICE_REG (U_SCRB2__SERVICE_1_REG)

#define FLASH_OCT_GPIO_PORT_MODE_MASK  0x003fffff
#define FLASH_OCT_GPIO_PORT_MODE       0x00155555
#define FLASH_OCT_GPIO_PORT_MODE_SWCS  0x00155554
#define FLASH_OCT_GPIO_IOCTRL_MASK     0x000007ff
#define FLASH_OCT_GPIO_IOCTRL          0x000007fb
#define FLASH_OCT_GPIO_MUX63_32_MASK   0x000007ff
#define FLASH_OCT_GPIO_MUX63_32        0x000007ff
#define FLASH_OCT_GPIO_PULL47_32_MASK  0x003fffff
#define FLASH_OCT_GPIO_PULL47_32       0x00155575 // DQS on pulldown, rest on pullup

#define FLASH_QUAD_GPIO_PORT_MODE_MASK 0x00003fff
#define FLASH_QUAD_GPIO_PORT_MODE      0x00001555
#define FLASH_QUAD_GPIO_PORT_MODE_SWCS 0x00001554
#define FLASH_QUAD_GPIO_IOCTRL_MASK    0x0000007f
#define FLASH_QUAD_GPIO_IOCTRL         0x0000007b
#define FLASH_QUAD_GPIO_MUX63_32_MASK  0x0000007f
#define FLASH_QUAD_GPIO_MUX63_32       0x0000007f
#define FLASH_QUAD_GPIO_PULL47_32_MASK 0x00003fff
#define FLASH_QUAD_GPIO_PULL47_32      0x00001575 // DQS on pulldown, rest on pullup

#define FLASH_DUAL_GPIO_PORT_MODE_MASK 0x000003ff
#define FLASH_DUAL_GPIO_PORT_MODE      0x00000155
#define FLASH_DUAL_GPIO_PORT_MODE_SWCS 0x00000154
#define FLASH_DUAL_GPIO_IOCTRL_MASK    0x0000001f
#define FLASH_DUAL_GPIO_IOCTRL         0x0000001b
#define FLASH_DUAL_GPIO_MUX63_32_MASK  0x0000001f
#define FLASH_DUAL_GPIO_MUX63_32       0x0000001f
#define FLASH_DUAL_GPIO_PULL47_32_MASK 0x000003ff
#define FLASH_DUAL_GPIO_PULL47_32      0x00000175 // DQS on pulldown, rest on pullup

#define FLASH_SINGLE_GPIO_PORT_MODE_MASK 0x000003ff
#define FLASH_SINGLE_GPIO_PORT_MODE      0x00000155
#define FLASH_SINGLE_GPIO_PORT_MODE_SWCS 0x00000154
#define FLASH_SINGLE_GPIO_IOCTRL_MASK    0x0000001f
#define FLASH_SINGLE_GPIO_IOCTRL         0x0000001b
#define FLASH_SINGLE_GPIO_IOCTRL_SWCS    0x0000001a
#define FLASH_SINGLE_GPIO_MUX63_32_MASK  0x0000001f
#define FLASH_SINGLE_GPIO_MUX63_32       0x0000001f
#define FLASH_SINGLE_GPIO_PULL47_32_MASK 0x000003ff
#define FLASH_SINGLE_GPIO_PULL47_32      0x00000175 // DQS on pulldown, rest on pullup

#define FLASH_OSPI_CS_ENABLE() do { REG32(U_GPIOX__GPIO_OUT_CLEAR_1) = 0x00000001; } while (0)
#define FLASH_OSPI_CS_DISABLE() do { REG32(U_GPIOX__GPIO_OUT_SET_1)  = 0x00000001; } while (0)

extern volatile PNIO_UINT32 flash_dummy32_vectors;
#endif // BOARD_TYPE_STEP_3

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/