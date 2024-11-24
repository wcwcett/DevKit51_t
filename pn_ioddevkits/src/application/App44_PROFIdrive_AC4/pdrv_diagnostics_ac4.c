/*********************************************************************************************************************/
/**@file        pdrv_diagnostics_ac4.c
 * @brief       PROFIdrive Diagnostics mechanisms
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
 *              Shorthand symbol: Diag (DIAGnostics)<br>
 *              Diagnostics implements the fault and warning handling.
 *              see PDRV V4.2 figure 58
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
#include "pdrv_diagnostics_ac4.h"
#include "pdrv_statemachine_ac4.h"
#include "pdrv_setpointchannel_ac4.h"
#include "pdrv_application_ac4.h"
#include "pdrv_parmanager_ac4.h"
#include "pdrv_pardatabase_ac4.h"
#include "pdrv_statemachine_ac4.h"

#if (EXAMPL_DEV_CONFIG_VERSION == 44)

/*------------  extern  functions  ------------*/
extern PDRV_UINT32 uPdrvUsr_ChanDiag (PDRV_UINT16 p_uAlarmState, PDRV_UINT16 p_uErrorNum, PDRV_UINT16 p_uDiagTag, PDRV_BOOL p_bIsWarning);

/*------------  extern  data  ------------*/

/*------------  type definitions, constants, enums  ------------*/
static PDRV_UINT uGetCurSitIndex(void);

/** fault reactions */
typedef enum
{
    FAULT_NOREACT,  /**< no additional reaction */
    FAULT_OFF1,     /**< request OFF1 (power down) */
    FAULT_OFF2,     /**< request OFF2 (coast stop) */
    FAULT_OFF3      /**< request OFF3 (quick stop) */
} PDRV_FAULTREACT;

/** warning attributes for m_tWarnAttribs[] */
typedef struct
{
    PDRV_DiagFaultClasses eClass;   /**< PROFIdrive fault class */
} PDRV_WARNINGATTRIBS;

/** fault attributes for m_tFaultAttribs[] */
typedef struct
{
    PDRV_UINT16 uCode;              /**< fault code PNU945 (optional) */
    PDRV_UINT16 uNr;                /**< fault number PNU947 (mandatory) */
    PDRV_FAULTREACT eReact;         /**< fault reaction (user specific) */
    PDRV_DiagFaultClasses eClass;   /**< PROFIdrive fault class */
    const char * szTxt;             /**< text (with 16 valid characters) for parameter PNU00951 (mandatory for complete fault buffer)*/
} PDRV_FAULTATTRIBS;

#define BITS_PER_WARNINGWORD    (16U)   /**< number of bit in m_uWarningBitArray[] */
#define BITS_PER_FAULTWORD      (32U)   /**< number of bit in m_uFaultBitArray[] */

/*------------  static  data  ------------*/

static PDRV_UINT16 m_uWarningBitArray[WARNING_MAX/sizeof(PDRV_UINT16) + 1U];/**< warning bit array PNU953..960 optional */
static PDRV_UINT32 m_uFaultBitArray[FAULT_MAX/sizeof(PDRV_UINT32) + 1U];    /**< fault bit array */

/** warning table, ePDRV_DiagWarningNumbers is index
 * @attention THIS TABLE HAS TO BE MODIFIED BY THE PDRV USER
*/
static const PDRV_WARNINGATTRIBS m_tWarningAttribs[WARNING_MAX + 1U] =
{/* Warning                  fault class */
  [WARNING_USER0]         = {FAULTCL_FIELDBUS_SYSTEM},
  [WARNING_USER1]         = {FAULTCL_MOTOR_OVERLOAD},
  [WARNING_USER2]         = {FAULTCL_OTHER},
  [WARNING_USER3]         = {FAULTCL_MAINS_SUPPLY},
  [WARNING_USER4]         = {FAUTLCL_AUXILIARYDEV},
  [WARNING_USER5]         = {FAULTCL_OVERTEMP_ELECTRODEV},
  [WARNING_USER6]         = {FAULTCL_MOTOR_OVERLOAD},
  [WARNING_USER7]         = {FAULTCL_FEEDBACK},
  [WARNING_USER8]         = {FAULTCL_ENGINEERING},
  [WARNING_USER9]         = {FAULTCL_TECHNOLOGY},
  [WARNING_INTERNALDIAG]  = {FAULTCL_MC_HARDSOFTWARE},
  [WARNING_MAX]           = {FAULTCL_MC_HARDSOFTWARE}
};

/** fault table, ePDRV_DiagFaultNumbers is index
 * @attention THIS TABLE HAS TO BE MODIFIED BY THE PDRV USER
*/
static const PDRV_FAULTATTRIBS m_tFaultAttribs[FAULT_MAX + 1U] =
{/* Fault                  code, nr,      reaction, fault class,                  , fault text */
  [FAULT_NONE]          = {   0,  0, FAULT_NOREACT, FAULTCL_MC_HARDSOFTWARE,       "No fault"},
  [FAULT_USER0]         = {1000,  1, FAULT_NOREACT, FAULTCL_INTERNAL_COMM,         "User fault #0"},
  [FAULT_USER1]         = {1001,  2, FAULT_OFF1,    FAULTCL_LINE_FILTER,           "User fault #1"},
  [FAULT_USER2]         = {2002,  3, FAULT_OFF2,    FAULTCL_POWER_ELECTRONICS,     "User fault #2"},
  [FAULT_USER3]         = {3003,  4, FAULT_OFF3,    FAULTCL_DC_LINK_OVERVOLTAGE,   "User fault #3"},
  [FAULT_USER4]         = {4004,  5, FAULT_OFF1,    FAULTCL_BRAKE_RESISTOR,        "User fault #4"},
  [FAULT_USER5]         = {5005,  6, FAULT_NOREACT, FAULTCL_FIELDBUS_SYSTEM,       "User fault #5"},
  [FAULT_USER6]         = {6006,  7, FAULT_OFF1,    FAULTCL_EARTH_GROUND_FAULT,    "User fault #6"},
  [FAULT_USER7]         = {7007,  8, FAULT_OFF1,    FAULTCL_INFEED,                "User fault #7"},
  [FAULT_USER8]         = {8008,  9, FAULT_NOREACT, FAULTCL_EXTERNAL,              "User fault #9"},
  [FAULT_USER9]         = {9009, 10, FAULT_OFF1,    FAULTCL_OTHER,                 "User fault #A"},
  [FAULT_INTERNALDIAG]  = {  42, 11, FAULT_OFF1,    FAULTCL_MC_HARDSOFTWARE,       "Internal fault"},
  [FAULT_SIGNOFLIFE]    = {  43, 12, FAULT_OFF1,    FAULTCL_FIELDBUS_SYSTEM,       "Sign of life"},
  [FAULT_SENSOR]        = {  44, 13, FAULT_OFF1,    FAUTLCL_AUXILIARYDEV,          "Sensor fault"},
  [FAULT_MAX]           = {   0, 14, FAULT_OFF1,    FAULTCL_MC_HARDSOFTWARE,       "fatal fault"}
};

static PDRV_UINT16 m_uFaultMsgCnt;    /**< fault message counter PNU944 (mandatory) */
static PDRV_UINT16 m_uFaultSitCnt;    /**< fault situation counter PNU952 (mandatory for complete fault buffer) */
static PDRV_UINT16 m_uFaultCodes[PDRV_FAULT_BUFFER_LEN];    /**< fault code   PNU945 (optional) as circular buffer */
static PDRV_UINT16 m_uFaultNumbers[PDRV_FAULT_BUFFER_LEN];  /**< fault number PNU947 (mandatory) as circular buffer */
static PDRV_UINT32 m_uFaultTimes[PDRV_FAULT_BUFFER_LEN];    /**< fault time   PNU948 (optional) as circular buffer */
static PDRV_UINT16 m_uFaultValues[PDRV_FAULT_BUFFER_LEN];   /**< fault value  PNU949 (optional) as circular buffer */
static PDRV_UINT16 m_uFaultNrIndex;   /**< index of next fault message in the fault buffers */
static PDRV_BOOL m_bFaultOff1;        /**< fault reaction OFF1 (power down) is demanded */
static PDRV_BOOL m_bFaultOff2;        /**< fault reaction OFF2 (coast stop) is demanded */
static PDRV_BOOL m_bFaultOff3;        /**< fault reaction OFF3 (quick stop) is demanded */

/*------------  public data  ------------*/

