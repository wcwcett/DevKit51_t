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
/*  F i l e               &F: tskma_task_app.c                          :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*   Task creation and startup handling for PROFINET stack                   */
/*                                                                           */
/*****************************************************************************/

#include "tskma_inc.h"
#include "tskma_pdb.h"

#define TSKMA_MODULE_ID TSKMA_MODULE_ID_TSKMA_TASK_APP

#if DEBUG_TASKCNT
extern PNIO_UINT32  TaskCnt[MAXNUM_OF_TASKS];
#endif

//*----------------------------------------------------------------------------
//* tskma_task_app_setup()
//*
//* Fill up open parameters. Init system ram and isr. 
//* Install interrupt handler and start watchdog
//*----------------------------------------------------------------------------
LSA_VOID tskma_task_app_setup(LSA_VOID)
{
    tskma_pdb_open();

    TSKMA_SYSTEM_RAM_INIT();

    TSKMA_INIT_ISR(0);

    TSKMA_START_WATCHDOG();      // to enable a watchdog timer
}


//*----------------------------------------------------------------------------
//* tskma_task_app_init()
//*
//* Init PNDV, IOM, and EVMA 
//*----------------------------------------------------------------------------
LSA_VOID tskma_task_app_init(LSA_VOID)
{
    pndv_init((PNDV_IFACE_STRUCT_PTR)&glob_coupling_interface.pndv);

    iom_init();

    evma_init();
}


//*----------------------------------------------------------------------------
//* tskma_task_app_open()
//*
//* Open PNDV and PNPB
//*----------------------------------------------------------------------------
LSA_VOID tskma_task_app_open(LSA_VOID )
{
    pndv_open( &tskma_pdb_pndv_open_parameter); /* sync */

    pnpb_init();
    {
        pnpb_open_parameter_t pnpb_init_struct;
        pnpb_init_struct.pndvInterface = (PNDV_IFACE_STRUCT_PTR) &glob_coupling_interface.pndv;
        pnpb_init_struct.done_cbf      = tskma_task_app_pnpb_open_done;

        pnpb_open(&pnpb_init_struct);
    }
}


//*----------------------------------------------------------------------------
//* tskma_task_app_startup_stopped()
//*
//* not use so far
//*----------------------------------------------------------------------------
LSA_VOID tskma_task_app_startup_stopped()
{
    /*lint -esym(522,tskma_task_app_startup_stopped) ignore for empty functions */
}


//*----------------------------------------------------------------------------
//* tskma_task_app_cyclic_ea()
//*
//* reset task state of IO update
//*----------------------------------------------------------------------------
LSA_VOID tskma_task_app_cyclic_ea()
{
    tskma_data.ts_state = TSKMA_TS_SCHEDULING_STATE_TS_TASK_FINISHED;
}



//*----------------------------------------------------------------------------
//* tskma_task_app_send_pnpb_trigger()
//*
//* Trigger PNPB action
//*----------------------------------------------------------------------------
PNIO_VOID tskma_task_app_send_pnpb_trigger()
{
    if (tskma_data.app.rqb.trigger_pndv_to_pnpb.static_rqb_in_use == LSA_FALSE)
    {
        tskma_data.app.rqb.trigger_pndv_to_pnpb.static_rqb_in_use = LSA_TRUE;
        TSKMA_OS_SEND_MAIL(TSKMA_TASK_NUM_APP, &tskma_data.app.rqb.trigger_pndv_to_pnpb);
    }
}

//*----------------------------------------------------------------------------
//* tskma_task_app_send_pndv_trigger()
//*
//* Trigger PNDV action
//*----------------------------------------------------------------------------
PNIO_VOID tskma_task_app_send_pndv_trigger()
{
    if (tskma_data.app.rqb.trigger_pnpb_to_pndv.static_rqb_in_use == LSA_FALSE)
    {
        tskma_data.app.rqb.trigger_pnpb_to_pndv.static_rqb_in_use = LSA_TRUE;
        TSKMA_OS_SEND_MAIL(TSKMA_TASK_NUM_APP, &tskma_data.app.rqb.trigger_pnpb_to_pndv);
    }
}


