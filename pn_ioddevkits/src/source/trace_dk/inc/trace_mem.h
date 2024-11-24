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
/*  F i l e               &F: trace_mem.h                               :F&  */
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
#ifndef _TRACE_MEM_H
#define _TRACE_MEM_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif


    PNIO_UINT32    TrcInit	    (PNIO_UINT32 TrcBufSize);

    PNIO_UINT32    TrcTraceMsgStore (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level,
                                     PNIO_INT8 * pMsgBuf, PNIO_UINT32 StrLen);

    PNIO_UINT32    TrcPrintMsgStore (PNIO_INT8 * fmt, PNIO_va_list argptr);

    PNIO_VOID      TrcMsg2Mem (PNIO_INT8* pMsgBuf, PNIO_UINT32 MsgSize);


    PNIO_UINT8*    TrcGetBufPtr    (PNIO_VOID);
    PNIO_UINT32    TrcGetTotalSize (PNIO_VOID);
    PNIO_UINT32    TrcGetFillStart (PNIO_VOID);
    PNIO_UINT32    TrcGetFillSize  (PNIO_VOID);
    PNIO_UINT32    TrcStoreBuf     (PNIO_VOID);

    PNIO_VOID      SaveTraceBuffer    (PNIO_VOID);
    PNIO_VOID      RestoreTraceBuffer (PNIO_VOID);


    PNIO_VOID dk_trace_array (PNIO_UINT32 ModId, 
                              PNIO_UINT32 Line, 
                              PNIO_UINT32 subsys, 
                              PNIO_UINT32 level, 
                              PNIO_UINT8* ptr, PNIO_UINT32 len);


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
    PNIO_VOID dk_trace_00 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level, PNIO_VOID* msg);

    PNIO_VOID dk_trace_01 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level, PNIO_VOID* msg, 
                           PNIO_UINT32 para1);

    PNIO_VOID dk_trace_02 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level, PNIO_VOID* msg, 
                           PNIO_UINT32 para1, PNIO_UINT32 para2);

    PNIO_VOID dk_trace_03 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level, PNIO_VOID* msg, 
                           PNIO_UINT32 para1, PNIO_UINT32 para2, PNIO_UINT32 para3);

    PNIO_VOID dk_trace_04 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level, PNIO_VOID* msg, 
                           PNIO_UINT32 para1, PNIO_UINT32 para2, PNIO_UINT32 para3, PNIO_UINT32 para4);

    PNIO_VOID dk_trace_05 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg, 
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4, 
                           PNIO_UINT32 para5);

    PNIO_VOID dk_trace_06 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg, 
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4, 
                           PNIO_UINT32 para5,  PNIO_UINT32 para6);

    PNIO_VOID dk_trace_07 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg, 
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4, 
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7); 

    PNIO_VOID dk_trace_08 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg, 
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4, 
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8); 


    PNIO_VOID dk_trace_09 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg, 
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4, 
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8, 
                           PNIO_UINT32 para9); 


    PNIO_VOID dk_trace_10 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg, 
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4, 
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8, 
                           PNIO_UINT32 para9,  PNIO_UINT32 para10); 

    PNIO_VOID dk_trace_11 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg, 
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4, 
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8, 
                           PNIO_UINT32 para9,  PNIO_UINT32 para10, PNIO_UINT32 para11); 

    PNIO_VOID dk_trace_12 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg, 
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4, 
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8, 
                           PNIO_UINT32 para9,  PNIO_UINT32 para10, PNIO_UINT32 para11, PNIO_UINT32 para12); 

    PNIO_VOID dk_trace_13 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg, 
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4, 
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8, 
                           PNIO_UINT32 para9,  PNIO_UINT32 para10, PNIO_UINT32 para11, PNIO_UINT32 para12, 
                           PNIO_UINT32 para13); 


    PNIO_VOID dk_trace_14 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg, 
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4, 
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8, 
                           PNIO_UINT32 para9,  PNIO_UINT32 para10, PNIO_UINT32 para11, PNIO_UINT32 para12, 
                           PNIO_UINT32 para13,  PNIO_UINT32 para14); 


    PNIO_VOID dk_trace_15 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg, 
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4, 
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8, 
                           PNIO_UINT32 para9,  PNIO_UINT32 para10, PNIO_UINT32 para11, PNIO_UINT32 para12, 
                           PNIO_UINT32 para13,  PNIO_UINT32 para14,  PNIO_UINT32 para15); 


    PNIO_VOID dk_trace_16 (PNIO_UINT32 modid, PNIO_UINT32 line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg, 
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4, 
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8, 
                           PNIO_UINT32 para9,  PNIO_UINT32 para10, PNIO_UINT32 para11, PNIO_UINT32 para12, 
                           PNIO_UINT32 para13,  PNIO_UINT32 para14,  PNIO_UINT32 para15,  PNIO_UINT32 para16); 

                     
    // *---------------------------------------------------
    // *  detailled LTRC information, memory extensive
    // *---------------------------------------------------
        #define PRINT_SUBSYS_LEVEL(_subsys, _level) PNIO_printf("Sub=%d Level=%d ", _subsys, _level)

        #define DK_TRACE_00(modid, line, subsys, level, msg) { \
                        dk_trace_00(modid, line, subsys, level, msg); }

        #define DK_TRACE_01(modid, line, subsys, level, msg, para1) { \
                        dk_trace_01(modid, line, subsys, level, msg, \
                                (PNIO_UINT32) para1); }

        #define DK_TRACE_02(modid, line, subsys, level, msg, para1, para2) { \
                        dk_trace_02(modid, line, subsys, level, msg, \
                                (PNIO_UINT32) para1, (PNIO_UINT32) para2); }

        #define DK_TRACE_03(modid, line, subsys, level, msg, para1, para2, para3) { \
                        dk_trace_03(modid, line, subsys, level, msg, \
                                (PNIO_UINT32) para1, (PNIO_UINT32) para2, \
                                (PNIO_UINT32) para3); }

        #define DK_TRACE_04(modid, line, subsys, level, msg, para1, para2, para3, para4) { \
                        dk_trace_04(modid, line, subsys, level, msg, \
                                (PNIO_UINT32) para1, (PNIO_UINT32) para2, \
                                (PNIO_UINT32) para3, (PNIO_UINT32) para4); }

        #define DK_TRACE_05(modid, line, subsys, level, msg, para1, para2, para3, para4, para5) { \
                        dk_trace_05(modid, line, subsys, level, msg, \
                                (PNIO_UINT32) para1, (PNIO_UINT32) para2,  (PNIO_UINT32) para3,  (PNIO_UINT32) para4, \
                                (PNIO_UINT32) para5); }

        #define DK_TRACE_06(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6) { \
                        dk_trace_06(modid, line, subsys, level, msg, \
                                (PNIO_UINT32) para1, (PNIO_UINT32) para2,  (PNIO_UINT32) para3,  (PNIO_UINT32) para4, \
                                (PNIO_UINT32) para5, (PNIO_UINT32) para6); }

        #define DK_TRACE_07(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7) { \
                        dk_trace_07(modid, line, subsys, level, msg, \
                                (PNIO_UINT32) para1, (PNIO_UINT32) para2,  (PNIO_UINT32) para3,  (PNIO_UINT32) para4, \
                                (PNIO_UINT32) para5, (PNIO_UINT32) para6,  (PNIO_UINT32) para7); }

        #define DK_TRACE_08(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8) { \
                        dk_trace_08(modid, line, subsys, level, msg, \
                                (PNIO_UINT32) para1, (PNIO_UINT32) para2,  (PNIO_UINT32) para3,  (PNIO_UINT32) para4, \
                                (PNIO_UINT32) para5, (PNIO_UINT32) para6,  (PNIO_UINT32) para7,  (PNIO_UINT32) para8); }

        #define DK_TRACE_09(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9) { \
                        dk_trace_09(modid, line, subsys, level, msg, \
                                (PNIO_UINT32) para1, (PNIO_UINT32) para2,  (PNIO_UINT32) para3,  (PNIO_UINT32) para4, \
                                (PNIO_UINT32) para5, (PNIO_UINT32) para6,  (PNIO_UINT32) para7,  (PNIO_UINT32) para8, \
                                (PNIO_UINT32) para9); }

        #define DK_TRACE_10(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10) { \
                        dk_trace_10(modid, line, subsys, level, msg, \
                                (PNIO_UINT32) para1, (PNIO_UINT32) para2,  (PNIO_UINT32) para3,  (PNIO_UINT32) para4, \
                                (PNIO_UINT32) para5, (PNIO_UINT32) para6,  (PNIO_UINT32) para7,  (PNIO_UINT32) para8, \
                                (PNIO_UINT32) para9, (PNIO_UINT32) para10); }

        #define DK_TRACE_11(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11) { \
                        dk_trace_11(modid, line, subsys, level, msg, \
                                (PNIO_UINT32) para1, (PNIO_UINT32) para2,  (PNIO_UINT32) para3,  (PNIO_UINT32) para4, \
                                (PNIO_UINT32) para5, (PNIO_UINT32) para6,  (PNIO_UINT32) para7,  (PNIO_UINT32) para8, \
                                (PNIO_UINT32) para9, (PNIO_UINT32) para10, (PNIO_UINT32) para11); }

        #define DK_TRACE_12(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12) { \
                        dk_trace_12(modid, line, subsys, level, msg, \
                                (PNIO_UINT32) para1, (PNIO_UINT32) para2,  (PNIO_UINT32) para3,  (PNIO_UINT32) para4, \
                                (PNIO_UINT32) para5, (PNIO_UINT32) para6,  (PNIO_UINT32) para7,  (PNIO_UINT32) para8, \
                                (PNIO_UINT32) para9, (PNIO_UINT32) para10, (PNIO_UINT32) para11, (PNIO_UINT32) para12); }

        #define DK_TRACE_13(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13) { \
                        dk_trace_13(modid, line, subsys, level, msg, \
                                (PNIO_UINT32) para1, (PNIO_UINT32) para2,  (PNIO_UINT32) para3,  (PNIO_UINT32) para4, \
                                (PNIO_UINT32) para5, (PNIO_UINT32) para6,  (PNIO_UINT32) para7,  (PNIO_UINT32) para8, \
                                (PNIO_UINT32) para9, (PNIO_UINT32) para10, (PNIO_UINT32) para11, (PNIO_UINT32) para12, \
                                (PNIO_UINT32) para13); }
        
        #define DK_TRACE_14(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14) { \
                        dk_trace_14(modid, line, subsys, level, msg, \
                                (PNIO_UINT32) para1, (PNIO_UINT32) para2,  (PNIO_UINT32) para3,  (PNIO_UINT32) para4, \
                                (PNIO_UINT32) para5, (PNIO_UINT32) para6,  (PNIO_UINT32) para7,  (PNIO_UINT32) para8, \
                                (PNIO_UINT32) para9, (PNIO_UINT32) para10, (PNIO_UINT32) para11, (PNIO_UINT32) para12, \
                                (PNIO_UINT32) para13, (PNIO_UINT32) para14); }
        
        #define DK_TRACE_15(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14, para15) { \
                        dk_trace_15(modid, line, subsys, level, msg, \
                                (PNIO_UINT32) para1, (PNIO_UINT32) para2,  (PNIO_UINT32) para3,  (PNIO_UINT32) para4, \
                                (PNIO_UINT32) para5, (PNIO_UINT32) para6,  (PNIO_UINT32) para7,  (PNIO_UINT32) para8, \
                                (PNIO_UINT32) para9, (PNIO_UINT32) para10, (PNIO_UINT32) para11, (PNIO_UINT32) para12, \
                                (PNIO_UINT32) para13, (PNIO_UINT32) para14, (PNIO_UINT32) para15); }
        
        #define DK_TRACE_16(modid, line, subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14, para15, para16) { \
                        dk_trace_16(modid, line, subsys, level, msg, \
                                (PNIO_UINT32) para1, (PNIO_UINT32) para2,  (PNIO_UINT32) para3,  (PNIO_UINT32) para4, \
                                (PNIO_UINT32) para5, (PNIO_UINT32) para6,  (PNIO_UINT32) para7,  (PNIO_UINT32) para8, \
                                (PNIO_UINT32) para9, (PNIO_UINT32) para10, (PNIO_UINT32) para11, (PNIO_UINT32) para12, \
                                (PNIO_UINT32) para13, (PNIO_UINT32) para14, (PNIO_UINT32) para15, (PNIO_UINT32) para16); }


        #define DK_TRACE_BYTE_ARRAY(modid, line, subsys, level, msg, ptr, len) { \
                        dk_trace_array (modid, line, subsys, level, (PNIO_UINT8* )ptr, len);  }


#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
