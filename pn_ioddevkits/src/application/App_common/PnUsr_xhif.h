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
/*  F i l e               &F: PnUsr_xhif.h                              :F&  */
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


/**
* @file     PnUsr_xhif.h
* @brief    Functions for XHIF - library for Ertec
*
* XHIF functionality allows user to use ERTEC Devkit only for PN-stack functionalities
* and to realize user functionality on other device. The other device have to upload
* firmware to ERTEC Devkit as a binary and then communicate data.. This can be realized
* via XHIF memory interface.
*/

#ifndef APPLICATION_APP_COMMON_PNUSR_XHIF_H_
#define APPLICATION_APP_COMMON_PNUSR_XHIF_H_

#if(1 == IOD_USED_WITH_XHIF_HOST)

#include "usrapp_cfg.h"

#define PNUSR_XHIF_NUMOF_BYTES_PER_ACYCLIC_TELEGRAM     1024
#define PNUSR_XHIF_ACYCLIC_TELEGRAM_HEADER_SIZE         4   /* 2B ID + 2B prm_size */
#define PNUSR_XHIF_ACYCLIC_DATA_MAX                     (PNUSR_XHIF_NUMOF_BYTES_PER_ACYCLIC_TELEGRAM - PNUSR_XHIF_ACYCLIC_TELEGRAM_HEADER_SIZE)
#define PNUSR_XHIF_SIZE_OF_BUFFER_FOR_CYCLIC            470 /* max num of records */
#define PNUSR_XHIF_SIZE_OF_BUFFER_FOR_ACYC              250 /* max num of records */
#define PNUSR_XHIF_MAX_NUM_OF_SUBMODULES                NUMOF_SLOTS * NUMOF_SUBSLOTS
#define PNUSR_XHIF_SIZE_OF_BUFFER_FOR_TRACES            (1024*128) /* max num of bytes */

/* Placement of memory interface - sections from linkerscript */
#define PNUSR_XHIF_CYC_IN               ".xhif_mi_cyclic_in"
#define PNUSR_XHIF_CYC_IN_BUF           ".xhif_mi_cyclic_in_buf"
#define PNUSR_XHIF_SECTION_CYC_IN       __attribute__((section (PNUSR_XHIF_CYC_IN)))
#define PNUSR_XHIF_SECTION_CYC_IN_BUF   __attribute__((section (PNUSR_XHIF_CYC_IN_BUF)))
#define PNUSR_XHIF_CYC_OUT              ".xhif_mi_cyclic_out"
#define PNUSR_XHIF_CYC_OUT_BUF          ".xhif_mi_cyclic_out_buf"
#define PNUSR_XHIF_SECTION_CYC_OUT      __attribute__((section (PNUSR_XHIF_CYC_OUT)))
#define PNUSR_XHIF_SECTION_CYC_OUT_BUF  __attribute__((section (PNUSR_XHIF_CYC_OUT_BUF)))
#define PNUSR_XHIF_ACY_IN               ".xhif_mi_acyc_in"
#define PNUSR_XHIF_ACY_IN_BUF           ".xhif_mi_acyc_in_buf"
#define PNUSR_XHIF_SECTION_ACY_IN       __attribute__((section (PNUSR_XHIF_ACY_IN)))
#define PNUSR_XHIF_SECTION_ACY_IN_BUF   __attribute__((section (PNUSR_XHIF_ACY_IN_BUF)))
#define PNUSR_XHIF_ACY_OUT              ".xhif_mi_acyc_out"
#define PNUSR_XHIF_ACY_OUT_BUF          ".xhif_mi_acyc_out_buf"
#define PNUSR_XHIF_SECTION_ACY_OUT      __attribute__((section (PNUSR_XHIF_ACY_OUT)))
#define PNUSR_XHIF_SECTION_ACY_OUT_BUF  __attribute__((section (PNUSR_XHIF_ACY_OUT_BUF)))
#define PNUSR_XHIF_TRACES               ".xhif_mi_traces"
#define PNUSR_XHIF_SECTION_TRACES       __attribute__((section (PNUSR_XHIF_TRACES)))

#define PNUSR_XHIF_DIRECTION_NO_DATA    0x00
#define PNUSR_XHIF_DIRECTION_IN         0x01
#define PNUSR_XHIF_DIRECTION_OUT        0x02
#define PNUSR_XHIF_DIRECTION_INOUT      0x03

#define PNUSR_INTERRUPT_GPIO2           34
#define PNUSR_INTERRUPT_GPIO6           38


/* Memory interface telegram IDs - Have to comply with same on Ertec side */
typedef enum PNPB_XHIF_ACYC_TELEGRAMS
{
    /* Ertec - Host */
    PNPB_XHIF_ACYC_NO_TELEGRAM = 0,
    PNPB_XHIF_ACYC_AR_CONNECT_IND,
    PNPB_XHIF_ACYC_AR_OWNERSHIP_IND,
    PNPB_XHIF_ACYC_AR_OWNERSHIP_IND_MORE_FOLLOWS,
    PNPB_XHIF_ACYC_PARAM_END_IND,
    PNPB_XHIF_ACYC_READY_FOR_INPUT_UPDATE_IND,
    PNPB_XHIF_ACYC_AR_INDATA_IND,
    PNPB_XHIF_ACYC_AR_DISCONNECT_IND,
    PNPB_XHIF_ACYC_REPORT_ARFSU_RECORD,
    PNPB_XHIF_ACYC_SUB_PLUG_LIST_CBF,
    PNPB_XHIF_ACYC_ASYNC_REQUEST_DONE,
    PNPB_XHIF_ACYC_ASYNC_REQUEST_ERROR,
    PNPB_XHIF_ACYC_REC_READ,
    PNPB_XHIF_ACYC_REC_READ_MORE_FOLLOWS,
    PNPB_XHIF_ACYC_REC_WRITE,
    PNPB_XHIF_ACYC_REC_WRITE_MORE_FOLLOWS,
    PNPB_XHIF_ACYC_AMR_READ,
    PNPB_XHIF_ACYC_PE_RESPONSE,
    PNPB_XHIF_ACYC_PE_REQUEST,
    PNPB_XHIF_ACYC_PE_REQUEST_MORE_FOLLOWS,
    PNPB_XHIF_ACYC_NV_DATA_SYNC,
    PNPB_XHIF_ACYC_NV_DATA_SYNC_MORE_FOLLOWS,
    PNPB_XHIF_ACYC_NV_DATA_FLASH_DONE,
    PNPB_XHIF_ACYC_IM_DATA_FLASH_DONE,
    PNPB_XHIF_ACYC_NV_DATA_RESET_DONE,
    PNPB_XHIF_ACYC_SUBSTVAL_OUT_READ,
    PNPB_XHIF_ACYC_SAVE_STATION_NAME,
    PNPB_XHIF_ACYC_SAVE_IP_ADDR,
    PNPB_XHIF_ACYC_STORE_REMA_MEM,
    PNPB_XHIF_ACYC_STORE_REMA_MEM_MORE_FOLLOWS,
    PNPB_XHIF_ACYC_REPORT_NEW_IP_ADDR,
    PNPB_XHIF_ACYC_RESET_FACTORY_SETTINGS,
    PNPB_XHIF_ACYC_RESULT_NEW_DEVICE_ADDRESS,
    PNPB_XHIF_ACYC_START_LED_BLINK,
    PNPB_XHIF_ACYC_STOP_LED_BLINK,
    PNPB_XHIF_ACYC_DEVICE_STARTUP_DONE,
    PNPB_XHIF_ACYC_TRACE_READY,
    PNPB_XHIF_ACYC_RETURN_ISR_HANDLE,
    PNPB_XHIF_ACYC_PERFORM_ISR_CALLBACK,
    PNPB_XHIF_ACYC_RESPONSE_APDU_STATUS,
    PNPB_XHIF_ACYC_IM_WRITE,
    PNPB_XHIF_ACYC_IM_WRITE_MORE_FOLLOWS,
    PNPB_XHIF_ACYC_IM_READ,
    PNPB_XHIF_ACYC_IM_STORE,
    PNPB_XHIF_ACYC_IM_STORE_MORE_FOLLOWS,
	PNPB_XHIF_ACYC_NV_DATA_STORE_HOST,
	PNPB_XHIF_ACYC_NV_DATA_STORE_HOST_MORE_FOLLOWS,
	PNPB_XHIF_ACYC_NV_DATA_SET_DEFAULT,
	PNPB_XHIF_ACYC_NV_DATA_SET_DEFAULT_MORE_FOLLOWS,
    /* To find out number of defined cbf telegrams */
    PNPB_XHIF_ACYC_NUM_OF_ERTEC_HOST_TELEGRAMS,
    /* Host - Ertec */
    PNPB_XHIF_ACYC_DIAG_CHANNEL_ADD = 128,  /* First in this direction (as featurespec)*/
    PNPB_XHIF_ACYC_DIAG_CHANNEL_REMOVE,
    PNPB_XHIF_ACYC_EXT_DIAG_CHANNEL_ADD,
    PNPB_XHIF_ACYC_EXT_DIAG_CHANNEL_REMOVE,
    PNPB_XHIF_ACYC_DIAG_GENERIC_ADD,
    PNPB_XHIF_ACYC_DIAG_GENERIC_REMOVE,
    PNPB_XHIF_ACYC_PROCESS_ALARM_SEND,
	PNPB_XHIF_ACYC_STATUS_ALARM_SEND,
    PNPB_XHIF_ACYC_RET_OF_SUB_ALARM_SEND,
    PNPB_XHIF_ACYC_UPLOAD_RETRIEVAL_ALARM_SEND,
    PNPB_XHIF_ACYC_DEVICE_SETUP,
    PNPB_XHIF_ACYC_DEVICE_SETUP_MORE_FOLLOWS,
    PNPB_XHIF_ACYC_DEVICE_START,
    PNPB_XHIF_ACYC_DEVICE_STOP,
    PNPB_XHIF_ACYC_DEVICE_AR_ABORT,
    PNPB_XHIF_ACYC_DEVICE_OPEN,
    PNPB_XHIF_ACYC_ACTIVATE_IO_DAT_XCH,
    PNPB_XHIF_ACYC_DEACTIVATE_IO_DAT_XCH,
    PNPB_XHIF_ACYC_SLAVE_REBOOT,
    PNPB_XHIF_ACYC_SUB_PLUG_LIST,
    PNPB_XHIF_ACYC_SUB_PLUG,
    PNPB_XHIF_ACYC_SUB_PULL,
    PNPB_XHIF_ACYC_SUBSTVAL_OUT_READ_DONE,
    PNPB_XHIF_ACYC_SET_DEV_STATE,
    PNPB_XHIF_ACYC_REC_READ_RSP,
    PNPB_XHIF_ACYC_REC_READ_RSP_MORE_FOLLOWS,
    PNPB_XHIF_ACYC_REC_WRITE_RSP,
    PNPB_XHIF_ACYC_REC_WRITE_RSP_MORE_FOLLOWS,
    PNPB_XHIF_ACYC_AMR_READ_RSP,
    PNPB_XHIF_ACYC_AMR_READ_RSP_MORE_FOLLOWS,
    PNPB_XHIF_ACYC_PE_RESPONSE_RSP,
    PNPB_XHIF_ACYC_PE_RESPONSE_RSP_MORE_FOLLOWS,
    PNPB_XHIF_ACYC_PE_REQUEST_RSP,
    PNPB_XHIF_ACYC_NV_DATA_INIT,
    PNPB_XHIF_ACYC_NV_DATA_INIT_MORE_FOLLOWS,
    PNPB_XHIF_ACYC_NV_DATA_CLEAR,
    PNPB_XHIF_ACYC_NV_DATA_STORE,
    PNPB_XHIF_ACYC_NV_DATA_STORE_MORE_FOLLOWS,
    PNPB_XHIF_ACYC_NV_DATA_IM_STORE,
    PNPB_XHIF_ACYC_NV_DATA_IM_STORE_MORE_FOLLOWS,
    PNPB_XHIF_ACYC_GET_LAST_APDU_STATUS,
    PNPB_XHIF_ACYC_SET_IOPS,
    PNPB_XHIF_ACYC_ISO_ACTIVATE_ISR_OBJ,
    PNPB_XHIF_ACYC_ISO_FREE_OBJ,
    PNPB_XHIF_ACYC_ISO_ACTIVATE_GPIO_OBJ,
    PNPB_XHIF_ACYC_HW_WATCHDOG_SET,
    PNPB_XHIF_ACYC_HW_WATCHDOG_COMMAND,
    PNPB_XHIF_ACYC_STORE_NEW_MAC,
    PNPB_XHIF_ACYC_STORE_NEW_IP,
    PNPB_XHIF_ACYC_STORE_NEW_DEVICE_NAME,
    PNPB_XHIF_ACYC_TRACE_COMMAND,
    PNPB_XHIF_ACYC_TRACE_SETTINGS,
    PNPB_XHIF_ACYC_READ_APDU_STATUS,
    PNPB_XHIF_ACYC_IM_WRITE_RSP,
    PNPB_XHIF_ACYC_IM_READ_RSP,
    PNPB_XHIF_ACYC_IM_READ_RSP_MORE_FOLLOWS,
    PNPB_XHIF_ACYC_TEST,
    /* To find out number of defined telegrams */
    PNPB_XHIF_ACYC_NUM_OF_TELEGRAMS
}PNPB_XHIF_ACYC_TELEGRAMS;

typedef enum PNPB_USR_STARTUP_STATE
{
    PNPB_USR_START_IDLE = 0x12345678,
    PNPB_USR_START_BOOT_OK = 0x24682468,
    PNPB_USR_START_START_OK = 0x55555555,
    PNPB_USR_START_START_FAILED = 0xa0b0c0d0
}PNPB_USR_STARTUP_STATE;

#define PNPB_XHIF_ACYC_FIRST_HOST_ERTEC_TELEGRAM    128
#define PNPB_XHIF_ACYC_NUM_OF_HOST_ERTEC_TELEGRAMS  (PNPB_XHIF_ACYC_NUM_OF_TELEGRAMS - PNPB_XHIF_ACYC_FIRST_HOST_ERTEC_TELEGRAM)


/* Structures for parameters transferred by memory interface*/
typedef struct PNIO_CONNECT_IND_PRM{
    PNIO_AR_TYPE ArType;
    PNIO_UINT32 ArNum;
    PNIO_UINT16 SendClock;
    PNIO_UINT16 RedRatioIocrIn;
    PNIO_UINT16 RedRatioIocrOut;
    PNIO_UINT16 ArSessionKey;
    PNIO_UINT32 hostIP;
}PNIO_CONNECT_IND_PRM;

typedef struct PNIO_OWNERSHIP_IND_PRM
{
    PNIO_UINT32 ArNum;      /* AR number 1....NumOfAR */
    PNIO_EXP OwnSub;        /* Expected configuration in ownership indication */
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
} PNIO_RESULT_NEW_DEV_ADDR_PRM;

typedef struct PNIO_SUBSTVAL_OUT_READ_PRM
{
    PNIO_DEV_ADDR Addr;
    PNIO_UINT32 BufLen;
    PNIO_UINT32 BufAddr;
    PNIO_UINT32 SubstModeAddr;
    PNIO_UINT32 SubstActiveAddr;
} PNIO_SUBSTVAL_OUT_READ_PRM;

typedef struct PNIO_SEND_INFO_PRM
{
    PNIO_UINT8 ip_suite[12];
    PNIO_UINT8 mac_addr[6];
} PNIO_SEND_INFO_PRM;

typedef struct PNIO_FW_VERSION
{
    PNIO_UINT8 VerPrefix;
    PNIO_UINT8 VerHh;
    PNIO_UINT8 VerH;
    PNIO_UINT8 VerL;
    PNIO_UINT8 VerLl;
}PNIO_FW_VERSION;

typedef struct PNIO_STARTUP_DONE_PRM
{
    PNIO_UINT32 State;
    PNIO_FW_VERSION Version;
	PNIO_UINT32 Fw_DAP;
} PNIO_STARTUP_DONE_PRM;

typedef struct PNIO_ASYNC_ERROR_PRM
{
    PNIO_UINT32 Type;
    PNIO_UINT32 ErrorCode;
} PNIO_ASYNC_ERROR_PRM;

/* Data for one subslot as will be exchanged by memory interface */
typedef volatile struct pnusr_xhif_subslot_data
{
    PNIO_UINT32 Lock;
    PNIO_UINT32 Slot;
    PNIO_UINT32 Subslot;
    PNIO_UINT32 IOxS;
    PNIO_UINT32 Data_size;
	PNIO_UINT8  Data[NUMOF_BYTES_PER_SUBSLOT];
}pnusr_xhif_subslot_data;

typedef volatile struct pnusr_subslot_params
{
    PNIO_UINT32 Slot;
    PNIO_UINT32 Subslot;
    PNIO_UINT8  InIOpS;
    PNIO_UINT8  InIOcS;
    PNIO_UINT8  OutIOpS;
    PNIO_UINT8  OutIOcS;
    PNIO_UINT8  InData_size;
    PNIO_UINT8  OutData_size;
    PNIO_UINT8  Direction;
}pnusr_subslot_params;
typedef pnusr_subslot_params * pnusr_subslot_params_ptr;

typedef volatile struct pnusr_subslot_data
{
    PNIO_UINT8 Data[NUMOF_BYTES_PER_SUBSLOT];
}pnusr_subslot_data;
typedef pnusr_subslot_data * pnusr_subslot_data_ptr;

/* Any acyclic telegram as will be exchanged by memory interface */
typedef volatile struct pnusr_xhif_acyclic
{
    PNIO_UINT32 lock;
	PNIO_UINT32 id;
	PNIO_UINT32 data_len;
	PNIO_UINT8 data[PNUSR_XHIF_NUMOF_BYTES_PER_ACYCLIC_TELEGRAM];
}pnusr_xhif_acyclic;


/* Each buffer have to have read and write pointer - cyclic buffer realization */
typedef volatile struct pnusr_xhif_buffer_services
{
    PNIO_UINT32 read_ptr;
    PNIO_UINT32 write_ptr;
}pnusr_xhif_buffer_services;


pnusr_subslot_data_ptr      pnusr_submodule_IO_data [PNUSR_XHIF_MAX_NUM_OF_SUBMODULES];
volatile pnusr_subslot_params* pnusr_submodule_params  [PNUSR_XHIF_MAX_NUM_OF_SUBMODULES];

typedef volatile struct pnusr_xhif_acyclic_telegrams
{
    PNIO_UINT32 prm_len;
    PNIO_VOID *function_call;
}pnusr_xhif_acyclic_telegrams;

typedef struct pnusr_xhif_acyclic_recieve_continuous
{
    PNIO_UINT32 more_follows;
    PNIO_INT32  data_remains;
    PNIO_UINT8  *p_start_data;
    PNIO_UINT8  *p_write_data;
}pnusr_xhif_acyclic_recieve_continuous;
pnusr_xhif_acyclic_recieve_continuous pnusr_xhif_acyclic_recieve_cont_manage;

typedef struct pnusr_xhif_function_calls
{
    PNPB_XHIF_ACYC_TELEGRAMS   id;
    PNIO_VOID                   (*function_call) (PNIO_UINT8*);
}pnusr_xhif_function_calls;
pnusr_xhif_function_calls pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_NUM_OF_TELEGRAMS];

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

typedef struct PNUSR_DEV_EXP_INSTANCE
{
    PNIO_UINT16     VendorId;
    PNIO_UINT16     DeviceId;
    PNIO_UINT32     MaxNumOfSubslots;
    PNIO_UINT32     MaxNumOfBytesPerSubslot;
    PNIO_UINT32     DevTypeLen;
} PNUSR_DEV_EXP_INSTANCE;

PNIO_UINT32   PnioNewAcyclicSemId;
PNIO_UINT32   PnioAcycSyncSemId;

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

typedef struct PNIO_ACYC_WRITE_MSG
{
    PNPB_XHIF_ACYC_TELEGRAMS       id;
    PNIO_UINT16                    prm_len;
    PNIO_VOID                      *p_prm;
}PNIO_ACYC_WRITE_MSG;

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
 * global prototypes
 */
PNIO_UINT32 PnUsr_xhif_IO_buffer_init(PNIO_SUB_LIST_ENTRY* pIoSubList,
                                    PNIO_UINT32          NumOfSubListEntries);
PNIO_VOID PnUsr_xhif_no_telegram(PNIO_UINT8* params);
PNIO_VOID PnUsr_xhif_pn_on(PNIO_UINT8* params);
PNIO_VOID PnUsr_xhif_pn_off(PNIO_UINT8* params);

/* From stack to XHIF and through it to application */
PNIO_IOXS PnUsr_xhif_IO_data_read(PNIO_UINT32   DevHndl,        // [in]  device handle
                                PNIO_DEV_ADDR   *pAddr,         // [in]  geographical or logical address
                                PNIO_UINT32     BufLen,         // [in]  length of the submodule input data
                                PNIO_UINT8*     pBuffer,        // [in]  Ptr to data buffer to read from
                                PNIO_IOXS       IOpS            // [in]  (io controller) provider status)
                                );
/* Read from XHIF, in services = write to stack */
PNIO_UINT32 PnUsr_xhif_cyclical_read();
PNIO_UINT32 PnUsr_xhif_cyclical_write_apdu(PNIO_UINT32 ArNum, PNIO_UINT32 Apdu);
PNIO_IOXS PnUsr_xhif_IO_data_write(PNIO_UINT32  DevHndl,        // [in]  device handle
                                PNIO_DEV_ADDR   *pAddr,         // [in]  geographical or logical address
                                PNIO_UINT32     BufLen,         // [in]  length of the submodule input data
                                PNIO_UINT8      *pBuffer,       // [in]  Ptr to data buffer to read from
                                PNIO_IOXS       IOcS            // [in]  remote (io controller) consumer status
                                );

PNIO_VOID PnUsr_xhif_gpio_init();
PNIO_VOID PnUsr_xhif_acyc_set_int(PNIO_VOID);
PNIO_VOID PnUsr_xhif_acyc_confirm_set_int(PNIO_VOID);
PNIO_VOID PnUsr_xhif_acyc_read();
PNIO_VOID PnUsr_xhif_cycl_gpio_trigger();
PNIO_UINT32 PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_TELEGRAMS id, PNIO_UINT16 prm_len, PNIO_UINT8 *p_prm);
PNIO_VOID PnUsr_xhif_wait( PNIO_VOID );
/*
 * Callbacks from ERTEC to BBB
 */
PNIO_VOID PNIOext_cbf_device_startup_done(PNPB_USR_STARTUP_STATE State, PNIO_FW_VERSION *Version);
PNIO_VOID PNIOext_cbf_ar_connect_ind(PNIO_AR_TYPE ArType, PNIO_UINT32 ArNum,
        PNIO_UINT16 SendClock, PNIO_UINT16 RedRatioIocrIn, PNIO_UINT16 RedRatioIocrOut, PNIO_UINT16 ArSessionKey, PNIO_UINT32 hostIP);
PNIO_VOID PNIOext_cbf_ar_ownership_ind(PNIO_UINT32 ArNum,PNIO_EXP *pOwnSub);
PNIO_VOID PNIOext_cbf_param_end_ind(PNIO_UINT16 ArNum,
                                        PNIO_UINT16 SessionKey,
                                        PNIO_UINT32 Api,
                                        PNIO_UINT16 SlotNum,
                                        PNIO_UINT16 SubslotNum,
                                        PNIO_BOOL   MoreFollows);
PNIO_VOID PNIOext_cbf_ready_for_input_update_ind(PNIO_UINT16 ArNum, PNIO_INP_UPDATE_STATE InpUpdState, PNIO_UINT32 Apdu);
PNIO_VOID PNIOext_cbf_ar_indata_ind(PNIO_UINT16 ArNum, PNIO_UINT16 SessionKey);
PNIO_VOID PNIOext_cbf_ar_disconn_ind(PNIO_UINT16 ArNum, PNIO_UINT16 SessionKey, PNIO_AR_REASON ReasonCode);
PNIO_VOID PNIOext_cbf_report_ARFSU_record(PNIO_UINT8 ARFSU_enabled, PNIO_UINT8 ARFSU_changed);
PNIO_VOID PNIOext_cbf_async_req_done(PNIO_UINT32     ArNum,
                                    PNIO_ALARM_TYPE AlarmType,
                                    PNIO_UINT32     Api,
                                    PNIO_DEV_ADDR   *pAddr,
                                    PNIO_UINT32     Status,
                                    PNIO_UINT16     Diag_tag);
PNIO_VOID PNIOext_cbf_rec_read(PNIO_UINT32         Api,
                                PNIO_UINT16         ArNum,
                                PNIO_UINT16         SessionKey,
                                PNIO_UINT32         SequenceNum,
                                PNIO_DEV_ADDR       *pAddr,
                                PNIO_UINT32         RecordIndex,
                                PNIO_UINT32         *pBufLen,
                                PNIO_UINT8          *pBuffer,
                                PNIO_ERR_STAT       *pPnioState);

PNIO_VOID PNIOext_cbf_rec_write(PNIO_UINT32         Api,
                                PNIO_UINT16         ArNum,
                                PNIO_UINT16         SessionKey,
                                PNIO_UINT32         SequenceNum,
                                PNIO_DEV_ADDR       *pAddr,
                                PNIO_UINT32         RecordIndex,
                                PNIO_UINT32         *pBufLen,
                                PNIO_UINT8          *pBuffer,
                                PNIO_ERR_STAT       *pPnioState);
PNIO_VOID PNIOext_cbf_save_station_name(
        PNIO_UINT16 NameLength,
        PNIO_INT8 *pStationName,
        PNIO_UINT8 Remanent);
