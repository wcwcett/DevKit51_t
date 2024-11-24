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
/*  F i l e               &F: pnpb_api.c                                :F&  */
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
* @file     pnpb_api.c
* @brief    Interface api functions which are leading to PNPB module
*
*/

// *** general includes ***
#include "pniousrd.h"
#include "version_dk.h"
#include "xx_mem.h"

// *** platform dependent includes ***
#include "pndv_inc.h"
#include "pnpb.h"

// *-----------------------------------------------------------------
// * defines
// *-----------------------------------------------------------------

#ifdef LTRC_ACT_MODUL_ID
	#undef LTRC_ACT_MODUL_ID
#endif

#define LTRC_ACT_MODUL_ID   201
#define PNPB_MODULE_ID      201


// *-----------------------------------------------------------------
// * external functions
// *-----------------------------------------------------------------

// *-----------------------------------------------------------------
// * external variables
// *-----------------------------------------------------------------
extern PNPB_REAL PnpbMod;
extern PNPB_INSTANCE    Pnpb;

// *=================================================================
// * public variables
// *=================================================================
PNIO_UINT32 PnpbLastApiReqError = 0;
LSA_UINT8*  pRxTxMem = NULL;

// *-----------------------------------------------------------------
// * static variables
// *-----------------------------------------------------------------

// pay attention to section settings in the linker file and the MPU settings for the appropriate region
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
static LSA_UINT8 rxtx_mem[RX_TX_MEM_SIZE] __attribute__ ((section(".uncached_mem")));
#else
#error "platform not defined"
#endif

typedef struct
{
  PNIO_CP_CBF       pCbf;
  PNIO_BOOL         Active;
} REGISTER_CBF;

static REGISTER_CBF RegCbf_IrtTransEnd;


// *-----------------------------------------------------------------
// *  PNIO_init
// *  must be called first, to start the stack and application
// *  initializes static and public variables
// *-----------------------------------------------------------------
PNIO_VOID  PNIO_init (PNIO_VOID)
{
#if(1 == IOD_USED_WITH_XHIF_HOST)
    //init ertec ready signal
    REG32(U_GPIO__GPIO_PORT_MODE_0_L) &= ~( 0x000000C0 );     //function == GPIO for  3
    REG32(U_GPIO__GPIO_IOCTRL_0) &= ~( 0x00000008 );          //output
    REG32(U_GPIO__GPIO_OUT_SET_0) |= 0x00000008;          	  //set output
#endif

    // *------------------------------------------------*
    // *  initialize interfaces to OS, BSP, Socket
    // *------------------------------------------------*
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)

	if( MODE_SPI_0x03 == ( REG32( U_SCRB__BOOT_REG ) &0xf ) )
	{
		FLASH_INIT;                             //SPI for nv data
	}

#endif
	PNPB_OS_INIT();		                        // operating system init
	
    TRACE_INIT (PNIO_TRACE_BUFFER_SIZE);        // do not call before PNPB_OS_INIT()
    PnpbNvInit();

    // *------------------------------------------------*
    // *  initialize RX/TX memory
    // *------------------------------------------------*
    pRxTxMem = &rxtx_mem[0];	//Allocation based on linker script

    PNPB_MEMSET(&RegCbf_IrtTransEnd, 0, sizeof (RegCbf_IrtTransEnd));
}


// *----------------------------------------------------------------*
// *  PNIO_setup ()
// *----------------------------------------------------------------*
PNIO_UINT32 PNIO_setup  (PNIO_INT8	 *pStationName,     // unique station name
                         PNIO_UINT16 StationNameLen,    // length of station name (may not be zero terminated)
						 PNIO_INT8	 *pStationType,     // station type (must fit to GSDML file)
                         PNIO_UINT32 IpAddr,            // IP Suite: IP Addr
                         PNIO_UINT32 SubnetMask,        // IP Suite: Subnet Mask
                         PNIO_UINT32 DefRouterAddr)     // IP Suite: default router address
{
    PNIO_UINT32 Status = PNIO_OK;
    PNIO_UINT32 StationTypeLen;

    PnpbSetLastError (PNIO_ERR_INIT_VALUE);     // reset initial error value

    StationTypeLen = PNPB_STR_LEN((PNIO_CHAR*)pStationType);

    // *** check parameter ***
    if (StationTypeLen >= PNPB_MAX_STATION_TYPE_LEN)
    {
        PNPB_API_TRACE_01(LSA_TRACE_LEVEL_ERROR,  "API: PNIO_setup invalid StationTypeLen = %d \n", StationTypeLen);
        PnpbSetLastError (PNIO_ERR_IOD_INVALID_DEVTYPE);
        return (PNIO_NOT_OK);
    }

    // *** check parameter ***
    if (StationNameLen >= PNPB_MAX_STATION_TYPE_LEN)
    {
        PNPB_API_TRACE_01(LSA_TRACE_LEVEL_ERROR,  "API: PNIO_setup invalid StationNameLen = %d \n", StationNameLen);

        PnpbSetLastError (PNIO_ERR_IOD_INVALID_DEVNAME);
        return (PNIO_NOT_OK);
    }

    PnpbNvSetStationName (pStationName, StationNameLen);     // store station name in PNPB
    PnpbNvSetStationType (pStationType, StationTypeLen);     // store station type in PNPB
    PnpbNvSetIpSuite     (IpAddr,SubnetMask, DefRouterAddr); // store IP suite in PNPB

    PNPB_API_TRACE_01(LSA_TRACE_LEVEL_NOTE_HIGH,  "API: PNIO_setup State=%d \n", Status);

    return (Status);
}

// *----------------------------------------------------------------*
// *  PNIO_pdev_setup ()
// *----------------------------------------------------------------*
//lint -e{832, 578} Parameter 'Symbol' not explicitly declared, int  assumed
PNIO_UINT32 PNIO_pdev_setup(PNIO_SUB_LIST_ENTRY   *pIoSubList,            // plugged submodules, including PDEV
                            PNIO_UINT32           NumOfSubListEntries)    // number of entries in pPioSubList
{
    PNIO_UINT32 Status = PNIO_OK;
    PNIO_UINT32 i;
    PNIO_UINT32 PDEVnum = 0;

    for (i = 0; i < NumOfSubListEntries; i++)
    {
        if (pIoSubList[i].Subslot & 0x8000)
        {
            /* PDEV record found */
            PDEVnum++;
        }
    }

    if (PDEVnum)
    {
        /* Alloc space for found records */
        PNIO_SUB_LIST_ENTRY *PDEVSubList;
        PNPB_ALLOC_MEM((PNIO_VOID**)&PDEVSubList, 0, sizeof(PNIO_SUB_LIST_ENTRY) * PDEVnum);

        PDEVnum = 0; /* Use variable as index in following loop */
        for (i = 0; i < NumOfSubListEntries; i++)
        {
            if (pIoSubList[i].Subslot & 0x8000)
            {
                /* PDEV record found - copy its content */
                PNPB_COPY_BYTE(&PDEVSubList[PDEVnum], &pIoSubList[i], sizeof(PNIO_SUB_LIST_ENTRY));
                PDEVnum++;
            }
        }

        /* Save list of PDEV */
        PnpbNvSetSubmodId(PDEVSubList, PDEVnum);
    }
    else
    {
        Status = PNIO_NOT_OK;
    }

    return Status;
}

// *----------------------------------------------------------------*
// *  PNIO_shutdown ()
// *----------------------------------------------------------------*
PNIO_UINT32 PNIO_shutdown (PNIO_VOID)
{
    PNIO_printf ("PNIO_shutdown not implemenented\n");
    PNPB_API_TRACE_00 (LSA_TRACE_LEVEL_FATAL, "PNIO_shutdown not implemenented\n");
    return (PNIO_NOT_OK);
}


// *----------------------------------------------------------------*
// *  PNIO_device_open ()
// *----------------------------------------------------------------*
PNIO_UINT32  PNIO_device_open
							(PNIO_UINT16		    VendorId,		    // [in]  vendor id (see gsdml file)
							 PNIO_UINT16		    DeviceId,		    // [in]  device id (see gsdml file)
							 PNIO_ANNOTATION*	    pDevAnnotation,     // [in]  device annotation
                             PNIO_SNMP_LLDP*        pSnmpPar,           // [in]  snmp parameters for the LLDP MIB
							 PNIO_UINT32		    *pDevHndl)          // [out] device handle
{
    PNIO_UINT32 Status = PNIO_OK;

    // initialize board support package
    Bsp_Init ();
    // ***** save Vendor- and Device-ID ***
    PnpbNvSetVendorAndDeviceId (VendorId, DeviceId);
    PnpbNvSetAnnotation        (pDevAnnotation);
    PnpbNvSetSnmpData          (pSnmpPar);

    // * ------------------------------------
    // * start PNIO stack
    // * ------------------------------------

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
    XX_MEM_init(RX_TX_MEM_SIZE, 0xee, pRxTxMem, XX_MEM_DO_NOT_PRESET, 0xDD);
#else
#error ("no valid platform selected")
#endif
    tskma_init();
    tskma_open();
    // * ------------------------------------
    // * wait until PNDV has been started
    // * ------------------------------------
    while (pnpb_get_state () < PNPB_PNDV_STARTED)
    {
        PNPB_WAIT_MS(10);
    }
    pnpb_sub_real_init ();
    PNPB_API_TRACE_03(LSA_TRACE_LEVEL_NOTE_HIGH,
                      "API: PNIO_DevOpen State=%d V%d D%d  \n",
                       Status, VendorId, DeviceId );

    // **** return the device handle to the calling function
    *pDevHndl = PNIO_SINGLE_DEVICE_HNDL;                // only one device available in this version
    return (Status);
}

// *------------------------------------------------------------------------
// *         PNIO_device_ar_abort (DevHndl, ArNum)
// *------------------------------------------------------------------------
PNIO_UINT32 PNIO_device_ar_abort (PNIO_UINT32 DevHndl, PNIO_UINT16 ArNum)
{
    PNIO_UINT32 Status = PNIO_OK;

    // ***** remove submodule in real config *****
    Status = pnpb_device_ar_abort (ArNum);

    // *----------------------------------------------------------
    // * unlock access to PnpbMod from other contexts
    // *----------------------------------------------------------
    return (Status);
}

// *----------------------------------------------------------------*
// *  PNIO_device_close ()
// *----------------------------------------------------------------*
PNIO_UINT32  PNIO_device_close
						  (PNIO_UINT32 DeviceHndl)        // device handle
{
    LSA_UNUSED_ARG (DeviceHndl);

    PNPB_API_TRACE_00 (LSA_TRACE_LEVEL_FATAL, "PNIO_device_close() not implemented\n");
    return (PNIO_NOT_OK);
}

// *-----------------------------------------------------------------
// * PNIO_device_stop ()
// *-----------------------------------------------------------------
PNIO_UINT32  PNIO_device_start  ( PNIO_UINT32 DevHndl ) // deactivate readiness to connect
{
    PNIO_UINT32 Status = PNIO_OK;

    pnpb_activate();

    return (Status);
}

// *-----------------------------------------------------------------
// * PNIO_device_stop ()
// *-----------------------------------------------------------------
PNIO_UINT32  PNIO_device_stop  ( PNIO_UINT32 DevHndl ) // deactivate readiness to connect
{
    PNIO_UINT32 Status = PNIO_OK;

    pnpb_deactivate();

    return (Status);
}

PNIO_UINT32 PNIO_change_ip_suite ( PNIO_UINT32 newIpAddr, PNIO_UINT32 SubnetMask, PNIO_UINT32 DefRouterAddr )
{
    PNIO_UINT32 Status = PNIO_OK;

    // setup to pnpb nv space
    PnpbNvSetIpSuite( newIpAddr, SubnetMask, DefRouterAddr );

    // send ip address to stack
    Pnpb_set_new_address( PNPB_CTRL_IP_SUITE );

    return (Status);
}

PNIO_UINT32 PNIO_change_device_name ( PNIO_INT8 *pStationName, PNIO_UINT16 NameLength )
{
    PNIO_UINT32 Status = PNIO_OK;

    // setup to pnpb nv space
    PnpbNvSetStationName( pStationName, NameLength );

    // send device name to stack
    Pnpb_set_new_address( PNPB_CTRL_STATION_NAME );

    return (Status);
}

// *----------------------------------------------------------------*
// *  PNIO_sub_plug_list ()
// *----------------------------------------------------------------*
PNIO_UINT32   PNIO_sub_plug_list (PNIO_UINT32           DevHndl,
                                  PNIO_SUB_LIST_ENTRY   *pIoSubList,           // plugged submodules, including PDEV
                                  PNIO_UINT32           NumOfSubListEntries,   // number of entries in pIoSubList
                                  PNIO_IM0_LIST_ENTRY   *pIm0List,             // list of IM0 data sets
                                  PNIO_UINT32           NumOfIm0ListEntries,   // number of entries in pIm0List
                                  PNIO_UINT32*          pStatusList)           // list of return-Stati[NumOfSublistEntries]
{
    PNIO_UINT32 Status = PNIO_OK;

    LSA_UNUSED_ARG (DevHndl);

    // *----------------------------------------------------------
    // * lock access to PnpbMod from other contexts
    // *----------------------------------------------------------
    PNPB_ENTER(OS_MUTEX_PNPB_PLUG);
    PNPB_ENTER(OS_MUTEX_PNPB);

    Status = pnpb_sub_real_plug_list
                       (pIoSubList,           // plugged submodules, including PDEV
                        NumOfSubListEntries,  // number of entries in pIoSubList
                        pIm0List,             // list of IM0 data sets
                        NumOfIm0ListEntries,  // number of entries in pIm0List
                        pStatusList);         // pointer to list of return values (stati)

    // *----------------------------------------------------------
    // * unlock access to PnpbMod from other contexts
    // *----------------------------------------------------------
    PNPB_EXIT(OS_MUTEX_PNPB);
    PNPB_EXIT(OS_MUTEX_PNPB_PLUG);

    PNPB_API_TRACE_05(LSA_TRACE_LEVEL_NOTE_HIGH,
                      "API: subPlugList(pSubL(%d)=0x%x pIm0(%d)=0x%x pStat=0x%x\n",
                      NumOfSubListEntries,
                      pIoSubList,
                      NumOfIm0ListEntries,
                      pIm0List,
                      pStatusList);

    return (Status);
}


