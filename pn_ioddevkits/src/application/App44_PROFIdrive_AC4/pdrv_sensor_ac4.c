/*********************************************************************************************************************/
/**@file        pdrv_sensor_ac4.c
 * @brief       PROFIdrive Sensor simulation
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
 *              Shorthand symbol: SEN (Sensor)<br>
 *              The file implements sensor related functions.
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
#include "pdrv_sensor_ac4.h"
#include "pdrv_parmanager_ac4.h"
#include "pdrv_pardatabase_ac4.h"
#include "pdrv_setpointchannel_ac4.h"
#include "pdrv_diagnostics_ac4.h"
#include "pdrv_statemachine_ac4.h"
#include "pdrv_application_ac4.h"

#if (EXAMPL_DEV_CONFIG_VERSION == 44)

/*------------  extern  functions  ------------*/

/*------------  extern  data  ------------*/

/*------------  type definitions, constants, enums  ------------*/
static PDRV_SENSORSTATES_TYPE m_eSensorState = SENSORSTATE_IDLE;    /**< last sensor state */
static PDRV_UINT16 m_uGx_STW = 0U;                                  /**< last sensor STW value */

/*------------  static  data  ------------*/
static PDRV_UINT32 m_uParkingTime = 0U;                 /**< start time for state of parking sensor [1ms] */
static PDRV_BOOL m_bIsSensorError = PDRV_FALSE;         /**< simulated sensor has an error */
static PDRV_INT32 m_nSensorErrorValue = 0;              /**< simulated sensor value */
static PDRV_BOOL m_bIsGxXist1Valid = PDRV_FALSE;    /**< simulated sensor delivers a valid value */
static PDRV_INT32 m_nG1_Xist2 = 0;                      /**< simulated sensor G1_XIST2 */


/*------------  public data  ------------*/

/*------------  functions  ------------*/

