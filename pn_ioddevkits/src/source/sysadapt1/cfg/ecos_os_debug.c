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
/*  F i l e               &F: ecos_os_debug.c                           :F&  */
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

#include "compiler.h"

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)

    #define LTRC_ACT_MODUL_ID   113
    #define	IOD_MODULE_ID       113

    #include <cyg/kernel/kapi.h>
    #include <pniousrd.h>
    #include "tcp_if.h"
    #include <stdlib.h>
    #include "os.h"


	/* --------------------------------------------------------------------------------------------------
	 * ECOS kernel needs to be compiled with support for instrumentation support when enabling this:     */

	#ifdef  ECOS_KERNEL_INSTRUMENTATION
	#include <pkgconf/kernel.h>
	#include <cyg/kernel/instrmnt.h>
	#endif

    PNIO_VOID OsPrintThreads(PNIO_VOID)
    {
        cyg_handle_t hthread = 0;
        cyg_uint16 id = 0;

        PNIO_printf("ID    state cpri/prio handle    stackbase stacksize stackused / percent name\n");

        while( cyg_thread_get_next( &hthread, &id ) )
        {
            cyg_thread_info info;

            if( !cyg_thread_get_info( hthread, id, &info ) )
                break;

            PNIO_printf("%04xh %04xh %4d/%-4d %08xh %08xh %08xh %08xh /%7.3f%% %s\n",
        		    info.id, info.state, info.cur_pri, info.set_pri,
        		    info.handle,
        		    info.stack_base, info.stack_size,info.stack_used,
        		    (100.0 * info.stack_used) / info.stack_size,
        		    info.name?info.name:"[NA]"
            );
        }
        PNIO_printf("states: RUNNING   =00h, SLEEPING  =01h, COUNTSLEEP=02h,\n"
    		           "        SUSPENDED =04h, CREATING  =08h, EXITED    =10h\n");
    }

    PNIO_VOID OsPrintResources(PNIO_VOID)
    {
	    cyg_thread* thread;
        cyg_handle_t hthread = 0;
        cyg_uint16 id = 0;

        PNIO_printf("Resource List\n");

        while( cyg_thread_get_next( &hthread, &id ) )
        {
            thread = (cyg_thread*)hthread;

    	    cyg_alarm* alarm;

    	    PNIO_printf("\n%s\n  ID %04xh,  State %04xh, CurrentPrio %2d, NormalPrio %2d, Prio is%s inherited\n",
    			    thread->name,
    			    thread->unique_id,thread->state,
    			    thread->priority,thread->original_priority,thread->priority_inherited?"":" not"
    	    );

    	    PNIO_printf("  SuspendCount %08xh, WakeupCount %08xh, MutexCount  %08xh\n"
    			           "  SleepReason %10d, WakeReason %10d, ASR inhibit %08xh, ASR is%s pending\n"
    			           "  StackBase    %08xh, StackLimit  %08xh, StackSize   %08xh, StackPtr %08xh\n",
    			    thread->suspend_count, thread->wakeup_count, thread->mutex_count,
    			    thread->sleep_reason, thread->wake_reason, thread->asr_inhibit, thread->asr_pending?"":" not",
    			    thread->stack_base,thread->stack_limit,thread->stack_size, thread->stack_ptr);

		    alarm = thread->timer.prev;
		    do{
			    alarm = alarm->next;
			    if(alarm->enabled) {
				    PNIO_TrcPrintf("   using Alarm %08xh, trigger %13llu, interval %12llu,\n"
					               "       counter %08xh, counts %14llu, increment %11lu\n",
					        alarm, alarm->trigger, alarm->interval,
					        alarm->counter, alarm->counter->counter, alarm->counter->increment);
			    }
		    }while(alarm != thread->timer.prev);
        }
        PNIO_printf("\nReasons: NONE  = %2d, WAIT     = %2d, DELAY = %2d, TIMEOUT = %2d,\n"
    			       "         BREAK = %2d, DESTRUCT = %2d, EXIT  = %2d, DONE    = %2d\n",
        CYG_REASON_NONE,CYG_REASON_WAIT,CYG_REASON_DELAY,CYG_REASON_TIMEOUT,
        CYG_REASON_BREAK,CYG_REASON_DESTRUCT,CYG_REASON_EXIT,CYG_REASON_DONE);


    }

    PNIO_VOID OsPrintMemInfo(PNIO_VOID)
    {
   	    struct mallinfo mi = mallinfo();

   	    PNIO_printf(
   			    "####################################################################\n"
   			    "total size of memory arena                         : %d\n"
			    "number of ordinary memory blocks                   : %d\n"
			    "number of small memory blocks                      : %d\n"
			    "number of mmapped regions                          : %d\n",
   			    mi.arena,mi.ordblks,mi.smblks,mi.hblks);
   	    PNIO_printf(
			    "total space in mmapped regions                     : %d\n"
			    "space used by small memory blocks                  : %d\n"
			    "space available for small memory blocks            : %d\n"
			    "space used by ordinary memory blocks               : %d\n",
   			    mi.hblkhd,mi.usmblks, mi.fsmblks,mi.uordblks);
   	    PNIO_printf(
			    "space free for ordinary blocks                     : %d\n"
			    "top-most, releasable (via malloc_trim) space       : %d\n"
			    "(NON-STANDARD EXTENSION) size of largest free block: %d\n"
   			    "####################################################################\n",
   			    mi.fordblks,mi.keepcost,mi.maxfree);
    }

    //=============================================================================================
	#ifdef  ECOS_KERNEL_INSTRUMENTATION

    /* Event classes
    #define CYG_INSTRUMENT_CLASS_SCHED              0x0100
    #define CYG_INSTRUMENT_CLASS_THREAD             0x0200
    #define CYG_INSTRUMENT_CLASS_INTR               0x0300
    #define CYG_INSTRUMENT_CLASS_MUTEX              0x0400
    #define CYG_INSTRUMENT_CLASS_CONDVAR            0x0500
    #define CYG_INSTRUMENT_CLASS_BINSEM             0x0600
    #define CYG_INSTRUMENT_CLASS_CNTSEM             0x0700
    #define CYG_INSTRUMENT_CLASS_CLOCK              0x0800
    #define CYG_INSTRUMENT_CLASS_ALARM              0x0900
    #define CYG_INSTRUMENT_CLASS_MBOXT              0x0a00
    #define CYG_INSTRUMENT_CLASS_SMP                0x0b00
    #define CYG_INSTRUMENT_CLASS_MLQ                0x0c00

	// Scheduler events
	#define CYG_INSTRUMENT_EVENT_SCHED_LOCK         1
	#define CYG_INSTRUMENT_EVENT_SCHED_UNLOCK       2
	#define CYG_INSTRUMENT_EVENT_SCHED_RESCHEDULE   3
	#define CYG_INSTRUMENT_EVENT_SCHED_TIMESLICE    4

	// Thread events
	#define CYG_INSTRUMENT_EVENT_THREAD_SWITCH      1
	#define CYG_INSTRUMENT_EVENT_THREAD_SLEEP       2
	#define CYG_INSTRUMENT_EVENT_THREAD_WAKE        3
	#define CYG_INSTRUMENT_EVENT_THREAD_SUSPEND     4
	#define CYG_INSTRUMENT_EVENT_THREAD_RESUME      5
	#define CYG_INSTRUMENT_EVENT_THREAD_PRIORITY    6
	#define CYG_INSTRUMENT_EVENT_THREAD_DELAY       7
	#define CYG_INSTRUMENT_EVENT_THREAD_ALARM       8
	#define CYG_INSTRUMENT_EVENT_THREAD_ENTER       9
	#define CYG_INSTRUMENT_EVENT_THREAD_CHECK_STACK 10
	#define CYG_INSTRUMENT_EVENT_THREAD_ATTACH_STACK 11
	#define CYG_INSTRUMENT_EVENT_THREAD_CREATE      12

	// Interrupt events
	#define CYG_INSTRUMENT_EVENT_INTR_RAISE         1
	#define CYG_INSTRUMENT_EVENT_INTR_END           2
	#define CYG_INSTRUMENT_EVENT_INTR_RESTORE       3
	#define CYG_INSTRUMENT_EVENT_INTR_POST_DSR      4
	#define CYG_INSTRUMENT_EVENT_INTR_CALL_DSR      5
	#define CYG_INSTRUMENT_EVENT_INTR_ATTACH        6
	#define CYG_INSTRUMENT_EVENT_INTR_DETACH        7
	#define CYG_INSTRUMENT_EVENT_INTR_SET_VSR       8
	#define CYG_INSTRUMENT_EVENT_INTR_DISABLE       9
	#define CYG_INSTRUMENT_EVENT_INTR_ENABLE        10
	#define CYG_INSTRUMENT_EVENT_INTR_MASK          11
	#define CYG_INSTRUMENT_EVENT_INTR_UNMASK        12
	#define CYG_INSTRUMENT_EVENT_INTR_CONFIGURE     13
	#define CYG_INSTRUMENT_EVENT_INTR_ACK           14
	#define CYG_INSTRUMENT_EVENT_INTR_CHAIN_ISR     15
	#define CYG_INSTRUMENT_EVENT_INTR_SET_CPU       16
	#define CYG_INSTRUMENT_EVENT_INTR_GET_CPU       17
	START_DSR  18
	STOP_DSR   19

	// Mutex events
	#define CYG_INSTRUMENT_EVENT_MUTEX_LOCK         1
	#define CYG_INSTRUMENT_EVENT_MUTEX_WAIT         2
	#define CYG_INSTRUMENT_EVENT_MUTEX_LOCKED       3
	#define CYG_INSTRUMENT_EVENT_MUTEX_TRY          4
	#define CYG_INSTRUMENT_EVENT_MUTEX_UNLOCK       5
	#define CYG_INSTRUMENT_EVENT_MUTEX_WAKE         6
	#define CYG_INSTRUMENT_EVENT_MUTEX_RELEASE      7
	#define CYG_INSTRUMENT_EVENT_MUTEX_RELEASED     8

	// Condition variable events
	#define CYG_INSTRUMENT_EVENT_CONDVAR_WAIT       1
	#define CYG_INSTRUMENT_EVENT_CONDVAR_WOKE       2
	#define CYG_INSTRUMENT_EVENT_CONDVAR_SIGNAL     3
	#define CYG_INSTRUMENT_EVENT_CONDVAR_WAKE       4
	#define CYG_INSTRUMENT_EVENT_CONDVAR_BROADCAST  5
	#define CYG_INSTRUMENT_EVENT_CONDVAR_TIMED_WAIT 6

	// Binary semaphore events
	#define CYG_INSTRUMENT_EVENT_BINSEM_CLAIM       1
	#define CYG_INSTRUMENT_EVENT_BINSEM_WAIT        2
	#define CYG_INSTRUMENT_EVENT_BINSEM_WOKE        3
	#define CYG_INSTRUMENT_EVENT_BINSEM_TRY         4
	#define CYG_INSTRUMENT_EVENT_BINSEM_POST        5
	#define CYG_INSTRUMENT_EVENT_BINSEM_WAKE        6
	#define CYG_INSTRUMENT_EVENT_BINSEM_TIMEOUT     7

	// Counting semaphore events
	#define CYG_INSTRUMENT_EVENT_CNTSEM_CLAIM       1
	#define CYG_INSTRUMENT_EVENT_CNTSEM_WAIT        2
	#define CYG_INSTRUMENT_EVENT_CNTSEM_WOKE        3
	#define CYG_INSTRUMENT_EVENT_CNTSEM_TRY         4
	#define CYG_INSTRUMENT_EVENT_CNTSEM_POST        5
	#define CYG_INSTRUMENT_EVENT_CNTSEM_WAKE        6
	#define CYG_INSTRUMENT_EVENT_CNTSEM_TIMEOUT     7

	// Clock events
	#define CYG_INSTRUMENT_EVENT_CLOCK_TICK_START   1
	#define CYG_INSTRUMENT_EVENT_CLOCK_TICK_END     2
	#define CYG_INSTRUMENT_EVENT_CLOCK_ISR          3

	// Alarm events
	#define CYG_INSTRUMENT_EVENT_ALARM_ADD          1
	#define CYG_INSTRUMENT_EVENT_ALARM_REM          2
	#define CYG_INSTRUMENT_EVENT_ALARM_CALL         3
	#define CYG_INSTRUMENT_EVENT_ALARM_INIT         4
	#define CYG_INSTRUMENT_EVENT_ALARM_TRIGGER      5
	#define CYG_INSTRUMENT_EVENT_ALARM_INTERVAL     6

	// Mboxt events
	#define CYG_INSTRUMENT_EVENT_MBOXT_WAIT         1
	#define CYG_INSTRUMENT_EVENT_MBOXT_GET          2
	#define CYG_INSTRUMENT_EVENT_MBOXT_GOT          3
	#define CYG_INSTRUMENT_EVENT_MBOXT_TIMEOUT      4
	#define CYG_INSTRUMENT_EVENT_MBOXT_WAKE         5
	#define CYG_INSTRUMENT_EVENT_MBOXT_TRY          6
	#define CYG_INSTRUMENT_EVENT_MBOXT_PUT          7

	// SMP events
	#define CYG_INSTRUMENT_EVENT_SMP_LOCK_INC       1
	#define CYG_INSTRUMENT_EVENT_SMP_LOCK_ZERO      2
	#define CYG_INSTRUMENT_EVENT_SMP_LOCK_SET       3
	#define CYG_INSTRUMENT_EVENT_SMP_CPU_START      4
	#define CYG_INSTRUMENT_EVENT_SMP_LOCK_WAIT      5
	#define CYG_INSTRUMENT_EVENT_SMP_LOCK_GOT       6
	#define CYG_INSTRUMENT_EVENT_SMP_RESCHED_SEND   8
	#define CYG_INSTRUMENT_EVENT_SMP_RESCHED_RECV   9

	// MLQ scheduler events

	#define CYG_INSTRUMENT_EVENT_MLQ_SCHEDULE       1
	#define CYG_INSTRUMENT_EVENT_MLQ_RESCHEDULE     2
	#define CYG_INSTRUMENT_EVENT_MLQ_ADD            3
	#define CYG_INSTRUMENT_EVENT_MLQ_REM            4
	#define CYG_INSTRUMENT_EVENT_MLQ_TIMESLICE      5
	#define CYG_INSTRUMENT_EVENT_MLQ_YIELD          6
	#define CYG_INSTRUMENT_EVENT_MLQ_ENQUEUE        7
	#define CYG_INSTRUMENT_EVENT_MLQ_DEQUEUE        8
	#define CYG_INSTRUMENT_EVENT_MLQ_REMOVE         9


	// User events

	#define CYG_INSTRUMENT_EVENT_USER_1             1
	#define CYG_INSTRUMENT_EVENT_USER_2             2
	#define CYG_INSTRUMENT_EVENT_USER_3             3
	#define CYG_INSTRUMENT_EVENT_USER_4             4
	#define CYG_INSTRUMENT_EVENT_USER_5             5
	#define CYG_INSTRUMENT_EVENT_USER_6             6
	#define CYG_INSTRUMENT_EVENT_USER_7             7
	#define CYG_INSTRUMENT_EVENT_USER_8             8
	#define CYG_INSTRUMENT_EVENT_USER_9             9
	#define CYG_INSTRUMENT_EVENT_USER_10            10
	#define CYG_INSTRUMENT_EVENT_USER_11            11
	#define CYG_INSTRUMENT_EVENT_USER_12            12
	#define CYG_INSTRUMENT_EVENT_USER_13            13
	#define CYG_INSTRUMENT_EVENT_USER_14            14
	#define CYG_INSTRUMENT_EVENT_USER_15            15
	#define CYG_INSTRUMENT_EVENT_USER_16            16
	#define CYG_INSTRUMENT_EVENT_USER_17            17
	#define CYG_INSTRUMENT_EVENT_USER_18            18
	#define CYG_INSTRUMENT_EVENT_USER_19            19

	*/

	#define CYG_INSTRUMENT_EVENT_USER_START			CYG_INSTRUMENT_EVENT_USER_1
	#define CYG_INSTRUMENT_EVENT_USER_STOP			CYG_INSTRUMENT_EVENT_USER_2


    #define ECOS_INSTRUMENT_BUFFER_SIZE			0x40000			/* 16 Byte/Event * 0x40000 = 4MB   */
//	#define ECOS_INSTRUMENT_BUFFER_SIZE			0x10000			/* 16 Byte/Event * 0x10000 = 1MB   */
//  #define ECOS_INSTRUMENT_BUFFER_SIZE			0x2000		    /* 16 Byte/Event * 0x2000  = 128kB */

    struct Instrument_Record
    {
     CYG_WORD16 type; // record type
     CYG_WORD16 thread; // current thread id
     CYG_WORD timestamp; // 32 bit timestamp
     CYG_WORD arg1; // first arg
     CYG_WORD arg2; // second arg
    };

    struct Instrument_Record 		instrument_buffer[ECOS_INSTRUMENT_BUFFER_SIZE];
    cyg_uint32 						instrument_buffer_size = ECOS_INSTRUMENT_BUFFER_SIZE;
    volatile cyg_uint32 			instrument_user_flag = 0;

    #define ECOS_THREAD_INFO_MAX 		32			/* max number of thread info entries */
	#define ECOS_THREAD_INFO_NAME_SIZE 	28			/* max number of thread info entries */

    struct Thread_Info_Record
    {
     CYG_WORD32 id;
     CYG_BYTE   name[ECOS_THREAD_INFO_NAME_SIZE];
    };

    struct Thread_Info_Record 		thread_info_buffer[ECOS_THREAD_INFO_MAX];




    PNIO_VOID* OsIntrumentationBuffer(PNIO_VOID)
    {
    	return (PNIO_VOID*) &instrument_buffer;
    }

    PNIO_UINT32 OsIntrumentationBufferSize(PNIO_VOID)
    {
    	return sizeof(instrument_buffer);
    }

    PNIO_VOID* OsThreadInfoBuffer(PNIO_VOID)
    {
    	// fill thread info buffer
        cyg_handle_t hthread = 0;
        cyg_uint16 id = 0;
        PNIO_UINT32 index = 0;
        OsMemSet(thread_info_buffer, 0x00, sizeof(thread_info_buffer));
        while ((index < ECOS_THREAD_INFO_MAX) && (cyg_thread_get_next( &hthread, &id)))
        {
        	cyg_thread* thread = (cyg_thread*)hthread;
            thread_info_buffer[index].id  = thread->unique_id;
            OsStrnCpy(thread_info_buffer[index].name, thread->name, ECOS_THREAD_INFO_NAME_SIZE-1);
			index++;
        };
    	return (PNIO_VOID*) &thread_info_buffer;
    }

    PNIO_UINT32 OsThreadInfoBufferSize(PNIO_VOID)
    {
    	return sizeof(thread_info_buffer);
    }


    PNIO_VOID OsIntrumentationEnable(PNIO_UINT32 mask)
    {
    	 //cyg_instrument_enable(mask, 0);
    	 cyg_instrument_enable(CYG_INSTRUMENT_CLASS_CLOCK,   0);
       //cyg_instrument_enable(CYG_INSTRUMENT_CLASS_SCHED,   0);
    	 cyg_instrument_enable(CYG_INSTRUMENT_CLASS_THREAD,  0);
         cyg_instrument_enable(CYG_INSTRUMENT_CLASS_INTR,    0); //CYG_INSTRUMENT_EVENT_INTR_RAISE | CYG_INSTRUMENT_EVENT_INTR_POST_DSR);
         cyg_instrument_enable(CYG_INSTRUMENT_CLASS_MUTEX,   0); //CYG_INSTRUMENT_EVENT_MUTEX_LOCK | CYG_INSTRUMENT_EVENT_MUTEX_UNLOCK);
       //cyg_instrument_enable(CYG_INSTRUMENT_CLASS_CONDVAR, 0);
       //cyg_instrument_enable(CYG_INSTRUMENT_CLASS_BINSEM,  0);
       //cyg_instrument_enable(CYG_INSTRUMENT_CLASS_CNTSEM,  0);
    	 cyg_instrument_enable(CYG_INSTRUMENT_CLASS_USER,    0);
    }

    PNIO_VOID OsIntrumentationDisable(PNIO_UINT32 mask)
    {
    	 //cyg_instrument_disable(mask, 0);
    	 cyg_instrument_disable(CYG_INSTRUMENT_CLASS_CLOCK,   0);
    	 cyg_instrument_disable(CYG_INSTRUMENT_CLASS_SCHED,   0);
    	 cyg_instrument_disable(CYG_INSTRUMENT_CLASS_THREAD,  0);
         cyg_instrument_disable(CYG_INSTRUMENT_CLASS_INTR,    0); //CYG_INSTRUMENT_EVENT_INTR_RAISE | CYG_INSTRUMENT_EVENT_INTR_POST_DSR);
    	 cyg_instrument_disable(CYG_INSTRUMENT_CLASS_MUTEX,   0);
    	 cyg_instrument_disable(CYG_INSTRUMENT_CLASS_CONDVAR, 0);
    	 cyg_instrument_disable(CYG_INSTRUMENT_CLASS_BINSEM,  0);
    	 cyg_instrument_disable(CYG_INSTRUMENT_CLASS_CNTSEM,  0);
    	 cyg_instrument_disable(CYG_INSTRUMENT_CLASS_USER,    0);
    }

    PNIO_VOID OsIntrumentUserEvent(PNIO_UINT8 event, PNIO_UINT32 arg1, PNIO_UINT32 arg2)
    {
    	instrument_user_flag = (volatile cyg_uint32) arg1;
    	CYG_INSTRUMENT_USER(event, arg1, arg2);
    }

    PNIO_VOID OsIntrumentUserStart(PNIO_UINT32 arg1, PNIO_UINT32 arg2)
    {
    	instrument_user_flag = (volatile cyg_uint32) arg1;
    	CYG_INSTRUMENT_USER(CYG_INSTRUMENT_EVENT_USER_START, arg1, arg2);
    }

    PNIO_VOID OsIntrumentUserStop(PNIO_UINT32 arg1, PNIO_UINT32 arg2)
    {
    	instrument_user_flag = (volatile cyg_uint32) arg1;
    	CYG_INSTRUMENT_USER(CYG_INSTRUMENT_EVENT_USER_STOP, arg1, arg2);
    }



	#define TCP_TRC_UPLOAD_PORTNUMBER    998

	#define MAX_BLOCK_SIZE 40000

	typedef struct
	{
		PNIO_INT StartTag;
		PNIO_INT Command;
	}   TCP_COMMAND_HEADER;


	static PNIO_INT LocSockId, RemSockId;


	/***************************************************************************/
	PNIO_VOID TcpUploadIntrumentationBuffer (PNIO_UINT8* pBuf, PNIO_UINT32 bufSize)
	{
		PNIO_UINT32 RestBytes = bufSize;
		PNIO_UINT32 RestBytes_htonl  = OsHtonl (RestBytes);
		PNIO_UINT32 BufOffset        = 0;
		PNIO_UINT16 BytesToSend      = 0;
		PNIO_UINT16 BytesSent        = 0;

		// **** send image file to tcp server *****
		BytesSent = tcp_if_send (RemSockId, (unsigned char*) &RestBytes_htonl, 4);

		while (RestBytes > 0)
		{
			BytesToSend = (RestBytes > MAX_BLOCK_SIZE) ? MAX_BLOCK_SIZE : (PNIO_UINT16) RestBytes;
			BytesSent 	= tcp_if_send (RemSockId, pBuf + BufOffset, BytesToSend);

			if (BytesSent <= BytesToSend)
			{
				RestBytes -= BytesSent;
				BufOffset += BytesSent;
				PNIO_printf(".");
			}
		}
		PNIO_printf(" ");
	}


	/***************************************************************************/
	PNIO_UINT32 TcpIntrumentationUpload  (PNIO_VOID)
	{
		TCP_COMMAND_HEADER Hdr;
		PNIO_UINT32 NumOfBytes;

		PNIO_printf ("TcpIntrumentationUpload Server: Init TCP interface\n");

		// ***** init tcp interface ******
		LocSockId = tcp_if_inits();
		if (LocSockId == TCP_SOCK_ERROR)
		{
			PNIO_printf ("create socket error\n");
			return (PNIO_NOT_OK);
		}
		PNIO_printf ("TcpTrTcpIntrumentationUploadaceUpload Server: Wait on connection ... \n");

		// ***** connect to client ******
		RemSockId = tcp_if_connectS (LocSockId, 0, TCP_TRC_UPLOAD_PORTNUMBER);
		if (RemSockId == TCP_SOCK_ERROR)
		{
			PNIO_printf ("TCP CONNECT ERROR!\n");
			return (PNIO_NOT_OK);
		}
		PNIO_printf ("TcpIntrumentationUpload Server: Connection established\n");

		// ***** read data header  ******
		PNIO_printf ("TcpIntrumentationUpload Server: Wait on upload request ...");
		NumOfBytes = tcp_if_receive (RemSockId, (PNIO_UINT8*) &Hdr, sizeof (Hdr));
		if (NumOfBytes != sizeof (Hdr))
		{
			PNIO_printf ("\nError intrumentation upload header header size = %d\n", NumOfBytes);
			tcp_if_disconnect (RemSockId);
			tcp_if_close (LocSockId);
			return (PNIO_NOT_OK);
		}
		PNIO_printf ("OK, upload requested\n");

		// **** send number of trace buffers to tcp server *****
		{
			PNIO_UINT32 FileIndex;
			PNIO_UINT32 FileCount 		= 2;
			PNIO_UINT32 FileCount_htonl  = OsHtonl (FileCount);

			tcp_if_send (RemSockId, (unsigned char*) &FileCount_htonl, 4);

			for (FileIndex = 0; FileIndex < FileCount; FileIndex++)
			{
				PNIO_UINT8* pBuf = NULL;
				PNIO_UINT32 bufSize = 0;
				PNIO_printf ("TcpIntrumentationUpload Server: Send intrumentation file #%d with %d bytes ... ", FileIndex, OsIntrumentationBufferSize());
				switch (FileIndex)
				{
					case 0:
					{
						pBuf    = OsIntrumentationBuffer();
						bufSize = OsIntrumentationBufferSize();
						break;
					}
					case 1:
					{
						pBuf    = OsThreadInfoBuffer();
						bufSize = OsThreadInfoBufferSize();
						break;
					}
					default: PNIO_printf ("TcpIntrumentationUpload: ERROR Unknown file index = %d\n", FileIndex);
				}
				// **** send trace buffer to TCP server ****
				if (pBuf)
				{
					TcpUploadIntrumentationBuffer (pBuf, bufSize);

					// **** wait for TRACE READY command from TCP server ****
					NumOfBytes = tcp_if_receive (RemSockId, (PNIO_UINT8*) &Hdr, sizeof (Hdr));
					if (NumOfBytes != sizeof (Hdr))
					{
						PNIO_printf ("\nError upload header header size = %d\n", NumOfBytes);
						tcp_if_disconnect (RemSockId);
						tcp_if_close (LocSockId);
						return (PNIO_NOT_OK);
					}
					PNIO_printf ("done\n");
				}
			}
		}

		// ***** disconnect TCP server ****
		PNIO_printf("TcpIntrumentationUpload Server: disconnect and close\n");
		OsWait_ms (1000);
		tcp_if_disconnect (RemSockId);
		tcp_if_close (LocSockId);

		PNIO_printf("TcpIntrumentationUpload Server: done\n");
		return (PNIO_OK);
	}
	#else /* PNIOD_ECOS_KERNEL_INSTRUMENTATION */

    PNIO_VOID OsIntrumentationEnable     (PNIO_UINT32 mask)
    {
    }
    PNIO_VOID OsIntrumentationDisable    (PNIO_UINT32 mask)
    {
    }
	PNIO_UINT32 TcpIntrumentationUpload  (PNIO_VOID)
	{
		return 0;
	}

    #endif


#endif


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
