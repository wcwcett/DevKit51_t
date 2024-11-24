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
/*  F i l e               &F: bspledadapt.h                             :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  LED abstraction interface                                                */
/*                                                                           */
/*****************************************************************************/
#ifndef _LED_ADAPT_H
#define _LED_ADAPT_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

	#include "bspadapt.h"

	PNIO_VOID BspLed_Open 			(PNIO_VOID);
    PNIO_VOID BspLed_InitLedBlink   (PNIO_VOID);
    PNIO_VOID BspLed_StartLedBlink  (PNIO_UINT32 frequency);
    PNIO_VOID BspLed_StopLedBlink   (PNIO_VOID);
	PNIO_UINT32 BspLed_Switch       (PNIO_LEDTYPE Led, PNIO_UINT32 Val);



#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
