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
/*  F i l e               &F: pndv_dat.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  internal data                                                            */
/*                                                                           */
/*****************************************************************************/



/*****************************************************************************/
/* contents:

    - internal structures
    - internal data

*/
/*****************************************************************************/
/* reinclude protection */


#ifndef PNDV_DAT_H
#define PNDV_DAT_H


/*****************************************************************************/
/* extern declaration */
#define PNDV_EXTERN_ATTR

#ifndef PNDV_EXTERN_ATTR

    #define PNDV_EXTERN_ATTR extern

#endif


/*****************************************************************************/
/* internal structures */



/*****************************************************************************/
/* internal data */

#include "pndv_uni.h"

PNDV_EXTERN_ATTR  PNDV_DATA_T                     pndv_data;

PNDV_EXTERN_ATTR  PNDV_GLOB_DATA_T                pndv_glob_data;

PNDV_EXTERN_ATTR  LSA_FATAL_ERROR_TYPE            pndv_errcb;


PNDV_EXTERN_ATTR const PNIO_UINT8 pndv_deb_str_not_supp[1];

/*****************************************************************************/
/* reinclude-protection */


#else
    #pragma message ("The header PNDV_DAT.H is included twice or more !")
#endif


/*****************************************************************************/
/*  end of file.                                                             */
/*****************************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
