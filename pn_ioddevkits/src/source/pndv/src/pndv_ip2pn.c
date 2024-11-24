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
/*  F i l e               &F: pndv_ip2pn.c                              :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Implementation of pndv - ip2pn services                                  */
/*                                                                           */
/*****************************************************************************/
/* include hierarchy */

#include "pndv_inc.h"
#include "pnpb_nvdata.h"

#define PNDV_MODULE PNDV_ERR_MODULE_IP2PN

/* Helper Function Declarations */
PNIO_VOID        write_snmp_record_data_element(PNIO_UINT8* element_ptr, PNIO_UINT32 element_length, PNIO_UINT8* data_copy_ptr, 
                                                PNIO_UINT32* copy_pos_ptr, PNDV_IP2PN_SNMP_BLOCK_INDEX_TYPE block_type);
PNIO_UINT16 get_ipsuite_data_from_rema_record(IP2PN_UPPER_MEM_U8_CONST_PTR_TYPE pRecData, PNIO_UINT32 RecordLen, PNDV_IP2PN_IPSUITE_INFO_T* pIpSuite);
PNIO_UINT16 get_snmp_data_from_rema_record(IP2PN_UPPER_MEM_U8_CONST_PTR_TYPE pRecData, PNIO_UINT32 RecordLen, PNDV_IP2PN_SNMP_DATA_TYPE_T* pSnmpData);
PNIO_UINT8  validate_ip_suite(PNIO_UINT8* pIpSuite);

/*****************************************************************************/
/*  IP2PN Startup Functions                                                  */
/*****************************************************************************/

