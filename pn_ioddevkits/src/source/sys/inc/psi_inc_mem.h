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
/*  F i l e               &F: psi_inc_mem.h                             :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  The main include file for PSI                                            */
/*                                                                           */
/*  Defines psi alloc local mem                                              */
/*                                                                           */
/*****************************************************************************/

#ifndef PNDV_INC_MEM_H_
#define PNDV_INC_MEM_H_

#include "os.h"


/*--------------------------------------------------------------------------*/
/* alloc/free implementation for TCIP memory and OpenBSD Page memory */
/* allocate from HEAP */

/* PSI_ALLOC_TCIP_MEM_POOL and PSI_FREE_TCIP_MEM_POOL are not implemented.
 * The memory needed by TCIP is reserved at linker step.
 * The related memory region is rxtx_mem[0]. See pnpb_api.c */

#define PSI_ALLOC_TCIP_MEM_POOL(mem_ptr_ptr, length, sys_ptr, comp_id, mem_type)
#define PSI_FREE_TCIP_MEM_POOL(ret_val_ptr, mem_ptr, sys_ptr, comp_id, mem_type)

#define PSI_ALLOC_OBSD_MEM_POOL(mem_ptr_ptr, length, sys_ptr, comp_id, mem_type) \
    PSI_ALLOC_LOCAL_MEM(mem_ptr_ptr, length, sys_ptr, comp_id, mem_type)

#define PSI_FREE_OBSD_MEM_POOL(ret_val_ptr, mem_ptr, sys_ptr, comp_id, mem_type) \
    PSI_FREE_LOCAL_MEM(ret_val_ptr, mem_ptr, sys_ptr, comp_id, mem_type)

#endif
/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/