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
/*  F i l e               &F: pnpb_lib_int.h                            :F&  */
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

#ifndef APP1_STANDART_PNPB_LIB_INT_H_
#define APP1_STANDART_PNPB_LIB_INT_H_


#define PNPB_GPMC_SETTINGS_REGS_OFFSET              0x50000000
#define PNPB_GPMC_SETTINGS_REGS_SIZE                0x200

#define BASE_PNPB_GPMC_REG_ADDRESS                  0x00
/* Addresses of registers of GPMC bank 0 */
#define PNPB_GPMC_CONFIG_BANK_0                     0
#define PNPB_GPMC_CONFIG_REG1_0                     (BASE_PNPB_GPMC_REG_ADDRESS + 0x60)
#define PNPB_GPMC_CONFIG_REG2_0                     (BASE_PNPB_GPMC_REG_ADDRESS + 0x64)
#define PNPB_GPMC_CONFIG_REG3_0                     (BASE_PNPB_GPMC_REG_ADDRESS + 0x68)
#define PNPB_GPMC_CONFIG_REG4_0                     (BASE_PNPB_GPMC_REG_ADDRESS + 0x6C)
#define PNPB_GPMC_CONFIG_REG5_0                     (BASE_PNPB_GPMC_REG_ADDRESS + 0x70)
#define PNPB_GPMC_CONFIG_REG6_0                     (BASE_PNPB_GPMC_REG_ADDRESS + 0x74)
#define PNPB_GPMC_CONFIG_REG7_0                     (BASE_PNPB_GPMC_REG_ADDRESS + 0x78)
/* Addresses of registers of GPMC bank 6 */
#define PNPB_GPMC_CONFIG_BANK_6                     6
#define PNPB_GPMC_CONFIG_REG1_6                     (BASE_PNPB_GPMC_REG_ADDRESS + 0x180)
#define PNPB_GPMC_CONFIG_REG2_6                     (BASE_PNPB_GPMC_REG_ADDRESS + 0x184)
#define PNPB_GPMC_CONFIG_REG3_6                     (BASE_PNPB_GPMC_REG_ADDRESS + 0x188)
#define PNPB_GPMC_CONFIG_REG4_6                     (BASE_PNPB_GPMC_REG_ADDRESS + 0x18C)
#define PNPB_GPMC_CONFIG_REG5_6                     (BASE_PNPB_GPMC_REG_ADDRESS + 0x190)
#define PNPB_GPMC_CONFIG_REG6_6                     (BASE_PNPB_GPMC_REG_ADDRESS + 0x194)
#define PNPB_GPMC_CONFIG_REG7_6                     (BASE_PNPB_GPMC_REG_ADDRESS + 0x198)
/* Addresses of general GPMC config register */
#define PNPB_GPMC_CONFIG_REG                        (BASE_PNPB_GPMC_REG_ADDRESS +  0x50)

/* Bank used for GPMC - XHIF */
#define PNPB_GPMC_CONFIG_BANK                       PNPB_GPMC_CONFIG_BANK_6
#if(PNPB_GPMC_CONFIG_BANK_6 == PNPB_GPMC_CONFIG_BANK)
#define PNPB_GPMC_CONFIG_REG1                       PNPB_GPMC_CONFIG_REG1_6
#define PNPB_GPMC_CONFIG_REG2                       PNPB_GPMC_CONFIG_REG2_6
#define PNPB_GPMC_CONFIG_REG3                       PNPB_GPMC_CONFIG_REG3_6
#define PNPB_GPMC_CONFIG_REG4                       PNPB_GPMC_CONFIG_REG4_6
#define PNPB_GPMC_CONFIG_REG5                       PNPB_GPMC_CONFIG_REG5_6
#define PNPB_GPMC_CONFIG_REG6                       PNPB_GPMC_CONFIG_REG6_6
#define PNPB_GPMC_CONFIG_REG7                       PNPB_GPMC_CONFIG_REG7_6
#else
#error If other bank is meant to be used, it have to be defined here!
#endif

/* Bank used for XHIF page switch */
#define PNPB_GPMC_PAGE_CONFIG_BANK                  PNPB_GPMC_CONFIG_BANK_0
#if(PNPB_GPMC_CONFIG_BANK_0 == PNPB_GPMC_PAGE_CONFIG_BANK)
#define PNPB_GPMC_PAGE_CONFIG_REG1                  PNPB_GPMC_CONFIG_REG1_0
#define PNPB_GPMC_PAGE_CONFIG_REG2                  PNPB_GPMC_CONFIG_REG2_0
#define PNPB_GPMC_PAGE_CONFIG_REG3                  PNPB_GPMC_CONFIG_REG3_0
#define PNPB_GPMC_PAGE_CONFIG_REG4                  PNPB_GPMC_CONFIG_REG4_0
#define PNPB_GPMC_PAGE_CONFIG_REG5                  PNPB_GPMC_CONFIG_REG5_0
#define PNPB_GPMC_PAGE_CONFIG_REG6                  PNPB_GPMC_CONFIG_REG6_0
#define PNPB_GPMC_PAGE_CONFIG_REG7                  PNPB_GPMC_CONFIG_REG7_0
#else
#error If other bank is meant to be used, it have to be defined here!
#endif

/* settings of gpmc used for GPMC - XHIF */
/* for details of each register consult official TI Sitara documentation */
/* REG general config */
#define PNPB_GPMC_REG_VAL_WAIT1PINPOLARITY          0x01    /* 0 -  1 */
#define PNPB_GPMC_REG_VAL_WAIT0PINPOLARITY          0x01    /* 0 -  1 */
#define PNPB_GPMC_REG_VAL_WRITEPROTECT              0x00    /* 0 -  1 */
#define PNPB_GPMC_REG_VAL_LIMITEDADDRESS            0x00    /* 0 -  1 */
#define PNPB_GPMC_REG_VAL_NANDFORCEPOSTEDWRITE      0x00    /* 0 -  1 */
/* REG CONFIG1 */
#define PNPB_GPMC_REG_VAL_WRAPBURST                 0x00	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_READMULTIPLE              0x00	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_READTYPE                  0x00	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_WRITEMULTIPLE             0x00	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_WRITETYPE                 0x00	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_CLKACTIVATIONTIME         0x00	/* 0 -  2 */
#define PNPB_GPMC_REG_VAL_ATTACHEDDEVICEPAGELENGTH  0x00	/* 0 -  2 */
#define PNPB_GPMC_REG_VAL_WAITREADMONITORING        0x01	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_WAITWRITEMONITORING       0x01	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_WAITONMONITORINGTIME      0x02	/* 0 -  2 */
#define PNPB_GPMC_REG_VAL_WAITPINSELECT             0x01	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_DEVICESIZE                0x01	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_DEVICETYPE                0x00	/* 0, 2   */
#define PNPB_GPMC_REG_VAL_MUXADDATA                 0x00	/* 0 -  2 */
#define PNPB_GPMC_REG_VAL_TIMEPARAGRANULARITY       0x00	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_GPMCFCLKDIVIDER           0x00	/* 0 -  3 */
/* REG CONFIG2 */
#define PNPB_GPMC_REG_VAL_CSWROFFTIME               0x0A 	/* 0 - 1F */
#define PNPB_GPMC_REG_VAL_CSRDOFFTIME               0x0A 	/* 0 - 1F */
#define PNPB_GPMC_REG_VAL_CSEXTRADELAY              0x00 	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_CSONTIME                  0x02 	/* 0 - 1F */
/* REG CONFIG3 */
#define PNPB_GPMC_REG_VAL_ADVAADMUXWROFFTIME        0x00 	/* 0 -  7 */
#define PNPB_GPMC_REG_VAL_ADVAADMUXRDOFFTIME        0x00 	/* 0 -  7 */
#define PNPB_GPMC_REG_VAL_ADVWROFFTIME              0x00 	/* 0 - 1F */
#define PNPB_GPMC_REG_VAL_ADVRDOFFTIME              0x00 	/* 0 - 1F */
#define PNPB_GPMC_REG_VAL_ADVEXTRADELAY             0x00 	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_ADVAADMUXONTIME           0x00 	/* 0 -  9 */
#define PNPB_GPMC_REG_VAL_ADVONTIME                 0x00 	/* 0 -  F */
/* REG 4 */
#define PNPB_GPMC_REG_VAL_WEOFFTIME                 0x09 	/* 0 - 1F */
#define PNPB_GPMC_REG_VAL_WEEXTRADELAY              0x00 	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_WEONTIME                  0x01 	/* 0 -  F */
#define PNPB_GPMC_REG_VAL_OEAADMUXOFFTIME           0x00 	/* 0 -  7 */
#define PNPB_GPMC_REG_VAL_OEOFFTIME                 0x09 	/* 0 - 1F */
#define PNPB_GPMC_REG_VAL_OEEXTRADELAY              0x00 	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_OEAADMUXONTIME            0x00 	/* 0 -  7 */
#define PNPB_GPMC_REG_VAL_OEONTIME                  0x01 	/* 0 -  F */
/* REG 5 */
#define PNPB_GPMC_REG_VAL_RDACCESSTIME              0x08 	/* 0 - 1F */
#define PNPB_GPMC_REG_VAL_WRCYCLETIME               0x0A 	/* 0 - 1F */
#define PNPB_GPMC_REG_VAL_RDCYCLETIME               0x0A 	/* 0 - 1F */
/* REG 6 */
#define PNPB_GPMC_REG_VAL_WRACCESSTIME              0x08 	/* 0 - 1F */
#define PNPB_GPMC_REG_VAL_WRDATAONADMUXBUS          0x01 	/* 0 -  F */
#define PNPB_GPMC_REG_VAL_CYCLE2CYCLEDELAY          0x00 	/* 0 -  F */
#define PNPB_GPMC_REG_VAL_CYCLETOCYCLESAMECSEN      0x00 	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_CYCLE2CYCLEDIFFCSEN       0x00 	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_BUSTURNAROUND             0x00	/* 0 -  F */
/* REG 7 for bank 0 */
#define PNPB_GPMC_REG_VAL_BANK0_MASKADDRESS         0x0F	/* 0 -  F */
#define PNPB_GPMC_REG_VAL_BANK0_CSVALID             0x01 	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_BANK0_BASEADDRESS         0x02 	/* 0 - 1F */
/* REG 7 for bank 6 */
#define PNPB_GPMC_REG_VAL_BANK6_MASKADDRESS         0x0F	/* 0 -  F */
#define PNPB_GPMC_REG_VAL_BANK6_CSVALID             0x01 	/* 0 -  1 */
#define PNPB_GPMC_REG_VAL_BANK6_BASEADDRESS         0x01 	/* 0 - 1F */

#define PNPB_BINARY_FOR_ERTEC                       ("ecos.bin")
#define PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE        917504

#define PNPB_ERTEC_APB_REGISTERS_OFFSET             0x40000000
#define PNPB_ERTEC_EMC_REGISTERS_OFFSET             0x10D00000
#define PNPB_ERTEC_ASYN_RES_CTRL_REG                0x0000F00C
#define PNPB_ERTEC_WD_CTRL_STATUS_REG               0x00013000

/* Informs primary bootloader of Ertec, that firmware was uploaded to Ertec DK*/
#define PNPB_ERTEC_FIRMWARE_LOADED_CODE             0x68799786

#define PNPB_RW_LOCK_RETRIES                        3
#define PNPB_RW_LOCK_WAIT_NS                        500 /* Real time is longer, minimal time is several us */

/* prototypes */
PNIO_VOID   pnpb_set_gpmc(PNIO_VOID* p_gpmc);
PNIO_UINT32 pnpb_set_page(PNIO_VOID* p_xhif_set_page, PNIO_UINT32 page, PNIO_UINT32 addr);
PNIO_UINT32 pnpb_set_multiple_pages(PNIO_VOID* p_xhif_set_page, PNIO_UINT32 page_strt, PNIO_UINT32 pages, PNIO_UINT32 strt_addr);
PNIO_VOID   pnpb_print_xhif_page_reg(PNIO_VOID* p_xhif_set_page);
PNIO_VOID   pnpb_set_ertec_registers(PNIO_VOID* p_xhif_set_page, PNIO_VOID* p_xhif_data);
PNIO_UINT32 pnpb_copy_firmware(PNIO_VOID* p_xhif_set_page, PNIO_VOID* p_xhif_data);
PNIO_UINT32 pnpb_start_firmware(PNIO_VOID* p_xhif_set_page, PNIO_VOID* p_xhif_data);
PNIO_UINT32 pnpb_wait_for_ertec(PNIO_VOID* p_xhif_set_page, PNIO_VOID* p_xhif_data);
PNIO_UINT32 pnpb_cleanup_ertec(PNIO_VOID* p_xhif_set_page, PNIO_VOID* p_xhif_data);
PNIO_UINT32 pnpb_obtain_binary_size(FILE* source, PNIO_UINT32* p_sdram_size, PNIO_UINT32* p_itcm_size);
PNIO_UINT32 pnpb_copy_one_page_of_firmware( PNIO_VOID* p_xhif_set_page,
                                            PNIO_VOID* p_xhif_data,
                                            FILE* source,
                                            PNIO_UINT32 page,
                                            PNIO_UINT32 page_size,
                                            PNIO_UINT32 page_start);
