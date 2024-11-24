/*********************************************************************************************************************/
/**@file        pdrv_types_ac4.h
 * @brief       PROFIdrive datatype definitions
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
 *              Datatypes definitions for PROFIdrive.
 *
 * @internal
 * @note        Doxygen comments are used for automatic documentation generation.
 *              For further information about Doxygen please look at http://www.stack.nl/~dimitri/doxygen/index.html
 *              IDE Eclipse's folding feature is used too. see https://www.eclipse.org
 * @endinternal
*/
/*********************************************************************************************************************/

#ifndef PDRV_TYPES_AC4_H
#define PDRV_TYPES_AC4_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

#include "compiler.h"   /* compiler specific declarations (packed data formats) */
//#include "lsa_types.h"  /* datatype definitions in the PDRV DevKit project */

/** @name compiler specific definitions
 *  : using definition in compiler.h
 *  @{
 */
#define ATTR_PDRV_PACKED_PRE    ATTR_PNIO_PACKED_PRE
#define ATTR_PDRV_PACKED        ATTR_PNIO_PACKED
#define PDRV_BIG_ENDIAN         PNIO_BIG_ENDIAN
/** @} */

/** @name basic types
  * : according ANSI C1999 Standard (ISO/IEC 9899:1999)
  * @{
*/
typedef LSA_UINT    PDRV_UINT;      /**< UNSIGNED-x-bit (compiler specific) minimal range: 0 ... +65535 */
typedef LSA_UINT8   PDRV_UINT8;     /**< UNSIGNED-8-Bit:  0 ...                  +255 */
typedef LSA_UINT16  PDRV_UINT16;    /**< UNSIGNED-16-Bit: 0 ...                +65535 */
typedef LSA_UINT32  PDRV_UINT32;    /**< UNSIGNED-32-Bit: 0 ...           +4294967295 */
typedef LSA_UINT64  PDRV_UINT64;    /**< UNSIGNED-64-Bit: 0 ... +18446744073709551615 */
typedef LSA_INT     PDRV_INT;       /**< SIGNED-x-bit (compiler specific) minimal range: -32768 ... +32767 */
typedef LSA_INT8    PDRV_INT8;      /**< SIGNED-8-Bit:                  -128 ...                 +127 */
typedef LSA_INT16   PDRV_INT16;     /**< SIGNED-16-Bit:               -32768 ...               +32767 */
typedef LSA_INT32   PDRV_INT32;     /**< SIGNED-32-Bit:          -2147483648 ...          +2147483647 */
typedef LSA_INT64   PDRV_INT64;     /**< SIGNED-64-Bit: -9223372036854775808 ... +9223372036854775807 */
typedef float       PDRV_FLT32;     /**< FLOATING-POINT-32-Bit single precision according IEEE-754 */
typedef double      PDRV_FLT64;     /**< FLOATING-POINT-64-Bit double precision according IEEE-754 */
/** @} */

/** @name additional types
  * : used for C programming
  * @{
*/
typedef LSA_BOOL    PDRV_BOOL;      /**< this boolean type is compiler and cpu specific*/
typedef LSA_VOID    PDRV_VOID;      /**< void type for PROFIdrive */
/** @} */


/** @name PROFIdrive types
  * : according PROFIdrive and IEC 61158-5-10 (see PDRV V4.2 table 1)
  * @{
*/
typedef PDRV_INT8   PDRV_BL;    /**< Boolean */
typedef PDRV_INT8   PDRV_I1;    /**< Signed8 */
typedef PDRV_INT16  PDRV_I2;    /**< Signed16 */
typedef PDRV_INT32  PDRV_I4;    /**< Signed32 */
typedef PDRV_INT64  PDRV_I8;    /**< Signed64 */
typedef PDRV_UINT8  PDRV_O1;    /**< Unsigned8 */
typedef PDRV_UINT16 PDRV_O2;    /**< Unsigned16 */
typedef PDRV_UINT32 PDRV_O4;    /**< Unsigned32 */
typedef PDRV_UINT64 PDRV_O8;    /**< Unsigned64 */
typedef PDRV_FLT32  PDRV_F4;    /**< Floating Point 32 */
typedef PDRV_FLT64  PDRV_F8;    /**< Floating Point 64 (double) */
typedef PDRV_UINT8  PDRV_VS;    /**< VisibleString */
typedef PDRV_UINT8  PDRV_OS;    /**< OctetString */
typedef PDRV_UINT16 PDRV_US;    /**< UnicodeString */
typedef ATTR_PDRV_PACKED_PRE struct
{
    PDRV_UINT32 ms;             /**< milliseconds */
    PDRV_UINT16 d;              /**< days after 1984-01-01 */
} ATTR_PDRV_PACKED PDRV_TODW;    /**< TimeOfDay with date indication */
typedef ATTR_PDRV_PACKED_PRE struct
{
    PDRV_UINT16 ms;             /**< milliseconds fraction of minute */
    PDRV_UINT8 min;             /**< minutes fraction of hour */
    PDRV_UINT8 h;               /**< hours fraction of day */
    PDRV_UINT8 d;               /**< day of week (3 MSB) and day of month (5 LSB) */
    PDRV_UINT8 m;               /**< month */
    PDRV_UINT8 y;               /**< year with 0..50 = 2000..2050, 51..99 = 1951..1999 */
    PDRV_UINT8 alignment;       /**< necessary for 16bit alignment */
} ATTR_PDRV_PACKED PDRV_DATE;   /**< BinaryDate */
typedef PDRV_UINT32 PDRV_TOD;   /**< TimeOfDay without date indication  */
typedef ATTR_PDRV_PACKED_PRE struct
{
    PDRV_UINT32 ms;             /**< milliseconds */
    PDRV_UINT16 d;              /**< days */
} ATTR_PDRV_PACKED PDRV_TDW;    /**< TimeDifference with date indication */
typedef PDRV_UINT32 PDRV_TD;    /**< TimeDifference without date indication */
typedef PDRV_INT16  PDRV_N2;    /**< N2 Normalised value (16 bit with 0x4000 == 100%) */
typedef PDRV_INT32  PDRV_N4;    /**< N4 Normalised value (32 bit with 0x40000000 == 100%) */
typedef PDRV_UINT16 PDRV_V2;    /**< V2 Bit sequence (16 bit) */
typedef PDRV_UINT16 PDRV_L2;    /**< L2 Nibble (16 bit) */
typedef PDRV_UINT16 PDRV_R2;    /**< R2 Reciprocal time constant (16 bit: 1 ... 16384) */
typedef PDRV_UINT16 PDRV_T2;    /**< T2 Time constant (16 bit: 0 ... 32767, values >32767 are identical with zero) */
typedef PDRV_UINT32 PDRV_T4;    /**< T4 Time constant (32 bit) */
typedef PDRV_UINT16 PDRV_D2;    /**< D2 Time constant (16 bit: 0 ... 32767) */
typedef PDRV_INT16  PDRV_E2;    /**< E2 Fixed point value (16 bit with 1 == 2^-7) */
typedef PDRV_INT32  PDRV_C4;    /**< C4 Fixed point value (32 bit with 1 == 0.0001) */
typedef PDRV_INT16  PDRV_X2;    /**< X2 Normalised value (16 bit with variable normalisation) */
typedef PDRV_INT32  PDRV_X4;    /**< X4 Normalised value (32 bit with variable normalisation) */
/* @} */


/** @name PROFIdrive boolean values
  * : Be aware that all other values are undefined.<br>
  * It is NOT according to the C/C++ definition with a value != 0 is logical true.
  * @{
*/
#define PDRV_FALSE (0U)     /**< value for logical false */
#define PDRV_TRUE  (1U)     /**< value for logical true */
/* @} */

#define PDRV_NULL   ((PDRV_VOID *)NULL)  /**< null pointer definition */

#define PDRV_API    (0x3A00UL)  /**< PROFIdrive API number see PDRV V4.2 chapter 8.3.3 */
#define PDRV_RECLOC (0xB02EU)   /**< Map index of base mode parameter access local see PDRV V4.2 table 186 */
#define PDRV_RECGLO (0xB02FU)   /**< Map index of base mode parameter access global see PDRV V4.2 table 186 */


/** absolute value of a signed integer with handling of special case abs(0x8000) == 0x7FFF */
#define nPdrvAbs(a)     ((a)>=0? (a) : ((a)!=-(a)? -(a): ~(a)))

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif
