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
/*  F i l e               &F: psi_cfg.h                                 :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  System integration of LSA-component PSI.                                 */
/*  Using the   PNIO framework.                                              */
/*                                                                           */
/*****************************************************************************/

/*
* included by "psi_inc.h"
*/

#ifndef PSI_CFG_H               /* ----- reinclude-protection ----- */
#define PSI_CFG_H

#ifdef __cplusplus              /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

#include "psi_cfg_plf_eb200p.h"

/*===========================================================================*/
/*                             compiler-switches                             */
/*===========================================================================*/

#ifndef PSI_DEBUG
    #error "config error, PSI_DEBUG is not defined"
#endif

/*---------------------------------------------------------------------------*/
/* definition for invalid LOCK- and Event-Handles                            */
/*---------------------------------------------------------------------------*/

#define PSI_LOCK_ID_INVALID     0
#define PSI_EVENT_NO_TIMEOUT    0xFFFFFFFFUL

/*---------------------------------------------------------------------------*/
/* Type of systemhandles used by system for device and interface             */
/*---------------------------------------------------------------------------*/

#define PSI_SYS_HANDLE      LSA_VOID_PTR_TYPE
#define PSI_EDD_HDDB        LSA_VOID_PTR_TYPE

/*===========================================================================*/
/*                              basic attributes                             */
/*===========================================================================*/
/* none PSI only supports flat memory modell */

/*===========================================================================*/
/*                         code- and data-attributes                         */
/*===========================================================================*/
/* none PSI only supports flat memory modell */

/*===========================================================================*/
/* LSA-HEADER and LSA-TRAILER                                                */
/*===========================================================================*/

#define PSI_RQB_HEADER      LSA_RQB_HEADER(PSI_UPPER_RQB_PTR_TYPE)

#define PSI_RQB_TRAILER     LSA_RQB_TRAILER

#define PSI_RQB_SET_NEXT_RQB_PTR(rb,v)      LSA_RQB_SET_NEXT_RQB_PTR(rb,v)
#define PSI_RQB_SET_PREV_RQB_PTR(rb,v)      LSA_RQB_SET_PREV_RQB_PTR(rb,v)
#define PSI_RQB_SET_OPCODE(rb,v)            LSA_RQB_SET_OPCODE(rb,v)
#define PSI_RQB_SET_HANDLE(rb,v)            LSA_RQB_SET_HANDLE(rb,v)
#define PSI_RQB_SET_RESPONSE(rb,v)          LSA_RQB_SET_RESPONSE(rb,v)
#define PSI_RQB_SET_USER_ID_PTR(rb,v)       LSA_RQB_SET_USER_ID_PTR(rb,v)
#define PSI_RQB_SET_COMP_ID(rb,v)           LSA_RQB_SET_COMP_ID(rb,v)

#define PSI_RQB_GET_NEXT_RQB_PTR(rb)        LSA_RQB_GET_NEXT_RQB_PTR(rb)
#define PSI_RQB_GET_PREV_RQB_PTR(rb)        LSA_RQB_GET_PREV_RQB_PTR(rb)
#define PSI_RQB_GET_OPCODE(rb)              LSA_RQB_GET_OPCODE(rb)
#define PSI_RQB_GET_HANDLE(rb)              LSA_RQB_GET_HANDLE(rb)
#define PSI_RQB_GET_RESPONSE(rb)            LSA_RQB_GET_RESPONSE(rb)
#define PSI_RQB_GET_USER_ID_PTR(rb)         LSA_RQB_GET_USER_ID_PTR(rb)
#define PSI_RQB_GET_COMP_ID(rb)             LSA_RQB_GET_COMP_ID(rb)

#define PSI_EDD_RQB_SET_HANDLE_LOWER(rb,v)  EDD_RQB_SET_HANDLE_LOWER(rb,v)

#define PSI_EDD_RQB_GET_HANDLE_LOWER(rb)    EDD_RQB_GET_HANDLE_LOWER(rb)

/*------------------------------------------------------------------------------
// enable / disable PSI_ASSERT
//----------------------------------------------------------------------------*/

#if PSI_DEBUG
    #define PSI_SAFTY_MAGIC_HANDLE__PSI_SYS_TYPE        0xBB000000
    #define PSI_SAFTY_MAGIC_HANDLE__PSI_EDD_SYS_TYPE    0xBB000001
    #define PSI_SAFTY_MAGIC_HANDLE__PSI_HIF_SYS_TYPE    0xBB000002
    /* check for condition: internal programming error */
    #define PSI_ASSERT(cond) if (! (cond)) PSI_FATAL (0) 

#else

    /* no more programming error exists :-) */
    #define PSI_ASSERT(cond) { LSA_UNUSED_ARG(cond); } // Added body to avoid unreferenced parameters compiler warnings.

#endif

/*------------------------------------------------------------------------------
// pointer test
//----------------------------------------------------------------------------*/

void* PSI_TEST_ALIGN2( void const * ptr );
void* PSI_TEST_ALIGN4( void const * ptr );

/*------------------------------------------------------------------------------
// interface to BTRACE
//----------------------------------------------------------------------------*/

#ifndef PSI_FILE_SYSTEM_EXTENSION
    #define PSI_FILE_SYSTEM_EXTENSION(module_id_)
#endif

/*---------------------------------------------------------------------------*/
/* Inline function implementation                                            */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Configure PNIO packages based on PSI base configuration                   */
/*---------------------------------------------------------------------------*/
#define PSI_CFG_MRP_MAX_NO_OF_INTERFACES (PSI_CFG_MAX_IF_CNT)
/*---------------------------------------------------------------------------*/
/* Configure Tracing                                                         */
/*---------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
// interface to LSA trace
//  0 .. no LSA trace
//  1 .. LSA trace [default]
//  2 .. LSA index trace
//----------------------------------------------------------------------------*/
#ifndef PSI_CFG_TRACE_MODE
#define PSI_CFG_TRACE_MODE 1
#endif

/*---------------------------------------------------------------------------*/
/* Configure Common                                                          */
/*---------------------------------------------------------------------------*/

/* The system capabilities contain the supported functions of the system (IEEE 802.1AB) */
#define PSI_CFG_LLDP_SYSCAP_SUPPORTED   (LLDP_TLV_SYSCAB_STATION_ONLY|LLDP_TLV_SYSCAB_BRIDGE)

/* The enabled capabilities contain the currently enabled functions of the system capabilities (IEEE 802.1AB) */
#define PSI_CFG_LLDP_SYSCAP_ENABLED     (LLDP_TLV_SYSCAB_STATION_ONLY|LLDP_TLV_SYSCAB_BRIDGE)

/* type for PSI_EXCHANGE_LONG */
#define PSI_EXCHANGE_TYPE               LSA_INT32

/*---------------------------------------------------------------------------*/
/* Configure CM                                                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Configure extra client resources used by user of CL clients               */
/* needed for use case IOC,IOM,IOD = 0                                       */
/* note: if not configured default off (=0) is set                           */
/*---------------------------------------------------------------------------*/
#ifndef PSI_CFG_MAX_CL_TOOLS
    #define PSI_CFG_MAX_CL_TOOLS        0
#endif

/*---------------------------------------------------------------------------*/
/* System Redundancy (Primary/Backup) for S2-Multidevice                     */
/* see CM documentation, CM_CFG_MAX_SV_ARSETS_PER_DEVICE                     */
/*---------------------------------------------------------------------------*/
#define PSI_CFG_MAX_SV_ARSETS_PER_DEVICE    1

/*---------------------------------------------------------------------------*/
/* PNIO configuration for PSI-LD (LD components)                             */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Configure CLRPC                                                           */
/*---------------------------------------------------------------------------*/

#define PSI_CFG_CLRPC_CFG_USE_CASTS         1
#define PSI_CFG_CLRPC_CFG_MAX_CLIENTS       (((PSI_CFG_MAX_CL_TOOLS) + (PSI_CFG_MAX_CL_DEVICES) + (PSI_CFG_MAX_SV_DEVICES)*(PSI_CFG_CM_CFG_MAX_SV_ARS)) * (PSI_CFG_MAX_IF_CNT)) /*AP01511246 This constant will be replaced by a runtime parameter in future*/

#if (PSI_CFG_MAX_SV_DEVICES == 0)
    #if ((PSI_CFG_USE_IOC == 0) && (PSI_CFG_USE_IOD == 0) && (PSI_CFG_USE_IOM == 0))
        #define PSI_CFG_CLRPC_CFG_MAX_SERVERS   0 /* PDev only: no server-part */
    #else
        #define PSI_CFG_CLRPC_CFG_MAX_SERVERS   (1/*EPM*/ + (1/*CM_CL*/ + (PSI_CFG_MAX_SV_DEVICES)/*CM-SV*/) * (PSI_CFG_MAX_IF_CNT))
    #endif
#else
    #define PSI_CFG_CLRPC_CFG_MAX_SERVERS   (1/*EPM*/ + (1/*CM_CL*/ + (PSI_CFG_MAX_SV_DEVICES)/*CM-SV*/) * (PSI_CFG_MAX_IF_CNT))