// *-----------------------------------------------------------------
// * PNIO_sub_plug ()
// *-----------------------------------------------------------------
PNIO_UINT32 PNIO_sub_plug
               (PNIO_UINT32		    DevHndl,
                PNIO_UINT32         ApiNum,
                PNIO_DEV_ADDR	    *pAddr,
                PNIO_UINT32		    ModIdent,
                PNIO_UINT32		    SubIdent,
                PNIO_UINT32         InputDataLen,
                PNIO_UINT32         OutputDataLen,
                PNIO_IM0_SUPP_ENUM  Im0Support,
                IM0_DATA            *pIm0Dat,       // pointer to IM0 data
                PNIO_UINT8          IopsIniVal)     // initial value for iops-input, ONLY FOR SUBMOD WITHOUT IO DATA (e.g. PDEV)
{
    PNIO_UINT32 Status = PNIO_OK;
    PNIO_UINT32 Index = 0;

    // *----------------------------------------------------------
    // * lock access to PnpbMod from other contexts
    // *----------------------------------------------------------
    PNPB_ENTER(OS_MUTEX_PNPB_PLUG);
    PNPB_ENTER(OS_MUTEX_PNPB);

    // *----------------------------------------------------------
    // * plug single submodule
    // *----------------------------------------------------------
    Status = pnpb_sub_real_plug (DevHndl,
                                 ApiNum,
                                 pAddr,
                                 ModIdent,
                                 SubIdent,
                                 InputDataLen,
                                 OutputDataLen,
                                 Im0Support,
                                 pIm0Dat,
                                 IopsIniVal,
                                 &Index,
                                 PNIO_FALSE);  // no more follows --> process request in PNDV directly

    // **** increment number of submoduls
    if (Status == PNIO_OK)
    {
        PnpbMod.NumOfPluggedSub++;        // increment number of plugged submodules

        if ((0 != Index) && (PNPB_INVALID_INDEX != Index))
        {
            if (InputDataLen)
            {
                PnpbMod.Sub[Index].DataDirection |= PNIO_SUB_PROP_IN;
            }
            if (OutputDataLen)
            {
                PnpbMod.Sub[Index].DataDirection |= PNIO_SUB_PROP_OUT;
            }
        }
    }

    // *----------------------------------------------------------
    // * unlock access to PnpbMod from other contexts
    // *----------------------------------------------------------
    PNPB_EXIT(OS_MUTEX_PNPB);
    PNPB_EXIT(OS_MUTEX_PNPB_PLUG);

    PNPB_API_TRACE_07(LSA_TRACE_LEVEL_NOTE_HIGH,
                      "API: subPlug(A%d S%d SS%d MID%d SID%d IOPS%d more%d) state%d Err%d\n",
                      ApiNum,
                      pAddr->Geo.Slot,
                      pAddr->Geo.Subslot,
                      ModIdent,
                      SubIdent,
                      IopsIniVal,
                      Status);

    /* if there are no active ARs, just send acknowledgement to user */
    PNIO_UINT32 i, Num_Of_Ar = 0;
    for( i = 0; i < IOD_CFG_NUMOF_AR; i++ )
	{
    	if( PNPB_AR_OFFLINE != Pnpb.ArState[ i ] )
    	{
    		Num_Of_Ar++;
    	}
	}
    if( 0 == Num_Of_Ar )
    {
    	PNIO_cbf_async_req_done( DevHndl, 0, PNIO_ALM_RET_OF_SUB, ApiNum, pAddr, 0, 0 );
    }
    return (Status);
}


