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
/*  F i l e               &F: pndv_ar.c                                 :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/* Functions that process configuration and parameterization                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* include hierarchy */

#include "pndv_inc.h"

#define PNDV_MODULE PNDV_ERR_MODULE_AR



/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**********                                                         **********/
/**********                   INTERNAL FUNCTIONS                    **********/
/**********                                                         **********/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


//! @name Connect
//@{
/**
 *  @brief handle a check ind from cm
 *  @ingroup pndv_ar_connect
 *
 *  @param[in]  rqb_ptr pointer to request block with connect req
 *
 *
 */
PNIO_VOID pndv_ar_cm_connect_ind(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    CM_UPPER_EVENT_PTR_TYPE     event_ptr;

    PNIO_UINT32                 response,
                                ar_idx;

    PNDV_CONNECT_IND_RET_VAL_T  ret_val;
    PNIO_BOOL                   is_io_memory_allocated = PNIO_FALSE;

    ret_val = PNDV_CONNECT_IND_RET_OK;

    response = PNDV_RQB_GET_RESPONSE( rqb_ptr);


    if ( CM_OK != response )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, response);
    }

    event_ptr = rqb_ptr->args.pd.event;

    ret_val = pndv_ar_tool_connect_ind_find_ar_idx(&ar_idx, event_ptr);
    pndv_ar_tool_connect_ind_store_ar_data(ar_idx, event_ptr);

    /** save rqb_ptr for later response **/
    pndv_data.ar[ar_idx].con_ind_rqb_ptr = rqb_ptr;

    if ( pndv_data.cfg.peri_cfg.sm_state == PNDV_SM_PERIBUS_STATE_UNKNOWN )
    {
        /* connect service disabled or unknown -> ar abort (except for DA)*/
        ret_val = PNDV_CONNECT_IND_RET_ABORT;
    }

    if (ret_val == PNDV_CONNECT_IND_RET_OK)
    {
        ret_val = pndv_ar_tool_connect_ind_handle_iocr(ar_idx, pndv_data.ar[ar_idx].ar_set_nr, pndv_data.ar[ar_idx].session_key, event_ptr);
        if (ret_val == PNDV_CONNECT_IND_RET_OK)
        {
            is_io_memory_allocated = PNIO_TRUE;
        }
    }

    // check for sr ar
    if (    (pndv_data.ar[ar_idx].ar_set_nr != 0)
        &&  (ret_val == PNDV_CONNECT_IND_RET_OK)    // only if not already an error rejects the ar
        )
    {
        PNIO_UINT32 ar_set_idx;
        // this ar ist part of a system redundancy ar-set
        ar_set_idx = pndv_ar_get_sr_idx_by_ar_set_nr(pndv_data.ar[ar_idx].ar_set_nr);
        if (ar_set_idx == PNDV_SR_SET_IDX_NOT_USED)
        {
            // the addressed ar_set is not know yet in use
            // find a free resource
            ar_set_idx = pndv_ar_get_sr_idx_by_ar_set_nr(PNDV_SR_SET_NR_NOT_USED);
            if (PNDV_SR_SET_IDX_NOT_USED == ar_set_idx)
            {
                // no free ar_set resource found (maximum of sr sets must be reached)
                ret_val = PNDV_CONNECT_IND_RET_ERROR_RESOURCE;
            }
            else
            {
                // its much less effort to initialize the ar_set_nr outside the sm
                pndv_data.sr[ar_set_idx].ar_set_nr = pndv_data.ar[ar_idx].ar_set_nr;
                pndv_ar_sr_sm(ar_set_idx, PNDV_SR_EVENT_ADD_TO_ARSET, ar_idx);
            }
        }
        else
        {
            // ar-set already present, this is an aditional ar
            if (pndv_data.sr[ar_set_idx].sm == PNDV_SR_STATE_ABORT)
            {
                // ar-set is in abort state, don't add an ar now
                ret_val = PNDV_CONNECT_IND_RET_ABORT;
            }
            else
            {
                // add the ar to the set
                pndv_ar_sr_sm(ar_set_idx, PNDV_SR_EVENT_ADD_TO_ARSET, ar_idx);
            }
        }
    }

    switch (ret_val)
    {
        case PNDV_CONNECT_IND_RET_OK:
        {
            /* set response ok */
            pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_CONNECT_IND_OK_DONE);

            break;
        }
        case PNDV_CONNECT_IND_RET_ABORT:
        case PNDV_CONNECT_IND_RET_ERROR_RESOURCE:
        {
            if(is_io_memory_allocated)
            {
                PNDV_FREE_IOCR_MEMORY(ar_idx);
            }

            /* ar not allowed or no resource*/
            pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_CHK_IND_AR_ABORT, (PNIO_UINT16)__LINE__, 0);
            /* there will be no disconnect ind */
            pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_CONNECT_IND_ERROR_DONE);
            break;
        }
        default:
        {
            break;
        }
    }
}

/**
 *  @brief search for existing ar in repository
 *  @ingroup pndv_ar_connect
 *
 *  @param[out] ar_idx_ptr  pointer to an ar index, returns the index of a free ar resource
 *  @param[out] event_ptr   pointer to an event structure
 *  @return     PNDV_CHECK_IND_RET_OK if a new ar or a repeaded ar, PNDV_CHECK_IND_RET_ABORT otherwise
 *
 *  check if ar is new or repeated. Return existing ar index or index of a free ar resource
 *
 */
PNDV_CONNECT_IND_RET_VAL_T pndv_ar_tool_connect_ind_find_ar_idx(PNIO_UINT32* ar_idx_ptr, CM_UPPER_EVENT_PTR_TYPE event_ptr)
{
    PNDV_CONNECT_IND_RET_VAL_T ret_val = PNDV_CONNECT_IND_RET_OK;


    if( !ar_idx_ptr )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    if( !event_ptr )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    /* search ar, check for existing ar_nr and sm_ar != OFFLINE */
    for( (*ar_idx_ptr) = 0; (*ar_idx_ptr) < PNDV_CM_AR_NO; (*ar_idx_ptr)++ )
    {
        if(  ( PNDV_AR_SM_OFFLINE != pndv_data.ar[(*ar_idx_ptr)].sm_state )
          && ( event_ptr->ar_nr   == pndv_data.ar[(*ar_idx_ptr)].ar_nr )
          )
        {
            break;  /* existing ar_nr, maybe a repeated connect ind*/
        }
    }

    switch( (*ar_idx_ptr) )
    {
        case PNDV_CM_AR_NO:
        {
            /* index is on final value -> ar is a new one */
            /* possibly accept new ar */
            switch (event_ptr->u.sv.ar_connect->ar_type)
            {
                /* User data - AR */
                case CM_AR_TYPE_SINGLE:
                case CM_AR_TYPE_SINGLE_RTC3:
                case CM_AR_TYPE_SINGLE_SYSRED:
                case CM_AR_TYPE_SUPERVISOR:
                {
                    if (    ( CM_AR_TYPE_SUPERVISOR         == event_ptr->u.sv.ar_connect->ar_type )
                         && ( CM_AR_PROP_DEVICE_ACCESS &  event_ptr->u.sv.ar_connect->ar_properties )
                       )
                    {
                        /* EasySupervisor AR - Device Access */
                        for( (*ar_idx_ptr) = PNDV_MAX_ARS_RT; (*ar_idx_ptr) < (PNDV_MAX_ARS_RT + PNDV_MAX_ARS_DA); (*ar_idx_ptr)++ )
                        {
                            if( PNDV_AR_SM_OFFLINE == pndv_data.ar[(*ar_idx_ptr)].sm_state )
                            {
                                break;
                            }
                        }

                        if( (PNDV_MAX_ARS_RT + PNDV_MAX_ARS_DA) <= (*ar_idx_ptr) )
                        {
                            /* more than (PNDV_MAX_ARS_RT + PNDV_MAX_ARS_DA) not possible ! */
                            ret_val = PNDV_CONNECT_IND_RET_ABORT;
                            (*ar_idx_ptr)  = PNDV_CM_AR_NO; /* set ar_idx to abort ressource */

                            pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_CHK_IND_AR_ABORT, (PNIO_UINT16)__LINE__, (*ar_idx_ptr));
                        }
                    }
                    else
                    {
                        /* User data AR including IOSAR */
                        for( (*ar_idx_ptr) = 0; (*ar_idx_ptr) < PNDV_MAX_ARS_RT; (*ar_idx_ptr)++ )
                        {
                            if( PNDV_AR_SM_OFFLINE == pndv_data.ar[(*ar_idx_ptr)].sm_state )
                            {
                                break;
                            }
                        }

                        if( PNDV_MAX_ARS_RT <= (*ar_idx_ptr) )
                        {
                            /* more than PNDV_MAX_ARS_RT not possible ! */
                            ret_val = PNDV_CONNECT_IND_RET_ABORT;
                            (*ar_idx_ptr)  = PNDV_CM_AR_NO; /* set ar_idx to abort ressource */

                            pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_CHK_IND_AR_ABORT, (PNIO_UINT16)__LINE__, (*ar_idx_ptr));
                        }
                    }

                    break;
                }

                /* We don't know -> we don't want to */
                default:
                {
                    ret_val = PNDV_CONNECT_IND_RET_ABORT;
                    (*ar_idx_ptr)  = PNDV_CM_AR_NO; /* set ar_idx to abort ressource */

                    pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_CHK_IND_AR_ABORT, (PNIO_UINT16)__LINE__, event_ptr->u.sv.ar_connect->ar_type);

                    break;
                }
            }
        }
        break;

        default:
        {
            if( (*ar_idx_ptr) < PNDV_CM_AR_NO )
            {
                /* Repetition */
            }
            else
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, (*ar_idx_ptr));
            }
        }
    }

    /* last parameter:
        0xttppppii   t: ar_type Bit 0..7  p: ar_properties Bit 0..15  i: ar_idx Bit 0..7
     */
    pndv_in_write_debug_buffer_3__( PNDV_DC_CM_CONNECT_IND,
                                    ((PNDV_MAX_ARS_RT + PNDV_MAX_ARS_DA) <= (*ar_idx_ptr))? PNDV_AR_SM_OFFLINE : pndv_data.ar[(*ar_idx_ptr)].sm_state,
                                    event_ptr->session_key,
                                      ((((PNIO_UINT32)(event_ptr->u.sv.ar_connect->ar_type)      ) << 24 ) & 0xFF000000)
                                    | ((((PNIO_UINT32)(event_ptr->u.sv.ar_connect->ar_properties)) <<  8 ) & 0x00FFFF00)
                                    | (( (PNIO_UINT32)(*ar_idx_ptr)                                ) & 0x000000FF)
                                  );

    return ret_val;
}


/**
 *  @brief stores ar data to internal resource
 *  @ingroup pndv_ar_connect
 *
 *  @param[in]  ar_idx ar index
 *  @param[in]  event_ptr event pointer
 *
 *
 */
PNIO_VOID pndv_ar_tool_connect_ind_store_ar_data(PNIO_UINT32 ar_idx, CM_UPPER_EVENT_PTR_TYPE event_ptr)
{
    if( !event_ptr )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);            /*NOTREACHED*/
    }

    if( ar_idx == PNDV_AR_IDX_NOT_USED )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);    /*NOTREACHED*/
    }

    if (PNDV_AR_SM_OFFLINE == pndv_data.ar[ar_idx].sm_state )
    {
        PNIO_UINT32 i;
        /*
        event_ptr->u.sv.ar_connect.host_ip;
        event_ptr->u.sv.ar_connect.ar_type;
        event_ptr->u.sv.ar_connect.ar_properties;
        event_ptr->u.sv.ar_connect.ar_uuid;
        event_ptr->u.sv.ar_connect.cmi_station_name;
        event_ptr->u.sv.ar_connect.cmi_obj_uuid;
        event_ptr->u.sv.ar_connect.alarm_send_max_length;
        event_ptr->u.sv.ar_connect.nr_of_iocrs;
        event_ptr->u.sv.ar_connect.iocr[1];
        event_ptr->u.sv.ar_connect.is_iosar_with_device_access;
        event_ptr->u.sv.ar_connect.disconnect;
        */

        pndv_data.ar[ar_idx].ar_type        = event_ptr->u.sv.ar_connect->ar_type;         /* store ar type */
        pndv_data.ar[ar_idx].ar_properties  = event_ptr->u.sv.ar_connect->ar_properties;   /* store ar ar_properties */
        pndv_data.ar[ar_idx].sr_properties  = event_ptr->u.sv.ar_connect->sr_properties;
        pndv_data.ar[ar_idx].sr_rdht_msec   = event_ptr->u.sv.ar_connect->sr_rdht_msec;
        pndv_data.ar[ar_idx].sr_firstAR     = event_ptr->u.sv.ar_connect->sr_firstAR;
        pndv_data.ar[ar_idx].nr_of_iocrs    = event_ptr->u.sv.ar_connect->nr_of_iocrs;

        pndv_data.ar[ar_idx].input_cr_io_data_length = 0;
        pndv_data.ar[ar_idx].input_cr_ioxs_length    = 0;
        for (i = 0; i < event_ptr->u.sv.ar_connect->nr_of_iocrs; i++)
        {
            if(event_ptr->u.sv.ar_connect->iocr[i].iocr_type == CM_IOCR_TYPE_INPUT || event_ptr->u.sv.ar_connect->iocr[i].iocr_type == CM_IOCR_TYPE_MULTICAST_PROVIDER)
            {
                /* provider */
                pndv_data.ar[ar_idx].input_cr_io_data_length += event_ptr->u.sv.ar_connect->iocr[i].sum_iodata;
                pndv_data.ar[ar_idx].input_cr_ioxs_length    += event_ptr->u.sv.ar_connect->iocr[i].sum_ioxs;

            }
            else if(event_ptr->u.sv.ar_connect->iocr[i].iocr_type == CM_IOCR_TYPE_OUTPUT || event_ptr->u.sv.ar_connect->iocr[i].iocr_type == CM_IOCR_TYPE_MULTICAST_CONSUMER)
            {
                pndv_data.ar[ar_idx].output_cr_io_data_length += event_ptr->u.sv.ar_connect->iocr[i].sum_iodata;
                pndv_data.ar[ar_idx].output_cr_ioxs_length    += event_ptr->u.sv.ar_connect->iocr[i].sum_ioxs;
            }
        }

        pndv_data.ar[ar_idx].ar_nr          = event_ptr->ar_nr;
        pndv_data.ar[ar_idx].session_key    = event_ptr->session_key;
        pndv_data.ar[ar_idx].ar_set_nr      = CM_SV_SESSION_KEY_TO_ARSET_NR(event_ptr->session_key);

        pndv_data.ar[ar_idx].ar_set_trigger_running = PNIO_FALSE;

        pndv_data.ar[ar_idx].ar_uuid        = event_ptr->u.sv.ar_connect->ar_uuid;
        pndv_data.ar[ar_idx].cmi_obj_uuid   = event_ptr->u.sv.ar_connect->cmi_obj_uuid;

        pndv_data.ar[ar_idx].host_ip        = event_ptr->u.sv.ar_connect->host_ip;
        pndv_data.ar[ar_idx].host_name      = event_ptr->u.sv.ar_connect->cmi_station_name;

        #ifdef PNDV_CFG_ENABLE_RS_INFO_BLOCK_SUPPORT
        pndv_data.ar[ar_idx].has_RSInfoBlock      = event_ptr->u.sv.ar_connect->has_RSInfoBlock;
        #endif

        pndv_data.ar[ar_idx].send_clock     = event_ptr->u.sv.ar_connect->send_clock_31250ns;       // needed by devkits
        if (event_ptr->u.sv.ar_connect->nr_of_iocrs <= PNDV_MAX_IOCR_PER_AR)
        {
            for (i = 0; i < event_ptr->u.sv.ar_connect->nr_of_iocrs; i++)
            {
                pndv_data.ar[ar_idx].reduction_ratio[i]= event_ptr->u.sv.ar_connect->iocr[i].reduction_ratio;   // needed by devkits
            }
        }
        else
        {
            pndv_in_write_debug_buffer_all__(PNDV_DC_IOCR_MAX_NUM_EXCEED, event_ptr->u.sv.ar_connect->nr_of_iocrs );
        }

        if (event_ptr->u.sv.ar_connect->ARFSUBlock_ptr)
        {
            pndv_ds_check_arfsu_data_adjust((PNIO_UINT8*)event_ptr->u.sv.ar_connect->ARFSUBlock_ptr, &pndv_data.ar[ar_idx].ar_fsu_enable, pndv_data.ar[ar_idx].ar_fsu_uuid);
        }

        pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_CONNECT_IND);

    }
    else
    {
        /* call within other states not allowed */
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.ar[ar_idx].sm_state );
    }
}


/**
 *  @brief check consistancy of iocr data and request com mem
 *  @ingroup pndv_ar_connect
 *
 *  @param[in]  ar_idx ar index
 *  @param[out] event_ptr       event pointer
 *  @param[out] ar_set_nr       ar set number
 *  @param[out] session_key     session key
 *  @return     returns PNDV_CHECK_IND_RET_OK or PNDV_CONNECT_IND_RET_ERROR_RESOURCE
 *
 *  checks the iocr of a new ar and allocates communication memory for it, if needed
 *
 */
PNDV_CONNECT_IND_RET_VAL_T pndv_ar_tool_connect_ind_handle_iocr(PNIO_UINT32 ar_idx, PNIO_UINT16 ar_set_nr, PNIO_UINT16 session_key, CM_UPPER_EVENT_PTR_TYPE event_ptr)
{
    PNDV_CONNECT_IND_RET_VAL_T      ret_val = PNDV_CONNECT_IND_RET_OK;

    if( !event_ptr )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    if( ar_idx == PNDV_AR_IDX_NOT_USED )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);
    }

    if( ar_idx > (PNDV_CM_AR_NO - 1) )
    {
        /* only one permitted ar (and none that is currently being dismantled) may come here */
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);
    }

    /* ----------------- rt-io ar ------------------------------*/
    /* only read iocr list with the first check indication of rt-io ar's */
    if (    ( PNDV_AR_SM_SESSION_START == pndv_data.ar[ar_idx].sm_state )
         && (    ( CM_AR_TYPE_SINGLE        == pndv_data.ar[ar_idx].ar_type )
              || ( CM_AR_TYPE_SINGLE_SYSRED == pndv_data.ar[ar_idx].ar_type )
              || ( CM_AR_TYPE_SINGLE_RTC3   == pndv_data.ar[ar_idx].ar_type )
              || ( CM_AR_TYPE_SINGLE_RTC3   == pndv_data.ar[ar_idx].ar_type )
              || (    ( CM_AR_TYPE_SUPERVISOR  == pndv_data.ar[ar_idx].ar_type )
                   && ( !(CM_AR_PROP_DEVICE_ACCESS & pndv_data.ar[ar_idx].ar_properties) )
                 )
            )
       )
    {
        PNIO_UINT32 i;
        for (i = 0; i < event_ptr->u.sv.ar_connect->nr_of_iocrs; i++)
        {
            CM_SV_AR_CONNECT_IOCR_TYPE* local_iocr_ptr;
            PNIO_UINT32                 ret_value;

            local_iocr_ptr = &event_ptr->u.sv.ar_connect->iocr[i];

            ret_value  = PNDV_ALLOCATE_IOCR_MEMORY(ar_idx, ar_set_nr, session_key, local_iocr_ptr);

            if (ret_value == PNIO_FALSE)
            {
                /* iocr could not be served with a resource */
                if (i > 0)
                {
                    PNDV_FREE_IOCR_MEMORY(ar_idx);
                }
                ret_val = PNDV_CONNECT_IND_RET_ERROR_RESOURCE;
                break;
            }
        }

        for (i = 0; i < event_ptr->u.sv.ar_connect->nr_of_iocrs; i++)
        {
            CM_SV_AR_CONNECT_IOCR_TYPE* local_iocr_ptr;
            PNIO_UINT32                 ret_value;

            local_iocr_ptr = &event_ptr->u.sv.ar_connect->iocr[i];

            ret_value = PNDV_COMPLETE_IOCR_INFORMATION(ar_idx, ar_set_nr, session_key , local_iocr_ptr);

            pndv_unused__(local_iocr_ptr);

            if (ret_value == PNIO_FALSE)
            {
                /* error during iocr completion */
                ret_val = PNDV_CONNECT_IND_RET_ERROR_RESOURCE;
                break;
            }
        }
    }
    /* ----------------- supervisor ar with device access -----------*/
    else if (    ( PNDV_AR_SM_SESSION_START == pndv_data.ar[ar_idx].sm_state)
              && ( CM_AR_TYPE_SUPERVISOR    == pndv_data.ar[ar_idx].ar_type)
              && ( CM_AR_PROP_DEVICE_ACCESS & pndv_data.ar[ar_idx].ar_properties)
            )
    {
        /* nothing to do */
    }

    return ret_val;
}


/**
 *  @brief process parked connect ind
 *
 *  This function must only be called on startup when state of peri
 *  bus changes from "Unknown" to any other state.
 *  Therefor parked ar's will be processed now.
 *
 */
PNIO_VOID pndv_ar_tool_connect_ind_process_parked()
{
    CM_UPPER_RQB_PTR_TYPE tmp_rqb_ptr;
    PNIO_UINT32           ret_val;

    if (pndv_data.serv.con_serv_run == PNIO_TRUE)
    {
        /* parking connect.ind serivce was used */

        /* stop the timer */
        PNDV_STOP_TIMER(&ret_val, pndv_data.connect_indication_timeout_timer_id);
        LSA_UNUSED_ARG(ret_val);

        pndv_data.serv.con_serv_run = PNIO_FALSE;

        tmp_rqb_ptr = 0;
        pndv_ar_peri_service_get_next(&tmp_rqb_ptr, PNDV_PERI_SERVICE_IDENT_CON);
        while (tmp_rqb_ptr != 0)
        {
            //! there is a parked connect.ind
            /* process the indication */
            pndv_ar_cm_connect_ind(tmp_rqb_ptr);

            /* check for next parked indication */
            pndv_ar_peri_service_get_next(&tmp_rqb_ptr, PNDV_PERI_SERVICE_IDENT_CON);
        }
    }
}

/**
 *  @brief connect ind timeout
 *
 *  This function triggers the processing of a delayed connect ind.
 *  Reason for a delayed processing is giving some time to peri to
 *  get ready. This avoids connect polling at startup.
 *
 *  @attention This function is meant to be a method of mail to get timeout into right task context.
 *
 */
PNIO_VOID pndv_ar_sys_connect_ind_timeout()
{
    pndv_ar_tool_connect_ind_process_parked();
}
//@}

//! @name Disconnect
//@{
/**
 *  @brief an ar goes offline
 *  @ingroup pndv_ar_disconnect
 *
 *  @param[in]  rqb_ptr request block pointer
 *
 *  long_description
 *
 */
PNIO_VOID pndv_ar_cm_ar_disconnect_ind(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 ar_idx;

    /* ar_index ermitteln */

    ar_idx = pndv_ar_get_ar_idx_by_session_key( rqb_ptr->args.pd.event->session_key );

    if (PNDV_AR_IDX_NOT_USED == ar_idx)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, (PNIO_UINT32)rqb_ptr->args.pd.event->session_key );    /*NOTREACHED*/
    }

    pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_AR_DISCONNECT_IND,
                                         (PNIO_UINT16)rqb_ptr->args.pd.event->u.sv.ar_disconnect->disconnect.reason_code, ar_idx);

    //! save rqb_ptr if request needs to wait
    pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr = rqb_ptr;

    pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_DISCONNECT_IND);

}

/**
 *  @brief processing disconnect.ind by peri was finished
 *  @ingroup pndv_ar_disconnect
 *  - is called on disconnect.ind from CM
 *  - is called in case of negative connect.rsp
 *
 *  @param[in]  ar_idx Index of the ar
 *
 *  long_description
 *
 */
PNIO_VOID pndv_ar_peri_disconnect_ind_done(PNIO_UINT8 ar_idx)
{
    //! nothing to do
    //pndv_data.ar[ar_idx].ar_abort_running = PNIO_FALSE; /* AR-Abort ist durch */

     /** @note this must be done synchron now, peri must not use kram after disconnect_ind,
      *  so no buffer should be locked
      */
    if (ar_idx < PNDV_MAX_ARS_RT)
    {
        /* free memory only for RT-AR's */
        PNDV_FREE_IOCR_MEMORY(ar_idx);
    }

    /* z / s is allowed again after AR dismantling -> possibly jammed req. */
    pndv_in_cm_check_dev_req();

    /* DiAl were jammed between app_ready and InData - possibly InData was not reached after ApplReady
       -> now it goes on */
    /* check for parked alarms and diags */
    pndv_in_al_check_dial_continue();
    pndv_in_al_check_rosal_queue();

    /* must be called after setting the AR state */
    pndv_ar_tool_disconnect_ind_control_bf_led();

    if ( ar_idx < PNDV_MAX_ARS_RT )
    {
        PNIO_UINT32 local_ar_idx;

        /* SO Locked can only be set for user data ARs
           -> Therefore only reset from ARS with ar_idx < PNDV_MAX_ARS_RT
           -> ar_so_locked_state has dimension PNDV_MAX_ARS_RT */

        if (PNDV_SOL_EVENT_FLAG_MOD_ID_HEAD & pndv_data.ar_so_locked_state[ar_idx])
        {
            /* SO_Locked ist aktiv -> STATE fuer Kopf kann aufgehoben werden */
            pndv_ar_set_so_state(PNDV_SOL_EVENT_FLAG_MOD_ID_HEAD, PNIO_FALSE, ar_idx);
        }

        if (PNDV_SOL_EVENT_FLAG_LOCKED_BY_STACK & pndv_data.ar_so_locked_state[ar_idx])
        {
            /* ar was locked by cm, flag must be cleared */
            pndv_ar_set_so_state(PNDV_SOL_EVENT_FLAG_LOCKED_BY_STACK, PNIO_FALSE, ar_idx);
        }

        if (PNDV_SOL_EVENT_FLAG_SHARED_NO_REMA & pndv_data.ar_so_locked_state[ar_idx])
        {
            /* clear this lock flag of this ar */
            pndv_ar_set_so_state( PNDV_SOL_EVENT_FLAG_SHARED_NO_REMA, PNIO_FALSE, ar_idx );
        }

        if (PNDV_SOL_EVENT_FLAG_PDEV_FAULT & pndv_data.ar_so_locked_state[ar_idx])
        {
            /* clear this lock flag of this ar */
            pndv_ar_set_so_state( PNDV_SOL_EVENT_FLAG_PDEV_FAULT, PNIO_FALSE, ar_idx );
        }

        /* an rt ar is offline now, check if any other ar was locked by this one */
        for (local_ar_idx=0;local_ar_idx<PNDV_MAX_ARS_RT ;local_ar_idx++ )
        {
            /* clear the lock flag of this ar in any other ar */
            pndv_ar_set_so_state( ((PNIO_UINT32)(1 << ar_idx)), PNIO_FALSE, local_ar_idx );
        }
    }

    pndv_data.ar[ar_idx].no_prm_end_ind     = PNIO_FALSE;
    pndv_data.ar[ar_idx].ar_abort_requested = PNIO_FALSE;
    pndv_data.ar[ar_idx].pndv_ar_type   = PNDV_AR_TYPE_UNKNOWN;
    if(pndv_data.cfg.akt_isom_ar_idx == ar_idx)
    {
        pndv_data.cfg.prev_isom_ar_idx = pndv_data.cfg.akt_isom_ar_idx;
        pndv_data.cfg.akt_isom_ar_idx = PNDV_AR_IDX_NOT_USED;

        pndv_data.cfg.mode_isom = PNIO_FALSE;
#ifdef PNDV_CFG_ISOM_NO_SHARED
        {
            PNIO_UINT32 local_ar_idx;

            for (local_ar_idx=0;local_ar_idx<PNDV_MAX_ARS_RT ;local_ar_idx++ )
            {
                /* clear the lock_by_isom flag in any other ar */
                pndv_ar_set_so_state( PNDV_SOL_EVENT_FLAG_LOCKED_BY_ISOM, PNIO_FALSE, local_ar_idx );
            }
        }
#endif
    }

#ifdef PNDV_CFG_ENABLE_RECORD_RESPONDER
    if ( PNDV_RD_IO_RECORD_STATE_COPY_REQ == pndv_data.rd_io.rd_io_req.state )
    {
        /* Data can no longer be acknowledged */
        pndv_in_dfc_rd_io_record_fill( NIL, 0, pndv_data.rd_io.rd_io_req.rqb_ptr, NIL );
        pndv_in_dfc_rd_io_record_done( pndv_data.rd_io.rd_io_req.rqb_ptr );
    }
#endif

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_AR_DISCONNECT_IND_DONE, ar_idx);

    if (pndv_data.ar[ar_idx].ar_set_nr != 0)
    {
        PNIO_UINT32 sr_set_idx;
        // this ar is part of a sysred ar_set
        sr_set_idx = pndv_ar_get_sr_idx_by_ar_set_nr(pndv_data.ar[ar_idx].ar_set_nr);
        if (sr_set_idx == PNDV_SR_SET_IDX_NOT_USED)
        {
            // must not be here
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }

        pndv_ar_sr_sm(sr_set_idx, PNDV_SR_EVENT_REMOVE_FROM_ARSET, ar_idx);

    }

    pndv_ar_tool_disconnect_ind_cleanup(ar_idx);
    pndv_ar_tool_disconnect_ind_reset_set_cfg(ar_idx);

    pndv_data.ar[ar_idx].ar_nr  = PNDV_AR_NR_NOT_USED;

    pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_DISCONNECT_IND_DONE);

    if (pndv_ar_get_num_connected_ars() == 0)
    {
        /* tell the user if no more ARs are established. Use case: During RTF, the PNIO stack aborts all ARs
         * and the device has to wait for this before it resets itself. */
        // Indicate All ARs are Gone not implemented
    }

    pndv_in_al_check_diag();
}

/**
 *  @brief control bf led with disconnect ind
 *  @ingroup pndv_ar_disconnect
 *
 *  checks if the bf led has to set with this offline ind
 *
 */
PNIO_VOID pndv_ar_tool_disconnect_ind_control_bf_led ( PNIO_VOID )
{
    PNIO_BOOL   ar_in_data;
    PNIO_UINT32 tmp_ar_idx;

    ar_in_data = PNIO_FALSE;

    /* Affect BF led only with a regular AR */

    for( tmp_ar_idx = 0; tmp_ar_idx < PNDV_MAX_ARS_RT; tmp_ar_idx++ )
    {
        /* is a regular AR in the IN_DATA state? */

        if(  (PNDV_AR_SM_IN_DATA  == pndv_data.ar[tmp_ar_idx].sm_state )
           ||(PNDV_AR_SM_IN_DATA_RTC3 == pndv_data.ar[tmp_ar_idx].sm_state )
           )
        {
            ar_in_data = PNIO_TRUE;
        }
    }

    if (   (PNIO_FALSE == ar_in_data )
        && (PNIO_TRUE  == pndv_data.ar_in_data_led_used )
       )
    {

        PNDV_LED_CTRL_DATA_AR_OFFLINE();

        pndv_data.ar_in_data_led_used = PNIO_FALSE;
    }
}

/**
 *  @brief do some cleanup work after ar is disconnected
 *  @ingroup pndv_ar_disconnect
 *
 *  @param[in]  ar_idx AR index
 *
 *  long_description
 *
 */
PNIO_VOID pndv_ar_tool_disconnect_ind_cleanup(PNIO_UINT32 ar_idx)
{
    PNIO_UINT32 sr_set_idx,
                ar_is_owner,
                sr_remains = PNIO_FALSE,
                remaining_ar_idx = 0;

    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;


    if ( PNDV_MAX_ARS_RT > ar_idx  ) /* we have a payload AR */
    {
        /* Delete RT class and delete tmp-buffer_ptr */
        pndv_data.iface_ptr->ar[ar_idx].current_rt_class  = PNDV_RT_NONE;
        pndv_data.iface_ptr->ar[ar_idx].red_int_info_used = PNIO_FALSE;

        /* Workaround ES-Bus technology modules AP00769528 */
        /**************************************************/
        /*
            If the isochronous RTC3-AR does not get up properly, 
            it failing before the RTC3 and goes down again, 
            the isochronous parameterization comes again without the modules getting a GC. 
            1SSI and 1COUNT take this from us and refuse to serve. 
            Therefore GC in this state once
        */
        if (    (PNIO_TRUE                == pndv_data.cfg.mode_isom)    /* taktsync. RTC3-AR */
             && (PNDV_SOL_NO_EVENT        == pndv_data.ar_so_locked_state[ar_idx])
             && (PNDV_AR_SM_W_PRM_END_IND <= pndv_data.ar[ar_idx].sm_state)
             && (PNDV_AR_SM_W_RIR_IND     >= pndv_data.ar[ar_idx].sm_state)
           )

        {
            PNDV_IFACE_CMD_ENTRY_T tmp_event;

            tmp_event.cmd = PNDV_EV_TO_PERI_ONESHOT_GC;
            pndv_in_peri_write_coupl_event( tmp_event );
        }
    }

    if( PNDV_AR_IDX_NOT_USED == ar_idx )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx );    /*NOTREACHED*/
    }

    /* RT_Class ruecksetzen */
    pndv_data.ar[ar_idx].current_rt_class         = PNDV_RT_NONE;

    pndv_data.ar[ar_idx].input_cr_io_data_length  = 0;
    pndv_data.ar[ar_idx].input_cr_ioxs_length     = 0;
    pndv_data.ar[ar_idx].output_cr_io_data_length = 0;
    pndv_data.ar[ar_idx].output_cr_ioxs_length    = 0;
    pndv_data.ar[ar_idx].send_clock               = 0;
    PNDV_MEMSET(pndv_data.ar[ar_idx].reduction_ratio, 0, sizeof(pndv_data.ar[ar_idx].reduction_ratio));
    /* Reset prm end counter, needed for disconnect between prm_end and appl_ready (TFS RQ 1219116) */
    pndv_data.ar[ar_idx].prm_end_elem_cnt  = 0;
    pndv_data.ar[ar_idx].empty_prm_end_ind = PNIO_FALSE;
    pndv_data.ar[ar_idx].pdev_8000_locked  = PNIO_FALSE;
    pndv_data.ar[ar_idx].pdev_9000_locked  = PNIO_FALSE;

    if (pndv_data.ar[ar_idx].ar_set_nr != 0)
    {
        // ar is part of an ar-set (sysred)
        sr_set_idx = pndv_ar_get_sr_idx_by_ar_set_nr(pndv_data.ar[ar_idx].ar_set_nr);
        if( PNDV_SR_SET_IDX_NOT_USED == sr_set_idx )
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.ar[ar_idx].ar_set_nr );    /*NOTREACHED*/
        }
        if(  (pndv_data.sr[sr_set_idx].sm != PNDV_SR_STATE_W_OFFLINE_DONE) // last ar of a set is going down
           &&(pndv_data.sr[sr_set_idx].sm != PNDV_SR_STATE_ABORT)          // set is being aborted, clear can be done with the first ar going down
           )
        {
            // find another ar that belongs to this ar set, first ar wins
            for( remaining_ar_idx = 0; remaining_ar_idx <= PNDV_CM_AR_NO; remaining_ar_idx++ )
            {
                    if(  (pndv_data.ar[ar_idx].ar_set_nr == pndv_data.ar[remaining_ar_idx].ar_set_nr )
                       &&(ar_idx != remaining_ar_idx)
                      )
                    {
                            break;
                    }
            }

            if( PNDV_CM_AR_NO < remaining_ar_idx )
            {
                // should not be possible here
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
            }

            // there is another ar in this ar-set
            sr_remains = PNIO_TRUE;
        }
        else
        {
            // last ar of the set or abort is going down
            sr_remains = PNIO_FALSE;
        }
    }

    for (PNDV_LIST_EACH(local_sub_res, &pndv_data.cfg.peri_cfg.in_use_list, PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T))
    {
        if (local_sub_res->own_ar_idx == ar_idx)
        {
            ar_is_owner = PNIO_TRUE;
        }
        else
        {
            ar_is_owner = PNIO_FALSE;
        }

        if (ar_is_owner == PNIO_TRUE)
        {
            // disconnected ar was owner
            if (sr_remains != PNIO_TRUE)
            {
                // ar was not in a set or last ar of a set was disconnected
                if (local_sub_res->res_state > PNDV_CFG_SUBMODULE_RES_STATE_W_OWN)
                {

                    local_sub_res->own_ar_idx       = PNDV_AR_IDX_NOT_USED;
                    local_sub_res->ar_set_nr        = PNDV_SR_SET_NR_NOT_USED;;
                    local_sub_res->submodule_state  = PNDV_SUBMODULE_STATE_NO_OWNER;
                    local_sub_res->para_error       = PNIO_FALSE;
                    pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_RELEASE); //!< release from onwnership
                }
            }
            else
            {
                //if there remains a ar in a ar_set, change ownership
                if (local_sub_res->ar_set_nr == pndv_data.ar[ar_idx].ar_set_nr)
                {
                    if (local_sub_res->res_state > PNDV_CFG_SUBMODULE_RES_STATE_W_PRM_END)
                    {
                        /* EnRa: if there is another AR (sr_remains = PNIO_TRUE) and the dismantled AR was owner (ar_is_owner == PNIO_TRUE) 
                         * then this AR must have been Appl-Ready, otherwise the CM would not let a 2nd AR through
                         *
                         * --> it is checked for> PNDV_CFG_SUBMODULE_RES_STATE_W_PRM_END.
                         *     This takes into account one case:
                         *     - a submodule is added in a backup AR (first gets the owner)
                         *     - Submodule is not parameterized because backup
                         *     - this AR is dismantled again -> ownership must now be deactivated again
                         *
                         */


                        //trigger sm for every element that moves ownership
                        pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_CHANGE_OWNER);
                        pndv_in_write_debug_buffer_3__(PNDV_DC_COMMON_MODUL_INFO,
                                                       local_sub_res->sub_module.slot_nr,
                                                       local_sub_res->sub_module.subslot_nr,
                                                       ((PNIO_UINT32)local_sub_res->sub_module.mod_ident) | ((PNIO_UINT32)local_sub_res->sub_module.sub_ident) << 16
                                                       );
                        local_sub_res->own_ar_idx = remaining_ar_idx;
                    }
                    else if(local_sub_res->res_state > PNDV_CFG_SUBMODULE_RES_STATE_W_OWN)
                    {
                        // applies to state PNDV_CFG_SUBMODULE_RES_STATE_W_OWN_PASSIV (R1 case, backup side is disconnected)
                        local_sub_res->own_ar_idx      = PNDV_AR_IDX_NOT_USED;
                        local_sub_res->ar_set_nr       = PNDV_SR_SET_NR_NOT_USED;;
                        local_sub_res->submodule_state = PNDV_SUBMODULE_STATE_NO_OWNER;
                        local_sub_res->para_error      = PNIO_FALSE;
                        pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_RELEASE); //!< release from onwnership
                    }
                }
            }
        }
        else
        {
            // sub not owned by this ar
            // normaly do nothing...
            if (   (pndv_data.ar[ar_idx].ar_set_nr != 0)
                && (pndv_data.ar[ar_idx].ar_set_nr == local_sub_res->ar_set_nr)
                && (sr_remains                     == PNIO_FALSE)
                )
            {
                // ...but this submodule belongs to an R1-AR-Set in backup state
                if(local_sub_res->res_state > PNDV_CFG_SUBMODULE_RES_STATE_W_OWN)
                {
                    local_sub_res->own_ar_idx      = PNDV_AR_IDX_NOT_USED;
                    local_sub_res->ar_set_nr       = PNDV_SR_SET_NR_NOT_USED;;
                    local_sub_res->submodule_state = PNDV_SUBMODULE_STATE_NO_OWNER;
                    local_sub_res->para_error      = PNIO_FALSE;
                    pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_RELEASE); //!< release from onwnership
                }
            }
        }
    }
}


/**
 *  @brief clears config info of one ar
 *  @ingroup pndv_ar_disconnect
 *
 *  @param[in]  ar_nr ar number
 *
 *  clears info about submodules of an ar within the interface structure
 *  and some infos held in pndv_data
 *
 *
 */
PNIO_VOID pndv_ar_tool_disconnect_ind_reset_set_cfg(PNIO_UINT32 ar_idx)
{
    PNIO_UINT16 entity_nr;

    pndv_peri_reset_entity_admin_by_ar_idx(ar_idx);

    for ( entity_nr = 0; entity_nr < PNDV_CM_SV_SUBSLOT_COUNT; entity_nr++ )
    {
        // clean set cfg in any case
        PNDV_MEMSET((PNIO_VOID *)&pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr],0, sizeof(PNDV_SET_CFG_T));
    }

    pndv_in_check_led_info();
}


/**
 *  @brief init config info of all ar
 *  @ingroup pndv_ar_disconnect
 *
 *  clears info about submodules of an ar within the interface structure
 *  and some infos held in pndv_data
 *
 */
PNIO_VOID pndv_ar_init_set_cfg(PNIO_VOID)
{
    PNIO_UINT32 ar_idx;

    for ( ar_idx = 0; ar_idx <= PNDV_CM_AR_NO ; ar_idx++ )  /* <= so that too much AR can be terminated cleanly */
    {
        /* Reset RT_Class */
        pndv_data.ar[ar_idx].current_rt_class      = PNDV_RT_NONE;

        if(ar_idx < PNDV_CM_AR_NO)
        {
            PNIO_UINT16 entity_nr;

            for (entity_nr = 0; entity_nr < PNDV_CM_SV_SUBSLOT_COUNT; entity_nr++ )
            {
                PNDV_MEMSET((PNIO_VOID *)&pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr],0, sizeof(PNDV_SET_CFG_T));
            }
        }
    }
}


/**
 *  @brief abort an ar
 *  @ingroup pndv_ar_disconnect
 *
 *  @param[in]     ar_idx      Index of AR to be aborted
 *  @param[in]  call_line   Line of function call
 *  @return        PNDV_OK Abort ok, PNDV_ERR_SEQUENCE Sequenc error (rqb is in use), PNDV_OK_ASYNC Abort was send to cm
 *
 *  Request an abort of a single ar. This service does not involve the ar_sm, it's only a trigger for cm to disconnect an ar (disconnect.ind).
 *
 *  @remark ar_abort_running, ar_abort_req removed. Argument: an ar abort is only possible after a connect.ind of an ar
 *  so sm state is bigger offline. If an abort req to this ar is already running there is no need to
 *  send a request again. If an abort request is already done by cm it is not harmfull to send one again,
 *  as long as the ar_sm state is bigger offline (no disconnect yet, session_key still valid).
 *
 */
PNIO_UINT32 pndv_ar_abort_req(PNIO_UINT32 ar_idx, PNIO_UINT32 call_line)
{
    CM_UPPER_RQB_PTR_TYPE tmp_rqb_ptr;
    PNIO_UINT32           ret_val;

    ret_val = PNDV_OK;

    if (   (PNDV_CM_AR_NO            >= ar_idx)    /* <= so that too much AR can be terminated cleanly */
            /* valid ar_idx */
        && (PNDV_AR_SM_W_OWNERSHIP_IND <= pndv_data.ar[ar_idx].sm_state)
       )
    {
        if (0 != PNDV_RQB_GET_OPCODE( &pndv_data.rqb.ar_abort[ar_idx]))
        {

            //! an abort request for this ar is already running, ignore this one
            ret_val = PNDV_OK_DONE;
        }
        else
        {
            pndv_data.rqb.ar_abort[ar_idx].args.sv.ar_abort->device_nr   = PNDV_CM_DEVICE_NO;
            pndv_data.rqb.ar_abort[ar_idx].args.sv.ar_abort->session_key = pndv_data.ar[ar_idx].session_key;

            pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_AR_ABORT, (PNIO_UINT16)ar_idx, call_line);

            PNDV_RQB_SET_HANDLE(&pndv_data.rqb.ar_abort[ar_idx], pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
            PNDV_RQB_SET_OPCODE(&pndv_data.rqb.ar_abort[ar_idx], CM_OPC_SV_AR_ABORT);

            tmp_rqb_ptr = &pndv_data.rqb.ar_abort[ar_idx];

            ret_val = PNDV_OK_ASYNC;

            PNDV_REQUEST(tmp_rqb_ptr, LSA_COMP_ID_CM);

        }
    }
    else
    {
        //! this ar is not up or waiting for connect ind done
        ret_val = PNDV_ERR_SEQUENCE;
    }

    return(ret_val);
}



/**
 *  @brief an abort request has been confirmed by cm
 *  @ingroup pndv_ar_disconnect
 *
 *  @param[in]     rqb_ptr pointer to request block
 *
 *  ar abort is a set and forget service, there is no need to do anything with the confirmation
 *  except freeing the rqb by setting opcode to zero and trace
 *
 */
PNIO_VOID pndv_ar_cm_ar_abort_done (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response,
                ar_idx;

    response = PNDV_RQB_GET_RESPONSE( rqb_ptr);

    if (   (CM_OK           != response) //!< first abort of an ar
        && (CM_OK_CANCELLED != response) //!< repeated abort of an ar
       )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, response);
    }

    ar_idx = pndv_ar_get_ar_idx_by_session_key( rqb_ptr->args.sv.ar_abort->session_key );

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_AR_ABORT_DONE, (PNIO_UINT16)ar_idx);

    if( PNDV_AR_IDX_NOT_USED == ar_idx )
    {
        //! this can only happen if a repeated abort request has been overtaken by a disconnect
        //! its not generally an error
    }
    else
    {

        PNDV_RQB_SET_OPCODE( &pndv_data.rqb.ar_abort[ar_idx], 0);

    }
}


/*****************************************************************************/


/**
 *  @brief search and abort ar's marked as shared
 *  @ingroup pndv_ar_disconnect
 *
 *
 */
PNIO_VOID pndv_ar_abort_shared_ar (PNIO_VOID)
{
    /* Station parameters with a PRIMARY_AR have changed 
       -> shared ARs have to be aborted and restarted */

    PNIO_UINT32 ar_idx;

    for (ar_idx = 0; ar_idx < PNDV_MAX_ARS_RT ; ar_idx++)
    {
        if(( PNDV_AR_SM_OFFLINE != pndv_data.ar[ar_idx].sm_state) && 
           ((PNDV_AR_TYPE_SHARED_WITHOUT_PDEV == pndv_data.ar[ar_idx].pndv_ar_type) ||
            (PNDV_AR_TYPE_SHARED_WITH_PDEV    == pndv_data.ar[ar_idx].pndv_ar_type)))
        {
            pndv_ar_abort_req(ar_idx, (PNIO_UINT32)__LINE__ );
        }
    }
}
//@}

//! @name Ownership
//@{
/**
 *  @brief ownership ind for one or more submodules
 *  @ingroup pndv_ar_ownership
 *
 *  @param[in]  rqb_ptr pointer to request block
 *
 *  fills the info about all submodules of this ar into the interface
 *
 */
PNIO_VOID pndv_ar_cm_sv_ownership_ind( CM_UPPER_RQB_PTR_TYPE rqb_ptr )
{
    PNIO_UINT32 ar_idx;
    CM_UPPER_SV_AR_OWNERSHIP_PTR_TYPE local_ar_ownership;

    if (PNDV_RQB_GET_OPCODE( rqb_ptr) != CM_OPC_SV_AR_OWNERSHIP_IND)
    {
        //! only ownership.ind can be processed here
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE( rqb_ptr) );
    }

    if (pndv_data.serv.own_serv_run == PNIO_FALSE)
    {
        //! no ownership ind running

        ar_idx = pndv_ar_get_ar_idx_by_session_key( rqb_ptr->args.sv.ar_event->session_key );

        //! save rqb_ptr for async response
        pndv_data.ar[ar_idx].own_ind_rqb_ptr = rqb_ptr;

        local_ar_ownership = rqb_ptr->args.sv.ar_event->u.sv.ar_ownership;

        switch (local_ar_ownership->ar_context)
        {
            case CM_SV_AR_CONTEXT_CONN:
            {
                if ( pndv_data.ar[ar_idx].sm_state != PNDV_AR_SM_W_OWNERSHIP_IND)
                {
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.ar[ar_idx].sm_state );
                }

                //! send ownership event to sm (only in context of connect)
                pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_OWNERSHIP_IND);

                if (pndv_data.ar[ar_idx].ar_type == CM_AR_TYPE_SINGLE_RTC3)
                {
                    pndv_data.ar[ar_idx].current_rt_class = PNDV_RT_CLASS_3;
                }
                else
                {
                    /* all other ar types   CM_AR_TYPE_SINGLE, CM_AR_TYPE_SUPERVISOR, CM_AR_TYPE_SINGLE_SYSRED */
                    pndv_data.ar[ar_idx].current_rt_class = PNDV_RT_CLASS_1;
                }

                pndv_ar_tool_ownership_ind_check_plausibility(rqb_ptr);

                //local_ar_ownership->set_mrp_off         = LSA_FALSE;     //by default enable MRP, see ind_done

            }
            //! no break
            //lint -fallthrough
            case CM_SV_AR_CONTEXT_PLUG:
            {
                PNIO_UINT16 elem_cnt;
                PNIO_UINT32 entity_nr;

                for (elem_cnt = 0; elem_cnt < local_ar_ownership->nr_of_elems; elem_cnt++)
                {
                    PNDV_IFACE_CMD_ENTRY_T tmp_event;
                    CM_SV_AR_OWNERSHIP_ELEMENT_TYPE  *act_elem_ptr;
                    PNDV_SET_CFG_T  *set_cfg_elem_ptr;

                    act_elem_ptr = &(local_ar_ownership->elem[elem_cnt]);

                    entity_nr = PNDV_GET_ENTITY_NR(act_elem_ptr->api, act_elem_ptr->slot_nr, act_elem_ptr->subslot_nr, ar_idx);
                    if(entity_nr >= PNDV_CM_SV_SUBSLOT_COUNT)
                    {
                        pndv_in_fatal_error( PNDV_MODULE, __LINE__, entity_nr);
                    }
                    set_cfg_elem_ptr = &pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr];

                    //! copy ownership element to interface
                    PNDV_COPY_BYTE((PNIO_VOID *)&set_cfg_elem_ptr->elem.own_info, (PNIO_VOID *)act_elem_ptr, sizeof(CM_SV_AR_OWNERSHIP_ELEMENT_TYPE));

                    //! mark the submodule as being projected
                    set_cfg_elem_ptr->elem.ownership_passed = PNIO_TRUE;

                    if (set_cfg_elem_ptr->elem.own_info.owner_session_key == pndv_data.ar[ar_idx].session_key)
                    {
                        //! sub is owned by this ar
                        set_cfg_elem_ptr->flags |= PNDV_SET_CFG_FLAGS_OWN;
                    }
                    else
                    {
                        //! sub is not owned by this ar
                        set_cfg_elem_ptr->flags &=  ~((PNIO_UINT32) PNDV_SET_CFG_FLAGS_OWN);
                    }

                    // submodule ist ready by default, peri can force arp by setting the bit
                    set_cfg_elem_ptr->flags &=  ~((PNIO_UINT32) PNDV_SET_CFG_FLAGS_SET_ARP);

                    //! mark service to be in use
                    pndv_data.serv.own_serv_run = PNIO_TRUE;

                    set_cfg_elem_ptr->elem.own_state = PNDV_IFACE_SERVICE_NEW;
                    //! prepare interface command
                    tmp_event.add_1 = (PNIO_UINT8)  ar_idx;
                    tmp_event.add_2 = (PNIO_UINT16) entity_nr;
                    if (elem_cnt == (local_ar_ownership->nr_of_elems-1))
                    {
                        //! last sub in ownership list
                        tmp_event.cmd = PNDV_EV_TO_PERI_OWN_IND;
                    }
                    else
                    {
                        //! there are more subs in ownership list
                        tmp_event.cmd = PNDV_EV_TO_PERI_OWN_IND_MORE_FOLLOWS;
                    }

                    pndv_in_peri_write_coupl_event( tmp_event );
                    pndv_in_write_debug_buffer_3__(PNDV_DC_COMMON_MODUL_INFO,
                                                   pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.slot_nr,
                                                   pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.subslot_nr,
                                                   ((PNIO_UINT32)pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.mod_ident) | ((PNIO_UINT32)pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.sub_ident) << 16
                                                   );
                }

                break;
            }
            case CM_SV_AR_CONTEXT_DISC:
            {
                //! this context is dont care because the disconnect ind is used to clean up all data
                //! if subs go to another ar, a ownership.ind with context PLUG in this ar will follow anyway

                PNDV_RQB_SET_RESPONSE(pndv_data.ar[ar_idx].own_ind_rqb_ptr, CM_OK);

                PNDV_RQB_SET_HANDLE(pndv_data.ar[ar_idx].own_ind_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                PNDV_RQB_SET_OPCODE(pndv_data.ar[ar_idx].own_ind_rqb_ptr, CM_OPC_SV_AR_OWNERSHIP_RSP);

                PNDV_REQUEST(pndv_data.ar[ar_idx].own_ind_rqb_ptr, LSA_COMP_ID_CM);

                pndv_data.ar[ar_idx].own_ind_rqb_ptr = 0;

                break;
            }
            case CM_SV_AR_CONTEXT_PULLPDEV: /* something to do yet */
            default:
            {
                //! unknown context
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, local_ar_ownership->ar_context);
                break;
            }
        }
    }
    else
    {
        //! there is still a ownership indication running
        if (pndv_ar_do_service_queue(rqb_ptr, PNDV_PERI_SERVICE_IDENT_OWN) != PNDV_OK)
        {
            //! queueing not possible, no free resource, check PNDV_PERI_SERVICE_QUEUES_T elem
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }

    }
}

/**
 *  @brief ownership has been processed by peri
 *  @ingroup pndv_ar_ownership
 *
 *  @param[in]  ar_idx ar index
 *  @param[in]  entity_nr nr of the entity (submodule description block)
 *  @param[in]  more_follows PNIO_TRUE = more responses follow, PNIO_FALSE = last response
 *
 *  ownership responses have to come back in order of request
 *
 */
PNIO_VOID pndv_ar_peri_ownership_ind_done(PNIO_UINT8 ar_idx, PNIO_UINT16 entity_nr, PNIO_BOOL more_follows)    /*lint -esym(661,elem_cnt) */
{
    CM_UPPER_RQB_PTR_TYPE   rqb_ptr;
    PNIO_UINT32             elem_cnt;
    PNIO_UINT32             ar_is_second_of_r1_set;
    CM_UPPER_SV_AR_OWNERSHIP_PTR_TYPE local_ar_ownership;
    CM_SV_AR_OWNERSHIP_ELEMENT_TYPE*  local_ar_ownership_elem;

    rqb_ptr = pndv_data.ar[ar_idx].own_ind_rqb_ptr;

    if (rqb_ptr == 0)
    {
        // state conflict, no ownership req in process
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx );
    }


    elem_cnt = pndv_data.ar[ar_idx].own_rsp_count;
    if (pndv_data.ar[ar_idx].own_rsp_count == 0)
    {
        // first call
    }

    local_ar_ownership = rqb_ptr->args.sv.ar_event->u.sv.ar_ownership;

    if (elem_cnt > local_ar_ownership->nr_of_elems)
    {
        // more responses than requests
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, elem_cnt );
    }

    local_ar_ownership_elem = &local_ar_ownership->elem[elem_cnt];

    /*
     * evaluate SO-Locked in case of wrong head submodule
     */
    switch (pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.cmp_result)
    {
        case PNDV_CMP_RES_OK:
        case PNDV_CMP_RES_OK_CHANGE:
        case PNDV_CMP_RES_OK_SUBSTITUTED:
        {
            local_ar_ownership_elem->is_wrong = LSA_FALSE;
            // at least one module is ok
            break;
        }
        case PNDV_CMP_RES_ERROR:            /* check for PDEV error -> SOL in case of error */
        {
            if (  (local_ar_ownership_elem->slot_nr == PNDV_IM_SLOT_NO)
#ifdef IM_SUPPORT_PN_RX_REDUNDANCY
                ||(local_ar_ownership_elem->slot_nr == PNDV_IM_SLOT_NO_PARTNER)
#endif
               )
            {
            #ifdef PNDV_CFG_DISABLE_SOL_BY_PDEV
                if (!(   ((local_ar_ownership_elem->subslot_nr & 0xF000) == 0x8000)
                       ||((local_ar_ownership_elem->subslot_nr & 0xF000) == 0x9000)
                      )
                    )
            #endif
                {
                    /* if head is wrong ar must be locked */
                    if (   (elem_cnt != 0)
                         &&(!pndv_data.ar_so_locked_state[ar_idx])
                        )
                    {
                        PNIO_UINT32 local_cnt;
                        /* if head is not the first element and ar is not already locked
                         * all prior elems must also be so_locked now
                         */
                        for (local_cnt = 0; local_cnt < elem_cnt; local_cnt++)
                        {
                            if (local_ar_ownership->elem[local_cnt].owner_session_key == pndv_data.ar[ar_idx].session_key)
                            {
                                /* ownership was offered to the current ar*/
                                if (local_ar_ownership->elem[local_cnt].owner_session_key != 0)
                                {
                                    /* this element was already set to be owned */
                                    PNIO_UINT16  slot_nr;
                                    PNIO_UINT16  subslot_nr;
                                    PNIO_UINT32  subslot_error;
                                    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;
                                    local_ar_ownership->elem[local_cnt].owner_session_key = 0;


                                    //! find module in real config
                                    slot_nr    = local_ar_ownership->elem[local_cnt].slot_nr;
                                    subslot_nr = local_ar_ownership->elem[local_cnt].subslot_nr;
                                    pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
                                    if(subslot_error)
                                    {
                                        // slot/subslot not possible, must be an implementation error
                                        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
                                    }

                                    if(local_sub_res != 0)
                                    {
                                        //! submodule is currently bound by pndv
                                        if (  (local_ar_ownership->ar_context == CM_SV_AR_CONTEXT_CONN)
                                            ||(local_ar_ownership->ar_context == CM_SV_AR_CONTEXT_PLUG)
                                            ||(local_ar_ownership->ar_context == CM_SV_AR_CONTEXT_PULLPDEV)
                                           )
                                        {
                                            if(local_sub_res->res_state > PNDV_CFG_SUBMODULE_RES_STATE_W_OWN)
                                            {
                                                local_sub_res->own_ar_idx      = PNDV_AR_IDX_NOT_USED;
                                                local_sub_res->ar_set_nr       = PNDV_SR_SET_NR_NOT_USED;;
                                                local_sub_res->submodule_state = PNDV_SUBMODULE_STATE_NO_OWNER;
                                                local_sub_res->para_error      = PNIO_FALSE;
                                                pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_RELEASE); //!< release from ownership
                                            }
                                        }
                                    }
                                }
                            }
                        }

                    }
                    pndv_ar_set_so_state(PNDV_SOL_EVENT_FLAG_MOD_ID_HEAD, PNIO_TRUE, ar_idx);
                }
#ifdef PNDV_CFG_DISABLE_SOL_BY_PDEV
                else
                {
                    PNIO_UINT32 local_cnt;

                    if ((local_ar_ownership_elem->subslot_nr & 0xF000) == 0x8000)
                    {
                        pndv_data.ar[ar_idx].pdev_8000_locked = PNIO_TRUE;
                    }
                    else
                    {
                        pndv_data.ar[ar_idx].pdev_9000_locked = PNIO_TRUE;
                    }


                   /* if head is not the first element and ar is not already locked
                    * all prior elems must also be so_locked now
                    */
                   for (local_cnt = 0; local_cnt < elem_cnt; local_cnt++)
                   {
                       if (local_ar_ownership->elem[local_cnt].owner_session_key == pndv_data.ar[ar_idx].session_key)
                       {
                           /* ownership was offered to the current ar*/
                           if (   (local_ar_ownership->elem[local_cnt].owner_session_key != 0)
                                &&(local_ar_ownership->elem[local_cnt].slot_nr == (local_ar_ownership_elem->slot_nr))
                                &&((local_ar_ownership->elem[local_cnt].subslot_nr & 0xF000) == (local_ar_ownership_elem->subslot_nr & 0xF000))
                              )
                           {
                               /* this element was already set to be owned */
                               PNIO_UINT16 slot_nr;
                               PNIO_UINT16 subslot_nr;
                               PNIO_UINT32 subslot_error;
                               PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;
                               local_ar_ownership->elem[local_cnt].owner_session_key = 0;


                               //! find module in real config
                               slot_nr    = local_ar_ownership->elem[local_cnt].slot_nr;
                               subslot_nr = local_ar_ownership->elem[local_cnt].subslot_nr;
                               pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
                               if(subslot_error)
                               {
                                   // slot/subslot not possible, must be an implementation error
                                   pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
                               }

                               if(local_sub_res != 0)
                               {
                                   //! submodule is currently bound by pndv
                                   if (  (local_ar_ownership->ar_context == CM_SV_AR_CONTEXT_CONN)
                                       ||(local_ar_ownership->ar_context == CM_SV_AR_CONTEXT_PLUG)
                                       ||(local_ar_ownership->ar_context == CM_SV_AR_CONTEXT_PULLPDEV)
                                      )
                                   {
                                       if(local_sub_res->res_state > PNDV_CFG_SUBMODULE_RES_STATE_W_OWN)
                                       {
                                           local_sub_res->own_ar_idx       = PNDV_AR_IDX_NOT_USED;
                                           local_sub_res->ar_set_nr        = PNDV_SR_SET_NR_NOT_USED;;
                                           local_sub_res->submodule_state  = PNDV_SUBMODULE_STATE_NO_OWNER;
                                           local_sub_res->para_error       = PNIO_FALSE;
                                           pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_RELEASE); //!< release from onwnership
                                       }
                                   }
                               }
                           }
                       }
                   }
                }
#endif
            }
        }
        // no break
        //lint -fallthrough
        case PNDV_CMP_RES_NO_MODULE:
        {
            local_ar_ownership_elem->is_wrong = LSA_TRUE;
            break;
        }
        default:
        {
            // impossible result
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.cmp_result );
            break;
        }
    }

#ifdef PNDV_CFG_DISABLE_SOL_BY_PDEV
    if (pndv_data.ar[ar_idx].pdev_8000_locked == PNIO_TRUE)
    {
        if (  (  (local_ar_ownership_elem->slot_nr == PNDV_IM_SLOT_NO)
               ||(local_ar_ownership_elem->slot_nr == PNDV_IM_SLOT_NO_PARTNER)
               )
            &&((local_ar_ownership_elem->subslot_nr & 0xF000) == 0x8000)
           )
        {
            /* decline ownership for all pdev-submodule */
            local_ar_ownership_elem->owner_session_key = 0;
        }
    }

    if (pndv_data.ar[ar_idx].pdev_9000_locked == PNIO_TRUE)
    {
        if (  ( (local_ar_ownership_elem->slot_nr == PNDV_IM_SLOT_NO)
               ||(local_ar_ownership_elem->slot_nr == PNDV_IM_SLOT_NO_PARTNER)
               )
            &&((local_ar_ownership_elem->subslot_nr & 0xF000) == 0x9000)
           )
        {
            /* decline ownership for all pdev-submodule */
            local_ar_ownership_elem->owner_session_key = 0;
        }
    }
#endif

    /*
     * evaluate SO-locked, requested by application
     */

    //! here the submodule can be made so locked by setting owner_session_key = 0
    //! local_ar_ownership_elem->owner_session_key;
    if (  (!(pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].flags & PNDV_SET_CFG_FLAGS_OWN))
        )
    {
        if (local_ar_ownership_elem->owner_session_key == pndv_data.ar[ar_idx].session_key)
        {
            /* ownership was offered to the current ar, decline it */
            local_ar_ownership_elem->owner_session_key = 0;
            ar_is_second_of_r1_set = LSA_FALSE;
        }
        else
        {
            ar_is_second_of_r1_set = LSA_FALSE;
        }
    }
    else if (pndv_data.ar_so_locked_state[ar_idx])
    {
        /* AR shall completely be SO locked (in contrast to former implementation, interface+ports are no exception here (see SPH PDEV))*/
        local_ar_ownership_elem->owner_session_key = 0;
        ar_is_second_of_r1_set = LSA_FALSE;
    }
    else
    {
        /* redundancy is not of interrest here, ownership was offered and accepted */
        ar_is_second_of_r1_set = LSA_FALSE;
    }

    //! handle set_mrp_off here
    //! set_mrp_off=LSA_TRUE will result in an internal MRP_OFF record if no MRP-Record is send during parameter phase
    //! This should be dependent to the module-id of PNDV_IM_SLOT_NO. Only None-MRP module-id's should turn off MRP
    if (  (pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.slot_nr == PNDV_IM_SLOT_NO)
        &&(((pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].flags & PNDV_SET_CFG_FLAGS_MRP_OFF)))
        )
    {
        // if bit is set in any of the IM_SLOT subslots, disable MRP
        local_ar_ownership->set_mrp_off         = LSA_TRUE;
    }



    pndv_in_write_debug_buffer_3__(PNDV_DC_AR_OWNERSHIP_RESP, pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.cmp_result, local_ar_ownership_elem->owner_session_key, pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].flags);
    pndv_in_write_debug_buffer_3__(PNDV_DC_COMMON_MODUL_INFO,
                                   pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.slot_nr,
                                   pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.subslot_nr,
                                   ((PNIO_UINT32)pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.mod_ident)|((PNIO_UINT32)pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.sub_ident)<<16
                                   );

    pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_state = PNDV_IFACE_SERVICE_IDLE;


    /*
     * update owner-AR-Index, Ar-Set and submodule state for this submodule
     */
    if (  (local_ar_ownership_elem->owner_session_key == pndv_data.ar[ar_idx].session_key)
        ||(ar_is_second_of_r1_set == LSA_TRUE) /* need to do some special checks for redundancy ars even if ownership is not offered */
        )
    {
        //! this ar is owner of the submodule

        PNIO_UINT16 slot_nr;
        PNIO_UINT16 subslot_nr;
        PNIO_UINT32 subslot_error;
        PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

        //! find module in real config
        slot_nr    = pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.slot_nr;
        subslot_nr = pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.subslot_nr;
        pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
        if(subslot_error)
        {
            // slot/subslot not possible, must be an implementation error
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }

        if(local_sub_res != 0)
        {
            //! submodule is currently bound by pndv
            if (  (local_ar_ownership->ar_context == CM_SV_AR_CONTEXT_CONN)
                ||(local_ar_ownership->ar_context == CM_SV_AR_CONTEXT_PLUG)
                ||(local_ar_ownership->ar_context == CM_SV_AR_CONTEXT_PULLPDEV)
               )
            {
                //! because not only plugged modules are indicated, further checks to pp_sm are needed
                if (  (local_sub_res->res_state == PNDV_CFG_SUBMODULE_RES_STATE_W_OWN )     // wait for ownership
                    ||(local_sub_res->res_state == PNDV_CFG_SUBMODULE_RES_STATE_IN_DATA )   // normaly in context plug
                    ||(local_sub_res->res_state == PNDV_CFG_SUBMODULE_RES_STATE_W_IN_DATA )   // normaly in context plug
                    ||(local_sub_res->res_state == PNDV_CFG_SUBMODULE_RES_STATE_W_PRM_END)  // repeated ownership (ownership changed the config)
                    )
                {
                    /* reset para_error */
                    local_sub_res->para_error = PNIO_FALSE;
                    if (local_ar_ownership_elem->owner_session_key == pndv_data.ar[ar_idx].session_key)
                    {
                        local_sub_res->own_ar_idx = ar_idx;
                    }
                    local_sub_res->ar_set_nr = pndv_data.ar[ar_idx].ar_set_nr;
                    if (local_ar_ownership_elem->is_wrong)
                    {
                        // module is wrong
                        local_sub_res->submodule_state = PNDV_SUBMODULE_STATE_WRONG_MODUL;
                    }
                    else
                    {
                        // module is valid
                        local_sub_res->submodule_state = PNDV_SUBMODULE_STATE_VALID_MODUL;
                    }

                    if (  (local_ar_ownership_elem->owner_session_key == pndv_data.ar[ar_idx].session_key)
                        ||(local_sub_res->own_ar_idx != PNDV_AR_IDX_NOT_USED)
                        )
                    {
                        local_sub_res->own_ar_idx = ar_idx;
                        pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_CM_OWN_IND);
                    }
                    else
                    {
                        pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_R1_PASSIV_OWNER);
                    }


                }
                else
                {
                    //! the resource is not ready to be owned (plug not send to cm)
                    //! can only occur in context connect
                    //! as the plug is underway the information above will be set in repeated
                    //! ownership indication with context plug

                    //! overwrite comp result, will be also corrected in context plug
                    //! changing the comp_result has no effect if mod/sub id's are matching
                    local_ar_ownership_elem->is_wrong = LSA_TRUE;

                }
            }
        }
    }

    if (more_follows == PNIO_FALSE)
    {
        //! last response

        switch (local_ar_ownership->ar_context)
        {
            case CM_SV_AR_CONTEXT_CONN:
            {
                PNIO_UINT32 local_cnt;

                //for every ar assume that is locked or ownes no modules and prove the opposite
                pndv_data.ar[ar_idx].no_prm_end_ind = PNIO_TRUE;

                if (!pndv_data.ar_so_locked_state[ar_idx])
                {
                    // ar is not locked

                    //////////////////////////////////////////////////////////////////////////////
                    /// evaluate if PRM End will follow (-> at least 1 submodule ok / matching ID)
                    //////////////////////////////////////////////////////////////////////////////
                    for (local_cnt = 0; local_cnt < local_ar_ownership->nr_of_elems; local_cnt++)
                    {
                        //search for a submodule that is not wrong and owned by this ar
                        if (local_ar_ownership->elem[local_cnt].owner_session_key == pndv_data.ar[ar_idx].session_key)
                        {
                            //submodule is owned by this ar
                            if (!local_ar_ownership->elem[local_cnt].is_wrong)
                            {
                                //a good module found -> ar will get a prm_end_ind
                                pndv_data.ar[ar_idx].no_prm_end_ind = PNIO_FALSE;
                                //break loop
                                break;
                            }
                            else
                            {
                                CM_SV_AR_OWNERSHIP_ELEMENT_TYPE  *act_elem_ptr;
                                PNDV_SET_CFG_T  *set_cfg_elem_ptr;

                                act_elem_ptr = &(local_ar_ownership->elem[local_cnt]);
                                entity_nr = (PNIO_UINT16)PNDV_GET_ENTITY_NR(act_elem_ptr->api, act_elem_ptr->slot_nr, act_elem_ptr->subslot_nr, ar_idx);

                                if( entity_nr >= PNDV_MAX_SV_ENTITY )
                                {
                                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, act_elem_ptr->slot_nr );
                                }

                                set_cfg_elem_ptr = &pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr];

                                if(pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.cmp_result == PNDV_CMP_RES_ERROR)
                                {
                                    if (act_elem_ptr->slot_nr != PNDV_IM_SLOT_NO)
                                    {
                                        //detect the case that the real submodule and the expected one have the identical module/submodule IDs
                                        //if so the CM will issue a PRM_END even if the comp result of the module is wrong
                                        PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;
                                        PNIO_UINT32 subslot_error;

                                        pndv_get_submod_resource_ptr(&local_sub_res, act_elem_ptr->slot_nr, act_elem_ptr->subslot_nr, &subslot_error);

                                        if(subslot_error)
                                        {
                                            // slot/subslot not possible, must be an implementation error
                                            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
                                        }

                                        if(local_sub_res)
                                        {
                                            //the submodule is plugged
                                            if( (local_sub_res->sub_module.mod_ident == set_cfg_elem_ptr->elem.own_info.mod_ident) &&
                                                (local_sub_res->sub_module.sub_ident == set_cfg_elem_ptr->elem.own_info.sub_ident)
                                              )
                                            {
                                                //the expected and real config IDs are equal -> ar will get a prm_end_ind
                                                pndv_data.ar[ar_idx].no_prm_end_ind = PNIO_FALSE;
                                                //break loop
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    /////////////////////////////////////////////////////////////////////////////
                    /// check if PDEV is projected incompletely. if so set SO-locked for whole AR
                    /////////////////////////////////////////////////////////////////////////////
                    pndv_ar_peri_ownership_ind_done_check_pdev(ar_idx, local_ar_ownership);

                }   // if (!pndv_data.ar_so_locked_state[ar_idx])

                pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_OWNERSHIP_IND_DONE);

#ifdef PNDV_CFG_NO_EMPTY_PRM_END_SUPPORT
                if (pndv_data.ar[ar_idx].no_prm_end_ind == PNIO_TRUE)
                {
                    // if the flag is still true the first input update has to be triggered now
                    if(pndv_data.ar[ar_idx].ar_type != CM_AR_TYPE_SINGLE_RTC3)
                    {
                        PNDV_IFACE_CMD_ENTRY_T tmp_event;

                        //! for RTC1/2 AR request input update now
                        //! prepare interface command
                        tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                        tmp_event.add_2 = 0;
                        tmp_event.cmd = PNDV_EV_TO_PERI_READY_FOR_INPUT_UPDATE;

                        pndv_in_peri_write_coupl_event( tmp_event );
                    }
                }
#endif
            }
            // no break
            //lint -fallthrough
            case CM_SV_AR_CONTEXT_PLUG:
            {
                CM_UPPER_RQB_PTR_TYPE tmp_rqb_ptr;

                PNDV_RQB_SET_RESPONSE(pndv_data.ar[ar_idx].own_ind_rqb_ptr, CM_OK);

                PNDV_RQB_SET_HANDLE(pndv_data.ar[ar_idx].own_ind_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                PNDV_RQB_SET_OPCODE(pndv_data.ar[ar_idx].own_ind_rqb_ptr, CM_OPC_SV_AR_OWNERSHIP_RSP);

                PNDV_REQUEST(pndv_data.ar[ar_idx].own_ind_rqb_ptr, LSA_COMP_ID_CM);

                pndv_data.ar[ar_idx].own_ind_rqb_ptr = 0;
                pndv_data.ar[ar_idx].own_rsp_count = 0;

                pndv_data.serv.own_serv_run = PNIO_FALSE;
                //! check for waiting own requests
                tmp_rqb_ptr = 0;
                pndv_ar_peri_service_get_next(&tmp_rqb_ptr, PNDV_PERI_SERVICE_IDENT_OWN);
                if (tmp_rqb_ptr != 0)
                {
                    //! there is a waiting request of ownership
                    pndv_ar_cm_sv_ownership_ind(tmp_rqb_ptr);
                }
                break;
            }
            //! not possible here at the moment, request in this context is respondet synchronous
            case CM_SV_AR_CONTEXT_DISC:
            // no break
            case CM_SV_AR_CONTEXT_PULLPDEV: /* something to do yet */
            default:
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, local_ar_ownership->ar_context );
                break;
            }
        }
    }
    else
    {
        //! more own responses following
        pndv_data.ar[ar_idx].own_rsp_count++;
    }
}

/**
 * @brief   check if PDEV is projected incompletely and set SO-locked, if so
 *
 * (detecting this would have been possible already before ownerships are forwarded, but at the time
 *  of implementation this would have caused risky changes in upper layers)
 */
PNIO_VOID pndv_ar_peri_ownership_ind_done_check_pdev(PNIO_UINT8 ar_idx, CM_UPPER_SV_AR_OWNERSHIP_PTR_TYPE local_ar_ownership)
{
    PNIO_UINT8  projected_port_subslots      = 0;
    PNIO_UINT8  projected_interface_subslots = 0;
    PNIO_UINT16 local_cnt;

    for (local_cnt = 0; local_cnt < local_ar_ownership->nr_of_elems; local_cnt++)
    {
        if(local_ar_ownership->elem[local_cnt].slot_nr == PNDV_IM_SLOT_NO)
        {
            if(local_ar_ownership->elem[local_cnt].subslot_nr == pndv_data.cfg.pd.interface_subslot_nr)
            {
                projected_interface_subslots++;
            }
            else
            {
                PNIO_UINT16 port_count;
                for (port_count = 1; port_count <= pndv_data.cfg.pd.port_count_used; port_count++)
                {
                   if (local_ar_ownership->elem[local_cnt].subslot_nr == (pndv_data.cfg.pd.interface_subslot_nr+port_count) )
                   {
                       projected_port_subslots++;
                   }
                }
            }
        }
    }

#if (PNDV_CFG_PORT_IS_MODULAR == 0)
    /* compact PDEV: all or no PDEV submodule must be projected. Otherwise SO-Locked */
    if(   (projected_interface_subslots + projected_port_subslots != 1 + pndv_data.cfg.pd.port_count_used)
       && (projected_interface_subslots + projected_port_subslots != 0)
          )
#else
    /* modular PDEV: the AR having the PDEV must have at least one port projected. Otherwise SO-Locked. Doensn't need to be complete */
    if(projected_interface_subslots && (projected_port_subslots == 0))
#endif
    {
        // PDEV incomplete. In case of compact PDEV, all ports and the interface have to be projected together. Otherwise the
        // application has to set all PDEV subslots to SO-locked. See CM-spec, section ownership_ind
        pndv_in_write_debug_buffer_2__(PNDV_DC_LINE_INFO, PNDV_MODULE, __LINE__);
        pndv_ar_set_so_state( PNDV_SOL_EVENT_FLAG_PDEV_FAULT, PNIO_TRUE, ar_idx );

            // - reset ownership flag -> SO locked
            // - reset submodule state for submodules that already got the ownership response
        for (local_cnt = 0; local_cnt < local_ar_ownership->nr_of_elems; local_cnt++)
        {
            PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;
            PNIO_UINT32                         subslot_error;
            PNIO_UINT16                         slot_nr;
            PNIO_UINT16                         subslot_nr;

            local_ar_ownership->elem[local_cnt].owner_session_key = 0;

            slot_nr    = local_ar_ownership->elem[local_cnt].slot_nr;
            subslot_nr = local_ar_ownership->elem[local_cnt].subslot_nr;
            pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
            if(subslot_error)
            {
                // slot/subslot not possible, must be an implementation error
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
            }

            if(local_sub_res != 0)
            {
                //! submodule is currently bound by pndv
                if(local_sub_res->res_state > PNDV_CFG_SUBMODULE_RES_STATE_W_OWN)
                {
                    local_sub_res->own_ar_idx      = PNDV_AR_IDX_NOT_USED;
                    local_sub_res->ar_set_nr       = PNDV_SR_SET_NR_NOT_USED;;
                    local_sub_res->submodule_state = PNDV_SUBMODULE_STATE_NO_OWNER;
                    local_sub_res->para_error      = PNIO_FALSE;
                    pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_RELEASE); //!< release from onwnership
                }
            }
        }
    }
}

/**
 *  @brief check ar validity
 *
 *  @param[in]  rqb_ptr pointer to request block
 *  @return     returns an error or PNDV_OK
 *
 *
 */
PNIO_UINT32 pndv_ar_tool_ownership_ind_check_plausibility(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNDV_ENUM_AR_T                      ar_share_type = PNDV_AR_TYPE_UNKNOWN;
    PNIO_UINT32                         ret_val = PNDV_OK;
    PNIO_UINT32                         ar_wants_primary = LSA_FALSE;
    PNIO_UINT32                         ar_wants_pdev = LSA_FALSE;
    PNIO_UINT32                         ar_old_proj = LSA_FALSE;
    PNIO_UINT16                         elem_cnt;
    PNIO_UINT32                         ar_idx;
    CM_UPPER_SV_AR_OWNERSHIP_PTR_TYPE   own_ptr;
    PNIO_UINT32                         subslot_error;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;
    PNIO_UINT32                         pr_owner_ar = PNDV_AR_IDX_NOT_USED;
    PNIO_UINT32                         pr_owner_mod_ident = 0;
    PNIO_UINT32                         if_owner_ar = PNDV_AR_IDX_NOT_USED;
    PNIO_UINT32                         port_count;


    if (PNDV_RQB_GET_OPCODE( rqb_ptr) != CM_OPC_SV_AR_OWNERSHIP_IND)
    {
        //! only ownership.ind can be processed here
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE( rqb_ptr) );
    }

    ar_idx = pndv_ar_get_ar_idx_by_session_key( rqb_ptr->args.sv.ar_event->session_key );

    own_ptr = rqb_ptr->args.sv.ar_event->u.sv.ar_ownership;

    if( ar_idx == PNDV_AR_IDX_NOT_USED )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);
    }

    if( ar_idx > (PNDV_CM_AR_NO - 1) )
    {
        /* only one permitted ar (and none that is currently being dismantled) may come here */
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);
    }

    /* ---------------------------------------------------------------------
        PART 1:
            check ar type, DA AR
       --------------------------------------------------------------------- */


    switch (pndv_data.ar[ar_idx].ar_type)
    {
        /* User data AR */
        case CM_AR_TYPE_SINGLE_RTC3:
        case CM_AR_TYPE_SINGLE:
        case CM_AR_TYPE_SINGLE_SYSRED:
        {
            break;
        }
        case CM_AR_TYPE_SUPERVISOR:
        {
            if ( CM_AR_PROP_DEVICE_ACCESS & pndv_data.ar[ar_idx].ar_properties )
            {
                ar_share_type = PNDV_AR_TYPE_SUPERVISOR_DEVICE_ACCESS;
                LSA_UNUSED_ARG(ar_share_type);
                /* nothing more to be checked for a DA AR */
                return ret_val;
            }
            break;
        }
        default:
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.ar[ar_idx].ar_type);

            break;
        }
    }

    /* ---------------------------------------------------------------------
        PART 2:
            check submodul list of ownership.ind, search for head submodules
       --------------------------------------------------------------------- */

    for (elem_cnt = 0; elem_cnt < own_ptr->nr_of_elems; elem_cnt++)
    {
        CM_SV_AR_OWNERSHIP_ELEMENT_TYPE  *act_elem_ptr;

        act_elem_ptr = &(own_ptr->elem[elem_cnt]);

        if( act_elem_ptr->slot_nr == PNDV_IM_SLOT_NO )
        {
            /* Only IM-slot subs are considered here */

            if (PNDV_IS_SUPPORTED_OLD_PROJECTING(act_elem_ptr->mod_ident))
            {
                ar_old_proj = LSA_TRUE;
            }

            if (act_elem_ptr->subslot_nr == 1)
            {
                /* this ar want to have the primary submodule, the actual ownership is not of
                 * interrest only the fact that the primary submodule is projected
                 */
                ar_wants_primary = LSA_TRUE;
            }
            else if (  (act_elem_ptr->subslot_nr == pndv_data.cfg.pd.interface_subslot_nr) )
            {
                /* this ar want to have the pdev submodules, the actual ownership is not of
                 * interrest only the fact that the pdev submodules are projected
                 */
                ar_wants_pdev = LSA_TRUE;
            }
            else
            {
                for (port_count = 1; port_count <= pndv_data.cfg.pd.port_count_used; port_count++  )
                {
                    if (act_elem_ptr->subslot_nr == (pndv_data.cfg.pd.interface_subslot_nr+port_count) )
                    {
                        /* this ar want to have the pdev submodules, the actual ownership is not of
                         * interrest only the fact that the pdev submodules are projected
                         */
                        ar_wants_pdev = LSA_TRUE;
                    }
                }
            }
        }
    }

    /* ---------------------------------------------------------------------
        PART 3:
            determine the share type of the ar by previously found informations
       --------------------------------------------------------------------- */

    if (pndv_data.ar[ar_idx].ar_type == CM_AR_TYPE_SINGLE_SYSRED)
    {
        ar_share_type = PNDV_AR_TYPE_SYSRED;
    }
    else if (ar_old_proj)
    {
        ar_share_type = PNDV_AR_TYPE_OLD_PROJECTING;
    }
    else if (ar_wants_pdev && ar_wants_primary)
    {
        ar_share_type = PNDV_AR_TYPE_PRIMARY_WITH_PDEV;
    }
    else if (ar_wants_primary)
    {
        ar_share_type = PNDV_AR_TYPE_PRIMARY_WITHOUT_PDEV;
    }
    else if (ar_wants_pdev)
    {
        ar_share_type = PNDV_AR_TYPE_SHARED_WITH_PDEV;
    }
    else
    {
        ar_share_type = PNDV_AR_TYPE_SHARED_WITHOUT_PDEV;
    }

    /* ---------------------------------------------------------------------
        PART 4:
            determine whether this ar should be sol or not
       --------------------------------------------------------------------- */

    /* ---------------------------------------------------------------------
        PART 4A:
            -> determine owner-ARs of head submodules
            -> PDEV: check if all plugged PDEV subslots belong to the same AR or none (reject shared PDEV)
               -> non-plugged projected PDEV subslots lead to SOL by ownership-result (->wrong/no submodule)
       --------------------------------------------------------------------- */

    /* subslot 1 ("primary" subslot) */
    pndv_get_submod_resource_ptr(&local_sub_res, PNDV_IM_SLOT_NO/*slot_nr*/, 1/*subslot_nr*/, &subslot_error);
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
    pr_owner_ar = local_sub_res->own_ar_idx;
    if (pr_owner_ar != PNDV_AR_IDX_NOT_USED)
    {
        /* get modident for later use */
        PNIO_UINT32 entity_nr;
        /* look into set cfg to get mod id */
        entity_nr = PNDV_GET_ENTITY_NR(local_sub_res->sub_module.api, local_sub_res->sub_module.slot_nr, local_sub_res->sub_module.subslot_nr, pr_owner_ar);
        pr_owner_mod_ident = pndv_data.iface_ptr->set_cfg[pr_owner_ar][entity_nr].elem.own_info.mod_ident;
    }

    /* interface subslot */
    pndv_get_submod_resource_ptr(&local_sub_res, PNDV_IM_SLOT_NO/*slot_nr*/, pndv_data.cfg.pd.interface_subslot_nr/*subslot_nr*/, &subslot_error);
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
    if_owner_ar = local_sub_res->own_ar_idx;

    /* port subslots */
    for (port_count = 1; port_count <= pndv_data.cfg.pd.port_count_used; port_count++)
    {
        pndv_get_submod_resource_ptr(&local_sub_res, PNDV_IM_SLOT_NO/*slot_nr*/, (PNIO_UINT16)(pndv_data.cfg.pd.interface_subslot_nr + port_count/*subslot_nr*/), &subslot_error);
        if(subslot_error)
        {
            // slot/subslot not possible, must be an implementation error
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }

        if(local_sub_res == 0)
        {
            // port submodule not plugged
            #if (PNDV_CFG_PORT_IS_MODULAR == 0)
            // compact PDEV -> all ports must be plugged. This is an error in code!
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, (PNIO_UINT32)local_sub_res);
            #endif
        }
        else
        {
            if (local_sub_res->own_ar_idx == if_owner_ar)
            {
                // ports must be used by the same ar as the interface
            }
            else
            {
                // interface owner not identical with at least one port owner
                ret_val = PNDV_ERR_RESOURCE;
                pndv_in_write_debug_buffer_2__(PNDV_DC_LINE_INFO, PNDV_MODULE, __LINE__);
                pndv_ar_set_so_state(PNDV_SOL_EVENT_FLAG_PDEV_FAULT, PNIO_TRUE, ar_idx );
                break;
            }
        }
    }
    /* use if_owner_ar as representative for further pdev checks */

    /* ---------------------------------------------------------------------
        PART 4B:
            check head modules ownership against ar_share_type of own ind
       --------------------------------------------------------------------- */

    /* ---------------------------------------------------------------------
        PART 4B1:
            check if an old projecting ar already exists
       --------------------------------------------------------------------- */
    if (pr_owner_ar != PNDV_AR_IDX_NOT_USED)
    {
        /* primary has a owner*/
        if (pr_owner_ar == if_owner_ar)
        {
            /* check previously determined modident*/
            if (PNDV_IS_SUPPORTED_OLD_PROJECTING(pr_owner_mod_ident))
            {
                /* old ar's can only exist for its own */
                /* all other ar must be locked */
                ret_val = PNDV_ERR_RESOURCE;
                pndv_in_write_debug_buffer_2__(PNDV_DC_LINE_INFO, PNDV_MODULE, __LINE__);
                pndv_ar_set_so_state(PNDV_SOL_EVENT_FLAG_PDEV_FAULT, PNIO_TRUE, ar_idx );
            }
        }
    }

    /* ---------------------------------------------------------------------
        PART 4B2:
            check share type now
       --------------------------------------------------------------------- */

    if ( ret_val == PNDV_OK)
    {
        switch (ar_share_type)
        {
            case PNDV_AR_TYPE_OLD_PROJECTING:
            {
                break;
            }
            case PNDV_AR_TYPE_PRIMARY_WITH_PDEV:
            {
                /*
                 * current AR wants subslot 1 and PDEV
                 * -> set SOL if another AR already owns subslot 1 or the interface
                 */

                if (  (pr_owner_ar != PNDV_AR_IDX_NOT_USED)
                    ||(if_owner_ar == PNDV_AR_IDX_NOT_USED)
                    )
                {
                    /* special case */
                    /* primary already established, pdev not */
#if PNDV_IM_HAS_GLOB_PARA == 1
                    /* in devices without glob params, this is an allowed condition */
                    ret_val = PNDV_ERR_RESOURCE;
                    pndv_in_write_debug_buffer_2__(PNDV_DC_LINE_INFO, PNDV_MODULE, __LINE__);
                    pndv_ar_set_so_state(PNDV_SOL_EVENT_FLAG_PDEV_FAULT, PNIO_TRUE, ar_idx );
#endif
                }
                else if (  (pr_owner_ar != PNDV_AR_IDX_NOT_USED)
                         ||(if_owner_ar != PNDV_AR_IDX_NOT_USED)
                         )
                {
                    /* common case */
                    /* ether primary nor pdev must already be owned */
                    if (   !(pr_owner_ar == ar_idx)
                        && !(if_owner_ar == ar_idx)
                        )
                    {
                        /* its also no repeated ownership ind */
                        ret_val = PNDV_ERR_RESOURCE;
                        pndv_in_write_debug_buffer_2__(PNDV_DC_LINE_INFO, PNDV_MODULE, __LINE__);
                        pndv_ar_set_so_state(PNDV_SOL_EVENT_FLAG_PDEV_FAULT, PNIO_TRUE, ar_idx );
                    }

                }
                break;
            }
            case PNDV_AR_TYPE_PRIMARY_WITHOUT_PDEV:
            {
                /* there must not be any existing primary ar */
                if (pr_owner_ar != PNDV_AR_IDX_NOT_USED)
                {
#if PNDV_IM_HAS_GLOB_PARA == 1
                    if (!(pr_owner_ar == ar_idx))
                    {
                        /* its also no repeated ownership ind */
                        ret_val = PNDV_ERR_RESOURCE;
                        pndv_in_write_debug_buffer_2__(PNDV_DC_LINE_INFO, PNDV_MODULE, __LINE__);
                        pndv_ar_set_so_state(PNDV_SOL_EVENT_FLAG_PDEV_FAULT, PNIO_TRUE, ar_idx);
                    }
#endif
                }
                break;
            }
            case PNDV_AR_TYPE_SHARED_WITH_PDEV:
            {
                /*
                 * current AR wants PDEV without subslot 1
                 * -> set SOL if the interace already belongs to another AR
                 */
                if (if_owner_ar != PNDV_AR_IDX_NOT_USED)
                {
                    if (!(if_owner_ar == ar_idx))
                    {
                        /* its also no repeated ownership ind */
                        ret_val = PNDV_ERR_RESOURCE;
                        pndv_in_write_debug_buffer_2__(PNDV_DC_LINE_INFO, PNDV_MODULE, __LINE__);
                        pndv_ar_set_so_state(PNDV_SOL_EVENT_FLAG_PDEV_FAULT, PNIO_TRUE, ar_idx);
                    }
                }
                break;
            }
            case PNDV_AR_TYPE_SHARED_WITHOUT_PDEV:
            {
                /* every ar without head is ok (here) */
                break;
            }
            case PNDV_AR_TYPE_SUPERVISOR_DEVICE_ACCESS:
            {
                /* not possible here but would be ok too */
                break;
            }
            case PNDV_AR_TYPE_SYSRED:
            {
                /* only possible if there are no other ar types already in use */
                break;
            }
            case PNDV_AR_TYPE_UNKNOWN:
            default:
            {
                ret_val = PNDV_ERR_RESOURCE;
                // pndv_in_write_debug_buffer_all_add__(PNDV_DC_CM_CHK_IND_AR_ABORT, (PNIO_UINT16)__LINE__, modul_ptr->mod_ident);
                break;
            }
        }
    }

    /* ---------------------------------------------------------------------
        PART 4C:
            check for existing isochronous ar
       --------------------------------------------------------------------- */
    if (pndv_data.cfg.mode_isom == LSA_TRUE)
    {
        /* there is already an isochronous ar established */
        #ifdef PNDV_CFG_ISOM_NO_SHARED

        #endif
    }

    return ret_val;
}
//@}

//! @name Prm-End
//@{
/**
 *  @brief indicates the end of parameter phase
 *  @ingroup pndv_ar_prm_end
 *
 *  @param[in]     rqb_ptr pointer to request block
 *
 *  long_description
 *
 */
PNIO_VOID pndv_ar_cm_sv_event_prm_end_ind (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 ar_idx;
    CM_UPPER_SV_AR_PRM_END_PTR_TYPE local_ar_prm_end_ptr;

    if (PNDV_RQB_GET_OPCODE( rqb_ptr) != CM_OPC_SV_AR_PRM_END_IND)
    {
        //! only prm_end.ind can be processed here
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE( rqb_ptr) );
    }
    if (pndv_data.serv.prm_serv_run == PNIO_FALSE)
    {
        ar_idx = pndv_ar_get_ar_idx_by_session_key( rqb_ptr->args.sv.ar_event->session_key);

        //! save rqb_ptr for async response
        pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr = rqb_ptr;

        local_ar_prm_end_ptr = rqb_ptr->args.sv.ar_event->u.sv.ar_prm_end;

        switch (local_ar_prm_end_ptr->ar_context)
        {
            case CM_SV_AR_CONTEXT_CONN:
            {
                if ( pndv_data.ar[ar_idx].sm_state != PNDV_AR_SM_W_PRM_END_IND)
                {
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.ar[ar_idx].sm_state );
                }

                //! send prm_end event to sm (only in context of connect)
                pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_PRM_END_IND);

                if (local_ar_prm_end_ptr->nr_of_elems == 0)
                {
                    // special case: no modules in ar
                    // just trigger the ar sm and send the rqb back

                    PNDV_RQB_SET_RESPONSE(pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr, CM_OK);

                    PNDV_RQB_SET_HANDLE(pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                    PNDV_RQB_SET_OPCODE(pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr, CM_OPC_SV_AR_PRM_END_RSP);

                    if (pndv_data.ar[ar_idx].ar_type != CM_AR_TYPE_SINGLE_RTC3)
                    {
                        /* save the rqb and send it back after input update */

                        //! mark service to be in use, to prevent other prm end from overtaking
                        pndv_data.serv.prm_serv_run = PNIO_TRUE;

                        pndv_data.ar[ar_idx].empty_prm_end_ind = PNIO_TRUE;
                        pndv_data.ar[ar_idx].prm_end_rsp_count = 0;

                        pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_PRM_END_IND_DONE);

                        /* input update can only be done for ARs with iocr */
                        /* check if there can be an input update */
                        if (pndv_data.ar[ar_idx].nr_of_iocrs == 0)
                        {
                            /* no update possible, do so as if there was one */
                            pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_FIRST_INPUT_UPDATE_DONE);
                        }
                    }
                    else
                    {
                        //send the response now only for rtc3 ARs
                        PNDV_REQUEST(pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr, LSA_COMP_ID_CM);

                        pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr = 0;

                        pndv_data.ar[ar_idx].prm_end_rsp_count = 0;

                        pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_PRM_END_IND_DONE);
                    }
                }
            }
            // no break
            //lint -fallthrough
            case CM_SV_AR_CONTEXT_PLUG:
            {
                PNIO_UINT16 elem_cnt;
                PNIO_UINT16 entity_nr;

                for (elem_cnt = 0; elem_cnt < local_ar_prm_end_ptr->nr_of_elems; elem_cnt++)
                {
                    PNDV_IFACE_CMD_ENTRY_T tmp_event;
                    CM_SV_AR_PRM_END_ELEMENT_TYPE *act_elem_ptr;
                    PNDV_SET_CFG_T  *set_cfg_elem_ptr;

                    act_elem_ptr = &(local_ar_prm_end_ptr->elem[elem_cnt]);

                    entity_nr = (PNIO_UINT16)PNDV_GET_ENTITY_NR(act_elem_ptr->api, act_elem_ptr->slot_nr, act_elem_ptr->subslot_nr, ar_idx);
                    set_cfg_elem_ptr = &pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr];

                    //! mark service to be in use
                    pndv_data.serv.prm_serv_run = PNIO_TRUE;

                    set_cfg_elem_ptr->elem.prm_end_state = PNDV_IFACE_SERVICE_NEW;
                    if((act_elem_ptr->appl_ready_pending) && (((act_elem_ptr->subslot_nr & 0xF000) == 0x8000) || ((act_elem_ptr->subslot_nr & 0xF000) == 0x9000)))
                    {
                        /* PDEV subslots may request ARP/IOXS bad */
                        set_cfg_elem_ptr->flags |= PNDV_SET_CFG_FLAGS_SET_ARP;
                    }
                    else
                    {
                        /* for all other subslots, ARP may not be requested by PNIO. Preset flag with 0 */
                        set_cfg_elem_ptr->flags &= ~PNDV_SET_CFG_FLAGS_SET_ARP;
                    }
                    //! prepare interface command
                    tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                    tmp_event.add_2 = entity_nr;
                    if (elem_cnt == (local_ar_prm_end_ptr->nr_of_elems-1))
                    {
                        //! last sub in ownership list
                        tmp_event.cmd = PNDV_EV_TO_PERI_PRM_END_IND;
                    }
                    else
                    {
                        //! there are more subs in ownership list
                        tmp_event.cmd = PNDV_EV_TO_PERI_PRM_END_IND_MORE_FOLLOWS;
                    }

                    pndv_in_peri_write_coupl_event( tmp_event );
                }
                break;
            }

            case CM_SV_AR_CONTEXT_DISC:
            {
                //! this context is dont care because the disconnect ind is used to clean up all data
                //! if subs go to another ar, a prm_end with context PLUG in this ar will follow anyway

                PNDV_RQB_SET_RESPONSE( pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr, CM_OK);

                PNDV_RQB_SET_HANDLE( pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                PNDV_RQB_SET_OPCODE( pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr, CM_OPC_SV_AR_PRM_END_RSP);

                PNDV_REQUEST(pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr, LSA_COMP_ID_CM);

                pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr = 0;
                pndv_data.ar[ar_idx].prm_end_rsp_count = 0;

                break;
            }
            case CM_SV_AR_CONTEXT_PULLPDEV: /* something to do yet */
            default:
            {
                //! unknown context
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, local_ar_prm_end_ptr->ar_context);
                break;
            }
        }
    }
    else
    {
        //! there is still a prm_end indication running
        if (pndv_ar_do_service_queue(rqb_ptr, PNDV_PERI_SERVICE_IDENT_PRM) != PNDV_OK)
        {
            //! queueing not possible, no free resource, check PNDV_PERI_SERVICE_QUEUES_T elem
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }

    }
}

/**
 *  @brief prm end has been processed by peri
 *  @ingroup pndv_ar_prm_end
 *
 *  @param[in]  ar_idx ar index
 *  @param[in]  entity_nr nr of the entity (submodule description block)
 *  @param[in]  more_follows PNIO_TRUE = more responses follow, PNIO_FALSE = last response
 *
 *  prm_end responses have to come back in order of request
 *
 */

PNIO_VOID pndv_ar_peri_prm_end_ind_done(PNIO_UINT8 ar_idx, PNIO_UINT16 entity_nr, PNIO_BOOL more_follows)
{
    CM_UPPER_RQB_PTR_TYPE               rqb_ptr;
    PNIO_UINT32                         elem_cnt;
    PNIO_UINT16                         slot_nr;
    PNIO_UINT16                         subslot_nr;
    PNIO_UINT32                         subslot_error;
    PNIO_UINT32                         sr_set_idx;
    CM_UPPER_SV_AR_PRM_END_PTR_TYPE     local_ar_prm_end_ptr;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;
    PNIO_UINT16                         wait_for_r1;

    rqb_ptr = pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr;

    if (rqb_ptr == 0)
    {
        // state conflict, no prm_end req in process
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx );
    }

    elem_cnt = pndv_data.ar[ar_idx].prm_end_rsp_count;
    if (pndv_data.ar[ar_idx].prm_end_rsp_count == 0)
    {
        // first call
        // no entries deleted yet
        pndv_data.ar[ar_idx].prm_end_rsp_deleted = 0;
    }

    local_ar_prm_end_ptr = rqb_ptr->args.sv.ar_event->u.sv.ar_prm_end;

    if (elem_cnt > local_ar_prm_end_ptr->nr_of_elems)
    {
        // more responses than requests
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, elem_cnt );
    }

    //! find module in real config
    slot_nr    = pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.slot_nr;
    subslot_nr = pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.subslot_nr;

    pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);

    if(subslot_error)
    {
        // slot/subslot not possible, must be an implementation error
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.prm_end_state = PNDV_IFACE_SERVICE_IDLE;

    wait_for_r1 = PNIO_FALSE;

    if(local_sub_res != 0)
    {
        /* R1 redundancy: here we are on the primary side and tell the backup side about our PRMEnd, so
         *                that it can adjust its submodule_ptr->res_state (within pndv_pp_sm()). This is important for plugged
         *                submodules because they don't get an ownership on the backup side and they need
         *                the info, that they are owned (so that they can be switched to INDATA on a b->p edge later on)
         */
        //! as only plugged subs are indicated no further checks to pp_sm are needed

        if (local_ar_prm_end_ptr->ar_context == CM_SV_AR_CONTEXT_PLUG)
        {
            if (pndv_data.ar[ar_idx].ar_set_nr != 0)
            {
                // this ar is part of a sysred ar_set
                // Waidelich: to care only in context plug
                sr_set_idx = pndv_ar_get_sr_idx_by_ar_set_nr(pndv_data.ar[ar_idx].ar_set_nr);
                if (sr_set_idx == PNDV_SR_SET_IDX_NOT_USED)
                {
                    // must not be here
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);        /*NOTREACHED*/
                }
            }
        }
    }

    if (!wait_for_r1)
    {
        pndv_ar_peri_prm_end_ind_done_continue(ar_idx, slot_nr, subslot_nr, more_follows);
    }
}

