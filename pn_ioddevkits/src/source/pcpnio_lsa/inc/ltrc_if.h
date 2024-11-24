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
/*  F i l e               &F: ltrc_if.h                                 :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*                       Frame for file "trc_if.h".                          */
/*                       ===========================                         */
/*                                                                           */
/*  Include-File:                                                            */
/*                                                                           */
/*  This file is prepared for subsystems will be abel to trace               */
/*  Includes of ltrc and of the different systems, compilers or              */
/*  operating system.                                                        */
/*                                                                           */
/*  This file has to be overwritten during system integration, because       */
/*  some includes depend on the different system, compiler or                */
/*  operating system.                                                        */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

#ifndef LTRC_IF_H                        /* ----- reinclude-protection ----- */
#define LTRC_IF_H


#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

/*===================================================================================================================*/
#include "compiler.h" 
#include "pnio_types.h"
#include "trc_if.h"
/*===================================================================================================================*/
/* >>> Define trace macros marking _idx as unused for GSY and PSI to prevent from a big number of warnings : <<<     */
/*===================================================================================================================*/

/*===================================================================================================================*/
/*===================================================================================================================*/
/*====   GSY   ======================================================================================================*/
/*===================================================================================================================*/
/*===================================================================================================================*/

#ifndef GSY_UPPER_TRACE_01
#define GSY_UPPER_TRACE_01(idx_, level_, msg_, para1_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_01(TRACE_SUBSYS_GSY_UPPER, level_, msg_, para1_); }
#endif
#ifndef GSY_UPPER_TRACE_02
#define GSY_UPPER_TRACE_02(idx_, level_, msg_, para1_, para2_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_02(TRACE_SUBSYS_GSY_UPPER, level_, msg_, para1_, para2_); }
#endif
#ifndef GSY_UPPER_TRACE_03
#define GSY_UPPER_TRACE_03(idx_, level_, msg_, para1_, para2_, para3_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_03(TRACE_SUBSYS_GSY_UPPER, level_, msg_, para1_, para2_, para3_); }
#endif
#ifndef GSY_UPPER_TRACE_04
#define GSY_UPPER_TRACE_04(idx_, level_, msg_, para1_, para2_, para3_, para4_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_04(TRACE_SUBSYS_GSY_UPPER, level_, msg_, para1_, para2_, para3_, para4_); }
#endif

#ifndef GSY_LOWER_TRACE_03
#define GSY_LOWER_TRACE_03(idx_, level_, msg_, para1_, para2_, para3_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_03(TRACE_SUBSYS_GSY_LOWER, level_, msg_, para1_, para2_, para3_); }
#endif

#ifndef GSY_SYSTEM_TRACE_00
#define GSY_SYSTEM_TRACE_00(idx_, level_, msg_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_00(TRACE_SUBSYS_GSY_SYSTEM, level_, msg_); }
#endif
#ifndef GSY_SYSTEM_TRACE_01
#define GSY_SYSTEM_TRACE_01(idx_, level_, msg_, para1_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_01(TRACE_SUBSYS_GSY_SYSTEM, level_, msg_, para1_); }
#endif
#ifndef GSY_SYSTEM_TRACE_02
#define GSY_SYSTEM_TRACE_02(idx_, level_, msg_, para1_, para2_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_02(TRACE_SUBSYS_GSY_SYSTEM, level_, msg_, para1_, para2_); }
#endif
#ifndef GSY_SYSTEM_TRACE_03
#define GSY_SYSTEM_TRACE_03(idx_, level_, msg_, para1_, para2_, para3_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_03(TRACE_SUBSYS_GSY_SYSTEM, level_, msg_, para1_, para2_, para3_); }
#endif
#ifndef GSY_SYSTEM_TRACE_04
#define GSY_SYSTEM_TRACE_04(idx_, level_, msg_, para1_, para2_, para3_, para4_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_04(TRACE_SUBSYS_GSY_SYSTEM, level_, msg_, para1_, para2_, para3_, para4_); }
#endif

#ifndef GSY_FUNCTION_TRACE_00
#define GSY_FUNCTION_TRACE_00(idx_, level_, msg_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_00(TRACE_SUBSYS_GSY_FUNCTION, level_, msg_); }
#endif
#ifndef GSY_FUNCTION_TRACE_01
#define GSY_FUNCTION_TRACE_01(idx_, level_, msg_, para1_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_01(TRACE_SUBSYS_GSY_FUNCTION, level_, msg_, para1_); }
#endif
#ifndef GSY_FUNCTION_TRACE_02
#define GSY_FUNCTION_TRACE_02(idx_, level_, msg_, para1_, para2_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_02(TRACE_SUBSYS_GSY_FUNCTION, level_, msg_, para1_, para2_); }
#endif
#ifndef GSY_FUNCTION_TRACE_03
#define GSY_FUNCTION_TRACE_03(idx_, level_, msg_, para1_, para2_, para3_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_03(TRACE_SUBSYS_GSY_FUNCTION, level_, msg_, para1_, para2_, para3_); }
#endif
#ifndef GSY_FUNCTION_TRACE_04
#define GSY_FUNCTION_TRACE_04(idx_, level_, msg_, para1_, para2_, para3_, para4_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_04(TRACE_SUBSYS_GSY_FUNCTION, level_, msg_, para1_, para2_, para3_, para4_); }
#endif

#ifndef GSY_ERROR_TRACE_00
#define GSY_ERROR_TRACE_00(idx_, level_, msg_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_00(TRACE_SUBSYS_GSY_ERROR, level_, msg_); }
#endif
#ifndef GSY_ERROR_TRACE_01
#define GSY_ERROR_TRACE_01(idx_, level_, msg_, para1_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_01(TRACE_SUBSYS_GSY_ERROR, level_, msg_, para1_); }
#endif
#ifndef GSY_ERROR_TRACE_02
#define GSY_ERROR_TRACE_02(idx_, level_, msg_, para1_, para2_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_02(TRACE_SUBSYS_GSY_ERROR, level_, msg_, para1_, para2_); }
#endif
#ifndef GSY_ERROR_TRACE_03
#define GSY_ERROR_TRACE_03(idx_, level_, msg_, para1_, para2_, para3_) { LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_03(TRACE_SUBSYS_GSY_ERROR, level_, msg_, para1_, para2_, para3_); }
#endif
#ifndef GSY_ERROR_TRACE_04
#define GSY_ERROR_TRACE_04(idx_, level_, msg_, para1_, para2_, para3_, para4_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_04(TRACE_SUBSYS_GSY_ERROR, level_, msg_, para1_, para2_, para3_, para4_); }
#endif

#ifndef GSY_DIAG_TRACE_04
#define GSY_DIAG_TRACE_04(idx_, level_, msg_, para1_, para2_, para3_, para4_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_04(TRACE_SUBSYS_GSY_DIAG, level_, msg_, para1_, para2_, para3_, para4_); }
#endif

#ifndef GSY_DEL_TRACE_08
#define GSY_DEL_TRACE_08(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_08(TRACE_SUBSYS_GSY_DEL, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_); }
#endif
#ifndef GSY_DEL_TRACE_09
#define GSY_DEL_TRACE_09(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_09(TRACE_SUBSYS_GSY_DEL, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_); }
#endif
#ifndef GSY_DEL_TRACE_10
#define GSY_DEL_TRACE_10(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_10(TRACE_SUBSYS_GSY_DEL, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_); }
#endif
#ifndef GSY_DEL_TRACE_11
#define GSY_DEL_TRACE_11(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_11(TRACE_SUBSYS_GSY_DEL, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_); }
#endif

#ifndef GSY_FWD_TRACE_00
#define GSY_FWD_TRACE_00(idx_, level_, msg_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_00(TRACE_SUBSYS_GSY_FWD, level_, msg_); }
#endif
#ifndef GSY_FWD_TRACE_01
#define GSY_FWD_TRACE_01(idx_, level_, msg_, para1_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_01(TRACE_SUBSYS_GSY_FWD, level_, msg_, para1_); }
#endif
#ifndef GSY_FWD_TRACE_02
#define GSY_FWD_TRACE_02(idx_, level_, msg_, para1_, para2_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_02(TRACE_SUBSYS_GSY_FWD, level_, msg_, para1_, para2_); }
#endif
#ifndef GSY_FWD_TRACE_03
#define GSY_FWD_TRACE_03(idx_, level_, msg_, para1_, para2_, para3_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_03(TRACE_SUBSYS_GSY_FWD, level_, msg_, para1_, para2_, para3_); }
#endif
#ifndef GSY_FWD_TRACE_04
#define GSY_FWD_TRACE_04(idx_, level_, msg_, para1_, para2_, para3_, para4_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_04(TRACE_SUBSYS_GSY_FWD, level_, msg_, para1_, para2_, para3_, para4_); }
#endif
#ifndef GSY_FWD_TRACE_05
#define GSY_FWD_TRACE_05(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_05(TRACE_SUBSYS_GSY_FWD, level_, msg_, para1_, para2_, para3_, para4_, para5_); }
#endif
#ifndef GSY_FWD_TRACE_06
#define GSY_FWD_TRACE_06(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_06(TRACE_SUBSYS_GSY_FWD, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_); }
#endif

#ifndef GSY_PRM_TRACE_00
#define GSY_PRM_TRACE_00(idx_, level_, msg_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_00(TRACE_SUBSYS_GSY_PRM, level_, msg_); }
#endif
#ifndef GSY_PRM_TRACE_01
#define GSY_PRM_TRACE_01(idx_, level_, msg_, para1_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_01(TRACE_SUBSYS_GSY_PRM, level_, msg_, para1_); }
#endif
#ifndef GSY_PRM_TRACE_02
#define GSY_PRM_TRACE_02(idx_, level_, msg_, para1_, para2_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_02(TRACE_SUBSYS_GSY_PRM, level_, msg_, para1_, para2_); }
#endif
#ifndef GSY_PRM_TRACE_03
#define GSY_PRM_TRACE_03(idx_, level_, msg_, para1_, para2_, para3_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_03(TRACE_SUBSYS_GSY_PRM, level_, msg_, para1_, para2_, para3_); }
#endif
#ifndef GSY_PRM_TRACE_04
#define GSY_PRM_TRACE_04(idx_, level_, msg_, para1_, para2_, para3_, para4_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_04(TRACE_SUBSYS_GSY_PRM, level_, msg_, para1_, para2_, para3_, para4_); }
#endif
#ifndef GSY_PRM_TRACE_05
#define GSY_PRM_TRACE_05(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_05(TRACE_SUBSYS_GSY_PRM, level_, msg_, para1_, para2_, para3_, para4_, para5_); }
#endif
#ifndef GSY_PRM_TRACE_06
#define GSY_PRM_TRACE_06(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_06(TRACE_SUBSYS_GSY_PRM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_); }
#endif
#ifndef GSY_PRM_TRACE_07
#define GSY_PRM_TRACE_07(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_07(TRACE_SUBSYS_GSY_PRM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_); }
#endif
#ifndef GSY_PRM_TRACE_08
#define GSY_PRM_TRACE_08(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_08(TRACE_SUBSYS_GSY_PRM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_); }
#endif
#ifndef GSY_PRM_TRACE_09
#define GSY_PRM_TRACE_09(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_09(TRACE_SUBSYS_GSY_PRM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_); }
#endif

#ifndef GSY_SYNC_TRACE_00
#define GSY_SYNC_TRACE_00(idx_, level_, msg_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_00(TRACE_SUBSYS_GSY_SYNC, level_, msg_); }
#endif
#ifndef GSY_SYNC_TRACE_01
#define GSY_SYNC_TRACE_01(idx_, level_, msg_, para1_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_01(TRACE_SUBSYS_GSY_SYNC, level_, msg_, para1_); }
#endif
#ifndef GSY_SYNC_TRACE_02
#define GSY_SYNC_TRACE_02(idx_, level_, msg_, para1_, para2_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_02(TRACE_SUBSYS_GSY_SYNC, level_, msg_, para1_, para2_); }
#endif
#ifndef GSY_SYNC_TRACE_03
#define GSY_SYNC_TRACE_03(idx_, level_, msg_, para1_, para2_, para3_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_03(TRACE_SUBSYS_GSY_SYNC, level_, msg_, para1_, para2_, para3_); }
#endif
#ifndef GSY_SYNC_TRACE_04
#define GSY_SYNC_TRACE_04(idx_, level_, msg_, para1_, para2_, para3_, para4_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_04(TRACE_SUBSYS_GSY_SYNC, level_, msg_, para1_, para2_, para3_, para4_); }
#endif
#ifndef GSY_SYNC_TRACE_05
#define GSY_SYNC_TRACE_05(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_05(TRACE_SUBSYS_GSY_SYNC, level_, msg_, para1_, para2_, para3_, para4_, para5_); }
#endif
#ifndef GSY_SYNC_TRACE_06
#define GSY_SYNC_TRACE_06(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_06(TRACE_SUBSYS_GSY_SYNC, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_); }
#endif
#ifndef GSY_SYNC_TRACE_07
#define GSY_SYNC_TRACE_07(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_07(TRACE_SUBSYS_GSY_SYNC, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_); }
#endif
#ifndef GSY_SYNC_TRACE_08
#define GSY_SYNC_TRACE_08(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_08(TRACE_SUBSYS_GSY_SYNC, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_); }
#endif
#ifndef GSY_SYNC_TRACE_09
#define GSY_SYNC_TRACE_09(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_09(TRACE_SUBSYS_GSY_SYNC, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_); }
#endif
#ifndef GSY_SYNC_TRACE_10
#define GSY_SYNC_TRACE_10(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_10(TRACE_SUBSYS_GSY_SYNC, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_); }
#endif

#ifndef GSY_MASTER_TRACE_00
#define GSY_MASTER_TRACE_00(idx_, level_, msg_) { LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_00(TRACE_SUBSYS_GSY_MASTER, level_, msg_); }
#endif
#ifndef GSY_MASTER_TRACE_01
#define GSY_MASTER_TRACE_01(idx_, level_, msg_, para1_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_01(TRACE_SUBSYS_GSY_MASTER, level_, msg_, para1_); }
#endif
#ifndef GSY_MASTER_TRACE_02
#define GSY_MASTER_TRACE_02(idx_, level_, msg_, para1_, para2_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_02(TRACE_SUBSYS_GSY_MASTER, level_, msg_, para1_, para2_); }
#endif
#ifndef GSY_MASTER_TRACE_03
#define GSY_MASTER_TRACE_03(idx_, level_, msg_, para1_, para2_, para3_) { LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_03(TRACE_SUBSYS_GSY_MASTER, level_, msg_, para1_, para2_, para3_); }
#endif
#ifndef GSY_MASTER_TRACE_04
#define GSY_MASTER_TRACE_04(idx_, level_, msg_, para1_, para2_, para3_, para4_) { LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_04(TRACE_SUBSYS_GSY_MASTER, level_, msg_, para1_, para2_, para3_, para4_); }
#endif
#ifndef GSY_MASTER_TRACE_05
#define GSY_MASTER_TRACE_05(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_05(TRACE_SUBSYS_GSY_MASTER, level_, msg_, para1_, para2_, para3_, para4_, para5_); }
#endif
#ifndef GSY_MASTER_TRACE_06
#define GSY_MASTER_TRACE_06(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_06(TRACE_SUBSYS_GSY_MASTER, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_); }
#endif
#ifndef GSY_MASTER_TRACE_07
#define GSY_MASTER_TRACE_07(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_07(TRACE_SUBSYS_GSY_MASTER, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_); }
#endif
#ifndef GSY_MASTER_TRACE_08
#define GSY_MASTER_TRACE_08(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_08(TRACE_SUBSYS_GSY_MASTER, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_); }
#endif
#ifndef GSY_MASTER_TRACE_09
#define GSY_MASTER_TRACE_09(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_09(TRACE_SUBSYS_GSY_MASTER, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_); }
#endif

#ifndef GSY_SNDRCV_TRACE_BYTE_ARRAY
#define GSY_SNDRCV_TRACE_BYTE_ARRAY(idx_, level_, msg_, ptr_, len_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_BYTE_ARRAY(TRACE_SUBSYS_GSY_SNDRCV, level_, msg_, ptr_, len_); }
#endif
#ifndef GSY_ERROR_TRACE_BYTE_ARRAY
#define GSY_ERROR_TRACE_BYTE_ARRAY(idx_, level_, msg_, ptr_, len_)	{ LSA_UNUSED_ARG(idx_) \
		LSA_TRACE_BYTE_ARRAY(TRACE_SUBSYS_GSY_ERROR, level_, msg_, ptr_, len_); }
#endif

/*===================================================================================================================*/
/*===================================================================================================================*/
/*====   PSI    =====================================================================================================*/
/*===================================================================================================================*/
/*===================================================================================================================*/

#ifndef PSI_SYSTEM_TRACE_00
#define PSI_SYSTEM_TRACE_00(idx_, level_, msg_){ LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_00(TRACE_SUBSYS_PSI_SYSTEM, level_, msg_); }
#endif

#ifndef PSI_SYSTEM_TRACE_01
#define PSI_SYSTEM_TRACE_01(idx_, level_, msg_, para1_){ LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_01(TRACE_SUBSYS_PSI_SYSTEM, level_, msg_, para1_); }
#endif

#ifndef PSI_SYSTEM_TRACE_02
#define PSI_SYSTEM_TRACE_02(idx_, level_, msg_, para1_, para2_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_02(TRACE_SUBSYS_PSI_SYSTEM, level_, msg_, para1_, para2_); }
#endif

#ifndef PSI_SYSTEM_TRACE_03
#define PSI_SYSTEM_TRACE_03(idx_, level_, msg_, para1_, para2_, para3_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_03(TRACE_SUBSYS_PSI_SYSTEM, level_, msg_, para1_, para2_, para3_); }
#endif

#ifndef PSI_SYSTEM_TRACE_04
#define PSI_SYSTEM_TRACE_04(idx_, level_, msg_, para1_, para2_, para3_, para4_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_04(TRACE_SUBSYS_PSI_SYSTEM, level_, msg_, para1_, para2_, para3_, para4_); }
#endif

#ifndef PSI_SYSTEM_TRACE_05
#define PSI_SYSTEM_TRACE_05(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_){ LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_05(TRACE_SUBSYS_PSI_SYSTEM, level_, msg_, para1_, para2_, para3_, para4_, para5_); }
#endif

#ifndef PSI_SYSTEM_TRACE_06
#define PSI_SYSTEM_TRACE_06(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_06(TRACE_SUBSYS_PSI_SYSTEM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_); }
#endif

#ifndef PSI_SYSTEM_TRACE_07
#define PSI_SYSTEM_TRACE_07(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_07(TRACE_SUBSYS_PSI_SYSTEM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_); }
#endif

#ifndef PSI_SYSTEM_TRACE_08
#define PSI_SYSTEM_TRACE_08(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_08(TRACE_SUBSYS_PSI_SYSTEM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_); }
#endif

#ifndef PSI_SYSTEM_TRACE_09
#define PSI_SYSTEM_TRACE_09(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_09(TRACE_SUBSYS_PSI_SYSTEM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_); }
#endif

#ifndef PSI_SYSTEM_TRACE_10
#define PSI_SYSTEM_TRACE_10(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_10(TRACE_SUBSYS_PSI_SYSTEM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_); }
#endif

#ifndef PSI_SYSTEM_TRACE_11
#define PSI_SYSTEM_TRACE_11(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_11(TRACE_SUBSYS_PSI_SYSTEM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_); }
#endif

#ifndef PSI_SYSTEM_TRACE_12
#define PSI_SYSTEM_TRACE_12(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_12(TRACE_SUBSYS_PSI_SYSTEM, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_); }
#endif


/*----------------------------------------------------------------------------*/
/* TRACE_SUBSYS_PSI_LD */

#ifndef PSI_LD_TRACE_00
#define PSI_LD_TRACE_00(idx_, level_, msg_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_00(TRACE_SUBSYS_PSI_LD, level_, msg_); }
#endif

#ifndef PSI_LD_TRACE_01
#define PSI_LD_TRACE_01(idx_, level_, msg_, para1_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_01(TRACE_SUBSYS_PSI_LD, level_, msg_, para1_); }
#endif

#ifndef PSI_LD_TRACE_02
#define PSI_LD_TRACE_02(idx_, level_, msg_, para1_, para2_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_02(TRACE_SUBSYS_PSI_LD, level_, msg_, para1_, para2_); }
#endif

#ifndef PSI_LD_TRACE_03
#define PSI_LD_TRACE_03(idx_, level_, msg_, para1_, para2_, para3_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_03(TRACE_SUBSYS_PSI_LD, level_, msg_, para1_, para2_, para3_); }
#endif

#ifndef PSI_LD_TRACE_04
#define PSI_LD_TRACE_04(idx_, level_, msg_, para1_, para2_, para3_, para4_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_04(TRACE_SUBSYS_PSI_LD, level_, msg_, para1_, para2_, para3_, para4_); }
#endif

#ifndef PSI_LD_TRACE_05
#define PSI_LD_TRACE_05(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_05(TRACE_SUBSYS_PSI_LD, level_, msg_, para1_, para2_, para3_, para4_, para5_); }
#endif

#ifndef PSI_LD_TRACE_06
#define PSI_LD_TRACE_06(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_06(TRACE_SUBSYS_PSI_LD, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_); }
#endif

#ifndef PSI_LD_TRACE_07
#define PSI_LD_TRACE_07(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_07(TRACE_SUBSYS_PSI_LD, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_); }
#endif

#ifndef PSI_LD_TRACE_08
#define PSI_LD_TRACE_08(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_08(TRACE_SUBSYS_PSI_LD, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_); }
#endif

#ifndef PSI_LD_TRACE_09
#define PSI_LD_TRACE_09(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_09(TRACE_SUBSYS_PSI_LD, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_); }
#endif

#ifndef PSI_LD_TRACE_10
#define PSI_LD_TRACE_10(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_10(TRACE_SUBSYS_PSI_LD, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_); }
#endif

#ifndef PSI_LD_TRACE_11
#define PSI_LD_TRACE_11(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_11(TRACE_SUBSYS_PSI_LD, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_); }
#endif

#ifndef PSI_LD_TRACE_12
#define PSI_LD_TRACE_12(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_12(TRACE_SUBSYS_PSI_LD, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_); }
#endif

/*----------------------------------------------------------------------------*/
/* TRACE_SUBSYS_PSI_HD */

#ifndef PSI_HD_TRACE_00
#define PSI_HD_TRACE_00(idx_, level_, msg_) { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_00(TRACE_SUBSYS_PSI_HD, level_, msg_); }
#endif

#ifndef PSI_HD_TRACE_01
#define PSI_HD_TRACE_01(idx_, level_, msg_, para1_)  { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_01(TRACE_SUBSYS_PSI_HD, level_, msg_, para1_); }
#endif

#ifndef PSI_HD_TRACE_02
#define PSI_HD_TRACE_02(idx_, level_, msg_, para1_, para2_)  { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_02(TRACE_SUBSYS_PSI_HD, level_, msg_, para1_, para2_); }
#endif

#ifndef PSI_HD_TRACE_03
#define PSI_HD_TRACE_03(idx_, level_, msg_, para1_, para2_, para3_)  { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_03(TRACE_SUBSYS_PSI_HD, level_, msg_, para1_, para2_, para3_); }
#endif

#ifndef PSI_HD_TRACE_04
#define PSI_HD_TRACE_04(idx_, level_, msg_, para1_, para2_, para3_, para4_)  { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_04(TRACE_SUBSYS_PSI_HD, level_, msg_, para1_, para2_, para3_, para4_); }
#endif

#ifndef PSI_HD_TRACE_05
#define PSI_HD_TRACE_05(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_)  { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_05(TRACE_SUBSYS_PSI_HD, level_, msg_, para1_, para2_, para3_, para4_, para5_); }
#endif

#ifndef PSI_HD_TRACE_06
#define PSI_HD_TRACE_06(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_)  { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_06(TRACE_SUBSYS_PSI_HD, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_); }
#endif

#ifndef PSI_HD_TRACE_07
#define PSI_HD_TRACE_07(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_)  { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_07(TRACE_SUBSYS_PSI_HD, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_); }
#endif

#ifndef PSI_HD_TRACE_08
#define PSI_HD_TRACE_08(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_)  { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_08(TRACE_SUBSYS_PSI_HD, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_); }
#endif

#ifndef PSI_HD_TRACE_09
#define PSI_HD_TRACE_09(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_)  { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_09(TRACE_SUBSYS_PSI_HD, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_); }
#endif

#ifndef PSI_HD_TRACE_10
#define PSI_HD_TRACE_10(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_)  { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_10(TRACE_SUBSYS_PSI_HD, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_); }
#endif

#ifndef PSI_HD_TRACE_11
#define PSI_HD_TRACE_11(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_)  { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_11(TRACE_SUBSYS_PSI_HD, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_); }
#endif

#ifndef PSI_HD_TRACE_12
#define PSI_HD_TRACE_12(idx_, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_)  { LSA_UNUSED_ARG(idx_) \
    LSA_TRACE_12(TRACE_SUBSYS_PSI_HD, level_, msg_, para1_, para2_, para3_, para4_, para5_, para6_, para7_, para8_, para9_, para10_, para11_, para12_); }
#endif



/*---------------------------------------------------------------------------*/
#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif  /* of LTRC_IF_H */


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
