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
/*  F i l e               &F: iom_inc.h                                 :F&  */
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

#ifndef IOM_INC_H_
#define IOM_INC_H_

#include "glob_sys.h"
#include "pnio_types.h"

#include "edd_cfg.h"
#include "cm_cfg.h"
#include "trace_cfg.h"
#include "pndv_cfg.h"

#include "clrpc_inc.h"

#include "edd_usr.h"

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P) /* for EDDP use this */

    #include "arm926.h"
    #include "hama_com.h"
    #include "ertec200p_reg.h"
    #include "bspadapt.h"

    #include "eddp_cfg.h"
    #include "eddp_usr.h"
    #include "eddp_sys.h"
    #include "gdma_cfg.h"
    #include "gdma_com.h"

#else
    #error ("no valid platform selected")
#endif

#include "mrp_cfg.h"
#include "mrp_sys.h"
#include "mrp_usr.h"

#include "cm_usr.h" /* needs the pointers with attributes */
#include "cm_sys.h"

#include "pndv_com.h"

#include "iom_cfg.h"
#include "iom_plau.h"
#include "iom_com.h"
#include "iom_int.h"

#endif /* IOM_INC_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
