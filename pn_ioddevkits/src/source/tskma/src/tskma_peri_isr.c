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
/*  F i l e               &F: tskma_peri_isr.c                          :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*   TSKMA interrupt service routines for PERI                               */
/*                                                                           */
/*****************************************************************************/

#include "tskma_inc.h"

#define TSKMA_MODULE_ID     TSKMA_MODULE_ID_TSKMA_PERI_ISR

//*============================================================================
//*=====     PERI     =========================================================
//*============================================================================
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P) /* PNIP */

//*----------------------------------------------------------------------------
//*    tskma_parity_error_isr()
//*
//* @brief PERI parity error ISR
//*----------------------------------------------------------------------------
PNIO_VOID tskma_parity_error_isr(LSA_UINT32 int_source)
{
    tskma_parity_error_count++;
    tskma_parity_error_source |= int_source;
    //tskma_fatal (TSKMA_MODULE_ID, __LINE__, int_source);
}


//*----------------------------------------------------------------------------
//*    tskma_access_error_isr()
//*
//* @brief PERI data access error ISR
//*----------------------------------------------------------------------------
PNIO_VOID tskma_access_error_isr(LSA_UINT32 int_source)
{
    tskma_access_error_count++;
    tskma_access_error_source |= int_source;
    //tskma_fatal (TSKMA_MODULE_ID, __LINE__, int_source);
}

#endif // (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