PNIO_VOID PNIOext_cbf_save_ip_address(
        PNIO_UINT32 NewIpAddr,
        PNIO_UINT32 SubnetMask,
        PNIO_UINT32 DefRouterAddr,
        PNIO_UINT8 Remanent);
PNIO_VOID PNIOext_cbf_report_new_ip_address(
        PNIO_UINT32 NewIpAddr,
        PNIO_UINT32 SubnetMask,
        PNIO_UINT32 DefRouterAddr);
PNIO_VOID PNIOext_cbf_reset_factory_settings(PNIO_UINT32 RtfOption);
PNIO_VOID PNIOext_cbf_result_new_device_address (PNIO_NEW_DEV_ADDR_COMMAND command, PNIO_UINT8 result);
PNIO_UINT32 PNIOext_cbf_substval_out_read(
        PNIO_DEV_ADDR  *pAddr,
        PNIO_UINT32    BufLen,
        PNIO_UINT8     *pBuffer,
        PNIO_UINT16*   pSubstMode,
        PNIO_UINT16*   pSubstActive);
PNIO_VOID PNIOext_cbf_amr_response_handler(
        PNIO_DEV_ADDR         *pAddr,
        PNIO_UINT32           *pBufLen,
        PNIO_UINT8            *pBuffer);
PNIO_VOID PNIOext_cbf_pe_response_handler(
        PNIO_DEV_ADDR         *pAddr,
        PNIO_UINT32           *pBufLen,
        PNIO_UINT8            *pBuffer,
        PNIO_UINT16            ArNum);
PNIO_VOID PNIOext_cbf_pe_request_handler(
        PNIO_DEV_ADDR         *pAddr,
        PNIO_UINT32           *pBufLen,
        PNIO_UINT8            *pBuffer,
        PNIO_UINT16            ArNum);
PNIO_VOID PNIOext_cbf_nv_data_sync(PNIO_UINT8* pData, PNIO_UINT32 DatLen, PNIO_UINT32 errOccured);
PNIO_VOID PNIOext_cbf_nv_data_flash_done(PNIO_UINT32 Status, PNIO_UINT32 DatLen, PNIO_UINT32 nvDataType);
PNIO_VOID PNIOext_cbf_im_data_flash_done(PNIO_UINT32 Status, PNIO_UINT32 DatLen, PNIO_UINT32 nvDataType);
PNIO_VOID PNIOext_cbf_nv_data_factory_reset_flash_done(PNIO_UINT32 Status, PNIO_UINT32 DatLen);
PNIO_VOID PNIOext_cbf_store_rema_mem(PNIO_UINT32 BufLen, PNIO_UINT8* Data);
PNIO_VOID PNIOext_cbf_im_write(
        PNIO_UINT32         IMidx,
        PNIO_UINT32         Api,              // api number
        PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
        PNIO_UINT32         *pBufLen,         // [in, out] in: length to read, out: length, read by user
        PNIO_UINT8          *pBuffer,         // [in] buffer pointer
        PNIO_UINT32         PeriphRealCfgInd, // entity index in periph interface-real_cfg
        PNIO_ERR_STAT       *pPnioState
);
PNIO_VOID PNIOext_cbf_im_read(
        PNIO_UINT32         IMidx,
        PNIO_UINT32         Api,              // api number
        PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
        PNIO_UINT32         *pBufLen,         // [in, out] in: length to read, out: length, read by user
        PNIO_UINT8          *pBuffer,         // [in] buffer pointer
        PNIO_UINT32         PeriphRealCfgInd, // entity index in periph interface-real_cfg
        PNIO_ERR_STAT       *pPnioState
);
PNIO_VOID PNIOext_cbf_im_data_store(
		PNIO_UINT32    		NvDataType,      // type of data (device name, IP suite, PDEV records,...)
		void*               pMem,            // pointer to data source
		PNIO_UINT32         MemSize,         // size of memory to store
        PNIO_UINT32         PeriphRealCfgInd, // entity index in periph interface-real_cfg
        PNIO_UINT32         triggerPNDV
);

PNIO_VOID PNIOext_cbf_nv_data_store(
		PNIO_UINT32    		NvDataType,      // type of data (device name, IP suite, PDEV records,...)
		void*               pMem,            // pointer to data source
		PNIO_UINT32         MemSize          // size of memory to store
);

