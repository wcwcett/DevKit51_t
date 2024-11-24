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
/*  F i l e               &F: pnpb_rec.h                                :F&  */
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
#ifndef PNPB_REC_H
#define PNPB_REC_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

PNIO_VOID  pnpb_rec_read_write  (PNIO_UINT8         Add1,
                                 PNIO_UINT16        Add2,
                                 PNDV_IFACE_STRUCT* pPndvIf);


// *-------------------------------------------------
// *    asynchronous record read, record write response
// *-------------------------------------------------
PNIO_UINT32  pnpb_rec_read_rsp
         (
            PNIO_VOID*          pRqHnd,         // request handle from PNIO_cbf_rec_read
            PNIO_UINT8*         pDat,           // data pointer (may be 0 if error)
            PNIO_UINT32         NettoDatLength, // total length of read record data
            PNIO_ERR_STAT*      pPnioStat       // PNIO state pointer (may be 0, if no error)
         );

PNIO_UINT32  pnpb_rec_write_rsp
         (
            PNIO_VOID*          pRqHnd,         // request handle from PNIO_cbf_rec_read
            PNIO_UINT32         NettoDatLength, // total length of written record data
            PNIO_ERR_STAT*      pPnioStat       // PNIO state pointer (may be 0, if no error)
         );

PNIO_VOID*  pnpb_rec_set_rsp_async (PNIO_VOID);          // set asynchronous response mode (for 1 Request)


/* ARFSU record has fixed length -> ARFSU_LEN(20B) + 2x header(8B) = 36B*/
#define PNPB_ARFSU_RECORD_LEN	36


#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif



#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
