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
/*  F i l e               &F: Os_utils.c                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  OS abstraction layer implementation for a windows operating system       */
/*                                                                           */
/*****************************************************************************/


    // *---------------------------------------------------------------------
    // *    OS system adation layer 
    // *
    // *
    // *
    // *
    // *
    // *---------------------------------------------------------------------
    #include "compiler.h"
    #include "pniousrd.h"
    #include "trc_if.h"
    #include "os.h"
    #include "os_utils.h"

    #define LTRC_ACT_MODUL_ID   106
    #define	IOD_MODULE_ID       106

    // **** external variables ****
    extern  PNIO_UINT32 clock (PNIO_VOID);

    // *--------------------------------------------------*
    // *   test and debugging
    // *--------------------------------------------------*
    #define DEBUG_TEST1         1

    #define CIRCUF_HW_BASED     0    // 0:  sw based timer for OsSetCircBufEntry ()



    // *------------------------------------------------------------------------------
    // * circular buffer for debugging
    // *----------------------------------------------------------------------------*/
#if  _DEBUG_LOGGING_CIRC_BUF
    #define CIRCBUF_SIZE	2000     	  // number of entries in every circular buffer
    typedef struct
    {
        PNIO_UINT32 DatIndex;
        PNIO_UINT32 TotalCounts;
	    PNIO_INT32	MaxVal;
	    PNIO_INT32  MinVal;
        PNIO_UINT32 LastVal;
        PNIO_UINT32 Data [CIRCBUF_SIZE];
	    PNIO_INT32  Started;
    }  CIRC_BUF;
    static PNIO_UINT16 CircBufEnabled = 1;
    static CIRC_BUF CircBuf[NUMOF_CIRCBUFS];	// circular buffer for debugging




    // *--------------------------------------------*
    // *   OsEnableCircBuf (Enable)
    // *   initialize circular buffer for debbugging
    // *--------------------------------------------*
    PNIO_VOID OsEnableCircBuf (PNIO_BOOL Enable)
    {
        if (Enable == PNIO_FALSE)
        {
            CircBufEnabled = 0;
        }
        else
        {
            PNIO_INT i;
            for (i=0; i < NUMOF_CIRCBUFS; i++)
            {
                CircBuf[i].Started = 0;
            }
            CircBufEnabled = 1;
        }
    }


    // *--------------------------------------------*
    // *   OsResetMaxMinCircBuf ()
    // *   resets max/min values 
    // *--------------------------------------------*
    PNIO_VOID OsResetMaxMinCircBuf (LSA_UINT32 BufIndex)
    {
        if (BufIndex != 0xffffffff)
        {
            CircBuf[BufIndex].MaxVal = 0;           // preset to minimum possible value
            CircBuf[BufIndex].MinVal = 0x7fffffff;  // preset to maximum possible value
        }
        else
        { // reset all
            PNIO_INT i;
            for (i=0; i < NUMOF_CIRCBUFS; i++)
            {
                CircBuf[i].MinVal = 0x7fffffff;  // preset to maximum possible value first
                CircBuf[i].MaxVal = 0;           // preset to minimum possible value
            }
        }
    }

    // *--------------------------------------------*
    // *   OsInitCircBuf (PNIO_VOID)
    // *   initialize circular buffer for debbugging
    // *--------------------------------------------*
    PNIO_VOID OsInitCircBuf (PNIO_VOID)
    {
        OsEnableCircBuf(PNIO_FALSE);   // first disable 

        OsMemSet((PNIO_INT8*)CircBuf, 0, sizeof (CircBuf));

        OsResetMaxMinCircBuf(0xffffffff);
    }

    // *--------------------------------------------*
    // *   OsGetLastEntryCircBuf (LSA_UINT32 Index)
    // *   input:   Index		 Bufferindex
    // *--------------------------------------------*
    PNIO_UINT32 OsGetLastEntryCircBuf (LSA_UINT32 BufIndex)
    {
        PNIO_UINT32 EntryIndex;
        EntryIndex = CircBuf[BufIndex].DatIndex;
        if (EntryIndex == 0)
            EntryIndex = CIRCBUF_SIZE - 1;
        else
            EntryIndex--;
        return (CircBuf[BufIndex].Data[EntryIndex]);

    }

    // *--------------------------------------------*
    // *   OsGetMaxValCircBuf   (LSA_UINT32 Index)
    // *   input:   Index		 Bufferindex
    // *--------------------------------------------*
    PNIO_UINT32 OsGetMaxValCircBuf (LSA_UINT32 BufIndex)
    {
        return (CircBuf[BufIndex].MaxVal);
    }

    // *--------------------------------------------*
    // *   OsGetLastEntryCircBuf (LSA_UINT32 Index)
    // *   input:   Index		 Bufferindex
    // *--------------------------------------------*
    PNIO_UINT32 OsGetMinValCircBuf (LSA_UINT32 BufIndex)
    {
        return (CircBuf[BufIndex].MinVal);
    }


    // *--------------------------------------------*
    // *   OsSetEntryCircBuf (LSA_UINT32 Index, LSA_UINT32 DisableInt))
    // *   make new entry in  circular buffer for debbugging
    // *   input:   Index		 Bufferindex
    // *            Func       0: default, 1: store deltas
    // *--------------------------------------------*
    PNIO_VOID OsSetEntryCircBuf (LSA_UINT32 Index, LSA_UINT32 func)
	{
    	CIRC_BUF*  pCircBuf = &CircBuf[Index];
    	if (Index >= NUMOF_CIRCBUFS)
    	{
    		LSA_TRACE_00  (TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "invalid circular buffer index\n");
    		return;				// if fatal returns...
    	}

    	if (CircBufEnabled)
		{
			PNIO_UINT32 NewTim ;

			#if CIRCUF_HW_BASED
			NewTim = OsGetTime_us();
			#else
			NewTim = clock();
			#endif

			switch (func)
			{
				case OS_ENTRY_DEFAULT:
				{
					// *-------------------------------------------------
					// *  default
					// *-------------------------------------------------
					pCircBuf->Data[CircBuf[Index].DatIndex] = NewTim;

					pCircBuf->TotalCounts++;
					pCircBuf->DatIndex++;
					if (pCircBuf->DatIndex >= CIRCBUF_SIZE)
					{
						pCircBuf->DatIndex = 0;
					}
					break;
				}
				case OS_ENTRY_START:
				{
					// *-------------------------------------------------
					// *  start new measure cycle
					// *-------------------------------------------------
					pCircBuf->LastVal = NewTim;
					pCircBuf->Started = 1;
					break;
				}
				case OS_ENTRY_STOP:
				{
					// *-------------------------------------------------
					// *  finish measure cycle, save only time difference
					// *-------------------------------------------------
					if (pCircBuf->Started)
					{
						PNIO_INT32 dTim = (PNIO_INT32) NewTim - pCircBuf->LastVal;
						pCircBuf->Started = 0;
						if (dTim < 0)
						{
							dTim -= dTim;
						}
						pCircBuf->Data[pCircBuf->DatIndex] = dTim;

						pCircBuf->TotalCounts++;
						pCircBuf->DatIndex++;

						if (pCircBuf->DatIndex >= CIRCBUF_SIZE)
						{
							pCircBuf->DatIndex = 0;
						}
						// **** save MaxVal ****
						if (pCircBuf->MaxVal < dTim)
						{
							pCircBuf->MaxVal = dTim;
						}
						if (pCircBuf->MinVal > dTim)
						{
							pCircBuf->MinVal = dTim;
						}
					}
					break;
				}

				default:
				{

				}
			}
		}
    }

    // *--------------------------------------------*
    // *   OsSetEntryCircBufX (PNIO_UINT32 Index, value)
    // *   stores the value in buffer index
    // *--------------------------------------------*
    PNIO_VOID OsSetEntryCircBufX (PNIO_UINT32 Index, PNIO_UINT32 value)
    {
		CIRC_BUF*  pCircBuf = &CircBuf[Index];
		if (CircBufEnabled)
		{
			if (Index >= NUMOF_CIRCBUFS)
			{
				LSA_TRACE_00  (TRACE_SUBSYS_IOD_SYSADAPT, LSA_TRACE_LEVEL_FATAL, "invalid circular buffer index\n");
				return;				// if fatal returns...
			}

			pCircBuf->Data[CircBuf[Index].DatIndex] = value;
			pCircBuf->TotalCounts++;
			pCircBuf->DatIndex++;
			if (pCircBuf->DatIndex >= CIRCBUF_SIZE)
			{
				pCircBuf->DatIndex = 0;
			}
			// **** save MaxVal ****
			if (pCircBuf->MaxVal < (PNIO_INT32)value)
			{
				pCircBuf->MaxVal = (PNIO_INT32)value;
			}
			if (pCircBuf->MinVal > (PNIO_INT32)value)
			{
				pCircBuf->MinVal = (PNIO_INT32)value;
			}
		}
    }


    // *--------------------------------------------*
    // *   OsPrintCircBuf (Enable)
    // *   initialize circular buffer for debbugging
    // *--------------------------------------------*
    PNIO_VOID OsPrintCircBuf (PNIO_UINT32 BufIndex)
    {
        PNIO_UINT32 i;
        PNIO_UINT32 Fillstate;
        PNIO_UINT32 DatInd;

        if (CircBuf[BufIndex].TotalCounts >= CIRCBUF_SIZE)
        {
            DatInd = CircBuf[BufIndex].DatIndex;
            Fillstate = CIRCBUF_SIZE;
        }
        else
        {
            DatInd = 0;
            Fillstate = CircBuf[BufIndex].TotalCounts;
        }



        for (i=0; i < Fillstate; i= i+1)
        {
            
            PNIO_printf ( "CBuf [%d] = %d\n",
                          DatInd,
                          CircBuf[BufIndex].Data[DatInd]
                        );

            DatInd++;
            if (DatInd >= CIRCBUF_SIZE)
                DatInd = 0;

            if ((i % 25) == 0)
                OsWait_ms (50);
        }
        
        PNIO_printf ("Print CircBuf [%d], Fill = %d MaxVal = %d MinVal = %d\n", 
        			 BufIndex, 
        			 Fillstate,
        			 CircBuf[BufIndex].MaxVal,
        			 CircBuf[BufIndex].MinVal);
  }
#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