PNIO_VOID pndv_ar_peri_prm_end_ind_done_continue(PNIO_UINT8 ar_idx, PNIO_UINT16 slot_nr, PNIO_UINT16 subslot_nr, PNIO_BOOL more_follows)
{
    CM_UPPER_RQB_PTR_TYPE   rqb_ptr;
    PNIO_UINT32             elem_cnt;
    PNIO_UINT32             entity_nr;
    PNIO_UINT32             subslot_error;
    PNIO_UINT32             sr_set_idx;
    PNIO_UINT32             cfg_flags;
    CM_UPPER_SV_AR_PRM_END_PTR_TYPE     local_ar_prm_end_ptr;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;
    PNIO_BOOL is_res_state_unknown = PNIO_FALSE; 

    rqb_ptr = pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr;

    if (rqb_ptr == 0)
    {
        // state conflict, no prm_end req in process
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx );
    }

    elem_cnt = pndv_data.ar[ar_idx].prm_end_rsp_count;
    if (pndv_data.ar[ar_idx].prm_end_rsp_count == 0)
    {
        // first call
        // no entries deleted yet
        pndv_data.ar[ar_idx].prm_end_rsp_deleted = 0;
    }

    local_ar_prm_end_ptr = rqb_ptr->args.sv.ar_event->u.sv.ar_prm_end;

    if (elem_cnt > local_ar_prm_end_ptr->nr_of_elems)
    {
        // more responses than requests
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, elem_cnt );
    }

    // to point to the right element (if some where deleted already) elemt_cnt needs fixing
    elem_cnt -= pndv_data.ar[ar_idx].prm_end_rsp_deleted;

    pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);

    if (subslot_error)
    {
        // slot/subslot not possible, must be an implementation error
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }
    
    entity_nr = PNDV_GET_ENTITY_NR(local_sub_res->sub_module.api, slot_nr, subslot_nr, ar_idx);

    if (local_sub_res != 0)
    {
        if ((entity_nr < PNDV_MAX_SV_ENTITY) &&
            (local_sub_res->res_state == PNDV_CFG_SUBMODULE_RES_STATE_W_PRM_END)) // added check to pp-sm, needed to detect if a submodule was pulled and replugged meanwhile
        {
            cfg_flags  = pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].flags;

            //! submodule is currently bound by pndv
            pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_CM_PRM_END_IND);

            // appl_ready_xx needs only to be filled for a bound sub
            // unbound subs are deleted from the response (see else branch)
            if (local_ar_prm_end_ptr->ar_context == CM_SV_AR_CONTEXT_CONN)
            {
                /* don't change appl ready state for pdev (change in pdev arp handling with PN 6.0) */
                if (!(PNDV_IS_PDEV(local_ar_prm_end_ptr->elem[elem_cnt].slot_nr, local_ar_prm_end_ptr->elem[elem_cnt].subslot_nr)))
                {
                    if (pndv_data.ar[ar_idx].ar_type == CM_AR_TYPE_SINGLE_RTC3)
                    {
                        // appl ready state is known
                        local_ar_prm_end_ptr->elem[elem_cnt].appl_ready_follows = LSA_FALSE;
                        // appl is ready
                        if ((local_sub_res->para_error) || (cfg_flags & PNDV_SET_CFG_FLAGS_SET_ARP))
                        {
                            // para error -> not ready
                            local_ar_prm_end_ptr->elem[elem_cnt].appl_ready_pending = LSA_TRUE;
                        }
                        else
                        {
                            // no para error -> ready
                            local_ar_prm_end_ptr->elem[elem_cnt].appl_ready_pending = LSA_FALSE;
                        }
                    }
                    else
                    {
                        /* all other ar types   CM_AR_TYPE_SINGLE, CM_AR_TYPE_SUPERVISOR, CM_AR_TYPE_SINGLE_SYSRED */
                        // appl ready state is not known yet
                        local_ar_prm_end_ptr->elem[elem_cnt].appl_ready_follows = LSA_TRUE;
                        // appl is not ready, ignore
                        local_ar_prm_end_ptr->elem[elem_cnt].appl_ready_pending = LSA_TRUE;
                    }
                }
            }
            else
            {
                // context plug
                // sub is appl ready pending if para ds was wrong or pndv user forces arp by setting PNDV_SET_CFG_FLAGS_SET_ARP
                local_ar_prm_end_ptr->elem[elem_cnt].appl_ready_follows = LSA_FALSE;
                // appl is ready
                if ((local_sub_res->para_error) || (cfg_flags & PNDV_SET_CFG_FLAGS_SET_ARP))
                {
                    // para error -> not ready
                    local_ar_prm_end_ptr->elem[elem_cnt].appl_ready_pending = LSA_TRUE;
                }
                else
                {
                    // no para error -> ready
                    local_ar_prm_end_ptr->elem[elem_cnt].appl_ready_pending = LSA_FALSE;
                }

                if (pndv_data.ar[ar_idx].ar_set_nr != 0)
                {
                    // this ar is part of a sysred ar_set
                    // Waidelich: to care only in context plug
                    sr_set_idx = pndv_ar_get_sr_idx_by_ar_set_nr(pndv_data.ar[ar_idx].ar_set_nr);
                    if (sr_set_idx == PNDV_SR_SET_IDX_NOT_USED)
                    {
                        // must not be here
                        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);        /*NOTREACHED*/
                    }

                    if (pndv_data.sr[sr_set_idx].primary_ar_idx != ar_idx)
                    {
                        // this ar is not the primary, module has to remain appl ready pending
                        local_ar_prm_end_ptr->elem[elem_cnt].appl_ready_pending = LSA_TRUE;
                    }
                }
            }
        }
        else if ((entity_nr < PNDV_MAX_SV_ENTITY) &&
                 (local_sub_res->res_state == PNDV_CFG_SUBMODULE_RES_STATE_W_OWN_PASSIV))
        {
            //! PRMEnd for a submodule that's now in a backup AR / passive. This happens if there's a primary->backup transition
            //! during PRMEnd. CM spec says: "Due to the asynchronous interface the user may see a prm-end indication on an AR that is no longer the primary AR or "first AR".
            //! In this case the prm-end indication has to be responded with appl_ready_follows = LSA_FALSE and appl_ready_pending = LSA_TRUE."
            local_ar_prm_end_ptr->elem[elem_cnt].appl_ready_follows = LSA_FALSE;
            local_ar_prm_end_ptr->elem[elem_cnt].appl_ready_pending = LSA_TRUE;
        }
        else
        {
            // if local_sub_res->res_state is not one of the selected values above if checks, it should handled as deleted element. 
            is_res_state_unknown = PNIO_TRUE;
        }
    }

    if ((local_sub_res == 0) || (entity_nr >= PNDV_MAX_SV_ENTITY) || is_res_state_unknown)
    {
        PNIO_UINT32 local_elem_cnt;
        //! submodule has been pulled or replugged meanwhile
        //! delete this element from response rqb
        for (local_elem_cnt = elem_cnt; local_elem_cnt < (local_ar_prm_end_ptr->nr_of_elems - pndv_data.ar[ar_idx].prm_end_rsp_deleted - 1); local_elem_cnt++)
        {
            // overwrite current element with next (starting element will be deleted)
            local_ar_prm_end_ptr->elem[local_elem_cnt] = local_ar_prm_end_ptr->elem[local_elem_cnt+1];
        }

        // one more element has been deleted
        pndv_data.ar[ar_idx].prm_end_rsp_deleted++;
    }

    if (more_follows == PNIO_FALSE)
    {
        //! last response

        //! subtract deleted elements
        local_ar_prm_end_ptr->nr_of_elems -= (PNIO_UINT16)pndv_data.ar[ar_idx].prm_end_rsp_deleted;

        // theoretical case: if primary->backup edge occurs while we've already processed some PRMEndDones, we have to
        //                   set them all to "appl. ready pending".
        if (pndv_data.ar[ar_idx].ar_set_nr != 0)
        {
            sr_set_idx = pndv_ar_get_sr_idx_by_ar_set_nr(pndv_data.ar[ar_idx].ar_set_nr);
            if (ar_idx != pndv_data.sr[sr_set_idx].primary_ar_idx)
            {
                PNIO_UINT32 local_element_cnt;
                for (local_element_cnt = 0; local_element_cnt < local_ar_prm_end_ptr->nr_of_elems; local_element_cnt++)
                {
                    local_ar_prm_end_ptr->elem[local_element_cnt].appl_ready_follows = LSA_FALSE;
                    local_ar_prm_end_ptr->elem[local_element_cnt].appl_ready_pending = LSA_TRUE;
                }
            }
        }

        switch (local_ar_prm_end_ptr->ar_context)
        {
            case CM_SV_AR_CONTEXT_CONN:
            {
                pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_PRM_END_IND_DONE);

                if(pndv_data.ar[ar_idx].ar_type != CM_AR_TYPE_SINGLE_RTC3)
                {
                    PNIO_UINT32 local_element_cnt;
                    if (pndv_data.ar[ar_idx].nr_of_iocrs == 0)
                    {
                        /* should not happen here as PRM_END_IND_DONE for empty ARs is given in pndv_ar_cm_sv_event_prm_end_ind */
                        pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.ar[ar_idx].nr_of_iocrs);
                    }
                    //! copy elements of prm_end_ind to a local buffer (needed for appl_ready_req)
                    pndv_data.ar[ar_idx].prm_end_elem_cnt = pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr->args.sv.ar_event->u.sv.ar_prm_end->nr_of_elems;
                    PNDV_COPY_BYTE(&pndv_data.ar[ar_idx].prm_end_elem_list[0],  &(pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr->args.sv.ar_event->u.sv.ar_prm_end->elem[0]),(pndv_data.ar[ar_idx].prm_end_elem_cnt) * sizeof(CM_SV_AR_PRM_END_ELEMENT_TYPE));

                    /* delete pdev and head from appl ready list*/
                    for (local_element_cnt = 0; local_element_cnt < pndv_data.ar[ar_idx].prm_end_elem_cnt; local_element_cnt++)
                    {
                        if (PNDV_IS_PDEV(pndv_data.ar[ar_idx].prm_end_elem_list[local_element_cnt].slot_nr, pndv_data.ar[ar_idx].prm_end_elem_list[local_element_cnt].subslot_nr))
                        {
                            PNIO_UINT32 move_element_cnt;

                            /* delete entry and move next entrys */
                            for( move_element_cnt = local_element_cnt; move_element_cnt < (pndv_data.ar[ar_idx].prm_end_elem_cnt - 1); move_element_cnt++)
                            {
                                pndv_data.ar[ar_idx].prm_end_elem_list[move_element_cnt] = pndv_data.ar[ar_idx].prm_end_elem_list[move_element_cnt+1];
                            }
                            /* decrement count of elements in list */
                            pndv_data.ar[ar_idx].prm_end_elem_cnt--;
                            /* decrement loop counter to evaluate the first moved element next*/
                            local_element_cnt--;
                        }
                    }
                }

            }
            // no break
            //lint -fallthrough
            case CM_SV_AR_CONTEXT_PLUG:
            {
                CM_UPPER_RQB_PTR_TYPE tmp_rqb_ptr;

                PNDV_RQB_SET_RESPONSE(pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr, CM_OK);

                PNDV_RQB_SET_HANDLE(pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                PNDV_RQB_SET_OPCODE(pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr, CM_OPC_SV_AR_PRM_END_RSP);

                PNDV_REQUEST(pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr, LSA_COMP_ID_CM);

                /* check for parked alarms and diags */
                pndv_in_al_check_dial_continue();
                pndv_in_al_check_rosal_queue();
                pndv_in_al_check_diag();

                pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr = 0;
                pndv_data.ar[ar_idx].prm_end_rsp_count = 0;

                pndv_data.serv.prm_serv_run = PNIO_FALSE;
                //! check for waiting prm_end requests
                tmp_rqb_ptr = 0;
                pndv_ar_peri_service_get_next(&tmp_rqb_ptr, PNDV_PERI_SERVICE_IDENT_PRM);
                if (tmp_rqb_ptr != 0)
                {
                    //! there is a waiting request of ownership
                    pndv_ar_cm_sv_event_prm_end_ind(tmp_rqb_ptr);
                }
                break;
            }
            //! not possible here at the moment, request in this context is respondet synchronous
            case CM_SV_AR_CONTEXT_DISC:
            // no break
            case CM_SV_AR_CONTEXT_PULLPDEV: /* something to do yet */
            default:
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, local_ar_prm_end_ptr->ar_context );
                break;
            }
        }
    }
    else
    {
        //! more prm_end responses following
        pndv_data.ar[ar_idx].prm_end_rsp_count++;
    }
}

/**
 *  @brief      check special case where ar has no modules (empty prm end)
 *
 *  @param[in]  ar_idx index of the ar
 *
 *  to be called within the ar_sm
 *
 */
PNIO_VOID pndv_ar_tool_prm_end_check_empty_request(PNIO_UINT32 ar_idx)
{
    //check ar-state, only usefull in state PNDV_AR_SM_W_IN_DATA_IND and PNDV_AR_SM_DISCONNECT_IND_W_DONE
    if(   (pndv_data.ar[ar_idx].sm_state == PNDV_AR_SM_W_IN_DATA_IND)
        ||(pndv_data.ar[ar_idx].sm_state == PNDV_AR_SM_W_DISCONNECT_IND_DONE)
       )
    {
        CM_UPPER_RQB_PTR_TYPE tmp_rqb_ptr;

        /* special case, no modules owned by ar, no appl ready possible */
        if (pndv_data.ar[ar_idx].empty_prm_end_ind == PNIO_TRUE)
        {

            if (pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr == 0)
            {
                // not possible here
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
            }

            pndv_data.ar[ar_idx].empty_prm_end_ind = PNIO_FALSE;
            PNDV_REQUEST(pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr, LSA_COMP_ID_CM);

            pndv_data.ar[ar_idx].prm_end_ind_rqb_ptr = 0;
            pndv_data.ar[ar_idx].prm_end_rsp_count = 0;

            pndv_data.serv.prm_serv_run = PNIO_FALSE;
            //! check for waiting prm_end requests
            tmp_rqb_ptr = 0;
            pndv_ar_peri_service_get_next(&tmp_rqb_ptr, PNDV_PERI_SERVICE_IDENT_PRM);
            if (tmp_rqb_ptr != 0)
            {
                //! there is a waiting request of ownership
                pndv_ar_cm_sv_event_prm_end_ind(tmp_rqb_ptr);
            }
        }
    }
}
//@}

//! @name Input Update
//@{
/**
 *  @brief first input update of an ar is done
 *  @ingroup pndv_ar_input_update
 *
 *  @param[in]     ar_idx ar index
 *
 *  long_description
 *
 */
PNIO_VOID pndv_ar_peri_ready_for_input_update_done(PNIO_UINT8 ar_idx)
{
    pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_FIRST_INPUT_UPDATE_DONE);
}
//@}

//! @name Application ready
//@{
/**
 *  @brief prepare appl ready req and send to cm
 *  @ingroup pndv_ar_appl_ready
 *
 *  @param[in]     ar_idx ar index
 *
 *  This funktion must only be called after the first input update of a sequence
 *  of modules is done after a prm_end_ind
 *
 */
PNIO_VOID pndv_ar_do_appl_ready_req(PNIO_UINT32 ar_idx)
{
    //! appl ready req must contain all modules that where indicated with prm_end.ind

    PNIO_UINT16               elem_cnt, i, skip_cnt;
    CM_UPPER_RQB_PTR_TYPE     local_rqb_ptr;
    CM_SV_AR_APPL_READY_TYPE* local_ready_ptr;

    //! inti local rqb pointers
    local_rqb_ptr = &pndv_data.rqb.app_ready[ar_idx];
    local_ready_ptr = (CM_SV_AR_APPL_READY_TYPE *)&pndv_data.rqb.app_ready_args[ar_idx];


    //! get count of elements from prm_end.ind
    elem_cnt = pndv_data.ar[ar_idx].prm_end_elem_cnt;

    if (elem_cnt > PNDV_MAX_SV_ENTITY)
    {
        //! elem_cnt is to big, this would cause an overflow
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, elem_cnt);
    }

    if (elem_cnt)
    {
        local_ready_ptr->device_nr      = PNDV_CM_DEVICE_NO;

        for(i=0, skip_cnt = 0 ;i < elem_cnt; i++)
        {
            PNIO_UINT32 tmp_api;
            PNIO_UINT16 tmp_slot, tmp_sub;
            PNIO_UINT32 tmp_subs_error, entity_nr;
            PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

            tmp_api     = pndv_data.ar[ar_idx].prm_end_elem_list[i].api;
            tmp_slot    = pndv_data.ar[ar_idx].prm_end_elem_list[i].slot_nr;
            tmp_sub     = pndv_data.ar[ar_idx].prm_end_elem_list[i].subslot_nr;

            entity_nr = PNDV_GET_ENTITY_NR(tmp_api, tmp_slot, tmp_sub, ar_idx);

            //! check if submodule was pulled meanwhile
            if (  (pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.cmp_result == PNDV_CMP_RES_NO_MODULE)
                )
            {
                //! submodule is pulled or has error now -> skip it
                skip_cnt++;
            }
            else
            {
                //! submodule is still ok -> put it to appl_ready list
                local_ready_ptr->elem[i-skip_cnt].session_key    = pndv_data.ar[ar_idx].session_key;

                local_ready_ptr->elem[i-skip_cnt].api                = tmp_api;
                local_ready_ptr->elem[i-skip_cnt].slot_nr            = tmp_slot;
                local_ready_ptr->elem[i-skip_cnt].subslot_nr         = tmp_sub;


                pndv_get_submod_resource_ptr(&local_sub_res, tmp_slot, tmp_sub, &tmp_subs_error);
                if(tmp_subs_error)
                {
                    // slot/subslot not possible, must be an implementation error
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
                }

                if(local_sub_res != 0)
                {
                    //! submodule is plugged, everything is impossible here (see skip above)
                    if (  (local_sub_res->para_error)
                        ||(pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.cmp_result == PNDV_CMP_RES_ERROR)
                        ||(pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].flags & PNDV_SET_CFG_FLAGS_SET_ARP) // pndv user forces arp
                        )
                    {
                        local_ready_ptr->elem[i-skip_cnt].appl_ready_pending = CM_SV_APPL_READY_PENDING_YES;
                    }
                    else
                    {
                        local_ready_ptr->elem[i-skip_cnt].appl_ready_pending = CM_SV_APPL_READY_PENDING_NO;
                    }
                }
            }
        }

        local_ready_ptr->nr_of_elems                    = elem_cnt-skip_cnt;
        local_rqb_ptr->args.sv.ar_appl_ready            = local_ready_ptr;

        PNDV_RQB_SET_HANDLE(local_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
        PNDV_RQB_SET_OPCODE(local_rqb_ptr, CM_OPC_SV_AR_APPL_READY);

        PNDV_CFG_MEASUREMENT_APPL_READY();

        PNDV_REQUEST(local_rqb_ptr, LSA_COMP_ID_CM);
    }
    else
    {
        /* special case, no modules owned by ar, no appl ready possible */
        /* just trigger the sm */
        pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_APP_READY_RSP);
    }

}

/**
 *  @brief processing confirmation to appl ready req
 *  @ingroup pndv_ar_appl_ready
 *
 *  @param[in]     rqb_ptr pointer to request block
 *
 *  long_description
 *
 */
PNIO_VOID pndv_ar_cm_sv_appl_ready_done (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response,
                ar_idx;

    response = PNDV_RQB_GET_RESPONSE( rqb_ptr);

    ar_idx = pndv_ar_get_ar_idx_by_session_key( rqb_ptr->args.sv.ar_appl_ready->elem[0].session_key );

    if (   (CM_OK            != response)
        && (CM_OK_CANCELLED  != response)
       )
    {
        if (response == CM_ERR_ELEM)
        {
            PNIO_UINT32 i;
            CM_UPPER_SV_AR_APPL_READY_ELEMENT_PTR_TYPE act_elem_ptr = NULL;

            for(i=0; i < rqb_ptr->args.sv.ar_appl_ready->nr_of_elems; i++)
            {
                if(rqb_ptr->args.sv.ar_appl_ready->elem[i].response == CM_ERR_ELEM)
                {
                    act_elem_ptr = &rqb_ptr->args.sv.ar_appl_ready->elem[i];
                    break;
                }
            }
            if (act_elem_ptr)
            {
                PNIO_UINT32 entity_nr = PNDV_GET_ENTITY_NR(act_elem_ptr->api, act_elem_ptr->slot_nr, act_elem_ptr->subslot_nr, ar_idx);
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, entity_nr);
            }
            else
            {
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0xE0DE1111);
            }
        }
        else
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, response);
        }
    }

    if( PNDV_AR_IDX_NOT_USED == ar_idx )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, rqb_ptr->args.sv.ar_appl_ready->elem[0].session_key );
        /*NOTREACHED*/
    }

    pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_APP_READY_RSP);

    //! a prm_end appl_ready sequence is now done, reset prm_end_elem_cnt to free this resource
    pndv_data.ar[ar_idx].prm_end_elem_cnt = 0;

    /* check for parked alarms and diags */
    pndv_in_al_check_dial_continue();
    pndv_in_al_check_rosal_queue();
    pndv_in_al_check_diag();

}
//@}

//! @name In data
//@{
/**
 *  @brief process in_data.ind
 *  @ingroup pndv_ar_in_data
 *
 *  @param[in]     rqb_ptr pointer to request block
 *
 *  long_description
 *
 */
PNIO_VOID pndv_ar_cm_ar_in_data_ind (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 ar_idx;
    CM_UPPER_SV_AR_IN_DATA_PTR_TYPE local_ar_in_data;

    if (PNDV_RQB_GET_OPCODE( rqb_ptr) != CM_OPC_SV_AR_IN_DATA_IND)
    {
        //! only in_data.ind can be processed here
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE( rqb_ptr) );
    }

    if (pndv_data.serv.ida_serv_run == PNIO_FALSE)
    {
        ar_idx = pndv_ar_get_ar_idx_by_session_key( rqb_ptr->args.pd.event->session_key );

        if( PNDV_AR_IDX_NOT_USED == ar_idx )
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, rqb_ptr->args.pd.event->session_key ); /*NOTREACHED*/
        }
        //! save rqb_ptr for async response
        pndv_data.ar[ar_idx].indata_ind_rqb_ptr = rqb_ptr;

        local_ar_in_data = rqb_ptr->args.sv.ar_event->u.sv.ar_in_data;

        //! as there is no context element within in_data ind, its needed to check ar_sm state
        switch ((pndv_data.ar[ar_idx].sm_state) )
        {


            case PNDV_AR_SM_W_IN_DATA_IND:
            case PNDV_AR_SM_W_IN_DATA_IND_RTC3:
            {
                //! in_data indication is in context connect
                pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_AR_IN_DATA);
                pndv_in_al_check_dial_continue();
            }
            // no break
            //lint -fallthrough
            case PNDV_AR_SM_IN_DATA:
            case PNDV_AR_SM_IN_DATA_RTC3:
            {
                PNIO_UINT16 elem_cnt;
                PNIO_UINT16 entity_nr;


                if (  local_ar_in_data->nr_of_elems == 0 ) // all modules pulled or not owned
                {
                    //! all modules pulled or not owned but ar has to go in_data anyway

                    //! shortcut to finish the indication
                    pndv_ar_peri_in_data_done((PNIO_UINT8)ar_idx, 0 , LSA_FALSE);

                }
                else
                {
                    for (elem_cnt = 0; elem_cnt < local_ar_in_data->nr_of_elems; elem_cnt++)
                    {
                        PNDV_IFACE_CMD_ENTRY_T         tmp_event;
                        CM_SV_AR_IN_DATA_ELEMENT_TYPE* act_elem_ptr;
                        PNIO_UINT16                    slot_nr;
                        PNIO_UINT16                    subslot_nr;
                        PNIO_UINT32                    subslot_error;
                        PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

                        act_elem_ptr = &(local_ar_in_data->elem[elem_cnt]);

                        entity_nr = (PNIO_UINT16)PNDV_GET_ENTITY_NR(act_elem_ptr->api, act_elem_ptr->slot_nr, act_elem_ptr->subslot_nr, ar_idx);

                        //! find module in real config
                        slot_nr    = local_ar_in_data->elem[elem_cnt].slot_nr;
                        subslot_nr = local_ar_in_data->elem[elem_cnt].subslot_nr;
                        pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
                        if(subslot_error)
                        {
                            // slot/subslot not possible, must be an implementation error
                            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
                        }

                        if(local_sub_res != 0)
                        {
                            //! submodule is currently bound by cm
                            //! as only plugged subs are indicated no further checks to pp_sm are needed

                            pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_CM_IN_DATA_IND);

                        }

                        //! mark service to be in use
                        pndv_data.serv.ida_serv_run = PNIO_TRUE;


                        //! prepare interface command
                        tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                        tmp_event.add_2 = entity_nr;
                        if (elem_cnt == (local_ar_in_data->nr_of_elems-1))
                        {
                            //! last sub in in data list
                            tmp_event.cmd = PNDV_EV_TO_PERI_SM_IN_DATA;
                        }
                        else
                        {
                            //! there are more subs in in data list
                            tmp_event.cmd = PNDV_EV_TO_PERI_SM_IN_DATA_MORE_FOLLOWS;
                        }

                        pndv_in_peri_write_coupl_event( tmp_event );
                    }
                }

                break;
            }
            case PNDV_AR_SM_W_PRM_END_IND:
            {
                //! in_data indication is in context connect
                //! but no modules in ar, or so locked
                pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_AR_IN_DATA);

                //! mark service to be in use
                pndv_data.serv.ida_serv_run = PNIO_TRUE;
                //! shortcut to finish the indication
                pndv_ar_peri_in_data_done((PNIO_UINT8)ar_idx, 0 , LSA_FALSE);

                break;
            }
            default:
            {
                //! no other case possible
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, pndv_data.ar[ar_idx].sm_state );
                break;
            }
        }
    }
    else
    {
        //! there is still an in_data indication running
        if (pndv_ar_do_service_queue(rqb_ptr, PNDV_PERI_SERVICE_IDENT_IDA) != PNDV_OK)
        {
            //! queueing not possible, no free resource, check PNDV_PERI_SERVICE_QUEUES_T elem
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }
    }
}


/**
 *  @brief in data ind has been processed by peri
 *  @ingroup pndv_ar_in_data
 *
 *  @param[in]  ar_idx    ar index
 *  @param[in]  entity_nr entity number
 *  @param[in]  more_follows PNIO_TRUE = more responses follow, PNIO_FALSE = last response
 *
 *  in_data indication has been processed by peri. more_follows == PNIO_TRUE indicates
 *  the end of the sequence of all submodules indicated in one rqb.
 *
 *  @note rqb is returned by statemachine,
 *
 *
 */
PNIO_VOID pndv_ar_peri_in_data_done(PNIO_UINT8 ar_idx, PNIO_UINT16 entity_nr, PNIO_BOOL more_follows)
{
    PNIO_UINT32                         ar_properties;
    CM_UPPER_RQB_PTR_TYPE               rqb_ptr;
    PNIO_UINT32                         elem_cnt;
    PNIO_UINT16                         slot_nr;
    PNIO_UINT16                         subslot_nr;
    PNIO_UINT32                         subslot_error;
    CM_UPPER_SV_AR_IN_DATA_PTR_TYPE     local_ar_in_data;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;
    CM_UPPER_RQB_PTR_TYPE               tmp_rqb_ptr;

    tmp_rqb_ptr = 0;

    if (more_follows == LSA_TRUE)
    {
        // this is not the last element of the sequence
        // to keep it simple don't do anything here
        // everything is done with the last element of the sequence in a loop

        return;
    }

    ar_properties = pndv_data.ar[ar_idx].ar_properties;

    rqb_ptr = pndv_data.ar[ar_idx].indata_ind_rqb_ptr;

    if (rqb_ptr == 0)
    {
        // state conflict, no in_data req in process
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx );
    }

    local_ar_in_data = rqb_ptr->args.sv.ar_event->u.sv.ar_in_data;

    elem_cnt = 0;

    while( elem_cnt < local_ar_in_data->nr_of_elems)
    {
        //! loop over all elements within in_data req
        //! find module in real config
        slot_nr    = local_ar_in_data->elem[elem_cnt].slot_nr;
        subslot_nr = local_ar_in_data->elem[elem_cnt].subslot_nr;
        pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
        if(subslot_error)
        {
            // slot/subslot not possible, must be an implementation error
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }

        if(local_sub_res != 0)
        {
            //! submodule is currently bound by cm
            //! as only plugged subs are indicated no further checks to pp_sm are needed

            pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_PERI_IN_DATA_IND_DONE);
            // increase element counter
            elem_cnt++;
        }
        else
        {
            //! submodule has been pulled meanwhile
            //! delete this element from response rqb
            for (elem_cnt = 0; elem_cnt < (local_ar_in_data->nr_of_elems - pndv_data.ar[ar_idx].prm_end_rsp_deleted - 1); elem_cnt++)
            {
                // overwrite current element with next (starting element will be deleted)
                local_ar_in_data->elem[elem_cnt] = local_ar_in_data->elem[elem_cnt+1];        /*lint !e661*/
            }

            // decrease the nr of elements within rqb
            local_ar_in_data->nr_of_elems -= 1;
        }
    }

    if (  (PNDV_AR_SM_IN_DATA != pndv_data.ar[ar_idx].sm_state)
        &&(PNDV_AR_SM_IN_DATA_RTC3 != pndv_data.ar[ar_idx].sm_state)
        )
    {
        //! in_data indication is in context connect

        pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_AR_IN_DATA_DONE);

        if (!(CM_AR_PROP_DEVICE_ACCESS & ar_properties))
        {
            /* Only influence BF Led with a regular SINGLE AR (RTC1 / 2) or IOSAR 
               BF Led for RTC3 -> pndv_cm _callback case: CM_OPC_SV_EVENT_AR_RTC3_IND 
               which is not the "first of a companion ar" (first at IRT) */
           pndv_data.ar_in_data_led_used = PNIO_TRUE;

           PNDV_LED_CTRL_DATA_AR_IN_DATA();

        }
    }

    /* check for pending rosal (after AR was set to IN_DATA). Used when Rosal was issued before IN_DATA and queued */
    pndv_in_al_check_rosal_queue();

    PNDV_RQB_SET_RESPONSE(pndv_data.ar[ar_idx].indata_ind_rqb_ptr, CM_OK);

    PNDV_RQB_SET_HANDLE(pndv_data.ar[ar_idx].indata_ind_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
    PNDV_RQB_SET_OPCODE(pndv_data.ar[ar_idx].indata_ind_rqb_ptr, CM_OPC_SV_AR_IN_DATA_RSP);

    PNDV_REQUEST(pndv_data.ar[ar_idx].indata_ind_rqb_ptr, LSA_COMP_ID_CM);

    pndv_data.ar[ar_idx].indata_ind_rqb_ptr = 0;

    pndv_data.serv.ida_serv_run = PNIO_FALSE;
    //! check for waiting prm_end requests
    pndv_ar_peri_service_get_next(&tmp_rqb_ptr, PNDV_PERI_SERVICE_IDENT_IDA);
    if (tmp_rqb_ptr != 0)
    {
        //! there is a waiting request for in_data
        pndv_ar_cm_ar_in_data_ind(tmp_rqb_ptr);
    }
}
//@}

//! @name Receive in red
//@{
/**
 *  @brief process rir.ind of RTC3 AR
 *  @ingroup pndv_ar_rir
 *
 *  @param[in]  rqb_ptr pointer to request block
 *
 *  long_description
 *
 */
PNIO_VOID pndv_ar_cm_sv_event_ar_rir_ind(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 ar_idx;

    ar_idx = pndv_ar_get_ar_idx_by_session_key( rqb_ptr->args.pd.event->session_key );

    if( PNDV_AR_IDX_NOT_USED == ar_idx )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, rqb_ptr->args.pd.event->ar_nr );    /*NOTREACHED*/
    }

    if (rqb_ptr->args.sv.ar_event->u.sv.ar_rir->nr_of_elems == 0)
    {
        //! there must allways be elements
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0 );    /*NOTREACHED*/
    }


#if defined (PNDV_CFG_USE_RED_INT_INFO)
    pndv_data.iface_ptr->ar[ar_idx].red_int_info.RxEndNs                = rqb_ptr->args.sv.ar_event->u.sv.ar_rir->rsv_interval_red.RxEndNs;
    pndv_data.iface_ptr->ar[ar_idx].red_int_info.RxLocalTransferEndNs   = rqb_ptr->args.sv.ar_event->u.sv.ar_rir->rsv_interval_red.RxLocalTransferEndNs;
    pndv_data.iface_ptr->ar[ar_idx].red_int_info.TxEndNs                = rqb_ptr->args.sv.ar_event->u.sv.ar_rir->rsv_interval_red.TxEndNs;
    pndv_data.iface_ptr->ar[ar_idx].red_int_info.TxLocalTransferEndNs   = rqb_ptr->args.sv.ar_event->u.sv.ar_rir->rsv_interval_red.TxLocalTransferEndNs;
    pndv_data.iface_ptr->ar[ar_idx].red_int_info.TxLocalTransferStartNs = rqb_ptr->args.sv.ar_event->u.sv.ar_rir->rsv_interval_red.TxLocalTransferStartNs;

    pndv_data.iface_ptr->ar[ar_idx].red_int_info_used                   = PNIO_TRUE;
#endif

    pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_RIR_IND);

    //! response == CM_OK -> ACK, response unchanged -> ack later
    //PNDV_RQB_SET_RESPONSE( rqb_ptr, CM_OK);

    PNDV_RQB_SET_HANDLE(rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
    PNDV_RQB_SET_OPCODE(rqb_ptr, CM_OPC_SV_AR_RIR_RSP);

    PNDV_REQUEST(rqb_ptr, LSA_COMP_ID_CM);

    /* input update can only be done for ARs with iocr */
    /* check if there can be an input update */
    if (pndv_data.ar[ar_idx].nr_of_iocrs == 0)
    {
        /* no update possible, do so as if there was one */
        pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_FIRST_INPUT_UPDATE_DONE);
    }
}

/**
 *  @brief send rir_ack.req to cm
 *  @ingroup pndv_ar_rir
 *
 *  @param[in]     ar_idx AR Index
 *
 *  prepares and sends a rir acknoledge to cm after the first input update is done
 *
 */
PNIO_VOID pndv_ar_do_rir_ack_req(PNIO_UINT32 ar_idx)
{
    CM_UPPER_RQB_PTR_TYPE       local_rqb_ptr;

    local_rqb_ptr = &pndv_data.rqb.rir_rqb[ar_idx];

    if (LSA_RQB_GET_OPCODE(local_rqb_ptr) != 0)
    {
        //! rqb is in use, state conflict
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, LSA_RQB_GET_OPCODE(local_rqb_ptr) );
    }

    LSA_RQB_SET_OPCODE(local_rqb_ptr, CM_OPC_SV_AR_RIR_ACK);

    local_rqb_ptr->args.sv.ar_rir_ack->device_nr = PNDV_CM_DEVICE_NO;
    local_rqb_ptr->args.sv.ar_rir_ack->session_key = pndv_data.ar[ar_idx].session_key;

    PNDV_RQB_SET_HANDLE(local_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
    PNDV_RQB_SET_OPCODE(local_rqb_ptr, CM_OPC_SV_AR_RIR_ACK);

    PNDV_REQUEST(local_rqb_ptr, LSA_COMP_ID_CM);
}

/**
 *  @brief rir_ack confirmation
 *  @ingroup pndv_ar_rir
 *
 *  @param[in]  rqb_ptr pointer to request block
 *
 *  long_description
 *
 */
PNIO_VOID pndv_ar_cm_sv_rir_ack_cnf(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 ar_idx;

    ar_idx = pndv_ar_get_ar_idx_by_session_key( rqb_ptr->args.sv.ar_rir_ack->session_key );

    if( PNDV_AR_IDX_NOT_USED == ar_idx )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, rqb_ptr->args.pd.event->ar_nr );
    }

    if (    (LSA_RQB_GET_RESPONSE(rqb_ptr) != CM_OK )
          &&(LSA_RQB_GET_RESPONSE(rqb_ptr) != CM_OK_CANCELLED )
        )
    {
        //! error in response
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, LSA_RQB_GET_RESPONSE(rqb_ptr) );
    }


    //! free static rqb by setting opcode to 0
    LSA_RQB_SET_OPCODE(rqb_ptr, 0);

    pndv_ar_sm(ar_idx, PNDV_AR_SM_EVENT_RIR_ACK_CNF);

    /* check for parked alarms and diags */
    pndv_in_al_check_dial_continue();
    pndv_in_al_check_rosal_queue();
    pndv_in_al_check_diag();
}
//@}

/**
 *  @brief indicates the timeout of the sysred data hold timer
 *
 *  @param[in] ar_idx ar index
 *
 *  The data hold timeout of a system redundancy ar-set is watched by peri.
 *  If this function is called, the indicated ar (ar_idx) has a rdht timeout.
 *  The ar must be part of a system redundancy ar-set.
 *
 */
PNIO_VOID pndv_ar_peri_sr_rdht_timeout(PNIO_UINT32 ar_idx)
{
    union
    {
        CM_UPPER_RQB_PTR_TYPE rqb;
        LSA_VOID_PTR_TYPE     void_;
    } ptr;

    if (pndv_data.ar[ar_idx].ar_set_nr != 0)
    {
        PNIO_UINT16 sr_set_idx;
        // this ar is part of a sysred ar_set
        sr_set_idx = (PNIO_UINT16)pndv_ar_get_sr_idx_by_ar_set_nr(pndv_data.ar[ar_idx].ar_set_nr);
        if (sr_set_idx == PNDV_SR_SET_IDX_NOT_USED)
        {
            // must not be here
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }

        pndv_ar_sr_sm(sr_set_idx, PNDV_SR_EVENT_RDHT_TIMEOUT, PNDV_AR_IDX_NOT_USED);

    }

    PNDV_ALLOC_RQB(&ptr.void_, sizeof(CM_RQB_TYPE));

    if (pndv_host_ptr_is_nil__(ptr.rqb))
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    PNDV_ALLOC_MEM(((LSA_VOID_PTR_TYPE)&ptr.rqb->args.sv.arset_abort), sizeof(CM_SV_ARSET_ABORT_TYPE));

    if (pndv_host_ptr_is_nil__(ptr.rqb->args.sv.arset_abort))
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    ptr.rqb->args.sv.arset_abort->arset_nr  = pndv_data.ar[ar_idx].ar_set_nr;
    ptr.rqb->args.sv.arset_abort->device_nr = PNDV_CM_DEVICE_NO;

    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_ARSET_ABORT, pndv_data.ar[ar_idx].ar_set_nr);

    PNDV_RQB_SET_HANDLE(ptr.rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
    PNDV_RQB_SET_OPCODE(ptr.rqb, CM_OPC_SV_ARSET_ABORT);

    PNDV_REQUEST(ptr.rqb, LSA_COMP_ID_CM);
}

PNIO_VOID pndv_ar_cm_sr_rdht_timeout_cnf(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;
    PNIO_UINT16 ar_set_nr;
    PNIO_UINT32 sr_set_idx;
    PNIO_UINT32 tmp_int;

    response = PNDV_RQB_GET_RESPONSE( rqb_ptr);

    if (  (CM_OK           != response )
        &&(CM_OK_CANCELLED != response )
        )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, response);
    }

    ar_set_nr = rqb_ptr->args.sv.arset_abort->arset_nr;

    sr_set_idx = pndv_ar_get_sr_idx_by_ar_set_nr(ar_set_nr);
    if (sr_set_idx == PNDV_SR_SET_IDX_NOT_USED)
    {
        // must not be here
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    pndv_ar_sr_sm(sr_set_idx, PNDV_SR_EVENT_RDHT_TIMEOUT_DONE, PNDV_AR_IDX_NOT_USED);

    PNDV_FREE_MEM(&tmp_int, rqb_ptr->args.sv.arset_abort);
    if ( LSA_OK != tmp_int)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, tmp_int);
    }

    PNDV_FREE_RQB( &tmp_int, rqb_ptr);

    if ( LSA_OK != tmp_int)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, tmp_int);
    }
}

/**
 *  @brief indicates the backup -> primary edge of a sysred ar-set
 *
 *  @param[in]  ar_idx ar index
 *  @param[in]  is_primary indicates a primary edge if PNIO_TRUE, otherwise a backup edge
 *
 *  A system redundancy ar-set has a backup primary edge in the case that both
 *  ars were in backup state.
 *
 */
PNIO_VOID pndv_ar_peri_sr_edge_indication(PNIO_UINT32 ar_idx, PNIO_UINT32 is_primary)
{
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;
    PNIO_UINT16 sr_set_idx = PNDV_SR_SET_IDX_NOT_USED;

    union
    {
        CM_UPPER_RQB_PTR_TYPE rqb;
        LSA_VOID_PTR_TYPE     void_;
    }   ptr;

//#define PNDV_S2_REDUNDANCY_EDGE_WHILE_DISCONNECTING_FIX
#ifdef PNDV_S2_REDUNDANCY_EDGE_WHILE_DISCONNECTING_FIX
   /*having edge event while in PNDV_AR_SM_IN_DATA is OK
     * edge event in any sooner state (building AR) was not observed and shouldn't be possible
     * edge event in PNDV_AR_SM_OFFLINE is working OK
     * but edge in case of disconnecting AR is not treated
     * and should use same approach as PNDV_AR_SM_OFFLINE*/
    if ( (pndv_data.ar[ar_idx].sm_state != PNDV_AR_SM_OFFLINE)
            && (pndv_data.ar[ar_idx].sm_state != PNDV_AR_SM_DISCONNECT_IND_W_DONE)
            && (pndv_data.ar[ar_idx].sm_state != PNDV_AR_SM_W_DISCONNECT_IND_DONE)
        )
#else
     if ( (pndv_data.ar[ar_idx].sm_state != PNDV_AR_SM_OFFLINE)
         )
#endif
    {
        /* ar is not offline */
        if (pndv_data.ar[ar_idx].ar_set_nr != 0)
        {
            // this ar is part of a sysred ar_set
            sr_set_idx = (PNIO_UINT16)pndv_ar_get_sr_idx_by_ar_set_nr(pndv_data.ar[ar_idx].ar_set_nr);
            if (sr_set_idx == PNDV_SR_SET_IDX_NOT_USED)
            {
                // must not be here
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);        /*NOTREACHED*/
            }
        }
        else
        {
            // must not be here
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);            /*NOTREACHED*/
        }

        PNDV_ALLOC_RQB( &ptr.void_, sizeof(CM_RQB_TYPE));

        if ( pndv_host_ptr_is_nil__(ptr.rqb) )
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }

        PNDV_ALLOC_MEM( ((LSA_VOID_PTR_TYPE)&ptr.rqb->args.sv.arset_trigger), sizeof(CM_SV_ARSET_TRIGGER_TYPE));

        if ( pndv_host_ptr_is_nil__(ptr.rqb->args.sv.arset_trigger) )
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }

        if (is_primary)
        {
            /* this ar becomes primary, save this information to handle missleaded requests (e.g. records) */
            /* this can be kept simple, the last primary edge wins */
            pndv_data.sr[sr_set_idx].primary_ar_idx = (PNIO_UINT16)ar_idx; // set the new primary ar the leading one
            /* change owner information for all subs of this ar_set */
            for (PNDV_LIST_EACH(local_sub_res, &pndv_data.cfg.peri_cfg.in_use_list, PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T))
            {
                if (   (local_sub_res->ar_set_nr == pndv_data.ar[ar_idx].ar_set_nr)
                     &&(local_sub_res->res_state > PNDV_CFG_SUBMODULE_RES_STATE_W_OWN)
                    )
                {
                    PNDV_SET_CFG_T* set_cfg_elem_ptr;
                    PNIO_UINT32     entity_nr;

                    entity_nr = PNDV_GET_ENTITY_NR(0 /* API 0 assumed !! */, local_sub_res->sub_module.slot_nr, local_sub_res->sub_module.subslot_nr, ar_idx);
                    set_cfg_elem_ptr = &pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr];

                    //! trigger sm for every element of the add list
                    if(set_cfg_elem_ptr->elem.ownership_passed)
                    {
                        // submodule is part of the new primary AR -> change the owner AR index and indicate "EVENT_CHANGE_OWNER"
                        pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_CHANGE_OWNER);
                        pndv_in_write_debug_buffer_3__(PNDV_DC_COMMON_MODUL_INFO,
                                                       local_sub_res->sub_module.slot_nr,
                                                       local_sub_res->sub_module.subslot_nr,
                                                       ((PNIO_UINT32)local_sub_res->sub_module.mod_ident) | ((PNIO_UINT32)local_sub_res->sub_module.sub_ident) << 16
                                                       );
                        local_sub_res->own_ar_idx = ar_idx;
                    }
                    else
                    {
                        // S1/S2-redundancy: CIR remove   (in R1, res_state is W_OWN -> we don't get here)
                        // submodule is not part of the new primary AR -> CIR remove. We have to reset the sub_res->res_state because
                        // no alarms may be sent anymore over the ar (submodule not owned by primary AR)
                        //    (hint: practically, in CIR, the new backup AR (old primary) is always disconnected after CIR remove)
                        pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_RELEASE);
                        pndv_in_write_debug_buffer_3__(PNDV_DC_COMMON_MODUL_INFO,
                                                       local_sub_res->sub_module.slot_nr,
                                                       local_sub_res->sub_module.subslot_nr,
                                                       ((PNIO_UINT32)local_sub_res->sub_module.mod_ident) | ((PNIO_UINT32)local_sub_res->sub_module.sub_ident) << 16
                                                       );
                            // submodule has no owner anymore, treat it like SO locked or at AR disconnect
                        local_sub_res->own_ar_idx      = PNDV_AR_IDX_NOT_USED;
                        local_sub_res->ar_set_nr       = PNDV_SR_SET_NR_NOT_USED;;
                        local_sub_res->submodule_state = PNDV_SUBMODULE_STATE_NO_OWNER;
                        local_sub_res->para_error      = PNIO_FALSE;
                    }
                }
            }
        }

        if (pndv_data.ar[ar_idx].ar_set_trigger_running  == PNIO_FALSE)
        {
            pndv_data.ar[ar_idx].ar_set_trigger_running = PNIO_TRUE;

            ptr.rqb->args.sv.arset_trigger->session_key = pndv_data.ar[ar_idx].session_key;
            ptr.rqb->args.sv.arset_trigger->device_nr   = PNDV_CM_DEVICE_NO;
            ptr.rqb->args.sv.arset_trigger->is_primary  = (PNIO_UINT8)is_primary;

            pndv_in_write_debug_buffer_all__(PNDV_DC_CM_ARSET_TRIGGER, pndv_data.ar[ar_idx].ar_set_nr);

            PNDV_RQB_SET_HANDLE(ptr.rqb, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
            PNDV_RQB_SET_OPCODE(ptr.rqb, CM_OPC_SV_ARSET_TRIGGER);

            PNDV_REQUEST(ptr.rqb, LSA_COMP_ID_CM);
        }
        else
        {
            /* CM_OPC_SV_ARSET_TRIGGER for this ar is already running */
            pndv_data.ar[ar_idx].ar_set_trigger_new_event        = PNIO_TRUE;
            pndv_data.ar[ar_idx].ar_set_trigger_safed_is_primary = is_primary;
        }
    }
    else
    {
        /* ar is going down don't issue the command */
        PNDV_IFACE_CMD_ENTRY_T  tmp_event;

        tmp_event.add_1 = 0xff;
        tmp_event.add_2 = (PNIO_UINT16)is_primary;
        tmp_event.cmd = PNDV_EV_TO_PERI_SR_EDGE_IND_DONE;

        pndv_in_peri_write_coupl_event(tmp_event);
    }

}

PNIO_VOID pndv_ar_cm_sr_ar_set_trigger_cnf(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32 response;
    PNIO_UINT32 session_key;
    PNIO_UINT16 sr_set_idx;
    PNIO_UINT16 ar_idx;

    PNDV_IFACE_CMD_ENTRY_T  tmp_event;
    PNIO_UINT32             tmp_int;

    response = PNDV_RQB_GET_RESPONSE( rqb_ptr);

    if (  (CM_OK != response )              // OK
        &&(CM_OK_CANCELLED != response )    // OK but AR no longer existent
        )
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, response);
    }

    session_key = rqb_ptr->args.sv.arset_trigger->session_key;

    ar_idx = (PNIO_UINT16)pndv_ar_get_ar_idx_by_session_key((PNIO_UINT16)session_key);
    if (ar_idx >= PNDV_CM_AR_NO)
    {
        // must not be here
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }
    sr_set_idx = (PNIO_UINT16)pndv_ar_get_sr_idx_by_ar_set_nr(pndv_data.ar[ar_idx].ar_set_nr);
    if (sr_set_idx == PNDV_SR_SET_IDX_NOT_USED)
    {
        // must not be here
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    pndv_data.ar[ar_idx].ar_set_trigger_running = PNIO_FALSE;

    tmp_event.add_1 = (PNIO_UINT8)  ar_idx;
    tmp_event.add_2 = (PNIO_UINT16) rqb_ptr->args.sv.arset_trigger->is_primary;

    PNDV_FREE_MEM(&tmp_int, rqb_ptr->args.sv.arset_trigger);
    if ( LSA_OK != tmp_int)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, tmp_int);
    }

    PNDV_FREE_RQB( &tmp_int, rqb_ptr);

    if ( LSA_OK != tmp_int)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, tmp_int);
    }

    tmp_event.cmd = PNDV_EV_TO_PERI_SR_EDGE_IND_DONE;

    pndv_in_peri_write_coupl_event( tmp_event );

    if (pndv_data.ar[ar_idx].ar_set_trigger_new_event == PNIO_TRUE)
    {
        pndv_data.ar[ar_idx].ar_set_trigger_new_event = PNIO_FALSE;
        if (pndv_data.ar[ar_idx].ar_set_trigger_safed_is_primary != tmp_event.add_2)
        {
            pndv_ar_peri_sr_edge_indication(ar_idx, pndv_data.ar[ar_idx].ar_set_trigger_safed_is_primary);
        }

    }
}

/*
 * called in R1 redundancy at switchover (B->P / P->B).
 */
PNIO_VOID pndv_ar_peri_sr_owner_takeover_ind(PNIO_UINT32 ar_idx, PNIO_UINT32 is_owner)
{
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;
    PNIO_UINT32 sr_set_idx = PNDV_SR_SET_IDX_NOT_USED;

    if ( (pndv_data.ar[ar_idx].sm_state != PNDV_AR_SM_OFFLINE)
       )
    {
        /* ar is not going down */
        if (pndv_data.ar[ar_idx].ar_set_nr != 0)
        {
            // this ar is part of a sysred ar_set
            sr_set_idx = pndv_ar_get_sr_idx_by_ar_set_nr(pndv_data.ar[ar_idx].ar_set_nr);
            if (sr_set_idx == PNDV_SR_SET_IDX_NOT_USED)
            {
                // must not be here
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);     /*NOTREACHED*/
            }
        }
        else
        {
            // must not be here
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);         /*NOTREACHED*/
        }

        if (is_owner)
        {
            /* this ar becomes primary, save this information to handle missleaded requests (e.g. records) */
            /* this can be kept simple, the last primary edge wins */
            pndv_data.sr[sr_set_idx].primary_ar_idx = (PNIO_UINT16)ar_idx; // set the new primary ar the leading one
            /* change owner information for all subs of this ar_set */
            for (PNDV_LIST_EACH(local_sub_res, &pndv_data.cfg.peri_cfg.in_use_list, PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T))
            {
                if (   (local_sub_res->ar_set_nr == pndv_data.ar[ar_idx].ar_set_nr)
                     &&(local_sub_res->res_state >= PNDV_CFG_SUBMODULE_RES_STATE_W_OWN_PASSIV)
                    )
                {
                    //! trigger sm for every element of the add list
                    pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_CHANGE_OWNER);
                    pndv_in_write_debug_buffer_3__(PNDV_DC_COMMON_MODUL_INFO,
                                                   local_sub_res->sub_module.slot_nr,
                                                   local_sub_res->sub_module.subslot_nr,
                                                   ((PNIO_UINT32)local_sub_res->sub_module.mod_ident) | ((PNIO_UINT32)local_sub_res->sub_module.sub_ident) << 16
                                                   );
                    local_sub_res->own_ar_idx = ar_idx;
                }
                // else: submodule doesn't belong to AR-Set (might also be CIR-Remove in R1). Nothing to be done (if this was
                //       a CIR-removed submodule on this side, it didn't get an ownership anyway and just stays in W_OWN)
            }
        }
        else
        {
            pndv_data.sr[sr_set_idx].primary_ar_idx = PNDV_AR_IDX_NOT_USED; // set the new primary ar the leading one

            /* change owner information for all subs of this ar_set */
            for (PNDV_LIST_EACH(local_sub_res, &pndv_data.cfg.peri_cfg.in_use_list, PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T))
            {
                if (   (local_sub_res->ar_set_nr == pndv_data.ar[ar_idx].ar_set_nr)
                    )
                {
                    //! trigger sm for every element of the add list
                    pndv_pp_sm(local_sub_res, PNDV_CFG_SUBMODULE_RES_EVENT_R1_PASSIV_OWNER);
                    pndv_in_write_debug_buffer_3__(PNDV_DC_COMMON_MODUL_INFO,
                                                   local_sub_res->sub_module.slot_nr,
                                                   local_sub_res->sub_module.subslot_nr,
                                                   ((PNIO_UINT32)local_sub_res->sub_module.mod_ident) | ((PNIO_UINT32)local_sub_res->sub_module.sub_ident) << 16
                                                   );
                    local_sub_res->own_ar_idx = PNDV_AR_IDX_NOT_USED;
                }
                // else: submodule doesn't belong to AR-set (might also be CIR-remove in R1). Nothing to be done as above.
            }
        }
    }


    {

        PNDV_IFACE_CMD_ENTRY_T  tmp_event;

        tmp_event.add_1 = (PNIO_UINT8)  ar_idx;
        tmp_event.add_2 = (PNIO_UINT16) is_owner;
        tmp_event.cmd = PNDV_EV_TO_PERI_SR_OWNER_TAKEOVER_DONE;

        pndv_in_peri_write_coupl_event( tmp_event );
    }

}

//! @name Service queue
//@{
/**
 *  @brief queues an rqb to addressed service queue
 *  @ingroup pndv_ar_man
 *
 *  @param[in]     rqb_ptr pointer to request block that should be queued in
 *  @param[in]  serv_id ID of the service queue to put the rqb in
 *  @return        PNDV_OK = queueing successfull, PNDV_ERR_RESOURCE = no free queueing resource
 *
 *  long_description
 *
 */
PNIO_UINT32 pndv_ar_do_service_queue(CM_UPPER_RQB_PTR_TYPE rqb_ptr, PNDV_PERI_SERVICE_IDENT_T serv_id)
{
    PNDV_LIST_ENTRY_TYPE* local_list_ptr = NULL;
    PNDV_PERI_SERVICE_ELEMENT_PTR_T local_serv_elem_ptr;

    //! get an service-resource from free list

    local_serv_elem_ptr = PNDV_LIST_FIRST(&pndv_data.serv.free_list, PNDV_PERI_SERVICE_ELEMENT_PTR_T);
    if ( local_serv_elem_ptr == LSA_NULL )
    {
        //! no more free resources, service can not be queued
        return PNDV_ERR_RESOURCE;
    }

    switch (serv_id)
    {
        case PNDV_PERI_SERVICE_IDENT_CON:
        {
            local_list_ptr = &pndv_data.serv.con_wait_list;
            break;
        }
        case PNDV_PERI_SERVICE_IDENT_OWN:
        {
            local_list_ptr = &pndv_data.serv.own_wait_list;
            break;
        }
        case PNDV_PERI_SERVICE_IDENT_PRM:
        {
            local_list_ptr = &pndv_data.serv.prm_wait_list;
            break;
        }
        case PNDV_PERI_SERVICE_IDENT_IDA:
        {
            local_list_ptr = &pndv_data.serv.ida_wait_list;
            break;
        }
        default:
        {
            //! unknown service queue, must not happen
            return PNDV_ERR_NOT_SUPPORTED;
        }
    }

    //! remove entry from free list
    PNDV_LIST_REMOVE_ENTRY(&local_serv_elem_ptr->link);

    local_serv_elem_ptr->rqb_ptr = rqb_ptr;

    //! append to corresponding wait list
    PNDV_ASSERT(local_list_ptr != LSA_NULL)
    PNDV_LIST_INSERT_TAIL(local_list_ptr, &local_serv_elem_ptr->link);

    return PNDV_OK;
}


/**
 *  @brief get the next waiting request of the addressed service queue
 *  @ingroup pndv_ar_man
 *
 *  @param[in]  serv_id     ID of the service queue to get the next request from
 *  @param[out] rqb_ptr_ptr pointer to the rqb of a waiting request, if no request is waiting a nil-pointer is returned
 *
 *  long_description
 *
 */
PNIO_VOID pndv_ar_peri_service_get_next(CM_UPPER_RQB_PTR_TYPE *rqb_ptr_ptr, PNDV_PERI_SERVICE_IDENT_T serv_id)
{
    PNDV_LIST_ENTRY_TYPE* local_list_ptr = NULL;
    PNDV_PERI_SERVICE_ELEMENT_PTR_T local_serv_elem_ptr;


    switch (serv_id)
    {
        case PNDV_PERI_SERVICE_IDENT_CON:
        {
            local_list_ptr = &pndv_data.serv.con_wait_list;
            break;
        }
        case PNDV_PERI_SERVICE_IDENT_OWN:
        {
            local_list_ptr = &pndv_data.serv.own_wait_list;
            break;
        }
        case PNDV_PERI_SERVICE_IDENT_PRM:
        {
            local_list_ptr = &pndv_data.serv.prm_wait_list;
            break;
        }
        case PNDV_PERI_SERVICE_IDENT_IDA:
        {
            local_list_ptr = &pndv_data.serv.ida_wait_list;
            break;
        }
        default:
        {
            //! unknown service queue, must not happen
            *rqb_ptr_ptr = 0;
            return;
        }
    }

    PNDV_ASSERT(local_list_ptr != LSA_NULL);
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
        PNDV_LIST_INSERT_TAIL(&pndv_data.serv.free_list, &local_serv_elem_ptr->link);
    }
}
//@}


/**
 * @brief function writes a new peri-event to user  or queues it
 * @param ev_queue      queue to be used
 * @param event         event to be written to peri
 * @param ar_idx        ar_idx of peri-event
 */
PNIO_VOID pndv_pevent_serialization_new_event_to_user(PNDV_PERI_EVENT_SERIALIZATION_T* ev_queue, PNDV_IFACE_CMD_ENTRY_T event, PNIO_UINT8 ar_idx)
{
    if(ar_idx >= sizeof(ev_queue->event_array) / sizeof(ev_queue->event_array[0]))
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);
    }
    if(ev_queue->event_array[ar_idx].in_use)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);
    }

    ev_queue->event_array[ar_idx].event  = event;
    ev_queue->event_array[ar_idx].in_use = PNIO_TRUE;
    ev_queue->in_use_count++;
    if(ev_queue->in_use_count == 1)
    {
        ev_queue->event_array[ar_idx].sent_to_user = PNIO_TRUE;
        pndv_in_peri_write_coupl_event( event );
    }
    else
    {
        ev_queue->event_array[ar_idx].sent_to_user = PNIO_FALSE;
    }
}

/**
 * @brief function acknowledges an event that has been processed by the user and optionally triggers the same (queued) event for another AR
 * @param ev_queue      queue to be used
 * @param ar_idx        ar_idx of peri-event
 */
PNIO_VOID pndv_pevent_serialization_event_done(PNDV_PERI_EVENT_SERIALIZATION_T* ev_queue, PNIO_UINT8 ar_idx)
{
    if(ar_idx >= sizeof(ev_queue->event_array) / sizeof(ev_queue->event_array[0]))
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);
    }
    if(!ev_queue->event_array[ar_idx].in_use)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);
    }
    if(ev_queue->in_use_count == 0)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);
    }

    ev_queue->event_array[ar_idx].in_use       = PNIO_FALSE;
    ev_queue->event_array[ar_idx].sent_to_user = PNIO_FALSE;
    ev_queue->in_use_count--;
    if (ev_queue->in_use_count > 0)
    {
        PNIO_UINT8 other_ar_idx;
        for (other_ar_idx = 0; other_ar_idx < PNDV_CM_AR_NO; other_ar_idx++)
        {
            if (ev_queue->event_array[other_ar_idx].in_use)
            {
                ev_queue->event_array[other_ar_idx].sent_to_user = PNIO_TRUE;
                pndv_in_peri_write_coupl_event( ev_queue->event_array[other_ar_idx].event );
                break;
            }
        }
        if (other_ar_idx >= PNDV_CM_AR_NO)
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);
        }
    }
}

/**
 * @brief   returns if a peri-event is currently processed by the user
 * @return  PNIO_TRUE if the event for <ar_idx> is currently active&processed by user
 */
PNIO_BOOL pndv_pevent_serialization_is_event_processing(PNDV_PERI_EVENT_SERIALIZATION_T* ev_queue, PNIO_UINT8 ar_idx)
{
    PNIO_BOOL is_event_processing = PNIO_FALSE;
    if (ev_queue->event_array[ar_idx].in_use && ev_queue->event_array[ar_idx].sent_to_user)
    {
        is_event_processing = PNIO_TRUE;
    }
    return is_event_processing;
}

/**
 * @brief   returns if a peri-event is queued and not yet processed by the user
 */
PNIO_BOOL pndv_pevent_serialization_is_event_queued(PNDV_PERI_EVENT_SERIALIZATION_T* ev_queue, PNIO_UINT8 ar_idx)
{
    PNIO_BOOL is_event_queued = PNIO_FALSE;
    if (ev_queue->event_array[ar_idx].in_use && !ev_queue->event_array[ar_idx].sent_to_user)
    {
        is_event_queued = PNIO_TRUE;
    }
    return is_event_queued;
}

/**
 * @brief   function removes a queued event
 */
PNIO_VOID pndv_pevent_serialization_remove_event(PNDV_PERI_EVENT_SERIALIZATION_T* ev_queue, PNIO_UINT8 ar_idx)
{
    if (ar_idx >= sizeof(ev_queue->event_array) / sizeof(ev_queue->event_array[0]))
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);
    }
    if (!ev_queue->event_array[ar_idx].in_use || ev_queue->event_array[ar_idx].sent_to_user)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);
    }
    if (ev_queue->in_use_count < 1)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);
    }

    ev_queue->event_array[ar_idx].in_use       = PNIO_FALSE;
    ev_queue->event_array[ar_idx].sent_to_user = PNIO_FALSE;
    ev_queue->in_use_count--;
}


//! @name AR management
//@{
/**
 *
 *  long_description
 *
 */

/** @brief State machine for ar state.
 *  @ingroup pndv_ar_man
 *
 *  @param[in]  ar_idx ar index
 *  @param[in]  event event to statemachine
 *
 *  According to the manner of object oriented programming this function is
 *  commonly a method that handles the given ar_idx as a reference to a
 *  ar resource object.
 *  The state of the ar state machine is evaluated and the given event
 *  is handled in the appropriate case.
 *
 *  To get a graphical overview of the state flow you may refer to
 *  <A HREF="file:\\R:\Firmware\pndv.cmp\doc\PNDV_V2\pndv_sm_ar.pdf">pndv_sm_ar.pdf (absolut local)</A>.<br>
 *  If the link is not reachable you can find a copy in the components doc
 *  directory.
 *
 *  For a detailed description of the states please refer to @ref PNDV_AR_SM_STATE_E
 *
 *  For a detailed description of the events please refer to @ref PNDV_AR_SM_EVENT_E
 *
 */
PNIO_VOID pndv_ar_sm(PNIO_UINT32 ar_idx,PNDV_AR_SM_EVENT_E_T event)
{
    PNDV_AR_SM_STATE_T *ar_sm_state_ptr;


    if (ar_idx >= PNDV_CM_AR_NO + 1)
    {
        // ar_idx out of range
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);        /*NOTREACHED*/
    }

    ar_sm_state_ptr = &pndv_data.ar[ar_idx].sm_state;

    pndv_in_write_debug_buffer_3__(PNDV_DC_PNDV_AR_SM, event, *ar_sm_state_ptr, ar_idx);

    switch (*ar_sm_state_ptr)
    {

        case PNDV_AR_SM_OFFLINE:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_CONNECT_IND:
                {

                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_SESSION_START;

                    /* activity */

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
        case PNDV_AR_SM_SESSION_START:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_CONNECT_IND_OK_DONE:
                {
                    //< Connect successfully processed by pndv
                    PNDV_IFACE_CMD_ENTRY_T tmp_event;
                    PNIO_UINT32            i;

                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_CONNECT_IND_PERI_DONE;
                    /* activity */

                    if (ar_idx >= PNDV_CM_AR_NO)
                    {
                        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);        /*NOTREACHED*/
                    }
                    pndv_data.iface_ptr->ar[ar_idx].sr_rdht_msec  = pndv_data.ar[ar_idx].sr_rdht_msec;
                    pndv_data.iface_ptr->ar[ar_idx].sr_properties = pndv_data.ar[ar_idx].sr_properties;
                    pndv_data.iface_ptr->ar[ar_idx].sr_ar_set_nr  = pndv_data.ar[ar_idx].ar_set_nr;
                    pndv_data.iface_ptr->ar[ar_idx].sr_firstAR    = pndv_data.ar[ar_idx].sr_firstAR;
                    pndv_data.iface_ptr->ar[ar_idx].cmi_obj_uuid  = pndv_data.ar[ar_idx].cmi_obj_uuid;
                    pndv_data.iface_ptr->ar[ar_idx].host_ip       = pndv_data.ar[ar_idx].host_ip;
                    pndv_data.iface_ptr->ar[ar_idx].host_name     = pndv_data.ar[ar_idx].host_name;
                    pndv_data.iface_ptr->ar[ar_idx].ar_session_key= pndv_data.ar[ar_idx].session_key;
                    pndv_data.iface_ptr->ar[ar_idx].input_cr_io_data_length  = pndv_data.ar[ar_idx].input_cr_io_data_length;
                    pndv_data.iface_ptr->ar[ar_idx].input_cr_ioxs_length     = pndv_data.ar[ar_idx].input_cr_ioxs_length;
                    pndv_data.iface_ptr->ar[ar_idx].output_cr_io_data_length = pndv_data.ar[ar_idx].output_cr_io_data_length;
                    pndv_data.iface_ptr->ar[ar_idx].output_cr_ioxs_length    = pndv_data.ar[ar_idx].output_cr_ioxs_length;
                    pndv_data.iface_ptr->ar[ar_idx].ar_fsu_enable = pndv_data.ar[ar_idx].ar_fsu_enable;

                    #ifdef PNDV_CFG_ENABLE_RS_INFO_BLOCK_SUPPORT
                    pndv_data.iface_ptr->ar[ar_idx].has_RSInfoBlock = pndv_data.ar[ar_idx].has_RSInfoBlock;
                    #endif
                    PNDV_COPY_BYTE(pndv_data.iface_ptr->ar[ar_idx].ar_fsu_uuid, pndv_data.ar[ar_idx].ar_fsu_uuid, 16);

                    pndv_data.iface_ptr->ar[ar_idx].send_clock =  pndv_data.ar[ar_idx].send_clock;
                    for (i = 0; i < PNDV_MAX_IOCR_PER_AR; i++)
                    {
                        pndv_data.iface_ptr->ar[ar_idx].reduction_ratio[i] = pndv_data.ar[ar_idx].reduction_ratio[i];
                    }

                    tmp_event.cmd   = PNDV_EV_TO_PERI_CONNECT_IND;
                    tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                    tmp_event.add_2 = pndv_data.ar[ar_idx].ar_type;

                    pndv_in_peri_write_coupl_event( tmp_event );

                    break;
                }
                case PNDV_AR_SM_EVENT_CONNECT_IND_ERROR_DONE:
                {
                    //< Connect was rejected

                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_OFFLINE;

                    /* activity */
                    pndv_data.ar[ar_idx].ar_nr  = PNDV_AR_NR_NOT_USED;
                    PNDV_RQB_SET_RESPONSE(pndv_data.ar[ar_idx].con_ind_rqb_ptr, CM_ERR_RESOURCE);

                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_CONNECT_RSP_SYNC, pndv_data.ar[ar_idx].sm_state);

                    PNDV_RQB_SET_HANDLE(pndv_data.ar[ar_idx].con_ind_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                    PNDV_RQB_SET_OPCODE(pndv_data.ar[ar_idx].con_ind_rqb_ptr, CM_OPC_SV_AR_CONNECT_RSP);

                    PNDV_REQUEST(pndv_data.ar[ar_idx].con_ind_rqb_ptr, LSA_COMP_ID_CM);
                    break;
                }
                //< this does not make sense, connect has not been responded yet
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                // no break
                default:
                {
                    // unexpected event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_AR_SM_W_CONNECT_IND_PERI_DONE:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_CONNECT_IND_PERI_OK_DONE:
                {
                    //< Connect was accepted by peri
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_OWNERSHIP_IND;

                    /* activity */
                    PNDV_RQB_SET_RESPONSE(pndv_data.ar[ar_idx].con_ind_rqb_ptr, CM_OK);

                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_CONNECT_RSP_ASYNC, pndv_data.ar[ar_idx].sm_state);

                    PNDV_RQB_SET_HANDLE(pndv_data.ar[ar_idx].con_ind_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                    PNDV_RQB_SET_OPCODE(pndv_data.ar[ar_idx].con_ind_rqb_ptr, CM_OPC_SV_AR_CONNECT_RSP);

                    PNDV_REQUEST(pndv_data.ar[ar_idx].con_ind_rqb_ptr, LSA_COMP_ID_CM);
                    break;
                }
                case PNDV_AR_SM_EVENT_CONNECT_IND_PERI_ERROR_DONE:
                case PNDV_AR_SM_EVENT_CONNECT_IND_PERI_RESOURCE_ERROR_DONE:
                {
                    //< Connect was rejected by peri
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_DISCONNECT_IND_DONE__CONTEXT_CONNECT;

                    /* activity */
                        /* prepare connect.rsp for later response */
                    pndv_data.ar[ar_idx].ar_nr  = PNDV_AR_NR_NOT_USED;
                    if(event == PNDV_AR_SM_EVENT_CONNECT_IND_PERI_ERROR_DONE)
                    {
                            /* AR rejected -> User abort */
                        PNDV_RQB_SET_RESPONSE(pndv_data.ar[ar_idx].con_ind_rqb_ptr, CM_ERR_RESOURCE);
                    }
                    else
                    {
                            /* AR rejected -> AR parameters violate GSD MaxApplicationXXXLength */
                        PNDV_RQB_SET_RESPONSE(pndv_data.ar[ar_idx].con_ind_rqb_ptr, CM_ERR_APPL_RESOURCE);
                    }

                        /* issue disconnect to user (before responding to CM) */
                    {
                        PNDV_IFACE_CMD_ENTRY_T tmp_event;

                        // internally generated Disconnect.Ind is issued to user after Connect.Ind-reject!
                        // -> user + PNDV can clean up data structures / IO memory

                        // prepare interface command
                        tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                        tmp_event.add_2 = 0;

                        tmp_event.cmd = PNDV_EV_TO_PERI_AR_DISCONNECT_IND;

                        pndv_in_peri_write_coupl_event( tmp_event );
                    }

                    break;
                }
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                {
                    //< this does not make sense, connect has not been responded yet
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
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
        case PNDV_AR_SM_W_DISCONNECT_IND_DONE__CONTEXT_CONNECT:
        {
            /* connect was rejected by user, now waiting for disconnect response ... */
            switch (event)
            {
                case PNDV_AR_SM_EVENT_DISCONNECT_IND_DONE:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_OFFLINE;

                    /* activity */

                    //check for sr issues to be handled
                    if (pndv_data.ar[ar_idx].ar_set_nr != 0)
                    {
                        PNIO_UINT32 sr_set_idx;
                        // this ar is part of a sysred ar_set
                        sr_set_idx = pndv_ar_get_sr_idx_by_ar_set_nr(pndv_data.ar[ar_idx].ar_set_nr);
                        if (sr_set_idx == PNDV_SR_SET_IDX_NOT_USED)
                        {
                            // must not be here
                            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
                        }

                        pndv_ar_sr_sm(sr_set_idx, PNDV_SR_EVENT_REMOVE_DONE, ar_idx);
                    }

                    pndv_in_write_debug_buffer_all__(PNDV_DC_CM_CONNECT_RSP_ASYNC, pndv_data.ar[ar_idx].sm_state);

                        // response code has been prepared in state PNDV_AR_SM_W_CONNECT_IND_PERI_DONE already ...
                    //PNDV_RQB_SET_RESPONSE()

                    PNDV_RQB_SET_HANDLE(pndv_data.ar[ar_idx].con_ind_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                    PNDV_RQB_SET_OPCODE(pndv_data.ar[ar_idx].con_ind_rqb_ptr, CM_OPC_SV_AR_CONNECT_RSP);

                    PNDV_REQUEST(pndv_data.ar[ar_idx].con_ind_rqb_ptr, LSA_COMP_ID_CM);
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
        case PNDV_AR_SM_W_OWNERSHIP_IND:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_OWNERSHIP_IND:
                {
                    // Ownership.ind in context of connect arrived
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_OWNERSHIP_IND_DONE;

                    break;
                }
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_DISCONNECT_IND_DONE;

                    /* activity */

                    {
                        PNDV_IFACE_CMD_ENTRY_T tmp_event;

                        // for RTC1/2 AR request input update now

                        // prepare interface command
                        tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                        if (pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr)
                        {
                            tmp_event.add_2 = (PNIO_UINT16)pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr->args.pd.event->u.sv.ar_disconnect->disconnect.reason_code;
                        }
                        else
                        {
                            tmp_event.add_2 = 0;
                        }
                        tmp_event.cmd = PNDV_EV_TO_PERI_AR_DISCONNECT_IND;

                        pndv_in_peri_write_coupl_event( tmp_event );
                    }

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
        case PNDV_AR_SM_W_OWNERSHIP_IND_DONE:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_OWNERSHIP_IND_DONE:
                {
                    // ownership request has been fully processed by peri

                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_PRM_END_IND;

                    /* activity */

                    break;
                }
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_DISCONNECT_IND_W_DONE;

                    /* activity */

                    // don't cleanup ar data before PNDV_AR_SM_EVENT_OWNERSHIP_IND_DONE


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
        case PNDV_AR_SM_W_PRM_END_IND:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_PRM_END_IND:
                {
                    // Ownership.ind in context of connect arrived
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_PRM_END_IND_DONE;

                    break;
                }
                case PNDV_AR_SM_EVENT_AR_IN_DATA:
                {
                    // in_data in context connect arrived
                    // only possible here if ar is so_locked or has no modules
                    if (pndv_data.ar[ar_idx].ar_type == CM_AR_TYPE_SINGLE_RTC3)
                    {
                        pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_IN_DATA_IND_DONE_RTC3;
                    }
                    else
                    {
                        /* all other ar types   CM_AR_TYPE_SINGLE, CM_AR_TYPE_SUPERVISOR, CM_AR_TYPE_SINGLE_SYSRED */
                        pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_IN_DATA_IND_DONE;
                    }

                    break;
                }
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_DISCONNECT_IND_DONE;

                    /* activity */

                    {
                        PNDV_IFACE_CMD_ENTRY_T tmp_event;

                        // for RTC1/2 AR request input update now

                        // prepare interface command
                        tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                        if (pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr)
                        {
                            tmp_event.add_2 = (PNIO_UINT16)pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr->args.pd.event->u.sv.ar_disconnect->disconnect.reason_code;
                        }
                        else
                        {
                            tmp_event.add_2 = 0;
                        }
                        tmp_event.cmd = PNDV_EV_TO_PERI_AR_DISCONNECT_IND;

                        pndv_in_peri_write_coupl_event( tmp_event );
                    }

                    break;
                }
                case PNDV_AR_SM_EVENT_FIRST_INPUT_UPDATE_DONE:
                {
                    // first update of inputes was done
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_IN_DATA_IND;


                    /* activity */

                    // this branch of the sm can only be reached by ar's without good modules or so-locked
                    // no appl-ready needed in this case

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
        case PNDV_AR_SM_W_PRM_END_IND_DONE:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_PRM_END_IND_DONE:
                {
                    // prm_end request has been fully processed by peri

                    if (pndv_data.ar[ar_idx].ar_type == CM_AR_TYPE_SINGLE_RTC3)
                    {
                        pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_RIR_IND;
                    }
                    else
                    {
                        /* all other ar types   CM_AR_TYPE_SINGLE, CM_AR_TYPE_SUPERVISOR, CM_AR_TYPE_SINGLE_SYSRED */
                        pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_INPUT_UPDATE;
                    }

                    /* activity */

                    if(pndv_data.ar[ar_idx].ar_type != CM_AR_TYPE_SINGLE_RTC3)
                    {
                        PNDV_IFACE_CMD_ENTRY_T tmp_event;

                        // for RTC1/2 AR request input update now
                        // prepare interface command
                        tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                        tmp_event.add_2 = 0;
                        tmp_event.cmd = PNDV_EV_TO_PERI_READY_FOR_INPUT_UPDATE;

                        /* only send if this ar has iocr */
                        if (pndv_data.ar[ar_idx].nr_of_iocrs != 0)
                        {
                            // user expects one READY_FOR_INPUT_UPATE at a time -> serialize
                            //pndv_in_peri_write_coupl_event( tmp_event );
                            pndv_pevent_serialization_new_event_to_user( &pndv_data.ev_queue_ready_for_input_update, tmp_event, (PNIO_UINT8)ar_idx );
                        }
                    }

                    break;
                }
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_DISCONNECT_IND_W_DONE;

                    /* activity */

                    // don't cleanup ar data yet, wait for PNDV_AR_SM_EVENT_PRM_END_IND_DONE

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

        // RTC3
        case PNDV_AR_SM_W_RIR_IND:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_RIR_IND:
                {
                    PNDV_IFACE_CMD_ENTRY_T tmp_event;

                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_INPUT_UPDATE_RTC3;

                    /* activity */
                    if (ar_idx >= PNDV_CM_AR_NO)
                    {
                        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);        /*NOTREACHED*/
                    }

                    pndv_data.iface_ptr->ar[ar_idx].current_rt_class = pndv_data.ar[ar_idx].current_rt_class;

                    if ( PNDV_RT_NONE != pndv_data.iface_ptr->ar[ar_idx].current_rt_class )
                    {
                        /* if an RT Class! = NONE is entered, the previous_rt_class is also set,  
                           which ensures that if the AR is ended immediately and thus RT_NONE is entered in current_rt_class, 
                           the slave side has a difference between previous (RTCx) and current (RT_NONE) */
                        pndv_data.iface_ptr->ar[ar_idx].previous_rt_class = pndv_data.iface_ptr->ar[ar_idx].current_rt_class;
                    }

                    // for RTC3 AR request input update now
                    // prepare interface command
                    tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                    tmp_event.add_2 = 0;
                    tmp_event.cmd   = PNDV_EV_TO_PERI_READY_FOR_INPUT_UPDATE;

                    /* only send if this ar has iocr */
                    if (pndv_data.ar[ar_idx].nr_of_iocrs != 0)
                    {
                        // user expects one READY_FOR_INPUT_UPATE at a time -> serialize
                        //pndv_in_peri_write_coupl_event( tmp_event );
                        pndv_pevent_serialization_new_event_to_user( &pndv_data.ev_queue_ready_for_input_update, tmp_event, (PNIO_UINT8)ar_idx );
                    }

                    break;
                }
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_DISCONNECT_IND_DONE;

                    /* activity */

                    {
                        PNDV_IFACE_CMD_ENTRY_T tmp_event;

                        // for RTC1/2 AR request input update now

                        // prepare interface command
                        tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                        if (pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr)
                        {
                            tmp_event.add_2 = (PNIO_UINT16)pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr->args.pd.event->u.sv.ar_disconnect->disconnect.reason_code;
                        }
                        else
                        {
                            tmp_event.add_2 = 0;
                        }
                        tmp_event.cmd = PNDV_EV_TO_PERI_AR_DISCONNECT_IND;

                        pndv_in_peri_write_coupl_event( tmp_event );
                    }

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
        case PNDV_AR_SM_W_INPUT_UPDATE_RTC3:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_FIRST_INPUT_UPDATE_DONE:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_RIR_ACK_CNF;

                    /* activity */

                    pndv_ar_do_rir_ack_req(ar_idx);

                    // free input-update-serialization resource and check if other ARs have queued ready-for-input events
                    pndv_pevent_serialization_event_done(&pndv_data.ev_queue_ready_for_input_update, (PNIO_UINT8)ar_idx);

                    break;
                }
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_DISCONNECT_IND_W_DONE;

                    /* activity */

                    // wait for input update to finish

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
        case PNDV_AR_SM_W_RIR_ACK_CNF:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_RIR_ACK_CNF:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_IN_DATA_IND_RTC3;

                    break;
                }
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_DISCONNECT_IND_W_DONE;

                    /* activity */

                    // wait for rir_ack confirmation

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
        case PNDV_AR_SM_W_IN_DATA_IND_RTC3:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_AR_IN_DATA:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_IN_DATA_IND_DONE_RTC3;

                    break;
                }
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_DISCONNECT_IND_DONE;

                    /* activity */

                    {
                        PNDV_IFACE_CMD_ENTRY_T tmp_event;

                        // for RTC1/2 AR request input update now

                        // prepare interface command
                        tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                        if (pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr)
                        {
                            tmp_event.add_2 = (PNIO_UINT16)pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr->args.pd.event->u.sv.ar_disconnect->disconnect.reason_code;
                        }
                        else
                        {
                            tmp_event.add_2 = 0;
                        }
                        tmp_event.cmd = PNDV_EV_TO_PERI_AR_DISCONNECT_IND;

                        pndv_in_peri_write_coupl_event( tmp_event );
                    }

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
        case PNDV_AR_SM_W_IN_DATA_IND_DONE_RTC3:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_AR_IN_DATA_DONE:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_IN_DATA_RTC3;

                    /* activity */

                    {
                        PNDV_IFACE_CMD_ENTRY_T tmp_event;

                        // prepare interface command
                        tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                        tmp_event.add_2 = 0;
                        tmp_event.cmd   = PNDV_EV_TO_PERI_AR_IN_DATA;

                        pndv_in_peri_write_coupl_event( tmp_event );

                    }

                    break;
                }
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_DISCONNECT_IND_DONE;

                    /* activity */

                    {
                        PNDV_IFACE_CMD_ENTRY_T tmp_event;

                        // for RTC1/2 AR request input update now

                        // prepare interface command
                        tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                        if (pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr)
                        {
                            tmp_event.add_2 = (PNIO_UINT16)pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr->args.pd.event->u.sv.ar_disconnect->disconnect.reason_code;
                        }
                        else
                        {
                            tmp_event.add_2 = 0;
                        }
                        tmp_event.cmd = PNDV_EV_TO_PERI_AR_DISCONNECT_IND;

                        pndv_in_peri_write_coupl_event( tmp_event );
                    }

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
        case PNDV_AR_SM_IN_DATA_RTC3:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_DISCONNECT_IND_DONE;

                    /* activity */

                    {
                        PNDV_IFACE_CMD_ENTRY_T tmp_event;

                        // for RTC1/2 AR request input update now

                        // prepare interface command
                        tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                        if (pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr)
                        {
                            tmp_event.add_2 = (PNIO_UINT16)pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr->args.pd.event->u.sv.ar_disconnect->disconnect.reason_code;
                        }
                        else
                        {
                            tmp_event.add_2 = 0;
                        }
                        tmp_event.cmd = PNDV_EV_TO_PERI_AR_DISCONNECT_IND;

                        pndv_in_peri_write_coupl_event( tmp_event );
                    }

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

        // RTC1/2
        case PNDV_AR_SM_W_INPUT_UPDATE:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_FIRST_INPUT_UPDATE_DONE:
                {
                    // first update of inputes was done
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_APP_READY_RSP;


                    /* activity */

                    // prepare and send application ready req to cm
                    pndv_ar_do_appl_ready_req(ar_idx);

                    // free serialization resource and check if other ARs have queued ready-for-input events
                    pndv_pevent_serialization_event_done(&pndv_data.ev_queue_ready_for_input_update, (PNIO_UINT8)ar_idx);

                    break;
                }
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_DISCONNECT_IND_W_DONE;

                    /* activity */

                    // wait for input update done

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
        case PNDV_AR_SM_W_APP_READY_RSP:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_APP_READY_RSP:
                {
                    // confirmation to appl_ready req received
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_IN_DATA_IND;

                    /* activity */

                    pndv_ar_tool_prm_end_check_empty_request(ar_idx);
                    //! a prm_end appl_ready sequence is now done, reset prm_end_elem_cnt to free this resource
                    pndv_data.ar[ar_idx].prm_end_elem_cnt = 0;

                    break;
                }
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_DISCONNECT_IND_W_DONE;

                    /* activity */

                    // wait for app_ready response

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
        case PNDV_AR_SM_W_IN_DATA_IND:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_AR_IN_DATA:
                {
                    // confirmation to appl_ready req received
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_IN_DATA_IND_DONE;

                    /* activity */

                    break;
                }
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_DISCONNECT_IND_DONE;

                    /* activity */

                    {
                        PNDV_IFACE_CMD_ENTRY_T tmp_event;

                        // for RTC1/2 AR request input update now

                        // prepare interface command
                        tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                        if (pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr)
                        {
                            tmp_event.add_2 = (PNIO_UINT16)pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr->args.pd.event->u.sv.ar_disconnect->disconnect.reason_code;
                        }
                        else
                        {
                            tmp_event.add_2 = 0;
                        }
                        tmp_event.cmd = PNDV_EV_TO_PERI_AR_DISCONNECT_IND;

                        pndv_in_peri_write_coupl_event( tmp_event );
                    }

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
        case PNDV_AR_SM_W_IN_DATA_IND_DONE:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_AR_IN_DATA_DONE:
                 {
                     // confirmation to appl_ready req received
                     pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_IN_DATA;

                     /* activity */

                     {
                         PNDV_IFACE_CMD_ENTRY_T tmp_event;

                         // prepare interface command
                         tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                         tmp_event.add_2 = 0;
                         tmp_event.cmd   = PNDV_EV_TO_PERI_AR_IN_DATA;

                         pndv_in_peri_write_coupl_event( tmp_event );

                     }

                     break;
                 }
                 case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                 {
                     pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_DISCONNECT_IND_DONE;

                     /* activity */

                     {
                         PNDV_IFACE_CMD_ENTRY_T tmp_event;

                         // for RTC1/2 AR request input update now

                         // prepare interface command
                         tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                         if (pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr)
                         {
                             tmp_event.add_2 = (PNIO_UINT16)pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr->args.pd.event->u.sv.ar_disconnect->disconnect.reason_code;
                         }
                         else
                         {
                             tmp_event.add_2 = 0;
                         }
                         tmp_event.cmd = PNDV_EV_TO_PERI_AR_DISCONNECT_IND;

                         pndv_in_peri_write_coupl_event( tmp_event );
                     }

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
        case PNDV_AR_SM_IN_DATA:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_DISCONNECT_IND_DONE;

                    /* activity */

                    {
                        PNDV_IFACE_CMD_ENTRY_T tmp_event;

                        // for RTC1/2 AR request input update now

                        // prepare interface command
                        tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                        if (pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr)
                        {
                            tmp_event.add_2 = (PNIO_UINT16)pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr->args.pd.event->u.sv.ar_disconnect->disconnect.reason_code;
                        }
                        else
                        {
                            tmp_event.add_2 = 0;
                        }
                        tmp_event.cmd = PNDV_EV_TO_PERI_AR_DISCONNECT_IND;

                        pndv_in_peri_write_coupl_event( tmp_event );
                    }

                    break;
                }
                case PNDV_AR_SM_EVENT_FIRST_INPUT_UPDATE_DONE:
                {
                    // can happen if peri is very fast
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
        // Abort/Offline
        case PNDV_AR_SM_DISCONNECT_IND_W_DONE:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_OWNERSHIP_IND_DONE:
                {
                    // ownership request has been fully processed by peri

                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_DISCONNECT_IND_DONE;

                    /* activity */
                    PNDV_RQB_SET_RESPONSE(pndv_data.ar[ar_idx].own_ind_rqb_ptr, CM_OK);

                    PNDV_RQB_SET_HANDLE(pndv_data.ar[ar_idx].own_ind_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                    PNDV_RQB_SET_OPCODE(pndv_data.ar[ar_idx].own_ind_rqb_ptr, CM_OPC_SV_AR_OWNERSHIP_RSP);

                    PNDV_REQUEST(pndv_data.ar[ar_idx].own_ind_rqb_ptr, LSA_COMP_ID_CM);

                    pndv_data.ar[ar_idx].own_ind_rqb_ptr = 0;
                    pndv_data.ar[ar_idx].own_rsp_count = 0;

                    // now ar data can be cleaned up
                    {
                        PNDV_IFACE_CMD_ENTRY_T tmp_event;

                        // for RTC1/2 AR request input update now

                        // prepare interface command
                        tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                        if (pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr)
                        {
                            tmp_event.add_2 = (PNIO_UINT16)pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr->args.pd.event->u.sv.ar_disconnect->disconnect.reason_code;
                        }
                        else
                        {
                            tmp_event.add_2 = 0;
                        }
                        tmp_event.cmd = PNDV_EV_TO_PERI_AR_DISCONNECT_IND;

                        pndv_in_peri_write_coupl_event( tmp_event );
                    }

                    break;
                }
                case PNDV_AR_SM_EVENT_PRM_END_IND_DONE:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_DISCONNECT_IND_DONE;

                    {
                        PNDV_IFACE_CMD_ENTRY_T tmp_event;

                        // for RTC1/2 AR request input update now

                        // prepare interface command
                        tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                        if (pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr)
                        {
                            tmp_event.add_2 = (PNIO_UINT16)pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr->args.pd.event->u.sv.ar_disconnect->disconnect.reason_code;
                        }
                        else
                        {
                            tmp_event.add_2 = 0;
                        }
                        tmp_event.cmd = PNDV_EV_TO_PERI_AR_DISCONNECT_IND;

                        pndv_in_peri_write_coupl_event( tmp_event );
                    }

                    break;
                }
                case PNDV_AR_SM_EVENT_RIR_ACK_CNF:
                case PNDV_AR_SM_EVENT_FIRST_INPUT_UPDATE_DONE:
                case PNDV_AR_SM_EVENT_APP_READY_RSP:
                {
                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_W_DISCONNECT_IND_DONE;

                    /* activity */

                    if (event == PNDV_AR_SM_EVENT_APP_READY_RSP)
                    {
                        // clean up in case of appl ready rsp
                        pndv_ar_tool_prm_end_check_empty_request(ar_idx);
                        //! a prm_end appl_ready sequence is now done, reset prm_end_elem_cnt to free this resource
                        pndv_data.ar[ar_idx].prm_end_elem_cnt = 0;
                    }
                    else if(event == PNDV_AR_SM_EVENT_FIRST_INPUT_UPDATE_DONE)
                    {
                        // we got a disconnect while ready-for-input-update was processed by peri for this AR. Acknowledge it and check
                        // for ready-for-input-update of other ARs
                        pndv_pevent_serialization_event_done(&pndv_data.ev_queue_ready_for_input_update, (PNIO_UINT8)ar_idx );
                    }

                    {
                        PNDV_IFACE_CMD_ENTRY_T tmp_event;

                        // for RTC1/2 AR request input update now

                        // prepare interface command
                        tmp_event.add_1 = (PNIO_UINT8)ar_idx;
                        if (pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr)
                        {
                            tmp_event.add_2 = (PNIO_UINT16)pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr->args.pd.event->u.sv.ar_disconnect->disconnect.reason_code;
                        }
                        else
                        {
                            tmp_event.add_2 = 0;
                        }
                        tmp_event.cmd = PNDV_EV_TO_PERI_AR_DISCONNECT_IND;

                        pndv_in_peri_write_coupl_event( tmp_event );
                    }

                    break;
                }
                case PNDV_AR_SM_EVENT_PRM_END_IND:
                case PNDV_AR_SM_EVENT_OWNERSHIP_IND:
                case PNDV_AR_SM_EVENT_CONNECT_IND_PERI_ERROR_DONE:
                case PNDV_AR_SM_EVENT_CONNECT_IND_PERI_RESOURCE_ERROR_DONE:
                case PNDV_AR_SM_EVENT_CONNECT_IND_PERI_OK_DONE:
                case PNDV_AR_SM_EVENT_CONNECT_IND_ERROR_DONE:
                case PNDV_AR_SM_EVENT_CONNECT_IND_OK_DONE:
                case PNDV_AR_SM_EVENT_CONNECT_IND:
                case PNDV_AR_SM_EVENT_AR_IN_DATA:
                case PNDV_AR_SM_EVENT_RIR_IND:
                case PNDV_AR_SM_EVENT_DISCONNECT_IND:
                case PNDV_AR_SM_EVENT_DISCONNECT_IND_DONE:
                default:
                {
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);
                    break;
                }
            }
            break;
        }
        case PNDV_AR_SM_W_DISCONNECT_IND_DONE:
        {
            switch (event)
            {
                case PNDV_AR_SM_EVENT_DISCONNECT_IND_DONE:
                {
                    // ar is now fully disconnected

                    pndv_data.ar[ar_idx].sm_state = PNDV_AR_SM_OFFLINE;

                    /* activity */

                    //check for sr issues to be handled
                    if (pndv_data.ar[ar_idx].ar_set_nr != 0)
                    {
                        PNIO_UINT32 sr_set_idx;
                        // this ar is part of a sysred ar_set
                        sr_set_idx = pndv_ar_get_sr_idx_by_ar_set_nr(pndv_data.ar[ar_idx].ar_set_nr);
                        if (sr_set_idx == PNDV_SR_SET_IDX_NOT_USED)
                        {
                            // must not be here
                            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
                        }

                        pndv_ar_sr_sm(sr_set_idx, PNDV_SR_EVENT_REMOVE_DONE, ar_idx);

                    }

                    //respond to cm
                    PNDV_RQB_SET_RESPONSE(pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr, CM_OK);

                    PNDV_RQB_SET_HANDLE(pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);
                    PNDV_RQB_SET_OPCODE(pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr, CM_OPC_SV_AR_DISCONNECT_RSP);

                    PNDV_REQUEST(pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr, LSA_COMP_ID_CM);

                    pndv_data.ar[ar_idx].disconnect_ind_rqb_ptr = 0;

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

        default:
        {
            // impossible state
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, *ar_sm_state_ptr);
            break;
        }
    }
}


/**
 *  @brief state machine for ar-set administration (sysred)
 *
 *  @param[in]  sr_set_idx  Index of the adressed ar-set resource
 *  @param[in]  event       event to send to the state machine
 *  @param[in]  ar_idx      additional parameter used for adding or removing
 *                          ars from a set
 *
 *  long_description
 *
 */
PNIO_VOID pndv_ar_sr_sm(PNIO_UINT32 sr_set_idx, PNDV_SR_EVENT_T event, PNIO_UINT32 ar_idx)
{
    PNDV_SR_STATE_T    *sr_sm_state_ptr;


    if (sr_set_idx >= (PNDV_MAX_AR_SET + 1))
    {
        //! sr_set_idx out of range
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, sr_set_idx);    /*NOTREACHED*/
    }

    sr_sm_state_ptr = &pndv_data.sr[sr_set_idx].sm;

    pndv_in_write_debug_buffer_3__(PNDV_DC_PNDV_SR_SM, event, *sr_sm_state_ptr, sr_set_idx);

    switch (*sr_sm_state_ptr)
    {

        case PNDV_SR_STATE_OFFLINE:
        {
            switch (event)
            {
                case PNDV_SR_EVENT_ADD_TO_ARSET:
                {
                    // first AR of a set comes in
                    // arset is established now

                    *sr_sm_state_ptr = PNDV_SR_STATE_ESTABLISH;

                    /* activity */

                    pndv_data.sr[sr_set_idx].primary_ar_idx = (PNIO_UINT16)ar_idx;


                    break;
                }
                case PNDV_SR_EVENT_REMOVE_DONE:
                case PNDV_SR_EVENT_REMOVE_FROM_ARSET:
                case PNDV_SR_EVENT_RDHT_TIMEOUT:
                case PNDV_SR_EVENT_RDHT_TIMEOUT_DONE:
                case PNDV_SR_EVENT_NONE:
                default:
                {
                    //! impossible event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);

                    break;
                }
            }
            break;
        }
        case PNDV_SR_STATE_ESTABLISH:
        {
            switch (event)
            {
                case PNDV_SR_EVENT_ADD_TO_ARSET:
                {
                    // second ar of a ar set is connecting
                    // everything is ok now

                    *sr_sm_state_ptr = PNDV_SR_STATE_ONLINE;

                    /* activity */

                    break;
                }
                case PNDV_SR_EVENT_REMOVE_FROM_ARSET:
                {
                    // the last ar of this set is going down

                    *sr_sm_state_ptr = PNDV_SR_STATE_W_OFFLINE_DONE;

                    /* activity */

                    break;
                }
                case PNDV_SR_EVENT_RDHT_TIMEOUT:
                {
                    // data hold timer ran out

                    *sr_sm_state_ptr = PNDV_SR_STATE_ABORT;

                    /* activity */
                    break;
                }
                case PNDV_SR_EVENT_RDHT_TIMEOUT_DONE:
                case PNDV_SR_EVENT_REMOVE_DONE:
                case PNDV_SR_EVENT_NONE:
                default:
                {
                    //! impossible event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);

                    break;
                }
            }
            break;
        }
        case PNDV_SR_STATE_ONLINE:
        {
            switch (event)
            {
                case PNDV_SR_EVENT_REMOVE_FROM_ARSET:
                {
                    // first set-ar is disconnecting
                    // ar_set is still present but not in a safe state

                    *sr_sm_state_ptr = PNDV_SR_STATE_W_REMOVE_DONE;

                    /* activity */

                    break;
                }
                case PNDV_SR_EVENT_RDHT_TIMEOUT:
                {
                    // data hold timer ran out

                    *sr_sm_state_ptr = PNDV_SR_STATE_ABORT;

                    /* activity */
                    break;
                }
                case PNDV_SR_EVENT_RDHT_TIMEOUT_DONE:
                case PNDV_SR_EVENT_ADD_TO_ARSET:
                case PNDV_SR_EVENT_REMOVE_DONE:
                case PNDV_SR_EVENT_NONE:
                default:
                {
                    //! impossible event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);

                    break;
                }
            }
            break;
        }
        case PNDV_SR_STATE_W_REMOVE_DONE:
        {
            switch (event)
            {
                case PNDV_SR_EVENT_REMOVE_DONE:
                {
                    // remove is done, fall back to "Established" State

                    *sr_sm_state_ptr = PNDV_SR_STATE_ESTABLISH;

                    /* activity */

                    break;
                }
                case PNDV_SR_EVENT_ADD_TO_ARSET:
                case PNDV_SR_EVENT_REMOVE_FROM_ARSET:
                case PNDV_SR_EVENT_RDHT_TIMEOUT:
                case PNDV_SR_EVENT_RDHT_TIMEOUT_DONE:
                case PNDV_SR_EVENT_NONE:
                default:
                {
                    //! impossible event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);

                    break;
                }
            }
            break;
        }
        case PNDV_SR_STATE_W_OFFLINE_DONE:
        {
            switch (event)
            {
                case PNDV_SR_EVENT_REMOVE_DONE:
                {
                    // remove of the last ar from this set is done

                    *sr_sm_state_ptr = PNDV_SR_STATE_OFFLINE;

                    /* activity */

                    // ar-set no longer existent, purge it
                    pndv_data.sr[sr_set_idx].ar_set_nr = PNDV_SR_SET_NR_NOT_USED;
                    pndv_data.sr[sr_set_idx].primary_ar_idx = PNDV_AR_IDX_NOT_USED;

                    break;
                }
                case PNDV_SR_EVENT_ADD_TO_ARSET:
                case PNDV_SR_EVENT_REMOVE_FROM_ARSET:
                case PNDV_SR_EVENT_RDHT_TIMEOUT:
                case PNDV_SR_EVENT_RDHT_TIMEOUT_DONE:
                case PNDV_SR_EVENT_NONE:
                default:
                {
                    //! impossible event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);

                    break;
                }
            }
            break;
        }
        case PNDV_SR_STATE_ABORT:
        {
            switch (event)
            {
                case PNDV_SR_EVENT_REMOVE_FROM_ARSET:
                case PNDV_SR_EVENT_REMOVE_DONE:
                {
                    // sr in state abort a disconnect does nothing now
                    // even if no ar of the set is existing anymore
                    // the set is first freed with TIMEOUT_DONE
                    break;
                }
                case PNDV_SR_EVENT_RDHT_TIMEOUT_DONE:
                {
                    PNIO_UINT32 tmp_ar_idx;
                    // timeout has been processed the ar-set no longer existst

                    *sr_sm_state_ptr = PNDV_SR_STATE_OFFLINE;

                    /* activity */

                    //remove sr info from ARs that may belong to this set
                    for( tmp_ar_idx = 0; tmp_ar_idx < PNDV_MAX_ARS_RT; tmp_ar_idx++ )
                    {
                        if (pndv_data.sr[sr_set_idx].ar_set_nr  == pndv_data.ar[tmp_ar_idx].ar_set_nr )
                        {
                            pndv_data.ar[tmp_ar_idx].ar_set_nr = 0;
                        }
                    }

                    // ar-set resource is free now
                    pndv_data.sr[sr_set_idx].ar_set_nr = PNDV_SR_SET_NR_NOT_USED;
                    pndv_data.sr[sr_set_idx].primary_ar_idx = PNDV_AR_IDX_NOT_USED;

                    break;
                }
                case PNDV_SR_EVENT_RDHT_TIMEOUT:
                case PNDV_SR_EVENT_NONE:
                case PNDV_SR_EVENT_ADD_TO_ARSET:
                default:
                {
                    //! impossible event in this state
                    pndv_in_fatal_error( PNDV_MODULE, __LINE__, event);

                    break;
                }
            }
            break;
        }
        case PNDV_SR_STATE_NONE:
        default:
        {
            //! impossible state
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, *sr_sm_state_ptr);
            break;
        }
    }

}

/**
 *  @brief set/reset superordinated locked state of a ar
 *  @ingroup pndv_ar_man
 *
 *  @param[in] event  so lock event to be set / reset
 *  @param[in] lock   PNIO_TRUE == set event, PNIO_FALSE == reset event
 *  @param[in] ar_idx index of the ar which should be locked or unlocked
 *  @return    PNDV_OK:  ar will not be aborted, other: ar will be aborted
 *
 *  This function sets or resets a superordinated locked event of an ar.
 *  If the addressed ar gets a transition from "no event" to "any event" or vice versa,
 *  the ar will signaled to be aborted within this function
 *  (AR may still be running upon leaving this function).
 *
 *  An abort will be indicated to the caller by a return statement other then PNDV_OK.
 *
 */
PNIO_UINT32 pndv_ar_set_so_state(PNIO_UINT32 event, PNIO_BOOL lock, PNIO_UINT32 ar_idx )
{
    PNIO_UINT32 ar_idx_local;
    PNIO_UINT32 tmp_ret_val;
    PNIO_UINT32 ret_val;
    PNIO_UINT32 one_ar;
    PNIO_UINT32 i;
    PNIO_UINT32 event_dez_trace;

    static const PNIO_UINT32 bit_test[] = {0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00, 0xFFFF0000};

    ret_val = PNDV_OK;

    if (  (ar_idx >= PNDV_MAX_ARS_RT)
            &&(ar_idx != PNDV_SOL_ALL_ARS)
        )
    {
        /* index out of range */
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ar_idx);
    }

    /* event-bit fuer den Trace-Eintrag in dez-Zahl umrechnen */
    event_dez_trace = (event & bit_test[0]) != 0;
    for (i = 4; i > 0; i--)
    {
        event_dez_trace |= (PNIO_UINT32)((event & bit_test[i]) != 0) << i;
    }

    if (ar_idx == PNDV_SOL_ALL_ARS)
    {
        /* lock event for all ar's */
        one_ar = PNIO_FALSE;
        ar_idx_local = 0;
    }
    else
    {
        /* lock event for a single ar */
        one_ar = PNIO_TRUE;
        ar_idx_local = ar_idx;
    }

    for(/*initialized above*/;ar_idx_local < PNDV_MAX_ARS_RT; ar_idx_local++)
    {
        PNIO_UINT32 old_flags;

        old_flags = pndv_data.ar_so_locked_state[ar_idx_local];
        if (lock)
        {
            /* add one so lock event */
            pndv_data.ar_so_locked_state[ar_idx_local] |= event;

            /* remember ar_uuid if so_event is PNDV_SOL_EVENT_FLAG_ISOM_LOCKED */
            if ( PNDV_SOL_EVENT_FLAG_ISOM_LOCKED == event )
            {
                pndv_data.so_locked_isom_uuid = pndv_data.ar[pndv_data.cfg.akt_isom_ar_idx].ar_uuid;
            }

            if (old_flags)
            {
                /* so_locked already aktive, nothing to do */

                if ( old_flags != pndv_data.ar_so_locked_state[ar_idx_local] )
                {
                    /* Trace entry only when the SO flags are changed */
                    pndv_in_write_debug_buffer_all_add__(PNDV_DC_SO_LOCKED,
                                                        (PNIO_UINT16)event_dez_trace,
                                                        (   ((pndv_data.ar_so_locked_state[ar_idx_local]) << 8) & 0xFFFFFF00)
                                                          | ((((PNIO_UINT32)(lock))                       << 5) & 0x00000020)
                                                          | ((1                                           << 4) & 0x00000010)
                                                          | (ar_idx_local                                       & 0x0000000F));
                }
            }
            else
            {
                /* switch to so_locked state */

                /* last parameter:
                    0xppppppti   p: still active flags 0..15 t_1: incoming / outgoing PNIO_TRUE / PNIO_FALSE t_0: new SO_Locked_State PNIO_TRUE / PNIO_FALSE i: ar_idx  */
                pndv_in_write_debug_buffer_all_add__(PNDV_DC_SO_LOCKED,
                                                    (PNIO_UINT16)event_dez_trace,
                                                    (   ((pndv_data.ar_so_locked_state[ar_idx_local]) << 8) & 0xFFFFFF00)
                                                      | ((((PNIO_UINT32)(lock))                       << 5) & 0x00000020)
                                                      | ((1                                           << 4) & 0x00000010)
                                                      | (ar_idx_local                                       & 0x0000000F));

                if (   (pndv_data.ar[ar_idx_local].sm_state >= PNDV_AR_SM_OFFLINE)
                        && (pndv_data.ar[ar_idx_local].sm_state <=  PNDV_AR_SM_W_OWNERSHIP_IND_DONE)
                    )
                {
                    /* ar is not yet online or is not the peri ar, no need to abort it */
                }
                else
                {
                    /* ar is up, abort is needed */
                    tmp_ret_val = pndv_ar_abort_req( ar_idx_local, (PNIO_UINT32)__LINE__ );

                    if ( PNDV_OK != tmp_ret_val )
                    {
                        ret_val = tmp_ret_val;
                    }
                }
            }
        }
        else /* lock */
        {
            /* remove one lock event */

            pndv_data.ar_so_locked_state[ar_idx_local] &= ~event;

            if (
                        ( PNDV_SOL_NO_EVENT     == pndv_data.ar_so_locked_state[ar_idx_local] )
                    &&( PNDV_SOL_NO_EVENT     != old_flags )
                )
            {
                /* switch off so_locked state */

                /* last parameter:
                    0xppppppti   p: still active flags 0..15 t_1: incoming / outgoing PNIO_TRUE / PNIO_FALSE t_0: new SO_Locked_State PNIO_TRUE / PNIO_FALSE i: ar_idx  */
                pndv_in_write_debug_buffer_all_add__(PNDV_DC_SO_LOCKED,
                                                    (PNIO_UINT16)event_dez_trace,
                                                    (   ((pndv_data.ar_so_locked_state[ar_idx_local]) << 8) & 0xFFFFFF00)
                                                      | ((((PNIO_UINT32)(lock))                       << 5) & 0x00000020)
                                                      | ((PNIO_FALSE                                  << 4) & 0x00000010)
                                                      | (ar_idx_local                                       & 0x0000000F));

                if (   (pndv_data.ar[ar_idx_local].sm_state >= PNDV_AR_SM_OFFLINE)
                        && (pndv_data.ar[ar_idx_local].sm_state <=  PNDV_AR_SM_W_OWNERSHIP_IND_DONE)
                    )
                {
                    /* ar is not yet online or is not the peri ar, no need to abort it */
                }
                else
                {
                    /* ar is up, abort is needed */
                    tmp_ret_val = pndv_ar_abort_req(ar_idx_local, (PNIO_UINT32)__LINE__);

                    if ( PNDV_OK != tmp_ret_val )
                    {
                            ret_val = tmp_ret_val;
                    }
                }

            }
            else
            {
                if (PNDV_SOL_NO_EVENT     == old_flags)
                {
                    ;/* already unlocked ... */
                }
                else
                {
                    /* stil locked ...*/
                    if ( old_flags != pndv_data.ar_so_locked_state[ar_idx_local] )
                    {
                        /* Trace entry only when the SO flags are changed */
                        pndv_in_write_debug_buffer_all_add__(PNDV_DC_SO_LOCKED,
                                                            (PNIO_UINT16)event_dez_trace,
                                                            (   ((pndv_data.ar_so_locked_state[ar_idx_local]) << 8) & 0xFFFFFF00)
                                                              | ((((PNIO_UINT32)(lock))                       << 5) & 0x00000020)
                                                              | ((1                                           << 4) & 0x00000010)
                                                              | (ar_idx_local                                       & 0x0000000F));
                    }
                }
            }
        }

        if (PNIO_TRUE == one_ar)
        {
            /* only one loop, then break */
            break;
        }
    }

    return ( ret_val );
}

/**
 * @brief get number of connected ARs (RT and device access ARs)
 *
 * @return number of ARs that are currently not offline
 */
PNIO_UINT8 pndv_ar_get_num_connected_ars()
{
    PNIO_UINT8 tmp_ar_idx;
    PNIO_UINT8 num_connected_ars = 0;

    for( tmp_ar_idx = 0; tmp_ar_idx < PNDV_CM_AR_NO; tmp_ar_idx++ )
    {
        if(PNDV_AR_SM_OFFLINE  != pndv_data.ar[tmp_ar_idx].sm_state)
        {
            num_connected_ars++;
        }
    }
    return num_connected_ars;
}

/**
 *  @brief Get ar index (by ar number)
 *  @ingroup pndv_ar_man
 *
 *  @param[in] ar_nr number of the ar that should be searched for
 *  @return    Index of the searched ar or PNDV_AR_IDX_NOT_USED if the ar was not found
 *
 *  Internal ar management is searched for the ar matching the given ar number (ar_nr),
 *  the ar number is set while processing the connect indication and reset by
 *  processing the disconnect indication of an ar.
 *  The ar number is assgined by the cm.
 *  The ar index is assgined by pndv intself and addresses the ar within internal structures.
 *
 */
PNIO_UINT32 pndv_ar_get_ar_idx_by_ar_nr(PNIO_UINT32 ar_nr)
{
    PNIO_UINT32 idx;

    for( idx = 0; idx <= PNDV_CM_AR_NO; idx++ )    /* <= so that too much AR can be terminated cleanly */
    {
        if( ar_nr == pndv_data.ar[idx].ar_nr )
        {
            break;
        }
    }

    if( PNDV_CM_AR_NO < idx )
    {
        idx = PNDV_AR_IDX_NOT_USED;
    }

    return( idx );
}

/**
 *  @brief Get ar index (by ar session key)
 *  @ingroup pndv_ar_man
 *
 *  @param[in] session_key session key of the ar that should be searched for
 *  @return    Index of the searched ar or PNDV_AR_IDX_NOT_USED if the ar was not found
 *
 *  Internal ar management is searched for the ar matching the given ar session key (session_key),
 *  the ar session key is set while processing the connect indication and reset by
 *  processing the disconnect indication of an ar.
 *  The ar session key is assgined by the cm.
 *  The ar index is assgined by pndv intself and addresses the ar within internal structures.
 *
 */PNIO_UINT32 pndv_ar_get_ar_idx_by_session_key(PNIO_UINT16 session_key)
{

    PNIO_UINT32 idx;

    for( idx = 0; idx <= PNDV_CM_AR_NO; idx++ )    /* <= so that too much AR can be terminated cleanly */
    {
        if( session_key == pndv_data.ar[idx].session_key )
        {

            break;
        }
    }

    if( PNDV_CM_AR_NO < idx )
    {
        idx = PNDV_AR_IDX_NOT_USED;
    }

    return( idx );
}
/**
 *  @brief Get sr index (by ar_set_nr)
 *  @ingroup pndv_ar_man
 *
 *  @param[in]  sr_set_nr sr-set number of the ar that should be searched for
 *  @return     Index of the searched sr-set or PNDV_SR_IDX_NOT_USED if the sr-set was not found
 *
 *  Internal sr-set management is searched for the ar-set matching the given ar-set number,
 *  the sr-set number is set while processing the connect indication of system redundancy ars and reset by
 *  processing the disconnect indication of that ar.
 *  The ar-set number is assgined by the cm and part of the session key.
 *  The sr-set index is assgined by pndv intself and addresses the sr-set within internal structures.
 *
 */
PNIO_UINT32 pndv_ar_get_sr_idx_by_ar_set_nr(PNIO_UINT16 sr_set_nr)
{
    PNIO_UINT32 idx;

    for( idx = 0; idx < (PNDV_MAX_AR_SET + 1 ); idx++ )
    {
        if( sr_set_nr == pndv_data.sr[idx].ar_set_nr )
        {
            break;
        }
    }

    if( PNDV_MAX_AR_SET < idx )
    {
        idx = PNDV_SR_SET_IDX_NOT_USED;
    }

    return( idx );
}
//@}

/*****************************************************************************/
/*  end of file.                                                             */
/*****************************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
