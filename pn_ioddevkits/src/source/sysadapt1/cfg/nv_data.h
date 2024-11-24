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
/*****************************************************************************/
#ifndef _NV_DATA_H
#define _NV_DATA_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif


    #define NV_DATA_VERSION     0x0410                   // version 4.1.0 of NV-Data


    // NV data configuration
    #define NV_USERDATA_TOT_LEN  0x8000      //* Size of the NVRAM sector for non-volatile data
    #define NV_USERDATA_SEC_LEN  0x4000      //* Size of the NVRAM single data sector
    #define NV_USERDATA_SEC1     0           //* First NVRAM sector number
    #define NV_USERDATA_SEC2     1           //* Second NVRAM sector number
    // Important - in case of NAND Flash nv data are stored in sectors of size 0x4000 therefore
    // second section offset has exactly that size

	#define ENTER_NV_DATA	OsEnterX(OS_MUTEX_NV_DATA);
	#define EXIT_NV_DATA	OsExitX(OS_MUTEX_NV_DATA);

    #define MAC_ADDR_LEN              6
    #define REMA_DATA_MAXLEN       1024

    #define SNMP_SYSNAME_MAXLEN     255     // according to RFC1213, do not change
    #define SNMP_SYSLOC_MAXLEN      255     // according to RFC1213, do not change
    #define SNMP_SYSCONT_MAXLEN     255     // according to RFC1213, do not change
    #define SNMP_SYSDESCR_MAXLEN    255     // according to RFC1213, do not change
    #define DEVICE_NAME_MAXLEN      256
    #define PRIV_DATA_MAXLEN        128
	#define IP_SUITE_LEN			12
	#define ARFSU_LEN				20		//4B Mode + 16B UUID



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
        PNIO_NVDATA_PRIVATE_DATA,						// Own remanent DATA (e.g. MAC address)
		PNIO_NVDATA_ARFSU								// UUID for AR-FSU functionality
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

    typedef PNIO_VOID (*nv_function_handler)(PNIO_VOID*, PNIO_UINT32);

    typedef struct
    {
        PNIO_BOOL   trigger_pndv;
        PNIO_BOOL   perform_host_callback;
        PNIO_BOOL   im_callback;
        PNIO_UINT32 nv_data_type;
        PNIO_BOOL   restore_first_section;
    } nv_function_param;

    PNIO_UINT32  Bsp_im_data_store
    		(
    			PNIO_NVDATA_TYPE    NvDataType,     // type of data (device name, IP suite, PDEV records,...)
    			PNIO_VOID*          pMem,           // pointer to data source
    			PNIO_UINT32         MemSize,         // size of memory to store
    			LSA_UINT32			ModIdent        // entity index in periph interface-real_cfg
    		);

	PNIO_UINT32  Bsp_im_data_store_no_pndv_trigger
		(
			PNIO_NVDATA_TYPE    NvDataType,     // type of data (device name, IP suite, PDEV records,...)
			PNIO_VOID*          pMem,           // pointer to data source
			PNIO_UINT32         MemSize,         // size of memory to store
			LSA_UINT32			ModIdent        // entity index in periph interface-real_cfg
		);

    PNIO_UINT32  Bsp_nv_data_store
		    (
                PNIO_NVDATA_TYPE    NvDataType,         // type of data (device name, IP suite, PDEV records,...)
			    PNIO_VOID*		    pMem,		        // pointer to data source
			    PNIO_UINT32     	MemSize             // size of memory to store
            );

    PNIO_UINT32  Bsp_im_data_restore
      		(
      			PNIO_NVDATA_TYPE    NvDataType,     // type of data (device name, IP suite, PDEV records,...)
      			PNIO_VOID**         ppMem,           // pointer to data source
      			PNIO_UINT32*        pMemSize,         // size of memory to restore
      			LSA_UINT32			ModIdent
      		);


    PNIO_UINT32  Bsp_nv_data_restore
		    (
                PNIO_NVDATA_TYPE    NvDataType,         // type of data (device name, IP suite, PDEV records,...)
			    PNIO_VOID** 		ppMem, 		        // destination pointer
			    PNIO_UINT32*     	pMemSize            // size of memory to restore
            );

   PNIO_UINT32  Bsp_nv_data_init    (PNIO_VOID* nvDataInit, PNIO_UINT32 nvDataLen);
   PNIO_UINT32  Bsp_nv_data_clear   (PNIO_RTF_OPTION RtfOption);// reset to factory settings
   PNIO_UINT32  Bsp_nv_data_memfree (PNIO_VOID* pMem);  // free memory (must be called after



#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
