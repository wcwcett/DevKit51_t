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
/*  F i l e               &F: pnpb_im_func.c                            :F&  */
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
/*                                                                           */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*  2016-12-23                      fix06.4  adjust IM1..4 error handling,   */
/*                                  adjust some error responses in           */
/*                                  im-write handler from 0xde to 0xdf       */
/*---------------------------------------------------------------------------*/
#include "compiler.h"

#if IOD_INCLUDE_IM0_4            // IM 0..4 is handled inside the PN stack

    #include "pniousrd.h"
    #include "trc_if.h"

    #include "pndv_inc.h"
    #include "pnpb.h"
    #include "nv_data.h"
    #include "usriod_cfg.h"

    #define LTRC_ACT_MODUL_ID   203
    #define PNPB_MODULE_ID      203


    // ********* external data *************

    // ********* static data *************
    static  IM0_DATA            Im0Device;
	static  PNIO_DEV_ADDR		AddrProxy;

    // ********* public data *************
    PNIO_UINT32      PnioLogRecordRdWr = 0;


    // *-----------------------------------------------------------------------
    // *    some defines for IM0 data, that may be unchanged
    // *   (user configurated defines for IM0  see useriod_cfg.h)
    // *-----------------------------------------------------------------------
    #define IM0_VERSMAJ             1       // must be 01 in this version
    #define IM0_VERSMIN             1       // must be 01 in this version

    // *----------------------------------------------------------------*
    // *
    // *  FillUpStringWithBlanks (PNIO_VOID)
    // *----------------------------------------------------------------*
    // *
    // *   parses the string for a zero terminator '\0' and replaces it
    // *   with a blank. The rest of the string is filled with blanks,
    // *   until the specified stringlength has been reached.
    // *
    // *
    // *----------------------------------------------------------------*
    static PNIO_VOID FillUpStringWithBlanks (PNIO_UINT8* pStr, PNIO_UINT32 StringLen)
    {
        PNIO_UINT32 TerminatorOfs;   // offset of terminator character \0 in string

        for (TerminatorOfs=0; TerminatorOfs < StringLen; TerminatorOfs++)
        {
            if (*(pStr+TerminatorOfs) == 0)
            {
                *(pStr+TerminatorOfs) = ' ';
                break;
            }

        }
        if ((StringLen - TerminatorOfs) >= 2)
        {
            PNPB_MEMSET(pStr + TerminatorOfs+1, ' ', StringLen - TerminatorOfs - 1);
        }
    }


    // *----------------------------------------------------------------*
    // *
    // *  pnpb_copy_Im0Data  (....)
    // *----------------------------------------------------------------*
    // *  copies the IM0 data from host address to destination address
    // *  and converts the endian format from host to network (big endian)
    // *  A zero terminated string is filled up with blanks until the
    // *  specified stringlength  (character \0 is not allowed here..)
    // *----------------------------------------------------------------*
    //lint -e{832, 578} Parameter 'Symbol' not explicitly declared, int assumed
    PNIO_VOID pnpb_copy_Im0Data (IM0_DATA*           pIm0Dst,     // destination address, IM0 in network order (big endian)
                                 IM0_DATA*           pIm0Src,     // source address, IM0 in host order
                                 PNIO_IM0_SUPP_ENUM  Im0Support)  // stands for NOTHING / SUBMODULE / MODULE / DEVICE
    {
            // *--------------------------------------------------------
            // *  copy IM0 data to the specified destination
            // *--------------------------------------------------------
            PNPB_COPY_BYTE(pIm0Dst, pIm0Src, sizeof (IM0_DATA));

            // **** fill rest with blanks (zero-terminator is not allowed here...) ***
            FillUpStringWithBlanks ((PNIO_UINT8*)&(pIm0Dst->OrderId[0]), sizeof (Im0Device.OrderId));
            FillUpStringWithBlanks ((PNIO_UINT8*)&(pIm0Dst->SerNum[0]),  sizeof (Im0Device.SerNum));


            // *--------------------------------------------------------
            // *  save IM0 data set, that is proxy for the device
            // *  this set is responded to an IM0 read request, if the
            // *  specified submodule has not its own IM0 data
            // *--------------------------------------------------------
            if (Im0Support & PNIO_IM0_DEVICE)
            {
                PNPB_COPY_BYTE(&Im0Device, pIm0Dst, sizeof (IM0_DATA));
            }
    }

	// *----------------------------------------------------------------*
	// *
	// *  pnpb_save_proxy  (....)
	// *----------------------------------------------------------------*
	// *  copies the address of the proxy to local variable to be 
	// *  later used for reading of IM data of modules without its own
	// *  data.
	// *----------------------------------------------------------------*
	PNIO_VOID pnpb_save_proxy(PNIO_DEV_ADDR* pAddr)  // geographical address
	{
		// *--------------------------------------------------------
		// *  copy address to the specified destination
		// *--------------------------------------------------------
		PNPB_COPY_BYTE(&AddrProxy, pAddr, sizeof(PNIO_DEV_ADDR));
	}

    // *----------------------------------------------------------------*
    // *
    // *  pnpb_Im0_read_Handler  (....)
    // *----------------------------------------------------------------*
    // *  this function handles a record request on RecordIndex 0xaff0,
    // *  which is specified as a "I&M0" request.
    // *  If an error occures, the error structure PNIO_ERR_STAT is filled.
    // *  See PNIO specification for more details, how to fill the error
    // *  structure.
    // *----------------------------------------------------------------*
    PNIO_UINT32  pnpb_Im0_read_Handler
		    (
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
        PNPB_REAL_SUB*  pSub = NULL;		// pointer to subslot
        IM0_STRUCT*     pIm0 = (IM0_STRUCT*)    pBuffer;

	    if (PnioLogRecordRdWr)
            PNIO_printf ("##READ IM0 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
				         Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);

        // **** check if subslot is plugged, else error ****
        PNPB_ENTER(OS_MUTEX_PNPB);
        pSub   = pnpb_sub_real_getp  (Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        if (pSub == NULL)
        {
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"
            PNPB_EXIT(OS_MUTEX_PNPB);
            return (PNIO_NOT_OK);
          }

        // **** check buffer length, set total data length ***
        if (*pBufLen <  sizeof (IM0_STRUCT))        // plausibility check
        { // ***** error *****
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xc0;  // example: Error Class 12 = resource, ErrorNr  = "write constrain conflict"
            PNPB_EXIT(OS_MUTEX_PNPB);
            return (PNIO_NOT_OK);
        }
        else
            *pBufLen = sizeof (IM0_STRUCT);


        // **** fill IM0 blockheader ****
        pIm0->BlockHeader.Type      = PNPB_HTONS(0x0020);              // must be 0x0020
        pIm0->BlockHeader.Len       = PNPB_HTONS(sizeof (IM0_STRUCT) - sizeof (REC_IO_BLOCKHDR) + 2); // blocklength, header excluded
        pIm0->BlockHeader.Version   = PNPB_HTONS(0x0100);              // must be 0x0100 (see PNIO specification)


        // **** copy IM0 data from subslot or from device (as a proxy) ****
        if (pSub->Im0Support)
        {
            PNPB_COPY_BYTE(&pIm0->IM0, &pSub->Im0Dat, sizeof (IM0_DATA)); // subslot has own im0 data
        }
        else
        {
            PNPB_COPY_BYTE(&pIm0->IM0, &Im0Device, sizeof (IM0_DATA));    // subslot has not own im0 data --> take from device
        }

        PNPB_EXIT(OS_MUTEX_PNPB);

        // *--------------------------------------------------------
        // * convert into network format (big endian)
        // *--------------------------------------------------------
        pIm0->IM0.VendorId       = PNPB_HTONS(pIm0->IM0.VendorId);
        pIm0->IM0.HwRevision     = PNPB_HTONS(pIm0->IM0.HwRevision);
        pIm0->IM0.Revcnt         = PNPB_HTONS(pIm0->IM0.Revcnt);
        pIm0->IM0.ProfId         = PNPB_HTONS(pIm0->IM0.ProfId);
        pIm0->IM0.ProfSpecTyp    = PNPB_HTONS(pIm0->IM0.ProfSpecTyp);
        pIm0->IM0.ImXSupported   = PNPB_HTONS(pIm0->IM0.ImXSupported);

        // *** clear error structure ***
        PNPB_MEMSET(pPnioState, 0, sizeof (PNIO_ERR_STAT));
        return (PNIO_OK);
    }


    // *----------------------------------------------------------------*
    // *
    // *  pnpb_Im1_read_Handler  (....)
    // *----------------------------------------------------------------*
    // *  this function handles a record read request on RecordIndex 0xaff1
    // *----------------------------------------------------------------*
    PNIO_UINT32  pnpb_Im1_read_Handler
		    (
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user (including blockheader)
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
        IM1_STRUCT*     pIm = (IM1_STRUCT*)    pBuffer;
        IM1_DATA*       pImDatTmp;
        PNIO_UINT32     ImDatLen;
        PNIO_UINT32     Status;
        PNPB_REAL_SUB*  pSub = NULL;        // pointer to subslot

	    if (PnioLogRecordRdWr)
	        PNIO_printf ("##READ IM1 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
				         Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);

       // **** check if subslot is plugged, else error ****
        PNPB_ENTER(OS_MUTEX_PNPB);
        pSub = pnpb_sub_real_getp( Api, pAddr->Geo.Slot, pAddr->Geo.Subslot );
        PNPB_EXIT(OS_MUTEX_PNPB);
        if (pSub == NULL)
        {
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"
            return (PNIO_NOT_OK);
         }

        if ((pSub->Im0Support == PNIO_IM0_NOTHING) && (Im0Device.ImXSupported & IM1_SUPPORT_MSK))
        {
            /* Return data of proxy */
            PNPB_ENTER(OS_MUTEX_PNPB);
            pSub = pnpb_sub_real_getp(Api, AddrProxy.Geo.Slot, AddrProxy.Geo.Subslot);
            PNPB_EXIT(OS_MUTEX_PNPB);
            if (pSub == NULL)
            {
                pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
                pPnioState->ErrDecode = 0x80;  // PNIORW
                pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"
                return (PNIO_NOT_OK);
            }
        }

        if (((pSub->Im0Support)      && ((pSub->Im0Dat.ImXSupported & IM1_SUPPORT_MSK) == 0)) ||
            ((pSub->Im0Support == 0) && ((Im0Device.ImXSupported    & IM1_SUPPORT_MSK) == 0)))
        {
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb0;  // example: Error Class 11 = Access, ErrorNr  = 0 "invalid index"
            return (PNIO_NOT_OK);
        }


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
        pIm->BlockHeader.Type      = PNPB_HTONS(0x0021);              // must be 0x0021
        pIm->BlockHeader.Len       = PNPB_HTONS(sizeof (IM1_STRUCT) - sizeof (REC_IO_BLOCKHDR) + 2); // blocklength, header excluded
        pIm->BlockHeader.Version   = PNPB_HTONS(0x0100);              // must be 0x0100 (see PNIO specification)

	    // *--------------------------------------------------------------
	    // * read IM1 data from NV memory and save in pIm1
	    // *--------------------------------------------------------------
        Status = Bsp_im_data_restore (PNIO_NVDATA_IM1, // data type
                                      (PNIO_VOID**) &pImDatTmp,            // data pointer (allocated by
                                      &ImDatLen, pSub->PeriphRealCfgInd);

        if (Status != PNIO_OK)
        {
            Bsp_nv_data_memfree (pImDatTmp);
            return (Status);
        }

        PNPB_COPY_BYTE(&pIm->IM1,
                       pImDatTmp,
                       sizeof (IM1_DATA));
	    Bsp_nv_data_memfree (pImDatTmp);


        // *** clear error structure ***
        PNPB_MEMSET(pPnioState, 0, sizeof (PNIO_ERR_STAT));
        return (PNIO_OK);
    }


    // *----------------------------------------------------------------*
    // *
    // *  pnpb_Im2_read_Handler  (....)
    // *----------------------------------------------------------------*
    // *  this function handles a record read request on RecordIndex 0xaff2
    // *----------------------------------------------------------------*
    PNIO_UINT32  pnpb_Im2_read_Handler
		    (
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user (including blockheader)
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
        IM2_STRUCT*     pIm = (IM2_STRUCT*)    pBuffer;
        IM2_DATA*       pImDatTmp;
        PNIO_UINT32     ImDatLen;
        PNIO_UINT32     Status;
        PNPB_REAL_SUB*  pSub = NULL;        // pointer to subslot

	    if (PnioLogRecordRdWr)
	        PNIO_printf ("##READ IM2 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
				         Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);

        // **** check if subslot is plugged, else error ****
        PNPB_ENTER(OS_MUTEX_PNPB);
        pSub = pnpb_sub_real_getp( Api, pAddr->Geo.Slot, pAddr->Geo.Subslot );
        PNPB_EXIT(OS_MUTEX_PNPB);
        if (pSub == NULL)
        {
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"
            return (PNIO_NOT_OK);
        }

        if ((pSub->Im0Support == PNIO_IM0_NOTHING) && (Im0Device.ImXSupported & IM2_SUPPORT_MSK))
        {
            /* Return data of proxy */
            PNPB_ENTER(OS_MUTEX_PNPB);
            pSub = pnpb_sub_real_getp(Api, AddrProxy.Geo.Slot, AddrProxy.Geo.Subslot);
            PNPB_EXIT(OS_MUTEX_PNPB);
            if (pSub == NULL)
            {
                pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
                pPnioState->ErrDecode = 0x80;  // PNIORW
                pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"
                return (PNIO_NOT_OK);
            }
        }

        if (((pSub->Im0Support)      && ((pSub->Im0Dat.ImXSupported & IM2_SUPPORT_MSK) == 0)) ||
            ((pSub->Im0Support == 0) && ((Im0Device.ImXSupported    & IM2_SUPPORT_MSK) == 0)))
        {
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb0;  // example: Error Class 11 = Access, ErrorNr  = 0 "invalid index"
            return (PNIO_NOT_OK);
        }


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
        pIm->BlockHeader.Type      = PNPB_HTONS(0x0022);              // must be 0x0022
        pIm->BlockHeader.Len       = PNPB_HTONS(sizeof (IM2_STRUCT) - sizeof (REC_IO_BLOCKHDR) + 2); // blocklength, header excluded
        pIm->BlockHeader.Version   = PNPB_HTONS(0x0100);              // must be 0x0100 (see PNIO specification)

	    // *--------------------------------------------------------------
	    // * read IM2 data from NV memory and save in pIm2
	    // *--------------------------------------------------------------
        Status = Bsp_im_data_restore (PNIO_NVDATA_IM2, // data type
                                      (PNIO_VOID**) &pImDatTmp,            // data pointer (allocated by
                                      &ImDatLen, pSub->PeriphRealCfgInd);

        if (Status != PNIO_OK)
        {
            Bsp_nv_data_memfree (pImDatTmp);
            return (Status);
        }

        PNPB_COPY_BYTE(&pIm->IM2,
                       pImDatTmp,
                       sizeof (IM2_DATA));
	    Bsp_nv_data_memfree (pImDatTmp);


        // *** clear error structure ***
        PNPB_MEMSET(pPnioState, 0, sizeof (PNIO_ERR_STAT));
        return (PNIO_OK);
    }


    // *----------------------------------------------------------------*
    // *
    // *  pnpb_Im3_read_Handler  (....)
    // *----------------------------------------------------------------*
    // *  this function handles a record read request on RecordIndex 0xaff3
    // *----------------------------------------------------------------*
    PNIO_UINT32  pnpb_Im3_read_Handler
		    (
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user (including blockheader)
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
        IM3_STRUCT*     pIm = (IM3_STRUCT*)    pBuffer;
        IM3_DATA*       pImDatTmp;
        PNIO_UINT32     ImDatLen;
        PNIO_UINT32     Status;
        PNPB_REAL_SUB*  pSub = NULL;        // pointer to subslot

	    if (PnioLogRecordRdWr)
	        PNIO_printf ("##READ IM3 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
				         Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);

        // **** check if subslot is plugged, else error ****
        PNPB_ENTER(OS_MUTEX_PNPB);
        pSub = pnpb_sub_real_getp( Api, pAddr->Geo.Slot, pAddr->Geo.Subslot );
        PNPB_EXIT(OS_MUTEX_PNPB);
        if (pSub == NULL)
        {
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"
            return (PNIO_NOT_OK);
        }

        if ((pSub->Im0Support == PNIO_IM0_NOTHING) && (Im0Device.ImXSupported & IM3_SUPPORT_MSK))
        {
            /* Return data of proxy */
            PNPB_ENTER(OS_MUTEX_PNPB);
            pSub = pnpb_sub_real_getp(Api, AddrProxy.Geo.Slot, AddrProxy.Geo.Subslot);
            PNPB_EXIT(OS_MUTEX_PNPB);
            if (pSub == NULL)
            {
                pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
                pPnioState->ErrDecode = 0x80;  // PNIORW
                pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"
                return (PNIO_NOT_OK);
            }
        }

        if (((pSub->Im0Support)      && ((pSub->Im0Dat.ImXSupported & IM3_SUPPORT_MSK) == 0)) ||
            ((pSub->Im0Support == 0) && ((Im0Device.ImXSupported    & IM3_SUPPORT_MSK) == 0)))
        {
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb0;  // example: Error Class 11 = Access, ErrorNr  = 0 "invalid index"
            return (PNIO_NOT_OK);
        }

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
        pIm->BlockHeader.Type      = PNPB_HTONS(0x0023);              // must be 0x0023
        pIm->BlockHeader.Len       = PNPB_HTONS(sizeof (IM3_STRUCT) - sizeof (REC_IO_BLOCKHDR) + 2); // blocklength, header excluded
        pIm->BlockHeader.Version   = PNPB_HTONS(0x0100);              // must be 0x0100 (see PNIO specification)

	    // *--------------------------------------------------------------
	    // * read IM2 data from NV memory and save in pIm2
	    // *--------------------------------------------------------------
        Status = Bsp_im_data_restore (PNIO_NVDATA_IM3, // data type
                                      (PNIO_VOID**) &pImDatTmp,            // data pointer (allocated by
                                      &ImDatLen, pSub->PeriphRealCfgInd);

        if (Status != PNIO_OK)
        {
            Bsp_nv_data_memfree (pImDatTmp);
            return (Status);
        }

        PNPB_COPY_BYTE(&pIm->IM3,
                       pImDatTmp,
                       sizeof (IM3_DATA));
	    Bsp_nv_data_memfree (pImDatTmp);


        // *** clear error structure ***
        PNPB_MEMSET(pPnioState, 0, sizeof (PNIO_ERR_STAT));
        return (PNIO_OK);
    }


    // *----------------------------------------------------------------*
    // *
    // *  pnpb_Im4_read_Handler  (....)
    // *----------------------------------------------------------------*
    // *  this function handles a record read request on RecordIndex 0xaff4
    // *----------------------------------------------------------------*
    PNIO_UINT32  pnpb_Im4_read_Handler
		    (
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
        IM4_STRUCT*     pIm = (IM4_STRUCT*)    pBuffer;
        IM4_DATA*       pImDatTmp;
        PNIO_UINT32     ImDatLen;
        PNIO_UINT32     Status;
        PNPB_REAL_SUB*  pSub = NULL;        // pointer to subslot

	    if (PnioLogRecordRdWr)
	        PNIO_printf ("##READ IM4 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
				         Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);

        // **** check if subslot is plugged, else error ****
        PNPB_ENTER(OS_MUTEX_PNPB);
        pSub = pnpb_sub_real_getp( Api, pAddr->Geo.Slot, pAddr->Geo.Subslot );
        PNPB_EXIT(OS_MUTEX_PNPB);
        if (pSub == NULL)
        {
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"
            return (PNIO_NOT_OK);
        }

        if ((pSub->Im0Support == PNIO_IM0_NOTHING) && (Im0Device.ImXSupported & IM4_SUPPORT_MSK))
        {
            /* Return data of proxy */
            PNPB_ENTER(OS_MUTEX_PNPB);
            pSub = pnpb_sub_real_getp(Api, AddrProxy.Geo.Slot, AddrProxy.Geo.Subslot);
            PNPB_EXIT(OS_MUTEX_PNPB);
            if (pSub == NULL)
            {
                pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
                pPnioState->ErrDecode = 0x80;  // PNIORW
                pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"
                return (PNIO_NOT_OK);
            }
        }

        if (((pSub->Im0Support)      && ((pSub->Im0Dat.ImXSupported & IM4_SUPPORT_MSK) == 0)) ||
            ((pSub->Im0Support == 0) && ((Im0Device.ImXSupported    & IM4_SUPPORT_MSK) == 0)))
        {
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb0;  // example: Error Class 11 = Access, ErrorNr  = 0 "invalid index"
            return (PNIO_NOT_OK);
        }

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
        pIm->BlockHeader.Type      = PNPB_HTONS(0x0024);              // must be 0x0024
        pIm->BlockHeader.Len       = PNPB_HTONS(sizeof (IM4_STRUCT) - sizeof (REC_IO_BLOCKHDR) + 2); // blocklength, header excluded
        pIm->BlockHeader.Version   = PNPB_HTONS(0x0100);              // must be 0x0100 (see PNIO specification)

	    // *--------------------------------------------------------------
	    // * read IM2 data from NV memory and save in pIm2
	    // *--------------------------------------------------------------
        Status = Bsp_im_data_restore (PNIO_NVDATA_IM4, // data type
                                      (PNIO_VOID**) &pImDatTmp,            // data pointer (allocated by
                                      &ImDatLen, pSub->PeriphRealCfgInd);
        if (Status != PNIO_OK)
        {
            Bsp_nv_data_memfree (pImDatTmp);
            return (Status);
        }

        PNPB_COPY_BYTE(&pIm->IM4,
                       pImDatTmp,
                       sizeof (IM4_DATA));
	    Bsp_nv_data_memfree (pImDatTmp);


        // *** clear error structure ***
        PNPB_MEMSET(pPnioState, 0, sizeof (PNIO_ERR_STAT));
        return (PNIO_OK);
    }



    // *----------------------------------------------------------------*
    // *
    // *  pnpb_Im0_write_Handler  (....)
    // *----------------------------------------------------------------*
    // *  this function handles a record write request on RecordIndex 0xaff0
    // *----------------------------------------------------------------*
    PNIO_UINT32  pnpb_Im0_write_Handler
		    (
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
				PNIO_ERR_STAT		*pPnioState		// 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
        PNIO_UINT32 Status = PNIO_NOT_OK; /* Return error - data read only */

        LSA_UNUSED_ARG (Api);
        LSA_UNUSED_ARG (pAddr);
        LSA_UNUSED_ARG (pBufLen);
        LSA_UNUSED_ARG (pBuffer);

        PNIO_printf ("##WRITE IM0 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                     Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);

        // *** return error:  IM0 is readable only ***
        pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
        pPnioState->ErrDecode = 0x80;  // PNIORW
        pPnioState->ErrCode1  = 0xb6;  // example: Error Class 11 = Access, ErrorNr 6 = "access denied"
        pPnioState->ErrCode2  = 0;     // here dont care
        pPnioState->AddValue1 = 0;     // here dont care
        pPnioState->AddValue2 = 0;     // here dont care

        return (Status);
    }


    // *----------------------------------------------------------------*
    // *
    // *  pnpb_Im1_write_Handler  (....)
    // *----------------------------------------------------------------*
    // *  this function handles a record write request on RecordIndex 0xaff1
    // *----------------------------------------------------------------*
    PNIO_UINT32  pnpb_Im1_write_Handler
		    (
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to write, out: length, write by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
				PNIO_ERR_STAT		*pPnioState		// 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
        PNIO_UINT32 Status;
        PNPB_REAL_SUB*  pSub = NULL;        // pointer to subslot

        PNIO_printf ("##WRITE IM1 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                     Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);

        // **** check if subslot is plugged, else error ****
        PNPB_ENTER(OS_MUTEX_PNPB);
        pSub   = pnpb_sub_real_getp  (Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        PNPB_EXIT(OS_MUTEX_PNPB);

        if (pSub == NULL)
        {
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"

            return (PNIO_NOT_OK);
        }

        if ((pSub->Im0Support == PNIO_IM0_NOTHING) && (Im0Device.ImXSupported & IM1_SUPPORT_MSK))
        {
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb6;  // example: Error Class 11 = Access, ErrorNr  = 0 "access denied"

            return (PNIO_NOT_OK);
        }

        if ((pSub->Im0Dat.ImXSupported & IM1_SUPPORT_MSK) == 0)
        {
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb0;  // example: Error Class 11 = Access, ErrorNr  = 0 "invalid index"

            return (PNIO_NOT_OK);
        }

        // **** check buffer length, set total data length ***
        if (*pBufLen >  sizeof (IM1_STRUCT))        // plausibility check
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
                                     *pBufLen, pSub->PeriphRealCfgInd);      // length of the device name

        if (Status == PNIO_OK)
        {
            PNIO_printf ("##WRITE IM1 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
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
            PNIO_printf ("##ERROR WRITE IM1 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
		                 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);
        }

        return (Status);
    }


    // *----------------------------------------------------------------*
    // *
    // *  pnpb_Im2_write_Handler  (....)
    // *----------------------------------------------------------------*
    // *  this function handles a record write request on RecordIndex 0xaff2
    // *----------------------------------------------------------------*
    PNIO_UINT32  pnpb_Im2_write_Handler
		    (
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
				PNIO_ERR_STAT		*pPnioState		// 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
        PNIO_UINT32 Status;
        PNPB_REAL_SUB*  pSub = NULL;        // pointer to subslot

        PNIO_printf ("##WRITE IM2 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                     Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);

        // **** check if subslot is plugged, else error ****
        PNPB_ENTER(OS_MUTEX_PNPB);
        pSub   = pnpb_sub_real_getp  (Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        PNPB_EXIT(OS_MUTEX_PNPB);

        if (pSub == NULL)
        {
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"

            return (PNIO_NOT_OK);
        }

        if ((pSub->Im0Support == PNIO_IM0_NOTHING) && (Im0Device.ImXSupported & IM2_SUPPORT_MSK))
        {
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb6;  // example: Error Class 11 = Access, ErrorNr  = 0 "access denied"

            return (PNIO_NOT_OK);
        }

        if ((pSub->Im0Dat.ImXSupported & IM2_SUPPORT_MSK) == 0)
        {
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb0;  // example: Error Class 11 = Access, ErrorNr  = 0 "invalid index"

            return (PNIO_NOT_OK);
        }

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

        Status =  Bsp_im_data_store (PNIO_NVDATA_IM2,                       // nv data type: device name
                                     pBuffer + sizeof (REC_IO_BLOCKHDR),    // source pointer to the devicename
                                     *pBufLen, pSub->PeriphRealCfgInd);      // length of the device name

        if (Status == PNIO_OK)
        {
            PNIO_printf ("##WRITE IM2 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
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
            PNIO_printf ("##ERROR WRITE IM2 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
		                 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);
        }

        return (Status);
    }


    // *----------------------------------------------------------------*
    // *
    // *  iod_Im3_write_Handler  (....)
    // *----------------------------------------------------------------*
    // *  this function handles a record write request on RecordIndex 0xaff3
    // *----------------------------------------------------------------*
    PNIO_UINT32  pnpb_Im3_write_Handler
		    (
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
				PNIO_ERR_STAT		*pPnioState		// 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
        PNIO_UINT32 Status;
        PNPB_REAL_SUB*  pSub = NULL;        // pointer to subslot

        PNIO_printf ("##WRITE IM3 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                     Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);


        // **** check if subslot is plugged, else error ****
        PNPB_ENTER(OS_MUTEX_PNPB);
        pSub   = pnpb_sub_real_getp  (Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        PNPB_EXIT(OS_MUTEX_PNPB);

        if (pSub == NULL)
        {
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"

            return (PNIO_NOT_OK);
        }

        if ((pSub->Im0Support == PNIO_IM0_NOTHING) && (Im0Device.ImXSupported & IM3_SUPPORT_MSK))
        {
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb6;  // example: Error Class 11 = Access, ErrorNr  = 0 "access denied"

            return (PNIO_NOT_OK);
        }

        if ((pSub->Im0Dat.ImXSupported & IM3_SUPPORT_MSK) == 0)
        {
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb0;  // example: Error Class 11 = Access, ErrorNr  = 0 "invalid index"

            return (PNIO_NOT_OK);
        }

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

        Status =  Bsp_im_data_store (PNIO_NVDATA_IM3,                    // nv data type: device name
                                     pBuffer + sizeof (REC_IO_BLOCKHDR), // source pointer to the devicename
                                     *pBufLen, pSub->PeriphRealCfgInd);      // length of the device name

        if (Status == PNIO_OK)
        {
            PNIO_printf ("##WRITE IM3 Data, Api=%d Slot=%d Subslot=%d DatLen=%d\n",
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
            PNIO_printf ("##ERROR WRITE IM3 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
		                 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);
        }

        return (Status);
    }


    // *----------------------------------------------------------------*
    // *
    // *  iod_Im4_write_Handler  (....)
    // *----------------------------------------------------------------*
    // *  this function handles a record write request on RecordIndex 0xaff4
    // *----------------------------------------------------------------*
    PNIO_UINT32  pnpb_Im4_write_Handler
		    (
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
				PNIO_ERR_STAT		*pPnioState		// 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    )
    {
        PNIO_UINT32 Status;
        PNPB_REAL_SUB*  pSub = NULL;        // pointer to subslot

        PNIO_printf ("##WRITE IM4 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
                     Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);


        // **** check if subslot is plugged, else error ****
        PNPB_ENTER(OS_MUTEX_PNPB);
        pSub   = pnpb_sub_real_getp  (Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        PNPB_EXIT(OS_MUTEX_PNPB);

        if (pSub == NULL)
        {
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb2;  // example: Error Class 11 = Access, ErrorNr  = 2 "invalid slot/subslot"

            return (PNIO_NOT_OK);
        }

        if ((pSub->Im0Support == PNIO_IM0_NOTHING) && (Im0Device.ImXSupported & IM4_SUPPORT_MSK))
        {
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb6;  // example: Error Class 11 = Access, ErrorNr  = 0 "access denied"

            return (PNIO_NOT_OK);
        }

        if ((pSub->Im0Dat.ImXSupported & IM4_SUPPORT_MSK) == 0)
        {
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb0;  // example: Error Class 11 = Access, ErrorNr  = 0 "invalid index"

            return (PNIO_NOT_OK);
        }

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

        Status =  Bsp_im_data_store (PNIO_NVDATA_IM4,                    // nv data type: device name
                                     pBuffer + sizeof (REC_IO_BLOCKHDR), // source pointer to the devicename
                                     *pBufLen, pSub->PeriphRealCfgInd);      // length of the device name

        if (Status == PNIO_OK)
        {
            PNIO_printf ("##WRITE IM4 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
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
            PNIO_printf ("##ERROR WRITE IM4 Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
		                 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen);
        }

        return (Status);
    }

#endif //IOD_INCLUDE_IM0_4


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
