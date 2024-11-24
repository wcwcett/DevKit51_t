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
/*  F i l e               &F: pnpb_lib_mem_int.c                        :F&  */
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
* @file     pnpb_lib_mem_int.c
* @brief    PNPB library for XHIF - memory interface - upper level
*
* XHIF functionality allows user to use ERTEC Devkit only for PN-stack functionalities
* and to realize user functionality on other device. The other device have to upload
* firmware to ERTEC Devkit as a binary and then communicate data.. This can be realized
* via XHIF memory interface.
* This file is such implementation developed for BeagleBone Black driven by TI Sitara
* processor.
* Upper level interface - functionalities for buffers realized over memory interface
*/

#include "pnpb_lib.h"
#include "pnpb_lib_int.h"
#include "pnpb_lib_mem_int.h"
#include "pnpb_lib_acyc.h"
#include <string.h>
#include "pnpb_gpio_lib.h"

/******************** Internal prototypes ********************/
PNIO_UINT32 pnpb_xhif_addr_slot_to_submodule(PNIO_UINT32 slot, PNIO_UINT32 subslot, PNIO_UINT32 *submodule);
PNIO_UINT32 pnpb_xhif_addr_submodule_to_slot(PNIO_UINT32 *slot, PNIO_UINT32 *subslot, PNIO_UINT32* submodule);
PNIO_UINT32 pnpb_xhif_check_cyclical_read_pointer(PNIO_UINT32 *p_read_ptr);
PNIO_VOID   pnpb_xhif_increment_cyclical_read_pointer(PNIO_UINT32 *p_read_ptr);
PNIO_UINT32 pnpb_xhif_check_cyclical_write_pointer(PNIO_UINT32 *p_write_ptr);
PNIO_VOID   pnpb_xhif_increment_cyclical_write_pointer(PNIO_UINT32 write_ptr);
PNIO_UINT32 pnpb_xhif_cleanup_after_call(PNIO_UINT8 *prm);
PNIO_VOID pnpb_xhif_call_function(PNPB_XHIF_ACYC_TELEGRAMS id, PNIO_UINT8 *prm);
PNIO_UINT32 pnpb_xhif_check_acyc_read_pointer(PNIO_UINT32 *p_read_ptr);
PNIO_VOID   pnpb_xhif_increment_acyc_read_pointer(PNIO_UINT32 *p_read_ptr);
PNIO_UINT32 pnpb_xhif_check_acyc_write_pointer(PNIO_UINT32 *p_write_ptr);
PNIO_VOID   pnpb_xhif_increment_acyc_write_pointer(PNIO_UINT32 write_ptr);
PNPB_XHIF_ACYC_TELEGRAMS pnpb_xhif_acyc_has_more_follows(PNPB_XHIF_ACYC_TELEGRAMS id);
PNIO_UINT32 pnpb_xhif_acyc_is_more_follows(PNPB_XHIF_ACYC_TELEGRAMS id);
PNIO_VOID pnpb_xhif_acyc_read_continue(PNIO_UINT32 id, PNIO_UINT32 is_more_follows, PNIO_UINT32 read_ptr);
PNIO_VOID pnpb_xhif_acyc_read_telegram(PNIO_UINT32 id, PNIO_UINT32 read_ptr);
PNIO_VOID   pnpb_xhif_cyclical_out_trylock(PNIO_UINT32 read_ptr);
PNIO_VOID   pnpb_xhif_cyclical_out_unlock(PNIO_UINT32 read_ptr);
PNIO_VOID   pnpb_xhif_cyclical_in_trylock(PNIO_UINT32 write_ptr);
PNIO_VOID   pnpb_xhif_cyclical_in_unlock(PNIO_UINT32 write_ptr);
PNIO_VOID   pnpb_xhif_acyc_in_trylock(PNIO_UINT32 write_ptr);
PNIO_VOID   pnpb_xhif_acyc_in_unlock(PNIO_UINT32 write_ptr);
PNIO_VOID   pnpb_xhif_acyc_out_trylock(PNIO_UINT32 read_ptr);
PNIO_VOID   pnpb_xhif_acyc_out_unlock(PNIO_UINT32 read_ptr);
PNIO_VOID pnpb_xhif_acyc_write_internal(PNPB_XHIF_ACYC_TELEGRAMS id, PNIO_UINT16 prm_len_tot, PNIO_UINT16 prm_len, PNIO_UINT8 *p_prm);

#define LOCK_ALSO_FOR_READING 1

/**
 *  @brief Slot and subslot address to number of submodule
 *
 *  @param[in]      slot            Slot number
 *  @param[in]      subslot         Subslot number
 *  @param[out]     *submodule      Submodule number
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Address transfer
 *
 */
PNIO_UINT32 pnpb_xhif_addr_slot_to_submodule(PNIO_UINT32 slot, PNIO_UINT32 subslot, PNIO_UINT32 *submodule)
{
    PNIO_UINT32 i;
    for(i = 0; i < PNPB_XHIF_MAX_NUM_OF_SUBMODULES; i++)
    {
        /* unattended position -> = end of the list -> not found */
        if(PNPB_NULL == pnpb_submodule_params[i])
        {
            continue;
        }
        else
        {
            /* found */
            if((slot == pnpb_submodule_params[i]->Slot) && (subslot == pnpb_submodule_params[i]->Subslot))
            {
                *submodule = i;
                return PNPB_OK;
            }
        }
    }
    return PNPB_NOT_OK;
}   /* pnpb_xhif_addr_slot_to_submodule */


/**
 *  @brief Submodule address to number of  slot and subslot
 *
 *  @param[in]      *slot           Slot number
 *  @param[in]      *subslot        Subslot number
 *  @param[out]     submodule       Submodule number
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Address transfer
 *
 */
PNIO_UINT32 pnpb_xhif_addr_submodule_to_slot(PNIO_UINT32 *slot, PNIO_UINT32 *subslot, PNIO_UINT32* submodule)
{
    PNIO_UINT32 i;

    /* Avoid empty items in array - skip index */
    for( i = 0; i < (PNPB_XHIF_MAX_NUM_OF_SUBMODULES); i++ )
    {
        if(PNPB_NULL != pnpb_submodule_params[i])
        {
            break;
        }
    }

    if(*submodule >= (PNPB_XHIF_MAX_NUM_OF_SUBMODULES))
    {
        printf("Error: Submodule slot not found\n");
        return PNPB_NOT_OK;
    }

    /* submodule position not occupied */
    if((0 == pnpb_submodule_params[*submodule]->Slot) && (0 == pnpb_submodule_params[*submodule]->Subslot))
    {
        printf("Error: Submodule slot empty -> SUBMODULE NOT FOUND\n");
        return PNPB_NOT_OK;
    }

    *slot = pnpb_submodule_params[*submodule]->Slot;
    *subslot = pnpb_submodule_params[*submodule]->Subslot;

    return PNPB_OK;
}   /* pnpb_xhif_addr_submodule_to_slot */


/**
 *  @brief Initialization of device - used submodules
 *
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Example submodules are set in here
 *
 */
//lint -e{832} Parameter 'Symbol' not explicitly declared, int assumed
PNIO_UINT32 pnpb_xhif_IO_buffer_init(PNIO_SUB_LIST_ENTRY* pIoSubList,
                                    PNIO_UINT32          NumOfSubListEntries)
{
    PNIO_UINT32 submodule_data_size;
    PNIO_UINT32 i;
    PNIO_SUB_LIST_ENTRY* pIoSub;

    for (i = 0; i < PNPB_XHIF_MAX_NUM_OF_SUBMODULES; ++i)
    {
        pnpb_submodule_params[i] = NULL;
    }

    for(i = 0; i < NumOfSubListEntries; i++)
    {
        pIoSub = pIoSubList + i;
        pnpb_submodule_params[i] = calloc(sizeof(pnpb_subslot_params), 1);
        pnpb_submodule_params[i]->Slot = pIoSub->Slot;
        pnpb_submodule_params[i]->Subslot = pIoSub->Subslot;
        pnpb_submodule_params[i]->InData_size = pIoSub->InDatLen;
        pnpb_submodule_params[i]->OutData_size = pIoSub->OutDatLen;
        pnpb_submodule_params[i]->ModId = pIoSub->ModId;

        pnpb_submodule_params[i]->Direction = PNPB_XHIF_DIRECTION_NO_DATA;
        if(0 < pIoSub->InDatLen)
        {
            pnpb_submodule_params[i]->Direction |=  PNPB_XHIF_DIRECTION_IN;
        }
        if(0 < pIoSub->OutDatLen)
        {
            pnpb_submodule_params[i]->Direction |=  PNPB_XHIF_DIRECTION_OUT;
        }
        pnpb_submodule_params[i]->IOcS = PNIO_S_GOOD;
        pnpb_submodule_params[i]->IOpS = PNIO_S_BAD;

        pnpb_submodule_params[i]->Update = PNPB_TRUE;
    }

    pnpb_submodule_params[i] = PNPB_NULL;
    pnpb_dev_params.NoOfSubmodules = i;

    if(PNPB_XHIF_MAX_NUM_OF_SUBMODULES <= pnpb_dev_params.NoOfSubmodules)
    {
        printf("Error: Too many submodules\n");
        PNPB_LIB_FATAL
    }

    for(i = 0; i < pnpb_dev_params.NoOfSubmodules; i++)
    {
        /*check*/
        if((0 > pnpb_submodule_params[i]->InData_size) || (255 < pnpb_submodule_params[i]->InData_size) ||
                (0 > pnpb_submodule_params[i]->OutData_size) || (255 < pnpb_submodule_params[i]->OutData_size))
        {
            /* error */
            printf("Error: Invalid submodule data size\n");
            PNPB_LIB_FATAL
        }
        if(0 < (pnpb_submodule_params[i]->InData_size + pnpb_submodule_params[i]->OutData_size))    /* have data */
        {
            pnpb_submodule_IO_data[i] = calloc((pnpb_submodule_params[i]->InData_size + pnpb_submodule_params[i]->OutData_size), 1);
        }
        else
        {
            pnpb_submodule_IO_data[i] = PNPB_NULL;
        }
    }
    return PNPB_OK;
}   /* pnpb_xhif_IO_buffer_init */


/**
 *  @brief Checks if there are data ready to be read
 *
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Underflow protection
 *
 */
PNIO_UINT32 pnpb_xhif_check_cyclical_read_pointer(PNIO_UINT32 *p_read_ptr)
{
    PNIO_UINT32 tmp_write_ptr, tmp_read_ptr;
    PNIO_UINT32 i, status = PNPB_NOT_OK;

    /* Read index from GPMC */
    tmp_read_ptr = PNPB_XHIF_CYCLIC_OUT_SERV.read_ptr;

    /* Check against read from invalid indexes */
    if(PNPB_XHIF_SIZE_OF_BUFFER_FOR_CYCLIC <= tmp_read_ptr)
    {
        printf("Invalid cyclical read index %d\n", tmp_read_ptr);
        PNPB_LIB_FATAL
    }

    /* Check against buffer underflow */
    tmp_write_ptr = PNPB_XHIF_CYCLIC_OUT_SERV.write_ptr;

    for (i = 0; i < 2; i++)
    {
        if (tmp_read_ptr != tmp_write_ptr)
        {
            status = PNPB_OK;
            break;
        }

        pnpb_xhif_wait();

        /* Read again */
        tmp_read_ptr = PNPB_XHIF_CYCLIC_OUT_SERV.read_ptr;
    }

    /* Return read index */
    *p_read_ptr = tmp_read_ptr;
    return status;
}   /* pnpb_xhif_check_cyclical_read_pointer */


/**
 *  @brief Increment read pointer
 *
 *  @return         void
 *
 *  Respects ring buffer
 *
 */
PNIO_VOID pnpb_xhif_increment_cyclical_read_pointer(PNIO_UINT32 *p_read_ptr)
{
    PNIO_UINT32 tmp_read_ptr = *p_read_ptr;

    /* Increment */
    if((PNPB_XHIF_SIZE_OF_BUFFER_FOR_CYCLIC - 1) <= tmp_read_ptr)
    {
        tmp_read_ptr = 0;    /* buffer went over max to 0*/
    }
    else
    {
        tmp_read_ptr++;
    }

    /* Return new value */
    *p_read_ptr = tmp_read_ptr;

}   /* pnpb_xhif_increment_cyclical_read_pointer */


/**
 *  @brief Check write possibility
 *
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Buffer overflow protection
 *
 */
PNIO_UINT32 pnpb_xhif_check_cyclical_write_pointer(PNIO_UINT32 *p_write_ptr)
{
    PNIO_UINT32 tmp_write_ptr, tmp_read_ptr;

    /* Read index from GPMC */
    tmp_read_ptr = PNPB_XHIF_CYCLIC_IN_SERV.read_ptr;
    tmp_write_ptr = PNPB_XHIF_CYCLIC_IN_SERV.write_ptr;

    /* Check */
    if(PNPB_XHIF_SIZE_OF_BUFFER_FOR_CYCLIC <= tmp_write_ptr)
    {
        printf("Error: invalid cyclical write index %d\n", tmp_write_ptr);
        PNPB_LIB_FATAL
    }

    /* Increment */
    if((PNPB_XHIF_SIZE_OF_BUFFER_FOR_CYCLIC - 1) <= tmp_write_ptr)
    {
        tmp_write_ptr = 0;    /* buffer went over max to 0*/
    }
    else
    {
        tmp_write_ptr++;
    }

    if(tmp_read_ptr == tmp_write_ptr)
    {
        printf("Error: cyclical write buffer overflow %d\n", tmp_write_ptr);
        PNPB_LIB_FATAL
    }
    /* Clean the header of data slot */
	// Below lines are used instead of memset to keep volitile qualifier happy  
	//lint -e{419} Apparent data overrun for function 'Symbol', argument Integer exceeds argument Integer
	memset((void*)&PNPB_XHIF_CYCLIC_IN_DATA[tmp_write_ptr].Lock, 0, 20);
 

	*p_write_ptr = tmp_write_ptr;
    return PNPB_OK;
}   /* pnpb_xhif_check_cyclical_write_pointer */


/**
 *  @brief Increment write pointer
 *
 *  @return         void
 *
 *  Respects ring buffer
 *
 */
PNIO_VOID pnpb_xhif_increment_cyclical_write_pointer(PNIO_UINT32 write_ptr)
{
    /* Write to GPMC */
    PNPB_XHIF_CYCLIC_IN_SERV.write_ptr = write_ptr;

}   /* pnpb_xhif_increment_cyclical_write_pointer */


/**
 *  @brief Lock against read while write
 *
 *  @param[in]      void
 *  @return         void
 *
 *  Two devices accesses same memory
 *
 */
PNIO_VOID pnpb_xhif_cyclical_out_trylock(PNIO_UINT32 read_ptr)
{
    PNIO_UINT32 tmp_lock;
    PNIO_UINT32 fails = 0;

    while(PNPB_TRUE)
    {
        /* Read from GPMC */
        tmp_lock = PNPB_XHIF_CYCLIC_OUT_DATA[read_ptr].Lock;

        /* Do not lock the resource, only check if not locked */
        if(0 == tmp_lock)
        {
#if (1 == LOCK_ALSO_FOR_READING)
            /* Write to GPMC */
            PNPB_XHIF_CYCLIC_OUT_DATA[read_ptr].Lock = 0x10;
#endif
            return;
        }
        else
        {
            printf("Error: Cyclical out trylock waiting too long on index %d\n", read_ptr);
            PNPB_LIB_FATAL
        }
    }
}   /* pnpb_xhif_cyclical_trylock */


/**
 *  @brief Unlock for protection against read while write
 *
 *  @param[in]      void
 *  @return         void
 *
 *  Two devices accesses same memory
 *
 */
PNIO_VOID pnpb_xhif_cyclical_out_unlock(PNIO_UINT32 read_ptr)
{
    /* Nothing to be done - read lock is only check, so nothing to unlock */
#if (1 == LOCK_ALSO_FOR_READING)
    /* Write to GPMC */
    PNPB_XHIF_CYCLIC_OUT_DATA[read_ptr].Lock = 0x00;
#endif

}   /* pnpb_xhif_cyclical_unlock */


/**
 *  @brief Lock against read while write
 *
 *  @param[in]      void
 *  @return         void
 *
 *  Two devices accesses same memory
 *
 */
PNIO_VOID pnpb_xhif_cyclical_in_trylock(PNIO_UINT32 write_ptr)
{
    PNIO_UINT32 tmp_lock;
    PNIO_UINT32 fails = 0;

    while(PNPB_TRUE)
    {
        /* Read from GPMC */
        tmp_lock = PNPB_XHIF_CYCLIC_IN_DATA[write_ptr].Lock;

        /* Lock if available, else wait */
        if(0 == tmp_lock)
        {
            /* Write to GPMC */
            PNPB_XHIF_CYCLIC_IN_DATA[write_ptr].Lock = 0x10;
            return;
        }
        else
        {
            printf("Error: Cyclical in trylock waiting too long on index %d\n", write_ptr);
            PNPB_LIB_FATAL
        }
    }
}   /* pnpb_xhif_cyclical_trylock */


/**
 *  @brief Unlock for protection against read while write
 *
 *  @param[in]      void
 *  @return         void
 *
 *  Two devices accesses same memory
 *
 */
PNIO_VOID pnpb_xhif_cyclical_in_unlock(PNIO_UINT32 write_ptr)
{
    PNIO_UINT32 tmp_lock;
    /* Write to GPMC */
    PNPB_XHIF_CYCLIC_IN_DATA[write_ptr].Lock = 0;
    tmp_lock = PNPB_XHIF_CYCLIC_IN_DATA[write_ptr].Lock;
    if(0 != tmp_lock)
    {
        /*FAILED -> Retry*/
        printf("Cyclical in unlock failed on index %d: lock state %x -> RETRY\n",
                            write_ptr, tmp_lock);
        PNPB_XHIF_CYCLIC_IN_DATA[write_ptr].Lock = 0;
        tmp_lock = PNPB_XHIF_CYCLIC_IN_DATA[write_ptr].Lock;
        if(0 != tmp_lock)
        {
            printf("Error: Cyclical in unlock failed on index %d: lock state %x\n",
                    write_ptr, tmp_lock);
            PNPB_LIB_FATAL
        }
    }
}   /* pnpb_xhif_cyclical_unlock */


/**
 *  @brief Memory interface init
 *
 *  @param[in]      *p_xhif_set_page    Pointer to xhif page switch registers
 *  @param[in]      *p_xhif_data        Pointer to xhif data access
 *  @return         void
 *
 *  Set pages and load addresses of memory interface
 *
 */
PNIO_VOID pnpb_xhif_memory_interface_start(PNIO_VOID* p_xhif_set_page, PNIO_VOID* p_xhif_data)
{
    /* Set pages of XHIF */
    pnpb_set_multiple_pages(p_xhif_set_page, 0, 7, PNPB_XHIF_SECTION_CYC_IN);
    /* Set address to ((start of page) - (start address set as XHIF first page)) */
    pnpb_xhif_cyclic_in_p = (PNIO_VOID*)(p_xhif_data + PNPB_XHIF_SECTION_CYC_IN - PNPB_XHIF_SECTION_CYC_IN);
    pnpb_xhif_cyclic_out_p = (PNIO_VOID*)(p_xhif_data + PNPB_XHIF_SECTION_CYC_OUT - PNPB_XHIF_SECTION_CYC_IN);

    pnpb_xhif_acyc_in_p = (PNIO_VOID*)(p_xhif_data + PNPB_XHIF_SECTION_ACY_IN - PNPB_XHIF_SECTION_CYC_IN);
    pnpb_xhif_acyc_out_p = (PNIO_VOID*)(p_xhif_data + PNPB_XHIF_SECTION_ACY_OUT - PNPB_XHIF_SECTION_CYC_IN);

    pnpb_xhif_traces_p = (PNIO_VOID*)(p_xhif_data + PNPB_XHIF_SECTION_TRACES - PNPB_XHIF_SECTION_CYC_IN);
}   /* pnpb_xhif_memory_interface_start */




/**
 *  @brief Writes cyclical data to XHIF
 *
 *  @param[in]      submodule_no    Number of submodule
 *  @param[in]      slot_no         Slot number
 *  @param[in]      subslot_no      Subslot number
 *  @param[in]      DataLen         Amount of data to be transferred
 *  @param[in]      *pData          Pointer to data
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Write to XHIF, in services = write to stack
 *
 */
PNIO_UINT32 pnpb_xhif_cyclical_write(PNIO_UINT32 submodule_no,
                                    PNIO_UINT32 slot_no,
                                    PNIO_UINT32 subslot_no,
                                    PNIO_IOXS Iocs,
                                    PNIO_UINT32 DataLen,
                                    PNIO_UINT8 *pData)
{
    PNIO_UINT32 write_ptr;
	
    /* Check if there is some space in memory interface */
    if(PNPB_NOT_OK == pnpb_xhif_check_cyclical_write_pointer(&write_ptr))
    {
        printf("Error: Write buffer not ready to be used - overflow risk\n");
        return PNPB_NOT_OK;
    }

    /* Exclusive access */
    pnpb_xhif_cyclical_in_trylock(write_ptr);

    /* Only IN have data to be sent to stack, otherwise sent IOxS only */
    if( 0 != (pnpb_submodule_params[submodule_no]->Direction & PNPB_XHIF_DIRECTION_IN))    /* direction IN or INOUT */
    {
        PNPB_XHIF_CYCLIC_IN_DATA[write_ptr].Data_size = DataLen;
        if(0 != DataLen)
        {
            memcpy((void*)PNPB_XHIF_CYCLIC_IN_DATA[write_ptr].Data, (const void*)pData, DataLen);
        }
    }
    else    /* No data, but still have to send to XHIF (transfer of IOxS) */
    {
        PNPB_XHIF_CYCLIC_IN_DATA[write_ptr].Data_size = 0;
    }
    /* Every telegram have to have info about slot, subslot and IOxS */

    PNPB_XHIF_CYCLIC_IN_DATA[write_ptr].Slot = slot_no;
    PNPB_XHIF_CYCLIC_IN_DATA[write_ptr].Subslot = subslot_no;
    PNPB_XHIF_CYCLIC_IN_DATA[write_ptr].IOxS = Iocs;

    pnpb_xhif_cyclical_in_unlock(write_ptr);

    /* Increment write pointer after write */
    pnpb_xhif_increment_cyclical_write_pointer(write_ptr);

    return PNPB_OK;
}   /* pnpb_xhif_cyclical_write */


/**
 *  @brief Sends all the relevant data to xhif
 *
 *  @return         void
 *
 *  Will execute the send
 *
 */
PNIO_UINT32 pnpb_xhif_send_all_IO_data()
{
    PNIO_UINT32 submodule_cnt;
    PNIO_IOXS Iocs = PNIO_S_BAD;
    PNIO_DEV_ADDR  Addr;
    PNIO_UINT8 Data[PNPB_XHIF_NUMOF_BYTES_PER_SUBSLOT];

    for(submodule_cnt = 0; submodule_cnt < PNPB_XHIF_MAX_NUM_OF_SUBMODULES/*pnpb_dev_params.NoOfSubmodules */; submodule_cnt ++)
    {
        if(PNPB_NULL != pnpb_submodule_params[submodule_cnt] )
        {
            if((0 != pnpb_submodule_params[submodule_cnt]->Slot) && (0 != pnpb_submodule_params[submodule_cnt]->Subslot))
            {
                /* Send if received, even for DataLen = 0, IOxS data have to be exchanged */
                /* This ensures, that submodule is parameterized and owned */
                if(PNPB_TRUE == pnpb_submodule_params[submodule_cnt]->Update)
                {
                    Addr.Geo.Slot = pnpb_submodule_params[submodule_cnt]->Slot;
                    Addr.Geo.Subslot = pnpb_submodule_params[submodule_cnt]->Subslot;
                    /* Nice to have: no temporary buffer [Data] */
                    if(PNPB_XHIF_DIRECTION_IN & pnpb_submodule_params[submodule_cnt]->Direction)
                    {
                    	Iocs = PNIO_cbf_data_write(&Addr, pnpb_submodule_params[submodule_cnt]->InData_size,
                                                Data, pnpb_submodule_params[submodule_cnt]->IOpS);
                    }

                /* Update stored IOcS */
                    pnpb_submodule_params[submodule_cnt]->IOcS = Iocs;
                    pnpb_xhif_cyclical_write(submodule_cnt, pnpb_submodule_params[submodule_cnt]->Slot,
                            pnpb_submodule_params[submodule_cnt]->Subslot, Iocs,
                            pnpb_submodule_params[submodule_cnt]->InData_size, Data);

                    pnpb_submodule_params[submodule_cnt]->Update = PNPB_FALSE;  /* Was updated, wait for new cycle data */
                }
            }
        }
    }
    return PNPB_OK;
}   /* pnpb_xhif_send_all_IO_data */


/**
 *  @brief Read cyclicall IO data from XHIF
 *
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Read from XHIF, out services = read from stack
 *
 */
PNIO_UINT32 pnpb_xhif_cyclical_read()
{
    volatile PNIO_UINT32 subslot_no, slot_no;
    PNIO_UINT32 submodule_no;
    PNIO_IOXS IOcS;
    PNIO_DEV_ADDR  Addr;
    PNIO_UINT32 *Data;
    PNIO_UINT32 ArNum, Apdu;
    PNIO_UINT32 read_ptr, data_size;

    /*Read all available*/

    while(PNPB_TRUE)
    {
        if(PNPB_NOT_OK == pnpb_xhif_check_cyclical_read_pointer(&read_ptr))
        {
            //PNPB_LIB_FATAL
            return PNPB_OK;
        }
        /* ptr ++ */
        pnpb_xhif_increment_cyclical_read_pointer(&read_ptr);
        /* Exclusive access */
        pnpb_xhif_cyclical_out_trylock(read_ptr);

        /* Read slot and subslot number of incomming data */
        slot_no = PNPB_XHIF_CYCLIC_OUT_DATA[read_ptr].Slot;
        subslot_no = PNPB_XHIF_CYCLIC_OUT_DATA[read_ptr].Subslot;

        /* Apdu status */
        if((0xF0 == slot_no) && (0xF0 == subslot_no))
        {
            Data = (PNIO_UINT32 *)(PNPB_XHIF_CYCLIC_OUT_DATA[read_ptr].Data);

            ArNum = *Data;
            Data++;
            Apdu = *Data;

            if(PNPB_XHIF_MAX_NUM_OF_AR >= ArNum)
            {
                pnpb_ar_apdu_status[ArNum - 1] = Apdu;
            }
            else
            {
                printf("Error: Invalid AR of Apdu Status %x\n", ArNum);
            }
        }

        else    /* Cyclical data */
        {
            /* Obtain data size */
            data_size = PNPB_XHIF_CYCLIC_OUT_DATA[read_ptr].Data_size;

            /* Find data position - valid submodule_no */
            if(PNPB_NOT_OK == pnpb_xhif_addr_slot_to_submodule(slot_no, subslot_no, &submodule_no))
            {
                if(slot_no == LastPulledSlot && subslot_no == LastPulledSubslot)
                {
                    printf("Ignored cyclic data for last pulled module\n");
                }else
                {
                    /* went through whole list and not found */
                    printf("Submodule search reached end of the list -> SUBMODULE NOT FOUND\nSlot: %d\nSubslot: %d\n", slot_no, subslot_no);
                PNPB_LIB_FATAL
                }


                /* Write to GPMC */
                pnpb_xhif_cyclical_out_unlock(read_ptr);
                PNPB_XHIF_CYCLIC_OUT_SERV.read_ptr = read_ptr;
                continue;
            }

            /* Check size */

            /* INOUT module needs special handling */
            if(PNPB_XHIF_DIRECTION_INOUT == pnpb_submodule_params[submodule_no]->Direction)
            {
                /* InData of INOUT */
                if(0 == data_size)
                {
                    /* Copy IOxS */
                    pnpb_submodule_params[submodule_no]->IOpS = PNPB_XHIF_CYCLIC_OUT_DATA[read_ptr].IOxS;
                }
                /* OutData of INOUT*/
                else if(data_size == pnpb_submodule_params[submodule_no]->OutData_size)
                {
                    Addr.Geo.Slot = slot_no;
                    Addr.Geo.Subslot = subslot_no;

                    /* Protected access to GPMC */

                    IOcS = PNIO_cbf_data_read
                            (&Addr, pnpb_submodule_params[submodule_no]->OutData_size,
                            (void*) (PNIO_UINT8 *) PNPB_XHIF_CYCLIC_OUT_DATA[read_ptr].Data,
                             PNPB_XHIF_CYCLIC_OUT_DATA[read_ptr].IOxS);
                    pnpb_submodule_params[submodule_no]->IOcS = IOcS;
                    /* Copy IOxS */
                    pnpb_submodule_params[submodule_no]->IOpS = PNPB_XHIF_CYCLIC_OUT_DATA[read_ptr].IOxS;

                    /* This submodule is Ready for data -> should be also written to stack, set flag */
                    pnpb_submodule_params[submodule_no]->Update = PNPB_TRUE;
                }
            }

            /* Here, only OUT expected to have IO data, otherwise only IOxS => size = 0 */
            else if( 0 != (pnpb_submodule_params[submodule_no]->Direction & PNPB_XHIF_DIRECTION_OUT))    /* direction OUT or INOUT */
            {
                /* Check for expected data size */
                if (data_size == pnpb_submodule_params[submodule_no]->OutData_size) /* Data present => Copy them to Ertec buffer*/
                {
                    Addr.Geo.Slot = slot_no;
                    Addr.Geo.Subslot = subslot_no;

                    /* Protected access to GPMC */

                    IOcS = PNIO_cbf_data_read
                            (&Addr, pnpb_submodule_params[submodule_no]->OutData_size,
                            (void*)(PNIO_UINT8 *)PNPB_XHIF_CYCLIC_OUT_DATA[read_ptr].Data,
                             PNPB_XHIF_CYCLIC_OUT_DATA[read_ptr].IOxS);
                    pnpb_submodule_params[submodule_no]->IOcS = IOcS;
                    /* Copy IOxS */
                    pnpb_submodule_params[submodule_no]->IOpS = PNPB_XHIF_CYCLIC_OUT_DATA[read_ptr].IOxS;

                    /* This submodule is Ready for data -> should be also written to stack, set flag */
                    pnpb_submodule_params[submodule_no]->Update = PNPB_TRUE;
                }
            }
            else    /* direction IN or NO_DATA */
            {
                /* Only acceptible data size = 0 */
                if(0 != data_size)
                {
                    printf("Error:IN or NO_DATA IO telegram containing data, size 0x%x = %d for sl %x su %x pos %d\n",
                            data_size, data_size,
                            slot_no, subslot_no, submodule_no);
                }
                else
                {
                    /* No data to transfer, but IOxS and update status have to be actualized */
                    /* Copy IOxS */
                    pnpb_submodule_params[submodule_no]->IOpS = PNPB_XHIF_CYCLIC_OUT_DATA[read_ptr].IOxS;
                    /* This submodule is Ready for data -> should be also written to stack, set flag */
                    pnpb_submodule_params[submodule_no]->Update = PNPB_TRUE;
                }
            }
        }
        /* Write to GPMC */
        pnpb_xhif_cyclical_out_unlock(read_ptr);
        PNPB_XHIF_CYCLIC_OUT_SERV.read_ptr = read_ptr;
    }   /* while(PNPB_TRUE) */

    /* Actually will not happen */
    printf("Error in read function - end of function reached\n");
    PNPB_LIB_FATAL
    return PNIO_NOT_OK;
}   /* pnpb_xhif_cyclical_read */



/* **** ACYCLIC **** *///


/**
 *  @brief Writes cyclical data to XHIF
 *
 *  @param[in]      submodule_no    Number of submodule
 *  @param[in]      slot_no         Slot number
 *  @param[in]      subslot_no      Subslot number
 *  @param[in]      DataLen         Amount of data to be transferred
 *  @param[in]      *pData          Pointer to data
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Write to XHIF, in services = write to stack
 *
 */
PNIO_VOID pnpb_xhif_prepare_function_calls()
{
    PNIO_UINT32 i;
    /* Empty all telegram number reactions */
    for(i = 0; i < PNPB_XHIF_ACYC_NUM_OF_TELEGRAMS; i++)
    {
        pnpb_xhif_acyclic_functions[i].function_call = pnpb_xhif_no_telegram;
    }

    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_AR_CONNECT_IND].function_call = PNIOext_cbf_ar_connect_ind;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_AR_OWNERSHIP_IND].function_call = PNIOext_cbf_ar_ownership_ind;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_PARAM_END_IND].function_call = PNIOext_cbf_param_end_ind;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_READY_FOR_INPUT_UPDATE_IND].function_call = PNIOext_cbf_ready_for_input_update_ind;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_AR_INDATA_IND].function_call = PNIOext_cbf_ar_indata_ind;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_AR_DISCONNECT_IND].function_call = PNIOext_cbf_ar_disconn_ind;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_REPORT_ARFSU_RECORD].function_call = PNIOext_cbf_report_ARFSU_record;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_SUB_PLUG_LIST_CBF].function_call = PNIOext_cbf_sub_plug_list;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_ASYNC_REQUEST_DONE].function_call = PNIOext_cbf_async_req_done;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_ASYNC_REQUEST_ERROR].function_call = PNIOext_cbf_async_req_error;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_REC_READ].function_call = PNIOext_cbf_rec_read;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_REC_WRITE].function_call = PNIOext_cbf_rec_write;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_SUBSTVAL_OUT_READ].function_call = PNIOext_cbf_substval_out_read;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_SAVE_STATION_NAME].function_call = PNIOext_cbf_save_station_name;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_SAVE_IP_ADDR].function_call = PNIOext_cbf_save_ip_addr;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_REPORT_NEW_IP_ADDR].function_call = PNIOext_cbf_report_new_ip_addr;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_RESET_FACTORY_SETTINGS].function_call = PNIOext_cbf_reset_factory_settings;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_RESULT_NEW_DEVICE_ADDRESS].function_call = PNIOext_cbf_result_new_device_address;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_START_LED_BLINK].function_call = PNIOext_cbf_start_led_blink;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_STOP_LED_BLINK].function_call = PNIOext_cbf_stop_led_blink;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_DEVICE_STARTUP_DONE].function_call = PNIOext_cbf_device_startup_done;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_TRACE_READY].function_call =  PNIOext_cbf_trace_ready;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_RETURN_ISR_HANDLE].function_call = PNIOext_cbf_return_isr_handle;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_PERFORM_ISR_CALLBACK].function_call = PNIOext_cbf_perform_isr_callback;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_AMR_READ].function_call = PNIOext_cbf_amr_response_handler;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_PE_RESPONSE].function_call = PNIOext_cbf_pe_response_handler;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_PE_REQUEST].function_call = PNIOext_cbf_pe_request_handler;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_NV_DATA_SYNC].function_call = PNIOext_cbf_nv_data_sync;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_NV_DATA_FLASH_DONE].function_call = PNIOext_cbf_nv_data_flash_done;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_IM_DATA_FLASH_DONE].function_call = PNIOext_cbf_im_data_flash_done;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_NV_DATA_RESET_DONE].function_call = PNIOext_cbf_nv_data_factory_reset_flash_done;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_STORE_REMA_MEM].function_call = PNIOext_cbf_store_rema_mem;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_IM_WRITE].function_call = PNIOext_cbf_im_write;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_IM_READ].function_call = PNIOext_cbf_im_read;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_IM_STORE].function_call = PNIOext_cbf_im_data_store;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_NV_DATA_STORE_HOST].function_call  = PNIOext_cbf_nv_data_store;
    pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_NV_DATA_SET_DEFAULT].function_call = PNIOext_cbf_nv_data_set_default;
}   /* pnpb_xhif_prepare_function_calls */


/**
 *  @brief Cleans resources allocated for received acyclic call
 *
 *  @param[in]      *prm            Pointer to memory used for parameters
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Do after every call with prms != 0
 *
 */
PNIO_UINT32 pnpb_xhif_cleanup_after_call(PNIO_UINT8 *prm)
{
    if(NULL != prm)
    {
        free (prm);
    }
    return PNPB_OK;
}   /* pnpb_xhif_cleanup_after_call */


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
PNIO_VOID pnpb_xhif_call_function(PNPB_XHIF_ACYC_TELEGRAMS id, PNIO_UINT8 *prm)
{
    if(PNPB_USR_START_IDLE == PnpbDeviceStartupState)
    {
        if(PNPB_XHIF_ACYC_DEVICE_STARTUP_DONE != id)
        {
            printf("Error: leftover data\n");
            PNPB_LIB_FATAL
        }
    }

    if(PNPB_XHIF_ACYC_NUM_OF_TELEGRAMS <= id)
    {
        printf("Error: Recieved invalid record ID: %d\n", id);
        PNPB_LIB_FATAL
    }
    else
    {
	    /* Has to be unlocked in case the function sends response! */
        pthread_mutex_unlock(&gpmc_access_mutex);
        pnpb_xhif_acyclic_functions[id].function_call(prm);
        pthread_mutex_lock(&gpmc_access_mutex);
        /* Function was executed, parameters are not needed any more */
        pnpb_xhif_cleanup_after_call(prm);
	}
}   /* pnpb_xhif_call_function */


/**
 *  @brief Lock against read while write
 *
 *  @param[in]      void
 *  @return         void
 *
 *  Two devices accesses same memory
 *
 */
PNIO_VOID pnpb_xhif_acyc_out_trylock(PNIO_UINT32 read_ptr)
{
    PNIO_UINT32 tmp_lock;
    PNIO_UINT32 fails = 0;

    while(PNPB_TRUE)
    {
        /* Protected access to GPMC */
        tmp_lock = PNPB_XHIF_ACYC_OUT_DATA[read_ptr].lock;

        /* Do not lock the resource, only check if not locked */
        if(0 == tmp_lock)
        {
#if (1 == LOCK_ALSO_FOR_READING)
            /* Write to GPMC */
            PNPB_XHIF_ACYC_OUT_DATA[read_ptr].lock = 0x10;
#endif
            return;
        }
        else
        {
            printf("Error: Acyclic out trylock waiting too long on index %d\n", read_ptr);
            PNPB_LIB_FATAL
        }
    }
}   /* pnpb_xhif_acyc_out_trylock */


/**
 *  @brief Unlock for protection against read while write
 *
 *  @param[in]      void
 *  @return         void
 *
 *  Two devices accesses same memory
 *
 */
PNIO_VOID pnpb_xhif_acyc_out_unlock(PNIO_UINT32 read_ptr)
{
    /* Nothing to be done - read lock is only check, so nothing to unlock */
#if (1 == LOCK_ALSO_FOR_READING)
            /* Write to GPMC */
            PNPB_XHIF_ACYC_OUT_DATA[read_ptr].lock = 0x00;
#endif

}   /* pnpb_xhif_acyc_out_unlock */


/**
 *  @brief Lock against read while write
 *
 *  @param[in]      void
 *  @return         void
 *
 *  Two devices accesses same memory
 *
 */
PNIO_VOID pnpb_xhif_acyc_in_trylock(PNIO_UINT32 write_ptr)
{
    PNIO_UINT32 tmp_lock;
    PNIO_UINT32 fails = 0;

    while(PNPB_TRUE)
    {
        /* Protected access to GPMC */
        tmp_lock = PNPB_XHIF_ACYC_IN_DATA[write_ptr].lock;

        if(0 == tmp_lock)
        {
            PNPB_XHIF_ACYC_IN_DATA[write_ptr].lock = 0x10;
            return;
        }
        else
        {
            printf("Error: Acyclic in trylock waiting too long on index %d\n", write_ptr);
            PNPB_LIB_FATAL
        }
    }
}   /* pnpb_xhif_acyc_in_trylock */


/**
 *  @brief Unlock for protection against read while write
 *
 *  @param[in]      void
 *  @return         void
 *
 *  Two devices accesses same memory
 *
 */
PNIO_VOID pnpb_xhif_acyc_in_unlock(PNIO_UINT32 write_ptr)
{
    PNPB_XHIF_ACYC_IN_DATA[write_ptr].lock = 0;
}   /* pnpb_xhif_acyc_in_unlock */


/**
 *  @brief Check against acyclic buffer underflow
 *
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Do before every read
 *
 */
PNIO_UINT32 pnpb_xhif_check_acyc_read_pointer(PNIO_UINT32 *p_read_ptr)
{
    PNIO_UINT32 tmp_read_ptr, tmp_write_ptr;
    PNIO_UINT32 i, status = PNPB_NOT_OK;
    /* Read */
    tmp_read_ptr = PNPB_XHIF_ACYC_OUT_SERV.read_ptr;
    tmp_write_ptr = PNPB_XHIF_ACYC_OUT_SERV.write_ptr;

    /* Check against read from invalid indexes */
    if(PNPB_XHIF_SIZE_OF_BUFFER_FOR_ACYC <= tmp_read_ptr)
    {
        printf("Invalid acyclic read index %d\n", tmp_read_ptr);
        PNPB_LIB_FATAL
    }
    /* Check against buffer underflow */
    for (i = 0; i < 2; i++)
    {
        tmp_read_ptr = PNPB_XHIF_ACYC_OUT_SERV.read_ptr;

        if (tmp_read_ptr != tmp_write_ptr)
        {
            status = PNPB_OK;
            break;
        }

        pnpb_xhif_wait();
    }

    *p_read_ptr = tmp_read_ptr;

    return status;
}   /* pnpb_xhif_check_acyc_read_pointer */


/**
 *  @brief Increment acyclic buffer ptr
 *
 *  @return         void
 *
 *  Do after every read
 *
 */
PNIO_VOID pnpb_xhif_increment_acyc_read_pointer(PNIO_UINT32 *p_read_ptr)
{
    PNIO_UINT32 tmp_read_ptr = *p_read_ptr;
    /* Increment */
    if((PNPB_XHIF_SIZE_OF_BUFFER_FOR_ACYC - 1) <= tmp_read_ptr)
    {
        tmp_read_ptr = 0;    /* buffer went over max to 0*/
    }
    else
    {
        tmp_read_ptr++;
    }
    /* Save new value */
    *p_read_ptr = tmp_read_ptr;

}   /* pnpb_xhif_increment_acyc_read_pointer */


/**
 *  @brief Check against acyclic buffer overflow
 *
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Do before every write
 *  Also increments pointer
 *
 */
PNIO_UINT32 pnpb_xhif_check_acyc_write_pointer(PNIO_UINT32 *p_write_ptr)
{
    PNIO_UINT32 tmp_write_ptr, tmp_read_ptr;
    /* return write + 1 but do not increment yet */

    /* read */
    tmp_write_ptr = PNPB_XHIF_ACYC_IN_SERV.write_ptr;
    tmp_read_ptr = PNPB_XHIF_ACYC_IN_SERV.read_ptr;

    /* Check */
    if(PNPB_XHIF_SIZE_OF_BUFFER_FOR_ACYC <= tmp_write_ptr)
    {
        printf("Error: invalid acyclic write index %d\n", tmp_write_ptr);
        PNPB_LIB_FATAL
    }

    /* Increment */
    if((PNPB_XHIF_SIZE_OF_BUFFER_FOR_ACYC - 1) <= tmp_write_ptr)
    {
        tmp_write_ptr = 0;    /* buffer went over max to 0*/
    }
    else
    {
        tmp_write_ptr++;
    }

    if(tmp_write_ptr == tmp_read_ptr)
    {
        printf("Error: acyc write failed - buffer full\n");
        PNPB_LIB_FATAL
    }

    /* Return */
    *p_write_ptr = tmp_write_ptr;
    return PNPB_OK;
}   /* pnpb_xhif_check_acyc_write_pointer */


/**
 *  @brief Increment number of records in acyclic buffer
 *
 *  @return         void
 *
 *  Do after every read
 *  Also signals to slave
 *
 */
PNIO_VOID pnpb_xhif_increment_acyc_write_pointer(PNIO_UINT32 write_ptr)
{
    /* pointer to GPMC accessed memory */
    PNPB_XHIF_ACYC_IN_SERV.write_ptr = write_ptr;

}   /* pnpb_xhif_increment_acyc_write_pointer */


/**
 *  @brief Returns id of "more follows" to id in parameter
 *
 *  @param[in]      id              ID of called function
 *  @return                         ID of more follows of called function
 *
 *  For telegrams longer than 1024B
 *
 */
PNPB_XHIF_ACYC_TELEGRAMS pnpb_xhif_acyc_has_more_follows(PNPB_XHIF_ACYC_TELEGRAMS id)
{
    switch (id)
    {
        case PNPB_XHIF_ACYC_NO_TELEGRAM:
            return PNPB_XHIF_ACYC_NO_TELEGRAM;

        case PNPB_XHIF_ACYC_REC_READ_RSP:
            return PNPB_XHIF_ACYC_REC_READ_RSP_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_REC_WRITE_RSP:
            return PNPB_XHIF_ACYC_REC_WRITE_RSP_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_AMR_READ_RSP:
            return PNPB_XHIF_ACYC_AMR_READ_RSP_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_PE_RESPONSE_RSP:
            return PNPB_XHIF_ACYC_PE_RESPONSE_RSP_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_NV_DATA_INIT:
            return PNPB_XHIF_ACYC_NV_DATA_INIT_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_NV_DATA_STORE:
            return PNPB_XHIF_ACYC_NV_DATA_STORE_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_NV_DATA_IM_STORE:
            return PNPB_XHIF_ACYC_NV_DATA_IM_STORE_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_IM_READ_RSP:
            return PNPB_XHIF_ACYC_IM_READ_RSP_MORE_FOLLOWS;

        case PNPB_XHIF_ACYC_DEVICE_SETUP:
            return PNPB_XHIF_ACYC_DEVICE_SETUP_MORE_FOLLOWS;

        default:
            return PNPB_XHIF_ACYC_NO_TELEGRAM;
    }
}   /* pnpb_xhif_acyc_has_more_follows */


/**
 *  @brief Identifies "more follows" based on id
 *
 *  @param[in]      id              ID of called function
 *  @return                         Is more follows [PNPB_TRUE, PNPB_FALSE]
 *
 *  For telegrams longer than 1024B
 *
 */
PNIO_UINT32 pnpb_xhif_acyc_is_more_follows(PNPB_XHIF_ACYC_TELEGRAMS id)
{
    switch (id)
    {
        case PNPB_XHIF_ACYC_REC_WRITE_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_REC_READ_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_AR_OWNERSHIP_IND_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_PE_REQUEST_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_NV_DATA_SYNC_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_STORE_REMA_MEM_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_IM_WRITE_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_IM_STORE_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_NV_DATA_STORE_HOST_MORE_FOLLOWS:
        case PNPB_XHIF_ACYC_NV_DATA_SET_DEFAULT_MORE_FOLLOWS:
            return PNPB_TRUE;
        default:
            return PNPB_FALSE;
    }
}   /* pnpb_xhif_acyc_is_more_follows */


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
PNIO_VOID pnpb_xhif_acyc_write_internal(PNPB_XHIF_ACYC_TELEGRAMS id, PNIO_UINT16 prm_len_tot, PNIO_UINT16 prm_len, PNIO_UINT8 *p_prm)
{
    PNIO_UINT32 write_ptr;
	
    /* Check send ready */
    if(PNPB_NOT_OK == pnpb_xhif_check_acyc_write_pointer(&write_ptr))
    {
       /* error */
        printf("Error: Not ready for acyc write\n");
        PNPB_LIB_FATAL
    }
    pnpb_xhif_acyc_in_trylock(write_ptr);

    PNPB_XHIF_ACYC_IN_DATA[write_ptr].id = id;
    PNPB_XHIF_ACYC_IN_DATA[write_ptr].data_len = prm_len_tot;
    if((0 < prm_len) && (PNPB_XHIF_ACYCLIC_DATA_MAX >= prm_len) && (NULL != p_prm))
    {
        //lint -e{670} Possible access beyond array for function 'Symbol', argument Integer exceeds argument Integer Reference
        memcpy((void*)PNPB_XHIF_ACYC_IN_DATA[write_ptr].data, (const void*)p_prm, prm_len);
    }
    else if((0 == prm_len))
    {
        /* Command with 0 parameters - Nothing to copy */
    }
    else
    {
        printf("Error: Invalid data len for acyc write %d\n", prm_len);
        PNPB_LIB_FATAL
    }

    pnpb_xhif_acyc_in_unlock(write_ptr);
    pnpb_xhif_increment_acyc_write_pointer(write_ptr);

    /* Signal by GPIO signal */
    bbb_gpio_set(GPIO_BANK_0, GPIO_NUM_14, 1);
    bbb_gpio_set(GPIO_BANK_0, GPIO_NUM_14, 0);
}

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
PNIO_VOID pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_TELEGRAMS id, PNIO_UINT16 prm_len, PNIO_UINT8 *p_prm)
{
    PNIO_UINT16 send_prm_len, send_id, remaining;
    PNIO_UINT8 *p_prm_send;
    PNIO_UINT8 *p_prm_send_next;
    PNIO_UINT32 write_ptr;

    pthread_mutex_lock(&gpmc_access_mutex);


    remaining = prm_len;
    p_prm_send_next = p_prm;

    do
    {
        p_prm_send = p_prm_send_next;
        /* Can be transfered in one shot or last iteration of non_one_shot */
        if((PNPB_XHIF_ACYCLIC_DATA_MAX) > remaining)
        {
            send_prm_len = remaining;
            remaining = 0;
            send_id = (PNIO_UINT16)id;
        }
        else
        {
            send_id = pnpb_xhif_acyc_has_more_follows(id);
            remaining -= PNPB_XHIF_ACYCLIC_DATA_MAX;
            send_prm_len = PNPB_XHIF_ACYCLIC_DATA_MAX;
            //lint -e{662} possible creation of out-of-bounds pointer ('Integer' beyond end of data) by operator 'String'
            //size of 'p_prm_send_next' is 'prm_len' and 'remaining' is initalized as 'prm_len'.
            //So when 'remaining' is bigger than 'PNPB_XHIF_ACYCLIC_DATA_MAX', '+=' will never cause a possible out-of-bound error
            p_prm_send_next += PNPB_XHIF_ACYCLIC_DATA_MAX;
        }
        /*send*/
        pnpb_xhif_acyc_write_internal(send_id, prm_len, send_prm_len, p_prm_send);

    }while(0 != remaining);

    pthread_mutex_unlock(&gpmc_access_mutex);
}   /* pnpb_xhif_acyc_write */


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
PNIO_VOID pnpb_xhif_acyc_read_continue(PNIO_UINT32 id, PNIO_UINT32 is_more_follows, PNIO_UINT32 read_ptr)
{
    PNIO_UINT32 incoming_size;
	
    /* First call of continual read */
    if(PNPB_FALSE == pnpb_xhif_acyclic_recieve_cont_manage.more_follows)
    {
        /* Read size of incoming telegram */
        incoming_size = PNPB_XHIF_ACYC_OUT_DATA[read_ptr].data_len;


        if(0 > (incoming_size - PNPB_XHIF_ACYCLIC_DATA_MAX))
        {
            printf("Error: Trying to read too many bytes - this should not be more follows \n");
            PNPB_LIB_FATAL
        }

        /* Allocate memory for newly received telegram */
        pnpb_xhif_acyclic_recieve_cont_manage.p_start_data = calloc(incoming_size, 1);

        memcpy((void*)pnpb_xhif_acyclic_recieve_cont_manage.p_start_data,
               (const void*) PNPB_XHIF_ACYC_OUT_DATA[read_ptr].data,
                PNPB_XHIF_ACYCLIC_DATA_MAX);

        pnpb_xhif_acyclic_recieve_cont_manage.more_follows = PNPB_TRUE;
        pnpb_xhif_acyclic_recieve_cont_manage.p_write_data =
                pnpb_xhif_acyclic_recieve_cont_manage.p_start_data + PNPB_XHIF_ACYCLIC_DATA_MAX;
        pnpb_xhif_acyclic_recieve_cont_manage.data_remains = incoming_size - PNPB_XHIF_ACYCLIC_DATA_MAX;
    }

    else
    {

        /* Last call of continuous read */
        if(PNPB_FALSE == is_more_follows)
        {
            if(0 >= pnpb_xhif_acyclic_recieve_cont_manage.data_remains)
            {
                printf("Error: Trying to read zero bytes \n");
                PNPB_LIB_FATAL
            }

            if(PNPB_XHIF_ACYCLIC_DATA_MAX < pnpb_xhif_acyclic_recieve_cont_manage.data_remains)
            {
                printf("Error: Continuous read finished, but there are data remaining \n");
                PNPB_LIB_FATAL
            }

            memcpy((void*)pnpb_xhif_acyclic_recieve_cont_manage.p_write_data,
                   (const void*) PNPB_XHIF_ACYC_OUT_DATA[read_ptr].data,
                    pnpb_xhif_acyclic_recieve_cont_manage.data_remains);

            pnpb_xhif_call_function(id, pnpb_xhif_acyclic_recieve_cont_manage.p_start_data);
            pnpb_xhif_acyclic_recieve_cont_manage.more_follows = PNPB_FALSE;
            pnpb_xhif_acyclic_recieve_cont_manage.data_remains = 0;

        }
        else
        {
            if(0 >= (pnpb_xhif_acyclic_recieve_cont_manage.data_remains - PNPB_XHIF_ACYCLIC_DATA_MAX))
            {
                printf("Error: Trying to read too much data \n");
                PNPB_LIB_FATAL
            }
			
            memcpy((void*)pnpb_xhif_acyclic_recieve_cont_manage.p_write_data,
                   (const void*) PNPB_XHIF_ACYC_OUT_DATA[read_ptr].data,
                    PNPB_XHIF_ACYCLIC_DATA_MAX);

            pnpb_xhif_acyclic_recieve_cont_manage.p_write_data += PNPB_XHIF_ACYCLIC_DATA_MAX;
            pnpb_xhif_acyclic_recieve_cont_manage.data_remains -= PNPB_XHIF_ACYCLIC_DATA_MAX;
        }
    }
}   /* pnpb_xhif_acyc_read_continue */