PNIO_UINT32 pnpb_copy_firmware_to_itcm( PNIO_VOID* p_xhif_set_page,
                                        PNIO_VOID* p_xhif_data,
                                        FILE* source,
                                        PNIO_UINT32 page_size);
PNIO_UINT32 pnpb_check_loaded_firmware( PNIO_VOID* p_xhif_set_page,
                                        PNIO_VOID* p_xhif_data,
                                        FILE* source,
                                        PNIO_UINT32 sum_pages,
                                        PNIO_UINT32 itcm_size,
                                        PNIO_UINT32 binary_size);
PNIO_UINT32 pnpb_check_firmware_in_itcm(PNIO_VOID* p_xhif_set_page,
                                        PNIO_VOID* p_xhif_data,
                                        FILE* source,
                                        PNIO_UINT32 page_size);
PNIO_UINT32 pnpb_check_one_page_of_firmware(PNIO_VOID* p_xhif_set_page,
                                            PNIO_VOID* p_xhif_data,
                                            FILE* source,
                                            PNIO_UINT32 page,
                                            PNIO_UINT32 page_size,
                                            PNIO_UINT32 page_start);
PNIO_VOID * pnpb_acyclical_capture_thread(PNIO_VOID * i);
PNIO_VOID * pnpb_cyclical_execute_thread(PNIO_VOID * i);
PNIO_VOID pnpb_test_set(PNIO_UINT32 gpio, PNIO_UINT32 val);
PNIO_VOID pnpb_write_ertec_register(PNIO_VOID* p_xhif_data, PNIO_UINT32 ertec_addr, PNIO_UINT32 value);
PNIO_VOID pnpb_read_ertec_register(PNIO_VOID* p_xhif_data, PNIO_UINT32 ertec_addr, PNIO_UINT32 *p_value);
PNIO_VOID pnpb_write_ertec_register_masked(PNIO_VOID* p_xhif_data, PNIO_UINT32 ertec_addr, PNIO_UINT32 value, PNIO_UINT32 mask);
PNIO_VOID pnpb_reset_Ertec();

#endif /* APP1_STANDART_PNPB_LIB_INT_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