/** PROFIdrive sensor statemachine
 *  @details    see PDRV V4.2 chapter 6.3.6.7.1 "State diagram", table 110 "States", table 111 "Transitions"
 *  @return     none
*/
PDRV_VOID PdrvSen_SensorStateMachine(PDRV_VOID)
{
    PDRV_SENSORSTATES_TYPE eSensorState = m_eSensorState;   /* sensor state */
    PDRV_UINT16 uGx_STW = uPdrvSpc_GetG1Stw();              /* sensor control word */
    PDRV_UINT16 uGx_Cmd = uGx_STW & PDRV_SEN_STW_CMD;       /* sensor control command */
    PDRV_UINT16 uGx_ZSW = uPdrvSpc_GetG1Zsw();              /* sensor status word */
    PDRV_BOOL bIsSensorError = bPdrvSen_IsSensorError();    /* sensor error flag */
    PDRV_BOOL bIsGxXist1Valid = bPdrvSen_IsGxXist1Valid();  /* XIST1 is valid flag */

    /* calculate new state */
    switch (eSensorState)
    {
    case SENSORSTATE_IDLE:                   /* state after power on */
    {
        eSensorState = SENSORSTATE_NORMAL_OPERATION;
        break;
    }
    case SENSORSTATE_NORMAL_OPERATION:       /* state SD1  normal operation */
    {
         /* TD16 */
        if (uGx_STW & PDRV_SEN_STW_PARK)                                    /* TD16 sensor control word */
        {
            eSensorState = SENSORSTATE_PARKING;
        }    /* TD20 */
        else if (   bIsSensorError                                          /* TD20 error occurred or illegal command */
                 || (uGx_Cmd > PDRV_SEN_STW_CMD_CANC)
                )
        {
            eSensorState = SENSORSTATE_ERROR;
        }    /* TD3 */
        else if (   !(uGx_STW & PDRV_SEN_STW_MODE)                          /* TD3 sensor control word */
                 && (uGx_Cmd == PDRV_SEN_STW_CMD_READ)
                 && (uGx_STW & PDRV_SEN_STW_FUNCX)
                 && bPdrvSen_IsRefValFound()                                /* reference value is found? */
                )
        {
            eSensorState = SENSORSTATE_REF_VALUE;
        }    /* TD6 */
        else if (   !(uGx_STW & PDRV_SEN_STW_MODE)                          /* TD6 sensor control word */
                 && (uGx_Cmd == PDRV_SEN_STW_CMD_ACT)
                 && (uGx_STW & PDRV_SEN_STW_FUNCX)
               )
        {
            eSensorState = SENSORSTATE_WAIT_REF_MARK;
        }    /* TD11 */
        else if (   (uGx_STW & PDRV_SEN_STW_MODE)                           /* TD11 sensor control word */
                 && (uGx_Cmd == PDRV_SEN_STW_CMD_ACT)
                 && (uGx_STW & PDRV_SEN_STW_FUNCX)
                )

        {
            eSensorState = SENSORSTATE_WAIT_MEAS_VAL;
        }    /* TD14 */
        else if (   (uGx_STW & PDRV_SEN_STW_MODE)                           /* TD14 sensor control word */
                 && (uGx_Cmd == PDRV_SEN_STW_CMD_READ)
                 && (uGx_STW & PDRV_SEN_STW_FUNCX)
                 && bPdrvSen_IsRefValFound()                                /* related measured value is found */
                )
        {
            eSensorState = SENSORSTATE_RDY_MEAS_VAL;
        }    /* TD7 */
        else if ((uGx_STW & PDRV_SEN_STW_HOME_R) == PDRV_SEN_STW_HOME_R)    /* TD7 sensor control word */
        {
            eSensorState = SENSORSTATE_SET_HOMEPOS;
        }
        break;
    }
    case SENSORSTATE_ERROR_ACKNOWLEDGEMENT:  /* state SD2  error acknowledgement */
    {
        /* TD23 */
        if (   (uGx_STW & PDRV_SEN_STW_PARK)                                /* TD23 sensor control word */
            || (   !(uGx_STW & PDRV_SEN_STW_ACK)
                && !bIsSensorError                                  /* error removed while Gx_XIST1 is invalid */
                && !bIsGxXist1Valid
               )
           )
        {
            PdrvDiag_AckSenFault();                         /* acknowledge sensor faults in diagnostics module too */
            eSensorState = SENSORSTATE_PARKING;
        }     /* TD1 */
        else if (   !(uGx_STW & PDRV_SEN_STW_ACK)                           /* TD1 sensor control word */
                 && !bIsSensorError                                         /* error is removed too */
                )
        {
            PdrvDiag_AckSenFault();                         /* acknowledge sensor faults in diagnostics module too */
            eSensorState = SENSORSTATE_NORMAL_OPERATION;
        }    /* TD22 */
        else if (   !(uGx_STW & PDRV_SEN_STW_ACK)                           /* TD22 sensor control word */
                 && bIsSensorError                               /* error still exists while Gx_XIST1 is valid */
                 && bIsGxXist1Valid
                )
        {
            eSensorState = SENSORSTATE_ERROR;
        }
        break;
    }
    case SENSORSTATE_ERROR:                  /* state SD3  error i.e. error code in Gx_XIST2 */
    {
        /* TD23 */
        if (   (uGx_STW & PDRV_SEN_STW_PARK)                                /* TD23 sensor control word */
            || (   !(uGx_STW & PDRV_SEN_STW_ACK)
                && !bIsSensorError                                         /* error removed while Gx_XIST1 is invalid */
                && !bIsGxXist1Valid
               )
           )
        {
            PdrvDiag_AckSenFault();                         /* acknowledge sensor faults in diagnostics module too */
            eSensorState = SENSORSTATE_PARKING;
        }     /* TD21 */
        else if (   (uGx_STW & PDRV_SEN_STW_ACK)                            /* TD21 sensor control word */
                 && bIsGxXist1Valid                                           /* Gx_XIST1 is valid? */
                )
        {
            eSensorState = SENSORSTATE_ERROR_ACKNOWLEDGEMENT;
        }
        break;
    }
    case SENSORSTATE_REF_VALUE:              /* state SD4  reference value in Gx_XIST2 */
    {
        /* TD16 */
        if (uGx_STW & PDRV_SEN_STW_PARK)                                    /* TD16 sensor control word */
        {
           eSensorState = SENSORSTATE_PARKING;
        }    /* TD20 */
        else if (   bIsSensorError                                          /* TD20 error occurred or illegal command */
                 || (uGx_Cmd > PDRV_SEN_STW_CMD_CANC)
                )
        {
            eSensorState = SENSORSTATE_ERROR;
        }    /* TD2 */
        else if (uGx_Cmd == PDRV_SEN_STW_CMD_IDLE)                          /* TD2 sensor control word */
        {
            eSensorState = SENSORSTATE_NORMAL_OPERATION;
        }
        break;
    }
    case SENSORSTATE_WAIT_REF_MARK:          /* state SD5  wait for reference mark */
    {
        /* TD16 */
        if (uGx_STW & PDRV_SEN_STW_PARK)                                    /* TD16 sensor control word */
        {
           eSensorState = SENSORSTATE_PARKING;
        }    /* TD20 */
        else if (   bIsSensorError                                          /* TD20 error occurred or illegal command */
                 || (uGx_Cmd > PDRV_SEN_STW_CMD_CANC)
                )
        {
            eSensorState = SENSORSTATE_ERROR;
        }    /* TD4 OR TD5 */
        else if (   (   (uGx_Cmd == PDRV_SEN_STW_CMD_IDLE)                  /* TD4 sensor control word */
                     && bPdrvSen_IsRefMarkFound()                           /* reference mark is found */
                    )
                 || (uGx_Cmd == PDRV_SEN_STW_CMD_CANC)                      /* TD5 sensor control word */
                )
        {
            eSensorState = SENSORSTATE_NORMAL_OPERATION;
        }
        break;
    }
    case SENSORSTATE_SET_HOMEPOS:            /* state SD7  set/shift home position */
    {
        /* TD16 */
        if (uGx_STW & PDRV_SEN_STW_PARK)                                    /* TD16 sensor control word */
        {
           eSensorState = SENSORSTATE_PARKING;
        }    /* TD20 */
        else if (   bIsSensorError                                          /* TD20 error occurred or illegal command */
                 || (uGx_Cmd > PDRV_SEN_STW_CMD_CANC)
                )
        {
            eSensorState = SENSORSTATE_ERROR;
        }    /* TD8 */
        else if (!(uGx_STW & PDRV_SEN_STW_HOME_R))                          /* TD8 sensor control word */
        {
            eSensorState = SENSORSTATE_NORMAL_OPERATION;
        }
        break;
    }
    case SENSORSTATE_WAIT_MEAS_VAL:          /* state SD10 wait for measured values */
    {
        /* TD16 */
        if (uGx_STW & PDRV_SEN_STW_PARK)                                    /* TD16 sensor control word */
        {
           eSensorState = SENSORSTATE_PARKING;
        }    /* TD20 */
        else if (   bIsSensorError                                          /* TD20 error occurred or illegal command */
                 || (uGx_Cmd > PDRV_SEN_STW_CMD_CANC)
                )
        {
            eSensorState = SENSORSTATE_ERROR;
        }    /* TD12 OR TD13*/
        else if (   (uGx_Cmd == PDRV_SEN_STW_CMD_CANC)                      /* TD12 sensor control word */
                 || (   (uGx_Cmd == PDRV_SEN_STW_CMD_IDLE)                  /* TD13 sensor control word */
                     && bPdrvSen_IsRefValFound()                            /* measured values are found? */
                    )
                )
        {
            eSensorState = SENSORSTATE_NORMAL_OPERATION;
        }    /* TD31 */
        else if (   (uGx_Cmd == PDRV_SEN_STW_CMD_ACT)                       /* TD31 sensor control word */
                 && ((m_uGx_STW & PDRV_SEN_STW_CMD) == PDRV_SEN_STW_CMD_IDLE)
                 && (uGx_STW & PDRV_SEN_STW_MODE)
                 && (uGx_STW & PDRV_SEN_STW_FUNCX)
                )
        {
            eSensorState = SENSORSTATE_WAIT_MEAS_VAL;
        }    /* TD32 */
        else if (   (uGx_STW & PDRV_SEN_STW_MODE)                           /* TD32 sensor control word */
                 && (uGx_Cmd == PDRV_SEN_STW_CMD_READ)
                 && (uGx_STW & PDRV_SEN_STW_FUNCX)
                 && bPdrvSen_IsRefValFound()                                /* related measured value found? */
                )
        {
            eSensorState = SENSORSTATE_RDY_MEAS_VAL;
        }
        break;
    }
    case SENSORSTATE_RDY_MEAS_VAL:           /* state SD11 measured value in Gx_XIST2 */
    {
        /* TD16 */
        if (uGx_STW & PDRV_SEN_STW_PARK)                                    /* TD16 sensor control word */
        {
           eSensorState = SENSORSTATE_PARKING;
        }    /* TD20 */
        else if (   bIsSensorError                                          /* TD20 error occurred or illegal command */
                 || (uGx_Cmd > PDRV_SEN_STW_CMD_CANC)
                )
        {
            eSensorState = SENSORSTATE_ERROR;
        }    /* TD15 */
        else if (   (   (uGx_Cmd == PDRV_SEN_STW_CMD_IDLE)                  /* TD15 sensor control word */
                     && !bPdrvSen_IsMeasureActive()           /* no measurement is active (all measured values found) */
                    )
                 || (   (uGx_STW & PDRV_SEN_STW_MODE)                       /* TD15 sensor control word */
                     && (uGx_Cmd == PDRV_SEN_STW_CMD_CANC)
                    )
                )
        {
            eSensorState = SENSORSTATE_NORMAL_OPERATION;
        }    /* TD33 */
        else if (   (   (uGx_STW & PDRV_SEN_STW_MODE)                       /* TD33 sensor control word */
                     && (uGx_Cmd == PDRV_SEN_STW_CMD_ACT)
                     && (uGx_STW & PDRV_SEN_STW_FUNCX)
                    )
                 || (   (uGx_Cmd == PDRV_SEN_STW_CMD_IDLE)
                     && bPdrvSen_IsMeasureActive() /* measurement process still active (not all measured value found)? */
                    )
                )
        {
            eSensorState = SENSORSTATE_WAIT_MEAS_VAL;
        }    /* TD34 */
        else if (   (uGx_Cmd == PDRV_SEN_STW_CMD_READ)                      /* TD34 sensor control word */
                 && ((m_uGx_STW & PDRV_SEN_STW_CMD) == PDRV_SEN_STW_CMD_IDLE)
                 && (uGx_STW & PDRV_SEN_STW_MODE)
                 && (uGx_STW & PDRV_SEN_STW_FUNCX)
                 && bPdrvSen_IsRefValFound()                                /* related measured value found */
                )
        {
            eSensorState = SENSORSTATE_RDY_MEAS_VAL;
        }
        break;
    }
    case SENSORSTATE_PARKING:                /* state SD12 wait for measured values */
    {
        /* TD17 */
        if (   !(uGx_STW & PDRV_SEN_STW_PARK)                               /* TD17 sensor control word */
            && (100U < (uPdrvApp_GetTimer1ms() - m_uParkingTime))          /* Gx_ZSW Bit14 was set for at least 100ms */
           )
        {
            eSensorState = SENSORSTATE_NORMAL_OPERATION;
        }
        break;
    }
    case SENSORSTATE_PARKING_ERROR:          /* state SD13 parking and error (optional) */
    {
        /* TODO: not implemented yet */
        break;
    }
    case SENSORSTATE_PARKING_ERROR_ACK:      /* state SD14 parking and error acknowledgement (optional) */
    {
        /* TODO: not implemented yet */
        break;
    }
    default:                                /* Error of the Sensor State Machine */
        eSensorState = SENSORSTATE_STATEMACHINE_ERROR;
        break;
    }

    /* changed sensor state? */
    if (   (eSensorState != m_eSensorState)
        || (eSensorState == SENSORSTATE_ERROR_ACKNOWLEDGEMENT)
       )
    {
        /* actions for new state see PDRV V4.2 table 110 */
        switch (eSensorState)
        {
        case SENSORSTATE_IDLE:                   /* state after power on */
        {
            uGx_ZSW = 0x0000U;
            break;
        }
        case SENSORSTATE_NORMAL_OPERATION:       /* state SD1  normal operation */
        {
            uGx_ZSW &= PDRV_SEN_ZSW_ABS_T;
            break;
        }
        case SENSORSTATE_ERROR_ACKNOWLEDGEMENT:  /* state SD2  error acknowledgement */
        {
            uGx_ZSW &= 0x7F00U;
            uGx_ZSW |= PDRV_SEN_ZSW_ACK;
            /* set bit15 as long as valid Errorcode is posted in Gx_XIST2 (as long as error is present) */
            if (bIsSensorError)
            {
                uGx_ZSW |= PDRV_SEN_ZSW_ERROR;
            }
            break;
        }
        case SENSORSTATE_ERROR:                  /* state SD3  error i.e. error code in Gx_XIST2 */
        {
            uGx_ZSW &= 0xF700U;
            uGx_ZSW |= PDRV_SEN_ZSW_ERROR;
            break;
        }
        case SENSORSTATE_REF_VALUE:              /* state SD4  reference value in Gx_XIST2 */
        {
            uGx_ZSW &= 0xFF00U;
            uGx_ZSW |= (uGx_STW & PDRV_SEN_STW_FUNCX) << 4U;
            break;
        }
        case SENSORSTATE_WAIT_REF_MARK:          /* state SD5  wait for reference mark */
        {
            uGx_ZSW &= 0xFF00U;
            uGx_ZSW |= uGx_STW & PDRV_SEN_STW_FUNCX;
            break;
        }
        case SENSORSTATE_SET_HOMEPOS:            /* state SD7  set/shift home position */
        {
            uGx_ZSW &= 0xFF00U;
            uGx_ZSW |= PDRV_SEN_ZSW_HOME;
            break;
        }
        case SENSORSTATE_WAIT_MEAS_VAL:          /* state SD10 wait for measured values */
        {
            uGx_ZSW &= 0xFF00U;
            uGx_ZSW |= uGx_STW & PDRV_SEN_STW_FUNCX;
            break;
        }
        case SENSORSTATE_RDY_MEAS_VAL:           /* state SD11 measured value in Gx_XIST2 */
        {
            uGx_ZSW &= 0xFF00U;
            uGx_ZSW |= (uGx_STW & PDRV_SEN_STW_FUNCX) << 4U;
            break;
        }
        case SENSORSTATE_PARKING:                /* state SD12 wait for measured values */
        {
            uGx_ZSW = PDRV_SEN_ZSW_PARK;
            m_uParkingTime = uPdrvApp_GetTimer1ms();
            break;
        }
        case SENSORSTATE_PARKING_ERROR:          /* state SD13 parking and error (optional) */
        {
            /* TODO: not implemented yet */
            break;
        }
        case SENSORSTATE_PARKING_ERROR_ACK:      /* state SD14 parking and error acknowledgement (optional) */
        {
            /* TODO: not implemented yet */
            break;
        }
        default:                                /* Error of the Sensor State Machine */
            break;
        }
    }

    m_eSensorState = eSensorState;
    m_uGx_STW = uGx_STW;
    PdrvSpc_SetG1Zsw(uGx_ZSW);
}

/** Get the sensor state
 * @return  sensor state
*/
PDRV_SENSORSTATES_TYPE ePdrvSen_GetSensorState(PDRV_VOID)
{
    return m_eSensorState;
}

/** Simple Sensor simulation
 *  @details    PDRV user should be change the functionality and conditions to his own application!<br>
 *              Sensor simulation: position is calculated from speed.<br>
 *  @return     output of sensor simulation [N4]
*/
PDRV_INT32 nPdrvSen_Sensor
    (PDRV_INT32 p_nX    /**< [in] input signal [N4] */
    )
{
    static PDRV_INT64 nY = 0LL; /* internal 64 bit position is calculated from speed */
    PDRV_INT32 nXist1 = 0L;     /* new XIST1 */
    PDRV_INT32 nXist2 = 0L;     /* new XIST2 */
    PDRV_UINT16 uGxStw = uPdrvSpc_GetG1Stw();   /* sensor control word */
    PDRV_SENSORSTATES_TYPE eSensorState = ePdrvSen_GetSensorState();    /* sensor state */

    if (   (eSensorState == SENSORSTATE_PARKING)    /* SD12 action? see PDRV V4.2 table 110 */
        || (eSensorState == SENSORSTATE_IDLE)
       )
    {
        m_bIsSensorError = PDRV_FALSE;
        m_bIsGxXist1Valid = PDRV_FALSE;
    }
    else if (eSensorState >= SENSORSTATE_STATEMACHINE_ERROR)
    {
        m_bIsSensorError = PDRV_TRUE;
        m_nSensorErrorValue = PDRV_SEN_ERR_STATE;
        m_bIsGxXist1Valid = PDRV_FALSE;
    }
    else
    {
        /* Unsupported functions, commands, modes, request for that simulated sensor? */
        if (   //(uGxStw & PDRV_SEN_STW_FUNCX)    /* JM: only set functions will not trigger sensor error except combined with cmd*/
            (uGxStw & PDRV_SEN_STW_CMD)      /* no commands */
            /*|| (uGxStw & PDRV_SEN_STW_MODE)*/     /* no mode control */
            || (uGxStw & PDRV_SEN_STW_HOME_R)   /* no set/shift of home position */
           )
        {
            m_bIsSensorError = PDRV_TRUE;
            m_nSensorErrorValue = PDRV_SEN_ERR_CMD;
        }
        else if (uGxStw & PDRV_SEN_STW_ABS_R)   /* no absolute value cyclically */
        {
            m_bIsSensorError = PDRV_TRUE;
            m_nSensorErrorValue = PDRV_SEN_ERR_ABS1;
        }
        else if (eSensorState == SENSORSTATE_ERROR_ACKNOWLEDGEMENT)
        {
            m_bIsSensorError = PDRV_FALSE;
        }

        /* calculate new XIST1 from speed value */
        nY += p_nX;
        nXist1 = (PDRV_INT32) (((PDRV_UINT64)nY / 10ULL) >> 11ULL);
        nXist2 = (PDRV_INT32) (((PDRV_UINT64)nY / 10ULL) >> 22ULL);
        m_bIsGxXist1Valid = PDRV_TRUE;
    }

    /* Gx_XIST2 assignment, see PDRV V4.2 chapter 6.3.6.3 table 106 */
    /* SD12? (priority 1) */
    if (eSensorState == SENSORSTATE_PARKING)
    {
        m_nG1_Xist2 = 0L;   /* no valid value */
    }    /* BD2: error state OR error acknowledge with a persistent error? (priority 2) */
    else if (   (eSensorState == SENSORSTATE_ERROR)
             || (eSensorState == SENSORSTATE_PARKING_ERROR)
             || (eSensorState >= SENSORSTATE_STATEMACHINE_ERROR)
             || (   (   (eSensorState == SENSORSTATE_ERROR_ACKNOWLEDGEMENT)
                     || (eSensorState == SENSORSTATE_PARKING_ERROR_ACK)
                    )
                 && m_bIsSensorError
                )
            )
    {
    	//TODO: Only error which will cause drive stop will add diagnosis buffer (Jie Ming 2021/06/29)
        //PdrvDiag_SetFaultMsg(FAULT_SENSOR, m_nSensorErrorValue);
        m_nG1_Xist2 = m_nSensorErrorValue;  /* error code */
    }    /* BD2: error acknowledge without a persistent error? (priority 2) */
    else if (   (eSensorState == SENSORSTATE_ERROR_ACKNOWLEDGEMENT)
             || (eSensorState == SENSORSTATE_PARKING_ERROR_ACK)
            )
    {
        m_nSensorErrorValue = 0L;
        m_nG1_Xist2 = 0L;
    }    /* SD4 OR SD11? (priority 3) */
    else if (   (eSensorState == SENSORSTATE_REF_VALUE)
             || (eSensorState == SENSORSTATE_RDY_MEAS_VAL)
            )
    {
        m_nG1_Xist2 = 0L;   /* position actual value (reference mark or measured value) */
    }    /* SD1 OR SD10? (priority 4) */
    else if (   (eSensorState == SENSORSTATE_NORMAL_OPERATION)
             || (eSensorState == SENSORSTATE_WAIT_MEAS_VAL)
            )
    {
        m_nG1_Xist2 = (uGxStw & PDRV_SEN_STW_ABS_R)? nXist2 : 0L;   /* position actual value if an absolute value is available */
    }    /* communication in preparation AND Gx_ZSW transmit absolute value cyclically? */
    else if (   (eSensorState == SENSORSTATE_IDLE)
             && (uPdrvSpc_GetG1Zsw() & PDRV_SEN_ZSW_ABS_T)
            )
    {
        m_nG1_Xist2 = 0L;   /* absolute position actual value if already present, indicated by Gx_ZSW.13 = 1 */
    }
    else
    {
        m_nG1_Xist2 = 0L;   /* no assignment defined */
    }

    return nXist1;
}

/** Is there an sensor error
 *  @details
 *  @return     PDRV_TRUE if a sensor error exists
*/
PDRV_BOOL bPdrvSen_IsSensorError(PDRV_VOID)
{
    return m_bIsSensorError;
}

/** True if sensor Gx_XIST1 gets a valid value
 *  @details    PDRV user should be change the functionality and conditions to his own application!
 *  @return     PDRV_TRUE if Gx_XIST1 is valid
*/
PDRV_BOOL bPdrvSen_IsGxXist1Valid(PDRV_VOID)
{
    return m_bIsGxXist1Valid;
}

/** True if sensor reference value is found
 *  @details    PDRV user should be change the functionality and conditions to his own application!
 *  @return     PDRV_TRUE if reference value found
*/
PDRV_BOOL bPdrvSen_IsRefValFound(PDRV_VOID)
{
    return PDRV_TRUE;
}

/** True if sensor measurement is active
 *  @details    PDRV user should be change the functionality and conditions to his own application!
 *  @return     PDRV_TRUE if measurement is active
*/
PDRV_BOOL bPdrvSen_IsMeasureActive(PDRV_VOID)
{
    return PDRV_FALSE;
}

/** True if sensor reference mark is found
 *  @details    PDRV user should be change the functionality and conditions to his own application!
 *  @return     PDRV_TRUE if reference mark is found
*/
PDRV_BOOL bPdrvSen_IsRefMarkFound(PDRV_VOID)
{
    return PDRV_TRUE;
}

