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
/*  F i l e               &F: evma_tcm_enter.h                          :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  this file lets all following function- and variable definitions          */
/*  be located in the TCM section                                            */
/*                                                                           */
/*****************************************************************************/


#ifdef EVMA_USE_TCM

#if defined (TOOL_CHAIN_GREENHILLS_ARM)
    #pragma ghs section bss=EVMA_DTCM_SECTION_NAME
    #pragma ghs section text=EVMA_ITCM_SECTION_NAME
#elif defined (TOOL_CHAIN_GNU_PPC)
    #define EVMA_ATTRIBUTE_DTCM __attribute__ ((section (EVMA_DTCM_SECTION_NAME)))
    #define EVMA_ATTRIBUTE_ITCM __attribute__ ((section (EVMA_ITCM_SECTION_NAME)))
#else
    #define EVMA_ATTRIBUTE_DTCM __attribute__ ((section (EVMA_DTCM_SECTION_NAME)))
    #define EVMA_ATTRIBUTE_ITCM __attribute__ ((section (EVMA_ITCM_SECTION_NAME)))
#endif

#endif



#ifndef EVMA_ATTRIBUTE_DTCM
    #define EVMA_ATTRIBUTE_DTCM
#endif
#ifndef EVMA_ATTRIBUTE_ITCM
    #define EVMA_ATTRIBUTE_ITCM
#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
