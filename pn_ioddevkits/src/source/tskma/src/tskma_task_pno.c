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
/*  F i l e               &F: tskma_task_pno.c                          :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*   TSKMA message handler for PNO task                                      */
/*                                                                           */
/*****************************************************************************/

#include "tskma_inc.h"

#define TSKMA_MODULE_ID     TSKMA_MODULE_ID_TSKMA_TASK_PNO

#if DEBUG_TASKCNT
extern PNIO_UINT32          TaskCnt[MAXNUM_OF_TASKS];
#endif

//*----------------------------------------------------------------------------
//*	tskma_pnio_stack_open_done()
//*
//* Done function of pnio stack open
//*----------------------------------------------------------------------------
static PNIO_VOID tskma_pnio_stack_open_done(LSA_VOID* pRQBData)
{
    LSA_UINT16 retVal;
    PSI_UPPER_RQB_PTR_TYPE pRQB = (PSI_UPPER_RQB_PTR_TYPE)pRQBData;

    TSKMA_ASSERT(pRQB != LSA_NULL);
    TSKMA_ASSERT(PSI_RQB_GET_RESPONSE(pRQB) == PSI_OK);

    PSI_FREE_LOCAL_MEM(&retVal, ((PNIO_VOID**)&pRQB), (PNIO_VOID*)0, LSA_COMP_ID_PSI, 0);
    TSKMA_ASSERT(retVal == LSA_OK);

    TSKMA_RQB_SET_REQ_FCT_PTR(&tskma_data.cold_start.tskma_rqb, tskma_task_stp_perform_cold_start_sm);
    TSKMA_OS_SEND_MAIL(TSKMA_TASK_NUM_STP, &tskma_data.cold_start.tskma_rqb);
}


//*----------------------------------------------------------------------------
//* tskma_prepare_psi_startup_params()
//*
//* Fills PSI startup parameters to configure PSI
//*----------------------------------------------------------------------------
PNIO_VOID tskma_prepare_psi_startup_params(PSI_UPPER_LD_OPEN_PTR_TYPE pOpen)
{
    TSKMA_ASSERT(pOpen != NULL);

    /*---------------------------------------------------------------------------------*/
    /* LD Parameters                                                                   */
    /*---------------------------------------------------------------------------------*/
    pOpen->hd_count                                  = 1;
    pOpen->ld_in.ld_runs_on                          = PSI_LD_RUNS_ON_LIGHT;
    pOpen->psi_request_upper_done_ptr                = (PSI_UPPER_CALLBACK_FCT_PTR_TYPE)tskma_pnio_stack_open_done;
    // TCIP Arguments
    pOpen->tcip_args.tcip_copy_on_send               = (PSI_CFG_TCIP_CFG_COPY_ON_SEND == 1) ? PSI_FEATURE_ENABLE : PSI_FEATURE_DISABLE;
    pOpen->tcip_args.tcip_iperf3_start_instances     = 0; // PSI_CFG_OBSD_PNIO_CFG_IPERF3_ENABLE_SERVICE is not defined
    // IP2PN Arguments
    pOpen->ip2pn_args.ip2pn_snmp_disabled_at_startup = PSI_FEATURE_DISABLE;


    /*---------------------------------------------------------------------------------*/
    /* SOCK Parameters                                                                 */
    /*---------------------------------------------------------------------------------*/
    for (LSA_UINT8 i = 0; i < PSI_CFG_MAX_SOCK_APP_CHANNELS; i++)
    {
        PSI_SOCK_APP_CH_DETAIL_DETAILS_TYPE* pSockDetail = &pOpen->sock_args.sock_app_ch_details[i].sock_detail;
        pSockDetail->linger_time                     = 0;
        pSockDetail->rec_buffer_max_len              = 0;
        pSockDetail->send_buffer_max_len             = 0;
        pSockDetail->sock_close_at_once              = 0;
        pSockDetail->socket_option.SO_REUSEADDR_     = LSA_TRUE;  /* allow local address reuse */
        pSockDetail->socket_option.SO_DONTROUTE_     = LSA_FALSE; /* just use interface addresses */
        pSockDetail->socket_option.SO_SNDBUF_        = LSA_FALSE; /* send buffer size */
        pSockDetail->socket_option.SO_RCVBUF_        = LSA_FALSE; /* receive buffer size */
        pSockDetail->socket_option.SO_LINGER_        = LSA_FALSE; /* socket lingers on close */
        pSockDetail->socket_option.SO_TCPNODELAY_    = LSA_TRUE;  /* delayed ACK */
        pSockDetail->socket_option.SO_TCPNOACKDELAY_ = LSA_TRUE;  /* delay sending ACKs */
        pSockDetail->socket_option.SO_BROADCAST_     = LSA_FALSE; /* broadcast allowed */
    }


    /*---------------------------------------------------------------------------------*/
    /* HD Interface Parameters                                                         */
    /*---------------------------------------------------------------------------------*/
    PSI_HD_INPUT_PTR_TYPE pHdIf = &pOpen->hd_args[0];
    pHdIf->hd_id                       = 1;                           // HD number [1..N]
    pHdIf->rev_nr                      = 0;                           // Rev number for this HD - unused
    pHdIf->hd_location.hd_selection    = PSI_HD_SELECT_EMBEDDED;      // HD address information (PCI, MAC)
    pHdIf->hd_runs_on_level_ld         = PSI_HD_RUNS_ON_LEVEL_LD_YES; // 1: HD is on LD level / 0: HD is on separate level from LD
    pHdIf->nr_of_ports                 = IOD_CFG_PDEV_NUMOF_PORTS;    // nr of ports (within one HD)
    pHdIf->edd_type                    = LSA_COMP_ID_EDDP;            // EDDx type (LSA-COMP-ID) for this HD
    pHdIf->asic_type                   = PSI_ASIC_TYPE_ERTEC200P;     // ASIC type for this HD
    pHdIf->pn_dev_drv_type             = PSI_PNDEV_DRIVER_PNASIC;     // Information which LLIF is used (PnDevDrv, AFPacket, ...)
    pHdIf->ld_runs_on                  = PSI_LD_RUNS_ON_LIGHT;        // LD runs on application level
    pHdIf->send_clock_factor           = 32;                          // SendClockFactor = CycleBaseFactor
    pHdIf->allow_non_pnio_mautypes     = PSI_FEATURE_DISABLE;         // PSI_FEATURE_ENABLE: 10Mbps and half-duplex (for all speeds) are allowed, PSI_FEATURE_DISABLE: not allowed
    pHdIf->nos_allow_upper_cases       = PSI_FEATURE_ENABLE;
    pHdIf->fill_active                 = PSI_FEATURE_ENABLE;          // PSI_FEATURE_ENABLE: enable FeedInLoadLimitation support, PSI_FEATURE_DISABLE: disable FeedInLoadLimitation support
    pHdIf->io_configured               = PSI_FEATURE_ENABLE;          // PSI_FEATURE_ENABLE: IO data are configured, PSI_FEATURE_DISABLE: This PN Interface is unable to have provider and consumer (= only NRT interface)
    pHdIf->hsync_application           = PSI_FEATURE_DISABLE;
    pHdIf->mra_enable_legacy_mode      = PSI_FEATURE_DISABLE;         // Enables MRA legacy mode. 0 - no legacy mode; != 0 - legacy mode. See parameter MRAEnableLegacyMode in edd docu.
    pHdIf->supported_service_protocols = PSI_HD_CLRPC_SUPPORT;        // PSI_HD_RSI_SUPPORT: only RSI is supported. PSI_HD_CLRPC_SUPPORT: only CLRPC is supported. PSI_HD_RSI_CLRPC_SUPPORT: both RSI and CLRPC are supported.
    pHdIf->cluster_IP_support          = (PSI_CFG_OBSD_CFG_CLUSTER_IP_SUPPORTED == 1) ? PSI_FEATURE_ENABLE : PSI_FEATURE_DISABLE;
    pHdIf->tcip_copy_on_send           = (PSI_CFG_TCIP_CFG_COPY_ON_SEND == 1)         ? PSI_FEATURE_ENABLE : PSI_FEATURE_DISABLE;

    for (LSA_UINT8 port_idx = 0; port_idx < PSI_CFG_MAX_PORT_CNT; port_idx++)
    {
        if (port_idx < IOD_CFG_PDEV_NUMOF_PORTS)
        {
            pHdIf->hw_port[port_idx].user_nr           = port_idx + 1;
            pHdIf->hw_port[port_idx].mrp_ringport_type = PSI_RING_PORT_DEFAULT;
        }
        else
        {
            pHdIf->hw_port[port_idx].user_nr           = (LSA_UINT16)EDDP_PORT_NOT_CONNECTED;
            pHdIf->hw_port[port_idx].mrp_ringport_type = PSI_NO_RING_PORT;
        }
    }

    // user to HW/ASIC port mapping (port_map[0] is reserved, port_map[1..x] = user-port-idx)
    for (LSA_UINT8 port_idx = 1; port_idx <= IOD_CFG_PDEV_NUMOF_PORTS; port_idx++)
    {
        pHdIf->port_map[port_idx].asic_port_id = port_idx;
        pHdIf->port_map[port_idx].hw_port_id   = port_idx;
    }

    pHdIf->irte.use_setting    = PSI_USE_SETTING_NO; // for eddi
    pHdIf->stdmac.use_setting  = PSI_USE_SETTING_NO; // for edds
    pHdIf->ti_icss.use_setting = PSI_USE_SETTING_NO; // for eddt

    /*---------------------------------------------------------------------------------*/
    /* PNIO Interface Parameters                                                       */
    /*---------------------------------------------------------------------------------*/
    PSI_HD_IF_INPUT_PTR_TYPE pPnioIf = &(pHdIf->pnio_if);
    pPnioIf->edd_if_id = TSKMA_IM_INTERFACE_ID();
    pPnioIf->trace_idx = 0; // unused

    // IP Parameters
    pPnioIf->ip.use_setting          = PSI_USE_SETTING_YES;
    pPnioIf->ip.nr_of_icmp           = 10;
    pPnioIf->ip.nr_of_tcp            = 10;
    pPnioIf->ip.nr_of_udp            = 98;
    pPnioIf->ip.nr_of_arp            = 20;
    pPnioIf->ip.nr_of_send           = 40;
    pPnioIf->ip.multicast_support_on = PSI_FEATURE_ENABLE;

    // IOD Parameters
    pPnioIf->iod.use_setting         = PSI_USE_SETTING_YES;
    pPnioIf->iod.nr_of_instances     = 1;
    pPnioIf->iod.iod_max_ar_DAC      = IOD_CFG_NUMOF_DEV_ACCESS_AR;
    pPnioIf->iod.iod_max_ar_IOC      = IOD_CFG_NUMOF_IO_AR;
    pPnioIf->iod.max_icr_data_size   = PNDV_MAX_IO_INPUT_LEN;
    pPnioIf->iod.max_ocr_data_size   = PNDV_MAX_IO_OUTPUT_LEN;
    pPnioIf->iod.max_mcr_data_size   = PNDV_MAX_IO_LEN;

    // IOC Parameters - not used
    pPnioIf->ioc.use_setting         = PSI_USE_SETTING_NO;

    // CBA Parameters - not used
    pPnioIf->cba.use_setting         = PSI_USE_SETTING_NO;

    // IP2PN Parameters
    pPnioIf->ip2pn.use_setting       = PSI_USE_SETTING_YES;
    pPnioIf->ip2pn.check_ip_enabled  = PSI_FEATURE_DISABLE;

    // NRT Parameters
    pPnioIf->nrt.RxFilterUDP_Broadcast = PSI_FEATURE_ENABLE;
    pPnioIf->nrt.RxFilterUDP_Unicast   = PSI_FEATURE_ENABLE;

    // MRP Parameters
#if (PSI_CFG_USE_MRP == 1)
    pPnioIf->mrp.use_setting                  = PSI_USE_SETTING_YES;
    pPnioIf->mrp.max_instances                = 1;
    pPnioIf->mrp.supported_role               = EDD_MRP_ROLE_CAP_CLIENT;
    pPnioIf->mrp.default_role_instance0       = EDD_MRP_ROLE_CLIENT;
    pPnioIf->mrp.max_mrp_interconn_instances  = 1;
    pPnioIf->mrp.supported_mrp_interconn_role = 0;
    pPnioIf->mrp.supported_multiple_role      = 0;
    for (LSA_UINT8 i = 0; i < PSI_CFG_MAX_PORT_CNT; i++)
    {
        pPnioIf->mrp.supports_mrp_interconn_port_config[i] = 0;
    }
#else
    pPnioIf->mrp.use_setting                  = PSI_USE_SETTING_NO;
#endif


    /*---------------------------------------------------------------------------------*/
    /* PNIP Parameters                                                                 */
    /*---------------------------------------------------------------------------------*/
    PSI_HD_PNIP_INPUT_PTR_TYPE pPnip = &(pHdIf->pnip);

    pPnip->use_setting                  = PSI_FEATURE_ENABLE;
    pPnip->eddp_hera_error_int_handling = PSI_FEATURE_DISABLE;
    pPnip->buffer_capacity_use_case     = PSI_PNIP_USE_CASE_DEFAULT;
    pPnip->k32fw_trace_level            = 0; // trace level off
    pPnip->ppm_lifetime64_slow_mem      = 0;
    TSKMA_MEMSET(&pPnip->io_param, 0, sizeof(pPnip->io_param)); // not used since hera is not supported

    // FeatureSupport
    TSKMA_MEMSET(&pPnip->FeatureSupport, 0, sizeof(pPnip->FeatureSupport));
    pPnip->FeatureSupport.use_settings                       = PSI_USE_SETTING_YES;
    pPnip->FeatureSupport.irt_forwarding_mode_supported      = EDD_DPB_FEATURE_IRTFWDMODE_SUPPORTED_ABSOLUTE | EDD_DPB_FEATURE_IRTFWDMODE_SUPPORTED_RELATIVE;
#if (IOD_INCLUDE_MRPD == 1)    
    pPnip->FeatureSupport.mrpd_supported                     = PSI_FEATURE_ENABLE;
#endif        
#if (IOD_CFG_PU_FEATURES_SUPPORT == 1)     
    pPnip->FeatureSupport.fragmentationtype_supported        = EDD_DPB_FEATURE_FRAGTYPE_SUPPORTED_DYNAMIC;
    pPnip->FeatureSupport.short_preamble_supported           = PSI_FEATURE_ENABLE;
    pPnip->FeatureSupport.max_dfp_frames                     = 8;
#endif
    pPnip->FeatureSupport.mrp_interconn_originator_supported = PSI_FEATURE_DISABLE;
    pPnip->FeatureSupport.tx_scattering_support              = PSI_FEATURE_DISABLE;
    pPnip->FeatureSupport.tx_cs_offloading_support           = PSI_FEATURE_DISABLE;
    pPnip->FeatureSupport.gigabit_supported                  = PSI_FEATURE_DISABLE;
    pPnip->FeatureSupport.tsn_enable                         = PSI_FEATURE_DISABLE;
}


//*----------------------------------------------------------------------------
//* tskma_pnio_stack_open()
//*
//* Initialize pnio stack over PSI.
//*----------------------------------------------------------------------------
PNIO_VOID tskma_pnio_stack_open(PNIO_VOID)
{
    PSI_UPPER_RQB_PTR_TYPE pRQB;

    PSI_ALLOC_LOCAL_MEM(((PNIO_VOID**)&pRQB), sizeof(*pRQB), 0, LSA_COMP_ID_PSI, 0);
    TSKMA_ASSERT(pRQB != NULL);

    PSI_RQB_SET_OPCODE(pRQB, PSI_OPC_LD_OPEN_DEVICE);
    PSI_RQB_SET_HANDLE(pRQB, 0);
    PSI_RQB_SET_COMP_ID(pRQB, LSA_COMP_ID_PSI);

    tskma_prepare_psi_startup_params(&pRQB->args.ld_open);

    psi_ld_system(pRQB);
}


//*----------------------------------------------------------------------------
//* tskma_task_pno_request()
//*
//* Service PNO requests
//*----------------------------------------------------------------------------
PNIO_VOID tskma_task_pno_request(TSKMA_RQB_S_PTR_T rqb_ptr)
{
    switch (TSKMA_RQB_GET_OPCODE(rqb_ptr))
    {
        case TSKMA_OPC_PNO_REQ_OPEN:
        {
            tskma_pnio_stack_open();
            break;
        }
        default:
        {
            /* opcode not supported */
            TSKMA_FATAL(TSKMA_RQB_GET_OPCODE(rqb_ptr));
        }
    }
}


//*----------------------------------------------------------------------------
//* tskma_task_pno()
//*
//* TSKMA message handler for PNO task
//*----------------------------------------------------------------------------
OS_CODE_FAST PNIO_VOID tskma_task_pno (PNIO_VOID)
{
#if DEBUG_TASKCNT
    LSA_UINT32  taskID;
#endif

    TSKMA_OS_WAIT_ON_TASKSTART();

#if DEBUG_TASKCNT
    taskID = TSKMA_GET_THREAD_ID();
#endif

    TSKMA_TASK_INIT_STATE(TSKMA_TASK_NUM_PNO);

    for (;;)
    {
        union
        {
            TSKMA_MAIL_ELEM_S_PTR_T  rqb_ptr;
            TSKMA_VOID_PTR_TYPE      void_ptr;
        } msg;

        TSKMA_OS_READ_MAIL(&msg.void_ptr, TSKMA_TASK_NUM_PNO);

        while ((msg.void_ptr))
        {
            LSA_RQB_GET_REQ_FCT_PTR(msg.rqb_ptr) (msg.void_ptr);

            TSKMA_OS_READ_MAIL(&msg.void_ptr, TSKMA_TASK_NUM_PNO);

#if DEBUG_TASKCNT
            TaskCnt[taskID]++;
#endif
        }

        TSKMA_WAKE_NEXT_TS_TASK(TSKMA_TASK_NUM_PNO);
    }
}


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
