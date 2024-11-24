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
/*  F i l e               &F: pnpb.c                                    :F&  */
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

#include "pniousrd.h"
#include "trc_if.h"
#include "pndv_inc.h"
#include "pnpb.h"
#include "os_taskprio.h"

#define LTRC_ACT_MODUL_ID   213
#define PNPB_MODULE_ID      213

// *---------------------------------------------
// * public variables
// *---------------------------------------------
volatile PNPB_INSTANCE   Pnpb;

// *---------------------------------------------
// * static variables
// *---------------------------------------------
PNIO_UINT32             TskId_Pnpb = 0;


// *---------------------------------------------
// * static functions
// *---------------------------------------------
static LSA_INT32   Task_Pnpb (PNIO_VOID);



// *---------------------------------------------
// * public functions
// *---------------------------------------------

    PNIO_VOID pnpb_init (PNIO_VOID)
    {
        PNIO_UINT32 Status;

        // *** init pnpb peri interface ***
        pnpb_peri_init ();
        pnpb_sub_exp_init();

        #if IOD_INCLUDE_REC8028_8029
            pnpb_io_init();
        #endif

        //TrcDkInit();

        pnbp_StartTaskPost();

        // *-----------------------------------------------------------
        // * create task PNPB
        // *-----------------------------------------------------------
        // *** create and start Task for EDD high priority context, add message queue to task ***
        Status = (PNPB_TASK_CREATE((PNIO_VOID(*)(PNIO_VOID))Task_Pnpb, 0, (PNIO_UINT8*)"Pnio_Pnpb", TASK_PRIO_PNPB, PNPB_TASK_DEFAULT_STACKSIZE, &TskId_Pnpb));
        PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: PNPB_TASK_CREATE\n");
        Status = PNPB_MBOX_CREATE(TskId_Pnpb);  // install the task message queue
        PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: PNPB_MBOX_CREATE\n");
        Status = PNPB_TASK_START(TskId_Pnpb);
        PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: PNPB_TASK_START\n");
        Pnpb.State = PNPB_INIT;

        return;
    }


    PNIO_VOID pnpb_open (pnpb_sys_parameter_ptr_t parameter_ptr)
    {
        // ***** save memory pointer of coupling interface ****
        pPnpbIf = parameter_ptr->pndvInterface;
        Pnpb.OpenDoneCbf = parameter_ptr->done_cbf;

        // *---------------------------------------
        // * open PNPB
        // *---------------------------------------
        {

            Pnpb.State = PNPB_OPEN;


            // **** start PNDV ****
            pnpb_write_event_to_pndv (  PNDV_EV_TO_PNDV_PNDV_START, // command
                                        0,                          // add value 1
                                        0,
                                        NULL);
            PNPB_TRIGGER_PNDV ();
        }
    }

    // *--------------------------------------------------------
    // *  Task_PNPB
    // *
    // *
    // *--------------------------------------------------------
    PNPB_CODE_FAST  static LSA_INT32  Task_Pnpb (PNIO_VOID)
    {
      // Wait until own TaskID has been saved by the father process
      PNPB_WAIT_ON_ENABLE(); // must be first call in every task

      // *-----------------------------------------------------------
      // * loop forever
      // *-----------------------------------------------------------
      while (1)
      {
        PNPB_SEM_B_TAKE(Pnpb.SemId[SEM_PNPB_PERIF]);
        Pnpb.SemEventAvailable = 0;

        if (Pnpb.State >= PNPB_OPEN)
           pnpb_process_service_requests();
      }
      return (0); /*lint !e527 unreachable code */
    }


    PNPB_STATE      pnpb_get_state            (PNIO_VOID)
    {
        return (Pnpb.State);
    }

    PNIO_VOID pnpb_activate (PNIO_VOID)
    {
        pnpb_write_event_to_pndv (  PNDV_EV_TO_PNDV_PNDV_AKT, // command
                                    0,                          // add value 1
                                    0,
                                    NULL);
        PNPB_TRIGGER_PNDV ();
    }

    PNIO_VOID pnpb_deactivate (PNIO_VOID)
    {
        pnpb_write_event_to_pndv (  PNDV_EV_TO_PNDV_PNDV_DEAKT, // command
                                    0,                          // add value 1
                                    0,
                                    NULL);
        PNPB_TRIGGER_PNDV ();
    }

    PNIO_VOID pnpb_handle_nau_irq (PNIO_VOID)
    {
        return;
    }

    PNIO_VOID pnpb_close (PNIO_VOID)
    {
        // **** stop PNDV ****
        pnpb_write_event_to_pndv (  PNDV_EV_TO_PNDV_PNDV_STOP,  // command
                                        0,                          // add value 1
                                        0,
                                        NULL);

        PNPB_TRIGGER_PNDV ();

        return;
    }

    PNIO_UINT32 pnpb_set_dev_state (PNIO_UINT32 DevHndl,   // device handle
                                    PNIO_UINT32 DevState)  // device state CLEAR/OPERATE
    {
        PNIO_UINT8 add1 = 0;
        switch (DevState)
        {

            case PNIO_DEVSTAT_OPERATE:
                {
                    add1 = PNDV_PERI_STATE_OK;
                }
                break;
            case PNIO_DEVSTAT_CLEAR:
                {
                    add1 = PNDV_PERI_STATE_NOT_OK;
                }
                break;
            default:
                return (PNIO_NOT_OK);
        }

        PnpbReqSync    (PNDV_EV_TO_PNDV_PERI_STATE_IND,
                        add1,
                        0,
                        Pnpb.SemId[SEM_PNPB_START],
                        LSA_NULL);

        return (PNIO_OK);
    }


    // *----------------------------------------------------------------*
    // *
    // *  pnpb_device_ar_abort (...)
    // *
    // *----------------------------------------------------------------*
    // *  kills the specified AR
    // *  to hand over all the stored PDEV-record data to the stack.
    // *
    // *  Input:        PNIO_UINT32          ArNum, // [in]  AR number (1...N)
    // *
    // *  Output:       return               PNIO_OK, PNIO_NOT_OK
    // *
    // *----------------------------------------------------------------*
    PNIO_UINT32     pnpb_device_ar_abort (PNIO_UINT32 ArNum)
    {

        // *---------------------------------------------
        // *  write command to perif and trigger PNDV
        // *---------------------------------------------
        pnpb_write_event_to_pndv (PNDV_EV_TO_PNDV_AR_ABORT_REQ,
                                  (PNIO_UINT8)ArNum - 1,
                                  0,
                                  NULL);
        PNPB_TRIGGER_PNDV ();
        return (PNIO_OK);
    }

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
