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
/*  F i l e               &F: trace_con.h                               :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  declaration of the user defined callback functions                       */
/*                                                                           */
/*****************************************************************************/
#ifndef _TRACE_CON
#define _TRACE_CON
#include "pniousrd.h"
#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

    PNIO_UINT32     TrcInit	      (PNIO_UINT32 TrcBufSize);

    
    
    PNIO_VOID TrcArray (const PNIO_VOID* ptr, PNIO_UINT32 len);

	PNIO_VOID SaveTraceBuffer    (PNIO_VOID);
	PNIO_VOID RestoreTraceBuffer (PNIO_VOID);

    /*-------------------------------------------------------*/
    /*                2. function-logging                    */
    /*-------------------------------------------------------*/
    #define cm_log_get_fct		CM_LOG_GET_FCT      /* see ## below, new token is expanded after pasting! */
    #define acp_log_get_fct		ACP_LOG_GET_FCT     /* see ## below, new token is expanded after pasting! */
    #define clrpc_log_get_fct	CLRPC_LOG_GET_FCT   /* see ## below, new token is expanded after pasting! */
    #define dcp_log_get_fct	    DCP_LOG_GET_FCT     /* see ## below, new token is expanded after pasting! */
    #define edd_log_get_fct		EDD_LOG_GET_FCT
    #define gsy_log_get_fct		GSY_LOG_GET_FCT     /* see ## below, new token is expanded after pasting! */
    #define lldp_log_get_fct	LLDP_LOG_GET_FCT 
    #define mrp_log_get_fct	    MRP_LOG_GET_FCT 
    #define nare_log_get_fct	"nare_fct"
    #define oha_log_get_fct		OHA_LOG_GET_FCT
    #define pcsrt_log_get_fct	PCSRT_LOG_GET_FCT   /* see ## below, new token is expanded after pasting! */
    #define sock_log_get_fct    "sock_fct" 


    /*-------------------------------------------------------*/
    /*                3. TRACE MACROS                        */
    /*-------------------------------------------------------*/
    // *---------------------------------------------------
    // *  detailled LTRC information, memory extensive
    // *---------------------------------------------------
        #ifdef FILENAME_ENABLE_TRACE_CONSOLE
            #define FILENAME __FILE__
            #define PRINT_SUBSYS_LEVEL(_modid, _line, _subsys, _level) PNIO_printf("%s mod=%d line=%d Sub=%d Level=%d ",FILENAME, _modid, _line, _subsys, _level)
        #else
            #define PRINT_SUBSYS_LEVEL(_modid, _line, _subsys, _level) PNIO_printf("mod=%d line=%d Sub=%d Level=%d ", _modid, _line, _subsys, _level)
        #endif
        
        #define DK_TRACE_00(modid, line, subsys, level, msg) { \
                    PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
                    PNIO_printf (msg);  \
                    PNIO_printf ("\n"); }

        #define DK_TRACE_01(modid, line, subsys, level, msg, para1) { \
                    PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
                    PNIO_printf (msg, para1);  \
                    PNIO_printf ("\n"); }

        #define DK_TRACE_02(modid, line, subsys, level, msg, para1, para2) { \
                    PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
                    PNIO_printf (msg, para1, para2);  \
                    PNIO_printf ("\n"); }

        #define DK_TRACE_03(modid, line, subsys, level, msg, para1, para2, para3) { \
                    PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
                    PNIO_printf (msg, para1, para2, para3);  \
                    PNIO_printf ("\n"); }

        #define DK_TRACE_04(modid, line, subsys, level, msg, para1, para2, para3, para4) { \
                    PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
                    PNIO_printf (msg, para1, para2, para3, para4);  \
                    PNIO_printf ("\n"); }

        #define DK_TRACE_05(modid, line, subsys, level, msg, para1, para2, para3, para4, para5) { \
                    PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
                    PNIO_printf (msg, para1, para2, para3, para4, para5);  \
                    PNIO_printf ("\n"); }

        #define DK_TRACE_06(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6) { \
                    PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
                    PNIO_printf (msg, para1, para2, para3, para4, para5, para6);  \
                    PNIO_printf ("\n"); }

        #define DK_TRACE_07(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7) { \
                    PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
                    PNIO_printf (msg, para1, para2, para3, para4, para5, para6, para7);  \
                    PNIO_printf ("\n"); }

        #define DK_TRACE_08(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8) { \
                    PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
                    PNIO_printf (msg, para1, para2, para3, para4, para5, para6, para7, para8);  \
                    PNIO_printf ("\n"); }

        #define DK_TRACE_09(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9) { \
                    PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
                    PNIO_printf (msg, para1, para2, para3, para4, para5, para6, para7, para8, para9);  \
                    PNIO_printf ("\n"); }

        #define DK_TRACE_10(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10) { \
                    PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
                    PNIO_printf (msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10);  \
                    PNIO_printf ("\n"); }

        #define DK_TRACE_11(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11) { \
                    PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
                    PNIO_printf (msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11);  \
                    PNIO_printf ("\n"); }

        #define DK_TRACE_12(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12) { \
                    PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
                    PNIO_printf (msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12);  \
                    PNIO_printf ("\n"); }

		#define DK_TRACE_13(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13) { \
					PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
					PNIO_printf (msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13);  \
					PNIO_printf ("\n"); }

		#define DK_TRACE_14(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14) { \
					PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
					PNIO_printf (msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14);  \
					PNIO_printf ("\n"); }

		#define DK_TRACE_15(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14, para15) { \
					PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
					PNIO_printf (msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14, para15);  \
					PNIO_printf ("\n"); }

		#define DK_TRACE_16(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14, para15, para16) { \
					PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
					PNIO_printf (msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14, para15, para16);  \
					PNIO_printf ("\n"); }

        #define DK_TRACE_BYTE_ARRAY(modid, line, subsys, level, msg, ptr, len) { \
                    PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
                    PNIO_printf (msg); \
                    TrcArray (ptr, len);  \
                    PNIO_printf ("\n"); }

		#define DK_TRACE_STRING_EXT(modid, line, subsys, level, msg, st) { \
					PRINT_SUBSYS_LEVEL(modid, line, subsys,level); \
					PNIO_printf (msg, st);  \
					PNIO_printf ("\n"); }

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