#endif
/*---------------------------------------------------------------------------*/
/* Configure DNS                                                             */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Configure IP2PN                                                             */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Configure SNMPX                                                           */
/*---------------------------------------------------------------------------*/

#define PSI_CFG_SNMPX_CFG_TRACE_RQB_ERRORS      0

#if (PSI_CFG_USE_SNMPX == 1)
    #define PSI_CFG_SNMPX_CFG_MAX_MANAGER_SESSIONS      16 /*PSI_CFG_MAX_SOCKETS*/
    #if (defined(PSI_CFG_SNMPX_CFG_MAX_MANAGER_SESSIONS) && (PSI_CFG_SNMPX_CFG_MAX_MANAGER_SESSIONS > 0))
        #define PSI_CFG_USE_SNMPX_MGR                   1
        #define PSI_CFG_SNMPX_CFG_SNMP_MANAGER
    #endif
#endif

/*---------------------------------------------------------------------------*/
/* Configure SOCK                                                            */
/*---------------------------------------------------------------------------*/
#if (PSI_CFG_USE_SOCK == 1)
    /* global CLRPC + SNMPX + SOCK App Users + SOCKApp Channels */
    #define PSI_CFG_SOCK_CFG_MAX_CHANNELS       (2 + (PSI_CFG_MAX_SOCK_APP_CHANNELS))
#else // (PSI_CFG_USE_SOCK == 0)
    // SOCK is not used, no channels to SOCK, redefine to 0
    #if defined (PSI_CFG_MAX_SOCK_APP_CHANNELS)
        #undef PSI_CFG_MAX_SOCK_APP_CHANNELS
        #define PSI_CFG_MAX_SOCK_APP_CHANNELS   0
    #endif

    #define PSI_CFG_SOCK_CFG_MAX_CHANNELS   0
#endif // (PSI_CFG_USE_SOCK == 1)

/*----------------------------------------------------------*/
/* calculation of PSI_CFG_MAX_SOCKETS                       */
/* same as TCIP_CFG_MAX_SOCKETS                             */
/*----------------------------------------------------------*/
//PSI_CFG_MAX_APPL_SOCKETS
#if PSI_CFG_USE_IOC
    #define PSI_MAX_SOCKETS_IOC     1
#else
    #define PSI_MAX_SOCKETS_IOC     0
#endif

#define PSI_CFG_MAX_APPL_SOCKETS    7

#define PSI_CFG_MAX_SOCKETS     ((PSI_CFG_MAX_APPL_SOCKETS) + (PSI_CFG_MAX_SV_DEVICES)*(PSI_CFG_MAX_IF_CNT) + (PSI_MAX_SOCKETS_IOC)*(PSI_CFG_MAX_IF_CNT)) + 8/*buffer for application sockets*/

/*---------------------------------------------------------------------------*/
/* Configure TCIP                                                            */
/*---------------------------------------------------------------------------*/
#if (PSI_CFG_USE_TCIP == 1)

#if (PSI_CFG_TCIP_STACK_OPEN_BSD == 1)
    /* OpenBSD Page Memory: 500 pages with 4k size */
    #define PSI_CFG_TCIP_CFG_OBSD_PAGE_SIZE             4096
    #define PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_PAGES       120
    #define PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_BITS        32
    #define PSI_CFG_SOCK_CFG_ENABLE_MULTICASTS          1
    #define PSI_CFG_TCIP_CFG_ENABLE_MULTICAST           1
    #define PSI_CFG_TCIP_CFG_OBSD_USED_PAGE_ARR_SIZE    ((PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_PAGES / PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_BITS) + ((PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_PAGES % PSI_CFG_TCIP_CFG_OBSD_NUMBER_OF_BITS) ? 1 : 0))
#endif // (PSI_CFG_TCIP_STACK_OPEN_BSD == 1)

#if (PSI_CFG_TCIP_STACK_WINSOCK == 1)
    #define PSI_CFG_SOCK_CFG_ENABLE_MULTICASTS  0
    #define PSI_CFG_TCIP_CFG_ENABLE_MULTICAST   0
#endif // (PSI_CFG_TCIP_STACK_WINSOCK == 1)

#if (PSI_CFG_TCIP_STACK_CUSTOM == 1)
    // #define PSI_CFG_SOCK_CFG_ENABLE_MULTICASTS  ?
    #define PSI_CFG_TCIP_CFG_ENABLE_MULTICAST   0
#endif // (PSI_CFG_TCIP_STACK_CUSTOM == 1)

#endif // (PSI_CFG_USE_TCIP == 1)

#define PSI_CFG_TCIP_CFG_SOMAXCONN              10 /* maximum length of the TCP backlog (1-127, 0 defaults to 5) */
#define PSI_CFG_TCIP_CFG_SYS_SERVICES           0x4C /* 0x4C == 0b01001100 == Layer 7+4+3, see RFC3418, sysServices */ 

/*
 * This controls definitions in OBSD Kernel headers that are visible in "Userland" components as well.
 * SNMPX is an OBSD "Userland" component, even if the OBSD IP-Stack itself is NOT used in the project.
 * At the moment, SNMPX does NOT actually depend on OBSD alignment definitions. However there is no
 * systematic reason why that could not change in the future.
 * Therefore the "clean" solution for OBSD alignment is a definition that exists even if
 * PSI_CFG_TCIP_STACK_OPEN_BSD == 0 so that it can still be used in SNMPX in that case.
 * Set to either 4 (32bit target architecture) or 8 (64bit target architecture). For most targets, 4 is the likely setting.
 */
#define PSI_CFG_OBSD_PNIO_CFG_ALIGNMENT_BYTES   4 /* 4 Byte alignment for all targets (for variants with any OBSD component) */

/* Used for TCIP_CFG_MAXARPS */
#define PSI_CFG_MAX_SV_IOC_ARS              9 

/*-------------------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* PNIO configuration for PSI-HD (HD components)                             */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Configure ACP                                                             */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Configure DCP                                                             */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Configure EDDx (for all)                                                  */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Configure EDDI                                                            */
/*---------------------------------------------------------------------------*/
#if (PSI_CFG_USE_EDDI == 1)

/* Please define the interrupt or polling mode settings for EDDI */
/* 	Samples - description see eddi_sys.doc
	#if ([PollingMode])
	#define PSI_CFG_EDDI_CFG_SII_POLLING_MODE
	
	#elif ([EXTTIMERMODE+INTERRUPTS])
	#define PSI_CFG_EDDI_CFG_SII_EXTTIMER_MODE_ON
	#define PSI_CFG_EDDI_CFG_SII_REMOVE_ISR_LOCKS
	#define PSI_CFG_EDDI_CFG_SII_INT_RESTART_MODE_LOOP
	
	#elif ([NewCycleSyncMode])
	#define PSI_CFG_EDDI_CFG_SII_NEW_CYCLE_SYNC_MODE
	#define PSI_CFG_EDDI_CFG_SII_NEW_CYCLE_SYNC_MODE_NRT_CHECK_US   250     //x = 250 or 500 or 1000 us
	
	#elif ([NormalInterrupt])
	#define PSI_CFG_EDDI_CFG_SII_NRT_TX_RELOAD_REDUCTION
	#define PSI_CFG_EDDI_CFG_SII_INT_RESTART_MODE_LOOP
	#define PSI_CFG_EDDI_CFG_SII_NO_PNDEVDRV_SUPPORT
	#define PSI_CFG_EDDI_CFG_SII_USE_SPECIAL_EOI (EDDI_PASTE_32(00,00,00,07))
	#define PSI_CFG_EDDI_CFG_SII_REMOVE_ISR_LOCKS
	#endif
*/

/*---------------------------------------------------------------------------*/
/* Max number of providers per interface supported.                          */
/* (1..0x7FFF)                                                               */
/*---------------------------------------------------------------------------*/
#define PSI_CFG_EDDI_CFG_MAX_NR_PROVIDERS       ( (PSI_CFG_MAX_CL_DEVICES) + (PSI_CFG_MAX_MC_DEVICES) + ((PSI_CFG_MAX_SV_DEVICES)*(PSI_CFG_MAX_SV_IO_ARS)) )
#if (PSI_CFG_EDDI_CFG_MAX_NR_PROVIDERS > 1024)
    #undef PSI_CFG_EDDI_CFG_MAX_NR_PROVIDERS
    #define PSI_CFG_EDDI_CFG_MAX_NR_PROVIDERS   1024 // limit it to allowed maximum
#endif

/* Supported PHYs*/
#ifdef PSI_CFG_EDDI_CFG_SOC
    // EPS variants support all PHYs that are supported in SOC1 boards
    #define PSI_CFG_EDDI_CFG_PHY_TRANSCEIVER_NSC
    #define PSI_CFG_EDDI_CFG_PHY_TRANSCEIVER_NEC
    #define PSI_CFG_EDDI_CFG_PHY_TRANSCEIVER_BROADCOM
    #define PSI_CFG_EDDI_CFG_PHY_TRANSCEIVER_TI
    #define PSI_CFG_EDDI_CFG_PHY_BLINK_EXTERNAL_NEC
#endif // #if (PSI_CFG_EDDI_CFG_SOC == 1)

#ifdef PSI_CFG_EDDI_CFG_ERTEC_200
    // EPS variants support all PHYs that are supported in EB200PCIe and EB200PCI boards    
    #define PSI_CFG_EDDI_CFG_PHY_TRANSCEIVER_NSC
    #define PSI_CFG_EDDI_CFG_PHY_TRANSCEIVER_NEC
    #define PSI_CFG_EDDI_CFG_PHY_TRANSCEIVER_BROADCOM
    #define PSI_CFG_EDDI_CFG_PHY_BLINK_EXTERNAL_NEC
#endif // #if (PSI_CFG_EDDI_CFG_ERTEC_200 == 1)

#ifdef PSI_CFG_EDDI_CFG_ERTEC_400
    // EPS variants support all PHYs that are supported in ERTEC400 PCIe and CP1616 boards
    #define PSI_CFG_EDDI_CFG_PHY_TRANSCEIVER_NSC           /* NSC Transceiver (PHY)                */
    #define PSI_CFG_EDDI_CFG_PHY_TRANSCEIVER_NEC           /* BROADCOM (BCM5221) Transceiver (PHY) */
    #define PSI_CFG_EDDI_CFG_PHY_TRANSCEIVER_BROADCOM      /* NEC (AATHPHYC2) Transceiver (PHY)    */
#endif // #if (PSI_CFG_EDDI_CFG_ERTEC_400 == 1)

#endif // #if (PSI_CFG_USE_EDDI == 1)

/*---------------------------------------------------------------------------*/
/* Configure EDDP                                                            */
/*---------------------------------------------------------------------------*/
#if (PSI_CFG_USE_EDDP == 1)

#endif // #if (PSI_CFG_USE_EDDP == 1)

/*---------------------------------------------------------------------------*/
/* Configure EDDS                                                            */
/*---------------------------------------------------------------------------*/
#if (PSI_CFG_USE_EDDS == 1)

#endif // #if (PSI_CFG_USE_EDDS == 1)

/*---------------------------------------------------------------------------*/
/* Configure GSY                                                             */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Configure LLDP                                                            */
/*---------------------------------------------------------------------------*/
#define PSI_CFG_LLDP_CFG_USE_ENTER_EXIT_TIMEOUT

/*---------------------------------------------------------------------------*/
/* Configure MRP                                                             */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Configure POF                                                             */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Configure IOH                                                             */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Configure HERA IO module                                                  */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Configure PNTRACE                                                         */
/*---------------------------------------------------------------------------*/

#define PNTRC_CFG_COMPILE_MODULE_ID     1
#define PNTRC_CFG_COMPILE_FILE         	1
#define PNTRC_CFG_COMPILE_LINE         	1
#define PNTRC_CFG_LEVEL_DEFAULT        	4
#define PNTRC_CFG_LOCAL_SYSTEM_ID      	1   /* PNTRC_SYSTEM_ID_APP */
#define PNTRC_CFG_SYNC_INTERVAL        	3000 /* 3000 ms Time Sync Interval to lower systems, timer is currently used for periodic trace buffer switching */

/*---------------------------------------------------------------------------*/
/* Configure HIF                                                             */
/*---------------------------------------------------------------------------*/

#define PSI_CFG_HIF_CFG_MAX_LD_CH_HANDLES           100 // globale User + MX IF* IF Pipes
#define PSI_CFG_HIF_CFG_MAX_HD_CH_HANDLES           155 // Max IF * HD IF Pipes + HD Pipe

#define PSI_CFG_HIF_CFG_POLL_INTERVAL_TIME_FACTOR   1 // Set to 1 in PSI, timer base is 100ms base. Results in a timer with 100ms base.

#define PSI_CFG_HIF_CFG_USE_CPHD_APP_SOCK_CHANNELS  0 // Don't use app specific HD Sock channels

#if (EPS_PLF == EPS_PLF_SOC_MIPS)
    //#define PSI_CFG_HIF_DEBUG_MEASURE_PERFORMANCE // set during performance measurement if needed
#endif

/*---------------------------------------------------------------------------*/
/* Configure PSI Messagequeue                                                */
/*---------------------------------------------------------------------------*/

#define PSI_MSG_MAX_MSG_SIZE            4

/*----------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Configure HSA                                                             */
/*---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif
#endif  /* of PSI_CFG_H */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
