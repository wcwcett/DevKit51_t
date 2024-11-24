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
/*  F i l e               &F: pndv_pp.c                                 :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Functions that process configuration and parameterization                */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/* contents:


*/
/*****************************************************************************/
/* 2do:


*/
/*****************************************************************************/
/* include hierarchy */

#include "pndv_inc.h"

#define PNDV_MODULE PNDV_ERR_MODULE_PP

/**
 * @file pndv_pp.c
 * @brief pull and plug of submodule related stuff
 * @author cn3dit09
 *
 */

/** @brief Methode for handling submodule resource states.
 *
 *  @param[in]  res_ptr pointer to a submodule resource
 *  @param[in]  event   event that is send to the statemachine
 *
 *  According to the manner of object oriented programming this function is
 *  commonly a method that handles the given res_ptr as a pointer to a
 *  submodule resource object.
 *  The state of the submodules state machine is evaluated and the given event
 *  is handled in the appropriate case.
 *
 *  To get a graphical overview of the state flow you may refer to
 *  <A HREF="file:\\R:\Firmware\pndv.cmp\doc\PNDV_V2\pndv_sm_pp.pdf">pndv_sm_pp.pdf (absolut local)</A>.<br>
 *  If the link is not reachable you can find a copy in the components doc
 *  directory.
 *
 *  For a detailed description of the states please refer to @ref PNDV_CFG_SUBMODULE_RES_STATE_E
 *
 *  For a detailed description of the events please refer to @ref PNDV_CFG_SUBMODULE_RES_EVENT_E
 *
 */
PNIO_VOID pndv_pp_sm(PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T res_ptr, PNDV_CFG_SUBMODULE_RES_EVENT_T event)
{
    PNDV_ASSERT(res_ptr != LSA_NULL);
    PNDV_CFG_SUBMODULE_RES_STATE_T old_state = res_ptr->res_state;

    switch (res_ptr->res_state)
    {
        case PNDV_CFG_SUBMODULE_RES_STATE_FREE:
        {
            switch (event)
            {
                case PNDV_CFG_SUBMODULE_RES_EVENT_PERI_ADD_REQ:
                {
                    // the submodule descriped by this resource was just indicated to be plugged
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_ADD;

                    /* activity */

                    pndv_in_write_debug_buffer_3__(PNDV_DC_COMMON_MODUL_INFO,
                                                   res_ptr->sub_module.slot_nr,
                                                   res_ptr->sub_module.subslot_nr,
                                                   ((PNIO_UINT32)res_ptr->sub_module.mod_ident)|((PNIO_UINT32)res_ptr->sub_module.sub_ident)<<16);
                    break;
                }
                default:
                {
                    // unexpected event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_CFG_SUBMODULE_RES_STATE_W_ADD:
        {
            switch (event)
            {
                case PNDV_CFG_SUBMODULE_RES_EVENT_ADD_REQ:
                {
                    PNIO_UINT32 elem_cnt;
                    // this resource is to be filled into the request block of the submodule add now
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_OWN;

                    /* activity */
                    if (pndv_pp_tool_check_sub_is_local_pd(res_ptr))
                    {
                        /* handle pd submodule by ignoring them */
                        /* simply ignore the plug and quit to peri without calling cm,
                         * this is only allowed during startup and needed for referencing the submodule
                         */

                        if (pndv_data.sm != PNDV_SM_W_ADD_IM_SUBMODULES_DONE)
                        {
                            //not in startup state
                            pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.sm);
                        }

                    }
                    else
                    {
                        /* handle common submodule */
                        elem_cnt = pndv_data.rqb.sub_add_rem.args.sv.submod_add->nr_of_elems;
                        if (elem_cnt < PNDV_MAX_SV_ENTITY)
                        {
                            // copy submodule information
                                /*lint --e(661,662) possible out of bounds pointer */
                            pndv_data.rqb.sub_add_rem.args.sv.submod_add->elem[elem_cnt] = res_ptr->sub_module;
                            // increase count of used elements
                            pndv_data.rqb.sub_add_rem.args.sv.submod_add->nr_of_elems++;
                        }
                        else
                        {
                            pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                        }
                    }
                    res_ptr->submodule_state = PNDV_SUBMODULE_STATE_NO_OWNER;

                    break;
                }
                default:
                {
                    // unexpected event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }

        case PNDV_CFG_SUBMODULE_RES_STATE_W_OWN:
        {
            switch (event)
            {
                case PNDV_CFG_SUBMODULE_RES_EVENT_CM_OWN_IND:
                {
                    // this submodule is becoming owned by an ar, prm phase is starting
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_PRM_END;
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_PERI_REM_REQ:
                {
                    // this submodule is requested to be removed from cm

                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_REM;

                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_R1_PASSIV_OWNER:
                {
                    // ownership for a module in a backup r1 ar

                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_OWN_PASSIV;

                    break;
                }
                default:
                {
                    // unexpected event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_CFG_SUBMODULE_RES_STATE_W_OWN_PASSIV:
        {
            switch (event)
            {
                case PNDV_CFG_SUBMODULE_RES_EVENT_CHANGE_OWNER:
                {
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_IN_DATA;
                    break;
                }

                case PNDV_CFG_SUBMODULE_RES_EVENT_RELEASE:
                {
                    //< owner ar disconnected during prm phase, release ownership
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_OWN;
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_PERI_REM_REQ:
                {
                    // this submodule is requested to be removed from cm
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_REM;

                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_R1_PASSIV_OWNER:          /* we are already in our destination state ... */
                case PNDV_CFG_SUBMODULE_RES_EVENT_CM_OWN_IND:               /* primary->backup during ownership. We keep the W_OWN_PASSIVE state */
                case PNDV_CFG_SUBMODULE_RES_EVENT_CM_PRM_END_IND:           /* use case: primary->backup during PRMend. We stay backup and keep W_OWN_PASSIVE state */
                case PNDV_CFG_SUBMODULE_RES_EVENT_PERI_IN_DATA_IND_DONE:    /* primary->backup while processing INDATA. We keep the W_OWN_PASSIVE state */
                                                                            /*   -> W_OWN_PASSIVE is important for a future backup->primary transition as it's the */
                                                                            /*      precondition for a switch to INDATA */
                {
                    // do nothing
                    break;
                }
                default:
                {
                    // unexpected event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_CFG_SUBMODULE_RES_STATE_W_PRM_END:
        {
            switch (event)
            {
                case PNDV_CFG_SUBMODULE_RES_EVENT_CM_OWN_IND:
                {
                    //< reapted own ind, do nothing
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_CM_PRM_END_IND:
                {
                    //< prm phase of this submodules ends, in data should be next
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_IN_DATA;
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_RELEASE:
                {
                    //< owner ar disconnected during prm phase, release ownership
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_OWN;
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_PERI_REM_REQ:
                {
                    // this submodule is requested to be removed from cm
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_REM;

                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_PERI_IN_DATA_IND_DONE:
                {
                    //< a new prm sequence interrupted the last in_data processing
                    //< this event is "late" an can be ignored

                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_CHANGE_OWNER:
                {
                    // do nothing
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_R1_PASSIV_OWNER:
                {
                    // passive ownership for a module in a backup r1 ar

                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_OWN_PASSIV;

                    break;
                }
                default:
                {
                    // unexpected event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_CFG_SUBMODULE_RES_STATE_W_IN_DATA:
        {
            switch (event)
            {
                case PNDV_CFG_SUBMODULE_RES_EVENT_CM_IN_DATA_IND:
                {
                    //< submodule received in data indication, now wait for peri response
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_IN_DATA_DONE;
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_RELEASE:
                {
                    //< owner ar disconnected while waiting for in-data, release ownership
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_OWN;
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_PERI_REM_REQ:
                {
                    // this submodule is requested to be removed from cm

                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_REM;

                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_CM_OWN_IND:
                {
                    // this submodule is waiting for in data but gets a parameter update before that
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_PRM_END;
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_CHANGE_OWNER:
                {
                    // do nothing
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_R1_PASSIV_OWNER:
                {
                    // passive ownership for a module in a backup r1 ar

                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_OWN_PASSIV;

                    break;
                }
                default:
                {
                    // unexpected event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }

        case PNDV_CFG_SUBMODULE_RES_STATE_W_IN_DATA_DONE:
        {
            switch (event)
            {
                case PNDV_CFG_SUBMODULE_RES_EVENT_PERI_IN_DATA_IND_DONE:
                {
                    //< submodule now is in data
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_IN_DATA;
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_RELEASE:
                {
                    //< owner ar disconnected while waiting for in-data, release ownership
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_OWN;
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_PERI_REM_REQ:
                {
                    // this submodule is requested to be removed from cm

                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_REM;

                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_CM_OWN_IND:
                {
                    // this submodule is processing in data but gets a parameter update
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_PRM_END;
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_CHANGE_OWNER:
                {
                    // do nothing
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_R1_PASSIV_OWNER:
                {
                    // passive ownership for a module in a backup r1 ar

                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_OWN_PASSIV;

                    break;
                }
                default:
                {
                    // unexpected event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }

        case PNDV_CFG_SUBMODULE_RES_STATE_IN_DATA:
        {
            switch (event)
            {
                case PNDV_CFG_SUBMODULE_RES_EVENT_RELEASE:
                {
                    //< owner ar disconnected while being in-data, release ownership
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_OWN;
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_PERI_REM_REQ:
                {
                    // this submodule is requested to be removed from cm

                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_REM;

                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_CM_OWN_IND:
                {
                    // this submodule is in data but gets a parameter update
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_PRM_END;
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_CHANGE_OWNER:
                {
                    // do nothing
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_CM_IN_DATA_IND:
                case PNDV_CFG_SUBMODULE_RES_EVENT_PERI_IN_DATA_IND_DONE:
                {
                    /* temp workaround until finally resolved
                     * in special case of system redundance you can have the
                     * following state:
                     *  - First AR established and in Data.
                     *  - Second AR comming up and in State prm-end done / appl ready
                     *    No modules indicated in prm-end due to not owning them
                     *  - No primary endge until now
                     *
                     * Now the First AR goes offline. (Owner changes internaly)
                     * Then the second AR gets in-data and indicates all modules (again).
                     *
                     * Sending a RELEASE when the first ar goes offline doesn't solve the
                     * problem as there is no new prm sequence befor the in-data.
                     *
                     */

                    // do nothing
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_R1_PASSIV_OWNER:
                {
                    // passive ownership for a module in a backup r1 ar

                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_W_OWN_PASSIV;

                    break;
                }
                default:
                {
                    // unexpected event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }

        case PNDV_CFG_SUBMODULE_RES_STATE_W_REM:
        {
            switch (event)
            {
                case PNDV_CFG_SUBMODULE_RES_EVENT_REM_REQ:
                {
                    PNIO_UINT32 elem_cnt;
                    // this resource is to be filled into the request block of the submodule remove now
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_REM_RUN;

                    /* activity */
                    elem_cnt = pndv_data.rqb.sub_add_rem.args.sv.submod_remove->nr_of_elems;
                    // copy submodule information
                    pndv_data.rqb.sub_add_rem.args.sv.submod_remove->elem[elem_cnt].api         = res_ptr->sub_module.api;
                    pndv_data.rqb.sub_add_rem.args.sv.submod_remove->elem[elem_cnt].slot_nr     = res_ptr->sub_module.slot_nr;
                    pndv_data.rqb.sub_add_rem.args.sv.submod_remove->elem[elem_cnt].subslot_nr  = res_ptr->sub_module.subslot_nr;

                    // increase count of used elements
                    pndv_data.rqb.sub_add_rem.args.sv.submod_remove->nr_of_elems++;

                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_CM_OWN_IND:
                case PNDV_CFG_SUBMODULE_RES_EVENT_CM_PRM_END_IND:
                case PNDV_CFG_SUBMODULE_RES_EVENT_CM_IN_DATA_IND:
                case PNDV_CFG_SUBMODULE_RES_EVENT_PERI_IN_DATA_IND_DONE:
                case PNDV_CFG_SUBMODULE_RES_EVENT_CHANGE_OWNER:
                case PNDV_CFG_SUBMODULE_RES_EVENT_R1_PASSIV_OWNER:
                {
                    //this module is about to be removed, ignore this events
                    break;
                }
                default:
                {
                    // unexpected event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_CFG_SUBMODULE_RES_STATE_REM_RUN:
        {
            switch (event)
            {
                case PNDV_CFG_SUBMODULE_RES_EVENT_REM_CM_DONE:
                {
                    // removing is done, this resource is free now
                    res_ptr->res_state = PNDV_CFG_SUBMODULE_RES_STATE_FREE;

                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_CM_OWN_IND:
                case PNDV_CFG_SUBMODULE_RES_EVENT_CM_PRM_END_IND:
                case PNDV_CFG_SUBMODULE_RES_EVENT_CM_IN_DATA_IND:
                case PNDV_CFG_SUBMODULE_RES_EVENT_PERI_IN_DATA_IND_DONE:
                case PNDV_CFG_SUBMODULE_RES_EVENT_CHANGE_OWNER:
                case PNDV_CFG_SUBMODULE_RES_EVENT_R1_PASSIV_OWNER:
                {
                    //this module is about to be removed, ignore this events
                    break;
                }
                case PNDV_CFG_SUBMODULE_RES_EVENT_PERI_REM_REQ:
                default:
                {
                    // unexpected event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_CFG_SUBMODULE_RES_STATE_UNKOWN:
        default:
        {
            // unknown state
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, res_ptr->res_state);
            break;
        }
    }

    pndv_in_write_debug_buffer_3__(PNDV_DC_PNDV_PP_SM, (PNIO_UINT16)event,  (PNIO_UINT16)(((PNIO_UINT16)old_state) | (((PNIO_UINT16)res_ptr->res_state)<<8)) , (PNIO_UINT32)res_ptr);
}

/**
 *  @brief peri indicates a pulled submodule
 *
 *  @param[in]  entity Index of a submodule entity within interface
 *  @param[in]  more_follows PNIO_TRUE = more pulls will follow, PNIO_FALSE = this is the last pulled submodule
 *
 *  long_description
 *
 */
PNIO_VOID pndv_pp_peri_submodule_remove(PNIO_UINT16 entity, PNIO_BOOL more_follows)
{
    PNIO_UINT16 slot_nr;
    PNIO_UINT16 subslot_nr;
    PNIO_UINT32 subslot_error;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    if (pndv_data.iface_ptr->real_cfg[entity].elem.state != PNDV_IFACE_SERVICE_NEW)
    {
        // this request is erroneous, interface state is wrong
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.iface_ptr->real_cfg[entity].elem.state );
    }

    slot_nr = pndv_data.iface_ptr->real_cfg[entity].elem.sub_descr.slot_nr;
    subslot_nr = pndv_data.iface_ptr->real_cfg[entity].elem.sub_descr.subslot_nr;

    pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
    if(subslot_error)
    {
        // slot/subslot not possible, must be an implementation error
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    if(local_sub_res == 0)
    {
        // submodule not plugged, must be an error here
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, (PNIO_UINT32)local_sub_res);
    }

    if (local_sub_res->entity != entity)
    {
        // ERROR! the entity of this submodule has changed since it was added
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, local_sub_res->entity );
    }

#ifdef PNDV_CFG_DISABLE_SOL_BY_PDEV
    /* check if head was pulled (happens in R1 redundancy) and reset SO-locked marker of head submodule */
    if (  ( (slot_nr == PNDV_IM_SLOT_NO) || (slot_nr == PNDV_IM_SLOT_NO_PARTNER)) &&((subslot_nr & 0xF000) == 0x8000) )
    {
        PNIO_UINT8 ar_idx;
        for(ar_idx = 0; ar_idx <= PNDV_CM_AR_NO ; ar_idx++)
        {
            pndv_data.ar[ar_idx].pdev_8000_locked = PNIO_FALSE;
        }
    }
    if (  ( (slot_nr == PNDV_IM_SLOT_NO) || (slot_nr == PNDV_IM_SLOT_NO_PARTNER)) &&((subslot_nr & 0xF000) == 0x9000) )
    {
        PNIO_UINT8 ar_idx;
        for(ar_idx = 0; ar_idx <= PNDV_CM_AR_NO ; ar_idx++)
        {
            pndv_data.ar[ar_idx].pdev_9000_locked = PNIO_FALSE;
        }
    }
#endif

    pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_PERI_REM_REQ);

    // remove resource from in_use list
    PNDV_LIST_REMOVE_ENTRY(&local_sub_res->link);

    // put resource to wait remove list
    PNDV_LIST_INSERT_TAIL(&pndv_data.cfg.peri_cfg.cm_add_rem_wait_list, &local_sub_res->link);

    if (pndv_pp_peri_check_trigger_cm_add_remove( more_follows ) )
    {
        pndv_pp_tool_submodule_add_remove_do_request();
    }
}

/**
 *  @brief removing a submodule from cm has been done
 *
 *  @param[in]  rqb_ptr pointer to request block
 *
 *  long_description
 *
 */
PNIO_VOID pndv_pp_cm_submodule_remove_done(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    if(PNDV_RQB_GET_OPCODE(rqb_ptr) != CM_OPC_SV_SUBMODULE_REMOVE)
    {
        // only confirmations of submodule remove requests can be handled here
        pndv_in_fatal_error( PNDV_MODULE, __LINE__,  PNDV_RQB_GET_OPCODE(rqb_ptr));
    }

    switch (PNDV_RQB_GET_RESPONSE(rqb_ptr))
    {
        case CM_OK:
        {
            // OK
            break;
        }
        // more evaluations may follow later
        case CM_ERR_SEQUENCE:
        case CM_ERR_ELEM:
        case CM_ERR_PARAM:
        case CM_ERR_OWNED:
        default:
        {
            // all other responses lead to an error
            pndv_in_fatal_error( PNDV_MODULE, __LINE__,  PNDV_RQB_GET_RESPONSE(rqb_ptr));
            break;
        }
    };

    // no error so far, now the responses to peri are beeing prepared

    for (PNDV_LIST_EACH(local_sub_res, &pndv_data.cfg.peri_cfg.cm_rem_list, PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T))
    {
        PNIO_UINT16 slot_nr;
        PNIO_UINT16 subslot_nr;
        PNDV_IFACE_CMD_ENTRY_T tmp_event;
        PNIO_UINT32 subslot_error = 1;

        slot_nr = local_sub_res->sub_module.slot_nr;
        subslot_nr = local_sub_res->sub_module.subslot_nr;

        // trigger sm for every element of the rem list
        pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_REM_CM_DONE);

        // set interface state 
        pndv_data.iface_ptr->real_cfg[local_sub_res->entity].elem.state = PNDV_IFACE_SERVICE_PROCCESSING;
        // check for last element
        if( PNDV_LIST_NEXT(&pndv_data.cfg.peri_cfg.cm_rem_list, &local_sub_res->link, PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T))
        {
            //* not the last one
            tmp_event.cmd = PNDV_EV_TO_PERI_PULL_AL_MORE_FOLLOWS_QUIT;
        }
        else
        {
            // last element, this finishes the sequence
            tmp_event.cmd = PNDV_EV_TO_PERI_PULL_AL_QUIT;
        }

        // reset diagnosis counter for this subslot
        pndv_data.diag.ch_diag_to_cm_count[local_sub_res->entity] = 0;

        // clear unreported diagnosis
        pndv_in_al_search_clear_diag( slot_nr, subslot_nr, PNDV_DIAG_REQ_NISCHT);
        pndv_al_search_clear_rosal(slot_nr, subslot_nr);

        tmp_event.add_1 = 0;
        tmp_event.add_2 = local_sub_res->entity;
        pndv_in_peri_write_coupl_event( tmp_event );

        // unbind this resource from its submodule
        pndv_unlock_submod_resource(local_sub_res ,slot_nr, subslot_nr, &subslot_error);

        // init entity ressource data (so that there's no chance to interpret seemingly valid data if ressource is used again later)
        pndv_in_init_local_sub_res(local_sub_res);

        if(subslot_error)
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }
    }

    // append the whole rem list to the free list
    PNDV_LIST_APPEND(&pndv_data.cfg.peri_cfg.free_list, &pndv_data.cfg.peri_cfg.cm_rem_list);

    // work is done, free the rqb resource
    PNDV_RQB_SET_OPCODE( rqb_ptr, 0);

    if (!PNDV_LIST_IS_EMPTY(&pndv_data.cfg.peri_cfg.cm_add_rem_wait_list))
    {
        // there are more pulls / plugs waiting to be sent to CM -> continue sending them now
        pndv_pp_tool_submodule_add_remove_do_request();
    }
}

/**
 * @brief   function checks the pull/plug waiting queue and tells if a pull/plug-RQB should be assembled CM / sent to CM.
 *          @see pndv_pp_tool_submodule_add_remove_do_request()
 *
 * @param[in]   more_follows    tells if the user wants to append more pulls/plugs to the list.
 */
PNIO_BOOL pndv_pp_peri_check_trigger_cm_add_remove(PNIO_BOOL more_follows)
{
    PNIO_BOOL trigger_cm_req = PNIO_FALSE;

    if( PNDV_RQB_GET_OPCODE(&pndv_data.rqb.sub_add_rem) == 0)
    {
        /* no pull/plug currently running at CM */
        if (more_follows == PNIO_FALSE)
        {
            /* user requested sending to CM (last pull/plug in a row -> forward to CM) */
            trigger_cm_req = PNIO_TRUE;
        }
        else
        {
            /* user puts in a row of pulls and plugs. Because we've just one queue and need to adhere to the pull-plug-order, we can just combine a sequence
             * of uniform pulls or a sequence of uniform plugs in one request --> check here, if we have to issue a uniform request to CM */
            PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;
            PNDV_CFG_SUBMODULE_RES_STATE_T      last_elem_res_state;

            local_sub_res = PNDV_LIST_FIRST(&pndv_data.cfg.peri_cfg.cm_add_rem_wait_list, PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T);
            if(local_sub_res)
            {
                last_elem_res_state = local_sub_res->res_state;

                for (PNDV_LIST_EACH(local_sub_res, &pndv_data.cfg.peri_cfg.cm_add_rem_wait_list, PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T))
                {
                    if( !((local_sub_res->res_state == PNDV_CFG_SUBMODULE_RES_STATE_W_REM) || (local_sub_res->res_state == PNDV_CFG_SUBMODULE_RES_STATE_W_ADD)))
                    {
                        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
                    }

                    if(local_sub_res->res_state != last_elem_res_state)
                    {
                        /* there is a pull at the end of a plug sequence or vice versa -> the first n uniform elements should be put to CM */
                        trigger_cm_req = PNIO_TRUE;
                        break;
                    }
                    last_elem_res_state = local_sub_res->res_state;
                }
            }
        }
    }
    // else: opcode not zero, the rqb is in use
    //       this means another request is on the way
    //       do nothing right now and wait for the running request to be finished

    return trigger_cm_req;
}

/**
 * @brief       function takes as much as possible submodule add- or remove-requests uniformly out of the waiting list
 *              and forwards them to CM.
 *              "Uniformly" means that adds and removes can't be mixed within the RQB.
 *              Using only one queue for pulls and plugs is important to always assure the right sequence. Otherwise
 *              module-ID inconsistencies might be possible (use case: pull+plug on a slot having 2 submodules with different module IDs)
 */
PNIO_VOID pndv_pp_tool_submodule_add_remove_do_request()
{
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    if ((PNDV_RQB_GET_OPCODE(&pndv_data.rqb.sub_add_rem) != 0) || 
         PNDV_LIST_IS_EMPTY(&pndv_data.cfg.peri_cfg.cm_add_rem_wait_list))
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    local_sub_res = PNDV_LIST_FIRST(&pndv_data.cfg.peri_cfg.cm_add_rem_wait_list, PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T);

    PNDV_ASSERT( local_sub_res != LSA_NULL );

    if (local_sub_res->res_state == PNDV_CFG_SUBMODULE_RES_STATE_W_ADD)
    {
        // prepare rqb for use
        PNDV_RQB_SET_OPCODE(&pndv_data.rqb.sub_add_rem, CM_OPC_SV_SUBMODULE_ADD);
        PNDV_RQB_SET_HANDLE(&pndv_data.rqb.sub_add_rem, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
        pndv_data.rqb.sub_add_rem.args.sv.submod_add              = (CM_UPPER_SV_SUBMODULE_ADD_PTR_TYPE) &pndv_data.rqb.sub_add_args;
        pndv_data.rqb.sub_add_rem.args.sv.submod_add->device_nr   = PNDV_CM_DEVICE_NO;
        pndv_data.rqb.sub_add_rem.args.sv.submod_add->nr_of_elems = 0;    //< initially, no elements are attached

        while (!PNDV_LIST_IS_EMPTY(&pndv_data.cfg.peri_cfg.cm_add_rem_wait_list))
        {
            local_sub_res = PNDV_LIST_FIRST(&pndv_data.cfg.peri_cfg.cm_add_rem_wait_list, PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T);
            if (local_sub_res == LSA_NULL)
            {
                break;
            }

            if (local_sub_res->res_state == PNDV_CFG_SUBMODULE_RES_STATE_W_ADD)
            {
                // fill RQB. Trigger sm for every element of the wait list
                pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_ADD_REQ);

                PNDV_LIST_REMOVE_ENTRY(&local_sub_res->link);
                PNDV_LIST_INSERT_TAIL(&pndv_data.cfg.peri_cfg.cm_add_list, &local_sub_res->link);
            }
            else
            {
                break;
            }
        }
        // all submodules are filled into the rqb now
    }
    else if (local_sub_res->res_state == PNDV_CFG_SUBMODULE_RES_STATE_W_REM)
    {
        PNDV_RQB_SET_OPCODE(&pndv_data.rqb.sub_add_rem, CM_OPC_SV_SUBMODULE_REMOVE);
        PNDV_RQB_SET_HANDLE(&pndv_data.rqb.sub_add_rem, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
        pndv_data.rqb.sub_add_rem.args.sv.submod_remove              = (CM_UPPER_SV_SUBMODULE_REMOVE_PTR_TYPE) &pndv_data.rqb.sub_rem_args;
        pndv_data.rqb.sub_add_rem.args.sv.submod_remove->device_nr   = PNDV_CM_DEVICE_NO;
        pndv_data.rqb.sub_add_rem.args.sv.submod_remove->nr_of_elems = 0;    //< initially, no elements are attached

        while (!PNDV_LIST_IS_EMPTY(&pndv_data.cfg.peri_cfg.cm_add_rem_wait_list))
        {
            local_sub_res = PNDV_LIST_FIRST(&pndv_data.cfg.peri_cfg.cm_add_rem_wait_list, PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T);
            if (local_sub_res == LSA_NULL)
            {
                break;
            }

            if (local_sub_res->res_state == PNDV_CFG_SUBMODULE_RES_STATE_W_REM)
            {
                // fill RQB. Trigger sm for every element of the wait list
                pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_REM_REQ);

                PNDV_LIST_REMOVE_ENTRY(&local_sub_res->link);
                PNDV_LIST_INSERT_TAIL(&pndv_data.cfg.peri_cfg.cm_rem_list, &local_sub_res->link);
            }
            else
            {
                break;
            }
        }
        // all submodules are filled into the rqb now
    }
    else
    {
        // submodules in the add_rem_wait_list should be in add-/rmv-waiting state!
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    // finaly send the add-request to cm
    PNDV_REQUEST(&pndv_data.rqb.sub_add_rem, LSA_COMP_ID_CM);
}

/**
 *  @brief peri indicates a plugged submodule
 *
 *  @param[in]  entity Index of a submodule entity within interface
 *  @param[in]  more_follows PNIO_TRUE = more plugs will follow, PNIO_FALSE = this is the last plugged submodule
 *
 *  long_description
 *
 */
PNIO_VOID pndv_pp_peri_submodule_add(PNIO_UINT16 entity, PNIO_BOOL more_follows)
{
    PNIO_UINT16 slot_nr;
    PNIO_UINT16 subslot_nr;
    PNIO_UINT32 subslot_error;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    if (pndv_data.iface_ptr->real_cfg[entity].elem.state != PNDV_IFACE_SERVICE_NEW)
    {
        // this request is erroneous, interface state is wrong
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.iface_ptr->real_cfg[entity].elem.state );
    }

    slot_nr = pndv_data.iface_ptr->real_cfg[entity].elem.sub_descr.slot_nr;
    subslot_nr = pndv_data.iface_ptr->real_cfg[entity].elem.sub_descr.subslot_nr;
    pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
    if(subslot_error)
    {
        // slot/subslot not possible, must be an implementation error
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    if(local_sub_res != 0)
    {
        // ERROR! If there is already an entry at this place, there was a plug before last pull
        // has been finished
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, (PNIO_UINT32)local_sub_res);
    }

    // get an submodul-resource from free list

    local_sub_res = PNDV_LIST_FIRST(&pndv_data.cfg.peri_cfg.free_list, PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T);
    if ( local_sub_res == LSA_NULL )
    {
        // no more free resources, this is fatal and must not happen, check pndv configuration
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0 );
    }
    // remove entry from free list
    PNDV_LIST_REMOVE_ENTRY(&local_sub_res->link);
    // bind this resource by inserting his index into ff table
    pndv_lock_submod_resource(local_sub_res ,slot_nr, subslot_nr, &subslot_error);
    if(subslot_error)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }
    // copy submodule informations from interface to resource
    local_sub_res->sub_module = pndv_data.iface_ptr->real_cfg[entity].elem.sub_descr;
    // the entity has to be valid until this submodule is pulled and the pull is confirmed to peri
    // therefor the entity can be saved and used to address this submodule inside the interface for later use
    local_sub_res->entity     = entity;
    //PNDV_COPY_BYTE(&local_sub_res->sub_module, pndv_data.iface_ptr->real_cfg.elem[entity].sub_descr, sizeof(CM_SV_SUBMODULE_ADD_ELEMENT_TYPE));

    pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_PERI_ADD_REQ);

    // resource is now waiting to be plugged
    // put to wait list
    PNDV_LIST_INSERT_TAIL(&pndv_data.cfg.peri_cfg.cm_add_rem_wait_list, &local_sub_res->link);

    // check if an RQB with as much as possible queued plugs should be sent to CM
    if (pndv_pp_peri_check_trigger_cm_add_remove( more_follows ) )
    {
        // send an RQB to CM
        pndv_pp_tool_submodule_add_remove_do_request();
    }
}

/**
 *  @brief adding a submodule to cm has been done
 *
 *  @param[in]  rqb_ptr pointer to request block
 *
 *  long_description
 *
 */
PNIO_VOID pndv_pp_cm_submodule_add_done(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    if(PNDV_RQB_GET_OPCODE(rqb_ptr) != CM_OPC_SV_SUBMODULE_ADD)
    {
        // only confirmations of submodule add requests can be handled here
        pndv_in_fatal_error( PNDV_MODULE, __LINE__,  PNDV_RQB_GET_OPCODE(rqb_ptr));
    }

    switch (PNDV_RQB_GET_RESPONSE(rqb_ptr))
    {
        case CM_OK:
        case CM_ERR_ELEM:
        {
            // OK
            break;
        }
        // more evaluations may follow later
        case CM_ERR_SEQUENCE:
        case CM_ERR_PARAM:
        case CM_ERR_OWNED:
        default:
        {
            // all other responses lead to an error
            pndv_in_fatal_error( PNDV_MODULE, __LINE__,  PNDV_RQB_GET_RESPONSE(rqb_ptr));
            break;
        }
    };

    // no error so far, now the responses to peri are beeing prepared

    for (PNDV_LIST_EACH(local_sub_res, &pndv_data.cfg.peri_cfg.cm_add_list, PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T))
    {
        PNDV_IFACE_CMD_ENTRY_T tmp_event;

        // trigger sm for every element of the add list
        //pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_ADD_CM_DONE);

        // set interface state 
        pndv_data.iface_ptr->real_cfg[local_sub_res->entity].elem.state = PNDV_IFACE_SERVICE_PROCCESSING;
        // check for last element
        if( PNDV_LIST_NEXT(&pndv_data.cfg.peri_cfg.cm_add_list, &local_sub_res->link, PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T))
        {
            // not the last one
            tmp_event.cmd = PNDV_EV_TO_PERI_PLUG_AL_MORE_FOLLOWS_QUIT;
        }
        else
        {
            // last element, this finishes the sequence
            tmp_event.cmd = PNDV_EV_TO_PERI_PLUG_AL_QUIT;
        }
        tmp_event.add_1 = 0;
        tmp_event.add_2 = local_sub_res->entity;
        pndv_in_peri_write_coupl_event( tmp_event );
    }

    // append the whole add list to the in_use list
    PNDV_LIST_APPEND(&pndv_data.cfg.peri_cfg.in_use_list, &pndv_data.cfg.peri_cfg.cm_add_list);

    // check if this is the first add after starting pndv (this has to plug im subslots)
    if (pndv_data.sm == PNDV_SM_W_ADD_IM_SUBMODULES_DONE)
    {
        //startup detected, plug is done -> trigger the sm
        pndv_sm(PNDV_SM_EVENT_ADD_IM_SUBMODULES_DONE);
    }

    // work is done, free the rqb resource
    PNDV_RQB_SET_OPCODE( rqb_ptr, 0);

    /* check for queued diagnosis */
    pndv_in_al_check_dial_continue();

    if (!PNDV_LIST_IS_EMPTY(&pndv_data.cfg.peri_cfg.cm_add_rem_wait_list))
    {
        // there are still adds or removes -> send next RQB now
        pndv_pp_tool_submodule_add_remove_do_request();
    }

}

/**
 *  @brief adding a pdev submodule to cm has been done
 *
 *  @param[in]  rqb_ptr pointer to request block
 *
 *  long_description
 *
 */
PNIO_VOID pndv_pp_cm_pd_submodule_add_done(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 tmp_int;

    if(PNDV_RQB_GET_OPCODE(rqb_ptr) != CM_OPC_PD_SUBMODULE_CONTROL)
    {
        // only confirmations of submodule add requests can be handled here
        pndv_in_fatal_error( PNDV_MODULE, __LINE__,  PNDV_RQB_GET_OPCODE(rqb_ptr));
    }

    switch (PNDV_RQB_GET_RESPONSE(rqb_ptr))
    {
        case CM_OK:
        {
            // OK
            break;
        }
        // more evaluations may follow later
        case CM_ERR_ELEM:
        case CM_ERR_SEQUENCE:
        case CM_ERR_PARAM:
        case CM_ERR_OWNED:
        default:
        {
            // all other responses lead to an error

            pndv_in_fatal_error( PNDV_MODULE, __LINE__,  PNDV_RQB_GET_RESPONSE(rqb_ptr));
            break;
        }
    };

    /* ignore for now, just free the mem */

    PNDV_FREE_MEM( &tmp_int, rqb_ptr->args.pd.submodule_control);

    if ( LSA_OK != tmp_int)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, tmp_int);
    }

    PNDV_FREE_RQB( &tmp_int, rqb_ptr);

    if ( LSA_OK != tmp_int)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, tmp_int);
    }

    /* this service is not used at the moment and should not be called */
    pndv_in_fatal_error( PNDV_MODULE, __LINE__,  PNDV_RQB_GET_OPCODE(rqb_ptr));

}


PNIO_UINT32 pndv_pp_tool_check_sub_is_local_pd(PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T res_ptr)
{
    PNIO_UINT32 ret_val = PNIO_FALSE;

    if (res_ptr->sub_module.slot_nr == PNDV_IM_SLOT_NO)
    {
        PNIO_UINT16 local_subslot_nr = 0x8000;
        if (((res_ptr->sub_module.subslot_nr & 0xff00)>>8) == (local_subslot_nr >> 8))
        {
            ret_val = PNIO_TRUE;
        }
    }

    return ret_val;
}

/*****************************************************************************/
/*  end of file.                                                             */
/*****************************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
