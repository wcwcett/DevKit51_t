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
/*  F i l e               &F: tskma_task_idl.c                          :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*   TSKMA message handler for IDLE task                                     */
/*                                                                           */
/*****************************************************************************/

#include "tskma_inc.h"

#define TSKMA_MODULE_ID TSKMA_MODULE_ID_TSKMA_TASK_IDL

#if DEBUG_TASKCNT
extern PNIO_UINT32  TaskCnt[MAXNUM_OF_TASKS];
#endif


/*----------------------------------------------------------------------------
 *  tskma_task_idl()
 *
 *  @brief TSKMA message handler for IDLE task
 *
 *  The tskma idle task, as the task with the lowest priority, is used to bring
 *  all other tasks to life and initiate the startup process.
 *  Using a dynamic OS the other tasks are created.
 *  Using a static OS all other tasks are already running and is assumed that
 *  the idle task is the last task to be established.
 *  In any of the above cases the higher tasks would be suspendet waiting on there
 *  empty mailboxes.
 *  The only thing doing in this task is to send a trigger mail to STP task.
 *----------------------------------------------------------------------------*/
PNIO_VOID tskma_task_idl (PNIO_VOID)
{
    LSA_UINT32 ret_val;
    LSA_UINT32 task_count;
#if DEBUG_TASKCNT
    LSA_UINT32 taskID;
#endif

    TSKMA_OS_WAIT_ON_TASKSTART();
#if DEBUG_TASKCNT
    taskID = TSKMA_GET_THREAD_ID();
#endif

    /* perform cold start */

    // now start all tasks beginng with the highest priority
    // starting the last task (stp-task) will bring up the system
    for (task_count = 0; task_count < (TSKMA_TASK_NUM - 1/* IDLE TASK already started */); task_count++)
    {
        TSKMA_OS_TASK_CREATE(  tskma_com_data.task_info[task_count].task_ptr,
                               tskma_com_data.task_info[task_count].task_prio,
                              &tskma_com_data.task_info[task_count].task_id,
                               tskma_com_data.task_info[task_count].task_name_ptr,
                               tskma_com_data.task_info[task_count].task_stack_size,
                              &ret_val);
        TSKMA_CHECK_FATAL(ret_val);
    }

    // create message queues, before all tasks are started. So messages can already be stored, before
    // the destination task is running.
    TSKMA_OS_MBOX_CREATE(tskma_com_data.task_info[TSKMA_TASK_NUM_APP].task_id, TSKMA_TASK_NUM_APP);
    TSKMA_OS_MBOX_CREATE(tskma_com_data.task_info[TSKMA_TASK_NUM_RBT].task_id, TSKMA_TASK_NUM_RBT);
    TSKMA_OS_MBOX_CREATE(tskma_com_data.task_info[TSKMA_TASK_NUM_PNO].task_id, TSKMA_TASK_NUM_PNO);
    TSKMA_OS_MBOX_CREATE(tskma_com_data.task_info[TSKMA_TASK_NUM_STP].task_id, TSKMA_TASK_NUM_STP);

    TSKMA_PSI_SYSTEM_INIT(); // system startup initialization of PSI

    for (task_count = 0; task_count < (TSKMA_TASK_NUM - 1); task_count++)
    {
        TSKMA_OS_TASK_START(tskma_com_data.task_info[task_count].task_id, &ret_val);
    }

    tskma_data.cold_start.done = LSA_TRUE;

    /* we'll should never get here */
    for (;;)
    {
        TSKMA_IDLE_LOOP_CHECK();
#if DEBUG_TASKCNT
        TaskCnt[taskID]++;
#endif
    }
}

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
