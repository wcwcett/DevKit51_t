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
/*  F i l e               &F: pndv.c                                    :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  General functions for the Profinet device (cm-user)                      */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/* contents:

--- SYSTEM INTERFACE FUNCTIONS ----------------

        - pndv_init
        - pndv_open
        - pndv_start
        - pndv_stop
        - pndv_perform_services

        - pndv_version
        - pndv_debug_extern
        - pndv_debug_get_info
        - pndv_check_session
        - pndv_check_indication_trigger_timeout
        - pndv_so_locked_delay_trigger_timeout

--- INTERNAL FUNCTIONS ------------------------

        - pndv_in_so_locked_delay_timeout
        - pndv_in_set_so_state
        - pndv_in_get_ar_idx

        - pndv_in_debug_control
        - pndv_in_write_debug_buffer
        - pndv_in_get_last_debug_buffer
        - pndv_in_fatal_error

*/
/*****************************************************************************/
/* include hierarchy */

#include "pndv_inc.h"

#define PNDV_MODULE PNDV_ERR_MODULE
#define LTRC_ACT_MODUL_ID PNDV_MODULE

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**********                                                         **********/
/**********               SYSTEM INTERFACE FUNCTIONS                **********/
/**********                                                         **********/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

volatile GLOB_COUPLING_INTERFACE  glob_coupling_interface;

/*****************************************************************************/


/**
 *  @brief Initialization of pndv data
 *
 *  @param[in] iface_ptr  Pointer to peri interface
 *
 *  This function initializes internal data structures of the pndv.
 *  The function must be called only once befor any other pndv function calls.
 *
 *  @note Pointer to interface needs to be known here, because it must be initialized too.
 *        Initializing it later would lead to second sunchronization point at device start
 *        on distributed hardware solutions.
 *
 */
PNIO_VOID pndv_init(PNDV_IFACE_STRUCT_PTR iface_ptr)
{
    PNIO_UINT32 async_cnt;
    PNIO_UINT32 port;


    if (!iface_ptr)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    pndv_data.reset_reason          = PNDV_GET_RESET_REASON_CODE();

    if (PNDV_RESET_REASON_CODE_RESTART_AFTER_FATAL != pndv_data.reset_reason)
    {
        PNDV_MEMSET(&pndv_debug_buffer,0,sizeof(pndv_debug_buffer));
#ifdef PNDV_CFG_DEBUG_ENABLE
        pndv_in_debug_control( PNDV_DEBUG_ON_ALL );
    }
    else
    {
        pndv_in_debug_control( PNDV_DEBUG_OFF );
#endif
    }

    pndv_data.iface_ptr             = iface_ptr;
    pndv_data.sm                    = PNDV_SM_CLOSED;
    pndv_data.stop_req              = PNIO_FALSE;
    pndv_data.block_ar_while_rtf    = PNIO_FALSE;

    LSA_USER_ID_INIT(&pndv_data.null_user_id);  /* pndv_data.null_user_id = 0xff (LSA_USER_ID_UINT8_UNUSED) */

    pndv_data.cfg.first_real_cfg_modul_index    = 0;
    pndv_data.cfg.slot_no_running_cm_modul_req  = 0xffff;

    for (async_cnt = 0; async_cnt <= PNDV_CM_AR_NO; async_cnt++)   /* <= so that too much AR can be terminated cleanly */
    {
        pndv_data.ar[async_cnt].sm_state = PNDV_AR_SM_OFFLINE;

        pndv_data.ar[async_cnt].no_prm_end_ind     = PNIO_FALSE;
        pndv_data.ar[async_cnt].ar_abort_requested = PNIO_FALSE;
        pndv_data.ar[async_cnt].empty_prm_end_ind  = PNIO_FALSE;
        pndv_data.ar[async_cnt].pndv_ar_type       = PNDV_AR_TYPE_UNKNOWN;
        pndv_data.ar[async_cnt].pdev_8000_locked   = PNIO_FALSE;
        pndv_data.ar[async_cnt].pdev_9000_locked   = PNIO_FALSE;


        /* RQB fuer App-ready */
        /* --------------------------------------------------------------------- */

        pndv_data.rqb.app_ready[async_cnt].args.sv.ar_appl_ready = (CM_UPPER_SV_AR_APPL_READY_PTR_TYPE) &pndv_data.rqb.app_ready_args[async_cnt];

        /* RQB als frei markieren */

        PNDV_RQB_SET_OPCODE( &pndv_data.rqb.app_ready[async_cnt], 0);


        /* RQB fuer AR-Abort */
        /* --------------------------------------------------------------------- */

        pndv_data.rqb.ar_abort[async_cnt].args.sv.ar_abort = (CM_UPPER_SV_AR_ABORT_PTR_TYPE) &pndv_data.rqb.ar_abort_args[async_cnt];

        /* RQB als frei markieren */

        PNDV_RQB_SET_OPCODE(&pndv_data.rqb.ar_abort[async_cnt], 0);


        /* RQB Pointer fuer stored RTC3.Ind */
        /* --------------------------------------------------------------------- */

        pndv_data.rqb.stored_rtc3_ind_rqb_ptr[async_cnt] = NIL;


        /* RQB Pointer fuer stored PrmEnd.Ind */
        /* --------------------------------------------------------------------- */

        pndv_data.rqb.stored_prm_end_ind_rqb_ptr[async_cnt] = NIL;

        /* RQB fuer RIR.ack */
        /* --------------------------------------------------------------------- */

        pndv_data.rqb.rir_rqb[async_cnt].args.sv.ar_rir_ack = (CM_UPPER_SV_AR_RIR_ACK_PTR_TYPE) &pndv_data.rqb.rir_ack_args[async_cnt];

        /* RQB als frei markieren */

        PNDV_RQB_SET_OPCODE(&pndv_data.rqb.rir_rqb[async_cnt], 0);

    }

    for (async_cnt = 0; async_cnt < PNDV_MAX_ARS_RT; async_cnt++)
    {
        pndv_data.ar_so_locked_state[async_cnt] = PNIO_FALSE;
    }

    for (async_cnt = 0; async_cnt < (PNDV_MAX_AR_SET + 1); async_cnt++)
    {
        pndv_data.sr[async_cnt].sm = PNDV_SR_STATE_OFFLINE;
        pndv_data.sr[async_cnt].primary_ar_idx = 0;
        pndv_data.sr[async_cnt].ar_set_nr = PNDV_SR_SET_NR_NOT_USED;
    }

    /* RQB als frei markieren */


    PNDV_MEMSET(&pndv_data.cfg.peri_cfg, 0x0, sizeof(PNDV_PERI_CFG_T));

    /* Modul ID des Kopfes wird erst in pndv_open bekannt */

    PNDV_LIST_INITIALIZE(&pndv_data.cfg.peri_cfg.free_list);
    PNDV_LIST_INITIALIZE(&pndv_data.cfg.peri_cfg.in_use_list);
    PNDV_LIST_INITIALIZE(&pndv_data.cfg.peri_cfg.cm_add_list);
    PNDV_LIST_INITIALIZE(&pndv_data.cfg.peri_cfg.cm_rem_list);
    PNDV_LIST_INITIALIZE(&pndv_data.cfg.peri_cfg.cm_add_rem_wait_list);


    // init service lists
    PNDV_LIST_INITIALIZE(&pndv_data.serv.free_list);
    PNDV_LIST_INITIALIZE(&pndv_data.serv.con_wait_list);
    PNDV_LIST_INITIALIZE(&pndv_data.serv.own_wait_list);
    PNDV_LIST_INITIALIZE(&pndv_data.serv.prm_wait_list);
    PNDV_LIST_INITIALIZE(&pndv_data.serv.ida_wait_list);
    pndv_data.serv.con_serv_run = PNIO_FALSE;
    pndv_data.serv.own_serv_run = PNIO_FALSE;
    pndv_data.serv.prm_serv_run = PNIO_FALSE;
    pndv_data.serv.ida_serv_run = PNIO_FALSE;

    //init rosal lists

    PNDV_LIST_INITIALIZE(&pndv_data.al.q_rosal.free_list);
    PNDV_LIST_INITIALIZE(&pndv_data.al.q_rosal.in_use);

#ifdef PNDV_CFG_PERI_QUEUE_DS_REQ
    // init rqb_queue
    PNDV_LIST_INITIALIZE(&pndv_data.rqb_queue.free_list);
    PNDV_LIST_INITIALIZE(&pndv_data.rqb_queue.in_use);
#endif

    //init pdev prm record list
    PNDV_LIST_INITIALIZE(&pndv_data.prm_queue.free_list);
    PNDV_LIST_INITIALIZE(&pndv_data.prm_queue.in_use);

    // put prm res to free_list
    for(async_cnt = 0; async_cnt < PNDV_PD_MAX_PRM_REC_COUNT; async_cnt++)
    {
        PNDV_LIST_INSERT_TAIL(&pndv_data.prm_queue.free_list, &pndv_data.prm_queue.elem[async_cnt].link);
    }

    // put resources to free_list
    for(async_cnt = 0; async_cnt < PNDV_MAX_SV_ENTITY; async_cnt++)
    {
        PNDV_LIST_INSERT_TAIL(&pndv_data.serv.free_list, &pndv_data.serv.elem[async_cnt].link);
#ifdef PNDV_CFG_PERI_QUEUE_DS_REQ
        PNDV_LIST_INSERT_TAIL(&pndv_data.rqb_queue.free_list, &pndv_data.rqb_queue.elem[async_cnt].link);
#endif

        pndv_in_init_local_sub_res(&pndv_data.cfg.peri_cfg.sub_res[async_cnt]);

        pndv_data.cfg.peri_cfg.sub_res[async_cnt].my_field_index = async_cnt;

        PNDV_LIST_INSERT_TAIL(&pndv_data.cfg.peri_cfg.free_list, &pndv_data.cfg.peri_cfg.sub_res[async_cnt].link);
        PNDV_LIST_INSERT_TAIL(&pndv_data.al.q_rosal.free_list, &pndv_data.al.q_rosal.elem[async_cnt].link);
    }

    PNDV_MEMSET(&pndv_data.ev_queue_ready_for_input_update, 0, sizeof(pndv_data.ev_queue_ready_for_input_update));

    PNDV_MEMSET(&pndv_data.cfg.peri_cfg.ff_index_array[0][0], 0xFF, sizeof(pndv_data.cfg.peri_cfg.ff_index_array));

    pndv_peri_init_entity_admin();

    pndv_data.cfg.peri_cfg.sm_state = PNDV_SM_PERIBUS_STATE_UNKNOWN;

    pndv_data.rqb.ium_rqb_event.u.sv.ar_rec.data  = &pndv_data.rqb.ium_rqb_event_record_data[0];
    pndv_data.rqb.ium_rqb.args.pd.event           = &pndv_data.rqb.ium_rqb_event;


    /* common data */
    pndv_data.are_annotation_and_sysident_strs_initialized = PNIO_FALSE;
        
    pndv_data.cfg.sm_connect_service      = PNDV_SM_CONNECT_SERVICE_STATE_UNKNOWN;

    pndv_data.cfg.akt_isom_ar_idx         = PNDV_AR_IDX_NOT_USED;
    pndv_data.cfg.prev_isom_ar_idx        = PNDV_AR_IDX_NOT_USED;
    pndv_data.cfg.mode_isom               = PNIO_FALSE;
    pndv_data.cfg.all_modul_running       = PNIO_TRUE;

    /* rd input, rd output data */
    pndv_data.rd_io.rd_io_req.inp_slot    = 0xFFFF;                 /* invalid -> there is no req */
    pndv_data.rd_io.rd_io_req.outp_slot   = 0xFFFF;                 /* invalid -> there is no req */
    pndv_data.rd_io.rd_io_req.state       = PNDV_RD_IO_RECORD_STATE_NONE;

    /* RQB for DCP Indication */
    /* --------------------------------------------------------------------- */
    pndv_data.rqb.dcp_rqb_in_use                                              = PNIO_FALSE;
    pndv_data.rqb.dcp_ind_rqb.args.pd.dcp_query_indication                    = (CM_UPPER_PD_DCP_QUERY_INDICATION_PTR_TYPE)&pndv_data.rqb.dcp_ind_args;
    pndv_data.rqb.dcp_ind_rqb.args.pd.dcp_query_indication->NameOfStation_ptr = LSA_NULL;
    pndv_data.rqb.dcp_ind_rqb.args.pd.dcp_query_indication->NameOfStation_len = 0;

    /* RQB for Rema Indication */
    /* --------------------------------------------------------------------- */
    pndv_data.rqb.rema_rqb_in_use                                 = PNIO_FALSE;
    pndv_data.rqb.rema_ind_rqb.args.pd.rema_indication            = (CM_UPPER_PD_REMA_INDICATION_PTR_TYPE) &pndv_data.rqb.rema_ind_args;
    pndv_data.rqb.rema_ind_rqb.args.pd.rema_indication->data_ptr  = pndv_data.rqb.rema_ind_data;

    /* RQB for DeviceControl */
    /* --------------------------------------------------------------------- */

    pndv_data.rqb.dev_control.args.sv.dev_control         = (CM_UPPER_SV_DEVICE_CONTROL_PTR_TYPE) &pndv_data.rqb.dev_control_args;

    /* Mark RQB as free */

    PNDV_RQB_SET_OPCODE(&pndv_data.rqb.dev_control, 0);


    /* RQB for Diag-add */
    /* --------------------------------------------------------------------- */

    pndv_data.rqb.diag_add.args.sv.diag_add               = (CM_UPPER_SV_DIAG_ADD_PTR_TYPE) &pndv_data.rqb.diag_add_args;

    /* Mark RQB as free */

    PNDV_RQB_SET_OPCODE(&pndv_data.rqb.diag_add, 0);

    /* RQB for Diag-remove */
    /* --------------------------------------------------------------------- */


#if (defined PNDV_CFG_USE_DIAG2) || (defined PNDV_CFG_USE_MULTIDIAG)
    pndv_data.rqb.diag_remove.args.sv.diag_add             = (CM_UPPER_SV_DIAG_ADD_PTR_TYPE) &pndv_data.rqb.diag_remove_keys;
#else
    pndv_data.rqb.diag_remove.args.sv.diag_remove          = (CM_UPPER_SV_DIAG_REMOVE_PTR_TYPE) &pndv_data.rqb.diag_remove_args;
#endif


    /* Mark RQB as free */

    PNDV_RQB_SET_OPCODE(&pndv_data.rqb.diag_remove, 0);

    /* RQB for SF-LED info */
    /* --------------------------------------------------------------------- */

    pndv_data.rqb.device_led_info.args.sv.dev_led_info            = (CM_UPPER_SV_DEVICE_LED_INFO_PTR_TYPE) &pndv_data.rqb.device_led_info_args;

    /* Mark RQB as free */

    PNDV_RQB_SET_OPCODE(&pndv_data.rqb.device_led_info, 0);

    /* RQB for Sync-LED Info */
    /* --------------------------------------------------------------------- */

    pndv_data.rqb.sync_led_info.args.pd.sync_led_info            = (CM_UPPER_PD_SYNC_LED_INFO_PTR_TYPE) &pndv_data.rqb.sync_led_info_args;

    /* Mark RQB as free */

    PNDV_RQB_SET_OPCODE(&pndv_data.rqb.sync_led_info, 0);

    /* RQB for submodule add / remove */
    /* --------------------------------------------------------------------- */
    /* mark RQB as free */
    PNDV_RQB_SET_OPCODE(&pndv_data.rqb.sub_add_rem, 0);

    /* Debug */
    /* --------------------------------------------------------------------- */

#ifdef PNDV_CFG_DEBUG_COUNTER
    pndv_data.trigger.read_record_ind  =
    pndv_data.trigger.read_record_rsp  =
    pndv_data.trigger.write_record_ind =
    pndv_data.trigger.write_record_rsp =

    pndv_data.trigger.al_send          =
    pndv_data.trigger.al_send_done     =
    pndv_data.trigger.al_ack           = 0;
#endif

#ifdef PNDV_CFG_DEBUG_ENABLE
    PNDV_TRC_TP_INIT(pndv_debug_buffer, PNDV_CFG_DEBUG_ELEMENT_NUMBER, TRC_ID_PNDV);
#endif

    /* Dual-Port Ram */
    /* --------------------------------------------------------------------- */

    pndv_in_peri_init( );

    pndv_ar_init_set_cfg();

    /* Alarms */
    /* --------------------------------------------------------------------- */

    pndv_in_al_init();

    pndv_data.ar_in_data_led_used = PNIO_FALSE;

    pndv_data.akt_clear_state = PNIO_TRUE;

    pndv_data.alloc_mem_counter = 0;
    pndv_data.alloc_rqb_counter = 0;

    pndv_data.cmpd_prm_default  = PNIO_FALSE;

#ifdef PNDV_CFG_ENABLE_RECORD_RESPONDER
    pndv_in_ds_rd_input_data_init();
    pndv_in_ds_rd_output_data_init();
#endif


    /* PortLinkStatus */
    /* --------------------------------------------------------------------- */

    for (port = 0; port < PNDV_MAX_PORT; port++)
    {
        pndv_data.port_link_status[port] = CM_LINK_STATUS_DOWN;
    }

    /* check indication timeout handling */

    pndv_data.connect_indication_timeout_timer_id = 0;
}

/**
 * @brief initialize local_sub_res data
 *
 * @param[in]   local_sub_res       pointer to local submodule ressource whose data is to be reset.
 */
PNIO_VOID pndv_in_init_local_sub_res(PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res)
{
    local_sub_res->para_error                 = 0;
    local_sub_res->own_ar_idx                 = PNDV_AR_IDX_NOT_USED;
    local_sub_res->ar_set_nr                  = PNDV_SR_SET_NR_NOT_USED;

    local_sub_res->entity                     = 0xFFFF;
    local_sub_res->plug_cm                    = PNDV_SUBMODUL_PLUG_STATE_NIX;
    local_sub_res->res_state                  = PNDV_CFG_SUBMODULE_RES_STATE_FREE;
    local_sub_res->submodule_state            = PNDV_SUBMODULE_STATE_NO_MODULE;
}



/**
 *  @brief System open of the pndv
 *
 *  @param[in] parameter_ptr  Pointer to pndv parameter structure
 *
 *  This function must be called after initializing the pndv (pndv_init()).
 *  A pointer to a pndv parameter structure must be provided (see ::PNDV_SYS_PARAMETER_PTR).
 *  The function must be called only once before starting the pndv (pndv_start())
 *
 */
PNIO_VOID pndv_open(PNDV_SYS_PARAMETER_PTR parameter_ptr)
{
    PNIO_UINT8 ret_val;
    PNIO_UINT16 tmp_ret_val;

    ret_val = PNDV_OK;
    tmp_ret_val = LSA_RET_OK;

    pndv_in_write_debug_buffer_all__(PNDV_DC_ALL_OPEN, 0);

    if (PNDV_SM_CLOSED == pndv_data.sm)
    {
        PNIO_UINT32 tmp_count;
        PNDV_PRM_REQUEST_ELEMENT_PTR_T local_prm_res;

        /* store parameter ptr for later usage */
        pndv_data.oem_data_ptr   = parameter_ptr->cm.oem_data_ptr;

        pndv_data.rema_station_prm_ptr = (PNDV_RQB_DS1_PTR)parameter_ptr->rema_station.prm_ptr;

        pndv_data.rd_in_out.ds_struc_ptr = (PNDV_RQB_DS_PTR)parameter_ptr->sys.rqb_ds_res_ptr[PNDV_DS_RES_IDX_RD_IO];

        PNDV_MEMSET(&parameter_ptr->sys.rqb_ds_res_ptr[PNDV_DS_RES_IDX_RD_IO]->record_data[0], 0, sizeof(parameter_ptr->sys.rqb_ds_res_ptr[PNDV_DS_RES_IDX_RD_IO]->record_data));

        parameter_ptr->sys.rqb_ds_res_ptr[PNDV_DS_RES_IDX_RD_IO]->event_type.u.sv.ar_rec.data = (CM_COMMON_MEM_U8_PTR_TYPE)& parameter_ptr->sys.rqb_ds_res_ptr[PNDV_DS_RES_IDX_RD_IO]->record_data[0];
        parameter_ptr->sys.rqb_ds_res_ptr[PNDV_DS_RES_IDX_RD_IO]->rqb.args.pd.event           = & parameter_ptr->sys.rqb_ds_res_ptr[PNDV_DS_RES_IDX_RD_IO]->event_type;

        /* alloc a timer for timeout handling of the check indication */
        PNDV_ALLOC_TIMER(&tmp_ret_val, &pndv_data.connect_indication_timeout_timer_id, LSA_TIMER_TYPE_ONE_SHOT,
                         LSA_TIME_BASE_100MS);

        if (LSA_RET_OK != tmp_ret_val)
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, (PNIO_UINT32)tmp_ret_val);
        }

        /* Hardware environment */
        /* ------------------------------------------------------------- */

        for(PNDV_LIST_EACH(local_prm_res, &pndv_data.prm_queue.free_list, PNDV_PRM_REQUEST_ELEMENT_PTR_T))
        {
            CM_UPPER_RQB_PTR_TYPE rqb_ptr;

            PNDV_ALLOC_RQB( ((LSA_VOID_PTR_TYPE)&rqb_ptr), sizeof(CM_RQB_TYPE));

            if ( pndv_host_ptr_is_nil__(rqb_ptr) )
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
            }

            PNDV_ALLOC_MEM( ((LSA_VOID_PTR_TYPE)&rqb_ptr->args.pd.prm_rdwr), sizeof(CM_PD_PRM_RDWR_TYPE));

            if ( pndv_host_ptr_is_nil__(rqb_ptr->args.pd.prm_rdwr) )
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
            }
            local_prm_res->rqb_ptr = rqb_ptr;
        }

        /* set system paths for cm channels */
        for (tmp_count = 0; PNDV_MAX_CM_CHANNEL > tmp_count; tmp_count++)
        {
            pndv_data.sys_path_cm[tmp_count] = parameter_ptr->cm.sys_path_cm[tmp_count];
        }
        /* set system paths for ip2pn channels */
        for (tmp_count = 0; PNDV_MAX_IP2PN_CHANNEL > tmp_count; tmp_count++)
        {
            pndv_data.sys_path_ip2pn[tmp_count] = parameter_ptr->ip2pn.sys_path_ip2pn[tmp_count];
        }

        /* get device id */
        pndv_data.cfg.device_id = parameter_ptr->cm.device_id;


        /* pdev data */

        if (parameter_ptr->pd.port_count_used > PNDV_CFG_MAX_PORT_CNT)
        {
            ret_val = LSA_RET_ERR_PARAM;
        }
        pndv_data.cfg.pd = parameter_ptr->pd;

        /* open done (ok) */
        /* ------------------------------------------------------------- */

        if (ret_val == PNDV_OK)
        {
            pndv_sm(PNDV_SM_EVENT_OPEN);
        }

        PNDV_OPEN_DONE( ret_val );

    }
    else
    {
        PNDV_OPEN_DONE( PNDV_ERR_SEQUENCE);
    }
}



/** @brief PN device state machine
 *
 *  @param[in] event Event that is been send to the state machine
 *
 *  This is the global statemachine that is used for starting up the pndv
 *  component.
 *  It manages the startup and shutdown process of the profinet device.
 *
 *  The state of the pn state machine is evaluated and the given event
 *  is handled in the appropriate case.
 *
 *  To get a graphical overview of the state flow you may refer to
 *  <A HREF="file:\\R:\Firmware\pndv.cmp\doc\PNDV_V2\pndv_sm.pdf">pndv_sm.pdf (absolut local)</A>.<br>
 *  If the link is not reachable you can find a copy in the components doc
 *  directory.
 *
 *  For a detailed description of the states please refer to @ref PNDV_SM_STATE_E
 *
 *  For a detailed description of the events please refer to @ref PNDV_SM_EVENT_E
 *
 *  @note As there can only be one instance of the pn device at the moment, the related object
 *        is hardcoded into this state machine method.
 *
 *
 */
PNIO_VOID pndv_sm(PNDV_SM_EVENT_E_T event)
{
    pndv_in_write_debug_buffer_2__(PNDV_DC_PNDV_SM, event, pndv_data.sm);

    switch (pndv_data.sm)
    {
        case PNDV_SM_CLOSED:                                // Initial state of pndv
        {
            switch (event)
            {
                case PNDV_SM_EVENT_OPEN:
                {
                    /* pndv is open now */
                    pndv_data.sm = PNDV_SM_OPEN;
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error(PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_SM_OPEN:                                  // Open state of pndv
        {
            switch (event)
            {
                case PNDV_SM_EVENT_IP2PN_OPEN:
                {
                    /* opens pndv-ip2pn channels and initial services */
                    pndv_data.sm = PNDV_SM_W_OPEN_IP2PN_DONE;
                    pndv_in_ip2pn_open();
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error(PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_SM_W_OPEN_IP2PN_DONE:                     // Pndv-ip2pn open is done
        {
            switch (event)
            {
                case PNDV_SM_EVENT_IP2PN_OPEN_DONE:
                {
                    /* opens pndv-cm pd server channel */
                    pndv_data.sm = PNDV_SM_W_OPEN_CMPDSV_ACP_CHANNEL_DONE;
                    pndv_in_cm_open_channel(PNDV_INDEX_PATH_CMPDSV_ACP);
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error(PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_SM_W_OPEN_CMPDSV_ACP_CHANNEL_DONE:        // Pndv-cm pd server channel open is done
        {
            switch (event)
            {
                case PNDV_SM_EVENT_CMPDSV_ACP_CHANNEL_OPEN_DONE:
                {
                    /* setups and creates cm pd server */
                    pndv_data.sm = PNDV_SM_W_CREATE_PD_SERVER_CNF;
                    pndv_in_cm_pdsv_create();
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error(PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        } 
        case PNDV_SM_W_CREATE_PD_SERVER_CNF:                // PD server is created
        {
            switch (event)
            {
                case PNDV_SM_EVENT_CREATE_PD_SERVER_DONE:
                {
                    /* opens pndv-cm iod cm server channel */
                    pndv_data.sm = PNDV_SM_W_OPEN_IOD_CMSV_ACP_CHANNEL_DONE;
                    pndv_in_cm_open_channel(PNDV_INDEX_PATH_IOD_CM_ACP);
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_SM_W_OPEN_IOD_CMSV_ACP_CHANNEL_DONE:      // Pndv-cm iod cm server channel open is done
        {
            switch (event)
            {
                case PNDV_SM_EVENT_OPEN_IOD_CMSV_ACP_CHANNEL_DONE:
                {
                    /* sets initial ipsuite over ip2pn */
                    pndv_data.sm = PNDV_SM_W_IP2PN_SET_IPSUITE_DONE;
                    pndv_in_ip2pn_set_initial_ipsuite();
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error(PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_SM_W_IP2PN_SET_IPSUITE_DONE:              // Set initial ipsuite request is done
        {
            switch (event)
            {
                case PNDV_SM_EVENT_IP2PN_SET_IPSUITE_DONE:
                {
                    /* creates the CM server*/
                    pndv_data.sm = PNDV_SM_W_CREAT_SERVER_CNF;
                    pndv_in_cm_sv_create();
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error(PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_SM_W_CREAT_SERVER_CNF:                    // Create cm server is done
        {
            switch (event)
            {
                case PNDV_SM_EVENT_CREATE_CM_SERVER_DONE:
                {
                    /* add device to cm server*/
                    pndv_data.sm = PNDV_SM_W_ADD_DEVICE_CNF;
                    pndv_in_cm_sv_device_add();
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error(PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_SM_W_ADD_DEVICE_CNF:                      // Device is added to cm server
        {
            switch (event)
            {
                case PNDV_SM_EVENT_DEVICE_ADD_DONE:
                {
                    pndv_data.sm = PNDV_SM_W_ADD_IM_SUBMODULES_DONE;
                    /* Set RQB with current LED state to CM - acknowledgment only comes with a change*/
                    pndv_in_cm_provide_led_info_rqb();

                    // starting PNDV is done on reaching here
                    PNDV_IFACE_CMD_ENTRY_T tmp_event;
                    tmp_event.cmd = PNDV_EV_TO_PERI_PNDV_START_DONE;
                    pndv_in_peri_write_coupl_event(tmp_event);
                    break;
                }
                case PNDV_SM_EVENT_STOP:
                {
                    /* stop event received while cm request is on the way */
                    pndv_data.sm = PNDV_SM_STOP_W_CM_CNF;
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error(PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_SM_W_ADD_IM_SUBMODULES_DONE:
        {
            switch (event)
            {
                case PNDV_SM_EVENT_ADD_IM_SUBMODULES_DONE:
                {
                    /* all im submodules are plugged now, now pdev prm can be done */
                    pndv_data.sm = PNDV_SM_PRM_PDEV;
                    pndv_in_cm_pd_prm_begin(); // initiates a chain of _PD_PRM_PREPARE, _PD_PRM_WRITE, and finally PD_PRM_END
                                               // _PD_PRM_END leads to a _PD_EVENT_APPL_READY_IND (next event for the sm)
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error(PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_SM_PRM_PDEV:
        {
            switch (event)
            {
                case PNDV_SM_EVENT_PRM_PDEV_DONE:
                {
                    PNIO_UINT32 async_cnt;
                    pndv_data.sm = PNDV_SM_DEVICE_ACTIVE;

                    /* Device starts -> initialize all AR, must be done here again if the device has been shut down */
                    for (async_cnt = 0; async_cnt <= PNDV_CM_AR_NO; async_cnt++) /* <= so that too much AR can be terminated cleanly */
                    {
                        pndv_data.ar[async_cnt].sm_state = PNDV_AR_SM_OFFLINE;
                        pndv_data.ar[async_cnt].ar_nr    = PNDV_AR_NR_NOT_USED;
                    }

                    pndv_data.device_control = CM_SV_DEVICE_CONTROL_CMD_ACTIVATE;
                    pndv_in_device_control(pndv_data.device_control);

                    // provide rqb resource for the DCP set indication (CM_OPC_PD_DCP_QUERY_INDICATION)
                    pndv_in_cm_pd_dcp_indication_provide_resource();

                    // provide rqb resource for the REMA indication (CM_OPC_PD_REMA_INDICATION)
                    pndv_in_cm_pd_rema_indication_provide_resource();

                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error(PNDV_MODULE, __LINE__, event);
                    break;
                }
            }

            break;
        }

        // PNDV CLOSE flow starts here 
        case PNDV_SM_DEVICE_ACTIVE:                     // Stable state of pndv
        {
            switch (event)
            {
                case PNDV_SM_EVENT_STOP:
                {
                    /* stop event received, remove device from cm and start pndv shutdown*/
                    pndv_data.sm = PNDV_SM_W_REMOVE_DEVICE_CNF;
                    pndv_in_cm_sv_device_remove();

                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_SM_W_REMOVE_DEVICE_CNF:               // Device remove is done
        {
            switch (event)
            {
                case PNDV_SM_EVENT_DEVICE_REMOVE_DONE:
                {
                    /* device has been removed, delete cm server now */
                    pndv_data.sm = PNDV_SM_W_DELETE_SERVER_CNF;
                    pndv_in_cm_sv_delete();
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_SM_W_DELETE_SERVER_CNF:               // Cm Server is deleted
        {
            switch (event)
            {
                case PNDV_SM_EVENT_CM_SV_DELETE_DONE:
                {
                    pndv_data.sm = PNDV_SM_W_CLOSE_IOD_CMSV_ACP_CHANNEL_CNF;
                    pndv_in_cm_close_channel(PNDV_INDEX_PATH_IOD_CM_ACP);
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error(PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_SM_W_CLOSE_IOD_CMSV_ACP_CHANNEL_CNF:  // Pndv-cm iod cm server channel is closed
        {
            switch (event)
            {
                case PNDV_SM_EVENT_IOD_CMSV_ACP_CLOSE_CHANNEL_DONE:
                {
                    /* cm server has been deleted, delete pd server now */
                    pndv_data.sm = PNDV_SM_W_DELETE_PD_SERVER_CNF;

                    /* activity */
                    pndv_in_cm_pdsv_delete();
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_SM_W_DELETE_PD_SERVER_CNF:            // PD server is deleted
        {
            /* state: wait on pd server to be deleted */
            switch (event)
            {
                case PNDV_SM_EVENT_DELETE_PD_SERVER_DONE:
                {
                    /* pd server has been deleted, close CMPDSV channel now */
                    pndv_data.sm = PNDV_SM_W_CLOSE_CMPDSV_ACP_CHANNEL_DONE;

                    /* activity */
                    pndv_in_cm_close_channel(PNDV_INDEX_PATH_CMPDSV_ACP);
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error(PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_SM_W_CLOSE_CMPDSV_ACP_CHANNEL_DONE:   // Pndv-cm pd server channel is closed
        {
            switch (event)
            {
                case PNDV_SM_EVENT_CMPDSV_ACP_CLOSE_CHANNEL_DONE:
                {
                    pndv_data.sm = PNDV_SM_W_CLOSE_IP2PN_CHANNELS_DONE;
                    // (NOT implemented, pass to next state) pndv_ip2pn_channels_close();
                    pndv_sm(PNDV_SM_EVENT_IP2PN_CLOSE_CHANNEL_DONE);
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error(PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_SM_W_CLOSE_IP2PN_CHANNELS_DONE:       // Pndv-ip2pn channels are closed.
        {
            switch (event)
            {
                case PNDV_SM_EVENT_IP2PN_CLOSE_CHANNEL_DONE:
                {
                    /* stop has been finished, return to open state */
                    pndv_data.sm = PNDV_SM_OPEN;
                    PNDV_STOP_DONE(PNDV_OK);
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }

        case PNDV_SM_STOP_W_CM_CNF:
        {
            /* state: waiting for cm request to be finished to stop pndv */
            switch (event)
            {
                case PNDV_SM_EVENT_CREATE_PD_SERVER_DONE:
                {
                    /* cm channels were about to be opened while a stop was received */
                    /* and can now be closed already */
                    pndv_data.sm = PNDV_SM_W_DELETE_PD_SERVER_CNF;
                    pndv_in_cm_pdsv_delete();
                    break;
                }
                case PNDV_SM_EVENT_OPEN_IOD_CMSV_ACP_CHANNEL_DONE:
                case PNDV_SM_EVENT_IP2PN_SET_IPSUITE_DONE:
                {
                    /* PD Server was about to be created while a stop was received */
                    /* and can now be deleted already */
                    pndv_data.sm = PNDV_SM_W_CLOSE_IOD_CMSV_ACP_CHANNEL_CNF;
                    /* firstly must be closed ACP channel */
                    pndv_in_cm_close_channel(PNDV_INDEX_PATH_IOD_CM_ACP);
                    break;

                }
                case PNDV_SM_EVENT_CREATE_CM_SERVER_DONE:
                {
                    /* CM Server was about to be created while a stop was received */
                    /* and can now be deleted already */
                    pndv_data.sm = PNDV_SM_W_DELETE_SERVER_CNF;

                    /* activity */

                    pndv_in_cm_sv_delete();
                    break;
                }
                case PNDV_SM_EVENT_DEVICE_ADD_DONE:
                {
                    /* device was about to be added while a stop was received */
                    /* and can now be removed already */
                    pndv_data.sm = PNDV_SM_W_REMOVE_DEVICE_CNF;

                    /* activity */

                    pndv_in_cm_sv_device_remove();
                    break;
                }
                default:
                {
                    /* event mismatch */
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        default:
        {
            /* state mismatch */
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.sm);
            break;
        }
    }
}


/**
 *  @brief cyclic service function
 *
 *  This function checks if there is any work to do within pndv.
 *  The function must be called repeatedly within a task/thread.
 *
 */
PNIO_VOID pndv_perform_services (PNIO_VOID)
{
        PNDV_IFACE_CMD_ENTRY_T   event;

        for(event = pndv_in_peri_read_coupl_event();event.cmd != PNDV_EV_NO;event = pndv_in_peri_read_coupl_event())    /*lint !e441*/
        {
                /* alle events lesen */

                pndv_in_peri_check_event( event);
        }
}


/**
 *  @brief Provides the pnd version
 *
 *  @param[in]  version_len length of user provided version buffer (must be at least the size of LSA_VERSION_TYPE)
 *  @param[out] version_ptr pointer to a user provided version buffer
 *  @return     0: OK, version_len: error
 *
 *  This function returns the version information of the pndv into the user provided buffer
 *  in type of LSA_VERSION_TYPE.
 *
 */
PNIO_UINT16 pndv_version(PNIO_UINT16 version_len, PNDV_SYS_VERSION_PTR version_ptr)
{
        if ( sizeof( LSA_VERSION_TYPE) <= version_len )
        {
                PNIO_UINT32 i;
                PNIO_UINT8  tmp_prefix[LSA_PREFIX_SIZE] = PNDV_LSA_PREFIX;

                version_ptr->lsa_component_id = PNDV_LSA_COMPONENT_ID;

                for ( i = 0; LSA_PREFIX_SIZE > i; i++ )
                {
                        version_ptr->lsa_prefix[i] = tmp_prefix[i];
                }

                version_ptr->kind                = PNDV_KIND;
                version_ptr->version             = PNDV_VERSION;
                version_ptr->distribution        = PNDV_DISTRIBUTION;
                version_ptr->fix                 = PNDV_FIX;
                version_ptr->hotfix              = PNDV_HOTFIX;
                version_ptr->project_number      = PNDV_PROJECT_NUMBER;
                version_ptr->increment           = PNDV_INCREMENT;
                version_ptr->integration_counter = PNDV_INTEGRATION_COUNTER;
                version_ptr->gen_counter         = PNDV_GEN_COUNTER;

                return( 0);
        }
        else
        {
                return( version_len);
        }
}


/**
 *  @brief external debug entry
 *
 *  @param[in] detail_1 debug detail 1
 *  @param[in] detail_2 debug detail 2
 *
 *  This function can be used by other components to write external marks into
 *  pndv trace.
 *
 *  @warning Not thread safe keep task context in mind.
 *
 */
PNIO_VOID pndv_debug_extern(PNIO_UINT16 detail_1, PNIO_UINT16 detail_2)
{
        pndv_in_write_debug_buffer_all__((PNDV_DEBUG_CODE_E_T)detail_1, detail_2);
}


/*****************************************************************************/


/**
 *  @brief provide information about trace buffer
 *
 *  @param[out] debug_info User provided buffer for debug information
 *
 *  This function provides information of the internal trace buffer to
 *  the system integration (e.g. for saving or transmitting in any way).
 *  User must provide a pointer to a buffer of type ::PNDV_SYS_DEBUG_INFO_PTR
 *
 */
PNIO_VOID pndv_debug_get_info (PNDV_SYS_DEBUG_INFO_PTR debug_info)
{

#ifdef PNDV_CFG_DEBUG_ENABLE

    debug_info->trace_data_ptr = (PNIO_UINT8*)&pndv_debug_buffer;
    debug_info->trace_data_len = sizeof(pndv_debug_buffer);

    debug_info->local_data_ptr = (PNIO_UINT8*)&pndv_data;
    debug_info->local_data_len = sizeof(pndv_data);


#else

    debug_info->local_data_ptr = (PNIO_UINT8*)&pndv_deb_str_not_supp;
    debug_info->local_data_len = sizeof(pndv_deb_str_not_supp);

#endif

}


/**
 *  @brief Timeout handler for connect ind delay
 *
 *  @param[in]  timer_id ID of the given timer, this is dont care as only one timer is used.
 *  @param[in]  user_id user id as given with alloc timer, dont care
 *
 *  This function is set as a handler to process timeout events.
 *  The corresponding timer is used to delay a connect request at startup, if peri is still not available
 *  to prevent connect ind polling.
 *  This function is used as a wrapper to handle the timeout in pndv context.
 *
 */
PNIO_VOID pndv_connect_ind_trigger_timeout(PNIO_UINT16 timer_id, PNDV_USER_ID_TYPE user_id)
{
    PNDV_RQB_SET_OPCODE(&pndv_data.rqb.connect_indication_timeout_rqb, PNDV_OPC_TIMEOUT_INTERNAL);
    PNDV_REQUEST_LOCAL(&pndv_data.rqb.connect_indication_timeout_rqb);
}


PNIO_VOID pndv_request(PNDV_RQB_PTR_TYPE rqb_ptr)
{
    switch (PNDV_RQB_GET_OPCODE(rqb_ptr))
    {
        case PNDV_OPC_TIMEOUT_INTERNAL:
        {
            pndv_ar_sys_connect_ind_timeout();

            break;
        }
#ifdef PNDV_CFG_USE_DEVICE_READ
        case PNDV_OPC_DEVICE_READ:
        {
            pndv_cm_in_device_read_req(rqb_ptr);
            break;
        }
#endif
        default:
        {
            /* unknown opcode */
            pndv_in_fatal_error(PNDV_MODULE, __LINE__,  PNDV_RQB_GET_OPCODE(rqb_ptr));
        }
        break;
    }


}


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
 *  @brief Startup of pn device
 *
 *  Send the start signal to pndv state machine to begin the startup of the
 *  pn device.
 *
 *  @warning With V2.x this function must no longer be called by systemintegration
 *           it is triggerd by a peri event (PNDV_EV_TO_PNDV_PNDV_START).
 */
PNIO_VOID pndv_in_start( PNIO_VOID)
{
    pndv_in_write_debug_buffer_all__(PNDV_DC_ALL_START, 0);

    if (PNDV_SM_OPEN == pndv_data.sm)
    {
        pndv_sm(PNDV_SM_EVENT_IP2PN_OPEN);
    }
    else
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, pndv_data.sm);
    }
}


/**
 *  @brief Shutdown of pn device
 *
 *
 *  Send the stop signal to pndv state machine to begin the shutdown of the
 *  pn device.
 *
 *  @warning With V2.x this function must no longer be called by systemintegration
 *           it is triggerd by a peri event (PNDV_EV_TO_PNDV_PNDV_STOP).
 *
 */
PNIO_VOID pndv_in_stop(PNIO_VOID)
{
    pndv_in_write_debug_buffer_all__(PNDV_DC_ALL_STOP, 0);

    if (PNDV_SM_W_OPEN_IOD_CMSV_ACP_CHANNEL_DONE > pndv_data.sm)
    {
        PNDV_STOP_DONE(PNDV_ERR_SEQUENCE);
    }
    else
    {
        pndv_sm(PNDV_SM_EVENT_STOP);
    }
}

/**
 *  @brief Check for errors to be displayed
 *
 *  Checks for errors or informations to be displayed by the device leds.
 *
 */
PNIO_VOID pndv_in_check_led_info(PNIO_VOID)
{
    if (  (pndv_glob_data.led_info & CM_SV_DEVICE_LED_INFO_DIAG)
        ||(pndv_glob_data.led_info & CM_SV_DEVICE_LED_INFO_ARP)
        )
    {
        /* diag-error */

        if (PNIO_TRUE != pndv_glob_data.sf_on)
        {
            pndv_glob_data.sf_on = PNIO_TRUE;

            pndv_in_write_debug_buffer_all__(PNDV_DC_SF_ON, 0);

            PNDV_LED_CTRL_SYS_ERROR();

            /* Dependencies of the Maint_LED on SF_LED? */
            PNDV_MAINT_LED_CONTROL(pndv_glob_data.sf_on)
        }
    }
    else if (  (pndv_glob_data.led_info & CM_SV_DEVICE_LED_INFO_LOCKED_SO)
             ||(pndv_glob_data.led_info & CM_SV_DEVICE_LED_INFO_WRONG)
             ||(pndv_glob_data.led_info & CM_SV_DEVICE_LED_INFO_NO)
            )
    {
        /* turn sf led on if ar is established so_locked */
        if (PNIO_TRUE != pndv_glob_data.sf_on)
        {
            pndv_glob_data.sf_on = PNIO_TRUE;

            pndv_in_write_debug_buffer_all__(PNDV_DC_SF_ON, 0);

            PNDV_LED_CTRL_SYS_ERROR();

            /* Dependencies of the Maint_LED on SF_LED? */
            PNDV_MAINT_LED_CONTROL(pndv_glob_data.sf_on)
        }
    }
    else
    {
        if (PNIO_FALSE != pndv_glob_data.sf_on)
        {
            pndv_glob_data.sf_on = PNIO_FALSE;

            pndv_in_write_debug_buffer_all__(PNDV_DC_SF_OFF, 0);

            PNDV_LED_CTRL_NO_SYS_ERROR();

            /* Dependencies of the Maint_LED on SF_LED? */
            PNDV_MAINT_LED_CONTROL(pndv_glob_data.sf_on)
        }
    }
}

PNIO_VOID pndv_in_check_sync_info(PNIO_VOID)
{
    PNIO_UINT32 sync_led_local_on = 0;

    // check if any of the ports is sync
    if (pndv_glob_data.sync_led_info == CM_PD_SYNC_LED_INFO_SYNC) sync_led_local_on = 1;

    if (sync_led_local_on != pndv_glob_data.syn_led_on)
    {
        // need to change state
        if (sync_led_local_on == 1)
        {
            // switch on
            PNDV_LED_CTRL_IN_SYNC();
        }
        else
        {
            // swtich off
            PNDV_LED_CTRL_OUT_OFF_SYNC();
        }

        pndv_glob_data.syn_led_on = sync_led_local_on;
    }
}

#ifndef PNDV_CFG_USE_FAST_INDEX_SEARCH
PNIO_VOID pndv_get_submod_resource_ptr(PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T *res_ptr_ptr, PNIO_UINT16 slot, PNIO_UINT16 subslot, PNIO_UINT32 *error)
{
    PNIO_UINT32                          subslot_count;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T  local_sub_res_ptr;
    // there will be no error, every range ob slot and subslot is allowed that way
    *error = PNIO_FALSE;

    // by default no resource is found
    *res_ptr_ptr = 0;

    // not to search in every list, just check all resources
    // this may be slow if there are very much subslot resources defined

    for(subslot_count = 0; subslot_count < PNDV_MAX_SV_ENTITY; subslot_count++)
    {
        local_sub_res_ptr = &pndv_data.cfg.peri_cfg.sub_res[subslot_count];
        if(  (local_sub_res_ptr->entity != PNDV_CFG_UNUSED_ENTITY)
           &&(local_sub_res_ptr->res_state != PNDV_CFG_SUBMODULE_RES_STATE_FREE)
           )
        {
            //the resource is occupied
            if (   (local_sub_res_ptr->sub_module.slot_nr == slot)
                 &&(local_sub_res_ptr->sub_module.subslot_nr == subslot)
                )
            {
                //found
                *res_ptr_ptr = local_sub_res_ptr;
                break;
            }
        }
    }

    return;
}
#endif
/*****************************************************************************/
#ifdef PNDV_CFG_DEBUG_ENABLE


/**
 *  @brief Controll level of debug trace
 *
 *  @param[in] debug_control Mode that is set for the internal debug system
 *  @return     PNDV_OK: OK, PNDV_ERR_NOT_SUPPORTED: unknwon debug mode
 *
 *  Trace can be turned on or off at any time.
 *
 */
PNIO_UINT32 pndv_in_debug_control(PNDV_DEBUG_MODE_TYPE debug_control)
{
        PNIO_UINT32 ret_val = PNDV_OK;

        switch( debug_control )
        {
            case PNDV_DEBUG_OFF :
            case PNDV_DEBUG_ON_ALL :
                pndv_data.debug_control = debug_control;
                break;

            default:
                ret_val = PNDV_ERR_NOT_SUPPORTED;
                break;

        } /* end switch( debug_control ) */

        return( ret_val );
}




#ifdef PNDV_TRACE_MSG
PNIO_UINT8 deb_code_str[100];
#endif

/**
 *  @brief Write a debug trace entry
 *
 *  @param[in] detail_1 First detail value of the trace entry
 *  @param[in] detail_2 Second detail value of the trace entry
 *  @param[in] detail_3 Third detail value of the trace entry
 *  @param[in] deb_code actual debug code
 *  @param[in] add      additional information (not used at the moment)
 *
 *  This function inserts a new entry to the debug buffer but only if the appropriate
 *  debug mode is set.
 *
 */
PNIO_VOID pndv_in_write_debug_buffer(PNIO_UINT16 detail_1, PNIO_UINT16 detail_2, PNIO_UINT32 detail_3, PNDV_DEBUG_CODE_E_T deb_code, PNIO_UINT32 add)
{
#ifdef PNDV_TRACE_MSG
    // PNIO_UINT8* p_deb_code_str;
    PNDV_TRACE_MSG_GET_STR(LSA_TRACE_LEVEL_NOTE_HIGH, &deb_code_str[0], deb_code, detail_1, detail_2, detail_3);
    PNDV_TRACE_MSG(0,      LSA_TRACE_LEVEL_NOTE_HIGH, (PNIO_CHAR*)&deb_code_str[0], detail_1, detail_2, detail_3);
#else
    if ( PNDV_DEBUG_OFF != pndv_data.debug_control)
    {
        if ( 1
#ifdef PNDV_DEBUG_DS_TRACE_DISABLED
            &&(PNDV_DC_DFC_DS_DONE      != deb_code)
            &&(PNDV_DC_DFC_CM_DS_RD_REQ != deb_code)
#endif
            )
        {
            PNIO_UINT32 buf_idx;

            PNDV_TRC_TEH_FILL( pndv_debug_buffer, buf_idx)

            PNDV_TRC_TE_ACCESS( pndv_debug_buffer, buf_idx).debug_code = deb_code;
            PNDV_TRC_TE_ACCESS( pndv_debug_buffer, buf_idx).detail_1   = detail_1;
            PNDV_TRC_TE_ACCESS( pndv_debug_buffer, buf_idx).detail_2   = detail_2;
            PNDV_TRC_TE_ACCESS( pndv_debug_buffer, buf_idx).detail_3   = detail_3;
        }
    }
#endif
}

#endif /*#ifdef PNDV_CFG_DEBUG_ENABLE*/

/**
 *  @brief Fatal error
 *
 *  @param[in] error_module Module number where the error occurred
 *  @param[in] error_line   Line within the module where the error occurred
 *  @param[in] error_code   Error code
 *
 *  This function is called within pndv whenever an error occurred that can not be
 *  handled and pndv will no longer work correctly.
 *
 */
PNIO_VOID pndv_in_fatal_error(PNIO_UINT8 error_module, PNIO_UINT16 error_line, PNIO_UINT32 error_code)
{
    pndv_in_write_debug_buffer_all__(PNDV_DC_FATAL, error_line);

    pndv_errcb.lsa_component_id  = PNIO_PACKID_PNDV;

    pndv_errcb.module_id         = error_module;
    pndv_errcb.line              = error_line;

    pndv_errcb.error_code[0]     = error_code;
    pndv_errcb.error_code[1]     =
    pndv_errcb.error_code[2]     =
    pndv_errcb.error_code[3]     = 0;

    pndv_errcb.error_data_length = (PNIO_UINT16)sizeof(pndv_data);
    pndv_errcb.error_data_ptr    = &pndv_data;

    PNDV_FATAL_ERROR(&pndv_errcb);
    /*NOTREACHED*/
}

/*****************************************************************************/
/*  end of file.                                                             */
/*****************************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
