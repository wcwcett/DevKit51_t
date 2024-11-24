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
/*  F i l e               &F: ecos_ertec_bsp_ospi_flash.h               :F&  */
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

#ifndef SYSADAPT1_CFG_ECOS_ERTEC_BSP_OSPI_FLASH_H_
#define SYSADAPT1_CFG_ECOS_ERTEC_BSP_OSPI_FLASH_H_

#ifdef BOARD_TYPE_STEP_3
// TODO this part should be added to eb200p registers. also check missing
#define PN_SUB_GPIOX__GPIO_PORT_MODE_0_H_RESET__VALUE 0x0
#define PN_SUB_GPIOX__GPIO_IOCTRL_1                                                                             (0x40026020 + ERTEC200P_TOP_BASE)
#define PN_SUB_GPIOX__GPIO_IOCTRL_1__GPIO_IOCTRL_1     

#define PN_SUB_GPIOX__GPIO_IN_1_RESET__VALUE 0x0
#define PN_SUB_GPIOX__GPIO_PORT_MODE_1_L                                                                        (0x40026038 + ERTEC200P_TOP_BASE)
#define PN_SUB_GPIOX__GPIO_PORT_MODE_1_L__GPIO_32_MODE_1_L     

#define PN_SUB_PN_SCRB__PULL47_32GPIO                                                                           (0x4000F09C + ERTEC200P_TOP_BASE)
#define PN_SUB_PN_SCRB__PULL47_32GPIO__PR_GPIO32   

#define PN_SUB_PN_SCRB2__MUX63_32GPIO                                                                            (0x4000F120 + ERTEC200P_TOP_BASE)




#define SYS_OSPI_ID_MASK                0xffffff
#define SYS_OSPI_GPIO_CONFIG_DATA       0xa9a8a7a6

#define SYS_OSPI_RESET_REQUIRED         1
#define SYS_OSPI_NO_RESET_REQUIRED      0

#define SYS_OSPI_RSC_ENCODED_SIZE       236 // for ospi rsc encoded size 
#define SYS_OSPI_RSC_SIZE               16  // for ospi rsc key size

#define SYS_OSPI_START_ADDR             0
#define SYS_OSPI_SECTOR_SIZE            0xffff //64 kb

typedef struct {
    PNIO_UINT32 jedec_id;              // read from device by the command 0x9F
    PNIO_UINT32 flash_param_id;        // input to HWAL configuration
    const PNIO_CHAR *vendor;             // name of vendor (info)
    const PNIO_CHAR *chiptype;           // name of part type (info)
} JedecMapping_t;


PNIO_UINT32 init_ospi();
PNIO_UINT32 ospi_erase_sector(PNIO_UINT32 sector_addr);
PNIO_UINT32 ospi_erase_block(PNIO_UINT32 block_addr);
PNIO_UINT32 ospi_erase(PNIO_UINT32 start_addr, PNIO_UINT32 length);
PNIO_UINT32 ospi_erase_verify(PNIO_UINT32 mem_addr, PNIO_UINT32 length);
PNIO_UINT32 ospi_write_data(PNIO_UINT32 mem_addr, PNIO_VOID_PTR_TYPE src, PNIO_UINT32 length);
PNIO_UINT32 ospi_write_data_rsc(PNIO_UINT32 mem_addr, PNIO_VOID_PTR_TYPE src, PNIO_UINT32 length);
PNIO_UINT32 ospi_verify_write_data(PNIO_UINT32 mem_addr, PNIO_VOID_PTR_TYPE src, PNIO_UINT32 length);
PNIO_UINT32 ospi_read_data(PNIO_UINT32 mem_addr, PNIO_VOID_PTR_TYPE dest, PNIO_UINT32 length);
PNIO_UINT32 ospi_read_data_rsc(PNIO_UINT32 mem_addr, PNIO_VOID_PTR_TYPE dest, PNIO_UINT32 length);

#endif //BOARD_TYPE_STEP_3
#endif /* SYSADAPT1_CFG_ECOS_ERTEC_BSP_OSPI_FLASH_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
