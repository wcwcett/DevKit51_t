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
/*  F i l e               &F: pnpb_peri_diag.h                          :F&  */
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
#ifndef PNPB_PERI_DIAG_H
#define PNPB_PERI_DIAG_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif


// *--------------------------------------------
// * definition of the standard
// * channel diagnostic data and extended
// * diagnostic data.
// * NOTE: must fit to the PNIO-specification!!
// *--------------------------------------------

PNIO_UINT32  pnpb_process_alarm_send
						(PNIO_UINT32	DevHndl,		        // device handle
                         PNIO_UINT32    Api,                    // application process identifier
						 PNIO_DEV_ADDR	*pAddr,		            // geographical or logical address
						 PNIO_UINT8		*pData,		            // AlarmItem.Data
						 PNIO_UINT32	DataLen,		        // length of AlarmItem.Data
						 PNIO_UINT16	UserStructIdent,        // AlarmItem.UserStructureIdentifier, s. IEC61158-6
				 		 PNIO_UINT32	UserHndl); 	            // user defined handle


PNIO_UINT32  pnpb_status_alarm_send
                        (PNIO_UINT32    DevHndl,                // device handle
                         PNIO_UINT32    Api,                    // application process identifier
                         PNIO_DEV_ADDR  *pAddr,                 // geographical or logical address
                         PNIO_UINT8     *pData,                 // AlarmItem.Data
                         PNIO_UINT32    DataLen,                // length of AlarmItem.Data
                         PNIO_UINT16    UserStructIdent,        // AlarmItem.UserStructureIdentifier, s. IEC61158-6
                         PNIO_UINT32    UserHndl);              // user defined handle
						 
PNIO_UINT32  pnpb_diag_channel_add
						(PNIO_UINT32	    DevHndl,		    // device handle
                         PNIO_UINT32        Api,                // application process identifier
						 PNIO_DEV_ADDR      *pAddr,			    // geographical or logical address
						 PNIO_UINT16	    ChannelNum,		    // channel number
						 PNIO_UINT16        ErrorNum,	        // error number, see PNIO specification coding of "ChannelErrorType"
						 DIAG_CHANPROP_DIR  ChanDir,	        // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
						 DIAG_CHANPROP_TYPE ChanTyp,	        // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
                         PNIO_BOOL          MaintenanceReq,     // maintenance required
                         PNIO_BOOL          MaintenanceDem,     // maintenance demanded
						 PNIO_UINT16	    DiagTag); 		    // user defined diag tag != 0

PNIO_UINT32  pnpb_diag_channel_remove
						(PNIO_UINT32	    DevHndl,		    // device handle
                         PNIO_UINT32        Api,                // application process identifier
						 PNIO_DEV_ADDR      *pAddr,			    // geographical or logical address
						 PNIO_UINT16	    ChannelNum,		    // channel number
                         PNIO_UINT16        ErrorNum,           // error number, see PNIO specification coding of "ChannelErrorType"
						 DIAG_CHANPROP_DIR  ChanDir,	        // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
						 DIAG_CHANPROP_TYPE ChanTyp, 	        // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
						 PNIO_UINT16	    DiagTag,            // user defined diag tag != 0
                         PNIO_UINT16        AlarmState);        // DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE

PNIO_UINT32  pnpb_ext_diag_channel_add
						(PNIO_UINT32	    DevHndl,		    // device handle
                         PNIO_UINT32        Api,                // application process identifier
						 PNIO_DEV_ADDR      *pAddr,			    // geographical or logical address
						 PNIO_UINT16	    ChannelNum,		    // channel number
						 PNIO_UINT16        ErrorNum,	        // error number, see PNIO specification coding of "ChannelErrorType"
						 DIAG_CHANPROP_DIR  ChanDir,	        // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
						 DIAG_CHANPROP_TYPE ChanTyp,	        // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
						 PNIO_UINT16	    ExtChannelErrType,	// channel error type           (see PNIO spec.)
						 PNIO_UINT32	    ExtChannelAddValue, // extended channel add. value  (see PNIO spec.)
                         PNIO_BOOL          MaintenanceReq,     // maintenance required
                         PNIO_BOOL          MaintenanceDem,     // maintenance demanded
						 PNIO_UINT16	    DiagTag); 		    // user defined diag tag != 0

PNIO_UINT32  pnpb_ext_diag_channel_remove
                        (PNIO_UINT32        DevHndl,            // device handle
                         PNIO_UINT32        Api,                // application process identifier
                         PNIO_DEV_ADDR      *pAddr,             // geographical or logical address
                         PNIO_UINT16        ChannelNum,         // channel number
                         PNIO_UINT16        ErrorNum,           // error number, see PNIO specification coding of "ChannelErrorType"
                         DIAG_CHANPROP_DIR  ChanDir,            // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
                         DIAG_CHANPROP_TYPE ChanTyp,            // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
                         PNIO_UINT16        ExtChannelErrType,  // channel error type           (see PNIO spec.)
                         PNIO_UINT32        ExtChannelAddValue, // extended channel add. value  (see PNIO spec.)
						 PNIO_UINT16	    DiagTag,            // user defined diag tag != 0
                         PNIO_UINT16        AlarmState);        // DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE

PNIO_UINT32 pnpb_diag_generic_add
		                (PNIO_UINT32	    DevHndl,
                         PNIO_UINT32        Api,
                         PNIO_DEV_ADDR	    *pAddr,			    // location (slot, subslot)
                         PNIO_UINT16        ChanNum,            // channel number
		                 DIAG_CHANPROP_DIR  ChanDir,            // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		                 DIAG_CHANPROP_TYPE ChanTyp,            // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
                         PNIO_UINT16	    DiagTag,		    // user defined diag tag != 0
		                 PNIO_UINT16	    UserStructIdent,    // structure of info data (see IEC 61158)
		                 PNIO_UINT8*	    pInfoData,		    // diag data
		                 PNIO_UINT32	    InfoDataLen,	    // length of diag data in bytes
                         PNIO_BOOL          MaintenanceReq,     // maintenance required
                         PNIO_BOOL          MaintenanceDem);    // maintenance demanded

PNIO_UINT32  pnpb_diag_generic_remove
						(PNIO_UINT32	    DevHndl,		    // device handle
                         PNIO_UINT32        Api,                // application process identifier
						 PNIO_DEV_ADDR      *pAddr,			    // geographical or logical address
						 PNIO_UINT16	    ChannelNum,		    // channel number
						 DIAG_CHANPROP_DIR  ChanDir,	        // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
						 DIAG_CHANPROP_TYPE ChanTyp,	        // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
						 PNIO_UINT16	    DiagTag,		    // user defined diag tag != 0
		                 PNIO_UINT16	    UserStructIdent);   // structure of info data (see IEC 61158)

PNIO_UINT32  pnpb_ret_of_sub_alarm_send
                        (PNIO_UINT32    DevHndl,        // device handle
                         PNIO_UINT32    Api,            // application process identifier
                         PNIO_DEV_ADDR  *pAddr,         // geographical or logical address
                         PNIO_UINT32    UserHndl);      // user defined handle


PNIO_UINT32  pnpb_upload_retrieval_alarm_send
                        (PNIO_UINT32    DevHndl,       // device handle
                         PNIO_UINT32    Api,           // application process identifier
                         PNIO_DEV_ADDR  *pAddr,        // geographical or logical address
                         PNIO_UINT8     *pData,        // AlarmItem.Data
                         PNIO_UINT32    DataLen,       // length of AlarmItem.Data
                         PNIO_UINT32    UsrHndl);      // user defined handle


#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
