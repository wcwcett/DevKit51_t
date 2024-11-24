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
/*  F i l e               &F: psi_cfg_plf_eb200p.h                      :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Template for platform specific system integration of LSA-component PSI   */
/*  Note: rename the file with your product name (i.E.: psi_cfg_pciox.h)     */
/*                                                                           */
/*  This file has to be overwritten during system integration, because       */
/*  some definitions depend on the different system, compiler or             */
/*  operating system.                                                        */ 
/*                                                                           */
/*****************************************************************************/

/*
 * included by "psi_cfg.h"
 */

#ifndef PSI_CFG_PLATFORM_H      /* ----- reinclude-protection ----- */
#define PSI_CFG_PLATFORM_H

#ifdef __cplusplus              /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

#include "iod_cfg.h"

/*===========================================================================*/
/*        LSA compiler-switches                                              */
/*===========================================================================*/

#ifdef _DEBUG
#define PSI_DEBUG     1
#else
#define PSI_DEBUG     0
#endif

/*===========================================================================*/
/* Select the TOOL Chain for adaption LSA Types to Toolchain (see lsa_cfg.h) */
/*  #define TOOL_CHAIN_MICROSOFT   MS-C/C++ Compiler                         */
/*  #define TOOL_CHAIN_TASKING_TRICORE                                       */
/*  #define TOOL_CHAIN_GNU_PPC                                               */
/*  #define TOOL_CHAIN_CC386                                                 */
/*  #define TOOL_CHAIN_GREENHILLS_ARM                                        */ 
/*  #define TOOL_CHAIN_NRK  ... ARM/Thumb C/C++ Compiler                     */
/*===========================================================================*/
#ifndef TOOL_CHAIN_GNU
#define TOOL_CHAIN_GNU
#endif

/*============================================================================*/
/*       Endianess:                                                           */
/* If you have a computer system whitch stors most significant byte at the    */
/* lowest address of the word/doubleword: Define this                         */
/* #define LSA_HOST_ENDIANESS_BIG                                             */
/* else define this                                                           */
/* #define LSA_HOST_ENDIANESS_LITTLE                                          */
/*============================================================================*/
#define LSA_HOST_ENDIANESS_LITTLE
#define PSI_CFG_USE_EXTERNAL_MSGQ   0
#define PSI_CFG_LOCK_MSGBOX_WITH_IR_LOCK

/*---------------------------------------------------------------------------*/
/* Maximum number of PNIO interfaces (HDs) of all EDDx integrated            */
/* in a system: 1x PNIO interface = 1x HD = 1x EDD                           */
/*---------------------------------------------------------------------------*/

#define PSI_CFG_MAX_IF_CNT      1

/*---------------------------------------------------------------------------*/
/* Maximum number of ports supported by EDD API services                     */
/*---------------------------------------------------------------------------*/

#define PSI_CFG_MAX_PORT_CNT    4

/*---------------------------------------------------------------------------*/
/* PNIO Features                                                             */
/*---------------------------------------------------------------------------*/

#define PSI_CFG_USE_IOC                 0
#define PSI_CFG_USE_IOM                 0
#define PSI_CFG_USE_IOD                 1

/*---------------------------------------------------------------------------*/
/* Common defines                                                            */
/*---------------------------------------------------------------------------*/

#define PSI_CFG_MAX_SOCK_APP_CHANNELS  2 /* Maximum number of user application sock channels, Values (1..8) */
#define PSI_CFG_MAX_DIAG_TOOLS         6 /* Maximum number of diagnosis tools used in the system */

/*---------------------------------------------------------------------------*/
/* PNIO component usage (compiling) keys                                     */
/*---------------------------------------------------------------------------*/
// Note: 0 is off, 1 used, 2 included i.E. on an distributed system 
//       but not compiled on this system
/* include mandatory components for HD */
#define PSI_CFG_USE_ACP         1
#define PSI_CFG_USE_CM          1
#define PSI_CFG_USE_CM_TSN      0
#define PSI_CFG_USE_CM_NM       0
#define PSI_CFG_USE_LLDP        1
/* RSI enable/disable */
#define PSI_CFG_USE_RSI         0

/* CIM enable/disable */
#define PSI_CFG_USE_CIM         0
#define PSI_CFG_USE_DCP_SERVER  1

/* include mandatory components for LD */
#define PSI_CFG_USE_CLRPC       1
#define PSI_CFG_USE_IP2PN       1
#define PSI_CFG_USE_NME         0
#define PSI_CFG_USE_SOCK        1
#define PSI_CFG_USE_SNMPX       1

/* include optional components for HD */
#define PSI_CFG_USE_GSY         1
#define PSI_CFG_USE_IOH         0
#define PSI_CFG_USE_MRP         1
#define PSI_CFG_USE_POF         1

/* include optional components for LD */
#define PSI_CFG_USE_DNS         0 /*/1 < Set by variant, active for OBSD Variants. */
#define PSI_CFG_USE_TCIP        1

/* Set configuration for HIF */
#define PSI_CFG_USE_HIF         0
#define PSI_CFG_USE_HIF_LD      0
#define PSI_CFG_USE_HIF_HD      0
#define PSI_CFG_USE_LD_COMP     1
#define PSI_CFG_USE_HD_COMP     1

/* Set configuration for system adaptation modules */
#define PSI_CFG_USE_PNTRC       0 
#define PSI_CFG_USE_EPS_RQBS    0 

/*---------------------------------------------------------------------------*/
/* Activate the using of PSI CACHE Macros for NRT memory                     */
/* 0: disable / 1: enable                                                    */
/*---------------------------------------------------------------------------*/
#define PSI_CFG_USE_NRT_CACHE_SYNC  0

/*---------------------------------------------------------------------------*/
/* Feature setting for CM-CL                                                 */
/*---------------------------------------------------------------------------*/

#define PSI_CFG_MAX_CL_DEVICES      0     // Max devices for create CM-CL client - no client functionality
#define PSI_CFG_MAX_CL_OSU_DEVICES  0      // range 1..32 (see EDD_DCP_MAX_DCP_HELLO_FILTER(=32)) - no client functionality

/*---------------------------------------------------------------------------*/
/* Feature setting for CM-MC                                                 */
/*---------------------------------------------------------------------------*/

#define PSI_CFG_MAX_MC_DEVICES      0      // Max devices for create CM-MC client - no client functionality

/*---------------------------------------------------------------------------*/
/* Feature setting for CM-SV                                                 */
/*---------------------------------------------------------------------------*/
#define PSI_CFG_CM_CFG_MAX_SV_ARS    (IOD_CFG_NUMOF_IO_AR + IOD_CFG_NUMOF_DEV_ACCESS_AR)  // Fixed to max value as this doesn't cost much memory */ /*AP01511178 This constant will be replaced by a runtime parameter in future
#define PSI_CFG_MAX_SV_DEVICES       1      // Max devices for create CM-SV sever

#define PSI_MAX_SV_ARSETS_PER_DEVICE        1

/*---------------------------------------------------------------------------*/
/* Feature setting for EDD                                                   */
/*---------------------------------------------------------------------------*/
#define PSI_CFG_EDD_CFG_MAX_PORT_CNT                    (PSI_CFG_MAX_PORT_CNT)
#define PSI_CFG_EDD_CFG_MAX_INTERFACE_CNT               (PSI_CFG_MAX_IF_CNT)
#define PSI_CFG_EDD_CFG_MAX_PORT_CNT_ALL_INTERFACE      ((PSI_CFG_MAX_PORT_CNT) * (PSI_CFG_MAX_IF_CNT))
#define PSI_CFG_EDD_CFG_MAX_MRP_INSTANCE_CNT            (((IOD_CFG_PDEV_NUMOF_PORTS-1)/2)+1)

// select one of more EDD types, but at least one. Sample: All
#define PSI_CFG_USE_EDDI    0
#define PSI_CFG_USE_EDDP    1
#define PSI_CFG_USE_EDDS    0

/*---------------------------------------------------------------------------*/
/* Feature setting for EDDP                                                  */
/*---------------------------------------------------------------------------*/
#if (PSI_CFG_USE_EDDP == 1)
#define PSI_CFG_MAX_EDDP_DEVICES        1   // Maximum number of devices that the EDDP supports
#define PSI_CFG_EDDP_CFG_HW_ERTEC200P_SUPPORT

#ifndef BOARD_TYPE_STEP_3
    #define PSI_CFG_EDDP_CFG_PHY_NEC_SUPPORT
    #define PSI_CFG_EDDP_CFG_PHY_NEC_MAU_TYPE_INTERNAL
#else
    #define PSI_CFG_EDDP_CFG_PHY_PNPHY_MEDIA_TYPE_INTERNAL
    #define PSI_CFG_EDDP_CFG_PHY_PNPHY_MAU_TYPE_INTERNAL
    #define PSI_CFG_EDDP_CFG_PHY_PNPHY_SUPPORT
#endif
#endif

/*---------------------------------------------------------------------------*/
/* Feature setting for HIF                                                   */
/*---------------------------------------------------------------------------*/
// Sample: Don't use HIF, if HIF is used, see EPS sample
#define PSI_CFG_HIF_CFG_COMPILE_SERIALIZATION       0
#define PSI_CFG_HIF_CFG_MAX_LD_INSTANCES            0
#define PSI_CFG_HIF_CFG_MAX_HD_INSTANCES            0  

/*---------------------------------------------------------------------------*/
/* Feature setting for the IP Stack                                          */
/*---------------------------------------------------------------------------*/
// select exactly one. Sample: OBSD
#define PSI_CFG_TCIP_STACK_OPEN_BSD       1
#define PSI_CFG_TCIP_STACK_WINSOCK        0
#define PSI_CFG_TCIP_STACK_CUSTOM         0
#define PSI_CFG_TCIP_USE_OBSD_SNMPD       0

#define PSI_CFG_OBSD_CFG_CLUSTER_IP_SUPPORTED   0
#define PSI_CFG_OBSD_PNIO_CFG_DHCP_CLIENT_ON    0

/**
 * This define enables the TCP communication channel from SOCK to EDDx
 * By disabling this define a TCP communication is not available (e.g. webserver)
 * ARP, ICMP and UDP are still working
 */
#define PSI_CFG_TCIP_CFG_SUPPORT_PATH_EDD_TCP

/**
 * Using COPY IF for TCIP
 */

#define PSI_CFG_TCIP_CFG_COPY_ON_SEND                   1

#define PSI_CFG_MAX_SOCK_SOCKAPP_CHANNELS               0
#define PSI_CFG_OHA_MAX_DESCRIPTION_LEN                 255
#define PSI_CFG_EDD_CFG_MAX_MRP_IN_INSTANCE_CNT         1
#define PSI_CFG_MAX_SNMPX_MGR_SESSIONS                  16 /*PSI_CFG_MAX_SOCKETS*/
#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif
#endif  /* of PSI_CFG_PLATFORM_H */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
