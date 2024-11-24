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
/*  F i l e               &F: os_taskprio.h                             :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  OS dependent configuration for PNIO Devkit                               */
/*                                                                           */
/*****************************************************************************/
#ifndef OS_TASKPRIO_H                     /* ----- reinclude-protection ----- */
#define OS_TASKPRIO_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif


// *----------------------------------------------------------------*
// *    T A S K S
// *----------------------------------------------------------------*
/*----- Task Priority ----------------------*/
#include "compiler.h"

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
    #ifdef CYGDAT_NET_STACK_CFG
        #include CYGDAT_NET_STACK_CFG
    #endif
    #include <pkgconf/kernel.h>

    /* ECOS taskpriorities for standard scheduler */
    #define TASK_PRIO_OS_TIMER                  1
    #define TASK_PRIO_APPL_CYCLE_IO             2    // highest prio for IRT C3 and sync. application
    #define TASK_PRIO_RBT                       3
    #define TASK_PRIO_PSI                       3
    #define TASK_PRIO_LD                        3
    #define TASK_PRIO_HD                        3
    #define TASK_PRIO_EDDP                      3
    #define TASK_PRIO_ORG                       3
    #define TASK_PRIO_IP_STACK                  3
    #define TASK_PRIO_PNO                       3
    #define TASK_PRIO_POF                       3
    #define TASK_PRIO_STARTUP                   3
    #define TASK_PRIO_TSKMA                     3
    #define TASK_PRIO_PNPB_OWN_IND              3
    #define TASK_PRIO_PNPB                      3
    #define TASK_PRIO_OS_UDP_SOCKET             3
    #define TASK_PRIO_LOW                       3
#if (PNIOD_PLATFORM & (PNIOD_PLATFORM_ECOS_EB200P))
    #define TASK_PRIO_PRINT_POST                4
    #define TASK_PRIO_NV_DATA                   4
    #define TASK_EDC_POLL                       4
    #define TASK_PRIO_TCP_FW_LOADER             4
#else
    #define TASK_PRIO_PRINT_POST                3
    #define TASK_PRIO_NV_DATA                   3
#endif
    #define TASK_PRIO_MAIN                      5
    #define TASK_PRIO_APPL_CYCLE_IO_LOW         4
    #define TASK_PRIO_TRCMEMXT                  5
    #define TASK_PRIO_IDLE                      6

#else
    #error "platform not defined"
#endif



#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
