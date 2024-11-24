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
/*  F i l e               &F: pnpb_lib_acyc.c                           :F&  */
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

/**
* @file     pnpb_lib_acyc.c
* @brief    PNPB library for XHIF - acyclic functions
*
* XHIF functionality allows user to use ERTEC Devkit only for PN-stack functionalities
* and to realize user functionality on other device. The other device have to upload
* firmware to ERTEC Devkit as a binary and then communicate data.. This can be realized
* via XHIF memory interface.
* This file is such implementation developed for BeagleBone Black driven by TI Sitara
* processor.
*/

#include "pnpb_lib.h"
#include "pnpb_lib_acyc.h"
#include "pnpb_lib_mem_int.h"
#include "usriod_AMR.h"
#include "usriod_PE.h"
#include "usriod_im_func.h"
#include "nv_data.h"

PNIO_UINT32 LastApduStatus;
static PNIO_BOOL wasPlug;
static PNIO_UINT32 LastPluggedSlot;
static PNIO_UINT32 LastPluggedSubslot;

static FILE* trace_file_fd;

/**
 *  @brief AR Connect reached in state machine of establishing AR
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  Information to user that AR was established
 *
 */
PNIO_VOID PNIOext_cbf_ar_connect_ind(PNIO_UINT8 *params)
{
    PNIO_CONNECT_IND_PRM prm;
    prm = *(PNIO_CONNECT_IND_PRM*)params;

    NumOfAr++;      // increment number of running ARs

    printf ("##CONNECT_IND AR=%d AR type=%d sendclock=%d, reduction_ratio_in=%d, reduction_ratio_out=%d\n",
            prm.ArNum, prm.ArType, prm.SendClock, prm.RedRatioIocrIn, prm.RedRatioIocrOut);
}   /* PNIOext_cbf_ar_connect_ind */


/**
 *  @brief Ownership indication
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  Information to user about submodules owned by CPU, which is establishing AR
 *
 */
PNIO_VOID PNIOext_cbf_ar_ownership_ind(PNIO_UINT8* params)
{
    PNIO_UINT32 i;
    PNIO_OWNERSHIP_IND_PRM *prm;

    prm = (PNIO_OWNERSHIP_IND_PRM*)params;

    printf("##OWNERSHIP_IND AR = %d number of submodules = %d\n", prm->ArNum, prm->OwnSub.NumOfPluggedSub);

    for (i = 0; i < prm->OwnSub.NumOfPluggedSub; i++)
    {
        printf("  Api=%d Slot=%d Sub=%d ModID=%d SubID=%d OwnSessKey=%d isWrong=%d\n",
                prm->OwnSub.Sub[i].ApiNum,
                prm->OwnSub.Sub[i].SlotNum,
                prm->OwnSub.Sub[i].SubNum,
                prm->OwnSub.Sub[i].ModIdent,
                prm->OwnSub.Sub[i].SubIdent,
                prm->OwnSub.Sub[i].OwnSessionKey,
                prm->OwnSub.Sub[i].IsWrongSubmod
                );
    }

}   /* PNIOext_cbf_ar_ownership_ind */


/**
 *  @brief Prm end received
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  CPU establishing AR sent prm_end telegram
 *
 */
PNIO_VOID PNIOext_cbf_param_end_ind(PNIO_UINT8* params)
{
    PNIO_PRM_END_IND_PRM *prm;

    prm = (PNIO_PRM_END_IND_PRM*) params;


    if (prm->SubslotNum == 0)
    {
        /* param end for all submodules */
        printf("##PARAM END  for all submodules, ArNum=%d, Session=%d \n", prm->ArNum, prm->SessionKey);
    }
    else
    {
        /* param end for one submodule */
        printf("##PARAM END for Api=%d, Slot=%d, Sub=%d, ArNum=%d, Session=%d \n",
                prm->Api, prm->SlotNum, prm->SubslotNum, prm->ArNum, prm->SessionKey);
    }

    //return (PNIO_SUBMOD_STATE_RUN);     // system generates automatically "application ready"-telegram

}   /* PNIOext_cbf_param_end_ind */


/**
 *  @brief Ready for input update received from CPU
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  Reaction to telegram received from CPU during AR establishment
 *
 */
PNIO_VOID PNIOext_cbf_ready_for_input_update_ind(PNIO_UINT8* params)
{
    PNIO_INPUT_UPDATE_IND_PRM *prm;
    PNIO_UINT32 Status = PNPB_OK;
    PNIO_UINT32 ApduStatus;

    /* Parse received parameters */
    prm = (PNIO_INPUT_UPDATE_IND_PRM*) params;
    /* Perform a first data exchange, to set IOPS and IOCS to STATE_GOOD */
    /* Necessary to set IO controller state to RUN without error. */

    //Status =  PNIO_initiate_data_read (DevHndl);

    if (PNPB_OK == Status)
    {
        //Status =  PNIO_initiate_data_write (DevHndl);
        if (PNPB_OK != Status)
        {
            printf("##Error IO Data Exchange in PNIO_cbf_ready_for_input_update_ind()\n");
        }
    }
    //    PnUsr_ActivateIoDatXch();

    /* Notify, if first "readyForInputUpdate" after AR start or */
    /* because of replugging a submodule. */
    if (prm->InpUpdState == PNIO_AR_STARTUP)
    {
        printf("##READY FOR INPUT UPDATE DURING AR_STARTUP ARnum=%d\n", prm->ArNum);
    }
    else
    {
        printf("##READY FOR INPUT UPDATE DURING AR INDATA  ARnum=%d\n", prm->ArNum);
    }
}   /* PNIOext_cbf_ready_for_input_update_ind */


/**
 *  @brief Data input valid
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  Stack informs user about first valid data received
 *
 */
PNIO_VOID PNIOext_cbf_ar_indata_ind(PNIO_UINT8* params)
{
    PNIO_AR_INDATA_IND_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_AR_INDATA_IND_PRM*) params;

    printf("##AR IN-Data event indication received, ArNum = %xh, Session = %xh\n", prm->ArNum, prm->SessionKey);
}   /* PNIOext_cbf_ar_indata_ind */


/**
 *  @brief AR released - device offline
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  Reason code is provided by the context manager of the pnio stack
 *
 */
PNIO_VOID PNIOext_cbf_ar_disconn_ind(PNIO_UINT8* params)
{
    PNIO_AR_DISCONNECT_IND_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_AR_DISCONNECT_IND_PRM*) params;

    printf("##AR Offline indication ,ArNum = %xh, Session = %xh, Reason = %xh\n",
                  prm->ArNum, prm->SessionKey, prm->ReasonCode);

    if (NumOfAr)
        NumOfAr--;      /* decrement number of running ARs */

    // Stops IO data exchange if AR NR equal to zero.
    if (0 == NumOfAr)
    {
    	PNIOext_DeactivateIoDatXch();
    }

}   /* PNIOext_cbf_ar_disconn_ind */


/**
 *  @brief User functionality as reaction to new ARFSU
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  This function is called when ARFSU write record is received
 *  Function is ready for user functionality
 *  Informs user if ARFSU UUID was changed or not -> if parameterization was changed
 *
 */
PNIO_VOID PNIOext_cbf_report_ARFSU_record(PNIO_UINT8* params)
{

    PNIO_ARFSU_IND_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_ARFSU_IND_PRM*) params;

    if( PNIO_ARFSU_CHANGED == prm->ARFSU_changed )
    {
        printf("New ARFSU UUID -> parameterization of device was changed\n");
    }
}   /* PNIOext_cbf_report_ARFSU_record */


/**
 *  @brief x
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  x
 *
 */
PNIO_VOID PNIOext_cbf_sub_plug_list(PNIO_UINT8* params)
{
    PNIO_UINT32 *pStatusList;           /* list of return-Stati[NumOfSublistEntries] */
    pStatusList = (PNIO_UINT32 *)params;

    /* Caution: data valid for limited time.. memcpy if data needed for further use */
    printf("Sub plug response dummy\n");

}   /* PNIOext_cbf_sub_plug_list */


/**
 *  @brief x
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  x
 *
 */
PNIO_VOID PNIOext_cbf_async_req_done(PNIO_UINT8* params)
{
    PNIO_ASYNC_DONE_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_ASYNC_DONE_PRM*) params;
    PNIO_UINT32 j;
    PNIO_UINT32 Direction;

    if( 0 == prm->ArNum )
    {
        printf("Asynchronous request not sent - no existing AR. Alarmtype=%d Api=%d Slot=%d Subslot=%d\n",
                prm->AlarmType, prm->Api, prm->Addr.Geo.Slot, prm->Addr.Geo.Subslot);
    }
    else
    {
        printf("Asynchronous request ArNum=%d Alarmtype=%d Api=%d Slot=%d Subslot=%d Status = %d\n",
                prm->ArNum, prm->AlarmType, prm->Api, prm->Addr.Geo.Slot, prm->Addr.Geo.Subslot, prm->Status);
    }
    printf("User tag %x\n", prm->Diag_tag );

    if(prm->AlarmType == PNIO_ALM_RET_OF_SUB)
    {
        if( wasPlug == PNIO_FALSE)
        {
            printf("Pull request done\n");
            for (j = 0; j < PNPB_XHIF_MAX_NUM_OF_SUBMODULES; ++j)
            {
                if(pnpb_submodule_params[j] != NULL
                && pnpb_submodule_params[j]->Slot == prm->Addr.ad.Geo1.Slot
                && pnpb_submodule_params[j]->Subslot == prm->Addr.ad.Geo1.Subslot)
                {
                	/* Remember direction of data for a while more */
                	Direction = pnpb_submodule_params[j]->Direction;

                    if(pnpb_submodule_IO_data[j] != NULL)
                    {
                        free(pnpb_submodule_IO_data[j]);
                        pnpb_submodule_IO_data[j] = NULL;
                    }
                    free((void *) pnpb_submodule_params[j]);
                    pnpb_submodule_params[j] = NULL;

                    InDatLen[prm->Addr.ad.Geo1.Slot][prm->Addr.ad.Geo1.Subslot] = 0;
                    OutDatLen[prm->Addr.ad.Geo1.Slot][prm->Addr.ad.Geo1.Subslot] = 0;

                    pnpb_dev_params.NoOfSubmodules--;

                    /* Module address container for IOxS handling */
        			PNIO_DEV_ADDR  Addr;
        			Addr.Geo.Slot = prm->Addr.ad.Geo1.Slot;
        			Addr.Geo.Subslot = prm->Addr.ad.Geo1.Subslot;

                    /* Based on direction, reset IOxS of pulled submodule */
                    /* For INOUT call both, for NO_DATA call none */
                    if(Direction & PNPB_XHIF_DIRECTION_IN)
                    {
                    	PNIO_cbf_data_write_IOxS_only(&Addr, PNIO_S_BAD);
                    }
                    if(Direction & PNPB_XHIF_DIRECTION_OUT)
                    {
                    	PNIO_cbf_data_read_IOxS_only(&Addr, PNIO_S_BAD);
                    }
                }
            }
        }
        else
        {
            printf("Plug request done\n");
        }
    }

}   /* PNIOext_cbf_async_req_done */

PNIO_VOID PNIOext_cbf_async_req_error(PNIO_UINT8 *params)
{
    PNIO_ASYNC_ERROR_PRM *prm;
    PNIO_UINT32 j;
    prm = (PNIO_ASYNC_ERROR_PRM*) params;

    printf ("Error %x occured\n",  prm->ErrorCode);

    if(prm->Type == PNIO_ALM_RET_OF_SUB)
    {
        if( wasPlug == PNIO_FALSE)
        {
           printf("pull error\n");
        }
        else
        {
            printf("plug error\n");
            for (j = 0; j < PNPB_XHIF_MAX_NUM_OF_SUBMODULES; ++j)
            {
                if(pnpb_submodule_params[j] != NULL
                && pnpb_submodule_params[j]->Slot == LastPluggedSlot
                && pnpb_submodule_params[j]->Subslot == LastPluggedSubslot)
                {
                    if(pnpb_submodule_IO_data[j] != NULL)
                    {
                        free(pnpb_submodule_IO_data[j]);
                        pnpb_submodule_IO_data[j] = NULL;
                    }
                    free((void *) pnpb_submodule_params[j]);
                    pnpb_submodule_params[j] = NULL;
                    pnpb_dev_params.NoOfSubmodules--;
                }
            }
        }
    }
}

/**
 *  @brief CPU tries to read data from device
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  Reading from index - returns to stack prepared data under certain index or
 *  error code if there are no data for the requested index
 *
 */
PNIO_VOID PNIOext_cbf_rec_read(PNIO_UINT8* params)
{
    PNIO_RECORD_READ_PRM *prm;

    PNIO_UINT32   Status        = PNPB_OK;
    PNIO_UINT32 slot_nr, subslot_nr;
    PNIO_UINT32 DoHexdumpData;
    PNIO_UINT8 *pBuffer = (PNIO_VOID*) 0;
    PNIO_ERR_STAT       PnioState;     /* 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6 */

    /* Parse received parameters */
    prm = (PNIO_RECORD_READ_PRM*) params;

    /* Reset Err stat */
    PnioState.ErrCode = 0;
    PnioState.ErrDecode = 0;
    PnioState.ErrCode1 = 0;
    PnioState.ErrCode2 = 0;
    PnioState.AddValue1 = 0;
    PnioState.AddValue2 = 0;

    slot_nr         = prm->Addr.Geo.Slot;
    subslot_nr      = prm->Addr.Geo.Subslot;
    DoHexdumpData   = PNPB_FALSE;

    PNPB_UNUSED_ARG (prm->Api);
    PNPB_UNUSED_ARG (prm->ArNum);
    PNPB_UNUSED_ARG (prm->SessionKey);
    PNPB_UNUSED_ARG (prm->SequenceNum);


    // *----------------------------------------------*
    // *  handle special indizes for I&M
    // *----------------------------------------------*
    switch (prm->RecordIndex)
    {
        case 1: // **** user specific startup record *****
                printf("##READ startup-Record (here: demo record is returned), Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
                        prm->Api, slot_nr, subslot_nr, prm->BufLen, prm->SequenceNum);
                if (prm->BufLen > sizeof(DEMO_RECORD))
                    prm->BufLen = sizeof (DEMO_RECORD);
                pBuffer = calloc(prm->BufLen, 1);
                if(NULL != pBuffer)
                {
                    memcpy(pBuffer, DEMO_RECORD, prm->BufLen);
                }
                else
                {
                    printf ("Error: Memory could not be allocated!\n");
                    PNPB_LIB_FATAL;
                }
                break;

        case 2: // **** user specific demo record *****
                printf("##READ Demo-Record , Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
                        prm->Api, slot_nr, subslot_nr, prm->BufLen, prm->SequenceNum);

                if (prm->BufLen > sizeof(DEMO_RECORD))
                    prm->BufLen = sizeof (DEMO_RECORD);
                pBuffer = calloc(prm->BufLen, 1);
                if(NULL != pBuffer)
                {
                    memcpy(pBuffer, DEMO_RECORD, prm->BufLen);
                }
                else
                {
                    printf ("Error: Memory could not be allocated!\n");
                    PNPB_LIB_FATAL;
                }    
                break;

#if EXAMPLE_IPAR_SERVER
case EXAMPL_IPAR_REC_IND: // **** read example data for the iPar Server *****

        prm->BufLen = 4;
        pBuffer = calloc(prm->BufLen, 1);
        if(NULL != pBuffer)
        {
            memcpy(pBuffer,
                &Example_iParData[slot_nr][subslot_nr],
                prm->BufLen);
            printf("##READ iPar example data, Api=%d Slot=%d Subslot=%d Len=%d, RecordIndex=%d\n",
                 prm->Api, slot_nr, subslot_nr, prm->BufLen, prm->RecordIndex);
            DoHexdumpData = PNPB_TRUE;
        }
        else
        {
            printf ("Error: Memory could not be allocated!\n");
            PNPB_LIB_FATAL;
        }		
     break;
#endif

        default:
            // *----------------------------------------------*
            // *  all record number < 0x7fff are user specific,
            // *  but we intercept the simatic specific rec-read
            // *  request on slot=0, subslot=1, index={0,1}, which
            // *  is not supported here.
            // *----------------------------------------------*
            if(prm->RecordIndex <= 0x7fff)
            {
                if(((slot_nr == 0) && (subslot_nr == 1))
                    || (subslot_nr >= 200))
                {   // ** we do not support that request, may be a SIEMENS-specific call      **
                    // ** we respond with an adequate error message, specified in IEC 61158-6 **
                    PnioState.ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
                    PnioState.ErrDecode = 0x80;  // PNIORW
                    PnioState.ErrCode1  = 0xa9;  // example: Error Class 10 = application, ErrorNr 9 = "feature not supported"
                    prm->BufLen = 0;
                }
                else
                {
                    // *----------------------------------------------*
                    // *  read manufacturer specific record data,
                    // *  implemented by the user
                    // *----------------------------------------------*
                    PNIO_UINT8    ReadRecDummyData[] = {"**Data1234 ReadRecord**"};

                    // **** copy dummy data into the buffer, set data-size ***

                    if (prm->BufLen > sizeof(ReadRecDummyData))
                    {
                        prm->BufLen = sizeof(ReadRecDummyData);
                    }
                    pBuffer = calloc(prm->BufLen, 1);
                    if(NULL != pBuffer)
                    {
                        memcpy(pBuffer, ReadRecDummyData, prm->BufLen);
                        printf("##READ_REC RQ,  Api =%d Slot=%d Subslot=%d Index=0x%x, Len=%d, Sequ_nr=%d\n",
                            prm->Api, slot_nr, subslot_nr, prm->RecordIndex, prm->BufLen, prm->SequenceNum);
                    }
                    else
                    {
                        printf ("Error: Memory could not be allocated!\n");
                        PNPB_LIB_FATAL;
                    }
                }
            }
            else
            {
                // *----------------------------------------------*
                // *  all other record indizes >= 0x8000
                // *  may be imlemented by user, otherwise
                // *  we return "invalid index"
                // *----------------------------------------------*
                printf("##READ_REC RQ, Api=%d Slot=%d Subslot=%d Index=0x%x, Len=%d, Sequ_nr=%d\n",
                        prm->Api, slot_nr, subslot_nr, prm->RecordIndex, prm->BufLen, prm->SequenceNum);

                // *** if an error occured, it must be specified according IEC 61158-6
                PnioState.ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
                PnioState.ErrDecode = 0x80;  // PNIORW
                PnioState.ErrCode1  = 0xb0;  // example: Error Class 11 = access, ErrorNr 0 = "invalid index"
                prm->BufLen = 0;
            }
            break;
    }

    // **** print data as hexdump ****
    if (DoHexdumpData)
    {
        PNIO_UINT32 i;
        PNIO_UINT8* p8 = (PNIO_UINT8*) pBuffer;
        printf ("##REC_DATA = ");
        for (i=0; i < prm->BufLen; i++)
        {
            PNIO_UINT32  Val;
            Val = (PNIO_UINT32 ) *(p8+i);;
            printf ("0x%02x ", Val);
        }

        printf ("\n\n");
    }
    /* Send response back to Ertec */
    PNIOext_rec_read_rsp(prm, pBuffer, &PnioState);

    if(((PNIO_VOID*) 0) != pBuffer)
    {
        free(pBuffer);
    }
}   /* PNIOext_cbf_rec_read */


/**
 *  @brief CPU tries to write data to device
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  Device allows to write data under prepared Record index
 *
 */
PNIO_VOID PNIOext_cbf_rec_write(PNIO_UINT8* params)
{

    PNIO_RECORD_WRITE_PRM *prm;

    PNIO_ERR_STAT       PnioState;     /* 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6 */

    // **** copy dummy data into the buffer, set data-size ***
    PNIO_UINT8      WriteRecDummyData[50];
    PNIO_UINT32     i;
    PNIO_UINT32     Status = PNPB_OK;
    PNIO_UINT32     DoHexdumpData = PNPB_TRUE;
    PNIO_UINT32 slot_nr, subslot_nr;

    prm = (PNIO_RECORD_WRITE_PRM *)params;

    /* Reset Errot stat */
    PnioState.ErrCode = 0;
    PnioState.ErrDecode = 0;
    PnioState.ErrCode1 = 0;
    PnioState.ErrCode2 = 0;
    PnioState.AddValue1 = 0;
    PnioState.AddValue2 = 0;


    PNPB_UNUSED_ARG (prm->Api);
    PNPB_UNUSED_ARG (prm->ArNum);
    PNPB_UNUSED_ARG (prm->SessionKey);
    PNPB_UNUSED_ARG (prm->SequenceNum);

    slot_nr         = prm->Addr.Geo.Slot;
    subslot_nr      = prm->Addr.Geo.Subslot;

    // *** check data size (accepted data >= provided data ??) ***
    if (prm->BufLen > sizeof (WriteRecDummyData))
    {
       printf("Error: provided record data (%d) index = %d  > expected data (%d)\n",
               prm->RecordIndex, prm->BufLen, sizeof (WriteRecDummyData));
       Status = PNPB_NOT_OK;
    }
    else
    {
        switch (prm->RecordIndex)
        {
            // *----------------------------------------------*
            // *  user defined records  index 0..7fffh
            // *----------------------------------------------*
#if EXAMPLE_IPAR_SERVER
            case EXAMPL_IPAR_REC_IND: // **** read example data for the iPar Server *****
                    printf("##download iPar example, Api=%d Slot=%d Subslot=%d Len=%d, RecordIndex=%d\n",
                            prm->Api, slot_nr, subslot_nr, prm->BufLen, prm->RecordIndex);

                    // ***** save record data for the specified slot/subslot ****
                    memcpy (&Example_iParData[slot_nr][subslot_nr],
                            (PNIO_UINT8 *)(&(prm->pBuffer)),
                              4);
                    break;
#endif

            case 1: // **** user specific startup record *****
                    printf("##WRITE startup-Record from GSD file , Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
                            prm->Api, slot_nr, subslot_nr, prm->BufLen, prm->SequenceNum);
                    break;
            case 2: // **** user specific demo record *****
                    printf( "##WRITE Demo-Record , Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
                            prm->Api, slot_nr, subslot_nr, prm->BufLen, prm->SequenceNum);
                    break;
            default:
                 Status = PNPB_NOT_OK;
                 break;
        }
    }

    if (Status == PNPB_OK)
    {
        /* Copy the record data into a buffer for further use */
        memcpy(WriteRecDummyData, (PNIO_UINT8 *)(&(prm->pBuffer)), prm->BufLen);
        // **** print data as hexdump ****
        if (DoHexdumpData)
        {
            printf("##WRITE_REC RQ, Api=%d Slot=%d Subslot=%d Index=0x%x, Len=%d, Sequ_nr=%d\n",
                    prm->Api, slot_nr, subslot_nr, prm->RecordIndex, prm->BufLen, prm->SequenceNum);

            printf("##REC_DATA = ");
            for (i=0; i < prm->BufLen; i++)
            {
                PNIO_UINT32  Val;
                Val = (PNIO_UINT32) WriteRecDummyData[i];
                printf("0x%02x ", Val);
            }
            printf("\n\n");
        }
    }
    else
    {
        /* If an error occurred, it must be specified according to IEC 61158-6 */
        PnioState.ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
        PnioState.ErrDecode = 0x80;  // PNIORW
        PnioState.ErrCode1  = 0xb6;  // example: Error Class 11 = Access, ErrorNr 6 = "access denied"
        PnioState.ErrCode2  = 0;     // here don`t care
        PnioState.AddValue1 = 0;     // here don`t care
        PnioState.AddValue2 = 0;     // here don`t care
    }
    /* Send response back to Ertec */
    PNIOext_rec_write_rsp(prm, &PnioState);

}   /* PNIOext_cbf_rec_write */

/**
 *  @brief Request - read record index 0xF880
 *
 *  @param[in]      *params
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_cbf_amr_response_handler(PNIO_UINT8* params)
{
    PNIO_AMR_HANDLER_PRM* prm = (PNIO_AMR_HANDLER_PRM*) params;
    PNIO_UINT8* pData;
    PNIO_ERR_STAT PnioState;     /* 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6 */

    pData = (PNIO_UINT8*) calloc(prm->pBufLen, 1);
    memset(&PnioState, 0, sizeof(PNIO_ERR_STAT));

    /* Call handler */
    AMR_ResponseHandler(&prm->pAddr, &prm->pBufLen, pData, &PnioState);

    /* Call back to Ertec */
    PNIOext_amr_response_handler_rsp(prm, &PnioState, pData);

    /* Cleanup */
    free(pData);
}

/**
 *  @brief Request - read record index 0xF880
 *
 *  @param[in]      *params
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_cbf_pe_response_handler(PNIO_UINT8* params)
{
    PNIO_PE_RESPONSE_HANDLER_PRM* prm = (PNIO_PE_RESPONSE_HANDLER_PRM*) params;
    PNIO_UINT8* pData;
    PNIO_ERR_STAT PnioState;     /* 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6 */

    pData = (PNIO_UINT8*) calloc(prm->pBufLen, 1);
    memset(&PnioState, 0, sizeof(PNIO_ERR_STAT));

    /* Call handler */
    PROFIenergy_ResponseHandler(&prm->pAddr, &prm->pBufLen, pData, &PnioState, prm->ArNum);

    /* Call back to Ertec */
    PNIOext_pe_response_handler_rsp(prm, &PnioState, pData);

    /* Cleanup */
    free(pData);
}

/**
 *  @brief Request - read record index 0xF880
 *
 *  @param[in]      *params
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_cbf_pe_request_handler(PNIO_UINT8* params)
{
    PNIO_PE_REQUEST_HANDLER_PRM* prm = (PNIO_PE_REQUEST_HANDLER_PRM*) params;
    PNIO_ERR_STAT PnioState;     /* 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6 */
    memset(&PnioState, 0, sizeof(PNIO_ERR_STAT));

    /* Call handler */
    PROFIenergy_RequestHandler (&prm->pAddr, &prm->pBufLen, prm->pData, &PnioState, prm->ArNum);

    /* Call back to Ertec */
    PNIOext_pe_request_handler_rsp(&PnioState);
}

/**
 *  @brief Synchronize data downloaded from Ertec with host memory
 *
 *  @param[in]      *params
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_cbf_nv_data_sync(PNIO_UINT8* params)
{
    PNIO_NV_DATA_SYNC_PRM* prm = (PNIO_NV_DATA_SYNC_PRM*) params;

    /* Call nv_data */
    Bsp_nv_data_sync (prm->pData, prm->pBufLen, prm->errOccured);
}

/**
 *  @brief Callback from Ertec - data were written
 *
 *  @param[in]      *params
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_cbf_nv_data_flash_done(PNIO_UINT8* params)
{
    PNIO_NV_DATA_FLASH_DONE_PRM* prm = (PNIO_NV_DATA_FLASH_DONE_PRM*) params;

    /* Call nv_data */
    Bsp_nv_data_flash_done(prm->Status, prm->DatLen, prm->nvDataType);
}

/**
 *  @brief Callback from Ertec - data were written
 *
 *  @param[in]      *params
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_cbf_im_data_flash_done(PNIO_UINT8* params)
{
    PNIO_NV_DATA_FLASH_DONE_PRM* prm = (PNIO_NV_DATA_FLASH_DONE_PRM*) params;

    /* Call nv_data */
    Bsp_im_data_flash_done(prm->Status, prm->DatLen, prm->nvDataType);
}

/**
 *  @brief Callback from Ertec - data reset completed
 *
 *  @param[in]      *params
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_cbf_nv_data_factory_reset_flash_done(PNIO_UINT8* params)
{
    PNIO_NV_DATA_RESET_DONE_PRM* prm = (PNIO_NV_DATA_RESET_DONE_PRM*) params;

    /* Call nv_data */
    Bsp_nv_data_factory_reset_flash_done(prm->Status, prm->DatLen, 0);
}

/**
 *  @brief Callback from Ertec - store rema mem
 *
 *  @param[in]      *params
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_cbf_store_rema_mem(PNIO_UINT8* params)
{
    PNIO_STORE_REMA_MEM_PRM* prm = (PNIO_STORE_REMA_MEM_PRM*) params;

    printf ("##REMA SHADOW MEM STORE total memsize=%d\n", prm->pBufLen);
    // *** store data in non volatile memory ***
    Bsp_nv_data_store (PNIO_NVDATA_PDEV_RECORD, prm->pData, prm->pBufLen);
}

/**
 *  @brief Callback from Ertec - IM data write
 *
 *  @param[in]      *params
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_cbf_im_write(PNIO_UINT8* params)
{
    PNIO_IM_WRITE_PRM* prm = (PNIO_IM_WRITE_PRM*) params;

    if((prm->pPnioState.ErrCode == 0) &&
            (prm->pPnioState.ErrDecode == 0) &&
            (prm->pPnioState.ErrCode1 == 0))
    {
        /* No error comming from Ertec */

        printf((PNIO_INT8*) "##WRITE IM%d Data, Api=%d Slot=%d Subslot=%d Len=%d\n",
            prm->IMidx, prm->Api, prm->Addr.ad.Geo1.Slot, prm->Addr.ad.Geo1.Subslot, prm->pBufLen);

        switch(prm->IMidx)
        {
            case 0:
                /* IM0 data */
                Im0_write_Handler(prm->Api, &prm->Addr, &prm->pBufLen, prm->pData, &prm->pPnioState, prm->PeriphRealCfgInd);
                break;
            case 1:
                /* IM1 data */
                Im1_write_Handler(prm->Api, &prm->Addr, &prm->pBufLen, prm->pData,  &prm->pPnioState, prm->PeriphRealCfgInd);
                break;
            case 2:
                /* IM2 data */
                Im2_write_Handler(prm->Api, &prm->Addr, &prm->pBufLen, prm->pData,  &prm->pPnioState, prm->PeriphRealCfgInd);
                break;
            case 3:
                /* IM3 data */
                Im3_write_Handler(prm->Api, &prm->Addr, &prm->pBufLen, prm->pData,  &prm->pPnioState, prm->PeriphRealCfgInd);
                break;
            case 4:
                /* IM4 data */
                Im4_write_Handler(prm->Api, &prm->Addr, &prm->pBufLen, prm->pData,  &prm->pPnioState, prm->PeriphRealCfgInd);
                break;
            default:
                /* Fatal - it is not an option to have another index */
                PNPB_LIB_FATAL
        }
    }
}

/**
 *  @brief Callback from Ertec - IM data read
 *
 *  @param[in]      *params
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_cbf_im_read(PNIO_UINT8* params)
{
    PNIO_IM_READ_PRM* prm = (PNIO_IM_READ_PRM*) params;
    PNIO_UINT8* buffer;
    PNIO_UINT32 Status;

    /* Alloc buffer for IM data */
    buffer = (PNIO_UINT8*) calloc(prm->pBufLen, 1);

    if((prm->pPnioState.ErrCode == 0) &&
            (prm->pPnioState.ErrDecode == 0) &&
            (prm->pPnioState.ErrCode1 == 0))
    {
        /* No error comming from Ertec */

        switch(prm->IMidx)
         {
             case 0:
                 /* IM0 data */
                 Im0_read_Handler(prm->Api, &prm->Addr, &prm->pBufLen, buffer,  &prm->pPnioState, prm->PeriphRealCfgInd);
                 break;
             case 1:
                 /* IM1 data */
                 Im1_read_Handler(prm->Api, &prm->Addr, &prm->pBufLen, buffer,  &prm->pPnioState, prm->PeriphRealCfgInd);
                 break;
             case 2:
                 /* IM2 data */
                 Im2_read_Handler(prm->Api, &prm->Addr, &prm->pBufLen, buffer,  &prm->pPnioState, prm->PeriphRealCfgInd);
                 break;
             case 3:
                 /* IM3 data */
                 Im3_read_Handler(prm->Api, &prm->Addr, &prm->pBufLen, buffer,  &prm->pPnioState, prm->PeriphRealCfgInd);
                 break;
             case 4:
                 /* IM4 data */
                 Im4_read_Handler(prm->Api, &prm->Addr, &prm->pBufLen, buffer,  &prm->pPnioState, prm->PeriphRealCfgInd);
                 break;
             default:
                 /* Fatal - it is not an option to have another index */
                 PNPB_LIB_FATAL
         }
    }
    else
    {
        /* Error comming from Ertec - just send it back to trigger PNDV */
        prm->pBufLen = 0;
    }

    PNIOext_im_read_rsp(prm, buffer);

    /* Cleanup */
    free(buffer);
}

/**
 *  @brief Callback from Ertec - IM data read
 *
 *  @param[in]      *params
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_cbf_im_data_store(PNIO_UINT8* params)
{
	PNIO_IM_STORE_PRM* prm = (PNIO_IM_STORE_PRM*) params;
    
    Bsp_im_data_store ((PNIO_NVDATA_TYPE) prm->NvDataType,       // nv data type: device name
    					prm->pData,               	// source pointer to the devicename
						prm->pBufLen,              	// length of the device name
						prm->PeriphRealCfgInd);     // entity index in periph interface-real_cfg
}

/**
 *  @brief Callback from Ertec - SNMP  data store to BBB/Host
 *
 *  @param[in]      *params
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_cbf_nv_data_store(PNIO_UINT8* params)
{
	PNIO_NV_DATA_STORE_PRM* prm = (PNIO_NV_DATA_STORE_PRM*) params;

    Bsp_nv_data_store ((PNIO_NVDATA_TYPE) prm->NvDataType,      // nv data type: device name
    					prm->pData,               	             // source pointer to the devicename
						prm->pBufLen);              	         // length of the device name
}

/**
 *  @brief Callback from Ertec - Set default values to BBB/Host
 *
 *  @param[in]      *params
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_cbf_nv_data_set_default(PNIO_UINT8* params)
{
    PNIO_UINT32       Status = PNIO_OK;
    PNIO_NV_DATA_STORE_PRM* prm = (PNIO_NV_DATA_STORE_PRM*) params;
	NvDataSetDefaultValues((PNIO_UINT32) prm->NvDataType);
    Status = SaveDataToFlashMem();
    if(Status != PNIO_OK)
    {
       printf ("NV data saving error! during sync \n");
    }
}

/**
 *  @brief Replacement value handling
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  Example functionality, part disabled in App1_STANDART -> done the same
 *
 */
PNIO_VOID PNIOext_cbf_substval_out_read(PNIO_UINT8* params)
{
    PNIO_SUBSTVAL_OUT_READ_PRM*  prm;
    PNIO_UINT8* pBuffer;                /* Ptr to submodule output substitute data */
    PNIO_UINT16 SubstMode = 0;          /* [BIG ENDIAN] SubstitutionMode: 0=ZERO or inactive (default), 1:LastValue, 2:Replacement value SubstitutionMode: 0=ZERO or inactive, 1:LastValue, 2:Replacement value */
    PNIO_UINT16 SubstActive = 0;        /* [BIG ENDIAN] SubstituteActiveFlag:  0=operation, 1=substitute. default value is 0: if (IOPS & IOCS = GOOD), else: 1 */

    /* Parse arguments */
    prm = (PNIO_SUBSTVAL_OUT_READ_PRM*) params;

    /* Allocate memory */
    pBuffer = calloc(prm->BufLen ,1);
    if(NULL != pBuffer)
    {
        /* Set the substitute data  */
        memset(pBuffer, 0xab, prm->BufLen);    /* example: all substitute data are 0xab (the user will change that...) */
    }
    else
    {
        printf ("Error: Memory could not be allocated!\n");
        PNPB_LIB_FATAL;
    }

    /* Send data back to stack */
    PNIOext_substval_out_read_done(pBuffer, prm->BufLen, prm->BufAddr, SubstMode, prm->SubstModeAddr, SubstActive, prm->SubstActiveAddr);

}   /* PNIOext_cbf_substval_out_read */


/**
 *  @brief Reports to user about save station name event
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  Also informs about remanency of new Device name
 *  Data storage is handled in Ertec
 *
 */
PNIO_VOID PNIOext_cbf_save_station_name(PNIO_UINT8* params)
{
	PNIO_UINT8 pDevName[256];
	PNIO_UINT32 DevNameLen;

    PNIO_SAVE_STATION_NAME_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_SAVE_STATION_NAME_PRM*) params;

    /* Store new device name */
    if(prm->NameLength > 255)
    {
        printf("##Warning: Received station name from ERTEC too long!\n");
        return;
    }

    /* Store new name */
    memcpy(pDevName, prm->StationName, prm->NameLength);
    pDevName[prm->NameLength] = '\0';

    if (prm->Remanent)
    {
    	printf ( "##Save remanent: ");

        // **** store data in non volatile memory ****
           Bsp_nv_data_store (PNIO_NVDATA_DEVICENAME,    // nv data type: device name
        		   	   	   	   pDevName,              	 // source pointer to the devicename
							   prm->NameLength);         // length of the device name

    }
    else
    {
    	printf ( (PNIO_INT8*) "Station Name = %.*s  received, Length=%d  Remanent = %d\n",
    			prm->NameLength, pDevName, prm->NameLength, prm->Remanent);
    	 Bsp_nv_data_store (PNIO_NVDATA_DEVICENAME,    // nv data type: device name
    	                              "",              // source pointer to the devicename
    	                              0);              // length of the device name
    }
} /* PNIOext_cbf_save_station_name */

/**
 *  @brief Reports to user about new ip suite data
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  Data storage is handled in Ertec
 *
 */
PNIO_VOID PNIOext_cbf_save_ip_addr(PNIO_UINT8* params)
{
	PNIO_UINT8 pNewIpSuite[12];
    PNIO_SAVE_IP_ADDR_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_SAVE_IP_ADDR_PRM*) params;

    /* Store IP address */
    pNewIpSuite[0] = prm->NewIpAddr[3];
    pNewIpSuite[1] = prm->NewIpAddr[2];
    pNewIpSuite[2] = prm->NewIpAddr[1];
    pNewIpSuite[3] = prm->NewIpAddr[0];

    /* Store netmask */
    pNewIpSuite[4] = prm->SubnetMask[3];
    pNewIpSuite[5] = prm->SubnetMask[2];
    pNewIpSuite[6] = prm->SubnetMask[1];
    pNewIpSuite[7] = prm->SubnetMask[0];

    /* Store router */
    pNewIpSuite[8] = prm->DefRouterAddr[3];
    pNewIpSuite[9] = prm->DefRouterAddr[2];
    pNewIpSuite[10] = prm->DefRouterAddr[1];
    pNewIpSuite[11] = prm->DefRouterAddr[0];

    if (prm->Remanent)
    {
    	printf("##Remanent address was stored \n");

        // **** save ip suite in non volatile memory ****
        Bsp_nv_data_store ( PNIO_NVDATA_IPSUITE,      // nv data type: ip suite
        					pNewIpSuite,              // source pointer
                            sizeof (NV_IP_SUITE));    // length of the device name
    }

    else
    {
        // *-----------------------------------------------------
        // *  according to the PNIO SPEC an already (remanent)
        // *  stored IP address must be deleted, if a new ip address
        // *  is set non remanent.
        // *-----------------------------------------------------
        memset(pNewIpSuite, 0, sizeof (NV_IP_SUITE));
        Bsp_nv_data_store (	PNIO_NVDATA_IPSUITE,       // nv data type: ip suite
        					pNewIpSuite,               // source pointer
							sizeof (NV_IP_SUITE));     // length of the device name

        printf("Remanent address was deleted \n");
    }
}   /* PNIOext_cbf_save_ip_addr */


/**
 *  @brief x
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  x
 *
 */
PNIO_VOID PNIOext_cbf_report_new_ip_addr(PNIO_UINT8* params)
{
    PNIO_REPORT_NEW_IP_ADDR_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_REPORT_NEW_IP_ADDR_PRM*) params;

    printf("IP address = %03d.%03d.%03d.%03d, Subnet mask = %03d.%03d.%03d.%03d, Default router = %03d.%03d.%03d.%03d\n",
        prm->NewIpAddr[0], prm->NewIpAddr[1], prm->NewIpAddr[2], prm->NewIpAddr[3],
        prm->SubnetMask[0], prm->SubnetMask[1], prm->SubnetMask[2], prm->SubnetMask[3],
        prm->DefRouterAddr[0], prm->DefRouterAddr[1], prm->DefRouterAddr[2], prm->DefRouterAddr[3]);

    return;
}   /* PNIOext_cbf_report_new_ip_addr */


/**
 *  @brief Inform BBB that ERTEC will perform factory reset
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 */
PNIO_VOID PNIOext_cbf_reset_factory_settings(PNIO_UINT8* params)
{
    PNIO_RESET_FACTORY_SETTINGS_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_RESET_FACTORY_SETTINGS_PRM*) params;

    printf("##Reset to factory settings, RtfOption = %d", prm->RtfOption);
}   /* PNIOext_cbf_reset_factory_settings */


/**
 *  @brief Result of ip addr / mac addr / device name change
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  This function transfers result of device_name, IP or MAC address user
 *  change. In case of wrong input by user, will print reason for rejection to
 *  console.
 *
 */
PNIO_VOID PNIOext_cbf_result_new_device_address(PNIO_UINT8* params)
{
    PNIO_RESULT_NEW_DEV_ADDR_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_RESULT_NEW_DEV_ADDR_PRM*) params;

    if(prm->command == (PNIO_UINT8) PNIO_STORE_NEW_MAC)
    {
        switch ( prm->result )
        {
            case USR_NAME_INVALID_INPUT:
                printf("MAC address is in wrong format\n");
                break;
            case USR_ADDR_NO_CHANGE:
                printf("New MAC address is same like current MAC address\n");
                break;
            case USR_ADDR_TEST_OK:
                printf("new MAC address is processed\n");
                // *** store new mac address in flash memory ****
                Bsp_nv_data_store (PNIO_NVDATA_MAC_ADDR, (PNIO_VOID*) pTmpMacAddr, 6);

			printf ("*-----------------------------------------------------------*\n");
			printf ("* to activate the new mac address, perform a system restart *\n");
			printf ("*-----------------------------------------------------------*\n");
			printf ("\n");
                break;
            default:
                printf("Result of new MAC address configuration = 0x%x\n", prm->result);
                break;
        }
    }
    else if(prm->command == (PNIO_UINT8) PNIO_STORE_NEW_IP)
    {
        switch ( prm->result )
        {
            case USR_ADDR_AR_FORBIDDEN:
                printf( "Change of IP is forbidden for device with active AR\n" );
                break;
            case USR_ADDR_NO_CHANGE:
                printf( "New ip suite is same as old one - no need to save it again\n" );
                break;
            case USR_ADDR_TEST_OK:
                // *** store new mac address in flash memory ****
                Bsp_nv_data_store (PNIO_NVDATA_IPSUITE, (PNIO_VOID*) pTmpIpSuite, 12);
                break;
            default:
                printf( "Invalid IP address, problem #0x%02x\n", prm->result );
                break;
        }
    }
    else if(prm->command == (PNIO_UINT8) PNIO_STORE_NEW_NAME)
    {
        switch (prm->result)
        {
            case USR_ADDR_TEST_OK:
                // *** store new mac address in flash memory ****
                Bsp_nv_data_store (PNIO_NVDATA_DEVICENAME, (PNIO_VOID*) pTmpDevName, pTmpDevNameLen);
                break;
            case USR_ADDR_AR_FORBIDDEN:
                printf( "Change of device name is forbidden for device with active AR\n" );
                break;
            case USR_ADDR_NO_CHANGE:
                printf( "New device name is same as old one - no need to save it again\n" );
                break;
            default:
                printf( "Invalid device name, problem #0x%02x\n", prm->result );
                break;
        }
    }
    else
    {
        printf("PNIOext_cbf_result_new_device_address command unresolved\n");
    }
}   /* PNIOext_cbf_result_new_device_address */


/**
 *  @brief Reports that LED blinking was started
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  x
 *
 */
PNIO_VOID PNIOext_cbf_start_led_blink(PNIO_UINT8* params)
{
    PNIO_UINT32 PortNum;
    PNIO_UINT32 frequency;

    /* Parse received parameters */
    PortNum = (PNIO_UINT32) *params;
    params += sizeof(PNIO_UINT32);
    frequency = (PNIO_UINT32) *params;

    printf("##LED Blink START, Port = %d, frequency = %d Hz \n", PortNum, frequency);

}   /* PNIOext_cbf_start_led_blink */


/**
 *  @brief Reports that LED blinking was started
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  x
 *
 */
PNIO_VOID PNIOext_cbf_stop_led_blink(PNIO_UINT8* params)
{
    PNIO_UINT32 PortNum;

    /* Parse received parameters */
    PortNum = (PNIO_UINT32) *params;

    printf("##LED Blink STOP, Port = %d\n", PortNum);

}   /* PNIOext_cbf_stop_led_blink */


/**
 *  @brief Ertrec finished startup and is ready for use
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  Finished firmware load and startup in Ertec
 *  Memory interface can be used in full scope from now on
 *
 */
PNIO_VOID PNIOext_cbf_device_startup_done(PNIO_UINT8* params)
{
    PNPB_USR_STARTUP_STATE NewState = 0;
    PNIO_STARTUP_DONE_PRM *prm = (PNIO_STARTUP_DONE_PRM *) params;
    NewState = ((PNPB_USR_STARTUP_STATE)(prm->State));
    if((PNPB_USR_START_BOOT_OK == NewState) &&
            (PNPB_USR_START_IDLE == PnpbDeviceStartupState))
    {
        /* Obtain firmware version */
        DeviceFwVersion.VerPrefix = prm->Version.VerPrefix;
        DeviceFwVersion.VerHh = prm->Version.VerHh;
        DeviceFwVersion.VerH = prm->Version.VerH;
        DeviceFwVersion.VerL = prm->Version.VerL;
        DeviceFwVersion.VerLl = prm->Version.VerLl;

        if(prm->Fw_DAP != MODULE_ID_DAP)
        {
        	printf("\n WARNING: FW DAP(%u) and HOST APP DAP(%u) are different from each other.\n\n",prm->Fw_DAP, MODULE_ID_DAP);
        }
        PnpbDeviceStartupState = NewState;
        printf("Ertec start-up succesful\n");
        sem_post(&PnioDeviceReadySemId);
    }
    else if((PNPB_USR_START_START_OK == NewState) &&
            (PNPB_USR_START_BOOT_OK == PnpbDeviceStartupState))
    {
        PnpbDeviceStartupState = NewState;
        printf("Ertec PN stack start-up succesful\n");
        sem_post(&PnioDeviceReadySemId);
    }
    else if((PNPB_USR_START_START_FAILED == NewState) &&
            (PNPB_USR_START_BOOT_OK == PnpbDeviceStartupState))
    {
        PnpbDeviceStartupState = NewState;
        printf("Ertec PN stack start-up failed\n");
        sem_post(&PnioDeviceReadySemId);
    }
    else
    {
        printf("Error: Invalid Ertec startup state: going from %x to %x\n", PnpbDeviceStartupState, NewState);
    }
}   /* PNIOext_cbf_device_startup_done */

/**
 *  @brief Callback from Ertec - traces ready
 *
 *  @param[in]      params
 *  @return         void
 *
 *  Received from Ertec when traces (in string form)
 *  are ready to be transferred via GPMC from shared memory
 *
 */

PNIO_VOID PNIOext_cbf_trace_ready(PNIO_UINT8* params)
{
    PNIO_UINT32 i;
    PNIO_TRACE_READY_PRM* prm;
    prm = (PNIO_TRACE_READY_PRM*) params;

    /* Determine whether to store or print incoming traces */
    if(prm->store == 1)
    {
    	size_t write_len;
    	PNIO_UINT8* tmp_buf;

    	tmp_buf = (PNIO_UINT8*) malloc(prm->len);
        if(NULL == tmp_buf)
        {
            printf ("Error: Memory could not be allocated!\n");
            PNPB_LIB_FATAL;
        }
        else
        {
            if(prm->len > 0)
            {
                // Below loop is used instead of memcpy to keep volitile qualifier happy 
				for (i = 0; i < prm->len; i++)
				{
					tmp_buf[i] = pnpb_xhif_traces_p[i];
				}

    			if(trace_file_fd != NULL)
    			{
            		/* Store trace */
					write_len = fwrite(tmp_buf, 1, prm->len, trace_file_fd);
					/* write_len equals the number of bytes transferred only when size of item is 1 */
					if(write_len != prm->len)
					{
						printf("Wrong number of written bytes of NV traces (was %dB, expected %dB)!\n", write_len, prm->len);
					}
    			}
    			else
    			{
    				printf("Cannot store traces to NV - file is not opened!\n");
    			}
            }

    		if((prm->trace_end == 1) || (prm->len <= 0))
    		{
    			if(trace_file_fd != NULL)
    			{
        			/* close file */
        			fclose(trace_file_fd);
        			/* Operation is finished */
        			printf("ok, done\n");
    			}
    		}
		}
    }
    else
    {
       /* Print trace */
       for(i = 0; i < prm->len; i++)
       {
           putchar(pnpb_xhif_traces_p[i]);
       }
    }

    /* It this is not last trace, continue */
    if((prm->trace_end == 0) && (prm->len != 0))
    {
        /* Ask ertec to prepare next one */
        PNIOext_trace_command(PNIO_TRACE_PRINT_NEXT);
    }
    else
    {
        /* Allow trace commands */
        sem_post(&TracesSemId);
    }
}


/**
 *  @brief Handle of interrupt handling routine
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  Used for further manipulation with interrupt
 *
 */
PNIO_VOID PNIOext_cbf_return_isr_handle(PNIO_UINT8* params)
{
    PNIO_UINT32 ObjHnd;

    ObjHnd = (PNIO_UINT32) *params;
}   /* PNIOext_cbf_return_isr_handle */


/**
 *  @brief Calls callback function of interrupt
 *
 *  @param[in]      *params             All parameters as delivered via memory interface
 *  @return         void
 *
 *  x
 *
 */

PNIO_VOID PNIOext_cbf_perform_isr_callback(PNIO_UINT8* params)
{
 //   PNIO_VOID (*pIsrCbf)(PNIO_UINT32);

//    pIsrCbf = (PNIO_VOID *) params;
//    (*pIsrCbf)(0);


}   /* PNIOext_cbf_perform_isr_callback */



/*
 * Calls from BBB to Ertec
 */

/*
 * User alarms call
 */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]		Addr			PNIO_DEV_ADDR*		geographical or logical address
 *  @param[in]		ChannelNum		PNIO_UINT16			channel number
 *  @param[in]		ErrorNum		PNIO_UINT16			error number, see PNIO specification coding of "ChannelErrorType"
 *  @param[in]		ChanDir			PNIO_UINT16			channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
 *  @param[in]		ChanTyp			PNIO_UINT16			channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
 *  @param[in]		MaintReq		PNIO_BOOL			maintenance required
 *  @param[in]		MaintDem		PNIO_BOOL			maintenance demanded
 *  @param[in]		DiagTag			PNIO_UINT16			user defined diag tag, used as reference
 *
 *  @return         void
 *
 *  Provide standard channel diagnostic, send alarm appears
 *
 */

PNIO_VOID PNIOext_diag_channel_add(
		PNIO_DEV_ADDR *Addr,		// geographical or logical address
		PNIO_UINT16 ChannelNum,	 	// channel number
		PNIO_UINT16 ErrorNum,		// error number, see PNIO specification coding of "ChannelErrorType"
		PNIO_UINT16 ChanDir,		// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		PNIO_UINT16 ChanTyp,		// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
		PNIO_BOOL MaintReq,	 		// maintenance required
		PNIO_BOOL MaintDem,	 		// maintenance demanded
		PNIO_UINT16 DiagTag    		// user defined diag tag, used as reference
		)
{
	PNIO_DIAG_CHANNEL_PRM* params = calloc(sizeof(PNIO_DIAG_CHANNEL_PRM), 1);

	/* Collect all standard params into one buffer */
	params->StdParams.ChannelNum = ChannelNum;
	params->StdParams.ErrorNum = ErrorNum;
	params->StdParams.ChanDir = ChanDir;
	params->StdParams.ChanTyp = ChanTyp;
	/* Collect all specific params into one buffer */
	params->MaintReq = MaintReq;
	params->MaintDem = MaintDem;
	params->DiagTag = DiagTag;

	/* Address copy */
	params->StdParams.Addr.Type = Addr->Type;
	params->StdParams.Addr.ad = Addr->ad;

	/* Send params and function ID to cyclical buffer */
	pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_DIAG_CHANNEL_ADD,
			sizeof(PNIO_DIAG_CHANNEL_PRM), (PNIO_UINT8*)(params));

	/* Free allocated memory */
	free(params);
}	/* PNIOext_diag_channel_add */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]		Addr			PNIO_DEV_ADDR*		geographical or logical address
 *  @param[in]		ChannelNum		PNIO_UINT16			channel number
 *  @param[in]		ErrorNum		PNIO_UINT16			error number, see PNIO specification coding of "ChannelErrorType"
 *  @param[in]		ChanDir			PNIO_UINT16			channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
 *  @param[in]		ChanTyp			PNIO_UINT16			channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
 *  @param[in]		DiagTag			PNIO_UINT16			user defined diag tag, used as reference
 *  @param[in]		AlarmState		PNIO_UINT16			DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
 *
 *  @return         void
 *
 *  Provide standard channel diagnostic, send alarm disappears
 *
 */

PNIO_VOID PNIOext_diag_channel_remove(
		PNIO_DEV_ADDR *Addr,		// geographical or logical address
		PNIO_UINT16 ChannelNum,	 	// channel number
		PNIO_UINT16 ErrorNum,		// error number, see PNIO specification coding of "ChannelErrorType"
		PNIO_UINT16 ChanDir,		// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		PNIO_UINT16 ChanTyp,		// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
		PNIO_UINT16 DiagTag,    	// user defined diag tag, used as reference
		PNIO_UINT16 AlarmState		// DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
		)
{
	PNIO_DIAG_CHANNEL_REMOVE_PRM* params = calloc(sizeof(PNIO_DIAG_CHANNEL_REMOVE_PRM), 1);

	/* Collect all standard params into one buffer */
	params->StdParams.ChannelNum = ChannelNum;
	params->StdParams.ErrorNum = ErrorNum;
	params->StdParams.ChanDir = ChanDir;
	params->StdParams.ChanTyp = ChanTyp;
	/* Collect all specific params into one buffer */
	params->DiagTag = DiagTag;
	params->AlarmState = AlarmState;

	/* Address copy */
	params->StdParams.Addr.Type = Addr->Type;
	params->StdParams.Addr.ad = Addr->ad;

	/* Send params and function ID to cyclical buffer */
	pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_DIAG_CHANNEL_REMOVE,
			sizeof(PNIO_DIAG_CHANNEL_REMOVE_PRM), (PNIO_UINT8*)(params));

	/* Free allocated memory */
	free(params);
} /* PNIOext_diag_channel_remove */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]		Addr				PNIO_DEV_ADDR*		geographical or logical address
 *  @param[in]		ChannelNum			PNIO_UINT16			channel number
 *  @param[in]		ErrorNum			PNIO_UINT16			error number, see PNIO specification coding of "ChannelErrorType"
 *  @param[in]		ChanDir				PNIO_UINT16			channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
 *  @param[in]		ChanTyp				PNIO_UINT16			channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
 *  @param[in]		ExtChannelErrType	PNIO_UINT16			ExtChannelErrorType (see IEC61158)
 *  @param[in]		ExtChannelAddValue	PNIO_UINT16			ExtChannelAddValue (see IEC61158)
 *  @param[in]		MaintReq			PNIO_BOOL			maintenance required
 *  @param[in]		MaintDem			PNIO_BOOL			maintenance demanded
 *  @param[in]		DiagTag				PNIO_UINT16			user defined diag tag, used as reference
 *
 *  @return         void
 *
 *  Provide external channel diagnostic, send alarm appears
 *
 */

PNIO_VOID PNIOext_ext_diag_channel_add(
		PNIO_DEV_ADDR *Addr,		// geographical or logical address
		PNIO_UINT16 ChannelNum,	 	// channel number
		PNIO_UINT16 ErrorNum,		// error number, see PNIO specification coding of "ChannelErrorType"
		PNIO_UINT16 ChanDir,		// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		PNIO_UINT16 ChanTyp,		// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
		PNIO_UINT16 ExtChannelErrType,	// ExtChannelErrorType (see IEC61158)
		PNIO_UINT16 ExtChannelAddValue,	// ExtChannelAddValue (see IEC61158)
		PNIO_BOOL MaintReq,	 		// maintenance required
		PNIO_BOOL MaintDem,	 		// maintenance demanded
		PNIO_UINT16 DiagTag    		// user defined diag tag, used as reference
		)
{
	PNIO_EXT_DIAG_CHANNEL_PRM* params = calloc(sizeof(PNIO_EXT_DIAG_CHANNEL_PRM), 1);

	/* Collect all standard params into one buffer */
	params->StdParams.ChannelNum = ChannelNum;
	params->StdParams.ErrorNum = ErrorNum;
	params->StdParams.ChanDir = ChanDir;
	params->StdParams.ChanTyp = ChanTyp;
	/* Collect all specific params into one buffer */
	params->ExtChannelErrType = ExtChannelErrType;
	params->ExtChannelAddValue = ExtChannelAddValue;
	params->MaintReq = MaintReq;
	params->MaintDem = MaintDem;
	params->DiagTag = DiagTag;

	/* Address copy */
	params->StdParams.Addr.Type = Addr->Type;
	params->StdParams.Addr.ad = Addr->ad;

	/* Send params and function ID to cyclical buffer */
	pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_EXT_DIAG_CHANNEL_ADD,
			sizeof(PNIO_EXT_DIAG_CHANNEL_PRM), (PNIO_UINT8*)(params));

	/* Free allocated memory */
	free(params);
} /* PNIOext_ext_diag_channel_add */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]		Addr				PNIO_DEV_ADDR*		geographical or logical address
 *  @param[in]		ChannelNum			PNIO_UINT16			channel number
 *  @param[in]		ErrorNum			PNIO_UINT16			error number, see PNIO specification coding of "ChannelErrorType"
 *  @param[in]		ChanDir				PNIO_UINT16			channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
 *  @param[in]		ChanTyp				PNIO_UINT16			channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
 *  @param[in]		ExtChannelErrType	PNIO_UINT16			ExtChannelErrorType (see IEC61158)
 *  @param[in]		ExtChannelAddValue	PNIO_UINT16			ExtChannelAddValue (see IEC61158)
 *  @param[in]		DiagTag				PNIO_UINT16			user defined diag tag, used as reference
 *  @param[in]		AlarmState			PNIO_UINT16			DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
 *
 *  @return         void
 *
 *  Provide external channel diagnostic, send alarm disappears
 *
 */

PNIO_VOID PNIOext_ext_diag_channel_remove(
		PNIO_DEV_ADDR *Addr,		// geographical or logical address
		PNIO_UINT16 ChannelNum,	 	// channel number
		PNIO_UINT16 ErrorNum,		// error number, see PNIO specification coding of "ChannelErrorType"
		PNIO_UINT16 ChanDir,		// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		PNIO_UINT16 ChanTyp,		// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
		PNIO_UINT16 ExtChannelErrType,	 // ExtChannelErrorType (see IEC61158)
		PNIO_UINT16 ExtChannelAddValue,	 // ExtChannelAddValue (see IEC61158)
		PNIO_UINT16 DiagTag,    	// user defined diag tag, used as reference
		PNIO_UINT16 AlarmState		// DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
		)
{
	PNIO_EXT_DIAG_CHANNEL_REMOVE_PRM* params = calloc(sizeof(PNIO_EXT_DIAG_CHANNEL_REMOVE_PRM), 1);

	/* Collect all standard params into one buffer */
	params->StdParams.ChannelNum = ChannelNum;
	params->StdParams.ErrorNum = ErrorNum;
	params->StdParams.ChanDir = ChanDir;
	params->StdParams.ChanTyp = ChanTyp;
	/* Collect all specific params into one buffer */
	params->ExtChannelErrType = ExtChannelErrType;
	params->ExtChannelAddValue = ExtChannelAddValue;
	params->DiagTag = DiagTag;
	params->AlarmState = AlarmState;

	/* Address copy */
	params->StdParams.Addr.Type = Addr->Type;
	params->StdParams.Addr.ad = Addr->ad;

	/* Send params and function ID to cyclical buffer */
	pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_EXT_DIAG_CHANNEL_REMOVE,
			sizeof(PNIO_EXT_DIAG_CHANNEL_REMOVE_PRM), (PNIO_UINT8*)(params));

	/* Free allocated memory */
	free(params);
} /* PNIOext_ext_diag_channel_remove */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]		Addr				PNIO_DEV_ADDR*		geographical or logical address
 *  @param[in]		ChannelNum			PNIO_UINT16			channel number
 *  @param[in]		ErrorNum			PNIO_UINT16			error number, see PNIO specification coding of "ChannelErrorType"
 *  @param[in]		ChanDir				PNIO_UINT16			channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
 *  @param[in]		ChanTyp				PNIO_UINT16			channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
 *  @param[in]		DiagTag				PNIO_UINT16			user defined diag tag, used as reference
 *  @param[in]		UserStructIdent		PNIO_UINT16			manufacturer specific, 0...0x7fff, see IEC 61158
 *  @param[in]		InfoDataLen			PNIO_UINT16			length of generic diagnostic data
 *  @param[in]		InfoData			PNIO_UINT8*			user defined generic diagnostic data
 *  @param[in]		MaintReq			PNIO_BOOL			maintenance required
 *  @param[in]		MaintDem			PNIO_BOOL			maintenance demanded
 *
 *  @return         void
 *
 *  Provide generic diagnostic, send alarm appears
 *
 */

PNIO_VOID PNIOext_diag_generic_add(
		PNIO_DEV_ADDR *Addr,		// geographical or logical address
		PNIO_UINT16 ChannelNum,	 	// channel number
		PNIO_UINT16 ChanDir,	 	// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		PNIO_UINT16 ChanTyp,	 	// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
		PNIO_UINT16 DiagTag,    	// user defined diag tag, used as reference
		PNIO_UINT16 UserStructIdent,   	// manufacturer specific, 0...0x7fff, see IEC 61158
		PNIO_UINT32 InfoDataLen,	// length of generic diagnostic data
		PNIO_UINT8 *InfoData,		// user defined generic diagnostic data
		PNIO_BOOL MaintReq,	 	// maintenance required
		PNIO_BOOL MaintDem	 	// maintenance demanded
		)
{
	PNIO_DIAG_GENERIC_PRM* params = calloc(sizeof(PNIO_DIAG_GENERIC_PRM) + InfoDataLen + 1, 1);

	/* Collect all standard params into one buffer */
	params->StdParams.ChannelNum = ChannelNum;
	params->StdParams.ChanDir = ChanDir;
	params->StdParams.ChanTyp = ChanTyp;
	params->StdParams.DiagTag = DiagTag;
	params->StdParams.UserStructIdent = UserStructIdent;

	/* Collect all specific params into one buffer */
	params->MaintReq = MaintReq;
	params->MaintDem = MaintDem;
	params->InfoDataLen = InfoDataLen;

	/* Address copy */
	params->StdParams.Addr.Type = Addr->Type;
	params->StdParams.Addr.ad = Addr->ad;

	/* Data copy */
	memcpy(&params->InfoData[0], InfoData, InfoDataLen);

	/* Send params and function ID to cyclical buffer */
	pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_DIAG_GENERIC_ADD,
			(sizeof(PNIO_DIAG_GENERIC_PRM) + InfoDataLen + 1), (PNIO_UINT8*)(params));

	/* Free allocated memory */
	free(params);
} /* PNIOext_diag_generic_add */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]		Addr				PNIO_DEV_ADDR*		geographical or logical address
 *  @param[in]		ChannelNum			PNIO_UINT16			channel number
 *  @param[in]		ErrorNum			PNIO_UINT16			error number, see PNIO specification coding of "ChannelErrorType"
 *  @param[in]		ChanDir				PNIO_UINT16			channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
 *  @param[in]		ChanTyp				PNIO_UINT16			channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
 *  @param[in]		DiagTag				PNIO_UINT16			user defined diag tag, used as reference
 *  @param[in]		UserStructIdent		PNIO_UINT16			manufacturer specific, 0...0x7fff, see IEC 61158
 *
 *  @return         void
 *
 *  Provide generic diagnostic, send alarm disappears
 *
 */

PNIO_VOID PNIOext_diag_generic_remove(
		PNIO_DEV_ADDR *Addr,		// geographical or logical address
		PNIO_UINT16 ChannelNum,	 	// channel number
		PNIO_UINT16 ChanDir,	 	// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		PNIO_UINT16 ChanTyp,	 	// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
		PNIO_UINT16 DiagTag,    	// user defined diag tag, used as reference
		PNIO_UINT16 UserStructIdent	// manufacturer specific, 0...0x7fff, see IEC 61158
		)
{
	PNIO_DIAG_GENERIC_REMOVE_PRM* params = calloc(sizeof(PNIO_DIAG_GENERIC_REMOVE_PRM), 1);

	/* Collect all standard params into one buffer */
	params->StdParams.ChannelNum = ChannelNum;
	params->StdParams.ChanDir = ChanDir;
	params->StdParams.ChanTyp = ChanTyp;
	params->StdParams.DiagTag = DiagTag;
	params->StdParams.UserStructIdent = UserStructIdent;

	/* Address copy */
	params->StdParams.Addr.Type = Addr->Type;
	params->StdParams.Addr.ad = Addr->ad;

	/* Send params and function ID to cyclical buffer */
	pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_DIAG_GENERIC_REMOVE,
			sizeof(PNIO_DIAG_GENERIC_REMOVE_PRM), (PNIO_UINT8*)(params));

	/* Free allocated memory */
	free(params);
} /* PNIOext_diag_generic_remove */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]		Addr			PNIO_DEV_ADDR*		geographical or logical address
 *  @param[in]		DataLen			PNIO_UINT32			length of AlarmItem.Data
 *  @param[in]		Data			PNIO_UINT8*			AlarmItem.Data
 *  @param[in]		UserStructIdent	PNIO_UINT16			AlarmItem.UserStructureIdentifier, s. IEC61158-6
 *  @param[in]		UserHndl		PNIO_UINT32			user defined handle
 *
 *  @return         void
 *
 *  Send process alarms
 *
 */

PNIO_VOID PNIOext_process_alarm_send(
		PNIO_DEV_ADDR *Addr,			// geographical or logical address
		PNIO_UINT32 DataLen,			// length of AlarmItem.Data
		PNIO_UINT8 *Data,				// AlarmItem.Data
		PNIO_UINT16 UserStructIdent,	// AlarmItem.UserStructureIdentifier, s. IEC61158-6
		PNIO_UINT32 UserHndl) 			// user defined handle
{
	PNIO_PROCESS_ALARM_SET_PRM* params = calloc(sizeof(PNIO_PROCESS_ALARM_SET_PRM) + DataLen + 1, 1);

	/* Collect all params into one buffer */
	params->DataLen = DataLen;
	params->UserStructIdent = UserStructIdent;
	params->UserHndl = UserHndl;

	/* Data copy */
	memcpy(&params->Data[0], Data, DataLen);

	/* Address copy */
	params->Addr.Type = Addr->Type;
	params->Addr.ad = Addr->ad;

	/* Send params and function ID to cyclical buffer */
	pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_PROCESS_ALARM_SEND,
			sizeof(PNIO_PROCESS_ALARM_SET_PRM) + DataLen + 1, (PNIO_UINT8*)(params));

	/* Free allocated memory */
	free(params);
}	/* PNIOext_process_alarm_send */



/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]		Addr			PNIO_DEV_ADDR*		geographical or logical address
 *  @param[in]		DataLen			PNIO_UINT32			length of AlarmItem.Data
 *  @param[in]		Data			PNIO_UINT8*			AlarmItem.Data
 *  @param[in]		UserStructIdent	PNIO_UINT16			AlarmItem.UserStructureIdentifier, s. IEC61158-6
 *  @param[in]		UserHndl		PNIO_UINT32			user defined handle
 *
 *  @return         void
 *
 *  Send process alarms
 *
 */

PNIO_VOID PNIOext_status_alarm_send(
		PNIO_DEV_ADDR *Addr,			// geographical or logical address
		PNIO_UINT32 Api,     			// application process identifier
		PNIO_UINT32 DataLen,			// length of AlarmItem.Data
		PNIO_UINT8 *Data,				// AlarmItem.Data
		PNIO_UINT16 UserStructIdent,	// AlarmItem.UserStructureIdentifier, s. IEC61158-6
		PNIO_UINT32 UserHndl) 			// user defined handle
{
	PNIO_STATUS_ALARM_SET_PRM* params = calloc(sizeof(PNIO_STATUS_ALARM_SET_PRM) + DataLen + 1, 1);

	/* Collect all params into one buffer */
	params->DataLen = DataLen;
	params->UserStructIdent = UserStructIdent;
	params->UserHndl = UserHndl;
	params->Api = Api;

	if ((Data == NULL) || (DataLen > PNDV_AL_STAL_INFO_LEN ))
	{
		printf("Can not send status alarm. invalid data or data length: %d \n", DataLen);
		return; 
	}

	/* Data copy */
	memcpy(&params->Data[0], Data, DataLen);

	/* Address copy */
	params->Addr.Type = Addr->Type;
	params->Addr.ad = Addr->ad;

	/* Send params and function ID to cyclical buffer */
	pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_STATUS_ALARM_SEND,
			sizeof(PNIO_STATUS_ALARM_SET_PRM) + DataLen + 1, (PNIO_UINT8*)(params));

	/* Free allocated memory */
	free(params);
}	/* PNIOext_status_alarm_send */


