/*********************************************************************************************************************/
/**@file        pdrv_application_ac4.h
 * @brief       PROFIdrive application header file
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
 *              Shorthand symbol: App (APPlication)<br>
 *
 * @internal
 * @note        Doxygen comments are used for automatic documentation generation.
 *              For further information about Doxygen please look at http://www.stack.nl/~dimitri/doxygen/index.html
 *              IDE Eclipse's folding feature is used too. see https://www.eclipse.org
 * @endinternal
*/
/*********************************************************************************************************************/

#ifndef PDRV_APPLICATION_AC4_H
#define PDRV_APPLICATION_AC4_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

/** @name Identification data
  * : data is used in PNU00964 "drive unit identification" and  PNU00975 "drive object identification"
  * @attention TODO Change all this data to your own data
  * @{
*/
#define PDRV_ID_MANUFACTURER    (0x002AU)   /**< Manufacturer: Siemens AG */
#define PDRV_ID_DUTYPE            (3019U)   /**< Drive Unit Type */
#define PDRV_ID_DOTYPE            (3019U)   /**< Drive Object Type */
#define PDRV_ID_FWVERSION          (200U)   /**< FW-Version 2.00 */
#define PDRV_ID_FWDATE_Y          (2018U)   /**< FW-Date year: 2018 */
#define PDRV_ID_FWDATA_DM         (1001U)   /**< FW-Date day/month: 10th of January */
/** @} */

/** @name PROFIdrive module IDs
 *  : (must fit to GSD file)
  * @attention TODO Change module ID to your own data and remove example for compatible module ID
 *  @{
 */
#define PDRV_MODULE_ID_MAP1 0x00004101UL    /**< compatible AC1 Module ID */
#define PDRV_MODULE_ID_MAP  0x00004401UL    /**< Module ID */
#define PDRV_SUBMOD_ID_PAP  0x0000FFFFUL    /**< Submodule ID for Parameter Access Point */
#define PDRV_SUBMOD_ID_TLG1 0x00000001UL    /**< Submodule ID for Standard Telegram 1 */
#define PDRV_SUBMOD_ID_TLG2 0x00000002UL    /**< Submodule ID for Standard Telegram 2 */
#define PDRV_SUBMOD_ID_TLG3 0x00000003UL    /**< Submodule ID for Standard Telegram 3 */
/** @} */

/** @name slot number, subslot number for PROFIdrive Parameter Access Point
 *  : (must fit to GSD file)
 *  @{
 */
#define PDRV_PAP_SLOT       (1) /**< Slot number for PROFIdrive Parameter Access Point */
#define PDRV_PAP_SUBSLOT    (1) /**< Subslot number for PROFIdrive Parameter Access Point */
/** @} */

/** @name slot number, subslot number and IO lengths for PROFIdrive telegrams
 *  : (must fit to GSD file)
 *  @{
 */
#define PDRV_STDTLG_SLOT    (1) /**< Slot number for PROFIdrive telegram */
#define PDRV_STDTLG_SUBSLOT (2) /**< Subslot number for PROFIdrive telegram */
#define PDRV_STDTLG1_INLEN  (4) /**< Input Data Length [bytes] of standard telegram 1 */
#define PDRV_STDTLG1_OUTLEN (4) /**< Output Data Length [bytes] of standard telegram 1 */
#define PDRV_STDTLG2_INLEN  (8) /**< Input Data Length [bytes] of standard telegram 2 */
#define PDRV_STDTLG2_OUTLEN (8) /**< Output Data Length [bytes] of standard telegram 2 */
#define PDRV_STDTLG3_INLEN  (18)/**< Input Data Length [bytes] of standard telegram 3 */
#define PDRV_STDTLG3_OUTLEN (10)/**< Output Data Length [bytes] of standard telegram 3 */
/** @} */

/** @name Isochronous Mode Data
 *  : (T_DC_Min, T_DC_Max, T_IO_InputMin and T_IO_OutputMin must fit to GSD file)
 *  @{
 */
#define PDRV_ISO_CACF_MIN   (1)         /**< minimal Controller Application Cycle Factor */
#define PDRV_ISO_CACF_MAX   (14)        /**< maximal Controller Application Cycle Factor, see PDRV V4.2 chapter 6.3.12.1 */
#define PDRV_ISO_TDC_MIN    (500000UL) /**< T_DC_Min minimal time data cycle [ns] */
#define PDRV_ISO_TDC_MAX    (4000000UL) /**< T_DC_Max maximal time data cycle [ns] */
#define PDRV_ISO_IOI_MIN     (125000UL) /**< T_IO_InputMin minimal time for data input [ns] */
#define PDRV_ISO_IOO_MIN     (125000UL) /**< T_IO_OutputMin minimal time for data output [ns] */
/** @} */

/** @name PROFIdrive Standard Telegram Numbers
 *  : 
 *  @{
 */
#define PDRV_STDTLG1 0x0001U    /**< Standard Telegram 1 */
#define PDRV_STDTLG2 0x0002U    /**< Standard Telegram 2 */
#define PDRV_STDTLG3 0x0003U    /**< Standard Telegram 3 */
#define PDRV_STDTLG4 0x0004U    /**< Standard Telegram 4 */
#define PDRV_STDTLG5 0x0005U    /**< Standard Telegram 5 */
#define PDRV_STDTLG6 0x0006U    /**< Standard Telegram 6 */
#define PDRV_STDTLG7 0x0007U    /**< Standard Telegram 7 */
#define PDRV_STDTLG9 0x0009U    /**< Standard Telegram 9 */
/** @} */


extern void PdrvApp_main(void);
extern PDRV_BOOL bPdrvApp_IsAxisStandstill(PDRV_VOID);
extern PDRV_BOOL bPdrvApp_IsTransitionCondition(PDRV_AXISSTATES_TYPE p_eActAxisMainState, PDRV_AXISSTATES_TYPE p_eReqAxisMainState);
extern PDRV_INT32 nPdrvApp_CalcPT1(PDRV_INT32 p_nX);
extern PDRV_UINT32 uPdrvApp_GetTimer1s(PDRV_VOID);
extern PDRV_UINT32 uPdrvApp_GetTimer1ms(PDRV_VOID);

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif
