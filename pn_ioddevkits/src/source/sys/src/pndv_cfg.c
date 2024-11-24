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
/*  F i l e               &F: pndv_cfg.c                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  system adaption of this component                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* contents:

    - pndv_maint_led_control
    - pndv_check_maint_info
*/
/*****************************************************************************/

#define PNDV_MODULE  PNDV_ERR_MODULE_CFG

#include "pndv_inc.h"

#define PNDV_MAINT_LED  3

PNIO_VOID pndv_maint_led_control(LSA_UINT32 sf_led_mode)
{
    /* Dependency of the Maint-LED on the SF-LED */
    /* Special feature ecoPN: Maint-LED and SF-LED under one light guide */
    /* only one can light up -> SF-LED has priority */

    if (LSA_TRUE == sf_led_mode)
    {
        if (LSA_TRUE == pndv_glob_data.save_maint_on)
        {
            pndv_glob_data.maint_on = LSA_FALSE;
            pndv_debug_extern(0xF0 | PNDV_MAINT_LED, 0);

            PNDV_LED_CTRL_MAINT_OFF();
        }
    }
    else
    {
    /* if the SF event disappears and maintenance is still pending, it must go ON */
        if (LSA_TRUE == pndv_glob_data.save_maint_on)
        {
            pndv_glob_data.maint_on = LSA_TRUE;
            pndv_debug_extern(0xF0 | PNDV_MAINT_LED, 1);

            PNDV_LED_CTRL_MAINT_ON();
        }
    }
}


/*****************************************************************************/


PNIO_VOID pndv_check_maint_info(PNIO_VOID)
{
    /* Maintenance behavior according to SPH PDEV V2.0i09 */
    switch (pndv_glob_data.led_maint_info & ~CM_MAINTENANCE_STATUS_QUA_BITS)
    {
        case CM_MAINTENANCE_STATUS_DEMANDED:
        case (CM_MAINTENANCE_STATUS_DEMANDED | CM_MAINTENANCE_STATUS_REQUIRED):
        case ~CM_MAINTENANCE_STATUS_QUA_BITS:
        {
            /* maintenance demanded -> MAINT-LED ON */

            pndv_glob_data.save_maint_on = PNIO_TRUE;

            if (PNIO_TRUE != pndv_glob_data.maint_on)
            {
                pndv_glob_data.maint_on = PNIO_TRUE;
                pndv_debug_extern(0xF0 | PNDV_MAINT_LED, 1);

                PNDV_LED_CTRL_MAINT_ON();
            }

            break;
        }
        default :
        {
            /* maintenance required = CM_MAINTENANCE_STATUS_REQUIRED -> MAINT-LED OFF */
            /* no maintenance       = 0                              -> MAINT-LED OFF */

            pndv_glob_data.save_maint_on = PNIO_FALSE;

            if (PNIO_FALSE != pndv_glob_data.maint_on)
            {
                pndv_glob_data.maint_on = PNIO_FALSE;
                pndv_debug_extern( 0xF0 | PNDV_MAINT_LED, 0 );

                PNDV_LED_CTRL_MAINT_OFF();
            }

            break;
        }
    }
}

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
