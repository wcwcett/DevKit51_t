/*********************************************************************************************************************/
/**@file        pdrv_parameter_ac4.inc
 * @brief       Macro-Header include file for PROFIdrive pardatabase.
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
 *              This file inlcudes all parameter for macro definitions which are used in pardatabase.
 *  
 * @attention   This file can be included more than once in the same file! Don't use a include guard!
 *
 * @internal
 * @note        Doxygen comments are used for automatic documentation generation.
 *              For further information about Doxygen please look at http://www.stack.nl/~dimitri/doxygen/index.html
 *              IDE Eclipse's folding feature is used too. see https://www.eclipse.org
 * @endinternal
*/
/*********************************************************************************************************************/

#if 0
/** PNU000 is only a default pattern! PNU000 is forbidden. see PDRV 4.2 chapter 6.2.1.1 */
PDRV_PARAMETER(
    0U,                 /**< parameter number */
    0U,                 /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0UL,                /**< low limit */
    0UL,                /**< high limit */
    "1234567890123456", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    PDRV_NULL_R,        /**< function pointer - function is called before value is read */
    PDRV_NULL_W,        /**< function pointer - function is called after value is written */
)
#endif

/* for parameter descriptions see PDRV V4.2 table 144 and others */

/* PNU00001..PNU00899: device specific, insert your parameters here -------------------------------------------------*/

/** PNU00100: gradient of ramp in [%/ms] for the Ramp Function Generator
 * @details Setting parameter for the Ramp Function Generator (PDRV V4.2 Figure 29)
 *          application specific, can be changed by the PDRV user
 */
PDRV_PARAMETER(
    100U,               /**< parameter number */
    (PDRV_PARID_N4),    /**< identifier */
    0U,                 /**< number of elements or length of string */
    PDRV_UNIT_PCT,      /**< variable attribute */
    60000U,             /**< DO IO DATA reference parameter */
    0x801EU,            /**< DO IO DATA normalisation */
    FLOAT_N4_FAC,       /**< standardisation factor */
    0U,                 /**< low limit */
    0x10000000U,        /**< high limit */
    "Ramp Gradient",    /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00100,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00100    /**< function pointer - function is called after value is written */
)

/** PNU00110: +- allowed speed tolerance for ZSW1 bit 8 "speed errror within tolerance range"
 * @details Setting parameter for calculation of ZSW1 bit 8 (PDRV V4.2 Figure 29)
 *          application specific, can be changed by the PDRV user
 */
PDRV_PARAMETER(
    110U,               /**< parameter number */
    (PDRV_PARID_N4),    /**< identifier */
    0U,                 /**< number of elements or length of string */
    PDRV_UNIT_PCT,      /**< variable attribute */
    60000U,             /**< DO IO DATA reference parameter */
    0x801EU,            /**< DO IO DATA normalisation */
    FLOAT_N4_FAC,       /**< standardisation factor */
    0U,                 /**< low limit */
    0x7FFFFFFFU,        /**< high limit */
    "range speederror", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00110,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00110    /**< function pointer - function is called after value is written */
)

/** PNU00111: time of speed tolerance for ZSW1 bit 8 "speed errror within tolerance range"
 * @details Setting parameter for calculation of ZSW1 bit 8 (PDRV V4.2 Figure 29)
 *          application specific, can be changed by the PDRV user
 */
PDRV_PARAMETER(
    111U,               /**< parameter number */
    (PDRV_PARID_O2),    /**< identifier */
    0U,                 /**< number of elements or length of string */
    PDRV_UNIT_MSEC,     /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_1,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFU,            /**< high limit */
    "tMax speed error", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00111,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00111    /**< function pointer - function is called after value is written */
)

/** PNU00115: comparison value for ZSW1 bit 10 "f or n reached"
 * @details Setting parameter for calculation of ZSW1 bit 10 (PDRV V4.2 Figure 29)
 *          application specific, can be changed by the PDRV user
 */
PDRV_PARAMETER(
    115U,               /**< parameter number */
    (PDRV_PARID_N4),    /**< identifier */
    0U,                 /**< number of elements or length of string */
    PDRV_UNIT_PCT,      /**< variable attribute */
    60000U,             /**< DO IO DATA reference parameter */
    0x801EU,            /**< DO IO DATA normalisation */
    FLOAT_N4_FAC,       /**< standardisation factor */
    0U,                 /**< low limit */
    0x07FFFFFFFU,       /**< high limit */
    "speed comp value", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00115,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00115    /**< function pointer - function is called after value is written */
)

/** PNU00120: array of ramps during [0]=ramp stop, [1]=quick stop
 * @details ramps during ramp stop and quick stop (PDRV V4.2 Figure 27)
 *          application specific, can be changed by the PDRV user
 */
PDRV_PARAMETER(
    120U,               /**< parameter number */
    (PDRV_PARID_N4 | PDRV_PARID_ARRAY | PDRV_PARID_ADDTEXT),    /**< identifier */
    2U,                 /**< number of elements or length of string */
    PDRV_UNIT_PCT,      /**< variable attribute */
    60000U,             /**< DO IO DATA reference parameter */
    0x801EU,            /**< DO IO DATA normalisation */
    FLOAT_N4_FAC,       /**< standardisation factor */
    0U,                 /**< low limit */
    0x07FFFFFFFU,       /**< high limit */
    "stop ramps",       /**< pointer at name (16 valid characters) */
    pcPdrv_TfPnu00120,  /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00120,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00120    /**< function pointer - function is called after value is written */
)

/** PNU00200: reciprocal gain factor for loop control simulation with PT1 element during [0]=normal operation, [1]=coast stop
 * @details  application specific, can be changed by the PDRV user
 */
PDRV_PARAMETER(
    200U,               /**< parameter number */
    (PDRV_PARID_I2 | PDRV_PARID_NOSTD | PDRV_PARID_ARRAY | PDRV_PARID_ADDTEXT), /**< identifier */
    2U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x7FFFU,            /**< high limit */
    "PT1 gain factors", /**< pointer at name (16 valid characters) */
    pcPdrv_TfPnu00200,  /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00200,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00200    /**< function pointer - function is called after value is written */
)

/** PNU00300: choose source for "Telegram selection"
 * @details 0 = Source for telegram selection is Expected Configuration Data Record (S7 Hardware Config)
 *          1 = Source for telegram selection is PROFIdrive parameter PNU00922
 */
PDRV_PARAMETER(
    300U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    1U,			        /**< high limit */
    "Source tgm sel.",  /**< pointer at name (16 valid characters) */
    pcPdrv_TfPnu00300,  /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00300,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00300    /**< function pointer - function is called after value is written */
)

/** PNU00700: set fault with fault number and fault value
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    700U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD | PDRV_PARID_ARRAY | PDRV_PARID_ADDTEXT), /**< identifier */
    2U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFU,            /**< high limit */
    "Set Fault \43",    /**< pointer at name (16 valid characters) */
    pcPdrv_TfPnu00700,  /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00700,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00700    /**< function pointer - function is called after value is written */
)

/** PNU00701: reset fault with fault number
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    701U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    WARNING_USER9,      /**< high limit */
    "Reset Fault \43",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00701,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00701    /**< function pointer - function is called after value is written */
)

/** PNU00702: set and reset warning with warning number and warning value
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    702U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD | PDRV_PARID_ADDTEXT | PDRV_PARID_ARRAY), /**< identifier */
    2U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    WARNING_USER9,      /**< high limit */
    "Set Warning \43",  /**< pointer at name (16 valid characters) */
    pcPdrv_TfPnu00702,  /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00702,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00702    /**< function pointer - function is called after value is written */
)

/** PNU00720: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    720U,               /**< parameter number */
    (PDRV_PARID_BL | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ADDTEXT), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    1U,                 /**< high limit */
    "RO-Single Par BL", /**< pointer at name (16 valid characters) */
    pcPdrv_TfPnu00800,  /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00721: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    721U,               /**< parameter number */
    (PDRV_PARID_I1 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0x81U,              /**< low limit */
    0x7FU,              /**< high limit */
    "RO-Single Par I1", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00722: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    722U,               /**< parameter number */
    (PDRV_PARID_I2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0x8001U,            /**< low limit */
    0x7FFFU,            /**< high limit */
    "RO-Single Par I2", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00723: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    723U,               /**< parameter number */
    (PDRV_PARID_I4 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0x80000001UL,       /**< low limit */
    0x7FFFFFFFUL,       /**< high limit */
    "RO-Single Par I4", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00724: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    724U,               /**< parameter number */
    (PDRV_PARID_O1 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFU,              /**< high limit */
    "RO-Single Par O1", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00725: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    725U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFU,            /**< high limit */
    "RO-Single Par O2", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00726: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    726U,               /**< parameter number */
    (PDRV_PARID_O4 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "RO-Single Par O4", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00727: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    727U,               /**< parameter number */
    (PDRV_PARID_F4 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0xc5bb8000UL,       /**< low limit -FLOAT_6000 */
    0x45bb8000UL,       /**< high limit FLOAT_6000 */
    "RO-Single Par F4", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00728: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    728U,               /**< parameter number */
    (PDRV_PARID_VS | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    16U,                /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0x01U,              /**< low limit */
    0xFFU,              /**< high limit */
    "RO-Single Par VS", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00729: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    729U,               /**< parameter number */
    (PDRV_PARID_OS | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    16U,                /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFU,              /**< high limit */
    "RO-Single Par OS", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00730: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    730U,               /**< parameter number */
    (PDRV_PARID_TODW | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFU,        /**< high limit */
    "RO-Single Par TO", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00731: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    731U,               /**< parameter number */
    (PDRV_PARID_F8 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0xc5bb8000UL,       /**< low limit -FLOAT_6000 */
    0x45bb8000UL,       /**< high limit FLOAT_6000 */
    "RO-Single Par F8", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00732: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    732U,               /**< parameter number */
    (PDRV_PARID_US | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    1U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x0FFFFU,           /**< high limit */
    "RO-Single Par US", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00733: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    733U,               /**< parameter number */
    (PDRV_PARID_DATE | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "RO-Single Par DA", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00734: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    734U,               /**< parameter number */
    (PDRV_PARID_TOD | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFU,        /**< high limit */
    "RO-Single Par TO", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00735: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    735U,               /**< parameter number */
    (PDRV_PARID_TDW | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "RO-Single Par TD", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00736: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    736U,               /**< parameter number */
    (PDRV_PARID_TD | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "RO-Single Par TD", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00737: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    737U,               /**< parameter number */
    (PDRV_PARID_I8 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0x80000000UL,       /**< low limit */
    0x7FFFFFFFUL,       /**< high limit */
    "RO-Single Par I8", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00738: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    738U,               /**< parameter number */
    (PDRV_PARID_O8 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "RO-Single Par O8", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00739: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    739U,               /**< parameter number */
    (PDRV_PARID_N2 | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    PDRV_UNIT_PCT,      /**< variable attribute */
    60000U,             /**< DO IO DATA reference parameter */
    0x800EU,            /**< DO IO DATA normalisation */
    FLOAT_N2_FAC,       /**< standardisation factor */
    0x08001U,           /**< low limit */
    0x07FFFU,           /**< high limit */
    "RO-Single Par N2", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00740: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    740U,               /**< parameter number */
    (PDRV_PARID_N4 | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    PDRV_UNIT_PCT,      /**< variable attribute */
    60000U,             /**< DO IO DATA reference parameter */
    0x801EU,            /**< DO IO DATA normalisation */
    FLOAT_N4_FAC,       /**< standardisation factor */
    0x80000001UL,       /**< low limit */
    0x7FFFFFFFUL,       /**< high limit */
    "RO-Single Par N4", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00741: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    741U,               /**< parameter number */
    (PDRV_PARID_V2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ADDTEXT), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFU,            /**< high limit */
    "RO-Single Par V2", /**< pointer at name (16 valid characters) */
    pcPdrv_TfPnu00741,  /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00742: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    742U,               /**< parameter number */
    (PDRV_PARID_L2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFU,            /**< high limit */
    "RO-Single Par L2", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00743: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    743U,               /**< parameter number */
    (PDRV_PARID_R2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    1U,                 /**< low limit */
    0x4000U,            /**< high limit */
    "RO-Single Par R2", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00744: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    744U,               /**< parameter number */
    (PDRV_PARID_T2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x7FFFU,            /**< high limit */
    "RO-Single Par T2", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00745: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    745U,               /**< parameter number */
    (PDRV_PARID_T4 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "RO-Single Par T4", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00746: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    746U,               /**< parameter number */
    (PDRV_PARID_D2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x07FFFU,           /**< high limit */
    "RO-Single Par D2", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00747: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    747U,               /**< parameter number */
    (PDRV_PARID_E2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFU,            /**< high limit */
    "RO-Single Par E2", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00748: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    748U,               /**< parameter number */
    (PDRV_PARID_C4 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "RO-Single Par C4", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00749: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    749U,               /**< parameter number */
    (PDRV_PARID_X2 | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    PDRV_UNIT_PCT,      /**< variable attribute */
    60000U,             /**< DO IO DATA reference parameter */
    0x800CU,            /**< DO IO DATA normalisation */
    FLOAT_X2_FAC,       /**< standardisation factor */
    0x8001U,            /**< low limit */
    0x7FFFU,            /**< high limit */
    "RO-Single Par X2", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00750: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    750U,               /**< parameter number */
    (PDRV_PARID_X4 | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    PDRV_UNIT_PCT,      /**< variable attribute */
    60000U,             /**< DO IO DATA reference parameter */
    0x801CU,            /**< DO IO DATA normalisation */
    FLOAT_X4_FAC,       /**< standardisation factor */
    0x80000001UL,       /**< low limit */
    0x7FFFFFFFUL,       /**< high limit */
    "RO-Single Par X4", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00760: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    760U,               /**< parameter number */
    (PDRV_PARID_BL | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    1U,                 /**< high limit */
    "RO-Array Par BL",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00761: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    761U,               /**< parameter number */
    (PDRV_PARID_I1 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    240U,               /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0x81U,              /**< low limit */
    0x7FU,              /**< high limit */
    "RO-Array Par I1",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00762: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    762U,               /**< parameter number */
    (PDRV_PARID_I2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    120U,               /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0x8001U,            /**< low limit */
    0x7FFFU,            /**< high limit */
    "RO-Array Par I2",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00763: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    763U,               /**< parameter number */
    (PDRV_PARID_I4 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    60,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0x80000001UL,       /**< low limit */
    0x7FFFFFFFUL,       /**< high limit */
    "RO-Array Par I4",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00764: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    764U,               /**< parameter number */
    (PDRV_PARID_O1 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    0xFFFEU,            /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFU,              /**< high limit */
    "RO-Array Par O1",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00765: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    765U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    0x1000U,            /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFU,            /**< high limit */
    "RO-Array Par O2",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00766: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    766U,               /**< parameter number */
    (PDRV_PARID_O4 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    0x2000U,            /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "RO-Array Par O4",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00767: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    767U,               /**< parameter number */
    (PDRV_PARID_F4 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    10U,                /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0xc5bb8000UL,       /**< low limit -FLOAT_6000 */
    0x45bb8000UL,       /**< high limit FLOAT_6000 */
    "RO-Array Par F4",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00768: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    768U,               /**< parameter number */
    (PDRV_PARID_X2 | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    10U,                /**< number of elements or length of string */
    PDRV_UNIT_PCT,      /**< variable attribute */
    60000U,             /**< DO IO DATA reference parameter */
    0x800CU,            /**< DO IO DATA normalisation */
    FLOAT_X2_FAC,       /**< standardisation factor */
    0x8001U,            /**< low limit */
    0x7FFFU,            /**< high limit */
    "RO-Array Par X2",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00769: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    769,                /**< parameter number */
    (PDRV_PARID_X4 | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    10U,                /**< number of elements or length of string */
    PDRV_UNIT_PCT,      /**< variable attribute */
    60000U,             /**< DO IO DATA reference parameter */
    0x801CU,            /**< DO IO DATA normalisation */
    FLOAT_X4_FAC,       /**< standardisation factor */
    0x80000001UL,       /**< low limit */
    0x7FFFFFFFUL,       /**< high limit */
    "RO-Array Par X4",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00770: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    770U,               /**< parameter number */
    (PDRV_PARID_TODW | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFU,        /**< high limit */
    "RO-Array Par TO",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00771: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    771U,               /**< parameter number */
    (PDRV_PARID_F8 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0xc5bb8000UL,       /**< low limit -FLOAT_6000 */
    0x45bb8000UL,       /**< high limit FLOAT_6000 */
    "RO-Array Par F8",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00772: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    772U,               /**< parameter number */
    (PDRV_PARID_C4 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    10U,                /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "RO-Array Par C4",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00773: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    773U,               /**< parameter number */
    (PDRV_PARID_DATE | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "RO-Array Par DAT", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00774: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    774U,               /**< parameter number */
    (PDRV_PARID_TOD | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "RO-Array Par TOD", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00775: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    775U,               /**< parameter number */
    (PDRV_PARID_TDW | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "RO-Array Par TDW", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00776: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    776U,               /**< parameter number */
    (PDRV_PARID_TD | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "RO-Array Par TD",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00777: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    777U,               /**< parameter number */
    (PDRV_PARID_I8 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    0xFFFEU,            /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0x80000000UL,       /**< low limit */
    0x7FFFFFFFUL,       /**< high limit */
    "RO-Array Par I8",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00778: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    778U,               /**< parameter number */
    (PDRV_PARID_O8 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    0xFFFEU,            /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "RO-Array Par O8",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00779: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    779U,               /**< parameter number */
    (PDRV_PARID_N2 | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    PDRV_UNIT_PCT,      /**< variable attribute */
    60000U,             /**< DO IO DATA reference parameter */
    0x800EU,            /**< DO IO DATA normalisation */
    FLOAT_N2_FAC,       /**< standardisation factor */
    0x08001U,           /**< low limit */
    0x07FFFU,           /**< high limit */
    "RO-Array Par N2",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00780: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    780U,               /**< parameter number */
    (PDRV_PARID_N4 | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    10U,                /**< number of elements or length of string */
    PDRV_UNIT_PCT,      /**< variable attribute */
    60000U,             /**< DO IO DATA reference parameter */
    0x801EU,            /**< DO IO DATA normalisation */
    FLOAT_N4_FAC,       /**< standardisation factor */
    0x80000001UL,       /**< low limit */
    0x7FFFFFFFUL,       /**< high limit */
    "RO-Array Par N4",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00781: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    781U,               /**< parameter number */
    (PDRV_PARID_V2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    10U,                /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFU,            /**< high limit */
    "RO-Array Par V2",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00782: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    782U,               /**< parameter number */
    (PDRV_PARID_L2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    10U,                /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFU,            /**< high limit */
    "RO-Array Par L2",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00783: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    783U,               /**< parameter number */
    (PDRV_PARID_R2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    10U,                /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    1U,                 /**< low limit */
    0x4000U,            /**< high limit */
    "RO-Array Par R2",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00784: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    784U,               /**< parameter number */
    (PDRV_PARID_T2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    10U,                /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x7FFFU,            /**< high limit */
    "RO-Array Par T2",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00785: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    785U,               /**< parameter number */
    (PDRV_PARID_T4 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    10U,                /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "RO-Array Par T4",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00786: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    786U,               /**< parameter number */
    (PDRV_PARID_D2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    10U,                /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x07FFFU,           /**< high limit */
    "RO-Array Par D2",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00787: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    787U,               /**< parameter number */
    (PDRV_PARID_E2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    10U,                /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFU,            /**< high limit */
    "RO-Array Par E2",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00720,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00800: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    800U,               /**< parameter number */
    (PDRV_PARID_BL | PDRV_PARID_NOSTD | PDRV_PARID_ADDTEXT), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0,                  /**< low limit */
    1,                  /**< high limit */
    "Test parameter 0", /**< pointer at name (16 valid characters) */
    pcPdrv_TfPnu00800,  /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00800,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00800    /**< function pointer - function is called after value is written */
)

/** PNU00801: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    801U,               /**< parameter number */
    (PDRV_PARID_I1 | PDRV_PARID_NOSTD), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    (PDRV_UINT32)-127,  /**< low limit */
    127U,               /**< high limit */
    "Test parameter 1", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00801,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00801    /**< function pointer - function is called after value is written */
)

/** PNU00802: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    802U,               /**< parameter number */
    (PDRV_PARID_I2),    /**< identifier */
    0U,                 /**< number of elements or length of string */
    PDRV_UNIT_RMIN,     /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_1,            /**< standardisation factor */
    (PDRV_UINT32)-32767,/**< low limit */
    32767,              /**< high limit */
    "Test parameter 2", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00802,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00802    /**< function pointer - function is called after value is written */
)

/** PNU00803: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    803U,               /**< parameter number */
    (PDRV_PARID_I4 | PDRV_PARID_NOSTD), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    (PDRV_UINT32)-0x7FFFFFFFL, /**< low limit */
    0x7FFFFFFFUL,       /**< high limit */
    "Test parameter 3", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00803,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00803    /**< function pointer - function is called after value is written */
)

/** PNU00804: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    804U,               /**< parameter number */
    (PDRV_PARID_O1 | PDRV_PARID_NOSTD), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    255U,               /**< high limit */
    "Test parameter 4", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00804,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00804    /**< function pointer - function is called after value is written */
)

/** PNU00805: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    805U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    65535UL,            /**< high limit */
    "Test parameter 5", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00805,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00805    /**< function pointer - function is called after value is written */
)

/** PNU00806: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    806U,               /**< parameter number */
    (PDRV_PARID_O4 | PDRV_PARID_NOSTD), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0UL,                /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "Test parameter 6", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00806,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00806    /**< function pointer - function is called after value is written */
)

/** PNU00807: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    807U,               /**< parameter number */
    (PDRV_PARID_F4 | PDRV_PARID_NOSTD), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0xc5bb8000UL,       /**< low limit -FLOAT_6000 */
    0x45bb8000UL,       /**< high limit FLOAT_6000 */
    "Test parameter 7", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00807,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00807    /**< function pointer - function is called after value is written */
)

/** PNU00808: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    808U,               /**< parameter number */
    (PDRV_PARID_V2 | PDRV_PARID_NOSTD), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0UL,                /**< low limit */
    0x0FFFFUL,          /**< high limit */
    "Test parameter 8", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00808,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00808    /**< function pointer - function is called after value is written */
)

/** PNU00809: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    809U,               /**< parameter number */
    (PDRV_PARID_BL | PDRV_PARID_NOSTD | PDRV_PARID_ARRAY), /**< identifier */
    242U,               /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0UL,                /**< low limit */
    1UL,                /**< high limit */
    "Test array 1",     /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00809,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00809    /**< function pointer - function is called after value is written */
)

/** PNU00810: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    810U,               /**< parameter number */
    (PDRV_PARID_I1 | PDRV_PARID_NOSTD | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0UL,                /**< low limit */
    0x7FUL,             /**< high limit */
    "Test array 2",     /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00810,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00810    /**< function pointer - function is called after value is written */
)

/** PNU00811: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    811U,               /**< parameter number */
    (PDRV_PARID_I2 | PDRV_PARID_NOSTD | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0UL,                /**< low limit */
    0x7FFFU,            /**< high limit */
    "Test array 3",     /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00811,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00811    /**< function pointer - function is called after value is written */
)

/** PNU00812: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    812U,               /**< parameter number */
    (PDRV_PARID_I4 | PDRV_PARID_NOSTD | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0UL,                /**< low limit */
    0x7FFFFFFFUL,       /**< high limit */
    "Test array 4",     /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00812,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00812    /**< function pointer - function is called after value is written */
)

/** PNU00813: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    813U,               /**< parameter number */
    (PDRV_PARID_O1 | PDRV_PARID_NOSTD | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFU,              /**< high limit */
    "Test array 4",     /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00813,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00813    /**< function pointer - function is called after value is written */
)

/** PNU00814: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    814U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFU,            /**< high limit */
    "Test array 5",     /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00814,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00814    /**< function pointer - function is called after value is written */
)

/** PNU00815: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    815U,               /**< parameter number */
    (PDRV_PARID_O4 | PDRV_PARID_NOSTD | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0xFFFFFFFFU,        /**< high limit */
    "Test array 6",     /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00815,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00815    /**< function pointer - function is called after value is written */
)

/** PNU00816: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    816U,               /**< parameter number */
    (PDRV_PARID_F4 | PDRV_PARID_NOSTD | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0xc5bb8000UL,       /**< low limit -FLOAT_6000 */
    0x45bb8000UL,       /**< high limit FLOAT_6000 */
    "Test array 7",     /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00816,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00816    /**< function pointer - function is called after value is written */
)

/** PNU00817: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    817U,               /**< parameter number */
    (PDRV_PARID_TD | PDRV_PARID_NOSTD | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0UL,                /**< low limit */
    0xFFFFFFFFUL,       /**< high limit */
    "Test array 8",     /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00817,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00817    /**< function pointer - function is called after value is written */
)

/** PNU00818: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    818U,               /**< parameter number */
    (PDRV_PARID_X2 | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    PDRV_UNIT_PCT,      /**< variable attribute */
    60000U,             /**< DO IO DATA reference parameter */
    0x800CU,            /**< DO IO DATA normalisation */
    FLOAT_X2_FAC,       /**< standardisation factor */
    (PDRV_UINT32)-0x7FFFU, /**< low limit */
    0x7FFFUL,            /**< high limit */
    "Test array 9",     /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00818,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00818    /**< function pointer - function is called after value is written */
)

/** PNU00819: test parameter for Profile Tester
 * @attention Parameter is used for test cases of Profile Tester. 
 * @attention THIS PARAMETER HAS TO BE DELETED BY THE PDRV USER
 */
PDRV_PARAMETER(
    819U,               /**< parameter number */
    (PDRV_PARID_X4 | PDRV_PARID_ARRAY), /**< identifier */
    9U,                 /**< number of elements or length of string */
    PDRV_UNIT_PCT,      /**< variable attribute */
    60000U,             /**< DO IO DATA reference parameter */
    0x801CU,            /**< DO IO DATA normalisation */
    FLOAT_X4_FAC,       /**< standardisation factor */
    (PDRV_UINT32)-0x7FFFFFFFU, /**< low limit */
    0x7FFFFFFFU,        /**< high limit */
    "Test array 10",    /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00819,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00819    /**< function pointer - function is called after value is written */
)

/* PNU00915: Selection switch for DO IO Data in the setpoint telegram, optional, DO-specific */
/* PNU00916: Selection switch for DO IO Data in the actual value telegram, optional, DO-specific */
/* PNU00919: Device or Axis system number, optional, DO-specific */

/** PNU00922: Telegram selection, mandatory, DO-specific */
PDRV_PARAMETER(
    922U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD ), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    1U,                 /**< low limit */
    3U,                 /**< high limit */
    "Telegram Select",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00922,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00922    /**< function pointer - function is called after value is written */
)

/* PNU00923: List of all parameters for signals, optional, DO-specific  */

/** PNU00925: Number of Controller Sign-Of-Life failures which may be tolerated, optional, DO-specific */
PDRV_PARAMETER(
    925U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD ), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    1U,                 /**< low limit */
    0xFFFFU,            /**< high limit */
    "max. C-LS faults", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00925,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00925    /**< function pointer - function is called after value is written */
)

/* PNU00927: Operation priority of parameters, optional, DO-specific (PROFINET) */
/* PNU00928: Control priority DO IO Data, optional, DO-specific */

/** PNU00930: Operating Mode, mandatory if other modes are implemented, DO-specific */
PDRV_PARAMETER(
    930U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD ), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    1U,                 /**< low limit */
    3U,                 /**< high limit */
    "Operating Mode",   /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00930,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00930    /**< function pointer - function is called after value is written */
)

/** PNU00944: Fault Message Counter, mandatory, DO-specific */
PDRV_PARAMETER(
    944U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x0FFFFU,           /**< high limit */
    "Fault Counter",    /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00944,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00945: Fault Code, optional, DO-specific */
PDRV_PARAMETER(
    945U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    PDRV_FAULT_BUFFER_LEN, /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x0FFFFU,           /**< high limit */
    "Fault Code",       /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00945,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00947: Fault Number, mandatory, DO-specific */
PDRV_PARAMETER(
    947U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    PDRV_FAULT_BUFFER_LEN, /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x0FFFFU,           /**< high limit */
    "Fault Number",     /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00947,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00948: Fault Time, optional, DO-specific */
PDRV_PARAMETER(
    948U,               /**< parameter number */
    (PDRV_PARID_TD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    PDRV_FAULT_BUFFER_LEN, /**< number of elements or length of string */
    PDRV_UNIT_SEC,      /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_1,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x0FFFFFFFFU,       /**< high limit */
    "Fault Time",       /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00948,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00949: Fault Value, optional, DO-specific */
PDRV_PARAMETER(
    949U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    PDRV_FAULT_BUFFER_LEN, /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x0FFFFU,           /**< high limit */
    "Fault Value",     /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00949,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00950: Scaling of the fault buffer, mandatory if non standard fault buffer size is used , DO-specific */
PDRV_PARAMETER(
    950U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    2U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x0FFFFU,           /**< high limit */
    "FaultBuffScaling", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00950,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00951: fault number list with text, mandatory if complete fault buffer, DO-specific */
PDRV_PARAMETER(
    951U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ADDTEXT | PDRV_PARID_ARRAY), /**< identifier */
    FAULT_MAX,          /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x0FFFFU,           /**< high limit */
    "FaultTextList",    /**< pointer at name (16 valid characters) */
    pcPdrv_TfPnu00951,  /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00951,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00952: Fault situation counter, mandatory if complete fault buffer, DO-specific */
PDRV_PARAMETER(
    952U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD | PDRV_PARID_RESETONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x0FFFFU,           /**< high limit */
    "\43 of Faults",    /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00952,   /**< function pointer - function is called before value is read */
    uPdrv_WfPnu00952    /**< function pointer - function is called after value is written */
)

/** PNU00953: warning parameters, optional, DO-specific */
PDRV_PARAMETER(
    953U,               /**< parameter number */
    (PDRV_PARID_V2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ADDTEXT), /**< identifier */
    0U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x0FFFFU,           /**< high limit */
    "Warning Param 1",  /**< pointer at name (16 valid characters) */
    pcPdrv_TfPnu00953,  /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00953,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/* PNU00954: Warning Param 2, warning parameters, optional, DO-specific */
/* PNU00955: Warning Param 3, warning parameters, optional, DO-specific */
/* PNU00956: Warning Param 4, warning parameters, optional, DO-specific */
/* PNU00957: Warning Param 5, warning parameters, optional, DO-specific */
/* PNU00958: Warning Param 6, warning parameters, optional, DO-specific */
/* PNU00959: Warning Param 7, warning parameters, optional, DO-specific */
/* PNU00960: Warning Param 8, warning parameters, optional, DO-specific */
/* PNU00962: sampling time to convert time parameters, mandatory if time data types are used, global */

/** PNU00964: Drive Unit identification, mandatory, global */
PDRV_PARAMETER(
    964U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    6U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x0FFFFU,           /**< high limit */
    "Drive Unit ID",    /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00964,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00965: Profile identification number, mandatory, DO-specific (PROFINET) */
PDRV_PARAMETER(
    965U,               /**< parameter number */
    (PDRV_PARID_OS | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    2U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x0FFU,             /**< high limit */
    "ProfileID Number", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00965,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/* PNU00967: Control Word 1, optional, DO-specific */
/* PNU00968: Status Word 1, optional, DO-specific */
/* PNU00968: Actual time difference, optional, DO-specific (PROFINET) */
/* PNU00970: Factory Setting, Load parameter set, optional, DO-specific */
/* PNU00971: EEPROM Saving, Transfer into a non-volatile memory, optional, DO-specific */
/* PNU00972: Power On Reset, Drive reset, optional, global */

/** PNU00974: Base Mode Parameter Access service identification, optional, Global for Homogeneous F-Devices */
PDRV_PARAMETER(
    974U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    3U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x0FFFFU,           /**< high limit */
    "Base Mode Par ID", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00974,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00975: DO identification, mandatory (PROFINET), DO-specific  */
PDRV_PARAMETER(
    975U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    8U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x0FFFFU,           /**< high limit */
    "Drive Object ID",  /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00975,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/* PNU00976: Load device parameter set, optional, global  */
/* PNU00977: Transfer in non-volatile memory (global), optional, global */
/* PNU00978: List of all DO-IDs, mandatory in special circumstances, global */

/** PNU00979: Sensor Format, mandatory if feedback interface is realised, DO-specific */
PDRV_PARAMETER(
    979U,               /**< parameter number */
    (PDRV_PARID_O4 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    31U,                /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x0FFFFU,           /**< high limit */
    "Sensor format",    /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00979,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU00980: Number list of defined parameter, mandatory, DO-specific */
PDRV_PARAMETER(
    980U,               /**< parameter number */
    (PDRV_PARID_O2 | PDRV_PARID_NOSTD | PDRV_PARID_READONLY | PDRV_PARID_ARRAY), /**< identifier */
    PDRV_NOOFPARAMETERS, /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x0FFFFU,           /**< high limit */
    "Par \43 List",     /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu00980,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/* PNU00981: Par # List pt2, Number list of defined parameter, mandatory, DO-specific */
/* PNU00982: Par # List pt3, Number list of defined parameter, mandatory, DO-specific */
/* PNU00983: Par # List pt4, Number list of defined parameter, mandatory, DO-specific */
/* PNU00984: Par # List pt5, Number list of defined parameter, mandatory, DO-specific */
/* PNU00985: Par # List pt6, Number list of defined parameter, mandatory, DO-specific */
/* PNU00986: Par # List pt7, Number list of defined parameter, mandatory, DO-specific */
/* PNU00987: Par # List pt8, Number list of defined parameter, mandatory, DO-specific */
/* PNU00988: Par # List pt9, Number list of defined parameter, mandatory, DO-specific */
/* PNU00989: Par # List pt10, Number list of defined parameter, mandatory, DO-specific */

/* PNU00990: Par # List chg1, Number list of changed parameter, optional, DO-specific */
/* PNU00991: Par # List chg2, Number list of changed parameter, optional, DO-specific */
/* PNU00992: Par # List chg3, Number list of changed parameter, optional, DO-specific */
/* PNU00993: Par # List chg4, Number list of changed parameter, optional, DO-specific */
/* PNU00994: Par # List chg5, Number list of changed parameter, optional, DO-specific */
/* PNU00995: Par # List chg6, Number list of changed parameter, optional, DO-specific */
/* PNU00996: Par # List chg7, Number list of changed parameter, optional, DO-specific */
/* PNU00997: Par # List chg8, Number list of changed parameter, optional, DO-specific */
/* PNU00998: Par # List chg9, Number list of changed parameter, optional, DO-specific */
/* PNU00999: Par # ListChg10, Number list of changed parameter, optional, DO-specific */

/* PNU01000..PNU59999: device specific, insert your parameters here -------------------------------------------------*/

/* PNU60000..PNU65535 are reserved for PROFIdrive -------------------------------------------------------------------*/

/** PNU60000: Velocity reference value, mandatory if NSOLL_x or NIST_x are used, DO-specific */
PDRV_PARAMETER(
    60000U,             /**< parameter number */
    (PDRV_PARID_F4 | PDRV_PARID_READONLY), /**< identifier */
    0U,                 /**< number of elements or length of string */
    PDRV_UNIT_RMIN,     /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_1,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x45bb8000UL,       /**< high limit FLOAT_6000 */
    "VelocityRefValue", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu60000,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU61000: Name of Station, optional, global */
PDRV_PARAMETER(
    61000U,             /**< parameter number */
    (PDRV_PARID_OS | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    240U,               /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x000FFU,           /**< high limit */
    "NameOfStation",    /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu61000,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU61001: IP of Station, optional, global */
PDRV_PARAMETER(
    61001U,             /**< parameter number */
    (PDRV_PARID_OS | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    4U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x000FFU,           /**< high limit */
    "IpOfStation",      /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu61001,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU61002: MAC of Station, optional, global */
PDRV_PARAMETER(
    61002U,             /**< parameter number */
    (PDRV_PARID_OS | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    6U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x000FFU,           /**< high limit */
    "MacOfStation",     /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu61002,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU61003: Default Gateway of Station, optional, global */
PDRV_PARAMETER(
    61003U,             /**< parameter number */
    (PDRV_PARID_OS | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    4U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x000FFU,           /**< high limit */
    "DefGateOfStation", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu61001,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/** PNU61004: Subnet Mask of Station, optional, global */
PDRV_PARAMETER(
    61004U,             /**< parameter number */
    (PDRV_PARID_OS | PDRV_PARID_NOSTD | PDRV_PARID_READONLY), /**< identifier */
    4U,                 /**< number of elements or length of string */
    0U,                 /**< variable attribute */
    0U,                 /**< DO IO DATA reference parameter */
    0U,                 /**< DO IO DATA normalisation */
    FLOAT_0,            /**< standardisation factor */
    0U,                 /**< low limit */
    0x000FFU,           /**< high limit */
    "SubMaskOfStation", /**< pointer at name (16 valid characters) */
    PDRV_NULL_T,        /**< function pointer - function is called if additional text is read */
    uPdrv_RfPnu61001,   /**< function pointer - function is called before value is read */
    PDRV_NULL_W         /**< function pointer - function is called after value is written */
)

/* PNU65000: Preset value, optional, DO-specific */

