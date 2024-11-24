/*********************************************************************************************************************/
/**@file        pdrv_application_ac4.c
 * @brief       PROFIdrive application itself
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
 *              The application consists of correct calling sequence, some call back functions,
 *              a simple motor simulation and lot of parameters for testing purposes.
 *              The PDRV_ApplTimer() is called cyclic every 1 ms.
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
#include "pniousrd.h"   /* for PNIO_printf needed */

#include "pdrv_types_ac4.h"  /* PROFIdrive datatype definitions */
#include "pdrv_setpointchannel_ac4.h"
#include "pdrv_statemachine_ac4.h"
#include "pdrv_application_ac4.h"
#include "pdrv_diagnostics_ac4.h"
#include "pdrv_parmanager_ac4.h"
#include "pdrv_pardatabase_ac4.h"
#include "pdrv_synchronisation_ac4.h"
#include "pdrv_sensor_ac4.h"

#if (EXAMPL_DEV_CONFIG_VERSION == 44)

/*------------  extern  functions  ------------*/
extern PDRV_UINT16 uPdrvUsr_GetTelegramNo(PDRV_VOID);

/*------------  extern  data  ------------*/

/*------------  static  data  ------------*/
static PDRV_UINT32 m_uTimerCnt1ms = 0U;         /**< counts call of PDRV_ApplTimerIsr() [ms], overflow ca. 49,7t */
static PDRV_UINT32 m_uTimerValNewState = 0U;    /**< timer value of last main state change */

static PDRV_INT32 m_nRampGradient = 0x00100000; /**< gradient of ramp in [%/ms] for the Ramp Function Generator  */
static PDRV_INT32 m_nNTolerance = 0x01000000;   /**< tolerance for calculation for speed error in tolerance range ZSW1 bit 8 [N4] */
static PDRV_UINT16 m_uTmax = 100U;              /**< allowed time getting in tolerance range ZSW1 bit 8 [ms] */
static PDRV_INT32 m_nRampDown[2] = {0x150000, 0x150000};    /**< ramps in [%/ms] during [0]=ramp stop, [1]=quick stop */
static PDRV_INT16 m_nPT1GainFactor[2] = {0x0001, 0x0008};  /**< gain factor of pt1 during [0]=operation , [1]=coast [N4] */
static PDRV_INT32 m_nCompVal = 0x4000000;      /**< comparison value to caclulate ZSW Bit 10 [N4] */

/** @name test parameter variables for Profile Tester
 * : SHOULD BE REMOVED BY PDRV USER
 * @{
*/
static PDRV_UINT16 m_uValPnu700[2] = {0U, 0U};  /**< value of PNU00700: [0] = fault number, [1] = fault value */
static PDRV_UINT16 m_uFaultVal[FAULT_USER9] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U}; /**< fault value of permanent fault of PNU00700 */
static PDRV_UINT16 m_uValPnu701 = 0U;           /**< value of PNU00701: fault number */
static PDRV_UINT16 m_uValPnu702[2] = {0U, 0U};  /**< value of PNU00702: [0] = warning number, [1] = warning value */
static PDRV_BOOL m_uValPnu800 = 0;              /**< value of PNU00801 */
static PDRV_INT8 m_nValPnu801 = -127;           /**< value of PNU00801 */
static PDRV_INT16 m_nValPnu802 = -32767;        /**< value of PNU00802 */
static PDRV_INT32 m_nValPnu803 = -0x7FFFFFFFL;  /**< value of PNU00803 */
static PDRV_UINT8 m_uValPnu804 = 0x080U;        /**< value of PNU00804 */
static PDRV_UINT16 m_uValPnu805 = 0x8FFFU;      /**< value of PNU00805 */
static PDRV_UINT32 m_uValPnu806 = 0x0FFUL;      /**< value of PNU00806 */
static PDRV_FLT32 m_fValPnu807 = FLOAT_3000;    /**< value of PNU00807 */
static PDRV_UINT16 m_uValPnu808 = 0x05A;        /**< value of PNU00808 */
static PDRV_BOOL m_bValPnu809[242];             /**< values of PNU00809 */
static PDRV_INT8 m_nValPnu810[9];               /**< values of PNU00810 */
static PDRV_INT16 m_nValPnu811[9];              /**< values of PNU00811 */
static PDRV_INT32 m_nValPnu812[9];              /**< values of PNU00812 */
static PDRV_UINT8 m_uValPnu813[9];              /**< values of PNU00813 */
static PDRV_UINT16 m_uValPnu814[9];             /**< values of PNU00814 */
static PDRV_UINT32 m_uValPnu815[9];             /**< values of PNU00815 */
static PDRV_FLT32 m_fValPnu816[9];              /**< values of PNU00816 */
static PDRV_UINT32 m_uValPnu817[9];             /**< values of PNU00817 */
static PDRV_INT16 m_nValPnu818[9];              /**< values of PNU00818 */
static PDRV_INT32 m_nValPnu819[9];              /**< values of PNU00819 */
/** @} */

#define PRINTF_COUNT (3000U)                        /**< time for cyclical printf [ms] */
static PDRV_UINT m_uPrintfCntDwn = PRINTF_COUNT;    /**< counter for cyclical printf */

static PDRV_VOID AppPrintDriveStatus(PDRV_INT32 p_nNsollB, PDRV_INT32 p_nNrfg, PDRV_INT32 p_nNctrl, PDRV_INT32 p_nNistB, PDRV_BOOL p_bIsSpeedOk, PDRV_AXISSTATES_TYPE p_eAxisMainState);
static PDRV_VOID AppPrintSensorStatus(PDRV_INT32 p_nXist1, PDRV_INT32 p_nXist2);
static PDRV_VOID AppPrintSignOfLifeStatus(PDRV_VOID);

/*------------  public data  ------------*/

/** PROFIdrive application
 *  @details    This function will be called every 1ms by the OS (see MainAppl())
 *              Adapt this function to specific application needs
 *  @return     void
*/
void PdrvApp_main(void)
{
    PDRV_INT32 nNsollB; /* setpoint speed */
    PDRV_INT32 nNrfg;   /* speed after Ramp Function Generator */
    PDRV_INT32 nNctrl;  /* speed after Speed Setpoint Control */
    PDRV_INT32 nNistB;  /* drive speed */
    PDRV_AXISSTATES_TYPE eAxisMainState;    /* state of the general state machine */
    PDRV_BOOL bIsSpeedOk;   /* flag if speed is within tolerance */
    PDRV_INT32 nXist1;  /* position */
    PDRV_INT32 nXist2;  /* G1_XIST2 */

    /* cyclic processing steps */
    m_uTimerCnt1ms++;                                                                /* count calls i.e. measure time */

    PdrvSyn_CheckSignOfLife();                                                        /* Sign of Life synchronisation */

    eAxisMainState = ePdrvSma_AxisGeneralStateMachine();                         /* process the general state machine */
    nNsollB = nPdrvSpc_GetNsollB();                                                        /* get the actual setpoint */
    if (uPdrvSpc_GetOperatingMode() == PDRV_CLCOPMODE_REDSSC)
    {
        nNrfg = nPdrvSpc_CalcReducedSsc(nNsollB, m_nRampDown[0]);     /* calculate the reduced speed setpoint channel */
    }
    else
    {
        nNrfg = nPdrvSpc_CalcRfg(nNsollB, m_nRampGradient);                  /* calculate the ramp function generator */
    }
    bIsSpeedOk = bPdrvSpc_CalcSpeedWithinTolerance(nPdrvSpc_GetNistB(), nNrfg, m_nNTolerance, m_uTmax); /* speed test */
    nNctrl = nPdrvSpc_CalcSsc(nNrfg, m_nRampDown[0], m_nRampDown[1]);               /* setpoint or special ramp downs */
    nNistB = nPdrvApp_CalcPT1(nNctrl);                                                     /* control loop simulation */
    PdrvSpc_SetNistB(nNistB);                                              /* set new actual speed of simulated drive */
    bPdrvSpc_CalcSpeedReached(nNistB, m_nCompVal);                                            /* calculate ZSW Bit 10 */

    nXist1 = nPdrvSen_Sensor(nNistB);                                                       /* calculate new position */
    PdrvSpc_SetG1Xist1(nXist1);                                                            /* set new actual position */
    PdrvSen_SensorStateMachine();
    nXist2 = nPdrvSen_GetGxXist2();
    PdrvSpc_SetG1Xist2(nXist2);                                                                       /* set G1_XIST2 */

    AppPrintDriveStatus(nNsollB, nNrfg, nNctrl, nNistB, bIsSpeedOk, eAxisMainState);         /* some terminal outputs */
    AppPrintSensorStatus(nXist1, nXist2);
    AppPrintSignOfLifeStatus();

    /* realize permanent faults of PNU00700 for Profile Tester test cases */
    {
        PDRV_UINT uI;

        for (uI = FAULT_USER1; uI < FAULT_USER9; uI++)
        {
            if (m_uFaultVal[uI] != 0U)
            {
                PdrvDiag_SetFaultMsg(uI, m_uFaultVal[uI]);
            }
        }
    }
}

/** True, if drive standstill is detected
 *  @details    Simple implementation. Should be adapted in real application.
 *              Function is mandatory because of use in general state machine.
 *  @return     TRUE = Axis standstill is detected
*/
PDRV_BOOL bPdrvApp_IsAxisStandstill(PDRV_VOID)
{
    PDRV_UINT16 uTelegramNo = uPdrvUsr_GetTelegramNo();
    PDRV_BOOL bIsAxisStandstill = PDRV_FALSE;
    
    if (   (   (uTelegramNo == PDRV_STDTLG1)
            && (nPdrvSpc_GetNistA() == 0)
           )
        || (   (uTelegramNo != PDRV_STDTLG1)
            && (nPdrvSpc_GetNistB() == 0L)
           )
       )
    {
        bIsAxisStandstill = PDRV_TRUE;
    }

    return bIsAxisStandstill;
}

/** True, if transition condition for next required state is fulfilled
 *  @details    PDRV user should be change the functionality and conditions to his own application!
 *              Function is mandatory because of use in general state machine.
 *  @return     PDRV_TRUE = transition condition is fulfilled
*/
PDRV_BOOL bPdrvApp_IsTransitionCondition
    (PDRV_AXISSTATES_TYPE p_eActAxisMainState, /**< [in] actual state */
     PDRV_AXISSTATES_TYPE p_eReqAxisMainState  /**< [in] required state */
    )
{
    PDRV_BOOL bIsTransitionCondition = PDRV_FALSE;

    switch (p_eActAxisMainState)
    {
    case AXISSTATE_SWITCHING_ON_INHIBITED:   /* general state S1: Switching On Inhibited*/
    {
        if (p_eReqAxisMainState == AXISSTATE_READY_FOR_SWITCHING_ON)
        {
            /* Insert your application specific tests here: e.g. main contacts should be open */
            if (30U < (m_uTimerCnt1ms - m_uTimerValNewState))  /* example implementation of a simple 30ms delay */
            {
                bIsTransitionCondition = PDRV_TRUE;
            }
        }
        break;
    }
    case AXISSTATE_READY_FOR_SWITCHING_ON:   /* general state S2: Ready For Switching On */
    {
        if (p_eReqAxisMainState == AXISSTATE_SWITCHED_ON)
        {
            /* Insert your application specific tests here: e.g. main contacts are closed, infeed is ok, DC link voltage is ok */
            if (20U < (m_uTimerCnt1ms - m_uTimerValNewState))  /* example implementation of a simple 20ms delay */
            {
                bIsTransitionCondition = PDRV_TRUE;
            }
        }
        break;
    }
    case AXISSTATE_SWITCHED_ON:              /* general state S3: Switched On */
    {
        if (p_eReqAxisMainState == AXISSTATE_OPERATION)
        {
            /* Insert your application specific tests here: e.g. de-energizing delay because of motor remanence */
            if (10U < (m_uTimerCnt1ms - m_uTimerValNewState))  /* example implementation of a simple 10ms delay */
            {
                bIsTransitionCondition = PDRV_TRUE;
            }
        }
        break;
    }
    default:
        break;
    }

    return bIsTransitionCondition;
}

/** Returns the timer value with 1 s resolution
 *  @details    Function is mandatory because of use in PROFIdrive fault buffer.
 *              Timer value is calculated from 1ms timer. Should be adapted to a real 1s timer.
 *              Timer overflows in ca. 49,7 days because of 1ms basis.
 *  @return     timer value [s]
*/
PDRV_UINT32 uPdrvApp_GetTimer1s(PDRV_VOID)
{
    return m_uTimerCnt1ms/1000U;
}

/** Returns the timer value with 1 ms resolution
 *  @details    Function is mandatory for AC4 because of use in sensor state machine.
 *              Timer value is calculated from 1ms timer. Should be adapted to a real 1ms timer.
 *              Timer overflows in ca. 49,7 days.
 *  @return     timer value [ms]
*/
PDRV_UINT32 uPdrvApp_GetTimer1ms(PDRV_VOID)
{
    return m_uTimerCnt1ms;
}


/** PT1 element for simple simulation of control loop (controller + inverter + motor + encoder)
 *  @details    implementation of PT1
 *  @return     output of PT1 element [N4]
*/
PDRV_INT32 nPdrvApp_CalcPT1
    (PDRV_INT32 p_nX    /**< [in] input signal [N4] */
    )
{
    static PDRV_INT32 nY = 0;
    PDRV_INT32 nDiff;
    PDRV_INT32 nGain;

    nDiff = p_nX - nY;
    if (nDiff != 0)
    {
        PDRV_AXISSTATES_TYPE eAxisMainState = ePdrvSma_GetAxisMainState();
        PDRV_UINT uI = eAxisMainState > AXISSTATE_SWITCHED_ON ? 0U : 1U;

        nGain = m_nPT1GainFactor[uI];
        if (   (nDiff < -nGain)
            || (nGain < nDiff)
           )
        {
            nDiff /= nGain;
        }
        else
        {
            nDiff = (nDiff < 0) ? -1 : 1;
        }
        nY += nDiff;

        /* friction loss simulation for coast stop */
        if (   (eAxisMainState < AXISSTATE_SWITCHED_ON)
            && (nY != 0)
           )
        {
            PDRV_INT32 nFricLoss = 0x00000400L; /* friction loss */
            nDiff = nY;

            if (   (nDiff > 0)
                && (nDiff > nFricLoss)
               )
            {
                nDiff = nFricLoss;
            }
            else if (   (nDiff < 0)
                     && (nDiff < nFricLoss)
                    )
            {
                nDiff = -nFricLoss;
            }
            nY -= nDiff;
        }

    }
    return nY;
}

/** prints datas of drive status
 *  @details
 *  @return     none
*/
static PDRV_VOID AppPrintDriveStatus
    (PDRV_INT32 p_nNsollB,                  /**< [in] speed setpoint */
     PDRV_INT32 p_nNrfg,                    /**< [in] speed output of Ramp Function Generator */
     PDRV_INT32 p_nNctrl,                   /**< [in] speed output of Speed Setpoint Control */
     PDRV_INT32 p_nNistB,                   /**< [in] actual drive speed */
     PDRV_BOOL p_bIsSpeedOk,                /**< [in] speed within tolerance */
     PDRV_AXISSTATES_TYPE p_eAxisMainState  /**< [in] state of general statemachine */
    )
{
    const char * cAxisMainStateText[AXISSTATE_STATEMACHINE_ERROR+1] =
    {
        [AXISSTATE_POWERON]                = "S0 PowerOn",    /* state after power on */
        [AXISSTATE_IDLE]                   = "S0 Idle",       /* state with no control by PLC */
        [AXISSTATE_SWITCHING_ON_INHIBITED] = "S1 Inhibited",  /* general state S1: Switching On Inhibited*/
        [AXISSTATE_READY_FOR_SWITCHING_ON] = "S2 Ready",      /* general state S2: Ready For Switching On */
        [AXISSTATE_SWITCHED_ON] =            "S3 SwitchOn",   /* general state S3: Switched On */
        [AXISSTATE_OPERATION] =              "S4 Operation",  /* general state S4: Operation */
        [AXISSTATE_RAMP_STOP] =              "S5 RampStop",   /* general state S5A: Switching Off with Ramp Stop */
        [AXISSTATE_QUICK_STOP] =             "S5 QuickStop",  /* general state S5B: Switching Off with Quick Stop */
        [AXISSTATE_STATEMACHINE_ERROR] =     "SF ERROR!"      /* Error of the General State Machine */
    };
    static PDRV_INT32 nNsollO = 0;                  /* old value of nNsollA */
    static PDRV_AXISSTATES_TYPE eAxisMainStateO;    /* old value of MainState */
    static PDRV_BOOL bIsSpeedOkO = PDRV_FALSE;      /* old value of SpeedOk */
    static PDRV_BOOL bIsStandStillO = PDRV_TRUE;    /* old value of StandStill */
    PDRV_BOOL bIsStandStill = bPdrvApp_IsAxisStandstill();
    static PDRV_UINT16 uStwO = 0U;
    PDRV_UINT16 uStw1 = uPdrvSpc_GetStw1();
    static PDRV_UINT16 uZswO = 0U;
    PDRV_UINT16 uZsw1 = uPdrvSpc_GetZsw1();

    /* Is something changed OR time expired? */
    if (   (nNsollO != p_nNsollB)
        || (eAxisMainStateO != p_eAxisMainState)
        || (bIsSpeedOkO != p_bIsSpeedOk)
        || (bIsStandStillO != bIsStandStill)
        || (uStwO != uStw1)
        || (uZswO != uZsw1)
        || (   (bIsStandStill == PDRV_FALSE)
            && (m_uPrintfCntDwn == 0U)
           )
       )
    {
        /* standard telegram 1? */
        if (uPdrvUsr_GetTelegramNo() == PDRV_STDTLG1)
        {
            PNIO_printf ((PNIO_CHAR*) "%12d\tNs:%04X Nr:%04X Nc:%04X Na:%04X St:%04X Zs:%04X\t",
                         m_uTimerCnt1ms, (p_nNsollB >> 16U) & 0xFFFFU, (p_nNrfg >> 16U) & 0xFFFFU,
                         (p_nNctrl >> 16U) & 0xFFFFU, (p_nNistB >> 16U) & 0xFFFFU, uStw1,  uZsw1);
        }
        else
        {
            PNIO_printf ((PNIO_CHAR*) "%12d\tNs:%08X Nr:%08X Nc:%08X Na:%08X St:%04X Zs:%04X\t",
                         m_uTimerCnt1ms, p_nNsollB, p_nNrfg, p_nNctrl, p_nNistB, uStw1,  uZsw1);
        }
        if (eAxisMainStateO != p_eAxisMainState)
        {
            PNIO_printf ((PNIO_CHAR*) "%s. ", cAxisMainStateText[p_eAxisMainState]);
            m_uTimerValNewState = m_uTimerCnt1ms;
            eAxisMainStateO = p_eAxisMainState;
        }
        if (nNsollO != p_nNsollB)
        {
            PNIO_printf ((PNIO_CHAR*) "New Setpoint. ");
            nNsollO = p_nNsollB;
        }
        if (bIsSpeedOkO != p_bIsSpeedOk)
        {
            PNIO_printf ((PNIO_CHAR*) "Speed %sok. ", p_bIsSpeedOk ? "" : "not ");
            bIsSpeedOkO = p_bIsSpeedOk;
        }
        if (bIsStandStillO != bIsStandStill)
        {
            PNIO_printf ((PNIO_CHAR*) "%s. ",  bIsStandStill ? "Standstill" : "DriveStart");
            bIsStandStillO = bIsStandStill;
        }
        PNIO_printf ((PNIO_CHAR*) "\n");
        uStwO = uStw1;
        uZswO = uZsw1;
        m_uPrintfCntDwn = PRINTF_COUNT;
    }
    if (m_uPrintfCntDwn)
    {
        m_uPrintfCntDwn--;
    }
}


/** prints datas of sensor status
 *  @details
 *  @return     none
*/
static PDRV_VOID AppPrintSensorStatus
    (PDRV_INT32 p_nXist1,   /**< [in] G1_XIST1 */
     PDRV_INT32 p_nXist2    /**< [in] G1_XIST2 */
    )
{
    const char * cSensorStateText[SENSORSTATE_STATEMACHINE_ERROR+1] =
    {
        [SENSORSTATE_IDLE]                  = "SD0 Idle",           /* state after power on */
        [SENSORSTATE_NORMAL_OPERATION]      = "SD1 Normal",         /* state SD1  normal operation */
        [SENSORSTATE_ERROR_ACKNOWLEDGEMENT] = "SD2 ErrAck",         /* state SD2  error acknowledgement */
        [SENSORSTATE_ERROR]                 = "SD3 Err",            /* state SD3  error i.e. error code in Gx_XIST2 */
        [SENSORSTATE_REF_VALUE]             = "SD4 RefVal",         /* state SD4  reference value in Gx_XIST2 */
        [SENSORSTATE_WAIT_REF_MARK]         = "SD5 RefMark",        /* state SD5  wait for reference mark */
        [SENSORSTATE_SD6_UNUSED]            = "SD6 unused",         /* state SD6  is unused */
        [SENSORSTATE_SET_HOMEPOS]           = "SD7 Home",           /* state SD7  set/shift home position */
        [SENSORSTATE_SD8_UNUSED]            = "SD8 unused",         /* state SD8  is unused */
        [SENSORSTATE_SD9_UNUSED]            = "SD9 unused",         /* state SD9  is unused */
        [SENSORSTATE_WAIT_MEAS_VAL]         = "SD10 MeasVal",       /* state SD10 wait for measured values */
        [SENSORSTATE_RDY_MEAS_VAL]          = "SD11 MeasVal",       /* state SD11 measured value in Gx_XIST2 */
        [SENSORSTATE_PARKING]               = "SD12 Parking",       /* state SD12 parking */
        [SENSORSTATE_PARKING_ERROR]         = "SD13 ParkErr",       /* state SD13 parking and error (optional) */
        [SENSORSTATE_PARKING_ERROR_ACK]     = "SD14 ParkErrAck",    /* state SD14 parking and error acknowledgement (optional) */
        [SENSORSTATE_STATEMACHINE_ERROR]    = "SEN SM ERROR!"       /* Error of the Sensor State Machine */
    };
    static PDRV_SENSORSTATES_TYPE eSensorStateO;    /* old value of Sensor State */
    PDRV_SENSORSTATES_TYPE eSensorState = ePdrvSen_GetSensorState();  /* state of sensor statemachine */
    static PDRV_UINT16 uG1StwO = 0U;
    PDRV_UINT16 uG1Stw = uPdrvSpc_GetG1Stw();     /* G1_STW */
    static PDRV_UINT16 uG1ZswO = 0U;
    PDRV_UINT16 uG1Zsw = uPdrvSpc_GetG1Zsw();     /* G1_ZSW */

    /* Is something changed? */
    if (   (eSensorStateO != eSensorState)
        || (uG1StwO != uG1Stw)
        || (uG1ZswO != uG1Zsw)
       )
    {
        PNIO_printf ((PNIO_CHAR*) "%12d\tX1:%08X X2:%08X SS:%04X SZ:%04X\t",
                     m_uTimerCnt1ms,  p_nXist1, p_nXist2, uG1Stw, uG1Zsw);
        if (eSensorStateO != eSensorState)
        {
            PNIO_printf ((PNIO_CHAR*) "%s", cSensorStateText[eSensorState]);
            eSensorStateO = eSensorState;
        }
        PNIO_printf ((PNIO_CHAR*) "\n");
        uG1StwO = uG1Stw;
        uG1ZswO = uG1Zsw;
        m_uPrintfCntDwn = PRINTF_COUNT;
    }
}

/** prints datas of SignOfLifeSynchronisation
 *  @details
 *  @return     none
*/
static PDRV_VOID AppPrintSignOfLifeStatus(PDRV_VOID)
{
    static PDRV_SYNCSTATES_TYPE eStateSignOfLifeO = SYNCSTATE_IDLE;         /* old value of Sign Of Life state */
    PDRV_SYNCSTATES_TYPE eStateSignOfLife = ePdrvSyn_GetSignOfLifeState();  /* Sign Of Life state */
    static PDRV_UINT uFailCntO = 0U;                                        /* old fail counter of Sign Of Life sync */
    PDRV_UINT uFailCnt = uPdrvSyn_GetFailCnt();                             /* fail counter of Sign Of Life sync */

    /* Is something changed? */
    if (   (uFailCntO != uFailCnt)
        || (eStateSignOfLifeO != eStateSignOfLife)
       )
    {
        PNIO_printf ((PNIO_CHAR*) "%12d\tSo:%04X Sn:%04X CL:%04X DL:%04X Er:%04X \n",
                     m_uTimerCnt1ms, eStateSignOfLifeO, eStateSignOfLife, uPdrvSpc_GetStw2(), uPdrvSpc_GetZsw2(), uFailCnt);
        m_uPrintfCntDwn = PRINTF_COUNT;
    }

    uFailCntO = uFailCnt;
    eStateSignOfLifeO = eStateSignOfLife;
}


/* Parameter functions: text, read, write. Application parameters ----------------------------------------*/

/** PROFIdrive read function for parameter PNU00100 "Ramp Gradient"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00100
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->i4[0] = m_nRampGradient;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00100 "Ramp Gradient"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00100
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    m_nRampGradient = p_ptValues->i4[0];
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00110 "range speederror"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00110
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->i4[0] = m_nNTolerance;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00110 "range speederror"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00110
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    m_nNTolerance = p_ptValues->i4[0];
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00111 "tMax speed error"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00111
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->o2[0] = m_uTmax;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00111 "tMax speed error"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00111
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    m_uTmax = p_ptValues->o2[0];
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00115 "speed comp value"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00115
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->i4[0] = m_nCompVal;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00115 "speed comp value"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00115
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    m_nCompVal = p_ptValues->i4[0];
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive text read function for parameter PNU00120 "stop ramps"
 *  @details
 *  @return     pointer to text string
*/
const char * pcPdrv_TfPnu00120
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex            /**< [in] first subindex to write/read */
    )
{
    return (p_uSubindex == 0U) ? "ramp stop" : "quick stop";    /* subindex 0: "ramp stop", subindex 1: "quick stop" */
}

/** PROFIdrive read function for parameter PNU00120 "stop ramps"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00120
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->i4[uI] = m_nRampDown[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00120 "stop ramps"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00120
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        m_nRampDown[p_uSubindex + uI] = p_ptValues->i4[uI];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive text read function for parameter PNU00200 "PT1 gain factors"
 *  @details
 *  @return     pointer to text string
*/
const char * pcPdrv_TfPnu00200
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex            /**< [in] first subindex to write/read */
    )
{
    return (p_uSubindex == 0U) ? "norml operation" : "coast stop";
}

/** PROFIdrive read function for parameter PNU00200 "PT1 gain factors"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00200
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->i2[uI] = m_nPT1GainFactor[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00200 "PT1 gain factors"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00200
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        m_nPT1GainFactor[p_uSubindex + uI] = p_ptValues->i2[uI];
    }
    return PDRV_EV1_NOERROR;
}


/* Parameter functions: text, read, write. For test cases of Profile Tester. ----------------------------------------*/

/** PROFIdrive text read function for parameter PNU00700 "Set Fault"
 *  @details
 *  @return     pointer to text string
*/
const char * pcPdrv_TfPnu00700
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex            /**< [in] first subindex to write/read */
    )
{
    return (p_uSubindex == 0U) ? "fault number" : "fault value";
}

/** PROFIdrive read function for parameter PNU00700 "Set Fault"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00700
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->o2[uI] = m_uValPnu700[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00700 "Set Fault"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00700
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;
    PDRV_UINT32 uError = PDRV_EV1_NOERROR;

    /* subindex 0 AND value too big? */
    if (   (p_uSubindex == 0U)
        && (p_ptValues->o2[0U] > FAULT_USER9)
       )
    {
        uError = PDRV_EV1_VALUE_IMPERMISS;
    }

    /* no error? */
    if (uError == PDRV_EV1_NOERROR)
    {
        for (uI = 0U; uI < p_uNrOfElements; uI++)
        {
            m_uValPnu700[p_uSubindex + uI] = p_ptValues->o2[uI];
        }
        /* trigger a fault? */
        if (   (p_uSubindex == 0U)
            && (m_uValPnu700[0U] != 0U)
           )
        { /* writing of parameter subindex 0 triggers a fault event */
            m_uFaultVal[m_uValPnu700[0]] = m_uValPnu700[1];
            PdrvDiag_SetFaultMsg(m_uValPnu700[0], m_uValPnu700[1]);
        }
    }
    return uError;
}

/** PROFIdrive read function for parameter PNU00701 "Reset Fault"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00701
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->o2[0U] = m_uValPnu701;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00701 "Reset Fault"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00701
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    m_uValPnu701 = p_ptValues->o2[0U];
    m_uFaultVal[m_uValPnu701] = 0U;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive text read function for parameter PNU00702 "Set Warning"
 *  @details
 *  @return     pointer to text string
*/
const char * pcPdrv_TfPnu00702
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex            /**< [in] first subindex to write/read */
    )
{
    return (p_uSubindex == 0U) ? "warning number" : "0=reset, 0<>set";
}

/** PROFIdrive read function for parameter PNU00702 "Set Warning"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00702
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->o2[uI] = m_uValPnu702[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00702 "Set Warning"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00702
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;
    PDRV_UINT32 uError = PDRV_EV1_NOERROR;

    /* test value of subindex 1 */
    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        /* subindex 1 AND value other than TRUE or FALSE? */
        if (   (uI == 1U)
            && (p_ptValues->o2[uI] > PDRV_TRUE)
           )
        {
            uError = PDRV_EV1_VALUE_IMPERMISS;
        }
    }

    /* no error? */
    if (uError == PDRV_EV1_NOERROR)
    {
        for (uI = 0U; uI < p_uNrOfElements; uI++)
        {
            m_uValPnu702[p_uSubindex + uI] = p_ptValues->o2[uI];
        }
        /* reset/set a warning? */
        if (p_uSubindex == 0U)
        { /* writing of parameter subindex 0 triggers warning event */
            PdrvDiag_SetWarning(m_uValPnu702[0], m_uValPnu702[1]);
        }
    }
    return uError;
}

/** PROFIdrive read function for parameter PNU00720..PNU00790 Test for "read only" parameters
 *  @details    parameter test with Profile Tester
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00720
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT8 ParObjFmt = p_ptParObj->uIdentifier & 0x00FFU;
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        switch (ParObjFmt)
        {
        case PDRV_PARID_BL:
            p_ptValues->bl[uI] = (p_ptParObj->uPnu + p_uSubindex + uI) & 1 ? PDRV_TRUE : PDRV_FALSE;
            break;
        case PDRV_PARID_I1:
            p_ptValues->i1[uI] = -(PDRV_INT8)(p_ptParObj->uPnu + p_uSubindex + uI);
            break;
        case PDRV_PARID_I2:
        case PDRV_PARID_N2:
        case PDRV_PARID_X2:
        case PDRV_PARID_E2:
            p_ptValues->i2[uI] = -p_ptParObj->uPnu + p_uSubindex + uI;
            break;
        case PDRV_PARID_I4:
        case PDRV_PARID_N4:
        case PDRV_PARID_X4:
        case PDRV_PARID_C4:
        case PDRV_PARID_TOD:
        case PDRV_PARID_TD:
            p_ptValues->i4[uI] = -p_ptParObj->uPnu + p_uSubindex + uI;
            break;
        case PDRV_PARID_I8:
            p_ptValues->i8[uI] = -p_ptParObj->uPnu + p_uSubindex + uI;
            break;
        case PDRV_PARID_O1:
            p_ptValues->o1[uI] = (PDRV_UINT8)(p_ptParObj->uPnu + p_uSubindex + uI);
            break;
        case PDRV_PARID_VS:
        case PDRV_PARID_OS:
            p_ptValues->vs[uI] = p_ptParObj->puName[uI];
            break;
        case PDRV_PARID_O2:
        case PDRV_PARID_US:
        case PDRV_PARID_V2:
        case PDRV_PARID_L2:
        case PDRV_PARID_R2:
        case PDRV_PARID_T2:
        case PDRV_PARID_D2:
            p_ptValues->o2[uI] = p_ptParObj->uPnu + p_uSubindex + uI;
            break;
        case PDRV_PARID_O4:
        case PDRV_PARID_T4:
            p_ptValues->o4[uI] = p_ptParObj->uPnu + p_uSubindex + uI;
            break;
        case PDRV_PARID_O8:
            p_ptValues->o8[uI] = p_ptParObj->uPnu + p_uSubindex + uI;
            break;
        case PDRV_PARID_F4:
            p_ptValues->f4[uI] = FLOAT_3000 + p_ptParObj->uPnu + p_uSubindex + uI;
            break;
        case PDRV_PARID_F8:
            p_ptValues->f8[uI] = FLOAT_6000 + p_ptParObj->uPnu + p_uSubindex + uI;
            break;
        case PDRV_PARID_DATE:
        {
            const PDRV_DATE eDate = { .ms = 300U, .min = 59U, .h = 23U, .d = 0x21U, .m = 1U, .y = 51U};
            p_ptValues->date[uI] = eDate;
            p_ptValues->date[uI].ms = p_ptParObj->uPnu + p_uSubindex + uI;
            break;
        }
        case PDRV_PARID_TDW:
            p_ptValues->tdw[uI].d = p_uSubindex + uI;
            p_ptValues->tdw[uI].ms = p_ptParObj->uPnu + p_uSubindex + uI;
            break;
        case PDRV_PARID_TODW:
            p_ptValues->todw[uI].d = p_uSubindex + uI;
            p_ptValues->todw[uI].ms = p_ptParObj->uPnu + p_uSubindex + uI;
            break;

        default:
            break;
        }
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive text read function for parameter PNU00741 "RO-Single Par V2"
 *  @details
 *  @return     pointer to text string
*/
const char * pcPdrv_TfPnu00741
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex            /**< [in] first subindex to write/read */
    )
{
    static const char * pcBitTexts[32] =
        {
                "Bit 0: false", "Bit 0: true",
                "Bit 1: false", "Bit 1: true",
                "Bit 2: false", "Bit 2: true",
                "Bit 3: false", "Bit 3: true",
                "Bit 4: false", "Bit 4: true",
                "Bit 5: false", "Bit 5: true",
                "Bit 6: false", "Bit 6: true",
                "Bit 7: false", "Bit 7: true",
                "Bit 8: false", "Bit 8: true",
                "Bit 9: false", "Bit 9: true",
                "Bit10: false", "Bit10: true",
                "Bit11: false", "Bit11: true",
                "Bit12: false", "Bit12: true",
                "Bit13: false", "Bit13: true",
                "Bit14: false", "Bit14: true",
                "Bit15: false", "Bit15: true"
        };

    return pcBitTexts[p_uSubindex];
}

