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
/*  F i l e               &F: pnpb_trc.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  trace subsystems:                                                        */
/*                                                                           */
/*    TRACE_SUBSYS_PNPB_SYS                                                  */
/*    TRACE_SUBSYS_PNPB_API                                                  */
/*    TRACE_SUBSYS_PNPB_PERIF                                                */
/*    TRACE_SUBSYS_PNPB_PLATFORM                                             */
/*                                                                           */
/*****************************************************************************/
#ifndef PNPB_TRACE_H
#define PNPB_TRACE_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif


typedef enum _PNPB_TRACE_ACTION
{
	PNPB_TRACE_INIT_VALUE,
	PNPB_TRACE_INIT,
	PNPB_TRACE_WRITE_EVENT,
	PNPB_TRACE_READ_EVENT,
	PNPB_TRACE_DO_SERVICE,
	PNPB_TRACE_CHANGE_AR_STATE,
	PNPB_TRACE_CHANGE_PERI_STATE,
	PNPB_TRACE_CHANGE_DATA_STATE,
	PNPB_TRACE_FATAL_ERROR
} PNPB_TRACE_ACTION;


/*----------------------------------------------------------------------------*/
/* TRACE_SUBSYS_PNPB_SYSADAPT */
/*----------------------------------------------------------------------------*/
#ifndef PNPB_SYS_TRACE_00
	#define PNPB_SYS_TRACE_00(level_, msg_) \
	LSA_TRACE_00(TRACE_SUBSYS_PNPB_SYSADAPT, level_, msg_)
#endif

#ifndef PNPB_SYS_TRACE_01
	#define PNPB_SYS_TRACE_01(level_, msg_, para1_) \
	LSA_TRACE_01(TRACE_SUBSYS_PNPB_SYSADAPT, level_, msg_, para1_)
#endif

#ifndef PNPB_SYS_TRACE_02
	#define PNPB_SYS_TRACE_02(level_, msg_, para1_, para2_) \
	LSA_TRACE_02(TRACE_SUBSYS_PNPB_SYSADAPT, level_, msg_, para1_, para2_)
#endif

#ifndef PNPB_SYS_TRACE_03
	#define PNPB_SYS_TRACE_03(level_, msg_, para1_, para2_, para3_) \
	LSA_TRACE_03(TRACE_SUBSYS_PNPB_SYSADAPT, level_, msg_, para1_, para2_, para3_)
#endif

#ifndef PNPB_SYS_TRACE_04
	#define PNPB_SYS_TRACE_04(level_, msg_, para1_, para2_, para3_, para4_) \
	LSA_TRACE_04(TRACE_SUBSYS_PNPB_SYSADAPT, level_, msg_, para1_, para2_, para3_, para4_)
#endif

#ifndef PNPB_SYS_TRACE_05
	#define PNPB_SYS_TRACE_05(level_, msg_, para1_, para2_, para3_, para4_, para5_) \
	LSA_TRACE_05(TRACE_SUBSYS_PNPB_SYSADAPT, level_, msg_, para1_, para2_, para3_, para4_, para5_)
#endif

#ifndef PNPB_SYS_TRACE_06
	#define PNPB_SYS_TRACE_06(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_) \
	LSA_TRACE_06(TRACE_SUBSYS_PNPB_SYSADAPT, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_)
#endif

#ifndef PNPB_SYS_TRACE_07
	#define PNPB_SYS_TRACE_07(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_) \
	LSA_TRACE_07(TRACE_SUBSYS_PNPB_SYSADAPT, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_)
#endif

#ifndef PNPB_SYS_TRACE_08
	#define PNPB_SYS_TRACE_08(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_) \
	LSA_TRACE_08(TRACE_SUBSYS_PNPB_SYSADAPT, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_)
#endif

#ifndef PNPB_SYS_TRACE_09
	#define PNPB_SYS_TRACE_09(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_) \
	LSA_TRACE_09(TRACE_SUBSYS_PNPB_SYSADAPT, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_)
#endif

#ifndef PNPB_SYS_TRACE_10
	#define PNPB_SYS_TRACE_10(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_) \
	LSA_TRACE_10(TRACE_SUBSYS_PNPB_SYSADAPT, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_)
#endif

#ifndef PNPB_SYS_TRACE_11
	#define PNPB_SYS_TRACE_11(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_) \
	LSA_TRACE_11(TRACE_SUBSYS_PNPB_SYSADAPT, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_)
#endif

#ifndef PNPB_SYS_TRACE_12
	#define PNPB_SYS_TRACE_12(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_) \
	LSA_TRACE_12(TRACE_SUBSYS_PNPB_SYSADAPT, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_)
#endif


/*----------------------------------------------------------------------------*/
/* TRACE_SUBSYS_PNPB_API */
/*----------------------------------------------------------------------------*/

#ifndef PNPB_API_TRACE_00
	#define PNPB_API_TRACE_00(level_, msg_) \
	LSA_TRACE_00(TRACE_SUBSYS_PNPB_API, level_, msg_)
#endif

#ifndef PNPB_API_TRACE_01
	#define PNPB_API_TRACE_01(level_, msg_, para1_) \
	LSA_TRACE_01(TRACE_SUBSYS_PNPB_API, level_, msg_, para1_)
#endif

#ifndef PNPB_API_TRACE_02
	#define PNPB_API_TRACE_02(level_, msg_, para1_, para2_) \
	LSA_TRACE_02(TRACE_SUBSYS_PNPB_API, level_, msg_, para1_, para2_)
#endif

#ifndef PNPB_API_TRACE_03
	#define PNPB_API_TRACE_03(level_, msg_, para1_, para2_, para3_) \
	LSA_TRACE_03(TRACE_SUBSYS_PNPB_API, level_, msg_, para1_, para2_, para3_)
#endif

#ifndef PNPB_API_TRACE_04
	#define PNPB_API_TRACE_04(level_, msg_, para1_, para2_, para3_, para4_) \
	LSA_TRACE_04(TRACE_SUBSYS_PNPB_API, level_, msg_, para1_, para2_, para3_, para4_)
#endif

#ifndef PNPB_API_TRACE_05
	#define PNPB_API_TRACE_05(level_, msg_, para1_, para2_, para3_, para4_, para5_) \
	LSA_TRACE_05(TRACE_SUBSYS_PNPB_API, level_, msg_, para1_, para2_, para3_, para4_, para5_)
#endif

#ifndef PNPB_API_TRACE_06
	#define PNPB_API_TRACE_06(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_) \
	LSA_TRACE_06(TRACE_SUBSYS_PNPB_API, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_)
#endif

#ifndef PNPB_API_TRACE_07
	#define PNPB_API_TRACE_07(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_) \
	LSA_TRACE_07(TRACE_SUBSYS_PNPB_API, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_)
#endif

#ifndef PNPB_API_TRACE_08
	#define PNPB_API_TRACE_08(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_) \
	LSA_TRACE_08(TRACE_SUBSYS_PNPB_API, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_)
#endif

#ifndef PNPB_API_TRACE_09
	#define PNPB_API_TRACE_09(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_) \
	LSA_TRACE_09(TRACE_SUBSYS_PNPB_API, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_)
#endif

#ifndef PNPB_API_TRACE_10
	#define PNPB_API_TRACE_10(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_) \
	LSA_TRACE_10(TRACE_SUBSYS_PNPB_API, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_)
#endif

#ifndef PNPB_API_TRACE_11
	#define PNPB_API_TRACE_11(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_) \
	LSA_TRACE_11(TRACE_SUBSYS_PNPB_API, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_)
#endif

#ifndef PNPB_API_TRACE_12
	#define PNPB_API_TRACE_12(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_) \
	LSA_TRACE_12(TRACE_SUBSYS_PNPB_API, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_)
#endif


/*----------------------------------------------------------------------------*/
/* TRACE_SUBSYS_PNPB_PERIF */
/*----------------------------------------------------------------------------*/

#ifndef PNPB_PERIF_TRACE_00
	#define PNPB_PERIF_TRACE_00(level_, msg_) \
	LSA_TRACE_00(TRACE_SUBSYS_PNPB_PERIF, level_, msg_)
#endif

#ifndef PNPB_PERIF_TRACE_01
	#define PNPB_PERIF_TRACE_01(level_, msg_, para1_) \
	LSA_TRACE_01(TRACE_SUBSYS_PNPB_PERIF, level_, msg_, para1_)
#endif

#ifndef PNPB_PERIF_TRACE_02
	#define PNPB_PERIF_TRACE_02(level_, msg_, para1_, para2_) \
	LSA_TRACE_02(TRACE_SUBSYS_PNPB_PERIF, level_, msg_, para1_, para2_)
#endif

#ifndef PNPB_PERIF_TRACE_03
	#define PNPB_PERIF_TRACE_03(level_, msg_, para1_, para2_, para3_) \
	LSA_TRACE_03(TRACE_SUBSYS_PNPB_PERIF, level_, msg_, para1_, para2_, para3_)
#endif

#ifndef PNPB_PERIF_TRACE_04
	#define PNPB_PERIF_TRACE_04(level_, msg_, para1_, para2_, para3_, para4_) \
	LSA_TRACE_04(TRACE_SUBSYS_PNPB_PERIF, level_, msg_, para1_, para2_, para3_, para4_)
#endif

#ifndef PNPB_PERIF_TRACE_05
	#define PNPB_PERIF_TRACE_05(level_, msg_, para1_, para2_, para3_, para4_, para5_) \
	LSA_TRACE_05(TRACE_SUBSYS_PNPB_PERIF, level_, msg_, para1_, para2_, para3_, para4_, para5_)
#endif

#ifndef PNPB_PERIF_TRACE_06
	#define PNPB_PERIF_TRACE_06(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_) \
	LSA_TRACE_06(TRACE_SUBSYS_PNPB_PERIF, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_)
#endif

#ifndef PNPB_PERIF_TRACE_07
	#define PNPB_PERIF_TRACE_07(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_) \
	LSA_TRACE_07(TRACE_SUBSYS_PNPB_PERIF, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_)
#endif

#ifndef PNPB_PERIF_TRACE_08
	#define PNPB_PERIF_TRACE_08(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_) \
	LSA_TRACE_08(TRACE_SUBSYS_PNPB_PERIF, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_)
#endif

#ifndef PNPB_PERIF_TRACE_09
	#define PNPB_PERIF_TRACE_09(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_) \
	LSA_TRACE_09(TRACE_SUBSYS_PNPB_PERIF, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_)
#endif

#ifndef PNPB_PERIF_TRACE_10
	#define PNPB_PERIF_TRACE_10(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_) \
	LSA_TRACE_10(TRACE_SUBSYS_PNPB_PERIF, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_)
#endif

#ifndef PNPB_PERIF_TRACE_11
	#define PNPB_PERIF_TRACE_11(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_) \
	LSA_TRACE_11(TRACE_SUBSYS_PNPB_PERIF, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_)
#endif

#ifndef PNPB_PERIF_TRACE_12
	#define PNPB_PERIF_TRACE_12(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_) \
	LSA_TRACE_12(TRACE_SUBSYS_PNPB_PERIF, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_)
#endif


/*----------------------------------------------------------------------------*/
/* TRACE_SUBSYS_PNPB_PLATFORM */
/*----------------------------------------------------------------------------*/

#ifndef PNPB_PLATFORM_TRACE_00
	#define PNPB_PLATFORM_TRACE_00(level_, msg_) \
	LSA_TRACE_00(TRACE_SUBSYS_PNPB_PLATFORM, level_, msg_)
#endif

#ifndef PNPB_PLATFORM_TRACE_01
	#define PNPB_PLATFORM_TRACE_01(level_, msg_, para1_) \
	LSA_TRACE_01(TRACE_SUBSYS_PNPB_PLATFORM, level_, msg_, para1_)
#endif

#ifndef PNPB_PLATFORM_TRACE_02
	#define PNPB_PLATFORM_TRACE_02(level_, msg_, para1_, para2_) \
	LSA_TRACE_02(TRACE_SUBSYS_PNPB_PLATFORM, level_, msg_, para1_, para2_)
#endif

#ifndef PNPB_PLATFORM_TRACE_03
	#define PNPB_PLATFORM_TRACE_03(level_, msg_, para1_, para2_, para3_) \
	LSA_TRACE_03(TRACE_SUBSYS_PNPB_PLATFORM, level_, msg_, para1_, para2_, para3_)
#endif

#ifndef PNPB_PLATFORM_TRACE_04
	#define PNPB_PLATFORM_TRACE_04(level_, msg_, para1_, para2_, para3_, para4_) \
	LSA_TRACE_04(TRACE_SUBSYS_PNPB_PLATFORM, level_, msg_, para1_, para2_, para3_, para4_)
#endif

#ifndef PNPB_PLATFORM_TRACE_05
	#define PNPB_PLATFORM_TRACE_05(level_, msg_, para1_, para2_, para3_, para4_, para5_) \
	LSA_TRACE_05(TRACE_SUBSYS_PNPB_PLATFORM, level_, msg_, para1_, para2_, para3_, para4_, para5_)
#endif

#ifndef PNPB_PLATFORM_TRACE_06
	#define PNPB_PLATFORM_TRACE_06(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_) \
	LSA_TRACE_06(TRACE_SUBSYS_PNPB_PLATFORM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_)
#endif

#ifndef PNPB_PLATFORM_TRACE_07
	#define PNPB_PLATFORM_TRACE_07(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_) \
	LSA_TRACE_07(TRACE_SUBSYS_PNPB_PLATFORM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_)
#endif

#ifndef PNPB_PLATFORM_TRACE_08
	#define PNPB_PLATFORM_TRACE_08(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_) \
	LSA_TRACE_08(TRACE_SUBSYS_PNPB_PLATFORM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_)
#endif

#ifndef PNPB_PLATFORM_TRACE_09
	#define PNPB_PLATFORM_TRACE_09(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_) \
	LSA_TRACE_09(TRACE_SUBSYS_PNPB_PLATFORM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_)
#endif

#ifndef PNPB_PLATFORM_TRACE_10
	#define PNPB_PLATFORM_TRACE_10(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_) \
	LSA_TRACE_10(TRACE_SUBSYS_PNPB_PLATFORM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_)
#endif

#ifndef PNPB_PLATFORM_TRACE_11
	#define PNPB_PLATFORM_TRACE_11(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_) \
	LSA_TRACE_11(TRACE_SUBSYS_PNPB_PLATFORM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_)
#endif

#ifndef PNPB_PLATFORM_TRACE_12
	#define PNPB_PLATFORM_TRACE_12(level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_) \
	LSA_TRACE_12(TRACE_SUBSYS_PNPB_PLATFORM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_)
#endif




#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
