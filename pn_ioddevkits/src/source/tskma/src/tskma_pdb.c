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
/*  F i l e               &F: tskma_pdb.c                               :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*   TSKMA open parameter for pndv                                           */
/*                                                                           */
/*****************************************************************************/

#include "tskma_inc.h"

#include "psi_sys.h"

#include "usrapp_cfg.h"

#include "pnpb_nvdata.h" 

#define TSKMA_PDB_EXT_ATTR

#define TSKMA_MODULE_ID TSKMA_MODULE_ID_TSKMA_PDB

#include "tskma_pdb.h"
#include "usriod_cfg.h"

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P )
#include "ertec200p_reg.h"
#endif

#include "version_dk.h"

#define OEM_DEVKIT_VENDOR_ID                    IOD_CFG_VENDOR_ID
#define OEM_DEVKIT_DEVICE_ID                    IOD_CFG_DEVICE_ID
#define OEM_DEVKIT_SNMP_INTERFACE_DESCRIPTION   IOD_CFG_SNMP_INTERFACE_DESCRIPTION
#define OEM_DEVKIT_DEV_ANNOTATION_ORDER_ID      IOD_CFG_DEV_ANNOTATION_ORDER_ID
#define OEM_DEVKIT_IM0_SERIAL_NUMBER            IOD_CFG_IM0_SERIAL_NUMBER
#define OEM_DEVKIT_HW_REVISION_ERTEC200P2       2
#define OEM_DEVKIT_HW_REVISION_ERTEC200P        1
#define OEM_DEVKIT_VERSION_PREFIX               'V'
#define OEM_DEVKIT_VERSION_HH                   5
#define OEM_DEVKIT_VERSION_H                    0
#define OEM_DEVKIT_VERSION_L                    0

CM_PD_OEM_DATA_TYPE oem_data =
{
    OEM_DEVKIT_VENDOR_ID,                       /* vendor_id */
    OEM_DEVKIT_DEVICE_ID,                       /* device_id */
    OEM_DEVKIT_SNMP_INTERFACE_DESCRIPTION "                                   " ,      /* ERTEC 200P based PNIO device */
    OEM_DEVKIT_DEV_ANNOTATION_ORDER_ID "                                            ", /* 6ES7195-3BE00-0YA1 */
    OEM_DEVKIT_IM0_SERIAL_NUMBER,               /*IM_Serial_Number[16];*/
    OEM_DEVKIT_HW_REVISION_ERTEC200P,           /* IM hardware revision */
    OEM_DEVKIT_VERSION_PREFIX,                  /* SW revision prefix Visibile string */
    OEM_DEVKIT_VERSION_HH,                      /* SW revision Functional Enhancement */
    OEM_DEVKIT_VERSION_H,                       /* Bug Fix */
    OEM_DEVKIT_VERSION_L                        /* SW revision internal change */
};


PNIO_VOID tskma_pdb_setup_pndv()
{
    LSA_UINT32 tmp_count_0;
    LSA_UINT32 i, subsl_cnt;

    /* pndv parameter -------------------------------------------------------*/
    tskma_pdb_pndv_open_parameter.cm.sys_path_cm[PNDV_INDEX_PATH_CMPDSV_ACP] = PSI_PATH_IF_APP_CMPD_ACP;
    tskma_pdb_pndv_open_parameter.cm.sys_path_cm[PNDV_INDEX_PATH_IOD_CM_ACP] = PSI_PATH_IF_APP_IOD_CMSV_ACP;
    tskma_pdb_pndv_open_parameter.cm.annotation_ptr                          = TSKMA_IM_GET_ANNOTATION_STR_PTR();
    tskma_pdb_pndv_open_parameter.cm.device_id                               = TSKMA_IM_DEVICE_ID();
 
    PNDV_SYSPATH_SET_PATH(tskma_pdb_pndv_open_parameter.ip2pn.sys_path_ip2pn[PNDV_INDEX_PATH_IP2PN_GLO_APP], PSI_PATH_GLO_APP_IP2PN);
    PNDV_SYSPATH_SET_PATH(tskma_pdb_pndv_open_parameter.ip2pn.sys_path_ip2pn[PNDV_INDEX_PATH_IP2PN_IF_CM],   PSI_PATH_IF_APP_IP2PN_CM);

#if ( IOD_INCLUDE_IM5 )
    /*Set HW revision*/
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P )
    if (0x0100 == (REG32(U_SCRB__ID_REG) & 0x0000FF00))
    {
        oem_data.IM_Hardware_Revision = OEM_DEVKIT_HW_REVISION_ERTEC200P;
    }
    else
    {
        oem_data.IM_Hardware_Revision = OEM_DEVKIT_HW_REVISION_ERTEC200P2;
    }
#else
    oem_data.IM_Hardware_Revision = OEM_DEVKIT_HW_REVISION_ERTEC200P;
#endif
    tskma_pdb_pndv_open_parameter.cm.oem_data_ptr        = (LSA_UINT32 *) &oem_data;
#else
    tskma_pdb_pndv_open_parameter.cm.oem_data_ptr        = LSA_NULL;
#endif

    for (tmp_count_0 = 0; tmp_count_0 < PNDV_RECORD_MAX_RES; tmp_count_0++)
    {
        tskma_pdb_pndv_open_parameter.sys.rqb_ds_res_ptr[tmp_count_0] = (PNDV_RQB_DS_PTR)&glob_coupling_interface.pndv.ds_rw.pndv_rqb_ds_res[tmp_count_0];
    }

    tskma_pdb_pndv_open_parameter.rema_station.prm_ptr    = 0;


    tskma_pdb_pndv_open_parameter.pd.interface_subslot_nr = TSKMA_IM_SUBMODULE_NR_INTERFACE();
    tskma_pdb_pndv_open_parameter.pd.port_count_used      = TSKMA_IM_SUBMODULE_NR_PORTS_USED;
    tskma_pdb_pndv_open_parameter.pd.im_mod_ident         = TSKMA_IM_MODULE_ID();

    subsl_cnt = TSKMA_IM_SUBMODULE_NUM_P();
    for (i = 0; i < subsl_cnt; i++)
    {
#if( IOD_INCLUDE_IM5 )
        tskma_pdb_pndv_open_parameter.pd.port[i].im_0_bits = CM_SV_IM0_BITS_IM5 | CM_SV_IM0_BITS_SUBMODULE;
#else
        tskma_pdb_pndv_open_parameter.pd.port[i].im_0_bits = CM_SV_IM0_BITS_SUBMODULE;
#endif
        tskma_pdb_pndv_open_parameter.pd.port[i].submod_id = TSKMA_IM_SUBMODULE_ID_P(i);
    }
}

PNIO_VOID tskma_pdb_open()
{
    tskma_pdb_setup_pndv();
}

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
