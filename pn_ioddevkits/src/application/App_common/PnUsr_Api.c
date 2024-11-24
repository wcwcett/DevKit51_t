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
/*  F i l e               &F: PnUsr_Api.c                               :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Implementation of the simple IO interface                                */
/*                                                                           */
/*  Features:                                                                */
/*                                                                           */
/*  To use this application example, set #define EXAMPL_DEV_CONFIG_VERSION 3 */
/*  in file \application\usriod_cfg.h                                        */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*  note: this application is a simple demo application. To keep it simple,  */
/*        the return values of the IodApixx functions are often not analyzed.*/
/*        But it is strongly recommended, to do this in a real application.  */
/*                                                                           */
/*  THIS MODULE HAS TO BE MODIFIED BY THE PNIO USER                          */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/

// *--------------------------------------------------------------------------
// *  FUNCTIONAL DESCRIPTION
// *
// *
// *
// *--------------------------------------------------------------------------
// * NOTES:
// *  1) RT and IRT have the same application interface for IO data exchange.
// *
// *  2) In our example only the DAP has its own IM0 data (this is mandatory). All other modules
// *     (PDEV, IO-modules) don't have its own IM0 data (this is an option), so they have to respond
// *     with the IM data of the next higher level. In our example only the DAP has IM0 data,
// *     so all other subslots respond with the IM0 data of the DAP.
// *
// *--------------------------------------------------------------------------


#include "compiler.h"
#include "usriod_cfg.h"
#include "version_dk.h"

#if (EXAMPL_DEV_CONFIG_VERSION <= 9) || (21 == EXAMPL_DEV_CONFIG_VERSION) || (44 == EXAMPL_DEV_CONFIG_VERSION)
    #include "pniousrd.h"
    #include "iodapi_event.h"
    #include "os.h"
    #include "os_taskprio.h"
    #include "usriod_diag.h"
    #include "usriod_utils.h"
    #include "usriod_im_func.h"
    #include "pnio_trace.h"
    #include "iodapi_rema.h"
    #include "nv_data.h"
    #include "PnUsr_Api.h"
    #include "usriod_PE.h"
#if IOD_INCLUDE_AMR
    #include  "usriod_AMR.h"
#endif
    #include "usriod_utils.h"
    #include "bspadapt.h"
    #include "bspledadapt.h"


#include <iod_cfg.h>
#if(1 == IOD_USED_WITH_XHIF_HOST)
#include "PnUsr_xhif.h"
#endif

	#define LTRC_ACT_MODUL_ID   10

    /*== defines ================================================================*/
    #define DEBUG_TRC_UPLOAD        0


    // *----------------------------------------------------------------*
    // *
    // *  MainAppl(void)

    /*===========================================================================*/
    #if (PNIOD_PLATFORM &  PNIOD_PLATFORM_ECOS_EB200P)
        PNIO_INT32       IOReductFact = 4;         // exchange IO data between application and stack every IOReductFact bus cycle
    #endif

	#if DEBUG_TASKCNT
		extern PNIO_UINT32 		TaskCnt[MAXNUM_OF_TASKS];
	#endif


    // *------------ external functions  ----------*
    //lint -e{578} Declaration of symbol 'Symbol' hides symbol 'Symbol' (Location)
    void PnUsr_cbf_iodapi_event_varinit (PNIO_SUB_LIST_ENTRY* pIoSubList,
 									     PNIO_UINT32          NumOfSubListEntries);

	#if (PNIO_TRACE == PNIO_TRACE_DK_LSA)
	PNIO_UINT32 TraceLsaRdRecord (PNIO_UINT32   RecordIndex,
                                  PNIO_UINT32   *pBufLen,		// [in,out] length to read, out: length, read by user
	                              PNIO_UINT8    *pBuffer,     	// [in] buffer pointer
                                  PNIO_ERR_STAT	*pPnioState  	// [in,out] return status, only modify if error
                                 );
	#endif
    // *------------ public data  ----------*
    PNIO_UINT32 AsyncRecRdWrMode    = PNIO_FALSE;

    // *------------ local data  ----------*
                    PNIO_UINT32   TskId_CycleIO;        // task id for executing cyclic data exchange
    static          PNIO_UINT32   Task_CycleIO_Active;  // activate/deactivate cyclic data exchange
    static          PNIO_UINT32   TransEndSemId;        // semaphore to trigger io thread at newCycle

    // *------------ local data for record read/write ----------*
    static  PNIO_VOID*      AsyncRecHndl = NULL;
    static  PNIO_UINT32     AsyncRecLen  = 0;
    static  PNIO_VOID*      pAsyncRecBuf = NULL;
    static  PNIO_ANNOTATION DevAnnotation;

    #if (PNIOD_PLATFORM &  PNIOD_PLATFORM_ECOS_EB200P)
        PNIO_UINT32 ReductionValue;
    #endif


    // *----------------------------------------------------------------*
    // *
    // *  UsrBuildDeviceAnnotation (void)
    // *----------------------------------------------------------------*
    // *  this function builds a device annotation string, used for function
    // *  PNIO_device_open.  Because some of the annotation structure
    // *  elements are also needed in the I&M functions, it is placed into
    // *  this modulule.
    // *  UsrBuildDeviceAnnotation must be called once during system
    // *  startup, before ImFunctions () is called.
    // *----------------------------------------------------------------*
    PNIO_ANNOTATION*  UsrIod_BuildDeviceAnnotation (void)
    {
        OsMemSet (&DevAnnotation, ' ', sizeof (DevAnnotation)); // preset value = <Blank>

        OsMemCpy (DevAnnotation.DeviceType,
                IOD_CFG_DEVICE_TYPE,                        // annotation.DeviceType  and DCP.DeviceVendorValue must fit together
                OsStrLen (IOD_CFG_DEVICE_TYPE));

        OsMemCpy (DevAnnotation.OrderId,
                IOD_CFG_DEV_ANNOTATION_ORDER_ID,
                OsStrLen (IOD_CFG_DEV_ANNOTATION_ORDER_ID));

        OsMemCpy (DevAnnotation.SerialNumber,
                IOD_CFG_IM0_SERIAL_NUMBER,
                OsStrLen (IOD_CFG_IM0_SERIAL_NUMBER));

		DevAnnotation.HwRevision = IOD_CFG_HW_REVISION;  // defined as big endian in PNIO spec.




        { // * --------------------------------------------------------
          // * in this example we read the version from the PNIO devkit
          // * user may create his own product version here
          // * --------------------------------------------------------
            DevAnnotation.SwRevisionPrefix  = DEVKIT_VERSION_PREFIX;    // maybe  V, R, P, U, T
            DevAnnotation.SwRevision1       = DEVKIT_VERSION_HH;        // sw-revision, highest number
            DevAnnotation.SwRevision2       = DEVKIT_VERSION_H;         // sw-revision, high number
            DevAnnotation.SwRevision3       = DEVKIT_VERSION_L;         // sw-revision, lowest number
        }
        return (&DevAnnotation);
    }



