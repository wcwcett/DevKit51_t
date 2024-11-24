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
/*  F i l e               &F: Tcp_Flash_fw.c                            :F&  */
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
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/
    #include "compiler.h"
    #include "usriod_cfg.h"
    #include "pniousrd.h"
    #include "pnio_trace.h"
    #include "os.h"
    #include "os_taskprio.h"
    #include "nv_data.h"
    #include "bspadapt.h"
    #include "usriod_utils.h"
    #include "Tcp_IF.h"
 
    #define LTRC_ACT_MODUL_ID   11

    #define TCP_FLASHLOAD_PORTNUMBER    999

    extern PNIO_UINT32 TcpFlashFirmware (void* pBuf, PNIO_UINT32 BufSize);

    typedef struct 
    {
        int StartTag;
        PNIO_UINT32 FileSize;
        int FileCheckSum;
    }   IMG_FILE_HEADER;


    static int LocSockId, RemSockId;
    static PNIO_UINT32 TskId_TcpFlash;
    

    PNIO_UINT32 TcpReceiveAndFlashFirmware  (void)
    {
	    // *-------------------------------------------
	    // *   receive flash image by tcp
	    // *-------------------------------------------
        IMG_FILE_HEADER Hdr;
        PNIO_UINT8* pBuf;
        PNIO_UINT32 NumOfBytes;
        PNIO_UINT32 SumBytes = 0;
        PNIO_UINT32 Status = 0;

        PNIO_printf("\n--------------------------------------------------\n");

        // ***** init tcp interface ******
        PNIO_printf("TCP interface init ... ");
        LocSockId = tcp_if_inits();
        if (LocSockId == TCP_SOCK_ERROR)
        {
            PNIO_printf("ERROR cannot create socket\n");
            return (PNIO_NOT_OK);
        }
        PNIO_printf("OK\n");

        // ***** connect to client ******
        PNIO_printf("TCP interface wait on connection ... ");
        RemSockId = tcp_if_connectS (LocSockId, 0, TCP_FLASHLOAD_PORTNUMBER);
        if (RemSockId == TCP_SOCK_ERROR)
        {
            PNIO_printf("ERROR during TCP connect\n");
            return (PNIO_NOT_OK);
        }
        PNIO_printf("OK, established\n");


        // ***** read data header  ******
        PNIO_printf("Receive firmware image header ... ");
        NumOfBytes = tcp_if_receive (RemSockId, 
                                     (PNIO_UINT8*) &Hdr,
                                     sizeof (Hdr));
        if (NumOfBytes != sizeof (Hdr))
        {
            PNIO_printf("ERROR image size = %d\n", NumOfBytes);
            tcp_if_disconnect (RemSockId);
            tcp_if_close (LocSockId);
            return (PNIO_NOT_OK);
        }
    	PNIO_printf("Image size = %d\n", Hdr.FileSize);


        // ***** alloc memory for file data ******
        PNIO_printf("Allocate memory ... ");
        if ( OsAllocF ((void**)&pBuf, Hdr.FileSize) != PNIO_OK)
        {
            PNIO_printf("ERROR OsAlloc\n");
            tcp_if_disconnect (RemSockId);
            tcp_if_close (LocSockId);
            return (PNIO_NOT_OK);
        }
       	PNIO_printf("done\n");


        // ***** read the image file data ******
        {
            PNIO_UINT8 ProgressPrint = 0;
            PNIO_printf("Receive firmware image data\n");
            while (SumBytes < Hdr.FileSize)
            {
                NumOfBytes = tcp_if_receive ( RemSockId, 
                                              (PNIO_UINT8*) (pBuf+SumBytes),
                                              Hdr.FileSize - SumBytes);

                if (!ProgressPrint) { PNIO_printf ("."); }
                ProgressPrint = (ProgressPrint==10) ? 0 : ProgressPrint+1;
                SumBytes += NumOfBytes;
            }
            PNIO_printf("\n finished, %d bytes received\n", SumBytes);
        }

        // **** create checksum *****
        {
            int CheckSum = 0;
            int i;
            for (i = 0; i < Hdr.FileSize; i++)
            {
            	CheckSum += (int) *(pBuf+i);
            }
            CheckSum = -CheckSum;

            if (CheckSum != Hdr.FileCheckSum)
            {
                PNIO_printf("ERROR: Wrong Checksum = 0x%x (0x%x)\n",
                             CheckSum, Hdr.FileCheckSum);
                tcp_if_disconnect (RemSockId);
                tcp_if_close (LocSockId);
                OsFree (pBuf);
                return (PNIO_NOT_OK);
            }
            PNIO_printf("Checksum = 0x%x OK\n", CheckSum);
        }

        // ***** now erase flash and program the firmware ****
        PNIO_printf("\n!!! DO NOT SWITCH POWER OFF !!!\n");

        PNIO_printf("\nERASE & PROGRAM FLASH:\n");
        Status = TcpFlashFirmware (pBuf, Hdr.FileSize);
        {
            PNIO_UINT8 buffer[40]; 
            if (Status == PNIO_NOT_OK)
            {
                PNIO_sprintf(buffer, "\nERROR at firmware flashing\n");
            }
            else
            {
                PNIO_sprintf(buffer, "\nOK, Flashing firmware finished\n");
            }

            PNIO_printf("%s", buffer);
            NumOfBytes = tcp_if_send (RemSockId, &buffer[0], sizeof (buffer));
            if (NumOfBytes < sizeof (buffer))
            {
                PNIO_printf("\nERROR sending firmware update status!\n");
            }
            if (Status == PNIO_OK)
            {
                PNIO_printf("-----------------------------------------------\n");
                PNIO_printf("=> RESET your system, to start new firmware ...\n");
                PNIO_printf("-----------------------------------------------\n");
            }
        }

        // ***** disconnect and free memory ****
        OsWait_ms (2000);
        tcp_if_disconnect (RemSockId);
        tcp_if_close (LocSockId);
        OsFree (pBuf);
        return (PNIO_OK);
    }


    // *----------------------------------------------------------------*
    // *    
    // *  Task_TcpFlash (void) 
    // *    
    // *----------------------------------------------------------------*
    // *  cyclic exchange of IO data
    // *    
    // *  This task performs the cyclic IO data exchange in an endless   
    // *  loop.  Every IO data exchange (one data read and one data write) 
    // *         is triggered by a semaphore, that is set in PNIO_cbf_trigger_io_exchange()
    // *    
    // *----------------------------------------------------------------*
    // *  Input:    ----    
    // *  Output:   ----
    // *          
    // *----------------------------------------------------------------*
    static PNIO_INT32 Task_TcpFlash(void)
    {
        PNIO_UINT32 Status;
        
        // *----------------------------------------------------------
        // * Synchronization to parent process
        // *----------------------------------------------------------
        OsWaitOnEnable();           // must be first call in every task
        PNIO_printf ("AutoFlash Task started...\n");
        
        while (1)
        {
        	Status = TcpReceiveAndFlashFirmware ();
        	if (Status == PNIO_OK)
        	{
        		PNIO_printf ("new firmware flashed, reenable AutoFlash again..\n");
        		OsWait_ms(100);
        	}
        	else
        	{
        		PNIO_printf ("###Error flashing firmware, terminate task..\n");
        		return PNIO_NOT_OK;
        	}
        }
        return PNIO_OK; /*lint !e527 Unreachable code */
    }
    
    void TcpAutoFlashEnable (void)
    {
        PNIO_UINT32 Status;
        
        // *----------------------------------------------------------
        // * TCP Flash task
        // *----------------------------------------------------------
        Status = OsCreateThread((void(*)(void))Task_TcpFlash, 0, (PNIO_UINT8*)"Pnio_TcpFlash", TASK_PRIO_TCP_FW_LOADER, OS_TASK_DEFAULT_STACKSIZE, &TskId_TcpFlash);
        PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: OsCreateThread\n");
        Status = OsStartThread(TskId_TcpFlash);
        PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: OsStartThread\n");
   }


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
