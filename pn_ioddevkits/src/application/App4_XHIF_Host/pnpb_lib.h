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
/*  F i l e               &F: pnpb_lib.h                                :F&  */
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
* @file     Pnpb_lib.h
* @brief    Functions for XHIF - library for Ertec
*
* XHIF functionality allows user to use ERTEC Devkit only for PN-stack functionalities
* and to realize user functionality on other device. The other device have to upload
* firmware to ERTEC Devkit as a binary and then communicate data.. This can be realized
* via XHIF memory interface.
*/
#ifndef APP1_STANDART_PNPB_LIB_H_
#define APP1_STANDART_PNPB_LIB_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include "pnio_types.h"
#include <semaphore.h>
#include "iod_cfg.h"
#include <pthread.h>
#include "nv_data.h"

#define DEMO_RECORD  "ABCDEFGH"


// *------ IOD API defines  ----*
#define PNIO_FALSE                  0
#define PNIO_TRUE                   1

// *-----------------------------------------------
// * IOD API error Numbers
// *-----------------------------------------------
#define PNIO_NOT_OK                 0
#define PNIO_OK                     1

#define IOD_INCLUDE_IM5                 1

#define EXAMPL_DEV_CONFIG_VERSION 4

#define MODULE_ID_DAP_DR                 0x01       // DAP 1, MRP, S2 redundancy with DR (CiR), IRT, IsoM
#define MODULE_ID_DAP_NO_IRT_ISOM        0x02       // DAP 2, MRP, S2 redundancy with DR
#define MODULE_ID_DAP_NO_S2_DR           0x03       // DAP 3, MRP, IRT, IsoM
#define MODULE_ID_DAP_2PORT_MRP          0x04       // DAP 4, MRP
#define MODULE_ID_DAP_POF                0x05       // DAP 5, including physical device for fiber optic (POF)

#ifndef IOD_CFG_PDEV_NUMOF_PORTS
    #error "IOD_CFG_PDEV_NUMOF_PORTS is not defined"
#endif
#if ((IOD_INCLUDE_S2_REDUNDANCY == 1) && (IOD_INCLUDE_DR == 1))
    #if (IOD_INCLUDE_MRPD == 1)
        #define MODULE_ID_DAP          MODULE_ID_DAP_DR
    #else
    	#define MODULE_ID_DAP		   MODULE_ID_DAP_NO_IRT_ISOM
    #endif
#else
#if (IOD_INCLUDE_POF == 0)
    #if (IOD_INCLUDE_MRPD == 1)
        #define MODULE_ID_DAP          MODULE_ID_DAP_NO_S2_DR
    #else
        #define MODULE_ID_DAP          MODULE_ID_DAP_2PORT_MRP
    #endif
#else
        #define MODULE_ID_DAP          MODULE_ID_DAP_POF
#endif
#endif

#define SUBMOD_ID_DEFAULT      1       /* we use only one ID for all submodules here, except PDEV ports */
#define SUBMOD_ID_PDEV_IF      2       /* ID for PDEV interface submodule */
#define SUBMOD_ID_PDEV_PORT    3       /* ID for PDEV port submodules */

#define MOD_ID_CNT                  13
#define SUB_ID_CNT                  1
#define SUBMODULE_ID_1              1

#define IO_MODULE_1_BYTE_INOUT      0x20    /* 1 byte I + 1 byte O */
#define IO_MODULE_1_BYTE_IN         0x21    /* 1 byte I */
#define IO_MODULE_1_BYTE_OUT        0x22    /* 1 byte O */
#define IO_MODULE_64_BYTE_INOUT     0x29    /* 64 byte I */
#define IO_MODULE_64_BYTE_IN        0x30    /* 64 byte I */
#define IO_MODULE_64_BYTE_OUT       0x31    /* 64 byte O */
#define IO_MODULE_64_BYTE_IN_IRT    0x50    /* 64 byte I, synchronized application (IRT only) */
#define IO_MODULE_64_BYTE_OUT_IRT   0x51    /* 64 byte O, synchronized application (IRT only) */
#define IO_MODULE_1_BYTE_IN_IRT     0x52    // 1 byte I, synchronized application (IRT only)
#define IO_MODULE_1_BYTE_OUT_IRT    0x53    // 1 byte O, synchronized application (IRT only)
#define IO_MODULE_MULTISUBSLOT      0x60    /* 1 byte I + 1 byte I + 1 byte O + 1 byte O */
#define IO_MODULE_250_BYTE_INOUT    0x2D
#define IO_MODULE_250_BYTE_IN       0x2E
#define IO_MODULE_250_BYTE_OUT      0x2F

#define EXAMPL_MOD_ID_SLOT1             IO_MODULE_64_BYTE_IN        /* standard input module, usable in RT or IRT */
#define EXAMPL_MOD_ID_SLOT2             IO_MODULE_64_BYTE_OUT       /* standard output module, usable in RT or IRT */

#define DATE_VERSION_STR       "2018-05-22 08:00"


/* *** devive hardware version *** */
#define DEVKIT_HW_REVISION_ERTEC200P      0x0001                /* hardware revision, used in IM0 and annotation string (for EPM) */
#define DEVKIT_HW_REVISION_ERTEC200P2     0x0002                /* hardware revision, used in IM0 and annotation string (for EPM) */
#define DEVKIT_HW_REVISION                DEVKIT_HW_REVISION_ERTEC200P2



#define PNIO_SUPPORT_IM1234   0x1e
#define PNIO_SUPPORT_IM12345  0x3e


/* XHIF INTERFACE SYNCHRONIZATION DATA - have to be same in Ertec firmware */
#define PNPB_XHIF_NUMOF_SLOTS                           68  /* 4 Slots for Head, Node and 2 ports, 64 slots for submodules */
#define PNPB_XHIF_NUMOF_SUBSLOTS                        6
#define PNPB_XHIF_MAX_NUM_OF_SUBMODULES                 ( (PNPB_XHIF_NUMOF_SLOTS) * (PNPB_XHIF_NUMOF_SUBSLOTS) )
#define PNPB_XHIF_MAX_NUM_OF_AR                         (IOD_CFG_NUMOF_IO_AR + IOD_CFG_NUMOF_DEV_ACCESS_AR)

#define PNPB_XHIF_NUMOF_BYTES_PER_SUBSLOT               IOD_CFG_MAX_IO_NET_LENGTH_PER_SUBSLOT
#define PNPB_XHIF_NUMOF_BYTES_PER_ACYCLIC_TELEGRAM      1024

/* Number of records available in ring buffers*/
#define PNPB_XHIF_SIZE_OF_BUFFER_FOR_CYCLIC             470
#define PNPB_XHIF_SIZE_OF_BUFFER_FOR_ACYC               250

/*one page of XHIF - set according to jumper configuration*/
#define PNPB_XHIF_PAGE_SIZE                     0x20000     /*128kB*/
#define PNPB_XHIF_STRT                          0x00        /*where to place mapped image, NULL = chosen by kernel*/
#define PNPB_XHIF_SIZE                          0x100000    /*size of whole memory space mapped via XHIF - 1MB*/

/*Offsets in /dev/mem - dependant on registers GPMC settings*/
#define PNPB_XHIF_DATA_OFFSET                   0x01000000  /*data via XHIF*/
#define PNPB_XHIF_SET_REG_OFFSET                0x02000000  /*setting of XHIF pages placement*/
#define PNPB_XHIF_SET_REG_SIZE                  0x80        /*Size of registers for XHIF settings*/

/* page offsets for computing address in dev/mem */
#define PNPB_XHIF_OFFSET_PAGE_0                 (0x00)
#define PNPB_XHIF_OFFSET_PAGE_1                 (PNPB_XHIF_PAGE_SIZE)
#define PNPB_XHIF_OFFSET_PAGE_2                 (2 * PNPB_XHIF_PAGE_SIZE)
#define PNPB_XHIF_OFFSET_PAGE_3                 (3 * PNPB_XHIF_PAGE_SIZE)
#define PNPB_XHIF_OFFSET_PAGE_4                 (4 * PNPB_XHIF_PAGE_SIZE)
#define PNPB_XHIF_OFFSET_PAGE_5                 (5 * PNPB_XHIF_PAGE_SIZE)
#define PNPB_XHIF_OFFSET_PAGE_6                 (6 * PNPB_XHIF_PAGE_SIZE)
#define PNPB_XHIF_OFFSET_PAGE_7                 (7 * PNPB_XHIF_PAGE_SIZE)

/* Used register offset in Ertec */
#define PNPB_ERTEC_REG_SDRAM_CONFIG             0x08

/* Used settings of Ertec */
#define PNPB_ERTEC_SDRAM_CONFIG_VAL             0x2522	/* 10 Columns, 13 Rows, 2 Pages */


#define READ32(_addr)           ((unsigned int)(*((unsigned int *)(_addr))))
#define READ16(_addr)           ((unsigned short)(*((unsigned short *)(_addr))))
#define WRITE32(_addr, _data)   ((*((unsigned int *)(_addr))) = ((unsigned int)(_data)))
#define WRITE16(_addr, _data)   ((*((unsigned short *)(_addr))) = ((unsigned short)(_data)))

#define PNPB_UNUSED_ARG(arg_)   {arg_ = arg_;}

#define PNPB_FALSE              0
#define PNPB_TRUE               1
#define PNPB_NULL               (void*) 0
#define PNPB_NOT_OK             1
#define PNPB_OK                 0

#define PNPB_XHIF_DIRECTION_NO_DATA     0x00
#define PNPB_XHIF_DIRECTION_IN          0x01
#define PNPB_XHIF_DIRECTION_OUT         0x02
#define PNPB_XHIF_DIRECTION_INOUT       0x03

/* Configuration of Ertec Devkit - have to be same as in Devkit firmware */
#define IOD_CFG_MAX_NUMOF_SUBSLOTS          (IOD_CFG_MAX_SLOT_NUMBER + 2 + IOD_CFG_PDEV_NUMOF_PORTS)  // total max. subslots (Modules+Head+Ports)

/* Dummy values, actual values will be filled based on version from Ertec version_dk.h transfered by XHIF */
#define DEVKIT_VERSION_PREFIX   0
#define DEVKIT_VERSION_HH       0
#define DEVKIT_VERSION_H        0
#define DEVKIT_VERSION_L        0
#define DEVKIT_VERSION_LL       0

/*
 * Default values for alarm
 */

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

// *=======================================================
// *  public data
// *=======================================================
#if  EXAMPLE_IPAR_SERVER
   // ***** EXAMPLE IPAR:  upload&storage alarm ********
   PNIO_UINT32  Example_iParData [IOD_CFG_MAX_SLOT_NUMBER + 2 + IOD_CFG_PDEV_NUMOF_PORTS][IOD_CFG_MAX_NUMOF_SUBSL_PER_SLOT]; // 4 byte example data per subslot
#endif

