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
/*  F i l e               &F: pnpb_lib_acyc.h                           :F&  */
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

#ifndef HOST_SRC_PNPB_LIB_ACYC_H_
#define HOST_SRC_PNPB_LIB_ACYC_H_

extern PNIO_UINT16 InDatLen  [PNPB_XHIF_NUMOF_SLOTS][PNPB_XHIF_NUMOF_SUBSLOTS+1];
extern PNIO_UINT16 OutDatLen [PNPB_XHIF_NUMOF_SLOTS][PNPB_XHIF_NUMOF_SUBSLOTS+1];

extern PNIO_UINT8  LastPulledSlot;
extern PNIO_UINT8  LastPulledSubslot;

/* Structures from Ertec Devkit */
typedef enum
{
    PNPB_AR_TYPE_SINGLE        = 0x0001, /* IOCARSingle */
    PNPB_AR_TYPE_SUPERVISOR    = 0x0006, /* IOSAR, The supervisor AR is a special form of the IOCARSingle */
    PNPB_AR_TYPE_SINGLE_RTC3   = 0x0010, /* IOCARSingle using RT_CLASS_3 */
    PNPB_AR_TYPE_SINGLE_SYSRED = 0x0020, /* IOCARSR, The SR AR is a special form of the IOCARSingle indicating system redundancy or configure in run usage */
    PNPB_AR_TYPE_RESERVED /* all other types are reserved for future use */
} PNPB_AR_TYPE;

/* Structures for parameters transferred by memory interface*/
typedef struct PNIO_CONNECT_IND_PRM{
    PNPB_AR_TYPE ArType;
    PNIO_UINT32 ArNum;
    PNIO_UINT16 SendClock;
    PNIO_UINT16 RedRatioIocrIn;
    PNIO_UINT16 RedRatioIocrOut;
}PNIO_CONNECT_IND_PRM;

typedef struct PNIO_OWNERSHIP_IND_PRM
{
    PNIO_UINT32 ArNum;      /* AR number 1....NumOfAR */
    PNIO_EXP OwnSub;      /* Expected configuration in ownership indication */
}PNIO_OWNERSHIP_IND_PRM;

typedef struct PNIO_PRM_END_IND_PRM
{
    PNIO_UINT16 ArNum;
    PNIO_UINT16 SessionKey;
    PNIO_UINT32 Api;
    PNIO_UINT16 SlotNum;
    PNIO_UINT16 SubslotNum;
    PNIO_BOOL   MoreFollows;
}PNIO_PRM_END_IND_PRM;

typedef struct PNIO_INPUT_UPDATE_IND_PRM
{
    PNIO_UINT16 ArNum;
    PNIO_INP_UPDATE_STATE InpUpdState;
    PNIO_UINT32 Apdu;
}PNIO_INPUT_UPDATE_IND_PRM;

typedef struct PNIO_AR_INDATA_IND_PRM
{
    PNIO_UINT16 ArNum;
    PNIO_UINT16 SessionKey;
}PNIO_AR_INDATA_IND_PRM;

typedef struct PNIO_AR_DISCONNECT_IND_PRM
{
    PNIO_UINT16 ArNum;
    PNIO_UINT16 SessionKey;
    PNIO_AR_REASON ReasonCode;
}PNIO_AR_DISCONNECT_IND_PRM;

typedef struct PNIO_ARFSU_IND_PRM
{
    PNIO_UINT8 ARFSU_enabled;
    PNIO_UINT8 ARFSU_changed;
}PNIO_ARFSU_IND_PRM;

typedef struct PNIO_ASYNC_DONE_PRM
{
    PNIO_UINT32     ArNum;
    PNIO_ALARM_TYPE AlarmType;
    PNIO_UINT32     Api;
    PNIO_DEV_ADDR   Addr;
    PNIO_UINT32     Status;
    PNIO_UINT16     Diag_tag;
}PNIO_ASYNC_DONE_PRM;

typedef struct PNIO_RECORD_READ_PRM
{
    PNIO_UINT32         Api;
    PNIO_UINT16         ArNum;
    PNIO_UINT16         SessionKey;
    PNIO_UINT32         SequenceNum;
    PNIO_DEV_ADDR       Addr;
    PNIO_UINT32         RecordIndex;
    PNIO_UINT32         BufLen;
    PNIO_UINT32         pBufLen;
    PNIO_UINT32         pPnioState;
    PNIO_UINT32         pBuffer;
}PNIO_RECORD_READ_PRM;

typedef struct PNIO_RECORD_WRITE_PRM
{
    PNIO_UINT32         Api;
    PNIO_UINT16         ArNum;
    PNIO_UINT16         SessionKey;
    PNIO_UINT32         SequenceNum;
    PNIO_DEV_ADDR       Addr;
    PNIO_UINT32         RecordIndex;
    PNIO_UINT32         BufLen;
    PNIO_UINT32         pBufLen;
    PNIO_UINT32         pPnioState;
    PNIO_UINT32         pBuffer;
}PNIO_RECORD_WRITE_PRM;

typedef struct PNIO_SAVE_STATION_NAME_PRM
{
    PNIO_UINT16 NameLength;
    PNIO_UINT8 Remanent;
    PNIO_INT8 StationName[];
}PNIO_SAVE_STATION_NAME_PRM;

typedef struct PNIO_SAVE_IP_ADDR_PRM
{
    PNIO_UINT8 NewIpAddr[4];
    PNIO_UINT8 SubnetMask[4];
    PNIO_UINT8 DefRouterAddr[4];
    PNIO_UINT8 Remanent;
}PNIO_SAVE_IP_ADDR_PRM;

typedef struct PNIO_REPORT_NEW_IP_ADDR_PRM
{
    PNIO_UINT8 NewIpAddr[4];
    PNIO_UINT8 SubnetMask[4];
    PNIO_UINT8 DefRouterAddr[4];
}PNIO_REPORT_NEW_IP_ADDR_PRM;

typedef struct PNIO_RESET_FACTORY_SETTINGS_PRM
{
    PNIO_UINT32 RtfOption;
}PNIO_RESET_FACTORY_SETTINGS_PRM;

typedef enum PNIO_NEW_DEV_ADDR_COMMAND
{
    PNIO_STORE_NEW_MAC = 1,
    PNIO_STORE_NEW_IP = 2,
    PNIO_STORE_NEW_NAME = 3
} PNIO_NEW_DEV_ADDR_COMMAND;

typedef struct PNIO_RESULT_NEW_DEV_ADDR_PRM
{
    PNIO_UINT8 command; /* retype PNIO_NEW_DEV_ADDR_COMMAND */
    PNIO_UINT8 result;
}PNIO_RESULT_NEW_DEV_ADDR_PRM;

typedef struct PNIO_SEND_INFO_PRM
{
    PNIO_UINT8 ip_suite[12];
    PNIO_UINT8 mac_addr[6];
} PNIO_SEND_INFO_PRM;

typedef struct PNIO_STARTUP_DONE_PRM
{
    PNIO_UINT32 State;
    PNIO_FW_VERSION Version;
	PNIO_UINT32 Fw_DAP;
} PNIO_STARTUP_DONE_PRM;

/*
 * Params of calls from BBB to ERTEC
 */

/*
 * User alarm params
 */

typedef struct PNIO_STD_DIAG_CHANNEL_PRM
{
	PNIO_DEV_ADDR Addr;				// geographical or logical address
	PNIO_UINT16 ChannelNum;	 		// channel number
	PNIO_UINT16 ErrorNum;	 		// error number, see PNIO specification coding of "ChannelErrorType"
	PNIO_UINT16 ChanDir;	 		// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
	PNIO_UINT16 ChanTyp;	 		// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
} PNIO_STD_DIAG_CHANNEL_PRM;

typedef struct PNIO_DIAG_CHANNEL_PRM
{
	PNIO_STD_DIAG_CHANNEL_PRM StdParams; 	// Standard parameters for channel diagnostics
	PNIO_BOOL MaintReq;	 					// maintenance required
	PNIO_BOOL MaintDem;	 					// maintenance demanded
	PNIO_UINT16 DiagTag;    				// user defined diag tag, used as reference
} PNIO_DIAG_CHANNEL_PRM;

typedef struct PNIO_DIAG_CHANNEL_REMOVE_PRM
{
	PNIO_STD_DIAG_CHANNEL_PRM StdParams; 	// Standard parameters for channel diagnostics
	PNIO_UINT16 DiagTag;    				// user defined diag tag, used as reference
	PNIO_UINT16 AlarmState;					// DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
} PNIO_DIAG_CHANNEL_REMOVE_PRM;

typedef struct PNIO_EXT_DIAG_CHANNEL_PRM
{
	PNIO_STD_DIAG_CHANNEL_PRM StdParams; 	// Standard parameters for channel diagnostics
	PNIO_UINT16 ExtChannelErrType;	 		// ExtChannelErrorType (see IEC61158)
	PNIO_UINT16 ExtChannelAddValue;	 		// ExtChannelAddValue (see IEC61158)
	PNIO_BOOL MaintReq;	 					// maintenance required
	PNIO_BOOL MaintDem;	 					// maintenance demanded
	PNIO_UINT16 DiagTag;    				// user defined diag tag, used as reference
} PNIO_EXT_DIAG_CHANNEL_PRM;

typedef struct PNIO_EXT_DIAG_CHANNEL_REMOVE_PRM
{
	PNIO_STD_DIAG_CHANNEL_PRM StdParams; 	// Standard parameters for channel diagnostics
	PNIO_UINT16 ExtChannelErrType;	 		// ExtChannelErrorType (see IEC61158)
	PNIO_UINT16 ExtChannelAddValue;	 		// ExtChannelAddValue (see IEC61158)
	PNIO_UINT16 DiagTag;    				// user defined diag tag, used as reference
	PNIO_UINT16 AlarmState;					// DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
} PNIO_EXT_DIAG_CHANNEL_REMOVE_PRM;

typedef struct PNIO_STD_DIAG_GENERIC_PRM
{
	PNIO_DEV_ADDR Addr;				// geographical or logical address
	PNIO_UINT16 ChannelNum;	 		// channel number
	PNIO_UINT16 ChanDir;	 		// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
	PNIO_UINT16 ChanTyp;	 		// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
	PNIO_UINT16 DiagTag;    		// user defined diag tag, used as reference
	PNIO_UINT16 UserStructIdent;   	// manufacturer specific, 0...0x7fff, see IEC 61158
} PNIO_STD_DIAG_GENERIC_PRM;

typedef struct PNIO_DIAG_GENERIC_PRM
{
	PNIO_STD_DIAG_GENERIC_PRM StdParams; 	// Standard parameters for generic diagnostics
	PNIO_BOOL MaintReq;	 					// maintenance required
	PNIO_BOOL MaintDem;	 					// maintenance demanded
	PNIO_UINT32 InfoDataLen;       			// length of generic diagnostic data
	PNIO_UINT8 InfoData[];					// user defined generic diagnostic data
} PNIO_DIAG_GENERIC_PRM;

typedef struct PNIO_DIAG_GENERIC_REMOVE_PRM
{
	PNIO_STD_DIAG_GENERIC_PRM StdParams; 	// Standard parameters for generic diagnostics
} PNIO_DIAG_GENERIC_REMOVE_PRM;

typedef struct PNIO_PROCESS_ALARM_SET_PRM
{
	PNIO_DEV_ADDR Addr;						// geographical or logical address
	PNIO_UINT16	UserStructIdent;				// AlarmItem.UserStructureIdentifier, s. IEC61158-6
	PNIO_UINT32	UserHndl;						// user defined handle
	PNIO_UINT32	DataLen;					// length of AlarmItem.Data
	PNIO_UINT8 Data[];						// AlarmItem.Data
} PNIO_PROCESS_ALARM_SET_PRM;

typedef struct PNIO_STATUS_ALARM_SET_PRM
{
	PNIO_DEV_ADDR Addr;						// geographical or logical address
	PNIO_UINT16	UserStructIdent;				// AlarmItem.UserStructureIdentifier, s. IEC61158-6
	PNIO_UINT32	UserHndl;						// user defined handle
	PNIO_UINT32 Api;						// application process identifier
	PNIO_UINT32	DataLen;					// length of AlarmItem.Data
	PNIO_UINT8 Data[];						// AlarmItem.Data
} PNIO_STATUS_ALARM_SET_PRM;

typedef struct PNIO_RET_OF_SUB_ALARM_SEND_PRM
{
	PNIO_DEV_ADDR Addr;				// geographical or logical address
	PNIO_UINT32 UserHndl;	 		// user defined handle
} PNIO_RET_OF_SUB_ALARM_SEND_PRM;

typedef struct PNIO_UPLOAD_RETRIEVAL_ALARM_PRM
{
	PNIO_DEV_ADDR Addr;				// geographical or logical address
	PNIO_UINT32 UserHndl;	 		// user defined handle
	PNIO_UINT32	DataLen;			// length of AlarmItem.Data
	PNIO_UINT8 Data[];				// AlarmItem.Data
} PNIO_UPLOAD_RETRIEVAL_ALARM_PRM;

/*
 * Initialize, activate, deactivate, abort
 */

typedef struct PNIO_DEVICE_SETUP_PRM
{
    PNIO_UINT32 NumOfSublistEntries;
    PNIO_UINT32 NumOfIm0ListEntries;
    PNIO_UINT8  Data;
}PNIO_DEVICE_SETUP_PRM;

typedef struct PNIO_AR_ABORT_PRM
{
    PNIO_UINT32 ArNum;
} PNIO_AR_ABORT_PRM;

/*
 * Pull/plug submodules
 */

typedef struct PNIO_SUB_PLUG_LIST_PRM
{
	PNIO_UINT32 NumOfSubListEntries;	// number of entries in pIoSubList
	PNIO_UINT32 NumOfIm0ListEntries;	// number of entries in pIm0List
	PNIO_UINT8 Data[];					// First part of data array:
										// plugged submodules, including PDEV
										// Second part of data array:
										// list of IM0 data sets
} PNIO_SUB_PLUG_LIST_PRM;

typedef struct PNIO_SUB_PLUG_PRM
{
    PNIO_DEV_ADDR Addr;					// geographical or logical address
    PNIO_UINT32 ModIdent;				// module identifier  (see GSDML file)
    PNIO_UINT32 SubIdent;				// submodule identifier (see GSDML file)
    PNIO_UINT32 InputDataLen;	        // submodule input data length
    PNIO_UINT32 OutputDataLen;		    // submodule output data length
    PNIO_IM0_SUPP_ENUM Im0Support;		// for I&M0FilterData, see enum PNIO_IM0_SUPP_ENUM
    IM0_DATA Im0Dat;					// IM0 data
    PNIO_UINT8 IopsIniVal;     			// initial value for iops-input
} PNIO_SUB_PLUG_PRM;

typedef struct PNIO_SUB_PULL_PRM
{
    PNIO_DEV_ADDR Addr;					// geographical or logical address
} PNIO_SUB_PULL_PRM;

typedef struct PNIO_SUBSTVAL_OUT_READ_PRM
{
    PNIO_DEV_ADDR Addr;
    PNIO_UINT32 BufLen;
    PNIO_UINT32 BufAddr;
    PNIO_UINT32 SubstModeAddr;
    PNIO_UINT32 SubstActiveAddr;
} PNIO_SUBSTVAL_OUT_READ_PRM;

typedef struct PNIO_SUBSTVAL_OUT_READ_DONE_PRM
{
    PNIO_UINT32 BufLen;
    PNIO_UINT32 BufAddr;
    PNIO_UINT16 SubstMode;
    PNIO_UINT32 SubstModeAddr;
    PNIO_UINT16 SubstActive;
    PNIO_UINT32 SubstActiveAddr;
    PNIO_UINT8 BufData[];
} PNIO_SUBSTVAL_OUT_READ_DONE_PRM;

/*
 * Watchdog functionality
 */

typedef enum PNIO_WD_GRANITY
{
    PNIO_WD_100MS,
    PNIO_WD_10MS,
    PNIO_WD_1MS,
    PNIO_WD_100US
}PNIO_WD_GRANITY;

typedef struct PNIO_HW_WD_SET_PRM
{
    PNIO_UINT32 time;
    PNIO_WD_GRANITY granity;
} PNIO_HW_WD_SET_PRM;

typedef struct PNIO_HW_WD_COMMAND_PRM
{
    PNIO_UINT32 command;
} PNIO_HW_WD_COMMAND_PRM;

/*
 * Change device name , IP and MAC address
 */

typedef struct PNIO_STORE_NEW_MAC_PRM
{
    PNIO_UINT8 mac_addr[6];
} PNIO_STORE_NEW_MAC_PRM;

typedef struct PNIO_STORE_NEW_IP_PRM
{
    PNIO_UINT8 ip_suite[12];
} PNIO_STORE_NEW_IP_PRM;

typedef struct PNIO_STORE_NEW_DEV_NAME_PRM
{
    PNIO_UINT32 dev_len;
    PNIO_UINT8 dev_name[];
} PNIO_STORE_NEW_DEV_NAME_PRM;

/*
 * Trace functionality
 */

typedef enum PNIO_TRACE_COMMAND
{
    PNIO_TRACE_SAVE,
    PNIO_TRACE_RESTORE,
    PNIO_TRACE_PRINT_START,
    PNIO_TRACE_PRINT_NEXT
} PNIO_TRACE_COMMAND;

typedef struct PNIO_TRACE_COMMAND_PRM
{
    PNIO_TRACE_COMMAND command;
} PNIO_TRACE_COMMAND_PRM;

typedef enum PNIO_TRACE_SETTINGS_COMMAND
{
    PNIO_TRACE_ALL_SUBMODULES,
    PNIO_TRACE_SINGE_SUBMODULE
} PNIO_TRACE_SETTINGS_COMMAND;

typedef struct PNIO_TRACE_SETTINGS_PRM
{
    PNIO_TRACE_SETTINGS_COMMAND command;
    PNIO_UINT32 module;
    PNIO_UINT32 trc_level;
} PNIO_TRACE_SETTINGS_PRM;

typedef struct PNIO_TRACE_READY_PRM
{
	PNIO_UINT8 store;   	/* = 1 if trace should be stored and not printed */
    PNIO_UINT8 trace_end;   /* = 1 if trace is last one */
    PNIO_UINT32 len;
} PNIO_TRACE_READY_PRM;

/*
 * Non-volatile memory functionality
 */

typedef struct PNIO_NV_DATA_INIT_PRM
{
    PNIO_UINT32         useErtecNVMem;  /* 1 == use Ertec non-volatile memory */
    PNIO_UINT32         pBufLen;
    PNIO_UINT8          pData[];
} PNIO_NV_DATA_INIT_PRM;

typedef struct PNIO_NV_DATA_SYNC_PRM
{
    PNIO_UINT32         errOccured;  /* 1 == error occured */
    PNIO_UINT32         pBufLen;
    PNIO_UINT8          pData[];
} PNIO_NV_DATA_SYNC_PRM;

typedef struct PNIO_NV_DATA_FLASH_DONE_PRM
{
    PNIO_UINT32 Status;
    PNIO_UINT32 DatLen;
    PNIO_UINT32 nvDataType;
} PNIO_NV_DATA_FLASH_DONE_PRM;

typedef struct PNIO_NV_DATA_FLASH_DONE_PRM PNIO_NV_DATA_RESET_DONE_PRM;

typedef struct PNIO_NV_DATA_CLEAR_PRM
{
    PNIO_UINT32 RtfOption;
} PNIO_NV_DATA_CLEAR_PRM;

typedef struct PNIO_NV_DATA_STORE_PRM
{
    PNIO_UINT32         NvDataType;
    PNIO_UINT32         pBufLen;
    PNIO_UINT8          pData[];
} PNIO_NV_DATA_STORE_PRM;

typedef struct PNIO_NV_IM_DATA_STORE_PRM
{
    PNIO_UINT32         NvDataType;
    PNIO_UINT32         ModIdent;
    PNIO_UINT32         pBufLen;
    PNIO_UINT8          pData[];
} PNIO_NV_IM_DATA_STORE_PRM;

typedef struct PNIO_STORE_REMA_MEM_PRM
{
    PNIO_UINT32         pBufLen;
    PNIO_UINT8          pData[];
} PNIO_STORE_REMA_MEM_PRM;

/*
 * IM data handling
 */

typedef struct PNIO_IM_WRITE_PRM
{
    PNIO_UINT32         IMidx;  /* 0..4 */
    PNIO_UINT32         Api;
    PNIO_DEV_ADDR       Addr;
    PNIO_UINT32         PeriphRealCfgInd;
    PNIO_ERR_STAT       pPnioState;
    PNIO_UINT32         pBufLen;
    PNIO_UINT8          pData[];
} PNIO_IM_WRITE_PRM;

typedef struct PNIO_IM_WRITE_RSP_PRM
{
    PNIO_ERR_STAT       PnioState;
} PNIO_IM_WRITE_RSP_PRM;

typedef struct PNIO_IM_READ_PRM
{
    PNIO_UINT32         IMidx;  /* 0..4 */
    PNIO_UINT32         Api;
    PNIO_DEV_ADDR       Addr;
    PNIO_UINT32         PeriphRealCfgInd;
    PNIO_ERR_STAT       pPnioState;
    PNIO_UINT32         pBufLen;
    PNIO_UINT32         pBufAddr;
} PNIO_IM_READ_PRM;

typedef struct PNIO_IM_READ_RSP_PRM
{
    PNIO_ERR_STAT       PnioState;
    PNIO_UINT32         pBufLen;
    PNIO_UINT32         pBufAddr;
    PNIO_UINT8          pData[];
} PNIO_IM_READ_RSP_PRM;

typedef struct PNIO_IM_STORE_PRM
{
    PNIO_UINT32         NvDataType;
    PNIO_UINT32         PeriphRealCfgInd;
    PNIO_UINT32         pBufLen;
    PNIO_UINT8          pData[];
} PNIO_IM_STORE_PRM;

typedef struct PNIO_IM_STORE_RSP_RM
{
    PNIO_UINT32         Status;
    PNIO_UINT32 		DataLen;
} PNIO_IM_STORE_RSP_RM;

/*
 * Request responses
 */

typedef struct PNIO_RECORD_READ_RSP_PRM
{
    PNIO_UINT32         BufLen;
    PNIO_UINT32         pBufLen;
    PNIO_UINT32         pPnioState;
    PNIO_UINT32         pBuffer;
    PNIO_ERR_STAT       PnioState;
    PNIO_UINT8          Buffer;
}PNIO_RECORD_READ_RSP_PRM;

typedef struct PNIO_RECORD_WRITE_RSP_PRM
{
    PNIO_UINT32         BufLen;
    PNIO_UINT32         pBufLen;
    PNIO_UINT32         pPnioState;
    PNIO_ERR_STAT       PnioState;
}PNIO_RECORD_WRITE_RSP_PRM;

/*
 * AMR handler structures
 */

typedef struct PNIO_AMR_HANDLER_PRM
{
    PNIO_DEV_ADDR       pAddr;
    PNIO_UINT32         pBufLen;
    PNIO_UINT32         pBufAddr;
}PNIO_AMR_HANDLER_PRM;

typedef struct PNIO_AMR_HANDLER_RSP_PRM
{
    PNIO_AMR_HANDLER_PRM pAMRHndlPrm;
    PNIO_ERR_STAT        PnioState;
    PNIO_UINT8           pData[];
}PNIO_AMR_HANDLER_RSP_PRM;

/*
 * PROFIenergy handler structures
 */

typedef struct PNIO_PE_RESPONSE_HANDLER_PRM
{
    PNIO_DEV_ADDR       pAddr;
    PNIO_UINT32         pBufLen;
    PNIO_UINT32         pBufAddr;
    PNIO_UINT32         ArNum;
}PNIO_PE_RESPONSE_HANDLER_PRM;

typedef struct PNIO_PE_RESPONSE_HANDLER_RSP_PRM
{
    PNIO_PE_RESPONSE_HANDLER_PRM pPEHndlPrm;
    PNIO_ERR_STAT                PnioState;
    PNIO_UINT8                   pData[];
}PNIO_PE_RESPONSE_HANDLER_RSP_PRM;

