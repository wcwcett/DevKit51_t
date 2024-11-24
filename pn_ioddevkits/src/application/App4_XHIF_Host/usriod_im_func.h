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
/*  F i l e               &F: usriod_im_func.h                          :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  header file to usriod_im_func.c.                                         */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/
#ifndef _USR_IOD_IM_FUNC_H
#define _USR_IOD_IM_FUNC_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

#include "pnpb_lib.h"

PNIO_VOID UsrIod_BuildDeviceAnnotation (void);

    
PNIO_UINT32  Im0_read_Handler 
		(
	            PNIO_UINT32         Api,              // api number
	            PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
	            PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
	            PNIO_UINT8          *pBuffer,         // [in] buffer pointer
	            PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
	            PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
		);

PNIO_UINT32  Im1_read_Handler 
		(
	            PNIO_UINT32         Api,              // api number
	            PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
	            PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
	            PNIO_UINT8          *pBuffer,         // [in] buffer pointer
	            PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
	            PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
		);

PNIO_UINT32  Im2_read_Handler 
		(
	            PNIO_UINT32         Api,              // api number
	            PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
	            PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
	            PNIO_UINT8          *pBuffer,         // [in] buffer pointer
	            PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
	            PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
		);

PNIO_UINT32  Im3_read_Handler 
		(
	            PNIO_UINT32         Api,              // api number
	            PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
	            PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
	            PNIO_UINT8          *pBuffer,         // [in] buffer pointer
	            PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
	            PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
		);

PNIO_UINT32  Im4_read_Handler 
		(
	            PNIO_UINT32         Api,              // api number
	            PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
	            PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
	            PNIO_UINT8          *pBuffer,         // [in] buffer pointer
	            PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
	            PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
		);

PNIO_UINT32  Im0_write_Handler 
		(
	            PNIO_UINT32         Api,             // api number
	            PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
	            PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
	            PNIO_UINT8          *pBuffer,         // [in] buffer pointer
	            PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
	            PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
		);

PNIO_UINT32  Im1_write_Handler 
		(
	            PNIO_UINT32         Api,              // api number
	            PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
	            PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
	            PNIO_UINT8          *pBuffer,         // [in] buffer pointer
	            PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
	            PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
		);

PNIO_UINT32  Im2_write_Handler 
		(
	            PNIO_UINT32         Api,             // api number
	            PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
	            PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
	            PNIO_UINT8          *pBuffer,         // [in] buffer pointer
	            PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
	            PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
		);

PNIO_UINT32  Im3_write_Handler 
		(
	            PNIO_UINT32         Api,              // api number
	            PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
	            PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
	            PNIO_UINT8          *pBuffer,         // [in] buffer pointer
	            PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
	            PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
		);

PNIO_UINT32  Im4_write_Handler 
		(
	            PNIO_UINT32         Api,            // api number
	            PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
	            PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
	            PNIO_UINT8          *pBuffer,         // [in] buffer pointer
	            PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
	            PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
		);

PNIO_VOID  ImX_write_Handler_done
        (
                PNIO_UINT32 IMidx,
                PNIO_UINT32 DatLen,
                PNIO_UINT32 Status
        );

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
