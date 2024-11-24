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
/*  F i l e               &F: tskma_com.h                               :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*   TSKMA interface header                                                  */
/*                                                                           */
/*****************************************************************************/

#ifndef TSKMA_COM_H_
#define TSKMA_COM_H_


/* defining the type of a mail element */

typedef LSA_VOID_PTR_TYPE TSKMA_VOID_PTR_TYPE;

typedef struct TSKMA_MAIL_ELEM_S* TSKMA_MAIL_ELEM_S_PTR_T;

typedef struct TSKMA_MAIL_ELEM_S
{
    TSKMA_RQB_HEADER
    /* don't use any more elements, this is only intended to access the rqb header
     * other elements are private !!
     */
} TSKMA_MAIL_ELEM_S_T;


typedef struct TSKMA_RQB_S* TSKMA_RQB_S_PTR_T;

typedef struct TSKMA_RQB_S
{
    TSKMA_RQB_HEADER
    LSA_BOOL  static_rqb_in_use; // can be used with static rqbs to indicate that it is in use
    TSKMA_RQB_TRAILER
} TSKMA_RQB_S_T;

//=================================================================================================
#define TSKMA_TASK_NUM_APP                    0 /* application task */
#define TSKMA_TASK_ID_APPLICATION             tskma_com_data.task_info[TSKMA_TASK_NUM_APP].task_id
//-------------------------------------------------------------------------------------------------
// !!! IMPORTANT: all tasks below the application task are time slicing tasks if enabled !!!
//-------------------------------------------------------------------------------------------------
#define TSKMA_TASK_NUM_ORG                    1 /* NRT-ORG task */
#define TSKMA_TASK_ID_ORG                     tskma_com_data.task_info[TSKMA_TASK_NUM_ORG].task_id
//-------------------------------------------------------------------------------------------------
#define TSKMA_TASK_NUM_EDDP                   2 /* EDDP task */
#define TSKMA_TASK_ID_EDDP                    tskma_com_data.task_info[TSKMA_TASK_NUM_EDDP].task_id
//-------------------------------------------------------------------------------------------------
#define TSKMA_TASK_NUM_LD                     3 /* PSI LD task */
#define TSKMA_TASK_ID_LD                      tskma_com_data.task_info[TSKMA_TASK_NUM_LD].task_id
//-------------------------------------------------------------------------------------------------
#define TSKMA_TASK_NUM_HD                     4 /* PSI HD task */
#define TSKMA_TASK_ID_HD                      tskma_com_data.task_info[TSKMA_TASK_NUM_HD].task_id
//-------------------------------------------------------------------------------------------------
#define TSKMA_TASK_NUM_PSI                    5 /* PSI task */
#define TSKMA_TASK_ID_PSI                     tskma_com_data.task_info[TSKMA_TASK_NUM_PSI].task_id
//-------------------------------------------------------------------------------------------------
#define TSKMA_TASK_NUM_POF                    6 /* POF task */
#define TSKMA_TASK_ID_POF                     tskma_com_data.task_info[TSKMA_TASK_NUM_POF].task_id
//-------------------------------------------------------------------------------------------------
#define TSKMA_TASK_NUM_PNO                    7 /* PNIO task  */
#define TSKMA_TASK_ID_PNIO                    tskma_com_data.task_info[TSKMA_TASK_NUM_PNO].task_id
//-------------------------------------------------------------------------------------------------
#define TSKMA_TASK_NUM_RBT                    8 /* Reboot task */
#define TSKMA_TASK_ID_REBOOT                  tskma_com_data.task_info[TSKMA_TASK_NUM_RBT].task_id
//-------------------------------------------------------------------------------------------------
#define TSKMA_TASK_NUM_STP                    9 /* Startup and control task */
#define TSKMA_TASK_ID_STARTUP                 tskma_com_data.task_info[TSKMA_TASK_NUM_STP].task_id
//-------------------------------------------------------------------------------------------------
#define TSKMA_TASK_NUM_IDL                    10 /* Idle task */
#define TSKMA_TASK_ID_IDLE                    tskma_com_data.task_info[TSKMA_TASK_NUM_IDL].task_id
//-------------------------------------------------------------------------------------------------
#define TSKMA_TASK_NUM                        11
//=================================================================================================


typedef struct TSKMA_COM_TASKINFO_S
{
    LSA_UINT32  task_id;                /* must be a unique id given by os (or os adaption) at task creation */
    LSA_UINT32  task_prio;              /* priority of the task  */
    LSA_VOID    (*task_ptr)(LSA_VOID);  /* pointer to the task function */
    LSA_CHAR*   task_name_ptr;          /* task name (optional, max. 20 bytes */
    LSA_UINT32  task_stack_size;        /* stack size of the task */

} TSKMA_COM_TASKINFO_TYPE;

typedef struct TSKMA_COM_DATA_S
{
    TSKMA_COM_TASKINFO_TYPE task_info[TSKMA_TASK_NUM];
} TSKMA_COM_DATA_TYPE;

typedef TSKMA_COM_DATA_TYPE* TSKMA_COM_DATA_TYPE_PTR;
TSKMA_COM_DATA_TYPE  tskma_com_data;

#define TSKMA_RET_ERROR     (LSA_UINT32)0x00000000
#define TSKMA_RET_OK        (LSA_UINT32)0x00000001

LSA_VOID    tskma_parity_error_isr(LSA_UINT32 int_source);
LSA_VOID    tskma_access_error_isr(LSA_UINT32 int_source);

LSA_VOID    tskma_init(LSA_VOID);
LSA_UINT32  tskma_open(LSA_VOID);

LSA_VOID    tskma_task_app_send_pndv_trigger(LSA_VOID);
LSA_VOID    tskma_task_app_send_pnpb_trigger(LSA_VOID);
LSA_VOID    tskma_task_app_pnpb_open_done   (LSA_VOID);

LSA_VOID    tskma_task_stp_perform_cold_start_sm(LSA_VOID);


#endif /* TSKMA_COM_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
