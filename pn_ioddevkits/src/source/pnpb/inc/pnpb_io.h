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
/*  F i l e               &F: pnpb_io.h                                 :F&  */
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
#ifndef PNPB_IO_H
#define PNPB_IO_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

// *-------------------------------------------------
// *    PNPB internal functions
// *-------------------------------------------------
PNIO_UINT32 PnpbConsLock(PNIO_UINT8** ppDatOut, PNIO_UINT32** ppApduStat, PNIO_UINT16 arIdx);
PNIO_UINT32 PnpbConsUnlock(PNIO_UINT16 arIdx);
PNIO_UINT32 PnpbProvLock(PNIO_UINT8** ppDatIn, PNIO_UINT16 arIdx, PNIO_UINT16 dataOffset, PNIO_UINT16 dataLength);
PNIO_UINT32 PnpbProvUnlock(PNIO_UINT16 arIdx);

PNIO_UINT32 PnpbCopyRecReadIoData(PNIO_UINT8* pDatIn, PNIO_UINT16 ArIdx, PNIO_UINT16 dataOffset, PNIO_UINT16 dataLength);

// *-------------------------------------------------
// *    public functions
// *-------------------------------------------------
PNIO_UINT32 pnpb_io_init(PNIO_VOID);
PNIO_UINT32 pnpb_initiate_data_read  (PNIO_UINT32  DevHndl);

PNIO_UINT32 pnpb_initiate_data_write (PNIO_UINT32  DevHndl);

PNIO_UINT32 pnpb_set_iops   (PNIO_UINT32 Api, 
                             PNIO_UINT32 SlotNum, 
                             PNIO_UINT32 SubNum, 
                             PNIO_UINT8  Iops);

#if IOD_INCLUDE_REC8028_8029
PNIO_UINT32 pnpb_get_input_data  (PNIO_UINT32    ArInd,
                                  PNIO_EXP_SUB*  pExpSub,
                                  PNIO_UINT8*    pIoDatBuf,
                                  PNIO_UINT8*    pIopsBuf,
                                  PNIO_UINT8*    pIocsBuf);

PNIO_UINT32 pnpb_get_output_data (PNIO_UINT32    ArInd,
                                  PNIO_EXP_SUB*  pExpSub,
                                  PNIO_UINT8*    pIoDatBuf,
                                  PNIO_UINT8*    pIopsBuf,
                                  PNIO_UINT8*    pIocsBuf);
#endif

PNIO_UINT32 pnpb_get_last_apdu_status  (PNIO_UINT32   ArNum);


#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
