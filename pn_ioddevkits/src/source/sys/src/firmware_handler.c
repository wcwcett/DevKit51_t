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
/*  F i l e               &F: firmware_handler.c                        :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Firmware handler for PNIODDevkits                                        */
/*                                                                           */
/*****************************************************************************/

#include "psi_inc.h"
#include "firmware_handler.h"
#include "ertec200p_reg.h"
#include "arm926.h"
#include "eddp_k32_image_rev2.h"

#define PSI_MODULE_ID      32006
#define LTRC_ACT_MODUL_ID  32006

/*****************************************************************************/
static LSA_UINT16 firmware_boot_krisc_arm9(LSA_UINT32 programEntryAddress)
{
    // start the krisc core
    volatile LSA_UINT32* pAPB_SCRB_ASYN_RES_CTRL_REG;
    volatile LSA_UINT32* pK32_LIVE_TRACE;

    pAPB_SCRB_ASYN_RES_CTRL_REG = (volatile LSA_UINT32*)(PNIO_VOID*)(U_SCRB__BASE + ERTEC200P_APB_SCRB__ASYN_RES_CTRL_REG_OFFSET);
    pK32_LIVE_TRACE             = (volatile LSA_UINT32*)KRISC_DTCM_BASE;

    // initialize physical addresses before release of reset
    *(pK32_LIVE_TRACE + 12) = U_PNIP__BASE;    //physicalK32PNIPBase
    *(pK32_LIVE_TRACE + 13) = KRISC_DTCM_BASE; //physicalPNIPK32DTCMBase
    // release reset
    *pAPB_SCRB_ASYN_RES_CTRL_REG &= ~0x20ul;
    return PSI_OK;
}

/*****************************************************************************/
static LSA_UINT16 firmware_load_krisc_arm9(LSA_UINT8 const* pCode, LSA_UINT32 code_length, LSA_UINT8 const* pData, LSA_UINT32 data_length)
{
    volatile LSA_UINT32* pPNIP_REG_KRISC_TCM_MAP;
    volatile LSA_UINT32* pAPB_SCRB_ASYN_RES_CTRL_REG;
    volatile LSA_UINT8*  pDTCM = (LSA_UINT8*)KRISC_DTCM_BASE;
    volatile LSA_UINT8*  pITCM;

    LSA_UINT32 tile_size = 16 * 1024;
    LSA_UINT32 tcm_size  = 256 * 1024;
    LSA_UINT32 tcmValue  = ((LSA_UINT32)code_length + (tile_size - 1)) / (tile_size);  /* only DTCM */

    // Set I/DTCM ratio to 0:8 (tcmValue == 0)
    pPNIP_REG_KRISC_TCM_MAP  = (volatile LSA_UINT32*) (PNIO_VOID*)(U_PNIP__BASE + ERTEC200P_PNIP__KRISC_TCM_MAP_REG_OFFSET);
    *pPNIP_REG_KRISC_TCM_MAP = 0x0ul;

    // hold reset
    pAPB_SCRB_ASYN_RES_CTRL_REG = (volatile LSA_UINT32*) (PNIO_VOID*)(U_SCRB__BASE + ERTEC200P_APB_SCRB__ASYN_RES_CTRL_REG_OFFSET);
    *pAPB_SCRB_ASYN_RES_CTRL_REG |= 0x20ul;

    // load the binaries
    pDTCM += 0; /* start of dtcm */
    /* copy data */
    PSI_MEMCPY((PNIO_VOID*)pDTCM, (PNIO_VOID*)pData, data_length);
    /* fill rest of data segment with zero */
    PSI_MEMSET((PNIO_VOID*)pDTCM + data_length, 0, (tile_size - (data_length % tile_size)));

    pDTCM = (LSA_UINT8*)KRISC_DTCM_BASE + (tcm_size - tile_size); /* get ITCM pointer, (start of itcm tile in dtcm ) */
    while (code_length > 0)
    {
        pITCM = pDTCM;      /* restore ITCM pointer */
        if(code_length >= tile_size)
        {
            PSI_MEMCPY((PNIO_VOID*)pITCM, (PNIO_VOID*)pCode, tile_size);
            pCode       += tile_size;
            code_length -= tile_size;
        }
        else
        {
            PSI_MEMCPY((PNIO_VOID*)pITCM, (PNIO_VOID*)pCode, (code_length+3) & 0xFFFFFFFC);
            code_length = 0; /* no more code to copy */
        }
        pDTCM -= tile_size;
    }

    /* Now set I-TCM / D-TCM ratio */
    *((volatile LSA_UINT32*)((U_PNIP__BASE + ERTEC200P_PNIP__KRISC_TCM_MAP_REG_OFFSET))) = (LSA_UINT32)(tcmValue);
    return PSI_OK;
}

/*****************************************************************************/
LSA_VOID firmware_hw_load_coprocessor_pnip_arm9()
{
    if (PSI_OK != firmware_load_krisc_arm9((LSA_UINT8 const*)k32_rev2_code_bin, (LSA_UINT32)k32_rev2_code_bin_len, 
                                           (LSA_UINT8 const*)k32_rev2_data_bin, (LSA_UINT32)k32_rev2_data_bin_len))
    {
        PSI_SYSTEM_TRACE_00(0, LSA_TRACE_LEVEL_FATAL, "firmware_hw_load_coprocessor(): Loading krisc/arm9 failed");
        PSI_FATAL(0);
    }

    if (PSI_OK != firmware_boot_krisc_arm9(0))
    {
        PSI_SYSTEM_TRACE_00(0, LSA_TRACE_LEVEL_FATAL, "firmware_hw_load_coprocessor(): Booting krisc/arm9 failed");
        PSI_FATAL(0);
    }
}

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