PNIO_VOID pndv_in_ip2pn_open_channel(PNIO_UINT8 channel_idx)
{
    PNIO_INT8 syspath_if_id = -1;

    switch (channel_idx)
    {
        case PNDV_INDEX_PATH_IP2PN_GLO_APP:
        {
            PNDV_ALLOC_RQB(&pndv_data.ip2pn.stp_rqb_ptr, sizeof(IP2PN_RQB_TYPE));
            if (pndv_data.ip2pn.stp_rqb_ptr == LSA_NULL)
            {
                pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
            }
            syspath_if_id = 0;  // 0 = global, 1-16 = PNIO IF
            break;
        }
        case PNDV_INDEX_PATH_IP2PN_IF_CM:
        {
            syspath_if_id = 1;  // 0 = global, 1-16 = PNIO IF
            break;
        }
        default:
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
        }
    }

    IP2PN_UPPER_RQB_PTR_TYPE stp_rqb_ptr = pndv_data.ip2pn.stp_rqb_ptr;

    PNDV_SYSPATH_SET_IF(pndv_data.sys_path_ip2pn[channel_idx], syspath_if_id);
    PNDV_SYSPATH_SET_HD(pndv_data.sys_path_ip2pn[channel_idx], 1);

    stp_rqb_ptr->args.channel.handle                       = IP2PN_INVALID_HANDLE;
    stp_rqb_ptr->args.channel.handle_upper                 = channel_idx;
    stp_rqb_ptr->args.channel.sys_path                     = pndv_data.sys_path_ip2pn[channel_idx];
    stp_rqb_ptr->args.channel.ip2pn_request_upper_done_ptr = pndv_in_ip2pn_to_appl_cbf;

    pndv_in_write_debug_buffer_all__(PNDV_DC_IP2PN_OPEN_CH, (PNIO_UINT16)channel_idx);

    PNDV_RQB_SET_OPCODE(stp_rqb_ptr, IP2PN_OPC_OPEN_CHANNEL);
    PNDV_RQB_SET_HANDLE(stp_rqb_ptr, 0);

    PNDV_OPEN_CHANNEL_LOWER(stp_rqb_ptr, LSA_COMP_ID_IP2PN);
}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_open_channel_done(IP2PN_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    LSA_RESPONSE_TYPE response;
    LSA_HANDLE_TYPE   ch_handle = rqb_ptr->args.channel.handle_upper;

    response = PNDV_RQB_GET_RESPONSE(rqb_ptr);
    if (response != LSA_RSP_OK)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, response);
    }

    if (!((ch_handle == PNDV_INDEX_PATH_IP2PN_GLO_APP) || (ch_handle == PNDV_INDEX_PATH_IP2PN_IF_CM)))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, (LSA_UINT32)rqb_ptr->args.channel.handle_upper);
    }

    pndv_data.ip2pn_handle[ch_handle] = rqb_ptr->args.channel.handle;
    pndv_in_ip2pn_open_sm(pndv_data.ip2pn.open_state);
}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_set_system_description()
{
    IP2PN_UPPER_RQB_PTR_TYPE stp_rqb_ptr = pndv_data.ip2pn.stp_rqb_ptr; 

    PNDV_GET_SNMP_SYS_DESCR(&stp_rqb_ptr->args.mib2_sysdescr_set.sysDescr, &stp_rqb_ptr->args.mib2_sysdescr_set.Length);

    PNDV_RQB_SET_HANDLE(stp_rqb_ptr, pndv_data.ip2pn_handle[PNDV_INDEX_PATH_IP2PN_GLO_APP]);
    PNDV_RQB_SET_OPCODE(stp_rqb_ptr, IP2PN_OPC_MIB2_SYSDESCR_SET);

    PNDV_REQUEST(stp_rqb_ptr, LSA_COMP_ID_IP2PN);
}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_set_initial_multiple_snmp()
{
    PNIO_UINT8* name_ptr        = 0;
    PNIO_UINT32 name_length     = 0;
    PNIO_UINT8* contact_ptr     = 0;
    PNIO_UINT32 contact_length  = 0;
    PNIO_UINT8* location_ptr    = 0;
    PNIO_UINT32 location_length = 0;

    PNDV_GET_SNMP_DATA(&name_ptr, &name_length, &contact_ptr, &contact_length, &location_ptr, &location_length);
    if (!(name_ptr || contact_ptr || location_ptr))
    {
        /* no SNMP data present -> go to next step directly */
        pndv_in_ip2pn_open_sm(PNDV_IP2PN_CH_STATE_OPEN_IP2PN_CM);
        return;
    }

    PNIO_UINT8* data_copy_ptr;
    PNIO_UINT32 copy_pos      = 0;
    PNIO_UINT32 total_rec_len = 0;

    IP2PN_UPPER_RQB_PTR_TYPE stp_rqb_ptr = pndv_data.ip2pn.stp_rqb_ptr;

    /* Calculate total record length */   
    if (name_ptr)
    {
        total_rec_len = (total_rec_len + name_length + PNDV_PRM_REC_BLOCK_HEADER_LEN + PNDV_PRM_REC_BLOCK_LEN_SNMP + 3) >> 2;
        total_rec_len = total_rec_len << 2;
    }

    if (contact_ptr)
    {
        total_rec_len = (total_rec_len + contact_length + PNDV_PRM_REC_BLOCK_HEADER_LEN + PNDV_PRM_REC_BLOCK_LEN_SNMP + 3) >> 2;
        total_rec_len = total_rec_len << 2;
    }

    if (location_ptr)
    {
        total_rec_len = (total_rec_len + location_length + PNDV_PRM_REC_BLOCK_HEADER_LEN + PNDV_PRM_REC_BLOCK_LEN_SNMP + 3) >> 2;
        total_rec_len = total_rec_len << 2;
    }

    PNDV_ALLOC_MEM(((LSA_VOID_PTR_TYPE)&stp_rqb_ptr->args.record.record_data), total_rec_len);
    if (stp_rqb_ptr->args.record.record_data == LSA_NULL)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, (LSA_UINT32)0);
    }

    data_copy_ptr = stp_rqb_ptr->args.record.record_data;

    write_snmp_record_data_element(name_ptr, name_length, data_copy_ptr, &copy_pos, PNDV_IP2PN_SNMP_BLOCK_INDEX_NAME_TYPE);
    write_snmp_record_data_element(contact_ptr, contact_length, data_copy_ptr, &copy_pos, PNDV_IP2PN_SNMP_BLOCK_INDEX_CONTACT_TYPE);
    write_snmp_record_data_element(location_ptr, location_length, data_copy_ptr, &copy_pos, PNDV_IP2PN_SNMP_BLOCK_INDEX_LOCATION_TYPE);

    stp_rqb_ptr->args.record.InterfaceID        = 0; // interface 0 = global
    stp_rqb_ptr->args.record.record_index       = IP2PN_RECORD_INDEX_MULTIPLE_SNMP;
    stp_rqb_ptr->args.record.record_data_length = total_rec_len;
    stp_rqb_ptr->args.record.mk_rema            = IP2PN_RECORD_REMANENT_YES; 

    PNDV_RQB_SET_HANDLE(stp_rqb_ptr, pndv_data.ip2pn_handle[PNDV_INDEX_PATH_IP2PN_GLO_APP]);
    PNDV_RQB_SET_OPCODE(stp_rqb_ptr, IP2PN_OPC_RECORD_WRITE);

    PNDV_REQUEST(stp_rqb_ptr, LSA_COMP_ID_IP2PN);
}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_record_write_done(IP2PN_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    LSA_UINT16 ret_val = PNDV_RQB_GET_RESPONSE(rqb_ptr);
    if (ret_val != LSA_RSP_OK)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, ret_val);
    }

    PNIO_UINT32 record_index = rqb_ptr->args.record.record_index;
    if (!((record_index == IP2PN_RECORD_INDEX_MULTIPLE_SNMP) || (record_index == IP2PN_RECORD_INDEX_IPSUITE)))
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, (LSA_UINT32)record_index);
    }

    // free the memory that has been allocated for record data
    PNDV_FREE_MEM(&ret_val, rqb_ptr->args.record.record_data);
    if (ret_val != LSA_RET_OK)
    {
        pndv_in_fatal_error( PNDV_MODULE, __LINE__, ret_val);
    }
    rqb_ptr->args.record.record_data = LSA_NULL;

    if (record_index == IP2PN_RECORD_INDEX_MULTIPLE_SNMP)
    {
        pndv_in_ip2pn_open_sm(PNDV_IP2PN_CH_STATE_OPEN_IP2PN_CM);
    }
    else if (record_index == IP2PN_RECORD_INDEX_IPSUITE)
    {
        PNDV_FREE_RQB(&ret_val, rqb_ptr); // free ipsuite startup RQB resources
        if(ret_val != LSA_RET_OK)
        {
            pndv_in_fatal_error( PNDV_MODULE, __LINE__, ret_val);
        }

        pndv_data.ip2pn.ipsuite_stp_rqb_ptr = LSA_NULL;
        pndv_in_ip2pn_set_initial_ipsuite_done();
    }
}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_set_if_decription()
{
    LSA_UINT8*  if_descr_ptr = 0;
    LSA_UINT32  if_descr_len = 0; 

    IP2PN_UPPER_RQB_PTR_TYPE stp_rqb_ptr = pndv_data.ip2pn.stp_rqb_ptr;
    
    if (pndv_data.ip2pn.set_port_id == 0)     // interface description
    {
        PNDV_GET_IFDESCR_INTERFACE(&if_descr_ptr, &if_descr_len);
        PNDV_ASSERT(!((if_descr_len == 0) && (if_descr_ptr != 0)));
    }
    else if (pndv_data.ip2pn.set_port_id > 0) // port description
    {
        PNDV_GET_IFDESCR_PORT(&if_descr_ptr, &if_descr_len, pndv_data.ip2pn.set_port_id);
        PNDV_ASSERT(if_descr_ptr != 0);
    }
    else
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, pndv_data.ip2pn.open_state);
    }

    stp_rqb_ptr->args.mib2_ifdescr_set.InterfaceID = 1;
    stp_rqb_ptr->args.mib2_ifdescr_set.PortID      = pndv_data.ip2pn.set_port_id;
    stp_rqb_ptr->args.mib2_ifdescr_set.ifDescr     = if_descr_ptr;
    PNDV_ASSERT(if_descr_len < 0xffff) //make sure smaller than max uint16 value.
    stp_rqb_ptr->args.mib2_ifdescr_set.Length      = (LSA_UINT16)if_descr_len;

    PNDV_RQB_SET_HANDLE(stp_rqb_ptr, pndv_data.ip2pn_handle[PNDV_INDEX_PATH_IP2PN_GLO_APP]);
    PNDV_RQB_SET_OPCODE(stp_rqb_ptr, IP2PN_OPC_MIB2_IFDESCR_SET);

    pndv_data.ip2pn.set_port_id++;

    PNDV_REQUEST(stp_rqb_ptr, LSA_COMP_ID_IP2PN);
}

