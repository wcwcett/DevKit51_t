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
/*  F i l e               &F: Tcp_IF.c                                  :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Example : TCP INTERFACE                                                  */
/*            performs point to point TCP connection to a remote device      */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/

#include "compiler.h"
    #include "os_taskprio.h"
    #include "Tcp_IF.h"
	#include "pniousrd.h"
    #include "obsd_userland_settings.h"
    #include "obsd_userland_unistd.h"
    #include "obsd_kernel_socket.h"
    #include "obsd_kernel_in.h"
    #include "os.h"

    #define BUFFER_SIZE     1024

    #define LTRC_ACT_MODUL_ID   12

    // *----------------------------------------------------------------*
    // *    
    // *   () tcp_if_inits (void)
    // *    
    // *----------------------------------------------------------------*
    // *  initializes the interface, creates the tcp socket
    // *  
    // *----------------------------------------------------------------*
    // *  Input:     --
    // *----------------------------------------------------------------*
    int tcp_if_inits (void)
    {
	    // *--------------------------------------------------
	    // * create socket
	    // *--------------------------------------------------
	    int LocalSockId = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	    if (LocalSockId < 0)
	    {
            PNIO_printf("socket() failed\n");
            LSA_TRACE_00  (TRACE_SUBSYS_APPL_PLATFORM, LSA_TRACE_LEVEL_FATAL, "ERROR socket open\n" );
            return (TCP_SOCK_ERROR);      
	    }

        return (LocalSockId);
    }


    // *----------------------------------------------------------------*
    // *    
    // *   ()
    // *    
    // *----------------------------------------------------------------*
    // *  
    // *  
    // *----------------------------------------------------------------*
    // *  Input:     --
    // *----------------------------------------------------------------*
    unsigned int tcp_if_connectC (int            LocalSockId,
                                  unsigned int   RemIpAddr,  
                                  unsigned int   Port)
    {
	    struct   sockaddr_in srv_addr = {0};       // server address
        srv_addr.sin_addr.s_addr = (RemIpAddr == 0) ? INADDR_ANY : OsHtonl(RemIpAddr);
        srv_addr.sin_port = OsHtons((PNIO_UINT16)Port);
	    srv_addr.sin_family = AF_INET;
	    
	    // *--------------------------------------------------
	    // * Bind socket to local IP port (and remote IP addr.).
	    // *--------------------------------------------------
        if (connect(LocalSockId,  (struct sockaddr *) &srv_addr, sizeof(srv_addr)) == -1)
	    {
            PNIO_printf("connect() failed");
		    return (PNIO_FALSE);
	    }
	    
        return (PNIO_TRUE);
    }

    // *----------------------------------------------------------------*
    // *    
    // *   tcp_if_connectS (unsigned int   RemIpAddr,unsigned int   Port()
    // *    
    // *----------------------------------------------------------------*
    // *  opens a socket for tcp server and waits on connection
    // *  
    // *----------------------------------------------------------------*
    // *  Input:     RemIpAddr    Ip addr in 32 bit network format
    // *             Port         local Tcp Port number
    // *  return:                 PNIO_OK, PNIO_NOT_OK
    // *----------------------------------------------------------------*
    int tcp_if_connectS (int            LocalSockId,
                                  unsigned int   RemIpAddr,  
                                  unsigned int   Port)
    {
        struct   sockaddr_in cli_addr = {0};       // client address
	    struct   sockaddr_in srv_addr = {0};       // server address
        int      cli_size;                   // sizeof (cli_addr)
        int      RemoteSockId;
        srv_addr.sin_addr.s_addr = (RemIpAddr == 0) ? INADDR_ANY : OsHtonl (RemIpAddr);
	    srv_addr.sin_port = OsHtons((PNIO_UINT16)Port);
	    srv_addr.sin_family = AF_INET;

	    // *--------------------------------------------------
	    // * Bind socket to local IP port (and remote IP addr.).
	    // *--------------------------------------------------
        if (bind(LocalSockId,  (struct sockaddr *) &srv_addr, sizeof(srv_addr)) == -1)
	    {
            PNIO_printf("bind() failed\n");
		    return (TCP_SOCK_ERROR);
	    }

	    // *--------------------------------------------------
	    // * create a number of queues for incoming connection
        // * requests on this port
	    // *--------------------------------------------------
        if (listen(LocalSockId, 3) == -1)
	    {
            PNIO_printf("listen() failed\n");
		    return (TCP_SOCK_ERROR);
	    }

        cli_size = sizeof(cli_addr);

	    // *--------------------------------------------------
	    // * accept an incoming connection request
	    // *--------------------------------------------------
		RemoteSockId = -1;
		while (RemoteSockId == -1)
		{
	        RemoteSockId = accept(LocalSockId, (struct sockaddr *)&cli_addr, (socklen_t *)&cli_size);
	        if (RemoteSockId == -1)
	            OsWait_ms (100);
		}

		return (RemoteSockId);
    }

    // *----------------------------------------------------------------*
    // *    
    // *   tcp_if_send   (unsigned char* pDat,  unsigned int DatLen)()
    // *    
    // *----------------------------------------------------------------*
    // *   sends the specified data to the remote device
    // *  
    // *----------------------------------------------------------------*
    // *  Input:     pDat       pointer to data buffer
    // *             DatLen     length of data in bytes
    // *  return                -1:  error   else: length of transferred data
    // *----------------------------------------------------------------*
    unsigned int tcp_if_send   (int RemoteSockId, unsigned char* pDat,  unsigned int DatLen)
    {
        int bytes = 0;
        
        if (DatLen == 0)
            return (0);
        
        while (bytes == 0)
        {
            bytes = send(RemoteSockId, (char*)pDat, DatLen, 0);
            if (bytes == 0)
                OsWait_ms (100);
            else
                OsWait_ms (10);
        }
        if (bytes < 0)
        {
            bytes = PNIO_NOT_OK;
        }
        return ((unsigned int)bytes);
    }


    // *----------------------------------------------------------------*
    // *    
    // *   () tcp_if_receive  (unsigned char* pDat, unsigned int MaxDatLen)
    // *    
    // *----------------------------------------------------------------*
    // *  
    // *  
    // *----------------------------------------------------------------*
    // *  Input:     --
    // *----------------------------------------------------------------*
    unsigned int tcp_if_receive  (int RemoteSockId, unsigned char* pDat, unsigned int MaxDatLen)
    {
        int bytes = 0;
        
        if (MaxDatLen == 0)
            return (0);

        while (bytes == 0)
        {
            bytes = recv(RemoteSockId, (char*)pDat, MaxDatLen, 0);
            if (bytes == -1)    // invalid value
                bytes = 0;
            
            if (bytes == 0) 
                OsWait_ms (100);
            else
                OsWait_ms (10);
        }
        return ((unsigned int)bytes);
    }
 

    // *----------------------------------------------------------------*
    // *    
    // *   () tcp_if_disconnect  (void) 
    // *    
    // *----------------------------------------------------------------*
    // *  
    // *  
    // *----------------------------------------------------------------*
    // *  Input:     --
    // *----------------------------------------------------------------*
    unsigned int tcp_if_disconnect  (int RemoteSockId) 
    { 
       int status = close(RemoteSockId);
       if (status == 0)
            return (PNIO_TRUE);
       else
            return (PNIO_FALSE);
    }


    // *----------------------------------------------------------------*
    // *    
    // *   () tcp_if_disconnect  (void) 
    // *    
    // *----------------------------------------------------------------*
    // *  
    // *  
    // *----------------------------------------------------------------*
    // *  Input:     --
    // *----------------------------------------------------------------*
    unsigned int tcp_if_close  (int LocalSockId) 
    { 
       int status = 0;

       status = close(LocalSockId);
       if (status == 0)
            return (PNIO_TRUE);
       else
            return (PNIO_FALSE);
    }

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
