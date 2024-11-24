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
/*  F i l e               &F: pndv_ds.c                                 :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Profinet device (cm-user) DS Record-Handling                             */
/*                                                                           */
/*****************************************************************************/



/*****************************************************************************/
/* contents:


    - pndv_in_write_record
    - pndv_in_read_record
    - pndv_in_get_para

    - pndv_in_dfc_rd_io_record_fill
    - pndv_in_dfc_rd_io_record_done
    - pndv_in_ds_build_rd_input_data
    - pndv_in_ds_build_rd_output_data
    - pndv_in_ds_rd_input_data_init
    - pndv_in_ds_rd_output_data_init

    - pndv_in_get_slot_nr
    - pndv_dfc_cbf_cm
    - pndv_dfc_cbf_pndv_ds
    - pndv_set_rqb_err
    - pndv_read_write_record


*/
/*****************************************************************************/
/* 2do:


*/
/*****************************************************************************/
/* include hierarchy */

#include "pndv_inc.h"

#define PNDV_MODULE PNDV_ERR_MODULE_DS


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

/**
 *  @brief returns slot number depending on the index
 *
 *  @param[in]  record_ptr pointer to record structure
 *  @return     slot number
 *
 *  Corresponding to the iec 61158 part 6 table 404 this function checks the
 *  records index range and returns either the slot number or a default value
 *
 */
PNIO_UINT16 pndv_ds_get_slot_nr(CM_UPPER_SV_RECORD_PTR_TYPE record_ptr)
{
    PNIO_UINT16 tmp_slot_nr;

    /* check index range */

    if (record_ptr->record_index < 0xE000 ) /* see iec 61158 part 6 table 404 */
    {
        /* slot must be evaluated */
        tmp_slot_nr = record_ptr->slot_nr;
    }
    else
    {
        /* map to default slot */
        tmp_slot_nr = PNDV_IM_SLOT_NO;
    }

    return tmp_slot_nr;
}

/**
 *  @brief returns slot number depending on the index
 *
 *  @param[in]  record_ptr pointer to record structure
 *  @return     subslot number
 *
 *  Corresponding to the iec 61158 part 6 table 403 this function checks the
 *  records index range and returns either the subslot number or a default value
 *
 */
PNIO_UINT16 pndv_ds_get_subslot_nr(CM_UPPER_SV_RECORD_PTR_TYPE record_ptr)
{
    PNIO_UINT16 tmp_subslot_nr;

    /* check index range */

    if (record_ptr->record_index < 0xC000 ) /* see iec 61158 part 6 table 403 */
    {
        /* subslot must be evaluated */
        tmp_subslot_nr = record_ptr->subslot_nr;
    }
    else
    {
        /* map to default slot */
        tmp_subslot_nr = PNDV_IM_DEFAULT_SUBSLOT_NR;
    }

    return tmp_subslot_nr;
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

/*****************************************************************************/

PNIO_VOID pndv_in_cm_read_write_ind(CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNIO_UINT32                    ar_idx, sr_set_idx, host_ip;
    CM_UPPER_SV_RECORD_PTR_TYPE    record_ptr;

    record_ptr = &rqb_ptr->args.pd.event->u.sv.ar_rec;

    ar_idx = pndv_ar_get_ar_idx_by_session_key(rqb_ptr->args.pd.event->session_key);
    host_ip = (ar_idx < PNDV_CM_AR_NO) ? pndv_data.iface_ptr->ar[ar_idx].host_ip : 0;
    pndv_in_write_debug_buffer_3__(PNDV_DC_DFC_CM_DS_RW_REQ, (PNIO_UINT16)ar_idx, (PNIO_UINT16)(record_ptr->data_length)-CM_RECORD_OFFSET, host_ip);

    if (PNDV_RQB_GET_OPCODE( rqb_ptr) == CM_OPC_SV_AR_READ_IND)
    {

        pndv_in_write_debug_buffer_3__(PNDV_DC_DFC_CM_DS_RD_REQ, rqb_ptr->args.pd.event->u.sv.ar_rec.slot_nr,
                                                                 rqb_ptr->args.pd.event->u.sv.ar_rec.subslot_nr,
                                                                 rqb_ptr->args.pd.event->u.sv.ar_rec.record_index);

        PNDV_DFC_DS_REQ ( rqb_ptr, PNDV_DFC_REQ_ID_CM, 0);
    }
    else if (PNDV_RQB_GET_OPCODE( rqb_ptr) == CM_OPC_SV_AR_WRITE_IND)
    {

        pndv_in_write_debug_buffer_3__(PNDV_DC_DFC_CM_DS_WR_REQ, rqb_ptr->args.pd.event->u.sv.ar_rec.slot_nr,
                                                                 rqb_ptr->args.pd.event->u.sv.ar_rec.subslot_nr,
                                                                 rqb_ptr->args.pd.event->u.sv.ar_rec.record_index);

        /* search ar, check for existing ar_nr and sm_ar == OFFLINE */
        for( ar_idx = 0; ar_idx < PNDV_CM_AR_NO; ar_idx++ )
        {
            if(  ( PNDV_AR_SM_OFFLINE            != pndv_data.ar[ar_idx].sm_state )
              && ( rqb_ptr->args.pd.event->ar_nr == pndv_data.ar[ar_idx].ar_nr )
              )
            {
                break;  /* existing ar_nr */
            }
        }

        if (pndv_data.ar[ar_idx].ar_set_nr != 0)
        {
            // this ar is part of a sysred ar_set
            sr_set_idx = pndv_ar_get_sr_idx_by_ar_set_nr(pndv_data.ar[ar_idx].ar_set_nr);
            if (sr_set_idx == PNDV_SR_SET_IDX_NOT_USED)
            {
                // must not be here
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0); /*NOTREACHED*/
            }

            if (pndv_data.sr[sr_set_idx].primary_ar_idx != ar_idx)
            {
                /* the addressed sysred ar is not primary and therefor can not handle records */
                pndv_set_rqb_err(rqb_ptr, PNDV_EC1_REC_WRITE_BACKUP);
                PNDV_RQB_SET_OPCODE(rqb_ptr, CM_OPC_SV_AR_WRITE_RSP );

                pndv_in_write_debug_buffer_all_add__(PNDV_DC_DFC_DS_DONE, record_ptr->slot_nr, record_ptr->cm_pnio_err);

                /* give rqb back to cm */
                PNDV_RQB_SET_HANDLE(rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);

                PNDV_REQUEST(rqb_ptr, LSA_COMP_ID_CM);
            }
            else
            {
                PNDV_DFC_DS_REQ ( rqb_ptr, PNDV_DFC_REQ_ID_CM, ar_idx);
            }
        }
        else
        {
            PNDV_DFC_DS_REQ ( rqb_ptr, PNDV_DFC_REQ_ID_CM, ar_idx);
        }

    }
    else
    {
        // must not be here
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }
}

/**
 *  @brief callback for dfc requests
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *  @param[in]  con_id Connection id
 *
 *  Callback for returning dfc requests. (PNDV_DFC_REQ_ID_CM)
 *
 */
PNIO_VOID pndv_dfc_cbf_cm( CM_UPPER_RQB_PTR_TYPE rqb_ptr, PNIO_UINT8 con_id )
{
    CM_UPPER_SV_RECORD_PTR_TYPE    record_ptr;
    PNIO_UINT16                    tmp_slot_nr, tmp_subslot_nr;
    PNIO_UINT32                    ar_idx,
                                   subslot_error;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    record_ptr = &rqb_ptr->args.pd.event->u.sv.ar_rec;

    tmp_slot_nr     = pndv_ds_get_slot_nr( record_ptr );
    tmp_subslot_nr  = pndv_ds_get_subslot_nr( record_ptr );

    /* con_id == ar_idx: as this is dfc specific the ar_idx is now determined
     * by checking for the ar_nr inside of the record
     */

    ar_idx = pndv_ar_get_ar_idx_by_ar_nr(rqb_ptr->args.pd.event->ar_nr);

    switch ( PNDV_RQB_GET_OPCODE(rqb_ptr))
    {
        case CM_OPC_SV_AR_READ_IND:
        {
            /* give rqb unseen to cm */
            PNDV_RQB_SET_OPCODE( rqb_ptr, CM_OPC_SV_AR_READ_RSP );

            break;
        }
        case CM_OPC_SV_AR_WRITE_IND:
        {
            pndv_get_submod_resource_ptr(&local_sub_res, tmp_slot_nr, tmp_subslot_nr, &subslot_error);
            if(subslot_error)
            {
                // slot/subslot not possible, must be an implementation error
                pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
            }

            if ( PNDV_CM_AR_NO > ar_idx )
            {
                if (local_sub_res == 0)
                {
                    pndv_set_rqb_err( rqb_ptr, PNDV_EC1_REC_SLOT_INVALID);
                }
                else
                {
                    if (   ( CM_PNIO_ERR_NONE != record_ptr->cm_pnio_err )
                            /* ds-wr-err */
                        && (PNDV_CFG_SUBMODULE_RES_STATE_W_PRM_END == local_sub_res->res_state)
                            /* Run-up para */

                        && ( PNDV_IM_SLOT_NO != tmp_slot_nr )
                            /* on a peripheral module */

                        && (PNIO_TRUE == pndv_data.iface_ptr->ds_rw.suppress_error_if_para_ds)
                            /* PNPB wants me to suppress the error */
                       )
                    {
                        /* do as if there is no error */
                        record_ptr->cm_pnio_err = CM_PNIO_ERR_NONE;
                    }


                    if ( (CM_PNIO_ERR_NONE != record_ptr->cm_pnio_err) )
                    {
                         if (  (PNDV_CFG_SUBMODULE_RES_STATE_W_OWN <= local_sub_res->res_state)
                             &&(ar_idx == local_sub_res->own_ar_idx) // only records from the owner lead to a para error
                            )
                         {
                             local_sub_res->para_error = PNIO_TRUE;
                         }
                    }
                    else
                    {
                         if (  (PNDV_CFG_SUBMODULE_RES_STATE_W_OWN <= local_sub_res->res_state)
                             &&(ar_idx == local_sub_res->own_ar_idx)
                            )
                         {
                             /* Do noting to prevent overwriting a para error if a
                              * submodule has more than one para record.
                              * para_error is reset on owner ar disconnect.
                              */
                             // local_sub_res->para_error = PNIO_FALSE;

                             /* detecting isochronouse mode */
                        #ifdef PNDV_ISOM_DS_NR_NORM
                             if (    ( PNDV_ISOM_DS_NR_NORM  == record_ptr->record_index )
                                  && (   ( PNDV_IM_SLOT_NO   == record_ptr->slot_nr )
                                       ||( 0xFFFF            == record_ptr->slot_nr )
                                     )
                                 )
                             {
                                 pndv_data.cfg.akt_isom_ar_idx = ar_idx;
                                 pndv_data.cfg.mode_isom       = PNIO_TRUE;

                                #ifdef PNDV_CFG_ISOM_NO_SHARED
                                {
                                    PNIO_UINT32 tmp_ar_idx;

                                    /* set all other ars to SOL, this will kick out all established ars */

                                    for ( tmp_ar_idx = 0; tmp_ar_idx < PNDV_MAX_ARS_RT ; tmp_ar_idx++ )
                                    {
                                        if ( pndv_data.cfg.akt_isom_ar_idx != tmp_ar_idx )
                                        {
                                            pndv_ar_set_so_state( PNDV_SOL_EVENT_FLAG_LOCKED_BY_ISOM, PNIO_TRUE, tmp_ar_idx );
                                        }
                                    }
                                }
                                #endif
                             }
                        #endif
                        }
                    }
                }
            }
            /* else: ar existiert nicht mehr -> rqb geht unveraendert zurueck */

            PNDV_RQB_SET_OPCODE( rqb_ptr, CM_OPC_SV_AR_WRITE_RSP );
            break;
        }
        default:
        {
            /* invalid OPCODE */
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE( rqb_ptr));
            break;
        }
    }

    record_ptr->ret_val_1 =
    record_ptr->ret_val_2 = 0;

    pndv_in_write_debug_buffer_all_add__(PNDV_DC_DFC_DS_DONE, record_ptr->slot_nr, record_ptr->cm_pnio_err);

    /* everything ok, give rqb back to cm */
    PNDV_RQB_SET_HANDLE(rqb_ptr, pndv_data.cm_handle[PNDV_INDEX_PATH_IOD_CM_ACP]);

    PNDV_REQUEST(rqb_ptr, LSA_COMP_ID_CM);

}


/**
 *  @brief set error into rqb_ptr
 *
 *  @param[in]  rqb_ptr Pointer to request block
 *  @param[in]  err_code Error code to be put into rqb
 *
 *  Error code is beeing formated an put into rqb.
 *
 */
PNIO_VOID pndv_set_rqb_err(CM_UPPER_RQB_PTR_TYPE rqb_ptr, PNIO_UINT8 err_code)
{
    CM_UPPER_SV_RECORD_PTR_TYPE record_ptr;

    record_ptr = &rqb_ptr->args.pd.event->u.sv.ar_rec;

    if (CM_PNIO_ERR_NONE != err_code)
    {
        switch (PNDV_RQB_GET_OPCODE(rqb_ptr))
        {
            case CM_OPC_SV_AR_READ_IND:
            case CM_OPC_SV_AR_READ_RSP:
            {
                record_ptr->data_length = CM_RECORD_OFFSET;
                record_ptr->cm_pnio_err = CM_PNIO_ERR_MAKE( CM_PNIO_ERR_CODE_READ,
                                                            CM_PNIO_ERR_DECODE_PNIORW,
                                                            err_code,
                                                            0);
                break;
            }
            case CM_OPC_SV_AR_WRITE_IND:
            case CM_OPC_SV_AR_WRITE_RSP:
            {
                record_ptr->data_length = CM_RECORD_OFFSET;
                record_ptr->cm_pnio_err = CM_PNIO_ERR_MAKE( CM_PNIO_ERR_CODE_WRITE,
                                                            CM_PNIO_ERR_DECODE_PNIORW,
                                                            err_code,
                                                            0);
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else
    {
        record_ptr->cm_pnio_err = CM_PNIO_ERR_NONE;
    }

}


/******************************************************************************
 *   Read IO and 0xE050 handling
 */

#ifdef PNDV_CFG_ENABLE_RECORD_RESPONDER

/*****************************************************************************/

PNIO_VOID pndv_in_ds_rd_input_data_init( PNIO_VOID )
{
    /* from 65C_3xx_NP_PROFINET_IO_v20_i282.doc

       Lengths are in parentheses

       RecordInputDataObjectElement(12 + netto_src_data_len):
           BlockHeader(6), LengthIOCS(1), IOCS(1), LengthIOPS(1), IOPS(1), LengthData(2), Data(netto_src_data_len)

               BlockHeader:
                    BlockType(2), BlockLength(2), BlockVersionHigh(1), BlockVersionLow(1)
    */

    PNIO_UINT32 ar_idx;

    for (ar_idx = 0; ar_idx < PNDV_CM_AR_NO; ar_idx++)
    {
        /* BlockType(2) */
        pndv_data.rd_io.rd_input_buffer[PNDV_RD_REC_POS_BLOCK_TYPE_H]      = (PNIO_UINT8)(PNDV_IO_RECORD_BLK_TYPE_INPUT >> 8);
        pndv_data.rd_io.rd_input_buffer[PNDV_RD_REC_POS_BLOCK_TYPE_L]      = (PNIO_UINT8)(PNDV_IO_RECORD_BLK_TYPE_INPUT);

        /* BlockLength(2) -> fill out when editing a record */

        /* BlockVersionHigh(1) */
        pndv_data.rd_io.rd_input_buffer[PNDV_RD_REC_POS_BLOCK_VERS_H]      = (PNIO_UINT8)PNDV_IO_RECORD_VERSION_HIGH;

        /* BlockVersionLow(1) */
        pndv_data.rd_io.rd_input_buffer[PNDV_RD_REC_POS_BLOCK_VERS_L]      = (PNIO_UINT8)PNDV_IO_RECORD_VERSION_LOW;

        /* LengthIOCS(1) */
        pndv_data.rd_io.rd_input_buffer[PNDV_RD_INP_REC_POS_LENGTH_IOCS]   = 1;

        /* IOCS(1)  -> fill out when editing a record */

        /* LengthIOPS(1) */
        pndv_data.rd_io.rd_input_buffer[PNDV_RD_INP_REC_POS_LENGTH_IOPS]   = 1;

        /* IOCS(1)  -> fill out when editing a record */

        /* LengthData(2) -> fill out when editing a record */
    }
}


/*****************************************************************************/


PNIO_VOID pndv_in_ds_rd_output_data_init( PNIO_VOID )
{
    /* from 65C_3xx_NP_PROFINET_IO_v20_i282.doc

       Lengths are in parentheses

       RecordOutputDataObjectElement
           BlockHeader(6), SubstituteActiveFlag(2), LengthIOCS(1), LengthIOPS(1),
           LengthData(2), DataItem(netto_src_data_len+2), SubstituteValue

                BlockHeader(6):
                    BlockType(2), BlockLength(2), BlockVersionHigh(1), BlockVersionLow(1)

                SubstituteActiveFlag:   0 - OPERATION
                                        1 - SUBSTITUTE

                DataItem:
                    IOCS(1), DataObjectElement(netto_src_data_len + 1)

                        DataObjectElement   Data(netto_src_data_len), IOPS(1)

                SubstituteValue:
                    BlockHeader(6), SubstitutionMode(2), DataItem(netto_src_data_len+2)

                because no substitute values are available
                -> SubstitutionMode = 0
                   DataItem.DataObjectElement.IOPS = BAD
                   DataItem.IOCS                   = RecordOutputDataObjectElement.DataItem.IOCS
    */

    PNIO_UINT32 ar_idx;

    for (ar_idx = 0; ar_idx < PNDV_CM_AR_NO; ar_idx++)
    {
        /* BlockType(2) */
        pndv_data.rd_io.rd_output_buffer[PNDV_RD_REC_POS_BLOCK_TYPE_H]          = (PNIO_UINT8)(PNDV_IO_RECORD_BLK_TYPE_OUTPUT >> 8);
        pndv_data.rd_io.rd_output_buffer[PNDV_RD_REC_POS_BLOCK_TYPE_L]          = (PNIO_UINT8)(PNDV_IO_RECORD_BLK_TYPE_OUTPUT);

        /* BlockLength(2) -> fill out when editing a record */

        /* BlockVersionHigh(1) */
        pndv_data.rd_io.rd_output_buffer[PNDV_RD_REC_POS_BLOCK_VERS_H]          = (PNIO_UINT8)PNDV_IO_RECORD_VERSION_HIGH;

        /* BlockVersionLow(1) */
        pndv_data.rd_io.rd_output_buffer[PNDV_RD_REC_POS_BLOCK_VERS_L]          = (PNIO_UINT8)PNDV_IO_RECORD_VERSION_LOW;

    /* SubstituteActiveFlag(2) -> Fill in the low part when editing the record */
        pndv_data.rd_io.rd_output_buffer[PNDV_RD_OUTP_REC_POS_SUBST_ACT_FLAG_H] = 0;

    /* LengthIOCS(1) */
        pndv_data.rd_io.rd_output_buffer[PNDV_RD_OUTP_REC_POS_LENGTH_IOCS]      = 1;

    /* LengthIOPS(1) */
        pndv_data.rd_io.rd_output_buffer[PNDV_RD_OUTP_REC_POS_LENGTH_IOPS]      = 1;

    /* LengthData(2) -> fill out when editing a record */
    }
}

/**
 *  @brief dfc responder function to get ds requests
 *
 *  @param[in]  req_ptr Pointer to request block containing a READ or WRITE Request
 *
 *  Handle read write requests from dfc.
 *
 */
PNIO_VOID pndv_read_write_record (PNIO_VOID *req_ptr)
{
    /* is called by the DFC - no parallelism needs to be considered */

    CM_UPPER_RQB_PTR_TYPE rqb_ptr;

    rqb_ptr = (CM_UPPER_RQB_PTR_TYPE)req_ptr;

    switch ( PNDV_RQB_GET_OPCODE( rqb_ptr) )
    {
        case CM_OPC_SV_AR_READ_IND:
        {
            pndv_in_read_record( rqb_ptr);

            break;
        }

        case CM_OPC_SV_AR_WRITE_IND:
        {
            pndv_in_write_record( rqb_ptr);

            break;
        }

        default:
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, PNDV_RQB_GET_OPCODE( rqb_ptr));

            break;
        }
    }
}


PNIO_VOID pndv_in_write_record (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    CM_UPPER_SV_RECORD_PTR_TYPE         record; /* see definition */

    PNIO_UINT16 rec_index;
    PNIO_UINT8  rec_ret_val;


    record = &rqb_ptr->args.pd.event->u.sv.ar_rec;

    rec_index   = record->record_index;

    rec_ret_val = CM_PNIO_ERR_NONE;

    pndv_in_write_debug_buffer_ds__(PNDV_DC_DS_WRITE,rec_index);

    /*
    subslot 0: Deputy module (AR bound)
    slot 0 subslot 0xFFFF: Device representative (can also be used without AR)

    slot 0xFFFF is used for DS for the AR

    */

    switch (rec_index)
    {
        case PNDV_PARA_RECORD_INDEX_FIX:
        case PNDV_RECORD_ARFSU_DATA_ADJUST: /* ARFSUDataAdjust for one AR, to be used later */
        {
            break;
        }
        default:
        {
            rec_ret_val = PNDV_EC1_REC_INDEX_INVALID;
            break;
        }
    }

    if ( CM_PNIO_ERR_NONE != rec_ret_val )
    {
        /* not OK */
        record->data_length = CM_RECORD_OFFSET;

        pndv_in_write_debug_buffer_ds__(PNDV_DC_DS_WRITE_DONE,0);
    }
    else
    {
        /* OK */

        pndv_in_write_debug_buffer_ds__(PNDV_DC_DS_WRITE_DONE,1);
    }

    PNDV_DFC_RQB_DONE( rqb_ptr, rec_ret_val);

}


/*****************************************************************************/


PNIO_VOID pndv_in_read_record (CM_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    CM_UPPER_SV_RECORD_PTR_TYPE record;
    PNIO_UINT16                 slot_nr;
    PNIO_UINT16                 subslot_nr;
    PNIO_UINT32                 ar_idx;
    PNIO_UINT32                 subslot_error;
    PNIO_UINT16                 entity_nr;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    record = &rqb_ptr->args.pd.event->u.sv.ar_rec;
    ar_idx = PNDV_AR_IDX_NOT_USED;

    slot_nr     = pndv_ds_get_slot_nr( record );
    subslot_nr  = pndv_ds_get_subslot_nr( record );

    pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
    if(subslot_error)
    {
        // slot/subslot not possible, must be an implementation error
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }
    /* regular or implicit ar, check for existing pndv_data.cfg.soll_modul[slot_nr].ar_idx to get io data */
    if(local_sub_res != 0)
    {
        //! get ar_idx from real slot, own_ar_idx is set when submodule gets an owner
        ar_idx    = local_sub_res->own_ar_idx;
    }



    /*
    subslot 0: Deputy module (AR bound)
    slot 0 subslot 0xFFFF: Devicestellvertreter (kann auch ohne AR benutzt werden)

    slot 0xFFFF is used for DS for the AR

    */

    if (   ( PNDV_CM_AR_NO        > ar_idx )                          /* submodule belongs to an ar */
        || ( 0xFFFF              == slot_nr )                                       /* slot 0xFFFF is used for DS for the AR ??? */
       )
    {
        if (   (PNDV_RECORD_INDEX_RD_INP  == record->record_index )
            || (PNDV_RECORD_INDEX_RD_OUTP == record->record_index )
           )
        {
            if ( 0xFFFF != slot_nr )
            {
                /* DS-req is valid */

                /* it can only be 
                   read from an input module (supposed-cfg) input data and 
                   from an output module (target-cfg) 
                   output data in Data-ex */
                /* ar_idx must be valid here, so its save to call PNDV_GET_ENTITY_NR */
                entity_nr = PNDV_GET_ENTITY_NR(record->api, slot_nr, subslot_nr, ar_idx);

                if (  (    (PNDV_RECORD_INDEX_RD_INP == record->record_index )
                        && (0 != pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.descr[0].data_length) //! input length - answer only if inputs present (RQ 1377698)
                      )
                       /* rd_output only works if there is real data -> otherwise empty record but ok */
                    ||(   (PNDV_RECORD_INDEX_RD_OUTP == record->record_index )
                       && (0 != pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.descr[1].data_length) //! output length
                      )

                   )
                {
                    CM_UPPER_SV_RECORD_PTR_TYPE peri_record_ptr = & pndv_data.rd_in_out.ds_struc_ptr->event_type.u.sv.ar_rec;

                    /* Get data from the peri modules including IOxS */
                    PNDV_RQB_SET_OPCODE(&pndv_data.rd_in_out.ds_struc_ptr->rqb, CM_OPC_SV_AR_READ_IND);
                    peri_record_ptr->cm_pnio_err  = 0;
                    peri_record_ptr->record_index = (PNDV_RECORD_INDEX_RD_INP == record->record_index)?
                                                     PNDV_PERI_RECORD_INDEX_RD_INP:
                                                     PNDV_PERI_RECORD_INDEX_RD_OUTP;
                    peri_record_ptr->sequence_nr  = 0;
                    peri_record_ptr->slot_nr      = slot_nr;
                    peri_record_ptr->subslot_nr   = subslot_nr;

                    /* The data is always read from the periphery with the target length according to cfg  
                       Increased peri_record data_length to insert IOXS for provider and consumer status bytes
                    */
                    if ( PNDV_RECORD_INDEX_RD_INP == record->record_index )
                    {
                        peri_record_ptr->data_length = pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.descr[0].data_length + PNDV_IOXS_OFFS + CM_RECORD_OFFSET;
                    }
                    else
                    {
                        peri_record_ptr->data_length = pndv_data.iface_ptr->set_cfg[ar_idx][entity_nr].elem.own_info.descr[1].data_length + PNDV_IOXS_OFFS + CM_RECORD_OFFSET;
                    }


                    if ( ( PNDV_AR_SM_IN_DATA == pndv_data.ar[ar_idx].sm_state)
                        ||( PNDV_AR_SM_IN_DATA_RTC3 == pndv_data.ar[ar_idx].sm_state)
                       )
                    {
                        /* User data exchange in progress -> Get subslot data from the higher-level SW layer */

                        /* is called by the DFC - no parallelism has to be considered 
                           there is only one record at a time */

                        pndv_data.rd_io.rd_io_req.state    = PNDV_RD_IO_RECORD_STATE_COPY_REQ;
                        pndv_data.rd_io.rd_io_req.rqb_ptr  = rqb_ptr;

                        PNDV_DFC_DS_REQ(&pndv_data.rd_in_out.ds_struc_ptr->rqb, PNDV_DFC_REQ_ID_PNDV_DS, PNDV_DFC_CON_ID_RD_INP_OUTP_BASIS + ar_idx );

                    }
                    else
                    {
                        /* User data exchange is not yet running */
                        /* -> Enter zero data + IOXS bad directly in our RQB / return the correct structure (certification test) */

                        PNDV_MEMSET(&peri_record_ptr->data[CM_RECORD_OFFSET], 0, peri_record_ptr->data_length - (CM_RECORD_OFFSET + PNDV_IOXS_OFFS));
                        peri_record_ptr->data[peri_record_ptr->data_length - 2] = PNDV_IOXS_BAD_LOC_DEVICE;  /* rdOutput: OPS ; rdInput: IPS */
                        peri_record_ptr->data[peri_record_ptr->data_length - 1] = PNDV_IOXS_BAD_LOC_DEVICE;  /* rdOutput: OCS ; rdInput: ICS*/

                        pndv_in_dfc_rd_io_record_fill(&peri_record_ptr->data[CM_RECORD_OFFSET],
                                                       peri_record_ptr->data_length - (CM_RECORD_OFFSET + PNDV_IOXS_OFFS) ,
                                                       rqb_ptr,
                                                      (&peri_record_ptr->data[CM_RECORD_OFFSET]) + (peri_record_ptr->data_length - (CM_RECORD_OFFSET + PNDV_IOXS_OFFS))); /* Zeiger auf IOxS */

                        pndv_in_write_debug_buffer_all_add__(PNDV_DC_DS_READ_DONE, slot_nr, CM_PNIO_ERR_NONE);

                        PNDV_DFC_RQB_DONE( rqb_ptr, CM_PNIO_ERR_NONE);

                    }
                }
                else
                {
                    /* rd_input from outp-slot or rd_output from input_slot according to LSA_CM_Detailspec 3.12I40.5*/

                    record->data_length = CM_RECORD_OFFSET;

                    pndv_in_write_debug_buffer_all_add__(PNDV_DC_DS_READ_DONE, slot_nr, PNDV_EC1_REC_INDEX_INVALID);

                    PNDV_DFC_RQB_DONE( rqb_ptr, PNDV_EC1_REC_INDEX_INVALID);
                }
            }
            else
            {
                /* slot == 0xFFFF - AR-record are not supported */

                record->data_length = CM_RECORD_OFFSET;

                pndv_in_write_debug_buffer_all_add__(PNDV_DC_DS_READ_DONE, slot_nr, PNDV_EC1_REC_NOT_SUPPORTED);

                PNDV_DFC_RQB_DONE( rqb_ptr, PNDV_EC1_REC_NOT_SUPPORTED);

            }
        }
        else
        {
            record->data_length = CM_RECORD_OFFSET;

            pndv_in_write_debug_buffer_all_add__(PNDV_DC_DS_READ_DONE, slot_nr, PNDV_EC1_REC_INDEX_INVALID);

            PNDV_DFC_RQB_DONE( rqb_ptr, PNDV_EC1_REC_INDEX_INVALID);

        }
    }
    else
    {
        record->data_length = CM_RECORD_OFFSET;

        pndv_in_write_debug_buffer_all_add__(PNDV_DC_DS_READ_DONE, slot_nr, PNDV_EC1_REC_INDEX_INVALID);

        PNDV_DFC_RQB_DONE( rqb_ptr, PNDV_EC1_REC_INDEX_INVALID);

    }
}


/**
 *  @brief callback for dfc requests (internal built ds)
 *
 *  @param[in]  ds_ptr pointer to a data set
 *  @param[in]  con_id Connection id
 *
 *  Callback for returning dfc requests of pndv internal
 *  built ds requests (e.g. post or re para ds) (PNDV_DFC_REQ_ID_PNDV_DS)
 *
 */
PNIO_VOID pndv_dfc_cbf_pndv_ds(CM_UPPER_RQB_PTR_TYPE ds_ptr, PNIO_UINT8 con_id)
{
    PNIO_UINT32 ar_idx;

    {
        ar_idx = con_id - PNDV_DFC_CON_ID_RD_INP_OUTP_BASIS;

        if ( PNDV_CM_AR_NO > ar_idx )
        {
            /* valid ar_idx */

            if ( PNDV_RD_IO_RECORD_STATE_COPY_REQ == pndv_data.rd_io.rd_io_req.state )
            {
                if ((CM_RECORD_OFFSET+2) > ds_ptr->args.pd.event->u.sv.ar_rec.data_length )
                {
                    /* less than 2 * ioxs -> empty DS back */

                    pndv_in_dfc_rd_io_record_fill(NIL, 0, pndv_data.rd_io.rd_io_req.rqb_ptr, NIL);

                }
                else
                {
                    pndv_in_dfc_rd_io_record_fill(&ds_ptr->args.pd.event->u.sv.ar_rec.data[CM_RECORD_OFFSET], /* Structure ds: data, iops, iocs */
                                                  ds_ptr->args.pd.event->u.sv.ar_rec.data_length - (CM_RECORD_OFFSET+2) ,
                                                  pndv_data.rd_io.rd_io_req.rqb_ptr,
                                                  (&ds_ptr->args.pd.event->u.sv.ar_rec.data[CM_RECORD_OFFSET]) + (ds_ptr->args.pd.event->u.sv.ar_rec.data_length - (CM_RECORD_OFFSET+2))); /* Pointer to IOxS */
                }

                pndv_in_dfc_rd_io_record_done( pndv_data.rd_io.rd_io_req.rqb_ptr );
            }
            /* rd_io_req is no longer valid -> ignore */
        }
        else
        {
            /* unknown con_id */
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, con_id);
        }
    }
}


/*****************************************************************************/


PNIO_VOID pndv_in_dfc_rd_io_record_fill(PNIO_UINT8*           data_ptr,
                                        PNIO_UINT32           data_len,
                                        CM_UPPER_RQB_PTR_TYPE rqb_ptr,
                                        PNIO_UINT8*           ioxs_ptr )
{
    CM_UPPER_SV_RECORD_PTR_TYPE record; /* see definition */

    PNIO_UINT32 tmp_data_len,
                ar_idx = PNDV_CM_AR_NO;
    PNIO_UINT16 slot_nr,
                subslot_nr,
                real_netto_len = 0;
    PNIO_UINT32 subslot_error;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    PNIO_UINT8* tmp_record_ptr;
    PNIO_UINT32 length_error;


    length_error = PNIO_FALSE;

    record = &rqb_ptr->args.pd.event->u.sv.ar_rec;

    slot_nr     = pndv_ds_get_slot_nr( record );
    subslot_nr  = pndv_ds_get_subslot_nr( record );

    pndv_get_submod_resource_ptr(&local_sub_res, slot_nr, subslot_nr, &subslot_error);
    if(subslot_error)
    {
        // slot/subslot not possible, must be an implementation error
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
    }

    if(local_sub_res != 0)
    {
        //! get ar_idx from real slot, own_ar_idx is set when submodule gets an owner
        ar_idx    = local_sub_res->own_ar_idx;
    }

    /* read-input; read-output */
    /* --------------------------------------------------------- */

    if (   ( CM_OPC_SV_AR_READ_IND == PNDV_RQB_GET_OPCODE( rqb_ptr) )
        && (    (PNDV_RECORD_INDEX_RD_INP  == record->record_index )
             || (PNDV_RECORD_INDEX_RD_OUTP == record->record_index )
           )
       )
    {
        if (   (   ( PNDV_RECORD_INDEX_RD_INP == record->record_index )
                && ( PNDV_MAX_RD_IO_DATA_LEN  >= data_len )
               )
            || (   ( PNDV_RECORD_INDEX_RD_OUTP == record->record_index )
                && ( PNDV_MAX_RD_IO_DATA_LEN  >= data_len )
               )
           )
        {
            if (  !pndv_host_ptr_is_nil__(data_ptr) )
            {

                if( PNDV_AR_IDX_NOT_USED != ar_idx )
                {

                    if ( PNDV_RECORD_INDEX_RD_INP == record->record_index )
                    {
                        tmp_data_len = pndv_in_ds_build_rd_input_data(data_ptr, data_len, slot_nr, subslot_nr, ar_idx, ioxs_ptr);

                        tmp_record_ptr = &pndv_data.rd_io.rd_input_buffer[0];

                        /* Determine the minimum from the required length and record length */

                        if (tmp_data_len > (record->data_length - CM_RECORD_OFFSET))
                        {
                            length_error = PNIO_TRUE;
                        }
                        else
                        {
                            real_netto_len = tmp_data_len;
                        }
                    }
                    else
                    {
                        tmp_data_len = pndv_in_ds_build_rd_output_data(data_ptr, data_len, slot_nr, subslot_nr, ar_idx, ioxs_ptr);

                        tmp_record_ptr = &pndv_data.rd_io.rd_output_buffer[0];

                        /* Determine the minimum from the required length and record length */

                        if (tmp_data_len > (record->data_length - CM_RECORD_OFFSET))
                        {
                            length_error = PNIO_TRUE;
                        }
                        else
                        {
                            real_netto_len = tmp_data_len;
                        }
                    }

                    /* copy according to the required length */

                    if (length_error)
                    {
                        /* the requested data record length is too small */

                        record->data_length = CM_RECORD_OFFSET;

                        record->cm_pnio_err = CM_PNIO_ERR_MAKE( CM_PNIO_ERR_CODE_READ,
                                                                CM_PNIO_ERR_DECODE_PNIORW,
                                                                PNDV_EC1_REC_INVALID_TYPE,
                                                                0);


                    }
                    else if ( real_netto_len )
                    {
                        /* there is room for data */

                        PNDV_COPY_BYTE(record->data + CM_RECORD_OFFSET,
                                       tmp_record_ptr,
                                       real_netto_len);

                        record->data_length = real_netto_len + CM_RECORD_OFFSET;

                        /* Length is now maximally as originally requested */
                    }
                    else
                    {
                        /* Record remains empty */

                        record->data_length = CM_RECORD_OFFSET;
                    }

                }
                else
                {
                    /* no AR, if no data is available the entire DS remains empty */

                    record->data_length = CM_RECORD_OFFSET;
                }

            }
            else
            {
                /* if no data is available, the entire DS remains empty */

                record->data_length = CM_RECORD_OFFSET;
            }

            if (PNIO_FALSE == length_error)
            {
                record->cm_pnio_err =  CM_PNIO_ERR_NONE;
            }
            else
            {
                /* Errors already set */
            }
        }
        else
        {
            /* the tmp. Buffer in which I assemble the record is too small */

            record->data_length = CM_RECORD_OFFSET;

            record->cm_pnio_err = CM_PNIO_ERR_MAKE( CM_PNIO_ERR_CODE_READ,
                                                    CM_PNIO_ERR_DECODE_PNIORW,
                                                    PNDV_EC1_REC_INVALID_TYPE,
                                                    0);
        }
    }
    else
    {
        /* that's none of my business */

        record->data_length = CM_RECORD_OFFSET;

        record->cm_pnio_err = CM_PNIO_ERR_MAKE( CM_PNIO_ERR_CODE_READ,
                                                CM_PNIO_ERR_DECODE_PNIORW,
                                                PNDV_EC1_REC_INDEX_INVALID,
                                                0);
    }

    record->ret_val_1 =
    record->ret_val_2 = 0;

    if( PNDV_CM_AR_NO > ar_idx )
    {
        pndv_data.rd_io.rd_io_req.state     = PNDV_RD_IO_RECORD_STATE_QUIT_REQ;
        pndv_data.rd_io.rd_io_req.inp_slot  =
        pndv_data.rd_io.rd_io_req.outp_slot = 0xFFFF;   /* with 0xFFFF there is no reaction */
    }
    /* else: no AR, fatal? @@ habo ?? */
}


/*****************************************************************************/


PNIO_VOID pndv_in_dfc_rd_io_record_done( CM_UPPER_RQB_PTR_TYPE rqb_ptr )
{
    pndv_data.rd_io.rd_io_req.state = PNDV_RD_IO_RECORD_STATE_NONE;

    pndv_in_write_debug_buffer_all_add__(PNDV_DC_DS_READ_DONE, rqb_ptr->args.pd.event->u.sv.ar_rec.slot_nr,
                                                               rqb_ptr->args.pd.event->u.sv.ar_rec.cm_pnio_err);

    PNDV_DFC_RQB_DONE(rqb_ptr, ((PNIO_UINT8)((rqb_ptr->args.pd.event->u.sv.ar_rec.cm_pnio_err & 0x0000FF00) >> 8)));

}


/*****************************************************************************/

PNIO_UINT32 pndv_in_ds_build_rd_input_data(PNIO_UINT8* src_data_ptr,
                                           PNIO_UINT32 netto_src_data_len,
                                           PNIO_UINT16 slot,
                                           PNIO_UINT16 subslot,
                                           PNIO_UINT32 ar_idx,
                                           PNIO_UINT8* ioxs_ptr )
{
    PNIO_UINT32 BlockLength,
                tmp_record_len;
    PNIO_UINT32 subslot_error;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    tmp_record_len = 0;

    pndv_get_submod_resource_ptr(&local_sub_res, slot, subslot, &subslot_error);
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

    if(netto_src_data_len > PNDV_MAX_RD_IO_DATA_LEN)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, netto_src_data_len);        /* NOTREACHED */
    }


    if ( !pndv_host_ptr_is_nil__(src_data_ptr) )
    {
        /* Data is valid, and this always corresponds to the data according to the target cfg */


        /* from 65C_3xx_NP_PROFINET_IO_v20_i282.doc

           Lengths are in parentheses

           RecordInputDataObjectElement(12 + netto_src_data_len):
               BlockHeader(6), LengthIOCS(1), IOCS(1), LengthIOPS(1), IOPS(1), LengthData(2), Data(netto_src_data_len)

                   BlockHeader:
                        BlockType(2), BlockLength(2), BlockVersionHigh(1), BlockVersionLow(1)
        */

        /* BlockLength(2) */
        BlockLength = PNDV_RD_INP_REC_BLOCK_LEN_FIX + netto_src_data_len;

        pndv_data.rd_io.rd_input_buffer[PNDV_RD_REC_POS_BLOCK_LEN_H] = (PNIO_UINT8)(BlockLength >> 8);
        pndv_data.rd_io.rd_input_buffer[PNDV_RD_REC_POS_BLOCK_LEN_L] = (PNIO_UINT8)(BlockLength);

        if ( pndv_host_ptr_is_nil__(ioxs_ptr) )
        {
            /* no ioxs from the source */

            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }
        else
        {
            /* IOCS(1) */
            pndv_data.rd_io.rd_input_buffer[PNDV_RD_INP_REC_POS_IOCS] = *(ioxs_ptr + PNDV_RD_INP_OUTP_IOCS_OFFS);

            /* IOPS(1) */
            pndv_data.rd_io.rd_input_buffer[PNDV_RD_INP_REC_POS_IOPS] = *(ioxs_ptr + PNDV_RD_INP_OUTP_IOPS_OFFS);

        }

        /* LengthData(2) */
        pndv_data.rd_io.rd_input_buffer[PNDV_RD_INP_REC_POS_LENGTH_DATA_H] = (PNIO_UINT8)(netto_src_data_len >> 8);
        pndv_data.rd_io.rd_input_buffer[PNDV_RD_INP_REC_POS_LENGTH_DATA_L] = (PNIO_UINT8)(netto_src_data_len);


        if ( 0 != netto_src_data_len )
        {
            PNDV_COPY_BYTE(&pndv_data.rd_io.rd_input_buffer[PNDV_RD_INP_REC_POS_DATA],
                           src_data_ptr,
                           netto_src_data_len);
        }

        tmp_record_len = PNDV_RD_INP_REC_POS_DATA + netto_src_data_len;
    }

    return(tmp_record_len);
}


/*****************************************************************************/


PNIO_UINT32 pndv_in_ds_build_rd_output_data(PNIO_UINT8* src_data_ptr,
                                            PNIO_UINT32 netto_src_data_len,
                                            PNIO_UINT16 slot,
                                            PNIO_UINT16 subslot,
                                            PNIO_UINT32 ar_idx,
                                            PNIO_UINT8* ioxs_ptr )
{
    PNIO_UINT32 act_pos_data,
                BlockLength,
                subslot_error;
    PNDV_STRUCT_CFG_SUBMODULE_RES_PTR_T local_sub_res;

    act_pos_data = 0;

    pndv_get_submod_resource_ptr(&local_sub_res, slot, subslot, &subslot_error);
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

    if(netto_src_data_len > PNDV_MAX_RD_IO_DATA_LEN)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, netto_src_data_len);        /* NOTREACHED */
    }


    if (   ( 0 != netto_src_data_len )
        && ( !pndv_host_ptr_is_nil__(src_data_ptr) )
       )
    {
        /* Data is valid, and this always corresponds to the data according to the target cfg */


        /* from 65C_3xx_NP_PROFINET_IO_v20_i282.doc

           Lengths are in parentheses

           RecordOutputDataObjectElement
               BlockHeader(6), SubstituteActiveFlag(2), LengthIOCS(1), LengthIOPS(1),
               LengthData(2), DataItem(netto_src_data_len+2), SubstituteValue

                    BlockHeader(6):
                        BlockType(2), BlockLength(2), BlockVersionHigh(1), BlockVersionLow(1)

                    SubstituteActiveFlag:   0 - OPERATION
                                            1 - SUBSTITUTE

                    DataItem:
                        IOCS(1), DataObjectElement(netto_src_data_len + 1)

                            DataObjectElement:
                               Data(netto_src_data_len), IOPS(1)

                    SubstituteValue:
                        BlockHeader(6), SubstitutionMode(2), DataItem(netto_src_data_len+2)

                    because no substitute values are available (substitution mode is unknown)
                    -> SubstitutionMode = 0 ,
                       CPU-RUN: DataItem.Data = Ausgangsdaten / CPU-STOP : DataItem.Data = 0 (siehe IEC 61158-5 , Kapitel "Read Record Output Data Object Element")
                       DataItem.DataObjectElement.IOPS = GOOD
                       DataItem.IOCS                   = RecordOutputDataObjectElement.DataItem.IOCS
        */

        /* BlockLength(2) = Length including the Substitute Value block
                            Length of all fixed elements + 2 * DataItem */
        BlockLength = PNDV_RD_OUTP_REC_BLOCK_LEN_FIX + 2 * (netto_src_data_len + PNDV_RD_OUTP_REC_DATA_ITEM_LEN_FIX);

        pndv_data.rd_io.rd_output_buffer[PNDV_RD_REC_POS_BLOCK_LEN_H] = (PNIO_UINT8)(BlockLength >> 8);
        pndv_data.rd_io.rd_output_buffer[PNDV_RD_REC_POS_BLOCK_LEN_L] = (PNIO_UINT8)(BlockLength);

        /* SubstituteActiveFlag(2) high part filled in init*/
        if ( pndv_host_ptr_is_nil__(ioxs_ptr) )
        {
            /* no ioxs from the source */
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }
        else
        {
            /* The database is PERI -> take ops from PERI for comparison */
            if ( PNDV_IOXS_BAD == ((*(ioxs_ptr + PNDV_RD_INP_OUTP_IOPS_OFFS)) & PNDV_IOXS_GB_MASK) )
            {
                pndv_data.rd_io.rd_output_buffer[PNDV_RD_OUTP_REC_POS_SUBST_ACT_FLAG_L] = 1;  /* SUBSTITUTE */
            }
            else
            {
                pndv_data.rd_io.rd_output_buffer[PNDV_RD_OUTP_REC_POS_SUBST_ACT_FLAG_L] = 0;  /* OPERATION */
            }
        }

        /* LengthData(2) */
        pndv_data.rd_io.rd_output_buffer[PNDV_RD_OUTP_REC_POS_LENGTH_DATA_H] = (PNIO_UINT8)(netto_src_data_len >> 8);
        pndv_data.rd_io.rd_output_buffer[PNDV_RD_OUTP_REC_POS_LENGTH_DATA_L] = (PNIO_UINT8)(netto_src_data_len);


        act_pos_data = PNDV_RD_OUTP_REC_POS_DATA_ITEM;


        /* ----------------------------------------------------------------- */

        /* DataItem(netto_src_data_len+2) */
        /*     DataItem:
                   IOCS(1), DataObjectElement(netto_src_data_len + 1)

                       DataObjectElement   Data(netto_src_data_len), IOPS(1)
        */

        /* DataItem.IOCS(1) */

        if ( pndv_host_ptr_is_nil__(ioxs_ptr) )
        {
            /* no ioxs from the source */
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }
        else
        {
            pndv_data.rd_io.rd_output_buffer[act_pos_data++] = *(ioxs_ptr + PNDV_RD_INP_OUTP_IOCS_OFFS);
        }


        /* DataItem.DataObjectElement.Data */

        PNDV_COPY_BYTE( &pndv_data.rd_io.rd_output_buffer[act_pos_data],
                       src_data_ptr,
                       netto_src_data_len);

        act_pos_data += netto_src_data_len;

        /* DataItem.DataObjectElement.IOPS */

        if ( pndv_host_ptr_is_nil__(ioxs_ptr) )
        {
            /* no ioxs from the source */
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }
        else
        {
            pndv_data.rd_io.rd_output_buffer[act_pos_data++] = *(ioxs_ptr + PNDV_RD_INP_OUTP_IOPS_OFFS);

        }

        /* ----------------------------------------------------------------- */

        /* SubstituteValue:
              BlockHeader(6), SubstitutionMode(2), DataItem(netto_src_data_len+2)
        */

        /* SubstituteValue.BlockType(2) */
        pndv_data.rd_io.rd_output_buffer[act_pos_data + PNDV_RD_REC_POS_BLOCK_TYPE_H] = (PNIO_UINT8)(PNDV_IO_RECORD_BLK_TYPE_SUBSTITUDE >> 8);
        pndv_data.rd_io.rd_output_buffer[act_pos_data + PNDV_RD_REC_POS_BLOCK_TYPE_L] = (PNIO_UINT8)(PNDV_IO_RECORD_BLK_TYPE_SUBSTITUDE);

        /* SubstituteValue.BlockLength(2) */
        BlockLength = PNDV_RD_SUBST_REC_BLOCK_LEN_FIX + netto_src_data_len + PNDV_RD_OUTP_REC_DATA_ITEM_LEN_FIX;

        pndv_data.rd_io.rd_output_buffer[act_pos_data + PNDV_RD_REC_POS_BLOCK_LEN_H] = (PNIO_UINT8)(BlockLength >> 8);
        pndv_data.rd_io.rd_output_buffer[act_pos_data + PNDV_RD_REC_POS_BLOCK_LEN_L] = (PNIO_UINT8)(BlockLength);

        /* SubstituteValue.BlockVersionHigh(1) */
        pndv_data.rd_io.rd_output_buffer[act_pos_data + PNDV_RD_REC_POS_BLOCK_VERS_H] = (PNIO_UINT8)PNDV_IO_RECORD_VERSION_HIGH;

        /* SubstituteValue.BlockVersionLow(1) */
        pndv_data.rd_io.rd_output_buffer[act_pos_data + PNDV_RD_REC_POS_BLOCK_VERS_L] = (PNIO_UINT8)PNDV_IO_RECORD_VERSION_LOW;

        act_pos_data += PNDV_RD_REC_POS_BLOCK_VERS_L + 1;

        /* SubstituteValue.SubstitutionMode(2) */
        pndv_data.rd_io.rd_output_buffer[act_pos_data++] = 0;
        pndv_data.rd_io.rd_output_buffer[act_pos_data++] = 0;


        /* ----------------------------------------------------------------- */

        /*
            because no substitute values are available
            -> SubstitutionMode = 0 ,
               CPU-RUN: DataItem.Data = Ausgangsdaten / CPU-STOP : DataItem.Data = 0  (siehe IEC 61158-5 , Kapitel "Read Record Output Data Object Element")
               DataItem.DataObjectElement.IOPS = GOOD
               DataItem.IOCS                   = RecordOutputDataObjectElement.DataItem.IOCS
        */

        /* SubstituteValue.DataItem.IOCS(1) */

        if ( pndv_host_ptr_is_nil__(ioxs_ptr) )
        {
            /* no ioxs from the source */
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, 0);
        }
        else
        {
            pndv_data.rd_io.rd_output_buffer[act_pos_data++] = *(ioxs_ptr + PNDV_RD_INP_OUTP_IOCS_OFFS);
        }

        /* SubstituteValue.DataItem.DataObjectElement.Data */
        if ( PNDV_IOXS_BAD == ((*(ioxs_ptr + PNDV_RD_INP_OUTP_IOPS_OFFS)) & PNDV_IOXS_GB_MASK) )
        {
            PNDV_MEMSET( &pndv_data.rd_io.rd_output_buffer[act_pos_data],
                          0,
                          netto_src_data_len);
        }
        else
        {
            PNDV_COPY_BYTE( &pndv_data.rd_io.rd_output_buffer[act_pos_data],
                            src_data_ptr,
                            netto_src_data_len);
        }
        act_pos_data += netto_src_data_len;

        /* SubstituteValue.DataItem.DataObjectElement.IOPS */
        pndv_data.rd_io.rd_output_buffer[act_pos_data++] = PNDV_IOXS_GOOD;
    }

    return(act_pos_data);
}


