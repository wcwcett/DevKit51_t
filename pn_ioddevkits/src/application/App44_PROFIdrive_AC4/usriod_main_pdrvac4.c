/*********************************************************************************************************************/
/**@file        usriod_main_pdrvac4.c
 * @brief       Main application program
 *
 * @author      Siemens AG
 * @copyright   Copyright (C) Siemens AG 2017. All rights reserved.
 *              This program is protected by German copyright law and international treaties.
 *              The use of this software including but not limited to its Source Code is subject to restrictions as
 *              agreed in the license agreement between you and Siemens.
 *              Copying or distribution is not allowed unless expressly permitted according to your license agreement
 *              with Siemens.
 *
 * @version     V4.2
 *
 * @details     Project: PROFIdrive Application Example<br>
 *              Shorthand symbol: Usr (USeR)<br>
 *              Main application program for a RT and IRT Profinet IO Device uses the Standard Interface (SI)
 *              for IO data access
 *
 *              Features:
 *                - universal interface, usable for RT and IRT in the same way
 *                - Multiple AR handling (shared device) is included in SI
 *                - includes simple terminal application, it performs
 *                    * messages via printf
 *                    * starts executing typical API-commands via terminal-keyboard
 *                    * connect terminal (e.g. Hyperterminal) via RS232 interface
 *
 *              To use this application example, set EXAMPL_DEV_CONFIG_VERSION 44
 *              in file /application/App_common/usrapp_cfg.h
 *
 * @note        this application is a simple demo application. To keep it simple, the return values of the
 *              IodApixx functions are often not analyzed. But it is strongly recommended, to do this in
 *              a real application.
 *
 * @attention   THIS MODULE HAS TO BE MODIFIED BY THE PNIO USER
 *
 * @internal
 * @note        Doxygen comments are used for automatic documentation generation.
 *              For further information about Doxygen please look at http://www.stack.nl/~dimitri/doxygen/index.html
 *              IDE Eclipse's folding feature is used too. see https://www.eclipse.org
 * @endinternal
*/
/*********************************************************************************************************************/

/**---------------------------------------------------------------------------
 * @file
 * @details
 *  FUNCTIONAL DESCRIPTION
 *
 *  This example consists of 2 tasks and one callback-function:
 *    -  a main task (MainAppl() function) , that starts the PNIO stack, plugs
 *       all modules and submodules and waits on console input afterwards.
 *       Console input commands can be used for example plug/pull module/submodule,
 *       send alarm, activate/passivate device,...
 *
 *    -  a cyclic task Task_CycleIO(), that performs IO data access. This task waits on a
 *       trigger event in an endless loop. For every trigger event one
 *       IO-read and  IO-write is performed.
 *
 *    -  a callback function, which is connected to a TransEnd-interrupt.
 *       When executed, the callback function sends a trigger via message to Task_CycleIO,
 *       that starts one IO data exchange in Task_CycleIO().
 *
 *   PROFIdrive example settings:
 *
 *     DAP:           Slot 0, Subslot 1,              has own IM0 data<br>
 *     Network-IF:    Slot 0, Subslot 0x8000,         no own IM0 data<br>
 *     Network Ports: Slot 0, Subslot 0x8001, 0x8002  no own IM0 data<br>
 *     PROFIdrive Module: Slot 1, Subslot 1, 1 input, 1 output, ModID = 0x0FFFF, no own IM0 data, MAP & PAP<br>
 *     PROFIdrive Module: Slot 1, Subslot 2, 4 Byte input, 4 Byte output, ModID = 0x0001, no own IM0 data<br>
 *
 *----------------------------------------------------------------------------
 * @note
 *  1) RT and IRT have the same application interface for IO data exchange.<br>
 *  2) In our example only the DAP has its own IM0 data (this is mandatory). All other modules
 *     (PDEV, IO-modules) don't have its own IM0 data (this is an option), so they have to respond
 *     with the IM data of the next higher level. In our example only the DAP has IM0 data,
 *     so all other subslots respond with the IM0 data of the DAP.
 *
 *----------------------------------------------------------------------------
*/

#include "compiler.h"
#include "iod_cfg.h"
#include "usriod_cfg.h"
#include "usrapp_cfg.h"   /* example application spec. configuration settings */

#if (EXAMPL_DEV_CONFIG_VERSION == 44)

#include "version_dk.h"
#include "usriod_cfg.h"
#include "pniousrd.h"
#include "bspadapt.h"
#include "iodapi_event.h"
#include "os.h"
#include "os_taskprio.h"
#include "usriod_diag.h"
#include "usriod_utils.h"
#include "usriod_im_func.h"
#include "pnio_trace.h"
//#include "iod_cfg1.h"
#include "iodapi_rema.h"
#include "nv_data.h"
#include "PnUsr_Api.h"
//#include "logadapt.h"
#include "perform_measure.h"
#include "evaluate_edc.h"

#include "pdrv_types_ac4.h"  /* PROFIdrive datatype definitions */
#include "pdrv_statemachine_ac4.h"
#include "pdrv_application_ac4.h"
#include "pdrv_diagnostics_ac4.h"
#include "pdrv_parmanager_ac4.h"
#include "pdrv_pardatabase_ac4.h"

#if ( (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P) || (PNIOD_PLATFORM & PNIOD_PLATFORM_POSIX_EB200P) )
#include "hama_com.h"
#include <cyg/hal/ertec200p_reg.h>
#include <cyg/kernel/kapi.h>
#include <cyg/hal/hama_timer.h>
#endif


/*===========================================================================*/
#define XHIF_SIZE_WORD  4096

/*------------ external data  ----------*/
extern PNIO_UINT32      PnioLogDest;            /**< destination for logging messages */
extern PNIO_UINT8     OutData		  [NUMOF_SLOTS][NUMOF_SUBSLOTS+1][NUMOF_BYTES_PER_SUBSLOT];
extern PNIO_UINT16    OutDatLen	      [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];
extern PNIO_UINT8     InData		  [NUMOF_SLOTS][NUMOF_SUBSLOTS+1][NUMOF_BYTES_PER_SUBSLOT];
extern PNIO_UINT16    InDatLen		  [NUMOF_SLOTS][NUMOF_SUBSLOTS+1];
/*------------ external functions  ----------*/
extern void PrintAllUsedIoData (void);                  /**< print io data on the terminal */
extern PNIO_UINT32 TcpReceiveAndFlashFirmware (void);   /**< receive flash image by tcp */
extern PNIO_UINT32 TcpTraceUpload (void);               /**< receive flash image by tcp */
extern PDRV_BOOL bPdrvUsr_IsIsoActiv(PDRV_VOID);
/*------------ internal functions  ----------*/

/*------------ public data  ----------*/

/*------------ application synchronization  ----------*/
PNIO_ISO_OBJ_HNDL  IsoGpioNewCycHndl = 0;
PNIO_ISO_OBJ_HNDL  ApplTransEndHndl  = 0;

/*------------ local data  ----------*/
static PNUSR_DEVICE_INSTANCE PnUsrDev;
static PNIO_UINT32           LastRemoteApduStatus = 0;

static PNIO_UINT16           xhif_mem[XHIF_SIZE_WORD] __attribute__((section(".xhif_mi_cyclic_in_buf")));
static PNIO_UINT16           *xhif_mem_remap;
static PNIO_UINT8            *xhif_mem_addr;
static PNIO_UINT8            xhif_init;

static PNIO_UINT8 SlotNums[2] = {0x02,0x01};
static PNIO_UINT8 SubNums[2]  = {0x01,0x01};

#ifdef BOARD_TYPE_STEP_3
    static PNIO_UINT32           TskId_EDC;
#endif

#if EXAMPLE_IPAR_SERVER
/** upload alarm for iPar server*/
static  PNIO_UINT8 UploadAlarmData[] =
                    {   0x0F,0x02,                          /**< blocktype */
                        0x00,0x14,                          /**< length without blocktype, length */
                        0x01,0x00,                          /**< version */
                        0x00,0x00,                          /**< padding */
                        0x01,0x00,0x00,0x01,                /**< iPar_Req_Header (01 = upload) */
                        0x00,0x00,0x00,0x04,                /**< Max_Segm_Size */
                        0x00,0x00,0x00,EXAMPL_IPAR_REC_IND, /**< Transfer_Index (record index) */
                        0x00,0x00,0x00,EXAMPL_IPAR_UPL_LEN  /**< number of bytes to store */
                    } ;

/** retrieval alarm for iPar server */
static  PNIO_UINT8 RetrievalAlarmData[] =
                    {   0x0F,0x02,                          /**< blocktype */
                        0x00,0x14,                          /**< length without blocktype, length */
                        0x01,0x00,                          /**< version */
                        0x00,0x00,                          /**< padding */
                        0x01,0x00,0x00,0x03,                /**< iPar_Req_Header  (03 = retrieval) */
                        0x00,0x00,0x00,0x04,                /**< number of bytes to store in iPar server */
                        0x00,0x00,0x00,EXAMPL_IPAR_REC_IND, /**< Transfer_Index  (record index = 50h in this example) */
                        0x00,0x00,0x00,EXAMPL_IPAR_RTV_LEN  /**< max. number of bytes to restore, 0= take all from upload */
                    };
#endif

/*-------------------------------------------------------------------------------------------*/
/** list of IO subslot (including also PDEV subslots), that shall be plugged during startup.
 *
 * @details
 * list order:  DAP first, then PDEV, then IO submodules:
 *    DAP                              (mandatory)
 *    PDEF-Interface                   (mandatory)
 *    PDEV port 1 .... PDEV port n     (mandatory)
 *    IO submodules                    (optional)
 *
 *  IO subslots can optionally be plugged or pulled later.
 *  DAP and PDEV subslots can not be pulled.
 *
 *  Restrictions  (see also example IoSubList[] below)
 *  1. exact one submodule must be the proxy for the device (must be INTERFACE subslot 0x8000)
 *  2. IM1...4 is supported
 *
 * @note    I&M0 support for the DAP is mandatory, but optional for all other submodules.
 *          In this case they have to respond to an IM0 read request with the IM0 data of the DAP, that is
 *          a proxy for the device related IM0 data here.
*/
static PNIO_SUB_LIST_ENTRY IoSubList []
= {
    /* Api  Slot    Subslot,    ModId,              SubId,                InLen,  OutLen, I&M0 support */
    {  0,   0,      1,          MODULE_ID_DAP,      SUBMOD_ID_DEFAULT,    0,      0,      PNIO_IM0_SUBMODULE + PNIO_IM0_DEVICE} ,
    {  0,   0,      0x8000,     MODULE_ID_DAP,      SUBMOD_ID_PDEV_IF,    0,      0,      PNIO_IM0_SUBMODULE},   /**< PDEV interface */
    {  0,   0,      0x8001,     MODULE_ID_DAP,      SUBMOD_ID_PDEV_PORT,  0,      0,      PNIO_IM0_SUBMODULE},   /**< PDEV port1 */
    #if (IOD_CFG_PDEV_NUMOF_PORTS >= 2)
    {  0,   0,      0x8002,     MODULE_ID_DAP,      SUBMOD_ID_PDEV_PORT,  0,      0,      PNIO_IM0_SUBMODULE},   /**< PDEV port2 */
    #endif
    #if (IOD_CFG_PDEV_NUMOF_PORTS >= 3)
    {  0,   0,      0x8003,     MODULE_ID_DAP,      SUBMOD_ID_PDEV_PORT,  0,      0,      PNIO_IM0_SUBMODULE},   /**< PDEV port3 */
    #endif
    #if (IOD_CFG_PDEV_NUMOF_PORTS >= 4)
    {  0,   0,      0x8004,     MODULE_ID_DAP,      SUBMOD_ID_PDEV_PORT,  0,      0,      PNIO_IM0_SUBMODULE},   /**< PDEV port4 */
    #endif
    {PDRV_API,   1,      1,   PDRV_MODULE_ID_MAP,      PDRV_SUBMOD_ID_PAP,      1,      1,      PNIO_IM0_SUBMODULE + PNIO_IM0_MODULE}, /**< PAP subslot */
    {PDRV_API,   1,      2,   PDRV_MODULE_ID_MAP,      PDRV_SUBMOD_ID_TLG3,     18,      10,      PNIO_IM0_SUBMODULE},    /**< standard telegram 1 subslot */
};


