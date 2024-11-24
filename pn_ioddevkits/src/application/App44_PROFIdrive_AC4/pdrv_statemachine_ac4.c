/*********************************************************************************************************************/
/**@file        pdrv_statemachine_ac4.c
 * @brief       PROFIdrive General State Machine
 *
 * @author      Siemens AG
 * @copyright   Copyright (C) Siemens AG 2017. All rights reserved.
 *              This program is protected by German copyright law and international treaties.
 *              The use of this software including but not limited to its Source Code is subject to restrictions as
 *              agreed in the license agreement between you and Siemens.
 *              Copying or distribution is not allowed unless expressly permitted according to your license agreement
 *              with Siemens.
 *
 * @version     V1.0
 *
 * @details     Project: PROFIdrive Application Example<br>
 *              Shorthand symbol: SMA (StateMAchine)<br>
 *              The file implements the PROFIdrive general state machine.
 *
 * @internal
 * @note        Doxygen comments are used for automatic documentation generation.
 *              For further information about Doxygen please look at http://www.stack.nl/~dimitri/doxygen/index.html
 *              IDE Eclipse's folding feature is used too. see https://www.eclipse.org
 * @endinternal
*/
/*********************************************************************************************************************/

/*------------  header includes  ------------*/
#include "iod_cfg.h"
#include "usriod_cfg.h" /* for definition of EXAMPL_DEV_CONFIG_VERSION needed */

#include "pdrv_types_ac4.h"  /* PROFIdrive datatype definitions */
#include "pdrv_statemachine_ac4.h"
#include "pdrv_setpointchannel_ac4.h"
#include "pdrv_synchronisation_ac4.h"
#include "pdrv_application_ac4.h"
#include "pdrv_diagnostics_ac4.h"

#if (EXAMPL_DEV_CONFIG_VERSION == 44)

/*------------  extern  functions  ------------*/
extern PDRV_BOOL bPdrvUsr_IsIsoReq(PDRV_VOID);

/*------------  extern  data  ------------*/

/*------------  type definitions, constants, enums  ------------*/

/*------------  static  data  ------------*/

static PDRV_AXISSTATES_TYPE m_eAxisState = AXISSTATE_POWERON;    /**< state for the general state machine */

/*------------  public data  ------------*/

/*------------  functions  ------------*/

/** PROFIdrive General State Machine
 * @details Implements the PROFIdrive General State Machine according General State Diagram (PDRV V4.2 Figure 27)
 * @return  new axis state calculated
*/
PDRV_AXISSTATES_TYPE ePdrvSma_AxisGeneralStateMachine (PDRV_VOID)
{
    PDRV_UINT16 uStw1 = uPdrvSpc_GetStw1();    /* local copy of control word 1 STW1 */
    PDRV_AXISSTATES_TYPE eAxisState;        /* new drive state */

    eAxisState =  m_eAxisState;   /* get state before */

    /* consider fault reactions OFF1, OFF2, OFF3 */
    if (bPdrvDiag_IsFaultOff1())
    {
        uStw1 &= ~PDRV_STW1_NOOFF1;
    }
    if (bPdrvDiag_IsFaultOff2())
    {
        uStw1 &= ~PDRV_STW1_NOOFF2;
    }
    if (bPdrvDiag_IsFaultOff3())
    {
        uStw1 &= ~PDRV_STW1_NOOFF3;
    }

    switch(eAxisState)
    {
        case AXISSTATE_POWERON:
        {
            /* established PROFInet connection? */
            if (uPdrvSpc_GetFirstIOState() == (FIRSTIO_EVENT + FIRSTIO_STW1))
            {
                eAxisState = AXISSTATE_IDLE;
            }
            break;
        }
        case AXISSTATE_IDLE:
        {
            /* axis controlled by PLC? */
            if (uStw1 & PDRV_STW1_PLCCTRL)
            {
                eAxisState = AXISSTATE_SWITCHING_ON_INHIBITED;
            }
            break;
        }
        case AXISSTATE_SWITCHING_ON_INHIBITED:
        {
            if (   !(uStw1 & PDRV_STW1_NOOFF1) /*     OFF */
                &&  (uStw1 & PDRV_STW1_NOOFF2) /* AND No Coast Stop */
                &&  (uStw1 & PDRV_STW1_NOOFF3) /* AND No Quick Stop */
               )
            {
                /* application ready for next state AND sync ok? */
                if (   (bPdrvApp_IsTransitionCondition(eAxisState, AXISSTATE_READY_FOR_SWITCHING_ON))
                    && (   (bPdrvUsr_IsIsoReq() == PDRV_FALSE)                      /* no sync necessary */
                        || (uPdrvSyn_GetNoCLSFaults() == PDRV_SYNC_LS_OFF)          /* OR sync switched off */
                        || (ePdrvSyn_GetSignOfLifeState() == SYNCSTATE_OPERATION)   /* OR synced */
                       )
                   )
                {
                    eAxisState = AXISSTATE_READY_FOR_SWITCHING_ON;
                }
            }
          break;
        }
        case AXISSTATE_READY_FOR_SWITCHING_ON:
        {
            if (   !(uStw1 & PDRV_STW1_NOOFF2) /*    Coast Stop */
                || !(uStw1 & PDRV_STW1_NOOFF3) /* OR Quick Stop */
               )
            {
                eAxisState = AXISSTATE_SWITCHING_ON_INHIBITED;
            }
            else if (uStw1 & PDRV_STW1_NOOFF1)    /* ON */
            {
                /* application ready for next state? */
                if (bPdrvApp_IsTransitionCondition(eAxisState, AXISSTATE_SWITCHED_ON))
                {
                    eAxisState = AXISSTATE_SWITCHED_ON;
                }
            }
            break;
        }
        case AXISSTATE_SWITCHED_ON:
        {
            if (   !(uStw1 & PDRV_STW1_NOOFF2) /*    Coast Stop */
                || !(uStw1 & PDRV_STW1_NOOFF3) /* OR Quick Stop */
               )
            {
                eAxisState = AXISSTATE_SWITCHING_ON_INHIBITED;
            }
            else if (!(uStw1 & PDRV_STW1_NOOFF1))    /* OFF */
            {
                eAxisState = AXISSTATE_READY_FOR_SWITCHING_ON;
            }
            else if (uStw1 & PDRV_STW1_OPENABLE)  /* Enable Operation */
            {
                /* application ready for next state? */
                if (bPdrvApp_IsTransitionCondition(eAxisState, AXISSTATE_OPERATION))
                {
                    eAxisState = AXISSTATE_OPERATION;
                }
            }
            break;
        }
        case AXISSTATE_OPERATION:
        {
            if (!(uStw1 & PDRV_STW1_NOOFF2))   /* Coast Stop */
            {
                eAxisState = AXISSTATE_SWITCHING_ON_INHIBITED;
            }
            else if (!(uStw1 & PDRV_STW1_NOOFF3))   /* Quick Stop */
            {
                eAxisState = AXISSTATE_QUICK_STOP;
            }
            else if (!(uStw1 & PDRV_STW1_NOOFF1))   /* OFF */
            {
                eAxisState = AXISSTATE_RAMP_STOP;
            }
            else if (!(uStw1 & PDRV_STW1_OPENABLE)) /* Disable Operation */
            {
                eAxisState = AXISSTATE_SWITCHED_ON;
            }
            break;
        }
        case AXISSTATE_RAMP_STOP:
        {
            if (!(uStw1 & PDRV_STW1_NOOFF2))   /* Coast Stop */
            {
                eAxisState = AXISSTATE_SWITCHING_ON_INHIBITED;
            }
            else if (!(uStw1 & PDRV_STW1_NOOFF3))   /* Quick Stop */
            {
                eAxisState = AXISSTATE_QUICK_STOP;
            }
            else if (   !(uStw1 & PDRV_STW1_OPENABLE)               /* Disable Operation */
                     ||  (bPdrvApp_IsAxisStandstill() == PDRV_TRUE) /* Standstill detected */
                    )
            {
                eAxisState = AXISSTATE_READY_FOR_SWITCHING_ON;
            }
            else if (uStw1 & PDRV_STW1_NOOFF1)    /* ON */
            {
                eAxisState = AXISSTATE_OPERATION;
            }
            break;
        }
        case AXISSTATE_QUICK_STOP:
        {
            if (!(uStw1 & PDRV_STW1_NOOFF2))  /* Coast Stop */
            {
                eAxisState = AXISSTATE_SWITCHING_ON_INHIBITED;
            }
            else if (   !(uStw1 & PDRV_STW1_OPENABLE)               /* Disable Operation */
                     ||  (bPdrvApp_IsAxisStandstill() == PDRV_TRUE) /* Standstill detected */
                    )
            {
                eAxisState = AXISSTATE_SWITCHING_ON_INHIBITED;
            }
            break;
        }
        default:
        {
            eAxisState = AXISSTATE_STATEMACHINE_ERROR;
            break;
        }
    }

    /* set ZSW1 bit 4 "coast stop is present" (PDRV V4.2 figure 139) */
    if (uStw1 & PDRV_STW1_NOOFF2)
    {   /* 1 = NO OFF2 command (no coast stop) */
        PdrvSpc_SetBitsZsw1(PDRV_ZSW1_NOOFF2);
    }
    else
    {   /* 0 = OFF2 command is present (coast stop) */
        PdrvSpc_ClrBitsZsw1(PDRV_ZSW1_NOOFF2);
    }

    /* set ZSW1 bit 5 "quick stop is present" (PDRV V4.2 figure 139) */
    if (   (uStw1 & PDRV_STW1_NOOFF3)
        && (eAxisState != AXISSTATE_QUICK_STOP)
       )
    {   /* 1 = NO OFF3 command is present (no quick stop) */
        PdrvSpc_SetBitsZsw1(PDRV_ZSW1_NOOFF3);
    }
    else
    {   /* 0 = OFF3 command is present (quick stop) */
        PdrvSpc_ClrBitsZsw1(PDRV_ZSW1_NOOFF3);
    }

    /* changed drive state? */
    if (eAxisState != m_eAxisState)
    {
        m_eAxisState = eAxisState;
        /* set ZSW1 bits according new state */
        switch(eAxisState)
        {
            case AXISSTATE_IDLE:
            {
                PdrvSpc_ClrBitsZsw1(PDRV_ZSW1_R2SWION | PDRV_ZSW1_R2OPERATE | PDRV_ZSW1_OPENABLE);
                PdrvSpc_SetBitsZsw1(PDRV_ZSW1_SWIONINHI | PDRV_ZSW1_CTRLREQ);
                break;
            }
            case AXISSTATE_SWITCHING_ON_INHIBITED:
            {
                PdrvSpc_ClrBitsZsw1(PDRV_ZSW1_R2SWION | PDRV_ZSW1_R2OPERATE | PDRV_ZSW1_OPENABLE);
                PdrvSpc_SetBitsZsw1(PDRV_ZSW1_SWIONINHI);
                break;
            }
            case AXISSTATE_READY_FOR_SWITCHING_ON:
            {
                PdrvSpc_ClrBitsZsw1(PDRV_ZSW1_R2OPERATE | PDRV_ZSW1_OPENABLE | PDRV_ZSW1_SWIONINHI);
                PdrvSpc_SetBitsZsw1(PDRV_ZSW1_R2SWION);
                break;
            }
            case AXISSTATE_SWITCHED_ON:
            case AXISSTATE_RAMP_STOP:
            case AXISSTATE_QUICK_STOP:
            {
                PdrvSpc_ClrBitsZsw1(PDRV_ZSW1_OPENABLE | PDRV_ZSW1_SWIONINHI);
                PdrvSpc_SetBitsZsw1(PDRV_ZSW1_R2SWION | PDRV_ZSW1_R2OPERATE);
                break;
            }
            case AXISSTATE_OPERATION:
            {
                PdrvSpc_ClrBitsZsw1(PDRV_ZSW1_SWIONINHI);
                PdrvSpc_SetBitsZsw1(PDRV_ZSW1_R2SWION | PDRV_ZSW1_R2OPERATE | PDRV_ZSW1_OPENABLE);
                break;
            }
            case AXISSTATE_POWERON:
            case AXISSTATE_STATEMACHINE_ERROR:
            default:
            {
                PdrvSpc_ClrBitsZsw1(PDRV_ZSW1_R2SWION | PDRV_ZSW1_R2OPERATE | PDRV_ZSW1_OPENABLE | PDRV_ZSW1_CTRLREQ
                               | PDRV_ZSW1_SWIONINHI);
                break;
            }
        }
    }
    return eAxisState;
}

/** Get the main state of the axis
 * @return  axis state calculated by PdrvSma_AxisGeneralStateMachine()
*/
PDRV_AXISSTATES_TYPE ePdrvSma_GetAxisMainState(PDRV_VOID)
{
    return m_eAxisState;
}

#endif
