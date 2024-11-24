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
/*  F i l e               &F: gdma_cfg.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  configuration header containing for the                                  */
/*  software module handling the ERTEC200+ / Triton2 GDMA module             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                general defines and declarations                           */
/*****************************************************************************/
/* copy data */

#define GDMA_COPY_BYTE(_DEST_PTR, _SRC_PTR, _LEN)                             \
{                                                                             \
    OsMemCpy((PNIO_VOID*)(_DEST_PTR), (PNIO_VOID*)(_SRC_PTR), _LEN);          \
}

#define GDMA_MEMSET(_DST_PTR, _VAL, _LEN)                                     \
{                                                                             \
    OsMemSet((PNIO_VOID*)(_DST_PTR), _VAL, _LEN);                             \
}

/* ----------------------------------- */
/* choose the underlying hardware base */
/* ----------------------------------- */
#define GDMA_CFG_HW_BASE_ERTEC200P
// #define GDMA_CFG_HW_BASE_TRITON2


    /* define the base address of GDMA IP */
#ifdef GDMA_CFG_HW_BASE_ERTEC200P
        /* base address of the IP */
    #define GDMA_CFG_BASE_ADDRESS   U_GDMA__GDMA_REG_ADDR
#endif
#ifdef GDMA_CFG_HW_BASE_TRITON2
    #define GDMA_CFG_BASE_ADDRESS   0x20300000
#endif

/* ------------------------------------------------------------------------------------ */
/* lock/unlock macros (optional interrupt blockade) to guarantee the non-reentrant      */
/* behaviour of functions. This is necessary if multiple tasks access the GDMA module   */
/* simultaneously                                                                       */
/* ------------------------------------------------------------------------------------ */
#define GDMA_CFG_ENTER()
#define GDMA_CFG_EXIT()

/* ------------------------------ */
/* interrupt registering function */
/* ------------------------------ */
#define GDMA_SET_INT_HANDLER(callback)      Bsp_EVMA_register_ISR(SY_INT_GDMA_IRQ, callback)   /* choose between IRQ / FIQ handler function */
#define GDMA_RESET_INT_HANDLER()            Bsp_EVMA_unregister_ISR(SY_INT_GDMA_IRQ)

/* -------------------------------------------------------------------------------------------------*/
/* this define reserves specific DMA job numbers for tasks, that need hardware flow control. If not */
/* needed -> comment the define                                                                     */
/* -------------------------------------------------------------------------------------------------*/
// #define GDMA_KEEP_JOBS_AVAILABLE_FOR_HW_FLOW {GDMA_SPI1_SSPRXDMA}

/* -------------------------------------------------------------------------------------------------------------------- */
/* number of separated GDMA transfer lists. If there is more than 1 transfer list configured, one                       */
/* can switch between them during operation. The membership of a specific job is parameterized via the job_add-function */
/* -------------------------------------------------------------------------------------------------------------------- */
#define GDMA_CFG_NUM_JOB_LISTS 2

/* ---------------------------------------------------------------------------- */
/* error handling. if GDMA_CFG_FATAL_ERROR_ON_FAILURE is defined, all asserts   */
/* will lead to fatal error. Otherwise you'll get a corresponding return value  */
/* ---------------------------------------------------------------------------- */
#define GDMA_CFG_FATAL_ERROR_ON_FAILURE

/* error handling */
extern void PNIO_FatalError(LSA_FATAL_ERROR_TYPE* pLsaErr);

#define GDMA_LSA_COMPONENT_ID                 PNIO_PACKID_GDMA
#define GDMA_FATAL_ERROR(_ERROR_DETAIL_PTR)   PNIO_FatalError(_ERROR_DETAIL_PTR)
#define GDMA_MODULE_GDMA                      0x01       /* firmware module id inside the GDMA (gdma.c), used in fatal error routine */

/* --------- */
/* TCM usage */
/* --------- */

    /* if defined : use the ITCM for time-critical instruction code (ISR) */
#define GDMA_CFG_LOCATE_INSTRUCTIONS_TO_TCM
#ifdef GDMA_CFG_LOCATE_INSTRUCTIONS_TO_TCM
    #define GDMA_ITCM_SECTION_NAME 			OS_ITCM_SECTION_NAME
    #define GDMA_DEFAULT_TEXT_SECTION_NAME  ".text_sys_gdma"
#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
