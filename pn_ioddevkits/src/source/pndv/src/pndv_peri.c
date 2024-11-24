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
/*  F i l e               &F: pndv_peri.c                               :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Profinet device (cm-user) Interface to the peripheral interface          */
/*                                                                           */
/*****************************************************************************/



/*****************************************************************************/
/* contents:

    - pndv_in_peri_init

    - pndv_in_peri_check_event
    - pndv_in_peri_read_coupl_event
    - pndv_in_peri_write_coupl_event
    - pndv_in_peri_put_ds_to_interface
    - pndv_in_peri_dial_quit
    - pndv_in_peri_pral_quit
    - pndv_in_peri_upal_quit
    - pndv_in_peri_ros_al_quit

    - pndv_peri_read_write_record

*/
/*****************************************************************************/
/* 2do:


*/
/*****************************************************************************/
/* include hierarchy */

#include "pndv_inc.h"

#define PNDV_MODULE PNDV_ERR_MODULE_PERI

/* extern Dual-Port Ram  */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**********                                                         **********/
/**********                   INTERNAL MACROS                       **********/
/**********                                                         **********/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#define swap_pointer__(_ptr1, _ptr2)                                    \
{                                                                       \
    PNIO_UINT16 tmp = (_ptr1);                                          \
                                                                        \
    (_ptr1) = (_ptr2);                                                  \
                                                                        \
    (_ptr2) = tmp;                                                      \
                                                                        \
}                                                                       \


#define swap_length__(_len1, _len2)                                     \
{                                                                       \
    PNIO_UINT16 tmp = (_len1);                                          \
                                                                        \
    (_len1) = (_len2);                                                  \
                                                                        \
    (_len2) = tmp;                                                      \
                                                                        \
}                                                                       \

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**********                                                         **********/
/**********                   FUNCTIONS                             **********/
/**********                                                         **********/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/**
 *  @brief initialize peri interface
 *
 *  Set all states and data elements of the coupling interface to initial state.
 *
 */
PNIO_VOID pndv_in_peri_init (PNIO_VOID)
{
    PNIO_UINT16 count;
    PNIO_UINT32 ar_cnt,elem_cnt;

    /* ---- INIT PNDV-PERI COUPLING DATA ---- */

    PNDV_MEMSET((PNIO_VOID*)pndv_data.iface_ptr, 0, sizeof(PNDV_IFACE_STRUCT));

    for ( count = 0; count < (PNDV_MAX_ARS_RT + PNDV_MAX_ARS_DA) ; count++ )
    {
        pndv_data.iface_ptr->ar[count].current_rt_class           = PNDV_RT_NONE;
        pndv_data.iface_ptr->ar[count].previous_rt_class          = PNDV_RT_NONE;
        pndv_data.iface_ptr->ar[count].red_int_info_used          = PNIO_FALSE;
    }

    //pndv_data.iface_ptr->real_cfg.state         = PNDV_IFACE_SERVICE_IDLE;
    pndv_data.iface_ptr->ds_rw.state            = PNDV_IFACE_SERVICE_IDLE;
    //pndv_data.iface_ptr->real_cfg.critical_slot = 0;

    for ( count = 0; count <= PNDV_MAX_SV_ENTITY ; count++ )
    {
        pndv_data.iface_ptr->ps.lost_state[count]            = PNDV_IFACE_SERVICE_IDLE;
        pndv_data.iface_ptr->ps.return_state[count]          = PNDV_IFACE_SERVICE_IDLE;
    }

    //init the generic diag interface ressources
    for ( count = 0; count != PNDV_MAX_GENERIC_DIAG_NUMBER ; count++ )
    {
        //never change the index value !
        pndv_data.iface_ptr->generic_diag_data[ count ].interface_index = count;
        pndv_data.iface_ptr->generic_diag_data[ count ].state = PNDV_IFACE_SERVICE_IDLE;
    }

    /* initialize real_cfg area of interface */
    for (elem_cnt = 0; elem_cnt < PNDV_MAX_SV_ENTITY; elem_cnt++)
    {
        pndv_data.iface_ptr->real_cfg[elem_cnt].elem.state  = PNDV_IFACE_SERVICE_IDLE;
        pndv_data.iface_ptr->real_cfg[elem_cnt].dial.state  = PNDV_IFACE_SERVICE_IDLE;
        pndv_data.iface_ptr->real_cfg[elem_cnt].xdial.state = PNDV_IFACE_SERVICE_IDLE;
        pndv_data.iface_ptr->real_cfg[elem_cnt].pral.state  = PNDV_IFACE_SERVICE_IDLE;
        pndv_data.iface_ptr->real_cfg[elem_cnt].pral.info.usi          = PNDV_AL_USI_PROCESSALARM;
        pndv_data.iface_ptr->real_cfg[elem_cnt].pral.info.data_length  = 4; /* default pral len */
        pndv_data.iface_ptr->real_cfg[elem_cnt].upal.state  = PNDV_IFACE_SERVICE_IDLE;
        pndv_data.iface_ptr->real_cfg[elem_cnt].ural.state  = PNDV_IFACE_SERVICE_IDLE;
        pndv_data.iface_ptr->real_cfg[elem_cnt].stal.state  = PNDV_IFACE_SERVICE_IDLE;
        pndv_data.iface_ptr->real_cfg[elem_cnt].ros.state   = PNDV_IFACE_SERVICE_IDLE;
    }

    /* initialize set_cfg area of interface*/
    for (ar_cnt = 0; ar_cnt < PNDV_CM_AR_NO; ar_cnt++)
    {
        for (elem_cnt = 0; elem_cnt < PNDV_CM_SV_SUBSLOT_COUNT; elem_cnt++)
        {
            pndv_data.iface_ptr->set_cfg[ar_cnt][elem_cnt].elem.cmp_result          = PNDV_CMP_RES_NO_MODULE;
            pndv_data.iface_ptr->set_cfg[ar_cnt][elem_cnt].elem.own_state           = PNDV_IFACE_SERVICE_IDLE;
            pndv_data.iface_ptr->set_cfg[ar_cnt][elem_cnt].elem.prm_end_state       = PNDV_IFACE_SERVICE_IDLE;
        }
    }


    /* ---- INIT OTHER PERI DATA ---- */

    pndv_data.rqb.store_ds_ptr_for_recopy = NIL;

}

/*****************************************************************************/

PNIO_VOID pndv_peri_sm(PNDV_SM_PERIBUS_EVENT event)
{

    switch (pndv_data.cfg.peri_cfg.sm_state)
    {

        case PNDV_SM_PERIBUS_STATE_UNKNOWN:
        {
            /* initial state, peri-bus state is not known yet */

            switch (event)
            {
                case PNDV_SM_PERIBUS_EVENT_OK:
                {
                    /* peri bus is immediately ok */

                    pndv_in_write_debug_buffer_dpr__(PNDV_DC_PERI_BUSWIEDERKEHR, pndv_data.cfg.peri_cfg.sm_state);

                    pndv_data.cfg.peri_cfg.sm_state = PNDV_SM_PERIBUS_STATE_OK;

                    pndv_ar_set_so_state(PNDV_SOL_EVENT_FLAG_PERIBUS_STATE, PNIO_FALSE, PNDV_SOL_ALL_ARS);

                    pndv_ar_tool_connect_ind_process_parked();

                    break;
                }

                case PNDV_SM_PERIBUS_EVENT_ERROR:
                {
                    /* peri-bus has failed at startup */

                    pndv_in_write_debug_buffer_dpr__(PNDV_DC_PERI_BUSAUSFALL, pndv_data.cfg.peri_cfg.sm_state);

                    pndv_data.cfg.peri_cfg.sm_state = PNDV_SM_PERIBUS_STATE_NOTOK;

                    pndv_ar_set_so_state(PNDV_SOL_EVENT_FLAG_PERIBUS_STATE, PNIO_TRUE, PNDV_SOL_ALL_ARS);

                    pndv_ar_tool_connect_ind_process_parked();

                    break;
                }

                default:
                {
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }

            break;
        }/* PNDV_SM_PERIBUS_STATE_UNKNOWN */

        case PNDV_SM_PERIBUS_STATE_OK:
        {
            /* this is the "good" state */
            switch (event)
            {
                case PNDV_SM_PERIBUS_EVENT_ERROR:
                {
                    PNIO_UINT32 tmp_ret_val;

                    pndv_in_write_debug_buffer_dpr__(PNDV_DC_PERI_BUSAUSFALL, pndv_data.cfg.peri_cfg.sm_state);

                    pndv_data.cfg.peri_cfg.sm_state = PNDV_SM_PERIBUS_STATE_NOTOK;

                    /* In the event of an S-Bus failure, all user data ARs are canceled */
                    /* if SO_LOCKED_STATE is true, the user data ARs remain */
                    tmp_ret_val = pndv_ar_set_so_state(PNDV_SOL_EVENT_FLAG_PERIBUS_STATE, PNIO_TRUE, PNDV_SOL_ALL_ARS);

                    /* there is a user data AR -> first ar-abort -> when finished then pull modules 
                       so there are no pull alarms to the controller */

                    if ( PNDV_OK != tmp_ret_val )
                    {
                        /* waiting for abort_done */
                    }

                    pndv_ar_tool_connect_ind_process_parked();

                    break;
                }
                case PNDV_SM_PERIBUS_EVENT_OK:
                {
                    /* do nothing */

                    break;
                }

                default:
                {
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }

            break;
        }/* PNDV_SM_PERIBUS_STATE_OK */

        case PNDV_SM_PERIBUS_STATE_NOTOK:
        {
            /* this is the "bad" state at normal work */
            switch (event)
            {
                case PNDV_SM_PERIBUS_EVENT_OK:
                {
                    /* peri-bus is now ok */

                    pndv_in_write_debug_buffer_dpr__(PNDV_DC_PERI_BUSWIEDERKEHR, pndv_data.cfg.peri_cfg.sm_state);

                    pndv_data.cfg.peri_cfg.sm_state = PNDV_SM_PERIBUS_STATE_OK;

                    pndv_ar_set_so_state(PNDV_SOL_EVENT_FLAG_PERIBUS_STATE, PNIO_FALSE, PNDV_SOL_ALL_ARS);

                    // pndv_in_al_clear_modul_error(); no longer a task of this sm

                    pndv_ar_tool_connect_ind_process_parked();

                    break;
                }

                case PNDV_SM_PERIBUS_EVENT_ERROR:
                {
                    /* do nothing */

                    break;
                }

                default:
                {
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }

            break;
        }/* PNDV_SM_PERIBUS_STATE_NOTOK */
        case PNDV_SM_PERIBUS_STATE_NOT_USED: /* sm must not be called if peri_sm is set to be not used */
        default:
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.cfg.peri_cfg.sm_state);

            break;
        }
    }
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**********                                                         **********/
/**********                   INTERNAL FUNCTION                     **********/
/**********                                                         **********/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


/**
 *  @brief identify and execute peri commands
 *
 *  @param[in]  event event to be handled
 *
 *  Check the given event for its command and execute the requested service.
 *
 */
PNIO_VOID pndv_in_peri_check_event ( PNDV_IFACE_CMD_ENTRY_T event)
{

    //pndv_in_write_debug_buffer_all_add__(PNDV_DC_PERI_COUPLE_EVENT, 0/*read*/, event);
    //pndv_in_write_debug_buffer_3__(PNDV_DC_PERI_COUPLE_EVENT, event.cmd, event.add_1, event.add_2);

    switch (event.cmd)
    {
        case PNDV_EV_NO:
            break;

        case PNDV_EV_TO_PNDV_PNDV_START:
        {
            // pndv is signaled to start
            // add_1 = submodul count of im and virtual submodules
            // add_2 = not used
            pndv_data.cfg.peri_cfg.first_peri_sub_idx = event.add_1 + 1;
            pndv_in_start();
            //PNDV_EV_TO_PERI_PNDV_START_DONE is done by pndv_sm on reaching the state PNDV_SM_W_ADD_IM_SUBMODULES_DONE

            break;
        }

        case PNDV_EV_TO_PNDV_PNDV_STOP:
        {
            // pndv is signaled to stop
            // add_1 = not used
            // add_2 = not used
            pndv_in_stop();
            //pndv_in_peri_write_coupl_event( PNDV_EV_TO_PERI_PNDV_STOP_DONE) >> PNDV_STOP_DONE()

            break;
        }
        case PNDV_EV_TO_PNDV_PNDV_AKT:
        {
            pndv_data.device_control = CM_SV_DEVICE_CONTROL_CMD_ACTIVATE;
            pndv_in_device_control(pndv_data.device_control);
            break;
        }
        case PNDV_EV_TO_PNDV_PNDV_DEAKT:
        {
            pndv_data.device_control = CM_SV_DEVICE_CONTROL_CMD_PASSIVATE;
            pndv_in_device_control(pndv_data.device_control);
            break;
        }
        case PNDV_EV_TO_PNDV_CM_DV_AKT_IND_DONE:
        {
            //dev activate done
            //add_1 = not used
            //add_2 = not used

            //nothing to do

            break;
        }
        case PNDV_EV_TO_PNDV_CM_DV_DEAKT_IND_DONE:
        {
            //dev deactivate done
            //add_1 = not used
            //add_2 = not used

            //nothing to do

            break;
        }
        case PNDV_EV_TO_PNDV_PERI_STATE_IND:
        {
            // pndv is signaled to disbale the connect service
            // add_1 = 0|PERI not OK, 1|PERI OK
            // add_2 = not used
            PNDV_IFACE_CMD_ENTRY_T tmp_event;

            if (event.add_1 == 0)
            {
                pndv_peri_sm(PNDV_SM_PERIBUS_EVENT_ERROR);
            }
            else if (event.add_1 == 1)
            {
                pndv_peri_sm(PNDV_SM_PERIBUS_EVENT_OK);
            }

            tmp_event.cmd = PNDV_EV_TO_PERI_PERI_STATE_IND_DONE;
            tmp_event.add_1 = event.add_1;
            tmp_event.add_2 = event.add_2;
            pndv_in_peri_write_coupl_event( tmp_event );
            break;
        }



        case PNDV_EV_TO_PNDV_CONNECT_IND_DONE:
        {
            // connect ind has been processed by peri
            // add_1 = ar index
            // add_2 = response

            switch(event.add_2)
            {
                case PNDV_OK:
                {
                    pndv_ar_sm(event.add_1, PNDV_AR_SM_EVENT_CONNECT_IND_PERI_OK_DONE);
                    break;
                }
                case PNDV_ERR_APPL_RESOURCE:
                {
                    /* AR parameters violate GSD MaxApplicationXXXLength -> AR abort by CM with error code DataLength (8)*/
                    pndv_ar_sm(event.add_1, PNDV_AR_SM_EVENT_CONNECT_IND_PERI_RESOURCE_ERROR_DONE);
                    break;
                }
                default:
                {
                    /* error (out of IO buffers) -> AR abort by CM, User abort */
                    pndv_ar_sm(event.add_1, PNDV_AR_SM_EVENT_CONNECT_IND_PERI_ERROR_DONE);
                    break;
                }
            }
            break;
        }

        case PNDV_EV_TO_PNDV_OWN_IND_DONE:
        {
            // response to an ownership for one submodule (single or last of many)
            // add_1 = ar index
            // add_2 = entity_nr
            pndv_ar_peri_ownership_ind_done(event.add_1, event.add_2, LSA_FALSE);
            break;
        }

        case PNDV_EV_TO_PNDV_OWN_IND_DONE_MORE_FOLLOWS:
        {
            // response to an ownership for one submodule (more following)
            // add_1 = ar index
            // add_2 = entity_nr
            pndv_ar_peri_ownership_ind_done(event.add_1, event.add_2, LSA_TRUE);
            break;
        }

        case PNDV_EV_TO_PNDV_PRM_END_IND_DONE:
        {
            // response to an prm_end for one submodule (single or last of many)
            // add_1 = ar index
            // add_2 = entity_nr
            pndv_ar_peri_prm_end_ind_done(event.add_1, event.add_2, LSA_FALSE);
            break;
        }

        case PNDV_EV_TO_PNDV_PRM_END_IND_DONE_MORE_FOLLOWS:
        {
            // response to an prm_end for one submodule (more following)
            // add_1 = ar index
            // add_2 = entity_nr
            pndv_ar_peri_prm_end_ind_done(event.add_1, event.add_2, LSA_TRUE);
            break;
        }

        case PNDV_EV_TO_PNDV_AR_DISCONNECT_IND_DONE:
        {
            // response to a disconnect ind
            // add_1 = ar index
            // add_2 = not used
            pndv_ar_peri_disconnect_ind_done(event.add_1);
            break;
        }

        case PNDV_EV_TO_PNDV_AR_ABORT_REQ:
        {
            PNIO_UINT32 ar_idx;
            PNDV_IFACE_CMD_ENTRY_T tmp_event;

            ar_idx = event.add_1;

            pndv_ar_abort_req(ar_idx, (PNIO_UINT32)__LINE__);

            tmp_event.cmd = PNDV_EV_TO_PERI_AR_ABORT_REQ_DONE;
            tmp_event.add_1 = (PNIO_UINT8)ar_idx;
            tmp_event.add_2 = 0;

            pndv_in_peri_write_coupl_event( tmp_event);

            break;
        }

        case PNDV_EV_TO_PNDV_SM_IN_DATA_DONE:
        {
            //! response to an in_data indication
            //! add_1 = ar index
            //! add_2 = entity index
            pndv_ar_peri_in_data_done(event.add_1, event.add_2, LSA_FALSE);
            break;
        }

        case PNDV_EV_TO_PNDV_SM_IN_DATA_DONE_MORE_FOLLOWS:
        {
            //! response to an in_data indication  (more following)
            //! add_1 = ar index
            //! add_2 = entity index
            pndv_ar_peri_in_data_done(event.add_1, event.add_2, LSA_TRUE);
            break;
        }


        case PNDV_EV_TO_PNDV_DS_RW_DONE:
        {
            //! Read or write request has been processed
            //! add_1 = not used
            //! add_2 = not used
            CM_UPPER_RQB_PTR_TYPE       ds_ptr;
            CM_UPPER_SV_RECORD_PTR_TYPE record_ptr;

            if ( PNDV_IFACE_SERVICE_PROCCESSING == pndv_data.iface_ptr->ds_rw.state )
            {
                ds_ptr = pndv_data.iface_ptr->ds_rw.ptr_host;

                interface_convert_ptr_from_slave_to_host__( ds_ptr->args.pd.event,
                                                            ds_ptr->args.pd.event,
                                                            CM_UPPER_EVENT_PTR_TYPE );

                interface_convert_ptr_from_slave_to_host__( ds_ptr->args.pd.event->u.sv.ar_rec.data,
                                                            ds_ptr->args.pd.event->u.sv.ar_rec.data,
                                                            CM_COMMON_MEM_U8_PTR_TYPE );

                record_ptr = & ds_ptr->args.pd.event->u.sv.ar_rec;

                record_ptr->slot_nr = pndv_data.ds_rw_soll_slot_stored;

                /* Copy DS possibly from interface */
                /* --------------------------------------------------------- */

                if ( !pndv_host_ptr_is_nil__( pndv_data.rqb.store_ds_ptr_for_recopy))
                {
                    /* original DS is not in the interface -> copy back */

                    if ( CM_OPC_SV_AR_READ_IND == PNDV_RQB_GET_OPCODE(ds_ptr) )
                    {
                        /* !!! Don't touch CLRPC - DATA !!! Only copy data without cm_record_data */
                        PNDV_COPY_BYTE(pndv_data.rqb.store_ds_ptr_for_recopy->args.pd.event->u.sv.ar_rec.data + CM_RECORD_OFFSET,
                                       ds_ptr->args.pd.event->u.sv.ar_rec.data + CM_RECORD_OFFSET,
                                       record_ptr->data_length - CM_RECORD_OFFSET );
                    }

                    pndv_data.rqb.store_ds_ptr_for_recopy->args.pd.event->u.sv.ar_rec.data_length
                        = ds_ptr->args.pd.event->u.sv.ar_rec.data_length;

                    pndv_data.rqb.store_ds_ptr_for_recopy->args.pd.event->u.sv.ar_rec.cm_pnio_err
                        = ds_ptr->args.pd.event->u.sv.ar_rec.cm_pnio_err;

                    pndv_data.rqb.store_ds_ptr_for_recopy->args.pd.event->u.sv.ar_rec.ret_val_1
                        = ds_ptr->args.pd.event->u.sv.ar_rec.ret_val_1;

                    pndv_data.rqb.store_ds_ptr_for_recopy->args.pd.event->u.sv.ar_rec.ret_val_2
                        = ds_ptr->args.pd.event->u.sv.ar_rec.ret_val_2;

                    ds_ptr = pndv_data.rqb.store_ds_ptr_for_recopy;

                    pndv_data.rqb.store_ds_ptr_for_recopy = NIL;
                }

                #ifdef PNDV_CFG_DEBUG_ENABLE

                pndv_in_write_debug_buffer_ds__(PNDV_DC_PERI_DS_RW_DONE, ds_ptr->args.pd.event->u.sv.ar_rec.cm_pnio_err);

                #endif

                PNDV_DFC_DS_DONE(ds_ptr, PNDV_DFC_RES_ID_PERI );

                pndv_data.iface_ptr->ds_rw.state = PNDV_IFACE_SERVICE_IDLE;

#ifdef PNDV_CFG_PERI_QUEUE_DS_REQ
                //are there other waiting ds requests
                ds_ptr = 0;
                pndv_peri_rqb_queue_get_next(&ds_ptr);
                if (ds_ptr != 0)
                {
                    //! there is a waiting ds request
                    pndv_peri_read_write_record(ds_ptr);
                }
#endif
            }
            else
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.iface_ptr->ds_rw.state);
            }

            break;
        }

        case PNDV_EV_TO_PNDV_PULL_AL:
        {
            // peri signals that one or last of many submodule is pulled
            // add_1 = not used
            // add_2 = entity index

            pndv_pp_peri_submodule_remove(event.add_2, PNIO_FALSE);

            break;
        }

        case PNDV_EV_TO_PNDV_PULL_AL_MORE_FOLLOWS:
        {
            // peri signals that one submodule is pulled, more will follow
            // add_1 = not used
            // add_2 = entity index

            pndv_pp_peri_submodule_remove(event.add_2, PNIO_TRUE);
            break;
        }

        case PNDV_EV_TO_PNDV_PLUG_AL:
        {
            // peri signals that one or last of many submodule is plugged
            // add_1 = not used
            // add_2 = entity index

            /* pnpb has already updated the real config */

            pndv_pp_peri_submodule_add(event.add_2, PNIO_FALSE);

            break;
        }

        case PNDV_EV_TO_PNDV_PLUG_AL_MORE_FOLLOWS:
        {
            // peri signals that one submodule is plugged, more will follow
            // add_1 = not used
            // add_2 = entity index

            pndv_pp_peri_submodule_add(event.add_2, PNIO_TRUE);
            break;
        }

        case PNDV_EV_TO_PNDV_READY_FOR_INPUT_UPDATE_DONE:
        {
            // first input update was done
            // add_1 = ar_index
            // add_2 = unused
            pndv_ar_peri_ready_for_input_update_done(event.add_1);
            break;
        }

        case PNDV_EV_TO_PNDV_CHANNEL_DIAG:
        {
            // channel diag request
            //! add_1 = unused
            //! add_2 = entity_nr

            PNIO_UINT16 entity_nr = event.add_2;

            if (  entity_nr >= PNDV_MAX_SV_ENTITY )
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, entity_nr);        /*NOTREACHED*/
            }

            if ( PNDV_IFACE_SERVICE_NEW == pndv_data.iface_ptr->real_cfg[entity_nr].dial.state )
            {
                pndv_data.iface_ptr->real_cfg[entity_nr].dial.state = PNDV_IFACE_SERVICE_PROCCESSING;

                /* note how many channel diagnostics have come. 
                   The alarm as such is only acknowledged to the periphery when all CM alarms 
                   that resulted from the channel diagnostics have been acknowledged

                   The whole assumption is that an alarm is generated from each channel diagnosis
                */

                pndv_data.diag.wait_for_ch_diag_quit_cnt[entity_nr] = pndv_data.iface_ptr->real_cfg[entity_nr].dial.anz_chn_diag;

                if ( 0 == pndv_data.diag.wait_for_ch_diag_quit_cnt[entity_nr] )
                {
                    /* if everything has gone, the number of channel diagrams is 0 
                       -> a global outgoing alarm is issued for this slot 
                          which is then also acknowledged
                    */

                    pndv_data.diag.wait_for_ch_diag_quit_cnt[entity_nr] = 1;
                }
                pndv_al_peri_check_dial( entity_nr, (PNDV_REAL_CFG_T *) &pndv_data.iface_ptr->real_cfg[entity_nr] );
            }

            break;
        }

        case PNDV_EV_TO_PNDV_EXT_CHANNEL_DIAG:
        {
            // channel diag request
            //! add_1 = unused
            //! add_2 = entity_nr

            PNIO_UINT16 entity_nr = event.add_2;

            if (  entity_nr >= PNDV_MAX_SV_ENTITY )
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, entity_nr);    /*NOTREACHED*/
            }

            if ( PNDV_IFACE_SERVICE_NEW == pndv_data.iface_ptr->real_cfg[entity_nr].xdial.state )
            {
                pndv_data.iface_ptr->real_cfg[entity_nr].xdial.state = PNDV_IFACE_SERVICE_PROCCESSING;

                /* notice how many channel diagnostics have come
                   The alarm as such is only acknowledged to the periphery when all CM alarms 
                   that have resulted from the channel diagnostics have been acknowledged

                   The whole assumption is that an alarm is generated from each channel diagnosis
                */

                pndv_data.diag.wait_for_xch_diag_quit_cnt[entity_nr] = pndv_data.iface_ptr->real_cfg[entity_nr].xdial.anz_chn_diag;

                if ( 0 == pndv_data.diag.wait_for_xch_diag_quit_cnt[entity_nr] )
                {
                    /* if everything has gone, the number of channel diagrams is 0 
                       -> a global outgoing alarm is issued for this slot 
                          which is then also acknowledged
                    */

                    pndv_data.diag.wait_for_xch_diag_quit_cnt[entity_nr] = 1;
                }
                pndv_al_peri_check_xdial( entity_nr, (PNDV_REAL_CFG_T *) &pndv_data.iface_ptr->real_cfg[entity_nr] );
            }

            break;
        }

        case PNDV_EV_TO_PNDV_CHANNEL_DIAG_RMV_ALL:
        {
            // request to remove all channel diag of one ar
            // add_1 = ar_idx
            // add_2 = not used

            PNIO_UINT32 ar_idx;
            ar_idx = event.add_1;

            pndv_al_peri_dial_rmv_all(ar_idx);

            break;
        }

        case PNDV_EV_TO_PNDV_GENERIC_DIAG:
        {
            //  generic diag request
            //! add_1 = unused
            //! add_2 = interface_index

            PNIO_UINT16 interface_index;

            interface_index = event.add_2;

            if ( interface_index >= PNDV_MAX_GENERIC_DIAG_NUMBER )
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, interface_index); /*NOTREACHED*/
            }

            if ( PNDV_IFACE_SERVICE_NEW == pndv_data.iface_ptr->generic_diag_data[interface_index].state )
            {
                pndv_data.iface_ptr->generic_diag_data[interface_index].state = PNDV_IFACE_SERVICE_PROCCESSING;

                /* Set the diag req of the GenericDiag and attach it to the queue */

                if (PNIO_TRUE == pndv_data.iface_ptr->generic_diag_data[interface_index].alm_kommend)
                {
                    pndv_in_al_diag_req( PNDV_AL_USI_GENERIC_DIAG, interface_index, 0, 0, 0, 0, 0, PNDV_GENERIC_DIAG_ADD_REQ );
                }
                else
                {
                    pndv_in_al_diag_req( PNDV_AL_USI_GENERIC_DIAG, interface_index, 0, 0, 0, 0, 0, PNDV_GENERIC_DIAG_RMV_REQ );
                }
            }
            else
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, interface_index);
            }

            break;
        }

        case PNDV_EV_TO_PNDV_PRAL:
        {
            // process alarm request
            // add_1 = not used
            // add_2 = entity index
            PNIO_UINT16 entity_nr;

            entity_nr   = event.add_2;

            if ( PNDV_IFACE_SERVICE_NEW == pndv_data.iface_ptr->real_cfg[entity_nr].pral.state )
            {
                pndv_data.iface_ptr->real_cfg[entity_nr].pral.state = PNDV_IFACE_SERVICE_PROCCESSING;

                pndv_al_peri_pral_req( entity_nr, (PNDV_SENDMSG_PRALINFO_PTR) &pndv_data.iface_ptr->real_cfg[entity_nr].pral.info.data[0]);
            }

            break;
        }

        case PNDV_EV_TO_PNDV_UPAL:
        {
            // update alarm request
            // add_1 = not used
            // add_2 = entity index
            PNIO_UINT16 entity_nr;

            entity_nr   = event.add_2;

            if ( PNDV_IFACE_SERVICE_NEW == pndv_data.iface_ptr->real_cfg[entity_nr].upal.state )
            {
                pndv_data.iface_ptr->real_cfg[entity_nr].upal.state = PNDV_IFACE_SERVICE_PROCCESSING;

                pndv_al_peri_upal_req( entity_nr, (PNDV_SENDMSG_UPALINFO_PTR) &pndv_data.iface_ptr->real_cfg[entity_nr].upal.info.data[0]);
            }

            break;
        }

        case PNDV_EV_TO_PNDV_URAL:
        {
            // update alarm request
            // add_1 = not used
            // add_2 = entity index
            PNIO_UINT16 entity_nr;

            entity_nr = event.add_2;

            if ( PNDV_IFACE_SERVICE_NEW == pndv_data.iface_ptr->real_cfg[entity_nr].ural.state )
            {
                pndv_data.iface_ptr->real_cfg[entity_nr].ural.state = PNDV_IFACE_SERVICE_PROCCESSING;

                pndv_al_peri_ural_req( entity_nr, &pndv_data.iface_ptr->real_cfg[entity_nr].ural.info);
            }

            break;
        }

        case PNDV_EV_TO_PNDV_STAL:
        {
            // update alarm request
            // add_1 = not used
            // add_2 = entity index
            PNIO_UINT16 entity_nr;

            entity_nr   = event.add_2;

            if ( PNDV_IFACE_SERVICE_NEW == pndv_data.iface_ptr->real_cfg[entity_nr].stal.state )
            {
                pndv_data.iface_ptr->real_cfg[entity_nr].stal.state = PNDV_IFACE_SERVICE_PROCCESSING;

                pndv_al_peri_stal_req( entity_nr, &pndv_data.iface_ptr->real_cfg[entity_nr].stal.info);
            }

            break;
        }

        case PNDV_EV_TO_PNDV_PS_LOST:
        {
            // indication of lost backplane power for a single submodule
            // add_1 = not used
            // add_2 = entity index
            PNIO_UINT16 entity;
            PNDV_IFACE_CMD_ENTRY_T tmp_event;

            entity = event.add_2;

            if ( PNDV_IFACE_SERVICE_NEW == pndv_data.iface_ptr->ps.lost_state[entity] )
            {
                pndv_data.iface_ptr->ps.lost_state[entity] = PNDV_IFACE_SERVICE_PROCCESSING;

                tmp_event.cmd = PNDV_EV_TO_PERI_PS_LOST_QUIT;
                tmp_event.add_1 = event.add_1;
                tmp_event.add_2 = entity;

                pndv_in_peri_write_coupl_event( tmp_event );
            }
            else
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.iface_ptr->ps.lost_state[entity]);
            }

            break;
        }

        case PNDV_EV_TO_PNDV_PS_RETURN:
        {
            // indication of returning backplane power for a single submodule
            // add_1 = not used
            // add_2 = entity index
            PNIO_UINT16 entity;
            PNDV_IFACE_CMD_ENTRY_T tmp_event;

            entity = event.add_2;

            if ( PNDV_IFACE_SERVICE_NEW == pndv_data.iface_ptr->ps.return_state[entity] )
            {
                pndv_data.iface_ptr->ps.return_state[entity] = PNDV_IFACE_SERVICE_PROCCESSING;

                tmp_event.cmd = PNDV_EV_TO_PERI_PS_RETURN_QUIT;
                tmp_event.add_1 = event.add_1;
                tmp_event.add_2 = entity;

                pndv_in_peri_write_coupl_event( tmp_event );
            }
            else
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.iface_ptr->ps.return_state[entity]);
            }

            break;
        }

        case PNDV_EV_TO_PNDV_ROS_AL:
        {
            // return of submodule alarm indication
            // add_1 = not used
            // add_2 = entity index
            PNIO_UINT16 entity;

            entity = event.add_2;


            if ( PNDV_IFACE_SERVICE_NEW == pndv_data.iface_ptr->real_cfg[entity].ros.state )
            {
                pndv_data.iface_ptr->real_cfg[entity].ros.state = PNDV_IFACE_SERVICE_PROCCESSING;

                pndv_al_peri_rosal_req(entity);
            }

            break;
        }

        case PNDV_EV_TO_PNDV_SR_RDHT_TIMEOUT:
        {
            // indicates the timeout of sysred data hold timer
            // add_1 = ar_index
            // add_2 = unused

            pndv_ar_peri_sr_rdht_timeout(event.add_1);

            break;
        }

        case PNDV_EV_TO_PNDV_SR_EDGE_IND:
        {
            // indicates the every backup -> primary edge in the case that all ars of arset were in state backup
            // add_1 = ar_index
            // add_2 = is_primary

            pndv_ar_peri_sr_edge_indication(event.add_1, event.add_2);

            break;
        }

        case PNDV_EV_TO_PNDV_SR_OWNER_TAKEOVER_IND:
        {
            // request to take ownership (PNIO_TRUE) or to release ownership (PNIO_FALSE)
            // add_1 = ar_index
            // add_2 = is_owner

            pndv_ar_peri_sr_owner_takeover_ind(event.add_1, event.add_2);

            break;
        }

        case PNDV_EV_TO_PNDV_SR_ALARM_REPORTED_AFTER_SWO:
        {
            // the application has reported all diags and alarms after a switchover
            // the application want ot know if alle diags are set as alarm to CM

            pndv_al_peri_alarm_reported_after_swo();

            break;
        }

        case PNDV_EV_TO_PNDV_IP_SUITE: 
        {
            // set ip suite of device 
            // add_1 = not used
            // add_2 = not used
             pndv_in_ip2pn_set_ipsuite_user_req(pndv_data.iface_ptr->ip_suite.ip_address,
                                                pndv_data.iface_ptr->ip_suite.netmask,
                                                pndv_data.iface_ptr->ip_suite.gateway,
                                                pndv_data.iface_ptr->ip_suite.mk_remanent);
             break;

        }
        case PNDV_EV_TO_PNDV_NOS: 
        {
            // set name of station of device 
            // add_1 = not used
            // add_2 = not used
            pndv_in_cm_pd_set_address(pndv_data.iface_ptr->name_of_station.pStationName,
                                      pndv_data.iface_ptr->name_of_station.StationNameLen);
            break;
        }
        default:
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, event.cmd);
            break;
        }
    }
}




/**
 *  @brief read an event from interface
 *
 *  @return     returns the next event PNDV_EV_NO if none
 *
 *  Check the command interaface for the next event from peri
 *
 */
PNDV_IFACE_CMD_ENTRY_T pndv_in_peri_read_coupl_event (PNIO_VOID)
{
    PNDV_IFACE_CMD_ENTRY_T ret_event;

    if ( PNDV_SM_OPEN <= pndv_data.sm )
    {
        if ( pndv_data.iface_ptr->cmd.read_ptr_peri_to_pndv != pndv_data.iface_ptr->cmd.write_ptr_peri_to_pndv )
        {
            /* the ring buffer contains an entry */
            ret_event = pndv_data.iface_ptr->cmd.peri_to_pndv[pndv_data.iface_ptr->cmd.read_ptr_peri_to_pndv];

            pndv_in_write_debug_buffer_3__(PNDV_DC_PERI_READ_EVENT_HIGH, ret_event.cmd, ret_event.add_1, ret_event.add_2);

            /* Field is 256 bytes long and index is PNIO_UINT8 */
            if (++pndv_data.iface_ptr->cmd.read_ptr_peri_to_pndv == PNDV_IFACE_CMD_CNT)
            {
                pndv_data.iface_ptr->cmd.read_ptr_peri_to_pndv = 0;
            }
        }
        else
        {
            ret_event.cmd = PNDV_EV_NO;
        }
    }
    else
    {
        ret_event.cmd = PNDV_EV_NO;
    }

    return(ret_event);
}


/*****************************************************************************/


PNIO_VOID pndv_in_peri_write_coupl_event (PNDV_IFACE_CMD_ENTRY_T event)
{
    PNIO_UINT32 write_ptr_next = pndv_data.iface_ptr->cmd.write_ptr_pndv_to_peri + 1;

    pndv_in_write_debug_buffer_3__(PNDV_DC_PERI_WRITE_EVENT_HIGH, event.cmd, event.add_1, event.add_2
            | ((pndv_data.iface_ptr->cmd.write_ptr_pndv_to_peri & 0xFF) << 16)
            | ((pndv_data.iface_ptr->cmd.read_ptr_pndv_to_peri  & 0xFF) << 24));

    if (PNDV_IFACE_CMD_CNT <= write_ptr_next)
    {
        write_ptr_next = 0;
    }
    if ( pndv_data.iface_ptr->cmd.read_ptr_pndv_to_peri == write_ptr_next )
    {
        /* overflow -> fatal */
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }
    else
    {
        pndv_data.iface_ptr->cmd.pndv_to_peri[pndv_data.iface_ptr->cmd.write_ptr_pndv_to_peri] = event;

        pndv_data.iface_ptr->cmd.write_ptr_pndv_to_peri = write_ptr_next;

        /* trigger interface partner */
        PNDV_TRIGGER_INTERFACE_RESPONDER();
    }
}


/*****************************************************************************/


PNIO_VOID pndv_in_peri_put_ds_to_interface (CM_UPPER_RQB_PTR_TYPE ds_ptr )
{
    CM_UPPER_RQB_PTR_TYPE       tmp_ds_ptr;
    PNIO_UINT16                 soll_slot,tmp_subslot_nr;

    PNIO_UINT16                 entity_nr;
    PNDV_IFACE_CMD_ENTRY_T      tmp_event;
    PNIO_UINT32                 subslot_error;
    CM_UPPER_SV_RECORD_PTR_TYPE record_ptr;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    record_ptr  = &ds_ptr->args.pd.event->u.sv.ar_rec;
    soll_slot       = pndv_ds_get_slot_nr( record_ptr);
    tmp_subslot_nr  = pndv_ds_get_subslot_nr( record_ptr );

    pndv_get_submod_resource_ptr(&local_sub_res, soll_slot, tmp_subslot_nr, &subslot_error);
    if(subslot_error)
    {
        // slot/subslot not possible, must be an implementation error
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    if(local_sub_res == 0)
    {
        /* module is not plugged, don't send it to peri */
        pndv_set_rqb_err(ds_ptr, PNDV_EC1_REC_SLOT_INVALID);
        pndv_in_write_debug_buffer_ds__(PNDV_DC_PERI_DS_RW_DONE, ds_ptr->args.pd.event->u.sv.ar_rec.cm_pnio_err);

        pndv_data.iface_ptr->ds_rw.state = PNDV_IFACE_SERVICE_IDLE;

        PNDV_DFC_DS_DONE(ds_ptr, PNDV_DFC_RES_ID_PERI);

    }
    else
    {
        entity_nr = local_sub_res->entity;

        if (   ((PNIO_UINT32)ds_ptr <  (PNIO_UINT32)&glob_coupling_interface )
            || ((PNIO_UINT32)ds_ptr > ((PNIO_UINT32)(&glob_coupling_interface) + sizeof(glob_coupling_interface)))
           )
        {
            /* ds is not in the coupling interface 
               -> it must be copied there, the DFC resource can be used for this */

            if ( CM_OPC_SV_AR_READ_IND == PNDV_RQB_GET_OPCODE(ds_ptr) )
            {
                PNDV_COPY_BYTE(&pndv_data.iface_ptr->ds_rw.dfc_ds_struc_peri.rqb,
                               ds_ptr,
                               sizeof(CM_RQB_TYPE));
                PNDV_COPY_BYTE(&pndv_data.iface_ptr->ds_rw.dfc_ds_struc_peri.event_type,
                               ds_ptr->args.pd.event,
                               sizeof(CM_EVENT_TYPE));
            }
            else
            {
                PNDV_COPY_BYTE(&pndv_data.iface_ptr->ds_rw.dfc_ds_struc_peri.rqb,
                               ds_ptr,
                               sizeof(CM_RQB_TYPE));
                PNDV_COPY_BYTE(&pndv_data.iface_ptr->ds_rw.dfc_ds_struc_peri.event_type,
                               ds_ptr->args.pd.event,
                               sizeof(CM_EVENT_TYPE));
                PNDV_COPY_BYTE(&pndv_data.iface_ptr->ds_rw.dfc_ds_struc_peri.record_data[0],
                               ds_ptr->args.pd.event->u.sv.ar_rec.data,
                               ds_ptr->args.pd.event->u.sv.ar_rec.data_length );
            }

            interface_convert_ptr_from_host_to_slave__( &pndv_data.iface_ptr->ds_rw.dfc_ds_struc_peri.record_data[0],
                                                        pndv_data.iface_ptr->ds_rw.dfc_ds_struc_peri.event_type.u.sv.ar_rec.data,
                                                        CM_COMMON_MEM_U8_PTR_TYPE );

            interface_convert_ptr_from_host_to_slave__( &pndv_data.iface_ptr->ds_rw.dfc_ds_struc_peri.event_type,
                                                        pndv_data.iface_ptr->ds_rw.dfc_ds_struc_peri.rqb.args.pd.event,
                                                        CM_UPPER_EVENT_PTR_TYPE );

            tmp_ds_ptr = (CM_UPPER_RQB_PTR_TYPE) &pndv_data.iface_ptr->ds_rw.dfc_ds_struc_peri.rqb;

            pndv_data.rqb.store_ds_ptr_for_recopy = ds_ptr;
        }
        else
        {
            tmp_ds_ptr = ds_ptr;

            interface_convert_ptr_from_host_to_slave__( tmp_ds_ptr->args.pd.event->u.sv.ar_rec.data,
                                                        tmp_ds_ptr->args.pd.event->u.sv.ar_rec.data,
                                                        CM_COMMON_MEM_U8_PTR_TYPE );

            interface_convert_ptr_from_host_to_slave__( tmp_ds_ptr->args.pd.event,
                                                        tmp_ds_ptr->args.pd.event,
                                                        CM_UPPER_EVENT_PTR_TYPE );

            pndv_data.rqb.store_ds_ptr_for_recopy = NIL;
        }

        pndv_data.iface_ptr->ds_rw.ptr_host = tmp_ds_ptr;

        interface_convert_ptr_from_host_to_slave__( pndv_data.iface_ptr->ds_rw.ptr_host,
                                                    pndv_data.iface_ptr->ds_rw.ptr,
                                                    CM_UPPER_RQB_PTR_TYPE );
        pndv_data.ds_rw_soll_slot_stored = soll_slot;

        tmp_event.cmd = PNDV_EV_TO_PERI_DS_RW;
        tmp_event.add_1 = (PNIO_UINT8)pndv_ar_get_ar_idx_by_session_key(tmp_ds_ptr->args.pd.event->session_key);
        tmp_event.add_2 = entity_nr;
        pndv_in_peri_write_coupl_event( tmp_event);
    }
}


/*****************************************************************************/


PNIO_VOID pndv_in_peri_dial_quit(PNIO_UINT16 entity_nr, PNIO_UINT8 ret_val_to_peri)
{
    PNDV_IFACE_CMD_ENTRY_T tmp_event;

    if ( PNDV_IFACE_SERVICE_PROCCESSING == pndv_data.iface_ptr->real_cfg[entity_nr].dial.state )
    {
        pndv_data.diag.wait_for_ch_diag_quit_cnt[entity_nr]--;

        if ( 0 == pndv_data.diag.wait_for_ch_diag_quit_cnt[entity_nr] )
        {
            /* all alarms from channel diagnostics are acknowledged */

            tmp_event.cmd   = PNDV_EV_TO_PERI_CHANNEL_DIAG_QUIT;
            tmp_event.add_1 = ret_val_to_peri;
            tmp_event.add_2 = entity_nr;

            pndv_in_peri_write_coupl_event( tmp_event );
        }

    }
    else
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.iface_ptr->real_cfg[entity_nr].dial.state );
    }
}

/*****************************************************************************/

PNIO_VOID pndv_in_peri_xdial_quit(PNIO_UINT16 entity_nr, PNIO_UINT8 ret_val_to_peri)
{
    PNDV_IFACE_CMD_ENTRY_T tmp_event;

    if ( PNDV_IFACE_SERVICE_PROCCESSING == pndv_data.iface_ptr->real_cfg[entity_nr].xdial.state )
    {
        pndv_data.diag.wait_for_xch_diag_quit_cnt[entity_nr]--;

        if ( 0 == pndv_data.diag.wait_for_xch_diag_quit_cnt[entity_nr] )
        {
            /* all alarms from channel diagnostics are acknowledged */

            tmp_event.cmd   = PNDV_EV_TO_PERI_EXT_CHANNEL_DIAG_QUIT;
            tmp_event.add_1 = ret_val_to_peri;
            tmp_event.add_2 = entity_nr;

            pndv_in_peri_write_coupl_event( tmp_event );
        }

    }
    else
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.iface_ptr->real_cfg[entity_nr].xdial.state );
    }
}


/*****************************************************************************/


PNIO_VOID pndv_in_peri_generic_dial_quit(PNIO_UINT16 interface_idx, PNIO_UINT8 ret_val_to_peri)
{
    PNDV_IFACE_CMD_ENTRY_T tmp_event;

    if( interface_idx >= PNDV_MAX_GENERIC_DIAG_NUMBER )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, interface_idx );    /*NOTREACHED*/
    }

    if ( PNDV_IFACE_SERVICE_PROCCESSING == pndv_data.iface_ptr->generic_diag_data[interface_idx].state )
    {
        tmp_event.cmd = PNDV_EV_TO_PERI_GENERIC_DIAG_QUIT;
        tmp_event.add_1 = ret_val_to_peri;
        tmp_event.add_2 = interface_idx;

        pndv_in_peri_write_coupl_event( tmp_event );
    }
    else
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.iface_ptr->generic_diag_data[interface_idx].state );
    }
}


/*****************************************************************************/


PNIO_VOID pndv_in_peri_pral_quit(PNIO_UINT16 entity_nr, PNIO_UINT8 ret_val_to_peri)
{
    PNDV_IFACE_CMD_ENTRY_T tmp_event;

    if ( PNDV_IFACE_SERVICE_PROCCESSING == pndv_data.iface_ptr->real_cfg[entity_nr].pral.state )
    {
        tmp_event.cmd = PNDV_EV_TO_PERI_PRAL_QUIT;
        tmp_event.add_1 = ret_val_to_peri;
        tmp_event.add_2 = entity_nr;

        pndv_in_peri_write_coupl_event( tmp_event );
    }
    else
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.iface_ptr->real_cfg[entity_nr].pral.state);
    }
}


/*****************************************************************************/


PNIO_VOID pndv_in_peri_upal_quit(PNIO_UINT16 entity_nr)
{
    PNDV_IFACE_CMD_ENTRY_T tmp_event;

    if ( PNDV_IFACE_SERVICE_PROCCESSING == pndv_data.iface_ptr->real_cfg[entity_nr].upal.state )
    {
        tmp_event.cmd = PNDV_EV_TO_PERI_UPAL_QUIT;
        tmp_event.add_1 = 0;
        tmp_event.add_2 = entity_nr;

        pndv_in_peri_write_coupl_event( tmp_event );
    }
    else
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.iface_ptr->real_cfg[entity_nr].upal.state);
    }
}

/*****************************************************************************/


PNIO_VOID pndv_in_peri_ural_quit(PNIO_UINT16 entity_nr)
{
    PNDV_IFACE_CMD_ENTRY_T tmp_event;

    if ( PNDV_IFACE_SERVICE_PROCCESSING == pndv_data.iface_ptr->real_cfg[entity_nr].ural.state )
    {
        tmp_event.cmd = PNDV_EV_TO_PERI_URAL_QUIT;
        tmp_event.add_1 = 0;
        tmp_event.add_2 = entity_nr;

        pndv_in_peri_write_coupl_event( tmp_event );
    }
    else
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.iface_ptr->real_cfg[entity_nr].ural.state);
    }
}

/*****************************************************************************/


PNIO_VOID pndv_in_peri_stal_quit(PNIO_UINT16 entity_nr, PNIO_UINT8 ret_val_to_peri)
{
    PNDV_IFACE_CMD_ENTRY_T tmp_event;

    if ( PNDV_IFACE_SERVICE_PROCCESSING == pndv_data.iface_ptr->real_cfg[entity_nr].stal.state )
    {
        tmp_event.cmd = PNDV_EV_TO_PERI_STAL_QUIT;
        tmp_event.add_1 = ret_val_to_peri;
        tmp_event.add_2 = entity_nr;

        pndv_in_peri_write_coupl_event( tmp_event );
    }
    else
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.iface_ptr->real_cfg[entity_nr].stal.state);
    }
}

/*****************************************************************************/


PNIO_VOID pndv_in_peri_ros_al_quit(PNIO_UINT16 entity_nr)
{
    PNDV_IFACE_CMD_ENTRY_T tmp_event;

    if ( PNDV_IFACE_SERVICE_PROCCESSING == pndv_data.iface_ptr->real_cfg[entity_nr].ros.state )
    {
        tmp_event.cmd = PNDV_EV_TO_PERI_ROS_AL_QUIT;
        tmp_event.add_1 = 0;
        tmp_event.add_2 = entity_nr;

        pndv_in_peri_write_coupl_event( tmp_event );
    }
    else
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.iface_ptr->real_cfg[entity_nr].ros.state );
    }
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**********                                                         **********/
/**********               SYSTEM INTERFACE FUNCTIONS                **********/
/**********                                                         **********/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


/**
 *  @brief Sending records through the peri interface
 *
 *  @param[in]  req_ptr Pointer to the request block
 *
 *  This function is used to send a pn record through the peri interface.
 *  After some checks the record informations are copied into the interface and
 *  the appropriate service is triggerd.
 *  The function is designed to be used as a responder function to the dfc
 *  component.
 *  The returning request triggers PNDV_DFC_DS_DONE
 *
 */
PNIO_VOID pndv_peri_read_write_record (PNIO_VOID *req_ptr)
{
    CM_UPPER_RQB_PTR_TYPE       rqb_ptr;
    CM_UPPER_SV_RECORD_PTR_TYPE record_ptr;
    PNIO_UINT16                 tmp_slot_nr, tmp_subslot_nr;
    PNIO_BOOL                   record_to_peri;
#ifndef PNDV_NO_SPECIAL_TREATMENT_FOR_RW_RECORD_INDEX_1
    PNIO_UINT32                 data_length;
    PNIO_UINT32                 ret_val = PNIO_FALSE;
#endif
    PNIO_UINT32                 subslot_error;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;


    rqb_ptr     = (CM_UPPER_RQB_PTR_TYPE)req_ptr;
    record_ptr  = &rqb_ptr->args.pd.event->u.sv.ar_rec;

    record_to_peri = PNIO_FALSE;

    tmp_slot_nr     = pndv_ds_get_slot_nr( record_ptr );
    tmp_subslot_nr  = pndv_ds_get_subslot_nr( record_ptr );

    pndv_in_write_debug_buffer_3__(PNDV_DC_PERI_DS_RW, record_ptr->slot_nr, record_ptr->subslot_nr, record_ptr->record_index);

    /* --------------------------------------------------------------------- */

    /* This goes to PERI */
    /* ------------------------------------------------------------- */

    /* ar looking for */
    pndv_get_submod_resource_ptr(&local_sub_res, tmp_slot_nr, tmp_subslot_nr, &subslot_error);
    if(subslot_error)
    {
        // slot/subslot not possible, must be an implementation error
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    if ( PNDV_IFACE_SERVICE_IDLE == pndv_data.iface_ptr->ds_rw.state )
    {
        record_to_peri = PNIO_TRUE; /* send all records to peri */

        if (PNIO_TRUE == record_to_peri)
        {
            /* Retentivity of the station parameters */
#ifndef PNDV_NO_SPECIAL_TREATMENT_FOR_RW_RECORD_INDEX_1     // switch is a devkit requirement
            if (    ( PNDV_IM_SLOT_NO == tmp_slot_nr )
                 && (               1 == record_ptr->record_index )
               )
            {
                PNIO_UINT32 bytes_to_copy;
                PNIO_BOOL   new_prm;
                /* DS1 of the head with station parameters */

                new_prm = PNIO_TRUE;

                if ( 0 != data_length )
                {
                    /* Prm data is already available */
                    PNIO_UINT32 length_rem;
                    PNIO_UINT32 length_new;

                    length_rem = pndv_data.rema_station_prm_ptr->event_type.u.sv.ar_rec.data_length-CM_RECORD_OFFSET;
                    length_new = rqb_ptr->args.pd.event->u.sv.ar_rec.data_length-CM_RECORD_OFFSET;

                    if (length_rem == length_new)
                    {
                        /* Lengths are the same, form checksums over the data */
                    PNDV_COMPARE_BYTE(ret_val,
                                      &pndv_data.rema_station_prm_ptr->data[0],
                                      (rqb_ptr->args.pd.event->u.sv.ar_rec.data)+CM_RECORD_OFFSET,
                                      length_rem);
                        if (PNIO_TRUE != ret_val)
                        {
                            /* no difference */
                            new_prm = PNIO_FALSE;
                        }
                        else
                        {
                            /* Data are different */
                            new_prm = PNIO_TRUE;
                        }
                    }
                    else
                    {
                        /* Length is different, data are new */
                        new_prm = PNIO_TRUE;
                    }
                }

                if (PNDV_MAX_RECORD_GLOB_PARAM < data_length)
                {
                    /* the retentive data is unusually large, we'd rather take the new ones */
                    new_prm = PNIO_TRUE;
                }

                if( (new_prm) && (pndv_data.rema_station_prm_ptr != 0) )
                {
                    if ((CM_RECORD_OFFSET+PNDV_MAX_RECORD_DS1_LEN) > rqb_ptr->args.pd.event->u.sv.ar_rec.data_length)
                    {
                        bytes_to_copy = rqb_ptr->args.pd.event->u.sv.ar_rec.data_length;
                    }
                    else
                    {
                        bytes_to_copy = CM_RECORD_OFFSET + PNDV_MAX_RECORD_DS1_LEN;
                    }

                    PNDV_COPY_BYTE(&pndv_data.rema_station_prm_ptr->rqb,
                                   rqb_ptr,
                                   sizeof(CM_RQB_TYPE));
                    PNDV_COPY_BYTE(&pndv_data.rema_station_prm_ptr->event_type,
                                   rqb_ptr->args.pd.event,
                                   sizeof(CM_EVENT_TYPE));
                    PNDV_COPY_BYTE(&pndv_data.rema_station_prm_ptr->header[0],
                                   rqb_ptr->args.pd.event->u.sv.ar_rec.data,
                                   bytes_to_copy );

                    if (0 == data_length)
                    {
                        pndv_ar_abort_shared_ar ();
                    }
                }
            }
#endif  // PNDV_NO_SPECIAL_TREATMENT_FOR_RW_RECORD_INDEX_1
            pndv_data.iface_ptr->ds_rw.state = PNDV_IFACE_SERVICE_NEW;

            pndv_data.iface_ptr->ds_rw.ptr   = rqb_ptr;

            pndv_in_peri_put_ds_to_interface( rqb_ptr );
        }
    }
    else
    {
#ifdef PNDV_CFG_PERI_QUEUE_DS_REQ
        // interface already used, the request has to be queued
        if (PNDV_ERR_RESOURCE == pndv_peri_do_rqb_queue(rqb_ptr))
        {
            //request can not be queued
            pndv_set_rqb_err(rqb_ptr, PNDV_EC1_REC_BUSY);
            pndv_in_write_debug_buffer_ds__(PNDV_DC_PERI_DS_RW_DONE, rqb_ptr->args.pd.event->u.sv.ar_rec.cm_pnio_err); /* Record wird mit busy quittiert */

            PNDV_DFC_DS_DONE(rqb_ptr, PNDV_DFC_RES_ID_PERI);
        }

#else
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.iface_ptr->ds_rw.state);
#endif
    }
}

