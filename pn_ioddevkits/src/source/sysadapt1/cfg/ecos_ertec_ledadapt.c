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
/*  F i l e               &F: ecos_ertec_ledadapt.c                     :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Interface for LED control (if available) for PNIO stack and application  */
/*                                                                           */
/*  Note: Mandatory is to signal the BLINK event, but it is                  */
/*        up to the customer, which LED or LEDs are used for that function   */
/*                                                                           */
/*        All other LED signaling is optional, this example is only          */
/*        a recommendation.                                                  */
/*                                                                           */
/*        included signaling:                                                */
/*        RUN LED (green):    on:  IO data exchange running                  */
/*                            off: no IO data exchange                       */
/*                                                                           */
/*        ERROR LED (red):    error occured or diagnosis entry available     */
/*        MAINT LED (yellow): maintenance required or demanded               */
/*                                                                           */
/*        blink:       DCP blink request (from engineering)                  */
/*                     all LEDs (error LED, ) are blinking                   */
/*                                                                           */
/*****************************************************************************/

#include "compiler.h"

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)

    #include "glob_sys.h"
    #include "trc_if.h"
    #include "bspadapt.h"
    #include "bspledadapt.h"
    #include "ertec_inc.h"
    #include "ertec_x00.h"

    // *------------ local data for LED blink  ----------*
    static  PNIO_UINT16     IdentTimerID  = 0;
    static  PNIO_BOOL       led_state     = 0;
    static  PNIO_UINT8      LedState[PNIO_LED_NUMOF_ENTRIES];


    typedef enum
    { // LED
        // User GPIO 0..7 = ERTEC GPIO(0)....GPIO(7)
        GPIO_LED_USER_00 = 00,
        GPIO_LED_USER_01 = 01,
        GPIO_LED_USER_02 = 02,
        GPIO_LED_USER_03 = 03,
        GPIO_LED_USER_04 = 04,
        GPIO_LED_USER_05 = 05,
        GPIO_LED_USER_06 = 06,
        GPIO_LED_USER_07 = 07,
#ifndef BOARD_TYPE_STEP_3
        GPIO_LED_USER_08 = 16,
        GPIO_LED_USER_09 = 17,
        GPIO_LED_USER_10 = 18,
        GPIO_LED_USER_11 = 19,
        GPIO_LED_USER_12 = 20,
        GPIO_LED_USER_13 = 21,
        GPIO_LED_USER_14 = 22,
        GPIO_LED_USER_15 = 23,
        GPIO_LED_ERROR   = 25, // red
        GPIO_LED_MAINT   = 26, // yellow
        GPIO_LED_DIAG    = 27, // yellow
        GPIO_LED_FO2     = 31, // fo maintenance port2
#else /*BOARD_TYPE_STEP_3*/
        GPIO_LED_USER_08 = 8,
        GPIO_LED_USER_09 = 9,//11,
        GPIO_LED_USER_11 = 11,//12,
        GPIO_LED_USER_16 = 16,//13,
        GPIO_LED_USER_17 = 17,
        GPIO_LED_USER_18 = 18,
        GPIO_LED_USER_19 = 19,
        GPIO_LED_USER_23 = 23,
        GPIO_LED_MAINT   = 20, // yellow
        GPIO_LED_DIAG    = 21, // yellow
        GPIO_LED_FO2     = 22, // fo maintenance port2
        GPIO_LED_ERROR   = 25, // red
#endif /*BOARD_TYPE_STEP_3*/
        GPIO_LED_PENERGY = 28, // green
        GPIO_LED_SYNC    = 29, // green
        GPIO_LED_FO1     = 30, // fo maintenance port1
        GPIO_LED_NUMOF_ENTRIES
    } GPIO_LEDTYPE;


    static PNIO_VOID BspLed_IdentTimerCbf(PNIO_VOID)
    {
        led_state ^= 1;
        Bsp_EbSetLed(PNIO_LED_MAINT, led_state);
        Bsp_EbSetLed(PNIO_LED_ERROR, led_state);
    }

    PNIO_VOID  BspLed_Open (PNIO_VOID)
    {
        OsMemSet (LedState, 0, sizeof (LedState));
        BspLed_InitLedBlink ();

#if (IOD_INCLUDE_POF == 1)
        ertecx00GpioDirectionSet(GPIO_LED_FO1,     ERTECX00_GPIO_OUT);
        ertecx00GpioFunctionSet (GPIO_LED_FO1,     ERTECX00_GPIO_FUNC_0);
        ertecx00GpioDirectionSet(GPIO_LED_FO2,     ERTECX00_GPIO_OUT);
        ertecx00GpioFunctionSet (GPIO_LED_FO2,     ERTECX00_GPIO_FUNC_0);
        // disable FO maintenance LEDs
        Bsp_EbSetLed(PNIO_LED_FO1, 0);
        Bsp_EbSetLed(PNIO_LED_FO2, 0);
#endif
    }


    PNIO_VOID BspLed_StartLedBlink (PNIO_UINT32 frequency)
    {
        PNIO_printf ("START BLINK LED \n");
        OsStartTimer(IdentTimerID, 0, (PNIO_UINT16)(5/frequency)); /* half the period normalized on 100ms */
    }

    PNIO_VOID BspLed_StopLedBlink (PNIO_VOID)
    {
        PNIO_printf ("STOP BLINK LED \n");
        OsStopTimer(IdentTimerID);
        led_state = PNIO_FALSE;
        Bsp_EbSetLed(PNIO_LED_RUN,  LedState[PNIO_LED_RUN]);
        Bsp_EbSetLed(PNIO_LED_ERROR,LedState[PNIO_LED_ERROR]);
        Bsp_EbSetLed(PNIO_LED_MAINT,LedState[PNIO_LED_MAINT]);
    }


    // *-----------------------------------------------------------*
    // *  Bsp_EbSetLed ()
    // *
    // *  switch on/off the green/red LED on the EB
    // *  example implementation the port mac address is build by
    // *  incrementing the device mac address.
    // *
    // *
    // *  input:  Led               PNIO_LED_...
    // *          Val               1: switch on, 0: switch off
    // *  output: return            PNIO_OK
    // *                            PNIO_NOT_OK   invalid params
    // *-----------------------------------------------------------*
    PNIO_UINT32 Bsp_EbSetLed (PNIO_LEDTYPE Led, PNIO_UINT32 Val)
    {
        PNIO_UINT32 gpioNr = 0xFFFF;
        switch (Led)
        {
        /*    note: SPI boot uses GPIO 26, 27 and 31.. so this ones have to be freed from LED duty..    */
        /*          three new leds are present on SPI adapter.. use GPIO 8, 10 and 11 instead         */
            case PNIO_LED_MAINT   :
            {
                #ifndef BOARD_TYPE_STEP_3
                    if( MODE_SPI_0x03 == ( REG32( U_SCRB__BOOT_REG ) &0xf )  ||
                        ( 1 == spi_in_use ) )
                    {
                        gpioNr = 8;
                        break;
                    }
                    else
                    {
                        gpioNr = GPIO_LED_MAINT;
                        break;
                    }
                #else /*BOARD_TYPE_STEP_3*/
                        gpioNr = GPIO_LED_MAINT  ; break;
                #endif /*BOARD_TYPE_STEP_3*/

            }
            case PNIO_LED_ERROR   : gpioNr = GPIO_LED_ERROR  ; break;
            case PNIO_LED_SYNC    : gpioNr = GPIO_LED_SYNC   ; break;
            case PNIO_LED_FO1     : gpioNr = GPIO_LED_FO1    ; break;
            case PNIO_LED_FO2     :
            {
                #ifndef BOARD_TYPE_STEP_3 /*BOARD_TYPE_STEP_3*/
                    if( MODE_SPI_0x03 == ( REG32( U_SCRB__BOOT_REG ) &0xf ) ||
                        ( 1 == spi_in_use ) )
                    {
                        gpioNr = 10;
                        break;
                    }
                    else
                    {
                        gpioNr = GPIO_LED_FO2;
                        break;
                    }
                #else /*BOARD_TYPE_STEP_3*/
                    gpioNr = GPIO_LED_FO2; break;
                #endif /*BOARD_TYPE_STEP_3*/
            }
            case PNIO_LED_DIAG  :
            {
                #ifndef BOARD_TYPE_STEP_3
                    if( MODE_SPI_0x03 == ( REG32( U_SCRB__BOOT_REG ) &0xf )  ||
                        ( 1 == spi_in_use ) )
                    {
                        gpioNr = 11;
                        break;
                    }
                    else 
                    {
                        gpioNr = GPIO_LED_DIAG;
                        break;
                    }
                #else /*BOARD_TYPE_STEP_3*/
                    gpioNr = GPIO_LED_DIAG; break;
                #endif /*BOARD_TYPE_STEP_3*/
            }
            case PNIO_LED_PENERGY : gpioNr = GPIO_LED_PENERGY; break;
            case PNIO_LED_USER_00 : gpioNr = GPIO_LED_USER_00; break;
            case PNIO_LED_USER_01 : gpioNr = GPIO_LED_USER_01; break;
            case PNIO_LED_USER_02 : gpioNr = GPIO_LED_USER_02; break;
            case PNIO_LED_USER_03 : gpioNr = GPIO_LED_USER_03; break;
            case PNIO_LED_USER_04 : gpioNr = GPIO_LED_USER_04; break;
            case PNIO_LED_USER_05 : gpioNr = GPIO_LED_USER_05; break;
            case PNIO_LED_USER_06 : gpioNr = GPIO_LED_USER_06; break;
            case PNIO_LED_USER_07 : gpioNr = GPIO_LED_USER_07; break;
            case PNIO_LED_USER_08 : gpioNr = GPIO_LED_USER_08; break;
            case PNIO_LED_USER_09 : gpioNr = GPIO_LED_USER_09; break;
#ifndef BOARD_TYPE_STEP_3
            case PNIO_LED_USER_10 : gpioNr = GPIO_LED_USER_10; break;
            case PNIO_LED_USER_11 : gpioNr = GPIO_LED_USER_11; break;
            case PNIO_LED_USER_12 : gpioNr = GPIO_LED_USER_12; break;
            case PNIO_LED_USER_13 : gpioNr = GPIO_LED_USER_13; break;
            case PNIO_LED_USER_14 : gpioNr = GPIO_LED_USER_14; break;
            case PNIO_LED_USER_15 : gpioNr = GPIO_LED_USER_15; break;
#else /*BOARD_TYPE_STEP_3*/
            case PNIO_LED_USER_10 : gpioNr = GPIO_LED_USER_11; break;
            case PNIO_LED_USER_11 : gpioNr = GPIO_LED_USER_16; break;
            case PNIO_LED_USER_12 : gpioNr = GPIO_LED_USER_17; break;
            case PNIO_LED_USER_13 : gpioNr = GPIO_LED_USER_18; break;
            case PNIO_LED_USER_14 : gpioNr = GPIO_LED_USER_19; break;
            case PNIO_LED_USER_15 : gpioNr = GPIO_LED_USER_23; break;
#endif /*BOARD_TYPE_STEP_3*/
            case PNIO_LED_BLINK:
            case PNIO_LED_RUN:
            case PNIO_LED_LINKP1:
            case PNIO_LED_LINKP2:
            case PNIO_LED_LINKP3:
            case PNIO_LED_LINKP4:
            case PNIO_LED_ACTIVP1:
            case PNIO_LED_ACTIVP2:
            case PNIO_LED_ACTIVP3:
            case PNIO_LED_ACTIVP4:
            case PNIO_LED_POWER:
            default:
            {
                return PNIO_NOT_OK;
            }
        }

        if (0xFFFF != gpioNr)
        {
            if (Val)
            {
                // PNIO_printf (" ON\n");
                REG32(U_GPIO__GPIO_OUT_SET_0) = (1<<gpioNr);
            }
            else
            {
                // PNIO_printf (" OFF\n");
                REG32(U_GPIO__GPIO_OUT_CLEAR_0) = (1<<gpioNr);
            }
        }

        return PNIO_OK;
    }

    PNIO_VOID  BspLed_InitLedBlink (PNIO_VOID)
    {
        /* init timer for identify blinking */
        OsAllocTimer(&IdentTimerID, LSA_TIMER_TYPE_CYCLIC ,LSA_TIME_BASE_100MS ,BspLed_IdentTimerCbf);
    }


#endif // PNIOD_PLATFORM


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
