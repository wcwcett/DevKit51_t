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
/*  F i l e               &F: compiler.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  compiler dependent definitions                                           */
/*                                                                           */
/*****************************************************************************/
#ifndef _COMPILER_H
#define _COMPILER_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif


// *------------------------------------------------------
// * include compiler1.h at the start of this file
// *------------------------------------------------------
#include "compiler1.h"      // don't change


// *========================================================*
// *   PLATFORM DEFINITIONS
// *========================================================*
// *---------------------------------------------
// * define platform  (select 1 of N)
// * we use at the moment
// *  PNIOD_PLATFORM_ECOS_EB200P     ERTEC 200P evaluation kit
// *---------------------------------------------
// **** level 1 of platforms ***
#define PNIOD_PLATFORM_NONE             0x00000000      // no valid platform
#define PNIOD_PLATFORM_ECOS_EB200P      0x00000010      // eCOS Platform ekit based eb200p + SOCK-lsa
#define PNIOD_PLATFORM_ECOS_EB200P3     (0x00001000 | PNIOD_PLATFORM_ECOS_EB200P) // ertec200p step3 platform
#define PNIOD_PLATFORM_WIN_EB200P       0x00000100      // test only: EB200P  + Windows XP


// *---------------------------------------------
// * derived platform definitions
// *---------------------------------------------
// **** level 2 of platforms ***
#define PNIOD_PLATFORM_EB200P           (PNIOD_PLATFORM_ECOS_EB200P | PNIOD_PLATFORM_WIN_EB200P)



// *--------------------------------------------------*
// *  automatic platform select
// *  (for test only, may be deleted)
// *  if no platform per auto_platform_select.h was
// *  defined, select a platform here.
// *--------------------------------------------------*
#include "auto_platform_select.h"



// *-------------------------------------------------------------------------*
// *  platform: EB200P,  tool chain GNU
// *-------------------------------------------------------------------------*
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
    // *--------------------------------------
    // * alignment orders und endian format
    // *--------------------------------------
    #define ATTR_PNIO_PACKED          __attribute__((packed))
    #define ATTR_PNIO_PACKED_PRE      // nothing

    #define PNIO_BIG_ENDIAN     0


// *-------------------------------------------------------------------------*
// *   TOOL CHAIN MICROSOFT   (only an example, not tested !!)
// *-------------------------------------------------------------------------*
#elif (PNIOD_PLATFORM & PNIOD_PLATFORM_WIN_EB200P)
    // *--------------------------------------
    // * alignment orders und endian format
    // *--------------------------------------
    #define ATTR_PNIO_PACKED                // nothing
    #define ATTR_PNIO_PACKED_PRE            // nothing

    #define PNIO_BIG_ENDIAN     0

    // **** suppress specific compiler warnings ****
    #pragma warning (disable : 4103 )
    #pragma warning (disable : 4244 )
    #pragma warning (disable : 4761 )

    #ifdef NULL
        #undef NULL
    #endif

#else
    #error "platform not defined"
#endif


// *------------------------------------------------------
// * some definitions from standard header files
// *------------------------------------------------------
#define NULL    (void*)0


// *------------------------------------------------------
// * DEBUG defines  (LSA trace macros, error logging)
// *
// * note: set to 1  increases code size.
// *------------------------------------------------------
#define  DEBUG_TASKCNT                  1      // 1: loop counter for all pn tasks

#define  DEBUG_1                        0      // for debugging only
#define  DEBUG_2                        0      // for debugging only
#define  DEBUG_3                        0      // for debugging only
#define  DEBUG_4                        0      // for debugging only


// *------------------------------------------------------
// * disable line info, to make code image independent from
// * adding or removing comment lines
// *------------------------------------------------------
#define  DISABLE_LINE_INFO          0      // 0: line info, 1: no line info, to keep image unchanged
#if (DISABLE_LINE_INFO)
    #undef  __LINE__
    #define __LINE__                0      // no line numbers, e.g. to keep image unchanged after including new comment lines
#endif

// *---- circular buffer traces, ONLY FOR DEBUGGING MODE !! ----
#define _DEBUG_LOGGING_CIRC_BUF     0     // NV data post
#define _DEBUG_LOGGING_CIRC_BUF1    0     // System Timer und Task-Timer
#define _DEBUG_LOGGING_CIRC_BUF2    0
#define _DEBUG_LOGGING_CIRC_BUF3    0
#define _DEBUG_LOGGING_CIRC_BUF4    0
#define _DEBUG_LOGGING_CIRC_BUF_FSU 0




/*-------------------------------------------------------*/
/*              1.  TRACE Settings                       */
/*-------------------------------------------------------*/
// ******* set trace output medium *******
#define PNIO_TRACE_NONE         0       // no LSA trace
#define PNIO_TRACE_DK_MEM       1       // trace output into circular memory and/or RS232 (DEFAULT)
#define PNIO_TRACE_DK_MEMXT     2       // extended trace output into circular memory and/or RS232 (DEFAULT)
#define PNIO_TRACE_DK_LSA       3       // trace output in LSA format to circular memory and/or RS232
#define PNIO_TRACE_DK_CONSOLE   4       // trace output on RS232 console
#define PNIO_TRACE_DK_UDP       5       // reserved
#define PNIO_TRACE_DK_TCP       6       // reserved

//#define PNIO_TRACE                PNIO_TRACE_DK_LSA
#define PNIO_TRACE                  PNIO_TRACE_DK_MEMXT  // we should aspire to MEMXT for DK 4.4, because realtime performance is bettera
#define PNIO_TRACE_BUFFER_SIZE      1000

#if (PNIO_TRACE != PNIO_TRACE_NONE)
// For minimum side-affect of trace on real-time performance,
// default value is set to PNIO_LOG_WARNING_HIGH. It could be higher to increase performance of trace versus sacrifice of
// realtime performance or getting risks such as loss of AR.
#define PNIO_TRACE_COMPILE_LEVEL    PNIO_LOG_WARNING_HIGH
#else
#define PNIO_TRACE_COMPILE_LEVEL    PNIO_LOG_DEACTIVATED
#endif


// *------------------------------------------------------
// * fixes for the current version
// *------------------------------------------------------

// *------------------------------------------------------
// * include compiler2.h at the end of this file
// *------------------------------------------------------
#include "compiler2.h"      // don't change
#include "iod_cfg.h"      // don't change
#include "pniousrd.h"

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif
 
/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
