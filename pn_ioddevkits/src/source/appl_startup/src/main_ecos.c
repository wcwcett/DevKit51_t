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
/*  F i l e               &F: main_ecos.c                               :F&  */
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

    #include "compiler_stdlibs.h"
    #include "os.h"
    #include "os_taskprio.h"
    #include "pniousrd.h"
    #include "perform_measure.h"

	#include <stdio.h>                      /* printf */
	#include <string.h>                     /* strlen */
	#include <cyg/kernel/kapi.h>            /* All the kernel specific stuff */
	#include <cyg/io/io.h>                  /* I/O functions */
	#include <cyg/hal/hal_arch.h>           /* CYGNUM_HAL_STACK_SIZE_TYPICAL */
	#include <cyg/infra/diag.h>


	/* DEFINES */

	#define NTHREADS 1
	#define STACKSIZE ( CYGNUM_HAL_STACK_SIZE_TYPICAL + 4096 )

	/* STATICS */

	static cyg_handle_t thread[NTHREADS];
	static cyg_thread thread_obj[NTHREADS];
	static PNIO_CHAR stack[NTHREADS][STACKSIZE];

    PNIO_INT main_ecos(PNIO_VOID);

	static PNIO_VOID sample_thread_entry(CYG_ADDRESS data)
	{
		main_ecos();
	}

	PNIO_VOID cyg_user_start(PNIO_VOID)
	{
		cyg_thread_create(4, sample_thread_entry, (cyg_addrword_t) 0, "PNIODK",
						  (PNIO_VOID *)stack[0], STACKSIZE, &thread[0], &thread_obj[0]);
		cyg_thread_resume(thread[0]);
	}


    extern PNIO_VOID MainAppl   (PNIO_VOID);

    LSA_UINT32 TskId_Main;


    PNIO_INT main_ecos(PNIO_VOID)
    {
		PNIO_UINT32 Status;
        // *------------------------------------------------*
        // *  initialize PNIO stack
        // *------------------------------------------------*
        PNIO_init ();

        #ifdef INCLUDE_PERFORMANCE_MEASURE
		// *----------------------------------------------------------
		// * Do the reference performance measure before starting the
		// * pnio stack. This is a test-option only and not necessary
		// * for PNIO.
		// *----------------------------------------------------------
		{
			Status = InitPerformanceMeasure ();
			if (Status != LSA_OK)
			{
				PNIO_printf ("error in InitPerformanceMeasure()\n");
			}
		}
        #endif // INCLUDE_PERFORMANCE_MEASURE
 
		// *------------------------------------------------*
		// *  create and start PROFINET user application
		// *------------------------------------------------*
        Status = OsCreateThread(MainAppl, 0, (PNIO_UINT8*)"MainAppl", TASK_PRIO_MAIN, OS_TASK_DEFAULT_STACKSIZE, &TskId_Main);
        if (Status != PNIO_OK)
        {
            printf ("error OsCreateThread\n");
        }

		Status = OsCreateMsgQueue (TskId_Main);
        if (Status != PNIO_OK)
        {
            printf ("error OsCreateThread\n");
        }

		Status = OsStartThread (TskId_Main);
        if (Status != PNIO_OK)
        {
            printf ("error OsCreateThread\n");
        }

	    while (1)
	    {
		    #ifndef DEBUG
		    cyg_thread_suspend(cyg_thread_self()); // sleep forever
		    #else
		    OsWait_ms(1000); // for debug
		    #endif
	    }
	    return (0); /*lint !e527 Unreachable code */
    }

#endif
 
/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
