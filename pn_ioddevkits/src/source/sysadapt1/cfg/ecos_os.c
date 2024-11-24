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
/*  F i l e               &F: ecos_os.c                                 :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  OS abstraction layer implementation for ECOS                             */
/*                                                                           */
/*****************************************************************************/



// *---------------------------------------------------------------------
// *    OS system adation layer
// *
// *
// *
// *
// *
// *---------------------------------------------------------------------
#define PRINT_PNIOD_PLATFORM
#define USE_DEF_VA_LIST         // must be defined before include


#include "compiler.h"
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)

    #define LTRC_ACT_MODUL_ID   112
    #define IOD_MODULE_ID       112

    #include "stdarg.h"
    #include "compiler_stdlibs.h"  // include standard header files
    #include "pniousrd.h"
    #include "edd_inc.h"
    #include "os.h"
    #include "os_cfg.h"
    #include "os_taskprio.h"
    #include "pnio_trace.h"
    #include "bspadapt.h"
    #include <cyg/kernel/kapi.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include <math.h>
    #include "usriod_cfg.h"
    #include "arm926.h"

    #include "trc_if.h"

    #include "tskma_int.h"
    #include "edd_usr.h"
    #include "ertec_inc.h"

#if MAXNUM_MSG_ENTRIES != CYGNUM_KERNEL_SYNCH_MBOX_QUEUE_SIZE
    #error CYGNUM_KERNEL_SYNCH_MBOX_QUEUE_SIZE in your ECOS-config should have the same value like MAXNUM_MSG_ENTRIES.
#endif

    #define PNIOUSR_LOWER_RQB_TYPE          EDD_RQB_TYPE

    #define RS232_PRINT_POSTTHREAD          1    // 1: print messages in a separate low prio thread 0: print directly and wait until finish

    /**************************************************************************
     * external functions
     */
    extern PNIO_UINT32  Bsp_nv_data_init (PNIO_VOID* nvDataInit, PNIO_UINT32 nvDataLen);

    #if DEBUG_TASKCNT
        PNIO_UINT32 TaskCnt[MAXNUM_OF_TASKS];  // for debugging only
    #endif

    /**************************************************************************/

    #define TICKS_TO_MS(Ticks)  \
        ((Ticks)*(CYGNUM_HAL_RTC_NUMERATOR / CYGNUM_HAL_RTC_DENOMINATOR / (1000 * 1000)))

    #define MS_TO_TICKS(PauseTime_ms)  \
        ((PauseTime_ms + ((CYGNUM_HAL_RTC_NUMERATOR / CYGNUM_HAL_RTC_DENOMINATOR / 1000000)-1))\
        /\
        (CYGNUM_HAL_RTC_NUMERATOR / CYGNUM_HAL_RTC_DENOMINATOR / 1000000)\
        )


    #define OS_MSGX_ID               0x3829acdc // any unique tag, to identify OsSendMessageX-Telegram

    static PNIO_UINT32 TskId_OsTimer;
    static PNIO_UINT32 OsInitialized;

    // *-------------------------------------------------------------
    // * debugging Macros
    // *-------------------------------------------------------------
    #define MBX_WAIT_IF_FULL_QUEUE  0
    #define LOG_MEM_ALLOCS          0       // log memory alloc/free
    #define LOG_MSG_SND_REC         0       // test only: log numof send/recv. messages
    #define DEBUG_TIMCNT            1
    #define RS232_BUF_SIZE          512     // buffer size for RS232 messages

    PNIO_UINT32     PnioLogDest = 1;   // debug logging   0:none, 1:Console, 2:Tracebuffer


    // *-------DEBUG--------
    PNIO_UINT32 os_log_level = 1;
    #define OS_LOG_FCT(fn_) static char* _os_log_fct = fn_;

    #define OS_DEBUG        0        // != 0: debug infos for OS are stored

    // *-----------------------------------------------------
    // * defines
    // *-----------------------------------------------------
    #define INVALID         0xffff


    // *---------------------------------*
    // * task list incl. message queues
    // *---------------------------------*
    typedef struct
    {
      PNIO_BOOL     Occupied;           // valid, if value = 1, 0: invalid
      //PNIO_UINT32    volatile State;  // Task state (BLOCKED/ENABLED/OPERATING)
      PNIO_UINT32   MsgQueueId;         // ID of the message queue
      PNIO_UINT32   MsgQueueFillStat;   // fill level indicator
      PNIO_UINT32   MsgQueueMaxFillStat;// maximum value of fill level (for debug statistics only)
      PNIO_UINT32   MsgQueueNumOfSnd;   // number of sent messages
      PNIO_UINT32   MsgQueueNumOfRec;   // number of received messages
      PNIO_UINT32   MsgQueueErrCnt;     // counter for message queue error
      PNIO_VOID*    pMsgQueueMem;       // pointer to the memalloc source data block
      PNIO_VOID*    pTaskStack;         // pointer to start of the task-stack
      PNIO_UINT32   StackSize;          // size in byte of the stack
      cyg_handle_t  MsgQueue;           /* pointer to a task message queue (identifier) */
      cyg_mbox      MsgBox;
      cyg_handle_t  ThreadId;           /* task id in the system range                  */
      cyg_thread    Thread;             /* thread structure */
      PNIO_UINT8 TaskName[MAXSIZE_TASKNAME];
    } TASK_PROPERTIES;

    // *---------------------------------*
    // * semaphore list
    // *---------------------------------*
#if (MAXNUM_OF_BIN_SEMAPH != 0)    // optional
    typedef struct
    {
        cyg_sem_t              Mutex;        // Mutex ID
        PNIO_UINT16            Allocated;    // allocate/free tag
    } SEM_ALLOC_ENTRY;

    OS_BSS_FAST SEM_ALLOC_ENTRY Semaph [MAXNUM_OF_BIN_SEMAPH];  // free mutexes (may be used by dynamically allocate/free)
#endif

    static TASK_PROPERTIES  TaskProp[MAXNUM_OF_TASKS];

    // *---------------------------------*
    // * mutexes
    // *---------------------------------*
    OS_BSS_FAST struct {
        cyg_mutex_t mutex;
        PNIO_UINT32 count;
        PNIO_BOOL   isUsed;
    } NamedMutex[MAXNUM_OF_NAMED_MUTEXES];

    // *---------------------------------*
    // * event flags
    // *---------------------------------*
#if (MAXNUM_OF_EVFLAGS != 0)    // optional
    typedef struct
    {
        sem_t           EvHndl;         // event handle from the operating system
        PNIO_UINT32     Allocated;      // allocate/free tag
    } EVFLAG_ALLOC_ENTRY;

    static EVFLAG_ALLOC_ENTRY    EvFlag[MAXNUM_OF_EVFLAGS];        // free mutexes (may be used by dynamically allocate/free)
#endif

    // *---------------------------------*
    // * timer
    // *---------------------------------*

    typedef struct
    {
        PNIO_BOOL       Occupied;           // PNIO_TRUE: timer is occupied, PNIO_FALSE: timer is free
        PNIO_BOOL       Running;            // PNIO_FALSE: timer is not running, PNIO_TRUE: timer is running
        /* PNIO_BOOL    Used;               // Used to meassure usage of SWTimer */
        cyg_handle_t    TxTimer;            // ID of the actual Timer
        PNIO_VOID       (*callback_timeout) (PNIO_UINT16 timer_id,
                                             PNIO_UINT32 user_id);
        PNIO_UINT32     user_id;            // user id, handled to the callback function
        PNIO_UINT32     timer_base;         // timer base 1/10/100 msec
        PNIO_UINT32     timer_typ;
        PNIO_UINT32     TimeVal_ms;         // timer value in msec
        PNIO_UINT32     TimeVal_Ticks;      // timer value in Ticks
        cyg_alarm       Alarm;              // instance memory for the actual Timer
    } OS_TIMER;

    OS_BSS_FAST OS_TIMER        OsTimer[PCPNIO_TIMER_MAX];
    #if DEBUG_TIMCNT
        static PNIO_UINT32      OsTimCounter[PCPNIO_TIMER_MAX];
    #endif


#if (RS232_PRINT_POSTTHREAD)

    static PNIO_UINT32 TskId_PrintPost = 0;
    static PNIO_VOID   PrintRS232PostTask (PNIO_VOID);

    static PNIO_UINT32 PrintPostQueueSize   = 0;
    static PNIO_BOOL   PrintPostQueueEnable = PNIO_TRUE;

    #define PRINT_POST_QUEUE_DISABLE    500        /* disable   messages to PrintRS232PostTask when queue is above */
    #define PRINT_POST_QUEUE_ENABLE     400        /* re-enable messages to PrintRS232PostTask when queue is below */

#endif

    // *---------------------------------*
    // * interrupt locking
    // *---------------------------------*
    /* eCos supports recursive int_disable/_enable */
    #define M_INTERR_DISABLE    cyg_interrupt_disable();
    #define M_INTERR_ENABLE     cyg_interrupt_enable();

    /* ------------------------------------------------------ */
    /* STACK ---------- have to be placed in DTCM ----------- */
    /* Valid only for EB200P and EB200P-2 based HW ---------- */
    /* ------------------------------------------------------ */
    OS_BSS_STACK_FAST __attribute__((aligned(32))) PNIO_UINT8 TaskStack[OS_TOTAL_TASK_STACKSIZE];
    static PNIO_UINT8* g_pTaskStackMem = &TaskStack[0];

    static PNIO_INT32  TaskOsTimer  (void);
#if LOG_MSG_SND_REC
    static PNIO_UINT32 MsqQueueMaxFillStatAll = 0;
