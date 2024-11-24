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
/*  F i l e               &F: tskma_task_stp.c                          :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*   TSKMA message handler for STARTUP task                                  */
/*                                                                           */
/*****************************************************************************/

#include "tskma_inc.h"

#define TSKMA_MODULE_ID  TSKMA_MODULE_ID_TSKMA_TASK_STP

#if DEBUG_TASKCNT
extern PNIO_UINT32 TaskCnt[MAXNUM_OF_TASKS];
#endif

static LSA_UINT32 TskmaSemId = 0;


//*----------------------------------------------------------------------------
//* tskma_task_stp_perform_cold_start()
//*
//* Initially trigger cold start state machine
//*----------------------------------------------------------------------------
PNIO_VOID tskma_task_stp_perform_cold_start(PNIO_VOID)
{
    tskma_data.cold_start.sm = TSKMA_COLD_START_SM_ALLOC;
    tskma_task_stp_perform_cold_start_sm();
}


//*----------------------------------------------------------------------------
//* tskma_task_stp_perform_cold_start_sm()
//*
//* Cold start state machine
//*----------------------------------------------------------------------------
PNIO_VOID tskma_task_stp_perform_cold_start_sm(PNIO_VOID)
{
    switch(tskma_data.cold_start.sm)
    {
        case TSKMA_COLD_START_SM_ALLOC:
        {
            LSA_UINT32 ret_val;
            tskma_data.cold_start.sm = TSKMA_COLD_START_SM_APP_SETUP;
            TSKMA_OS_SEM_B_ALLOC(&TskmaSemId, ret_val); // allocate semaphore
            TSKMA_CHECK_FATAL(ret_val);
        }
        // no break
        //lint -fallthrough
        case TSKMA_COLD_START_SM_APP_SETUP:
        {
            tskma_data.cold_start.sm = TSKMA_COLD_START_SM_OPEN_PNIO_STACK;
            TSKMA_RQB_SET_OPCODE(&tskma_data.cold_start.tskma_rqb, TSKMA_OPC_APP_SETUP);
            TSKMA_RQB_SET_REQ_FCT_PTR(&tskma_data.cold_start.tskma_rqb, tskma_task_app_request);
            TSKMA_OS_SEND_MAIL(TSKMA_TASK_NUM_APP, &tskma_data.cold_start.tskma_rqb);
            break;
        }
        case TSKMA_COLD_START_SM_OPEN_PNIO_STACK:
        {
            tskma_data.cold_start.sm = TSKMA_COLD_START_SM_APP_INIT;
            TSKMA_RQB_SET_OPCODE(&tskma_data.cold_start.tskma_rqb, TSKMA_OPC_PNO_REQ_OPEN);
            TSKMA_RQB_SET_REQ_FCT_PTR(&tskma_data.cold_start.tskma_rqb, tskma_task_pno_request);
            TSKMA_OS_SEND_MAIL(TSKMA_TASK_NUM_PNO, &tskma_data.cold_start.tskma_rqb);
            break;
        }
        case TSKMA_COLD_START_SM_APP_INIT:
        {
            tskma_data.cold_start.sm = TSKMA_COLD_START_SM_APP_OPEN;
            TSKMA_RQB_SET_OPCODE(&tskma_data.cold_start.tskma_rqb, TSKMA_OPC_APP_INIT);
            TSKMA_RQB_SET_REQ_FCT_PTR(&tskma_data.cold_start.tskma_rqb, tskma_task_app_request);
            TSKMA_OS_SEND_MAIL(TSKMA_TASK_NUM_APP, &tskma_data.cold_start.tskma_rqb);
            break;
        }
        case TSKMA_COLD_START_SM_APP_OPEN:
        {
            tskma_data.cold_start.sm = TSKMA_COLD_START_SM_APP_CYCLIC_EA;
            TSKMA_RQB_SET_OPCODE(&tskma_data.cold_start.tskma_rqb, TSKMA_OPC_APP_OPEN);
            TSKMA_RQB_SET_REQ_FCT_PTR(&tskma_data.cold_start.tskma_rqb, tskma_task_app_request);
            TSKMA_OS_SEND_MAIL(TSKMA_TASK_NUM_APP, &tskma_data.cold_start.tskma_rqb);
            break;
        }
        case TSKMA_COLD_START_SM_APP_CYCLIC_EA:
        {
            tskma_data.cold_start.sm = TSKMA_COLD_START_SM_DONE;
            // init cyclic EA request
            TSKMA_RQB_SET_OPCODE    (&tskma_data.rqb.cyclic_ea_rqb, TSKMA_OPC_APP_CYCLIC_EA);
            TSKMA_RQB_SET_REQ_FCT_PTR(&tskma_data.rqb.cyclic_ea_rqb, tskma_task_app_request);
            // trigger initial pndv and pnpb perform service
            TSKMA_RQB_SET_OPCODE    (&tskma_data.cold_start.tskma_rqb, TSKMA_OPC_APP_INIT_EA);
            TSKMA_RQB_SET_REQ_FCT_PTR(&tskma_data.cold_start.tskma_rqb, tskma_task_app_request);
            TSKMA_OS_SEND_MAIL(TSKMA_TASK_NUM_APP, &tskma_data.cold_start.tskma_rqb);
            break;
        }
        case TSKMA_COLD_START_SM_DONE:
        {
            /* after initialization all volatile options are expected to be performed */

            /* startup was observed by watchdog and has been completed at this point */
            break;
        }
        case TSKMA_COLD_START_SM_CLOSED:
        default:
        {
            TSKMA_FATAL(tskma_data.cold_start.sm);
        }
    }
}


//*----------------------------------------------------------------------------
//* tskma_task_stp()
//*
//* TSKMA message handler for STARTUP task
//*----------------------------------------------------------------------------
PNIO_VOID tskma_task_stp (PNIO_VOID)
{
#if DEBUG_TASKCNT
    LSA_UINT32 taskID;
#endif

    TSKMA_OS_WAIT_ON_TASKSTART();

#if DEBUG_TASKCNT
    taskID = TSKMA_GET_THREAD_ID();
#endif

    // startup the device ...
    tskma_task_stp_perform_cold_start();
    // ... startup done when reaching here

    TSKMA_TASK_INIT_STATE(TSKMA_TASK_NUM_STP);

    for (;;)
    {
        union
        {
            TSKMA_MAIL_ELEM_S_PTR_T     rqb_ptr;
            TSKMA_VOID_PTR_TYPE         void_ptr;
        } msg;

        TSKMA_OS_READ_MAIL(&msg.void_ptr, TSKMA_TASK_NUM_STP);

        while ((msg.void_ptr))
        {
            LSA_RQB_GET_REQ_FCT_PTR(msg.rqb_ptr) (msg.void_ptr);

            TSKMA_OS_READ_MAIL(&msg.void_ptr, TSKMA_TASK_NUM_STP);

#if DEBUG_TASKCNT
            TaskCnt[taskID]++;
#endif
        }

        TSKMA_WAKE_NEXT_TS_TASK(TSKMA_TASK_NUM_STP);
    }
}

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
