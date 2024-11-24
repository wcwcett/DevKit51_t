/*********************************************************************************************************************/
/**@file        pdrv_pardatabase_ac4.h
 * @brief       PROFIdrive Parameter Database header file
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
 *              Shorthand symbol: PAR (PARameter)<br>
 *              Parameter Database implements the parameter objects.
 *
 * @internal
 * @note        Doxygen comments are used for automatic documentation generation.
 *              For further information about Doxygen please look at http://www.stack.nl/~dimitri/doxygen/index.html
 *              IDE Eclipse's folding feature is used too. see https://www.eclipse.org
 * @endinternal
*/
/*********************************************************************************************************************/

#ifndef PDRV_PARDATABASE_AC4_H
#define PDRV_PARDATABASE_AC4_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

/** @name bit strings of floating point numbers (PDRV without any floating point c-libraries)
 *  : https://www.h-schmidt.net/FloatConverter/IEEE754.html
   @{ */
#if 0
#define FLOAT_0         (0x00000000UL)          /**< IEEE754 32bit representation of 0.f */
#define FLOAT_1         (0x3f800000UL)          /**< IEEE754 32bit representation of 1.f */
#define FLOAT_3000      (0x453b8000UL)          /**< IEEE754 32bit representation of 3000.f */
#define FLOAT_6000      (0x45bb8000UL)          /**< IEEE754 32bit representation of 6000.f */
#define FLOAT_N2_FAC    (0x3bc80000UL)          /**< IEEE754 32bit representation of standardisation factor for data type N2 */
#define FLOAT_N4_FAC    (0x33c80000UL)          /**< IEEE754 32bit representation of standardisation factor for data type N4 */
#define FLOAT_X2_FAC    (0x3cc80000UL)          /**< IEEE754 32bit representation of standardisation factor for data type X2 with Bit12 */
#define FLOAT_X4_FAC    (0x34c80000UL)          /**< IEEE754 32bit representation of standardisation factor for data type X4 with Bit28 */
#else
#define FLOAT_0         (0.f)                   /**< Zero in floating point */
#define FLOAT_1         (1.f)                   /**< One in floating point */
#define FLOAT_3000      (3000.f)                /**< 3000 in floating point */
#define FLOAT_6000      (6000.f)                /**< 6000 in floating point */
#define FLOAT_N2_FAC    (100.f/0x4000U)         /**< standardisation factor for data type N2 */
#define FLOAT_N4_FAC    (100.f/0x40000000UL)    /**< standardisation factor for data type N4 */
#define FLOAT_X2_FAC    (100.f/0x1000U)         /**< standardisation factor for data type X2 with Bit12 */
#define FLOAT_X4_FAC    (100.f/0x10000000UL)    /**< standardisation factor for data type X4 with Bit28 */
#endif
/* @} */

/** pre declaration of parameter object */
typedef struct PDRV_PAR_OBJ PDRV_PAR_OBJ;

/** parameter object inclusive description elements (see PDRV V4.2 table 17)
 * @details no reserved datas, different order from PDRV description, order is arbitrary (consider alignment)
 */
struct PDRV_PAR_OBJ
{
    PDRV_UINT16 uPnu;           /**< parameter number */
    PDRV_UINT16 uIdentifier;    /**< identifier */
    PDRV_UINT16 uNrOfElements;  /**< number of elements or length of string */
    PDRV_UINT16 uVarAttrib;     /**< variable attribute */
    PDRV_UINT16 uRefPar;        /**< DO IO DATA reference parameter */
    PDRV_UINT16 uNormalisation; /**< DO IO DATA normalisation */
    PDRV_FLT32  fStdFactor;     /**< standardisation factor */
    PDRV_UINT32 uLoLimit;       /**< low limit */
    PDRV_UINT32 uHiLimit;       /**< high limit */
    const char* puName;         /**< pointer at name */
    char* (*pfnText)(const PDRV_PAR_OBJ *p_ptParObj, PDRV_UINT16 p_uSubindex); /**< function pointer - function is called if additional text is read */
    PDRV_UINT32 (*pfnRead)(const PDRV_PAR_OBJ *p_ptParObj, PDRV_UINT16 p_uSubindex, PDRV_UINT16 p_uNrOfElements, PDRV_ParValues * p_ptValues); /**< function pointer - function is called before value is read */
    PDRV_UINT32 (*pfnWrite)(const PDRV_PAR_OBJ *p_ptParObj, PDRV_UINT16 p_uSubindex, PDRV_UINT16 p_uNrOfElements, PDRV_ParValues * p_ptValues); /**< function pointer - function is called after value is written */
};

/** @name PROFIdrive SI Units
  * : variable index and conversion index for SI units see PDRV V4.2 table 20 <br>
  * Used units are implemented only yet.
  * @{
*/
#define PDRV_UNIT_SEC   (( 4U << 8U) + 0U)      /**< time [s] */
#define PDRV_UNIT_MSEC  (( 4U << 8U) + 0xFDU)   /**< time [ms] */
#define PDRV_UNIT_RMIN  ((11U << 8U) + 67U)     /**< speed [r/min] */
#define PDRV_UNIT_PCT   ((24U << 8U) + 0U)      /**< percent [%] */
/* @} */


extern const PDRV_PAR_OBJ *ptPdrvPar_GetParObj(PDRV_UINT16 p_uPnu);

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif
