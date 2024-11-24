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
/*  F i l e               &F: trace_lsa_utils.c                         :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  trace                                                                    */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/* contents:

    - trace_init
    - TRACE_ADD_HEADER
    - trace_add_entryNN

*/

/* include hierarchy */
#include "lsa_cfg.h"
#include "pnio_types.h"
#if (PNIO_TRACE == PNIO_TRACE_DK_LSA)
    #include "pndv_inc.h"
    #include "pnio_trace.h"
    #include "PnUsr_Api.h"
    #include "trace_dk.h"
    #include "trace_lsa_inc.h"
    #include "trace_lsa_com.h"
    #include "trace_lsa_dat.h"



    /* module ID */
    #define TRACE_MODULE_ID    143
    #define LTRC_ACT_MODUL_ID  143


    static 	PNIO_UINT8*		pTraceBuffer;
    static 	PNIO_UINT32 	TraceDataLeft;

    PNIO_UINT32 TraceLsaRdRecord (PNIO_UINT32   RecordIndex,
                                  PNIO_UINT32   *pBufLen,		// [in,out] length to read, out: length, read by user
	                              PNIO_UINT8    *pBuffer,  	    // [in] buffer pointer
                                  PNIO_ERR_STAT	*pPnioState 	// [in,out] return status, only modify if error
                                 )
    {
        PNIO_UINT32 Status = PNIO_OK;

	    // *----------------------------------------------*
	    // *  handle special indizes for I&M
	    // *----------------------------------------------*
        switch (RecordIndex)
        {
            case 0x7777:	/* get trace header */
            {
        	    PNIO_TRACE_BUFFER_PTR pTraceHead = (PNIO_TRACE_BUFFER_PTR) pBuffer;
        	    pTraceHead->Version 	= 'T'+('R'<<8)+('C'<<16)+('0'<<24);
        	    pTraceHead->Result  	= 0; // OK
        	    pTraceHead->BufferSize  = sizeof(tracebuffer);
        	    TRACE_MEMCPY(&pTraceHead->BufferData[0], "lsa_tracebuffer", sizeof("lsa_tracebuffer"));
        	    pTraceBuffer = (PNIO_UINT8*) &tracebuffer[0];
        	    TraceDataLeft = sizeof(tracebuffer);
                TRACE_MEMSET(pPnioState, 0, sizeof (PNIO_ERR_STAT));  // no error
        	    break;
            }

            case 0x7778:	/* get trace data */
            {
        	    PNIO_TRACE_BUFFER_PTR pTraceHead = (PNIO_TRACE_BUFFER_PTR) pBuffer;
        	    PNIO_UINT32 CopyLen = *pBufLen - 12; // -header
        	    if (TraceDataLeft < CopyLen)
        	    {
        		    CopyLen = TraceDataLeft;
        	    }
        	    pTraceHead->Version 	= 'T'+('R'<<8)+('C'<<16)+('1'<<24);
        	    pTraceHead->Result  	= 0; // OK
        	    pTraceHead->BufferSize  = CopyLen;
        	    TRACE_MEMCPY(&pTraceHead->BufferData, pTraceBuffer, CopyLen);
        	    pTraceBuffer += CopyLen;
        	    *pBufLen = CopyLen + 12; // +header
                TRACE_MEMSET(pPnioState, 0, sizeof (PNIO_ERR_STAT));  // no error
        	    break;
            }
            default:
            {
            	PNIO_Fatal();
            }
        }
        return (Status);
    }


    // *----------------------------------------------------------------*
    // *
    // *  TrcStoreBuf  ()
    // *
    // *----------------------------------------------------------------*
    // *  store trace buffer to file (WIN) or flash (NoneWIN) file system
    // *----------------------------------------------------------------*

    PNIO_UINT32 TrcStoreBuf    (PNIO_VOID)
    {
        PNIO_printf ("TrcStoreBuf() not supported on this platform\n");
        return (PNIO_NOT_OK);
    }


#endif


/*** end of file *************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
