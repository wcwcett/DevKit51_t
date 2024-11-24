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
/*  F i l e               &F: os.h                                      :F&  */
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
#ifndef _OS_H
#define _OS_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif



// *-----------------------------------------------------------------------
// * includes
// *-----------------------------------------------------------------------
#include "compiler.h"
#include "lsa_cfg.h"
#include "pnio_types.h"
#include "os_cfg.h"
#include "os_utils.h"

/*
 *  Debug purposes
 */
//#define OS_DEBUG_RELEASES   1

// *-----------------------------------------------------------------------
// * defines
// *-----------------------------------------------------------------------
#define INVALID_TASK_ID 0

/* if (!(cond)) { printf ("Fatal Error File %s, Line %d\n", (char*) _ThisFileName_, __LINE__); } \
*/
#define OS_ASSERT(_cond_) \
    if (!(_cond_)) { LSA_TRACE_00  (TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "OS_ASSERT"); printf ("OS_ASSERT(): Fatal Error File %s, Line %d\n", (char*) __FILE__, __LINE__); while (1); }


// Task states
#define TASK_BLOCKED    0
#define TASK_ENABLED    1
#define TASK_OPERATING  2

// ****** dynamic memory Pools *****
#define MEMPOOL_DEFAULT        0
#define MEMPOOL_FAST           1        // maybe cached
#define MEMPOOL_RX_TX_BUF      2        // uncached
#define MEMPOOL_CACHED         3        // maybe cached
#define MEMPOOL_UNCACHED       4        // uncached

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
    #define PN_CLOCKS_PER_SEC   1000                // scheduler tick in ticks per second
    #define RX_TX_MEM_SIZE      0x200000            // size for memory based pRxTxMem,  located in XX_MEM
#else
    #error "no valid platform selected"
#endif

// ****** scheduling time *****
#ifndef PN_CLOCKS_PER_SEC
   #define BSP_TICKS_PER_SECOND                 PN_CLOCKS_PER_SEC
#endif

#define OS_PRINT_POST_QUEUE_DISABLE             500
#define OS_PRINT_POST_QUEUE_ENABLE              400


    // *------------------------------------------------------------------------------
    // *    os interface startup
    // *------------------------------------------------------------------------------
    PNIO_UINT32 OsInit (PNIO_VOID);

    // *------------------------------------------------------------------------------
    // *    memory management
    // *------------------------------------------------------------------------------
    PNIO_UINT32 OsAllocF  (PNIO_VOID** ppMem, PNIO_UINT32 Length);
    PNIO_UINT32 OsAlloc   (PNIO_VOID** pMem, PNIO_UINT8 Value, PNIO_UINT32 Length);
    PNIO_UINT32 OsAllocX  (PNIO_VOID** pMem, PNIO_UINT8 Value, PNIO_UINT32 Length, PNIO_UINT32 PoolId);
    PNIO_UINT32 OsFree    (PNIO_VOID* pMem);
    PNIO_UINT32 OsFreeX   (PNIO_VOID* pMem, PNIO_UINT32 PoolId);

    // *------------------------------------------------------------------------------
    // *    time management
    // *------------------------------------------------------------------------------
    PNIO_UINT32 OsAllocTimer  ( PNIO_UINT16* timer_id_ptr, PNIO_UINT16 timer_type, PNIO_UINT16 time_base, PNIO_VOID* callback_timeout);
    PNIO_UINT32 OsStartTimer  ( PNIO_UINT16 timer_id,PNIO_UINT32 user_id, PNIO_UINT16 delay);
    PNIO_UINT32 OsStopTimer   ( PNIO_UINT16 timer_id );
    PNIO_UINT32 OsFreeTimer   ( PNIO_UINT16 timer_id );
    PNIO_VOID   OsWait_ms     ( PNIO_UINT32 PauseTimeInterval_ms );
    PNIO_UINT32 OsGetTime_us  ( PNIO_VOID );
    PNIO_UINT32 OsGetTime_ms  ( PNIO_VOID );
    PNIO_UINT32 OsGetUnixTime ( PNIO_VOID );
    PNIO_UINT64 OsGetUuidTime ( PNIO_VOID );
    PNIO_UINT32 OsResetTimer  ( PNIO_UINT16 timer_id);
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
    // ***** optional hardware supported functions ****
    PNIO_UINT32 OsGetTimDiffSinceCycleStart(PNIO_VOID);
