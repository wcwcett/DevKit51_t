/*********************************************************************************************************************/
/**@file        pdrv_setpointchannel_ac4.h
 * @brief       PROFIdrive setpoint channel header file
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
 *              Shorthand symbol: Spc (SetPoint Channel)<br>
 *              Setpoint channel implements the handling of setpoint
 *
 * @internal
 * @note        Doxygen comments are used for automatic documentation generation.
 *              For further information about Doxygen please look at http://www.stack.nl/~dimitri/doxygen/index.html
 *              IDE Eclipse's folding feature is used too. see https://www.eclipse.org
 * @endinternal
*/
/*********************************************************************************************************************/

#ifndef PDRV_SETPOINTCHANNEL_AC4_H
#define PDRV_SETPOINTCHANNEL_AC4_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

/** @name STW1 bit masks
  * : control bit masks at control word 1 STW1 for speed control
  * @{
*/
#define PDRV_STW1_NOOFF1    (1U << 0U)  /**< 0 = OFF1 active (power down),<br> 1 = main contact should be closed */
#define PDRV_STW1_NOOFF2    (1U << 1U)  /**< 0 = OFF2 active (coast stop),<br> 1 = no coast stop */
#define PDRV_STW1_NOOFF3    (1U << 2U)  /**< 0 = OFF3 active (quick stop),<br> 1 = no quick stop */
#define PDRV_STW1_OPENABLE  (1U << 3U)  /**< 1 = enable operation */
#define PDRV_STW1_RFGENABLE (1U << 4U)  /**< 1 = enable ramp function generator (RFG) */
#define PDRV_STW1_RFGUNFREZ (1U << 5U)  /**< 0 = freeze RFG; 1 = unfreeze RFG */
#define PDRV_STW1_RFGSETPNT (1U << 6U)  /**< 0 = disable RFG setpoint,<br> 1 = enable RFG setpoint */
#define PDRV_STW1_FAULTACK  (1U << 7U)  /**< fault acknowledge with positive edge 0->1 */
#define PDRV_STW1_RFGJOG1   (1U << 8U)  /**< 1 = set RFG setpoint to jogging setpoint 1 */
#define PDRV_STW1_RFGJOG2   (1U << 9U)  /**< 1 = set RFG setpoint to jogging setpoint 2 */
#define PDRV_STW1_PLCCTRL   (1U <<10U)  /**< 0 = no control by PLC,<br> 1 = control by PLC */
/* @} */

/** @name ZSW1 bit masks
  * : status bit masks at status word 1 ZSW1 for speed control
  * @{
*/
#define PDRV_ZSW1_R2SWION   (1U << 0U)  /**< 1 = ready to switch on */
#define PDRV_ZSW1_R2OPERATE (1U << 1U)  /**< 1 = ready to operate */
#define PDRV_ZSW1_OPENABLE  (1U << 2U)  /**< 1 = operation enabled, drive follows setpoint */
#define PDRV_ZSW1_FAULT     (1U << 3U)  /**< 1 = fault is present */
#define PDRV_ZSW1_NOOFF2    (1U << 4U)  /**< 0 = OFF2 command is present (coast stop),<br> 1 = no coast stop */
#define PDRV_ZSW1_NOOFF3    (1U << 5U)  /**< 0 = OFF3 command is present (quick stop),<br> 1 = no quick stop */
#define PDRV_ZSW1_SWIONINHI (1U << 6U)  /**< 1 = switching on inhibited */
#define PDRV_ZSW1_WARNING   (1U << 7U)  /**< 1 = warning is present */
#define PDRV_ZSW1_NOSPDERR  (1U << 8U)  /**< 0 = speed error is out of tolerance,<br> 1 = speed error within tolerance */
#define PDRV_ZSW1_CTRLREQ   (1U << 9U)  /**< 1 = control requested */
#define PDRV_ZSW1_FNREACHED (1U <<10U)  /**< 1 = comparison value for f or n is reached or exceeded */
/* @} */

/** @name Flags of received data events
  * : bit mask for different events after power on
  * @{
*/
#define FIRSTIO_EVENT   (0x01U) /**< PNIO stack informs about first received IO data */
#define FIRSTIO_STW1    (0x02U) /**< first received STW1 is latched into PDRV */
/* @} */

/** @name Closed loop control operating modes
  * : see PDRV V4.2 chapter 6.4.1.6
  * @{
*/
#define PDRV_CLCOPMODE_RFG      (1U)    /**< speed control mode with full RFG */
#define PDRV_CLCOPMODE_POS      (2U)    /**< positioning mode */
#define PDRV_CLCOPMODE_REDSSC   (3U)    /**< speed control mode with reduced speed setpoint channel */
/* @} */


extern PDRV_VOID PdrvSpc_NotifyFirstIO(PDRV_VOID);
extern PDRV_UINT uPdrvSpc_GetFirstIOState(PDRV_VOID);
extern PDRV_VOID PdrvSpc_SetStw1(PDRV_UINT16 p_uStw1);
extern PDRV_VOID PdrvSpc_SetStw2(PDRV_UINT16 p_uStw2);
extern PDRV_VOID PdrvSpc_SetNsollA(PDRV_INT16 p_nNsollA);
extern PDRV_VOID PdrvSpc_SetNsollB(PDRV_INT32 p_nNsollB);
extern PDRV_VOID PdrvSpc_SetG1Stw(PDRV_UINT16 p_uG1Stw);
extern PDRV_VOID PdrvSpc_SetG1Zsw(PDRV_UINT16 p_uG1Zsw);
extern PDRV_VOID PdrvSpc_SetG1Xist1(PDRV_INT32 p_nG1Xist1);
extern PDRV_VOID PdrvSpc_SetG1Xist2(PDRV_INT32 p_nG1Xist2);
extern PDRV_UINT16 uPdrvSpc_GetStw1(PDRV_VOID);
extern PDRV_UINT16 uPdrvSpc_GetStw2(PDRV_VOID);
extern PDRV_UINT16 uPdrvSpc_GetZsw1(PDRV_VOID);
extern PDRV_UINT16 uPdrvSpc_GetZsw2(PDRV_VOID);
extern PDRV_UINT16 uPdrvSpc_GetG1Stw(PDRV_VOID);
extern PDRV_UINT16 uPdrvSpc_GetG1Zsw(PDRV_VOID);
extern PDRV_INT16 nPdrvSpc_GetNsollA(PDRV_VOID);
extern PDRV_INT32 nPdrvSpc_GetNsollB(PDRV_VOID);
extern PDRV_INT16 nPdrvSpc_GetNistA(PDRV_VOID);
extern PDRV_INT32 nPdrvSpc_GetNistB(PDRV_VOID);
extern PDRV_UINT32 uPdrvSpc_GetG1Xist1(PDRV_VOID);
extern PDRV_UINT32 uPdrvSpc_GetG1Xist2(PDRV_VOID);
extern PDRV_VOID PdrvSpc_SetBitsZsw1(PDRV_UINT16 p_uSetBit);
extern PDRV_VOID PdrvSpc_ClrBitsZsw1(PDRV_UINT16 p_uClrBit);
extern PDRV_VOID PdrvSpc_SetZsw2(PDRV_UINT16 p_uZsw2, PDRV_UINT16 p_uMsk);
extern PDRV_VOID PdrvSpc_SetNistA(PDRV_INT16 p_nNistA);
extern PDRV_VOID PdrvSpc_SetNistB(PDRV_INT32 p_nNistB);
extern PDRV_INT32 nPdrvSpc_CalcRfg (PDRV_INT32 p_nSetPnt, PDRV_INT32 p_nRFGStp);
extern PDRV_INT32 nPdrvSpc_CalcReducedSsc (PDRV_INT32 p_nSetPnt, PDRV_INT32 p_nRFGStp);
extern PDRV_BOOL bPdrvSpc_CalcSpeedWithinTolerance(PDRV_INT32 p_nNistB, PDRV_INT32 p_nNsollB, PDRV_INT32 p_nNTolerance, PDRV_UINT16 p_uTmax);
extern PDRV_BOOL bPdrvSpc_CalcSpeedReached(PDRV_INT32 p_nNistB, PDRV_INT32 p_nNcomp);
extern PDRV_INT32 nPdrvSpc_CalcSsc(PDRV_INT32 p_nSpeedRfg,  PDRV_INT32 p_nRampStp, PDRV_INT32 p_nQuickStp);

extern PDRV_UINT16 uPdrvSpc_GetOperatingMode(PDRV_VOID);

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif
