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
/*  F i l e               &F: usriod_main_xhif.c                        :F&  */
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
/*  THIS MODULE SHOULD NOT BE MODIFIED BY THE PNIO USER                      */
/*  ALL USER MODIFICATION SHOULD BE DONE IN SOURCE FOR HOST DEVICE           */
/*                                                                           */
/*****************************************************************************/


/**
* @file     usriod_main_xhif.c
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


#include "compiler.h"
#include "usriod_cfg.h"
#include "usrapp_cfg.h"   // example application spec. configuration settings

#include "iod_cfg.h"

#if (EXAMPL_DEV_CONFIG_VERSION == 4)
#if(1 == IOD_USED_WITH_XHIF_HOST)
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
    #include "iodapi_rema.h"
    #include "nv_data.h"
    #include "PnUsr_Api.h"
    #include "perform_measure.h"
    #include "evaluate_edc.h"

    #if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
        //#include "hama_com.h"
        #include <cyg/hal/ertec200p_reg.h>
        #include <cyg/kernel/kapi.h>
        #include <cyg/hal/hama_timer.h>
    #endif



#include "PnUsr_xhif.h"

    /*===========================================================================*/
    #define XTESTMENUE 0

    // *------------ external data  ----------*
    extern PNIO_UINT32      PnioLogDest;            // destination for logging messages

    // *------------ external functions  ----------*
    extern PNIO_UINT32 TcpReceiveAndFlashFirmware (void);
#if(PNIO_TRACE != PNIO_TRACE_NONE)
    extern PNIO_UINT32 TcpTraceUpload (void);
#endif

    #if (defined  XTESTMENUE)  && (XTESTMENUE != 0) // for stack development only
		extern void	XTestMenue(void);
    #endif

    // *------------ internal functions  ----------*

    // *------------ public data  ----------*


    // *------------ local data  ----------*
    static PNIO_UINT32           LastRemoteApduStatus = 0;
#ifdef BOARD_TYPE_STEP_3
    static PNIO_UINT32           TskId_EDC;
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


    /* xhif test data */
    PNIO_UINT8 PnUsr_test_data_1[64];
    PNIO_UINT8 PnUsr_test_data_2[64];

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
        PNIO_UINT32 exitAppl = PNIO_FALSE;
        /* To transfer device version to Host*/
        PNIO_FW_VERSION Version;

        #if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
            //init USER LEDs (GPIO[0-7,16-23] = LED 20 to 35)
            REG32(U_GPIO__GPIO_PORT_MODE_0_L) &= ~( 0x0000FF3F );     //function == GPIO for  0- 7
            REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= ~( 0x0000FF3F );     //function == GPIO for 16-23
            REG32(U_GPIO__GPIO_IOCTRL_0) &= ~( 0x00FF00F7 );          //output
            REG32(U_GPIO__GPIO_OUT_CLEAR_0) |= 0x00FF00F7;            //clear outputs == LEDs off


            //TODO: ifdef control block will being added for P3 board led adaptation.
            //      Other usriod_main files can be reviewed for this development.

            //init special purpose LEDs (GPIO[25-31] = LED 11 - 17)
            REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= ~( 0xfffc0000 );     //function == GPIO for 25-31
            REG32(U_GPIO__GPIO_IOCTRL_0) &= ~( 0xFE000000 );          //output
            REG32(U_GPIO__GPIO_OUT_CLEAR_0) |= 0xFE000000;          //clear outputs == LEDs off
        #endif

        // *----------------------------------------------------------
        // * create and start EDC evaluation task
        // *----------------------------------------------------------
        #ifdef BOARD_TYPE_STEP_3
           Status = OsCreateThread((void(*)(void))evaluate_edc_errors, 0, (PNIO_UINT8*)"Task_EDC", TASK_EDC_POLL, OS_TASK_DEFAULT_STACKSIZE, &TskId_EDC);
           Status = OsStartThread(TskId_EDC);
        #endif

		/* Semaphore for acyclic data receival */
		OsAllocSemB(&PnioNewAcyclicSemId);

		/* Install interrupt handler and prepare callbacks */
        PnUsr_xhif_prepare_function_calls();
        PnUsr_xhif_gpio_init();

        /* Set ready flag - ertec will soon receive response with pulse on GPIO6 */
        REG32(U_GPIO__GPIO_OUT_CLEAR_0) |= (0x00000008);

		/* Wait for GPIO trigger from host - host is ready to receive */
		OsTakeSemB(PnioNewAcyclicSemId);

        Version.VerPrefix = DEVKIT_VERSION_PREFIX;
        Version.VerHh = DEVKIT_VERSION_HH;
        Version.VerH = DEVKIT_VERSION_H;
        Version.VerL = DEVKIT_VERSION_L;
        Version.VerLl = DEVKIT_VERSION_LL;

        /* Signalize startup done to host */
        PNIOext_cbf_device_startup_done(PNPB_USR_START_BOOT_OK, &Version);

        /* Main functionality - captures and executes acyclic requests from Host*/
        while(exitAppl == PNIO_FALSE)
        {
            OsTakeSemB(PnioNewAcyclicSemId);
            PnUsr_xhif_acyc_read();
        }
    }   /* MainAppl */




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

        /* *--------------------------------------------------- */
        /* *  read output data from PNIO controller */
        /* *--------------------------------------------------- */
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

        /* Read all the data from XHIF memory interface */
        PnUsr_xhif_cyclical_read();

        /* *--------------------------------------------------- */
        /* *  send input data to PNIO controller */
        /* *--------------------------------------------------- */
        if (Status != PNIO_NOT_OK)
            Status = PNIO_initiate_data_write (PNIO_SINGLE_DEVICE_HNDL);        /* input data */
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


        return (Status);
    }



#endif
#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
