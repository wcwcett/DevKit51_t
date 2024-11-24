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
/*  F i l e               &F: usriod_dbai.h                             :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  configuration file for the user application, only needed for the         */
/*  DBAI example when #define  EXAMPL_DEV_CONFIG_VERSION is set to value  11 */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
#ifndef _USRIOD_DBAI_H
#define _USRIOD_DBAI_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

typedef struct
{
    PNIO_IO_PROP_ENUM   IoProp;
    PNIO_UINT32 Api;
    PNIO_UINT32 Slot;
    PNIO_UINT32 Sub;
    PNIO_UINT32 InDatOffs;
    PNIO_UINT32 OutDatOffs;
    PNIO_UINT32 InDatLen;
    PNIO_UINT32 OutDatLen;
    PNIO_UINT32 InIopsOffs;
    PNIO_UINT32 InIocsOffs;
    PNIO_UINT32 OutIopsOffs;
    PNIO_UINT32 OutIocsOffs;
} USRIOD_SUBMOD;


typedef struct
{
    PNIO_UINT32             IocrValid;
    PNIO_BUFFER_LOCK_TYPE	InCr;
    PNIO_BUFFER_LOCK_TYPE	OutCr;
    PNIO_UINT32             NumOfSub;
    USRIOD_SUBMOD*          pSubList;            
} USRIOD_DBAI_IOCR;

PNIO_UINT8     IsPlugged[NUMOF_SLOTS][NUMOF_SUBSLOTS + 1];
#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

      
/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