#endif

    // *------------------------------------------------------------------------------
    // *    mutex
    // *------------------------------------------------------------------------------
    PNIO_UINT32 OsAllocReentranceX ( PNIO_UINT16* lock_handle_ptr );
    PNIO_VOID   OsFreeReentranceX  ( PNIO_UINT32 MutexId );
    PNIO_UINT32 OsCreateMutex      ( PNIO_UINT16 MutexId );
    PNIO_VOID   OsEnterX           ( PNIO_UINT32 MutexId );
    PNIO_VOID   OsExitX            ( PNIO_UINT32 MutexId );
    PNIO_VOID   OsEnterShort       ( PNIO_VOID );
    PNIO_VOID   OsExitShort        ( PNIO_VOID );

    // *------------------------------------------------------------------------------
    // *    semaphore
    // *------------------------------------------------------------------------------
#if (MAXNUM_OF_BIN_SEMAPH != 0) // optional
    PNIO_UINT32 OsCreateSemB ( PNIO_UINT32 SemId);
    PNIO_UINT32 OsAllocSemB  ( PNIO_UINT32* pSemId );
    PNIO_UINT32 OsTakeSemB   ( PNIO_UINT32 SemId );
    PNIO_UINT32 OsGiveSemB   ( PNIO_UINT32 SemId );
    PNIO_UINT32 OsFreeSemB   ( PNIO_UINT32 SemId );
#endif

    // *------------------------------------------------------------------------------
    // *    thread handling
    // *------------------------------------------------------------------------------
    PNIO_UINT32 OsCreateThread             ( PNIO_VOID (*pThreadEntry)(PNIO_VOID), PNIO_VOID_PTR_TYPE TaskEntryData,PNIO_UINT8* pThreadName, PNIO_UINT32 ThreadPrio, PNIO_UINT32 TaskStackSize, PNIO_UINT32* pThreadId);
    PNIO_UINT8* OsReserveStackForThread    ( PNIO_UINT32 StackSize );
    PNIO_UINT32 OsSetThreadPrio            ( PNIO_UINT32 ThreadId, PNIO_UINT32 NewThreadPrio );
    PNIO_UINT32 OsStartThread              ( PNIO_UINT32 ThreadId );
    PNIO_UINT32 OsJoinThread               ( PNIO_UINT32 ThreadId );
    PNIO_UINT32 OsExitThread               ( PNIO_VOID );
    PNIO_VOID   OsKillOtherThreads         ( PNIO_VOID );
    PNIO_UINT32 OsWaitOnEnable             ( PNIO_VOID );
    PNIO_UINT32 OsGetThreadId              ( PNIO_VOID );

    // *------------------------------------------------------------------------------
    // *    messages
    // *------------------------------------------------------------------------------
    PNIO_UINT32 OsCreateMsgQueue            (PNIO_UINT32 ThreadId);
    PNIO_UINT32 OsDeleteMsgQueue            (PNIO_UINT32 ThreadId);
    PNIO_UINT32 OsReadMessageBlocked        (PNIO_VOID** ppMessage, PNIO_UINT32 ThreadId);
    PNIO_UINT32 OsReadMessageBlockedX       (PNIO_VOID** ppMessage1, PNIO_VOID** ppMessage2, PNIO_UINT32 ThreadId);
    PNIO_UINT32 OsSendMessage               (PNIO_UINT32 ThreadId,PNIO_VOID* pMessage,PNIO_UINT32 MsgPrio);
    PNIO_UINT32 OsSendMessageX              (PNIO_UINT32 ThreadId,PNIO_VOID* pMessage1,PNIO_VOID* pMessage2,PNIO_UINT32 MsgPrio);

    PNIO_VOID   OsIntDisable (PNIO_VOID);
    PNIO_VOID   OsIntEnable  (PNIO_VOID);

    PNIO_UINT32 OsHtonl(PNIO_UINT32 val32);
    PNIO_UINT16 OsHtons(PNIO_UINT16 val16);
    PNIO_UINT32 OsNtohl(PNIO_UINT32 val32);
    PNIO_UINT16 OsNtohs(PNIO_UINT16 val16);

    /* string.h */
    PNIO_VOID*  OsMemCpy(PNIO_VOID* pDst, const PNIO_VOID* pSrc, PNIO_UINT32 size);
    PNIO_VOID*  OsMemMove(PNIO_VOID* pDst, const PNIO_VOID* pSrc, PNIO_UINT32 size);
    PNIO_VOID*  OsMemSet(PNIO_VOID* pDst, PNIO_INT val, PNIO_UINT32 size);
    PNIO_INT    OsMemCmp(const PNIO_VOID* pBuf1, const PNIO_VOID* pBuf2, PNIO_UINT32 size);
    PNIO_INT    OsStrCmp(const PNIO_CHAR* pBuf1, const PNIO_CHAR* pBuf2);
    PNIO_INT    OsStrnCmp(const PNIO_CHAR* pBuf1, const PNIO_CHAR* pBuf2, PNIO_UINT32 size);
    PNIO_CHAR*  OsStrCpy(PNIO_CHAR* pDst, const PNIO_CHAR* pSrc);
    PNIO_CHAR*  OsStrnCpy(PNIO_CHAR* pDst, const PNIO_CHAR* pSrc, PNIO_UINT32 size);
    PNIO_UINT32 OsStrLen(const PNIO_CHAR* pBuf);

    /* stdlib.h */
    PNIO_INT    OsRand();
    PNIO_VOID   OsSrand(PNIO_UINT val);
    PNIO_INT    OsAtoi(const PNIO_CHAR* pStr);
    PNIO_CHAR*  OsStrChr(const PNIO_CHAR* pBuf, PNIO_INT c);
    PNIO_UINT8  OsGetChar (PNIO_VOID);
    PNIO_UINT32 OsKeyScan32(PNIO_CHAR* pText, PNIO_UINT32 InputBase);
    PNIO_UINT32 OsKeyScanString(PNIO_CHAR* pText, PNIO_UINT8* pStr, PNIO_UINT32 MaxLen );

    // *------------------------------------------------------------------------------
    // *    other functions
    // *------------------------------------------------------------------------------
    PNIO_VOID   OsReboot           (PNIO_VOID);
    PNIO_VOID   OsRebootService    (PNIO_VOID);
    PNIO_VOID   OsShutdownNetwork  (PNIO_VOID);
    PNIO_VOID   OsSystemRamInit    (PNIO_VOID);

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
    //#define  ECOS_KERNEL_INSTRUMENTATION

    #ifdef  ECOS_KERNEL_INSTRUMENTATION
        #include <pkgconf/kernel.h>
        #include <cyg/kernel/instrmnt.h>

        #ifdef CYGDBG_KERNEL_INSTRUMENT_USER
            #define OS_INSTRUMENT_USER(_event_,_arg1_,_arg2_)   OsIntrumentUserEvent(_event_,_arg1_,_arg2_)
            #define OS_INSTRUMENT_USER_START(_arg1_,_arg2_)     OsIntrumentUserStart(_arg1_,_arg2_)
            #define OS_INSTRUMENT_USER_STOP(_arg1_,_arg2_)      OsIntrumentUserStop(_arg1_,_arg2_)
        #else
            #define OS_INSTRUMENT_USER(_event_,_arg1_,_arg2_)
            #define OS_INSTRUMENT_USER_START(_arg1_,_arg2_)
            #define OS_INSTRUMENT_USER_STOP(_arg1_,_arg2_)
            #endif
    #else
        #define OS_INSTRUMENT_USER(_event_,_arg1_,_arg2_)
        #define OS_INSTRUMENT_USER_START(_arg1_,_arg2_)
        #define OS_INSTRUMENT_USER_STOP(_arg1_,_arg2_)
    #endif
#else
    #define OS_INSTRUMENT_USER(_event_,_arg1_,_arg2_)
    #define OS_INSTRUMENT_USER_START(_arg1_,_arg2_)
    #define OS_INSTRUMENT_USER_STOP(_arg1_,_arg2_)
#endif


#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
