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
/*  F i l e               &F: usriod_im_func.c                          :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  This module contains an example code how to handle pnio I&M (identifica- */
/*  tion and maintanance) functions. Support of I&M0..I&M4 is mandatory for  */
/*  the DAP. All other I&M is optional.                                      */
/*                                                                           */
/*  THIS MODULE HAS TO BE MODIFIED BY THE PNIO USER                          */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/

#include "iod_cfg.h"

#include "pnpb_lib.h"
#include "pnpb_lib_acyc.h"
#include "usriod_im_func.h"
#include "nv_data.h"
#include <arpa/inet.h>

typedef struct
{
    PNIO_UINT32         Api;             // api number
    PNIO_UINT32 		Slot;
    PNIO_UINT32 		Subslot;
    PNIO_UINT32         pBufLen;         // [in, out] in: length to write, out: length, write by user
} im_write_item;

static  im_write_item   im_write_queue[5];
static  PNIO_ANNOTATION DevAnnotation;

// ********* external data *************

// *-----------------------------------------------------------------------
// *    some defines for IM0 data, that may be unchanged
// *   (user configurated defines for IM0  see iod_cfg.h)
// *-----------------------------------------------------------------------
#define IM0_REVCNT                              1           // 0.. 0xffff
#define IM0_PROFID                              0           // 0.. 0xffff, Using shall be defined by profiles
#define IM0_PROFSPECTYP                         3           // if ProfId = 0, ProfSpecTyp must be 1..6, e.g. 3="io module"
#define IM0_VERSMAJ                             1           // must be 01 in this version
#define IM0_VERSMIN                             1           // must be 01 in this version
#define IM0_FILTER_DATA_DAP                     0x1e        // additional support of  IM1...IM4
#define IM0_FILTER_DATA_NODAP                   0x00        // IM0 only for all submoduls except DAP

PNIO_VOID  ImX_write_add_queue
        (
                PNIO_UINT32         IMidx,            // IM number
                PNIO_UINT32         Api,              // api number
                PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
                PNIO_UINT32         *pBufLen          // [in, out] in: length to write, out: length, write by user

        )
{
    im_write_queue[IMidx].Api = Api;
    im_write_queue[IMidx].pBufLen = *pBufLen;

    im_write_queue[IMidx].Slot = pAddr->ad.Geo1.Slot;
    im_write_queue[IMidx].Subslot = pAddr->ad.Geo1.Subslot;
}

// *----------------------------------------------------------------*
// *
// *  UsrBuildDeviceAnnotation (void)
// *----------------------------------------------------------------*
// *  this function builds a device annotation string, used for function
// *  PNIO_device_open.  Because some of the annotation structure
// *  elements are also needed in the I&M functions, it is placed into
// *  this modulule.
// *  UsrBuildDeviceAnnotation must be called once during system
// *  startup, before ImFunctions () is called.
// *----------------------------------------------------------------*
PNIO_VOID  UsrIod_BuildDeviceAnnotation (void)
{
    PNIO_ANNOTATION* pDevAnnotation = &DevAnnotation;
    memset (pDevAnnotation, ' ', sizeof (DevAnnotation)); // preset value = <Blank>

    memcpy (DevAnnotation.DeviceType,
            IOD_CFG_DEVICE_TYPE,                        // annotation.DeviceType  and DCP.DeviceVendorValue must fit together
            strlen (IOD_CFG_DEVICE_TYPE));

    memcpy (DevAnnotation.OrderId,
            IOD_CFG_DEV_ANNOTATION_ORDER_ID,
            strlen (IOD_CFG_DEV_ANNOTATION_ORDER_ID));

    memcpy (DevAnnotation.SerialNumber,
            IOD_CFG_IM0_SERIAL_NUMBER,
            strlen (IOD_CFG_IM0_SERIAL_NUMBER));

    DevAnnotation.HwRevision = IOD_CFG_HW_REVISION;  // defined as big endian in PNIO spec.

    { // * --------------------------------------------------------
      // * in this example we read the version from the PNIO devkit
      // * user may create his own product version here
      // * --------------------------------------------------------
        DevAnnotation.SwRevisionPrefix  = DEVKIT_VERSION_PREFIX;    // maybe  V, R, P, U, T
        DevAnnotation.SwRevision1       = DEVKIT_VERSION_HH;        // sw-revision, highest number
        DevAnnotation.SwRevision2       = DEVKIT_VERSION_H;         // sw-revision, high number
        DevAnnotation.SwRevision3       = DEVKIT_VERSION_L;         // sw-revision, lowest number
    }
}

// *----------------------------------------------------------------*
// *
// *  Im0_read_Handler  (....)
// *----------------------------------------------------------------*
// *  this function handles a record request on RecordIndex 0xaff0,
// *  which is specified as a "I&M0" request.
// *  If an error occures, the error structure PNIO_ERR_STAT is filled.
// *  See PNIO specification for more details, how to fill the error
// *  structure.
// *----------------------------------------------------------------*
PNIO_UINT32  Im0_read_Handler
        (
                PNIO_UINT32         Api,              // api number
                PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
                PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
                PNIO_UINT8          *pBuffer,         // [in] buffer pointer
                PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
                PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
        )
{
    IM0_STRUCT*     pIm0 = (IM0_STRUCT*)    pBuffer;

    printf ( (PNIO_INT8*) "##READ IM0 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);

    // **** check buffer length, set total data length ***
    if (*pBufLen <  sizeof (IM0_STRUCT))        // plausibility check
    { // ***** error *****
        pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
        pPnioState->ErrDecode = 0x80;  // PNIORW
        pPnioState->ErrCode1  = 0xc0;  // example: Error Class 12 = resource, ErrorNr  = "write constrain conflict"
        return (PNIO_NOT_OK);
    }
    else
        *pBufLen = sizeof (IM0_STRUCT);


    // **** fill IM0 blockheader ****
    pIm0->BlockHeader.Type      = htons (0x0020);              // must be 0x0020
    pIm0->BlockHeader.Len       = htons (sizeof (IM0_STRUCT) - sizeof (REC_IO_BLOCKHDR) + 2); // blocklength, header excluded
    pIm0->BlockHeader.Version   = htons (0x0100);              // must be 0x0100 (see PNIO specification)

    // **** fill IM0 vendor ID ****
    pIm0->IM0.VendorId              = htons (IOD_CFG_VENDOR_ID);              // vendor ID

    // **** fill IM0 order ID  and serial number****
    memcpy (pIm0->IM0.OrderId,
            DevAnnotation.OrderId,
            sizeof (pIm0->IM0.OrderId));

    memcpy (pIm0->IM0.SerNum,
            IOD_CFG_IM0_SERIAL_NUMBER,
            sizeof (pIm0->IM0.SerNum));

    // * -----------------------------------------------------
    // * fill IM0  hardware- and software revision
    // *
    // * NOTE:  HwRevision in IM0 is defined as BIG ENDIAN.
    // * =====  HwRevision in DevAnnotation is defined as
    // *        machine dependant endian, because the stack
    // *        performs swapping of DevAnnotation.
    // * -----------------------------------------------------
    pIm0->IM0.HwRevision     = htons (DevAnnotation.HwRevision);           // defined as big endian in PNIO spec.

    pIm0->IM0.SwRevision.srp = DevAnnotation.SwRevisionPrefix;               // 0.. 0xff
    pIm0->IM0.SwRevision.fe  = (PNIO_UINT8) DevAnnotation.SwRevision1;       // 0.. 0xff
    pIm0->IM0.SwRevision.bf  = (PNIO_UINT8) DevAnnotation.SwRevision2;       // 0.. 0xff
    pIm0->IM0.SwRevision.ic  = (PNIO_UINT8) DevAnnotation.SwRevision3;       // 0.. 0xff

    // **** fill other elements of IM0 data structure ****
    pIm0->IM0.Revcnt         = htons (IM0_REVCNT);                     // 0.. 0xffff
    pIm0->IM0.ProfId         = htons (IM0_PROFID);                     // 0.. 0xffff, Using shall be defined by profiles
    pIm0->IM0.ProfSpecTyp    = htons (IM0_PROFSPECTYP);                // if ProfId = 0, ProfSpecTyp must be 1..6, e.g. 3="io module"
    pIm0->IM0.VersMaj        = IM0_VERSMAJ;                              // must be 01 in this version
    pIm0->IM0.VersMin        = IM0_VERSMIN;                              // must be 01 in this version

    if ((pAddr->Geo.Slot == 0) && (pAddr->Geo.Subslot == 1))
    {
        pIm0->IM0.ImXSupported   = htons (IM0_FILTER_DATA_DAP);                // IM0 filter data
    }
    else
    {
        pIm0->IM0.ImXSupported   = htons (IM0_FILTER_DATA_NODAP);                // IM0 filter data
    }

    // *** clear error structure ***
    memset (pPnioState, 0, sizeof (PNIO_ERR_STAT));
    return (PNIO_OK);
}



// *----------------------------------------------------------------*
// *
// *  Im1_read_Handler  (....)
// *----------------------------------------------------------------*
// *  this function handles a record read request on RecordIndex 0xaff1
// *----------------------------------------------------------------*
PNIO_UINT32  Im1_read_Handler
        (
                PNIO_UINT32         Api,              // api number
                PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
                PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
                PNIO_UINT8          *pBuffer,         // [in] buffer pointer
                PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
                PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
        )
{
    IM1_STRUCT*     pIm = (IM1_STRUCT*)    pBuffer;
    IM1_DATA*       pImDatTmp;
    PNIO_UINT32     ImDatLen;
    PNIO_UINT32     Status;

    printf ( (PNIO_INT8*) "##READ IM1 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);

    // **** check buffer length, set total data length ***
    if (*pBufLen <  sizeof (IM1_STRUCT))        // plausibility check
    { // ***** error *****
        pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
        pPnioState->ErrDecode = 0x80;  // PNIORW
        pPnioState->ErrCode1  = 0xc0;  // example: Error Class 12 = resource, ErrorNr  = "write constrain conflict"
        return (PNIO_NOT_OK);
    }
    else
        *pBufLen = sizeof (IM1_STRUCT);


    // **** fill IM1 blockheader ****
    pIm->BlockHeader.Type      = htons (0x0021);              // must be 0x0021
    pIm->BlockHeader.Len       = htons (sizeof (IM1_STRUCT) - sizeof (REC_IO_BLOCKHDR) + 2); // blocklength, header excluded
    pIm->BlockHeader.Version   = htons (0x0100);              // must be 0x0100 (see PNIO specification)

    // *--------------------------------------------------------------
    // * read IM1 data from NV memory and save in pIm1
    // *--------------------------------------------------------------
    Status = Bsp_im_data_restore (PNIO_NVDATA_IM1, // data type
                                  (PNIO_VOID**) &pImDatTmp,            // data pointer (allocated by
                                  &ImDatLen,
                                  PeriphRealCfgInd);
    IM1_DATA* pIM1 = &(pIm->IM1);
    memcpy (pIM1,
              pImDatTmp,
              sizeof (IM1_DATA));
    Bsp_nv_data_memfree (pImDatTmp);


    // *** clear error structure ***
    memset (pPnioState, 0, sizeof (PNIO_ERR_STAT));
    return (PNIO_OK);
}


// *----------------------------------------------------------------*
// *
// *  Im2_read_Handler  (....)
// *----------------------------------------------------------------*
// *  this function handles a record read request on RecordIndex 0xaff2
// *----------------------------------------------------------------*
PNIO_UINT32  Im2_read_Handler
        (
                PNIO_UINT32         Api,              // api number
                PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
                PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
                PNIO_UINT8          *pBuffer,         // [in] buffer pointer
                PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
                PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
        )
{
    IM2_STRUCT*     pIm = (IM2_STRUCT*)    pBuffer;
    IM2_DATA*       pImDatTmp;
    PNIO_UINT32     ImDatLen;
    PNIO_UINT32     Status;

    printf ( (PNIO_INT8*) "##READ IM2 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);

    // **** check buffer length, set total data length ***
    if (*pBufLen <  sizeof (IM2_STRUCT))        // plausibility check
    { // ***** error *****
        pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
        pPnioState->ErrDecode = 0x80;  // PNIORW
        pPnioState->ErrCode1  = 0xc0;  // example: Error Class 12 = resource, ErrorNr  = "write constrain conflict"
        return (PNIO_NOT_OK);
    }
    else
        *pBufLen = sizeof (IM2_STRUCT);


    // **** fill IM2 blockheader ****
    pIm->BlockHeader.Type      = htons (0x0022);              // must be 0x0022
    pIm->BlockHeader.Len       = htons (sizeof (IM2_STRUCT) - sizeof (REC_IO_BLOCKHDR) + 2); // blocklength, header excluded
    pIm->BlockHeader.Version   = htons (0x0100);              // must be 0x0100 (see PNIO specification)

    // *--------------------------------------------------------------
    // * read IM2 data from NV memory and save in pIm2
    // *--------------------------------------------------------------
    Status = Bsp_im_data_restore (PNIO_NVDATA_IM2, // data type
                                  (PNIO_VOID**) &pImDatTmp,            // data pointer (allocated by
                                  &ImDatLen,
                                  PeriphRealCfgInd);
    IM2_DATA* pIM2 = &(pIm->IM2);
    memcpy(pIM2,
              pImDatTmp,
              sizeof (IM2_DATA));
    Bsp_nv_data_memfree (pImDatTmp);


    // *** clear error structure ***
    memset (pPnioState, 0, sizeof (PNIO_ERR_STAT));
    return (PNIO_OK);
}


// *----------------------------------------------------------------*
// *
// *  Im3_read_Handler  (....)
// *----------------------------------------------------------------*
// *  this function handles a record read request on RecordIndex 0xaff3
// *----------------------------------------------------------------*
PNIO_UINT32  Im3_read_Handler
        (
                PNIO_UINT32         Api,            // api number
                PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
                PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
                PNIO_UINT8          *pBuffer,         // [in] buffer pointer
                PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
                PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
        )
{
    IM3_STRUCT*     pIm = (IM3_STRUCT*)    pBuffer;
    IM3_DATA*       pImDatTmp;
    PNIO_UINT32     ImDatLen;
    PNIO_UINT32     Status;

    printf ( (PNIO_INT8*) "##READ IM3 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);

    // **** check buffer length, set total data length ***
    if (*pBufLen <  sizeof (IM3_STRUCT))        // plausibility check
    { // ***** error *****
        pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
        pPnioState->ErrDecode = 0x80;  // PNIORW
        pPnioState->ErrCode1  = 0xc0;  // example: Error Class 12 = resource, ErrorNr  = "write constrain conflict"
        return (PNIO_NOT_OK);
    }
    else
        *pBufLen = sizeof (IM3_STRUCT);


    // **** fill IM3 blockheader ****
    pIm->BlockHeader.Type      = htons (0x0023);              // must be 0x0023
    pIm->BlockHeader.Len       = htons (sizeof (IM3_STRUCT) - sizeof (REC_IO_BLOCKHDR) + 2); // blocklength, header excluded
    pIm->BlockHeader.Version   = htons (0x0100);              // must be 0x0100 (see PNIO specification)

    // *--------------------------------------------------------------
    // * read IM2 data from NV memory and save in pIm2
    // *--------------------------------------------------------------
    Status = Bsp_im_data_restore (PNIO_NVDATA_IM3, // data type
                                  (PNIO_VOID**) &pImDatTmp,            // data pointer (allocated by
                                  &ImDatLen,
                                  PeriphRealCfgInd);
	
    IM3_DATA* pIM3 = &(pIm->IM3);
    memcpy (pIM3,
              pImDatTmp,
              sizeof (IM3_DATA));
    Bsp_nv_data_memfree (pImDatTmp);


    // *** clear error structure ***
    memset (pPnioState, 0, sizeof (PNIO_ERR_STAT));
    return (PNIO_OK);
}


// *----------------------------------------------------------------*
// *
// *  Im4_read_Handler  (....)
// *----------------------------------------------------------------*
// *  this function handles a record read request on RecordIndex 0xaff4
// *----------------------------------------------------------------*
PNIO_UINT32  Im4_read_Handler
        (
                PNIO_UINT32         Api,              // api number
                PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
                PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
                PNIO_UINT8          *pBuffer,         // [in] buffer pointer
                PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
                PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
        )
{
    IM4_STRUCT*     pIm = (IM4_STRUCT*)    pBuffer;
    IM4_DATA*       pImDatTmp;
    PNIO_UINT32     ImDatLen;
    PNIO_UINT32     Status;

    printf ( (PNIO_INT8*) "##READ IM4 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);

    // **** check buffer length, set total data length ***
    if (*pBufLen <  sizeof (IM4_STRUCT))        // plausibility check
    { // ***** error *****
        pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
        pPnioState->ErrDecode = 0x80;  // PNIORW
        pPnioState->ErrCode1  = 0xc0;  // example: Error Class 12 = resource, ErrorNr  = "write constrain conflict"
        return (PNIO_NOT_OK);
    }
    else
        *pBufLen = sizeof (IM4_STRUCT);


    // **** fill IM4 blockheader ****
    pIm->BlockHeader.Type      = htons (0x0024);              // must be 0x0024
    pIm->BlockHeader.Len       = htons (sizeof (IM4_STRUCT) - sizeof (REC_IO_BLOCKHDR) + 2); // blocklength, header excluded
    pIm->BlockHeader.Version   = htons (0x0100);              // must be 0x0100 (see PNIO specification)

    // *--------------------------------------------------------------
    // * read IM2 data from NV memory and save in pIm2
    // *--------------------------------------------------------------
    Status = Bsp_im_data_restore (PNIO_NVDATA_IM4, // data type
                                  (PNIO_VOID**) &pImDatTmp,            // data pointer (allocated by
                                  &ImDatLen,
                                  PeriphRealCfgInd);
    IM4_DATA* pIM4 = &(pIm->IM4);
    memcpy (pIM4,
              pImDatTmp,
              sizeof (IM4_DATA));
    Bsp_nv_data_memfree (pImDatTmp);


    // *** clear error structure ***
    memset (pPnioState, 0, sizeof (PNIO_ERR_STAT));
    return (PNIO_OK);
}



// *----------------------------------------------------------------*
// *
// *  Im0_write_Handler  (....)
// *----------------------------------------------------------------*
// *  this function handles a record write request on RecordIndex 0xaff0
// *----------------------------------------------------------------*
PNIO_UINT32  Im0_write_Handler
        (
                PNIO_UINT32         Api,              // api number
                PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
                PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
                PNIO_UINT8          *pBuffer,         // [in] buffer pointer
                PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
                PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
        )
{
    ImX_write_add_queue(0, Api, pAddr, pBufLen);

    // *** return error:  IM0 is readable only ***
    pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
    pPnioState->ErrDecode = 0x80;  // PNIORW
    pPnioState->ErrCode1  = 0xb6;  // example: Error Class 11 = Access, ErrorNr 6 = "access denied"
    pPnioState->ErrCode2  = 0;     // here dont care
    pPnioState->AddValue1 = 0;     // here dont care
    pPnioState->AddValue2 = 0;     // here dont care

    ImX_write_Handler_done(0, PNIO_NOT_OK, *pBufLen);

    return (PNIO_NOT_OK);
}


// *----------------------------------------------------------------*
// *
// *  Im1_write_Handler  (....)
// *----------------------------------------------------------------*
// *  this function handles a record write request on RecordIndex 0xaff1
// *----------------------------------------------------------------*
PNIO_UINT32  Im1_write_Handler
        (
                PNIO_UINT32         Api,             // api number
                PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
                PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
                PNIO_UINT8          *pBuffer,         // [in] buffer pointer
                PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
                PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
        )
{
    PNIO_UINT32 Status;

    if (*pBufLen > sizeof (IM1_DATA))
        *pBufLen = sizeof (IM1_DATA);

    ImX_write_add_queue(1, Api, pAddr, pBufLen);

    Status = Bsp_im_data_store (PNIO_NVDATA_IM1,       // nv data type: device name
                                 pBuffer,               // source pointer to the devicename
                                 *pBufLen,              // length of the device name
                                 PeriphRealCfgInd);     // entity index in periph interface-real_cfg

    if(Status != PNIO_OK)
    {
        ImX_write_Handler_done(1, PNIO_NOT_OK, *pBufLen);
    }

    return (Status);
}


// *----------------------------------------------------------------*
// *
// *  Im2_write_Handler  (....)
// *----------------------------------------------------------------*
// *  this function handles a record write request on RecordIndex 0xaff2
// *----------------------------------------------------------------*
PNIO_UINT32  Im2_write_Handler
        (
                PNIO_UINT32         Api,             // api number
                PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
                PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
                PNIO_UINT8          *pBuffer,         // [in] buffer pointer
                PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
                PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
        )
{
    PNIO_UINT32 Status;

    if (*pBufLen > sizeof (IM2_DATA))
        *pBufLen = sizeof (IM2_DATA);

    ImX_write_add_queue(2, Api, pAddr, pBufLen);

    Status = Bsp_im_data_store (PNIO_NVDATA_IM2,       // nv data type: device name
                                 pBuffer,               // source pointer to the devicename
                                 *pBufLen,              // length of the device name
                                 PeriphRealCfgInd);     // entity index in periph interface-real_cfg

    if(Status != PNIO_OK)
    {
        ImX_write_Handler_done(2, PNIO_NOT_OK, *pBufLen);
    }

    return (Status);
}


// *----------------------------------------------------------------*
// *
// *  Im3_write_Handler  (....)
// *----------------------------------------------------------------*
// *  this function handles a record write request on RecordIndex 0xaff3
// *----------------------------------------------------------------*
PNIO_UINT32  Im3_write_Handler
        (
                PNIO_UINT32         Api,             // api number
                PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
                PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
                PNIO_UINT8          *pBuffer,         // [in] buffer pointer
                PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
                PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
        )
{
    PNIO_UINT32 Status;

    if (*pBufLen > sizeof (IM3_DATA))
        *pBufLen = sizeof (IM3_DATA);

    ImX_write_add_queue(3, Api, pAddr, pBufLen);

    Status = Bsp_im_data_store (PNIO_NVDATA_IM3,       // nv data type: device name
                                 pBuffer,               // source pointer to the devicename
                                 *pBufLen,              // length of the device name
                                 PeriphRealCfgInd);     // entity index in periph interface-real_cfg

    if(Status != PNIO_OK)
    {
        ImX_write_Handler_done(3, PNIO_NOT_OK, *pBufLen);
    }

    return (Status);
}


// *----------------------------------------------------------------*
// *
// *  Im4_write_Handler  (....)
// *----------------------------------------------------------------*
// *  this function handles a record write request on RecordIndex 0xaff4
// *----------------------------------------------------------------*
PNIO_UINT32  Im4_write_Handler
        (
                PNIO_UINT32         Api,              // api number
                PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
                PNIO_UINT32         *pBufLen,         // [in, out] in: length to write, out: length, write by user
                PNIO_UINT8          *pBuffer,         // [in] buffer pointer
                PNIO_ERR_STAT       *pPnioState,      // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
                PNIO_UINT32         PeriphRealCfgInd  // entity index in periph interface-real_cfg
        )
{
    PNIO_UINT32 Status;

    if (*pBufLen > sizeof (IM4_DATA))
        *pBufLen = sizeof (IM1_DATA);

    ImX_write_add_queue(4, Api, pAddr, pBufLen);

    Status = Bsp_im_data_store (PNIO_NVDATA_IM4,       // nv data type: device name
                                 pBuffer,               // source pointer to the devicename
                                 *pBufLen,              // length of the device name
                                 PeriphRealCfgInd);     // entity index in periph interface-real_cfg

    if(Status != PNIO_OK)
    {
        ImX_write_Handler_done(4, PNIO_NOT_OK, *pBufLen);
    }

    return (Status);
}

// *----------------------------------------------------------------*
// *
// *  ImX_write_Handler_done  (....)
// *----------------------------------------------------------------*
PNIO_VOID  ImX_write_Handler_done
        (
                PNIO_UINT32 IMidx,
                PNIO_UINT32 Status,
                PNIO_UINT32 DatLen
        )
{
    PNIO_ERR_STAT PnioState;
    memset(&PnioState, 0, sizeof(PNIO_ERR_STAT));

    if (Status == PNIO_OK)
    {
        printf((PNIO_INT8*) "##WRITE IM%d Data done\n", IMidx);
    } else
    {
        // *** return error ***
        PnioState.ErrCode = 0xdf;    // IODWriteRes with ErrorDecode = PNIORW
        PnioState.ErrDecode = 0x80;  // PNIORW
        PnioState.ErrCode1 = 0xb6;   // example: Error Class 11 = Access, ErrorNr 6 = "access denied"
        PnioState.ErrCode2 = 0;      // here dont care
        PnioState.AddValue1 = 0;     // here dont care
        PnioState.AddValue2 = 0;     // here dont care
        printf(
                (PNIO_INT8*) "##WRITE IM%d Data failed\n", IMidx);
    }

    /* Send response to host */
    PNIOext_im_write_rsp(&PnioState);
}

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
