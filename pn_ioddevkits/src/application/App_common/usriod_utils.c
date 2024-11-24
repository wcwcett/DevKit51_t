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
/*  F i l e               &F: usriod_utils.c                            :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  utilities for the user, include                                          */
/*                                                                           */
/*   - print devkit version                                                  */
/*   - record handling index 0x8028, 0x8029                                  */
/*   - error checking of record-blockheader                                  */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/


/**
* @file     usriod_utils.c
* @brief    Utilities for device identification
*
*
*/

#include "compiler.h"
#include "usriod_cfg.h"
#include "pniousrd.h"
#include "os.h"
#include "os_taskprio.h"
#include "usriod_utils.h"
#include "nv_data.h"
#include "iodapi_event.h"
#include "version_dk.h"

#include "pndv_inc.h"		/* CM includes are needed as well */
#include "pnpb_com.h"
#include "pnpb_sub_real.h"

/*== defines ================================================================*/

// *** local defines ***
#define INITIAL_MEASURE		1
#define PERFORM_MEASURE		2

#define DAP_MODULE_ID_VALID       0x13572857  // unique tag in flash, if valid dap module id in flash
#define DAP_MODULE_ID_INVALID     0xffffffff  // unique tag in flash, if no valid dap module stored

extern PNPB_REAL PnpbMod;

/*------------------------------------------------------------------*/
/*             LOCAL FUNCTION PROTOTYPES                            */
/*------------------------------------------------------------------*/
PNIO_UINT8	Usr_Check_Device_Name_Validity	( PNIO_UINT8* pDevName, PNIO_UINT32 DevNameLen );
PNIO_UINT8	Usr_Check_Ip_Suite_Validity		( PNIO_UINT8* pIpSuite );
PNIO_VOID	Usr_Print_Ip_Suite				( PNIO_UINT8* pIpSuite );
PNIO_UINT8	Usr_Look_Like_Port_Name			( PNIO_UINT8* pIpSuite, PNIO_UINT8 len );
PNIO_UINT8	Usr_Analyze_Device_Name			( PNIO_UINT8* pDevName, PNIO_UINT32 DevNameLen, PNIO_UINT8* is_like_ip );
PNIO_UINT8	Usr_Check_Subnet_Mask_Validity	( PNIO_UINT32 subnet_mask);
PNIO_VOID 	Usr_Read_And_Parse_Ip			( PNIO_UINT8* pIpSuite, PNIO_UINT8* pOldIpSuite );

/*------------------------------------------------------------------*/
/*             RETURN CODES OF TESTS                                */
/*------------------------------------------------------------------*/
#define USR_ADDR_TEST_OK				0x00
#define USR_NAME_INVALID_INPUT			0x01
#define USR_NAME_INVALID_CHAR			0x02
#define USR_NAME_LIKE_IP_ADDR			0x03
#define USR_NAME_LIKE_PORT_NAME			0x04
#define USR_NAME_INVALID_POSITION		0x05	/* character at invalid position - i.e. ending by '-' */
#define USR_NAME_LABEL_BAD_SIZE			0x06	/* max length of label = 63, min length = 1 */
#define USR_IP_INVALID_MAC				0x21
#define USR_IP_MUTUALY_EXCLUSIVE_PRMS	0x22	/* i.e. incompatible ip versus mask*/
#define USR_IP_INVALID_GATEWAY			0x23
#define USR_ADDR_AR_FORBIDDEN           0x30
#define USR_ADDR_NO_CHANGE              0x31

// *----------------------------------------------------------------*
// *
// *  PrintDevkitVersion (void)
// *
// *----------------------------------------------------------------*
PNIO_UINT32 PrintDevkitVersion (void)
{
    PNIO_DK_VERSION Version;
    PNIO_UINT32  Status;

    Status = PNIO_get_version (&Version);

    PNIO_ConsolePrintf("PNIOD DEVKIT appl.-example no.%d, %c %d.%d.%d.%d, %s - %s\n",
                       EXAMPL_DEV_CONFIG_VERSION, DEVKIT_VERSION_PREFIX,
                       Version.item_hh, Version.item_h, Version.item_l, Version.item_ll,
                       __DATE__, __TIME__);
#if (IOD_INCLUDE_POF == 1)
    PNIO_printf(">>> POF <<<\n");
#endif
    
    PNIO_ConsolePrintf("DAP %d, Platform 0x%04x\n", MODULE_ID_DAP, PNIOD_PLATFORM);
    
    return (Status);
}


//----------------------------------------------------------------*
// *
// *  SetGpioLED (void)
// *
// *----------------------------------------------------------------*
void SetGpioLed (PNIO_UINT8 OnOff)
{
    PNIO_UINT32 LedNum;
    PNIO_UINT32 Status = PNIO_OK;
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
    PNIO_ConsolePrintf ("\nGPIO LEDs and Values\n");
    PNIO_ConsolePrintf ("GPIO_LED_USER_00 .......................Value = 18\n");
    PNIO_ConsolePrintf ("GPIO_LED_USER_01 .......................Value = 19\n");
    PNIO_ConsolePrintf ("GPIO_LED_USER_02 .......................Value = 20\n");
    PNIO_ConsolePrintf ("GPIO_LED_USER_03 .......................Value = 21\n");
    PNIO_ConsolePrintf ("GPIO_LED_USER_04 .......................Value = 22\n");
    PNIO_ConsolePrintf ("GPIO_LED_USER_05 .......................Value = 23\n");
    PNIO_ConsolePrintf ("GPIO_LED_USER_06 .......................Value = 24\n");
    PNIO_ConsolePrintf ("GPIO_LED_USER_07 .......................Value = 25\n");
    PNIO_ConsolePrintf ("GPIO_LED_USER_08 .......................Value = 26\n");
    PNIO_ConsolePrintf ("GPIO_LED_USER_09 .......................Value = 27\n");
    PNIO_ConsolePrintf ("GPIO_LED_USER_10 .......................Value = 28\n");
    PNIO_ConsolePrintf ("GPIO_LED_USER_11 .......................Value = 29\n");
    PNIO_ConsolePrintf ("GPIO_LED_USER_12 .......................Value = 30\n");
    PNIO_ConsolePrintf ("GPIO_LED_USER_13 .......................Value = 31\n");
    PNIO_ConsolePrintf ("GPIO_LED_USER_14 .......................Value = 32\n");
    PNIO_ConsolePrintf ("GPIO_LED_USER_15 .......................Value = 33\n");
    PNIO_ConsolePrintf ("GPIO_LED_ERROR   red    ............... Value =  2\n");
    PNIO_ConsolePrintf ("GPIO_LED_MAINT   yellow ............... Value =  1\n");
    PNIO_ConsolePrintf ("GPIO_LED_DIAG    yellow ............... Value = 15\n");
    PNIO_ConsolePrintf ("GPIO_LED_PENERGY green  ............... Value = 16\n");
    PNIO_ConsolePrintf ("GPIO_LED_SYNC    green  ............... Value =  3\n");
    PNIO_ConsolePrintf ("GPIO_LED_FO1     fo maintenance port1 ..Value = 12\n");
    PNIO_ConsolePrintf ("GPIO_LED_FO2     fo maintenance port2 ..Value = 13\n");

    if (OnOff == 1)
    {
       	LedNum  = OsKeyScan32 ("Select GPIO LED Value for ON , (-1)=all) = ", 10);
       	if (LedNum == (PNIO_UINT32)-1)
       	{
       	   PNIO_ConsolePrintf ("set all LEDs ON\n");
       	   for (LedNum = 0; LedNum < PNIO_LED_NUMOF_ENTRIES; LedNum++)
       	   {
       	   	  Bsp_EbSetLed (LedNum, 1);  // ignore return value here, invalid values do nothing
       	   }
       	}
       	else
       	{
       	   PNIO_ConsolePrintf ("set LED %d ON\n", LedNum);
       	   Status = Bsp_EbSetLed (LedNum, 1);
       	   if (Status == PNIO_NOT_OK)
       	   {
       	   	PNIO_ConsolePrintf ("ERROR: set unknown LED 0x%x\n", LedNum);
       	   }
       	}
    }
    else
    {

        LedNum  = OsKeyScan32 ("Select GPIO LED Value for OFF, (-1)=all) = ", 10);
        if (LedNum == (PNIO_UINT32)-1)
        {
            PNIO_ConsolePrintf ("set all LEDs OFF\n");
            for (LedNum = 0; LedNum < PNIO_LED_NUMOF_ENTRIES; LedNum++)
            {
               Bsp_EbSetLed (LedNum, 0);  // ignore return value here, invalid values do nothing
            }
        }
        else
        {
           PNIO_ConsolePrintf ("set LED %d OFF\n", LedNum);
           Status = Bsp_EbSetLed (LedNum, 0);
           if (Status == PNIO_NOT_OK)
           {
             PNIO_ConsolePrintf ("ERROR: set unknown LED 0x%x\n", LedNum);
           }
        }
     }
#elif

    PNIO_ConsolePrintf ("ERROR: invalid platform \n");
#endif
}

// *----------------------------------------------------------------*
// *
// *  usr_rec_check_blockhdr (pBlockHdr, BlockType, BlockLen, BlockVers
// *
// *  Checks the blockheader of a read/write record request and
// *  sets the error values according PNIO spec. 61158-6
// *
// *----------------------------------------------------------------*
//lint -e{832} Parameter 'Symbol' not explicitly declared, int assumed
PNIO_UINT32 usr_rec_check_blockhdr
(
    REC_IO_BLOCKHDR*    pBlockHdr,      // pointer to real blockheader
    PNIO_UINT16         BlockType,      // expected block type
    PNIO_UINT8          BlockLen,       // expected block length
    PNIO_UINT16         BlockVers,      // expected block version
    PNIO_ERR_STAT       *pPnioState     // return 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
)
{
  // **** check block type  ***
  if (pBlockHdr->Type != OsHtons(BlockType))
  {
        return (PNIO_NOT_OK);
  }

  // **** check block length  ***
  if (pBlockHdr->Len != OsHtons ((PNIO_UINT16)BlockLen))
  {
        return (PNIO_NOT_OK);
  }

  // **** check block version   ***
  if (pBlockHdr->Version != OsHtons(BlockVers))
  {
        return (PNIO_NOT_OK);
  }

  return (PNIO_OK);
}