PNIO_VOID PNIOext_cbf_nv_data_set_default(
		PNIO_UINT32    		NvDataType
);

/* Install function pointers to array */
PNIO_VOID PnUsr_xhif_prepare_function_calls(void);

PNIO_VOID PNIO_XHIF_Write_Task();

/*
 * 	User alarm calls
 */
PNIO_VOID PNIOext_diag_channel_add(PNIO_UINT8* params);
PNIO_VOID PNIOext_diag_channel_remove(PNIO_UINT8* params);
PNIO_VOID PNIOext_ext_diag_channel_add(PNIO_UINT8* params);
PNIO_VOID PNIOext_ext_diag_channel_remove(PNIO_UINT8* params);
PNIO_VOID PNIOext_diag_generic_add(PNIO_UINT8* params);
PNIO_VOID PNIOext_diag_generic_remove(PNIO_UINT8* params);
PNIO_VOID PNIOext_process_alarm_send(PNIO_UINT8* params);
PNIO_VOID PNIOext_status_alarm_send(PNIO_UINT8* params);
PNIO_VOID PNIOext_ret_of_sub_alarm_send(PNIO_UINT8* params);
PNIO_VOID PNIOext_upload_retrieval_alarm_send(PNIO_UINT8* params);

/*
 * Initialize, activate, deactivate, abort
 */
PNIO_VOID PNIOExt_DeviceSetup(PNIO_UINT8* params);
PNIO_VOID PNIOext_device_start(PNIO_UINT8* params);
PNIO_VOID PNIOext_device_stop(PNIO_UINT8* params);
PNIO_VOID PNIOext_device_ar_abort(PNIO_UINT8* params);
PNIO_VOID PNIOext_device_open(PNIO_UINT8* params);
PNIO_VOID PNIOext_ActivateIoDatXch(PNIO_UINT8* params);
PNIO_VOID PNIOext_DeactivateIoDatXch(PNIO_UINT8* params);
PNIO_VOID PNIOext_slave_reboot(PNIO_UINT8* params);

/*
 * Pull/plug submodules
 */
PNIO_VOID PNIOext_sub_plug_list(PNIO_UINT8* params);
PNIO_VOID PNIOext_sub_plug(PNIO_UINT8* params);
PNIO_VOID PNIOext_sub_pull(PNIO_UINT8* params);
PNIO_VOID PNIOext_substval_out_read_done(PNIO_UINT8* params);

/*
 * Watchdog feature
 */
PNIO_VOID PNIOext_hw_watchdog_set(PNIO_UINT8* params);
PNIO_VOID PNIOext_hw_watchdog_command(PNIO_UINT8* params);

/*
 * Change device name , IP and MAC address
 */
PNIO_VOID PNIOext_store_new_MAC(PNIO_UINT8* params);
PNIO_VOID PNIOext_store_new_IP(PNIO_UINT8* params);
PNIO_VOID PNIOext_store_new_device_name(PNIO_UINT8* params);

/*
 * Request responses, AMR + PE
 */
PNIO_VOID PNIOext_rec_read_rsp(PNIO_UINT8* params);
PNIO_VOID PNIOext_rec_write_rsp(PNIO_UINT8* params);
PNIO_VOID PNIOext_amr_response_handler_rsp(PNIO_UINT8* params);
PNIO_VOID PNIOext_pe_response_handler_rsp(PNIO_UINT8* params);
PNIO_VOID PNIOext_pe_request_handler_rsp(PNIO_UINT8* params);

/*
 * Trace functionality
 */
PNIO_VOID PNIOext_trace_command(PNIO_UINT8* params);
PNIO_VOID PNIOext_trace_settings(PNIO_UINT8* params);

/*
 * Non-volatile memory functionality
 */
PNIO_VOID PNIOext_nv_data_init(PNIO_UINT8* params);
PNIO_VOID PNIOext_nv_data_clear(PNIO_UINT8* params);
PNIO_VOID PNIOext_nv_data_store(PNIO_UINT8* params);
PNIO_VOID PNIOext_im_data_store(PNIO_UINT8* params);

/*
 * IM data handling
 */
PNIO_VOID PNIOext_im_write_rsp(PNIO_UINT8* params);
PNIO_VOID PNIOext_im_read_rsp(PNIO_UINT8* params);

#endif /* (1 == IOD_USED_WITH_XHIF_HOST) */
#endif /* APPLICATION_APP_COMMON_PNUSR_XHIF_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
