/*********************************************************************************************************************/
/**@file        pdrv_diagnostics_ac4.h
 * @brief       PROFIdrive Diagnostics header file
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
 *              Diagnostics implements the PROFIdrive fault and warning handling.
 *
 * @internal
 * @note        Doxygen comments are used for automatic documentation generation.
 *              For further information about Doxygen please look at http://www.stack.nl/~dimitri/doxygen/index.html
 *              IDE Eclipse's folding feature is used too. see https://www.eclipse.org
 * @endinternal
*/
/*********************************************************************************************************************/

#ifndef PDRV_DIAGNOSTICS_AC4_H
#define PDRV_DIAGNOSTICS_AC4_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

/** @name Fault buffer mechanism
  * : scaling of fault buffer PNU950 (see PDRV V4.2 figure 62)
  * @{
*/
#define PDRV_FAULT_SITUATIONS_NR   (8U) /**< number of fault situations PNU950.0 */
#define PDRV_FAULT_MESSAGES_NR     (8U) /**< number of fault messages per fault situation PNU950.1 */
#define PDRV_FAULT_BUFFER_LEN     (64U) /**< number of all fault messages = PNU950.0 * PNU950.1 */
/** @} */

/** all possible faults in a sequential list
 * @details It is index of fault table and bit number in the fault bit array.
 * @attention THIS LIST HAS TO BE MODIFIED BY THE PDRV USER
*/
typedef enum
{
    FAULT_NONE,         /**< value for no fault is zero! */
    FAULT_USER0,        /**< PDRV user defined fault, should be changed */
    FAULT_USER1,        /**< PDRV user defined fault, should be changed */
    FAULT_USER2,        /**< PDRV user defined fault, should be changed */
    FAULT_USER3,        /**< PDRV user defined fault, should be changed */
    FAULT_USER4,        /**< PDRV user defined fault, should be changed */
    FAULT_USER5,        /**< PDRV user defined fault, should be changed */
    FAULT_USER6,        /**< PDRV user defined fault, should be changed */
    FAULT_USER7,        /**< PDRV user defined fault, should be changed */
    FAULT_USER8,        /**< PDRV user defined fault, should be changed */
    FAULT_USER9,        /**< PDRV user defined fault, should be changed */
    FAULT_INTERNALDIAG, /**< fault in diagnostics modul */
    FAULT_SIGNOFLIFE,   /**< fault in isochronous mode with sign of life */
    FAULT_SENSOR,       /**< sensor fault */
    FAULT_MAX           /**< LAST entry, it is used internal for implementation */
} PDRV_DiagFaultNumbers;

/** all possible warnings in a sequential list
 * @details It is bit number in the warning bit array.
 * @attention THIS LIST HAS TO BE MODIFIED BY THE PDRV USER
*/
typedef enum
{
    WARNING_USER0,        /**< PDRV user defined warning, should be changed */
    WARNING_USER1,        /**< PDRV user defined warning, should be changed */
    WARNING_USER2,        /**< PDRV user defined warning, should be changed */
    WARNING_USER3,        /**< PDRV user defined warning, should be changed */
    WARNING_USER4,        /**< PDRV user defined warning, should be changed */
    WARNING_USER5,        /**< PDRV user defined warning, should be changed */
    WARNING_USER6,        /**< PDRV user defined warning, should be changed */
    WARNING_USER7,        /**< PDRV user defined warning, should be changed */
    WARNING_USER8,        /**< PDRV user defined warning, should be changed */
    WARNING_USER9,        /**< PDRV user defined warning, should be changed */
    WARNING_INTERNALDIAG, /**< fault in diagnostics modul */
    WARNING_MAX           /**< LAST entry, it is used internal for implementation */
} PDRV_DiagWarningNumbers;

/** definition of the PROFIdrive fault classes
 * @details see PDRV V4.2 table 117
*/
typedef enum
{
    FAULTCL_MC_HARDSOFTWARE     = 1,    /**< Microcontroller Hardware or Software */
    FAULTCL_MAINS_SUPPLY        = 2,    /**< Mains Supply */
    FAULTCL_LOW_VOLTAGE_SUPPLY  = 3,    /**< Low Voltage Supply */
    FAULTCL_DC_LINK_OVERVOLTAGE = 4,    /**< DC Link Overvoltage */
    FAULTCL_POWER_ELECTRONICS   = 5,    /**< Power Electronics */
    FAULTCL_OVERTEMP_ELECTRODEV = 6,    /**< Overtemperature Electronic Device */
    FAULTCL_EARTH_GROUND_FAULT  = 7,    /**< Earth or Ground Fault */
    FAULTCL_MOTOR_OVERLOAD      = 8,    /**< Motor Overload */
    FAULTCL_FIELDBUS_SYSTEM     = 9,    /**< Fieldbus System */
    FAULTCL_SAFETY_CHANNEL      =10,    /**< Safety Channel */
    FAULTCL_FEEDBACK            =11,    /**< Feedback */
    FAULTCL_INTERNAL_COMM       =12,    /**< Internal Communication */
    FAULTCL_INFEED              =13,    /**< Infeed */
    FAULTCL_BRAKE_RESISTOR      =14,    /**< Brake Resistor */
    FAULTCL_LINE_FILTER         =15,    /**< Line Filter */
    FAULTCL_EXTERNAL            =16,    /**< External */
    FAULTCL_TECHNOLOGY          =17,    /**< Technology */
    FAULTCL_ENGINEERING         =18,    /**< Engineering */
    FAULTCL_OTHER               =19,    /**< Other */
    FAUTLCL_AUXILIARYDEV        =20,    /**< Auxiliary Device */   
} PDRV_DiagFaultClasses;

/** definition of PROFINET ChannelPropertiesSpecifier
 * @details see PDRV V4.2 table 188 (same definition like DIAG_CHANPROP_SPEC_ERR in pniousrd.h)
*/
typedef enum
{
    PDRV_CHANPROP_SPEC_ERR_DISAPP_ALL   = 0,    /*< all subsequent errors disappear for this channel */
    PDRV_CHANPROP_SPEC_ERR_APP          = 1,    /*< new error appears */
    PDRV_CHANPROP_SPEC_ERR_DISAPP       = 2,    /*< error disappears, no more error */
    PDRV_CHANPROP_SPEC_ERR_DISAPP_MORE  = 3     /*< error disappears, but other errors remain */
} PDRV_CHANPROP_SPEC_ERR;

extern PDRV_VOID PdrvDiag_Init(PDRV_VOID);
extern PDRV_VOID PdrvDiag_SetFaultMsg(PDRV_DiagFaultNumbers p_eFaultBitNr, PDRV_UINT16 p_uFaultValue);
extern PDRV_VOID PdrvDiag_SetWarning(PDRV_DiagWarningNumbers p_eWarningNr,  PDRV_BOOL p_bWarnVal);
extern PDRV_VOID PdrvDiag_AckFaultSit(PDRV_VOID);
extern PDRV_VOID PdrvDiag_ResetFaultBuffer(PDRV_VOID);
extern PDRV_VOID PdrvDiag_AckSenFault(PDRV_VOID);
extern PDRV_BOOL bPdrvDiag_IsFaultOff1(PDRV_VOID);
extern PDRV_BOOL bPdrvDiag_IsFaultOff2(PDRV_VOID);
extern PDRV_BOOL bPdrvDiag_IsFaultOff3(PDRV_VOID);

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif
