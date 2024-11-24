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
/*  F i l e               &F: pndv_cm.c                                 :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Operation of the RQB interface from CM                                   */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/* contents:

    - pndv_cm_callback

    - pndv_in_cm_open_channel
    - pndv_in_cm_open_channel_done
    - pndv_in_cm_close_channel
    - pndv_in_cm_close_channel_done

    - pndv_in_cm_put_resource
    - pndv_in_cm_event_res_done
    - pndv_in_cm_al_res_done
    - pndv_in_cm_link_status_info
    - pndv_in_cm_provide_led_info_rqb
    - pndv_in_cm_dev_led_info
    - pndv_in_cm_sync_led_info
    - pndv_in_cm_pdsv_create
    - pndv_in_cm_pdsv_create_done
    - pndv_in_cm_pdsv_delete
    - pndv_in_cm_pdsv_delete_done

    - pndv_in_cm_sv_create
    - pndv_in_cm_sv_create_done
    - pndv_in_cm_sv_delete
    - pndv_in_cm_sv_delete_done
    - pndv_in_cm_sv_control_done
    - pndv_in_cm_check_dev_req
    - pndv_in_cm_sv_device_add
    - pndv_in_cm_sv_device_add_done
    - pndv_in_cm_sv_device_remove
    - pndv_in_cm_sv_device_remove_done
    - pndv_in_cm_sv_device_read_done

    - pndv_in_cm_pd_prm_begin
    - pndv_in_cm_pd_build_prm_list
    - pndv_in_cm_pd_prm_begin_done
    - pndv_in_cm_pd_prm_write
    - pndv_in_cm_pd_prm_write_done
    - pndv_in_cm_pd_prm_end
    - pndv_in_cm_pd_prm_end_done
    - pndv_in_cm_pd_event_appl_ready_ind
    - pndv_in_cm_pd_set_address
    - pndv_in_cm_pd_set_address_done
    - pndv_in_cm_pd_rema_indication_provide_resource
    - pndv_in_cm_pd_rema_indication_handle
    - pndv_in_cm_pd_dcp_indication_provide_resource
    - pndv_in_cm_pd_dcp_indication_handle
    - pndv_in_cm_pd_dcp_indication_response
    - pndv_in_cm_pd_dcp_indication_response_done

    - pndv_in_cm_sv_device_provide_event_done
    - pndv_in_cm_sv_device_provide_alarm_done
    - pndv_cm_in_device_read_req
    - pndv_in_device_control
    - pndv_in_cm_sv_device_control_done
    - pndv_in_cm_sv_ar_control_done
    - pndv_ar_cm_ar_abort_done
    - pndv_in_cm_sv_event_prm_end_ind
    - pndv_in_cm_sv_event_apdu_status_ind
    - pndv_ar_cm_ar_in_data_ind
    - pndv_in_cm_to_appl_cbf

*/
/*****************************************************************************/
/* 2do:


*/
/*****************************************************************************/
/* include hierarchy */

#include "pndv_inc.h"

#define PNDV_MODULE PNDV_ERR_MODULE_CM


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**********                                                         **********/
/**********               SYSTEM INTERFACE FUNCTIONS                **********/
/**********                                                         **********/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
PNIO_UINT16 get_nos_data_from_rema_record(PNIO_UINT8* pRecData, LSA_UINT32 RecordLen,
                                          PNIO_UINT8** nos_data_ptr_ptr, PNIO_UINT16* nos_data_len_ptr);


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**********                                                         **********/
/**********                 CM INTERFACE FUNCTIONS                  **********/
/**********                                                         **********/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


PNIO_VOID pndv_cm_callback(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 free_rqb;
    PNIO_UINT32 rqb_back;
    PNIO_UINT32 tmp_int;

    free_rqb = PNIO_TRUE;
    rqb_back = PNIO_FALSE;

    /* ignore
    PNDV_RQB_GET_HANDLE( rqb_ptr);
    */

    switch (PNDV_RQB_GET_OPCODE(rqb_ptr))
    {

        /******************** CM Channel *********************/
        /******************* Confirmations *******************/

        case CM_OPC_OPEN_CHANNEL:
        {
            pndv_in_cm_open_channel_done( rqb_ptr);

            break;
        }

        case CM_OPC_CLOSE_CHANNEL:
        {
            pndv_in_cm_close_channel_done( rqb_ptr);

            break;
        }

        case CM_OPC_PD_PROVIDE_EVENT:
        {
            pndv_in_cm_event_res_done( rqb_ptr);

            break;
        }

        case CM_OPC_PD_PROVIDE_ALARM:
        {
            pndv_in_cm_al_res_done( rqb_ptr);

            break;
        }

        /******************** CM PD **************************/
        /******************* Confirmations *******************/

        case CM_OPC_PD_CREATE:
        {
            pndv_in_cm_pdsv_create_done( rqb_ptr );

            break;
        }

        case CM_OPC_PD_DELETE:
        {
            pndv_in_cm_pdsv_delete_done( rqb_ptr );

            break;
        }

        case CM_OPC_PD_LINK_STATUS_INFO:
        {
            rqb_back = pndv_in_cm_link_status_info( rqb_ptr);

            if ( PNIO_TRUE == rqb_back )
            {
                /* RQB goes back to CM unchanged */

                free_rqb = PNIO_FALSE;
            }
            else
            {
                /* if RQB does not go back, it will be released and allocated with the next open_channel  */

                free_rqb = PNIO_TRUE;
            }

            break;
        }

        case CM_OPC_PD_REMAP:
        {
            //! we do not do that
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE( rqb_ptr));
            break;
        }

        case CM_OPC_PD_PRM_READ:
        {
            //! we do not do that
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE( rqb_ptr));
            break;
        }

        case CM_OPC_PD_PRM_BEGIN:
        {
            pndv_in_cm_pd_prm_begin_done( rqb_ptr );

            break;
        }

        case CM_OPC_PD_PRM_WRITE:
        {
            free_rqb = PNIO_FALSE;

            pndv_in_cm_pd_prm_write_done( rqb_ptr );

            break;
        }

        case CM_OPC_PD_PRM_END:
        {
            pndv_in_cm_pd_prm_end_done( rqb_ptr );

            break;
        }

        case CM_OPC_PD_SET_ADDRESS:
        {
            free_rqb = PNIO_TRUE;
            pndv_in_cm_pd_set_address_done(rqb_ptr);

            break;
        }

        case CM_OPC_PD_SYNC_LED_INFO:
        {
            free_rqb = PNIO_FALSE;

            rqb_back = pndv_in_cm_sync_led_info(rqb_ptr);

            break;
        }

        /******************** Indications ********************/

        case CM_OPC_PD_ALARM_IND:
        {
            //! we don't want that at all
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE( rqb_ptr));
            break;
        }

        case CM_OPC_PD_EVENT_APPL_READY_IND:
        {
            free_rqb = PNIO_FALSE;

            pndv_in_cm_pd_event_appl_ready_ind( rqb_ptr);

            break;
        }

        case CM_OPC_PD_DCP_QUERY_INDICATION:
        {
            free_rqb = PNIO_FALSE;
            rqb_back = PNIO_FALSE;

            pndv_in_cm_pd_dcp_indication_handle(rqb_ptr);

            break;
        }

        case CM_OPC_PD_DCP_QUERY_RESULT:
        {
            free_rqb = PNIO_FALSE;
            rqb_back = PNIO_FALSE;

            pndv_in_cm_pd_dcp_indication_response_done(rqb_ptr);

            break;
        }

        case CM_OPC_PD_REMA_INDICATION:
        {
            free_rqb = PNIO_FALSE;
            rqb_back = PNIO_FALSE;

            pndv_in_cm_pd_rema_indication_handle(rqb_ptr);
            break;
        }

        /******************** CM Server **********************/
        /******************* Confirmations *******************/

        case CM_OPC_SV_CREATE:
        {
            pndv_in_cm_sv_create_done( rqb_ptr);

            break;
        }

        case CM_OPC_SV_DELETE:
        {
            pndv_in_cm_sv_delete_done( rqb_ptr);

            break;
        }

        case CM_OPC_SV_CONTROL:
        {
            /* Do not release statically created RQB and mark it as free */

            free_rqb = PNIO_FALSE;

            PNDV_RQB_SET_OPCODE( rqb_ptr, 0);

            pndv_in_cm_sv_control_done( rqb_ptr);

            break;
        }

        case CM_OPC_SV_DEVICE_ADD:
        {
            pndv_in_cm_sv_device_add_done( rqb_ptr);

            break;
        }

        case CM_OPC_SV_DEVICE_REMOVE:
        {
            pndv_in_cm_sv_device_remove_done( rqb_ptr);

            break;
        }

        case CM_OPC_SV_DEVICE_CONTROL:
        {
            /* Do not release statically created RQB and mark it as free */

            free_rqb = PNIO_FALSE;

            PNDV_RQB_SET_OPCODE( rqb_ptr, 0);

            pndv_in_cm_sv_device_control_done( rqb_ptr);

            break;
        }

        case CM_OPC_SV_DEVICE_LED_INFO:
        {
            /* Do not release statically created RQB */

            free_rqb = PNIO_FALSE;

            rqb_back = pndv_in_cm_dev_led_info( rqb_ptr);

            break;
        }

        case CM_OPC_SV_DEVICE_PROVIDE_EVENT:
        {
            pndv_in_cm_sv_device_provide_event_done(rqb_ptr);
            break;
        }

        case CM_OPC_SV_DEVICE_PROVIDE_ALARM:
        {
            pndv_in_cm_sv_device_provide_alarm_done(rqb_ptr);
            break;
        }

#ifdef PNDV_CFG_USE_DEVICE_READ
        case CM_OPC_SV_DEVICE_READ:
        {
            free_rqb = PNIO_FALSE;

            pndv_in_cm_sv_device_read_done(rqb_ptr);
            break;
        }
#endif
        case CM_OPC_SV_SUBMODULE_ADD:
        {
            /* Do not release statically created RQB and mark it as free */

            free_rqb = PNIO_FALSE;

            pndv_pp_cm_submodule_add_done(rqb_ptr);

            break;
        }

        case CM_OPC_SV_SUBMODULE_REMOVE:
        {
            /* Do not release statically created RQB and mark it as free */

            free_rqb = PNIO_FALSE;

            pndv_pp_cm_submodule_remove_done( rqb_ptr);

            break;
        }

#if (defined PNDV_CFG_USE_DIAG2)||(defined PNDV_CFG_USE_MULTIDIAG)
        case CM_OPC_SV_DIAG_ADD2:
        {
            free_rqb = PNIO_FALSE;

            pndv_in_al_diag_add_done( rqb_ptr);
            break;
        }

        case CM_OPC_SV_DIAG_REMOVE2:
        {
            /* Do not release statically created RQB and mark it as free */

            free_rqb = PNIO_FALSE;

            pndv_in_al_diag_remove2_done( rqb_ptr);;
            break;
        }
#endif
#if !(defined PNDV_CFG_USE_DIAG2)
        case CM_OPC_SV_DIAG_ADD:
        {
            free_rqb = PNIO_FALSE;

            pndv_in_al_diag_add_done( rqb_ptr);

            break;

        }

        case CM_OPC_SV_DIAG_REMOVE:
        {
            /* Do not release statically created RQB and mark it as free */

            free_rqb = PNIO_FALSE;

            pndv_in_al_diag_remove_done( rqb_ptr);

            break;
        }
#endif

        case CM_OPC_SV_ARSET_TRIGGER:
        {
            free_rqb = PNIO_FALSE;
            rqb_back = PNIO_FALSE;

            pndv_ar_cm_sr_ar_set_trigger_cnf(rqb_ptr);

            break;
        }
        case CM_OPC_SV_ARSET_ABORT:
        {
            free_rqb = PNIO_FALSE;
            rqb_back = PNIO_FALSE;

            pndv_ar_cm_sr_rdht_timeout_cnf(rqb_ptr);

            break;
        }
        case CM_OPC_SV_AR_APPL_READY:
        {
            /* Do not release statically created RQB and mark it as free */

            free_rqb = PNIO_FALSE;

            PNDV_RQB_SET_OPCODE( rqb_ptr, 0);

            pndv_ar_cm_sv_appl_ready_done( rqb_ptr);

            break;
        }

        case CM_OPC_SV_AR_CONTROL:
        {
            pndv_in_cm_sv_ar_control_done( rqb_ptr);

            break;
        }

        case CM_OPC_SV_AR_ABORT:
        {
            /* Do not release statically created RQB and mark it as free */

            free_rqb = PNIO_FALSE;

            pndv_ar_cm_ar_abort_done( rqb_ptr);

            break;
        }

        case CM_OPC_SV_AR_RIR_ACK:
        {
            free_rqb = PNIO_FALSE;

            // confirmation to rir_ack arrived
            pndv_ar_cm_sv_rir_ack_cnf(rqb_ptr);
            break;
        }

        case CM_OPC_SV_AR_ALARM_SEND:
        {
            // only static rqbs used here
            free_rqb = PNIO_FALSE;
            pndv_in_al_al_ack( rqb_ptr);

            break;
        }

        case CM_OPC_SV_AR_ALARM_ACK:
        {
            //! not used
            free_rqb = PNIO_FALSE;
            rqb_back = PNIO_TRUE;

            pndv_in_al_sv_alarm_ack_done( rqb_ptr);

            break;
        }

        /******************** Indications ********************/

        case CM_OPC_SV_AR_ALARM_IND:
        {
            /* can't actually come because CM_OPC_SV_CREATE
               the reject bit is set for all alarms
               -> CM acknowledges with PNIO_ERR */

            free_rqb = PNIO_FALSE;
            rqb_back = PNIO_TRUE;

            pndv_in_al_sv_alarm_ind( rqb_ptr);

            break;
        }

        case CM_OPC_SV_AR_CONNECT_IND:
        {
            free_rqb = PNIO_FALSE;
            //! OK
            if ((pndv_data.cfg.peri_cfg.sm_state == PNDV_SM_PERIBUS_STATE_UNKNOWN) &&
                ((rqb_ptr->args.sv.ar_event->u.sv.ar_connect->ar_type != CM_AR_TYPE_SUPERVISOR) ||
                 (rqb_ptr->args.sv.ar_event->u.sv.ar_connect->nr_of_iocrs != 0)))
            {
                PNIO_UINT16 ret_val;
                LSA_USER_ID_TYPE user_id;

                if (pndv_ar_do_service_queue(rqb_ptr, PNDV_PERI_SERVICE_IDENT_CON) != PNDV_OK)
                {
                    //! queueing not possible, no free resource, check PNDV_PERI_SERVICE_QUEUES_T elem
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
                }

                /* start the timeout timer */
                user_id.uvar32 = 0;
                if (pndv_data.serv.con_serv_run == PNIO_FALSE)
                {
                    pndv_data.serv.con_serv_run = PNIO_TRUE;
                    PNDV_START_TIMER( &ret_val, pndv_data.connect_indication_timeout_timer_id, user_id, 33);
                    LSA_UNUSED_ARG(ret_val);
                }
                else
                {
                    /* don't restart the timer */
                }

                pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_CONNECT_IND_3, 0xffff,0);
            }
            else

            {
                pndv_ar_cm_connect_ind(rqb_ptr);
            }
            break;
        }

        case CM_OPC_SV_AR_DISCONNECT_IND:
        {
            //! OK
            free_rqb = PNIO_FALSE;
            rqb_back = PNIO_FALSE; /* rqb returned to cm - directly: non PERI-AR or not directly: PERI-AR */

            pndv_ar_cm_ar_disconnect_ind(rqb_ptr);

            break;
        }

        case CM_OPC_SV_AR_OWNERSHIP_IND:
        {
            //! indication resource allways go back to cm (inside function)
            free_rqb = PNIO_FALSE;

            pndv_ar_cm_sv_ownership_ind(rqb_ptr);

            break;
        }

        case CM_OPC_SV_AR_PRM_END_IND:
        {
            free_rqb = PNIO_FALSE;
            rqb_back = PNIO_FALSE; /* RQB is returned in the function so that PRM_END.Rsp goes to the CM before a possibly following module Z / S */

            pndv_ar_cm_sv_event_prm_end_ind(rqb_ptr);

            break;
        }

        case CM_OPC_SV_AR_IN_DATA_IND:
        {
            free_rqb = PNIO_FALSE;
            rqb_back = PNIO_FALSE;

            pndv_ar_cm_ar_in_data_ind(rqb_ptr);

            break;
        }

        case CM_OPC_SV_AR_RIR_IND:
        {
            free_rqb = PNIO_FALSE;
            rqb_back = PNIO_FALSE; /* rqb returned to cm after input update */

            pndv_ar_cm_sv_event_ar_rir_ind(rqb_ptr);

            break;
        }

        case CM_OPC_SV_AR_READ_IND:
        case CM_OPC_SV_AR_WRITE_IND:
        {
            free_rqb = PNIO_FALSE;
            rqb_back = PNIO_FALSE;

            pndv_in_cm_read_write_ind(rqb_ptr);

            break;
        }

        case CM_OPC_SV_AR_DATA_STATUS_IND:
        {
            free_rqb = PNIO_FALSE;
            rqb_back = PNIO_TRUE;

            pndv_in_cm_sv_event_apdu_status_ind(rqb_ptr);

            break;
        }

        case CM_OPC_SV_AR_PDEVPRM_IND:
        case CM_OPC_SV_AR_PDEVPRM_RSP:
        case CM_OPC_SV_AR_PDEVPRM_READ:
        case CM_OPC_SV_AR_PDEVPRM_WRITE:
        {
            /* what do I do with it? Well let's see if it pops. */
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE(rqb_ptr));
            break;
        }

        case CM_OPC_PD_SUBMODULE_CONTROL:
        {
            /* returning plug of a pdev subslot */
            pndv_pp_cm_pd_submodule_add_done(rqb_ptr);
            break;
        }

        case CM_OPC_PD_PTCP_MASTER_CONTROL:
        {
            /* what do I do with it? Well let's see if it pops. */
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE(rqb_ptr));
            break;
        }

        case CM_OPC_SV_R1BRIDGE_SEND_PROVIDE:
        case CM_OPC_SV_R1BRIDGE_SEND_IND:
        case CM_OPC_SV_R1BRIDGE_SEND_RSP:
        case CM_OPC_SV_R1BRIDGE_RECEIVED:
        case CM_OPC_SV_R1BRIDGE_CONTROL:
        {
            free_rqb = PNIO_FALSE;
            rqb_back = PNIO_FALSE;
            break;
        }

        /************************* reserved opcodes **********************/

        /* for a better compatibility all reserved and unused opcodes removed */

        /* ------------------------------------------------------------- */

        default:
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE(rqb_ptr));
            break;
        }

    } /* end switch */


    if (PNIO_TRUE == free_rqb)
    {
        PNDV_FREE_RQB(&tmp_int, rqb_ptr);
        if (LSA_OK != tmp_int)
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, tmp_int);
        }
    }
    else if (PNIO_TRUE == rqb_back)
    {
        if ((CM_OPC_PD_LINK_STATUS_INFO == PNDV_RQB_GET_OPCODE(rqb_ptr)) ||
            (CM_OPC_PD_SYNC_LED_INFO    == PNDV_RQB_GET_OPCODE(rqb_ptr)))
        {
            PNDV_RQB_SET_HANDLE(rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_CMPDSV_ACP]);
        }
        else
        {
            PNDV_RQB_SET_HANDLE(rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
        }

        PNDV_REQUEST(rqb_ptr, LSA_COMP_ID_CM);
    }
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**********                                                         **********/
/**********                DPM-Simu INTERFACE FUNCTIONS             **********/
/**********                                                         **********/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/



/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**********                                                         **********/
/**********                   INTERNAL FUNCTIONS                    **********/
/**********                                                         **********/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


/**
 *  @brief open channel to cm
 *
 *  @param[in]     channel_idx channel index
 *
 *  internal service function to open a channel to cm
 *  confirmation in pndv_in_cm_open_channel_done()
 *
 */
PNIO_VOID pndv_in_cm_open_channel(PNIO_UINT8 channel_idx)
{
    union
    {
        CM_UPPER_RQB_PTR_TYPE rqb;
        LSA_VOID_PTR_TYPE     void_;
    } ptr;

    PNDV_ALLOC_RQB(&ptr.void_, sizeof(CM_RQB_TYPE));
    if (pndv_host_ptr_is_nil__(ptr.rqb))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    PNDV_ALLOC_MEM(((LSA_VOID_PTR_TYPE)&ptr.rqb->args.channel.open), sizeof(CM_OPEN_CHANNEL_TYPE));
    if (pndv_host_ptr_is_nil__(ptr.rqb->args.channel.open))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    PNDV_SYSPATH_SET_HD(pndv_data.sys_path_cm[channel_idx], 1);
    ptr.rqb->args.channel.open->handle                    = CM_INVALID_HANDLE;
    ptr.rqb->args.channel.open->handle_upper              = channel_idx;                        /* my handle */
    ptr.rqb->args.channel.open->sys_path                  = pndv_data.sys_path_cm[channel_idx];
    ptr.rqb->args.channel.open->cm_request_upper_done_ptr = pndv_in_cm_to_appl_cbf;

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_OPEN_CH, (PNIO_UINT16)channel_idx);

    PNDV_RQB_SET_HANDLE(ptr.rqb, 0);
    PNDV_RQB_SET_OPCODE(ptr.rqb, CM_OPC_OPEN_CHANNEL);
    PNDV_OPEN_CHANNEL_LOWER(ptr.rqb, LSA_COMP_ID_CM);
}


/*****************************************************************************/


/**
 *  @brief confirmation to open channel service
 *
 *  @param[in]     rqb_ptr pointer to request block with confirmation information
 *
 *  called by pndv_cm_callback(), confirmation to a open channel request
 *
 */
PNIO_VOID pndv_in_cm_open_channel_done(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 tmp_int;
    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_OPEN_CH_DONE, PNDV_RQB_GET_HANDLE(rqb_ptr));

    PNIO_UINT16 response  = PNDV_RQB_GET_RESPONSE(rqb_ptr);
    if ((CM_OK          != response) && 
        (CM_OK_REPLACED != response))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }

    LSA_HANDLE_TYPE ch_handle = rqb_ptr->args.channel.open->handle_upper;
    if (!((ch_handle == PNDV_INDEX_PATH_CMPDSV_ACP) || (ch_handle == PNDV_INDEX_PATH_IOD_CM_ACP)))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, ch_handle);
    }

    pndv_data.cm_handle[ch_handle] = rqb_ptr->args.channel.open->handle;

    PNDV_FREE_MEM(&tmp_int, rqb_ptr->args.channel.open);
    if (LSA_OK != tmp_int)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, tmp_int);
    }

    switch (pndv_data.sm)
    {
        case PNDV_SM_W_OPEN_CMPDSV_ACP_CHANNEL_DONE:
        {
            pndv_sm(PNDV_SM_EVENT_CMPDSV_ACP_CHANNEL_OPEN_DONE);
            break;
        }
        case PNDV_SM_W_OPEN_IOD_CMSV_ACP_CHANNEL_DONE:
        {
            pndv_sm(PNDV_SM_EVENT_OPEN_IOD_CMSV_ACP_CHANNEL_DONE);
            break;
        }
        default:
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, pndv_data.sm);
            break;
        }
    }
}


/*****************************************************************************/


/**
 *  @brief close a channel to cm
 *
 *  @param[in]     channel_idx Index of the channel that should be closed
 *
 *  This functions sends a close request of a channel to cm (see CM_OPC_CLOSE_CHANNEL).
 *
 *
 */
PNIO_VOID pndv_in_cm_close_channel(PNIO_UINT16 channel_idx)
{
    union
    {
        CM_UPPER_RQB_PTR_TYPE rqb;
        LSA_VOID_PTR_TYPE     void_;
    }   ptr;

    PNDV_ALLOC_RQB(&ptr.void_, sizeof(CM_RQB_TYPE));
    if (pndv_host_ptr_is_nil__(ptr.rqb))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_CLOSE_CH,channel_idx);

    PNDV_RQB_SET_HANDLE(ptr.rqb, pndv_data.cm_handle[channel_idx]);
    PNDV_RQB_SET_OPCODE(ptr.rqb, CM_OPC_CLOSE_CHANNEL);

    PNDV_CLOSE_CHANNEL_LOWER(ptr.rqb, LSA_COMP_ID_CM);
}


/**
 *  @brief callback to channel close
 *
 *  @param[in]     rqb_ptr Request block of the close channel request
 *
 *  This function handles the callback of a channel close (see CM_OPC_CLOSE_CHANNEL)
 *
 */
PNIO_VOID pndv_in_cm_close_channel_done(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_CLOSE_CH_DONE, PNDV_RQB_GET_HANDLE(rqb_ptr));

    if ((CM_OK          != response) && 
        (CM_OK_REPLACED != response))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }

    switch (PNDV_RQB_GET_HANDLE(rqb_ptr))
    {
        case PNDV_INDEX_PATH_IOD_CM_ACP:
        {
            pndv_sm(PNDV_SM_EVENT_IOD_CMSV_ACP_CLOSE_CHANNEL_DONE);
            break;
        }
        case PNDV_INDEX_PATH_CMPDSV_ACP:
        {
            pndv_sm(PNDV_SM_EVENT_CMPDSV_ACP_CLOSE_CHANNEL_DONE);
            break;
        }
        default:
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
            break;
        }
    }
}


/*****************************************************************************/


/**
 *  @brief send pd event resources to cm
 *
 *  This function is called during pndv startup after opening the channels.
 *  It is used to send some indication resources (CM_OPC_PD_PROVIDE_EVENT)
 *  to CM_PD
 */
PNIO_VOID pndv_in_cm_put_resource (PNIO_UINT32 num_event_resources, PNIO_UINT32 num_alarm_resources)
{
    union
    {
        CM_UPPER_RQB_PTR_TYPE rqb;
        LSA_VOID_PTR_TYPE     void_;
    }   ptr;

    PNIO_UINT16           i;

    /* Event-Ressourcen CM-PD */
    /* --------------------------------------------------------------------- */

    for (i = 0; i < num_event_resources; i++)
    {
        PNDV_ALLOC_RQB(&ptr.void_, sizeof(CM_RQB_TYPE));
        if (pndv_host_ptr_is_nil__(ptr.rqb))
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
        }

        PNDV_ALLOC_MEM(((LSA_VOID_PTR_TYPE)&ptr.rqb->args.pd.event), sizeof(CM_EVENT_TYPE));
        if (pndv_host_ptr_is_nil__(ptr.rqb->args.pd.event))
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
        }

        pndv_in_write_debug_buffer_all__(PNDV_DC_CM_PD_RES_PROV,i);

        PNDV_RQB_SET_HANDLE(ptr.rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_CMPDSV_ACP]);
        PNDV_RQB_SET_OPCODE(ptr.rqb, CM_OPC_PD_PROVIDE_EVENT);

        PNDV_REQUEST(ptr.rqb, LSA_COMP_ID_CM);
    }

    /* PD Alarm resources: we don't need these alarms (port data change notif. / time data changed / sync data change / redundancy / ...)
     * -> don't provide resources */
}


/*****************************************************************************/


/**
 *  @brief callback for provide event resources
 *
 *  @param[in]     rqb_ptr Pointer to request block
 *
 *  This callback handles CM_OPC_PD_PROVIDE_EVENT.
 *  Only triggered when resources should be freed.
 *  "Used" Resources change there opcode to indicate something.
 *
 */
PNIO_VOID pndv_in_cm_event_res_done(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;
    PNIO_UINT32 tmp_int;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_RES_BACK,0);

    if (CM_OK_CANCELLED != response)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }

    PNDV_FREE_MEM(&tmp_int, rqb_ptr->args.pd.event);
    if (LSA_OK != tmp_int)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, tmp_int);
    }
}

/**
 *  @brief callback for provide alarm resoruces
 *
 *  @param[in]     rqb_ptr Pointer to request block
 *
 *  This callback handles CM_OPC_ALARM_IND_RES_PROVIDE.
 *  Only triggered when resources should be freed.
 *  "Used" Resources change there opcode to indicate something.
 *
 */
PNIO_VOID pndv_in_cm_al_res_done(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;
    PNIO_UINT32 tmp_int;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    pndv_in_write_debug_buffer_al__(PNDV_DC_CM_AL_RES_BACK,0);

    if (CM_OK_CANCELLED != response)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }

    PNDV_FREE_MEM(&tmp_int, rqb_ptr->args.pd.alarm);
    if (LSA_OK != tmp_int)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, tmp_int);
    }
}

/**
 *  @brief callback for device provide event resources
 *
 *  @param[in]     rqb_ptr Pointer to request block
 *
 *  This callback handles CM_OPC_SV_DEVICE_PROVIDE_EVENT.
 *  Only triggered when resources should be freed.
 *  "Used" Resources change there opcode to indicate something.
 *
 */
PNIO_VOID pndv_in_cm_sv_device_provide_event_done(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;
    PNIO_UINT32 tmp_int;

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_SV_DEVICE_PROVIDE_EVENT_DONE, PNDV_RQB_GET_HANDLE(rqb_ptr));

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);
    if ( CM_OK_CANCELLED != response )
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }

    PNDV_FREE_MEM(&tmp_int, rqb_ptr->args.sv.dev_provide_event);
    if ( LSA_OK != tmp_int)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, tmp_int);
    }
}

/**
 *  @brief callback for device provide alarm resources
 *
 *  @param[in]     rqb_ptr Pointer to request block
 *
 *  This callback handles CM_OPC_SV_DEVICE_PROVIDE_ALARM.
 *  Only triggered when resources should be freed.
 *  "Used" Resources change there opcode to indicate something.
 *
 */
PNIO_VOID pndv_in_cm_sv_device_provide_alarm_done(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;
    PNIO_UINT32 tmp_int;

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_SV_DEVICE_PROVIDE_ALARM_DONE, PNDV_RQB_GET_HANDLE(rqb_ptr));

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);


    if (CM_OK_CANCELLED != response)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }

    PNDV_FREE_MEM( &tmp_int, rqb_ptr->args.sv.dev_provide_alarm);

    if (LSA_OK != tmp_int)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, tmp_int);
    }
}


/*****************************************************************************/


/**
 *  @brief callback for link status info
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  This callback handles CM_OPC_PD_LINK_STATUS_INFO.
 *
 */
PNIO_BOOL pndv_in_cm_link_status_info(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response,
                port,
                tmp_port_cnt;
    PNIO_BOOL   rqb_back;
    PNIO_UINT32 tmp_int;

    tmp_port_cnt = 0;
    rqb_back = PNIO_TRUE;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    if (CM_OK_CANCELLED == response)
    {
        rqb_back = PNIO_FALSE; /* Channel was closed RQB kept and released in pndv_cm_callback */
    }
    else
    {
        if ((CM_OK           != response) &&
            (CM_OK_REPLACED  != response))
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, response);
        }
    }

    pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_LINK_STATUS_IND,
                                         rqb_ptr->args.pd.link_status->status,
                                         (((PNIO_UINT32)rqb_ptr->args.pd.link_status->port_id)     |
                                          ((PNIO_UINT32)rqb_ptr->args.pd.link_status->speed) << 16 |
                                          ((PNIO_UINT32)rqb_ptr->args.pd.link_status->mode)  << 24));

/*
    LSA_UINT16 interface_id; / * out(!), see channel-info * /
    LSA_UINT16 port_id;      / * CM_PORT_ID_AUTO or 1..n (see channel-info) * /
    LSA_UINT8  status;       / * see cm_link_status_enum, out: never "unknown" * /

                CM_LINK_STATUS_UNKNOWN     = ACP_LINK_STATUS_UNKNOWN,
                CM_LINK_STATUS_UP          = ACP_LINK_STATUS_UP,
                CM_LINK_STATUS_DOWN        = ACP_LINK_STATUS_DOWN,
                CM_LINK_STATUS_UP_CLOSED   = ACP_LINK_STATUS_UP_CLOSED, / * same as "down" for normal communication *
                CM_LINK_STATUS_UP_DISABLED = ACP_LINK_STATUS_UP_DISABLED / * same as "down" for normal communication * /

    LSA_UINT8  speed;        / * see cm_link_speed_enum, out: valid if "up" * /

                CM_LINK_SPEED_UNKNOWN = ACP_LINK_SPEED_UNKNOWN,
                CM_LINK_SPEED_10_M    = ACP_LINK_SPEED_10_M,
                CM_LINK_SPEED_100_M   = ACP_LINK_SPEED_100_M,
                CM_LINK_SPEED_1_G     = ACP_LINK_SPEED_1_G,
                CM_LINK_SPEED_10_G    = ACP_LINK_SPEED_10_G

    LSA_UINT8  mode;         / * see cm_link_mode_enum, out: valid if "up" * /

                CM_LINK_MODE_UNKNOWN     = ACP_LINK_MODE_UNKNOWN,
                CM_LINK_MODE_HALF_DUPLEX = ACP_LINK_MODE_HALF_DUPLEX,
                CM_LINK_MODE_FULL_DUPLEX = ACP_LINK_MODE_FULL_DUPLEX

*/

    /* Save PortLinkStatus in pndv_data */

    pndv_data.port_link_status[(rqb_ptr->args.pd.link_status->port_id)-1] = rqb_ptr->args.pd.link_status->status;

    /* Test all link states on! = CM_LINK_STATUS_UP -> BF_LED control */

    for (port = 0; port < PNDV_MAX_PORT; port++)
    {
        if (CM_LINK_STATUS_UP != pndv_data.port_link_status[port])
        {
            tmp_port_cnt++;
        }
    }

    if (PNDV_MAX_PORT != tmp_port_cnt)
    {
        PNDV_CFG_MEASUREMENT_LINK_UP();
    }

    if (PNIO_FALSE == rqb_back)
    {
        /* Channel was closed RQB will release in pndv_cm_callback 
           -> also release argument memory
        */

        PNDV_FREE_MEM(&tmp_int, rqb_ptr->args.pd.link_status);
        if (LSA_OK != tmp_int)
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, tmp_int);
        }
    }

    return(rqb_back);
}

PNIO_VOID pndv_in_cm_provide_led_info_rqb( PNIO_VOID )
{
    CM_UPPER_RQB_PTR_TYPE tmp_rqb_ptr;

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_LED_INFO_REQ, 0);

    /*********************** device led info ********************************/

    tmp_rqb_ptr = &pndv_data.rqb.device_led_info;

    if (0 != PNDV_RQB_GET_OPCODE(tmp_rqb_ptr))
    {
        /* RQB in use */
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE( tmp_rqb_ptr));
    }

    tmp_rqb_ptr->args.sv.dev_led_info->device_nr  = PNDV_CM_DEVICE_NO; /* device number */
    tmp_rqb_ptr->args.sv.dev_led_info->led_info   = 0;      /* bit set = some submodules have diagnosis information */
    tmp_rqb_ptr->args.sv.dev_led_info->maint_info = 0;      /* bit set = some submodules have maintenance or qualified information */
    /*
     * bit  0 = Maintenance-Required
     * bit  1 = Maintenance-Demanded
     * bit  2 = Qualifier_2
     * ...
     * bit 31 = Qualifier_31
     *
     * NOTE: see PNIO-Spec "Values for MaintenanceStatus"
     */

    PNDV_RQB_SET_HANDLE(tmp_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
    PNDV_RQB_SET_OPCODE(tmp_rqb_ptr, CM_OPC_SV_DEVICE_LED_INFO);

    PNDV_REQUEST(tmp_rqb_ptr, LSA_COMP_ID_CM);

    /*********************** sync led info **********************************/
    tmp_rqb_ptr = &pndv_data.rqb.sync_led_info;

    if (0 != PNDV_RQB_GET_OPCODE(tmp_rqb_ptr))
    {
        /* RQB in use */

        pndv_in_fatal_error( PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE( tmp_rqb_ptr));
    }

    tmp_rqb_ptr->args.pd.sync_led_info->sync_led_info = 0;
    tmp_rqb_ptr->args.pd.sync_led_info->edd_port_id   = 0; // interface sync
    tmp_rqb_ptr->args.pd.sync_led_info->is_rate_valid = LSA_FALSE;
    tmp_rqb_ptr->args.pd.sync_led_info->local_prio    = 0;
    tmp_rqb_ptr->args.pd.sync_led_info->pll_state     = CM_PD_PLL_STATE_UNKNOWN;
    tmp_rqb_ptr->args.pd.sync_led_info->rcv_sync_prio = 0;

    PNDV_RQB_SET_HANDLE(tmp_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_CMPDSV_ACP]);
    PNDV_RQB_SET_OPCODE(tmp_rqb_ptr, CM_OPC_PD_SYNC_LED_INFO);

    PNDV_REQUEST(tmp_rqb_ptr, LSA_COMP_ID_CM);
}


/**
 *  @brief callback for device led info
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  This callback handles CM_OPC_SV_DEVICE_LED_INFO.
 *
 */
PNIO_BOOL pndv_in_cm_dev_led_info(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;
    PNIO_BOOL   rqb_back;

    rqb_back = PNIO_FALSE;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_DEV_LED_INFO_IND,
                                         (PNIO_UINT16)rqb_ptr->args.sv.dev_led_info->led_info,
                                         rqb_ptr->args.sv.dev_led_info->maint_info);

    if (CM_OK == response)
    {
        /* save state */
        pndv_glob_data.led_maint_info   = rqb_ptr->args.sv.dev_led_info->maint_info;
        pndv_glob_data.led_info         = rqb_ptr->args.sv.dev_led_info->led_info;

        /* Control SF LED */
        pndv_in_check_led_info();

        /* Control maintenance LED */
        pndv_check_maint_info();

        /* RQB is returned in pndv_cm_callback */

        rqb_back = PNIO_TRUE;
    }
    else
    {
        if (CM_OK_CANCELLED != response)
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
        }

        PNDV_RQB_SET_OPCODE(&pndv_data.rqb.device_led_info, 0);

        /* else: with CM_OK_CANCELLED there is no device -> keep RQB and mark as free 
        rqb_back = PNIO_FALSE has already been set above*/
    }

    return (rqb_back);
}

/**
 *  @brief callback for sync led info
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  This callback handles CM_OPC_PD_SYNC_LED_INFO.
 *
 */
PNIO_BOOL pndv_in_cm_sync_led_info(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;
    PNIO_BOOL   rqb_back;
    PNIO_UINT32 port_nr;

    rqb_back = PNIO_FALSE;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_SYNC_LED_INFO_IND,
                                         rqb_ptr->args.pd.sync_led_info->sync_led_info,
                                         rqb_ptr->args.pd.sync_led_info->edd_port_id);

    port_nr = rqb_ptr->args.pd.sync_led_info->edd_port_id;

    if (port_nr != 0)
    {
        // not supported
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, port_nr);
    }

    if (CM_OK == response)
    {
        pndv_glob_data.sync_led_info = rqb_ptr->args.pd.sync_led_info->sync_led_info;

        pndv_in_check_sync_info();

        rqb_back = PNIO_TRUE;
    }
    else
    {
        if (CM_OK_CANCELLED != response)
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
        }

        PNDV_RQB_SET_OPCODE(&pndv_data.rqb.sync_led_info, 0);

    }

    return( rqb_back);
}

static PNIO_VOID pndv_in_cm_get_sys_id_and_annotation_string(PNIO_UINT8** sys_id_str,     PNIO_UINT16* sys_id_str_len,
                                                             PNIO_UINT8** annotation_str, PNIO_UINT16* annotation_str_len)
{
    if (!pndv_data.are_annotation_and_sysident_strs_initialized)
    {
        PNIO_UINT32 hardware_revision                       = 0;
        PNIO_UINT8  software_revision[4]                    = {};
        PNIO_UINT8  order_id[MAX_ORDER_ID_LENGTH]           = {};
        PNIO_UINT8  device_type[MAX_DEVICE_TYPE_LENGTH]     = {};
        PNIO_UINT8  serial_number[MAX_SERIAL_NUMBER_LENGTH] = {};
        PNIO_UINT32 order_id_len                            = MAX_ORDER_ID_LENGTH;
        PNIO_UINT32 device_type_len                         = MAX_DEVICE_TYPE_LENGTH;
        PNIO_UINT32 serial_number_len                       = MAX_SERIAL_NUMBER_LENGTH;
        const PNIO_UINT16 SPACE_ASCII_NR                    = 0x20;

        pndv_data.are_annotation_and_sysident_strs_initialized = PNIO_TRUE;

        PNDV_GET_DEVICE_TYPE(device_type, device_type_len);
        PNDV_GET_ORDER_ID(order_id, order_id_len);
        PNDV_GET_SERIAL_NUMBER(serial_number, serial_number_len);
        PNDV_GET_HW_REVISION(hardware_revision);
        PNDV_GET_SW_REVISION(software_revision, (software_revision + 1), (software_revision + 2), (software_revision + 3));

        if ((device_type_len   > MAX_DEVICE_TYPE_LENGTH) ||
            (order_id_len      > MAX_ORDER_ID_LENGTH)    ||
            (serial_number_len > MAX_SERIAL_NUMBER_LENGTH))
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
        }


        // ------------------------- ANNOTATION STRING ---------------------------
        /* DeviceType           25 octets   char                                */
        /* Blank                1 octet     0x20                                */
        /* OrderID              20 octets   char                                */
        /* Blank                1 octet     0x20                                */
        /* HWRevision           5 octets    char "0"-"9" and "<Blank>"          */
        /* Blank                1 octet     0x20                                */
        /* SWRevisionPrefix     1 octet     "V","R","P","U" or "T"              */
        /* SWRevision           9 octets    char "0"-"9" and "<Blank>"          */
        /* EndTerm              1 octet     0x0                                 */
        /* see  CLRPC_PNIO_ANNOTATION_FORMAT                                    */

        // Inits annotation str with blanks
        PNDV_MEMSET(&pndv_data.annotation_str[0], SPACE_ASCII_NR, sizeof(pndv_data.annotation_str));
        // Device Type
        PNDV_COPY_BYTE(&pndv_data.annotation_str[PNDV_CMPD_ANNOT_STR_POS_DEV_TYPE], device_type, device_type_len);
        // Order ID
        PNDV_COPY_BYTE(&pndv_data.annotation_str[PNDV_CMPD_ANNOT_STR_POS_ORDER_ID], order_id, order_id_len);
        // Hardware Revision
        PNDV_SPRINTF(&pndv_data.annotation_str[PNDV_CMPD_ANNOT_STR_POS_HW_REV], "%5u", hardware_revision);
        pndv_data.annotation_str[PNDV_CMPD_ANNOT_STR_POS_SW_REV_PREF - 1] = ' ';
        // Software Revision Prefix
        pndv_data.annotation_str[PNDV_CMPD_ANNOT_STR_POS_SW_REV_PREF] = software_revision[0];
        // Software Revision Numbers
        PNDV_SPRINTF(&pndv_data.annotation_str[PNDV_CMPD_ANNOT_STR_POS_SW_REV1], "%3u", software_revision[1]);
        PNDV_SPRINTF(&pndv_data.annotation_str[PNDV_CMPD_ANNOT_STR_POS_SW_REV2], "%3u", software_revision[2]);
        PNDV_SPRINTF(&pndv_data.annotation_str[PNDV_CMPD_ANNOT_STR_POS_SW_REV3], "%3u", software_revision[3]);
        pndv_data.annotation_str[PNDV_CMPD_ANNOT_STR_MAX_LEN] = 0;


        // ------------------------- SYSTEM IDENTIFICATION STRING ---------------------------
        /* SystemIdentification ::= DeviceType, Blank, OrderID, Blank, IM_Serial_Number, Blank, 
        *                           HWRevision, Blank, SWRevisionPrefix, SWRevision
        * DeviceType        ... VisibleString[25] = DeviceVendorValue padded with blanks (1st char must be a non-blank)
        * OrderID           ... VisibleString[20]
        * IM_Serial_Number  ... VisibleString[16]
        * HWRevision        ... VisibleString[5] = "0" to "99999".
        * SWRevisionPrefix  ... VisibleString[1] = "V","R","P","U" or "T"
        * SWRevision        ... VisibleString[9] = "0  0  0" to "999999999"
        * See IEC 61158-6. and CM_PD_SYSTEM_IDENTIFICATION_FORMAT */

        // Inits system identification str with blanks
        PNDV_MEMSET(&pndv_data.system_identification_str[0], SPACE_ASCII_NR, sizeof(pndv_data.system_identification_str));
        // Device Type
        PNDV_COPY_BYTE(&pndv_data.system_identification_str[PNDV_CMPD_SYSIDENT_STR_POS_DEV_TYPE], device_type, device_type_len);
        // Order ID
        PNDV_COPY_BYTE(&pndv_data.system_identification_str[PNDV_CMPD_SYSIDENT_STR_POS_ORDER_ID], order_id, order_id_len);
        // Serial Number
        PNDV_COPY_BYTE(&pndv_data.system_identification_str[PNDV_CMPD_SYSIDENT_STR_POS_SERIAL_NR], serial_number, serial_number_len);
        // Hardware Revision
        PNDV_SPRINTF(&pndv_data.system_identification_str[PNDV_CMPD_SYSIDENT_STR_POS_HW_REV], "%5u", hardware_revision);
        pndv_data.system_identification_str[PNDV_CMPD_SYSIDENT_STR_POS_SW_REV_PREF - 1] = ' ';
        // Software Revision Prefix
        pndv_data.system_identification_str[PNDV_CMPD_SYSIDENT_STR_POS_SW_REV_PREF] = software_revision[0];
        // Software Revision Numbers
        PNDV_SPRINTF(&pndv_data.system_identification_str[PNDV_CMPD_SYSIDENT_STR_POS_SW_REV1], "%3u", software_revision[1]);
        PNDV_SPRINTF(&pndv_data.system_identification_str[PNDV_CMPD_SYSIDENT_STR_POS_SW_REV2], "%3u", software_revision[2]);
        PNDV_SPRINTF(&pndv_data.system_identification_str[PNDV_CMPD_SYSIDENT_STR_POS_SW_REV3], "%3u", software_revision[3]);
        pndv_data.system_identification_str[PNDV_CMPD_SYSIDENT_STR_MAX_LEN] = 0;
    }

    if (sys_id_str)
    {
        *sys_id_str         = pndv_data.system_identification_str;
        *sys_id_str_len     = PNDV_CMPD_SYSIDENT_STR_MAX_LEN;
    }

    if (annotation_str)
    {
        *annotation_str     = pndv_data.annotation_str;
        *annotation_str_len = PNDV_CMPD_ANNOT_STR_MAX_LEN + 1;
    }
}

/**
 *  @brief create the cmpd server
 *
 *  setup and create the cm physical device server CM_OPC_PD_CREATE.
 *
 */
PNIO_VOID pndv_in_cm_pdsv_create (PNIO_VOID)
{
    PNIO_UINT16 port_count;
    PNIO_UINT8*  sys_id_string;
    PNIO_UINT16  sys_id_string_len;

    union
    {
        CM_UPPER_RQB_PTR_TYPE rqb;
        LSA_VOID_PTR_TYPE     void_;
    }   ptr;

    PNDV_ALLOC_RQB(&ptr.void_, sizeof(CM_RQB_TYPE));
    if (pndv_host_ptr_is_nil__(ptr.rqb))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    PNDV_ALLOC_MEM(((LSA_VOID_PTR_TYPE)&ptr.rqb->args.pd.create),
                   (sizeof(CM_PD_CREATE_TYPE) + (pndv_data.cfg.pd.port_count_used * sizeof(CM_PD_MAP_TYPE)))); /* NOTE: the cm-user must alloc additional memory for the ports */
    if (pndv_host_ptr_is_nil__(ptr.rqb->args.pd.create) )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    ptr.rqb->args.pd.create->max_alarm_data_length          = CM_ALARM_DATA_LENGTH_MIN;
    ptr.rqb->args.pd.create->map_count                      = pndv_data.cfg.pd.port_count_used + 1;  /* Number of Ports + Interface */
    ptr.rqb->args.pd.create->oem_data                         = (CM_UPPER_OEM_DATA_PTR_TYPE) pndv_data.oem_data_ptr;

    pndv_in_cm_get_sys_id_and_annotation_string(&sys_id_string, &sys_id_string_len, LSA_NULL, LSA_NULL);

    ptr.rqb->args.pd.create->VendorID                 = PNDV_CM_VENDOR_ID;
    ptr.rqb->args.pd.create->DeviceID                 = pndv_data.cfg.device_id;
    ptr.rqb->args.pd.create->InstanceID               = 0;                             /* Evaluated if InstanceIDValid == CM_PD_INSTANCE_ID_VALID_YES, otherwise don't care (set to 0).     */
    ptr.rqb->args.pd.create->InstanceIDValid          = CM_PD_INSTANCE_ID_VALID_NO;    /* CM_PD_INSTANCE_ID_VALID_NO:  DCP SuboptionDeviceInstance will not be created.
                                                                                          CM_PD_INSTANCE_ID_VALID_YES: DCP SuboptionDeviceInstance with value "InstanceID" will be created. */
    ptr.rqb->args.pd.create->SystemIdentification     = sys_id_string;  /* SystemIdentification ::= DeviceType, Blank, OrderID, Blank, IM_Serial_Number, Blank, HWRevision, Blank, SWRevisionPrefix, SWRevision */
    ptr.rqb->args.pd.create->SystemIdentificationLen  = sys_id_string_len;

    ptr.rqb->args.pd.create->is_Controller            = CM_PD_IS_CONTROLLER_NO;
    ptr.rqb->args.pd.create->use_long_NameOfPort      = CM_PD_USE_LONG_NAMEOFPORT_NO;

    ptr.rqb->args.pd.create->clock_offset             = 0;      /* initial value of CM_GET_TIMESTAMP() in us (which maps to PSI_GET_TICKS_100NS()). Runtime starts at 0 */
    ptr.rqb->args.pd.create->enable_DHCPOption_legacy = CM_NO;  // there are no legacy DHCP devices
    ptr.rqb->args.pd.create->default_delay_protocol   = CM_PD_DEFAULT_DELAY_PROTOCOL_PTCP;  // default sync measurement method, if PDEV parameterization does not tell something else

    for (port_count = 0; port_count < pndv_data.cfg.pd.port_count_used + 1/*interface*/; port_count++)
    {
        ptr.rqb->args.pd.create->map[port_count].slot_nr                    = PNDV_IM_SLOT_NO;
        ptr.rqb->args.pd.create->map[port_count].mod_ident                  = pndv_data.cfg.pd.im_mod_ident;
        ptr.rqb->args.pd.create->map[port_count].subslot_nr                 = pndv_data.cfg.pd.interface_subslot_nr + port_count;
        ptr.rqb->args.pd.create->map[port_count].sub_ident                  = pndv_data.cfg.pd.port[port_count].submod_id;
        ptr.rqb->args.pd.create->map[port_count].im0_bits                   = pndv_data.cfg.pd.port[port_count].im_0_bits;

        if (port_count == 0)
        {
            // special case interface, must be COMPACT
            ptr.rqb->args.pd.create->map[port_count].pdev_properties        = CM_PDEV_PROP_TYPE_COMPACT|CM_PDEV_PROP_STATE_PLUGGED;
        }
        else
        {
#if (PNDV_CFG_PORT_IS_MODULAR == 1)
            ptr.rqb->args.pd.create->map[port_count].pdev_properties        = CM_PDEV_PROP_TYPE_MODULAR| (PNDV_CFG_IS_PDEV_PORT_PLUGGED(port_count) ? CM_PDEV_PROP_STATE_PLUGGED : CM_PDEV_PROP_STATE_PULLED);
#else
            ptr.rqb->args.pd.create->map[port_count].pdev_properties        = CM_PDEV_PROP_TYPE_COMPACT|CM_PDEV_PROP_STATE_PLUGGED;
#endif
        }
    }

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_PDSV_CREATE,0);

    PNDV_RQB_SET_HANDLE(ptr.rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_CMPDSV_ACP]);
    PNDV_RQB_SET_OPCODE(ptr.rqb, CM_OPC_PD_CREATE);

    PNDV_REQUEST(ptr.rqb, LSA_COMP_ID_CM);
}


/**
 *  @brief callback to created cmpd server
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  This callback handles CM_OPC_PD_CREATE.
 *
 */
PNIO_VOID pndv_in_cm_pdsv_create_done (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    union
    {
        CM_UPPER_RQB_PTR_TYPE rqb;
        LSA_VOID_PTR_TYPE     void_;
    }   ptr;


    PNIO_UINT32 response;
    PNIO_UINT16 i;
    PNIO_UINT32 tmp_int;

    response = PNDV_RQB_GET_RESPONSE( rqb_ptr);

    if (CM_OK != response)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, response);
    }

    // provide the requested number of PD event resources to CM.
    // They are used for CM_OPC_PD_EVENT_APPL_READY_IND signaling, later on ...
    pndv_in_cm_put_resource(rqb_ptr->args.pd.create->nr_of_event_resources , rqb_ptr->args.pd.create->nr_of_alarm_resources);

    // pd_create.cnf returns the interface ID that is to be used in IP2PN_OPC_IPSUITE_SET / IP2PN_OPC_RESET_TO_FACTORY. 1 for X1 .
    pndv_data.interface_id = rqb_ptr->args.pd.create->interface_id;

    PNDV_FREE_MEM(&tmp_int, rqb_ptr->args.pd.create);
    if ( LSA_OK != tmp_int)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, tmp_int);
    }

    /* link_status ressource */
    /* --------------------------------------------------------------------- */

    for (i = 0; i < PNDV_MAX_PORT ; i++)
    {
        PNDV_ALLOC_RQB( &ptr.void_, sizeof(CM_RQB_TYPE));
        if (pndv_host_ptr_is_nil__(ptr.rqb))
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
        }

        PNDV_ALLOC_MEM(((LSA_VOID_PTR_TYPE)&ptr.rqb->args.pd.link_status), sizeof(CM_PD_LINK_STATUS_INFO_TYPE));
        if (pndv_host_ptr_is_nil__(ptr.rqb->args.pd.link_status))
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }

      /*ptr.rqb->args.pd.link_status->interface_id  dont care*/
        ptr.rqb->args.pd.link_status->port_id      = i + 1;
        ptr.rqb->args.pd.link_status->status       = CM_LINK_STATUS_UNKNOWN;
        ptr.rqb->args.pd.link_status->speed        = CM_LINK_SPEED_UNKNOWN;
        ptr.rqb->args.pd.link_status->mode         = CM_LINK_MODE_UNKNOWN;
#ifndef PNDV_CFG_LINK_STATE_WITHOUT_AUTONEG
        ptr.rqb->args.pd.link_status->autoneg      = CM_LINK_AUTONEG_UNKNOWN;
#endif

        pndv_in_write_debug_buffer_al__(PNDV_DC_CM_LINK_STATUS_RES_PROV,i+1);

        PNDV_RQB_SET_HANDLE(ptr.rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_CMPDSV_ACP]);
        PNDV_RQB_SET_OPCODE(ptr.rqb, CM_OPC_PD_LINK_STATUS_INFO);

        PNDV_REQUEST(ptr.rqb, LSA_COMP_ID_CM);
    }

    pndv_sm(PNDV_SM_EVENT_CREATE_PD_SERVER_DONE);

}

/**
 *  @brief delete the cmpd server
 *
 *  delete the cm physical device server CM_OPC_PD_DELETE.
 *
 */
PNIO_VOID pndv_in_cm_pdsv_delete(PNIO_VOID)
{
    union
    {
        CM_UPPER_RQB_PTR_TYPE rqb;
        LSA_VOID_PTR_TYPE     void_;
    }   ptr;

    PNDV_ALLOC_RQB(&ptr.void_, sizeof(CM_RQB_TYPE));
    if (pndv_host_ptr_is_nil__(ptr.rqb))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_PDSV_DELETE,0);

    PNDV_RQB_SET_HANDLE(ptr.rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_CMPDSV_ACP]);
    PNDV_RQB_SET_OPCODE(ptr.rqb, CM_OPC_PD_DELETE);

    PNDV_REQUEST(ptr.rqb, LSA_COMP_ID_CM);
}

/**
 *  @brief callback to delete cmpd server
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  This callback handles CM_OPC_PD_DELETE.
 *
 *
 */
PNIO_VOID pndv_in_cm_pdsv_delete_done(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    if (CM_OK != response)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }

    pndv_sm(PNDV_SM_EVENT_DELETE_PD_SERVER_DONE);

}



/**
 *  @brief create the cm server
 *
 *  setup and create the cm server CM_OPC_SV_CREATE.
 *
 */
PNIO_VOID pndv_in_cm_sv_create(PNIO_VOID)
{
    union
    {
        CM_UPPER_RQB_PTR_TYPE rqb;
        LSA_VOID_PTR_TYPE     void_;
    }   ptr;

    PNDV_ALLOC_RQB(&ptr.void_, sizeof(CM_RQB_TYPE));
    if (pndv_host_ptr_is_nil__(ptr.rqb))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    PNDV_ALLOC_MEM(((LSA_VOID_PTR_TYPE)&ptr.rqb->args.sv.create), sizeof(CM_SV_CREATE_TYPE));
    if (pndv_host_ptr_is_nil__(ptr.rqb->args.sv.create))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }


    /* application must respond within this number of seconds */
    /* or call will be cancelled, 0=default */
    ptr.rqb->args.sv.create->enable_AlarmResponder      = LSA_FALSE;  /* alle Alarme werden durch CM automatisch quittiert */
    ptr.rqb->args.sv.create->max_alarm_data_length      = CM_ALARM_DATA_LENGTH_MIN;
    ptr.rqb->args.sv.create->is_MultiDevice             = CM_SV_IS_MULTIDEVICE_NO;     /* one device instance */
    /* LSA_FALSE ... all received alarms will be acknowledged internally with CM_PNIO_ERR_AlarmTypeNotSupported (AP01035514) */
    /* LSA_TRUE .... all received alarms will be indicated to the cm-user (who will acknowledge them) */
    ptr.rqb->args.sv.create->can_RecordDataReadQuery    = LSA_FALSE;
    /* LSA_FALSE ... reject a record-read with a RecordDataReadQuery-block */
    /* LSA_TRUE .... accept a record-read with a RecordDataReadQuery-block */
    /*               and the cm-user will handle it correctly */
    ptr.rqb->args.sv.create->can_ARFSUDataAdjust        = PNDV_HANDLE_ARFSU_DATA_ADJUST;
    /* LSA_FALSE ... the ARFSUDataAdjust record is handled internally */
    /* LSA_TRUE .... the ARFSUDataAdjust record will be indicated to the cm-user */
    /*               and the cm-user will handle it correctly */
    ptr.rqb->args.sv.create->enable_DataStatusIndication = LSA_TRUE;
    /* LSA_FALSE ... CM_OPC_SV_AR_DATA_STATUS_IND is not indicated to the cm-user */
    /* LSA_TRUE .... CM_OPC_SV_AR_DATA_STATUS_IND is indicated to the cm-user */
    ptr.rqb->args.sv.create->IOXS_Required               = PNDV_NO_DISCARD_IOXS_SUPPORT; /* corresponds to the GSDML key "IOXS_Required" (V6.0i2.1x, TIA 1218229) */
    /* LSA_FALSE ... SubmoduleProperties.DiscardIOXS is supported and the cm-user will handle it correctly */
    /* LSA_TRUE .... SubmoduleProperties.DiscardIOXS is not supported */


    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_SV_CREATE,0);

    PNDV_RQB_SET_HANDLE(ptr.rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
    PNDV_RQB_SET_OPCODE(ptr.rqb, CM_OPC_SV_CREATE);

    PNDV_REQUEST(ptr.rqb, LSA_COMP_ID_CM);
}


/**
 *  @brief callback to created cm server
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  This callback handles CM_OPC_SV_CREATE.
 *
 */
PNIO_VOID pndv_in_cm_sv_create_done (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;
    PNIO_UINT32 tmp_int;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    if (CM_OK != response)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }

    PNDV_FREE_MEM(&tmp_int, rqb_ptr->args.sv.create);

    if (LSA_OK != tmp_int)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, tmp_int);
    }

    pndv_sm(PNDV_SM_EVENT_CREATE_CM_SERVER_DONE);

}


/**
 *  @brief delete the cm server
 *
 *  delete the cm server CM_OPC_SV_DELETE.
 *
 */
PNIO_VOID pndv_in_cm_sv_delete (PNIO_VOID)
{
    union
    {
        CM_UPPER_RQB_PTR_TYPE rqb;
        LSA_VOID_PTR_TYPE     void_;
    }   ptr;

    PNDV_ALLOC_RQB(&ptr.void_, sizeof(CM_RQB_TYPE));
    if (pndv_host_ptr_is_nil__(ptr.rqb))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_SV_DELETE,0);

    PNDV_RQB_SET_HANDLE(ptr.rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
    PNDV_RQB_SET_OPCODE(ptr.rqb, CM_OPC_SV_DELETE);

    PNDV_REQUEST(ptr.rqb, LSA_COMP_ID_CM);
}


/**
 *  @brief callback to delete cmpd server
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  This callback handles CM_OPC_SV_DELETE.
 *
 *
 */
PNIO_VOID pndv_in_cm_sv_delete_done (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    if (CM_OK != response)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }

    pndv_sm(PNDV_SM_EVENT_CM_SV_DELETE_DONE);

}