/** PROFIdrive read function for parameter PNU00800 "test parameter 0"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00800
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->bl[0] = m_uValPnu800;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00800 "test parameter 0"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00800
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    m_uValPnu800 = p_ptValues->bl[0];
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive text read function for parameter PNU00800 "test parameter 0" and PNU00720 "RO-Single Par BL"
 *  @details
 *  @return     pointer to text string
*/
const char * pcPdrv_TfPnu00800
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex            /**< [in] first subindex to write/read */
    )
{
    return (p_uSubindex == 0U) ? "false" : "true";
}

/** PROFIdrive read function for parameter PNU00801 "test parameter 1"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00801
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->i1[0] = m_nValPnu801;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00801 "test parameter 1"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00801
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    m_nValPnu801 = p_ptValues->i1[0];
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00802 "test parameter 2"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00802
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->i2[0] = m_nValPnu802;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00802 "test parameter 2"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00802
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    m_nValPnu802 = p_ptValues->i2[0];
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00803 "test parameter 3"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00803
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->i4[0] = m_nValPnu803;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00803 "test parameter 3"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00803
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    m_nValPnu803 = p_ptValues->i4[0];
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00804 "test parameter 4"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00804
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->o1[0] = m_uValPnu804;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00804 "test parameter 4"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00804
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    m_uValPnu804 = p_ptValues->o1[0];
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00805 "test parameter 5"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00805
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->o2[0] = m_uValPnu805;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00805 "test parameter 5"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00805
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    m_uValPnu805 = p_ptValues->o2[0];
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00806 "test parameter 6"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00806
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->o4[0] = m_uValPnu806;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00806 "test parameter 6"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00806
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    m_uValPnu806 = p_ptValues->o4[0];
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00807 "test parameter 7"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00807
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->f4[0] = m_fValPnu807;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00807 "test parameter 7"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00807
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    m_fValPnu807 = p_ptValues->f4[0];
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00808 "test parameter 8"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00808
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->o2[0] = m_uValPnu808;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00808 "test parameter 8"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00808
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    m_uValPnu808 = p_ptValues->o2[0];
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00809 "test parameter 9"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00809
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->bl[uI] = m_bValPnu809[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;

}

/** PROFIdrive write function for parameter PNU00809 "test parameter 9"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00809
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        m_bValPnu809[p_uSubindex + uI] = p_ptValues->bl[uI];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00810 "test parameter 10"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00810
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->i1[uI] = m_nValPnu810[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;

}

/** PROFIdrive write function for parameter PNU00810 "test parameter 10"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00810
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        m_nValPnu810[p_uSubindex + uI] = p_ptValues->i1[uI];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00811 "test parameter 11"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00811
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->i2[uI] = m_nValPnu811[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;

}

/** PROFIdrive write function for parameter PNU00811 "test parameter 11"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00811
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        m_nValPnu811[p_uSubindex + uI] = p_ptValues->i2[uI];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00812 "test parameter 12"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00812
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->i4[uI] = m_nValPnu812[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;

}

/** PROFIdrive write function for parameter PNU00812 "test parameter 12"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00812
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        m_nValPnu812[p_uSubindex + uI] = p_ptValues->i4[uI];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00813 "test parameter 13"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00813
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->o1[uI] = m_uValPnu813[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;

}

/** PROFIdrive write function for parameter PNU00813 "test parameter 13"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00813
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        m_uValPnu813[p_uSubindex + uI] = p_ptValues->o1[uI];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00814 "test parameter 14"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00814
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->o2[uI] = m_uValPnu814[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;

}

/** PROFIdrive write function for parameter PNU00814 "test parameter 14"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00814
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        m_uValPnu814[p_uSubindex + uI] = p_ptValues->o2[uI];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00815 "test parameter 15"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00815
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->o4[uI] = m_uValPnu815[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00815 "test parameter 15"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00815
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        m_uValPnu815[p_uSubindex + uI] = p_ptValues->o4[uI];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00816 "test parameter 16"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00816
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->f4[uI] = m_fValPnu816[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;

}

/** PROFIdrive write function for parameter PNU00816 "test parameter 16"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00816
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        m_fValPnu816[p_uSubindex + uI] = p_ptValues->f4[uI];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00817 "test parameter 17"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00817
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->td[uI] = m_uValPnu817[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;

}

/** PROFIdrive write function for parameter PNU00817 "test parameter 17"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00817
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        m_uValPnu817[p_uSubindex + uI] = p_ptValues->td[uI];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00818 "test parameter 18"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00818
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->i2[uI] = m_nValPnu818[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;

}

/** PROFIdrive write function for parameter PNU00818 "test parameter 18"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00818
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        m_nValPnu818[p_uSubindex + uI] = p_ptValues->i2[uI];
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU00819 "test parameter 19"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00819
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->i4[uI] = m_nValPnu819[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;

}

/** PROFIdrive write function for parameter PNU00819 "test parameter 19"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00819
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to write/read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        m_nValPnu819[p_uSubindex + uI] = p_ptValues->i4[uI];
    }
    return PDRV_EV1_NOERROR;
}

#endif
