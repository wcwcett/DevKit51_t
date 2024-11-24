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
/*  F i l e               &F: pniousrd.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  description of the pnio user interface. It contains the functions of the */
/*  pnio stack, which are called by the user application.                    */
/*                                                                           */
/*****************************************************************************/
#ifndef PNIOUSRD_H
#define PNIOUSRD_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version    What                                              */
/*  2017-04-18             function PNIO_diag_channel_remove and             */
/*                         PNIO_diag_channel_remove add parameter AlarmState */
/*---------------------------------------------------------------------------*/
#include "lsa_cfg.h"
#include "pnio_types.h"
#include "pnioerrx.h"
#include "iod_cfg.h"
#include "usrapp_cfg.h"
#include <ecos_ertec_bsp_spi_flash.h>
#include <ecos_ertec_bsp_ospi_flash.h>

// *----------------------------------------------------------------------------------------*
// *    defines
// *----------------------------------------------------------------------------------------*
#define PNIO_SINGLE_DEVICE_HNDL         1       // only single device supported in this version
#define PNIO_DEFAULT_API                0       // default API is always 0

// **** IOCR Properties ****
typedef enum
{
    PNIO_IOCR_TYPE_INPUT               = 0x0001,
    PNIO_IOCR_TYPE_OUTPUT              = 0x0002,
    // r.f.u.   PNIO_IOCR_TYPE_MULTICAST_PROVIDER  = 0x0003,
    // r.f.u.   PNIO_IOCR_TYPE_MULTICAST_CONSUMER  = 0x0004
} PNIO_IOCR_TYPE_ENUM;


// **** submodule properties (io direction) ****
#ifndef PNIO_IO_PROP_USE_ENUM_TYPE
    #define PNIO_SUB_PROP_NO_DATA   0x00          /* submodule with no io data                           */
    #define PNIO_SUB_PROP_IN        0x01          /* submodule with input data                           */
    #define PNIO_SUB_PROP_OUT       0x02          /* submodule with output data                          */
    #define PNIO_SUB_PROP_IN_OUT    0x03          /* submodule with input + output data                  */
    #define PNIO_IO_PROP_ENUM       PNIO_UINT8    /* shouldn't be ENUM, as bitwise combinations are used */
#else
    typedef enum
    {
        PNIO_SUB_PROP_NO_DATA   = 0x0000,  // submodule with no io data
        PNIO_SUB_PROP_IN        = 0x0001,  // submodule with input data
        PNIO_SUB_PROP_OUT       = 0x0002,  // submodule with output data
        PNIO_SUB_PROP_IN_OUT    = 0x0003   // submodule with input + output data
    } PNIO_IO_PROP_ENUM;
#endif

/* SCRB Boot reg */
#define     MODE_SPI_0x03               0xe
#define     MODE_XHIF                   0xf

/*Switch off to use SPI flash, used macros*/
#ifdef BOARD_TYPE_STEP_3
    #define FLASH_INIT                                                                      init_ospi(  )
    #define FLASH_ERASE( _flash_strt_addr, _data_size, _pError )                            ospi_erase( _flash_strt_addr, _data_size )
    #define FLASH_PROGRAM( _flash_strt_addr, _transmit_addr_ptr, _data_size, _pError )      ospi_write_data( _flash_strt_addr, _transmit_addr_ptr, _data_size ) // TODO pError removed ?
    #define FLASH_READ( _flash_strt_addr, _transmit_addr_ptr, _data_size, _pError )         ospi_read_data( _flash_strt_addr, _transmit_addr_ptr, _data_size ) // TODO pError removed ?
    #define FLASH_VERIFY( _pFlash8, _pBuf8, _BufSize )                                      ospi_verify_write_data( _pFlash8, _pBuf8, _BufSize )
    #define FLASH_ERASE_CHECK( _flash_strt_addr, _data_size )                               ospi_erase_verify( _flash_strt_addr, _data_size )

    #define OSPI_NV_ADDRESS                                                                  0x6400000 //100. megabyte
    #define OSPI_NV_ADDRESS_SEC_OFFSET                                                       (0x1000)
    #define OSPI_NV_ADDRESS_SEC1                                                             (OSPI_NV_ADDRESS + OSPI_NV_ADDRESS_SEC_OFFSET * 0)
    #define OSPI_NV_ADDRESS_SEC2                                                             (OSPI_NV_ADDRESS + OSPI_NV_ADDRESS_SEC_OFFSET * 1)

#else
#define PNIO_USE_SPI_FLASH_FOR_FW
#ifdef PNIO_USE_SPI_FLASH_FOR_FW
    #define FLASH_INIT                                                                      spi_flash_init(  )
    #define FLASH_ERASE( _flash_strt_addr, _data_size, _pError )                            spi_flash_erase( _flash_strt_addr, _data_size, _pError )
    #define FLASH_ERASE_CHECK( _flash_strt_addr, _data_size )                               spi_flash_erase_verify( _flash_strt_addr, _data_size )
    #define FLASH_PROGRAM( _flash_strt_addr, _transmit_addr_ptr, _data_size, _pError )      spi_flash_program( _flash_strt_addr, _transmit_addr_ptr, _data_size, _pError )
    #define FLASH_READ( _flash_strt_addr, _transmit_addr_ptr, _data_size, _pError )         spi_flash_read( _flash_strt_addr, _transmit_addr_ptr, _data_size, _pError )
    #define FLASH_VERIFY( _pFlash8, _pBuf8, _BufSize )                                      spi_flash_verify( _pFlash8, _pBuf8, _BufSize )
    #define FLASH_CHIP_ERASE                                                                spi_flash_chip_erase(  )
    /* SPI non-volatile memory */
    #define SPI_NV_ADDRESS                                                                  0x5f0000
    #define SPI_NV_ADDRESS_SEC_OFFSET                                                       (0x4000)
    #define SPI_NV_ADDRESS_SEC1                                                             (0x5f0000 + SPI_NV_ADDRESS_SEC_OFFSET * 0)
    #define SPI_NV_ADDRESS_SEC2                                                             (0x5f0000 + SPI_NV_ADDRESS_SEC_OFFSET * 1)
#else
    #define FLASH_INIT
    #define FLASH_ERASE( _flash_strt_addr, _data_size, _pError )                            flash_erase( _flash_strt_addr, _data_size, _pError )
    #define FLASH_ERASE_CHECK( _flash_strt_addr, _data_size )                               flash_erase_check( _flash_strt_addr, _data_size )
    #define FLASH_PROGRAM( _flash_strt_addr, _transmit_addr_ptr, _data_size, _pError )      flash_program( _flash_strt_addr, _transmit_addr_ptr, _data_size, _pError )
    #define FLASH_READ( _flash_strt_addr, _transmit_addr_ptr, _data_size, _pError )         flash_read( _flash_strt_addr, _transmit_addr_ptr, _data_size, _pError )
    #define FLASH_VERIFY( _pFlash8, _pBuf8, _BufSize )                                      flash_verify( _pFlash8, _pBuf8, _BufSize )
#endif //PNIO_USE_SPI_FLASH_FOR_FW

#endif //BOARD_TYPE_STEP_3
typedef enum
{
    PNIO_SUBMOD_STATE_UNKNOWN           = 0,     // reserved, do not use
    PNIO_SUBMOD_STATE_STOP              = 1,     // submodule is in state STOP
    PNIO_SUBMOD_STATE_RUN               = 2,     // submodule is in state RUN
    PNIO_SUBMOD_STATE_APPL_RDY_FOLLOWS  = 3      // ApplicationReady follows later
} PNIO_SUBMOD_STATE;

// *------- reason code for event indication of AR-Offline, AR-Abort -----
// definition has to match cm_ar_reason_enum (cm_usr.h)
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

/* PNIO_MODULE_STATE  */
typedef enum
{
    PNIO_SUB_STATE_NO_SUBMODULE,
    PNIO_SUB_STATE_WRONG_SUBMODULE,
    PNIO_SUB_STATE_SUBSTITUTED_SUBMODULE,
    PNIO_SUB_STATE_PROPER_SUBMODULE,
} PNIO_SUBMODULE_STATE ;