/*
* Provide an RQB for the IP2PN Rema Indication of stack
*/
/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_provide_rema_resources()
{
    IP2PN_UPPER_RQB_PTR_TYPE rema_ind_rqb_ptr;

    PNDV_ALLOC_RQB(&rema_ind_rqb_ptr, sizeof(IP2PN_RQB_TYPE));
    if (rema_ind_rqb_ptr == LSA_NULL)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    PNDV_ALLOC_MEM(((LSA_VOID_PTR_TYPE)&rema_ind_rqb_ptr->args.rema_data.Data), IP2PN_MAX_REMA_DATA_SIZE);
    if (rema_ind_rqb_ptr->args.rema_data.Data == LSA_NULL)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    rema_ind_rqb_ptr->args.rema_data.DataSize    = IP2PN_MAX_REMA_DATA_SIZE;
    rema_ind_rqb_ptr->args.rema_data.InterfaceID = 0; // don't care for req
    rema_ind_rqb_ptr->args.rema_data.Index       = 0; // don't care for req
    rema_ind_rqb_ptr->args.rema_data.DataLength  = 0; // don't care for req

    PNDV_RQB_SET_HANDLE(rema_ind_rqb_ptr, pndv_data.ip2pn_handle[PNDV_INDEX_PATH_IP2PN_GLO_APP]);
    PNDV_RQB_SET_OPCODE(rema_ind_rqb_ptr, IP2PN_OPC_REMA_INDICATION);

    PNDV_REQUEST(rema_ind_rqb_ptr, LSA_COMP_ID_IP2PN);

    pndv_in_ip2pn_open_sm(pndv_data.ip2pn.open_state);
}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_handle_rema_indication(IP2PN_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    LSA_UINT16 ret_val = PNDV_RQB_GET_RESPONSE(rqb_ptr);
    if (ret_val != LSA_RSP_OK)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, ret_val);
    }

    IP2PN_UPPER_REMA_DATA_PTR_TYPE pRemaData = &rqb_ptr->args.rema_data;
    switch (pRemaData->Index)
    {
        case IP2PN_RECORD_INDEX_IPSUITE:
        {
            if (pRemaData->DataLength)
            {
                PNDV_CFG_REMA_WRITE_IP_DATA(pRemaData->Data, pRemaData->DataLength);
                PNDV_CFG_REPORT_NEW_IP_DATA(*((PNIO_UINT32*)(&(pRemaData->Data)[PNDV_RD_BLK_HEADER_SIZE + 2])),
                                            *((PNIO_UINT32*)(&(pRemaData->Data)[PNDV_RD_BLK_HEADER_SIZE + 2 + 4])),
                                            *((PNIO_UINT32*)(&(pRemaData->Data)[PNDV_RD_BLK_HEADER_SIZE + 2 + 8])));
            }
            else
            {
                PNDV_CFG_REMA_DELETE_IP_DATA();
            }
            break;
        }
        case IP2PN_RECORD_INDEX_MULTIPLE_SNMP:
        {
            if (pRemaData->DataLength)
            {
                PNDV_CFG_REMA_WRITE_SNMP_DATA(pRemaData->Data, pRemaData->DataLength);
            }
            else
            {
                PNDV_CFG_REMA_DELETE_SNMP_DATA();
            }
            break;
        }
        default:
        {
            break;
        }
    }

    /* reprovide the rema indication RQB to IP2PN for the next rema indication */
    rqb_ptr->args.rema_data.DataSize    = IP2PN_MAX_REMA_DATA_SIZE;
    rqb_ptr->args.rema_data.InterfaceID = 0; // don't care for req
    rqb_ptr->args.rema_data.Index       = 0; // don't care for req
    rqb_ptr->args.rema_data.DataLength  = 0; // don't care for req

    PNDV_RQB_SET_HANDLE(rqb_ptr, pndv_data.ip2pn_handle[PNDV_INDEX_PATH_IP2PN_GLO_APP]);
    PNDV_RQB_SET_OPCODE(rqb_ptr, IP2PN_OPC_REMA_INDICATION);

    PNDV_REQUEST(rqb_ptr, LSA_COMP_ID_IP2PN);
}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_free_stp_rqb()
{
    LSA_UINT16 resp;

    PNDV_FREE_RQB(&resp, pndv_data.ip2pn.stp_rqb_ptr);
    if (resp != LSA_OK)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, resp);
    }

    pndv_data.ip2pn.stp_rqb_ptr = LSA_NULL;

    pndv_in_ip2pn_open_done();
}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_open_sm(PNDV_IP2PN_CH_OPEN_STATE state)
{
    switch (state)
    {
        case PNDV_IP2PN_CH_STATE_OPEN_IP2PN_GLO_APP:
        {
            pndv_data.ip2pn.open_state = PNDV_IP2PN_CH_STATE_SYSDESCR_SET;

            pndv_in_ip2pn_open_channel(PNDV_INDEX_PATH_IP2PN_GLO_APP);
            break;
        }
        case PNDV_IP2PN_CH_STATE_SYSDESCR_SET:
        {
            pndv_data.ip2pn.open_state = PNDV_IP2PN_CH_STATE_SET_INITIAL_MULT_SNMP;

            pndv_in_ip2pn_set_system_description();
            break;
        }
        case PNDV_IP2PN_CH_STATE_SET_INITIAL_MULT_SNMP:
        {
            pndv_data.ip2pn.open_state = PNDV_IP2PN_CH_STATE_OPEN_IP2PN_CM;

            pndv_in_ip2pn_set_initial_multiple_snmp();
            break;
        }
        case PNDV_IP2PN_CH_STATE_OPEN_IP2PN_CM:
        {
            pndv_data.ip2pn.open_state = PNDV_IP2PN_CH_STATE_SET_IFDESCR_INTERFACE;

            pndv_in_ip2pn_open_channel(PNDV_INDEX_PATH_IP2PN_IF_CM);
            break;
        }
        case PNDV_IP2PN_CH_STATE_SET_IFDESCR_INTERFACE:
        {
            pndv_data.ip2pn.open_state = PNDV_IP2PN_CH_STATE_SET_IFDESCR_PORT;

            pndv_in_ip2pn_set_if_decription();
            break;
        }
        case PNDV_IP2PN_CH_STATE_SET_IFDESCR_PORT: 
        {
            if (pndv_data.ip2pn.set_port_id == IOD_CFG_PDEV_NUMOF_PORTS) // if entering for the last port's descr set
            {
                pndv_data.ip2pn.open_state = PNDV_IP2PN_CH_STATE_PROVIDE_REMA_RES;
            }

            pndv_in_ip2pn_set_if_decription();
            break;
        }
        case PNDV_IP2PN_CH_STATE_PROVIDE_REMA_RES:
        {
            pndv_data.ip2pn.open_state = PNDV_IP2PN_CH_STATE_FREE_STP_RQB;

            pndv_in_ip2pn_provide_rema_resources();
            break;
        }
        case PNDV_IP2PN_CH_STATE_FREE_STP_RQB:
        {
            pndv_data.ip2pn.open_state = PNDV_IP2PN_CH_STATE_OPEN_DONE;

            pndv_in_ip2pn_free_stp_rqb();
            break;
        }
        default:
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, state);
            break;
        }
    }
}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_set_initial_ipsuite()
{
    PNIO_UINT8* ipsuite_ptr = 0;
    PNIO_UINT32 ipsuite_len = 0;

    PNDV_GET_IP_DATA(&ipsuite_ptr, &ipsuite_len);
    if ((ipsuite_ptr == LSA_NULL) || (ipsuite_len != 12)) // in case of no IP, expect zeroed data to be returned
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, ipsuite_len);
    }

    PNIO_UINT8 resp = PNDV_VALIDATE_INITIAL_IP(ipsuite_ptr);
    if (resp == PNIO_NOT_OK) // in case of corrupted ip data is stored, sets ip suite to 0
    {
        PNDV_MEMSET(ipsuite_ptr, 0, ipsuite_len);
    }

    IP2PN_UPPER_RQB_PTR_TYPE pIpSuiteRqb = pndv_data.ip2pn.ipsuite_stp_rqb_ptr;

    PNIO_UINT8* record_data_ptr;
    PNIO_UINT32 copy_pos        = 0;
    PNIO_UINT32 total_rec_len   = (PNDV_PRM_REC_BLOCK_LEN_IP_SUITE + ipsuite_len + 3) >> 2;

    total_rec_len = total_rec_len << 2;

    PNDV_ALLOC_RQB(&pIpSuiteRqb, sizeof(IP2PN_RQB_TYPE));
    if (pIpSuiteRqb == LSA_NULL)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    PNDV_ALLOC_MEM(&pIpSuiteRqb->args.record.record_data, (PNDV_PRM_REC_BLOCK_HEADER_LEN + total_rec_len));
    if (pIpSuiteRqb->args.record.record_data == LSA_NULL)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    /* prepare ipsuite record data header */
    record_data_ptr = pIpSuiteRqb->args.record.record_data;
    record_data_ptr[copy_pos++] = (PNIO_UINT8)(PNDV_PRM_REC_BLOCK_TYPE_IP_SUITE >> 8);
    record_data_ptr[copy_pos++] = (PNIO_UINT8)(PNDV_PRM_REC_BLOCK_TYPE_IP_SUITE);
    record_data_ptr[copy_pos++] = (PNIO_UINT8)((total_rec_len) >> 8);
    record_data_ptr[copy_pos++] = (PNIO_UINT8)((total_rec_len));
    record_data_ptr[copy_pos++] = (PNIO_UINT8)(PNDV_PRM_REC_BLOCK_VERS_H);
    record_data_ptr[copy_pos++] = (PNIO_UINT8)(PNDV_PRM_REC_BLOCK_VERS_L);
    record_data_ptr[copy_pos++] = (PNIO_UINT8)(0x00);
    record_data_ptr[copy_pos++] = (PNIO_UINT8)(0x00);
    PNDV_COPY_BYTE((PNIO_UINT8*)&record_data_ptr[copy_pos], ipsuite_ptr, ipsuite_len);

    pIpSuiteRqb->args.record.InterfaceID        = 1;
    pIpSuiteRqb->args.record.record_index       = IP2PN_RECORD_INDEX_IPSUITE;
    pIpSuiteRqb->args.record.record_data_length = PNDV_PRM_REC_BLOCK_HEADER_LEN + total_rec_len;
    pIpSuiteRqb->args.record.mk_rema            = IP2PN_RECORD_REMANENT_YES;

    pndv_in_write_debug_buffer_3__(PNDV_DC_IP2PN_SET_DEVICE_IP,
                                   *((PNIO_UINT32*)ipsuite_ptr) & 0xFFFF /*IP*/,
                                   *((PNIO_UINT32*)ipsuite_ptr) >> 16    /*IP*/,
                                   *((PNIO_UINT32*)ipsuite_ptr + 1)      /*netmask*/);

    PNDV_RQB_SET_HANDLE(pIpSuiteRqb, pndv_data.ip2pn_handle[PNDV_INDEX_PATH_IP2PN_GLO_APP]);
    PNDV_RQB_SET_OPCODE(pIpSuiteRqb, IP2PN_OPC_RECORD_WRITE);

    PNDV_REQUEST(pIpSuiteRqb, LSA_COMP_ID_IP2PN);
}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_set_initial_ipsuite_done()
{
    pndv_sm(PNDV_SM_EVENT_IP2PN_SET_IPSUITE_DONE);
}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_open()
{
    pndv_data.ip2pn.ipsuite_rqb_in_use = PNIO_FALSE;
    pndv_data.ip2pn.rtf_rqb_in_use     = PNIO_FALSE;
    pndv_data.ip2pn.open_state         = PNDV_IP2PN_CH_STATE_OPEN_IP2PN_GLO_APP;
    pndv_data.ip2pn.set_port_id        = 0;

    pndv_in_ip2pn_open_sm(PNDV_IP2PN_CH_STATE_OPEN_IP2PN_GLO_APP);
}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_open_done()
{
    pndv_sm(PNDV_SM_EVENT_IP2PN_OPEN_DONE);
}


/*****************************************************************************/
/*  IP2PN Application Call Functions                                         */
/*****************************************************************************/

/**
 * - Called from PNDV when an RTF command is written by DCP.
 * - It sends the command to IP2PN so that it can reset and acknowledges the service
 * afterwards by calling the cbf.
 */
PNIO_VOID pndv_in_ip2pn_reset_to_factory()
{
    if (pndv_data.ip2pn.open_state != PNDV_IP2PN_CH_STATE_OPEN_DONE)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, pndv_data.ip2pn.open_state);
    }

    if (pndv_data.ip2pn.rtf_rqb_in_use != PNIO_FALSE)
    {
        pndv_in_write_debug_buffer_1__(PNDV_DC_IP2PN_RTF_REQUEST, 1);
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    pndv_in_write_debug_buffer_1__(PNDV_DC_IP2PN_RTF_REQUEST, 0);

    pndv_data.ip2pn.rtf_rqb_in_use                            = PNIO_TRUE;
    pndv_data.ip2pn.rtf_rqb.args.reset_to_factory.InterfaceID = pndv_data.interface_id;

    PNDV_RQB_SET_HANDLE(&pndv_data.ip2pn.rtf_rqb, pndv_data.ip2pn_handle[PNDV_INDEX_PATH_IP2PN_GLO_APP]);
    PNDV_RQB_SET_OPCODE(&pndv_data.ip2pn.rtf_rqb, IP2PN_OPC_RESET_TO_FACTORY);

    PNDV_REQUEST(&pndv_data.ip2pn.rtf_rqb, LSA_COMP_ID_IP2PN);
}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_reset_to_factory_done(IP2PN_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    pndv_data.ip2pn.rtf_rqb_in_use = PNIO_FALSE;

    LSA_RESPONSE_TYPE resp = PNDV_RQB_GET_RESPONSE(rqb_ptr);
    if (resp != IP2PN_RSP_OK)
    {
        // pndv_in_fatal_error(PNDV_MODULE, __LINE__, resp);
        pndv_in_cm_pd_dcp_indication_response(PNIO_FALSE);
    }
    else
    {
        pndv_in_cm_pd_dcp_indication_response(PNIO_TRUE);
    }
}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_set_ipsuite(PNDV_IP2PN_IPSUITE_INFO_T* ipsuite_ptr, PNIO_UINT8 mk_remanent, PNDV_IP2PN_REQ_OWNER_TYPE req_owner)
{
    if (pndv_data.ip2pn.open_state != PNDV_IP2PN_CH_STATE_OPEN_DONE)
    {
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, pndv_data.ip2pn.open_state);
    }

    if (pndv_data.ip2pn.ipsuite_rqb_in_use != PNIO_FALSE)
    {
        pndv_in_write_debug_buffer_3__(PNDV_DC_IP2PN_SET_DEVICE_IP_RESOURCE_ERR, ipsuite_ptr->ip_address & 0xFFFF, ipsuite_ptr->ip_address >> 16, ipsuite_ptr->netmask);
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);
    }

    PNDV_ASSERT((req_owner == PNDV_IP2PN_REQ_OWNER_DCP_TYPE) || (req_owner == PNDV_IP2PN_REQ_OWNER_USER_TYPE));

    pndv_in_write_debug_buffer_3__(PNDV_DC_IP2PN_SET_DEVICE_IP, ipsuite_ptr->ip_address & 0xFFFF, ipsuite_ptr->ip_address >> 16, ipsuite_ptr->netmask);
    pndv_data.ip2pn.ipsuite_rqb_in_use = PNIO_TRUE;
    pndv_data.ip2pn.ipsuite_req_owner  = req_owner; // request identifier (user or DCP)

    IP2PN_IPSUITE_TYPE* pIpSuite = &pndv_data.ip2pn.ipsuite_rqb.args.ipsuite;
    pIpSuite->InterfaceID = pndv_data.interface_id;
    pIpSuite->IpAddr      = ipsuite_ptr->ip_address;
    pIpSuite->SubnetMask  = ipsuite_ptr->netmask;
    pIpSuite->Gateway     = ipsuite_ptr->gateway;
    pIpSuite->MkRemanent  = mk_remanent;
    pIpSuite->IsEmergency = IP2PN_IPSUITE_IS_EMERGENCY_NO;
    pIpSuite->ViaDHCP     = IP2PN_IPSUITE_VIA_DHCP_NO;

    PNDV_RQB_SET_HANDLE(&pndv_data.ip2pn.ipsuite_rqb, pndv_data.ip2pn_handle[PNDV_INDEX_PATH_IP2PN_IF_CM]);
    PNDV_RQB_SET_OPCODE(&pndv_data.ip2pn.ipsuite_rqb, IP2PN_OPC_IPSUITE_SET);

    PNDV_REQUEST(&pndv_data.ip2pn.ipsuite_rqb, LSA_COMP_ID_IP2PN);
}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_set_ipsuite_done(IP2PN_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    pndv_data.ip2pn.ipsuite_rqb_in_use = PNIO_FALSE;

    LSA_RESPONSE_TYPE resp = PNDV_RQB_GET_RESPONSE(rqb_ptr);

    switch (pndv_data.ip2pn.ipsuite_req_owner)
    {
        case PNDV_IP2PN_REQ_OWNER_USER_TYPE:
        {
            if (resp != LSA_RSP_OK)
            {
                pndv_in_write_debug_buffer_1__(PNDV_DC_IP2PN_SET_DEVICE_IP_RESP_NOT_OK, resp);
                pndv_in_fatal_error(PNDV_MODULE, __LINE__, resp);
            }
            break;
        }
        case PNDV_IP2PN_REQ_OWNER_DCP_TYPE:
        {
            pndv_in_cm_pd_dcp_indication_response(resp == LSA_RSP_OK);
            break;
        }
        default:
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, pndv_data.ip2pn.ipsuite_req_owner);
            break;
        }
    }

}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_set_ipsuite_user_req(PNIO_UINT32 ip_address, PNIO_UINT32 netmask, PNIO_UINT32 gateway, PNIO_UINT8 mk_remanent)
{
    PNDV_IP2PN_IPSUITE_INFO_T ipsuite;
    ipsuite.ip_address = ip_address;
    ipsuite.netmask    = netmask;
    ipsuite.gateway    = gateway;

    PNIO_UINT8 remanent = (mk_remanent == PNIO_TRUE) ? IP2PN_IPSUITE_MK_REMANENT_YES : IP2PN_IPSUITE_MK_REMANENT_NO;;

    pndv_in_ip2pn_set_ipsuite(&ipsuite, remanent, PNDV_IP2PN_REQ_OWNER_USER_TYPE);
}

