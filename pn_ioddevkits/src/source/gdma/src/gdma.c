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
/*  F i l e               &F: gdma.c                                    :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  software module handling the ERTEC200+ / Triton2 GDMA module             */
/*                                                                           */
/*****************************************************************************/

#include "gdma_inc.h"

/*static*/ LSA_VOID  gdma_in_fatal_error( LSA_UINT8 error_module, LSA_UINT16 error_line, LSA_UINT16 error_code);
/*static*/ LSA_INT16 gdma_setup_transfer_records(LSA_UINT32 transfer_list_base_addr, LSA_UINT16 offset, LSA_UINT16 num_transfers, gdma_transfer_descr_t *transfers);

/* reservation of specific job numbers for hardware flow control based jobs is defined here */
#ifdef GDMA_KEEP_JOBS_AVAILABLE_FOR_HW_FLOW
static gdma_hw_flow_src_t gdma_keep_jobs_available_for_hw_flow[] = GDMA_KEEP_JOBS_AVAILABLE_FOR_HW_FLOW;
#endif

/**------------------------------------------------------------ */
/* internal data of GDMA module                                 */
/* ------------------------------------------------------------ */
typedef struct gdma_data_s
{
    LSA_BOOL   gdma_opened;                                                                 /**< flag indicates, if the GDMA has already been opened (used once). if not -> ISR is registered on first job_add() */

    LSA_UINT8  gdma_semaphore;                                                              /**< this counter prevents multiple calls of an interrupt lock macro */

    LSA_UINT8  gdma_active_job_pool_idx;                                                    /**< stores the index of the currently used transfer list (the module supports multiple transfer/job-lists that can be switched) */

    LSA_UINT32 gdma_used_jobs_map[GDMA_CFG_NUM_JOB_LISTS][(GDMA_MAX_JOBS + 31) /32];        /**< bit field indicating used jobs (bit == 1 -> job used */
    LSA_UINT32 gdma_software_triggered_jobs_map[GDMA_CFG_NUM_JOB_LISTS][(GDMA_MAX_JOBS+31)/32];     /**< bit field indicating all jobs that are triggered by software */
    LSA_UINT32 gdma_disabled_jobs_map[GDMA_CFG_NUM_JOB_LISTS][(GDMA_MAX_JOBS+31)/32];       /**< bit field indicating jobs that may not be triggered (by software / hardware) */
    LSA_UINT32 gdma_transfer_list_memory_map[GDMA_CFG_NUM_JOB_LISTS][(GDMA_MAX_TRANSFERS+31)/32];   /**< bit field indicating used transfer list entries (bit==1 -> entry used */
    LSA_VOID (*gdma_job_done_callbacks[GDMA_CFG_NUM_JOB_LISTS][GDMA_MAX_JOBS])(LSA_INT16, LSA_UINT32);   /**< job done callbacks for all jobs. parameters are: job-handle, user defined additional callback argument */
    LSA_UINT32 gdma_callback_args[GDMA_CFG_NUM_JOB_LISTS][GDMA_MAX_JOBS];                   /**< user defined 32 bit argument, that is passed to the corresponding job callback. */

    gdma_jobx_ctrl_reg_t gdma_job_control_reg_set[GDMA_CFG_NUM_JOB_LISTS][GDMA_MAX_JOBS];   /**< stores the job control registers of the different job pools */
    #if GDMA_CFG_NUM_JOB_LISTS > 1
    struct
    {
        LSA_UINT32  additional_transfer_lists_data[GDMA_MAX_TRANSFERS * 4 + 4];             /**< first transfer list is located in GDMA RAM, further lists in SDRAM / DTCM. SDRAM lists must be 16 byte aligned! */
        LSA_UINT8   stack[GDMA_STACK_SIZE];                                                 /**< behind the transfer list, there is always the GDMA stack: reserve memory for it ... */
    }gdma_ext_mem[GDMA_CFG_NUM_JOB_LISTS-1];
    #endif
}gdma_data_t;

/* static module data isn't put into DTCM because the GDMA may reach it using a different address range compared to the processor -> address
 * conversion would be necessary */
static gdma_data_t gdma_data;


#ifdef GDMA_CFG_LOCATE_INSTRUCTIONS_TO_TCM
    #if defined (TOOL_CHAIN_GREENHILLS_ARM)
        #pragma ghs section text=GDMA_ITCM_SECTION_NAME
        #define GDMA_ATTRIBUTE_ITCM
    #elif defined (TOOL_CHAIN_GNU_PPC)
        #define GDMA_ATTRIBUTE_ITCM __attribute__ ((section (GDMA_ITCM_SECTION_NAME)))
    #else
        #define GDMA_ATTRIBUTE_ITCM __attribute__ ((section (GDMA_ITCM_SECTION_NAME)))
    #endif
#else
    #define GDMA_ATTRIBUTE_ITCM
#endif

/**
 * @brief   This interrupt service routine is executed after a DMA job finished.
 *          A user callback belonging to this job gets called.
 *          Attention for the programmer: gdma_isr() might also be called from gdma_switch_to_job_pool() / from a CBF, if
 *          a pool switch is performed in interrupt context! -> the function needs reentrant behavior !
 */
GDMA_ATTRIBUTE_ITCM static LSA_VOID gdma_isr()
{
    LSA_UINT8   irq_source;

    while(*GDMA_IRQ_STATUS)
    {
        irq_source = 0;

        while(irq_source < 32)
        {
            /* search the interrupt source */
            while(((*GDMA_IRQ_STATUS & (1 << irq_source)) == 0) && (irq_source < 32))
            {
                irq_source++;
            }

            if(irq_source < 32)
            {
                if((*GDMA_IRQ_STATUS & (1 << irq_source)) != 0)
                {
                    /* acknowledge IRQ. Do this before calling the callback, because the callback might trigger the
                     * job again -> we could loose one interrupt, if we acknowledged it afterwards */
                    *GDMA_IRQ_STATUS = (1 << irq_source);

                    if(gdma_data.gdma_job_done_callbacks[gdma_data.gdma_active_job_pool_idx][irq_source])
                    {
                        /* call the registered IRQ callback for this job.
                         * Argument 1 : job handle
                         * Argument 2 : additional user defined 32 bit data
                         * */
                        gdma_data.gdma_job_done_callbacks[gdma_data.gdma_active_job_pool_idx][irq_source](gdma_data.gdma_active_job_pool_idx * GDMA_MAX_JOBS + irq_source, gdma_data.gdma_callback_args[gdma_data.gdma_active_job_pool_idx][irq_source]);
                    }
                }
                irq_source++;
            }
        }
    }
}

/**
 * @brief   This function checks, if the job <job_num> is currently allocated (in any pool,
 *          no matter if its enabled or disabled)
 * @param   job_num     specifies the job to be checked (handle was returned by gdma_add_job())
 * @return  LSA_TRUE if the job is known to the GDMA manager, LSA_FALSE if it doesn't exist
 */
GDMA_ATTRIBUTE_ITCM LSA_BOOL gdma_is_job_allocated(LSA_INT16 job_num)
{
    LSA_UINT16 job_pool_index = job_num / GDMA_MAX_JOBS;

    job_num = job_num % GDMA_MAX_JOBS;

    if(job_pool_index >= GDMA_CFG_NUM_JOB_LISTS)
    {
        return LSA_FALSE;
    }

    if( (gdma_data.gdma_used_jobs_map[job_pool_index][job_num >> 5] & (1 << (job_num % 32))) != 0)
    {
        return LSA_TRUE;
    }

    return LSA_FALSE;
}

#ifdef GDMA_CFG_LOCATE_INSTRUCTIONS_TO_TCM
    #if defined (TOOL_CHAIN_GREENHILLS_ARM)
        #pragma ghs section text=GDMA_DEFAULT_TEXT_SECTION_NAME
    #elif defined (TOOL_CHAIN_GNU_PPC)
        /* nothing needs to be done here */
    #else
        /* nothing needs to be done here */
    #endif
#endif

/**
 * @brief   This function initializes the GDMA subsystem and module internal data. All DMA jobs
 *          are disabled. No external services are called from here (ISR registration for example).
 */
LSA_VOID gdma_init()
{
    gdma_main_ctrl_reg_t main_ctrl_reg;

    gdma_data.gdma_semaphore = 0;

    GDMA_ENTER();

    // GDMA Registers Base Address - points to the first address of the GDMA registers and serves as a
    // write protection (so that DMA cannot modify its own memory)
    *GDMA_REG_ADDR = (LSA_UINT32) GDMA_CONVERT_TO_TARGET_HW_ADDR(GDMA_REG_ADDR);

    // DMA Transfer List Base Address - Points to the first address of the Transfer List of the DMA RAM.
    *GDMA_LIST_ADDR = (LSA_UINT32) GDMA_CONVERT_TO_TARGET_HW_ADDR(GDMA_LIST_RAM_START);

    // GDMA Main Control Register
    main_ctrl_reg.bits.dma_en = 0;             // disable "global enable"
    main_ctrl_reg.bits.sw_reset = 0;           // disable reset signal
    main_ctrl_reg.bits.list_size = 255;        // total number of transfers in transfer list (0-255), 0 means 1
    main_ctrl_reg.bits.err_int_en = 0;         // disable error interrupt
    GDMA_MAIN_CTRL->reg =  main_ctrl_reg.reg;

    // disable job counter for all jobs
    *GDMA_JC_EN = 0;

    // init job control register
    // 32 jobs, one job = 1 register * 32bit
    // 32 * 1 * 4bytes = 128bytes --> 0x80
    GDMA_MEMSET((LSA_UINT8 *) GDMA_JOBX_CTRL(0), 0, GDMA_MAX_JOBS * 4);

    // reset finished jobs status register -> all jobs unfinished
    *GDMA_FINISHED_JOBS = 0xFFFFFFFF;

    // reset job finished irq -> all jobs unfinished
    *GDMA_IRQ_STATUS = 0xFFFFFFFF;

    // init transfer list ram
    // 256 transfer list entries, one entry = 4 registers * 32bit
    // 256 * 4 * 4byte = 4096byte --> 0x1000
    GDMA_MEMSET((LSA_UINT8*) GDMA_LIST_RAM_START, 0, GDMA_LIST_RAM_END - GDMA_LIST_RAM_START);
    // init job stack ram
    // 32 jobs, one job = 4 registers * 32bit
    // 32 * 4 * 4byte = 512byte --> 0x0200
    GDMA_MEMSET((LSA_UINT8*) GDMA_JOB_STACK_RAM_START, 0, GDMA_JOB_STACK_RAM_END - GDMA_JOB_STACK_RAM_START);

    /* reset DMA meta data */
    GDMA_MEMSET(gdma_data.gdma_used_jobs_map,                0, sizeof(gdma_data.gdma_used_jobs_map));
    GDMA_MEMSET(gdma_data.gdma_disabled_jobs_map,            0, sizeof(gdma_data.gdma_disabled_jobs_map));
    GDMA_MEMSET(gdma_data.gdma_transfer_list_memory_map,     0, sizeof(gdma_data.gdma_transfer_list_memory_map));
    GDMA_MEMSET(gdma_data.gdma_software_triggered_jobs_map,  0, sizeof(gdma_data.gdma_software_triggered_jobs_map));

#if GDMA_CFG_NUM_JOB_LISTS > 1
    GDMA_MEMSET(gdma_data.gdma_ext_mem, 0, sizeof(gdma_data.gdma_ext_mem));
#endif
    GDMA_MEMSET(gdma_data.gdma_job_done_callbacks, 0, sizeof(gdma_data.gdma_job_done_callbacks));
    GDMA_MEMSET(gdma_data.gdma_callback_args, 0, sizeof(gdma_data.gdma_callback_args));

    gdma_data.gdma_active_job_pool_idx = 0;

    gdma_data.gdma_opened = LSA_FALSE;

    GDMA_EXIT();
}

/**
 * @brief   This function sets up a new DMA job and its corresponding transfers.
 *
 * @param job_pool_index    Transfer lists can be created in different pools (number of pools is defined by
 *                  GDMA_CFG_NUM_JOB_LISTS). With this parameter you specify the job pool, the job to be added
 *                  is appended to. During operation, only one set of jobs (one pool) can be active at the same time.
 *                  range : 0 to GDMA_CFG_NUM_JOB_LISTS-1
 * @param job_desc  Pointer to a structure of type gdma_job_descriptor_t, describing the job and
 *                  all related transfers. The "transfers"-field of the structure points to a
 *                  descriptor of the job's transfers. It can be initialized via the macro
 *                  GDMA_SET_TRANSFER_DESCR.
 * @return          On success the job identifier (0-31) is returned. Otherwise GDMA_ERR_PARAMETER
 *                  or GDMA_ERR_MEM can be returned.
 */
LSA_INT16 gdma_add_job(LSA_UINT8 job_pool_index, gdma_job_descriptor_t* job_desc)
{
    LSA_UINT16              job_num;
    LSA_UINT16              offset, offset_end;
    gdma_jobx_ctrl_reg_t    job_reg;
    LSA_UINT32              transfer_list_base_addr = 0;
    LSA_UINT32              job_control_regs_base_addr;

    GDMA_ENTER();

    if(!gdma_data.gdma_opened)
    {
        gdma_data.gdma_opened = LSA_TRUE;

        /* register ISR */
        GDMA_SET_INT_HANDLER(gdma_isr);
    }

            /* first parameter check */
    GDMA_ASSERT(job_pool_index < GDMA_CFG_NUM_JOB_LISTS, GDMA_ERR_PARAMETER);   /* error case : job_pool_index is out of range */
    GDMA_ASSERT(job_desc != LSA_NULL, GDMA_ERR_PARAMETER);                      /* error case : job_desc is not a valid pointer */
    GDMA_ASSERT(job_desc->priority < 32, GDMA_ERR_PARAMETER);                   /* error case : priority is not in the range 0 - 31 */

    GDMA_ASSERT(job_desc->num_transfers > 0, GDMA_ERR_PARAMETER);               /* error case : a new job needs at least 1 transfer */
    GDMA_ASSERT(((LSA_INT8)job_desc->trigger >= 0) && ((LSA_INT8)job_desc->trigger <= GDMA_TRIGGER_SW), GDMA_ERR_PARAMETER);   /*lint !e568 non-negative quantity is never less than zero */
                                                                                /* error case : trigger source is not valid */

            /* determine base address of the transfer list and job control register to be used */
    if(job_pool_index == 0)
    {
        transfer_list_base_addr = (LSA_UINT32) GDMA_LIST_RAM_START;      /* the first transfer list is located in the GDMA RAM */
    }
    #if GDMA_CFG_NUM_JOB_LISTS > 1
    else
    {
        if( (job_pool_index-1) < (GDMA_CFG_NUM_JOB_LISTS-1) )
        {
            /* further transfer lists are located in SDRAM / DTCM. These lists must be 16 byte aligned ! */
            transfer_list_base_addr = (((LSA_UINT32) gdma_data.gdma_ext_mem[job_pool_index-1].additional_transfer_lists_data + 15) & 0xFFFFFFF0);
        }
    }
    #endif
    if(job_pool_index == gdma_data.gdma_active_job_pool_idx)
    {
        job_control_regs_base_addr = (LSA_UINT32) GDMA_JOBX_CTRL(0);
    }
    else
    {
        job_control_regs_base_addr = (LSA_UINT32) gdma_data.gdma_job_control_reg_set[job_pool_index];
    }


    /*
     * find an empty job (job control register)
     */
    if(job_desc->hw_flow_src == GDMA_HW_FLOW_OFF)
    {
        /* hw_flow isn't needed -> we can use any job number, that is not reserved for a future hw_flow dependent task */
        for(job_num = 0; job_num < GDMA_MAX_JOBS; job_num++)
        {
            if((gdma_data.gdma_used_jobs_map[job_pool_index][job_num >> 5] & (1 << (job_num % 32))) == 0)
            {
                LSA_BOOL reserved_for_hw_flow = LSA_FALSE;
                #ifdef GDMA_KEEP_JOBS_AVAILABLE_FOR_HW_FLOW
                LSA_UINT8 i;

                /* job resource still free -> check if it's configured as reserved for a specific hardware-related task
                 * (a task needs this specific job number to realize hardware flow control) */
                for(i = 0; i < sizeof(gdma_keep_jobs_available_for_hw_flow) / sizeof(gdma_hw_flow_src_t); i++)
                {
                    if(job_num == gdma_keep_jobs_available_for_hw_flow[i])
                    {
                        reserved_for_hw_flow = LSA_TRUE;
                        break;
                    }
                }
                #endif

                if(reserved_for_hw_flow == LSA_FALSE)
                {
                    break;
                }
            }
        }

        GDMA_ASSERT(job_num < GDMA_MAX_JOBS, GDMA_ERR_MEM);         /* error case : no free job was found */
    }
    else
    {
        /* hw-flow enabled -> a specific job number is required -> check if it's available */
        job_num = job_desc->hw_flow_src;
        GDMA_ASSERT(job_num < GDMA_HW_FLOW_OFF, GDMA_ERR_PARAMETER);   /* error case : the hardware flow control source ID is wrong. It must be assignable to a valid job number */
        GDMA_ASSERT((gdma_data.gdma_used_jobs_map[job_pool_index][job_num >> 5] & (1 << (job_num % 32))) == 0, GDMA_ERR_MEM); /* error case : the job that is needed for HW flow control is already initialized */
    }


    /*
     * find enough contiguous memory for the transfer lists
     */
    for(offset = 0; offset < GDMA_MAX_TRANSFERS; offset++)
    {
        // check if the block of transfer records, beginning at "offset", is suitable
        for(offset_end = offset; (offset_end < offset + job_desc->num_transfers) && (offset_end < GDMA_MAX_TRANSFERS); offset_end++)
        {
            /* is the transfer record already used? -> if yes -> continue the search after the transfer record */
            if((gdma_data.gdma_transfer_list_memory_map[job_pool_index][offset_end >> 5] & (1 << (offset_end % 32))) != 0)
            {
                offset = offset_end;
                break;
            }
        }

        if(offset_end >= offset + job_desc->num_transfers)
        {
            // a memory block that is large enough has been found -> exit loop
            break;
        }
    }

    GDMA_ASSERT(offset < GDMA_MAX_TRANSFERS, GDMA_ERR_MEM);     /* error case : not enough free memory for the transfer list entry was found */

    /* ------------------------------------------------------------------------------------------ */
    /* setup of transfer records in the transfer list                                             */
    /* ------------------------------------------------------------------------------------------ */
    if(gdma_setup_transfer_records(transfer_list_base_addr, offset, job_desc->num_transfers, job_desc->transfers) != GDMA_OK)
    {
        return GDMA_ERR_PARAMETER;
    }

    /* ------------------------------------------------------------------------------------------ */
    /* don't return with an error from here on -> now the valid parameterization is applied */
    /* ------------------------------------------------------------------------------------------ */

    /* WRITE METADATA */
    /* mark register job index (->job control register) and transfer list entries as occupied in corresponding meta data */
    {
        LSA_UINT16 i = offset;
        while(i < offset_end)
        {
            gdma_data.gdma_transfer_list_memory_map[job_pool_index][i >> 5] |= (1 << (i % 32));
            i++;
        }

        gdma_data.gdma_used_jobs_map[job_pool_index][job_num / 32]      |= (1 << (job_num % 32));   /* job is valid / parameterized */
        gdma_data.gdma_disabled_jobs_map[job_pool_index][job_num / 32]  |= (1 << (job_num % 32));   /* job may not be triggered yet */

        gdma_data.gdma_job_done_callbacks[job_pool_index][job_num]      = job_desc->job_done_callback;
        gdma_data.gdma_callback_args[job_pool_index][job_num]           = job_desc->cbf_arg;
        if((job_desc->job_done_callback) && (job_pool_index == gdma_data.gdma_active_job_pool_idx))
        {
            /* acknowledge IRQ */
            *GDMA_IRQ_STATUS = (1 << job_num);
        }
    }

    /*
     * set job control register for this job
     */
    job_reg.bits.transfer_ptr = offset;                          /*  first transfer number of job (offset in transfer list, "transfer_ptr") */

    job_reg.bits.job_prio = job_desc->priority;

    job_reg.bits.job_reset = 0;                                  /* job reset (must remain disabled) */

    if(job_desc->job_done_callback != LSA_NULL)             /* interrupt request generation enable (intr_en) */
    {
        job_reg.bits.intr_en = 1;
    }
    else
    {
        job_reg.bits.intr_en = 0;
    }
    if(job_desc->hw_flow_src == GDMA_HW_FLOW_OFF)
    {
        job_reg.bits.hw_flow_en = 0;                              /* hardware triggered flow enable (hw_flow_en) = off */
    }
    else
    {
        job_reg.bits.hw_flow_en = 1;
    }

    if(job_desc->trigger == GDMA_TRIGGER_SW)
    {
        gdma_data.gdma_software_triggered_jobs_map[job_pool_index][job_num / 32] |= (1UL << (job_num % 32));
    }
    else
    {
        gdma_data.gdma_software_triggered_jobs_map[job_pool_index][job_num / 32] &= ~(1UL << (job_num % 32));
        job_reg.bits.hw_select = job_desc->trigger;              /* hw job start selector */
    }
    job_reg.bits.hw_job_start_en = 0;                            /* hw job start enable : is set in an enable_job() - function according to the hw_select selection */

    job_reg.bits.job_en = 1;                                     /* enable job (must be set to 1) */
    job_reg.bits.sw_job_start = 0;                               /* start job by software (sw_job_start). Must remain disabled here */

    GDMA_JOBX_CTRL_VAR_BASE_ADDR(job_control_regs_base_addr, job_num)->reg = job_reg.reg;

    GDMA_EXIT_RETURN((LSA_INT16)(job_num + (job_pool_index * GDMA_MAX_JOBS)));       /* first job set : IDs 0-31, second job set 32-63, ... */
}

/**
 * This function enables the user to add transfers to an existing job
 * (allocated with gdma_add_job() before). The transfers are appended
 * (-> this works only if no other job reserved the transfer list memory there already)
 *
 * @param job_num           Specifies the job, that is to be extended with several new transfers. (= return value of gdma_add_job())
 * @param add_transfer_desc structure that describes the additional transfers
 *
 * @return  GDMA_OK is returned on success. In case of a parameterization error, GDMA_ERR_PARAMETER
 *          is returned. GDMA_ERR_MEM is given back, if the transfer list memory doesn't provide enough
 *          space anymore.
 */
LSA_INT16 gdma_add_transfers_to_job(LSA_INT16 job_num_global, gdma_add_transfers_descriptor_t *add_transfers_desc)
{
    LSA_UINT16              offset;
    LSA_UINT16              offset_end;
    LSA_UINT16              i;
    LSA_UINT32              transfer_list_base_addr = 0;
    LSA_UINT32              job_control_regs_base_addr;
    gdma_jobx_ctrl_reg_t    job_reg;
    LSA_UINT16              job_pool_index = (LSA_UINT16)(job_num_global / GDMA_MAX_JOBS);
    LSA_INT16               job_num;
    LSA_BOOL                is_job_active = LSA_FALSE;

    job_num = job_num_global % GDMA_MAX_JOBS;

    GDMA_ENTER();

    GDMA_ASSERT(add_transfers_desc != LSA_NULL, GDMA_ERR_PARAMETER);
    GDMA_ASSERT(job_pool_index < GDMA_CFG_NUM_JOB_LISTS, GDMA_ERR_PARAMETER);                                                      /* error case: job_num out of range */
    GDMA_ASSERT( (gdma_data.gdma_used_jobs_map[job_pool_index][job_num >> 5] & (1 << (job_num % 32))) != 0, GDMA_ERR_PARAMETER );  /* error case: the job to be extended is not initialized */
    if(add_transfers_desc->num_transfers == 0)
    {
        GDMA_EXIT_RETURN(GDMA_OK);
    }

    /* determine base address of the transfer list and job control register to be used */
    if(job_pool_index == 0)
    {
        transfer_list_base_addr = (LSA_UINT32) GDMA_LIST_RAM_START;      /* the first transfer list is located in the GDMA RAM */
    }
#if GDMA_CFG_NUM_JOB_LISTS > 1
    else
    {
        if( (job_pool_index-1) < (GDMA_CFG_NUM_JOB_LISTS-1) )
        {
            /* further transfer lists are located in SDRAM / DTCM. They have to be 16 byte aligned! */
            transfer_list_base_addr = (((LSA_UINT32) gdma_data.gdma_ext_mem[job_pool_index-1].additional_transfer_lists_data + 15) & 0xFFFFFFF0);
        }
    }
#endif
    if(job_pool_index == gdma_data.gdma_active_job_pool_idx)
    {
        /* extend a currently active job (-> the job control registers lie in GDMA memory) */
                /* make sure, that the job is not running right now */
        LSA_INT16 ret_val;

        if((gdma_data.gdma_disabled_jobs_map[job_pool_index][job_num / 32] & (1 << (job_num % 32))) == 0)
        {
            is_job_active = LSA_TRUE;   /* remember, that the job was active -> activate it again at the end of the function */
        }

        ret_val = gdma_disable_job(job_num_global);
        if(ret_val != GDMA_OK)
        {
            GDMA_EXIT_RETURN(ret_val);
        }

        job_control_regs_base_addr = (LSA_UINT32) GDMA_JOBX_CTRL(0);
        job_reg.reg = GDMA_JOBX_CTRL_VAR_BASE_ADDR(job_control_regs_base_addr, job_num)->reg;
    }
    else
    {
        job_control_regs_base_addr = (LSA_UINT32) gdma_data.gdma_job_control_reg_set[job_pool_index];
        job_reg.reg = GDMA_JOBX_CTRL_VAR_BASE_ADDR(job_control_regs_base_addr, job_num)->reg;
    }

    /* check if there is enough memory left in the transfer list */
            /* find the end of the current transfer list */
    offset = (PNIO_UINT16)job_reg.bits.transfer_ptr;
    while(GDMA_TRANS_RECX_CNT(transfer_list_base_addr, offset)->bits.last_tr == 0)
    {
        offset++;
    }
    for(offset_end = offset + 1; (offset_end < offset + 1 + add_transfers_desc->num_transfers) && (offset_end < GDMA_MAX_TRANSFERS); offset_end++)
    {
        /* is the transfer record already used? -> if yes -> we can't allocate the new transfers directly behind
         * the old transfer list */
        if((gdma_data.gdma_transfer_list_memory_map[job_pool_index][offset_end >> 5] & (1 << (offset_end % 32))) != 0)
        {
            break;
        }
    }
    if(offset_end - (offset+1) != add_transfers_desc->num_transfers)
    {
        /* the necessary amount of memory is not available */
        GDMA_EXIT_RETURN(GDMA_ERR_MEM);
    }

    /* ----------------------------- */
    /* add the transfer list entries */
    /* ----------------------------- */
            /* remove the "last transfer-bit" of the last transfer */
    {
        gdma_trans_recx_cnt_reg_t   transfer_count_reg;
        transfer_count_reg.reg = GDMA_TRANS_RECX_CNT(transfer_list_base_addr, offset)->reg;
        transfer_count_reg.bits.last_tr = 0;
        GDMA_TRANS_RECX_CNT(transfer_list_base_addr, offset)->reg = transfer_count_reg.reg;
    }

            /* setup the new transfer records */
    gdma_setup_transfer_records(transfer_list_base_addr, offset + 1, add_transfers_desc->num_transfers, add_transfers_desc->transfers);

            /* mark the used transfer list entries as used */
    i = offset + 1;
    while(i < offset_end)
    {
        gdma_data.gdma_transfer_list_memory_map[job_pool_index][i >> 5] |= (1 << (i % 32));
        i++;
    }

    /*
     * reactivate the job (if it was active before and is in the currently active GDMA pool)
     */
    if(job_pool_index == gdma_data.gdma_active_job_pool_idx)
    {
        if(is_job_active)
        {
            /* the job was active before -> enable it again */
            gdma_enable_job(job_num_global);
        }
    }

    GDMA_EXIT_RETURN(GDMA_OK);
}

/**
 * @brief   This function removes a job that has been installed previously using gdma_add_job().
 *
 * @param job_num   Identifies the job to be removed. It equals the return value of gdma_add_job() in case of a
 *                  successfull allocation.
 * @return          GDMA_OK is returned on success. In case of a parameterization error GDMA_ERR_PARAMETER
 *                  is returned.
 */
LSA_INT16 gdma_remove_job(LSA_INT16 job_num)
{
    gdma_jobx_ctrl_reg_t        job_reg;
    gdma_trans_recx_cnt_reg_t   transfer_list_count_field;
    LSA_UINT32                  transfer_list_base_addr = 0;
    LSA_UINT32                  job_control_regs_base_addr;
    LSA_UINT16                  offset;
    LSA_UINT16                  transfer_idx;
    LSA_UINT16                  job_pool_index = (LSA_UINT16)(job_num / GDMA_MAX_JOBS);

    job_num = job_num % GDMA_MAX_JOBS;

    GDMA_ENTER();

    GDMA_ASSERT(job_pool_index < GDMA_CFG_NUM_JOB_LISTS, GDMA_ERR_PARAMETER);                                                      /* error case: job_num out of range */
    GDMA_ASSERT((gdma_data.gdma_used_jobs_map[job_pool_index][job_num >> 5] & (1 << (job_num % 32))) != 0, GDMA_ERR_PARAMETER);    /* error case: the job to be removed is not initialized */

    /* determine base address of the transfer list and job control register to be used */
    if(job_pool_index == 0)
    {
        transfer_list_base_addr = (LSA_UINT32) GDMA_LIST_RAM_START;      /* the first transfer list is located in the GDMA RAM */
    }
    #if GDMA_CFG_NUM_JOB_LISTS > 1
    else
    {
        if( (job_pool_index-1) < (GDMA_CFG_NUM_JOB_LISTS-1) )
        { 
            /* further transfer lists are located in SDRAM / DTCM. They have to be 16 byte aligned! */
            transfer_list_base_addr = (((LSA_UINT32) gdma_data.gdma_ext_mem[job_pool_index-1].additional_transfer_lists_data + 15) & 0xFFFFFFF0);
        }
    }
    #endif
    if(job_pool_index == gdma_data.gdma_active_job_pool_idx)
    {
            /* remove a currently active job */
        job_control_regs_base_addr = (LSA_UINT32) GDMA_JOBX_CTRL(0);
        job_reg.reg = GDMA_JOBX_CTRL_VAR_BASE_ADDR(job_control_regs_base_addr, job_num)->reg;

        /* prevent further triggering of the job to be removed -> avoid an interruption of a running transfer */
        job_reg.bits.hw_job_start_en = 0;
        GDMA_JOBX_CTRL_VAR_BASE_ADDR(job_control_regs_base_addr, job_num)->reg = job_reg.reg;       /* job may not be triggered by hardware anymore */
        gdma_data.gdma_disabled_jobs_map[job_pool_index][job_num / 32]  |= (1 << (job_num % 32));   /* job may not be triggered by software anymore */

        /* wait until the DMA job is no longer busy */
        while(GDMA_ACTUAL_STATUS->bits.act_job_val == 1 && GDMA_ACTUAL_STATUS->bits.act_job == job_num)
            ;
    }
    else
    {
            /* remove a job of the inactive job pool */
        job_control_regs_base_addr = (LSA_UINT32) gdma_data.gdma_job_control_reg_set[job_pool_index];
        job_reg.reg = GDMA_JOBX_CTRL_VAR_BASE_ADDR(job_control_regs_base_addr, job_num)->reg;
    }

    /* disable the job */
    offset = (PNIO_UINT16)job_reg.bits.transfer_ptr;

    job_reg.bits.job_en = 0;
    job_reg.bits.job_reset = 1;
    job_reg.bits.hw_select = 0;
    job_reg.bits.intr_en = 0;
    job_reg.bits.transfer_ptr = 0;
    GDMA_JOBX_CTRL_VAR_BASE_ADDR(job_control_regs_base_addr, job_num)->reg = job_reg.reg;

    /* remove transfer list entries and free allocated resources in the transfer list memory map */
    {
        gdma_trans_recx_cnt_reg_t count_field_zero;
        GDMA_MEMSET(&count_field_zero, 0, sizeof(gdma_trans_recx_cnt_reg_t));

        transfer_idx = 0;
        do
        {
            transfer_list_count_field.reg = GDMA_TRANS_RECX_CNT(transfer_list_base_addr, offset + transfer_idx)->reg;

            *GDMA_TRANS_RECX_SRC(transfer_list_base_addr, offset + transfer_idx) = 0;
            *GDMA_TRANS_RECX_DST(transfer_list_base_addr, offset + transfer_idx) = 0;
            *GDMA_TRANS_RECX_CTRL(transfer_list_base_addr, offset + transfer_idx) = 0;
            GDMA_TRANS_RECX_CNT(transfer_list_base_addr, offset + transfer_idx)->reg = count_field_zero.reg;

            gdma_data.gdma_transfer_list_memory_map[job_pool_index][(offset + transfer_idx) >> 5] &= ~(1UL << ((offset + transfer_idx) % 32));

            transfer_idx++;
        }while((transfer_list_count_field.bits.last_tr == 0) && (offset + transfer_idx < GDMA_MAX_TRANSFERS));

        GDMA_ASSERT(transfer_list_count_field.bits.last_tr == 1, GDMA_ERR_INTERNAL);                             /* error case: transfer list corrupted */

        gdma_data.gdma_job_done_callbacks[job_pool_index][job_num] = LSA_NULL;
        gdma_data.gdma_callback_args[job_pool_index][job_num]      = 0;
        gdma_data.gdma_used_jobs_map[job_pool_index][job_num >> 5] &= ~(1UL << (job_num % 32));
    }

    GDMA_EXIT_RETURN(GDMA_OK);
}

/**
 * @brief   This function sets the job pool <job_pool_index> as the current set of jobs. This way
 *          you can dynamically change between two transfer lists and job lists during operation. Beforehand
 *          all the current jobs are disabled and currently running jobs are not interrupted. After the switch
 *          the user has to enable all jobs.
 *
 * @param   job_pool_index  "pool id" of the job set that is to be set as the current pool
 * @return                  GDMA_OK if pool switch can be performed without error, GDMA_ERR_PARAMETER if the parameter is wrong.
 */
LSA_INT16 gdma_switch_to_job_pool(LSA_UINT8 job_pool_index)
{
    GDMA_ENTER();

    GDMA_ASSERT(job_pool_index < GDMA_CFG_NUM_JOB_LISTS, GDMA_ERR_PARAMETER);

    /* disable all jobs. Afterwards no DMA job is running / can be triggered */
    gdma_disable_all_jobs();


    if(*GDMA_IRQ_STATUS)
    {
        /* there's an interrupt pending ... */

        /* use cases:                                                                                   */
        /* 1. while executing an ISR CBF, gdma_switch_to_job_pool() was called -> all ISR CBFs          */
        /*    of the old job pool have to be called before we switch the pool                           */
        /*    -> we are in interrupt context here and need our reentrant gdma_isr() - routine           */
        /* 2. when we are in task context here, the interrupt must be currently blocked by GDMA_ENTER() */
        /*    -> it is save to enter gdma_isr() now -> and serve all pending interrupts                 */
        gdma_isr();
    }

    /* copy the current job control registers to the database in RAM */
    GDMA_COPY_BYTE(gdma_data.gdma_job_control_reg_set[gdma_data.gdma_active_job_pool_idx], (LSA_VOID*) GDMA_JOBX_CTRL(0), GDMA_MAX_JOBS * sizeof(gdma_jobx_ctrl_reg_t));

    /* copy the new set of job control registers to the hardware registers */
    GDMA_COPY_BYTE((LSA_VOID*)GDMA_JOBX_CTRL(0), &gdma_data.gdma_job_control_reg_set[job_pool_index][0], GDMA_MAX_JOBS * sizeof(gdma_jobx_ctrl_reg_t));

    /* set the transfer list pointer to the new transfer list */
    if(job_pool_index == 0)
    {
        *GDMA_LIST_ADDR = (LSA_UINT32) GDMA_LIST_RAM_START;
    }
    #if GDMA_CFG_NUM_JOB_LISTS > 1
    else
    {
        if( (job_pool_index-1) < (GDMA_CFG_NUM_JOB_LISTS-1) )
        {
            /* 16 byte aligned transfer list in SDRAM */
            *GDMA_LIST_ADDR = ((LSA_UINT32) gdma_data.gdma_ext_mem[job_pool_index-1].additional_transfer_lists_data + 15) & 0xFFFFFFF0;
        }
    }
    #endif

    gdma_data.gdma_active_job_pool_idx = job_pool_index;

    GDMA_EXIT_RETURN(GDMA_OK);
}

/**
 * @brief   This function activates a single deactivated job. The job can be triggered via hardware or software afterwards.
 *
 * @param   job_num     job number of the job to be activated
 * @return  GDMA_OK if the job can be activated, GDMA_ERR_PARAMETER if the passed parameter is wrong
 */
LSA_INT16 gdma_enable_job(LSA_INT16 job_num)
{
    LSA_UINT16 job_pool_index = (LSA_UINT16)(job_num / GDMA_MAX_JOBS);

    job_num = job_num % GDMA_MAX_JOBS;

    GDMA_ENTER();

    GDMA_ASSERT(job_pool_index < GDMA_CFG_NUM_JOB_LISTS, GDMA_ERR_PARAMETER);                                                   /* error case : job_num is out of range */
    GDMA_ASSERT(job_pool_index == gdma_data.gdma_active_job_pool_idx, GDMA_ERR_PARAMETER);                                      /* error case : only jobs of the current job set can be activated */
    GDMA_ASSERT((gdma_data.gdma_used_jobs_map[job_pool_index][job_num / 32] & (1 << (job_num % 32))) != 0, GDMA_ERR_PARAMETER);   /* error case : job is not initialized */

    if((gdma_data.gdma_software_triggered_jobs_map[job_pool_index][job_num / 32] & (1 << (job_num % 32))) == 0)
    {
        GDMA_JOBX_CTRL(job_num)->bits.hw_job_start_en = 1;     /* job may be triggered by hardware again */
    }
    gdma_data.gdma_disabled_jobs_map[gdma_data.gdma_active_job_pool_idx][job_num / 32]  &= ~(1UL << (job_num % 32));  /* job may be triggered by software again */

    /*
     * set global DMA enable
     */
    GDMA_MAIN_CTRL->bits.dma_en = 1;

    GDMA_EXIT_RETURN(GDMA_OK);
}


/**
 * @brief   This function deactivates all jobs of the current job set.
 *          The jobs can't be triggered via hardware or software afterwards. A running job is not interrupted but finished.
 */
LSA_VOID gdma_disable_all_jobs()
{
    LSA_UINT16                  job_num;
    gdma_jobx_ctrl_reg_t        job_reg;

    GDMA_ENTER();

    /* ******************************* */
    /* wait for running jobs to finish */
    /* ******************************* */

            /* disable all hardware / software triggers */
    for(job_num = 0; job_num < GDMA_MAX_JOBS; job_num++)
    {
        job_reg.reg = GDMA_JOBX_CTRL(job_num)->reg;

        /* prevent further triggering of the job to be removed -> avoid an interruption of a running transfer */
        job_reg.bits.hw_job_start_en = 0;
        GDMA_JOBX_CTRL(job_num)->reg = job_reg.reg;                                                                         /* job may not be triggered by software anymore */
        gdma_data.gdma_disabled_jobs_map[gdma_data.gdma_active_job_pool_idx][job_num / 32]  |= (1 << (job_num % 32));       /* job may not be triggered by software anymore */
    }

    /* wait while a DMA job is in the state START / RUN / INTERRUPT.
     * All jobs in state START (queued but not executed so far) are finished first because the bits JOB_EN and DMA_EN are still active.
     *  */
    while(GDMA_ACTUAL_STATUS->bits.act_job_val == 1)
        ;

    /*
     * set global DMA disable
     */
    GDMA_MAIN_CTRL->bits.dma_en = 0;

    GDMA_EXIT();
}

/**
 * @brief   This function deactivates a single job. The job can't be triggered via hardware or software afterwards.
 *
 * @param   job_num     job number of the job to be deactivated
 * @return  GDMA_OK if the job can be activated, GDMA_ERR_PARAMETER if the passed parameter is wrong
 */
LSA_INT16 gdma_disable_job(LSA_INT16 job_num)
{
    LSA_UINT16 job_pool_index = (LSA_UINT16)(job_num / GDMA_MAX_JOBS);

    job_num = job_num % GDMA_MAX_JOBS;

    GDMA_ENTER();

    GDMA_ASSERT(job_pool_index < GDMA_CFG_NUM_JOB_LISTS, GDMA_ERR_PARAMETER);                                                   /* error case : job_num is out of range */
    GDMA_ASSERT(job_pool_index == gdma_data.gdma_active_job_pool_idx, GDMA_ERR_PARAMETER);                                      /* error case : only jobs of the current job set can be deactivated */
    GDMA_ASSERT((gdma_data.gdma_used_jobs_map[job_pool_index][job_num / 32] & (1 << (job_num % 32))) != 0, GDMA_ERR_PARAMETER);   /* error case : job is not initialized */

    GDMA_JOBX_CTRL(job_num)->bits.hw_job_start_en = 0;
    gdma_data.gdma_disabled_jobs_map[gdma_data.gdma_active_job_pool_idx][job_num / 32]  |= (1 << (job_num % 32));               /* job may not be triggered by software again */

    /* wait until the DMA job is no longer busy */
    while(GDMA_ACTUAL_STATUS->bits.act_job_val == 1 && GDMA_ACTUAL_STATUS->bits.act_job == job_num)
        ;

    GDMA_EXIT_RETURN(GDMA_OK);
}

#ifdef GDMA_CFG_LOCATE_INSTRUCTIONS_TO_TCM
    #if defined (TOOL_CHAIN_GREENHILLS_ARM)
        #pragma ghs section text=GDMA_ITCM_SECTION_NAME
    #endif
#endif
/**
 * @brief   This function triggers a DMA job through software.
 *
 * @param job_num   Job identifier of the job to be triggered. The identifier equals the
 *                  return value of gdma_add_job() for the corresponding job.
 */
GDMA_ATTRIBUTE_ITCM LSA_INT16 gdma_trigger_job(LSA_INT16 job_num)
{
    LSA_UINT16 job_pool_index = job_num / GDMA_MAX_JOBS;

    job_num = job_num % GDMA_MAX_JOBS;

    GDMA_ENTER();

    GDMA_ASSERT(job_pool_index < GDMA_CFG_NUM_JOB_LISTS, GDMA_ERR_PARAMETER);                                                   /* error case : the job_num passed is wrong */
    GDMA_ASSERT(job_pool_index == gdma_data.gdma_active_job_pool_idx, GDMA_ERR_PARAMETER);                                      /* error case : only jobs in the active pool may be triggered */
    
    /* Boundry check is done. First index is defined as [GDMA_CFG_NUM_JOB_LISTS] and second index is defined as [(GDMA_MAX_JOBS + 31) /32] */
    if( (job_pool_index < GDMA_CFG_NUM_JOB_LISTS) && ((job_num / 32) < ((GDMA_MAX_JOBS + 31) /32)) )
    {
        GDMA_ASSERT((gdma_data.gdma_used_jobs_map[job_pool_index][job_num / 32]  & (1 << (job_num % 32))) != 0, GDMA_ERR_PARAMETER);  /* error case : job is not initialized */
        if(gdma_data.gdma_disabled_jobs_map[job_pool_index][job_num / 32] & (1 << (job_num % 32)))
        {
            /* job may not be triggered / it's disabled */
            GDMA_EXIT_RETURN(GDMA_OK);
        }
    }

    GDMA_JOBX_CTRL(job_num)->bits.sw_job_start = 1;
    GDMA_EXIT_RETURN(GDMA_OK);
}
#ifdef GDMA_CFG_LOCATE_INSTRUCTIONS_TO_TCM
    #if defined (TOOL_CHAIN_GREENHILLS_ARM)
        #pragma ghs section text=GDMA_DEFAULT_TEXT_SECTION_NAME
    #endif
#endif


/* ------------------------------------------------------------------------ */
/* internal functions                                                       */
/* ------------------------------------------------------------------------ */

/**
 * setup of transfer records in the transfer list
 *
 * @param   transfer_list_base_addr     addresses the base address of the transfer list
 * @param   offset                      describes the offset in reference to transfer_list_base_addr, that is used for the first new transfer list entry
 * @param   num_transfers               specifies the number of new transfer list entries
 * @param   transfers                   structure containing "num_transfers" entries, that describes each transfer individually
 */
/*static*/ LSA_INT16 gdma_setup_transfer_records(LSA_UINT32 transfer_list_base_addr, LSA_UINT16 offset, LSA_UINT16 num_transfers, gdma_transfer_descr_t *transfers)
{
    LSA_UINT16 transfer_idx;

    GDMA_ASSERT(num_transfers <= GDMA_MAX_TRANSFERS, GDMA_ERR_PARAMETER);

    for(transfer_idx = 0; transfer_idx < num_transfers; transfer_idx++)
    {
        gdma_trans_recx_cnt_reg_t transfer_count;
        transfer_count.reg = 0;

        GDMA_ASSERT(transfers[transfer_idx].magic_number == GDMA_TRANSFER_DESCR_MAGIC, GDMA_ERR_PARAMETER);
            /* error case: there's no magic number in the transfer lists field -> invalid record
             * we return without resetting already parameterized transfer list entries - they're not activated anyway (-> job ctrl-reg.)*/

        volatile LSA_UINT32* pRecxSrc = GDMA_TRANS_RECX_SRC(transfer_list_base_addr, offset + transfer_idx);
        volatile LSA_UINT32* pRecxDst = GDMA_TRANS_RECX_DST(transfer_list_base_addr, offset + transfer_idx);

        GDMA_ASSERT(pRecxSrc != LSA_NULL, GDMA_ERR_MEM);
        GDMA_ASSERT(pRecxDst != LSA_NULL, GDMA_ERR_MEM);

        *pRecxSrc = (LSA_UINT32) transfers[transfer_idx].src_addr;
        *pRecxDst = (LSA_UINT32) transfers[transfer_idx].dst_addr;

        /* source mode = increment address, dest mode = increment address, burst mode = INCR8*/
        GDMA_ASSERT(transfers[transfer_idx].burst_mode <= GDMA_BURST_MODE_INCR16, GDMA_ERR_PARAMETER);   /*lint !e685 Relational operator '>' always evaluates to 'true' */
                                                                                                         /* error case : burst mode is set incorrectly */
        *GDMA_TRANS_RECX_CTRL(transfer_list_base_addr, offset + transfer_idx) = transfers[transfer_idx].burst_mode << 18;


        /* transfer count data is set */
        transfer_count.bits.en_dma_ack = 0;                      /* no HW_DMA_ACK after a finished transfer */

        if(transfer_idx == num_transfers - 1)
        {
            transfer_count.bits.last_tr = 1;
        }
        else
        {
            transfer_count.bits.last_tr = 0;
        }

        GDMA_ASSERT(transfers[transfer_idx].elem_size <= GDMA_ESIZE_32BIT, GDMA_ERR_PARAMETER);   /* error case : elem_size is incorrectly set */
        transfer_count.bits.esize = transfers[transfer_idx].elem_size;

        GDMA_ASSERT(transfers[transfer_idx].transfer_count > 0 , GDMA_ERR_PARAMETER);     /* error case : transfer_count field in transfer descriptor is incorrectly set */
        transfer_count.bits.tr_count = transfers[transfer_idx].transfer_count - 1;     /* 1 transfer corresponds to register value 0 */

        GDMA_TRANS_RECX_CNT(transfer_list_base_addr, offset + transfer_idx)->reg = transfer_count.reg;
    }

    return GDMA_OK;
}

static LSA_FATAL_ERROR_TYPE gdma_error_descriptor;

/*static*/ LSA_VOID gdma_in_fatal_error(LSA_UINT8 error_module, LSA_UINT16 error_line, LSA_UINT16 error_code)
{
    gdma_error_descriptor.lsa_component_id = PNIO_PACKID_GDMA;

    gdma_error_descriptor.module_id        = error_module;
    gdma_error_descriptor.line             = error_line;

    gdma_error_descriptor.error_code[0]    = error_code;
    gdma_error_descriptor.error_code[1]    =
    gdma_error_descriptor.error_code[2]    =
    gdma_error_descriptor.error_code[3]    = 0;

    gdma_error_descriptor.error_data_length = sizeof(gdma_data);
    gdma_error_descriptor.error_data_ptr    = &gdma_data;

    GDMA_FATAL_ERROR(&gdma_error_descriptor);
}

/**
 *LSA function for retreiving the version info of the GDMA component
 */
LSA_UINT16 gdma_version(LSA_UINT16 version_len, gdma_version_info_ptr_t version_ptr)
{
    if ( sizeof( LSA_VERSION_TYPE) <= version_len )
    {
        LSA_UINT32 i;
        LSA_UINT8  tmp_prefix[LSA_PREFIX_SIZE] = GDMA_LSA_PREFIX;

        version_ptr->lsa_component_id = GDMA_LSA_COMPONENT_ID;

        for ( i = 0; LSA_PREFIX_SIZE > i; i++ )
        {
            version_ptr->lsa_prefix[i] = tmp_prefix[i];
        }

        version_ptr->kind                = GDMA_KIND;
        version_ptr->version             = GDMA_VERSION;
        version_ptr->distribution        = GDMA_DISTRIBUTION;
        version_ptr->fix                 = GDMA_FIX;
        version_ptr->hotfix              = GDMA_HOTFIX;
        version_ptr->project_number      = GDMA_PROJECT_NUMBER;
        version_ptr->increment           = GDMA_INCREMENT;
        version_ptr->integration_counter = GDMA_INTEGRATION_COUNTER;
        version_ptr->gen_counter         = GDMA_GEN_COUNTER;

        return( 0);
    }
    else
    {
        return( version_len);
    }
}

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
