/*********************************************************************************************************************/
/**@file        pdrv_setpointchannel_ac4.c
 * @brief       PROFIdrive Setpoint Channel
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
 *              The file implements the setpoint channel.
 *              set value with standard telegram
 *              actual value with standard telegram
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
#include "pdrv_setpointchannel_ac4.h"
#include "pdrv_statemachine_ac4.h"
#include "pdrv_application_ac4.h"
#include "pdrv_diagnostics_ac4.h"
#include "pdrv_parmanager_ac4.h"
#include "pdrv_pardatabase_ac4.h"

#if (EXAMPL_DEV_CONFIG_VERSION == 44)

/*------------  extern  functions  ------------*/

/*------------  extern  data  ------------*/

/*------------  type definitions, constants, enums  ------------*/

/*------------  static  data  ------------*/

/* data for standard telegram 1 */
static PDRV_UINT16 m_uStw1 = 0U;        /**< STW1 control word 1 */
//static PDRV_INT16  m_nNsollA = 0U;    /**< NSOLL_A speed setpoint value A [N2] */
static PDRV_UINT16 m_uZsw1 = 0U;        /**< ZSW1 status word 1 */
//static PDRV_INT16  m_nNistA = 0U;     /**< NIST_A speed actual value A [N2] */

/* additional data for standard telegram 2 */
static PDRV_UINT16 m_uStw2 = 0U;        /**< STW2 control word 2 */
static PDRV_INT32  m_nNsollB = 0UL;     /**< NSOLL_B speed setpoint value A [N4] */
static PDRV_UINT16 m_uZsw2 = 0U;        /**< ZSW2 status word 2 */
static PDRV_INT32  m_nNistB = 0UL;      /**< NIST_B speed actual value A [N4] */

/* additional data for standard telegram 3 */
static PDRV_UINT16 m_uG1Stw = 0U;       /**< G1_STW sensor 1 control word */
static PDRV_UINT16 m_uG1Zsw = 0U;       /**< G1_ZSW sensor 1 status word */
static PDRV_UINT32 m_uG1Xist1 = 0UL;    /**< G1_XIST1 sensor 1 position actual value 1 */
static PDRV_UINT32 m_uG1Xist2 = 0UL;    /**< G1_XIST2 sensor 1 position actual value 2 */

static PDRV_INT32 m_nRfgOut = 0;            /**< internal actual value of RFG/Reduced RFG [N4] */
static PDRV_BOOL m_bIsSpeedOk = PDRV_FALSE; /**< speed is within tolerance */

static PDRV_UINT m_uFirstIOData = 0U;   /**< first IO-receive is completed after power on */

static PDRV_UINT16 m_uOperatingMode = 3U;   /**< value of PNU00930 operating mode */

/*------------  public data  ------------*/

/*------------  functions  ------------*/

/** Incoming message about first valid IO data received after power on
 *  @details
 *  @return     PDVR_VOID
*/
PDRV_VOID PdrvSpc_NotifyFirstIO(PDRV_VOID)
{
    m_uFirstIOData |= FIRSTIO_EVENT; /* flag that PNIO stack get the first io data after an established connection */
}

/** Return state about first valid IO data received
 *  @details    State of received IO data after power on
 *  @return     state word of first valid received IO data
*/
PDRV_UINT uPdrvSpc_GetFirstIOState(PDRV_VOID)
{
    return m_uFirstIOData;
}

/** Function sets control word 1 STW1
 *  @details    function has to be called if new valid outputs are received
 *  @return     PDRV_VOID
*/
PDRV_VOID PdrvSpc_SetStw1
    (PDRV_UINT16 p_uStw1    /**< [in] new STW1 */
    )
{
    /* axis controlled by PLC? */
    if (p_uStw1 & PDRV_STW1_PLCCTRL)
    {
        /* Control word 1 changed? */
        if (p_uStw1 != m_uStw1)
        {
            /* Edge detection */
            /* Fault acknowledged i.e. postive edge 0->1? */
            if (    (p_uStw1 & PDRV_STW1_FAULTACK)  /* new fault acknowledge bit 1 */
                && !(m_uStw1 & PDRV_STW1_FAULTACK)  /* AND old fault acknowledge bit 0 */
               )
            {
                PdrvDiag_AckFaultSit(); /* acknowledge fault situation */
            }

            m_uStw1 = p_uStw1;
        }
    }
    else
    {
        m_uStw1 &= ~PDRV_STW1_PLCCTRL;
    }
    m_uFirstIOData |= FIRSTIO_STW1; /* flag that a STW1 is latched */
}

/** Get control word 1 STW1
 *  @details
 *  @return     value of STW1
*/
PDRV_UINT16 uPdrvSpc_GetStw1(PDRV_VOID)
{
    return m_uStw1;
}

/** Function sets speed setpoint NSOLL_A
 *  @details    function has to be called if new valid outputs are received
 *  @return     PDRV_VOID
*/
PDRV_VOID PdrvSpc_SetNsollA
    (PDRV_INT16 p_nNsollA  /**< [in] new NSOLL_A [N2] */
    )
{
    if (m_uStw1 & PDRV_STW1_PLCCTRL)
    {
        m_nNsollB = p_nNsollA * 0x10000L;   /* high word of NIST_B */;
    }
}

/** Get speed setpoint NSOLL_A
 *  @details
 *  @return     value of NSOLL_A [N2]
*/
PDRV_INT16 nPdrvSpc_GetNsollA(PDRV_VOID)
{
    return m_nNsollB / 0x10000L;   /* high word of NIST_B */
}

/** Get actual value of status word ZSW1
 *  @details    function has to be called if new inputs are transmitted
 *  @return     actual value of status word ZSW1
*/
PDRV_UINT16 uPdrvSpc_GetZsw1(PDRV_VOID)
{
    return m_uZsw1;
}

/** Set bits (ORed) in actual value of status word ZSW1
 *  @details
 *  @return     PDRV_VOID
*/
PDRV_VOID PdrvSpc_SetBitsZsw1
    (PDRV_UINT16 p_uSetBit  /**< [in] bit mask set ZSW1 bits to 1 */
    )
{
    m_uZsw1 |= p_uSetBit;
}

/** Clear bits (NANDed) in PROFIdrive actual value of status word ZSW1
 *  @details
 *  @return     PDRV_VOID
*/
PDRV_VOID PdrvSpc_ClrBitsZsw1
    (PDRV_UINT16 p_uClrBit  /**< [in] bit mask clear ZSW1 bits to 0 */
    )
{
    m_uZsw1 &= ~p_uClrBit;
}

/** Get actual value NIST_A [N2]
 *  @details    function has to be called if new inputs are transmitted
 *  @return     actual value of NIST_A [N2]
*/
PDRV_INT16 nPdrvSpc_GetNistA(PDRV_VOID)
{
    return m_nNistB / 0x10000L;   /* high word of NIST_B */
}

/** Sets actual value NIST_A [N2]
 * @details
 * @return  PDRV_VOID
*/
PDRV_VOID PdrvSpc_SetNistA
    (PDRV_INT16 p_nNistA    /**< [in] actual value of NIST_A [N2] */
    )
{
    m_nNistB = p_nNistA * 0x10000L; /* high word of NIST_B */
}

/** Function sets control word 2 STW2
 *  @details    function has to be called if new valid outputs are received
 *  @return     PDRV_VOID
*/
PDRV_VOID PdrvSpc_SetStw2
    (PDRV_UINT16 p_uStw2    /**< [in] new STW2 */
    )
{
    /* axis controlled by PLC? */
    if (m_uStw1 & PDRV_STW1_PLCCTRL)
    {
        m_uStw2 = p_uStw2;
    }
    else
    {   /* no control by PLC: Sign-Of-Life is valid nevertheless. */
        m_uStw2 = (m_uStw2 & 0x0FFFU) | (p_uStw2 & 0xF000U);
    }
}

/** Get control word 2 STW2
 *  @details
 *  @return     value of STW2
*/
PDRV_UINT16 uPdrvSpc_GetStw2(PDRV_VOID)
{
    return m_uStw2;
}

/** Function sets speed setpoint NSOLL_B [N4]
 *  @details    function has to be called if new valid outputs are received
 *  @return     PDRV_VOID
*/
PDRV_VOID PdrvSpc_SetNsollB
    (PDRV_INT32 p_nNsollB  /**< [in] new NSOLL_B [N4] */
    )
{
    if (m_uStw1 & PDRV_STW1_PLCCTRL)
    {
        m_nNsollB = p_nNsollB;
    }
}

/** Get speed setpoint NSOLL_B
 *  @details
 *  @return     value of NSOLL_B [N4]
*/
PDRV_INT32 nPdrvSpc_GetNsollB(PDRV_VOID)
{
    return m_nNsollB;
}

/** Get actual value of status word ZSW2
 *  @details    function has to be called if new inputs are transmitted
 *  @return     actual value of status word ZSW2
*/
PDRV_UINT16 uPdrvSpc_GetZsw2(PDRV_VOID)
{
    return m_uZsw2;
}

/** Set value of status word ZSW2 for selected bits which are selected with bit mask
 *  @details
 *  @return     PDRV_VOID
*/
PDRV_VOID PdrvSpc_SetZsw2
    (PDRV_UINT16 p_uZsw2,   /**< [in] new value of ZSW2 */
     PDRV_UINT16 p_uMsk     /**< [in] bit mask for bit selection */
    )
{
    PDRV_UINT16 uZsw2 = m_uZsw2 & (PDRV_UINT16)~(p_uMsk);

    m_uZsw2 = uZsw2 | (p_uZsw2 & p_uMsk);
}



/** Get actual value NIST_B [N4]
 *  @details    function has to be called if new inputs are transmitted
 *  @return     actual value of NIST_B [N4]
*/
PDRV_INT32 nPdrvSpc_GetNistB(PDRV_VOID)
{
    return m_nNistB;
}

/** Sets actual value NIST_B [N4]
 * @details
 * @return  PDRV_VOID
*/
PDRV_VOID PdrvSpc_SetNistB
    (PDRV_INT32 p_nNistB    /**< [in] actual value of NIST_B [N4] */
    )
{
    m_nNistB = p_nNistB;
}

/** Function sets G1_STW sensor 1 control word
 *  @details    function has to be called if new valid outputs are received
 *  @return     PDRV_VOID
*/
PDRV_VOID PdrvSpc_SetG1Stw
    (PDRV_UINT16 p_uG1Stw    /**< [in] new G1_STW */
    )
{
    /* axis controlled by PLC? */
    if (m_uStw1 & PDRV_STW1_PLCCTRL)
    {
        m_uG1Stw = p_uG1Stw;
    }
}

/** Get actual value of G1_STW sensor 1 control word
 *  @details
 *  @return     actual value of G1_STW sensor 1 control word
*/
PDRV_UINT16 uPdrvSpc_GetG1Stw(PDRV_VOID)
{
    return m_uG1Stw;
}

/** Get actual value of G1_ZSW sensor 1 status word
 *  @details    function has to be called if new inputs are transmitted
 *  @return     actual value of G1_ZSW sensor 1 status word
*/
PDRV_UINT16 uPdrvSpc_GetG1Zsw(PDRV_VOID)
{
    return m_uG1Zsw;
}

/** Function sets G1_ZSW sensor 1 status word
 *  @details
 *  @return     PDRV_VOID
*/
PDRV_VOID PdrvSpc_SetG1Zsw
    (PDRV_UINT16 p_uG1Zsw    /**< [in] new G1_ZSW */
    )
{
    m_uG1Zsw = p_uG1Zsw;
}

/** Get actual value of G1_XIST1 sensor 1 position actual value 1
 *  @details    function has to be called if new inputs are transmitted
 *  @return     actual value of G1_XIST1 sensor 1 position actual value 1
*/
PDRV_UINT32 uPdrvSpc_GetG1Xist1(PDRV_VOID)
{
    return m_uG1Xist1;
}

/** Sets actual value G1_XIST1 [N4]
 * @details
 * @return  PDRV_VOID
*/
PDRV_VOID PdrvSpc_SetG1Xist1
    (PDRV_INT32 p_nG1Xist1    /**< [in] actual value of G1_XIST1 [N4] */
    )
{
    m_uG1Xist1 = p_nG1Xist1;
}

/** Get actual value of G1_XIST2 sensor 1 position actual value 2
 *  @details    function has to be called if new inputs are transmitted
 *  @return     actual value of G1_XIST2 sensor 1 position actual value 2
*/
PDRV_UINT32 uPdrvSpc_GetG1Xist2(PDRV_VOID)
{
    return m_uG1Xist2;
}

/** Sets actual value G1_XIST2 [N4]
 * @details
 * @return  PDRV_VOID
*/
PDRV_VOID PdrvSpc_SetG1Xist2
    (PDRV_INT32 p_nG1Xist2    /**< [in] actual value of G1_XIST2 [N4] */
    )
{
    m_uG1Xist2 = p_nG1Xist2;
}

/** Ramp Function Generator (RFG)
 * @details This function implements the RFG (PDRV V4.2 Figure 29 without Jogging functionality)
 *          Should be called if it is parameterized with PNU930 = 1.
 * @return  actual output of RFG [N4]
*/
PDRV_INT32 nPdrvSpc_CalcRfg
    (PDRV_INT32 p_nSetPnt,  /**< [in] actual setpoint for the RFG [N4] */
     PDRV_INT32 p_nRfgStp   /**< [in] RFG ramp up/down step [N4] */
    )
{
    PDRV_UINT16 uStw1 = uPdrvSpc_GetStw1(); /* local copy of control word 1 STW1 */
    PDRV_INT32 nRfgOut = 0;                 /* new output of the Ramp Function Generator */
    PDRV_INT32 nSetPnt = 0;                 /* actual setpoint of the Ramp Function Generator */
    PDRV_INT32 nRfgStp = 0;                 /* absolute value of RFG step */

    /* Is RFG enabled? */
    if (uStw1 & PDRV_STW1_RFGENABLE)
    {
        nRfgOut = m_nRfgOut;

        /* Is RFG not freezed? */
        if (uStw1 & PDRV_STW1_RFGUNFREZ)
        {
            /* Do RFG follows setpoint? */
            if (uStw1 & PDRV_STW1_RFGSETPNT)
            {
                nSetPnt = p_nSetPnt;
            }

            nRfgStp = nPdrvAbs(p_nRfgStp);   /* absolute value of RfgStp */

            /* Is actual RfgOut smaller than actual setpoint? */
            if (nRfgOut < nSetPnt)
            {   /* ramp up */
                if ((PDRV_UINT32)nRfgStp < (PDRV_UINT32)(nSetPnt - nRfgOut))
                {
                    nRfgOut += nRfgStp; /* step up */
                }
                else
                {
                    nRfgOut = nSetPnt;  /* limit output to setpoint */
                }
            }    /* Is actual RfgOut higher than actual setpoint? */
            else if (nRfgOut > nSetPnt)
            {   /* ramp down */
                if ((PDRV_UINT32)nRfgStp < (PDRV_UINT32)(nRfgOut - nSetPnt))
                {
                    nRfgOut -= nRfgStp; /* step down */
                }
                else
                {
                    nRfgOut = nSetPnt;  /* limit output to setpoint */
                }
            }
        }
    }
    m_nRfgOut = nRfgOut;
    return m_nRfgOut;
}

/** reduced Speed Setpoint channel for application class 4
 * @details This function implements the reduced Speed Setpoint Channel (PDRV V4.2 Figure 31 without Jogging functionality)
 *          Should be called if it is parameterized with PNU930 = 3.
 * @return  actual output of RFG [N4]
*/
PDRV_INT32 nPdrvSpc_CalcReducedSsc
    (PDRV_INT32 p_nSetPnt,  /**< [in] actual setpoint for the RFG [N4] */
     PDRV_INT32 p_nRfgStp   /**< [in] RFG ramp down step [N4] */
    )
{
    PDRV_UINT16 uStw1 = uPdrvSpc_GetStw1(); /* local copy of control word 1 STW1 */
    PDRV_INT32 nRfgOut = 0;                 /* new output of the Ramp Function Generator */
    PDRV_INT32 nRfgStp = 0;                 /* absolute value of RFG step */

    /* Is RFG enabled? */
    if (uStw1 & PDRV_STW1_RFGENABLE)
    {
        /* Do RFG follows setpoint? */
        if (uStw1 & PDRV_STW1_RFGSETPNT)
        {
            nRfgOut = p_nSetPnt;
        }
        else
        {
            nRfgStp = nPdrvAbs(p_nRfgStp);   /* absolute value of RfgStp */
            nRfgOut = m_nRfgOut;

            /* Is actual RfgOut negative? */
            if (nRfgOut < 0L)
            {   /* ramp up */
                if (nRfgOut < -nRfgStp)
                {
                    nRfgOut += nRfgStp; /* step up */
                }
                else
                {
                    nRfgOut = 0L;       /* set to zero */
                }
            }    /* Is actual RfgOut positive? */
            else if (nRfgOut > 0L)
            {   /* ramp down */
                if (nRfgStp < nRfgOut)
                {
                    nRfgOut -= nRfgStp; /* step down */
                }
                else
                {
                    nRfgOut = 0L;       /* set to zero */
                }
            }
        }
    }
    m_nRfgOut = nRfgOut;
    return m_nRfgOut;
}

