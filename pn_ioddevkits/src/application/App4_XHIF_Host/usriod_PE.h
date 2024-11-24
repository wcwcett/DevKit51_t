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
/*  F i l e               &F: usriod_PE.h                               :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  header file to usriod_PE.c.                                              */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/

#ifndef _USR_IOD_PE_H
#define _USR_IOD_PE_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

#include "pnpb_lib.h"

    #ifndef PNIO_FLOAT
        #define PNIO_FLOAT  float
    #endif

    // *---------------------------------------------
    // * definition of supported PE services
    // *---------------------------------------------
    #define NUMOF_PE_SERVICES   				7  // number of supported PE services (in this example)

    #define PE_SRV_START_PAUSE  				    0x01	//modifier 0x00
    #define PE_SRV_END_PAUSE    			    	0x02	//modifier 0x00
    #define PE_SRV_QUERY_MODES  				    0x03	//modifier 0x01
	#define PE_SRV_GET_MODE						    0x03	//modifier 0x02
    #define PE_SRV_PEM_STATUS   				    0x04	//modifier 0x00
    #define PE_SRV_IDENTIFY     				    0x05	//modifier 0x00
	#define PE_GET_MEASUREMENT_LIST				    0x10	//modifier 0x01, 0x03
	#define PE_GET_MEASUREMENT_VALUES			    0x10	//modifier 0x02, 0x04
	#define PE_START_PAUSE_WITH_TIME_RESP		    0x01	//modifier 0x01
	#define PE_PEM_STATUS_WITH_CTTO				    0x04	//modifier 0x01
	#define PE_QUERY_VERSION					    0x06	//modifier 0x00
	/*modifiers*/
	#define PE_MOD_SRV_START_PAUSE  			    0x00
	#define PE_MOD_SRV_END_PAUSE    			    0x00
	#define PE_MOD_SRV_QUERY_MODES  			    0x01
	#define PE_MOD_SRV_GET_MODE					    0x02
	#define PE_MOD_SRV_PEM_STATUS   			    0x00
	#define PE_MOD_SRV_IDENTIFY     			    0x00
    #define PE_MOD_GET_MEASUREMENT_LIST             0x01
    #define PE_MOD_GET_MEASUREMENT_VALUES           0x02
	#define PE_MOD_GET_MEASUREMENT_LIST_OBJ_NUM		0x03
	#define PE_MOD_GET_MEASUREMENT_VALUES_OBJ_NUM	0x04
	#define PE_MOD_START_PAUSE_WITH_TIME_RESP	    0x01
	#define PE_MOD_PEM_STATUS_WITH_CTTO			    0x01
	#define PE_MOD_QUERY_VERSION				    0x00

    // *---------------------------------------------
    // * definition supported power modes of this device
    // *---------------------------------------------
    #define NUMOF_PE_MODES              3   // Number of supported PE power saving modes (in this example)

    #define PE_MODE_POWER_OFF           0   // ID of PE "power off" mode
    #define PE_MODE_PAUSE1              1   // ID of PE save mode 1 (we support here only two save mode, more is possible)
	#define PE_MODE_PAUSE2              2   // ID of PE save mode 2
    #define PE_MODE_READY_TO_OPERATE   0xff // ID of PE mode 'ready to operate'


    // *---------------------------------------------
    // * definition of PROFIenergy request header
    // *---------------------------------------------
    typedef  ATTR_PNIO_PACKED_PRE struct
    {
        PNIO_UINT8       ServReqId;				// Request-Header.service rquest ID
        PNIO_UINT8       ReqRef;			    // Request-Header.Request Reference
        PNIO_UINT8       Modifier;              // Service-Header-Request.Modifier
        PNIO_UINT8       DatStrucIdRq;          // Service-Header-Request.Data Structure Identifier RQ
    }  ATTR_PNIO_PACKED PE_REQ_HDR;

    // *---------------------------------------------
    // * definition of PROFIenergy response header
    // * and service header response
    // *---------------------------------------------
    typedef  ATTR_PNIO_PACKED_PRE struct
    {
        PNIO_UINT8       ServReqId;				// Response-Header.service rquest ID
        PNIO_UINT8       ReqRef;			    // Response-Header.Responset Reference
        PNIO_UINT8       Status;                // Service-Header-Response.Modifier
        PNIO_UINT8       DatStrucIdRsp;         // Service-Header-Response.Data Structure Identifier RQ
    }  ATTR_PNIO_PACKED PE_RSP_HDR;

    // *** define blockheader value ***
    #define BLOCKHDR_PE_RQ   0x0800
    #define BLOCKHDR_PE_RSP  0x0801

    // **return values for PE_RSP_HDR.Status **
    #define PE_RSP_STAT_READY_OK          1
    #define PE_RSP_STAT_READY_ERROR       2
    #define PE_RSP_STAT_DATA_INCOMPLETE   3


    // **error code return values **
    #define PE_RET_OK                              0x00
    #define PE_RET_INVALID_SERVICE_REQUEST_ID      0x01
    #define PE_RET_BAD_REQUEST_REFERENCE           0x02
    #define PE_RET_INVALID_MODIFIER                0x03
    #define PE_RET_INVALID_DATA_STRUCT_IDENT_RQ    0x04
    #define PE_RET_INVALID_DATA_STRUCT_IDENT_RS    0x05
    #define PE_RET_NO_PE_SAVING_MODE_SUPPORTED     0x06
    #define PE_RET_RESPONSE_TOO_LONG               0x07
    #define PE_RET_INVALID_BLOCK_HEADER            0x08
    
    // **specific error return values **
    #define PE_RET_NO_SUITABLE_ENERGY_SAVING_MODE       0x50
    #define PE_RET_TIME_IS_NOT_SUPPORTED                0x51
    #define PE_RET_IMPERMISSIBLE_PE_MODE_ID             0x52
    #define PE_RET_NO_SWITCH_TO_MODE                    0x53
    #define PE_RET_SERVICE_TEMPORARILY_NOT_AVAILABLE    0x54

    /*  Note about specific error return values : 
        Specific error codes are not fully implemented. Only PE_RET_IMPERMISSIBLE_PE_MODE_ID is implemented.
    */


    typedef struct PE_PROCESS_RESP
    {
    	PNIO_UINT32 	( *pe_method_ptr) (PNIO_UINT8 *, PNIO_UINT8 *, PNIO_UINT16);
        PNIO_UINT32 	pe_response;

        PNIO_UINT32		exp_blocklen;
        PNIO_UINT32		exp_blockver;
    } PE_PROCESS_RESP;


    // *---------------------------------------------
    // * definition of PROFIenergy header
    // *---------------------------------------------
    typedef ATTR_PNIO_PACKED_PRE struct
    {
       REC_IO_BLOCKHDR  BlockHdr;	              // see pnio spec, BlockHeader
       PE_REQ_HDR       ReqHdr;                   // see PROFIenergy spec, Request Header and Service Header Request
    }  ATTR_PNIO_PACKED PE_SUM_REQ_HDR;

    typedef ATTR_PNIO_PACKED_PRE struct
    {
       REC_IO_BLOCKHDR  BlockHdr;	              // see pnio spec, BlockHeader
       PE_RSP_HDR       RspHdr;                   // see PROFIenergy spec, Request Header and Service Header Request
    }  ATTR_PNIO_PACKED PE_SUM_RSP_HDR;



    // *---------------------------------------------
    // * command "start pause", serviceRequestID = 1
    // *---------------------------------------------
    typedef ATTR_PNIO_PACKED_PRE struct
    {
       PNIO_UINT32      PauseTime;                // see pnio spec, Order_ID, visible string
    }  ATTR_PNIO_PACKED PE_SDR_REQ_START_PAUSE;

    typedef ATTR_PNIO_PACKED_PRE struct
    {
       PNIO_UINT8       PE_ModeId;                // see pnio spec, Order_ID, visible string
       PNIO_UINT8       reserved;                 // see pnio spec, Order_ID, visible string
    }  ATTR_PNIO_PACKED PE_SDR_RSP_START_PAUSE;

    typedef ATTR_PNIO_PACKED_PRE struct
    {
       PNIO_UINT8       PE_ModeId;                // see pnio spec, Order_ID, visible string
       PNIO_UINT8       reserved;                 // see pnio spec, Order_ID, visible string
       PNIO_UINT32		time_to_dest;			  // see pnio spec, Order_ID, visible string
       PNIO_UINT32		time_to_operate;		  // see pnio spec, Order_ID, visible string
       PNIO_UINT32		min_len_to_stay;		  // see pnio spec, Order_ID, visible string
    }  ATTR_PNIO_PACKED PE_SDR_RSP_START_PAUSE_WITH_TIME_RESP;


    // *---------------------------------------------
    // * command "end pause", serviceRequestID = 2
    // *---------------------------------------------

    typedef ATTR_PNIO_PACKED_PRE struct
    {
       PNIO_UINT32      TimeToOperate;            // see pnio spec, Order_ID, visible string
    }  ATTR_PNIO_PACKED PE_SDR_RSP_END_PAUSE;

    // *---------------------------------------------
    // * command "query modes", serviceRequestID = 3
    // * NOTE: structure PE_SDR_RSP_QUERY_MODES must be
    // *       word aligned, if necessary add padding
    // *       byte (see below)
    // *---------------------------------------------

    typedef ATTR_PNIO_PACKED_PRE struct
    {
        PNIO_UINT8       NumOfPeModeIDs;                   // nunber of supported PE modes
        #if ((NUMOF_PE_MODES % 2) != 0)                    // PeMode unpair --> PE_SDR_RSP_QUERY_MODES pair --> ok, no padding
            PNIO_UINT8       PeMode[NUMOF_PE_MODES];       // list of supported PE modes,
        #else
            PNIO_UINT8       PeMode[NUMOF_PE_MODES + 1];   // list of supported PE modes + padding byte,
        #endif
    }  ATTR_PNIO_PACKED PE_SDR_RSP_QUERY_MODES;


    // *---------------------------------------------
    // * command "query modes - get mode",
    // * serviceRequestID = 3
    // *---------------------------------------------
    typedef ATTR_PNIO_PACKED_PRE struct
    {
       PNIO_UINT8       PeModeId;                // PE_Mode_ID
       PNIO_UINT8       reserved;
    }  ATTR_PNIO_PACKED PE_SDR_REQ_GET_MODE;

    typedef ATTR_PNIO_PACKED_PRE struct
    {
       PNIO_UINT8       PeModeId;                 // requested PE mode
       PNIO_UINT8       PeModeAttr;               // 0: only static values, 1: dyn. values
       PNIO_UINT32      TimeMinPause;
       PNIO_UINT32      TimeToPause;
       PNIO_UINT32      RegularTimeToOperate;
       PNIO_UINT32      TimeMinLenOfStay;
       PNIO_UINT32      TimeMaxLenOfStay;
       PNIO_FLOAT       ModePowerConsumption;     // Mode spec. power consumption
       PNIO_FLOAT       EnergyConsumpToPause;     // mode spec. energy consumption to destination mode
       PNIO_FLOAT       EnergyConsumpToOperate;   // mode spec. energy consumption to operate

    }  ATTR_PNIO_PACKED PE_SDR_RSP_GET_MODE;

    // *---------------------------------------------
    // * command "PEM status", serviceRequestID = 4
    // *---------------------------------------------
    typedef ATTR_PNIO_PACKED_PRE struct
    {
       PNIO_UINT8       PeModeIdSrc;              // number of supported PE commands
       PNIO_UINT8       PeModeIdDest;             // PE mode ID source
       PNIO_UINT32      TimeToOperate;            // PE mode ID destination
       PNIO_UINT32      RemainingTimeToDest;      // time to operate
       PNIO_FLOAT       ModePowerConsumption;     // Mode spec. power consumption
       PNIO_FLOAT       EnergyConsumpToDest;      // mode spec. energy consumption to destination mode
       PNIO_FLOAT       EnergyConsumpToOperate;   // mode spec. energy consumption to operate
    }  ATTR_PNIO_PACKED PE_SDR_RSP_PEM_STATUS;

    typedef ATTR_PNIO_PACKED_PRE struct
    {
       PNIO_UINT8       PeModeIdSrc;              // number of supported PE commands
       PNIO_UINT8       PeModeIdDest;             // PE mode ID source
       PNIO_UINT32      RegularTimeToOperate;
       PNIO_UINT32      CurrentTimeToOperate;
       PNIO_UINT32      CurrentTimeToDest;
       PNIO_FLOAT       ModePowerConsumption;     // Mode spec. power consumption
       PNIO_FLOAT       EnergyConsumpToDest;      // mode spec. energy consumption to destination mode
       PNIO_FLOAT       EnergyConsumpToOperate;   // mode spec. energy consumption to operate
    }  ATTR_PNIO_PACKED PE_SDR_RSP_PEM_CTTO_STATUS;



    // *---------------------------------------------
    // * command "PE identify", serviceRequestID = 5
    // *---------------------------------------------
    #define NUMOF_PE_COMMANDS   7   // Number of supported PE commands (PE services)

    typedef ATTR_PNIO_PACKED_PRE struct
    {
       PNIO_UINT8       NumOfCmd;                 // number of supported PE commands
       PNIO_UINT8       Cmd[NUMOF_PE_COMMANDS];                 //
    }  ATTR_PNIO_PACKED PE_SDR_RSP_IDENTIFY;


    // *---------------------------------------------
    // * command "Query_version", serviceRequestID = 6
    // *---------------------------------------------

    typedef ATTR_PNIO_PACKED_PRE struct
	{
	   PNIO_UINT8       major;
	   PNIO_UINT8       minor;
	}  ATTR_PNIO_PACKED PE_VERSION;

	// *---------------------------------------------
	// * commands for measurement, serviceRequestID = 10
	// *---------------------------------------------
	#define PE_NO_MEASURED_ITEMS	3

    typedef ATTR_PNIO_PACKED_PRE struct
    {
       PNIO_UINT8       Count;
       PNIO_UINT8       reserved;
       struct ATTR_PNIO_PACKED
       {
           PNIO_UINT16  Measurement_ID;
           PNIO_UINT8   Accuracy_domain;
           PNIO_UINT8   Accuracy_class;
           PNIO_FLOAT   Range;
       }item[PE_NO_MEASURED_ITEMS];

    }  ATTR_PNIO_PACKED PE_GET_MEAS_LIST;


    typedef ATTR_PNIO_PACKED_PRE struct
    {
       PNIO_UINT8       Count;
       PNIO_UINT8       reserved;
       struct ATTR_PNIO_PACKED
       {
           PNIO_UINT16  Length_of_Structure;
           PNIO_UINT8   Measurement_Data_Structure_ID;
           PNIO_UINT16  Measurement_ID;
           PNIO_UINT8   Status_of_Measurement_Value;
           PNIO_FLOAT   Measurement_Value;
       }item[PE_NO_MEASURED_ITEMS];

    }  ATTR_PNIO_PACKED PE_GET_MEAS_VALUES;


	typedef ATTR_PNIO_PACKED_PRE struct
	{
	   PNIO_UINT8       Count;
	   PNIO_UINT8       reserved;
	   struct ATTR_PNIO_PACKED
	   {
		   PNIO_UINT16	Object_number;
		   PNIO_UINT16	Measurement_ID;
		   PNIO_UINT8	Accuracy_domain;
		   PNIO_UINT8	Accuracy_class;
		   PNIO_FLOAT	Range;
	   }item[PE_NO_MEASURED_ITEMS];

	}  ATTR_PNIO_PACKED PE_GET_MEAS_LIST_OBJ_NUM;


    typedef ATTR_PNIO_PACKED_PRE struct
	{
	   PNIO_UINT8       Count;
	   PNIO_UINT8       reserved;
	   struct ATTR_PNIO_PACKED
	   {
		   PNIO_UINT16	Length_of_Structure;
		   PNIO_UINT8	Measurement_Data_Structure_ID;
		   PNIO_UINT8	Status_of_Measurement_Value;
		   PNIO_UINT16	Object_number;
		   PNIO_UINT16	Measurement_ID;
		   PNIO_FLOAT	Measurement_Value;
	   }item[PE_NO_MEASURED_ITEMS];

	}  ATTR_PNIO_PACKED PE_GET_MEAS_VALUES_OBJ_NUM;


    typedef ATTR_PNIO_PACKED_PRE struct
	{
	   PNIO_UINT8       Count;
	   PNIO_UINT8       reserved;
	   struct ATTR_PNIO_PACKED
	   {
		   PNIO_UINT16	Object_number;
		   PNIO_UINT16	Measurement_ID;
	   }item[PE_NO_MEASURED_ITEMS];

	}  ATTR_PNIO_PACKED PE_GET_MEAS_VALUES_RQ;


    // *---------------------------------------------
    // *  error: unknown serviceRequestID
    // *---------------------------------------------
    typedef ATTR_PNIO_PACKED_PRE struct
    {
       PNIO_UINT8       ErrorCode;           // error code
       PNIO_UINT8       reserved;
    }  ATTR_PNIO_PACKED PE_SDR_RSP_ERROR;


// *---------------------------------------------
// * PE service functions
// *---------------------------------------------

PNIO_UINT32  PROFIenergy_RequestHandler
		(
            PNIO_DEV_ADDR       *pAddr,         // [in] location (slot, subslot)
            PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			PNIO_ERR_STAT		*pPnioState,	// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    PNIO_UINT16         ArNum         /* [in] number of AR initiating request*/
		);

PNIO_UINT32  PROFIenergy_ResponseHandler
		(
            PNIO_DEV_ADDR       *pAddr,         // [in] location (slot, subslot)
			PNIO_UINT32			*pBufLen,		// [in, out] in: length to read, out: length, read by user
			PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			PNIO_ERR_STAT		*pPnioState,	// [out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
            PNIO_UINT16         ArNum         /* [in] number of AR initiating request*/
		);


#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