// ***==================================================================================***
// ***
// ***           IO ACCESS INTERFACE
// ***
// ***==================================================================================***
    // *----------------------------------------------------------------*
    // *
    // *  PnUsr_ActivateIoDatXch ()
    // *
    // *----------------------------------------------------------------*
    // *  activates the call of the trigger function PNIO_cbf_trigger_io_exchange
    // *----------------------------------------------------------------*
    // *  Input:     --
    // *  Output:    --
    // *----------------------------------------------------------------*
    PNIO_VOID PnUsr_ActivateIoDatXch    (void)      // activate IO Data exchange in task Task_CycleIO

    {
        Task_CycleIO_Active = 1;
    }

    // *----------------------------------------------------------------*
    // *
    // *  PnUsr_DeactivateIoDatXch ()
    // *
    // *----------------------------------------------------------------*
    // *  deactivates the call of the trigger function PNIO_cbf_trigger_io_exchange
    // *----------------------------------------------------------------*
    // *  Input:     --
    // *  Output:    --
    // *----------------------------------------------------------------*
    PNIO_VOID PnUsr_DeactivateIoDatXch    (void)      // activate IO Data exchange in task Task_CycleIO

    {
        Task_CycleIO_Active = 0;
    }

#if (PNIOD_PLATFORM &  PNIOD_PLATFORM_ECOS_EB200P)
        // *----------------------------------------------------------------*
        // *
        // *  PNIO_cbf_trigger_io_exchange ()
        // *
        // *----------------------------------------------------------------*
        // *  This callback function is executed by the PNIO stack, when the
        // *  transmission of IO data has been finished. This is the trigger for
        // *  the application, to read and handle the output data (device is
        // *  consumer) from the IO controller and write the input data
        // *  (device is provider) to the PNIO stack.
        // *
        // *  This trigger can be used for the application, to start io data
        // *  exchange between application and stack.
        // *
        // *  The trigger is divided from the local hardware clock of the ERTEC
        // *  and works for RT and for IRT in the same way. The difference
        // *  between IRT and RT is, that for IRT all local clocks of all
        // *  devices and controllers in the same sync-domain are synchronized
        // *  to the sync-master.
        // *
        // *  Function PNIO_cbf_trigger_io_exchange() only sends a trigger to
        // *  the cycle IO task via semaphore. Optional a reduction value can
        // *  be set.
        // *
        // *  NOTE:  Strongly avoid a reentrant call of UsrDbai_initiate_data_write()
        // *         or UsrDbai_initiate_data_read(), also PNIO_initiate_data_read()
        // *         or PNIO_initiate_data_write().
        // *
        // *         PNIO_cfg_trigger_io_exchange() is called in ecos DSR state.
        // *         Here any waiting service calls like enter- and exit-functions
        // *         do not work!!
        // *----------------------------------------------------------------*
        // *  Input:     --
        // *  Output:    --
        // *----------------------------------------------------------------*

    OS_CODE_FAST void PNIO_cbf_trigger_io_exchange (void)
    {
        if (Task_CycleIO_Active)
        {
#ifdef PNIO_USES_REDUCTION_RATIO
            ReductionValue++;
            if (ReductionValue >= 4)   // exchange IO data between application and stack every 4th bus cycle
            {
                ReductionValue = 0;

                // ** send a trigger to Task_CycleIO, which handles cyclic RT data **
                OsGiveSemB (TransEndSemId);
            }
#else
#if(1 == IOD_USED_WITH_XHIF_HOST)
            /* Exchange cyclical IO data via xhif */
            PnUsr_xhif_cycl_gpio_trigger();
#endif  /* 1 == IOD_USED_WITH_XHIF_HOST */
            OsGiveSemB (TransEndSemId);
#endif  /* #ifdef PNIO_USES_REDUCTION_RATIO */
        }
    }
#endif

    // *----------------------------------------------------------------*
    // *
    // *  Task_CycleIO (void)
    // *
    // *----------------------------------------------------------------*
    // *  cyclic exchange of IO data
    // *
    // *  This task performs the cyclic IO data exchange in an endless
    // *  loop.  Every IO data exchange (one data read and one data write)
    // *         is triggered by a semaphore, that is set in PNIO_cbf_trigger_io_exchange()
    // *
    // *----------------------------------------------------------------*
    // *  Input:    ----
    // *  Output:   ----
    // *
    // *----------------------------------------------------------------*
    OS_CODE_FAST static PNIO_INT32 Task_CycleIO(void)
    {
#if DEBUG_TASKCNT
        LSA_UINT32  taskID;
#endif

        // *----------------------------------------------------------
        // * Synchronization to parent process
        // *----------------------------------------------------------
        OsWaitOnEnable();            // must be first call in every task
#if DEBUG_TASKCNT
        taskID = OsGetThreadId();
#endif

        //PnUsr_ActivateIoDatXch();   // start cyclic io data exchange

        while (1)
        {
            // **** receive a cyclic trigger, to start initiate data rd/wr ***
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
            OsTakeSemB (TransEndSemId);
#else
            OsWait_ms (20);
#endif

#if DEBUG_TASKCNT
            TaskCnt[taskID]++;
#endif

            PnUsr_cbf_IoDatXch ();
        }
        return (0); /*lint !e527 Unreachable code */
    }


// ***==================================================================================***
// ***
// ***           ACYCLIC SERVICES INTERFACE
// ***
// ***==================================================================================***
    // *----------------------------------------------------------------*
    // *
    // *  PlugSubmodules (Api)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  This function hands over the system configuration to the pnio
    // *  stack. It is done by plugging all modules and submodules.
    // *
    // *
    // *  The module IDs and submodule ID's must correspond the the
    // *  appropriate entry in the XML GSD file and the configuration
    // *  from Step7 / NCM-PC.
    // *
    // *  Input:    ----
    // *  Output:   ----
    // *
    // *----------------------------------------------------------------*
    //lint -e{832, 578} Parameter 'Symbol' not explicitly declared, int assumed
    static PNIO_UINT32   PlugSubmodules
                            (PNIO_SUB_LIST_ENTRY    *pIoSubList,           // plugged submodules, including PDEV
                             PNIO_UINT32            NumOfSubListEntries,   // number of entries in pIoSubList
                             PNIO_IM0_LIST_ENTRY    *pIm0List,             // list of IM0 data sets
                             PNIO_UINT32            NumOfIm0ListEntries)   // number of entries in pIm0List
    {

	    PNIO_UINT32   Status = PNIO_OK;
        PNIO_UINT32*  pStatusList = NULL;
        PNIO_UINT32   i;
        // *=================================================================
        // *  alloc memory for error list
        // *=================================================================
        OsAlloc ((void**)&pStatusList, 0, sizeof (PNIO_UINT32) * NumOfSubListEntries);

        // *=================================================================
        // *  plug all modules and submodules
        // *=================================================================
        PNIO_sub_plug_list (PNIO_SINGLE_DEVICE_HNDL,    // device hndl (must be PNIO_SINGLE_DEVICE_HNDL)
                            pIoSubList,                 // plugged submodules, including PDEV
                            NumOfSubListEntries,        // number of entries in pIoSubList
                            pIm0List,                   // list of IM0 data sets
                            NumOfIm0ListEntries,        // number of entries in pIm0List
                            pStatusList);               // pointer to list of return values (stati)

        // * ----------------------------------------------------
        // *   evaluate error list
        // *   note: the response values in errorList are
        // *   available after PNIO_sub_plug with MoreFollows = FALSE !
        // *   The allocated memory for pErrList must not be set free before !.
        // * ----------------------------------------------------
        for (i = 0; i < NumOfSubListEntries; i++)
        {
            if (*(pStatusList+i) == PNIO_OK)
                PNIO_printf ("SubPlugResponse(%d) = OK\n", i, *(pStatusList+i));
            else
                PNIO_printf ("SubPlugResponse(%d) = ERROR\n", i, *(pStatusList+i));
        }

        // *=================================================================
        // *  free memory for error list
        // *=================================================================
        OsFree ((void*)pStatusList);

	    return (Status);
    }


    // *-----------------------------------------------------------------------*
    // *
    // *  StartupPnioDevice (void)
    // *  performs the following steps to startup a device:
    // *
    // *   - restore REMA data, if available
    // *   - hands over IP suite and device name before pnio stack startup
    // *   - fills the callback table (needed only if activated per #define)
    // *   - builds the device annotation string (compatible to PNIO specification)
    // *   - opens a device instance
    // *   - adds all APIs (application process identifier) to device instance
    // *   - plugs all modules and submodules to the corresponding APIs
    // *   - transfers the PDEV records from REMA to the stack, if available
    // *   - set device state to OPERATE
    // *   - call device_start, to activate attendance to build up a connection
    // *     to a PNIO controller.
    // *-----------------------------------------------------------------------*
    static PNIO_UINT32  StartupPnioDevice (PNUSR_DEVICE_INSTANCE *pPnUsrDev,            // device setup configuration
                                           PNIO_SUB_LIST_ENTRY   *pIoSubList,           // plugged submodules, including PDEV
                                           PNIO_UINT32           NumOfSubListEntries,   // number of entries in pPioSubList
                                           PNIO_IM0_LIST_ENTRY   *pIm0List,             // list of IM0 data sets
                                           PNIO_UINT32           NumOfIm0ListEntries)   // number of entries in pIm0List
    {
        PNIO_UINT32         Status = PNIO_OK;     // status for return value of subroutines
        PNIO_UINT32         DevHndl= 0;           // device handle, reserved  for future use
        NV_IP_SUITE*        pNvIpSuite;           // temp pointer for non volatile IP suite
        PNIO_UINT8*         pNvDevName;           // temp pointer for non volatile device name
        PNIO_UINT32         StationNameLen;       // temp value (size of non volatile data)
        PNIO_UINT32         MemSize;              // temp value (size of non volatile data)
        PNIO_UINT8*         pNvSnmpSysName;       // temp pointer for non volatile snmp object sysName
        PNIO_UINT32         SnmpSysNameLen;
        PNIO_UINT8*         pNvSnmpSysCont;       // temp pointer for non volatile snmp object sysContact
        PNIO_UINT32         SnmpSysContLen;
        PNIO_UINT8*         pNvSnmpSysLoc;        // temp pointer for non volatile snmp object sysLocation
        PNIO_UINT32         SnmpSysLocLen;


         // *--------------------------------------------------------------
         // * initialize some static variables first
         // *--------------------------------------------------------------
         PnUsr_cbf_iodapi_event_varinit (pIoSubList, NumOfSubListEntries);

         // *--------------------------------------------------------------
         // * read the devicename, IP suite and PDEV record data from the
         // * non volatile memory (e.g. flash eprom).
         // * Bsp_nv_data_restore allocates a data block and copies the
         // * requested data into that block. application has to free the
         // * memory of that block after use by calling Bsp_nv_data_memfree().
         // *--------------------------------------------------------------
         Status = Bsp_nv_data_restore (PNIO_NVDATA_DEVICENAME,  // data type
                                       (PNIO_VOID**) &pNvDevName,             // data pointer (allocated by
                                       &StationNameLen);
         Status = Bsp_nv_data_restore (PNIO_NVDATA_IPSUITE,     // data type
                                       (PNIO_VOID**) &pNvIpSuite,             // data pointer (allocated by
                                       &MemSize);

         Status = Bsp_nv_data_restore (PNIO_NVDATA_SNMP_SYSNAME,     // data type
                                       (PNIO_VOID**) &pNvSnmpSysName,             // data pointer (allocated by
                                       &SnmpSysNameLen);
         Status = Bsp_nv_data_restore (PNIO_NVDATA_SNMP_SYSCONT,     // data type
                                       (PNIO_VOID**) &pNvSnmpSysCont,             // data pointer (allocated by
                                       &SnmpSysContLen);
         Status = Bsp_nv_data_restore (PNIO_NVDATA_SNMP_SYSLOC,     // data type
                                       (PNIO_VOID**) &pNvSnmpSysLoc,             // data pointer (allocated by
                                       &SnmpSysLocLen);


	    // *-------------------------------------------
	    // * open device (only one device is possible)
	    // *-------------------------------------------
	    // ********* open device **********
	    if (Status == PNIO_OK)
	    {
            PNIO_ANNOTATION *pDevAnnotation;
            PNIO_SNMP_LLDP  SnmpPar;
            PNIO_UINT8 * SysDescrBuff;
            PNIO_UINT32 SysDescrBuff_size;

		    // *----------------------------------------------------------
		    // * set device annotation structure. The structure consists of
		    // *
		    // *   - device type:				user specific string, max. 25 bytes
		    // *   - order ID:					user specific string, max.   bytes
		    // *   - hw revision number:			number, max. 5 bytes
		    // *   - sw revision prefix          Version, Release, Prototype, Under field test, Test device
		    // *   - sw revision number 1..3:	number, each 2 bytes
		    // *
		    // * sw revision = SwRevision1.SwRevision2.SwRevision3
		    // *----------------------------------------------------------
            pDevAnnotation = UsrIod_BuildDeviceAnnotation ();


		    // *----------------------------------------------------------
            // * set SNMP MIB parameter for LLDP MIB
            // *
            // *
            // * NOTE1:  system name, system description and snmp interface
            // *         description shall be set according to the MIB2 entries.
            // *
            // * NOTE2:  size of string must be size WITHOUT zero-terminator,
            // *         so use Len=sizeof(string)-1   or Len = OsStrLen (string)
            // *----------------------------------------------------------
            SnmpPar.pSysName    = (PNIO_INT8*)pNvSnmpSysName;
            SnmpPar.SysNameLen  = SnmpSysNameLen;
#define SNMP_SYSTEM_DESCRIPTION_ACCORDING_TO_SPH
#ifndef SNMP_SYSTEM_DESCRIPTION_ACCORDING_TO_SPH
            SnmpPar.pSysDescr   = IOD_CFG_SNMP_SYSTEM_DESCRIPTION;
            SnmpPar.SysDescrLen = OsStrLen (IOD_CFG_SNMP_SYSTEM_DESCRIPTION);
#else
            SysDescrBuff_size = 0;
            OsAllocF ((void**)&SysDescrBuff, 0xff);
            SnmpPar.pSysDescr = (PNIO_INT8*)SysDescrBuff;
            SysDescrBuff_size += PNIO_sprintf(&SysDescrBuff[SysDescrBuff_size], DEVKIT_VENDOR);
            SysDescrBuff_size += PNIO_sprintf(&SysDescrBuff[SysDescrBuff_size], ", ");
            SysDescrBuff_size += PNIO_sprintf(&SysDescrBuff[SysDescrBuff_size], DEVKIT_PRODUCTFAMILY);
            SysDescrBuff_size += PNIO_sprintf(&SysDescrBuff[SysDescrBuff_size], ", ");
            SysDescrBuff_size += PNIO_sprintf(&SysDescrBuff[SysDescrBuff_size], DEVKIT_PRODUCTNAME);
            SysDescrBuff_size += PNIO_sprintf(&SysDescrBuff[SysDescrBuff_size], ", ");
            SysDescrBuff_size += PNIO_sprintf(&SysDescrBuff[SysDescrBuff_size], DEVKIT_ORDERNUMBER);
            SysDescrBuff_size += PNIO_sprintf(&SysDescrBuff[SysDescrBuff_size], ", ");
            SysDescrBuff_size += PNIO_sprintf(&SysDescrBuff[SysDescrBuff_size], "%04d", DEVKIT_HW_REVISION);
            SysDescrBuff_size += PNIO_sprintf(&SysDescrBuff[SysDescrBuff_size], ", ");
            SysDescrBuff_size += PNIO_sprintf(&SysDescrBuff[SysDescrBuff_size], "%c%02d.%02d.%02d.%02d", DEVKIT_VERSION_PREFIX,
                                                                                                         DEVKIT_VERSION_HH,
                                                                                                         DEVKIT_VERSION_H,
                                                                                                         DEVKIT_VERSION_L,
                                                                                                         DEVKIT_VERSION_LL);
            SysDescrBuff_size += PNIO_sprintf(&SysDescrBuff[SysDescrBuff_size], ", ");
            SysDescrBuff_size += PNIO_sprintf(&SysDescrBuff[SysDescrBuff_size], DEVKIT_PRODUCTSERIALNUMBER);


            if(0xff < SysDescrBuff_size)
            {
            	SysDescrBuff_size = 0xff;
            }
            SnmpPar.SysDescrLen = SysDescrBuff_size;
#endif
            SnmpPar.pSysContact   = (PNIO_INT8*)pNvSnmpSysCont;
            SnmpPar.SysContactLen = SnmpSysContLen;

            SnmpPar.pSysLoc   = (PNIO_INT8*)pNvSnmpSysLoc;
            SnmpPar.SysLocLen = SnmpSysLocLen;

            SnmpPar.pIfDescr    = IOD_CFG_SNMP_INTERFACE_DESCRIPTION;
            SnmpPar.IfDescrLen  = OsStrLen (IOD_CFG_SNMP_INTERFACE_DESCRIPTION);

            SnmpPar.pPortName1   = IOD_CFG_SNMP_PORT1_NAME;
            SnmpPar.PortNameLen1 = OsStrLen (IOD_CFG_SNMP_PORT1_NAME);

            #if (IOD_CFG_PDEV_NUMOF_PORTS >= 2)
                SnmpPar.pPortName2   = IOD_CFG_SNMP_PORT2_NAME;
                SnmpPar.PortNameLen2 = OsStrLen (IOD_CFG_SNMP_PORT2_NAME);
            #endif

            #if (IOD_CFG_PDEV_NUMOF_PORTS >= 3)
                SnmpPar.pPortName3   = SNMP_PORT3_NAME;
                SnmpPar.PortNameLen3 = OsStrLen (SNMP_PORT3_NAME);
            #endif

            #if (IOD_CFG_PDEV_NUMOF_PORTS >= 4)
                SnmpPar.pPortName4   = SNMP_PORT4_NAME;
                SnmpPar.PortNameLen4 = OsStrLen (SNMP_PORT4_NAME);
            #endif


            // *----------------------------------------------------------
            // * Startup PNIO Stack and initialize as a pnio device
            // *
            // * get the station-name (e.g. from non volatile ram) and
            // * handle it to the pnio stack by  iod_set_station_name ().
            // * The name size is limited to DCP_MAX_STATION_NAME_LEN.
            // *
            // * Also save Vendor-ID, Device-ID and Instance ID.
            // * Set Loggging File and Log-Level  (default: none)
            // *
            // * 0:  no logging
            // * 1:  logging of error messages
            // * 2:  logging of level 1 + important notes
            // * 3:  logging of level 2 + normal notes
            // * 4:  logging of level 3 + memory allocation notes
            // *----------------------------------------------------------
            PNIO_setup  ((PNIO_INT8*)pNvDevName ,   // device name, saved in persistent memory
                            (PNIO_UINT16)StationNameLen,            // size of station name (may be not zero terminated)
                            pPnUsrDev->pDevType,	    // same name as "ProductFamily" in GSDML File
                            pNvIpSuite->IpAddr,	    // IP Addr in network format
                            pNvIpSuite->SubnetMask,	// Subnet Mask
                            pNvIpSuite->DefRouter);	// local Router

            // *----------------------------------------------------------
            // * save PDEV setup to be used by stack 
            // *----------------------------------------------------------
            PNIO_pdev_setup(pIoSubList, NumOfSubListEntries);

            // *----------------------------------------------------------
            // * open a new device instance (only one is possible)
            // *----------------------------------------------------------
            Status = PNIO_device_open(  pPnUsrDev->VendorId,		    // Vendor ID, same as in GSDML File
                                        pPnUsrDev->DeviceId,		    // device ID  same as in GSDML File
                                        pDevAnnotation,                 // device annotation
                                        &SnmpPar,                       // snmp parameter for the LLDP MIB
                                        &DevHndl);                      // out: Device Handle, created by PNIO stack



	    }

        // *-----------------------------------------
        // *    start up with bus fault LED = ON
        // *-----------------------------------------
        Bsp_EbSetLed (PNIO_LED_ERROR, 1);

	    // *----------------------------------------------------------
	    // * Plug all modules and submodules of all APIs
        // * (API = "Application Process Identifier, see PNIO specification)
	    // *----------------------------------------------------------
	    if (Status == PNIO_OK)
		    Status = PlugSubmodules(pIoSubList, NumOfSubListEntries, pIm0List, NumOfIm0ListEntries);


	    // *-----------------------------------------
	    // *    set iod state to OPERATE, NO ERROR
	    // *-----------------------------------------
	    if (Status == PNIO_OK)
		    PNIO_set_dev_state (PNIO_SINGLE_DEVICE_HNDL,
                                PNIO_DEVSTAT_OPERATE);

	    // *----------------------------------------------------------
	    // * now enable Ethernet communication. Before this all
	    // * received data packets from Ethernet will be discarded.
	    // *----------------------------------------------------------
	    if (Status == PNIO_OK)
		    Status = PNIO_netcom_enable ();

#if ( IOD_INCLUDE_AMR == 1 )
	    AMR_Init();
#endif
	    // *----------------------------------------------------------
	    // * activate device
        // * after this a controller can establish a connection
	    // *----------------------------------------------------------
	    //if (Status == PNIO_OK)
		//    Status = PNIO_device_start (PNIO_SINGLE_DEVICE_HNDL);


	    // *-----------------------------------------
	    // *    abort system startup, if an error
        // *    happened
	    // *-----------------------------------------
	    if (Status != PNIO_OK)
	    { // **** user specific error handling ****
            PNIO_printf ("error in PROFINET IO Setup \n");
	    }


	    // *-----------------------------------------
	    // *    free the memory for the restored
        // *    nv data
	    // *-----------------------------------------
        Bsp_nv_data_memfree (pNvDevName);
        Bsp_nv_data_memfree (pNvIpSuite);



        return (Status);
    }


    #if (PNIOD_PLATFORM &  PNIOD_PLATFORM_EB200P)
    extern volatile LSA_UINT32 tskma_parity_error_count;
    extern volatile LSA_UINT32 tskma_parity_error_source;
    extern volatile LSA_UINT32 tskma_access_error_count;
    extern volatile LSA_UINT32 tskma_access_error_source;
	#endif

    // *----------------------------------------------------------------*
    // *
    // *  PnUsr_DeviceSetup(void)
    // *
    // *----------------------------------------------------------------*
    // *
    // *  main application function
    // *   - starts the pnio stack
    // *   - starts user interface task
    // *   - handles user inputs and starts the selected test functions
    // *
    // *  Input:    argc			not used yet
    // *            argv			not used yet
    // *  Output:   ----
    // *
    // *----------------------------------------------------------------*
    PNIO_BOOL PnUsr_DeviceSetup (PNUSR_DEVICE_INSTANCE  *pPnUsrDev,          // device setup configuration
                                 PNIO_SUB_LIST_ENTRY    *pIoSubList,         // plugged submodules, including PDEV
                                 PNIO_UINT32            NumOfSublistEntries, // number of entries in pPioSubList
                                 PNIO_IM0_LIST_ENTRY    *pIm0List,           // list of IM0 data sets
                                 PNIO_UINT32            NumOfIm0ListEntries) // number of entries in pIm0List
    {
	    PNIO_UINT32     Status  = PNIO_NOT_OK; // return status

	    /* Clear global bss section */
	    NumOfAr = 0;

	    // *----------------------------------------------------------
	    // * Synchronization to father process. Must be called first
	    // * in every task, which is started by OsStartTask().
	    // *----------------------------------------------------------
	    Status = OsWaitOnEnable();		// must be first call

	    // *----------------------------------------------------------
	    // * creates semaphore, to trigger Task_CycleIO at new Cycle
        // * interrupt
	    // *----------------------------------------------------------
        OsAllocSemB (&TransEndSemId);

	    // *----------------------------------------------------------
	    // * creates and starts a task for initiating io data exchange
        // * betweenuser application and pnio stack. This task is cyclically
        // * triggered by a message. At every trigger it performs a
        // * PNIO_initiate_data_read() and PNIO_initiate_data_write()
	    // *----------------------------------------------------------
	    Status = OsCreateThread ((void(*)(void))Task_CycleIO, 0, (PNIO_UINT8*)"Pnio_CycleIO", TASK_PRIO_APPL_CYCLE_IO, OS_TASK_DEFAULT_STACKSIZE, &TskId_CycleIO);
		PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: OsCreateThread\n");
	    Status = OsCreateMsgQueue (TskId_CycleIO);   // install the task message queue
		PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: OsCreateMsgQueue\n");
	    Status = OsStartThread (TskId_CycleIO);		// start, after the task message queue has been installed
		PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: OsStartThread\n");

        // *-----------------------------------------
        // *    PNIO  SYSTEM Startup
        // *    creates, starts and initializes all
        // *    tasks of the PNIO stack and plugs all
        // *    submodules. After return the pnio stack
        // *    is ready for buildingup a connection
        // *    to a PNIO controller.
        // *-----------------------------------------
	    Status = StartupPnioDevice (pPnUsrDev,
                                    pIoSubList,
                                    NumOfSublistEntries,
                                    pIm0List,
                                    NumOfIm0ListEntries);
		PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: StartupPnioDevice\n");

#if (PNIOD_PLATFORM &  PNIOD_PLATFORM_ECOS_EB200P)
        ReductionValue = 0;
#endif

        // *-----------------------------------------
        // *    optional register callback, that notifies
        // *    the application every communication
        // *    cycle at begin of new cycle.
        // *    (only valid on ERTEC based platforms)
        // *-----------------------------------------
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
        if (Status == PNIO_OK)
        {
            PNIO_CP_register_cbf(PNIO_CP_CBE_TRANS_END_IND,   // transmission end
                                 (PNIO_CP_CBF)PNIO_cbf_trigger_io_exchange);
        }
#endif


	    // *-----------------------------------------
	    // *    print current devkit-version
	    // *-----------------------------------------
        PrintDevkitVersion ();


		#if (PNIOD_PLATFORM &  PNIOD_PLATFORM_EB200P)
        if (tskma_parity_error_count)
        {
            PNIO_ConsolePrintf("ERTEC200P PARITY ERROR: COUNT=%d, SOURCE=0x%08X, EDC_EVENT=0x%08X\n",
                                tskma_parity_error_count, tskma_parity_error_source, *((unsigned long*) U_SCRB__EDC_EVENT));
        }
        if (tskma_access_error_count)
        {
            PNIO_ConsolePrintf("ERTEC200P ACCESS ERROR: COUNT=%d, SOURCE=0x%08X\n",
                                tskma_access_error_count, tskma_access_error_source);
        }
		#endif

        return ((Status == PNIO_OK) ? PNIO_TRUE : PNIO_FALSE);
    }

    void PNIOUSR_device_start( PNUSR_DEVICE_INSTANCE  *pPnUsrDev,          // device setup configuration
                               PNIO_SUB_LIST_ENTRY    *pIoSubList,         // plugged submodules, including PDEV
                               PNIO_UINT32            NumOfSublistEntries, // number of entries in pPioSubList
                               PNIO_IM0_LIST_ENTRY    *pIm0List,           // list of IM0 data sets
                               PNIO_UINT32            NumOfIm0ListEntries) // number of entries in pIm0List
    {
        PNIO_UINT32         Status = PNIO_OK;     // status for return value of subroutines

        PNIO_device_start(1);

        // *----------------------------------------------------------
        // * Plug all modules and submodules of all APIs
        // * (API = "Application Process Identifier, see PNIO specification)
        // *----------------------------------------------------------
        if (Status == PNIO_OK)
            Status = PlugSubmodules(pIoSubList, NumOfSublistEntries, pIm0List, NumOfIm0ListEntries);


        // *-----------------------------------------
        // *    set iod state to OPERATE, NO ERROR
        // *-----------------------------------------
        if (Status == PNIO_OK)
            PNIO_set_dev_state (PNIO_SINGLE_DEVICE_HNDL,
                                PNIO_DEVSTAT_OPERATE);
    }

