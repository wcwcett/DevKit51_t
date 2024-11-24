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
/*  F i l e               &F: version_dk.h                              :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*   version file                                                            */
/*                                                                           */
/*****************************************************************************/
#ifndef VERSION_DK_H
#define VERSION_DK_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif


#include "compiler.h"

// *-------------------------------------------------
// *     Device - software version
// *     current version: 4.6.0.0
// *
// *     This values are used in the endpoint mapper
// *     and the I&M0 device proxy (DAP)
// *-------------------------------------------------

// *** define software version ***
/* point of version view

   meaning of letters:

   - development
    R   release
    C   correction
    S   special
    T   test
    B   lab
   - external beta testing
    P   pilot
   - customer
    V   version
    K   correction
    D   demonstration

*/


#if ( (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P))

    #define GLOB_FW_VERSION_LETTER                    'V'
    #define GLOB_FW_VERSION_FUNCTION                  5
    #define GLOB_FW_VERSION_RELEASE                   1
    #define GLOB_FW_VERSION_SERVICE_PACK              0

    /* developments point of version view */
    #if ('B' != GLOB_FW_VERSION_LETTER)
        /* V,R,... official view: developments point of version view */
      #define GLOB_FW_VERSION_PROJECT                   ((10 * GLOB_FW_VERSION_FUNCTION)+ GLOB_FW_VERSION_RELEASE) /* fixed as long as we work on the same customer version above */
      #define GLOB_FW_VERSION_INCREMENT                 0  /* should be increased on major changes during development that cause incompatibilities */
      #define GLOB_FW_VERSION_INTEGRATION               11  /* increment when generating a new version ! */
      #define GLOB_FW_VERSION_GENERATION                0  /* fix used to check for additional compatibility issues*/
    #else
        /* B: internal view: every build passed to the integration test has to have a unique build number - do not forget */
      #define GLOB_FW_VERSION_PROJECT                   ((10 * GLOB_FW_VERSION_FUNCTION)+ GLOB_FW_VERSION_RELEASE) /* fixed as long as we work on the same customer version above */
      #define GLOB_FW_VERSION_INCREMENT                 0
      #define GLOB_FW_VERSION_INTEGRATION               1 /* increment when generating a new version ! */
      #define GLOB_FW_VERSION_GENERATION                94  /* fix used to check for additional compatibility issues*/
    #endif

#endif



#if (   ('V' == GLOB_FW_VERSION_LETTER)                                         \
     || ('K' == GLOB_FW_VERSION_LETTER)                                         \
     || ('D' == GLOB_FW_VERSION_LETTER)                                         \
     || ('P' == GLOB_FW_VERSION_LETTER)                                         \
    )

    #define GLOB_FW_VERSION_0                         GLOB_FW_VERSION_LETTER
    #define GLOB_FW_VERSION_1                         GLOB_FW_VERSION_FUNCTION
    #define GLOB_FW_VERSION_2                         GLOB_FW_VERSION_RELEASE
    #define GLOB_FW_VERSION_3                         GLOB_FW_VERSION_SERVICE_PACK
    #define GLOB_FW_VERSION_4                         GLOB_FW_VERSION_GENERATION

#elif (   ('R' == GLOB_FW_VERSION_LETTER)                                       \
       || ('C' == GLOB_FW_VERSION_LETTER)                                       \
       || ('S' == GLOB_FW_VERSION_LETTER)                                       \
       || ('T' == GLOB_FW_VERSION_LETTER)                                       \
       || ('B' == GLOB_FW_VERSION_LETTER)                                       \
      )

    #define GLOB_FW_VERSION_0                         GLOB_FW_VERSION_LETTER
    #define GLOB_FW_VERSION_1                         GLOB_FW_VERSION_PROJECT
    #define GLOB_FW_VERSION_2                         GLOB_FW_VERSION_INCREMENT
    #define GLOB_FW_VERSION_3                         GLOB_FW_VERSION_INTEGRATION
    #define GLOB_FW_VERSION_4                         GLOB_FW_VERSION_GENERATION

#else

    #error GLOBAL CONFIGURATION: IM_FW_VERSION_LETTER invalid !

#endif



#define DEVKIT_VERSION_PREFIX   GLOB_FW_VERSION_0                   // must be V, R, P, U, T, see PNIO spec., search for field 'SWRevisionPrefix"
#define DEVKIT_VERSION_HH       GLOB_FW_VERSION_1                   // highest element of version
#define DEVKIT_VERSION_H        GLOB_FW_VERSION_2                   // high element of version
#define DEVKIT_VERSION_L        GLOB_FW_VERSION_3                   // low element of version
#define DEVKIT_VERSION_LL       GLOB_FW_VERSION_4                   // lowest element of version

// *** devive hardware version ***
#define DEVKIT_HW_REVISION            IOD_CFG_HW_REVISION
/*snmp system description data*/
#define DEVKIT_VENDOR                 IOD_CFG_DEVKIT_VENDOR
#define DEVKIT_PRODUCTFAMILY          IOD_CFG_DEVKIT_PRODUCTFAMILY
#define DEVKIT_PRODUCTNAME            IOD_CFG_DEVKIT_PRODUCTNAME
#define DEVKIT_ORDERNUMBER            IOD_CFG_DEV_ANNOTATION_ORDER_ID
#define DEVKIT_PRODUCTSERIALNUMBER    IOD_CFG_IM0_SERIAL_NUMBER


// *** date string (used in example application only)  ***
#if ((PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P))
    #define DATE_VERSION_STR       "2020-06-23 08:00"
#endif



#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
