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
/*  F i l e               &F: evma_cfg.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  This file contains the configuration of the EVMA module                  */
/*                                                                           */
/*****************************************************************************/

#ifndef EVMA_CFG_H_
#define EVMA_CFG_H_

/**                            common
 * choose the Hardware by defining EVMA_USE_PNIP or not.
 *
 * By defining EVMA_USE_PNIP, an Ertec200p device is expected.
 */
#define IM_HW_ERTEC200P
#ifdef IM_HW_ERTEC200P
    #define EVMA_USE_PNIP
#endif

/**                            common
 * The interface functions of the EVMA module are not reentrant -> if several
 * tasks / interruptible software layers might access EVMA's functions
 * simultaneously, this has to be prevented by an interrupt lock or semaphore.
 */
#define EVMA_ENTER()
#define EVMA_EXIT()

/*
 * TCM usage macros
 */

    /**
     * use the EVMA_USE_TCM define to locate ISR code and data that is used inside
     * the ISR into DTCM + ITCM. Give the respective memory sections of the DTCM
     * and ITCM as well as the default memory section (SDRAM) by setting the following defines ...
     */
#define EVMA_USE_TCM

#ifdef EVMA_USE_TCM
    #define EVMA_DTCM_SECTION_NAME OS_DATA_DTCM_SECTION_NAME    /**< section name of the DTCM */
    #define EVMA_ITCM_SECTION_NAME OS_ITCM_SECTION_NAME         /**< section name of the ITCM */

    #define EVMA_DEFAULT_BSS_SECTION_NAME  ".bss_sys_evma"      /**< section name of the default data section for EVMA */
    #define EVMA_DEFAULT_TEXT_SECTION_NAME ".text_sys_evma"     /**< section name of the default code section for EVMA */
#endif

/**
 * EVMA_MAX_NUM_INSTANCES_PER_COMPARATOR defines the maximum number of instances,
 * that can be allocated for one comparator. Multiple instances referring to the same
 * comparator are only possible, if all the instances use the same comparator threshold value.
 * The instances might use different outputs (GPIOs, interrupts, GDMA targets)
 */
#define EVMA_MAX_NUM_INSTANCES_PER_COMPARATOR   10

/**                            common
 * EVMA_SET_INT_HANDLER registers an interrupt callback function for a specific interrupt.
 * ERTEC200P : the IRQ index is ASIC specific and is known by EVMA
 * If cbf is NULL, the interrupt has to be disabled; if cbf isn't equal to NULL, the interrupt
 * has to be enabled and the cbf must be registered.
 * @param   irq_index   index of the IRQ to be handled
 * @param   cbf         function pointer to the IRQ callback [void (func*)(void)] or NULL to disable IRQ
 */
#ifdef IM_HW_ERTEC200P
    #define EVMA_SET_INT_HANDLER(irq_index, cbf)  Bsp_EVMA_register_ISR(irq_index, cbf)
#endif

/*****************************************************************
 *                 general defines and declarations               
 *****************************************************************/
#define EVMA_MEMSET(_DST, _VAL, _LEN)  OsMemSet(_DST, _VAL, _LEN)


/*                        hardware specifc
 * register access macros
 *
 * Normally no adjustments should be necessary here ...
 */
#ifdef EVMA_USE_PNIP
        /**
         *  EVMA_WRITE_PNIP_PLL_OUT_CONTROL is used to set the Multiplexer of the PNIP PNPLL module.
         *  @param x        defines the multiplexer output index (0-20)
         *  @param value    defines the 32 bit register value to be written
         */
    #define EVMA_WRITE_PNIP_PLL_OUT_CONTROL(x, value)       EDDP_USR_REG32_WRITE(U_PNIP__IP_VERSION, U_PNIP__PLL_OUT_CONTROL_0-U_PNIP__IP_VERSION+4*x, value)

        /**
         *  EVMA_WRITE_PNIP_APPLTIMER2_COMP_TIME is used to set a comparator value of application timer 2
         *  @param x        defines the comparator index (0(1) - 6)
         *  @param value    defines the 32 bit register value to be written
         */
    #define EVMA_WRITE_PNIP_APPLTIMER2_COMP_TIME(x, value)  EDDP_USR_REG32_WRITE(U_PNIP__IP_VERSION, U_PNIP__APPLCOMPARE_2_1-U_PNIP__IP_VERSION+x*4, value)
        /**
         * EVMA_READ_PNIP_APPLTIMER2_TIME reads the current timer value of application timer 2
         */
    #define EVMA_READ_PNIP_APPLTIMER2_TIME()                EDDP_USR_REG32_READ(U_PNIP__IP_VERSION, U_PNIP__APPLTIMER_2-U_PNIP__IP_VERSION)
        /**
         * EVMA_READ_PNIP_APPLTIMER2_LENGTH reads the cycle length of application timer 2
         */
    #define EVMA_READ_PNIP_APPLTIMER2_LENGTH()              EDDP_USR_REG32_READ(U_PNIP__IP_VERSION, U_PNIP__APPLLENGTH_2-U_PNIP__IP_VERSION)
        /**
         * EVMA_READ_PNIP_APPLTIMER2_COMP_TIME reads a comparator value of application timer 2. 0(1) <= x <= 6 (= comparator index)
         */
    #define EVMA_READ_PNIP_APPLTIMER2_COMP_TIME(x)          EDDP_USR_REG32_READ(U_PNIP__IP_VERSION, U_PNIP__APPLCOMPARE_2_1-U_PNIP__IP_VERSION+4*x)

#endif /* end of EVMA_USE_PNIP */


/**                 common

 * error handling
 */

    /** if you define EVMA_CFG_FATAL_ERROR_ON_FAILURE, EVMA will go to fatal error if an assert fails */
#define EVMA_CFG_FATAL_ERROR_ON_FAILURE

    /** the lsa_component_id is passed to the fatal-error handler within the error detail structure */
#define EVMA_LSA_COMPONENT_ID                    PNIO_PACKID_EVMA
    /** set the fatal error handler here. it gets the error detail structure as a parameter (type of the structure : LSA_FATAL_ERROR_TYPE) */
extern void PNIO_FatalError(LSA_FATAL_ERROR_TYPE* pLsaErr);
#define EVMA_FATAL_ERROR(_ERROR_DETAIL_PTR)      PNIO_FatalError(_ERROR_DETAIL_PTR)
    /** set the module id of evma_pnpll.c here. It is passed to the fatal error handler through the detail structure
     *  (-> field ERROR_DETAIL_PTR.module_id)*/
#define EVMA_MODULE                              0x01


#endif /* EVMA_CFG_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
