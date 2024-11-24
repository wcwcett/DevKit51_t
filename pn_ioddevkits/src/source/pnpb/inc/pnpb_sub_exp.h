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
/*  F i l e               &F: pnpb_sub_exp.h                            :F&  */
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
#ifndef PNPB_SUB_EXP_H
#define PNPB_SUB_EXP_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif


// *-------------------------------------------------
// *    PNPB internal functions
// *-------------------------------------------------
PNIO_VOID       pnpb_sub_exp_init(PNIO_VOID);

PNIO_EXP_SUB*   pnpb_sub_exp_oneAr_getp (PNIO_UINT32   ArNum,
                                         PNIO_UINT32   ApiNum, 
                                         PNIO_UINT32   SlotNum, 
                                         PNIO_UINT32   SubNum);

PNIO_EXP_SUB*   pnpb_sub_exp_allAr_getp (PNIO_UINT32*  pArNum,
                                         PNIO_UINT32   ApiNum, 
                                         PNIO_UINT32   SlotNum, 
                                         PNIO_UINT32   SubNum);

// *-------------------------------------------------
// *    public functions
// *-------------------------------------------------
PNIO_UINT32 pnpb_sub_exp_add (PNIO_UINT32 ArInd,                // [in]
                              PNIO_UINT32 EntityIndSetCfg,      // [in]
                              PNDV_SET_CFG_ELEMENT_T*  pElem,   // [in]
                              PNIO_UINT32 Flags,                // [in]
                              PNIO_EXP_SUB** ppSubExp,          // [in, out]
                              PNIO_BOOL  MoreFollows);          // [in]

PNIO_BOOL pnpb_remove_all_sub  (PNIO_UINT32 ArNum);


#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif



#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
