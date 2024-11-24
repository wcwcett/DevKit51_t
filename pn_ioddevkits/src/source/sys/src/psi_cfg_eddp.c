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
/*  F i l e               &F: psi_cfg_eddp.c                            :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Implementation of PSI EDDP HW system adaption                            */
/*                                                                           */
/*****************************************************************************/

#define PSI_MODULE_ID      32003
#define LTRC_ACT_MODUL_ID  32003

#include "psi_inc.h"
#include "eddp_int.h"
#include "eddp_phy.h"

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P) /* for EDDP use this */
#include "arm926.h"
#include "ertec200p_reg.h"
#endif


#if ((PSI_CFG_USE_EDDP == 1) && (PSI_CFG_USE_HD_COMP == 1))
/*----------------------------------------------------------------------------*/

PSI_FILE_SYSTEM_EXTENSION( PSI_MODULE_ID )

/*---------------------------------------------------------------------------*/

#ifndef PSI_EDDP_ALLOC_UPPER_RX_MEM
LSA_VOID PSI_EDDP_ALLOC_UPPER_RX_MEM(
    PSI_SYS_HANDLE      hSysDev,
    LSA_VOID_PTR_TYPE * upper_mem_ptr_ptr,
    LSA_UINT32          length)
{
    LSA_UNUSED_ARG(hSysDev);

    PSI_NRT_ALLOC_RX_MEM(upper_mem_ptr_ptr, length, 0, LSA_COMP_ID_EDDP);
    
    PSI_SYSTEM_TRACE_02(0, LSA_TRACE_LEVEL_CHAT,
                        "PSI_EDDP_ALLOC_UPPER_RX_MEM(): ptr(0x%08x) size(%u)", 
                        *upper_mem_ptr_ptr, length);
}
#else
#error "by design a function!"
#endif

/*---------------------------------------------------------------------------*/

#ifndef PSI_EDDP_FREE_UPPER_RX_MEM
LSA_VOID PSI_EDDP_FREE_UPPER_RX_MEM(
    PSI_SYS_HANDLE      hSysDev,
    LSA_UINT16          *ret_val_ptr,
    LSA_VOID_PTR_TYPE   upper_mem_ptr)
{
    LSA_UNUSED_ARG(hSysDev);

    PSI_SYSTEM_TRACE_01(0, LSA_TRACE_LEVEL_CHAT, 
                        "PSI_EDDP_FREE_UPPER_RX_MEM(): ptr(0x%08x)", 
                        upper_mem_ptr);

    PSI_NRT_FREE_RX_MEM(ret_val_ptr, upper_mem_ptr, 0, LSA_COMP_ID_EDDP);
}
#else
#error "by design a function!"
#endif

/*---------------------------------------------------------------------------*/

#ifndef PSI_EDDP_ALLOC_UPPER_TX_MEM
LSA_VOID PSI_EDDP_ALLOC_UPPER_TX_MEM(
    PSI_SYS_HANDLE      hSysDev,
    LSA_VOID_PTR_TYPE * upper_mem_ptr_ptr,
    LSA_UINT32          length)
{
    LSA_UNUSED_ARG(hSysDev);

    PSI_NRT_ALLOC_TX_MEM(upper_mem_ptr_ptr, length, 0, LSA_COMP_ID_EDDP);

    PSI_SYSTEM_TRACE_02(0, LSA_TRACE_LEVEL_CHAT,
                        "PSI_EDDP_ALLOC_UPPER_TX_MEM(): ptr(0x%08x) size(%u)", 
                        *upper_mem_ptr_ptr, length); 
}
#else
#error "by design a function!"
#endif

/*---------------------------------------------------------------------------*/

#ifndef PSI_EDDP_FREE_UPPER_TX_MEM
LSA_VOID PSI_EDDP_FREE_UPPER_TX_MEM(
    PSI_SYS_HANDLE      hSysDev,
    LSA_UINT16        * ret_val_ptr,
    LSA_VOID_PTR_TYPE   upper_mem_ptr)
{
    LSA_UNUSED_ARG(hSysDev);

    PSI_SYSTEM_TRACE_01(0, LSA_TRACE_LEVEL_CHAT, 
                        "PSI_EDDP_FREE_UPPER_TX_MEM(): ptr(0x%08x)", 
                        upper_mem_ptr);

    PSI_NRT_FREE_TX_MEM(ret_val_ptr, upper_mem_ptr, 0, LSA_COMP_ID_EDDP);
}
#else
#error "by design a function!"
#endif

/*---------------------------------------------------------------------------*/

#ifndef PSI_EDDP_ALLOC_UPPER_DEV_MEM
LSA_VOID PSI_EDDP_ALLOC_UPPER_DEV_MEM(
    PSI_SYS_HANDLE      hSysDev,
    LSA_VOID_PTR_TYPE * upper_mem_ptr_ptr,
    LSA_UINT32          length)
{
    LSA_UNUSED_ARG(hSysDev);

    PSI_DEV_ALLOC_MEM(upper_mem_ptr_ptr, length, 0, LSA_COMP_ID_EDDP);

    PSI_SYSTEM_TRACE_02(0, LSA_TRACE_LEVEL_CHAT,
                        "PSI_EDDP_ALLOC_UPPER_DEV_MEM(): ptr(0x%08x) size(%u)", 
                        *upper_mem_ptr_ptr, length);
}
#else
#error "by design a function!"
#endif

/*---------------------------------------------------------------------------*/

#ifndef PSI_EDDP_FREE_UPPER_DEV_MEM
LSA_VOID PSI_EDDP_FREE_UPPER_DEV_MEM(
    PSI_SYS_HANDLE      hSysDev,
    LSA_UINT16        * ret_val_ptr,
    LSA_VOID_PTR_TYPE   upper_mem_ptr)
{
    LSA_UNUSED_ARG(hSysDev);

    PSI_SYSTEM_TRACE_01(0, LSA_TRACE_LEVEL_CHAT, 
                        "PSI_EDDP_FREE_UPPER_DEV_MEM(): ptr(0x%08x)", 
                        upper_mem_ptr);

    PSI_DEV_FREE_MEM(ret_val_ptr, upper_mem_ptr, 0, LSA_COMP_ID_EDDP);
}
#else
#error "by design a function!"
#endif

/*---------------------------------------------------------------------------*/

#ifndef PSI_EDDP_RX_MEM_ADDR_UPPER_TO_PNIP
LSA_VOID PSI_EDDP_RX_MEM_ADDR_UPPER_TO_PNIP(
    PSI_SYS_HANDLE      hSysDev,
    LSA_VOID_PTR_TYPE   Ptr,
    LSA_UINT32        * pAddr)
{
    *pAddr = (LSA_UINT32)Ptr;

    LSA_UNUSED_ARG(hSysDev);
}
#else
#error "by design a function!"
#endif

/*---------------------------------------------------------------------------*/

#ifndef PSI_EDDP_TX_MEM_ADDR_UPPER_TO_PNIP
LSA_VOID PSI_EDDP_TX_MEM_ADDR_UPPER_TO_PNIP(
    PSI_SYS_HANDLE      hSysDev,
    LSA_VOID_PTR_TYPE   Ptr,
    LSA_UINT32        * pAddr)
{
    *pAddr = (LSA_UINT32)Ptr;

    LSA_UNUSED_ARG(hSysDev);
}
#else
#error "by design a function!"
#endif

/*---------------------------------------------------------------------------*/

#ifndef PSI_EDDP_DEV_MEM_ADDR_UPPER_TO_PNIP
LSA_VOID PSI_EDDP_DEV_MEM_ADDR_UPPER_TO_PNIP(
    PSI_SYS_HANDLE      hSysDev,
    LSA_VOID_PTR_TYPE   Ptr,
    LSA_UINT32        * pAddr)
{
    *pAddr = (LSA_UINT32)Ptr;

    LSA_UNUSED_ARG(hSysDev);
}
#else
#error "by design a function!"
#endif

/*---------------------------------------------------------------------------*/

#ifndef PSI_EDDP_IO_EXTRAM_ADDR_UPPER_TO_PNIP
LSA_VOID PSI_EDDP_IO_EXTRAM_ADDR_UPPER_TO_PNIP(
    PSI_SYS_HANDLE      hSysDev,
    LSA_VOID_PTR_TYPE   Ptr,
    LSA_UINT32        * pAddr)
{
    *pAddr = (LSA_UINT32)Ptr;

    LSA_UNUSED_ARG(hSysDev);
}
#else
#error "by design a function!"
#endif

/*---------------------------------------------------------------------------*/

#ifndef PSI_EDDP_IO_PERIF_ADDR_UPPER_TO_PNIP
LSA_VOID PSI_EDDP_IO_PERIF_ADDR_UPPER_TO_PNIP(
    PSI_SYS_HANDLE      hSysDev,
    LSA_VOID_PTR_TYPE   Ptr,
    LSA_UINT32        * pAddr)
{
    *pAddr = (LSA_UINT32)Ptr;

    LSA_UNUSED_ARG(hSysDev);
}
#else
#error "by design a function!"
#endif

/*---------------------------------------------------------------------------*/

