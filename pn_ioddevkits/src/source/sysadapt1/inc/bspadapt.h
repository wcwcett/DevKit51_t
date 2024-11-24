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
/*  F i l e               &F: bspadapt.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Interface between EDD system adaption and the                            */
/*  OS-specific board support package                                        */
/*                                                                           */
/*****************************************************************************/
#ifndef _BSPADAPT_H
#define _BSPADAPT_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif


#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
#include "evma_inc.h"
#endif

// *** get device mac address and port mac addresses ****
PNIO_UINT32 Bsp_GetMacAddr       (PNIO_UINT8* pDevMacAddr);                          // get device mac address
PNIO_UINT32 Bsp_GetPortMacAddr   (PNIO_UINT8* pPortMacAddr, PNIO_UINT32 PortNum);    // get port mac address
PNIO_UINT32 Bsp_Init             (PNIO_VOID);

PNIO_VOID   Bsp_PofInit          (PNIO_VOID);


#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)

    // *** connect ertec switch interrupt ****
    typedef  PNIO_VOID    (*PNIO_CBF_ERTEC_SWI_INT_H)       (PNIO_UINT32 Dummy);
    typedef  PNIO_VOID    (*PNIO_CBF_ERTEC_SWI_INT_L)       (PNIO_UINT32 Dummy);
    typedef  PNIO_VOID    (*PNIO_CBF_ERTEC_TIM1_INT)        (PNIO_UINT32 Dummy);
    typedef  PNIO_VOID    (*PNIO_CBF_ERTEC_PARITY_ERR_INT)  (PNIO_UINT32 Dummy);
    typedef  PNIO_VOID    (*PNIO_CBF_ERTEC_ACCESS_ERR_INT)  (PNIO_UINT32 Dummy);

    PNIO_UINT32 Bsp_ErtecSwiIntConnect  (PNIO_CBF_ERTEC_SWI_INT_H   ErtecSwiIntH,
                                         PNIO_CBF_ERTEC_SWI_INT_L   ErtecSwiIntL);
    PNIO_UINT32 Bsp_ErtecErrIntConnect  (PNIO_CBF_ERTEC_PARITY_ERR_INT   pErtecParityErrInt,
                                         PNIO_CBF_ERTEC_ACCESS_ERR_INT   pErtecAccessErrInt);

    PNIO_VOID Bsp_EVMA_register_ISR(PNIO_UINT32 vector, PNIO_VOID (*isr)(PNIO_VOID));
    PNIO_VOID Bsp_EVMA_unregister_ISR(PNIO_UINT32 vector);

#endif

// **** LED handling on the evaluation board ***
typedef enum
{ // LED
    PNIO_LED_RUN,
    PNIO_LED_MAINT,
    PNIO_LED_ERROR,
    PNIO_LED_SYNC,
    PNIO_LED_LINKP1,
    PNIO_LED_LINKP2,
    PNIO_LED_LINKP3,
    PNIO_LED_LINKP4,
    PNIO_LED_ACTIVP1,
    PNIO_LED_ACTIVP2,
    PNIO_LED_ACTIVP3,
    PNIO_LED_ACTIVP4,
    PNIO_LED_FO1,
    PNIO_LED_FO2,
    PNIO_LED_POWER,
    PNIO_LED_DIAG,
    PNIO_LED_PENERGY,
    PNIO_LED_BLINK,
    PNIO_LED_USER_00,
    PNIO_LED_USER_01,
    PNIO_LED_USER_02,
    PNIO_LED_USER_03,
    PNIO_LED_USER_04,
    PNIO_LED_USER_05,
    PNIO_LED_USER_06,
    PNIO_LED_USER_07,
    PNIO_LED_USER_08,
    PNIO_LED_USER_09,
    PNIO_LED_USER_10,
    PNIO_LED_USER_11,
    PNIO_LED_USER_12,
    PNIO_LED_USER_13,
    PNIO_LED_USER_14,
    PNIO_LED_USER_15,
    PNIO_LED_NUMOF_ENTRIES
#ifndef PNIO_LEDTYPE_USE_ENUM_TYPE
} PNIO_LEDTYPE_ENUM;
#define PNIO_LEDTYPE    PNIO_UINT32
#else
} PNIO_LEDTYPE;
#endif

PNIO_VOID   BspLed_Open           ( PNIO_VOID );
PNIO_VOID   BspLed_StartLedBlink  ( PNIO_UINT32 frequency );
PNIO_VOID   BspLed_StopLedBlink   ( PNIO_VOID );
PNIO_UINT32 Bsp_EbSetLed          ( PNIO_LEDTYPE Led, PNIO_UINT32 Val );

typedef enum
{
    GPIO_DIR_IN  = 00,
    GPIO_DIR_OUT = 01
} PNIO_GPIO_DIR;

PNIO_UINT32 Bsp_SetGpioMode         ( PNIO_UINT32    Gpio,
                                      PNIO_UINT32    Function,
                                      PNIO_GPIO_DIR  InOut );

PNIO_VOID   Bsp_ClearGPIOout_0_to_31 ( PNIO_UINT32 OutMsk );
PNIO_VOID   Bsp_SetGPIOout_0_to_31   ( PNIO_UINT32 OutMsk );


PNIO_UINT32 Bsp_ReadGPIOin_0_to_31  ( PNIO_VOID );

typedef enum BSP_WD_GRANITY
{
    BSP_WD_100MS,
    BSP_WD_10MS,
    BSP_WD_1MS,
    BSP_WD_100US
} BSP_WD_GRANITY;

PNIO_VOID        Bsp_hw_watchdog_init    ( PNIO_UINT32 wd_time, BSP_WD_GRANITY wd_granity );
PNIO_VOID        Bsp_hw_watchdog_deinit  ( PNIO_VOID );
PNIO_VOID        Bsp_hw_watchdog_start   ( PNIO_VOID );
PNIO_VOID        Bsp_hw_watchdog_stop    ( PNIO_VOID );
PNIO_VOID        Bsp_hw_watchdog_trigger ( PNIO_VOID );

PNIO_VOID        Bsp_GPIO_set_int_for_acyc(PNIO_UINT32 int_vector, PNIO_VOID* gpio_isr, PNIO_VOID* gpio_dsr);
PNIO_VOID        Bsp_GPIO_set_int_for_acyc_confirm(PNIO_UINT32 int_vector, PNIO_VOID* gpio_isr, PNIO_VOID* gpio_dsr);
PNIO_VOID        Bsp_GPIO_isr(PNIO_UINT32 int_vector);



#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