/**
 *  @brief Read telegram of maximally 1024B
 *
 *  @param[in]      id              ID of called function
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Also allocates buffer for function parameters and executes the function
 *
 */
PNIO_VOID pnpb_xhif_acyc_read_telegram(PNIO_UINT32 id, PNIO_UINT32 read_ptr)
{
    PNIO_UINT32 incoming_size;
    PNIO_UINT8 *parameters;

    /* Read size of incoming telegram */
    incoming_size = PNPB_XHIF_ACYC_OUT_DATA[read_ptr].data_len;

    /* Check size of expected data */
    if(PNPB_XHIF_NUMOF_BYTES_PER_ACYCLIC_TELEGRAM <= incoming_size)
    {
        printf("Invalid telegram size %d\n", incoming_size);
        PNPB_LIB_FATAL
    }
    if(0 != incoming_size)
    {
        /* Allocate memory for newly received telegram */
        parameters = calloc(incoming_size, 1);
        if(NULL != parameters)
        {
            memcpy((void*)parameters, (const void*)PNPB_XHIF_ACYC_OUT_DATA[read_ptr].data, incoming_size);
        }
        else
        {
            printf ("Error: Memory could not be allocated!\n");
			PNPB_LIB_FATAL;
        }
    }
    else
    {
        parameters = NULL;
    }

    pnpb_xhif_call_function(id, parameters);
}   /* pnpb_xhif_acyc_read_telegram */


/**
 *  @brief Receives acyclic telegram
 *
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Decide if function parameters received are in one telegram or divided into more,
 *  process accordingly.
 *
 */
PNIO_VOID pnpb_xhif_wait( PNIO_VOID )
{
    volatile int i = 0;

    for ( i = 0 ; i < 500 ; i++);
}   /* pnpb_xhif_wait */


/**
 *  @brief Receives acyclic telegram
 *
 *  @return                         Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Decide if function parameters received are in one telegram or divided into more,
 *  process accordingly.
 *
 */
PNIO_UINT32 pnpb_xhif_acyc_read()
{
    PNIO_UINT32 is_more_follows;
    PNPB_XHIF_ACYC_TELEGRAMS incoming_id;
    PNIO_UINT32 read_ptr;

    pthread_mutex_lock(&gpmc_access_mutex);
    if(PNPB_NOT_OK == pnpb_xhif_check_acyc_read_pointer(&read_ptr))
    {
        printf("Acyclic buffer not ready to be read\n");
        PNPB_LIB_FATAL
        return PNPB_NOT_OK;
    }
    pnpb_xhif_increment_acyc_read_pointer(&read_ptr);
    pnpb_xhif_acyc_out_trylock(read_ptr);

    incoming_id = PNPB_XHIF_ACYC_OUT_DATA[read_ptr].id;

    /* Ertec in startup -> accept only Device startup telegrams */
    if(PNPB_USR_START_IDLE == PnpbDeviceStartupState)
    {
        if(PNPB_XHIF_ACYC_DEVICE_STARTUP_DONE != incoming_id)
        {
            printf("Error: Acyclic comuncation, but startup not finished\n");
            PNPB_LIB_FATAL
            return PNPB_NOT_OK;
        }
    }

    /* Decide if more follows */
    is_more_follows = pnpb_xhif_acyc_is_more_follows(incoming_id);

    if((PNPB_TRUE == is_more_follows) || (PNPB_TRUE == pnpb_xhif_acyclic_recieve_cont_manage.more_follows))
    {
        /* Divided into several telegrams, receive all before function call! */
        pnpb_xhif_acyc_read_continue(incoming_id, is_more_follows, read_ptr);
    }
    else
    {
        /* All fitted in one telegram */
        pnpb_xhif_acyc_read_telegram(incoming_id, read_ptr);
    }
    pnpb_xhif_acyc_out_unlock(read_ptr);

    PNPB_XHIF_ACYC_OUT_SERV.read_ptr = read_ptr;

    pthread_mutex_unlock(&gpmc_access_mutex);

    return PNPB_OK;
}   /* pnpb_xhif_acyc_read */



/************************ FUNCTIONS CALLED BY ACYCLIC TELEGRAM RECIEVAL **************************/

/* Only error handling - actual functions in pnpb_lib_acyc.c file */

/**
 *  @brief No function set to process this telegram
 *
 *  @param[in]      *prm            Pointer to parameters
 *  @return         void
 *
 *  Error handling
 *
 */
PNIO_VOID pnpb_xhif_no_telegram(PNIO_UINT8* params)
{
    PNPB_UNUSED_ARG(params);
    printf("Invalid telegram number\n");
    PNPB_LIB_FATAL
}   /* pnpb_xhif_no_telegram */


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
