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
/*  F i l e               &F: pnpb_peri_real.h                          :F&  */
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
#ifndef PNPB_PERI_REAL_H
#define PNPB_PERI_REAL_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

PNIO_VOID   PnpbRealCfgInit     (PNIO_VOID);

PNIO_VOID   PnpbRealCfgFree     (PNIO_UINT32 EntityIndex);

PNIO_UINT32 PnpbRealCfgOccupy   (PNIO_UINT32 PrefIndex);

PNIO_UINT32 PnpbRealCfgGetEntityInd (PNIO_UINT32  Api,     // api number
                                     PNIO_UINT32  Slot,    // slot number
                                     PNIO_UINT32  Sub);    // subslot number

PNIO_UINT32 PnpbRealCfgGetApiSlotSubslot   (PNIO_UINT32 EntityInd, 
                                            PNIO_UINT32*pApi,
                                            PNIO_UINT32 *pSlot, 
                                            PNIO_UINT32 *pSub);

PNIO_BOOL   PnpbAlarmSubPlug (PNIO_UINT32          Ind,
                              PNIO_UINT32          api,
                              PNIO_UINT32          slot_nr,
                              PNIO_UINT32          subslot_nr,
                              PNIO_UINT32          modul_ident,
                              PNIO_UINT32          submodul_ident,
                              PNIO_IM0_SUPP_ENUM   Im0Support,
                              PNIO_BOOL            more_follows);

PNIO_UINT32 PnpbAlarmSubPlugGetState (PNIO_UINT32 Index);   // index in real-cfg list in perif

PNIO_UINT32 PnpbAlarmSubPull (PNIO_UINT32   Api,            // api number
                              PNIO_UINT32   Slot,           // slot number
                              PNIO_UINT32   Sub,            // subslot number
                              PNIO_BOOL     more_follows);  // more follows PNIO_TRUE/PNIO_FALSE


#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
