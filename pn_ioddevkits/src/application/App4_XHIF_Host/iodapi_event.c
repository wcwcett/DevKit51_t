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
/*  F i l e               &F: iodapi_event.c                            :F&  */
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
* @file     iodapi_event.c
* @brief    PNIO event handler for this application example
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

#define EXAMPLE_IN_VALUE            0x11  // example: default input value (data provider)

    /* *** Output Data (IO Controller ==> IO Device) and substituted values for output data *** */
    PNIO_UINT8     OutData        [PNPB_XHIF_NUMOF_SLOTS][PNPB_XHIF_NUMOF_SUBSLOTS+1][PNPB_XHIF_NUMOF_BYTES_PER_SUBSLOT];
#if !IOD_INCLUDE_REC8028_8029
    PNIO_UINT8     OutSubstVal    [PNPB_XHIF_NUMOF_SLOTS][PNPB_XHIF_NUMOF_SUBSLOTS+1][PNPB_XHIF_NUMOF_BYTES_PER_SUBSLOT];
#endif
    PNIO_UINT16    OutDatLen      [PNPB_XHIF_NUMOF_SLOTS][PNPB_XHIF_NUMOF_SUBSLOTS+1];
    PNIO_UINT8     OutDatIocs     [PNPB_XHIF_NUMOF_SLOTS][PNPB_XHIF_NUMOF_SUBSLOTS+1];
    PNIO_UINT8     OutDatIops     [PNPB_XHIF_NUMOF_SLOTS][PNPB_XHIF_NUMOF_SUBSLOTS+1];
    PNIO_UINT8     OutDatIops_old [PNPB_XHIF_NUMOF_SLOTS][PNPB_XHIF_NUMOF_SUBSLOTS+1];

    /* *** Input Data  (IO Device ==> IO Controller) *** */
    PNIO_UINT8     InData         [PNPB_XHIF_NUMOF_SLOTS][PNPB_XHIF_NUMOF_SUBSLOTS+1][PNPB_XHIF_NUMOF_BYTES_PER_SUBSLOT];
    PNIO_UINT16    InDatLen       [PNPB_XHIF_NUMOF_SLOTS][PNPB_XHIF_NUMOF_SUBSLOTS+1];
    PNIO_UINT8     InDatIops      [PNPB_XHIF_NUMOF_SLOTS][PNPB_XHIF_NUMOF_SUBSLOTS+1];
    PNIO_UINT8     InDatIocs      [PNPB_XHIF_NUMOF_SLOTS][PNPB_XHIF_NUMOF_SUBSLOTS+1];
    PNIO_UINT8     InDatIocs_old  [PNPB_XHIF_NUMOF_SLOTS][PNPB_XHIF_NUMOF_SUBSLOTS+1];

    PNIO_UINT8  LastPulledSlot;
    PNIO_UINT8  LastPulledSubslot;



/**
 *  @brief Return index of subslot
 *
 *  @param[in]      slot                    Slot number
 *  @param[in]      subslot                 Subslot number
 *
 *  @return         Position of subslot
 *
 *  Solves conversion of Node and Port subslot => special handling for slot 0 subslots
 *
 */
PNIO_UINT32 PnUsr_get_subslot_index(PNIO_UINT32 slot, PNIO_UINT32 subslot)
{
    if (slot) /* PERI */
    {
        return subslot;
    }
    else /* HEAD */
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
}   /* PnUsr_get_subslot_index */


/**
 *  @brief Initialize static variables
 *
 *  @param[in]      *pIoSubList             Plugged submodules, including PDEV
 *  @param[in]      NumOfSublistEntries     Number of entries in pPioSubList
 *
 *  @return         void
 *
 *  Must be called first during startup
 *
 */
//lint -e{832} Parameter 'Symbol' not explicitly declared, int assumed
void PnUsr_cbf_iodapi_event_varinit (PNIO_SUB_LIST_ENTRY* pIoSubList,
                                     PNIO_UINT32          NumOfSubListEntries)
{
    PNIO_UINT32 sublistIndex;
    PNIO_UINT32 subslotIndex;
    PNIO_UINT32 i;

    memset (&InData[0][0][0],       EXAMPLE_IN_VALUE,   sizeof (InData));           // io data (input)
    memset (&InDatIops[0][0],       PNIO_S_GOOD,        sizeof (InDatIops));        // local provider status
    memset (&InDatIocs[0][0],       PNIO_S_GOOD,        sizeof (InDatIocs));        // remote consumer status
    memset (&InDatIocs_old[0][0],   PNIO_S_GOOD,        sizeof (InDatIocs_old));    // old value of remote consumer status

    memset (&OutData[0][0][0],      0,                  sizeof (OutData));          // io data (output)
    memset (&OutDatIocs[0][0],      PNIO_S_GOOD,        sizeof (OutDatIocs));       // local consumer status
    memset (&OutDatIops[0][0],      PNIO_S_GOOD,        sizeof (OutDatIops));       // remote provider status
    memset (&OutDatIops_old[0][0],  PNIO_S_GOOD,        sizeof (OutDatIops_old));   // old value of remote provider status

    LastPulledSlot = 0;
    LastPulledSubslot = 0;

    for (sublistIndex = 0; sublistIndex < NumOfSubListEntries; sublistIndex++)
    {
        subslotIndex = PnUsr_get_subslot_index(pIoSubList->Slot, pIoSubList->Subslot);
        if (subslotIndex)
        {
            /* --- inputs ----------------------------- */
            InDatLen   [pIoSubList->Slot][subslotIndex]  = (PNIO_UINT16) pIoSubList->InDatLen;
            /* --- outputs ----------------------------- */
            OutDatLen   [pIoSubList->Slot][subslotIndex]  = (PNIO_UINT16) pIoSubList->OutDatLen;

            /* IOxS init */
            memset (&OutDatIocs[pIoSubList->Slot][subslotIndex], PNIO_S_GOOD, sizeof (OutDatIocs));       // local consumer status
        }
        pIoSubList++;
    }

    /* Init storage for last Apdu statuses */
    for(i = 0; i < PNPB_XHIF_MAX_NUM_OF_AR; i++)
    {
        pnpb_ar_apdu_status[i] = 0;
    }

}   /* PnUsr_cbf_iodapi_event_varinit */


/**
 *  @brief Print IO data
 *
 *  @param[in]      void
 *
 *  @return         void
 *
 *  All data in active IOCRs to console
 *
 */
void PrintAllUsedIoData (void)
{
    PNIO_INT32 SlotNum; /* loop var for slot-number */
    PNIO_INT32 SubNum;  /* loop var for subslot-number */
    PNIO_INT32 IoInd;   /* index of an io-data byte in a subslot */

    PNIO_INT32 DataAvailable = 0;

    /* *--------------------------------------------------------------- */
    /* *  print input data                                              */
    /* *--------------------------------------------------------------- */
    for (SlotNum = 0; SlotNum < PNPB_XHIF_NUMOF_SLOTS; SlotNum++)       /* loop over all slots */
    {
        for (SubNum = 0; SubNum < PNPB_XHIF_NUMOF_SUBSLOTS; SubNum++)   /* loop over all subslots */
        {
          if (InDatLen[SlotNum][SubNum] != 0)       /* data used by io ar == data_len is <> 0?? */
          {
            DataAvailable = 1;
            printf ("##DatIn Slot=%d  Sub=%d length=%d : ",
                    SlotNum,                    /* slot num */
                    SubNum,                     /* subslot num */
                    InDatLen[SlotNum][SubNum]); /* data length, defined in step7/TIA hw-config */

            for (IoInd=0; IoInd < InDatLen[SlotNum][SubNum]; IoInd++)
            {
                printf ("0x%02x ", InData[SlotNum][SubNum][IoInd]);
            }
            printf ("\n");
          }
        }
    }

    /* *--------------------------------------------------------------- */
    /* *  print output data                                             */
    /* *--------------------------------------------------------------- */
    for (SlotNum = 0; SlotNum < PNPB_XHIF_NUMOF_SLOTS; SlotNum++)       /* loop over all slots */
    {
        for (SubNum = 0; SubNum < PNPB_XHIF_NUMOF_SUBSLOTS; SubNum++)   /* loop over all subslots */
        {
          if (OutDatLen[SlotNum][SubNum] != 0)      /* data used by io ar == data_len is <> 0?? */
          {
            DataAvailable = 1;
            printf("##DatOut Slot=%d  Sub=%d length=%d : ",
                    SlotNum,                        /* slot num */
                    SubNum,                         /* subslot num */
                    OutDatLen[SlotNum][SubNum]);    /* data length, defined in step 7 hw-config */

            for(IoInd=0; IoInd < OutDatLen[SlotNum][SubNum]; IoInd++)
            {
                printf("0x%02X ", OutData[SlotNum][SubNum][IoInd]);
            }
            printf("\n");
          }
        }
    }

    if (DataAvailable == 0)
        printf("no io data available in any active IOCR\n");

}   /* PrintAllUsedIoData */

PNIO_VOID PrintSubmodules( PNIO_VOID )
{
	PNIO_UINT32 i;

    printf( "Plugged Submodules on device \n" );
    for (i = 0; i < pnpb_dev_params.NoOfSubmodules; i++)
	{

		printf ("%02d Slot=%02d Sub=%04x ModID=%02x \n",
				  i,
				  pnpb_submodule_params[i]->Slot,
				  pnpb_submodule_params[i]->Subslot,
				  pnpb_submodule_params[i]->ModId
	             );
	}

}

/**
 *  @brief Change value in all IO data
 *
 *  @param[in]      DiffVal             Increment/Decrement of how much - this value will be added to current value. Signed!
 *
 *  @return         void
 *
 *  Increment/decrement according to param value
 *
 */
void ChangeAllInputData (PNIO_INT8 DiffVal)
{
    PNIO_INT32 SlotNum;// loop var for slot-number
    PNIO_INT32 SubNum;  // loop var for subslot-number
    PNIO_INT32 IoInd;   // index of an io-data byte in a subslot
    PNIO_UINT8 LocalDiffVal;

    // *---------------------------------------------------------------
    // *  print input data
    // *---------------------------------------------------------------
    for (SlotNum = 0; SlotNum < PNPB_XHIF_NUMOF_SLOTS; SlotNum++)         /* loop over all slots */
    {
	    for (SubNum = 0; SubNum < PNPB_XHIF_NUMOF_SUBSLOTS; SubNum++) /* loop over all subslots */
	    {
	    	if (InDatLen[SlotNum][SubNum] != 0)     /* data used by io ar == data_len is <> 0?? */
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
}   /* ChangeAllInputData */

/**
 *  @brief Sets the IO provider state for the specified slot/subslot
 *
 *  @param[in]      slot_num                Slot number of module
 *  @param[in]      subslot_num             Subslot number of submodule
 *  @param[in]      Iops                    Io provider state
 *
 *  @return         void
 *
 */
void UsrSetIops (PNIO_UINT32 SlotNum, PNIO_UINT32 SubslotNum, PNIO_UINT8 Iops)
{
    InDatIops [SlotNum][SubslotNum] = Iops;
}   /* UsrSetIops */


/**
 *  @brief Input data from the application to the stack
 *
 *  @param[in]      *pAddr                  Location (slot, subslot)
 *  @param[in]      data_len                Data length
 *  @param[in, out] *pBuffer                Pointer to the input data
 *  @param[in]      Iocs                    (Io controller) consumer status
 *
 *  @return                                 Io-provider-status  [IOCS_STATE_GOOD, IOCS_STATE_BAD]
 *
 * The application reads the data from the specified input module
 *  and handles them to the stack.
 *
 */
PNIO_IOXS    PNIO_cbf_data_write
        (PNIO_DEV_ADDR  *pAddr,
         PNIO_UINT32    BufLen,
         PNIO_UINT8     *pBuffer,
         PNIO_IOXS      Iocs)
{
    PNIO_UINT32 slot_num = 0;
    PNIO_UINT32 subslot_num = 0;
    
    if(NULL != pAddr)
    {
        //lint --e{415} access out-of-bound pointer ('Integer' beyond end of data) by operator 'String'
        //pAddr->Geo.Slot and pAddr->Geo.Subslot are already allocated and initialized
        slot_num    = pAddr->Geo.Slot;
        subslot_num = pAddr->Geo.Subslot;
    }
    else
    {
        printf("Error: Null pointer\n");
    }

    /* Perform memcpy only if there are data to be copied */
    if((0 < BufLen) && (NULL != pBuffer))
    {
        //lint -e{422} Passing to function 'Symbol' a negative value
        //BufLen is already checked if it is bigger than 0
        memcpy(pBuffer, &InData[slot_num][subslot_num][0], BufLen);
    }
    InDatIocs [slot_num][subslot_num] = Iocs;    /* consumer status (of remote io controller) */

    /* *---- check provider status and notify modifications ------* */
    if (InDatIocs_old [slot_num][subslot_num] != Iocs)
    {
         printf("new IO controller input consumer status (ICS) = 0x%x in slot %d, subslot %d\n",
                  Iocs, slot_num, subslot_num);
         InDatIocs_old [slot_num][subslot_num] = Iocs;
    }
    return((PNIO_IOXS)InDatIops [slot_num][subslot_num]); // return local provider state
}   /* PNIO_cbf_data_write */


/**
 *  @brief Output data from the stack to the application
 *
 *  @param[in]      *pAddr                  Location (slot, subslot)
 *  @param[in]      data_len                Data length
 *  @param[in]      *pBuffer                Pointer to the output data
 *  @param[in]      Iops                    (Io controller) provider status
 *
 *  @return                                 Io-consumer-status  [IOCS_STATE_GOOD, IOCS_STATE_BAD]
 *
 * The application takes the data and writes them to the specified
 *  output module.
 *
 */
PNIO_IOXS     PNIO_cbf_data_read
        (PNIO_DEV_ADDR  *pAddr,
         PNIO_UINT32    BufLen,
         PNIO_UINT8*    pBuffer,
         PNIO_IOXS      Iops)
{
    PNIO_UINT32 slot_num = 0;
    PNIO_UINT32 subslot_num = 0;
    
    if(NULL != pAddr)
    {
        //lint --e{415} access out-of-bound pointer ('Integer' beyond end of data) by operator 'String'
        //pAddr->Geo.Slot and pAddr->Geo.Subslot are already allocated and initialized
        slot_num    = pAddr->Geo.Slot;
        subslot_num = pAddr->Geo.Subslot;
    }
    else
    {
        printf("Error: Null pointer\n");
    }
    /* save last data block */
    if((0 < BufLen) && (NULL != pBuffer))
    {
        //lint -e{422} Passing to function 'Symbol' a negative value
        //BufLen is already checked if it is bigger than 0
        memcpy(&OutData[slot_num][subslot_num][0], pBuffer, BufLen);
    }

    OutDatIops [slot_num][subslot_num] = Iops;    /* provider status (of remote io controller) */

    /* *---- check provider status and notify modifications ------* */
    if (OutDatIops_old [slot_num][subslot_num] != Iops)
    {
        printf("new IO controller output provider status (OPS) = 0x%x in slot %d, subslot %d\n",
                       Iops, slot_num, subslot_num);
        OutDatIops_old [slot_num][subslot_num] = Iops;
    }

    return((PNIO_IOXS)OutDatIocs [slot_num][subslot_num]);    /* consumer state (of local io device) */
}   /* PNIO_cbf_data_read */


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
	    (PNIO_DEV_ADDR	*pAddr,
	     PNIO_IOXS	    Iocs
        )
{
    PNIO_UINT32 slot_num = 0;
    PNIO_UINT32 subslot_num = 0;
    
    if(NULL != pAddr)
    {
        //lint --e{415} access out-of-bound pointer ('Integer' beyond end of data) by operator 'String'
        //pAddr->Geo.Slot and pAddr->Geo.Subslot are already allocated and initialized
        slot_num    = pAddr->Geo.Slot;
        subslot_num = pAddr->Geo.Subslot;
    }
    else
    {
        printf("Error: Null pointer\n");
    }

    InDatIocs [slot_num][subslot_num] = Iocs;    // consumer status (of remote io controller)

    // *---- check provider status and notify modifications ------*
    if (InDatIocs_old [slot_num][subslot_num] != Iocs)
    {
         printf("new IO controller input consumer status (ICS) = 0x%x in slot %d, subslot %d\n",
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
	    (PNIO_DEV_ADDR	*pAddr,
	     PNIO_IOXS	    Iops
        )
{
    PNIO_UINT32 slot_num = 0;
    PNIO_UINT32 subslot_num = 0;
    
    if(NULL != pAddr)
    {
        //lint --e{415} access out-of-bound pointer ('Integer' beyond end of data) by operator 'String'
        //pAddr->Geo.Slot and pAddr->Geo.Subslot are already allocated and initialized
        slot_num    = pAddr->Geo.Slot;
        subslot_num = pAddr->Geo.Subslot;
    }
    else
    {
        printf("Error: Null pointer\n");
    }
    OutDatIops [slot_num][subslot_num] = Iops;    // provider status (of remote io controller)

    // *---- check provider status and notify modifications ------*
    if (OutDatIops_old [slot_num][subslot_num] != Iops)
    {
        printf("new IO controller output provider status (OPS) = 0x%x in slot %d, subslot %d\n",
                       Iops, slot_num, subslot_num);
        OutDatIops_old [slot_num][subslot_num] = Iops;
    }

    return ( (PNIO_IOXS)OutDatIocs [slot_num][subslot_num]);	// consumer state (of local io device)
}

/**
 * @brief  Plug module with User choice of IO module, used by CiR feature
 *
 * @return  PNIO_VOID
 */
PNIO_VOID InputAndPlugSubmodule( PNIO_VOID )
{
    PNIO_UINT32 ModID, SubID, Slot, Subslot, InDataLen, OutDataLen, i;
    PNIO_DEV_ADDR  Addr;
    PNIO_UINT32 availableModIdList[MOD_ID_CNT] = { IO_MODULE_1_BYTE_INOUT,
                                                   IO_MODULE_1_BYTE_IN,
                                                   IO_MODULE_1_BYTE_OUT,
                                                   IO_MODULE_64_BYTE_INOUT,
                                                   IO_MODULE_64_BYTE_IN,
                                                   IO_MODULE_64_BYTE_OUT,
                                                   IO_MODULE_64_BYTE_IN_IRT,
                                                   IO_MODULE_64_BYTE_OUT_IRT,
                                                   IO_MODULE_1_BYTE_IN_IRT,
                                                   IO_MODULE_1_BYTE_OUT_IRT,
                                                   IO_MODULE_250_BYTE_INOUT,
                                                   IO_MODULE_250_BYTE_IN,
                                                   IO_MODULE_250_BYTE_OUT   };


    PNIO_UINT8 availableSubIdList[SUB_ID_CNT] = {SUBMODULE_ID_1};
    PNIO_UINT32 Status;

    printf("--Plug of new submodule--\nList of available ModIDs: ");
    for (i = 0; i < MOD_ID_CNT; ++i)
    {
        printf("0x%02x ",availableModIdList[i]);
    }
    printf("\nList of available SubIDs: ");
    for (i = 0; i < SUB_ID_CNT; ++i)
    {
        printf("0x%02x ",availableSubIdList[i]);
    }

    printf("\nPick ModID: 0x");
    scanf ("%x", (PNIO_UINT32 *) &ModID);
    /* Check whether ModId is in availableModIdList */
    for (i = 0; i < MOD_ID_CNT; ++i)
    {
    	if(ModID == availableModIdList[i])
    	{
    		break;
    	}
    }
    if(i == MOD_ID_CNT)
    {
    	printf("Wrong ModId!\n");
    	return;
    }

    printf("Pick SubID: 0x");
    scanf ("%x", (PNIO_UINT32 *) &SubID);
    /* Check whether SubId is in availableSubIdList */
    for (i = 0; i < SUB_ID_CNT; ++i)
    {
    	if(SubID == availableSubIdList[i])
        {
    		break;
    	}
    }
    if(i == SUB_ID_CNT)
    {
    	printf("Wrong SubId!\n");
    	return;
    }

    printf("Pick slot: ");
    scanf ("%d", (PNIO_UINT32 *) &Slot);
    printf("Pick subslot: ");
    scanf ("%d", (PNIO_UINT32 *) &Subslot);

	if((Slot > IOD_CFG_MAX_SLOT_NUMBER) || (Subslot > IOD_CFG_MAX_NUMOF_SUBSL_PER_SLOT))
    {
		printf("There is maximum of %d slots and %d subslots. No plug was done\n",
				IOD_CFG_MAX_SLOT_NUMBER, IOD_CFG_MAX_NUMOF_SUBSL_PER_SLOT);
                    return;
    }

    printf("Select input data length (0-1440): ");
    InDataLen = OsKeyScan32(NULL, 10);
    if (InDataLen > 1440)
    {
        printf("Trying to plug module with invalid data length\n");
        return;
    }

    printf("Select output data length (0-1440): ");
    OutDataLen = OsKeyScan32(NULL, 10);
    if (OutDataLen > 1440)
    {
        printf("Trying to plug module with invalid data length\n");
        return;
    }

    Addr.Type = PNIO_ADDR_GEO;
    Addr.Geo.Slot      = Slot;            // slot 1
    Addr.Geo.Subslot   = Subslot;         // subslot 1

    PNIOext_sub_plug( &Addr,                        // location (slot, subslot)
                        ModID,
                        SubID,                        // submodule identifier
                        InDataLen,
                        OutDataLen,
                        PNIO_IM0_SUBMODULE,           // Submodule supports IM0
                        (IM0_DATA*)NULL,
                        PNIO_S_GOOD);                 // initial iops value, only for submodules without io data
}

/**
 * @brief  Pull module with User choice of IO module, used by CiR feature
 *
 * @return  PNIO_VOID
 */
PNIO_VOID InputAndPullSubmodule(PNIO_VOID)
{

    PNIO_UINT32 Slot, Subslot;
    PNIO_DEV_ADDR  Addr;
    PNIO_UINT32 Status;

    printf("--Pull of existing submodule--\n");

    /*let user pick slot and subslot via console input*/
    printf("Pick slot: ");
    scanf ("%d", (PNIO_UINT32 *) &Slot);
    printf("Pick subslot: ");
    scanf ("%d", (PNIO_UINT32 *) &Subslot);

	if(((Slot > IOD_CFG_MAX_SLOT_NUMBER) || (Subslot > IOD_CFG_MAX_NUMOF_SUBSL_PER_SLOT)) ||
		((0 == Slot) && (1 == Subslot)))
	{
		printf("Trying to pull invalid module or module which does not support pull\n");
	}
    else{
        Addr.Type           = PNIO_ADDR_GEO;
        Addr.Geo.Slot       = Slot;            // slot 1
        Addr.Geo.Subslot    = Subslot;         // subslot 1

        /*Pull from specified slot and subslot*/
        PNIOext_sub_pull (&Addr);   // location (slot, subslot)
    }
}


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
