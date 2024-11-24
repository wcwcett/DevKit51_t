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
/*  F i l e               &F: iodapi_event.h                            :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  declaration of the user defined callback functions                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/
#ifndef _IODAPIEVENT_H
#define _IODAPIEVENT_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

    // *---------------------------------------------------------
    // *	AR management
    // *---------------------------------------------------------
    typedef struct 
    {
        PNIO_UINT16 ArNum;
        PNIO_UINT16 SessionKey;
        PNIO_UINT16 IocrProperties;  // specifies RT class 1, 2, 3 (all iocr's of an AR have the same value)
    } AR_PROP;


    //--------------------------------------------------------------------------------------
    // READ RECORD, WRITE RECORD:
    //--------------------------------------------------------------------------------------

    typedef struct
    {
	    PNIO_UINT32			DevHndl;        // device handle
	    PNIO_UINT32			Api;            // application process identifier
        PNIO_UINT16         ArNum;			// ar - handle
        PNIO_UINT16 		SessionKey;	    // ar session number
	    PNIO_UINT32			SequenceNum;    // CLRPC sequence number
	    PNIO_DEV_ADDR       Addr;		    // location (module/submodule)
        PNIO_UINT32			RecordIndex;    // record index
        PNIO_UINT32			BufLen;		    // record data length
        PNIO_VOID*          pRqHnd;         // Requestblock pointer, used as Handle 
    }  USR_REC_READ_RQ;

    typedef struct
    {
	    PNIO_UINT32			DevHndl;        // device handle
	    PNIO_UINT32			Api;            // application process identifier
        PNIO_UINT16         ArNum;			// ar - handle
        PNIO_UINT16 		SessionKey;	    // ar session number
	    PNIO_UINT32			SequenceNum;    // CLRPC sequence number
	    PNIO_DEV_ADDR       Addr;		    // location (module/submodule)
        PNIO_UINT32			RecordIndex;    // record index
        PNIO_UINT32			BufLen;		    // record data length	
	    PNIO_UINT8			*pBuffer;		// record data buffer pointer
        PNIO_VOID*          pRqHnd;         // Requestblock pointer, used as Handle 
    }  USR_REC_WRITE_RQ;

    PNIO_EXTRN PNIO_UINT32  usr_rec_read_response_async (void);
    PNIO_EXTRN PNIO_UINT32  usr_rec_write_response_async (void);



    //--------------------------------------------------------------------------------------
    // iPAR server example 
    //--------------------------------------------------------------------------------------
    // ****** example for special features (must be modified by user)
    #define EXAMPLE_IPAR_SERVER     1       // 1: support iPar - example data on record index 50, 0: else


    // ***** EXAMPLE IPAR:  upload&storage alarm ********
    #define EXAMPL_IPAR_SLOTNUM             1                           // slot number
    #define EXAMPL_IPAR_SUBSLOTNUM          1                           // subslot number, must be 1
    #define EXAMPL_IPAR_REC_IND             50                          // free defined by the user
    #define EXAMPL_IPAR_UPL_LEN             4                           // length of ipar data for upload
    #define EXAMPL_IPAR_RTV_LEN             4                           // length of ipar data for retrieval (0: no limit)


    //--------------------------------------------------------------------------------------
    // structure of special records 
    //--------------------------------------------------------------------------------------
    // *--------------------------------------
    // * record 0x8030, see IEC61158-6  "IsochronousModeData"
    // *--------------------------------------
    typedef  ATTR_PNIO_PACKED_PRE struct
    {
        REC_IO_BLOCKHDR     BlockHeader;
        PNIO_UINT8          Padding[2];
        PNIO_UINT16         SlotNumber;
        PNIO_UINT16         SubSlotNumber;
        PNIO_UINT16         ControllerApplicationCycleFactor;
        PNIO_UINT16         TimeDataCycle;
        PNIO_UINT32         TimeIOInput;         // value in nsec
        PNIO_UINT32         TimeIOOutput;        // value in nsec
        PNIO_UINT32         TimeIOInputValid;    // value in nsec
        PNIO_UINT32         TimeIOOutputValid;   // value in nsec
    }  ATTR_PNIO_PACKED REC8030_ISOMDATA;  


#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