typedef struct PNIO_PE_REQUEST_HANDLER_PRM
{
    PNIO_DEV_ADDR       pAddr;
    PNIO_UINT32         pBufLen;
    PNIO_UINT32         ArNum;
    PNIO_UINT8          pData[];
}PNIO_PE_REQUEST_HANDLER_PRM;

typedef struct PNIO_PE_REQUEST_HANDLER_RSP_PRM
{
    PNIO_ERR_STAT       PnioState;
}PNIO_PE_REQUEST_HANDLER_RSP_PRM;

/*
 * Callbacks from ERTEC
 */

PNIO_VOID PNIOext_cbf_ar_connect_ind(PNIO_UINT8 *params);
PNIO_VOID PNIOext_cbf_ar_ownership_ind(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_param_end_ind(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_ready_for_input_update_ind(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_ar_indata_ind(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_ar_disconn_ind(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_report_ARFSU_record(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_sub_plug_list(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_async_req_done(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_async_req_error(PNIO_UINT8 *params);
PNIO_VOID PNIOext_cbf_rec_read(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_rec_write(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_amr_response_handler(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_pe_response_handler(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_pe_request_handler(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_substval_out_read(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_save_station_name(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_save_ip_addr(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_report_new_ip_addr(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_reset_factory_settings(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_result_new_device_address(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_start_led_blink(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_stop_led_blink(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_device_startup_done(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_trace_ready(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_return_isr_handle(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_perform_isr_callback(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_nv_data_sync(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_nv_data_flash_done(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_im_data_flash_done(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_nv_data_factory_reset_flash_done(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_store_rema_mem(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_im_write(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_im_read(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_im_data_store(PNIO_UINT8* params);
PNIO_VOID PNIOext_cbf_nv_data_store(PNIO_UINT8* params);

PNIO_VOID PNIOext_cbf_nv_data_set_default(PNIO_UINT8* params);
/*
 * Calls from BBB to Ertec
 */

/*
 * User alarms call
 */
PNIO_VOID PNIOext_diag_channel_add(
		PNIO_DEV_ADDR *Addr,		// geographical or logical address
		PNIO_UINT16 ChannelNum,	 	// channel number
		PNIO_UINT16 ErrorNum,		// error number, see PNIO specification coding of "ChannelErrorType"
		PNIO_UINT16 ChanDir,		// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		PNIO_UINT16 ChanTyp,		// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
		PNIO_BOOL MaintReq,	 		// maintenance required
		PNIO_BOOL MaintDem,	 		// maintenance demanded
		PNIO_UINT16 DiagTag    		// user defined diag tag, used as reference
);
PNIO_VOID PNIOext_diag_channel_remove(
		PNIO_DEV_ADDR *Addr,		// geographical or logical address
		PNIO_UINT16 ChannelNum,	 	// channel number
		PNIO_UINT16 ErrorNum,		// error number, see PNIO specification coding of "ChannelErrorType"
		PNIO_UINT16 ChanDir,		// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		PNIO_UINT16 ChanTyp,		// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
		PNIO_UINT16 DiagTag,    	// user defined diag tag, used as reference
		PNIO_UINT16 AlarmState		// DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
);
PNIO_VOID PNIOext_ext_diag_channel_add(
		PNIO_DEV_ADDR *Addr,		// geographical or logical address
		PNIO_UINT16 ChannelNum,	 	// channel number
		PNIO_UINT16 ErrorNum,		// error number, see PNIO specification coding of "ChannelErrorType"
		PNIO_UINT16 ChanDir,		// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		PNIO_UINT16 ChanTyp,		// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
		PNIO_UINT16 ExtChannelErrType,	// ExtChannelErrorType (see IEC61158)
		PNIO_UINT16 ExtChannelAddValue,	// ExtChannelAddValue (see IEC61158)
		PNIO_BOOL MaintReq,	 		// maintenance required
		PNIO_BOOL MaintDem,	 		// maintenance demanded
		PNIO_UINT16 DiagTag    		// user defined diag tag, used as reference
);
PNIO_VOID PNIOext_ext_diag_channel_remove(
		PNIO_DEV_ADDR *Addr,		// geographical or logical address
		PNIO_UINT16 ChannelNum,	 	// channel number
		PNIO_UINT16 ErrorNum,		// error number, see PNIO specification coding of "ChannelErrorType"
		PNIO_UINT16 ChanDir,		// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		PNIO_UINT16 ChanTyp,		// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
		PNIO_UINT16 ExtChannelErrType,	 // ExtChannelErrorType (see IEC61158)
		PNIO_UINT16 ExtChannelAddValue,	 // ExtChannelAddValue (see IEC61158)
		PNIO_UINT16 DiagTag,    	// user defined diag tag, used as reference
		PNIO_UINT16 AlarmState		// DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
);
PNIO_VOID PNIOext_diag_generic_add(
		PNIO_DEV_ADDR *Addr,		// geographical or logical address
		PNIO_UINT16 ChannelNum,	 	// channel number
		PNIO_UINT16 ChanDir,	 	// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		PNIO_UINT16 ChanTyp,	 	// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
		PNIO_UINT16 DiagTag,    	// user defined diag tag, used as reference
		PNIO_UINT16 UserStructIdent,   	// manufacturer specific, 0...0x7fff, see IEC 61158
		PNIO_UINT32 InfoDataLen,	// length of generic diagnostic data
		PNIO_UINT8 *InfoData,		// user defined generic diagnostic data
		PNIO_BOOL MaintReq,	 		// maintenance required
		PNIO_BOOL MaintDem	 		// maintenance demanded
);
PNIO_VOID PNIOext_diag_generic_remove(
		PNIO_DEV_ADDR *Addr,		// geographical or logical address
		PNIO_UINT16 ChannelNum,	 	// channel number
		PNIO_UINT16 ChanDir,	 	// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		PNIO_UINT16 ChanTyp,	 	// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
		PNIO_UINT16 DiagTag,    	// user defined diag tag, used as reference
		PNIO_UINT16 UserStructIdent	// manufacturer specific, 0...0x7fff, see IEC 61158
);
PNIO_VOID PNIOext_process_alarm_send(
		PNIO_DEV_ADDR *Addr,			// geographical or logical address
		PNIO_UINT32 DataLen,			// length of AlarmItem.Data
		PNIO_UINT8 *Data,				// AlarmItem.Data
		PNIO_UINT16 UserStructIdent,	// AlarmItem.UserStructureIdentifier, s. IEC61158-6
		PNIO_UINT32 UserHndl			// user defined handle
);
PNIO_VOID PNIOext_status_alarm_send(
		PNIO_DEV_ADDR *Addr,			// geographical or logical address
		PNIO_UINT32 Api,     			// application process identifier
		PNIO_UINT32 DataLen,			// length of AlarmItem.Data
		PNIO_UINT8 *Data,				// AlarmItem.Data
		PNIO_UINT16 UserStructIdent,	// AlarmItem.UserStructureIdentifier, s. IEC61158-6
		PNIO_UINT32 UserHndl			// user defined handle
);
PNIO_VOID PNIOext_ret_of_sub_alarm_send(
		PNIO_DEV_ADDR *Addr,			// geographical or logical address
		PNIO_UINT32 UserHndl	 		// user defined handle
);
PNIO_VOID PNIOext_upload_retrieval_alarm_send(
		PNIO_DEV_ADDR *Addr,			// geographical or logical address
		PNIO_UINT32 DataLen,			// length of AlarmItem.Data
		PNIO_UINT8 *Data,				// AlarmItem.Data
		PNIO_UINT32 UserHndl	 		// user defined handle
);

/*
 * Initialize, activate, deactivate, abort
 */
PNIO_UINT32 PNIOExt_DeviceSetup(PNUSR_DEVICE_INSTANCE  *pPnUsrDev,
                                PNIO_SUB_LIST_ENTRY    *pIoSubList,
                                PNIO_UINT32            NumOfSublistEntries,
                                PNIO_IM0_LIST_ENTRY    *pIm0List,
                                PNIO_UINT32            NumOfIm0ListEntries);
PNIO_VOID PNIOext_device_start(PNIO_VOID);
PNIO_VOID PNIOext_device_stop(PNIO_VOID);
PNIO_VOID PNIOext_device_ar_abort(PNIO_UINT32 ArNum);
PNIO_VOID PNIOext_ActivateIoDatXch(PNIO_VOID);
PNIO_VOID PNIOext_DeactivateIoDatXch(PNIO_VOID);
PNIO_VOID PNIOext_slave_reboot(PNIO_VOID);

/*
 * Pull/plug submodules
 */
PNIO_VOID PNIOext_sub_plug_list(
		 PNIO_SUB_LIST_ENTRY* pIoSubList,		// plugged submodules, including PDEV
		 PNIO_UINT32 NumOfSubListEntries,		// number of entries in pIoSubList
		 PNIO_IM0_LIST_ENTRY* pIm0List,			// list of IM0 data sets
		 PNIO_UINT32 NumOfIm0ListEntries		// number of entries in pIm0List
);
PNIO_VOID PNIOext_sub_plug(
		PNIO_DEV_ADDR *Addr,			// geographical or logical address
		PNIO_UINT32 ModIdent,			// module ident number
		PNIO_UINT32 SubIdent,			// submodule ident number
        PNIO_UINT32 InputDataLen,	    // submodule input data length
        PNIO_UINT32 OutputDataLen,		// submodule output data length
        PNIO_IM0_SUPP_ENUM Im0Support,	// subslot has IM0 data for subslot/slot/device/nothing
        IM0_DATA* pIm0Dat,				// pointer to IM0 data set (if subslot has own IM0 data)
        PNIO_UINT8 IopsIniVal			// initial value for iops-input, ONLY FOR SUBMOD WITHOUT IO DATA (e.g. PDEV)
);
PNIO_VOID PNIOext_sub_pull(PNIO_DEV_ADDR *Addr);	// geographical or logical address
PNIO_VOID PNIOext_substval_out_read_done(
        PNIO_UINT8 *Buffer,          /* Ptr to submodule output substitute data */
        PNIO_UINT32 BufLen,          /* Pass length from ERTEC */
        PNIO_UINT32 BufAddr,         /* Pass address from ERTEC */
        PNIO_UINT16 SubstMode,       /* [BIG ENDIAN] SubstitutionMode: 0=ZERO or inactive (default), 1:LastValue, 2:Replacement value SubstitutionMode: 0=ZERO or inactive, 1:LastValue, 2:Replacement value */
        PNIO_UINT32 SubstModeAddr,   /* Pass address from ERTEC */
        PNIO_UINT16 SubstActive,     /* [BIG ENDIAN] SubstituteActiveFlag:  0=operation, 1=substitute. default value is 0: if (IOPS & IOCS = GOOD), else: 1 */
        PNIO_UINT32 SubstActiveAddr);

/*
 * Watchdog feature
 */
PNIO_VOID PNIOext_hw_watchdog_set(PNIO_UINT32 time_watchdog, PNIO_WD_GRANITY granity);
PNIO_VOID PNIOext_hw_watchdog_command(PNIO_UINT32 command);

/*
 * Change device name , IP and MAC address
 */
PNIO_VOID PNIOext_store_new_MAC(PNIO_UINT8* mac_addr);
PNIO_VOID PNIOext_store_new_IP(PNIO_UINT8* ip_suite);
PNIO_VOID PNIOext_store_new_device_name(PNIO_UINT8* dev_name);

/*
 * Trace functionality
 */
PNIO_VOID PNIOext_trace_command(PNIO_TRACE_COMMAND command);
PNIO_VOID PNIOext_trace_settings(PNIO_TRACE_SETTINGS_COMMAND command, PNIO_UINT32 module, PNIO_UINT32 trc_level);

/*
 * Non-volatile memory functionality
 */
PNIO_VOID PNIOext_nv_data_init(PNIO_UINT8* pData, PNIO_UINT32 pBufLen);
PNIO_VOID PNIOext_nv_data_clear(PNIO_UINT32 RtfOption);
PNIO_VOID PNIOext_nv_data_store(PNIO_UINT32 NvDataType, PNIO_UINT32 pBufLen, PNIO_UINT8* pData);
PNIO_VOID PNIOext_im_data_store(PNIO_UINT32 NvDataType, PNIO_UINT32 ModIdent, PNIO_UINT32 pBufLen, PNIO_UINT8* pData);

/*
 * Request responses
 */
PNIO_VOID PNIOext_rec_read_rsp(PNIO_RECORD_READ_PRM *prm, PNIO_UINT8 *pBuffer, PNIO_ERR_STAT *PnioState);
PNIO_VOID PNIOext_rec_write_rsp(PNIO_RECORD_WRITE_PRM *prm, PNIO_ERR_STAT *pPnioState);
PNIO_VOID PNIOext_amr_response_handler_rsp(PNIO_AMR_HANDLER_PRM *prm, PNIO_ERR_STAT *pPnioState, PNIO_UINT8* pData);
PNIO_VOID PNIOext_pe_response_handler_rsp(PNIO_PE_RESPONSE_HANDLER_PRM *prm, PNIO_ERR_STAT *pPnioState, PNIO_UINT8* pData);
PNIO_VOID PNIOext_pe_request_handler_rsp(PNIO_ERR_STAT *pPnioState);

PNIO_UINT32 PNIOext_get_last_apdu_status(PNIO_UINT32 ArNum);

/*
 * IM data handling
 */
PNIO_VOID PNIOext_im_write_rsp(PNIO_ERR_STAT* PnioState);
PNIO_VOID PNIOext_im_read_rsp(PNIO_IM_READ_PRM* pParams, PNIO_UINT8* pBuffer);


/*------------------------------------------------------------------*/
/*             RETURN CODES OF TESTS                                */
/*------------------------------------------------------------------*/
#define USR_ADDR_TEST_OK                0x00
#define USR_NAME_INVALID_INPUT          0x01
#define USR_NAME_INVALID_CHAR           0x02
#define USR_NAME_LIKE_IP_ADDR           0x03
#define USR_NAME_LIKE_PORT_NAME         0x04
#define USR_NAME_INVALID_POSITION       0x05    /* character at invalid position - i.e. ending by '-' */
#define USR_NAME_LABEL_BAD_SIZE         0x06    /* max length of label = 63, min length = 1 */
#define USR_IP_INVALID_MAC              0x21
#define USR_IP_MUTUALY_EXCLUSIVE_PRMS   0x22    /* i.e. incompatible ip versus mask*/
#define USR_IP_INVALID_GATEWAY          0x23
#define USR_ADDR_AR_FORBIDDEN           0x30
#define USR_ADDR_NO_CHANGE              0x31


#endif /* HOST_SRC_PNPB_LIB_ACYC_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
