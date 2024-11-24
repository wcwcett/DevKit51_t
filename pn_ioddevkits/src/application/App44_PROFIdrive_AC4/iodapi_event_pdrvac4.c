/*****************************************************************************/
/*  Copyright (C) 2020 Siemens Aktiengesellschaft. All rights reserved.      */
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
/*  F i l e               &F: iodapi_event_isoapp.c                     :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P04.07.01.01_00.01.00.18     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2020-11-09                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  PNIO event handler for ischochronous application example                 */
/*                                                                           */
/*  performs user functions, called by stack in case of events and alarms    */
/*                                                                           */
/*  To use this application example, set #define EXAMPL_DEV_CONFIG_VERSION 3 */
/*  in file \application\usriod_app.h                                        */
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
#include "iod_cfg.h"
#include "usriod_cfg.h"
#include "usrapp_cfg.h"   // example application spec. configuration settings

#if (EXAMPL_DEV_CONFIG_VERSION == 44)
    #include "pniousrd.h"
    #include "iodapi_event.h"
    #include "os.h"
    #include "usriod_im_func.h"
    #include "nv_data.h"
    #include "bspadapt.h"
    #include "PnUsr_Api.h"
    #include "usriod_PE.h"
    #include "usriod_utils.h"

	#include "pdrv_types_ac4.h"
	#include "pdrv_statemachine_ac4.h"
	#include "pdrv_application_ac4.h"
	#include "pdrv_parmanager_ac4.h"
	#include "pdrv_pardatabase_ac4.h"
	#include "pdrv_setpointchannel_ac4.h"

    // *=======================================================
    // *  defines
    // *=======================================================
    #define EXAMPLE_SUBSTITUTE_VALUE    0xab  // example: substituted value for output (data consumer)
    #define EXAMPLE_IN_VALUE		    0x11  // example: default input value (data provider)

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

	static PDRV_UINT16 m_uPDRVSourceTgmSel = 0U;        /**< value of PNU00300 source of telegram selection */
	static PDRV_UINT16 m_uPDRVTelegram = 3U;            /**< value of PNU00922 telegram selection, see PDRV V4.2 table 97 */
	static PDRV_UINT16 m_uPDRVIsoCacf = 0U;             /**< Controller Application Cycle Factor */
	static PDRV_BOOL m_bPDRVIsIsoActiv = PDRV_FALSE;    /**< isochronous mode is activated */
	static PDRV_BOOL m_bPDRVIsIsoReq = PDRV_TRUE;      /**< isochronous mode is required */

	volatile PDRV_BOOL m_bPDRVIsDisconnect = PDRV_FALSE;    /**< ar disconnect flag */

    #if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
        // **** isochronous application *****
        static PNIO_UINT32          IsoGpiosActivated      = PNIO_FALSE;
        static PNIO_ISO_OBJ_HNDL    IsoGpioTiHndl          = 0;
        static PNIO_ISO_OBJ_HNDL    IsoGpioToHndl          = 0;
        static PNIO_ISO_OBJ_HNDL    IsoPdrvTiHndl          = 0;
    #endif
    static PNIO_UINT32          IsoArNum               = 0;

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




    // *** Output Data (IO Controller ==> IO Device) and substituted values for output data ***
    PNIO_UINT8     OutData		  [NUMOF_SLOTS][NUMOF_SUBSLOTS+1][NUMOF_BYTES_PER_SUBSLOT];
#if !IOD_INCLUDE_REC8028_8029
    PNIO_UINT8     OutSubstVal	  [NUMOF_SLOTS][NUMOF_SUBSLOTS+1][NUMOF_BYTES_PER_SUBSLOT];
#endif
    PNIO_UINT16    OutDatLen	  [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];
    PNIO_UINT8     OutDatIocs	  [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];
    PNIO_UINT8     OutDatIops	  [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];
    PNIO_UINT8     OutDatIops_old [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];

    // *** Input Data  (IO Device ==> IO Controller)
    PNIO_UINT8     InData		  [NUMOF_SLOTS][NUMOF_SUBSLOTS+1][NUMOF_BYTES_PER_SUBSLOT];
    PNIO_UINT16    InDatLen		  [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];
    PNIO_UINT8     InDatIops	  [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];
    PNIO_UINT8     InDatIocs	  [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];
    PNIO_UINT8     InDatIocs_old  [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];

    typedef struct
    {
        PNIO_AR_TYPE    ArType;             // [in] type of AR (see cm_ar_type_enum)
        PNIO_UINT32     ArNum;
    } PNIO_AR_INFO;

    PNIO_AR_INFO Ar_Connection_Info[10];

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
    		case 0x0001:	return 1;
    		case 0x8000:	return 2;
    		case 0x8001:	return 3;
    		case 0x8002:	return 4;
    		case 0x8003:	return 5;
    		case 0x8004:	return 6;
    		default: 		return 0;
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



    // *----------------------------------------------------------------*
    // *
    // *  ChangeAllInputData (DiffVal)
    // *
    // *  prints all input and output data, which are used by an
    // *  application relation with data length <> 0
    // *
    // *----------------------------------------------------------------*
    void ChangeAllInputData (PNIO_INT8 DiffVal)
    {
	    PNIO_INT32 SlotNum;// loop var for slot-number
	    PNIO_INT32 SubNum;	// loop var for subslot-number
	    PNIO_INT32 IoInd;	// index of an io-data byte in a subslot

	    // *---------------------------------------------------------------
	    // *  print input data
	    // *---------------------------------------------------------------
	    for (SlotNum = 0; SlotNum < NUMOF_SLOTS; SlotNum++)			// loop over all slots
	    {
		    for (SubNum = 0; SubNum < NUMOF_SUBSLOTS; SubNum++)	// loop over all subslots
		    {
		      if (InDatLen[SlotNum][SubNum] != 0)		// data used by io ar == data_len is <> 0??
		      {
			    for (IoInd=0; IoInd < InDatLen[SlotNum][SubNum]; IoInd++)
			    {
				    InData[SlotNum][SubNum][IoInd] += DiffVal;
			    }
		      }
            }
	    }
    }


    // *----------------------------------------------------------------*
    // *
    // *  PrintAllUsedIoData ()
    // *
    // *  prints all input and output data, which are used by an
    // *  application relation with data length <> 0
    // *
    // *----------------------------------------------------------------*
    void PrintAllUsedIoData (void)
    {
	    PNIO_INT32 SlotNum;// loop var for slot-number
	    PNIO_INT32 SubNum;	// loop var for subslot-number
	    PNIO_INT32 IoInd;	// index of an io-data byte in a subslot

	    PNIO_INT32 DataAvailable = 0;

	    // *---------------------------------------------------------------
	    // *  print input data
	    // *---------------------------------------------------------------
	    for (SlotNum = 0; SlotNum < NUMOF_SLOTS; SlotNum++)			// loop over all slots
	    {
		    for (SubNum = 0; SubNum < NUMOF_SUBSLOTS; SubNum++)	// loop over all subslots
		    {
		      if (InDatLen[SlotNum][SubNum] != 0)		// data used by io ar == data_len is <> 0??
		      {
			    DataAvailable = 1;
			    PNIO_printf ( (PNIO_CHAR*) "##DatIn Slot=%d  Sub=%d length=%d : ",
				        SlotNum,					// slot num
					    SubNum,						// subslot num
					    InDatLen[SlotNum][SubNum]);	// data length, defined in step 7 hw-config

			    for (IoInd=0; IoInd < InDatLen[SlotNum][SubNum]; IoInd++)
			    {
				    PNIO_printf ( (PNIO_CHAR*) "0x%02x ", InData[SlotNum][SubNum][IoInd]);
			    }
			    PNIO_printf ( (PNIO_CHAR*) "\n");
		      }
		    }
	    }

	    // *---------------------------------------------------------------
	    // *  print output data
	    // *---------------------------------------------------------------
	    for (SlotNum = 0; SlotNum < NUMOF_SLOTS; SlotNum++)			// loop over all slots
	    {
		    for (SubNum = 0; SubNum < NUMOF_SUBSLOTS; SubNum++)	// loop over all subslots
		    {
		      if (OutDatLen[SlotNum][SubNum] != 0)		// data used by io ar == data_len is <> 0??
		      {
			    DataAvailable = 1;
			    PNIO_printf ( (PNIO_CHAR*) "##DatOut Slot=%d  Sub=%d length=%d : ",
				        SlotNum,					// slot num
					    SubNum,						// subslot num
					    OutDatLen[SlotNum][SubNum]);// data length, defined in step 7 hw-config

			    for (IoInd=0; IoInd < OutDatLen[SlotNum][SubNum]; IoInd++)
			    {
				    PNIO_printf ( (PNIO_CHAR*) "0x%02X ", OutData[SlotNum][SubNum][IoInd]);
			    }
			    PNIO_printf ( (PNIO_CHAR*) "\n");
		      }
		    }
	    }

	    if (DataAvailable == 0)
		    PNIO_printf ( (PNIO_CHAR*) "no io data available in any active IOCR\n");

    }

    // *----------------------------------------------------------------*
    // *
    // *  UsrSetIops (slot_num, subslot_num, Iops
    // *----------------------------------------------------------------*
    // *
    // *  sets the IO provider state for the specified slot/subslot
    // *
    // *  Input:	   PNIO_UINT32     slot_num,	// [in]  slot number of module
    // *			   PNIO_UINT32     subslot_num,	// [in]  subslot number of submodule
    // *			   PNIO_UINT8      Iops			// [in]  io provider state
    // *
    // *	Output:	   ---
    // *
    // *
    // *----------------------------------------------------------------*
    void UsrSetIops (PNIO_UINT32  SlotNum, PNIO_UINT32  SubslotNum, PNIO_UINT8    Iops)
    {
        InDatIops [SlotNum][SubslotNum] = Iops;
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
    // *  Input:	   PNIO_UINT32    DevHndl,		// [in]  Device Handle
    // *			   PNIO_DEV_ADDR  *pAddr,		// [in]  location (slot, subslot)
    // *			   PNIO_UINT32    data_len,	    // [in] data length
    // *			   PNIO_UINT8     *pBuffer      // [in,out] pointer to the input data
    // *			   PNIO_UINT8     Iocs			// [in]  (io controller) consumer status
    // *
    // *	Output:	   return   io-provider-status  // PNIO_S_GOOD, PNIO_S_BAD
    // *
    // *
    // *----------------------------------------------------------------*
    OS_CODE_FAST PNIO_IOXS    PNIO_cbf_data_write
		    (PNIO_UINT32	DevHndl,		// [in]  device handle
		     PNIO_DEV_ADDR	*pAddr,			// geographical or logical address
		     PNIO_UINT32 	BufLen,			// [in]	 length of the submodule input data
		     PNIO_UINT8     *pBuffer, 		// [in,out] Ptr to data buffer to write to
		     PNIO_IOXS	    Iocs            // [in]  remote (io controller) consumer status
            )
	{
		PNIO_UINT32 slot_num	= pAddr->Geo.Slot;
		PNIO_UINT32 subslot_num = pAddr->Geo.Subslot;
		PNIO_UINT8 *puInData = InData[slot_num][subslot_num];   /* InData pointer */
		PDRV_UINT16 uTelegramNo = m_uPDRVTelegram;

		PNIO_UNUSED_ARG (DevHndl);
		/* PDRV: Slot used for PROFIdrive standard telegrams? */
		if (slot_num == PDRV_STDTLG_SLOT)
		{
			/* Is PROFIdrive standard telegram 1 addressed? */
			if (   (uTelegramNo == PDRV_STDTLG1)
				&& (subslot_num == PDRV_STDTLG_SUBSLOT)
				&& (BufLen == PDRV_STDTLG1_INLEN)
			   )
			{
				PDRV_UINT16 uZsw1 = uPdrvSpc_GetZsw1();                    /* ZSW1 status word 1 */
				PDRV_UINT16 uNistA = (PDRV_UINT16) nPdrvSpc_GetNistA();    /* NSOLL_A speed actual value */

				/* create BigEndian data */
				puInData[0] = (PNIO_UINT8) (uZsw1 >> 8);
				puInData[1] = (PNIO_UINT8) uZsw1;
				puInData[2] = (PNIO_UINT8) (uNistA >> 8);
				puInData[3] = (PNIO_UINT8) uNistA;
			}    /* Is PROFIdrive standard telegram 2 addressed? */
			else if (   (uTelegramNo == PDRV_STDTLG2)
					 && (subslot_num == PDRV_STDTLG_SUBSLOT)
					 && (BufLen == PDRV_STDTLG2_INLEN)
					)
			{
				PDRV_UINT16 uZsw1 = uPdrvSpc_GetZsw1();                    /* ZSW1 status word 1 */
				PDRV_UINT16 uZsw2 = uPdrvSpc_GetZsw2();                    /* ZSW2 status word 2 */
				PDRV_UINT32 uNistB = (PDRV_UINT32) nPdrvSpc_GetNistB();    /* NSOLL_B speed actual value  */

				/* create BigEndian data */
				puInData[0] = (PNIO_UINT8) (uZsw1 >> 8U);
				puInData[1] = (PNIO_UINT8) uZsw1;
				puInData[2] = (PNIO_UINT8) (uNistB >> 24U) & 0xFFU;
				puInData[3] = (PNIO_UINT8) (uNistB >> 16U) & 0xFFU;;
				puInData[4] = (PNIO_UINT8) (uNistB >> 8U) & 0xFFU;
				puInData[5] = (PNIO_UINT8) uNistB;
				puInData[6] = (PNIO_UINT8) (uZsw2 >> 8U);
				puInData[7] = (PNIO_UINT8) uZsw2;
			}    /* Is PROFIdrive standard telegram 3 addressed? */
			else if (   (uTelegramNo == PDRV_STDTLG3)
					 && (subslot_num == PDRV_STDTLG_SUBSLOT)
					 && (BufLen == PDRV_STDTLG3_INLEN)
					)
			{
				PDRV_UINT16 uZsw1 = uPdrvSpc_GetZsw1();                    /* ZSW1 status word 1 */
				PDRV_UINT16 uZsw2 = uPdrvSpc_GetZsw2();                    /* ZSW2 status word 2 */
				PDRV_UINT32 uNistB = (PDRV_UINT32) nPdrvSpc_GetNistB();    /* NSOLL_B speed actual value  */
				PDRV_UINT16 uG1Zsw = uPdrvSpc_GetG1Zsw();                  /* G1_ZSW sensor 1 status word */
				PDRV_UINT32 uG1Xist1 = uPdrvSpc_GetG1Xist1();              /* G1_XIST1 sensor 1 position actual value 1 */
				PDRV_UINT32 uG1Xist2 = uPdrvSpc_GetG1Xist2();              /* G1_XIST2 sensor 1 position actual value 2 */

				/* create BigEndian data */
				puInData[0] = (PNIO_UINT8) (uZsw1 >> 8U);
				puInData[1] = (PNIO_UINT8) uZsw1;
				puInData[2] = (PNIO_UINT8) (uNistB >> 24U) & 0xFFU;
				puInData[3] = (PNIO_UINT8) (uNistB >> 16U) & 0xFFU;;
				puInData[4] = (PNIO_UINT8) (uNistB >> 8U) & 0xFFU;
				puInData[5] = (PNIO_UINT8) uNistB;
				puInData[6] = (PNIO_UINT8) (uZsw2 >> 8U);
				puInData[7] = (PNIO_UINT8) uZsw2;
				puInData[8] = (PNIO_UINT8) (uG1Zsw >> 8U);;
				puInData[9] = (PNIO_UINT8) uG1Zsw;
				puInData[10] = (PNIO_UINT8) (uG1Xist1 >> 24U) & 0xFFU;
				puInData[11] = (PNIO_UINT8) (uG1Xist1 >> 16U) & 0xFFU;
				puInData[12] = (PNIO_UINT8) (uG1Xist1 >>  8U) & 0xFFU;
				puInData[13] = (PNIO_UINT8) uG1Xist1;
				puInData[14] = (PNIO_UINT8) (uG1Xist2 >> 24U) & 0xFFU;
				puInData[15] = (PNIO_UINT8) (uG1Xist2 >> 16U) & 0xFFU;
				puInData[16] = (PNIO_UINT8) (uG1Xist2 >>  8U) & 0xFFU;
				puInData[17] = (PNIO_UINT8) uG1Xist2;
			}
		}

		/* Perform memcpy only if there are data to be copied */
		if((0 < BufLen) && (NULL != pBuffer))
		{
			OsMemCpy (pBuffer, &InData[slot_num][subslot_num][0], BufLen);
		}
		InDatIocs [slot_num][subslot_num] = Iocs;    // consumer status (of remote io controller)

		// *---- check provider status and notify modifications ------*
		if (InDatIocs_old [slot_num][subslot_num] != Iocs)
		{
			 PNIO_printf ( (PNIO_CHAR*) "new IO controller input consumer status (ICS) = 0x%x in slot %d, subslot %d\n",
										Iocs, slot_num, subslot_num);
			 InDatIocs_old [slot_num][subslot_num] = Iocs;
		}
		return ( (PNIO_IOXS)InDatIops [slot_num][subslot_num]);	// return local provider state
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
    // *			   PNIO_UINT32    DevHndl       // [in] device handle
    // *			   PNIO_DEV_ADDR  *pAddr,		// [in]  location (slot, subslot)
    // *			   PNIO_UINT32    data_len,	    // [in] data length
    // *			   PNIO_UINT8*    pBuffer       // [in] pointer to the input data
    // *			   PNIO_UINT8     IoPs			// [in]  (io controller) provider status
    // *
    // *	Output:	   return   io-consumer-status  // IOCS_STATE_GOOD, IOCS_STATE_BAD
    // *
    // *
    // *----------------------------------------------------------------*
    OS_CODE_FAST PNIO_IOXS     PNIO_cbf_data_read
		    (PNIO_UINT32	DevHndl,		// [in]  device handle
		     PNIO_DEV_ADDR	*pAddr,			// [in]  geographical or logical address
		     PNIO_UINT32 	BufLen,			// [in]  length of the submodule input data
		     PNIO_UINT8*    pBuffer, 		// [in]  Ptr to data buffer to read from
		     PNIO_IOXS	    Iops            // [in]  (io controller) provider status
            )
	{
		PNIO_UINT32 slot_num	= pAddr->Geo.Slot;
		PNIO_UINT32 subslot_num = pAddr->Geo.Subslot;
		PNIO_UINT8 *puOutData;  /* OutData pointer */
		PDRV_UINT16 uTelegramNo = m_uPDRVTelegram;

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
			PNIO_printf ( (PNIO_CHAR*) "new IO controller output provider status (OPS) = 0x%x in slot %d, subslot %d\n",
									  Iops, slot_num, subslot_num);
			OutDatIops_old [slot_num][subslot_num] = Iops;

			(Iops == PNIO_S_GOOD)?(m_bPDRVIsDisconnect = PDRV_FALSE):(m_bPDRVIsDisconnect = PDRV_TRUE);
		}

		/* PDRV: Slot used for PROFIdrive standard telegrams? */
		#if !IOD_INCLUDE_REC8028_8029
			/* valid data then actual datas else substitute datas */
			puOutData = (Iops == PNIO_S_GOOD)? OutData[slot_num][subslot_num]: OutSubstVal[slot_num][subslot_num];
		#else
			puOutData = OutData[slot_num][subslot_num];
		#endif
		if (slot_num == PDRV_STDTLG_SLOT)
		{
			/* PDRV: Is PROFIdrive standard telegram 1 addressed? */
			if (   (uTelegramNo == PDRV_STDTLG1)
				&& (subslot_num == PDRV_STDTLG_SUBSLOT)
				&& (BufLen == PDRV_STDTLG1_OUTLEN)
			   )
			{
				PDRV_UINT16 uStw1;      /* STW1 control word 1 */
				PDRV_UINT16 uNsollA;    /* NSOLL_A speed setpoint value 1 */

				/* get data from PROFIdrive data with BigEndian OutData */
				uStw1 =    ((PDRV_UINT16)puOutData[0] << 8) + (PDRV_UINT16)puOutData[1];
				uNsollA = ((PDRV_UINT16)puOutData[2] << 8) + (PDRV_UINT16)puOutData[3];
				PdrvSpc_SetStw1(uStw1);
				PdrvSpc_SetNsollA((PDRV_INT16)uNsollA);
			}    /* Is PROFIdrive standard telegram 2 addressed? */
			else if (   (uTelegramNo == PDRV_STDTLG2)
					 && (subslot_num == PDRV_STDTLG_SUBSLOT)
					 && (BufLen == PDRV_STDTLG2_OUTLEN)
					)
			{
				PDRV_UINT16 uStw1;      /* STW1 control word 1 */
				PDRV_UINT16 uStw2;      /* STW2 control word 2 */
				PDRV_UINT32 uNsollB;    /* NSOLL_B speed setpoint value */

				/* get data from PROFIdrive data with BigEndian OutData */
				uStw1 =     ((PDRV_UINT16)puOutData[0] <<  8U)
						  +  (PDRV_UINT16)puOutData[1];
				uNsollB =   ((PDRV_UINT32)puOutData[2] << 24U)
						  + ((PDRV_UINT32)puOutData[3] << 16U)
						  + ((PDRV_UINT32)puOutData[4] <<  8U)
						  +  (PDRV_UINT32)puOutData[5];
				uStw2 =     ((PDRV_UINT16)puOutData[6] <<  8U)
						  +  (PDRV_UINT16)puOutData[7];
				PdrvSpc_SetStw1(uStw1);
				PdrvSpc_SetNsollB((PDRV_INT32)uNsollB);
				PdrvSpc_SetStw2(uStw2);
			}    /* Is PROFIdrive standard telegram 3 addressed? */
			else if (   (uTelegramNo == PDRV_STDTLG3)
					 && (subslot_num == PDRV_STDTLG_SUBSLOT)
					 && (BufLen == PDRV_STDTLG3_OUTLEN)
					)
			{
				PDRV_UINT16 uStw1;      /* STW1 control word 1 */
				PDRV_UINT16 uStw2;      /* STW2 control word 2 */
				PDRV_UINT32 uNsollB;    /* NSOLL_B speed setpoint value */
				PDRV_UINT16 uG1Stw;     /* G1_STW sensor 1 control word */

				/* get data from PROFIdrive data with BigEndian OutData */
				uStw1 =     ((PDRV_UINT16)puOutData[0] <<  8U)
						  +  (PDRV_UINT16)puOutData[1];
				uNsollB =   ((PDRV_UINT32)puOutData[2] << 24U)
						  + ((PDRV_UINT32)puOutData[3] << 16U)
						  + ((PDRV_UINT32)puOutData[4] <<  8U)
						  +  (PDRV_UINT32)puOutData[5];
				uStw2 =     ((PDRV_UINT16)puOutData[6] <<  8U)
						  +  (PDRV_UINT16)puOutData[7];
				uG1Stw =    ((PDRV_UINT16)puOutData[8] <<  8U)
						  +  (PDRV_UINT16)puOutData[9];
				PdrvSpc_SetStw1(uStw1);
				PdrvSpc_SetNsollB((PDRV_INT32)uNsollB);
				PdrvSpc_SetStw2(uStw2);
				PdrvSpc_SetG1Stw(uG1Stw);
			}
		}

		return ( (PNIO_IOXS)OutDatIocs [slot_num][subslot_num]);	// consumer state (of local io device)
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
             PNIO_printf ( (PNIO_CHAR*) "new IO controller input consumer status (ICS) = 0x%x in slot %d, subslot %d\n",
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
            PNIO_printf ( (PNIO_CHAR*) "new IO controller output provider status (OPS) = 0x%x in slot %d, subslot %d\n",
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
    // *    Input:	   PNIO_UINT32    pAr;		// ID of the Application Relation
    // *    Output:	   ---
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
        PNIO_UINT32 ReportHostIP = HostIp;
        PNIO_UINT8* u8ReportHostIP = (PNIO_UINT8*)&ReportHostIP;

        PNIO_UNUSED_ARG (DevHndl);
        NumOfAr++;      // increment number of running ARs

        Ar_Connection_Info[ArNum-1].ArType = ArType;
        Ar_Connection_Info[ArNum-1].ArNum = ArNum;

        m_bPDRVIsDisconnect = PDRV_FALSE;

        PNIO_printf ( (PNIO_CHAR*) "##CONNECT_IND AR=%d AR type=%d sendclock=%d, reduction_ratio_in=%d, reduction_ratio_out=%d, sessionKey=%d, hostIP=%03d.%03d.%03d.%03d\n",
                      ArNum,
                      ArType,
                      SendClock,
                      RedRatioIocrIn, RedRatioIocrOut,
                      ArSessionKey, 
                      *(u8ReportHostIP + 0), *(u8ReportHostIP + 1), *(u8ReportHostIP + 2), *(u8ReportHostIP + 3));

        if (ArType == 0x10)            //  CM_AR_TYPE_SINGLE_RTC3
            IsoArNum = ArNum;
        bPdrvPar_EstablishCxn(ArType, ArNum);    /* PROFIdrive open a parameter channel */
    }


    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_ar_ownership_ind (pAR)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  The AR ownership Indication of a module
    // *
    // *  Input:   PNIO_UINT32 pAr;		                ID of the Application Relation
    // *
    // *  Output:  PNIO_EXP*   pOwnSub->OwnSessionKey   set to 0 only if ownership is rejected
    // *			                                    else keep unchanged
    // *                       pOwnSub->IsWrongSubmod   set to PNIO_TRUE only if wrong module,
    // *			                                    else keep unchanged
    // *
    // *----------------------------------------------------------------*
    PNIO_VOID   PNIO_cbf_ar_ownership_ind
			(
				PNIO_UINT32		DevHndl,
				PNIO_UINT32     ArNum,          // AR number 1....NumOfAR
				PNIO_EXP*   	pOwnSub     	// [in] expected configuration in ownership indication
			)
	{
		PNIO_UINT32 i;

		PNIO_UNUSED_ARG (DevHndl);

		PNIO_printf ( (PNIO_CHAR*) "##OWNERSHIP_IND AR = %d number of submodules = %d\n",
					 ArNum, pOwnSub->NumOfPluggedSub);

		for (i = 0; i < pOwnSub->NumOfPluggedSub; i++)
		{
			 /* Wrong Submodule AND PROFIdrive Submodule with compatible standard telegram? */
			if (   (pOwnSub->Sub[i].IsWrongSubmod == PNIO_TRUE)
				&& (m_uPDRVSourceTgmSel == 0U)
				&& (pOwnSub->Sub[i].ApiNum == PDRV_API)
				&& (pOwnSub->Sub[i].SlotNum == PDRV_STDTLG_SLOT)
				&& (pOwnSub->Sub[i].SubNum == PDRV_STDTLG_SUBSLOT)
				&& (   (pOwnSub->Sub[i].ModIdent == PDRV_MODULE_ID_MAP)
					|| (pOwnSub->Sub[i].ModIdent == PDRV_MODULE_ID_MAP1)
				   )
				&& (   (pOwnSub->Sub[i].SubIdent == PDRV_SUBMOD_ID_TLG1)
					|| (pOwnSub->Sub[i].SubIdent == PDRV_SUBMOD_ID_TLG2)
					|| (pOwnSub->Sub[i].SubIdent == PDRV_SUBMOD_ID_TLG3)
				   )
				)
			{
				PNIO_DEV_ADDR  Addr;    /* location (module/submodule) */
				PNIO_UINT32 Status;
				PNIO_UINT32 InputDataLen;
				PNIO_UINT32 OutputDataLen;

				Addr.Geo.Slot    = PDRV_STDTLG_SLOT;
				Addr.Geo.Subslot = PDRV_STDTLG_SUBSLOT;

				/* first step: pull the old submodule */
				Status = PNIO_sub_pull (PNIO_SINGLE_DEVICE_HNDL,
										PDRV_API,
										&Addr);         /* location (slot, subslot) */
				if (Status == PNIO_OK)
				{   /* second step: plug the new submodule */
					m_uPDRVTelegram = pOwnSub->Sub[i].SubIdent & 0x0FFFFUL; /* set expected standard telegram */
					if(pOwnSub->Sub[i].SubIdent == PDRV_SUBMOD_ID_TLG1)
					{
						InputDataLen = 4;
						OutputDataLen = 4;
					}
					if(pOwnSub->Sub[i].SubIdent == PDRV_SUBMOD_ID_TLG2)
					{
						InputDataLen = 8;
						OutputDataLen = 8;
					}
					if(pOwnSub->Sub[i].SubIdent == PDRV_SUBMOD_ID_TLG3)
					{
						InputDataLen = 18;
						OutputDataLen = 10;
					}
					Status = PNIO_sub_plug (PNIO_SINGLE_DEVICE_HNDL,
											PDRV_API,
											&Addr,			                    // location (slot, subslot)
											PDRV_MODULE_ID_MAP,
											pOwnSub->Sub[i].SubIdent,	        // submodule identifier
											InputDataLen,                       // submodule input data length
											OutputDataLen,                      // submodule output data length
											PNIO_IM0_NOTHING,                    // Submodule not supports IM0
											(IM0_DATA*)NULL,
											PNIO_S_GOOD);                        // initial iops value, only for submodules without io data

				}
				if (Status == PNIO_OK)
				{
					pOwnSub->Sub[i].IsWrongSubmod = PNIO_FALSE;     /* is a compatible substitute-submodule */
					/* update data lengths of IO-DATA */
					InDatLen[PDRV_STDTLG_SLOT][PDRV_STDTLG_SUBSLOT] = pOwnSub->Sub[i].In.data_length;
					OutDatLen[PDRV_STDTLG_SLOT][PDRV_STDTLG_SUBSLOT] = pOwnSub->Sub[i].Out.data_length;
					/* set if isochronous mode is required */
					m_bPDRVIsIsoReq = m_uPDRVTelegram == PDRV_STDTLG3 ? PDRV_TRUE : PDRV_FALSE;
				}
				else
				{
					PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
				}
			}

			/* Is a compatible module id used? */
			if (   (pOwnSub->Sub[i].IsWrongSubmod == PNIO_TRUE)
				&& (pOwnSub->Sub[i].ApiNum == PDRV_API)
				&& (pOwnSub->Sub[i].ModIdent == PDRV_MODULE_ID_MAP1)
				&& (   (   (pOwnSub->Sub[i].SlotNum == PDRV_PAP_SLOT)
						&& (pOwnSub->Sub[i].SubNum == PDRV_PAP_SUBSLOT)
						&& (pOwnSub->Sub[i].SubIdent == PDRV_SUBMOD_ID_PAP)
					   )
					|| (   (pOwnSub->Sub[i].SlotNum == PDRV_STDTLG_SLOT)
						&& (pOwnSub->Sub[i].SubNum == PDRV_STDTLG_SUBSLOT)
						&& ((pOwnSub->Sub[i].SubIdent & 0x0FFFFUL) == m_uPDRVTelegram)
					   )
				   )
			   )
			{
				pOwnSub->Sub[i].IsWrongSubmod = PNIO_FALSE;     /* It is a compatible module. */
			}
			PNIO_printf ( (PNIO_CHAR*) "  Api=%d Slot=%d Sub=%d ModID=%d SubID=%d OwnSessKey=%d isWrong=%d\n",
				 pOwnSub->Sub[i].ApiNum,
				 pOwnSub->Sub[i].SlotNum,
				 pOwnSub->Sub[i].SubNum,
				 pOwnSub->Sub[i].ModIdent,
				 pOwnSub->Sub[i].SubIdent,
				 pOwnSub->Sub[i].OwnSessionKey,
				 pOwnSub->Sub[i].IsWrongSubmod
				);

			#if 0
				// *------------------------------------------------
				// *  Only if submodule ownership shall be rejected,
				// *  application has to set the OwnSessionKey to 0,
				// *  otherwise keep unchanged
				// *------------------------------------------------
				pOwnSub->Sub[i].OwnSessionKey = 0;     // reject ownership
			#endif
			#if 0
				// *------------------------------------------------
				// *  If real and expected module/submodule ID are
				// *  not equal, the stack presets IsWrongSubmod = TRUE.
				// *  No IO data exchange is possible with this submodule.
				// *  if the submodule is a compatible substitute module,
				// *  set IsWrongSubmod = FALSE, otherwise keep unchanged.
				// *------------------------------------------------
				pOwnSub->Sub[i].IsWrongSubmod = PNIO_FALSE;     // is a compatible substitute-submodule
			#endif

		}  // end for (i = 0; i < pOwnSub->NumOfPluggedSub...
	}



    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_param_end_ind ()
    // *
    // *----------------------------------------------------------------*
    // *
    // *  The PNIO controller has sent an "PARAM END" event
    // *
    // *  Input:	DevHndl        Device handle
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
        if (SubslotNum == 0)
        { // ****param end for all submodules ****
    	    PNIO_printf ( (PNIO_CHAR*) "##PARAM END  for all submodules, ArNum=%d, Session=%d \n",
                         ArNum, SessionKey);
        }
        else
        { // ****param end for one submodule ****
    	    PNIO_printf ( (PNIO_CHAR*) "##PARAM END for Api=%d, Slot=%d, Sub=%d, ArNum=%d, Session=%d \n",
                         Api, SlotNum, SubslotNum, ArNum, SessionKey);
        }

	    return (PNIO_SUBMOD_STATE_RUN);	    // system generates automatically "application ready"-telegram
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
    // *  Input:	DevHndl        Device handle
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

	    // *-----------------------------------------------------------------------
        // *    perform a first data exchange, to set IOPS and IOCS to STATE_GOOD
        // *    necessary to set IO controller state to RUN without error.
	    // *-----------------------------------------------------------------------
		Status =  PNIO_initiate_data_read (DevHndl);

		{ // Example: APDU status can read here
			PNIO_UINT32 ApduStatus = PNIO_get_last_apdu_status (PNIO_SINGLE_DEVICE_HNDL, ArNum);
			PNIO_UNUSED_ARG(ApduStatus);
		} /*lint !e438 last value assigned is not used */

		if (PNIO_OK == Status)
		{
			Status =  PNIO_initiate_data_write (DevHndl);
			if (PNIO_OK != Status)
			{
				PNIO_printf ( "##Error IO Data Exchange in PNIO_cbf_ready_for_input_update_ind()\n");
			}
        }

        PnUsr_ActivateIoDatXch();

	    // *-----------------------------------------------------------------------
        // *    notify, if first "readyForInputUpdate" after AR start or
        // *    because of replugging a submodule.
	    // *-----------------------------------------------------------------------
        if (InpUpdState == PNIO_AR_STARTUP)
        {
    		PNIO_printf ( "##READY FOR INPUT UPDATE DURING AR_STARTUP ARnum=%d\n", ArNum);
        }
        else
        {
    		PNIO_printf ( "##READY FOR INPUT UPDATE DURING AR INDATA  ARnum=%d\n", ArNum);
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
    // *  Input:	DevHndl        Device handle
    // *            ArNum          AR number (1...N)
    // *            SessionKey     0  (not used here)
    // *  Output:   ---
    // *
    // *----------------------------------------------------------------*
    void PNIO_cbf_ar_indata_ind (PNIO_UINT32 DevHndl,
                                 PNIO_UINT16 ArNum,
                                 PNIO_UINT16 SessionKey)
    {
	    PNIO_UNUSED_ARG (DevHndl);
	    PNIO_printf ( (PNIO_CHAR*) "##AR IN-Data event indication received, ArNum = %xh, Session = %xh\n",
				      ArNum, SessionKey);
	    /* PROFIdrive: sent message to general state machine */
	    PdrvSpc_NotifyFirstIO();
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
    // *  Input:	DevHndl        Device handle
    // *            ArNum          AR number (1...N)
    // *            SessionKey     0  (not used here)
    // *            ReasonCode	   reason code (see PNIO_AR_REASON)
    // *	 Output:   ---
    // *
    // *----------------------------------------------------------------*
    void PNIO_cbf_ar_disconn_ind	 (PNIO_UINT32	 DevHndl,
                                      PNIO_UINT16    ArNum,
                                      PNIO_UINT16    SessionKey,
								      PNIO_AR_REASON ReasonCode)
    {
	    PNIO_UNUSED_ARG (DevHndl);
	    PNIO_printf ( (PNIO_CHAR*) "##AR Offline indication ,ArNum = %xh, Session = %xh, Reason = %xh\n",
		              ArNum, SessionKey, ReasonCode);

        if (NumOfAr)
            NumOfAr--;      // decrement number of running ARs

        if(    (Ar_Connection_Info[ArNum-1].ArType == PNIO_AR_TYPE_SINGLE)
        	|| (Ar_Connection_Info[ArNum-1].ArType == PNIO_AR_TYPE_SINGLE_RTC3)
        )
		{
        	m_bPDRVIsDisconnect = PDRV_TRUE;
		}

		#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
			if ((IsoGpiosActivated == PNIO_TRUE) && (ArNum == IsoArNum))
			{
                PNIO_UINT32 Status = PNIO_OK;

				IsoArNum = 0;
				IsoGpiosActivated = PNIO_FALSE;

				if(PNIO_IsoObjCheck(IsoGpioTiHndl) == PNIO_OK)
				{
				    PNIO_printf ("deactivate GPIOs for Ti....");
				    Status = PNIO_IsoFreeObj (IsoGpioTiHndl);
	                if (Status == PNIO_OK)
	                    PNIO_printf ("OK\n");
	                else
	                    PNIO_printf ("ERROR PNIO_IsoFreeObj (Event=0x%x) occured\n", IsoGpioTiHndl  );
				}

				if(PNIO_IsoObjCheck(IsoPdrvTiHndl) == PNIO_OK)
				{
				    PNIO_printf ("deactivate input for Ti....");
				    Status = PNIO_IsoFreeObj (IsoPdrvTiHndl);
	                if (Status == PNIO_OK)
	                    PNIO_printf ("OK\n");
	                else
	                    PNIO_printf ("ERROR PNIO_IsoFreeObj (Event=0x%x) occured\n", IsoPdrvTiHndl  );
				}

				if(PNIO_IsoObjCheck(IsoGpioToHndl) == PNIO_OK)
				{
				    PNIO_printf ("deactivate GPIOs for To....");
                    Status = PNIO_IsoFreeObj (IsoGpioToHndl);
                    if (Status == PNIO_OK)
                         PNIO_printf ("OK\n");
                     else
                         PNIO_printf ("ERROR PNIO_IsoFreeObj (Event=0x%x) occured\n", IsoGpioToHndl );

				}
			}
			bPdrvPar_DisconnCxn(ArNum);    /* PDRV: PROFIdrive close a parameter channel */
	#endif
    }



    // *****************************************************************************************
    // *************************** ALARM INDICATIONS  ******************************************
    // *****************************************************************************************
    void    PNIO_cbf_dev_alarm_ind (PNIO_UINT32			DevHndl,
								    PNIO_DEV_ALARM_DATA	*pAlarm)
    {
	    PNIO_printf ( (PNIO_CHAR*) "##APDU alarm indication, Dev=%d, Slot=%d, Subslot=%d, AR=%d, Sess=%d\n",
                    DevHndl,
                    pAlarm->SlotNum,
                    pAlarm->SubslotNum,
                    pAlarm->ArNum,
                    pAlarm->SessionKey
                );

        PNIO_printf ( (PNIO_CHAR*) "  Prio=%xh, Type=%xh, DiagCH = %d, DiagGen=%d, Tag=%d, DatLen=%d\n",
                    pAlarm->AlarmPriority,
                    pAlarm->AlarmType,
                    pAlarm->DiagChannelAvailable,
                    pAlarm->DiagGenericAvailable,
                    pAlarm->UserStructIdent,
                    pAlarm->UserAlarmDataLength
                );
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
									PNIO_UINT16		Diag_tag)
    {
       PNIO_UNUSED_ARG (DevHndl);
   	if( 0 == ArNum )
   	{
   		PNIO_printf( "Asynchronous request not sent - no existing AR. Alarmtype=%d Api=%d Slot=%d Subslot=%d\n",
   				AlarmType, Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
   	}
   	else
   	{
      PNIO_printf ( (PNIO_CHAR*) "Asynchronous request ArNum=%d Alarmtype=%d Api=%d Slot=%d Subslot=%d Status = %x\n",
		              ArNum, AlarmType, Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, Status);
   	}
      PNIO_printf( "User tag %x\n", Diag_tag );
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
		PNIO_UINT32			DevHndl,        // device handle
		PNIO_UINT32			Api,            // application process identifier
        PNIO_UINT16         ArNum,			// ar - number
        PNIO_UINT16 		SessionKey,	    // ar session number
		PNIO_UINT32			SequenceNum,    // CLRPC sequence number
		PNIO_DEV_ADDR		*pAddr,			// geographical or logical address
		PNIO_UINT32			RecordIndex,    // record index
		PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
		PNIO_UINT8			*pBuffer,		// [in] buffer pointer
		PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
	)
    {
		PNIO_UINT32   Status	    = PNIO_OK;

		PNIO_UINT32 slot_nr		    = pAddr->Geo.Slot;
		PNIO_UINT32 subslot_nr	    = pAddr->Geo.Subslot;
		PNIO_UINT32 DoHexdumpData   = PNIO_FALSE;

		PNIO_UNUSED_ARG (DevHndl);
		PNIO_UNUSED_ARG (Api);
		PNIO_UNUSED_ARG (ArNum);
		PNIO_UNUSED_ARG (SessionKey);
		PNIO_UNUSED_ARG (SequenceNum);

		// *----------------------------------------------*
		// *  handle special indizes for I&M
		// *----------------------------------------------*
		switch (RecordIndex)
		{
			// *----------------------------------------------*
			// *  read IO data fro submodule
			// *----------------------------------------------*
			#if !IOD_INCLUDE_REC8028_8029
				case 0x8028: // **** read io input data from submodule *****
						PNIO_printf ( (PNIO_INT8*) "##READ IO INPUT DATA, Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
									 Api, slot_nr, subslot_nr, *pBufLen, SequenceNum);

						if (pAddr->Geo.Slot != 0)   // not DAP --> normal input, output or input/output modules
						{
							Status = RecInpDatObjElement_Handler
										 ( Api,                                                 // api number
										   pAddr,                                               // slot/subslot number
										   pBufLen,                                             // total response buffer length
										   pBuffer,                                             // pointer to response buffer
										   pPnioState,                                          // request stat (see pnio spec.)
										   InDatLen[pAddr->Geo.Slot][pAddr->Geo.Subslot],       // length of io input-data block
										   &(InData[pAddr->Geo.Slot][pAddr->Geo.Subslot][0]),   // pointer to input data block
										   InDatIocs[pAddr->Geo.Slot][pAddr->Geo.Subslot],      // iocs value for input data block
										   InDatIops[pAddr->Geo.Slot][pAddr->Geo.Subslot]       // iops value for input data block
										  );
						}
						else
						{  // DAP --> no input or output data in this example
							PNIO_UINT8 Dummy = 0;
							Status = RecInpDatObjElement_Handler
										 ( Api,                                                 // api number
										   pAddr,                                               // slot/subslot number
										   pBufLen,                                             // total response buffer length
										   pBuffer,                                             // pointer to response buffer
										   pPnioState,                                          // request stat (see pnio spec.)
										   0,                                                   // length of io input-data block
										   &Dummy,                                              // any dummy pointer
										   PNIO_S_GOOD,                                         // we assume: always good in this example
										   PNIO_S_GOOD                                          // we assume: always good in this example
										  );
						}
					 break;

				case 0x8029: // **** read io output data from submodule + substituted values *****
						PNIO_printf ( (PNIO_INT8*) "##READ IO OUTPUT DATA and SUBSTITUTED VALUES, Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
									 Api, slot_nr, subslot_nr, *pBufLen, SequenceNum);
						if (pAddr->Geo.Slot != 0)   // not DAP --> normal input, output or input/output modules
						{
							Status = RecOutDatObjElement_Handler
										 ( Api,                                                   // api number
										   pAddr,                                                 // slot/subslot number
										   pBufLen,                                               // total response buffer length
										   pBuffer,                                               // pointer to response buffer
										   pPnioState,                                            // request stat (see pnio spec.)
										   OutDatLen[pAddr->Geo.Slot][pAddr->Geo.Subslot],        // length of io input-data block
										   &(OutData[pAddr->Geo.Slot][pAddr->Geo.Subslot][0]),    // pointer to input data block
										   OutDatIocs[pAddr->Geo.Slot][pAddr->Geo.Subslot],       // iocs value for input data block
										   OutDatIops[pAddr->Geo.Slot][pAddr->Geo.Subslot],       // iops value for input data block
										   &(OutSubstVal[pAddr->Geo.Slot][pAddr->Geo.Subslot][0]),// pointer to substituted values                                          // pointer to substituted values for output data
										   2,                                                     // SubstitutionMode   = 0: ZERO,  1: last value, 2: replacement value (see pnio-spec)
										   0                                                      // SubstitutionActive = 0: output values active, 1: substituted values active
										  );
						}
						else
						{  // PDEV port: no io data, iops = iocs = GOOD
							PNIO_UINT8 Dummy = 0;
							Status = RecOutDatObjElement_Handler
										 ( Api,                                                   // api number
										   pAddr,                                                 // slot/subslot number
										   pBufLen,                                               // total response buffer length
										   pBuffer,                                               // pointer to response buffer
										   pPnioState,                                            // request stat (see pnio spec.)
										   0,                                                     // length of io input-data block
										   &Dummy,                                                // any dummy pointer to input data block
										   PNIO_S_GOOD,                                           // we assume: always good in this example
										   PNIO_S_GOOD,                                           // we assume: always good in this example
										   &Dummy,                                                // dummy pointer to input data block
										   2,                                                     // SubstitutionMode   = 0: ZERO,  1: last value, 2: replacement value (see pnio-spec)
										   0                                                      // SubstitutionActive = 0: output values active, 1: substituted values active
										  );
						}
					 break;
			#endif

			// *----------------------------------------------*
			// *  read isochronous mode data
			// *----------------------------------------------*
			case 0x8030: // **** IM 4 *****
					PNIO_printf ( (PNIO_CHAR*) "##READ IsoMData, Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
								 Api, slot_nr, subslot_nr, *pBufLen, SequenceNum);
					*pBufLen = 0;
				 break;

			case 0x2F:
			case PDRV_RECLOC: /* PROFIdrive record of base parameter access local see PDRV V4.2 table 186 */
				{
					PNIO_UINT8 uEc1;
					//PNIO_printf ( (PNIO_INT8*) "##READ PDRV Record: Api=%d Slot=%d Subslot=%d Len=%d, RecordIndex=%d\n",
					            // Api, slot_nr, subslot_nr, *pBufLen, RecordIndex);
					uEc1 = uPdrvPar_ReadReqCxn(ArNum, pBufLen, pBuffer);
					if (uEc1 != PDRV_EC1_OK)
					{
						PNIO_printf ( (PNIO_CHAR*) "##Error Code 1 = 0x%02x \n\n", (PNIO_UINT32 ) uEc1);

						pPnioState->ErrCode   = 0xde;  /* IODReadRes with ErrorDecode = PNIORW */
						pPnioState->ErrDecode = 0x80;  /* PNIORW */
						pPnioState->ErrCode1  = uEc1;  /* Error Code 1 */
						pPnioState->ErrCode2  = 0;     /* here dont care */
						pPnioState->AddValue1 = 0;     /* here dont care */
						pPnioState->AddValue2 = 0;     /* here dont care */
						return PNIO_NOT_OK;
					}
					DoHexdumpData = PNIO_FALSE;
					break;
				}

			// *----------------------------------------------*
			// *  user defined records  index 0..7fffh
			// *----------------------------------------------*
			case 2: // **** user specific demo record *****
					PNIO_printf ( (PNIO_CHAR*) "##READ Demo-Record , Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
								 Api, slot_nr, subslot_nr, *pBufLen, SequenceNum);

					if (*pBufLen > sizeof(DEMO_RECORD))
						*pBufLen = sizeof (DEMO_RECORD);

					OsMemCpy (pBuffer, DEMO_RECORD, *pBufLen);
					break;

			case 1: // **** user specific startup record *****
					PNIO_printf ( (PNIO_CHAR*) "##READ startup-Record (here: demo record is returned), Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
								 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen, SequenceNum);
					if (*pBufLen > sizeof(DEMO_RECORD))
						*pBufLen = sizeof (DEMO_RECORD);

					OsMemCpy (pBuffer, DEMO_RECORD, *pBufLen);
					break;

			default:
				// *----------------------------------------------*
				// *  all record number < 0x7fff are user specific,
				// *  but we intercept the simatic specific rec-read
				// *  request on slot=0, subslot=1, index={0,1}, which
				// *  is not supported here.
				// *----------------------------------------------*
				 if (  (RecordIndex <= 0x7fff) || (RecordIndex == PDRV_RECLOC) || (RecordIndex == PDRV_RECGLO))
				 {
					if (    ( (pAddr->Geo.Slot == 0) && (pAddr->Geo.Subslot == 1))
						|| (pAddr->Geo.Subslot >= 200))
					{   // ** we do not support that request, may be a SIEMENS-specific call      **
						// ** we respond with an adequate error message, specified in IEC 61158-6 **
						pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
						pPnioState->ErrDecode = 0x80;  // PNIORW
						pPnioState->ErrCode1  = 0xa9;  // example: Error Class 10 = application, ErrorNr 9 = "feature not supported"
						return (PNIO_NOT_OK);
					}
					else
					{
						// *----------------------------------------------*
						// *  read manufacturer specific record data,
						// *  implemented by the user
						// *----------------------------------------------*
						PNIO_UINT8    ReadRecDummyData[] = {"**Data1234 ReadRecord**"};

						// **** copy dummy data into the buffer, set data-s?e ***

						if (*pBufLen > sizeof (ReadRecDummyData))
						{
						   *pBufLen = sizeof (ReadRecDummyData);
						}
						OsMemCpy (pBuffer, ReadRecDummyData, *pBufLen);
						PNIO_printf ( (PNIO_CHAR*) "##READ_REC RQ,  Api =%d Slot=%d Subslot=%d Index=0x%x, Len=%d, Sequ_nr=%d\n",
								 Api, slot_nr, subslot_nr, RecordIndex, *pBufLen, SequenceNum);
					}
				 }
				 else
				 {
					// *----------------------------------------------*
					// *  all other record indizes >= 0x8000
					// *  may be imlemented by user, otherwise
					// *  we return "invalid index"
					// *----------------------------------------------*
					PNIO_printf ( (PNIO_CHAR*) "##READ_REC RQ, Api=%d Slot=%d Subslot=%d Index=0x%x, Len=%d, Sequ_nr=%d\n",
								 Api, slot_nr, subslot_nr, RecordIndex, *pBufLen, SequenceNum);

					// *** if an error occured, it must be specified according IEC 61158-6
					pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
					pPnioState->ErrDecode = 0x80;  // PNIORW
					pPnioState->ErrCode1  = 0xb0;  // example: Error Class 11 = access, ErrorNr 0 = "invalid index"
					pPnioState->ErrCode2  = 0;     // here dont care
					pPnioState->AddValue1 = 0;     // here dont care
					pPnioState->AddValue2 = 0;     // here dont care
					return (PNIO_NOT_OK);
				 }
				 break;
		}


		// **** print data as hexdump ****
		if (DoHexdumpData)
		{
			PNIO_UINT32 i;
			PNIO_UINT8* p8 = (PNIO_UINT8*) pBuffer;
			PNIO_printf ( (PNIO_CHAR*) "##REC_DATA = ");
			for (i=0; i < *pBufLen; i++)
			{
				PNIO_UINT32  Val;
				Val = (PNIO_UINT32 ) *(p8+i);;
				PNIO_printf ( (PNIO_CHAR*) "0x%02x ", Val);
			}

			PNIO_printf ( (PNIO_CHAR*) "\n\n");
		}



	  return (Status);
	}

    // *----------------------------------------------------------------*
    // *
    // *  PnUsr_cbf_Ti_input(void)
    // *
    // *  get data from stm32f407 and send to the plc  take 44us
    // *
    // *
    // *----------------------------------------------------------------*
    void PnUsr_cbf_Ti_input(void)
    {
    	volatile PNIO_UINT32 Status;

		Status = PNIO_initiate_data_write (PNIO_SINGLE_DEVICE_HNDL);        // input data
		switch (Status)
		{
			case PNIO_OK:
				break;
			case PNIO_NOT_OK:
				break;
			default:
				PNIO_ConsolePrintf ("Error 0x%x at PNIO_initiate_data_write()\n", Status);
				break;
		}
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
		PNIO_UINT32			DevHndl,        // device handle
		PNIO_UINT32			Api,            // application process identifier
        PNIO_UINT16         ArNum,          // ar - number
        PNIO_UINT16 		SessionKey,	    // ar session number
		PNIO_UINT32			SequenceNum,    // CLRPC sequence number
		PNIO_DEV_ADDR		*pAddr,			// geographical or logical address
		PNIO_UINT32			RecordIndex,    // record index
		PNIO_UINT32			*pBufLen,   	// [in, out] in: length to write, out: length, written by user
		PNIO_UINT8			*pBuffer,		// [in] buffer pointer
		PNIO_ERR_STAT		*pPnioState		// 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
	)

    {
		// **** copy dummy data into the buffer, set data-s?e ***
		PNIO_UINT8      WriteRecDummyData[PDRV_PAR_BLOCKSIZE];
		PNIO_UINT32     i;
		PNIO_UINT32     Status = PNIO_OK;
		PNIO_UINT32     DoHexdumpData = PNIO_FALSE;
		PNIO_UINT8      uEc1 = 0xB0;  /* Error Code 1 example: Error Class 11 = Access, ErrorNr 6 = "access denied" */

		PNIO_UNUSED_ARG (DevHndl);
		PNIO_UNUSED_ARG (Api);
		PNIO_UNUSED_ARG (ArNum);
		PNIO_UNUSED_ARG (SessionKey);
		PNIO_UNUSED_ARG (SequenceNum);

		// *** check data size (accepted data >= provided data ??) ***
		if (*pBufLen > sizeof (WriteRecDummyData))
		{
		   PNIO_printf ("Error: provided record data (%d) index = %d  > expected data (%d)\n",
						RecordIndex,
						*pBufLen,
						sizeof (WriteRecDummyData));
		   return (PNIO_NOT_OK);
		}

		switch (RecordIndex)
		{

				// *-----------------------------------------------------------------*
				// *  write isochronous mode data
				// *  (optional feature for special IRT modules only,
				// *  that support isochronous appliation)
				// *-----------------------------------------------------------------*
				case 0x8030:
					// ****************************************************************
					// *  BEGIN  user code
					// *
					// *  Note: A real profinet device, that performs an isochronous
					// *        application,
					// *
					// *        - must process the data of record 0x8030, including
					// *          plausibility check of the data.
					// *        - must be able to realize the TI, TO and application
					// *          cycle factors by hard- and software
					// *        - must activate this optional feature in the GSD file
					// *

					{
						// ********************* activate GPIOs for T_input, T_output ****************
						REC8030_ISOMDATA* pIsoM = (REC8030_ISOMDATA*)pBuffer;
						PNIO_UINT32 Slot    = (PNIO_UINT32) OsNtohs (pIsoM->SlotNumber);
						PNIO_UINT32 Sub     = (PNIO_UINT32) OsNtohs (pIsoM->SubSlotNumber);
						PNIO_UINT32 Tcyc_ns = (PNIO_UINT32) OsNtohs (pIsoM->TimeDataCycle) * 31250;   // TimeDataCycle * 31,25us  * 1000
						PNIO_UINT32 Ti_ns   = OsNtohl (pIsoM->TimeIOInput);
						PNIO_UINT32 To_ns   = OsNtohl (pIsoM->TimeIOOutput);
						PNIO_UINT32 TiDelay_ns = 0;
						PNIO_UINT16 Cacf = OsHtons (pIsoM->ControllerApplicationCycleFactor);

						#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
							if (Ti_ns < Tcyc_ns)
								TiDelay_ns = Tcyc_ns - Ti_ns;
							else
								PNIO_printf ("Ti (%d ns) must be lower Tcyc (%d ns)\n", Ti_ns, Tcyc_ns);

							if (Slot == 1)
							{
								// ** aktivate Ti on GPIO 5 ***
								if (IsoGpioTiHndl != 0)
								{
									PNIO_IsoFreeObj(IsoGpioTiHndl);
								}
								Status = PNIO_IsoActivateGpioObj(5, TiDelay_ns,  ISO_GPIO_LOW_ACTIVE, &IsoGpioTiHndl);
								if (Status != PNIO_OK)
								{
										PNIO_printf ("Error PNIO_IsoActivateGpioObj Ti=%d\n", Ti_ns);
								}

								if (IsoPdrvTiHndl != 0)
								{
									PNIO_IsoFreeObj(IsoPdrvTiHndl);
								}
								Status = PNIO_IsoActivateIsrObj((PNIO_VOID  *)&PnUsr_cbf_Ti_input, TiDelay_ns, &IsoPdrvTiHndl);
								if (Status != PNIO_OK)
								{
									PNIO_printf ("Error PNIO_IsoActivateIsrObj Ti=%d\n", Ti_ns);
								}

								// ** aktivate To on GPIO 7 ***
								if (IsoGpioToHndl != 0)
								{
									PNIO_IsoFreeObj(IsoGpioToHndl);
								}
								Status = PNIO_IsoActivateGpioObj(7, To_ns,  ISO_GPIO_LOW_ACTIVE, &IsoGpioToHndl);
								if (Status != PNIO_OK)
								{
									PNIO_printf ("Error PNIO_IsoActivateGpioObj Ti=%d\n", To_ns);
								}
							}
							IsoGpiosActivated = PNIO_TRUE;
						#endif


						PNIO_printf ( (PNIO_CHAR*) "##WRITE IsoM-Data, Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
									 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen, SequenceNum);
								{
									PNIO_printf ( (PNIO_CHAR*) "\n=========== decode ISOM record with index 0x8030 =============\n");
									PNIO_printf ( (PNIO_CHAR*) "Slotnumber          = %10d\n", Slot);
									PNIO_printf ( (PNIO_CHAR*) "SubSlotnumber       = %10d\n", Sub);
									PNIO_printf ( (PNIO_CHAR*) "ConApplCycleFactor  = %10d\n", OsHtons (pIsoM->ControllerApplicationCycleFactor));
									PNIO_printf ( (PNIO_CHAR*) "TimeDataCycle       = %10d nsec\n", Tcyc_ns);
									PNIO_printf ( (PNIO_CHAR*) "TimeIOInput         = %10d nsec\n", Ti_ns);
									PNIO_printf ( (PNIO_CHAR*) "TimeIOOutput        = %10d nsec\n", To_ns);
									PNIO_printf ( (PNIO_CHAR*) "TimeIOInputValid    = %10d nsec\n", OsHtonl (pIsoM->TimeIOInputValid));
									PNIO_printf ( (PNIO_CHAR*) "timeIOOutputValid   = %10d nsec\n", OsHtonl (pIsoM->TimeIOOutputValid));
									PNIO_printf ( (PNIO_CHAR*) "\n");
									DoHexdumpData = PNIO_FALSE;        // do not print data as hexdump again..
								}
							/* Test the ISOM data. */
							if(   (Slot == PDRV_STDTLG_SLOT)
							   && (Sub == PDRV_STDTLG_SUBSLOT)
							   && (Cacf >= PDRV_ISO_CACF_MIN)
							   && (Cacf <= PDRV_ISO_CACF_MAX)
							   && (Tcyc_ns >= PDRV_ISO_TDC_MIN)
							   && (Tcyc_ns <= PDRV_ISO_TDC_MAX)
							   && (Ti_ns >= PDRV_ISO_IOI_MIN)
							   && (To_ns >= PDRV_ISO_IOO_MIN)
							  )
							{
								m_uPDRVIsoCacf = Cacf;
								m_bPDRVIsIsoActiv = PDRV_TRUE;
							}
							else
							{
								m_uPDRVIsoCacf = 0U;
								m_bPDRVIsIsoActiv = PDRV_FALSE;
								PNIO_printf ( (PNIO_CHAR*) "ERROR: IsoM-Data are not in range.\n\n");
								Status = PNIO_NOT_OK;
							}
					}



					// ****************************************************************
					// *     END    user code
					// ****************************************************************
					 break;

			// *----------------------------------------------*
			// *  user defined records  index 0..7fffh
			// *----------------------------------------------*
			case 0x2F:
			case PDRV_RECLOC: /* PDRV: PROFIdrive record of base parameter access local see PDRV V4.2 table 186 */
			{

				//PNIO_printf ( (PNIO_INT8*) "##WRITE PDRV Record: Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
				  //           Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen, SequenceNum);
				uEc1 = uPdrvPar_WriteReqCxn(ArNum, pBufLen, pBuffer);
				if (uEc1 != PDRV_EC1_OK)
				{
					PNIO_printf ( (PNIO_CHAR*) "##Error Code 1 = 0x%02x \n\n", (PNIO_UINT32 ) uEc1);
					Status = PNIO_NOT_OK;
				}
				break;
			}
			case 2: // **** user specific demo record *****
					PNIO_printf ( (PNIO_CHAR*) "##WRITE Demo-Record , Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
								 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen, SequenceNum);
					break;

			case 1: // **** user specific startup record *****
					PNIO_printf ( (PNIO_CHAR*) "##WRITE startup-Record from GSD file , Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
								 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen, SequenceNum);
					break;
			default:
				 PNIO_printf ( "Default: Api=%d Slot=%d Subslot=%d Len=%d, RecordIndex=%d\n", Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen, RecordIndex);
				 Status = PNIO_NOT_OK;
				 break;
		}



		if (Status == PNIO_OK)
		{
			// *** copy the record data into a buffer for further use **
			OsMemCpy (WriteRecDummyData,		// destination pointer for record data
					pBuffer,				// source pointer for record data
					*pBufLen);			    // length of the accepted data


			// **** print data as hexdump ****
			if (DoHexdumpData)
			{
				PNIO_printf ( (PNIO_CHAR*) "##WRITE_REC RQ, Api=%d Slot=%d Subslot=%d Index=0x%x, Len=%d, Sequ_nr=%d\n",
							 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, RecordIndex, *pBufLen, SequenceNum);

				PNIO_printf ( (PNIO_CHAR*) "##REC_DATA = ");
				for (i=0; i < *pBufLen; i++)
				{
					PNIO_UINT32  Val;
					Val = (PNIO_UINT32 ) WriteRecDummyData[i];
					PNIO_printf ( (PNIO_CHAR*) "0x%02x ", Val);
				}

				PNIO_printf ( (PNIO_CHAR*) "\n\n");
			}
		}
		else
		{// *** if an error occured, it must be specify  according to IEC 61158-6
			pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
			pPnioState->ErrDecode = 0x80;  // PNIORW
			pPnioState->ErrCode1  = uEc1;  // example: Error Class 11 = Access, ErrorNr 6 = "access denied"
			pPnioState->ErrCode2  = 0;     // here dont care
			pPnioState->AddValue1 = 0;     // here dont care
			pPnioState->AddValue2 = 0;     // here dont care
		}

		return (Status);   // OK: function executed, NOT_OK: error occured
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
    // *  Input:	PNIO_UINT8      *pStationName   name string,
    // * 			PNIO_UINT16     NameLength	   length of the name string
    // * 			PNIO_UINT8      Remanent        <> 0: save remanent
    // *
    // *
    // *  Output:   return          PNIO_OK: function completed successfully
    // *	                        PNIO_NOT_OK: function could not be completed
    // *----------------------------------------------------------------*
    PNIO_UINT32  PNIO_cbf_save_station_name
                        (	PNIO_INT8    *pStationName,
						    PNIO_UINT16   NameLength,
						    PNIO_UINT8   Remanent )
    {
       if (Remanent)
       {
           PNIO_printf((PNIO_CHAR*) "##Save remanent: Station Name = %.*s  received, Length=%d  Remanent = %d\n",
               NameLength, pStationName, NameLength, Remanent);

          // **** store data in non volatile memory ****
          Bsp_nv_data_store (PNIO_NVDATA_DEVICENAME,    // nv data type: device name
                             pStationName,              // source pointer to the devicename
                             NameLength);               // length of the device name
       }

       else
       {
           PNIO_printf ( (PNIO_CHAR*) "Station Name = %.*s  received, Length=%d  Remanent = %d\n",
	                     NameLength, pStationName, NameLength, Remanent);

           Bsp_nv_data_store (PNIO_NVDATA_DEVICENAME,    // nv data type: device name
                              "",                        // source pointer to the devicename
                              0);               // length of the device name
       }

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
    // *  Input:	PNIO_UINT32      NewIpAddr      new ip addres
    // * 			PNIO_UINT32      SubnetMask     Subnet mask
    // * 			PNIO_UINT32      DefRouterAddr  default gateway
    // * 			PNIO_UINT8       Remanent       <> 0: save remanent,
    // *                                            == 0: do not save remanent
    // *
    // *
    // *  Output:   return		    PNIO_OK: function completed successfully
    // *	                        PNIO_NOT_OK: function could not be completed
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
            PNIO_printf("##Remanent address was stored \n");

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

            PNIO_printf("Remanent address was deleted \n");
        }

        OsFree(pIpSuite);

        return (PNIO_OK);

    }

    /**
     *  @brief there is new ip data
     *
     *
     *  information about new ip, mask and gateway
     *	only printed to console to show the data
     *
     */
    PNIO_UINT32 PNIO_cbf_report_new_ip_addr(	PNIO_UINT32 NewIpAddr,
    								    	PNIO_UINT32 SubnetMask,
											PNIO_UINT32 DefRouterAddr)
    {
        PNIO_UINT32 ReportIpSuite = NewIpAddr;
        PNIO_UINT32 ReportSubnetMask = SubnetMask;
        PNIO_UINT32 ReportDefRouterAddr = DefRouterAddr;

        PNIO_UINT8* u8ReportIpSuite = (PNIO_UINT8*)&ReportIpSuite;
        PNIO_UINT8* u8ReportSubnetMask = (PNIO_UINT8*)&ReportSubnetMask;
        PNIO_UINT8* u8ReportDefRouterAddr = (PNIO_UINT8*)&ReportDefRouterAddr;

        PNIO_printf("IP address = %03d.%03d.%03d.%03d, Subnet mask = %03d.%03d.%03d.%03d, Default router = %03d.%03d.%03d.%03d\n",
            *(u8ReportIpSuite + 0), *(u8ReportIpSuite + 1), *(u8ReportIpSuite + 2), *(u8ReportIpSuite + 3),
            *(u8ReportSubnetMask + 0), *(u8ReportSubnetMask + 1), *(u8ReportSubnetMask + 2), *(u8ReportSubnetMask + 3),
            *(u8ReportDefRouterAddr + 0), *(u8ReportDefRouterAddr + 1), *(u8ReportDefRouterAddr + 2), *(u8ReportDefRouterAddr + 3));

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
        PNIO_printf ( (PNIO_CHAR*) "##LED Blink START, Port = %d, frequency = %d Hz \n", PortNum, frequency);
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
        PNIO_printf ( (PNIO_CHAR*) "##LED Blink STOP, Port = %d\n", PortNum);
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
    // *  Input:	PNIO_UINT32     DevHndl     Device handle
    // *  Input:	PNIO_RTF_OPTION RtfOption   specifies, which data have
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

           // *** set the substitute data  ***
           OsMemSet (pBuffer, 0xab, BufLen);    // example: all substitute data are 0xab (the user will change that...)

           // *** *pSubstMode = 2;      // we accept the preset value from the stack (set mode to "replace"), so nothing to do here"
           //*pSubstMode = 2;

           // *** we accept the preset value from stack:  "0 = Operation", if (remoteIOPS == GOOD and localIOCS == GOOD),  else "1 = Substitute"
           //*pSubstActive = REC8029_SUBSTSTATE_OPERATION;

           // we assume, the output data set is valid
           return (PNIO_S_GOOD);
        }
    #endif

    /**
     * @brief User functionality as reaction to new ARFSU
     *
     * @param[in]		  ARFSU_enabled      		PNIO_ARFSU_ENABLED, PNIO_ARFSU_DISABLED
     * @param[in]         ARFSU_changed        		PNIO_ARFSU_CHANGED, PNIO_ARFSU_NOT_CHANGED
     *
     * @return            void
     *
     * 	This function is called when ARFSU write record is recieved
     * 	Function is ready for user functionality
     * 	Informs user if ARFSU UUID was changed or not -> if parameterization was changed
     *
     */
    PNIO_VOID PNIO_cbf_report_ARFSU_record(
		    PNIO_UINT8			ARFSU_enabled,
		    PNIO_UINT8			ARFSU_changed
		    )
    {
	    if( PNIO_ARFSU_CHANGED == ARFSU_changed )
	    {
		    PNIO_printf( "New ARFSU UUID -> parameterization of device was changed\n" );
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
    	//    InDatLen[pAddr->ad.Geo1.Slot][pAddr->ad.Geo1.Subslot]  = (PNIO_UINT16)InputDataLen;
    	//    OutDatLen[pAddr->ad.Geo1.Slot][pAddr->ad.Geo1.Subslot] = (PNIO_UINT16)OutputDataLen;


    	//suzhen 20231214
    	PNIO_UINT32 subslotIndex = PnUsr_get_subslot_index(pAddr->ad.Geo1.Slot,pAddr->ad.Geo1.Subslot);
        if (subslotIndex)
    	{
    	    InDatLen[pAddr->ad.Geo1.Slot][subslotIndex]  = (PNIO_UINT16)InputDataLen;
    	    OutDatLen[pAddr->ad.Geo1.Slot][subslotIndex] = (PNIO_UINT16)OutputDataLen;
    	}
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

    /*------------------------------  PROFIdrive related functions                          ------------------------------*/

    /** Get the actual PROFIdrive telegram number which is selected in PNU00922 "Telegram selection"
     *  @details    none
     *  @return     PROFIdrive telegram number
    */
    PDRV_UINT16 uPdrvUsr_GetTelegramNo(PDRV_VOID)
    {
        return m_uPDRVTelegram;
    }

    /** Get the Controller Application Cycle Factor
     *  @details    none
     *  @return     Controller Application Cycle Factor which is received via ISOM
    */
    PDRV_UINT16 uPdrvUsr_GetIsoCacf(PDRV_VOID)
    {
        return m_uPDRVIsoCacf;
    }

    /** Get status of iscochronous activation
     *  @details    none
     *  @return     PDRV_TRUE when a valid ISOM is received and an isochronous AR is active
    */
    PDRV_BOOL bPdrvUsr_IsIsoActiv(PDRV_VOID)
    {
        return m_bPDRVIsIsoActiv;
    }

    /** Get status of isochronous requirement
     *  @details    none
     *  @return     PDRV_TRUE when isochrounous mode is required
    */
    PDRV_BOOL bPdrvUsr_IsIsoReq(PDRV_VOID)
    {
        return m_bPDRVIsIsoReq;
    }

    /** True isochron PLL is synchronized
     *  @details    PDRV user should be change the functionality and conditions to his own application!
     *  @return     PDRV_TRUE if PLL is isochron synchronisation is done
    */
    PDRV_BOOL bPdrvUsr_IsSyncedPLL(PDRV_VOID)
    {
        return (OutDatIops[PDRV_STDTLG_SLOT][PDRV_STDTLG_SUBSLOT] == PNIO_S_GOOD) ? PDRV_TRUE : PDRV_FALSE;
    }

    /*-------------  PROFIdrive parameter manager assigned text functions, read functions, write functions  --------------*/

    /** PROFIdrive read function for parameter PNU00300 "Source tgm sel."
     *  @details    none
     *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
    */
    PDRV_UINT32 uPdrv_RfPnu00300
        (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
         PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
         PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
         PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
        )
    {
        p_ptValues->o2[0U] = m_uPDRVSourceTgmSel;
        return PDRV_EV1_NOERROR;
    }

    /** PROFIdrive write function for parameter PNU00300 "Source tgm sel."
     *  @details    Function pulls and plugs submodules depending of telegram selection.
     *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
    */
    PDRV_UINT32 uPdrv_WfPnu00300
        (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
         PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
         PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
         PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
        )
    {
        m_uPDRVSourceTgmSel = p_ptValues->o2[0];
        return PDRV_EV1_NOERROR;
    }

    /** PROFIdrive text read function for parameter PNU00300 "Source tgm sel."
     *  @details
     *  @return     pointer to text string
    */
    const char * pcPdrv_TfPnu00300
        (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
         PDRV_UINT16 p_uSubindex            /**< [in] first subindex to write/read */
        )
    {
        return (p_uSubindex == 0U) ? "expected config" : "PNU00922";
    }

    /** PROFIdrive read function for parameter PNU00900 "Setpoint telegram"
     *  @details    none
     *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
    */
    PDRV_UINT32 uPdrv_RfPnu00900
        (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
         PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
         PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
         PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
        )
    {
        PDRV_UINT uI;
        PDRV_UINT8 uBuffer[PDRV_STDTLG3_OUTLEN];

        /* reset buffer first */
        for (uI = 0U; uI < sizeof(uBuffer)/sizeof(uBuffer[0]); uI++)
        {
            uBuffer[uI] = 0U;
        }

        /* Is PROFIdrive standard telegram 1 addressed? */
        if (m_uPDRVTelegram == PDRV_STDTLG1)
        {
            PDRV_UINT16 uStw1 = uPdrvSpc_GetStw1();     /* STW1 control word 1 */
            PDRV_UINT16 uNsollA = nPdrvSpc_GetNsollA(); /* NSOLL_A speed setpoint value 1 */

            /* get valid data from PROFIdrive (BigEndian OutData) */
            uBuffer[0] = uStw1 >> 8U;
            uBuffer[1] = uStw1 & 0x0FFU;
            uBuffer[2] = uNsollA >> 8U;
            uBuffer[3] = uNsollA & 0x0FFU;
        }
        else if (m_uPDRVTelegram == PDRV_STDTLG2)
        {
            PDRV_UINT16 uStw1 = uPdrvSpc_GetStw1();     /* STW1 control word 1 */
            PDRV_UINT16 uStw2 = uPdrvSpc_GetStw2();     /* STW2 control word 2 */
            PDRV_UINT16 uNsollB = nPdrvSpc_GetNsollB(); /* NSOLL_B speed setpoint value 1 */

            /* get data from PROFIdrive data with BigEndian OutData */
            uBuffer[0] = uStw1 >> 8U;
            uBuffer[1] = uStw1 & 0x0FFU;
            uBuffer[2] = (uNsollB >> 24U) & 0x0FFUL;
            uBuffer[3] = (uNsollB >> 16U) & 0x0FFUL;
            uBuffer[4] = (uNsollB >>  8U) & 0x0FFUL;
            uBuffer[5] = uNsollB & 0x0FFUL;
            uBuffer[6] = uStw2 >> 8U;
            uBuffer[7] = uStw2 & 0x0FFU;
        }
        else if (m_uPDRVTelegram == PDRV_STDTLG3)
        {
            PDRV_UINT16 uStw1 = uPdrvSpc_GetStw1();     /* STW1 control word 1 */
            PDRV_UINT16 uStw2 = uPdrvSpc_GetStw2();     /* STW2 control word 2 */
            PDRV_UINT16 uNsollB = nPdrvSpc_GetNsollB(); /* NSOLL_B speed setpoint value 1 */
            PDRV_UINT16 uG1Stw = uPdrvSpc_GetG1Stw();   /* G1_STW sensor 1 control word */

            /* get data from PROFIdrive data with BigEndian OutData */
            uBuffer[0] = uStw1 >> 8U;
            uBuffer[1] = uStw1 & 0x0FFU;
            uBuffer[2] = (uNsollB >> 24U) & 0x0FFUL;
            uBuffer[3] = (uNsollB >> 16U) & 0x0FFUL;
            uBuffer[4] = (uNsollB >>  8U) & 0x0FFUL;
            uBuffer[5] = uNsollB & 0x0FFUL;
            uBuffer[6] = uStw2 >> 8U;
            uBuffer[7] = uStw2 & 0x0FFU;
            uBuffer[8] = uG1Stw >> 8U;
            uBuffer[9] = uG1Stw & 0x0FFU;
        }

        for (uI = 0U; uI < p_uNrOfElements; uI++)
        {
            p_ptValues->os[uI] = uBuffer[p_uSubindex + uI];
        }
        return PDRV_EV1_NOERROR;
    }

    /** PROFIdrive read function for parameter PNU00907 "Actual value telegram"
     *  @details    none
     *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
    */
    PDRV_UINT32 uPdrv_RfPnu00907
        (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
         PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
         PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
         PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
        )
    {
        PDRV_UINT uI;

        for (uI = 0U; uI < p_uNrOfElements; uI++)
        {
            p_ptValues->os[uI] = InData[PDRV_STDTLG_SLOT][PDRV_STDTLG_SUBSLOT][p_uSubindex + uI];
        }
        return PDRV_EV1_NOERROR;
    }

    /** PROFIdrive read function for parameter PNU00922 "Telegram selection"
     *  @details    none
     *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
    */
    PDRV_UINT32 uPdrv_RfPnu00922
        (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
         PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
         PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
         PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
        )
    {
        p_ptValues->o2[0U] = m_uPDRVTelegram;
        return PDRV_EV1_NOERROR;
    }

    /** PROFIdrive write function for parameter PNU00922 "Telegram selection"
     *  @details    Function pulls and plugs submodules depending of telegram selection.
     *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
    */
    PDRV_UINT32 uPdrv_WfPnu00922
        (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
         PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
         PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
         PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
        )
    {
        PDRV_UINT32 uError = PDRV_EV1_NOERROR;

        /* Is source of telegram selection the Expected Configuration Data? */
        if (m_uPDRVSourceTgmSel == 0U)
        {
            uError = PDRV_EV1_VALUE_IMPERMISS;
        }    /* telegram changed? */
        else if (m_uPDRVTelegram != p_ptValues->o2[0])
        {
            PNIO_DEV_ADDR  Addr;    /* location (module/submodule) */
            PNIO_UINT32 Status;
            PNIO_UINT32 InputDataLen;
            PNIO_UINT32 OutputDataLen;

            Addr.Geo.Slot    = PDRV_STDTLG_SLOT;
            Addr.Geo.Subslot = PDRV_STDTLG_SUBSLOT;

            /* first step: pull the old submodule */
            Status = PNIO_sub_pull (PNIO_SINGLE_DEVICE_HNDL,
                                    PDRV_API,
                                    &Addr);         /* location (slot, subslot) */
            if (Status == PNIO_OK)
            {   /* second step: plug the new submodule */
                m_uPDRVTelegram = p_ptValues->o2[0];
                if(m_uPDRVTelegram == PDRV_SUBMOD_ID_TLG1)
                {
                	InputDataLen = 4;
                	OutputDataLen = 4;
                }
                if(m_uPDRVTelegram == PDRV_SUBMOD_ID_TLG2)
    			{
    				InputDataLen = 8;
    				OutputDataLen = 8;
    			}
                if(m_uPDRVTelegram == PDRV_SUBMOD_ID_TLG3)
    			{
    				InputDataLen = 18;
    				OutputDataLen = 10;
    			}
                Status = PNIO_sub_plug (PNIO_SINGLE_DEVICE_HNDL,
                		                PDRV_API,
    									&Addr,			                    // location (slot, subslot)
    									PDRV_MODULE_ID_MAP,
    									m_uPDRVTelegram,	                // submodule identifier
    									InputDataLen,                       // submodule input data length
    									OutputDataLen,                      // submodule output data length
    									PNIO_IM0_NOTHING,                   // Submodule not supports IM0
    								    (IM0_DATA*)NULL,
    									PNIO_S_GOOD);                       // initial iops value, only for submodules without io data

            }
            if (Status != PNIO_OK)
            {
                uError = PDRV_EV1_OP_STATE;
                PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
            }
            else
            {   /* update data lengths of IO-DATA */
                if (m_uPDRVTelegram == PDRV_STDTLG1)
                {
                    InDatLen[PDRV_STDTLG_SLOT][PDRV_STDTLG_SUBSLOT] = PDRV_STDTLG1_INLEN;
                    OutDatLen[PDRV_STDTLG_SLOT][PDRV_STDTLG_SUBSLOT] = PDRV_STDTLG1_OUTLEN;
                }
                else if (m_uPDRVTelegram == PDRV_STDTLG2)
                {
                    InDatLen[PDRV_STDTLG_SLOT][PDRV_STDTLG_SUBSLOT] = PDRV_STDTLG2_INLEN;
                    OutDatLen[PDRV_STDTLG_SLOT][PDRV_STDTLG_SUBSLOT] = PDRV_STDTLG2_OUTLEN;
                }
                else if (m_uPDRVTelegram == PDRV_STDTLG3)
                {
                    InDatLen[PDRV_STDTLG_SLOT][PDRV_STDTLG_SUBSLOT] = PDRV_STDTLG3_INLEN;
                    OutDatLen[PDRV_STDTLG_SLOT][PDRV_STDTLG_SUBSLOT] = PDRV_STDTLG3_OUTLEN;
                    m_bPDRVIsIsoReq = PDRV_TRUE;
                }
            }
        }
        return uError;
    }

#endif

/*****************************************************************************/
/*  Copyright (C) 2020 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
