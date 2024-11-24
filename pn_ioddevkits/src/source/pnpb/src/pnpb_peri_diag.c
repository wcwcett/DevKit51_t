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
/*  F i l e               &F: pnpb_peri_diag.c                          :F&  */
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

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        What                                          */
/*  2017-04-18                 fix07.2                                       */
/*---------------------------------------------------------------------------*/

/**
* @file     pnpb_peri_diag.c
* @brief    Internal PNPB functions for diagnostic alarms and process alarms
*
*/

#include "compiler.h"
#include "pniousrd.h"
#include "trc_if.h"
#include "pndv_inc.h"
#include "pnpb.h"

#define LTRC_ACT_MODUL_ID   206
#define PNPB_MODULE_ID      206


// *-----------------------------------------------------------------
// * defines
// *-----------------------------------------------------------------


// *-----------------------------------------------------------------
// * external functions
// *-----------------------------------------------------------------

// *-----------------------------------------------------------------
// * external variables
// *-----------------------------------------------------------------
extern PNDV_IFACE_STRUCT_PTR    pPnpbIf;

// *=================================================================
// * static functions
// *=================================================================
PNIO_UINT32 volatile ProcessAlarmProcessing = PNIO_FALSE;
PNIO_UINT32 volatile DiagAlarmProcessing    = PNIO_FALSE;
PNIO_UINT32 volatile RosAlarmProcessing     = PNIO_FALSE;
PNIO_UINT32 volatile UplRetrAlarmProcessing = PNIO_FALSE;
PNIO_UINT32 volatile StatusAlarmProcessing = PNIO_FALSE;


// *=================================================================
// * public functions
// *=================================================================


// *-----------------------------------------------------------------
// * PNIO_build_channel_properties (..)
// * builds the channel property data structure for use in
// * channel diagnostic alarms according to IEC 61151
// *-----------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT16 pnpb_build_channel_properties (PNIO_UINT16 type,
                                           PNIO_UINT16 spec,
                                           PNIO_UINT16 dir,
                                           PNIO_BOOL   Accumulative,
                                           PNIO_BOOL   MaintenanceRequired,
                                           PNIO_BOOL   MaintenanceDemanded)
{
  PNIO_UINT16 ChProp;
  ChProp = PNPB_DIAG_CHANNEL_PROPERTIES_MAKE (type, Accumulative, MaintenanceRequired, MaintenanceDemanded, spec, dir);
  return (ChProp);
}


/**
 * @brief add channel diagnostic alarm to PN stack.
 *
 * @param[in]   DevHndl              PNIO_UINT32         device handle
 * @param[in]   Api                  PNIO_UINT32         application process identifier
 * @param[in]   pAddr                PNIO_DEV_ADDR*      geographical or logical address
 * @param[in]   ChannelNum           PNIO_UINT16         channel number
 * @param[in]   ErrorNum             PNIO_UINT16         error number, see PNIO specification coding of "ChannelErrorType"
 * @param[in]   ChanDir              DIAG_CHANPROP_DIR   channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
 * @param[in]   ChanTyp              DIAG_CHANPROP_TYPE  channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
 * @param[in]   MaintenanceReq       PNIO_BOOL           maintenance required
 * @param[in]   MaintenanceDem       PNIO_BOOL           maintenance demanded
 * @param[in]   DiagTag              PNIO_UINT16         user defined diag tag != 0
 *
 * @return  PNIO_UINT32     PNIO_TRUE if sucess, PNIO_FALSE if failure
 *
 * Add diagnostic 'add' event to PNDV structure and send it to PNDV module
 */