// *---------------- geographical and logical addressing -------------*
typedef struct
{
    PNIO_ADDR_TYPE     Type;
    union
    {
        PNIO_UINT32    Addr; // logical address
        struct
        {
            PNIO_UINT32 Slot;
            PNIO_UINT32 Subslot;
        } Geo1;            // geographical address
    } ad;        // dont care element
} PNIO_DEV_ADDR;

#define Geo     ad.Geo1

// *--------------------------------------------------------------------------------------------------
// *     Alarm-Item.UserStructureIdentifier of an Alarm notification PDU
// *
// *     values from            0...0x7fff     manufacturer specific
// *                            0x8000         channel diagnosis
// *                            0x8001         multiple channel diagnosis
// *                            0x8002         extended channel diagnosis
// *--------------------------------------------------------------------------------------------------
#define PNIO_USR_STRUC_ID_CHANNEL_DIAG          0x8000  // channel diagnostic, see IEC 61158
#define PNIO_USR_STRUC_ID_MULTIPLE_CHANNEL_DIAG 0x8001  // multiple channel diagnostic, see IEC 61158
#define PNIO_USR_STRUC_ID_EXT_CHANNEL_DIAG      0x8002  // extended channel diagnostic, see IEC 61158

// *----------------------------------------------
// * read sw-version of the developers kit
// *----------------------------------------------
typedef struct
{
    PNIO_UINT8    item_hh;    // highest item
    PNIO_UINT8    item_h;     // high item
    PNIO_UINT8    item_l;     // low item
    PNIO_UINT8    item_ll;    // lowest item
} PNIO_DK_VERSION;

PNIO_EXTRN PNIO_UINT32 PNIO_netcom_enable (void);
PNIO_EXTRN PNIO_UINT32 PNIO_get_version (PNIO_DK_VERSION *pVersion);

// *----------------------------------------------
// * alarm data
// *----------------------------------------------
typedef struct
 {

    //    PNIO_UINT16  DeviceNum;       /* Device-Handle, see PNIO_device_open () */
    PNIO_UINT16  ArNum;                 /* AR-number, see PNIO_device_open() or SV_EVENT_CHECK_IND */
    PNIO_UINT16  SessionKey;            /* see CL_EVENT_IN_DATA_IND or SV_EVENT_CHECK_IND */
    PNIO_UINT32  Api;                   /* application process identifier */

    PNIO_UINT16  SlotNum;               /* 0 = device substitute */
    PNIO_UINT16  SubslotNum;            /* 0 = module substitute */

    PNIO_UINT16  AlarmPriority;         /* CM_ALARM_PRIORITY_LOW or CM_ALARM_PRIORITY_HIGH */

    PNIO_UINT16  AlarmType;             /* see CM_ALARM_TYPE_... */

    PNIO_UINT16  AlarmSequence;         /* see "AlarmSpecifier" bits 0-10     */
    PNIO_BOOL    DiagChannelAvailable;  /* see "AlarmSpecifier" bit 11        */
    PNIO_BOOL    DiagGenericAvailable;  /* see "AlarmSpecifier" bit 12        */
    PNIO_BOOL    DiagSubmodAvailable;   /* see "AlarmSpecifier" bit 13        */
    PNIO_UINT8   Reserved;              /* see "AlarmSpecifier" bit 14        */

    PNIO_BOOL    ArDiagnosisState;      /* see "AlarmSpecifier" bit 15        */
    PNIO_UINT32  ModIdent;              /* module identifier */
    PNIO_UINT32  SubIdent;              /* submodule identifier */

    PNIO_UINT16  UserStructIdent;       /* user-structure-tag for alarm_data, see CM_ALARM_TAG_CHANNEL_DIAGNOSIS */
    PNIO_UINT16  UserAlarmDataLength;   /* length of alarm_data */
    PNIO_UINT8*  UserAlarmData;         /* see Alarm-PDU, see CM_ALARM_OFFSET_DATA */

    PNIO_UINT32  CmPnioErr;             /* alarm-ack only, see the makro CM_PNIO_ERR_MAKE() in file cm_err.h */
}  PNIO_DEV_ALARM_DATA;

// *---------------------------------------------------
// *    data exchange functions
// *---------------------------------------------------
    //--------------------------------------------------------------------------------------
    // DATA EXCHANGE:
    // starts the exchange of input and output data between pnio stack and application.
    // After calling PNIO_initiate_data_read by the user, the stack calls PNIO_cbf_data_read()
    // with parameters device handle/slot/subslot/data-length and Bufferpointer.
    // After calling PNIO_initiate_data_write by the user, the stack calls PNIO_cbf_data_write()
    // with parameters device handle/slot/subslot/data-length and Bufferpointer.
    // During this time the IO data should not be modified by the application, to avoid
    // data inconsistency.
    // PNIO_initiate_data_read() and PNIO_initiate_data_write() are synchronous functions,
    // if they return, the data exchange has been finished.
    //--------------------------------------------------------------------------------------

    // *---------------------------------------------------
    // *    data exchange functions, called by the pnio stack
    // *---------------------------------------------------

    PNIO_EXTRN PNIO_IOXS    PNIO_cbf_data_write	// write data to IO stack (local ==> remote)
		    (
		     PNIO_UINT32    DevHndl,			// [in] Handle für Multidevice
		     PNIO_DEV_ADDR	*pAddr,				// [in] geographical or logical address
		     PNIO_UINT32 	BufLen,				// [in]  length of the submodule input data
		     PNIO_UINT8     *pBuffer, 			// [in,out] Ptr to data buffer to write to
		     PNIO_IOXS	    Iocs				// [in]  remote (io controller) consumer status
            );

    PNIO_EXTRN PNIO_IOXS    PNIO_cbf_data_read 	// read data from IO stack (remote ==> local)
		    (
		     PNIO_UINT32    DevHndl,			// [in] Handle für Multidevice
		     PNIO_DEV_ADDR	*pAddr,				// [in] geographical or logical address
		     PNIO_UINT32 	BufLen,				// [in] length of the submodule input data
		     PNIO_UINT8     *pBuffer, 			// [in] Ptr to data buffer to read from
		     PNIO_IOXS	    Iops				// [in] (io controller) provider status
            );

    PNIO_IOXS    PNIO_cbf_data_write_IOxS_only
		    (PNIO_UINT32	DevHndl,
		     PNIO_DEV_ADDR	*pAddr,
		     PNIO_IOXS	    Iocs);

    PNIO_IOXS     PNIO_cbf_data_read_IOxS_only
		    (PNIO_UINT32	DevHndl,
		     PNIO_DEV_ADDR	*pAddr,
		     PNIO_IOXS	    Iops
            );

// *---------------------------------------------------
// *    Functions for handling ALARMS and EVENTS
// *---------------------------------------------------
    #include "sys_pck1.h"
    #include "sys_unpck.h"

    typedef struct
    {
        //PNIO_UINT16       data_description;   //  see cm_sub_data_description_enum, note that CM_SUB_DATA_DESCR_TYPE_MASK is consistent with CM_SUB_PROP_TYPE_MASK */
        PNIO_UINT16       data_offset;          //  offset of IOData */
        PNIO_UINT16       data_length;          //  expected length of IOData */
        PNIO_UINT16       iocs_offset;          //  offset of IOCS */
        PNIO_UINT16       iops_offset;          //  offset of IOPS */

        PNIO_UINT8        iops_val;             //  iops value (for internal use only)
        PNIO_UINT8        iocs_val;             //  iops value (for internal use only)
    } PNPB_OWNSHIP;

    typedef enum
    {
        ELEM_FREE     = 0,
        ELEM_OCCUPIED = 1
    } EXP_ENTITY_STATE;

    typedef struct
    {
	    PNIO_UINT32			ApiNum;	            // [in] PNIO subslot number (0...0xffff)
	    PNIO_UINT32			SlotNum;	        // [in] PNIO subslot number (0...0xffff)
	    PNIO_UINT32			SubNum;	            // [in] subslot handle = table index in SubslotArray
	    PNIO_UINT32			ModIdent;		    // [in] module ident number (from GSDML file)
	    PNIO_UINT32			SubIdent;		    // [in] submodule ident number (from GSDML file)
	    PNIO_UINT32			ModProp;		    // [in] module properties
	    PNIO_UINT32			SubProp;		    // [in] submodule properties
        PNIO_UINT32         EntityIndSetCfg;    // [in] entity index in set_cfg of Perif-IF
        PNIO_UINT32         EntityState;        // [in] ELEM_FREE / ELEM_OCCUPIED
        PNPB_OWNSHIP        In;                 // [in] ownership information for input data
        PNPB_OWNSHIP        Out;                // [in] ownership information for output data
	    PNIO_UINT8			isPlugged;		    // [in] subslot includes any submodule (right or wrong...)
        PNIO_IO_PROP_ENUM   IoProp;             // [in] IO properties (in/out/in+out/noIO data). Field is valid only, if submodule is already plugged (else:IoProp = 0)
	    PNIO_UINT16         OwnSessionKey;      // [in, out] ind: the owner; rsp: 0 (reject ownership) or unchanged (take ownership)
        PNIO_BOOL           IsWrongSubmod;      // [in, out] ind: PNIO_FALSE (submodule is OK); rsp: PNIO_TRUE (wrong submodule) or unchanged
        PNIO_BOOL           ParamEndValid;      // [in] PNIO_TRUE ParamEnd event has been executed for this submodule
    } PNIO_EXP_SUB;

    typedef struct _PNIO_EXP
    {
	    // *** device identification
        PNIO_BOOL       ArValid;							   // AR is valid  (ownership indication has been processed or is under work)
        PNIO_BOOL       FirstParamEndValid; 		           // param end event has been signaled for this submodule
        PNIO_BOOL       Rdy4InpUpdateValid;  				   // input data exchange is possible (ready for input update event has been signaled)
        PNIO_UINT32     NumOfPluggedSub;                       // current number of pluggedsubslots
        PNIO_UINT32     IocrLen;                               // length of IOCR (data + IOxS + APDU state)
        PNIO_UINT32     LastApduStat;                          // value of last remote Apdu state from PNIO_initiate_data_read()
        PNIO_EXP_SUB    Sub [IOD_CFG_MAX_NUMOF_SUBSLOTS];
        PNIO_BOOL       IoUpdatePending;                       //
        PNIO_UINT16		ArType;
    } PNIO_EXP;

    typedef enum
    {
        PNIO_AR_TYPE_SINGLE        = 0x0001, /* IOCARSingle */
        PNIO_AR_TYPE_SUPERVISOR    = 0x0006, /* IOSAR, The supervisor AR is a special form of the IOCARSingle */
        PNIO_AR_TYPE_SINGLE_RTC3   = 0x0010, /* IOCARSingle using RT_CLASS_3 */
        PNIO_AR_TYPE_SINGLE_SYSRED = 0x0020, /* IOCARSR, The SR AR is a special form of the IOCARSingle indicating system redundancy or configure in run usage */
        PNIO_AR_TYPE_RESERVED /* all other types are reserved for future use */
    } PNIO_AR_TYPE;

    PNIO_EXTRN    void PNIO_cbf_ar_connect_ind
            (
                    PNIO_UINT32     DevHndl,            // [in] handle for a multidevice
                    PNIO_AR_TYPE    ArType,             // [in] type of AR (see cm_ar_type_enum)
                    PNIO_UINT32     ArNum,              // [in] AR number  (device access: ArNumber = 3)
                    PNIO_UINT16     ArSessionKey,       // [in] AS session key
                    PNIO_UINT16     SendClock,          // [in] sendclock
                    PNIO_UINT16     RedRatioIocrIn,     // [in] reduction ratio of input IOCR
                    PNIO_UINT16     RedRatioIocrOut,    // [in] reduction ratio of output IOCR
                    PNIO_UINT32     HostIp              // [in] ip address of host ( PN-Controller )
            );

    PNIO_EXTRN void  PNIO_cbf_ar_ownership_ind
            (
                PNIO_UINT32     DevHndl,
                PNIO_UINT32     ArNum,          // AR number 1....NumOfAR
                PNIO_EXP*       pOwnSub         // [in] list of submodules of expected configuration
            );

    PNIO_EXTRN void PNIO_cbf_check_ind
            (
             PNIO_UINT32            DevHndl,            // [in]  handle for a multidevice
             PNIO_UINT32            Api,                // [in]  Api number
             PNIO_DEV_ADDR	        *pAddr,             // [in]  geographical or logical address
             PNIO_UINT32            ModIdent,           // [in]  Ptr to module identifier
             PNIO_UINT32            SubIdent,           // [in]  Ptr to submodule identifier
             PNIO_UINT16            ArNum,              // [in]  ar - handle
             PNIO_SUBMODULE_STATE   *pSubState          // [out] Ptr to submodule state
            );

    PNIO_EXTRN void PNIO_cbf_ar_indata_ind    (PNIO_UINT32     DevHndl,       // [in] data exchange has been started
                                               PNIO_UINT16     ArNum,         // [in] AR number 1....NumOfAR
                                               PNIO_UINT16     SessionKey);   // [in] session key

    PNIO_EXTRN void PNIO_cbf_ar_disconn_ind   (PNIO_UINT32     DevHndl,       // [in] handle for a multidevice
                                               PNIO_UINT16     ArNum,         // [in] AR number 1....NumOfAR
                                               PNIO_UINT16     SessionKey,    // [in] session key
                                               PNIO_AR_REASON  ReasonCode);   // [in] AR abort after ArInData-indication

    PNIO_EXTRN PNIO_SUBMOD_STATE  PNIO_cbf_param_end_ind (PNIO_UINT32 DevHndl,            // [in] handle for a multidevice
                                                          PNIO_UINT16 ArNum,              // [in] AR number 1....NumOfAR
                                                          PNIO_UINT16 SessionKey,         // [in] session key
                                                          PNIO_UINT32 Api,                // [in] API (valid only, if SubslotNum <> 0)
                                                          PNIO_UINT16 SlotNum,            // [in] SlotNum (valid only, if SubslotNum <> 0)
                                                          PNIO_UINT16 SubslotNum,         // [in] 0: for all modules, <>0: for this submodule
                                                          PNIO_BOOL   MoreFollows);

    typedef enum
    {
        PNIO_AR_STARTUP   = 0x0001,  //* Ready for input update comes first during AR startup        (use case first valid input update after AR has been started)
        PNIO_AR_INDATA    = 0x0002   //* Ready for input update comes after AR startup has finished  (use case replug a submodule during indata)
    } PNIO_INP_UPDATE_STATE;

    PNIO_EXTRN void PNIO_cbf_ready_for_input_update_ind (PNIO_UINT32 DevHndl, // [in] handle for a multidevice
                                                         PNIO_UINT16 ArNum, // [in] handle for a multidevice
                                                         PNIO_INP_UPDATE_STATE InpUpdState);// [in] input update state (AR_STARTUP / AR_INDATA)

    PNIO_EXTRN void PNIO_cbf_dev_alarm_ind  (PNIO_UINT32            DevHndl, // [in] handle for a multidevice
                                             PNIO_DEV_ALARM_DATA    *pAlarm);// [in] alarm properties

    PNIO_EXTRN void PNIO_cbf_trigger_io_exchange (void);

    void set_default_init( void );

//--------------------------------------------------------------------------------------
// READ RECORD, WRITE RECORD:
//--------------------------------------------------------------------------------------

// **********************************************
// * block header, see IEC61158-6  "BlockHeader"
// **********************************************
#define BLOCKVERSION    0x0100

#include "sys_pck1.h"
    typedef  ATTR_PNIO_PACKED_PRE struct
    {
        PNIO_UINT16      Type;				// block type, big endian format
        PNIO_UINT16      Len;		        // block length
        PNIO_UINT16      Version;           // block version, , big endian format
    }  ATTR_PNIO_PACKED REC_IO_BLOCKHDR;    // more details see pnio spec
#include "sys_unpck.h"

    PNIO_EXTRN PNIO_UINT32  PNIO_cbf_rec_read
		    (
			    PNIO_UINT32			DevHndl,
			    PNIO_UINT32			Api,
                PNIO_UINT16         ArNum,			// ar - handle
                PNIO_UINT16 		SessionKey,	    // ar session number
			    PNIO_UINT32			SequenceNum,
			    PNIO_DEV_ADDR		*pAddr,			// [in] geographical or logical address
			    PNIO_UINT32			RecordIndex,
			    PNIO_UINT32			*pBufLen,		// [in,out] length to read, out: length, read by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [in,out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    );

    PNIO_EXTRN PNIO_UINT32  PNIO_cbf_rec_write
		    (
			    PNIO_UINT32			DevHndl,
			    PNIO_UINT32			Api,
                PNIO_UINT16         ArNum,			// ar - handle
                PNIO_UINT16 		SessionKey,	    // ar session number
			    PNIO_UINT32			SequenceNum,
			    PNIO_DEV_ADDR		*pAddr,			// [in] geographical or logical address
			    PNIO_UINT32			RecordIndex,
			    PNIO_UINT32			*pBufLen,		// [in,out] in: length to write, out: length, written by user
			    PNIO_UINT8			*pBuffer,		// [in] buffer pointer
			    PNIO_ERR_STAT		*pPnioState		// [in,out] 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
		    );

    // *-----------------------------------------------------------------
    // * Trigger PNDV_EV_TO_PNDV_DS_RW_DONE
    // * Used for XHIF host functionality
    // *-----------------------------------------------------------------
    PNIO_UINT32  PNIO_trigger_pndv_ds_rw_done (PNIO_ERR_STAT* PnioStat, PNIO_UINT32 bufLen);

    // *-------------------------------------------------
    // *    asynchronous record read, record write response
    // *-------------------------------------------------
    PNIO_UINT32  PNIO_rec_read_rsp
             (
                PNIO_VOID*          pRqHnd,         // request handle from PNIO_cbf_rec_read
                PNIO_UINT8*         pDat,           // data pointer (may be 0 if error)
                PNIO_UINT32         NettoDatLength, // total length of read record data
                PNIO_ERR_STAT*      pPnioStat       // PNIO state pointer (may be 0, if no error)
             );

    PNIO_UINT32  PNIO_rec_write_rsp
             (
                PNIO_VOID*          pRqHnd,         // request handle from PNIO_cbf_rec_read
                PNIO_UINT32         NettoDatLength, // total length of written record data
                PNIO_ERR_STAT*      pPnioStat       // PNIO state pointer (may be 0, if no error)
             );

    PNIO_VOID*  PNIO_rec_set_rsp_async (void);          // set asynchronous response mode (for 1 Request)

//--------------------------------------------------------------------------------------
// DCP Functions
//--------------------------------------------------------------------------------------
    PNIO_EXTRN PNIO_UINT32 PNIO_cbf_save_station_name
							    (	PNIO_INT8		*pStationName,
								    PNIO_UINT16		NameLength,
								    PNIO_UINT8		Remanent );

    PNIO_EXTRN PNIO_UINT32 PNIO_cbf_save_ip_addr
							     (	PNIO_UINT32 NewIpAddr,
								    PNIO_UINT32 SubnetMask,
								    PNIO_UINT32 DefRouterAddr,
								    PNIO_UINT8  Remanent);

    PNIO_UINT32 PNIO_cbf_report_new_ip_addr( PNIO_UINT32 NewIpAddr,
       								    	 PNIO_UINT32 SubnetMask,
   											 PNIO_UINT32 DefRouterAddr);

    typedef enum  {
        PNIO_RTF_RES_ALL            = 0x0000, //* suboption RTF - reset all device data (could also be
                                              //* the old service factory reset, which has the same meaning)
        PNIO_RTF_RES_APPL_PAR       = 0x0001, //* suboption RTF - reset application data for one interface
        PNIO_RTF_RES_COMM_PAR       = 0x0002, //* suboption RTF - reset communication data for one interface
        PNIO_RTF_RES_ENG_PAR        = 0x0003, //* suboption RTF - reset engineering data for one interface
        PNIO_RTF_RES_FWUP           = 0x0004
    } PNIO_RTF_OPTION;

    PNIO_UINT32 PNIO_cbf_reset_factory_settings (PNIO_UINT32 DevHndl,
                                                 PNIO_RTF_OPTION RtfOption);

    PNIO_EXTRN  PNIO_UINT32 PNIO_cbf_start_led_blink (PNIO_UINT32 DevHndl,
                                                      PNIO_UINT32 PortNum,
                                                      PNIO_UINT32 frequency);

    PNIO_EXTRN  PNIO_UINT32 PNIO_cbf_stop_led_blink  (PNIO_UINT32 DevHndl,
                                                      PNIO_UINT32 PortNum);

    #include "iodapi_rema.h"

// *-----------------------------------------------------------
// *  annotation
// *-----------------------------------------------------------
#define MAX_DEVICE_TYPE_LENGTH      25
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

// *-------------------------------------------------------------------
// * I&M 0....4
// *-------------------------------------------------------------------

#define IM0_ORDER_ID_LEN       20                   // length of IM0-Order ID
#define IM0_SER_NUM_LEN        16                   // length of IM0-serial number

#include "sys_pck1.h"

#define PNIO_SUPPORT_IM1234   0x1e
#define PNIO_SUPPORT_IM12345  0x3e

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
       REC_IO_BLOCKHDR      BlockHeader;	            // see pnio spec, BlockHeader
       IM0_DATA             IM0;                        // see pnio spec, data block
    }  ATTR_PNIO_PACKED  IM0_STRUCT;

    typedef ATTR_PNIO_PACKED_PRE struct
    {
       REC_IO_BLOCKHDR      BlockHeader;	            // see pnio spec, BlockHeader
       IM1_DATA             IM1;                        // see pnio spec, data block
    }  ATTR_PNIO_PACKED  IM1_STRUCT;

    typedef ATTR_PNIO_PACKED_PRE struct
    {
       REC_IO_BLOCKHDR      BlockHeader;	            // see pnio spec, BlockHeader
       IM2_DATA             IM2;                        // see pnio spec, data block
    }  ATTR_PNIO_PACKED  IM2_STRUCT;

    typedef ATTR_PNIO_PACKED_PRE struct
    {
       REC_IO_BLOCKHDR      BlockHeader;	            // see pnio spec, BlockHeader
       IM3_DATA             IM3;                        // see pnio spec, data block
    }  ATTR_PNIO_PACKED  IM3_STRUCT;

    typedef ATTR_PNIO_PACKED_PRE struct
    {
       REC_IO_BLOCKHDR      BlockHeader;                // see pnio spec, BlockHeader
       IM4_DATA             IM4;                        // see pnio spec, data block
    }  ATTR_PNIO_PACKED  IM4_STRUCT;

#ifndef PNIO_IM0_SUPP_USE_ENUM_TYPE
    #define PNIO_IM0_NOTHING     0x00   /* this submodule does not contain I&M data                    */
    #define PNIO_IM0_SUBMODULE   0x01   /* this submodule contains I&M data                            */
    #define PNIO_IM0_MODULE      0x02   /* optional modifier: this submodule stands for the module     */
    #define PNIO_IM0_DEVICE      0x04   /* optional modifier: this submodule stands for the device     */
    #define PNIO_IM0_SUPP_ENUM   PNIO_UINT8  /* shouldn't be ENUM, as bitwise combinations are used    */
#else
    typedef enum
    { /* see I&M0FilterData (record index F840) */
        PNIO_IM0_NOTHING   = 0x00,  // * this submodule does not contain I&M data
        PNIO_IM0_SUBMODULE = 0x01,  // * this submodule contains I&M data
        PNIO_IM0_MODULE    = 0x02,  // * optional modifier: this submodule stands for the module
        PNIO_IM0_DEVICE    = 0x04   // * optional modifier: this submodule stands for the device
    } PNIO_IM0_SUPP_ENUM;
#endif

    typedef struct
    {
        PNIO_UINT32            Api;        // api number
        PNIO_UINT32            Slot;       // slot number (1..0x7fff)
        PNIO_UINT32            Subslot;    // subslot number (1..0x7fff)
        PNIO_UINT32            ModId;      // module ID
        PNIO_UINT32            SubId;      // submodule ID
        PNIO_UINT32            InDatLen;   // input data length
        PNIO_UINT32            OutDatLen;  // output data length
        PNIO_IM0_SUPP_ENUM     Im0Support; // submodule has own I&M0 data, values see PNIO_IM0_SUPP_ENUM
    } PNIO_SUB_LIST_ENTRY;

    typedef ATTR_PNIO_PACKED_PRE struct
    {
        PNIO_UINT32      Api;                        // api number
        PNIO_UINT32      Slot;                       // slot number (1..0x7fff)
        PNIO_UINT32      Subslot;                    // subslot number (1..0x7fff)
        IM0_DATA         Im0Dat;
    } ATTR_PNIO_PACKED  PNIO_IM0_LIST_ENTRY;

#include "sys_unpck.h"

// *-----------------------------------------------------------
// *  snmp parameters for the LLDP MIB
// *
// *-----------------------------------------------------------

typedef struct
{
    PNIO_INT8*  pSysName;           // system name, read from MIB2
    PNIO_UINT32 SysNameLen;         // length of string *pSysName
    PNIO_INT8*  pSysContact;        // system contac, read from MIB2
    PNIO_UINT32 SysContactLen;      // length of string *pSysContact
    PNIO_INT8*  pSysLoc;            // system location, read from MIB2
    PNIO_UINT32 SysLocLen;          // length of string *pSysLoc
    PNIO_INT8*  pSysDescr;          // system description, read from MIB2
    PNIO_UINT32 SysDescrLen;        // length of string *pSysDescr
    PNIO_CHAR*  pIfDescr;           // interface description, read from MIB2
    PNIO_UINT32 IfDescrLen;         // length of string *pIfDescr
    PNIO_CHAR*  pPortName1;         // port name
    PNIO_UINT32 PortNameLen1;       // length of string *pPortName
#if (IOD_CFG_PDEV_NUMOF_PORTS >= 2)
    PNIO_CHAR*  pPortName2;         // port name
    PNIO_UINT32 PortNameLen2;       // length of string *pPortName
#endif
#if (IOD_CFG_PDEV_NUMOF_PORTS >= 3)
    PNIO_CHAR*  pPortName3;         // port name
    PNIO_UINT32 PortNameLen3;       // length of string *pPortName
#endif
#if (IOD_CFG_PDEV_NUMOF_PORTS >= 4)
    PNIO_CHAR*  pPortName4;         // port name
    PNIO_UINT32 PortNameLen4;       // length of string *pPortName
#endif

} PNIO_SNMP_LLDP;

// *=========================================================================================
// *  functions, called by the user application
// *=========================================================================================
// *-----------------------------------------------
// * init PNIO, must be called first!!
// *-----------------------------------------------

void PNIO_init (void);

// *-----------------------------------------------
// * pnio startup
// *-----------------------------------------------

PNIO_EXTRN PNIO_UINT32 PNIO_setup
                            (PNIO_INT8	 *pStationName,     // unique station name
                             PNIO_UINT16 StationNameLen,    // length of station name (may not be zero terminated)
						     PNIO_INT8	 *pStationType,     // station type (must fit to GSDML file)
                             PNIO_UINT32 IpAddr,            // IP Suite: IP Addr
                             PNIO_UINT32 SubnetMask,        // IP Suite: Subnet Mask
                             PNIO_UINT32 DefRouterAddr);    // IP Suite: default router address

PNIO_EXTRN PNIO_UINT32 PNIO_pdev_setup
                           (PNIO_SUB_LIST_ENTRY   *pIoSubList,            // plugged submodules, including PDEV
                            PNIO_UINT32           NumOfSubListEntries);   // number of entries in pPioSubList

PNIO_EXTRN PNIO_UINT32  PNIO_shutdown (void);

PNIO_EXTRN PNIO_UINT32  PNIO_device_open
                            (PNIO_UINT16            VendorId,           // [in]  vendor id (see gsdml file)
                             PNIO_UINT16            DeviceId,           // [in]  device id (see gsdml file)
                             PNIO_ANNOTATION*       pDevAnnotation,     // [in]  device annotation
                             PNIO_SNMP_LLDP*        pSnmpPar,           // [in]  snmp parameters for the LLDP MIB
                             PNIO_UINT32            *pDevHndl);         // [out] device handle

PNIO_EXTRN PNIO_UINT32  PNIO_device_close ( PNIO_UINT32 DeviceHndl );       // device handle

PNIO_EXTRN PNIO_UINT32  PNIO_device_start ( PNIO_UINT32 DevHndl );// activate readiness to connect

PNIO_EXTRN PNIO_UINT32  PNIO_device_stop  ( PNIO_UINT32 DevHndl );

PNIO_EXTRN PNIO_UINT32  PNIO_change_ip_suite ( PNIO_UINT32 newIpAddr, PNIO_UINT32 SubnetMask, PNIO_UINT32 DefRouterAddr );

PNIO_EXTRN PNIO_UINT32  PNIO_change_device_name ( PNIO_INT8 *pStationName, PNIO_UINT16 NameLength );

void    PNIO_Fatal              ( void );

typedef enum PNIO_WD_GRANITY
{
    PNIO_WD_100MS,
    PNIO_WD_10MS,
    PNIO_WD_1MS,
    PNIO_WD_100US
}PNIO_WD_GRANITY;

void    PNIO_hw_watchdog_init   ( PNIO_UINT32 time, PNIO_WD_GRANITY granity );
void    PNIO_hw_watchdog_deinit ( void );
void    PNIO_hw_watchdog_start  ( void );
void    PNIO_hw_watchdog_stop   ( void );
void    PNIO_hw_watchdog_trigger( void );

PNIO_UINT32 PNIO_device_ar_abort (PNIO_UINT32 DevHndl, PNIO_UINT16 ArNum);

PNIO_UINT32  PNIO_sub_plug
                (PNIO_UINT32	    DevHndl,        // device handle
                 PNIO_UINT32        Api,            // application process identifier
                 PNIO_DEV_ADDR	    *pAddr,		    // geographical or logical address (slot/subslot)
                 PNIO_UINT32		ModIdent,       // module identifier  (see GSDML file)
                 PNIO_UINT32	    SubIdent,       // submodule identifier (see GSDML file)
                 PNIO_UINT32        InputDataLen,   // submodule input data length
                 PNIO_UINT32        OutputDataLen,  // submodule output data length
                 PNIO_IM0_SUPP_ENUM Im0Support,     // for I&M0FilterData, see enum PNIO_IM0_SUPP_ENUM
                 IM0_DATA           *pIm0Dat,       // pointer to IM0 data
                 PNIO_UINT8         IopsIniVal);    // initial value for iops-input, ONLY FOR SUBMOD WITHOUT IO DATA (e.g. PDEV)

PNIO_UINT32  PNIO_sub_plug_list
                (PNIO_UINT32           DevHndl,               // device handle
                 PNIO_SUB_LIST_ENTRY   *pIoSubList,           // plugged submodules, including PDEV
                 PNIO_UINT32           NumOfSubListEntries,   // number of entries in pIoSubList
                 PNIO_IM0_LIST_ENTRY   *pIm0List,             // list of IM0 data sets
                 PNIO_UINT32           NumOfIm0ListEntries,   // number of entries in pIm0List
                 PNIO_UINT32*          pStatusList);          // list of return-Stati[NumOfSublistEntries]

PNIO_UINT32  PNIO_sub_pull
                (PNIO_UINT32	DevHndl,        // device handle
                 PNIO_UINT32    Api,            // application process identifier
                 PNIO_DEV_ADDR	*pAddr);		// geographical or logical address (slot/subslot)

// *-----------------------------------------------
// * set diagnostic data in pnio stack
// *-----------------------------------------------
// *------ defines for channel properties.type --------

typedef enum
{
    DIAG_CHANPROP_TYPE_OTHERS	=	0,	// any other data type
    DIAG_CHANPROP_TYPE_1BIT		=	1,	// data type 1 Bit
    DIAG_CHANPROP_TYPE_2BIT		=	2,	// data type 2 Bit
    DIAG_CHANPROP_TYPE_4BIT		=	3,	// data type 4 Bit
    DIAG_CHANPROP_TYPE_BYTE		=	4,	// data type 8 Bit
    DIAG_CHANPROP_TYPE_WORD		=	5,	// data type 16 Bit
    DIAG_CHANPROP_TYPE_DWORD	=	6,	// data type 32 Bit
    DIAG_CHANPROP_TYPE_LWORD	=	7	// data type 64 Bit
#ifndef DIAG_CHANPROP_TYPE_USE_ENUM_TYPE
} DIAG_CHANPROP_TYPE_ENUM;
#define DIAG_CHANPROP_TYPE		PNIO_UINT16
#else
} DIAG_CHANPROP_TYPE;
#endif

// *------ defines for channel properties.specifier --------

typedef enum
{
    DIAG_CHANPROP_SPEC_ERR_APP			= 1,	// new error appears
    DIAG_CHANPROP_SPEC_ERR_DISAPP		= 2,	// error disappears, no more error
    DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE	= 3	    // error disappears, but other errors remain
} DIAG_CHANPROP_SPEC_ERR;


// *------ defines for channel properties.direction  --------

typedef enum
{
    DIAG_CHANPROP_DIR_MANUSPEC	=   0,                    	// manufacturer specific
    DIAG_CHANPROP_DIR_IN		=	PNIO_SUB_PROP_IN, 	    // Input
    DIAG_CHANPROP_DIR_OUT		=	PNIO_SUB_PROP_OUT, 	    // Output
    DIAG_CHANPROP_DIR_IN_OUT	=	PNIO_SUB_PROP_IN_OUT 	// input output

#ifndef	DIAG_CHANPROP_DIR_USE_ENUM_TYPE
} DIAG_CHANPROP_DIR;
#define DIAG_CHANPROP_DIR	PNIO_UINT16
#else
} DIAG_CHANPROP_DIR;
#endif

// *------ defines for channel error type --------
#define DIAG_CHAN_ERR_SHORT_CIRCUIT			1	//
#define DIAG_CHAN_ERR_UNDEVOLTAGE			2	//
#define DIAG_CHAN_ERR_OVERVOLTAGE			3	//
#define DIAG_CHAN_ERR_OVERLOAD				4	//
#define DIAG_CHAN_ERR_OVERTEMPERATURE		5	//
#define DIAG_CHAN_ERR_LINE_BREAK			6	//
#define DIAG_CHAN_ERR_UPPER_LIMIT_EXCEEDED	7	//
#define DIAG_CHAN_ERR_LOWER_LIMIT_EXCEEDED	8	//
#define DIAG_CHAN_ERR_ERROR					9	//

PNIO_EXTRN PNIO_UINT32  PNIO_diag_channel_add
						(PNIO_UINT32	     DevHndl,		     // device handle
                         PNIO_UINT32         Api,                // application process identifier
						 PNIO_DEV_ADDR       *pAddr,			 // geographical or logical address
						 PNIO_UINT16	     ChannelNum,		 // channel number
						 PNIO_UINT16	     ErrorNum,	         // error number
						 DIAG_CHANPROP_DIR   ChanDir,            // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
						 DIAG_CHANPROP_TYPE  ChanTyp,            // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
                         PNIO_BOOL           MaintenanceReq,     // maintenance required
                         PNIO_BOOL           MaintenanceDem,     // maintenance demanded
						 PNIO_UINT16	     DiagTag);           // user defined diag tag != 0

PNIO_EXTRN PNIO_UINT32  PNIO_diag_channel_remove
						(PNIO_UINT32	DevHndl,		        // device handle
                         PNIO_UINT32    Api,                    // application process identifier
						 PNIO_DEV_ADDR  *pAddr,			        // geographical or logical address
						 PNIO_UINT16	ChannelNum,		        // channel number
                         PNIO_UINT16    ErrorNum,               // error number
						 DIAG_CHANPROP_DIR  ChanDir,            // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
						 DIAG_CHANPROP_TYPE ChanTyp,            // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
						 PNIO_UINT16	    DiagTag, 		    // user defined diag tag != 0
                         PNIO_UINT16        AlarmState);        // DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE   2017-04-18

PNIO_EXTRN PNIO_UINT32  PNIO_ext_diag_channel_add
						(PNIO_UINT32	    DevHndl,		    // device handle
                         PNIO_UINT32        Api,                // application process identifier
						 PNIO_DEV_ADDR      *pAddr,			    // geographical or logical address
						 PNIO_UINT16	    ChanNum,		    // channel number
						 PNIO_UINT16	    ErrorNum,	        // error number
						 DIAG_CHANPROP_DIR  ChanDir,            // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
						 DIAG_CHANPROP_TYPE ChanTyp,            // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
						 PNIO_UINT16	    ExtChannelErrType,	// channel error type           (see PNIO spec.)
						 PNIO_UINT32	    ExtChannelAddValue, // extended channel add. value  (see PNIO spec.)
                         PNIO_BOOL          MaintenanceReq,     // maintenance required
                         PNIO_BOOL          MaintenanceDem,     // maintenance demanded
						 PNIO_UINT16	    DiagTag); 		    // user defined diag tag != 0

PNIO_UINT32  PNIO_ext_diag_channel_remove
                        (PNIO_UINT32        DevHndl,            // device handle
                         PNIO_UINT32        Api,                // application process identifier
                         PNIO_DEV_ADDR      *pAddr,             // geographical or logical address
                         PNIO_UINT16        ChannelNum,         // channel number
                         PNIO_UINT16        ErrorNum,           // error number, see PNIO specification coding of "ChannelErrorType"
                         DIAG_CHANPROP_DIR  ChanDir,            // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
                         DIAG_CHANPROP_TYPE ChanTyp,            // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
                         PNIO_UINT16        ExtChannelErrType,  // channel error type           (see PNIO spec.)
                         PNIO_UINT32        ExtChannelAddValue, // extended channel add. value  (see PNIO spec.)
						 PNIO_UINT16	    DiagTag, 		    // user defined diag tag != 0
                         PNIO_UINT16        AlarmState);        // DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE

PNIO_EXTRN PNIO_UINT32  PNIO_diag_generic_add
						(PNIO_UINT32	    DevHndl,		    // device handle
                         PNIO_UINT32        Api,                // application process identifier
						 PNIO_DEV_ADDR      *pAddr,			    // geographical or logical address
                         PNIO_UINT16        ChannelNum,         // channel number
						 DIAG_CHANPROP_DIR  ChanDir,            // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
						 DIAG_CHANPROP_TYPE ChanTyp,            // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
						 PNIO_UINT16	    DiagTag,		    // user defined diag tag != 0
						 PNIO_UINT16	    UserStructIdent,    // structure of info data (see IEC61158)
						 PNIO_UINT8		    *pInfoData,		    // diag data
						 PNIO_UINT32	    InfoDataLen, 	    // length of diag data in bytes
                         PNIO_BOOL          MaintenanceReq,     // maintenance required
                         PNIO_BOOL          MaintenanceDem);    // maintenance demanded

PNIO_EXTRN PNIO_UINT32  PNIO_diag_generic_remove
						(PNIO_UINT32	    DevHndl,		    // device handle
                         PNIO_UINT32        Api,                // application process identifier
						 PNIO_DEV_ADDR      *pAddr,			    // geographical or logical address
                         PNIO_UINT16        ChanNum,            // channel number
		                 DIAG_CHANPROP_DIR  ChanDir,            // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		                 DIAG_CHANPROP_TYPE ChanTyp,            // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
						 PNIO_UINT16	    DiagTag,		    // user defined diag tag != 0
		                 PNIO_UINT16	    UserStructIdent);    // structure of info data (see IEC 61158)

// *-----------------------------------------------
// * diag alarms and process alarms
// *
// * Note: only one diagnosis- or process alarm at
// *       a time is possible. Wait before sending
// *       a new alarm,until the previous one has
// *       been confirmed.
// *-----------------------------------------------

PNIO_EXTRN PNIO_UINT32  PNIO_process_alarm_send
						(PNIO_UINT32	 DevHndl,		// device handle
                         PNIO_UINT32     Api,           // application process identifier
						 PNIO_DEV_ADDR	 *pAddr,		// geographical or logical address
						 PNIO_UINT8		 *pData,		// AlarmItem.Data
						 PNIO_UINT32	 DataLen,		// length of AlarmItem.Data
						 PNIO_UINT16	 UserStructIdent,// AlarmItem.UserStructureIdentifier, s. IEC61158-6
				 		 PNIO_UINT32	 UserHndl);		// user defined handle
						 
PNIO_EXTRN PNIO_UINT32  PNIO_status_alarm_send
						(PNIO_UINT32	 DevHndl,		// device handle
                         PNIO_UINT32     Api,           // application process identifier
						 PNIO_DEV_ADDR	 *pAddr,		// geographical or logical address
						 PNIO_UINT8		 *pData,		// AlarmItem.Data
						 PNIO_UINT32	 DataLen,		// length of AlarmItem.Data
						 PNIO_UINT16	 UserStructIdent,// AlarmItem.UserStructureIdentifier, s. IEC61158-6
				 		 PNIO_UINT32	 UserHndl);		// user defined handle

PNIO_EXTRN PNIO_UINT32  PNIO_ret_of_sub_alarm_send
						(PNIO_UINT32	DevHndl,		// device handle
                         PNIO_UINT32    Api,            // application process identifier
						 PNIO_DEV_ADDR  *pAddr,			// geographical or logical address
				 		 PNIO_UINT32	 UserHndl);		// user defined handle

PNIO_EXTRN PNIO_UINT32  PNIO_upload_retrieval_alarm_send
                        (PNIO_UINT32    DevHndl,       // device handle
                         PNIO_UINT32    Api,           // application process identifier
                         PNIO_DEV_ADDR  *pAddr,        // geographical or logical address
                         PNIO_UINT8     *pData,        // AlarmItem.Data
                         PNIO_UINT32    DataLen,       // length of AlarmItem.Data
                         PNIO_UINT32    UsrHndl);       // user defined handle

typedef enum
{
    PNIO_ALM_PROC = 1,              // 0 is not used
    PNIO_ALM_STATUS,
    PNIO_ALM_CHAN_DIAG,
    PNIO_ALM_EXT_CHAN_DIAG,
    PNIO_ALM_GEN_DIAG,
    PNIO_ALM_RET_OF_SUB,
    PNIO_ALM_UPLOAD_RETRIEVAL,
    PNIO_ALM_NUMOF_ENTRIES          // must be last element in this enum struct !!
} PNIO_ALARM_TYPE;

PNIO_EXTRN void PNIO_cbf_async_req_done (PNIO_UINT32     DevHndl,       // device handle
                                         PNIO_UINT32     ArNum,         // AR number 1....NumOfAR
                                         PNIO_ALARM_TYPE AlarmType,     // alarm type
                                         PNIO_UINT32     Api,           // API number
                                         PNIO_DEV_ADDR   *pAddr,        // location (slot, subslot)
                                         PNIO_UINT32     Status,        // status
                                         PNIO_UINT16    Diag_tag);

// *-----------------------------------------------
// * device control
// *------------------------------------------------

// *** set device state  CLEAR/OPERATE
#define PNIO_DEVSTAT_OPERATE                    1
#define PNIO_DEVSTAT_CLEAR                      2

PNIO_EXTRN PNIO_UINT32 PNIO_set_dev_state (PNIO_UINT32 DevHndl,   // device handle
                                           PNIO_UINT32 DevState); // device state CLEAR/OPERATE

// *---------------------------------------------------------
// *  data exchange with the simple callback interface (SCI)
// *  (default interface, backward-compatibility to V2.0)
// *  The SCI is the simplest way to perform IO data exchange
// *  between PNIO stack and application  for RT, IRT flex and
// *  IRT top
// *
// *  structure of APDU status (see PNIO specification)
// *  byte 0,1:    16 bit Cycle counter (big endian format)
// *  byte 2:      APDU-Statusbyte, see PNIO_APDU_STATUSBYTE_MASK
// *  byte 3:      Transfer status
// *---------------------------------------------------------
// *------- APDU status code according to PNIO specification -----

typedef enum
{
    PNIO_APDU_STATUS_PRIMARY         = 0x0001,  // bit=1: primary    0: backup
    PNIO_APDU_STATUS_REDUNDANCY      = 0x0002,  // bit=1: primary AR 0: Backup AR
    PNIO_APDU_STATUS_DATA_VALID      = 0x0004,  // bit=1: data valid 0: data invalid
    PNIO_APDU_STATUS_RUN             = 0x0010,  // bit=1: state run  0: state stop
    PNIO_APDU_STATUS_STATION_OK      = 0x0020,  // bit=1: OK         0: problem
} PNIO_APDU_STATUSBYTE_MASK;

PNIO_EXTRN PNIO_UINT32 PNIO_initiate_data_read  (PNIO_UINT32  DevHndl);     // device handle

PNIO_EXTRN PNIO_UINT32 PNIO_initiate_data_write (PNIO_UINT32  DevHndl);     // device handle

PNIO_EXTRN PNIO_UINT32 PNIO_set_iops  (PNIO_UINT32 Api,
                                       PNIO_UINT32 SlotNum,
                                       PNIO_UINT32 SubNum,
                                       PNIO_UINT8  Iops);

PNIO_UINT32 PNIO_get_last_apdu_status (PNIO_UINT32  DevHndl,
                                       PNIO_UINT32  ArNum);

// *------------------------------------------------------------
// *  DBAI   (Direct Buffer Access Interface)
// *  The DBAI provides a direct buffer access to the IOCR-data.
// *  This provides a performance benefit compared to the Standard
// *  Interface (SI) in the following use case:
// *    - device has a lot of submodules with few bytes in
// *      every submodule
// *-------------------------------------------------------------

typedef struct
{
    PNIO_UINT32             DevHndl;      // [in]  device handle
    PNIO_UINT32             ArNum;        // [in]  AR number (1....NumOfAR), obtained in PNIO_cbf_ar_ownership_ind
    PNIO_IOCR_TYPE_ENUM     IoDir;        // IO direction (PNIO_IOCR_TYPE_INPUT, PNIO_IOCR_TYPE_OUTPUT)
    PNIO_UINT8*             pBuf;         // [out] pointer to the IOCR buffer inside PNIO stack
    PNIO_UINT32             ApduStat;     // [out] Cycle counter der Input IOCR
} PNIO_BUFFER_LOCK_TYPE;


void        PNIO_dbai_enter		(void);
void        PNIO_dbai_exit		(void);

PNIO_UINT32 PNIO_dbai_buf_lock    (PNIO_BUFFER_LOCK_TYPE* pLock);
PNIO_UINT32 PNIO_dbai_buf_unlock  (PNIO_BUFFER_LOCK_TYPE* pLock);

// *-------------------------------------------------
// *	asynchronous record read, record write response
// *-------------------------------------------------

PNIO_UINT32  PNIO_rec_read_rsp
         (
            PNIO_VOID*          pRqHnd,         // request handle from PNIO_cbf_rec_read
            PNIO_UINT8*         pDat,		    // data pointer (may be 0 if error)
            PNIO_UINT32         NettoDatLength, // total length of read record data
            PNIO_ERR_STAT*      pPnioStat       // PNIO state pointer (may be 0, if no error)
         );

PNIO_UINT32  PNIO_rec_write_rsp
         (
            PNIO_VOID*          pRqHnd,         // request handle from PNIO_cbf_rec_read
            PNIO_UINT32         NettoDatLength, // total length of written record data
            PNIO_ERR_STAT*      pPnioStat       // PNIO state pointer (may be 0, if no error)
         );

PNIO_VOID*  PNIO_rec_set_rsp_async (void);          // set asynchronous response mode (for 1 Request)

#if IOD_INCLUDE_REC8028_8029

    // *** values for pSubstActive ***
    #define REC8029_SUBSTSTATE_OPERATION     0  // state normal operation, the normal output values are valid, IOPS = IOCS = PNIO_S_OK
    #define REC8029_SUBSTSTATE_SUBSTITUTE    1  // substitute values are active, IOPS or IOCS or both are bad


    // **** special case:   read record index 0x8029  (read output data)
    PNIO_EXTRN PNIO_IOXS    PNIO_cbf_substval_out_read  // read substitution values for output submodule
            (
             PNIO_UINT32    DevHndl,            // [in] Handle für Multidevice
             PNIO_DEV_ADDR  *pAddr,             // [in] geographical or logical address
             PNIO_UINT32    BufLen,             // [in] length of the submodule output substitute data
             PNIO_UINT8     *pBuffer,           // [in] Ptr to submodule output substitute data
             PNIO_UINT16*   pSubstMode,         // [in, out] SubstitutionMode, see define REC8029_SUBSTMOD_xxxx
             PNIO_UINT16*   pSubstActive        // [in] SubstituteActiveFlag see define REC8029_SUBSTSTATE_xxx,   default value is 0: if (IOPS & IOCS = GOOD), else: 1
            );
#endif

// *--------------------------------------------------------------------------------------
// * WRITE PARAM RECORDS:
// * --------------------------------------------------------------------------------------