// *-----------------------------------------------------------------
// * PNIO_Api_sub_pull ()
// *-----------------------------------------------------------------
PNIO_UINT32 PNIO_sub_pull(	PNIO_UINT32   DevHndl,
                            PNIO_UINT32   ApiNum,
                            PNIO_DEV_ADDR *pAddr)
{
    PNIO_UINT32 Status = PNIO_OK;

    // *----------------------------------------------------------
    // * lock access to PnpbMod from other contexts
    // *----------------------------------------------------------
    PNPB_ENTER(OS_MUTEX_PNPB_PLUG);
    PNPB_ENTER(OS_MUTEX_PNPB);

    // ***** remove submodule in real config *****
    Status = pnpb_sub_real_pull (DevHndl, ApiNum, pAddr);

    // *----------------------------------------------------------
    // * unlock access to PnpbMod from other contexts
    // *----------------------------------------------------------
    PNPB_EXIT(OS_MUTEX_PNPB);
    PNPB_EXIT(OS_MUTEX_PNPB_PLUG);

    /* if there are no active ARs, just send acknowledgement to user */
    PNIO_UINT32 i, Num_Of_Ar = 0;
    for( i = 0; i < IOD_CFG_NUMOF_AR; i++ )
	{
    	if( PNPB_AR_OFFLINE != Pnpb.ArState[ i ] )
    	{
    		Num_Of_Ar++;
    	}
	}
    if( 0 == Num_Of_Ar )
    {
    	PNIO_cbf_async_req_done( DevHndl, 0, PNIO_ALM_RET_OF_SUB, ApiNum, pAddr, 0, 0 );
    }

    return (Status);
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
 * Interface function which call internal PNPB functions for adding channel diagnostic
 */
PNPB_CODE_FAST PNIO_UINT32  PNIO_diag_channel_add
						(PNIO_UINT32	    DevHndl,		// device handle
                         PNIO_UINT32        Api,            // application process identifier
						 PNIO_DEV_ADDR      *pAddr,			// geographical or logical address
						 PNIO_UINT16	    ChannelNum,		// channel number
						 PNIO_UINT16        ErrorNum,	    // error number, see PNIO specification coding of "ChannelErrorType"
						 DIAG_CHANPROP_DIR  ChanDir,	    // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
						 DIAG_CHANPROP_TYPE ChanTyp,	    // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
                         PNIO_BOOL          MaintenanceReq, // maintenance required
                         PNIO_BOOL          MaintenanceDem, // maintenance demanded
						 PNIO_UINT16	    DiagTag)		// user defined diag tag != 0
{
    PNIO_UINT32 Status = PNIO_OK;

    Status = pnpb_diag_channel_add
						(DevHndl,		    // device handle
                         Api,               // application process identifier
						 pAddr,			    // geographical or logical address
						 ChannelNum,		// channel number
						 ErrorNum,	        // error numberg
						 ChanDir,	        // channel error type
						 ChanTyp,	        // channel error type
                         MaintenanceReq,
                         MaintenanceDem,
						 DiagTag);  	      // user defined diag tag != 0

    return (Status);
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
 * Interface function which call internal PNPB functions for removing channel diagnostic
 */
PNPB_CODE_FAST PNIO_UINT32  PNIO_diag_channel_remove
						(PNIO_UINT32	    DevHndl,		// device handle
                         PNIO_UINT32        Api,            // application process identifier
						 PNIO_DEV_ADDR      *pAddr,			// geographical or logical address
						 PNIO_UINT16	    ChannelNum,		// channel number
                         PNIO_UINT16        ErrorNum,       // error number, see PNIO specification coding of "ChannelErrorType"
						 DIAG_CHANPROP_DIR  ChanDir,	    // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
						 DIAG_CHANPROP_TYPE ChanTyp,	    // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
						 PNIO_UINT16	    DiagTag,		// user defined diag tag != 0
                         PNIO_UINT16        AlarmState)     // DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
{
    PNIO_UINT32 Status = PNIO_OK;
    if ((AlarmState !=DIAG_CHANPROP_SPEC_ERR_DISAPP) && (AlarmState !=DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE))
    {
        PNPB_API_TRACE_01(LSA_TRACE_LEVEL_ERROR,  "invalid param AlarmState = %d\n", AlarmState);
        return (PNIO_NOT_OK);
    }

    Status = pnpb_diag_channel_remove
						(DevHndl,		    // device handle
                         Api,               // application process identifier
						 pAddr,			    // geographical or logical address
						 ChannelNum,		// channel number
                         ErrorNum,          // error number, see PNIO specification coding of "ChannelErrorType"
						 ChanDir,	        // channel error type
						 ChanTyp, 	        // channel error type
						 DiagTag,           // user defined diag tag != 0
                         AlarmState);       // DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
return (Status);
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
 * Interface function which call internal PNPB functions for adding channel diagnostic
 */
PNPB_CODE_FAST PNIO_UINT32  PNIO_ext_diag_channel_add
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
						 PNIO_UINT16	    DiagTag)			// user defined diag tag != 0
{
    PNIO_UINT32 Status = PNIO_OK;

    Status = pnpb_ext_diag_channel_add
						(DevHndl,		      // device handle
                         Api,                 // application process identifier
						 pAddr,			      // geographical or logical address
						 ChannelNum,		  // channel number
						 ErrorNum,	          // error numberg
						 ChanDir,	          // channel error type
						 ChanTyp,	          // channel error type
						 ExtChannelErrType,	  // channel error type           (see PNIO spec.)
						 ExtChannelAddValue,  // extended channel add. value  (see PNIO spec.)
						 MaintenanceReq,
						 MaintenanceDem,
						 DiagTag);  	      // user defined diag tag != 0
    return (Status);
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
 * Interface function which call internal PNPB functions for removing channel diagnostic
 */
PNPB_CODE_FAST PNIO_UINT32  PNIO_ext_diag_channel_remove
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
                         PNIO_UINT16        AlarmState)         // DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
{
    PNIO_UINT32 Status = PNIO_OK;

    if ((AlarmState !=DIAG_CHANPROP_SPEC_ERR_DISAPP) && (AlarmState !=DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE))
    {
        PNPB_API_TRACE_01(LSA_TRACE_LEVEL_ERROR,  "invalid param AlarmState = %d\n", AlarmState);
        return (PNIO_NOT_OK);
    }
    Status = pnpb_ext_diag_channel_remove
                        (DevHndl,               // device handle
                         Api,                   // application process identifier
                         pAddr,                 // geographical or logical address
                         ChannelNum,            // channel number
                         ErrorNum,              // error number, see PNIO specification coding of "ChannelErrorType"
                         ChanDir,               // channel error type
                         ChanTyp,               // channel error type
                         ExtChannelErrType,     // channel error type           (see PNIO spec.)
                         ExtChannelAddValue,    // extended channel add. value  (see PNIO spec.)
						 DiagTag,		        // user defined diag tag != 0
                         AlarmState);           // DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE

    return (Status);
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
 * Interface function which call internal PNPB functions for adding channel diagnostic
 */
PNIO_UINT32  PNIO_diag_generic_add
						(PNIO_UINT32	    DevHndl,		// device handle
                         PNIO_UINT32        Api,            // application process identifier
						 PNIO_DEV_ADDR      *pAddr,			// geographical or logical address
                         PNIO_UINT16        ChanNum,        // channel number
		                 DIAG_CHANPROP_DIR  ChanDir,        // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		                 DIAG_CHANPROP_TYPE ChanTyp,        // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
						 PNIO_UINT16	    DiagTag,		// user defined diag tag != 0
		                 PNIO_UINT16	    UserStructIdent,// structure of info data (see IEC 61158)
						 PNIO_UINT8		    *pInfoData,		// diag data
						 PNIO_UINT32	    InfoDataLen, 	// length of diag data in bytes
                         PNIO_BOOL          MaintenanceReq, // maintenance required
                         PNIO_BOOL          MaintenanceDem) // maintenance demanded
{
    PNIO_UINT32 Status = PNIO_OK;

    Status = pnpb_diag_generic_add
						(DevHndl,		        // device handle
                         Api,                   // application process identifier
						 pAddr,			        // geographical or logical address
						 ChanNum,		        // channel number
						 ChanDir,		        // channel direction (in, out, in/out)
						 ChanTyp,		        // channel type (BYTE, WORD,..)
						 DiagTag,		        // user defined diag tag != 0
                         UserStructIdent,       // structure of info data (see IEC 61158)
						 pInfoData,		        // diag data
						 InfoDataLen, 	        // length of diag data in bytes
                         MaintenanceReq,
                         MaintenanceDem);

    return (Status);
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
 * Interface function which call internal PNPB functions for removing channel diagnostic
 */
PNIO_UINT32  PNIO_diag_generic_remove
						(PNIO_UINT32	    DevHndl,		    // device handle
                         PNIO_UINT32        Api,                // application process identifier
						 PNIO_DEV_ADDR      *pAddr,			    // geographical or logical address
                         PNIO_UINT16        ChanNum,            // channel number
		                 DIAG_CHANPROP_DIR  ChanDir,            // channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
		                 DIAG_CHANPROP_TYPE ChanTyp,            // channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
						 PNIO_UINT16	    DiagTag,		    // user defined diag tag != 0
		                 PNIO_UINT16	    UserStructIdent)    // structure of info data (see IEC 61158)
{
    PNIO_UINT32 Status = PNIO_OK;

    Status = pnpb_diag_generic_remove
						(DevHndl,		        // device handle
                         Api,                   // application process identifier
						 pAddr,			        // geographical or logical address
						 ChanNum,		        // channel number
						 ChanDir,		        // channel direction (in, out, in/out)
						 ChanTyp,		        // channel type (BYTE, WORD,..)
						 DiagTag,		        // user defined diag tag != 0
                         UserStructIdent);      // structure of info data (see IEC 61158)
    return (Status);
}


// *-----------------------------------------------------------------
// * PNIO_process_alarm_send  ()
// *-----------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32  PNIO_process_alarm_send
						(PNIO_UINT32	 DevHndl,		// device handle
                         PNIO_UINT32     Api,           // application process identifier
						 PNIO_DEV_ADDR	 *pAddr,		// geographical or logical address
						 PNIO_UINT8		 *pData,		// AlarmItem.Data
						 PNIO_UINT32	 DataLen,		// length of AlarmItem.Data
						 PNIO_UINT16	 UserStructIdent,// AlarmItem.UserStructureIdentifier, s. IEC61158-6
				 		 PNIO_UINT32	 UserHndl) 		// user defined handle
{
    PNIO_UINT32 Status = PNIO_OK;

    Status = pnpb_process_alarm_send
					(DevHndl,		    // device handle
                     Api,               // application process identifier
					 pAddr,		        // geographical or logical address
					 pData,		        // AlarmItem.Data
					 DataLen,		    // length of AlarmItem.Data
					 UserStructIdent,   // AlarmItem.UserStructureIdentifier, s. IEC61158-6
				 	 UserHndl); 		// user defined handle

    /* if there are no active ARs, just send acknowledgement to user */
    PNIO_UINT32 i, Num_Of_Ar = 0;
    for( i = 0; i < IOD_CFG_NUMOF_AR; i++ )
	{
    	if( PNPB_AR_OFFLINE != Pnpb.ArState[ i ] )
    	{
    		Num_Of_Ar++;
    	}
	}
    if( 0 == Num_Of_Ar )
    {
    	PNIO_cbf_async_req_done( DevHndl, 0, PNIO_ALM_PROC, Api, pAddr, 0, UserHndl );
    }
    return (Status);
}

// *-----------------------------------------------------------------
// * PNIO_status_alarm_send  ()
// *-----------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32  PNIO_status_alarm_send
						(PNIO_UINT32	 DevHndl,		// device handle
                         PNIO_UINT32     Api,           // application process identifier
						 PNIO_DEV_ADDR	 *pAddr,		// geographical or logical address
						 PNIO_UINT8		 *pData,		// AlarmItem.Data
						 PNIO_UINT32	 DataLen,		// length of AlarmItem.Data
						 PNIO_UINT16	 UserStructIdent,// AlarmItem.UserStructureIdentifier, s. IEC61158-6
				 		 PNIO_UINT32	 UserHndl) 		// user defined handle
{
    PNIO_UINT32 Status = PNIO_OK;

    Status = pnpb_status_alarm_send
					(DevHndl,		    // device handle
                     Api,               // application process identifier
					 pAddr,		        // geographical or logical address
					 pData,		        // AlarmItem.Data
					 DataLen,		    // length of AlarmItem.Data
					 UserStructIdent,   // AlarmItem.UserStructureIdentifier, s. IEC61158-6
				 	 UserHndl); 		// user defined handle

    /* if there are no active ARs, just send acknowledgement to user */
    PNIO_UINT32 i, Num_Of_Ar = 0;
    for( i = 0; i < IOD_CFG_NUMOF_AR; i++ )
	{
    	if( PNPB_AR_OFFLINE != Pnpb.ArState[ i ] )
    	{
    		Num_Of_Ar++;
    	}
	}
    if( 0 == Num_Of_Ar )
    {
    	PNIO_cbf_async_req_done( DevHndl, 0, PNIO_ALM_STATUS, Api, pAddr, 0, UserHndl );
    }
    return (Status);
}


// *-----------------------------------------------------------------
// * PNIO_ret_of_sub_alarm_send  ()
// *-----------------------------------------------------------------
PNIO_UINT32  PNIO_ret_of_sub_alarm_send
						(PNIO_UINT32	DevHndl,		// device handle
                         PNIO_UINT32    Api,            // application process identifier
						 PNIO_DEV_ADDR  *pAddr,			// geographical or logical address
				 		 PNIO_UINT32	 UserHndl) 		// user defined handle
{
    PNIO_UINT32 Status = PNIO_OK;

    Status = pnpb_ret_of_sub_alarm_send ( DevHndl, Api, pAddr, UserHndl);

    return (Status);
}


// *-----------------------------------------------------------------
// * PNIO_upload_retrieval_alarm_send  ()
// *-----------------------------------------------------------------
PNIO_UINT32  PNIO_upload_retrieval_alarm_send
                        (PNIO_UINT32    DevHndl,       // device handle
                         PNIO_UINT32    Api,           // application process identifier
                         PNIO_DEV_ADDR  *pAddr,        // geographical or logical address
                         PNIO_UINT8     *pData,        // AlarmItem.Data
                         PNIO_UINT32    DataLen,       // length of AlarmItem.Data
                         PNIO_UINT32    UserHndl)      // user defined handle
{
    PNIO_UINT32 Status = PNIO_OK;

    Status = pnpb_upload_retrieval_alarm_send (DevHndl, Api, pAddr, pData, DataLen, UserHndl);
    return (Status);
}


// *-----------------------------------------------------------------
// * PNIO_set_dev_state  ()
// *-----------------------------------------------------------------
PNIO_UINT32 PNIO_set_dev_state (PNIO_UINT32 DevHndl,   // device handle
                                PNIO_UINT32 DevState)  // device state CLEAR/OPERATE
{
    PNIO_UINT32 Status = PNIO_OK;

    Status = pnpb_set_dev_state (DevHndl, DevState);
    return (Status);
}

// *-----------------------------------------------------------------
// * PNIO_trigger_pndv_ds_rw_done  ()
// *-----------------------------------------------------------------
//lint -e{832,578} Parameter 'Symbol' not explicitly declared, PNIO_INT assumed
PNIO_UINT32  PNIO_trigger_pndv_ds_rw_done (PNIO_ERR_STAT* PnioStat, PNIO_UINT32 bufLen)
{
    PNDV_RQB_DS* pDsPeri;
    PNIO_UINT32* vu32 = (PNIO_UINT32*) PnioStat;
    pDsPeri = &pPnpbIf->ds_rw.dfc_ds_struc_peri;

    pDsPeri->event_type.u.sv.ar_rec.cm_pnio_err = PNPB_HTONL(*vu32);
    pDsPeri->event_type.u.sv.ar_rec.ret_val_1   = PnioStat->AddValue1;
    pDsPeri->event_type.u.sv.ar_rec.ret_val_2   = PnioStat->AddValue2;
    pDsPeri->event_type.u.sv.ar_rec.data_length = CM_RECORD_OFFSET + bufLen;

    /* Add1, Add2 not used */
    pnpb_write_event_to_pndv(PNDV_EV_TO_PNDV_DS_RW_DONE, 0, 0, NULL);
    PNPB_TRIGGER_PNDV ();

    return PNIO_OK;
}

// *-----------------------------------------------------------------
// * PNIO_rec_read_rsp  ()
// *-----------------------------------------------------------------
PNIO_UINT32  PNIO_rec_read_rsp
         (
            PNIO_VOID*          pRqHnd,         // request handle from PNIO_cbf_rec_read
            PNIO_UINT8*         pDat,		    // data pointer (may be 0 if error)
            PNIO_UINT32         NettoDatLength, // total length of read record data
            PNIO_ERR_STAT*      pPnioStat       // PNIO state pointer (may be 0, if no error)
         )
{
    PNIO_UINT32 Status = PNIO_OK;

    Status = pnpb_rec_read_rsp (pRqHnd, pDat, NettoDatLength, pPnioStat);
    return (Status);
}


// *-----------------------------------------------------------------
// * PNIO_rec_write_rsp  ()
// *-----------------------------------------------------------------
PNIO_UINT32  PNIO_rec_write_rsp
         (
            PNIO_VOID*          pRqHnd,         // request handle from PNIO_cbf_rec_read
            PNIO_UINT32         NettoDatLength, // total length of written record data
            PNIO_ERR_STAT*      pPnioStat       // PNIO state pointer (may be 0, if no error)
         )
{
    PNIO_UINT32 Status = PNIO_OK;

    Status = pnpb_rec_write_rsp (pRqHnd, NettoDatLength, pPnioStat);
    return (Status);
}


// *-----------------------------------------------------------------
// * struct ASYNC_REC_DATA
// *-----------------------------------------------------------------
typedef struct
{
    PNIO_UINT32 Api;
    PNIO_UINT32 Slot;
    PNIO_VOID*  pRecDatInPerif;
    PNIO_UINT32 EntityIndex;
} ASYNC_REC_DATA;


// *-----------------------------------------------------------------
// * PNIO_rec_set_rsp_async  ()
// *-----------------------------------------------------------------
PNIO_VOID*  PNIO_rec_set_rsp_async (PNIO_VOID)
{
    // set asynchronous response mode (for 1 Request)
    return ((PNIO_VOID*)pnpb_rec_set_rsp_async());
}


// *-----------------------------------------------------------------
// * PNIO_netcom_enable (..)
// * enables the receiving of ethernet telegrams.
// * Before calling this function all received ethernet frames will
// * be discarded.
// *-----------------------------------------------------------------
PNIO_UINT32 PNIO_netcom_enable ()
{
    return (PNIO_OK);
}


// *-----------------------------------------------------------------
// * PNIO_get_version (..)
// * reads the version of the devkit software.
// *-----------------------------------------------------------------
PNIO_UINT32 PNIO_get_version (PNIO_DK_VERSION *pVersion)
{
	pVersion->item_hh = DEVKIT_VERSION_HH;	// highest element
	pVersion->item_h  = DEVKIT_VERSION_H ;
	pVersion->item_l  = DEVKIT_VERSION_L;
	pVersion->item_ll = DEVKIT_VERSION_LL;	// lowest element
	return (PNIO_OK);
}


// *---------------------------------------------------------------*
// *
// *   PnpbSetLastError (PNIO_VOID)
// *---------------------------------------------------------------
// *
// *   sets the last error variable to the actual error value
// *---------------------------------------------------------------*
//lint -e{832, 578} Parameter 'Symbol' not explicitly declared, int assumed
PNIO_VOID PnpbSetLastError (PNIO_ERR_ENUM LastErr)
{
  PnpbLastApiReqError = (LSA_UINT32) LastErr;
}


// *---------------------------------------------------------------*
// *
// *   PNIO_get_last_error (PNIO_VOID)
// *---------------------------------------------------------------
// *
// *   handles a more detailed error number to the application.
// *   The number was saved as result of the last CM request and
// *   is converted to a user - error number.
// *---------------------------------------------------------------*
PNIO_ERR_ENUM	PNIO_get_last_error (PNIO_VOID)
{
	PNIO_ERR_ENUM LastErr;

	// ****** return numbers PNIO_ERR_RANGE_BEGIN...ERR_RANGE_END directly**
	if ((PnpbLastApiReqError >= PNIO_ERR_RANGE_BEGIN)
	&&	(PnpbLastApiReqError <= PNIO_ERR_RANGE_END))
	{
		return ((PNIO_ERR_ENUM) PnpbLastApiReqError);
	}
	LastErr = (PNIO_ERR_ENUM)PnpbLastApiReqError;
	PNPB_API_TRACE_01 (LSA_TRACE_LEVEL_ERROR, "Undefined Error 0x%x", PnpbLastApiReqError);
	return (LastErr);
}


// *------------------------------------------------------------------------
// *		PNIO_initiate_data_read()
// *
// *		initiates reading the RT and/or IRT output data from stack and
// *        writing them to the physical output
// *------------------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32 PNIO_initiate_data_read   (PNIO_UINT32   DevHndl)
{
#ifdef PNPB_SYSTEM_REDUNDANCY
	PNIO_UINT32 i;
	pnpb_s2_present = PNPB_NO_S2_AR;
	for( i = 0; i <= PNPB_MAX_AR; i++ )
	{
		if( PNIO_AR_TYPE_SINGLE_SYSRED == pnpb_ArType[ i ] )
		{
			pnpb_s2_present = PNPB_S2_AR_PRESENT;
		}
	}
	if( PNPB_S2_AR_PRESENT != pnpb_s2_present )	/*redundancy S1 - no redundancy*/
	{
    return (pnpb_initiate_data_read (DevHndl));
	}
	else	/*redundancy S2 - */
	{
		/*check rdht timeouts first*/

		for (i = 0; i < PNPB_MAX_S2_AR_SETS; i++)
		{
			if (PNIO_TRUE == pnpb_data.timer[i].rqb_in_use)
			{
				pnpb_rdht_timeout(i);
			}
		}
		return (pnpb_initiate_s2_data_read (DevHndl));
	}

#else
    return (pnpb_initiate_data_read (DevHndl));
#endif
}


// *------------------------------------------------------------------------
// *		PNIO_initiate_data_write()
// *
// *		initiates writing the RT and/or IRT output data to stack
// *
// *------------------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32 PNIO_initiate_data_write (PNIO_UINT32  DevHndl)
{
#ifdef PNPB_SYSTEM_REDUNDANCY
	if( PNPB_S2_AR_PRESENT != pnpb_s2_present )	/*redundancy S1 - no redundancy*/
	{
    return (pnpb_initiate_data_write (DevHndl));
	}
	else
	{
		return (pnpb_initiate_s2_data_write (DevHndl));
	}

#else
    return (pnpb_initiate_data_write (DevHndl));
#endif
}


// *------------------------------------------------------------------------
// *		PNIO_get_last_apdu_status()
// *
// *		gets the value of thee remote apdu status from last
// *        output data access
// *------------------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32 PNIO_get_last_apdu_status (PNIO_UINT32  DevHndl, PNIO_UINT32  ArNum)
{
    return (pnpb_get_last_apdu_status (ArNum));
}


// *------------------------------------------------------------------------
// *        PNIO_dbai_enter (),
// *        PNIO_dbai_exit ()
// *
// *        occupies/frees semaphore of the IOD thread. So application access to
// *        common data is protected against PNIO_cbf_xxx() functions.
// *------------------------------------------------------------------------
PNIO_VOID PNIO_dbai_enter (PNIO_VOID)
{
    PNPB_ENTER(OS_MUTEX_PNPB_IO);
}

PNIO_VOID PNIO_dbai_exit (PNIO_VOID)
{
    PNPB_EXIT(OS_MUTEX_PNPB_IO);
}

// *------------------------------------------------------------------------
// *        PNIO_dbai_buf_lock ()
// *
// *        locks a data buffer for a specified output IOCR for direct IO
// *        data access.
// *------------------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32 PNIO_dbai_buf_lock  (PNIO_BUFFER_LOCK_TYPE*  pLock)
{
    PNIO_UINT32   Status    = PNIO_OK;
    PNIO_UINT32*  pApduStat = NULL;

    // *** parameter check ***
    if ((pLock == NULL) || (pLock->ArNum == 0) || (pLock->ArNum > IOD_CFG_NUMOF_AR))
    {
        return (PNIO_NOT_OK);
    }
    pLock->pBuf = NULL;     // Preset value: iocr buffer = 0

    // **** get IOCR buffer ****
    switch (pLock->IoDir)
    {
        case PNIO_IOCR_TYPE_INPUT:
            Status = PnpbProvLock (&(pLock->pBuf), (PNIO_UINT16) (pLock->ArNum - 1), 0, 0xFFFF);
            PnpbCopyRecReadIoData(pLock->pBuf, ( pLock->ArNum - 1 ) , 0, 0xFFFF);
            break;
        case PNIO_IOCR_TYPE_OUTPUT:
             Status = PnpbConsLock (&(pLock->pBuf), &pApduStat, (PNIO_UINT16) (pLock->ArNum - 1));
             if (pApduStat)
                 pLock->ApduStat = *pApduStat;
             else
                 pLock->ApduStat = 0;
             break;
        default:
             return (PNIO_NOT_OK);
    }
    if (pLock->pBuf == NULL)
    {
        return (PNIO_ERR_NOT_ACCESSIBLE);
    }
    return (Status);
}


// *------------------------------------------------------------------------
// *        PNIO_dbai_buf_unlock()
// *
// *        unlocks a data buffer for a specified output IOCR for direct IO
// *        data access.
// *------------------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32 PNIO_dbai_buf_unlock   (PNIO_BUFFER_LOCK_TYPE*  pLock)
{
    PNIO_UINT32    Status = PNIO_OK;

    if (pLock == NULL)
    {
         return (PNIO_NOT_OK);
    }
    // **** get IOCR buffer ****
    switch (pLock->IoDir)
    {
        case PNIO_IOCR_TYPE_INPUT:
            Status = PnpbProvUnlock ((PNIO_UINT16)(pLock->ArNum - 1));
            if (pLock->ArNum)
                PnpbExp[pLock->ArNum-1].IoUpdatePending = PNIO_FALSE;   // notify to PNPB,that Input update has been done.
            break;
        case PNIO_IOCR_TYPE_OUTPUT:
            Status = PnpbConsUnlock ((PNIO_UINT16)(pLock->ArNum - 1));
            break;
        default:
            return (PNIO_NOT_OK);
    }
    return (Status);
}


// *------------------------------------------------------------------------
// *		PNIO_set_iops()
// *
// *		changes the IOPS value of submodules, that have no IO data
// *
// *		input:  Subslot     subslot number of PDEV (0x8000, 0x8001,..)
// *		        Iops        PNIO_S_GOOD, PNIO_S_BAD
// *
// *
// *------------------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32 PNIO_set_iops (PNIO_UINT32 Api,
                           PNIO_UINT32 SlotNum,
                           PNIO_UINT32 SubNum,
                           PNIO_UINT8  Iops)
{
    return (pnpb_set_iops (Api,SlotNum, SubNum, Iops));
}


#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)

    // *----------------------------------------------------------------*
    // *  PNIO_IsoActivateIsrObj ()
    // *
    // *----------------------------------------------------------------*
    // *  installs an isr handler, that is executed with a defined .
    // *  delay to NewCycle. The service searches for a free object,
    // *  configures it and returns an object handle.
    // *  Input:
    // *    pIspCbf             pointer to vcallback function
    // *    DelayTim_ns         delay value to new cycle  in nsec
    // *    pObjHnd             return handle to Isr object
    // *
    // *  output:
    // *    return              PNIO_OK, PNIO_NOT_OK
    // *----------------------------------------------------------------*
    PNIO_UINT32   PNIO_IsoActivateIsrObj(   PNIO_VOID          (*pIsrCbf)(PNIO_VOID),
                                            PNIO_UINT32         DelayTim_ns,
                                            PNIO_ISO_OBJ_HNDL*  pObjHnd)
    {
        evma_alloc_params_t param;
        evma_handle_t  ObjHnd  = 0;

        PNIO_EVMA_INSTANCE_HEADER * eih = ( PNIO_EVMA_INSTANCE_HEADER * ) *pObjHnd;
        if ( ( PNIO_EVMA_STATE_IN_USE == eih->state ) && ( PNIO_EVMA_INST_DESC_MAGIC == eih->magic_number ))
        {
        	PNIO_printf("Timer already in use. Free it first! ");
        	return( PNIO_FALSE );
        }

        param.application_type = EVMA_APPLICATION_ARM_ICU;
        param.spec_params.icu_params.event_cbf = pIsrCbf;
        ObjHnd  = evma_alloc_event(&param, DelayTim_ns);
        if (ObjHnd)
        {
            *pObjHnd = ObjHnd;
            PNPB_API_TRACE_03(LSA_TRACE_LEVEL_NOTE,  "IsoObject(pIsr=0x%x Delay=%d pObj=0x%x)\n",
                              pIsrCbf,
                              DelayTim_ns,
                              ObjHnd);
            return (PNIO_TRUE);
        }
        else
        {
            *pObjHnd = 0;
            PNPB_API_TRACE_01(LSA_TRACE_LEVEL_ERROR,  "ERROR IsoObject (Delay=%d \n", DelayTim_ns);
            return (PNIO_NOT_OK);
        }
    }

    // *----------------------------------------------------------------*
    // *  PNIO_IsoActivateTransEndObj ()
    // *
    // *----------------------------------------------------------------*
    // *  installs an isr handler, that is executed after end of trans-
    // *  mission.
    // *
    // *  Input:
    // *    pIspCbf             pointer to vcallback function
    // *    pObjHnd             return handle to Isr object
    // *
    // *  output:
    // *    return              PNIO_OK, PNIO_NOT_OK
    // *----------------------------------------------------------------*
    PNIO_UINT32   PNIO_IsoActivateTransEndObj(  PNIO_VOID           (*pIsrCbf)(PNIO_VOID),
                                                PNIO_ISO_OBJ_HNDL*   pObjHnd)
    {
        evma_alloc_params_t param;
        evma_handle_t  ObjHnd  = 0;

        param.application_type = EVMA_APPLICATION_ARM_ICU;
        param.spec_params.icu_params.event_cbf = pIsrCbf;
        ObjHnd  = evma_alloc_event_transfer_end(&param);
        if (ObjHnd)
        {
            *pObjHnd = ObjHnd;
            PNPB_API_TRACE_02(LSA_TRACE_LEVEL_NOTE,  "IsoObject(TE, pIsr=0x%x pObj=0x%x)?n",
                              pIsrCbf,
                              ObjHnd);
            return (PNIO_TRUE);
        }
        else
        {
            *pObjHnd = 0;
            PNPB_API_TRACE_00(LSA_TRACE_LEVEL_ERROR,  "ERROR IsoTransEndObject\n");
            return (PNIO_FALSE);
        }
    }

    PNIO_UINT32   PNIO_IsoObjCheck  (PNIO_ISO_OBJ_HNDL ObjHnd)
    {
        if(ObjHnd != 0)
        {
            PNIO_EVMA_INSTANCE_HEADER * eih = ( PNIO_EVMA_INSTANCE_HEADER * ) ObjHnd;
            if(PNIO_EVMA_STATE_IN_USE != eih->state)
            {
                return PNIO_NOT_OK;
            }else
            {
                return PNIO_OK;
            }
        }else
        {
            return PNIO_NOT_OK;
        }

    }


    // *----------------------------------------------------------------*
    // *  PNIO_IsoFreeObj ()
    // *
    // *----------------------------------------------------------------*
    // *  deactivates the specified object.
    // *    pObjHnd             return handle to Isr object
    // *
    // *  output:
    // *    return              PNIO_OK, PNIO_NOT_OK
    // *----------------------------------------------------------------*
    PNIO_UINT32   PNIO_IsoFreeObj   (PNIO_ISO_OBJ_HNDL ObjHnd)
    {
        PNIO_UINT8 Status;

        /*RQ1851162 fatal error by deactivating not activated object*/
        /*Report error and don't go to evma*/

        PNIO_EVMA_INSTANCE_HEADER * eih = ( PNIO_EVMA_INSTANCE_HEADER * ) ObjHnd;
        if ( ( PNIO_EVMA_STATE_IN_USE != eih->state ) || ( PNIO_EVMA_INST_DESC_MAGIC != eih->magic_number ) )
        {
        	PNIO_printf("ERROR: Trying to deactivate non-existing object %x\n", ObjHnd);
            return (PNIO_FALSE);
        }
        else
        {
			Status = evma_free_event (ObjHnd);
			if (Status == LSA_RET_OK)
			{
				PNPB_API_TRACE_01(LSA_TRACE_LEVEL_NOTE,  "IsoFreeObject(Obj=0x%x)\n", ObjHnd);
				return (PNIO_TRUE);
			}
			else
			{
				PNPB_API_TRACE_01(LSA_TRACE_LEVEL_ERROR,  "ERROR IsoFreeObject(Obj=0x%x)\n", ObjHnd);
				return (PNIO_FALSE);
			}
        }
    }


    // *----------------------------------------------------------------*
    // *  PNIO_IsoActivateGpioObj ()
    // *
    // *----------------------------------------------------------------*
    // *  installs an isr handler, that is executed with a defined .
    // *  delay to NewCycle. The service searches for a free object,
    // *  configures it and returns an object handle.
    // *  Input:
    // *    pIspCbf             pointer to vcallback function
    // *    DelayTim_ns         delay value to new cycle  in nsec
    // *    pObjHnd             return handle to Isr object
    // *
    // *  output:
    // *    return              PNIO_OK, PNIO_NOT_OK
    // *----------------------------------------------------------------*
    PNIO_UINT32   PNIO_IsoActivateGpioObj  (PNIO_UINT32                 Gpio,
                                            PNIO_UINT32                 DelayTim_ns,
                                            PNIO_ISO_GPIO_LEVEL_TYPE    GpioLevelType,
                                            PNIO_ISO_OBJ_HNDL*          pObjHnd)
    {
        evma_alloc_params_t param;
        evma_handle_t  ObjHnd  = 0;

        if ((Gpio == 0) || (Gpio > 7))
        {
            return (PNIO_FALSE);
        }

        Bsp_SetGpioMode (Gpio,          // GPIO number (1...7)
                         01,            // alternate function 01 = PNPLL_OUT
                         GPIO_DIR_OUT); // direction output


        param.application_type = EVMA_APPLICATION_GPIO;

        param.spec_params.gpio_params.gpio_index = Gpio;
        if (GpioLevelType == ISO_GPIO_HIGH_ACTIVE)
        {
            param.spec_params.gpio_params.level_type = EVMA_GPIO_HIGH_ACTIVE;
        }
        else
        {
            param.spec_params.gpio_params.level_type = EVMA_GPIO_LOW_ACTIVE;
        }

        ObjHnd  = evma_alloc_event(&param, DelayTim_ns);

        if (ObjHnd)
        {
            *pObjHnd = ObjHnd;
            PNPB_API_TRACE_04(LSA_TRACE_LEVEL_NOTE,  "IsoObjectGpio (GPIO=%d Delay=%d Typ=%d pObj=0x%x\n",
                              Gpio,
                              DelayTim_ns,
                              GpioLevelType,
                              ObjHnd);
            return (PNIO_TRUE);
        }
        else
        {
            *pObjHnd = 0;
            PNPB_API_TRACE_01(LSA_TRACE_LEVEL_ERROR,  "ERROR IsoObject(Delay=%d \n", DelayTim_ns);
            return (PNIO_FALSE);
        }

    }


    // *-----------------------------------------------------------*
    // *  PNIO_CP_register_cbf
    // *  registers a user defined callback function
    // *  input:   AppHndl      user defined handle, returned to
    // *                        user in the callback function
    // *           CbeType      specifies, if StartOp, OpFault, NewCycle
    // *                        shall  be signaled via cbf
    // *           pCbf         start address of  the callback function
    // *
    // *  output   return       Status  (PNIO_OK)
    // *-----------------------------------------------------------*
    PNIO_ISO_OBJ_HNDL PnpbTransEndHndl = 0;
    PNIO_ISO_OBJ_HNDL PnpbNewCycHndl   = 0;

    PNIO_UINT32 PNIO_CP_register_cbf(PNIO_CP_CBE_TYPE CbeType, PNIO_CP_CBF pCbf)
    {
        PNIO_UINT32 Status = PNIO_OK;

        PNPB_API_TRACE_02(LSA_TRACE_LEVEL_NOTE,
                          "Register Cbf (Type=%d pCbf=0x%x)\n",
                          CbeType, pCbf);

        if (CbeType == PNIO_CP_CBE_TRANS_END_IND)
        {
            Status = (pCbf) ? PNIO_IsoActivateTransEndObj(pCbf, &PnpbTransEndHndl) : PNIO_IsoFreeObj (PnpbTransEndHndl);
        }
        if (CbeType == PNIO_CP_CBE_NEWCYCLE_IND)
        {
            Status = (pCbf) ? PNIO_IsoActivateIsrObj(pCbf, 0, &PnpbNewCycHndl) : PNIO_IsoFreeObj (PnpbNewCycHndl);
        }
        PNPB_API_TRACE_03(LSA_TRACE_LEVEL_NOTE,
                          "Register Cbf (Type=%d pCbf=0x%x State=%d)\n",
                          CbeType, pCbf, Status);
        return (Status);
    }
#endif


    PNIO_VOID PNIO_Fatal(PNIO_VOID)
    {
        LSA_FATAL_ERROR_TYPE PnioErr;
        PnioErr.lsa_component_id  = PNIO_PACKID_IODAPI;
        PnioErr.module_id         = PNPB_MODULE_ID;
        PnioErr.line              = 0;
        PnioErr.error_code[0]     = 0;
        PnioErr.error_code[1]     = 0;
        PnioErr.error_code[2]     = 0;
        PnioErr.error_code[3]     = 0;
        PnioErr.error_data_length = 0;
        PnioErr.error_data_ptr    = LSA_NULL;
        PNPB_FATAL_ERROR(&PnioErr);
    }


    PNIO_VOID PNIO_hw_watchdog_init(PNIO_UINT32 time, PNIO_WD_GRANITY granity)
    {
        Bsp_hw_watchdog_init(time, granity);
    }


    PNIO_VOID PNIO_hw_watchdog_deinit(PNIO_VOID)
    {
        Bsp_hw_watchdog_deinit();
    }


    PNIO_VOID PNIO_hw_watchdog_start(PNIO_VOID)
    {
        Bsp_hw_watchdog_start();
    }


    PNIO_VOID PNIO_hw_watchdog_stop(PNIO_VOID)
    {
        Bsp_hw_watchdog_stop();
    }


    PNIO_VOID PNIO_hw_watchdog_trigger(PNIO_VOID)
    {
        Bsp_hw_watchdog_trigger();
    }


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
