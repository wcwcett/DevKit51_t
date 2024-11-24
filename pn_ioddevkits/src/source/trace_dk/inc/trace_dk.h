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
/*  F i l e               &F: trace_dk.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  compiler dependent definitions                                           */
/*                                                                           */
/*****************************************************************************/

#ifndef TRACE_DK_
#define TRACE_DK_

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif



	#ifndef TRC_OUTP_DEVICE_USE_ENUM_TYPE
		#define DEV_STD     1    	/* bit 0 for send messages to stdout console					*/
		#define DEV_MEM     2     	/* bit 1 for send message to circular memory buffer				*/
		#define DEV_LSA     4     	/* bit 2 for send message to circular LSA trace buffer			*/
		#define DEV_ALL		7		/* bit 0 + bit1 for send message to stdout and circular buffer	*/
		#define TRC_OUTP_DEVICE		PNIO_UINT8
	#else
		typedef enum
        {
            DEV_STD       = 1,     // bit 0 for send messages to stdout console
            DEV_MEM       = 2,     // bit 1 for send message to circular memory buffer
            DEV_LSA       = 4,     // bit 2 for send message to circular LSA trace buffer
            DEV_ALL       = 7      // bit 0 + bit1 for send message to stdout and circular buffer
        } TRC_OUTP_DEVICE;
	#endif

        typedef struct _SUBS_TRACE_LEVEL
        {
            PNIO_UINT8      level;
            PNIO_UINT8      MinLevel_Mem;
            PNIO_UINT8      MinLevel_StdOut;
            PNIO_UINT8      MinLevel_Lsa;
        } SUBS_TRACE_LEVEL;


        // *-------------------------------------------------------------------------
        // * functions of Trace DK Interface
        // *-------------------------------------------------------------------------
        PNIO_VOID   TrcDkInit (PNIO_VOID);

        PNIO_VOID   TrcDkSetLevel (PNIO_UINT32 Subsys, PNIO_UINT32 TraceLevel);
        PNIO_VOID   TrcDkSetAllLevel (PNIO_UINT32 TraceLevel);
        PNIO_VOID   TrcDkSetPackageLevel (PNIO_UINT32 Subsys, PNIO_UINT32 TraceLevel);

        PNIO_VOID   TrcDkSetMinLevel (PNIO_UINT32 Subsys, TRC_OUTP_DEVICE OutDev, PNIO_UINT32 MinLevel);
        PNIO_VOID   TrcDkSetAllMinLevel (TRC_OUTP_DEVICE OutDev, PNIO_UINT32 MinLevel);
        PNIO_VOID   TrcDkSetPackageMinLevel(PNIO_UINT32 Subsys, TRC_OUTP_DEVICE OutDev,  PNIO_UINT32 MinLevel);
        PNIO_UINT32 TrcDkGetSubsysIds(PNIO_UINT32 Subsys,PNIO_UINT32* pFirstSubsys, PNIO_UINT32* pLastSubsys);

        // *-------------------------------------------------------------------------
        // * define ltrace levels */
        // *-------------------------------------------------------------------------

        #ifdef IM_HW_FLASH_32BIT
        #define LSA_TRACE_COMPILE_LEVEL  LSA_TRACE_LEVEL_NOTE
        #else
        #define LSA_TRACE_COMPILE_LEVEL  PNIO_TRACE_COMPILE_LEVEL
        #endif

		// *-------------------------------------------------------------------------
		// * defines the available TRACE SUBSYSTEMS
		// * note:  TRACE_SUBSYS_xxx_LOWER  must be first element of lsa package xxx
		// *        TRACE_SUBSYS_xxxUPPER   must be last  element of lsa package xxx
		// *        TRACE_SUBSYS_NUM        must be last  element of the list
		// *-------------------------------------------------------------------------
		#if (PNIO_TRACE == PNIO_TRACE_DK_LSA)
			typedef enum
			{
				TRACE_SUBSYS_TRACE,

				TRACE_SUBSYS_ACP_LOWER,
				TRACE_SUBSYS_ACP_MEMORY,
				TRACE_SUBSYS_ACP_RTA,
				TRACE_SUBSYS_ACP_FUNCTION,
				TRACE_SUBSYS_ACP_UPPER,

				TRACE_SUBSYS_CLRPC_LOWER,
				TRACE_SUBSYS_CLRPC_CL,
				TRACE_SUBSYS_CLRPC_CL_PKT,
				TRACE_SUBSYS_CLRPC_SV,
				TRACE_SUBSYS_CLRPC_SV_PKT,
				TRACE_SUBSYS_CLRPC_EPM,
				TRACE_SUBSYS_CLRPC_UPPER,


				TRACE_SUBSYS_CM_LOWER,
				TRACE_SUBSYS_CM_ACP,
				TRACE_SUBSYS_CM_AR,
				TRACE_SUBSYS_CM_CL,
				TRACE_SUBSYS_CM_EDD,
				TRACE_SUBSYS_CM_GSY,
				TRACE_SUBSYS_CM_MC,
				TRACE_SUBSYS_CM_MEM,
				TRACE_SUBSYS_CM_MRP,
				TRACE_SUBSYS_CM_NARE,
				TRACE_SUBSYS_CM_OHA,
				TRACE_SUBSYS_CM_PD,
				TRACE_SUBSYS_CM_POF,
				TRACE_SUBSYS_CM_RPC,
				TRACE_SUBSYS_CM_REMA,
				TRACE_SUBSYS_CM_SV,
				TRACE_SUBSYS_CM_UPPER,

				TRACE_SUBSYS_DCP_LOWER,
				TRACE_SUBSYS_DCP_SYSTEM,
				TRACE_SUBSYS_DCP_ERROR,
				TRACE_SUBSYS_DCP_FUNCTION,
				TRACE_SUBSYS_DCP_SERVER,
				TRACE_SUBSYS_DCP_SRVERR,
				TRACE_SUBSYS_DCP_SNDRCV,
				TRACE_SUBSYS_DCP_UPPER,

				TRACE_SUBSYS_GSY_LOWER,
				TRACE_SUBSYS_GSY_SYSTEM,
				TRACE_SUBSYS_GSY_ERROR,
				TRACE_SUBSYS_GSY_FUNCTION,
				TRACE_SUBSYS_GSY_PROGRAM,
				TRACE_SUBSYS_GSY_DEL,
				TRACE_SUBSYS_GSY_DIAG,
				TRACE_SUBSYS_GSY_FWD,
				TRACE_SUBSYS_GSY_PRM,
				TRACE_SUBSYS_GSY_SYNC,
				TRACE_SUBSYS_GSY_SNDRCV,
				TRACE_SUBSYS_GSY_MASTER,
				TRACE_SUBSYS_GSY_UPPER,

				TRACE_SUBSYS_LLDP_LOWER,
				TRACE_SUBSYS_LLDP_SYSTEM,
				TRACE_SUBSYS_LLDP_FUNCTION,
				TRACE_SUBSYS_LLDP_PROGRAM,
				TRACE_SUBSYS_LLDP_UPPER,

				TRACE_SUBSYS_EDDP_LOWER,
				TRACE_SUBSYS_EDDP_SYSTEM,
				TRACE_SUBSYS_EDDP_CORE,
				TRACE_SUBSYS_EDDP_NRT,
				TRACE_SUBSYS_EDDP_CRT,
				TRACE_SUBSYS_EDDP_PYH,
				TRACE_SUBSYS_EDDP_SWI,
				TRACE_SUBSYS_EDDP_PRM,
				TRACE_SUBSYS_EDDP_K32,
				TRACE_SUBSYS_EDDP_CMD,
				TRACE_SUBSYS_EDDP_K32_FW,
				TRACE_SUBSYS_EDDP_USER,
				TRACE_SUBSYS_EDDP_UPPER,

				TRACE_SUBSYS_MRP_LOWER,
				TRACE_SUBSYS_MRP_SYSTEM,
				TRACE_SUBSYS_MRP_PROGRAM,
				TRACE_SUBSYS_MRP_UPPER,

				TRACE_SUBSYS_SOCK_LOWER,

				TRACE_SUBSYS_SOCK_SYSTEM,
				TRACE_SUBSYS_SOCK_PROTOCOL,
				TRACE_SUBSYS_SOCK_UPPER,


				TRACE_SUBSYS_POF_LOWER,
				TRACE_SUBSYS_POF_SYSTEM ,
				TRACE_SUBSYS_POF_FUNCTION,
				TRACE_SUBSYS_POF_PROGRAM,
				TRACE_SUBSYS_POF_UPPER,

				TRACE_SUBSYS_IOD_LOWER,
				TRACE_SUBSYS_IOD_SYSADAPT,          // system adaptation of the lsa packages
				TRACE_SUBSYS_IOD_PLATFORM,          // platform adaptation
				TRACE_SUBSYS_IOD_API,               // user application interface
				TRACE_SUBSYS_IOD_MEMORY,            // memory allocation
				TRACE_SUBSYS_IOD_MSG,               // send message / receive message
				TRACE_SUBSYS_IOD_UPPER,

				TRACE_SUBSYS_PNPB_LOWER,
				TRACE_SUBSYS_PNPB_SYSADAPT,          // system adaptation of the lsa packages
				TRACE_SUBSYS_PNPB_PLATFORM,          // platform adaptation
				TRACE_SUBSYS_PNPB_API,               // user application interface
				TRACE_SUBSYS_PNPB_PERIF,             // memory allocation
				TRACE_SUBSYS_PNPB_UPPER,

				TRACE_SUBSYS_PNDV_LOWER,
				TRACE_SUBSYS_PNDV_SYSADAPT,          // system adaptation of the lsa packages
				TRACE_SUBSYS_PNDV_PLATFORM,          // platform adaptation
				TRACE_SUBSYS_PNDV_API,               // user application interface
				TRACE_SUBSYS_PNDV_MEMORY,            // memory allocation
				TRACE_SUBSYS_PNDV_UPPER,

				TRACE_SUBSYS_TCIP_LOWER,
				TRACE_SUBSYS_TCIP_INICHE,
				TRACE_SUBSYS_TCIP_MEM,
				TRACE_SUBSYS_TCIP_UPPER,

				TRACE_SUBSYS_APPL_LOWER,
				TRACE_SUBSYS_APPL_PLATFORM,
				TRACE_SUBSYS_APPL_UPPER,

				TRACE_SUBSYS_TSKMSG_LOWER,
				TRACE_SUBSYS_TSKMSG_APP,
				TRACE_SUBSYS_TSKMSG_EDD,
				TRACE_SUBSYS_TSKMSG_ORG,
				TRACE_SUBSYS_TSKMSG_PNO,
				TRACE_SUBSYS_TSKMSG_TCP,
				TRACE_SUBSYS_TSKMSG_POF,
				TRACE_SUBSYS_TSKMSG_REM,
				TRACE_SUBSYS_TSKMSG_STP,
				TRACE_SUBSYS_TSKMSG_IDL,
				TRACE_SUBSYS_TSKMSG_OSTIM,
				TRACE_SUBSYS_TSKMSG_PNPB,
				TRACE_SUBSYS_TSKMSG_UPPER,

				/*************************************************************************/
				TRACE_SUBSYS_NUM    /* dont change the last entry, its necessary for ltrc */
			} TRACE_SUBSYS_ID;
		#else
			typedef enum
			{
				TRACE_SUBSYS_TRACE           = 0,

				TRACE_SUBSYS_ACP_LOWER,
				TRACE_SUBSYS_ACP_MEMORY,
				TRACE_SUBSYS_ACP_RTA,
				TRACE_SUBSYS_ACP_FUNCTION,
				TRACE_SUBSYS_ACP_UPPER,

				TRACE_SUBSYS_CLRPC_LOWER    = 10,
				TRACE_SUBSYS_CLRPC_CL,
				TRACE_SUBSYS_CLRPC_CL_PKT,
				TRACE_SUBSYS_CLRPC_SV,
				TRACE_SUBSYS_CLRPC_SV_PKT,
				TRACE_SUBSYS_CLRPC_EPM,
				TRACE_SUBSYS_CLRPC_UPPER,


				TRACE_SUBSYS_CM_LOWER      = 20,
				TRACE_SUBSYS_CM_ACP,
				TRACE_SUBSYS_CM_AR,
				TRACE_SUBSYS_CM_CL,
				TRACE_SUBSYS_CM_DCP,
				TRACE_SUBSYS_CM_EDD,
				TRACE_SUBSYS_CM_GSY,
				TRACE_SUBSYS_CM_LLDP,
				TRACE_SUBSYS_CM_MC,
				TRACE_SUBSYS_CM_MEM,
				TRACE_SUBSYS_CM_MRP,      /*30*/
				TRACE_SUBSYS_CM_PD,
				TRACE_SUBSYS_CM_POF,
				TRACE_SUBSYS_CM_RPC,
				TRACE_SUBSYS_CM_REMA,
				TRACE_SUBSYS_CM_SV,
				TRACE_SUBSYS_CM_TIME,
				TRACE_SUBSYS_CM_UPPER,

				TRACE_SUBSYS_GSY_LOWER      = 40,
				TRACE_SUBSYS_GSY_SYSTEM,
				TRACE_SUBSYS_GSY_ERROR,
				TRACE_SUBSYS_GSY_FUNCTION,
				TRACE_SUBSYS_GSY_PROGRAM,
				TRACE_SUBSYS_GSY_DEL,
				TRACE_SUBSYS_GSY_DIAG,
				TRACE_SUBSYS_GSY_FWD,
				TRACE_SUBSYS_GSY_PRM,
				TRACE_SUBSYS_GSY_SYNC,
				TRACE_SUBSYS_GSY_SNDRCV,   /*50*/
				TRACE_SUBSYS_GSY_MASTER,
				TRACE_SUBSYS_GSY_UPPER,

				TRACE_SUBSYS_IP2PN_PROGRAM,

				TRACE_SUBSYS_LLDP_LOWER     = 60,
				TRACE_SUBSYS_LLDP_SYSTEM,
				TRACE_SUBSYS_LLDP_FUNCTION,
				TRACE_SUBSYS_LLDP_PROGRAM,
				TRACE_SUBSYS_LLDP_UPPER,

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_EB200P)
				TRACE_SUBSYS_EDDP_LOWER     = 70,
				TRACE_SUBSYS_EDDP_SYSTEM,
				TRACE_SUBSYS_EDDP_CORE,
				TRACE_SUBSYS_EDDP_NRT,
				TRACE_SUBSYS_EDDP_CRT,
				TRACE_SUBSYS_EDDP_PYH,
				TRACE_SUBSYS_EDDP_SWI,
				TRACE_SUBSYS_EDDP_PRM,
				TRACE_SUBSYS_EDDP_K32,
				TRACE_SUBSYS_EDDP_CMD,
				TRACE_SUBSYS_EDDP_K32_FW,
				TRACE_SUBSYS_EDDP_USER,
				TRACE_SUBSYS_EDDP_UPPER,
#else
#error "platform not defined"
#endif

				TRACE_SUBSYS_MRP_LOWER      = 80,
				TRACE_SUBSYS_MRP_SYSTEM,
				TRACE_SUBSYS_MRP_PROGRAM,
				TRACE_SUBSYS_MRP_UPPER,

				TRACE_SUBSYS_SNMPX_FUNCTION = 90,
				TRACE_SUBSYS_SNMPX_LOWER,
				TRACE_SUBSYS_SNMPX_PROGRAM,
				TRACE_SUBSYS_SNMPX_SYSTEM,
				TRACE_SUBSYS_SNMPX_UPPER,
				TRACE_SUBSYS_SNMPX_PROTOCOL,
				TRACE_SUBSYS_SNMPX_MEMORY,

				TRACE_SUBSYS_SOCK_LOWER     = 100,
				TRACE_SUBSYS_SOCK_SYSTEM,
				TRACE_SUBSYS_SOCK_PROTOCOL,
				TRACE_SUBSYS_SOCK_UPPER,

				TRACE_SUBSYS_POF_LOWER      = 110,
				TRACE_SUBSYS_POF_SYSTEM ,
				TRACE_SUBSYS_POF_FUNCTION,
				TRACE_SUBSYS_POF_PROGRAM,
				TRACE_SUBSYS_POF_UPPER,

				TRACE_SUBSYS_PSI_HD         = 115,
				TRACE_SUBSYS_PSI_LD,
				TRACE_SUBSYS_PSI_SYSTEM,

				TRACE_SUBSYS_IOD_LOWER      = 120,
				TRACE_SUBSYS_IOD_SYSADAPT,          // system adaptation of the lsa packages
				TRACE_SUBSYS_IOD_PLATFORM,          // platform adaptation
				TRACE_SUBSYS_IOD_API,               // user application interface
				TRACE_SUBSYS_IOD_MEMORY,            // memory allocation
				TRACE_SUBSYS_IOD_MSG,               // send message / receive message
				TRACE_SUBSYS_IOD_UPPER,

				TRACE_SUBSYS_PNPB_LOWER      = 130,
				TRACE_SUBSYS_PNPB_SYSADAPT,          // system adaptation of the lsa packages
				TRACE_SUBSYS_PNPB_PLATFORM,          // platform adaptation
				TRACE_SUBSYS_PNPB_API,               // user application interface
				TRACE_SUBSYS_PNPB_PERIF,             // memory allocation
				TRACE_SUBSYS_PNPB_UPPER,

				TRACE_SUBSYS_PNDV_LOWER      = 140,
				TRACE_SUBSYS_PNDV_SYSADAPT,          // system adaptation of the lsa packages
				TRACE_SUBSYS_PNDV_PLATFORM,          // platform adaptation
				TRACE_SUBSYS_PNDV_API,               // user application interface
				TRACE_SUBSYS_PNDV_MEMORY,            // memory allocation
				TRACE_SUBSYS_PNDV_UPPER,

				TRACE_SUBSYS_TCIP_ADAPT     = 150,
				TRACE_SUBSYS_TCIP_ANYIP,
				TRACE_SUBSYS_TCIP_LOWER,
				TRACE_SUBSYS_TCIP_MEM,
				TRACE_SUBSYS_TCIP_OBSD,
				TRACE_SUBSYS_TCIP_OBSD_DHCP,
				TRACE_SUBSYS_TCIP_UPPER,

				TRACE_SUBSYS_APPL_LOWER      = 160,
				TRACE_SUBSYS_APPL_PLATFORM,
				TRACE_SUBSYS_APPL_UPPER,

				TRACE_SUBSYS_TSKMSG_LOWER    = 170,
				TRACE_SUBSYS_TSKMSG_APP,
				TRACE_SUBSYS_TSKMSG_EDD,
				TRACE_SUBSYS_TSKMSG_ORG,
				TRACE_SUBSYS_TSKMSG_PNO,
				TRACE_SUBSYS_TSKMSG_TCP,
				TRACE_SUBSYS_TSKMSG_POF,
				TRACE_SUBSYS_TSKMSG_REM,
				TRACE_SUBSYS_TSKMSG_STP,
				TRACE_SUBSYS_TSKMSG_IDL,
				TRACE_SUBSYS_TSKMSG_OSTIM,
				TRACE_SUBSYS_TSKMSG_PNPB,
				TRACE_SUBSYS_TSKMSG_UPPER,

				/*************************************************************************/
				TRACE_SUBSYS_NUM    /* dont change the last entry, its necessary for ltrc */
			} TRACE_SUBSYS_ID;
		#endif

#define CHECK_LEVEL(_subsys, _idx, _level) \
    ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (_level > PNIO_LOG_DEACTIVATED) && \
    (SubsysTrace[_subsys].level >= _level))

        extern SUBS_TRACE_LEVEL SubsysTrace[TRACE_SUBSYS_NUM ];


#if (PNIO_TRACE == PNIO_TRACE_DK_LSA)

	#define LSA_TRACE_00(subsys_, level_, msg_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_00(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_)
		/*LTRC_TRACE_00(subsys_, level_, msg_)                                                                                                                                              */
	#define LSA_TRACE_01(subsys_, level_, msg_, para1_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_01(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_, (TRACE_UINT32)para1_)
		/*LTRC_TRACE_01(subsys_, level_, msg_, para1_)                                                                                                                                      */
	#define LSA_TRACE_02(subsys_, level_, msg_, para1_, para2_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_02(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_, (TRACE_UINT32)para1_, (TRACE_UINT32)para2_)
		/*LTRC_TRACE_02(subsys_, level_, msg_, para1_, para2_)                                                                                                                              */
	#define LSA_TRACE_03(subsys_, level_, msg_, para1_, para2_, para3_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_03(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_, (TRACE_UINT32)para1_, (TRACE_UINT32)para2_, (TRACE_UINT32)para3_)
		/*LTRC_TRACE_03(subsys_, level_, msg_, para1_, para2_, para3_)                                                                                                                      */
	#define LSA_TRACE_04(subsys_, level_, msg_, para1_, para2_, para3_, para4_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_04(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_, (TRACE_UINT32)para1_, (TRACE_UINT32)para2_, (TRACE_UINT32)para3_, (TRACE_UINT32)para4_)
		/*LTRC_TRACE_04(subsys_, level_, msg_, para1_, para2_, para3_, para4_)                                                                                                              */
	#define LSA_TRACE_05(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_05(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_, (TRACE_UINT32)para1_, (TRACE_UINT32)para2_, (TRACE_UINT32)para3_, (TRACE_UINT32)para4_, (TRACE_UINT32)para5_)
		/*LTRC_TRACE_05(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_)                                                                                                      */
	#define LSA_TRACE_06(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_06(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_, (TRACE_UINT32)para1_, (TRACE_UINT32)para2_, (TRACE_UINT32)para3_, (TRACE_UINT32)para4_, (TRACE_UINT32)para5_, (TRACE_UINT32)para6_)
		/*LTRC_TRACE_06(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_)                                                                                              */
	#define LSA_TRACE_07(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_07(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_, (TRACE_UINT32)para1_, (TRACE_UINT32)para2_, (TRACE_UINT32)para3_, (TRACE_UINT32)para4_, (TRACE_UINT32)para5_, (TRACE_UINT32)para6_, (TRACE_UINT32)para7_)
		/*LTRC_TRACE_07(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_)                                                                                      */
	#define LSA_TRACE_08(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_08(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_, (TRACE_UINT32)para1_, (TRACE_UINT32)para2_, (TRACE_UINT32)para3_, (TRACE_UINT32)para4_, (TRACE_UINT32)para5_, (TRACE_UINT32)para6_, (TRACE_UINT32)para7_, (TRACE_UINT32)para8_)
		/*LTRC_TRACE_08(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_)                                                                              */
	#define LSA_TRACE_09(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_09(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_, (TRACE_UINT32)para1_, (TRACE_UINT32)para2_, (TRACE_UINT32)para3_, (TRACE_UINT32)para4_, (TRACE_UINT32)para5_, (TRACE_UINT32)para6_, (TRACE_UINT32)para7_, (TRACE_UINT32)para8_, (TRACE_UINT32)para9_)
		/*LTRC_TRACE_09(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_)                                                                      */
	#define LSA_TRACE_10(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_10(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_, (TRACE_UINT32)para1_, (TRACE_UINT32)para2_, (TRACE_UINT32)para3_, (TRACE_UINT32)para4_, (TRACE_UINT32)para5_, (TRACE_UINT32)para6_, (TRACE_UINT32)para7_, (TRACE_UINT32)para8_, (TRACE_UINT32)para9_, (TRACE_UINT32)para10_)
		/*LTRC_TRACE_10(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_)                                                             */
	#define LSA_TRACE_11(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_11(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_, (TRACE_UINT32)para1_, (TRACE_UINT32)para2_, (TRACE_UINT32)para3_, (TRACE_UINT32)para4_, (TRACE_UINT32)para5_, (TRACE_UINT32)para6_, (TRACE_UINT32)para7_, (TRACE_UINT32)para8_, (TRACE_UINT32)para9_, (TRACE_UINT32)para10_, (TRACE_UINT32)para11_)
		/*LTRC_TRACE_11(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_)                                                    */
	#define LSA_TRACE_12(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_12(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_, (TRACE_UINT32)para1_, (TRACE_UINT32)para2_, (TRACE_UINT32)para3_, (TRACE_UINT32)para4_, (TRACE_UINT32)para5_, (TRACE_UINT32)para6_, (TRACE_UINT32)para7_, (TRACE_UINT32)para8_, (TRACE_UINT32)para9_, (TRACE_UINT32)para10_, (TRACE_UINT32)para11_, (TRACE_UINT32)para12_)
		/*LTRC_TRACE_12(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_)                                           */
	#define LSA_TRACE_13(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_, para13_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_13(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_, (TRACE_UINT32)para1_, (TRACE_UINT32)para2_, (TRACE_UINT32)para3_, (TRACE_UINT32)para4_, (TRACE_UINT32)para5_, (TRACE_UINT32)para6_, (TRACE_UINT32)para7_, (TRACE_UINT32)para8_, (TRACE_UINT32)para9_, (TRACE_UINT32)para10_, (TRACE_UINT32)para11_, (TRACE_UINT32)para12_, (TRACE_UINT32)para13_)
		/*LTRC_TRACE_13(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_, para13_)                                  */
	#define LSA_TRACE_14(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_, para13_, para14_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_14(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_, (TRACE_UINT32)para1_, (TRACE_UINT32)para2_, (TRACE_UINT32)para3_, (TRACE_UINT32)para4_, (TRACE_UINT32)para5_, (TRACE_UINT32)para6_, (TRACE_UINT32)para7_, (TRACE_UINT32)para8_, (TRACE_UINT32)para9_, (TRACE_UINT32)para10_, (TRACE_UINT32)para11_, (TRACE_UINT32)para12_, (TRACE_UINT32)para13_, (TRACE_UINT32)para14_)
		/*LTRC_TRACE_14(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_, para13_, para14_)                         */
	#define LSA_TRACE_15(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_, para13_, para14_, para15_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_15(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_, (TRACE_UINT32)para1_, (TRACE_UINT32)para2_, (TRACE_UINT32)para3_, (TRACE_UINT32)para4_, (TRACE_UINT32)para5_, (TRACE_UINT32)para6_, (TRACE_UINT32)para7_, (TRACE_UINT32)para8_, (TRACE_UINT32)para9_, (TRACE_UINT32)para10_, (TRACE_UINT32)para11_, (TRACE_UINT32)para12_, (TRACE_UINT32)para13_, (TRACE_UINT32)para14_, (TRACE_UINT32)para15_)
		/*LTRC_TRACE_15(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_, para13_, para14_, para15_)                */
	#define LSA_TRACE_16(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_, para13_, para14_, para15_, para16_) \
		if(level_ <= LSA_TRACE_COMPILE_LEVEL)       \
		trace_add_entry_16(subsys_, LTRC_ACT_MODUL_ID, (LSA_UINT16)__LINE__, level_, (TRACE_UINT32)para1_, (TRACE_UINT32)para2_, (TRACE_UINT32)para3_, (TRACE_UINT32)para4_, (TRACE_UINT32)para5_, (TRACE_UINT32)para6_, (TRACE_UINT32)para7_, (TRACE_UINT32)para8_, (TRACE_UINT32)para9_, (TRACE_UINT32)para10_, (TRACE_UINT32)para11_, (TRACE_UINT32)para12_, (TRACE_UINT32)para13_, (TRACE_UINT32)para14_, (TRACE_UINT32)para15_, (TRACE_UINT32)para16_)
		/*LTRC_TRACE_16(subsys_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_, para13_, para14_, para15_, para16_)       */

	#define LSA_TRACE_BYTE_ARRAY(_subsys, _level, msg, ptr_, len_)


#else

        #define LSA_TRACE_00(_subsys, _level, msg) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_00(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg)

        #define LSA_TRACE_01(_subsys, _level, msg, para1) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_01(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg, (para1))

        #define LSA_TRACE_02(_subsys, _level, msg, para1, para2) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_02(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg, (para1), (para2))

        #define LSA_TRACE_03(_subsys, _level, msg, para1, para2, para3) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_03(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg, (para1), (para2), (para3))

        #define LSA_TRACE_04(_subsys, _level, msg, para1, para2, para3, para4) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_04(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg, (para1), (para2), (para3), (para4))

        #define LSA_TRACE_05(_subsys, _level, msg, para1, para2, para3, para4, para5) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_05(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5))

        #define LSA_TRACE_06(_subsys, _level, msg, para1, para2, para3, para4, para5, para6)  \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_06(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6))

        #define LSA_TRACE_07(_subsys, _level, msg, para1, para2, para3, para4, para5, para6, para7) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_07(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7))

        #define LSA_TRACE_08(_subsys, _level, msg, para1, para2, para3, para4, para5, para6, para7, para8) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_08(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8))

        #define LSA_TRACE_09(_subsys, _level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_09(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8), (para9))

        #define LSA_TRACE_10(_subsys, _level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_10(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8), (para9), (para10))

        #define LSA_TRACE_11(_subsys, _level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_11(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8), (para9), (para10), (para11))

        #define LSA_TRACE_12(_subsys, _level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_12(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8), (para9), (para10), (para11), (para12))

        #define LSA_TRACE_13(_subsys, _level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_13(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8), (para9), (para10), (para11), (para12), (para13))

        #define LSA_TRACE_14(_subsys, _level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_14(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8), (para9), (para10), (para11), (para12), (para13), (para14))

        #define LSA_TRACE_15(_subsys, _level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14, para15) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_15(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8), (para9), (para10), (para11), (para12), (para13), (para14), (para15))

        #define LSA_TRACE_16(_subsys, _level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14, para15, para16) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_16(LTRC_ACT_MODUL_ID , __LINE__, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8), (para9), (para10), (para11), (para12), (para13), (para14), (para15), (para16))

        #define LSA_TRACE_BYTE_ARRAY(_subsys, _level, msg, ptr_, len_) \
                if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
                  DK_TRACE_BYTE_ARRAY (LTRC_ACT_MODUL_ID, __LINE__, _subsys, _level, msg, ptr_, len_)

    #define LSA_TRACE_00_EXT(_subsys, _level, modul_, line_, msg) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_00(modul_ , line_, _subsys, _level, msg)

    #define LSA_TRACE_01_EXT(_subsys, _level, modul_, line_, msg, para1_) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_01(modul_ , line_, _subsys, _level, msg, (para1))

    #define LSA_TRACE_02_EXT(_subsys, _level, modul_, line_, msg, para1, para2) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_02(modul_ , line_, _subsys, _level, msg, (para1), (para2))

    #define LSA_TRACE_03_EXT(_subsys, _level, modul_, line_, msg, para1, para2, para3) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_03(modul_ , line_, _subsys, _level, msg, (para1), (para2), (para3))

    #define LSA_TRACE_04_EXT(_subsys, _level, modul_, line_, msg, para1, para2, para3, para4) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_04(modul_ , line_, _subsys, _level, msg, (para1), (para2), (para3), (para4))

    #define LSA_TRACE_05_EXT(_subsys, _level, modul_, line_, msg, para1, para2, para3, para4, para5) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_05(modul_ , line_, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5))

    #define LSA_TRACE_06_EXT(_subsys, _level, modul_, line_, msg, para1, para2, para3, para4, para5, para6) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_06(modul_ , line_, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6))

    #define LSA_TRACE_07_EXT(_subsys, _level, modul_ , line_, msg, para1, para2, para3, para4, para5, para6, para7) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_07(modul_ , line_, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7))

    #define LSA_TRACE_08_EXT(_subsys, _level, modul_, line_, msg, para1, para2, para3, para4, para5, para6, para7, para8) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_08(modul_ , line_, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8))

    #define LSA_TRACE_09_EXT(_subsys, _level, modul_, line_, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_09(modul_ , line_, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8), (para9))

    #define LSA_TRACE_10_EXT(_subsys, _level, modul_, line_, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_10(modul_ , line_, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8), (para9), (para10))

    #define LSA_TRACE_11_EXT(_subsys, _level, modul_, line_, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_11(modul_ , line_, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8), (para9), (para10), (para11))

    #define LSA_TRACE_12_EXT(_subsys, _level, modul_, line_, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_12(modul_ , line_, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8), (para9), (para10), (para11), (para12))

    #define LSA_TRACE_13_EXT(_subsys, _level, modul_, line_, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_13(modul_ , line_, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8), (para9), (para10), (para11), (para12), (para13))

    #define LSA_TRACE_14_EXT(_subsys, _level, modul_, line_, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_14(modul_ , line_, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8), (para9), (para10), (para11), (para12), (para13), (para14))

    #define LSA_TRACE_15_EXT(_subsys, _level, modul_, line_, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14, para15) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_15(modul_ , line_, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8), (para9), (para10), (para11), (para12), (para13), (para14), (para15))

    #define LSA_TRACE_16_EXT(_subsys, _level, modul_, line_, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14, para15, para16) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_16(modul_ , line_, _subsys, _level, msg, (para1), (para2), (para3), (para4), (para5), (para6), (para7), (para8), (para9), (para10), (para11), (para12), (para13), (para14), (para15), (para16))

    #define LSA_TRACE_BYTE_ARRAY_EXT(_subsys, _level, modul_, line_, msg, ptr_, len_) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_BYTE_ARRAY (modul_, line_, _subsys, _level, msg, ptr_, len_)

    #define LSA_TRACE_STRING_EXT(_subsys, _level, modul_, line_, msg, st) \
            if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[_subsys].level >= _level)) \
              DK_TRACE_STRING_EXT(modul_, line_, _subsys, _level, msg, st)

#endif

#define        LTRC_LEVEL_TYPE            PNIO_UINT



#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
