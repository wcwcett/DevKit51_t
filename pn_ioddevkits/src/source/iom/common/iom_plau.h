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
/*  F i l e               &F: iom_plau.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/* this header checks the plausibility of the IO manager's configuration     */
/*                                                                           */
/*****************************************************************************/

#ifndef IOM_PLAU_H_
#define IOM_PLAU_H_

/* check PERIF configuration */
#ifdef IOM_CFG_PNIP
    #ifdef IOM_CFG_PERIF_NUM_AR
        #if IOM_CFG_PERIF_NUM_AR > 8
            #error a maximum of 8 ARs have been tested
        #endif
    #else
        #error IOM_CFG_PERIF_NUM_AR has to be set
    #endif

    #ifdef IOM_CFG_PERIF_NUM_CR
        #if IOM_CFG_PERIF_NUM_CR > 27
            #error a maximum of 27 CRs have been tested
        #endif
    #else
        #error IOM_CFG_PERIF_NUM_CR has to be set
    #endif

    #ifdef IOM_CFG_LOCATE_DATA_TO_TCM
        #ifndef IOM_TCM_SECTION_NAME
            #error set IOM_TCM_SECTION_NAME to the DTCM section name
        #endif
        #ifndef IOM_DEFAULT_BSS_SECTION_NAME
            #error set IOM_DEFAULT_BSS_SECTION_NAME to the default bss section name of the IOM module
        #endif
    #endif

    #ifdef IOM_CFG_LOCATE_INSTRUCTIONS_TO_TCM
        #ifndef IOM_ITCM_SECTION_NAME
            #error set IOM_ITCM_SECTION_NAME to the ITCM section name
        #endif
        #ifndef IOM_DEFAULT_TEXT_SECTION_NAME
            #error set IOM_DEFAULT_TEXT_SECTION_NAME to the default text section name of the IOM module
        #endif
    #endif

    #ifndef IOM_SET_INT_HANDLER
        #error set IOM_SET_INT_HANDLER in order to be able to activate the data-received-interrupt
    #endif
#endif


#endif /* IOM_PLAU_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
