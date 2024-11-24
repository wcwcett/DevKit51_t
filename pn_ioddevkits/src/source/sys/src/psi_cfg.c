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
/*  F i l e               &F: psi_cfg.c                                 :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Implements system integration of LSA-component PSI.                      */
/*                                                                           */
/*****************************************************************************/

#define PSI_MODULE_ID      32099
#define LTRC_ACT_MODUL_ID  32099

#include "psi_inc.h"
#include "stdio.h"
#include "math.h"
#include "ertec200p_reg.h"
#include "arm926.h"
#include "bspadapt.h"
#include "os_taskprio.h"
#include "psi_helper.h"
#include "firmware_handler.h"

//PSI_FILE_SYSTEM_EXTENSION(PSI_MODULE_ID)

#define PSI_VALID_POOL_HANDLE      ((LSA_UINT32)1)
#define PSI_INVALID_LOCK_HANDLE    0xFFFE

extern LSA_UINT8* pRxTxMem;

static LSA_BOOL   g_isPsiInFatal = LSA_FALSE;
PSI_EDD_SYS_TYPE  g_eddpSysHandle;

/*===========================================================================*/
/*                 Timer                                                     */
/*===========================================================================*/
#ifndef PSI_ALLOC_TIMER_TGROUP0
LSA_VOID PSI_ALLOC_TIMER_TGROUP0(
    LSA_UINT16               * ret_val_ptr,
    LSA_TIMER_ID_TYPE        * timer_id_ptr,
    LSA_UINT16                 timer_type,
    LSA_UINT16                 time_base,
    PSI_TIMEOUT_CBF_PTR_TYPE   psi_timeout )
{
    PSI_ASSERT(time_base);
    PSI_ASSERT(timer_type);
    *ret_val_ptr = (LSA_UINT16)OsAllocTimer(timer_id_ptr, timer_type, time_base, psi_timeout);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_ALLOC_TIMER_TGROUP1
LSA_VOID PSI_ALLOC_TIMER_TGROUP1(
    LSA_UINT16               * ret_val_ptr,
    LSA_TIMER_ID_TYPE        * timer_id_ptr,
    LSA_UINT16                 timer_type,
    LSA_UINT16                 time_base,
    PSI_TIMEOUT_CBF_PTR_TYPE   psi_timeout )
{
    PSI_ASSERT(time_base);
    PSI_ASSERT(timer_type);
    *ret_val_ptr = (LSA_UINT16)OsAllocTimer(timer_id_ptr, timer_type, time_base, psi_timeout);
}
#else
#error "by design a function!"
#endif


#ifndef PSI_START_TIMER
LSA_VOID PSI_START_TIMER(
    LSA_UINT16        * ret_val_ptr,
    LSA_TIMER_ID_TYPE   timer_id,
    LSA_USER_ID_TYPE    user_id,
    LSA_UINT16          timeVal )
{
    *ret_val_ptr = (LSA_UINT16)OsStartTimer(timer_id, user_id.uvar32, timeVal);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_STOP_TIMER
LSA_VOID PSI_STOP_TIMER(
    LSA_UINT16        * ret_val_ptr,
    LSA_TIMER_ID_TYPE   timer_id )
{
    *ret_val_ptr = (LSA_UINT16) OsStopTimer(timer_id);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_FREE_TIMER
LSA_VOID PSI_FREE_TIMER(
    LSA_UINT16        * ret_val_ptr,
    LSA_TIMER_ID_TYPE   timer_id )
{
    *ret_val_ptr = (PNIO_UINT16)OsFreeTimer(timer_id);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/
/*  Ticks count                                                               */
/*----------------------------------------------------------------------------*/

#ifndef PSI_GET_TICKS_1MS
LSA_UINT32 PSI_GET_TICKS_1MS( LSA_VOID ) 
{
    return OsGetTime_ms();
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_GET_TICKS_100NS
LSA_UINT64 PSI_GET_TICKS_100NS( LSA_VOID )
{
    return OsGetUnixTime();
}
#else
#error "by design a function!"
#endif

/*===========================================================================*/
/*                 Reentrance Locks                                          */
/*===========================================================================*/

#ifndef PSI_ALLOC_REENTRANCE_LOCK
LSA_VOID PSI_ALLOC_REENTRANCE_LOCK(
    LSA_UINT16 * ret_val_ptr,
    LSA_UINT16 * lock_handle_ptr )
{
    *ret_val_ptr = (PNIO_UINT16)OsAllocReentranceX(lock_handle_ptr);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_FREE_REENTRANCE_LOCK
LSA_VOID PSI_FREE_REENTRANCE_LOCK(
     LSA_UINT16 * ret_val_ptr,
     LSA_UINT16   lock_handle )
{
    if (lock_handle < OS_DEVKITS_MUTEX_MAX_COUNT)
    {
        *ret_val_ptr = LSA_RET_ERR_PARAM;
        return; 
    }

    if (lock_handle != PSI_INVALID_LOCK_HANDLE)
    {
        OsFreeReentranceX(lock_handle);
    }

    *ret_val_ptr = LSA_RET_OK;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_ENTER_REENTRANCE_LOCK
LSA_VOID PSI_ENTER_REENTRANCE_LOCK( LSA_UINT16 lock_handle )
{
    PSI_ASSERT(lock_handle != PSI_INVALID_LOCK_HANDLE);
    OsEnterX(lock_handle);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_EXIT_REENTRANCE_LOCK
LSA_VOID PSI_EXIT_REENTRANCE_LOCK( LSA_UINT16 lock_handle )
{
    PSI_ASSERT(lock_handle != PSI_INVALID_LOCK_HANDLE);
    OsExitX(lock_handle);
}
#else
#error "by design a function!"
#endif

/*===========================================================================*/
/*                     IR Locks                                              */
/*===========================================================================*/
/* Note: no IR lock avaliable on WIN, using reentrance                       */

#ifndef PSI_ALLOC_IR_LOCK
LSA_VOID PSI_ALLOC_IR_LOCK(
    LSA_UINT16 * ret_val_ptr,
    LSA_UINT16 * lock_handle_ptr )
{
    *ret_val_ptr = LSA_RET_OK;
    LSA_UNUSED_ARG(lock_handle_ptr);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_FREE_IR_LOCK
LSA_VOID PSI_FREE_IR_LOCK(
    LSA_UINT16 * ret_val_ptr,
    LSA_UINT16   lock_handle )
{
    *ret_val_ptr = LSA_RET_OK;
    LSA_UNUSED_ARG(lock_handle);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_ENTER_IR_LOCK
LSA_VOID PSI_ENTER_IR_LOCK( LSA_UINT16 lock_handle )
{
    LSA_UNUSED_ARG(lock_handle);
    OsIntDisable();
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_EXIT_IR_LOCK
LSA_VOID PSI_EXIT_IR_LOCK( LSA_UINT16 lock_handle )
{
    LSA_UNUSED_ARG(lock_handle);
    OsIntEnable();
}
#else
#error "by design a function!"
#endif

/*===========================================================================*/
/* Wait events                                                             */
/*===========================================================================*/

#ifndef PSI_ALLOC_EVENT
LSA_VOID PSI_ALLOC_EVENT(
    LSA_UINT16  * ret_val_ptr,
    LSA_UINT16  * event_handle_ptr )
{
    PNIO_UINT32* pBlcWait = (PNIO_UINT32*)event_handle_ptr;
    PSI_ASSERT(pBlcWait);
    *ret_val_ptr = (LSA_UINT16)OsAllocSemB(pBlcWait);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_FREE_EVENT
LSA_VOID PSI_FREE_EVENT(
    LSA_UINT16  * ret_val_ptr,
    LSA_UINT16    event_handle )
{
    PNIO_UINT32 p_bloc_wait_id = (PNIO_UINT32)event_handle;
    *ret_val_ptr = (LSA_UINT16)OsFreeSemB(p_bloc_wait_id);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_SET_EVENT
LSA_VOID PSI_SET_EVENT(
    LSA_UINT16 event_handle )
{
    PNIO_UINT32 retVal = 0;
    PNIO_UINT32 p_bloc_wait_id = (PNIO_UINT32)event_handle;
    PSI_ASSERT(p_bloc_wait_id < MAXNUM_OF_BIN_SEMAPH);

    retVal = OsGiveSemB(p_bloc_wait_id);
    if (retVal != PNIO_OK)
    {
        PSI_SYSTEM_TRACE_01(0, LSA_TRACE_LEVEL_ERROR, "PSI_SET_EVENT: OsGiveSemB() failed with code %d", retVal);
        PSI_FATAL(0);
    }
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_WAIT_EVENT
LSA_VOID PSI_WAIT_EVENT(
    LSA_UINT16        * ret_val_ptr,
    LSA_UINT16          event_handle,
    LSA_UINT32          const uTimeOutMs )
{
    PNIO_UINT32 retVal = 0;
    PNIO_UINT32 p_bloc_wait_id = (PNIO_UINT32)event_handle;
    PSI_ASSERT(p_bloc_wait_id < MAXNUM_OF_BIN_SEMAPH);
    *ret_val_ptr = LSA_RET_OK;

    retVal = OsTakeSemB(p_bloc_wait_id);
    if (retVal != PNIO_OK)
    {
        PSI_SYSTEM_TRACE_01(0, LSA_TRACE_LEVEL_ERROR, "PSI_WAIT_EVENT: OsTakeSemB() failed with code %d", retVal);
        *ret_val_ptr = LSA_RET_ERR_RESOURCE;
        PSI_FATAL(0);
    }
    LSA_UNUSED_ARG(uTimeOutMs);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_WAIT_NS
/*
    Ertec runs at 250MHZ (1 cycle = 4 ns)
    - Calculations made based on gcc 8.3.1
    - Optimization level : -O2
    - Instruction set    : armv5
*/
LSA_VOID PSI_WAIT_NS(
    LSA_UINT64 uTimeNs )
{
    if (uTimeNs <= 35)      // 3 cycles
    {
        return;               //+2 cycle
    }

    if (uTimeNs <= 148)      // 3 cycles
    {
        asm volatile ("nop");  //+1 cycle
        return;                //+5 cycle
    }

    // 36  ns:  2 if checks made above(+6) + push(+3)             => 9(cycles) * 4(ns)
    // 112 ns:  sub(+3) + mov(+12) + subc(+3) + division(+~10)    => 28(cycles) * 4(ns)
    // 28  ns:  subs(+2) + sbc(+2) + orrs(+2) + nop(+1)           => 7(cycles) * 4(ns)
    uTimeNs = (uTimeNs - 36/*operations until here*/ - 112/*this operation itself*/) / 28/*each itearation of following for loop*/;

    for (; uTimeNs > 0; uTimeNs--) // initial check : 3 cycle
    {
        asm volatile ("nop");       // 7 cycle in each loop
    }                              // if not entered in for +3 cycle (it is not included in calculation)
}
#else
#error "by design a function!"
#endif

/*===========================================================================*/
/* Cache                                                                      */
/*===========================================================================*/

#if ( PSI_CFG_USE_NRT_CACHE_SYNC == 1 )

#ifndef PSI_CACHE_SYNC
LSA_VOID PSI_CACHE_SYNC(
    LSA_VOID_PTR_TYPE   basePtr,
    LSA_UINT32          length )
{
}
#else
#error "by design a function!"
#endif

#ifndef PSI_CACHE_INV
LSA_VOID PSI_CACHE_INV(
    LSA_VOID_PTR_TYPE   basePtr,
    LSA_UINT32          length )
{
}
#else
#error "by design a function!"
#endif

#ifndef PSI_CACHE_WB
LSA_VOID PSI_CACHE_WB(
    LSA_VOID_PTR_TYPE   basePtr,
    LSA_UINT32          length )
{
}
#else
#error "by design a function!"
#endif

#ifndef PSI_CACHE_WB_INV
LSA_VOID PSI_CACHE_WB_INV(
    LSA_VOID_PTR_TYPE   basePtr,
    LSA_UINT32          length )
{
}
#else
#error "by design a function!"
#endif

#endif // PSI_CFG_USE_NRT_CACHE_SYNC

/*===========================================================================*/
/*                         Local memory                                      */
/*===========================================================================*/
#ifndef PSI_ALLOC_LOCAL_MEM
LSA_VOID PSI_ALLOC_LOCAL_MEM(
    LSA_VOID_PTR_TYPE * mem_ptr_ptr,
    LSA_UINT32          length,
    LSA_SYS_PTR_TYPE    sys_ptr,
    LSA_UINT16          comp_id,
    LSA_UINT16          mem_type )
{
    LSA_UNUSED_ARG(sys_ptr);
    LSA_UNUSED_ARG(comp_id);
    LSA_UNUSED_ARG(mem_type);

    OsAllocX(mem_ptr_ptr, 0, length, MEMPOOL_DEFAULT);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_FREE_LOCAL_MEM
LSA_VOID PSI_FREE_LOCAL_MEM(
    LSA_UINT16        * ret_val_ptr,
    LSA_VOID_PTR_TYPE   mem_ptr,
    LSA_SYS_PTR_TYPE    sys_ptr,
    LSA_UINT16          comp_id,
    LSA_UINT16          mem_type )
{
     *(ret_val_ptr) = (LSA_UINT16)OsFreeX(mem_ptr, MEMPOOL_DEFAULT);
}
#else
#error "by design a function!"
#endif

/*===========================================================================*/
/*                     HIF memory pool                                       */
/*===========================================================================*/

#ifndef PSI_CREATE_HIF_MEM_POOL
LSA_VOID PSI_CREATE_HIF_MEM_POOL(
    LSA_VOID_PTR_TYPE   basePtr,
    LSA_UINT32          length,
    PSI_SYS_HANDLE      sys_handle,
    LSA_UINT32        * pool_handle_ptr )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_DELETE_HIF_MEM_POOL
LSA_VOID PSI_DELETE_HIF_MEM_POOL(
    LSA_UINT16     * ret_val_ptr,
    PSI_SYS_HANDLE   sys_handle,
    LSA_UINT32       pool_handle )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_HIF_ALLOC_MEM
LSA_VOID PSI_HIF_ALLOC_MEM(
    LSA_VOID_PTR_TYPE * mem_ptr_ptr,
    LSA_UINT32          length,
    PSI_SYS_HANDLE      sys_handle,
    LSA_UINT32          pool_handle )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_HIF_FREE_MEM
LSA_VOID PSI_HIF_FREE_MEM(
    LSA_UINT16        * ret_val_ptr,
    LSA_VOID_PTR_TYPE   mem_ptr,
    PSI_SYS_HANDLE      sys_handle,
    LSA_UINT32          pool_handle )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_HIF_SHM_DMA_COPY
LSA_VOID PSI_HIF_SHM_DMA_COPY(
    LSA_VOID       * * ptr_ptr_rqb,
    LSA_UINT32         rqb_len,
    PSI_SYS_HANDLE     sys_handle )
{
}
#else
#error "by design a function!"
#endif

/*===========================================================================*/
/*                     DEV memory pool                                       */
/*===========================================================================*/
#if ((PSI_CFG_USE_EDDP == 1) || (PSI_CFG_USE_EDDS == 1) || (PSI_CFG_USE_EDDT == 1))
#ifndef PSI_CREATE_DEV_MEM_POOL
LSA_VOID PSI_CREATE_DEV_MEM_POOL(
    LSA_VOID_PTR_TYPE   basePtr,
    LSA_UINT32          length,
    PSI_SYS_HANDLE      sys_handle,
    LSA_UINT32        * pool_handle_ptr )
{
    // Create Pool structure is not implemented, a general pool which is implemented at PNIO_device_open() is used for all pools.
    LSA_UNUSED_ARG(basePtr);
    LSA_UNUSED_ARG(length);
    LSA_UNUSED_ARG(sys_handle);
    *pool_handle_ptr = PSI_VALID_POOL_HANDLE;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_DELETE_DEV_MEM_POOL
LSA_VOID PSI_DELETE_DEV_MEM_POOL(
    LSA_UINT16     * ret_val_ptr,
    PSI_SYS_HANDLE   sys_handle,
    LSA_UINT32       pool_handle )
{
    LSA_UNUSED_ARG(pool_handle);
    LSA_UNUSED_ARG(sys_handle);
    *ret_val_ptr = LSA_RET_OK;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_DEV_ALLOC_MEM
LSA_VOID PSI_DEV_ALLOC_MEM(
    LSA_VOID_PTR_TYPE * mem_ptr_ptr,
    LSA_UINT32          length,
    LSA_UINT32          pool_handle,
    LSA_UINT16          comp_id )
{
    XX_MEM_DWORD retVal = 0;
    LSA_UNUSED_ARG(comp_id);
    LSA_UNUSED_ARG(pool_handle);

    retVal = XX_MEM_alloc(mem_ptr_ptr, length, pRxTxMem);
    if (retVal != XX_MEM_RET_OK)
    {
        PSI_SYSTEM_TRACE_01(0, LSA_TRACE_LEVEL_ERROR, "PSI_DEV_ALLOC_MEM: XX_MEM_alloc() failed with code %d", retVal);
    }
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_DEV_FREE_MEM
LSA_VOID PSI_DEV_FREE_MEM(
    LSA_UINT16        * ret_val_ptr,
    LSA_VOID_PTR_TYPE   mem_ptr,
    LSA_UINT32          pool_handle,
    LSA_UINT16          comp_id )
{
    XX_MEM_DWORD retVal = 0;
    LSA_UNUSED_ARG(comp_id);
    LSA_UNUSED_ARG(pool_handle);
    *ret_val_ptr = LSA_RET_OK;

    retVal = XX_MEM_free(mem_ptr);
    if (retVal != XX_MEM_RET_OK)
    {
        PSI_SYSTEM_TRACE_01(0, LSA_TRACE_LEVEL_ERROR, "PSI_DEV_FREE_MEM: XX_MEM_free() failed with code %d", retVal);
        *ret_val_ptr = LSA_RET_ERR_RESOURCE;
    }
}
#else
#error "by design a function!"
#endif
#endif // ((PSI_CFG_USE_EDDP == 1) || (PSI_CFG_USE_EDDS == 1) || (PSI_CFG_USE_EDDT == 1))
/*===========================================================================*/
/*                     NRT memory pool                                       */
/*===========================================================================*/

#ifndef PSI_CREATE_NRT_MEM_POOL
LSA_VOID PSI_CREATE_NRT_MEM_POOL(
    LSA_VOID_PTR_TYPE   basePtr,
    LSA_UINT32          length,
    PSI_SYS_HANDLE      sys_handle,
    LSA_UINT32        * pool_handle_ptr,
    LSA_UINT8           cp_mem_nrt_type )
{
    // Create Pool structure is not implemented, a general pool which is implemented at PNIO_device_open() is used for all pools.
    LSA_UNUSED_ARG(basePtr);
    LSA_UNUSED_ARG(length);
    LSA_UNUSED_ARG(sys_handle);
    LSA_UNUSED_ARG(cp_mem_nrt_type);
    *pool_handle_ptr = PSI_VALID_POOL_HANDLE;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_DELETE_NRT_MEM_POOL
LSA_VOID PSI_DELETE_NRT_MEM_POOL(
    LSA_UINT16     * ret_val_ptr,
    PSI_SYS_HANDLE   sys_handle,
    LSA_UINT32       pool_handle,
    LSA_UINT8        cp_mem_nrt_type )
{
    LSA_UNUSED_ARG(pool_handle);
    LSA_UNUSED_ARG(sys_handle);
    LSA_UNUSED_ARG(cp_mem_nrt_type);
    *ret_val_ptr = LSA_RET_OK;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_NRT_ALLOC_TX_MEM
LSA_VOID PSI_NRT_ALLOC_TX_MEM(
    LSA_VOID_PTR_TYPE * mem_ptr_ptr,
    LSA_UINT32          length,
    LSA_UINT32          pool_handle,
    LSA_UINT16          comp_id )
{
    XX_MEM_DWORD retVal = 0;
    LSA_UNUSED_ARG(comp_id);
    LSA_UNUSED_ARG(pool_handle);
    
    retVal = XX_MEM_alloc(mem_ptr_ptr, length, pRxTxMem);
    if (retVal != XX_MEM_RET_OK)
    {
        PSI_SYSTEM_TRACE_01(0, LSA_TRACE_LEVEL_ERROR, "PSI_NRT_ALLOC_TX_MEM: XX_MEM_alloc() failed with code %d", retVal);
    }
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_NRT_FREE_TX_MEM
LSA_VOID PSI_NRT_FREE_TX_MEM(
    LSA_UINT16        * ret_val_ptr,
    LSA_VOID_PTR_TYPE   mem_ptr,
    LSA_UINT32          pool_handle,
    LSA_UINT16          comp_id )
{
    XX_MEM_DWORD retVal = 0;
    LSA_UNUSED_ARG(comp_id);
    LSA_UNUSED_ARG(pool_handle);
    *ret_val_ptr = LSA_RET_OK;

    retVal = XX_MEM_free(mem_ptr);
    if (retVal != XX_MEM_RET_OK)
    {
        PSI_SYSTEM_TRACE_01(0, LSA_TRACE_LEVEL_ERROR, "PSI_NRT_FREE_TX_MEM: XX_MEM_free() failed with code %d", retVal);
        *ret_val_ptr = LSA_RET_ERR_RESOURCE;
    }    
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_NRT_ALLOC_RX_MEM
LSA_VOID PSI_NRT_ALLOC_RX_MEM(
    LSA_VOID_PTR_TYPE * mem_ptr_ptr,
    LSA_UINT32          length,
    LSA_UINT32          pool_handle,
    LSA_UINT16          comp_id )
{
    XX_MEM_DWORD retVal = 0;
    LSA_UNUSED_ARG(comp_id);
    LSA_UNUSED_ARG(pool_handle);

    retVal = XX_MEM_alloc(mem_ptr_ptr, length, pRxTxMem);
    if (retVal != XX_MEM_RET_OK)
    {
        PSI_SYSTEM_TRACE_01(0, LSA_TRACE_LEVEL_ERROR, "PSI_NRT_ALLOC_RX_MEM: XX_MEM_alloc() failed with code %d", retVal);
    }
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_NRT_FREE_RX_MEM
LSA_VOID PSI_NRT_FREE_RX_MEM(
    LSA_UINT16        * ret_val_ptr,
    LSA_VOID_PTR_TYPE   mem_ptr,
    LSA_UINT32          pool_handle,
    LSA_UINT16          comp_id )
{
    XX_MEM_DWORD retVal = 0;
    LSA_UNUSED_ARG(comp_id);
    LSA_UNUSED_ARG(pool_handle);
    *ret_val_ptr = LSA_RET_OK;

    retVal = XX_MEM_free(mem_ptr);
    if (retVal != XX_MEM_RET_OK)
    {
        PSI_SYSTEM_TRACE_01(0, LSA_TRACE_LEVEL_ERROR, "PSI_NRT_FREE_RX_MEM: XX_MEM_free() failed with code %d", retVal);
        *ret_val_ptr = LSA_RET_ERR_RESOURCE;
    }
}
#else
#error "by design a function!"
#endif

/*===========================================================================*/
/*                     PI memory pool                                        */
/*===========================================================================*/

#ifndef PSI_CREATE_PI_MEM_POOL
/** Systemadaption for creating mempools
 * 
 * Depending on the hardware device type and the hd_nr a special
 * memset function is set to p_memset_fct pointer. With this information
 * the mempool is created by calling eps_cp_mem_create_pi_pool.
 *  
 * @param basePtr Base address of memory block
 * @param length Size of memory block [bytes]
 * @param sys_handle EPS system information (EPS_SYS_PTR_TYPE)
 * @param pool_handle_ptr handle to adress allocated pool
 */
LSA_VOID PSI_CREATE_PI_MEM_POOL(
    LSA_VOID_PTR_TYPE   basePtr,
    LSA_UINT32          length,
    PSI_SYS_HANDLE      sys_handle,
    LSA_UINT32        * pool_handle_ptr )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_DELETE_PI_MEM_POOL
LSA_VOID PSI_DELETE_PI_MEM_POOL(
    LSA_UINT16     * ret_val_ptr,
    PSI_SYS_HANDLE   sys_handle,
    LSA_UINT32       pool_handle )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_PI_ALLOC_MEM
LSA_VOID PSI_PI_ALLOC_MEM(
    LSA_VOID_PTR_TYPE * mem_ptr_ptr,
    LSA_UINT32          length,
    LSA_UINT32          pool_handle,
    LSA_BOOL            is_provider )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_PI_FREE_MEM
LSA_VOID PSI_PI_FREE_MEM(
    LSA_UINT16        * ret_val_ptr,
    LSA_VOID_PTR_TYPE   mem_ptr,
    LSA_UINT32          pool_handle,
    LSA_BOOL            is_provider)
{
}
#else
#error "by design a function!"
#endif

/*===========================================================================*/
/* TCIP memory pool                                                          */
/*===========================================================================*/

#if ( PSI_CFG_USE_TCIP == 1 )

#ifndef PSI_CREATE_TCIP_MEM_POOL
LSA_VOID PSI_CREATE_TCIP_MEM_POOL(
    LSA_UINT16        * ret_val_ptr,
    LSA_VOID_PTR_TYPE   basePtr,
    LSA_UINT32          length )
{
    // Create Pool structure is not implemented, a general pool which is implemented at PNIO_device_open() is used for all pools.
    LSA_UNUSED_ARG(basePtr);
    LSA_UNUSED_ARG(length);
    *ret_val_ptr = LSA_RET_OK;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_DELETE_TCIP_MEM_POOL
LSA_VOID PSI_DELETE_TCIP_MEM_POOL(
    LSA_UINT16 * ret_val_ptr )
{
    *ret_val_ptr = LSA_RET_OK;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_TCIP_ALLOC_MEM
LSA_VOID PSI_TCIP_ALLOC_MEM(
    LSA_VOID_PTR_TYPE * mem_ptr_ptr,
    LSA_UINT32          length )
{
    XX_MEM_DWORD retVal = 0;

    retVal = XX_MEM_alloc(mem_ptr_ptr, length, pRxTxMem);
    if (retVal != XX_MEM_RET_OK)
    {
        PSI_SYSTEM_TRACE_01(0, LSA_TRACE_LEVEL_ERROR, "PSI_TCIP_ALLOC_MEM: XX_MEM_alloc() failed with code %d", retVal);
    }
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_TCIP_FREE_MEM
LSA_VOID PSI_TCIP_FREE_MEM(
    LSA_UINT16        * ret_val_ptr,
    LSA_VOID_PTR_TYPE   mem_ptr )
{
    XX_MEM_DWORD retVal = 0;
    *ret_val_ptr = LSA_RET_OK;

    retVal = XX_MEM_free(mem_ptr);
    if (retVal != XX_MEM_RET_OK)
    {
        PSI_SYSTEM_TRACE_01(0, LSA_TRACE_LEVEL_ERROR, "PSI_TCIP_FREE_MEM: XX_MEM_free() failed with code %d", retVal);
        *ret_val_ptr = LSA_RET_ERR_RESOURCE;
    }
}
#else
#error "by design a function!"
#endif

#endif // PSI_CFG_USE_TCIP

/*===========================================================================*/
/* OpenBSD Page memory pool                                                  */
/*===========================================================================*/

#if (PSI_CFG_TCIP_STACK_OPEN_BSD == 1)

LSA_VOID_PTR_TYPE open_bsd_pool_memory_ptr    = 0;
LSA_UINT32        open_bsd_pool_memory_length = 0;
LSA_UINT32        open_bsd_pool_memory_used_pages[PSI_CFG_TCIP_CFG_OBSD_USED_PAGE_ARR_SIZE];
LSA_VOID_PTR_TYPE open_bsd_pool_allocated_ptrs[PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_PAGES];
LSA_UINT32        open_bsd_pool_allocated_lengths[PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_PAGES];

#ifndef PSI_CREATE_OBSD_MEM_POOL
LSA_VOID PSI_CREATE_OBSD_MEM_POOL(
    LSA_UINT16        * ret_val_ptr,
    LSA_VOID_PTR_TYPE   basePtr,
    LSA_UINT32          length )
{
    PSI_ASSERT(ret_val_ptr != LSA_NULL);

    if (length > PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_PAGES * PSI_CFG_TCIP_CFG_OBSD_PAGE_SIZE)
    {
        psi_fatal_error(LSA_COMP_ID_PSI, PSI_MODULE_ID, (LSA_UINT16)__LINE__, 0, 0, 0, 0, 0, LSA_NULL);
    }

    /* ensure 4kB alignment for pages */
    open_bsd_pool_memory_ptr    = (LSA_VOID_PTR_TYPE)(((LSA_UINT32) basePtr + (PSI_CFG_TCIP_CFG_OBSD_PAGE_SIZE-1)) & ~(PSI_CFG_TCIP_CFG_OBSD_PAGE_SIZE-1));
    open_bsd_pool_memory_length = length - (basePtr - open_bsd_pool_memory_ptr);
    PSI_MEMSET(open_bsd_pool_memory_used_pages, 0, sizeof(open_bsd_pool_memory_used_pages));
    PSI_MEMSET(open_bsd_pool_allocated_ptrs,    0, sizeof(open_bsd_pool_allocated_ptrs));
    PSI_MEMSET(open_bsd_pool_allocated_lengths, 0, sizeof(open_bsd_pool_allocated_lengths));

    *ret_val_ptr = LSA_RET_OK;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_DELETE_OBSD_MEM_POOL
LSA_VOID PSI_DELETE_OBSD_MEM_POOL(
    LSA_UINT16 * ret_val_ptr )
{
    PSI_ASSERT(ret_val_ptr != LSA_NULL);

    open_bsd_pool_memory_ptr    = 0;
    open_bsd_pool_memory_length = 0;
    PSI_MEMSET(open_bsd_pool_memory_used_pages, 0, sizeof(open_bsd_pool_memory_used_pages));
    PSI_MEMSET(open_bsd_pool_allocated_ptrs,    0, sizeof(open_bsd_pool_allocated_ptrs));
    PSI_MEMSET(open_bsd_pool_allocated_lengths, 0, sizeof(open_bsd_pool_allocated_lengths));

    *ret_val_ptr = LSA_RET_OK;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_OBSD_ALLOC_MEM
LSA_VOID PSI_OBSD_ALLOC_MEM(
    LSA_VOID_PTR_TYPE * mem_ptr_ptr,
    LSA_UINT32          length )
{
    LSA_UINT32 page_index_begin     = 0;
    LSA_UINT32 connected_mem_length = 0;
    LSA_BOOL   mem_area_found       = LSA_FALSE;

    PSI_ASSERT(mem_ptr_ptr != LSA_NULL);
    PSI_ASSERT(open_bsd_pool_memory_ptr != LSA_NULL);

    *mem_ptr_ptr = 0;

    /* search a block of connected 4kB pages that's large enough for length */
    for (page_index_begin = 0; page_index_begin < PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_PAGES; page_index_begin++)
    {
        LSA_UINT32 cur_page_index = page_index_begin;

        /* search unused bits on the related page */
        while ((open_bsd_pool_memory_used_pages[cur_page_index / PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_BITS]
              & (1 << (cur_page_index % PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_BITS))) == 0)
        {
            /* page still unused ... */
            connected_mem_length += PSI_CFG_TCIP_CFG_OBSD_PAGE_SIZE;
            if (connected_mem_length >= length)
            {
                mem_area_found = LSA_TRUE;
                break;
            }
            cur_page_index++;
            if (!(cur_page_index < PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_PAGES))
            {
                break;
            }
        }

        if (mem_area_found == LSA_TRUE)
        {
            break;
        }
    }

    if (mem_area_found)
    {
        LSA_UINT32 cur_page_index;
        LSA_UINT32 meta_desc_index;

        if (open_bsd_pool_memory_ptr + (page_index_begin * PSI_CFG_TCIP_CFG_OBSD_PAGE_SIZE) + connected_mem_length <= open_bsd_pool_memory_ptr + open_bsd_pool_memory_length)
        {
            /* end of allocated block is still within page memory */

            for (cur_page_index = page_index_begin; cur_page_index < page_index_begin + connected_mem_length/PSI_CFG_TCIP_CFG_OBSD_PAGE_SIZE; cur_page_index++)
            {
                /* replaces the bits that have been used */
                if (!(cur_page_index < PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_PAGES))
                {
                    break;
                }
                open_bsd_pool_memory_used_pages[cur_page_index/PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_BITS] |= (1 << (cur_page_index%PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_BITS));
            }

            for (meta_desc_index = 0; meta_desc_index < PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_PAGES; meta_desc_index++)
            {
                if (open_bsd_pool_allocated_ptrs[meta_desc_index] == 0)
                {
                    /* free meta descriptor found -> fill it with info */
                    *mem_ptr_ptr = open_bsd_pool_memory_ptr + (page_index_begin * PSI_CFG_TCIP_CFG_OBSD_PAGE_SIZE);

                    open_bsd_pool_allocated_ptrs[meta_desc_index]    = *mem_ptr_ptr;
                    open_bsd_pool_allocated_lengths[meta_desc_index] = connected_mem_length;
                    break;
                }
            }

            if (*mem_ptr_ptr == 0)
            {
                psi_fatal_error(LSA_COMP_ID_PSI, PSI_MODULE_ID, (LSA_UINT16)__LINE__, length, 0, 0, 0, 0, LSA_NULL);
            }
        }
        else
        {
            psi_fatal_error(LSA_COMP_ID_PSI, PSI_MODULE_ID, (LSA_UINT16)__LINE__, length, 0, 0, 0, 0, LSA_NULL);
        }
    }
    else
    {
        psi_fatal_error(LSA_COMP_ID_PSI, PSI_MODULE_ID, (LSA_UINT16)__LINE__, length, 0, 0, 0, 0, LSA_NULL);
    }

}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_OBSD_FREE_MEM
LSA_VOID PSI_OBSD_FREE_MEM(
    LSA_UINT16        * ret_val_ptr,
    LSA_VOID_PTR_TYPE   mem_ptr )
{
    PSI_ASSERT(ret_val_ptr != LSA_NULL);
    PSI_ASSERT(mem_ptr != LSA_NULL);

    *ret_val_ptr = LSA_RET_ERR_RESOURCE;

    if(  (mem_ptr >= open_bsd_pool_memory_ptr)
      && ((LSA_UINT32) mem_ptr < ((LSA_UINT32)open_bsd_pool_memory_ptr + PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_PAGES * PSI_CFG_TCIP_CFG_OBSD_PAGE_SIZE))
      && ((LSA_UINT32) mem_ptr & (PSI_CFG_TCIP_CFG_OBSD_PAGE_SIZE - 1)) == 0)
    {
        LSA_UINT32 cur_page_index;
        LSA_UINT32 end_page_index;
        LSA_UINT32 meta_desc_index;
        LSA_UINT32 length          = 0;
        LSA_BOOL   meta_desc_found = LSA_FALSE;

        for (meta_desc_index = 0; meta_desc_index < PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_PAGES; meta_desc_index++)
        {
            if (open_bsd_pool_allocated_ptrs[meta_desc_index] == mem_ptr)
            {
                meta_desc_found = LSA_TRUE;
                length          = open_bsd_pool_allocated_lengths[meta_desc_index];

                open_bsd_pool_allocated_ptrs[meta_desc_index]    = 0;
                open_bsd_pool_allocated_lengths[meta_desc_index] = 0;
                break;
            }
        }

        if (meta_desc_found == LSA_FALSE)
        {
            psi_fatal_error(LSA_COMP_ID_PSI, PSI_MODULE_ID, (LSA_UINT16)__LINE__, (LSA_UINT32) mem_ptr, 0, 0, 0, 0, LSA_NULL);
        }

        cur_page_index = (LSA_UINT32)((mem_ptr - open_bsd_pool_memory_ptr) / PSI_CFG_TCIP_CFG_OBSD_PAGE_SIZE);
        end_page_index = (LSA_UINT32)((mem_ptr + length - open_bsd_pool_memory_ptr) / PSI_CFG_TCIP_CFG_OBSD_PAGE_SIZE);
        while (cur_page_index < end_page_index)
        {
            open_bsd_pool_memory_used_pages[cur_page_index / PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_BITS] &= ~(1u << (cur_page_index % PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_BITS));
            cur_page_index++;
        }
        *ret_val_ptr = LSA_RET_OK;
    }
    else
    {
        psi_fatal_error(LSA_COMP_ID_PSI, PSI_MODULE_ID, (LSA_UINT16)__LINE__, (LSA_UINT32) mem_ptr, 0, 0, 0, 0, LSA_NULL);
    }
}
#else
#error "by design a function!"
#endif

#endif // PSI_CFG_TCIP_STACK_OPEN_BSD

/*===========================================================================*/
/*                     CRT slow memory pool                                  */
/*===========================================================================*/

#ifndef PSI_CREATE_CRT_SLOW_MEM_POOL
LSA_VOID PSI_CREATE_CRT_SLOW_MEM_POOL(
    LSA_VOID_PTR_TYPE   basePtr,
    LSA_UINT32          length,
    PSI_SYS_HANDLE      sys_handle,
    LSA_UINT32        * pool_handle_ptr,
    LSA_UINT8           cp_mem_crt_slow_type )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_DELETE_CRT_SLOW_MEM_POOL
LSA_VOID PSI_DELETE_CRT_SLOW_MEM_POOL(
    LSA_UINT16     * ret_val_ptr,
    PSI_SYS_HANDLE   sys_handle,
    LSA_UINT32       pool_handle,
    LSA_UINT8        cp_mem_crt_slow_type )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_CRT_SLOW_ALLOC_MEM
LSA_VOID PSI_CRT_SLOW_ALLOC_MEM(
    LSA_VOID_PTR_TYPE * mem_ptr_ptr,
    LSA_UINT32          length,
    LSA_UINT32          pool_handle,
    LSA_UINT16          comp_id )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_CRT_SLOW_ALLOC_TX_MEM
LSA_VOID PSI_CRT_SLOW_ALLOC_TX_MEM(
    LSA_VOID_PTR_TYPE * mem_ptr_ptr,
    LSA_UINT32          length,
    LSA_UINT32          pool_handle,
    LSA_UINT16          comp_id )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_CRT_SLOW_ALLOC_RX_MEM
LSA_VOID PSI_CRT_SLOW_ALLOC_RX_MEM(
    LSA_VOID_PTR_TYPE * mem_ptr_ptr,
    LSA_UINT32          length,
    LSA_UINT32          pool_handle,
    LSA_UINT16          comp_id )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_CRT_SLOW_FREE_MEM
LSA_VOID PSI_CRT_SLOW_FREE_MEM(
    LSA_UINT16        * ret_val_ptr,
    LSA_VOID_PTR_TYPE   mem_ptr,
    LSA_UINT32          pool_handle,
    LSA_UINT16          comp_id )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_CRT_SLOW_FREE_TX_MEM
LSA_VOID PSI_CRT_SLOW_FREE_TX_MEM(
    LSA_UINT16        * ret_val_ptr,
    LSA_VOID_PTR_TYPE   mem_ptr,
    LSA_UINT32          pool_handle,
    LSA_UINT16          comp_id )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_CRT_SLOW_FREE_RX_MEM
LSA_VOID PSI_CRT_SLOW_FREE_RX_MEM(
    LSA_UINT16        * ret_val_ptr,
    LSA_VOID_PTR_TYPE   mem_ptr,
    LSA_UINT32          pool_handle,
    LSA_UINT16          comp_id )
{
}
#else
#error "by design a function!"
#endif

/*===========================================================================*/
/*                    stdlib                                                 */
/*===========================================================================*/

#ifndef PSI_MEMSET
LSA_VOID PSI_MEMSET(
    LSA_VOID_PTR_TYPE ptr,
    LSA_INT     const val,
    LSA_UINT32  const len )
{
    OsMemSet(ptr, val, len);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_MEMCPY
LSA_VOID PSI_MEMCPY(
    LSA_VOID_PTR_TYPE       dst,
    LSA_VOID_CONST_PTR_TYPE src,
    LSA_UINT32 const        len )
{
    OsMemCpy(dst, src, len);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_MEMCMP
LSA_INT PSI_MEMCMP(
    LSA_VOID_CONST_PTR_TYPE pBuf1,
    LSA_VOID_CONST_PTR_TYPE pBuf2,
    LSA_UINT32 const        Length )
{
    return (OsMemCmp(pBuf1, pBuf2, Length));
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_MEMMOVE
LSA_VOID PSI_MEMMOVE(
    LSA_VOID_PTR_TYPE       dst,
    LSA_VOID_CONST_PTR_TYPE src,
    LSA_UINT32 const        len )
{
    OsMemMove(dst, src, len);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_STRCPY
LSA_VOID PSI_STRCPY(
    LSA_VOID_PTR_TYPE dst,
    LSA_VOID_CONST_PTR_TYPE src )
{
    OsStrCpy(dst, src);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_STRLEN
LSA_UINT PSI_STRLEN(
    LSA_VOID_CONST_PTR_TYPE str )
{
    return ((LSA_UINT)OsStrLen(str));
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_SPRINTF
LSA_INT PSI_SPRINTF(
    LSA_UINT8      * str,
    const LSA_CHAR * fmt,
    ... )
{
    LSA_INT ret;
    va_list argptr = {0};

    va_start(argptr, fmt);
    ret = vsprintf((char*)str, fmt, argptr);
    va_end(argptr);

    return ret;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_VSPRINTF
LSA_INT PSI_VSPRINTF(
    LSA_CHAR       * str,
    const LSA_CHAR * fmt,
    LSA_CHAR       * argptr )
{
    LSA_INT ret;
    va_list* ap = (va_list*)argptr;
    
    ret = vsprintf((LSA_CHAR*)str, fmt, *ap);

    return ret;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_VSNPRINTF
LSA_INT PSI_VSNPRINTF(
    LSA_CHAR       * str,
    LSA_UINT32       count,
    const LSA_CHAR * fmt,
    LSA_CHAR       * argptr )
{
    LSA_INT ret;
    va_list* ap = (va_list*)argptr;

    ret = vsnprintf((LSA_CHAR*)str, count, fmt, *ap);

    return ret;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_SSCANF_UUID
LSA_INT PSI_SSCANF_UUID(
    const LSA_UINT8  * uuid_string,
    const LSA_CHAR   * fmt,
    LSA_UINT32 * time_low,
    LSA_INT    * time_mid,
    LSA_INT    * time_hi_and_version,
    LSA_INT    * clock_seq_hi_and_reserved,
    LSA_INT    * clock_seq_low,
    LSA_INT    * node0,
    LSA_INT    * node1,
    LSA_INT    * node2,
    LSA_INT    * node3,
    LSA_INT    * node4,
    LSA_INT    * node5,
    LSA_INT    * read_count )
{
    LSA_INT ret;

    ret = sscanf((const char*)uuid_string, (const char*)fmt,
                  time_low,
                  time_mid,
                  time_hi_and_version,
                  clock_seq_hi_and_reserved,
                  clock_seq_low,
                  node0, node1, node2, node3, node4, node5,
                  read_count);

    return ret;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_LOG_10
LSA_VOID PSI_LOG_10(
    LSA_UINT32 const Arg_in_ppm,
    LSA_INT32      * pResult_in_ppm )
{
    double LogOut;

    LogOut = log10( (double)Arg_in_ppm );

    LogOut *= 1000000.0; /* ppm, parts per million */

    *pResult_in_ppm = (LSA_INT32)LogOut;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_POW_10
LSA_VOID PSI_POW_10(
    LSA_INT16  const numerator,
    LSA_UINT16 const denominator,
    LSA_UINT32     * pResult )
{
    *pResult = (LSA_UINT32)pow(10.0, (double)numerator / (double)denominator);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_EXCHANGE_LONG
PSI_EXCHANGE_TYPE PSI_EXCHANGE_LONG(
    PSI_EXCHANGE_TYPE volatile * long_ptr,
    PSI_EXCHANGE_TYPE            val )
{
    long ret_val;

    PSI_ENTER();

    ret_val = *long_ptr;
    *long_ptr = val;

    PSI_EXIT();

    return ret_val;
}
#else
#error "by design a function!"
#endif

/*===========================================================================*/
/*   FATAL / ERROR                                                           */
/*===========================================================================*/

#ifndef PSI_FATAL_ERROR
LSA_VOID PSI_FATAL_ERROR(
    const LSA_CHAR             *comp,
    LSA_UINT16                 length,
    PSI_FATAL_ERROR_PTR_TYPE   error_ptr )
{
    g_isPsiInFatal = LSA_TRUE;

    LSA_UNUSED_ARG(comp);
    LSA_UNUSED_ARG(length);

    PSI_ASSERT(error_ptr != LSA_NULL);
    error_ptr->lsa_component_id = (PNIO_UINT16)DecodePackId(error_ptr->lsa_component_id);

    PNIO_FatalError(error_ptr);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_RQB_ERROR
LSA_VOID PSI_RQB_ERROR(
    LSA_UINT16 const  comp_id,
    LSA_UINT16 const  comp_id_lower,
    LSA_VOID_PTR_TYPE rqb_ptr )
{

    PSI_SYSTEM_TRACE_05(0, LSA_TRACE_LEVEL_FATAL,
                        "RQB_ERROR occured, compId(%u/%#x) compId-Lower(%u/%#x) ptr(0x%08x)",
                        comp_id, comp_id,
                        comp_id_lower, comp_id_lower,
                        rqb_ptr);

    LSA_FATAL_ERROR_TYPE LSAError;

    LSAError.lsa_component_id  = comp_id;
    LSAError.module_id         = PSI_MODULE_ID;
    LSAError.line              = (LSA_UINT16)__LINE__;
    LSAError.error_code[0]     = 0;
    LSAError.error_code[1]     = 0;
    LSAError.error_code[2]     = 0;
    LSAError.error_code[3]     = 0;
    LSAError.error_data_length = 0;
    LSAError.error_data_ptr    = LSA_NULL; /* currently no message */
    PSI_FATAL_ERROR("PSI", (LSA_UINT16)(sizeof(LSA_FATAL_ERROR_TYPE)), &LSAError);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_FATAL_ERROR_OCCURED
LSA_BOOL PSI_FATAL_ERROR_OCCURED( LSA_VOID )
{
    return g_isPsiInFatal;
}
#else
#error "by design a function!"
#endif

/*===========================================================================*/
/*   HW param adaption                                                       */
/*===========================================================================*/

#ifndef PSI_GET_HD_PARAM
LSA_VOID PSI_GET_HD_PARAM(
    LSA_UINT16                    * ret_val_ptr,
    const PSI_HD_INPUT_TYPE       * const hd_ptr,
    const PSI_HD_SYS_ID_TYPE      * hd_sys_id_ptr,
    PSI_HD_PARAM_PTR_TYPE           hd_param_ptr )
{
    // PSI_SYS_HANDLE
    g_eddpSysHandle.hd_nr       = hd_ptr->hd_id;
    g_eddpSysHandle.pnio_if_nr  = 0;
    g_eddpSysHandle.edd_comp_id = hd_ptr->edd_type;
    hd_param_ptr->hd_sys_handle = &g_eddpSysHandle;

    // PSI EDDP Parameters
    hd_param_ptr->edd_type                                = LSA_COMP_ID_EDDP;
    hd_param_ptr->edd.eddp.icu_location                   = EDDP_LOCATION_LOCAL;
    hd_param_ptr->edd.eddp.hw_type                        = EDDP_HW_ERTEC200P;
    hd_param_ptr->edd.eddp.hw_interface                   = EDDP_HWIF1;

#ifdef BOARD_TYPE_STEP_3
    hd_param_ptr->edd.eddp.board_type                     = PSI_EDDP_BOARD_TYPE_EB200P__ERTEC200P_REV2_STEP3;
#else
    hd_param_ptr->edd.eddp.board_type                     = PSI_EDDP_BOARD_TYPE_EB200P__ERTEC200P_REV2;
#endif
    hd_param_ptr->edd.eddp.appl_timer_mode                = EDDP_APPL_TIMER_CFG_MODE_TRANSFER_END;
    hd_param_ptr->edd.eddp.appl_timer_reduction_ratio     = 1;
    hd_param_ptr->edd.eddp.is_transfer_end_correction_pos = LSA_TRUE;
    hd_param_ptr->edd.eddp.transfer_end_correction_value  = 0;

    hd_param_ptr->edd.eddp.pnip.base_ptr                  = (LSA_UINT8*)U_PNIP__BASE;
    hd_param_ptr->edd.eddp.pnip.phy_addr                  = U_PNIP__BASE;
    hd_param_ptr->edd.eddp.pnip.size                      = 0x00200000UL;

    hd_param_ptr->edd.eddp.sdram_CRT.base_ptr             = (LSA_UINT8*) 0;
    hd_param_ptr->edd.eddp.sdram_CRT.phy_addr             = 0;
    hd_param_ptr->edd.eddp.sdram_CRT.size                 = 0x00001000;

    hd_param_ptr->edd.eddp.perif_ram.base_ptr             = (LSA_UINT8*)U_PERIF_AHB__BASE;
    hd_param_ptr->edd.eddp.perif_ram.phy_addr             = U_PERIF_AHB__BASE;
    hd_param_ptr->edd.eddp.perif_ram.size                 = 0x00010000UL; 

    hd_param_ptr->edd.eddp.k32_tcm.base_ptr               = (LSA_UINT8*)KRISC_DTCM_BASE;
    hd_param_ptr->edd.eddp.k32_tcm.phy_addr               = KRISC_DTCM_BASE;
    hd_param_ptr->edd.eddp.k32_tcm.size                   = 0x00020000UL;

    hd_param_ptr->edd.eddp.apb_periph_scrb.base_ptr       = (LSA_UINT8*)U_SCRB__BASE;
    hd_param_ptr->edd.eddp.apb_periph_scrb.phy_addr       = U_SCRB__BASE;
    hd_param_ptr->edd.eddp.apb_periph_scrb.size           = 0x00000100UL;

    hd_param_ptr->edd.eddp.apb_periph_perif.base_ptr      = (LSA_UINT8*)U_PERIF_APB__BASE;
    hd_param_ptr->edd.eddp.apb_periph_perif.phy_addr      = U_PERIF_APB__BASE;
    hd_param_ptr->edd.eddp.apb_periph_perif.size          = 0x00008000UL;

    hd_param_ptr->edd.eddp.isIODataInHost                 = LSA_FALSE; // do not care since hera is not supported

    for (LSA_UINT8 port_idx = 0; port_idx < IOD_CFG_PDEV_NUMOF_PORTS; port_idx++)
    {
        hd_param_ptr->port_map[port_idx + 1].hw_phy_nr    = port_idx;
        hd_param_ptr->port_map[port_idx + 1].hw_port_id   = port_idx + 1;
        Bsp_GetPortMacAddr((LSA_UINT8*)&hd_param_ptr->port_mac[port_idx], port_idx + 1);
    }
    Bsp_GetMacAddr((LSA_UINT8*)hd_param_ptr->if_mac);

    firmware_hw_load_coprocessor_pnip_arm9();

    *ret_val_ptr = PSI_OK;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_FREE_HD_PARAM
LSA_VOID PSI_FREE_HD_PARAM(
    LSA_UINT16 * ret_val_ptr,
    LSA_UINT16   hd_id )
{
    *ret_val_ptr = LSA_RET_OK;
    LSA_UNUSED_ARG(hd_id);
}
#else
#error "by design a function!"
#endif

/*===========================================================================*/
/*   PNIO IR adaption                                                        */
/*===========================================================================*/

#ifndef PSI_HD_ENABLE_IR
LSA_VOID PSI_HD_ENABLE_IR(
    PSI_SYS_HANDLE sys_handle )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_HD_DISABLE_IR
LSA_VOID PSI_HD_DISABLE_IR(
    PSI_SYS_HANDLE sys_handle )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_THREAD_READY
LSA_VOID PSI_THREAD_READY( LSA_VOID_PTR_TYPE arg )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_THREAD_STOPPED
LSA_VOID PSI_THREAD_STOPPED( LSA_VOID_PTR_TYPE arg )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_GET_IM_SERIAL_NUMBER
LSA_VOID PSI_GET_IM_SERIAL_NUMBER(
    LSA_UINT16 * ret_val_ptr,
    LSA_UINT8  * im_serial_nr_ptr,
    LSA_UINT16   im_serial_nr_size )
{
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_HD_ENABLE_EVENT
LSA_VOID PSI_HD_ENABLE_EVENT(
    PSI_SYS_HANDLE sys_handle )
{
    LSA_UNUSED_ARG(sys_handle);
    PSI_INIT_ISR();
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_HD_DISABLE_EVENT
LSA_VOID PSI_HD_DISABLE_EVENT(
    PSI_SYS_HANDLE sys_handle )
{
    // shutdown sequence not implemented. 
    LSA_UNUSED_ARG(sys_handle);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_ALLOC_REENTRANCE_LOCK_RECURSIVE
LSA_VOID PSI_ALLOC_REENTRANCE_LOCK_RECURSIVE(
    LSA_UINT16 * ret_val_ptr,
    LSA_UINT16 * lock_handle_ptr )
{
    PSI_ALLOC_REENTRANCE_LOCK(ret_val_ptr, lock_handle_ptr); 
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/
#ifndef PSI_ALLOC_REENTRANCE_LOCK_PRIO_PROTECTED
LSA_VOID PSI_ALLOC_REENTRANCE_LOCK_PRIO_PROTECTED(
    LSA_UINT16 * ret_val_ptr,
    LSA_UINT16 * lock_handle_ptr )
{
    // Only necessary for hera support cases. No need for implementation.
    *lock_handle_ptr = PSI_INVALID_LOCK_HANDLE;
    *ret_val_ptr = LSA_RET_OK;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_LD_GET_HD_PARAM
LSA_VOID PSI_LD_GET_HD_PARAM(
    LSA_UINT16            * ret_val_ptr,
    LSA_UINT16              hd_id,
    PSI_HD_PARAM_PTR_TYPE   hd_param_ptr )
{
    LSA_UNUSED_ARG(hd_id);

    hd_param_ptr->edd.eddp.isIODataInHost = LSA_FALSE; // do not care since hera is not supported
    *ret_val_ptr = PSI_OK;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_GET_LD_PARAM
LSA_VOID PSI_GET_LD_PARAM(
    LSA_UINT16 *                 ret_val_ptr,
    const PSI_LD_SYS_ID_TYPE   * ld_sys_id_ptr,
    PSI_LD_PARAM_PTR_TYPE        ld_param_ptr )
{
    LSA_UNUSED_ARG (ld_sys_id_ptr);
    LSA_UNUSED_ARG (ld_param_ptr);
    // since only one hd number exists, return "ok"
    *ret_val_ptr = PSI_OK;
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/
/* Lib for multi thread support                                               */
/*----------------------------------------------------------------------------*/

/* atomic functions */

// Atomic Add
// **********
// returns: result of addition
// -----------------------
//  *ptr += val;
//  return *ptr;
#ifndef PSI_ATOMIC_ADD_INT32
LSA_INT32 PSI_ATOMIC_ADD_INT32(
    LSA_INT32 volatile * const ptr,
    LSA_INT32            const val )
{
    PSI_ENTER_IR_LOCK(0);
    *ptr += val;
    PSI_EXIT_IR_LOCK(0);

    return *ptr;
}
#else
#error "by design a function!"
#endif

// Atomic Or
// *********
// returns: initial value of *ptr
// -----------------------
//  initial_value = *ptr;
//  *ptr |= val;
//  return initial_value;
#ifndef PSI_ATOMIC_OR_INT32
LSA_INT32 PSI_ATOMIC_OR_INT32(
    LSA_INT32 volatile * const ptr,
    LSA_INT32            const val )
{
    LSA_INT32 initial_value;

    PSI_ENTER_IR_LOCK(0);
    initial_value = *ptr;
    *ptr |= val;
    PSI_EXIT_IR_LOCK(0);

    return initial_value;
}
#else
#error "by design a function!"
#endif

// Atomic And
// **********
// returns: initial value of *ptr
// -----------------------
//  initial_value = *ptr;
//  *ptr &= val;
//  return initial_value;
#ifndef PSI_ATOMIC_AND_INT32
LSA_INT32 PSI_ATOMIC_AND_INT32(
    LSA_INT32 volatile * const ptr,
    LSA_INT32            const val )
{
    LSA_INT32 initial_value;

    PSI_ENTER_IR_LOCK(0);
    initial_value = *ptr;
    *ptr &= val;
    PSI_EXIT_IR_LOCK(0);
 
    return initial_value;
}
#else
#error "by design a function!"
#endif

// Atomic Compare-and-Swap
// ***********************
// returns: initial value of *ptr
// -----------------------
//  initial_value = *ptr;
//  if (*ptr == expected)
//  {
//      *ptr = val;
//  }
//  return initial_value;
#ifndef PSI_ATOMIC_CAS_INT32
LSA_INT32 PSI_ATOMIC_CAS_INT32(
    LSA_INT32 volatile * const ptr,
    LSA_INT32            const expected,
    LSA_INT32            const val )
{
    LSA_INT32 initial_value;

    PSI_ENTER_IR_LOCK(0);
    initial_value = *ptr;
    if (*ptr == expected)
    {
        *ptr = val;
    }
    PSI_EXIT_IR_LOCK(0);

    return initial_value;
}
#else
#error "by design a function!"
#endif

/* thread functions */

#ifndef PSI_GET_CURRENT_THREAD_ID
LSA_VOID PSI_GET_CURRENT_THREAD_ID(
    LSA_UINT16 * const ret_val_ptr,
    LSA_UINT32 * const thread_id_ptr )
{
    *thread_id_ptr = OsGetThreadId();
    *ret_val_ptr   = LSA_RET_OK;
}
#else
#error "by design a function!"
#endif

#ifndef PSI_CREATE_THREAD
LSA_VOID PSI_CREATE_THREAD(
    LSA_UINT16 *       const ret_val_ptr,
    LSA_UINT32 *       const thread_id_ptr,
    const LSA_CHAR *   const thread_name_ptr,
    LSA_INT32          const thread_priority,
    PSI_SYS_THREAD_CBF const cbf_fct_ptr,
    LSA_UINT32         const u_param,
    LSA_VOID_PTR_TYPE  const args_ptr,
    LSA_BOOL           const bJoinableThread )
{
    PNIO_UINT32 retVal = 0;
    PNIO_UINT32 priority = 0;

    switch (thread_priority)
    {
        case PSI_CREATE_THREAD_PRIORITY_NORMAL:
        {
            priority = TASK_PRIO_IP_STACK;
            break;
        }
        default:
        {
            PSI_SYSTEM_TRACE_01(0, LSA_TRACE_LEVEL_ERROR, "PSI_CREATE_THREAD: unexpected thread priority: %d", thread_priority);
            PSI_FATAL(0);
            break;
        }
    }

    retVal = OsCreateThread((PNIO_VOID(*)(PNIO_VOID))u_param, args_ptr, (LSA_UINT8*)thread_name_ptr, priority, OS_TASK_DEFAULT_STACKSIZE, thread_id_ptr);
    PSI_ASSERT(retVal == PNIO_OK);

    retVal = OsStartThread(*thread_id_ptr);
    PSI_ASSERT(retVal == PNIO_OK);

    LSA_UNUSED_ARG(cbf_fct_ptr);
    LSA_UNUSED_ARG(bJoinableThread);

    *ret_val_ptr = LSA_RET_OK;
}
#else
#error "by design a function!"
#endif

#ifndef PSI_JOIN_THREAD
LSA_VOID PSI_JOIN_THREAD(
    LSA_UINT16 * ret_val_ptr,
    LSA_UINT32   thread_id )
{
    PNIO_UINT32 retVal = 0;

    retVal = OsJoinThread(thread_id);
    PSI_ASSERT(retVal == PNIO_OK);

    *ret_val_ptr = LSA_RET_OK;
}
#else
#error "by design a function!"
#endif

#ifndef PSI_EXIT_THREAD
LSA_VOID PSI_EXIT_THREAD(
    LSA_INT32 const exit_code )
{
    PNIO_UINT32 retVal = 0;

    retVal = OsExitThread();
    PSI_ASSERT(retVal == PNIO_OK);
    
    LSA_UNUSED_ARG(exit_code);
}
#else
#error "by design a function!"
#endif

/* memory barrier functions */

#ifndef PSI_SYNC_SYNCHRONIZE
LSA_VOID PSI_SYNC_SYNCHRONIZE( LSA_VOID )
{
    // implementation is not necessary
}
#else
#error "by design a function!"
#endif

/*****************************************************************************/
/*                             Message Queues                                */
/*****************************************************************************/

#if (PSI_CFG_USE_EXTERNAL_MSGQ == 1)
#ifndef PSI_MSGQ_ALLOC
LSA_UINT16 PSI_MSGQ_ALLOC(
    LSA_UINT16 * const msgq_id_ptr)
{
    LSA_UINT32  taskID;
    PNIO_UINT32 retVal = 0;

    taskID = OsGetThreadId();

    retVal = OsCreateMsgQueue(taskID);
    PSI_ASSERT(retVal == PNIO_OK);

    *msgq_id_ptr = taskID;

    return LSA_RET_OK;
}
#else
#error "by design a function!"
#endif

#ifndef PSI_MSGQ_FREE
LSA_UINT16 PSI_MSGQ_FREE(
    LSA_UINT16 const msgq_id)
{
    PNIO_UINT32 retVal = 0;

    retVal = OsDeleteMsgQueue(msgq_id);
    PSI_ASSERT(retVal == PNIO_OK);

    return LSA_RET_OK;
}
#else
#error "by design a function!"
#endif

#ifndef PSI_MSGQ_SEND_MSG
LSA_VOID PSI_MSGQ_SEND_MSG(
    LSA_UINT16 const msgq_id,
    LSA_VOID  * rqb)
{
    PNIO_UINT32 retVal = 0;

    retVal = OsSendMessage(msgq_id, rqb, OS_MBX_PRIO_NORM);
    PSI_ASSERT(retVal == PNIO_OK);
}
#else
#error "by design a function!"
#endif

#ifndef PSI_MSGQ_RECEIVE_MSG
LSA_VOID PSI_MSGQ_RECEIVE_MSG(
    LSA_UINT16 const msgq_id,
    LSA_VOID * rqb)
{
    PNIO_UINT32 retVal = 0;

    retVal = OsReadMessageBlocked(&rqb, msgq_id);
    PSI_ASSERT(retVal == PNIO_OK);
}
#else
#error "by design a function!"
#endif
#endif //PSI_CFG_USE_EXTERNAL_MSGQ == 1

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