/** gets the actual value of Gx_XIST2
 *  @details
 *  @return     value of Gx_XIST2
*/
PDRV_INT32 nPdrvSen_GetGxXist2(PDRV_VOID)
{
    return m_nG1_Xist2;
}

/** PROFIdrive read function for parameter PNU00979 "sensor format"
 *  @details    for structure of PNU00979 see PDRV V4.2 table 102
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00979
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    /* PDRV user should be change the values to his own application! */
    static const PDRV_O4 uPnu00979[31] =
    {
       [ 0U] = 0x00005112U,             /* header see PDRV V4.2 table 103 */
       [ 1U] = 0x80000002U,             /* G1 sensor type see PDRV V4.2 table 104 */
       [ 2U] = 512U,             		/* G1 sensor resolution see PDRV V4.2 table 105 */
       [ 3U] = 11U,                     /* G1 shift factor of Gx_XIST1 */
       [ 4U] = 11U,                     /* G1 shift factor of Gx_XIST2 */
       [ 5U] = 0U,                      /* G1 determinable revolutions */
       [ 6U] = 0U,                      /* G1 reserved for future use */
       [ 7U] = 0U,                      /* G1 reserved for future use */
       [ 8U] = 0U,                      /* G1 reserved for future use */
       [ 9U] = 0U,                      /* G1 reserved for future use */
       [10U] = 0U,                      /* G1 reserved for future use */
       [11U] = 0U,                      /* G2 sensor type see PDRV V4.2 table 104 */
       [12U] = 0U,                      /* G2 sensor resolution see PDRV V4.2 table 105 */
       [13U] = 0U,                      /* G2 shift factor of Gx_XIST1 */
       [14U] = 0U,                      /* G2 shift factor of Gx_XIST2 */
       [15U] = 0U,                      /* G2 determinable revolutions */
       [16U] = 0U,                      /* G2 reserved for future use */
       [17U] = 0U,                      /* G2 reserved for future use */
       [18U] = 0U,                      /* G2 reserved for future use */
       [19U] = 0U,                      /* G2 reserved for future use */
       [20U] = 0U,                      /* G2 reserved for future use */
       [21U] = 0U,                      /* G3 sensor type see PDRV V4.2 table 104 */
       [22U] = 0U,                      /* G3 sensor resolution see PDRV V4.2 table 105 */
       [23U] = 0U,                      /* G3 shift factor of Gx_XIST1 */
       [24U] = 0U,                      /* G3 shift factor of Gx_XIST2 */
       [25U] = 0U,                      /* G3 determinable revolutions */
       [26U] = 0U,                      /* G3 reserved for future use */
       [27U] = 0U,                      /* G3 reserved for future use */
       [28U] = 0U,                      /* G3 reserved for future use */
       [29U] = 0U,                      /* G3 reserved for future use */
       [30U] = 0U,                      /* G3 reserved for future use */
    };
    PDRV_UINT uI;

    for (uI = 0; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->o4[uI] = uPnu00979[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;
}


#endif