/*****************************************************************************/
PNIO_VOID pndv_ip2pn_callback(IP2PN_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    pndv_in_write_debug_buffer_2__(PNDV_DC_IP2PN_CALLBACK, PNDV_RQB_GET_OPCODE(rqb_ptr), PNDV_RQB_GET_RESPONSE(rqb_ptr));

    switch (PNDV_RQB_GET_OPCODE(rqb_ptr))
    {
        case IP2PN_OPC_OPEN_CHANNEL:
        {
            pndv_in_ip2pn_open_channel_done(rqb_ptr);
            break;
        }
        case IP2PN_OPC_MIB2_SYSDESCR_SET:
        case IP2PN_OPC_MIB2_IFDESCR_SET:
        {
            if (PNDV_RQB_GET_RESPONSE(rqb_ptr) != LSA_RSP_OK)
            {
                pndv_in_fatal_error(PNDV_MODULE, __LINE__, PNDV_RQB_GET_RESPONSE(rqb_ptr));
            }
            pndv_in_ip2pn_open_sm(pndv_data.ip2pn.open_state);
            break;
        }
        case IP2PN_OPC_RECORD_WRITE:
        {
            pndv_in_ip2pn_record_write_done(rqb_ptr);
            break;
        }
        case IP2PN_OPC_REMA_INDICATION:
        {
            pndv_in_ip2pn_handle_rema_indication(rqb_ptr);
            break;
        }
        case IP2PN_OPC_RESET_TO_FACTORY:
        {
            pndv_in_ip2pn_reset_to_factory_done(rqb_ptr);
            break;
        }
        case IP2PN_OPC_IPSUITE_SET:
        {
            pndv_in_ip2pn_set_ipsuite_done(rqb_ptr);
            break;
        }
        default:
        {
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, (LSA_UINT32) rqb_ptr->args.channel.handle_upper);
        }
    }
}

/*****************************************************************************/
PNIO_VOID pndv_in_ip2pn_to_appl_cbf(IP2PN_UPPER_RQB_PTR_TYPE rqb_ptr)
{
    PNDV_RQB_APPL_REQUEST(rqb_ptr, pndv_ip2pn_callback);
}


/*****************************************************************************/
/*  IP2PN Helper Functions                                                   */
/*****************************************************************************/

PNIO_VOID write_snmp_record_data_element(PNIO_UINT8* element_ptr, PNIO_UINT32 element_length,
                                         PNIO_UINT8* data_copy_ptr, PNIO_UINT32* copy_pos_ptr, PNDV_IP2PN_SNMP_BLOCK_INDEX_TYPE block_type)
{
    PNIO_UINT32 copy_pos = *copy_pos_ptr;

    if (element_ptr)
    {
        PNIO_UINT32 copy_pos_data_end;
        PNIO_UINT32 BlockLength          = element_length + 4;
        PNIO_UINT32 interface_subslot_nr = pndv_data.cfg.pd.interface_subslot_nr;

        /* BlockType / Index = 0x00011003 */
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)0x00; 
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)0x01;
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)0x10;
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)block_type;
        /* API = 0x00000000 */
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)0x00;
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)0x00;
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)0x00;
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)0x00;
        /* Slot = 0x0000 */
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)0x00;
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)0x00;
        /* SubSlot = 0x8000 */
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)(interface_subslot_nr >> 8);
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)(interface_subslot_nr);
        /* BlockLength */
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)((BlockLength) >> 24);
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)((BlockLength) >> 16);
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)((BlockLength) >> 8);
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)((BlockLength));
        /* Comp_ID = 0x0000 */
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)0x00;
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)0x00;
        /* Reserved = 0x0000 */
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)0x00;
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)0x00;
        /* SNMP-Data type 0x0004 */
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)0x00;
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)0x04;
        /* SNMP-Data length */
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)((element_length) >> 8);
        data_copy_ptr[copy_pos++] = (PNIO_UINT8)((element_length));

        /* SNMP-Data */
        PNDV_COPY_BYTE((PNIO_UINT8*)&data_copy_ptr[copy_pos], (PNIO_UINT8*)element_ptr, element_length);
        copy_pos          = copy_pos + element_length;
        copy_pos_data_end = copy_pos;

        while (copy_pos < ((copy_pos_data_end + 3) & ~0x03))
        {
            data_copy_ptr[copy_pos++] = (PNIO_UINT8)(0x00);
        }
    }

    *copy_pos_ptr = copy_pos;
}

