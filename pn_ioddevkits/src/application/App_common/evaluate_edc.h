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
/*  F i l e               &F: evaluate_edc.h                            :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  header file to evaluate_edc.c.                                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/
#ifndef _EVALUATE_EDC_H_
#define _EVALUATE_EDC_H_

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif
#ifdef BOARD_TYPE_STEP_3

#define PN_SUB_PN_SCRB__EDC_EVENT                                    (0x4000F054)
#define PN_SUB_PN_SCRB__EDC_EVENT__I_TCM926_1B                       (0x00000001)
#define PN_SUB_PN_SCRB__EDC_EVENT__D_TCM926_1B                       (0x00000004)
#define PN_SUB_PN_SCRB__EDC_EVENT__GDMA_1B                           (0x00000010)
#define PN_SUB_PN_SCRB__EDC_EVENT__PN_1B                             (0x00000040)
#define PN_SUB_PN_SCRB__EDC_EVENT__PERIF_1B                          (0x00000100)
#define PN_SUB_PN_SCRB__EDC_EVENT__I_TCM966_1B                       (0x00004000)
#define PN_SUB_PN_SCRB__EDC_EVENT__D_TCM966_1B                       (0x00010000)
#define PN_SUB_PN_SCRB__EDC_EVENT__I_TCM926_2B                       (0x00000002)
#define PN_SUB_PN_SCRB__EDC_EVENT__D_TCM926_2B                       (0x00000008)
#define PN_SUB_PN_SCRB__EDC_EVENT__GDMA_2B                           (0x00000020)
#define PN_SUB_PN_SCRB__EDC_EVENT__PN_2B                             (0x00000080)
#define PN_SUB_PN_SCRB__EDC_EVENT__PERIF_2B                          (0x00000200)
#define PN_SUB_PN_SCRB__EDC_EVENT__D_CACHE_PAR                       (0x00000800)
#define PN_SUB_PN_SCRB__EDC_EVENT__D_TAG_PAR                         (0x00002000)
#define PN_SUB_PN_SCRB__EDC_EVENT__I_CACHE_PAR                       (0x00000400)
#define PN_SUB_PN_SCRB__EDC_EVENT__I_TAG_PAR                         (0x00001000)
#define PN_SUB_PN_SCRB__EDC_EVENT__D_TCM966_2B                       (0x00020000)
#define PN_SUB_OSPI_SPFU_PREFETCH_RAM__start                         (0x4002C200)
#define PN_SUB_AHB_SRAM__start                                       (0x11300000)

#define PN_SUB_PN_SCRB__EDC_EVENT__I_TCM966_2B                       (0x00008000)
#define PN_SUB_PN_SCRB__EDC_EVENT__SPI_PFU_1B                        (0x00100000)
#define PN_SUB_PN_SCRB__EDC_EVENT__SPI_PFU_2B                        (0x00200000)
#define PN_SUB_PN_SCRB__EDC_EVENT__SRAM_1B                           (0x00400000)
#define PN_SUB_PN_SCRB__EDC_EVENT__SRAM_2B                           (0x00800000)
#define PN_SUB_PN_SCRB__EDC_EVENT__I_TCM926_2B_RFSH                  (0x01000000)
#define PN_SUB_PN_SCRB__EDC_EVENT__D_TCM926_2B_RFSH                  (0x02000000)
#define PN_SUB_PN_SCRB__EDC_EVENT__PN_2B_RFSH                        (0x04000000)
#define PN_SUB_PN_SCRB__EDC_EVENT__I_TCM966_2B_RFSH                  (0x08000000)
#define PN_SUB_PN_SCRB__EDC_EVENT__D_TCM966_2B_RFSH                  (0x10000000)
#define PN_SUB_PN_SCRB__EDC_EVENT__SRAM_2B_RFSH                      (0x20000000)

#define PN_SUB_PN_SCRB2__EDC_STAT_REG                                (0x4000F13C)
#define PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_I_TCM926             (0x00000001)
#define PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_D_TCM926             (0x00000002)
#define PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_I_TCM966             (0x00000004)
#define PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_D_TCM966             (0x00000008)
#define PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_PNIP                 (0x00000010)
#define PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_SRAM                 (0x00000020)

    PNIO_BOOL evaluate_edc_errors();
    PNIO_BOOL irreparable_ecc_problem_present(PNIO_UINT32*);

#endif
#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif /* APP_COMMON_EVALUATE_EDC_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/