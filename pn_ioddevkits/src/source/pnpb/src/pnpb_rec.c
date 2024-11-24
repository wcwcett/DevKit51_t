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
/*  F i l e               &F: pnpb_rec.c                                :F&  */
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


// *** general includes ***
#include "compiler.h"
#include "pniousrd.h"

// *** platform dependent includes ***
#include "trc_if.h"
#include "pndv_inc.h"
#include "pnpb.h"
#include "nv_data.h"


#define LTRC_ACT_MODUL_ID   209
#define PNPB_MODULE_ID      209


// *-----------------------------------------------------------------
// * defines
// *-----------------------------------------------------------------
typedef struct _ASYNC_REC_DATA
{
    PNDV_RQB_DS*       pDsPeri;
    PNIO_UINT8         Add1;
    PNIO_UINT16        Add2;
    PNDV_IFACE_STRUCT* pPndvIf;
} ASYNC_REC_DATA;

typedef union
{
    PNIO_UINT32     vu32;
    PNIO_ERR_STAT   State;   // 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
}   UNION_PNIO_ERR_STAT;

// *-----------------------------------------------------------------
// * static data
// *-----------------------------------------------------------------
static ASYNC_REC_DATA* pAsyncRecReq;

// *-----------------------------------------------------------------
// * local prototypes
// *-----------------------------------------------------------------

PNIO_UINT32 pnpb_ARFSU_UUID_read_Handler( PNIO_UINT16 *pBufLen,	PNIO_UINT8 *pBuffer	);
PNIO_UINT32 pnpb_ARFSU_UUID_write_Handler( PNIO_UINT16 *pBufLen, PNIO_UINT8	*pBuffer );
PNIO_UINT32 pnpb_process_ARFSU_record( PNIO_UINT32 NettoDatLength, PNIO_UINT8 *NettoDatP8 );

// *-----------------------------------------------------------------
// * pnpb_rec_read_write
// *-----------------------------------------------------------------
PNIO_VOID pnpb_rec_read_write (  PNIO_UINT8         Add1,
                                 PNIO_UINT16        Add2,
                                 PNDV_IFACE_STRUCT* pPndvIf)
{
    PNIO_DEV_ADDR   Addr;        // [in] geographical or logical address
    PNIO_UINT32     Ret;
    PNIO_UINT32     NettoDatLength;
    LSA_UINT8*		NettoDatP8;
    PNDV_RQB_DS*    pDsPeri;
#if (1 == IOD_USED_WITH_XHIF_HOST)
    PNIO_UINT32     TriggerPndv = PNIO_FALSE;
#endif
    UNION_PNIO_ERR_STAT   PnioStat;
    PnioStat.vu32 = 0;	// preset to zero

    pPndvIf->ds_rw.suppress_error_if_para_ds = PNIO_FALSE;

    if (PNDV_IFACE_SERVICE_NEW == pPndvIf->ds_rw.state)
    {
        pPndvIf->ds_rw.state = PNDV_IFACE_SERVICE_PROCCESSING;
        pDsPeri = &pPndvIf->ds_rw.dfc_ds_struc_peri;

        /* overstrike with dataset header information */
        if (CM_OPC_SV_AR_READ_IND == LSA_RQB_GET_OPCODE(pPndvIf->ds_rw.ptr))
        {
            // * -----------------------------------------------
            // *    read record
            // * -----------------------------------------------
            Addr.Geo.Slot    = pDsPeri->event_type.u.sv.ar_rec.slot_nr;
            Addr.Geo.Subslot = pDsPeri->event_type.u.sv.ar_rec.subslot_nr;
            NettoDatLength   = pDsPeri->event_type.u.sv.ar_rec.data_length - CM_RECORD_OFFSET;
            NettoDatP8 = ((PNIO_UINT8*)pDsPeri->record_data)+ CM_RECORD_OFFSET; // get pointer to netto data

            pAsyncRecReq = (ASYNC_REC_DATA*)NULL;    // preselection: no async rec data handling
            switch (pDsPeri->event_type.u.sv.ar_rec.record_index)
            {
            	case 0xE050:	// PNDV_RECORD_ARFSU_DATA_ADJUST
            	{
            		Ret = PNIO_NOT_OK;
            		break;
            	}

			#if IOD_INCLUDE_REC8028_8029
				case 0x8028:
				{
					Ret = pnpb_rec8028_Handler ((PNIO_UINT32) Add1, pDsPeri->event_type.u.sv.ar_rec.api,
												 &Addr, &NettoDatLength, NettoDatP8, (PNIO_ERR_STAT*) &PnioStat);
					break;
				}
				case 0x8029:
				{
					Ret = pnpb_rec8029_Handler ((PNIO_UINT32) Add1, pDsPeri->event_type.u.sv.ar_rec.api,
											     &Addr, &NettoDatLength, NettoDatP8, (PNIO_ERR_STAT*) &PnioStat);
					break;
				}
			#else
				case 0x8028:
				{
					Ret = PNIO_cbf_rec_read (PNIO_SINGLE_DEVICE_HNDL, pDsPeri->event_type.u.sv.ar_rec.api,
											 pDsPeri->event_type.ar_nr, pDsPeri->event_type.session_key,
											 pDsPeri->event_type.u.sv.ar_rec.sequence_nr, &Addr,
											 pDsPeri->event_type.u.sv.ar_rec.record_index, &NettoDatLength,
											 NettoDatP8, (PNIO_ERR_STAT*) &PnioStat);
					break;
            	}
				case 0x8029:
				{
					Ret = PNIO_cbf_rec_read (PNIO_SINGLE_DEVICE_HNDL,   pDsPeri->event_type.u.sv.ar_rec.api,
											 pDsPeri->event_type.ar_nr, pDsPeri->event_type.session_key,
											 pDsPeri->event_type.u.sv.ar_rec.sequence_nr, &Addr,
											 pDsPeri->event_type.u.sv.ar_rec.record_index, &NettoDatLength,
											 NettoDatP8, (PNIO_ERR_STAT*) &PnioStat);
					break;
				}
			#endif
			#if IOD_INCLUDE_IM0_4
				case 0xaff0:
				{
					Ret = pnpb_Im0_read_Handler (pDsPeri->event_type.u.sv.ar_rec.api, &Addr, &NettoDatLength,
												 NettoDatP8, (PNIO_ERR_STAT*) &PnioStat);
					break;
				}
				case 0xaff1:
				{
					Ret = pnpb_Im1_read_Handler (pDsPeri->event_type.u.sv.ar_rec.api, &Addr, &NettoDatLength,
												 NettoDatP8, (PNIO_ERR_STAT*) &PnioStat);
					break;
				}
				case 0xaff2:
				{
					Ret = pnpb_Im2_read_Handler (pDsPeri->event_type.u.sv.ar_rec.api, &Addr, &NettoDatLength,
											     NettoDatP8, (PNIO_ERR_STAT*) &PnioStat);
					break;
				}
				case 0xaff3:
				{
					Ret = pnpb_Im3_read_Handler (pDsPeri->event_type.u.sv.ar_rec.api, &Addr, &NettoDatLength,
												 NettoDatP8, (PNIO_ERR_STAT*) &PnioStat);
					break;
				}
				case 0xaff4:
				{
					Ret = pnpb_Im4_read_Handler (pDsPeri->event_type.u.sv.ar_rec.api, &Addr, &NettoDatLength,
												 NettoDatP8, (PNIO_ERR_STAT*) &PnioStat);
					break;
				}
			#endif
                default:
                {
                	Ret = PNIO_cbf_rec_read (PNIO_SINGLE_DEVICE_HNDL,   pDsPeri->event_type.u.sv.ar_rec.api,
                                        	 pDsPeri->event_type.ar_nr, pDsPeri->event_type.session_key,
                                        	 pDsPeri->event_type.u.sv.ar_rec.sequence_nr, &Addr,
                                        	 pDsPeri->event_type.u.sv.ar_rec.record_index, &NettoDatLength,
                                        	 NettoDatP8, (PNIO_ERR_STAT*) &PnioStat);
					 break;
                }
            }

            if (pAsyncRecReq == NULL)
            { // *** default:  synchronous record read ***
                if (Ret == PNIO_OK)
                { // *** OK, set PnioState (IEC61158-6)  to "no error"
                    pDsPeri->event_type.u.sv.ar_rec.cm_pnio_err = CM_PNIO_ERR_NONE; // no error
                    pDsPeri->event_type.u.sv.ar_rec.data_length = CM_RECORD_OFFSET + NettoDatLength;    // ok
                }
                else
                { // *** ERROR occured: copy PnioState (IEC61158-6)
                    pDsPeri->event_type.u.sv.ar_rec.cm_pnio_err = PNPB_HTONL(PnioStat.vu32);
                    pDsPeri->event_type.u.sv.ar_rec.ret_val_1   = PnioStat.State.AddValue1;
                    pDsPeri->event_type.u.sv.ar_rec.ret_val_2   = PnioStat.State.AddValue2;
                    pDsPeri->event_type.u.sv.ar_rec.data_length = CM_RECORD_OFFSET;                     // error, NettoDatLength must be 0
                }
            }
            else
            {// *** asynchronous record read ***
                pAsyncRecReq->Add1    = Add1;
                pAsyncRecReq->Add2    = Add2;
                pAsyncRecReq->pDsPeri = pDsPeri;
                pAsyncRecReq->pPndvIf = pPndvIf;
                return;
            }
        }
        else
        {
            // * -----------------------------------------------
            // *    write record
            // * -----------------------------------------------
            Addr.Geo.Slot    = pDsPeri->event_type.u.sv.ar_rec.slot_nr;
            Addr.Geo.Subslot = pDsPeri->event_type.u.sv.ar_rec.subslot_nr;
            NettoDatLength   = pDsPeri->event_type.u.sv.ar_rec.data_length - CM_RECORD_OFFSET;
            NettoDatP8 = ((PNIO_UINT8*)pDsPeri->record_data)+ CM_RECORD_OFFSET; // get pointer to netto data

            pAsyncRecReq = (ASYNC_REC_DATA*)NULL;    // preselection: no async rec data handling

            switch (pDsPeri->event_type.u.sv.ar_rec.record_index)
            {
            	case 0xE050:	// PNDV_RECORD_ARFSU_DATA_ADJUST
            	{
            		Ret = pnpb_process_ARFSU_record( NettoDatLength, NettoDatP8 );
            		break;
            	}
			#if IOD_INCLUDE_IM0_4
				case 0xaff0:
				{
                    // *** asynchronous record write - IM0 data ***
					Ret = pnpb_Im0_write_Handler (pDsPeri->event_type.u.sv.ar_rec.api, &Addr, &NettoDatLength,
												  NettoDatP8, (PNIO_ERR_STAT*) &PnioStat);
					break;
				}
				case 0xaff1:
				{
                    // *** asynchronous record write - IM1 data ***
					Ret = pnpb_Im1_write_Handler (pDsPeri->event_type.u.sv.ar_rec.api, &Addr, &NettoDatLength,
												  NettoDatP8, (PNIO_ERR_STAT*) &PnioStat);
					break;
				}
				case 0xaff2:
				{
                    // *** asynchronous record write - IM2 data ***
					Ret = pnpb_Im2_write_Handler (pDsPeri->event_type.u.sv.ar_rec.api, &Addr, &NettoDatLength,
												  NettoDatP8, (PNIO_ERR_STAT*) &PnioStat);
					break;
				}
				case 0xaff3:
				{
                    // *** asynchronous record write - IM3 data ***
					Ret = pnpb_Im3_write_Handler (pDsPeri->event_type.u.sv.ar_rec.api, &Addr, &NettoDatLength,
												  NettoDatP8, (PNIO_ERR_STAT*) &PnioStat);
					break;
				}
				case 0xaff4:
				{
                    // *** asynchronous record write - IM4 data ***
					Ret = pnpb_Im4_write_Handler (pDsPeri->event_type.u.sv.ar_rec.api, &Addr, &NettoDatLength,
												  NettoDatP8, (PNIO_ERR_STAT*) &PnioStat);
					break;
				}
			#endif
                default:
                {
                	Ret = PNIO_cbf_rec_write (PNIO_SINGLE_DEVICE_HNDL,   pDsPeri->event_type.u.sv.ar_rec.api,
                                              pDsPeri->event_type.ar_nr, pDsPeri->event_type.session_key,
                                              pDsPeri->event_type.u.sv.ar_rec.sequence_nr, &Addr,
                                              pDsPeri->event_type.u.sv.ar_rec.record_index, &NettoDatLength,
                                              NettoDatP8, (PNIO_ERR_STAT*) &PnioStat);
                	break;
                }
            }

            if (pAsyncRecReq == NULL)
            { // *** default:  synchronous record write ***
            	if (Ret == PNIO_OK)
            	{ // *** OK, set PnioState (IEC61158-6)  to "no error"
                     pDsPeri->event_type.u.sv.ar_rec.cm_pnio_err = CM_PNIO_ERR_NONE; // no error
                     pDsPeri->event_type.u.sv.ar_rec.data_length = CM_RECORD_OFFSET + NettoDatLength;    // ok
            	}
            	else
            	{ // *** ERROR occured: copy PnioState (IEC61158-6)
            		pDsPeri->event_type.u.sv.ar_rec.cm_pnio_err = PNPB_HTONL(PnioStat.vu32);
            		pDsPeri->event_type.u.sv.ar_rec.ret_val_1   = PnioStat.State.AddValue1;
            		pDsPeri->event_type.u.sv.ar_rec.ret_val_2   = PnioStat.State.AddValue2;
            		pDsPeri->event_type.u.sv.ar_rec.data_length = CM_RECORD_OFFSET;                     // error, NettoDatLength must be 0
            	}
            }
            else
            { // *** asynchronous record write ***
            	pAsyncRecReq->Add1    = Add1;
            	pAsyncRecReq->Add2    = Add2;
            	pAsyncRecReq->pDsPeri = pDsPeri;
            	pAsyncRecReq->pPndvIf = pPndvIf;
            	return;
            }

            /* IM data write - async */
            if((pDsPeri->event_type.u.sv.ar_rec.record_index >= 0xaff0)
            		&& (pDsPeri->event_type.u.sv.ar_rec.record_index <= 0xaff4))
            {
            	if(Ret == PNIO_OK)
            	{
            		/* Response will be sent from NV write done function */
            		return;
            	}
            	else
            	{
            		/* Something went wrong - trigger response immediatelly */
            		pnpb_write_event_to_pndv(PNDV_EV_TO_PNDV_DS_RW_DONE, Add1, Add2, NULL);
            		PNPB_TRIGGER_PNDV ();
            		return;
            	}
            }
        }
    }
    else
    {
        /* Avoid pDsPeri uninitialized warning! */
        return;
    }

    // ***** send confirmation to PNPB *****
#if (1 != IOD_USED_WITH_XHIF_HOST)
    pnpb_write_event_to_pndv(PNDV_EV_TO_PNDV_DS_RW_DONE, Add1, Add2, NULL);
    PNPB_TRIGGER_PNDV ();
#else
    /* For user read and writes, this trigger is performed by acyclic event received by XHIF interface */
    /* But non - user record_indexes have to have trigger here */
    if (CM_OPC_SV_AR_READ_IND == LSA_RQB_GET_OPCODE(pPndvIf->ds_rw.ptr))
    {
        switch (pDsPeri->event_type.u.sv.ar_rec.record_index)
        {

            case 0xE050:
#if (PNIO_TRACE == PNIO_TRACE_DK_LSA)
            case 0x7777:    /* get trace header */
            case 0x7778:
#endif
#if IOD_INCLUDE_REC8028_8029
            case 0x8028:
            case 0x8029:
#endif
#if IOD_INCLUDE_IM0_4
            case 0xaff0:
            case 0xaff1:
            case 0xaff2:
            case 0xaff3:
            case 0xaff4:
#endif
            {
                TriggerPndv = PNIO_TRUE;
                break;
            }
            default:
            {
                TriggerPndv = PNIO_FALSE;
                break;
            }
        }

    }
    else
    {
        if(0x8030 == (pDsPeri->event_type.u.sv.ar_rec.record_index))
        {
            TriggerPndv = PNIO_FALSE;
        }
        else if(0x80A0 == (pDsPeri->event_type.u.sv.ar_rec.record_index))
        {
            /* PROFIenergy */
            TriggerPndv = PNIO_FALSE;
        }
        else if((pDsPeri->event_type.u.sv.ar_rec.record_index) <= 0x7fff)
        {
            TriggerPndv = PNIO_FALSE;
        }
        else if((pDsPeri->event_type.u.sv.ar_rec.record_index) == 0xaff0)
        {
            /* IM0 data */
            TriggerPndv = PNIO_FALSE;
        }
        else if((pDsPeri->event_type.u.sv.ar_rec.record_index) == 0xaff1)
        {
            /* IM1 data */
            TriggerPndv = PNIO_FALSE;
        }
        else if((pDsPeri->event_type.u.sv.ar_rec.record_index) == 0xaff2)
        {
            /* IM2 data */
            TriggerPndv = PNIO_FALSE;
        }
        else if((pDsPeri->event_type.u.sv.ar_rec.record_index) == 0xaff3)
        {
            /* IM3 data */
            TriggerPndv = PNIO_FALSE;
        }
        else if((pDsPeri->event_type.u.sv.ar_rec.record_index) == 0xaff4)
        {
            /* IM4 data */
            TriggerPndv = PNIO_FALSE;
        }
        else
        {
            TriggerPndv = PNIO_TRUE;
        }
    }

    if(PNIO_TRUE == TriggerPndv)
    {
        pnpb_write_event_to_pndv(PNDV_EV_TO_PNDV_DS_RW_DONE, Add1, Add2, NULL);
        PNPB_TRIGGER_PNDV ();
    }

#endif  /* #if #else (1 != IOD_USED_WITH_XHIF_HOST)*/
}


// *---------------------------------------------------------------------------
// *  pnpb_rec_read_rsp
// *  asynchronous record read response
// *---------------------------------------------------------------------------
PNIO_UINT32  pnpb_rec_read_rsp
         (
            PNIO_VOID*          pRqHnd,         // request handle from PNIO_cbf_rec_read
            PNIO_UINT8*         pSrcDat,        // data pointer (may be 0 if error)
            PNIO_UINT32         NettoDatLength, // total length of read record data
            PNIO_ERR_STAT*      pPnioStat       // PNIO state pointer (may be 0, if no error)
         )
{
    ASYNC_REC_DATA*       pRecRdReq      = (ASYNC_REC_DATA*) pRqHnd;
    PNDV_RQB_DS*          pDsPeri;
    UNION_PNIO_ERR_STAT*  pUnionPnioStat = (UNION_PNIO_ERR_STAT*)pPnioStat;

    if (pRecRdReq == NULL)
    {
        PNPB_API_TRACE_02(LSA_TRACE_LEVEL_ERROR,  "ERROR PNIO_rec_read_rsp: no rq, pDat=0x%x Len=%d",
                          pSrcDat, NettoDatLength);
        return (PNIO_NOT_OK);
    }

    pDsPeri  = pRecRdReq->pDsPeri;
    if (pDsPeri == NULL)
    {
        PNPB_API_TRACE_00(LSA_TRACE_LEVEL_ERROR,  "ERROR PNIO_rec_read_rsp: pDsPer=0");
        return (PNIO_NOT_OK);
    }

    if (pPnioStat == NULL)
    { // *** OK, set PnioState (IEC61158-6)  to "no error"
    	pDsPeri->event_type.u.sv.ar_rec.cm_pnio_err = CM_PNIO_ERR_NONE; // no error
    	pDsPeri->event_type.u.sv.ar_rec.ret_val_1   = 0;
    	pDsPeri->event_type.u.sv.ar_rec.ret_val_2   = 0;
    }
    else
    { // *** copy detailed PnioState (see IEC61158-6), maybe with or without error
    	pDsPeri->event_type.u.sv.ar_rec.cm_pnio_err = PNPB_HTONL(pUnionPnioStat->vu32);
    	pDsPeri->event_type.u.sv.ar_rec.ret_val_1   = pPnioStat->AddValue1;
    	pDsPeri->event_type.u.sv.ar_rec.ret_val_2   = pPnioStat->AddValue2;
    }

    // ****** set data length and copy data ******
    if (pDsPeri->event_type.u.sv.ar_rec.cm_pnio_err == CM_PNIO_ERR_NONE)
    { // *** OK, no error ****
    	pDsPeri->event_type.u.sv.ar_rec.data_length = CM_RECORD_OFFSET + NettoDatLength;    // ok
    	// **** copy data ***
    	PNPB_COPY_BYTE(((PNIO_UINT8*)pDsPeri->record_data)+ CM_RECORD_OFFSET, pSrcDat, NettoDatLength);
    	PNPB_API_TRACE_02(LSA_TRACE_LEVEL_NOTE_HIGH,  "PNIO_rec_read_rsp OK pDat=0x%x Len=%d", pSrcDat, NettoDatLength);
    }
    else
    { // *** Error occured..****
    	pDsPeri->event_type.u.sv.ar_rec.data_length = CM_RECORD_OFFSET;    // error, NettoDatLength must be 0

        PNPB_API_TRACE_06(LSA_TRACE_LEVEL_WARN,  "PNIO_rec_read_rsp EC=%d ED=%d EC1=$d EC2=%d AV1=%d AV2=%d",
                          pPnioStat->ErrCode,  pPnioStat->ErrDecode, pPnioStat->ErrCode1,
                          pPnioStat->ErrCode2, pPnioStat->AddValue1, pPnioStat->AddValue2);
    }
    // pRecRdReq->pPndvIf->ds_rw.state = PNDV_IFACE_SERVICE_IDLE;   // done by PNDV

    // ***** send confirmation to PNPB *****
    pnpb_write_event_to_pndv(PNDV_EV_TO_PNDV_DS_RW_DONE, pRecRdReq->Add1, pRecRdReq->Add2, NULL);

    PNPB_TRIGGER_PNDV ();

    if (pRqHnd)
    {
         PNPB_FREE_MEM(pRqHnd);
    }
    return (PNIO_OK);
}


// *---------------------------------------------------------------------------
// *  pnpb_rec_write_rsp
// *  asynchronous record write response
// *---------------------------------------------------------------------------
PNIO_UINT32  pnpb_rec_write_rsp
         (
            PNIO_VOID*          pRqHnd,         // request handle from PNIO_cbf_rec_read
            PNIO_UINT32         NettoDatLen,    // total length of written record data
            PNIO_ERR_STAT*      pPnioStat       // PNIO state pointer (may be 0, if no error)
         )
{
    PNDV_RQB_DS*            pDsPeri;
    ASYNC_REC_DATA*         pRecWrReq      = (ASYNC_REC_DATA*) pRqHnd;
    UNION_PNIO_ERR_STAT*    pUnionPnioStat = (UNION_PNIO_ERR_STAT*)pPnioStat;

    if (pRecWrReq == NULL)
    {
        PNPB_API_TRACE_01(LSA_TRACE_LEVEL_ERROR,  "ERROR PNIO_rec_write_rsp: no rq, Len=%d", NettoDatLen);
        return (PNIO_NOT_OK);
    }

    pDsPeri  = pRecWrReq->pDsPeri;
    if (pDsPeri == NULL)
    {
        PNPB_API_TRACE_00(LSA_TRACE_LEVEL_ERROR,  "ERROR PNIO_rec_write_rsp: pDsPer=0");
        return (PNIO_NOT_OK);
    }

    if (pPnioStat == NULL)
    { // *** OK, set PnioState (IEC61158-6)  to "no error"
    	pDsPeri->event_type.u.sv.ar_rec.cm_pnio_err = CM_PNIO_ERR_NONE; // no error
    	pDsPeri->event_type.u.sv.ar_rec.ret_val_1   = 0;
    	pDsPeri->event_type.u.sv.ar_rec.ret_val_2   = 0;
    }
    else
    { // *** copy detailed PnioState (see IEC61158-6), maybe with or without error
    	pDsPeri->event_type.u.sv.ar_rec.cm_pnio_err = PNPB_HTONL(pUnionPnioStat->vu32);
    	pDsPeri->event_type.u.sv.ar_rec.ret_val_1   = pPnioStat->AddValue1;
    	pDsPeri->event_type.u.sv.ar_rec.ret_val_2   = pPnioStat->AddValue2;
    }

    // ****** set data length and copy data ******
    if (pDsPeri->event_type.u.sv.ar_rec.cm_pnio_err == CM_PNIO_ERR_NONE)
    { // *** OK, no error ****
       pDsPeri->event_type.u.sv.ar_rec.data_length = CM_RECORD_OFFSET + NettoDatLen;    // ok
       PNPB_API_TRACE_01(LSA_TRACE_LEVEL_NOTE_HIGH,  "PNIO_rec_write_rsp OK Len=%d", NettoDatLen);
    }
    else
    { // *** Error occured..****
    	pDsPeri->event_type.u.sv.ar_rec.data_length = CM_RECORD_OFFSET;                     // error, NettoDatLength must be 0

    	PNPB_API_TRACE_06(LSA_TRACE_LEVEL_WARN,  "PNIO_rec_write_rsp EC=%d ED=%d EC1=$d EC2=%d AV1=%d AV2=%d",
                          pPnioStat->ErrCode,  pPnioStat->ErrDecode, pPnioStat->ErrCode1,
                          pPnioStat->ErrCode2, pPnioStat->AddValue1, pPnioStat->AddValue2);
    }
    // pRecWrReq->pPndvIf->ds_rw.state = PNDV_IFACE_SERVICE_IDLE;  // done by PNDV

    // ***** send confirmation to PNPB *****
    pnpb_write_event_to_pndv(PNDV_EV_TO_PNDV_DS_RW_DONE, pRecWrReq->Add1, pRecWrReq->Add2, NULL);

    PNPB_TRIGGER_PNDV ();

    if (pRqHnd)
    {
    	PNPB_FREE_MEM(pRqHnd);
    }
    return (PNIO_OK);
}


