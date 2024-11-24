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
/*  F i l e               &F: trace_lsa_tcp.c                           :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Usr functions, called by pnio stack in case of events and alarms         */
/*                                                                           */
/*  THIS MODULE HAS TO BE MODIFIED BY THE PNIO USER                          */
/*                                                                           */
/*****************************************************************************/

#include "compiler.h"
#include "usriod_cfg.h"

#if (PNIO_TRACE == PNIO_TRACE_DK_LSA)

    #include "pniousrd.h"
    #include "os.h"
    #include "nv_data.h"
    #include "bspadapt.h"
    #include "usriod_utils.h"
    #include "tcp_if.h"
    //#include "trace_mem.h"
	#include "trace_dk.h"

	#include "trace_lsa_inc.h"
	#include "trace_lsa_com.h"
	#include "trace_lsa_dat.h"

    #define FLASH_TRCBUF_START_ADR     0x30d00000     /* Flash start address in virtual memory */
    #define FLASH_TRCBUF_SIZE          0x00020000     /* 128 MB excluding the last 64k sector */

    #define TCP_TRC_UPLOAD_PORTNUMBER    998

    #define MAX_BLOCK_SIZE 40000

    typedef struct 
    {
        PNIO_INT StartTag;
        PNIO_INT Command;
    }   TCP_COMMAND_HEADER;


    static PNIO_INT LocSockId, RemSockId;


    /***************************************************************************/
    PNIO_VOID TcpTraceUploadBuffer (PNIO_UINT8* pBuf, PNIO_UINT32 bufSize)
    {
    	PNIO_UINT32 RestBytes = bufSize;
    	PNIO_UINT32 RestBytes_htonl  = TRACE_HTONL(RestBytes);
        PNIO_UINT32 BufOffset        = 0;
        PNIO_UINT16 BytesToSend      = 0;
        PNIO_UINT16 BytesSent        = 0;

        // **** send image file to tcp server *****
        BytesSent = tcp_if_send (RemSockId, (unsigned char*) &RestBytes_htonl, 4);

        while (RestBytes > 0)
        {
        	BytesToSend = (RestBytes > MAX_BLOCK_SIZE) ? MAX_BLOCK_SIZE : (PNIO_UINT16) RestBytes;
            BytesSent 	= tcp_if_send (RemSockId, pBuf + BufOffset, BytesToSend);

			if (BytesSent <= BytesToSend)
			{
				RestBytes -= BytesSent;
	            BufOffset += BytesSent;
		        PNIO_printf(".");
			}
        }
        PNIO_printf(" ");
    }


    /***************************************************************************/
    PNIO_UINT32 TcpTraceUpload  (PNIO_VOID)
    {
	    // *-------------------------------------------
	    // *   receive flash image by tcp
	    // *-------------------------------------------
        TCP_COMMAND_HEADER Hdr;
        PNIO_UINT32 NumOfBytes;

	    PNIO_printf ("TcpTraceUpload Server: Init TCP interface\n");

        // ***** init tcp interface ******
        LocSockId = tcp_if_inits();
        if (LocSockId == TCP_SOCK_ERROR)
        {
            PNIO_printf ("create socket error\n");
            return (PNIO_NOT_OK);
        }
	    PNIO_printf ("TcpTraceUpload Server: Wait on connection ... \n");

        // ***** connect to client ******
        RemSockId = tcp_if_connectS (LocSockId, 0, TCP_TRC_UPLOAD_PORTNUMBER);
        if (RemSockId == TCP_SOCK_ERROR)
        {
            PNIO_printf ("TCP CONNECT ERROR!\n");
            return (PNIO_NOT_OK);
        }
        PNIO_printf ("TcpTraceUpload Server: Connection established\n");

        // ***** read data header  ******
	    PNIO_printf ("TcpTraceUpload Server: Wait on upload request ...");
        NumOfBytes = tcp_if_receive (RemSockId, (PNIO_UINT8*) &Hdr, sizeof (Hdr));
        if (NumOfBytes != sizeof (Hdr))
        {
            PNIO_printf ("\nError trace upload header header size = %d\n", NumOfBytes);
            tcp_if_disconnect (RemSockId);
            tcp_if_close (LocSockId);
            return (PNIO_NOT_OK);
        }
	    PNIO_printf ("OK, upload requested\n");

        // **** send number of trace buffers to tcp server *****
        {
        	PNIO_UINT32 TraceIndex;
        	PNIO_UINT32 TraceBufferCount 		= TrcGetBufMax();
        	PNIO_UINT32 TraceBufferCount_htonl  = TRACE_HTONL(TraceBufferCount);

        	tcp_if_send (RemSockId, (unsigned char*) &TraceBufferCount_htonl, 4);

        	for (TraceIndex = 0; TraceIndex < TraceBufferCount; TraceIndex++)
        	{
        	    PNIO_printf ("TcpTraceUpload Server: Send trace buffer #%d with %d bytes ... ", TraceIndex, TrcGetBufSize(TraceIndex));
        		// **** send trace buffer to TCP server ****
        		TcpTraceUploadBuffer (TrcGetBufPtr(TraceIndex), TrcGetBufSize(TraceIndex));

        		// **** wait for TRACE READY command from TCP server ****
        		NumOfBytes = tcp_if_receive (RemSockId, (PNIO_UINT8*) &Hdr, sizeof (Hdr));
                if (NumOfBytes != sizeof (Hdr))
                {
                    PNIO_printf ("\nError trace upload header header size = %d\n", NumOfBytes);
                    tcp_if_disconnect (RemSockId);
                    tcp_if_close (LocSockId);
                    return (PNIO_NOT_OK);
                }
        	    PNIO_printf ("done\n");
        	}
        }

        // ***** disconnect TCP server ****
        PNIO_printf("TcpTraceUpload Server: disconnect and close\n");
        TRACE_WAIT_MS(1000);
        tcp_if_disconnect (RemSockId);
        tcp_if_close (LocSockId);

		PNIO_printf("TcpTraceUpload Server: done\n");
        return (PNIO_OK);
    }


#endif /* PNIO_TRACE == PNIO_TRACE_DK_LSA */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