#endif /* PNDV_CFG_ENABLE_RECORD_RESPONDER */

/*****************************************************************************/

PNIO_UINT32 pndv_ds_check_arfsu_data_adjust(PNIO_UINT8 * record_ptr, PNIO_UINT8 *ret_fsu_enabled, PNIO_UINT8 *ret_fsuuuid_ptr)
{
    PNIO_UINT32  ret_val;
    PNIO_UINT32  offset;
    PNIO_UINT16  block_type;
    PNIO_UINT16  block_length;
    PNIO_UINT16* ptr16;
    PNIO_UINT32* ptr32;

    ret_val = PNDV_OK;
    offset = 0;

    if (record_ptr)
    {
        offset = 0;

        ptr16        = (PNIO_UINT16*) ( record_ptr );
        block_type   = PNDV_NTOHS(*ptr16);
        ptr16        = (PNIO_UINT16*) ( record_ptr + 2 );
        block_length = PNDV_NTOHS(*ptr16);

        if ((block_type == 0x010b) && (block_length == 32 ))
        {
            offset += 8; /* whole header */
            ptr16        = (PNIO_UINT16*) ( record_ptr + offset );
            block_type   = PNDV_NTOHS(*ptr16);
            ptr16        = (PNIO_UINT16*) ( record_ptr + offset + 2 );
            block_length = PNDV_NTOHS(*ptr16);
            if ((block_type == 0x0601) && (block_length == 24 ))
            {
                PNIO_UINT32 fs_parameter_mode;  // FSParameterMode.Mode

                offset           += 8; /* whole header */
                ptr32             = (PNIO_UINT32*) ( record_ptr + offset );
                fs_parameter_mode = PNDV_NTOHL(*ptr32);
                if ((fs_parameter_mode & 3) == 0x1)
                {
                    *ret_fsu_enabled = LSA_TRUE;
                }
                else
                {
                    *ret_fsu_enabled = LSA_FALSE;
                }

                offset += 4;
                PNDV_COPY_BYTE(ret_fsuuuid_ptr, (record_ptr+offset), 16);
            }
            else
            {
                ret_val = PNDV_ERR_PARAMETER;
            }
        }
        else
        {
            ret_val = PNDV_ERR_PARAMETER;
        }
    }
    return ret_val;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**********                                                         **********/
/**********                CM INTERFACE FUNCTIONS                   **********/
/**********                                                         **********/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**********                                                         **********/
/**********                PNES INTERFACE FUNCTIONS                 **********/
/**********                                                         **********/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


/*****************************************************************************/
/*  end of file.                                                             */
/*****************************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