// *---------------------------------------------------------------------------
// *  pnpb_rec_set_rsp_async
// *  set asynchronous response mode (for 1 Request)
// *---------------------------------------------------------------------------
PNIO_VOID*  pnpb_rec_set_rsp_async (PNIO_VOID)
{
	PNPB_ALLOC_MEM((PNIO_VOID**) &pAsyncRecReq, 0, sizeof (ASYNC_REC_DATA));
    return (pAsyncRecReq);
}

/**
 * @brief Processes ARFSU data record
 *
 * @param[in]	      NettoDatLength      	length of recieved telegram
 * @param[in]         *NettoDatP8        	buffer pointer
 *
 * @return            PNIO_OK             everything is valid
 *                    PNIO_NOT_OK         invalid state
 *
 * 	It restores ARFSU UUID and information if ARFSU is enabled from non volatile memory.
 * 	Both enabled(4B) and ARFSU(16B) are in common buffer, enabled information goes first
 * 	Then the new ARFSU data is compared with stored ones and user is informed
 *
 */
PNIO_UINT32 pnpb_process_ARFSU_record(
		PNIO_UINT32     NettoDatLength,
		PNIO_UINT8		*NettoDatP8
		)
{
	PNIO_UINT8 shift_ptr = 0;
	PNIO_UINT16 FSUHeader, FSULen;
	PNIO_UINT8 ARFSU_enabled, ARFSU_changed;
	PNIO_UINT32 ARFSUMode;


	if( PNPB_ARFSU_RECORD_LEN != NettoDatLength )
	{
		return PNIO_NOT_OK;
	}

	/* 2B BlockType, 2B BlockLen, 2B blockVersion, 2B Padding */
	/* check if ARFSUDataAdjust and valid len */
	FSUHeader = PNPB_NTOHS( ( PNIO_UINT16 )( *( PNIO_UINT16* )( NettoDatP8 + shift_ptr ) ) );
	FSULen = PNPB_NTOHS( ( PNIO_UINT16 )( *( PNIO_UINT16* )( NettoDatP8 + shift_ptr + 2 ) ) );
	if( ( 0x0609 == FSUHeader ) && ( 0x0020 == FSULen ) )
	{
		/* first header ok, do not need it any more */
		shift_ptr += 8;
		/* 2B BlockType, 2B BlockLen, 2B blockVersion, 2B Padding */
		/* check if FSParameterBlock and valid len */
		FSUHeader = PNPB_NTOHS( ( PNIO_UINT16 )( *( PNIO_UINT16* )( NettoDatP8 + shift_ptr ) ) );
		FSULen = PNPB_NTOHS( ( PNIO_UINT16 )( *( PNIO_UINT16* )( NettoDatP8 + shift_ptr + 2 ) ) );
		if( ( 0x0601 == FSUHeader ) && ( 0x0018 == FSULen ) )
		{
			/* second header ok, do not need it any more */
			shift_ptr += 8;
			ARFSUMode = PNPB_NTOHL( ( PNIO_UINT32 )( *( PNIO_UINT32* )( NettoDatP8 + shift_ptr ) ) );
			if( 0x00000001 == ARFSUMode )
			{
				ARFSU_enabled = PNIO_ARFSU_ENABLED;
			}
			else if( 0x00000000 == ARFSUMode )
			{
				ARFSU_enabled = PNIO_ARFSU_DISABLED;
			}
			else
			{
				return PNIO_NOT_OK;
			}
			shift_ptr += 4;
			FSULen = ARFSU_LEN;

			PNIO_UINT8 * TmpBuff;
			PNPB_ALLOC_MEM( ( PNIO_VOID** )&TmpBuff, 0xff, ARFSU_LEN );
			pnpb_ARFSU_UUID_read_Handler( &FSULen, TmpBuff );
			if (0 == (PNPB_CMPR_BYTE(TmpBuff, (PNIO_VOID*)(NettoDatP8 + shift_ptr), ARFSU_LEN)))
			{
				/*old and new ARFSU are same*/
				ARFSU_changed = PNIO_ARFSU_NOT_CHANGED;
			}
			else
			{
				ARFSU_changed = PNIO_ARFSU_CHANGED;
				/* change stored ARFSU */
				pnpb_ARFSU_UUID_write_Handler( &FSULen, ( PNIO_VOID* )( NettoDatP8 + shift_ptr ) );
			}
			PNPB_FREE_MEM(TmpBuff);
			PNIO_cbf_report_ARFSU_record( ARFSU_enabled, ARFSU_changed );
/*
			PNIO_printf("ARFSU_UUID %08x-%04x-%04x-%04x-%04x%08x\n",
					( PNIO_UINT32 )( *( PNIO_UINT32* )( NettoDatP8 + shift_ptr + 0 ) ),
					( PNIO_UINT16 )( *( PNIO_UINT16* )( NettoDatP8 + shift_ptr + 4 ) ),
					( PNIO_UINT16 )( *( PNIO_UINT16* )( NettoDatP8 + shift_ptr + 6 ) ),
					( PNIO_UINT16 )( *( PNIO_UINT16* )( NettoDatP8 + shift_ptr + 8 ) ),
					( PNIO_UINT16 )( *( PNIO_UINT16* )( NettoDatP8 + shift_ptr + 10 ) ),
					( PNIO_UINT32 )( *( PNIO_UINT32* )( NettoDatP8 + shift_ptr + 12 ) )
					);
*/
		}
		else
		{
			PNIO_printf( "ARFSU data adjust record, but unknown FS block type\n" );
			return PNIO_NOT_OK;
		}
	}
	else
	{
		PNIO_printf( "ARFSU data adjust record, but unknown block type\n" );
		return PNIO_NOT_OK;
	}
	return PNIO_OK;
}

/**
 * @brief Restore ARFSU information
 *
 * @param[in, out]    *pBufLen      		in: length to read, out: length, read by user
 * @param[in]         *pBuffer        		buffer pointer
 *
 * @return            PNIO_OK             everything is valid
 *                    PNIO_NOT_OK         invalid state
 *
 * 	It restores ARFSU UUID and information if ARFSU is enabled in non volatile memory.
 * 	Both enabled(4B) and ARFSU(16B) are in common buffer, enabled information goes first
 * 	The length have to be ARFSU_LEN, otherwise function returns error
 *
 */
PNIO_UINT32 pnpb_ARFSU_UUID_read_Handler(
		PNIO_UINT16			*pBufLen,		/* [in, out] in: length to read, out: length, read by user */
		PNIO_UINT8			*pBuffer		/* [in] buffer pointer */
		)
{
	PNIO_UINT32     Status;
	PNIO_UINT8 *	TmpDataPtr;
	PNIO_UINT32		TmpLen = ARFSU_LEN;

	/* check wrong size of requested data */
	if( ARFSU_LEN != ( *pBufLen ) )
	{
		return PNIO_NOT_OK;
	}
	/* check valid pointer to buffer */

	if( NULL == pBuffer )
	{
		return PNIO_NOT_OK;
	}

	/* request data from nv_data */
	Status = Bsp_nv_data_restore( PNIO_NVDATA_ARFSU, 			/* data type */
								 ( PNIO_VOID** )&TmpDataPtr,    /* data pointer (will be allocated inside) */
								 &TmpLen );
	if (Status != PNIO_OK)
	{
		/* read of nv_data failed, free memory */
		Bsp_nv_data_memfree( TmpDataPtr );
		return (Status);
	}

	PNPB_COPY_BYTE(pBuffer, TmpDataPtr, ARFSU_LEN);

	Bsp_nv_data_memfree( TmpDataPtr	);

	return PNIO_OK;
}

/**
 * @brief Store ARFSU information
 *
 * @param[in, out]    *pBufLen      		in: length to read, out: length, read by user
 * @param[in]         *pBuffer        		buffer pointer
 *
 * @return            PNIO_OK             everything is valid
 *                    PNIO_NOT_OK         invalid state
 *
 * 	It stores ARFSU UUID and information if ARFSU is enabled in non volatile memory.
 * 	Both enabled(4B) and ARFSU(16B) are in common buffer, enabled information goes first
 * 	The length have to be ARFSU_LEN, otherwise function returns error
 *
 */
PNIO_UINT32 pnpb_ARFSU_UUID_write_Handler(
		PNIO_UINT16			*pBufLen,		/* [in, out] in: length to read, out: length, read by user */
		PNIO_UINT8			*pBuffer		/* [in] buffer pointer */
		)
{
	PNIO_UINT32     Status;
	PNIO_UINT32		TmpLen = ARFSU_LEN;

	/* check wrong size of requested data */
	if( ARFSU_LEN != ( *pBufLen ) )
	{
		return PNIO_NOT_OK;
	}
	/* check valid pointer to buffer */
	if( NULL == pBuffer )
	{
		return PNIO_NOT_OK;
	}

	/* request data from nv_data */
	Status = Bsp_nv_data_store( PNIO_NVDATA_ARFSU, 				/* data type */
							   ( PNIO_VOID* )pBuffer,    	/* data pointer (will be allocated inside) */
							   TmpLen );


	return Status;
}

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
