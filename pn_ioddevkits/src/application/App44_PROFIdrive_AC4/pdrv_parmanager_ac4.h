/*********************************************************************************************************************/
/**@file        pdrv_parmanager_ac4.h
 * @brief       PROFIdrive Parameter Manager header file
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
 *              The Parameter Manager implements the PROFIdrive parameter access.
 *
 * @internal
 * @note        Doxygen comments are used for automatic documentation generation.
 *              For further information about Doxygen please look at http://www.stack.nl/~dimitri/doxygen/index.html
 *              IDE Eclipse's folding feature is used too. see https://www.eclipse.org
 * @endinternal
*/
/*********************************************************************************************************************/

#ifndef PDRV_PARMANAGER_AC4_H
#define PDRV_PARMANAGER_AC4_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

#define PDRV_PAR_BLOCKSIZE  (240U)  /**< parameter response/request block size see PDRV V4.2 chapter 6.2.3.2 */

/** values for a parameter request */
typedef ATTR_PDRV_PACKED_PRE union par_values
{
    PDRV_BL bl[PDRV_PAR_BLOCKSIZE];                         /**< Boolean */
    PDRV_I1 i1[PDRV_PAR_BLOCKSIZE];                         /**< Signed8 */
    PDRV_I2 i2[PDRV_PAR_BLOCKSIZE/sizeof(PDRV_I2)];         /**< Signed16 */
    PDRV_I4 i4[PDRV_PAR_BLOCKSIZE/sizeof(PDRV_I4)];         /**< Signed32 */
    PDRV_I8 i8[PDRV_PAR_BLOCKSIZE/sizeof(PDRV_I8)];         /**< Signed64 */
    PDRV_O1 o1[PDRV_PAR_BLOCKSIZE];                         /**< Unsigned8 */
    PDRV_O2 o2[PDRV_PAR_BLOCKSIZE/sizeof(PDRV_O2)];         /**< Unsigned16 */
    PDRV_O4 o4[PDRV_PAR_BLOCKSIZE/sizeof(PDRV_O4)];         /**< Unsigned32 */
    PDRV_O8 o8[PDRV_PAR_BLOCKSIZE/sizeof(PDRV_O8)];         /**< Unsigned64 */
    PDRV_F4 f4[PDRV_PAR_BLOCKSIZE/sizeof(PDRV_F4)];         /**< Floating Point 32 */
    PDRV_F8 f8[PDRV_PAR_BLOCKSIZE/sizeof(PDRV_F8)];         /**< Floating Point 64 (double) */
    char    vs[PDRV_PAR_BLOCKSIZE];                         /**< VisibleString */
    PDRV_OS os[PDRV_PAR_BLOCKSIZE];                         /**< OctetString */
    PDRV_TODW todw[PDRV_PAR_BLOCKSIZE/sizeof(PDRV_TODW)];   /**< TimeOfDay with date indication */
    PDRV_DATE date[PDRV_PAR_BLOCKSIZE/sizeof(PDRV_DATE)];   /**< BinaryDate */
    PDRV_TOD  tod[PDRV_PAR_BLOCKSIZE/sizeof(PDRV_TOD)];     /**< TimeOfDay without date indication */
    PDRV_TDW  tdw[PDRV_PAR_BLOCKSIZE/sizeof(PDRV_TDW)];     /**< TimeDifference with date indication */
    PDRV_TD   td[PDRV_PAR_BLOCKSIZE/sizeof(PDRV_TD)];       /**< TimeDifference without date indication */
} ATTR_PDRV_PACKED PDRV_ParValues;                          /**< values for a parameter request */

/** @name coding of PROFIdrive types for identifier
  * : see PDRV V4.2 table 1 and table 2
  * @{
*/
#define PDRV_PARID_BL   (1U)    /**< Boolean */
#define PDRV_PARID_I1   (2U)    /**< Signed8 */
#define PDRV_PARID_I2   (3U)    /**< Signed16 */
#define PDRV_PARID_I4   (4U)    /**< Signed32 */
#define PDRV_PARID_O1   (5U)    /**< Unsigned8 */
#define PDRV_PARID_O2   (6U)    /**< Unsigned16 */
#define PDRV_PARID_O4   (7U)    /**< Unsigned32 */
#define PDRV_PARID_F4   (8U)    /**< FloatingPoint32 single precision */
#define PDRV_PARID_VS   (9U)    /**< VisibleString */
#define PDRV_PARID_OS  (10U)    /**< OctetString */
#define PDRV_PARID_TODW (12U)   /**< TimeOfDay with date indication */
#define PDRV_PARID_F8  (15U)    /**< FloatingPoint64 double precision */
#define PDRV_PARID_US  (39U)    /**< UnicodeString */
#define PDRV_PARID_DATE (50U)   /**< BinaryDate */
#define PDRV_PARID_TOD (52U)    /**< TimeOfDay without date indication  */
#define PDRV_PARID_TDW (53U)    /**< TimeDifference with date indication */
#define PDRV_PARID_TD  (54U)    /**< TimeDifference without date indication */
#define PDRV_PARID_I8  (55U)    /**< Signed64 */
#define PDRV_PARID_O8  (56U)    /**< Unsigned64 */
#define PDRV_PARID_N2 (113U)    /**< N2 Normalised value (16 bit with 0x4000 == 100%) */
#define PDRV_PARID_N4 (114U)    /**< N4 Normalised value (32 bit with 0x40000000 == 100%) */
#define PDRV_PARID_V2 (115U)    /**< V2 Bit sequence (16 bit) */
#define PDRV_PARID_L2 (116U)    /**< L2 Nibble (16 bit) */
#define PDRV_PARID_R2 (117U)    /**< R2 Reciprocal time constant (16 bit: 1 ... 16384) */
#define PDRV_PARID_T2 (118U)    /**< T2 Time constant (16 bit: 0 ... 32767, values >32767 are identical with zero) */
#define PDRV_PARID_T4 (119U)    /**< T4 Time constant (32 bit) */
#define PDRV_PARID_D2 (120U)    /**< D2 Time constant (16 bit: 0 ... 32767) */
#define PDRV_PARID_E2 (121U)    /**< E2 Fixed point value (16 bit with 1 == 2^-7) */
#define PDRV_PARID_C4 (122U)    /**< C4 Fixed point value (32 bit with 1 == 0.0001) */
#define PDRV_PARID_X2 (123U)    /**< X2 Normalised value (16 bit with variable normalisation) */
#define PDRV_PARID_X4 (124U)    /**< X4 Normalised value (32 bit with variable normalisation) */
/** @} */

/** @name Coding of identifier
  * : see PDRV V4.2 table 18
  * @{
*/
#define PDRV_PARID_NOSTD        (1U <<  8U) /**< standardisation factor and variable attribute not relevant */
#define PDRV_PARID_READONLY     (1U <<  9U) /**< parameter is read only i.e. parameter not writable */
#define PDRV_PARID_ADDTEXT      (1U << 10U) /**< additional text array is available */
#define PDRV_PARID_CHANGED      (1U << 12U) /**< parameter was changed with respect to the factory setting */
#define PDRV_PARID_RESETONLY    (1U << 13U) /**< parameter value may be reset only */
#define PDRV_PARID_ARRAY        (1U << 14U) /**< array */
/* @} */

/** @name Error_Code_1 definitions
  * : see PDRV V4.2 table 169
  * @{
*/
#define PDRV_EC1_OK                 (0x00U)   /**< no error */
#define PDRV_EC1_APP_READ           (0xA0U)   /**< application read error */
#define PDRV_EC1_APP_WRITE          (0xA1U)   /**< application write error */
#define PDRV_EC1_APP_MODULE         (0xA2U)   /**< application module failure */
#define PDRV_EC1_APP_BUSY           (0xA7U)   /**< application busy */
#define PDRV_EC1_APP_VER_CONFLICT   (0xA8U)   /**< application version conflict */
#define PDRV_EC1_APP_NOT_SUPPORTED  (0xA9U)   /**< application feature not supported */
#define PDRV_EC1_ACC_INDEX_INVALID  (0xB0U)   /**< access invalid index; PDRV no PAP i.e. parameter request unsupported */
#define PDRV_EC1_ACC_WRITE_LEN      (0xB1U)   /**< access write length error */
#define PDRV_EC1_ACC_SLOT_INVALID   (0xB2U)   /**< access invalid slot */
#define PDRV_EC1_ACC_TYPE_CONFLICT  (0xB3U)   /**< access type conflict */
#define PDRV_EC1_ACC_STATE_CONFLICT (0xB5U)   /**< access state conflict; PDRV PAP access temporarily not possible due to internal reason */
#define PDRV_EC1_ACC_DENIED         (0xB6U)   /**< access access denied */
#define PDRV_EC1_ACC_INVALID_RANGE  (0xB7U)   /**< access invalid range; PDRV write with error in the Base Mode parameter request header */
#define PDRV_EC1_ACC_INVALID_PARAM  (0xB8U)   /**< access invalid parameter */
#define PDRV_EC1_ACC_INVALID_TYPE   (0xB9U)   /**< access invalid type */
/* @} */

/** @name PROFIdrive error numbers in Base mode parameter responses
  * : see error value 1 PDRV V4.2 table 32
  * @{
*/
#define PDRV_EV1_NOERROR            (0x0100U) /**< return value of parameter read/write function */
#define PDRV_EV1_PNU                (0x00U)   /**< impermissible parameter number */
#define PDRV_EV1_VALUE_CHANG        (0x01U)   /**< parameter value cannot be changed */
#define PDRV_EV1_VALUE_LIMIT        (0x02U)   /**< low or high limit exceeded */
#define PDRV_EV1_SUBINDEX           (0x03U)   /**< faulty subindex */
#define PDRV_EV1_NO_ARRAY           (0x04U)   /**< no array */
#define PDRV_EV1_DATATYPE           (0x05U)   /**< incorrect data type */
#define PDRV_EV1_RESET_ONLY         (0x06U)   /**< setting not permitted (may only be reset) */
#define PDRV_EV1_DESC_CHANGE        (0x07U)   /**< description element cannot be changed */
#define PDRV_EV1_NO_DESC            (0x09U)   /**< no description available */
#define PDRV_EV1_NO_OP_PRIO         (0x0BU)   /**< no operation priority */
#define PDRV_EV1_NO_TEXT_ARRAY      (0x0FU)   /**< no text array available */
#define PDRV_EV1_OP_STATE           (0x11U)   /**< request cannot be executed because of operating state */
#define PDRV_EV1_VALUE_IMPERMISS    (0x14U)   /**< value impermissible */
#define PDRV_EV1_RESP_TOO_LONG      (0x15U)   /**< response too long */
#define PDRV_EV1_PAR_ADDR_IMPERMISS (0x16U)   /**< parameter address impermissible */
#define PDRV_EV1_FORMAT_ILLEGAL     (0x17U)   /**< illegal format (write request) */
#define PDRV_EV1_NR_VAL_INCONSISTNT (0x18U)   /**< number of values are not consistent (write request) */
#define PDRV_EV1_AXIS               (0x19U)   /**< axis/DO nonexistent */
#define PDRV_EV1_PAR_TXT_CHANGE     (0x20U)   /**< parameter text element cannot be changed */
#define PDRV_EV1_NOT_SUPPORTED      (0x21U)   /**< service not supported */
#define PDRV_EV1_TOO_MUCH_REQ       (0x22U)   /**< too much parameter request (multi parameter request) */
#define PDRV_EV1_NO_MULTI_PAR       (0x23U)   /**< multi parameter access is not supported */
/* @} */

extern PDRV_VOID PdrvPar_Init(PDRV_VOID);
extern PDRV_VOID PdrvPar_ProcessReq (PDRV_VOID);
extern PDRV_BOOL bPdrvPar_EstablishCxn(PDRV_UINT16 p_uArType, PDRV_UINT16  p_uArNum);
extern PDRV_BOOL bPdrvPar_DisconnCxn(PDRV_UINT16 p_uArNum);
extern PDRV_UINT8 uPdrvPar_WriteReqCxn(PDRV_UINT16 p_uArNum, PDRV_UINT32 *p_puBufLen, PDRV_UINT8  *p_puBuffer);
extern PDRV_UINT8 uPdrvPar_ReadReqCxn(PDRV_UINT16 p_uArNum, PDRV_UINT32 *p_puBufLen, PDRV_UINT8  *p_puBuffer);

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif
