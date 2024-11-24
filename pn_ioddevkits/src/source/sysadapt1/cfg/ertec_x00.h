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
/*  F i l e               &F: ertec_x00.h                               :F&  */
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
/* ertecx00.h - ertecx00 startet kit board header */

/* Copyright 2004 Wind River Systems, Inc. */

/*
modification history
--------------------
01b,07aug06,mk   user timestamp.

Wind River Services - Osterfeldstrasse 84 - D-85737 Ismaning, Germany
*/

/*
This file contains I/O address and related constants for the ERTEC200 BSP.
*/

#ifndef INCertecx00h
#define INCertecx00h

#ifdef __cplusplus
extern "C" {
#endif


/* POF GPIO Ports */
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
  // fix03.2
    #define GPIO_POF2_SDA                6
    #define GPIO_POF2_SCL                7
    #define GPIO_POF1_SDA                4
    #define GPIO_POF1_SCL                5
#else     // not supported, only for simulation
    #define GPIO_POF2_SDA                0 
    #define GPIO_POF2_SCL                0
    #define GPIO_POF1_SDA                0
    #define GPIO_POF1_SCL                0
#endif



/* macros */

#ifndef _ASMLANGUAGE
    typedef enum
    {
        ERTECX00_GPIO_OUT = 0,
        ERTECX00_GPIO_IN
    } ERTECX00_GPIO_DIRECTION;

    typedef enum
    {
        ERTECX00_GPIO_FUNC_0 = 0,
        ERTECX00_GPIO_FUNC_1,
        ERTECX00_GPIO_FUNC_2,
        ERTECX00_GPIO_FUNC_3
    } ERTECX00_GPIO_FUNCTION;

#endif /* _ASMLANGUAGE */


/* function declarations */
extern PNIO_BOOL                    ertecx00GpioDirectionSet    (PNIO_UINT32 port, ERTECX00_GPIO_DIRECTION direction);
extern PNIO_BOOL                    ertecx00GpioFunctionSet     (PNIO_UINT32 port, ERTECX00_GPIO_FUNCTION function);
extern PNIO_BOOL                    ertecx00GpioIn              (PNIO_UINT32 port);
extern PNIO_UINT32                  ertecx00GpioOut             (PNIO_UINT32 port, PNIO_BOOL output);


#ifdef __cplusplus
}
#endif

#endif  /* ertec_x00.h */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
