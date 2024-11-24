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
/*  F i l e               &F: tskma_task_psi.c                          :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*   TSKMA message handler for PSI tasks                                     */
/*                                                                           */
/*****************************************************************************/

#include "tskma_inc.h"
#include "psi_inc.h"

#define TSKMA_MODULE_ID TSKMA_MODULE_ID_TSKMA_TASK_PSI


//*----------------------------------------------------------------------------
//*	tskma_task_psi()
//*
//* TSKMA message handler for internal PSI task
//*----------------------------------------------------------------------------
OS_CODE_FAST LSA_VOID tskma_task_psi(LSA_VOID)
{
    psi_thread_proc(0, PSI_PN_TASK_ID);
}

//*----------------------------------------------------------------------------
//* tskma_task_ld()
//*
//* TSKMA message handler for PSI LD tasks: CLRPC, IP2PN, SNMPX, SOCK, TCIP
//*----------------------------------------------------------------------------
OS_CODE_FAST LSA_VOID tskma_task_ld(LSA_VOID)
{
    psi_thread_proc(0, PSI_PN_LSA_LD_TASK_ID);
}

//*----------------------------------------------------------------------------
//* tskma_task_hd()
//*
//* TSKMA message handler for PSI HD tasks: ACP, CM
//*----------------------------------------------------------------------------
OS_CODE_FAST LSA_VOID tskma_task_hd(LSA_VOID)
{
    psi_thread_proc(0, PSI_PN_LSA_HD_TASK_ID);
}

//*----------------------------------------------------------------------------
//* tskma_task_org()
//*
//* TSKMA message handler for internal NRT ORG tasks: GSY, LLDP, MRP
//*----------------------------------------------------------------------------
OS_CODE_FAST LSA_VOID tskma_task_org(LSA_VOID)
{
    psi_thread_proc(0, PSI_NRT_ORG_TASK_ID);
}

//*----------------------------------------------------------------------------
//* tskma_task_pof()
//*
//* TSKMA message handler for POF task
//*----------------------------------------------------------------------------
OS_CODE_FAST LSA_VOID tskma_task_pof(LSA_VOID)
{
    psi_thread_proc(0, PSI_POF_TASK_ID);
}

//*----------------------------------------------------------------------------
//* tskma_task_eddp()
//*
//* TSKMA message handler for EDDP task
//*----------------------------------------------------------------------------
OS_CODE_FAST LSA_VOID tskma_task_eddp(LSA_VOID)
{
    psi_thread_proc(0, PSI_EDDP_L1_TASK_ID);
}


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
