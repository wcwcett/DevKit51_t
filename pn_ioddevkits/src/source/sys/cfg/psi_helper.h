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
/*  F i l e               &F: psi_helper.h                              :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Defines PSI helper function for userr application                        */
/*                                                                           */
/*****************************************************************************/

#ifndef PSI_HELPER_H                     /* ----- reinclude-protection ----- */
#define PSI_HELPER_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

#if (PSI_CFG_USE_HD_COMP == 1)

#define PSI_INIT_ISR()                               \
{                                                    \
    Bsp_ErtecSwiIntConnect (psi_isr_00, psi_isr_01 );\
};

LSA_VOID psi_helper_isr_00(
    LSA_UINT16 const hd_nr,
    LSA_UINT32 const int_src);

LSA_VOID psi_isr_00(
    LSA_UINT32 int_source
);

LSA_VOID psi_isr_01(
    LSA_UINT32 int_source
);

#endif // PSI_CFG_USE_HD_COMP


#endif // of PSI_HELPER_H
/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
