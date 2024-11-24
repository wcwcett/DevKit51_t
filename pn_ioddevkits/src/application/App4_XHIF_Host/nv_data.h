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
/*  F i l e               &F: nv_data.h                                 :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/* socket layer abstraction interface, implementation for Windows            */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/
#ifndef _NV_DATA_H
#define _NV_DATA_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

    #include "pnio_types.h"

	// *********** defines **********

	#define USE_ERTEC_NV_MEM   		0
	#define NV_DATA_FILE_NAME    	"nv_data.dat"
	#define NV_TRACE_FILE_NAME    	"nv_traces.dat"

    #define NV_DATA_VERSION     0x0410                   // version 4.1.0 of NV-Data

    #define MAC_ADDR_LEN              6
    #define REMA_DATA_MAXLEN       1024

    #define SNMP_SYSNAME_MAXLEN     255     // according to RFC1213, do not change
    #define SNMP_SYSLOC_MAXLEN      255     // according to RFC1213, do not change
    #define SNMP_SYSCONT_MAXLEN     255     // according to RFC1213, do not change
    #define SNMP_SYSDESCR_MAXLEN    255     // according to RFC1213, do not change
    #define DEVICE_NAME_MAXLEN      256
    #define PRIV_DATA_MAXLEN        128
    #define IP_SUITE_LEN            12
    #define ARFSU_LEN               20      //4B Mode + 16B UUID

    typedef enum
    {
        PNIO_NVDATA_MAC_ADDR ,                          // type = device mac address
        PNIO_NVDATA_DEVICENAME,                         // type = device name
        PNIO_NVDATA_IPSUITE,                            // type = IP suite:  IP address, Subnet Mask, Default Router, format: network (big endian)
        PNIO_NVDATA_PDEV_RECORD,                        // type = physical device records
        PNIO_NVDATA_IM1,                                // type = I&M1 data
        PNIO_NVDATA_IM2,                                // type = I&M1 data
        PNIO_NVDATA_IM3,                                // type = I&M1 data
        PNIO_NVDATA_IM4,                                // type = I&M1 data
        PNIO_NVDATA_SNMP_SYSNAME,                       // SNMP MIB2 object "system name"
        PNIO_NVDATA_SNMP_SYSCONT,                       // SNMP MIB2 object "system contact"
        PNIO_NVDATA_SNMP_SYSLOC,                        // SNMP MIB2 object "system location"
        PNIO_NVDATA_PRIVATE_DATA,                       // Own remanent DATA (e.g. MAC address)
        PNIO_NVDATA_ARFSU                               // UUID for AR-FSU functionality
    } PNIO_NVDATA_TYPE;

    typedef struct
    {
        PNIO_UINT32 IpAddr;                             // ip address
        PNIO_UINT32 SubnetMask;                         // network mask
        PNIO_UINT32 DefRouter;                          // default router
    } NV_IP_SUITE;  // format: network (big endian)

    typedef struct
    {
        PNIO_INT8   sysName     [SNMP_SYSNAME_MAXLEN];     // MIB2 object sysName
        PNIO_INT8   sysContact  [SNMP_SYSCONT_MAXLEN];     // MIB2 object sysContact
        PNIO_INT8   sysLocation [SNMP_SYSLOC_MAXLEN];      // MIB2 object sysLocation
    } NV_SNMP;

    typedef enum  {
        PNIO_RTF_RES_ALL            = 0x0000, //* suboption RTF - reset all device data (could also be
                                              //* the old service factory reset, which has the same meaning)
        PNIO_RTF_RES_APPL_PAR       = 0x0001, //* suboption RTF - reset application data for one interface
        PNIO_RTF_RES_COMM_PAR       = 0x0002, //* suboption RTF - reset communication data for one interface
        PNIO_RTF_RES_ENG_PAR        = 0x0003, //* suboption RTF - reset engineering data for one interface
        PNIO_RTF_RES_FWUP           = 0x0004
    } PNIO_RTF_OPTION;

    PNIO_UINT32  Bsp_im_data_store
            (
                PNIO_NVDATA_TYPE    NvDataType,     // type of data (device name, IP suite, PDEV records,...)
                void*               pMem,           // pointer to data source
                PNIO_UINT32         MemSize,         // size of memory to store
                PNIO_UINT32          ModIdent        // entity index in periph interface-real_cfg
            );

    PNIO_UINT32  Bsp_nv_data_store
            (
                PNIO_NVDATA_TYPE    NvDataType,         // type of data (device name, IP suite, PDEV records,...)
                void*               pMem,               // pointer to data source
                PNIO_UINT32         MemSize             // size of memory to store
            );

    PNIO_UINT32  Bsp_im_data_restore
            (
                PNIO_NVDATA_TYPE    NvDataType,     // type of data (device name, IP suite, PDEV records,...)
                void**              ppMem,           // pointer to data source
                PNIO_UINT32*         pMemSize,         // size of memory to restore
                PNIO_UINT32          ModIdent
            );


    PNIO_UINT32  Bsp_nv_data_restore
            (
                PNIO_NVDATA_TYPE    NvDataType,         // type of data (device name, IP suite, PDEV records,...)
                void**              ppMem,              // destination pointer
                PNIO_UINT32*        pMemSize            // size of memory to restore
            );

   PNIO_UINT32  Bsp_nv_data_init    (PNIO_VOID);
   PNIO_UINT32  Bsp_nv_data_sync    (PNIO_VOID* nvData, PNIO_UINT32 nvSize, PNIO_UINT32 errOccured);
   PNIO_UINT32  Bsp_nv_data_clear   (PNIO_RTF_OPTION RtfOption);// reset to factory settings
   PNIO_UINT32  Bsp_nv_data_memfree (PNIO_VOID* pMem);  // free memory (must be called after

   PNIO_VOID Bsp_nv_data_flash_done ( PNIO_UINT32 Status, PNIO_UINT32 DatLen, PNIO_UINT32 nvDataType );
   PNIO_VOID Bsp_im_data_flash_done ( PNIO_UINT32 Status, PNIO_UINT32 DatLen, PNIO_UINT32 nvDataType );
   PNIO_VOID Bsp_nv_data_factory_reset_flash_done ( PNIO_UINT32 Status, PNIO_UINT32 DatLen, PNIO_UINT32 nvDataType );

   PNIO_VOID   NvDataSetDefaultValues ( PNIO_UINT32 NvDataTypes );
   PNIO_UINT32 SaveDataToFlashMem(PNIO_VOID);

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif
      
/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
