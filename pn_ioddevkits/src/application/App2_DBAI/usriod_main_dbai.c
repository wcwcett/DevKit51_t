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
/*  F i l e               &F: usriod_main_dbai.c                        :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Main application program for a RT and IRT  Profinet IO Device            */
/*  uses the Direct Buffer Access Interface (DBAI) for IO data access        */
/*                                                                           */
/*  Features:                                                                */
/*    - direct buffered data access via DBAI                                 */
/*    - usable for RT and IRT in the same way                                */
/*    - same IO access for RT  and IRT,                                      */
/*    - Example application only supports 1 AR here (no shared device)       */
/*      (but feature can be implemented by user into his application)        */
/*                                                                           */
/*    - includes simple terminal application, it performs                    */
/*        * messages via printf                                              */
/*        * starts executing typical API-commands via terminal-keyboard      */
/*        * connect terminal (e.g. Hyperterminal) via RS232 interface        */
/*                                                                           */
/*  To use this application example, set #define EXAMPL_DEV_CONFIG_VERSION 2 */
/*  in file \application\usriod_cfg.h                                        */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*  note: this application is a simple demo application. To keep it simple,  */
/*        the return values of the IodApixx functions are often not analyzed.*/
/*        But it is strongly recommended, to do this in a real application.  */
/*                                                                           */
/*  THIS MODULE HAS TO BE MODIFIED BY THE PNIO USER, if DBA-interface is used*/
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/


// *--------------------------------------------------------------------------
// *  FUNCTIONAL DESCRIPTION
// *
// *  This example consists of 2 tasks and one callback-function:
// *    -  a main task (mainAppl() function) , that starts the PNIO stack, plugs  and
// *       all modules and submodules and waits on console input afterwards.
// *       Console input commands can be used for example plug/pull module/submodule,
// *       send alarm, activate/passivate device,...
// *
// *    -  a cyclic task Task_CycleIO(), that performs cyclically an IO data access.
// *
// *
// *   example settings:
// *   =================
// *         DAP:           Slot 0, Subslot 1,              has own IM0 data
// *         Network-IF:    Slot 0, Subslot 0x8000,         no own IM0 data
// *         Network Ports: Slot 0, Subslot 0x8001, 0x8002  no own IM0 data
// *         Input Module:  Slot 1, Subslot 1, 64 Byte input,  ModID = 0x30, no own IM0 data
// *         Output Module: Slot 2, Subslot 1, 64 Byte output, ModID = 0x31, no own IM0 data
// *
// *--------------------------------------------------------------------------
// *   Differences, using the DBA interface
// *
// *    1.) unlike the example 1 (RT, IRT) in this example the IO access
// *        via PNIO_initiate_data_read() or PNIO_initiate_data_write is NOT USED.
// *        Instead of that new functions PNIO_dbai_enter(), PNIO_dbai_exit(),
// *        PNIO_dbai_buffer_lock() and PNIO_dbai_buffer_unlock() are used.
// *
// *    2.) Extracting the submodule specific IO data, IOPS and IOCS from the
// *        IOCR data block must be done by user application. The additional
// *        information, to do that, is handed out from PNIO stack to application
// *        in function PNIO_cbf_ar_info_indication and stored in a data structure
// *        USRIOD_DBAI_IOCR UsrIocr.
// *
// *--------------------------------------------------------------------------
// *   How to use DBA interface:
// *
// *   1. additional information, needed to lock a buffer with PNIO-dbai_buffer_lock() and
// *      to extract the subslot specific IO data, IOPS, IOCS from that buffer, are
// *      provided to application in function PNIO_cbf_ar_info_indication (). The
// *      application has to store this information for further IO data access.
// *      You find the appropriate example code in iodapie_event_dbai.c
// *
// *   2. To make a IO data exchange between stack and application, do the following steps
// *      (see also example code in functions UsrIod_initiate_data_read () and UsrIod_initiate_data_write ()).
// *
// *       step 1:  call PNIO_dbai_enter(), to prevent stack from calling PNIO_cbf_xxx()
// *                and modifying internal data structures for this AR/IOCR.
// *
// *       step 2:  Check if IOCR is valid, if true call PNIO_dbai_buffer_lock(), to get a
// *                pointer to a IOCR buffer, containing IO data, IOPS, IOCS. IOCR
// *                must be either a input IOCR or output IOCR.
// *
// *       step 3:  If IOCR is input IOCR, copy input IO data and IOPS of all contained
// *                input submodules into IOCR-buffer.
// *                Then copy the local IOCS of all contained output modules into IOCR-buffer.
// *
// *                If IOCR is output IOCR, extract output data and remote IOPS
// *                of all contained output submodules.
// *                Then extract also the remote IOCS from the contained input modules.
// *
// *       step 4:  call PNIO_dbai_buffer_unlock(), to disable IOCR buffer locking.
// *
// *       step 5:  call PNIO_dbai_exit(), to disable PNIO_cbf_xxx - locking.
// *
// *
// *
// *      To keep it simple, this example assumes,
// *        - that only one AR at the same time is available.
// *        - that an ownership is always accepted (only one AR)
// *
// *
// *      This example can be expanded by the user to handle more AR's.
// *
// *      Consider also the following requirements:
// *       - if a wrong submodule is plugged, IOPS of inputs or IOCS of outputs
// *         have to be set to BAD
// *       - before return from PNIO_cbf_param_end_ind() all correct input submodules have to have
// *         IOPS = GOOD. Return value is RUN. No entry in the ModDiffBlock is done from stack.
// *       - For not running input submodules PNIO_cbf_param_end_ind() returns with state STOP or APPL_RDY_FOLLOWS.
// *         IOPS state is BAD and they are included in the ModDiffblock of the applReady telegram.
// *       - all wrong submodules have IOPS = BAD. They are marked as Wrong in the ownership-indication
// *         No param_end indication comes for wrong submodules. A ModDiffblock entry is automaticall
// *         done by the stack in the connection response and application ready request.
// *       - if a wrong DAP is plugged, all submodules of the device are marked as "superordinated locked"
// *         by the stack automatically.  So in this case all submodules have to have IO-state BAD.
// *--------------------------------------------------------------------------


#include "compiler.h"
#include "usriod_cfg.h"
#include "usrapp_cfg.h"   // example application spec. configuration settings

#if (EXAMPL_DEV_CONFIG_VERSION == 2)
    #include "version_dk.h"
    #include "pniousrd.h"
    #include "bspadapt.h"
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
    #include "perform_measure.h"
    #include "usriod_dbai.h"
    #include "evaluate_edc.h"

    #if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
        #include "hama_com.h"
        #include <cyg/hal/ertec200p_reg.h>
        #include <cyg/kernel/kapi.h>
        #include <cyg/hal/hama_timer.h>
    #endif
    /*===========================================================================*/
    // *-------------plausibility checks -----*
    #if (IOD_CFG_NUMOF_IO_AR > 1)
        #pragma message ("###  only one AR is managed in the DBAI example application. ###")
        #pragma message ("###  multi AR support is notsupported only in the SI         ###")
        #pragma message ("###  example. So either set IOD_CFG_NUMOF_IO_AR to 1         ###")
        #pragma message ("###  or implement multi AR support into your                 ###")
        #pragma message ("###  DBAI based application.                                 ###")

        #error ("ERROR  number of IO-AR in DBAI example must be 1")
    #endif

    // *------------ defines -----------------*

    // *------------ external data  ----------*
    extern PNIO_UINT32      PnioLogDest;            // destination for logging messages


    // *------------ external functions  ----------*
    extern void PrintAllUsedIoData (void);              // print io data on the terminal
    extern void ChangeAllInputData (PNIO_UINT8 DiffVal);// change io data values
    extern PNIO_UINT32 TcpReceiveAndFlashFirmware (void);
    extern PNIO_UINT32 UsrDbai_initiate_data_read( void);
    extern PNIO_UINT32 UsrDbai_initiate_data_write (void);

    // *------------ internal functions  ----------*

    // *------------ public data  ----------*


    // *------------ local data  ----------*
    static PNUSR_DEVICE_INSTANCE PnUsrDev;
#ifdef BOARD_TYPE_STEP_3
    static PNIO_UINT32           TskId_EDC;
#endif

    #if EXAMPLE_IPAR_SERVER
        /* upload alarm for iPar server*/
        static  PNIO_UINT8 UploadAlarmData[] =
                            {   0x0F,0x02,                          // blocktype
                                0x00,0x14,                          // length without blocktype, length
                                0x01,0x00,                          // version
                                0x00,0x00,                          // padding
                                0x01,0x00,0x00,0x01,                // iPar_Req_Header (01 = upload)
                                0x00,0x00,0x00,0x04,                // Max_Segm_Size
                                0x00,0x00,0x00,EXAMPL_IPAR_REC_IND, // Transfer_Index (record index)
                                0x00,0x00,0x00,EXAMPL_IPAR_UPL_LEN  // number of bytes to store
                            } ;

        /* retrieval alarm for iPar server */
        static  PNIO_UINT8 RetrievalAlarmData[] =
                            {   0x0F,0x02,                          // blocktype
                                0x00,0x14,                          // length without blocktype, length
                                0x01,0x00,                          // version
                                0x00,0x00,                          // padding
                                0x01,0x00,0x00,0x03,                // iPar_Req_Header  (03 = retrieval)
                                0x00,0x00,0x00,0x04,                // number of bytes to store in iPar server
                                0x00,0x00,0x00,EXAMPL_IPAR_REC_IND, // Transfer_Index  (record index = 50h in this example)
                                0x00,0x00,0x00,EXAMPL_IPAR_RTV_LEN  // max. number of bytes to restore, 0= take all from upload
                            };
    #endif

    // *-------------------------------------------------------------------------------------------
    // * list of IO subslot (including also PDEV subslots), that shall be plugged during startup.
    // *
    // * list order:  DAP first, then PDEV, then IO submodules:
    // *    DAP                              (mandatory)
    // *    PDEF-Interface                   (mandatory)
    // *    PDEV port 1 .... PDEV port n     (mandatory)
    // *    IO submodules                    (optional)
    // *
    // *
    // * Note, that I&M0 support for the DAP is mandatory, but optional for all other submoduls.
    // * In this case they have to respond to an IM0 read request with the IM0 data of the DAP, that is
    // * a proxy for the device related IM0 data here.
    // *
    // * IO subslots can optionally be plugged or pulled later.
    // * DAP and PDEV subslots can not be pulled.
    // *
    // *
    // * RESTRICTIONS  (see also example IoSubList[] below)
    // * ============
    // * 1. exact one submodule must be the proxy for the device (must be INTERFACE subslot 0x8000)
    // * 2. IM1...4 is supported
    // *
    // *
    // *-------------------------------------------------------------------------------------------
    static PNIO_SUB_LIST_ENTRY IoSubList []
    = {
		// Api  Slot	Subslot,    ModId,              SubId,                InLen,  OutLen, I&M0 support
		{  0,	0,      1,          MODULE_ID_DAP,  	SUBMOD_ID_DEFAULT,    0,      0,      PNIO_IM0_SUBMODULE + PNIO_IM0_DEVICE} ,
		{  0,   0,      0x8000,     MODULE_ID_DAP,  	SUBMOD_ID_PDEV_IF,    0,      0,      PNIO_IM0_SUBMODULE}, // PDEV interface
		{  0,   0,      0x8001,     MODULE_ID_DAP,  	SUBMOD_ID_PDEV_PORT,  0,      0,      PNIO_IM0_SUBMODULE},   // PDEV port1
		#if (IOD_CFG_PDEV_NUMOF_PORTS >= 2)
		{  0,   0,      0x8002,     MODULE_ID_DAP,  	SUBMOD_ID_PDEV_PORT,  0,      0,      PNIO_IM0_SUBMODULE},   // PDEV port2
		#endif
		#if (IOD_CFG_PDEV_NUMOF_PORTS >= 3)
		{  0,   0,      0x8003,     MODULE_ID_DAP,  	SUBMOD_ID_PDEV_PORT,  0,      0,      PNIO_IM0_SUBMODULE},   // PDEV port3
		#endif
		#if (IOD_CFG_PDEV_NUMOF_PORTS >= 4)
		{  0,   0,      0x8004,     MODULE_ID_DAP,  	SUBMOD_ID_PDEV_PORT,  0,      0,      PNIO_IM0_SUBMODULE},   // PDEV port4
		#endif
		{  0,   1,      1,          EXAMPL_MOD_ID_SLOT1,  SUBMOD_ID_DEFAULT, 64,      0,      PNIO_IM0_SUBMODULE}, // IO subslot
        {  0,   2,      1,          EXAMPL_MOD_ID_SLOT2,  SUBMOD_ID_DEFAULT,  0,     64,      PNIO_IM0_SUBMODULE}    // IO subslot
    };


    // * --------------------------------------------
    // * List of IM0 data
    // * ---------------------------
    // * Api, Slot, Subslot         in machine format
    // * Vendor ID, OrderID,..,...  in BIG ENDIAN FORMAT
    // * --------------------------------------------
    static PNIO_IM0_LIST_ENTRY Im0List []
        = { // * --------------------------------------------
            // * IM0 Data for the DAP
            // * (optional)
            // * --------------------------------------------
           {0,                          // PNIO_UINT32     Api;                 // api number
            0,                          // PNIO_UINT32     Slot;                // slot number (1..0x7fff)
            1,                          // PNIO_UINT32     Subslot;             // subslot number (1..0x7fff)
               {0x2a,                   // PNIO_UINT16     VendorId;            // VendorIDHigh, VendorIDLow
                IOD_CFG_DEV_ANNOTATION_ORDER_ID,// PNIO_UINT8      OrderId [20];        // Order_ID, visible, must be 20 bytes here (fill with blanks)
                IOD_CFG_IM0_SERIAL_NUMBER,      // PNIO_UINT8      SerNum  [16];        // IM_Serial_Number, visible string, must be 16 bytes here (fill with blanks)
                DEVKIT_HW_REVISION,     // PNIO_UINT16     HwRevision;          // IM_Hardware_Revision
               {DEVKIT_VERSION_PREFIX,  // PNIO_UINT8      SwRevPrefix;         // software revision prefix
                DEVKIT_VERSION_HH,      // PNIO_UINT8      SwRevFuncEnhanc;     // IM_SW_Revision_Funct_Enhancement
                DEVKIT_VERSION_H ,      // PNIO_UINT8      SwRevBugFix;         // IM_SW_Revision_Bug_Fix
                DEVKIT_VERSION_L},      // PNIO_UINT8      SwRevInternChange;   // IM_SW_Revision_Internal_Change
				DEVKIT_VERSION_LL,      // PNIO_UINT16     Revcnt;              // IM_Revision_Counter, notifies a hw modification
                0x00,                   // PNIO_UINT16     ProfId;              // IM_Profile_ID, see Profile Guideline I&M functions
                0x03,                   // PNIO_UINT16     ProfSpecTyp;         // IM_Profile_Spec_Type (e.g. 3="io module")
                0x01,                   // PNIO_UINT8      VersMaj;             // IM_Version major
                0x01,                   // PNIO_UINT8      VersMin;             // IM_Version minor
#if IOD_INCLUDE_IM5
                PNIO_SUPPORT_IM12345    // PNIO_UINT16     ImSupported;         // IM_Supported, IM1...5 supported (bit list, bit 1...bit5, here: 0 ==> IM1..5  supported)
#else
                PNIO_SUPPORT_IM1234     // PNIO_UINT16     ImSupported;         // IM_Supported, IM1...4 supported (bit list, bit 1...bit4, here: 0 ==> IM1..4  supported)
#endif
                }},
        // * --------------------------------------------
        // * IM0 Data for the slot 0 interface
        // * (optional)
        // * --------------------------------------------
           {0,                          // PNIO_UINT32     Api;                 // api number
            0,                          // PNIO_UINT32     Slot;                // slot number (1..0x7fff)
            0x8000,                     // PNIO_UINT32     Subslot;             // subslot number (1..0x7fff)
           	   {0x2a,                   // PNIO_UINT16     VendorId;            // VendorIDHigh, VendorIDLow
           	   IOD_CFG_DEV_ANNOTATION_ORDER_ID,// PNIO_UINT8      OrderId [20];        // Order_ID, visible, must be 20 bytes here (fill with blanks)
           	   IOD_CFG_IM0_SERIAL_NUMBER,      // PNIO_UINT8      SerNum  [16];        // IM_Serial_Number, visible string, must be 16 bytes here (fill with blanks)
               DEVKIT_HW_REVISION,     // PNIO_UINT16     HwRevision;          // IM_Hardware_Revision
               {DEVKIT_VERSION_PREFIX, // PNIO_UINT8      SwRevPrefix;         // software revision prefix
               DEVKIT_VERSION_HH,      // PNIO_UINT8      SwRevFuncEnhanc;     // IM_SW_Revision_Funct_Enhancement
               DEVKIT_VERSION_H ,      // PNIO_UINT8      SwRevBugFix;         // IM_SW_Revision_Bug_Fix
               DEVKIT_VERSION_L},      // PNIO_UINT8      SwRevInternChange;   // IM_SW_Revision_Internal_Change
               DEVKIT_VERSION_LL,      // PNIO_UINT16     Revcnt;              // IM_Revision_Counter, notifies a hw modification
               0x00,                   // PNIO_UINT16     ProfId;              // IM_Profile_ID, see Profile Guideline I&M functions
               0x05,                   // PNIO_UINT16     ProfSpecTyp;         // IM_Profile_Spec_Type (e.g. 5="interface module")
               0x01,                   // PNIO_UINT8      VersMaj;             // IM_Version major
               0x01,                   // PNIO_UINT8      VersMin;             // IM_Version minor (must be 1, do not change !!)
#if IOD_INCLUDE_IM5
               PNIO_SUPPORT_IM12345    // PNIO_UINT16     ImSupported;         // IM_Supported, IM1...5 supported (bit list, bit 1...bit5, here: 0 ==> IM1..5  supported)
#else
               PNIO_SUPPORT_IM1234     // PNIO_UINT16     ImSupported;         // IM_Supported, IM1...4 supported (bit list, bit 1...bit4, here: 0 ==> IM1..4  supported)
#endif
               }},
		// * --------------------------------------------
		// * IM0 Data for the slot 0 port 1
		// * (optional)
		// * --------------------------------------------
           {0,                          // PNIO_UINT32     Api;                 // api number
            0,                          // PNIO_UINT32     Slot;                // slot number (1..0x7fff)
            0x8001,                     // PNIO_UINT32     Subslot;             // subslot number (1..0x7fff)
                {0x2a,                  // PNIO_UINT16     VendorId;            // VendorIDHigh, VendorIDLow
                IOD_CFG_DEV_ANNOTATION_ORDER_ID,// PNIO_UINT8      OrderId [20];        // Order_ID, visible, must be 20 bytes here (fill with blanks)
                IOD_CFG_IM0_SERIAL_NUMBER,      // PNIO_UINT8      SerNum  [16];        // IM_Serial_Number, visible string, must be 16 bytes here (fill with blanks)
                DEVKIT_HW_REVISION,     // PNIO_UINT16     HwRevision;          // IM_Hardware_Revision
                {DEVKIT_VERSION_PREFIX, // PNIO_UINT8      SwRevPrefix;         // software revision prefix
                DEVKIT_VERSION_HH,      // PNIO_UINT8      SwRevFuncEnhanc;     // IM_SW_Revision_Funct_Enhancement
                DEVKIT_VERSION_H ,      // PNIO_UINT8      SwRevBugFix;         // IM_SW_Revision_Bug_Fix
                DEVKIT_VERSION_L},      // PNIO_UINT8      SwRevInternChange;   // IM_SW_Revision_Internal_Change
                DEVKIT_VERSION_LL,      // PNIO_UINT16     Revcnt;              // IM_Revision_Counter, notifies a hw modification
                0x00,                   // PNIO_UINT16     ProfId;              // IM_Profile_ID, see Profile Guideline I&M functions
                0x05,                   // PNIO_UINT16     ProfSpecTyp;         // IM_Profile_Spec_Type (e.g. 5="interface module")
                0x01,                   // PNIO_UINT8      VersMaj;             // IM_Version major
                0x01,                   // PNIO_UINT8      VersMin;             // IM_Version minor (must be 1, do not change !!)
#if IOD_INCLUDE_IM5
			   PNIO_SUPPORT_IM12345    // PNIO_UINT16     ImSupported;         // IM_Supported, IM1...5 supported (bit list, bit 1...bit5, here: 0 ==> IM1..5  supported)
#else
			   PNIO_SUPPORT_IM1234     // PNIO_UINT16     ImSupported;         // IM_Supported, IM1...4 supported (bit list, bit 1...bit4, here: 0 ==> IM1..4  supported)
#endif
			   }},
#if (IOD_CFG_PDEV_NUMOF_PORTS >= 2)
		// * --------------------------------------------
		// * IM0 Data for the slot 0 port 2
		// * (optional)
		// * --------------------------------------------
           {0,                          // PNIO_UINT32     Api;                 // api number
            0,                          // PNIO_UINT32     Slot;                // slot number (1..0x7fff)
            0x8002,                     // PNIO_UINT32     Subslot;             // subslot number (1..0x7fff)
                {0x2a,                  // PNIO_UINT16     VendorId;            // VendorIDHigh, VendorIDLow
                IOD_CFG_DEV_ANNOTATION_ORDER_ID,// PNIO_UINT8      OrderId [20];        // Order_ID, visible, must be 20 bytes here (fill with blanks)
                IOD_CFG_IM0_SERIAL_NUMBER,      // PNIO_UINT8      SerNum  [16];        // IM_Serial_Number, visible string, must be 16 bytes here (fill with blanks)
                DEVKIT_HW_REVISION,     // PNIO_UINT16     HwRevision;          // IM_Hardware_Revision
                {DEVKIT_VERSION_PREFIX, // PNIO_UINT8      SwRevPrefix;         // software revision prefix
                DEVKIT_VERSION_HH,      // PNIO_UINT8      SwRevFuncEnhanc;     // IM_SW_Revision_Funct_Enhancement
                DEVKIT_VERSION_H ,      // PNIO_UINT8      SwRevBugFix;         // IM_SW_Revision_Bug_Fix
                DEVKIT_VERSION_L},      // PNIO_UINT8      SwRevInternChange;   // IM_SW_Revision_Internal_Change
                DEVKIT_VERSION_LL,      // PNIO_UINT16     Revcnt;              // IM_Revision_Counter, notifies a hw modification
                0x00,                   // PNIO_UINT16     ProfId;              // IM_Profile_ID, see Profile Guideline I&M functions
                0x05,                   // PNIO_UINT16     ProfSpecTyp;         // IM_Profile_Spec_Type (e.g. 5="interface module")
                0x01,                   // PNIO_UINT8      VersMaj;             // IM_Version major
                0x01,                   // PNIO_UINT8      VersMin;             // IM_Version minor (must be 1, do not change !!)
#if IOD_INCLUDE_IM5
               PNIO_SUPPORT_IM12345    // PNIO_UINT16     ImSupported;         // IM_Supported, IM1...5 supported (bit list, bit 1...bit5, here: 0 ==> IM1..5  supported)
#else
               PNIO_SUPPORT_IM1234     // PNIO_UINT16     ImSupported;         // IM_Supported, IM1...4 supported (bit list, bit 1...bit4, here: 0 ==> IM1..4  supported)
#endif
               }},
#endif
            // * --------------------------------------------
            // * IM0 Data for the slot 1 (64 byte input)
            // * (mandatory, because proxy for device)
            // * --------------------------------------------
           {0,                          // PNIO_UINT32     Api;                 // api number
            1,                          // PNIO_UINT32     Slot;                // slot number (1..0x7fff)
            1,                          // PNIO_UINT32     Subslot;             // subslot number (1..0x7fff)
               {0x2a,                   // PNIO_UINT16     VendorId;            // VendorIDHigh, VendorIDLow
                IOD_CFG_DEV_ANNOTATION_ORDER_ID,// PNIO_UINT8      OrderId [20];        // Order_ID, visible, must be 20 bytes here (fill with blanks)
                IOD_CFG_IM0_SERIAL_NUMBER,      // PNIO_UINT8      SerNum  [16];        // IM_Serial_Number, visible string, must be 16 bytes here (fill with blanks)
                DEVKIT_HW_REVISION,     // PNIO_UINT16     HwRevision;          // IM_Hardware_Revision
               {'V',                    // PNIO_UINT8      SwRevPrefix;         // software revision prefix
                DEVKIT_VERSION_HH,      // PNIO_UINT8      SwRevFuncEnhanc;     // IM_SW_Revision_Funct_Enhancement
                DEVKIT_VERSION_H,       // PNIO_UINT8      SwRevBugFix;         // IM_SW_Revision_Bug_Fix
                DEVKIT_VERSION_L},      // PNIO_UINT8      SwRevInternChange;   // IM_SW_Revision_Internal_Change
                DEVKIT_VERSION_LL,      // PNIO_UINT16     Revcnt;              // IM_Revision_Counter, notifies a hw modification
                0x00,                   // PNIO_UINT16     ProfId;              // IM_Profile_ID, see Profile Guideline I&M functions (default: 0)
                0x03,                   // PNIO_UINT16     ProfSpecTyp;         // IM_Profile_Spec_Type (e.g. 3="io module")
                0x01,                   // PNIO_UINT8      VersMaj;             // IM_Version major (must be 1, do not change !!)
                0x01,                   // PNIO_UINT8      VersMin;             // IM_Version minor (must be 1, do not change !!)
                PNIO_SUPPORT_IM1234}},  // PNIO_UINT16     ImSupported;         // IM_Supported, IM1...4 supported (bit list, bit 1...bit4, here: 0 ==> IM1..15 not supported)
           // * --------------------------------------------
           // * IM0 Data for the slot 2 (64 byte output)
           // * (mandatory, because proxy for device)
           // * --------------------------------------------
          {0,                          // PNIO_UINT32     Api;                 // api number
           2,                          // PNIO_UINT32     Slot;                // slot number (1..0x7fff)
           1,                          // PNIO_UINT32     Subslot;             // subslot number (1..0x7fff)
              {0x2a,                   // PNIO_UINT16     VendorId;            // VendorIDHigh, VendorIDLow
               IOD_CFG_DEV_ANNOTATION_ORDER_ID,// PNIO_UINT8      OrderId [20];        // Order_ID, visible, must be 20 bytes here (fill with blanks)
               IOD_CFG_IM0_SERIAL_NUMBER,      // PNIO_UINT8      SerNum  [16];        // IM_Serial_Number, visible string, must be 16 bytes here (fill with blanks)
               DEVKIT_HW_REVISION,     // PNIO_UINT16     HwRevision;          // IM_Hardware_Revision
              {'V',                    // PNIO_UINT8      SwRevPrefix;         // software revision prefix
               DEVKIT_VERSION_HH,      // PNIO_UINT8      SwRevFuncEnhanc;     // IM_SW_Revision_Funct_Enhancement
               DEVKIT_VERSION_H,       // PNIO_UINT8      SwRevBugFix;         // IM_SW_Revision_Bug_Fix
               DEVKIT_VERSION_L},      // PNIO_UINT8      SwRevInternChange;   // IM_SW_Revision_Internal_Change
               DEVKIT_VERSION_LL,      // PNIO_UINT16     Revcnt;              // IM_Revision_Counter, notifies a hw modification
               0x00,                   // PNIO_UINT16     ProfId;              // IM_Profile_ID, see Profile Guideline I&M functions (default: 0)
               0x03,                   // PNIO_UINT16     ProfSpecTyp;         // IM_Profile_Spec_Type (e.g. 3="io module")
               0x01,                   // PNIO_UINT8      VersMaj;             // IM_Version major (must be 1, do not change !!)
               0x01,                   // PNIO_UINT8      VersMin;             // IM_Version minor (must be 1, do not change !!)
               PNIO_SUPPORT_IM1234}}   // PNIO_UINT16     ImSupported;         // IM_Supported, IM1...4 supported (bit list, bit 1...bit4, here: 0 ==> IM1..15 not supported)

    };


    // *----------------------------------------------------------------*
    // *
    // *  PrintHelp (void)
    // *
    // *----------------------------------------------------------------*
    static void PrintHelp (void)
    {   // *---------------------------------------
        // *      help
        // *---------------------------------------
        PNIO_ConsolePrintf ("\n\nCOMMAND LIST    ");
        PrintDevkitVersion ();

        PNIO_ConsolePrintf ("\nCONTROL:\n");
        PNIO_ConsolePrintf ("'a'      send process alarm on slot1, subslot1\n");
        PNIO_ConsolePrintf ("'h'      send status alarm on slot1, subslot1\n");
        PNIO_ConsolePrintf ("'B / b'  send channel diag-alarm 'line break' appear/disapp. on slot1, subsl.1\n");
        PNIO_ConsolePrintf ("'C / c'  send generic diag-alarm  appear/disapp. on slot0, subsl.1\n");
        PNIO_ConsolePrintf ("'E / e'  send extended channel diag-alarm appear/disapp. on slot2, subsl.1\n");
        PNIO_ConsolePrintf ("'Q / q'  increment/decrement input data values\n");
        PNIO_ConsolePrintf ("'f'      download firmware via TCP and store into flash\n");
        #if EXAMPLE_IPAR_SERVER
            PNIO_ConsolePrintf ("'G'      send upload alarm (backup) to ipar-server \n");
            PNIO_ConsolePrintf ("'g'      send retrieval alarm (restore) to ipar-server  \n");
        #endif
        PNIO_ConsolePrintf ("'H'      print plugged submodules \n");
        PNIO_ConsolePrintf ("'j'      print cyclic io data \n");
		#if(PNIO_TRACE != PNIO_TRACE_NONE)
        	PNIO_ConsolePrintf ("'K'      set RS232 output trace level \n");
        	PNIO_ConsolePrintf ("'k'      set MEM output trace level \n");
		#endif
        PNIO_ConsolePrintf ("'L'      set LED (GPIO 0..31, (-1):all) on \n");
        PNIO_ConsolePrintf ("'l'      set LED (GPIO 0..31, (-1):all) off \n");
        PNIO_ConsolePrintf ("'n'      set device name and store in flash \n");
        PNIO_ConsolePrintf ("'N'      set MAC address and store in flash \n");
        PNIO_ConsolePrintf ("'o'      set IP address and store into flash \n");
        PNIO_ConsolePrintf ("'P'      enable user printf output on serial console\n");
        PNIO_ConsolePrintf ("'p'      disable user printf output\n");
        PNIO_ConsolePrintf ("'S'      plug a submodule\n");
        PNIO_ConsolePrintf ("'s'      pull a submodule\n");
		#if(PNIO_TRACE != PNIO_TRACE_NONE)
        	PNIO_ConsolePrintf ("'T'      set trace level for a package \n");
        	PNIO_ConsolePrintf ("'t'      set trace level for a single subsystem \n");
		#endif
		#ifdef INCLUDE_PERFORMANCE_MEASURE
        	PNIO_ConsolePrintf ("'z'      measure system performance\n");
		#endif
        PNIO_ConsolePrintf ("'d'      perform stop of PN device \n");
        PNIO_ConsolePrintf ("'D'      perform start of PN device \n");
        PNIO_ConsolePrintf ("'0'      Perform System Reboot\n");
        PNIO_ConsolePrintf ("\n");
    }


 // *----------------------------------------------------------------*
 // *
 // *  MainAppl(void)
 // *
 // *----------------------------------------------------------------*
 // *
 // *  main application function
 // *   - starts the pnio stack
 // *   - starts user interface task
 // *   - handles user inputs and starts the selected test functions
 // *
 // *  Input:    argc            not used yet
 // *            argv            not used yet
 // *  Output:   ----
 // *
 // *----------------------------------------------------------------*
    void MainAppl (void)

    {
        PNIO_UINT32 Status;
        PNIO_UINT32 exitAppl = PNIO_FALSE;
        PNIO_BOOL isDeviceRunning = PNIO_FALSE;

        PNIO_UINT8     ProcessAlarmData[] = {"Process Alarm Slot 1, Subslot 1 "};           // dummy data for process alarm
        PNIO_UINT8     StatusAlarmData[] = {'a', 'b', 'c', 'd'};           // dummy data for status alarm
        PNIO_UINT8     DiagAlarmData[]    = {"Diagnostic Alarm Slot 2, Subslot 1 "};    // dummy data for diagnostic alarm

        #if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
            //init USER LEDs (GPIO[0-7,16-23] = LED 20 to 35)
            REG32(U_GPIO__GPIO_PORT_MODE_0_L) &= ~( 0x0000FFFF );     //function == GPIO for  0- 7
            REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= ~( 0x0000FFFF );     //function == GPIO for 16-23
            REG32(U_GPIO__GPIO_IOCTRL_0) &= ~( 0x00FF00FF );          //output
            #ifndef BOARD_TYPE_STEP_3
                REG32(U_GPIO__GPIO_OUT_CLEAR_0) |= 0x00FF00FF;        //clear outputs == LEDs off
            #else
                REG32(U_GPIO__GPIO_OUT_CLEAR_0) |= 0xFFFFFFFF;
            #endif
            //init special purpose LEDs (GPIO[25-31] = LED 11 - 17)
            REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= ~( 0xfffc0000 );     //function == GPIO for 25-31
            REG32(U_GPIO__GPIO_IOCTRL_0) &= ~( 0xFE000000 );          //output
            REG32(U_GPIO__GPIO_OUT_CLEAR_0) |= 0xFE000000;            //clear outputs == LEDs off

            if( MODE_SPI_0x03 == ( REG32( U_SCRB__BOOT_REG ) &0xf ) )
    		{

            	REG32(U_GPIO__GPIO_IOCTRL_0) &= ( ~( 1 << 31 ) );
            	REG32(U_GPIO__GPIO_IOCTRL_0) &= ( ~( 1 << 24 ) );
            	REG32(U_GPIO__GPIO_IOCTRL_0) &= ( ~( 1 << 26 ) );
            	REG32(U_GPIO__GPIO_IOCTRL_0) |= ( 1 << 27 );
            	REG32(U_GPIO__GPIO_PORT_MODE_0_H) |= ( 1 << 16 );
            	REG32(U_GPIO__GPIO_PORT_MODE_0_H) |= ( 1 << 20 );
            	REG32(U_GPIO__GPIO_PORT_MODE_0_H) |= ( 1 << 22 );
            	REG32(U_GPIO__GPIO_OUT_SET_0) = ( 1 << 31 );

            //in case of SPI boot, init GPIO 8, 10, 11 for LEDS
            REG32(U_GPIO__GPIO_PORT_MODE_0_L) &= ~( 0x00F30000 );         //function == GPIO for 25-31
            #ifndef BOARD_TYPE_STEP_3
                REG32(U_GPIO__GPIO_IOCTRL_0) &= ~( 0x00000D00 );          //output
                REG32(U_GPIO__GPIO_OUT_CLEAR_0) |= 0x00000D00;            //clear outputs == LEDs off   
            #else
                REG32(U_GPIO__GPIO_IOCTRL_0) &= ~( 0x00000F00 );          //output
                REG32(U_GPIO__GPIO_OUT_CLEAR_0) |= 0x00000F00;            //clear outputs == LEDs off
            #endif
            }
        #endif


        // *-----------------------------------------------------
        // *     set startup parameter for the device
        // *     Note: in this simple example we suppose,
        // *     the DAP has no MRP capability.
        // *     If MPR shall be supported, the PN controller must
        // *     be capabable to send an MRP configuration record,
        // *     even if MRP is not activated.
        // *     More info to this topic see example App1_Standard,
        // *     file usriod_main.c
        // *-----------------------------------------------------
        PnUsrDev.VendorId            = IOD_CFG_VENDOR_ID;               // Vendor ID, requested from PROFIBUS/PROFINET organization (PI)
        PnUsrDev.DeviceId            = IOD_CFG_DEVICE_ID;               // Device ID, must be unique inside one Vendor ID
        PnUsrDev.MaxNumOfSubslots    = IOD_CFG_NUMOF_SUBSLOTS;          // maximum number of subslots per slot, managable by PN Stack
        PnUsrDev.pDevType            = (PNIO_INT8*)IOD_CFG_DEVICE_TYPE; // see also GSDML file, product family

        // **** startup the PN stack ****
        // *-----------------------------------------------------------
        // *     setup device stack and plug all io modules
        // *     (number of plugged modules <= IOD_CFG_NUMOF_SUBSLOTS)
        // *-----------------------------------------------------------
        Status = PnUsr_DeviceSetup (&PnUsrDev,                                  // device specific configuration
                                    &IoSubList[0],                              // list of plugged submodules (real configuration), including PDEV
                                    sizeof (IoSubList) / sizeof (IoSubList[0]), // number of entries in IoSubList
                                    &Im0List[0],                                // list of IM0 data sets
                                    sizeof (Im0List) / sizeof (Im0List[0]));     // number of entries in Im0List

	    // *----------------------------------------------------------
	    // * create and start EDC evaluation task
	    // *----------------------------------------------------------
        #ifdef BOARD_TYPE_STEP_3
           Status = OsCreateThread((void(*)(void))evaluate_edc_errors, 0, (PNIO_UINT8*)"Task_EDC", TASK_EDC_POLL, OS_TASK_DEFAULT_STACKSIZE, &TskId_EDC);
           Status = OsStartThread(TskId_EDC);
        #endif

        // *----------------------------------------------------------
        // * print startup result message
        // *----------------------------------------------------------
        if (Status == PNIO_OK)
        {
            PNIO_ConsolePrintf ("SYSTEM STARTUP FINISHED, OK\n");
            isDeviceRunning = PNIO_TRUE;
        }
        else
            PNIO_ConsolePrintf ("##ERROR AT SYSTEM STARTUP\n");

        // *----------------------------------------------------------
        // *endless loop: wait on key pressed
        // *----------------------------------------------------------
        while (exitAppl == PNIO_FALSE)
        {
            // *-----------------------------------
            // * wait on key pressed by the user
            // *-----------------------------------
            PNIO_INT32 PressedKey = OsGetChar();

            switch (PressedKey)
            {

                PNIO_DEV_ADDR  Addr;                                // location (module/submodule)

                // ***-------------------------------------------------------------------***
                // ***       A L A R M S
                // ***-------------------------------------------------------------------***
                case 'a':
                        // *---------------------------------------
                        // *      send process alarm
                        // *      Note: only one process alarm at a time is possible
                        // *      Wait before sending a new alarm,
                        // *      until the previous one has been confirmed.
                            // *---------------------------------------
                        Addr.Geo.Slot       = EXAMPL_1_SLOTNUM;             // slot number
                        Addr.Geo.Subslot    = EXAMPL_1_SUBSLOTNUM;          // subslot number
                        Status = PNIO_process_alarm_send
                                            (PNIO_SINGLE_DEVICE_HNDL,
                                             PNIO_DEFAULT_API,
                                             &Addr,                         // location (slot, subslot)
                                             &ProcessAlarmData[0],          // alarm data
                                             sizeof (ProcessAlarmData),     // alarm data length
                                             EXAMPL_1_USER_STRUCT_IDENT,    // 0...0x7fff: user struct. is manufac. specific
                                             EXAMPL_1_DIAG_TAG);            // user defined handle

                        if (Status != PNIO_OK)
                            PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
                        break;

			    case 'h':
					    // *---------------------------------------
					    // *      send status alarm
                        // *      Note: only one status alarm at a time is possible
                        // *      Wait before sending a new alarm,
                        // *      until the previous one has been confirmed.
					    // *---------------------------------------
					    Addr.Geo.Slot		= EXAMPL_1_SLOTNUM;	            // slot number
					    Addr.Geo.Subslot	= EXAMPL_1_SUBSLOTNUM;			// subslot number
					    Status = PNIO_status_alarm_send
										    (PNIO_SINGLE_DEVICE_HNDL,
                                             PNIO_DEFAULT_API,
                                             &Addr,						    // location (slot, subslot)
										     &StatusAlarmData[0],		    // alarm data
										     sizeof (StatusAlarmData),      // alarm data length
											 EXAMPL_1_USER_STRUCT_IDENT,	// 0...0x7fff: user struct. is manufac. specific
										     EXAMPL_1_DIAG_TAG);	        // user defined handle

					    if (Status != PNIO_OK)
						    PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
						
				        break;

                case 'B':
					    // *----------------------------------------------
                        // *      provide standard channel diagnostic
                        // *      and send alarm appears, "line break"
                        // *      Note: only one diagnosis alarm at a time is possible
                        // *      Wait before sending a new alarm,
                        // *      until the previous one has been confirmed.
                        // *      (see also PNO document "diagnosis guideline" (on devkit CD)
					    // *-----------------------------------------------
                        Status = UsrChanDiag
                                    (DIAG_CHANPROP_SPEC_ERR_APP,          // new alarm appears
                                     EXAMPL_2_SLOTNUM,                    // slot number
                                     EXAMPL_2_SUBSLOTNUM,                 // subslot number
                                     EXAMPL_2_CHANNELNUM,                 // channel number
                                     EXAMPL_2_CHAN_ERR_TYPE,              // error number (see IEC61158)
                                     EXAMPL_2_IO_DIRECTION,               // channel is a input module
                                     EXAMPL_2_IO_DATA_TYPE,               // channel data type is BYTE
									 EXAMPL_2_DIAG_TAG
                                    );
                        if (Status != PNIO_OK)
                            PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());

                        break;

                case 'b':
                        // *-----------------------------------------------
                        // *      provide standard channel diagnostic
                        // *      and send alarm disappears, "line break"
                        // *      (no more errors pending)
                        // *      Note: only one diagnosis alarm at a time is possible
                        // *      Wait before sending a new alarm,
                        // *      until the previous one has been confirmed.
                        // *      (see also PNO document "diagnosis guideline" (on devkit CD)
                        // *-----------------------------------------------
                        Status = UsrChanDiag
                                    (DIAG_CHANPROP_SPEC_ERR_DISAPP,   // alarm disappears
                                     EXAMPL_2_SLOTNUM,                // slot number
                                     EXAMPL_2_SUBSLOTNUM,             // subslot number
                                     EXAMPL_2_CHANNELNUM,             // channel number
                                     EXAMPL_2_CHAN_ERR_TYPE,          // error number (see IEC61158)
                                     EXAMPL_2_IO_DIRECTION,           // channel is a input module
                                     EXAMPL_2_IO_DATA_TYPE,           // channel data type is BYTE
									 EXAMPL_2_DIAG_TAG
                                    );
                        if (Status != PNIO_OK)
                            PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
                        break;

                case 'C':
					    // *-----------------------------------------------
					    // *      send generic diagnostic
					    // *      alarm appears
                        // *      Note: only one diagnosis alarm at a time is possible
                        // *      Wait before sending a new alarm,
                        // *      until the previous one has been confirmed.
                        // *-----------------------------------------------
                        Status = UsrGenericDiag
                                            (DIAG_CHANPROP_SPEC_ERR_APP,    // alarm disappears
								             EXAMPL_4_SLOTNUM,              // slot number
								             EXAMPL_4_SUBSLOTNUM,           // subslot number
								             EXAMPL_4_CHANNELNUM,           // channel number
										     &DiagAlarmData[0],		    	// alarm data
										     sizeof (DiagAlarmData),	    // alarm data length
											 EXAMPL_4_DIAG_TAG,             // user defined tag
                                             EXAMPL_4_USER_STRUCT_IDENT,    // UserStructureIdentifier, manufacturer specific, 0...0x7fff, see IEC 61158
								             EXAMPL_4_IO_DIRECTION,	        // input/output/input-output
											 EXAMPL_4_IO_DATA_TYPE);	    // channel data type

                        if (Status != PNIO_OK)
						    PNIO_printf ("Error %x occured\n", PNIO_get_last_error ());

				        break;

			    case 'c':
					    // *-----------------------------------------------
					    // *      send generic diagnostic
					    // *      alarm disappears (no more errors
                        // *      pending)
                        // *      Note: only one diagnosis alarm at a time is possible
                        // *      Wait before sending a new alarm,
                        // *      until the previous one has been confirmed.
					    // *-----------------------------------------------
                        Status = UsrGenericDiag
                                            (DIAG_CHANPROP_SPEC_ERR_DISAPP, // alarm disappears
								             EXAMPL_4_SLOTNUM,              // slot number
								             EXAMPL_4_SUBSLOTNUM,           // subslot number
								             EXAMPL_4_CHANNELNUM,           // channel number
										     NULL,          		    	// no alarm data, don't care
										     0,                     	    // alarm data size = 0, don't care
											 EXAMPL_4_DIAG_TAG,             // user defined tag
                                             EXAMPL_4_USER_STRUCT_IDENT,    // UserStructureIdentifier, manufacturer specific, 0...0x7fff, see IEC 61158
                                             EXAMPL_4_IO_DIRECTION,         // input/output/input-output
											 EXAMPL_4_IO_DATA_TYPE);        // channel data type

                        if (Status != PNIO_OK)
						    PNIO_printf ("Error %x occured\n", PNIO_get_last_error ());

				        break;


                case 'E':
                        // *-----------------------------------------------
                        // *      provide extended channel diagnostic
                        // *      and send alarm appears, "line break"
                        // *      Note: only one diagnosis alarm at a time is possible
                        // *      Wait before sending a new alarm,
                        // *      until the previous one has been confirmed.
                        // *      (see also PNO document "diagnosis guideline" (on devkit CD)
                        // *-----------------------------------------------
                        Status = UsrExtDiag
                                    (DIAG_CHANPROP_SPEC_ERR_APP,          // new alarm appears
                                     EXAMPL_3_SLOTNUM,                    // slot number
                                     EXAMPL_3_SUBSLOTNUM,                 // subslot number
                                     EXAMPL_3_CHANNELNUM,                 // channel number
                                     EXAMPL_3_CHAN_ERR_TYPE,              // error number (see IEC61158)
                                     EXAMPL_3_EXT_CHAN_ERR_TYPE,          // ExtChannelErrorType (see IEC61158)
                                     EXAMPL_3_EXT_CHAN_ADD_VALUE,         // ExtChannelAddValue (see IEC61158)
                                     EXAMPL_3_IO_DIRECTION,               // channel is a input module
                                     EXAMPL_3_IO_DATA_TYPE,               // channel data type is BYTE
									 EXAMPL_3_DIAG_TAG
                                    );
                        if (Status != PNIO_OK)
                            PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());

                        break;

                case 'e':
                        // *---------------------------------------
                        // *      provide standard channel diagnostic
                        // *      and send alarm disappears, "line break"
                        // *      (no more errors pending)
                        // *      (see also PNO document "diagnosis guideline" (on devkit CD)
                        // *---------------------------------------
                        Status = UsrExtDiag
                                    (DIAG_CHANPROP_SPEC_ERR_DISAPP,   // alarm disappears
                                     EXAMPL_3_SLOTNUM,                // slot number
                                     EXAMPL_3_SUBSLOTNUM,             // subslot number
                                     EXAMPL_3_CHANNELNUM,             // channel number
                                     EXAMPL_3_CHAN_ERR_TYPE,          // error number (see IEC61158)
                                     EXAMPL_3_EXT_CHAN_ERR_TYPE,      // ExtChannelErrorType (see IEC61158)
                                     EXAMPL_3_EXT_CHAN_ADD_VALUE,     // ExtChannelAddValue (see IEC61158)
                                     EXAMPL_3_IO_DIRECTION,           // channel is a input module
                                     EXAMPL_3_IO_DATA_TYPE,           // channel data type is BYTE
									 EXAMPL_3_DIAG_TAG
                                    );
                        if (Status != PNIO_OK)
                            PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
                        break;

                // ***-------------------------------------------------------------------***
                // ***       P L U G  /  P U L L
                // ***-------------------------------------------------------------------***
                case 'S':
                        // *---------------------------------------
                        // * plug a single submodule  (here: Sub1 in Mod1)
                        // *---------------------------------------
                        PNIO_ConsolePrintf ("Plug Submodule 1 of Module 1...\n");
                        Addr.Geo.Slot       = 1;            // slot 1
                        Addr.Geo.Subslot    = 1;            // subslot 1
                        Status = PNIO_sub_plug (PNIO_SINGLE_DEVICE_HNDL,
                                                PNIO_DEFAULT_API,
                                                &Addr,                              // location (slot, subslot)
                                                EXAMPL_MOD_ID_SLOT1,
                                                SUBMOD_ID_DEFAULT,                  // submodule identifier
                                                64,                                 // submodule input data length
                                                0,                                  // submodule output data length
                                                PNIO_IM0_SUBMODULE,                 // Submodule supports IM0
                                                (IM0_DATA*)NULL,
                                                PNIO_S_GOOD);                       // initial iops value, only for submodules without io data

                        if (Status != PNIO_OK)
                            PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
						else
						{
							IsPlugged[Addr.Geo.Slot][Addr.Geo.Subslot] = PNIO_TRUE;
						}


                        PNIO_ConsolePrintf ("done\n");
                        break;

                case 's':
                        // *---------------------------------------
                        // * pull a submodule  (here: Sub1 in Mod1)
                        // *---------------------------------------
                        PNIO_ConsolePrintf ("Pull Submodule 1 of Module 1...\n");
                        Addr.Geo.Slot       = 1;            // slot 1
                        Addr.Geo.Subslot    = 1;            // subslot 1
                        Status = PNIO_sub_pull (PNIO_SINGLE_DEVICE_HNDL,
                                                PNIO_DEFAULT_API,
                                                &Addr);         // location (slot, subslot)
                        if (Status != PNIO_OK)
                            PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
						else
						{
							IsPlugged[Addr.Geo.Slot][Addr.Geo.Subslot] = PNIO_FALSE;
						}
                        PNIO_ConsolePrintf ("done\n");
                        break;

                // ***-------------------------------------------------------------------***
                // ***       T E S T   U T I L I T I E S
                // ***-------------------------------------------------------------------***
                case 'f':
                        Status = TcpReceiveAndFlashFirmware ();
                        break;

                #if EXAMPLE_IPAR_SERVER
                    case 'G':
					         PNIO_ConsolePrintf ("Send upload alarm for module 1...\n");
                             Addr.Geo.Slot       = EXAMPL_IPAR_SLOTNUM;
                             Addr.Geo.Subslot    = EXAMPL_IPAR_SUBSLOTNUM;
                             Status = PNIO_upload_retrieval_alarm_send
                                                 (PNIO_SINGLE_DEVICE_HNDL,
                                                  PNIO_DEFAULT_API,
                                                  &Addr,                        // geographical or logical address
                                                  &UploadAlarmData[0],          // AlarmItem.Data
                                                  sizeof (UploadAlarmData),     // length of AlarmItem.Data
                                                  0x1357);
                             if (Status != PNIO_OK)
                                     PNIO_printf ("Error %x occured\n", PNIO_get_last_error ());
                             break;

                    case 'g':
                            PNIO_ConsolePrintf ("Send retrieval alarm for module 1...\n");
                            Addr.Geo.Slot       = EXAMPL_IPAR_SLOTNUM;
                            Addr.Geo.Subslot    = EXAMPL_IPAR_SUBSLOTNUM;
                            Status = PNIO_upload_retrieval_alarm_send
                                                     (PNIO_SINGLE_DEVICE_HNDL,
                                                      PNIO_DEFAULT_API,
                                                      &Addr,                        // geographical or logical address
                                                      &RetrievalAlarmData[0],       // AlarmItem.Data
                                                      sizeof (RetrievalAlarmData),  // length of AlarmItem.Data
                                                      0x1358);
                            if (Status != PNIO_OK)
                                   PNIO_printf ("Error %x occured\n", PNIO_get_last_error ());
                             break;
                #endif

                case 'H':
                	    PrintSubmodules();
                	    break;

                case 'j':
                        // *---------------------------------------
                        // *      print io data
                        // *---------------------------------------
                        PrintAllUsedIoData ();
                        break;

                case 'Q':
                        // *---------------------------------------
                        // *      increment io data values
                        // *---------------------------------------
                        PNIO_ConsolePrintf ("increment input data values\n");
                        ChangeAllInputData (1);
                        break;

                case 'q':
                        // *---------------------------------------
                        // *      decrement io data values
                        // *---------------------------------------
                        PNIO_ConsolePrintf ("decrement input data values\n");
                        ChangeAllInputData (-1);
                        break;

                case 'N':
                        // *---------------------------------------
                        // *      input MAC address
                        // *---------------------------------------
                        {
                            PNIO_ConsolePrintf ("input mac address\n");
                            InputAndStoreMacAddress();
                        }

                        break;

			    case 'n':
					    // *---------------------------------------
					    // *      input device name
					    // *---------------------------------------
						{
	                        PNIO_ConsolePrintf ("input device name\n");
						    InputAndStoreDeviceName();
						}

                        break;

			    case 'o':
					    // *---------------------------------------
					    // *      input ip address
					    // *---------------------------------------
						{
	                        PNIO_ConsolePrintf ("input ip address\n");
						    InputAndStoreIpAddress();
						}

                        break;
#if(PNIO_TRACE != PNIO_TRACE_NONE)
                case 'T':
                        // *---------------------------------------
                        // *  set TRACE LEVEL for all
                        // *  all subsystems of a package
                        // *---------------------------------------
                        {
                            PNIO_UINT32 LowerSubsys;
                            PNIO_UINT32 AllPackages = 0;
                            PNIO_UINT32 NewTraceLevel;

                            PNIO_ConsolePrintf ("set Package trace level\n");

                            AllPackages = OsKeyScan32 ("Press 1 for all Packages or 0 for specific one = ", 10);
                            if (AllPackages == 1)
                            {
                            	NewTraceLevel  = OsKeyScan32 ("TraceLevel  = ", 10);
                            	TrcDkSetAllLevel (NewTraceLevel);
                            }
                            else
                            {
                            	LowerSubsys    = OsKeyScan32 ("LowerSubsys = ", 10);

                            	NewTraceLevel  = OsKeyScan32 ("TraceLevel  = ", 10);

                            	if ((NewTraceLevel == 0) || (LowerSubsys == 0 ))
                            	{
                            		PNIO_ConsolePrintf ("Invalid Input\n");
                            	}
                            	else
                            	{
                            		TrcDkSetPackageLevel (LowerSubsys, NewTraceLevel);
                            	}
                            }
                        }

                        break;

                case 't':
                        // *---------------------------------------
                        // *  set TRACE Level for a single subsystem
                        // *---------------------------------------
                        {
                            PNIO_UINT32 Subsys;
                            PNIO_UINT32 NewTraceLevel;
                            PNIO_ConsolePrintf ("set single subsystem trace level\n");
                            Subsys         = OsKeyScan32 ("Subsys     = ", 10);
                            NewTraceLevel  = OsKeyScan32 ("TraceLevel = ", 10);
                            TrcDkSetLevel (Subsys, NewTraceLevel);
                        }

                        break;

                case 'K':
                        // *---------------------------------------
                        // *  RS232 TRACE LEVEL
                        // *---------------------------------------
                        {
                            PNIO_UINT32 NewTraceLevel;
                            NewTraceLevel  = OsKeyScan32 ("RS232 TRACELEVEL = ", 10);
                            TrcDkSetAllMinLevel  (DEV_STD, NewTraceLevel);
                         }

                        break;

                case 'k':
                        // *---------------------------------------
                        // *  MEM TRACE LEVEL
                        // *---------------------------------------
                        {
                            PNIO_UINT32 NewTraceLevel;
                            NewTraceLevel  = OsKeyScan32 ("MEM TRACELEVEL = ", 10);
                            TrcDkSetAllMinLevel  (DEV_MEM, NewTraceLevel);
                         }

                        break;
#endif
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
                    case 'L':
                            // *---------------------------------------
                            // *  set LED on
                            // *---------------------------------------
                            SetGpioLed(1);
                    		break;

                    case 'l':
                            // *---------------------------------------
                            // *  set LED off
                            // *---------------------------------------
                    	    SetGpioLed(0);
                    		break;
#endif
                    case 'P':
                            // *---------------------------------------
                            // *  Set user printf on RS232 console
                            // *---------------------------------------
                            PNIO_ConsolePrintf ("enable serial console output\n");
                            PnioLogDest = 1;       // 0:none, 1:Console, 3: UDP console
                            PNIO_ConsolePrintf ("Set serial  console output\n");
                            break;

                    case 'p':
                            // *---------------------------------------
                            // *  disable console output
                            // *---------------------------------------
                            PNIO_ConsolePrintf ("disable console output\n");
                            PnioLogDest = 0;       // 0:none, 1:Console, 3: UDP console
                            break;
                    case 'D':
                            if(isDeviceRunning == PNIO_TRUE)
                            {
                                PNIO_ConsolePrintf ("PN device is already stared..\n");
                            }
                            else
                            {
                                PNIO_ConsolePrintf ("PN device will be started..\n");
                                PNIO_device_start(PNIO_SINGLE_DEVICE_HNDL);
                                isDeviceRunning = PNIO_TRUE;                      
                            }
                            break;
                    case 'd':
                            if(isDeviceRunning == PNIO_FALSE)
                            {
                                PNIO_ConsolePrintf ("PN device is already stopped..\n");
                            }
                            else
                            {
                                PNIO_ConsolePrintf ("PN device will be stoped..\n");
                                PNIO_device_stop(PNIO_SINGLE_DEVICE_HNDL);
                                isDeviceRunning = PNIO_FALSE;
                            }
                            break;

                    #ifdef INCLUDE_PERFORMANCE_MEASURE
                      case 'z':
                            // *---------------------------------------
                            // *      do performance measure
                            // *      (test option only)
                            // *---------------------------------------
                            ExecutePerformanceMeasure ();
                            break;
                    #endif

                    case '?':
                            // *---------------------------------------
                            // *   help: print all keyboard commands
                            // *---------------------------------------
                            PrintHelp();
                            break;

                    #if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
                        case '!':
                        {
                            extern void OsPrintMemInfo();
                            extern void OsPrintThreads();
                            OsPrintMemInfo();
                            OsPrintThreads();
                            break;
                        }
                    #endif

                case '0':
                        // *---------------------------------------
                        // *
                        // *---------------------------------------
                        PNIO_ConsolePrintf ("OsReboot in 2 sec....\n");
                        OsReboot();

                        break;

                default:
                        PNIO_ConsolePrintf ("key 0x%x pressed, press key '?' to get more info\n", PressedKey);
                        break;
            }
        }
    }


    // *----------------------------------------------------------------*
    // *
    // *  PnUsr_cbf_IoDatXch (void)
    // *
    // *----------------------------------------------------------------*
    // *  cyclic exchange of IO data
    // *
    // *  This function performs the cyclic IO data exchange
    // *  Every IO data exchange (one data read and one data write)
    // *  it is called from the PNIO stack.
    // *
    // *----------------------------------------------------------------*
    // *  Input:    ----
    // *  Output:   ----
    // *
    // *----------------------------------------------------------------*
    PNUSR_CODE_FAST PNIO_BOOL PnUsr_cbf_IoDatXch (void)
    {
        volatile PNIO_UINT32 Status;

        // *---------------------------------------------------
        // *  implement user software here
        // * (e.g. sample and read the physical inputs)
        // *---------------------------------------------------



        // *---------------------------------------------------
        // *  send input data to PNIO controller
        // *  read output data from PNIO controller
        // *---------------------------------------------------
        PNIO_dbai_enter();
        Status =  UsrDbai_initiate_data_write ();
	    if (Status != PNIO_OK)
        {
   		    //  *** do user error handling here..***..
		    PNIO_ConsolePrintf ("Error in UsrDbai_initiate_data_write()\n");
        }

        Status =  UsrDbai_initiate_data_read ();
	    if (Status != PNIO_OK)
        {
   		    //  *** do user error handling here..***..
		    PNIO_ConsolePrintf ("Error in UsrDbai_initiate_data_read()\n");
        }

        PNIO_dbai_exit();

        // *---------------------------------------------------
        // *  implement user software here
        // *  (e.g. write physical outputs)
        // *---------------------------------------------------

        return (Status);
    }



#endif


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