#endif


    // *-----------------------------------------------------------*
    // *  OsInit ()
    // *
    // *  must be called, before task mainAppl is started. Initializes and
    // *  provides system ressources like memory, interprocess communi-
    // *  cation mechanism, task handling.
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsInit (void)
    {
        PNIO_UINT32 i;
        PNIO_UINT32 Status;

        PNIO_UINT8 MutexName[12]; // length = 12 byte, MutexName[9] = Index

        OsInitialized = PNIO_FALSE;

        if (OsInitialized == PNIO_TRUE)
        {
            LSA_TRACE_00  (TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_ERROR, "OsInit called twice\n\n");
            return (PNIO_FALSE);
        }

        OsMemSet (&MutexName[0], 0, sizeof (MutexName));
        OsMemCpy (&MutexName[0], "PnioMutex  ", sizeof (MutexName));

#if DEBUG_TASKCNT
        OsMemSet (&TaskCnt[0], 0, sizeof (TaskCnt));
#endif

        // *----------------------------------------------------
        // * initialize data for taskmanagement
        // *----------------------------------------------------
        OsMemSet (&TaskProp[0], 0, sizeof (TaskProp));        // init memory for task properties
        for (i = 0; i < MAXNUM_OF_TASKS; i++)
        {
            TaskProp[i].Occupied = PNIO_FALSE;
        }

        // *----------------------------------------------------
        // *  creates named mutexes for OsEnterX() and OsExitX()
        // *  mutexes which will be used by PN stack are not created.
        // *  (must be counting mutexes)
        // *----------------------------------------------------
        for (i = 0; i < OS_DEVKITS_MUTEX_MAX_COUNT; i++)
        {
            MutexName[9] = '0' + (i / 10);
            MutexName[10]= '0' + (i % 10);
            /* NOTE eCOS Kernel Configuration - Priority Inversion Protection Protocols */
            PNIO_UINT32 create_mut_ret_val = OsCreateMutex((PNIO_UINT16)i);
            OS_ASSERT(create_mut_ret_val == PNIO_OK);
        }

        // *----------------------------------------------------
        // *  create all binary semaphores
        // *----------------------------------------------------
#if (MAXNUM_OF_BIN_SEMAPH != 0)   /* optional */
        OsMemSet(&Semaph[0], 0, sizeof (Semaph));  // init memory for task properties
        for (i = 0; i < MAXNUM_OF_BIN_SEMAPH; i++)
        {
            // create semaphore
            PNIO_UINT32 create_sema_ret_val = OsCreateSemB(i);
            if (create_sema_ret_val != PNIO_OK)
            {
                return create_sema_ret_val;
            }
        }
#endif /* (MAXNUM_OF_BIN_SEMAPH != 0) */

#if (MAXNUM_OF_EVFLAGS != 0)
            // *----------------------------------------------------
            // *  create a Event Flag for use by the application.
            // *  and initialize data for eventflag management
            // *----------------------------------------------------
            OsMemSet(&EvFlag, 0, sizeof (EvFlag));    // init memory for task properties
            for (i = 0; i < MAXNUM_OF_EVFLAGS; i++)
            {
                status = sem_init(&EvFlag[i].EvHndl,0,0);
                if (status == ERROR)
                    LSA_TRACE_01(TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "Error Event create\n", status);

                EvFlag[i].Allocated = PNIO_FALSE;
            }
#endif


        // *----------------------------------------------------
        // * initialize timer management
        // *----------------------------------------------------
#if DEBUG_TIMCNT
            OsMemSet(&OsTimCounter[0], 0, sizeof (OsTimCounter));
#endif
        OsMemSet(&OsTimer[0], 0, sizeof (OS_TIMER));            // init memory for the timers
        for (i = 0; i < PCPNIO_TIMER_MAX; i++)
        {
            OsTimer[i].Occupied = PNIO_FALSE;
            OsTimer[i].Running  = PNIO_FALSE;
         /* OsTimer[i].Used     = PNIO_FALSE; */
        }

        // **** mark OS adaptation initialized  before calling any OS - services ***
        OsInitialized = PNIO_TRUE;

        // *** set init value for Srand ****
        OsSrand ( (unsigned) OsGetTime_ms());

#if (RS232_PRINT_POSTTHREAD)
        Status = OsCreateThread(PrintRS232PostTask, 0, (PNIO_UINT8*)"PrintRS232PostTask", TASK_PRIO_PRINT_POST, OS_TASK_DEFAULT_STACKSIZE, &TskId_PrintPost);
        PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: OsCreateThread\n");
        Status = OsCreateMsgQueue(TskId_PrintPost);
        PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: OsCreateMsgQueue\n");
        Status = OsStartThread(TskId_PrintPost);
        PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: OsStartThread\n");
#endif

        // *** start a high prior task, that handles timer events in  task context
        Status = OsCreateThread((void(*)(void))TaskOsTimer, 0, (PNIO_UINT8*) "Task_OS_Timer", TASK_PRIO_OS_TIMER, OS_TASK_DEFAULT_STACKSIZE, &TskId_OsTimer);
        PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: OsCreateThread\n");
        Status = OsCreateMsgQueue(TskId_OsTimer);
        PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: OsCreateThread\n");
        Status = OsStartThread(TskId_OsTimer);
        PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: OsCreateThread\n");

        // **** initialize circular buffer for debug entries ***
#if  _DEBUG_LOGGING_CIRC_BUF
        OsInitCircBuf ();
#endif

#if (0 == IOD_USED_WITH_XHIF_HOST)
        /* If used with XHIF host, initialization is performed by
         * command send from host */
        Bsp_nv_data_init (NULL, 0);
#endif

        return (LSA_OK);
    }

    // *----------------------------------------------------------------*
    // *
    // *  OsHtonl, OsNtohl  ()
    // *  OsHtons, OsNtohs  ()
    // *----------------------------------------------------------------*
    // *  host to network conversion and vice versa
    // *
    // *----------------------------------------------------------------*

    PNIO_UINT16 OsHtons (PNIO_UINT16 val16)
    {
        #if PNIO_BIG_ENDIAN
            return (val16);
        #else
            return (((val16 & 0xFF)<<8 ) | ((val16 & 0xFF00)>>8));
        #endif
    }

    PNIO_UINT32 OsHtonl (PNIO_UINT32 val32)
    {
        #if PNIO_BIG_ENDIAN
            return (val32);
        #else
            return (((val32 & 0xFF)<<24 ) | ((val32 & 0xFF00)<<8) | ((val32 & 0xFF0000)>>8) | ((val32 & 0xFF000000)>>24));
        #endif
    }

    PNIO_UINT16 OsNtohs (PNIO_UINT16 val16)
    {
        #if PNIO_BIG_ENDIAN
            return (val16);
        #else
            return (((val16 & 0xFF)<<8 ) | ((val16 & 0xFF00)>>8));
        #endif
    }

    PNIO_UINT32 OsNtohl (PNIO_UINT32 val32)
    {
        #if PNIO_BIG_ENDIAN
            return (val32);
        #else
            return (((val32 & 0xFF)<<24 ) | ((val32 & 0xFF00)<<8) | ((val32 & 0xFF0000)>>8) | ((val32 & 0xFF000000)>>24));
        #endif
    }


    // *----------------------------------------------------------------*
    // *  Map standard functions to OS-Interface
    // *
    // *  OsMemCpy   ()   copy memory block
    // *  OsMemMove  ()   move memory block
    // *  OsMemSet   ()   fill memory block with a defined value
    // *  OsdMemCmp  ()   compare Memory block with character
    // *  OsStrChr   ()   find character in a string
    // *  OsStrCmp   ()   compare string
    // *  OsStrnCmp  ()   compare part of a string
    // *  OsStrCpy   ()   copy string
    // *  OsStrnCpy  ()   copy string with maxlen check
    // *  OsStrLen   ()   evaluate string length
    // *  OsAtoi     ()   convert ascii to integer value
    // *  OsRand     ()   get a random number
    // *  OsSrand    ()   initialize random number generator
    // *----------------------------------------------------------------*

    PNIO_VOID* OsMemCpy(PNIO_VOID* pDst, const PNIO_VOID* pSrc, PNIO_UINT32 size)
    {
        if (size == 0)
        {
            return pDst;
        }

        if (((PNIO_UINT8*)pDst >= ((PNIO_UINT8*)pSrc + size)) || (((PNIO_UINT8*)pDst + size) <= (PNIO_UINT8*)pSrc))
        {
            ;
        }
        else
        {
            unsigned int i;
            if (pSrc < pDst)
            {
                for (i = size; i != 0; i--)
                {
                    *(((PNIO_UINT8*)pDst) + (i-1)) = *(((PNIO_UINT8*)pSrc) + (i-1));
                }
                return pDst;
            }
            else
            {
                for (i = 0; i < size; i++)
                {
                    *(((PNIO_UINT8*)pDst) + i) = *(((PNIO_UINT8*)pSrc) + i);
                }
                return pDst;
            }
        }
        return (memcpy(pDst, pSrc, size));
    }

    PNIO_VOID* OsMemMove(PNIO_VOID* pDst, const PNIO_VOID * pSrc, PNIO_UINT32 size)
    {
        return memmove(pDst,pSrc,size);
    }

    PNIO_VOID* OsMemSet(PNIO_VOID* pDst,PNIO_INT val,PNIO_UINT32 size)
    {
        return memset(pDst,val,size);
    }

    PNIO_INT OsMemCmp(const PNIO_VOID* pBuf1,const PNIO_VOID* pBuf2, PNIO_UINT32 size)
    {
        return memcmp(pBuf1,pBuf2,size);
    }

    PNIO_INT OsRand(void)
    {
        return rand();
    }

    PNIO_VOID OsSrand(PNIO_UINT val)
    {
        srand(val);
    }

    PNIO_CHAR* OsStrChr(const PNIO_CHAR* pBuf, PNIO_INT c)
    {
        return strchr(pBuf,c);
    }

    PNIO_INT OsStrCmp(const PNIO_CHAR* pBuf1, const PNIO_CHAR* pBuf2)
    {
        return strcmp(pBuf1,pBuf2);
    }

    PNIO_INT OsStrnCmp(const PNIO_CHAR* pBuf1, const PNIO_CHAR* pBuf2, PNIO_UINT32 size)
    {
        return strncmp(pBuf1,pBuf2,size);
    }

    PNIO_CHAR* OsStrCpy(PNIO_CHAR* pDst, const PNIO_CHAR* pSrc)
    {
        return strcpy(pDst,pSrc);
    }

    PNIO_CHAR* OsStrnCpy(PNIO_CHAR* pDst, const PNIO_CHAR* pSrc, PNIO_UINT32 size)
    {
        return strncpy(pDst,pSrc,size);
    }

    PNIO_UINT32 OsStrLen(const PNIO_CHAR* pBuf)
    {
        return strlen(pBuf);
    }

    PNIO_INT OsAtoi(const PNIO_CHAR* pStr)
    {
        return atoi(pStr);
    }

    // *----------------------------------------------------------------*
    // *
    // *  OsGetChar (void)
    // *
    // *----------------------------------------------------------------*
    // *  read a character (pressed key) from console
    // *
    // *----------------------------------------------------------------*
    PNIO_UINT8  OsGetChar (void)
    {
        PNIO_UINT8 Char = getchar();
        while ((Char == 0x0a) || (Char == 0x00))   // hide carriage return and NUL
        {
            Char = getchar();
        }

        return (Char);
    }

    /**
     * @brief get formated string from input
     *
     * @param[in]   pText       PNIO_CHAR*     text which should be sent to stdout
     * @param[in]   *pStr       PNIO_UINT32*   pointer to input buffer
     *
     * @return  PNIO_UINT32 - length of read string
     *
     */
    PNIO_UINT32 OsKeyScanString(PNIO_CHAR* pText, PNIO_UINT8* pStr, PNIO_UINT32 MaxLen )
    {
        PNIO_INT32 len;
        PNIO_UINT32 NewChar;
        if (pText)
        {
            PNIO_printf(pText);
        }

        for (len = 0; len < MaxLen;)
        {
            NewChar = getchar();
            if (0x0a == NewChar)    /* enter */
            {
                /* ignore first enter */
                if (0 == len)
                {
                    /* do nothing, not even increment len */
                }
                else
                {    /* some chars, then enter = valid input ended by enter */
                    pStr[len] = 0x00;    /* \0 */
                    break;
                }
            }
            else
            {
                pStr[len] = NewChar;
                len++;
            }
        }

        if (len == MaxLen)
        {
            PNIO_printf("ERROR: Input sequence too long\n");
            return 0;
        }

        return len;    /* do not count last \0 character */
    }


    // *----------------------------------------------------------------*
    // *
    // *  OsKeyScan32 (PNIO_UINT8* pText, PNIO_UINT32 InputBase)
    // *
    // *----------------------------------------------------------------*
    // *  prints the text and reads a value from console
    // *  new io data exchange.
    // *
    // *  Input:    pText       Textstring to print
    // *            InputBase   10:  decimal base (default), 16: hex base, others: default
    // *  Output:   return      Value
    // *----------------------------------------------------------------*
    PNIO_UINT32 OsKeyScan32(PNIO_CHAR* pText, PNIO_UINT32 InputBase)
    {
        PNIO_UINT32 val32;

        if (pText)
        {
            PNIO_ConsolePrintf(pText);
        }

        if (InputBase == 16)
        {
            scanf ("%lx", &val32);
        }
        else
        {
            scanf ("%ld", &val32);
        }

        return (val32);
    }


    // *-----------------------------------------------------------*
    // *  OsAllocX ()
    // *
    // *  Alloc a memory block from a specified memory pool.
    // *  The block is initialized with a predefined value.
    // *
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsAllocX(void**      ppMem,
                         PNIO_UINT8  Value,
                         PNIO_UINT32 Length,
                         PNIO_UINT32 PoolId)
    {
        PNIO_UINT32 retval = OsAllocF(ppMem, Length);
        if (retval == PNIO_OK)
        {
            OsMemSet(*ppMem, (PNIO_UINT32)Value, Length);
            LSA_UNUSED_ARG(PoolId);
        }
        else
        {
            LSA_TRACE_03(TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_ERROR,
                         "OsAllocX: pMem = 0x%x, Length = %d, Pool = %d\n",
                         *ppMem,
                         Length,
                         PoolId);
        }

       return (retval);
    }


    // *-----------------------------------------------------------*
    // *  OsFreeX ()
    // *
    // *-----------------------------------------------------------*
    // *  deallocs an occupied memory block from a specified
    // *  memory pool (threadX provides the the pool selection for an
    // *  tx_byte_release (), so no case-switch is necessary.
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsFreeX(void* pMem, PNIO_UINT32 PoolId)
    {
        LSA_UNUSED_ARG (PoolId);
        OS_ASSERT (OsInitialized == PNIO_TRUE);
        // ******* free memory ******
        if (pMem)
        {
           free(pMem);
        }

        return (LSA_OK);
    }

    // *-----------------------------------------------------------*
    // *  OsAllocF ()
    // *
    // *-----------------------------------------------------------*
    // *  alloc a memory block from default pool.
    // *  The block is not initialized.
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsAllocF(void**      ppMem,
                         PNIO_UINT32 Length)
    {
       OS_ASSERT(OsInitialized == PNIO_TRUE);

        if (Length == 0)
        {
            LSA_TRACE_02(TRACE_SUBSYS_IOD_SYSADAPT,
                         LSA_TRACE_LEVEL_ERROR,
                         "OsAllocF: pMem = 0x%x, Length = %d\n",
                         *ppMem,
                         Length);
            *ppMem = 0;
            return (PNIO_NOT_OK);
        }

        *ppMem = malloc(Length);
        if (NULL == *ppMem)
        {
            return PNIO_NOT_OK;
        }

        return PNIO_OK;
    }

    // *-----------------------------------------------------------*
    // *  OsAlloc  ()
    // *
    // *-----------------------------------------------------------*
    // *  alloc a memory block from default pool and initialize
    // *  it with a predefined value.
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsAlloc(void**      pMem,
                        PNIO_UINT8  Value,
                        PNIO_UINT32 Length)
    {
        return OsAllocX(pMem,Value,Length,MEMPOOL_DEFAULT);
    }

    // *-----------------------------------------------------------*
    // *  OsFree ()
    // *
    // *-----------------------------------------------------------*
    // *  free a memory block from default pool
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsFree (void* pMem)
    {
        return OsFreeX(pMem,MEMPOOL_DEFAULT);
    }


    // *-----------------------------------------------------------*
    // *  OsWait_ms  ()
    // *
    // *-----------------------------------------------------------*
    // *  wait a predefined time in msec.
    // *-----------------------------------------------------------*
    void OsWait_ms (PNIO_UINT32 PauseTimeInterval_ms)
    {
#ifdef CYGSEM_KERNEL_SCHED_TIMEPART
        cyg_thread_delay( MS_TO_TICKS(PauseTimeInterval_ms));
#else
        cyg_thread_delay(
            (PauseTimeInterval_ms + ((CYGNUM_HAL_RTC_NUMERATOR / CYGNUM_HAL_RTC_DENOMINATOR / 1000000)-1))
            /
            (CYGNUM_HAL_RTC_NUMERATOR / CYGNUM_HAL_RTC_DENOMINATOR / 1000000)
            );
#endif
    }


    // *-----------------------------------------------------------*
    // *  OsIntDisable ()
    // *  disable interrupts
    // *-----------------------------------------------------------*
    // *  wait a predefined time in msec.
    void OsIntDisable(void)
    {
        M_INTERR_DISABLE
    }


    // *-----------------------------------------------------------*
    // *  OsIntEnaable ()
    // *  enaable interrupts
    // *-----------------------------------------------------------*
    void OsIntEnable(void)
    {
        M_INTERR_ENABLE
    }

    // *-----------------------------------------------------------*
    // *  OsGetTime_us ()
    // *
    // *  get the actual time, value in microseconds
    // *
    // *
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsGetTime_us (void)
    {
        OS_ASSERT (OsInitialized == PNIO_TRUE);

#ifdef CYGSEM_KERNEL_SCHED_TIMEPART
        return (TICKS_TO_MS(cyg_current_time())*1000);
#else
        return (cyg_current_time()*(CYGNUM_HAL_RTC_NUMERATOR/CYGNUM_HAL_RTC_DENOMINATOR/1000));
#endif
    }


    // *-----------------------------------------------------------*
    // *  OsGetTimDiffSinceCycleStart ()
    // *
    // *  get the actual time, value in microseconds from hardware
    // *
    // *
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsGetTimDiffSinceCycleStart(void)
    {
        PNIO_UINT32 ret_val = 0;
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
        ret_val = EVMA_GET_CYCLE_TIME();
#endif
        return (ret_val);
    }

    // *-----------------------------------------------------------*
    // *  OsGetTime_ms ()
    // *
    // *  get the actual time, value in milliseconds
    // *
    // *
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsGetTime_ms (void)
#ifdef CYGSEM_KERNEL_SCHED_TIMEPART
    {
       OS_ASSERT (OsInitialized == PNIO_TRUE);
       return (TICKS_TO_MS(cyg_current_time()));
    }
#else
    {
       OS_ASSERT (OsInitialized == PNIO_TRUE);
       return (cyg_current_time()*(CYGNUM_HAL_RTC_NUMERATOR/CYGNUM_HAL_RTC_DENOMINATOR/(1000*1000)));
    }
#endif

    // *-----------------------------------------------------------*
    // *  OsGetUnixTime ()
    // *
    // *  get the actual time in UNIX format (seconds since 01.01.1970, 00:00:00
    // *
    // *
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsGetUnixTime (void)
    {
       OS_ASSERT (OsInitialized == PNIO_TRUE);
#ifdef CYGSEM_KERNEL_SCHED_TIMEPART
       return (TICKS_TO_MS(cyg_current_time())/1000);
#else
       return (cyg_current_time()/((1000*1000*1000)/(CYGNUM_HAL_RTC_NUMERATOR/CYGNUM_HAL_RTC_DENOMINATOR)));
#endif
    }

    PNIO_UINT64 OsGetUuidTime ( void )
    {
        PNIO_UINT64 time;

        // RFC constant
        static const PNIO_UINT64 num100nsec1582 = 0x01b21dd213814000;

        time = cyg_current_time() + num100nsec1582;

        // return
        return time;
    }

    // *-----------------------------------------------------------*
    // *  OsCreateMsgQueue ()
    // *
    // *  Create a message queue,  assigned to the spezified task
    // *
    // *
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsCreateMsgQueue (PNIO_UINT32 TaskId)
    {
        OS_ASSERT (OsInitialized == PNIO_TRUE);

        if ((TaskId >= MAXNUM_OF_TASKS)||(TaskId == 0))
        {
            LSA_TRACE_02(TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "Error OsCreateMsgQueue(%d) (MAXNUM_OF_TASKS==%d)\n",
                         TaskId, MAXNUM_OF_TASKS);
            return (PNIO_NOT_OK);   /* not ok */
        }
        /**** create message queue ****/
        cyg_mbox_create(&TaskProp[TaskId].MsgQueue,&TaskProp[TaskId].MsgBox);

        return (LSA_OK);
    }

    // *-----------------------------------------------------------*
    // *  OsDeleteMsgQueue ()
    // *
    // *  Delete a message queue,  assigned to the specified task
    // *
    // *
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsDeleteMsgQueue(PNIO_UINT32 TaskId)
    {
        OS_ASSERT(OsInitialized == PNIO_TRUE);

        if ((TaskId >= MAXNUM_OF_TASKS) || (TaskId == 0))
        {
            LSA_TRACE_02(TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "Error OsDeleteMsgQueue(%d) (MAXNUM_OF_TASKS==%d)\n",
                         TaskId, MAXNUM_OF_TASKS);
            return (PNIO_NOT_OK);   /* not ok */
        }
        /**** delete message queue ****/
        cyg_mbox_delete(TaskProp[TaskId].MsgQueue);

        return (LSA_OK);
    }


    // *-----------------------------------------------------------*
    // *  OsReadMessageBlocked ()
    // *
    // *  reads a message from the message queue of the calling thread
    // *  the function will be blocked if no message is available.
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsReadMessageBlocked (void** ppMessage, PNIO_UINT32 TaskId)
    {
        OS_ASSERT (OsInitialized == PNIO_TRUE);
        OS_ASSERT (ppMessage);

        if ((TaskId >= MAXNUM_OF_TASKS)||(TaskId == 0))
        {
            PNIO_printf(("Error in OsReadMessageBlocked: invalid Task Id()"));
            return (PNIO_NOT_OK);   /* not ok */
        }
        *ppMessage = cyg_mbox_get(TaskProp[TaskId].MsgQueue);

#if LOG_MSG_SND_REC
        TaskProp[TaskId].MsgQueueFillStat = cyg_mbox_peek (TaskProp[TaskId].MsgQueue);
        TaskProp[TaskId].MsgQueueNumOfRec++;    // total number of received messages--
#endif

        return ((*ppMessage) ? PNIO_OK : PNIO_NOT_OK);
    }

    // *-----------------------------------------------------------*
    // *  OsReadMessageBlockedX ()
    // *
    // *  reads a message from the message queue of the calling thread,
    // *  including 2 message pointers.
    // *  the function will be blocked if no message is available.
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsReadMessageBlockedX(void** ppMessage1, void** ppMessage2, PNIO_UINT32 TaskId)
    {
        PNIO_UINT32 Status;
        PNIO_UINT32* pMsg;

        Status = OsReadMessageBlocked((void**)&pMsg, TaskId);
        OS_ASSERT (Status == PNIO_OK);

        if (pMsg)
        {
            if (*(pMsg+2) == OS_MSGX_ID)
            {
                *ppMessage1 = (void*) *(pMsg+0);
                *ppMessage2 = (void*) *(pMsg+1);
            }
            else
            {
                Status = PNIO_NOT_OK;
                LSA_TRACE_02  (TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_ERROR, "not OS_MSGX_ID (0x%x) TskId%d \n", *(pMsg+2), TaskId);
            }

            OsFree (pMsg);
        }
        else
        {
            Status = PNIO_NOT_OK;
            LSA_TRACE_01  (TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_ERROR, "NullPtr received TskId%d\n", TaskId);
        }

        return (Status);
    }

    // *-----------------------------------------------------------*
    // *  OsSendMessageX
    // *
    // *  sends a expanded message to the message queue of the specified
    // *  task. It includes 2 pointers in one message instead
    // *  of one pointer. So we can send a function pointer and a
    // *  requestblock pointer in one one message, to execute a
    // *  function call in a remote task.
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsSendMessageX(PNIO_UINT32 TaskId,
                               void*       pMessage1,
                               void*       pMessage2,
                               PNIO_UINT32 MsgPrio)
    {
        PNIO_UINT32* pMsg;
        PNIO_UINT32  Status;

        OsAlloc((void**)&pMsg, 0, 3 * sizeof(PNIO_UINT32)); // size for pMessage1, pMessage2, Tag-Id

        *(pMsg+0) = (PNIO_UINT32)pMessage1;
        *(pMsg+1) = (PNIO_UINT32)pMessage2;
        *(pMsg+2) = (PNIO_UINT32)OS_MSGX_ID;

        Status = OsSendMessage (TaskId, pMsg, MsgPrio);
        if (Status == PNIO_NOT_OK)
        {
            OsFree (pMsg);
        }
        return (Status);
    }


    // *-----------------------------------------------------------*
    // *  OsSendMessage
    // *
    // *  sends a message to the message queue of the specified
    // *  task
    // *
    // *
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsSendMessage(PNIO_UINT32 TaskId,
                              void*       pMessage,
                              PNIO_UINT32 MsgPrio)
    {
        cyg_bool_t  status;
        PNIO_UINT32 MboxFillStat;

        // ***** return, if mbox is more than 90% full (tryput may not work correctly) ****
        MboxFillStat = cyg_mbox_peek (TaskProp[TaskId].MsgQueue);

        if (MboxFillStat > ((MAXNUM_MSG_ENTRIES * 9)/10))
        { // no trace entry here --> otherwise recursive call of OsSendMessage
            return (PNIO_NOT_OK);
        }

        status = cyg_mbox_tryput(TaskProp[TaskId].MsgQueue,pMessage);
        if (!status)
        {
            OsEnterShort();
            TaskProp[TaskId].MsgQueueErrCnt++;
            OsExitShort();
            LSA_TRACE_02(TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "cyg_mbox_put(%s,%p) failed\n",TaskProp[TaskId].TaskName,pMessage);
            return (PNIO_NOT_OK);
        }


        // **** if ok, refresh queuestate ***
#if LOG_MSG_SND_REC
        //M_INTERR_DISABLE;
        TaskProp[TaskId].MsgQueueFillStat = MboxFillStat + 1;

        TaskProp[TaskId].MsgQueueNumOfSnd++;    // total number of sent messages++
        if (TaskProp[TaskId].MsgQueueFillStat >= TaskProp[TaskId].MsgQueueMaxFillStat) // save max fillstate value
            TaskProp[TaskId].MsgQueueMaxFillStat = TaskProp[TaskId].MsgQueueFillStat;
        if (TaskProp[TaskId].MsgQueueFillStat >= MsqQueueMaxFillStatAll) // save max fillstate value
            MsqQueueMaxFillStatAll = TaskProp[TaskId].MsgQueueFillStat;
        //M_INTERR_ENABLE;
#endif

        return (PNIO_OK); //ok
    }


#if (MAXNUM_OF_BIN_SEMAPH != 0)

        // *-----------------------------------------------------------*
        // *  OsCreateSemB()
        // *  Creates a binary mutex (dynamically allocatable)
        // *  and initialize data for mutex management.
        // *  Semaphore must be  occupied after creation.
        // *-----------------------------------------------------------*
        PNIO_UINT32 OsCreateSemB(PNIO_UINT32 SemId)
        {
            // **** create semaphore ***
            cyg_semaphore_init(&Semaph[SemId].Mutex, 1);

            // *** occupy semaphore ***
            if (!cyg_semaphore_wait(&Semaph[SemId].Mutex))
            {
                LSA_TRACE_01 (TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "OsCreateSemB: error bin semaphore %d\n", SemId);
                return (PNIO_NOT_OK);
            }

            // **** mark as free, i.e. not allocated ***
            Semaph[SemId].Allocated = PNIO_FALSE;
            return PNIO_OK;
        }

        // *-----------------------------------------------------------*
        // *  OsAllocSemB()
        // *  Allocates a binary semaphore. The semaphore must be
        // *  occupied at initial state.
        // *-----------------------------------------------------------*
        PNIO_UINT32 OsAllocSemB (PNIO_UINT32* pSemId)
        {
            int i;
            OS_ASSERT(OsInitialized == PNIO_TRUE);

            OsIntDisable();
            for (i = 0; i < MAXNUM_OF_BIN_SEMAPH; i++)
            {
                if (Semaph[i].Allocated == PNIO_FALSE)
                {
                    Semaph[i].Allocated = PNIO_TRUE;
                    OsIntEnable();
                    *pSemId = i;
                    return (PNIO_OK);
                }
            }

            OsIntEnable();
            LSA_TRACE_00(TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "no more binary semaphore ressources\n");
            return (PNIO_NOT_OK);
        }

        // *-----------------------------------------------------------*
        // *  OsTakeSemB()
        // *  Occupies a binary semaphore. If semaphore is already
        // *  occupied, the function is blocking.
         // *-----------------------------------------------------------*
        PNIO_UINT32 OsTakeSemB(PNIO_UINT32 SemId)
        {
            OS_ASSERT (OsInitialized == PNIO_TRUE);
            if (!cyg_semaphore_wait(&Semaph[SemId].Mutex))
            {
                LSA_TRACE_01  (TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "error bin semaphore %d\n", SemId);
                return (PNIO_NOT_OK);
            }
            else
            {
                return (PNIO_OK);
            }
        }

        // *-----------------------------------------------------------*
        // *  OsGiveSemB()
        // *  Deoccupies a binary semaphore, that has been occupied before
        // *  with function OsTakeSemaph
        // *  This function may be called from task- or interrupt context.
        // *-----------------------------------------------------------*
        PNIO_UINT32 OsGiveSemB(PNIO_UINT32 SemId)
        {
            OS_ASSERT (OsInitialized == PNIO_TRUE);
            if (Semaph[SemId].Mutex.count == 0)
            {
                cyg_semaphore_post(&Semaph[SemId].Mutex);
            }
            return (PNIO_OK);
        }

        // *-----------------------------------------------------------*
        // *  OsFreeSemB()
        // *  Frees a binary semaphore, that has been allocated before.
        // *-----------------------------------------------------------*
        PNIO_UINT32 OsFreeSemB(PNIO_UINT32 SemId)
        {
            OS_ASSERT (OsInitialized == PNIO_TRUE);
            if (SemId >= MAXNUM_OF_BIN_SEMAPH)
            {
                LSA_TRACE_01  (TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_ERROR, "Error OsFreeSemaph %d\n", SemId);
                return (PNIO_NOT_OK);
            }
            Semaph[SemId].Allocated = PNIO_FALSE;
            return (PNIO_OK);
        }
    #endif

    // *-----------------------------------------------------------*
    // *  OsEnterShort()
    // *  occpupy critical section, but short and fast.
    // *  Can be implemented e.g. by scheduler lock.
     // *-----------------------------------------------------------*
    void OsEnterShort(void)
    {
        cyg_scheduler_lock();
    }

    // *-----------------------------------------------------------*
    // *  OsFreeShort()
    // *  free short critical
    // *-----------------------------------------------------------*
    void OsExitShort(void)
    {
        cyg_scheduler_unlock();
    }

    // *-----------------------------------------------------------*
    // *  OsCreateMutex ()
    // *
    // *  Create mutex with the given mutex ID
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsCreateMutex(PNIO_UINT16 MutexId)
    {
        PNIO_UINT32 ret_value = PNIO_NOT_OK;

        if ((NamedMutex[MutexId].isUsed == PNIO_FALSE) && (MutexId < MAXNUM_OF_NAMED_MUTEXES))
        {
            cyg_mutex_init(&(NamedMutex[MutexId].mutex));
            cyg_mutex_set_protocol(&NamedMutex[MutexId].mutex, CYG_MUTEX_INHERIT);

            NamedMutex[MutexId].count  = 0;
            NamedMutex[MutexId].isUsed = PNIO_TRUE;
            ret_value = PNIO_OK;
        }
        else
        {
            LSA_TRACE_03(TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL,
                         "Error in OsCreateMutex(%d): NamedMutex[MutexId].isUsed=%d, MAXNUM_OF_NAMED_MUTEXES=%d\n",
                         MutexId, NamedMutex[MutexId].isUsed, MAXNUM_OF_NAMED_MUTEXES);
        }

        return ret_value;
    }


    // *-----------------------------------------------------------*
    // *  OsAllocReentranceX ()
    // *
    // *  Create Reentrance mutex and return index of mutex
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsAllocReentranceX(PNIO_UINT16* lock_handle_ptr)
    {
        PNIO_UINT32 ret_value = LSA_RET_ERR_RESOURCE; 
        PNIO_UINT16 i = 0;

        for (i = OS_DEVKITS_MUTEX_MAX_COUNT; i < MAXNUM_OF_NAMED_MUTEXES; i++)
        {
            if (NamedMutex[i].isUsed == PNIO_FALSE)
            {
                ret_value = OsCreateMutex(i);
                *lock_handle_ptr = i;
                return ret_value; 
            }
        }
        return ret_value;
    }


    // *-----------------------------------------------------------*
    // *  OsFreeReentranceX ()
    // *
    // *  Free Reentrance mutex from its index
    // *-----------------------------------------------------------*    
    void OsFreeReentranceX(PNIO_UINT32 MutexId)
    {
        OS_ASSERT(MutexId < MAXNUM_OF_NAMED_MUTEXES);
        OS_ASSERT(NamedMutex[MutexId].isUsed == PNIO_TRUE);

        cyg_mutex_destroy(&(NamedMutex[MutexId].mutex));
        NamedMutex[MutexId].count = 0;
        NamedMutex[MutexId].isUsed = PNIO_FALSE;
    }

    // *-----------------------------------------------------------*
    // *  OsEnterX ()
    // *
    // *  MutexId = 0....(MAXNUM_OF_NAMED_MUTEXES - 1)
    // *-----------------------------------------------------------*
    void OsEnterX(PNIO_UINT32 MutexId)
    {
        OS_ASSERT(OsInitialized == PNIO_TRUE);
        OS_ASSERT(MutexId < MAXNUM_OF_NAMED_MUTEXES);

        if (!cyg_mutex_trylock(&(NamedMutex[MutexId].mutex)))
        {
            if (NamedMutex[MutexId].mutex.owner->unique_id != cyg_thread_get_id(cyg_thread_self()))
            {
                cyg_mutex_lock(&NamedMutex[MutexId].mutex);
            }
        }
        ++(NamedMutex[MutexId].count);
    }

    // *-----------------------------------------------------------*
    // *  OsExitX()
    // *
    // *
    // *-----------------------------------------------------------*
    void OsExitX(PNIO_UINT32 MutexId)
    {
        // PNIO_UINT32 myid = OsGetThreadId();
        OS_ASSERT(OsInitialized == PNIO_TRUE);
        OS_ASSERT(MutexId < MAXNUM_OF_NAMED_MUTEXES)

        if (NamedMutex[MutexId].mutex.owner->unique_id == cyg_thread_get_id(cyg_thread_self()))
        {
            --(NamedMutex[MutexId].count);
        }
        else
        {
            cyg_thread_info info;
            cyg_thread_get_info(cyg_thread_self(),cyg_thread_get_id(cyg_thread_self()),&info);
            LSA_TRACE_04(TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL,
                         "OsExitX mutex freed from task %s/#%d,\nbut owned by %d/#%d",
                         info.name,
                         info.id,
                         NamedMutex[MutexId].mutex.owner->name,
                         NamedMutex[MutexId].mutex.owner->unique_id);

            OsEnterShort();
            while(1)
            {}
        }

        if (0 == NamedMutex[MutexId].count)
        {
            cyg_mutex_unlock(&NamedMutex[MutexId].mutex);
        }
    }

    // *-----------------------------------------------------------*
    // *  TxTimerCallback()
    // *
    // *  adaption of the OS timer -calling interface to LSA Callback-
    // *  interface
    // *-----------------------------------------------------------*
    static void TxTimerCallback(cyg_handle_t alarm, cyg_addrword_t timer_id)
    {
        OS_ASSERT (OsInitialized == PNIO_TRUE);
        // ** set entry in circular buffer for debugging
        if (timer_id >= PCPNIO_TIMER_MAX)
        {
            LSA_TRACE_01(TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "Maxnum of Timers exceeded, ID%d\n", timer_id);
            return;
        }

        if (OsTimer[timer_id].timer_typ == LSA_TIMER_TYPE_ONE_SHOT)
        {
            OsTimer[timer_id].Running = PNIO_FALSE;    // set timer state: not running
            //cyg_alarm_delete(OsTimer[timer_id].TxTimer);
        }
        else
        {
            /* ecos supports cyclic timers*/
        }

#if DEBUG_TIMCNT
        OsTimCounter[timer_id]++;
#endif

        //***** call user defined callback function *****
        OsSendMessage(TskId_OsTimer, (void*) timer_id, OS_MBX_PRIO_NORM);
    }


    /*=============================================================================
     * function name:  OsAllocTimer
     *
     * function:       allocate a timer
     *
     * parameters:     PNIO_UINT16  ...  *  timer_id:    return value: pointer to id of timer
     *
     *                 PNIO_UINT16          timer_type:  LSA_TIMER_TYPE_ONE_SHOT or
     *                                                   LSA_TIMER_TYPE_CYCLIC
     *                 PNIO_UINT16          time_base:   LSA_TIME_BASE_1MS,
     *                                                   LSA_TIME_BASE_10MS,
     *                                                   LSA_TIME_BASE_100MS,
     *                                                   LSA_TIME_BASE_1S,
     *
     *                   callback_timeout                wird bei Ablauf des Timers aufgerufen
     *
     *
     * return value:   LSA_RET_OK            timer has been allocated
     *                 LSA_RET_ERR_NO_TIMER  no timer has been allocated
     *===========================================================================*/
    PNIO_UINT32  OsAllocTimer(PNIO_UINT16* timer_id,
                              PNIO_UINT16  timer_typ,
                              PNIO_UINT16  timer_base,
                              PNIO_VOID*   callback_timeout)
    {
        PNIO_UINT32 Index;
        PNIO_UINT32 i;
        OS_ASSERT(OsInitialized == PNIO_TRUE);

        // *---------------------------------------------------------------
        // plausibility check of the function parameters
        // *---------------------------------------------------------------
        if (! (timer_base == LSA_TIME_BASE_1MS  ||  timer_base == LSA_TIME_BASE_10MS ||
               timer_base == LSA_TIME_BASE_100MS  ||  timer_base == LSA_TIME_BASE_1S))
        // not supported||  timer_base == LSA_TIME_BASE_10S  ||  timer_base == LSA_TIME_BASE_100S))
        {
            LSA_TRACE_00  (TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "pcpnio_alloc_timer:!timer_base\n");
            return LSA_RET_ERR_PARAM;
        }

        if (! (timer_typ == LSA_TIMER_TYPE_ONE_SHOT || timer_typ == LSA_TIMER_TYPE_CYCLIC))
        {
            LSA_TRACE_00  (TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "pcpnio_alloc_timer:!timer_typ\n");
            return LSA_RET_ERR_PARAM;
        }

        // *** search for free memory block for timer ***
        Index = INVALID;
        for (i = 0; i < PCPNIO_TIMER_MAX; i++)
        {
            OsEnterX(OS_MUTEX_DEFAULT);
            if (OsTimer[i].Occupied == PNIO_FALSE)
            {
                OsTimer[i].Occupied = PNIO_TRUE;
                /* OsTimer[i].Used  = PNIO_TRUE;*/
                Index = i;
                OsExitX(OS_MUTEX_DEFAULT);
                break;
            }
            OsExitX(OS_MUTEX_DEFAULT);
        }

        if (Index == INVALID)
        {
            LSA_TRACE_00(TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "Error in OsAllocTimer\n");
            return (LSA_RET_ERR_PARAM);
        }

        switch (timer_base)
        {
            case LSA_TIME_BASE_1MS:     OsTimer[Index].timer_base = 1;      break;
            case LSA_TIME_BASE_10MS:    OsTimer[Index].timer_base = 10;     break;
            case LSA_TIME_BASE_100MS:   OsTimer[Index].timer_base = 100;    break;
            case LSA_TIME_BASE_1S:      OsTimer[Index].timer_base = 1000;   break;
            case LSA_TIME_BASE_10S:     OsTimer[Index].timer_base = 10000;  break;
            case LSA_TIME_BASE_100S:    OsTimer[Index].timer_base = 100000; break;
            default:
            {
                LSA_TRACE_00(TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "OsAllocTimer: Invalid timer base!\n");
                return (LSA_RET_ERR_PARAM);
            }
        }

        /* use OsTimer[Index].TxTimer temporary to hold RTC-timer handle*/
        cyg_clock_to_counter(cyg_real_time_clock(),&OsTimer[Index].TxTimer);
        cyg_alarm_create(OsTimer[Index].TxTimer,TxTimerCallback,Index,&OsTimer[Index].TxTimer,&OsTimer[Index].Alarm);

        OsTimer[Index].callback_timeout = callback_timeout; // save LSA callback function for timer-expiration
        OsTimer[Index].timer_typ        = timer_typ;        // save type (Cyclic/OneShot)
        *timer_id = Index;  // return timer id, value = 0...PCPNIO_TIMER_MAX-1
        return (LSA_OK);
    }


    /*=============================================================================
     * function name:  OsStartTimer
     *
     * function:       start a timer
     *
     * parameters:
     *                 PNIO_UINT16          timer_id:  id of timer
     *                 LSA_USER_ID_TYPE     user_id:   id of prefix
     *                 PNIO_UINT16          delay:     running time in msec
     *
     * return value:   LSA_RET_OK                  timer has been started
     *                 LSA_RET_OK_TIMER_RESTARTED  timer has been
     *                 restarted
     *                 LSA_RET_ERR_PARAM           timer hasn't been started
     *                                             because of wrong timer-id
     *                                             After the expiration of the running
     *                                             time system will call edd_timeout().

     *===========================================================================*/
    PNIO_UINT32 OsStartTimer(PNIO_UINT16 timer_id,
                             PNIO_UINT32 user_id,
                             PNIO_UINT16 delay)
    {
        OS_ASSERT (OsInitialized == PNIO_TRUE);

        OsTimer[timer_id].TimeVal_ms    = OsTimer[timer_id].timer_base*delay;
        OsTimer[timer_id].TimeVal_Ticks = MS_TO_TICKS(OsTimer[timer_id].TimeVal_ms);

        // **** save user_id ***
        OsTimer[timer_id].user_id = user_id;
        // **** activate timer ***
        OsTimer[timer_id].Running = PNIO_TRUE;                        // set timer state: running
        cyg_alarm_initialize(OsTimer[timer_id].TxTimer, cyg_current_time()+OsTimer[timer_id].TimeVal_Ticks, 
                             OsTimer[timer_id].timer_typ == LSA_TIMER_TYPE_CYCLIC ? OsTimer[timer_id].TimeVal_Ticks : 0);

        return (LSA_OK);
    }


    /*=============================================================================
     * function name:  OsStopTimer
     *
     * function:       stops an active timer
     *
     *
     *
     * return value:   LSA_RET_OK                    timer has been stopped
     *                   LSA_RET_OK_TIMER_NOT_RUNNING  timer was not running
     *                   LSA_RET_ERR_PARAM             error occured
     *
     *===========================================================================*/
    PNIO_UINT32 OsStopTimer(PNIO_UINT16 timer_id)
    {
        OS_ASSERT (OsInitialized == PNIO_TRUE);
        if (OsTimer[timer_id].Running == PNIO_FALSE)        // timer is not running, return
            return (LSA_RET_OK_TIMER_NOT_RUNNING);

        cyg_alarm_disable(OsTimer[timer_id].TxTimer);

        OsTimer[timer_id].Running = PNIO_FALSE;
        return (LSA_OK);
    }

    /**
     * @brief resets timer value
     *
     * @param[in]   timer_id        id of timer
     *
     * @return  PNIO_UINT32     LSA_RET_OK                    timer has been reset
     *                          LSA_RET_ERR_PARAM             error occured
     *
     *    Reset can be performed only over timer, which is currently not running.
     *    Thus, this function should be preceded by timer stop, or it should preceed timer start.
     */
    PNIO_UINT32 OsResetTimer(PNIO_UINT16 timer_id)
    {
        /* Empty for this platform - not needed */
        return 0;
    }


    /*=============================================================================
     * function name:  OsFreeTimer
     *
     * function:       free a timer
     *
     * parameters:     PNIO_UINT16  ...  *  ret_val_ptr:
     *                return value: LSA_RET_OK                    timer has been
     *                                                            deallocated
     *                              LSA_RET_ERR_TIMER_IS_RUNNING  because timer is
     *                                                            running timer has
     *                                                            not been
     *                                                            deallocated
     *                              LSA_RET_ERR_PARAM             no deallocation
     *                                                            because of wrong
     *                                                            timer-id
     *                 PNIO_UINT16          timer_id:  id of timer
     *
     * return value:   LSA_VOID
     *===========================================================================*/
    PNIO_UINT32   OsFreeTimer (    PNIO_UINT16 timer_id)
    {

        OS_ASSERT (OsInitialized == PNIO_TRUE);
        if (OsTimer[timer_id].Running == PNIO_TRUE)
            return (LSA_RET_ERR_TIMER_IS_RUNNING);

        cyg_alarm_delete(OsTimer[timer_id].TxTimer);

        OsTimer[timer_id].Occupied = PNIO_FALSE;

        return (LSA_OK);
    }



    // *----------------------------- Task Handling -----------------------------------------------*

    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /*+                                                                       +*/
    /*+  function name         :   OsCreateThread                             +*/
    /*+  input parameter       :   ...                                        +*/
    /*+  return parameter      :   PNIO_OK/PNIO_NOT_OK                        +*/
    /*+                                                                       +*/
    /*+-----------------------------------------------------------------------+*/
    /*+                                                                       +*/
    /*+  description           :   Creates the thread according to the given  +*/
    /*+                            specifications.                            +*/
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    PNIO_UINT32 OsCreateThread(PNIO_VOID          (*pTaskEntry)(PNIO_VOID), // [in] pointer to task handler
                               PNIO_VOID_PTR_TYPE TaskEntryData,            // [in] argument to be passed to thread func
                               PNIO_UINT8*        TaskName,                 // Task name for Debugging
                               PNIO_UINT32        TaskPrio,                 // [in] task priority
                               PNIO_UINT32        TaskStackSize,            // [in] stack size of task
                               PNIO_UINT32*       pTaskId)                  // [out] pointer to Task ID
    {
        PNIO_INT32 TaskIndex = INVALID;
        PNIO_UINT8 i;

        OS_ASSERT(OsInitialized == PNIO_TRUE);
        for ( i = 1; i < MAXNUM_OF_TASKS; i++) // we dont use index 0 here
        {
            OsEnterX(OS_MUTEX_DEFAULT);
            if (TaskProp[i].Occupied == PNIO_FALSE) // is it a free entry ??
            { // yes, now occupy it
                TaskProp[i].Occupied = PNIO_TRUE;   // occupy entry
                TaskIndex = i;
                OsExitX(OS_MUTEX_DEFAULT);
                break;
            }
            OsExitX(OS_MUTEX_DEFAULT);
        }

        if (TaskIndex == INVALID)
        { // ** error: no more task can be created **
            LSA_TRACE_00(TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "Error in OsStartTask()\n");
            return (PNIO_NOT_OK);
        }

        // create and start the task
        TaskProp[TaskIndex].StackSize  = TaskStackSize;
        TaskProp[TaskIndex].pTaskStack = OsReserveStackForThread(TaskStackSize);

        OsMemCpy((void*)&(TaskProp[TaskIndex].TaskName[0]), (void*)&TaskName[0], MAXSIZE_TASKNAME);

        cyg_thread_create(TaskPrio,                             /* scheduling info (priority) */
                          (cyg_thread_entry_t*)pTaskEntry,      /* thread entry point         */
                          (cyg_addrword_t)TaskEntryData,        /* entry point argument       */
                          (char*)TaskProp[TaskIndex].TaskName,  /* name of thread             */
                          TaskProp[TaskIndex].pTaskStack,       /* pointer to stack base      */
                          TaskProp[TaskIndex].StackSize,        /* size of stack in bytes     */
                          &TaskProp[TaskIndex].ThreadId,        /* returned thread handle     */
                          &TaskProp[TaskIndex].Thread);         /* space to store thread data */

        *pTaskId = TaskIndex;   // take the index as an OS-independent task ID

        return (PNIO_OK);
    }


    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /*+                                                                       +*/
    /*+  function name         :    OsReserveStackForThread                   +*/
    /*+  input parameter       :    StackSize -  stack size of thread         +*/
    /*+  output parameter      :    pStackBase - stack base pointer of thread +*/
    /*+                                                                       +*/
    /*+-----------------------------------------------------------------------+*/
    /*+                                                                       +*/
    /*+  description           :    Reserves the given stack size from the    +*/
    /*+                             TaskStack memory and return the base      +*/
    /*+                             pointer of the reserved stack             +*/
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    PNIO_UINT8* OsReserveStackForThread(PNIO_UINT32 StackSize)
    {
        PNIO_UINT8* pStackBase = 0;

        OsEnterX(OS_MUTEX_DEFAULT);

        // Null check of TaskStack available memory address pointer
        OS_ASSERT(g_pTaskStackMem != LSA_NULL);
        // Check if there is enough stack space for the thread
        OS_ASSERT(&TaskStack[OS_TOTAL_TASK_STACKSIZE - 1] >= (g_pTaskStackMem + (sizeof(PNIO_UINT8) * StackSize)));

        pStackBase      = g_pTaskStackMem;
        g_pTaskStackMem = g_pTaskStackMem + (sizeof(PNIO_UINT8) * StackSize);

        OsExitX(OS_MUTEX_DEFAULT);

        return pStackBase;
    }


    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /*+                                                                       +*/
    /*+  function name         :    OsStartThread                             +*/
    /*+  input parameter       :    ThreadId                                  +*/
    /*+  output parameter      :    ----                                      +*/
    /*+                                                                       +*/
    /*+-----------------------------------------------------------------------+*/
    /*+                                                                       +*/
    /*+  description           :   starts a thread, that was created with     +*/
    /*+                            OsCreateThread                             +*/
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    PNIO_UINT32 OsStartThread ( PNIO_UINT32 ThreadId)
    {
        OS_ASSERT (OsInitialized == PNIO_TRUE);

        cyg_thread_resume(TaskProp[ThreadId].ThreadId);

        return (PNIO_OK);
    }

    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /*+                                                                       +*/
    /*+  function name         :    OsExitThread                              +*/
    /*+  input parameter       :    ----                                      +*/
    /*+  output parameter      :    ----                                      +*/
    /*+                                                                       +*/
    /*+-----------------------------------------------------------------------+*/
    /*+                                                                       +*/
    /*+  description           :   exits a thread.                            +*/
    /*+                                                                       +*/
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    PNIO_UINT32 OsExitThread()
    {
        cyg_thread_exit();

        return (PNIO_OK);
    }

    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /*+                                                                       +*/
    /*+  function name         :    OsJoinThread                              +*/
    /*+  input parameter       :    ThreadId                                  +*/
    /*+  output parameter      :    ----                                      +*/
    /*+                                                                       +*/
    /*+-----------------------------------------------------------------------+*/
    /*+                                                                       +*/
    /*+  description           :                                              +*/
    /*+                                                                       +*/
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    PNIO_UINT32 OsJoinThread(PNIO_UINT32 ThreadId)
    {
        OS_ASSERT(OsInitialized == PNIO_TRUE);

        while (cyg_thread_find(TaskProp[ThreadId].ThreadId))
        {
            OsWait_ms(1);
        }

        return (PNIO_OK);
    }


    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /*+                                                                       +*/
    /*+  function name         :    OsKillOtherThreads                        +*/
    /*+  input parameter       :    ----                                      +*/
    /*+  output parameter      :    ----                                      +*/
    /*+                                                                       +*/
    /*+-----------------------------------------------------------------------+*/
    /*+                                                                       +*/
    /*+  description           :   Kills all non-critical threads other       +*/
    /*+                            than itself                                +*/
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    PNIO_VOID OsKillOtherThreads(PNIO_VOID)
    {
        cyg_handle_t thread_handle = 0;
        cyg_uint16 thread_id = 0;

        cyg_thread* current_thread_ptr = (cyg_thread*)cyg_thread_self();
        cyg_uint16 current_thread_id = current_thread_ptr->unique_id;
        while (cyg_thread_get_next(&thread_handle, &thread_id))
        {
            // Kills other threads which do not affect the successful fatal execution
            if (thread_id == current_thread_id      ||
                thread_id < 4 /*Critical Threads*/ )
            {
                continue;
            }

            cyg_thread_kill(thread_handle);
        }
    }


    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /*+                                                                       +*/
    /*+  function name         :    OsWaitOnEnable                           +*/
    /*+  input parameter       :    ThreadId                                  +*/
    /*+  output parameter      :    ----                                      +*/
    /*+                                                                       +*/
    /*+-----------------------------------------------------------------------+*/
    /*+                                                                       +*/
    /*+  description           :   waits until the task is enabled by the     +*/
    /*+                            creating task.                             +*/
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    PNIO_UINT32 OsWaitOnEnable ()
    {
        /*lint -esym(522,OsWaitOnEnable) lacks side-effects */
        return (LSA_OK);
    }


    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /*+                                                                       +*/
    /*+  function name         :    OsGetThreadId                             +*/
    /*+  input parameter       :    --                                        +*/
    /*+  output parameter      :    ID of this thread                         +*/
    /*+                                                                       +*/
    /*+-----------------------------------------------------------------------+*/
    /*+                                                                       +*/
    /*+  description           :    returns the ID of the running thread      +*/
    /*+                                                                       +*/
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    PNIO_UINT32  OsGetThreadId (void)
    {
        PNIO_INT32  i;
        PNIO_INT32  ThisThreadId;

        OS_ASSERT (OsInitialized == PNIO_TRUE);
        ThisThreadId = cyg_thread_self();   // must be called from task state only, not from system or interrupt state
        for (i = 1; i < MAXNUM_OF_TASKS; i++)
        {
            if (TaskProp[i].ThreadId == ThisThreadId)
            {
                return (i);
            }
        }
        //   task not found, then return ID of a error task with message queue.
        //   if a message will be sent to this task, it will be received by the error task
        //   and a error message will be generated.
        //   a "unregistered" thread ID may be happen if a OS-Task (e.g. LSA-Timeout)
        //   calls a Callbackfunction, which sends a message and is executed in the context
        //   of the Timer Task.
        LSA_TRACE_00(TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "invalid task not found\n");
        return (INVALID_TASK_ID);
    }


    // *-----------------------------------------------------------*
    // *  OsSetThreadPrio()
    // *  changes the priority of the specified thread
    // *
    // *  Input:  ThreadId        ID of the thread, which priority is to change
    // *          NewThreadPrio   new thread priority
    // *  output: return          LSA_OK, LSA_NOT_OK
    // *-----------------------------------------------------------*
    PNIO_UINT32 OsSetThreadPrio(PNIO_UINT32 ThreadId,
                                PNIO_UINT32 NewThreadPrio)
    {
        OS_ASSERT (OsInitialized == PNIO_TRUE);
        cyg_thread_set_priority(TaskProp[ThreadId].ThreadId,NewThreadPrio);
        return (LSA_OK);
    }

    // *----------------------------------------------------------------*
    // *
    // *  PNIO_vsprintf (void)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  printf on standard console
    // *
    // *  Input:    like the  printf() function
    // *  Output:   ----
    // *
    // *----------------------------------------------------------------*
    PNIO_UINT32 PNIO_vsprintf(PNIO_UINT8* pBuf, PNIO_CHAR* fmt, PNIO_va_list argptr)
    {
        return (vsprintf ((char*)pBuf, fmt, argptr));
    }

    // *----------------------------------------------------------------*
    // *
    // *  PNIO_sprintf (void)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  printf on standard console
    // *
    // *  Input:    like the  printf() function
    // *  Output:   ----
    // *
    // *----------------------------------------------------------------*
    PNIO_UINT32 PNIO_sprintf(PNIO_UINT8* pBuf, PNIO_CHAR* fmt, ...)
    {
        PNIO_UINT32 Cnt = 0;
        va_list argptr = {0};
        va_start(argptr, fmt);

        Cnt = PNIO_vsprintf(pBuf, fmt, argptr);
        va_end (argptr);
        return Cnt;
    }

    // *----------------------------------------------------------------*
    // *
    // *  PNIO_snprintf (void)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  printf on buffer
    // *
    // *----------------------------------------------------------------*
    PNIO_UINT32 PNIO_snprintf(PNIO_UINT8* str, PNIO_UINT32 count, PNIO_UINT8* fmt, ...)
    {
        va_list argptr = {0};
        LSA_INT ret;
        LSA_UNUSED_ARG(count);
        va_start(argptr, (char*)(fmt));
        ret = PNIO_vsprintf(str, (char*)(fmt), argptr);
        if (strlen((const char *)str) >= count)
            LSA_TRACE_00(TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "error PNIO_snprintf\n");

        va_end(argptr);
        return ret;
    }

