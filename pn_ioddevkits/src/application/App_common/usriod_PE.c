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
/*  F i l e               &F: usriod_PE.c                               :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  PROFIenergy (PE) example code (implements an PE server)                  */
/*                                                                           */
/*                                                                           */
/*  THIS MODULE HAS TO BE MODIFIED BY THE PNIO USER                          */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/

/**
* @file     usriod_PE.c
* @brief    functions for of Profienergy functionality
*
* A PROFIenergy (PE)  request is initiated from a PE-client with a write record
*  request. The PE server checks the request, provides the response data and
*  stores them in a static memory buffer.
*  Reading the PE response data is initiated from the PE client with a read record
*  request.
*
*  Notes:
*   - correct sequence musts be RecWr1 ==> RecRd1,  RecW2r ==> RecRd2,...
*   - case RecWr1 ==> RecWr2:  in this case  the PE server has 2 options:
*      a) if data providing for RecWr1 is pending, PE server can respond to RecWr2 with error "resource busy"
*      b) if data providing for RecWr1 is finished, Rec1Wr can be deleted and PE server responds to RecWr2
*         in a correct RecRd2
*/


#include "compiler.h"
#include "os.h"
#include "pniousrd.h"
#include "iodapi_event.h"
#include "usriod_cfg.h"
#include "usriod_PE.h"
#include "usriod_utils.h"
#include "PnUsr_xhif.h"

#if (1 != IOD_USED_WITH_XHIF_HOST)
/* ************ static data *********** */
static PNIO_UINT8  CurrentPE_Mode        = PE_MODE_READY_TO_OPERATE;  /* current power save mode */

#if (IOD_CFG_NUMOF_IO_AR == 1)
static PNIO_UINT8  ResponseDataAvailable[IOD_CFG_NUMOF_AR + 1] = {PNIO_FALSE,PNIO_FALSE};
#else
static PNIO_UINT8  ResponseDataAvailable[IOD_CFG_NUMOF_AR + 1] = {PNIO_FALSE,PNIO_FALSE,PNIO_FALSE,PNIO_FALSE,PNIO_FALSE};
#endif

#define RSP_MAXBUFSIZE  ( sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_GET_MODE ) + 20 )

PNIO_UINT8   PE_RspBuffer[ IOD_CFG_NUMOF_AR + 1 ][ RSP_MAXBUFSIZE ] = {0};  /* buffer for PE response */
PNIO_UINT32  PE_RspLen[IOD_CFG_NUMOF_AR + 1];                       /* length of PE response (must be <= RSP_MAXBUFSIZE) */
//lint --e(578) Declaration of symbol 'Symbol' hides symbol 'Symbol' (Location)
typedef ATTR_PNIO_PACKED_PRE struct
{
    PE_SUM_REQ_HDR Hdr;          /* blockheader + service header */
    PNIO_UINT8    Sdr[ 2 ];      /* the first 2 bytes of service data request block */
} ATTR_PNIO_PACKED   LAST_PE_RQ;


/**
 *  @brief Endiannes change
 *
 *  @param[in]   	*pDst 		pointer to destination value in network format
 *  @param[in] 		*pSrc 		pointer to source value in device format
 *  @return      	void
 *
 *  Internal functionality
 *  copies 32 bit value in unaligned structures and transfers the
 *  value from host format (big or little endian, machine dependent)
 *  into the network format (big endian)
 *
 */
static void SetVal32_Htonl ( PNIO_VOID* pDst, PNIO_VOID* pSrc )
{
    PNIO_UINT8* pDst8 = ( PNIO_UINT8* )pDst;
    PNIO_UINT8* pSrc8 = ( PNIO_UINT8* )pSrc;

    #if PNIO_BIG_ENDIAN
        *( pDst8 + 0 ) = *( pSrc8 + 0 );
        *( pDst8 + 1 ) = *( pSrc8 + 1 );
        *( pDst8 + 2 ) = *( pSrc8 + 2 );
        *( pDst8 + 3 ) = *( pSrc8 + 3 );
    #else
        *( pDst8 + 0 ) = *( pSrc8 + 3 );
        //lint -e{415} controlled block of pointer
        *( pDst8 + 1 ) = *( pSrc8 + 2 );
        //lint -e{415,416} controlled block of pointer
        *( pDst8 + 2 ) = *( pSrc8 + 1 );
        //lint -e{415,416} controlled block of pointer
        *( pDst8 + 3 ) = *( pSrc8 + 0 );
    #endif
}


/**
 *  @brief Endiannes change
 *
 *  @param[in]   	*pDst 		pointer to destination value in network format
 *  @param[in] 		*pSrc 		pointer to source value in device format
 *  @return      	void
 *
 *  Internal functionality
 *  copies 16 bit value in unaligned structures and transfers the
 *  value from host format (big or little endian, machine dependent)
 *  into the network format (big endian)
 *
 */
static void SetVal16_Htons ( PNIO_VOID* pDst, PNIO_VOID* pSrc )
{
    PNIO_UINT8* pDst8 = ( PNIO_UINT8* )pDst;
    PNIO_UINT8* pSrc8 = ( PNIO_UINT8* )pSrc;

    #if PNIO_BIG_ENDIAN
        *( pDst8 + 0 ) = *( pSrc8 + 0 );
        *( pDst8 + 1 ) = *( pSrc8 + 1 );
    #else
        *( pDst8 + 0 ) = *( pSrc8 + 1 );
        *( pDst8 + 1 ) = *( pSrc8 + 0 );
    #endif
}


/**
 *  @brief Sets error in case of invalid PE command
 *
 *  @param[in]   	*pRspBuf 		response buffer pointer (for later record read request)
 *  @param[in] 		  PE_ErrNum 	PE error code
 *  @return      					Returns	length of response
 *
 *  Internal functionality
 *
 */
static PNIO_UINT32  PE_SetErrRsp
    (
      PNIO_UINT8		*pRspBuf,		/* [in] response buffer pointer (for later record read request) */
	  PNIO_UINT8         PE_ErrNum		/* [in] PE error code */
    )
{
    PE_SDR_RSP_ERROR* pRspErr    = ( PE_SDR_RSP_ERROR* )( pRspBuf + sizeof( PE_SUM_RSP_HDR ) );
    PE_SUM_RSP_HDR*   pCmdRspHdr = ( PE_SUM_RSP_HDR* )    pRspBuf;
    PNIO_UINT32       RspLen;

    PNIO_printf( "Request not succesfull, responding with error code 0x%x\n", PE_ErrNum );

    /* ***adapt blockheader *** */
    pCmdRspHdr->BlockHdr.Type  = OsHtons( 0x0801 );
    pCmdRspHdr->BlockHdr.Len   = OsHtons( sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_ERROR ) - 4 );

    /* *** set service header response **** */
    pCmdRspHdr->RspHdr.Status        = PE_RSP_STAT_READY_ERROR ;  /* ready */
    pCmdRspHdr->RspHdr.DatStrucIdRsp = 0xff;                      /* shall be ff on error */

    /* *** set service data response **** */
    pRspErr->ErrorCode = PE_ErrNum;   /* error code */
    pRspErr->reserved  = 0;

    RspLen = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_ERROR );   /* save response length for later record read */
    return( RspLen );
}


/**
 *  @brief Plausibility check of block header and service header request
 *
 *  @param[in]   	 *pReqBuf 			RQ:  saved PE-RQ from record write
 *  @param[in] 		 ExpModifier 		RQ:  expected RQ modifier value
 *  @param[in] 		 ExpDatStrucIdRq 	RQ:  expected RQ DataStrucIdRQ value
 *  @param[out] 	 *pRspBuf 			RSP: buffer, starting at RSP-blockheader
 *  @param[in] 		 BlockLen 			RSP: length value in blockheader
 *  @param[in] 		 DataStrucIdRs_ok 	RSP: data structure identifier response, of ok
 *
 *  @return      						Returns	PE_RET_OK (0) if OK or PE error code
 *
 *	internal functionality
 *
 */
static PNIO_UINT8 CheckRqHdr_SrvHdr (PNIO_VOID*     pReqBuf,            /* [in]  RQ:  saved PE-RQ from record write */
                                     PNIO_UINT8     ExpModifier,        /* [in]  RQ:  expected RQ modifier value */
                                     PNIO_UINT8     ExpDatStrucIdRq,    /* [in]  RQ:  expected RQ DataStrucIdRQ value */
                                     PNIO_UINT8*    pRspBuf,            /* [out] RSP: buffer, starting at RSP-blockheader */
                                     PNIO_UINT16    BlockLen,           /* [in]  RSP: length value in blockheader */
                                     PNIO_UINT8     DataStrucIdRs_ok)   /* [in]  RSP: data structure identifier response, of ok */
{
    PE_SUM_REQ_HDR* pCmdReqHdr = ( PE_SUM_REQ_HDR* )pReqBuf;
    PE_SUM_RSP_HDR* pCmdRspHdr = ( PE_SUM_RSP_HDR* )pRspBuf;

    /* *** copy request header into response header (in the response buffer) *** */
    OsMemCpy( ( PNIO_VOID* ) pRspBuf, ( PNIO_VOID* ) pReqBuf, sizeof( PE_SUM_REQ_HDR ) );

    /* ***adapt blockheader *** */
    pCmdRspHdr->BlockHdr.Type  = OsHtons( 0x0801 );
    pCmdRspHdr->BlockHdr.Len   = OsHtons( BlockLen );

        /* *** plausibility check service header  **** */
    if( pCmdReqHdr->ReqHdr.Modifier != ExpModifier )
    {
        return( PE_RET_INVALID_MODIFIER );
    }

    /* *** plausibility check service header  **** */
    if( pCmdReqHdr->ReqHdr.DatStrucIdRq != ExpDatStrucIdRq )
    {
        return( PE_RET_INVALID_DATA_STRUCT_IDENT_RQ );
    }

    /* *** ok, no error found *** */
    pCmdRspHdr->RspHdr.DatStrucIdRsp    = DataStrucIdRs_ok ;  /* ready */
    pCmdRspHdr->RspHdr.Status           = PE_RSP_STAT_READY_OK ;  /* ready */
    return( PE_RET_OK );
}


/**
 *  @brief Request handler for PE command "start pause"
 *
 *  @param[in]   	*pRqBuf 		request buffer pointer (from record write request)
 *  @param[in] 		*pRspBuf 		response buffer pointer (for later record read request)
 *  @return      					Returns	PNIO_OK (1) if OK
 *
 *  This function turns the device into power save mode
 *  CPU sends the expected length of power save mode in parameter
 *  Example functionality
 */
static PNIO_UINT32  PE_start_pause_RQ
    (
      PNIO_UINT8			*pRqBuf,		/* [in] request buffer pointer (from record write request) */
      PNIO_UINT8			*pRspBuf, 		/* [in] response buffer pointer (for later record read request) */
      PNIO_UINT16           ArNum           /* [in] number of AR initiating request*/
    )
{
    PE_SDR_REQ_START_PAUSE* pRqStartP  = ( PE_SDR_REQ_START_PAUSE* )( pRqBuf  + sizeof( PE_SUM_REQ_HDR ) );
    PE_SDR_RSP_START_PAUSE* pRspStartP = ( PE_SDR_RSP_START_PAUSE* )( pRspBuf + sizeof( PE_SUM_RSP_HDR ) );
    PNIO_UINT8 ErrCode;

    /* *** plausibility check Modifier, DataStrucIdRQ, sets complete header (always) and complete response on error *** */
    ErrCode = CheckRqHdr_SrvHdr( pRqBuf,	/* [in]  RQ:  saved PE-RQ from record write */
                                  0x00,		/* [in]  RQ:  expected modifier value */
                                  0x01,		/* [in]  RQ:  expected DataStrucIdRQ value */
                                  pRspBuf,	/* [out] RSP: response header */
								  	  	    /* [in]  RSP: length value in blockheader */
                                  sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_START_PAUSE ) - 4,
                                  0x01);	/* [out] RSP: DataStrucIdRs, if no error */

    /* *************************************************************** */
    /*     BEGIN  user code */

        if( ErrCode == PE_RET_OK )
        {
        	/* if ok, set data structure identifier RS and service data response values *** */
        	PNIO_printf ( "PROFIenergy command START_PAUSE, time = %dmsec  received\n", OsHtonl( pRqStartP->PauseTime ) );

        	/* we assume in this example , expected power save mode has already been achieved... */
            CurrentPE_Mode          = PE_MODE_PAUSE1;
            pRspStartP->PE_ModeId   = CurrentPE_Mode;
            pRspStartP->reserved    = 00;
            PE_RspLen[ArNum] = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_START_PAUSE );
        }
        else
        {
            PE_RspLen[ArNum] = PE_SetErrRsp( pRspBuf, ErrCode );
        }
    /*     END    user code */
    /* *************************************************************** */

    return( PNIO_OK );
}


/**
 *  @brief Request handler for PE command "end pause"
 *
 *  @param[in]   	*pRqBuf 		request buffer pointer (from record write request)
 *  @param[in] 		*pRspBuf 		response buffer pointer (for later record read request)
 *  @return      					Returns	PNIO_OK (1) if OK
 *
 *  This function begins switching the device to ready_to_operate state
 *  and responds to CPU with expected time to reach the state
 *  Example functionality
 */
static PNIO_UINT32  PE_end_pause_RQ
    (
      PNIO_UINT8			*pRqBuf,		/* [in] request buffer pointer (from record write request) */
      PNIO_UINT8			*pRspBuf, 		/* [in] response buffer pointer (for later record read request) */
      PNIO_UINT16           ArNum           /* [in] number of AR initiating request*/
    )
{
    PE_SDR_RSP_END_PAUSE* pRspEndP = ( PE_SDR_RSP_END_PAUSE* )( pRspBuf + sizeof( PE_SUM_RSP_HDR ) );
    PNIO_UINT8 ErrCode;
    PNIO_UINT32 val32;

    /* *** plausibility check Modifier, DataStrucIdRQ, sets complete header (always) and complete response on error *** */
    ErrCode = CheckRqHdr_SrvHdr( pRqBuf,	/* [in]  RQ:  saved PE-RQ from record write */
                                  0x00,		/* [in]  RQ:  expected modifier value */
                                  0x00,		/* [in]  RQ:  expected DataStrucIdRQ value */
                                  pRspBuf,	/* [out] RSP: response header */
								  	  	    /* [in]  RSP: length value in blockheader */
                                  sizeof(PE_SUM_RSP_HDR) + sizeof(PE_SDR_RSP_END_PAUSE) - 4,
                                  0x01);	/* [out] RSP: DataStrucIdRs, if no error */

    /* *************************************************************** */
    /*     BEGIN  user code */

        if( ErrCode == PE_RET_OK )
        {
            /* if ok, set data structure identifier RS and service data response values *** */
            PNIO_printf ( "PROFIenergy command END_PAUSE received\n" );

            val32 = 6000; /* example: 6 sec until "ready to operate" */
            SetVal32_Htonl(&pRspEndP->TimeToOperate, &val32);

			/* we assume in this example , expected power save mode has already been achieved... */
            CurrentPE_Mode          = PE_MODE_READY_TO_OPERATE;
            PE_RspLen[ArNum] = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_END_PAUSE );
            return( PNIO_OK );
        }
        else
        {
            PE_RspLen[ArNum] = PE_SetErrRsp( pRspBuf, ErrCode );
        }
    /*     END    user code */
    /* *************************************************************** */

    return( PNIO_OK );
}


/**
 *  @brief Request handler for PE command "query modes"
 *
 *  @param[in]   	*pRqBuf 		request buffer pointer (from record write request)
 *  @param[in] 		*pRspBuf 		response buffer pointer (for later record read request)
 *  @return      					Returns	PNIO_OK (1) if OK
 *
 *  This function responds to CPU with list of available power save modes
 *  Example functionality
 */
static PNIO_UINT32  PE_query_modes_RQ
    (
      PNIO_UINT8			*pRqBuf,		/* [in] request buffer pointer (from record write request) */
      PNIO_UINT8			*pRspBuf, 		/* [in] response buffer pointer (for later record read request) */
      PNIO_UINT16           ArNum           /* [in] number of AR initiating request*/
    )
{
    PE_SDR_RSP_QUERY_MODES* pRspQueryM = ( PE_SDR_RSP_QUERY_MODES* )( pRspBuf + sizeof( PE_SUM_RSP_HDR ) );
    PNIO_UINT8 ErrCode;

    /* *** plausibility check Modifier, DataStrucIdRQ, sets complete header (always) and complete response on error *** */
    ErrCode = CheckRqHdr_SrvHdr  (pRqBuf,	/* [in]  RQ:  saved PE-RQ from record write */
                                  0x01,		/* [in]  RQ:  expected modifier value */
                                  0x00,		/* [in]  RQ:  expected DataStrucIdRQ value */
                                  pRspBuf,	/* [out] RSP: response header */
								  	  	    /* [in]  RSP: length value in blockheader */
                                  sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_QUERY_MODES ) - 4,
                                  0x01);	/* [out] RSP: DataStrucIdRs, if no error */

    /* *************************************************************** */
    /*     BEGIN  user code */

        if( ErrCode == PE_RET_OK )
        {
            /* if ok, set data structure identifier RS and service data response values *** */
            PNIO_printf ( "PROFIenergy command QUERY_MODES received\n" );

            pRspQueryM->NumOfPeModeIDs	 = NUMOF_PE_MODES;   /* number of power save modes */
            pRspQueryM->PeMode[ 0 ]		 = PE_MODE_READY_TO_OPERATE;	/*mandatory*/
            pRspQueryM->PeMode[ 1 ]		 = PE_MODE_PAUSE1;
            pRspQueryM->PeMode[ 2 ]		 = PE_MODE_PAUSE2;
            /**/
			#if ((NUMOF_PE_MODES % 2) == 0)
            pRspQueryM->PeMode[ NUMOF_PE_MODES ]		 = 0x00;	/*alignment to 16bit - aligning bzte have to be 0x00*/
			#endif
            PE_RspLen[ArNum] = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_QUERY_MODES );
        }
        else
        {
            PE_RspLen[ArNum] = PE_SetErrRsp( pRspBuf, ErrCode );
        }
    /*     END    user code */
    /* *************************************************************** */

    return( PNIO_OK );	/* is ok, because its an appl. error, but not a record read error */
}


/**
 *  @brief Request handler for PE command "get mode"
 *
 *  @param[in]   	*pRqBuf 		request buffer pointer (from record write request)
 *  @param[in] 		*pRspBuf 		response buffer pointer (for later record read request)
 *  @return      					Returns	PNIO_OK (1) if OK
 *
 *  This function responds to CPU with information about device properties in current state
 *  Example functionality
 */
static PNIO_UINT32  PE_get_mode_RQ
    (
      PNIO_UINT8			*pRqBuf,		/* [in] request buffer pointer (from record write request) */
      PNIO_UINT8			*pRspBuf, 		/* [in] response buffer pointer (for later record read request) */
      PNIO_UINT16           ArNum           /* [in] number of AR initiating request*/
    )
{
    PE_SDR_REQ_GET_MODE* pRqGetM  = ( PE_SDR_REQ_GET_MODE* )( pRqBuf  + sizeof( PE_SUM_REQ_HDR ) );
    PE_SDR_RSP_GET_MODE* pRspGetM = ( PE_SDR_RSP_GET_MODE* )( pRspBuf + sizeof( PE_SUM_RSP_HDR ) );
    PNIO_UINT8 ErrCode;

    /**** plausibility check Modifier, DataStrucIdRQ, sets complete header (always) and complete response on error *** */
    ErrCode = CheckRqHdr_SrvHdr  (pRqBuf,	/* [in]  RQ:  saved PE-RQ from record write */
                                  0x02,		/* [in]  RQ:  expected modifier value */
                                  0x01,		/* [in]  RQ:  expected DataStrucIdRQ value */
                                  pRspBuf,	/* [out] RSP: response header */
								  	  	  	/* [in]  RSP: length value in blockheader */
                                  sizeof(PE_SUM_RSP_HDR) + sizeof(PE_SDR_RSP_GET_MODE) - 4,
                                  0x02);	/* [out] RSP: DataStrucIdRs, if no error */

    /* *************************************************************** */
    /*     BEGIN  user code */


        switch (pRqGetM->PeModeId)
        {
            case PE_MODE_POWER_OFF:
            case PE_MODE_PAUSE1:
            case PE_MODE_PAUSE2:
            case PE_MODE_READY_TO_OPERATE:
                break;
            default: 
                ErrCode = PE_RET_IMPERMISSIBLE_PE_MODE_ID;
                break;
        }

        if( ErrCode == PE_RET_OK )
        {
            PNIO_UINT32 Val32;
            PNIO_FLOAT  f1;

            /* if ok, set data structure identifier RS and service data response values *** */
            PNIO_printf( "PROFIenergy command GET_MODE received\n" );

            pRspGetM->PeModeId                 = pRqGetM->PeModeId;		/* return requested energy saving mode */
            pRspGetM->PeModeAttr               = 0;                   	/* only static values available */

            Val32 = 1500;
            SetVal32_Htonl( &( pRspGetM->TimeMinPause ), &Val32 );   	/* example time value:  10 minutes */

            if( CurrentPE_Mode == PE_MODE_READY_TO_OPERATE )  			/* current state is OPERATE */
            {
                Val32 = 600;										/* example time value: not specified */
                SetVal32_Htonl( &( pRspGetM->TimeToPause ), &Val32 );
                Val32 = 600;												/* 0 sec, is already operating */
                SetVal32_Htonl( &( pRspGetM->RegularTimeToOperate ), &Val32 );
                Val32 = 200;											/* example time value: 10 minutes */
                SetVal32_Htonl( &( pRspGetM->TimeMinLenOfStay ), &Val32 );
                Val32 = 0xffffffff;										/* example time value: not specified */
                SetVal32_Htonl( &( pRspGetM->TimeMaxLenOfStay ), &Val32 );

                f1 = 30.0f;												/* example value:  30 kW */
                SetVal32_Htonl( &( pRspGetM->ModePowerConsumption ), ( PNIO_VOID* )&f1 );
                f1 = 0.0f;												/* example value:  not specified */
                SetVal32_Htonl( &( pRspGetM->EnergyConsumpToPause ), ( PNIO_VOID* )&f1 );
                f1 = 0.0f;												/* 0 kW, is already operate */
                SetVal32_Htonl( &( pRspGetM->EnergyConsumpToOperate ), ( PNIO_VOID* )&f1 );
            }
            else 														/* current state is PAUSE */
            {
                Val32 = 0;												/* 0 second, is already in pause state */
                SetVal32_Htonl( &( pRspGetM->TimeToPause ), &Val32 );
                Val32 = 1800;											/* example time value: 30min = 1800sec */
                SetVal32_Htonl( &( pRspGetM->RegularTimeToOperate ), &Val32 );
                Val32 = 600;											/* example time value: 10 minutes */
                SetVal32_Htonl( &( pRspGetM->TimeMinLenOfStay ), &Val32 );
                Val32 = 0xffffffff;										/* example time value: not specified */
                SetVal32_Htonl( &( pRspGetM->TimeMaxLenOfStay ), &Val32 );

                f1 = 10.0f;												/* example value:  10 kW */
                SetVal32_Htonl( &( pRspGetM->ModePowerConsumption ), ( PNIO_VOID* )&f1 );
                f1 = 0.0f;												/* example value:  not specified */
                SetVal32_Htonl( &( pRspGetM->EnergyConsumpToPause ), ( PNIO_VOID* )&f1 );
                f1 = 5.0f;												/* example value:  5 kWh */
                SetVal32_Htonl( &( pRspGetM->EnergyConsumpToOperate ), ( PNIO_VOID* )&f1 );
            }

            PE_RspLen[ArNum] = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_GET_MODE );
        }
        else
        {
            PE_RspLen[ArNum] = PE_SetErrRsp( pRspBuf, ErrCode );
        }
    /*     END    user code */
    /* *************************************************************** */

    return( PNIO_OK );
}


/**
 *  @brief Request handler for PE command "pem_status"
 *
 *  @param[in]   	*pRqBuf 		request buffer pointer (from record write request)
 *  @param[in] 		*pRspBuf 		response buffer pointer (for later record read request)
 *  @return      					Returns	PNIO_OK (1) if OK
 *
 *  This function responds to CPU with information about state of device
 *  Example functionality
 */
static PNIO_UINT32  PE_pem_status_RQ
    (
      PNIO_UINT8			*pRqBuf,		/* [in] request buffer pointer (from record write request) */
      PNIO_UINT8			*pRspBuf, 		/* [in] response buffer pointer (for later record read request) */
      PNIO_UINT16           ArNum           /* [in] number of AR initiating request*/
    )
{
    PE_SDR_RSP_PEM_STATUS* pRspPemStat = ( PE_SDR_RSP_PEM_STATUS* )( pRspBuf + sizeof( PE_SUM_RSP_HDR ) );
    PNIO_UINT8  ErrCode;
    PNIO_FLOAT  f1;
    PNIO_UINT32 Val32;

    /* *** plausibility check Modifier, DataStrucIdRQ, sets complete header (always) and complete response on error *** */
    ErrCode = CheckRqHdr_SrvHdr( pRqBuf,	/* [in]  RQ:  saved PE-RQ from record write */
                                  0x00,		/* [in]  RQ:  expected modifier value */
                                  0x00,		/* [in]  RQ:  expected DataStrucIdRQ value */
                                  pRspBuf,	/* [out] RSP: response header */
								  	  	    /* [in]  RSP: length value in blockheader */
                                  sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_PEM_STATUS ) - 4,
                                  0x01 );	/* [out] RSP: DataStrucIdRs, if no error */

    /* *************************************************************** */
    /*     BEGIN  user code */

        if( ErrCode == PE_RET_OK )
        {
            /* if ok, set data structure identifier RS and service data response values *** */
            PNIO_printf ( "PROFIenergy command PEM STATUS received\n" );

            if( CurrentPE_Mode == PE_MODE_READY_TO_OPERATE )
            {
                pRspPemStat->PeModeIdSrc   = PE_MODE_READY_TO_OPERATE;      /* PE mode ID source */
                pRspPemStat->PeModeIdDest  = PE_MODE_READY_TO_OPERATE;      /* PE mode ID destination */
                Val32 = 0000;												/* example value for time to operate */
                SetVal32_Htonl( &( pRspPemStat->TimeToOperate ), &Val32 );
                Val32 = 0xffffffff;											/* example time value: not specified */
                SetVal32_Htonl( &( pRspPemStat->RemainingTimeToDest ), &Val32 );

                f1 = 30.0f;													/* example value 30 kW */
                SetVal32_Htonl( &( pRspPemStat->ModePowerConsumption ), &f1 );
                f1 = 0.0f;													/* example value: not specified */
                SetVal32_Htonl( &( pRspPemStat->EnergyConsumpToDest ), &f1 );
                f1 = 0.0f;													/* example value: 0 kW, is already in operate state */
                SetVal32_Htonl( &( pRspPemStat->EnergyConsumpToOperate ), &f1 );
                PE_RspLen[ArNum] = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_PEM_STATUS );
            }
            else
            {
                pRspPemStat->PeModeIdSrc   = PE_MODE_PAUSE1;                /* PE mode ID source */
                pRspPemStat->PeModeIdDest  = PE_MODE_PAUSE1;                /* PE mode ID destination */
                Val32 = 1800;												/* example time value: 30min = 1800sec */
                SetVal32_Htonl( &( pRspPemStat->TimeToOperate ), &Val32 );
                Val32 = 0xffffffff;											/* example time value: not specified */
                SetVal32_Htonl( &( pRspPemStat->RemainingTimeToDest ), &Val32 );

                f1 = 10.0f;													/* example value 10 kW */
                SetVal32_Htonl( &( pRspPemStat->ModePowerConsumption ), &f1 );
                f1 = 0.0f;													/* example value: not specified */
                SetVal32_Htonl( &( pRspPemStat->EnergyConsumpToDest ), &f1 );
                f1 = 5.0f;													/* example value: 5 kWh */
                SetVal32_Htonl( &( pRspPemStat->EnergyConsumpToOperate ), &f1 );
                PE_RspLen[ArNum] = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_PEM_STATUS );
            }
        }
        else
        {
            PE_RspLen[ArNum] = PE_SetErrRsp( pRspBuf, ErrCode );
        }
    /*     END    user code */
    /* *************************************************************** */

    return( PNIO_OK );
}


/**
 *  @brief Request handler for PE command "identify"
 *
 *  @param[in]   	*pRqBuf 		request buffer pointer (from record write request)
 *  @param[in] 		*pRspBuf 		response buffer pointer (for later record read request)
 *  @return      					Returns	PNIO_OK (1) if OK
 *
 *  This function responds to CPU with information about supported requests
 *  Example functionality
 */
static PNIO_UINT32  PE_identify_RQ
    (
      PNIO_UINT8			*pRqBuf,		// [in] request buffer pointer (from record write request)
      PNIO_UINT8			*pRspBuf, 		// [in] response buffer pointer (for later record read request)
      PNIO_UINT16           ArNum           /* [in] number of AR initiating request*/
    )
{
    PE_SDR_RSP_IDENTIFY* pRspIdent = ( PE_SDR_RSP_IDENTIFY* )( pRspBuf + sizeof( PE_SUM_RSP_HDR ) );
    PNIO_UINT8 ErrCode;

    /* *** plausibility check Modifier, DataStrucIdRQ, sets complete header (always) and complete response on error *** */
    ErrCode = CheckRqHdr_SrvHdr( pRqBuf,	/* [in]  RQ:  saved PE-RQ from record write */
                                  0x00,		/* [in]  RQ:  expected modifier value */
                                  0x00,		/* [in]  RQ:  expected DataStrucIdRQ value */
                                  pRspBuf,	/* [out] RSP: response header */
								  	  	    /* [in]  RSP: length value in blockheader */
                                  sizeof(PE_SUM_RSP_HDR) + sizeof(PE_SDR_RSP_IDENTIFY) - 4,
                                  0x01);	/* [out] RSP: DataStrucIdRs, if no error */

    /* *************************************************************** */
    /*     BEGIN  user code */

        if ( ErrCode == PE_RET_OK )
        {
            /* if ok, set data structure identifier RS and service data response values *** */
            PNIO_printf ( "PROFIenergy command IDENTIFY received\n" );

            pRspIdent->NumOfCmd = NUMOF_PE_SERVICES;
            pRspIdent->Cmd[ 0 ]   = PE_SRV_START_PAUSE;
            pRspIdent->Cmd[ 1 ]   = PE_SRV_END_PAUSE;
            pRspIdent->Cmd[ 2 ]   = PE_SRV_QUERY_MODES;
            pRspIdent->Cmd[ 3 ]   = PE_SRV_PEM_STATUS;
            pRspIdent->Cmd[ 4 ]   = PE_SRV_IDENTIFY;
            pRspIdent->Cmd[ 5 ]   = PE_QUERY_VERSION;
            pRspIdent->Cmd[ 6 ]   = PE_GET_MEASUREMENT_LIST;
            pRspIdent->Cmd[ 7 ]   = PE_QUERY_ATTRIBUTES;
            pRspIdent->Cmd[ 8 ]   = 0; 

            PE_RspLen[ArNum] = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_IDENTIFY );
        }
        else
        {
            PE_RspLen[ArNum] = PE_SetErrRsp( pRspBuf, ErrCode );
        }

    /*     END    user code */
    /* *************************************************************** */

    return( PNIO_OK );	/* is ok, because its an appl. error, but not a record read error */
}


/**
 *  @brief Request handler for PE command "PE_Get_Measurement_Values"
 *
 *  @param[in]      *pRqBuf         request buffer pointer (from record write request)
 *  @param[in]      *pRspBuf        response buffer pointer (for later record read request)
 *  @return                         Returns PNIO_OK (1) if OK
 *
 *  This function responds to CPU with measured data
 *  Information which of measurable data should be sent is part of the request
 *  Example functionality
 */
static PNIO_UINT32  PE_Get_Measurement_Values_No_Obj_Num
    (
      PNIO_UINT8            *pRqBuf,        /* [in] request buffer pointer (from record write request) */
      PNIO_UINT8            *pRspBuf,        /* [in] response buffer pointer (for later record read request) */
      PNIO_UINT16           ArNum           /* [in] number of AR initiating request*/
    )
{
    PE_GET_MEAS_VALUES* pRspMV = ( PE_GET_MEAS_VALUES* )( pRspBuf + sizeof( PE_SUM_RSP_HDR ) );
    PNIO_UINT8 ErrCode;
    int i;
    PNIO_FLOAT  f1;
    PNIO_UINT16 Val16;

    // **** plausibility check Modifier, DataStrucIdRQ, sets complete header (always) and complete response on error ***
    ErrCode = CheckRqHdr_SrvHdr( pRqBuf,    /* [in]  RQ:  saved PE-RQ from record write */
                                  0x02,     /* [in]  RQ:  expected modifier value */
                                  0x01,     /* [in]  RQ:  expected DataStrucIdRQ value */
                                  pRspBuf,  /* [out] RSP: response header */
                                            /* [in]  RSP: length value in blockheader */
                                  sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_GET_MEAS_VALUES ) - 4,
                                  0x01 );   /* [out] RSP: DataStrucIdRs, if no error */

    /* *************************************************************** */
    /*     BEGIN  user code */

        if( ErrCode == PE_RET_OK )
        {
            // * if ok, set data structure identifier RS and service data response values ***
            PNIO_printf ( "PROFIenergy Get Measurement values received\n" );

            pRspMV->Count = PE_NO_MEASURED_ITEMS;
            pRspMV->reserved = 0x00;

            i = 0;
            //lint -e{866} Unusual use of '+' in argument to sizeof
            Val16 = sizeof( pRspMV->item[ i ] );
            SetVal16_Htons ( &( pRspMV->item[i].Length_of_Structure ), &Val16 );
            pRspMV->item[i].Measurement_Data_Structure_ID = 0x01;   /* according to specification, always 0x01 */
            pRspMV->item[i].Status_of_Measurement_Value = 1;        /* 1 = valid,  2 = not available, 3 = temporary not available */
            Val16 = 13;                                         /* Specifies type of measurement; 13 = Active power in Watts [W], phase a */
            SetVal16_Htons ( &( pRspMV->item[i].Measurement_ID ), &Val16 );
            f1 = 120.0f;                                            /* example value */
            SetVal32_Htonl ( &( pRspMV->item[i].Measurement_Value ), &f1 );

            i++;
            //lint -e{866} Unusual use of '+' in argument to sizeof
            Val16 = sizeof( pRspMV->item[ i ] );
            SetVal16_Htons ( &( pRspMV->item[i].Length_of_Structure ), &Val16 );
            pRspMV->item[i].Measurement_Data_Structure_ID = 0x01;   /* according to specification, always 0x01 */
            pRspMV->item[i].Status_of_Measurement_Value = 1;        /* 1 = valid,  2 = not available, 3 = temporary not available */
            Val16 = 14;                                         /* Specifies type of measurement; 14 = Active power in Watts [W], phase b */
            SetVal16_Htons ( &( pRspMV->item[i].Measurement_ID ), &Val16 );
            f1 = 161.0f;                                            /* example value */
            SetVal32_Htonl ( &( pRspMV->item[i].Measurement_Value ), &f1 );

            i++;
			//lint -e{866} Unusual use of '+' in argument to sizeof
            Val16 = sizeof( pRspMV->item[ i ] );
            SetVal16_Htons ( &( pRspMV->item[i].Length_of_Structure ), &Val16 );
            pRspMV->item[i].Measurement_Data_Structure_ID = 0x01;   /* according to specification, always 0x01 */
            pRspMV->item[i].Status_of_Measurement_Value = 1;        /* 1 = valid,  2 = not available, 3 = temporary not available */
            Val16 = 15;                                         /* Specifies type of measurement; 15 = Active power in Watts [W], phase c */
            SetVal16_Htons ( &( pRspMV->item[i].Measurement_ID ), &Val16 );
            f1 = 55.0f;                                         /* example value */
            SetVal32_Htonl ( &( pRspMV->item[i].Measurement_Value ), &f1 );

            PE_RspLen[ArNum] = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_GET_MEAS_VALUES );
        }
        else
        {
            PE_RspLen[ArNum] = PE_SetErrRsp( pRspBuf, ErrCode );
        }

    /*     END    user code */
    /* *************************************************************** */

    return ( PNIO_OK ); /* is ok, because its an appl. error, but not a record read error */
}


/**
 *  @brief Request handler for PE command "Get_Measurement_List"
 *
 *  @param[in]      *pRqBuf         request buffer pointer (from record write request)
 *  @param[in]      *pRspBuf        response buffer pointer (for later record read request)
 *  @return                         Returns PNIO_OK (1) if OK
 *
 *  This function responds to CPU with data about measurable parameters
 *  Example functionality
 */
static PNIO_UINT32  PE_Get_Measurement_List_No_Obj_Num
    (
      PNIO_UINT8            *pRqBuf,        /* [in] request buffer pointer (from record write request) */
      PNIO_UINT8            *pRspBuf,        /* [in] response buffer pointer (for later record read request) */
      PNIO_UINT16           ArNum           /* [in] number of AR initiating request*/
    )
{
    PE_GET_MEAS_LIST* pRspML = (PE_GET_MEAS_LIST*)(pRspBuf + sizeof(PE_SUM_RSP_HDR));
    PNIO_UINT8 ErrCode;
    int i;
    PNIO_FLOAT  f1;
    PNIO_UINT16 Val16 = 0;

    /**** plausibility check Modifier, DataStrucIdRQ, sets complete header (always) and complete response on error *** */
    ErrCode = CheckRqHdr_SrvHdr( pRqBuf,    /* [in]  RQ:  saved PE-RQ from record write */
                                  0x01,     /* [in]  RQ:  expected modifier value */
                                  0x00,     /* [in]  RQ:  expected DataStrucIdRQ value */
                                  pRspBuf,  /* [out] RSP: response header */
                                            /* [in]  RSP: length value in blockheader */
                                  sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_GET_MEAS_LIST ) - 4,
                                  0x02 );   /* [out] RSP: DataStrucIdRs, if no error */


    /* *************************************************************** */
    /*     BEGIN  user code */

        if( ErrCode == PE_RET_OK )
        {
            /* if ok, set data structure identifier RS and service data response values *** */
            PNIO_printf ( "PROFIenergy Get Measurement list received\n" );

            pRspML->Count = PE_NO_MEASURED_ITEMS;
            pRspML->reserved = 0x00;

            i = 0;
            pRspML->item[ i ].Accuracy_class = 3;   /* 0.05%, for detail to values coding, see documentation */
            pRspML->item[ i ].Accuracy_domain = 1;  /* accuracy as percent of full scale reading */
            Val16 = 13;                             /* Specifies type of measurement; 13 = Active power in Watts [W], phase a */
            SetVal16_Htons ( &( pRspML->item[ i ].Measurement_ID ), &Val16 );
            f1 = 300.0f;                            /* valid only for Accuracy_domain = 1 */
                                                    /* Specifies full scale reading of measured value with unit as defined by Measurement_ID */
            SetVal32_Htonl ( &( pRspML->item[ i ].Range ), &f1 );

            i++;
            pRspML->item[ i ].Accuracy_class = 3;   /* 0.05%, for detail to values coding, see documentation */
            pRspML->item[ i ].Accuracy_domain = 1;  /* accuracy as percent of full scale reading */
            Val16 = 14;                             /* Specifies type of measurement; 14 = Active power in Watts [W], phase b */
            SetVal16_Htons ( &( pRspML->item[ i ].Measurement_ID ), &Val16 );
            f1 = 300.0f;                            /* valid only for Accuracy_domain = 1 */
                                                    /* Specifies full scale reading of measured value with unit as defined by Measurement_ID */
            SetVal32_Htonl ( &( pRspML->item[ i ].Range ), &f1 );

            i++;
            pRspML->item[ i ].Accuracy_class = 3;   /* 0.05%, for detail to values coding, see documentation */
            pRspML->item[ i ].Accuracy_domain = 1;  /* accuracy as percent of full scale reading */
            Val16 = 15;                             /* Specifies type of measurement; 15 = Active power in Watts [W], phase c */
            SetVal16_Htons ( &( pRspML->item[ i ].Measurement_ID ), &Val16 );
            f1 = 300.0f;                            /* valid only for Accuracy_domain = 1 */
                                                    /* Specifies full scale reading of measured value with unit as defined by Measurement_ID */
            SetVal32_Htonl ( &( pRspML->item[ i ].Range ), &f1 );

            PE_RspLen[ArNum] = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_GET_MEAS_LIST);
        }
        else
        {
            PE_RspLen[ArNum] = PE_SetErrRsp( pRspBuf, ErrCode );
        }

    /*     END    user code */
    /* *************************************************************** */

    return ( PNIO_OK );                    /* is ok, because its an appl. error, but not a record read error */
}



/**
 *  @brief Request handler for PE command "PE_Get_Measurement_Values_with_object_number"
 *
 *  @param[in]   	*pRqBuf 		request buffer pointer (from record write request)
 *  @param[in] 		*pRspBuf 		response buffer pointer (for later record read request)
 *  @return      					Returns	PNIO_OK (1) if OK
 *
 *  This function responds to CPU with measured data
 *  Information which of measurable data should be sent is part of the request
 *  Example functionality
 */
static PNIO_UINT32  PE_Get_Measurement_Values
    (
      PNIO_UINT8			*pRqBuf,		/* [in] request buffer pointer (from record write request) */
      PNIO_UINT8			*pRspBuf, 		/* [in] response buffer pointer (for later record read request) */
      PNIO_UINT16           ArNum           /* [in] number of AR initiating request*/
    )
{
	PE_GET_MEAS_VALUES_OBJ_NUM* pRspMV = ( PE_GET_MEAS_VALUES_OBJ_NUM* )( pRspBuf + sizeof( PE_SUM_RSP_HDR ) );
	PE_GET_MEAS_VALUES_RQ* pRqMV = ( PE_GET_MEAS_VALUES_RQ* )( pRqBuf + sizeof( PE_SUM_REQ_HDR ) );
    PNIO_UINT8 ErrCode;
    PNIO_UINT16 i;
    PNIO_FLOAT  f1;
    PNIO_UINT16 Val16 = 0;

    // **** plausibility check Modifier, DataStrucIdRQ, sets complete header (always) and complete response on error ***
    ErrCode = CheckRqHdr_SrvHdr( pRqBuf,	/* [in]  RQ:  saved PE-RQ from record write */
                                  0x04,		/* [in]  RQ:  expected modifier value */
                                  0x00,		/* [in]  RQ:  expected DataStrucIdRQ value */
                                  pRspBuf,	/* [out] RSP: response header */
								  	  	    /* [in]  RSP: length value in blockheader */
                                  sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_GET_MEAS_VALUES_OBJ_NUM ) - 4,
                                  0x03 );	/* [out] RSP: DataStrucIdRs, if no error */

    /* *************************************************************** */
    /*     BEGIN  user code */

		if( PE_NO_MEASURED_ITEMS < pRqMV->Count )
		{
			ErrCode = PE_RET_INVALID_BLOCK_HEADER;
		}
		else
		{
			for( i = 0; i < pRqMV->Count; i++)
			{
				SetVal16_Htons (  &Val16, &( pRqMV->item[i].Object_number ) );
				if( PE_NO_MEASURED_ITEMS < Val16 )
				{
				  ErrCode = PE_RET_INVALID_BLOCK_HEADER;
				}
			}
		}

        if( ErrCode == PE_RET_OK )
        {
			// * if ok, set data structure identifier RS and service data response values ***
			PNIO_printf ( "PROFIenergy Get Measurement values with objectnumber received\n" );

			pRspMV->Count = PE_NO_MEASURED_ITEMS;
			pRspMV->reserved = 0x00;

			i = 0;
			//lint -e{866} Unusual use of '+' in argument to sizeof
			Val16 = sizeof( pRspMV->item[ i ] );
			SetVal16_Htons ( &( pRspMV->item[i].Length_of_Structure ), &Val16 );
			pRspMV->item[i].Measurement_Data_Structure_ID = 0x02;	/* according to specification, always 0x02 */
			pRspMV->item[i].Status_of_Measurement_Value = 1;		/* 1 = valid,  2 = not available, 3 = temporary not available */
			Val16  = i;											/* object number # */
			SetVal16_Htons ( &( pRspMV->item[i].Object_number ), &Val16 );
			Val16 = 13;											/* Specifies type of measurement; 13 = Active power in Watts [W], phase a */
			SetVal16_Htons ( &( pRspMV->item[i].Measurement_ID ), &Val16 );
			f1 = 120.0f;											/* example value */
			SetVal32_Htonl ( &( pRspMV->item[i].Measurement_Value ), &f1 );

			i++;
			//lint -e{866} Unusual use of '+' in argument to sizeof
			Val16 = sizeof( pRspMV->item[ i ] );
			SetVal16_Htons ( &( pRspMV->item[i].Length_of_Structure ), &Val16 );
			pRspMV->item[i].Measurement_Data_Structure_ID = 0x02;	/* according to specification, always 0x02 */
			pRspMV->item[i].Status_of_Measurement_Value = 1;		/* 1 = valid,  2 = not available, 3 = temporary not available */
			Val16  = i;											/* object number # */
			SetVal16_Htons ( &( pRspMV->item[i].Object_number ), &Val16 );
			Val16 = 14;											/* Specifies type of measurement; 14 = Active power in Watts [W], phase b */
			SetVal16_Htons ( &( pRspMV->item[i].Measurement_ID ), &Val16 );
			f1 = 161.0f;											/* example value */
			SetVal32_Htonl ( &( pRspMV->item[i].Measurement_Value ), &f1 );

			i++;
			//lint -e{866} Unusual use of '+' in argument to sizeof
			Val16 = sizeof( pRspMV->item[ i ] );
			SetVal16_Htons ( &( pRspMV->item[i].Length_of_Structure ), &Val16 );
			pRspMV->item[i].Measurement_Data_Structure_ID = 0x02;	/* according to specification, always 0x02 */
			pRspMV->item[i].Status_of_Measurement_Value = 1;		/* 1 = valid,  2 = not available, 3 = temporary not available */
			Val16  = i;											/* object number # */
			SetVal16_Htons ( &( pRspMV->item[i].Object_number ), &Val16 );
			Val16 = 15;											/* Specifies type of measurement; 15 = Active power in Watts [W], phase c */
			SetVal16_Htons ( &( pRspMV->item[i].Measurement_ID ), &Val16 );
			f1 = 55.0f;											/* example value */
			SetVal32_Htonl ( &( pRspMV->item[i].Measurement_Value ), &f1 );

            PE_RspLen[ArNum] = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_GET_MEAS_VALUES_OBJ_NUM );
        }
        else
        {
            PE_RspLen[ArNum] = PE_SetErrRsp( pRspBuf, ErrCode );
        }

    /*     END    user code */
    /* *************************************************************** */

    return ( PNIO_OK ); /* is ok, because its an appl. error, but not a record read error */
}


/**
 *  @brief Request handler for PE command "Get_Measurement_List_with_object_number"
 *
 *  @param[in]   	*pRqBuf 		request buffer pointer (from record write request)
 *  @param[in] 		*pRspBuf 		response buffer pointer (for later record read request)
 *  @return      					Returns	PNIO_OK (1) if OK
 *
 *  This function responds to CPU with data about measurable parameters
 *  Example functionality
 */
static PNIO_UINT32  PE_Get_Measurement_List
    (
      PNIO_UINT8			*pRqBuf,		/* [in] request buffer pointer (from record write request) */
      PNIO_UINT8			*pRspBuf, 		/* [in] response buffer pointer (for later record read request) */
      PNIO_UINT16           ArNum           /* [in] number of AR initiating request*/
    )
{
	PE_GET_MEAS_LIST_OBJ_NUM* pRspML = (PE_GET_MEAS_LIST_OBJ_NUM*)(pRspBuf + sizeof(PE_SUM_RSP_HDR));
    PNIO_UINT8 ErrCode;
    PNIO_UINT16 i;
    PNIO_FLOAT  f1;
    PNIO_UINT16 Val16;

    /**** plausibility check Modifier, DataStrucIdRQ, sets complete header (always) and complete response on error *** */
    ErrCode = CheckRqHdr_SrvHdr( pRqBuf,	/* [in]  RQ:  saved PE-RQ from record write */
                                  0x03,		/* [in]  RQ:  expected modifier value */
                                  0x00,		/* [in]  RQ:  expected DataStrucIdRQ value */
                                  pRspBuf,	/* [out] RSP: response header */
								  	  	    /* [in]  RSP: length value in blockheader */
                                  sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_GET_MEAS_LIST_OBJ_NUM ) - 4,
                                  0x04 );	/* [out] RSP: DataStrucIdRs, if no error */

    /* *************************************************************** */
    /*     BEGIN  user code */

        if( ErrCode == PE_RET_OK )
        {
			/* if ok, set data structure identifier RS and service data response values *** */
			PNIO_printf ( "PROFIenergy Get Measurement list with object number received\n" );
			
			pRspML->Count = PE_NO_MEASURED_ITEMS;
			pRspML->reserved = 0x00;

			i = 0;
			pRspML->item[ i ].Accuracy_class = 3;	/* 0.05%, for detail to values coding, see documentation */
			pRspML->item[ i ].Accuracy_domain = 1;	/* accuracy as percent of full scale reading */
			Val16 = 13;								/* Specifies type of measurement; 13 = Active power in Watts [W], phase a */
			SetVal16_Htons ( &( pRspML->item[ i ].Measurement_ID ), &Val16 );
			Val16 = i;								/* object number # */
			SetVal16_Htons ( &( pRspML->item[ i ].Object_number ), &Val16 );
			f1 = 300.0f;							/* valid only for Accuracy_domain = 1 */
													/* Specifies full scale reading of measured value with unit as defined by Measurement_ID */
			SetVal32_Htonl ( &( pRspML->item[ i ].Range ), &f1 );

			i++;
			pRspML->item[ i ].Accuracy_class = 3;	/* 0.05%, for detail to values coding, see documentation */
			pRspML->item[ i ].Accuracy_domain = 1;	/* accuracy as percent of full scale reading */
			Val16 = 14;								/* Specifies type of measurement; 14 = Active power in Watts [W], phase b */
			SetVal16_Htons ( &( pRspML->item[ i ].Measurement_ID ), &Val16 );
			Val16 = i;								/* object number # */
			SetVal16_Htons ( &( pRspML->item[ i ].Object_number ), &Val16 );
			f1 = 300.0f;							/* valid only for Accuracy_domain = 1 */
													/* Specifies full scale reading of measured value with unit as defined by Measurement_ID */
			SetVal32_Htonl ( &( pRspML->item[ i ].Range ), &f1 );

			i++;
			pRspML->item[ i ].Accuracy_class = 3;	/* 0.05%, for detail to values coding, see documentation */
			pRspML->item[ i ].Accuracy_domain = 1;	/* accuracy as percent of full scale reading */
			Val16 = 15;								/* Specifies type of measurement; 15 = Active power in Watts [W], phase c */
			SetVal16_Htons ( &( pRspML->item[ i ].Measurement_ID ), &Val16 );
			Val16 = i;								/* object number # */
			SetVal16_Htons ( &( pRspML->item[ i ].Object_number ), &Val16 );
			f1 = 300.0f;							/* valid only for Accuracy_domain = 1 */
													/* Specifies full scale reading of measured value with unit as defined by Measurement_ID */
			SetVal32_Htonl ( &( pRspML->item[ i ].Range ), &f1 );

			PE_RspLen[ArNum] = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_GET_MEAS_LIST_OBJ_NUM );
        }
        else
        {
        	PE_RspLen[ArNum] = PE_SetErrRsp( pRspBuf, ErrCode );
        }

    /*     END    user code */
    /* *************************************************************** */

    return ( PNIO_OK );                    /* is ok, because its an appl. error, but not a record read error */
}


/**
 *  @brief Request handler for PE command "PEM_Status_with_CTTO"
 *
 *  @param[in]   	*pRqBuf 		request buffer pointer (from record write request)
 *  @param[in] 		*pRspBuf 		response buffer pointer (for later record read request)
 *  @return      					Returns	PNIO_OK (1) if OK
 *
 *  This function responds to CPU with data about device status
 *  Returned values are ongoing - dynamic - Current Time To Operate
 *  Example functionality
 */
static PNIO_UINT32  PE_PEM_Status_with_CTTO
    (
      PNIO_UINT8			*pRqBuf,		/* [in] request buffer pointer (from record write request) */
      PNIO_UINT8			*pRspBuf, 		/* [in] response buffer pointer (for later record read request) */
      PNIO_UINT16           ArNum           /* [in] number of AR initiating request*/
    )
{
	PE_SDR_RSP_PEM_CTTO_STATUS* pRspPemStat = ( PE_SDR_RSP_PEM_CTTO_STATUS* )( pRspBuf + sizeof( PE_SUM_RSP_HDR ) );
    PNIO_UINT8 ErrCode;
    PNIO_FLOAT  f1;
    PNIO_UINT32 Val32;

    /* **** plausibility check Modifier, DataStrucIdRQ, sets complete header (always) and complete response on error *** */
    ErrCode = CheckRqHdr_SrvHdr( pRqBuf,	/* [in]  RQ:  saved PE-RQ from record write */
                                  0x01,		/* [in]  RQ:  expected modifier value */
                                  0x00,		/* [in]  RQ:  expected DataStrucIdRQ value */
                                  pRspBuf,	/* [out] RSP: response header */
								  	  	    /* [in]  RSP: length value in blockheader */
                                  sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_PEM_CTTO_STATUS ) - 4,
                                  0x02 );	/* [out] RSP: DataStrucIdRs, if no error */

    /* *************************************************************** */
    /*     BEGIN  user code */

        if ( ErrCode == PE_RET_OK )
        {
            /* if ok, set data structure identifier RS and service data response values *** */
            PNIO_printf ( "PROFIenergy PEM Status with CTTO received\n" );

            if ( CurrentPE_Mode == PE_MODE_READY_TO_OPERATE )
            {
                pRspPemStat->PeModeIdSrc   = PE_MODE_READY_TO_OPERATE;        		/* PE mode ID source */
                pRspPemStat->PeModeIdDest  = PE_MODE_READY_TO_OPERATE;        		/* PE mode ID destination */
                Val32 = 0000;
                SetVal32_Htonl ( &( pRspPemStat->RegularTimeToOperate ), &Val32 );  /* example value for time to operate */
                Val32 = 0000;
                SetVal32_Htonl ( &( pRspPemStat->CurrentTimeToOperate), &Val32 );   /* example value for time to operate */
                Val32 = 0xffffffff;
                SetVal32_Htonl( &( pRspPemStat->CurrentTimeToDest), &Val32 );   	/* example time value: not specified */

                f1 = 30.0f;
                SetVal32_Htonl ( &( pRspPemStat->ModePowerConsumption ), &f1 ); 	/* example value 30 kW */
                f1 = 0.0f;
                SetVal32_Htonl ( &( pRspPemStat->EnergyConsumpToDest ), &f1 ); 		/* example value: not specified */
                f1 = 0.0f;
                SetVal32_Htonl ( &( pRspPemStat->EnergyConsumpToOperate ), &f1 ); 	/* example value: 0 kW, is already in operate state */
                PE_RspLen[ArNum] = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_PEM_CTTO_STATUS );
            }
            else
            {
                pRspPemStat->PeModeIdSrc   = PE_MODE_PAUSE1;                  		/* PE mode ID source */
                pRspPemStat->PeModeIdDest  = PE_MODE_PAUSE1;                  		/* PE mode ID destination */
                Val32 = 1800;
                SetVal32_Htonl( &( pRspPemStat->RegularTimeToOperate ), &Val32 );   /* example time value: 30min = 1800sec */
                Val32 = 0600;
                SetVal32_Htonl( &( pRspPemStat->CurrentTimeToOperate ), &Val32 );   /* example value for time to operate */
                Val32 = 0xffffffff;
                SetVal32_Htonl( &( pRspPemStat->CurrentTimeToDest ), &Val32 );   	/* example time value: not specified */

                f1 = 10.0f;
                SetVal32_Htonl( &( pRspPemStat->ModePowerConsumption ), &f1 ); 		/* example value 10 kW */
                f1 = 0.0f;
                SetVal32_Htonl( &( pRspPemStat->EnergyConsumpToDest ), &f1 );		/*  example value: not specified */
                f1 = 5.0f;
                SetVal32_Htonl( &( pRspPemStat->EnergyConsumpToOperate ), &f1 ); 	/* example value: 5 kWh */
                PE_RspLen[ArNum] = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_PEM_CTTO_STATUS );
            }
        }
        else
        {
            PE_RspLen[ArNum] = PE_SetErrRsp ( pRspBuf, ErrCode );
        }

    /*     END    user code */
    /* *************************************************************** */

    return( PNIO_OK );                    /* is ok, because its an appl. error, but not a record read error */
}


/**
 *  @brief Request handler for PE command "Start_Pause_with_time_response"
 *
 *  @param[in]   	*pRqBuf 		request buffer pointer (from record write request)
 *  @param[in] 		*pRspBuf 		response buffer pointer (for later record read request)
 *  @return      					Returns	PNIO_OK (1) if OK
 *
 *  This function transfers the device to power save mode
 *  (only sets CurrentPE_mode value, as this is only example)
 *  And responds to CPU with timing data
 *  Example functionality
 */
static PNIO_UINT32  PE_Start_Pause_with_time_response
    (
      PNIO_UINT8			*pRqBuf,		/* [in] request buffer pointer (from record write request) */
      PNIO_UINT8			*pRspBuf, 		/* [in] response buffer pointer (for later record read request) */
      PNIO_UINT16           ArNum           /* [in] number of AR initiating request*/
    )
{
	PE_SDR_RSP_START_PAUSE_WITH_TIME_RESP* PE_pRsp = ( PE_SDR_RSP_START_PAUSE_WITH_TIME_RESP* )( pRspBuf + sizeof( PE_SUM_RSP_HDR ) );
    PNIO_UINT8 ErrCode;
    PNIO_UINT32 Val32;

    // **** plausibility check Modifier, DataStrucIdRQ, sets complete header (always) and complete response on error *** */
    ErrCode = CheckRqHdr_SrvHdr( pRqBuf,	/* [in]  RQ:  saved PE-RQ from record write */
                                  0x01,		/* [in]  RQ:  expected modifier value */
                                  0x01,		/* [in]  RQ:  expected DataStrucIdRQ value */
                                  pRspBuf,	/* [out] RSP: response header */
								  	  	    /* [in]  RSP: length value in blockheader */
                                  sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_START_PAUSE_WITH_TIME_RESP ) - 4,
                                  0x02 );	/* [out] RSP: DataStrucIdRs, if no error */

    /* *************************************************************** */
    /*     BEGIN  user code */

        if ( ErrCode == PE_RET_OK )
        {
            /* if ok, set data structure identifier RS and service data response values *** */
            PNIO_printf( "PROFIenergy Start Pause with time response\n" );

            CurrentPE_Mode          = PE_MODE_PAUSE1; /* we assume in this example , expected power save mode has already been achieved... */
            PE_pRsp->PE_ModeId   = CurrentPE_Mode;
            PE_pRsp->reserved    = 0x00;
            Val32 = 2000;						/* time to reach requested energy saving mode mode from ready_to_operate */
            SetVal32_Htonl( &( PE_pRsp->time_to_dest ), &Val32 );
            Val32 = 3000;						/* time to reach ready_to_operate mode from requested energy saving mode */
            SetVal32_Htonl( &( PE_pRsp->time_to_operate ), &Val32 );
            Val32 = 1000;						/* some devices have to be down for some time before activated again */
            SetVal32_Htonl( &( PE_pRsp->min_len_to_stay ), &Val32 );
            PE_RspLen[ArNum] = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_START_PAUSE_WITH_TIME_RESP );
        }
        else
        {
            PE_RspLen[ArNum] = PE_SetErrRsp ( pRspBuf, ErrCode );
        }

    /*     END    user code */
    /* *************************************************************** */

    return ( PNIO_OK );                    /* is ok, because its an appl. error, but not a record read error */
}


/**
 *  @brief Request handler for PE command "Query_Version"
 *
 *  @param[in]   	*pRqBuf 		request buffer pointer (from record write request)
 *  @param[in] 		*pRspBuf 		response buffer pointer (for later record read request)
 *  @return      					Returns	PNIO_OK (1) if OK
 *
 *  This function sends version of used PE profile to requesting CPU
 *  Example functionality
 */
static PNIO_UINT32  PE_Query_Version
    (
      PNIO_UINT8			*pRqBuf,		/* [in] request buffer pointer (from record write request) */
      PNIO_UINT8			*pRspBuf,		/* [in] response buffer pointer (for later record read request) */
      PNIO_UINT16           ArNum           /* [in] number of AR initiating request*/
    )
{
	PE_VERSION* pRspIdent = ( PE_VERSION* )( pRspBuf + sizeof( PE_SUM_RSP_HDR ) );
    PNIO_UINT8 ErrCode;

    /* *** plausibility check Modifier, DataStrucIdRQ, sets complete header (always) and complete response on error *** */
    ErrCode = CheckRqHdr_SrvHdr( pRqBuf,	/* [in]  RQ:  saved PE-RQ from record write */
                                  0x00,		/* [in]  RQ:  expected modifier value */
                                  0x00,		/* [in]  RQ:  expected DataStrucIdRQ value */
                                  pRspBuf,	/* [out] RSP: response header */
								  	  	  	/* [in]  RSP: length value in blockheader */
                                  sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_VERSION ) - 4,
                                  0x01 );	/* [out] RSP: DataStrucIdRs, if no error */

    /**************************************************************** */
    /*     BEGIN  user code */

        if (ErrCode == PE_RET_OK)
        {
        	/* if ok, set data structure identifier RS and service data response values *** */
        	PNIO_printf( "PROFIenergy Query Version response\n" );

            /* used version V1.1 */
            pRspIdent->major = 1;   /* major version number of PE profile */
            pRspIdent->minor = 3;   /* minor version number of PE profile */
            PE_RspLen[ArNum] = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_VERSION );
        }
        else
        {
            PE_RspLen[ArNum] = PE_SetErrRsp ( pRspBuf, ErrCode );
        }

    /*     END    user code */
    /* ************************************************************** */

    return ( PNIO_OK );                    // is ok, because its an appl. error, but not a record read error
}

static PNIO_UINT32  PE_Query_Attributes
    (
      PNIO_UINT8    *pRqBuf,        /* [in] request buffer pointer (from record write request) */
      PNIO_UINT8    *pRspBuf,       /* [in] response buffer pointer (for later record read request) */
      PNIO_UINT16    ArNum          /* [in] number of AR initiating request*/
    )
{
    PE_SDR_RSP_QUERY_ATTRIBUTES* pRspQA = ( PE_SDR_RSP_QUERY_ATTRIBUTES* )( pRspBuf + sizeof( PE_SUM_RSP_HDR ) );
    PNIO_UINT8 ErrCode;
    PNIO_UINT32 Val32;

    /* *** plausibility check Modifier, DataStrucIdRQ, sets complete header (always) and complete response on error *** */
    ErrCode = CheckRqHdr_SrvHdr( pRqBuf,    /* [in]  RQ:  saved PE-RQ from record write */
                                 0x00,      /* [in]  RQ:  expected modifier value */
                                 0x00,      /* [in]  RQ:  expected DataStrucIdRQ value */
                                 pRspBuf,   /* [out] RSP: response header */
                                            /* [in]  RSP: length value in blockheader */
                                 sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_QUERY_ATTRIBUTES ) - 4,
                                 0x01 );    /* [out] RSP: DataStrucIdRs, if no error */

    /**************************************************************** */
    /*     BEGIN  user code */

        if (ErrCode == PE_RET_OK)
        {
            /* used version V1.3 */
            pRspQA->major_version_number = 1;    /* major version number of PE profile */
            pRspQA->minor_version_number = 3;    /* minor version number of PE profile */
            pRspQA->Entity_Class = 3;            /* this is for supporting the class3  */
            pRspQA->Entity_Sub_Class = 1;        /* As it is written in the spec. document that means we are coding for subclass1*/
            pRspQA->Dyn_T_and_E_values = 1;      //TODO: this value should be tested.
            pRspQA->Use_of_PE_ASE = 1;           //TODO: this value should be tested.

            Val32 = 30000;                       /* In version smaller than V1.3, a timeout of 30s may be used to abort the polling process.*/
            SetVal32_Htonl(&(pRspQA->Max_Command_Respond_Time), &(Val32));

            PE_RspLen[ArNum] = sizeof( PE_SUM_RSP_HDR ) + sizeof( PE_SDR_RSP_QUERY_ATTRIBUTES );
        }
        else
        {
            PE_RspLen[ArNum] = PE_SetErrRsp ( pRspBuf, ErrCode );
        }


    /*     END    user code */
    /* ************************************************************** */


    return ( PNIO_OK );
}

#endif

/**
 *  @brief Calls correct method for PE request or returns error for invalid request
 *
 *  @param[in]   	*pAddr 			location (slot, subslot)
 *  @param[in, out] *pBufLen 		in: length to read, out: length, read by user
 *  @param[in]   	*pBuffer 		buffer pointer
 *  @param[out]  	*pPnioState	4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
 *  @return      					Returns	PNIO_OK (1) if OK
 *
 *  this function handles a record response on RecordIndex 0x80a0,
 *  which is specified as a "PROFIenergy" request.
 *  If an error occures, the error structure PNIO_ERR_STAT is filled.
 *  See PNIO specification for more details, how to fill the error
 *  structure.
 */
PNIO_UINT32  PROFIenergy_RequestHandler
    (
	  PNIO_DEV_ADDR       	*pAddr,		    /* [in] location (slot, subslot) */
      PNIO_UINT32			*pBufLen,		/* [in, out] in: length to read, out: length, read by user */
      PNIO_UINT8			*pBuffer,		/* [in] buffer pointer */
      PNIO_ERR_STAT			*pPnioState,	/* [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6 */
      PNIO_UINT16           ArNum           /* [in] number of AR initiating request*/
    )
{
#if (1 != IOD_USED_WITH_XHIF_HOST)
    PE_SUM_REQ_HDR* pSumReqHdr = ( PE_SUM_REQ_HDR* ) pBuffer;
    PNIO_UINT32 Status = PNIO_OK;
    PE_PROCESS_RESP pe_process_resp;


    /* *** clear error structure *** */
    OsMemSet( pPnioState, 0, sizeof( PNIO_ERR_STAT ) );

    /* -------------------------------------------------------------------------- */
    /* check if selected submodule supports PROFIenergy */
    /* In the example GSD file only all the IO-submodules support PROFIenergy, */
    /* but not the DAP submodules. */
    /* If the requested submodule does not support PROFIenergy, invalid index */
    /* has to be returned. */
    /* -------------------------------------------------------------------------- */
    if( pAddr->Geo.Slot == 0 )  /* is DAP ? */
    {
        pPnioState->ErrCode   = 0xdf;  /* IODWriteRes with ErrorDecode = PNIORW */
        pPnioState->ErrDecode = 0x80;  /* PNIORW */
        pPnioState->ErrCode1  = 0xb0;  /* example: Error Class 11 = Access, ErrorNr 0 = "invalid index" */
        pPnioState->ErrCode2  = 0x00;
        return( PNIO_NOT_OK );
    }


    /* **** preset value: correct response data are available **** */
    ResponseDataAvailable[ArNum] = PNIO_TRUE;

   /* *********** check block header ********** */
    pe_process_resp.pe_response = PE_RET_INVALID_MODIFIER;	/* will be changed if valid combination of request id and modifier, */
                                							/* othervise return error */

    switch ( pSumReqHdr->ReqHdr.ServReqId )
    {

		case( PE_SRV_START_PAUSE ):
		{
		  pe_process_resp.exp_blocklen = 10;

		  if( PE_MOD_SRV_START_PAUSE == pSumReqHdr->ReqHdr.Modifier )
		  {
			pe_process_resp.pe_method_ptr = PE_start_pause_RQ;	/* prepare corresponding method*/
			pe_process_resp.pe_response = PE_RET_OK;			/* valid service and modifier*/
		  }
		  else if( PE_MOD_START_PAUSE_WITH_TIME_RESP == pSumReqHdr->ReqHdr.Modifier )
		  {
			pe_process_resp.pe_method_ptr = PE_Start_Pause_with_time_response;	/* prepare corresponding method*/
			pe_process_resp.pe_response = PE_RET_OK;			/* valid service and modifier*/
		  }
		  break;
		}

		case( PE_SRV_END_PAUSE ):
		{
		  pe_process_resp.exp_blocklen = 6;

		  if( PE_MOD_SRV_END_PAUSE == pSumReqHdr->ReqHdr.Modifier )
		  {
			pe_process_resp.pe_method_ptr = PE_end_pause_RQ;	/* prepare corresponding method*/
			pe_process_resp.pe_response = PE_RET_OK;			/* valid service and modifier*/
		  }
		  break;
		}

		case( PE_SRV_QUERY_MODES ):
		{
		  if( PE_MOD_SRV_QUERY_MODES == pSumReqHdr->ReqHdr.Modifier )
		  {
			pe_process_resp.exp_blocklen = 6;
			pe_process_resp.pe_method_ptr = PE_query_modes_RQ;	/* prepare corresponding method*/
			pe_process_resp.pe_response = PE_RET_OK;			/* valid service and modifier*/
		  }
		  else if( PE_MOD_SRV_GET_MODE == pSumReqHdr->ReqHdr.Modifier )
		  {
			pe_process_resp.exp_blocklen = 8;
			pe_process_resp.pe_method_ptr = PE_get_mode_RQ;		/* prepare corresponding method*/
			pe_process_resp.pe_response = PE_RET_OK;			/* valid service and modifier*/
		  }
		  break;
		}

		case( PE_SRV_PEM_STATUS ):
		{
		  pe_process_resp.exp_blocklen = 6;

		  if( PE_MOD_SRV_PEM_STATUS == pSumReqHdr->ReqHdr.Modifier )
		  {
			pe_process_resp.pe_method_ptr = PE_pem_status_RQ;	/* prepare corresponding method*/
			pe_process_resp.pe_response = PE_RET_OK;			/* valid service and modifier*/
		  }
		  else if( PE_MOD_PEM_STATUS_WITH_CTTO == pSumReqHdr->ReqHdr.Modifier )
		  {
			pe_process_resp.pe_method_ptr = PE_PEM_Status_with_CTTO;	/* prepare corresponding method*/
			pe_process_resp.pe_response = PE_RET_OK;					/* valid service and modifier*/
		  }
		  break;
		}

		case( PE_SRV_IDENTIFY ):
		{
		  pe_process_resp.exp_blocklen = 6;

		  if( PE_MOD_SRV_END_PAUSE == pSumReqHdr->ReqHdr.Modifier )
		  {
			pe_process_resp.pe_method_ptr = PE_identify_RQ;		/* prepare corresponding method*/
			pe_process_resp.pe_response = PE_RET_OK;			/* valid service and modifier*/
		  }
		  break;
		}

		case( PE_GET_MEASUREMENT_LIST ):
		{
		  if( PE_MOD_GET_MEASUREMENT_LIST == pSumReqHdr->ReqHdr.Modifier )
          {
            pe_process_resp.exp_blocklen = 6;
            pe_process_resp.pe_method_ptr = PE_Get_Measurement_List_No_Obj_Num;     /* prepare corresponding method*/
            pe_process_resp.pe_response = PE_RET_OK;                                /* valid service and modifier*/
          }
          else if( PE_MOD_GET_MEASUREMENT_VALUES == pSumReqHdr->ReqHdr.Modifier )
          {
            pe_process_resp.exp_blocklen = 14;
            pe_process_resp.pe_method_ptr = PE_Get_Measurement_Values_No_Obj_Num;   /* prepare corresponding method*/
            pe_process_resp.pe_response = PE_RET_OK;                                /* valid service and modifier*/
          }
          else if( PE_MOD_GET_MEASUREMENT_LIST_OBJ_NUM == pSumReqHdr->ReqHdr.Modifier )
		  {
			pe_process_resp.exp_blocklen = 6;
			pe_process_resp.pe_method_ptr = PE_Get_Measurement_List;	            /* prepare corresponding method*/
			pe_process_resp.pe_response = PE_RET_OK;					            /* valid service and modifier*/
		  }
		  else if( PE_MOD_GET_MEASUREMENT_VALUES_OBJ_NUM == pSumReqHdr->ReqHdr.Modifier )
		  {
			pe_process_resp.exp_blocklen = 20;
			pe_process_resp.pe_method_ptr = PE_Get_Measurement_Values;	            /* prepare corresponding method*/
			pe_process_resp.pe_response = PE_RET_OK;					            /* valid service and modifier*/
		  }
		  break;
		}

		case( PE_QUERY_VERSION ):
		{
		  pe_process_resp.exp_blocklen = 6;

		  if( PE_MOD_QUERY_VERSION == pSumReqHdr->ReqHdr.Modifier )
		  {
			pe_process_resp.pe_method_ptr = PE_Query_Version;	/* prepare corresponding method*/
			pe_process_resp.pe_response = PE_RET_OK;			/* valid service and modifier*/
		  }
		  break;
		}

        case( PE_QUERY_ATTRIBUTES ):
        {
            pe_process_resp.exp_blocklen = 6; //TODO: this value should be tested.
            if( PE_MOD_QUERY_ATTRIBUTES == pSumReqHdr->ReqHdr.Modifier )
            {
                pe_process_resp.pe_method_ptr = PE_Query_Attributes;	/* prepare corresponding method*/
                pe_process_resp.pe_response = PE_RET_OK;			/* valid service and modifier*/
            }
            break;
        }

		default:
		{	/* requested service is not available */
		  pe_process_resp.pe_response = PE_RET_INVALID_SERVICE_REQUEST_ID;
		  break;
		}
    }

    /* Cleanup PE_RspBuffer */
    OsMemCpy( ( PNIO_VOID* )&PE_RspBuffer[ ArNum ][ 0 ], ( PNIO_VOID* ) pSumReqHdr, sizeof( PE_SUM_REQ_HDR ) );

    /* execute method set by switch */
    if( PE_RET_OK == pe_process_resp.pe_response )
    {
    	pe_process_resp.exp_blockver = BLOCKVERSION;
        Status = usr_rec_check_blockhdr( ( REC_IO_BLOCKHDR* )&( pSumReqHdr->BlockHdr ), /* [in]  pointer to blockheader */
                        				BLOCKHDR_PE_RQ,                 				/* [in]  expected blocktype value */
                                        pe_process_resp.exp_blocklen,                  	/* [in]  expected blocklength value */
										pe_process_resp.exp_blockver,                   /* [in]  expected blockversion value */
                                        pPnioState );                                	/* [out] return 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6 */
        if( Status == PNIO_OK )
        {
          ( pe_process_resp.pe_method_ptr ) ( pBuffer , &PE_RspBuffer[ ArNum ][ 0 ] , ArNum);
        }
        else
        {
            pe_process_resp.pe_response = PE_RET_INVALID_BLOCK_HEADER;
            PE_RspLen[ ArNum ] = PE_SetErrRsp( &PE_RspBuffer[ ArNum ][ 0 ], pe_process_resp.pe_response );
        }
    }
    else	/* report bad request */
    {
        PE_RspLen[ ArNum ] = PE_SetErrRsp( &PE_RspBuffer[ ArNum ][ 0 ], pe_process_resp.pe_response );
    }

    return( Status );
#else
    /* Save parameters to structures and send it to BBB with request */
    /* BBB will send them back together with valid data */

    PNIOext_cbf_pe_request_handler(pAddr, pBufLen, pBuffer, ArNum);

    return PNIO_OK;
#endif
}


/**
 *  @brief Gives prepared response to PNIO_cbf_rec_read
 *
 *  @param[in]   	*pAddr 			location (slot, subslot)
 *  @param[in, out] *pBufLen 		in: length to read, out: length, read by user
 *  @param[in]   	*pBuffer 		buffer pointer
 *  @param[out]  	*pPnioState	4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
 *  @return      					Returns	PNIO_OK (1) if OK
 *
 *  this function handles a record response on RecordIndex 0x80a0,
 *  which is specified as a "PROFIenergy" request.
 *  If an error occures, the error structure PNIO_ERR_STAT is filled.
 *  See PNIO specification for more details, how to fill the error
 *  structure.
 */
PNIO_UINT32  PROFIenergy_ResponseHandler
    (
	  PNIO_DEV_ADDR       	*pAddr,         /* [in] location (slot, subslot)	*/
      PNIO_UINT32			*pBufLen,		/* [in, out] in: length to read, out: length, read by user	*/
      PNIO_UINT8			*pBuffer,		/* [in] buffer pointer	*/
      PNIO_ERR_STAT			*pPnioState,	/* [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6	*/
      PNIO_UINT16           ArNum           /* [in] number of AR initiating request*/
    )
{
#if (1 != IOD_USED_WITH_XHIF_HOST)
    PNIO_UINT32 Status = PNIO_OK;

    /* -------------------------------------------------------------------------- */
    /* check if selected submodule supports PROFIenergy */
    /* In the example GSD file only all the IO-submodules support PROFIenergy, */
    /* but not the DAP submodules. */
    /* If the requested submodule does not support PROFIenergy, invalid index */
    /* has to be returned. */
    /* -------------------------------------------------------------------------- */
    if( pAddr->Geo.Slot == 0 )  /* is DAP ? */
    {
        pPnioState->ErrCode   = 0xde;  /* IODReadRes with ErrorDecode = PNIORW */
        pPnioState->ErrDecode = 0x80;  /* PNIORW */
        pPnioState->ErrCode1  = 0xb0;  /* example: Error Class 11 = Access, ErrorNr 0 = "invalid index" */
        pPnioState->ErrCode2  = 0x00;
        return ( PNIO_NOT_OK );
    }

    /* ----------------------------------------------------------------------- */
    /* note: if data provision from last write request was invalid, */
    /*       then application responds with error code "state conflict". */

    /*       if data provision from last write request is still running, */
    /*       then application would respond with error code "resource busy". */
    /* ----------------------------------------------------------------------- */
    if( ResponseDataAvailable[ArNum] == PNIO_FALSE )
    {
        pPnioState->ErrCode   = 0xde;  /* IODReadRes with ErrorDecode = PNIORW */
        pPnioState->ErrDecode = 0x80;  /* PNIORW */
        pPnioState->ErrCode1  = 0xb5;  /* b = access, 5 = "state conflict" */
        pPnioState->ErrCode2  = 0x00;
        return( PNIO_NOT_OK );
    }
    else
    { /* ** reset flag "response data available", so a new write request must follow.. ** */
        ResponseDataAvailable[ArNum]  = PNIO_FALSE;
    }

    /* **** copy the provided response data to the specified destination buffer *** */
    OsMemCpy( pBuffer, &PE_RspBuffer[ ArNum ][ 0 ], PE_RspLen[ ArNum ] );

    /* **** limit to buffer size and copy receive buffer **** */
    if( PE_RspLen[ ArNum ] < *pBufLen )
        *pBufLen = PE_RspLen[ ArNum ];

    OsMemCpy ( pBuffer, &PE_RspBuffer[ ArNum ][ 0 ], *pBufLen );

    return ( Status );
#else
    /* Save parameters to structures and send it to BBB with request */
    /* BBB will send them back together with valid data */

    PNIOext_cbf_pe_response_handler(pAddr, pBufLen, pBuffer, ArNum);

    return PNIO_OK;
#endif
}


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
