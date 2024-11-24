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
/*  F i l e               &F: psi_cfg_pof.c                             :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Implementation of PSI POF HW system adaption                             */
/*                                                                           */
/*****************************************************************************/

#define PSI_MODULE_ID     32004
#define LTRC_ACT_MODUL_ID 32004

#include "psi_inc.h"
#include "bspadapt.h"


#if ((PSI_CFG_USE_POF == 1) && (PSI_CFG_USE_HD_COMP == 1))

#ifndef PSI_POF_FO_LED
LSA_VOID PSI_POF_FO_LED(
    LSA_UINT16        const PortID,
    LSA_UINT8         const ON,
    PSI_SYS_HANDLE    hSysDev )
{
    LSA_UNUSED_ARG(hSysDev);

    switch (PortID)
    {
        case 1:
        {
            Bsp_EbSetLed(PNIO_LED_FO1, ON);
            break;
        }
        case 2:
        {
            Bsp_EbSetLed(PNIO_LED_FO2, ON);
            break;
        }
        default:
        {
            break;
        }
    }
}
#endif

#ifndef PSI_POF_WATCHDOG
LSA_VOID PSI_POF_WATCHDOG( LSA_VOID )
{

}
#endif

#endif /* PSI_CFG_USE_POF && PSI_CFG_USE_HD_COMP  */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/