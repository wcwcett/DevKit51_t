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
/*  F i l e               &F: usriod_utils.h                            :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  header file to usriod_utils.c.                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/

#ifndef _USR_IOD_UTILS_H
	#define _USR_IOD_UTILS_H

	#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
	extern "C"
	{
	#endif

	PNIO_UINT32 NumOfAr;    // number of running ARs

	PNIO_UINT32 PrintDevkitVersion (void);

	void SetGpioLed (PNIO_UINT8 OnOff);

    PNIO_UINT32 usr_rec_check_blockhdr
            (
                REC_IO_BLOCKHDR* pBlockHdr,      // pointer to real blockheader
                PNIO_UINT16         BlockType,      // expected block type
                PNIO_UINT8          BlockLen,       // expected block length
                PNIO_UINT16         BlockVers,      // expected block version
                PNIO_ERR_STAT       *pPnioState     // return 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
            );

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
			);

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

			);



    // * ------------------------------------------
    // *  structure of the  PRIVATE DATA in flash
    // *  (this is only an example on EB200/400)
    // *
    // *  OFFSET         CONTENT
    // *  ------         -------
    // *    0...3        Dap module ID
    // *
    // * ------------------------------------------
    // **** structure of private data on EB200/400 ***
    #define NV_PRIVDATA_MODID_OFFS      0                   /* Maximum of the boot line */
    #define MOD_ID_CNT                  13
    #define SUB_ID_CNT                  1

    #define SUBMODULE_ID_1              1

    void        StoreDapModuleId 		( PNIO_UINT32 DapModuleId );      /* store DAP module id into flash */
    PNIO_UINT32 RestoreDapModuleId 		( PNIO_UINT32* pDapModuleId );  	/* restore DAP module id from flash */
	void        InputAndStoreMacAddress ( void );                  		/* input mac adress and store into flash */
	PNIO_VOID 	InputAndStoreIpAddress	( PNIO_VOID );					/* input new ip set and store into flash */
	PNIO_VOID 	InputAndStoreDeviceName	( PNIO_VOID );					/* input new device name and store into flash */
	PNIO_VOID   InputAndPlugSubmodule   ( PNIO_VOID );
	PNIO_VOID   InputAndPullSubmodule   ( PNIO_VOID );
	PNIO_VOID   PrintSubmodules         ( PNIO_VOID );

	PNIO_UINT32 TrcStoreBuf				( void );

	PNIO_UINT8   ParamStoreMacAddress ( PNIO_UINT8* MacAddr );    /* store mac adress from parameter into flash */
    PNIO_UINT8   ParamStoreIpAddress  ( PNIO_UINT8* IpSuite );    /* set new ip from parameter and store into flash */
    PNIO_UINT8   ParamStoreDeviceName ( PNIO_UINT8* DevName, PNIO_UINT32 DevNameLen );    /* update device name from parameter and store into flash */

	#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
	}
	#endif
#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