/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]		Addr			PNIO_DEV_ADDR*		geographical or logical address
 *  @param[in]		UserHndl		PNIO_UINT32			user defined handle
 *
 *  @return         void
 *
 *
 */

PNIO_VOID PNIOext_ret_of_sub_alarm_send(
		PNIO_DEV_ADDR *Addr,			// geographical or logical address
		PNIO_UINT32 UserHndl	 		// user defined handle
		)
{
	PNIO_RET_OF_SUB_ALARM_SEND_PRM* params = calloc(sizeof(PNIO_RET_OF_SUB_ALARM_SEND_PRM), 1);

	/* Collect all specific params into one buffer */
	params->UserHndl = UserHndl;

	/* Address copy */
	params->Addr.Type = Addr->Type;
	params->Addr.ad = Addr->ad;

	/* Send params and function ID to cyclical buffer */
	pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_RET_OF_SUB_ALARM_SEND,
			(sizeof(PNIO_RET_OF_SUB_ALARM_SEND_PRM)), (PNIO_UINT8*)(params));

	/* Free allocated memory */
	free(params);
} /* PNIOext_ret_of_sub_alarm_send */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]		Addr			PNIO_DEV_ADDR*		geographical or logical address
 *  @param[in]		DataLen			PNIO_UINT32			length of AlarmItem.Data
 *  @param[in]		Data			PNIO_UINT8			AlarmItem.Data
 *  @param[in]		UserHndl		PNIO_UINT32			user defined handle
 *
 *  @return         void
 *
 *
 */

PNIO_VOID PNIOext_upload_retrieval_alarm_send(
		PNIO_DEV_ADDR *Addr,			// geographical or logical address
		PNIO_UINT32 DataLen,			// length of AlarmItem.Data
		PNIO_UINT8 *Data,				// AlarmItem.Data
		PNIO_UINT32 UserHndl	 		// user defined handle
		)
{
	PNIO_UPLOAD_RETRIEVAL_ALARM_PRM* params = calloc(sizeof(PNIO_UPLOAD_RETRIEVAL_ALARM_PRM) + DataLen, 1);

	/* Collect all specific params into one buffer */
	params->UserHndl = UserHndl;
	params->DataLen = DataLen;

	/* Address copy */
	params->Addr.Type = Addr->Type;
	params->Addr.ad = Addr->ad;

	/* Data copy */
	memcpy(&params->Data[0], Data, DataLen);

	/* Send params and function ID to cyclical buffer */
	pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_UPLOAD_RETRIEVAL_ALARM_SEND,
			(sizeof(PNIO_UPLOAD_RETRIEVAL_ALARM_PRM) + DataLen), (PNIO_UINT8*)(params));

	/* Free allocated memory */
	free(params);
} /* PNIOext_upload_retrieval_alarm_send */

/*
 * Initialize, activate, deactivate, abort
 */

 /**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      *pPnUsrDev              Device setup configuration
 *  @param[in]      *pIoSubList             Plugged submodules, including PDEV
 *  @param[in]      NumOfSublistEntries     Number of entries in pPioSubList
 *  @param[in]      *pIm0List               List of IM0 data sets
 *  @param[in]      NumOfIm0ListEntries     Number of entries in pIm0List
 *
 *  @return                                 Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Setup device
 *
 */
PNIO_UINT32 PNIOExt_DeviceSetup(PNUSR_DEVICE_INSTANCE  *pPnUsrDev,
                                PNIO_SUB_LIST_ENTRY    *pIoSubList,
                                PNIO_UINT32            NumOfSublistEntries,
                                PNIO_IM0_LIST_ENTRY    *pIm0List,
                                PNIO_UINT32            NumOfIm0ListEntries)
{

    PNIO_UINT32 data_size, DevTypeLen, i, SublistEntriesDataSize, Im0ListEntriesDataSize;
    PNIO_UINT8 *data_ptr;
    PNUSR_DEV_EXP_INSTANCE DeviceInstance;
    PNIO_IM0_LIST_ENTRY *pIm0ToProcess;

	NumOfAr = 0;

    /* Init AMR */
    AMR_Init ();

    /* Initialize structures for processing cyclicall IO data */
    PnUsr_cbf_iodapi_event_varinit (pIoSubList, NumOfSublistEntries);
    pnpb_xhif_IO_buffer_init(pIoSubList, NumOfSublistEntries);

    DevTypeLen = 0;
    for(i = 0; i < PNPB_MAX_DEVTYPE_LEN; i++)
    {
        if('\0' == (*(pPnUsrDev->pDevType + i)))
        {
            DevTypeLen = i;
            break;
        }
    }
    if(0 == DevTypeLen)
    {
        printf("Error: Invalid Device type name\n");
        return PNPB_NOT_OK;
    }

    /* Convert PNUSR_DEVICE_INSTANCE to PNUSR_DEV_EXP_INSTANCE so it can be further processed by memcpy */
    /* PNUSR_DEV_EXP_INSTANCE does not carry pDevType as Device type is processed separately */
    DeviceInstance.VendorId = pPnUsrDev->VendorId;
    DeviceInstance.DeviceId = pPnUsrDev->DeviceId;
    DeviceInstance.MaxNumOfSubslots = pPnUsrDev->MaxNumOfSubslots;
    DeviceInstance.MaxNumOfBytesPerSubslot = pPnUsrDev->MaxNumOfBytesPerSubslot;
    DeviceInstance.DevTypeLen = DevTypeLen;

    /* Actualize data about fw version in IM0data */
    pIm0ToProcess = pIm0List;
    for(i = 0; i < NumOfIm0ListEntries; i++)
    {
        pIm0ToProcess->Im0Dat.SwRevision.srp = DeviceFwVersion.VerPrefix;
        pIm0ToProcess->Im0Dat.SwRevision.fe = DeviceFwVersion.VerHh;
        pIm0ToProcess->Im0Dat.SwRevision.bf = DeviceFwVersion.VerH;
        pIm0ToProcess->Im0Dat.SwRevision.ic = DeviceFwVersion.VerL;
        pIm0ToProcess->Im0Dat.Revcnt = DeviceFwVersion.VerLl;
        pIm0ToProcess++;
    }

    SublistEntriesDataSize = (sizeof(PNIO_SUB_LIST_ENTRY)) * NumOfSublistEntries;
    Im0ListEntriesDataSize = (sizeof(PNIO_IM0_LIST_ENTRY)) * NumOfIm0ListEntries;

    /* Size of data to be transferred - to allocate correct size of buffer */
    data_size  = 8;                                     /* NumOfSublistEntries + NumOfIm0ListEntries */
    data_size += sizeof(PNUSR_DEV_EXP_INSTANCE);        /* PNUSR_DEVICE_INSTANCE without pDevType */
    data_size += DevTypeLen;                            /* DevType */
    data_size += SublistEntriesDataSize;                /* Size of Submodule Entries list */
    data_size += Im0ListEntriesDataSize;                /* Size of IM0 Entries list */

    PNIO_DEVICE_SETUP_PRM* params = calloc(data_size, 1);

    /* Collect all params into one buffer */
    params->NumOfSublistEntries = NumOfSublistEntries;
    params->NumOfIm0ListEntries = NumOfIm0ListEntries;

    /* Copy data from structure pointers*/
    data_ptr = &(params->Data); /* Position pointer in prepared buffer */

    /* Submodule Entries list */
    memcpy(data_ptr, pIoSubList, SublistEntriesDataSize);
    data_ptr += SublistEntriesDataSize;   /* Position pointer in prepared buffer */
    /* IM0 Entries list */
    memcpy(data_ptr, pIm0List, Im0ListEntriesDataSize);
    data_ptr += Im0ListEntriesDataSize;   /* Position pointer in prepared buffer */
    /* PNUSR_DEVICE_INSTANCE without pDevType */
    PNUSR_DEV_EXP_INSTANCE* pDeviceInstance = &DeviceInstance;
    memcpy(data_ptr, pDeviceInstance, (sizeof(PNUSR_DEV_EXP_INSTANCE)));
    data_ptr += (sizeof(PNUSR_DEV_EXP_INSTANCE));   /* Position pointer in prepared buffer */
    /* Device type */
    memcpy(data_ptr, pPnUsrDev->pDevType, DevTypeLen);

    //Send initial xhif data buffer
    pnpb_xhif_send_all_IO_data();
    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_DEVICE_SETUP,
            data_size, (PNIO_UINT8*)(params));

    /* Free used resources */
    free(params);
    return PNPB_OK;

}/* PNIOExt_DeviceSetup */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      void
 *
 *  @return         void
 *
 *  Start PN device
 *
 */

PNIO_VOID PNIOext_device_start(PNIO_VOID)
{
    /* Collect all params into one buffer */
    /* No params */

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_DEVICE_START, 0, (void*)0);
}

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      void
 *
 *  @return         void
 *
 *  Stop PN device
 *
 */

PNIO_VOID PNIOext_device_stop(PNIO_VOID)
{
    /* Collect all params into one buffer */
    /* No params */

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_DEVICE_STOP, 0, (void*)0);
}

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      ArNum		PNIO_UINT32		AR number
 *
 *  @return         void
 *
 *  Abort AR
 *
 */

PNIO_VOID PNIOext_device_ar_abort(PNIO_UINT32 ArNum)
{
	PNIO_AR_ABORT_PRM params;
    /* Collect all params into one buffer */
    params.ArNum = ArNum;

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_DEVICE_AR_ABORT, sizeof(params), (PNIO_UINT8*)(&params));
}

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      void
 *
 *  @return         void
 *
 *  Activate IO data exchange
 *
 */

PNIO_VOID PNIOext_ActivateIoDatXch(PNIO_VOID)
{
    /* Collect all params into one buffer */
        /* No params */

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_ACTIVATE_IO_DAT_XCH, 0, (void*)0);

}   /* PNIOext_ActivateIoDatXch */


/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      void
 *
 *  @return         void
 *
 *  Deactivate IO data exchange
 *
 */

PNIO_VOID PNIOext_DeactivateIoDatXch(PNIO_VOID)
{
    /* Collect all params into one buffer */
        /* No params */

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_DEACTIVATE_IO_DAT_XCH, 0, (void*)0);

}   /* PNIOext_DeactivateIoDatXch */


/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      void
 *
 *  @return         void
 *
 *  Reboot OS on ERTEC
 *
 */

PNIO_VOID PNIOext_slave_reboot(PNIO_VOID)
{
    /* Collect all params into one buffer */
    /* No params */

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_SLAVE_REBOOT, 0, (void*)0);
}

/*
 * Pull/plug submodules
 */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]		pIoSubList				PNIO_SUB_LIST_ENTRY*	plugged submodules, including PDEV
 *  @param[in]		NumOfSubListEntries		PNIO_UINT32				number of entries in pIoSubList
 *  @param[in]		pIm0List				PNIO_IM0_LIST_ENTRY*	list of IM0 data sets
 *  @param[in]		NumOfIm0ListEntries		PNIO_UINT32				number of entries in pIm0List
 *
 *  @return         void
 *
 *  Plug all modules and submodules
 *
 */
//lint -e{832} Parameter 'Symbol' not explicitly declared, int assumed
PNIO_VOID PNIOext_sub_plug_list(
		PNIO_SUB_LIST_ENTRY* pIoSubList,		// plugged submodules, including PDEV
		PNIO_UINT32 NumOfSubListEntries,		// number of entries in pIoSubList
		PNIO_IM0_LIST_ENTRY* pIm0List,			// list of IM0 data sets
		PNIO_UINT32 NumOfIm0ListEntries)		// number of entries in pIm0List
{
	PNIO_SUB_PLUG_LIST_PRM* params = calloc(
			sizeof(PNIO_SUB_PLUG_LIST_PRM)
					+ sizeof(PNIO_SUB_LIST_ENTRY) * NumOfSubListEntries
					+ sizeof(PNIO_IM0_LIST_ENTRY) * NumOfIm0ListEntries, 1);

	/* Collect all specific params into one buffer */
	params->NumOfSubListEntries = NumOfSubListEntries;
	params->NumOfIm0ListEntries = NumOfIm0ListEntries;

	/* Data copy */
	memcpy(&params->Data[0], pIoSubList,
			sizeof(PNIO_SUB_LIST_ENTRY) * NumOfSubListEntries);
	memcpy(&params->Data[sizeof(PNIO_SUB_LIST_ENTRY) * NumOfSubListEntries],
			pIoSubList, sizeof(PNIO_IM0_LIST_ENTRY) * NumOfIm0ListEntries);

	/* Send params and function ID to cyclical buffer */
	pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_SUB_PLUG_LIST,
			(sizeof(PNIO_SUB_PLUG_LIST_PRM)
					+ sizeof(PNIO_SUB_LIST_ENTRY) * NumOfSubListEntries
					+ sizeof(PNIO_IM0_LIST_ENTRY) * NumOfIm0ListEntries),
			(PNIO_UINT8*) (params));

	/* Free allocated memory */
	free(params);
} /* PNIOext_sub_plug_list */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]		Addr			PNIO_DEV_ADDR*			geographical or logical address
 *  @param[in]		ModIdent		PNIO_UINT32				module ident number
 *  @param[in]		SubIdent		PNIO_UINT32				submodule ident number
 *  @param[in]		Im0Support		PNIO_IM0_SUPP_ENUM		subslot has IM0 data for subslot/slot/device/nothing
 *	@param[in]		pIm0Dat			IM0_DATA*				pointer to IM0 data set (if subslot has own IM0 data)
 *	@param[in]		IopsIniVal		PNIO_UINT8				initial value for iops-input, ONLY FOR SUBMOD WITHOUT IO DATA (e.g. PDEV)
 *
 *  @return         void
 *
 *  Plug submodules
 *
 */

PNIO_VOID PNIOext_sub_plug(
		PNIO_DEV_ADDR *Addr,			// geographical or logical address
		PNIO_UINT32 ModIdent,			// module ident number
		PNIO_UINT32 SubIdent,			// submodule ident number
        PNIO_UINT32 InputDataLen,	    // submodule input data length
        PNIO_UINT32 OutputDataLen,		// submodule output data length
        PNIO_IM0_SUPP_ENUM Im0Support,	// subslot has IM0 data for subslot/slot/device/nothing
        IM0_DATA* pIm0Dat,				// pointer to IM0 data set (if subslot has own IM0 data)
        PNIO_UINT8 IopsIniVal)			// initial value for iops-input, ONLY FOR SUBMOD WITHOUT IO DATA (e.g. PDEV)
{
	PNIO_SUB_PLUG_PRM* params = calloc(
			sizeof(PNIO_SUB_PLUG_PRM), 1);
	PNIO_UINT32 j;
	/* Address copy */
	params->Addr.Type = Addr->Type;
	params->Addr.ad = Addr->ad;

	/* Collect all specific params into one buffer */
	params->ModIdent = ModIdent;
	params->SubIdent = SubIdent;
    params->InputDataLen = InputDataLen;
    params->OutputDataLen = OutputDataLen;
    params->IopsIniVal = IopsIniVal;
    params->Im0Support = Im0Support;

    IM0_DATA* pIM0 = &(params->Im0Dat);
    /* Data copy */
    if(pIm0Dat == NULL)
    {
        /* if null, fill memory with zeros */
        memset(pIM0, 0, sizeof(IM0_DATA));
    }
    else
    {
        memcpy(pIM0, pIm0Dat, sizeof(IM0_DATA));
    }

	for (j = 0; j < PNPB_XHIF_MAX_NUM_OF_SUBMODULES; j++)
    {
        if (pnpb_submodule_params[j] != NULL)
        {
            if(pnpb_submodule_params[j]->Slot  == params->Addr.ad.Geo1.Slot)
            { //slot found
                if (pnpb_submodule_params[j]->Subslot  == params->Addr.ad.Geo1.Subslot
                    ||  pnpb_submodule_params[j]->ModId != ModIdent)
                {//same subslot or ModId
                     printf("Slot is already occupied and no plug was done!\n");
                     return;
                }
            }
        }
    }

    for (j = 0; j < PNPB_XHIF_MAX_NUM_OF_SUBMODULES; ++j)
    {
        if(NULL == pnpb_submodule_params[j])
        {
            pnpb_submodule_params[j] = calloc(sizeof(pnpb_subslot_params), 1);
            pnpb_submodule_params[j]->Slot = params->Addr.ad.Geo1.Slot;
            pnpb_submodule_params[j]->Subslot = params->Addr.ad.Geo1.Subslot;
            pnpb_submodule_params[j]->InData_size = InputDataLen;
            pnpb_submodule_params[j]->OutData_size = OutputDataLen;
            pnpb_submodule_params[j]->ModId = ModIdent;

            pnpb_submodule_params[j]->Direction = PNPB_XHIF_DIRECTION_NO_DATA;
            if(0 < InputDataLen)
            {
                pnpb_submodule_params[j]->Direction |=  PNPB_XHIF_DIRECTION_IN;
                InDatLen[params->Addr.ad.Geo1.Slot][params->Addr.ad.Geo1.Subslot] = InputDataLen;
            }
            if(0 < OutputDataLen)
            {
                pnpb_submodule_params[j]->Direction |=  PNPB_XHIF_DIRECTION_OUT;
                OutDatLen[params->Addr.ad.Geo1.Slot][params->Addr.ad.Geo1.Subslot] = OutputDataLen;
            }

            pnpb_submodule_params[j]->IOcS = PNIO_S_GOOD;
            pnpb_submodule_params[j]->IOpS = PNIO_S_BAD;

            pnpb_submodule_params[j]->Update = PNPB_FALSE;

            if(0 < (pnpb_submodule_params[j]->InData_size + pnpb_submodule_params[j]->OutData_size))    /* have data */
            {
                pnpb_submodule_IO_data[j] = calloc((pnpb_submodule_params[j]->InData_size + pnpb_submodule_params[j]->OutData_size), 1);
            }
            else
            {
                pnpb_submodule_IO_data[j] = PNPB_NULL;
            }
            pnpb_dev_params.NoOfSubmodules++;

            LastPluggedSlot = params->Addr.ad.Geo1.Slot;
            LastPluggedSubslot = params->Addr.ad.Geo1.Subslot;
            break;
        }
    }

	wasPlug = PNIO_TRUE;


	/* Send params and function ID to cyclical buffer */
	pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_SUB_PLUG, sizeof(PNIO_SUB_PLUG_PRM),
			(PNIO_UINT8*) (params));


	/* Free allocated memory */
	free(params);
} /* PNIOext_sub_plug */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]		Addr			PNIO_DEV_ADDR*			geographical or logical address
 *
 *  @return         void
 *
 *  Pull submodule
 *
 */

PNIO_VOID PNIOext_sub_pull(PNIO_DEV_ADDR *Addr)	// geographical or logical address
{
	PNIO_SUB_PULL_PRM* params = calloc(
			sizeof(PNIO_SUB_PULL_PRM), 1);

	/* Address copy */
	params->Addr.Type = Addr->Type;
	params->Addr.ad = Addr->ad;

	LastPulledSlot = Addr->ad.Geo1.Slot;
	LastPulledSubslot = Addr->ad.Geo1.Subslot;
	wasPlug = PNIO_FALSE;

	/* Send params and function ID to cyclical buffer */
	pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_SUB_PULL, sizeof(PNIO_SUB_PULL_PRM),
			(PNIO_UINT8*) (params));

	/* Free allocated memory */
	free(params);
} /* PNIOext_sub_pull */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      Buffer              Ptr to submodule output substitute data
 *  @param[in]      BufLen              Length received from ERTEC
 *  @param[in]      BufAddr             Address received from ERTEC
 *  @param[in]      SubstMode           [BIG ENDIAN] SubstitutionMode
 *  @param[in]      SubstModeAddr       Address received from ERTEC
 *  @param[in]      SubstActive         [BIG ENDIAN] SubstituteActiveFlag
 *  @param[in]      SubstActiveAddr     Address received from ERTEC
 *
 *  @return         void
 *
 *  Pass data to ERTEC
 *
 */

PNIO_VOID PNIOext_substval_out_read_done(
        PNIO_UINT8 *Buffer,          /* Ptr to submodule output substitute data */
        PNIO_UINT32 BufLen,          /* Pass length from ERTEC */
        PNIO_UINT32 BufAddr,         /* Pass address from ERTEC */
        PNIO_UINT16 SubstMode,       /* [BIG ENDIAN] SubstitutionMode: 0=ZERO or inactive (default), 1:LastValue, 2:Replacement value SubstitutionMode: 0=ZERO or inactive, 1:LastValue, 2:Replacement value */
        PNIO_UINT32 SubstModeAddr,   /* Pass address from ERTEC */
        PNIO_UINT16 SubstActive,     /* [BIG ENDIAN] SubstituteActiveFlag:  0=operation, 1=substitute. default value is 0: if (IOPS & IOCS = GOOD), else: 1 */
        PNIO_UINT32 SubstActiveAddr) /* Pass address from ERTEC */
{
    PNIO_SUBSTVAL_OUT_READ_DONE_PRM* params;

    /* Alloc memory */
    params = calloc(sizeof(PNIO_SUBSTVAL_OUT_READ_DONE_PRM) + BufLen, 1);

    /* Collect all params into one buffer */
    params->BufLen = BufLen;
    params->BufAddr = BufAddr;
    params->SubstMode = SubstMode;
    params->SubstModeAddr = SubstModeAddr;
    params->SubstActive = SubstActive;
    params->SubstActiveAddr = SubstActiveAddr;

    /* Data copy */
    //lint -e{668} Possibly passing a null pointer to function 'Symbol', Context Reference
    memcpy(&params->BufData[0], Buffer, BufLen);

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_SUBSTVAL_OUT_READ_DONE,
        sizeof(PNIO_SUBSTVAL_OUT_READ_DONE_PRM) + BufLen, (PNIO_UINT8*)(params));
} /* PNIOext_substval_out_read_done */

/*
 * Watchdog feature
 */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      time
 *  @param[in]      granity
 *
 *  @return         void
 *
 *  Configure watchdog
 *
 */

PNIO_VOID PNIOext_hw_watchdog_set(PNIO_UINT32 time_watchdog, PNIO_WD_GRANITY granity)
{
    PNIO_HW_WD_SET_PRM params;

    /* Init granity to zero first! Enum value change only one byte,
     * but there are four bytes allocated on the stack! */
    memset(&params, 0, sizeof(params));

    /* Collect all params into one buffer */
    params.time = time_watchdog;
    params.granity = granity;

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_HW_WATCHDOG_SET, sizeof(params), (PNIO_UINT8*)(&params));

}   /* PNIOext_hw_watchdog_set */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      command
 *
 *  @return         void
 *
 *  Send command to watchdog
 *
 */

PNIO_VOID PNIOext_hw_watchdog_command(PNIO_UINT32 command)
{
    PNIO_HW_WD_COMMAND_PRM params;
    /* Collect all params into one buffer */
    params.command = command;

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_HW_WATCHDOG_COMMAND, sizeof(params), (PNIO_UINT8*)(&params));
}   /* PNIOext_hw_watchdog_command */

/*
 * Change device name , IP and MAC address
 */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      mac_addr
 *
 *  @return         void
 *
 *  Set a new mac address
 *
 */

PNIO_VOID PNIOext_store_new_MAC(PNIO_UINT8* mac_addr)
{
    PNIO_STORE_NEW_MAC_PRM params;

    /* Collect all params into one buffer */
    memcpy(&params.mac_addr, mac_addr, 6);

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_STORE_NEW_MAC, sizeof(params), (PNIO_UINT8*)(&params));
}

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      ip_suite
 *
 *  @return         void
 *
 *  Set a new IP address
 *
 */

PNIO_VOID PNIOext_store_new_IP(PNIO_UINT8* ip_suite)
{
    PNIO_STORE_NEW_IP_PRM params;

    /* Collect all params into one buffer */
    memcpy(&params.ip_suite[0], ip_suite, 12);

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_STORE_NEW_IP, sizeof(PNIO_STORE_NEW_IP_PRM), (PNIO_UINT8*)(&params));
}

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      dev_name
 *
 *  @return         void
 *
 *  Set a new device name
 *
 */

PNIO_VOID PNIOext_store_new_device_name(PNIO_UINT8* dev_name)
{
    PNIO_STORE_NEW_DEV_NAME_PRM* params = calloc(sizeof(PNIO_STORE_NEW_DEV_NAME_PRM) + strlen(dev_name) + 1, 1);

    /* Collect all params into one buffer */
    strcpy(&params->dev_name[0], dev_name);
    /* Make sure that string will be terminated */
    params->dev_name[strlen(dev_name)] = '\0';
    params->dev_len = strlen(dev_name);

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_STORE_NEW_DEVICE_NAME,
            sizeof(PNIO_STORE_NEW_DEV_NAME_PRM) + strlen(dev_name) + 1, (PNIO_UINT8*)(params));
}

/*
 * Trace functionality
 */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      command
 *
 *  @return         void
 *
 *  Send trace command
 *
 */
PNIO_VOID PNIOext_trace_command(PNIO_TRACE_COMMAND command)
{
    PNIO_TRACE_COMMAND_PRM params;

#if(USE_ERTEC_NV_MEM == 0)
    if(command == PNIO_TRACE_RESTORE)
    {
    	int i;
	    char *buffer;
	    int string_size, read_size;

        /* Wait for previous trace command to end */
        sem_wait(&TracesSemId);

    	/* Restore and print traces from NV memory */
	    /* r: Opens a file for reading. The file must exist. */
    	trace_file_fd = fopen(NV_TRACE_FILE_NAME, "r");
        if (trace_file_fd == NULL)
        {
            printf("ERROR (FlashedTraceBuffer): No valid trace buffer found on flash!\n");
            sem_post(&TracesSemId);
            return;
        }

        /* Seek to the beginning of the file */
        fseek(trace_file_fd, 0, SEEK_END);
        /* Get filesize */
	    string_size = ftell(trace_file_fd);
	    /* Point back to beginning of file */
	    rewind(trace_file_fd);
        /* Alloc buffer */
	    buffer = (char*) calloc((string_size + 1), 1);
        if(NULL == buffer)
        {
            printf("ERROR: Memory could not be allocated!\n");
            fclose(trace_file_fd);
            sem_post(&TracesSemId);
            PNPB_LIB_FATAL;
        }
        else
        {
            /* Read traces and terminate string */
            read_size = fread(buffer, 1, string_size, trace_file_fd);
            buffer[string_size] = '\0';

            /* Print traces */
            printf ("FlashedTraceBuffer (Size=%dB)\n", read_size);
            printf ("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

            /* Print trace */
            for(i = 0; i < read_size; i++)
            {
                putchar(buffer[i]);
            }

            printf ("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

            /* close file and free memory */
            fclose(trace_file_fd);
            free(buffer);

            /* Allow trace commands */
            sem_post(&TracesSemId);

            return;
        }
    }
    else if(command == PNIO_TRACE_SAVE)
    {
        /* Wait for previous trace command to end */
        sem_wait(&TracesSemId);

        printf("Flash Trace Buffer ...\n");

    	printf("Now erasing flash... ");

    	/* Open trace file - will be filled after callback */
    	/* w or wb: Truncate to zero length or create file for writing. */
    	trace_file_fd = fopen(NV_TRACE_FILE_NAME, "w");
    	if(trace_file_fd != NULL)
    	{
    		printf("ok, done\n");
    		printf("Now programming flash... ");
    	}
    	else
    	{
    		printf("Couldn't find the file %s.\n",NV_TRACE_FILE_NAME);
            return;
    	}
    }
    else if (command == PNIO_TRACE_PRINT_START)
    {
        /* Wait for previous trace command to end */
        sem_wait(&TracesSemId);
    }

#else
    if(command == PNIO_TRACE_SAVE)
    {
    	printf ("Traces has been saved on Ertec non-volatile memory!\n");
    }
#endif

    /* Prepare memory */
    memset(&params, 0, sizeof(PNIO_TRACE_COMMAND_PRM));

    /* Collect all params into one buffer */
    params.command = command;

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_TRACE_COMMAND, sizeof(params), (PNIO_UINT8*)(&params));
}   /* PNIOext_trace_command */


/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      command
 *
 *  @return         void
 *
 *  Send trace settings
 *
 */
PNIO_VOID PNIOext_trace_settings(PNIO_TRACE_SETTINGS_COMMAND command, PNIO_UINT32 module, PNIO_UINT32 trc_level)
{
    PNIO_TRACE_SETTINGS_PRM params;

    /* Prepare memory */
    memset(&params, 0, sizeof(PNIO_TRACE_SETTINGS_PRM));

    /* Collect all params into one buffer */
    params.command = command;
    params.module = module;
    params.trc_level = trc_level;

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_TRACE_SETTINGS, sizeof(params), (PNIO_UINT8*)(&params));
}   /* PNIOext_trace_settings */