#if !IOD_INCLUDE_REC8028_8029

    // *----------------------------------------------------------------*
    // *
    // *  RecInpDatObjElement_Handler (....)
    // *----------------------------------------------------------------*
    // *  this function handles a record request on RecordIndex 0x8028,
    // *  which is specified as a "read input data from submodule" request.
    // *  If an error occures, the error structure PNIO_ERR_STAT is filled.
    // *  See PNIO specification iec 61158-6 for more details about how to build the
    // *  structure and how to set the error states. Search for
    // *  "RecordInputDataObjectElement".
    // *
    // *  If the specified submodule is a input module without any io data
    // *  (e.g. DAP, PDEV ports ==> datalen = 0 is assumed) only IOPS
    // *  and IOCS is returned, but no error.
    // *  If the submodule is an output submodule (assume in this example,
    // *  it is not the DAP slot and has no input data ==> it is an output module)
    // *  then an error is returned.
    // *----------------------------------------------------------------*
    PNIO_UINT32  RecInpDatObjElement_Handler
		    (
                PNIO_UINT32         Api,			// api number
			    PNIO_DEV_ADDR		*pAddr, 		// geographical or logical address
			    PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState, 	// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
                PNIO_UINT32         IoDatLen,       // length of input data
                PNIO_UINT8          *pDat,          // pointer to input data
                PNIO_UINT8          iocs,           // iocs value
                PNIO_UINT8          iops            // iops value
		    )
    {

        REC_IO_INPUT_STRUCT*     pIo = (REC_IO_INPUT_STRUCT*)    pBuffer;
        PNIO_UINT32              ExpectedBufLen;
        PNIO_UINT16              RecIoStrucSize = sizeof (REC_IO_INPUT_STRUCT);

        // *** get buffer length, buffer consists of REC_IO_INPUT_STRUCT + io-data length  ***
        ExpectedBufLen = RecIoStrucSize + IoDatLen;

        // **** plausibility check: module is an input module ?? ***
	    if ((IoDatLen == 0)&& (pAddr->Geo.Slot != 0)) // we have no io data inside the DAP subslots
        { // **** error ****
            OsMemSet (pPnioState, 0, sizeof (PNIO_ERR_STAT));  // first clear complete error structure
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb0;  // example: Error Class 11 , ErrorNr  = ""
            return (PNIO_NOT_OK);
        }

        // *** plausibility check buffer length, set total data length ***
        if (*pBufLen <  ExpectedBufLen)        // plausibility check
        { // **** error ****
            OsMemSet (pPnioState, 0, sizeof (PNIO_ERR_STAT));  // first clear complete error structure
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xc0;  // example: Error Class 12 = resource, ErrorNr  = "write constrain conflict"
            return (PNIO_NOT_OK);
        }

        // *** return value = real blocklength ***
        *pBufLen = ExpectedBufLen;


        // *** set blockheader ***
        pIo->BlockHeader.Type      = OsHtons (0x0015);              // must be 0x15 for RecordInputDataObjectElement
        pIo->BlockHeader.Len       = OsHtons ((PNIO_UINT16)(ExpectedBufLen - sizeof (REC_IO_BLOCKHDR) + 2)); // blocklength, header excluded
        pIo->BlockHeader.Version   = OsHtons (0x0100);                      // must be 0x0100 (see PNIO specification)

        // *** set length and values of IOCS and IOPS value ***
        pIo->LengthIOCS = 1;
        pIo->LengthIOPS = 1;
        pIo->IOCS = iocs;
        pIo->IOPS = iops;


        // *** set io data length ***
        pIo->LengthData            = OsHtons ((PNIO_UINT16)IoDatLen);   // input data: length includes data without iops,iocs

        // ***  copy IO data of the specified submodule ***
        OsMemCpy (& (*(pBuffer + RecIoStrucSize)),               // input data destination
                  pDat,  // input data source
                  IoDatLen);                                       // input data length


        // *** clear error structure ***
        OsMemSet (pPnioState, 0, sizeof (PNIO_ERR_STAT));
        return (PNIO_OK);
    }


    // *----------------------------------------------------------------*
    // *
    // *  RecOutDatObjElement_Handler (....)
    // *----------------------------------------------------------------*
    // *  this function handles a record request on RecordIndex 0x8029,
    // *  which is specified as a "read output data from submodule" request.
    // *  If an error occures, the error structure PNIO_ERR_STAT is filled.
    // *  See PNIO specification iec 61158-6 for more details about how to build the
    // *  structure and how to set the error states. Search for
    // *  "RecordOutputDataObjectElement".
    // *
    // *  If the specified submodule is a module without any io data
    // *  (e.g. DAP, PDEV ports ==> datalen = 0) then an error is returned.
    // *  If is an input submodule (we assume in this example, it is not
    // *  the DAP slot and has no input data ==> it is an input module)
    // *  then alos an error is returned.
    // *----------------------------------------------------------------*
    PNIO_UINT32  RecOutDatObjElement_Handler
		    (
                PNIO_UINT16         ArNum,          // AR number
                PNIO_UINT32         Api,            // api number
                PNIO_DEV_ADDR       *pAddr,         // geographical or logical address
                PNIO_UINT32         *pBufLen,       // [in, out] in: length to read, out: length, read by user
                PNIO_UINT8          *pBuffer,       // [in] destination buffer pointer
                PNIO_ERR_STAT       *pPnioState,    // [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
                PNIO_UINT32         IoDatLen,       // length of output data
                PNIO_UINT8          *pDat,          // pointer to output data
                PNIO_UINT8          iocs,           // iocs value
                PNIO_UINT8          iops,           // iops value
                PNIO_UINT8          *pSubstDat,     // pointer to substituted values for output data
                PNIO_UINT16         SubstMode,      // 0: ZERO,  1: last value, 2: replacement value (see pnio-spec)
                PNIO_UINT16         SubstActive     // 0: output values active, 1: substituted values active
		    )
    {

        REC_IO_OUTPUT_STRUCT*    pIo = (REC_IO_OUTPUT_STRUCT*)    pBuffer;
        PNIO_UINT16              RecIoStrucSize = sizeof (REC_IO_OUTPUT_STRUCT);
        PNIO_UINT32              ExpectedBufLen;
        PNIO_UINT32              BufOffset = 0;

#if(IOD_INCLUDE_S2_REDUNDANCY == 0)  
        PNIO_UNUSED_ARG(ArNum);
#endif
    // *   REC_IO_OUTPUT_STRUCT         (with BlockHeader type = 0x0016 = output)
    // *   OutputDataObjectElement      (normal output data)
    // *   iops                         (iops value)
    // *   REC_IO_SUBSTVAL_STRUCT       (with BlockHeader ttype = 0x0014 = substituted)
    // *   SubstituteDataObjectElement  (substitute output data)
    // *   Substitute data valid        (coded as iops and iocs)

        // *** get buffer length, buffer consists of REC_IO_INPUT_STRUCT + io-data + IOPS + substitueValues ***
        ExpectedBufLen =   sizeof (REC_IO_OUTPUT_STRUCT)     // size of the output header structure (iocs is included)
                         + 1                                 // IOCS-len
                         + IoDatLen                          // size of normal io data
                         + 1                                 // IOPS-len
                         + sizeof (REC_IO_SUBSTVAL_STRUCT)   // size of substituted value struct.
                         + 1                                 // IOCS-len of substituted data
                         + IoDatLen                          // size of substituted data (== size of normal io data)
                         + 1;                                // size of SubstituteDataValid

        // *** plausibility check: module is an input module ?? ***
        if (IoDatLen == 0)
        { // **** error ****
            OsMemSet (pPnioState, 0, sizeof (PNIO_ERR_STAT));  // first clear complete error structure
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb0;  // example: Error Class 11 , ErrorNr  = ""
            return (PNIO_NOT_OK);
        }

        // *** plausibility check buffer length, set total data length ***
        if (*pBufLen <  ExpectedBufLen)        // plausibility check
        { // ***** error *****
            OsMemSet (pPnioState, 0, sizeof (PNIO_ERR_STAT));  // first clear complete error structure
            pPnioState->ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
            pPnioState->ErrDecode = 0x80;  // PNIORW
            pPnioState->ErrCode1  = 0xb0;  // example: Error Class 11 = resource, ErrorNr  = ""
            return (PNIO_NOT_OK);
        }

        // *** set blocklength ***
        *pBufLen = ExpectedBufLen;


        // *** set blockheader ***
        pIo->BlockHeader.Type      = OsHtons (0x0016);              // must be 0x16 for RecordOutputDataObjectElement
        pIo->BlockHeader.Len       = OsHtons ((PNIO_UINT16)(ExpectedBufLen - sizeof (REC_IO_BLOCKHDR) + 2)); // blocklength, iops and header excluded
        pIo->BlockHeader.Version   = OsHtons (0x0100);              // must be 0x01 (see PNIO specification)

        // *** set length of IOCS and IOPS value ***
        pIo->LengthIOCS = 1;
        pIo->LengthIOPS = 1;


        // *** set io data length ***
        pIo->LengthData            = OsHtons ((PNIO_UINT16)(IoDatLen));   // output data: length includes data + iops + iocs

        // ***  copy IO data of the specified submodule             ***
        // ***  we assume in this example, its the physical data,   ***
        // ***  not the substitude data                             ***
        pIo->SubstActiveFlag = SubstActive;  // 0x0000 == operation, 0x0001 = substitute, see pnio spec.

        // *** set IOCS, located between REC_IO_BLOCKHDR and data block ***
        BufOffset = RecIoStrucSize;  // set buffer offset behind REC_IO_BLOCKHDR
        *(pBuffer + BufOffset) = iocs;

        //*** copy IO data block ***
        BufOffset +=1;   // set buffer offset behind IOCS byte
        OsMemCpy (& (*(pBuffer + BufOffset)),  // input data destination
                  pDat                      ,  // input data source
                  IoDatLen);

        // *** set new BufOffset behind io data ***
        BufOffset += IoDatLen;

        // *** set IOPS behind IO data block ***
#if(IOD_INCLUDE_S2_REDUNDANCY == 1)         
        PNIO_UINT16 ArInd = pndv_ar_get_ar_idx_by_ar_nr(ArNum);

        if((PnpbExp[ArInd].ArType == PNIO_AR_TYPE_SINGLE_SYSRED) && (pnpb_data.io.s2.primary_backup_state[ArInd] == PNPB_IO_DATA_STATE_P_B_BACKUP) && (pAddr->Geo.Slot != 0))
        {
            // In case of Backup AR we need to discard IOCs IOPS. Some IOCs (including certification tool ART) send IOPS as good in Backup AR as well.
            *(pBuffer + BufOffset) = PNIO_S_BAD;
        }
        else      
#endif
        {
            *(pBuffer + BufOffset) = iops;
        }

        // *** set new BufOffset behind iops ***
        BufOffset += 1;


        // *** copy substitute data behind IOPS                       ***
        // *** we assume in this example: substitue value = 0x00      ***
        // *** note: if necessary every io byte could have its own    ***
        // *** substitue value                                        ***
        {
            REC_IO_SUBSTVAL_STRUCT*  pSubst       = (REC_IO_SUBSTVAL_STRUCT*) (pBuffer + BufOffset);
            PNIO_UINT32              SubstDataLen =   sizeof (REC_IO_SUBSTVAL_STRUCT) // size of the output header structure
                                                     + IoDatLen                       // size of normal io data
                                                     + 2;                             // iocs-len and SubstitudeDataValid-len

            pSubst->BlockHeader.Type      = OsHtons (0x0014);              // must be 0x14 for SubstituteValue
            pSubst->BlockHeader.Len       = OsHtons ((PNIO_UINT16)(SubstDataLen - sizeof (REC_IO_BLOCKHDR) + 2)); // blocklength, header excluded
            pSubst->BlockHeader.Version   = OsHtons (0x0100);              // must be 0x01 (see PNIO specification)

            // *** we assume, substituted value mode is "replacement"
            pSubst->SubstMode = OsHtons (SubstMode);   // SubstitutionMode: 0=ZERO, 1:LastValue, 2:Replacement value

            // *** set iocs
            BufOffset += sizeof (REC_IO_SUBSTVAL_STRUCT);
            *(pBuffer + BufOffset) = 0x80;        // set iocs for substitude data (we assume, value is always GOOD)

            // *** set substitue data to replacement value (here: EXAMPLE_SUBSTITUTE_VALUE)
            BufOffset += 1;                      // set buffer offset to start of substitude data

            OsMemCpy (pBuffer + BufOffset,       // input data destination
                      pSubstDat,                 // substitute values
                      IoDatLen);                 // input data length

            BufOffset += IoDatLen;               // set buffer offset behind substitude data

            // *** set SubstitudeDataValid (coded as iops, iocs)
            *(pBuffer + BufOffset) = 0x80;       // set Substitute data valid (we assume, value is always GOOD)
        }

        // *** clear error structure ***
        OsMemSet (pPnioState, 0, sizeof (PNIO_ERR_STAT));


        return (PNIO_OK);
    }
#endif




 // *----------------------------------------------------------------*
 // *
 // *  InputMacAddress ()
 // *
 // *----------------------------------------------------------------*
 // *  This function performs a simple input menue, to set a new
 // *  mac address. The new mac address is stored afterwards in the
 // *  private data block of the nv_data.
 // *----------------------------------------------------------------*
 // *  Input:     --
 // *----------------------------------------------------------------*
void InputAndStoreMacAddress (void)
{
	PNIO_UINT32 Val;  // hexadecimal imput
	PNIO_UINT32 i;
	PNIO_UINT8* pMacAddr;
	PNIO_UINT32 MacAddrLen;
	// **** get the current mac address from flash memory ***
	Bsp_nv_data_restore (PNIO_NVDATA_MAC_ADDR, (PNIO_VOID**)&pMacAddr, &MacAddrLen);

	// **** print current mac address, input new mac address ***
    PNIO_printf("Current Ethernet address is: %02x:%02x:%02x:%02x:%02x:%02x\n",
                *(pMacAddr+0), *(pMacAddr+1), *(pMacAddr+2),
                *(pMacAddr+3), *(pMacAddr+4), *(pMacAddr+5));
    PNIO_printf("Modify all 6 bytes (board unique portion) of Ethernet Address\n");
    PNIO_printf("The first 3 bytes are manufacturer's default address block\n");
	for (i = 0; i < 6; i++)
	{
        PNIO_printf("%02x - ", *(pMacAddr+i));
        Val = OsKeyScan32 (NULL, 16);
        *(pMacAddr+i) = (PNIO_UINT8) Val;
	}

    PNIO_printf("store new Ethernet address  %02x:%02x:%02x:%02x:%02x:%02x\n",
                *(pMacAddr+0), *(pMacAddr+1), *(pMacAddr+2),
                *(pMacAddr+3), *(pMacAddr+4), *(pMacAddr+5));


	if ( ParamStoreMacAddress(pMacAddr) != USR_ADDR_TEST_OK )
	{
        PNIO_printf("Mac address was not stored - wrong MAC address or same MAC address \n");
	}
	else
	{
        PNIO_printf("*-----------------------------------------------------------*\n");
        PNIO_printf("* to activate the new mac address, perform a system restart *\n");
        PNIO_printf("*-----------------------------------------------------------*\n");
        PNIO_printf("\n");
	}

    // *** free ram block with private data, when no more needed ***
    Bsp_nv_data_memfree (pMacAddr);
}

// *----------------------------------------------------------------*
 // *
 // *  ParamStoreMacAddress ()
 // *
 // *----------------------------------------------------------------*
 // *  The new mac address is stored afterwards in the
 // *  private data block of the nv_data.
 // *----------------------------------------------------------------*
 // *  Input:     MacAddr
 // *----------------------------------------------------------------*
PNIO_UINT8 ParamStoreMacAddress (PNIO_UINT8* MacAddr)
{
    PNIO_UINT8* pMacOldAddr;
    PNIO_UINT32 MacAddrLen;
    PNIO_UINT8  res = USR_NAME_INVALID_INPUT;

     // MAC address check mechanism -> cannot be MC or BC
    if ( ( MacAddr[0] & 0x01 ) != 0 )
    {
        return res;
    }
	 
    // MAC address can not be 00:00:00:00:00:00
    if ( (0 == MacAddr[0]) && (0 == MacAddr[1]) && (0 == MacAddr[2]) &&
         (0 == MacAddr[3]) && (0 == MacAddr[4]) && (0 == MacAddr[5]) )
    {
        return res;
    }
	 
    // **** get the current mac address from flash memory ***
    Bsp_nv_data_restore (PNIO_NVDATA_MAC_ADDR, (PNIO_VOID**)&pMacOldAddr, &MacAddrLen);

    if ( 0 !=
            OsMemCmp ( (const PNIO_VOID *) pMacOldAddr, (const PNIO_VOID *) MacAddr, MacAddrLen )
       )
    {
        // *** store new mac address in flash memory ****
        Bsp_nv_data_store (PNIO_NVDATA_MAC_ADDR, (PNIO_VOID*) MacAddr, MacAddrLen);

        res = USR_ADDR_TEST_OK;
    }
    else
    {
        res = USR_ADDR_NO_CHANGE;
    }

    // *** free ram block with private data, when no more needed ***
    Bsp_nv_data_memfree (pMacOldAddr);

    return res;
}

/**
 * @brief Printout of ip address in valid format
 *
 * @param[in]		*pIpSuite	buffer with IP address, mask and gateway
 * @return			void		no return value
 *
 */
PNIO_VOID Usr_Print_Ip_Suite( PNIO_UINT8* pIpSuite )
{
	PNIO_printf( "Current IP address: %03d.%03d.%03d.%03d\n",
				  *( pIpSuite + 0 ), *( pIpSuite + 1 ), *( pIpSuite + 2 ), *( pIpSuite + 3 ) );
	PNIO_printf( "Mask:               %03d.%03d.%03d.%03d\n",
				  *( pIpSuite + 4 ), *( pIpSuite + 5 ), *( pIpSuite + 6 ), *( pIpSuite + 7 ) );
	PNIO_printf( "Default gateway:    %03d.%03d.%03d.%03d\n",
				  *( pIpSuite + 8 ), *( pIpSuite + 9 ), *( pIpSuite + 10 ), *( pIpSuite + 11 ) );
}


/**
 * @brief Check if network subnet mask is valid
 *
 * @param[in]		subnet_mask	buffer with IP address, mask and gateway
 *
 * @return			PNIO_TRUE	valid mask
 * 					PNIO_FALSE	invalid mask
 *
 */
PNIO_UINT8 Usr_Check_Subnet_Mask_Validity( PNIO_UINT32 subnet_mask)
{

	switch (subnet_mask)
	{
		case 0x00000000: /* /0 .... invalid because IP-stacks interpret it as "derive mask from ip-address" and not "no subnetting" */
		case 0x80000000: /* /1 .... invalid because of SPH IP Configuration */
		case 0xC0000000: /* /2 .... invalid because of SPH IP Configuration */
		case 0xE0000000: /* /3 .... invalid because of SPH IP Configuration */
		case 0xFFFFFFFE: /* /31 ... invalid because of SPH IP Configuration */
		case 0xFFFFFFFF: /* /32 ... invalid because of SPH IP Configuration */
		{
			return PNIO_FALSE;
		}

		default:
		{
			/* mask composed of consecutive 1 test */
			LSA_UINT32 tmp = ~subnet_mask + 1; /* note: if x == 1111000... then ~x == 0000111... then ~x+1 == 0001000... which is 2^x */
			if ((tmp & (tmp - 1)) != 0) /* 2^x check see OpenBSD, param.h */
			{
				return PNIO_FALSE;
			}
			break;
		}
	}
	return PNIO_TRUE;
}


/**
 * @brief Test validity of values in ip buffer
 *
 * @param[in]		*pIpSuite	buffer with IP address, mask and gateway
 *
 * @return			USR_ADDR_TEST_OK				address valid
 * 					USR_IP_INVALID_MAC				invalid mask value
 * 					USR_IP_MUTUALY_EXCLUSIVE_PRMS	invalid combination of ip, mask and gateway
 * 					USR_IP_INVALID_GATEWAY			invalid gateway value
 *
 *  This function performs set of checks against ip address
 *
 */
PNIO_UINT8 Usr_Check_Ip_Suite_Validity( PNIO_UINT8* pIpSuite )
{
    PNIO_UINT32 ip_addr     = OsNtohl(*(PNIO_UINT32*)(pIpSuite + 0));
    PNIO_UINT32 subnet_mask = OsNtohl(*(PNIO_UINT32*)(pIpSuite + 4));
    PNIO_UINT32 gateway     = OsNtohl(*(PNIO_UINT32*)(pIpSuite + 8));

	/* delete ip-suite -> also mac ang gateway have to be 0*/
	if( ( ip_addr == 0 ) && ( ( subnet_mask != 0 ) || ( gateway != 0 ) ) )
	{
		return USR_IP_MUTUALY_EXCLUSIVE_PRMS;
	}
	if( PNIO_FALSE == Usr_Check_Subnet_Mask_Validity( subnet_mask ) )
	{
		return USR_IP_INVALID_MAC;
	}

	LSA_UINT32 host0_addr = (ip_addr & subnet_mask);
	LSA_UINT32 host1_addr = (ip_addr | ~subnet_mask);

	if( ( ip_addr == host0_addr ) 					||
		( ip_addr == host1_addr ) 					||
		( ip_addr == 0xFFFFFFFF ) 					||	/* 255.255.255.255 (limited broadcast) */
		( (ip_addr & subnet_mask) == 0 ) 			|| 	/* net-address {0, <Host-number>} */
		( (ip_addr & 0xFF000000) == 0 ) 			|| 	/* net-address 0.0.0.0/8 */
		( (ip_addr & 0xFF000000) == 0x7F000000 ) 	||	/* 127.0.0.0/8 (loopback) */
		( (ip_addr & 0xF0000000) == 0xE0000000 ) 	||	/* 224.0.0.0/4 (multicast) */
		( (ip_addr & 0xF0000000) == 0xF0000000 ) 	) 	/* 240.0.0.0/4 (reserved) */
	{
		return USR_IP_MUTUALY_EXCLUSIVE_PRMS;
	}

	if( ( gateway == 0 ) || ( gateway == ip_addr ) )
	{
		/* no gateway */
	}
	else
	{
		if( ( gateway == 0xFFFFFFFF ) 					|| 	/* 255.255.255.255 (limited broadcast) */
			( ( gateway & 0xFF000000) == 0 ) 			||  /* net-address 0.0.0.0/8 */
			( ( gateway & 0xFF000000) == 0x7F000000 ) 	|| 	/* 127.0.0.0/8 (loopback) */
			( ( gateway & 0xF0000000) == 0xE0000000 ) 	|| 	/* 224.0.0.0/4 (multicast) */
			( ( gateway & 0xF0000000) == 0xF0000000 ) 	) 	/* 240.0.0.0/4 (reserved) */
		{
			return USR_IP_INVALID_GATEWAY;
		}

		if ((gateway & subnet_mask) != (ip_addr & subnet_mask))
		{
			return USR_IP_MUTUALY_EXCLUSIVE_PRMS;
		}
		else /* in same subnet */
		{
			host0_addr = (gateway & subnet_mask);
			host1_addr = (gateway | ~subnet_mask);

			if( ( gateway == host0_addr ) || ( gateway == host1_addr ) )
			{
				return USR_IP_MUTUALY_EXCLUSIVE_PRMS;
			}
		}
	}
	return USR_ADDR_TEST_OK;
}


/**
 * @brief Checks device name if it looks like port name
 *
 * @param[in]		*pDevName	buffer with device name
 * @param[in]		len			length of device name
 *
 * @return						PNIO_TRUE - looks like port name
 * 								PNIO_FALSE - else
 *
 */
PNIO_UINT8 Usr_Look_Like_Port_Name( PNIO_UINT8* pDevName, PNIO_UINT8 len )
{
	if( ( 'p' == pDevName[ 0 ] ) &&
		( 'o' == pDevName[ 1 ] ) &&
		( 'r' == pDevName[ 2 ] ) &&
		( 't' == pDevName[ 3 ] ) &&
		( '-' == pDevName[ 4 ] ) &&
		( ( '0' <= pDevName[ 5 ] ) && ( '9' >= pDevName[ 5 ] ) ) &&
		( ( '0' <= pDevName[ 6 ] ) && ( '9' >= pDevName[ 6 ] ) ) &&
		( ( '0' <= pDevName[ 7 ] ) && ( '9' >= pDevName[ 7 ] ) ) )
	{
		if(8 == len)	/*port-nnn ;  for n = <0..9>*/
		{
			return PNIO_TRUE;
		}
		else if(14 == len)	/*port-nnn-nnnnn ;  for n = <0..9>*/
		{
			if( ( '-' == pDevName[ 8 ] ) &&
				( ( '0' <= pDevName[ 9 ] ) && ( '9' >= pDevName[ 9 ] ) ) &&
				( ( '0' <= pDevName[ 10 ] ) && ( '9' >= pDevName[ 10 ] ) ) &&
				( ( '0' <= pDevName[ 11 ] ) && ( '9' >= pDevName[ 11 ] ) ) &&
				( ( '0' <= pDevName[ 12 ] ) && ( '9' >= pDevName[ 12 ] ) ) &&
				( ( '0' <= pDevName[ 13 ] ) && ( '9' >= pDevName[ 13 ] ) ) )
			{
				return PNIO_TRUE;
			}
		}
		else /* invalid params */
		{
			PNIO_Fatal(  );
		}
	}
	/* conditions not met -> do not look like port name */
	return PNIO_FALSE;
}


/**
 * @brief Test validity of value in device name
 *
 * @param[in]		*pDevName	buffer with device name
 * @param[in]		DevNameLen	length of device name
 * @param[out]		*is_like_ip can not be like ip any more
 *
 * @return			USR_NAME_LABEL_BAD_SIZE			invalid label size
 * 					USR_NAME_INVALID_POSITION		valid character on invalid position (i.e. beggining with '.')
 * 					USR_NAME_LIKE_PORT_NAME			device name looks like port name
 * 					USR_ADDR_TEST_OK				everything is valid
 *
 *  Testing label size, invalid character positions, ip and port name look-likeness
 */
