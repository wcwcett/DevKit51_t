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
/*  F i l e               &F: gdma_plau.h                               :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  this header checks the plausibility of the GDMA's configuration file     */
/*                                                                           */
/*****************************************************************************/

#ifndef FS_PLAU_H_
#define FS_PLAU_H_

#ifndef GDMA_CFG_HW_BASE_ERTEC200P
    #ifndef GDMA_CFG_HW_BASE_TRITON2
        #error configure either ERTEC200P or TRITON2 as your hardware base
    #endif
#endif

#ifdef GDMA_CFG_HW_BASE_ERTEC200P
    #ifdef GDMA_CFG_HW_BASE_TRITON2
        #error configure either ERTEC200P or TRITON2 as your hardware base
    #endif
#endif

#ifndef GDMA_CFG_BASE_ADDRESS
    #error set GDMA_CFG_BASE_ADDRESS - the base address of the GDMA IP
#endif

#ifndef GDMA_CFG_NUM_JOB_LISTS
    #error choose the number of job sets using GDMA_CFG_NUM_JOB_LISTS
#endif
#if GDMA_CFG_NUM_JOB_LISTS < 1
    #error GDMA_CFG_NUM_JOB_LISTS must be at least 1
#endif

#ifndef GDMA_CONVERT_TO_TARGET_HW_ADDR
    #define GDMA_CONVERT_TO_TARGET_HW_ADDR(_EXTERNAL_ADDR)   (_EXTERNAL_ADDR)
#endif

#ifdef GDMA_CFG_LOCATE_INSTRUCTIONS_TO_TCM
    #ifndef GDMA_ITCM_SECTION_NAME
        #error you forgot to specify the section name of the ITCM
    #endif
    #ifndef GDMA_DEFAULT_TEXT_SECTION_NAME
        #error you forgot to specify the section name of code (outside the ITCM)
    #endif
#endif

#endif /* FS_PLAU_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