/** PROFIdrive diagnostic init
 *  @details    Initialization of the diagnostic internal data.
 *              This function should be called during initialization.
 *  @return     PDRV_VOID
*/
PDRV_VOID PdrvDiag_Init(PDRV_VOID)
{
    PDRV_UINT uI;

    for(uI= 0U; uI < sizeof(m_uWarningBitArray)/sizeof(m_uWarningBitArray[0]); uI++)
    {
        m_uWarningBitArray[uI] = 0U;
    }
    PdrvDiag_ResetFaultBuffer();
    m_uFaultSitCnt = 0;
}

/** set/clear a warning in the warning buffer
 * @details see PDRV V4.2 figure 59
 * @return PDRV_VOID
 */
PDRV_VOID PdrvDiag_SetWarning
    (PDRV_DiagWarningNumbers p_eWarningNr,  /**< [in] warning number */
     PDRV_BOOL p_bWarnVal                   /**< [in] PDRV_TRUE - set warning, PDRV_FALSE - clear warning */
    )
{
    PDRV_UINT uI;           /* index counter */
    PDRV_BOOL bIsWarning;   /* warning active */
    PDRV_UINT uWordPos;     /* index within m_uWarningBitArray[] */
    PDRV_UINT uBitMsk;      /* bit mask of actual warning number for m_uWarningBitArray[uWordPos] */

    /* invalid warning number? */
    if (p_eWarningNr >= WARNING_MAX)
    {
        p_bWarnVal = PDRV_TRUE;
        p_eWarningNr = WARNING_INTERNALDIAG;
    }

    uWordPos = p_eWarningNr / BITS_PER_WARNINGWORD;
    uBitMsk  = 1U << (p_eWarningNr % BITS_PER_WARNINGWORD);

    if (p_bWarnVal == PDRV_FALSE)
    { /* clear a warning */
        m_uWarningBitArray[uWordPos] &= ~uBitMsk;   /* reset warning bit */

        /* any other warning active? */
        bIsWarning = PDRV_FALSE;
        for(uI= 0U; uI < sizeof(m_uWarningBitArray)/sizeof(m_uWarningBitArray[0]); uI++)
        {
            if (m_uWarningBitArray[uI] != 0U)
            {
                bIsWarning = PDRV_TRUE;
            }
        }
        if (bIsWarning == PDRV_FALSE)
        {
            PdrvSpc_ClrBitsZsw1(PDRV_ZSW1_WARNING);    /* clear warning bit in ZSW1 */
        }

        /* disappearance message via PROFINET IO alarm mechanism */
        uPdrvUsr_ChanDiag(uPdrvSpc_GetZsw1() & (PDRV_ZSW1_FAULT | PDRV_ZSW1_WARNING) ? PDRV_CHANPROP_SPEC_ERR_DISAPP_MORE : PDRV_CHANPROP_SPEC_ERR_DISAPP,
                          m_tWarningAttribs[p_eWarningNr].eClass,
                          p_eWarningNr,
                          PDRV_TRUE);
    }
    else
    { /* set a warning */
        m_uWarningBitArray[uWordPos] |= uBitMsk;    /* set warning bit */
        PdrvSpc_SetBitsZsw1(PDRV_ZSW1_WARNING);     /* set warning bit in ZSW1 */

        /* appearance message via PROFINET IO alarm mechanism */
        uPdrvUsr_ChanDiag(PDRV_CHANPROP_SPEC_ERR_APP,
                          m_tWarningAttribs[p_eWarningNr].eClass,
                          p_eWarningNr,
                          PDRV_TRUE);
    }
}

/** set a new fault message in actual fault situation
 * @details see PDRV V4.2 figure 62
 * @return PDRV_VOID
 */
