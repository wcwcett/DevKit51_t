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
/*  F i l e               &F: tskma_ext.c                               :F&  */
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
/*
 * tskma_ext.c
 *
 */

#include "tskma_inc.h"
#include "tskma_int.h"

#include "tskma_dat.h"

#define TSKMA_MODULE_ID TSKMA_MODULE_ID_TSKMA

PNIO_VOID  TSKMA_TASK_INIT_STATE (PNIO_UINT32 TaskNum)
{
    while(!tskma_data.cold_start.done)
    {
        union
        {
            TSKMA_MAIL_ELEM_S_PTR_T     rqb_ptr;
            TSKMA_VOID_PTR_TYPE         void_ptr;
        }msg;
        OsReadMessageBlocked(&msg.void_ptr, tskma_com_data.task_info[TaskNum].task_id);

        ( (msg.rqb_ptr)->_req_fct ) (msg.void_ptr);
    }
}


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
