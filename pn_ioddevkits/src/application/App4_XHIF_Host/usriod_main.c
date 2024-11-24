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
/*  F i l e               &F: usriod_main.c                             :F&  */
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
/*  uses the Standard Interface (SI) for IO data access                      */
/*                                                                           */
/*  Features:                                                                */
/*    - universal interface, usable for RT and IRT in the same way           */
/*    - Multiple AR handling (shared device) is included in SI               */
/*                                                                           */
/*    - includes simple terminal application, it performs                    */
/*        * messages via printf                                              */
/*        * starts executing typical API-commands via terminal-keyboard      */
/*        * connect terminal (e.g. Hyperterminal) via RS232 interface        */
/*                                                                           */
/*  To use this application example, set #define EXAMPL_DEV_CONFIG_VERSION 4 */
/*  in file \application\usriod_app.h                                        */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*  note: this application is a simple demo application. To keep it simple,  */
/*        the return values of the IodApixx functions are often not analyzed.*/
/*        But it is strongly recommended, to do this in a real application.  */
/*                                                                           */
/*  THIS MODULE HAS TO BE MODIFIED BY THE PNIO USER                          */
/*                                                                           */
/*****************************************************************************/


/**
* @file     usriod_main.c
* @brief    Main application of Devkit - Application example 4 - XHIF
*
* XHIF functionality allows user to use ERTEC Devkit only for PN-stack functionalities
* and to realize user functionality on other device. The other device have to upload
* firmware to ERTEC Devkit as a binary and then communicate data.. This can be realized
* via XHIF memory interface.
* This file is such implementation developed for BeagleBone Black driven by TI Sitara
* processor.
* Upper level interface - functionalities for buffers realized over memory interface
*/


/*--------------------------------------------------------------------------                */
/*  FUNCTIONAL DESCRIPTION                                                                  */
/*                                                                                          */
/*  This example consists of 2 tasks and one callback-function:                             */
/*    -  a main task (mainAppl() function) , that starts the PNIO stack, plugs              */
/*       all modules and submodules and waits on console input afterwards.                  */
/*       Console input commands can be used for example plug/pull module/submodule,         */
/*       send alarm, activate/passivate device,...                                          */
/*                                                                                          */
/*    -  a cyclic task Task_CycleIO(), that performs IO data access. This task waits on a   */
/*       trigger event in an endless loop. For every trigger event one                      */
/*       IO-read and  IO-write is performed.                                                */
/*                                                                                          */
/*    -  a callback function, which is connected to a TransEnd-interrupt.                   */
/*       When executed, the callback function sends a trigger via message to Task_CycleIO,  */
/*       that starts one IO data exchange in Task_CycleIO().                                */
/*                                                                                          */
/*   example settings:                                                                      */
/*   =================                                                                      */
/*         DAP:           Slot 0, Subslot 1,              has own IM0 data                  */
/*         Network-IF:    Slot 0, Subslot 0x8000,         no own IM0 data                   */
/*         Network Ports: Slot 0, Subslot 0x8001, 0x8002  no own IM0 data                   */
/*         Input Module:  Slot 1, Subslot 1, 64 Byte input,  ModID = 0x30, no own IM0 data  */
/*         Output Module: Slot 2, Subslot 1, 64 Byte output, ModID = 0x31, no own IM0 data  */
/*                                                                                          */
/*                                                                                          */
/*--------------------------------------------------------------------------                */
/* NOTES:                                                                                   */
/*  1) RT and IRT have the same application interface for IO data exchange.                 */
/*                                                                                          */
/*  2) In our example only the DAP has its own IM0 data (this is mandatory). All other      */
/*      modules (PDEV, IO-modules) don't have its own IM0 data (this is an option), so they */
/*      have to respond with the IM data of the next higher level. In our example only      */
/*      the DAP has IM0 data, so all other subslots respond with the IM0 data of the DAP.   */
/*                                                                                          */
/*--------------------------------------------------------------------------                */

#include "pnpb_lib.h"
#include "pnpb_lib_acyc.h"
#include "pnpb_lib_mem_int.h"
#include "usriod_im_func.h"

    /*===========================================================================*/
    
    #define NO_MENU     0

    // *------------ internal functions  ----------*

    PNIO_VOID PNIO_XHIF_test();

    // *------------ public data  ----------*

    PNIO_UINT8 pTmpIpSuite[12];
    PNIO_UINT8 pTmpMacAddr[6];
    PNIO_UINT8 pTmpDevName[256];
    PNIO_UINT16 pTmpDevNameLen;

    PNIO_UINT32 taray[6];

    // *------------ local data  ----------*
    static PNUSR_DEVICE_INSTANCE PnUsrDev;
    static PNIO_UINT32           LastRemoteApduStatus = 0;

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
    // * 2. IM1...4 is only supported for the proxy, all other submodules must have IM0.ImSupported = 0 !!
    // *
    // *
    // *-------------------------------------------------------------------------------------------
    static PNIO_SUB_LIST_ENTRY IoSubList []
    = {
        // Api  Slot    Subslot,    ModId,              SubId,                InLen,  OutLen, I&M0 support
        {  0,   0,      1,          MODULE_ID_DAP,      SUBMOD_ID_DEFAULT,    0,      0,      PNIO_IM0_SUBMODULE + PNIO_IM0_DEVICE} ,
        {  0,   0,      0x8000,     MODULE_ID_DAP,      SUBMOD_ID_PDEV_IF,    0,      0,      PNIO_IM0_SUBMODULE}, // PDEV interface
        {  0,   0,      0x8001,     MODULE_ID_DAP,      SUBMOD_ID_PDEV_PORT,  0,      0,      PNIO_IM0_SUBMODULE},   // PDEV port1
        #if (IOD_CFG_PDEV_NUMOF_PORTS >= 2)
        {  0,   0,      0x8002,     MODULE_ID_DAP,      SUBMOD_ID_PDEV_PORT,  0,      0,      PNIO_IM0_SUBMODULE},   // PDEV port2
        #endif
        #if (IOD_CFG_PDEV_NUMOF_PORTS >= 3)
        {  0,   0,      0x8003,     MODULE_ID_DAP,      SUBMOD_ID_PDEV_PORT,  0,      0,      PNIO_IM0_SUBMODULE},   // PDEV port3
        #endif
        #if (IOD_CFG_PDEV_NUMOF_PORTS >= 4)
        {  0,   0,      0x8004,     MODULE_ID_DAP,      SUBMOD_ID_PDEV_PORT,  0,      0,      PNIO_IM0_SUBMODULE},   // PDEV port4
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
            // * (mandatory, because proxy for device)
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
                0x05,                   // PNIO_UINT16     ProfSpecTyp;         // IM_Profile_Spec_Type (e.g. 5="interface module")
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
                PNIO_SUPPORT_IM12345   // PNIO_UINT16     ImSupported;         // IM_Supported, IM1...5 supported (bit list, bit 1...bit5, here: 0 ==> IM1..5  supported)
#else
                PNIO_SUPPORT_IM1234    // PNIO_UINT16     ImSupported;         // IM_Supported, IM1...4 supported (bit list, bit 1...bit4, here: 0 ==> IM1..4  supported)
#endif
                }},
        // * --------------------------------------------
        // * IM0 Data for the slot 0 port 1
        // * (optional)
        // * --------------------------------------------
           {0,                          // PNIO_UINT32     Api;                 // api number
            0,                          // PNIO_UINT32     Slot;                // slot number (1..0x7fff)
            0x8001,                     // PNIO_UINT32     Subslot;             // subslot number (1..0x7fff)
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
                PNIO_SUPPORT_IM12345   // PNIO_UINT16     ImSupported;         // IM_Supported, IM1...5 supported (bit list, bit 1...bit5, here: 0 ==> IM1..5  supported)
#else
                PNIO_SUPPORT_IM1234    // PNIO_UINT16     ImSupported;         // IM_Supported, IM1...4 supported (bit list, bit 1...bit4, here: 0 ==> IM1..4  supported)
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
#endif
            // * --------------------------------------------
            // * IM0 Data for the slot 1 (64 byte input)
            // * (optional)
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
                PNIO_SUPPORT_IM1234     // PNIO_UINT16     ImSupported;         // IM_Supported, IM1...4 supported (bit list, bit 1...bit4, here: 0 ==> IM1..4  supported)
                }},
           // * --------------------------------------------
           // * IM0 Data for the slot 2 (64 byte output)
           // * (optional)
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
               PNIO_SUPPORT_IM1234     // PNIO_UINT16     ImSupported;         // IM_Supported, IM1...4 supported (bit list, bit 1...bit4, here: 0 ==> IM1..4  supported)
               }},
    };


    // **** example data for process alarm ****
    static PNIO_UINT8     ProcessAlarmData[] = {"Process Alarm Slot 1, Subslot 1 "};           // dummy data for process alarm

    // **** example data for status alarm ****
    static PNIO_UINT8     StatusAlarmData[] = {'a', 'b', 'c', 'd'};           // dummy data for status alarm

    // *----------------------------------------------------------------*
    // *
    // *  PrintHelp (void)
    // *
    // *----------------------------------------------------------------*
    static void PrintHelp (void)
    {
        // *---------------------------------------
        // *      help
        // *---------------------------------------
        printf ("\n\nCOMMAND LIST    ");
        PrintDevkitVersion ();

        printf ("\nCONTROL:\n");
        printf ("'a'      send process alarm on slot1, subslot1\n");
        printf ("'h'      send status alarm on slot1, subslot1\n");
        printf ("'B / b'  send channel diag-alarm 'line break' appear/disapp. on slot1, subsl.1\n");
        printf ("'C / c'  send generic diag-alarm  appear/disapp. on slot0, subsl.1\n");
        printf ("'E / e'  send extended channel diag-alarm appear/disapp. on slot2, subsl.1\n");
        printf ("'Q / q'  increment/decrement input data values\n");
        #if EXAMPLE_IPAR_SERVER
            printf ("'G'      send upload alarm (backup) to ipar-server \n");
            printf ("'g'      send retrieval alarm (restore) to ipar-server  \n");
        #endif
        printf ("'H'      print plugged submodules \n");
        printf ("'j'      print cyclic io data \n");
        printf ("'m'      print memory trace buffer on console \n");
        printf ("'n'      set device name and store in flash \n");
        printf ("'N'      set MAC address and store in flash \n");
        printf ("'o'      set IP address and store into flash \n");
        printf ("'S'      plug a submodule\n");
        printf ("'s'      pull a submodule\n");
        printf ("'T'      set trace level for a package \n");
        printf ("'t'      set trace level for a single subsystem \n");
        printf ("'X'      activate cycle io update with application \n");
        printf ("'x'      deactivate cycle io update with application \n");
        printf ("'y'      print last remote APDU state\n");
        printf ("'d'      perform stop of PN device \n");
        printf ("'D'      perform start of PN device \n");
        printf ("'0'      Perform System Reboot\n");
        printf ("'$ / ='  save / restore trace buffer to non-volatile memory\n");
        printf ("\n");
    }

    static void InputAndStoreMacAddress(void)
    {
    	PNIO_UINT8* pMacAddr;
    	PNIO_UINT32 MacAddrLen;

		// **** get the current mac address from flash memory ***
		Bsp_nv_data_restore (PNIO_NVDATA_MAC_ADDR, (PNIO_VOID**)&pMacAddr, &MacAddrLen);

        /* Get MAC address */
        pnpb_scan_mac_address(pMacAddr);

        /* Save to temp variable */
        memcpy(pTmpMacAddr, pMacAddr, 6);

        /* Call to ERTEC */
        PNIOext_store_new_MAC(pMacAddr);

        // *** free ram block with private data, when no more needed ***
        Bsp_nv_data_memfree(pMacAddr);
    }

    static void InputAndStoreDeviceName(void)
    {
    	PNIO_UINT8* pDevName;
    	PNIO_UINT32 DevNameLen;
    	PNIO_UINT8 pTryDevName[256];
    	PNIO_UINT32 TryDevNameLen;

    	/* Get current device name stored in non volatile data */
    	Bsp_nv_data_restore( PNIO_NVDATA_DEVICENAME, ( PNIO_VOID** )&pDevName, &DevNameLen );

        /* By default the device name is empty/filled by 0xFF! */
        if((DevNameLen == 0) || (pDevName[0] == '\0') || (pDevName[0] == 0xFF))
        {
        	printf( "\nCurrent device name: (empty)\n");
        }
        else
        {
        	printf( "\nCurrent device name: %s\n", pDevName);
        }

        printf( "Set new device name, max length 240, longer name will be rejected\n" );
        printf( "New device name: " );

        /* Get device name */
        TryDevNameLen = OsKeyScanString(NULL, pTryDevName, 256);
        pTryDevName[TryDevNameLen] = '\0';

        /* Save to temp variable */
        memcpy(pTmpDevName, pTryDevName, TryDevNameLen);
        pTmpDevNameLen = TryDevNameLen;

        /* Call to ERTEC */
        PNIOext_store_new_device_name(pTryDevName);

        // *** free ram block with private data, when no more needed ***
        Bsp_nv_data_memfree(pDevName);
    }

    static void InputAndStoreIpAddress(void)
    {
    	PNIO_UINT8* pIpSuite;
    	PNIO_UINT32 IpSuiteLen;

    	if (0 != NumOfAr)
		{
			printf("Change of IP is forbidden for device with active AR\n");
			return;
		}
		
    	/* Get current ip stored in non volatile data */
		/* Incase of dynamic ip allocation returns 0.0.0.0 */
		Bsp_nv_data_restore( PNIO_NVDATA_IPSUITE, ( PNIO_VOID** )&pIpSuite, &IpSuiteLen );

        /* Get IP address */
        pnpb_scan_ip_address(pIpSuite);

        /* Save to temp variable */
        memcpy(pTmpIpSuite, pIpSuite, 12);

        /* Call to ERTEC */
        PNIOext_store_new_IP(pIpSuite);

        // *** free ram block with private data, when no more needed ***
        Bsp_nv_data_memfree(pIpSuite);
    }

    PNIO_VOID PNIO_XHIF_test()
    {
        pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_TEST, 0, (PNIO_UINT8*)0);
    }

    PNIO_VOID MainApplReinit()
    {
        printf("Initialize non-volatile memory...\n");
        Bsp_nv_data_init();

        // *----------------------------------------------------------
        // * set device annotation structure. The structure consists of
        // *
        // *   - device type:               user specific string, max. 25 bytes
        // *   - order ID:                  user specific string, max.   bytes
        // *   - hw revision number:            number, max. 5 bytes
        // *   - sw revision prefix          Version, Release, Prototype, Under field test, Test device
        // *   - sw revision number 1..3:   number, each 2 bytes
        // *
        // * sw revision = SwRevision1.SwRevision2.SwRevision3
        // *----------------------------------------------------------
        UsrIod_BuildDeviceAnnotation ();

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
        PnUsrDev.VendorId                   = IOD_CFG_VENDOR_ID;               // Vendor ID, requested from PROFIBUS/PROFINET organization (PI)
        PnUsrDev.DeviceId                   = IOD_CFG_DEVICE_ID;               // Device ID, must be unique inside one Vendor ID
        PnUsrDev.MaxNumOfSubslots           = IOD_CFG_NUMOF_SUBSLOTS;          // maximum number of subslots per slot, managable by PN Stack
        PnUsrDev.pDevType                   = IOD_CFG_DEVICE_TYPE;             // see also GSDML file, product family
        PnUsrDev.MaxNumOfBytesPerSubslot    = IOD_CFG_MAX_IO_NET_LENGTH_PER_SUBSLOT;

        // **** startup the PN stack ****
        // *-----------------------------------------------------------
        // *     setup device stack and plug all io modules
        // *     (number of plugged modules <= IOD_CFG_NUMOF_SUBSLOTS)
        // *-----------------------------------------------------------

        /* Send device configuration to Ertec */

        printf("Sending device config\n");
        //lint --e{866} Unusual use of '+' in argument to sizeof
        PNIOExt_DeviceSetup (&PnUsrDev,                                  // device specific configuration
                            &IoSubList[0],                              // list of plugged submodules (real configuration), including PDEV
                            sizeof (IoSubList) / sizeof (IoSubList[0]), // number of entries in IoSubList
                            &Im0List[0],                                // list of IM0 data sets
                            sizeof (Im0List) / sizeof (Im0List[0]));     // number of entries in Im0List

        sem_wait(&PnioDeviceReadySemId);

        PrintDevkitVersion();
        if(PNPB_USR_START_START_OK == PnpbDeviceStartupState)
        {
            printf ("\nSYSTEM STARTUP FINISHED, OK\n\n");
        }
        else
        {
            printf ("\n##ERROR AT SYSTEM STARTUP\n\n");
        }
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
        PNIO_BOOL   ConsoleProtection = PNPB_TRUE;
        PNIO_UINT32 Status = PNPB_OK;
        PNIO_UINT32 exitAppl = PNPB_FALSE;
        PNIO_BOOL isDeviceRunning = PNIO_FALSE;

        PNIO_UINT8  DiagAlarmData[]   = {"Diagnostic Alarm dummy data"};    // dummy data for diagnostic alarm

        printf("Initialize non-volatile memory...\n");
        Bsp_nv_data_init();

        // *----------------------------------------------------------
        // * set device annotation structure. The structure consists of
        // *
        // *   - device type:               user specific string, max. 25 bytes
        // *   - order ID:                  user specific string, max.   bytes
        // *   - hw revision number:            number, max. 5 bytes
        // *   - sw revision prefix          Version, Release, Prototype, Under field test, Test device
        // *   - sw revision number 1..3:   number, each 2 bytes
        // *
        // * sw revision = SwRevision1.SwRevision2.SwRevision3
        // *----------------------------------------------------------
        UsrIod_BuildDeviceAnnotation ();

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
        PnUsrDev.VendorId                   = IOD_CFG_VENDOR_ID;               // Vendor ID, requested from PROFIBUS/PROFINET organization (PI)
        PnUsrDev.DeviceId                   = IOD_CFG_DEVICE_ID;               // Device ID, must be unique inside one Vendor ID
        PnUsrDev.MaxNumOfSubslots           = IOD_CFG_NUMOF_SUBSLOTS;          // maximum number of subslots per slot, managable by PN Stack
        PnUsrDev.pDevType                   = IOD_CFG_DEVICE_TYPE;             // see also GSDML file, product family
        PnUsrDev.MaxNumOfBytesPerSubslot    = IOD_CFG_MAX_IO_NET_LENGTH_PER_SUBSLOT;

        // **** startup the PN stack ****
        // *-----------------------------------------------------------
        // *     setup device stack and plug all io modules
        // *     (number of plugged modules <= IOD_CFG_NUMOF_SUBSLOTS)
        // *-----------------------------------------------------------

        /* Send device configuration to Ertec */

        printf("Sending device config\n");
        //lint --e{866} Unusual use of '+' in argument to sizeof
        PNIOExt_DeviceSetup (&PnUsrDev,                                  // device specific configuration
                            &IoSubList[0],                              // list of plugged submodules (real configuration), including PDEV
                            sizeof (IoSubList) / sizeof (IoSubList[0]), // number of entries in IoSubList
                            &Im0List[0],                                // list of IM0 data sets
                            sizeof (Im0List) / sizeof (Im0List[0]));     // number of entries in Im0List

        sem_wait(&PnioDeviceReadySemId);

        PrintDevkitVersion();

        if(PNPB_USR_START_START_OK == PnpbDeviceStartupState)
        {
            printf ("\nSYSTEM STARTUP FINISHED, OK\n\n");
            Status = PNPB_OK;
            isDeviceRunning = PNIO_TRUE;
        }
        else
        {
            printf ("\n##ERROR AT SYSTEM STARTUP\n\n");
            Status = PNPB_NOT_OK;
        }

#if(NO_MENU == 0)
        // *----------------------------------------------------------
        // *endless loop: wait on key pressed
        // *----------------------------------------------------------
        while (exitAppl == PNPB_FALSE)
        {
            // *-----------------------------------
            // * wait on key pressed by the user
            // *-----------------------------------
            PNIO_INT32 PressedKey = OsGetChar();

            // prevent from execution of command when console output is disabled
            if (ConsoleProtection && (!PnioLogDest) && (PressedKey != '?') && (PressedKey != 'P') && (PressedKey != '&'))
            {
                continue;
            }

            switch (PressedKey)
            {

                PNIO_DEV_ADDR  Addr;                                // location (module/submodule)

                // ***-------------------------------------------------------------------***
                // ***       A L A R M S
                // ***-------------------------------------------------------------------***
                case 'a':
						printf("PNIOext_process_alarm_send\n");
                        // *---------------------------------------
                        // *      send process alarm
                        // *      Note: only one process alarm at a time is possible
                        // *      Wait before sending a new alarm,
                        // *      until the previous one has been confirmed.
                        // *---------------------------------------
						Addr.Geo.Slot       = EXAMPL_1_SLOTNUM;				// slot number
						Addr.Geo.Subslot    = EXAMPL_1_SUBSLOTNUM;			// subslot number

                    PNIOext_process_alarm_send (
                                             &Addr,                         // location (slot, subslot)
											sizeof (ProcessAlarmData),     // alarm data length
                                             &ProcessAlarmData[0],          // alarm data
								EXAMPL_1_USER_STRUCT_IDENT,		// 0...0x7fff: user struct. is manufac. specific
								EXAMPL_1_DIAG_TAG);	// user defined handle
                        break;

                case 'h':
						printf("PNIOext_status_alarm_send\n");
                        // *---------------------------------------
                        // *      send status alarm
                        // *      Note: only one status alarm at a time is possible
                        // *      Wait before sending a new alarm,
                        // *      until the previous one has been confirmed.
                        // *---------------------------------------
						Addr.Geo.Slot       = EXAMPL_1_SLOTNUM;				// slot number
						Addr.Geo.Subslot    = EXAMPL_1_SUBSLOTNUM;			// subslot number

                        PNIOext_status_alarm_send (
                                             &Addr,                         // location (slot, subslot)
                                             PNIO_XHIF_DEFAULT_API,         // application process identifier 
                                             sizeof (StatusAlarmData),     // alarm data length
                                             &StatusAlarmData[0],          // alarm data
                                             EXAMPL_1_USER_STRUCT_IDENT, // 0...0x7fff: user struct. is manufac. specific
                                             EXAMPL_1_DIAG_TAG);	// user defined handle
                        break;


                case 'B':
						printf("PNIOext_diag_channel_add\n");
                        // *----------------------------------------------
                        // *      provide standard channel diagnostic
                        // *      and send alarm appears, "line break"
                        // *      Note: only one diagnosis alarm at a time is possible
                        // *      Wait before sending a new alarm,
                        // *      until the previous one has been confirmed.
                        // *      (see also PNO document "diagnosis guideline" (on devkit CD)
                        // *-----------------------------------------------
						Addr.Geo.Slot       = EXAMPL_2_SLOTNUM;             // slot number
						Addr.Geo.Subslot    = EXAMPL_2_SUBSLOTNUM;          // subslot number
						PNIOext_diag_channel_add (
											&Addr,					// geographical or logical address
                                     EXAMPL_2_CHANNELNUM,                 // channel number
											EXAMPL_2_CHAN_ERR_TYPE,	// error number, see PNIO specification coding of "ChannelErrorType"
											EXAMPL_2_IO_DIRECTION,	// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
											EXAMPL_2_IO_DATA_TYPE,	// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
											EXAMPL_2_IO_MAINT_REQ,	// Maintenance required
											EXAMPL_2_IO_MAINT_DEM,	// Maintenance demanded
											EXAMPL_2_DIAG_TAG);    	// user defined diag tag, used as reference
                        break;

                case 'b':
                		printf("PNIOext_diag_channel_remove\n");
                        // *-----------------------------------------------
                        // *      provide standard channel diagnostic
                        // *      and send alarm disappears, "line break"
                        // *      (no more errors pending)
                        // *      Note: only one diagnosis alarm at a time is possible
                        // *      Wait before sending a new alarm,
                        // *      until the previous one has been confirmed.
                        // *      (see also PNO document "diagnosis guideline" (on devkit CD)
                        // *-----------------------------------------------
						Addr.Geo.Slot = EXAMPL_2_SLOTNUM;			// slot number
						Addr.Geo.Subslot = EXAMPL_2_SUBSLOTNUM;		// subslot number
                        PNIOext_diag_channel_remove(
											&Addr,							// geographical or logical address
                                     EXAMPL_2_CHANNELNUM,             // channel number
											EXAMPL_2_CHAN_ERR_TYPE,			// error number, see PNIO specification coding of "ChannelErrorType"
											EXAMPL_2_IO_DIRECTION,			// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
											EXAMPL_2_IO_DATA_TYPE,			// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
											EXAMPL_2_DIAG_TAG,    			// user defined diag tag, used as reference
											DIAG_CHANPROP_SPEC_ERR_DISAPP);	// DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
                        break;

                case 'C':
                		printf("PNIOext_diag_generic_add\n");
                        // *-----------------------------------------------
                        // *      send generic diagnostic
                        // *      alarm appears
                        // *      Note: only one diagnosis alarm at a time is possible
                        // *      Wait before sending a new alarm,
                        // *      until the previous one has been confirmed.
                        // *-----------------------------------------------
                        Addr.Geo.Slot = EXAMPL_4_SLOTNUM;			// slot number
                        Addr.Geo.Subslot = EXAMPL_4_SUBSLOTNUM;		// subslot number
                        PNIOext_diag_generic_add(
											&Addr,						// geographical or logical address
                                             EXAMPL_4_CHANNELNUM,           // channel number
											EXAMPL_4_IO_DIRECTION,	 	// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
											EXAMPL_4_IO_DATA_TYPE,	 	// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
											EXAMPL_4_DIAG_TAG,    		// user defined diag tag, used as reference
											EXAMPL_4_USER_STRUCT_IDENT,	// manufacturer specific, 0...0x7fff, see IEC 61158
											sizeof (DiagAlarmData),		// length of generic diagnostic data
											&DiagAlarmData[0],			// user defined generic diagnostic data
											EXAMPL_4_IO_MAINT_REQ,	 	// Maintenance required
											EXAMPL_4_IO_MAINT_DEM);	 	// Maintenance demanded
                        break;

                case 'c':
                		printf("PNIOext_diag_generic_remove\n");
                        // *-----------------------------------------------
                        // *      send generic diagnostic
                        // *      alarm disappears (no more errors
                        // *      pending)
                        // *      Note: only one diagnosis alarm at a time is possible
                        // *      Wait before sending a new alarm,
                        // *      until the previous one has been confirmed.
                        // *-----------------------------------------------
                        Addr.Geo.Slot = EXAMPL_4_SLOTNUM;			// slot number
                        Addr.Geo.Subslot = EXAMPL_4_SUBSLOTNUM;		// subslot number
                        PNIOext_diag_generic_remove(
											&Addr,							// geographical or logical address
                                             EXAMPL_4_CHANNELNUM,           // channel number
											EXAMPL_4_IO_DIRECTION,	 		// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
											EXAMPL_4_IO_DATA_TYPE,	 		// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
											EXAMPL_4_DIAG_TAG,    			// user defined diag tag, used as reference
											EXAMPL_4_USER_STRUCT_IDENT);	// manufacturer specific, 0...0x7fff, see IEC 61158
                        break;



                case 'E':
					printf("PNIOext_ext_diag_channel_add\n");
                        // *-----------------------------------------------
                        // *      provide extended channel diagnostic
                        // *      and send alarm appears, "line break"
                        // *      Note: only one diagnosis alarm at a time is possible
                        // *      Wait before sending a new alarm,
                        // *      until the previous one has been confirmed.
                        // *      (see also PNO document "diagnosis guideline" (on devkit CD)
                        // *-----------------------------------------------
                        Addr.Geo.Slot = EXAMPL_3_SLOTNUM;			// slot number
                        Addr.Geo.Subslot = EXAMPL_3_SUBSLOTNUM;		// subslot number
                        PNIOext_ext_diag_channel_add(
											&Addr,		// geographical or logical address
                                     EXAMPL_3_CHANNELNUM,                 // channel number
											EXAMPL_3_CHAN_ERR_TYPE,		// error number, see PNIO specification coding of "ChannelErrorType"
											EXAMPL_3_IO_DIRECTION,		// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
											EXAMPL_3_IO_DATA_TYPE,		// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
                                     EXAMPL_3_EXT_CHAN_ERR_TYPE,          // ExtChannelErrorType (see IEC61158)
                                     EXAMPL_3_EXT_CHAN_ADD_VALUE,         // ExtChannelAddValue (see IEC61158)
											EXAMPL_3_IO_MAINT_REQ,	 	// Maintenance required
											EXAMPL_3_IO_MAINT_DEM,	 	// Maintenance demanded
											EXAMPL_3_DIAG_TAG);    		// user defined diag tag, used as reference
                        break;

                case 'e':
                	printf("PNIOext_ext_diag_channel_remove\n");
                        // *---------------------------------------
                        // *      provide standard channel diagnostic
                        // *      and send alarm disappears, "line break"
                        // *      (no more errors pending)
                        // *      Note: only one diagnosis alarm at a time is possible
                        // *      Wait before sending a new alarm,
                        // *      until the previous one has been confirmed.
                        // *      (see also PNO document "diagnosis guideline" (on devkit CD)
                        // *---------------------------------------
                        Addr.Geo.Slot = EXAMPL_3_SLOTNUM;			// slot number
                        Addr.Geo.Subslot = EXAMPL_3_SUBSLOTNUM;		// subslot number
                        PNIOext_ext_diag_channel_remove(
											&Addr,							// geographical or logical address
                                     EXAMPL_3_CHANNELNUM,             // channel number
											EXAMPL_3_CHAN_ERR_TYPE,			// error number, see PNIO specification coding of "ChannelErrorType"
											EXAMPL_3_IO_DIRECTION,			// channel direction (DIAG_CHANPROP_DIR_IN, DIAG_CHANPROP_DIR_OUT,...)
											EXAMPL_3_IO_DATA_TYPE,			// channel type (DIAG_CHANPROP_TYPE_BYTE, DIAG_CHANPROP_TYPE_WORD,...)
                                     EXAMPL_3_EXT_CHAN_ERR_TYPE,      // ExtChannelErrorType (see IEC61158)
                                     EXAMPL_3_EXT_CHAN_ADD_VALUE,     // ExtChannelAddValue (see IEC61158)
											EXAMPL_3_DIAG_TAG,    			// user defined diag tag, used as reference
											DIAG_CHANPROP_SPEC_ERR_DISAPP);	// DIAG_CHANPROP_SPEC_ERR_APP, DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE
                        break;

                // ***-------------------------------------------------------------------***
                // ***       P L U G  /  P U L L
                // ***-------------------------------------------------------------------***
                case 'S':
                #if IOD_INCLUDE_DR
                        /*---------------------------------------
                         * plug any submodule
                         * -------------------------------------*/
                        InputAndPlugSubmodule();
                #else
                        // *---------------------------------------
                        // * plug a single submodule  (here: Sub1 in Mod1)
                        // *---------------------------------------
                        printf ("Plug Submodule 1 of Module 1...\n");
                        Addr.Geo.Slot       = 1;            // slot 1
                        Addr.Geo.Subslot    = 1;            // subslot 1
						PNIOext_sub_plug(&Addr,						// location (slot, subslot)
                                        EXAMPL_MOD_ID_SLOT1,
                                        SUBMOD_ID_DEFAULT,                  // submodule identifier
                                        64,                                 // in data len
                                        0,                                  // out data len
                                        PNIO_IM0_SUBMODULE,                 // Submodule supports IM0
                                        (IM0_DATA*)NULL,
                                        PNIO_S_GOOD);                        // initial iops value, only for submodules without io data

                        printf ("done\n");
                #endif
                        break;
                case 's':
                #if IOD_INCLUDE_DR
                        /*---------------------------------------
                         * pull any submodule
                         * -------------------------------------*/
                        InputAndPullSubmodule();
                #else
                        // *---------------------------------------
                        // * pull a submodule  (here: Sub1 in Mod1)
                        // *---------------------------------------
                        printf ("Pull Submodule 1 of Module 1...\n");
                        Addr.Geo.Slot       = 1;            // slot 1
                        Addr.Geo.Subslot    = 1;            // subslot 1
                        PNIOext_sub_pull (&Addr);	// location (slot, subslot)

                        printf ("done\n");
                #endif
                        break;

                // ***-------------------------------------------------------------------***
                // ***       T E S T   U T I L I T I E S
                // ***-------------------------------------------------------------------***
            #if EXAMPLE_IPAR_SERVER
                case 'G':
                         printf ("Send upload alarm for module 1...\n");
                         Addr.Geo.Slot       = EXAMPL_IPAR_SLOTNUM;
                         Addr.Geo.Subslot    = EXAMPL_IPAR_SUBSLOTNUM;
                         PNIOext_upload_retrieval_alarm_send(
                                              &Addr,                        // geographical or logical address
                                              sizeof (UploadAlarmData),     // length of AlarmItem.Data
											&UploadAlarmData[0],		// AlarmItem.Data
											0x1357);	 				// user defined handle
                         break;

                case 'g':
                        printf ("Send retrieval alarm for module 1...\n");
                        Addr.Geo.Slot       = EXAMPL_IPAR_SLOTNUM;
                        Addr.Geo.Subslot    = EXAMPL_IPAR_SUBSLOTNUM;
                        PNIOext_upload_retrieval_alarm_send(
                                                  &Addr,                        // geographical or logical address
                                                  sizeof (RetrievalAlarmData),  // length of AlarmItem.Data
											&RetrievalAlarmData[0],			// AlarmItem.Data
											0x1358);	 					// user defined handle
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
                        printf ("increment input data values\n");
                        ChangeAllInputData (1);
                        break;

                case 'q':
                        // *---------------------------------------
                        // *      decrement io data values
                        // *---------------------------------------
                        printf ("decrement input data values\n");
                        ChangeAllInputData (-1);
                        break;

                case 'D':
                        if(isDeviceRunning == PNIO_TRUE)
                        {
                            printf ("PN device is already started..\n");
                        }
                        else
                        {
                            printf ("PN device will be started..\n");
                            PNIOext_device_start();
                            isDeviceRunning = PNIO_TRUE;                      
                        }
                        break;
                case 'd':
                        if(isDeviceRunning == PNIO_FALSE)
                        {
                            printf ("PN device is already stopped..\n");
                        }
                        else
                        {
                            printf ("PN device will be stoped..\n");
                            PNIOext_device_stop();
                            isDeviceRunning = PNIO_FALSE;
                        }
                        break;

                case 'm':
                        // *---------------------------------------
                        // * print memory trace buffer on console
                        // *---------------------------------------
                        printf ("print trace buffer\n");
                        PNIOext_trace_command(PNIO_TRACE_PRINT_START);
                        break;

                case 'N':
                        printf ("input mac address\n");
                        InputAndStoreMacAddress();
                        break;

                case 'n':
                        printf ("input device name\n");
                        InputAndStoreDeviceName();
                        break;

                case 'o':
                        printf ("input ip address\n");
                        InputAndStoreIpAddress();
                        break;

                case 'T':
                        // *---------------------------------------
                        // *  set TRACE LEVEL for all
                        // *  all subsystems of a package
                        // *---------------------------------------

                	    {
                            PNIO_BOOL ParamCorrect = PNIO_OK;
                            PNIO_UINT32 LowerSubsys;
                            PNIO_UINT32 NewTraceLevel;
                            PNIO_UINT32 AllPackages = 0;

                            printf ("set Package trace level\n");


                            AllPackages = OsKeyScan32 ("Press 1 for all Packages or 0 for specific one = ", 10);
                            if (AllPackages == 1)
                            {
                            	NewTraceLevel  = OsKeyScan32 ("TraceLevel  = ", 10);
                            	PNIOext_trace_settings(PNIO_TRACE_ALL_SUBMODULES, TRACE_SUBSYS_NUM, NewTraceLevel);
                            }
                            else
                            {
                                LowerSubsys    = OsKeyScan32 ("LowerSubsys = ", 10);
                                NewTraceLevel  = OsKeyScan32 ("TraceLevel  = ", 10);

                                if ((NewTraceLevel > PNIO_LOG_CHAT) || (NewTraceLevel == 0))
                                {
                                     printf ("ERROR: (TrcDkSetPackageLevel) invalid trace level %d\n", NewTraceLevel);
                                     ParamCorrect = PNIO_NOT_OK;
                                }

                                switch(LowerSubsys)
                                {
                                    case TRACE_SUBSYS_ACP_LOWER:
						 	   	    case TRACE_SUBSYS_CLRPC_LOWER:
						 	   	    case TRACE_SUBSYS_CM_LOWER:
						 	   	    case TRACE_SUBSYS_DCP_LOWER:
						 	   	    case TRACE_SUBSYS_GSY_LOWER:
						 	   	    case TRACE_SUBSYS_NARE_LOWER:
						 	   	    case TRACE_SUBSYS_LLDP_LOWER:
						 	   	    case TRACE_SUBSYS_OHA_LOWER:
						 	   	    case TRACE_SUBSYS_EDDP_LOWER:
						 	   	    case TRACE_SUBSYS_MRP_LOWER:
						 	   	    case TRACE_SUBSYS_SOCK_LOWER:
						 	   	    case TRACE_SUBSYS_POF_LOWER:
						 	   	    case TRACE_SUBSYS_IOD_LOWER:
						 	   	    case TRACE_SUBSYS_PNPB_LOWER:
						 	   	    case TRACE_SUBSYS_PNDV_LOWER:
						 	   	    case TRACE_SUBSYS_TCIP_LOWER:
						 	   	    case TRACE_SUBSYS_APPL_LOWER:
						 	   	    case TRACE_SUBSYS_TSKMSG_LOWER:
						 	   	    {
						 	   	    	/* Valid submodule as is specified in Ertec code */
						 	   	    	break;
						 	   	    }
						 	   	    default:
						 	   	    {
	                                         printf ("ERROR: (TrcDkSetPackageLevel) invalid subsystem %d\n", LowerSubsys);
	                                         ParamCorrect = PNIO_NOT_OK;
						 	   	    }
                                }

                                if(ParamCorrect == PNIO_OK)
                                {
                                    PNIOext_trace_settings(PNIO_TRACE_ALL_SUBMODULES, LowerSubsys, NewTraceLevel);
                                }
                            }
                        }
                        break;

                case 't':
                        // *---------------------------------------
                        // *  set TRACE Level for a single subsystem
                        // *---------------------------------------
                        {
                            PNIO_BOOL ParamCorrect = PNIO_OK;
                            PNIO_UINT32 Subsys;
                            PNIO_UINT32 NewTraceLevel;
                            printf ("set single subsystem trace level\n");
                            Subsys         = OsKeyScan32 ("Subsys     = ", 10);
                            NewTraceLevel  = OsKeyScan32 ("TraceLevel = ", 10);

                            if ((NewTraceLevel > PNIO_LOG_CHAT) || (NewTraceLevel == 0))
                            {
                                 printf ("ERROR: (TrcDkSetLevel) invalid trace level %d\n", NewTraceLevel);
                                 ParamCorrect = PNIO_NOT_OK;
                            }

                            if (Subsys > TRACE_SUBSYS_NUM)
                            {
                                 printf ("ERROR: (TrcDkSetLevel) invalid subsystem %d\n", Subsys);
                                 ParamCorrect = PNIO_NOT_OK;
                            }

                            if(ParamCorrect == PNIO_OK)
                            {
                                PNIOext_trace_settings(PNIO_TRACE_SINGE_SUBMODULE, Subsys, NewTraceLevel);
                            }
                        }
                        break;

                case 'y':
                        // *---------------------------------------
                        // *  print last APDU-Status
                        // *---------------------------------------
                        printf ("last remote APDU State (AR=1) = 0x%x\n", PNIOext_get_last_apdu_status(1));
                        break;


                case 'X':
                        // *---------------------------------------
                        // *  activate Cycle IO data exchange
                        // *---------------------------------------
                        printf ("activate Cycle IO data exchange\n");
                        PNIOext_ActivateIoDatXch();

                        break;

                case 'x':
                        // *---------------------------------------
                        // *  deactivate Cycle IO data exchange
                        // *---------------------------------------
                        printf ("deactivate Cycle IO data exchange\n");
                        PNIOext_DeactivateIoDatXch();

                        break;

                case '?':
                        // *---------------------------------------
                        // *   help: print all keyboard commands
                        // *---------------------------------------
                        PrintHelp();
                        break;

                case '0':
                        // *---------------------------------------
                        // *
                        // *---------------------------------------
                        printf ("OsReboot....\n");
                        pnpb_lib_reboot();
                        MainApplReinit();
                        break;

                case '$':
                        // *---------------------------------------
                        // *
                        // *---------------------------------------
                        PNIOext_trace_command(PNIO_TRACE_SAVE);

                        break;

                case '&':
                        // *---------------------------------------
                        // *   toggle console protection
                        // *---------------------------------------
                        ConsoleProtection ^= PNPB_TRUE;
                        printf ("Console Protection %s\n", ConsoleProtection ? "ON" : "OFF");
                        break;

                case '=':
                {
                        PNIOext_trace_command(PNIO_TRACE_RESTORE);

                        break;
                }

                default:
                        printf ("key 0x%x pressed, press key '?' to get more info\n", PressedKey);
            }
        }
#else
        while(1)
        {
            sleep(1);
        }
#endif
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
    PNIO_BOOL PnUsr_cbf_IoDatXch (void)
    {
        volatile PNIO_UINT32 Status;

        // *---------------------------------------------------
        // *  send input data to PNIO controller
        // *---------------------------------------------------


        Status = pnpb_xhif_send_all_IO_data();
        switch (Status)
        {
            case PNPB_OK:
                 break;
            default:
                printf("Cyclical write failed\n");
                PNPB_LIB_FATAL
                 break;
        }


        // *---------------------------------------------------
        // *  read output data from PNIO controller
        // *---------------------------------------------------
        Status = pnpb_xhif_cyclical_read();
        switch (Status)
        {
            case PNPB_OK:
                 break;
            default:
                printf("Cyclical read failed\n");
                PNPB_LIB_FATAL
                 break;
        }

        return (Status);
    }


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
