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
/*  F i l e               &F: os_cfg.h                                  :F&  */
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
#ifndef OS_CFG_H                         /* ----- reinclude-protection ----- */
#define OS_CFG_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

#include "compiler.h"


/*TCM usage*/
#define OS_BSS_DTCM_SECTION_NAME            ".xdata_bss_dtcm"
#define OS_BSS_STACK_SECTION_NAME           ".bss_stack"
#define OS_DATA_DTCM_SECTION_NAME           ".xdata_d_tcm"
#define OS_ITCM_SECTION_NAME                ".xtext_i_tcm"

#define OS_BSS_FAST                         __attribute__((section (OS_BSS_DTCM_SECTION_NAME)))
#define OS_BSS_STACK_FAST                   __attribute__((section (OS_BSS_STACK_SECTION_NAME)))
#define OS_DATA_FAST                        __attribute__ ((section (OS_DATA_DTCM_SECTION_NAME)))
#define OS_CODE_FAST                        __attribute__ ((section (OS_ITCM_SECTION_NAME)))


// *----------------------------------------------------------------*
// *    options  (default: 0)
// *----------------------------------------------------------------*

/*----- Tasks ----------------------*/
#define MAXNUM_OF_TASKS              40     // number of usable tasks = MAXNUM_OF_TASKS - 1  !!
#define MAXSIZE_TASKNAME             32     // max number of bytes for taskname


// *----------------------------------------------------------------*
// *    task stacksize
// *----------------------------------------------------------------*
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
#define OS_TASK_DEFAULT_STACKSIZE   0x1000                                        // (4k) default stack size per thread for eb200p
#define OS_TOTAL_TASK_STACKSIZE     (MAXNUM_OF_TASKS * OS_TASK_DEFAULT_STACKSIZE) // Total stack size reserved for all threads
#else
#error "platform not defined\n"
#endif

// *----------------------------------------------------------------*
// *    M E S S A G E S
// *----------------------------------------------------------------*
#define OS_MBX_PRIO_NORM            5       // default pnio message priority
#define MAXNUM_MSG_ENTRIES          360     // maximal number of entries in a message queue
#define MSG_ENTRY_SIZE              8       // size of one message entry (must be 4,8,16,32,64 bytes)


// *----------------------------------------------------------------*
// *    Timer
// *----------------------------------------------------------------*
#define PCPNIO_TIMER_MAX            150

// *----------------------------------------------------------------*
// *    Semaphores and flags
// *----------------------------------------------------------------*
#define MAXNUM_OF_NAMED_MUTEXES     OS_MUTEX_Reserved  // maximum number of Mutexes for Enter ()/Exit()
#define MAXNUM_OF_BIN_SEMAPH        70                 // maximum number of bin semaphores

#define OS_MUTEX_DEFAULT            0       // Mutex default

// *** mutexes for the other LSA layers ***
#define OS_MUTEX_XX_MEM             1       // Mutex, used by XX_MEM
#define OS_MUTEX_NV_DATA            2       // Mutex, used by nv ram access (rd/wr)
#define OS_MUTEX_LTRC               3       // Mutex, used by LTRC
#define OS_MUTEX_PNPB               4       // Mutex, used by PNPB for user api
#define OS_MUTEX_PNPB_IO            5       // Mutex, used by PNPB for user io access
#define OS_MUTEX_PNPB_RD_PERI       6       // Mutex, used by PNPB to read from pndv-perif
#define OS_MUTEX_PNPB_WR_PERI       7       // Mutex, used by PNPB to write to pndv-perif
#define OS_MUTEX_PNPB_PLUG          8       // Mutex, used by PNPB to lock against multiple plug/pull call from user threads
#define OS_MUTEX_FLASH_DATA         9
#define OS_MUTEX_PNPB_PULL          10
// *** must be the last number.. ***
#define OS_MUTEX_Reserved           70      // must be the last one, to find out number of mutexes

#define OS_DEVKITS_MUTEX_MAX_COUNT  11

#define IO_UPDATE_POLLING_MS        1       // polling cycle time for IO update


#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
