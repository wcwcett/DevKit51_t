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
/*  F i l e               &F: iom_perif.c                               :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  software module handling the ERTEC200+ PER-IF                            */
/*                                                                           */
/*****************************************************************************/

#define IOM_LOCAL_ERR_MODULE IOM_MODULE_PERIF

#include "iom_inc.h"
#include "iom_perif.h"

#ifdef IOM_CFG_PNIP

/*lint -esym(578,index) Declaration of symbol 'index' hides symbol 'index(const char *, int)' for ECOS */

/* ------------------------------------- internal global variables ------------------------------------ */

/**
 * this structure contains all the internal data of the IOM perif module
 */
typedef struct iom_data_s
{
    iom_perif_cr_data_t perif_page_cr_data[IOM_CFG_PERIF_NUM_CR];    /**< descriptors for all CRs supported by PERIF hardware */

    iom_perif_ar_data_t perif_page_ar_data[IOM_CFG_PERIF_NUM_AR];    /**< descriptors for all ARs */

    iom_perif_page_mem_desc_t perif_page_mem_desc[IOM_PERIF_MAX_PAGE_INDEX]; /**< descriptors for all memory pages available in PERIF hardware */

    PNIO_UINT16         session_key_prim_ar;                         /**< bypassed AR-session_key, needed for setting data state  */
    LSA_BOOL            perif_opened;
    IOM_EDDP_SYS_TYPE   eddp_sys_handle;
    LSA_BOOL            use_consistency_control_hw;                  /**< TRUE: use the Consistency-Control hardware of Pallas, FALSE: use Perif (Ertec200p 1/2 / Pallas) */
    LSA_UINT16          page_size;                                   /**< for page mode: size of one page. Ertec200p 1/2: 256 bytes; Consistency Control: 512 bytes */
}iom_data_t;

#ifdef IOM_CFG_LOCATE_DATA_TO_TCM
    #if defined (TOOL_CHAIN_GREENHILLS_ARM)
        #pragma ghs section bss=IOM_TCM_SECTION_NAME
        #define IOM_ATTRIBUTE_TCM
    #elif defined (TOOL_CHAIN_GNU_PPC)
        #define IOM_ATTRIBUTE_TCM __attribute__ ((section (IOM_TCM_SECTION_NAME)))
    #else
        #define IOM_ATTRIBUTE_TCM __attribute__ ((section (IOM_TCM_SECTION_NAME)))
    #endif
#else
    #define IOM_ATTRIBUTE_TCM
#endif

static iom_data_t iom_data    IOM_ATTRIBUTE_TCM ;

#ifdef IOM_CFG_LOCATE_DATA_TO_TCM
    #if defined (TOOL_CHAIN_GREENHILLS_ARM)
        #pragma ghs section bss=IOM_DEFAULT_BSS_SECTION_NAME
    #elif defined (TOOL_CHAIN_GNU_PPC)
        /* nothing needs to be done here */
    #else
        /* nothing needs to be done here */
    #endif
#endif

#define IOM_INVALID_PROV_ID 0xFFFF

/* ----------------------------------------- helper functions ----------------------------------------- */

/**
 * This function seeks in >page< for a contiguous memory block of size >*size< , starting at the page address offset >search_start_index<.
 * If a suitable region is found, the function returns TRUE, the offset of the region found (>offset_region_start<), and the size of the
 * contiguous free memory [>= size] (>*size<).
 */
static LSA_BOOL iom_perif_find_free_mem_block(LSA_UINT8 page, LSA_UINT16* size, LSA_UINT16 search_start_index, LSA_UINT32 *offset_region_start)
{
    const LSA_UINT32 max_offset_in_page = (iom_data.page_size >> 3);    // index of the last 8-byte block within page
    LSA_UINT32       offset;         // number of 8byte-increments within page
    LSA_UINT32       offset_end;
    LSA_UINT32       size64 = (*size + 7) >> 3;

    search_start_index &= ~0x07;
    *size = 0;

    if(size64)
    {
        for(offset = (search_start_index >> 3); offset <= (max_offset_in_page - size64); offset++)
        {
            if ((iom_data.perif_page_mem_desc[page].memory_map[offset >> 5] & (1 << (offset % IOM_PERIF_MEMORY_MAX_OFFSET_IN_PAGE))) == 0)
            {
                const LSA_UINT32 offset_end_max = offset + size64;

                for (offset_end = offset + 1; offset_end < offset_end_max; offset_end++)
                {
                    if ((iom_data.perif_page_mem_desc[page].memory_map[offset_end >> 5] & (1 << (offset_end % IOM_PERIF_MEMORY_MAX_OFFSET_IN_PAGE))) != 0)
                    {
                        break;
                    }
                }

                if ((offset_end - offset) >= size64)
                {
                    /* the requested memory can be provided at address offset >offset<. Now examine how large
                     * the free region really is. This enables the calling function to determine a page that
                     * suits optimally regarding the memory request. */
                    while(   (offset_end < max_offset_in_page) &&
                           ((iom_data.perif_page_mem_desc[page].memory_map[offset_end >> 5] & (1 << (offset_end % IOM_PERIF_MEMORY_MAX_OFFSET_IN_PAGE))) == 0))
                    {
                        offset_end++;
                    }
                    *offset_region_start = offset << 3;
                    *size                = (offset_end - offset) << 3;
                    return LSA_TRUE;
                }
                else
                {
                    offset = offset_end + 1;
                }
            }
        }
    }
    return LSA_FALSE;
}

/**
 * This function reserves a piece of memory in a specific page for a specific CR. It updates the memory map
 * to mark the newly reserved memory and adds the page-usage-information to the CR descriptor.
 *
 * @return  LSA_TRUE if everything went well, otherwise LSA_FALSE
 */
static LSA_BOOL iom_perif_add_used_mem_block(iom_perif_cr_data_t *cr, LSA_UINT8 page_index, LSA_UINT32 buffer_size, LSA_UINT32 offset)
{
    LSA_UINT32 index;

    if(buffer_size == 0)
    {
        return LSA_TRUE;
    }

    if((cr->num_pages >= IOM_PERIF_MAX_NUM_PAGES) || (page_index >= IOM_PERIF_MAX_PAGE_INDEX))
    {
        return LSA_FALSE;
    }

    /* add the page / its address range to the CR descriptor */
    cr->used_pages[cr->num_pages].page_number      = page_index;
    cr->used_pages[cr->num_pages].page_offset_low  = offset & IOM_PERIF_8_BYTE_ALINGMENT;                     /* 8 byte aligned */
    cr->used_pages[cr->num_pages].page_offset_high = (offset + buffer_size + 7) & IOM_PERIF_8_BYTE_ALINGMENT; /* 8 byte aligned, specifies the index of the
                                                                                                                 first free word AFTER allocated section */

    /* mark the used memory in the global memory map, 4 byte aligned (1 bit corresponds to 4 bytes) */
    index = cr->used_pages[cr->num_pages].page_offset_low >> 3;
    while(index < (cr->used_pages[cr->num_pages].page_offset_high >> 3))
    {
        if(page_index < IOM_PERIF_MAX_PAGE_INDEX)
        {
            iom_data.perif_page_mem_desc[page_index].memory_map[index >> 5] |= (1 << (index % IOM_PERIF_MEMORY_MAX_OFFSET_IN_PAGE));
            index++;
        }
    }
    if(( cr->num_pages < IOM_PERIF_MAX_NUM_PAGES ) && (page_index < IOM_PERIF_MAX_PAGE_INDEX))
    {
        /* update variables describing free regions */
        if (cr->used_pages[cr->num_pages].page_offset_low < iom_data.perif_page_mem_desc[page_index].bytes_free_from_start)
        {
            iom_data.perif_page_mem_desc[page_index].bytes_free_from_start = cr->used_pages[cr->num_pages].page_offset_low;
        }
    if(iom_data.page_size - cr->used_pages[cr->num_pages].page_offset_high < iom_data.perif_page_mem_desc[page_index].bytes_free_from_end)
        {
        iom_data.perif_page_mem_desc[page_index].bytes_free_from_end = iom_data.page_size - cr->used_pages[cr->num_pages].page_offset_high;
        }
        iom_data.perif_page_mem_desc[page_index].bytes_free -= (cr->used_pages[cr->num_pages].page_offset_high - cr->used_pages[cr->num_pages].page_offset_low);
    }
    cr->num_pages++;

    return LSA_TRUE;
}

/**
 * This function releases all memory allocated by a CR. The memory maps of the affected pages are updated as
 * well as the page-usage-information of the CR descriptor.
 * No hardware accesses are performed here.
 */
static LSA_VOID iom_perif_delete_cr_used_mem_blocks(iom_perif_cr_data_t *cr)
{
    LSA_UINT8 i;
    LSA_UINT8 page_index;
    LSA_INT32 index;
    const LSA_UINT32    max_index_in_page = (iom_data.page_size >> 3);

    for(i = 0; i < cr->num_pages; i++)
    {
        page_index = cr->used_pages[i].page_number;
        index      = cr->used_pages[i].page_offset_low >> 3;

        /* mark the used memory in the global memory map as free */

        while(index < (cr->used_pages[i].page_offset_high >> 3))
        {
            iom_data.perif_page_mem_desc[page_index].memory_map[index >> 5] &= ~(1UL << (index % IOM_PERIF_MEMORY_MAX_OFFSET_IN_PAGE));
            index++;
        }


        /* update >bytes_free_from_start< of this page */
        if(cr->used_pages[i].page_offset_low == iom_data.perif_page_mem_desc[page_index].bytes_free_from_start)
        {
            /* count how many bytes are unused behind the memory area that is to be deleted */
            index = cr->used_pages[i].page_offset_high >> 3;
             while(   (index < max_index_in_page) &&
                   ((iom_data.perif_page_mem_desc[page_index].memory_map[index >> 5] & (1UL << (index % IOM_PERIF_MEMORY_MAX_OFFSET_IN_PAGE))) == 0))
            {
                index++;
            }
            iom_data.perif_page_mem_desc[page_index].bytes_free_from_start = index << 3;
        }

        /* update >bytes_free_from_end< of this page */
        if(iom_data.page_size - cr->used_pages[i].page_offset_high == iom_data.perif_page_mem_desc[page_index].bytes_free_from_end)
        {
            /* count how many bytes are unused in front of the memory area that is to be deleted */
            index = (cr->used_pages[i].page_offset_low >> 3) - 1;
            while ((index >= 0) &&
                   ((iom_data.perif_page_mem_desc[page_index].memory_map[index >> 5] & (1 << (index % IOM_PERIF_MEMORY_MAX_OFFSET_IN_PAGE))) == 0))
            {
                index--;
            }
            iom_data.perif_page_mem_desc[page_index].bytes_free_from_end = iom_data.page_size - ((index + 1) << 3);
        }

        iom_data.perif_page_mem_desc[page_index].bytes_free += (cr->used_pages[i].page_offset_high - cr->used_pages[i].page_offset_low);

        cr->used_pages[i].page_number = 0;
        cr->used_pages[i].page_offset_low = 0;
        cr->used_pages[i].page_offset_high = 0;
    }
    cr->num_pages = 0;
}

/**
 * This function calls iom_perif_delete_cr_used_mem_blocks() to release all memory of the CR >cr<.
 * Afterwards all other meta data of the CR is reset and the CR gets disconnected from its AR.
 * No hardware accesses are performed here.
 */
static LSA_VOID iom_perif_delete_cr_descriptor_data(LSA_UINT8 ar_idx, iom_perif_cr_data_t *cr)
{
    /* delete page descriptor data */
    iom_perif_delete_cr_used_mem_blocks(cr);

    /* delete all buffer data of the CR */
    cr->buffer_size            = 0;
    cr->buffer_size_no_padding = 0;
    cr->cr_index               = 0;
    cr->start_address          = 0;
    cr->apdu_status_address    = LSA_NULL;
    cr->dma_job                = -1;
#ifdef IOM_CFG_DMA_BUF_EXCHANGE
    cr->dma_job_buf_exchange   = -1;
#endif

    cr->locked                 = 0;
    cr->cons_last_user_buffer_idx = 0;

    /* disconnect CR from AR */
    if(iom_data.perif_page_ar_data[ar_idx].input_cr == cr)
    {
        iom_data.perif_page_ar_data[ar_idx].input_cr = LSA_NULL;
    }
    else if(iom_data.perif_page_ar_data[ar_idx].output_cr == cr)
    {
        iom_data.perif_page_ar_data[ar_idx].output_cr = LSA_NULL;
    }
    else
    {
        iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
    }

    /* AR is completely released -> reset all meta data applicable to the entire AR */
    if(iom_data.perif_page_ar_data[ar_idx].output_cr == LSA_NULL && iom_data.perif_page_ar_data[ar_idx].input_cr == LSA_NULL)
    {
        iom_data.perif_page_ar_data[ar_idx].new_data_cbf = LSA_NULL;
        iom_data.perif_page_ar_data[ar_idx].free_request_issued = LSA_FALSE;
    }
}

/**
 * This function is called from the "new data"-ISR-handler or from iom_perif_free_iocr_memory() directly
 * to deallocate all resources of an AR. Via iocr_type you can parameterize whether all available CRs (IOM_PERIF_ALL_CRS)
 * or a specific CR memory should be freed (IOM_PERIF_INPUT_CR, IOM_PERIF_OUTPUT_CR).
 */
static LSA_VOID iom_perif_free_iocr_memory_local(LSA_UINT8 ar_idx, iom_perif_iocr_t iocr_type)
{
    if(iocr_type == IOM_PERIF_INPUT_CR || iocr_type == IOM_PERIF_ALL_CRS)
    {
        if(iom_data.perif_page_ar_data[ar_idx].input_cr)
        {
            if(iom_data.perif_page_ar_data[ar_idx].input_cr->locked == 0)       /* if the CR is locked we cannot deallocate the memory */
            {
                #ifdef IOM_CFG_USE_GDMA
                /* remove the DMA job associated with this CR */
                if(GDMA_IS_VALID_JOB(iom_data.perif_page_ar_data[ar_idx].input_cr->dma_job))
                {
                    gdma_remove_job(iom_data.perif_page_ar_data[ar_idx].input_cr->dma_job);
                }

#ifdef IOM_CFG_DMA_BUF_EXCHANGE
                if(GDMA_IS_VALID_JOB(iom_data.perif_page_ar_data[ar_idx].input_cr->dma_job_buf_exchange))
                {
                    gdma_remove_job(iom_data.perif_page_ar_data[ar_idx].input_cr->dma_job_buf_exchange);
                }
#endif
                #endif

                /* reset CR_Address_x register */
                IOM_PERIF_CR_ADDRESS_N__SET_FIELD(iom_data.perif_page_ar_data[ar_idx].input_cr->cr_index, zero_data, 0);
                IOM_PERIF_CR_ADDRESS_N__SET_FIELD(iom_data.perif_page_ar_data[ar_idx].input_cr->cr_index, new_data_int, 0);         /* no data interrupts shall be triggered */

                /* reset CR_State_x register */
                IOM_PERIF_CR_STATE_N(iom_data.perif_page_ar_data[ar_idx].input_cr->cr_index)->perif_v1.mapping = 0;                /* no pages belong to this CR anymore */

                /* reset CR address guard register */
                IOM_PERIF_GUARD_CONTROL_N(iom_data.perif_page_ar_data[ar_idx].input_cr->cr_index)->guard_valid = 0;

                /* reset all cr descriptors / meta data */
                iom_perif_delete_cr_descriptor_data(ar_idx, iom_data.perif_page_ar_data[ar_idx].input_cr);
            }
            else
            {
                iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
            }

#ifdef IOM_SYSTEM_REDUNDANCY_SUPPORT
            if (iom_data.perif_page_ar_data[ar_idx].is_sysred_provider != PNIO_FALSE)
            {
                iom_data.perif_page_ar_data[ar_idx].is_sysred_provider = PNIO_FALSE;

                if (IOM_EDDP_PROVIDER_REMOVE(EDD_INTERFACE_ID_MIN-1, iom_data.perif_page_ar_data[ar_idx].prov_id) != EDD_STS_OK)
                {
                    iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
                }
                iom_data.perif_page_ar_data[ar_idx].prov_id = IOM_INVALID_PROV_ID;
            }
#endif
        }
    }

    if(iocr_type == IOM_PERIF_OUTPUT_CR || iocr_type == IOM_PERIF_ALL_CRS)
    {
        if(iom_data.perif_page_ar_data[ar_idx].output_cr)
        {
            if(iom_data.perif_page_ar_data[ar_idx].output_cr->locked == 0)
            {
                #ifdef IOM_CFG_USE_GDMA
                /* remove the DMA job associated with this CR */
                if(GDMA_IS_VALID_JOB(iom_data.perif_page_ar_data[ar_idx].output_cr->dma_job))
                {
                    gdma_remove_job(iom_data.perif_page_ar_data[ar_idx].output_cr->dma_job);
                }

#ifdef IOM_CFG_DMA_BUF_EXCHANGE
                if(GDMA_IS_VALID_JOB(iom_data.perif_page_ar_data[ar_idx].output_cr->dma_job_buf_exchange))
                {
                    gdma_remove_job(iom_data.perif_page_ar_data[ar_idx].output_cr->dma_job_buf_exchange);
                }
#endif
                #endif

                /* reset new data interrupt enable */
                IOM_PERIF_DEACTIVATE_NEW_DATA_INT(ar_idx);

                /* reset CR_Address_x register */
                IOM_PERIF_CR_ADDRESS_N__SET_FIELD(iom_data.perif_page_ar_data[ar_idx].output_cr->cr_index, zero_data, 0);
                IOM_PERIF_CR_ADDRESS_N__SET_FIELD(iom_data.perif_page_ar_data[ar_idx].output_cr->cr_index, new_data_int, 0);       /* no data interrupts shall be triggered */

                /* reset CR_State_x register */
                IOM_PERIF_CR_STATE_N(iom_data.perif_page_ar_data[ar_idx].output_cr->cr_index)->perif_v1.mapping = 0;               /* no pages belong to this CR anymore */

                /* reset CR address guard register */
                IOM_PERIF_GUARD_CONTROL_N(iom_data.perif_page_ar_data[ar_idx].output_cr->cr_index)->guard_valid = 0;

                /* reset all cr descriptors / meta data */
                iom_perif_delete_cr_descriptor_data(ar_idx, iom_data.perif_page_ar_data[ar_idx].output_cr);
            }
            else
            {
                iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
            }
        }
    }
}

#ifdef IOM_CFG_LOCATE_INSTRUCTIONS_TO_TCM
    #if defined (TOOL_CHAIN_GREENHILLS_ARM)
        #pragma ghs section text=IOM_ITCM_SECTION_NAME
        #define IOM_ATTRIBUTE_ITCM
    #elif defined (TOOL_CHAIN_GNU_PPC)
        #define IOM_ATTRIBUTE_ITCM __attribute__ ((section (IOM_ITCM_SECTION_NAME)))
    #else
        #define IOM_ATTRIBUTE_ITCM __attribute__ ((section (IOM_ITCM_SECTION_NAME)))
    #endif
#else
    #define IOM_ATTRIBUTE_ITCM
#endif

/**
 * @brief   Interrupt service routine that is executed after data in an output CR has been received.
 *          A corresponding user callback is called from here.
 */
IOM_ATTRIBUTE_ITCM static LSA_VOID iom_perif_isr_output_data_received()
{
    LSA_UINT32  new_data_state = (*IOM_PERIF_PN_IRQ_LOW >> 16) & ((1 << IOM_CFG_PERIF_NUM_AR) - 1);    // mask the "new data interrupts" states
    LSA_UINT8   cur_ar;
    while(new_data_state)
    {
        cur_ar = 0;
        while(((new_data_state & (1 << cur_ar)) == 0) && (cur_ar < IOM_CFG_PERIF_NUM_AR))        /* look for the next pending interrupt (= output CR / AR) */
        {
            cur_ar++;
        }

        if(cur_ar < IOM_CFG_PERIF_NUM_AR)
        {
            if(iom_data.perif_page_ar_data[cur_ar].new_data_cbf != LSA_NULL)
            {
                iom_data.perif_page_ar_data[cur_ar].new_data_cbf(cur_ar);               /* call the callback and inform about new data */
            }

            IOM_PERIF_ACKNOWLEDGE_NEW_DATA_INT(cur_ar);
        }
        new_data_state = (*IOM_PERIF_PN_IRQ_LOW >> 16) & ((1 << IOM_CFG_PERIF_NUM_AR) - 1);    // mask the "new data interrupts" states
    }
    IOM_PERIF_PN_EOI->wait_time = 1;
}
#ifdef IOM_CFG_LOCATE_INSTRUCTIONS_TO_TCM
    #if defined (TOOL_CHAIN_GREENHILLS_ARM)
        #pragma ghs section text=IOM_DEFAULT_TEXT_SECTION_NAME
    #elif defined (TOOL_CHAIN_GNU_PPC)
        /* nothing needs to be done here */
    #else
        /* nothing needs to be done here */
    #endif
#endif

/* ----------------------------------------- user functions ----------------------------------------- */

/**
 * @brief   This function initializes all meta data fields describing ARs and CRs. No external services
 *          are called from here (ISR registration for example).
 */
LSA_VOID iom_perif_init()
{
    LSA_UINT32 i, j;

#ifdef BOARD_TYPE_STEP_3
	if(!(IOM_PERIF_IP_VERSION->configuration == 0x01))
        {
            /* expect Pallas (32 kB IO-RAM) configuration */
            iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
        }

        iom_data.use_consistency_control_hw = PNIO_TRUE;
        iom_data.page_size                  = 512;
#else
    if(IOM_PERIF_IP_VERSION->configuration == 0x00 || IOM_PERIF_IP_VERSION->configuration == 0x02)
    {
        /* 6 CR or 9 CR configuration not supported by this driver. 27-CR hardware required. */
        iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
    }
    iom_data.use_consistency_control_hw = PNIO_FALSE;
    iom_data.page_size                  = 256;
#endif

    /* enable paged mode to avoid fragmentation */
    IOM_PERIF_ADDRESS_MODE->address_mode = 1;

    /* initialize AR data */
    for(i = 0; i < IOM_CFG_PERIF_NUM_AR; i++)
    {
        iom_data.perif_page_ar_data[i].output_cr = LSA_NULL;
        iom_data.perif_page_ar_data[i].input_cr = LSA_NULL;
        iom_data.perif_page_ar_data[i].new_data_cbf = LSA_NULL;
        iom_data.perif_page_ar_data[i].free_request_issued = PNIO_FALSE;
        iom_data.perif_page_ar_data[i].is_sysred_provider = PNIO_FALSE;
        iom_data.perif_page_ar_data[i].prov_id = IOM_INVALID_PROV_ID;
    }

    /* initialize CR data */
    for(i = 0; i < IOM_CFG_PERIF_NUM_CR; i++)
    {
        iom_data.perif_page_cr_data[i].locked = 0;
        iom_data.perif_page_cr_data[i].cons_last_user_buffer_idx = 0;
        iom_data.perif_page_cr_data[i].start_address = LSA_NULL;
        iom_data.perif_page_cr_data[i].buffer_size = 0;
        iom_data.perif_page_cr_data[i].buffer_size_no_padding = 0;
        iom_data.perif_page_cr_data[i].apdu_status_address = LSA_NULL;
        iom_data.perif_page_cr_data[i].dma_job = -1;
        #ifdef IOM_CFG_DMA_BUF_EXCHANGE
        iom_data.perif_page_cr_data[i].dma_job_buf_exchange = -1;
        #endif
        iom_data.perif_page_cr_data[i].cr_index = i;
        iom_data.perif_page_cr_data[i].num_pages = 0;
        for(j = 0; j < 7; j++)
        {
            iom_data.perif_page_cr_data[i].used_pages[j].page_number = 0;
            iom_data.perif_page_cr_data[i].used_pages[j].page_offset_low = 0;
            iom_data.perif_page_cr_data[i].used_pages[j].page_offset_high = 0;
        }
    }

    /* initialize descriptor of memory pages */
    for(i = 0; i < IOM_PERIF_MAX_PAGE_INDEX; i++)
    {
        iom_data.perif_page_mem_desc[i].bytes_free = iom_data.page_size;
        iom_data.perif_page_mem_desc[i].bytes_free_from_start = iom_data.page_size;
        iom_data.perif_page_mem_desc[i].bytes_free_from_end = iom_data.page_size;

        /* each bit of the memory map corresponds to 8 bytes of the page, memory_map is a 32 bit variable */
        for (j = 0; j < iom_data.page_size / 8 / IOM_PERIF_MEMORY_MAX_OFFSET_IN_PAGE; j++)
        {
            iom_data.perif_page_mem_desc[i].memory_map[j] = 0;
        }
    }

    iom_data.perif_opened        = LSA_FALSE;
    iom_data.session_key_prim_ar = 0;

    iom_data.eddp_sys_handle.hd_nr = 1;

#if (IOM_DEBUG == 1)
    iom_data.eddp_sys_handle.magic_handle_type = PSI_SAFTY_MAGIC_HANDLE__PSI_EDD_SYS_TYPE;
#endif

    if(IOM_PERIF_IP_VERSION->version == 0x01)
    {
        /* Ertec200p Step1 : parameterize AHB burst accesses -> see requirements of EDDP, eddp_sys.doc . These settings are
         * necessary due to several Ertec200p Step1 bugs! (RQ 1022514) */
        IOM_PERIF_BURST_CONFIG->burst_mode_com_ahb = 0x01;
    }
    else
    {
        /* Ertec200p Step2 */
        IOM_PERIF_BURST_CONFIG->burst_mode_com_ahb = 0x03;
    }
    IOM_PERIF_BURST_CONFIG->burst_mode_appl_ahb = 0x00;


    /* disable all interrupt sources */
    *IOM_PERIF_PN_IRQ_MASK_LOW  = 0xFFFFFFFF;       /* disable interrupts */
    *IOM_PERIF_PN_IRQ_MASK_HIGH = 0xFFFFFFFF;
    *IOM_PERIF_PN_IRQACK_LOW    = 0xFFFFFFFF;       /* acknowledge pending interrupts */
    *IOM_PERIF_PN_IRQACK_HIGH   = 0xFFFFFFFF;

#ifdef IOM_SYSTEM_REDUNDANCY_SUPPORT

    IOM_EDDP_SYSRED_INIT();
    if( EDD_STS_OK != eddp_SysRed_Setup(EDD_INTERFACE_ID_MIN-1,
                                        0, 
                                        IOM_PERIF_PNIP_BASE_ADDR,
                                        IOM_EDDP_SYSRED_GET_SHMEM(1),
                                        EDDP_HW_ERTEC200P))
    {
        iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
    }

    iom_data.session_key_prim_ar = 0;    /* not determined so far */


#endif
}

/**
 * @brief   This function allocates memory for a new CR.
 *          - only paged mode is used to prevent fragmentation
 *          - 3 buffer mode is used
 *
 * @param ar_idx        [in] contains the AR number for which a new CR shall be created. Zero based value.
 * @param iocr_ptr      [in] contains a pointer to a structure describing the CR. Evaluated fields are
 *                      io_buffer_size, iocr_type. The following fields are set: eddx_data_offset, eddp_cr_number, eddp_forward_mode
 * @return              returns true, if everything went well
 */
LSA_BOOL iom_perif_allocate_iocr_memory(LSA_UINT32 ar_idx, LSA_UINT16 ar_set_nr, LSA_UINT16 session_key, CM_UPPER_SV_AR_CONNECT_IOCR_PTR_TYPE iocr_ptr)
{
    LSA_UINT32          buffer_size;
    LSA_UINT8           cr_index;
    iom_perif_cr_data_t *cr;

    if (IOM_GET_IS_PACKFRAME_FROM_IOCR_PTR(iocr_ptr))
    {
        /* not implemented. See edd_usr.doc for buffer structure */
        return LSA_FALSE;
    }

    IOM_ENTER();

    if(!iom_data.perif_opened)
    {
        iom_data.perif_opened = LSA_TRUE;

        /* register the global PERIF interrupt handler. The ARM interrupt is enabled with it. */
        IOM_SET_INT_HANDLER(iom_perif_isr_output_data_received);
    }

    IOM_ASSERT(iocr_ptr != LSA_NULL, LSA_FALSE);
    IOM_ASSERT(ar_idx < IOM_CFG_PERIF_NUM_AR, LSA_FALSE);
    IOM_ASSERT(IOM_GET_ALLOC_LEN_FROM_IOCR_PTR(iocr_ptr) < IOM_PERIF_MAX_ALLOC_SIZE, LSA_FALSE); /* 512 Bytes data, 256 Subslotsx2 Byte, 768 Bytes gap due to 4-byte-alignment */

    buffer_size = IOM_GET_ALLOC_LEN_FROM_IOCR_PTR(iocr_ptr);
    buffer_size = (buffer_size + 7) & IOM_PERIF_8_BYTE_ALINGMENT;   /* alignment to 8 byte, no APDU status in Ertec200p */



    /* ****************
     *  search a free CR
     * **************** */
    for(cr_index = 0; cr_index < IOM_CFG_PERIF_NUM_CR; cr_index++)
    {
        if(iom_data.perif_page_cr_data[cr_index].start_address == LSA_NULL)
        {
            break;
        }
    }
    IOM_ASSERT(cr_index < IOM_CFG_PERIF_NUM_CR, LSA_FALSE);     /* error case : no free CR found */

    /* *****************************************************************
    * register the CR as part of the AR ; initialize AR-wide parameters
    * ***************************************************************** */
    if(iocr_ptr->iocr_type == CM_IOCR_TYPE_INPUT || iocr_ptr->iocr_type == CM_IOCR_TYPE_MULTICAST_PROVIDER)
    {
        /* provider */
        IOM_ASSERT(iom_data.perif_page_ar_data[ar_idx].input_cr == LSA_NULL, LSA_FALSE);
        iom_data.perif_page_ar_data[ar_idx].input_cr = &iom_data.perif_page_cr_data[cr_index];      /* connect CR with AR */
        cr = iom_data.perif_page_ar_data[ar_idx].input_cr;
    }
    else if(iocr_ptr->iocr_type == CM_IOCR_TYPE_OUTPUT || iocr_ptr->iocr_type == CM_IOCR_TYPE_MULTICAST_CONSUMER)
    {
        /* consumer */
        IOM_ASSERT(iom_data.perif_page_ar_data[ar_idx].output_cr == LSA_NULL, LSA_FALSE);
        iom_data.perif_page_ar_data[ar_idx].output_cr = &iom_data.perif_page_cr_data[cr_index];
        cr = iom_data.perif_page_ar_data[ar_idx].output_cr;
    }
    else
    {
        iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
        IOM_EXIT_RETURN(LSA_FALSE);
    }
    iom_data.perif_page_ar_data[ar_idx].new_data_cbf = LSA_NULL;
    iom_data.perif_page_ar_data[ar_idx].free_request_issued = LSA_FALSE;

    /* ******************************************
     * search memory pages for the CR (paged mode)
     * ****************************************** */
    {
        LSA_UINT16  size_to_alloc = buffer_size;
        LSA_UINT8   page_index;

        if(size_to_alloc <= iom_data.page_size)
        {
            /* look for a single page that offers enough space for the complete requested buffer */
            LSA_UINT32  offset;
            LSA_UINT32  opt_page_index = 0xFFFFFFFF;
            LSA_UINT32  opt_offset = 0;
            LSA_UINT32  min_memory_waste = 1024;

            for(page_index = 0; page_index < IOM_PERIF_MAX_PAGE_INDEX; page_index++)
            {
                /* check if the page provides enough free memory and look for the page whose gap of unused memory
                 * optimally fits the current memory requirement
                 *
                 * with this strategy we avoid that 16 small CRs use all 16 pages partially -> a 17. CR
                 * would have no chance to allocate a larger chunk of memory
                 *  */
                if(iom_data.perif_page_mem_desc[page_index].bytes_free >= size_to_alloc)
                {
                    LSA_UINT16 size = size_to_alloc;

                    /* Look for a contiguous memory block in this page. Start the search at the beginning of this page.
                     * The size of the largest available chunk of memory and its start offset is returned. */
                    if(iom_perif_find_free_mem_block(page_index, &size, 0, &offset))
                    {
                        /* the page provides enough contiguous space for the whole CR buffer */

                        if(size >= size_to_alloc)
                        {
                            LSA_UINT16 size_waste = size - size_to_alloc;
                            if(size_waste < min_memory_waste)     /* examine how much unused space is left behind the memory to be allocated. */
                            {                                     /* Try to minimize it. */
                                min_memory_waste = size_waste;
                                opt_page_index = page_index;
                                opt_offset = offset;
                            }
                        }
                    }
                }
            }

            if(opt_page_index < IOM_PERIF_MAX_PAGE_INDEX)
            {
                /* a page has been found -> allocate memory inside of it */

                /* mark the used memory area of the page in the memory map and update
                 * the page- and CR descriptor*/
                iom_perif_add_used_mem_block(cr, opt_page_index, size_to_alloc, opt_offset);

                /* no further allocations necessary */
                size_to_alloc = 0;
            }
        }   /* endif : size_to_alloc <= IOM_PERIF_MEMORY_PAGE_SIZE (256)*/

        if(size_to_alloc)
        {
            /* buffer to be allocated is larger than IOM_PERIF_MEMORY_PAGE_SIZE (256) bytes OR
             * buffer is smaller but doesn't fit into a single page due to present memory usage
             * -> two or more pages are needed
             * -> the first memory block needs to be at an end of a page to attain
             *    a virtual memory space without gaps.  The last memory block needs to start at the beginning of a page. */

            LSA_UINT8   index_best_first_page = 255;
            LSA_UINT8   index_best_last_page = 255;
            LSA_UINT16  memory_wasted_best_first_page = 1024;
            LSA_UINT16  alloc_bytes_last_page = 0;

            for(page_index = 0; page_index < IOM_PERIF_MAX_PAGE_INDEX; page_index++)
            {
                LSA_UINT32  space_left_to_alloc;
                LSA_UINT8   complete_pages_needed;
                LSA_UINT8   page_index2;

                if(iom_data.perif_page_mem_desc[page_index].bytes_free_from_end > 0)
                {
                    if(size_to_alloc > iom_data.perif_page_mem_desc[page_index].bytes_free_from_end)     /* the end of the first page shall be completely filled up */
                    {
                        space_left_to_alloc = size_to_alloc - iom_data.perif_page_mem_desc[page_index].bytes_free_from_end;
                        complete_pages_needed = space_left_to_alloc / iom_data.page_size;

                        for(page_index2 = page_index + 1; page_index2 < IOM_PERIF_MAX_PAGE_INDEX; page_index2++)
                        {
                            if(complete_pages_needed == 0)
                            {
                                if(iom_data.perif_page_mem_desc[page_index2].bytes_free_from_start - space_left_to_alloc < memory_wasted_best_first_page)
                                {
                                    /* the aim is to minimize the amount of memory that is left blank in the last used page */
                                    memory_wasted_best_first_page = iom_data.perif_page_mem_desc[page_index2].bytes_free_from_start - space_left_to_alloc;
                                    alloc_bytes_last_page = space_left_to_alloc;
                                    index_best_first_page = page_index;
                                    index_best_last_page = page_index2;
                                }
                            }
                            else
                            {
                                if(iom_data.perif_page_mem_desc[page_index2].bytes_free == iom_data.page_size)
                                {
                                    complete_pages_needed--;
                                    space_left_to_alloc -= iom_data.page_size;
                                    if (space_left_to_alloc == 0 && memory_wasted_best_first_page > 0)
                                    {
                                        memory_wasted_best_first_page = 0;
                                        alloc_bytes_last_page = iom_data.page_size;
                                        index_best_first_page = page_index;
                                        index_best_last_page  = page_index2;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if(index_best_first_page < IOM_PERIF_MAX_PAGE_INDEX)
            {
                LSA_UINT16  alloc_bytes_first_page = iom_data.perif_page_mem_desc[index_best_first_page].bytes_free_from_end;

                /* allocate memory of first page */
                iom_perif_add_used_mem_block(cr,                               \
                                             index_best_first_page,            \
                                             alloc_bytes_first_page,           \
                                               iom_data.page_size - alloc_bytes_first_page);

                /* allocate memory of last page */
                iom_perif_add_used_mem_block(cr,                                 \
                                             index_best_last_page,               \
                                             alloc_bytes_last_page,              \
                                             0);

                size_to_alloc -= alloc_bytes_first_page + alloc_bytes_last_page;

                /* allocate the left memory using completely free pages */
                page_index = index_best_first_page + 1;
                while (size_to_alloc && (page_index < index_best_last_page) && (page_index < IOM_PERIF_MAX_PAGE_INDEX))
                {
					if(iom_data.perif_page_mem_desc[page_index].bytes_free == iom_data.page_size)
                    {
                        /* mark the used memory area of the page in the memory map and update
                         * the page- and CR descriptor. Data is allocated from the page beginning */
						if(iom_perif_add_used_mem_block(cr, page_index, iom_data.page_size, 0) == LSA_FALSE)
                        {
                            /* page couldn't be allocated -> stop -> size_to_alloc stays greater than 0 -> error */
                            break;
                        }

						size_to_alloc -= iom_data.page_size;
                    }
                    page_index++;
                }
            }
        }

        cr->buffer_size = buffer_size;
        cr->buffer_size_no_padding = IOM_GET_ALLOC_LEN_FROM_IOCR_PTR(iocr_ptr);
        cr->cr_index = cr_index + 1;        /* cr index is a 1-based index */
        if(iom_data.page_size == 512)
        {
            IOM_ASSERT(IOM_CFG_PERIF_NUM_CR < 16, LSA_FALSE);
            IOM_ASSERT(cr->cr_index < 16, LSA_FALSE);  // number of usable CRs in page mode is limited to 15 CRs because there's just 64kB address space available
            cr->start_address = (LSA_UINT32*) (U_PERIF_AHB__BASE + (cr->cr_index << 12) + cr->used_pages[0].page_offset_low);   /* address of the first page */
        }
        else
        {
            cr->start_address = (LSA_UINT32*) (U_PERIF_AHB__BASE + (cr->cr_index << 11) + cr->used_pages[0].page_offset_low);   /* address of the first page */
        }
        if (iocr_ptr->iocr_type == CM_IOCR_TYPE_OUTPUT || iocr_ptr->iocr_type == CM_IOCR_TYPE_MULTICAST_CONSUMER)
        {
            /* get the address within PNIP, where the APDU-status is stored */
            IOM_ASSERT((IOM_GET_APDU_OFFSET_FROM_IOCR_PTR(iocr_ptr) != 0) && (IOM_GET_APDU_OFFSET_FROM_IOCR_PTR(iocr_ptr) != EDD_DATAOFFSET_INVALID), LSA_FALSE);
            cr->apdu_status_address = (LSA_VOID*)(U_PNIP__BASE + (IOM_GET_APDU_OFFSET_FROM_IOCR_PTR(iocr_ptr)));
        }

        if(size_to_alloc)
        {
            /* there's still need for memory -> we can't provide it -> deallocate the memory and return with error */
            iom_perif_delete_cr_descriptor_data(ar_idx, cr);

            IOM_EXIT_RETURN(LSA_FALSE);
        }

        if(iom_data.page_size == 512)
        {
            IOM_SET_DATA_OFFSET_IN_IOCR_PTR(iocr_ptr, (cr->cr_index << 12) + cr->used_pages[0].page_offset_low);
        }
        else
        {
            IOM_SET_DATA_OFFSET_IN_IOCR_PTR(iocr_ptr, (cr->cr_index << 11) + cr->used_pages[0].page_offset_low);
        }
        IOM_SET_CR_NUMBER_IN_IOCR_PTR(iocr_ptr, cr->cr_index);
        IOM_SET_FORWARD_MODE_IN_IOCR_PTR(iocr_ptr, EDD_FORWARDMODE_SF); /* store and forward mode */
    }

    #ifdef IOM_SYSTEM_REDUNDANCY_SUPPORT
    /* system redundancy specific: add a provider. This allows us to set the APDU status via
     * iom_perif_provider_set_data_state() */
    if( (iocr_ptr -> iocr_type ==CM_IOCR_TYPE_INPUT) || (iocr_ptr -> iocr_type ==CM_IOCR_TYPE_MULTICAST_PROVIDER) )
    {
        LSA_UINT16  ret_value;
        LSA_UINT8   local_prov_type;
        LSA_UINT16  local_provider_id = 0;

        if (ar_set_nr == 0)
        {
            local_prov_type = EDD_PROVADD_TYPE_DEFAULT;
        }
        else
        {
            local_prov_type = EDD_PROVADD_TYPE_SYSRED;
        }

        if (local_prov_type == EDD_PROVADD_TYPE_SYSRED)
        {
            ret_value = eddp_SysRed_Provider_Add(EDD_INTERFACE_ID_MIN - 1,
                                                 &local_provider_id,
                                                 IOM_GET_DATA_STATUS_OFFSET_FROM_IOCR_PTR(iocr_ptr)/*DataStatusOffset */,
                                                 local_prov_type,
                                                 (EDDP_SYS_HANDLE)&(iom_data.eddp_sys_handle));
            if (ret_value != EDD_STS_OK)
            {
                iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, ret_value);
            }

            ret_value = eddp_SysRed_ShM_Array_Set(EDD_INTERFACE_ID_MIN-1, ar_set_nr-1,iom_data.session_key_prim_ar);
            if (ret_value != EDD_STS_OK)
            {
                iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, ret_value);
            }

            /* now the provider ident and the corresponding acw is known */
            iom_data.perif_page_ar_data[ar_idx].prov_id =  local_provider_id;

            iom_data.perif_page_ar_data[ar_idx].is_sysred_provider = PNIO_TRUE;

            iom_data.perif_page_ar_data[ar_idx].prov_session_key = session_key;
        }
    }
    #endif

    /* now all the data structures are set up -> configure registers ... */


    /* *************************
     * set CR_Address_x register
     * ************************* */
    IOM_PERIF_CR_ADDRESS_N__SET_FIELD(cr->cr_index, cr_start_address, 0);  /* not applicable to paged mode */
    IOM_PERIF_CR_ADDRESS_N__SET_FIELD(cr->cr_index, cr_end_address  , 0);  /* not applicable to paged mode */
    IOM_PERIF_CR_ADDRESS_N__SET_FIELD(cr->cr_index, zero_data       , 0);  /* zero data after overflow of data-hold-timer watchdog / PNIP */
    IOM_PERIF_CR_ADDRESS_N__SET_FIELD(cr->cr_index, new_data_int    , 0);  /* new data interrupt to be triggered : default = off. See beneath for potential activation*/

    /* ************************************************************************************
     * set CR_State_x register
     * - only "normal mode" is supported by EDDP (see eddp_sys.doc, chapter 4.2)
     * - only "3 buffer mode" and cut through mode is supported by EDDP (see eddp_sys.doc, chapter 4.2)
     * - page mode is used to avoid fragementation
     * *************************************************************************************/
    {
        iom_perif_cr_state_reg_t    state_reg_temp;
        LSA_UINT8                   page_index;

        if(IOM_PERIF_IP_VERSION->version == 0x01)
        {
            /* Ertec200p Step1 */
            state_reg_temp.perif_v1.cr_state_u  = PERIF_BUFFER_0;        /* init state of 3 buffer exchange system */
            state_reg_temp.perif_v1.cr_state_n1 = PERIF_BUFFER_NIL;      /* no buffer in Next state */
            state_reg_temp.perif_v1.cr_state_f1 = PERIF_BUFFER_1;
            state_reg_temp.perif_v1.cr_state_d  = PERIF_BUFFER_2;
            state_reg_temp.perif_v1.cr_state_f2 = PERIF_BUFFER_NIL;      /* not applicable */
            state_reg_temp.perif_v1.cr_state_n2 = PERIF_BUFFER_NIL;      /* not applicable */
            state_reg_temp.perif_v1.reserved0   = 0;
            state_reg_temp.perif_v1.reserved1   = 0;

            cr->cons_last_user_buffer_idx = PERIF_BUFFER_0;
        }
        else
        {
            /* Ertec200p Step2 */
            if(iocr_ptr->iocr_type == CM_IOCR_TYPE_OUTPUT || iocr_ptr->iocr_type == CM_IOCR_TYPE_MULTICAST_CONSUMER)
            {
                /* O-FSM (4 buffers, now 2 different D-buffers "R1, R2" in redundance mode) */
                state_reg_temp.perif_v2.cr_state_u  = PERIF_BUFFER_3;        /* init state of 4 buffer exchange system */
                state_reg_temp.perif_v2.cr_state_n1 = PERIF_BUFFER_3;        /* n = u -> is interpreted as N = NIL in redundance mode */
                state_reg_temp.perif_v2.cr_state_f1 = PERIF_BUFFER_1;
                state_reg_temp.perif_v2.cr_state_d  = PERIF_BUFFER_2;
                state_reg_temp.perif_v2.cr_state_f2 = PERIF_BUFFER_NIL;      /* not applicable */
                state_reg_temp.perif_v2.cr_state_n2 = PERIF_BUFFER_NIL;      /* not applicable */
                state_reg_temp.perif_v2.cr_state_r2 = PERIF_BUFFER_0;
                state_reg_temp.perif_v2.redundance_mode = 1;           /* is set/used according to EDDP spec */

                cr->cons_last_user_buffer_idx = PERIF_BUFFER_3;
            }
            else
            {
                /* I-FSM (works like without redundance mode in Step1, R2 not used, buffer 3 can be interpreted as NIL) */
                state_reg_temp.perif_v2.cr_state_u  = PERIF_BUFFER_0;        /* init state of 3 buffer exchange system */
                state_reg_temp.perif_v2.cr_state_n1 = PERIF_BUFFER_NIL;
                state_reg_temp.perif_v2.cr_state_f1 = PERIF_BUFFER_1;
                state_reg_temp.perif_v2.cr_state_d  = PERIF_BUFFER_2;
                state_reg_temp.perif_v2.cr_state_f2 = PERIF_BUFFER_NIL;      /* not applicable */
                state_reg_temp.perif_v2.cr_state_n2 = PERIF_BUFFER_NIL;      /* not applicable */
                state_reg_temp.perif_v2.cr_state_r2 = 0;                     /* default - not used */
                state_reg_temp.perif_v2.redundance_mode = 1;
            }
        }

        state_reg_temp.perif_v1.three_buffer_mode = 0;                       /* normal mode */


        /* page mapping */
        state_reg_temp.perif_v1.mapping = 0;
        for(page_index = 0; page_index < cr->num_pages; page_index++)
        {
            state_reg_temp.perif_v1.mapping |= (1 << cr->used_pages[page_index].page_number);
        }

        *IOM_PERIF_CR_STATE_N(cr->cr_index) = state_reg_temp;
    }

    /*********************************************************
     * mark DMA job as disabled - has to be enabled separately
     *********************************************************/
    cr->dma_job = -1;
    IOM_PERIF_GUARD_CONTROL_N(cr->cr_index)->guard_valid = 0;

    /* initialize the data buffer */
    if((iocr_ptr -> iocr_type ==CM_IOCR_TYPE_INPUT) || (iocr_ptr -> iocr_type ==CM_IOCR_TYPE_MULTICAST_PROVIDER))
    {
        LSA_UINT8                   *buffer;
        iom_perif_cr_state_reg_t    state_reg_temp = *IOM_PERIF_CR_STATE_N(cr->cr_index);

            /* 1. initialize buffer 0 */
        buffer = (LSA_UINT8*) iom_data.perif_page_ar_data[ar_idx].input_cr->start_address;
        IOM_MEMSET(buffer, 0, buffer_size);
            /* 2. initialize buffer 1 (no difference in access between Step1 and Step2) */
        state_reg_temp.perif_v1.cr_state_u  = PERIF_BUFFER_1;
        state_reg_temp.perif_v1.cr_state_f1 = PERIF_BUFFER_0;
        *IOM_PERIF_CR_STATE_N(cr->cr_index) = state_reg_temp;
        IOM_MEMSET(buffer, 0, buffer_size);
            /* 3. initialize buffer 2 */
        state_reg_temp.perif_v1.cr_state_u  = PERIF_BUFFER_2;
        state_reg_temp.perif_v1.cr_state_d  = PERIF_BUFFER_1;
        *IOM_PERIF_CR_STATE_N(cr->cr_index) = state_reg_temp;
        IOM_MEMSET(buffer, 0, buffer_size);
            /* back to init state */
        state_reg_temp.perif_v1.cr_state_u  = PERIF_BUFFER_0;        /* init state of 3 buffer exchange system */
        state_reg_temp.perif_v1.cr_state_f1 = PERIF_BUFFER_1;
        state_reg_temp.perif_v1.cr_state_d  = PERIF_BUFFER_2;
        *IOM_PERIF_CR_STATE_N(cr->cr_index) = state_reg_temp;
    }

    /* return */
    IOM_EXIT_RETURN(LSA_TRUE);
}

#ifdef IOM_CFG_USE_GDMA
/**
 * @brief This function enables the address guard for a previously initialized CR
 *
 * @param ar_idx [in]    :  AR index of the AR that shall use address guard
 * @param iocr_type [in] :  specifies the CR of the AR for which address guard shall be enabled.
 *                          Values = CM_IOCR_TYPE_INPUT / CM_IOCR_TYPE_OUTPUT / ...
 * @param address        :  address at which a buffer exchange is triggered. For output CRs, the exchange
 *                          is triggered in advance to the access of <address>, otherwise after the access.
 * @return  IOM_OK if no errors occured. IOM_PARAMETER_ERROR otherwise.
 */
static LSA_UINT16 iom_perif_enable_address_guard(LSA_UINT32 ar_idx, LSA_UINT16 iocr_type, LSA_VOID *address)
{
    iom_perif_cr_data_t *cr;
    LSA_UINT32 address_offset;

    /* IOM_ENTER(); --> now internal function, no lock necessary */

    IOM_ASSERT(ar_idx < IOM_CFG_PERIF_NUM_AR, IOM_PARAMETER_ERROR);


    if(iom_data.page_size == 512)
    {
        address_offset = (LSA_UINT32) address & 0xFFF;  // 512 bytes per page -> 9 bit inner-page offset and 3 bits page index
    }
    else
    {
        // we just need the 11 bit address offset relative to PERIF's first address
        address_offset = (LSA_UINT32) address & 0x7FF;
    }

    switch(iocr_type)
    {
        case CM_IOCR_TYPE_INPUT:
        case CM_IOCR_TYPE_MULTICAST_PROVIDER:
            IOM_ASSERT(iom_data.perif_page_ar_data[ar_idx].input_cr != LSA_NULL, IOM_INVALID_BUFFER);
            cr = iom_data.perif_page_ar_data[ar_idx].input_cr;
            // trigger the buffer-switch after an access to address <address> in PERIF
            IOM_PERIF_GUARD_CONTROL_N(cr->cr_index)->guard_address = (LSA_UINT32) address_offset;
            IOM_PERIF_GUARD_CONTROL_N(cr->cr_index)->guard_type = 1;
            break;
        case CM_IOCR_TYPE_OUTPUT:
        case CM_IOCR_TYPE_MULTICAST_CONSUMER:
            IOM_ASSERT(iom_data.perif_page_ar_data[ar_idx].output_cr != LSA_NULL, IOM_INVALID_BUFFER);
            cr = iom_data.perif_page_ar_data[ar_idx].output_cr;
            // trigger the buffer-switch in advance to an access to address <address> in PERIF
            IOM_PERIF_GUARD_CONTROL_N(cr->cr_index)->guard_address = (LSA_UINT32) address_offset;
            IOM_PERIF_GUARD_CONTROL_N(cr->cr_index)->guard_type = 0;
            break;
        default:
            iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
            return IOM_INTERNAL_ERROR;
    }

    IOM_PERIF_GUARD_CONTROL_N(cr->cr_index)->guard_valid = 1;

    /* IOM_EXIT_RETURN(IOM_OK); */
    return IOM_OK;
}

/**
 * @brief   This function installs a DMA request as described by the parameter <dma_request> for the CR defined
 *          by <ar_idx> and <iocr_type>.
 *          Therefore the GDMA reusable software module is used. The CR buffer is completely copied as one block
 *          to / from Perif with a granularity of 4 bytes. The job is always created in GDMA job pool 0 (first job pool
 *          - see GDMA module for details).
 *          The DMA transfer gets uninstalled as soon as the PERIF AR is deallocated. Don't remove the job on your own.
 *          Don't forget to activate the DMA transfer after it has been installed.
 *
 * @param   ar_idx      [in] specifies the AR, to which the corresponding CR belongs
 * @param   iocr_type   [in] specifies the CR within the AR [CM_IOCR_TYPE_INPUT / CM_IOCR_TYPE_OUTPUT / ...]
 * @param   dma_request [in] determines the buffer addresses of the DMA copy job and further parameters
 * @param   gdma_handle [out] returns the handle of the created DMA job. It can be used to start software triggered jobs. If gdma_handle == LSA_NULL, no handle is returned.
 *
 * @return  IOM_OK if everything went well, IOM_INTERNAL_ERROR or IOM_PARAMETER_ERROR if an error occured
 */
LSA_UINT16 iom_perif_install_dma_transfer(LSA_UINT32 ar_idx, LSA_UINT16 iocr_type, iom_dma_request_t *dma_request, LSA_INT16 *gdma_handle)
{
    iom_perif_cr_data_t     *cr;
    gdma_job_descriptor_t   job_desc;
    gdma_transfer_descr_t   transfer_desc[1];

    IOM_ENTER();

    IOM_ASSERT(ar_idx < IOM_CFG_PERIF_NUM_AR, IOM_PARAMETER_ERROR);
    IOM_ASSERT(dma_request != LSA_NULL, IOM_PARAMETER_ERROR);

    job_desc.num_transfers = 1;
    job_desc.hw_flow_src = GDMA_HW_FLOW_OFF;
    job_desc.transfers = transfer_desc;

    switch(iocr_type)
    {
        case CM_IOCR_TYPE_INPUT:
        case CM_IOCR_TYPE_MULTICAST_PROVIDER:
            /* provider */
            IOM_ASSERT(iom_data.perif_page_ar_data[ar_idx].input_cr != LSA_NULL, IOM_INVALID_BUFFER);
            cr = iom_data.perif_page_ar_data[ar_idx].input_cr;

            // trigger the buffer-switch after the last byte is written into PERIF
            // -4 because we need the offset of the last data word (not the one after the last data word))
            iom_perif_enable_address_guard(ar_idx, iocr_type, (LSA_UINT8*)cr->start_address + cr->buffer_size - 4);

            job_desc.trigger  = dma_request->in_cr.trigger;
            job_desc.priority = dma_request->in_cr.priority;
            job_desc.job_done_callback = dma_request->in_cr.job_done_callback;
            job_desc.cbf_arg  = dma_request->in_cr.cbf_arg;

            /* we transfer with 4 byte granularity, transfer length = buffer_size */
            GDMA_SET_TRANSFER_DESCR(transfer_desc, 0, dma_request->in_cr.src_addr, cr->start_address, (cr->buffer_size >> 2), GDMA_ESIZE_32BIT, GDMA_BURST_MODE_INCR8);
            break;
        case CM_IOCR_TYPE_OUTPUT:
        case CM_IOCR_TYPE_MULTICAST_CONSUMER:
            /* consumer */
            IOM_ASSERT(iom_data.perif_page_ar_data[ar_idx].output_cr != LSA_NULL, IOM_INVALID_BUFFER);
            cr = iom_data.perif_page_ar_data[ar_idx].output_cr;

            // trigger the buffer-switch before the first byte is read from PERIF
            iom_perif_enable_address_guard(ar_idx, iocr_type, (LSA_UINT8*) cr->start_address);

            job_desc.trigger  = dma_request->out_cr.trigger;
            job_desc.priority = dma_request->out_cr.priority;
            job_desc.job_done_callback = dma_request->out_cr.job_done_callback;
            job_desc.cbf_arg  = dma_request->out_cr.cbf_arg;

            GDMA_SET_TRANSFER_DESCR(transfer_desc, 0, cr->start_address, dma_request->out_cr.dest_addr, (cr->buffer_size >> 2), GDMA_ESIZE_32BIT, GDMA_BURST_MODE_INCR8);
            break;
        default:
            iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
            IOM_EXIT_RETURN(IOM_INTERNAL_ERROR);
    }

    cr->dma_job = gdma_add_job(0, &job_desc);
    IOM_ASSERT(GDMA_IS_VALID_JOB(cr->dma_job), IOM_INTERNAL_ERROR);

    if(gdma_handle)
    {
        *gdma_handle = cr->dma_job;
    }

    IOM_EXIT_RETURN(IOM_OK);
}
#endif

/**
 * @brief   This function enables the "new data ready" interrupt for an output CR.
 *
 * @param ar_idx        [in] zero-based index of the AR whose ouput CR shall signal new data via an interrupt
 * @param new_data_cbf  [in] pointer to a callback function, that's called when new data is available.
 *                           The AR index is passed to this callback in case of an interrupt.
 * @return              IOM_OK if no error occured, IOM_PARAMETER_ERROR or IOM_INVALID_BUFFER otherwise.
 */
LSA_UINT16 iom_perif_install_consumer_data_ready_isr(LSA_UINT32 ar_idx, LSA_VOID (*new_data_cbf)(LSA_UINT8))
{
    iom_perif_cr_data_t *cr;

    IOM_ENTER();

    IOM_ASSERT(ar_idx < IOM_CFG_PERIF_NUM_AR, IOM_PARAMETER_ERROR);

    /* consumer */
    if(iom_data.perif_page_ar_data[ar_idx].output_cr == NULL)
    {
        /* no output CR was allocated for this AR. This can happen if it's an AR without cyclic data */
        IOM_EXIT_RETURN(IOM_INVALID_BUFFER);
    }
    cr = iom_data.perif_page_ar_data[ar_idx].output_cr;

    /* ******************************************************
     * enable new-data-interrupt for this CR / AR, if desired
     * ****************************************************** */
    if(new_data_cbf)
    {
        iom_data.perif_page_ar_data[ar_idx].new_data_cbf = new_data_cbf;
        IOM_PERIF_ACKNOWLEDGE_NEW_DATA_INT(ar_idx);                                 /* acknowledge pending interrupt events */
        IOM_PERIF_CR_ADDRESS_N__SET_FIELD(cr->cr_index, new_data_int, ar_idx + 1);  /* generate new data events */
        IOM_PERIF_ACTIVATE_NEW_DATA_INT(ar_idx);                                    /* allow events to pass the interrupt mask */
    }

    IOM_EXIT_RETURN(IOM_OK);
}

/**
 * @brief   This function releases all PERIF-resources, allocated by the CRs of an AR synchronously. No buffer may be locked
 *          at the time of release!
 *          An asynchronous attempt, that waits for the next incoming data from PNIP (receive interrupt) before the AR memory is released, failed:
 *          obviously the user data connection is stopped before this function is called.
 *          A corresponding callback is executed to inform the application when deallocation is done (see iom_cfg.h).
 *
 *          AR-/CR meta data is reset, CR-registers in PERIF are reset.
 *
 * @param ar_idx    [in] AR that is to be released
 */
LSA_VOID iom_perif_free_iocr_memory(LSA_UINT32 ar_idx)
{
    if(ar_idx >= IOM_CFG_PERIF_NUM_AR)
    {
        iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
        return;
    }
    if(iom_data.perif_page_ar_data[ar_idx].output_cr == LSA_NULL && iom_data.perif_page_ar_data[ar_idx].input_cr == LSA_NULL)
    {
        /* maybe it's an AR without cyclic data -> don't throw an error */
        IOM_IOCR_MEMORY_FREED(ar_idx);                                  /* inform user about the completion of deallocation */
        return;
        //iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
    }

    IOM_ENTER();

    if((iom_data.perif_page_ar_data[ar_idx].output_cr && iom_data.perif_page_ar_data[ar_idx].output_cr->locked) ||
       (iom_data.perif_page_ar_data[ar_idx].input_cr  && iom_data.perif_page_ar_data[ar_idx].input_cr->locked))
    {
        /* resources can't be released now - wait until the corresponding unlock()-routine gets called */
        iom_data.perif_page_ar_data[ar_idx].free_request_issued = LSA_TRUE;
    }
    else
    {
        iom_perif_free_iocr_memory_local(ar_idx, IOM_PERIF_ALL_CRS);    /* release all resources of the AR */
        IOM_IOCR_MEMORY_FREED(ar_idx);                                  /* inform user about the completion of deallocation */
    }

    IOM_EXIT();
}

#ifdef IOM_CFG_LOCATE_INSTRUCTIONS_TO_TCM
    #if defined (TOOL_CHAIN_GREENHILLS_ARM)
        #pragma ghs section text=IOM_ITCM_SECTION_NAME
    #endif
#endif
/**
 * @brief This function retrieves and locks a buffer of an output CR.
 *        No other task can lock the buffer again (prior to that an unlock is needed).
 *        If a DMA job was connected to this CR, an automatic data transfer
 *        to the parameterized destination buffer is initiated.
 *
 * @param consumer_buffer   [out] address of the current PERIF-user-CR buffer
 * @param apdu_status_ptr   [out] address within the PNIP, where the APDU status can be found
 * @param ar_idx            [in]  index of the AR whose output buffer should be retrieved
 * @return                  IOM_INVALID_BUFFER if the output CR is not initialized,
 *                          IOM_BUFFER_LOCKED if the CR memory is already locked,
 *                          IOM_OK_NEW_BUFFER or IOM_OK_OLD_BUFFER if locking was successful
 */
IOM_ATTRIBUTE_ITCM LSA_UINT16 iom_perif_consumer_lock(LSA_VOID** consumer_buffer, iom_apdu_status_t** apdu_status_ptr, LSA_UINT32 ar_idx)
{
    iom_perif_command_if_control_reg_t cmd_if_reg;

    IOM_ENTER_IO();

    IOM_ASSERT(ar_idx < IOM_CFG_PERIF_NUM_AR, IOM_PARAMETER_ERROR);
    IOM_ASSERT(consumer_buffer != NULL, IOM_PARAMETER_ERROR);
    IOM_ASSERT(iom_data.perif_page_ar_data[ar_idx].output_cr != LSA_NULL, IOM_INVALID_BUFFER);
    IOM_ASSERT(iom_data.perif_page_ar_data[ar_idx].output_cr->locked == 0, IOM_BUFFER_LOCKED);

    iom_data.perif_page_ar_data[ar_idx].output_cr->locked = 1;

    *consumer_buffer = iom_data.perif_page_ar_data[ar_idx].output_cr->start_address;
    if(apdu_status_ptr != LSA_NULL)
    {
        *apdu_status_ptr = (iom_apdu_status_t*) iom_data.perif_page_ar_data[ar_idx].output_cr->apdu_status_address;
    }

    /* copy the output data to the destination buffer. Do it via DMA if parameterized. If not,
     * get the latest buffer using buffer switch and return its address */
    #ifdef IOM_CFG_USE_GDMA
    if(GDMA_IS_VALID_JOB_IDX(iom_data.perif_page_ar_data[ar_idx].output_cr->dma_job))
    {
        gdma_trigger_job(iom_data.perif_page_ar_data[ar_idx].output_cr->dma_job);       /* the address guard performs the ConsRD before the first DMA word is read */
        IOM_EXIT_RETURN_IO(IOM_OK_NEW_BUFFER);
    }
    else
    #endif
    {
        /* get the next filled buffer from PN-IP and return the read address */
        IOM_PERIF_COMMAND_IF_CONTROL__SET_FIELD(&cmd_if_reg, cr_number   , iom_data.perif_page_ar_data[ar_idx].output_cr->cr_index);
        IOM_PERIF_COMMAND_IF_CONTROL__SET_FIELD(&cmd_if_reg, conf_request, 0);
        IOM_PERIF_COMMAND_IF_CONTROL__SET_FIELD(&cmd_if_reg, f_code      , 1);  /* ConsRD -> get new user buffer, previously filled by PNIP */
        IOM_PERIF_COMMAND_IF_CONTROL__SET_FIELD(&cmd_if_reg, user_id     , 0);

        *IOM_PERIF_COMMAND_IF_CONTROL = cmd_if_reg;

        if(iom_data.perif_page_ar_data[ar_idx].output_cr->cons_last_user_buffer_idx == IOM_PERIF_CR_STATE_N(iom_data.perif_page_ar_data[ar_idx].output_cr->cr_index)->perif_v1.cr_state_u)
        {
            iom_data.perif_page_ar_data[ar_idx].output_cr->cons_last_user_buffer_idx = IOM_PERIF_CR_STATE_N(iom_data.perif_page_ar_data[ar_idx].output_cr->cr_index)->perif_v1.cr_state_u;
            IOM_EXIT_RETURN_IO(IOM_OK_OLD_BUFFER);
        }
        else
        {
            iom_data.perif_page_ar_data[ar_idx].output_cr->cons_last_user_buffer_idx = IOM_PERIF_CR_STATE_N(iom_data.perif_page_ar_data[ar_idx].output_cr->cr_index)->perif_v1.cr_state_u;
            IOM_EXIT_RETURN_IO(IOM_OK_NEW_BUFFER);
        }
    }
}

/**
 * @brief   This function unlocks a buffer previously locked by iom_perif_consumer_lock(). iom_perif_consumer_unlock()
 *          may not be called if a previous call to iom_perif_consumer_lock() returned with an error.
 *
 * @param ar_idx    [in] specifies the AR that contains the output-CR to be unlocked
 * @return          IOM_PERIF_INVALID_BUFFER if the output CR is not initialized, IOM_PERIF_BUFFER_UNLOCKED if the buffer
 *                  isn't in the locked state, else IOM_PERIF_OK. Here you get no destinction between old and new buffers.
 */
IOM_ATTRIBUTE_ITCM LSA_UINT16 iom_perif_consumer_unlock(LSA_UINT32 ar_idx)
{
    IOM_ENTER_IO();

    IOM_ASSERT(ar_idx < IOM_CFG_PERIF_NUM_AR, IOM_PARAMETER_ERROR);
    IOM_ASSERT(iom_data.perif_page_ar_data[ar_idx].output_cr != LSA_NULL, IOM_INVALID_BUFFER);
    IOM_ASSERT(iom_data.perif_page_ar_data[ar_idx].output_cr->locked == LSA_TRUE, IOM_BUFFER_UNLOCKED);

    iom_data.perif_page_ar_data[ar_idx].output_cr->locked = 0;

    /* if the AR is about to be closed -> try to close it now */
    if(iom_data.perif_page_ar_data[ar_idx].free_request_issued)
    {
        if(!(iom_data.perif_page_ar_data[ar_idx].output_cr->locked || iom_data.perif_page_ar_data[ar_idx].input_cr->locked))
        {
            iom_perif_free_iocr_memory_local(ar_idx, IOM_PERIF_ALL_CRS);    /* release all resources of the AR */
            IOM_IOCR_MEMORY_FREED(ar_idx);                                  /* inform user about the completion of deallocation */
            iom_data.perif_page_ar_data[ar_idx].free_request_issued = LSA_FALSE;
        }
    }

    IOM_EXIT_RETURN_IO(IOM_OK);
}

#ifdef IOM_CFG_DMA_BUF_EXCHANGE
/* currently not part of the IOM interface. Reactivate the code if necessary*/

static iom_perif_command_if_control_reg_t iom_perif_dma_consumer_exchange_cmd;
/**
 * @brief   This function creates a DMA job, that triggers only the consumer buffer exchange. The address guard is not used
 *          and no data is copied in contrast to the DMA-parameterization opportunities that iom_perif_allocate_iocr_memory() offers.
 *
 * @return  LSA_TRUE on success, LSA_FALSE on error
 */
IOM_ATTRIBUTE_ITCM LSA_BOOL iom_perif_create_dma_consumer_buf_exchange(LSA_UINT32 ar_idx, gdma_trigger_src_t dma_trigger_source, LSA_UINT8 priority)
{
    gdma_job_descriptor_t job_desc_input;
    gdma_transfer_descr_t transfer_desc_input[1];

    IOM_ENTER();

    IOM_ASSERT(ar_idx < IOM_CFG_PERIF_NUM_AR, LSA_FALSE);
    IOM_ASSERT(GDMA_IS_VALID_JOB(iom_data.perif_page_ar_data[ar_idx].output_cr->dma_job) == LSA_FALSE, LSA_FALSE);              /* error case: DMA already activated for this CR */
    IOM_ASSERT(GDMA_IS_VALID_JOB(iom_data.perif_page_ar_data[ar_idx].output_cr->dma_job_buf_exchange) == LSA_FALSE, LSA_FALSE); /* error case: DMA already activated for this CR */

    iom_perif_dma_consumer_exchange_cmd.cr_number = iom_data.perif_page_ar_data[ar_idx].output_cr->cr_index;
    iom_perif_dma_consumer_exchange_cmd.conf_request = 0;
    iom_perif_dma_consumer_exchange_cmd.f_code = 1;              /* ConsRD -> get new user buffer, previously filled by PNIP */
    iom_perif_dma_consumer_exchange_cmd.user_id = 0;

    GDMA_SET_TRANSFER_DESCR(transfer_desc_input, 0,
                            &iom_perif_dma_consumer_exchange_cmd,
                            IOM_PERIF_COMMAND_IF_CONTROL,
                            1,
                            GDMA_ESIZE_32BIT,
                            GDMA_BURST_MODE_SINGLE);
    job_desc_input.job_done_callback = LSA_NULL;
    job_desc_input.num_transfers = 1;
    job_desc_input.priority = priority;
    job_desc_input.trigger = (gdma_trigger_src_t) dma_trigger_source;
    job_desc_input.hw_flow_src = GDMA_HW_FLOW_OFF;
    job_desc_input.transfers = transfer_desc_input;
    iom_data.perif_page_ar_data[ar_idx].output_cr->dma_job_buf_exchange = iom_gdma_add_job(&job_desc_input);
    IOM_ASSERT(GDMA_IS_VALID_JOB(iom_data.perif_page_ar_data[ar_idx].output_cr->dma_job_buf_exchange) == LSA_TRUE, LSA_FALSE);

    return LSA_TRUE;
}
#endif

/**
 * @brief   This function returns the address of the PERIF-input-CR, belonging to AR ar_idx. The buffer
 *          of the input CR gets locked until iom_perif_provider_unlock() is called.
 *
 * @param provider_buffer   [out] provider_buffer is set according to the buffer address of the AR's input CR
 * @param ar_idx            [in]  ar_idx specifies the AR whose input-CR-buffer shall be locked
 */
IOM_ATTRIBUTE_ITCM LSA_VOID iom_perif_provider_lock(PNIO_VOID **provider_buffer, LSA_UINT32 ar_idx)
{
     IOM_ENTER_IO();

     if(ar_idx >= IOM_CFG_PERIF_NUM_AR)
     {
         iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
         return;
     }
     if(provider_buffer == LSA_NULL)
     {
         iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
         return;
     }
     if(iom_data.perif_page_ar_data[ar_idx].input_cr == LSA_NULL)
     {
         iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
         return;
     }
     if(iom_data.perif_page_ar_data[ar_idx].input_cr->locked != LSA_FALSE)
     {
         iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
         return;
     }

     iom_data.perif_page_ar_data[ar_idx].input_cr->locked = 1;

     *provider_buffer = iom_data.perif_page_ar_data[ar_idx].input_cr->start_address;

     IOM_EXIT_IO();
}

/**
 * @brief   If no DMA job is parameterized for the specified CR, this function hands a locked user buffer over
 *          to PNIP for transmission. If, on the other hand, a DMA job was parameterized, it is triggered
 *          by this function - data is copied from the source buffer to the PERIF automatically.
 *
 * @param ar_idx    [in] specifies the AR whose input-CR-buffer is ready for transmission
 * @return          IOM_INVALID_BUFFER if the output CR is not initialized, IOM_BUFFER_UNLOCKED if the buffer
 *                  isn't in the locked state. IOM_OK_NEW_BUFFER if none of the above errors occured (the Perif can
 *                  always update the next provider buffer in contrast to the EDDI)
 *                  In DMA-mode, IOM_OK is always returned.
 */
IOM_ATTRIBUTE_ITCM LSA_UINT16 iom_perif_provider_unlock(LSA_UINT32 ar_idx)
{
    iom_perif_command_if_control_reg_t cmd_if_reg;
    LSA_UINT16 ret_val;

    IOM_ENTER_IO();

    IOM_ASSERT(ar_idx < IOM_CFG_PERIF_NUM_AR, LSA_NULL);
    IOM_ASSERT(iom_data.perif_page_ar_data[ar_idx].input_cr != LSA_NULL, IOM_INVALID_BUFFER);
    IOM_ASSERT(iom_data.perif_page_ar_data[ar_idx].input_cr->locked == LSA_TRUE, IOM_BUFFER_UNLOCKED);

#ifdef IOM_CFG_USE_GDMA
    if(GDMA_IS_VALID_JOB_IDX(iom_data.perif_page_ar_data[ar_idx].input_cr->dma_job))
    {
        gdma_trigger_job(iom_data.perif_page_ar_data[ar_idx].input_cr->dma_job);        /* data is copied to PERIF, ConsWR is performed by address guard */
        ret_val = IOM_OK;                                                               /* no distinction between new/old data because it's not that easy: time of buffer exchange is unknown in the context of DMA */
    }
    else
#endif
    {
        IOM_PERIF_COMMAND_IF_CONTROL__SET_FIELD(&cmd_if_reg, cr_number   , iom_data.perif_page_ar_data[ar_idx].input_cr->cr_index);
        IOM_PERIF_COMMAND_IF_CONTROL__SET_FIELD(&cmd_if_reg, conf_request, 0);
        IOM_PERIF_COMMAND_IF_CONTROL__SET_FIELD(&cmd_if_reg, f_code      , 2);              /* ConsWR -> user data is handed over to PNIP */
        IOM_PERIF_COMMAND_IF_CONTROL__SET_FIELD(&cmd_if_reg, user_id     , 0);

        *IOM_PERIF_COMMAND_IF_CONTROL = cmd_if_reg;

        ret_val = IOM_OK_NEW_BUFFER;
    }

    iom_data.perif_page_ar_data[ar_idx].input_cr->locked = 0;

    /* if the AR is about to be closed -> try to close it now */
    if(iom_data.perif_page_ar_data[ar_idx].free_request_issued)
    {
        if(!(iom_data.perif_page_ar_data[ar_idx].output_cr->locked || iom_data.perif_page_ar_data[ar_idx].input_cr->locked))
        {
            iom_perif_free_iocr_memory_local(ar_idx, IOM_PERIF_ALL_CRS);    /* release all resources of the AR */
            IOM_IOCR_MEMORY_FREED(ar_idx);                                  /* inform user about the completion of deallocation */
            iom_data.perif_page_ar_data[ar_idx].free_request_issued = LSA_FALSE;
        }
    }

    IOM_EXIT_RETURN_IO(ret_val);
}

#ifdef IOM_CFG_DMA_BUF_EXCHANGE
IOM_ATTRIBUTE_ITCM static iom_perif_command_if_control_reg_t iom_perif_dma_provider_exchange_cmd;
/**
 * @brief   This function creates a DMA job, that triggers only the provider buffer exchange. The address guard is not used
 *          and no data is copied in contrast to the DMA-parameterization opportunities that iom_perif_allocate_iocr_memory() offers.
 *
 * @return  LSA_TRUE on success, LSA_FALSE on error
 */
IOM_ATTRIBUTE_ITCM LSA_BOOL iom_perif_create_dma_provider_buf_exchange(LSA_UINT32 ar_idx, gdma_trigger_src_t dma_trigger_source, LSA_UINT8 priority)
{
    gdma_job_descriptor_t job_desc_input;
    gdma_transfer_descr_t transfer_desc_input[1];

    if(ar_idx >= IOM_CFG_PERIF_NUM_AR)
    {
        iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);
    }
    if(GDMA_IS_VALID_JOB(iom_data.perif_page_ar_data[ar_idx].input_cr->dma_job))
    {
        iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);    /* DMA already activated for this CR */
    }
    if(GDMA_IS_VALID_JOB(iom_data.perif_page_ar_data[ar_idx].input_cr->dma_job_buf_exchange))
    {
        iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, 0);    /* DMA already activated for this CR */
    }

    iom_perif_dma_provider_exchange_cmd.cr_number = iom_data.perif_page_ar_data[ar_idx].input_cr->cr_index;
    iom_perif_dma_provider_exchange_cmd.conf_request = 0;
    iom_perif_dma_provider_exchange_cmd.f_code = 2;              /* ConsWR -> user data is handed over to PNIP */
    iom_perif_dma_provider_exchange_cmd.user_id = 0;

    GDMA_SET_TRANSFER_DESCR(transfer_desc_input, 0,
                            &iom_perif_dma_provider_exchange_cmd,
                            IOM_PERIF_COMMAND_IF_CONTROL,
                            1,
                            GDMA_ESIZE_32BIT,
                            GDMA_BURST_MODE_SINGLE);

    job_desc_input.num_transfers = 1;
    job_desc_input.job_done_callback = LSA_NULL;
    job_desc_input.priority = priority;
    job_desc_input.trigger = (gdma_trigger_src_t) dma_trigger_source;
    job_desc_input.hw_flow_src = GDMA_HW_FLOW_OFF;
    job_desc_input.transfers = transfer_desc_input;
    iom_data.perif_page_ar_data[ar_idx].input_cr->dma_job_buf_exchange = gdma_add_job(&job_desc_input);
    if(!GDMA_IS_VALID_JOB(iom_data.perif_page_ar_data[ar_idx].input_cr->dma_job_buf_exchange))
    {
        return LSA_FALSE;
    }

    return LSA_TRUE;
}


/**
 * @brief   This function returns the consumer buffer address of an AR
 *
 * @param ar_idx    AR for which the output CR buffer address shall be returned
 * @return          LSA_NULL is returned in case of an unused AR/CR, otherwise the buffer address is returned
 */
IOM_ATTRIBUTE_ITCM LSA_UINT32* iom_perif_get_consumer_addr(LSA_UINT8 ar_idx)
{
    IOM_ENTER();

    IOM_ASSERT(ar_idx < IOM_CFG_PERIF_NUM_AR, LSA_NULL);
    IOM_ASSERT(iom_data.perif_page_ar_data[ar_idx].output_cr != LSA_NULL, LSA_NULL);

    IOM_EXIT_RETURN(iom_data.perif_page_ar_data[ar_idx].output_cr->start_address);
}

/**
 * @brief   This function returns the provider buffer address of an AR
 *
 * @param ar_idx    AR for which the input CR buffer address shall be returned
 * @return          LSA_NULL is returned in case of an unused AR/CR, otherwise the buffer address is returned
 */
IOM_ATTRIBUTE_ITCM LSA_UINT32* iom_perif_get_provider_addr(LSA_UINT8 ar_idx)
{
    IOM_ENTER();

    IOM_ASSERT(ar_idx < IOM_CFG_PERIF_NUM_AR, LSA_NULL);
    IOM_ASSERT(iom_data.perif_page_ar_data[ar_idx].input_cr != LSA_NULL, LSA_NULL);

    IOM_EXIT_RETURN(iom_data.perif_page_ar_data[ar_idx].input_cr->start_address);
}
#endif

#ifdef IOM_CFG_LOCATE_INSTRUCTIONS_TO_TCM
    #if defined (TOOL_CHAIN_GREENHILLS_ARM)
        #pragma ghs section text=IOM_DEFAULT_TEXT_SECTION_NAME
    #elif defined (TOOL_CHAIN_GNU_PPC)
        /* nothing needs to be done here */
    #else
        /* nothing needs to be done here */
    #endif
#endif

#ifdef IOM_SYSTEM_REDUNDANCY_SUPPORT
/**
 * @brief This function sets the currently primary AR.
 *
 * The primary AR is set by the application after a backup->primary edge was detected in the APDU status.
 * IOM herewith gets informed about the newly primary AR and later gets asked by CM about the primary AR's session key
 */
LSA_VOID iom_perif_set_session_key_primary_arid  (LSA_UINT32 ar_idx)
{
    switch(ar_idx)
    {
        case 0xFF:
        {
            iom_data.session_key_prim_ar = 0;
            break;
        }
        default:
        {
            iom_data.session_key_prim_ar = iom_data.perif_page_ar_data[ar_idx].prov_session_key;
        }
    }
}

/**
 *  @brief This function tells the session key of the primary AR.
 *
  * The primary AR is set by the application after a backup->primary edge was detected in the APDU status.
  * IOM gets informed about the change in primary AR by iom_perif_set_session_key_primary_arid().
  * This function is used by CM to retrieve the primary session key of the redundant AR-Set.
  */
LSA_UINT16 iom_perif_get_session_key_primary_arid  (LSA_UINT16 ARSetID)
{
    return iom_data.session_key_prim_ar;
}

/**
 * @brief This function sets the provider specific DataStatus (APDU). It's needed in the context
 * of system redundancy to set the primary/backup flag (bits 0 and 1).
 *
 * @param   ar_idx  specifies the AR to be modified
 * @param   value   DataStatus value : The bits "State" (= Backup or Primary) and "Redundancy" (= non-redundant / redundant) can be changed.
 * @param   mask    Mask for status bits that must be changed
 */
LSA_UINT32 iom_perif_provider_set_data_state(LSA_UINT32 ar_idx, LSA_UINT8 value, LSA_UINT8 mask)
{
    if (ar_idx < IOM_CFG_PERIF_NUM_AR)
    {
        LSA_RESULT ret_val;
        if (iom_data.perif_page_ar_data[ar_idx].prov_id == IOM_INVALID_PROV_ID)
        {
            /* skip service eddp_SysRed_ProviderDataStatus_Set if ar did not allocate memory see TFS 742609: Defekt bei doppelt vergebenem PDEV bei redundanter AR */
            return (0);
        }
        ret_val = eddp_SysRed_ProviderDataStatus_Set(EDD_INTERFACE_ID_MIN - 1, 
                                                     iom_data.perif_page_ar_data[ar_idx].prov_id,
                                                     value, 
                                                     mask, 
                                                     (EDDP_SYS_HANDLE)&(iom_data.eddp_sys_handle));
        if (ret_val != EDD_STS_OK)
        {
            iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, ret_val);
        }

        ret_val = eddp_SysRed_ShM_Array_Set(EDD_INTERFACE_ID_MIN-1, 0, iom_data.session_key_prim_ar);
        if (ret_val != EDD_STS_OK )
        {
            iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, __LINE__, ret_val);
        }
    }

    return (0);
}

#endif

/**
 * @brief   This function returns the input- and output-CR buffer sizes (no rounding)
 *
 * @param ar_idx                           [in] AR for which the buffer sizes shall be returned
 * @param input_cr_buffer_size            [out] buffer size of the input CR
 * @param output_cr_buffer_size           [out] buffer size of the output CR
 * @param input_cr_buffer_size_allocated  [out] buffer size of the input CR (internally allocated buffer size)
 * @param output_cr_buffer_size_allocated [out] buffer size of the output CR (internally allocated buffer size)
 *
 * @return  LSA_TRUE if no problems occured; LSA_FALSE otherwise
 */
LSA_BOOL iom_perif_get_iocr_buffer_sizes(LSA_UINT32 ar_idx, LSA_UINT32 *input_cr_buffer_size,           LSA_UINT32 *output_cr_buffer_size,
                                                            LSA_UINT32 *input_cr_buffer_size_allocated, LSA_UINT32 *output_cr_buffer_size_allocated)
{
    IOM_ENTER();

    IOM_ASSERT(ar_idx < IOM_CFG_PERIF_NUM_AR, LSA_FALSE);
    IOM_ASSERT((input_cr_buffer_size != LSA_NULL) && (output_cr_buffer_size != LSA_NULL), LSA_FALSE);
    IOM_ASSERT((input_cr_buffer_size_allocated != LSA_NULL) && (output_cr_buffer_size_allocated != LSA_NULL), LSA_FALSE);

    if(iom_data.perif_page_ar_data[ar_idx].input_cr != LSA_NULL)
    {
        *input_cr_buffer_size = iom_data.perif_page_ar_data[ar_idx].input_cr->buffer_size_no_padding;
        *input_cr_buffer_size_allocated  = iom_data.perif_page_ar_data[ar_idx].input_cr->buffer_size;
    }
    else
    {
        /* ARs might come up without any CR attached to it (happens if CM detects send-clock problem (adv. stup) / IRDataUUID problem) */
        *input_cr_buffer_size = 0;
        *input_cr_buffer_size_allocated  = 0;
    }

    if(iom_data.perif_page_ar_data[ar_idx].output_cr != LSA_NULL)
    {
        *output_cr_buffer_size = iom_data.perif_page_ar_data[ar_idx].output_cr->buffer_size_no_padding;
        *output_cr_buffer_size_allocated = iom_data.perif_page_ar_data[ar_idx].output_cr->buffer_size;
    }
    else
    {
        *output_cr_buffer_size = 0;
        *output_cr_buffer_size_allocated = 0;
    }

    IOM_EXIT_RETURN(LSA_TRUE);
}

static LSA_FATAL_ERROR_TYPE iom_error_descriptor;

static LSA_VOID iom_perif_in_fatal_error(LSA_UINT8 error_module, LSA_UINT16 error_line, LSA_UINT32 error_code)
{
    iom_error_descriptor.lsa_component_id = PNIO_PACKID_IOM;

    iom_error_descriptor.module_id        = error_module;
    iom_error_descriptor.line             = error_line;

    iom_error_descriptor.error_code[0]    = error_code;
    iom_error_descriptor.error_code[1]    =
    iom_error_descriptor.error_code[2]    =
    iom_error_descriptor.error_code[3]    = 0;

    iom_error_descriptor.error_data_length = sizeof(iom_data);
    iom_error_descriptor.error_data_ptr    = &iom_data;

    IOM_FATAL_ERROR(&iom_error_descriptor);
}

#endif // #ifdef IOM_CFG_PNIP

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
