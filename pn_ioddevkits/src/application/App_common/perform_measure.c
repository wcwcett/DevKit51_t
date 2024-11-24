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
/*  F i l e               &F: perform_measure.c                         :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  optional utilities for the user, include                                 */
/*                                                                           */
/*   - performance measuring                                                 */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/

#include "compiler.h"
#include "pniousrd.h"
#include "os.h"
#include "os_taskprio.h"
#include "usriod_utils.h"
#include "nv_data.h"
#include "iodapi_event.h"
#include "perform_measure.h"
#include "pnio_trace.h"

/*== defines ================================================================*/
#define LTRC_ACT_MODUL_ID   13




#ifdef INCLUDE_PERFORMANCE_MEASURE

// *** local defines ***
	#define INITIAL_MEASURE		1
	#define PERFORM_MEASURE		2
    #define MAXCNT_TIMLOOP          4500000

    /*== local data  ============================================================*/
    static volatile PNIO_UINT32   StartPerformanceMeasure;
    static volatile PNIO_UINT32   Cnt;
    static          PNIO_UINT32   TskId_Idle;



    // *----------------------------------------------------------------*
    // *
    // *  Task_Idle (void)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  Task with the lowest priority in the system. This task is not
    // *  necessary for PNIO, it is only used for executing a performance
    // *  measuring request and printing the result.
    // *
    // *  Input:    ----
    // *  Output:   ----
    // *
    // *----------------------------------------------------------------*

    static void Task_Idle(void)
    {
        PNIO_INT32	        TimInitial = 0;
	    PNIO_INT32			StartTim;
	    PNIO_INT32			FinTim;
	    PNIO_INT32			TimMeasure;
	    PNIO_INT32			SysPerformance;

	    // *----------------------------------------------------------
	    // * Synchronization to father process
	    // *----------------------------------------------------------
	    OsWaitOnEnable();		// must be first call in every task


	    while (1)
	    {
		    OsWait_ms (500);
		    if (StartPerformanceMeasure)
		    {
			    StartTim = OsGetTime_us ();
			    for (Cnt = 0; Cnt < MAXCNT_TIMLOOP; Cnt++)
			    {
				    Cnt++;
			    }

			    FinTim = OsGetTime_us();

			    if (StartPerformanceMeasure == INITIAL_MEASURE)
			    { // initial measure without system load
				    TimInitial = FinTim - StartTim;
				    if (TimInitial < 0)
					    TimInitial = -TimInitial;
				    PNIO_printf("Initial Performance finished, value = %d\n", TimInitial);

			    }
			    else
			    {
				    TimMeasure = FinTim - StartTim;
				    if (TimMeasure < 0)
				    {
					    TimMeasure = -TimMeasure;
				    }
			        SysPerformance = (TimMeasure) ? (TimInitial * 1000)/TimMeasure : 1000 /* => 1000 = 0% load */;
				    PNIO_printf("system load  = %d promille\n", 1000 - SysPerformance);
			    }

			    StartPerformanceMeasure = 0;
		    }
	    }
    }



    // *----------------------------------------------------------------*
    // *
    // *  InitPerformanceMeasure (void)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  init function, must be called once during startup, if performance
    // *  measurement shall be used.
    // *
    // *  Input:    ----
    // *  Output:   ----
    // *
    // *----------------------------------------------------------------*
    PNIO_UINT32 InitPerformanceMeasure (void)
    {
        PNIO_UINT32 Status;

	    // *----------------------------------------------------------
	    // * Do the reference performance measure before starting the
	    // * pnio stack.
	    // *----------------------------------------------------------
	    PNIO_printf("scheduling rate is %d ticks per second\n", PN_CLOCKS_PER_SEC);

	    // **** initial Performance measure ******
	    StartPerformanceMeasure = INITIAL_MEASURE;

	    Status = OsCreateThread (Task_Idle, 0, (PNIO_UINT8*)"Pnio_Idle", TASK_PRIO_IDLE, OS_TASK_DEFAULT_STACKSIZE, &TskId_Idle);
	    PNIO_APP_ASSERT (Status == PNIO_OK, "Error OsCreateThread\n");
		Status = OsStartThread (TskId_Idle);
	    PNIO_APP_ASSERT (Status == PNIO_OK, "Error OsStartThread\n");

	    StartPerformanceMeasure = 1;
        OsWait_ms (100);
	    while (StartPerformanceMeasure != 0)
	    {
		    OsWait_ms (100);
	    }
        return (PNIO_OK);
    }


    // *----------------------------------------------------------------*
    // *
    // *  ExecutePerformanceMeasure (void)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  initiates the execution of one measure cycle of actual system
    // *  performance and waits, until it has finished. The feature
    // *  and printing the result is implemented in  Task_Idle(void).
    // *
    // *  Input:    ----
    // *  Output:   ----
    // *----------------------------------------------------------------*
    PNIO_UINT32 ExecutePerformanceMeasure (void)
    {
	    StartPerformanceMeasure = PERFORM_MEASURE	;
	    OsWait_ms (100);
	    while (StartPerformanceMeasure != 0)
		    OsWait_ms (100);

        return (PNIO_OK);
    }

#endif		// performance measure


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