//*----------------------------------------------------------------------------
//* tskma_task_app_pnpb_open_done()
//*
//* Function is called when PNPB open was finished.
//*----------------------------------------------------------------------------
PNIO_VOID tskma_task_app_pnpb_open_done (PNIO_VOID)
{
    TSKMA_RQB_SET_REQ_FCT_PTR(tskma_data.cold_start.app_rqb_ptr, tskma_task_stp_perform_cold_start_sm);
    TSKMA_OS_SEND_MAIL(TSKMA_TASK_NUM_STP, tskma_data.cold_start.app_rqb_ptr);
}


//*----------------------------------------------------------------------------
//* tskma_task_app_request()
//*
//* Distribute TSKMA messages
//*----------------------------------------------------------------------------
OS_CODE_FAST PNIO_VOID tskma_task_app_request(TSKMA_RQB_S_PTR_T rqb_ptr)
{
    switch(TSKMA_RQB_GET_OPCODE(rqb_ptr))
    {
        case TSKMA_OPC_APP_SETUP:
        {
            tskma_task_app_setup();
            TSKMA_RQB_SET_REQ_FCT_PTR(rqb_ptr, tskma_task_stp_perform_cold_start_sm);
            TSKMA_OS_SEND_MAIL(TSKMA_TASK_NUM_STP, rqb_ptr);
            break;
        }
        case TSKMA_OPC_APP_INIT:
        {
            tskma_task_app_init();
            TSKMA_RQB_SET_REQ_FCT_PTR(rqb_ptr, tskma_task_stp_perform_cold_start_sm);
            TSKMA_OS_SEND_MAIL(TSKMA_TASK_NUM_STP, rqb_ptr);
            break;
        }
        case TSKMA_OPC_APP_OPEN:
        {
            tskma_data.cold_start.app_rqb_ptr = rqb_ptr;
            tskma_task_app_open();
            // ... retrigger via Pnpb.OpenDoneCbf -> tskma_task_app_pnpb_open_done
            break;
        }
        case TSKMA_OPC_APP_STARTUP_STOPPED:
        {
            tskma_task_app_startup_stopped();
            break;
        }
        case TSKMA_OPC_APP_INIT_EA:
        {
            tskma_task_app_cyclic_ea();
            TSKMA_RQB_SET_REQ_FCT_PTR(rqb_ptr, tskma_task_stp_perform_cold_start_sm);
            TSKMA_OS_SEND_MAIL(TSKMA_TASK_NUM_STP, rqb_ptr);
            break;
        }
        case TSKMA_OPC_APP_CYCLIC_EA:
        {
            tskma_task_app_cyclic_ea();
            // ... will be retriggered via tskma_isr_01()
            break;
        }
        case TSKMA_OPC_APP_TRIGGER_PNPB:
        {
            tskma_data.app.rqb.trigger_pndv_to_pnpb.static_rqb_in_use = LSA_FALSE;
            pnpb_perform_services();
            break;
        }
        case TSKMA_OPC_APP_TRIGGER_PNDV:
        {
            tskma_data.app.rqb.trigger_pnpb_to_pndv.static_rqb_in_use = LSA_FALSE;
            pndv_perform_services();
            break;
        }
        default:
        {
            /* opcode not supported */
            TSKMA_FATAL(TSKMA_RQB_GET_OPCODE(rqb_ptr));
        }
    }
}


//*----------------------------------------------------------------------------
//* tskma_task_app()
//*
//* TSKMA message handler for APP
//*----------------------------------------------------------------------------
OS_CODE_FAST PNIO_VOID tskma_task_app (PNIO_VOID)
{
#if DEBUG_TASKCNT
    LSA_UINT32 taskID;
#endif

    TSKMA_OS_WAIT_ON_TASKSTART();

#if DEBUG_TASKCNT
    taskID = TSKMA_GET_THREAD_ID();
#endif

    TSKMA_TASK_INIT_STATE(TSKMA_TASK_NUM_APP);

    for (;;)
    {
        union
        {
            TSKMA_MAIL_ELEM_S_PTR_T  rqb_ptr;
            TSKMA_VOID_PTR_TYPE      void_ptr;
        } msg;

        TSKMA_OS_READ_MAIL(&msg.void_ptr, TSKMA_TASK_NUM_APP);

        while ((msg.void_ptr))
        {
            LSA_RQB_GET_REQ_FCT_PTR(msg.rqb_ptr) (msg.void_ptr);

            TSKMA_OS_READ_MAIL(&msg.void_ptr, TSKMA_TASK_NUM_APP);

#if DEBUG_TASKCNT
            TaskCnt[taskID]++;
#endif
        }
    }
}

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