PNPB_CODE_FAST PNIO_UINT32  pnpb_diag_channel_add
						(PNIO_UINT32	    DevHndl,		// device handle
                         PNIO_UINT32        Api,            // application process identifier
						 PNIO_DEV_ADDR      *pAddr,			// geographical or logical address
						 PNIO_UINT16	    ChanNum,		// channel number
						 PNIO_UINT16	    ErrNum,	        // error number
						 DIAG_CHANPROP_DIR  ChanDir,        // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
						 DIAG_CHANPROP_TYPE ChanTyp,        // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
                         PNIO_BOOL          MaintenanceReq, // maintenance required
                         PNIO_BOOL          MaintenanceDem, // maintenance demanded
						 PNIO_UINT16	    DiagTag)		// user defined diag tag != 0
{
    PNDV_REAL_CFG_T* iface_real_cfg_ptr;
    PNIO_UINT8       cmd = 0;
    PNIO_UINT32      Ind;

    PNIO_UINT16             ChanProp;   // channel properties in machine dependant endian format

    LSA_UNUSED_ARG (DevHndl);

    PNPB_API_TRACE_05(LSA_TRACE_LEVEL_NOTE_HIGH,  "AddChDiag api%d sl%d ss%d Ch%d Err%d ExtTyp ExtVal",
                      Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, ChanNum, ErrNum);

    ChanProp = (PNIO_UINT16)pnpb_build_channel_properties
                    ( (PNIO_UINT16) ChanTyp,		            // type
                      (PNIO_UINT16) DIAG_CHANPROP_SPEC_ERR_APP,	// new error appears
                      (PNIO_UINT16) ChanDir,                    // IO data direction (in/out)
                      PNIO_FALSE,                               // not accumulative
                      MaintenanceReq,                           // no maintenance required
                      MaintenanceDem);                          // no maintenance demanded


        // *** get entity index in real_cfg ***
    Ind = PnpbRealCfgGetEntityInd (Api,
                                   pAddr->Geo.Slot,
                                   pAddr->Geo.Subslot);

    if (Ind == PNPB_INVALID_INDEX)
    {
       	PNPB_SYS_TRACE_03(LSA_TRACE_LEVEL_ERROR,  "invalid alarm location api%d slot%d subsl%d",
                          Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        return (PNIO_FALSE);
    }

    if (DiagAlarmProcessing == PNIO_TRUE)
    {
        PNPB_SYS_TRACE_04(LSA_TRACE_LEVEL_ERROR, "error: new alarm (sl%d ss%d ch%d err%d)before previous has been confirmed",
            pAddr->Geo.Slot, pAddr->Geo.Subslot, ChanNum, ErrNum);
        PnpbSetLastError(PNIO_ERR_IOD_ALARM_SEQUENCE);
        return (PNIO_NOT_OK);
    }

    iface_real_cfg_ptr = &pPnpbIf->real_cfg[Ind];

    if ( iface_real_cfg_ptr->dial.state == PNDV_IFACE_SERVICE_IDLE )
    {
        iface_real_cfg_ptr->dial.state = PNDV_IFACE_SERVICE_NEW;

        iface_real_cfg_ptr->dial.anz_chn_diag = 1;

        iface_real_cfg_ptr->dial.chn_diag[0].alm_kommend = LSA_TRUE;

        iface_real_cfg_ptr->dial.chn_diag[0].fehler     = ErrNum;
        iface_real_cfg_ptr->dial.chn_diag[0].kanal      = ChanNum;
        iface_real_cfg_ptr->dial.chn_diag[0].properties = ChanProp;
        iface_real_cfg_ptr->dial.chn_diag[0].diag_tag   = DiagTag;

        // *** set service ****
        cmd = PNDV_EV_TO_PNDV_CHANNEL_DIAG;

        // *** trigger PNDV to serve this request ***
        {
            DiagAlarmProcessing = PNIO_TRUE;
            pnpb_write_event_to_pndv (cmd, 0, (PNIO_UINT16)Ind, (PNIO_VOID*)NULL);
            PNPB_TRIGGER_PNDV ();
        }
    }
    else
    {
        return (PNIO_FALSE);
    }

    return (PNIO_TRUE);
}


/**
 * @brief remove channel diagnostic alarm to PN stack.
 *
 * @param[in]   DevHndl              PNIO_UINT32         device handle
 * @param[in]   Api                  PNIO_UINT32         application process identifier
 * @param[in]   pAddr                PNIO_DEV_ADDR*      geographical or logical address
 * @param[in]   ChannelNum           PNIO_UINT16         channel number
 * @param[in]   ErrorNum             PNIO_UINT16         error number, see PNIO specification coding of "ChannelErrorType"
 * @param[in]   ChanDir              DIAG_CHANPROP_DIR   channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
 * @param[in]   ChanTyp              DIAG_CHANPROP_TYPE  channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
 * @param[in]   DiagTag              PNIO_UINT16         user defined diag tag != 0
 *
 * @return  PNIO_UINT32     PNIO_TRUE if sucess, PNIO_FALSE if failure
 *
 * Add diagnostic 'remove' event to PNDV structure and send it to PNDV module
 */
PNIO_UINT32  pnpb_diag_channel_remove
						(PNIO_UINT32	    DevHndl,		// device handle
                         PNIO_UINT32        Api,            // application process identifier
						 PNIO_DEV_ADDR      *pAddr,			// geographical or logical address
						 PNIO_UINT16	    ChanNum,		// channel number
                         PNIO_UINT16        ErrNum,         // error number
						 DIAG_CHANPROP_DIR  ChanDir,        // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
						 DIAG_CHANPROP_TYPE ChanTyp,        // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
						 PNIO_UINT16	    DiagTag,		// user defined diag tag != 0
                         PNIO_UINT16        AlarmState)     // DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE

{
    PNDV_REAL_CFG_T* iface_real_cfg_ptr;
    PNIO_UINT8       cmd = 0;
    PNIO_UINT32      Ind;

    PNIO_UINT16             ChanProp;   // channel properties in machine dependant endian format

    LSA_UNUSED_ARG (DevHndl);

    PNPB_API_TRACE_05(LSA_TRACE_LEVEL_NOTE_HIGH,  "removChDiag api%d sl%d ss%d Ch%d Err%d ExtTyp ExtVal",
                      Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, ChanNum, ErrNum);

    ChanProp = (PNIO_UINT16)pnpb_build_channel_properties
                    ( (PNIO_UINT16) ChanTyp,		                // type
                      (PNIO_UINT16) AlarmState,	                    // error disappears
                      (PNIO_UINT16) ChanDir,                        // IO data direction (in/out)
                      PNIO_FALSE,                                   // not accumulative
                      PNIO_FALSE,                                   // no maintenance required
                      PNIO_FALSE);                                  // no maintenance demanded


        // *** get entity index in real_cfg ***
    Ind = PnpbRealCfgGetEntityInd (Api,
                                   pAddr->Geo.Slot,
                                   pAddr->Geo.Subslot);

    if (Ind == PNPB_INVALID_INDEX)
    {
       	PNPB_SYS_TRACE_03(LSA_TRACE_LEVEL_ERROR,  "invalid alarm location api%d slot%d subsl%d",
                          Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        return (PNIO_FALSE);
    }

    if (DiagAlarmProcessing == PNIO_TRUE)
    {
        PNPB_SYS_TRACE_04(LSA_TRACE_LEVEL_ERROR,  "error: new alarm (sl%d ss%d ch%d err%d)before previous has been confirmed",
                       pAddr->Geo.Slot, pAddr->Geo.Subslot, ChanNum, ErrNum);
        PnpbSetLastError (PNIO_ERR_IOD_ALARM_SEQUENCE);
        return (PNIO_NOT_OK);
    }

    iface_real_cfg_ptr = &pPnpbIf->real_cfg[Ind];

    if ( iface_real_cfg_ptr->dial.state == PNDV_IFACE_SERVICE_IDLE )
    {
        iface_real_cfg_ptr->dial.state = PNDV_IFACE_SERVICE_NEW;

        iface_real_cfg_ptr->dial.anz_chn_diag = 1;

        iface_real_cfg_ptr->dial.chn_diag[0].alm_kommend = LSA_FALSE;

        iface_real_cfg_ptr->dial.chn_diag[0].fehler     = ErrNum;
        iface_real_cfg_ptr->dial.chn_diag[0].kanal      = ChanNum;
        iface_real_cfg_ptr->dial.chn_diag[0].properties = ChanProp;
        iface_real_cfg_ptr->dial.chn_diag[0].diag_tag   = DiagTag;

        // *** set service ****
        cmd = PNDV_EV_TO_PNDV_CHANNEL_DIAG;

        // *** trigger PNDV to serve this request ***
        {
            DiagAlarmProcessing = PNIO_TRUE;
            pnpb_write_event_to_pndv (cmd, 0, (PNIO_UINT16)Ind, (PNIO_VOID*)NULL);
            PNPB_TRIGGER_PNDV ();
        }
    }
    else
    {
        return (PNIO_FALSE);
    }
    return (PNIO_TRUE);
}



/**
 * @brief add extended channel diagnostic alarm to PN stack.
 *
 * @param[in]   DevHndl              PNIO_UINT32         device handle
 * @param[in]   Api                  PNIO_UINT32         application process identifier
 * @param[in]   pAddr                PNIO_DEV_ADDR*      geographical or logical address
 * @param[in]   ChannelNum           PNIO_UINT16         channel number
 * @param[in]   ErrorNum             PNIO_UINT16         error number, see PNIO specification coding of "ChannelErrorType"
 * @param[in]   ChanDir              DIAG_CHANPROP_DIR   channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
 * @param[in]   ChanTyp              DIAG_CHANPROP_TYPE  channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
 * @param[in]   ExtChannelErrType    PNIO_UINT16         ext. channel error type           (see PNIO spec.)
 * @param[in]   ExtChannelAddValue   PNIO_UINT32         extended channel add. value  (see PNIO spec.)
 * @param[in]   MaintenanceReq       PNIO_BOOL           maintenance required
 * @param[in]   MaintenanceDem       PNIO_BOOL           maintenance demanded
 * @param[in]   DiagTag              PNIO_UINT16         user defined diag tag != 0
 *
 * @return  PNIO_UINT32     PNIO_TRUE if sucess, PNIO_FALSE if failure
 *
 * Add extended diagnostic 'add' event to PNDV structure and send it to PNDV module
 */
PNPB_CODE_FAST PNIO_UINT32  pnpb_ext_diag_channel_add
						(PNIO_UINT32	    DevHndl,		    // device handle
                         PNIO_UINT32        Api,                // application process identifier
						 PNIO_DEV_ADDR      *pAddr,			    // geographical or logical address
						 PNIO_UINT16	    ChanNum,		    // channel number
						 PNIO_UINT16        ErrNum,	            // error number, see PNIO specification coding of "ChannelErrorType"
						 DIAG_CHANPROP_DIR  ChanDir,	        // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
						 DIAG_CHANPROP_TYPE ChanTyp,	        // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
						 PNIO_UINT16	    ExtChannelErrType,	// channel error type           (see PNIO spec.)
						 PNIO_UINT32	    ExtChannelAddValue, // extended channel add. value  (see PNIO spec.)
                         PNIO_BOOL          MaintenanceReq,     // maintenance required
                         PNIO_BOOL          MaintenanceDem,     // maintenance demanded
						 PNIO_UINT16	    DiagTag)			// user defined diag tag != 0
{
    PNDV_REAL_CFG_T* iface_real_cfg_ptr;
    PNIO_UINT8       cmd = 0;
    PNIO_UINT32      Ind;

    PNIO_UINT16             ChanProp;   // channel properties in machine dependant endian format

    LSA_UNUSED_ARG (DevHndl);

    PNPB_API_TRACE_07(LSA_TRACE_LEVEL_NOTE_HIGH,  "add ExtChDiag api%d sl%d ss%d Ch%d Err%d ExtTyp ExtVal",
                      Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, ChanNum, ErrNum, ExtChannelErrType, ExtChannelAddValue);


    ChanProp = (PNIO_UINT16)pnpb_build_channel_properties
                    ( (PNIO_UINT16) ChanTyp,                    // type
                      (PNIO_UINT16) DIAG_CHANPROP_SPEC_ERR_APP, // new error appears
                      (PNIO_UINT16) ChanDir,                    // IO data direction (in/out)
                      PNIO_FALSE,                               // not accumulative
                      MaintenanceReq,                           // no maintenance required
                      MaintenanceDem);                          // no maintenance demanded



        // *** get entity index in real_cfg ***
    Ind = PnpbRealCfgGetEntityInd (Api,
                                   pAddr->Geo.Slot,
                                   pAddr->Geo.Subslot);

    if (Ind == PNPB_INVALID_INDEX)
    {
        PNPB_API_TRACE_03(LSA_TRACE_LEVEL_ERROR,  "invalid alarm location api%d slot%d subsl%d",
                          Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        return (PNIO_FALSE);
    }

    iface_real_cfg_ptr = &pPnpbIf->real_cfg[Ind];

    if (DiagAlarmProcessing == PNIO_TRUE)
    {
        PNPB_SYS_TRACE_04(LSA_TRACE_LEVEL_ERROR,  "error: new alarm (sl%d ss%d ch%d err%d)before previous has been confirmed",
                       pAddr->Geo.Slot, pAddr->Geo.Subslot, ChanNum, ErrNum);
        PnpbSetLastError (PNIO_ERR_IOD_ALARM_SEQUENCE);
        return (PNIO_NOT_OK);
    }

    if ( iface_real_cfg_ptr->xdial.state == PNDV_IFACE_SERVICE_IDLE )
    {
        iface_real_cfg_ptr->xdial.state = PNDV_IFACE_SERVICE_NEW;

        iface_real_cfg_ptr->xdial.anz_chn_diag = 1;

        iface_real_cfg_ptr->xdial.ext_diag[0].alm_kommend = LSA_TRUE;

        iface_real_cfg_ptr->xdial.ext_diag[0].fehler     = ErrNum;
        iface_real_cfg_ptr->xdial.ext_diag[0].kanal      = ChanNum;
        iface_real_cfg_ptr->xdial.ext_diag[0].properties = ChanProp;
        iface_real_cfg_ptr->xdial.ext_diag[0].ext_fehler = ExtChannelErrType;
        iface_real_cfg_ptr->xdial.ext_diag[0].ext_wert   = ExtChannelAddValue;
        iface_real_cfg_ptr->xdial.ext_diag[0].diag_tag   = DiagTag;

        // *** set service ****
        cmd = PNDV_EV_TO_PNDV_EXT_CHANNEL_DIAG;

        // *** trigger PNDV to serve this request ***
        {
            DiagAlarmProcessing = PNIO_TRUE;
            pnpb_write_event_to_pndv (cmd, 0, (PNIO_UINT16)Ind, (PNIO_VOID*)NULL);
            PNPB_TRIGGER_PNDV ();
        }
    }
    else
    {
        return (PNIO_FALSE);
    }
    return (PNIO_TRUE);
}


/**
 * @brief remove extended channel diagnostic alarm to PN stack.
 *
 * @param[in]   DevHndl              PNIO_UINT32         device handle
 * @param[in]   Api                  PNIO_UINT32         application process identifier
 * @param[in]   pAddr                PNIO_DEV_ADDR*      geographical or logical address
 * @param[in]   ChannelNum           PNIO_UINT16         channel number
 * @param[in]   ErrorNum             PNIO_UINT16         error number, see PNIO specification coding of "ChannelErrorType"
 * @param[in]   ChanDir              DIAG_CHANPROP_DIR   channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
 * @param[in]   ChanTyp              DIAG_CHANPROP_TYPE  channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
 * @param[in]   ExtChannelErrType    PNIO_UINT16         ext. channel error type           (see PNIO spec.)
 * @param[in]   ExtChannelAddValue   PNIO_UINT32         extended channel add. value  (see PNIO spec.)
 * @param[in]   DiagTag              PNIO_UINT16         user defined diag tag != 0
 *
 * @return  PNIO_UINT32     PNIO_TRUE if sucess, PNIO_FALSE if failure
 *
 * Add extended diagnostic 'remove' event to PNDV structure and send it to PNDV module
 */
PNPB_CODE_FAST PNIO_UINT32  pnpb_ext_diag_channel_remove
                        (PNIO_UINT32        DevHndl,            // device handle
                         PNIO_UINT32        Api,                // application process identifier
                         PNIO_DEV_ADDR      *pAddr,             // geographical or logical address
                         PNIO_UINT16        ChanNum,            // channel number
                         PNIO_UINT16        ErrNum,             // error number
                         DIAG_CHANPROP_DIR  ChanDir,            // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
                         DIAG_CHANPROP_TYPE ChanTyp,            // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
                         PNIO_UINT16        ExtChannelErrType,  // channel error type           (see PNIO spec.)
                         PNIO_UINT32        ExtChannelAddValue, // extended channel add. value  (see PNIO spec.)
						 PNIO_UINT16	    DiagTag,			// user defined diag tag != 0
                         PNIO_UINT16        AlarmState)         // DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
{
    PNDV_REAL_CFG_T* iface_real_cfg_ptr;
    PNIO_UINT8       cmd = 0;
    PNIO_UINT32      Ind;

    PNIO_UINT16             ChanProp;   // channel properties in machine dependant endian format

    LSA_UNUSED_ARG (DevHndl);

    PNPB_API_TRACE_07(LSA_TRACE_LEVEL_NOTE_HIGH,  "remExtChDiag api%d sl%d ss%d Ch%d Err%d ExtTyp ExtVal",
                      Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, ChanNum, ErrNum, ExtChannelErrType, ExtChannelAddValue);


    ChanProp = (PNIO_UINT16)pnpb_build_channel_properties
                    ( (PNIO_UINT16) ChanTyp,                        // type
                      (PNIO_UINT16) AlarmState,                     // error disappears
                      (PNIO_UINT16) ChanDir,                        // IO data direction (in/out)
                      PNIO_FALSE,                                   // not accumulative
                      PNIO_FALSE,                                   // no maintenance required
                      PNIO_FALSE);                                  // no maintenance demanded


        // *** get entity index in real_cfg ***
    Ind = PnpbRealCfgGetEntityInd (Api,
                                   pAddr->Geo.Slot,
                                   pAddr->Geo.Subslot);

    if (Ind == PNPB_INVALID_INDEX)
    {
        PNPB_API_TRACE_03(LSA_TRACE_LEVEL_ERROR,  "invalid alarm location api%d slot%d subsl%d",
                          Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        return (PNIO_FALSE);
    }

    if (DiagAlarmProcessing == PNIO_TRUE)
    {
        PNPB_SYS_TRACE_04(LSA_TRACE_LEVEL_ERROR,  "error: new alarm (sl%d ss%d ch%d err%d)before previous has been confirmed",
                       pAddr->Geo.Slot, pAddr->Geo.Subslot, ChanNum, ErrNum);
        PnpbSetLastError (PNIO_ERR_IOD_ALARM_SEQUENCE);
        return (PNIO_NOT_OK);
    }

    iface_real_cfg_ptr = &pPnpbIf->real_cfg[Ind];

    if ( iface_real_cfg_ptr->xdial.state == PNDV_IFACE_SERVICE_IDLE )
    {
        iface_real_cfg_ptr->xdial.state = PNDV_IFACE_SERVICE_NEW;

        iface_real_cfg_ptr->xdial.anz_chn_diag = 1;

        iface_real_cfg_ptr->xdial.ext_diag[0].alm_kommend = LSA_FALSE;

        iface_real_cfg_ptr->xdial.ext_diag[0].fehler     = ErrNum;
        iface_real_cfg_ptr->xdial.ext_diag[0].kanal      = ChanNum;
        iface_real_cfg_ptr->xdial.ext_diag[0].properties = ChanProp;
        iface_real_cfg_ptr->xdial.ext_diag[0].ext_fehler = ExtChannelErrType;
        iface_real_cfg_ptr->xdial.ext_diag[0].ext_wert   = ExtChannelAddValue;
        iface_real_cfg_ptr->xdial.ext_diag[0].diag_tag   = DiagTag;

        // *** set service ****
        cmd = PNDV_EV_TO_PNDV_EXT_CHANNEL_DIAG;

        // *** trigger PNDV to serve this request ***
        {
            DiagAlarmProcessing = PNIO_TRUE;
            pnpb_write_event_to_pndv (cmd, 0, (PNIO_UINT16)Ind, (PNIO_VOID*)NULL);
            PNPB_TRIGGER_PNDV ();
        }
    }
    else
    {
        return (PNIO_FALSE);
    }
    return (PNIO_TRUE);
}

/**
 * @brief add generic channel diagnostic alarm to PN stack.
 *
 * @param[in]   DevHndl              PNIO_UINT32         device handle
 * @param[in]   Api                  PNIO_UINT32         application process identifier
 * @param[in]   pAddr                PNIO_DEV_ADDR*      geographical or logical address
 * @param[in]   ChanDir              DIAG_CHANPROP_DIR   channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
 * @param[in]   ChanTyp              DIAG_CHANPROP_TYPE  channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
 * @param[in]   DiagTag              PNIO_UINT16         user defined diag tag != 0
 * @param[in]   UserStructIdent      PNIO_UINT16         manufacturer specific, 0...0x7fff, see IEC 61158
 * @param[in]   pInfoData            PNIO_UINT8*         user defined generic diagnostic data
 * @param[in]   InfoDataLen          PNIO_UINT32         length of generic diagnostic data
 * @param[in]   MaintenanceReq       PNIO_BOOL           maintenance required
 * @param[in]   MaintenanceDem       PNIO_BOOL           maintenance demanded
 *
 * @return  PNIO_UINT32     PNIO_TRUE if sucess, PNIO_FALSE if failure
 *
 * Add generic diagnostic 'add' event to PNDV structure and send it to PNDV module
 */
PNIO_UINT32 pnpb_diag_generic_add
		(PNIO_UINT32	    DevHndl,
         PNIO_UINT32        Api,
         PNIO_DEV_ADDR	    *pAddr,			// location (slot, subslot)
         PNIO_UINT16        ChanNum,        // channel number
		 DIAG_CHANPROP_DIR  ChanDir,        // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		 DIAG_CHANPROP_TYPE ChanTyp,        // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
         PNIO_UINT16	    DiagTag,		// user defined diag tag != 0
		 PNIO_UINT16	    UserStructIdent,// structure of info data (see IEC 61158)
		 PNIO_UINT8*	    pInfoData,		// diag data
		 PNIO_UINT32	    InfoDataLen,	// length of diag data in bytes
         PNIO_BOOL          MaintenanceReq, // maintenance required
         PNIO_BOOL          MaintenanceDem) // maintenance demanded
{
    PNDV_GENERIC_DIAG_T* iface_gendiag_ptr;
    PNIO_UINT8       cmd = 0;
    PNIO_UINT32      Ind = 0;       // we only use index = 0 --> send one generic diag at a time

    //PnpbRealCfgGetApiSlotSubslot(Ind, &Api, &pAddr->Geo.Slot, &pAddr->Geo.Subslot);

    LSA_UNUSED_ARG (DevHndl);

    PNPB_API_TRACE_05(LSA_TRACE_LEVEL_NOTE_HIGH,  "add generic diag alarm api%d slot%d subsl%d usi0x%x DTag0x%x\n",
                      Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, UserStructIdent, DiagTag);

    // *** get entity index in real_cfg ***
    Ind = PnpbRealCfgGetEntityInd (Api,
                                   pAddr->Geo.Slot,
                                   pAddr->Geo.Subslot);

    if (Ind == PNPB_INVALID_INDEX)
    {
        PNPB_API_TRACE_03(LSA_TRACE_LEVEL_ERROR,  "invalid alarm location api%d slot%d subsl%d",
                          Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        return (PNIO_FALSE);
    }

    iface_gendiag_ptr = &pPnpbIf->generic_diag_data[Ind];

    if (DiagAlarmProcessing == PNIO_TRUE)
    {
        PNPB_SYS_TRACE_04(LSA_TRACE_LEVEL_ERROR,  "error: new alarm (sl%d ss%d ch%d tag%d)before previous has been confirmed",
                       pAddr->Geo.Slot, pAddr->Geo.Subslot, ChanNum, DiagTag);
        PnpbSetLastError (PNIO_ERR_IOD_ALARM_SEQUENCE);
        return (PNIO_NOT_OK);
    }

    if ( iface_gendiag_ptr->state == PNDV_IFACE_SERVICE_IDLE )
    {
        iface_gendiag_ptr->state = PNDV_IFACE_SERVICE_NEW;

        iface_gendiag_ptr->alm_kommend     = LSA_TRUE;
        iface_gendiag_ptr->interface_index = 0;
        iface_gendiag_ptr->diag_tag        = DiagTag;
        iface_gendiag_ptr->api             = Api;
        iface_gendiag_ptr->data            = pInfoData;
        iface_gendiag_ptr->length          = (PNIO_UINT16)InfoDataLen;
        iface_gendiag_ptr->slot            = (PNIO_UINT16)pAddr->Geo.Slot;
        iface_gendiag_ptr->subslot         = (PNIO_UINT16)pAddr->Geo.Subslot;
        iface_gendiag_ptr->usi             = UserStructIdent;

        // *** set service ****
        cmd = PNDV_EV_TO_PNDV_GENERIC_DIAG;

        // *** trigger PNDV to serve this request ***
        {
            DiagAlarmProcessing = PNIO_TRUE;
            pnpb_write_event_to_pndv (cmd, (PNIO_UINT8)0, (PNIO_UINT16)Ind, NULL /*(PNIO_VOID*)DiagTag */  );
            PNPB_TRIGGER_PNDV ();
        }
    }
    else
    {
        return (PNIO_FALSE);
    }
    return (PNIO_TRUE);

}

/**
 * @brief remove generic channel diagnostic alarm to PN stack.
 *
 * @param[in]   DevHndl              PNIO_UINT32         device handle
 * @param[in]   Api                  PNIO_UINT32         application process identifier
 * @param[in]   pAddr                PNIO_DEV_ADDR*      geographical or logical address
 * @param[in]   ChanDir              DIAG_CHANPROP_DIR   channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
 * @param[in]   ChanTyp              DIAG_CHANPROP_TYPE  channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
 * @param[in]   DiagTag              PNIO_UINT16         user defined diag tag != 0
 * @param[in]   UserStructIdent      PNIO_UINT16         manufacturer specific, 0...0x7fff, see IEC 61158
 *
 * @return  PNIO_UINT32     PNIO_TRUE if sucess, PNIO_FALSE if failure
 *
 * Add generic diagnostic 'remove' event to PNDV structure and send it to PNDV module
 */
PNIO_UINT32  pnpb_diag_generic_remove
						(PNIO_UINT32	    DevHndl,		    // device handle
                         PNIO_UINT32        Api,                // application process identifier
						 PNIO_DEV_ADDR      *pAddr,			    // geographical or logical address
						 PNIO_UINT16	    ChanNum,		    // channel number
						 DIAG_CHANPROP_DIR  ChanDir,	        // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
						 DIAG_CHANPROP_TYPE ChanTyp,	        // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
						 PNIO_UINT16	    DiagTag,		    // user defined diag tag != 0
		                 PNIO_UINT16	    UserStructIdent)    // structure of info data (see IEC 61158)
{
    PNDV_GENERIC_DIAG_T* iface_gendiag_ptr;
    PNIO_UINT8       cmd = 0;
    PNIO_UINT32      Ind = 0;       // we only use index = 0 --> send one generic diag at a time

    LSA_UNUSED_ARG (DevHndl);

    //PnpbRealCfgGetApiSlotSubslot(Ind, &Api, &pAddr->Geo.Slot, &pAddr->Geo.Subslot);

    PNPB_API_TRACE_05(LSA_TRACE_LEVEL_NOTE_HIGH,  "remove generic diag alarm api%d slot%d subsl%d usi0x%x DTag0x%x\n",
                      Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, UserStructIdent, DiagTag);

    // *** get entity index in real_cfg ***
    Ind = PnpbRealCfgGetEntityInd (Api,
                                   pAddr->Geo.Slot,
                                   pAddr->Geo.Subslot);

    if (Ind == PNPB_INVALID_INDEX)
    {
        PNPB_API_TRACE_03(LSA_TRACE_LEVEL_ERROR,  "invalid alarm location api%d slot%d subsl%d",
                          Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        return (PNIO_FALSE);
    }

    iface_gendiag_ptr = &pPnpbIf->generic_diag_data[Ind];

    if (DiagAlarmProcessing == PNIO_TRUE)
    {
        PNPB_SYS_TRACE_04(LSA_TRACE_LEVEL_ERROR,  "error: new alarm (sl%d ss%d ch%d tag%d)before previous has been confirmed",
                       pAddr->Geo.Slot, pAddr->Geo.Subslot, ChanNum, DiagTag);
        PnpbSetLastError (PNIO_ERR_IOD_ALARM_SEQUENCE);
        return (PNIO_NOT_OK);
    }

    if ( iface_gendiag_ptr->state == PNDV_IFACE_SERVICE_IDLE )
    {
        iface_gendiag_ptr->state = PNDV_IFACE_SERVICE_NEW;

        iface_gendiag_ptr->alm_kommend     = LSA_FALSE;
        iface_gendiag_ptr->interface_index = 0;
        iface_gendiag_ptr->diag_tag        = DiagTag;
        iface_gendiag_ptr->api             = Api;
        iface_gendiag_ptr->data            = NULL;
        iface_gendiag_ptr->length          = 0;
        iface_gendiag_ptr->slot            = (PNIO_UINT16)pAddr->Geo.Slot;
        iface_gendiag_ptr->subslot         = (PNIO_UINT16)pAddr->Geo.Subslot;
        iface_gendiag_ptr->usi             = UserStructIdent;

        // *** set service ****
        cmd = PNDV_EV_TO_PNDV_GENERIC_DIAG;

        // *** trigger PNDV to serve this request ***
        {
            DiagAlarmProcessing = PNIO_TRUE;
            pnpb_write_event_to_pndv (cmd, 0, (PNIO_UINT16)Ind, NULL /*(PNIO_VOID*)DiagTag */  );
            PNPB_TRIGGER_PNDV ();
        }
    }
    else
    {
        return (PNIO_FALSE);
    }
    return (PNIO_TRUE);
}


// *-----------------------------------------------------------------
// * pnpb_process_alarm_send (..)
// *
// *
// *-----------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32  pnpb_process_alarm_send
                        (PNIO_UINT32    DevHndl,                // device handle
                         PNIO_UINT32    Api,                    // application process identifier
                         PNIO_DEV_ADDR  *pAddr,                 // geographical or logical address
                         PNIO_UINT8     *pData,                 // AlarmItem.Data
                         PNIO_UINT32    DataLen,                // length of AlarmItem.Data
                         PNIO_UINT16    UserStructIdent,        // AlarmItem.UserStructureIdentifier, s. IEC61158-6
                         PNIO_UINT32    UserHndl)               // user defined handle
{
    PNDV_REAL_CFG_T* iface_real_cfg_ptr;
    PNIO_UINT8       cmd = 0;
    PNIO_UINT32      Ind;
    PNIO_EXP_SUB*    pSubExp;
    PNIO_UINT32       ArInd;
    LSA_UNUSED_ARG (DevHndl);

        // *** get entity index in real_cfg ***
    Ind = PnpbRealCfgGetEntityInd (Api,
                                   pAddr->Geo.Slot,
                                   pAddr->Geo.Subslot);

    if (Ind == PNPB_INVALID_INDEX)
    {
        PNPB_API_TRACE_03(LSA_TRACE_LEVEL_ERROR,  "invalid alarm location api%d slot%d subsl%d",
                          Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        PnpbSetLastError (PNIO_ERR_IOD_INVALID_SUBSLOT);
        return (PNIO_FALSE);
    }
    if (DataLen > PNDV_AL_PRAL_INFO_LEN)
    {
        PNPB_API_TRACE_02(LSA_TRACE_LEVEL_ERROR,  "alarm data size too big %d (%d)",
                          DataLen, PNDV_AL_PRAL_INFO_LEN);
        PnpbSetLastError (PNIO_ERR_IOD_INVALID_BUFLEN);
        return (PNIO_FALSE);
    }

    if (ProcessAlarmProcessing == PNIO_TRUE)
    {
        PNPB_SYS_TRACE_03(LSA_TRACE_LEVEL_ERROR,  "error: new alarm (sl%d ss%d ch%d hnd%d)before previous has been confirmed",
                       pAddr->Geo.Slot, pAddr->Geo.Subslot, UserHndl);
        PnpbSetLastError (PNIO_ERR_IOD_ALARM_SEQUENCE);
        return (PNIO_NOT_OK);
    }

    PNPB_API_TRACE_04(LSA_TRACE_LEVEL_NOTE_HIGH,  "send procAlarm api%d slot%d subsl%d usi0x%x",
                      Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, UserStructIdent);

    // ***** check if one of the AR's is owner of the specified subslot
    pSubExp = pnpb_sub_exp_allAr_getp (&ArInd, Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
    if (pSubExp == NULL)
    {
        PNPB_API_TRACE_02(LSA_TRACE_LEVEL_ERROR,  "no AR for sl=%d ss=%d\n",
                          pAddr->Geo.Slot, pAddr->Geo.Subslot);
        PnpbSetLastError (PNIO_ERR_IOD_NO_AR);
        return (PNIO_FALSE);
    }



    iface_real_cfg_ptr = &pPnpbIf->real_cfg[Ind];
    iface_real_cfg_ptr->pral.diag_tag = UserHndl;

    if ( iface_real_cfg_ptr->pral.state == PNDV_IFACE_SERVICE_IDLE )
    {
        iface_real_cfg_ptr->pral.state = PNDV_IFACE_SERVICE_NEW;

        PNPB_COPY_BYTE(&iface_real_cfg_ptr->pral.info.data[0],
                       pData,
                       DataLen);

        iface_real_cfg_ptr->pral.info.usi    = UserStructIdent;
        iface_real_cfg_ptr->pral.info.data_length = DataLen;

        // *** set service ****
        cmd = PNDV_EV_TO_PNDV_PRAL;

        // *** trigger PNDV to serve this request ***
        {
            ProcessAlarmProcessing = PNIO_TRUE;
            pnpb_write_event_to_pndv (cmd, 0, (PNIO_UINT16)Ind, (PNIO_VOID*)UserHndl);
            PNPB_TRIGGER_PNDV ();
        }
    }
    else
    {
        return (PNIO_FALSE);
    }
    return (PNIO_TRUE);
}


// *-----------------------------------------------------------------
// * pnpb_status_alarm_send (..)
// *
// *
// *-----------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32  pnpb_status_alarm_send
                        (PNIO_UINT32    DevHndl,                // device handle
                         PNIO_UINT32    Api,                    // application process identifier
                         PNIO_DEV_ADDR  *pAddr,                 // geographical or logical address
                         PNIO_UINT8     *pData,                 // AlarmItem.Data
                         PNIO_UINT32    DataLen,                // length of AlarmItem.Data
                         PNIO_UINT16    UserStructIdent,        // AlarmItem.UserStructureIdentifier, s. IEC61158-6
                         PNIO_UINT32    UserHndl)               // user defined handle
{
    PNDV_REAL_CFG_T* iface_real_cfg_ptr;
    PNIO_UINT8       cmd = 0;
    PNIO_UINT32      Ind = 0;
    PNIO_EXP_SUB*    pSubExp = NULL;
    PNIO_UINT32       ArInd = 0;
    LSA_UNUSED_ARG (DevHndl);

        // *** get entity index in real_cfg ***
    Ind = PnpbRealCfgGetEntityInd (Api,
                                   pAddr->Geo.Slot,
                                   pAddr->Geo.Subslot);

    if (Ind == PNPB_INVALID_INDEX)
    {
        PNPB_API_TRACE_03(LSA_TRACE_LEVEL_ERROR,  "invalid alarm location api%d slot%d subsl%d",
                          Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        PnpbSetLastError (PNIO_ERR_IOD_INVALID_SUBSLOT);
        return (PNIO_FALSE);
    }
    if (DataLen > PNDV_AL_STAL_INFO_LEN)
    {
        PNPB_API_TRACE_02(LSA_TRACE_LEVEL_ERROR,  "alarm data size too big %d (%d)",
                          DataLen, PNDV_AL_STAL_INFO_LEN);
        PnpbSetLastError (PNIO_ERR_IOD_INVALID_BUFLEN);
        return (PNIO_FALSE);
    }

    if (StatusAlarmProcessing == PNIO_TRUE)
    {
        PNPB_SYS_TRACE_03(LSA_TRACE_LEVEL_ERROR,  "error: new alarm (sl%d ss%d ch%d hnd%d)before previous has been confirmed",
                       pAddr->Geo.Slot, pAddr->Geo.Subslot, UserHndl);
        PnpbSetLastError (PNIO_ERR_IOD_ALARM_SEQUENCE);
        return (PNIO_NOT_OK);
    }

    PNPB_API_TRACE_04(LSA_TRACE_LEVEL_NOTE_HIGH,  "send statusAlarm api%d slot%d subsl%d usi0x%x",
                      Api, pAddr->Geo.Slot, pAddr->Geo.Subslot, UserStructIdent);

    // ***** check if one of the AR's is owner of the specified subslot
    pSubExp = pnpb_sub_exp_allAr_getp (&ArInd, Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
    if (pSubExp == NULL)
    {
        PNPB_API_TRACE_02(LSA_TRACE_LEVEL_ERROR,  "no AR for sl=%d ss=%d\n",
                          pAddr->Geo.Slot, pAddr->Geo.Subslot);
        PnpbSetLastError (PNIO_ERR_IOD_NO_AR);
        return (PNIO_FALSE);
    }



    iface_real_cfg_ptr = &pPnpbIf->real_cfg[Ind];

    if ( iface_real_cfg_ptr->stal.state == PNDV_IFACE_SERVICE_IDLE )
    {
        iface_real_cfg_ptr->stal.state = PNDV_IFACE_SERVICE_NEW;

        PNPB_COPY_BYTE(&iface_real_cfg_ptr->stal.info.data[0],
                       pData,
                       DataLen);

        iface_real_cfg_ptr->stal.info.usi    = UserStructIdent;


        // *** set service ****
        cmd = PNDV_EV_TO_PNDV_STAL;

        // *** trigger PNDV to serve this request ***
        {
            StatusAlarmProcessing = PNIO_TRUE;
            pnpb_write_event_to_pndv (cmd, 0, (PNIO_UINT16)Ind, (PNIO_VOID*)UserHndl);
            PNPB_TRIGGER_PNDV ();
        }
    }
    else
    {
        return (PNIO_FALSE);
    }
    return (PNIO_TRUE);
}


// *-----------------------------------------------------------------
// * pnpb_ret_of_sub_alarm_send (..)
// *
// *
// *-----------------------------------------------------------------
PNIO_UINT32  pnpb_ret_of_sub_alarm_send
						(PNIO_UINT32	DevHndl,		// device handle
                         PNIO_UINT32    Api,            // application process identifier
						 PNIO_DEV_ADDR  *pAddr,			// geographical or logical address
				 		 PNIO_UINT32	 UserHndl) 		// user defined handle
{
    PNDV_REAL_CFG_T* iface_real_cfg_ptr;
    PNIO_UINT8       cmd = 0;
    PNIO_UINT32      Ind;
    PNIO_EXP_SUB*    pSubExp;
    PNIO_UINT32      ArInd;
    LSA_UNUSED_ARG (DevHndl);

    PNPB_API_TRACE_03(LSA_TRACE_LEVEL_NOTE_HIGH,  "send RetofSub alarm api%d slot%d subsl%d \n",
                      Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);

        // *** get entity index in real_cfg ***
    Ind = PnpbRealCfgGetEntityInd (Api,
                                   pAddr->Geo.Slot,
                                   pAddr->Geo.Subslot);

    if (Ind == PNPB_INVALID_INDEX)
    {
       	PNPB_API_TRACE_03(LSA_TRACE_LEVEL_ERROR,  "invalid alarm location api%d slot%d subsl%d",
                          Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        return (PNIO_FALSE);
    }

    if (RosAlarmProcessing == PNIO_TRUE)
    {
        PNPB_SYS_TRACE_03(LSA_TRACE_LEVEL_ERROR,  "error: new ROS alarm (sl%d ss%d hnd%d)before previous has been confirmed",
                       pAddr->Geo.Slot, pAddr->Geo.Subslot, UserHndl);
        PnpbSetLastError (PNIO_ERR_IOD_ALARM_SEQUENCE);
        return (PNIO_NOT_OK);
    }

    // ***** check if one of the AR's is owner of the specified subslot
    pSubExp = pnpb_sub_exp_allAr_getp (&ArInd, Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
    if (pSubExp == NULL)
    {
        PNPB_API_TRACE_02(LSA_TRACE_LEVEL_ERROR,  "no AR for sl=%d ss=%d\n",
                          pAddr->Geo.Slot, pAddr->Geo.Subslot);
        PnpbSetLastError (PNIO_ERR_IOD_NO_AR);
        return (PNIO_FALSE);
    }

    iface_real_cfg_ptr = &pPnpbIf->real_cfg[Ind];

    if ( iface_real_cfg_ptr->ros.state == PNDV_IFACE_SERVICE_IDLE )
    {
        iface_real_cfg_ptr->ros.state = PNDV_IFACE_SERVICE_NEW;

        // *** set service ****
        cmd = PNDV_EV_TO_PNDV_ROS_AL;

        // *** trigger PNDV to serve this request ***
        {
            RosAlarmProcessing = PNIO_TRUE;
            pnpb_write_event_to_pndv (cmd, 0, (PNIO_UINT16)Ind, (PNIO_VOID*)UserHndl);
            PNPB_TRIGGER_PNDV ();
        }
    }
    else
    {
        return (PNIO_FALSE);
    }
    return (PNIO_TRUE);
}



// *-----------------------------------------------------------------
// * pnpb_upload_retrieval_alarm_send (..)
// *
// *
// *-----------------------------------------------------------------
PNIO_UINT32  pnpb_upload_retrieval_alarm_send
                        (PNIO_UINT32    DevHndl,       // device handle
                         PNIO_UINT32    Api,           // application process identifier
                         PNIO_DEV_ADDR  *pAddr,        // geographical or logical address
                         PNIO_UINT8     *pData,        // AlarmItem.Data
                         PNIO_UINT32    DataLen,       // length of AlarmItem.Data
                         PNIO_UINT32    UsrHndl)       // user defined handle
{
    PNDV_REAL_CFG_T* iface_real_cfg_ptr;
    PNIO_UINT8       cmd = 0;
    PNIO_UINT32      Ind;
    PNIO_EXP_SUB*    pSubExp;
    PNIO_UINT32      ArInd;
    LSA_UNUSED_ARG (DevHndl);

    PNPB_API_TRACE_03(LSA_TRACE_LEVEL_NOTE_HIGH,  "send Retrieval alarm api%d slot%d subsl%d \n",
                      Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);

        // *** get entity index in real_cfg ***
    Ind = PnpbRealCfgGetEntityInd (Api,
                                   pAddr->Geo.Slot,
                                   pAddr->Geo.Subslot);

    if (Ind == PNPB_INVALID_INDEX)
    {
        PNPB_API_TRACE_03(LSA_TRACE_LEVEL_ERROR,  "invalid alarm location api%d slot%d subsl%d",
                          Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
        return (PNIO_FALSE);
    }
    if (DataLen > PNDV_AL_URAL_INFO_LEN)
    {
        PNPB_API_TRACE_02(LSA_TRACE_LEVEL_ERROR,  "alarm data size too big %d (%d)",
                          DataLen, PNDV_AL_URAL_INFO_LEN);
        PnpbSetLastError (PNIO_ERR_IOD_INVALID_BUFLEN);
        return (PNIO_FALSE);
    }
    if (UplRetrAlarmProcessing == PNIO_TRUE)
    {
        PNPB_SYS_TRACE_03(LSA_TRACE_LEVEL_ERROR,  "error: new UplRetr alarm (sl%d ss%d hnd%d)before previous has been confirmed",
                       pAddr->Geo.Slot, pAddr->Geo.Subslot, UsrHndl);
        PnpbSetLastError (PNIO_ERR_IOD_ALARM_SEQUENCE);
        return (PNIO_NOT_OK);
    }

    // ***** check if one of the AR's is owner of the specified subslot
    pSubExp = pnpb_sub_exp_allAr_getp (&ArInd, Api, pAddr->Geo.Slot, pAddr->Geo.Subslot);
    if (pSubExp == NULL)
    {
        PNPB_API_TRACE_02(LSA_TRACE_LEVEL_ERROR,  "no AR for sl=%d ss=%d\n",
                          pAddr->Geo.Slot, pAddr->Geo.Subslot);
        PnpbSetLastError (PNIO_ERR_IOD_NO_AR);
        return (PNIO_FALSE);
    }


    iface_real_cfg_ptr = &pPnpbIf->real_cfg[Ind];

    if ( iface_real_cfg_ptr->ural.state == PNDV_IFACE_SERVICE_IDLE )
    {
        iface_real_cfg_ptr->ural.state = PNDV_IFACE_SERVICE_NEW;

        // *** set service ****
        cmd = PNDV_EV_TO_PNDV_URAL;

        iface_real_cfg_ptr->ural.info.usi = PNDV_AL_USI_IPARAMETER;
        PNPB_COPY_BYTE(&iface_real_cfg_ptr->ural.info.data[0],
                       pData,
                       DataLen);

        // *** trigger PNDV to serve this request ***
        {
            UplRetrAlarmProcessing = PNIO_TRUE;
            pnpb_write_event_to_pndv (cmd, 0, (PNIO_UINT16)Ind, (PNIO_VOID*)UsrHndl);
            PNPB_TRIGGER_PNDV ();
        }
    }
    else
    {
        return (PNIO_FALSE);
    }
    return (PNIO_TRUE);
}



/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