PDRV_VOID PdrvDiag_SetFaultMsg
    (PDRV_DiagFaultNumbers p_eIntFaultNr,   /**< [in] internal fault number */
     PDRV_UINT16 p_uFaultValue              /**< [in] fault value PNU949 */
    )
{
    PDRV_UINT uWordPos;     /* index within m_uFaultBitArray[] */
    PDRV_UINT uBitMsk;      /* bit mask of actual fault number for m_uFaultBitArray[uWordPos] */
    PDRV_UINT16 uMsgPos;    /* message position within current situation */

    /* invalid internal fault number? */
    if (p_eIntFaultNr >= FAULT_MAX)
    {
        p_uFaultValue = p_eIntFaultNr;
        p_eIntFaultNr = FAULT_INTERNALDIAG; /* invalid number is replaced by internal fault */
    }

    uWordPos = p_eIntFaultNr / BITS_PER_FAULTWORD;
    uBitMsk  = 1U << (p_eIntFaultNr % BITS_PER_FAULTWORD);

    /* new fault? */
    if (!(m_uFaultBitArray[uWordPos] & uBitMsk))
    {
        m_uFaultBitArray[uWordPos] |= uBitMsk;  /* flag fault */

        /* record fault into fault buffers */
        m_uFaultNumbers[m_uFaultNrIndex] = m_tFaultAttribs[p_eIntFaultNr].uNr;
        m_uFaultCodes[m_uFaultNrIndex] = m_tFaultAttribs[p_eIntFaultNr].uCode;
        m_uFaultTimes[m_uFaultNrIndex] = uPdrvApp_GetTimer1s();
        m_uFaultValues[m_uFaultNrIndex] = p_uFaultValue;

        uMsgPos = m_uFaultNrIndex % PDRV_FAULT_MESSAGES_NR; /* last message position within current situation */
        /* not the last message within current situation? */
        if (uMsgPos < (PDRV_FAULT_MESSAGES_NR - 1))
        {
            m_uFaultNrIndex++;
        }

        /* first message within current situation? */
        if (uMsgPos == 0U)
        {
            /* set ZSW1 fault flag */
            PdrvSpc_SetBitsZsw1(PDRV_ZSW1_FAULT);

            /* no overflow for fault situation counter? */
            if (m_uFaultSitCnt < 0xFFFFU)
            {
                m_uFaultSitCnt++;
            }
        }

        /* overflow of current message counter? */
        if (m_uFaultMsgCnt == 0xFFFFU)
        {
            m_uFaultMsgCnt=0;   /* set fault message counter to 1 (0 is incremented afterwards) */
        }
        m_uFaultMsgCnt++;       /* increment fault message counter */

        /* additional fault reaction? */
        switch (m_tFaultAttribs[p_eIntFaultNr].eReact)
        {
        case FAULT_OFF1:    /* request fault reaction OFF1 */
            m_bFaultOff1 = PDRV_TRUE;
            break;
        case FAULT_OFF2:    /* request fault reaction OFF2 */
            m_bFaultOff2 = PDRV_TRUE;
            break;
        case FAULT_OFF3:    /* request fault reaction OFF3 */
            m_bFaultOff3 = PDRV_TRUE;
            break;
        default:            /* no additional reaction needed */
            break;
        }

        /* appearance message via PROFINET IO alarm mechanism */
        uPdrvUsr_ChanDiag(PDRV_CHANPROP_SPEC_ERR_APP,
                          m_tFaultAttribs[p_eIntFaultNr].eClass,
                          p_eIntFaultNr,
                          PDRV_FALSE);
    }
}

/** acknowledge a fault situation
 * @details Acknowledge a fault situation i.e. fault situation is shifted and all current faults are cleared.
 *          see PDRV V4.2 figure 61
 * @return PDRV_VOID
 */
PDRV_VOID PdrvDiag_AckFaultSit(PDRV_VOID)
{
    PDRV_UINT uI;
    PDRV_UINT16 uFaultNrIndex;
    PDRV_DiagFaultNumbers eIntFaultNr = 0;

    /* any fault message in the actual fault situation? */
    if (m_uFaultNrIndex % PDRV_FAULT_MESSAGES_NR)
    { /* there are unacknowledged fault messages */
        /* remember an old fault entry */
        eIntFaultNr = m_uFaultValues[uGetCurSitIndex()];

        /* calculate new index of new fault situation in circular buffer */
        uFaultNrIndex = PDRV_FAULT_BUFFER_LEN - PDRV_FAULT_MESSAGES_NR;
        if (m_uFaultNrIndex >= PDRV_FAULT_MESSAGES_NR)
        {
            uFaultNrIndex = (m_uFaultNrIndex / PDRV_FAULT_MESSAGES_NR - 1U) * PDRV_FAULT_MESSAGES_NR;
        }
        m_uFaultNrIndex = uFaultNrIndex;

        /* clear new fault situation */
        for(uI= uFaultNrIndex; uI < (uFaultNrIndex + PDRV_FAULT_MESSAGES_NR); uI++)
        {
            m_uFaultCodes[uI] = 0U;
            m_uFaultNumbers[uI] = 0U;
            m_uFaultTimes[uI] = 0U;
            m_uFaultValues[uI] = 0U;
        }

        /* overflow of current message counter? */
        if (m_uFaultMsgCnt == 0xFFFFU)
        {
            m_uFaultMsgCnt=0;   /* set fault message counter to 1 (0 is incremented afterwards) */
        }
        m_uFaultMsgCnt++;       /* increment fault message counter */

        /* reset internal bit flag buffer for every fault message */
        for(uI= 0U; uI < sizeof(m_uFaultBitArray)/sizeof(m_uFaultBitArray[0]); uI++)
        {
            m_uFaultBitArray[uI] = 0U;
        }
        m_bFaultOff1 = PDRV_FALSE;
        m_bFaultOff2 = PDRV_FALSE;
        m_bFaultOff3 = PDRV_FALSE;

        PdrvSpc_ClrBitsZsw1(PDRV_ZSW1_FAULT);  /* clear ZSW1 fault flag */

        /* disappearance message via PROFINET IO alarm mechanism */
        uPdrvUsr_ChanDiag(uPdrvSpc_GetZsw1() & PDRV_ZSW1_WARNING ? PDRV_CHANPROP_SPEC_ERR_DISAPP_MORE : PDRV_CHANPROP_SPEC_ERR_DISAPP,
                          m_tFaultAttribs[eIntFaultNr].eClass,
                          eIntFaultNr,
                          PDRV_FALSE);
    }
}

/** resets the fault buffers to ZERO, acknowledge all current faults
 * @details reset the fault buffers if PNU952 is set to 0
 *          see PDRV V4.2 figure 61
 * @return PDRV_VOID
 */
PDRV_VOID PdrvDiag_ResetFaultBuffer(PDRV_VOID)
{
    PDRV_UINT uI;

    for(uI= 0U; uI < sizeof(m_uFaultBitArray)/sizeof(m_uFaultBitArray[0]); uI++)
    {
        m_uFaultBitArray[uI] = 0U;
    }
    for(uI= 0U; uI < PDRV_FAULT_BUFFER_LEN; uI++)
    {
        m_uFaultCodes[uI] = 0U;
        m_uFaultNumbers[uI] = 0U;
        m_uFaultTimes[uI] = 0U;
        m_uFaultValues[uI] = 0U;
    }
    m_uFaultMsgCnt = 0U;
    m_uFaultNrIndex = 0U;
    m_bFaultOff1 = PDRV_FALSE;
    m_bFaultOff2 = PDRV_FALSE;
    m_bFaultOff3 = PDRV_FALSE;

    PdrvSpc_ClrBitsZsw1(PDRV_ZSW1_FAULT);  /* clear ZSW1 fault flag */
}

/** acknowledge a sensor fault situation only
 * @details Acknowledge a fault situation if only sensor faults exist.
 *          see PDRV V4.2 table 108
 * @return PDRV_VOID
 */
PDRV_VOID PdrvDiag_AckSenFault(PDRV_VOID)
{
    PDRV_UINT uCurSitIndex = uGetCurSitIndex();

    /* Is there only a sensor fault in current fault situation? */
    if (   (m_uFaultNumbers[uCurSitIndex + 1U] == 0U)
        && (m_uFaultNumbers[uCurSitIndex] == m_tFaultAttribs[FAULT_SENSOR].uNr)
       )
    {
        PdrvDiag_AckFaultSit();
    }

}

    /** fault reaction OFF1 (power down) is demanded
 *
 * @return TRUE - fault reaction OFF1 is demanded
 */
PDRV_BOOL bPdrvDiag_IsFaultOff1(PDRV_VOID)
{
    return m_bFaultOff1;
}

/** fault reaction OFF2 (coast stop) is demanded
 *
 * @return TRUE - fault reaction OFF2 is demanded
 */
PDRV_BOOL bPdrvDiag_IsFaultOff2(PDRV_VOID)
{
    return m_bFaultOff2;
}

/** fault reaction OFF3 (quick stop) is demanded
 *
 * @return TRUE - fault reaction OFF3 is demanded
 */
PDRV_BOOL bPdrvDiag_IsFaultOff3(PDRV_VOID)
{
    return m_bFaultOff3;
}

/** calculates the first index in circular buffer for the current situation
 *  @details
 *  @return     first index in circular buffer for the current situation
*/
static PDRV_UINT uGetCurSitIndex(PDRV_VOID)
{
    return (m_uFaultNrIndex / PDRV_FAULT_MESSAGES_NR) * PDRV_FAULT_MESSAGES_NR;
}

/*------------  parameter manager assigned text functions, read functions, write functions  ------------*/

/** PROFIdrive read function for parameter PNU00944 "fault message counter"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00944
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->o2[0U] = m_uFaultMsgCnt;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00945 "fault code"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00945
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;
    PDRV_UINT uJ;

    uJ = uGetCurSitIndex() + p_uSubindex;
    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        if (uJ >= PDRV_FAULT_BUFFER_LEN)
        {
            uJ -=  PDRV_FAULT_BUFFER_LEN;
        }
        p_ptValues->o2[uI] = m_uFaultCodes[uJ++];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00947 "fault number"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00947
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;
    PDRV_UINT uJ;

    uJ = uGetCurSitIndex() + p_uSubindex;
    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        if (uJ >= PDRV_FAULT_BUFFER_LEN)
        {
            uJ -=  PDRV_FAULT_BUFFER_LEN;
        }
        p_ptValues->o2[uI] = m_uFaultNumbers[uJ++];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00948 "fault time"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00948
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;
    PDRV_UINT uJ;

    uJ = uGetCurSitIndex() + p_uSubindex;
    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        if (uJ >= PDRV_FAULT_BUFFER_LEN)
        {
            uJ -=  PDRV_FAULT_BUFFER_LEN;
        }
        p_ptValues->o4[uI] = m_uFaultTimes[uJ++];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00949 "fault value"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00949
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;
    PDRV_UINT uJ;

    uJ = uGetCurSitIndex() + p_uSubindex;
    for (uI = 0; uI < p_uNrOfElements; uI++)
    {
        if (uJ >= PDRV_FAULT_BUFFER_LEN)
        {
            uJ -=  PDRV_FAULT_BUFFER_LEN;
        }
        p_ptValues->o2[uI] = m_uFaultValues[uJ++];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00950 "Scaling of the fault buffer"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00950
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->o2[uI] = ((p_uSubindex + uI) == 0U)? PDRV_FAULT_SITUATIONS_NR : PDRV_FAULT_MESSAGES_NR;
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive text read function for parameter PNU00951 "fault text list"
 *  @details
 *  @return     pointer to text string
*/
const char * pcPdrv_TfPnu00951
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex            /**< [in] subindex to read */
    )
{
    return m_tFaultAttribs[p_uSubindex].szTxt;
}

/** PROFIdrive read function for parameter PNU00951 "fault text list"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00951
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->o2[uI] = 0U;    /* no link between error and a parameter number yet */
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00952 "number of faults"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00952
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->o2[0U] = m_uFaultSitCnt;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00952 "number of faults"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00952
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PdrvDiag_ResetFaultBuffer();
    m_uFaultSitCnt = 0U;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00953 "warning parameter"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00953
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->o2[0U] = m_uWarningBitArray[0U];
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive text read function for parameter PNU00953 "Warning Param 1"
 *  @details
 *  @return     pointer to text string
*/
const char * pcPdrv_TfPnu00953
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex            /**< [in] first subindex to write/read */
    )
{
    static const char * pcBitTexts[32] =
        {
                "no warning #1",   "warning #1",
                "no warning #2",   "warning #2",
                "no warning #3",   "warning #3",
                "no warning #4",   "warning #4",
                "no warning #5",   "warning #5",
                "no warning #6",   "warning #6",
                "no warning #7",   "warning #7",
                "no warning #8",   "warning #8",
                "no warning #9",   "warning #9",
                "no warning #10",  "warning #10",
                "no internl warn", "internl warning",
                "unused",          "not valid 1",
                "unused",          "not valid 1",
                "unused",          "not valid 1",
                "unused",          "not valid 1",
                "unused",          "not valid 1"
        };

    return pcBitTexts[p_uSubindex];
}
#endif