PNIO_UINT32  PNIO_cbf_store_rema_mem
        (
            PNIO_UINT32     MemSize,        // size of rema data
            PNIO_UINT8*     pMem            // pointer to rema data
        );

PNIO_UINT32  PNIO_cbf_restore_rema_mem
        (
             PNIO_UINT32*   pMemSize,        // size of rema data
             PNIO_UINT8**   ppMem            // pointer to pointer to rema data
        );

PNIO_UINT32  PNIO_cbf_free_rema_mem     (PNIO_UINT8* pMem);

//--------------------------------------------------------------------------------------
// errors and fatal errors
//--------------------------------------------------------------------------------------
// *-------- define package ID's for logging levels -----
#define PNIO_PACKID_ACP         1 /* PN Stack Pack IDs */
#define PNIO_PACKID_CLRPC       2
#define PNIO_PACKID_CM          3
#define PNIO_PACKID_EDDP        4
#define PNIO_PACKID_GSY         5
#define PNIO_PACKID_IP2PN       6
#define PNIO_PACKID_LLDP        7
#define PNIO_PACKID_MRP         8
#define PNIO_PACKID_SOCK        9
#define PNIO_PACKID_SNMPX       10
#define PNIO_PACKID_POF         11
#define PNIO_PACKID_PSI         12

#define PNIO_PACKID_EVMA        20 /* IODDevkit Stack Pack IDs */
#define PNIO_PACKID_GDMA        21
#define PNIO_PACKID_IODAPI      22
#define PNIO_PACKID_IOM         23
#define PNIO_PACKID_PNDV        24
#define PNIO_PACKID_PNPB        25
#define PNIO_PACKID_TSKMA       26
#define PNIO_PACKID_TRC         27

#define PNIO_PACKID_OTHERS      99

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

PNIO_EXTRN void PNIO_Log
                    (PNIO_UINT32  DevHndl,      // device handle
                     PNIO_UINT32  ErrLevel,     // message level
                     PNIO_UINT32  PackId,       // package ID, see PNIO_PACKID_xxxx
                     PNIO_UINT32  ModId,        // module ID, see xx_MODULE_ID in sourcecode
                     PNIO_UINT32  LineNum);     // sourcecode line number

PNIO_EXTRN PNIO_UINT32 PNIO_set_log_level
                    (PNIO_UINT32  NewLogLevel,  // debug level
                     PNIO_UINT32  PackId);      // package ID, see PNIO_PACKID_xxxx

PNIO_EXTRN PNIO_UINT32 DecodePackId(PNIO_UINT32 LsaPackId);

PNIO_ERR_ENUM   PNIO_get_last_error (void);

//--------------------------------------------------------------------------------------
// printf
//--------------------------------------------------------------------------------------

PNIO_EXTRN void PNIO_printf(PNIO_CHAR* fmt, ...);

PNIO_EXTRN PNIO_UINT32 PNIO_sprintf(PNIO_UINT8* pBuf, PNIO_CHAR* fmt, ...);

PNIO_EXTRN PNIO_UINT32 PNIO_vsnprintf(PNIO_UINT8* pBuf, PNIO_UINT32 count, PNIO_CHAR* fmt, PNIO_va_list argptr);

PNIO_EXTRN PNIO_UINT32 PNIO_snprintf (PNIO_UINT8* str, PNIO_UINT32 count, PNIO_UINT8* fmt, ...);

PNIO_EXTRN PNIO_UINT32 PNIO_vsprintf(PNIO_UINT8* pBuf, PNIO_CHAR* fmt, PNIO_va_list argptr);

PNIO_EXTRN void PNIO_TrcPrintf(PNIO_CHAR* fmt, ...);

PNIO_EXTRN void PNIO_FatalError(LSA_FATAL_ERROR_TYPE* pLsaErr);

PNIO_EXTRN void PNIO_ConsolePrintf(PNIO_CHAR* fmt, ...);

PNIO_EXTRN void PNIO_ConsoleLog(PNIO_CHAR* fmt, ...);

//--------------------------------------------------------------------------------------
//  defines for IRT
//--------------------------------------------------------------------------------------

typedef enum
{
    PNIO_CP_CBE_UNKNOWN           = 0,
    PNIO_CP_CBE_STARTOP_IND       = 1,
    PNIO_CP_CBE_OPFAULT_IND       = 2,
    PNIO_CP_CBE_NEWCYCLE_IND      = 3,
    PNIO_CP_CBE_TRANS_END_IND     = 4,
    PNIO_CP_CBE_COMP1_IND         = 5
} PNIO_CP_CBE_TYPE;

typedef struct
{
    PNIO_UINT32*      pCycleCount;              // sync clock counter, step 31.25 usec
    PNIO_UINT32*      pClockCount;              // cycle counter, step 10 nsec
    PNIO_UINT32*      pCountSinceCycleStart;    // counts since newCycle, step 10nsec
    PNIO_UINT32*      pNewCycleSnap;            // snapshot of CycleCount at newCycle, step 10 nsec
} PNIO_CYCLE_INFO_PTR;

typedef void (*PNIO_CP_CBF)(void);

PNIO_UINT32  PNIO_CP_register_cbf(PNIO_CP_CBE_TYPE CbeType,
                                  PNIO_CP_CBF      pCbf);

// *----------------------------------------------------------
// *  PNIO ISO INTERFACE
// *----------------------------------------------------------

typedef enum _PNIO_ISO_GPIO_LEVEL_TYPE
{
    ISO_GPIO_LOW_ACTIVE  = 0,
    ISO_GPIO_HIGH_ACTIVE = 1
}PNIO_ISO_GPIO_LEVEL_TYPE;

typedef enum PNIO_STATE_OF_EVMA
{
    PNIO_EVMA_STATE_IDLE,
    PNIO_EVMA_STATE_IN_USE

}PNIO_STATE_OF_EVMA;
#define PNIO_EVMA_INST_DESC_MAGIC        0x5a7f3922
typedef void*   PNIO_ISO_OBJ_HNDL;
typedef struct PNIO_EVMA_INSTANCE_HEADER
{
    PNIO_UINT32             magic_number;
    PNIO_STATE_OF_EVMA      state;
}PNIO_EVMA_INSTANCE_HEADER;

PNIO_UINT32   PNIO_IsoActivateIsrObj(   PNIO_VOID          (*pIsrCbf)(void),
                                        PNIO_UINT32         DelayTim_ns,
                                        PNIO_ISO_OBJ_HNDL*  pObjHnd);

PNIO_UINT32   PNIO_IsoActivateTransEndObj(PNIO_VOID  (*pIsrCbf)(void),
                                          PNIO_ISO_OBJ_HNDL*  pObjHnd);

PNIO_UINT32   PNIO_IsoObjCheck         (PNIO_ISO_OBJ_HNDL   ObjHnd);

PNIO_UINT32   PNIO_IsoFreeObj          (PNIO_ISO_OBJ_HNDL  ObjHnd);

PNIO_UINT32   PNIO_IsoActivateGpioObj  (PNIO_UINT32                 Gpio,
                                        PNIO_UINT32                 DelayTim_ns,
                                        PNIO_ISO_GPIO_LEVEL_TYPE    GpioLevelType,
                                        PNIO_ISO_OBJ_HNDL*          pObjHnd);

PNIO_VOID PNIO_cbf_report_ARFSU_record( PNIO_UINT8          ARFSU_enabled,
                                        PNIO_UINT8          ARFSU_changed );

PNIO_VOID PNIO_cbf_new_plug_ind( PNIO_DEV_ADDR *pAddr,
                                 PNIO_UINT32   InputDataLen,
                                 PNIO_UINT32   OutputDataLen);

PNIO_VOID PNIO_cbf_new_pull_ind( PNIO_DEV_ADDR       *pAddr );

#define PNIO_ARFSU_ENABLED      1
#define PNIO_ARFSU_DISABLED     0
#define PNIO_ARFSU_CHANGED      1
#define PNIO_ARFSU_NOT_CHANGED  0

double PNIO_log10   (double value);
double PNIO_pow     (double base, double exponent);

#ifndef __cplusplus
#ifndef SPI_IN_USE
#define SPI_IN_USE
unsigned char spi_in_use;
#endif
#endif

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