/* -------------------------------------------- */
/** List of IM0 data
 *
 * Api, Slot, Subslot         in machine format
 * Vendor ID, OrderID,..,...  in BIG ENDIAN FORMAT
*/
static PNIO_IM0_LIST_ENTRY Im0List []
    = {
       /** IM0 Data for the DAP (mandatory, because proxy for device) */
       {0,                                  /**< api number */
        0,                                  /**< slot number (1..0x7fff) */
        1,                                  /**< subslot number (1..0x7fff) */
           {0x2a,                           /**< VendorIDHigh, VendorIDLow */
        	IOD_CFG_DEV_ANNOTATION_ORDER_ID,/**< Order_ID, visible, must be 20 bytes here (fill with blanks) */
			IOD_CFG_IM0_SERIAL_NUMBER,     /**< IM_Serial_Number, visible string, must be 16 bytes here (fill with blanks) */
            DEVKIT_HW_REVISION,             /**< IM_Hardware_Revision */
           {DEVKIT_VERSION_PREFIX,          /**< software revision prefix */
            DEVKIT_VERSION_HH,              /**< IM_SW_Revision_Funct_Enhancement */
            DEVKIT_VERSION_H ,              /**< IM_SW_Revision_Bug_Fix */
            DEVKIT_VERSION_L},              /**< IM_SW_Revision_Internal_Change */
            DEVKIT_VERSION_LL,              /**< IM_Revision_Counter, notifies a hw modification */
            0x00,                           /**< IM_Profile_ID, see Profile Guideline I&M functions */
            0x05,                           /**< IM_Profile_Spec_Type (e.g. 5="interface module") */
            0x01,                           /**< IM_Version major */
            0x01,                           /**< IM_Version minor */
#if IOD_INCLUDE_IM5
            PNIO_SUPPORT_IM12345            /**< IM_Supported, IM1...5 supported (bit list, bit 1...bit5, here: 0 ==> IM1..5  supported) */
#else
            PNIO_SUPPORT_IM1234             /**< IM_Supported, IM1...4 supported (bit list, bit 1...bit4, here: 0 ==> IM1..4  supported) */
#endif
           }},
       /** IM0 Data for the slot 0 interface (optional) */
       {0,                                  /**< api number */
        0,                                  /**< slot number (1..0x7fff) */
        0x8000,                             /**< subslot number (1..0x7fff) */
          {0x2a,                            /**< VendorIDHigh, VendorIDLow */
           IOD_CFG_DEV_ANNOTATION_ORDER_ID, /**< Order_ID, visible, must be 20 bytes here (fill with blanks) */
		   IOD_CFG_IM0_SERIAL_NUMBER,       /**< IM_Serial_Number, visible string, must be 16 bytes here (fill with blanks) */
           DEVKIT_HW_REVISION,              /**< IM_Hardware_Revision */
           {DEVKIT_VERSION_PREFIX,          /**< software revision prefix */
           DEVKIT_VERSION_HH,               /**< IM_SW_Revision_Funct_Enhancement */
           DEVKIT_VERSION_H ,               /**< IM_SW_Revision_Bug_Fix */
           DEVKIT_VERSION_L},               /**< IM_SW_Revision_Internal_Change */
           DEVKIT_VERSION_LL,               /**< IM_Revision_Counter, notifies a hw modification */
           0x00,                            /**< IM_Profile_ID, see Profile Guideline I&M functions */
           0x05,                            /**< IM_Profile_Spec_Type (e.g. 5="interface module") */
           0x01,                            /**< IM_Version major */
           0x01,                            /**< IM_Version minor (must be 1, do not change !!) */
#if IOD_INCLUDE_IM5
           PNIO_SUPPORT_IM12345             /**< IM_Supported, IM1...5 supported (bit list, bit 1...bit5, here: 0 ==> IM1..5  supported) */
#else
           PNIO_SUPPORT_IM1234              /**< IM_Supported, IM1...4 supported (bit list, bit 1...bit4, here: 0 ==> IM1..4  supported) */
#endif
          }},
       /** IM0 Data for the slot 0 port 1 (optional) */
       {0,                                  /**< api number */
        0,                                  /**< slot number (1..0x7fff) */
        0x8001,                             /**< subslot number (1..0x7fff) */
          {0x2a,                            /**< VendorIDHigh, VendorIDLow */
           IOD_CFG_DEV_ANNOTATION_ORDER_ID, /**< Order_ID, visible, must be 20 bytes here (fill with blanks) */
		   IOD_CFG_IM0_SERIAL_NUMBER,       /**< IM_Serial_Number, visible string, must be 16 bytes here (fill with blanks) */
           DEVKIT_HW_REVISION,              /**< IM_Hardware_Revision */
           {DEVKIT_VERSION_PREFIX,          /**< software revision prefix */
           DEVKIT_VERSION_HH,               /**< IM_SW_Revision_Funct_Enhancement */
           DEVKIT_VERSION_H ,               /**< IM_SW_Revision_Bug_Fix */
           DEVKIT_VERSION_L},               /**< IM_SW_Revision_Internal_Change */
           DEVKIT_VERSION_LL,               /**< IM_Revision_Counter, notifies a hw modification */
           0x00,                            /**< IM_Profile_ID, see Profile Guideline I&M functions */
           0x05,                            /**< IM_Profile_Spec_Type (e.g. 5="interface module") */
           0x01,                            /**< IM_Version major */
           0x01,                            /**< IM_Version minor (must be 1, do not change !!) */
#if IOD_INCLUDE_IM5
           PNIO_SUPPORT_IM12345             /**< IM_Supported, IM1...5 supported (bit list, bit 1...bit5, here: 0 ==> IM1..5  supported) */
#else
           PNIO_SUPPORT_IM1234              /**< IM_Supported, IM1...4 supported (bit list, bit 1...bit4, here: 0 ==> IM1..4  supported) */
#endif
          }},
#if (IOD_CFG_PDEV_NUMOF_PORTS >= 2)
       /** IM0 Data for the slot 0 port 2 (optional) */
       {0,                                  /**< api number */
        0,                                  /**< slot number (1..0x7fff) */
        0x8002,                             /**< subslot number (1..0x7fff) */
          {0x2a,                            /**< VendorIDHigh, VendorIDLow */
           IOD_CFG_DEV_ANNOTATION_ORDER_ID,         /**< Order_ID, visible, must be 20 bytes here (fill with blanks) */
		   IOD_CFG_IM0_SERIAL_NUMBER,               /**< IM_Serial_Number, visible string, must be 16 bytes here (fill with blanks) */
           DEVKIT_HW_REVISION,              /**< IM_Hardware_Revision */
           {DEVKIT_VERSION_PREFIX,          /**< software revision prefix */
           DEVKIT_VERSION_HH,               /**< IM_SW_Revision_Funct_Enhancement */
           DEVKIT_VERSION_H ,               /**< IM_SW_Revision_Bug_Fix */
           DEVKIT_VERSION_L},               /**< IM_SW_Revision_Internal_Change */
           DEVKIT_VERSION_LL,               /**< IM_Revision_Counter, notifies a hw modification */
           0x00,                            /**< IM_Profile_ID, see Profile Guideline I&M functions */
           0x05,                            /**< IM_Profile_Spec_Type (e.g. 5="interface module") */
           0x01,                            /**< IM_Version major */
           0x01,                            /**< IM_Version minor (must be 1, do not change !!) */
#if IOD_INCLUDE_IM5
           PNIO_SUPPORT_IM12345             /**< IM_Supported, IM1...5 supported (bit list, bit 1...bit5, here: 0 ==> IM1..5  supported) */
#else
           PNIO_SUPPORT_IM1234              /**< IM_Supported, IM1...4 supported (bit list, bit 1...bit4, here: 0 ==> IM1..4  supported) */
#endif
          }},
#endif
       /** IM0 Data for the slot 1 Subslot 1 (optional) */
       {PDRV_API,                           /**< api number */
        1,                                  /**< slot number (1..0x7fff) */
        1,                                  /**< subslot number (1..0x7fff) */
          {0x2a,                            /**< VendorIDHigh, VendorIDLow */
           IOD_CFG_DEV_ANNOTATION_ORDER_ID, /**< Order_ID, visible, must be 20 bytes here (fill with blanks) */
		   IOD_CFG_IM0_SERIAL_NUMBER,       /**< IM_Serial_Number, visible string, must be 16 bytes here (fill with blanks) */
           DEVKIT_HW_REVISION,              /**< IM_Hardware_Revision */
          {'V',                             /**< software revision prefix */
           4,                               /**< IM_SW_Revision_Funct_Enhancement */
           0,                               /**< IM_SW_Revision_Bug_Fix */
           0},                              /**< IM_SW_Revision_Internal_Change */
           0x01,                            /**< IM_Revision_Counter, notifies a hw modification */
           0x00,                            /**< IM_Profile_ID, see Profile Guideline I&M functions (default: 0) */
           0x03,                            /**< IM_Profile_Spec_Type (e.g. 3="io module") */
           0x01,                            /**< IM_Version major (must be 1, do not change !!) */
           0x01,                            /**< IM_Version minor (must be 1, do not change !!) */
#if IOD_INCLUDE_IM5
           PNIO_SUPPORT_IM12345             /**< IM_Supported, IM1...5 supported (bit list, bit 1...bit5, here: 0 ==> IM1..5  supported) */
#else
           PNIO_SUPPORT_IM1234              /**< IM_Supported, IM1...4 supported (bit list, bit 1...bit4, here: 0 ==> IM1..4  supported) */
#endif
          }},

	   /** IM0 Data for the slot 1 Subslot 2 (optional) */
	   {PDRV_API,                           /**< api number */
	    1,                                  /**< slot number (1..0x7fff) */
	    2,                                  /**< subslot number (1..0x7fff) */
	      {0x2a,                            /**< VendorIDHigh, VendorIDLow */
	       IOD_CFG_DEV_ANNOTATION_ORDER_ID, /**< Order_ID, visible, must be 20 bytes here (fill with blanks) */
		   IOD_CFG_IM0_SERIAL_NUMBER,       /**< IM_Serial_Number, visible string, must be 16 bytes here (fill with blanks) */
	       DEVKIT_HW_REVISION,              /**< IM_Hardware_Revision */
	      {'V',                             /**< software revision prefix */
	       4,                               /**< IM_SW_Revision_Funct_Enhancement */
	       0,                               /**< IM_SW_Revision_Bug_Fix */
	       0},                              /**< IM_SW_Revision_Internal_Change */
	       0x01,                            /**< IM_Revision_Counter, notifies a hw modification */
	       0x00,                            /**< IM_Profile_ID, see Profile Guideline I&M functions (default: 0) */
	       0x03,                            /**< IM_Profile_Spec_Type (e.g. 3="io module") */
	       0x01,                            /**< IM_Version major (must be 1, do not change !!) */
	       0x01,                            /**< IM_Version minor (must be 1, do not change !!) */
	       PNIO_SUPPORT_IM1234              /**< IM_Supported, IM1...4 supported (bit list, bit 1...bit4, here: 0 ==> IM1..4  supported) */
	      }}
};

/** Prints the help for terminal usage
 *  @details    Prints the help for terminal usage
 *  @param      void
 *  @return     void
*/
static void PrintHelp (void)
{
    /* help */
    PNIO_ConsolePrintf ("\n\nCOMMAND LIST    ");
    PrintDevkitVersion ();

    PNIO_ConsolePrintf ("\nCONTROL:\n");
    PNIO_ConsolePrintf ("'f'      download firmware via TCP and store into flash\n");
    PNIO_ConsolePrintf ("'F'      upload trace buffers via TCP\n");
    #if EXAMPLE_IPAR_SERVER
        PNIO_ConsolePrintf ("'G'      send upload alarm (backup) to ipar-server \n");
        PNIO_ConsolePrintf ("'g'      send retrieval alarm (restore) to ipar-server  \n");
    #endif
    PNIO_ConsolePrintf ("'j'      print cyclic io data \n");
    PNIO_ConsolePrintf ("'K'      set RS232 output trace level \n");
    PNIO_ConsolePrintf ("'k'      set MEM output trace level \n");
    #if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
        PNIO_ConsolePrintf ("'L'      set LED (GPIO 0..31, (-1):all) on \n");
        PNIO_ConsolePrintf ("'l'      set LED (GPIO 0..31, (-1):all) off \n");
    #endif
		PNIO_ConsolePrintf ("'n'      set device name and store in flash \n");
        PNIO_ConsolePrintf ("'N'      set MAC address and store in flash \n");
		PNIO_ConsolePrintf ("'o'      set IP address and store into flash \n");
    PNIO_ConsolePrintf ("'P'      enable user printf output on serial console\n");
    PNIO_ConsolePrintf ("'p'      disable user printf output\n");
    PNIO_ConsolePrintf ("'T'      set trace level for a package \n");
    PNIO_ConsolePrintf ("'t'      set trace level for a single subsystem \n");
    #if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
        PNIO_ConsolePrintf ("'W'      activate   application timer-isr  and PLL-Signal on GPIO1, 2 \n");
        PNIO_ConsolePrintf ("'w'      deactivate application timer-isr  and PLL-Signals \n");
        PNIO_ConsolePrintf ("' '      print ISR counter\n");
        PNIO_ConsolePrintf ("'y'      print last remote APDU state\n");
    #endif
    PNIO_ConsolePrintf ("'z'      measure system performance\n");
    PNIO_ConsolePrintf ("'d'      perform stop of PN device \n");
    PNIO_ConsolePrintf ("'D'      perform start of PN device \n");
    PNIO_ConsolePrintf ("'0'      Perform System Reboot\n");
    PNIO_ConsolePrintf ("\n");
}

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
    unsigned int timer_val;
    unsigned int NumOfTimerIsrCalls = 0;
    unsigned int NumOfTransEndIsrCalls = 0;

    void appl_timer_isr()
    {
        NumOfTimerIsrCalls++;
        timer_val = EVMA_GET_CYCLE_TIME();
    }

    void appl_trans_end_isr()
    {
      NumOfTransEndIsrCalls++;
    }
#endif

static void spiInit(void)
{
	PNIO_UINT32 temp_reg;
	//Init SPI1 interface (GPIO 16,17,18,19 - Alternative A)
	temp_reg = REG32(U_GPIO__GPIO_PORT_MODE_0_H);
	temp_reg &= 0xffffff00;
	temp_reg |= 0x00000055;
	REG32(U_GPIO__GPIO_PORT_MODE_0_H) = temp_reg;     //function == Alternative A for 16-19
	REG16(U1_SPI__SSPCPSR) = 0x0005;//Clock prescale divisor
	REG16(U1_SPI__SSPCR0) = 0x1007; //125/10 Mbps, Motorola format, 8-bit, polarity = 0, phase = 0, SCR=1
	REG16(U1_SPI__SSPCR1) = 0x0050; //Disable all interrupt, SSP=1,SOD=0, master mode
}

static void xhifInit(void)
{
	PNIO_UINT32 temp_address;
	PNIO_UINT32 size;
	REG32(0x4000F030) = 0x01;
	PNIO_ConsolePrintf ("Remap Reg: 0X%X\n",REG32(0x4000F030));
	//Check the xhif configuration
	temp_address = (PNIO_UINT32)xhif_mem;
	PNIO_ConsolePrintf ("XHIF start address: 0X%X\n",temp_address);
	size = XHIF_SIZE_WORD*2;
	PNIO_ConsolePrintf ("End address: 0X%X\n",temp_address+size);
	REG32(U_SCRB__XHIF_MODE) = 0x00000001;                  //A20 as address line
	REG32(U_HOSTIF__XHIF_CONTROL) = 0x00000006;             //intel mode, ready high active, 16b
	REG32(U_HOSTIF__XHIF_0_P3_RG) = XHIF_SIZE_WORD*2;
	PNIO_ConsolePrintf ("Page3 Range: 0X%X\n",REG32(U_HOSTIF__XHIF_0_P3_RG));
	REG32(U_HOSTIF__XHIF_0_P3_OF) = temp_address;// - 0x20000000;
	PNIO_ConsolePrintf ("Page3 Offset: 0X%X\n",REG32(U_HOSTIF__XHIF_0_P3_OF));
	REG32(U_HOSTIF__XHIF_0_P3_CFG) = 0x00000001;
	temp_address -= 0x20000000;
	xhif_mem_remap = (PNIO_UINT16*)temp_address;
	PNIO_ConsolePrintf ("Remap address: 0X%X\n",xhif_mem_remap);
	xhif_mem_addr = (PNIO_UINT8*)temp_address;
	xhif_init = 0x01;
}

static PNIO_UINT8 SPI1WriteReadWord(PNIO_UINT8 data)
{
	PNIO_UINT8 recv_data;
	PNIO_UINT16 status;
	REG8(U1_SPI__SSPDR) = data;
	while(1)
	{
		status = REG16(U1_SPI__SSPSR);
		status &= 0x0010;
		if(status == 0) break;//No busy
	}
	recv_data = *(volatile unsigned char *)(U1_SPI__SSPDR);
	return recv_data;
}

static void IODataToXHIF(PNIO_UINT8 outputlen,PNIO_UINT8 inputlen)
{
	PNIO_INT32 IoInd;
	PNIO_UINT8 outputSlot,outputSub,inputSlot,inputSub;

	if(xhif_init == 0x00) return;
	outputSlot = SlotNums[0];
	outputSub = SubNums[0];
	inputSlot = SlotNums[1];
	inputSub = SubNums[1];

	if (0 == OutDatLen[outputSlot][outputSub])return;
	if (outputlen > OutDatLen[outputSlot][outputSub]) outputlen = OutDatLen[outputSlot][outputSub];

	if (0 == InDatLen[inputSlot][inputSub])return;
	if (inputlen > InDatLen[inputSlot][inputSub]) inputlen = InDatLen[inputSlot][inputSub];
	for (IoInd=0; IoInd < outputlen; IoInd++)
	{
		xhif_mem_addr[IoInd] = OutData[outputSlot][outputSub][IoInd];
	}
	for (IoInd=0; IoInd < inputlen; IoInd++)
	{
		InData[inputSlot][inputSub][IoInd] = xhif_mem_addr[IoInd+outputlen];
	}
}

static void IODataToSPI1(PNIO_UINT8 len)
{
	PNIO_UINT8 recv_data;
	PNIO_UINT8 check_sum;
	PNIO_UINT8 outputSlot,outputSub,inputSlot,inputSub;
	PNIO_INT32 IoInd;

	outputSlot = SlotNums[0];
	outputSub = SubNums[0];
	inputSlot = SlotNums[1];
	inputSub = SubNums[1];


	if (0 == OutDatLen[outputSlot][outputSub])return;
	if (len > OutDatLen[outputSlot][outputSub]) len = OutDatLen[outputSlot][outputSub];

	if (0 == InDatLen[inputSlot][inputSub])return;
	if (len > InDatLen[inputSlot][inputSub]) len = InDatLen[inputSlot][inputSub];
	SPI1WriteReadWord(0xAA);
	SPI1WriteReadWord((PNIO_UINT8)len);
	check_sum = (PNIO_UINT8)len;
	for (IoInd=0; IoInd < len; IoInd++)
	{
		check_sum += OutData[outputSlot][outputSub][IoInd];
		recv_data = SPI1WriteReadWord(OutData[outputSlot][outputSub][IoInd]);
		InData[inputSlot][inputSub][IoInd] = recv_data;
	}
	SPI1WriteReadWord(check_sum);
}

PNIO_UINT32  ParRW_TaskHandle;
void ParRW_Task(void)
{
	while(1)
	{
		OsWait_ms (2);
	    PdrvPar_ProcessReq();   /* acyclic parameter processing */
	}
}

/*********************************************************************************************************************/
/** main application function
 *  @details    - starts the pnio stack
 *              - starts user interface task
 *              - handles user inputs and starts the selected test functions
 *  @return     void
*/
void MainAppl (void)
{
    PNIO_BOOL   ConsoleProtection = PNIO_TRUE;
    PNIO_UINT32 Status = PNIO_OK;
    PNIO_UINT32 exit = PNIO_FALSE;

	#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
		PNIO_ISO_OBJ_HNDL  ApplGpio1Hndl     = 0;
		PNIO_ISO_OBJ_HNDL  ApplGpio2Hndl     = 0;
		PNIO_ISO_OBJ_HNDL  ApplTimHndl       = 0;
		PNIO_ISO_OBJ_HNDL  PdrvIsoHndl       = 0;
		#ifdef MINI_BOARD_V1
			REG32(U_GPIO__GPIO_PORT_MODE_1_L) &= ~(0x0000ffff);     //function == GPIO for 0-7
			REG32(U_GPIO__GPIO_IOCTRL_1) &= ~(0x000000ff);          //output
			REG32(U_GPIO__GPIO_OUT_CLEAR_1) = 0x000000ff;           //clear outputs == LEDs off
			spiInit();
	   #else
			REG32(U_GPIO__GPIO_PORT_MODE_0_L) &= ~(0x0fff0000);     //function == GPIO 8-13 8: 485_RTS ; 9-13 : LED
			REG32(U_GPIO__GPIO_IOCTRL_0) &= ~(0x00003f00);          //output
			REG32(U_GPIO__GPIO_OUT_CLEAR_0) = 0x00003f00;           //clear outputs == LEDs off
			xhifInit();
	   #endif
    #endif


    /*-----------------------------------------------------*/
    /* set startup parameter for the device
     * Note: in this simple example we suppose, the DAP has no MRP capability.
     * If MPR shall be supported, the PN controller must be capabable to send an MRP configuration record,
     * even if MRP is not activated.
     * More info to this topic see example App1_Standard, file usriod_main.c
    */
    PnUsrDev.VendorId            = IOD_CFG_VENDOR_ID;               /* Vendor ID, requested from PROFIBUS/PROFINET organization (PI) */
    PnUsrDev.DeviceId            = IOD_CFG_DEVICE_ID;               /* Device ID, must be unique inside one Vendor ID */
    PnUsrDev.MaxNumOfSubslots    = IOD_CFG_NUMOF_SUBSLOTS;          /* maximum number of subslots per slot, managable by PN Stack */
    PnUsrDev.pDevType            = (PNIO_INT8*)IOD_CFG_DEVICE_TYPE;             /* see also GSDML file, product family */

    /* startup the PN stack */
    /*-----------------------------------------------------------*/
    /* setup device stack and plug all io modules (number of plugged modules <= IOD_CFG_NUMOF_SUBSLOTS) */
    Status = PnUsr_DeviceSetup (&PnUsrDev,                                  /* device specific configuration */
                                &IoSubList[0],                              /* list of plugged submodules (real configuration), including PDEV */
                                sizeof (IoSubList) / sizeof (IoSubList[0]), /* number of entries in IoSubList */
                                &Im0List[0],                                /* list of IM0 data sets */
                                sizeof (Im0List) / sizeof (Im0List[0]));    /* number of entries in Im0List */

    // *----------------------------------------------------------
    // * create and start EDC evaluation task
    // *----------------------------------------------------------
    #ifdef BOARD_TYPE_STEP_3
       Status = OsCreateThread((void(*)(void))evaluate_edc_errors, 0, (PNIO_UINT8*)"Task_EDC", TASK_EDC_POLL, OS_TASK_DEFAULT_STACKSIZE, &TskId_EDC);
       Status = OsStartThread(TskId_EDC);
    #endif

    /*----------------------------------------------------------*/
    /* print startup result message */
    if (Status == PNIO_OK)
        PNIO_ConsolePrintf ("SYSTEM STARTUP FINISHED, OK\n");
    else
        PNIO_ConsolePrintf ("##ERROR AT SYSTEM STARTUP\n");

    #if (INCLUDE_LOGADAPT == 1)
        OsWait_ms (1000);
        LogAdaptInit ();    /* &&&2do  for LOGADAPT */
    #endif

    #if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
        /*----------------------------------------------------------*/
        /* activate NewCycle on GPIO 0 */
        Bsp_SetGpioMode (0,             /* GPIO 0 */
                         01,            /* alternate function 01 = PNPLL_OUT */
                         GPIO_DIR_OUT); /* direction output */

        /*----------------------------------------------------------*/
        /* PDRV: Initialization of PROFIdrive software parts */
        PdrvDiag_Init();
        PdrvPar_Init();

        /* Binds the PROFIdrive DO LS to the IsoIsr */
        Status = PNIO_IsoActivateIsrObj((PNIO_VOID  *)&PdrvApp_main, 0, &PdrvIsoHndl);
        PNIO_ConsolePrintf ("PROFIdrive ISO handler is %sbinded to ISO-ISR.\n", (Status != PNIO_OK)? "NOT " : "");

        Status = OsCreateThread ((void  (*)(void))ParRW_Task, 0, (PNIO_UINT8*)"ParRW_Task", TASK_PRIO_LOW, OS_TASK_DEFAULT_STACKSIZE, &ParRW_TaskHandle);
        Status = OsStartThread (ParRW_TaskHandle);

    #endif /* (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P) */

    /*----------------------------------------------------------*/
    /* endless loop: wait on key pressed */
    while (exit == PNIO_FALSE)
    {
        /*-----------------------------------*/
        /* wait on key pressed by the user */
        PNIO_INT32 PressedKey = OsGetChar();

        /* prevent from execution of command when console output is disabled */
        if (ConsoleProtection && (!PnioLogDest) && (PressedKey != '?') && (PressedKey != 'P') && (PressedKey != '*'))
        {
            continue;
        }

        switch (PressedKey)
        {
            PNIO_DEV_ADDR  Addr;     /* location (module/submodule) */

            /***-------------------------------------------------------------------***
             ***       T E S T   U T I L I T I E S
            */
#if ! ( PNIOD_PLATFORM & PNIOD_PLATFORM_LINUX_PC )
            case 'f':
                    Status = TcpReceiveAndFlashFirmware ();
                    break;

            case 'F':
                    Status = TcpTraceUpload ();
                    break;
#endif
        #if EXAMPLE_IPAR_SERVER
            case 'G':
                     PNIO_ConsolePrintf ("Send upload alarm for module 1...\n");
                     Addr.Geo.Slot       = EXAMPL_IPAR_SLOTNUM;
                     Addr.Geo.Subslot    = EXAMPL_IPAR_SUBSLOTNUM;
                     Status = PNIO_upload_retrieval_alarm_send
                                         (PNIO_SINGLE_DEVICE_HNDL,
                                          PNIO_DEFAULT_API,
                                          &Addr,                        /* geographical or logical address */
                                          &UploadAlarmData[0],          /* AlarmItem.Data */
                                          sizeof (UploadAlarmData),     /* length of AlarmItem.Data */
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
                                              &Addr,                        /* geographical or logical address */
                                              &RetrievalAlarmData[0],       /* AlarmItem.Data */
                                              sizeof (RetrievalAlarmData),  /* length of AlarmItem.Data */
                                              0x1358);
                    if (Status != PNIO_OK)
                        PNIO_printf ("Error %x occured\n", PNIO_get_last_error ());
                     break;
        #endif

            case 'j':
                    /*---------------------------------------
                     *      print io data
                    */
                    PrintAllUsedIoData ();
                    break;

            case 'D':
                    PNIO_ConsolePrintf ("PN device will be started..\n");
                    PNIO_device_start(PNIO_SINGLE_DEVICE_HNDL);
                    break;
            case 'd':
                    PNIO_ConsolePrintf ("PN device will be stopped..\n");
                    PNIO_device_stop(PNIO_SINGLE_DEVICE_HNDL);
                    break;

            case 'N':
                    /*---------------------------------------
                     *      input MAC address
                    */
                    {
                        PNIO_ConsolePrintf ("input mac address\n");
                        InputAndStoreMacAddress();
                    }

                    break;

            case 'n':
                    /*---------------------------------------
                     * input device name
                    */
                    {
                        PNIO_ConsolePrintf ("input device name\n");
                        InputAndStoreDeviceName();
                    }

                    break;

            case 'o':
                    /*---------------------------------------
                     * input ip address
                    */
                    {
                        PNIO_ConsolePrintf ("input ip address\n");
                        InputAndStoreIpAddress();
                    }

                    break;

            case 'T':
                    /*---------------------------------------
                     *  set TRACE LEVEL for all
                     *  all subsystems of a package
                    */
                    {
                        PNIO_UINT32 LowerSubsys;
                        PNIO_UINT32 NewTraceLevel;
                        PNIO_ConsolePrintf ("set Package trace level\n");
                        LowerSubsys    = OsKeyScan32 ("LowerSubsys = ", 10);
                        NewTraceLevel  = OsKeyScan32 ("TraceLevel  = ", 10);
                        TrcDkSetPackageLevel (LowerSubsys, NewTraceLevel);
                    }

                    break;

            case 't':
                    /*---------------------------------------
                     *  set TRACE Level for a single subsystem
                    */
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
                    /*---------------------------------------
                     *  RS232 TRACE LEVEL
                    */
                    {
                        PNIO_UINT32 NewTraceLevel;
                        NewTraceLevel  = OsKeyScan32 ("RS232 TRACELEVEL = ", 10);
                        TrcDkSetAllMinLevel  (DEV_STD, NewTraceLevel);
                     }

                    break;

            case 'k':
                    /*---------------------------------------
                     *  MEM TRACE LEVEL
                    */
                    {
                        PNIO_UINT32 NewTraceLevel;
                        NewTraceLevel  = OsKeyScan32 ("MEM TRACELEVEL = ", 10);
                        TrcDkSetAllMinLevel  (DEV_MEM, NewTraceLevel);
                     }

                    break;

            #if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
            case 'L':
                    /*---------------------------------------
                     *  set LED on
                    */
                    {
                        PNIO_UINT32 LedNum;
                        LedNum  = OsKeyScan32 ("LEDnum ON (0..N-1, (-1)=all) = ", 10);
                        if (LedNum == (PNIO_UINT32)-1)
                        {
                            PNIO_ConsolePrintf ("set all LEDs on\n");
                            for (LedNum = 0; LedNum < PNIO_LED_NUMOF_ENTRIES; LedNum++)
                                Bsp_EbSetLed (LedNum, 1);  /* ignore return value here, invalid values do nothing */
                        }
                        else
                        {
                            PNIO_ConsolePrintf ("set LED %d on\n", LedNum);
                            Status = Bsp_EbSetLed (LedNum, 1);
                            if (Status == PNIO_NOT_OK)
                                PNIO_ConsolePrintf ("ERROR: set unknown LED 0x%x\n", LedNum);
                        }

                     }

                    break;

            case 'l':
                    /*---------------------------------------
                     *  set LED off
                    */
                    {
                        PNIO_UINT32 LedNum;
                        LedNum  = OsKeyScan32 ("LEDnum ON (0..N-1, (-1)=all) = ", 10);
                        if (LedNum == (PNIO_UINT32)-1)
                        {
                            PNIO_ConsolePrintf ("set all LEDs off\n");
                            for (LedNum = 0; LedNum < PNIO_LED_NUMOF_ENTRIES; LedNum++)
                                Bsp_EbSetLed (LedNum, 0);  /* ignore return value here, invalid values do nothing */
                        }
                        else
                        {
                           PNIO_ConsolePrintf ("set LED %d off\n", LedNum);
                           Status = Bsp_EbSetLed (LedNum, 0);
                           if (Status == PNIO_NOT_OK)
                               PNIO_ConsolePrintf ("ERROR: set unknown LED 0x%x\n", LedNum);
                        }
                      }
                    break;
            #endif

            case 'P':
                    /*---------------------------------------
                     *  Set user printf on RS232 console
                    */
                    PNIO_ConsolePrintf ("enable serial console output\n");
                    PnioLogDest = 1;       /* 0:none, 1:Console, 3: UDP console */
                    PNIO_ConsolePrintf ("Set serial  console output\n");
                    break;

            case 'p':
                    /*---------------------------------------
                     *  disable console output
                    */
                    PNIO_ConsolePrintf ("disable console output\n");
                    PnioLogDest = 0;       /* 0:none, 1:Console, 3: UDP console */
                    break;

            #if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
            case 'W':
                  /*---------------------------------------
                   *  alloc timer event isr
                  */
                  {
                    PNIO_ConsolePrintf ("alloc timer event isr ....");
                    Status = PNIO_IsoActivateIsrObj(appl_timer_isr, 50000, &ApplTimHndl);
                    if (Status != PNIO_OK)
                        PNIO_ConsolePrintf ("ERROR occured\n");
                    else
                    {
                        PNIO_ConsolePrintf ("OK\n");
                        PNIO_UINT32 Delay;
                        PNIO_ConsolePrintf ("alloc timer event and events for  GPIO 1, 2 ....");

                        Delay = OsKeyScan32 ("Delay1 [nsec] = ", 10);
                        Status = PNIO_IsoActivateGpioObj(1, Delay,     ISO_GPIO_LOW_ACTIVE, &ApplGpio1Hndl);
                        if (Status != PNIO_OK)
                            PNIO_ConsolePrintf ("ERROR occured\n");
                        else
                            PNIO_ConsolePrintf ("OK\n");

                        Delay = OsKeyScan32 ("Delay2 [nsec] = ", 10);
                        Status = PNIO_IsoActivateGpioObj(2, Delay,     ISO_GPIO_LOW_ACTIVE, &ApplGpio2Hndl);
                        if (Status != PNIO_OK)
                            PNIO_ConsolePrintf ("ERROR occured\n");
                        else
                            PNIO_ConsolePrintf ("OK\n");
                    }
                  }
                  break;
            case 'w':
                  /*---------------------------------------
                   *  free timer event isr
                  */
                  {
                       PNIO_ConsolePrintf ("free timer event isr ....");
                       Status = PNIO_IsoFreeObj (ApplTimHndl);
                       if (Status == PNIO_OK)
                           PNIO_ConsolePrintf ("OK\n");
                       else
                           PNIO_ConsolePrintf ("ERROR occured\n");

                  }
                  {
                       PNIO_ConsolePrintf ("timer GPIO1 eventGPIO....");
                       Status = PNIO_IsoFreeObj (ApplGpio1Hndl);
                       if (Status == PNIO_OK)
                           PNIO_ConsolePrintf ("OK\n");
                       else
                           PNIO_ConsolePrintf ("ERROR occured\n");

                       PNIO_ConsolePrintf ("timer GPIO2 eventGPIO....");
                       Status = PNIO_IsoFreeObj (ApplGpio2Hndl);
                       if (Status == PNIO_OK)
                           PNIO_ConsolePrintf ("OK\n");
                       else
                           PNIO_ConsolePrintf ("ERROR occured\n");

                       /***** set back to GPIO *****/
                       Bsp_SetGpioMode (1,             /* GPIO 1 */
                                        00,            /* no alternate function 00 = GPIO */
                                        GPIO_DIR_OUT); /* direction output */

                       Bsp_SetGpioMode (2,             /* GPIO 2 */
                                        00,            /* no alternate function 00 = GPIO */
                                        GPIO_DIR_OUT); /* direction output */

                  }
                  break;

            case ' ':
                  /*---------------------------------------
                   *  print isr counter
                  */
                  PNIO_ConsolePrintf ("ApplTimCounter = %d  AppTransEndCounter = %d\n",
                               NumOfTimerIsrCalls,
                               NumOfTransEndIsrCalls);
                  break;

            case 'y':
                    /*---------------------------------------
                     *  print last APDU-Status
                    */
                    PNIO_ConsolePrintf ("last remote APDU State (AR=1) = 0x%x\n", LastRemoteApduStatus);
                    break;
            #endif /* (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P) */


            case 'X':
                  /*---------------------------------------
                   *  activate Cycle IO data exchange
                  */
                  PNIO_ConsolePrintf ("activate Cycle IO data exchange\n");
                  PnUsr_ActivateIoDatXch ();
                  break;

            case 'x':
                  /*---------------------------------------
                   *  deactivate Cycle IO data exchange
                  */
                  PNIO_ConsolePrintf ("deactivate Cycle IO data exchange\n");
                  PnUsr_DeactivateIoDatXch ();
                  break;


            #ifdef INCLUDE_PERFORMANCE_MEASURE
            case 'z':
                    /*---------------------------------------
                     *      do performance measure (test option only)
                    */
                    ExecutePerformanceMeasure ();
                    break;
            #endif

            case '?':
                    /*---------------------------------------
                     *   help: print all keyboard commands
                    */
                    PrintHelp();
                    break;

            #if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS)
            case '!':
            {
                    extern void OsPrintMemInfo();
                    extern void OsPrintThreads();
                    OsPrintMemInfo();
                    OsPrintThreads();
                    break;
            }
            #endif /* (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS) */

            case '0':
                    /*---------------------------------------
                     * Perform System Reboot
                    */
                    PNIO_ConsolePrintf ("OsReboot in 2 sec....\n");
                    OsReboot();
                    break;

            case '$':
                    /*---------------------------------------
                     * save trace buffer to non-volatile memory
                    */
                    TrcStoreBuf();
                    break;

            case '*':
                    /*---------------------------------------
                     *   toggle console protection
                    */
                    ConsoleProtection ^= PNIO_TRUE;
                    PNIO_ConsolePrintf ("Console Protection %s\n", ConsoleProtection ? "ON" : "OFF");
                    break;

            default:
                    PNIO_ConsolePrintf ("key 0x%x pressed, press key '?' to get more info\n", PressedKey);
                    break;
        }
    }
}


/*----------------------------------------------------------------*/
/** cyclic exchange of IO data
 *  @details    This function performs the cyclic IO data exchange.
 *              Every IO data exchange (one data read and one data write) it is called from the PNIO stack.
 *  @return     Status  return the status of IO communication
*/
    PNUSR_CODE_FAST OS_CODE_FAST PNIO_BOOL PnUsr_cbf_IoDatXch (void)
{
        volatile PNIO_UINT32 Status;

    /*---------------------------------------------------
     *  read output data from PNIO controller
    */
    Status = PNIO_initiate_data_read  (PNIO_SINGLE_DEVICE_HNDL);                    /* output data */
    LastRemoteApduStatus = PNIO_get_last_apdu_status (PNIO_SINGLE_DEVICE_HNDL, 1);  /* read last APDU state from first AR */

    switch (Status)
    {
        case PNIO_OK:
             break;
        case PNIO_NOT_OK:
             break;
        default:
             PNIO_ConsolePrintf ("Error 0x%x at PNIO_initiate_data_read()\n", Status);
             break;
    }

    /*---------------------------------------------------
     *  implement user software here (e.g. write physical outputs)
    */

    /*---------------------------------------------------
     *  send input data to PNIO controller
    */
    if(bPdrvUsr_IsIsoActiv() == PDRV_FALSE)
    {
		if (Status != PNIO_NOT_OK)
			Status = PNIO_initiate_data_write (PNIO_SINGLE_DEVICE_HNDL);        // input data
		switch (Status)
		{
			case PNIO_OK:
				 break;
			case PNIO_NOT_OK:
				 break;
			default:
				 PNIO_ConsolePrintf ("Error 0x%x at PNIO_initiate_data_write()\n", Status);
				 break;
		}
    }

    return (Status);
}


/*------------------------------  PROFIdrive diagnosis mapped on PROFINET IO diagnosis  ------------------------------*/

/** function sends the PROFIdrive diagnosis via PROFINET standard channel diagnostic alarm
 * @details The input parameter AlarmState specifies, if the alarm appears or disappears
 *          and if more errors are pending or not.
 *          see PDRV V4.2 chapter 8.8 Diagnosis
 * @return  PDRV_BOOL     PDRV_TRUE if success, PDRV_FALSE if failure
*/
OS_CODE_FAST PDRV_BOOL uPdrvUsr_ChanDiag
    (PDRV_UINT16 p_uAlarmState, /**< [in] error appears/disappears PDRV_CHANPROP_SPEC_ERR_... */
     PDRV_UINT16 p_uErrorNum,   /**< [in] error number, see PNIO specification coding of "ChannelErrorType" */
     PDRV_UINT16 p_uDiagTag,    /**< [in] user defined diag tag, used as reference */
     PDRV_BOOL   p_bIsWarning   /**< [in] FALSE = PDRV Fault, TRUE = PDRV Warning; corresponds to "no maintenance required" */
    )
{
    PNIO_UINT32 uStatus;
    PNIO_DEV_ADDR tAddr;           /* location (slotnumber/subslot number) */

    tAddr.Geo.Slot = PDRV_STDTLG_SLOT;
    tAddr.Geo.Subslot = PDRV_STDTLG_SUBSLOT;

    if (p_uAlarmState == PDRV_CHANPROP_SPEC_ERR_APP)
    {   /* new alarm appears */

        /* First make a diagnostic entry for this subslot. So in the following alarm PDU the ChannelDiagExist - bit
         * in the alarm specifier is set and the IO controller is notified, that diagnostic data are available. */
        uStatus = PNIO_diag_channel_add
                (   PNIO_SINGLE_DEVICE_HNDL,    /* Device handle */
                    PDRV_API,                   /* PROFIdrive API number */
                    &tAddr,                     /* location (slot/subslot) */
                    0x8000U,                    /* channel number 0x8000 whole submodul, 0..0x7fff vendor specific */
                    0x8FFFU + p_uErrorNum,      /* error number (see IEC 61158), see PDRV v4.2 table 189 */
                    DIAG_CHANPROP_DIR_IN_OUT,   /* channel direction (input, output, input-output) */
                    DIAG_CHANPROP_TYPE_OTHERS,  /* channel type (data size) */
                    p_bIsWarning,               /* no maintenance required */
                    PNIO_FALSE,                 /* no maintenance demanded */
                    p_uDiagTag);                /* user defined diag tag */
    }
    else
    { /* alarm disappears */
        uStatus = PNIO_diag_channel_remove
                (   PNIO_SINGLE_DEVICE_HNDL,
                    PDRV_API,
                    &tAddr,                     /* location (slot/subslot) */
                    0x8000U,                    /* channel number 0x8000 whole submodul, 0..0x7fff vendor specific */
                    0x8FFFU + p_uErrorNum,      /* error number (see IEC 61158), see PDRV v4.2 table 189 */
                    DIAG_CHANPROP_DIR_IN_OUT,   /* channel direction (input, output, input-output) */
                    DIAG_CHANPROP_TYPE_OTHERS,  /* channel type (data size) */
                    p_uDiagTag,                 /* user defined diag tag */
                    p_uAlarmState);             /* AlarmState: DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE */
    }

    if (uStatus != PNIO_OK)
        PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());

    return uStatus != PNIO_OK ? PDRV_FALSE : PDRV_TRUE;
}

/*-------------  PROFIdrive parameter manager assigned text functions, read functions, write functions  --------------*/

/** PROFIdrive read function for parameter PNU61000 "Name of Station"
 *  @details
 *  @return     error code
*/
PDRV_UINT32 uPdrv_RfPnu61000
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PNIO_UINT32 Status = PNIO_OK;     /* status for return value of subroutines */
    PNIO_UINT8* pNvDevName;           /* temp pointer for non volatile device name */
    PNIO_UINT32 StationNameLen;       /* temp value (size of non volatile data) */
    PDRV_UINT uI;

    /* read the devicename from the non volatile memory (e.g. flash eprom). */
    Status = Bsp_nv_data_restore (PNIO_NVDATA_DEVICENAME, (PNIO_VOID**) &pNvDevName, &StationNameLen);

    if (Status != PNIO_OK)
    {
        Status = PDRV_EV1_OP_STATE;
    }
    else
    {
        Status = PDRV_EV1_NOERROR;
        for (uI = 0U; (uI < p_uNrOfElements) && (pNvDevName[p_uSubindex + uI] != 0); uI++)
        {
            p_ptValues->os[uI] = pNvDevName[p_uSubindex + uI];
        }
        for (; uI < p_uNrOfElements; uI++)
        {
            p_ptValues->vs[uI] = ' ';
        }
    }

    Bsp_nv_data_memfree(pNvDevName);
    return Status;
}

/** PROFIdrive read function for parameter PNU61001 "IP of Station", PNU61003 "Standard Gateway of Station, PNU61004 "SubNetMask of Station"
 *  @details
 *  @return     error code
*/
PDRV_UINT32 uPdrv_RfPnu61001
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PNIO_UINT32  Status = PNIO_OK;     /* status for return value of subroutines */
    NV_IP_SUITE* pNvIpSuite;           /* temp pointer for non volatile IP suite */
    PNIO_UINT32 MemSize;               /* temp value (size of non volatile data) */
    PDRV_UINT32 uVal;
    PDRV_UINT uI;

    /* read IP suite and PDEV record data from the non volatile memory. */
    Status = Bsp_nv_data_restore (PNIO_NVDATA_IPSUITE, (PNIO_VOID**) &pNvIpSuite, &MemSize);
    if (Status != PNIO_OK)
    {
        Status = PDRV_EV1_OP_STATE;
    }
    else
    {
        Status = PDRV_EV1_NOERROR;
        if (p_ptParObj->uPnu == 61001)
            uVal = pNvIpSuite->IpAddr;
        else if (p_ptParObj->uPnu == 61003)
            uVal = pNvIpSuite->DefRouter;
        else /* PNU 61004 */
            uVal = pNvIpSuite->SubnetMask;

        for (uI = 0; uI < p_uNrOfElements; uI++)
        {
            p_ptValues->os[uI] = (uVal >> (8U * (3U - p_uSubindex - uI))) & 0xFF;
        }
    }

    Bsp_nv_data_memfree(pNvIpSuite);
    return Status;
}

/** PROFIdrive read function for parameter PNU61002 "Mac of Station"
 *  @details
 *  @return     error code
*/
PDRV_UINT32 uPdrv_RfPnu61002
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PNIO_UINT32 Status = PNIO_OK;      /* status for return value of subroutines */
    PNIO_UINT8* pMacAddr;
    PNIO_UINT32 MacAddrLen;
    PDRV_UINT uI;

     /* read the MAC data from the non volatile memory (e.g. flash eprom). */
     Status = Bsp_nv_data_restore (PNIO_NVDATA_MAC_ADDR, (PNIO_VOID**)&pMacAddr, &MacAddrLen);
     if (Status != PNIO_OK)
     {
         Status = PDRV_EV1_OP_STATE;
     }
     else
     {
         Status = PDRV_EV1_NOERROR;

         for (uI = 0; uI < p_uNrOfElements; uI++)
         {
             p_ptValues->os[uI] = pMacAddr[p_uSubindex + uI];
         }
     }

    Bsp_nv_data_memfree(pMacAddr);
    return Status;
}

#endif   /* EXAMPL_DEV_CONFIG_VERSION */
