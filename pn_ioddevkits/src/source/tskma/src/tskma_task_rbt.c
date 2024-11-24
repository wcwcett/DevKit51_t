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
/*  F i l e               &F: tskma_task_rbt.c                          :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*   TSKMA message handler for Reboot task                                   */
/*                                                                           */
/*****************************************************************************/

#include "tskma_inc.h"

#define TSKMA_MODULE_ID TSKMA_MODULE_ID_TSKMA_TASK_RBT

#if DEBUG_TASKCNT
extern PNIO_UINT32      TaskCnt[MAXNUM_OF_TASKS];
#endif


//*----------------------------------------------------------------------------
//*    tskma_task_rbt_request()
//*
//* Service reboot messages
//*----------------------------------------------------------------------------
PNIO_VOID tskma_task_rbt_request(TSKMA_RQB_S_PTR_T rqb_ptr)
{
    switch (TSKMA_RQB_GET_OPCODE(rqb_ptr))
    {
        case TSKMA_OPC_RBT_REQ_REBOOT:
        {
            TSKMA_REBOOT_SERVICE();
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
//*    tskma_task_rbt()
//*
//* TSKMA message handler for reboot
//*----------------------------------------------------------------------------
OS_CODE_FAST PNIO_VOID tskma_task_rbt(PNIO_VOID)
{
#if DEBUG_TASKCNT
    LSA_UINT32  taskID;
#endif

    TSKMA_OS_WAIT_ON_TASKSTART();

#if DEBUG_TASKCNT
    taskID = TSKMA_GET_THREAD_ID();
#endif

    TSKMA_TASK_INIT_STATE(TSKMA_TASK_NUM_RBT);

    for (;;)
    {
        union
        {
            TSKMA_MAIL_ELEM_S_PTR_T  rqb_ptr;
            TSKMA_VOID_PTR_TYPE      void_ptr;
        }msg;

        TSKMA_OS_READ_MAIL(&msg.void_ptr, TSKMA_TASK_NUM_RBT);

        while ((msg.void_ptr))
        {
            LSA_RQB_GET_REQ_FCT_PTR(msg.rqb_ptr) (msg.void_ptr);

            TSKMA_OS_READ_MAIL(&msg.void_ptr, TSKMA_TASK_NUM_RBT);
#if DEBUG_TASKCNT
            TaskCnt[taskID]++;
#endif
        }

        TSKMA_WAKE_NEXT_TS_TASK(TSKMA_TASK_NUM_RBT);
    }
}


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
