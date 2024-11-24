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
/*  F i l e               &F: interface.h                               :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  communication interface between periphery and application                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
*
*   H i s t o r y :
*   see /_doc/im151_3pn_hist.doc
*
* ****************************************************************************/


/*****************************************************************************/
/* contents:

    - COMMON

        - interface defines

*/
/*****************************************************************************/
/* reinclude protection */


#ifndef INTERFACE_H
#define INTERFACE_H

#ifndef INTERFACE_EXT_ATTR
    #define INTERFACE_EXT_ATTR extern
#else
    #define INTERFACE_EXT_ATTR_DECLARE
#endif



/*****************************************************************************/
/*****************************************************************************/
/*******************************                 *****************************/
/*******************************     COMMON      *****************************/
/*******************************                 *****************************/
/*****************************************************************************/
/*****************************************************************************/


/*****************************************************************************/
/*     interface between pnio and peripheral controller                      */
/*****************************************************************************/

#define interface_convert_ptr_from_host_to_slave__(_host_adr, _slave_adr, _cast)   \
{                                                                                  \
    (_slave_adr) = (_cast)((PNIO_UINT32)(_host_adr));                              \
}                                                                                  \

#define interface_convert_ptr_from_slave_to_host__(_host_adr, _slave_adr, _cast)   \
{                                                                                  \
    (_host_adr) = (_cast)((PNIO_UINT32)(_slave_adr));                              \
}                                                                                  \


/*****************************************************************************/
/* interface between peri and pndv */


typedef struct
{

    PNDV_IFACE_STRUCT           pndv;

}   GLOB_COUPLING_INTERFACE;

INTERFACE_EXT_ATTR volatile GLOB_COUPLING_INTERFACE  glob_coupling_interface;



/*****************************************************************************/
/* reinclude-protection */


#else
    #pragma message ("The header interface.h is included twice or more !")
#endif


/*****************************************************************************/
/*  end of file.                                                             */
/*****************************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