PNIO_VOID pndv_peri_init_entity_admin(PNIO_VOID)
{
    PNIO_UINT32 entity_count;
    PNIO_UINT32 ar_idx;

    for (ar_idx = 0; ar_idx < PNDV_CM_AR_NO; ar_idx++)
    {
        PNDV_LIST_INITIALIZE( &pndv_data.cfg.entity_admin[ar_idx].free_list );
        PNDV_LIST_INITIALIZE( &pndv_data.cfg.entity_admin[ar_idx].in_use_list );
        pndv_data.cfg.entity_admin[ar_idx].in_use_count = 0;
        for (entity_count = 0; entity_count < PNDV_MAX_SV_ENTITY; entity_count++)
        {
            pndv_data.cfg.entity_admin[ar_idx].entities[entity_count].entity_idx = entity_count;
            PNDV_LIST_INSERT_TAIL(&pndv_data.cfg.entity_admin[ar_idx].free_list, &pndv_data.cfg.entity_admin[ar_idx].entities[entity_count].link);
        }
    }
}

PNIO_VOID pndv_peri_reset_entity_admin_by_ar_idx(PNIO_UINT32 ar_idx)
{
    PNDV_LIST_APPEND( &pndv_data.cfg.entity_admin[ar_idx].free_list, &pndv_data.cfg.entity_admin[ar_idx].in_use_list);
    pndv_data.cfg.entity_admin[ar_idx].in_use_count = 0;
}



/**
 *  @brief Sending records through the peri interface
 *
 *  @param[in]  req_ptr Pointer to the request block
 *
 *
 */
PNIO_UINT32 pndv_peri_get_entity_index(PNIO_UINT16 slot, PNIO_UINT16 subslot, PNIO_UINT32 ar_idx)
{
    PNIO_UINT32                     local_entity_idx;
    PNDV_ENTITY_ADMIN_ELEM_PTR_T    local_entity_admin_elem_ptr;

    local_entity_idx = PNDV_CFG_UNUSED_ENTITY;

    if( ar_idx < PNDV_CM_AR_NO)
    {
        if(!PNDV_LIST_IS_EMPTY(&pndv_data.cfg.entity_admin[ar_idx].in_use_list))
        {
            for(PNDV_LIST_EACH(local_entity_admin_elem_ptr, &pndv_data.cfg.entity_admin[ar_idx].in_use_list, PNDV_ENTITY_ADMIN_ELEM_PTR_T))
            {
                if(  (local_entity_admin_elem_ptr->slot == slot )
                   &&(local_entity_admin_elem_ptr->sub_slot == subslot)
                   )
                {
                    local_entity_idx = local_entity_admin_elem_ptr->entity_idx;
                }
            }
        }
    }
    else
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0 );
    }

    if ( local_entity_idx == PNDV_CFG_UNUSED_ENTITY)
    {
        if( ar_idx < PNDV_CM_AR_NO)
        {
            // no used entity found, get a new one
            local_entity_admin_elem_ptr = PNDV_LIST_FIRST(&pndv_data.cfg.entity_admin[ar_idx].free_list, PNDV_ENTITY_ADMIN_ELEM_PTR_T);
            if(local_entity_admin_elem_ptr == 0)
            {
                // no more free entries
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0 );
            }
        }
        else
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0 );
        }
        PNDV_LIST_REMOVE_ENTRY(&local_entity_admin_elem_ptr->link);
        local_entity_idx = local_entity_admin_elem_ptr->entity_idx;
        local_entity_admin_elem_ptr->slot = slot;
        local_entity_admin_elem_ptr->sub_slot = subslot;
        if( ar_idx < PNDV_CM_AR_NO)
        {
            pndv_data.cfg.entity_admin[ar_idx].in_use_count++;
            PNDV_LIST_INSERT_TAIL(&pndv_data.cfg.entity_admin[ar_idx].in_use_list, &local_entity_admin_elem_ptr->link);
        }
        else
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0 );
        }
    }

    return local_entity_idx;
}

#ifdef PNDV_CFG_PERI_QUEUE_DS_REQ
PNIO_VOID pndv_peri_rqb_queue_get_next(CM_UPPER_RQB_PTR_TYPE *rqb_ptr_ptr)
{
    PNIO_UINT32 ret_val;
    PNDV_LIST_ENTRY_TYPE * local_list_ptr;
    PNDV_PERI_SERVICE_ELEMENT_PTR_T local_serv_elem_ptr;

    ret_val = PNDV_OK;


    local_list_ptr = &pndv_data.rqb_queue.in_use;


    if (ret_val == PNDV_OK)
    {
        local_serv_elem_ptr = PNDV_LIST_FIRST(local_list_ptr, PNDV_PERI_SERVICE_ELEMENT_PTR_T);

        if ( local_serv_elem_ptr == LSA_NULL )
        {
            //! no more waiting requests
            *rqb_ptr_ptr = 0;
        }
        else
        {
            //! remove entry from list
            PNDV_LIST_REMOVE_ENTRY(&local_serv_elem_ptr->link);

            *rqb_ptr_ptr = local_serv_elem_ptr->rqb_ptr;

            //! append to free list
            PNDV_LIST_INSERT_TAIL(&pndv_data.rqb_queue.free_list, &local_serv_elem_ptr->link);
        }
    }
}

PNIO_UINT32 pndv_peri_do_rqb_queue(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNDV_PERI_SERVICE_ELEMENT_PTR_T local_serv_elem_ptr;

    //! get an service-resource from free list

    local_serv_elem_ptr = PNDV_LIST_FIRST(&pndv_data.rqb_queue.free_list, PNDV_PERI_SERVICE_ELEMENT_PTR_T);
    if ( local_serv_elem_ptr == LSA_NULL )
    {
        //! no more free resources, service can not be queued
        return PNDV_ERR_RESOURCE;
    }

    //! remove entry from free list
    PNDV_LIST_REMOVE_ENTRY(&local_serv_elem_ptr->link);

    local_serv_elem_ptr->rqb_ptr = rqb_ptr;

    //! append to corresponding wait list
    PNDV_LIST_INSERT_TAIL(&pndv_data.rqb_queue.in_use, &local_serv_elem_ptr->link);

    return PNDV_OK;
}
#endif //ifdef PNDV_CFG_PERI_QUEUE_DS_REQ

/*****************************************************************************/
/*  end of file.                                                             */
/*****************************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