#ifndef PSI_EDDP_K32_RESET
LSA_VOID PSI_EDDP_K32_RESET(
    PSI_SYS_HANDLE hSysDev,
    LSA_BOOL       on)
{
    if (LSA_FALSE == on)
    {
        // release asynchronous reset for KRISC32 core
        clear_bit__(REG32(U_SCRB__ASYN_RES_CTRL_REG), U_SCRB__ASYN_RES_CTRL_REG__RES_SOFT_KRISC32_CORE);
    }
    else
    {
        // set asynchronous reset for KRISC32 core
        set_bit__(REG32(U_SCRB__ASYN_RES_CTRL_REG), U_SCRB__ASYN_RES_CTRL_REG__RES_SOFT_KRISC32_CORE);
    }
}
#else
#error "by design a function!"
#endif

/*---------------------------------------------------------------------------*/

#ifndef PSI_EDDP_HERA_GET_PHYADDRESSOFFSET
LSA_UINT32 PSI_EDDP_HERA_GET_PHYADDRESSOFFSET(
    PSI_SYS_HANDLE const        hSysDev,
    LSA_UINT32                  nr_of_ports,
    PSI_HD_PORT_MAP_PTR_TYPE    port_map_ptr)
{
    LSA_UNUSED_ARG(hSysDev);
    LSA_UNUSED_ARG(nr_of_ports);
    LSA_UNUSED_ARG(port_map_ptr);
    return LSA_FALSE; // for warnings
}
#else
#error "by design a function!"
#endif

#ifdef PSI_CFG_EDDP_CFG_PHY_PNPHY_SUPPORT
#ifndef PSI_CFG_EDDP_CFG_PHY_PNPHY_LED_BLINK_INTERNAL
LSA_VOID PSI_EDDP_PHY_PNPHY_LED_BlinkBegin(
    PSI_EDD_HDDB        hDDB,
    PSI_SYS_HANDLE      hSysDev,
    LSA_UINT32          HwPortID)
{
    LSA_UNUSED_ARG(hDDB);
    LSA_UNUSED_ARG(hSysDev);

    PSI_SYSTEM_TRACE_00(EDDP_UNDEF_TRACE_IDX, LSA_TRACE_LEVEL_CHAT,"PSI_EDDP_PHY_PNPHY_LED_BlinkBegin() is called");

    PNIO_cbf_start_led_blink(PNIO_SINGLE_DEVICE_HNDL, (PNIO_UINT32)HwPortID, 1);
}

/*---------------------------------------------------------------------------*/
LSA_VOID PSI_EDDP_PHY_PNPHY_LED_BlinkSetMode(
    PSI_EDD_HDDB        hDDB,
    PSI_SYS_HANDLE      hSysDev,
    LSA_UINT32          HwPortID,
    LSA_UINT32          LEDMode )
{
    LSA_UNUSED_ARG(hDDB);
    LSA_UNUSED_ARG(hSysDev);
    LSA_UNUSED_ARG(HwPortID);

    switch (LEDMode)
    {
        case EDDP_LED_MODE_OFF:
        {
            PSI_SYSTEM_TRACE_00(EDDP_UNDEF_TRACE_IDX, LSA_TRACE_LEVEL_CHAT,"PSI_EDDP_PHY_PNPHY_LED_BlinkSetMode() | LED OFF");
            break;
        }
        case EDDP_LED_MODE_ON:
        {
            PSI_SYSTEM_TRACE_00(EDDP_UNDEF_TRACE_IDX, LSA_TRACE_LEVEL_CHAT,"PSI_EDDP_PHY_PNPHY_LED_BlinkSetMode() | LED ON");
            break;
        }
        default:
        {
            PSI_SYSTEM_TRACE_00(EDDP_UNDEF_TRACE_IDX, LSA_TRACE_LEVEL_ERROR,"PSI_EDDP_PHY_PNPHY_LED_BlinkSetMode() | ERROR -> Wrong LED Mode!");
            break;
        }
    }
}

/*---------------------------------------------------------------------------*/
LSA_VOID PSI_EDDP_PHY_PNPHY_LED_BlinkEnd(
    PSI_EDD_HDDB        hDDB,
    PSI_SYS_HANDLE      hSysDev,
    LSA_UINT32          HwPortID )
{
    LSA_UNUSED_ARG(hDDB);
    LSA_UNUSED_ARG(hSysDev);

    PSI_SYSTEM_TRACE_00(EDDP_UNDEF_TRACE_IDX, LSA_TRACE_LEVEL_CHAT,"PSI_EDDP_PHY_PNPHY_LED_BlinkEnd() is called");

    PNIO_cbf_stop_led_blink(PNIO_SINGLE_DEVICE_HNDL, (PNIO_UINT32)HwPortID);
}

#else
#error "by design a function!"
#endif
#endif
/*===========================================================================*/
/*          NEC PHY Led Blink System Adaptation                              */
/*===========================================================================*/
#ifdef PSI_CFG_EDDP_CFG_PHY_NEC_SUPPORT
#ifndef PSI_CFG_EDDP_CFG_PHY_NEC_LED_BLINK_INTERNAL
LSA_VOID PSI_EDDP_PHY_NEC_LED_BlinkBegin(
    PSI_EDD_HDDB         hDDB,
    PSI_SYS_HANDLE       hSysDev,
    LSA_UINT32           HwPortID)
{
    LSA_UNUSED_ARG(hDDB);
    LSA_UNUSED_ARG(hSysDev);

    PSI_SYSTEM_TRACE_00(EDDP_UNDEF_TRACE_IDX, LSA_TRACE_LEVEL_CHAT,"PSI_EDDP_PHY_NEC_LED_BlinkBegin() is called");

    PNIO_cbf_start_led_blink(PNIO_SINGLE_DEVICE_HNDL, (PNIO_UINT32)HwPortID, 1);
}

/*---------------------------------------------------------------------------*/

LSA_VOID PSI_EDDP_PHY_NEC_LED_BlinkSetMode(
    PSI_EDD_HDDB        hDDB,
    PSI_SYS_HANDLE      hSysDev,
    LSA_UINT32          HwPortID,
    LSA_UINT32          LEDMode)
{
    LSA_UNUSED_ARG(hDDB);
    LSA_UNUSED_ARG(hSysDev);
    LSA_UNUSED_ARG(HwPortID);

    switch (LEDMode)
    {
        case EDDP_LED_MODE_OFF:
        {
            PSI_SYSTEM_TRACE_00(EDDP_UNDEF_TRACE_IDX, LSA_TRACE_LEVEL_CHAT,"PSI_EDDP_PHY_NEC_LED_BlinkSetMode() | LED OFF");
            break;
        }
        case EDDP_LED_MODE_ON:
        {
            PSI_SYSTEM_TRACE_00(EDDP_UNDEF_TRACE_IDX, LSA_TRACE_LEVEL_CHAT,"PSI_EDDP_PHY_NEC_LED_BlinkSetMode() | LED ON");
            break;
        }
        default:
        {
            PSI_SYSTEM_TRACE_00(EDDP_UNDEF_TRACE_IDX, LSA_TRACE_LEVEL_ERROR,"PSI_EDDP_PHY_NEC_LED_BlinkSetMode() | ERROR -> Wrong LED Mode!");
            break;
        }
    }
}

/*---------------------------------------------------------------------------*/

LSA_VOID PSI_EDDP_PHY_NEC_LED_BlinkEnd(
    PSI_EDD_HDDB        hDDB,
    PSI_SYS_HANDLE      hSysDev,
    LSA_UINT32          HwPortID)
{
    LSA_UNUSED_ARG(hDDB);
    LSA_UNUSED_ARG(hSysDev);

    PSI_SYSTEM_TRACE_00(EDDP_UNDEF_TRACE_IDX, LSA_TRACE_LEVEL_CHAT,"PSI_EDDP_PHY_NEC_LED_BlinkEnd() is called");

    PNIO_cbf_stop_led_blink(PNIO_SINGLE_DEVICE_HNDL, (PNIO_UINT32)HwPortID);
}
#endif // PSI_CFG_EDDP_CFG_PHY_NEC_LED_BLINK_INTERNAL
#endif // PSI_CFG_EDDP_CFG_PHY_NEC_SUPPORT

/*===========================================================================*/


#ifndef PSI_EDDP_SIGNAL_SENDCLOCK_CHANGE
LSA_VOID PSI_EDDP_SIGNAL_SENDCLOCK_CHANGE(
    PSI_SYS_HANDLE      hSysDev,
    LSA_UINT32          CycleBaseFactor,
    LSA_UINT8           Mode)
{
    LSA_UNUSED_ARG(hSysDev);
    LSA_UNUSED_ARG(CycleBaseFactor);
    LSA_UNUSED_ARG(Mode);
}
#else
#error "by design a function!"
#endif

/*----------------------------------------------------------------------------*/

#ifndef PSI_EDDP_I2C_SELECT
LSA_VOID PSI_EDDP_I2C_SELECT(
    LSA_UINT8      * const ret_val_ptr,
    PSI_SYS_HANDLE   const hSysDev,
    LSA_UINT16       const PortId,
    LSA_UINT16       const I2CMuxSelect )
{
    LSA_UNUSED_ARG(ret_val_ptr);
    LSA_UNUSED_ARG(hSysDev);
    LSA_UNUSED_ARG(PortId);
    LSA_UNUSED_ARG(I2CMuxSelect);
}
#else
#error "by design a function!"
#endif

#endif // ((PSI_CFG_USE_EDDP == 1) && (PSI_CFG_USE_HD_COMP == 1))

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