PNIO_UINT8 Usr_Analyze_Device_Name( PNIO_UINT8* pDevName, PNIO_UINT32 DevNameLen, PNIO_UINT8* is_like_ip )
{
	PNIO_UINT32 i;
	PNIO_UINT8 label_len = 0, num_of_labels = 0;

	for( i = 0; i < DevNameLen; i++ )
	{
		if( '.' == pDevName[ i ] )
		{
			/* if not of length 3, then there is no risk of look-like-ip */
			if(3 != label_len)
			{
				is_like_ip = PNIO_FALSE;
			}

			/* label size at least 1 */
			if( 0 == label_len )
			{
				return USR_NAME_LABEL_BAD_SIZE;
			}
			label_len = 0;
			num_of_labels++;

			if( 0 < i )
			{
				/* last char of label cannot '-' */
				if( '-' == pDevName[ i - 1 ] )
				{
					return USR_NAME_INVALID_POSITION;
				}
			}
		}
		else
		{
			label_len++;

			/* first char of label cannot be '-' */
			if( 1 == label_len )
			{
				if( '-' == pDevName[ i ] )
				{
					return USR_NAME_INVALID_POSITION;
				}
			}

			/* label (section of name divided by '.') have to be shorter than 63 */
			if( 63 < label_len )
			{
				return USR_NAME_LABEL_BAD_SIZE;
			}
		}
	}

	/* lastr char of last label cannot be '-' */
	if( '-' == pDevName[ DevNameLen - 1 ] )
	{
		return USR_NAME_INVALID_POSITION;
	}

	if( 4 != num_of_labels )
	{
		is_like_ip = PNIO_FALSE;
	}

	/* last label size at least 1 */
	if( 0 == label_len )
	{
		return USR_NAME_LABEL_BAD_SIZE;
	}
	/* first label shold not look like port name */
	if( ( 0 == num_of_labels ) && ( ( 8 == label_len ) || ( 14 == label_len ) ) )
	{
		if( PNIO_TRUE == Usr_Look_Like_Port_Name( pDevName, label_len ) )
		{
			return USR_NAME_LIKE_PORT_NAME;
		}
	}
	return USR_ADDR_TEST_OK;
}


/**
 * @brief Test validity of value in device name
 *
 * @param[in]		*pDevName	buffer with device name
 * @param[in]		DevNameLen	length of device name
 *
 * @return			USR_NAME_LABEL_BAD_SIZE			invalid label size
 * 					USR_NAME_INVALID_POSITION		valid character on invalid position (i.e. beggining with '.')
 * 					USR_NAME_LIKE_PORT_NAME			device name looks like port name
 * 					USR_NAME_INVALID_INPUT			invalid pointer or wrong name length
 * 					USR_NAME_INVALID_CHAR			invalid character in device name
 * 					USR_NAME_LIKE_IP_ADDR			device name looks like ip address
 * 					USR_ADDR_TEST_OK				everything is valid - device name ok
 *
 *  Testing label size, invalid character positions, ip and port name look-likeness,
 *  acceptable name length and characters validity
 */
PNIO_UINT8 Usr_Check_Device_Name_Validity( PNIO_UINT8* pDevName, PNIO_UINT32 DevNameLen )
{
	PNIO_UINT32 i;
	PNIO_UINT8 is_like_ip = PNIO_TRUE;
	PNIO_UINT8 RetVal;

	if( ( NULL == pDevName ) 	||	/* no data */
		( 0 == DevNameLen ) 	||	/* no name inputed */
		( 240 < DevNameLen )	)	/* too long */
	{
		return USR_NAME_INVALID_INPUT;
	}

	/* scan for invalid characters */
	for( i = 0; i < DevNameLen; i++ )
	{
		/* valid characters, which provents look-like-ip */
		if( ( ( 'a' <= pDevName[ i ] ) && ( 'z' >= pDevName[ i ] ) ) ||
			( ( 'A' <= pDevName[ i ] ) && ( 'Z' >= pDevName[ i ] ) ) ||
			( '-' == pDevName[ i ] ) )
		{
			is_like_ip = PNIO_FALSE;
			continue;
		}
		/* valid characters */
		if( ( ( '0' <= pDevName[ i ] ) && ( '9' >= pDevName[ i ] ) ) ||
			( '.' == pDevName[ i ] ) )
		{
			continue;
		}
		/* no hit -> invalid char found */
		return USR_NAME_INVALID_CHAR;
	}
	RetVal = Usr_Analyze_Device_Name(pDevName, DevNameLen, &is_like_ip);
	if( USR_ADDR_TEST_OK != RetVal )
	{
		return RetVal;	/* test failed in called function - push the recieved error */
	}

	/* name like ip address */
	if( PNIO_TRUE == is_like_ip )
	{
		return USR_NAME_LIKE_IP_ADDR;
	}
	return USR_ADDR_TEST_OK;
}


/**
 * @brief Input (from user), check and store new ip address
 *
 * @param[out]	*pIpSuite		pointer for buffer for new device name
 * @param[in]	*pOldIpSuite	pointer for old device name
 *
 * @return			void		no return value
 *
 *  This function performs a simple input menue, to set a new
 *  ip address. The new ip address is stored afterwards in the
 *  private data block of the nv_data.
 */
PNIO_VOID Usr_Read_And_Parse_Ip( PNIO_UINT8* pIpSuite, PNIO_UINT8* pOldIpSuite )
{
	PNIO_UINT32 Val;  /* decimal imput */
	PNIO_UINT8  i;
    PNIO_UINT8  max_val = 0xff;

	PNIO_printf ( "Modify 4 bytes of IP address, decadic input\n" );
	for( i = 0; i < 4; i++ )
	{
		PNIO_printf( "%03d - ", *( pOldIpSuite + i + 0 ) );
		Val = OsKeyScan32( NULL, 10 );
		if( max_val < Val )
		{
			PNIO_printf( "Inputed value too big, truncated to unsigned 8bit/n");
		}
		*( pIpSuite + i + 0 ) = ( PNIO_UINT8 ) Val;
	}

	PNIO_printf ( "Modify 4 bytes of IP mask, decadic input\n" );
	for( i = 0; i < 4; i++ )
	{
		PNIO_printf( "%03d - ", *( pOldIpSuite + i + 4 ) );
		Val = OsKeyScan32( NULL, 10 );
		if( max_val < Val )
		{
			PNIO_printf( "Inputed value too big, truncated to unsigned 8bit/n");
		}
		*( pIpSuite + i + 4 ) = ( PNIO_UINT8 ) Val;
	}

	PNIO_printf ( "Modify 4 bytes of default gateway, decadic input\n" );
	for( i = 0; i < 4; i++ )
	{
		PNIO_printf( "%03d - ", *( pOldIpSuite + i + 8 ) );
		Val = OsKeyScan32( NULL, 10 );
		if( max_val < Val )
		{
			PNIO_printf( "Inputed value too big, truncated to unsigned 8bit/n");
		}
		*( pIpSuite + i + 8 ) = ( PNIO_UINT8 ) Val;
	}
}


/**
 * @brief Input (from user), check and store new ip address
 *
 * @param[in, out]	void	no parameters
 *
 * @return			void	no return value
 *
 *  This function performs a simple input menue, to set a new
 *  ip address. The new ip address is stored afterwards in the
 *  private data block of the nv_data.
 */
PNIO_VOID InputAndStoreIpAddress( PNIO_VOID )
{
	PNIO_UINT8* pIpSuite;
	PNIO_UINT8* pOldIpSuite;
	PNIO_UINT32 IpSuiteLen;
	PNIO_UINT8  TestResult;

	if (0 != NumOfAr)
	{
		PNIO_printf( "Change of IP is forbidden for device with active AR\n" );
		return;
	}

	OsAlloc ( ( void** )&pIpSuite, 0xff, IP_SUITE_LEN );
	/* Get current ip stored in non volatile data */
	/* Incase of dynamic ip allocation returns 0.0.0.0 */
	Bsp_nv_data_restore( PNIO_NVDATA_IPSUITE, ( PNIO_VOID** )&pOldIpSuite, &IpSuiteLen );
	/* print current ip address, input new mac address */
	Usr_Print_Ip_Suite( pOldIpSuite );

	Usr_Read_And_Parse_Ip( pIpSuite, pOldIpSuite );

	Usr_Print_Ip_Suite( pIpSuite );

	TestResult = ParamStoreIpAddress( pIpSuite );

	switch (TestResult)
	{
	    case USR_ADDR_TEST_OK:
	        // no output for user right now, wait for callback
	        break;
	    case USR_ADDR_AR_FORBIDDEN:
	        PNIO_printf( "Change of IP is forbidden for device with active AR\n" );
	        break;
	    case USR_ADDR_NO_CHANGE:
	        PNIO_printf( "New ip suite is same as old one - no need to save it again\n" );
	        break;
	    default:
	        PNIO_printf( "Invalid IP address, problem #0x%02x\n", TestResult );
	        break;
	}

	/* free used memory, new data was copied to pnpbNvData, so it can be freed here */
	OsFree( pOldIpSuite );
	OsFree( pIpSuite );
}

