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
/*  F i l e               &F: auto_platform_select.h                    :F&  */
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

// *---------------------------------------------------------------------
// *   platform:  EB200plus
// *              toolkit:          GNU
// *              operating system: ECOS 
// *---------------------------------------------------------------------
#ifndef _AUTO_PLATFORM_SELECT_H
    #define _AUTO_PLATFORM_SELECT_H

    // **** select platform (possible values see compiler.h)***
#ifdef BOARD_TYPE_STEP_3
    #define PNIOD_PLATFORM       PNIOD_PLATFORM_ECOS_EB200P3
#else
    #define PNIOD_PLATFORM       PNIOD_PLATFORM_ECOS_EB200P
#endif

    // all config options; use ecosconfig to modify them
    #include <pkgconf/system.h>
    #include <cyg/error/codes.h>
#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