/*****************************************************************************/
PNIO_UINT16 get_ipsuite_data_from_rema_record(IP2PN_UPPER_MEM_U8_CONST_PTR_TYPE pRecData,
                                              PNIO_UINT32                       RecordLen,
                                              PNDV_IP2PN_IPSUITE_INFO_T*        pIpSuite)
{
    PNIO_UINT32 offset = 0;

    if (RecordLen < PNDV_RD_BLK_HEADER_SIZE)
    {
        return PNIO_NOT_OK;
    }

    // skip paddings
    offset += PNDV_RD_BLK_HEADER_SIZE + 2/*Reserved*/;

    PNDV_COPY_BYTE(&pIpSuite->ip_address, pRecData + offset, sizeof(PNIO_UINT32));
    offset += sizeof(PNIO_UINT32);

    PNDV_COPY_BYTE(&pIpSuite->netmask, pRecData + offset, sizeof(PNIO_UINT32));
    offset += sizeof(PNIO_UINT32);

    PNDV_COPY_BYTE(&pIpSuite->gateway, pRecData + offset, sizeof(PNIO_UINT32));

    return PNIO_OK;
}

/*****************************************************************************/
PNIO_UINT16 get_snmp_data_from_rema_record(IP2PN_UPPER_MEM_U8_CONST_PTR_TYPE pRecData,
                                           PNIO_UINT32                       RecordLen,
                                           PNDV_IP2PN_SNMP_DATA_TYPE_T*      pSnmpData)
{
    LSA_UINT32 index = 0;
    IP2PN_UPPER_MEM_U8_CONST_PTR_TYPE pRecord = pRecData; // local copy of record data

    if (pRecord == LSA_NULL)
    {
        return IP2PN_RSP_ERR_PARAM;
    }

    if (RecordLen < PNDV_RD_REC_IP2PN_REMA_RECORD_HEADER_LENGTH) /* sizeof(BlockHeader) */
    {
        return IP2PN_RSP_ERR_DATA;
    }

    while (RecordLen > index)
    {
        /* (i) BlockHeader (RemaWriteHeader) is part of the RQB:                                */
        /*                 RemaRecordIndex(u32), API(u32), SlotNumber(u16), SubslotNumber(u16), */
        /*                 RemaRecordDataLength (u32), RemaCompID(u16), Reserved(u16)           */
        LSA_UINT32 BlockIndex;
        LSA_UINT32 BlockLength;

        PNDV_COPY_BYTE(&BlockIndex, pRecord, sizeof(PNIO_UINT32)); /* RemaRecordIndex */
        BlockIndex = PNDV_NTOHL(BlockIndex);

        index   += sizeof(PNIO_UINT32);
        pRecord += sizeof(PNIO_UINT32);

        index   += sizeof(PNIO_UINT32);        /* API */
        if (index > RecordLen)
        {
            return IP2PN_RSP_ERR_DATA; 
        }
        pRecord += sizeof(PNIO_UINT32);

        index   += sizeof(PNIO_UINT32);       /* SlotNumber(u16) and SubslotNumber(u16) */
        pRecord += sizeof(PNIO_UINT32);

        PNDV_COPY_BYTE(&BlockLength, pRecord, sizeof(PNIO_UINT32)); /* RemaRecordDataLength */
        BlockLength = PNDV_NTOHL(BlockLength);

        index   += sizeof(PNIO_UINT32);
        pRecord += sizeof(PNIO_UINT32);

        index   += sizeof(PNIO_UINT16);       /* CompID */
        if (index > RecordLen)
        { 
            return IP2PN_RSP_ERR_DATA;
        }
        pRecord += sizeof(PNIO_UINT16);

        index   += sizeof(PNIO_UINT16);       /* Reserved */
        pRecord += sizeof(PNIO_UINT16);


        /* SNMP DATA BLOCK - Block Index Handling
         * Expected Indexes:
         *      IP2PN_RECORD_INDEX_SYS_NAME             (parse)
         *      IP2PN_RECORD_INDEX_SYS_CONTACT          (parse)
         *      IP2PN_RECORD_INDEX_SYS_LOCATION         (parse)
         *      IP2PN_RECORD_INDEX_LLDP_INTERFACE_DATA  (skip)
         *      IP2PN_RECORD_INDEX_LLDP_PORT_DATA       (skip)
        */
        if ((BlockIndex == IP2PN_RECORD_INDEX_LLDP_INTERFACE_DATA) ||
            (BlockIndex == IP2PN_RECORD_INDEX_LLDP_INTERFACE_DATA))
        {
            index   += BlockLength;
            pRecord += BlockLength;
            continue;
        }

        PNIO_UINT16* snmp_data_element_len_ptr = 0;
        PNIO_UINT8** snmp_data_element_ptr_ptr = 0;
        switch (BlockIndex)
        {
            case IP2PN_RECORD_INDEX_SYS_NAME:
            {
                snmp_data_element_len_ptr = &(pSnmpData->SysNameLen);
                snmp_data_element_ptr_ptr = &(pSnmpData->pSysName);
                break;
            }
            case IP2PN_RECORD_INDEX_SYS_CONTACT:
            {
                snmp_data_element_len_ptr = &(pSnmpData->SysContactLen);
                snmp_data_element_ptr_ptr = &(pSnmpData->pSysContact);
                break;
            }
            case IP2PN_RECORD_INDEX_SYS_LOCATION:
            {
                snmp_data_element_len_ptr = &(pSnmpData->SysLocationLen);
                snmp_data_element_ptr_ptr = &(pSnmpData->pSysLocation);
                break;
            }
            default:
            {
                return IP2PN_RSP_ERR_INDEX;
            }
        }

        /********************* System Data Element Block: (Name, Contact or Location) ********************/
        /* SNMPDataType(u16), SNMPDataLength(u16), SNMPDataValue a)                                      */
        /* a) the value of the SNMPDataType shall be an ASN_STRING (value=4) with 0 to 255 octets.       */
        /*************************************************************************************************/
        LSA_UINT16 SNMPDataType;
        LSA_UINT16 SNMPDataLength;

        PNDV_COPY_BYTE(&SNMPDataType, pRecord, sizeof(PNIO_UINT16));    /* SNMPDataType */
        SNMPDataType = PNDV_NTOHS(SNMPDataType);

        if (SNMPDataType != PNDV_RD_REC_IP2PN_SNMP_STRING)
        {
            return IP2PN_RSP_ERR_BLOCK;
        }
        index   += sizeof(PNIO_UINT16);
        pRecord += sizeof(PNIO_UINT16);

        PNDV_COPY_BYTE(&SNMPDataLength, pRecord, sizeof(PNIO_UINT16));  /* SNMPDataLength */
        SNMPDataLength = PNDV_NTOHS(SNMPDataLength);
        
        index   += sizeof(PNIO_UINT16);
        pRecord += sizeof(PNIO_UINT16);

        // check data content length
        index += SNMPDataLength;
        if (SNMPDataLength > LLDP_MAX_CHASSIS_ID_STRING_SIZE)
        {
            return IP2PN_RSP_ERR_DATA;
        }

        // set snmp data element length
        *snmp_data_element_len_ptr = SNMPDataLength;

        // set snmp data element content
        *snmp_data_element_ptr_ptr = (PNIO_UINT8*)pRecord;
        if (SNMPDataLength == 0 && *snmp_data_element_len_ptr)
        {
            **snmp_data_element_ptr_ptr = (PNIO_UINT8)(*snmp_data_element_len_ptr); /* return a Null string */
        }

        pRecord += SNMPDataLength;

        /* skip padding bytes after System Data Element Block */
        while ((index & 3) != 0)
        {
            index += sizeof(PNIO_UINT8);
            if (*pRecord != 0)  /* check the paddings */
            {
                return IP2PN_RSP_ERR_DATA;
            }
            pRecord += sizeof(PNIO_UINT8);  /* padding byte   */
        }
    }

    if (!(RecordLen == index))
    {
        return IP2PN_RSP_ERR_DATA;
    }

    return IP2PN_RSP_OK; // the record is ok
}

