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
/*  F i l e               &F: iom_cfg.h                                 :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  This file contains the configuration of the IO Manager                   */
/*                                                                           */
/*****************************************************************************/


#ifndef IOM_CFG_H_
#define IOM_CFG_H_

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
    #define IOM_CFG_PNIP  /* Ertec200p */
    #if (IOD_INCLUDE_S2_REDUNDANCY == 1)
        #define IOM_SYSTEM_REDUNDANCY_SUPPORT
    #endif
#else
    #error "invalid platform"
#endif

/*****************************************************************
 * access macros
 *****************************************************************/
extern EDDP_GSHAREDMEM_TYPE psi_eddp_srd_shmem[PSI_CFG_MAX_IF_CNT];

#define IOM_EDDP_PROVIDER_REMOVE(_INSTANCE_HANDLE, _APPL_PROVIDER_ID)  eddp_SysRed_Provider_Remove(_INSTANCE_HANDLE, _APPL_PROVIDER_ID)

#define IOM_EDDP_SYSRED_INIT()            eddp_SysRed_Init()
#define IOM_EDDP_SYSRED_GET_SHMEM(HD_NR)  &(psi_eddp_srd_shmem[HD_NR - 1])
#define IOM_EDDP_SYS_TYPE                 PSI_EDD_SYS_TYPE
#define IOM_DEBUG                         PSI_DEBUG

/*****************************************************************
 * general defines and declarations
 *****************************************************************/

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
    /* locking to enable multiple tasks to access the module (PNIP) */
    #define IOM_ENTER()
    #define IOM_EXIT()
    /* special enter/exit macros for lock() and unlock() functions - can be left empty
     * if buffer locking is always done in interrupt context */
    #define IOM_ENTER_IO()  IOM_ENTER()
    #define IOM_EXIT_IO()   IOM_EXIT()
#endif


#define IOM_MEMSET(_DST,_VAL,_LEN)  OsMemSet(_DST,_VAL,_LEN)

#define IOM_COPY_BYTE(_DEST_PTR, _SRC_PTR, _LEN)                              \
{                                                                             \
    OsMemCpy((PNIO_VOID*)(_DEST_PTR), (PNIO_VOID*)(_SRC_PTR), _LEN);          \
}

    /* if defined : use the DTCM to store meta data */
#define IOM_CFG_LOCATE_DATA_TO_TCM
#ifdef IOM_CFG_LOCATE_DATA_TO_TCM
        /* specifies the name of the DTCM section. for example use ".data_d_tcm" */
    #define IOM_TCM_SECTION_NAME            OS_DATA_DTCM_SECTION_NAME
    #define IOM_DEFAULT_BSS_SECTION_NAME    ".bss_sys_iom"
#endif

    /* if defined : use the ITCM for time-critical instruction code (ISR) */
#ifdef IOM_CFG_PNIP
    #define IOM_CFG_LOCATE_INSTRUCTIONS_TO_TCM
#endif
#ifdef IOM_CFG_LOCATE_INSTRUCTIONS_TO_TCM
    #define IOM_ITCM_SECTION_NAME           OS_ITCM_SECTION_NAME
    #define IOM_DEFAULT_TEXT_SECTION_NAME   ".text_sys_iom"
#endif

/* ---------------------------------------------------------------------------- */
/* error handling. if IOM_CFG_FATAL_ERROR_ON_FAILURE is defined, all asserts    */
/* will lead to fatal error. Otherwise you'll get a corresponding return value  */
/* ---------------------------------------------------------------------------- */
#define IOM_CFG_FATAL_ERROR_ON_FAILURE
    /* error handling */
#define IOM_LSA_COMPONENT_ID                 PNIO_PACKID_IOM

extern void PNIO_FatalError(LSA_FATAL_ERROR_TYPE* pLsaErr);
#define IOM_FATAL_ERROR(_ERROR_DETAIL_PTR)   PNIO_FatalError(_ERROR_DETAIL_PTR)

#define IOM_MODULE_PERIF                     0x01       /* firmware module id inside the IOM (iom_perif.c), used in fatal error routine */

    /* function that is called after the completion of deallocation of PERIF memory */
#define IOM_IOCR_MEMORY_FREED(cur_ar)        /* nothing to do */

    /* allow unbuffered locking / unlocking for RTC3 connections. Buffer switching becomes unnecessary
     * because of the exact event scheduling in RTC3. The herewith activated ...lock_unbuffered*/
#define IOM_CFG_RTC3_UNBUFFERED

/********************************************
 * defines that are specific to PERIF
 ********************************************/
#if defined IOM_CFG_PNIP
    /*
     * PERIF defines
     */

        /* if IOM_CFG_USE_GDMA is defined, the "GDMA reuseable module" can be accessed by
         * the Perif-module in order to install user data DMA jobs
         */
    #define IOM_CFG_USE_GDMA

        /* number of supported ARs */
    #define IOM_CFG_PERIF_NUM_AR 8
        /* number of supported CRs by PERIF-hardware */
    #define IOM_CFG_PERIF_NUM_CR 15

        /* interrupt configuration */
    #define IOM_SET_INT_HANDLER(callback)     Bsp_EVMA_register_ISR(SY_INT_PER_IF_ARM_IRQ, callback) /* choose between IRQ / FIQ handler function */
#else
    #error "config error: define IOM_CFG_PNIP"
#endif

#endif /* IOM_CFG_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
