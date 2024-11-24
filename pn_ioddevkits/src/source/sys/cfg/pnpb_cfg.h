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
/*  F i l e               &F: pnpb_cfg.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
#ifndef PNPB_CFG_H
#define PNPB_CFG_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

    extern void PNIO_FatalError(LSA_FATAL_ERROR_TYPE* pLsaErr);
 
    #define PNPB_FATAL_ERROR(_ERROR_DETAIL_PTR)        PNIO_FatalError(_ERROR_DETAIL_PTR)

    #define PNPB_COPY_BYTE(_DEST_PTR ,_SRC_PTR, _LEN)                                      \
    {                                                                                      \
        OsMemCpy((PNIO_VOID*)(_DEST_PTR), (PNIO_VOID*)(_SRC_PTR), _LEN);                   \
    }

    #define PNPB_CMPR_BYTE(_DEST_PTR ,_SRC_PTR, _LEN)                                      \
    {                                                                                      \
        OsMemCmp((PNIO_VOID*)_DEST_PTR, (PNIO_VOID*)_SRC_PTR, _LEN);                       \
    }

    #define PNPB_ALLOC_MEM(_MEM_PTR_PTR, _VAL, _LENGTH)                                    \
    {                                                                                      \
        OsAlloc((PNIO_VOID**)(_MEM_PTR_PTR), _VAL, _LENGTH);                               \
    }
    
    #define PNPB_FREE_MEM(_MEM_PTR)  OsFree(_MEM_PTR)

    #define PNPB_MEMSET(_DST_PTR, _VAL, _LEN)                                              \
    {                                                                                      \
        OsMemSet((PNIO_VOID*)(_DST_PTR), _VAL, _LEN);                                      \
    }

    #define PNPB_ALLOC_TIMER(_TIMER_ID_PTR, _TIMER_TYPE, _TIME_BASE, _TIMEOUT_PTR)         \
    {                                                                                      \
        OsAllocTimer(_TIMER_ID_PTR, _TIMER_TYPE, _TIME_BASE, _TIMEOUT_PTR);                \
    }

    #define PNPB_START_TIMER(_TIMER_ID, _USER_ID, _DELAY)                                  \
    {                                                                                      \
        OsStartTimer(_TIMER_ID, _USER_ID, _DELAY);                                         \
    }
    
    #define PNPB_STOP_TIMER(_TIMER_ID)       OsStopTimer(_TIMER_ID)
    #define PNPB_RESET_TIMER(_TIMER_ID)      OsResetTimer(_TIMER_ID)

    #define PNPB_GET_THREAD_ID()             OsGetThreadId()

    #define PNPB_SEM_B_ALLOC(_SEM_ID_PTR)    OsAllocSemB(_SEM_ID_PTR)
    #define PNPB_SEM_B_TAKE(_SEM_ID)         OsTakeSemB(_SEM_ID)
    #define PNPB_SEM_B_GIVE(_SEM_ID)         OsGiveSemB(_SEM_ID)

    #define PNPB_TASK_CREATE(_FCT_PTR, _ENTRY_DATA, _TSK_NAME, _PRIO, _STACK_SIZE, _ID_PTR)\
    {                                                                                      \
        OsCreateThread (_FCT_PTR,     /* [in] pointer to task handler */                   \
                        _ENTRY_DATA,  /* [in] argument to be passed to thread func */      \
                        _TSK_NAME,    /* Taskname for Debugging */                         \
                        _PRIO,        /* [in] task priority */                             \
                        _STACK_SIZE,  /* [in] stack size of task */                        \
                        _ID_PTR);     /* [out] pointer to Task ID */                       \
    }

    #define PNPB_TASK_START(_ID_PTR)         OsStartThread(_ID_PTR)

    #define PNPB_MBOX_CREATE(_TASK_ID)       OsCreateMsgQueue(_TASK_ID)

    #define PNPB_SEND_MAIL(_TASK_ID, _RQB_PTR, _PRIO)                                      \
    {                                                                                      \
        OsSendMessage(_TASK_ID, _RQB_PTR, _PRIO);                                          \
    }

    #define PNPB_READ_MAIL_BLOCKED(_RQB_PTR_PTR, _TASK_ID)  \
    {                                                       \
        OsReadMessageBlocked(_RQB_PTR_PTR, _TASK_ID);       \
    }

    #define PNPB_ENTER(_ID)         OsEnterX(_ID)
    #define PNPB_EXIT(_ID)          OsExitX(_ID)

    #define PNPB_NTOHS(_VAL)        OsNtohs(_VAL)
    #define PNPB_NTOHL(_VAL)        OsNtohl(_VAL)
    #define PNPB_HTONS(_VAL)        OsHtons(_VAL)
    #define PNPB_HTONL(_VAL)        OsHtonl(_VAL)

    #define PNPB_WAIT_ON_ENABLE()   OsWaitOnEnable()

    #define PNPB_WAIT_MS(_VAL)      OsWait_ms(_VAL)

    #define PNPB_STR_LEN(_PTR)      OsStrLen((PNIO_CHAR*)_PTR)

    #define PNPB_OS_INIT()          OsInit()
    
    // return TRUE if submodule supports ProfiEnergy
    #define PNPB_ENABLE_PROFIENERGY(_SLOT_NR, _SUBSLOT_NR, _MODUL_IDENT, _SUBMODUL_IDENT)  IOD_ENABLE_PROFIENERGY

    #define PNPB_ENABLE_IM5                         IOD_ENABLE_IM5

    #define PNPB_TRIGGER_PNDV()                     {tskma_task_app_send_pndv_trigger();}
    #define PNPB_WAIT_ON_TRIGGER_PNDV()             {OsReadMessageBlocked ((PNIO_VOID**)&pDat, taskID);}

    #define PNPB_BSS_FAST                           OS_BSS_FAST
    #define PNPB_DATA_FAST                          OS_DATA_FAST
    #define PNPB_CODE_FAST                          OS_CODE_FAST

    #define PNPB_TASK_DEFAULT_STACKSIZE             OS_TASK_DEFAULT_STACKSIZE

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif


#endif


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
