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
/*  F i l e               &F: usriod_diag.c                             :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  handle pnio diagnostic                                                   */
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
#include "os.h"
#include "pniousrd.h"
#include "usriod_diag.h"
#include "usriod_utils.h"

/**
* @file     usriod_diag.c
* @brief    functions for handling with pnio diagnostic
*
* THIS MODULE HAS TO BE MODIFIED BY THE PNIO USER
*/

/*===========================================================================*/

    /**
     * @brief UsrChanDiag shows, how to send a standard channel diagnostic alarm.
     *
     * @param[in]   AlarmState   PNIO_UINT16     DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
     * @param[in]   SlotNum      PNIO_UINT32     slot number
     * @param[in]   SubNum       PNIO_UINT32     subslot number
     * @param[in]   ChanNum      PNIO_UINT16     channel number
     * @param[in]   ErrorNum     PNIO_UINT16     error number, see PNIO specification coding of "ChannelErrorType"
     * @param[in]   ChanDir      PNIO_UINT16     channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
     * @param[in]   ChanTyp      PNIO_UINT16     channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
     * @param[in]   DiagTag      PNIO_UINT16     user defined diag tag, used as reference
     *
     * @return  PNIO_UINT32     PNIO_TRUE if sucess, PNIO_FALSE if failure
     *
     * The input parameter AlarmState specifies, if the alarm appears or disappears and if more errors are pending or not.
     * This function sets the following fixed entries, but may be adapted by the user: API                 = PNIO_DEFAULT_API
     */
OS_CODE_FAST PNIO_UINT32 UsrChanDiag     (PNIO_UINT16 AlarmState, // DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
                             PNIO_UINT32 SlotNum,	 // slot number
                             PNIO_UINT32 SubNum,	 // subslot number
                             PNIO_UINT16 ChanNum,	 // channel number
                             PNIO_UINT16 ErrorNum,	 // error number, see PNIO specification coding of "ChannelErrorType"
                             PNIO_UINT16 ChanDir,	 // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
                             PNIO_UINT16 ChanTyp,	 // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
							 PNIO_UINT16 DiagTag)    // user defined diag tag, used as reference
{
	PNIO_UINT32			Status;
	PNIO_DEV_ADDR		Addr;		    // location (slotnumber/subslot number)

    Addr.Geo.Slot = SlotNum;
    Addr.Geo.Subslot = SubNum;

    if (AlarmState == DIAG_CHANPROP_SPEC_ERR_APP)
	{   //--------------------------------------------------
	    //   new alarm appears
	    //--------------------------------------------------

		// *---------------------------------------
		// *  First make a diagnostic entry for this
		// *  subslot. So in the following alarm PDU
		// *  the ChannelDiagExist - bit in the
		// *  alarm specifier is set and the IO
		// *  controller ist notified, that
		// *  diagnostic data are available.
		// *---------------------------------------
		Status = PNIO_diag_channel_add
                (   PNIO_SINGLE_DEVICE_HNDL,  // Device handle
                    PNIO_DEFAULT_API,         // API number
					&Addr,					  // location (slot/subslot)
					ChanNum,				  // channel number 0..0x7fff
					ErrorNum,				  // error number (see IEC 61158)
					ChanDir,				  // channel direction (input, output, input-output)
					ChanTyp, 				  // channel type (data size)
					PNIO_FALSE,               // no maintenance required
					PNIO_FALSE,               // no maintenance demanded
					DiagTag);                 // user defined diag tag
	}
	else
	{//--------------------------------------------------
	 //   alarm disappears
	 //--------------------------------------------------
        Status = PNIO_diag_channel_remove
				(
					PNIO_SINGLE_DEVICE_HNDL,
                    PNIO_DEFAULT_API,
					&Addr,                    // location (slot/subslot)
					ChanNum,				  // channel number 0..0x7fff
                    ErrorNum,                 // error number (see IEC 61158)
					ChanDir,				  // channel direction (input, output, input-output)
					ChanTyp, 				  // channel type (data size)
					DiagTag,                  // user defined diag tag
                    AlarmState);              // DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE


	}

    /* if there are no active ARs, just send acknowledgement to user */
    if( 0 == NumOfAr )
    {
    	PNIO_cbf_async_req_done( PNIO_SINGLE_DEVICE_HNDL, 0, PNIO_ALM_CHAN_DIAG, PNIO_DEFAULT_API, &Addr, 0, DiagTag );
    }

	return (Status);
}

/**
 * @brief UsrExtDiag shows, how to send a extended channel diagnostic alarm.
 *
 * @param[in]   AlarmState              PNIO_UINT16     DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
 * @param[in]   SlotNum                 PNIO_UINT32     slot number
 * @param[in]   SubNum                  PNIO_UINT32     subslot number
 * @param[in]   ChanNum                 PNIO_UINT16     channel number
 * @param[in]   ErrorNum                PNIO_UINT16     error number, see PNIO specification coding of "ChannelErrorType"
 * @param[in]   ExtChannelErrType       PNIO_UINT16     ext. channel error type           (see PNIO spec.)
 * @param[in]   ExtChannelAddValue      PNIO_UINT32     extended channel add. value  (see PNIO spec.)
 * @param[in]   ChanDir                 PNIO_UINT16     channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
 * @param[in]   ChanTyp                 PNIO_UINT16     channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
 * @param[in]   DiagTag                 PNIO_UINT16     user defined diag tag, used as reference
 *
 * @return  PNIO_UINT32     PNIO_TRUE if sucess, PNIO_FALSE if failure
 *
 * The input parameter AlarmState specifies, if the alarm appears or disappears and if more errors are pending or not.
 * This function sets the following fixed entries, but may be adapted by the user: API                 = PNIO_DEFAULT_API
 */
OS_CODE_FAST PNIO_UINT32 UsrExtDiag      (PNIO_UINT16 AlarmState,         // DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
                             PNIO_UINT32 SlotNum,            // slot number
                             PNIO_UINT32 SubNum,             // subslot number
                             PNIO_UINT16 ChanNum,            // channel number
                             PNIO_UINT16 ErrorNum,           // error number, see PNIO specification coding of "ChannelErrorType"
                             PNIO_UINT16 ExtChannelErrType,  // ext. channel error type           (see PNIO spec.)
                             PNIO_UINT32 ExtChannelAddValue, // extended channel add. value  (see PNIO spec.)
                             PNIO_UINT16 ChanDir,            // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
                             PNIO_UINT16 ChanTyp,            // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
							 PNIO_UINT16 DiagTag)            // user defined diag tag, used as reference
{
    PNIO_UINT32         Status;
    PNIO_DEV_ADDR       Addr;           // location (slotnumber/subslot number)

    Addr.Geo.Slot = SlotNum;
    Addr.Geo.Subslot = SubNum;

    if (AlarmState == DIAG_CHANPROP_SPEC_ERR_APP)
    {   //--------------------------------------------------
        //   new alarm appears
        //--------------------------------------------------

        // *---------------------------------------
        // *  First make a diagnostic entry for this
        // *  subslot. So in the following alarm PDU
        // *  the ChannelDiagExist - bit in the
        // *  alarm specifier is set and the IO
        // *  controller ist notified, that
        // *  diagnostic data are available.
        // *---------------------------------------
        Status = PNIO_ext_diag_channel_add
                (   PNIO_SINGLE_DEVICE_HNDL,  // Device handle
                    PNIO_DEFAULT_API,         // API number
                    &Addr,                    // location (slot/subslot)
                    ChanNum,                  // channel number 0..0x7fff
                    ErrorNum,                 // error number (see IEC 61158)
                    ChanDir,                  // channel direction (input, output, input-output)
                    ChanTyp,                  // channel type (data size)
                    ExtChannelErrType,        // extended channel error type  (see PNIO spec.)
                    ExtChannelAddValue,       // extended channel add. value  (see PNIO spec.)
                    PNIO_FALSE,               // no maintenance required
                    PNIO_FALSE,               // no maintenance demanded
					DiagTag);                 // user defined diag tag

    }
    else
    {//--------------------------------------------------
     //   alarm disappears
     //--------------------------------------------------
        Status = PNIO_ext_diag_channel_remove
                (
                    PNIO_SINGLE_DEVICE_HNDL,
                    PNIO_DEFAULT_API,
                    &Addr,                    // location (slot/subslot)
                    ChanNum,                  // channel number 0..0x7fff
                    ErrorNum,                 // error number (see IEC 61158)
                    ChanDir,                  // channel direction (input, output, input-output)
                    ChanTyp,                  // channel type (data size)
                    ExtChannelErrType,        // extended channel error type  (see PNIO spec.)
                    ExtChannelAddValue,       // extended channel add. value  (see PNIO spec.)
					DiagTag,                  // user defined diag tag
                    AlarmState);              // DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE


    }

    /* if there are no active ARs, just send acknowledgement to user */
    if( 0 == NumOfAr )
    {
    	PNIO_cbf_async_req_done( PNIO_SINGLE_DEVICE_HNDL, 0, PNIO_ALM_EXT_CHAN_DIAG, PNIO_DEFAULT_API, &Addr, 0, DiagTag );
    }

    return (Status);
}

/**
 * @brief UsrGenericDiag shows, how to send a generic channel diagnostic alarm.
 *
 * @param[in]   AlarmState              PNIO_UINT16     DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
 * @param[in]   SlotNum                 PNIO_UINT32     slot number
 * @param[in]   SubNum                  PNIO_UINT32     subslot number
 * @param[in]   ChanNum                 PNIO_UINT32     channel number
 * @param[in]   pInfoData               PNIO_VOID*      user defined generic diagnostic data
 * @param[in]   InfoDataLen             PNIO_UINT32     length of generic diagnostic data
 * @param[in]   DiagTag                 PNIO_UINT16     user defined diag tag, used as reference
 * @param[in]   UserStructIdent         PNIO_UINT16     manufacturer specific, 0...0x7fff, see IEC 61158
 * @param[in]   ChanDir                 PNIO_UINT16     channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
 * @param[in]   ChanTyp                 PNIO_UINT16     channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
 *
 * @return  PNIO_UINT32     PNIO_TRUE if sucess, PNIO_FALSE if failure
 *
 * The input parameter AlarmState specifies, if the alarm appears or disappears and if more errors are pending or not.
 * This function sets the following fixed entries, but may be adapted by the user: API                 = PNIO_DEFAULT_API
 */

PNIO_UINT32 UsrGenericDiag
                                (PNIO_UINT16 AlarmState,        // DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
								 PNIO_UINT32 SlotNum,	        // slot number
								 PNIO_UINT32 SubNum,	        // subslot number
								 PNIO_UINT16 ChanNum,	        // channel number
								 PNIO_VOID*  pInfoData,         // user defined generic diagnostic data
                                 PNIO_UINT32 InfoDataLen,       // length of generic diagnostic data
                                 PNIO_UINT16 DiagTag,           // user defined diag tag, used as reference
                                 PNIO_UINT16 UserStructIdent,   // manufacturer specific, 0...0x7fff, see IEC 61158
								 PNIO_UINT16 ChanDir,	        // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
								 PNIO_UINT16 ChanTyp)	        // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
{
	PNIO_UINT32			Status;
	PNIO_DEV_ADDR		Addr;		// location (module/submodule)

	Addr.Type				= PNIO_ADDR_GEO;
	Addr.Geo.Slot			= SlotNum;
	Addr.Geo.Subslot		= SubNum;



	if (AlarmState == DIAG_CHANPROP_SPEC_ERR_APP)
	{//--------------------------------------------------
	 //   new alarm appears
	 //--------------------------------------------------

		Status = PNIO_diag_generic_add
				(	PNIO_SINGLE_DEVICE_HNDL,
                    PNIO_DEFAULT_API,
					&Addr,					        // location (slot/subslot)
					ChanNum,				        // channel number 0..0x7fff
					ChanDir,		                // channel properties
					ChanTyp,		                // channel properties
                    DiagTag,                        // user defined diag tag
					UserStructIdent,	            // 0..0x7fff, see IEC 61158
					(PNIO_UINT8*) pInfoData,	    // generic diagnostic data
                    InfoDataLen,                    // length of generic diagnostic data
                    PNIO_FALSE,                     // no maintenance required
                    PNIO_FALSE);                    // no maintenance demanded


	}
	else
	{//--------------------------------------------------
	 //   alarm disappears
	 //--------------------------------------------------
		Status = PNIO_diag_generic_remove
				(	PNIO_SINGLE_DEVICE_HNDL,
                    PNIO_DEFAULT_API,
					&Addr,					        // location (slot/subslot)
					ChanNum,				        // channel number 0..0x7fff
					ChanDir,		                // channel properties
					ChanTyp,		                // channel properties
                    DiagTag,                        // user defined diag tag
					UserStructIdent);	            // 0..0x7fff, see IEC 61158
	}

    /* if there are no active ARs, just send acknowledgement to user */
    if( 0 == NumOfAr )
    {
    	PNIO_cbf_async_req_done( PNIO_SINGLE_DEVICE_HNDL, 0, PNIO_ALM_GEN_DIAG, PNIO_DEFAULT_API, &Addr, 0, DiagTag );
    }

	return (Status);
}


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