/**
 * @brief Input (from parameter), check and store new ip address
 *
 * @param[in]       PNIO_UINT8*     ip_suite
 *
 * @return          PNIO_UINT8      Result of operation
 *
 *  The new ip address is stored afterwards in the
 *  private data block of the nv_data.
 */
PNIO_UINT8 ParamStoreIpAddress(PNIO_UINT8* IpSuite)
{
    PNIO_UINT32 IpSuiteLen;
    PNIO_UINT8* pOldIpSuite;
    PNIO_UINT8 TestResult = USR_ADDR_AR_FORBIDDEN;

    if (0 == NumOfAr)
    {
        TestResult = Usr_Check_Ip_Suite_Validity(IpSuite);

        if ( USR_ADDR_TEST_OK == TestResult)
        {
            Bsp_nv_data_restore( PNIO_NVDATA_IPSUITE, ( PNIO_VOID** )&pOldIpSuite, &IpSuiteLen );

            if (0 == OsMemCmp(IpSuite, pOldIpSuite, IP_SUITE_LEN))
            {
                TestResult = USR_ADDR_NO_CHANGE;
            }
            else
            {
                /* save new ip to non volatile memory */
                Bsp_nv_data_store(PNIO_NVDATA_IPSUITE, (PNIO_VOID*) IpSuite,
                        IpSuiteLen);

                /* deliver new ip to stack */
                PNIO_change_ip_suite((*(PNIO_UINT32*) IpSuite), (*(PNIO_UINT32*)(IpSuite + 4)), (*(PNIO_UINT32*)(IpSuite + 8)));
            }

            OsFree( pOldIpSuite );
        }
    }

    return TestResult;
}


PNIO_VOID PrintSubmodules( PNIO_VOID )
{
    PNIO_UINT32 i;
    PNIO_UINT32 index = 0; 

    PNIO_ConsolePrintf ( "Plugged Submodules on device \n" );
    for (i = 0; i < IOD_CFG_MAX_NUMOF_SUBSLOTS; i++)
    {
        if (PnpbMod.Sub[i].PlugState == PNPB_PLUGGED)
        {
            PNIO_ConsolePrintf("%02d Slot=%02d Sub=%04x ModID=%02x \n",
                               index,
                               PnpbMod.Sub[i].SlotNum,
                               PnpbMod.Sub[i].SubNum,
                               PnpbMod.Sub[i].ModIdent);
            index++;
        }

    }
}

/**
 * @brief Input (from console), check and plug submodule
 *
 * @param[in]       void        no in parameters
 *
 * @return          void        no return value
 *
 *  Function scans input from user defining new submodule to be plugged and proceeds the plug.
 */
PNIO_VOID InputAndPlugSubmodule( PNIO_VOID )
{
    PNIO_UINT32 ModID, SubID, Slot, InDataLen, OutDataLen, Subslot, i;
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

    PNIO_printf("--Plug of new submodule--\nList of available ModIDs: ");
    for (i = 0; i < MOD_ID_CNT; ++i)
    {
        PNIO_printf("0x%02x ",availableModIdList[i]);
    }
    PNIO_printf("\nList of available SubIDs: ");
    for (i = 0; i < SUB_ID_CNT; ++i)
    {
        PNIO_printf("0x%02x ",availableSubIdList[i]);
    }

    PNIO_printf("\nPick ModID: 0x");
    ModID = OsKeyScan32(NULL,16);
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
    	PNIO_printf("Wrong ModId!\n");
    	return;
    }

    PNIO_printf("Pick SubID: 0x");
    SubID = OsKeyScan32(NULL,16);
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
    	PNIO_printf("Wrong SubId!\n");
    	return;
    }

    PNIO_printf("Pick slot: ");
    Slot = OsKeyScan32(NULL,10);
    PNIO_printf("Pick subslot: ");
    Subslot = OsKeyScan32(NULL,10);

	if (((Slot > IOD_CFG_MAX_SLOT_NUMBER) || (Subslot > IOD_CFG_MAX_NUMOF_SUBSL_PER_SLOT)) ||
		((0 == Slot) && (1 == Subslot)))
	{
		PNIO_printf("Trying to plug to invalid slot or to slot which does not support plug\n");
        return;
	}

    PNIO_printf("Select input data length (0-1440): ");
    InDataLen = OsKeyScan32(NULL, 10);
    if (InDataLen > 1440)
    {
        PNIO_printf("Trying to plug module with invalid data length\n");
        return;
    }

    PNIO_printf("Select output data length (0-1440): ");
    OutDataLen = OsKeyScan32(NULL, 10);
    if (OutDataLen > 1440)
    {
        PNIO_printf("Trying to plug module with invalid data length\n");
        return;
    }

	Addr.Geo.Slot = Slot;            // slot
	Addr.Geo.Subslot = Subslot;         // subslot

	Status = PNIO_sub_plug(PNIO_SINGLE_DEVICE_HNDL,
		PNIO_DEFAULT_API,
		&Addr,                              // location (slot, subslot)
		ModID,
		SubID,                              // submodule identifier
        InDataLen,
        OutDataLen,
		PNIO_IM0_SUBMODULE,                 // Submodule supports IM0
		(IM0_DATA*)NULL,
		PNIO_S_GOOD);                        // initial iops value, only for submodules without io data
	if (PNIO_OK == Status)
	{
		PNIO_printf("Plug done\n");
	}
	else
	{
		PNIO_ConsolePrintf("Error %x occured\n", PNIO_get_last_error());
	}
}

/**
 * @brief Input (from console), pull submodule
 *
 * @param[in]       void        no in parameters
 *
 * @return          void        no return value
 *
 *  Function scans input from user defining submodule to be pulled and proceeds the pull.
 */
PNIO_VOID InputAndPullSubmodule(PNIO_VOID)
{

    PNIO_UINT32 Slot, Subslot;
    PNIO_DEV_ADDR  Addr;
    PNIO_UINT32 Status;

    PNIO_printf("--Pull of existing submodule--\n");

    /*let user pick slot and subslot via console input*/
    PNIO_printf("Pick slot: ");
    Slot = OsKeyScan32(NULL,10);
    PNIO_printf("Pick subslot: ");
    Subslot = OsKeyScan32(NULL,10);
	
	if(((Slot > IOD_CFG_MAX_SLOT_NUMBER) || (Subslot > IOD_CFG_MAX_NUMOF_SUBSL_PER_SLOT)) ||
		((0 == Slot) && (1 == Subslot)))
	{
		PNIO_printf("Trying to pull invalid module or module which does not support pull\n");
	}

	else
	{
		Addr.Geo.Slot = Slot;            // slot
		Addr.Geo.Subslot = Subslot;         // subslot

		/*Pull from specified slot and subslot*/
		Status = PNIO_sub_pull(PNIO_SINGLE_DEVICE_HNDL,
			PNIO_DEFAULT_API,
			&Addr);         // location (slot, subslot)
		if (Status != PNIO_OK)
		{
			PNIO_ConsolePrintf("Error %x occured\n", PNIO_get_last_error());
		}
		else
		{
			PNIO_ConsolePrintf("pull done\n");
		}
	}
}

/**
 * @brief Input (from user), check and store new device name
 *
 * @param[in, out]	void	no parameters
 *
 * @return			void	no return value
 *
 *  This function performs a simple input menue, to set a new
 *  device name. The new device name is stored afterwards in the
 *  private data block of the nv_data.
 */
PNIO_VOID InputAndStoreDeviceName( PNIO_VOID )
{
	PNIO_UINT8* pDevName, * pOldDevName;
	PNIO_UINT32 DevNameLen, OldDevNameLen;
	PNIO_UINT8  TestResult;

	OsAlloc ( ( void** )&pDevName, 0xff, DEVICE_NAME_MAXLEN );

	/* Get current device name stored in non volatile data */
	Bsp_nv_data_restore( PNIO_NVDATA_DEVICENAME, ( PNIO_VOID** )&pOldDevName, &OldDevNameLen );
	OsMemCpy( pDevName, pOldDevName, OldDevNameLen );

	/* add \0 after text string, so it will end printf*/
	pDevName[ OldDevNameLen ] = 0x00;	/* \0 */

	/* print current device name, input new mac address */
	PNIO_printf( "\nCurrent device name: %s\n" , pDevName );
	PNIO_printf( "Set new device name, max length 240, longer name will be rejected\n" );
	PNIO_printf( "New device name: " );

	DevNameLen = OsKeyScanString( NULL, pDevName, DEVICE_NAME_MAXLEN );

	TestResult = ParamStoreDeviceName(pDevName, DevNameLen);

    switch (TestResult)
    {
        case USR_ADDR_TEST_OK:
            // no output for user right now, wait for callback
            break;
        case USR_ADDR_AR_FORBIDDEN:
            PNIO_printf( "Change of device name is forbidden for device with active AR\n" );
            break;
        case USR_ADDR_NO_CHANGE:
            PNIO_printf( "New device name is same as old one - no need to save it again\n" );
            break;
        default:
            PNIO_printf( "Invalid device name, problem #0x%02x\n", TestResult );
            break;
    }

	/* free used memory, new data was copiend to pnpbNvData, so it can be freed here */
	OsFree( pDevName );
	OsFree( pOldDevName );
}

/**
 * @brief Input (from parameter), check and store new device name
 *
 * @param[in]       PNIO_UINT8*    dev_name
 *
 * @return          PNIO_UINT8     result of operation
 *
 *  The new device name is stored afterwards in the
 *  private data block of the nv_data.
 */
PNIO_UINT8 ParamStoreDeviceName( PNIO_UINT8* DevName, PNIO_UINT32 DevNameLen )
{
    PNIO_UINT8* pOldDevName;
    PNIO_UINT32 OldDevNameLen;
    PNIO_UINT8  TestResult = USR_ADDR_AR_FORBIDDEN;

    if( 0 == NumOfAr)
    {
        TestResult = Usr_Check_Device_Name_Validity( DevName, DevNameLen );

        if( USR_ADDR_TEST_OK == TestResult )
        {
            /* Get current device name stored in non volatile data */
            Bsp_nv_data_restore( PNIO_NVDATA_DEVICENAME, ( PNIO_VOID** )&pOldDevName, &OldDevNameLen );

            /* add \0 after text string, so it will end printf*/
            pOldDevName[ OldDevNameLen ] = 0x00;   /* \0 */

            if( ( 0 == OsMemCmp( DevName, pOldDevName, DevNameLen ) ) &&
                    (OldDevNameLen == DevNameLen) )
            {
                TestResult = USR_ADDR_NO_CHANGE;
            }
            else
            {
                /* store new device name to flash memory */
                Bsp_nv_data_store( PNIO_NVDATA_DEVICENAME, ( PNIO_VOID* )DevName, DevNameLen );

                // send device name to stack
                PNIO_change_device_name(( PNIO_INT8* )DevName, ( PNIO_UINT16 )DevNameLen);
            }

            /* free used memory, new data was copiend to pnpbNvData, so it can be freed here */
            OsFree( pOldDevName );
        }
    }

    return TestResult;
}


 // *----------------------------------------------------------------*
 // *
 // *  StoreDapModuleId  ()
 // *
 // *----------------------------------------------------------------*
 // *  stores the module ID of the DAP into the private data block
 // *  of the flash memory.
 // *  setup of the private data is
 // *
 // *
 // *  byte structure of the PRIVATE DATA in flash memory:
 // *
 // *
 // *  OFFSET         CONTENT
 // *  ------         -------
 // *  516...639      free for other user data
 // *  512...515      Dap module ID
 // *    0...511      BOOTLINE
 // *
 // *
 // *----------------------------------------------------------------*
 // *  Input:     --
 // *----------------------------------------------------------------*
void StoreDapModuleId (PNIO_UINT32 DapModuleId)
 {
	 PNIO_UINT8*  pPrivDat;
	 PNIO_UINT32  PrivDatLen = 0;
	 PNIO_UINT32* pDapModuleId;

	 // **** load image of private data from flash into ram ***
	 Bsp_nv_data_restore (PNIO_NVDATA_PRIVATE_DATA, (PNIO_VOID**)&pPrivDat, &PrivDatLen);

     // **** set DAP module id in flash image in ram ****
     pDapModuleId = (PNIO_UINT32*) (pPrivDat + NV_PRIVDATA_MODID_OFFS);
     *pDapModuleId = DapModuleId;
     *(pDapModuleId+1) = DAP_MODULE_ID_VALID;

	 // *** restore image into flash memory ****
	 Bsp_nv_data_store (PNIO_NVDATA_PRIVATE_DATA, (PNIO_VOID*) pPrivDat, PrivDatLen);

 }



 // *----------------------------------------------------------------*
 // *
 // *  RestoreDapModuleId  ()
 // *
 // *----------------------------------------------------------------*
 // *  restores the module ID of the DAP from the private data block
 // *  in the flash memory.
 // *----------------------------------------------------------------*
 // *  Input:     --
 // *----------------------------------------------------------------*
PNIO_UINT32 RestoreDapModuleId (PNIO_UINT32* pDapModuleId)
{
	PNIO_UINT8* pPrivDat;
	PNIO_UINT32 PrivDatLen = 0;
	PNIO_UINT32* pPrivDatDapModuleId;
	PNIO_UINT32 Status;

	// **** get the current mac address from flash memory ***
	Bsp_nv_data_restore (PNIO_NVDATA_PRIVATE_DATA, (PNIO_VOID**)&pPrivDat, &PrivDatLen);

	pPrivDatDapModuleId = (PNIO_UINT32*) (pPrivDat + NV_PRIVDATA_MODID_OFFS);

	// *** return valid dap module id with PNIO_TRUE, else no valid id in flash ***
	if (*(pPrivDatDapModuleId + 1) == DAP_MODULE_ID_VALID)
	{
		*pDapModuleId = *pPrivDatDapModuleId;
		Status = PNIO_TRUE;
	}
	else
	{
		*pDapModuleId = 0;
		Status = PNIO_FALSE;
	}

    // *** free ram block with private data, when no more needed ***
	Bsp_nv_data_memfree (pPrivDat);

	return (Status);
}


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
