/*********************************************************************************************************************/
/**@file        pdrv_statemachine_ac4.h
 * @brief       PROFIdrive state machines header file
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
 *              State machines implements the PROFIdrive state machines
 *
 * @internal
 * @note        Doxygen comments are used for automatic documentation generation.
 *              For further information about Doxygen please look at http://www.stack.nl/~dimitri/doxygen/index.html
 *              IDE Eclipse's folding feature is used too. see https://www.eclipse.org
 * @endinternal
*/
/*********************************************************************************************************************/

#ifndef PDRV_STATEMACHINE_AC4_H
#define PDRV_STATEMACHINE_AC4_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

/** General Axis States, see PDRV V4.2 figure 27 General State Diagram */
typedef enum
{
    AXISSTATE_POWERON,                  /**< state after power on */
    AXISSTATE_IDLE,                     /**< state with no control by PLC */
    AXISSTATE_SWITCHING_ON_INHIBITED,   /**< general state S1: Switching On Inhibited*/
    AXISSTATE_READY_FOR_SWITCHING_ON,   /**< general state S2: Ready For Switching On */
    AXISSTATE_SWITCHED_ON,              /**< general state S3: Switched On */
    AXISSTATE_OPERATION,                /**< general state S4: Operation */
    AXISSTATE_RAMP_STOP,                /**< general state S5A: Switching Off with Ramp Stop */
    AXISSTATE_QUICK_STOP,               /**< general state S5B: Switching Off with Quick Stop */
    AXISSTATE_STATEMACHINE_ERROR        /**< Error of the General State Machine */
} PDRV_AXISSTATES_TYPE;

extern PDRV_AXISSTATES_TYPE ePdrvSma_AxisGeneralStateMachine (PDRV_VOID);
extern PDRV_AXISSTATES_TYPE ePdrvSma_GetAxisMainState(PDRV_VOID);

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif
