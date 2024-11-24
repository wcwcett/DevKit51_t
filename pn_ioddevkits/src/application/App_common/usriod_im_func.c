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

#include "compiler.h"

#if (IOD_INCLUDE_IM0_4 == 0)  // IM0..4 not handled inside PN stack--> do it here
    #include "usriod_cfg.h"
    #include "os.h"
    #include "pniousrd.h"
    #include "iodapi_event.h"
    #include "usriod_im_func.h"

    #include "pndv_inc.h"
    #include "pnpb_api.h"
    #include "pnpb_sub_real.h"
    #include "pnpb_trc.h"
    #include "pnpb_im_func.h"
    #include "pnpb.h"

    #include "usriod_cfg.h"
    #include "nv_data.h"
    #include "PnUsr_xhif.h"

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
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
#if (1 != IOD_USED_WITH_XHIF_HOST)
        IM0_STRUCT*     pIm0 = (IM0_STRUCT*)    pBuffer;

        PNIO_printf("##READ IM0 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
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
        {
            *pBufLen = sizeof (IM0_STRUCT);
        }

        // **** fill IM0 blockheader ****
        pIm0->BlockHeader.Type      = OsHtons (0x0020);              // must be 0x0020
        pIm0->BlockHeader.Len       = OsHtons (sizeof (IM0_STRUCT) - sizeof (REC_IO_BLOCKHDR) + 2); // blocklength, header excluded
        pIm0->BlockHeader.Version   = OsHtons (0x0100);              // must be 0x0100 (see PNIO specification)

        // **** fill IM0 vendor ID ****
        pIm0->IM0.VendorId              = OsHtons (IOD_CFG_VENDOR_ID);              // vendor ID

        // **** fill IM0 order ID  and serial number****
        OsMemCpy (pIm0->IM0.OrderId,
                DevAnnotation.OrderId,
                sizeof (pIm0->IM0.OrderId));

        OsMemCpy (pIm0->IM0.SerNum,
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
        pIm0->IM0.HwRevision     = OsHtons (DevAnnotation.HwRevision);           // defined as big endian in PNIO spec.

        pIm0->IM0.SwRevision.srp = DevAnnotation.SwRevisionPrefix;               // 0.. 0xff
        pIm0->IM0.SwRevision.fe  = (PNIO_UINT8) DevAnnotation.SwRevision1;       // 0.. 0xff
        pIm0->IM0.SwRevision.bf  = (PNIO_UINT8) DevAnnotation.SwRevision2;       // 0.. 0xff
        pIm0->IM0.SwRevision.ic  = (PNIO_UINT8) DevAnnotation.SwRevision3;       // 0.. 0xff

        // **** fill other elements of IM0 data structure ****
        pIm0->IM0.Revcnt         = OsHtons (IM0_REVCNT);                     // 0.. 0xffff
        pIm0->IM0.ProfId         = OsHtons (IM0_PROFID);                     // 0.. 0xffff, Using shall be defined by profiles
        pIm0->IM0.ProfSpecTyp    = OsHtons (IM0_PROFSPECTYP);                // if ProfId = 0, ProfSpecTyp must be 1..6, e.g. 3="io module"
        pIm0->IM0.VersMaj        = IM0_VERSMAJ;                              // must be 01 in this version
        pIm0->IM0.VersMin        = IM0_VERSMIN;                              // must be 01 in this version

        if ((pAddr->Geo.Slot == 0) && (pAddr->Geo.Subslot == 1))
        {
            pIm0->IM0.ImXSupported   = OsHtons (IM0_FILTER_DATA_DAP);                // IM0 filter data
        }
        else
        {
            pIm0->IM0.ImXSupported   = OsHtons (IM0_FILTER_DATA_NODAP);                // IM0 filter data
        }

        // *** clear error structure ***
        OsMemSet (pPnioState, 0, sizeof (PNIO_ERR_STAT));

#else
        /* Save parameters to structures and send it to host with request */
        /* Host will send them back together with valid data */

        PNIOext_cbf_im_read(0, Api, pAddr, pBufLen, pBuffer, 0, pPnioState);
#endif

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
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
#if (1 != IOD_USED_WITH_XHIF_HOST)
        IM1_STRUCT*     pIm = (IM1_STRUCT*)    pBuffer;
        IM1_DATA*       pImDatTmp;
        PNIO_UINT32     ImDatLen;
        PNIO_UINT32     Status;
#endif

        PNIO_UINT32     PeriphRealCfgInd = 0;
        PNPB_REAL_SUB*  pSub = NULL;       // pointer to subslot

        // **** check if subslot is plugged, else error ****
        OsEnterX (OS_MUTEX_PNPB);
        pSub   = pnpb_sub_real_getp  (Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        OsExitX (OS_MUTEX_PNPB);
        if (pSub == NULL)
        {
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"
#if (1 != IOD_USED_WITH_XHIF_HOST)
            return (PNIO_NOT_OK);
#endif
        }
        else
        {
            PeriphRealCfgInd = pSub->PeriphRealCfgInd;
        }

#if (1 != IOD_USED_WITH_XHIF_HOST)
        PNIO_printf("##READ IM1 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
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
        {
            *pBufLen = sizeof (IM1_STRUCT);
        }

        // **** fill IM1 blockheader ****
        pIm->BlockHeader.Type      = OsHtons (0x0021);              // must be 0x0021
        pIm->BlockHeader.Len       = OsHtons (sizeof (IM1_STRUCT) - sizeof (REC_IO_BLOCKHDR) + 2); // blocklength, header excluded
        pIm->BlockHeader.Version   = OsHtons (0x0100);              // must be 0x0100 (see PNIO specification)

	    // *--------------------------------------------------------------
	    // * read IM1 data from NV memory and save in pIm1
	    // *--------------------------------------------------------------
        Status = Bsp_im_data_restore (PNIO_NVDATA_IM1, // data type
                                      (PNIO_VOID**) &pImDatTmp,            // data pointer (allocated by
                                      &ImDatLen, PeriphRealCfgInd);

        OsMemCpy (&pIm->IM1,
                  pImDatTmp,
                  sizeof (IM1_DATA));
	    Bsp_nv_data_memfree (pImDatTmp);


        // *** clear error structure ***
        OsMemSet (pPnioState, 0, sizeof (PNIO_ERR_STAT));
#else
        /* Save parameters to structures and send it to host with request */
        /* Host will send them back together with valid data */

        PNIOext_cbf_im_read(1, Api, pAddr, pBufLen, pBuffer, PeriphRealCfgInd, pPnioState);
#endif

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
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
#if (1 != IOD_USED_WITH_XHIF_HOST)
        IM2_STRUCT*     pIm = (IM2_STRUCT*)    pBuffer;
        IM2_DATA*       pImDatTmp;
        PNIO_UINT32     ImDatLen;
        PNIO_UINT32     Status;
#endif

        PNIO_UINT32     PeriphRealCfgInd = 0;
        PNPB_REAL_SUB*  pSub = NULL;       // pointer to subslot

        // **** check if subslot is plugged, else error ****
        OsEnterX (OS_MUTEX_PNPB);
        pSub   = pnpb_sub_real_getp  (Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        OsExitX (OS_MUTEX_PNPB);
        if (pSub == NULL)
        {
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"

#if (1 != IOD_USED_WITH_XHIF_HOST)
            return (PNIO_NOT_OK);
#endif
        }
        else
        {
            PeriphRealCfgInd = pSub->PeriphRealCfgInd;
        }

#if (1 != IOD_USED_WITH_XHIF_HOST)
        PNIO_printf("##READ IM2 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
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
        {
            *pBufLen = sizeof (IM2_STRUCT);
        }

        // **** fill IM2 blockheader ****
        pIm->BlockHeader.Type      = OsHtons (0x0022);              // must be 0x0022
        pIm->BlockHeader.Len       = OsHtons (sizeof (IM2_STRUCT) - sizeof (REC_IO_BLOCKHDR) + 2); // blocklength, header excluded
        pIm->BlockHeader.Version   = OsHtons (0x0100);              // must be 0x0100 (see PNIO specification)

	    // *--------------------------------------------------------------
	    // * read IM2 data from NV memory and save in pIm2
	    // *--------------------------------------------------------------
        Status = Bsp_im_data_restore (PNIO_NVDATA_IM2, // data type
                                      (PNIO_VOID**) &pImDatTmp,            // data pointer (allocated by
                                      &ImDatLen, PeriphRealCfgInd);

        OsMemCpy (&pIm->IM2,
                  pImDatTmp,
                  sizeof (IM2_DATA));
	    Bsp_nv_data_memfree (pImDatTmp);


        // *** clear error structure ***
        OsMemSet (pPnioState, 0, sizeof (PNIO_ERR_STAT));

#else
        /* Save parameters to structures and send it to host with request */
        /* Host will send them back together with valid data */

        PNIOext_cbf_im_read(2, Api, pAddr, pBufLen, pBuffer, PeriphRealCfgInd, pPnioState);
#endif

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
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
#if (1 != IOD_USED_WITH_XHIF_HOST)
        IM3_STRUCT*     pIm = (IM3_STRUCT*)    pBuffer;
        IM3_DATA*       pImDatTmp;
        PNIO_UINT32     ImDatLen;
        PNIO_UINT32     Status;
#endif

        PNIO_UINT32     PeriphRealCfgInd = 0;
        PNPB_REAL_SUB*  pSub = NULL;       // pointer to subslot

        // **** check if subslot is plugged, else error ****
        OsEnterX (OS_MUTEX_PNPB);
        pSub   = pnpb_sub_real_getp  (Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        OsExitX (OS_MUTEX_PNPB);
        if (pSub == NULL)
        {
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"
#if (1 != IOD_USED_WITH_XHIF_HOST)
            return (PNIO_NOT_OK);
#endif
        }
        else
        {
            PeriphRealCfgInd = pSub->PeriphRealCfgInd;
        }

#if (1 != IOD_USED_WITH_XHIF_HOST)
        PNIO_printf("##READ IM3 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
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
        {
            *pBufLen = sizeof (IM3_STRUCT);
        }

        // **** fill IM3 blockheader ****
        pIm->BlockHeader.Type      = OsHtons (0x0023);              // must be 0x0023
        pIm->BlockHeader.Len       = OsHtons (sizeof (IM3_STRUCT) - sizeof (REC_IO_BLOCKHDR) + 2); // blocklength, header excluded
        pIm->BlockHeader.Version   = OsHtons (0x0100);              // must be 0x0100 (see PNIO specification)

	    // *--------------------------------------------------------------
	    // * read IM2 data from NV memory and save in pIm2
	    // *--------------------------------------------------------------
        Status = Bsp_im_data_restore (PNIO_NVDATA_IM3, // data type
                                      (PNIO_VOID**) &pImDatTmp,            // data pointer (allocated by
                                      &ImDatLen, PeriphRealCfgInd);

        OsMemCpy (&pIm->IM3,
                  pImDatTmp,
                  sizeof (IM3_DATA));
	    Bsp_nv_data_memfree (pImDatTmp);


        // *** clear error structure ***
        OsMemSet (pPnioState, 0, sizeof (PNIO_ERR_STAT));

#else
        /* Save parameters to structures and send it to host with request */
        /* Host will send them back together with valid data */

        PNIOext_cbf_im_read(3, Api, pAddr, pBufLen, pBuffer, PeriphRealCfgInd, pPnioState);
#endif

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
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
#if (1 != IOD_USED_WITH_XHIF_HOST)
        IM4_STRUCT*     pIm = (IM4_STRUCT*)    pBuffer;
        IM4_DATA*       pImDatTmp;
        PNIO_UINT32     ImDatLen;
        PNIO_UINT32     Status;
#endif

         PNIO_UINT32     PeriphRealCfgInd = 0;
         PNPB_REAL_SUB*  pSub = NULL;       // pointer to subslot

         // **** check if subslot is plugged, else error ****
         OsEnterX (OS_MUTEX_PNPB);
         pSub   = pnpb_sub_real_getp  (Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
         OsExitX (OS_MUTEX_PNPB);
         if (pSub == NULL)
         {
             pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
             pPnioState->ErrDecode = 0x80;  // PNIORW
             pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"

 #if (1 != IOD_USED_WITH_XHIF_HOST)
             return (PNIO_NOT_OK);
 #endif
         }
         else
         {
             PeriphRealCfgInd = pSub->PeriphRealCfgInd;
         }

#if (1 != IOD_USED_WITH_XHIF_HOST)
        PNIO_printf("##READ IM4 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
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
        {
            *pBufLen = sizeof (IM4_STRUCT);
        }

        // **** fill IM4 blockheader ****
        pIm->BlockHeader.Type      = OsHtons (0x0024);              // must be 0x0024
        pIm->BlockHeader.Len       = OsHtons (sizeof (IM4_STRUCT) - sizeof (REC_IO_BLOCKHDR) + 2); // blocklength, header excluded
        pIm->BlockHeader.Version   = OsHtons (0x0100);              // must be 0x0100 (see PNIO specification)

	    // *--------------------------------------------------------------
	    // * read IM2 data from NV memory and save in pIm2
	    // *--------------------------------------------------------------
        Status = Bsp_im_data_restore (PNIO_NVDATA_IM4, // data type
                                      (PNIO_VOID**) &pImDatTmp,            // data pointer (allocated by
                                      &ImDatLen, PeriphRealCfgInd);

        OsMemCpy (&pIm->IM4,
                  pImDatTmp,
                  sizeof (IM4_DATA));
	    Bsp_nv_data_memfree (pImDatTmp);


        // *** clear error structure ***
        OsMemSet (pPnioState, 0, sizeof (PNIO_ERR_STAT));

#else
        /* Save parameters to structures and send it to host with request */
        /* Host will send them back together with valid data */

        PNIOext_cbf_im_read(4, Api, pAddr, pBufLen, pBuffer, PeriphRealCfgInd, pPnioState);
#endif

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
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
#if (1 != IOD_USED_WITH_XHIF_HOST)
        // *** return error:  IM0 is readable only ***
        pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
        pPnioState->ErrDecode = 0x80;  // PNIORW
        pPnioState->ErrCode1  = 0xb6;  // example: Error Class 11 = Access, ErrorNr 6 = "access denied"
        pPnioState->ErrCode2  = 0;     // here dont care
        pPnioState->AddValue1 = 0;     // here dont care
        pPnioState->AddValue2 = 0;     // here dont care
        return (PNIO_NOT_OK);

#else
        /* Save parameters to structures and send it to host with request */
        /* Host will send them back together with valid data */

        PNIOext_cbf_im_write(0, Api, pAddr, pBufLen, pBuffer, 0, pPnioState);
        return (PNIO_OK);
#endif
    }


    // *----------------------------------------------------------------*
    // *
    // *  Im1_write_Handler  (....)
    // *----------------------------------------------------------------*
    // *  this function handles a record write request on RecordIndex 0xaff1
    // *----------------------------------------------------------------*
    PNIO_UINT32  Im1_write_Handler
		    (
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to write, out: length, write by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
#if (1 != IOD_USED_WITH_XHIF_HOST)
        PNIO_UINT32 Status;
#endif

        PNIO_UINT32     PeriphRealCfgInd = 0;
        PNPB_REAL_SUB*  pSub = NULL;

        // **** check if subslot is plugged, else error ****
        OsEnterX (OS_MUTEX_PNPB);
        pSub   = pnpb_sub_real_getp  (Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        OsExitX (OS_MUTEX_PNPB);

        if (pSub == NULL)
        {
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"

#if (1 != IOD_USED_WITH_XHIF_HOST)
            return (PNIO_NOT_OK);
#endif
        }
        else
        {
            PeriphRealCfgInd = pSub->PeriphRealCfgInd;
        }

#if (1 != IOD_USED_WITH_XHIF_HOST)
        // **** check buffer length, set total data length ***
        if (*pBufLen > sizeof (IM1_STRUCT))        // plausibility check
        { // ***** error *****
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb1;  // example: Error Class 11 = Access, ErrorNr 1 = "write length error"

            return (PNIO_NOT_OK);
        }
        else
            *pBufLen = sizeof (IM1_DATA);    // length without blockheader

        Status =  Bsp_im_data_store (PNIO_NVDATA_IM1,                       // nv data type: device name
                                     pBuffer + sizeof (REC_IO_BLOCKHDR),    // source pointer to the devicename
                                     *pBufLen,                              // length of the device name
                                     PeriphRealCfgInd);

        if (Status == PNIO_OK)
        {
            PNIO_printf("##WRITE IM1 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                         Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);
        }
        else
        {
            // *** return error ***
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb6;  // example: Error Class 11 = Access, ErrorNr 6 = "access denied"
            pPnioState->ErrCode2  = 0;     // here dont care
            pPnioState->AddValue1 = 0;     // here dont care
            pPnioState->AddValue2 = 0;     // here dont care
            PNIO_printf("##ERROR WRITE IM1 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                         Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);
        }

        return (Status);

#else
        /* Save parameters to structures and send it to host with request */
        /* Host will send them back together with valid data */

        PNIOext_cbf_im_write(1, Api, pAddr, pBufLen, pBuffer + sizeof (REC_IO_BLOCKHDR), PeriphRealCfgInd, pPnioState);
        return (PNIO_OK);
#endif
    }


    // *----------------------------------------------------------------*
    // *
    // *  Im2_write_Handler  (....)
    // *----------------------------------------------------------------*
    // *  this function handles a record write request on RecordIndex 0xaff2
    // *----------------------------------------------------------------*
    PNIO_UINT32  Im2_write_Handler
		    (
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
#if (1 != IOD_USED_WITH_XHIF_HOST)
        PNIO_UINT32 Status;
#endif

        PNIO_UINT32     PeriphRealCfgInd = 0;
        PNPB_REAL_SUB*  pSub = NULL;

        // **** check if subslot is plugged, else error ****
        OsEnterX (OS_MUTEX_PNPB);
        pSub   = pnpb_sub_real_getp  (Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        OsExitX (OS_MUTEX_PNPB);

        if (pSub == NULL)
        {
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"

#if (1 != IOD_USED_WITH_XHIF_HOST)
            return (PNIO_NOT_OK);
#endif
        }
        else
        {
            PeriphRealCfgInd = pSub->PeriphRealCfgInd;
        }

#if (1 != IOD_USED_WITH_XHIF_HOST)
        // **** check buffer length, set total data length ***
        if (*pBufLen >  sizeof (IM2_STRUCT))        // plausibility check
        { // ***** error *****
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb1;  // example: Error Class 11 = Access, ErrorNr 1 = "write length error"

            return (PNIO_NOT_OK);
        }
        else
            *pBufLen = sizeof (IM2_DATA);   // length without blockheader

        Status =  Bsp_im_data_store (PNIO_NVDATA_IM2,       // nv data type: device name
                                     pBuffer + sizeof (REC_IO_BLOCKHDR),               // source pointer to the devicename
                                     *pBufLen,              // length of the device name
                                     PeriphRealCfgInd);

        if (Status == PNIO_OK)
        {
            PNIO_printf("##WRITE IM2 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                         Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);
        }
        else
        {
            // *** return error ***
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb6;  // example: Error Class 11 = Access, ErrorNr 6 = "access denied"
            pPnioState->ErrCode2  = 0;     // here dont care
            pPnioState->AddValue1 = 0;     // here dont care
            pPnioState->AddValue2 = 0;     // here dont care
            PNIO_printf("##ERROR WRITE IM2 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                         Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);
        }

        return (Status);

#else
        /* Save parameters to structures and send it to host with request */
        /* Host will send them back together with valid data */

        PNIOext_cbf_im_write(2, Api, pAddr, pBufLen, pBuffer + sizeof (REC_IO_BLOCKHDR), PeriphRealCfgInd, pPnioState);
        return (PNIO_OK);
#endif
    }


    // *----------------------------------------------------------------*
    // *
    // *  Im3_write_Handler  (....)
    // *----------------------------------------------------------------*
    // *  this function handles a record write request on RecordIndex 0xaff3
    // *----------------------------------------------------------------*
    PNIO_UINT32  Im3_write_Handler
		    (
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
#if (1 != IOD_USED_WITH_XHIF_HOST)
        PNIO_UINT32 Status;
#endif

        PNIO_UINT32     PeriphRealCfgInd = 0;
        PNPB_REAL_SUB*  pSub = NULL;

        // **** check if subslot is plugged, else error ****
        OsEnterX (OS_MUTEX_PNPB);
        pSub   = pnpb_sub_real_getp  (Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        OsExitX (OS_MUTEX_PNPB);

        if (pSub == NULL)
        {
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"

#if (1 != IOD_USED_WITH_XHIF_HOST)
            return (PNIO_NOT_OK);
#endif
        }
        else
        {
            PeriphRealCfgInd = pSub->PeriphRealCfgInd;
        }

#if (1 != IOD_USED_WITH_XHIF_HOST)
        // **** check buffer length, set total data length ***
        if (*pBufLen >  sizeof (IM3_STRUCT))        // plausibility check
        { // ***** error *****
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb1;  // example: Error Class 11 = Access, ErrorNr 1 = "write length error"
            return (PNIO_NOT_OK);
        }
        else
            *pBufLen = sizeof (IM3_DATA);    // length without blockheader

        Status =  Bsp_im_data_store (PNIO_NVDATA_IM3,       // nv data type: device name
                                     pBuffer + sizeof (REC_IO_BLOCKHDR),               // source pointer to the devicename
                                     *pBufLen,              // length of the device name
                                     PeriphRealCfgInd);

        if (Status == PNIO_OK)
        {
            PNIO_printf("##WRITE IM3 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                         Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);
        }
        else
        {
            // *** return error ***
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb6;  // example: Error Class 11 = Access, ErrorNr 6 = "access denied"
            pPnioState->ErrCode2  = 0;     // here dont care
            pPnioState->AddValue1 = 0;     // here dont care
            pPnioState->AddValue2 = 0;     // here dont care
            PNIO_printf("##ERROR WRITE IM3 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                         Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);
        }

        return (Status);

#else
        /* Save parameters to structures and send it to host with request */
        /* Host will send them back together with valid data */

        PNIOext_cbf_im_write(3, Api, pAddr, pBufLen, pBuffer + sizeof (REC_IO_BLOCKHDR), PeriphRealCfgInd, pPnioState);
        return (PNIO_OK);
#endif
    }


    // *----------------------------------------------------------------*
    // *
    // *  Im4_write_Handler  (....)
    // *----------------------------------------------------------------*
    // *  this function handles a record write request on RecordIndex 0xaff4
    // *----------------------------------------------------------------*
    PNIO_UINT32  Im4_write_Handler
		    (
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
#if (1 != IOD_USED_WITH_XHIF_HOST)
        PNIO_UINT32 Status;
#endif

        PNIO_UINT32     PeriphRealCfgInd = 0;
        PNPB_REAL_SUB*  pSub = NULL;

        // **** check if subslot is plugged, else error ****
        OsEnterX (OS_MUTEX_PNPB);
        pSub   = pnpb_sub_real_getp  (Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        OsExitX (OS_MUTEX_PNPB);

        if (pSub == NULL)
        {
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"

#if (1 != IOD_USED_WITH_XHIF_HOST)
            return (PNIO_NOT_OK);
#endif
        }
        else
        {
            PeriphRealCfgInd = pSub->PeriphRealCfgInd;
        }

#if (1 != IOD_USED_WITH_XHIF_HOST)
        // **** check buffer length, set total data length ***
        if (*pBufLen >  sizeof (IM4_STRUCT))        // plausibility check
        { // ***** error *****
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb1;  // example: Error Class 11 = Access, ErrorNr 1 = "write length error"

            return (PNIO_NOT_OK);
        }
        else
            *pBufLen = sizeof (IM4_DATA); // length without record header

        Status =  Bsp_im_data_store (PNIO_NVDATA_IM4,       // nv data type: device name
                                     pBuffer  + sizeof (REC_IO_BLOCKHDR),               // source pointer to the devicename
                                     *pBufLen,              // length of the device name
                                     PeriphRealCfgInd);

        if (Status == PNIO_OK)
        {
            PNIO_printf("##WRITE IM4 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                         Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);
        }
        else
        {
            // *** return error ***
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb6;  // example: Error Class 11 = Access, ErrorNr 6 = "access denied"
            pPnioState->ErrCode2  = 0;     // here dont care
            pPnioState->AddValue1 = 0;     // here dont care
            pPnioState->AddValue2 = 0;     // here dont care
            PNIO_printf("##ERROR WRITE IM4 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                         Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);
        }

        return (Status);

#else
        /* Save parameters to structures and send it to host with request */
        /* Host will send them back together with valid data */

        PNIOext_cbf_im_write(4, Api, pAddr, pBufLen, pBuffer + sizeof (REC_IO_BLOCKHDR), PeriphRealCfgInd, pPnioState);
        return (PNIO_OK);
#endif
    }

#endif //IOD_INCLUDE_IM0_4

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
