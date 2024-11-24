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
/*  F i l e               &F: compiler_stdlibs.h                        :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  platform dependent standard include files                                */
/*                                                                           */
/*****************************************************************************/
#ifndef _COMPILER_STDLIBS_H
#define _COMPILER_STDLIBS_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

#ifdef NULL
    #undef NULL
#endif

    // *--------------------------------------------------*
    // *   TOOL variants
    // *--------------------------------------------------*


// *-------------------------------------------------------------------------*
// *   TOOL CHAIN ECOS, GNU Tools
// *-------------------------------------------------------------------------*
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
  // *----------------------------
  // * include standard files
  // *----------------------------
  #include "ecos_ertec_os_utils.h"

// *-------------------------------------------------------------------------*
// *   TOOL CHAIN MICROSOFT   (only an example, not tested !!)
// *-------------------------------------------------------------------------*
#elif (PNIOD_PLATFORM &  PNIOD_PLATFORM_WIN_EB200P)
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include <sys/timeb.h>
    #include <time.h>
    #include <process.h>
    #include <conio.h>
    #include <malloc.h>
    #include <conio.h>
    #include <windows.h>
    #include <math.h>
#else
    #error "platform not defined\n"
#endif

    #ifdef HAVE_STDIO_H
        #include <stdio.h>
    #endif
    #ifdef HAVE_STDLIB_H
        #include <stdlib.h>
    #endif
    #ifdef HAVE_STDINT_H
        #include <stdint.h>
    #endif
    #ifdef HAVE_STDDEF_H
        #include <stddef.h>
    #endif
    #ifdef HAVE_STDARG_H
        #include <stdarg.h>
    #endif
    #ifdef HAVE_STRING_H
        #include <string.h>
    #endif
    #ifdef HAVE_MATH_H
        #include <math.h>
    #endif


#ifndef NULL
    #define  NULL   (PNIO_VOID*)0;
#endif


#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif
 
/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
