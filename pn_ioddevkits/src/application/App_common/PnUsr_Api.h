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
/*  F i l e               &F: PnUsr_Api.h                               :F&  */
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
/*  definition header file for the simple IO interface                       */
/*                                                                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/
#ifndef _PNUSR_API_H
#define _PNUSR_API_H

    #ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
    extern "C"
    {
    #endif

    typedef struct
    {
        PNIO_UINT16    VendorId;                    // Vendor ID
        PNIO_UINT16    DeviceId;                    // Device ID
        PNIO_INT8*     pDevType;                    // pointer to string with device type (zero terminated)
        PNIO_UINT32    MaxNumOfSubslots;            // maximum number of subslots, managable by PN Stack
        PNIO_UINT32    MaxNumOfBytesPerSubslot;     // maximum number of bytes per subslots, managable by PN Stack
    } PNUSR_DEVICE_INSTANCE;


    typedef struct
    {
    	PNIO_UINT32    Version;                     //
        PNIO_UINT32    Result;	                    //
        PNIO_UINT32    BufferSize;	                //
        PNIO_UINT8     BufferData[4];               //
    } PNIO_TRACE_BUFFER;

    typedef PNIO_TRACE_BUFFER* PNIO_TRACE_BUFFER_PTR;

    //systest app watchdog retriggering enabled
    PNIO_UINT8	  PNIO_WD_triggering;
    PNIO_UINT8	  PNIO_dynamic_cfg;
    PNIO_UINT8 	  PNIO_automatic_alrms;
    PNIO_UINT8	  PNIO_alrm_in_progress;
    //dynamic config - replace original vars
#if (EXAMPL_DEV_CONFIG_VERSION == 9)
#define DYNAMIC_CONFIG
#endif

#ifdef DYNAMIC_CONFIG
    PNIO_SUB_LIST_ENTRY IoSubList[ IOD_CFG_MAX_NUMOF_SUBSLOTS ];
    PNIO_IM0_LIST_ENTRY Im0List[ IOD_CFG_MAX_NUMOF_SUBSLOTS ];
#endif

#define PNUSR_CODE_FAST                          OS_CODE_FAST

    // ***** startup the PNIO stack and plug the real configuration ****
    PNIO_BOOL PnUsr_DeviceSetup (PNUSR_DEVICE_INSTANCE  *pPnUsrDev,           // device setup configuration
                                 PNIO_SUB_LIST_ENTRY    *pIoSubList,          // plugged submodules, including PDEV
                                 PNIO_UINT32            NumOfSubListEntries,  // number of entries in pPioSubList
                                 PNIO_IM0_LIST_ENTRY    *pIm0List,            // list of IM0 data sets
                                 PNIO_UINT32            NumOfIm0ListEntries); // number of entries in pIm0List

    void PNIOUSR_device_start( PNUSR_DEVICE_INSTANCE  *pPnUsrDev,           // device setup configuration
                               PNIO_SUB_LIST_ENTRY    *pIoSubList,          // plugged submodules, including PDEV
                               PNIO_UINT32            NumOfSubListEntries,  // number of entries in pPioSubList
                               PNIO_IM0_LIST_ENTRY    *pIm0List,            // list of IM0 data sets
                               PNIO_UINT32            NumOfIm0ListEntries); // number of entries in pIm0List

    // ***--------------------------------------------------------
    // ***
    // ***  IO DATA EXCHANGE
    // ***
    // ***--------------------------------------------------------
    PNIO_BOOL PnUsr_cbf_IoDatXch        (void);     // trigger to start IO data exchange (TRANS_END event)
    PNIO_VOID PnUsr_ActivateIoDatXch    (void);     // activate IO Data exchange in task Task_CycleIO
    PNIO_VOID PnUsr_DeactivateIoDatXch  (void);     // deactivate IO Data exchange in task Task_CycleIO

    // ***--------------------------------------------------------
    // ***
    // ***  RECORD READ / WRITE
    // ***
    // ***--------------------------------------------------------
    PNIO_UINT32  PnUsr_cbf_rec_read
	(
		PNIO_UINT32			DevHndl,        // device handle
		PNIO_UINT32			Api,            // application process identifier
        PNIO_UINT16         ArNum,			// ar - handle
        PNIO_UINT16 		SessionKey,	    // ar session number
		PNIO_UINT32			SequenceNum,    // CLRPC sequence number
		PNIO_DEV_ADDR		*pAddr,			// geographical or logical address
		PNIO_UINT32			RecordIndex,    // record index
		PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
		PNIO_UINT8			*pBuffer,		// [in] buffer pointer
		PNIO_ERR_STAT		*pPnioState		// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
	);

    PNIO_UINT32  PnUsr_cbf_rec_write
	(
		PNIO_UINT32			DevHndl,        // device handle
		PNIO_UINT32			Api,            // application process identifier
        PNIO_UINT16         ArNum,			// ar - handle
        PNIO_UINT16 		SessionKey,	    // ar session number
		PNIO_UINT32			SequenceNum,    // CLRPC sequence number
		PNIO_DEV_ADDR		*pAddr,			// geographical or logical address
		PNIO_UINT32			RecordIndex,    // record index
		PNIO_UINT32			*pBufLen,   	// [in, out] in: length to write, out: length, written by user
		PNIO_UINT8			*pBuffer,		// [in] buffer pointer
		PNIO_ERR_STAT		*pPnioState		// 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
	);

    // ***--------------------------------------------------------
    // ***
    // ***  Blink LED
    // ***
    // ***--------------------------------------------------------
    PNIO_VOID  InitLedBlink         (PNIO_VOID);


    #ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
    }
    #endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