// ***==================================================================================***
// ***
// ***           EVENT INTERFACE
// ***
// ***==================================================================================***

    void AsyncRecReadDoResponseOk (void)
    {
        PNIO_rec_read_rsp (AsyncRecHndl, (PNIO_UINT8*)pAsyncRecBuf, AsyncRecLen, NULL);

        if (pAsyncRecBuf)
        {
           OsFree (pAsyncRecBuf);
           pAsyncRecBuf = NULL;
        }
    }


    void AsyncRecReadDoResponseError (void)
    {
         PNIO_ERR_STAT PnioState;
         PnioState.ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
         PnioState.ErrDecode = 0x81;  // PNIO
         PnioState.ErrCode1  = 0x08;  // Read write rec. parameter error "faulty record"
         PnioState.ErrCode2  = 0x02;  // "error in block version"
         PnioState.AddValue1 = 1234;  //
         PnioState.AddValue2 = 5678;  //
         PNIO_rec_read_rsp (AsyncRecHndl, NULL, 0, &PnioState);
         if (pAsyncRecBuf)
         {
            OsFree (pAsyncRecBuf);
            pAsyncRecBuf = NULL;
         }
   }


    void AsyncRecWriteDoResponseOk (void)
    {
        PNIO_rec_write_rsp (AsyncRecHndl, AsyncRecLen, NULL);

        if (pAsyncRecBuf)
        {
           OsFree (pAsyncRecBuf);
           pAsyncRecBuf = NULL;
        }
    }

    void AsyncRecWriteDoResponseError (void)
    {
         PNIO_ERR_STAT PnioState;
         PnioState.ErrCode   = 0xde;  // IODReadRes with ErrorDecode = PNIORW
         PnioState.ErrDecode = 0x81;  // PNIO
         PnioState.ErrCode1  = 0x08;  // Read write rec. parameter error "faulty record"
         PnioState.ErrCode2  = 0x02;  // "error in block version"
         PnioState.AddValue1 = 1234;  //
         PnioState.AddValue2 = 5678;  //
         PNIO_rec_write_rsp (AsyncRecHndl, 0, &PnioState);
         if (pAsyncRecBuf)
         {
            OsFree (pAsyncRecBuf);
            pAsyncRecBuf = NULL;
         }
   }

    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_rec_read ()
    // *
    // *----------------------------------------------------------------*
    // *
    // *  The pnio stacks notifies the user, that a read record request
    // *  has been received from the pnio controller. The user has to
    // *  provide the record data and copies them to the specified buffer
    // *  address. The maximum pBufLen is also provided in the function
    // *  parameters and might not be exceeded !.
    // *  After serving this function, the user returns the real copied data
    // *  length and the success state (PNIO_OK),
    // *
    // *  manager of the pnio stack.
    // *
    // *  Input:	PNIO_UINT32      DevHndl        [in] device handle
    // *			PNIO_UINT32		 Api			[in] application process identifier
    // *            PNIO_UINT16      ArNum,		    [in] ar - number
    // *            PNIO_UINT16 	 SessionKey,	[in] ar session number
    // * 			PNIO_UINT32      SequenceNum    [in] CLRPC sequence number
    // *			PNIO_DEV_ADDR    *pAddr,		[in] location (slot, subslot)
    // * 			PNIO_UINT32      RecordIndex    [in] record index
    // * 			PNIO_UINT32		 *pBufLen		[in]  provided data length,
    // *											[out] accepted data length
    // * 			PNIO_UINT8		 *pBuffer		[in] buffer pointer
    // * 			PNIO_ERR_STAT	 *pPnioState	4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
    // *
    // *
    // *  Output:   return		     PNIO_OK
    // *			*pBuf		     copied Record data
    // *----------------------------------------------------------------*
    PNIO_UINT32  PNIO_cbf_rec_read
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
	)
    {
	    PNIO_UINT32 Status	= PNIO_OK;
        PNIO_UINT8* pTmpBuf;

	    PNIO_UNUSED_ARG (DevHndl);
	    PNIO_UNUSED_ARG (Api);
	    PNIO_UNUSED_ARG (ArNum);
	    PNIO_UNUSED_ARG (SessionKey);
	    PNIO_UNUSED_ARG (SequenceNum);

	    PNIO_APP_ASSERT(pBufLen, "ERROR: PNIO_cbf_rec_read() called with zero pBufLen !\n");

	    // * -----------------------------------------------
        // *    test async response
        // * -----------------------------------------------
		if (AsyncRecRdWrMode == PNIO_TRUE)
		{
			OsAlloc ((void**)&pTmpBuf, 0, *pBufLen); // create tmp buffer to park user data..
			pAsyncRecBuf  = (PNIO_VOID*) pTmpBuf;
			AsyncRecLen  = *pBufLen;
			AsyncRecHndl = PNIO_rec_set_rsp_async ();
		}
		else
		{
		    PNIO_APP_ASSERT(pBuffer, "ERROR: PNIO_cbf_rec_read() called with zero pBuffer !\n");
			pTmpBuf = pBuffer;
		}

	    // *----------------------------------------------*
	    // *  handle special indizes for I&M
	    // *----------------------------------------------*
        switch (RecordIndex)
        {
            #if (PNIO_TRACE == PNIO_TRACE_DK_LSA)
			case 0x7777:	/* get trace header */
			case 0x7778:
					Status = TraceLsaRdRecord (RecordIndex, pBufLen, pTmpBuf, pPnioState);
					break;
            #endif // (PNIO_TRACE == PNIO_TRACE_DK_LSA)
#if IOD_INCLUDE_AMR
			case 0xF880:
	            // *---------- AMR response ------------*
			        Status = AMR_ResponseHandler (pAddr, pBufLen, pTmpBuf, pPnioState);
			        break;
#endif
            case 0x80a0:
                // *---------- PROFIENERGY response ------------*
                    Status = PROFIenergy_ResponseHandler (pAddr, pBufLen, pTmpBuf, pPnioState, ArNum);
                    break;

            #if (IOD_INCLUDE_IM0_4 == 0)      // only if IM is not handled inside the PN stack..
			// *---------- IM 0 ... IM 4------------*
			case 0xaff0: // **** IM 0 *****
					Status = Im0_read_Handler (Api, pAddr, pBufLen, pTmpBuf, pPnioState);
					break;
			case 0xaff1: // **** IM 1 *****
					Status = Im1_read_Handler (Api, pAddr, pBufLen, pTmpBuf, pPnioState);
					break;
			case 0xaff2: // **** IM 2 *****
					Status = Im2_read_Handler (Api, pAddr, pBufLen, pTmpBuf, pPnioState);
					break;
			case 0xaff3: // **** IM 3 *****
					Status = Im3_read_Handler (Api, pAddr, pBufLen, pTmpBuf, pPnioState);
					break;
			case 0xaff4: // **** IM 4 *****
					Status = Im4_read_Handler (Api, pAddr, pBufLen, pTmpBuf, pPnioState);
					break;
			#endif // (IOD_INCLUDE_IM0_4 == 0)
	        // *----------------------------------------------*
	        // *  the rest hand over to application
	        // *----------------------------------------------*
            default:
                    Status = (PnUsr_cbf_rec_read (DevHndl, Api, ArNum, SessionKey, SequenceNum, pAddr,
                                        	      RecordIndex, pBufLen, pTmpBuf, pPnioState));
                    break;
        }

        if (AsyncRecRdWrMode == PNIO_TRUE)
        {
            PNIO_printf ("to finish async record rd/wr, press '3' for OK or '4' for ERROR\n");
            AsyncRecLen = *pBufLen;
        }

        return (Status);
    }



    // *----------------------------------------------------------------*
    // *
    // *  PNIO_cbf_rec_write ()
    // *
    // *----------------------------------------------------------------*
    // *
    // *  The pnio stacks notifies the user, that a write record request
    // *  has been received from the pnio controller. The user has to
    // *  take the record data from the specified source buffer
    // *  The length of the provided data are specified in function parameter
    // *  *pBufLen. The user has to change this pointer, if the size of
    // *  the accepted data differs from the size of the provided data.
    // *  After serving this function, the user returns the success state (PNIO_OK),
    // *
    // *  manager of the pnio stack.
    // *
    // *  Input:	PNIO_UINT32      DevHndl        device handle
    // *			PNIO_UINT32		 Api			application process identifier
    // *            PNIO_UINT16      ArNum,		    ar - numberg
    // *            PNIO_UINT16 	 SessionKey,	ar session number
    // * 			PNIO_UINT32      SequenceNum    CLRPC sequence number
    // *			PNIO_DEV_ADDR    *pAddr,		[in] location (slot, subslot)
    // * 			PNIO_UINT32		 RecordIndex    record index
    // * 			PNIO_UINT32		 *pBufLen		[in]  provided data length,
    // *											[out] accepted data length
    // * 			PNIO_UINT8		 *pBuffer		[in] buffer pointer
    // * 			PNIO_ERR_STAT	 *pPnioState	4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
    // *
    // *  Output:   return		    PNIO_OK			ok, no error (*pPnioState must not be set)
    // *	                     	PNIO_NOT_OK		error (*pPnioState must be set)
    // *			*pBuf		    copied Record data
    // *----------------------------------------------------------------*
    PNIO_UINT32  PNIO_cbf_rec_write
	(
		PNIO_UINT32			DevHndl,        // device handle
		PNIO_UINT32			Api,            // application process identifier
        PNIO_UINT16         ArNum,			// ar - number
        PNIO_UINT16 		SessionKey,	    // ar session number
		PNIO_UINT32			SequenceNum,    // CLRPC sequence number
		PNIO_DEV_ADDR		*pAddr,			// geographical or logical address
		PNIO_UINT32			RecordIndex,    // record index
		PNIO_UINT32			*pBufLen,	    // [in, out] in: length to write, out: length, written by user
		PNIO_UINT8			*pBuffer,		// [in] buffer pointer
		PNIO_ERR_STAT		*pPnioState		// 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
	)
    {
	    PNIO_UINT32     Status = PNIO_NOT_OK;

        // *** preset error return values to zero ****
        OsMemSet (pPnioState, 0, sizeof (PNIO_ERR_STAT));

        // *---------- IM 0 ... IM 4------------*
        switch (RecordIndex)
        {
            case 0x80a0:
                    Status = PROFIenergy_RequestHandler (pAddr, pBufLen, pBuffer, pPnioState, ArNum);
                    break;

            #if (IOD_INCLUDE_IM0_4 == 0)      // only if IM is not handled inside the PN stack..
			case 0xaff0:
					Status = Im0_write_Handler (Api, pAddr, pBufLen, pBuffer, pPnioState);
					break;
			case 0xaff1:
					Status = Im1_write_Handler (Api, pAddr, pBufLen, pBuffer, pPnioState);
					break;
			case 0xaff2:
					Status = Im2_write_Handler (Api, pAddr, pBufLen, pBuffer, pPnioState);
					break;
			case 0xaff3:
					Status = Im3_write_Handler (Api, pAddr, pBufLen, pBuffer, pPnioState);
					break;
			case 0xaff4:
					Status = Im4_write_Handler (Api, pAddr, pBufLen, pBuffer, pPnioState);
					break;
            #endif // (IOD_INCLUDE_IM0_4 == 0)

            // *** transfer the following records to application ***
            case 0x8030:
            case 0xB02E:
            case 0xB02F:
            		Status = PnUsr_cbf_rec_write (DevHndl, Api, ArNum, SessionKey, SequenceNum, pAddr,
            									  RecordIndex, pBufLen, pBuffer, pPnioState);
                    break;
            default:
					if (RecordIndex <= 0x7fff)
					{  // *** transfer user specific indizes ***
						Status = PnUsr_cbf_rec_write (DevHndl, Api, ArNum, SessionKey, SequenceNum, pAddr,
													  RecordIndex, pBufLen, pBuffer, pPnioState);
					}
					else // *** not supported index ***
					{// *** if an error occured, it must be specify  according to IEC 61158-6
						pPnioState->ErrCode   = 0xdf;   // IODWriteRes with ErrorDecode = PNIORW
						pPnioState->ErrDecode = 0x80;   // PNIORW
						pPnioState->ErrCode1  = 0xb0;   // example: Error Class 11 = access, ErrorNr 0 = "invalid index"
						Status = PNIO_NOT_OK;
					}
					break;
        }
        return (Status);
    }


#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
