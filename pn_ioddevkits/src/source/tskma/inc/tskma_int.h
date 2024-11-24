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
/*  F i l e               &F: tskma_int.h                               :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*   TSKMA internal header                                                   */
/*                                                                           */
/*****************************************************************************/


#ifndef TSKMA_INT_H_
#define TSKMA_INT_H_

//*----------------------------------------------------------------------------
//* opcodes
//*----------------------------------------------------------------------------

#define TSKMA_OPC_APP_SETUP             0x00
#define TSKMA_OPC_APP_INIT              0x01
#define TSKMA_OPC_APP_OPEN              0x02
#define TSKMA_OPC_APP_STARTUP_STOPPED   0x03
#define TSKMA_OPC_APP_CYCLIC_EA         0x04
#define TSKMA_OPC_APP_TRIGGER_PNPB      0x05
#define TSKMA_OPC_APP_TRIGGER_PNDV      0x06
#define TSKMA_OPC_APP_INIT_EA           0x07

#define TSKMA_OPC_RBT_REQ_REBOOT        0x08
#define TSKMA_OPC_PNO_REQ_OPEN          0x09


#define TSKMA_MODULE_ID_TSKMA_PERI_ISR  0x01
#define TSKMA_MODULE_ID_TSKMA_TASK_APP  0x02
#define TSKMA_MODULE_ID_TSKMA_TASK_RBT  0x03
#define TSKMA_MODULE_ID_TSKMA_TASK_IDL  0x04
#define TSKMA_MODULE_ID_TSKMA_TASK_PNO  0x06
#define TSKMA_MODULE_ID_TSKMA_TASK_PSI  0x07
#define TSKMA_MODULE_ID_TSKMA_TASK_STP  0x09
#define TSKMA_MODULE_ID_TSKMA           0x0B
#define TSKMA_MODULE_ID_TSKMA_PDB       0x0C
#define TSKMA_MODULE_ID_TSKMA_DAT       0x0D


//*----------------------------------------------------------------------------
//* types
//*----------------------------------------------------------------------------

typedef enum TSKMA_COLD_START_SM_E
{
    TSKMA_COLD_START_SM_CLOSED,
    TSKMA_COLD_START_SM_ALLOC,
    TSKMA_COLD_START_SM_APP_SETUP,
    TSKMA_COLD_START_SM_APP_INIT,
    TSKMA_COLD_START_SM_OPEN_PNIO_STACK,
    TSKMA_COLD_START_SM_APP_OPEN,
    TSKMA_COLD_START_SM_APP_CYCLIC_EA,
    TSKMA_COLD_START_SM_DONE,

} TSKMA_COLD_START_SM_E_T;


typedef enum TSKMA_TS_SCHEDULING_STATE_E
{
    TSKMA_TS_SCHEDULING_STATE_APPLICATION_TASK_RUNNING,
    TSKMA_TS_SCHEDULING_STATE_APPLICATION_TASK_FINISHED,
    TSKMA_TS_SCHEDULING_STATE_APPLICATION_TASK_SUSPENDED,
    TSKMA_TS_SCHEDULING_STATE_TS_TASK_FINISHED

} TSKMA_TS_SCHEDULING_STATE_E_T;


//*----------------------------------------------------------------------------
//* prototypes
//*----------------------------------------------------------------------------

LSA_VOID    tskma_pdb_setup_pndv (LSA_VOID);
LSA_VOID    tskma_pdb_open       (LSA_VOID);

LSA_VOID    tskma_task_app_setup           (LSA_VOID);
LSA_VOID    tskma_task_app_init            (LSA_VOID);
LSA_VOID    tskma_task_app_open            (LSA_VOID);
LSA_VOID    tskma_task_app_startup_stopped (LSA_VOID);
LSA_VOID    tskma_task_app_cyclic_ea       (LSA_VOID);

LSA_VOID    tskma_task_app_request (TSKMA_RQB_S_PTR_T rqb_ptr);
LSA_VOID    tskma_task_rbt_request (TSKMA_RQB_S_PTR_T rqb_ptr);
LSA_VOID    tskma_task_pno_request (TSKMA_RQB_S_PTR_T rqb_ptr);

LSA_VOID    tskma_task_app (LSA_VOID);
LSA_VOID    tskma_task_rbt (LSA_VOID);
LSA_VOID    tskma_task_idl (LSA_VOID);
LSA_VOID    tskma_task_pno (LSA_VOID);
LSA_VOID    tskma_task_psi (LSA_VOID);
LSA_VOID    tskma_task_ld  (LSA_VOID);
LSA_VOID    tskma_task_hd  (LSA_VOID);
LSA_VOID    tskma_task_org (LSA_VOID);
LSA_VOID    tskma_task_pof (LSA_VOID);
LSA_VOID    tskma_task_eddp(LSA_VOID);
LSA_VOID    tskma_task_stp (LSA_VOID);

LSA_VOID    tskma_fatal (LSA_UINT16 tskma_module_id, LSA_INT line, LSA_UINT32 ec_0);

#endif /* TSKMA_INT_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
