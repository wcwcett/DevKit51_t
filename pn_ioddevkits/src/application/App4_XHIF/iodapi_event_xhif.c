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
/*  F i l e               &F: iodapi_event_xhif.c                       :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  PNIO event handler for this application example                          */
/*                                                                           */
/*  performs user functions, called by stack in case of events and alarms    */
/*                                                                           */
/*  To use this application example, set #define EXAMPL_DEV_CONFIG_VERSION 4 */
/*  in file \application\usriod_app.h                                        */
/*                                                                           */
/*  THIS MODULE SHOULD NOT BE MODIFIED BY THE PNIO USER                      */
/*  ALL USER MODIFICATION SHOULD BE DONE IN SOURCE FOR HOST DEVICE           */
/*                                                                           */
/*****************************************************************************/


/**
* @file     iodapi_event_xhif.c
* @brief    Main application of Devkit - Application example 4 - XHIF
*
* XHIF functionality allows user to use ERTEC Devkit only for PN-stack functionalities
* and to realize user functionality on other device. The other device have to upload
* firmware to ERTEC Devkit as a binary and then communicate data.. This can be realized
* via XHIF memory interface.
* This file is such implementation developed for BeagleBone Black driven by TI Sitara
* processor.
* Upper level interface - functionalities for buffers realized over memory interface
*/

#include "compiler.h"

#include "iod_cfg.h"
#include "usriod_cfg.h"
#include "usrapp_cfg.h"   // example application spec. configuration settings

#if (EXAMPL_DEV_CONFIG_VERSION == 4)
    #include "pniousrd.h"
    #include "iodapi_event.h"
    #include "os.h"
    #include "usriod_im_func.h"
    #include "nv_data.h"
    #include "bspadapt.h"
    #include "PnUsr_Api.h"
    #include "usriod_PE.h"
    #include "usriod_utils.h"

#include "PnUsr_xhif.h"

    /*=======================================================================================================*/
    /*lint -esym(438,DevHndl)  438 Last value assigned to variable 'DevHndl' (defined at line 1197) not used */
    /*=======================================================================================================*/

    // *=======================================================
    // *  defines
    // *=======================================================
    #define EXAMPLE_SUBSTITUTE_VALUE    0xab  // example: substituted value for output (data consumer)
    #define EXAMPLE_IN_VALUE            0x11  // example: default input value (data provider)

    #define DEMO_RECORD  "ABCDEFGH"


    // *=======================================================
    // *  extern  functions
    // *=======================================================

    // *=======================================================
    // *  extern  data
    // *=======================================================

    // *=======================================================
    // *  static  data
    // *=======================================================

    // *=======================================================
    // *  public data
    // *=======================================================
    #if  EXAMPLE_IPAR_SERVER
       // ***** EXAMPLE IPAR:  upload&storage alarm ********
       PNIO_UINT32  Example_iParData [NUMOF_SLOTS][NUMOF_SUBSLOTS]; // 4 byte example data per subslot
    #endif


    // ***** asynchronous record rd/wr only
    USR_REC_READ_RQ   PnioRecReadRq;
    USR_REC_WRITE_RQ  PnioRecWriteRq;



    // *** AR properties


    // *** Output Data (IO Controller ==> IO Device) and substituted values for output data ***
    PNIO_UINT8     OutData        [NUMOF_SLOTS][NUMOF_SUBSLOTS+1][NUMOF_BYTES_PER_SUBSLOT];
#if !IOD_INCLUDE_REC8028_8029
    PNIO_UINT8     OutSubstVal    [NUMOF_SLOTS][NUMOF_SUBSLOTS+1][NUMOF_BYTES_PER_SUBSLOT];
#endif
    PNIO_UINT16    OutDatLen      [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];
    PNIO_UINT8     OutDatIocs     [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];
    PNIO_UINT8     OutDatIops     [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];
    PNIO_UINT8     OutDatIops_old [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];

    // *** Input Data  (IO Device ==> IO Controller)
    PNIO_UINT8     InData         [NUMOF_SLOTS][NUMOF_SUBSLOTS+1][NUMOF_BYTES_PER_SUBSLOT];
    PNIO_UINT16    InDatLen       [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];
    PNIO_UINT8     InDatIops      [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];
    PNIO_UINT8     InDatIocs      [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];
    PNIO_UINT8     InDatIocs_old  [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];


    // *----------------------------------------------------------------*
    // *  get subslot index from slot and subslot                       *
    // *----------------------------------------------------------------*
    PNIO_UINT32 PnUsr_get_subslot_index(PNIO_UINT32 slot, PNIO_UINT32 subslot)
    {
        if (slot) // PERI
        {
            return subslot;
        }
        else // HEAD
        {
            switch (subslot)
            {
            case 0x0001:    return 1;
            case 0x8000:    return 2;
            case 0x8001:    return 3;
            case 0x8002:    return 4;
            case 0x8003:    return 5;
            case 0x8004:    return 6;
            default:        return 0;
            }
        }
    }

    // *----------------------------------------------------------------*
    // *
    // *  PnUsr_cbf_iodapi_event_varinit (void)
    // *
    // *  initialize static variables (must be called first during startup )
    // *----------------------------------------------------------------*
    void PnUsr_cbf_iodapi_event_varinit (PNIO_SUB_LIST_ENTRY* pIoSubList,
                                         PNIO_UINT32          NumOfSubListEntries)
    {
        PNIO_UINT32 sublistIndex;
        PNIO_UINT32 subslotIndex;

        OsMemSet (&InData[0][0][0],       EXAMPLE_IN_VALUE,   sizeof (InData));           // io data (input)
        OsMemSet (&InDatIops[0][0],       PNIO_S_GOOD,        sizeof (InDatIops));        // local provider status
        OsMemSet (&InDatIocs[0][0],       PNIO_S_GOOD,        sizeof (InDatIocs));        // remote consumer status
        OsMemSet (&InDatIocs_old[0][0],   PNIO_S_GOOD,        sizeof (InDatIocs_old));    // old value of remote consumer status

        OsMemSet (&OutData[0][0][0],      0,                  sizeof (OutData));          // io data (output)
        OsMemSet (&OutDatIocs[0][0],      PNIO_S_GOOD,        sizeof (OutDatIocs));       // local consumer status
        OsMemSet (&OutDatIops[0][0],      PNIO_S_GOOD,        sizeof (OutDatIops));       // remote provider status
        OsMemSet (&OutDatIops_old[0][0],  PNIO_S_GOOD,        sizeof (OutDatIops_old));   // old value of remote provider status


        for (sublistIndex = 0; sublistIndex < NumOfSubListEntries; sublistIndex++)
        {
            subslotIndex = PnUsr_get_subslot_index(pIoSubList->Slot, pIoSubList->Subslot);
            if (subslotIndex)
            {
                // --- inputs -----------------------------
                InDatLen   [pIoSubList->Slot][subslotIndex]  = (PNIO_UINT16) pIoSubList->InDatLen;
                // --- outputs -----------------------------
                OutDatLen   [pIoSubList->Slot][subslotIndex]  = (PNIO_UINT16) pIoSubList->OutDatLen;
            }
            pIoSubList++;
        }
    }



    // *****************************************************************************************
    // *************************** read and write input data ***********************************
    // *****************************************************************************************

    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_data_write (...)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  Passes the input data from the application to the stack.
    // *  The application reads the data from the specified input module
    // *  and handles them to the stack.
    // *  function UsrReadInputData() is called by the pnio stack.
    // *
    // *  Input:       PNIO_UINT32    DevHndl,      // [in]  Device Handle
    // *               PNIO_DEV_ADDR  *pAddr,       // [in]  location (slot, subslot)
    // *               PNIO_UINT32    data_len,     // [in] data length
    // *               PNIO_UINT8     *pBuffer      // [in,out] pointer to the input data
    // *               PNIO_UINT8     Iocs          // [in]  (io controller) consumer status
    // *
    // *    Output:    return   io-provider-status  // PNIO_S_GOOD, PNIO_S_BAD
    // *
    // *
    // *----------------------------------------------------------------*
    PNIO_IOXS    PNIO_cbf_data_write
            (PNIO_UINT32    DevHndl,        // [in]  device handle
             PNIO_DEV_ADDR  *pAddr,         // geographical or logical address
             PNIO_UINT32    BufLen,         // [in]  length of the submodule input data
             PNIO_UINT8     *pBuffer,       // [in,out] Ptr to data buffer to write to
             PNIO_IOXS      Iocs            // [in]  remote (io controller) consumer status
            )
    {
        PNIO_UINT32 slot_num    = pAddr->Geo.Slot;
        PNIO_UINT32 subslot_num = pAddr->Geo.Subslot;

        PNIO_UNUSED_ARG (DevHndl);
	    /* Perform memcpy only if there are data to be copied */
	    if((0 < BufLen) && (NULL != pBuffer))
	    {
	    	OsMemCpy (pBuffer, &InData[slot_num][subslot_num][0], BufLen);
	    }
        InDatIocs [slot_num][subslot_num] = Iocs;    // consumer status (of remote io controller)

        // *---- check provider status and notify modifications ------*
        if (InDatIocs_old [slot_num][subslot_num] != Iocs)
        {
             PNIO_printf("new IO controller input consumer status (ICS) = 0x%x in slot %d, subslot %d\n",
                          Iocs, slot_num, subslot_num);
             InDatIocs_old [slot_num][subslot_num] = Iocs;
        }
        return ( (PNIO_IOXS)InDatIops [slot_num][subslot_num]); // return local provider state
    }

    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_data_read (...)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  Passes the output data from the stack to the application.
    // *  The application takes the data and writes them to the specified
    // *  output module.
    // *  function UsrWriteOutputData() is called by the pnio stack.
    // *
    // *  Input:
    // *               PNIO_UINT32    DevHndl       // [in] device handle
    // *               PNIO_DEV_ADDR  *pAddr,       // [in]  location (slot, subslot)
    // *               PNIO_UINT32    data_len,     // [in] data length
    // *               PNIO_UINT8*    pBuffer       // [in] pointer to the input data
    // *               PNIO_UINT8     IoPs          // [in]  (io controller) provider status
    // *
    // *    Output:    return   io-consumer-status  // IOCS_STATE_GOOD, IOCS_STATE_BAD
    // *
    // *
    // *----------------------------------------------------------------*
    PNIO_IOXS     PNIO_cbf_data_read
            (PNIO_UINT32    DevHndl,        // [in]  device handle
             PNIO_DEV_ADDR  *pAddr,         // [in]  geographical or logical address
             PNIO_UINT32    BufLen,         // [in]  length of the submodule input data
             PNIO_UINT8*    pBuffer,        // [in]  Ptr to data buffer to read from
             PNIO_IOXS      Iops            // [in]  (io controller) provider status
            )
    {
        PNIO_UINT32 slot_num    = pAddr->Geo.Slot;
        PNIO_UINT32 subslot_num = pAddr->Geo.Subslot;

        PNIO_UNUSED_ARG (DevHndl);

        // save last data block
	    if((0 < BufLen) && (NULL != pBuffer))
	    {
	    	OsMemCpy (&OutData[slot_num][subslot_num][0], pBuffer, BufLen);
	    }

        OutDatIops [slot_num][subslot_num] = Iops;    // provider status (of remote io controller)

        // *---- check provider status and notify modifications ------*
        if (OutDatIops_old [slot_num][subslot_num] != Iops)
        {
            PNIO_printf("new IO controller output provider status (OPS) = 0x%x in slot %d, subslot %d\n",
                         Iops, slot_num, subslot_num);
            OutDatIops_old [slot_num][subslot_num] = Iops;
        }

        return ( (PNIO_IOXS)OutDatIocs [slot_num][subslot_num]);    // consumer state (of local io device)
    }


	/**
	 *  @brief Update only IOxS for write
	 *
	 *  @param[in]      DevHndl         Device handle
	 *  @param[in]      *pAddr          Geographical or logical address
	 *  @param[in]      Iocs            Remote consumer status
	 *
	 *  @return                         Local provider status
	 *
	 *  Write without actual update of data
	 *
	 */
    PNIO_IOXS    PNIO_cbf_data_write_IOxS_only
		    (PNIO_UINT32	DevHndl,
		     PNIO_DEV_ADDR	*pAddr,
		     PNIO_IOXS	    Iocs
            )
    {
        PNIO_UINT32 slot_num	= pAddr->Geo.Slot;
	    PNIO_UINT32 subslot_num = pAddr->Geo.Subslot;

	    PNIO_UNUSED_ARG (DevHndl);

        InDatIocs [slot_num][subslot_num] = Iocs;    // consumer status (of remote io controller)

        // *---- check provider status and notify modifications ------*
        if (InDatIocs_old [slot_num][subslot_num] != Iocs)
        {
             PNIO_printf("* new IO controller input consumer status (ICS) = 0x%x in slot %d, subslot %d\n",
                          Iocs, slot_num, subslot_num);
             InDatIocs_old [slot_num][subslot_num] = Iocs;
        }
        return ( (PNIO_IOXS)InDatIops [slot_num][subslot_num]);	// return local provider state
    }


	/**
	 *  @brief Update only IOxS for read
	 *
	 *  @param[in]      DevHndl         Device handle
	 *  @param[in]      *pAddr          Geographical or logical address
	 *  @param[in]      Iops            Remote provider status
	 *
	 *  @return                         Local consumer status
	 *
	 *  Read without actual update of data
	 *
	 */
    PNIO_IOXS     PNIO_cbf_data_read_IOxS_only
		    (PNIO_UINT32	DevHndl,
		     PNIO_DEV_ADDR	*pAddr,
		     PNIO_IOXS	    Iops
            )
    {
        PNIO_UINT32 slot_num	= pAddr->Geo.Slot;
        PNIO_UINT32 subslot_num = pAddr->Geo.Subslot;

        PNIO_UNUSED_ARG (DevHndl);

        OutDatIops [slot_num][subslot_num] = Iops;    // provider status (of remote io controller)

        // *---- check provider status and notify modifications ------*
        if (OutDatIops_old [slot_num][subslot_num] != Iops)
        {
            PNIO_printf("* new IO controller output provider status (OPS) = 0x%x in slot %d, subslot %d\n",
                         Iops, slot_num, subslot_num);
            OutDatIops_old [slot_num][subslot_num] = Iops;
        }

        return ( (PNIO_IOXS)OutDatIocs [slot_num][subslot_num]);	// consumer state (of local io device)
    }



    // ****************************************************************************************
    // ******************* connect indication and ownership indication  ***********************
    // ****************************************************************************************


    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_ar_connect_ind (pAR)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  The AR Info Indication of a module
    // *
    // *    Input:     PNIO_UINT32    pAr;      // ID of the Application Relation
    // *    Output:    ---
    // *
    // *
    // *----------------------------------------------------------------*
    void PNIO_cbf_ar_connect_ind
        (
            PNIO_UINT32     DevHndl,            // [in] handle for a multidevice
            PNIO_AR_TYPE    ArType,             // [in] type of AR (see cm_ar_type_enum)
            PNIO_UINT32     ArNum,              // [in] AR number  (device access: ArNumber = 3)
            PNIO_UINT16     ArSessionKey,       // [in] AS session key
            PNIO_UINT16     SendClock,          // [in] sendclock
            PNIO_UINT16     RedRatioIocrIn,     // [in] reduction ratio of input IOCR
            PNIO_UINT16     RedRatioIocrOut,    // [in] reduction ratio of output IOCR
            PNIO_UINT32     HostIp              // [in] ip address of host ( PN-Controller )
        )
    {
        PNIO_UNUSED_ARG (DevHndl);
        NumOfAr++;      // increment number of running ARs
        PNIOext_cbf_ar_connect_ind(ArType,ArNum, SendClock, RedRatioIocrIn, RedRatioIocrOut, ArSessionKey, HostIp);
    }


    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_ar_ownership_ind (pAR)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  The AR ownership Indication of a module
    // *
    // *  Input:   PNIO_UINT32 pAr;                     ID of the Application Relation
    // *
    // *  Output:  PNIO_EXP*   pOwnSub->OwnSessionKey   set to 0 only if ownership is rejected
    // *                                                else keep unchanged
    // *                       pOwnSub->IsWrongSubmod   set to PNIO_TRUE only if wrong module,
    // *                                                else keep unchanged
    // *
    // *----------------------------------------------------------------*
    PNIO_VOID   PNIO_cbf_ar_ownership_ind
            (
                PNIO_UINT32     DevHndl,
                PNIO_UINT32     ArNum,          // AR number 1....NumOfAR
                PNIO_EXP*       pOwnSub         // [in] expected configuration in ownership indication
            )
    {
        PNIO_UNUSED_ARG (DevHndl);

        PNIOext_cbf_ar_ownership_ind( ArNum, pOwnSub);

    }



    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_param_end_ind ()
    // *
    // *----------------------------------------------------------------*
    // *
    // *  The PNIO controller has sent an "PARAM END" event
    // *
    // *  Input:    DevHndl        Device handle
    // *            ArNum          AR number (1...N)
    // *            SessionKey     session key
    // *            Api            API (valid only, if SubslotNum <> 0)
    // *            SlotNum        SlotNum (valid only, if SubslotNum <> 0)
    // *            SubslotNum     == 0:    param end for all submodules,
    // *                           <> 0:    param end only for this submodule
    // *            MoreFollows    PNIO_TRUE: more param end ind follow, PNIO_FALSE: last one
    // *  Output:   return         PNIO_SUBMOD_STATE_RUN               module works properly and provides valid data, stack will create appl-ready now
    // *                           PNIO_SUBMOD_STATE_STOP              module has problem and can't provide valid data, stack will create appl-ready now (mod-diffblock entry)
    // *                           PNIO_SUBMOD_STATE_APPL_RDY_FOLLOWS  module parameterization is still running, application will notify appl-ready later to stack
    // *
    // *----------------------------------------------------------------*
    PNIO_SUBMOD_STATE  PNIO_cbf_param_end_ind (PNIO_UINT32 DevHndl,
                                         PNIO_UINT16 ArNum,
                                         PNIO_UINT16 SessionKey,
                                         PNIO_UINT32 Api,
                                         PNIO_UINT16 SlotNum,
                                         PNIO_UINT16 SubslotNum,
                                         PNIO_BOOL   MoreFollows)

    {
        PNIOext_cbf_param_end_ind(ArNum, SessionKey, Api, SlotNum, SubslotNum, MoreFollows);

        return (PNIO_SUBMOD_STATE_RUN);     // system generates automatically "application ready"-telegram
    }


    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_ready_for_input_update_ind ()
    // *
    // *----------------------------------------------------------------*
    // *
    // *  The PNIO controller has sent an "READY FOR INPUT UPDATE" event
    // *
    // *  NOTE:    Strongly avoid a reentrant call of UsrDbai_initiate_data_write()
    // *           or UsrDbai_initiate_data_read(), also PNIO_initiate_data_read()
    // *           or PNIO_initiate_data_write().
    // *
    // *  Input:    DevHndl        Device handle
    // *            ArNum          AR number (1...N)
    // *            InpUpdState    input update state (AR_STARTUP or AR_INDATA )
    // *  Output:   ---
    // *
    // *----------------------------------------------------------------*
    PNIO_VOID  PNIO_cbf_ready_for_input_update_ind (PNIO_UINT32 DevHndl,
                                                    PNIO_UINT16 ArNum,
                                                    PNIO_INP_UPDATE_STATE InpUpdState)
    {
        PNIO_UINT32 Status = PNIO_OK;
        PNIO_UINT32 ApduStatus;
        // *-----------------------------------------------------------------------
        // *    perform a first data exchange, to set IOPS and IOCS to STATE_GOOD
        // *    necessary to set IO controller state to RUN without error.
        // *-----------------------------------------------------------------------
    	/* Exchange cyclical IO data via xhif */
    	PnUsr_xhif_cycl_gpio_trigger();

        Status =  PNIO_initiate_data_read (DevHndl);

        { // Example: APDU status can read here
            ApduStatus = PNIO_get_last_apdu_status (PNIO_SINGLE_DEVICE_HNDL, ArNum);
            PNIO_UNUSED_ARG(ApduStatus);
        } /*lint !e438 last value assigned is not used */

    	PnUsr_xhif_cyclical_read();

        if (PNIO_OK == Status)
        {
            Status =  PNIO_initiate_data_write (DevHndl);
            if (PNIO_OK != Status)
            {
                PNIO_printf ( "##Error IO Data Exchange in PNIO_cbf_ready_for_input_update_ind()\n");
            }
        }

        PnUsr_ActivateIoDatXch();

        PNIOext_cbf_ready_for_input_update_ind(ArNum, InpUpdState, ApduStatus);
        // *-----------------------------------------------------------------------
        // *    notify, if first "readyForInputUpdate" after AR start or
        // *    because of replugging a submodule.
        // *-----------------------------------------------------------------------
        if (InpUpdState == PNIO_AR_STARTUP)
        {
            //PNIO_printf ( "##READY FOR INPUT UPDATE DURING AR_STARTUP ARnum=%d\n", ArNum);
        }
        else
        {
            //PNIO_printf ( "##READY FOR INPUT UPDATE DURING AR INDATA  ARnum=%d\n", ArNum);
        }
    }


    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_ar_indata_ind ()
    // *
    // *----------------------------------------------------------------*
    // *
    // *  The pnio stack notifies the user, that after start of data
    // *  exchange  a first set of input data have been received. That means,
    // *  actual input data are valid.
    // *
    // *  Input:    DevHndl        Device handle
    // *            ArNum          AR number (1...N)
    // *            SessionKey     0  (not used here)
    // *  Output:   ---
    // *
    // *----------------------------------------------------------------*
    void PNIO_cbf_ar_indata_ind (PNIO_UINT32 DevHndl,
                                 PNIO_UINT16 ArNum,
                                 PNIO_UINT16 SessionKey)
    {
        PNIOext_cbf_ar_indata_ind(ArNum, SessionKey);
        PNIO_UNUSED_ARG (DevHndl);
    }



    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_ar_disconn_ind ()
    // *
    // *----------------------------------------------------------------*
    // *
    // *  The Device is offline. Reason code is provided by the context
    // *  manager of the pnio stack.
    // *
    // *  Input:    DevHndl        Device handle
    // *            ArNum          AR number (1...N)
    // *            SessionKey     0  (not used here)
    // *            ReasonCode     reason code (see PNIO_AR_REASON)
    // *  Output:   ---
    // *
    // *----------------------------------------------------------------*
    void PNIO_cbf_ar_disconn_ind     (PNIO_UINT32    DevHndl,
                                      PNIO_UINT16    ArNum,
                                      PNIO_UINT16    SessionKey,
                                      PNIO_AR_REASON ReasonCode)
    {
        PNIO_UNUSED_ARG (DevHndl);

        if (NumOfAr)
            NumOfAr--;      // decrement number of running ARs
        PNIOext_cbf_ar_disconn_ind(ArNum,  SessionKey,  ReasonCode);
        // if (NumOfAr == 0)
        //    done by stack      Bsp_EbSetLed (EB_LED_BUS_FAULT,  1);   // no more ARs running, set red LED,
    }



    // *****************************************************************************************
    // *************************** ALARM INDICATIONS  ******************************************
    // *****************************************************************************************
    void    PNIO_cbf_dev_alarm_ind (PNIO_UINT32         DevHndl,
                                    PNIO_DEV_ALARM_DATA *pAlarm)
    {
    }


    // *****************************************************************************************
    // ************************  Request callback function  ************************
    // *****************************************************************************************
    void PNIO_cbf_async_req_done   (PNIO_UINT32     DevHndl,       // device handle
                                    PNIO_UINT32     ArNum,         // AR number (1...N)
                                    PNIO_ALARM_TYPE AlarmType,     // alarm type
                                    PNIO_UINT32     Api,           // API number
                                    PNIO_DEV_ADDR   *pAddr,        // location (slot, subslot)
                                    PNIO_UINT32     Status,        // status
                                    PNIO_UINT16     Diag_tag)
    {
       PNIO_UNUSED_ARG (DevHndl);
       PNIOext_cbf_async_req_done(ArNum, AlarmType, Api, pAddr, Status, Diag_tag);
    }

    // *****************************************************************************************
    // *************************** RECORD INDICATIONS  *****************************************
    // *****************************************************************************************


    // *----------------------------------------------------------------*
    // *
    // *  PnUsr_cbf_rec_read
    // *
    // *----------------------------------------------------------------*
    // *
    // *----------------------------------------------------------------*
    PNIO_UINT32  PnUsr_cbf_rec_read
    (
        PNIO_UINT32         DevHndl,        // device handle
        PNIO_UINT32         Api,            // application process identifier
        PNIO_UINT16         ArNum,          // ar - number
        PNIO_UINT16         SessionKey,     // ar session number
        PNIO_UINT32         SequenceNum,    // CLRPC sequence number
        PNIO_DEV_ADDR       *pAddr,         // geographical or logical address
        PNIO_UINT32         RecordIndex,    // record index
        PNIO_UINT32         *pBufLen,       // [in, out] in: length to read, out: length, read by user
        PNIO_UINT8          *pBuffer,       // [in] buffer pointer
        PNIO_ERR_STAT       *pPnioState     // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
    )
    {
        PNIO_UNUSED_ARG (DevHndl);
        /* Send all the data to BBB */
        PNIOext_cbf_rec_read(Api, ArNum, SessionKey, SequenceNum,
                            pAddr, RecordIndex, pBufLen, pBuffer, pPnioState);

        return PNIO_OK;
    }



    // *----------------------------------------------------------------*
    // *
    // *  PnUsr_cbf_rec_write ()
    // *
    // *----------------------------------------------------------------*
    // *
    // *----------------------------------------------------------------*
    PNIO_UINT32  PnUsr_cbf_rec_write
    (
        PNIO_UINT32         DevHndl,        // device handle
        PNIO_UINT32         Api,            // application process identifier
        PNIO_UINT16         ArNum,          // ar - number
        PNIO_UINT16         SessionKey,     // ar session number
        PNIO_UINT32         SequenceNum,    // CLRPC sequence number
        PNIO_DEV_ADDR       *pAddr,         // geographical or logical address
        PNIO_UINT32         RecordIndex,    // record index
        PNIO_UINT32         *pBufLen,       // [in, out] in: length to write, out: length, written by user
        PNIO_UINT8          *pBuffer,       // [in] buffer pointer
        PNIO_ERR_STAT       *pPnioState     // 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
    )

    {
        PNIO_UNUSED_ARG (DevHndl);

        /* Send all the data to BBB */
        PNIOext_cbf_rec_write(Api, ArNum, SessionKey, SequenceNum,
                            pAddr, RecordIndex, pBufLen, pBuffer, pPnioState);

        return PNIO_OK;
    }


    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_save_station_name ()
    // *
    // *----------------------------------------------------------------*
    // *
    // *  The DCP package of the pnio stack has got a "set station name"
    // *  request from the pnio controller and notifies the user by this
    // *  function.
    // *  The user has to save this name in a non volatile memory (for example
    // *  a nv-ram), if the Remanent-parameter is <> 0.
    // *
    // *  manager of the pnio stack.
    // *
    // *  Input:    PNIO_UINT8      *pStationName   name string,
    // *            PNIO_UINT16     NameLength     length of the name string
    // *            PNIO_UINT8      Remanent        <> 0: save remanent
    // *
    // *
    // *  Output:   return          PNIO_OK: function completed successfully
    // *                            PNIO_NOT_OK: function could not be completed
    // *----------------------------------------------------------------*
    PNIO_UINT32  PNIO_cbf_save_station_name
                        (   PNIO_INT8    *pStationName,
                            PNIO_UINT16   NameLength,
                            PNIO_UINT8   Remanent )
    {
       if (Remanent)
       {
          // **** store data in non volatile memory ****
          Bsp_nv_data_store (PNIO_NVDATA_DEVICENAME,    // nv data type: device name
                             pStationName,              // source pointer to the devicename
                             NameLength);               // length of the device name
       }
       else
       {
           Bsp_nv_data_store (PNIO_NVDATA_DEVICENAME,    // nv data type: device name
                              "",                        // source pointer to the devicename
                              0);               // length of the device name
       }

       /* Call callback in BBB */
       PNIOext_cbf_save_station_name(NameLength, pStationName, Remanent);

       // *-----------------------------------------------------------------*
       // * return PNIO_OK:     name modification is accepted by application
       // *        PNIO_NOT_OK: name modification is denied   by application
       // *-----------------------------------------------------------------*
       return (PNIO_OK);
    }



    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_save_ip_addr()
    // *
    // *----------------------------------------------------------------*
    // *
    // *  The DCP package of the pnio stack has got a "set ip address"
    // *  request from the pnio controller and notifies the user by this
    // *  function.
    // *  The user has to save the ip address, subnet mask and default router
    // *  address in a non volatile memory (for example a nv-ram), if the
    // *  Remanent-parameter is <> 0.
    // *
    // *  manager of the pnio stack.
    // *
    // *  Input:    PNIO_UINT32      NewIpAddr      new ip addres
    // *            PNIO_UINT32      SubnetMask     Subnet mask
    // *            PNIO_UINT32      DefRouterAddr  default gateway
    // *            PNIO_UINT8       Remanent       <> 0: save remanent,
    // *                                            == 0: do not save remanent
    // *
    // *
    // *  Output:   return          PNIO_OK: function completed successfully
    // *                            PNIO_NOT_OK: function could not be completed
    // *
    // *----------------------------------------------------------------*
    PNIO_UINT32 PNIO_cbf_save_ip_addr
                                 (  PNIO_UINT32 NewIpAddr,
                                    PNIO_UINT32 SubnetMask,
                                    PNIO_UINT32 DefRouterAddr,
                                    PNIO_UINT8  Remanent)
    {
        NV_IP_SUITE* pIpSuite;

        OsAllocF((void**)&pIpSuite, sizeof(NV_IP_SUITE));

        if (Remanent)
        {
            // **** save ip suite in non volatile memory ****
            pIpSuite->IpAddr     = NewIpAddr;
            pIpSuite->SubnetMask = SubnetMask;
            pIpSuite->DefRouter  = DefRouterAddr;
            Bsp_nv_data_store(PNIO_NVDATA_IPSUITE,      // nv data type: ip suite
                              pIpSuite,                 // source pointer
                              sizeof(NV_IP_SUITE));     // length of the device name

        }
        else
        {
            // *-----------------------------------------------------
            // *  according to the PNIO SPEC an already (remanent)
            // *  stored IP address must be deleted, if a new ip address
            // *  is set non remanent.
            // *-----------------------------------------------------
            pIpSuite->IpAddr     = 0;
            pIpSuite->SubnetMask = 0;
            pIpSuite->DefRouter  = 0;
            Bsp_nv_data_store(PNIO_NVDATA_IPSUITE,      // nv data type: ip suite
                              pIpSuite,                 // source pointer
                              sizeof(NV_IP_SUITE));     // length of the device name

        }

        /* Call callback in BBB  */
        NewIpAddr = OsHtonl(NewIpAddr);
        SubnetMask = OsHtonl(SubnetMask);
        DefRouterAddr = OsHtonl(DefRouterAddr);
        PNIOext_cbf_save_ip_address(NewIpAddr, SubnetMask, DefRouterAddr, Remanent);

        OsFree(pIpSuite);

        return (PNIO_OK);

    }

    /**
     *  @brief there is new ip data
     *
     *
     *  information about new ip, mask and gateway
     *  only printed to console to show the data
     *
     */
    PNIO_UINT32 PNIO_cbf_report_new_ip_addr( PNIO_UINT32 NewIpAddr,
                                             PNIO_UINT32 SubnetMask,
                                             PNIO_UINT32 DefRouterAddr)
    {
        /* Call callback in BBB */
        PNIOext_cbf_report_new_ip_address(NewIpAddr, SubnetMask, DefRouterAddr);

        return (PNIO_OK);
    }

    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_start_led_blink ()
    // *
    // *----------------------------------------------------------------*
    // *
    // *  Start blinking with LED after DCP Request (received from the
    // *  engineering tool
    // *  function.
    // *  The user has to start blinking with the specified LED
    // *
    // *  Input:        PNIO_UINT32    DevHndl     Device handle
    // *                PNIO_UINT32    PortNum     port number  1..N
    // *                PNIO_UINT32    frequency   blinking frequency
    // *  Output:       return         must be PNIO_OK
    // *----------------------------------------------------------------*
    PNIO_UINT32 PNIO_cbf_start_led_blink (PNIO_UINT32 DevHndl,
                                          PNIO_UINT32 PortNum,
                                          PNIO_UINT32 frequency)
    {
        PNIO_UNUSED_ARG (DevHndl);
        BspLed_StartLedBlink(frequency);
        return (PNIO_OK);    // must be PNIO_OK, other's not possible
    }



    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_stop_led_blink ()
    // *
    // *----------------------------------------------------------------*
    // *  Stop blinking with LED (started after DCP Request)
    // *
    // *  Input:        PNIO_UINT32    DevHndl     Device handle
    // *                PNIO_UINT32    PortNum     port number  1..N
    // *  Output:       return         must be PNIO_OK
    // *----------------------------------------------------------------*
    PNIO_UINT32 PNIO_cbf_stop_led_blink (PNIO_UINT32 DevHndl,
                                         PNIO_UINT32 PortNum)
    {
        PNIO_UNUSED_ARG (DevHndl);
        BspLed_StopLedBlink ();
        return (PNIO_OK);    // must be PNIO_OK, other's not possible
    }


    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_reset_factory_settings ()
    // *
    // *----------------------------------------------------------------*
    // *  advises the application, to reset to factory settings
    // *
    // *  Input:    PNIO_UINT32     DevHndl     Device handle
    // *  Input:    PNIO_RTF_OPTION RtfOption   specifies, which data have
    // *                                        to be resetted
    // *  Output:   return          must be PNIO_OK
    // *----------------------------------------------------------------*
    PNIO_UINT32 PNIO_cbf_reset_factory_settings (PNIO_UINT32 DevHndl, PNIO_RTF_OPTION RtfOption)
    {
        PNIO_UNUSED_ARG (DevHndl);

        Bsp_nv_data_clear (RtfOption);

        return (PNIO_OK);    // must be PNIO_OK, other's not possible

    }


    #if IOD_INCLUDE_REC8028_8029
        // **** special case:   read record index 0x8029  (read output data)
        PNIO_IOXS    PNIO_cbf_substval_out_read  // read substitution values for output submodule
                (
                 PNIO_UINT32    DevHndl,            // [in] Handle for Multidevice
                 PNIO_DEV_ADDR  *pAddr,             // [in] geographical or logical address
                 PNIO_UINT32    BufLen,             // [in] length of the submodule output substitute data
                 PNIO_UINT8     *pBuffer,           // [in] Ptr to submodule output substitute data
                 PNIO_UINT16*   pSubstMode,         // [in, out - BIG ENDIAN] SubstitutionMode: 0=ZERO or inactive (default), 1:LastValue, 2:Replacement value SubstitutionMode: 0=ZERO or inactive, 1:LastValue, 2:Replacement value
                 PNIO_UINT16*   pSubstActive        // [in, out - BIG ENDIAN] SubstituteActiveFlag:  0=operation, 1=substitute. default value is 0: if (IOPS & IOCS = GOOD), else: 1
                )
        {
        	PNIO_UINT32 return_value;
        	/* Call BBB */
        	return_value = PNIOext_cbf_substval_out_read(pAddr, BufLen, pBuffer, pSubstMode, pSubstActive);

        	//if the output data set is valid, then return good. else, return bad.
        	if(return_value == PNIO_OK){
        		return (PNIO_S_GOOD);
        	}
            else
            {
        		return (PNIO_S_BAD);
        	}
        }
    #endif

    /**
     * @brief User functionality as reaction to new ARFSU
     *
     * @param[in]         ARFSU_enabled             PNIO_ARFSU_ENABLED, PNIO_ARFSU_DISABLED
     * @param[in]         ARFSU_changed             PNIO_ARFSU_CHANGED, PNIO_ARFSU_NOT_CHANGED
     *
     * @return            void
     *
     *  This function is called when ARFSU write record is recieved
     *  Function is ready for user functionality
     *  Informs user if ARFSU UUID was changed or not -> if parameterization was changed
     *
     */
    PNIO_VOID PNIO_cbf_report_ARFSU_record(
            PNIO_UINT8          ARFSU_enabled,
            PNIO_UINT8          ARFSU_changed
            )
    {
        PNIOext_cbf_report_ARFSU_record(ARFSU_enabled, ARFSU_changed);
        if( PNIO_ARFSU_CHANGED == ARFSU_changed )
        {
            //PNIO_printf( "New ARFSU UUID -> parameterization of device was changed\n" );
        }
    }

    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_new_plug_ind ()
    // *
    // *----------------------------------------------------------------*
    // *
    // *  The Plug Indication of a module
    // *
    // *    Input:	   PNIO_DEV_ADDR      *pAddr	        Module address
    // *               PNIO_UINT32         InputDataLen	    submodule input data length
    // *               PNIO_UINT32         OutputDataLen	submodule output data length
    // *    Output:	   ---
    // *
    // *
    // *----------------------------------------------------------------*
    PNIO_VOID PNIO_cbf_new_plug_ind
    (
        PNIO_DEV_ADDR       *pAddr,
        PNIO_UINT32          InputDataLen,
        PNIO_UINT32          OutputDataLen
    )
    {
        InDatLen[pAddr->ad.Geo1.Slot][pAddr->ad.Geo1.Subslot] = InputDataLen;
        OutDatLen[pAddr->ad.Geo1.Slot][pAddr->ad.Geo1.Subslot] = OutputDataLen;
    }

    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_new_pull_ind ()
    // *
    // *----------------------------------------------------------------*
    // *
    // *  The Pull Indication of a module
    // *
    // *    Input:	   PNIO_DEV_ADDR      *pAddr	Module address
    // *    Output:	   ---
    // *
    // *
    // *----------------------------------------------------------------*
    PNIO_VOID PNIO_cbf_new_pull_ind
    (
        PNIO_DEV_ADDR       *pAddr         // [in] slot/subslot number
    )
    {
        InDatLen[pAddr->ad.Geo1.Slot][pAddr->ad.Geo1.Subslot] = 0;
        OutDatLen[pAddr->ad.Geo1.Slot][pAddr->ad.Geo1.Subslot] = 0;
    }

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
