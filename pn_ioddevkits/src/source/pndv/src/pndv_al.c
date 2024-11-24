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
/*  F i l e               &F: pndv_al.c                                 :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Profinet device (cm-user) Alarm and diag handling                        */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/* contents:


    - pndv_in_al_init
    - pndv_in_al_init_submodul

    - pndv_in_al_check_peri_dial
    - pndv_in_al_peri_dial_rmv_all
    - pndv_in_al_peri_pral_req
    - pndv_in_al_ros_al_req
    - pndv_in_al_diag_req
    - pndv_in_al_check_diag
    - pndv_in_al_check_dial_continue
    - pndv_in_al_search_clear_diag

    - pndv_in_al_diag_branch
    - pndv_in_al_channel_diag
    - pndv_in_al_generic_diag
    - pndv_in_al_diag_add_done
    - pndv_in_al_diag_remove_done
    - pndv_in_al_diag_done

    - pndv_in_al_dial_send
    - pndv_in_al_al_ack

    - pndv_in_al_sv_alarm_ind
    - pndv_in_al_sv_alarm_ack_done

    - pndv_in_al_check_modul_error
    - pndv_in_al_set_sf_led_state
    - pndv_in_al_if_al_possib_with_ar_state



*/
/*****************************************************************************/
/* 2do:


*/
/*****************************************************************************/
/* include hierarchy */

#include "pndv_inc.h"

#define PNDV_MODULE PNDV_ERR_MODULE_AL

/**
 * @file pndv_al.c
 * @brief PNDV alarm and diagnosis handling
 * @author cn3dit09
 *
 *
 *
 */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**********                                                         **********/
/**********                   INTERNAL FUNCTIONS                    **********/
/**********                                                         **********/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


/*****************************************************************************/


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**********                                                         **********/
/**********               SYSTEM INTERFACE FUNCTIONS                **********/
/**********                                                         **********/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


/*****************************************************************************/


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**********                                                         **********/
/**********                CM INTERFACE FUNCTIONS                   **********/
/**********                                                         **********/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/**
 *  @brief alarm init
 *
 *  Initialization of alarm and diagnosis specific information and resources.
 *
 */
PNIO_VOID pndv_in_al_init (PNIO_VOID)
{
    PNIO_UINT32 tmp_cnt;
    PNIO_UINT16 subslot_cnt;

    pndv_data.diag.cm_diag_req_running    = PNIO_FALSE;
    pndv_data.diag.cm_dev_req_ignore_done = PNIO_FALSE;
    pndv_data.diag.cm_dev_req_running     = 0;
    pndv_data.diag.cm_dev_req.pending     = 0;

    pndv_data.diag.submodul_rmv_done_err_count = 0;

    /* Channel diagnostic resources */

    PNDV_LIST_INITIALIZE (&pndv_data.diag.channel_diag_free_list);
    PNDV_LIST_INITIALIZE (&pndv_data.diag.channel_diag_new_list);

    pndv_data.diag.channel_diag_new_list_bookmark = 0;

    for ( tmp_cnt = 0; tmp_cnt < PNDV_DIAG_CHANNEL_ANZ ; tmp_cnt++)
    {
        pndv_data.diag.channel_diag[tmp_cnt].ch_diag_sm          = PNDV_DIAG_CH_STATE_FREE;
        pndv_data.diag.channel_diag[tmp_cnt].as.ch.quit_to_peri  = PNIO_FALSE;

        PNDV_LIST_INSERT_TAIL(&pndv_data.diag.channel_diag_free_list, &pndv_data.diag.channel_diag[tmp_cnt].link);
    }

    for ( subslot_cnt = 0; subslot_cnt < PNDV_MAX_SV_ENTITY; subslot_cnt++ )
    {
        pndv_in_al_init_submodul( &pndv_data.al.submodul[subslot_cnt] );

        pndv_data.diag.ch_diag_to_cm_count[subslot_cnt] =
        pndv_data.diag.dial_repeat_req[subslot_cnt]     = PNIO_FALSE;
    }

    /* values that come with CM_OPC_SV_DEVICE_LED_INFO */
    pndv_glob_data.led_info       = 0;
    pndv_glob_data.led_maint_info = 0;

    pndv_glob_data.sf_on          = 3; /* not TRUE and not FALSE */
    pndv_glob_data.maint_on       = 3; /* not TRUE and not FALSE */
    pndv_glob_data.save_maint_on  = PNIO_FALSE;

}

//! @name Internal
//@{
/**
 *  @brief Init submodule dependent information
 *
 *  @param[in]  al_submodul_ptr pointer to one submodule alarm resource
 *
 *  Initialization of a submodule alarm resource wich are part of
 *  ::PNDV_AL_DATA_S. See also ::PNDV_DATA_S
 *
 *
 */
PNIO_VOID pndv_in_al_init_submodul (PNDV_AL_SUBMODUL_DATA_T *al_submodul_ptr )
{
    PNIO_UINT32 tmp_cnt;

    /* Dial dates */
    /* ----------------------------------------------------------------- */

    al_submodul_ptr->dial_state = PNDV_AL_FREE;

    /* Alarm-data-struc in RQB */

    al_submodul_ptr->dial_rqb.args.sv.ar_alarm_send = (CM_UPPER_ALARM_PTR_TYPE)&al_submodul_ptr->dial_data;

    al_submodul_ptr->dial_data.device_nr            = PNDV_CM_DEVICE_NO;

    /*Pral dates */
    /* ----------------------------------------------------------------- */

    al_submodul_ptr->pral_state = PNDV_AL_FREE;

    /* Hang alarm data struc in RQB */

    al_submodul_ptr->pral_rqb.args.sv.ar_alarm_send = (CM_UPPER_ALARM_PTR_TYPE)&al_submodul_ptr->pral_data;

    al_submodul_ptr->pral_data.device_nr           = PNDV_CM_DEVICE_NO;
    al_submodul_ptr->pral_data.alarm_type          = CM_ALARM_TYPE_PROCESS;
    al_submodul_ptr->pral_data.alarm_tag           = PNDV_AL_USI_PROCESSALARM;
    al_submodul_ptr->pral_data.alarm_data_length   = PNDV_AL_PRAL_INFO_MAX_LEN;

    /* Hang additional alarm info in alarm data structure */

    al_submodul_ptr->pral_data.alarm_data  = &al_submodul_ptr->pral_info[0];

    for (tmp_cnt = 0; tmp_cnt < al_submodul_ptr->pral_data.alarm_data_length; tmp_cnt++ )
    {
        al_submodul_ptr->pral_info[tmp_cnt] = 0;
    }

    /* UPAL data */
    /* ----------------------------------------------------------------- */

    al_submodul_ptr->upal_state = PNDV_AL_FREE;

    /* Hang alarm data struc in RQB */

    al_submodul_ptr->upal_rqb.args.sv.ar_alarm_send = (CM_UPPER_ALARM_PTR_TYPE)&al_submodul_ptr->upal_data;

    al_submodul_ptr->upal_data.device_nr           = PNDV_CM_DEVICE_NO;
    al_submodul_ptr->upal_data.alarm_type          = CM_ALARM_TYPE_UPDATE;
    al_submodul_ptr->upal_data.alarm_tag           = PNDV_AL_USI_UPDATEALARM;
    al_submodul_ptr->upal_data.alarm_data_length   = PNDV_AL_UPAL_INFO_LEN;

    /* Hang additional alarm info in alarm data structure */

    al_submodul_ptr->upal_data.alarm_data  = &al_submodul_ptr->upal_info[0];

    for (tmp_cnt = 0; tmp_cnt < PNDV_AL_UPAL_INFO_LEN; tmp_cnt++ )
    {
        al_submodul_ptr->upal_info[tmp_cnt] = 0;
    }

    /* URAL data */
    /* ----------------------------------------------------------------- */

    al_submodul_ptr->ural_state = PNDV_AL_FREE;

    /* Hang alarm data struc in RQB */

    al_submodul_ptr->ural_rqb.args.sv.ar_alarm_send = (CM_UPPER_ALARM_PTR_TYPE)&al_submodul_ptr->ural_data;

    al_submodul_ptr->ural_data.device_nr           = PNDV_CM_DEVICE_NO;
    al_submodul_ptr->ural_data.alarm_type          = CM_ALARM_TYPE_UPLOAD_AND_STORAGE;
    al_submodul_ptr->ural_data.alarm_tag           = PNDV_AL_USI_IPARAMETER;
    al_submodul_ptr->ural_data.alarm_data_length   = PNDV_AL_URAL_INFO_LEN;

    /* Hang additional alarm info in alarm data structure */

    al_submodul_ptr->ural_data.alarm_data  = &al_submodul_ptr->ural_info[0];

    for (tmp_cnt = 0; tmp_cnt < PNDV_AL_URAL_INFO_LEN; tmp_cnt++ )
    {
        al_submodul_ptr->ural_info[tmp_cnt] = 0;
    }

    /* STAL data */
    /* ----------------------------------------------------------------- */

    al_submodul_ptr->stal_state = PNDV_AL_FREE;

    /* Hang alarm data struc in RQB */

    al_submodul_ptr->stal_rqb.args.sv.ar_alarm_send = (CM_UPPER_ALARM_PTR_TYPE)&al_submodul_ptr->stal_data;

    al_submodul_ptr->stal_data.device_nr           = PNDV_CM_DEVICE_NO;
    al_submodul_ptr->stal_data.alarm_type          = CM_ALARM_TYPE_STATUS;
    al_submodul_ptr->stal_data.alarm_tag           = PNDV_AL_USI_IPARAMETER;
    al_submodul_ptr->stal_data.alarm_data_length   = PNDV_AL_STAL_INFO_LEN;

    /* Hang additional alarm info in alarm data structure */

    al_submodul_ptr->stal_data.alarm_data  = &al_submodul_ptr->stal_info[0];

    for (tmp_cnt = 0; tmp_cnt < PNDV_AL_STAL_INFO_LEN; tmp_cnt++ )
    {
        al_submodul_ptr->stal_info[tmp_cnt] = 0;
    }

    /* Rosal dates */
    /* ----------------------------------------------------------------- */

    al_submodul_ptr->rosal_state = PNDV_AL_FREE;

    /* Hang alarm data struc in RQB */

    al_submodul_ptr->rosal_rqb.args.sv.ar_alarm_send = (CM_UPPER_ALARM_PTR_TYPE)&al_submodul_ptr->rosal_data;

    al_submodul_ptr->rosal_data.device_nr          = PNDV_CM_DEVICE_NO;
    al_submodul_ptr->rosal_data.api                = 0;
    al_submodul_ptr->rosal_data.alarm_type         = CM_ALARM_TYPE_RETURN_OF_SUBMODULE;
    al_submodul_ptr->rosal_data.alarm_tag          = PNDV_AL_USI_RETURNOFSUBMODULALARM;
    al_submodul_ptr->rosal_data.alarm_data_length  = 0;
    al_submodul_ptr->rosal_data.alarm_data         = 0;
}
//@}


//! @name Interface
//@{
/**
 *  @brief evaluates channel diagnosis
 *
 *  @param[in]  entity_nr Number of the entity equals the real config entity nr
 *  @param[in]  real_cfg_channel_diag_ptr
 *
 *
 *
 */
PNIO_VOID pndv_al_peri_check_dial(PNIO_UINT16 entity_nr, PNDV_REAL_CFG_T *real_cfg_channel_diag_ptr)
{
    PNIO_UINT32             anz_chann_diag;
    PNDV_CHANNEL_DIAG_REQ_T req;
    PNIO_UINT32              i;


    /* only the channel diagnostics are evaluated, 
       the periphery only reports an outgoing DS1 without channel diagnostics if the load voltage is lost,
       but in this case there is always a breakdown message, 
       the module is removed and the channel diagnostics are invalid
    */

    if ( !pndv_host_ptr_is_nil__(real_cfg_channel_diag_ptr) )
    {
        anz_chann_diag = real_cfg_channel_diag_ptr->dial.anz_chn_diag;

        if ( PNDV_MAX_CHN_DIAG_PER_SLOT_AND_MOMENT < anz_chann_diag )
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, anz_chann_diag);
        }

        if ( 0 == anz_chann_diag )
        {
            /* all diagnoses have to go */

            pndv_in_al_diag_req( PNDV_AL_USI_CHANNELDIAG, entity_nr, 0, 0, 0, 0, 0, PNDV_CHANNEL_DIAG_RMV_ALL_REQ);
        }
        else
        {
            for (i = 0; i < anz_chann_diag; i++)
            {
                if (PNIO_TRUE == real_cfg_channel_diag_ptr->dial.chn_diag[i].alm_kommend)
                {
                    req = PNDV_CHANNEL_DIAG_ADD_REQ;
                }
                else
                {
                    req = PNDV_CHANNEL_DIAG_RMV_REQ;
                }

                pndv_in_al_diag_req(PNDV_AL_USI_CHANNELDIAG,
                                    entity_nr,
                                    real_cfg_channel_diag_ptr->dial.chn_diag[i].kanal,
                                    real_cfg_channel_diag_ptr->dial.chn_diag[i].fehler,
                                    real_cfg_channel_diag_ptr->dial.chn_diag[i].properties,
                                    0, 0, req);
            }
        }
    }

    pndv_in_check_led_info();
}


//! @name Interface
//@{
/**
 *  @brief evaluates ext channel diagnosis
 *
 *  @param[in]  entity_nr Number of the entity equals the real config entity nr
 *  @param[in]  real_cfg_channel_diag_ptr
 *
 *
 *
 */
PNIO_VOID pndv_al_peri_check_xdial(PNIO_UINT16 entity_nr, PNDV_REAL_CFG_T *real_cfg_channel_diag_ptr)
{
    PNIO_UINT32             anz_chann_diag;
    PNDV_CHANNEL_DIAG_REQ_T req;
    PNIO_UINT32              i;


    /* only the channel diagnostics are evaluated, 
       the periphery only reports an outgoing DS1 without channel diagnostics if the load voltage is lost, 
       but in this case there is always a breakdown message, 
       the module is removed and the channel diagnostics are invalid
    */

    if ( !pndv_host_ptr_is_nil__(real_cfg_channel_diag_ptr) )
    {
        anz_chann_diag = real_cfg_channel_diag_ptr->xdial.anz_chn_diag;

        if ( PNDV_MAX_CHN_DIAG_PER_SLOT_AND_MOMENT < anz_chann_diag )
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, anz_chann_diag);
        }

        if ( 0 == anz_chann_diag )
        {
            /* all diagnoses have to go */

            pndv_in_al_diag_req( PNDV_AL_USI_EXTCHANNELDIAG, entity_nr, 0, 0, 0, 0, 0, PNDV_EXT_CHANNEL_DIAG_RMV_ALL_REQ);
        }
        else
        {
            for (i = 0; i < anz_chann_diag; i++)
            {
                if (PNIO_TRUE == real_cfg_channel_diag_ptr->xdial.ext_diag[i].alm_kommend)
                {
                    req = PNDV_EXT_CHANNEL_DIAG_ADD_REQ;
                }
                else
                {
                    req = PNDV_EXT_CHANNEL_DIAG_RMV_REQ;
                }

                pndv_in_al_diag_req(PNDV_AL_USI_EXTCHANNELDIAG,
                                    entity_nr,
                                    real_cfg_channel_diag_ptr->xdial.ext_diag[i].kanal,
                                    real_cfg_channel_diag_ptr->xdial.ext_diag[i].fehler,
                                    real_cfg_channel_diag_ptr->xdial.ext_diag[i].properties,
                                    real_cfg_channel_diag_ptr->xdial.ext_diag[i].ext_fehler,
                                    real_cfg_channel_diag_ptr->xdial.ext_diag[i].ext_wert,
                                    req);
            }
        }
    }

    pndv_in_check_led_info();
}


/**
 *  @brief remove all channel diagnosis of one ar
 *
 *  @param[in]  ar_idx AR index
 *
 *
 *
 */
PNIO_VOID pndv_al_peri_dial_rmv_all(PNIO_UINT32 ar_idx)
{
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res_ptr;

    for (PNDV_LIST_EACH(local_sub_res_ptr, &pndv_data.cfg.peri_cfg.in_use_list, PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T))
    {   //check all used modules if they are owned by the requested ar, don't use set_cfg information for that
        if (local_sub_res_ptr->own_ar_idx == ar_idx)
        {
            pndv_in_al_diag_req( PNDV_AL_USI_CHANNELDIAG, local_sub_res_ptr->entity, 0, 0, 0, 0, 0, PNDV_CHANNEL_DIAG_RMV_ALL_REQ_PNDV );
        }

    }
}


/**
 *  @brief Indicates a process alarm for one subslot
 *
 *  @param[in]  entity_nr number of the entity equals the real config entity nr
 *  @param[in]  pral_ptr pointer to the process alarm information
 *
 *
 *
 */
PNIO_VOID pndv_al_peri_pral_req(PNIO_UINT16 entity_nr, PNDV_SENDMSG_PRALINFO_PTR pral_ptr)
{
    CM_UPPER_RQB_PTR_TYPE tmp_rqb_ptr;
    PNIO_UINT32           tmp_cnt,
                          subslot_error;
    PNIO_UINT16           slot_nr,
                          soll_slot,
                          subslot_nr;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    slot_nr     = pndv_data.iface_ptr->real_cfg[entity_nr].elem.sub_descr.slot_nr;
    subslot_nr  = pndv_data.iface_ptr->real_cfg[entity_nr].elem.sub_descr.subslot_nr;
    pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
    if(subslot_error)
    {
        // slot/subslot not possible, must be an implementation error
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    if(local_sub_res == 0)
    {
        // submodule not plugged, must be an error here
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, (PNIO_UINT32)local_sub_res);
    }

    if (local_sub_res->entity != entity_nr)
    {
        //! ERROR! the entity of this submodule has changed since it was added
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, local_sub_res->entity );
    }

    soll_slot = slot_nr; /* default 1:1 Mapping */

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_PRAL_REQ, local_sub_res->entity);

    /* In the case of PRALs, the order of events regarding the Alarm.Specifier is not taken into account 
       because I suspect that this must not be matching anyways with PRALst */

    if ( !pndv_host_ptr_is_nil__(pral_ptr) )
    {
        if ( PNDV_MAX_SV_ENTITY >= local_sub_res->entity )
        {
            if ( PNDV_AL_FREE  == pndv_data.al.submodul[local_sub_res->entity].pral_state )
            {
                if (    ( PNDV_SUBMODULE_STATE_VALID_MODUL == local_sub_res->submodule_state )
                     && ( PNIO_TRUE                        == pndv_al_al_possible_within_state(local_sub_res) )
                   )
                {
                    /* greater WAIT OWNER means:
                     * - Submodule is plugged into the CM
                     * - has an owner, is therefore configured
                     * - belongs to a user data AR
                     * submodule must be VALID
                     * AR must be established
                     */

                    /* Set off the alarm */
                    /* ----------------------------------------------------------------- */

                    pndv_data.al.submodul[local_sub_res->entity].pral_state = PNDV_AL_SENT;

                 /* pndv_data.al.submodul[local_sub_res->entity].pral_data.device_nr            = set in pndv_in_al_init_submodul */
                    pndv_data.al.submodul[local_sub_res->entity].pral_data.ar_nr                = (PNIO_UINT16)pndv_data.ar[local_sub_res->own_ar_idx].ar_nr;
                    pndv_data.al.submodul[local_sub_res->entity].pral_data.session_key          = pndv_data.ar[local_sub_res->own_ar_idx].session_key;
                    pndv_data.al.submodul[local_sub_res->entity].pral_data.api                  = local_sub_res->sub_module.api;
                    pndv_data.al.submodul[local_sub_res->entity].pral_data.slot_nr              = soll_slot;
                    pndv_data.al.submodul[local_sub_res->entity].pral_data.subslot_nr           = subslot_nr;
                 /* pndv_data.al.submodul[local_sub_res->entity].pral_data.alarm_type           = set in pndv_in_al_init_submodul */
                    pndv_data.al.submodul[local_sub_res->entity].pral_data.alarm_tag            = pral_ptr->usi;
                 /* pndv_data.al.submodul[local_sub_res->entity].pral_data.alarm_data_length    = set in pndv_in_al_init_submodul */
                    if (pral_ptr->data_length <= PNDV_AL_PRAL_INFO_MAX_LEN)
                    {
                        if (pral_ptr->data_length == 0)
                        {
                            pndv_data.al.submodul[local_sub_res->entity].pral_data.alarm_data           = 0;
                            pndv_data.al.submodul[local_sub_res->entity].pral_data.alarm_data_length    = 0;
                        }
                        else
                        {
                            pndv_data.al.submodul[local_sub_res->entity].pral_data.alarm_data_length    = pral_ptr->data_length;
                            pndv_data.al.submodul[local_sub_res->entity].pral_data.alarm_data           = &pndv_data.al.submodul[local_sub_res->entity].pral_info[0];

                            for (tmp_cnt = 0; tmp_cnt < pndv_data.al.submodul[local_sub_res->entity].pral_data.alarm_data_length; tmp_cnt++)
                            {
                                pndv_data.al.submodul[local_sub_res->entity].pral_info[tmp_cnt] = pral_ptr->data[tmp_cnt];
                            }
                        }
                    }
                    else
                    {
                        pndv_in_fatal_error( PNDV_MODULE, __LINE__, pral_ptr->data_length);
                    }

                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_PRAL_SEND, local_sub_res->entity);

                    PNDV_RQB_SET_HANDLE(&pndv_data.al.submodul[local_sub_res->entity].pral_rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                    PNDV_RQB_SET_OPCODE(&pndv_data.al.submodul[local_sub_res->entity].pral_rqb, CM_OPC_SV_AR_ALARM_SEND);

                    tmp_rqb_ptr = &pndv_data.al.submodul[local_sub_res->entity].pral_rqb;

                    PNDV_REQUEST(tmp_rqb_ptr, LSA_COMP_ID_CM);
                }
                else
                {
                    /* The alarm cannot be triggered
                       -> acknowledge immediately */

                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_PRAL_DONT_SEND, local_sub_res->entity);

                    pndv_in_peri_pral_quit(local_sub_res->entity, PNDV_TO_PERI_OK_NO_ALARM);

                    /* actually it cannot happen that a pral is made for a pulled module, 
                       since prals are processed synchronously when they arrive at PNDV, a pull can only come after them. 
                       PRALs can only be made after parameterization, i.e. the submodule has already been inserted
                    */
                }
            }
            else /* if ( PNDV_AL_FREE  == pndv_data.al.submodul[local_sub_res->entity].pral_state ) */
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.al.submodul[local_sub_res->entity].pral_state);
            }
        }
        else /* if ( PNDV_MAX_SV_ENTITY >= local_sub_res->entity ) */
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, local_sub_res->entity);
        }
    }
    else /* if ( !pndv_host_ptr_is_nil__(pral_ptr) ) */
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }
}


/**
 *  @brief Indicates a update alarm for one subslot
 *
 *  @param[in]  entity_nr number of the entity equals the real config entity nr
 *  @param[in]  upal_ptr pointer to the update alarm information
 *
 */
PNIO_VOID pndv_al_peri_upal_req(PNIO_UINT16 entity_nr, PNDV_SENDMSG_UPALINFO_PTR upal_ptr)
{
    CM_UPPER_RQB_PTR_TYPE               tmp_rqb_ptr;
    PNIO_UINT32                         tmp_cnt,
                                        subslot_error;
    PNIO_UINT16                         slot_nr,
                                        soll_slot,
                                        subslot_nr;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    slot_nr     = pndv_data.iface_ptr->real_cfg[entity_nr].elem.sub_descr.slot_nr;
    subslot_nr  = pndv_data.iface_ptr->real_cfg[entity_nr].elem.sub_descr.subslot_nr;
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

    if (local_sub_res->entity != entity_nr)
    {
        //! ERROR! the entity of this submodule has changed since it was added
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, local_sub_res->entity );
    }

    soll_slot = slot_nr; /* default 1:1 Mapping */

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_UPAL_REQ, local_sub_res->entity);

    if ( !pndv_host_ptr_is_nil__(upal_ptr) )
    {
        if ( PNDV_MAX_SV_ENTITY >= local_sub_res->entity )
        {
            if ( PNDV_AL_FREE  == pndv_data.al.submodul[local_sub_res->entity].upal_state )
            {
                if (    (PNDV_SUBMODULE_STATE_VALID_MODUL == local_sub_res->submodule_state)
                     && (PNIO_TRUE                        == pndv_al_al_possible_within_state(local_sub_res))
                   )
                {
                    /* greater WAIT OWNER means:
                     * - Submodule is plugged into the CM
                     * - has an owner, is therefore configured
                     * - belongs to a user data AR
                     * submodule must be VALID
                     * AR must be established
                     */

                    /* Set off the alarm */
                    /* ----------------------------------------------------------------- */

                    pndv_data.al.submodul[local_sub_res->entity].upal_state = PNDV_AL_SENT;

                 /* pndv_data.al.submodul[local_sub_res->entity].upal_data.device_nr            = set in pndv_in_al_init_submodul */
                    pndv_data.al.submodul[local_sub_res->entity].upal_data.ar_nr                = (PNIO_UINT16)pndv_data.ar[local_sub_res->own_ar_idx].ar_nr;
                    pndv_data.al.submodul[local_sub_res->entity].upal_data.session_key          = pndv_data.ar[local_sub_res->own_ar_idx].session_key;
                    pndv_data.al.submodul[local_sub_res->entity].upal_data.api                  = local_sub_res->sub_module.api;
                    pndv_data.al.submodul[local_sub_res->entity].upal_data.slot_nr              = soll_slot;
                    pndv_data.al.submodul[local_sub_res->entity].upal_data.subslot_nr           = subslot_nr;
                 /* pndv_data.al.submodul[local_sub_res->entity].upal_data.alarm_type           = set in pndv_in_al_init_submodul */
                 /* pndv_data.al.submodul[local_sub_res->entity].upal_data.alarm_tag            = set in pndv_in_al_init_submodul */
                 /* pndv_data.al.submodul[local_sub_res->entity].upal_data.alarm_data_length    = set in pndv_in_al_init_submodul */

                    /* Additional alarm information - blocks are already shown in init */

                    for (tmp_cnt = 0; tmp_cnt < PNDV_AL_UPAL_INFO_LEN; tmp_cnt++)
                    {
                        pndv_data.al.submodul[local_sub_res->entity].upal_info[tmp_cnt] = upal_ptr->data[tmp_cnt];
                    }

                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_UPAL_SEND, local_sub_res->entity);

                    PNDV_RQB_SET_HANDLE(&pndv_data.al.submodul[local_sub_res->entity].upal_rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                    PNDV_RQB_SET_OPCODE(&pndv_data.al.submodul[local_sub_res->entity].upal_rqb, CM_OPC_SV_AR_ALARM_SEND);

                    tmp_rqb_ptr = &pndv_data.al.submodul[local_sub_res->entity].upal_rqb;

                    PNDV_REQUEST(tmp_rqb_ptr, LSA_COMP_ID_CM);
                }
                else
                {
                    /* Alarms can not be abolished
                       -> acknowledge immediately */

                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_UPAL_DONT_SEND, local_sub_res->entity);

                    pndv_in_peri_upal_quit(local_sub_res->entity);
                    /* actually this cannot happen ... */

                }
            }
            else /* if ( PNDV_AL_FREE  == pndv_data.al.submodul[local_sub_res->entity].upal_state ) */
            {
                pndv_in_fatal_error(PNDV_MODULE, __LINE__, pndv_data.al.submodul[local_sub_res->entity].upal_state);
            }
        }
        else /* if ( PNDV_MAX_SV_ENTITY >= local_sub_res->entity ) */
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, local_sub_res->entity);
        }
    }
    else /* if ( !pndv_host_ptr_is_nil__(upal_ptr) ) */
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }
}

/**
 *  @brief Indicates a update alarm for one subslot
 *
 *  @param[in]  entity_nr number of the entity equals the real config entity nr
 *  @param[in]  ural_ptr pointer to the update alarm information
 *
 */
PNIO_VOID pndv_al_peri_ural_req(PNIO_UINT16 entity_nr, PNDV_SENDMSG_URALINFO_PTR ural_ptr)
{
    CM_UPPER_RQB_PTR_TYPE   tmp_rqb_ptr;
    PNIO_UINT32             tmp_cnt,
                            subslot_error;
    PNIO_UINT16             slot_nr,
                            soll_slot,
                            subslot_nr;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    slot_nr     = pndv_data.iface_ptr->real_cfg[entity_nr].elem.sub_descr.slot_nr;
    subslot_nr  = pndv_data.iface_ptr->real_cfg[entity_nr].elem.sub_descr.subslot_nr;
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

    if (local_sub_res->entity != entity_nr)
    {
        //! ERROR! the entity of this submodule has changed since it was added
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, local_sub_res->entity );
    }

    soll_slot = slot_nr; /* default 1:1 Mapping */

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_URAL_REQ, local_sub_res->entity);

    if ( !pndv_host_ptr_is_nil__(ural_ptr) )
    {
        if ( PNDV_MAX_SV_ENTITY >= local_sub_res->entity )
        {
            if ( PNDV_AL_FREE  == pndv_data.al.submodul[local_sub_res->entity].ural_state )
            {
                if (    (PNDV_SUBMODULE_STATE_VALID_MODUL == local_sub_res->submodule_state)
                     && (PNIO_TRUE                        == pndv_al_al_possible_within_state(local_sub_res))
                   )
                {
                    /* greater WAIT OWNER means:
                     * - Submodule is plugged into the CM
                     * - has an owner, is therefore configured
                     * - belongs to a user data AR
                     * submodule must be VALID
                     * AR must be established
                     */

                    /* Set off the alarm */
                    /* ----------------------------------------------------------------- */

                    pndv_data.al.submodul[local_sub_res->entity].ural_state = PNDV_AL_SENT;

                 /* pndv_data.al.submodul[local_sub_res->entity].ural_data.device_nr            = set in pndv_in_al_init_submodul */
                    pndv_data.al.submodul[local_sub_res->entity].ural_data.ar_nr                = (PNIO_UINT16)pndv_data.ar[local_sub_res->own_ar_idx].ar_nr;
                    pndv_data.al.submodul[local_sub_res->entity].ural_data.session_key          = pndv_data.ar[local_sub_res->own_ar_idx].session_key;
                    pndv_data.al.submodul[local_sub_res->entity].ural_data.api                  = local_sub_res->sub_module.api;
                    pndv_data.al.submodul[local_sub_res->entity].ural_data.slot_nr              = soll_slot;
                    pndv_data.al.submodul[local_sub_res->entity].ural_data.subslot_nr           = subslot_nr;
                 /* pndv_data.al.submodul[local_sub_res->entity].ural_data.alarm_type           = set in pndv_in_al_init_submodul */
                    pndv_data.al.submodul[local_sub_res->entity].ural_data.alarm_tag            = ural_ptr->usi;  // usi for ural i provided by user, 0x8200 and 0x8201 possible
                 /* pndv_data.al.submodul[local_sub_res->entity].ural_data.alarm_data_length    = set in pndv_in_al_init_submodul */

                    /* Additional alarm information - blocks are already shown in init */

                    for (tmp_cnt = 0; tmp_cnt < PNDV_AL_URAL_INFO_LEN; tmp_cnt++)
                    {
                        pndv_data.al.submodul[local_sub_res->entity].ural_info[tmp_cnt] = ural_ptr->data[tmp_cnt];
                    }

                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_URAL_SEND, local_sub_res->entity);

                    PNDV_RQB_SET_HANDLE(&pndv_data.al.submodul[local_sub_res->entity].ural_rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                    PNDV_RQB_SET_OPCODE(&pndv_data.al.submodul[local_sub_res->entity].ural_rqb, CM_OPC_SV_AR_ALARM_SEND);

                    tmp_rqb_ptr = &pndv_data.al.submodul[local_sub_res->entity].ural_rqb;

                    PNDV_REQUEST(tmp_rqb_ptr, LSA_COMP_ID_CM);
                }
                else
                {
                    /* The alarm cannot be triggered
                       -> acknowledge immediately */

                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_URAL_DONT_SEND, local_sub_res->entity);

                    pndv_in_peri_ural_quit(local_sub_res->entity);
                    /* actually this cannot happen ... */

                }
            }
            else /* if ( PNDV_AL_FREE  == pndv_data.al.submodul[local_sub_res->entity].ural_state ) */
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.al.submodul[local_sub_res->entity].ural_state);
            }
        }
        else /* if ( PNDV_MAX_SV_ENTITY >= local_sub_res->entity ) */
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, local_sub_res->entity);
        }
    }
    else /* if ( !pndv_host_ptr_is_nil__(ural_ptr) ) */
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }
}

/**
 *  @brief Indicates a update alarm for one subslot
 *
 *  @param[in]  entity_nr number of the entity equals the real config entity nr
 *  @param[in]  stal_ptr pointer to the update alarm information
 *
 */
PNIO_VOID pndv_al_peri_stal_req(PNIO_UINT16 entity_nr, PNDV_SENDMSG_STALINFO_PTR stal_ptr)
{
    CM_UPPER_RQB_PTR_TYPE               tmp_rqb_ptr;
    PNIO_UINT32                         tmp_cnt,
                                        subslot_error;
    PNIO_UINT16                         slot_nr,
                                        soll_slot,
                                        subslot_nr;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    slot_nr     = pndv_data.iface_ptr->real_cfg[entity_nr].elem.sub_descr.slot_nr;
    subslot_nr  = pndv_data.iface_ptr->real_cfg[entity_nr].elem.sub_descr.subslot_nr;
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

    if (local_sub_res->entity != entity_nr)
    {
        //! ERROR! the entity of this submodule has changed since it was added
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, local_sub_res->entity );
    }

    soll_slot = slot_nr; /* default 1:1 Mapping */

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_STAL_REQ, local_sub_res->entity);

    if ( !pndv_host_ptr_is_nil__(stal_ptr) )
    {
        if ( PNDV_MAX_SV_ENTITY >= local_sub_res->entity )
        {
            if ( PNDV_AL_FREE  == pndv_data.al.submodul[local_sub_res->entity].stal_state )
            {
                if (    (PNDV_SUBMODULE_STATE_VALID_MODUL == local_sub_res->submodule_state)
                     && (PNIO_TRUE                        == pndv_al_al_possible_within_state(local_sub_res))
                   )
                {
                    /* greater WAIT OWNER means:
                     * - Submodule is plugged into the CM
                     * - has an owner, is therefore configured
                     * - belongs to a user data AR
                     * submodule must be VALID
                     * AR must be established
                     */

                    /* Set off the alarm */
                    /* ----------------------------------------------------------------- */

                    pndv_data.al.submodul[local_sub_res->entity].stal_state = PNDV_AL_SENT;

                    if ((stal_ptr->usi >= PNDV_AL_STAL_RS_LOW_WATERMARK_USI) && (stal_ptr->usi <= PNDV_AL_STAL_RS_EVENT_USI)) // if Reporting System AlarmItem (RS_AlarmItem)
                    {
                     /* pndv_data.al.submodul[local_sub_res->entity].stal_data.device_nr            = set in pndv_in_al_init_submodul */
                        pndv_data.al.submodul[local_sub_res->entity].stal_data.ar_nr                = (PNIO_UINT16)pndv_data.ar[stal_ptr->ar_idx].ar_nr;
                        pndv_data.al.submodul[local_sub_res->entity].stal_data.session_key          = pndv_data.ar[stal_ptr->ar_idx].session_key;
                        pndv_data.al.submodul[local_sub_res->entity].stal_data.api                  = 0;
                        pndv_data.al.submodul[local_sub_res->entity].stal_data.slot_nr              = 0x8000;
                        pndv_data.al.submodul[local_sub_res->entity].stal_data.subslot_nr           = 0;
                     /* pndv_data.al.submodul[local_sub_res->entity].stal_data.alarm_type           = set in pndv_in_al_init_submodul */
                        pndv_data.al.submodul[local_sub_res->entity].stal_data.alarm_tag            = stal_ptr->usi;  // usi for stal i provided by user, 0x8200 and 0x8201 possible
                     /* pndv_data.al.submodul[local_sub_res->entity].stal_data.alarm_data_length    = set in pndv_in_al_init_submodul */
                    }
                    else
                    {
                     /* pndv_data.al.submodul[local_sub_res->entity].stal_data.device_nr            = set in pndv_in_al_init_submodul */
                        pndv_data.al.submodul[local_sub_res->entity].stal_data.ar_nr                = (PNIO_UINT16)pndv_data.ar[local_sub_res->own_ar_idx].ar_nr;
                        pndv_data.al.submodul[local_sub_res->entity].stal_data.session_key          = pndv_data.ar[local_sub_res->own_ar_idx].session_key;
                        pndv_data.al.submodul[local_sub_res->entity].stal_data.api                  = local_sub_res->sub_module.api;
                        pndv_data.al.submodul[local_sub_res->entity].stal_data.slot_nr              = soll_slot;
                        pndv_data.al.submodul[local_sub_res->entity].stal_data.subslot_nr           = subslot_nr;
                     /* pndv_data.al.submodul[local_sub_res->entity].stal_data.alarm_type           = set in pndv_in_al_init_submodul */
                        pndv_data.al.submodul[local_sub_res->entity].stal_data.alarm_tag            = stal_ptr->usi;  // usi for stal i provided by user, 0x8200 and 0x8201 possible
                     /* pndv_data.al.submodul[local_sub_res->entity].stal_data.alarm_data_length    = set in pndv_in_al_init_submodul */
                    }

                    /* Additional alarm information - blocks are already shown in init */

                    for (tmp_cnt = 0; tmp_cnt < PNDV_AL_STAL_INFO_LEN; tmp_cnt++)
                    {
                        pndv_data.al.submodul[local_sub_res->entity].stal_info[tmp_cnt] = stal_ptr->data[tmp_cnt];
                    }

                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_STAL_SEND, local_sub_res->entity);

                    PNDV_RQB_SET_HANDLE(&pndv_data.al.submodul[local_sub_res->entity].stal_rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                    PNDV_RQB_SET_OPCODE(&pndv_data.al.submodul[local_sub_res->entity].stal_rqb, CM_OPC_SV_AR_ALARM_SEND);

                    tmp_rqb_ptr = &pndv_data.al.submodul[local_sub_res->entity].stal_rqb;

                    PNDV_REQUEST(tmp_rqb_ptr, LSA_COMP_ID_CM);
                }
                else
                {
                    /* The alarm cannot be triggered
                       -> acknowledge immediately */

                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_STAL_DONT_SEND, local_sub_res->entity);

                    pndv_in_peri_stal_quit(local_sub_res->entity, PNDV_TO_PERI_OK_NO_ALARM);
                    /* actually this cannot happen ... */

                }
            }
            else /* if ( PNDV_AL_FREE  == pndv_data.al.submodul[local_sub_res->entity].stal_state ) */
            {
                pndv_in_fatal_error(PNDV_MODULE, __LINE__, pndv_data.al.submodul[local_sub_res->entity].stal_state);
            }
        }
        else /* if ( PNDV_MAX_SV_ENTITY >= local_sub_res->entity ) */
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, local_sub_res->entity);
        }
    }
    else /* if ( !pndv_host_ptr_is_nil__(stal_ptr) ) */
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }
}

/**
 *  @brief Indicates a return of submodule alarm for one subslot
 *
 *  @param[in]  entity_nr number of the entity equals the real config entity nr
 *
 *
 *
 */
PNIO_VOID pndv_al_peri_rosal_req (PNIO_UINT16 entity_nr)
{
    PNIO_UINT16                         slot_nr,
                                        soll_slot,
                                        subslot_nr;
    PNIO_UINT32                         subslot_error;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    slot_nr     = pndv_data.iface_ptr->real_cfg[entity_nr].elem.sub_descr.slot_nr;
    subslot_nr  = pndv_data.iface_ptr->real_cfg[entity_nr].elem.sub_descr.subslot_nr;
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

    if (local_sub_res->entity != entity_nr)
    {
        //! ERROR! the entity of this submodule has changed since it was added
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, local_sub_res->entity );
    }

    soll_slot = slot_nr; /* default 1:1 Mapping */

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_ROSAL_REQ, soll_slot);

    if ( PNDV_MAX_SV_ENTITY >= local_sub_res->entity )
    {
        if ( PNDV_AL_FREE  == pndv_data.al.submodul[local_sub_res->entity].rosal_state )
        {
            if(   (local_sub_res->submodule_state                   == PNDV_SUBMODULE_STATE_VALID_MODUL)
               && (local_sub_res->own_ar_idx                        != PNDV_AR_IDX_NOT_USED)
               && (pndv_data.ar[local_sub_res->own_ar_idx].sm_state != PNDV_AR_SM_DISCONNECT_IND_W_DONE)
               && (pndv_data.ar[local_sub_res->own_ar_idx].sm_state != PNDV_AR_SM_W_DISCONNECT_IND_DONE)
              )
            {
                /* subslot is owned and AR is currently not aborted -> try to send it */

                if (    ( (pndv_data.ar[local_sub_res->own_ar_idx].sm_state == PNDV_AR_SM_IN_DATA )
                        ||(pndv_data.ar[local_sub_res->own_ar_idx].sm_state == PNDV_AR_SM_IN_DATA_RTC3))
                     && (  local_sub_res->res_state                         >= PNDV_CFG_SUBMODULE_RES_STATE_W_IN_DATA )
                   )
                {
                    /* ROS-submodule must be
                     * - owned by an AR
                     * - AR should be in state IN_DATA (otherwise alarm is queued and executed after IN_DATA)
                     * - submodule state should be W_IN_DATA, at least (should be in IN_DATA. If we supported ROS after ARP,
                     *   PP_SM would stick in W_IN_DATA (=> this is the minimum required state))
                     */

                    /* Send alarm */
                    /* ----------------------------------------------------------------- */

                    pndv_data.al.submodul[local_sub_res->entity].rosal_state = PNDV_AL_SENT;

                 /* pndv_data.al.submodul[local_sub_res->entity].rosal_data.device_nr           = set in pndv_in_al_init_submodul */
                    pndv_data.al.submodul[local_sub_res->entity].rosal_data.ar_nr               = (PNIO_UINT16)pndv_data.ar[local_sub_res->own_ar_idx].ar_nr;
                    pndv_data.al.submodul[local_sub_res->entity].rosal_data.session_key         = pndv_data.ar[local_sub_res->own_ar_idx].session_key;
                    pndv_data.al.submodul[local_sub_res->entity].rosal_data.api                 = local_sub_res->sub_module.api;
                    pndv_data.al.submodul[local_sub_res->entity].rosal_data.slot_nr             = soll_slot;
                    pndv_data.al.submodul[local_sub_res->entity].rosal_data.subslot_nr          = subslot_nr;
                 /* pndv_data.al.submodul[local_sub_res->entity].pral_data.alarm_type           = set in pndv_in_al_init_submodul */
                 /* pndv_data.al.submodul[local_sub_res->entity].pral_data.alarm_tag            = set in pndv_in_al_init_submodul */
                 /* pndv_data.al.submodul[local_sub_res->entity].pral_data.alarm_data_length    = set in pndv_in_al_init_submodul */

                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_ROSAL_SEND, local_sub_res->entity);

                    PNDV_RQB_SET_HANDLE(&pndv_data.al.submodul[local_sub_res->entity].rosal_rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                    PNDV_RQB_SET_OPCODE(&pndv_data.al.submodul[local_sub_res->entity].rosal_rqb, CM_OPC_SV_AR_ALARM_SEND);

                    PNDV_REQUEST(&pndv_data.al.submodul[local_sub_res->entity].rosal_rqb, LSA_COMP_ID_CM);
                }
                else
                {
                    PNDV_PERI_ALARM_ELEMENT_PTR_T local_list_elem_ptr;

                    // AR or submodule not in requested state -> queue and wait for the AR to get into a suitable state
                    local_list_elem_ptr = PNDV_LIST_FIRST(&pndv_data.al.q_rosal.free_list, PNDV_PERI_ALARM_ELEMENT_PTR_T);

                    if (local_list_elem_ptr != LSA_NULL)
                    {
                        PNDV_LIST_REMOVE_ENTRY(&local_list_elem_ptr->link);

                        local_list_elem_ptr->entity_nr = entity_nr;

                        PNDV_LIST_INSERT_TAIL(&pndv_data.al.q_rosal.in_use, &local_list_elem_ptr->link);
                    }

                }
            }
            else
            {
                // submodule not owned OR AR is currently aborted -> discard alarm and quit to peri
                //   this is used if AR doesn't get into IN_DATA state and is disconnected then -> we have to clean up.
                pndv_in_peri_ros_al_quit(local_sub_res->entity);
            }
        }
        else /* if ( PNDV_AL_FREE  == pndv_data.al.submodul[local_sub_res->entity].rosal_state ) */
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, pndv_data.al.submodul[local_sub_res->entity].rosal_state);
        }
    }
    else /* if ( PNDV_MAX_SV_ENTITY >= local_sub_res->entity ) */
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, local_sub_res->entity);
    }
}

/**
 *  @brief brief_description
 *
 *  @param[in]  usi type of diagnosis
 *  @param[in]  entity_nr number of the entity equals the real config entity nr
 *  @param[in]  kanal channel number
 *  @param[in]  fehlernummer error number
 *  @param[in]  properties properties
 *  @param[in]  req request type
 *
 *
 *
 *
 */
PNIO_VOID pndv_in_al_diag_req(PNDV_AL_USI_T            usi,
                              PNIO_UINT16              entity_nr,
                              PNIO_UINT16              kanal,
                              PNIO_UINT16              fehlernummer,
                              PNIO_UINT16              properties,
                              PNIO_UINT16              ext_fehler,
                              PNIO_UINT32              ext_wert,
                              PNDV_CHANNEL_DIAG_REQ_T  req )
{
    if ((usi == PNDV_AL_USI_CHANNELDIAG) || (usi == PNDV_AL_USI_EXTCHANNELDIAG ))
    {
        //channel diag
        PNDV_REAL_CFG_T *       real_cfg_channel_ptr;
        PNDV_CHAN_DIAG_PTR      channel_diag_ptr;
        PNIO_UINT16             slot;
        PNIO_UINT16             subslot;
        PNIO_UINT16             soll_slot;

        real_cfg_channel_ptr    = &pndv_data.iface_ptr->real_cfg[entity_nr];

        slot        = real_cfg_channel_ptr->elem.sub_descr.slot_nr;
        subslot     = real_cfg_channel_ptr->elem.sub_descr.subslot_nr;

        soll_slot = slot; /* default 1:1 Mapping */

        if ( PNDV_LIST_IS_EMPTY( &pndv_data.diag.channel_diag_free_list ) )
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }
        /* get one from the free list */
        PNDV_LIST_REMOVE_HEAD( &pndv_data.diag.channel_diag_free_list, channel_diag_ptr, PNDV_CHAN_DIAG_PTR);

        channel_diag_ptr->req   = req;

        /* Whether there is an alarm or not is only decided in pndv_in_al_dial_send, 
           so the acknowledgment control in pndv_in_al_check_diag is correct because the same channel diagnosis 
           can run through the latter function several times, 
           if the ... DAL .. bit is not set, the diagnosis is made with every run acknowledged, that's too much */

        /* set diag and Dial */
        channel_diag_ptr->ch_diag_sm           = PNDV_DIAG_CH_STATE_REQ_AL;

        /* ATTENTION: The PNDV_DIAG_DIAL_REQ_BIT bit must never be set alone
           -> no processing was done */

        channel_diag_ptr->usi                 = usi;
        channel_diag_ptr->entity_nr           = entity_nr;
        channel_diag_ptr->ist_slot            = slot;
        channel_diag_ptr->slot                = soll_slot;
        channel_diag_ptr->subslot             = subslot;
        channel_diag_ptr->as.ch.kanal         = kanal;
        channel_diag_ptr->as.ch.fehlernummer  = fehlernummer;
        channel_diag_ptr->as.ch.properties    = properties;
        channel_diag_ptr->as.ch.quit_to_peri  = (PNDV_CHANNEL_DIAG_RMV_ALL_REQ_PNDV == req) ? PNIO_FALSE : PNIO_TRUE;

        if (usi == PNDV_AL_USI_EXTCHANNELDIAG)
        {
            channel_diag_ptr->as.xch.error_type   = ext_fehler;
            channel_diag_ptr->as.xch.add_value    = ext_wert;
        }

        #ifdef PNDV_CFG_DEBUG_ENABLE

        switch ( req )
        {
            case PNDV_CHANNEL_DIAG_ADD_REQ:
            {
                pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_ADD_CHANNEL_DIAG_REQ, entity_nr, ((PNIO_UINT32)( (((PNIO_UINT32)kanal) << 16) | ((PNIO_UINT32)fehlernummer) ) ));
                break;
            }
            case PNDV_CHANNEL_DIAG_RMV_REQ:
            {
                pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_RMV_CHANNEL_DIAG_REQ, entity_nr, ((PNIO_UINT32)( (((PNIO_UINT32)kanal) << 16) | ((PNIO_UINT32)fehlernummer) ) ));
                break;
            }
            case PNDV_CHANNEL_DIAG_RMV_ALL_REQ:
            {
                pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_RMV_CHANNEL_DIAG_ALL_REQ, entity_nr, ((PNIO_UINT32)( (((PNIO_UINT32)kanal) << 16) | ((PNIO_UINT32)fehlernummer) ) ));
                break;
            }
            case PNDV_CHANNEL_DIAG_RMV_ALL_REQ_PNDV:
            {
                pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_RMV_CHANNEL_DIAG_ALL_REQ_PNDV, entity_nr, ((PNIO_UINT32)( (((PNIO_UINT32)kanal) << 16) | ((PNIO_UINT32)fehlernummer) ) ));
                break;
            }
            case PNDV_EXT_CHANNEL_DIAG_ADD_REQ:
            {
                pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_ADD_EXT_CHANNEL_DIAG_REQ, entity_nr, ((PNIO_UINT32)( (((PNIO_UINT32)kanal) << 16) | ((PNIO_UINT32)fehlernummer) ) ));
                break;
            }
            case PNDV_EXT_CHANNEL_DIAG_RMV_REQ:
            {
                pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_RMV_EXT_CHANNEL_DIAG_REQ, entity_nr, ((PNIO_UINT32)( (((PNIO_UINT32)kanal) << 16) | ((PNIO_UINT32)fehlernummer) ) ));
                break;
            }
            case PNDV_EXT_CHANNEL_DIAG_RMV_ALL_REQ:
            {
                pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_RMV_EXT_CHANNEL_DIAG_ALL_REQ, entity_nr, ((PNIO_UINT32)( (((PNIO_UINT32)kanal) << 16) | ((PNIO_UINT32)fehlernummer) ) ));
                break;
            }
            case PNDV_EXT_CHANNEL_DIAG_RMV_ALL_REQ_PNDV:
            {
                pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_RMV_EXT_CHANNEL_DIAG_ALL_REQ_PNDV, entity_nr, ((PNIO_UINT32)( (((PNIO_UINT32)kanal) << 16) | ((PNIO_UINT32)fehlernummer) ) ));
                break;
            }
            default:
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, req);
                break;
            }
        }
        #endif
        PNDV_LIST_INSERT_TAIL( &pndv_data.diag.channel_diag_new_list, &channel_diag_ptr->link);
    }
    else
    {
        //generic diag
        PNDV_CHAN_DIAG_PTR   channel_diag_ptr;
        PNIO_UINT16          slot;
        PNIO_UINT16          subslot;
        PNIO_UINT16          soll_slot;
        PNDV_GENERIC_DIAG_T* generic_diag;
        PNIO_UINT16          interface_index = entity_nr;

        generic_diag = &pndv_data.iface_ptr->generic_diag_data[entity_nr];

        slot        = generic_diag->slot;
        subslot     = generic_diag->subslot;

        soll_slot = slot; /* default 1:1 Mapping */

        if ( PNDV_LIST_IS_EMPTY( &pndv_data.diag.channel_diag_free_list ) )
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }
        /* get one from the free list */
        PNDV_LIST_REMOVE_HEAD( &pndv_data.diag.channel_diag_free_list, channel_diag_ptr, PNDV_CHAN_DIAG_PTR);

        channel_diag_ptr->req   = req;

        /* Whether there is an alarm or not is only decided in pndv_in_al_dial_send, 
           so the acknowledgment control in pndv_in_al_check_diag is correct because the same channel diagnosis 
           can run through the latter function several times, if the ... DAL .. bit is not set, 
           the diagnosis is made with every run acknowledged, that's too much */

        /* set diag and Dial */
        channel_diag_ptr->ch_diag_sm            = PNDV_DIAG_CH_STATE_REQ_AL;
        channel_diag_ptr->as.ch.quit_to_peri    = PNIO_TRUE;

        /* ATTENTION: The PNDV_DIAG_DIAL_REQ_BIT bit must never be set alone
                    -> no processing was done */

        channel_diag_ptr->usi                 = usi;
        channel_diag_ptr->entity_nr           = entity_nr;
        channel_diag_ptr->ist_slot            = slot;
        channel_diag_ptr->slot                = soll_slot;
        channel_diag_ptr->subslot             = subslot;
        //store the interface index of the generic diag data within the diag data
        channel_diag_ptr->as.ch.kanal         = interface_index;
        channel_diag_ptr->as.ch.fehlernummer  = fehlernummer;
        channel_diag_ptr->as.ch.properties    = properties;

        #ifdef PNDV_CFG_DEBUG_ENABLE

        switch ( req )
        {
            case PNDV_GENERIC_DIAG_ADD_REQ:
            {
                pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_ADD_GENERIC_DIAG_REQ, interface_index, 0);
                break;
            }
            case PNDV_GENERIC_DIAG_RMV_REQ:
            {
                pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_RMV_GENERIC_DIAG_REQ, interface_index, 0);
                break;
            }
            default:
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, req);
                break;
            }
        }
        #endif
        PNDV_LIST_INSERT_TAIL( &pndv_data.diag.channel_diag_new_list, &channel_diag_ptr->link);
    }

    /* After entering it in the list, try to stop the diagnosis immediately. 
       This must be entered in any case, since the diagnostic alarm is triggered asynchronously 
       after diag (add, remove) _done */

    pndv_in_al_check_diag();
}
//@}

//@name Internal
//@{
/**
 *  @brief application has reported all alarms and want to know when this alarms are at CM
 *
 *
 */
PNIO_VOID pndv_al_peri_alarm_reported_after_swo(PNIO_VOID)
{
    // get last prt from diag queu

    if(!(pndv_data.diag.channel_diag_new_list_bookmark = PNDV_LIST_LAST(&pndv_data.diag.channel_diag_new_list, PNDV_CHAN_DIAG_PTR)))
    {
        // diag queu empty -> done
        pndv_al_peri_alarm_reported_after_swo_done();
    }
}
//@}


//@name Internal
//@{
/**
 *  @brief application has reported all alarms and want to know when this alarms are at CM
 *
 *
 */
PNIO_VOID pndv_al_peri_alarm_reported_after_swo_done(PNIO_VOID)
{
    // get last prt from diag queu

    PNDV_IFACE_CMD_ENTRY_T  tmp_event;

    tmp_event.add_1 = 0;
    tmp_event.add_2 = 0;
    tmp_event.cmd = PNDV_EV_TO_PERI_SR_ALARM_REPORTED_AFTER_SWO_DONE;

    pndv_in_peri_write_coupl_event( tmp_event );
}
//@}


//@name Internal
//@{
/**
 *  @brief check for waiting diag requests to lower interface (cm)
 *
 *
 */
PNIO_VOID pndv_in_al_check_diag (PNIO_VOID)
{
    PNDV_CHAN_DIAG_PTR  channel_diag_ptr;
    PNIO_UINT32         while_done;

    while_done = PNIO_FALSE;

    while (PNIO_FALSE == while_done)
    {
        if (!PNDV_LIST_IS_EMPTY( &pndv_data.diag.channel_diag_new_list ) )
        {
            /* es ist ein Auftrag in der Liste */

            channel_diag_ptr = PNDV_LIST_FIRST( &pndv_data.diag.channel_diag_new_list, PNDV_CHAN_DIAG_PTR);

            /* send chnannel_diag */
            /* ----------------------------------------------------------------- */

            if ( (channel_diag_ptr != NULL) && (PNDV_DIAG_CH_STATE_REQ_AL == channel_diag_ptr->ch_diag_sm) )
            {
                PNIO_UINT16 ret_value;

                switch ( ret_value = pndv_in_al_diag_branch( channel_diag_ptr)  )
                {
                    case PNDV_ERR_RESOURCE:
                    {
                        /* diag data to big */
                    }
                    /* no break */
                    //lint -fallthrough
                    case PNDV_OK:
                    {
                        /* diag is not discontinued because module is not there 
                           or wrong the whole (diag and alarm) we can forget*/

                        if (PNIO_TRUE == channel_diag_ptr->as.ch.quit_to_peri)
                        {
                            if ( PNDV_AL_USI_CHANNELDIAG == channel_diag_ptr->usi )
                            {
                                /* ChannelDiagnosis */
                                pndv_in_peri_dial_quit(channel_diag_ptr->entity_nr, PNDV_TO_PERI_UNREPORTED);
                            }
                            else if ( PNDV_AL_USI_EXTCHANNELDIAG == channel_diag_ptr->usi )
                            {
                                /* ExtChannelDiagnosis */
                                pndv_in_peri_xdial_quit(channel_diag_ptr->entity_nr, PNDV_TO_PERI_UNREPORTED);
                            }
                            else
                            {
                                /* GenericDiagnosis */
                                pndv_in_peri_generic_dial_quit(channel_diag_ptr->as.ch.kanal, PNDV_TO_PERI_UNREPORTED);
                            }
                        }

                        channel_diag_ptr->ch_diag_sm = PNDV_DIAG_CH_STATE_FREE;

                        break;
                    }

                    case PNDV_OK_ASYNC:
                    {
                        /* diag discontinued
                           wait for diag done and then send alarm */

                        if ( PNDV_DIAG_CH_STATE_REQ_AL == channel_diag_ptr->ch_diag_sm )
                        {
                            channel_diag_ptr->ch_diag_sm = PNDV_DIAG_CH_STATE_W_DIAG_DONE_AL;
                        }
                        else
                        {
                            pndv_in_fatal_error( PNDV_MODULE, __LINE__, channel_diag_ptr->ch_diag_sm);
                        }

                        while_done = PNIO_TRUE;

                        break;
                    }

                    case PNDV_ERR_SEQUENCE:
                    {
                        /* is currently not working (interface occupied or module still in the plugging phase)
                           -> try later*/

                        while_done = PNIO_TRUE;

                        break;
                    }

                    default:
                    {
                        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ret_value);

                        break;
                    }
                }
            }
            else
            {
                /* found block = PNDV_DIAG_CH_STATE_W_DIAG_DONE_AL
                   -> in progress -> cancel loop */
                while_done = PNIO_TRUE;
            }


            /* kill if ready */
            /* ----------------------------------------------------------------- */

            if ( (channel_diag_ptr != NULL) && (PNDV_DIAG_CH_STATE_FREE == channel_diag_ptr->ch_diag_sm) )
            {
                /* ready -> del req from list */

                PNDV_LIST_REMOVE_ENTRY( &channel_diag_ptr->link);

                if(pndv_data.diag.channel_diag_new_list_bookmark == channel_diag_ptr)
                {
                    pndv_data.diag.channel_diag_new_list_bookmark = 0;
                    // the intresting block was removed from diag que -> informe application about that
                    pndv_al_peri_alarm_reported_after_swo_done();
                }

                PNDV_LIST_INSERT_TAIL( &pndv_data.diag.channel_diag_free_list, &channel_diag_ptr->link);
            }
        }
        else
        {
            while_done = PNIO_TRUE;
        }
    } // end while
}
//@}

//@name Interface
//@{
/**
 *  @brief check if there are queued alarms to be send
 *
 *
 */
PNIO_VOID pndv_in_al_check_dial_continue (PNIO_VOID)
{
    PNDV_CHAN_DIAG_PTR  channel_diag_ptr;
    PNIO_UINT32         quit_req,
                        next_diag_req;
    PNIO_UINT16         entity_nr;
    PNIO_UINT16         tmp_ret_val;

    quit_req        = PNIO_FALSE;
    next_diag_req   = PNIO_FALSE;

    if ( !PNDV_LIST_IS_EMPTY( &pndv_data.diag.channel_diag_new_list ) )
    {
        /* it is an order in the list */

        channel_diag_ptr = PNDV_LIST_FIRST( &pndv_data.diag.channel_diag_new_list, PNDV_CHAN_DIAG_PTR);

        PNDV_ASSERT(channel_diag_ptr != LSA_NULL);

        entity_nr = channel_diag_ptr->entity_nr;

        /* send dial */
        /* ----------------------------------------------------------------- */

        if ( PNDV_DIAG_CH_STATE_DIAG_DONE == channel_diag_ptr->ch_diag_sm )
        {
            /* Only send the alarm when diag_add (rmv) to the CM has been closed. 
               So far there was a gap 
               if this function was available before diag_add (rmv) _done the alarm was sent 
               because it was the first in the list */

            switch ( tmp_ret_val = pndv_in_al_dial_send( channel_diag_ptr) )
            {
                case PNDV_OK:

                    /* Alarm abolish is not necessary */

                    if (PNIO_TRUE == channel_diag_ptr->as.ch.quit_to_peri)
                    {
                        quit_req = PNIO_TRUE;
                    }

                    channel_diag_ptr->ch_diag_sm = PNDV_DIAG_CH_STATE_FREE;

                    /* Processing of the diagnosis ends here - continue below with the next one */
                    next_diag_req = PNIO_TRUE;

                    /* if it was a wdh - reset here */
                    pndv_data.diag.dial_repeat_req[entity_nr] = PNIO_FALSE;

                    break;

                case PNDV_OK_ASYNC:

                    /* The alarm has been cleared -> wait for acknowledgment*/

                    channel_diag_ptr->ch_diag_sm = PNDV_DIAG_CH_STATE_FREE;

                    /* Processing of the diagnosis ends here - continue below with the next one */
                    next_diag_req   = PNIO_TRUE;

                    /* if it was a wdh - reset here */
                    pndv_data.diag.dial_repeat_req[entity_nr] = PNIO_FALSE;

                    break;

                case PNDV_ERR_SEQUENCE:

                    /* req remains because the diagnostic alarm block is currently occupied for this subslot
                       ( PNDV_AL_FREE != pndv_data.al.modul[slot_nr].dial_state)

                       -> wdh in pndv_in_al_al_ack */

                    pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_DIAL_REPEAT_REQ, entity_nr, tmp_ret_val);

                    pndv_data.diag.dial_repeat_req[entity_nr] = PNIO_TRUE;

                    break;
                default:
                    break;
            }
        }

        /* kill if ready */
        /* ----------------------------------------------------------------- */

        if ( PNDV_DIAG_CH_STATE_FREE == channel_diag_ptr->ch_diag_sm )
        {
            /* ready -> del req from list */

            PNDV_LIST_REMOVE_ENTRY( &channel_diag_ptr->link);

            if(pndv_data.diag.channel_diag_new_list_bookmark == channel_diag_ptr)
            {
                pndv_data.diag.channel_diag_new_list_bookmark = 0;
                // the intresting block was removed from diag que -> informe application about that
                pndv_al_peri_alarm_reported_after_swo_done();
            }

            PNDV_LIST_INSERT_TAIL( &pndv_data.diag.channel_diag_free_list, &channel_diag_ptr->link);
        }

        if (PNIO_TRUE == quit_req)
        {
            if ( PNDV_AL_USI_CHANNELDIAG == channel_diag_ptr->usi )
            {
                /* ChannelDiagnosis */
                pndv_in_peri_dial_quit(channel_diag_ptr->entity_nr, PNDV_TO_PERI_OK_NO_ALARM);
            }
            else if ( PNDV_AL_USI_EXTCHANNELDIAG == channel_diag_ptr->usi )
            {
                /* ExtChannelDiagnosis */
                pndv_in_peri_xdial_quit(channel_diag_ptr->entity_nr, PNDV_TO_PERI_OK_NO_ALARM);
            }
            else
            {
                /* GenericDiagnosis */
                pndv_in_peri_generic_dial_quit(channel_diag_ptr->as.ch.kanal, PNDV_TO_PERI_OK_NO_ALARM);
            }
        }

        if (PNIO_TRUE == next_diag_req)
        {
            pndv_in_al_check_diag();
        }
    }
}

/**
 *  @brief check if there are queued return of submodul alarms to be send
 *
 *
 */
PNIO_VOID pndv_in_al_check_rosal_queue(PNIO_VOID)
{
    if ( !PNDV_LIST_IS_EMPTY( &pndv_data.al.q_rosal.in_use ) )
    {
        /* there are queued rosal pending*/

        PNDV_PERI_ALARM_ELEMENT_PTR_T local_list_elem_ptr;
        PNIO_UINT16                   entity_nr;
        /* The alarm cannot be triggered
           -> acknowledge immediately */

        local_list_elem_ptr = PNDV_LIST_FIRST(&pndv_data.al.q_rosal.in_use, PNDV_PERI_ALARM_ELEMENT_PTR_T);

        if (local_list_elem_ptr != LSA_NULL)
        {
            PNDV_LIST_REMOVE_ENTRY(&local_list_elem_ptr->link);

            entity_nr = local_list_elem_ptr->entity_nr;

            PNDV_LIST_INSERT_TAIL(&pndv_data.al.q_rosal.free_list, &local_list_elem_ptr->link);

            pndv_al_peri_rosal_req(entity_nr);
        }
    }
}

/**
 *  @brief search the rosal queue for a request, remove and quit
 *
 *  @param[in]     slot    slot number of the module
 *  @param[in]  subslot subslot number of the module
 *  @return        true = request removed, false = no request found
 *
 *  Search the in_use list of the rosal queue for a request of the
 *  module addressed by the given slot and subslot value.
 *  If one or more requests are found, remove them from the in_use list
 *  and quit the request to peri if applicable.
 *
 */
PNIO_UINT32 pndv_al_search_clear_rosal(PNIO_UINT16 slot, PNIO_UINT16 subslot)
{
    PNIO_UINT32                     ret_val;
    PNDV_PERI_ALARM_ELEMENT_PTR_T   local_list_elem_ptr, local_list_next_elem_ptr;
    PNIO_UINT16                     local_slot, local_subslot, entity_nr;

    ret_val = PNIO_FALSE;

    if ( !PNDV_LIST_IS_EMPTY( &pndv_data.al.q_rosal.in_use ) )
    {
        local_list_elem_ptr = PNDV_LIST_FIRST(&pndv_data.al.q_rosal.in_use, PNDV_PERI_ALARM_ELEMENT_PTR_T);

        while (local_list_elem_ptr != LSA_NULL)
        {
            local_list_next_elem_ptr = PNDV_LIST_NEXT(&pndv_data.al.q_rosal.in_use, &local_list_elem_ptr->link ,PNDV_PERI_ALARM_ELEMENT_PTR_T);

            entity_nr = local_list_elem_ptr->entity_nr;

            local_slot     = pndv_data.iface_ptr->real_cfg[entity_nr].elem.sub_descr.slot_nr;
            local_subslot  = pndv_data.iface_ptr->real_cfg[entity_nr].elem.sub_descr.subslot_nr;

            if (   (slot == local_slot)
                 &&(subslot== local_subslot)
                )
            {
                /* found a request for the subslot */

                /* remove entry from in_use list */
                PNDV_LIST_REMOVE_ENTRY(&local_list_elem_ptr->link)
                /* put resource to free list */
                PNDV_LIST_INSERT_TAIL(&pndv_data.al.q_rosal.free_list, &local_list_elem_ptr->link);

                pndv_in_peri_ros_al_quit(entity_nr);

                ret_val = PNIO_TRUE;
            }

            local_list_elem_ptr = local_list_next_elem_ptr;
        }
    }
    else
    {
        /* queue is empty nothing to be removed */
        // ret_val = PNIO_FALSE;
    }

    return ret_val;
}

/**
 *  @brief clear unreported diagnosis
 *
 *  @param[in]  slot slot number
 *  @param[in]  subslot subslot number
 *  @param[in]  req request type
 *  @return     returns: PNIO_FALSE = nothing to remove, PNIO_TRUE = one or more
 *              diagnosis removed
 *
 *  This function searches for diagnosis requests that have not yet been send
 *  to cm.
 *  This can be usefull if a submodule has been removed.
 *
 */
PNIO_UINT32 pndv_in_al_search_clear_diag(PNIO_UINT16 slot, PNIO_UINT16 subslot, PNDV_CHANNEL_DIAG_REQ_T req)
{

    PNDV_CHAN_DIAG_PTR  channel_diag_ptr;
    PNDV_CHAN_DIAG_PTR  channel_diag_ptr_next;
    PNIO_UINT32         req_delete;

    req_delete = PNIO_FALSE;

    /* Diagnoses from slot, subslot that have not yet been sent are deleted */


    channel_diag_ptr = PNDV_LIST_FIRST( &pndv_data.diag.channel_diag_new_list, PNDV_CHAN_DIAG_PTR);

    while (LSA_NULL != channel_diag_ptr)
    {
        /* Get the next block beforehand, since the macro (PNDV_LIST_NEXT) returns 
           the next block from the free_List after removing the current block and adding it to free_List 
           and never arrives at the anchor of the new_list -> endless loop */

        channel_diag_ptr_next = PNDV_LIST_NEXT(&pndv_data.diag.channel_diag_new_list, &channel_diag_ptr->link, PNDV_CHAN_DIAG_PTR);

        if (   (   (slot    == channel_diag_ptr->slot )
                && (subslot == channel_diag_ptr->subslot)
               )
            && (   (req     == PNDV_DIAG_REQ_NISCHT)    /* Delete entry req independently */
                || (req     == channel_diag_ptr->req)   /* Only delete entry if req is correct */
               )
           )
        {
            /* Clear */

            /* Prevent duplicate releases */

            channel_diag_ptr->ch_diag_sm = PNDV_DIAG_CH_STATE_FREE;

            if(pndv_data.diag.channel_diag_new_list_bookmark == channel_diag_ptr)
            {
                // the interesting block should be deleted from the middle of the queue 
                // if it is not the first in the queue, note the one in front of it and inform about 
                // the subsequent notification of all alarms when the application is removed

                if (channel_diag_ptr == PNDV_LIST_FIRST( &pndv_data.diag.channel_diag_new_list, PNDV_CHAN_DIAG_PTR))
                {
                    // block to be removed is the 1st in the list 
                    // all previous ones are processed

                    pndv_data.diag.channel_diag_new_list_bookmark = 0;
                    // the intresting block was removed from diag queu -> informe application about that
                    pndv_al_peri_alarm_reported_after_swo_done();
                }
                else
                {
                    // Remember the previous one and when it is removed, 
                    // inform about the subsequent reporting of all alarms

                    pndv_data.diag.channel_diag_new_list_bookmark = PNDV_LIST_PREV_OR_HEAD( &channel_diag_ptr->link, PNDV_CHAN_DIAG_PTR);
                }
            }

            PNDV_LIST_REMOVE_ENTRY( &channel_diag_ptr->link);
            PNDV_LIST_INSERT_TAIL( &pndv_data.diag.channel_diag_free_list, &channel_diag_ptr->link);

            /* as long as the block is still in the new list, no DIAL has been sent
               -> acknowledge here */

            if (PNIO_TRUE == channel_diag_ptr->as.ch.quit_to_peri)
            {
                if ( PNDV_AL_USI_CHANNELDIAG == channel_diag_ptr->usi )
                {
                    /* ChannelDiagnosis */
                    pndv_in_peri_dial_quit(channel_diag_ptr->entity_nr, PNDV_TO_PERI_UNREPORTED);
                }
                else if ( PNDV_AL_USI_EXTCHANNELDIAG == channel_diag_ptr->usi )
                {
                    /* ExtChannelDiagnosis */
                    pndv_in_peri_xdial_quit(channel_diag_ptr->entity_nr, PNDV_TO_PERI_UNREPORTED);
                }
                else
                {
                    /* GenericDiagnosis */
                    pndv_in_peri_generic_dial_quit(channel_diag_ptr->as.ch.kanal, PNDV_TO_PERI_UNREPORTED);
                }
            }
            req_delete = PNIO_TRUE;
        }

        /* make next block the current one */

        channel_diag_ptr = channel_diag_ptr_next;
    }

    return( req_delete);
}
//@}

//! @name Internal
//@{
/**
 *  @brief Split up requests in channel and generic diagnosis
 *
 *  @param[in]  diag_channel_ptr pointer to a diagnosis request
 *  @return     pass the response of called functions
 *
 *
 */
PNIO_UINT16 pndv_in_al_diag_branch(PNDV_CHAN_DIAG_PTR diag_channel_ptr)
{

    PNIO_UINT16 ret_value = PNDV_ERR_SEQUENCE;

    /* Depending on the request code - ChannelDiagnosis or GenericDiagnosis */

    switch ( diag_channel_ptr->req )
    {
        case PNDV_CHANNEL_DIAG_ADD_REQ:
        case PNDV_CHANNEL_DIAG_RMV_REQ:
        case PNDV_CHANNEL_DIAG_RMV_ALL_REQ:
        case PNDV_CHANNEL_DIAG_RMV_ALL_REQ_PNDV:
        case PNDV_EXT_CHANNEL_DIAG_ADD_REQ:
        case PNDV_EXT_CHANNEL_DIAG_RMV_REQ:
        case PNDV_EXT_CHANNEL_DIAG_RMV_ALL_REQ:
        case PNDV_EXT_CHANNEL_DIAG_RMV_ALL_REQ_PNDV:
        {
            /* ChannelDiagnosis */

            ret_value = pndv_in_al_channel_diag( diag_channel_ptr);

            break;
        }

        case PNDV_GENERIC_DIAG_ADD_REQ:
        case PNDV_GENERIC_DIAG_RMV_REQ:
        {
            /* GenericDiagnosis */

            ret_value = pndv_in_al_generic_diag( diag_channel_ptr);

            break;
        }

        default:
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, diag_channel_ptr->req);  /*NOTREACHED*/
        }
    }

    return(ret_value);
}

/**
 *  @brief handle a channel diagnosis request
 *
 *  @param[in]  diag_channel_ptr pointer to channel diag request
 *  @return     PNDV_OK_ASYNC = request is handled asynchronous,
 *              PNDV_OK = request is handled synchronous,
 *              PNDV_ERR_SEQUENCE = request can not be handled
 *
 *
 */
PNIO_UINT16 pndv_in_al_channel_diag (PNDV_CHAN_DIAG_PTR diag_channel_ptr)
{

    CM_UPPER_SV_DIAG_ADD_PTR_TYPE       diag_channel_add;
#if (defined PNDV_CFG_USE_DIAG2) || (defined  PNDV_CFG_USE_MULTIDIAG)
    CM_UPPER_SV_DIAG_ADD_PTR_TYPE       diag_channel_rmv_key;
#else
    CM_UPPER_SV_DIAG_REMOVE_PTR_TYPE    diag_channel_rmv;
#endif
    CM_UPPER_RQB_PTR_TYPE   tmp_rqb_ptr;
    PNIO_UINT16             ret_value;
    PNIO_UINT16             device_nr;
    PNIO_UINT16             slot_nr;
    PNIO_UINT16             subslot_nr,
                            entity_nr;
    PNIO_UINT32             subslot_error;
    PNIO_UINT32             diag_tag = 0;
    PNIO_UINT32             elem_nr;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    device_nr    = PNDV_CM_DEVICE_NO;

    /* 1. Perislot (PM) has the identifier 0 for the USER */
    slot_nr     = diag_channel_ptr->slot;
    subslot_nr  = diag_channel_ptr->subslot;
    entity_nr   = diag_channel_ptr->entity_nr;
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

    if (local_sub_res->entity != entity_nr)
    {
        //! ERROR! the entity of this submodule has changed since it was added
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, local_sub_res->entity );
    }

    /* only relevant for PNDV_CHANNEL_DIAG_ADD_REQ, PNDV_CHANNEL_DIAG_RMV_REQ */
    /* added: consideration high-byte of the channel because of possible ChannelNumber 0x8000 */
#if (defined PNDV_CFG_USE_DIAG2) || (defined  PNDV_CFG_USE_MULTIDIAG)
    diag_tag = 1; /* always for ADD2 1 */
#else /* PNDV_CFG_USE_DIAG2 */
#ifndef PNDV_CFG_USE_USERTAG
    diag_tag = ((((PNIO_UINT32) diag_channel_ptr->as.ch.kanal ) << 16) |
                 ((PNIO_UINT32) diag_channel_ptr->as.ch.fehlernummer)
               )
               + 1; /* never 0 */

#ifdef PNDV_CFG_AL_USE_DIRECTION_TO_CHANNEL_NR_PATCH
    if (0 == (0x7000 & diag_channel_ptr->as.ch.kanal))
    {
        /* Record direction with in diag_tag 
         * is in properties in bit 13-15 (0xE000 = CM_CHPR_DIR_MASK) 
         * and comes in the upper 16 bit at position 12-14 in the diag tag
         */

        diag_tag |= (((PNIO_UINT32)(diag_channel_ptr->as.ch.properties & CM_CHPR_DIR_MASK) >> 1) << 16);
    }
    else
    {
        /* Channel number is not supported (bits 12-14 must be 0) */
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.diag.ch_diag_to_cm_count[slot_nr]);
    }
#endif
#else
    //PNDV_REAL_CFG_T p_real_cfg = &pndv_data.iface_ptr.real_cfg[entity_nr];
    switch ( diag_channel_ptr->req )
    {
        case PNDV_CHANNEL_DIAG_ADD_REQ:
        case PNDV_CHANNEL_DIAG_RMV_REQ:
        case PNDV_CHANNEL_DIAG_RMV_ALL_REQ:
        case PNDV_CHANNEL_DIAG_RMV_ALL_REQ_PNDV:
        {

            diag_tag = pndv_data.iface_ptr->real_cfg[entity_nr].dial.chn_diag[0].diag_tag;
            break;
        }
        case PNDV_EXT_CHANNEL_DIAG_ADD_REQ:
        case PNDV_EXT_CHANNEL_DIAG_RMV_REQ:
        case PNDV_EXT_CHANNEL_DIAG_RMV_ALL_REQ:
        case PNDV_EXT_CHANNEL_DIAG_RMV_ALL_REQ_PNDV:
        {

            diag_tag = pndv_data.iface_ptr->real_cfg[entity_nr].xdial.ext_diag[0].diag_tag;
            break;
        }
        default:
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.diag.ch_diag_to_cm_count[entity_nr]);
            break;
        }
    }
#endif
#endif /* PNDV_CFG_USE_DIAG2 */


    if (PNIO_TRUE == pndv_al_diag_possible_within_state(local_sub_res, diag_channel_ptr->req))
    {
        if (PNIO_FALSE == pndv_data.diag.cm_diag_req_running)
        {
            /* only if no diagnoses are running

             sequence:
             1. Make diagnosis on CM
             2. Send diag alarm
             */

            if(  (PNDV_CFG_SUBMODULE_RES_STATE_W_OWN          == local_sub_res->res_state) // modul is not about to be plugged or removed and is waiting for an owner
               ||(PNDV_CFG_SUBMODULE_RES_STATE_W_OWN_PASSIV   == local_sub_res->res_state)
               ||(   (PNDV_CFG_SUBMODULE_RES_STATE_W_IN_DATA <= local_sub_res->res_state) // valid module + owning AR is in-data (->modul is not within parameter phase)
                   &&(pndv_al_check_ar_is_ready(local_sub_res->own_ar_idx))               // -> let diag pass
                  )
               ||(   (PNDV_CFG_SUBMODULE_RES_STATE_W_PRM_END == local_sub_res->res_state)       // module is wrong, stuck in state W_PRM_END
                   &&(PNDV_SUBMODULE_STATE_WRONG_MODUL       == local_sub_res->submodule_state) // and the owning AR is indata -> let diag pass as well
                   &&(pndv_al_check_ar_is_ready(local_sub_res->own_ar_idx))                     // (owning ar is in-data)
                 )
              )
            {
                ret_value = PNDV_OK_ASYNC;

                switch (diag_channel_ptr->req)
                {
                    /* ----------------------------------------------------- */

                    case PNDV_CHANNEL_DIAG_ADD_REQ:
                    case PNDV_EXT_CHANNEL_DIAG_ADD_REQ:
                    {
                        diag_channel_add = pndv_data.rqb.diag_add.args.sv.diag_add;

                        /* only single diagnostics for the CM, since associated alarms have no elem structure */
                        diag_channel_add->nr_of_elems = 1;
                        elem_nr = diag_channel_add->nr_of_elems - 1;

                        diag_channel_add->device_nr                             = device_nr;
                        diag_channel_add->elem[elem_nr].api                     = local_sub_res->sub_module.api;
                        diag_channel_add->elem[elem_nr].slot_nr                 = slot_nr;
                        diag_channel_add->elem[elem_nr].subslot_nr              = subslot_nr;
                        diag_channel_add->elem[elem_nr].diag_tag                = diag_tag;

                        if (PNDV_CHANNEL_DIAG_ADD_REQ == diag_channel_ptr->req)
                        {
                            diag_channel_add->elem[elem_nr].diag_type               = CM_SV_DIAG_TYPE_CHANNEL;
                            diag_channel_add->elem[elem_nr].channel_number          = diag_channel_ptr->as.ch.kanal;
                            diag_channel_add->elem[elem_nr].channel_properties      = diag_channel_ptr->as.ch.properties;
                            diag_channel_add->elem[elem_nr].u.ch.channel_error_type = diag_channel_ptr->as.ch.fehlernummer;
                            pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_ADD_CHANNEL_DIAG, entity_nr, ((PNIO_UINT32)diag_tag));
                        }
                        else // PNDV_EXT_CHANNEL_DIAG_ADD_REQ
                        {
                            diag_channel_add->elem[elem_nr].diag_type                    = CM_SV_DIAG_TYPE_EXT_CHANNEL;
                            diag_channel_add->elem[elem_nr].channel_number               = diag_channel_ptr->as.xch.kanal;
                            diag_channel_add->elem[elem_nr].channel_properties           = diag_channel_ptr->as.xch.properties;
                            diag_channel_add->elem[elem_nr].u.ext.channel_error_type     = diag_channel_ptr->as.xch.fehlernummer;
                            diag_channel_add->elem[elem_nr].u.ext.ext_channel_error_type = diag_channel_ptr->as.xch.error_type;
                            diag_channel_add->elem[elem_nr].u.ext.ext_channel_add_value  = diag_channel_ptr->as.xch.add_value;
                            pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_ADD_EXT_CHANNEL_DIAG, entity_nr, ((PNIO_UINT32)diag_tag));

                        }

                        PNDV_RQB_SET_HANDLE(&pndv_data.rqb.diag_add, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
#if (defined PNDV_CFG_USE_DIAG2) || (defined  PNDV_CFG_USE_MULTIDIAG)
                        PNDV_RQB_SET_OPCODE(&pndv_data.rqb.diag_add, CM_OPC_SV_DIAG_ADD2);
#else
                        PNDV_RQB_SET_OPCODE(&pndv_data.rqb.diag_add, CM_OPC_SV_DIAG_ADD);
#endif

                        tmp_rqb_ptr = &pndv_data.rqb.diag_add;

                        pndv_data.diag.cm_diag_req_running = PNIO_TRUE;

                        /* increment counter in case of coming diag to CM */
                        pndv_data.diag.ch_diag_to_cm_count[entity_nr]++;

                        PNDV_REQUEST(tmp_rqb_ptr, LSA_COMP_ID_CM);

                        break;
                    }

                    /* ----------------------------------------------------- */

                    case PNDV_CHANNEL_DIAG_RMV_REQ:
                    case PNDV_CHANNEL_DIAG_RMV_ALL_REQ:
                    case PNDV_CHANNEL_DIAG_RMV_ALL_REQ_PNDV:
                    case PNDV_EXT_CHANNEL_DIAG_RMV_REQ:
                    case PNDV_EXT_CHANNEL_DIAG_RMV_ALL_REQ:
                    case PNDV_EXT_CHANNEL_DIAG_RMV_ALL_REQ_PNDV:
                    {
                        #ifndef IM_SUPPORT_PN_RX_REDUNDANCY //dont check diag count if R1 Device, the count will not be linked between the IMs
                        if ( 0 < pndv_data.diag.ch_diag_to_cm_count[entity_nr] )
                        #endif
                        {
                            /* remove one or all channel_diag */

#if (defined PNDV_CFG_USE_DIAG2) || (defined  PNDV_CFG_USE_MULTIDIAG)
                            //diag_channel_rmv = pndv_data.rqb.diag_remove.args.sv.diag_remove;
                            diag_channel_rmv_key = pndv_data.rqb.diag_remove.args.sv.diag_add;

                            /* only single diagnostics for the CM, since associated alarms have no elem structure */
                            diag_channel_rmv_key->nr_of_elems = 1;
                            elem_nr = diag_channel_rmv_key->nr_of_elems - 1;

                            diag_channel_rmv_key->device_nr                 = device_nr;
                            diag_channel_rmv_key->elem[elem_nr].api        = local_sub_res->sub_module.api;
                            diag_channel_rmv_key->elem[elem_nr].slot_nr    = slot_nr;
                            diag_channel_rmv_key->elem[elem_nr].subslot_nr = subslot_nr;

                            if (    ( PNDV_CHANNEL_DIAG_RMV_ALL_REQ          == diag_channel_ptr->req )
                                ||  ( PNDV_CHANNEL_DIAG_RMV_ALL_REQ_PNDV     == diag_channel_ptr->req )
                                ||  ( PNDV_EXT_CHANNEL_DIAG_RMV_ALL_REQ      == diag_channel_ptr->req )
                                ||  ( PNDV_EXT_CHANNEL_DIAG_RMV_ALL_REQ_PNDV == diag_channel_ptr->req )  )
                            {
                                /* remove all (ext) channel_diag */

                                diag_channel_rmv_key->elem[elem_nr].diag_tag = 0;

                                /* set counter to 0 - all channel diag removed */
                                pndv_data.diag.ch_diag_to_cm_count[entity_nr] = 0;

                            }
                            else
                            {
                                /* remove one (ext) channel_diag */

                                diag_channel_rmv_key->elem[elem_nr].diag_tag = diag_tag;

                                if (PNDV_CHANNEL_DIAG_RMV_REQ == diag_channel_ptr->req)
                                {
                                    diag_channel_rmv_key->elem[elem_nr].diag_type               = CM_SV_DIAG_TYPE_CHANNEL;
                                    diag_channel_rmv_key->elem[elem_nr].channel_number          = diag_channel_ptr->as.ch.kanal;
                                    diag_channel_rmv_key->elem[elem_nr].channel_properties      = diag_channel_ptr->as.ch.properties;
                                    diag_channel_rmv_key->elem[elem_nr].u.ch.channel_error_type = diag_channel_ptr->as.ch.fehlernummer;
                                }
                                else
                                {
                                    //PNDV_EXT_CHANNEL_DIAG_RMV_REQ
                                    diag_channel_rmv_key->elem[elem_nr].diag_type               = CM_SV_DIAG_TYPE_EXT_CHANNEL;
                                    diag_channel_rmv_key->elem[elem_nr].channel_number          = diag_channel_ptr->as.ch.kanal;
                                    diag_channel_rmv_key->elem[elem_nr].channel_properties      = diag_channel_ptr->as.ch.properties;
                                    diag_channel_rmv_key->elem[elem_nr].u.ext.channel_error_type = diag_channel_ptr->as.ch.fehlernummer;
                                    diag_channel_rmv_key->elem[elem_nr].u.ext.ext_channel_error_type = diag_channel_ptr->as.xch.error_type;
                                }

                                /* decrement counter in case of going diag to CM */
                                pndv_data.diag.ch_diag_to_cm_count[entity_nr]--;

                            }

                            if (    ( PNDV_CHANNEL_DIAG_RMV_REQ              == diag_channel_ptr->req )
                                ||  ( PNDV_CHANNEL_DIAG_RMV_ALL_REQ          == diag_channel_ptr->req )
                                ||  ( PNDV_CHANNEL_DIAG_RMV_ALL_REQ_PNDV     == diag_channel_ptr->req ))
                            {
                                pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_RMV_CHANNEL_DIAG, entity_nr, ((PNIO_UINT32)diag_channel_rmv_key->elem[elem_nr].diag_tag));
                            }
                            else
                            {
                                pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_RMV_EXT_CHANNEL_DIAG, entity_nr, ((PNIO_UINT32)diag_channel_rmv_key->elem[elem_nr].diag_tag));
                            }

#else  /* (defined PNDV_CFG_USE_DIAG2)||(defined  PNDV_CFG_USE_MULTIDIAG) */

                            diag_channel_rmv = pndv_data.rqb.diag_remove.args.sv.diag_remove;

                            /* only single diagnostics for the CM, since associated alarms have no elem structure */
                            diag_channel_rmv->nr_of_elems = 1;
                            elem_nr = diag_channel_rmv->nr_of_elems - 1;

                            diag_channel_rmv->device_nr                 = device_nr;
                            diag_channel_rmv->elem[elem_nr].api        = local_sub_res->sub_module.api;
                            diag_channel_rmv->elem[elem_nr].slot_nr    = slot_nr;
                            diag_channel_rmv->elem[elem_nr].subslot_nr = subslot_nr;

                            if (    ( PNDV_CHANNEL_DIAG_RMV_ALL_REQ          == diag_channel_ptr->req )
                                ||  ( PNDV_CHANNEL_DIAG_RMV_ALL_REQ_PNDV     == diag_channel_ptr->req )
                                ||  ( PNDV_EXT_CHANNEL_DIAG_RMV_ALL_REQ      == diag_channel_ptr->req )
                                ||  ( PNDV_EXT_CHANNEL_DIAG_RMV_ALL_REQ_PNDV == diag_channel_ptr->req )  )
                            {
                                /* remove all (ext) channel_diag */

                                diag_channel_rmv->elem[elem_nr].diag_tag = 0;

                                /* set counter to 0 - all channel diag removed */
                                pndv_data.diag.ch_diag_to_cm_count[entity_nr] = 0;

                            }
                            else
                            {
                                /* remove one (ext) channel_diag */

                                diag_channel_rmv->elem[elem_nr].diag_tag = diag_tag;


                                /* decrement counter in case of going diag to CM */
                                pndv_data.diag.ch_diag_to_cm_count[entity_nr]--;

                            }

                            if (   (PNDV_CHANNEL_DIAG_RMV_REQ              == diag_channel_ptr->req)
                                || (PNDV_CHANNEL_DIAG_RMV_ALL_REQ          == diag_channel_ptr->req)
                                || (PNDV_CHANNEL_DIAG_RMV_ALL_REQ_PNDV     == diag_channel_ptr->req))
                            {
                                pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_RMV_CHANNEL_DIAG, entity_nr, ((PNIO_UINT32)diag_channel_rmv->elem[elem_nr].diag_tag));
                            }
                            else
                            {
                                pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_RMV_EXT_CHANNEL_DIAG, entity_nr, ((PNIO_UINT32)diag_channel_rmv->elem[elem_nr].diag_tag));
                            }
#endif    /* (defined PNDV_CFG_USE_DIAG2)||(defined  PNDV_CFG_USE_MULTIDIAG) */

                            PNDV_RQB_SET_HANDLE( &pndv_data.rqb.diag_remove, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
#if (defined PNDV_CFG_USE_DIAG2) || (defined  PNDV_CFG_USE_MULTIDIAG)
                            PNDV_RQB_SET_OPCODE( &pndv_data.rqb.diag_remove, CM_OPC_SV_DIAG_REMOVE2);
#else
                            PNDV_RQB_SET_OPCODE( &pndv_data.rqb.diag_remove, CM_OPC_SV_DIAG_REMOVE);
#endif

                            tmp_rqb_ptr = &pndv_data.rqb.diag_remove;
                            pndv_data.diag.cm_diag_req_running = PNIO_TRUE;

                            PNDV_REQUEST(tmp_rqb_ptr, LSA_COMP_ID_CM);
                        }
#ifndef IM_SUPPORT_PN_RX_REDUNDANCY //dont check diag count if R1 Device, the count will not be linked between the IMs
                        else
                        {
                            if (0 == pndv_data.diag.ch_diag_to_cm_count[entity_nr])
                            {
                                /* ignore diag - CM without added active diag */
                                ret_value = PNDV_OK;
                            }
                            else
                            {
                                /* wrong counter */
                                pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.diag.ch_diag_to_cm_count[entity_nr]);
                            }
                        }
#endif
                        break;
                    }

                    /* ----------------------------------------------------- */

                    default:
                    {
                        pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.diag.ch_diag_to_cm_count[entity_nr]);

                        break;
                    }
                }
            }
            else
            {
                /* The module is still plugged in until submodule_add_done */

                ret_value = PNDV_ERR_SEQUENCE;
            }
        }
        else
        {
            ret_value = PNDV_ERR_SEQUENCE;
        }
    }
    else
    {
        /* no valid module or module has parameterization errors -> ignore diag */

        ret_value = PNDV_OK;
    }

    return(ret_value);
}

/**
 *  @brief handle a generic diagnosis request
 *
 *  @param[in]  diag_channel_ptr pointer to channel diag request
 *  @return     PNDV_OK_ASYNC = request is handled asynchronous,
 *              PNDV_OK = request is handled synchronous,
 *              PNDV_ERR_SEQUENCE = request can not be handled
 *
 *
 */
PNIO_UINT16 pndv_in_al_generic_diag (PNDV_CHAN_DIAG_PTR diag_channel_ptr )
{

    CM_UPPER_SV_DIAG_ADD_PTR_TYPE       diag_generic_add;
#if (defined PNDV_CFG_USE_DIAG2)
    CM_UPPER_SV_DIAG_ADD_PTR_TYPE       diag_generic_rmv_key;
#else
    CM_UPPER_SV_DIAG_REMOVE_PTR_TYPE    diag_generic_rmv;
#endif

    CM_UPPER_RQB_PTR_TYPE               tmp_rqb_ptr;
    PNIO_UINT16                         ret_value;
    PNIO_UINT16                         device_nr;
    PNIO_UINT16                         slot_nr;
    PNIO_UINT16                         subslot_nr,
                                        interface_idx;
    PNIO_UINT32                         subslot_error;
    PNIO_UINT32                         elem_nr;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    device_nr    = PNDV_CM_DEVICE_NO;

    slot_nr     = diag_channel_ptr->slot;
    subslot_nr  = diag_channel_ptr->subslot;
    interface_idx = diag_channel_ptr->as.ch.kanal;

    pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
    if(subslot_error)
    {
        // slot/subslot not possible, must be an implementation error
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    if(local_sub_res == 0)
    {
        // submodule not plugged, must be an error here
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, (PNIO_UINT32)local_sub_res);
    }

    if (PNIO_TRUE == pndv_al_diag_possible_within_state(local_sub_res, diag_channel_ptr->req))
    {
        if ((PNIO_FALSE == pndv_data.diag.cm_diag_req_running)
            && (  (PNDV_CFG_SUBMODULE_RES_STATE_W_OWN          == local_sub_res->res_state)// modul is not about to be plugged or removed and is waiting for an owner
                ||(PNDV_CFG_SUBMODULE_RES_STATE_W_OWN_PASSIV   == local_sub_res->res_state)
                ||(   (PNDV_CFG_SUBMODULE_RES_STATE_W_IN_DATA <= local_sub_res->res_state) // valid module + owning AR is in-data (->modul is not within parameter phase)
                    &&(pndv_al_check_ar_is_ready(local_sub_res->own_ar_idx))               // -> let diag pass
                   )
                ||(   (PNDV_CFG_SUBMODULE_RES_STATE_W_PRM_END  == local_sub_res->res_state)       // module is wrong, stuck in state W_PRM_END
                    &&(PNDV_SUBMODULE_STATE_WRONG_MODUL        == local_sub_res->submodule_state) // and the owning AR is indata -> let diag pass as well
                    &&(pndv_al_check_ar_is_ready(local_sub_res->own_ar_idx))                      // (owning ar is in-data)
                   )
                )
            )
        {
            /* only if no diagnoses are running */
            if (pndv_data.iface_ptr->generic_diag_data[interface_idx].length <= PNDV_MAX_GENERIC_DIAG_DATA_LENGTH)
            {
                /* only if data fits the buffer */
                ret_value = PNDV_OK_ASYNC;

                switch ( diag_channel_ptr->req )
                {
                    case PNDV_GENERIC_DIAG_ADD_REQ:
                    {
                        PNDV_GENERIC_DIAG_T     *generic_diag;

                        generic_diag = &pndv_data.iface_ptr->generic_diag_data[interface_idx];

                        diag_generic_add = pndv_data.rqb.diag_add.args.sv.diag_add;

                        //set the entity number of the submodule issuing the generic diag
                        diag_channel_ptr->entity_nr = local_sub_res->entity;

                        /* only single diagnostics for the CM, since associated alarms have no elem structure */
                        diag_generic_add->nr_of_elems = 1;
                        elem_nr = diag_generic_add->nr_of_elems - 1;

                        diag_generic_add->device_nr                     = device_nr;
                        diag_generic_add->elem[elem_nr].api             = local_sub_res->sub_module.api;
                        diag_generic_add->elem[elem_nr].slot_nr         = slot_nr;
                        diag_generic_add->elem[elem_nr].subslot_nr      = subslot_nr;
#ifdef PNDV_CFG_USE_DIAG2
                        diag_generic_add->elem[elem_nr].diag_tag        = 1;
#else
#ifdef PNDV_CFG_USE_MULTIDIAG
                        if (generic_diag->diag_tag == 1)
                        {
                            /* diag_tag == 1 is not allowed for MULTIDIAG use*/
                            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
                        }
#endif
                        diag_generic_add->elem[elem_nr].diag_tag        = generic_diag->diag_tag;
#endif

                        diag_generic_add->elem[elem_nr].diag_type       = CM_SV_DIAG_TYPE_GENERIC;
                        diag_generic_add->elem[elem_nr].channel_number  = 0x8000;

                        diag_generic_add->elem[elem_nr].channel_properties = PNDV_SV_DIAG_CHANNEL_PROPERTIES_MAKE(0, /* type */
                                                                                                                  PNDV_AL_SPEC_KOMMEND, /* spec */
                                                                                                                  0 /* direction */);

                        diag_generic_add->elem[elem_nr].u.gen.info_tag      = generic_diag->usi;
                        diag_generic_add->elem[elem_nr].u.gen.info_length   = generic_diag->length;
                        diag_generic_add->elem[elem_nr].u.gen.info_data     = (CM_COMMON_MEM_U8_PTR_TYPE) generic_diag->data;

                        pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_ADD_GENERIC_DIAG, generic_diag->usi, slot_nr);

                        PNDV_RQB_SET_HANDLE(&pndv_data.rqb.diag_add, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
#ifdef PNDV_CFG_USE_DIAG2
                        PNDV_RQB_SET_OPCODE(&pndv_data.rqb.diag_add, CM_OPC_SV_DIAG_ADD2);
#else
                        PNDV_RQB_SET_OPCODE(&pndv_data.rqb.diag_add, CM_OPC_SV_DIAG_ADD);
#endif

                        tmp_rqb_ptr = &pndv_data.rqb.diag_add;

                        pndv_data.diag.cm_diag_req_running = PNIO_TRUE;

                        /* increment counter in case of coming diag to CM */
                        /* Since diag_tag is always the same, even if the diagnosis is repeated several times, only one is stored at CM */

                        PNDV_REQUEST(tmp_rqb_ptr, LSA_COMP_ID_CM);

                        break;
                    }

                    /* ----------------------------------------------------- */

                    case PNDV_GENERIC_DIAG_RMV_REQ:
                    {
                        PNDV_GENERIC_DIAG_T     *generic_diag;

                        generic_diag = &pndv_data.iface_ptr->generic_diag_data[interface_idx];

                        //set the entity number of the submodule issuing the generic diag
                        diag_channel_ptr->entity_nr = local_sub_res->entity;

                        pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_RMV_GENERIC_DIAG, generic_diag->usi, slot_nr );

                        PNDV_RQB_SET_HANDLE( &pndv_data.rqb.diag_remove, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);

                        /* remove generic_diag */



#ifdef PNDV_CFG_USE_DIAG2
                        diag_generic_rmv_key = pndv_data.rqb.diag_remove.args.sv.diag_add;

                        diag_generic_rmv_key->nr_of_elems = 1;
                        elem_nr = diag_generic_rmv_key->nr_of_elems - 1;

                        diag_generic_rmv_key->device_nr                = device_nr;
                        diag_generic_rmv_key->elem[elem_nr].api        = local_sub_res->sub_module.api;
                        diag_generic_rmv_key->elem[elem_nr].slot_nr    = slot_nr;
                        diag_generic_rmv_key->elem[elem_nr].subslot_nr = subslot_nr;

                        diag_generic_rmv_key = pndv_data.rqb.diag_remove.args.sv.diag_add;
                        diag_generic_rmv_key->elem[elem_nr].channel_properties =
                                PNDV_SV_DIAG_CHANNEL_PROPERTIES_MAKE
                                    ( 0, /* type */
                                      PNDV_AL_SPEC_KOMMEND, /* spec */
                                      0 /* direction */
                                    );

                        diag_generic_rmv_key->elem[elem_nr].u.gen.info_tag      = generic_diag->usi;
                        PNDV_RQB_SET_OPCODE( &pndv_data.rqb.diag_remove, CM_OPC_SV_DIAG_REMOVE2);
#else
                        diag_generic_rmv = pndv_data.rqb.diag_remove.args.sv.diag_remove;

                        /* only single diagnostics for the CM, since associated alarms have no elem structure */
                        diag_generic_rmv->nr_of_elems = 1;
                        elem_nr = diag_generic_rmv->nr_of_elems - 1;

                        diag_generic_rmv->device_nr                 = device_nr;
                        diag_generic_rmv->elem[elem_nr].api        = local_sub_res->sub_module.api;
                        diag_generic_rmv->elem[elem_nr].slot_nr    = slot_nr;
                        diag_generic_rmv->elem[elem_nr].subslot_nr = subslot_nr;
#ifdef PNDV_CFG_USE_MULTIDIAG
                        if (generic_diag->diag_tag == 1)
                        {
                            /* diag_tag == 1 is not allowed for MULTIDIAG use*/
                            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
                        }
#endif
                        diag_generic_rmv->elem[elem_nr].diag_tag   = generic_diag->diag_tag;

                        PNDV_RQB_SET_OPCODE(&pndv_data.rqb.diag_remove, CM_OPC_SV_DIAG_REMOVE);
#endif

                        tmp_rqb_ptr = &pndv_data.rqb.diag_remove;

                        pndv_data.diag.cm_diag_req_running = PNIO_TRUE;

                        PNDV_REQUEST(tmp_rqb_ptr, LSA_COMP_ID_CM);

                        break;
                    }

                    /* ----------------------------------------------------- */

                    default:
                    {
                        pndv_in_fatal_error(PNDV_MODULE, __LINE__, diag_channel_ptr->req);

                        break;
                    }
                }
            }
            else
            {
                /* diag data to big */
                ret_value = PNDV_ERR_RESOURCE;
            }
        }
        else
        {
            ret_value = PNDV_ERR_SEQUENCE;
        }
    }
    else
    {
        /* no valid module or module has parameterization errors -> ignore diag */

        ret_value = PNDV_OK;
    }

    return(ret_value);
}
//@}

//! @name Interface
//@{
/**
 *  @brief callback to a diagnosis add request to lower layer (cm)
 *
 *  @param[in]  rqb_ptr pointer to a request block
 *
 *  This is the callback function for the lower layer (cm) to handle all
 *  responses of the type CM_OPC_SV_DIAG_ADD/ADD2.
 *  Also see pndv_in_al_channel_diag() and pndv_in_al_generic_diag().
 *
 */
PNIO_VOID pndv_in_al_diag_add_done (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32                         response,
                                        subslot_error,
                                        elem_cnt;
    PNIO_UINT16                         slot_nr,
                                        subslot_nr;
    CM_UPPER_SV_DIAG_ADD_PTR_TYPE       diag_channel_add;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;


    diag_channel_add = rqb_ptr->args.sv.diag_add;

    if (  1
#if (defined PNDV_CFG_USE_DIAG2) || (defined  PNDV_CFG_USE_MULTIDIAG)
        &&( CM_OPC_SV_DIAG_ADD2 != PNDV_RQB_GET_OPCODE(rqb_ptr) )
#endif
#if !(defined PNDV_CFG_USE_DIAG2)
        &&( CM_OPC_SV_DIAG_ADD != PNDV_RQB_GET_OPCODE(rqb_ptr) )
#endif
        )
    {
        /* only confirmation of submodule diagnosis request leads into this function */
        pndv_in_fatal_error( PNDV_MODULE, __LINE__,  PNDV_RQB_GET_OPCODE(rqb_ptr) );
    }

    PNDV_RQB_SET_OPCODE( rqb_ptr, 0);

    response  = PNDV_RQB_GET_RESPONSE( rqb_ptr);

    elem_cnt = 0;

    while( elem_cnt < diag_channel_add->nr_of_elems )
    {
        /* loop over all elements */

        if ( 0 != diag_channel_add->elem[elem_cnt].diag_tag)
        {
            /* we get back a valid diag_day */

            /* get local_sub_res of the real config */
            slot_nr     = diag_channel_add->elem[elem_cnt].slot_nr;
            subslot_nr  = diag_channel_add->elem[elem_cnt].subslot_nr;

            pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
            if(subslot_error)
            {
                // slot/subslot not possible, must be an implementation error
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
            }

            if(local_sub_res != 0)
            {
                /* There is a submodule in the real config */

                switch (response)
                {
                    case CM_OK:
                    {
                        pndv_in_al_diag_done( local_sub_res->entity, PNIO_FALSE);

                        break;
                    }
                    case CM_OK_REPLACED:
                    {
                        pndv_in_al_diag_done( local_sub_res->entity, PNIO_FALSE);

                        break;
                    }

                    case CM_ERR_ELEM:
                    {
                        if ( !(    (CM_OK          == diag_channel_add->elem[elem_cnt].response)
                                || (CM_OK_REPLACED == diag_channel_add->elem[elem_cnt].response)
                              )
                           )
                        {
                            pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_DIAG_ADD_DONE_ERR_ELEM, slot_nr, subslot_nr );
                        }

                        pndv_in_fatal_error( PNDV_MODULE, __LINE__, local_sub_res->entity);

                        break;
                    }

                    case CM_ERR_SEQUENCE:
                    case CM_ERR_PARAM:
                    default:
                    {
                        /* this responses leads into an error */
                        pndv_in_fatal_error( PNDV_MODULE, __LINE__, response );

                        break;
                    }
                }
            }
            else
            {
                /* a fixed system tester has pulled the submodule again since the request  */
                /* the receipt is now done */
            }
        }
        else
        {
            /* that cannot be because the diag_tag must always be! = 0 in the request */
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, diag_channel_add->elem[elem_cnt].diag_tag );
        }

        elem_cnt++;
    }
}

/**
 *  @brief callback to a diagnosis remove request to lower layer (cm)
 *
 *  @param[in]  rqb_ptr pointer to a request block
 *
 *  This is the callback function for the lower layer (cm) to handle all
 *  responses of the type CM_OPC_SV_DIAG_REMOVE/CM_OPC_SV_DIAG_REMOVE2.
 *  Also see pndv_in_al_channel_diag() and pndv_in_al_generic_diag().
 *
 *  CM_OPC_SV_DIAG_REMOVE/CM_OPC_SV_DIAG_REMOVE2
 *
 */
PNIO_VOID pndv_in_al_diag_remove_done (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32                         response,
                                        subslot_error,
                                        elem_cnt;
    PNIO_UINT16                         slot_nr,
                                        subslot_nr;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    CM_UPPER_SV_DIAG_REMOVE_PTR_TYPE    diag_channel_remove;
    diag_channel_remove = rqb_ptr->args.sv.diag_remove;

    if ( CM_OPC_SV_DIAG_REMOVE != PNDV_RQB_GET_OPCODE(rqb_ptr) )
    {
        /* only confirmation of submodule diagnosis request leads into this function */
        pndv_in_fatal_error( PNDV_MODULE, __LINE__,  PNDV_RQB_GET_OPCODE(rqb_ptr) );
    }


    PNDV_RQB_SET_OPCODE( rqb_ptr, 0);

    response = PNDV_RQB_GET_RESPONSE( rqb_ptr);

    elem_cnt = 0;

    while( elem_cnt < diag_channel_remove->nr_of_elems )
    {
        /* loop over all elements */

        PNIO_UINT32 diag_tag;

        /* diag_tag can also be 0 -> 0 = remove all */
        diag_tag = diag_channel_remove->elem[elem_cnt].diag_tag;

        /* get local_sub_res of the real config */
        slot_nr     = diag_channel_remove->elem[elem_cnt].slot_nr;
        subslot_nr  = diag_channel_remove->elem[elem_cnt].subslot_nr;

        pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
        if(subslot_error)
        {
            // slot/subslot not possible, must be an implementation error
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }

        if(local_sub_res != 0)
        {
            /* There is a submodule in the real config */

            switch ( response )
            {
                case CM_OK:
                {
                    /* maybe GenericDiagnosis */

                    //find the rigth interfac index
                    PNIO_UINT32 interface_idx;

                    for(interface_idx=0; interface_idx!=PNDV_MAX_GENERIC_DIAG_NUMBER; ++interface_idx)
                    {
                        if( pndv_data.iface_ptr->generic_diag_data[interface_idx].state == PNDV_IFACE_SERVICE_PROCCESSING )
                        {
                            if( (pndv_data.iface_ptr->generic_diag_data[interface_idx].slot == slot_nr) &&
                                (pndv_data.iface_ptr->generic_diag_data[interface_idx].subslot == subslot_nr) &&
                                (pndv_data.iface_ptr->generic_diag_data[interface_idx].diag_tag == diag_tag)
                            )
                            {
                                //found the diag to acknowledge
                                break;
                            }
                        }
                    }

                    if( interface_idx < PNDV_MAX_GENERIC_DIAG_NUMBER )
                    {
                        pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_RMV_GENERIC_DIAG_CBF, (PNIO_UINT16)diag_tag, interface_idx);
                    }
                    else
                    {
                        pndv_in_write_debug_buffer_all__(PNDV_DC_CM_RMV_CHANNEL_DIAG_CBF, local_sub_res->entity);
                    }

                    pndv_in_al_diag_done( local_sub_res->entity, PNIO_FALSE);

                    break;
                }

                case CM_ERR_ELEM:
                {
                    if ( !(    (CM_OK          == diag_channel_remove->elem[elem_cnt].response)
                            || (CM_OK_REPLACED == diag_channel_remove->elem[elem_cnt].response)
                          )
                       )
                    {
                        pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_DIAG_RMV_DONE_ERR_ELEM, slot_nr, subslot_nr );
                    }

                    pndv_data.diag.submodul_rmv_done_err_count++;

                    /* Error handling only through debug entry -> no fatal_error !!! */
                    /* Submodule could incorrectly repeat remove a channel diagnosis */
                    /* The cause is the submodule itself */
                    // it can be a pnpb or peri fault too that can cause a diagnosis remaining

                    pndv_in_al_diag_done(local_sub_res->entity, PNIO_TRUE); // to go on with the next diag, its needed to purge the waste

                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_RMV_DIAG_CBF_ERR, ((PNIO_UINT16)((((PNIO_UINT16)pndv_data.diag.submodul_rmv_done_err_count)<<8) | ((PNIO_UINT16)local_sub_res->entity) ) ));

                    break;
                }

                case CM_ERR_SEQUENCE:
                case CM_ERR_PARAM:
                default:
                {
                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_RMV_DIAG_CBF_ERR, local_sub_res->entity );

                    /* this responses leads into an error */
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, response );

                    break;
                }
            }
        }
        else
        {
            /* Since the request, a fixed system tester has already pulled the submodule again,   */
            /* so that the acknowledgment is done */
        }

        elem_cnt++;
    }
}

PNIO_VOID pndv_in_al_diag_remove2_done (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32                         response,
                                        subslot_error,
                                        elem_cnt;
    PNIO_UINT16                         slot_nr,
                                        subslot_nr;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;


    CM_UPPER_SV_DIAG_ADD_PTR_TYPE       diag_channel_rmv_key;
    diag_channel_rmv_key = rqb_ptr->args.sv.diag_add;


    if ( CM_OPC_SV_DIAG_REMOVE2 != PNDV_RQB_GET_OPCODE(rqb_ptr) )
    {
        /* only confirmation of submodule diagnosis request leads into this function */
        pndv_in_fatal_error( PNDV_MODULE, __LINE__,  PNDV_RQB_GET_OPCODE(rqb_ptr) );
    }


    PNDV_RQB_SET_OPCODE( rqb_ptr, 0);

    response = PNDV_RQB_GET_RESPONSE( rqb_ptr);

    elem_cnt = 0;

    while( elem_cnt < diag_channel_rmv_key->nr_of_elems )
    {
        /* loop ueber alle elemente */

        PNIO_UINT32 diag_tag;

        /* diag_tag darf auch 0 sein -> 0 = remove all */
        diag_tag = diag_channel_rmv_key->elem[elem_cnt].diag_tag;

        /* local_sub_res der real config holen */
        slot_nr     = diag_channel_rmv_key->elem[elem_cnt].slot_nr;
        subslot_nr  = diag_channel_rmv_key->elem[elem_cnt].subslot_nr;

        pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
        if(subslot_error)
        {
            // slot/subslot not possible, must be an implementation error
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }

        if(local_sub_res != 0)
        {
            /* There is a submodule in the real config */

            switch ( response )
            {
                case CM_OK:
                {
                    /* maybe GenericDiagnosis */

                    //find the rigth interfac index
                    PNIO_UINT16 interface_idx;

                    for(interface_idx=0; interface_idx!=PNDV_MAX_GENERIC_DIAG_NUMBER; ++interface_idx)
                    {
                        if( pndv_data.iface_ptr->generic_diag_data[interface_idx].state == PNDV_IFACE_SERVICE_PROCCESSING )
                        {
                            if( (pndv_data.iface_ptr->generic_diag_data[interface_idx].slot == slot_nr) &&
                                (pndv_data.iface_ptr->generic_diag_data[interface_idx].subslot == subslot_nr) &&
                                (pndv_data.iface_ptr->generic_diag_data[interface_idx].diag_tag == diag_tag)
                            )
                            {
                                //found the diag to acknowledge
                                break;
                            }
                        }
                    }

                    if( interface_idx < PNDV_MAX_GENERIC_DIAG_NUMBER )
                    {
                        /* a generic diag should not end up here */
                        pndv_in_fatal_error( PNDV_MODULE, __LINE__, interface_idx);
                    }
                    else
                    {
                        pndv_in_write_debug_buffer_all__(PNDV_DC_CM_RMV_CHANNEL_DIAG_CBF, local_sub_res->entity);
                    }

                    pndv_in_al_diag_done(local_sub_res->entity, PNIO_FALSE);

                    break;
                }

                case CM_ERR_ELEM:
                {
                    if ( !(    (CM_OK          == diag_channel_rmv_key->elem[elem_cnt].response)
                            || (CM_OK_REPLACED == diag_channel_rmv_key->elem[elem_cnt].response)
                          )
                       )
                    {
                        pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_DIAG_RMV_DONE_ERR_ELEM, slot_nr, subslot_nr );
                    }

                    pndv_data.diag.submodul_rmv_done_err_count++;

                    /* Error handling only through debug entry -> no fatal_error !!! */
                    /* Submodule could incorrectly repeat remove a channel diagnosis.  */
                    /* The cause here is necessarily the submodule itself */
                    // it can be a pnpb or peri fault too that can cause a diagnosis remaining

                    pndv_in_al_diag_done(local_sub_res->entity, PNIO_TRUE); // to go on with the next diag, its needed to purge the waste

                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_RMV_DIAG_CBF_ERR, ((PNIO_UINT16)((((PNIO_UINT16)pndv_data.diag.submodul_rmv_done_err_count)<<8) | ((PNIO_UINT16)local_sub_res->entity) ) ));
                    break;
                }

                case CM_ERR_SEQUENCE:
                case CM_ERR_PARAM:
                default:
                {
                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_RMV_DIAG_CBF_ERR, local_sub_res->entity );

                    /* this responses leads into an error */
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, response );

                    break;
                }
            }
        }
        else
        {
            /* Since the request, a fixed system tester has already pulled the submodule again,   */
            /* so that the acknowledgment is done */
        }

        elem_cnt++;
    }
}

//@}

//! @name Internal
//@{
/**
 *  @brief Check if a diag can be quit to peri
 *
 *  @param[in]  entity_nr       Check only for this entity_nr
 *  @param[in]  purge           purge (quit to peri) in any case
 *  @param[in]  ret_val_to_peri return value that should be given to peri
 *
 *  Check first element of channel_diag_new_list if it can be quit to peri.
 *
 */
PNIO_VOID pndv_in_al_diag_done(PNIO_UINT16 entity_nr, PNIO_UINT32 purge)
{
    PNDV_CHAN_DIAG_PTR channel_diag_ptr;

    pndv_data.diag.cm_diag_req_running = PNIO_FALSE;

    /* Switch on diag-SM */
    if ( !PNDV_LIST_IS_EMPTY( &pndv_data.diag.channel_diag_new_list  ) )
    {
        /* it is an order in the list */

        channel_diag_ptr = PNDV_LIST_FIRST( &pndv_data.diag.channel_diag_new_list, PNDV_CHAN_DIAG_PTR);
        PNDV_ASSERT(channel_diag_ptr != LSA_NULL);

        // only edit the subslot that just came from diag_done
        if ( entity_nr == channel_diag_ptr->entity_nr )
        {
            if (PNIO_TRUE == purge) // purging of this diag is requested - only possible at double diag.remove (ERR_ELEM from CM)
            {
                channel_diag_ptr->ch_diag_sm = PNDV_DIAG_CH_STATE_FREE;

                /* ready -> del req from list */

                PNDV_LIST_REMOVE_ENTRY( &channel_diag_ptr->link);

                if (pndv_data.diag.channel_diag_new_list_bookmark == channel_diag_ptr)
                {
                    pndv_data.diag.channel_diag_new_list_bookmark = 0;
                    // the intresting block was removed from diag que -> informe application about that
                    pndv_al_peri_alarm_reported_after_swo_done();
                }

                PNDV_LIST_INSERT_TAIL( &pndv_data.diag.channel_diag_free_list, &channel_diag_ptr->link);

                if (PNDV_AL_USI_CHANNELDIAG == channel_diag_ptr->usi)
                {
                    /* ChannelDiagnosis */
                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_ADD_CHANNEL_DIAG_CBF, channel_diag_ptr->entity_nr );
                    pndv_in_peri_dial_quit(channel_diag_ptr->entity_nr, PNDV_TO_PERI_OK_NO_ALARM);

                    /* Remove request was unsuccessful in CM but number of diag count was decremented before request
                       - restore its previous value */
                    pndv_data.diag.ch_diag_to_cm_count[channel_diag_ptr->entity_nr]++;
                }
                else if (PNDV_AL_USI_EXTCHANNELDIAG == channel_diag_ptr->usi)
                {
                    /* ExtChannelDiagnosis */
                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_ADD_EXT_CHANNEL_DIAG_CBF, channel_diag_ptr->entity_nr );
                    pndv_in_peri_xdial_quit(channel_diag_ptr->entity_nr, PNDV_TO_PERI_OK_NO_ALARM);

                    /* Remove request was unsuccessful in CM but number of diag count was decremented before request
                       - restore its previous value */
                    pndv_data.diag.ch_diag_to_cm_count[channel_diag_ptr->entity_nr]++;
                }
                else
                {
                    /* GenericDiagnosis */

                    //find the rigth interfac index
                    PNIO_UINT16 interface_idx = channel_diag_ptr->as.ch.kanal;

                    pndv_in_write_debug_buffer_all_add__( PNDV_DC_CM_ADD_GENERIC_DIAG_CBF,
                                                          pndv_data.iface_ptr->generic_diag_data[interface_idx].usi,
                                                          interface_idx
                                                        );

                    if( interface_idx < PNDV_MAX_GENERIC_DIAG_NUMBER )
                    {
                        pndv_in_peri_generic_dial_quit(interface_idx, PNDV_TO_PERI_OK_NO_ALARM);
                    }
                }
            }
            else if (PNDV_DIAG_CH_STATE_W_DIAG_DONE_AL == channel_diag_ptr->ch_diag_sm)
            {
                channel_diag_ptr->ch_diag_sm = PNDV_DIAG_CH_STATE_DIAG_DONE;

                if ( PNDV_AL_USI_CHANNELDIAG == channel_diag_ptr->usi )
                {
                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_ADD_CHANNEL_DIAG_DONE_CBF, channel_diag_ptr->entity_nr );
                }
                else if ( PNDV_AL_USI_EXTCHANNELDIAG == channel_diag_ptr->usi )
                {
                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_ADD_EXT_CHANNEL_DIAG_DONE_CBF, channel_diag_ptr->entity_nr );
                }
                else
                {
                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_ADD_GENERIC_DIAG_DONE_CBF, pndv_data.iface_ptr->generic_diag_data[channel_diag_ptr->as.ch.kanal].usi );
                }

                /* first try to send the current diagnosis as an alarm */
                pndv_in_al_check_dial_continue();
            }
            else
            {
                /* ignore, 
                   it may be that the diag that just came from diag_done is no longer in the list 
                   (due to module dragging, module_state! = valid ...?, generic_diag due to peri_err */
            }
        }

        /* then a possibly next edit diagnosis trigger 
           is pndv_data.diag.cm_diag_req_running = PNIO_FALSE */
        pndv_in_al_check_diag();
    }
}

/**
 *  @brief request an alarm send
 *
 *  @param[in]  diag_channel_ptr pointer to the diagnosis that should send an alarm request
 *  @return     PNDV_OK (no alarm send was requested), PNDV_OK_ASYNC (alarm send was reqested),
 *              PNDV_OK_WAIT (alarm must still wait), PNDV_ERR_SEQUENCE (alarm resource not free)
 *
 *  Try to send a alarm request to cm (CM_OPC_SV_AR_ALARM_SEND).
 *
 */
PNIO_UINT16 pndv_in_al_dial_send (PNDV_CHAN_DIAG_PTR diag_channel_ptr)
{

    PNDV_ASSERT(diag_channel_ptr != NULL);

    PNDV_AL_SUBMODUL_DATA_T*            al_submodul_ptr;
    CM_UPPER_RQB_PTR_TYPE               tmp_rqb_ptr;
    PNIO_UINT16                         ret_value;
    PNIO_UINT32                         api_nr;
    PNIO_UINT16                         slot_nr;
    PNIO_UINT16                         subslot_nr;
    PNIO_UINT16                         entity_nr;
    PNIO_UINT16                         specifier;
    PNIO_UINT32                         ar_idx;
    PNIO_UINT32                         subslot_error;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    ret_value = PNDV_OK_ASYNC;

    /* 1. Perislot (PM) has the identifier 0 for the USER */
    slot_nr     = diag_channel_ptr->slot;
    subslot_nr  = diag_channel_ptr->subslot;
    entity_nr   = diag_channel_ptr->entity_nr;
    //api_nr      = diag_channel_ptr->api_nr;
    pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
    if (subslot_error)
    {
        // slot/subslot not possible, must be an implementation error
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    if (local_sub_res == 0)
    {
        // submodule not plugged, must be an error here
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, (PNIO_UINT32)local_sub_res);
    }

    if (local_sub_res->entity != entity_nr)
    {
        //! ERROR! the entity of this submodule has changed since it was added
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, local_sub_res->entity );
    }

    api_nr      = local_sub_res->sub_module.api;

    /* One alarm block per entity_nr -> the subslots of the DAP are included */

    al_submodul_ptr = &pndv_data.al.submodul[entity_nr];

    /* The sequence of events so that the Alarm.Specifier is correct is taken into account outside

       sequence:
       1. Set APDU status
       2. Make diagnosis on CM
       3. Send diag alarm

    */

    if (PNDV_AL_FREE == al_submodul_ptr->dial_state)
    {
        ar_idx = local_sub_res->own_ar_idx;
        if (PNIO_TRUE == pndv_al_al_possible_within_state(local_sub_res))
        {
            /* alarm sending for this submodule resource is possible in the
             * current state.
             */
            if (PNDV_DIAG_CH_STATE_DIAG_DONE == diag_channel_ptr->ch_diag_sm)
            {
                /* Form specifier */
                /* ------------------------------------------------------------- */

                switch ( diag_channel_ptr->req )
                {
                    case PNDV_CHANNEL_DIAG_ADD_REQ:
                    {
                        specifier = PNDV_AL_SPEC_KOMMEND;

                        al_submodul_ptr->dial_data.alarm_type = CM_ALARM_TYPE_DIAGNOSIS;
                        al_submodul_ptr->dial_data.alarm_tag  = PNDV_AL_USI_CHANNELDIAG;

                        break;
                    }

                    case PNDV_CHANNEL_DIAG_RMV_ALL_REQ:
                    case PNDV_CHANNEL_DIAG_RMV_ALL_REQ_PNDV:
                    {
                        specifier = PNDV_AL_SPEC_GEHEND;

                        al_submodul_ptr->dial_data.alarm_type = CM_ALARM_TYPE_DIAGNOSIS_DISAPPEARS;
                        al_submodul_ptr->dial_data.alarm_tag  = PNDV_AL_USI_CHANNELDIAG;

                        break;
                    }

                    case PNDV_CHANNEL_DIAG_RMV_REQ:
                    /* for ExtChannel there is no other one available */
                    {
                        /* RMV_REQ means that the DS1 is still displayed */

                        specifier = PNDV_AL_SPEC_GEHEND_GESTOERT;

                        /* However, this alarm is an outgoing, since I only enter one channel diagnosis per alarm */

                        al_submodul_ptr->dial_data.alarm_type = CM_ALARM_TYPE_DIAGNOSIS_DISAPPEARS;
                        al_submodul_ptr->dial_data.alarm_tag  = PNDV_AL_USI_CHANNELDIAG;

                        break;
                    }

                    case PNDV_EXT_CHANNEL_DIAG_ADD_REQ:
                    {
                        specifier = PNDV_AL_SPEC_KOMMEND;

                        al_submodul_ptr->dial_data.alarm_type = CM_ALARM_TYPE_DIAGNOSIS;
                        al_submodul_ptr->dial_data.alarm_tag  = PNDV_AL_USI_EXTCHANNELDIAG;

                        break;
                    }

                    case PNDV_EXT_CHANNEL_DIAG_RMV_ALL_REQ:
                    case PNDV_EXT_CHANNEL_DIAG_RMV_ALL_REQ_PNDV:
                    {
                        specifier = PNDV_AL_SPEC_GEHEND;

                        al_submodul_ptr->dial_data.alarm_type = CM_ALARM_TYPE_DIAGNOSIS_DISAPPEARS;
                        al_submodul_ptr->dial_data.alarm_tag  = PNDV_AL_USI_EXTCHANNELDIAG;

                        break;
                    }

                    case PNDV_EXT_CHANNEL_DIAG_RMV_REQ:
                    /* for ExtChannel there is no other available ??? */
                    {
                        /* RMV_REQ means that the DS1 is still displayed */

                        specifier = PNDV_AL_SPEC_GEHEND_GESTOERT;

                        /* However, this alarm is an outgoing, since I only enter one channel diagnosis per alarm */

                        al_submodul_ptr->dial_data.alarm_type = CM_ALARM_TYPE_DIAGNOSIS_DISAPPEARS;
                        al_submodul_ptr->dial_data.alarm_tag  = PNDV_AL_USI_EXTCHANNELDIAG;

                        break;
                    }

                    case PNDV_GENERIC_DIAG_ADD_REQ:
                    {
                        specifier = PNDV_AL_SPEC_KOMMEND;

                        al_submodul_ptr->dial_data.alarm_type = CM_ALARM_TYPE_DIAGNOSIS;
                        al_submodul_ptr->dial_data.alarm_tag  = pndv_data.iface_ptr->generic_diag_data[ diag_channel_ptr->as.ch.kanal ].usi;

                        break;
                    }

                    case PNDV_GENERIC_DIAG_RMV_REQ:
                    /* for ExtChannel there is no other one available */
                    {
                        /* RMV_REQ means that the DS1 is still displayed */

                        specifier = PNDV_AL_SPEC_GEHEND_GESTOERT;

                        /* However, this alarm is an outgoing, since I only enter one channel diagnosis per alarm */

                        al_submodul_ptr->dial_data.alarm_type = CM_ALARM_TYPE_DIAGNOSIS_DISAPPEARS;
                        al_submodul_ptr->dial_data.alarm_tag  = pndv_data.iface_ptr->generic_diag_data[ diag_channel_ptr->as.ch.kanal ].usi;

                        break;
                    }

                    default:
                    {
                        specifier = PNDV_AL_SPEC_KEINE_ANGABE;
                        pndv_in_fatal_error( PNDV_MODULE, __LINE__, diag_channel_ptr->req);
                    }
                }

                /* Set off the alarm */
                /* ----------------------------------------------------------------- */

                al_submodul_ptr->dial_state = PNDV_AL_SENT;

              /*al_submodul_ptr->dial_data.device_nr           set in pndv_in_al_init_submodul */
                al_submodul_ptr->dial_data.ar_nr               = (PNIO_UINT16)pndv_data.ar[ar_idx].ar_nr;
                al_submodul_ptr->dial_data.session_key         = pndv_data.ar[ar_idx].session_key;    /* see CL_EVENT_IN_DATA_IND or SV_EVENT_AR_INFO_IND */
                al_submodul_ptr->dial_data.api                 = api_nr;
                al_submodul_ptr->dial_data.slot_nr             = slot_nr;
                al_submodul_ptr->dial_data.subslot_nr          = subslot_nr;
              /*al_submodul_ptr->dial_data.alarm_priority      dont care */
              /*al_submodul_ptr->dial_data.alarm_type          siehe oben */
              /*al_submodul_ptr->alarm_sequence                dont care */
              /*al_submodul_ptr->diag_channel_available        dont care */
              /*al_submodul_ptr->diag_generic_available        dont care */
              /*al_submodul_ptr->diag_submod_available         dont care */
              /*al_submodul_ptr->reserved                      dont care */
              /*al_submodul_ptr->ar_diagnosis_state            dont care */
              /*al_submodul_ptr->mod_ident                     dont care */
              /*al_submodul_ptr->sub_ident                     dont care */
              /*al_submodul_ptr->dial_data.alarm_tag           siehe oben */
              /*al_submodul_ptr->dial_data.alarm_data_length   siehe unten */
              /*al_submodul_ptr->dial_data.alarm_data          siehe unten */
              /*al_submodul_ptr->cm_pnio_err                   cnf */
              /*al_submodul_ptr->maintenance_status            dont care */

                if (PNDV_AL_SPEC_GEHEND == specifier)
                {
                    /* all gone
                       -> empty diagnosis */

                    al_submodul_ptr->dial_data.alarm_data_length  = 0;

                    /* If length = 0, Ptr = 0 must also be */
                    al_submodul_ptr->dial_data.alarm_data  = NIL;
                }
                else
                {
                    switch (diag_channel_ptr->req)
                    {
                        case PNDV_CHANNEL_DIAG_ADD_REQ:
                        case PNDV_CHANNEL_DIAG_RMV_REQ:
                        {
                            /* ChannelDiagnosisData */
                            /* --------------------------------------------- */

                            al_submodul_ptr->dial_data.alarm_data_length   = PNDV_AL_DIAL_CHN_INFO_LEN;

                            /* Because of the independence of the processor endianes, no structure is used here */
                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_CHANNEL_NO_H] = (PNIO_UINT8)(diag_channel_ptr->as.ch.kanal >> 8);
                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_CHANNEL_NO_L] = (PNIO_UINT8) diag_channel_ptr->as.ch.kanal;

                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_CHANNEL_SPEC] = (PNIO_UINT8)(diag_channel_ptr->as.ch.properties >> 8);
                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_CHANNEL_TYPE] = (PNIO_UINT8) diag_channel_ptr->as.ch.properties;

                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_ERR_TYPE_H]   = (PNIO_UINT8)(diag_channel_ptr->as.ch.fehlernummer >> 8);
                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_ERR_TYPE_L]   = (PNIO_UINT8) diag_channel_ptr->as.ch.fehlernummer;

                            break;
                        }

                        case PNDV_EXT_CHANNEL_DIAG_ADD_REQ:
                        case PNDV_EXT_CHANNEL_DIAG_RMV_REQ:
                        {
                            /* ExtChannelDiagnosisData */
                            /* --------------------------------------------- */

                            al_submodul_ptr->dial_data.alarm_data_length   = PNDV_AL_DIAL_XCH_INFO_LEN;

                            /* Because of the independence of the processor endianes, no structure is used here */
                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_CHANNEL_NO_H] = (PNIO_UINT8)(diag_channel_ptr->as.xch.kanal >> 8);
                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_CHANNEL_NO_L] = (PNIO_UINT8) diag_channel_ptr->as.xch.kanal;

                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_CHANNEL_SPEC] = (PNIO_UINT8)(diag_channel_ptr->as.xch.properties >> 8);
                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_CHANNEL_TYPE] = (PNIO_UINT8) diag_channel_ptr->as.xch.properties;

                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_ERR_TYPE_H]   = (PNIO_UINT8)(diag_channel_ptr->as.xch.fehlernummer >> 8);
                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_ERR_TYPE_L]   = (PNIO_UINT8) diag_channel_ptr->as.xch.fehlernummer;

                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_XERR_TYPE_H]  = (PNIO_UINT8)(diag_channel_ptr->as.xch.error_type >> 8);
                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_XERR_TYPE_L]  = (PNIO_UINT8) diag_channel_ptr->as.xch.error_type;

                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_ADD_VALUE_B3] = (PNIO_UINT8)(diag_channel_ptr->as.xch.add_value >> 24);
                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_ADD_VALUE_B2] = (PNIO_UINT8)(diag_channel_ptr->as.xch.add_value >> 16);
                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_ADD_VALUE_B1] = (PNIO_UINT8)(diag_channel_ptr->as.xch.add_value >> 8);
                            al_submodul_ptr->dial_info[PNDV_AL_DIAL_INFO_OFFS_ADD_VALUE_B0] = (PNIO_UINT8) diag_channel_ptr->as.xch.add_value;

                            break;
                        }

                        case PNDV_GENERIC_DIAG_ADD_REQ:
                        case PNDV_GENERIC_DIAG_RMV_REQ:
                        {
                            /* GenericDiagnosisData */
                            /* --------------------------------------------- */

                            al_submodul_ptr->dial_data.alarm_data_length =
                            pndv_data.iface_ptr->generic_diag_data[ diag_channel_ptr->as.ch.kanal ].length;

                            PNDV_COPY_BYTE( al_submodul_ptr->dial_info,
                                            pndv_data.iface_ptr->generic_diag_data[ diag_channel_ptr->as.ch.kanal ].data,
                                            al_submodul_ptr->dial_data.alarm_data_length
                                          );

                            break;
                        }
                        case PNDV_CHANNEL_DIAG_RMV_ALL_REQ:
                        default:
                        {
                            /* default: already checked above */
                        }
                    }

                    /* Append ptr to alarm rqb */

                    al_submodul_ptr->dial_data.alarm_data  = &al_submodul_ptr->dial_info[0];
                }

                pndv_in_write_debug_buffer_all__(PNDV_DC_CM_DIAL_SEND, slot_nr);

                PNDV_RQB_SET_HANDLE(&al_submodul_ptr->dial_rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                PNDV_RQB_SET_OPCODE(&al_submodul_ptr->dial_rqb, CM_OPC_SV_AR_ALARM_SEND);
                PNDV_STORE_CHANNEL_DIAG_QUIT_IN_RQB( &al_submodul_ptr->dial_rqb, diag_channel_ptr->as.ch.quit_to_peri);

                tmp_rqb_ptr = &al_submodul_ptr->dial_rqb;

                PNDV_REQUEST(tmp_rqb_ptr, LSA_COMP_ID_CM);
            }
            else
            {
                /* Diagnosis is to be repeated be discontinued,
                 It is not twice better here */

                pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
            }
        }
        else
        {
            /* either there is no module plugged
             * or an alarm is not possible within the current state of the
             * submodule.
             */

            ret_value = PNDV_OK;
        }
    }
    else /* if ( PNDV_AL_FREE == al_submodul_ptr->dial_state ) */
    {
        /* Maybe an alarm for another channel of this slot should be issued immediately 
          (2 or more channel diagnoses are reported at the same time), 
          this must wait */

        ret_value = PNDV_ERR_SEQUENCE;
    }
    return(ret_value);
}
//@}

//! @name Interface
//@{
/**
 *  @brief process alarm send response
 *
 *  @param[in]  rqb_ptr request block pointer to alarm
 *
 *  Process the response of a returning CM_OPC_SV_AR_ALARM_SEND request.
 *
 */
PNIO_VOID pndv_in_al_al_ack (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNDV_AL_SUBMODUL_DATA_T*            al_submodul_ptr;
    CM_UPPER_ALARM_PTR_TYPE             alarm_ptr;
    PNIO_UINT32                         response,
                                        subslot_error;
    PNIO_UINT16                         slot_nr,
                                        subslot_nr;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;
    PNIO_UINT8                          ret_to_peri = PNDV_ERR_FATAL;


    alarm_ptr = rqb_ptr->args.sv.ar_alarm_send;

    /* alarm_send-done == alarm_ack from controller */

    if ( CM_OPC_SV_AR_ALARM_SEND != PNDV_RQB_GET_OPCODE(rqb_ptr) )
    {
        /* only confirmation of submodule alarm request leads into this function */
        pndv_in_fatal_error( PNDV_MODULE, __LINE__,  PNDV_RQB_GET_OPCODE(rqb_ptr) );
    }

    PNDV_RQB_SET_OPCODE( rqb_ptr, 0);

    response = PNDV_RQB_GET_RESPONSE( rqb_ptr);


    switch (response)
    {
        case CM_OK:
        {
            ret_to_peri = PNDV_TO_PERI_OK;
            break;
        }
        case CM_OK_CANCELLED:
        {
            /* as described - OK */
            ret_to_peri = PNDV_TO_PERI_OK_CANCELLED;
            break;
        }
        case CM_OK_R1_NOT_ACKED:
        {
            ret_to_peri = PNDV_TO_PERI_OK_R1_NOT_ACKED;
            break;
        }

        case CM_ERR_SEQUENCE:
        case CM_ERR_PARAM:
        case CM_ERR_FORMAT:
        default:
        {
            /* this responses leads into an error */
            pndv_in_fatal_error( PNDV_MODULE, __LINE__,  response );    /* NOTREACHED */

            break;
        }
    }

    /* Determine submodule index and Ptr on pndv-al-blk */
    /* --------------------------------------------------------------------- */

    slot_nr     = alarm_ptr->slot_nr;
    subslot_nr  = alarm_ptr->subslot_nr;

    /* get local_sub_res of the real config */

    if ( alarm_ptr->alarm_type == CM_ALARM_TYPE_STATUS)
    {
        // 0x8300 - 0x8303  Reporting System 
        if (  (PNDV_AL_STAL_RS_LOW_WATERMARK_USI <= alarm_ptr->alarm_tag)
            && (alarm_ptr->alarm_tag <=  PNDV_AL_STAL_RS_EVENT_USI)
           )
        {
               slot_nr = PNDV_IM_SLOT_NO;
               subslot_nr = 1;
        }
    }

    pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
    if(subslot_error)
    {
        // slot/subslot not possible, must be an implementation error
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }


    if (local_sub_res != 0)
    {
        /* There is a submodule in the real config */

        al_submodul_ptr = &pndv_data.al.submodul[local_sub_res->entity];

        /* CM_OK == response -> Alarm acknowledge received */
        /* branch by type */
        /* --------------------------------------------------------------------- */

        switch ( alarm_ptr->alarm_type )
        {
            case CM_ALARM_TYPE_DIAGNOSIS:
            case CM_ALARM_TYPE_DIAGNOSIS_DISAPPEARS:
            {
                pndv_in_write_debug_buffer_3__(PNDV_DC_CM_DIAL_ACK, local_sub_res->entity, alarm_ptr->alarm_sequence, response);

                if ( PNDV_AL_SENT == al_submodul_ptr->dial_state )
                {
                    al_submodul_ptr->dial_state = PNDV_AL_FREE;
                }
                else
                {
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, al_submodul_ptr->dial_state );
                }

                if (alarm_ptr->alarm_tag == PNDV_AL_USI_CHANNELDIAG)
                {
                    if ( PNDV_LOAD_CHANNEL_DIAG_QUIT_FROM_RQB( rqb_ptr) != PNIO_FALSE)
                    {
                        pndv_in_peri_dial_quit(local_sub_res->entity, ret_to_peri);
                    }
                }
                else if (alarm_ptr->alarm_tag == PNDV_AL_USI_EXTCHANNELDIAG)
                {
                    pndv_in_peri_xdial_quit(local_sub_res->entity, ret_to_peri);
                }
                else //everything else is supposed to be an generic diag
                {
                    //generic diag ...

                    //the interface index of the coupling ressource
                    PNIO_UINT16 interface_index;
                    PNIO_BOOL   alarm_found = PNIO_FALSE;

                    for(interface_index=0; interface_index!=PNDV_MAX_GENERIC_DIAG_NUMBER; ++interface_index)
                    {
                        if (pndv_data.iface_ptr->generic_diag_data[interface_index].usi == alarm_ptr->alarm_tag)
                        {
                            // compare length and content of alarm data, because USI might not be unique to identify the correct interface_index
                            if (pndv_data.iface_ptr->generic_diag_data[interface_index].length == alarm_ptr->alarm_data_length)
                            {
                                PNIO_BOOL mem_cmp = PNDV_CMP_BYTE((PNIO_VOID*)pndv_data.iface_ptr->generic_diag_data[interface_index].data,
                                                                 (PNIO_VOID*)alarm_ptr->alarm_data,
                                                                 (PNIO_UINT32)alarm_ptr->alarm_data_length) ? PNIO_FALSE : PNIO_TRUE;
                                if (mem_cmp)
                                {
                                    pndv_in_peri_generic_dial_quit(interface_index, ret_to_peri);
                                    alarm_found = PNIO_TRUE;
                                    break;
                                }
                            }
                        }
                    }
                    if (!alarm_found)
                    {
                        pndv_in_fatal_error( PNDV_MODULE, __LINE__, alarm_ptr->alarm_tag );
                    }
                }
                /* Do not acknowledge DAP AL peri */


                /* Alarm resource is free -> report alarm if necessary */
                if (PNIO_TRUE ==  pndv_data.diag.dial_repeat_req[local_sub_res->entity])
                {
                    pndv_in_al_check_dial_continue();
                }


                break;
            }

            case CM_ALARM_TYPE_PROCESS:
            {
                pndv_in_write_debug_buffer_3__(PNDV_DC_CM_PRAL_ACK, local_sub_res->entity, alarm_ptr->alarm_sequence, response);

                if ( PNDV_AL_SENT == al_submodul_ptr->pral_state )
                {
                    al_submodul_ptr->pral_state = PNDV_AL_FREE;
                }
                else
                {
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, al_submodul_ptr->pral_state );
                }

                pndv_in_peri_pral_quit(local_sub_res->entity, ret_to_peri);
                /* Do not acknowledge DAP AL peri */

                break;
            }

            case CM_ALARM_TYPE_UPDATE:
            {
                pndv_in_write_debug_buffer_3__(PNDV_DC_CM_UPAL_ACK, local_sub_res->entity, alarm_ptr->alarm_sequence, response);

                if ( PNDV_AL_SENT == al_submodul_ptr->upal_state )
                {
                    al_submodul_ptr->upal_state = PNDV_AL_FREE;
                }
                else
                {
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, al_submodul_ptr->upal_state );
                }
                pndv_in_peri_upal_quit(local_sub_res->entity);

                break;
            }
            case CM_ALARM_TYPE_UPLOAD_AND_STORAGE:
            {
                pndv_in_write_debug_buffer_3__(PNDV_DC_CM_URAL_ACK, local_sub_res->entity, alarm_ptr->alarm_sequence, response);

                if ( PNDV_AL_SENT == al_submodul_ptr->ural_state )
                {
                    al_submodul_ptr->ural_state = PNDV_AL_FREE;
                }
                else
                {
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, al_submodul_ptr->ural_state );
                }
                pndv_in_peri_ural_quit(local_sub_res->entity);

                break;
            }
            case CM_ALARM_TYPE_STATUS:
            {
                pndv_in_write_debug_buffer_3__(PNDV_DC_CM_STAL_ACK, local_sub_res->entity, alarm_ptr->alarm_sequence, response);

                if ( PNDV_AL_SENT == al_submodul_ptr->stal_state )
                {
                    al_submodul_ptr->stal_state = PNDV_AL_FREE;
                }
                else
                {
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, al_submodul_ptr->stal_state );
                }
                pndv_in_peri_stal_quit(local_sub_res->entity, ret_to_peri);

                break;
            }
            case CM_ALARM_TYPE_RETURN_OF_SUBMODULE:
            {
                pndv_in_write_debug_buffer_3__(PNDV_DC_CM_ROSAL_ACK, local_sub_res->entity, alarm_ptr->alarm_sequence, response);

                if ( PNDV_AL_SENT == al_submodul_ptr->rosal_state )
                {
                    al_submodul_ptr->rosal_state = PNDV_AL_FREE;
                }
                else
                {
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, al_submodul_ptr->rosal_state );
                }

                pndv_in_peri_ros_al_quit(local_sub_res->entity);

               //  ROS-alarms on subslots with ARP are not supported here, because classic CPUs (3xx,4xx) don't support this!
               //  They ignore ROS-alarms.      -> ROS-alarms are always issued for subslots not being ARP in our products!
               //
               //  Theoretical: If submodules were set to ARP before ROS, then the PP_SM would stick in W_IN_DATA during AR startup
               //               and we would have to switch it to IN_DATA here.
               //  But: when submodules may also get ROS-alarms without being set to ARP (like in our products), we would have to check if
               //       ARP was set in order to be allowed to switch PP_SM here! Otherwise we could switch the PP_SM here although
               //       CM wants to do it in parallel
               //     pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_CM_IN_DATA_IND);
               //     pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_PERI_IN_DATA_IND_DONE);


                //check for pending rosal requests
                pndv_in_al_check_rosal_queue();

                break;
            }

            default:
            {
                pndv_in_write_debug_buffer_3__(PNDV_DC_CM_AL_ACK, local_sub_res->entity, alarm_ptr->alarm_sequence, response);

                pndv_in_fatal_error( PNDV_MODULE, __LINE__, alarm_ptr->alarm_type);

                break;
            }
        }

    }
    else
    {
        /* Since the request, a fixed system tester has already pulled the submodule again, */
        /*  so that the acknowledgment is done */
    }
}

/**
 *  @brief process a alarm indication
 *
 *  @param[in]  rqb_ptr pointer to the request block of the alarm ind
 *
 *  Alarm indications are not supported. Processing CM_OPC_SV_AR_ALARM_IND is
 *  limited to giving the resource back to CM.
 */
PNIO_VOID pndv_in_al_sv_alarm_ind (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;

    response = PNDV_RQB_GET_RESPONSE( rqb_ptr);

    if ( CM_OK != response)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, response);
    }

    pndv_in_write_debug_buffer_al__(PNDV_DC_CM_AL_IND,0);

    /* return blindly !! do not acknowledge !! */

    PNDV_RQB_SET_OPCODE( rqb_ptr, CM_OPC_SV_AR_ALARM_RSP);
}

/**
 *  @brief process a alarm ack
 *
 *  @param[in]  rqb_ptr pointer to the request block of the alarm ind
 *
 *  Alarm Acks are not supported. Precossing CM_OPC_SV_AR_ALARM_ACK is limited
 *  to giving the resource back to CM.
 */
PNIO_VOID pndv_in_al_sv_alarm_ack_done (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;

    response = PNDV_RQB_GET_RESPONSE( rqb_ptr);

    if (   (CM_ERR_SEQUENCE == response)
        || (CM_ERR_PARAM    == response)
       )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, response);
    }

    pndv_in_write_debug_buffer_al__(PNDV_DC_CM_AL_ACK_IND,0);

    /* that would be my RQB if I had once acknowledged an alarm */
}
//@}


//! @name Internal
//@{
/**
 *  @brief check if alarm/diag can be send in current ar state
 *
 *  @param[in]  ar_idx Index of the AR
 *  @return     PNIO_TRUE = Alarm possible, PNIO_FALSE = Alarm not possible
 *
 *  Check the AR-State if an Alarm can be send now. AR has to be in a state
 *  where appl ready decision is done.
 *
 */
PNIO_BOOL pndv_al_check_ar_is_ready(PNIO_UINT32 ar_idx)
{

    PNIO_BOOL ret_val;

    ret_val = PNIO_FALSE;

    if (ar_idx != PNDV_AR_IDX_NOT_USED)
    {
        if (   (   ( PNDV_AR_SM_W_IN_DATA_IND           <= pndv_data.ar[ar_idx].sm_state )     /* RTC1/RTC2 */
                 &&( PNDV_AR_SM_IN_DATA                 >= pndv_data.ar[ar_idx].sm_state )
                )
            || (   ( PNDV_AR_SM_W_IN_DATA_IND_RTC3      <= pndv_data.ar[ar_idx].sm_state )     /* RTC3 */
                && ( PNDV_AR_SM_IN_DATA_RTC3            >= pndv_data.ar[ar_idx].sm_state )
               )
           )
        {
            ret_val = PNIO_TRUE;
        }
    }

    return( ret_val);
}

/**
 *  @brief check if alarm can be send in current submodule state
 *
 *  @param[in]  sub_res_ptr pointer to the submodule resource to check
 *  @return     PNIO_TRUE = Alarm possible, PNIO_FALSE = Alarm not possible
 *
 *  Check the AR-State if an Alarm can be send now.
 *
 *  According to the LSA-CM specification an Alarm is possible as soon as a
 *  Module is "ready" or "ready pending".
 *  This information is providet to the CM with the PRM response, for the
 *  first time.
 *
 */
PNIO_BOOL pndv_al_al_possible_within_state(PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T sub_res_ptr)
{

    PNIO_BOOL ret_val;

    ret_val = PNIO_FALSE;

    if (  (sub_res_ptr->res_state > PNDV_CFG_SUBMODULE_RES_STATE_W_PRM_END)
        )
    {
        ret_val = PNIO_TRUE;
    }

    return( ret_val);
}

/**
 *  @brief check if alarm can be send in current submodule state
 *
 *  @param[in]  sub_res_ptr pointer to the submodule resource to check
 *  @return     PNIO_TRUE = Alarm possible, PNIO_FALSE = Alarm not possible
 *
 *  Check the AR-State if an Alarm can be send now.
 *
 *  According to the LSA-CM specification an Alarm is possible as soon as a
 *  Module is "ready" or "ready pending".
 *  This information is providet to the CM with the PRM response, for the
 *  first time.
 *
 */
PNIO_BOOL pndv_al_diag_possible_within_state(PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T sub_res_ptr, PNDV_CHANNEL_DIAG_REQ_T req)
{

    PNIO_BOOL ret_val;

    ret_val = PNIO_FALSE;

    if (PNDV_CFG_SUBMODULE_RES_STATE_W_ADD <= sub_res_ptr->res_state)  /* module plugged to cm */
    {
        ret_val = PNIO_TRUE;
    }

    return (ret_val);
}
//@}

/*****************************************************************************/
/*  end of file.                                                             */
/*****************************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
