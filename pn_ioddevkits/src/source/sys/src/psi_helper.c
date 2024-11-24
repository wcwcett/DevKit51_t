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
/*  F i l e               &F: psi_helper.c                              :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Implements PSI helper function for userr application.                    */
/*                                                                           */
/*****************************************************************************/

#define PSI_MODULE_ID      32005
#define LTRC_ACT_MODUL_ID  32005

#include "psi_helper.h"
#include "psi_inc.h"

#include "os_cfg.h"

// PSI_FILE_SYSTEM_EXTENSION(PSI_MODULE_ID)

#if (PSI_CFG_USE_HD_COMP == 1)
LSA_VOID psi_helper_isr_00(
    LSA_UINT16 const hd_nr,
    LSA_UINT32 const int_src)
{
#if (PSI_CFG_USE_EDDP == 1)
    PSI_EDD_HDDB hddb;
    psi_hd_get_edd_handle(hd_nr, &hddb);
    eddp_SetIRQ_WaitTime_to_MAX((EDDP_HANDLE)hddb, int_src); 
#endif
    psi_hd_interrupt(hd_nr, int_src);
}

OS_CODE_FAST LSA_VOID psi_isr_00(LSA_UINT32 int_source)
{
    psi_helper_isr_00(1/*hd_nr*/, 2);
}

OS_CODE_FAST LSA_VOID psi_isr_01(LSA_UINT32 int_source)
{
    psi_hd_interrupt(1/*hd_nr*/, 0); 
}

#endif // PSI_CFG_USE_HD_COMP


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