/**
 *  @brief callback for control requests
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  Callback for control requests CM_OPC_SV_CONTROL.
 *  Service not used atm.
 *
 */
PNIO_VOID pndv_in_cm_sv_control_done (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_CONTROL_STATION_DONE, 0);

    if (CM_OK != response)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }
}

/**
 *  @brief check if requests are pending
 *
 *
 *  Check if there are some pending request.
 *
 */
PNIO_VOID pndv_in_cm_check_dev_req (PNIO_VOID)
{
    if ((0 != pndv_data.diag.cm_dev_req.pending) &&
        (0 == pndv_data.diag.cm_dev_req_running ))
    {
        /* Acceleration query
           -> is there anything to do and
           -> may I now (nothing is currently running)
        */

        if (PNDV_CM_DEV_REQ_DEVICE_ADD & pndv_data.diag.cm_dev_req.pending)
        {
            pndv_data.diag.cm_dev_req.pending &= ~PNDV_CM_DEV_REQ_DEVICE_ADD;
            pndv_in_cm_sv_device_add();
        }

        if (PNDV_CM_DEV_REQ_DEVICE_RMV & pndv_data.diag.cm_dev_req.pending)
        {
            pndv_data.diag.cm_dev_req.pending &= ~PNDV_CM_DEV_REQ_DEVICE_RMV;
            pndv_in_cm_sv_device_remove();
        }

        if (PNDV_CM_DEV_REQ_DEVICE_CONTROL & pndv_data.diag.cm_dev_req.pending)
        {
            pndv_data.diag.cm_dev_req.pending &= ~PNDV_CM_DEV_REQ_DEVICE_CONTROL;
            pndv_in_device_control(pndv_data.device_control);
        }

        /* not used
        if ( PNDV_CM_DEV_REQ_SUBMODULE_RMV & pndv_data.diag.cm_dev_req.pending )
        {
            pndv_data.diag.cm_dev_req.pending &= ~PNDV_CM_DEV_REQ_SUBMODULE_RMV;

        }
        */
    }
}


/**
 *  @brief register the device to cm
 *
 *
 *  setup and add the device to cm CM_OPC_SV_DEVICE_ADD.
 *
 */
PNIO_VOID pndv_in_cm_sv_device_add (PNIO_VOID)
{
    union
    {
        CM_UPPER_RQB_PTR_TYPE rqb;
        LSA_VOID_PTR_TYPE     void_;
    }   ptr;

    if (0 == pndv_data.diag.cm_dev_req_running)
    {
        PNIO_UINT8* annotation_string;
        PNIO_UINT16 annotation_string_len;

        PNDV_ALLOC_RQB(&ptr.void_, sizeof(CM_RQB_TYPE));
        if (pndv_host_ptr_is_nil__(ptr.rqb) )
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
        }

        PNDV_ALLOC_MEM(((LSA_VOID_PTR_TYPE)&ptr.rqb->args.sv.dev_add), sizeof(CM_SV_DEVICE_ADD_TYPE));
        if (pndv_host_ptr_is_nil__(ptr.rqb->args.sv.dev_add))
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
        }

        ptr.rqb->args.sv.dev_add->device_nr                     = PNDV_CM_DEVICE_NO;            /* device handle 1..max_devices, see "create server" */
        ptr.rqb->args.sv.dev_add->device_id                     = pndv_data.cfg.device_id;      /* see construction of PN IO object UUID */
        ptr.rqb->args.sv.dev_add->vendor_id                     = PNDV_CM_VENDOR_ID;            /* see construction of PN IO object UUID */
        ptr.rqb->args.sv.dev_add->instance                      = PNDV_CM_DEVICE_INSTANZ;       /* see construction of PN IO object UUID */

        pndv_in_cm_get_sys_id_and_annotation_string(LSA_NULL, LSA_NULL, &annotation_string, &annotation_string_len);
        PNDV_COPY_BYTE(&ptr.rqb->args.sv.dev_add->annotation[0],
                       annotation_string,
                       CLRPC_MAX_ANNOTATION_SIZE);

        ptr.rqb->args.sv.dev_add->max_ars_IOC                    = PNDV_MAX_ARS_RT;              /* see pndv_cfg.h */
        ptr.rqb->args.sv.dev_add->max_ars_IOS                    = 0;
        ptr.rqb->args.sv.dev_add->max_ars_DAC                    = PNDV_MAX_ARS_DA;              /* see pndv_cfg.h */
        ptr.rqb->args.sv.dev_add->max_ars_NumberOfImplicitAR     = 1;
        ptr.rqb->args.sv.dev_add->max_rpc_payload_size           = PNDV_MAX_RECORD_DATA_LEN;     // 4kb
            /*
             * max_ars_RT + max_ars_DA >= 1
             * max_ars_RT + max_ars_DA <= CM_CFG_MAX_SV_ARS (see cm_cfg.h)
             *
             * "RT" means ARs with IOCRProperties.RTClass == 1/2/3/UDP
             * "DA" means ARs with ARProperties.DeviceAccess == 1 (ARs without IOCRs)
             *
             * the "implicit AR" exists always (thus the name).
             */
        ptr.rqb->args.sv.dev_add->max_slot_nr                   = PNDV_CM_SV_MAX_SLOT_NR;            /* 1..CM_CFG_MAX_SV_SLOTS, see cm_cfg.h */
        ptr.rqb->args.sv.dev_add->max_subslot_nr                = PNDV_CM_SV_MAX_SUBSLOT_NR;
        ptr.rqb->args.sv.dev_add->max_nr_of_subslots            = PNDV_CM_SV_SUBSLOT_COUNT;
            /*
             * the submodules of an AR (expected config) are checked against these bounds
             * if slot_nr > max_slot_nr then the AR is rejected
             * if subslot_nr > max_subslot_nr then the AR is rejected
             * if number of submodules > max_nr_of_subslots then the AR is rejected
             * note that there is no "max_api"
             * memory usage is thus bounded by O(max_nr_of_subslots * max_ars_RT)
             */
        ptr.rqb->args.sv.dev_add->alarm_send_max_length         = PNDV_ALARM_SEND_MAX_LENGTH; /* size of the biggest alarm sent (V5.2i1, AP01232054) */
            /*
             * usecase: the user specifies the size of the biggest alarm he will ever send (CM_OPC_SV_AR_ALARM_SEND)
             * a value less than 172 (= CM_ALARM_DATA_LENGTH_MIN - CM_ALARM_OFFSET_DATA) will be rounded up
             *
             * req: 0..1404 (= CM_ALARM_DATA_LENGTH_MAX - CM_ALARM_OFFSET_DATA)
             * cnf: effective value (different from .req in case of rounding up)
             *
             * if alarm_send_max_length > CM_SV_AR_CONNECT_TYPE::alarm_send_max_length then the AR is rejected
             */
        ptr.rqb->args.sv.dev_add->contains_pdev                 = LSA_TRUE;
            /*
             * LSA_FALSE ... 0x8ipp submodules cannot be added.
             * LSA_TRUE .... 0x8ipp submodules can be added (this device is the pdev-device, AP00995373)
             */
        ptr.rqb->args.sv.dev_add->central_pdev                  = LSA_FALSE;
            /*
             * LSA_FALSE ... the PDEV can be owned and parameterized by an AR (if the device "contains_pdev").
             * LSA_TRUE .... the PDEV cannot be owned by an AR and is parameterized via CM-PD only (AP01066848).
             */

        ptr.rqb->args.sv.dev_add->parking_mode                  = LSA_TRUE;

            /*
             * for FSU (fast startup) devices only (AP00654135, AP00975630)
             * the use-case is described in the documentation
             *
             * LSA_FALSE ... normal mode
             * LSA_TRUE .... parking mode
             */
        ptr.rqb->args.sv.dev_add->write_multiple_parallelism    = 1;
            /*
             * number of parallel CM_OPC_SV_AR_WRITE_IND per AR (AP00786836, CM V4.2i2.1)
             * this parameter is effective for "real time ARs" only (not for "device access ARs")
             * 0 ... reserved
             * 1 ... one write-indication at a time (compatibility mode)
             *       the cm-user sees no difference between a write-single and a write-multiple
             * n ... at most n write-indications at a time
             *       the cm-user must be able to handle the parallel indications
             *       the write-indications can be responded out-of-order
             */
        if (PNDV_MAX_ARS_RT <= 1)
        {
            ptr.rqb->args.sv.dev_add->SRD_max_nr_of_arsets      = 0;
            ptr.rqb->args.sv.dev_add->R1_enabled                = LSA_FALSE; //R1 review + open: HIF new parameters, pcIOX sdb2ini / asom2ini, CM documentation (also dependencies on other things)
            ptr.rqb->args.sv.dev_add->R1_bridge_max             = 0;
            ptr.rqb->args.sv.dev_add->CIR_enabled               = LSA_FALSE;
            ptr.rqb->args.sv.dev_add->gsd_SharedDeviceSupported = LSA_FALSE; /* see GSDML/SharedDeviceSupported (TIA 1587591) */
        }
        else
        {
#ifndef PNDV_CFG_ENABLE_S2
            // workaround: no RTC3 support if SRD_enabled
            ptr.rqb->args.sv.dev_add->SRD_max_nr_of_arsets      = 0;
            ptr.rqb->args.sv.dev_add->R1_enabled                = LSA_FALSE; //R1 review + open: HIF new parameters, pcIOX sdb2ini / asom2ini, CM documentation (also dependencies on other things)
            ptr.rqb->args.sv.dev_add->R1_bridge_max             = 0;
            ptr.rqb->args.sv.dev_add->CIR_enabled               = LSA_FALSE;
            ptr.rqb->args.sv.dev_add->gsd_SharedDeviceSupported = LSA_TRUE; /* see GSDML/SharedDeviceSupported (TIA 1587591) */
#else
            ptr.rqb->args.sv.dev_add->SRD_max_nr_of_arsets      = 1;
            ptr.rqb->args.sv.dev_add->R1_enabled                = LSA_FALSE; //R1 review + open: HIF new parameters, pcIOX sdb2ini / asom2ini, CM documentation (also dependencies on other things)
            ptr.rqb->args.sv.dev_add->R1_bridge_max             = 0;
            ptr.rqb->args.sv.dev_add->CIR_enabled               = LSA_FALSE;// default = FALSE - see below
            ptr.rqb->args.sv.dev_add->gsd_SharedDeviceSupported = LSA_TRUE; /* see GSDML/SharedDeviceSupported (TIA 1587591) */

#ifdef PNDV_CFG_ENABLE_CIR
            ptr.rqb->args.sv.dev_add->CIR_enabled = LSA_TRUE;
#endif
#endif

        }

#ifdef PNDV_CFG_ENABLE_RS_INFO_BLOCK_SUPPORT
        ptr.rqb->args.sv.dev_add->gsd_ReportingSystemSupported  = LSA_TRUE; /* see GSDML/ReportingSystem (TIA 1645525) */
#else
        ptr.rqb->args.sv.dev_add->gsd_ReportingSystemSupported  = LSA_FALSE;
#endif

#ifdef PNDV_CFG_DISABLE_SHARED
        ptr.rqb->args.sv.dev_add->gsd_SharedDeviceSupported = LSA_FALSE;
#endif

            /*
             * the use-cases are described in the documentation
             *
             * SRD_enabled
             *   LSA_FALSE ... normal mode
             *   LSA_TRUE .... functionality "System Redundancy" enabled
             *
             * CIR_enabled
             *   LSA_FALSE ... normal mode
             *   LSA_TRUE .... functionality "Configuration in Run" enabled
             *   CIR can be enabled only if SRD is enabled
             *
             * R1_enabled
             *   LSA_FALSE ... normal mode
             *   LSA_TRUE .... functionality "R1" enabled
             *   R1 can be enabled only if SRD is enabled
             */

        ptr.rqb->args.sv.dev_add->PDevPrmInd_enabled = LSA_FALSE;
            /*
             * the use-case is described in the documentation
             * see CM_OPC_SV_AR_PDEVPRM_IND/_RSP
             */

        ptr.rqb->args.sv.dev_add->skip_check_100Mbps_FDX = LSA_FALSE;
            /*
             * LSA_FALSE ... normative behavior
             * LSA_TRUE .... non-normative behavior: skip "PdevCheckFailed" of CMDEV (see PNIO-Spec)
             */

        ptr.rqb->args.sv.dev_add->min_device_interval_31250ns = 0; /* e.g., 32 -> 32*31250ns = 1ms (AP01451871) */
            /*
             * 0 ... no limitation
             * n ... see GSDML/MinDeviceInterval
             *
             * if AR::send_clock * AR::reduction_ratio < min_device_interval then the AR is rejected
             */

        /* TIA 1453731 */
        /* GSDML/NumberOfAR ... see max_ars_IOC */
        ptr.rqb->args.sv.dev_add->gsd_MaxInputLength   = PNDV_MAX_IO_INPUT_LEN;   /* see GSDML/MaxInputLength */
        ptr.rqb->args.sv.dev_add->gsd_MaxOutputLength  = PNDV_MAX_IO_OUTPUT_LEN; /* see GSDML/MaxOutputLength */
        ptr.rqb->args.sv.dev_add->gsd_MaxDataLength    = PNDV_MAX_IO_IN_OUT_LEN;   /* see GSDML/MaxDataLength or calculated as defined in GSDML */
        ptr.rqb->args.sv.dev_add->gsd_NumberOfInputCR  = PNDV_MAX_ARS_RT;   /* see GSDML/NumberOfInputCR (per device) */
        ptr.rqb->args.sv.dev_add->gsd_NumberOfOutputCR = PNDV_MAX_ARS_RT;   /* see GSDML/NumberOfOutputCR (per device) */
        ptr.rqb->args.sv.dev_add->gsd_NumberOfAdditionalInputCR   = 0; /* see GSDML/NumberOfAdditionalInputCR */
        ptr.rqb->args.sv.dev_add->gsd_NumberOfAdditionalOutputCR  = 0; /* see GSDML/NumberOfAdditionalOutputCR */
        ptr.rqb->args.sv.dev_add->gsd_NumberOfMulticastProviderCR = 0; /* see GSDML/NumberOfAdditionalMulticastProviderCR (note that "Additional" is misleading) */
        ptr.rqb->args.sv.dev_add->gsd_NumberOfMulticastConsumerCR = 0; /* see GSDML/NumberOfMulticastConsumerCR */
        ptr.rqb->args.sv.dev_add->gsd_AssetManagementSupported = PNDV_CFG_IS_ASSET_MANAGEMENT_SUPPORTED;    /* if true, the AMFilterData block is appended to
                                                                                                               the I&M0FilterData blocks when reading 0xF840. User can read 0xF880. */

        ptr.rqb->args.sv.dev_add->nr_of_event_resources         = 0;
            /*
             * req: don't care
             * cnf: the cm-user must provide this many CM_OPC_SV_DEVICE_PROVIDE_EVENT resources
             */
        ptr.rqb->args.sv.dev_add->nr_of_alarm_resources         = 0;
            /*
             * req: don't care
             * cnf: the cm-user must provide this many CM_OPC_SV_DEVICE_PROVIDE_ALARM resources
             */


        pndv_in_write_debug_buffer_all__(PNDV_DC_CM_DV_ADD,0);

        PNDV_RQB_SET_HANDLE(ptr.rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
        PNDV_RQB_SET_OPCODE(ptr.rqb, CM_OPC_SV_DEVICE_ADD);

        pndv_data.diag.cm_dev_req_running = PNDV_CM_DEV_REQ_DEVICE_ADD;

        PNDV_REQUEST(ptr.rqb, LSA_COMP_ID_CM);
    }
    else
    {
        pndv_data.diag.cm_dev_req.pending |= PNDV_CM_DEV_REQ_DEVICE_ADD;
    }
}


/**
 *  @brief callback to device add
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  Callback to handle CM_OPC_SV_DEVICE_ADD.
 *  Also provides some device event (CM_OPC_SV_DEVICE_PROVIDE_EVENT)
 *  and alarm resources (CM_OPC_SV_DEVICE_PROVIDE_ALARM) to cm.
 *
 */
PNIO_VOID pndv_in_cm_sv_device_add_done (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    CM_UPPER_RQB_PTR_TYPE local_rqb_ptr;
    PNIO_UINT32 response;
    PNIO_UINT32 tmp_int;
    LSA_UINT16  local_nr_alarm_resources, local_nr_event_resources;
    PNIO_UINT16 i;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);
    if (CM_OK != response)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    local_nr_event_resources = rqb_ptr->args.sv.dev_add->nr_of_event_resources;
    local_nr_alarm_resources = rqb_ptr->args.sv.dev_add->nr_of_alarm_resources;

    PNDV_FREE_MEM(&tmp_int, rqb_ptr->args.sv.dev_add);

    /* Event-Ressourcen CM-SV */
    /* --------------------------------------------------------------------- */

    for (i = 0; i < local_nr_event_resources ; i++)
    {
        PNDV_ALLOC_RQB(&local_rqb_ptr, sizeof(CM_RQB_TYPE));
        if (pndv_host_ptr_is_nil__(local_rqb_ptr))
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }

        PNDV_ALLOC_MEM(((LSA_VOID_PTR_TYPE)&local_rqb_ptr->args.sv.dev_provide_event), sizeof(CM_EVENT_TYPE));
        if (pndv_host_ptr_is_nil__(local_rqb_ptr->args.sv.dev_provide_event))
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
        }

        pndv_in_write_debug_buffer_all__(PNDV_DC_CM_RES_PROV,i);

        local_rqb_ptr->args.sv.dev_provide_event->device_nr = PNDV_CM_DEVICE_NO;

        PNDV_RQB_SET_HANDLE(local_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
        PNDV_RQB_SET_OPCODE(local_rqb_ptr, CM_OPC_SV_DEVICE_PROVIDE_EVENT);

        PNDV_REQUEST(local_rqb_ptr, LSA_COMP_ID_CM);
    }


    /* Event-Ressourcen CM-SV */
    /* --------------------------------------------------------------------- */

    for ( i = 0; i < local_nr_alarm_resources ; i++ )
    {
        PNDV_ALLOC_RQB(&local_rqb_ptr, sizeof(CM_RQB_TYPE));
        if (pndv_host_ptr_is_nil__(local_rqb_ptr))
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
        }

        PNDV_ALLOC_MEM(((LSA_VOID_PTR_TYPE)&local_rqb_ptr->args.sv.dev_provide_alarm), sizeof(CM_ALARM_TYPE));
        if (pndv_host_ptr_is_nil__(local_rqb_ptr->args.sv.dev_provide_alarm))
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
        }

        pndv_in_write_debug_buffer_al__(PNDV_DC_CM_AL_RES_PROV,i);

        local_rqb_ptr->args.sv.dev_provide_alarm->device_nr = PNDV_CM_DEVICE_NO;

        PNDV_RQB_SET_HANDLE(local_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
        PNDV_RQB_SET_OPCODE(local_rqb_ptr, CM_OPC_SV_DEVICE_PROVIDE_ALARM);

        PNDV_REQUEST(local_rqb_ptr, LSA_COMP_ID_CM);
    }

    if (LSA_OK != tmp_int)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, tmp_int);
    }


    pndv_data.diag.cm_dev_req_running = 0;

    /* Call of pndv_in_al_check_pp_req not necessary, the pndv_sm not yet in ..AP_ADD */

    pndv_sm(PNDV_SM_EVENT_DEVICE_ADD_DONE);

    /* cm_dev_req_running is 0 -> possibly other dev req rep. */
    pndv_in_cm_check_dev_req();
}


/**
 *  @brief remove device from cm
 *
 *  Remove the device from cm CM_OPC_SV_DEVICE_REMOVE.
 *
 */
PNIO_VOID pndv_in_cm_sv_device_remove (PNIO_VOID)
{
    union
    {
        CM_UPPER_RQB_PTR_TYPE rqb;
        LSA_VOID_PTR_TYPE     void_;
    }   ptr;

    if (0 == pndv_data.diag.cm_dev_req_running)
    {
        PNDV_ALLOC_RQB(&ptr.void_, sizeof(CM_RQB_TYPE));
        if (pndv_host_ptr_is_nil__(ptr.rqb))
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
        }

        PNDV_ALLOC_MEM(((LSA_VOID_PTR_TYPE)&ptr.rqb->args.sv.dev_remove), sizeof(CM_SV_DEVICE_REMOVE_TYPE));
        if ( pndv_host_ptr_is_nil__(ptr.rqb->args.sv.dev_remove) )
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
        }

        ptr.rqb->args.sv.dev_remove->device_nr = PNDV_CM_DEVICE_NO;              /* device handle 1..max_devices, see "create server" */

        pndv_in_write_debug_buffer_all__(PNDV_DC_CM_DV_REMOVE,0);

        PNDV_RQB_SET_HANDLE(ptr.rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
        PNDV_RQB_SET_OPCODE(ptr.rqb, CM_OPC_SV_DEVICE_REMOVE);

        pndv_data.diag.cm_dev_req_running = PNDV_CM_DEV_REQ_DEVICE_RMV;

        PNDV_REQUEST(ptr.rqb, LSA_COMP_ID_CM);
    }
    else
    {
        pndv_data.diag.cm_dev_req.pending |= PNDV_CM_DEV_REQ_DEVICE_RMV;
    }
}

/**
 *  @brief callback to device remove
 *
 *  Callback to device remove from cm CM_OPC_SV_DEVICE_REMOVE.
 *
 */
PNIO_VOID pndv_in_cm_sv_device_remove_done (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;
    PNIO_UINT32 tmp_int;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    if (CM_OK != response)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }

    PNDV_FREE_MEM( &tmp_int, rqb_ptr->args.sv.dev_remove);

    if (LSA_OK != tmp_int)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, tmp_int);
    }

    pndv_data.diag.cm_dev_req_running = 0;
    /* Device was just removed so that all device requests would lead to an error 
     * and are no longer relevant anyway. Calling pndv_in_cm_check_devreq () is therefore 
     * no longer necessary.
     */
    pndv_data.diag.cm_dev_req.pending = 0;
    /* Call of pndv_in_al_check_pp_req not necessary, the pndv_sm not yet in ..AP_ADD */

    pndv_sm(PNDV_SM_EVENT_DEVICE_REMOVE_DONE);

}

/**
 *  @brief callback for device read request
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  Callback for device read request CM_OPC_SV_DEVICE_READ.
 *  Service not used atm.
 *
 */
#ifdef PNDV_CFG_USE_DEVICE_READ
PNIO_VOID pndv_in_cm_sv_device_read_done(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32         response;
    PNDV_RQB_PTR_TYPE   local_rqb_ptr;
    LSA_UINT16          ret_val;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    if (CM_OK != response)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }

    local_rqb_ptr = pndv_data.usr.dev_read.usr_rqb;

    local_rqb_ptr->args->dev_read.pnio_status = rqb_ptr->args.sv.dev_read->pnio_status;
    local_rqb_ptr->args->dev_read.data_length = rqb_ptr->args.sv.dev_read->data_length;

    PNDV_FREE_MEM(&ret_val, rqb_ptr->args.sv.dev_read);
    if (ret_val != PNDV_OK)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, ret_val);
    }

    PNDV_FREE_MEM(&ret_val, rqb_ptr);
    if (ret_val != PNDV_OK)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, ret_val);
    }

    PNDV_RQB_SET_RESPONSE(local_rqb_ptr, PNDV_OK);

    pndv_data.usr.dev_read.service_running = PNIO_FALSE;

    PNDV_CFG_USR_REQ_DONE(local_rqb_ptr);

}
#endif

/**
 *  @brief starts local prm phase (PD) at startup
 *
 *
 *  Send CM_OPC_PD_PRM_PREPARE to indicate start of the local
 *  startup parameter phase.
 *
 */
PNIO_VOID pndv_in_cm_pd_prm_begin (PNIO_VOID)
{
    union
    {
        CM_UPPER_RQB_PTR_TYPE rqb;
        LSA_VOID_PTR_TYPE     void_;
    }   ptr;

    if (PNDV_SM_DEVICE_ACTIVE > pndv_data.sm)
    {
        PNDV_ALLOC_RQB( &ptr.void_, sizeof(CM_RQB_TYPE));
        if (pndv_host_ptr_is_nil__(ptr.rqb))
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
        }

        PNDV_RQB_SET_HANDLE(ptr.rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_CMPDSV_ACP]);
        PNDV_RQB_SET_OPCODE(ptr.rqb, CM_OPC_PD_PRM_BEGIN);

        pndv_in_write_debug_buffer_all__(PNDV_DC_CM_PD_PRM_PREPARE,0);

        PNDV_REQUEST(ptr.rqb, LSA_COMP_ID_CM);
    }
    else
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, pndv_data.sm);
    }
}

//PNIO_VOID PNDV_GET_PRM_LIST (PNDV_PDEV_PRM_RQB_QUEUE_PTR_T prm_queue_ptr)
PNIO_VOID pndv_in_cm_pd_build_prm_list(PNDV_PDEV_PRM_RQB_QUEUE_PTR_T prm_queue_ptr)
{
    PNDV_PRM_REQUEST_ELEMENT_PTR_T local_elem_ptr;

    /* prepare rqb for multiple pdev data */
    local_elem_ptr = PNDV_LIST_FIRST(&(prm_queue_ptr->free_list), PNDV_PRM_REQUEST_ELEMENT_PTR_T);
    if (local_elem_ptr == LSA_NULL)
    {
        /* no elements in free list, must not be here */
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }
    else
    {
        PNIO_UINT32* data_ptr = 0;
        PNIO_UINT32  data_length;
        /* Are there permanently stored PDEV data records? */

        PNDV_GET_PD_DS(&data_ptr, &data_length );

        if (data_ptr)
        {

            CM_UPPER_RQB_PTR_TYPE    rqb_ptr = local_elem_ptr->rqb_ptr;

            PNDV_ALLOC_MEM(&(rqb_ptr->args.pd.prm_rdwr->record_data), data_length);

            rqb_ptr->args.pd.prm_rdwr->record_index         = PNDV_RECORD_INDEX_MULTIPLE_PDEV;
            rqb_ptr->args.pd.prm_rdwr->record_data_length   = data_length;
                                                              /* Address of the first data element of the record_data */
                                                              /* in its original structure, the ptr stands on the record_data */
            rqb_ptr->args.pd.prm_rdwr->mk_remanent          = PNIO_TRUE;
            PNDV_COPY_BYTE(rqb_ptr->args.pd.prm_rdwr->record_data, data_ptr, data_length);

            PNDV_LIST_REMOVE_ENTRY(&local_elem_ptr->link);
            PNDV_LIST_INSERT_TAIL(&(prm_queue_ptr->in_use), &local_elem_ptr->link);
        }
    }

    /* prepare rqb for name of station data */
    local_elem_ptr = PNDV_LIST_FIRST(&(prm_queue_ptr->free_list), PNDV_PRM_REQUEST_ELEMENT_PTR_T);
    if (local_elem_ptr == LSA_NULL)
    {
        /* no elements in free list, must not be here */
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }
    else
    {
        PNIO_UINT32* data_ptr    = 0;
        PNIO_UINT16  data_length = 0;
        /* Are there permanently stored PDEV data records? */

        PNDV_GET_NOS_DATA (&data_ptr, &data_length );

        if (data_ptr)
        {
            CM_UPPER_RQB_PTR_TYPE     rqb_ptr = local_elem_ptr->rqb_ptr;
            CM_COMMON_MEM_U8_PTR_TYPE data_copy_ptr;
            PNIO_UINT32               copy_pos = 0;
            PNIO_UINT32               total_rec_len = (PNDV_PRM_REC_BLOCK_LEN_NAME_OF_STATION + data_length + 3)>>2;

            total_rec_len = total_rec_len<<2;

            PNDV_ALLOC_MEM(((LSA_VOID_PTR_TYPE)&rqb_ptr->args.pd.prm_rdwr->record_data), (PNDV_PRM_REC_BLOCK_HEADER_LEN + total_rec_len));

            data_copy_ptr = rqb_ptr->args.pd.prm_rdwr->record_data;


            /* prepare record data header */
            data_copy_ptr[copy_pos++] = (PNIO_UINT8)(PNDV_PRM_REC_BLOCK_TYPE_NAME_OF_STATION >> 8);
            data_copy_ptr[copy_pos++] = (PNIO_UINT8)(PNDV_PRM_REC_BLOCK_TYPE_NAME_OF_STATION);
            data_copy_ptr[copy_pos++] = (PNIO_UINT8)((total_rec_len) >> 8);
            data_copy_ptr[copy_pos++] = (PNIO_UINT8)((total_rec_len));
            data_copy_ptr[copy_pos++] = (PNIO_UINT8)(PNDV_RM_REC_BLOCK_VERS_H);
            data_copy_ptr[copy_pos++] = (PNIO_UINT8)(PNDV_RM_REC_BLOCK_VERS_L);
            data_copy_ptr[copy_pos++] = (PNIO_UINT8)(0x00);
            data_copy_ptr[copy_pos++] = (PNIO_UINT8)(0x00);
            data_copy_ptr[copy_pos++] = (PNIO_UINT8)(data_length >> 8);
            data_copy_ptr[copy_pos++] = (PNIO_UINT8)(data_length);
            data_copy_ptr[copy_pos++] = (PNIO_UINT8)(0x00);
            data_copy_ptr[copy_pos++] = (PNIO_UINT8)(0x00);
            PNDV_COPY_BYTE((CM_COMMON_MEM_U8_PTR_TYPE)&data_copy_ptr[copy_pos], (CM_COMMON_MEM_U8_PTR_TYPE)data_ptr, data_length);
            copy_pos += data_length;
            while (copy_pos < total_rec_len + PNDV_PRM_REC_BLOCK_HEADER_LEN)
            {
                data_copy_ptr[copy_pos++] = (PNIO_UINT8)(0x00);
            }

            rqb_ptr->args.pd.prm_rdwr->record_index         = PNDV_RECORD_INDEX_NAME_OF_STATION;
            rqb_ptr->args.pd.prm_rdwr->slot_nr              = PNDV_IM_SLOT_NO;
            rqb_ptr->args.pd.prm_rdwr->subslot_nr           = pndv_data.cfg.pd.interface_subslot_nr;
            rqb_ptr->args.pd.prm_rdwr->record_data_length   = PNDV_PRM_REC_BLOCK_HEADER_LEN + total_rec_len;
                                                              /* Address of the first data element of the record_data */
                                                              /* in its original structure, the ptr stands on the record_data */
            rqb_ptr->args.pd.prm_rdwr->mk_remanent          = PNIO_TRUE;

            PNDV_LIST_REMOVE_ENTRY(&local_elem_ptr->link);
            PNDV_LIST_INSERT_TAIL(&(prm_queue_ptr->in_use), &local_elem_ptr->link);
        }
    }
}


/**
 *  @brief callback to prm prepare (PD)
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  Callback to prm prepare request CM_OPC_PD_PRM_PREPARE.
 *  Also starts sending local parameter to cm.
 *
 */
PNIO_VOID pndv_in_cm_pd_prm_begin_done(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;
    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    if (CM_OK != response)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, response);
    }

    pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_PD_PRM_PREPARE_DONE, pndv_data.cmpd_prm_default, pndv_data.reset_reason);

    if ((PNIO_TRUE == pndv_data.cmpd_prm_default) ||
        (PNDV_RESET_REASON_CODE_ERR_PRM_CONSISTENCY == pndv_data.reset_reason ))
    {
        /* After the parameterization has been canceled, an immediate PNDV_DC_CM_PD_PRM_END takes place */
        /* -> default parameterization of PDEV */
        pndv_in_cm_pd_prm_end ( );
    }
    else
    {
        if (PNIO_TRUE == pndv_data.stop_req)
        {
            pndv_in_stop();
        }
        else
        {
            /* init prm record list */
            PNDV_GET_PRM_LIST(&pndv_data.prm_queue);
            /* check for prm records to send */
            pndv_in_cm_pd_prm_write();
        }
    }
}

/**
 *  @brief write parameter to physical device
 *
 *  @param[in]  pData Pointer of data to be writen
 *  @param[in]  DataLength Lenght of data to be writen
 *
 *  Sends the given data to cm using PNDV_DC_CM_PD_PRM_WRITE request.
 *  Data is expected to be a multiple pdev record.
 *
 */
PNIO_VOID pndv_in_cm_pd_prm_write()
{
    PNDV_PRM_REQUEST_ELEMENT_PTR_T local_elem_ptr;
    /* Are there permanently stored PDEV data records? */

    local_elem_ptr = PNDV_LIST_FIRST(&pndv_data.prm_queue.in_use, PNDV_PRM_REQUEST_ELEMENT_PTR_T);
    if (local_elem_ptr == LSA_NULL)
    {
        /* all elements processed or no remanently stored PDEV data sets in flash */
        pndv_in_cm_pd_prm_end( );
    }
    else
    {
        CM_UPPER_RQB_PTR_TYPE rqb_ptr = local_elem_ptr->rqb_ptr;

        PNDV_LIST_REMOVE_ENTRY(&local_elem_ptr->link);

        PNDV_RQB_SET_HANDLE(rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_CMPDSV_ACP]);
        PNDV_RQB_SET_OPCODE(rqb_ptr, CM_OPC_PD_PRM_WRITE);

        pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_PD_PRM_WRITE, 0, rqb_ptr->args.pd.prm_rdwr->record_index);

        PNDV_REQUEST(rqb_ptr, LSA_COMP_ID_CM);

        //! append to free list
        PNDV_LIST_INSERT_TAIL(&pndv_data.prm_queue.free_list, &local_elem_ptr->link);
    }
}


/**
 *  @brief callback to prm write
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  Callback to prm write request (CM_OPC_PD_PRM_WRITE).
 *  Paramter where send to cm. If data was unplausible default parameters are
 *  prepared otherwise the prm phase will be ended.
 *
 */
PNIO_VOID pndv_in_cm_pd_prm_write_done (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    if (CM_OK != response)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }

    if (CM_PNIO_ERR_NONE != rqb_ptr->args.pd.prm_rdwr->cm_pnio_err)
    {
        /* Written DS was inconsistent in content or incorrect with regard to slot, subslot, index.  */
        /* Resetting PNDV_DC_CM_PD_PRM_PREPARE causes the ongoing parameterization to be canceled     */

        pndv_data.cmpd_prm_default  = PNIO_TRUE;

        // pndv_in_cm_pd_prm_abort();
    }

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_PD_PRM_WRITE_DONE, (PNIO_UINT16)rqb_ptr->args.pd.prm_rdwr->record_index);

    if (PNIO_TRUE == pndv_data.stop_req)
    {
        pndv_in_stop();
    }
    else
    {
        /* check are there more parameters to write */
        pndv_in_cm_pd_prm_write();
    }
}



/**
 *  @brief indicate end of parameter phase (PD)
 *
 *
 *  Send a CM_OPC_PD_PRM_END request to cm to indicate the end of
 *  pdev paramter phase.
 *
 */
PNIO_VOID pndv_in_cm_pd_prm_end (PNIO_VOID)
{
    union
    {
        CM_UPPER_RQB_PTR_TYPE rqb;
        LSA_VOID_PTR_TYPE     void_;
    }   ptr;

    PNDV_ALLOC_RQB(&ptr.void_, sizeof(CM_RQB_TYPE));
    if (pndv_host_ptr_is_nil__(ptr.rqb))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    PNDV_ALLOC_MEM(((LSA_VOID_PTR_TYPE)&ptr.rqb->args.pd.prm_end), sizeof(CM_EVENT_TYPE));
    if (pndv_host_ptr_is_nil__(ptr.rqb->args.pd.event))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    ptr.rqb->args.pd.prm_end->set_mrp_off = LSA_FALSE;

    PNDV_RQB_SET_HANDLE(ptr.rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_CMPDSV_ACP]);
    PNDV_RQB_SET_OPCODE(ptr.rqb, CM_OPC_PD_PRM_END);

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_PD_PRM_END,0);

    PNDV_REQUEST(ptr.rqb, LSA_COMP_ID_CM);
}


/**
 *  @brief callback to prm end (PD)
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  Callback to parameter end request of the physical device
 *  (CM_OPC_PD_PRM_END).
 *
 */
PNIO_VOID pndv_in_cm_pd_prm_end_done (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;
    PNIO_UINT32 tmp_int;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);
    if (CM_OK != response)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }

    PNDV_FREE_MEM( &tmp_int, rqb_ptr->args.pd.prm_end);
    if (LSA_OK != tmp_int)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, tmp_int);
    }

    /* Reset flag for possibly triggered default parameterization */
    pndv_data.cmpd_prm_default = PNIO_FALSE;

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_PD_PRM_END_DONE,0);
}

/**
 *  @brief Handle appl ready indication
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  Handler for appl ready indication (CM_OPC_PD_EVENT_APPL_READY_IND).
 *  Resets device on inconsistent parameter error or gives CM_OPC_PD_EVENT_APPL_READY_RSP
 *  to cm.
 *
 */
PNIO_VOID pndv_in_cm_pd_event_appl_ready_ind(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT16 response;
    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    pndv_in_write_debug_buffer_al__(PNDV_DC_CM_PD_EVENT_APPL_READY_IND,response);

    switch (response)
    {
        case CM_OK:
        case CM_OK_RESET_TO_FACTORY:
        {
            break;
        }
        case CM_ERR_PRM_CONSISTENCY:
        {
            if (pndv_data.sm != PNDV_SM_DEVICE_ACTIVE) /* don't reset device, this indication is not the result of setting the startup parameter */
            {
                // Reset device is not implemented
            }
            break;
        }

        default:
        {
            /* 070104 AnRe here a specific evaluation of the .u.pd.appl_ready.diff_list has to be done -> CM-Spec 9.13 ??? */
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, (PNIO_UINT32)response);
            break;
        }
    }

    pndv_in_write_debug_buffer_al__(PNDV_DC_CM_PD_EVENT_APPL_READY_IND,0);

    /* Parameterization of the PDEV submodules */
    /* -> switch state and possibly insert peri-module                                                     */
    /* From basis 5.3 there is also an indication with every AR setup / PDEV parameterization.             */
    /* --------------------------------------------------------------------------------------------------- */

    /* PDEV is ready, device can be activated */
    if (pndv_data.sm != PNDV_SM_DEVICE_ACTIVE) /* don't trigger pndv_sm after device activate, this indications are a result of an ar establishment */
    {
        pndv_sm(PNDV_SM_EVENT_PRM_PDEV_DONE);
    }

    PNDV_RQB_SET_HANDLE(rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_CMPDSV_ACP]);
    PNDV_RQB_SET_OPCODE(rqb_ptr, CM_OPC_PD_EVENT_APPL_READY_RSP);

    PNDV_REQUEST(rqb_ptr, LSA_COMP_ID_CM);
}

/**
 *  @brief set name of station (PD)
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  Request NoS change (CM_OPC_PD_SET_ADDRESS)
 */
PNIO_VOID pndv_in_cm_pd_set_address(const PNIO_VOID* pStationName, LSA_UINT16 StationNameLen)
{
    CM_UPPER_RQB_PTR_TYPE p_rqb;
    CM_UPPER_MEM_PTR_TYPE pNos;

    PNDV_ALLOC_RQB(&p_rqb, sizeof(CM_RQB_TYPE));
    if (pndv_host_ptr_is_nil__(p_rqb))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    PNDV_ALLOC_MEM(((LSA_VOID_PTR_TYPE)&p_rqb->args.pd.set_address), sizeof(CM_PD_SET_ADDRESS_TYPE));
    if (pndv_host_ptr_is_nil__(p_rqb->args.pd.set_address))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    PNDV_ALLOC_MEM(&pNos, StationNameLen);
    if (pndv_host_ptr_is_nil__(p_rqb->args.pd.set_address))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }
    PNDV_COPY_BYTE(pNos, pStationName, StationNameLen);

    CM_UPPER_PD_SET_ADDRESS_PTR_TYPE addr = p_rqb->args.pd.set_address;
    addr->Value_ptr  = pNos;
    addr->Value_len  = StationNameLen;
    addr->Option     = CM_DCPCTRL_NAMEOFSTATION;
    addr->RemaOption = CM_DCPCTRL_NAMEOFSTATION; // Remanent

    pndv_in_write_debug_buffer_al__(PNDV_DC_CM_PD_SET_ADDRESS, 0);

    PNDV_RQB_SET_HANDLE(p_rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_CMPDSV_ACP]);
    PNDV_RQB_SET_OPCODE(p_rqb, CM_OPC_PD_SET_ADDRESS);

    PNDV_REQUEST(p_rqb, LSA_COMP_ID_CM);
}

/**
 *  @brief handles the response of set name of station (PD)
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  Handles the response to the CM_OPC_PD_SET_ADDRESS request
 */
PNIO_VOID pndv_in_cm_pd_set_address_done(CM_UPPER_RQB_PTR_TYPE p_rqb)
{
    PNIO_UINT16 response = PNDV_RQB_GET_RESPONSE(p_rqb);
    pndv_in_write_debug_buffer_al__(PNDV_DC_CM_PD_SET_ADDRESS_DONE, response);
    if (CM_OK != response)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }

    PNIO_UINT32 free;
    // Allocated in pndv_in_cm_pd_set_address()
    PNDV_FREE_MEM(&free, p_rqb->args.pd.set_address);
    if (LSA_OK != free)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, free);
    }
    PNDV_FREE_MEM(&free, p_rqb->args.pd.set_address->Value_ptr);
    if (LSA_OK != free)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, free);
    }

    pndv_in_write_debug_buffer_al__(PNDV_DC_CM_PD_SET_ADDRESS_DONE, 0);
}

/**
 *  @brief Provide Rema Indication Resource
 *
 * This function provides an rqb resource to CM that is returned by CM when a Rema indication 
 * comes for NoS or PDEV data changes with opc CM_OPC_PD_REMA_INDICATION
 *
 */
PNIO_VOID pndv_in_cm_pd_rema_indication_provide_resource()
{
    CM_UPPER_RQB_PTR_TYPE pRemaRqb = &pndv_data.rqb.rema_ind_rqb;

    if (pndv_data.rqb.rema_rqb_in_use)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }
    pndv_data.rqb.rema_rqb_in_use = PNIO_TRUE;

    pRemaRqb->args.pd.rema_indication->data_size   = PNDV_CM_PD_MAX_REMA_BUFFER_SIZE;
    pRemaRqb->args.pd.rema_indication->data_length = 0;      // don't care at req.
    pRemaRqb->args.pd.rema_indication->index       = 0;      // don't care at req.

    PNDV_RQB_SET_HANDLE(pRemaRqb, pndv_data.cm_handle[PNDV_INDEX_PATH_CMPDSV_ACP]);
    PNDV_RQB_SET_OPCODE(pRemaRqb, CM_OPC_PD_REMA_INDICATION);

    PNDV_REQUEST(pRemaRqb, LSA_COMP_ID_CM);
}

/**
 *  @brief Handle CM - Rema Indication
 *
 *  @param[in]  rqb_ptr pointer to request block
 *
 *  Handler for rema indication (CM_OPC_PD_REMA_INDICATION).
 *  - For PDEV-data change
 *  - For Name-Of-Station change
 *
 */
PNIO_VOID pndv_in_cm_pd_rema_indication_handle(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    CM_UPPER_PD_REMA_INDICATION_PTR_TYPE pRemaInd = rqb_ptr->args.pd.rema_indication;

    PNIO_UINT16 resp = PNDV_RQB_GET_RESPONSE(rqb_ptr);
    if (resp != LSA_RSP_OK)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, resp);
    }

    if (rqb_ptr != &pndv_data.rqb.rema_ind_rqb)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0); // unexpected rqb
    }

    switch (pRemaInd->index)
    {
        case CM_PD_RECORD_INDEX_MultiplePDev:
        {
            if (pRemaInd->data_length)
            {
                PNDV_CFG_REMA_WRITE_PDEV_DATA(pRemaInd->data_ptr, pRemaInd->data_length);
            }
            else
            {
                PNDV_CFG_REMA_DELETE_PDEV_DATA();
            }
            break;
        }
        case CM_PD_RECORD_INDEX_NameOfStationRecord:
        {
            if (pRemaInd->data_length)
            {
                PNDV_CFG_REMA_WRITE_NOS_DATA(pRemaInd->data_ptr, pRemaInd->data_length);
            }
            else
            {
                PNDV_CFG_REMA_DELETE_NOS_DATA();
            }
            break;
        }
        default:
        {
            break;
        }
    }

    pndv_data.rqb.rema_rqb_in_use = PNIO_FALSE;
    pndv_in_cm_pd_rema_indication_provide_resource(); // reprovide rema indication rqb resource
}

/**
 *  @brief Provide DCP Indication Resource
 *
 * This function provides an rqb resource to CM that is returned by CM when a DCP - Set indication 
 * comes for RTF, IP-Set, NoS-Set with opc CM_OPC_PD_DCP_QUERY_INDICATION
 *
 */
PNIO_VOID pndv_in_cm_pd_dcp_indication_provide_resource()
{
    CM_UPPER_RQB_PTR_TYPE pDcpRqb = &pndv_data.rqb.dcp_ind_rqb;

    if (pndv_data.rqb.dcp_rqb_in_use)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }
    pndv_data.rqb.dcp_rqb_in_use = PNIO_TRUE;

    PNDV_RQB_SET_HANDLE(pDcpRqb, pndv_data.cm_handle[PNDV_INDEX_PATH_CMPDSV_ACP]);
    PNDV_RQB_SET_OPCODE(pDcpRqb, CM_OPC_PD_DCP_QUERY_INDICATION);

    PNDV_REQUEST(pDcpRqb, LSA_COMP_ID_CM);
}


/**
 *  @brief Handle CM - DCP Indication
 *
 *  @param[in]  rqb_ptr pointer to request block
 *
 *  Handler for dcp set indication (CM_OPC_PD_DCP_QUERY_INDICATION).
 *  - For RTF and IPSuite set, notifies IP2PN before allowing DCP set
 *  - For Name-Of-Station set, directly accepts DCP-set
 *
 */
PNIO_VOID pndv_in_cm_pd_dcp_indication_handle(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    CM_UPPER_PD_DCP_QUERY_INDICATION_PTR_TYPE pDcpInd = rqb_ptr->args.pd.dcp_query_indication;
    LSA_UINT32 dcpOptions = pDcpInd->Options;

    PNIO_UINT16 resp = PNDV_RQB_GET_RESPONSE(rqb_ptr);
    if (resp != LSA_RSP_OK)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, resp);
    }

    if (rqb_ptr != &pndv_data.rqb.dcp_ind_rqb)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0); // unexpected rqb
    }

    if ((dcpOptions & CM_DCPCTRL_RESET_TO_FACTORY) ||           // Reset to Factory
        (dcpOptions & CM_DCPCTRL_RESET_COMMUNICATION))
    {
        // If RTF is in the options, no other option is set at the same time
        pndv_in_ip2pn_reset_to_factory();
    }
    else if (dcpOptions & CM_DCPCTRL_IP_SUITE)                  // IPSuite Set
    {
        // IPSuite and Name-Of-Station can be set simultaneously
        PNDV_IP2PN_IPSUITE_INFO_T ipsuite;
        PNIO_UINT8                mk_remanent;
        ipsuite.ip_address = pDcpInd->IpSuite_IpAddr;
        ipsuite.netmask    = pDcpInd->IpSuite_SubnetMask;
        ipsuite.gateway    = pDcpInd->IpSuite_Gateway;
        mk_remanent        = ((pDcpInd->RemaOptions & CM_DCPCTRL_IP_SUITE) ? IP2PN_IPSUITE_MK_REMANENT_YES : IP2PN_IPSUITE_MK_REMANENT_NO);

        pndv_in_ip2pn_set_ipsuite(&ipsuite, mk_remanent, PNDV_IP2PN_REQ_OWNER_DCP_TYPE);
    }
    else if ((dcpOptions & CM_DCPCTRL_NAMEOFSTATION) ||         // Name-Of-Station Set         or
             (dcpOptions & CM_DCPCTRL_SIGNAL))                  // DCP Signal (Led Blink Req etc.)
    {
        // If only Name-Of-Station is set or DCP Signal is sent, then accept directly.
        pndv_in_cm_pd_dcp_indication_response(PNIO_TRUE);
    }
    else                                                        // Other dcp indications 
    {
        // Reject DCP set otherwise
        pndv_in_cm_pd_dcp_indication_response(PNIO_FALSE);
    }
}

/**
 *  @brief Send result of DCP indication (accepted/rejected)
 *
 *  @param[in]  isAccepted bool for dcp set is accepted or not 
 *
 *  DCP set is done according to CM. Now handles required application changes
 *  according to what is set
 *
 */
PNIO_VOID pndv_in_cm_pd_dcp_indication_response(PNIO_BOOL isAccepted)
{
    CM_UPPER_RQB_PTR_TYPE dcp_ptr = &pndv_data.rqb.dcp_ind_rqb;

    if (isAccepted)
    {
        dcp_ptr->args.pd.dcp_query_indication->Result = DCP_RESULT_OK;          // accept all DCP set requests
    }
    else
    {
        dcp_ptr->args.pd.dcp_query_indication->Result = DCP_RESULT_DS_NOT_SET;  // reject DCP set request
    }

    PNDV_RQB_SET_HANDLE(dcp_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_CMPDSV_ACP]);
    PNDV_RQB_SET_OPCODE(dcp_ptr, CM_OPC_PD_DCP_QUERY_RESULT);

    PNDV_REQUEST(dcp_ptr, LSA_COMP_ID_CM);
}

/**
 *  @brief Done function of DCP indication response send
 *
 *  @param[in]  rqb_ptr pointer to request block
 *
 *  Notifies stack about the acceptance status of the dcp set indication.
 *
 */
PNIO_VOID pndv_in_cm_pd_dcp_indication_response_done(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    CM_UPPER_PD_DCP_QUERY_INDICATION_PTR_TYPE pDcpInd = rqb_ptr->args.pd.dcp_query_indication;
    LSA_UINT32 dcpOptions = pDcpInd->Options;

    if (dcpOptions & CM_DCPCTRL_RESET_TO_FACTORY)         // Reset to Factory
    {
        PNDV_RESET_TO_FACTORY_IND();
    }
    else if (dcpOptions & CM_DCPCTRL_RESET_COMMUNICATION) // Reset Communication
    {
        PNDV_RESET_COMMUNICATION_IND();
    }
    else if (dcpOptions & CM_DCPCTRL_IP_SUITE)            // Set IpSuite
    {
        if ((pDcpInd->RemaOptions & CM_DCPCTRL_IP_SUITE) == 0)
        {
            PNDV_CFG_REPORT_NEW_IP_DATA(pDcpInd->IpSuite_IpAddr, pDcpInd->IpSuite_SubnetMask, pDcpInd->IpSuite_Gateway)
        }
    }

    pndv_data.rqb.dcp_rqb_in_use = PNIO_FALSE;
    pndv_in_cm_pd_dcp_indication_provide_resource(); // reprovide dcp indication rqb resource
}

/**
 *  @brief send device control request to cm
 *
 *  @param[in]  device_control Control command to be send (CM_SV_DEVICE_CONTROL_CMD_ACTIVATE,CM_SV_DEVICE_CONTROL_CMD_PASSIVATE)
 *
 *  Sends the requested device control (CM_OPC_SV_DEVICE_CONTROL)
 *  command (activate or passivate) to cm.
 *  Only activation is allowed at the moment.
 *
 */
PNIO_VOID pndv_in_device_control(PNIO_UINT8 device_control)
{
    CM_UPPER_RQB_PTR_TYPE tmp_rqb_ptr;

    if (0 == pndv_data.diag.cm_dev_req_running)
    {
        switch (device_control)
        {
            case CM_SV_DEVICE_CONTROL_CMD_ACTIVATE:
            {
                if (((PNDV_SM_W_ADD_IM_SUBMODULES_DONE <= pndv_data.sm) && 
                     (PNDV_SM_DEVICE_ACTIVE            >= pndv_data.sm)))
                {
                    pndv_data.rqb.dev_control.args.sv.dev_control->device_nr = PNDV_CM_DEVICE_NO;
                    if (0 != PNDV_RQB_GET_OPCODE(&pndv_data.rqb.dev_control))
                    {
                        pndv_in_fatal_error(PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE(&pndv_data.rqb.dev_control));
                    }
                    pndv_data.rqb.dev_control.args.sv.dev_control->cmd = device_control;
                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_DV_AKT, device_control);

                    PNDV_RQB_SET_HANDLE(&pndv_data.rqb.dev_control, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                    PNDV_RQB_SET_OPCODE(&pndv_data.rqb.dev_control, CM_OPC_SV_DEVICE_CONTROL);

                    tmp_rqb_ptr = &pndv_data.rqb.dev_control;
                    pndv_data.diag.cm_dev_req_running = PNDV_CM_DEV_REQ_DEVICE_CONTROL;

                    PNDV_REQUEST(tmp_rqb_ptr, LSA_COMP_ID_CM);
                }
                else
                {
                    pndv_in_fatal_error(PNDV_MODULE, __LINE__, pndv_data.sm);
                }
                break;
            }
            case CM_SV_DEVICE_CONTROL_CMD_PASSIVATE:
            {
                pndv_data.rqb.dev_control.args.sv.dev_control->device_nr = PNDV_CM_DEVICE_NO;
                if (0 != PNDV_RQB_GET_OPCODE(&pndv_data.rqb.dev_control))
                {
                    pndv_in_fatal_error(PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE(&pndv_data.rqb.dev_control));
                }

                pndv_data.rqb.dev_control.args.sv.dev_control->cmd = device_control;

                pndv_in_write_debug_buffer_all__(PNDV_DC_CM_DV_AKT, device_control);

                PNDV_RQB_SET_HANDLE(&pndv_data.rqb.dev_control, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                PNDV_RQB_SET_OPCODE(&pndv_data.rqb.dev_control, CM_OPC_SV_DEVICE_CONTROL);

                tmp_rqb_ptr = &pndv_data.rqb.dev_control;

                pndv_data.diag.cm_dev_req_running = PNDV_CM_DEV_REQ_DEVICE_CONTROL;

                PNDV_REQUEST(tmp_rqb_ptr, LSA_COMP_ID_CM);

                break;
            }
            default:
            {
                pndv_in_fatal_error(PNDV_MODULE, __LINE__, pndv_data.sm);
                break;
            }
        }
    }
    else
    {
        pndv_data.diag.cm_dev_req.pending |= PNDV_CM_DEV_REQ_DEVICE_CONTROL;
    }
}

/**
 *  @brief handles the device control response
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  Handles the response to the CM_OPC_SV_DEVICE_CONTROL request.
 *
 */
PNIO_VOID pndv_in_cm_sv_device_control_done (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNDV_IFACE_CMD_ENTRY_T tmp_event;

    PNIO_UINT32 response;

    if (pndv_data.device_control == CM_SV_DEVICE_CONTROL_CMD_ACTIVATE)
    {
        response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

        if (CM_OK != response)
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
        }

        pndv_data.diag.cm_dev_req_running = 0;

        /* pp-req prevented due to running dev_req (pndv_data.diag.cm_dev_req_running). */

        // indicate device aktivation to peri
        tmp_event.cmd = PNDV_EV_TO_PERI_CM_DV_AKT_IND;
        pndv_in_peri_write_coupl_event(tmp_event);

        /* cm_dev_req_running ist 0 -> evtl. andere dev req wdh. */
        pndv_in_cm_check_dev_req();
    }
    else
    {
        response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

        if (CM_OK != response)
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
        }

        pndv_data.diag.cm_dev_req_running = 0;

        // indicate device aktivation to peri
        tmp_event.cmd = PNDV_EV_TO_PERI_CM_DV_DEAKT_IND;
        pndv_in_peri_write_coupl_event(tmp_event);
    }
}



/**
 *  @brief handler for ar control
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  Handles the response to CM_OPC_SV_AR_CONTROL.
 *  The request is not used at the moment.
 *  It is obsolet as the EDD-DataStatus-Api is used
 *  to control the data state.
 *
 */
PNIO_VOID pndv_in_cm_sv_ar_control_done (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    /* my change of APDU state done */

    PNIO_UINT32 response;
    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    if ((CM_OK           != response) &&
        (CM_ERR_SESSION  != response))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }
}

/**
 *  @brief Handler for apdu status indications
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *
 *  Handles incomming  ar data status indications (CM_OPC_SV_AR_DATA_STATUS_IND).
 *  Indication is responded without evaluation. APDU state changes are detected in
 *  another way.
 *
 */
PNIO_VOID pndv_in_cm_sv_event_apdu_status_ind  (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    /* APDU state was changed from external */

    PNIO_UINT32  response;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    if (CM_OK != response)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }

    /* apdu state will be checked with synchroneousely incoming output data */
    /* here at this point the apdu state is ignored */

    PNDV_RQB_SET_OPCODE(rqb_ptr, CM_OPC_SV_AR_DATA_STATUS_RSP);
}

#ifdef PNDV_CFG_USE_DEVICE_READ
PNIO_VOID pndv_cm_in_device_read_req(PNDV_RQB_PTR_TYPE rqb_ptr)
{
    CM_UPPER_RQB_PTR_TYPE local_rqb_ptr;

    if (PNDV_RQB_GET_OPCODE(rqb_ptr) == PNDV_OPC_DEVICE_READ)
    {

        if (pndv_data.usr.dev_read.service_running != PNIO_TRUE)
        {

            pndv_data.usr.dev_read.service_running = PNIO_TRUE;
            pndv_data.usr.dev_read.usr_rqb         = rqb_ptr;

            PNDV_ALLOC_MEM(&local_rqb_ptr, sizeof(CM_RQB_TYPE));
            if (local_rqb_ptr == 0)
            {
                /* no memory */
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
            }
            PNDV_ALLOC_MEM(&local_rqb_ptr->args.sv.dev_read, sizeof(CM_SV_DEVICE_READ_TYPE))
            if (local_rqb_ptr->args.sv.dev_read == 0)
            {
                /* no memory */
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
            }

            local_rqb_ptr->args.sv.dev_read->device_nr          = rqb_ptr->args->dev_read.device_nr; /* device number */

            local_rqb_ptr->args.sv.dev_read->data_length        = rqb_ptr->args->dev_read.data_length ; /* req: size of 'data' (including CM_RECORD_OFFSET), cnf: CM_RECORD_OFFSET +number of bytes read */
            local_rqb_ptr->args.sv.dev_read->data               = rqb_ptr->args->dev_read.data;

            local_rqb_ptr->args.sv.dev_read->api                = rqb_ptr->args->dev_read.api;
            local_rqb_ptr->args.sv.dev_read->slot_nr            = rqb_ptr->args->dev_read.slot_nr;
            local_rqb_ptr->args.sv.dev_read->subslot_nr         = rqb_ptr->args->dev_read.subslot_nr;
            local_rqb_ptr->args.sv.dev_read->record_index       = rqb_ptr->args->dev_read.record_index;  /* see PNIO-Spec */
            local_rqb_ptr->args.sv.dev_read->target_ar_uuid     = rqb_ptr->args->dev_read.target_ar_uuid;

            local_rqb_ptr->args.sv.dev_read->pnio_status        = 0; /* cnf: pnio error status, see PNIO-Spec */

            PNDV_RQB_SET_HANDLE(local_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
            PNDV_RQB_SET_OPCODE(local_rqb_ptr, CM_OPC_SV_DEVICE_READ);

            PNDV_REQUEST(local_rqb_ptr, LSA_COMP_ID_CM);
        }
        else
        {
            /* still another request running */
            PNDV_RQB_SET_RESPONSE(rqb_ptr, PNDV_ERR_SEQUENCE);

            PNDV_CFG_USR_REQ_DONE(rqb_ptr);
        }
    }
    else
    {
        /* wrong opcode */
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE(rqb_ptr));
    }

}
#endif

PNIO_VOID pndv_in_cm_to_appl_cbf(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNDV_RQB_APPL_REQUEST(rqb_ptr, pndv_cm_callback);
}


/*****************************************************************************/
/*  CM Helper Functions                                                   */
/*****************************************************************************/

PNIO_UINT16 get_nos_data_from_rema_record(PNIO_UINT8* pRecData, LSA_UINT32 RecordLen,
                                          PNIO_UINT8** nos_data_ptr_ptr, PNIO_UINT16* nos_data_len_ptr)
{
    PNIO_UINT32 offset = 0;
    PNIO_UINT16 nos_data_len = 0;

    if (RecordLen < PNDV_RD_MIN_NOS_REC_LEN || pRecData == LSA_NULL)
    {
        return PNIO_NOT_OK;
    }

    // skip paddings
    offset += PNDV_RD_BLK_HEADER_SIZE + 2/*Reserved*/;
    PNDV_COPY_BYTE(&nos_data_len, pRecData + offset, sizeof(PNIO_UINT16));
    nos_data_len = PNDV_NTOHS(nos_data_len);

    if (RecordLen < PNDV_RD_MIN_NOS_REC_LEN + nos_data_len)
    {
        return PNIO_NOT_OK;
    }

    offset += sizeof(nos_data_len) + 2/*Reserved*/;
    *nos_data_ptr_ptr = pRecData + offset;
    *nos_data_len_ptr = nos_data_len;

    return PNIO_OK;
}


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
