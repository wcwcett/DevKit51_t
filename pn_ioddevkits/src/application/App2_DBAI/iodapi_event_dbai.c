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
/*  F i l e               &F: iodapi_event_dbai.c                       :F&  */
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
/*  To use this application example, set #define EXAMPL_DEV_CONFIG_VERSION 1 */
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
#include "usriod_cfg.h"
#include "usrapp_cfg.h"   // example application spec. configuration settings

#if (EXAMPL_DEV_CONFIG_VERSION == 2)
    #include "pniousrd.h"
    #include "iodapi_event.h"
    #include "os.h"
    #include "usriod_im_func.h"
    #include "usriod_cfg.h"
    #include "usriod_dbai.h"
    #include "nv_data.h"
    #include "bspadapt.h"
    #include "PnUsr_Api.h"
    #include "usriod_PE.h"
    #include "usriod_utils.h"

    // *=======================================================
    // *  defines
    // *=======================================================
    #define EXAMPLE_SUBSTITUTE_VALUE    0xab  // example: substituted value for output (data consumer)
    #define EXAMPLE_IN_VALUE            0x11  // example: default input value (data provider)

    #define DEMO_RECORD  "ABCDEFGH"

    // ****** example for special features (must be modified by user)
    //#define EXAMPLE_IPAR_SERVER     0       // 1: support iPar - example data on record index 50, 0: else
    #if EXAMPLE_IPAR_SERVER
        // ***** EXAMPLE IPAR:  upload&storage alarm ********
        #define EXAMPL_IPAR_REC_IND             50                          // free defined by the user
    #endif


    // *=======================================================
    // *  extern  functions
    // *=======================================================

    // *=======================================================
    // *  extern  data
    // *=======================================================

    // *=======================================================
    // *  static  data
    // *=======================================================
    #if  EXAMPLE_IPAR_SERVER
        PNIO_UINT32  Example_iParData [NUMOF_SLOTS][NUMOF_SUBSLOTS]; // 4 byte example data per subslot
    #endif

    // *=======================================================
    // *  public data
    // *=======================================================


    // ***** asynchronous record rd/wr only
    USR_REC_READ_RQ   PnioRecReadRq;
    USR_REC_WRITE_RQ  PnioRecWriteRq;




    // *** Output Data (IO Controller ==> IO Device) and substituted values for output data ***
    PNIO_UINT8     OutData        [NUMOF_SLOTS][NUMOF_SUBSLOTS+1][NUMOF_BYTES_PER_SUBSLOT];
    PNIO_UINT8     OutSubstVal    [NUMOF_SLOTS][NUMOF_SUBSLOTS+1][NUMOF_BYTES_PER_SUBSLOT];
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

    static USRIOD_DBAI_IOCR         UsrIocr;                                // information about input and output CR


    // *----------------------------------------------------------------*
    // *  PNIO_cbf_data_read
    // *
    // *  dummy function in DBA interface, not used..
    // *----------------------------------------------------------------*
    PNIO_IOXS     PNIO_cbf_data_read
            (PNIO_UINT32    DevHndl,        // [in]  device handle
             PNIO_DEV_ADDR  *pAddr,         // [in]  geographical or logical address
             PNIO_UINT32    BufLen,         // [in]  length of the submodule input data
             PNIO_UINT8*    pBuffer,        // [in]  Ptr to data buffer to read from
             PNIO_IOXS      Iops            // [in]  (io controller) provider status
            )
    {
        PNIO_printf ("not used in DBA interface ""\n");
        return (PNIO_NOT_OK);
    }

    // *----------------------------------------------------------------*
    // *  PNIO_cbf_data_write
    // *
    // *  dummy function in DBA interface, not used..
    // *----------------------------------------------------------------*
    PNIO_IOXS    PNIO_cbf_data_write
            (PNIO_UINT32    DevHndl,        // [in]  device handle
             PNIO_DEV_ADDR  *pAddr,         // geographical or logical address
             PNIO_UINT32    BufLen,         // [in]  length of the submodule input data
             PNIO_UINT8     *pBuffer,       // [in,out] Ptr to data buffer to write to
             PNIO_IOXS      Iocs            // [in]  remote (io controller) consumer status
            )
    {
        PNIO_printf ("not used in DBA interface ""\n");
        return (PNIO_NOT_OK);
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
        PNIO_printf ("not used in DBA interface ""\n");
        return (PNIO_NOT_OK);
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
        PNIO_printf ("not used in DBA interface ""\n");
        return (PNIO_NOT_OK);
    }

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

        // **** initialize all to 0 ***
        OsMemSet (&InData,          0,          sizeof (InData));           // io data (input)
        OsMemSet (&InDatLen,        0,          sizeof (InDatLen));         // io data lenth(input)
        OsMemSet (&InDatIops,       PNIO_S_BAD, sizeof (InDatIops));        // local provider status
        OsMemSet (&InDatIocs,       PNIO_S_BAD, sizeof (InDatIocs));        // remote consumer status
        OsMemSet (&InDatIocs_old,   PNIO_S_BAD, sizeof (InDatIocs_old));    // old value of remote consumer status

        OsMemSet (&OutData,         0,          sizeof (OutData));          // io data (output)
        OsMemSet (&OutDatLen,       0,          sizeof (OutDatLen));        // io data length (output)
        OsMemSet (&OutDatIocs,      PNIO_S_BAD, sizeof (OutDatIocs));       // local consumer status
        OsMemSet (&OutDatIops,      PNIO_S_BAD, sizeof (OutDatIops));       // remote provider status
        OsMemSet (&OutDatIops_old,  PNIO_S_BAD, sizeof (OutDatIops_old));   // old value of remote provider status

        // **** set values slot 1:  input module, slot 2: output module ***

       // *** plug submodules at system startup  ****
        for (sublistIndex = 0; sublistIndex < NumOfSubListEntries; sublistIndex++)
        {
            subslotIndex = PnUsr_get_subslot_index(pIoSubList->Slot, pIoSubList->Subslot);
            if (subslotIndex)
            {
                // --- inputs -----------------------------
                InDatLen   [pIoSubList->Slot][subslotIndex]  = (PNIO_UINT16) pIoSubList->InDatLen;

                if ( InDatLen[pIoSubList->Slot][subslotIndex] )
                {
                    InDatIops  [pIoSubList->Slot][subslotIndex]  = PNIO_S_GOOD;
                    InDatIocs  [pIoSubList->Slot][subslotIndex]  = PNIO_S_GOOD;
					IsPlugged  [pIoSubList->Slot][subslotIndex] = PNIO_TRUE;

                    //**** preset values for example modules ****
                    OsMemSet (&InData[pIoSubList->Slot][subslotIndex], 0x11, InDatLen[pIoSubList->Slot][subslotIndex]);
                }

                // --- outputs -----------------------------
                OutDatLen   [pIoSubList->Slot][subslotIndex]  = (PNIO_UINT16) pIoSubList->OutDatLen;

                if  ( OutDatLen[pIoSubList->Slot][subslotIndex] )
                {
                    OutDatIops [pIoSubList->Slot][subslotIndex]  = PNIO_S_GOOD;
                    OutDatIocs [pIoSubList->Slot][subslotIndex]  = PNIO_S_GOOD;
					IsPlugged  [pIoSubList->Slot][subslotIndex] = PNIO_TRUE;

                    // *** preset output  values ***
                    OsMemSet (&OutData[pIoSubList->Slot][subslotIndex], 0x02, OutDatLen[pIoSubList->Slot][subslotIndex]);
                }
            }
            pIoSubList++;
        }

        // *** preset outputsubstitution values ***
        OsMemSet (&OutSubstVal,                 // example value for io output data
                  EXAMPLE_SUBSTITUTE_VALUE ,
                  sizeof (OutData));

        OsMemSet (&UsrIocr, 0, sizeof (UsrIocr));
        UsrIocr.OutCr.ArNum = 1;   // we always use the first AR here...
        UsrIocr.OutCr.IoDir = PNIO_IOCR_TYPE_OUTPUT;
        UsrIocr.InCr.ArNum  = 1;   // we always use the first AR here...
        UsrIocr.InCr.IoDir  = PNIO_IOCR_TYPE_INPUT;
        UsrIocr.IocrValid   = PNIO_FALSE;
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
        PNIO_INT32 SubNum;  // loop var for subslot-number
        PNIO_INT32 IoInd;   // index of an io-data byte in a subslot
        PNIO_UINT8 LocalDiffVal;

        // *---------------------------------------------------------------
        // *  print input data
        // *---------------------------------------------------------------
        for (SlotNum = 0; SlotNum < NUMOF_SLOTS; SlotNum++)         // loop over all slots
        {
            for (SubNum = 0; SubNum < NUMOF_SUBSLOTS; SubNum++) // loop over all subslots
            {
                if (InDatLen[SlotNum][SubNum] != 0)       // data used by io ar == data_len is <> 0??
                {
                    for (IoInd=0; IoInd < InDatLen[SlotNum][SubNum]; IoInd++)
                    {
                        if (DiffVal >= 0)
                        {
                            LocalDiffVal = (PNIO_UINT8)DiffVal;
                            InData[SlotNum][SubNum][IoInd] = InData[SlotNum][SubNum][IoInd] + LocalDiffVal;
                        }
                        else
                        {
                            LocalDiffVal = (PNIO_UINT8)(DiffVal * -1); //make positive
                            InData[SlotNum][SubNum][IoInd] = InData[SlotNum][SubNum][IoInd] - LocalDiffVal;
                        }
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
        PNIO_INT32 SubNum;  // loop var for subslot-number
        PNIO_INT32 IoInd;   // index of an io-data byte in a subslot

        PNIO_INT32 DataAvailable = 0;

        // *---------------------------------------------------------------
        // *  print input data
        // *---------------------------------------------------------------
        for (SlotNum = 0; SlotNum < NUMOF_SLOTS; SlotNum++)         // loop over all slots
        {
            for (SubNum = 0; SubNum < NUMOF_SUBSLOTS; SubNum++) // loop over all subslots
            {
              if (InDatLen[SlotNum][SubNum] != 0)       // data used by io ar == data_len is <> 0??
              {
                DataAvailable = 1;
                PNIO_printf("##DatIn Slot=%d  Sub=%d length=%d : ",
                            SlotNum,                    // slot num
                            SubNum,                     // subslot num
                            InDatLen[SlotNum][SubNum]); // data length, defined in step 7 hw-config

                for (IoInd=0; IoInd < InDatLen[SlotNum][SubNum]; IoInd++)
                {
                    PNIO_printf("0x%02x ", InData[SlotNum][SubNum][IoInd]);
                }
                PNIO_printf("\n");
              }
            }
        }

        // *---------------------------------------------------------------
        // *  print output data
        // *---------------------------------------------------------------
        for (SlotNum = 0; SlotNum < NUMOF_SLOTS; SlotNum++)         // loop over all slots
        {
            for (SubNum = 0; SubNum < NUMOF_SUBSLOTS; SubNum++) // loop over all subslots
            {
              if (OutDatLen[SlotNum][SubNum] != 0)      // data used by io ar == data_len is <> 0??
              {
                DataAvailable = 1;
                PNIO_printf("##DatOut Slot=%d  Sub=%d length=%d : ",
                            SlotNum,                    // slot num
                            SubNum,                     // subslot num
                            OutDatLen[SlotNum][SubNum]);// data length, defined in step 7 hw-config

                for (IoInd=0; IoInd < OutDatLen[SlotNum][SubNum]; IoInd++)
                {
                    PNIO_printf("0x%02X ", OutData[SlotNum][SubNum][IoInd]);
                }
                PNIO_printf("\n");
              }
            }
        }

        if (DataAvailable == 0)
            PNIO_printf("no io data available in any active IOCR\n");

    }

    // *----------------------------------------------------------------*
    // *
    // *  UsrSetIops (slot_num, subslot_num, Iops
    // *----------------------------------------------------------------*
    // *
    // *  sets the IO provider state for the specified slot/subslot
    // *
    // *  Input:       PNIO_UINT32     slot_num,    // [in]  slot number of module
    // *               PNIO_UINT32     subslot_num, // [in]  subslot number of submodule
    // *               PNIO_UINT8      Iops         // [in]  io provider state
    // *
    // *    Output:    ---
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
    PNIO_UINT32 UsrDbai_initiate_data_read(PNIO_VOID)
    {
        PNIO_UINT32 i;
        PNIO_UINT8* pIocrBuf;       // IOCR buffer in (ERTEC-) EDD

        // *** disable stack internal modification of AR data and call of PNIO_cbf_xxx () ***
        if (UsrIocr.IocrValid == PNIO_FALSE)
        {
            return (PNIO_OK);
        }

        // *--------------------------------------------
        // *  lock io buffer
        // *--------------------------------------------
        PNIO_dbai_buf_lock (&UsrIocr.OutCr);  // UsrIocr was filled in PNIO_cbf_ar_ownership_ind()
        pIocrBuf = UsrIocr.OutCr.pBuf;        // set pointer to start of IOCR buffer
        if (pIocrBuf == 0)
        {
            return (PNIO_NOT_OK);
        }

        // *--------------------------------------------
        // *  extract the output data of all modules/submodules from IOCR
        // *  extract IOPS of the remote output modules from IOCR
        // *  extract IOCS of the local input modules from IOCR
        // *--------------------------------------------
        for (i = 0; i < UsrIocr.NumOfSub; i++)
        {
            // *--------------------------------------------
            // *  handle output data of one submodule
            // *--------------------------------------------
            USRIOD_SUBMOD* pSubUsr = UsrIocr.pSubList + i;
            if (pSubUsr->IoProp & PNIO_SUB_PROP_OUT)
            {
               // ***save output data
                OsMemCpy (&OutData[pSubUsr->Slot][pSubUsr->Sub][0],
                          pIocrBuf + pSubUsr->OutDatOffs,
                          pSubUsr->OutDatLen);

                // ***save remote IOPS from PNIO controller
                OutDatIops [pSubUsr->Slot][pSubUsr->Sub] = *(pIocrBuf + pSubUsr->OutIopsOffs);    // provider status (of remote io controller)

                // ***check IOPS for change
                if (OutDatIops_old[pSubUsr->Slot][pSubUsr->Sub] != OutDatIops[pSubUsr->Slot][pSubUsr->Sub])
                {
                   PNIO_printf("new IO controller provider status = 0x%x in slot %d, subslot %d\n",
                               OutDatIops[pSubUsr->Slot][pSubUsr->Sub], pSubUsr->Slot, pSubUsr->Sub);
                   OutDatIops_old [pSubUsr->Slot][pSubUsr->Sub] = OutDatIops[pSubUsr->Slot][pSubUsr->Sub];
                }
            }
            if (pSubUsr->IoProp & PNIO_SUB_PROP_IN)
            {
               // *** save remote IOCS of input module (if needed...)  ***
                InDatIocs [pSubUsr->Slot][pSubUsr->Sub] = *(pIocrBuf + pSubUsr->InIocsOffs);    // remote consumer status of local input module
            }

        }

        PNIO_dbai_buf_unlock (&UsrIocr.OutCr);  // UsrIocr was filled in PNIO_cbf_ar_ownership_ind()
        return (PNIO_OK);
    }

    PNIO_UINT32 UsrDbai_initiate_data_write()
    {
        PNIO_UINT32 i;
        PNIO_UINT8* pIocrBuf;       // IOCR buffer in (ERTEC-) EDD

        // *** disable stack internal modification of AR data and call of PNIO_cbf_xxx () ***
        if (UsrIocr.IocrValid == PNIO_FALSE)
        {
            return (PNIO_OK);
        }

        // *--------------------------------------------
        // *  lock io buffer
        // *--------------------------------------------
        PNIO_dbai_buf_lock (&UsrIocr.InCr);   // UsrIocr was filled in PNIO_cbf_ar_info_ind()
        pIocrBuf = UsrIocr.InCr.pBuf;         // set pointer to start of IOCR buffer
        if (pIocrBuf == 0)
        {
            return (PNIO_OK);
        }



        // *--------------------------------------------
        // *  copy  the input data of all modules/submodules into IOCR data buffer
        // *  set IOPS of the local input modules in IOCR
        // *  set IOCS of the local output modules in IOCR
        // *--------------------------------------------
        for (i = 0; i < UsrIocr.NumOfSub; i++)
        {
            // *--------------------------------------------
            // *  handle output data of one submodule
            // *--------------------------------------------
            USRIOD_SUBMOD* pSubUsr = UsrIocr.pSubList + i;
			if (PNIO_TRUE == IsPlugged[pSubUsr->Slot][pSubUsr->Sub])
			{
				if (pSubUsr->IoProp & PNIO_SUB_PROP_IN)
				{
					// *** copy input data into IOCR
					OsMemCpy(pIocrBuf + pSubUsr->InDatOffs,
						&InData[pSubUsr->Slot][pSubUsr->Sub][0],
						pSubUsr->InDatLen);

					// ***set  local IOPS
					*(pIocrBuf + pSubUsr->InIopsOffs) = InDatIops[pSubUsr->Slot][pSubUsr->Sub];    // provider status of local input module
				}
				else
				{   // *--------------------------------------------
					// *  handle submodules without IO data
					// *  (e.g. DAP, PDEV)
					// *--------------------------------------------
					if (pSubUsr->IoProp == PNIO_SUB_PROP_NO_DATA)
					{
						// *** update provider state for this submodules ***
						*(pIocrBuf + pSubUsr->InIopsOffs) = PNIO_S_GOOD; // simple example: we assume, they are always good..
					}
				}


				// *** if subslot has output data, update (output-)consumer state in input frame ***
				if (pSubUsr->IoProp & PNIO_SUB_PROP_OUT)
				{  // ** submod has out put data ?? **
					*(pIocrBuf + pSubUsr->OutIocsOffs) = OutDatIocs[pSubUsr->Slot][pSubUsr->Sub];    // provider status of local input module
				}
			}
			else  /* Submodule not plugged -> PNIO_S_BAD*/
			{
				if (pSubUsr->IoProp & PNIO_SUB_PROP_IN)
				{
					OsMemSet(pIocrBuf + pSubUsr->InDatOffs, 0, pSubUsr->InDatLen);
					*(pIocrBuf + pSubUsr->InIopsOffs) = PNIO_S_BAD;
				}
				if(pSubUsr->IoProp & PNIO_SUB_PROP_OUT)
				{
					*(pIocrBuf + pSubUsr->OutIocsOffs) = PNIO_S_BAD;
				}
				if(pSubUsr->IoProp == PNIO_SUB_PROP_NO_DATA)
				{
					*(pIocrBuf + pSubUsr->InIopsOffs) = PNIO_S_BAD;
				}
			}

        }

        PNIO_dbai_buf_unlock (&UsrIocr.InCr);  // UsrIocr was filled in PNIO_cbf_ar_ownership_ind()
        return (PNIO_OK);
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
        PNIO_UINT32 ReportHostIP = HostIp;
        PNIO_UINT8* u8ReportHostIP = (PNIO_UINT8*)&ReportHostIP;

        PNIO_UNUSED_ARG (DevHndl);
        NumOfAr++;      // increment number of running ARs

        PNIO_printf("##CONNECT_IND AR=%d AR type=%d sendclock=%d, reduction_ratio_in=%d, reduction_ratio_out=%d, sessionKey=%d, hostIP=%03d.%03d.%03d.%03d\n",
                    ArNum,
                    ArType,
                    SendClock,
                    RedRatioIocrIn, RedRatioIocrOut,
                    ArSessionKey,
                    *(u8ReportHostIP + 0), *(u8ReportHostIP + 1), *(u8ReportHostIP + 2), *(u8ReportHostIP + 3));
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
    // *  Input:    DevHndl        Device handle
    // *            ArNum          AR number (1...N)
    // *            pOwnSubList    pointer to list of expected submodules
    // *
    // *  Output:  PNIO_EXP*   pOwnSub->OwnSessionKey   set to 0 only if ownership is rejected
    // *			                                    else keep unchanged
    // *                       pOwnSub->IsWrongSubmod   set to PNIO_TRUE only if wrong module,
    // *			                                    else keep unchanged
    // *
    // *----------------------------------------------------------------*
    PNIO_VOID   PNIO_cbf_ar_ownership_ind
            (
                PNIO_UINT32     DevHndl,
                PNIO_UINT32     ArNum,          // AR number 1....NumOfAR
                PNIO_EXP*       pOwnSubList     // [in] expected configuration in ownership indication
            )
    {
        PNIO_UINT32 i;

        PNIO_UNUSED_ARG (DevHndl);

        PNIO_printf("##OWNERSHIP_IND AR = %d number of submodules = %d\n",
                    ArNum, pOwnSubList->NumOfPluggedSub);

        // *-------------------------------------
        // * if no submoduls included --> return
        // *-------------------------------------
        if (pOwnSubList->NumOfPluggedSub == 0)
            return;

        // *-------------------------------------
        // * alloc mem for list of submoduls
        // *-------------------------------------
        OsAlloc ((void**)&UsrIocr.pSubList, 0, sizeof(USRIOD_SUBMOD) * pOwnSubList->NumOfPluggedSub);
        UsrIocr.NumOfSub = pOwnSubList->NumOfPluggedSub;

        // *-------------------------------------
        // * save data-offset-, ioxs-offset-
        // * and data length values in the
        // * user data for later IO data exchange
        // *-------------------------------------
        for (i = 0; i < UsrIocr.NumOfSub; i++)
        {
            USRIOD_SUBMOD* pSubUsr = UsrIocr.pSubList + i;
            PNIO_EXP_SUB*  pSubExp = &pOwnSubList->Sub[i];

            pSubUsr->Api         = pSubExp->ApiNum;
            pSubUsr->Slot        = pSubExp->SlotNum;
            pSubUsr->Sub         = pSubExp->SubNum;
            pSubUsr->InDatOffs   = pSubExp->In.data_offset;
            pSubUsr->OutDatOffs  = pSubExp->Out.data_offset;
            pSubUsr->InDatLen    = pSubExp->In.data_length;
            pSubUsr->OutDatLen   = pSubExp->Out.data_length;
            pSubUsr->InIopsOffs  = pSubExp->In.iops_offset;
            pSubUsr->InIocsOffs  = pSubExp->In.iocs_offset;
            pSubUsr->OutIopsOffs = pSubExp->Out.iops_offset;
            pSubUsr->OutIocsOffs = pSubExp->Out.iocs_offset;
            pSubUsr->IoProp      = pSubExp->IoProp;

            PNIO_printf("  Api=%d Slot=%d Sub=%d ModID=%d SubID=%d OwnSessKey=%d isWrong=%d\n",
                        pSubExp->ApiNum,
                        pSubExp->SlotNum,
                        pSubExp->SubNum,
                        pSubExp->ModIdent,
                        pSubExp->SubIdent,
                        pSubExp->OwnSessionKey,
                        pSubExp->IsWrongSubmod);

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

       }
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
        if (SubslotNum == 0)
        { // ****param end for all submodules ****
            PNIO_printf("##PARAM END  for all submodules, ArNum=%d, Session=%d \n",
                         ArNum, SessionKey);
        }
        else
        { // ****param end for one submodule ****
            PNIO_printf("##PARAM END for Api=%d, Slot=%d, Sub=%d, ArNum=%d, Session=%d \n",
                         Api, SlotNum, SubslotNum, ArNum, SessionKey);
        }

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
        UsrIocr.IocrValid   = PNIO_TRUE;

        PNIO_dbai_enter();
        Status =  UsrDbai_initiate_data_read ();
		if (Status != PNIO_OK)
		{
			PNIO_printf ( "## UsrDbai_initiate_data_read returned %d which is NOT OK - ARnum=%d\n", Status, ArNum);
		}
			
        Status =  UsrDbai_initiate_data_write ();
		if (Status != PNIO_OK)
		{
			PNIO_printf ( "## UsrDbai_initiate_data_write returned %d which is NOT OK - ARnum=%d\n", Status, ArNum);
		}
		
        PNIO_dbai_exit();

        PnUsr_ActivateIoDatXch();

	    // *-----------------------------------------------------------------------
        // *    notify, if first "readyForInputUpdate" after AR start or
        // *    because of replugging a submodule.
	    // *-----------------------------------------------------------------------
        if (InpUpdState == PNIO_AR_STARTUP)
    		PNIO_printf ( "##READY FOR INPUT UPDATE DURING AR_STARTUP ARnum=%d\n", ArNum);
        else
    		PNIO_printf ( "##READY FOR INPUT UPDATE DURING AR INDATA  ARnum=%d\n", ArNum);
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
        PNIO_printf("##AR IN-Data event indication received, ArNum = %xh, Session = %xh\n",
                    ArNum, SessionKey);
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
    // *     Output:   ---
    // *
    // *----------------------------------------------------------------*
    void PNIO_cbf_ar_disconn_ind     (PNIO_UINT32    DevHndl,
                                      PNIO_UINT16    ArNum,
                                      PNIO_UINT16    SessionKey,
                                      PNIO_AR_REASON ReasonCode)
    {
        PNIO_UNUSED_ARG (DevHndl);
        PNIO_printf("##AR Offline indication ,ArNum = %xh, Session = %xh, Reason = %xh\n",
                    ArNum, SessionKey, ReasonCode);

        UsrIocr.IocrValid   = PNIO_FALSE;

        if (NumOfAr)
            NumOfAr--;      // decrement number of running ARs

    }



    // *****************************************************************************************
    // *************************** ALARM INDICATIONS  ******************************************
    // *****************************************************************************************
    void    PNIO_cbf_dev_alarm_ind (PNIO_UINT32         DevHndl,
                                    PNIO_DEV_ALARM_DATA *pAlarm)
    {
        PNIO_printf("##APDU alarm indication, Dev=%d, Slot=%d, Subslot=%d, AR=%d, Sess=%d\n",
                    DevHndl,
                    pAlarm->SlotNum,
                    pAlarm->SubslotNum,
                    pAlarm->ArNum,
                    pAlarm->SessionKey);

        PNIO_printf("  Prio=%xh, Type=%xh, DiagCH = %d, DiagGen=%d, Tag=%d, DatLen=%d\n",
                    pAlarm->AlarmPriority,
                    pAlarm->AlarmType,
                    pAlarm->DiagChannelAvailable,
                    pAlarm->DiagGenericAvailable,
                    pAlarm->UserStructIdent,
                    pAlarm->UserAlarmDataLength);

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
      PNIO_printf("Asynchronous request ArNum=%d Alarmtype=%d Api=%d Slot=%d Subslot=%d Status = %x\n",
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
        PNIO_UINT32   Status        = PNIO_OK;

        PNIO_UINT32 slot_nr         = pAddr->Geo.Slot;
        PNIO_UINT32 subslot_nr      = pAddr->Geo.Subslot;
        PNIO_UINT32 DoHexdumpData   = PNIO_FALSE;

        PNIO_UNUSED_ARG (DevHndl);
        PNIO_UNUSED_ARG (Api);
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
                        PNIO_printf("##READ IO INPUT DATA, Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
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
                        PNIO_printf("##READ IO OUTPUT DATA and SUBSTITUTED VALUES, Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
                                    Api, slot_nr, subslot_nr, *pBufLen, SequenceNum);
						if (pAddr->Geo.Slot != 0)   // not DAP --> normal input, output or input/output modules
						{
							Status = RecOutDatObjElement_Handler
										 ( ArNum,                                                 // AR number
										   Api,                                                   // api number
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
										 ( ArNum,                                                 // AR number
										   Api,                                                   // api number
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
            // *  user defined records  index 0..7fffh
            // *----------------------------------------------*
            case 2: // **** user specific demo record *****
                    PNIO_printf("##READ Demo-Record , Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
                                 Api, slot_nr, subslot_nr, *pBufLen, SequenceNum);

                    if (*pBufLen > sizeof(DEMO_RECORD))
                        *pBufLen = sizeof (DEMO_RECORD);

                    OsMemCpy (pBuffer, DEMO_RECORD, *pBufLen);
                    break;

            case 1: // **** user specific startup record *****
                    PNIO_printf("##READ startup-Record (here: demo record is returned), Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
                                 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen, SequenceNum);
                    if (*pBufLen > sizeof(DEMO_RECORD))
                        *pBufLen = sizeof (DEMO_RECORD);

                    OsMemCpy (pBuffer, DEMO_RECORD, *pBufLen);
                    break;

            #if EXAMPLE_IPAR_SERVER
                case EXAMPL_IPAR_REC_IND: // **** read example data for the iPar Server *****

                         *pBufLen = 4;
                         OsMemCpy ( pBuffer,
                                    &Example_iParData[slot_nr][subslot_nr],
                                    *pBufLen);
                         PNIO_printf("##READ iPar example data, Api=%d Slot=%d Subslot=%d Len=%d, RecordIndex=%d\n",
                                      Api, slot_nr, subslot_nr, *pBufLen, RecordIndex);
                         DoHexdumpData = PNIO_TRUE;
                     break;
            #endif

            default:
                // *----------------------------------------------*
                // *  all record number < 0x7fff are user specific,
                // *  but we intercept the simatic specific rec-read
                // *  request on slot=0, subslot=1, index={0,1}, which
                // *  is not supported here.
                // *----------------------------------------------*
                 if ( RecordIndex <= 0x7fff)
                 {
                    if (    ( (pAddr->Geo.Slot == 0) && (pAddr->Geo.Subslot == 1))
                         || ( pAddr->Geo.Subslot >= 200)
                       )
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

                        // **** copy dummy data into the buffer, set data-size ***

                        if (*pBufLen > sizeof (ReadRecDummyData))
                        {
                           *pBufLen = sizeof (ReadRecDummyData);
                        }
                        OsMemCpy (pBuffer,
                                ReadRecDummyData,
                                *pBufLen);
                        PNIO_printf("##READ_REC RQ,  Api =%d Slot=%d Subslot=%d Index=0x%x, Len=%d, Sequ_nr=%d\n",
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
                    PNIO_printf("##READ_REC RQ, Api=%d Slot=%d Subslot=%d Index=0x%x, Len=%d, Sequ_nr=%d\n",
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
            PNIO_printf("##REC_DATA = ");
            for (i=0; i < *pBufLen; i++)
            {
                PNIO_UINT32  Val;
                Val = (PNIO_UINT32 ) *(p8+i);;
                PNIO_printf("0x%02x ", Val);
            }

            PNIO_printf("\n\n");
        }



      return (Status);
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
        // **** copy dummy data into the buffer, set data-size ***
        PNIO_UINT8      WriteRecDummyData[50];
        PNIO_UINT32     i;
        PNIO_UINT32     Status = PNIO_OK;
        PNIO_UINT32     DoHexdumpData = PNIO_TRUE;

        PNIO_UNUSED_ARG (DevHndl);
        PNIO_UNUSED_ARG (Api);
        PNIO_UNUSED_ARG (ArNum);
        PNIO_UNUSED_ARG (SessionKey);
        PNIO_UNUSED_ARG (SequenceNum);

        // *** check data size (accepted data >= provided data ??) ***
        if (*pBufLen > sizeof (WriteRecDummyData))
           Status = PNIO_NOT_OK;

        switch (RecordIndex)
        {
            // *----------------------------------------------*
            // *  user defined records  index 0..7fffh
            // *----------------------------------------------*
            #if EXAMPLE_IPAR_SERVER
                case EXAMPL_IPAR_REC_IND: // **** read example data for the iPar Server *****
                        PNIO_printf("##download iPar example, Api=%d Slot=%d Subslot=%d Len=%d, RecordIndex=%d\n",
                                     Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen, RecordIndex);

                        // ***** save record data for the specified slot/subslot ****
                        OsMemCpy (&Example_iParData[pAddr->Geo.Slot][pAddr->Geo.Subslot],
                                  pBuffer,
                                  4);
                     break;
            #endif
            case 2: // **** user specific demo record *****
                    PNIO_printf("##WRITE Demo-Record , Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
                                 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen, SequenceNum);
                    break;

            case 1: // **** user specific startup record *****
                    PNIO_printf("##WRITE startup-Record from GSD file , Api=%d Slot=%d Subslot=%d Len=%d, Sequ_nr=%d\n",
                                 Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, *pBufLen, SequenceNum);
                    break;
            default:
                 Status = PNIO_NOT_OK;
                 break;
        }



        if (Status == PNIO_OK)
        {
            // *** copy the record data into a buffer for further use **
            OsMemCpy (WriteRecDummyData,        // destination pointer for record data
                    pBuffer,                // source pointer for record data
                    *pBufLen);              // length of the accepted data


            // **** print data as hexdump ****
            if (DoHexdumpData)
            {
                PNIO_printf("##WRITE_REC RQ, Api=%d Slot=%d Subslot=%d Index=0x%x, Len=%d, Sequ_nr=%d\n",
                             Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, RecordIndex, *pBufLen, SequenceNum);

                PNIO_printf("##REC_DATA = ");
                for (i=0; i < *pBufLen; i++)
                {
                    PNIO_UINT32  Val;
                    Val = (PNIO_UINT32 ) WriteRecDummyData[i];
                    PNIO_printf("0x%02x ", Val);
                }

                PNIO_printf("\n\n");
            }
        }
        else
        {// *** if an error occured, it must be specify  according to IEC 61158-6
            pPnioState->ErrCode   = 0xdf;  // IODWriteRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb6;  // example: Error Class 11 = Access, ErrorNr 6 = "access denied"
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
           PNIO_printf("##Save remanent: Station Name = %.*s  received, Length=%d  Remanent = %d\n",
                       NameLength, pStationName, NameLength, Remanent);

          // **** store data in non volatile memory ****
          Bsp_nv_data_store (PNIO_NVDATA_DEVICENAME,    // nv data type: device name
                             pStationName,              // source pointer to the devicename
                             NameLength);               // length of the device name
       }

       else
       {
           PNIO_printf("Station Name = %.*s  received, Length=%d  Remanent = %d\n",
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
        PNIO_printf("##LED Blink START, Port = %d, frequency = %d Hz \n", PortNum, frequency);
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
        PNIO_printf("##LED Blink STOP, Port = %d\n", PortNum);
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
//    InDatLen[pAddr->ad.Geo1.Slot][pAddr->ad.Geo1.Subslot] = InputDataLen;
//    OutDatLen[pAddr->ad.Geo1.Slot][pAddr->ad.Geo1.Subslot] = OutputDataLen;

//    IsPlugged[pAddr->ad.Geo1.Slot][pAddr->ad.Geo1.Subslot] = PNIO_TRUE;


    //suzhen 20231214
    PNIO_UINT32 subslotIndex = PnUsr_get_subslot_index(pAddr->ad.Geo1.Slot,pAddr->ad.Geo1.Subslot);
    if (subslotIndex)
    {
      InDatLen[pAddr->ad.Geo1.Slot][subslotIndex]  = (PNIO_UINT16)InputDataLen;
      OutDatLen[pAddr->ad.Geo1.Slot][subslotIndex] = (PNIO_UINT16)OutputDataLen;

      IsPlugged[pAddr->ad.Geo1.Slot][subslotIndex] = PNIO_TRUE;
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

    IsPlugged[pAddr->ad.Geo1.Slot][pAddr->ad.Geo1.Subslot] = PNIO_FALSE;
}

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