// ***** EXAMPLE 1:  process alarm ********
#define EXAMPL_1_SLOTNUM		        1                           // slot number
#define EXAMPL_1_SUBSLOTNUM	            1                           // subslot number
#define EXAMPL_1_DIAG_TAG		        0x2345					    // free defined by the user
#define EXAMPL_1_USER_STRUCT_IDENT      0x1234                      // user defined (0...0x7fff)
#define PNIO_XHIF_DEFAULT_API           0                           // default application process identifier
#define PNDV_AL_STAL_INFO_LEN           4                           // dummy value 

// ***** EXAMPLE 2:  channel diagnostic alarm ********
#define EXAMPL_2_SLOTNUM		        1                           // slot number
#define EXAMPL_2_SUBSLOTNUM	            1                           // subslot number
#define EXAMPL_2_CHANNELNUM	            2                           // channel number
#define EXAMPL_2_CHAN_ERR_TYPE          6							// DIAG_CHAN_ERR_LINE_BREAK, see IEC 61158
#define EXAMPL_2_DIAG_TAG		        0x2346					    // free defined by the user
#define EXAMPL_2_USER_STRUCT_IDENT      0x1234                      // user defined (0...0x7fff)
#define EXAMPL_2_IO_DIRECTION           0x01        				// submodule with input data - input/output/input-output
#define EXAMPL_2_IO_DATA_TYPE           4     						// data type 8 Bit - data type (..., byte, word,.. )
#define EXAMPL_2_IO_MAINT_REQ           0        					// maintenance required
#define EXAMPL_2_IO_MAINT_DEM           0     						// maintenance demanded

// ***** EXAMPLE 3:  extended channel diagnostic alarm ********
#define EXAMPL_3_SLOTNUM		        2                           // slot number
#define EXAMPL_3_SUBSLOTNUM	            1                           // subslot number
#define EXAMPL_3_CHANNELNUM	            3                           // channel number
#define EXAMPL_3_CHAN_ERR_TYPE          6							// DIAG_CHAN_ERR_LINE_BREAK, see IEC 61158
#define EXAMPL_3_EXT_CHAN_ERR_TYPE      0x0000                  	// see IEC 61158
#define EXAMPL_3_EXT_CHAN_ADD_VALUE     0x00000000              	// see IEC 61158
#define EXAMPL_3_DIAG_TAG		        0x2347					    // free defined by the user
#define EXAMPL_3_USER_STRUCT_IDENT      0x1234                      // user defined (0...0x7fff)
#define EXAMPL_3_IO_DIRECTION           0x02       					// submodule with output data - input/output/input-output
#define EXAMPL_3_IO_DATA_TYPE           4     						// data type 8 Bit - data type (..., byte, word,.. )
#define EXAMPL_3_IO_MAINT_REQ           0        					// maintenance required
#define EXAMPL_3_IO_MAINT_DEM           0     						// maintenance demanded

// ***** EXAMPLE 4:  generic diagnostic alarm ********
#define EXAMPL_4_SLOTNUM		        0                           // slot number
#define EXAMPL_4_SUBSLOTNUM	            1                           // subslot number
#define EXAMPL_4_CHANNELNUM	            4                           // channel number
#define EXAMPL_4_DIAG_TAG		        0x2348					    // free defined by the user
#define EXAMPL_4_USER_STRUCT_IDENT      0x1234                      // user defined (0...0x7fff)
#define EXAMPL_4_IO_DIRECTION           0x01        				// submodule with input data - input/output/input-output
#define EXAMPL_4_IO_DATA_TYPE           4     						// data type 8 Bit - data type (..., byte, word,.. )
#define EXAMPL_4_IO_MAINT_REQ           0        					// maintenance required
#define EXAMPL_4_IO_MAINT_DEM           0     						// maintenance demanded

// ***** EXAMPLE 5:  PNIOext_ret_of_sub_alarm_send ********
#define EXAMPL_5_SLOTNUM		        0                           // slot number
#define EXAMPL_5_SUBSLOTNUM	            1                           // subslot number
#define EXAMPL_5_DIAG_TAG				0x2349						// free defined by the user

// ***** Plug/Pull ********
#define PNIO_S_BAD  0x00		/* io provider or consumer state = BAD  */
#define PNIO_S_GOOD 0x80      	/* io provider or consumer state = GOOD */
#define PNIO_IOXS 	PNIO_UINT8  /* shouldn't be ENUM, as other values can occur from IOC */

// *-------- define Log message types ---------------
#define PNIO_LOG_CHAT           9       // chat
#define PNIO_LOG_CHAT_HIGH      8       // chat, level high
#define PNIO_LOG_NOTE           7       // note, level normal
#define PNIO_LOG_NOTE_HIGH      6       // note, level high
#define PNIO_LOG_WARNING        5       // warning
#define PNIO_LOG_WARNING_HIGH   4       // warning, level high (unexpected situation)
#define PNIO_LOG_ERROR          3       // error, tradeable
#define PNIO_LOG_ERROR_FATAL    2       // error, not tradeable (cannot be switched off)
#define PNIO_LOG_DEACTIVATED    1       // logging off
/* Submodules for the use of trace system */
#define TRACE_SUBSYS_TRACE             0
#define TRACE_SUBSYS_ACP_LOWER         1
#define TRACE_SUBSYS_CLRPC_LOWER      10
#define TRACE_SUBSYS_CM_LOWER      	  20
#define TRACE_SUBSYS_DCP_LOWER        40
#define TRACE_SUBSYS_GSY_LOWER        50
#define TRACE_SUBSYS_NARE_LOWER       70
#define TRACE_SUBSYS_LLDP_LOWER       80
#define TRACE_SUBSYS_OHA_LOWER        90
#define TRACE_SUBSYS_EDDP_LOWER      100
#define TRACE_SUBSYS_MRP_LOWER       120
#define TRACE_SUBSYS_SOCK_LOWER      130
#define TRACE_SUBSYS_POF_LOWER       140
#define TRACE_SUBSYS_IOD_LOWER       150
#define TRACE_SUBSYS_PNPB_LOWER      160
#define TRACE_SUBSYS_PNDV_LOWER      170
#define TRACE_SUBSYS_TCIP_LOWER      180
#define TRACE_SUBSYS_APPL_LOWER      190
#define TRACE_SUBSYS_TSKMSG_LOWER    200

#define TRACE_SUBSYS_NUM        213     

// *------ defines for channel properties.specifier --------
typedef enum
{
    DIAG_CHANPROP_SPEC_ERR_APP			= 1,	// new error appears
    DIAG_CHANPROP_SPEC_ERR_DISAPP		= 2,	// error disappears, no more error
    DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE	= 3	    // error disappears, but other errors remain
} DIAG_CHANPROP_SPEC_ERR;

#define PNPB_MAX_DEVTYPE_LEN            256


#ifndef PNIO_IOXS_USE_ENUM_TYPE
    #define PNIO_S_BAD  0x00        /* io provider or consumer state = BAD  */
    #define PNIO_S_GOOD 0x80        /* io provider or consumer state = GOOD */
    #define PNIO_IOXS   PNIO_UINT8  /* shouldn't be ENUM, as other values can occur from IOC */
#endif

// *--------------------------------------
// * alignment orders und endian format
// *--------------------------------------
#define ATTR_PNIO_PACKED          __attribute__((packed))
#define ATTR_PNIO_PACKED_PRE      /* nothing */

typedef  ATTR_PNIO_PACKED_PRE struct
{
    PNIO_UINT16      Type;              // block type, big endian format
    PNIO_UINT16      Len;               // block length
    PNIO_UINT16      Version;           // block version, , big endian format
}  ATTR_PNIO_PACKED REC_IO_BLOCKHDR;    // more details see pnio spec

// *-----------------------------------------------------------
// *  annotation
// *-----------------------------------------------------------
#define MAX_DEVICE_TYPE_LENGTH      240  // value must be lower than DCP_MAX_STATION_TYPE_LEN
#define MAX_ORDER_ID_LENGTH         20
#define MAX_SERIAL_NUMBER_LENGTH    16

typedef struct
{
    PNIO_INT8   DeviceType [MAX_DEVICE_TYPE_LENGTH + 1];
    PNIO_INT8   OrderId [MAX_ORDER_ID_LENGTH + 1];
    PNIO_INT8   SerialNumber [MAX_SERIAL_NUMBER_LENGTH];
    PNIO_UINT16 HwRevision;
    PNIO_INT8   SwRevisionPrefix;
    PNIO_UINT16 SwRevision1;
    PNIO_UINT16 SwRevision2;
    PNIO_UINT16 SwRevision3;

} PNIO_ANNOTATION;


// **********************************************
// * block header, see IEC61158-6  "BlockHeader"
// **********************************************
#define BLOCKVERSION    0x0100

/* IO module parameters received by memory interface will be available to user in this structure */
typedef volatile struct pnpb_subslot_params
{
    PNIO_UINT32 Slot;
    PNIO_UINT32 Subslot;
    PNIO_UINT32 ModId;
    PNIO_UINT8  IOpS;
    PNIO_UINT8  IOcS;
    PNIO_UINT8  InData_size;
    PNIO_UINT8  OutData_size;
    PNIO_UINT8  Direction;
    PNIO_UINT8  Update;
}pnpb_subslot_params;
typedef pnpb_subslot_params * pnpb_subslot_params_ptr;

typedef volatile struct  pnpb_device_params
{
    PNIO_UINT32 NoOfSubmodules;
}pnpb_device_params;

/* IO module data */
typedef struct pnpb_subslot_data
{
    PNIO_UINT8 Data[PNPB_XHIF_NUMOF_BYTES_PER_SUBSLOT];
}pnpb_subslot_data;
typedef pnpb_subslot_data * pnpb_subslot_data_ptr;

/* IO module interface for user */
pnpb_subslot_data_ptr      pnpb_submodule_IO_data [PNPB_XHIF_MAX_NUM_OF_SUBMODULES];
volatile pnpb_subslot_params_ptr    pnpb_submodule_params  [PNPB_XHIF_MAX_NUM_OF_SUBMODULES];
PNIO_UINT32                pnpb_ar_apdu_status    [PNPB_XHIF_MAX_NUM_OF_AR];

pnpb_device_params         pnpb_dev_params;

/* Structures with parameters received from Ertec */

#define PNIO_IO_PROP_ENUM       PNIO_UINT8  /* shouldn't be ENUM, as bitwise combinations are used */

typedef struct
{
    PNIO_UINT16       data_offset;          /*  offset of IOData */
    PNIO_UINT16       data_length;          /*  expected length of IOData */
    PNIO_UINT16       iocs_offset;          /*  offset of IOCS */
    PNIO_UINT16       iops_offset;          /*  offset of IOPS */

    PNIO_UINT8        iops_val;             /*  iops value (for internal use only) */
    PNIO_UINT8        iocs_val;             /*  iops value (for internal use only) */
} PNPB_OWNSHIP;

typedef enum
{
    ELEM_FREE     = 0,
    ELEM_OCCUPIED = 1
} EXP_ENTITY_STATE;

typedef struct
{
    PNIO_UINT32         ApiNum;             /* [in] PNIO subslot number (0...0xffff) */
    PNIO_UINT32         SlotNum;            /* [in] PNIO subslot number (0...0xffff) */
    PNIO_UINT32         SubNum;             /* [in] subslot handle = table index in SubslotArray */
    PNIO_UINT32         ModIdent;           /* [in] module ident number (from GSDML file) */
    PNIO_UINT32         SubIdent;           /* [in] submodule ident number (from GSDML file) */
    PNIO_UINT32         ModProp;            /* [in] module properties */
    PNIO_UINT32         SubProp;            /* [in] submodule properties */
    PNIO_UINT32         EntityIndSetCfg;    /* [in] entity index in set_cfg of Perif-IF */
    PNIO_UINT32         EntityState;        /* [in] ELEM_FREE / ELEM_OCCUPIED */
    PNPB_OWNSHIP        In;                 /* [in] ownership information for input data */
    PNPB_OWNSHIP        Out;                /* [in] ownership information for output data */
    PNIO_UINT8          isPlugged;          /* [in] subslot includes any submodule (right or wrong...) */
    PNIO_IO_PROP_ENUM   IoProp;             /* [in] IO properties (in/out/in+out/noIO data). Field is valid only, if submodule is already plugged (else:IoProp = 0) */
    PNIO_UINT16         OwnSessionKey;      /* [in, out] ind: the owner; rsp: 0 (reject ownership) or unchanged (take ownership) */
    PNIO_BOOL           IsWrongSubmod;      /* [in, out] ind: PNIO_FALSE (submodule is OK); rsp: PNIO_TRUE (wrong submodule) or unchanged */
    PNIO_BOOL           ParamEndValid;      /* [in] PNIO_TRUE ParamEnd event has been executed for this submodule */
} PNIO_EXP_SUB;

typedef struct _PNIO_EXP
{
    /* device identification */
    PNIO_BOOL       ArValid;                               /* AR is valid  (ownership indication has been processed or is under work) */
    PNIO_BOOL       FirstParamEndValid;                    /* param end event has been signaled for this submodule */
    PNIO_BOOL       Rdy4InpUpdateValid;                    /* input data exchange is possible (ready for input update event has been signaled) */
    PNIO_UINT32     NumOfPluggedSub;                       /* current number of pluggedsubslots */
    PNIO_UINT32     IocrLen;                               /* length of IOCR (data + IOxS + APDU state) */
    PNIO_UINT32     LastApduStat;                          /* value of last remote Apdu state from PNIO_initiate_data_read() */
    PNIO_EXP_SUB    Sub [IOD_CFG_MAX_NUMOF_SUBSLOTS];
    PNIO_BOOL       IoUpdatePending;
    PNIO_UINT16     ArType;
} PNIO_EXP;

typedef enum
    {
        PNIO_AR_STARTUP   = 0x0001,  /* Ready for input update comes first during AR startup        (use case first valid input update after AR has been started) */
        PNIO_AR_INDATA    = 0x0002   /* Ready for input update comes after AR startup has finished  (use case replug a submodule during indata) */
    } PNIO_INP_UPDATE_STATE;



#define PNIO_ARFSU_ENABLED      1
#define PNIO_ARFSU_DISABLED     0
#define PNIO_ARFSU_CHANGED      1
#define PNIO_ARFSU_NOT_CHANGED  0


#define PNIO_IM0_NOTHING     0x00   /* this submodule does not contain I&M data                 */
#define PNIO_IM0_SUBMODULE   0x01   /* this submodule contains I&M data                         */
#define PNIO_IM0_MODULE      0x02   /* optional modifier: this submodule stands for the module  */
#define PNIO_IM0_DEVICE      0x04   /* optional modifier: this submodule stands for the device  */
#define PNIO_IM0_SUPP_ENUM   PNIO_UINT8  /* shouldn't be ENUM, as bitwise combinations are used */

#define IM0_ORDER_ID_LEN       20                   /* length of IM0-Order ID */
#define IM0_SER_NUM_LEN        16                   /* length of IM0-serial number */

typedef  ATTR_PNIO_PACKED_PRE struct
{
  PNIO_UINT8        srp;    // software revision prefix
  PNIO_UINT8        fe;     // IM_SW_Revision_Funct_Enhancement
  PNIO_UINT8        bf;     // IM_SW_Revision_Bug_Fix
  PNIO_UINT8        ic;     // IM_SW_Revision_Internal_Change
}  ATTR_PNIO_PACKED IM0_SW_REVISION;

typedef ATTR_PNIO_PACKED_PRE struct
{
   PNIO_UINT16          VendorId;                   // see pnio spec, VendorIDHigh, VendorIDLow
   PNIO_UINT8           OrderId [IM0_ORDER_ID_LEN]; // see pnio spec, Order_ID, visible string
   PNIO_UINT8           SerNum  [IM0_SER_NUM_LEN];  // see pnio spec, IM_Serial_Number, visible string
   PNIO_UINT16          HwRevision;                 // see pnio spec, IM_Hardware_Revision
   IM0_SW_REVISION      SwRevision;                 // see pnio spec, IM_Software_Revision
   PNIO_UINT16          Revcnt;                     // see pnio spec, IM_Revision_Counter, notifies a hw modification
   PNIO_UINT16          ProfId;                     // see pnio spec, IM_Profile_ID, see Profile Guideline I&M functions
   PNIO_UINT16          ProfSpecTyp;                // see pnio spec, IM_Profile_Spec_Type
   PNIO_UINT8           VersMaj;                    // see pnio spec, IM_Version major
   PNIO_UINT8           VersMin;                    // see pnio spec, IM_Version minor
   PNIO_UINT16          ImXSupported;               // see pnio spec, IM_Supported  (bits for IM1..IM15)
}  ATTR_PNIO_PACKED  IM0_DATA;

typedef ATTR_PNIO_PACKED_PRE struct
{
   PNIO_UINT8           TagFunction [32];           // see pnio spec, IM_Tag_Function
   PNIO_UINT8           TagLocation [22];           // see pnio spec, IM_TAG_Location
}  ATTR_PNIO_PACKED  IM1_DATA;

typedef ATTR_PNIO_PACKED_PRE struct
{
   PNIO_UINT8           InstallationDate [16];      // see pnio spec, IM_Date
   //PNIO_UINT8           Reserved         [38];
}  ATTR_PNIO_PACKED  IM2_DATA;

typedef ATTR_PNIO_PACKED_PRE struct
{
   PNIO_UINT8           Descriptor [54];            // see pnio spec, IM_Descriptor
}  ATTR_PNIO_PACKED  IM3_DATA;

typedef ATTR_PNIO_PACKED_PRE struct
{
   PNIO_UINT8           Signature  [54];            // see pnio spec, IM_Signature
}  ATTR_PNIO_PACKED  IM4_DATA;

typedef ATTR_PNIO_PACKED_PRE struct
{
   REC_IO_BLOCKHDR      BlockHeader;                // see pnio spec, BlockHeader
   IM0_DATA             IM0;                        // see pnio spec, data block
}  ATTR_PNIO_PACKED  IM0_STRUCT;

typedef ATTR_PNIO_PACKED_PRE struct
{
   REC_IO_BLOCKHDR      BlockHeader;                // see pnio spec, BlockHeader
   IM1_DATA             IM1;                        // see pnio spec, data block
}  ATTR_PNIO_PACKED  IM1_STRUCT;

typedef ATTR_PNIO_PACKED_PRE struct
{
   REC_IO_BLOCKHDR      BlockHeader;                // see pnio spec, BlockHeader
   IM2_DATA             IM2;                        // see pnio spec, data block
}  ATTR_PNIO_PACKED  IM2_STRUCT;

typedef ATTR_PNIO_PACKED_PRE struct
{
   REC_IO_BLOCKHDR      BlockHeader;                // see pnio spec, BlockHeader
   IM3_DATA             IM3;                        // see pnio spec, data block
}  ATTR_PNIO_PACKED  IM3_STRUCT;

typedef ATTR_PNIO_PACKED_PRE struct
{
   REC_IO_BLOCKHDR      BlockHeader;                // see pnio spec, BlockHeader
   IM4_DATA             IM4;                        // see pnio spec, data block
}  ATTR_PNIO_PACKED  IM4_STRUCT;

typedef struct
{
    PNIO_UINT32         Api;        /* api number */
    PNIO_UINT32         Slot;       /* slot number (1..0x7fff) */
    PNIO_UINT32         Subslot;    /* subslot number (1..0x7fff) */
    PNIO_UINT32         ModId;      /* module ID */
    PNIO_UINT32         SubId;      /* submodule ID */
    PNIO_UINT32         InDatLen;   /* input data length */
    PNIO_UINT32         OutDatLen;  /* output data length */
    PNIO_IM0_SUPP_ENUM  Im0Support; /* submodule has own I&M0 data, values see PNIO_IM0_SUPP_ENUM */
}   ATTR_PNIO_PACKED PNIO_SUB_LIST_ENTRY;

typedef struct
{
    PNIO_UINT32      Api;                        /* api number */
    PNIO_UINT32      Slot;                       /* slot number (1..0x7fff) */
    PNIO_UINT32      Subslot;                    /* subslot number (1..0x7fff) */
    IM0_DATA         Im0Dat;
}   ATTR_PNIO_PACKED PNIO_IM0_LIST_ENTRY;

typedef struct PNUSR_DEV_EXP_INSTANCE
{
    PNIO_UINT16     VendorId;
    PNIO_UINT16     DeviceId;
    PNIO_UINT32     MaxNumOfSubslots;
    PNIO_UINT32     MaxNumOfBytesPerSubslot;
    PNIO_UINT32     DevTypeLen;
}   PNUSR_DEV_EXP_INSTANCE;

typedef struct PNIO_DEVICESETUP_PRM
{
    PNIO_UINT32            NumOfSublistEntries;
    PNIO_UINT32            NumOfIm0ListEntries;

}   PNIO_DEVICESETUP_PRM;

typedef struct PNIO_ASYNC_ERROR_PRM
{
    PNIO_UINT32 Type;
    PNIO_UINT32 ErrorCode;
} PNIO_ASYNC_ERROR_PRM;

typedef enum
{
    PNIO_ALM_PROC = 1,              /* 0 is not used */
	PNIO_ALM_STATUS,
    PNIO_ALM_CHAN_DIAG,
    PNIO_ALM_EXT_CHAN_DIAG,
    PNIO_ALM_GEN_DIAG,
    PNIO_ALM_RET_OF_SUB,
    PNIO_ALM_UPLOAD_RETRIEVAL,
    PNIO_ALM_NUMOF_ENTRIES          /* must be last element in this enum struct !! */
}   PNIO_ALARM_TYPE;


typedef enum
{
    PNIO_ADDR_LOG = 0,      /* logical addressing, not supported */
    PNIO_ADDR_GEO = 1,      /* geographic addressing (slotnumber, subslot-number) */
    PNIO_ADDR_HND = 2       /* Handle addressing (slothandle, subslot-handle) for faster access than GEO */
}   PNIO_ADDR_TYPE;


typedef struct PNIO_FW_VERSION
{
    PNIO_UINT8 VerPrefix;
    PNIO_UINT8 VerHh;
    PNIO_UINT8 VerH;
    PNIO_UINT8 VerL;
    PNIO_UINT8 VerLl;
}PNIO_FW_VERSION;

typedef struct
{
    PNIO_ADDR_TYPE  Type;
    union
    {
        PNIO_UINT32 Addr; /* logical address */
        struct
        {
            PNIO_UINT32 Slot;
            PNIO_UINT32 Subslot;
        } Geo1;         /* geographical address */
    } ad;       /* dont care element */
}   PNIO_DEV_ADDR;

#define Geo     ad.Geo1


/* PNIO_ERR_STAT:
 * a) The first four parameters represent the PNIO Status.
 *    For details refer to IEC 61158-6 chapter 3.2.10.6 and 3.2.11.66
 * b) The last two parameters correspond to AdditionalValue1 and AdditionalValue2,
 *    see IEC 61158-6 chapter 3.2.11.50:
 *    The values shall contain additional user information within negative responses.
 *    The value zero indicates no further information.
 *    For positive read responses, the value 1 of the field AdditionalValue1 indicates
 *    that the Record Data Object contains more data than have been read.
 */
typedef struct
{
    PNIO_UINT8     ErrCode;   /* ErrorCode: Most significant word, most significant byte of PNIO Status */
    PNIO_UINT8     ErrDecode; /* ErrorDecode: Most significant word, least significant byte of PNIO Status */
    PNIO_UINT8     ErrCode1;  /* ErrorDecode: Least significant word, most significant byte of PNIO Status */
    PNIO_UINT8     ErrCode2;  /* ErrorCode2: Least significant word, least significant byte of PNIO Status */
    PNIO_UINT16    AddValue1;
    PNIO_UINT16    AddValue2;
}PNIO_ERR_STAT;

/* *------- reason code for event indication of AR-Offline, AR-Abort -----  */
/* definition has to match cm_ar_reason_enum (cm_usr.h) */
typedef enum
{
    PNIO_AR_REASON_NONE         =  0,
    /***/
    PNIO_AR_REASON_1            =  1, /* reserved */
    PNIO_AR_REASON_2            =  2, /* reserved */
    PNIO_AR_REASON_MEM          =  3, /* out of mem */
    PNIO_AR_REASON_FRAME        =  4, /* add provider or consumer failed */
    PNIO_AR_REASON_MISS         =  5, /* miss (consumer) */
    PNIO_AR_REASON_TIMER        =  6, /* cmi timeout */
    PNIO_AR_REASON_ALARM        =  7, /* alarm-open failed */
    PNIO_AR_REASON_ALSND        =  8, /* alarm-send.cnf(-) */
    PNIO_AR_REASON_ALACK        =  9, /* alarm-ack-send.cnf(-) */
    PNIO_AR_REASON_ALLEN        = 10, /* alarm-data too long */
    PNIO_AR_REASON_ASRT         = 11, /* alarm.ind(err) */
    PNIO_AR_REASON_RPC          = 12, /* rpc-client call.cnf(-) */
    PNIO_AR_REASON_ABORT        = 13, /* ar-abort.req */
    PNIO_AR_REASON_RERUN        = 14, /* re-run aborts existing */
    PNIO_AR_REASON_REL          = 15, /* got release.ind */
    PNIO_AR_REASON_PAS          = 16, /* device passivated */
    PNIO_AR_REASON_RMV          = 17, /* device / AR removed */
    PNIO_AR_REASON_PROT         = 18, /* protocol violation */
    PNIO_AR_REASON_NARE         = 19, /* NARE error */
    PNIO_AR_REASON_BIND         = 20, /* RPC-Bind error */
    PNIO_AR_REASON_CONNECT      = 21, /* RPC-Connect error */
    PNIO_AR_REASON_READ         = 22, /* RPC-Read error */
    PNIO_AR_REASON_WRITE        = 23, /* RPC-Write error */
    PNIO_AR_REASON_CONTROL      = 24, /* RPC-Control error */
    PNIO_AR_REASON_25           = 25, /* reserved (formerly: pull or plug in forbidden window) */
    PNIO_AR_REASON_26           = 26, /* reserved (formerly: AP removed) */
    PNIO_AR_REASON_LNK_DOWN     = 27, /* link "down", for local purpose only */
    PNIO_AR_REASON_MMAC         = 28, /* could not register multicast-mac */
    PNIO_AR_REASON_SYNC         = 29, /* not synchronized (cannot start companion-ar) */
    PNIO_AR_REASON_TOPO         = 30, /* wrong topology (cannot start companion-ar) */
    PNIO_AR_REASON_DCP_NAME     = 31, /* dcp, station-name changed */
    PNIO_AR_REASON_DCP_RESET    = 32, /* dcp, reset to factory-settings */
    PNIO_AR_REASON_33           = 33, /* reserved (formerly: cannot start  companion-ar) */
    PNIO_AR_REASON_IRDATA       = 34, /* no irdata record yet */
    PNIO_AR_REASON_PDEV         = 35, /* ownership of physical device */
    PNIO_AR_REASON_LNK_MODE     = 36, /* link mode not full duplex */
    PNIO_AR_REASON_IPSUITE      = 37, /* IP-Suite [of the IOC] changed by means of DCP_set(IPParameter) or local engineering */
    PNIO_AR_REASON_RDHT         = 38, /* IOCARSR RDHT expired */
    PNIO_AR_REASON_PDEV_PRM     = 39, /* IOCARSR PDev, parametrization impossible */
    PNIO_AR_REASON_ARDY         = 40, /* Remote application timeout expired */
    /***/
    PNIO_AR_REASON_MAX
} PNIO_AR_REASON ;


typedef struct
{
    PNIO_UINT16    VendorId;                    /* Vendor ID */
    PNIO_UINT16    DeviceId;                    /* Device ID */
    PNIO_INT8*     pDevType;                    /* pointer to string with device type (zero terminated) */
    PNIO_UINT32    MaxNumOfSubslots;            /* maximum number of subslots, managable by PN Stack */
    PNIO_UINT32    MaxNumOfBytesPerSubslot;     /* maximum number of bytes per subslots, managable by PN Stack */
} PNUSR_DEVICE_INSTANCE;

PNIO_UINT32 NumOfAr;    /* number of running ARs */

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

PNPB_USR_STARTUP_STATE PnpbDeviceStartupState;
PNIO_FW_VERSION DeviceFwVersion;


PNIO_UINT32                     PnioLogDest;                    /* debug logging   0:none, 1:Console, 2:Tracebuffer */

#define PNPB_XHIF_ACYC_FIRST_HOST_ERTEC_TELEGRAM    128
#define PNPB_XHIF_ACYC_NUM_OF_HOST_ERTEC_TELEGRAMS  (PNPB_XHIF_ACYC_NUM_OF_TELEGRAMS - PNPB_XHIF_ACYC_FIRST_HOST_ERTEC_TELEGRAM)

/* Semaphore handles */
sem_t PnioDeviceReadySemId;
sem_t TracesSemId;

/* Mutex handles */
pthread_mutex_t gpmc_access_mutex;

/*
 * Storage for IP address, MAC address and device name
 */
extern PNIO_UINT8 pTmpIpSuite[12];
extern PNIO_UINT8 pTmpMacAddr[6];
extern PNIO_UINT8 pTmpDevName[256];
extern PNIO_UINT16 pTmpDevNameLen;

/* Thread cancellation flag */
#define PNPB_THREAD_TERMINATE  0
#define PNPB_THREAD_RUNNING    1
extern volatile int pnpb_keep_running;
extern volatile int acyc_gpio_running;
extern volatile int acyc_service_running;
extern volatile int cyc_gpio_running;
extern volatile int cyc_service_running;

/* Fatal error handling */
#define PNPB_LIB_FATAL                                                              \
fprintf(stderr, "Line %d, file %s (function %s)\n", __LINE__, __FILE__, __func__);  \
pnpb_lib_in_fatal();


PNIO_UINT32 PrintDevkitVersion (void);
PNIO_UINT32 OsKeyScan32 (PNIO_INT8* pText, PNIO_UINT32 InputBase);
PNIO_UINT32 OsKeyScanString(PNIO_INT8* pText, PNIO_UINT8* pStr, PNIO_UINT32 MaxLen);
PNIO_UINT8  OsGetChar (void);
PNIO_UINT64 OsGetUuidTime (void);

void MainAppl (void);

PNIO_VOID pnpb_scan_ip_address(PNIO_UINT8* ip_suite);
PNIO_VOID pnpb_scan_mac_address(PNIO_UINT8* mac_addr);

void PnUsr_cbf_iodapi_event_varinit (PNIO_SUB_LIST_ENTRY* pIoSubList,
                                     PNIO_UINT32          NumOfSubListEntries);
void PrintAllUsedIoData (void);
void PrintSubmodules (void);
void ChangeAllInputData (PNIO_INT8 DiffVal);
void UsrSetIops (PNIO_UINT32 SlotNum, PNIO_UINT32 SubslotNum, PNIO_UINT8 Iops);
PNIO_IOXS    PNIO_cbf_data_write
        (PNIO_DEV_ADDR  *pAddr,
         PNIO_UINT32    BufLen,
         PNIO_UINT8     *pBuffer,
         PNIO_IOXS      Iocs);
PNIO_IOXS     PNIO_cbf_data_read
        (PNIO_DEV_ADDR  *pAddr,
         PNIO_UINT32    BufLen,
         PNIO_UINT8*    pBuffer,
         PNIO_IOXS      Iops);
PNIO_IOXS    PNIO_cbf_data_write_IOxS_only
	    (PNIO_DEV_ADDR	*pAddr,
	     PNIO_IOXS	    Iocs);

PNIO_IOXS     PNIO_cbf_data_read_IOxS_only
	    (PNIO_DEV_ADDR	*pAddr,
	     PNIO_IOXS	    Iops
        );
PNIO_BOOL PnUsr_cbf_IoDatXch (void);
PNIO_VOID pnpb_lib_in_fatal();

PNIO_VOID InputAndPullSubmodule(PNIO_VOID);
PNIO_VOID InputAndPlugSubmodule( PNIO_VOID );

PNIO_VOID pnpb_lib_init();
PNIO_VOID pnpb_lib_deinit();
PNIO_VOID pnpb_lib_reboot();

#endif /* APP1_STANDART_PNPB_LIB_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
