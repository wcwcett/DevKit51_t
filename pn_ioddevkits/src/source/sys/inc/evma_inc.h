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
/*  F i l e               &F: evma_inc.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  adapt this configuration file                                            */
/*  to your device specific include requirements!                            */
/*                                                                           */
/*****************************************************************************/



/*****************************************************************************/
/* contents:

    - header include hierarchy

*/
/*****************************************************************************/
/* reinclude protection */


#ifndef EVMA_INC_H
#define EVMA_INC_H


/*****************************************************************************/
/* header include hierarchy */

#include "glob_sys.h"

#include "edd_cfg.h"
#include "edd_usr.h"

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P) /* for EDDP use this */
    #include "eddp_cfg.h"
    #include "eddp_usr.h"
    #include "eddp_sys.h"
    #include "ertec200p_reg.h"
    #include "bspadapt.h"
    #include "evma_cfg.h"
    #include "evma_com.h"
    #include "evma_int.h"

#else
    #error ("no valid platform selected")
#endif



//#pragma ghs section bss=".bss_sys_evma"
//#pragma ghs section text=".text_sys_evma"
//#pragma ghs section rodata=".rodata_sys_evma"

/*****************************************************************************/
/* reinclude-protection */

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