#if (RS232_PRINT_POSTTHREAD)

    // *----------------------------------------------------------------*
    // *   TracePostTask (PNIO_VOID)
    // *----------------------------------------------------------------*
    // *  trace posttask, to print messages on lower task level
    // *
    // *  input :  ---
    // *  out   :  ---
    // *----------------------------------------------------------------*
    static PNIO_VOID PrintRS232PostTask (PNIO_VOID)
    {

        PNIO_CHAR* pMsg = NULL;            // pointer to Requestblock
        LSA_UINT32 taskID;

        // Wait until own TaskID has been saved by the father process
        OsWaitOnEnable(); // must be first call in every task
        taskID = OsGetThreadId();

        // *-----------------------------------------------------------
        // * loop forever
        // *-----------------------------------------------------------
        while (1)
        {
            OsReadMessageBlocked ((void**)&pMsg, taskID);

            if (pMsg == 0)
            {
                printf ("ERROR: PrintRS232PostTask() pMsg=0\n");
            }
            else
            {
                printf ("%s", pMsg);
                fflush(stdout);
                OsFree (pMsg);
                // remove message from queue and re-enable console print if possible

                M_INTERR_DISABLE;
                if (!PrintPostQueueSize)
                {
                    printf ("ERROR: PrintRS232PostTask() PrintPostQueueSize Underflow.\n");
                }

                PrintPostQueueSize--;

                if (!PrintPostQueueEnable && (PrintPostQueueSize < PRINT_POST_QUEUE_ENABLE))
                {
                    PrintPostQueueEnable = PNIO_TRUE;
                }
                M_INTERR_ENABLE;
            }
        }
    }
#endif


    // *----------------------------------------------------------------*
    // *   OsPrint
    // *----------------------------------------------------------------*
    // *  print to serial output
    // *
    // *  input :
    // *  out   :  ---
    // *----------------------------------------------------------------*
    static void OsPrint(PNIO_CHAR* fmt, PNIO_va_list argptr)
    {
#if (RS232_PRINT_POSTTHREAD)

        PNIO_UINT8  TmpBuf[RS232_BUF_SIZE];
        PNIO_UINT32 Status;
        PNIO_UINT32 Len;
        PNIO_UINT8* pConsBuf = NULL;    // send to print  in lower task context

        if (PrintPostQueueEnable)
        {
            OsMemSet(&TmpBuf[0], ' ', RS232_BUF_SIZE);

            if (PrintPostQueueSize < PRINT_POST_QUEUE_DISABLE)
            {
                Len = PNIO_vsprintf(&TmpBuf[0], fmt, argptr);
                M_INTERR_DISABLE;
                PrintPostQueueSize++;
                M_INTERR_ENABLE;
            }
            else
            {
                Len = PNIO_sprintf(&TmpBuf[0], "\nWARNING: OsPrint() Queue Limit Reached => Console Print DISABLED. \n\n");
                M_INTERR_DISABLE;
                PrintPostQueueSize++;
                PrintPostQueueEnable = PNIO_FALSE;
                M_INTERR_ENABLE;
            }

            if (Len > RS232_BUF_SIZE)
            {
                printf("WARNING: OsPrint() message to long (%d bytes) \n", (int) Len);
                Len = RS232_BUF_SIZE;
            }

            Status = OsAllocF((void**) &pConsBuf, Len + 1);
            if (Status == PNIO_OK)
            {
                OsMemCpy (pConsBuf, &TmpBuf[0], Len);
                *(pConsBuf+Len) = 0;    // terminate with zero
                OsSendMessage(TskId_PrintPost, (void*)pConsBuf, OS_MBX_PRIO_NORM);
            }
            else
            {
                printf("ERROR: OsPrint() Memory Allocation of %d Bytes Failed \n", (int) Len);
            }
        }
#else
        vprintf(fmt, argptr);
#endif
    }


    // *----------------------------------------------------------------*
    // *
    // *  PNIO_log10 (double)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  logarithm on basis 10
    // *
    // *  Input:    values
    // *  Output:   log10 of value
    // *
    // *----------------------------------------------------------------*
    double PNIO_log10(double value)
    {
       return log10(value);
    }


    // *----------------------------------------------------------------*
    // *
    // *  PNIO_pow (double,double)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  returns base raised to the exp power (base^exp)
    // *
    // *  Input:    base
    // *  Input:    exponent
    // *  Output:   pow(base,exponent)
    // *
    // *----------------------------------------------------------------*
    double PNIO_pow(double base, double exponent)
    {
        return pow(base, exponent);
    }

    // *----------------------------------------------------------------*
    // *
    // *  PNIO_ConsolePrintf (void)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  printf on standard console
    // *  indenpendent from log level and log destination
    // *
    // *  Input:    like the  printf() function
    // *  Output:   ----
    // *
    // *----------------------------------------------------------------*
    void PNIO_ConsolePrintf(PNIO_CHAR* fmt, ...)
    {
        va_list argptr = {0};
        va_start(argptr, fmt);
        OsPrint (fmt, argptr);
        va_end(argptr);
    }

    // *----------------------------------------------------------------*
    // *
    // *  PNIO_ConsoleLog (void)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  printf on standard console
    // *  if PnioLogDest is Console (1)
    // *
    // *  Input:    like the  printf() function
    // *  Output:   ----
    // *
    // *----------------------------------------------------------------*
    void PNIO_ConsoleLog(PNIO_CHAR* fmt, ...)
    {
        if (PnioLogDest == 1)
        {
            va_list argptr = {0};
            va_start(argptr, fmt);
            OsPrint (fmt, argptr);
            va_end(argptr);
        }
    }


    // *----------------------------------------------------------------*
    // *
    // *  PNIO_printf (void)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  printf on standard console or memory trace buffer
    // *
    // *  Input:    like the  printf() function
    // *  Output:   ----
    // *
    // *----------------------------------------------------------------*
    void PNIO_printf(PNIO_CHAR* fmt, ...)
    {
        switch (PnioLogDest)
        {
            case 0:
                break;
            case 1:
            {
                va_list argptr = {0};
                va_start(argptr, fmt);
                OsPrint (fmt, argptr);
                va_end(argptr);
            }
            break;
            case 2:
            {
                va_list argptr = {0};
                va_start(argptr, fmt);
                OsPrint (fmt, argptr);
                //TRACE_PRINT (fmt, (PNIO_va_list)argptr);
                va_end(argptr);
            }
            break;
            default:
            {

            }
        }
    }


    // *----------------------------------------------------------------*
    // *
    // *  PNIO_TrcPrintf (void)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  printf of debug logging
    // *
    // *  Input:    like the  printf() function
    // *  Output:   ----
    // *
    // *----------------------------------------------------------------*
    void PNIO_TrcPrintf(PNIO_CHAR* fmt, ...)
    {
        switch (PnioLogDest)
        {
            case 0:
                break;
            case 1: // **** print on stdout ****
            {
                va_list argptr = {0};
                va_start(argptr, fmt);
                OsPrint (fmt, argptr);
                va_end (argptr);
            }
            break;
            case 2: // **** print into tracebuffer memory ****
            {
                va_list argptr = {0};
                va_start(argptr, fmt);
                OsPrint (fmt, argptr);
                //TRACE_PRINT (fmt, argptr);
                va_end (argptr);
            }
            break;
            default:
                break;

        }
    }


    // *----------------------------------------------------------------*
    // *
    // *  PNIO_FatalError(LSA_FATAL_ERROR_TYPE*)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  generic pnio fatal error function
    // *
    // *  Input:    lsa fatal error pointer type
    // *  Output:   ----
    // *
    // *----------------------------------------------------------------*
    void PNIO_FatalError(LSA_FATAL_ERROR_TYPE* pLsaErr)
    {
        OS_ASSERT(pLsaErr != NULL);

        LSA_TRACE_09(TRACE_SUBSYS_IOD_SYSADAPT,
                     LSA_TRACE_LEVEL_FATAL,
                     "PNIO_FatalError(): Comp=%d Mod=%d L=%d ErrLen=%d EC1=%d EC2=%d EC3=%d EC4=%d pErr=0x%x\n",
                     pLsaErr->lsa_component_id,
                     pLsaErr->module_id,
                     pLsaErr->line,
                     pLsaErr->error_data_length,
                     pLsaErr->error_code[0],
                     pLsaErr->error_code[1],
                     pLsaErr->error_code[2],
                     pLsaErr->error_code[3],
                     pLsaErr->error_data_ptr);

        PNIO_Log(PNIO_SINGLE_DEVICE_HNDL,
                 PNIO_LOG_ERROR_FATAL,
                 pLsaErr->lsa_component_id,
                 pLsaErr->module_id,
                 pLsaErr->line);
    }


    // *--------------------------------------------*
    // *   TaskOsTimer (void)
    // *   task handler for OS-timeout requests
    // *--------------------------------------------*
    static PNIO_INT32  TaskOsTimer (void)
    {
        void* pVoid = LSA_NULL;   // pointer to Requestblock
        LSA_UINT32 taskID;

        // Wait until own TaskID has been saved by the father process
        OsWaitOnEnable(); // must be first call in every task
        taskID = OsGetThreadId();

        // *-----------------------------------------------------------
        // * loop forever
        // *-----------------------------------------------------------
        while (1)
        {
            PNIO_UINT32 timer_id;
            OsReadMessageBlocked(&pVoid, taskID);

#if DEBUG_TASKCNT
            TaskCnt[taskID]++;  // for debugging only,  can be deleted
#endif

            timer_id = (PNIO_UINT32) pVoid;

#if _DEBUG_LOGGING_CIRC_BUF1
            // *** save actual time in a circular buffer for debugging ***
            //OsSetEntryCircBuf (FIRST_TIM_BUF + 0,  OS_ENTRY_DEFAULT );
            OsSetEntryCircBufX(FIRST_TIM_BUF + 0,  timer_id );
#endif

             //***** call user defined callback function *****
            (*OsTimer[timer_id].callback_timeout)((PNIO_UINT16)timer_id, OsTimer[timer_id].user_id);
        }
        return 0; /*lint !e527 Unreachable code */
    }

    // *--------------------------------------------*
    // *   OsReboot (void)
    // *   force reboot of the system
    // *--------------------------------------------*
    void OsReboot (void)
    {
        PNIOUSR_LOWER_RQB_TYPE  *pReBootEvent;
        OsAlloc((void**) &pReBootEvent, 0, sizeof (PNIOUSR_LOWER_RQB_TYPE));

        LSA_RQB_SET_OPCODE(pReBootEvent, TSKMA_OPC_RBT_REQ_REBOOT);
        
        LSA_RQB_SET_REQ_FCT_PTR(pReBootEvent, tskma_task_rbt_request);
        OsSendMessage(TSKMA_TASK_ID_REBOOT, pReBootEvent, OS_MBX_PRIO_NORM);

        while (1) {}
    }

    #include "ertec200p_reg.h"

    void OsRebootService (void)
    {
        OsWait_ms (500); //time for console output
        cyg_interrupt_disable();
    
        OsShutdownNetwork();
        // * ------------------------------------------------
        // * bit 1: resets all IPs except PN-IP and KRISC32
        // * bit 2: resets only PNIP and KRISC
        // * bit 5: resets KRISC32 core system
        // * bit 6: resets ARM926EJ-S core system
        // * bit 7: resets SDMMC module
        // * bits 31-16: duration of the reset pulse
        // * ------------------------------------------------
        *((LSA_UINT32*) U_SCRB__ASYN_RES_CTRL_REG) = 0x80000006;
        while (1)
        {
            OsWait_ms (500);
            PNIO_printf ("still alive...\n");   // should not occur...
        }
    }


    // *--------------------------------------------*
    // *   OsPrintTaskProp (void)
    // *   print task properties
    // *   (optional, for debugging only)
    // *--------------------------------------------*
      void OsPrintTaskProp (void)
      {
        int TskId;
        for (TskId= 0; TskId < MAXNUM_OF_TASKS; TskId++)
        {
            if (TaskProp[TskId].Occupied)
            {
                printf("TaskID=%-3d, ThreadID=%-3d Name=%s\n",
                       TskId, cyg_thread_get_id(TaskProp[TskId].ThreadId), TaskProp[TskId].TaskName);
            }
        }
      }


    // *--------------------------------------------*
    // *    OsShutdownNetwork(void)
    // *    reset network
    // *--------------------------------------------*
    void OsShutdownNetwork(void)
    {
        OsSystemRamInit();
    }

    void OsSystemRamInit(void)
    {
#ifdef PSI_CFG_USE_EDDP
        LSA_UINT32 i;

        /* reset PNIP ================================================*/
        *((LSA_UINT32*)U_SCRB__ASYN_RES_CTRL_REG) |= U_SCRB__ASYN_RES_CTRL_REG__RES_SOFT_PN;

        /* reset PNIP-memories =======================================*/
        // reset StatisticRam
        for (i = 0; i < PNIP_RAM__SIZE_STATISTIC; i+=4)
        {
            WRITE_ULONG((U_PNIP__BASE + PNIP_RAM__OFFSET_STATISTIC + i), 0x00000000);
        }

        // reset SyncRam
        for (i = 0; i < PNIP_RAM__SIZE_SYNC; i+=4)
        {
            WRITE_ULONG((U_PNIP__BASE + PNIP_RAM__OFFSET_SYNC + i), 0x00000000);
        }
        
        // reset CmdRam
        for (i = 0; i < PNIP_RAM__SIZE_CMD; i+=4)
        {
            WRITE_ULONG((U_PNIP__BASE + PNIP_RAM__OFFSET_CMD + i), 0x00000000);
        }

        // reset ApiCtrlRam
        for (i = 0; i < PNIP_RAM__SIZE_API_CTRL; i+=4)
        {
            WRITE_ULONG((U_PNIP__BASE + PNIP_RAM__OFFSET_API_CTRL + i), 0x00000000);
        }
#endif
    }
#endif


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
