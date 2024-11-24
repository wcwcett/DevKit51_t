/*********************************************************************************************************************/
/**@file        pdrv_sensor_ac4.h
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

#ifndef PDRV_SENSOR_AC4_H
#define PDRV_SENSOR_AC4_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

/** states of the sensor state machine, see PDRV V4.2 chapter 6.3.6.7.1 */
typedef enum
{
    SENSORSTATE_IDLE,                   /**< state after power on */
    SENSORSTATE_NORMAL_OPERATION,       /**< state SD1  normal operation */
    SENSORSTATE_ERROR_ACKNOWLEDGEMENT,  /**< state SD2  error acknowledgement */
    SENSORSTATE_ERROR,                  /**< state SD3  error i.e. error code in Gx_XIST2 */
    SENSORSTATE_REF_VALUE,              /**< state SD4  reference value in Gx_XIST2 */
    SENSORSTATE_WAIT_REF_MARK,          /**< state SD5  wait for reference mark */
    SENSORSTATE_SD6_UNUSED,             /**< state SD6  is unused */
    SENSORSTATE_SET_HOMEPOS,            /**< state SD7  set/shift home position */
    SENSORSTATE_SD8_UNUSED,             /**< state SD8  is unused */
    SENSORSTATE_SD9_UNUSED,             /**< state SD9  is unused */
    SENSORSTATE_WAIT_MEAS_VAL,          /**< state SD10 wait for measured values */
    SENSORSTATE_RDY_MEAS_VAL,           /**< state SD11 measured value in Gx_XIST2 */
    SENSORSTATE_PARKING,                /**< state SD12 parking */
    SENSORSTATE_PARKING_ERROR,          /**< state SD13 parking and error (optional) */
    SENSORSTATE_PARKING_ERROR_ACK,      /**< state SD14 parking and error acknowledgement (optional) */
    SENSORSTATE_STATEMACHINE_ERROR      /**< Error of the Sensor State Machine, last entry! */
} PDRV_SENSORSTATES_TYPE;

/** @name sensor control word Gx_STW bit masks
  * : control bit masks at sensor control word Gx_STW, see PDRV V4.2 table 108
  * @{
*/
#define PDRV_SEN_STW_FUNCX  (0x0FU)     /**< mask for all function bits */
#define PDRV_SEN_STW_FUNC1  (1U << 0U)  /**< 1 = activate function 1 */
#define PDRV_SEN_STW_FUNC2  (1U << 1U)  /**< 1 = activate function 2 */
#define PDRV_SEN_STW_FUNC3  (1U << 2U)  /**< 1 = activate function 3 */
#define PDRV_SEN_STW_FUNC4  (1U << 3U)  /**< 1 = activate function 4 */
#define PDRV_SEN_STW_CMD    (7U << 4U)  /**< Command:<br> 0 = idle,<br> 1 = activate function,<br> 2 = read value,<br> 3 = cancel function */
#define PDRV_SEN_STW_MODE   (1U << 7U)  /**< 0 = reference mark search,<br> 1 = measure on the fly */
#define PDRV_SEN_STW_RES    (7U << 8U)  /**< reserved bits 8, 9 and 10 */
#define PDRV_SEN_STW_HOME_M (1U <<11U)  /**< 0 = set home position (absolute),<br> 1 = shift home position (relative) */
#define PDRV_SEN_STW_HOME_R (1U <<12U)  /**< request set/shift of home position */
#define PDRV_SEN_STW_ABS_R  (1U <<13U)  /**< request absolute value cyclically */
#define PDRV_SEN_STW_PARK   (1U <<14U)  /**< activate parking sensor */
#define PDRV_SEN_STW_ACK    (1U <<15U)  /**< acknowledge a sensor error */
/* @} */

/** @name command values of sensor control word Gx_STW
  * : see PDRV V4.2 table 108
  * @{
*/
#define PDRV_SEN_STW_CMD_IDLE   (0x0000U)   /**< nothing */
#define PDRV_SEN_STW_CMD_ACT    (0x0010U)   /**< activate functions X (specified in PDRV_SEN_STW_FUNCX, PDRV_SEN_STW_MODE) */
#define PDRV_SEN_STW_CMD_READ   (0x0020U)   /**< read values X (specified in PDRV_SEN_STW_FUNCX, PDRV_SEN_STW_MODE) */
#define PDRV_SEN_STW_CMD_CANC   (0x0030U)   /**< cancel functions X */
/* @} */


/** @name sensor status word Gx_ZSW bit masks
  * : status bit masks at sensor status word Gx_ZSW, see PDRV V4.2 table 109
  * @{
*/
#define PDRV_SEN_ZSW_FUNC1  (1U << 0U)  /**< 1 = function 1 active */
#define PDRV_SEN_ZSW_FUNC2  (1U << 1U)  /**< 1 = function 2 active */
#define PDRV_SEN_ZSW_FUNC3  (1U << 2U)  /**< 1 = function 3 active */
#define PDRV_SEN_ZSW_FUNC4  (1U << 3U)  /**< 1 = function 4 active */
#define PDRV_SEN_ZSW_VALUE1 (1U << 4U)  /**< 1 = value 1 available */
#define PDRV_SEN_ZSW_VALUE2 (1U << 5U)  /**< 1 = value 2 available */
#define PDRV_SEN_ZSW_VALUE3 (1U << 6U)  /**< 1 = value 3 available */
#define PDRV_SEN_ZSW_VALUE4 (1U << 7U)  /**< 1 = value 4 available */
#define PDRV_SEN_ZSW_PROBE1 (1U << 8U)  /**< 1 = probe 1 deflected */
#define PDRV_SEN_ZSW_PROBE2 (1U << 9U)  /**< 1 = probe 2 deflected */
#define PDRV_SEN_ZSW_ACK    (1U <<11U)  /**< 1 = error acknowledgement in process */
#define PDRV_SEN_ZSW_HOME   (1U <<12U)  /**< 1 = set/shift of home position is executed  */
#define PDRV_SEN_ZSW_ABS_T  (1U <<13U)  /**< 1 = transmit absolute value cyclically */
#define PDRV_SEN_ZSW_PARK   (1U <<14U)  /**< 1 = parking sensor active */
#define PDRV_SEN_ZSW_ERROR  (1U <<15U)  /**< 1 = sensor error */
/* @} */

/** @name error codes in Gx_XIST2
  * : error codes in Gx_XIST2, see PDRV V4.2 table 107
  * @{
*/
#define PDRV_SEN_ERR_GROUP  (0x0001UL)  /**< sensor group error */
#define PDRV_SEN_ERR_ZERO   (0x0002UL)  /**< zero mark monitoring */
#define PDRV_SEN_ERR_PARK   (0x0003UL)  /**< failure parking sensor */
#define PDRV_SEN_ERR_SEARCH (0x0004UL)  /**< abort reference value search */
#define PDRV_SEN_ERR_RRTRVL (0x0005UL)  /**< abort reference value retrieval */
#define PDRV_SEN_ERR_FLY    (0x0006UL)  /**< abort measurement on the fly */
#define PDRV_SEN_ERR_MRTRVL (0x0007UL)  /**< abort measured value retrieval */
#define PDRV_SEN_ERR_ABS1   (0x0008UL)  /**< abort absolute value transmission */
#define PDRV_SEN_ERR_ABS3   (0x000AUL)  /**< abort absolute value transmission */
#define PDRV_SEN_ERR_CMD    (0x0F01UL)  /**< command not supported */
#define PDRV_SEN_ERR_STATE  (0x1001UL)  /**< sensor state machine error, manufacturer specific */
/* @} */


extern PDRV_VOID PdrvSen_SensorStateMachine(PDRV_VOID);
extern PDRV_SENSORSTATES_TYPE ePdrvSen_GetSensorState(PDRV_VOID);
extern PDRV_INT32 nPdrvSen_Sensor(PDRV_INT32 p_nX);
extern PDRV_INT32 nPdrvSen_GetGxXist2(PDRV_VOID);
extern PDRV_BOOL bPdrvSen_IsSensorError(PDRV_VOID);
extern PDRV_BOOL bPdrvSen_IsGxXist1Valid(PDRV_VOID);
extern PDRV_BOOL bPdrvSen_IsRefValFound(PDRV_VOID);
extern PDRV_BOOL bPdrvSen_IsRefMarkFound(PDRV_VOID);
extern PDRV_BOOL bPdrvSen_IsMeasureActive(PDRV_VOID);

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif
