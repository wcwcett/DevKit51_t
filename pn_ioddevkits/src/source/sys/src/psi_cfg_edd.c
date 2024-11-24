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
/*  F i l e               &F: psi_cfg_edd.c                             :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Implementation of PSI EDD HW system adaption                             */
/*                                                                           */
/*****************************************************************************/

#define PSI_MODULE_ID      25408
#define LTRC_ACT_MODUL_ID  25408

#include "psi_inc.h"

#if (((PSI_CFG_USE_EDDI == 1) || (PSI_CFG_USE_EDDP == 1) || (PSI_CFG_USE_EDDS == 1) || (PSI_CFG_USE_EDDT == 1)) && (PSI_CFG_USE_HD_COMP == 1))

/*----------------------------------------------------------------------------*/

LSA_VOID PSI_EDD_GET_MEDIA_TYPE(
    PSI_SYS_HANDLE                 const hSysDev,
    LSA_UINT32                     const HwPortID,
    LSA_UINT8                      const isPofAutoDetection,
    PSI_PORT_MEDIA_TYPE          * const pMediaType,
    PSI_PORT_OPTICAL_TRANS_TYPE  * const pOpticalTransType,
    PSI_PORT_PHY_TYPE            * const pPhyType,
    PSI_PORT_FX_TRANSCEIVER_TYPE * const pFxTransceiverType )
{   
    static PSI_PORT_MEDIA_TYPE          s_MediaType[PSI_CFG_MAX_PORT_CNT+1] = {PSI_MEDIA_TYPE_UNKNOWN};
    static PSI_PORT_OPTICAL_TRANS_TYPE  s_OpticalTransType[PSI_CFG_MAX_PORT_CNT+1] = {PSI_OPTICAL_TRANS_TYPE_ISOTHER};
    static PSI_PORT_FX_TRANSCEIVER_TYPE s_FxTransceiverType[PSI_CFG_MAX_PORT_CNT+1] = {PSI_FX_TRANSCEIVER_TYPE_UNKNOWN};

    LSA_UINT16                   ret_val;
    
    PSI_ASSERT(pMediaType         != 0);
    PSI_ASSERT(pOpticalTransType  != 0);
    PSI_ASSERT(pPhyType           != 0);
    PSI_ASSERT(pFxTransceiverType != 0);

    if ((HwPortID >= 1) && (HwPortID <= PSI_CFG_MAX_PORT_CNT))
    {
        if (isPofAutoDetection == PSI_PORT_MEDIA_TYPE_POF_AUTO_DETECTION_YES)
        {
            // execute POF port auto detection
            psi_pof_port_auto_detection(&ret_val, 1/*hd_nr*/, HwPortID, &s_MediaType[HwPortID], &s_OpticalTransType[HwPortID], &s_FxTransceiverType[HwPortID]);

            if (LSA_RET_OK != ret_val)
            {
                PSI_SYSTEM_TRACE_03(0, LSA_TRACE_LEVEL_FATAL,
                                    "PSI_EDD_GET_MEDIA_TYPE(): invalid ret_val(0x%08x) from psi_pof_port_auto_detection(), hd_nr(%u), HwPortID(%u)",
                                    ret_val, 1/*hd_nr*/, HwPortID);
                PSI_FATAL(0);
            }
        }

        *pMediaType         = s_MediaType[HwPortID];
        *pOpticalTransType  = s_OpticalTransType[HwPortID];
        *pFxTransceiverType = s_FxTransceiverType[HwPortID];

        *pPhyType = PSI_PHY_TYPE_NEC;
    }
    else
    {
        PSI_SYSTEM_TRACE_01(0, LSA_TRACE_LEVEL_FATAL,
                            "PSI_EDD_GET_MEDIA_TYPE(): invalid HwPortID(%u) from psi_pof_port_auto_detection()",
                            HwPortID);
        PSI_FATAL(0);
    }
}

/*----------------------------------------------------------------------------*/

LSA_VOID PSI_EDD_NRT_SEND_HOOK(
    PSI_SYS_HANDLE    hSysDev,
    LSA_VOID_PTR_TYPE ptr_upper_rqb)
{
    // Send hook not used
    LSA_UNUSED_ARG(hSysDev);
    LSA_UNUSED_ARG(ptr_upper_rqb);
}

/*----------------------------------------------------------------------------*/

LSA_BOOL PSI_EDD_IS_PORT_PULLED(
    PSI_SYS_HANDLE hSysDev,
    LSA_UINT32     HwPortID)
{
    LSA_UNUSED_ARG(hSysDev);
    LSA_UNUSED_ARG(HwPortID);
    return LSA_FALSE;
}

/*----------------------------------------------------------------------------*/

/**
 * The implementation of this Out-Macro is used for all EDD types to get the poll_eoi_interval (us).
 * This macro may be called after PSI_HD_ENABLE_EVENT.
 *
 * @param [in]  hSysDev         - PSI handle, used to get the board
 *
 * @return                      - value of poll_eoi_interval in us (0, 250, 500, 1000)
 */
LSA_UINT32 PSI_EDD_GET_POLL_EOI_INTERVAL(
    PSI_SYS_HANDLE  const hSysDev )
{
    LSA_UINT32 poll_eoi_interval_us;

#if (PSI_CFG_USE_EDDP == 1)
#if defined (PSI_CFG_EDDP_CFG_ISR_POLLING_MODE)
    poll_eoi_interval_us = 1000;
#else
    // poll_eoi_interval must be given in steps of 1024ns - convert it into us value
    poll_eoi_interval_us = ((PSI_EDDP_EOI_INACTIVE_TIME * 1024/*ns*/) / 1000/*us*/);
#endif
#else
    PSI_SYSTEM_TRACE_01( 0, LSA_TRACE_LEVEL_FATAL, "<< PSI_EDD_GET_POLL_EOI_INTERVAL(): poll_eoi_interval_us(%u)", poll_eoi_interval_us);
    PSI_FATAL(0);
#endif

    LSA_UNUSED_ARG(hSysDev);

    return poll_eoi_interval_us;
}

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
