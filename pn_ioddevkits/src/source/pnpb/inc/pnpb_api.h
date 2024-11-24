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
/*  F i l e               &F: pnpb_api.h                                :F&  */
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
#ifndef PNPB_API_H
#define PNPB_API_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif


// * --------------------------------------------
// *  macros 
// * --------------------------------------------

// *** PNPB_DIAG_CHANNEL_PROPERTIES_MAKE must correspond to CM_DIAG_CHANNEL_PROPERTIES_MAKE !! ***
#define PNPB_DIAG_CHANNEL_PROPERTIES_MAKE(type, acc, m_req, m_dem, spec, dir) ((LSA_UINT16) \
	( (((type)  & 0xFF) <<  0) /* mask 0x00FF   DIAG_CHANPROP_TYPE_BYTE, ....*/ \
	| (((acc)   & 0x01) <<  8) /* mask 0x0100   accumulative            */ \
	| (((m_req) & 0x01) <<  9) /* mask 0x0200   maintenance required    */ \
	| (((m_dem) & 0x01) << 10) /* mask 0x0400   maintenance demanded    */ \
	| (((spec)  & 0x03) << 11) /* mask 0x1800   DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE*/ \
	| (((dir)   & 0x07) << 13) /* mask 0xE000   DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT */ \
	))



PNIO_VOID PnpbSetLastError (PNIO_ERR_ENUM LastErr);


#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif



#endif


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
