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
/*  F i l e               &F: pnpb_rec8028_8029.h                       :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  header file to pnpb_im_func.c.                                           */
/*                                                                           */
/*****************************************************************************/
#ifndef _PNPB_REC_8028_8029_H
#define _PNPB_REC_8028_8029_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

#define PNPB_REC_8028_8029_DISCARD_IOXS_MASK                   0x0020
#define PNPB_REC_8028_8029_DISCARD_IOXS_SUPPORTED              0x0020

PNIO_UINT32  pnpb_rec8028_Handler
        (
            PNIO_UINT32         ArIndExp,       // expected AR-index from the record read request index 0x8029
            PNIO_UINT32         Api,            // api number
            PNIO_DEV_ADDR       *pAddr,         // geographical or logical address
            PNIO_UINT32         *pBufLen,       // [in, out] in: length to read, out: length, read by user  
            PNIO_UINT8          *pBuffer,       // [in] buffer pointer
            PNIO_ERR_STAT       *pPnioState     // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
        );

PNIO_UINT32  pnpb_rec8029_Handler
        (
            PNIO_UINT32         ArIndExp,       // expected AR-index from the record read request index 0x8029
            PNIO_UINT32         Api,            // api number
            PNIO_DEV_ADDR       *pAddr,         // geographical or logical address
            PNIO_UINT32         *pBufLen,       // [in, out] in: length to read, out: length, read by user  
            PNIO_UINT8          *pBuffer,       // [in] buffer pointer
            PNIO_ERR_STAT       *pPnioState     // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
        );

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
