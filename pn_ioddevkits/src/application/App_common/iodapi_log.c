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
/*  F i l e               &F: iodapi_log.c                              :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Logging Output function for pnio                                         */
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
#include "pniousrd.h"
#include "usriod_utils.h"
#include "pnio_trace.h"
#include "os.h"
#include "bspadapt.h"

// *------------ external data and function ----------*
extern PNIO_UINT32      PnioLogDest;            // destination for logging messages

// *----------------------------------------------------------------*
// *
// *  PNIO_Log (void)
// *
// *----------------------------------------------------------------*
// *
// *  output of debug logging
// *  necessary for PNIO, it is only used for performance measuring.
// *
// *  Input:    MsgType      PNIO_LOG__ERROR_FATAL
// *                         PNIO_LOG__ERROR
// *                         PNIO_LOG__IMPORTANT
// *                         PNIO_LOG__NORMAL
// *                         PNIO_LOG__MEMORY
// *
// *            PackId       Package ID
// *            ModId        Module ID
// *            fmt          format string for vfprintf
// *            argptr       pointer to print-arguments
// *
// *  Output:   ----
// *
// *----------------------------------------------------------------*
void PNIO_Log(PNIO_UINT32 DevHndl,
              PNIO_UINT32 ErrLevel,
              PNIO_UINT32 PackId,
              PNIO_UINT32 ModId,
              PNIO_UINT32 LineNum)
{
    switch (ErrLevel)
    {
        case PNIO_LOG_ERROR_FATAL:
        {
            PNIO_UINT32 DoEndlessLoop = 1;
            if (PnioLogDest == 0)
            {
                PnioLogDest = 1;  // enable Console for Fatal Error print
            }
            PNIO_TrcPrintf("-------------------------------------------------\n");
            PNIO_TrcPrintf("##FATAL ERROR; DO ENDLESS LOOP TO KEEP STACKTRACE\n");
            PNIO_TrcPrintf("##PnioPackId %d, ModId %d Line %d Device %d\n",
                           PackId, ModId, LineNum, DevHndl);
            if (PnioLogDest == 2)
            {
                #if(PNIO_TRACE != PNIO_TRACE_NONE)
                SaveTraceBuffer();  // print trace buffer content
                #endif
            }
            /* Upload instrumentation buffer on fatal error
            {
                OsIntrumentationDisable();
                OsPrintTaskProp();
                TcpIntrumentationUpload();
            }
            */
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
            PNIO_UINT32* pWatchdogReload0L = (PNIO_UINT32*)U_WDOG__RELD0_LOW;  // 0x9876affe when loaded from ICE
            PNIO_UINT32* pWatchdogReload0H = (PNIO_UINT32*)U_WDOG__RELD0_HIGH; // 0x9876affe when loaded from ICE
            if (!(((*pWatchdogReload0L & 0x0000FFFF) == 0xAFFE) && ((*pWatchdogReload0H & 0x0000FFFF) == 0xAFFE)))
            {
#if(PNIO_TRACE != PNIO_TRACE_NONE)
                TrcStoreBuf(); // do not store trace buffer when loaded from ICE
#endif
                PNIO_ConsolePrintf("-------------------------------------------------\n");
                PNIO_ConsolePrintf("##FATAL ERROR STORED ON FLASH  <<<ENDLESS LOOP>>>\n");
                PNIO_ConsolePrintf("-------------------------------------------------\n");
            }

            if (((*pWatchdogReload0L & 0x0000FFFF) == 0xCAFE) && ((*pWatchdogReload0H & 0x0000FFFF) == 0xCAFE))
            {
                PNIO_ConsolePrintf ("### CONTINUE EXECUTION TO DEBUG FATAL ERROR REASON ... \n");
                DoEndlessLoop = 0;
            }
#endif

            PNIO_device_stop(PNIO_SINGLE_DEVICE_HNDL);
            if (DoEndlessLoop)
            {
                OsWait_ms(500); // give time for device de-activation
                OsEnterX(OS_MUTEX_DEFAULT);
                OsKillOtherThreads();
            }
            /* TODO:
             * If fatal error is called from CM or PNDV module, the AR does not drop since the device de-activation cannot
             * be triggered! Even if all other threads are killed, hardware-level cyclic data is still being sent.
             *
             * A function should be implemented to interrupt cyclic data transmission so that the AR is dropped and the
             * network is informed about the device failure.
             */

            while (DoEndlessLoop)
            {
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
                PNIO_UINT32 j;
                for (j = PNIO_LED_USER_00; j <= PNIO_LED_USER_15; j++)
                {
                    Bsp_EbSetLed (j, 1);
                    OsWait_ms (5);  // give time to send printf via UDP (if selected)
                }
                OsWait_ms (300);  // give time to send printf via UDP (if selected)
                for (j = PNIO_LED_USER_15; j >= PNIO_LED_USER_00; j--)
                {
                    Bsp_EbSetLed (j, 0);
                    OsWait_ms (5);  // give time to send printf via UDP (if selected)
                }
#else
                Bsp_EbSetLed (PNIO_LED_ERROR, 1);
                Bsp_EbSetLed (PNIO_LED_RUN,   1);
                OsWait_ms (300);  // give time to send printf via UDP (if selected)
                Bsp_EbSetLed (PNIO_LED_ERROR, 0);
                Bsp_EbSetLed (PNIO_LED_RUN,   0);
#endif
                OsWait_ms (300);  // give time to send printf via UDP (if selected)
            }
            break; /*lint !e527 unreachable code */
        }
        case PNIO_LOG_ERROR:
            PNIO_TrcPrintf("##ERROR: ");
            break;
        case PNIO_LOG_WARNING_HIGH:
            PNIO_TrcPrintf("##WARNING HIGH: ");
            break;
        case PNIO_LOG_WARNING:
            PNIO_TrcPrintf("##WARNING: ");
            break;
        case PNIO_LOG_NOTE_HIGH:
            PNIO_TrcPrintf("##NOTE HIGH: ");
            break;
        case PNIO_LOG_NOTE:
            PNIO_TrcPrintf("##NOTE: ");
            break;
        case PNIO_LOG_CHAT_HIGH:
            PNIO_TrcPrintf("##CHAT HIGH: ");
            break;
        case PNIO_LOG_CHAT:
            PNIO_TrcPrintf("##CHAT: ");
            break;
        default:
            PNIO_TrcPrintf("##UNDEFINED LOGMSG TYPE: ");
    }
    PNIO_TrcPrintf(" PackId %d, ModId %d Line %d Device %d\n",
                   PackId, ModId, LineNum, DevHndl);
}

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
