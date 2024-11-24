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
/*  F i l e               &F: trace_mem.c                               :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Logging Output function into a circular buffer                           */
/*                                                                           */
/*  THIS MODULE HAS TO BE MODIFIED BY THE PNIO USER                          */
/*                                                                           */
/*****************************************************************************/


#include "compiler.h"

#if (PNIO_TRACE == PNIO_TRACE_DK_MEM)

    #include "os.h"
    #include "pniousrd.h"
    #include "pnio_trace.h"

    // *------------ defines -----------------*
    #define TRC_MODULE_ID   112

    #define OFFS_MSG1       		0			// column start for header (including timestamp, subsystem, module, line)
    #define OFFS_MSG2       		43			// column start for message text
    #define TRCBUF_MAX_MSG_LEN      300         // maximum buffer length

    // *------------ external functins ----------*
    extern PNIO_UINT32 FlashTraceBuf  (PNIO_VOID* pBuf, PNIO_UINT32 BufSize);
	extern PNIO_UINT32 RestoreTraceBuf  (PNIO_VOID* pBuf, PNIO_UINT32 BufSize);

    // *------------ static data  ----------*

    // **** format output message ***
    static PNIO_UINT32  BufInd;                 // tracebuffer-index
    static PNIO_UINT8*  pBuf;                   // pointer to start of the trace buffer
    static PNIO_UINT32  BufSemId;               // semaphore for print-buffer access
    static PNIO_UINT32  TrcBufTotalSize;        // total size of the tracebuf
    static PNIO_UINT32  TrcBufFillStart;        // current start position of the tracebuf (oldest entry)
    static PNIO_UINT32  TrcBufFillSize;         // size of valid tracebuf content
    static PNIO_BOOL    TrcBufOverflow;         // tracebuf overflow

    static PNIO_UINT8  TempBuf [TRCBUF_MAX_MSG_LEN];
    static PNIO_UINT32 TempBufIndex = 0;
    static PNIO_UINT32 TrcCount = 0;

    extern PNIO_UINT32 PnioLogDest;

	static PNIO_VOID TrcMemLog(PNIO_INT8* pMsgBuf, PNIO_UINT32 MsgSize)
	{
		//if (PnioLogDest == 2)
		{
			TrcMsg2Mem (pMsgBuf, MsgSize);
		}
	}


    // *----------------------------------------------------------------*
    // *   Trcinit (BufSize)
    // *----------------------------------------------------------------*
    // *   Inititalizes the Trace Module
    // *
    // *  input :  PNIO_UINT32  Bufsize   (Buffer-size in Bytes,
    // *                                  value must be aligned 4 byte.
    // *  out   :  PNIO_OK, PNIO_NOT_OK
    // *----------------------------------------------------------------*
    PNIO_UINT32     TrcInit	  (PNIO_UINT32 BufSize)
    {
        PNIO_UINT32 Status = PNIO_OK;

        TrcBufTotalSize = (BufSize) ? BufSize : 0x20000;   // default value

        if (TrcBufTotalSize % 4 == 0)
        {
            Status = TRACE_ALLOC_MEM((PNIO_VOID**) &pBuf,' ', TrcBufTotalSize);
            if (Status == PNIO_OK)
                Status = TRACE_SEM_B_ALLOC(&BufSemId);
            if (Status == PNIO_OK)
                Status = TRACE_SEM_B_GIVE(BufSemId); // so the first instance has access without free first

            if ((pBuf == NULL) || (Status != PNIO_OK))
                PNIO_Log  (PNIO_SINGLE_DEVICE_HNDL, PNIO_LOG_ERROR_FATAL, PNIO_PACKID_OTHERS, TRC_MODULE_ID, __LINE__);

            // buffer start and size are stored at the end
            TrcBufTotalSize -= (sizeof(TrcBufFillStart)+sizeof(TrcBufFillSize));

            BufInd           = 0;
            TrcBufFillStart  = 0;
            TrcBufFillSize   = 0;
            TrcBufOverflow   = PNIO_FALSE;
        }
        else
        {
            Status = PNIO_NOT_OK;
            PNIO_Log  (PNIO_SINGLE_DEVICE_HNDL, PNIO_LOG_ERROR_FATAL, PNIO_PACKID_OTHERS, TRC_MODULE_ID, __LINE__);
        }

        return (Status);
    }

    // *----------------------------------------------------------------*
    // *   TrcTraceMsgStore (pBuf, MsgSize)
    // *----------------------------------------------------------------*
    // *  Add a new entry of size MsgSize at the end of the trace buffer.
    // *
    // *  input :   subys       trace subsystem id
    // *            level       message tracelevel
    // *            pBuf        pointer to the message buffer
    // *                        (min. buffer size TRCBUF_MAX_MSG_LEN bytes)
    // *  out   :  PNIO_OK, PNIO_NOT_OK
    // *----------------------------------------------------------------*
    PNIO_UINT32 TrcTraceMsgStore (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level, PNIO_INT8 * pMsgBuf, PNIO_UINT32 StrLen)
    {
    	PNIO_UINT32 TmpSize;
    	PNIO_UINT32 MsgSize;
        PNIO_UINT32 TimStampL = TRACE_TIMER_LOW;
        TRACE_SEM_B_TAKE(BufSemId);

        MsgSize = StrLen + OFFS_MSG2 + 3;

        // shorten message to max buffer length
        if (MsgSize >TRCBUF_MAX_MSG_LEN)
        {
            MsgSize = TRCBUF_MAX_MSG_LEN;
        }
        *(pMsgBuf+MsgSize-3) = 0x0d;    // add CR (overwrites end delimiter)
        *(pMsgBuf+MsgSize-2) = 0x0a;    // add line feet
        *(pMsgBuf+MsgSize-1)   = 0;       // new end delimiter

        // fill in message header
        TrcCount = (TrcCount > 99999) ? 0 : TrcCount+1;
        TmpSize = PNIO_sprintf ((PNIO_UINT8*) pMsgBuf, "#%05d %09d: SY=%03d MO=%03d L%04d LV=%01d", TrcCount, TimStampL, subsys, ModId, Line, level);

        // change end delimiter from last PNIO_sprintf to ' '
        *(pMsgBuf + TmpSize) = ' ';

        if (SubsysTrace[subsys].MinLevel_StdOut >= level)
        {
            PNIO_ConsoleLog ("%s", (PNIO_INT8*)((PNIO_UINT32)pMsgBuf+OFFS_MSG2));
        }
        if (SubsysTrace[subsys].MinLevel_Mem >= level)
        {
        	TrcMemLog (pMsgBuf, MsgSize);
        }
        TRACE_SEM_B_GIVE(BufSemId);

		switch (level)
		{
			case PNIO_LOG_ERROR_FATAL:
				 PNIO_Log  (PNIO_SINGLE_DEVICE_HNDL, PNIO_LOG_ERROR_FATAL, subsys, ModId, Line);
				 break;
			case PNIO_LOG_ERROR:
				 PNIO_Log  (PNIO_SINGLE_DEVICE_HNDL, PNIO_LOG_ERROR, subsys, ModId, Line);
				 break;
			case PNIO_LOG_WARNING_HIGH:
				 break;
			case PNIO_LOG_WARNING:
				 break;
			default:
				 break;
		}
		return (PNIO_OK);
    }

    // *----------------------------------------------------------------*
    // *   TrcPrintMsgStore (pBuf, MsgSize)
    // *----------------------------------------------------------------*
    // *  Format print message as trace entry and put it to trace buffer.
    // *
    // *  input :  PNIO_VOID*        pBuf      pointer to the message buffer
    // *           PNIO_UINT32  Msgsize   Size of the message in Bytes,
    // *  out   :  PNIO_OK, PNIO_NOT_OK
    // *----------------------------------------------------------------*
    PNIO_UINT32 TrcPrintMsgStore (PNIO_INT8 * fmt, PNIO_va_list argptr)
    {
        PNIO_UINT32 MsgSize;
        PNIO_UINT32 TimStampL;
        PNIO_UINT32 LineBreakIndex;
		TRACE_SEM_B_TAKE(BufSemId);
        if (TempBufIndex == 0)
        {
            // generate Trace numer and copy into buffer
        	TimStampL = TRACE_TIMER_LOW;
            TrcCount = (TrcCount > 99999) ? 0 : TrcCount+1;
            PNIO_sprintf (&TempBuf[0], "#%05d %09d: SY=%03d MO=%03d L%04d LV=%01d ", TrcCount, TimStampL, 0, 0, 0, 0);
            TempBufIndex +=    (sizeof("#12345 123456789: SY=123 MO=123 L1234 LV=1 ")-1); // w/o zero byte
        }
        MsgSize = PNIO_vsprintf(&TempBuf[TempBufIndex], fmt, argptr);
		TempBufIndex += MsgSize;
		if (TempBufIndex > TRCBUF_MAX_MSG_LEN)
		{
			PNIO_ConsolePrintf ("**ERROR tracebuffer overflow, message too long\n");
			PNIO_Log  (PNIO_SINGLE_DEVICE_HNDL, PNIO_LOG_ERROR_FATAL, PNIO_PACKID_OTHERS, TRC_MODULE_ID, __LINE__);
		}
		while (TempBufIndex > OFFS_MSG2)
		{
			// search for line break
			LineBreakIndex = 0;
			while ((LineBreakIndex < TempBufIndex) && (TempBuf[LineBreakIndex] != '\n')) { LineBreakIndex++; }
			// copy prepared message into circ buffer and actualize buffer index
			if (LineBreakIndex < TempBufIndex)
			{
				TrcMemLog((PNIO_INT8*)&TempBuf[0], LineBreakIndex+1);
				TempBufIndex -= (LineBreakIndex+1);
				if (TempBufIndex)
				{
					TRACE_MEMCPY(&TempBuf[OFFS_MSG2], &TempBuf[LineBreakIndex+1], TempBufIndex);
					TempBufIndex += OFFS_MSG2;
				}
			}
			else
			{
				break;
			}
		}
		TRACE_SEM_B_GIVE(BufSemId);
        return (PNIO_OK);
    }


    // *----------------------------------------------------------------*
    // *   TrcMsg2Mem (pBuf)
    // *----------------------------------------------------------------*
    // *   add a new entry of size MsgSize at the end of the trace buffer.
    // *
    // *  input :  PNIO_VOID*        pBuf      pointer to the message buffer
    // *           PNIO_UINT32  Msgsize   Size of the message in Bytes,
    // *  out   :  PNIO_OK, PNIO_NOT_OK
    // *----------------------------------------------------------------*
    PNIO_VOID TrcMsg2Mem  (PNIO_INT8* pMsgBuf, PNIO_UINT32 MsgSize)
    {
    	// copy prepared message into circ buffer and actualize buffer index
        if (BufInd > (TrcBufTotalSize - MsgSize))
        {   // circ buffer overflow, set index to buffer start
         	TrcBufFillSize  = BufInd;
        	BufInd 			= 0;
        	TrcBufOverflow  = PNIO_TRUE;
        }
        // append to last message buffer
        TRACE_MEMCPY(pBuf + BufInd, pMsgBuf, MsgSize);
        BufInd += MsgSize;
        if (TrcBufOverflow)
        {
        	TrcBufFillStart = BufInd;
        }
        else
        {
        	TrcBufFillSize  = BufInd;
        }
        // update trace start and size at buffer end
        *((PNIO_UINT32*)(pBuf+TrcBufTotalSize))                         = TrcBufFillStart;
        *((PNIO_UINT32*)(pBuf+TrcBufTotalSize+sizeof(TrcBufFillStart))) = TrcBufFillSize;
    }



    // *----------------------------------------------------------------*
    // *   TrcGetBufPtr    (PNIO_VOID)
    // *----------------------------------------------------------------*
    // *   returns the start address of the Tracebuffer.
    // *
    // *  input :  --
    // *  out   :  pTrcBbuf          pointer to the trace buffer
    // *----------------------------------------------------------------*
    PNIO_UINT8*    TrcGetBufPtr    (PNIO_VOID)
    {
        return ((PNIO_VOID*) pBuf);
    }


    // *----------------------------------------------------------------*
    // *   TrcGetBufSize    (PNIO_VOID)
    // *----------------------------------------------------------------*
    // *   returns the length of the tracebuffer.
    // *
    // *  input :  --
    // *  out   :  pTrcBbuf          pointer to the trace buffer
    // *----------------------------------------------------------------*
    PNIO_UINT32    TrcGetBufSize    (PNIO_VOID)
    {
        return (TrcGetTotalSize());
    }


    // *----------------------------------------------------------------*
    // *   TrcGetTotalSize    (PNIO_VOID)
    // *----------------------------------------------------------------*
    // *   returns the total size of the tracebuffer.
    // *
    // *  input :  --
    // *  out   :  pTrcBbuf          pointer to the trace buffer
    // *----------------------------------------------------------------*
    PNIO_UINT32    TrcGetTotalSize    (PNIO_VOID)
    {
        return (TrcBufTotalSize);
    }

    // *----------------------------------------------------------------*
    // *   TrcGetFillSize    (PNIO_VOID)
    // *----------------------------------------------------------------*
    // *   returns the length of the tracebuffer.
    // *
    // *  input :  --
    // *  out   :  pTrcBbuf          pointer to the trace buffer
    // *----------------------------------------------------------------*
    PNIO_UINT32    TrcGetFillSize    (PNIO_VOID)
    {
        return (TrcBufFillSize);
    }


    // *----------------------------------------------------------------*
    // *   TrcGetFillStart    (PNIO_VOID)
    // *----------------------------------------------------------------*
    // *   returns the start position of the tracebuffer.
    // *
    // *  input :  --
    // *  out   :  pTrcBbuf          pointer to the trace buffer
    // *----------------------------------------------------------------*
    PNIO_UINT32    TrcGetFillStart    (PNIO_VOID)
    {
        return (TrcBufFillStart);
    }


    // *----------------------------------------------------------------*
    // *   TrcGetFillState    (PNIO_VOID)
    // *----------------------------------------------------------------*
    // *   returns the length of the tracebuffer.
    // *
    // *  input :  --
    // *  out   :  pTrcBbuf          pointer to the trace buffer
    // *----------------------------------------------------------------*
    PNIO_UINT32    TrcGetFillState   (PNIO_VOID)
    {
        return (TrcBufFillStart);
    }


    // *----------------------------------------------------------------*
    // *   TrcStoreBuf    (PNIO_VOID)
    // *----------------------------------------------------------------*
    // *   stores the buffer into a persistent memory
    // *
    // *  input :  --
    // *  out   :  PNIO_OK, PNIO_NOT_OK
    // *----------------------------------------------------------------*
    PNIO_UINT32 TrcStoreBuf    (PNIO_VOID)
    {
    	PNIO_ConsolePrintf ("Flash Trace Buffer ...\n");
        FlashTraceBuf  ((PNIO_VOID*) TrcGetBufPtr(), TrcGetTotalSize() );
        return (PNIO_OK);
    }


	PNIO_VOID SaveTraceBuffer (PNIO_VOID)
	{
		PNIO_UINT32 i;
		PNIO_UINT8* pTrc;
		PNIO_UINT32 FillStart;
		PNIO_UINT32 FillSize;

		TRACE_SEM_B_TAKE(BufSemId);

		pTrc = TrcGetBufPtr ();
		FillStart = TrcGetFillStart();
		FillSize  = TrcGetFillSize();
		PNIO_ConsolePrintf ("SaveTraceBuffer (Ptr=0x%08X, Start = %d, Size=%d)\n", pTrc, FillStart, FillSize);
		PNIO_ConsolePrintf ("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		for (i = FillStart; i < FillSize; i++)
		{
			PNIO_ConsolePrintf ("%c", *(pTrc + i));
		}
		if (FillStart > 0) // overflow
		{
			for (i = 0; i < FillStart; i++)
			{
				PNIO_ConsolePrintf ("%c", *(pTrc + i));
			}
		}
		PNIO_ConsolePrintf ("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

		TRACE_SEM_B_GIVE(BufSemId);
	}

	PNIO_VOID RestoreTraceBuffer (PNIO_VOID)
	{
		PNIO_UINT32 i;
		PNIO_UINT8* pTrc;
		PNIO_UINT32 TotalSize;
		PNIO_UINT32 FillStart;
		PNIO_UINT32 FillSize;

		pTrc = TrcGetBufPtr ();
		TotalSize = TrcGetTotalSize();

		// restore trace buffer from flash
		RestoreTraceBuf (pTrc, TotalSize+sizeof(TrcBufFillStart)+sizeof(TrcBufFillSize));

		TRACE_SEM_B_TAKE(BufSemId);
		// get trace buffer start and size
		FillStart = *((PNIO_UINT32*)(pTrc+TotalSize));
		FillSize  = *((PNIO_UINT32*)(pTrc+TotalSize+sizeof(TrcBufFillStart)));
		if ((FillStart == 0xFFFFFFFF) || (FillSize == 0xFFFFFFFF) ||
		    (FillStart > TotalSize)   || (FillSize > TotalSize))
		{
			PNIO_ConsolePrintf ("ERROR (FlashedTraceBuffer): No valid trace buffer found on flash!\n");
			TRACE_SEM_B_GIVE(BufSemId);
			return;
		}
		PNIO_ConsolePrintf ("FlashedTraceBuffer (Ptr=0x%08X, Start = %d, Size=%d)\n", pTrc, FillStart, FillSize);
		PNIO_ConsolePrintf ("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		for (i = FillStart; i < FillSize; i++)
		{
			PNIO_ConsolePrintf ("%c", *(pTrc + i));
		}
		if (FillStart > 0) // overflow
		{
			for (i = 0; i < FillStart; i++)
			{
				PNIO_ConsolePrintf ("%c", *(pTrc + i));
			}
		}
		PNIO_ConsolePrintf ("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		//  cleanup trace buffer
		*((PNIO_UINT32*)(pTrc+TotalSize)) 						  = 0;
		*((PNIO_UINT32*)(pTrc+TotalSize+sizeof(TrcBufFillStart))) = 0;

		TRACE_SEM_B_GIVE(BufSemId);
	}


    PNIO_VOID dk_trace_array (PNIO_UINT32 ModId,
                              PNIO_UINT32 Line,
                              PNIO_UINT32 subsys,
                              PNIO_UINT32 level,
                              PNIO_UINT8* ptr, PNIO_UINT32 len)
    {
        PNIO_INT32 StrBufOfs = OFFS_MSG2;   // offset in the string buffer (destination)
        PNIO_UINT32 ofs = 0;                // offset in the array buffer (source)
        PNIO_INT32 rest = len;
        PNIO_UINT8* pStrBuf;

        TRACE_ALLOC_MEM((PNIO_VOID**) &pStrBuf, ' ', 0x1000);

        StrBufOfs += PNIO_sprintf ((PNIO_VOID*)(pStrBuf + StrBufOfs), "\n");
        while (rest > 0)
        {
            if (rest >= 10)
            {
                StrBufOfs += PNIO_sprintf (
                          (PNIO_VOID*)(pStrBuf + StrBufOfs),
                          "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n",
                          *(ptr+ofs+0),  *(ptr+ofs+1),  *(ptr+ofs+2),  *(ptr+ofs+3),  *(ptr+ofs+4),
                          *(ptr+ofs+5),  *(ptr+ofs+6),  *(ptr+ofs+7),  *(ptr+ofs+8),  *(ptr+ofs+9) );
                rest -= 10;
                ofs +=10;
            }
            else
            {
                PNIO_INT i;
                for (i = 0; i < rest; i++)
                    StrBufOfs += PNIO_sprintf ((PNIO_VOID*)(pStrBuf + StrBufOfs), "%02x", *(ptr+ofs+i));
                rest = 0;
            }

            if (StrBufOfs >= ((5 * TRCBUF_MAX_MSG_LEN) - OFFS_MSG2))
                PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                            PNIO_LOG_ERROR_FATAL,
                            PNIO_PACKID_TRC,
                            TRC_MODULE_ID,
                            __LINE__);
        }

        TrcTraceMsgStore (ModId, Line, subsys, level, (PNIO_INT8*) pStrBuf, StrBufOfs);
        TRACE_FREE_MEM((PNIO_VOID*)pStrBuf);
    }


    PNIO_VOID dk_trace_00 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level, PNIO_VOID* msg)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


    PNIO_VOID dk_trace_01 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level, PNIO_VOID* msg,
                           PNIO_UINT32 para1)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg, para1);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


    PNIO_VOID dk_trace_02 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level, PNIO_VOID* msg,
                           PNIO_UINT32 para1, PNIO_UINT32 para2)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg, para1, para2);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


    PNIO_VOID dk_trace_03 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level, PNIO_VOID* msg,
                           PNIO_UINT32 para1, PNIO_UINT32 para2, PNIO_UINT32 para3)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg, para1, para2, para3);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


    PNIO_VOID dk_trace_04 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level, PNIO_VOID* msg,
                           PNIO_UINT32 para1, PNIO_UINT32 para2, PNIO_UINT32 para3, PNIO_UINT32 para4)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg, para1, para2, para3, para4);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


    PNIO_VOID dk_trace_05 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level, PNIO_VOID* msg,
                           PNIO_UINT32 para1, PNIO_UINT32 para2, PNIO_UINT32 para3, PNIO_UINT32 para4,
                           PNIO_UINT32 para5)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg, para1, para2, para3, para4, para5);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


    PNIO_VOID dk_trace_06 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg,
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4,
                           PNIO_UINT32 para5,  PNIO_UINT32 para6)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg, para1, para2, para3, para4, para5, para6);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


    PNIO_VOID dk_trace_07 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg,
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4,
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg, para1, para2, para3, para4, para5, para6, para7);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


    PNIO_VOID dk_trace_08 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg,
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4,
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg, para1, para2, para3, para4, para5, para6, para7, para8);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


    PNIO_VOID dk_trace_09 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg,
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4,
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8,
                           PNIO_UINT32 para9)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg, para1, para2, para3, para4, para5, para6, para7, para8, para9);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


    PNIO_VOID dk_trace_10 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg,
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4,
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8,
                           PNIO_UINT32 para9,  PNIO_UINT32 para10)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


    PNIO_VOID dk_trace_11 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg,
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4,
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8,
                           PNIO_UINT32 para9,  PNIO_UINT32 para10, PNIO_UINT32 para11)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


    PNIO_VOID dk_trace_12 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg,
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4,
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8,
                           PNIO_UINT32 para9,  PNIO_UINT32 para10, PNIO_UINT32 para11, PNIO_UINT32 para12)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


    PNIO_VOID dk_trace_13 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg,
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4,
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8,
                           PNIO_UINT32 para9,  PNIO_UINT32 para10, PNIO_UINT32 para11, PNIO_UINT32 para12,
                           PNIO_UINT32 para13)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


    PNIO_VOID dk_trace_14 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg,
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4,
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8,
                           PNIO_UINT32 para9,  PNIO_UINT32 para10, PNIO_UINT32 para11, PNIO_UINT32 para12,
                           PNIO_UINT32 para13,  PNIO_UINT32 para14)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


    PNIO_VOID dk_trace_15 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg,
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4,
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8,
                           PNIO_UINT32 para9,  PNIO_UINT32 para10, PNIO_UINT32 para11, PNIO_UINT32 para12,
                           PNIO_UINT32 para13,  PNIO_UINT32 para14,  PNIO_UINT32 para15)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


    PNIO_VOID dk_trace_16 (PNIO_UINT32 ModId, PNIO_UINT32 Line, PNIO_UINT32 subsys, PNIO_UINT32 level,  PNIO_VOID* msg,
                           PNIO_UINT32 para1,  PNIO_UINT32 para2,  PNIO_UINT32 para3,  PNIO_UINT32 para4,
                           PNIO_UINT32 para5,  PNIO_UINT32 para6,  PNIO_UINT32 para7,  PNIO_UINT32 para8,
                           PNIO_UINT32 para9,  PNIO_UINT32 para10, PNIO_UINT32 para11, PNIO_UINT32 para12,
                           PNIO_UINT32 para13,  PNIO_UINT32 para14,  PNIO_UINT32 para15,  PNIO_UINT32 para16)
    {
        PNIO_INT8 buffer[TRCBUF_MAX_MSG_LEN];
        PNIO_UINT32 StrLength;

        TRACE_MEMSET(&buffer[0], ' ', sizeof (buffer));
        StrLength = PNIO_sprintf ((PNIO_UINT8*) &buffer[OFFS_MSG2], msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12);
        if (StrLength > (TRCBUF_MAX_MSG_LEN - OFFS_MSG2))
            PNIO_Log (  PNIO_SINGLE_DEVICE_HNDL,
                        PNIO_LOG_ERROR_FATAL,
                        PNIO_PACKID_TRC,
                        TRC_MODULE_ID,
                        __LINE__);
        TrcTraceMsgStore (ModId, Line, subsys, level, &buffer[0], StrLength);
    }


#endif




/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