/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      *prm                Parameters of received write
 *  @param[in]      *pBuffer            Buffer with read data to be returned to stack
 *  @param[in]      *pPnioState         Pointer to operation error result - will be returned to stack
 *
 *  @return         void
 *
 *  Sends response to record read - data from device to CPU
 *
 */
PNIO_VOID PNIOext_rec_read_rsp(PNIO_RECORD_READ_PRM *prm, PNIO_UINT8 *pBuffer, PNIO_ERR_STAT *pPnioState)
{
    PNIO_RECORD_READ_RSP_PRM *params;
    PNIO_UINT32 params_size;

    /* Size of data to transfer */
    params_size = sizeof(PNIO_RECORD_READ_RSP_PRM) + prm->BufLen - sizeof(PNIO_UINT32);

    params = calloc(params_size, 1);

    params->pBufLen = prm->pBufLen;
    params->BufLen = prm->BufLen;
    params->pPnioState = prm->pPnioState;
    params->pBuffer = prm->pBuffer;
    memcpy(&(params->PnioState), pPnioState, sizeof(PNIO_ERR_STAT));
    if(prm->BufLen != 0)
    {
        if(NULL != pBuffer)
        {
            memcpy(&(params->Buffer), pBuffer, prm->BufLen);
        }
        else
        {
            printf ("Error: Null pointer. Memory corrupted.\n");
            PNPB_LIB_FATAL;
        }
    }
    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_REC_READ_RSP, params_size, (PNIO_UINT8*)(params));

    /* Free used resources */
    free(params);

}   /* PNIOext_rec_read_rsp */


/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      *prm                Parameters of received write
 *  @param[in]      *pPnioState         Pointer to operation error result - will be returned to stack
 *
 *  @return         void
 *
 *  Sends response to record write - data from CPU to device
 *
 */
PNIO_VOID PNIOext_rec_write_rsp(PNIO_RECORD_WRITE_PRM *prm, PNIO_ERR_STAT *pPnioState)
{
    PNIO_RECORD_WRITE_RSP_PRM *params;
    PNIO_UINT32 params_size;

    /* Size of data to transfer */
    params_size = sizeof(PNIO_RECORD_WRITE_RSP_PRM);

    params = calloc(params_size, 1);

    params->pBufLen = prm->pBufLen;
    params->BufLen = prm->BufLen;
    params->pPnioState = prm->pPnioState;

    memcpy(&(params->PnioState), pPnioState, sizeof(PNIO_ERR_STAT));
    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_REC_WRITE_RSP, params_size, (PNIO_UINT8*)(params));

    /* Free used resources */
    free(params);

}   /* PNIOext_rec_write_rsp */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      *prm                Parameters of received write
 *  @param[in]      *pPnioState         Pointer to operation error result - will be returned to stack
 *
 *  @return         void
 *
 *  Sends response to record write - data from CPU to device
 *
 */
PNIO_VOID PNIOext_amr_response_handler_rsp(PNIO_AMR_HANDLER_PRM *prm, PNIO_ERR_STAT *pPnioState, PNIO_UINT8* pData)
{
    PNIO_AMR_HANDLER_RSP_PRM *params;
    PNIO_UINT32 params_size;

    /* Size of data to transfer */
    params_size = sizeof(PNIO_AMR_HANDLER_RSP_PRM) + prm->pBufLen;

    params = calloc(params_size, 1);

    /* Copy original parameters */
    params->pAMRHndlPrm = *prm;
    memcpy(&(params->PnioState), pPnioState, sizeof(PNIO_ERR_STAT));
    if(prm->pBufLen != 0)
    {
        if( (NULL != params->pData) && (NULL != pData))
        {
            memcpy(params->pData, pData, prm->pBufLen);
        }
        else
        {
            printf ("Error: Null pointer. Memory corrupted.\n");
            PNPB_LIB_FATAL;
        }
    }
    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_AMR_READ_RSP, sizeof(PNIO_AMR_HANDLER_RSP_PRM) + prm->pBufLen, (PNIO_UINT8*)(params));

    /* Free used resources */
    free(params);
}

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      *prm                Parameters of received write
 *  @param[in]      *pPnioState         Pointer to operation error result - will be returned to stack
 *
 *  @return         void
 *
 *  Sends response to record write - data from CPU to device
 *
 */
PNIO_VOID PNIOext_pe_response_handler_rsp(PNIO_PE_RESPONSE_HANDLER_PRM *prm, PNIO_ERR_STAT *pPnioState, PNIO_UINT8* pData)
{
    PNIO_PE_RESPONSE_HANDLER_RSP_PRM *params;

    params = (PNIO_PE_RESPONSE_HANDLER_RSP_PRM*) calloc(sizeof(PNIO_PE_RESPONSE_HANDLER_RSP_PRM) + prm->pBufLen, 1);

    /* Copy original parameters */
    memcpy(&(params->pPEHndlPrm), prm, sizeof(PNIO_PE_RESPONSE_HANDLER_PRM));
    memcpy(&(params->PnioState), pPnioState, sizeof(PNIO_ERR_STAT));

    if(prm->pBufLen > 0)
    {
        if(NULL != pData)
        {
            memcpy(&params->pData[0], pData, prm->pBufLen);
        }
        else
        {
            printf ("Error: Null pointer. Memory corrupted.\n");
            PNPB_LIB_FATAL;
        }
    }

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_PE_RESPONSE_RSP, sizeof(PNIO_PE_RESPONSE_HANDLER_RSP_PRM) + prm->pBufLen, (PNIO_UINT8*)(params));

    /* Free used resources */
    free(params);
}

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      *prm                Parameters of received write
 *  @param[in]      *pPnioState         Pointer to operation error result - will be returned to stack
 *
 *  @return         void
 *
 *  Sends response to record write - data from CPU to device
 *
 */
PNIO_VOID PNIOext_pe_request_handler_rsp(PNIO_ERR_STAT *pPnioState)
{
    PNIO_PE_REQUEST_HANDLER_RSP_PRM *params;

    params = calloc(sizeof(PNIO_PE_REQUEST_HANDLER_RSP_PRM), 1);

    /* Copy state */
    memcpy(&(params->PnioState), pPnioState, sizeof(PNIO_ERR_STAT));

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_PE_REQUEST_RSP, sizeof(PNIO_PE_REQUEST_HANDLER_RSP_PRM), (PNIO_UINT8*)(params));

    /* Free used resources */
    free(params);
}

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      ArNum               Aplication relation number
 *
 *  @return                             APDU status read from Ertec
 *
 *  Sends response to record write - data from CPU to device
 *
 */
PNIO_UINT32 PNIOext_get_last_apdu_status(PNIO_UINT32 ArNum)
{
    PNIO_UINT32 Apdu = 0;

    Apdu = pnpb_ar_apdu_status[ArNum - 1];

    /* return result - value was updated */
    return Apdu;
}   /* PNIOext_get_last_apdu_status */

/*
 * Non-volatile memory functionality
 */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      pBufLen
 *  @param[in]      *pData
 *
 *  Sends initialization of non-volatile data to Ertec
 *
 */
PNIO_VOID PNIOext_nv_data_init(PNIO_UINT8* pData, PNIO_UINT32 pBufLen)
{
    PNIO_NV_DATA_INIT_PRM *params;

    params = (PNIO_NV_DATA_INIT_PRM *) calloc(sizeof(PNIO_NV_DATA_INIT_PRM) + pBufLen, 1);

    if((pBufLen > 0) && (pData != NULL))
    {
        /* Copy state */
        //lint -e{419, 420} pData and params->pData are allready allocated
        memcpy(&(params->pData), pData, pBufLen);
        params->useErtecNVMem = 0;
    }
    else
    {
        params->useErtecNVMem = 1;
    }
    params->pBufLen = pBufLen;

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_NV_DATA_INIT, sizeof(PNIO_NV_DATA_INIT_PRM) + pBufLen, (PNIO_UINT8*)(params));

    /* Free used resources */
    free(params);
}

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      RtfOption
 *
 *  Sends clear of non-volatile data to Ertec
 *
 */
PNIO_VOID PNIOext_nv_data_clear(PNIO_UINT32 RtfOption)
{
    PNIO_NV_DATA_CLEAR_PRM *params;

    params = (PNIO_NV_DATA_CLEAR_PRM *) calloc(sizeof(PNIO_NV_DATA_CLEAR_PRM), 1);
    params->RtfOption = RtfOption;

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_NV_DATA_CLEAR, sizeof(PNIO_NV_DATA_CLEAR_PRM), (PNIO_UINT8*)(params));

    /* Free used resources */
    free(params);
}

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      NvDataType
 *  @param[in]      pBufLen
 *  @param[in]      *pData
 *
 *  Sends non-volatile data store command to Ertec
 *
 */
PNIO_VOID PNIOext_nv_data_store(PNIO_UINT32 NvDataType, PNIO_UINT32 pBufLen, PNIO_UINT8* pData)
{
    PNIO_NV_DATA_STORE_PRM *params;

    params = (PNIO_NV_DATA_STORE_PRM *) calloc(sizeof(PNIO_NV_DATA_STORE_PRM) + pBufLen, 1);

    if((pBufLen > 0) && (pData != NULL))
    {
        /* Copy state */
        memcpy(&(params->pData), pData, pBufLen);
    }

    params->pBufLen = pBufLen;
    params->NvDataType = NvDataType;

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_NV_DATA_STORE, sizeof(PNIO_NV_DATA_STORE_PRM) + pBufLen, (PNIO_UINT8*)(params));

    /* Free used resources */
    free(params);
}

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  @param[in]      NvDataType
 *  @param[in]      ModIdent
 *  @param[in]      pBufLen
 *  @param[in]      *pData
 *
 *  Sends non-volatile IM data store command to Ertec
 *
 */
PNIO_VOID PNIOext_im_data_store(PNIO_UINT32 NvDataType, PNIO_UINT32 ModIdent, PNIO_UINT32 pBufLen, PNIO_UINT8* pData)
{
    PNIO_NV_IM_DATA_STORE_PRM *params;

    params = (PNIO_NV_IM_DATA_STORE_PRM *) calloc(sizeof(PNIO_NV_IM_DATA_STORE_PRM) + pBufLen, 1);

    if((pBufLen > 0) && (pData != NULL))
    {
        /* Copy state */
        memcpy(&(params->pData), pData, pBufLen);
    }

    params->pBufLen = pBufLen;
    params->NvDataType = NvDataType;
    params->ModIdent = ModIdent;

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_NV_DATA_IM_STORE, sizeof(PNIO_NV_IM_DATA_STORE_PRM) + pBufLen, (PNIO_UINT8*)(params));

    /* Free used resources */
    free(params);
}

/*
 * IM data handling
 */

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  Send result of IM write command to Ertec
 *
 */
PNIO_VOID PNIOext_im_write_rsp(PNIO_ERR_STAT* PnioState)
{
    PNIO_IM_WRITE_RSP_PRM *params;

    params = (PNIO_IM_WRITE_RSP_PRM *) calloc(sizeof(PNIO_IM_WRITE_RSP_PRM), 1);

    /* Status copy */
    memcpy(&(params->PnioState), PnioState, sizeof(PNIO_ERR_STAT));

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_IM_WRITE_RSP, sizeof(PNIO_IM_WRITE_RSP_PRM), (PNIO_UINT8*)(params));

    /* Free used resources */
    free(params);
}

/**
 *  @brief Collect parameters, sends acyclic message
 *
 *  Send result of IM read command together with data to Ertec
 *
 */
PNIO_VOID PNIOext_im_read_rsp(PNIO_IM_READ_PRM* pParams, PNIO_UINT8* pBuffer)
{
    PNIO_IM_READ_RSP_PRM *params;

    params = (PNIO_IM_READ_RSP_PRM *) calloc(sizeof(PNIO_IM_READ_RSP_PRM) + pParams->pBufLen, 1);

    if((pParams->pBufLen > 0) && (pBuffer != NULL))
    {
        /* Copy data */
        memcpy(&(params->pData), pBuffer, pParams->pBufLen);
    }

    /* Parameters copy */
    params->pBufAddr = pParams->pBufAddr;
    params->pBufLen = pParams->pBufLen;
    memcpy(&(params->PnioState), &pParams->pPnioState, sizeof(PNIO_ERR_STAT));

    /* Send params and function ID to cyclical buffer */
    pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_IM_READ_RSP, sizeof(PNIO_IM_READ_RSP_PRM) + pParams->pBufLen, (PNIO_UINT8*)(params));

    /* Free used resources */
    free(params);
}

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