/*****************************************************************************/
PNIO_UINT8 validate_ip_suite(PNIO_UINT8* pIpSuite)
{
    PNIO_UINT32 ip_address = PNDV_NTOHL(*(PNIO_UINT32*)(pIpSuite + 0));
    PNIO_UINT32 netmask    = PNDV_NTOHL(*(PNIO_UINT32*)(pIpSuite + 4));
    PNIO_UINT32 gateway    = PNDV_NTOHL(*(PNIO_UINT32*)(pIpSuite + 8));

    if ((ip_address == 0) && ((netmask != 0) || (gateway != 0)))
    {
        return PNIO_NOT_OK;
    }

    switch (netmask)
    {
        case 0x00000000: // /0 .... invalid due to IP-stacks interpret it as "derive mask from ip address" and not "no subnetting"
        case 0x80000000: // /1 .... invalid due to SPH IP Config
        case 0xC0000000: // /2 .... invalid due to SPH IP Config
        case 0xE0000000: // /3 .... invalid due to SPH IP Config
        case 0xFFFFFFFE: // /31 ... invalid due to SPH IP Config
        case 0xFFFFFFFF: // /32 ... invalid due to SPH IP Config
        {
            return PNIO_NOT_OK;
        }
        default:
        {
            /* mask composed of consecutive 1 test */
            PNIO_UINT32 temp = ~netmask + 1; // note: if x == 1111000... then ~x == 0000111... then ~x+1 == 0001000... which is 2^x
            if ((temp & (temp - 1)) != 0)    // 2^x check see OpenBSD, param.h
            {
                return PNIO_NOT_OK;
            }
            break;
        }
    }

    PNIO_UINT32 host0 = (ip_address & netmask);
    PNIO_UINT32 host1 = (ip_address | ~netmask);

    if ((ip_address == host0)                      ||
        (ip_address == host1)                      ||
        (ip_address == 0xFFFFFFFF)                 || // 255.255.255.255 (limited broadcast)
        ((ip_address & netmask) == 0)              || // net address {0, <Host-number>}
        ((ip_address & 0xFF000000)  == 0)          || // net address 0.0.0.0/8
        ((ip_address & 0xFF000000)  == 0x7F000000) || // 127.0.0.0/8 (loopback)
        ((ip_address & 0xF0000000)  == 0xE0000000) || // 224.0.0.0/4 (multicast)
        ((ip_address & 0xF0000000)  == 0xF0000000))   // 240.0.0.0/4 (reserved)
    {
        return PNIO_NOT_OK;
    }

    if ((gateway == 0) || (gateway == ip_address))  /* no gateway */
    {
        return PNIO_OK;
    }

    if ((gateway == 0xFFFFFFFF)                ||  // 255.255.255.255 (limited broadcast)
        ((gateway & 0xFF000000) == 0)          ||  // net address 0.0.0.0/8
        ((gateway & 0xFF000000) == 0x7F000000) ||  // 127.0.0.0/8 (loopback)
        ((gateway & 0xF0000000) == 0xE0000000) ||  // 224.0.0.0/4 (multicast)
        ((gateway & 0xF0000000) == 0xF0000000))    // 240.0.0.0/4 (reserved)
    {
        return PNIO_NOT_OK;
    }

    if ((gateway & netmask) != (ip_address & netmask)) // not in the same subnet
    {
        return PNIO_NOT_OK;
    }

    host0 = (gateway & netmask);
    host1 = (gateway | ~netmask);
    if ((gateway == host0) || (gateway == host1))
    {
        return PNIO_NOT_OK;
    }

    return PNIO_OK;
}

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
