/*********************************************************************************************************************/
/**@file        pdrv_synchronisation_ac4.h
 * @brief       PROFIdrive synchronization header file
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
 *              Shorthand symbol: SYN (SYNchronisation)<br>
 *              State machines implements the PROFIdrive synchronisation mechanism
 *
 * @internal
 * @note        Doxygen comments are used for automatic documentation generation.
 *              For further information about Doxygen please look at http://www.stack.nl/~dimitri/doxygen/index.html
 *              IDE Eclipse's folding feature is used too. see https://www.eclipse.org
 * @endinternal
*/
/*********************************************************************************************************************/

#ifndef PDRV_SYNCHRONISATION_AC4_H
#define PDRV_SYNCHRONISATION_AC4_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

/** Synchronisation States, see PDRV V4.2 figure 108 */
typedef enum
 {
    SYNCSTATE_IDLE,         /**< idle state i.e. no synchronisations phase */
    SYNCSTATE_CSYNCING00,   /**< synchronisation to controller sign of life, at least 16 valid signs of life */
    SYNCSTATE_CSYNCING01,   /**< syncing to C-LS after 1st continuous valid signs of life */
    SYNCSTATE_CSYNCING02,   /**< syncing to C-LS after 2nd continuous valid signs of life */
    SYNCSTATE_CSYNCING03,   /**< syncing to C-LS after 3rd continuous valid signs of life */
    SYNCSTATE_CSYNCING04,   /**< syncing to C-LS after 4th continuous valid signs of life */
    SYNCSTATE_CSYNCING05,   /**< syncing to C-LS after 5th continuous valid signs of life */
    SYNCSTATE_CSYNCING06,   /**< syncing to C-LS after 6th continuous valid signs of life */
    SYNCSTATE_CSYNCING07,   /**< syncing to C-LS after 7th continuous valid signs of life */
    SYNCSTATE_CSYNCING08,   /**< syncing to C-LS after 8th continuous valid signs of life */
    SYNCSTATE_CSYNCING09,   /**< syncing to C-LS after 9th continuous valid signs of life */
    SYNCSTATE_CSYNCING10,   /**< syncing to C-LS after 10th continuous valid signs of life */
    SYNCSTATE_CSYNCING11,   /**< syncing to C-LS after 11th continuous valid signs of life */
    SYNCSTATE_CSYNCING12,   /**< syncing to C-LS after 12th continuous valid signs of life */
    SYNCSTATE_CSYNCING13,   /**< syncing to C-LS after 13th continuous valid signs of life */
    SYNCSTATE_CSYNCING14,   /**< syncing to C-LS after 14th continuous valid signs of life */
    SYNCSTATE_CSYNCING15,   /**< syncing to C-LS after 15th continuous valid signs of life */
    SYNCSTATE_DSYNCING00,   /**< synchronisation to drive object sign of life, at least 16 valid signs of life */
    SYNCSTATE_DSYNCING01,   /**< syncing to DO-LS after 1st continuous valid signs of life */
    SYNCSTATE_DSYNCING02,   /**< syncing to DO-LS after 2nd continuous valid signs of life */
    SYNCSTATE_DSYNCING03,   /**< syncing to DO-LS after 3rd continuous valid signs of life */
    SYNCSTATE_DSYNCING04,   /**< syncing to DO-LS after 4th continuous valid signs of life */
    SYNCSTATE_DSYNCING05,   /**< syncing to DO-LS after 5th continuous valid signs of life */
    SYNCSTATE_DSYNCING06,   /**< syncing to DO-LS after 6th continuous valid signs of life */
    SYNCSTATE_DSYNCING07,   /**< syncing to DO-LS after 7th continuous valid signs of life */
    SYNCSTATE_DSYNCING08,   /**< syncing to DO-LS after 8th continuous valid signs of life */
    SYNCSTATE_DSYNCING09,   /**< syncing to DO-LS after 9th continuous valid signs of life */
    SYNCSTATE_DSYNCING10,   /**< syncing to DO-LS after 10th continuous valid signs of life */
    SYNCSTATE_DSYNCING11,   /**< syncing to DO-LS after 11th continuous valid signs of life */
    SYNCSTATE_DSYNCING12,   /**< syncing to DO-LS after 12th continuous valid signs of life */
    SYNCSTATE_DSYNCING13,   /**< syncing to DO-LS after 13th continuous valid signs of life */
    SYNCSTATE_DSYNCING14,   /**< syncing to DO-LS after 14th continuous valid signs of life */
    SYNCSTATE_DSYNCING15,   /**< syncing to DO-LS after 15th continuous valid signs of life */
    SYNCSTATE_OPERATION     /**< synchronised operation */
 } PDRV_SYNCSTATES_TYPE;

#define PDRV_SYNC_LS_OFF   (0xFFFFUL)  /**< PNU00925 value if sign of life synchronisation is switched off */

extern PDRV_VOID PdrvSyn_CheckSignOfLife(PDRV_VOID);
extern PDRV_SYNCSTATES_TYPE ePdrvSyn_GetSignOfLifeState(PDRV_VOID);
extern PDRV_UINT32 uPdrvSyn_GetFailCnt(PDRV_VOID);
extern PDRV_UINT16 uPdrvSyn_GetNoCLSFaults(PDRV_VOID);

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif
