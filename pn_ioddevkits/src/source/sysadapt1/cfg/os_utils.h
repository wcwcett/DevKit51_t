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
/*  F i l e               &F: os_utils.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  OS abstraction layer interface definition                                */
/*                                                                           */
/*****************************************************************************/
#ifndef _OS_UTILS_H
#define _OS_UTILS_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif


// *------------------------------------------------------------------------------
// * circular buffers, optional, for debugging only
// *
// * from index 0,             number of buffers = NUMOF_TIM_BUFS  used for system timers
// * from index FIRST_SYS_BUF, number of buffers = NUMOF_SYS_BUFS  used for pnio threads
// * from index FIRST_USR_BUF, number of buffers = NUMOF_USR_BUFS  free for user
// *------------------------------------------------------------------------------
#define FIRST_TIM_BUF   0
#if _DEBUG_LOGGING_CIRC_BUF1
    #define NUMOF_TIM_BUFS  PCPNIO_TIMER_MAX
#else
    #define NUMOF_TIM_BUFS  0
#endif

#define FIRST_SYS_BUF   NUMOF_TIM_BUFS
#define NUMOF_SYS_BUFS  10

#define FIRST_USR_BUF   NUMOF_TIM_BUFS + NUMOF_SYS_BUFS
#define NUMOF_USR_BUFS  20

#define NUMOF_CIRCBUFS	(NUMOF_TIM_BUFS + NUMOF_SYS_BUFS + NUMOF_USR_BUFS) // number of independent circular Buffer (number 0...20 is already used!!)

// ***** defines for DisableInt ***
#define OS_ENTRY_DEFAULT    0   // save time in OsSetEntryCircBuf
#define OS_ENTRY_START      1   // start time measurement in OsSetEntryCircBuf
#define OS_ENTRY_STOP       2   // stop time measurement and store difference in OsSetEntryCircBuf

#if  _DEBUG_LOGGING_CIRC_BUF
PNIO_VOID        OsInitCircBuf           (PNIO_VOID);
PNIO_VOID        OsEnableCircBuf         (PNIO_BOOL    Enable);
PNIO_VOID        OsSetEntryCircBuf       (PNIO_UINT32, PNIO_UINT32 DisableInt);
PNIO_VOID        OsSetEntryCircBufX      (PNIO_UINT32 Index, PNIO_UINT32 value);
PNIO_VOID        OsResetMaxMinCircBuf    (PNIO_UINT32  BufIndex);

PNIO_UINT32      OsGetLastEntryCircBuf   (PNIO_UINT32  BufIndex);
PNIO_UINT32      OsGetMaxValCircBuf      (PNIO_UINT32  BufIndex);
PNIO_UINT32      OsGetMinValCircBuf      (PNIO_UINT32  BufIndex);
PNIO_VOID        OsPrintCircBuf          (PNIO_UINT32  BufIndex);
#endif


#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
