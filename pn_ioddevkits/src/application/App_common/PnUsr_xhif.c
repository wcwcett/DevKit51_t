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
/*  F i l e               &F: PnUsr_xhif.c                              :F&  */
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


/**
* @file     PnUsr_xhif.c
* @brief    Functions for XHIF - library for Ertec
*
* XHIF functionality allows user to use ERTEC Devkit only for PN-stack functionalities
* and to realize user functionality on other device. The other device have to upload
* firmware to ERTEC Devkit as a binary and then communicate data.. This can be realized
* via XHIF memory interface.
* This file is collection of functions, which are used by Ertec processor to communicate
* with host device.
*/

#include <iod_cfg.h>
#if(1 == IOD_USED_WITH_XHIF_HOST)
#include "pniousrd.h"
#include "pnio_types.h"
#include "usriod_utils.h"

#include "usrapp_cfg.h"
#include "PnUsr_xhif.h"
#include "PnUsr_Api.h"
#include "os.h"
#include "os_taskprio.h"
#include "xx_mem.h"

#include "pndv_inc.h"

#include "pnpb_com.h"
#include "pnpb_peri.h"
#include "pnpb_peri_diag.h"
#include "pnpb_sub_real.h"
#include "pnpb_cfg.h"
#include "pnpb.h"

#include "nv_data.h"

#include <cyg/hal/ertec200p_reg.h>
#include "bspadapt.h"

#include "pnpb_trc.h"

// *-----------------------------------------------------------------
// * defines
// *-----------------------------------------------------------------

#ifdef LTRC_ACT_MODUL_ID
	#undef LTRC_ACT_MODUL_ID
#endif

#define LTRC_ACT_MODUL_ID   201
#define PNPB_MODULE_ID      201

// *-----------------------------------------------------------------
// * external variables
// *-----------------------------------------------------------------
extern PNPB_INSTANCE    Pnpb;
extern PNPB_REAL PnpbMod;

/* Local prototypes */
PNIO_VOID PnUsr_xhif_cyclical_out_trylock(PNIO_UINT32 tmp_write_ptr);
PNIO_VOID PnUsr_xhif_cyclical_out_unlock(PNIO_UINT32 tmp_write_ptr);
PNIO_VOID PnUsr_xhif_cyclical_in_trylock(PNIO_UINT32 tmp_read_ptr);
PNIO_VOID PnUsr_xhif_cyclical_in_unlock(PNIO_UINT32 tmp_read_ptr);
PNIO_VOID PnUsr_xhif_acyc_out_trylock(PNIO_UINT32 tmp_write_ptr);
PNIO_VOID PnUsr_xhif_acyc_out_unlock(PNIO_UINT32 tmp_write_ptr);
PNIO_VOID PnUsr_xhif_acyc_in_trylock(PNIO_UINT32 tmp_read_ptr);
PNIO_VOID PnUsr_xhif_acyc_in_unlock(PNIO_UINT32 tmp_read_ptr);


PNIO_VOID PnUsr_xhif_check_cyclical_write_pointer(PNIO_UINT32 *ptr_tmp_write_ptr);
PNIO_VOID PnUsr_xhif_increment_cyclical_write_pointer(PNIO_UINT32 tmp_write_ptr);
PNIO_UINT32 PnUsr_xhif_check_cyclical_read_pointer(PNIO_UINT32 *p_read_ptr);
PNIO_VOID PnUsr_xhif_increment_cyclical_read_pointer(PNIO_UINT32 *p_read_ptr);

PNIO_UINT32 PnUsr_xhif_check_acyc_read_pointer(PNIO_UINT32 *p_read_ptr);
PNIO_VOID PnUsr_xhif_increment_acyc_read_pointer(PNIO_UINT32 *p_read_ptr);
PNIO_VOID PnUsr_xhif_acyc_read_continue(PNIO_UINT32 id, PNIO_UINT32 is_more_follows, PNIO_UINT32 tmp_read_ptr);
PNIO_VOID PnUsr_xhif_acyc_read_telegram(PNIO_UINT32 id, PNIO_UINT32 tmp_read_ptr);


PNIO_VOID PnUsr_xhif_call_function(PNPB_XHIF_ACYC_TELEGRAMS id, PNIO_UINT8 *prm);

PNIO_VOID PnUsr_xhif_acyc_isr(PNIO_VOID);
PNIO_VOID PnUsr_xhif_acyc_dsr(PNIO_VOID);
PNIO_VOID PnUsr_xhif_acyc_confirm_isr(PNIO_VOID);
PNIO_VOID PnUsr_xhif_acyc_confirm_dsr(PNIO_VOID);

PNIO_UINT8 PnUsr_get_in_data_size(PNIO_UINT32 ModId);
PNIO_UINT8 PnUsr_get_out_data_size(PNIO_UINT32 ModId);

PNIO_UINT32 read_waits;
PNIO_UINT32 write_waits;
static PNIO_BOOL use_ertec_nv;
static PNIO_BOOL traces_store = PNIO_FALSE;
static PNIO_BOOL traces_was_restore = PNIO_FALSE;

static          PNIO_UINT32   Task_Acyc_Write = 0;

#define LOCK_ALSO_FOR_READING 1

typedef struct _ASYNC_REC_DATA
{
    PNDV_RQB_DS*       pDsPeri;
    PNIO_UINT8         Add1;
    PNIO_UINT16        Add2;
    PNDV_IFACE_STRUCT* pPndvIf;
} ASYNC_REC_DATA;

typedef union
{
    PNIO_UINT32     vu32;
    PNIO_ERR_STAT   State;   // 4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
}   UNION_PNIO_ERR_STAT;

 PNUSR_XHIF_SECTION_CYC_IN      pnusr_xhif_buffer_services  pnusr_xhif_cyclic_in_services;
 PNUSR_XHIF_SECTION_CYC_IN_BUF  pnusr_xhif_subslot_data     pnusr_xhif_cyclic_in [PNUSR_XHIF_SIZE_OF_BUFFER_FOR_CYCLIC];
 PNUSR_XHIF_SECTION_CYC_OUT     pnusr_xhif_buffer_services  pnusr_xhif_cyclic_out_services;
 PNUSR_XHIF_SECTION_CYC_OUT_BUF pnusr_xhif_subslot_data     pnusr_xhif_cyclic_out[PNUSR_XHIF_SIZE_OF_BUFFER_FOR_CYCLIC];
 PNUSR_XHIF_SECTION_ACY_IN      pnusr_xhif_buffer_services  pnusr_xhif_acyc_in_services;
 PNUSR_XHIF_SECTION_ACY_IN_BUF  pnusr_xhif_acyclic          pnusr_xhif_acyc_in   [PNUSR_XHIF_SIZE_OF_BUFFER_FOR_ACYC];
 PNUSR_XHIF_SECTION_ACY_OUT     pnusr_xhif_buffer_services  pnusr_xhif_acyc_out_services;
 PNUSR_XHIF_SECTION_ACY_OUT_BUF pnusr_xhif_acyclic          pnusr_xhif_acyc_out  [PNUSR_XHIF_SIZE_OF_BUFFER_FOR_ACYC];
 PNUSR_XHIF_SECTION_TRACES      PNIO_UINT8                  pnusr_xhif_trace_out [PNUSR_XHIF_SIZE_OF_BUFFER_FOR_TRACES];


/**
 *  @brief Interrupt routine for new acyclic telegram from Host signaling
 *
 *  @param          void
 *
 *  @return         void
 *
 *  Acknowledge interrupt
 *
 */
PNIO_VOID PnUsr_xhif_acyc_isr(PNIO_VOID)
{
    Bsp_GPIO_isr(PNUSR_INTERRUPT_GPIO6);
}   /* PnUsr_xhif_acyc_isr */


/**
 *  @brief Interrupt routine for new acyclic telegram from Host signaling
 *
 *  @param          void
 *
 *  @return         void
 *
 *  Signal to main function that there are new data
 *
 */
PNIO_VOID PnUsr_xhif_acyc_dsr(PNIO_VOID)
{
    OsGiveSemB(PnioNewAcyclicSemId);
}   /* PnUsr_xhif_acyc_dsr */


/**
 *  @brief Register Interrupt routine for new acyclic telegram from Host signaling
 *
 *  @param          void
 *
 *  @return         void
 *
 *  Settings of interrupt handling
 *
 */
PNIO_VOID PnUsr_xhif_acyc_set_int(PNIO_VOID)
{
    Bsp_GPIO_set_int_for_acyc(PNUSR_INTERRUPT_GPIO6, PnUsr_xhif_acyc_isr, PnUsr_xhif_acyc_dsr);
}   /* PnUsr_xhif_acyc_set_int */


/**
 *  @brief Interrupt routine for  acyclic telegram receive confirmation from Host
 *
 *  @param          void
 *
 *  @return         void
 *
 *  Acknowledge interrupt
 *
 */
PNIO_VOID PnUsr_xhif_acyc_confirm_isr(PNIO_VOID)
{
    Bsp_GPIO_isr(PNUSR_INTERRUPT_GPIO2);
}   /* PnUsr_xhif_acyc_confirm_isr */


/**
 *  @brief Interrupt routine for  acyclic telegram receive confirmation from Host
 *
 *  @param          void
 *
 *  @return         void
 *
 *  Enable new write to happen
 *
 */
PNIO_VOID PnUsr_xhif_acyc_confirm_dsr(PNIO_VOID)
{
    /* Acyclic was captured by host, Ertec can continue sending acyclic telegrams */
    OsGiveSemB(PnioAcycSyncSemId);
}   /* PnUsr_xhif_acyc_confirm_dsr */


/**
 *  @brief Register Interrupt routine for acyclic telegram receive confirmation from Host
 *
 *  @param          void
 *
 *  @return         void
 *
 *  Settings of interrupt handling
 *
 */
PNIO_VOID PnUsr_xhif_acyc_confirm_set_int(PNIO_VOID)
{
    Bsp_GPIO_set_int_for_acyc_confirm(PNUSR_INTERRUPT_GPIO2, PnUsr_xhif_acyc_confirm_isr, PnUsr_xhif_acyc_confirm_dsr);
}   /* PnUsr_xhif_acyc_confirm_set_int */



/**
 *  @brief Slot and subslot address to number of submodule
 *
 *  @param[in]      slot            Slot number
 *  @param[in]      subslot         Subslot number
 *  @param[out]     *submodule      Submodule number
 *
 *  @return                         Success[PNIO_OK, PNIO_NOT_OK]
 *
 *  Address transfer
 *
 */
PNIO_UINT32 PnUsr_xhif_addr_slot_to_submodule(PNIO_UINT32 slot, PNIO_UINT32 subslot, PNIO_UINT32 *submodule)
{
    PNIO_UINT32 i;
    for(i = 0; i < PNUSR_XHIF_MAX_NUM_OF_SUBMODULES; i++)
    {
        if(NULL == pnusr_submodule_params[i])
        {
            /* Array item is empty - skip it */
            continue;
        }

        if(((slot == pnusr_submodule_params[i]->Slot) && (subslot == pnusr_submodule_params[i]->Subslot)))
        {
            *submodule = i;
            return PNIO_OK;
        }

        /* unattended position -> = end of the list -> not found */
        if((0 == pnusr_submodule_params[i]->Slot) && (0 == pnusr_submodule_params[i]->Subslot))
        {

            PNIO_printf("Unattended position -> = end of the list -> not found\n");

            return PNIO_NOT_OK;
        }
    }

    /* went through whole list and not found */
    return PNIO_NOT_OK;
}   /* PnUsr_xhif_addr_slot_to_submodule */

/**
 *  @brief Submodule address to number of  slot and subslot
 *
 *  @param[in]      *slot           Slot number
 *  @param[in]      *subslot        Subslot number
 *  @param[out]     submodule       Submodule number
 *
 *  @return                         Success[PNIO_OK, PNIO_NOT_OK]
 *
 *  Address transfer
 *
 */
PNIO_UINT32 PnUsr_xhif_addr_submodule_to_slot(PNIO_UINT32 *slot, PNIO_UINT32 *subslot, PNIO_UINT32 submodule)
{
    /* submodule position not occupied */
    if((NULL == pnusr_submodule_params[submodule]) ||
        ((0 == pnusr_submodule_params[submodule]->Slot) && (0 == pnusr_submodule_params[submodule]->Subslot)))
    {
        return PNIO_NOT_OK;
    }
    *slot = pnusr_submodule_params[submodule]->Slot;
    *subslot = pnusr_submodule_params[submodule]->Subslot;
    return PNIO_OK;
}   /* PnUsr_xhif_addr_submodule_to_slot */


/**
 *  @brief Initialization of device - used submodules
 *
 *  @param[in]      *prm            Parameters to be used for device
 *
 *  @return                         Success[PNIO_OK, PNIO_NOT_OK]
 *
 *  Example submodules are set in here
 *
 */
PNIO_UINT32 PnUsr_xhif_IO_buffer_init(PNIO_SUB_LIST_ENTRY* pIoSubList,
                                    PNIO_UINT32          NumOfSubListEntries)
{
    PNIO_UINT32 i;
    PNIO_SUB_LIST_ENTRY* pIoSub;

    for (i = 0; i < PNUSR_XHIF_MAX_NUM_OF_SUBMODULES; ++i) {
        pnusr_submodule_params[i] = NULL;
    }

    for(i = 0; i < NumOfSubListEntries; i++)
    {
        pIoSub = pIoSubList + i;
        OsAlloc((void**)&pnusr_submodule_params[i], 0x00, sizeof(pnusr_subslot_params));
        pnusr_submodule_params[i]->Slot = pIoSub->Slot;
        pnusr_submodule_params[i]->Subslot = pIoSub->Subslot;
        pnusr_submodule_params[i]->InData_size = pIoSub->InDatLen;
        pnusr_submodule_params[i]->OutData_size = pIoSub->OutDatLen;
        pnusr_submodule_params[i]->Direction = PNUSR_XHIF_DIRECTION_NO_DATA;
        /* Device is ready to receive data at the beginning => Out IOcS is PNIO_S_GOOD */
        /* Out.iocs have to be in PNIO_S_GOOD, others PNIO_S_BAD or we don't care - use BAD as default */
        pnusr_submodule_params[i]->InIOcS = PNIO_S_BAD;
        pnusr_submodule_params[i]->InIOpS = PNIO_S_BAD;
        pnusr_submodule_params[i]->OutIOcS = PNIO_S_GOOD;
        pnusr_submodule_params[i]->OutIOpS = PNIO_S_BAD;

        if(0 < pIoSub->InDatLen)
        {
            pnusr_submodule_params[i]->Direction |=  PNUSR_XHIF_DIRECTION_IN;
        }
        if(0 < pIoSub->OutDatLen)
        {
            pnusr_submodule_params[i]->Direction |=  PNUSR_XHIF_DIRECTION_OUT;
        }
        if((0 < pnusr_submodule_params[i]->OutData_size) || (0 < pnusr_submodule_params[i]->InData_size))
        {
            OsAlloc((void**)&pnusr_submodule_IO_data[i], 0x00,
                    (pnusr_submodule_params[i]->InData_size + pnusr_submodule_params[i]->OutData_size));
        }
    }

    return PNIO_OK;
}   /* PnUsr_xhif_IO_buffer_init */


/**
 *  @brief Checks if there are data ready to be read
 *
 *  @return                         Success[PNIO_OK, PNIO_NOT_OK]
 *
 *  Underflow protection
 *
 */
PNIO_UINT32 PnUsr_xhif_check_cyclical_read_pointer(PNIO_UINT32 *p_read_ptr)
{
    /* Check against buffer underflow */
    PNIO_UINT32 tmp_write_ptr, tmp_read_ptr;
    PNIO_UINT32 status = PNIO_NOT_OK, i;
    /* Read index from GPMC */
    tmp_read_ptr = pnusr_xhif_cyclic_in_services.read_ptr;

    /* Check against read from invalid indexes */
    if(PNUSR_XHIF_SIZE_OF_BUFFER_FOR_CYCLIC <= tmp_read_ptr)
    {
        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nInvalid cyclical read index\n");
    }

    /* Check against buffer underflow */
    tmp_write_ptr = pnusr_xhif_cyclic_in_services.write_ptr;

    /* Check against buffer underflow */
    for (i = 0; i < 2; i++)
    {
        if (tmp_read_ptr != tmp_write_ptr)
        {
            status = PNIO_OK;
            break;
        }

        PnUsr_xhif_wait();

        tmp_read_ptr = pnusr_xhif_cyclic_in_services.read_ptr;
    }

    /* Return read index */
    *p_read_ptr = tmp_read_ptr;

    return status;
}   /* PnUsr_xhif_check_cyclical_read_pointer */


/**
 *  @brief Increment read pointer
 *
 *  @return         void
 *
 *  Respects ring buffer
 *
 */
PNIO_VOID PnUsr_xhif_increment_cyclical_read_pointer(PNIO_UINT32 *p_read_ptr)
{
    PNIO_UINT32 tmp_read_ptr = *p_read_ptr;

    /* Increment */
    if((PNUSR_XHIF_SIZE_OF_BUFFER_FOR_CYCLIC - 1) <= tmp_read_ptr)
    {
        tmp_read_ptr = 0;    /* buffer went over max to 0*/
    }
    else
    {
        tmp_read_ptr++;
    }

    /* Return new value */
    *p_read_ptr = tmp_read_ptr;
}   /* PnUsr_xhif_increment_cyclical_read_pointer */


/**
 *  @brief Lock against read while write
 *
 *  @param[in]      void
 *  @return         void
 *
 *  Two devices accesses same memory
 *
 */
PNIO_VOID PnUsr_xhif_cyclical_in_trylock(PNIO_UINT32 tmp_read_ptr)
{
    if(0 == pnusr_xhif_cyclic_in[tmp_read_ptr].Lock)
    {
#if(1 == LOCK_ALSO_FOR_READING)
            pnusr_xhif_cyclic_in[tmp_read_ptr].Lock = 0x01;
#endif
    }
    else
    {
        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nCyclical in lock failed\n");
    }
}   /* PnUsr_xhif_cyclical_trylock */


/**
 *  @brief Unlock for protection against read while write
 *
 *  @param[in]      void
 *  @return         void
 *
 *  Two devices accesses same memory
 *
 */
PNIO_VOID PnUsr_xhif_cyclical_in_unlock(PNIO_UINT32 tmp_read_ptr)
{
#if(1 == LOCK_ALSO_FOR_READING)
    pnusr_xhif_cyclic_in[tmp_read_ptr].Lock = 0x00;
#endif
}   /* PnUsr_xhif_cyclical_unlock */


/**
 *  @brief Check write possibility
 *
 *  @return                         Success[PNIO_OK, PNIO_NOT_OK]
 *
 *  Buffer overflow protection
 *
 */
PNIO_VOID PnUsr_xhif_check_cyclical_write_pointer(PNIO_UINT32 *ptr_tmp_write_ptr)
{
    PNIO_UINT32 tmp_write_ptr;
    tmp_write_ptr = pnusr_xhif_cyclic_out_services.write_ptr;
    /* Increment */
    if((PNUSR_XHIF_SIZE_OF_BUFFER_FOR_CYCLIC - 1) <= tmp_write_ptr)
    {
        tmp_write_ptr = 0;    /* buffer went over max to 0*/
    }
    else
    {
        tmp_write_ptr++;
    }

    if(tmp_write_ptr == pnusr_xhif_cyclic_out_services.read_ptr)
    {
        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nCyclical buffer overflow\n");
    }

    /* Clean the header data (slot, subslot, lock, IOxS, data_size) */
    //OsMemSet(&pnusr_xhif_cyclic_out[tmp_write_ptr].Lock, 0, 20);
    OsMemSet((void*)(&pnusr_xhif_cyclic_out[tmp_write_ptr].Lock), 0, 20);
    // OsMemSet is not to keep volitile qualifier happy 
	
    *ptr_tmp_write_ptr = tmp_write_ptr;
}   /* PnUsr_xhif_check_cyclical_write_pointer */


/**
 *  @brief Increment write pointer
 *
 *  @return         void
 *
 *  Respects ring buffer
 *
 */
PNIO_VOID PnUsr_xhif_increment_cyclical_write_pointer(PNIO_UINT32 tmp_write_ptr)
{
    pnusr_xhif_cyclic_out_services.write_ptr = tmp_write_ptr;
}   /* PnUsr_xhif_increment_cyclical_write_pointer */


/**
 *  @brief Lock against read while write
 *
 *  @param[in]      void
 *  @return         void
 *
 *  Two devices accesses same memory
 *
 */
PNIO_VOID PnUsr_xhif_cyclical_out_trylock(PNIO_UINT32 tmp_write_ptr)
{
    if(0 == pnusr_xhif_cyclic_out[tmp_write_ptr].Lock)
    {
        pnusr_xhif_cyclic_out[tmp_write_ptr].Lock = 0x01;
    }
    else
    {
        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nCyclical out lock failed\n");
    }
}   /* PnUsr_xhif_cyclical_trylock */


/**
 *  @brief Unlock for protection against read while write
 *
 *  @param[in]      void
 *  @return         void
 *
 *  Two devices accesses same memory
 *
 */
PNIO_VOID PnUsr_xhif_cyclical_out_unlock(PNIO_UINT32 tmp_write_ptr)
{
    pnusr_xhif_cyclic_out[tmp_write_ptr].Lock = 0x00;

}   /* PnUsr_xhif_cyclical_unlock */


/**
 *  @brief Writes Apdu status to XHIF
 *
 *  @param[in]      ArNum           Number of Application relation
 *  @param[in]      Apdu            Apdu status of AR
 *  @return                         Success[PNIO_OK, PNIO_NOT_OK]
 *
 *  Write to XHIF, out services = read from stack
 *
 */
PNIO_UINT32 PnUsr_xhif_cyclical_write_apdu(PNIO_UINT32 ArNum, PNIO_UINT32 Apdu)
{
    PNIO_UINT32 tmp_write_ptr;
    PNIO_UINT32 *DataPtr;
    /* Check if there is some space in memory interface */
    PnUsr_xhif_check_cyclical_write_pointer(&tmp_write_ptr);

    /* Exclusive access to buffer */
    PnUsr_xhif_cyclical_out_trylock(tmp_write_ptr);

    pnusr_xhif_cyclic_out[tmp_write_ptr].Data_size = 8;  /* 4B of Apdu status + 4B of ArNum*/
    /* Save data */
    DataPtr = (PNIO_UINT32 *)(pnusr_xhif_cyclic_out[tmp_write_ptr].Data);
    *DataPtr = ArNum;
    DataPtr++;
    *DataPtr = Apdu;

    /* Every telegram have to have info about slot, subslot and IOxS */
    /* Use Slot F0, subslot F0 for Apdu status */
    pnusr_xhif_cyclic_out[tmp_write_ptr].Slot = 0xF0;
    pnusr_xhif_cyclic_out[tmp_write_ptr].Subslot = 0xF0;
    pnusr_xhif_cyclic_out[tmp_write_ptr].IOxS = 0;

    /* Increment write pointer after write */
    PnUsr_xhif_cyclical_out_unlock(tmp_write_ptr);
    PnUsr_xhif_increment_cyclical_write_pointer(tmp_write_ptr);

    return PNIO_OK;
}   /* PnUsr_xhif_cyclical_write_apdu */

/**
 *  @brief Writes cyclical data to XHIF
 *
 *  @param[in]      submodule_no    Number of submodule
 *  @param[in]      slot_no         Slot number
 *  @param[in]      subslot_no      Subslot number
 *  @param[in]      DataLen         Amount of data to be transferred
 *  @param[in]      *pData          Pointer to data
 *  @return                         Success[PNIO_OK, PNIO_NOT_OK]
 *
 *  Write to XHIF, out services = read from stack
 *
 */
PNIO_UINT32 PnUsr_xhif_cyclical_write(PNIO_UINT32 submodule_no,
                                    PNIO_UINT32 slot_no,
                                    PNIO_UINT32 subslot_no,
                                    PNIO_UINT32 DataLen,
                                    PNIO_UINT8 *pData)
{
    PNIO_UINT32 tmp_write_ptr ;
    /* Check if there is some space in memory interface */
    PnUsr_xhif_check_cyclical_write_pointer(&tmp_write_ptr);
    PnUsr_xhif_cyclical_out_trylock(tmp_write_ptr);

    /* Only OUT have data to be sent to host, otherwise sent IOxS only */
    if( 0 != (pnusr_submodule_params[submodule_no]->Direction & PNUSR_XHIF_DIRECTION_OUT))    /* direction OUT or INOUT */
    {

        pnusr_xhif_cyclic_out[tmp_write_ptr].Data_size = DataLen;
        if((0 != DataLen) && (NULL != pData))
        {
            OsMemCpy((void*)pnusr_xhif_cyclic_out[tmp_write_ptr].Data,
                    pData, DataLen);
        }
    }
    else    /* No data, but still have to send to XHIF (transfer of IOxS) */
    {
        pnusr_xhif_cyclic_out[tmp_write_ptr].Data_size = 0;
    }
    /* Every telegram have to have info about slot, subslot and IOxS */
    pnusr_xhif_cyclic_out[tmp_write_ptr].Slot = slot_no;
    pnusr_xhif_cyclic_out[tmp_write_ptr].Subslot = subslot_no;
    pnusr_xhif_cyclic_out[tmp_write_ptr].IOxS = pnusr_submodule_params[submodule_no]->OutIOpS;

    /* Increment write pointer after write */
    PnUsr_xhif_cyclical_out_unlock(tmp_write_ptr);
    PnUsr_xhif_increment_cyclical_write_pointer(tmp_write_ptr);
    return PNIO_OK;
}   /* PnUsr_xhif_cyclical_write */


/**
 *  @brief Read cyclicall IO data from XHIF
 *
 *  @return                         Success[PNIO_OK, PNIO_NOT_OK]
 *
 *  Read from XHIF, in services = write to stack
 *
 */
PNIO_UINT32 PnUsr_xhif_cyclical_read()
{
    PNIO_UINT32 submodule_no, slot_no, subslot_no, tmp_read_ptr;

    while(PNIO_TRUE)
    {
        /* check if data ready in memory interface */
        if(PNIO_NOT_OK == PnUsr_xhif_check_cyclical_read_pointer(&tmp_read_ptr))
        {
            //PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nCyclical read not ready\n");
            return PNIO_OK;
        }
        /* ptr ++ */
        PnUsr_xhif_increment_cyclical_read_pointer(&tmp_read_ptr);
        /* Exclusive access to buffer */
        PnUsr_xhif_cyclical_in_trylock(tmp_read_ptr);

        /* Read slot and subslot number of incomming data */
        slot_no = pnusr_xhif_cyclic_in[tmp_read_ptr].Slot;
        subslot_no = pnusr_xhif_cyclic_in[tmp_read_ptr].Subslot;
        /* Find data position - submodule_no */
        if(PNIO_FALSE == PnUsr_xhif_addr_slot_to_submodule(slot_no, subslot_no, &submodule_no))
        {
            /* Invalid submodule / submodule not present */
            PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nInvalid slot/subslot\n");
        }

        /* Check size */
        /* Here, only IN expected to have IO data, otherwise only IOxS => size = 0 */
        else if( 0 != (pnusr_submodule_params[submodule_no]->Direction & PNUSR_XHIF_DIRECTION_IN))    /* direction IN or INOUT */
        {
            if (pnusr_xhif_cyclic_in[tmp_read_ptr].Data_size ==
                pnusr_submodule_params[submodule_no]->InData_size) /* Data present => Copy them to Ertec buffer*/
            {
                if(0 != pnusr_xhif_cyclic_in[tmp_read_ptr].Data_size)
                {
                    OsMemCpy((void*)pnusr_submodule_IO_data[submodule_no],
                            (const void*)pnusr_xhif_cyclic_in[tmp_read_ptr].Data,
                            pnusr_xhif_cyclic_in[tmp_read_ptr].Data_size);
                }
                /* Copy IOxS */
                pnusr_submodule_params[submodule_no]->InIOpS = pnusr_xhif_cyclic_in[tmp_read_ptr].IOxS;
                pnusr_submodule_params[submodule_no]->InIOcS = pnusr_xhif_cyclic_in[tmp_read_ptr].IOxS;
            }
        }
        else    /* direction OUT or NO_DATA */
        {
            if(0 == pnusr_xhif_cyclic_in[tmp_read_ptr].Data_size)
            {
                /* No data to copy */
                /* But have consumer status */
                pnusr_submodule_params[submodule_no]->InIOcS = pnusr_xhif_cyclic_in[tmp_read_ptr].IOxS;
            }
        }

        PnUsr_xhif_cyclical_in_unlock(tmp_read_ptr);
        pnusr_xhif_cyclic_in_services.read_ptr= tmp_read_ptr;
    }
    return PNIO_NOT_OK;
}   /* PnUsr_xhif_cyclical_read */


/**
 *  @brief Instead of PNIO_cbf_data_read
 *
 *  @param[in]      DevHndl         Device handle
 *  @param[in]      *pAddr          Geographical or logical address
 *  @param[in]      BufLen          Length of the submodule input data
 *  @param[in]      *pBuffer        Ptr to data buffer to read from
 *  @param[in]      IOpS            Remote (io controller) provider status
 *  @return                         IOcS - Consumer status
 *
 *  From stack to XHIF and through it to application
 *
 */
PNIO_IOXS PnUsr_xhif_IO_data_read(PNIO_UINT32   DevHndl,
                                PNIO_DEV_ADDR   *pAddr,
                                PNIO_UINT32     BufLen,
                                PNIO_UINT8*     pBuffer,
                                PNIO_IOXS       IOpS
                                )
{
    PNIO_UINT32 submodule_no;
    PNIO_UINT32 slot_no    = pAddr->Geo.Slot;
    PNIO_UINT32 subslot_no = pAddr->Geo.Subslot;
    PNIO_UNUSED_ARG (DevHndl);

    /* Find slot and subslot */
    if(PNIO_FALSE == PnUsr_xhif_addr_slot_to_submodule(slot_no, subslot_no, &submodule_no))
    {
        return PNIO_S_BAD;
    }
    /* Store provider status */
    pnusr_submodule_params[submodule_no]->OutIOpS = IOpS;
    /* copy to pre xhif structure */
    if(PNIO_OK == PnUsr_xhif_cyclical_write(submodule_no, slot_no, subslot_no, BufLen, pBuffer))
    {
        return pnusr_submodule_params[submodule_no]->OutIOcS;
    }
    else
    {
        return PNIO_S_BAD;
    }
}   /* PnUsr_xhif_IO_data_read */


/**
 *  @brief Instead of PNIO_cbf_data_write
 *
 *  @param[in]      DevHndl         Device handle
 *  @param[in]      *pAddr          Geographical or logical address
 *  @param[in]      BufLen          Length of the submodule input data
 *  @param[in]      *pBuffer        Ptr to data buffer to read from
 *  @param[in]      IOpS            Remote (io controller) consumer status
 *  @return                         IOpS - Provider status
 *
 *  From Application to XHIF and through it to stack
 *
 */
PNIO_IOXS PnUsr_xhif_IO_data_write(PNIO_UINT32  DevHndl,
                                PNIO_DEV_ADDR   *pAddr,
                                PNIO_UINT32     BufLen,
                                PNIO_UINT8      *pBuffer,
                                PNIO_IOXS       IOcS
                                )
{
    PNIO_UINT32 submodule_no;
    PNIO_UINT32 slot_no    = pAddr->Geo.Slot;
    PNIO_UINT32 subslot_no = pAddr->Geo.Subslot;
    PNIO_UNUSED_ARG (DevHndl);

    /* Find slot and subslot */
    if(PNIO_FALSE == PnUsr_xhif_addr_slot_to_submodule(slot_no, subslot_no, &submodule_no))
    {
        return PNIO_S_BAD;
    }
    /* Store consumer status */
    pnusr_submodule_params[submodule_no]->InIOcS = IOcS;

    /* Read data received via XHIF from between buffer */
    OsMemCpy(pBuffer, (const void*)pnusr_submodule_IO_data[submodule_no], BufLen);
	
    return pnusr_submodule_params[submodule_no]->InIOpS;
}   /* PnUsr_xhif_IO_data_write */

/************************************ ACYCLIC *************************************/


/**
 *  @brief Prepare GPIO for signalling between Ertec and host
 *
 *  @param          void
 *
 *  @return         void
 *
 *  GPIO used as signals for cyclical and acyclical events
 *
 */
PNIO_VOID PnUsr_xhif_gpio_init()
{
    /* GPIOs 0, 2, 6, 8 */
    REG32(U_GPIO__GPIO_PORT_MODE_0_L)   &= ~(0x000F30F3);       /* Default mode*/
    REG32(U_GPIO__GPIO_IOCTRL_0)        &= ~(0x00000101);       /* 0, 8 output */
    REG32(U_GPIO__GPIO_IOCTRL_0)        |= (0x00000044);        /*  2, 6 input */
    REG32(U_GPIO__GPIO_OUT_CLEAR_0)     |= (0x00000101);        /* clear outputs == off */

    /* Allocate semaphore for acyclic XHIF telegrams confirmation */
    OsAllocSemB(&PnioAcycSyncSemId);

    OsCreateThread((void(*)(void))PNIO_XHIF_Write_Task, 0, (PNIO_UINT8*)"Pnio_CycleIO", TASK_PRIO_PNPB, OS_TASK_DEFAULT_STACKSIZE, &Task_Acyc_Write);
    OsCreateMsgQueue(Task_Acyc_Write);
    OsStartThread(Task_Acyc_Write);
    /* Attach interrupt to GPIO change */
    PnUsr_xhif_acyc_set_int();
    PnUsr_xhif_acyc_confirm_set_int();
}   /* PnUsr_xhif_gpio_init */


/**
 *  @brief Signalize outgoing acyclic event to host
 *
 *  @param          void
 *
 *  @return         void
 *
 *  Toggle with GPIO to do so
 *
 */
PNIO_VOID PnUsr_xhif_acyc_gpio_trigger()
{
    /*GPIO 8*/
#define ACYC_SIG_GPIO_POSITION   0x00000100
    if(0x00 == (ACYC_SIG_GPIO_POSITION & (REG32(U_GPIO__GPIO_OUT_0))))
    {
        REG32(U_GPIO__GPIO_OUT_SET_0)     |= (ACYC_SIG_GPIO_POSITION);
    }
    else
    {
        REG32(U_GPIO__GPIO_OUT_CLEAR_0)   |= (ACYC_SIG_GPIO_POSITION);
    }
}   /* PnUsr_xhif_acyc_gpio_trigger */


/**
 *  @brief Signalize cyclic event to host
 *
 *  @param          void
 *
 *  @return         void
 *
 *  Toggle with GPIO to do so
 *
 */
PNIO_VOID PnUsr_xhif_cycl_gpio_trigger()
{
    /*GPIO 2*/
    /*Changed to GPIO 0*/
    if(0x00 == (0x00000001 & (REG32(U_GPIO__GPIO_OUT_0))))
    {
        REG32(U_GPIO__GPIO_OUT_SET_0)     |= (0x00000001);
    }
    else
    {
        REG32(U_GPIO__GPIO_OUT_CLEAR_0)   |= (0x00000001);
    }
}   /* PnUsr_xhif_cycl_gpio_trigger */


/**
 *  @brief Prepare function pointers to be called for received acyclic telegrams
 *
 *  @param          void
 *
 *  @return         void
 *
 *  Also functions for unregistered functions
 *
 */
PNIO_VOID PnUsr_xhif_prepare_function_calls()
{
    PNIO_UINT32 i;
    /* Empty all telegram number reactions */
    for(i = 0; i < PNPB_XHIF_ACYC_NUM_OF_TELEGRAMS; i++)
    {
        pnusr_xhif_acyclic_functions[i].function_call = PnUsr_xhif_no_telegram;
    }

    /* User alarm calls */
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_DIAG_CHANNEL_ADD].function_call = PNIOext_diag_channel_add;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_DIAG_CHANNEL_REMOVE].function_call = PNIOext_diag_channel_remove;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_EXT_DIAG_CHANNEL_ADD].function_call = PNIOext_ext_diag_channel_add;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_EXT_DIAG_CHANNEL_REMOVE].function_call = PNIOext_ext_diag_channel_remove;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_DIAG_GENERIC_ADD].function_call = PNIOext_diag_generic_add;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_DIAG_GENERIC_REMOVE].function_call = PNIOext_diag_generic_remove;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_PROCESS_ALARM_SEND].function_call = PNIOext_process_alarm_send;
	pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_STATUS_ALARM_SEND].function_call = PNIOext_status_alarm_send;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_RET_OF_SUB_ALARM_SEND].function_call = PNIOext_ret_of_sub_alarm_send;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_UPLOAD_RETRIEVAL_ALARM_SEND].function_call = PNIOext_upload_retrieval_alarm_send;

    /* Inicialize, acticate, deactivate, abort */
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_DEVICE_SETUP].function_call = PNIOExt_DeviceSetup;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_DEVICE_START].function_call = PNIOext_device_start;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_DEVICE_STOP].function_call = PNIOext_device_stop;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_DEVICE_AR_ABORT].function_call = PNIOext_device_ar_abort;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_ACTIVATE_IO_DAT_XCH].function_call = PNIOext_ActivateIoDatXch;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_DEACTIVATE_IO_DAT_XCH].function_call = PNIOext_DeactivateIoDatXch;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_SLAVE_REBOOT].function_call = PNIOext_slave_reboot;

    /* Pull/plug submodules */
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_SUB_PLUG_LIST].function_call = PNIOext_sub_plug_list;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_SUB_PLUG].function_call = PNIOext_sub_plug;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_SUB_PULL].function_call = PNIOext_sub_pull;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_SUBSTVAL_OUT_READ_DONE].function_call = PNIOext_substval_out_read_done;

    /* Watchdog feature */
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_HW_WATCHDOG_SET].function_call = PNIOext_hw_watchdog_set;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_HW_WATCHDOG_COMMAND].function_call = PNIOext_hw_watchdog_command;

    /* Change device name , IP and MAC address */
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_STORE_NEW_MAC].function_call = PNIOext_store_new_MAC;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_STORE_NEW_IP].function_call = PNIOext_store_new_IP;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_STORE_NEW_DEVICE_NAME].function_call = PNIOext_store_new_device_name;

    /* Trace functionality */
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_TRACE_COMMAND].function_call = PNIOext_trace_command;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_TRACE_SETTINGS].function_call = PNIOext_trace_settings;

    /* Request responses */
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_REC_READ_RSP].function_call = PNIOext_rec_read_rsp;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_REC_WRITE_RSP].function_call = PNIOext_rec_write_rsp;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_AMR_READ_RSP].function_call = PNIOext_amr_response_handler_rsp;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_PE_RESPONSE_RSP].function_call = PNIOext_pe_response_handler_rsp;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_PE_REQUEST_RSP].function_call = PNIOext_pe_request_handler_rsp;

    /* Non-volatile memory responses */
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_NV_DATA_INIT].function_call = PNIOext_nv_data_init;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_NV_DATA_CLEAR].function_call = PNIOext_nv_data_clear;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_NV_DATA_STORE].function_call = PNIOext_nv_data_store;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_NV_DATA_IM_STORE].function_call = PNIOext_im_data_store;

    /* IM data handling */
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_IM_WRITE_RSP].function_call = PNIOext_im_write_rsp;
    pnusr_xhif_acyclic_functions[PNPB_XHIF_ACYC_IM_READ_RSP].function_call = PNIOext_im_read_rsp;

    /* reset memory interface pointers */
    pnusr_xhif_cyclic_in_services.read_ptr = 0;
    pnusr_xhif_cyclic_in_services.write_ptr = 0;
    pnusr_xhif_cyclic_out_services.read_ptr = 0;
    pnusr_xhif_cyclic_out_services.write_ptr = 0;
    pnusr_xhif_acyc_in_services.read_ptr = 0;
    pnusr_xhif_acyc_in_services.write_ptr = 0;
    pnusr_xhif_acyc_out_services.read_ptr = 0;
    pnusr_xhif_acyc_out_services.write_ptr = 0;

    /*reset lock flags*/
    for(i = 0; i < PNUSR_XHIF_SIZE_OF_BUFFER_FOR_CYCLIC; i++)
    {
        pnusr_xhif_cyclic_in[i].Lock = 0;
        pnusr_xhif_cyclic_out[i].Lock = 0;
    }

    /*reset lock flags*/
    for(i = 0; i < PNUSR_XHIF_SIZE_OF_BUFFER_FOR_ACYC; i++)
    {
        pnusr_xhif_acyc_in[i].lock = 0;
        pnusr_xhif_acyc_out[i].lock = 0;
    }

    /* Reset debug - measurement flags */
    read_waits = 0;
    write_waits = 0;

}   /* PnUsr_xhif_prepare_function_calls */


/**
 *  @brief Frees used resources
 *
 *  @param[in]      *prm            pointer to memory allocated for parameters
 *
 *  @return                         Success[PNIO_OK, PNIO_NOT_OK]
 *
 *  Do after every call with prms != 0
 *
 */
PNIO_UINT32 PnUsr_xhif_cleanup_after_call(PNIO_UINT8 *prm)
{
    if(NULL != prm)
    {
        OsFree(prm);
    }
    return PNIO_OK;
}   /* PnUsr_xhif_cleanup_after_call */


/**
 *  @brief Calls function registered for received telegram
 *
 *  @param[in]      id              ID of called function
 *  @param[in]      *prm            Pointer to memory used for parameters
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *
 *
 */

PNIO_VOID PnUsr_xhif_call_function(PNPB_XHIF_ACYC_TELEGRAMS id, PNIO_UINT8 *prm)
{
    if(PNPB_XHIF_ACYC_NUM_OF_TELEGRAMS < id)
    {
        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nError: Recieved invalid record ID\n");
    }
    if(PNPB_XHIF_ACYC_FIRST_HOST_ERTEC_TELEGRAM > id)
    {
        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nError: Recieved invalid record ID\n");
    }
    pnusr_xhif_acyclic_functions[id].function_call(prm);
    /* Function was executed, parameters are not needed any more */
    PnUsr_xhif_cleanup_after_call(prm);
}   /* PnUsr_xhif_call_function */


/**
 *  @brief Lock against read while write
 *
 *  @param[in]      void
 *  @return         void
 *
 *  Two devices accesses same memory
 *
 */
PNIO_VOID PnUsr_xhif_acyc_out_trylock(PNIO_UINT32 tmp_write_ptr)
{
    if(0 == pnusr_xhif_acyc_out[tmp_write_ptr].lock)
    {
        pnusr_xhif_acyc_out[tmp_write_ptr].lock = 0x01;
    }
    else
    {
        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nAcyclic lock failed\n");
    }
}   /* PnUsr_xhif_acyc_trylock */


/**
 *  @brief Unlock for protection against read while write
 *
 *  @param[in]      void
 *  @return         void
 *
 *  Two devices accesses same memory
 *
 */
PNIO_VOID PnUsr_xhif_acyc_out_unlock(PNIO_UINT32 tmp_write_ptr)
{
    pnusr_xhif_acyc_out[tmp_write_ptr].lock = 0x00;
}   /* PnUsr_xhif_acyc_unlock */


/**
 *  @brief Lock against read while write
 *
 *  @param[in]      void
 *  @return         void
 *
 *  Two devices accesses same memory
 *
 */
PNIO_VOID PnUsr_xhif_acyc_in_trylock(PNIO_UINT32 tmp_read_ptr)
{
    if(0 == pnusr_xhif_acyc_in[tmp_read_ptr].lock)
    {
#if(1 == LOCK_ALSO_FOR_READING)
        pnusr_xhif_acyc_in[tmp_read_ptr].lock = 0x01;
#endif
    }
    else
    {
        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nAcyclic lock failed\n");
    }

}   /* PnUsr_xhif_acyc_trylock */


/**
 *  @brief Unlock for protection against read while write
 *
 *  @param[in]      void
 *  @return         void
 *
 *  Two devices accesses same memory
 *
 */
PNIO_VOID PnUsr_xhif_acyc_in_unlock(PNIO_UINT32 tmp_read_ptr)
{
#if(1 == LOCK_ALSO_FOR_READING)
            pnusr_xhif_acyc_in[tmp_read_ptr].lock = 0x00;
#endif
}   /* PnUsr_xhif_acyc_unlock */


/**
 *  @brief Check against acyclic buffer underflow
 *
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Do before every read
 *
 */
PNIO_UINT32 PnUsr_xhif_check_acyc_read_pointer(PNIO_UINT32 *p_read_ptr)
{
    PNIO_UINT32 tmp_read_ptr, tmp_write_ptr;
    PNIO_UINT32 i, status = PNIO_NOT_OK;
    /* Read */
    tmp_read_ptr = pnusr_xhif_acyc_in_services.read_ptr;
    tmp_write_ptr = pnusr_xhif_acyc_in_services.write_ptr;

    /* Check against read from invalid indexes */
    if(PNUSR_XHIF_SIZE_OF_BUFFER_FOR_ACYC <= tmp_read_ptr)
    {
        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nInvalid acyclic read index\n");
    }

    /* Check against buffer underflow */
    for (i = 0; i < 2; i++)
    {
        if (tmp_read_ptr != tmp_write_ptr)
        {
            status = PNIO_OK;
            break;
        }

        PnUsr_xhif_wait();

        tmp_read_ptr = pnusr_xhif_acyc_in_services.read_ptr;
    }

    *p_read_ptr = tmp_read_ptr;
    return status;

}   /* PnUsr_xhif_check_acyc_read_pointer */


/**
 *  @brief Increment acyclic buffer ptr
 *
 *  @return         void
 *
 *  Do after every read
 *
 */
PNIO_VOID PnUsr_xhif_increment_acyc_read_pointer(PNIO_UINT32 *p_read_ptr)
{
    PNIO_UINT32 tmp_read_ptr = *p_read_ptr;
    /* Increment */
    if((PNUSR_XHIF_SIZE_OF_BUFFER_FOR_ACYC - 1) <= tmp_read_ptr)
    {
        tmp_read_ptr = 0;    /* buffer went over max to 0*/
    }
    else
    {
        tmp_read_ptr++;
    }
    /* Save new value */
    *p_read_ptr = tmp_read_ptr;

}   /* PnUsr_xhif_increment_acyc_read_pointer */


/**
 *  @brief Check against acyclic buffer overflow
 *
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Do before every write
 *  Also increments pointer
 *
 */
PNIO_UINT32 PnUsr_xhif_check_acyc_write_pointer(PNIO_UINT32 *p_write_ptr)
{
    PNIO_UINT32 tmp_write_ptr, tmp_read_ptr;
    /* return write + 1 but do not increment yet */

    /* read */
    tmp_write_ptr = pnusr_xhif_acyc_out_services.write_ptr;
    tmp_read_ptr = pnusr_xhif_acyc_out_services.read_ptr;

    /* Check */
    if(PNUSR_XHIF_SIZE_OF_BUFFER_FOR_ACYC <= tmp_write_ptr)
    {
        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nError: invalid acyclic write index\n");
    }

    /* Increment */
    if((PNUSR_XHIF_SIZE_OF_BUFFER_FOR_ACYC - 1) <= tmp_write_ptr)
    {
        tmp_write_ptr = 0;    /* buffer went over max to 0*/
    }
    else
    {
        tmp_write_ptr++;
    }

    if(tmp_write_ptr == tmp_read_ptr)
    {
        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nAcyclical buffer overflow\n");
    }

    /* Return */
    *p_write_ptr = tmp_write_ptr;
    return PNPB_OK;
}   /* PnUsr_xhif_check_acyc_write_pointer */


/**
 *  @brief Increment number of records in acyclic buffer
 *
 *  @return         void
 *
 *  Do after every read
 *  Also signals to slave
 *
 */
PNIO_VOID PnUsr_xhif_increment_acyc_write_pointer(PNIO_UINT32 tmp_write_ptr)
{
    pnusr_xhif_acyc_out_services.write_ptr = tmp_write_ptr;

}   /* PnUsr_xhif_increment_acyc_write_pointer */


/**
 *  @brief Returns id of "more follows" to id in parameter
 *
 *  @param[in]      id              ID of called function
 *  @return                         ID of more follows of called function
 *
 *  For telegrams longer than 1024B
 *
 */
PNPB_XHIF_ACYC_TELEGRAMS PnUsr_xhif_acyc_has_more_follows(PNPB_XHIF_ACYC_TELEGRAMS id)
{
    switch (id)
    {
        case PNPB_XHIF_ACYC_NO_TELEGRAM:
            return PNPB_XHIF_ACYC_NO_TELEGRAM;

        case PNPB_XHIF_ACYC_AR_OWNERSHIP_IND:
            return PNPB_XHIF_ACYC_AR_OWNERSHIP_IND_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_REC_READ:
            return PNPB_XHIF_ACYC_REC_READ_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_REC_WRITE:
            return PNPB_XHIF_ACYC_REC_WRITE_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_PE_REQUEST:
            return PNPB_XHIF_ACYC_PE_REQUEST_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_NV_DATA_SYNC:
            return PNPB_XHIF_ACYC_NV_DATA_SYNC_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_STORE_REMA_MEM:
            return PNPB_XHIF_ACYC_STORE_REMA_MEM_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_IM_WRITE:
            return PNPB_XHIF_ACYC_IM_WRITE_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_IM_STORE:
        	return PNPB_XHIF_ACYC_IM_STORE_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_NV_DATA_STORE_HOST:
           	return 	PNPB_XHIF_ACYC_NV_DATA_STORE_HOST_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_NV_DATA_SET_DEFAULT:
            return  PNPB_XHIF_ACYC_NV_DATA_SET_DEFAULT_MORE_FOLLOWS;

        default:
            return PNPB_XHIF_ACYC_NO_TELEGRAM;
    }
}   /* PnUsr_xhif_acyc_has_more_follows */


/**
 *  @brief Identifies "more follows" based on id
 *
 *  @param[in]      id              ID of called function
 *  @return                         Is more follows [PNPB_TRUE, PNPB_FALSE]
 *
 *  For telegrams longer than 1024B
 *
 */
PNIO_UINT32 PnUsr_xhif_acyc_is_more_follows(PNPB_XHIF_ACYC_TELEGRAMS id)
{
    switch (id)
    {
        case PNPB_XHIF_ACYC_REC_READ_RSP_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_REC_WRITE_RSP_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_AMR_READ_RSP_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_PE_RESPONSE_RSP_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_NV_DATA_INIT_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_NV_DATA_STORE_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_NV_DATA_IM_STORE_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_IM_READ_RSP_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_DEVICE_SETUP_MORE_FOLLOWS:
            return PNIO_TRUE;
        default:
            return PNIO_FALSE;
    }
}   /* PnUsr_xhif_acyc_is_more_follows */


/**
 *  @brief Write acyclic telegram to XHIF memory interface
 *
 *  @param[in]      id              ID of called function
 *  @param[in]      prm_len         Length of function parameters
 *  @param[in]      *p_prm          Pointer to called function parameters
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  For telegrams longer than 1024B
 *
 */
PNIO_UINT32 PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_TELEGRAMS id, PNIO_UINT16 prm_len, PNIO_UINT8 *p_prm)
{
    PNIO_ACYC_WRITE_MSG * WriteEvent;

    OsAlloc ((void**) &WriteEvent, 0, sizeof (PNIO_ACYC_WRITE_MSG));
    WriteEvent->id   = id;
    WriteEvent->prm_len = prm_len;
    WriteEvent->p_prm = p_prm;
    OsSendMessage (Task_Acyc_Write, WriteEvent, OS_MBX_PRIO_NORM);
    return PNIO_OK;
}   /* PnUsr_xhif_acyc_write */


/**
 *  @brief Write acyclic telegram to XHIF memory interface internally
 *
 *  @param[in]      id              ID of called function
 *  @param[in]      prm_len         Length of function parameters
 *  @param[in]      *p_prm          Pointer to called function parameters
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  For telegrams longer than 1024B
 *
 */
PNIO_VOID PnUsr_xhif_acyc_write_internal(PNPB_XHIF_ACYC_TELEGRAMS id, PNIO_UINT16 prm_len_tot, PNIO_UINT16 prm_len, PNIO_UINT8 *p_prm)
{
    PNIO_UINT32 write_ptr;
	
    /* Check send ready */
    if(PNIO_NOT_OK == PnUsr_xhif_check_acyc_write_pointer(&write_ptr))
    {
       /* error */
        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nSend not ready\n");
    }
    PnUsr_xhif_acyc_out_trylock(write_ptr);

    pnusr_xhif_acyc_out[write_ptr].id = id;
    pnusr_xhif_acyc_out[write_ptr].data_len = prm_len_tot;
    if((0 < prm_len) && (PNUSR_XHIF_ACYCLIC_DATA_MAX >= prm_len) && (NULL != p_prm))
    {
        OsMemCpy((void*)pnusr_xhif_acyc_out[write_ptr].data, p_prm, prm_len);
    }
    else
    {
        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nInvalid prm len\n");
    }

    PnUsr_xhif_acyc_out_unlock(write_ptr);
    PnUsr_xhif_increment_acyc_write_pointer(write_ptr);

    /* Signalize new event via GPIO signal */
    PnUsr_xhif_acyc_gpio_trigger();

    /* Wait for confirmation - do not send new acyclic message if previous was not handled */
    OsTakeSemB(PnioAcycSyncSemId);
}   /* PnUsr_xhif_acyc_write_internal */


/**
 *  @brief Write acyclic telegram to XHIF memory interface internally
 *
 *  @param[in]      id              ID of called function
 *  @param[in]      prm_len         Length of function parameters
 *  @param[in]      *p_prm          Pointer to called function parameters
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  For telegrams longer than 1024B
 *
 */
PNIO_VOID PnUsr_xhif_acyc_write_task(PNPB_XHIF_ACYC_TELEGRAMS id, PNIO_UINT16 prm_len, PNIO_UINT8 *p_prm)
{
    PNIO_UINT16 send_prm_len, send_id, remaining;
    PNIO_UINT8 *p_prm_send;
    PNIO_UINT8 *p_prm_send_next;

    remaining = prm_len;
    p_prm_send_next = p_prm;

    do
    {
        p_prm_send = p_prm_send_next;

        /* Can be transfered in one shot or last iteration of non_one_shot */
        if((PNUSR_XHIF_ACYCLIC_DATA_MAX) > remaining)
        {
            send_prm_len = remaining;
            remaining = 0;
            send_id = (PNIO_UINT16)id;
        }
        else
        {
            send_id = PnUsr_xhif_acyc_has_more_follows(id);
            remaining -= PNUSR_XHIF_ACYCLIC_DATA_MAX;
            send_prm_len = PNUSR_XHIF_ACYCLIC_DATA_MAX;
            p_prm_send_next += PNUSR_XHIF_ACYCLIC_DATA_MAX;
        }
        /*send*/
        PnUsr_xhif_acyc_write_internal(send_id, prm_len, send_prm_len, p_prm_send);

    } while(0 != remaining);

    if(NULL != p_prm)
    {
        OsFree(p_prm);
    }
}   /* PnUsr_xhif_acyc_write */


/**
 *  @brief Thread for putting acyclic telegram to XHIF memory interface
 *
 *  @param          void
 *  @return
 *
 *  Also waits for confirmation from host
 *
 */
PNIO_VOID PNIO_XHIF_Write_Task()
{
    PNIO_VOID* pData = LSA_NULL;
    PNIO_UINT32 taskID;

    OsWaitOnEnable();
    taskID = OsGetThreadId();

    while(1)
    {
        OsReadMessageBlocked ((void**)&pData, taskID);

        if (pData == 0)
        {
            PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nError in Task_Pnpb_Post()\n");
        }

        PNIO_ACYC_WRITE_MSG * WriteEvent = ( PNIO_ACYC_WRITE_MSG * ) pData;
        PnUsr_xhif_acyc_write_task(WriteEvent->id, WriteEvent->prm_len, (PNIO_UINT8 *) WriteEvent->p_prm);

        OsFree( pData );
    }
}   /* PNIO_XHIF_Write_Task */

/**
 *  @brief Read telegram longer than 1024B - distributed over several telegrams
 *
 *  @param[in]      id              ID of called function
 *  @param[in]      is_more_follows If not more follows, then last iteration of longer telegram
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Identifies in which part of telegram we are (Start, end, middle) and reacts accordingly
 *  First part have to create memory space for function parameters, others will append data to here
 *  After last part, the funciton have to be executed
 *
 */
PNIO_VOID PnUsr_xhif_acyc_read_continue(PNIO_UINT32 id,
                                        PNIO_UINT32 is_more_follows,
                                        PNIO_UINT32 tmp_read_ptr)
{
    PNIO_UINT32 incoming_size;
	
    /* First call of continual read */
    if(PNIO_FALSE == pnusr_xhif_acyclic_recieve_cont_manage.more_follows)
    {
        /* Read size of incoming telegram */
        incoming_size = pnusr_xhif_acyc_in[tmp_read_ptr].data_len;


        if(0 > (incoming_size - PNUSR_XHIF_ACYCLIC_DATA_MAX))
        {
            PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nTrying to read too many bytes - this should not be more follows\n");
        }

        /* Allocate memory for newly received telegram */
        OsAllocF((void**)&pnusr_xhif_acyclic_recieve_cont_manage.p_start_data, incoming_size);

        OsMemCpy((void*)pnusr_xhif_acyclic_recieve_cont_manage.p_start_data,
                (const void*)pnusr_xhif_acyc_in[tmp_read_ptr].data,
                PNUSR_XHIF_ACYCLIC_DATA_MAX);
		
        pnusr_xhif_acyclic_recieve_cont_manage.more_follows = PNIO_TRUE;
        pnusr_xhif_acyclic_recieve_cont_manage.p_write_data =
                pnusr_xhif_acyclic_recieve_cont_manage.p_start_data + PNUSR_XHIF_ACYCLIC_DATA_MAX;
        pnusr_xhif_acyclic_recieve_cont_manage.data_remains = incoming_size - PNUSR_XHIF_ACYCLIC_DATA_MAX;
    }

    else
    {

        /* Last call of continuous read */
        if(PNIO_FALSE == is_more_follows)
        {
            if(0 >= pnusr_xhif_acyclic_recieve_cont_manage.data_remains)
            {
                PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nError: Trying to read zero bytes\n");
            }

            if(PNUSR_XHIF_ACYCLIC_DATA_MAX < pnusr_xhif_acyclic_recieve_cont_manage.data_remains)
            {
                PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nError: Continuous read finished, but there are data remaining\n");
            }


            // Below loop is used instead of OsMemCpy to keep volitile qualifier happy 
            OsMemCpy((void*)pnusr_xhif_acyclic_recieve_cont_manage.p_write_data,
                    (const void*)pnusr_xhif_acyc_in[tmp_read_ptr].data,
                    pnusr_xhif_acyclic_recieve_cont_manage.data_remains);

		
            PnUsr_xhif_call_function(id, pnusr_xhif_acyclic_recieve_cont_manage.p_start_data);
            pnusr_xhif_acyclic_recieve_cont_manage.more_follows = PNIO_FALSE;
            pnusr_xhif_acyclic_recieve_cont_manage.data_remains = 0;

        }
        else
        {
            if(0 >= (pnusr_xhif_acyclic_recieve_cont_manage.data_remains - PNUSR_XHIF_ACYCLIC_DATA_MAX))
            {
                PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nError: Trying to read too much data\n");
            }
            OsMemCpy((void*)pnusr_xhif_acyclic_recieve_cont_manage.p_write_data,
                    (const void*)pnusr_xhif_acyc_in[tmp_read_ptr].data,
                    PNUSR_XHIF_ACYCLIC_DATA_MAX);

            pnusr_xhif_acyclic_recieve_cont_manage.p_write_data += PNUSR_XHIF_ACYCLIC_DATA_MAX;
            pnusr_xhif_acyclic_recieve_cont_manage.data_remains -= PNUSR_XHIF_ACYCLIC_DATA_MAX;
        }
    }
}   /* PnUsr_xhif_acyc_read_continue */


/**
 *  @brief Read telegram of maximally 1024B
 *
 *  @param[in]      id              ID of called function
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Also allocates buffer for function parameters and executes the function
 *
 */
PNIO_VOID PnUsr_xhif_acyc_read_telegram(PNIO_UINT32 id, PNIO_UINT32 tmp_read_ptr)
{
    PNIO_UINT32 incoming_size;
    PNIO_UINT8 *parameters;

    /* Read size of incoming telegram */
    incoming_size = pnusr_xhif_acyc_in[tmp_read_ptr].data_len;

    if(0 != incoming_size)
    {
        /* Allocate memory for newly received telegram */
        OsAllocF((void**)&parameters, incoming_size);

        // Below loop is used instead of OsMemCpy to keep volitile qualifier happy 
        OsMemCpy(parameters, (const void*)pnusr_xhif_acyc_in[tmp_read_ptr].data, incoming_size);

    }
    else
    {
        parameters = NULL;
    }

    PnUsr_xhif_call_function(id, parameters);
}   /* PnUsr_xhif_acyc_read_telegram */


PNIO_VOID PnUsr_xhif_wait( PNIO_VOID )
{
    volatile int i = 0;

    for ( i = 0 ; i < 500 ; i++);
}

/**
 *  @brief Receives acyclic telegram
 *
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Decide if function parameters received are in one telegram or divided into more,
 *  process accordingly.
 *
 */
PNIO_VOID PnUsr_xhif_acyc_read()
{
    PNIO_UINT32 tmp_read_ptr;
    PNIO_UINT32 is_more_follows;
    PNPB_XHIF_ACYC_TELEGRAMS incoming_id;
    if(PNIO_NOT_OK == PnUsr_xhif_check_acyc_read_pointer(&tmp_read_ptr))
    {
        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "\nACyclical read not ready\n");
        return;
    }
    PnUsr_xhif_increment_acyc_read_pointer(&tmp_read_ptr);
    PnUsr_xhif_acyc_in_trylock(tmp_read_ptr);
    /* Decide if more follows */
    incoming_id = pnusr_xhif_acyc_in[tmp_read_ptr].id;
    is_more_follows = PnUsr_xhif_acyc_is_more_follows(incoming_id);

    if((PNIO_TRUE == is_more_follows) || (PNIO_TRUE == pnusr_xhif_acyclic_recieve_cont_manage.more_follows))
    {
        PnUsr_xhif_acyc_read_continue(incoming_id, is_more_follows, tmp_read_ptr);
    }
    else
    {
        PnUsr_xhif_acyc_read_telegram(incoming_id, tmp_read_ptr);
    }
    PnUsr_xhif_acyc_in_unlock(tmp_read_ptr);
    pnusr_xhif_acyc_in_services.read_ptr = tmp_read_ptr;

}   /* PnUsr_xhif_acyc_read */

/************************ FUNCTIONS CALLED BY ACYCLIC TELEGRAM RECIEVAL **************************/

/**
 *  @brief No function set to process this telegram
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Error handling
 *
 */
PNIO_VOID PnUsr_xhif_no_telegram(PNIO_UINT8* params)
{
    PNIO_UNUSED_ARG(params);
    PNIO_printf("Invalid telegram number\n");
}   /* PnUsr_xhif_no_telegram */

/*
 * 	User alarm calls
 */

PNIO_VOID PNIOext_async_req_error ( PNIO_UINT32 type, PNIO_UINT32 error )
{
    PNIO_ASYNC_ERROR_PRM *params;

    OsAlloc((void**)&params, 0, sizeof(PNIO_ASYNC_ERROR_PRM));

    params->Type = type;
    params->ErrorCode = error;

    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_ASYNC_REQUEST_ERROR, sizeof(PNIO_ASYNC_ERROR_PRM), (PNIO_UINT8*)params);
}

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Provide standard channel diagnostic, send alarm appears
 *
 */
PNIO_VOID PNIOext_diag_channel_add(PNIO_UINT8* params)
{
	PNIO_UINT32 Status = PNIO_OK;
	PNIO_DIAG_CHANNEL_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_DIAG_CHANNEL_PRM*) params;

    /* Do functionality */
    PNIO_printf("Channel diagnostic, channel %d, err num %d, chan dir %d, chan type %d\n", prm->StdParams.ChannelNum,
    		prm->StdParams.ErrorNum, prm->StdParams.ChanDir, prm->StdParams.ChanTyp);

	// *--------------------------------------------------
	// *  new alarm appears
	// *---------------------------------------
	// *  First make a diagnostic entry for this
	// *  subslot. So in the following alarm PDU
	// *  the ChannelDiagExist - bit in the
	// *  alarm specifier is set and the IO
	// *  controller ist notified, that
	// *  diagnostic data are available.
	// *---------------------------------------
    Status = pnpb_diag_channel_add(
								PNIO_SINGLE_DEVICE_HNDL,	// device handle
								PNIO_DEFAULT_API,			// application process identifier
								&prm->StdParams.Addr,		// geographical or logical address
								prm->StdParams.ChannelNum,	// channel number
								prm->StdParams.ErrorNum,	// error numberg
								prm->StdParams.ChanDir,		// channel error dir
								prm->StdParams.ChanTyp,		// channel error type
								prm->MaintReq,				// maintenance required
								prm->MaintDem,				// maintenance demanded
								prm->DiagTag);				// user defined diag tag != 0

    /* if there are no active ARs, just send acknowledgement to user */
    if( 0 == NumOfAr )
    {
    	PNIO_cbf_async_req_done( PNIO_SINGLE_DEVICE_HNDL, 0, PNIO_ALM_CHAN_DIAG,
    			PNIO_DEFAULT_API, &prm->StdParams.Addr, 0, prm->DiagTag );
    }

    if (Status != PNIO_OK)
    {
        PNIOext_async_req_error(PNIO_ALM_CHAN_DIAG, PNIO_get_last_error ());
    	//PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
    }
} /* PNIOext_diag_channel_add */

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Provide standard channel diagnostic, send alarm disappears
 *
 */
PNIO_VOID PNIOext_diag_channel_remove(PNIO_UINT8* params)
{
	PNIO_UINT32 Status = PNIO_OK;
	PNIO_DIAG_CHANNEL_REMOVE_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_DIAG_CHANNEL_REMOVE_PRM*) params;

    /* Do functionality */
    PNIO_printf("Channel diagnostic remove, channel %d, err num %d, chan dir %d, chan type %d\n", prm->StdParams.ChannelNum,
    		prm->StdParams.ErrorNum, prm->StdParams.ChanDir, prm->StdParams.ChanTyp);

    if ((prm->AlarmState != DIAG_CHANPROP_SPEC_ERR_DISAPP)
    		&& (prm->AlarmState != DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE))
    {
    	PNPB_API_TRACE_01(LSA_TRACE_LEVEL_ERROR,  "invalid param AlarmState = %d\n", prm->AlarmState);
        return;
    }

	// *--------------------------------------------------
	// *   alarm disappears
	// *--------------------------------------------------
    Status = pnpb_diag_channel_remove (
						PNIO_SINGLE_DEVICE_HNDL,		// device handle
						PNIO_DEFAULT_API,				// application process identifier
						&prm->StdParams.Addr,			// geographical or logical address
						prm->StdParams.ChannelNum,		// channel number
						prm->StdParams.ErrorNum,		// error numberg
						prm->StdParams.ChanDir,			// channel error dir
						prm->StdParams.ChanTyp,			// channel error type
						prm->DiagTag,					// user defined diag tag != 0
						prm->AlarmState);				// DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE

    /* if there are no active ARs, just send acknowledgement to user */
    if( 0 == NumOfAr )
    {
    	PNIO_cbf_async_req_done( PNIO_SINGLE_DEVICE_HNDL, 0, PNIO_ALM_CHAN_DIAG,
    			PNIO_DEFAULT_API, &prm->StdParams.Addr, 0, prm->DiagTag );
    }

    if (Status != PNIO_OK)
    {
        PNIOext_async_req_error(PNIO_ALM_CHAN_DIAG, PNIO_get_last_error ());
    	//PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
    }
} /* PNIOext_diag_channel_remove */

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Provide extended channel diagnostic, alarm appears
 *
 */
PNIO_VOID PNIOext_ext_diag_channel_add(PNIO_UINT8* params)
{
	PNIO_UINT32 Status = PNIO_OK;
	PNIO_EXT_DIAG_CHANNEL_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_EXT_DIAG_CHANNEL_PRM*) params;

    /* Do functionality */
    PNIO_printf("Extended diagnostic, channel %d, err num %d, chan dir %d, chan type %d\n", prm->StdParams.ChannelNum,
    		prm->StdParams.ErrorNum, prm->StdParams.ChanDir, prm->StdParams.ChanTyp);

    // *--------------------------------------------------
	// *   new alarm appears
	// *---------------------------------------
	// *  First make a diagnostic entry for this
	// *  subslot. So in the following alarm PDU
	// *  the ChannelDiagExist - bit in the
	// *  alarm specifier is set and the IO
	// *  controller ist notified, that
	// *  diagnostic data are available.
	// *---------------------------------------
    Status = pnpb_ext_diag_channel_add (
						PNIO_SINGLE_DEVICE_HNDL,	// device handle
						PNIO_DEFAULT_API,			// application process identifier
						&prm->StdParams.Addr,		// geographical or logical address
						prm->StdParams.ChannelNum,	// channel number
						prm->StdParams.ErrorNum,	// error numberg
						prm->StdParams.ChanDir,		// channel error type
						prm->StdParams.ChanTyp,		// channel error type
						prm->ExtChannelErrType,	  	// channel error type           (see PNIO spec.)
						prm->ExtChannelAddValue,  	// extended channel add. value  (see PNIO spec.)
						prm->MaintReq,				// maintenance required
						prm->MaintDem,				// maintenance demanded
						prm->DiagTag);  	      	// user defined diag tag != 0

	/* if there are no active ARs, just send acknowledgement to user */
	if( 0 == NumOfAr )
	{
		PNIO_cbf_async_req_done( PNIO_SINGLE_DEVICE_HNDL, 0, PNIO_ALM_EXT_CHAN_DIAG,
				PNIO_DEFAULT_API, &prm->StdParams.Addr, 0, prm->DiagTag );
	}

    if (Status != PNIO_OK)
    {
        PNIOext_async_req_error(PNIO_ALM_EXT_CHAN_DIAG, PNIO_get_last_error ());
    	//PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
    }
} /* PNIOext_ext_diag_channel_add */

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Provide extended channel diagnostic, alarm disappears
 *
 */
PNIO_VOID PNIOext_ext_diag_channel_remove(PNIO_UINT8* params)
{
	PNIO_UINT32 Status = PNIO_OK;
	PNIO_EXT_DIAG_CHANNEL_REMOVE_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_EXT_DIAG_CHANNEL_REMOVE_PRM*) params;

    /* Do functionality */
    PNIO_printf("Extended diagnostic remove, channel %d, err num %d, chan dir %d, chan type %d\n", prm->StdParams.ChannelNum,
    		prm->StdParams.ErrorNum, prm->StdParams.ChanDir, prm->StdParams.ChanTyp);

	if ((prm->AlarmState != DIAG_CHANPROP_SPEC_ERR_DISAPP)
			&& (prm->AlarmState != DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE))
	{
		PNPB_API_TRACE_01(LSA_TRACE_LEVEL_ERROR,  "invalid param AlarmState = %d\n", prm->AlarmState);
		return;
	}

	// *--------------------------------------------------
	// *   alarm disappears
	// *--------------------------------------------------
    Status = pnpb_ext_diag_channel_remove(
                        PNIO_SINGLE_DEVICE_HNDL,	// device handle
                        PNIO_DEFAULT_API,			// application process identifier
						&prm->StdParams.Addr,		// geographical or logical address
						prm->StdParams.ChannelNum,	// channel number
						prm->StdParams.ErrorNum,	// error number, see PNIO specification coding of "ChannelErrorType"
						prm->StdParams.ChanDir,		// channel error type
						prm->StdParams.ChanTyp,		// channel error type
						prm->ExtChannelErrType,     // channel error type           (see PNIO spec.)
						prm->ExtChannelAddValue,    // extended channel add. value  (see PNIO spec.)
						prm->DiagTag,		        // user defined diag tag != 0
						prm->AlarmState);           // DIAG_CHANPROP_SPEC_ERR_DISAPP, DIAG_CHANPROP_SPEC_ERR_DISAPP_MORE

	/* if there are no active ARs, just send acknowledgement to user */
	if( 0 == NumOfAr )
	{
		PNIO_cbf_async_req_done( PNIO_SINGLE_DEVICE_HNDL, 0, PNIO_ALM_EXT_CHAN_DIAG,
				PNIO_DEFAULT_API, &prm->StdParams.Addr, 0, prm->DiagTag );
	}

    if (Status != PNIO_OK)
    {
        PNIOext_async_req_error(PNIO_ALM_EXT_CHAN_DIAG, PNIO_get_last_error ());
    	//PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
    }
} /* PNIOext_ext_diag_channel_remove */

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Send generic diagnostic, alarm appears
 *
 */
PNIO_VOID PNIOext_diag_generic_add(PNIO_UINT8* params)
{
	PNIO_UINT32 Status = PNIO_OK;
	PNIO_DIAG_GENERIC_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_DIAG_GENERIC_PRM*) params;

    /* Do functionality */
    PNIO_printf("Generic diagnostic, channel %d, chan dir %d, chan type %d\n", prm->StdParams.ChannelNum,
    		prm->StdParams.ChanDir, prm->StdParams.ChanTyp);

    /* geographic addressing (slotnumber, subslot-number) */
    prm->StdParams.Addr.Type = PNIO_ADDR_GEO;

	// *--------------------------------------------------
	// *   alarm appears
	// *--------------------------------------------------
	// *      send generic diagnostic
	// *      alarm appears
	// *      Note: only one diagnosis alarm at a time is possible
	// *      Wait before sending a new alarm,
	// *      until the previous one has been confirmed.
	// *-----------------------------------------------
    Status = pnpb_diag_generic_add (
								PNIO_SINGLE_DEVICE_HNDL,	// device handle
								PNIO_DEFAULT_API,			// application process identifier
								&prm->StdParams.Addr,		// geographical or logical address
								prm->StdParams.ChannelNum,	// channel number
								prm->StdParams.ChanDir,		// channel direction (in, out, in/out)
								prm->StdParams.ChanTyp,		// channel type (BYTE, WORD,..)
								prm->StdParams.DiagTag,		// user defined diag tag != 0
								prm->StdParams.UserStructIdent,	// structure of info data (see IEC 61158)
								&prm->InfoData[0],			// diag data
								prm->InfoDataLen,			// length of diag data in bytes
								prm->MaintReq,				// maintenance required
								prm->MaintDem);				// maintenance demanded

	/* if there are no active ARs, just send acknowledgement to user */
	if( 0 == NumOfAr )
	{
		PNIO_cbf_async_req_done( PNIO_SINGLE_DEVICE_HNDL, 0, PNIO_ALM_GEN_DIAG,
				PNIO_DEFAULT_API, &prm->StdParams.Addr, 0, prm->StdParams.DiagTag );
	}

    if (Status != PNIO_OK)
    {
        PNIOext_async_req_error(PNIO_ALM_GEN_DIAG, PNIO_get_last_error ());
    	//PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
    }
} /* PNIOext_diag_generic_add */

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Send generic diagnostic, alarm disappears
 *
 */
PNIO_VOID PNIOext_diag_generic_remove(PNIO_UINT8* params)
{
	PNIO_UINT32 Status = PNIO_OK;
	PNIO_DIAG_GENERIC_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_DIAG_GENERIC_PRM*) params;

    /* Do functionality */
    PNIO_printf("Generic diagnostic remove, channel %d, chan dir %d, chan type %d\n", prm->StdParams.ChannelNum,
    		prm->StdParams.ChanDir, prm->StdParams.ChanTyp);

    /* geographic addressing (slotnumber, subslot-number) */
    prm->StdParams.Addr.Type = PNIO_ADDR_GEO;

	// *--------------------------------------------------
	// *   alarm disappears
	// *-----------------------------------------------
    Status = pnpb_diag_generic_remove (
						PNIO_SINGLE_DEVICE_HNDL,			// device handle
						PNIO_DEFAULT_API,					// application process identifier
						&prm->StdParams.Addr,				// geographical or logical address
						prm->StdParams.ChannelNum,			// channel number
						prm->StdParams.ChanDir,				// channel direction (in, out, in/out)
						prm->StdParams.ChanTyp,				// channel type (BYTE, WORD,..)
						prm->StdParams.DiagTag,				// user defined diag tag != 0
						prm->StdParams.UserStructIdent);	// structure of info data (see IEC 61158)

	/* if there are no active ARs, just send acknowledgement to user */
	if( 0 == NumOfAr )
	{
		PNIO_cbf_async_req_done( PNIO_SINGLE_DEVICE_HNDL, 0, PNIO_ALM_GEN_DIAG,
				PNIO_DEFAULT_API, &prm->StdParams.Addr, 0, prm->StdParams.DiagTag );
	}

    if (Status != PNIO_OK)
    {
        PNIOext_async_req_error(PNIO_ALM_GEN_DIAG, PNIO_get_last_error ());
    	//PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
    }
} /* PNIOext_diag_generic_remove */

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Send process alarms
 *
 */
PNIO_VOID PNIOext_process_alarm_send(PNIO_UINT8* params)
{
	PNIO_UINT32 Status = PNIO_OK;
	PNIO_UINT32 i;
	PNIO_UINT32 Num_Of_Ar;
	PNIO_PROCESS_ALARM_SET_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_PROCESS_ALARM_SET_PRM*) params;

    /* Do functionality */

    Status = pnpb_process_alarm_send(
    				PNIO_SINGLE_DEVICE_HNDL,	// device handle
					PNIO_DEFAULT_API,			// application process identifier
					&prm->Addr,					// geographical or logical address
					&prm->Data[0],				// AlarmItem.Data
					prm->DataLen,				// length of AlarmItem.Data
					prm->UserStructIdent,		// AlarmItem.UserStructureIdentifier, s. IEC61158-6
					prm->UserHndl);				// user defined handle

    /* if there are no active ARs, just send acknowledgement to user */
    Num_Of_Ar = 0;
    for( i = 0; i < IOD_CFG_NUMOF_AR; i++ )
	{
    	if( PNPB_AR_OFFLINE != Pnpb.ArState[ i ] )
    	{
    		Num_Of_Ar++;
    	}
	}

    if( 0 == Num_Of_Ar )
    {
    	PNIO_cbf_async_req_done( PNIO_SINGLE_DEVICE_HNDL, 0, PNIO_ALM_PROC, PNIO_DEFAULT_API, &prm->Addr, 0, prm->UserHndl );
    }

    if (Status != PNIO_OK)
    {
        PNIOext_async_req_error(PNIO_ALM_PROC, PNIO_get_last_error ());
    	//PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
    }
}   /* PNIOext_process_alarm_send */


/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Send process alarms
 *
 */
PNIO_VOID PNIOext_status_alarm_send(PNIO_UINT8* params)
{
	PNIO_UINT32 Status = PNIO_OK;
	PNIO_UINT32 i;
	PNIO_UINT32 Num_Of_Ar;
	PNIO_STATUS_ALARM_SET_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_STATUS_ALARM_SET_PRM*) params;

    /* Do functionality */

    Status = pnpb_status_alarm_send(
    				PNIO_SINGLE_DEVICE_HNDL,	// device handle
					prm->Api,			        // application process identifier
					&prm->Addr,					// geographical or logical address
					&prm->Data[0],				// AlarmItem.Data
					prm->DataLen,				// length of AlarmItem.Data
					prm->UserStructIdent,		// AlarmItem.UserStructureIdentifier, s. IEC61158-6
					prm->UserHndl);				// user defined handle

    /* if there are no active ARs, just send acknowledgement to user */
    Num_Of_Ar = 0;
    for( i = 0; i < IOD_CFG_NUMOF_AR; i++ )
	{
    	if( PNPB_AR_OFFLINE != Pnpb.ArState[ i ] )
    	{
    		Num_Of_Ar++;
    	}
	}

    if( 0 == Num_Of_Ar )
    {
    	PNIO_cbf_async_req_done( PNIO_SINGLE_DEVICE_HNDL, 0, PNIO_ALM_STATUS, prm->Api, &prm->Addr, 0, prm->UserHndl );
    }

    if (Status != PNIO_OK)
    {
        PNIOext_async_req_error(PNIO_ALM_STATUS, PNIO_get_last_error ());
    	//PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
    }
}   /* PNIOext_process_alarm_send */


/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 * 	PNIOext_ret_of_sub_alarm_send
 *
 */
PNIO_VOID PNIOext_ret_of_sub_alarm_send(PNIO_UINT8* params)
{
	PNIO_UINT32 Status = PNIO_OK;
	PNIO_RET_OF_SUB_ALARM_SEND_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_RET_OF_SUB_ALARM_SEND_PRM*) params;

    Status = pnpb_ret_of_sub_alarm_send (
			PNIO_SINGLE_DEVICE_HNDL,	// device handle
			PNIO_DEFAULT_API,			// application process identifier
			&prm->Addr,					// geographical or logical address
			prm->UserHndl);				// user defined handle

    if (Status != PNIO_OK)
    {
    	PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
    }
} /* PNIOext_ret_of_sub_alarm_send */

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Upload retrieval alarm
 *
 */
PNIO_VOID PNIOext_upload_retrieval_alarm_send(PNIO_UINT8* params)
{
	PNIO_UINT32 Status = PNIO_OK;
	PNIO_UPLOAD_RETRIEVAL_ALARM_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_UPLOAD_RETRIEVAL_ALARM_PRM*) params;

    Status = pnpb_upload_retrieval_alarm_send (
			PNIO_SINGLE_DEVICE_HNDL, 	// device handle
			PNIO_DEFAULT_API,			// application process identifier
			&prm->Addr,					// geographical or logical address
			&prm->Data[0],				// AlarmItem.Data
			prm->DataLen,				// length of AlarmItem.Data
			prm->UserHndl);				// user defined handle

    if (Status != PNIO_OK)
    {
    	PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
    }
} /* PNIOext_upload_retrieval_alarm_send */

/*
 * Initialize, activate, deactivate, abort
 */

 /**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Set parameters of HW watchdog
 *
 */
PNIO_VOID PNIOExt_DeviceSetup(PNIO_UINT8* params)
{
    PNIO_DEVICE_SETUP_PRM *prm;
    PNIO_BOOL Status;
    PNIO_UINT8 *data_ptr;
    PNIO_FW_VERSION Version;

    /* Parse data into these structures */
    PNUSR_DEVICE_INSTANCE  *pPnUsrDev;          /* device setup configuration */
    PNIO_SUB_LIST_ENTRY    *pIoSubList;         /* plugged submodules, including PDEV */
    PNIO_UINT32            NumOfSublistEntries; /* number of entries in pPioSubList */
    PNIO_IM0_LIST_ENTRY    *pIm0List;           /* list of IM0 data sets */
    PNIO_UINT32            NumOfIm0ListEntries; /* number of entries in pIm0List */

    /* Temporary structure to parse pPnUsrDev */
    PNUSR_DEV_EXP_INSTANCE  *pDevSetup;
    /* Parse received parameters */
    prm = (PNIO_DEVICE_SETUP_PRM*) params;
    NumOfSublistEntries = prm->NumOfSublistEntries;
    NumOfIm0ListEntries = prm->NumOfIm0ListEntries;

    /* Allocate memory for IoSubList, Im0List. */
    /* No free here - these structures persist during whole device run */
    OsAllocF((void**)&pIoSubList, NumOfSublistEntries * sizeof(PNIO_SUB_LIST_ENTRY));
    OsAllocF((void**)&pIm0List, NumOfIm0ListEntries * sizeof(PNIO_IM0_LIST_ENTRY));
    OsAllocF((void**)&pPnUsrDev, sizeof(PNUSR_DEVICE_INSTANCE));
    /* Temporary structure to parse pPnUsrDev, will be freed */
    OsAllocF((void**)&pDevSetup, sizeof(PNUSR_DEV_EXP_INSTANCE));

    /* Parse data to structures */
    data_ptr = &(prm->Data);
    OsMemCpy(pIoSubList, data_ptr, NumOfSublistEntries * sizeof(PNIO_SUB_LIST_ENTRY));
    data_ptr += NumOfSublistEntries * sizeof(PNIO_SUB_LIST_ENTRY);
    OsMemCpy(pIm0List, data_ptr, NumOfIm0ListEntries * sizeof(PNIO_IM0_LIST_ENTRY));
    data_ptr += NumOfIm0ListEntries * sizeof(PNIO_IM0_LIST_ENTRY);
    OsMemCpy(pDevSetup, data_ptr, sizeof(PNUSR_DEV_EXP_INSTANCE));
    data_ptr += sizeof(PNUSR_DEV_EXP_INSTANCE);

    /* Size of pDevType is available now -> alloc structure for it. Persistent. */
    OsAllocF((void**)&pPnUsrDev->pDevType, (pDevSetup->DevTypeLen + 1));
    /* And copy it */
    OsMemCpy(pPnUsrDev->pDevType, data_ptr, pDevSetup->DevTypeLen);
    /* Devtype is terminated by '\0' */
    *(pPnUsrDev->pDevType + pDevSetup->DevTypeLen) = 0x00;

    /* Copy remaining data to PnUsrDev structure */
    pPnUsrDev->DeviceId = pDevSetup->DeviceId;
    pPnUsrDev->VendorId = pDevSetup->VendorId;
    pPnUsrDev->MaxNumOfBytesPerSubslot = pDevSetup->MaxNumOfBytesPerSubslot;
    pPnUsrDev->MaxNumOfSubslots = pDevSetup->MaxNumOfSubslots;

    /* DevSetup not needed any more */
    OsFree(pDevSetup);

    /* Prepare buffer for data processing */
    PnUsr_xhif_IO_buffer_init(pIoSubList, NumOfSublistEntries);

    //Update xhif data buffer
    PnUsr_xhif_cyclical_read();

    Status = PnUsr_DeviceSetup (pPnUsrDev,
                                pIoSubList,
                                NumOfSublistEntries,
                                pIm0List,
                                NumOfIm0ListEntries);


    /* Was alreadz sent before, so dummy now */
    Version.VerPrefix = 0;
    Version.VerPrefix = 0;
    Version.VerPrefix = 0;
    Version.VerPrefix = 0;
    Version.VerPrefix = 0;

    /* Report startup status back to BBB */
    if (Status == PNIO_OK)
    {
        PNIOext_cbf_device_startup_done(PNPB_USR_START_START_OK, &Version);
    }
    else
    {
        PNIOext_cbf_device_startup_done(PNPB_USR_START_START_FAILED, &Version);
    }

}   /* PNIOExt_DeviceSetup */

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Perform start of PN device
 *
 */

PNIO_VOID PNIOext_device_start(PNIO_UINT8* params)
{
    /* Parse received parameters */
    /* No params */
    PNIO_UNUSED_ARG(params);

    pnpb_activate();
}

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Perform stop of PN device
 *
 */

PNIO_VOID PNIOext_device_stop(PNIO_UINT8* params)
{
    /* Parse received parameters */
    /* No params */
    PNIO_UNUSED_ARG(params);

    pnpb_deactivate();
    PnUsr_DeactivateIoDatXch();

}

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Abort AR
 *
 */

PNIO_VOID PNIOext_device_ar_abort(PNIO_UINT8* params)
{
	PNIO_UINT32 Status = PNIO_OK;
	PNIO_AR_ABORT_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_AR_ABORT_PRM*) params;

    /* Do functionality */

	// ***** remove submodule in real config *****
	Status = pnpb_device_ar_abort (prm->ArNum);

    if (Status != PNIO_OK)
    {
    	PNIO_ConsolePrintf ("Error %x occured\n", PNIO_get_last_error ());
    }
}

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Activate IO data exchange
 *
 */

PNIO_VOID PNIOext_ActivateIoDatXch(PNIO_UINT8* params)
{
    /* Parse received parameters */
        /* No params */
    PNIO_UNUSED_ARG(params);
    /* Call functionality */
    PnUsr_ActivateIoDatXch();

}   /* PNIOext_ActivateIoDatXch */


/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Deactivate IO data exchange
 *
 */

PNIO_VOID PNIOext_DeactivateIoDatXch(PNIO_UINT8* params)
{
    /* Parse received parameters */
        /* No params */
    PNIO_UNUSED_ARG(params);
    /* Call functionality */
	PnUsr_DeactivateIoDatXch();

}   /* PNIOext_DeactivateIoDatXch */

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Reboot OS
 *
 */

PNIO_VOID PNIOext_slave_reboot(PNIO_UINT8* params)
{
    /* Parse received parameters */
    /* No params */
    PNIO_UNUSED_ARG(params);

    /* Perform reboot */
    PNIO_ConsolePrintf ("OsReboot in 2 sec....\n");
    OsReboot();
}

/*
 * Pull/plug submodules
 */

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Plug all modules and submodules
 *
 */

PNIO_VOID PNIOext_sub_plug_list(PNIO_UINT8* params)
{
    PNIO_UINT32 Status = PNIO_OK;
	PNIO_SUB_PLUG_LIST_PRM *prm;
	PNIO_UINT32* pStatusList = NULL;
	PNIO_UINT32 i;
	PNIO_SUB_LIST_ENTRY* pIoSubList;
	PNIO_IM0_LIST_ENTRY* pIm0List;

    /* Parse received parameters */
    prm = (PNIO_SUB_PLUG_LIST_PRM*) params;

    /* Do functionality */

    // *=================================================================
    // *  alloc memory for lists
    // *=================================================================
    OsAlloc ((void**)&pStatusList, 0, sizeof (PNIO_UINT32) * prm->NumOfSubListEntries);
    OsAlloc ((void**)&pIoSubList, 0, sizeof (PNIO_SUB_LIST_ENTRY) * prm->NumOfSubListEntries);
    OsAlloc ((void**)&pIm0List, 0, sizeof (PNIO_IM0_LIST_ENTRY) * prm->NumOfIm0ListEntries);

    // *=================================================================
    // *  copy data to structures
    // *=================================================================
    OsMemCpy(pIoSubList, &prm->Data[0], sizeof (PNIO_SUB_LIST_ENTRY) * prm->NumOfSubListEntries);
    OsMemCpy(pIoSubList, &prm->Data[sizeof (PNIO_SUB_LIST_ENTRY) * prm->NumOfSubListEntries],
    		sizeof (PNIO_IM0_LIST_ENTRY) * prm->NumOfIm0ListEntries);

	// *----------------------------------------------------------
	// * lock access to PnpbMod from other contexts
	// *----------------------------------------------------------
	OsEnterX(OS_MUTEX_PNPB_PLUG);
	OsEnterX(OS_MUTEX_PNPB);

	Status = pnpb_sub_real_plug_list(pIoSubList,	// plugged submodules, including PDEV
			prm->NumOfSubListEntries,				// number of entries in pIoSubList
			pIm0List,								// list of IM0 data sets
			prm->NumOfIm0ListEntries,				// number of entries in pIm0List
			pStatusList);							// pointer to list of return values (stati)

	// *----------------------------------------------------------
	// * unlock access to PnpbMod from other contexts
	// *----------------------------------------------------------
	OsExitX(OS_MUTEX_PNPB);
	OsExitX(OS_MUTEX_PNPB_PLUG);

    // * ----------------------------------------------------
    // *   evaluate error list
    // *   note: the response values in errorList are
    // *   available after PNIO_sub_plug with MoreFollows = FALSE !
    // *   The allocated memory for pErrList must not be set free before !.
    // * ----------------------------------------------------
    for (i = 0; i < prm->NumOfSubListEntries; i++)
    {
        if (*(pStatusList+i) == PNIO_OK)
            PNIO_printf ("SubPlugResponse(%d) = OK\n", i, *(pStatusList+i));
        else
            PNIO_printf ("SubPlugResponse(%d) = ERROR\n", i, *(pStatusList+i));
    }

    // *=================================================================
    // *  free memory for error list
    // *=================================================================
    OsFree ((void*)pStatusList);

	if (Status != PNIO_OK)
	{
		PNIO_ConsolePrintf("Error %x occured\n", PNIO_get_last_error());
	}
}

static PNIO_BOOL is_IM_DATA_empty(IM0_DATA* data)
{
	PNIO_UINT32 len = sizeof(IM0_DATA);
	PNIO_UINT8* ptr = (PNIO_UINT8*) data;

	while (len--)
	{
		if (*ptr++)
		{
			return PNIO_FALSE;
		}
	}

	return PNIO_TRUE;
}

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Plug a single submodule
 *
 */

PNIO_VOID PNIOext_sub_plug(PNIO_UINT8* params)
{
	PNIO_UINT32 Status = PNIO_OK;
	PNIO_UINT32 j;                  /*iteration var*/
    PNIO_SUB_PLUG_PRM *prm;

    /* Parse received parameters */
    prm = (PNIO_SUB_PLUG_PRM*) params;

    /* Do functionality */

    // *----------------------------------------------------------
    // * lock access to PnpbMod from other contexts
    // *----------------------------------------------------------
    OsEnterX (OS_MUTEX_PNPB_PLUG);
    OsEnterX (OS_MUTEX_PNPB);

    if(is_IM_DATA_empty(&prm->Im0Dat))
    {
		// *----------------------------------------------------------
		// * plug single submodule
		// *----------------------------------------------------------
		Status = pnpb_sub_real_plug(
				PNIO_SINGLE_DEVICE_HNDL,
				PNIO_DEFAULT_API,
				&prm->Addr,
				prm->ModIdent,
				prm->SubIdent,
                prm->InputDataLen,
                prm->OutputDataLen,
				prm->Im0Support,
				(IM0_DATA*) NULL,
				prm->IopsIniVal,
				NULL,
				PNIO_FALSE);  // no more follows --> process request in PNDV directly
    }
    else
    {
		// *----------------------------------------------------------
		// * plug single submodule
		// *----------------------------------------------------------
		Status = pnpb_sub_real_plug(
				PNIO_SINGLE_DEVICE_HNDL,
				PNIO_DEFAULT_API,
				&prm->Addr,
				prm->ModIdent,
				prm->SubIdent,
                prm->InputDataLen,
                prm->OutputDataLen,
				prm->Im0Support,
				&prm->Im0Dat,
				prm->IopsIniVal,
				NULL,
				PNIO_FALSE);  // no more follows --> process request in PNDV directly
    }

    // **** increment number of submoduls and find empty record in submodule params
    if (Status == PNIO_OK)
    {
        PnpbMod.NumOfPluggedSub++;        // increment number of plugged submodules

        for (j = 0; j < PNUSR_XHIF_MAX_NUM_OF_SUBMODULES; ++j) {
            if(NULL == pnusr_submodule_params[j])
            {
                OsAlloc((void**)&pnusr_submodule_params[j], 0x00, sizeof(pnusr_subslot_params));

                pnusr_submodule_params[j]->Slot = prm->Addr.ad.Geo1.Slot;
                pnusr_submodule_params[j]->Subslot = prm->Addr.ad.Geo1.Subslot;
                pnusr_submodule_params[j]->InData_size = prm->InputDataLen;
                pnusr_submodule_params[j]->OutData_size = prm->OutputDataLen;
                pnusr_submodule_params[j]->Direction = PNUSR_XHIF_DIRECTION_NO_DATA;
                /* Device is ready to receive data at the beginning => Out IOcS is PNIO_S_GOOD */
                /* Out.iocs have to be in PNIO_S_GOOD, others PNIO_S_BAD or we don't care - use BAD as default */
                pnusr_submodule_params[j]->InIOcS = PNIO_S_BAD;
                pnusr_submodule_params[j]->InIOpS = PNIO_S_BAD;
                pnusr_submodule_params[j]->OutIOcS = PNIO_S_GOOD;
                pnusr_submodule_params[j]->OutIOpS = PNIO_S_BAD;

                if(0 < pnusr_submodule_params[j]->InData_size)
                {
                    pnusr_submodule_params[j]->Direction |=  PNUSR_XHIF_DIRECTION_IN;
                }
                if(0 < pnusr_submodule_params[j]->OutData_size)
                {
                    pnusr_submodule_params[j]->Direction |=  PNUSR_XHIF_DIRECTION_OUT;
                }
                if((0 < pnusr_submodule_params[j]->OutData_size) || (0 < pnusr_submodule_params[j]->InData_size))
                {
                    OsAlloc((void**)&pnusr_submodule_IO_data[j], 0x00,
                            (pnusr_submodule_params[j]->InData_size + pnusr_submodule_params[j]->OutData_size));
                }

                break;
            }
        }
    }

    // *----------------------------------------------------------
    // * unlock access to PnpbMod from other contexts
    // *----------------------------------------------------------
    OsExitX (OS_MUTEX_PNPB);
    OsExitX (OS_MUTEX_PNPB_PLUG);

    /* if there are no active ARs, just send acknowledgement to user */
    PNIO_UINT32 i, Num_Of_Ar = 0;
    for( i = 0; i < IOD_CFG_NUMOF_AR; i++ )
	{
    	if( PNPB_AR_OFFLINE != Pnpb.ArState[ i ] )
    	{
    		Num_Of_Ar++;
    	}
	}

//    if( 0 == Num_Of_Ar )
//    {
//    	PNIO_cbf_async_req_done( PNIO_SINGLE_DEVICE_HNDL, 0, PNIO_ALM_RET_OF_SUB, PNIO_DEFAULT_API, &prm->Addr, 0, 0 );
//    }

    if (Status != PNIO_OK)
    {
        PNIOext_async_req_error(PNIO_ALM_RET_OF_SUB, PNIO_get_last_error ());
    }
    else
    {
        PNIO_cbf_async_req_done( PNIO_SINGLE_DEVICE_HNDL, Num_Of_Ar, PNIO_ALM_RET_OF_SUB, PNIO_DEFAULT_API, &prm->Addr, 0, 0 );
    }
}

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Pull a single submodule
 *
 */

PNIO_VOID PNIOext_sub_pull(PNIO_UINT8* params)
{
    PNIO_UINT32 Status = PNIO_OK;
    PNIO_SUB_PULL_PRM *prm;
    PNIO_UINT32 j;

    /* Parse received parameters */
    prm = (PNIO_SUB_PULL_PRM*) params;

    /* Do functionality */

	// *----------------------------------------------------------
	// * lock access to PnpbMod from other contexts
	// *----------------------------------------------------------
	OsEnterX(OS_MUTEX_PNPB_PLUG);
	OsEnterX(OS_MUTEX_PNPB);

	// ***** remove submodule in real config *****
	Status = pnpb_sub_real_pull(PNIO_SINGLE_DEVICE_HNDL, PNIO_DEFAULT_API, &prm->Addr);

	if (Status == PNIO_OK)
    {

	    for (j = 0; j < PNUSR_XHIF_MAX_NUM_OF_SUBMODULES; ++j)
	    {
	        if(pnusr_submodule_params[j] != NULL
	        && pnusr_submodule_params[j]->Slot == prm->Addr.ad.Geo1.Slot
	        && pnusr_submodule_params[j]->Subslot == prm->Addr.ad.Geo1.Subslot)
            {
	            if(pnusr_submodule_IO_data[j] != NULL)
	            {
                    OsFree((void**)&pnusr_submodule_IO_data[j]);
                    pnusr_submodule_IO_data[j] = NULL;
	            }
	            OsFree((void**)&pnusr_submodule_params[j]);
	            pnusr_submodule_params[j] = NULL;
            }
        }
    }else
    {
        PNIO_printf("pull not done\n");
    }
	// *----------------------------------------------------------
	// * unlock access to PnpbMod from other contexts
	// *----------------------------------------------------------
	OsExitX(OS_MUTEX_PNPB);
	OsExitX(OS_MUTEX_PNPB_PLUG);

	/* if there are no active ARs, just send acknowledgement to user */
	PNIO_UINT32 i, Num_Of_Ar = 0;
	for (i = 0; i < IOD_CFG_NUMOF_AR; i++)
	{
		if (PNPB_AR_OFFLINE != Pnpb.ArState[i])
		{
			Num_Of_Ar++;
		}
	}

//	if (0 == Num_Of_Ar)
//	{
//	    PNIO_cbf_async_req_done(PNIO_SINGLE_DEVICE_HNDL, 0, PNIO_ALM_RET_OF_SUB, PNIO_DEFAULT_API, &prm->Addr,
//	                    0, 0);
//	}

    if (Status != PNIO_OK)
    {
        PNIOext_async_req_error(PNIO_ALM_RET_OF_SUB, PNIO_get_last_error ());
    }
    else
    {
        PNIO_cbf_async_req_done(PNIO_SINGLE_DEVICE_HNDL, Num_Of_Ar, PNIO_ALM_RET_OF_SUB, PNIO_DEFAULT_API, &prm->Addr,
				0, 0);
    }
}

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Read substitution values for output submodule
 *
 */

PNIO_VOID PNIOext_substval_out_read_done(PNIO_UINT8* params)
{
    PNIO_SUBSTVAL_OUT_READ_DONE_PRM *prm;

    /* Parse received parameters */
    prm = (PNIO_SUBSTVAL_OUT_READ_DONE_PRM*) params;

    /* Perform action */

    // *** set the substitute data  ***
    OsMemCpy((PNIO_VOID*) prm->BufAddr, prm->BufData, prm->BufLen);

    // *** *SubstMode = 2;      // we accept the preset value from the stack (set mode to "replace"), so nothing to do here"
    //*(PNIO_UINT16*) prm->SubstModeAddr = prm->SubstMode;

    // *** we accept the preset value from stack:  "0 = Operation", if (remoteIOPS == GOOD and localIOCS == GOOD),  else "1 = Substitute"
    //*(PNIO_UINT16*) prm->SubstActiveAddr = REC8029_SUBSTSTATE_OPERATION;
} /* PNIOext_substval_out_read_done */

/*
 * Watchdog feature
 */

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Set parameters of HW watchdog
 *
 */
PNIO_VOID PNIOext_hw_watchdog_set(PNIO_UINT8* params)
{
    PNIO_HW_WD_SET_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_HW_WD_SET_PRM*) params;

    /* Do functionality */
    PNIO_printf("Test functionality, do nothing for now, received params for hw wd: time %d, granity %d\n",
            prm->time, prm->granity);

}   /* PNIOext_hw_watchdog_set */

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Incoming command to watchdog
 *
 */
PNIO_VOID PNIOext_hw_watchdog_command(PNIO_UINT8* params)
{
    PNIO_HW_WD_COMMAND_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_HW_WD_COMMAND_PRM*) params;

    /* Do functionality */
    PNIO_printf("Test functionality, do nothing for now, received params for hw wd: command %d",
            prm->command);

}   /* PNIOext_hw_watchdog_command */

/*
 * Change device name , IP and MAC address
 */

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Store new MAC address to non-volatile memory
 *
 */

PNIO_VOID PNIOext_store_new_MAC(PNIO_UINT8* params)
{
    PNIO_UINT8 result;
    PNIO_STORE_NEW_MAC_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_STORE_NEW_MAC_PRM*) params;

    /* Store new mac address */
    result = ParamStoreMacAddress (prm->mac_addr);

    /* Send result to BBB */
    PNIOext_cbf_result_new_device_address(PNIO_STORE_NEW_MAC, result);
} /* PNIOext_store_new_MAC */

/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Store new IP address to non-volatile memory
 *
 */

PNIO_VOID PNIOext_store_new_IP(PNIO_UINT8* params)
{
    PNIO_UINT8 result;
    PNIO_STORE_NEW_IP_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_STORE_NEW_IP_PRM*) params;

    /* Store new ip address */
    result = ParamStoreIpAddress (prm->ip_suite);

    /* Send result to BBB */
    PNIOext_cbf_result_new_device_address(PNIO_STORE_NEW_IP, result);
} /* PNIOext_store_new_IP */


/**
 *  @brief Parse parameters, do / call functionality
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Store new device name
 *
 */
PNIO_VOID PNIOext_store_new_device_name(PNIO_UINT8* params)
{
    PNIO_UINT8 result;
    PNIO_STORE_NEW_DEV_NAME_PRM *prm;
    /* Parse received parameters */
    prm = (PNIO_STORE_NEW_DEV_NAME_PRM*) params;

    /* Store new device name */
    result = ParamStoreDeviceName (prm->dev_name, prm->dev_len);

    /* Send result to BBB */
    PNIOext_cbf_result_new_device_address(PNIO_STORE_NEW_NAME, result);
} /* PNIOext_store_new_device_name */

/*
 * Request responses
 */

/**
 *  @brief Recieve response from read record
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Fill buffers, set semaphore
 *
 */
PNIO_VOID PNIOext_rec_read_rsp(PNIO_UINT8* params)
{
    /* Parse received data */
    PNIO_RECORD_READ_RSP_PRM *prm = (PNIO_RECORD_READ_RSP_PRM *)params;
    UNION_PNIO_ERR_STAT   PnioStat;
    PnioStat.vu32 = 0;

    OsMemCpy(&(PnioStat.State), &(prm->PnioState), sizeof(PNIO_ERR_STAT));
    OsMemCpy((PNIO_UINT8 *)prm->pBuffer, &(prm->Buffer), prm->BufLen);

    /* Done -> trigger PNDV */
    PNIO_trigger_pndv_ds_rw_done(&(PnioStat.State), prm->BufLen);
}   /* PNIOext_rec_read_rsp */


/**
 *  @brief Recieve response from write record
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Fill buffers, set semaphore
 *
 */
PNIO_VOID PNIOext_rec_write_rsp(PNIO_UINT8* params)
{
    /* Parse received data */
    PNIO_RECORD_WRITE_RSP_PRM *prm = (PNIO_RECORD_WRITE_RSP_PRM *)params;
    UNION_PNIO_ERR_STAT   PnioStat;
    PnioStat.vu32 = 0;

    OsMemCpy(&(PnioStat.State), &(prm->PnioState), sizeof(PNIO_ERR_STAT));

    /* Done -> trigger PNDV */
    PNIO_trigger_pndv_ds_rw_done(&(PnioStat.State), 0);
}   /* PNIOext_rec_write_rsp */

/**
 *  @brief Receive response from AMR handler in BBB
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Fill buffers, set semaphore
 *
 */
PNIO_VOID PNIOext_amr_response_handler_rsp(PNIO_UINT8* params)
{
	/* Parse received data */
	PNIO_AMR_HANDLER_RSP_PRM *prm = (PNIO_AMR_HANDLER_RSP_PRM *)params;
	/* Place data on requested pointers */
	PNIO_UINT8* BufPtr = (PNIO_UINT8*)prm->pAMRHndlPrm.pBufAddr;
	UNION_PNIO_ERR_STAT PnioStat;
	PnioStat.vu32 = 0;

	/* Save error */
	OsMemCpy(&(PnioStat), &(prm->PnioState), sizeof(PNIO_ERR_STAT));

	/* If err is not present, copy data to destination buffer */
	if ((PnioStat.vu32 == 0) && (PnioStat.State.AddValue1 == 0) && (PnioStat.State.AddValue2 == 0))
	{
		if (prm->pAMRHndlPrm.pBufLen > 0)
		{
			/* Copy data */
			OsMemCpy(BufPtr, prm->pData, prm->pAMRHndlPrm.pBufLen);
		}
	}
	else
	{
		/* No data coppied in case of error */
		prm->pAMRHndlPrm.pBufLen = 0;
	}

	/* Done -> trigger PNDV */
	PNIO_trigger_pndv_ds_rw_done(&(PnioStat.State), prm->pAMRHndlPrm.pBufLen);
}

/**
 *  @brief Receive response from PE handler in BBB
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Fill buffers, set semaphore
 *
 */
PNIO_VOID PNIOext_pe_response_handler_rsp(PNIO_UINT8* params)
{
    /* Parse received data */
    PNIO_PE_RESPONSE_HANDLER_RSP_PRM *prm = (PNIO_PE_RESPONSE_HANDLER_RSP_PRM *)params;
    /* Place data on requested pointers */
    PNIO_UINT8* BufPtr = (PNIO_UINT8*) prm->pPEHndlPrm.pBufAddr;
    UNION_PNIO_ERR_STAT   PnioStat;
    PnioStat.vu32 = 0;

    if(prm->pPEHndlPrm.pBufLen > 0)
    {
        /* Copy data */
    OsMemCpy(BufPtr, prm->pData, prm->pPEHndlPrm.pBufLen);
    }

    OsMemCpy(&(PnioStat.State), &(prm->PnioState), sizeof(PNIO_ERR_STAT));

    /* Done -> trigger PNDV */
    PNIO_trigger_pndv_ds_rw_done(&(PnioStat.State), prm->pPEHndlPrm.pBufLen);
}

/**
 *  @brief Receive request from PE handler in BBB
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Fill buffers, set semaphore
 *
 */
PNIO_VOID PNIOext_pe_request_handler_rsp(PNIO_UINT8* params)
{
    /* Parse received data */
    PNIO_PE_REQUEST_HANDLER_RSP_PRM *prm = (PNIO_PE_REQUEST_HANDLER_RSP_PRM *)params;
    /* Place data on requested pointers */
    UNION_PNIO_ERR_STAT   PnioStat;
    PnioStat.vu32 = 0;

    OsMemCpy(&(PnioStat.State), &(prm->PnioState), sizeof(PNIO_ERR_STAT));

    /* Done -> trigger PNDV */
    PNIO_trigger_pndv_ds_rw_done(&(PnioStat.State), 0);
}

/*
 * Trace functionality
 */

/**
 *  @brief Store/restore trace from/to non-volatile memory
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 */

PNIO_VOID PNIOext_trace_command(PNIO_UINT8* params)
{
    /* Parse received data */
    PNIO_TRACE_COMMAND_PRM *prm = (PNIO_TRACE_COMMAND_PRM *)params;

    /* Determine command from parameter */
    if(prm->command == PNIO_TRACE_SAVE)
    {
    	if(use_ertec_nv == PNIO_TRUE)
    	{
#if(PNIO_TRACE != PNIO_TRACE_NONE)
    		/* Storage of NV data is in Ertec's NV memory */
    		TrcStoreBuf();
#endif
    	}
    	else
    	{
    		/* Storage of NV data is in host, transfer
    		 * traces to its memory */
            PNIO_BOOL last_trace = PNIO_TRUE;
            PNIO_UINT32 len = 0;
            PNIO_TRACE_READY_PRM *param;

            /* Set flags */
            traces_store = PNIO_TRUE;
            traces_was_restore = PNIO_FALSE;

            OsAlloc((void **) &param, 0, sizeof(PNIO_TRACE_READY_PRM));

            /* Place buffer in memory */
#if(PNIO_TRACE != PNIO_TRACE_NONE)
            SendTraceBufferToMem(PNIO_TRUE, traces_store, &last_trace, pnusr_xhif_trace_out, &len);
#endif
            /* Save length and store flag */
            param->store = 1;
            param->len = len;

            /* Was it the last trace? */
            if(last_trace == PNIO_FALSE)
            {
                param->trace_end = 0;
            }
            else
            {
            	param->trace_end = 1;    /* Last one */
            }

            /* Send response */
            PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_TRACE_READY, sizeof(PNIO_TRACE_READY_PRM), (PNIO_UINT8*)(param));
        }
    }
    else if(prm->command == PNIO_TRACE_RESTORE)
    {
    	if(use_ertec_nv == PNIO_TRUE)
    	{
    		/* Storage of NV data is in Ertec's NV memory */

    		/* Send restored traces back */
            PNIO_BOOL last_trace = PNIO_TRUE;
            PNIO_UINT32 len = 0;
            PNIO_TRACE_READY_PRM *param;

            /* Set flags */
            traces_store = PNIO_FALSE;
            traces_was_restore = PNIO_TRUE;

            OsAlloc((void **) &param, 0, sizeof(PNIO_TRACE_READY_PRM));
#if(PNIO_TRACE != PNIO_TRACE_NONE)
            /* Place buffer in memory */
            RestoreTraceBufferToMem (PNIO_TRUE, &last_trace, pnusr_xhif_trace_out, &len);
#endif
            /* Save length and store flag */
            param->store = 0;
            param->len = len;

            /* Was it the last trace? */
            if(last_trace == PNIO_FALSE)
            {
                param->trace_end = 0;
            }
            else
            {
            	param->trace_end = 1;    /* Last one */
            }

            /* Send response */
            PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_TRACE_READY, sizeof(PNIO_TRACE_READY_PRM), (PNIO_UINT8*)(param));
    	}
    	else
    	{
    		/* Storage of NV data is in host - nothing to be done */
    	}
    }
    else if(prm->command == PNIO_TRACE_PRINT_START)
    {
        PNIO_BOOL last_trace = PNIO_TRUE;
        PNIO_UINT32 len = 0;
        PNIO_TRACE_READY_PRM *param;

        /* Set flags */
        traces_store = PNIO_FALSE;
        traces_was_restore = PNIO_FALSE;

        OsAlloc((void **) &param, 0, sizeof(PNIO_TRACE_READY_PRM));
#if(PNIO_TRACE != PNIO_TRACE_NONE)
        /* Place buffer in memory */
        SendTraceBufferToMem(PNIO_TRUE, traces_store, &last_trace, pnusr_xhif_trace_out, &len);
#endif
        /* Save length and store flag */
        param->store = 0;
        param->len = len;

        /* Was it the last trace? */
        if(last_trace == PNIO_FALSE)
        {
            param->trace_end = 0;
        }
        else
        {
        	param->trace_end = 1;    /* Last one */
        }

        /* Send response */
        PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_TRACE_READY, sizeof(PNIO_TRACE_READY_PRM), (PNIO_UINT8*)(param));
    }
    else if(prm->command == PNIO_TRACE_PRINT_NEXT)
    {
        PNIO_BOOL last_trace = PNIO_TRUE;
        PNIO_UINT32 len = 0;
        PNIO_TRACE_READY_PRM *param;

        OsAlloc((void **) &param, 0, sizeof(PNIO_TRACE_READY_PRM));
#if(PNIO_TRACE != PNIO_TRACE_NONE)
        /* Place buffer in memory */
        if(traces_was_restore == PNIO_FALSE)
        {
        	SendTraceBufferToMem(PNIO_FALSE, traces_store, &last_trace, pnusr_xhif_trace_out, &len);
        }
        else
        {
        	RestoreTraceBufferToMem (PNIO_TRUE, &last_trace, pnusr_xhif_trace_out, &len);
        }
#endif
        /* Tell host whether to store this traces or print them */
        if(traces_store == PNIO_FALSE)
        {
        	param->store = 0;
        }
        else
        {
        	param->store = 1;
        }
        /* Save length */
        param->len = len;

        /* Was it the last trace? */
        if(last_trace == PNIO_FALSE)
        {
            param->trace_end = 0;
        }
        else
        {
        	param->trace_end = 1;    /* Last one */
        }

        /* Send response */
        PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_TRACE_READY, sizeof(PNIO_TRACE_READY_PRM), (PNIO_UINT8*)(param));
    }
    else
    {
        /* Wrong parameter */
        PNIO_printf("Error: Received invalid command (trace command)\n");
    }
} /* PNIOext_trace_command */

/**
 *  @brief Change trace level for single subsystem / whole package
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_trace_settings(PNIO_UINT8* params)
{
    /* Parse received data */
    PNIO_TRACE_SETTINGS_PRM *prm = (PNIO_TRACE_SETTINGS_PRM *)params;
#if(PNIO_TRACE != PNIO_TRACE_NONE)
    /* Determine command from parameter */
    if(prm->command == PNIO_TRACE_ALL_SUBMODULES)
    {
    	if (prm->module == TRACE_SUBSYS_NUM)
    	{
    		TrcDkSetAllLevel (prm->trc_level);
    	}
    	else
    	{
    		TrcDkSetPackageLevel(prm->module, prm->trc_level);
    	}
    }
    else if(prm->command == PNIO_TRACE_SINGE_SUBMODULE)
    {
        TrcDkSetLevel(prm->module, prm->trc_level);
    }
    else
    {
        /* Wrong parameter */
        PNIO_printf("Error: Recieved invalid command (trace settings)\n");
    }
#endif
} /* PNIOext_trace_settings */



/**
 *  @brief Recieve and process command for PN ON
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Starts stopped PN stack
 *
 */
PNIO_VOID PnUsr_xhif_pn_on(PNIO_UINT8* params)
{
    PNIO_UNUSED_ARG(params);
    PNIO_device_start(PNIO_SINGLE_DEVICE_HNDL);
}   /* PnUsr_xhif_pn_on */


/**
 *  @brief Recieve and process command for PN ON
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Stops PN stack
 *
 */
PNIO_VOID PnUsr_xhif_pn_off(PNIO_UINT8* params)
{
    PNIO_UNUSED_ARG(params);
    PNIO_device_stop(PNIO_SINGLE_DEVICE_HNDL);
}   /* PnUsr_xhif_pn_off */

/*
 * Non-volatile memory functionality
 */

/**
 *  @brief Recieve init command from host
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 *  Init flash / store data from host to flash
 *
 */
PNIO_VOID PNIOext_nv_data_init(PNIO_UINT8* params)
{
    PNIO_NV_DATA_INIT_PRM* prm = (PNIO_NV_DATA_INIT_PRM*) params;

    if(prm->useErtecNVMem == 1)
    {
    	use_ertec_nv = PNIO_TRUE;
        Bsp_nv_data_init (NULL, 0);
    }
    else
    {
    	use_ertec_nv = PNIO_FALSE;
        Bsp_nv_data_init (prm->pData, prm->pBufLen);
    }
    /* Callback to host is sent inside Bsp_nv_data_init */
}

/**
 *  @brief Recieve clear command from host
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_nv_data_clear(PNIO_UINT8* params)
{
    PNIO_UINT32 Status = PNIO_OK;
    PNIO_NV_DATA_CLEAR_PRM* prm = (PNIO_NV_DATA_CLEAR_PRM*) params;

    Status = Bsp_nv_data_clear ((PNIO_RTF_OPTION) prm->RtfOption);
    if(Status != PNIO_OK)
    {
        /* Report error */
        PNIOext_cbf_nv_data_flash_done(PNIO_NOT_OK, 0, 0);
    }

    /* Callback sent in Bsp_nv_data_factory_reset_flash_done */
}

/**
 *  @brief Recieve nv data store command from host
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_nv_data_store(PNIO_UINT8* params)
{
    PNIO_UINT32 Status = PNIO_OK;
    PNIO_NV_DATA_STORE_PRM* prm = (PNIO_NV_DATA_STORE_PRM*) params;

    Status = Bsp_nv_data_store ((PNIO_NVDATA_TYPE) prm->NvDataType, prm->pData, prm->pBufLen);
    if(Status != PNIO_OK)
    {
        /* Report error */
        PNIOext_cbf_nv_data_flash_done(PNIO_NOT_OK, 0, prm->NvDataType);
    }
    /* Callback sent in Bsp_nv_data_flash_done */
}

/**
 *  @brief Recieve nv im data store command from host
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_im_data_store(PNIO_UINT8* params)
{
    PNIO_UINT32 Status = PNIO_OK;
    PNIO_NV_IM_DATA_STORE_PRM* prm = (PNIO_NV_IM_DATA_STORE_PRM*) params;

    Status = Bsp_im_data_store ((PNIO_NVDATA_TYPE) prm->NvDataType, prm->pData, prm->pBufLen, prm->ModIdent);
    if(Status != PNIO_OK)
    {
        /* Report error */
        PNIOext_cbf_nv_data_flash_done(PNIO_NOT_OK, 0, prm->NvDataType);
    }
    /* Callback sent in Bsp_nv_data_flash_done */
}

/**
 *  @brief Recieve response to IM write command
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_im_write_rsp(PNIO_UINT8* params)
{
    /* Parse received data */
    PNIO_IM_WRITE_RSP_PRM *prm = (PNIO_IM_WRITE_RSP_PRM *)params;
    /* Place data on requested pointers */
    UNION_PNIO_ERR_STAT   PnioStat;
    PnioStat.vu32 = 0;

    OsMemCpy(&(PnioStat.State), &(prm->PnioState), sizeof(PNIO_ERR_STAT));

    /* Done -> trigger PNDV */
    PNIO_trigger_pndv_ds_rw_done(&(PnioStat.State), 0);
}

/**
 *  @brief Recieve response to IM read command
 *
 *  @param[in]      *params         Pointer to parameters
 *
 *  @return         void
 *
 */
PNIO_VOID PNIOext_im_read_rsp(PNIO_UINT8* params)
{
    /* Parse received data */
    PNIO_IM_READ_RSP_PRM *prm = (PNIO_IM_READ_RSP_PRM *)params;
    /* Place data on requested pointers */
    PNIO_UINT8* BufPtr = (PNIO_UINT8*) prm->pBufAddr;
    UNION_PNIO_ERR_STAT   PnioStat;
    PnioStat.vu32 = 0;

    if(prm->pBufLen > 0)
    {
        /* Copy data */
        OsMemCpy(BufPtr, prm->pData, prm->pBufLen);
    }

    OsMemCpy(&(PnioStat), &(prm->PnioState), sizeof(PNIO_ERR_STAT));

    /* Done -> trigger PNDV */
    PNIO_trigger_pndv_ds_rw_done(&(PnioStat.State), prm->pBufLen);
}


/************************ SENDING ACYCLIC TELEGRAMS - FUNCTIONS TO BBB **************************/


/**
 *  @brief Startup status reporting
 *
 *  @param[in]      ArNum               AR number 1....NumOfAR
 *  @param[in]      *pOwnSub            Expected configuration in ownership indication
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_device_startup_done(PNPB_USR_STARTUP_STATE State, PNIO_FW_VERSION *Version)
{
    PNIO_STARTUP_DONE_PRM *prm;

    OsAlloc ( (void **)&prm, 0 , sizeof (PNIO_STARTUP_DONE_PRM) );

    prm->State = (PNIO_UINT32)State;
    prm->Version.VerPrefix = Version->VerPrefix;
    prm->Version.VerHh = Version->VerHh;
    prm->Version.VerH = Version->VerH;
    prm->Version.VerL = Version->VerL;
    prm->Version.VerLl = Version->VerLl;
    prm->Fw_DAP = MODULE_ID_DAP;
    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_DEVICE_STARTUP_DONE, sizeof(PNIO_STARTUP_DONE_PRM), (PNIO_UINT8*)prm);

}   /* PNIOext_cbf_ar_ownership_ind */


/**
 *  @brief AR Connect reached in state machine of establishing AR
 *
 *  @param[in]      *ArType             Type of AR (see cm_ar_type_enum)
 *  @param[in]      *ArNum              AR number
 *  @param[in]      *SendClock          Sendclock
 *  @param[in]      *RedRatioIocrIn     Reduction ratio of input IOCR
 *  @param[in]      *RedRatioIocrOut    Reduction ratio of output IOCR
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_ar_connect_ind(PNIO_AR_TYPE ArType, PNIO_UINT32 ArNum,
        PNIO_UINT16 SendClock, PNIO_UINT16 RedRatioIocrIn, PNIO_UINT16 RedRatioIocrOut, PNIO_UINT16 ArSessionKey, PNIO_UINT32 hostIP)
{

    PNIO_CONNECT_IND_PRM *params;

    OsAlloc ( (void **)&params, 0 , sizeof (PNIO_CONNECT_IND_PRM) );

    params->ArType = ArType;
    params->ArNum = ArNum;
    params->SendClock = SendClock;
    params->RedRatioIocrIn = RedRatioIocrIn;
    params->RedRatioIocrOut = RedRatioIocrOut;
    params->ArSessionKey = ArSessionKey;
    params->hostIP = hostIP;

    /* Send to XHIF */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_AR_CONNECT_IND, sizeof(PNIO_CONNECT_IND_PRM), (PNIO_UINT8*)params);

}   /* PNIOext_cbf_ar_connect_ind */


/**
 *  @brief Ownership indication
 *
 *  @param[in]      ArNum               AR number 1....NumOfAR
 *  @param[in]      *pOwnSub            Expected configuration in ownership indication
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_ar_ownership_ind(PNIO_UINT32 ArNum,PNIO_EXP *pOwnSub)
{
    PNIO_OWNERSHIP_IND_PRM *pparams;

    /* Allocate memory for prms to be sent */
    OsAlloc((void**)&pparams, 0, sizeof(PNIO_OWNERSHIP_IND_PRM));

    /* Copy params */
    pparams->ArNum = ArNum;
    OsMemCpy(&(pparams->OwnSub), pOwnSub, sizeof(PNIO_EXP));

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_AR_OWNERSHIP_IND, sizeof(PNIO_OWNERSHIP_IND_PRM), (PNIO_UINT8*)pparams);
}   /* PNIOext_cbf_ar_ownership_ind */


/**
 *  @brief End of parameterization
 *
 *  @param[in]      ArNum               AR number 1....NumOfAR
 *  @param[in]      SessionKey          Session Key
 *  @param[in]      Api                 API (valid only, if SubslotNum <> 0)
 *  @param[in]      SlotNum             SlotNum (valid only, if SubslotNum <> 0)
 *  @param[in]      SubslotNum          == 0:    param end for all submodules, <> 0:    param end only for this submodule
 *  @param[in]      MoreFollows         PNIO_TRUE: more param end ind follow, PNIO_FALSE: last one
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_param_end_ind    (PNIO_UINT16 ArNum,
                                        PNIO_UINT16 SessionKey,
                                        PNIO_UINT32 Api,
                                        PNIO_UINT16 SlotNum,
                                        PNIO_UINT16 SubslotNum,
                                        PNIO_BOOL   MoreFollows)
{
    PNIO_PRM_END_IND_PRM *pparams;
    /* Allocate memory for prms to be sent */
    OsAlloc((void**)&pparams, 0, sizeof(PNIO_PRM_END_IND_PRM));

    /* Copy params */
    pparams->ArNum = ArNum;
    pparams->SessionKey = SessionKey;
    pparams->Api = Api;
    pparams->SlotNum = SlotNum;
    pparams->SubslotNum = SubslotNum;
    pparams->MoreFollows = MoreFollows;

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_PARAM_END_IND, sizeof(PNIO_PRM_END_IND_PRM), (PNIO_UINT8*)pparams);
}   /* PNIOext_cbf_param_end_ind */


/**
 *  @brief Device ready for input update
 *
 *  @param[in]      ArNum               AR number 1....NumOfAR
 *  @param[in]      InpUpdState         Input update state (AR_STARTUP or AR_INDATA )
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_ready_for_input_update_ind(PNIO_UINT16 ArNum, PNIO_INP_UPDATE_STATE InpUpdState, PNIO_UINT32 Apdu)
{
    PNIO_INPUT_UPDATE_IND_PRM *params;

    OsAlloc((void**)&params, 0, sizeof(PNIO_INPUT_UPDATE_IND_PRM));

    /* Copy params */
    params->ArNum = ArNum;
    params->InpUpdState = InpUpdState;
    params->Apdu = Apdu;

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_READY_FOR_INPUT_UPDATE_IND, sizeof(PNIO_INPUT_UPDATE_IND_PRM), (PNIO_UINT8*)(params));

}   /* PNIOext_cbf_ready_for_input_update_ind */


/**
 *  @brief Application relation in data indication
 *
 *  @param[in]      ArNum               AR number 1....NumOfAR
 *  @param[in]      SessionKey          0 - not used here
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_ar_indata_ind(PNIO_UINT16 ArNum, PNIO_UINT16 SessionKey)
{
    PNIO_AR_INDATA_IND_PRM *params;

    OsAlloc((void**)&params, 0, sizeof(PNIO_AR_INDATA_IND_PRM));

    /* Copy params */
    params->ArNum = ArNum;
    params->SessionKey = SessionKey;

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_AR_INDATA_IND, sizeof(PNIO_AR_INDATA_IND_PRM), (PNIO_UINT8*)(params));

}   /* PNIOext_cbf_ar_indata_ind */


/**
 *  @brief Application relation disconnected indication
 *
 *  @param[in]      ArNum               AR number 1....NumOfAR
 *  @param[in]      SessionKey          0 - not used here
 *  @param[in]      ReasonCode          reason code (see PNIO_AR_REASON)
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_ar_disconn_ind(PNIO_UINT16 ArNum, PNIO_UINT16 SessionKey, PNIO_AR_REASON ReasonCode)
{
    PNIO_AR_DISCONNECT_IND_PRM *params;

    OsAlloc((void**)&params, 0, sizeof(PNIO_AR_DISCONNECT_IND_PRM));

    /* Copy params */
    params->ArNum = ArNum;
    params->SessionKey = SessionKey;
    params->ReasonCode = ReasonCode;

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_AR_DISCONNECT_IND, sizeof(PNIO_AR_DISCONNECT_IND_PRM), (PNIO_UINT8*)(params));

}   /* PNIOext_cbf_ar_disconn_ind */


/**
 *  @brief New ARFSU was received
 *
 *  @param[in]      ARFSU_enabled       AR number 1....NumOfAR
 *  @param[in]      ARFSU_changed       0 - not used here
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_report_ARFSU_record(PNIO_UINT8 ARFSU_enabled, PNIO_UINT8 ARFSU_changed)
{
    PNIO_ARFSU_IND_PRM *params;

    OsAlloc((void**)&params, 0, sizeof(PNIO_ARFSU_IND_PRM));

    /* Copy params */
    params->ARFSU_enabled = ARFSU_enabled;
    params->ARFSU_changed = ARFSU_changed;

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_REPORT_ARFSU_RECORD, sizeof(PNIO_ARFSU_IND_PRM), (PNIO_UINT8*)(params));

}   /* PNIOext_cbf_report_ARFSU_record */


/**
 *  @brief Indicate that asynchronous PN request was finished
 *
 *  @param[in]      ArNum               AR number 1....NumOfAR
 *  @param[in]      AlarmType           alarm type
 *  @param[in]      Api                 API number
 *  @param[in]      *pAddr              location (slot, subslot)
 *  @param[in]      Status              status
 *  @param[in]      Diag_tag            Diagnostic tag - to distinguish alarms
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_async_req_done(PNIO_UINT32     ArNum,
                                    PNIO_ALARM_TYPE AlarmType,
                                    PNIO_UINT32     Api,
                                    PNIO_DEV_ADDR   *pAddr,
                                    PNIO_UINT32     Status,
                                    PNIO_UINT16     Diag_tag)
{
    PNIO_ASYNC_DONE_PRM *params;

    OsAlloc((void**)&params, 0, sizeof(PNIO_ASYNC_DONE_PRM));

    /* Copy params */
    params->AlarmType = AlarmType;
    params->Api = Api;
	params->Status = Status;
    params->ArNum = ArNum;
    params->Diag_tag = Diag_tag;
    params->ArNum = ArNum;

	/* Address copy */
    /* Avoid OsMemCpy - union in address structure can cause problems */
    params->Addr.Type = pAddr->Type;
    params->Addr.ad = pAddr->ad;

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_ASYNC_REQUEST_DONE, sizeof(PNIO_ASYNC_DONE_PRM), (PNIO_UINT8*)(params));

}   /* PNIOext_cbf_async_req_done */


/**
 *  @brief Read request from CPU
 *
 *  @param[in]      Api                 application process identifier
 *  @param[in]      ArNum               AR number 1....NumOfAR
 *  @param[in]      SessionKey          ar session number
 *  @param[in]      SequenceNum         CLRPC sequence number
 *  @param[in]      *pAddr              geographical or logical address
 *  @param[in]      RecordIndex         record index
 *  @param[in, out] *BufLen             length to read, length read
 *  @param[out]     *pBuffer            buffer pointer
 *  @param[out]     *pPnioState         4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_rec_read(
        PNIO_UINT32         Api,
        PNIO_UINT16         ArNum,
        PNIO_UINT16         SessionKey,
        PNIO_UINT32         SequenceNum,
        PNIO_DEV_ADDR       *pAddr,
        PNIO_UINT32         RecordIndex,
        PNIO_UINT32         *pBufLen,
        PNIO_UINT8          *pBuffer,
        PNIO_ERR_STAT       *pPnioState
    )
{
    PNIO_RECORD_READ_PRM *params;

    /* Allocate memory for data to be sent */
    OsAlloc((void**) &params, 0, sizeof(PNIO_RECORD_READ_PRM));

    /* Fill buffer with data to be sent */
    params->Api = Api;
    params->ArNum = ArNum;
    params->SessionKey = SessionKey;
    params->SequenceNum = SequenceNum;
    params->RecordIndex = RecordIndex;

    /* Address copy */
    params->Addr.Type = pAddr->Type;
    params->Addr.ad = pAddr->ad;

    /* Amount of data to be read */
    params->BufLen = *pBufLen;

    /* Transfer also addresses of buffer, which will be filled by response */
    params->pBuffer = (PNIO_UINT32)pBuffer;
    params->pBufLen = (PNIO_UINT32)pBufLen;
    params->pPnioState = (PNIO_UINT32)pPnioState;

    /* Add buffer to transfered params */
    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_REC_READ, sizeof(PNIO_RECORD_READ_PRM), (PNIO_UINT8*)(params));

    /* Wait for response - response will trigger PNDV */

}   /* PNIOext_cbf_rec_read */


/**
 *  @brief Write request from CPU
 *
 *  @param[in]      Api                 application process identifier
 *  @param[in]      ArNum               AR number 1....NumOfAR
 *  @param[in]      SessionKey          ar session number
 *  @param[in]      SequenceNum         CLRPC sequence number
 *  @param[in]      *pAddr              geographical or logical address
 *  @param[in]      RecordIndex         record index
 *  @param[in, out] *BufLen             length to read, length read
 *  @param[out]     *pBuffer            buffer pointer
 *  @param[out]     *pPnioState         4 byte PNIOStatus (ErrCode, ErrDecode, ErrCode1, ErrCode2), see IEC61158-6
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_rec_write(
        PNIO_UINT32         Api,
        PNIO_UINT16         ArNum,
        PNIO_UINT16         SessionKey,
        PNIO_UINT32         SequenceNum,
        PNIO_DEV_ADDR       *pAddr,
        PNIO_UINT32         RecordIndex,
        PNIO_UINT32         *pBufLen,
        PNIO_UINT8          *pBuffer,
        PNIO_ERR_STAT       *pPnioState
    )
{
    PNIO_RECORD_WRITE_PRM *params;
    PNIO_UINT32 prm_size;

    /* Allocate memory for data to be sent */
    prm_size = sizeof(PNIO_RECORD_WRITE_PRM) - sizeof(PNIO_UINT32) + *pBufLen;
    OsAlloc((void**) &params, 0, prm_size);

    /* Fill buffer with data to be sent */
    params->Api = Api;
    params->ArNum = ArNum;
    params->SessionKey = SessionKey;
    params->SequenceNum = SequenceNum;
    params->RecordIndex = RecordIndex;

    /* Address copy */
    params->Addr.Type = pAddr->Type;
    params->Addr.ad = pAddr->ad;

    /* Amount of data to be read */
    params->BufLen = *pBufLen;

    /* Transfer also addresses of buffer, which will be filled by response */
    params->pBufLen = (PNIO_UINT32)pBufLen;
    params->pPnioState = (PNIO_UINT32)pPnioState;


    /* Add buffer to transfered params */
    OsMemCpy(&(params->pBuffer), pBuffer, *pBufLen);
    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_REC_WRITE, prm_size, (PNIO_UINT8*)(params));

    /* Wait for response - response will trigger PNDV */

}   /* PNIOext_cbf_rec_write */


/**
 *  @brief Indicate that device name storage/configuration was finished
 *
 *  @param[in]      NameLength
 *  @param[in]      pStationName
 *  @param[in]      Remanent
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_save_station_name(
        PNIO_UINT16 NameLength,
        PNIO_INT8 *pStationName,
        PNIO_UINT8 Remanent)
{
    PNIO_SAVE_STATION_NAME_PRM *params;

    OsAlloc((void**) &params, 0x00, sizeof(PNIO_SAVE_STATION_NAME_PRM) + NameLength + 1);

    /* Copy params */
    params->NameLength = NameLength;
    params->Remanent = Remanent;
    OsMemCpy(&params->StationName, pStationName, NameLength);

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_SAVE_STATION_NAME,
            sizeof(PNIO_SAVE_STATION_NAME_PRM) + NameLength + 1, (PNIO_UINT8*)(params));

}   /* PNIOext_cbf_save_station_name */

/**
 *  @brief Indicate that ip address storage/configuration was finished
 *
 *  @param[in]      NewIpAddr
 *  @param[in]      SubnetMask
 *  @param[in]      DefRouterAddr
 *  @param[in]      Remanent
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_save_ip_address(
        PNIO_UINT32 NewIpAddr,
        PNIO_UINT32 SubnetMask,
        PNIO_UINT32 DefRouterAddr,
        PNIO_UINT8 Remanent)
{
    PNIO_UINT32 pNewIpAddr = NewIpAddr;
    PNIO_UINT32 pSubnetMask = SubnetMask;
    PNIO_UINT32 pDefRouterAddr = DefRouterAddr;

    PNIO_SAVE_IP_ADDR_PRM *params;

    OsAlloc((void**) &params, 0x00, sizeof(PNIO_SAVE_IP_ADDR_PRM) );

    /* Copy params */
    OsMemCpy(&params->NewIpAddr, &pNewIpAddr, 4);
    OsMemCpy(&params->SubnetMask, &pSubnetMask, 4);
    OsMemCpy(&params->DefRouterAddr, &pDefRouterAddr, 4);
    params->Remanent = Remanent;

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_SAVE_IP_ADDR, sizeof(PNIO_SAVE_IP_ADDR_PRM), (PNIO_UINT8*)(params));

}   /* PNIOext_cbf_save_ip_address */

/**
 *  @brief Report new ip address to BBB
 *
 *  @param[in]      NewIpAddr
 *  @param[in]      SubnetMask
 *  @param[in]      DefRouterAddr
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_report_new_ip_address(
        PNIO_UINT32 NewIpAddr,
        PNIO_UINT32 SubnetMask,
        PNIO_UINT32 DefRouterAddr)
{
    PNIO_UINT32 pNewIpAddr = NewIpAddr;
    PNIO_UINT32 pSubnetMask = SubnetMask;
    PNIO_UINT32 pDefRouterAddr = DefRouterAddr;
    PNIO_REPORT_NEW_IP_ADDR_PRM *params;

    OsAlloc((void**) &params, 0x00, sizeof(PNIO_REPORT_NEW_IP_ADDR_PRM) );

    /* Copy params */
    OsMemCpy(&params->NewIpAddr, &pNewIpAddr, 4);
    OsMemCpy(&params->SubnetMask, &pSubnetMask, 4);
    OsMemCpy(&params->DefRouterAddr, &pDefRouterAddr, 4);

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_REPORT_NEW_IP_ADDR, sizeof(PNIO_REPORT_NEW_IP_ADDR_PRM), (PNIO_UINT8*)(params));

}   /* PNIOext_cbf_report_new_ip_address */

/**
 *  @brief Report reset to factory settings to BBB
 *
 *  @param[in]      RtfOption
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_reset_factory_settings(PNIO_UINT32 RtfOption)
{
    PNIO_RESET_FACTORY_SETTINGS_PRM *params;

    OsAlloc((void**) &params, 0x00, sizeof(PNIO_RESET_FACTORY_SETTINGS_PRM) );

    /* Copy params */
    params->RtfOption = RtfOption;

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_RESET_FACTORY_SETTINGS, sizeof(PNIO_RESET_FACTORY_SETTINGS_PRM), (PNIO_UINT8*)(params));

}   /* PNIOext_cbf_reset_factory_settings */

/**
 *  @brief Report result new device address to BBB
 *
 *  @param[in]      RtfOption
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_result_new_device_address (PNIO_NEW_DEV_ADDR_COMMAND command, PNIO_UINT8 result)
{
    PNIO_RESULT_NEW_DEV_ADDR_PRM *params;

    OsAlloc((void**) &params, 0x00, sizeof(PNIO_RESULT_NEW_DEV_ADDR_PRM) );

    /* Copy params */
    params->command = (PNIO_UINT8) command;
    params->result = result;

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_RESULT_NEW_DEVICE_ADDRESS, sizeof(PNIO_RESULT_NEW_DEV_ADDR_PRM), (PNIO_UINT8*)(params));

}   /* PNIOext_cbf_result_new_device_address */

/**
 *  @brief read record index 0x8029
 *
 *  @param[in]      *pAddr
 *  @param[in]      BufLen
 *  @param[in]      *pBuffer
 *  @param[in]      pSubstMode
 *  @param[in]      pSubstActive
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_UINT32 PNIOext_cbf_substval_out_read(
        PNIO_DEV_ADDR  *pAddr,             // [in] geographical or logical address
        PNIO_UINT32    BufLen,             // [in] length of the submodule output substitute data
        PNIO_UINT8     *pBuffer,           // [in] Ptr to submodule output substitute data
        PNIO_UINT16*   pSubstMode,         // [in, out - BIG ENDIAN] SubstitutionMode: 0=ZERO or inactive (default), 1:LastValue, 2:Replacement value SubstitutionMode: 0=ZERO or inactive, 1:LastValue, 2:Replacement value
        PNIO_UINT16*   pSubstActive)       // [in, out - BIG ENDIAN] SubstituteActiveFlag:  0=operation, 1=substitute. default value is 0: if (IOPS & IOCS = GOOD), else: 1
{
    PNIO_SUBSTVAL_OUT_READ_PRM *params;

    OsAlloc((void**) &params, 0x00, sizeof(PNIO_SUBSTVAL_OUT_READ_PRM) );

    /* Copy params */
    params->BufLen = BufLen;
    params->BufAddr = (PNIO_UINT32) pBuffer;
    params->SubstModeAddr = (PNIO_UINT32) pSubstMode;
    params->SubstActiveAddr = (PNIO_UINT32) pSubstActive;

    /* Address copy */
    params->Addr.Type = pAddr->Type;
    params->Addr.ad = pAddr->ad;

    /* Send */
    return(PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_SUBSTVAL_OUT_READ, sizeof(PNIO_SUBSTVAL_OUT_READ_PRM), (PNIO_UINT8*)(params)));
} /* PNIOext_cbf_substval_out_read */

/**
 *  @brief read record index 0xF880
 *
 *  @param[in]      *pAddr
 *  @param[in]      *BufLen
 *  @param[in]      *pBuffer
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_amr_response_handler(
        PNIO_DEV_ADDR         *pAddr,
        PNIO_UINT32           *pBufLen,
        PNIO_UINT8            *pBuffer)
{
    PNIO_AMR_HANDLER_PRM *params;

    OsAlloc((void**) &params, 0x00, sizeof(PNIO_AMR_HANDLER_PRM));

    /* Copy params */
    params->pBufLen = *pBufLen;
    params->pBufAddr = (PNIO_UINT32) pBuffer;

    /* Address copy */
    params->pAddr.Type = pAddr->Type;
    params->pAddr.ad = pAddr->ad;

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_AMR_READ, sizeof(PNIO_AMR_HANDLER_PRM), (PNIO_UINT8*)(params));
}

/**
 *  @brief read record index 0x80A0
 *
 *  @param[in]      *pAddr
 *  @param[in]      *BufLen
 *  @param[in]      *pBuffer
 *  @param[in]      ArNum
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_pe_response_handler(
        PNIO_DEV_ADDR         *pAddr,
        PNIO_UINT32           *pBufLen,
        PNIO_UINT8            *pBuffer,
        PNIO_UINT16            ArNum)
{
    PNIO_PE_RESPONSE_HANDLER_PRM *params;

    OsAlloc((void**) &params, 0x00, sizeof(PNIO_PE_RESPONSE_HANDLER_PRM));

    /* Copy params */
    params->pBufLen = *pBufLen;
    params->pBufAddr = (PNIO_UINT32) pBuffer;
    params->ArNum = (PNIO_UINT32) ArNum;

    /* Address copy */
    params->pAddr.Type = pAddr->Type;
    params->pAddr.ad = pAddr->ad;

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_PE_RESPONSE, sizeof(PNIO_PE_RESPONSE_HANDLER_PRM), (PNIO_UINT8*)(params));
}

/**
 *  @brief read record index 0x80A0
 *
 *  @param[in]      *pAddr
 *  @param[in]      *BufLen
 *  @param[in]      *pBuffer
 *  @param[in]      ArNum
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_pe_request_handler(
        PNIO_DEV_ADDR         *pAddr,
        PNIO_UINT32           *pBufLen,
        PNIO_UINT8            *pBuffer,
        PNIO_UINT16            ArNum)
{
    PNIO_PE_REQUEST_HANDLER_PRM *params;

    OsAlloc((void**) &params, 0x00, sizeof(PNIO_PE_REQUEST_HANDLER_PRM) + *pBufLen);

    /* Copy params */
    params->pBufLen = *pBufLen;
    params->ArNum = (PNIO_UINT32) ArNum;

    /* Address copy */
    params->pAddr.Type = pAddr->Type;
    params->pAddr.ad = pAddr->ad;

    /* Buffer copy */
    OsMemCpy(params->pData, pBuffer, *pBufLen);

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_PE_REQUEST, sizeof(PNIO_PE_REQUEST_HANDLER_PRM) + *pBufLen, (PNIO_UINT8*)(params));
}

/**
 *  @brief Send back synchronized data from Flash memory
 *
 *  @param[in]      DatLen
 *  @param[in]      *pData
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_nv_data_sync(PNIO_UINT8* pData, PNIO_UINT32 DatLen, PNIO_UINT32 errOccured)
{
    PNIO_NV_DATA_SYNC_PRM* params;
    OsAlloc((void**) &params, 0, sizeof(PNIO_NV_DATA_SYNC_PRM) + DatLen);

    /* Check arguments */
    if((pData != NULL) && (DatLen != 0))
    {
        /* Buffer copy */
        OsMemCpy(params->pData, pData, DatLen);
    }

    /* Copy params */
    params->pBufLen = DatLen;
    params->errOccured = errOccured;

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_NV_DATA_SYNC, sizeof(PNIO_NV_DATA_SYNC_PRM) + DatLen, (PNIO_UINT8*)(params));
}

/**
 *  @brief Send callback after flash write is done
 *
 *  @param[in]      DatLen
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_nv_data_flash_done(PNIO_UINT32 Status, PNIO_UINT32 DatLen, PNIO_UINT32 nvDataType)
{
    PNIO_NV_DATA_FLASH_DONE_PRM* params;
    OsAlloc((void**) &params, 0, sizeof(PNIO_NV_DATA_FLASH_DONE_PRM));

    /* Copy params */
    params->DatLen = DatLen;
    params->Status = Status;
    params->nvDataType = nvDataType;

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_NV_DATA_FLASH_DONE, sizeof(PNIO_NV_DATA_FLASH_DONE_PRM), (PNIO_UINT8*)(params));
}

/**
 *  @brief Send callback after flash write is done
 *
 *  @param[in]      DatLen
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_im_data_flash_done(PNIO_UINT32 Status, PNIO_UINT32 DatLen, PNIO_UINT32 nvDataType)
{
    PNIO_NV_DATA_FLASH_DONE_PRM* params;
    OsAlloc((void**) &params, 0, sizeof(PNIO_NV_DATA_FLASH_DONE_PRM));

    /* Copy params */
    params->DatLen = DatLen;
    params->Status = Status;
    params->nvDataType = nvDataType;

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_IM_DATA_FLASH_DONE, sizeof(PNIO_NV_DATA_FLASH_DONE_PRM), (PNIO_UINT8*)(params));
}

/**
 *  @brief Send callback after flash write is done
 *
 *  @param[in]      DatLen
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_nv_data_factory_reset_flash_done(PNIO_UINT32 Status, PNIO_UINT32 DatLen)
{
    PNIO_NV_DATA_RESET_DONE_PRM* params;
    OsAlloc((void**) &params, 0, sizeof(PNIO_NV_DATA_RESET_DONE_PRM));

    /* Copy params */
    params->DatLen = DatLen;
    params->Status = Status;

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_NV_DATA_RESET_DONE, sizeof(PNIO_NV_DATA_RESET_DONE_PRM), (PNIO_UINT8*)(params));
}

/**
 *  @brief Send callback after PNIO_cbf_store_rema_mem is called
 *
 *  @param[in]      BufLen
 *  @param[in]      Data
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_store_rema_mem(PNIO_UINT32 BufLen, PNIO_UINT8* Data)
{
    PNIO_STORE_REMA_MEM_PRM* params;
    OsAlloc((void**) &params, 0, sizeof(PNIO_STORE_REMA_MEM_PRM) + BufLen);

    /* Copy params */
    params->pBufLen = BufLen;
    OsMemCpy(&params->pData[0], Data, BufLen);

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_STORE_REMA_MEM, sizeof(PNIO_STORE_REMA_MEM_PRM) + BufLen, (PNIO_UINT8*)(params));
}

/**
 *  @brief Send callback after ImX_write_Handler is called
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_im_write(
        PNIO_UINT32         IMidx,
        PNIO_UINT32         Api,              // api number
        PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
        PNIO_UINT32         *pBufLen,         // [in, out] in: length to read, out: length, read by user
        PNIO_UINT8          *pBuffer,         // [in] buffer pointer
        PNIO_UINT32         PeriphRealCfgInd, // entity index in periph interface-real_cfg
        PNIO_ERR_STAT       *pPnioState
)
{
    PNIO_IM_WRITE_PRM* params;
    OsAlloc((void**) &params, 0, sizeof(PNIO_IM_WRITE_PRM) + *pBufLen);

    /* Address copy */
    /* Avoid OsMemCpy - union in address structure can cause problems */
    params->Addr.Type = pAddr->Type;
    params->Addr.ad = pAddr->ad;

    /* Copy params */
    params->pBufLen = *pBufLen;
    params->Api = Api;
    params->IMidx = IMidx;
    params->PeriphRealCfgInd = PeriphRealCfgInd;

    /* Copy data */
    OsMemCpy(&params->pPnioState, pPnioState, sizeof(PNIO_ERR_STAT));
    OsMemCpy(&params->pData[0], pBuffer, *pBufLen);

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_IM_WRITE, sizeof(PNIO_IM_WRITE_PRM) + *pBufLen, (PNIO_UINT8*)(params));
}

/**
 *  @brief Send callback after ImX_read_Handler is called
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *
 */
PNIO_VOID PNIOext_cbf_im_read(
        PNIO_UINT32         IMidx,
        PNIO_UINT32         Api,              // api number
        PNIO_DEV_ADDR       *pAddr,           // geographical or logical address
        PNIO_UINT32         *pBufLen,         // [in, out] in: length to read, out: length, read by user
        PNIO_UINT8          *pBuffer,         // [in] buffer pointer
        PNIO_UINT32         PeriphRealCfgInd, // entity index in periph interface-real_cfg
        PNIO_ERR_STAT       *pPnioState
)
{
    PNIO_IM_READ_PRM* params;
    OsAlloc((void**) &params, 0, sizeof(PNIO_IM_READ_PRM));

    /* Address copy */
    /* Avoid OsMemCpy - union in address structure can cause problems */
    params->Addr.Type = pAddr->Type;
    params->Addr.ad = pAddr->ad;

    /* Copy params */
    params->pBufLen = *pBufLen;
    params->Api = Api;
    params->IMidx = IMidx;
    params->pBufAddr = (PNIO_UINT32) pBuffer;
    params->PeriphRealCfgInd = PeriphRealCfgInd;
    OsMemCpy(&params->pPnioState, pPnioState, sizeof(PNIO_ERR_STAT));

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_IM_READ, sizeof(PNIO_IM_READ_PRM), (PNIO_UINT8*)(params));
}

/**
 *  @brief Send callback after Bsp_im_data_store is called
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *  Used only in case of NV data are stored in BBB and IM data are handled by stack
 *
 */
PNIO_VOID PNIOext_cbf_im_data_store(
		PNIO_UINT32    		NvDataType,      // type of data (device name, IP suite, PDEV records,...)
		void*               pMem,            // pointer to data source
		PNIO_UINT32         MemSize,          // size of memory to store
        PNIO_UINT32         PeriphRealCfgInd, // entity index in periph interface-real_cfg
        PNIO_UINT32         triggerPNDV
)
{
	PNIO_IM_STORE_PRM* params;
    OsAlloc((void**) &params, 0, sizeof(PNIO_IM_STORE_PRM) + MemSize);

    /* Copy params */
    params->pBufLen = MemSize;
    params->NvDataType = ( triggerPNDV << 24 ) | NvDataType;
    params->PeriphRealCfgInd = PeriphRealCfgInd;

    /* Copy data */
    OsMemCpy(&params->pData[0], pMem, MemSize);

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_IM_STORE, sizeof(PNIO_IM_STORE_PRM) + MemSize, (PNIO_UINT8*)(params));
}

/**
 *  @brief Send callback after Bsp_nv_data_store is called
 *
 *  @return         void
 *
 *  Collect parameters, send to XHIF
 *  Used only in case of NV data are stored in BBB and NV data are handled by stack
 *
 */
PNIO_VOID PNIOext_cbf_nv_data_store(
		PNIO_UINT32    		NvDataType,      // type of data (device name, IP suite, PDEV records,...)
		void*               pMem,            // pointer to data source
		PNIO_UINT32         MemSize          // size of memory to store
)
{
	PNIO_NV_DATA_STORE_PRM* params;
    OsAlloc((void**) &params, 0, sizeof(PNIO_NV_DATA_STORE_PRM) + MemSize);

    /* Copy params */
    params->pBufLen = MemSize;
    params->NvDataType = NvDataType;

    /* Copy data */
    OsMemCpy(&params->pData[0], pMem, MemSize);

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_NV_DATA_STORE_HOST, sizeof(PNIO_NV_DATA_STORE_PRM) + MemSize, (PNIO_UINT8*)(params));
}

/**
 *  @brief Send callback after Bsp_nv_data_store is called
 *
 *  @return         void
 *
 *  Collect parm of NvDataType , send to XHIF
 *  Used only in case of NV data are set to default in BBB and NV data are handled by stack
 *
 */
PNIO_VOID PNIOext_cbf_nv_data_set_default(
		PNIO_UINT32    		NvDataType
)
{
	PNIO_NV_DATA_STORE_PRM* params;
	OsAlloc((void**) &params, 0, sizeof(PNIO_NV_DATA_STORE_PRM));

    /* Copy parm */
	params->NvDataType = NvDataType;

    /* Send */
    PnUsr_xhif_acyc_write(PNPB_XHIF_ACYC_NV_DATA_SET_DEFAULT, sizeof(PNIO_NVDATA_TYPE), (PNIO_UINT8*)(params));
}


#endif  /* #if(1 == IOD_USED_WITH_XHIF_HOST) */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
