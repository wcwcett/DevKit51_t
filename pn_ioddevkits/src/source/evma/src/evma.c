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
/*  F i l e               &F: evma.c                                    :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  software module handling                                                 */
/*  - ERTEC200+ pnpll application timer 2 and its comparators                */
/*    as well as the pnpll MUX                                               */
/*                                                                           */
/*****************************************************************************/

#include "evma_inc.h"

#define EVMA_LOCAL_ERR_MODULE EVMA_MODULE

    /* disable inlining for debugging purposes */
#define EVMA_INLINE inline
#ifndef EVMA_INLINE
    #define EVMA_INLINE
#endif


typedef struct evma_data_s
{
        /* each EVMA instance, that is returned by evma_alloc_event() is saved here - for every comparator, EVMA_MAX_NUM_INSTANCES_PER_COMPARATOR
         * instances may be allocated. */
    evma_instance_t evma_instance_descriptor[EVMA_NUM_COMPARATORS][EVMA_MAX_NUM_INSTANCES_PER_COMPARATOR];

    LSA_UINT8       used_comparator_map;
    #ifdef EVMA_USE_PNIP
    LSA_UINT32      outputs_ref_counter[EVMA_NUM_OUTPUTS];
    #endif
}evma_data_t;

static evma_data_t evma_data;


/* -------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------------- */
/* *********************   general (PNIP) section, interrupt handling    ********************** */
/* -------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------------- */

/* locate all the ISR specific stuff into the TCM, if parameterized */
#include "evma_tcm_enter.h"        /*lint !e451 allow repeated include */

typedef struct evma_callback_descriptor_s
{
    struct       evma_list_cb list_head;
    PNIO_VOID    (*callback)(PNIO_VOID);
}evma_callback_descriptor_t;

/* PNPLL9...14 can trigger an interrupt -> for each PNPLL output several callback instances can be registered */
static evma_callback_descriptor_t evma_callback_descriptors[EVMA_NUM_ISR_VECTORS][EVMA_MAX_NUM_INSTANCES_PER_COMPARATOR]  EVMA_ATTRIBUTE_DTCM;
static struct evma_list_cb        evma_callback_lists[EVMA_NUM_ISR_VECTORS]                                               EVMA_ATTRIBUTE_DTCM;


#define EVMA_EXECUTE_ALL_CALLBACKS(irq_index)                                                           \
                    {                                                                                   \
                        struct evma_list_cb *list_ptr = &evma_callback_lists[irq_index];                \
                        struct evma_list_cb *cur_callback_elem_ptr;                                     \
                        evma_get_blk_from_list__(list_ptr, &cur_callback_elem_ptr);                     \
                        while(cur_callback_elem_ptr != list_ptr)                                        \
                        {                                                                               \
                            ((evma_callback_descriptor_t*)cur_callback_elem_ptr)->callback();           \
                            evma_get_blk_from_list__(cur_callback_elem_ptr, &cur_callback_elem_ptr);    \
                        }                                                                               \
                    }

#include "evma_tcm_exit.h"        /*lint !e451 allow repeated include */


#ifdef EVMA_USE_PNIP

/* -------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------------- */
/* ******************** PNIP specific implementation of the event manager ********************* */
/* -------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------------- */

/* locate all the ISR specific stuff into the TCM, if parameterized */
#include "evma_tcm_enter.h"        /*lint !e451 allow repeated include */

static LSA_VOID evma_pnip_9_isr() EVMA_ATTRIBUTE_ITCM;
static LSA_VOID evma_pnip_10_isr() EVMA_ATTRIBUTE_ITCM;
static LSA_VOID evma_pnip_11_isr() EVMA_ATTRIBUTE_ITCM;
static LSA_VOID evma_pnip_12_isr() EVMA_ATTRIBUTE_ITCM;
static LSA_VOID evma_pnip_13_isr() EVMA_ATTRIBUTE_ITCM;
static LSA_VOID evma_pnip_14_isr() EVMA_ATTRIBUTE_ITCM;

static LSA_VOID evma_pnip_9_isr()
{
    EVMA_EXECUTE_ALL_CALLBACKS(0);
}

static LSA_VOID evma_pnip_10_isr()
{
    EVMA_EXECUTE_ALL_CALLBACKS(1);
}

static LSA_VOID evma_pnip_11_isr()
{
    EVMA_EXECUTE_ALL_CALLBACKS(2);
}

static LSA_VOID evma_pnip_12_isr()
{
    EVMA_EXECUTE_ALL_CALLBACKS(3);
}

static LSA_VOID evma_pnip_13_isr()
{
    EVMA_EXECUTE_ALL_CALLBACKS(4);
}

static LSA_VOID evma_pnip_14_isr()
{
    EVMA_EXECUTE_ALL_CALLBACKS(5);
}

#include "evma_tcm_exit.h"        /*lint !e451 allow repeated include */



static LSA_INT8 evma_pnip_find_output_resource(evma_alloc_params_t *alloc_params)
{
    LSA_UINT8 min_output_index, max_output_index;
    LSA_UINT8 output_index;

    switch(alloc_params->application_type)
    {
        case EVMA_APPLICATION_ARM_ICU:
            min_output_index = 9;
            max_output_index = 14;
            break;
        case EVMA_APPLICATION_GDMA:
            min_output_index = 11;
            max_output_index = 20;
            break;
        case EVMA_APPLICATION_ARM_ICU_GDMA:
            min_output_index = 11;
            max_output_index = 14;
            break;
        case EVMA_APPLICATION_GPIO:
            if(alloc_params->spec_params.gpio_params.gpio_index > 8)              /* GPIOs 0-8 are connected to PNPLL_OUT0..8 */
            {
                return -1;
            }
            if(alloc_params->spec_params.gpio_params.gpio_index == 0)             /* GPIO 0 is managed by EDDP */
            {
                return -1;
            }
            min_output_index = alloc_params->spec_params.gpio_params.gpio_index;
            max_output_index = alloc_params->spec_params.gpio_params.gpio_index;
            break;
        case EVMA_APPLICATION_SSI:
            min_output_index = 3;
            max_output_index = 3;
            break;
        case EVMA_PERIF_TI_IN:
            evma_in_fatal_error(EVMA_LOCAL_ERR_MODULE, __LINE__, LSA_RET_ERR_PARAM);
            return -1;
            /* output 0 is reserved for EDDP !
            min_output_index = 0;
            max_output_index = 0;
            */
        case EVMA_PERIF_TO_IN:
            min_output_index = 1;
            max_output_index = 1;
            break;
        default:
            evma_in_fatal_error(EVMA_LOCAL_ERR_MODULE, __LINE__, LSA_RET_ERR_PARAM);
            return -1;
    }

    /* find a free output resource */
    for(output_index = min_output_index; output_index <= max_output_index; output_index++)
    {
        if(evma_data.outputs_ref_counter[output_index] == 0)
        {
            break;
        }
    }
    if(output_index > max_output_index)
    {
        /* all possible outputs are already in use */
        return -1;
    }

    return output_index;
}

static LSA_VOID evma_pnip_set_instance_descriptor(evma_instance_t *instance_ptr,
                                                  LSA_UINT8  comparator_index,
                                                  LSA_UINT32 trigger_time_ns,
                                                  LSA_INT8 output_index,
                                                  evma_alloc_params_t *alloc_params)
{
    instance_ptr->magic_value              = EVMA_INST_DESC_MAGIC;
    instance_ptr->state                    = EVMA_STATE_IN_USE;
    instance_ptr->app_type                 = alloc_params->application_type;
    instance_ptr->output_index             = output_index;
    instance_ptr->application_timer_index  = 2;
    instance_ptr->comparator_index         = comparator_index;
    instance_ptr->trigger_time_ns          = trigger_time_ns;

    switch(alloc_params->application_type)
    {
    case EVMA_APPLICATION_ARM_ICU:
        instance_ptr->pnpll_isr           = alloc_params->spec_params.icu_params.event_cbf;
        instance_ptr->gpio_index          = 0;
        instance_ptr->gpio_level          = EVMA_GPIO_LOW_ACTIVE;
        break;
    case EVMA_APPLICATION_ARM_ICU_GDMA:
        instance_ptr->gpio_index          = 0;
        instance_ptr->gpio_level          = EVMA_GPIO_LOW_ACTIVE;
        instance_ptr->pnpll_isr           = alloc_params->spec_params.icu_gdma_params.event_cbf;
        break;
    case EVMA_APPLICATION_GPIO:
        instance_ptr->gpio_index          = alloc_params->spec_params.gpio_params.gpio_index;
        instance_ptr->gpio_level          = alloc_params->spec_params.gpio_params.level_type;
        instance_ptr->pnpll_isr           = LSA_NULL;
        break;
    default:
        instance_ptr->gpio_index          = 0;
        instance_ptr->gpio_level          = EVMA_GPIO_LOW_ACTIVE;
        instance_ptr->pnpll_isr           = LSA_NULL;
        break;
    }
}

static LSA_VOID evma_pnip_set_pll_out_mux(LSA_INT8 output_index, LSA_UINT8 comparator_index, evma_alloc_params_t *alloc_params)
{
    if ((output_index < 0) || (output_index >= EVMA_NUM_OUTPUTS))
    {
        evma_in_fatal_error(EVMA_LOCAL_ERR_MODULE, __LINE__, LSA_RET_ERR_PARAM);
        return;
    }
    else
    {
        LSA_UINT8 output_idx = output_index;
        /* parameterize output MUX */
        if(evma_data.outputs_ref_counter[output_idx] == 0)
        {
            EVMA_PNIP_PLL_OUT_CONTROL_REG   reg_tmp;
                  /*
                   * bits 23-17: duration of active PNPLL output at a resolution of 8 ns
                   * bit     16: 1=high active, 0=low active
                   * bits 4 - 0: comparator event index. 1 = timer 1, comparator 1; ...; 8 = timer 2, comparator 1; 15 = timer 3, comparator 1
                   */
            reg_tmp.reg32 = 0;
            reg_tmp.bits.PLL_CMPSelect = (EVMA_NUM_COMPARATORS + comparator_index + 1);
            if(alloc_params->application_type == EVMA_APPLICATION_GPIO)
            {
                reg_tmp.bits.PLL_Level = alloc_params->spec_params.gpio_params.level_type;
            }
            else
            {
                reg_tmp.bits.PLL_Level = 1;
            }
            reg_tmp.bits.PLL_Length = 0x7F;

            EVMA_WRITE_PNIP_PLL_OUT_CONTROL(output_idx, reg_tmp.reg32);
        }

        evma_data.outputs_ref_counter[output_idx] += 1;           /* increase usage counter of this output */
    }
}

static evma_instance_t* evma_pnip_get_free_comparator_instance(LSA_UINT8 comparator_index)
{
    LSA_UINT8 comparator_instance_idx;
    for(comparator_instance_idx = 0; comparator_instance_idx < EVMA_MAX_NUM_INSTANCES_PER_COMPARATOR; comparator_instance_idx++)
    {
        if(evma_data.evma_instance_descriptor[comparator_index][comparator_instance_idx].state == EVMA_STATE_IDLE)
        {
            return &evma_data.evma_instance_descriptor[comparator_index][comparator_instance_idx];
        }
    }

    return LSA_NULL;
}

static LSA_VOID evma_pnip_try_add_cbf(LSA_UINT8 output_index, evma_alloc_params_t *alloc_params)
{
    LSA_UINT8 i;
    LSA_VOID (*cbf)(LSA_VOID) = LSA_NULL;

    if(alloc_params->application_type == EVMA_APPLICATION_ARM_ICU)
    {
        cbf = alloc_params->spec_params.icu_params.event_cbf;
    }
    else if(alloc_params->application_type == EVMA_APPLICATION_ARM_ICU_GDMA)
    {
        cbf = alloc_params->spec_params.icu_gdma_params.event_cbf;
    }

    if(cbf)
    {
        /* PNPLL9...14 can cause interrupts */
        if(output_index < 9 || output_index > 14)
        {
            evma_in_fatal_error(EVMA_LOCAL_ERR_MODULE, __LINE__, LSA_RET_ERR_PARAM);
            return;
        }
        /* find a free function pointer for the output >output_index< */
        for(i = 0; i < EVMA_MAX_NUM_INSTANCES_PER_COMPARATOR; i++)
        {
            if(evma_callback_descriptors[output_index - 9][i].callback == LSA_NULL)
            {
                break;
            }
        }
        if(i >= EVMA_MAX_NUM_INSTANCES_PER_COMPARATOR)
        {
            evma_in_fatal_error(EVMA_LOCAL_ERR_MODULE, __LINE__, LSA_RET_ERR_RESOURCE);
            return;
        }

        /* enable the interrupt, if it is not enabled so far */
        if(evma_list_empty__(&evma_callback_lists[output_index-9]))
        {
            switch(output_index)
            {
                case 9:
                    EVMA_SET_INT_HANDLER(EVMA_INT_PNPLL_OUT9, evma_pnip_9_isr);
                    break;
                case 10:
                    EVMA_SET_INT_HANDLER(EVMA_INT_PNPLL_OUT10, evma_pnip_10_isr);
                    break;
                case 11:
                    EVMA_SET_INT_HANDLER(EVMA_INT_PNPLL_OUT11, evma_pnip_11_isr);
                    break;
                case 12:
                    EVMA_SET_INT_HANDLER(EVMA_INT_PNPLL_OUT12, evma_pnip_12_isr);
                    break;
                case 13:
                    EVMA_SET_INT_HANDLER(EVMA_INT_PNPLL_OUT13, evma_pnip_13_isr);
                    break;
                case 14:
                    EVMA_SET_INT_HANDLER(EVMA_INT_PNPLL_OUT14, evma_pnip_14_isr);
                    break;
                default:
                    break;
            }
        }

        /* add the callback to the list. Order of these 2 commands is important! */
        evma_callback_descriptors[output_index - 9][i].callback = cbf;
        evma_put_blk_to_list__(&evma_callback_lists[output_index-9], &evma_callback_descriptors[output_index - 9][i].list_head);
    }
}

static LSA_VOID evma_pnip_remove_cbf(LSA_UINT8 output_index, LSA_VOID (*cbf)(LSA_VOID))
{
    LSA_UINT8 i;

    if(cbf)
    {
        /* PNPLL9...14 can cause interrupts */
        if(output_index < 9 || output_index > 14)
        {
            evma_in_fatal_error(EVMA_LOCAL_ERR_MODULE, __LINE__, LSA_RET_ERR_PARAM);
            return;
        }
        /* find the specified function pointer for the output >output_index< */
        for(i = 0; i < EVMA_MAX_NUM_INSTANCES_PER_COMPARATOR; i++)
        {
            if(evma_callback_descriptors[output_index - 9][i].callback == cbf)
            {
                break;
            }
        }
        if(i >= EVMA_MAX_NUM_INSTANCES_PER_COMPARATOR)
        {
            evma_in_fatal_error(EVMA_LOCAL_ERR_MODULE, __LINE__, LSA_RET_ERR_PARAM);
            return;
        }

        evma_remove_blk__(&evma_callback_descriptors[output_index - 9][i].list_head);
        evma_callback_descriptors[output_index - 9][i].callback = NULL;               /* order of these 2 commands is important! - the other way round we would jump to address 0 in ISR */

        /* disable the interrupt, if it is not needed anymore */
        if(evma_list_empty__(&evma_callback_lists[output_index-9]))
        {
            switch(output_index)
            {
                case 9:
                    EVMA_SET_INT_HANDLER(EVMA_INT_PNPLL_OUT9, NULL);
                    break;
                case 10:
                    EVMA_SET_INT_HANDLER(EVMA_INT_PNPLL_OUT10, NULL);
                    break;
                case 11:
                    EVMA_SET_INT_HANDLER(EVMA_INT_PNPLL_OUT11, NULL);
                    break;
                case 12:
                    EVMA_SET_INT_HANDLER(EVMA_INT_PNPLL_OUT12, NULL);
                    break;
                case 13:
                    EVMA_SET_INT_HANDLER(EVMA_INT_PNPLL_OUT13, NULL);
                    break;
                case 14:
                    EVMA_SET_INT_HANDLER(EVMA_INT_PNPLL_OUT14, NULL);
                    break;
                default:
                    break;
            }
        }
    }
}

static evma_instance_t* evma_pnip_check_comparator_reuse(LSA_UINT8 comparator_index,
                                                         evma_alloc_params_t* alloc_params,
                                                         LSA_UINT32 trigger_time_ns)
{
    LSA_UINT8 instance_idx;
    LSA_BOOL comparator_same_trigger_found = LSA_FALSE;
    LSA_INT8 output_index;
    evma_instance_t *new_instance;

    /* check if there is an active instance for this comparator and if the comparator uses the wanted trigger time AND is also
     * connected to a suitable PNPLL output (depending on the purpose / application type) */
    for(instance_idx = 0; instance_idx < EVMA_MAX_NUM_INSTANCES_PER_COMPARATOR; instance_idx++)
    {
        if(evma_data.evma_instance_descriptor[comparator_index][instance_idx].state == EVMA_STATE_IN_USE)
        {
            if(evma_data.evma_instance_descriptor[comparator_index][instance_idx].trigger_time_ns == trigger_time_ns)
            {
                if(evma_data.evma_instance_descriptor[comparator_index][instance_idx].app_type == alloc_params->application_type)
                {
                    LSA_BOOL instance_is_identical = LSA_FALSE;

                    switch(alloc_params->application_type)
                    {
                    case EVMA_APPLICATION_GPIO:
                        if(alloc_params->spec_params.gpio_params.gpio_index == evma_data.evma_instance_descriptor[comparator_index][instance_idx].gpio_index &&
                           alloc_params->spec_params.gpio_params.level_type == evma_data.evma_instance_descriptor[comparator_index][instance_idx].gpio_level)
                        {
                            instance_is_identical = LSA_TRUE;
                        }
                        break;
                    default:
                        instance_is_identical = LSA_TRUE;
                        break;
                    }

                    if(instance_is_identical)
                    {
                        /* a suitable instance already exists
                         * -> use the same comparator
                         * -> use the same output
                         * -> allocate a new instance descriptor */
                        new_instance = evma_pnip_get_free_comparator_instance(comparator_index);
                        if(new_instance)
                        {
                            output_index = evma_data.evma_instance_descriptor[comparator_index][instance_idx].output_index;     /* use the same output as the current instance */

                            evma_pnip_set_pll_out_mux(output_index, comparator_index, alloc_params);                                       /* increase output reference counter */
                            evma_pnip_set_instance_descriptor(new_instance, comparator_index, trigger_time_ns, output_index, alloc_params);
                            evma_pnip_try_add_cbf(output_index, alloc_params);                                             /* add callback to list for this output */
                            return new_instance;
                        }
                        else
                        {
                            return LSA_NULL;
                        }
                    }
                }
                comparator_same_trigger_found = LSA_TRUE;
            }
        }
    }

    if(comparator_same_trigger_found)
    {
        /* there is a comparator with a suitable trigger time but the PNPLL output link is unsuitable
         * -> try to make a second connection to this comparator (-> another output) */

        /* find a free output resource (output MUX) */
        output_index = evma_pnip_find_output_resource(alloc_params);
        if(output_index < 0)
        {
            /* no suitable output resource left */
            return LSA_NULL;
        }

        new_instance = evma_pnip_get_free_comparator_instance(comparator_index);
        if(new_instance)
        {
            /* connect free output resource to the comparator */
            evma_pnip_set_pll_out_mux(output_index, comparator_index, alloc_params);

            evma_pnip_set_instance_descriptor(new_instance, comparator_index, trigger_time_ns, output_index, alloc_params);
            evma_pnip_try_add_cbf(output_index, alloc_params);
            return new_instance;
        }
        else
        {
            return LSA_NULL;
        }
    }
    return LSA_NULL;
}

/**
 * helper function that returns LSA_TRUE, if all parameters in alloc_params are plausible;
 * LSA_FALSE otherwise.
 */
static LSA_BOOL evma_pnip_check_alloc_parameters(evma_alloc_params_t* alloc_params)
{
    switch(alloc_params->application_type)
    {
    case EVMA_APPLICATION_ARM_ICU:
        if(alloc_params->spec_params.icu_params.event_cbf == NULL)
        {
            return LSA_FALSE;
        }
        break;
    case EVMA_APPLICATION_ARM_ICU_GDMA:
        if(alloc_params->spec_params.icu_gdma_params.event_cbf == NULL)
        {
            return LSA_FALSE;
        }
        break;
    case EVMA_APPLICATION_GPIO:
        if(alloc_params->spec_params.gpio_params.gpio_index < 1 || alloc_params->spec_params.gpio_params.gpio_index > 8)
        {
            return LSA_FALSE;
        }
        if(alloc_params->spec_params.gpio_params.level_type > EVMA_GPIO_HIGH_ACTIVE)    /*lint !e685 Relational operator '>' always evaluates to 'false' */
        {
            return LSA_FALSE;
        }
        break;
    case EVMA_APPLICATION_GDMA:
    case EVMA_APPLICATION_SSI:
    case EVMA_PERIF_TI_IN:
    case EVMA_PERIF_TO_IN:
        break;
    default:
        return LSA_FALSE;
    }
    return LSA_TRUE;
}

/**
 * @brief   initialization function of the EVMA module. This function must be called before allocating any events.
 */
static EVMA_INLINE PNIO_VOID evma_pnip_init()
{
    LSA_UINT8 i;
    EVMA_MEMSET(evma_data.evma_instance_descriptor, 0, sizeof(evma_instance_t) * EVMA_NUM_COMPARATORS * EVMA_MAX_NUM_INSTANCES_PER_COMPARATOR);
    for(i = 0; i < EVMA_NUM_ISR_VECTORS; i++)
    {
        evma_init_list__(&evma_callback_lists[i]);
    }
    EVMA_MEMSET(evma_callback_descriptors, 0, sizeof(evma_callback_descriptors));
    EVMA_MEMSET(evma_data.outputs_ref_counter, 0, sizeof(evma_data.outputs_ref_counter));

    evma_data.used_comparator_map = 0x01;         /* comparator 0 of application timer 2 is used by EDDP (TransferEnd) */
}


static EVMA_INLINE evma_handle_t evma_pnip_alloc_event(evma_alloc_params_t *alloc_params, LSA_UINT32 trigger_time_ns)
{
    LSA_UINT8 comparator_index = 0xFF;
    LSA_INT8 output_index;
    evma_instance_t *new_instance;

    /* **************************** */
    /* check validity of parameters */
    /* **************************** */
    if(evma_pnip_check_alloc_parameters(alloc_params) == LSA_FALSE)
    {
        return LSA_NULL;
    }

    EVMA_ENTER();

    /* ******************************************* */
    /* look for a comparator with identical timing */
    /* ******************************************* */
    for(comparator_index = 1; comparator_index < EVMA_NUM_COMPARATORS; comparator_index++)
    {
        evma_instance_t *reuse_instance;
        reuse_instance = evma_pnip_check_comparator_reuse(comparator_index, alloc_params, trigger_time_ns);

        if(reuse_instance)
        {
            EVMA_EXIT();
            return reuse_instance;
        }
    }

    /* there is no comparator set to the same trigger level -> allocate a new comparator */

    /* find a free output resource (output MUX) */
    output_index = evma_pnip_find_output_resource(alloc_params);
    if(output_index < 0)
    {
        EVMA_EXIT();
        return LSA_NULL;
    }

    /* find a free comparator resource. Comparator 0 of application timer 2 is used by eddp */
    for(comparator_index = 1; comparator_index < EVMA_NUM_COMPARATORS; comparator_index++)
    {
        if((evma_data.used_comparator_map & (1 << comparator_index)) == 0)
        {
            break;
        }
    }
    if(comparator_index == EVMA_NUM_COMPARATORS)
    {
        /* all comparators are already used within this timer */
        EVMA_EXIT();
        return LSA_NULL;
    }

    /* find a free descriptor resource */
    new_instance = evma_pnip_get_free_comparator_instance(comparator_index);
    if(new_instance == LSA_NULL)
    {
        EVMA_EXIT();     /* no instance can be allocated any more */
        return LSA_NULL;
    }

    /* ---------------------------- all checks done - now I'm parameterizing ------------------- */

    /* ----------------------------------------------------------------------------------------- */
    /* parameterize the trigger threshold (comparator)                                           */

    /* mark the free comparator resource as used */
    evma_data.used_comparator_map |= (1 << comparator_index);   /* use comparator */

    /* SET APPLLENGTH -> is done by EDDP for timer 2 */

    /* set comparator value */
    EVMA_WRITE_PNIP_APPLTIMER2_COMP_TIME(comparator_index, trigger_time_ns);

    /* ---------------------------------------------------------------------------------------------- */
    /* parameterize output MUX -> output <output_index> is connected to comparator <comparator_index> */
    evma_pnip_set_pll_out_mux(output_index, comparator_index, alloc_params);

    /* ------------------------------------------ */
    /* save all values to the instance descriptor */
    evma_pnip_set_instance_descriptor(new_instance,
            comparator_index, trigger_time_ns, output_index, alloc_params);

    /* add callback to isr-list for this output (if specified) */
    evma_pnip_try_add_cbf(output_index, alloc_params);

    EVMA_EXIT();
    return (evma_handle_t) new_instance;
}

static EVMA_INLINE evma_handle_t evma_pnip_alloc_event_transfer_end( evma_alloc_params_t *alloc_params )
{
    LSA_INT8 output_index;
    evma_instance_t *reuse_instance;
    evma_instance_t *new_instance;

    /* **************************** */
    /* check validity of parameters */
    /* **************************** */
    if(evma_pnip_check_alloc_parameters(alloc_params) == LSA_FALSE)
    {
        return LSA_NULL;
    }

    EVMA_ENTER();

    /* check if comparator 0 is already in use and connected to an appropriate PNPLL-output
     * -> if so, return the instance pointer. Trigger-time is set to 0 for all instances of the TransferEnd comparator (is managed by EDDP)
     */
    reuse_instance = evma_pnip_check_comparator_reuse(0, alloc_params, 0);
    if(reuse_instance)
    {
        EVMA_EXIT();
        return reuse_instance;
    }

    /* comparator 0 is not used so far ... */

    /* find a free output resource (output MUX) */
    output_index = evma_pnip_find_output_resource(alloc_params);
    if(output_index < 0)
    {
        EVMA_EXIT();
        return LSA_NULL;
    }

    /* find a free descriptor resource */
    new_instance = evma_pnip_get_free_comparator_instance(0);
    if(new_instance == LSA_NULL)
    {
        EVMA_EXIT();     /* no instance can be allocated any more */
        return LSA_NULL;
    }

    /* ---------------------------- all checks done - now I'm parameterizing ------------------- */

    /* SET APPLLENGTH -> is done by EDDP */

    /* comparator value is set by EDDP, if parameterized during EDDP initialization */
    /* *((LSA_UINT32*) U_PNIP__APPLCOMPARE_2_1 + 0) = 500000;*/

    /* ---------------------------------------------------------------------------------------------- */
    /* parameterize output MUX -> output <output_index> is connected to comparator 0 */
    evma_pnip_set_pll_out_mux(output_index, 0, alloc_params);

    /* ------------------------------------------ */
    /* save all values to the instance descriptor */
        /* trigger_time -> set to 0 (set by EDDP in hardware), comparator index = 0 (transferEnd, managed by EDDP) */
    evma_pnip_set_instance_descriptor(new_instance, 0, 0, output_index, alloc_params);

    /* add callback to isr-list for this output (if specified) */
    evma_pnip_try_add_cbf(output_index, alloc_params);

    EVMA_EXIT();
    return (evma_handle_t) new_instance;
}

static EVMA_INLINE LSA_UINT8 evma_pnip_free_event(evma_handle_t evma_handle)
{
    evma_instance_t* instance = (evma_instance_t*) evma_handle;

    EVMA_ASSERT(instance                  != LSA_NULL            , LSA_RET_ERR_PARAM);
    EVMA_ASSERT(instance->magic_value     == EVMA_INST_DESC_MAGIC, LSA_RET_ERR_PARAM);
    EVMA_ASSERT(instance->state           != EVMA_STATE_IDLE     , LSA_RET_ERR_PARAM);
    EVMA_ASSERT(instance->comparator_index < EVMA_NUM_COMPARATORS, LSA_RET_ERR_PARAM);
    EVMA_ASSERT(instance->output_index     < EVMA_NUM_OUTPUTS    , LSA_RET_ERR_PARAM);
    EVMA_ASSERT(instance->application_timer_index == 2           , LSA_RET_ERR_PARAM);

    EVMA_ENTER();

    /*
     * 1. set the current instance to the inactive state
     */
    instance->state = EVMA_STATE_IDLE;

    /*
     * 2. try to unregister an ISR if there was one registered
     */
    evma_pnip_remove_cbf(instance->output_index, instance->pnpll_isr);

    /*
     * 3. reset the GPIO output MUX if necessary
     */
    evma_data.outputs_ref_counter[instance->output_index] -= 1;                 /* output isn't used anymore by this instance */
    if(evma_data.outputs_ref_counter[instance->output_index] == 0)
    {
        EVMA_PNIP_PLL_OUT_CONTROL_REG   reg_tmp;

        reg_tmp.reg32 = 0;
            /* disable forwarding of comparator events */
        reg_tmp.bits.PLL_CMPSelect = 0;

        EVMA_WRITE_PNIP_PLL_OUT_CONTROL(instance->output_index, reg_tmp.reg32);
    }

    /*
     * 4. check if the comparator is still used by ANY instance and mark it unused if necessary
     */
    if(instance->comparator_index != 0)                                      /* comparator 0 of application 2 is managed by EDDP */
    {
        LSA_UINT8 comparator_instance_index;

        /* check if the comparator is still used by ANY PNPLL instance */
        for(comparator_instance_index = 0; comparator_instance_index < EVMA_MAX_NUM_INSTANCES_PER_COMPARATOR; comparator_instance_index++)
        {
            if(evma_data.evma_instance_descriptor[instance->comparator_index][comparator_instance_index].state == EVMA_STATE_IN_USE)
            {
                break;
            }
        }
        if(comparator_instance_index >= EVMA_MAX_NUM_INSTANCES_PER_COMPARATOR)
        {
            /* comparator not used anymore -> free the resource */
            evma_data.used_comparator_map &= ~(1U << instance->comparator_index); /* dealloc comparator */
        }
    }

    EVMA_EXIT();

    return LSA_RET_OK;
}


#endif  /* EVMA_USE_PNIP */


/* -------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------------- */
/* ************************* general interface of the event manager *************************** */
/* -------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------------- */

/**
 * @brief initialization of the EVMA component
 */
PNIO_VOID evma_init(PNIO_VOID)
{
#ifdef EVMA_USE_PNIP
    evma_pnip_init();
#endif
}

/**
 * @brief allocates an application timer comparator event.
 *
 * EDDP care for the adjustment of the cycle time according to the Profinet cycle.
 *
 * EDDP      : - the application timer 2 of PNIP is used, and set its trigger threshold.
 *             - the application timer 2 is updated between the starting and finishing call of EDDP_SIGNAL_SENDCLOCK_CHANGE()
 *
 * @param alloc_params       : structure that determines the purpose of the event to be created.
 *                            (trigger ISR / generate GPIO signal / trigger GDMA / ...)
 *                             Available options depend on whether you have an Ertec200p device
 *                             (see structure definition).
 * @param trigger_time_ns    : specifies the comparator time in ns
 * @return LSA_NULL is returned on failure. Else an instance handle is returned.
 */
evma_handle_t evma_alloc_event(evma_alloc_params_t *alloc_params, LSA_UINT32 trigger_time_ns)
{
#ifdef EVMA_USE_PNIP
    return evma_pnip_alloc_event(alloc_params, trigger_time_ns);
#endif
}

/**
 * @brief On Ertec200p only: allocates an instance of the TransferEnd comparator, that is managed by the EDDP
 *                           (first comparator of application timer 2).
 *
 * @param alloc_params       : structure that determines the purpose of the event to be created
 *                             (trigger ISR / generate GPIO signal / trigger GDMA / ...). In addition you have to pass
 *                             specific parameters for the event type that you chose
 * @return LSA_NULL is returned on failure. Else an instance handle is returned.
 */
evma_handle_t evma_alloc_event_transfer_end( evma_alloc_params_t *alloc_params )
{
#ifdef EVMA_USE_PNIP
    return evma_pnip_alloc_event_transfer_end( alloc_params );
#endif
}

/**
 * @brief   deletes an event, that has been allocated by evma_alloc_event() or evma_alloc_event_transfer_end()
 *          and frees internal + hardware ressources.
 *
 * @param evma_handle   : handle, that was returned during the creation of the event
 * @return LSA_RET_OK / LSA_RET_ERR_PARAM
 */
LSA_UINT8 evma_free_event(evma_handle_t evma_handle)
{
#ifdef EVMA_USE_PNIP
    return evma_pnip_free_event(evma_handle);
#endif
}

/////////////////////////////////////////////////////////////////////
///LSA function for retrieving the version info of the EVMA component
/////////////////////////////////////////////////////////////////////
LSA_UINT16 evma_version(LSA_UINT16 version_len, evma_version_info_ptr_t version_ptr)
{
    if ( sizeof( LSA_VERSION_TYPE) <= version_len )
    {
        LSA_UINT32 i;
        LSA_UINT8  tmp_prefix[LSA_PREFIX_SIZE] = EVMA_LSA_PREFIX;

        version_ptr->lsa_component_id = EVMA_LSA_COMPONENT_ID;

        for ( i = 0; LSA_PREFIX_SIZE > i; i++ )
        {
            version_ptr->lsa_prefix[i] = tmp_prefix[i];
        }

        version_ptr->kind                = EVMA_KIND;
        version_ptr->version             = EVMA_VERSION;
        version_ptr->distribution        = EVMA_DISTRIBUTION;
        version_ptr->fix                 = EVMA_FIX;
        version_ptr->hotfix              = EVMA_HOTFIX;
        version_ptr->project_number      = EVMA_PROJECT_NUMBER;
        version_ptr->increment           = EVMA_INCREMENT;
        version_ptr->integration_counter = EVMA_INTEGRATION_COUNTER;
        version_ptr->gen_counter         = EVMA_GEN_COUNTER;

        return( 0);
    }
    else
    {
        return( version_len);
    }
}

/* ------------------------------------------------------------------------------ */

static LSA_FATAL_ERROR_TYPE evma_error_descriptor;

LSA_VOID evma_in_fatal_error(LSA_UINT8 error_module, LSA_UINT16 error_line, LSA_UINT16 error_code)
{
    evma_error_descriptor.lsa_component_id = PNIO_PACKID_EVMA;

    evma_error_descriptor.module_id        = error_module;
    evma_error_descriptor.line             = error_line;

    evma_error_descriptor.error_code[0]    = error_code;
    evma_error_descriptor.error_code[1]    =
    evma_error_descriptor.error_code[2]    =
    evma_error_descriptor.error_code[3]    = 0;

    evma_error_descriptor.error_data_length = sizeof(evma_data);
    evma_error_descriptor.error_data_ptr    = &evma_data;

    EVMA_FATAL_ERROR(&evma_error_descriptor);
}

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