/** Speed Setpoint Control (SSC)
 * @details This function implements the Speed Setpoint Control (PDRV V4.2 Figure 139).<br>
 *          It calculates the setpoint depending of general state.<br>
 *          In normal operation: the RFG speed is outputted.<br>
 *          During ramp stop: a down ramp with ramp down speed is calculated and outputted.<br>
 *          During quick stop: a down ramp with quick stop speed is calculated and outputted.
 * @return  actual speed setpoint [N4]
*/
PDRV_INT32 nPdrvSpc_CalcSsc
    (PDRV_INT32 p_nSpeedRfg,    /**< [in] RFG speed output [N4] */
     PDRV_INT32 p_nRampStp,     /**< [in] ramp  stop down step [N4] */
     PDRV_INT32 p_nQuickStp     /**< [in] quick stop down step [N4] */
    )
{
    static PDRV_BOOL bIsRampDown = PDRV_FALSE;                  /* SSC is in ramp down */
    PDRV_AXISSTATES_TYPE eAxisState = ePdrvSma_GetAxisMainState(); /* actual axis state */
    PDRV_BOOL bIsRfgEnabled = PDRV_FALSE;                       /* actual RFG enable flag */
    static PDRV_INT32 nSpeedO = 0;  /* old speed setpoint */
    PDRV_INT32 nSpeed = 0;          /* new speed setpoint */
    PDRV_INT32 nStp = 0;            /* ramp speed step */

    bIsRfgEnabled = (uPdrvSpc_GetStw1() & PDRV_STW1_RFGENABLE) ? PDRV_TRUE : PDRV_FALSE;

    /* Is ramp down needed?*/
    if (   (eAxisState == AXISSTATE_QUICK_STOP)
        || (eAxisState == AXISSTATE_RAMP_STOP)
        || (   (eAxisState == AXISSTATE_OPERATION)
            && (bIsRfgEnabled != PDRV_TRUE)
           )
       )
    {   /* ramp down mode */
        if (bIsRampDown != PDRV_FALSE)
        {   /* continue ramp down */
            nSpeed = nSpeedO;
        }
        else
        {   /* ramp down starts with actual speed */
            bIsRampDown = PDRV_TRUE;
            nSpeed = nPdrvSpc_GetNistB();
        }

        if (   (eAxisState == AXISSTATE_RAMP_STOP)  /* Ramp Stop */
            && (bIsRfgEnabled == PDRV_TRUE)         /* AND RFG enabled */
           )
        {   /* ramp down with normal ramp down speed */
            nStp = p_nRampStp;
        }
        else
        {   /* ramp down with maximum deceleration, see PDRV V4.2 table 71 quick stop and table 72 RFG reset/disable */
            nStp = p_nQuickStp;
        }
    }
    else
    {   /* SSC in normal operation */
        bIsRampDown = PDRV_FALSE;
        if (eAxisState == AXISSTATE_OPERATION)
        {   /* normal operation with RFG */
            nSpeed = p_nSpeedRfg;
        }
    }

    /* Ramp needed? */
    if (nStp != 0)
    {
        nStp = nPdrvAbs(nStp);   /* absolute step value */

        /* Negative speed? */
        if (nSpeed < 0)
        {   /* ramp up */
            nSpeed += nStp;
            if (nSpeed > 0) /* standstill? */
            {
                nSpeed = 0;
            }
        }
        else
        {   /* Postive speed: ramp down */
            nSpeed -= nStp;
            if (nSpeed < 0)  /* standstill? */
            {
                nSpeed = 0;
            }
        }
    }
    nSpeedO = nSpeed;
    return nSpeed;
}

/** Calculates if the speed error within tolerance range
 * @details This function implements the speed comparator (PDRV V4.2 Figure 29).
 *          It sets ZSW1 bit8 according of result.
 *          Function should be called once time every 1 ms because of implementation time measurement with uTime.
 * @return  TRUE if actual value is within tolerance range
*/
PDRV_BOOL bPdrvSpc_CalcSpeedWithinTolerance
    (PDRV_INT32 p_nNistB,       /**< [in] actual value [N4] */
     PDRV_INT32 p_nNsollB,      /**< [in] setpoint [N4] */
     PDRV_INT32 p_nNTolerance,  /**< [in] +- tolerance of actual value [N4] */
     PDRV_UINT16 p_uTmax        /**< [in] time to get into tolerance range [number of calls] */
    )
{
    static PDRV_UINT16 uTime = 0U;      /* counts number of calls if speed is out of range */
    PDRV_BOOL bSpeedOk;                 /* return value if speed error within tolerance range */
    PDRV_INT32 nNistB = p_nNistB;
    PDRV_INT32 nNsollB = p_nNsollB;
    PDRV_INT32 nNTolerance = 0;         /* absolute value of p_nNTolerance */
    PDRV_INT32 nLoLim;                  /* low limit of tolerance range */
    PDRV_INT32 nHiLim;                  /* high limit of tolerance range */

    nNTolerance = nPdrvAbs(p_nNTolerance);
    nLoLim = (nNsollB - nNTolerance) > nNsollB? 0x80000000L : (nNsollB - nNTolerance);
    nHiLim = (nNsollB + nNTolerance) < nNsollB? 0x7FFFFFFFL : (nNsollB + nNTolerance);

    /* speed error within tolerance range? */
    if (   (nLoLim <= nNistB)
        && (nNistB <= nHiLim)
       )
    {
        uTime = 0;  /* reset time */
    }
    else
    {
        uTime = (uTime < p_uTmax)? uTime + 1 : p_uTmax; /* time counts up (with limit) */
    }

    /* time not expired? */
    if (   (ePdrvSma_GetAxisMainState() > AXISSTATE_POWERON)
        && (uTime < p_uTmax)
       )
    { /* time not expired yet i.e. speed error is within tolerance range */
        bSpeedOk = PDRV_TRUE;
        PdrvSpc_SetBitsZsw1(PDRV_ZSW1_NOSPDERR);
    }
    else
    { /* time expired i.e. speed error is out of tolerance range */
        bSpeedOk = PDRV_FALSE;
        PdrvSpc_ClrBitsZsw1(PDRV_ZSW1_NOSPDERR);
    }
    m_bIsSpeedOk = bSpeedOk;
    return bSpeedOk;
}

/** Calculates if the actual speed has reached or exceeded a comparison value
 * @details ZSW1 bit 10 is set according NSOLL_A. (PDRV V4.2 figure 29, table 78)
 * @return  PDRV_TRUE if comparison value is reached or exceeded
*/
PDRV_BOOL bPdrvSpc_CalcSpeedReached
    (PDRV_INT32 p_nNistB,   /**< [in] actual value of NIST_B [N4] */
     PDRV_INT32 p_nNcomp    /**< [in] comparison value [N4] */
    )
{
    PDRV_INT32 nNistB = nPdrvAbs(p_nNistB); /* absolute NIST_B [N4] */
    PDRV_INT32 nNcomp = nPdrvAbs(p_nNcomp); /* absolute p_nNcomp [N4] */
    PDRV_BOOL bIsReached;

    /* actual value has reached setpoint? (PDRV V4.2 figure 29) */
    if (nNistB >= nNcomp)
    {
        PdrvSpc_SetBitsZsw1(PDRV_ZSW1_FNREACHED);
        bIsReached = PDRV_TRUE;
    }
    else
    {
        PdrvSpc_ClrBitsZsw1(PDRV_ZSW1_FNREACHED);
        bIsReached = PDRV_FALSE;
    }
    return bIsReached;
}

/** Returns if the speed error within tolerance range (PDRV V4.2 Figure 29).
 * @details
 * @return  TRUE if actual value is within tolerance range
*/
PDRV_BOOL bPDRVSpc_IsSpeedWithinTolerance(PDRV_VOID)
{
    return m_bIsSpeedOk;
}


/** Get the actual operating mode which is selected in PNU00930 "Operating mode"
 *  @details
 *  @return     PROFIdrive operating mode
*/
PDRV_UINT16 uPdrvSpc_GetOperatingMode(PDRV_VOID)
{
    return m_uOperatingMode;
}

/*-------------  PROFIdrive parameter manager assigned text functions, read functions, write functions  --------------*/

/** PROFIdrive read function for parameter PNU00930 "Operating mode"
 *  @details    none
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00930
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->o2[0U] = m_uOperatingMode;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00930 "Operating mode"
 *  @details    Parameter change is possible during AXISSTATE_SWITCHING_ON_INHIBITED only
 *              Only parameter value 1 and 3 are allowed (no position mode).
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00930
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT32 uError = PDRV_EV1_NOERROR;

    /* operating mode changed? */
    if (m_uOperatingMode != p_ptValues->o2[0])
    {
        /* Is operating mode 1 or 3 chosen? (Operating modes 1 and 3 are implemented.) */
        if (   (p_ptValues->o2[0] == PDRV_CLCOPMODE_RFG)
            || (p_ptValues->o2[0] == PDRV_CLCOPMODE_REDSSC)
           )
        {
            /* Proper main state? Change of operating mode is reasonable during stillstand only. */
            if (AXISSTATE_READY_FOR_SWITCHING_ON > ePdrvSma_GetAxisMainState())
            {
                m_uOperatingMode = p_ptValues->o2[0U];
            }
            else
            {
                uError = PDRV_EV1_OP_STATE;
            }
        }
        else
        {
            uError = PDRV_EV1_VALUE_IMPERMISS;
        }
    }
    return uError;
}

#endif
