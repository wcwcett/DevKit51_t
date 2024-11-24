/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
/*  This program is protected by German copyright law and international      */
/*  treaties. The use of this software including but not limited to its      */
/*  Source Code is subject to restrictions as agreed in the license          */
/*  agreement between you and Siemens.                                       */
/*  Copying or distribution is not allowed unless expressly permitted        */
/*  according to your license agreement with Siemens.                        */
/*****************************************************************************/
/*                                                                           */
/*  P r o j e c t         &P: PROFINET IO Runtime Software              :P&  */
/*                                                                           */
/*  P a c k a g e         &W: PROFINET IO Runtime Software              :W&  */
/*                                                                           */
/*  C o m p o n e n t     &C: PnIODDevkits                              :C&  */
/*                                                                           */
/*  F i l e               &F: pndv_com.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  export information                                                       */
/*                                                                           */
/*****************************************************************************/



/*****************************************************************************/
/* contents:

    - COMMON

        - return codes

    - SYSTEM

        - fct ptr for ds-plexer
        - pndv_open( parameter_ptr)
        - ZS Req
        - pndv_version
        - pndv_debug_get_info

        - system interface functions

    - PNDV

        - configuration data for pndv_cfg.c

*/
/*****************************************************************************/
/* reinclude protection */


#ifndef PNDV_COM_H
#define PNDV_COM_H


/*****************************************************************************/
/*****************************************************************************/
/*******************************                 *****************************/
/*******************************     COMMON      *****************************/
/*******************************                 *****************************/
/*****************************************************************************/
/*****************************************************************************/


/*****************************************************************************/
/* return codes */
/*****************************************************************************/

#define PNDV_ERR_FATAL                       ((PNIO_UINT32)0x00)

#define PNDV_OK                              ((PNIO_UINT32)0x01)
#define PNDV_OK_ASYNC                        ((PNIO_UINT32)0x02)
#define PNDV_OK_DONE                         ((PNIO_UINT32)0x03)
#define PNDV_OK_NONE                         ((PNIO_UINT32)0x04)
#define PNDV_OK_CHANGE_ALL                   ((PNIO_UINT32)0x05) /* legacy */
#define PNDV_OK_CHANGE_ID                    ((PNIO_UINT32)0x06) /* legacy */
#define PNDV_OK_WAIT                         ((PNIO_UINT32)0x07) /* legacy */
#define PNDV_OK_SUBSTITUTED                  ((PNIO_UINT32)0x08) /* legacy */

#define PNDV_ERR_MASK                        ((PNIO_UINT32)0x80)
#define PNDV_ERR_TIMER                       ((PNIO_UINT32)0x82)
#define PNDV_ERR_NOT_SUPPORTED               ((PNIO_UINT32)0x83)
#define PNDV_ERR_RESOURCE                    ((PNIO_UINT32)0x84)
#define PNDV_ERR_APPL_RESOURCE               ((PNIO_UINT32)0x85)
#define PNDV_ERR_PARAMETER                   ((PNIO_UINT32)0x86)
#define PNDV_ERR_SEQUENCE                    ((PNIO_UINT32)0x87)

#define PNDV_TO_PERI_OK                      ((PNIO_UINT8)0x01)
#define PNDV_TO_PERI_OK_CANCELLED            ((PNIO_UINT8)0x02)
#define PNDV_TO_PERI_OK_NO_ALARM             ((PNIO_UINT8)0x03)
#define PNDV_TO_PERI_OK_R1_NOT_ACKED         ((PNIO_UINT8)0x04)

#define PNDV_TO_PERI_UNREPORTED              ((PNIO_UINT8)0x11)  /* modul state does not allow a diag yet */


/*****************************************************************************/
/* rqb related */
/*****************************************************************************/

/***********************************************
 * OPCODES
 ***********************************************/
#define PNDV_OPC_NONE                 0
#define PNDV_OPC_TIMEOUT_INTERNAL     1
#define PNDV_OPC_DEVICE_READ          2

#define PNDV_INVALID_HANDLE           0


typedef struct PNDV_ARGS_DEVICE_READ_S
{
    LSA_UINT16                  device_nr; /* device number */

    LSA_UINT32                  data_length; /* req: size of 'data' (including CM_RECORD_OFFSET), cnf: CM_RECORD_OFFSET +number of bytes read */
    CM_COMMON_MEM_U8_PTR_TYPE   data;

    LSA_UINT32                  api;
    LSA_UINT16                  slot_nr;
    LSA_UINT16                  subslot_nr;
    LSA_UINT32                  record_index;  /* see PNIO-Spec */
    CM_UUID_TYPE                target_ar_uuid;

    LSA_UINT32                  pnio_status; /* cnf: pnio error status, see PNIO-Spec */

}PNDV_ARGS_DEVICE_READ_TYPE;

typedef PNDV_ARGS_DEVICE_READ_TYPE * PNDV_ARGS_DEVICE_READ_PTR_TYPE;

/* just for internal use at the moment */
typedef union PNDV_ARGS_U
{
    PNDV_ARGS_DEVICE_READ_TYPE  dev_read;

}PNDV_ARGS_TYPE;

typedef struct PNDV_RQB_S * PNDV_RQB_PTR_TYPE;

typedef PNDV_ARGS_TYPE *  PNDV_ARGS_PTR_TYPE;

typedef struct PNDV_RQB_S
{
    PNDV_RQB_HEADER

    PNDV_ARGS_PTR_TYPE  args;

    PNDV_RQB_TRAILER
}PNDV_RQB_TYPE;


/*****************************************************************************/
/*****************************************************************************/
/*******************************                 *****************************/
/*******************************       PNDV      *****************************/
/*******************************                 *****************************/
/*****************************************************************************/
/*****************************************************************************/

#define PNDV_MAX_PORT                       (pndv_data.cfg.pd.port_count_used)

#define PNDV_INDEX_PATH_CMPDSV_ACP          0
#define PNDV_INDEX_PATH_IOD_CM_ACP          1
#define PNDV_MAX_CM_CHANNEL                 2

#define PNDV_INDEX_PATH_IP2PN_GLO_APP       0
#define PNDV_INDEX_PATH_IP2PN_IF_CM         1
#define PNDV_MAX_IP2PN_CHANNEL              2

#define PNDV_RECORD_MAX_RES                 1      /* rd_in_out.rqb_ds_res_ptr */

#define PNDV_MAX_RECORD_DS1_LEN             7
#define PNDV_MAX_RECORD_GLOB_PARAM          CM_RECORD_OFFSET + sizeof(CM_EVENT_TYPE)+sizeof(CM_RQB_TYPE)+PNDV_MAX_RECORD_DS1_LEN /* 84 + 76 + 28 + 7 = 195 */

#define PNDV_RT_CLASS_1                     0x00000001         /* RT                     */
#define PNDV_RT_CLASS_2                     0x00000002         /* RT                     */
#define PNDV_RT_CLASS_3                     0x00000003         /* IRT                    */
#define PNDV_RT_CLASS_1_UDP                 0x00000004         /* RToverUDP              */
#define PNDV_RT_NONE                        0                  /* no cr running          */

/*****************************************************************************/
/* configuration data for pndv_cfg.c */
/*****************************************************************************/

typedef struct PNDV_GLOB_DATA_S
{
    /* information that come along with CM_OPC_SV_DEVICE_LED_INFO indication */
    PNIO_UINT32                     led_info;
    PNIO_UINT32                     led_maint_info;

    PNIO_UINT32                     sf_on;
    PNIO_UINT32                     maint_on;
    PNIO_UINT32                     save_maint_on;

    PNIO_UINT32                     sync_led_info;

    PNIO_UINT32                     syn_led_on;

} PNDV_GLOB_DATA_T;

extern  PNDV_GLOB_DATA_T  pndv_glob_data;


/*****************************************************************************/
/* coupling_interface data structures */
/*****************************************************************************/
//!@ingroup grPERI
//@{
/**
 *  @brief Peri interface command events
 *
 *  This enum describes a set of commands that can be used to trigger events
 *  or for messaging purposes.
 *  Naming convention:
 *      - PNDV_EV_TO_PERI_xxx - these are commands from the pndv side to the
 *        peripheral side
 *      - PNDV_EV_TO_PNDV_xxx - vice versa
 *      - PNDV_EV_TO_xxx_yyy_DONE - these are confirmations to a requestet service
 *        yyy
 *
 *  The event values are used as command in the peri interface
 *  PNDV_IFACE_CMD_ENTRY_T::cmd as part of the struct PNDV_IFACE_CMD_T::peri_to_pndv
 *  and PNDV_IFACE_CMD_T::pndv_to_peri
 *
 *  see also:
 *   - @ref PNDV_IFACE_STRUCT_S::cmd
 *   - @ref PNDV_IFACE_CMD_S
 */
typedef enum PNDV_EVENTS_E
{
    PNDV_EV_NO                                = 0x0, /**< indicates that there is no event */
    TRC_SUBLABEL_LIST_OPEN("TRC_PNDV_PERI_EVENT")

    /* range from 0x10 to 0x7F /// events from PNDV to PERI */
    PNDV_EV_TO_PERI_PNDV_START_DONE           = 0x10, TRC_EXPLAIN("PERI_PNDV_START_DONE") /**< pndv_start done @ref PNDV_EV_TO_PNDV_PNDV_START */
    PNDV_EV_TO_PERI_PNDV_STOP_DONE            = 0x11, TRC_EXPLAIN("PERI_PNDV_STOP_DONE") /**< pndv_stop done */
    PNDV_EV_TO_PERI_CM_DV_AKT_IND             = 0x12, TRC_EXPLAIN("PERI_CM_DV_AKT_IND") /**< device is now active, pnpb can start plugging modules, */
    PNDV_EV_TO_PERI_PERI_STATE_IND_DONE       = 0x13, TRC_EXPLAIN("PERI_PERI_STATE_IND_DONE") /**< done command to peri state report */
    PNDV_EV_TO_PERI_CM_DV_DEAKT_IND           = 0x14, TRC_EXPLAIN("PERI_CM_DV_DEAKT_IND")

    PNDV_EV_TO_PERI_CONNECT_IND               = 0x20, TRC_EXPLAIN("PERI_CONNECT_IND") /**< indicates the connect of an ar (ar_idx in add_info 1. ar_type in add_info 2), */
    PNDV_EV_TO_PERI_OWN_IND                   = 0x21, TRC_EXPLAIN("PERI_OWN_IND")/**< indicates the ownership of one/last submodule(ar_idx in add_info 1, entity_nr in add_info 2) */
    PNDV_EV_TO_PERI_OWN_IND_MORE_FOLLOWS      = 0x22, TRC_EXPLAIN("PERI_OWN_IND_MORE_FOLLOWS")/**< indicates the ownership of one submodule, more indications will follow (maybe already in cmd queue)(ar_idx in add_info 1, entity_nr in add_info 2) */
                                                      /**< responses must be given in same order as the indications */
    PNDV_EV_TO_PERI_PRM_END_IND               = 0x23, TRC_EXPLAIN("PERI_PRM_END_IND")/**< indicates the end of parametrization of one/last submodule */
    PNDV_EV_TO_PERI_PRM_END_IND_MORE_FOLLOWS  = 0x24, TRC_EXPLAIN("PERI_PRM_END_IND_MORE_FOLLOWS")/**< indicates the end of parametrization of one submodule, more indications will follow */
                                                      /**< def para is indicated with a flag (Bit 3 of set_cfg_flags) (replaces def_para_all/slot) */
    PNDV_EV_TO_PERI_READY_FOR_INPUT_UPDATE    = 0x26, TRC_EXPLAIN("PERI_READY_FOR_INPUT_UPDATE")/**< add_info 1 = ar_idx, ar is ready for input data */
    PNDV_EV_TO_PERI_AR_IN_DATA                = 0x27, TRC_EXPLAIN("PERI_AR_IN_DATA")/**< indicates that the ar is "in data" and outputs are valid (ar_idx in add_info 1, not used add_info 2) */
    PNDV_EV_TO_PERI_AR_DISCONNECT_IND         = 0x29, TRC_EXPLAIN("PERI_AR_DISCONNECT_IND")/**< indicates the disconnection of an ar (ar_idx in add_info 1) */
    PNDV_EV_TO_PERI_AR_ABORT_REQ_DONE         = 0x2A, TRC_EXPLAIN("PERI_AR_ABORT_REQ_DONE")/**< abort done */

    PNDV_EV_TO_PERI_DS_RW                            = 0x30, TRC_EXPLAIN("PERI_DS_RW") /**< indicates the access to data record (ar_idx in add_info 1, index to a subslot entity in add_info 2) */

    PNDV_EV_TO_PERI_PULL_AL_QUIT                     = 0x40, TRC_EXPLAIN("PERI_PULL_AL_QUIT") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PERI_PULL_AL_MORE_FOLLOWS_QUIT        = 0x41, TRC_EXPLAIN("PERI_PULL_AL_MORE_FOLLOWS_QUIT") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PERI_PLUG_AL_QUIT                     = 0x42, TRC_EXPLAIN("PERI_PLUG_AL_QUIT") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PERI_PLUG_AL_MORE_FOLLOWS_QUIT        = 0x43, TRC_EXPLAIN("PERI_PLUG_AL_MORE_FOLLOWS_QUIT") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PERI_SM_IN_DATA                       = 0x44, TRC_EXPLAIN("PERI_SM_IN_DATA")/**< indicates that one/last submodule is "in data" and outputs are valid (ar_idx in add_info 1, entity index in add_info 2) */
    PNDV_EV_TO_PERI_SM_IN_DATA_MORE_FOLLOWS          = 0x45, TRC_EXPLAIN("PERI_SM_IN_DATA_MORE_FOLLOWS")/**< indicates that one submodule is "in data" and outputs are valid, more indications will follow (ar_idx in add_info 1, entity index in add_info 2) */


    PNDV_EV_TO_PERI_CHANNEL_DIAG_QUIT       = 0x50, TRC_EXPLAIN("PERI_CHANNEL_DIAG_QUIT") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PERI_GENERIC_DIAG_QUIT       = 0x51, TRC_EXPLAIN("PERI_GENERIC_DIAG_QUIT") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PERI_PRAL_QUIT               = 0x52, TRC_EXPLAIN("PERI_PRAL_QUIT") /**< index to a subslot entity in add_info 2, rat_val in add_info_1 PNDV_TO_PERI_xx */
    PNDV_EV_TO_PERI_ROS_AL_QUIT             = 0x53, TRC_EXPLAIN("PERI_ROS_AL_QUIT") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PERI_UPAL_QUIT               = 0x54, TRC_EXPLAIN("PERI_UPAL_QUIT") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PERI_EXT_CHANNEL_DIAG_QUIT   = 0x55, TRC_EXPLAIN("PERI_EXT_CHANNEL_DIAG_QUIT") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PERI_URAL_QUIT               = 0x56, TRC_EXPLAIN("PERI_URAL_QUIT") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PERI_STAL_QUIT               = 0x57, TRC_EXPLAIN("PERI_STAL_QUIT") /**< index to a subslot entity in add_info 2, rat_val in add_info_1 PNDV_TO_PERI_xx */

    PNDV_EV_TO_PERI_PS_LOST_QUIT        = 0x60, TRC_EXPLAIN("PERI_PS_LOST_QUIT") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PERI_PS_RETURN_QUIT      = 0x61, TRC_EXPLAIN("PERI_PS_RETURN_QUIT") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PERI_ONESHOT_GC          = 0x62, TRC_EXPLAIN("PERI_ONESHOT_GC") /**< no mandatory information */

    PNDV_EV_TO_PERI_SR_EDGE_IND_DONE        = 0x70, TRC_EXPLAIN("PERI_SR_EDGE_IND_DONE") /**< ar_idx in add_info 1, add_info 2 = response to a backup -> primary (TRUE) or a primary -> backup (FALSE) edge */
    PNDV_EV_TO_PERI_SR_OWNER_TAKEOVER_DONE  = 0x71, TRC_EXPLAIN("PERI_SR_OWNER_TAKEOVER_DONE") /**< ar_idx in add_info 1, add_info 2 = response to taken ownership (TRUE) or to released ownership (FALSE) */
    PNDV_EV_TO_PERI_SR_ALARM_REPORTED_AFTER_SWO_DONE  = 0x72, TRC_EXPLAIN("PERI_SR_ALARM_REPORTED_AFTER_SWO_DONE") /**< */


    /* range from 0x80 to 0xFF /// events from PERI to PNDV */
    PNDV_EV_TO_PNDV_PNDV_START          = 0x80, TRC_EXPLAIN("PNDV_PNDV_START") /**< used to start pndv and indicates all im/virt submods in real_cfg, (add_inf 1 = subslot count of im+virt subslots) */
    PNDV_EV_TO_PNDV_PNDV_STOP           = 0x81, TRC_EXPLAIN("PNDV_PNDV_STOP")
    PNDV_EV_TO_PNDV_PNDV_AKT            = 0x82, TRC_EXPLAIN("PNDV_PNDV_AKT")
    PNDV_EV_TO_PNDV_PNDV_DEAKT          = 0x83, TRC_EXPLAIN("PNDV_PNDV_DEAKT")
    PNDV_EV_TO_PNDV_CM_DV_DEAKT_IND_DONE  = 0x84, TRC_EXPLAIN("PNDV_CM_DV_DEAKT_IND_DONE")
    PNDV_EV_TO_PNDV_CM_DV_AKT_IND_DONE  = 0x85, TRC_EXPLAIN("PNDV_CM_DV_AKT_IND_DONE")
    PNDV_EV_TO_PNDV_PERI_STATE_IND      = 0x86, TRC_EXPLAIN("PNDV_PERI_STATE_IND") /**< reports peri state to pndv (add_info1 = PNDV_PERI_STATE_NOT_OK | Peri not OK, add_info1 = PNDV_PERI_STATE_OK | PERI OK) */

    PNDV_EV_TO_PNDV_CONNECT_IND_DONE               = 0x90, TRC_EXPLAIN("PNDV_CONNECT_IND_DONE") /**< ar_idx in add_info 1, add_info 2 can be used to indicate a negative response to the connect ind */
    PNDV_EV_TO_PNDV_OWN_IND_DONE                   = 0x91, TRC_EXPLAIN("PNDV_OWN_IND_DONE") /**< ar_idx add_info 1, entity add_info 2 */
    PNDV_EV_TO_PNDV_OWN_IND_DONE_MORE_FOLLOWS      = 0x92, TRC_EXPLAIN("PNDV_OWN_IND_DONE_MORE_FOLLOWS") /**< ar_idx add_info 1, entity add_info 2 */
    PNDV_EV_TO_PNDV_PRM_END_IND_DONE               = 0x93, TRC_EXPLAIN("PNDV_PRM_END_IND_DONE") /**< ar_idx add_info 1, entity add_info 2 */
    PNDV_EV_TO_PNDV_PRM_END_IND_DONE_MORE_FOLLOWS  = 0x94, TRC_EXPLAIN("PNDV_PRM_END_IND_DONE_MORE_FOLLOWS") /**< ar_idx add_info 1, entity add_info 2 */
    PNDV_EV_TO_PNDV_READY_FOR_INPUT_UPDATE_DONE    = 0x96, TRC_EXPLAIN("PNDV_READY_FOR_INPUT_UPDATE_DONE") /**< first input update was done, add_info 1 = ar_idx */
    PNDV_EV_TO_PNDV_AR_DISCONNECT_IND_DONE         = 0x97, TRC_EXPLAIN("PNDV_AR_DISCONNECT_IND_DONE") /**< disconnect done (ar_idx in add_info 1) */
    PNDV_EV_TO_PNDV_AR_ABORT_REQ                   = 0x98, TRC_EXPLAIN("PNDV_AR_ABORT_REQ") /**< requests the abort of an ar (ar_idx in add_info 1) */

    PNDV_EV_TO_PNDV_DS_RW_DONE                     = 0xa0, TRC_EXPLAIN("PNDV_DS_RW_DONE") /**< Read or write request has been processed no use off add_info 1 or 2 */

    PNDV_EV_TO_PNDV_PULL_AL                        = 0xb0, TRC_EXPLAIN("PNDV_PULL_AL")/**< indicates that one or last submodule is pulled (add_info 2 = entity ) */
    PNDV_EV_TO_PNDV_PULL_AL_MORE_FOLLOWS           = 0xb1, TRC_EXPLAIN("PNDV_PULL_AL_MORE_FOLLOWS") /**< indicates that one submodule is pulled, more are following (add_info 2 = entity ) */
    PNDV_EV_TO_PNDV_PLUG_AL                        = 0xb2, TRC_EXPLAIN("PNDV_PLUG_AL") /**< indicates that one or last submodule is plugged */
    PNDV_EV_TO_PNDV_PLUG_AL_MORE_FOLLOWS           = 0xb3, TRC_EXPLAIN("PNDV_PLUG_AL_MORE_FOLLOWS") /**< indicates that one submodule is plugged, more are following */
    PNDV_EV_TO_PNDV_SM_IN_DATA_DONE                = 0xb4, TRC_EXPLAIN("PNDV_SM_IN_DATA_DONE") /**< indicates that the in_data ind for one/last submodule has been processed (ar_idx in add_info 1) */
    PNDV_EV_TO_PNDV_SM_IN_DATA_DONE_MORE_FOLLOWS   = 0xb5, TRC_EXPLAIN("PNDV_SM_IN_DATA_DONE_MORE_FOLLOWS") /**< indicates that the in_data ind for one submodule has been processed, more will follow (ar_idx in add_info 1, entity_idx in add_info 2) */


    PNDV_EV_TO_PNDV_CHANNEL_DIAG                   = 0xc0, TRC_EXPLAIN("PNDV_CHANNEL_DIAG") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PNDV_CHANNEL_DIAG_RMV_ALL           = 0xc1, TRC_EXPLAIN("PNDV_CHANNEL_DIAG_RMV_ALL") /**< ar_idx in add_info 1 */
    PNDV_EV_TO_PNDV_GENERIC_DIAG                   = 0xc2, TRC_EXPLAIN("PNDV_GENERIC_DIAG") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PNDV_PRAL                           = 0xc3, TRC_EXPLAIN("PNDV_PRAL") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PNDV_ROS_AL                         = 0xc4, TRC_EXPLAIN("PNDV_ROS_AL") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PNDV_UPAL                           = 0xc5, TRC_EXPLAIN("PNDV_UPAL") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PNDV_EXT_CHANNEL_DIAG               = 0xc6, TRC_EXPLAIN("PNDV_EXT_CHANNEL_DIAG") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PNDV_URAL                           = 0xc9, TRC_EXPLAIN("PNDV_URAL") /**< index to a subslot entity in add_info 2 */
    PNDV_EV_TO_PNDV_STAL                           = 0xca, TRC_EXPLAIN("PNDV_STAL") /**< index to a subslot entity in add_info 2 */

    PNDV_EV_TO_PNDV_PS_LOST                        = 0xd0, TRC_EXPLAIN("PNDV_PS_LOST") /**< index to a subslot entity in add_info 2, needed for isochronous ar */
    PNDV_EV_TO_PNDV_PS_RETURN                      = 0xd1, TRC_EXPLAIN("PNDV_PS_RETURN") /**< index to a subslot entity in add_info 2, needed for isochronous ar */

    PNDV_EV_TO_PNDV_SR_RDHT_TIMEOUT                = 0xe0, TRC_EXPLAIN("PNDV_SR_RDHT_TIMEOUT")   /**< ar_idx in add_info 1, indicates the timeout of sysred data hold timer */
    PNDV_EV_TO_PNDV_SR_EDGE_IND                    = 0xe1, TRC_EXPLAIN("PNDV_SR_EDGE_IND") /**< ar_idx in add_info 1, add_info 2 = indicates a backup -> primary (TRUE) or a primary -> backup (FALSE) edge */
    PNDV_EV_TO_PNDV_SR_OWNER_TAKEOVER_IND          = 0xe2, TRC_EXPLAIN("PNDV_SR_OWNER_TAKEOVER_IND") /**< ar_idx in add_info 1, add_info 2 = indicates to take ownership (TRUE) or to release ownership (FALSE) */
    PNDV_EV_TO_PNDV_SR_ALARM_REPORTED_AFTER_SWO    = 0xe3, TRC_EXPLAIN("PNDV_SR_ALARM_REPORTED_AFTER_SWO") /**<  */

    PNDV_EV_TO_PNDV_IP_SUITE                       = 0xf0, TRC_EXPLAIN("PNDV_IP_SUITE")   /**< device ip suite */
    PNDV_EV_TO_PNDV_NOS                            = 0xf1, TRC_EXPLAIN("PNDV_NOS")   /**< device name of station */

    TRC_SUBLABEL_LIST_CLOSE("TRC_PNDV_PERI_EVENT")

    PNDV_EV_TO_PNDV_NOT_USED                       = 0xff
    /* max value = 0xFF = 255 */
} PNDV_EVENTS_T;
//@}
/*------------------------------------*/

#ifdef PNDV_TRACE_MSG
#define PNDV_TRACE_MSG_EVENTS_STR(_p_deb_str, _deb_code)    \
{                                                            \
    char* _pstr5;                                            \
    switch (_deb_code)                                         \
    {                                                       \
        case PNDV_EV_NO                                         : _pstr5 = "NO EVENT";                        break; \
        case PNDV_EV_TO_PERI_PNDV_START_DONE                    : _pstr5 = "PNDV_START_DONE";                 break; \
        case PNDV_EV_TO_PERI_PNDV_STOP_DONE                     : _pstr5 = "PNDV_STOP_DONE";                 break; \
        case PNDV_EV_TO_PERI_CM_DV_AKT_IND                      : _pstr5 = "CM_DV_AKT_IND";                 break; \
        case PNDV_EV_TO_PERI_PERI_STATE_IND_DONE                : _pstr5 = "PERI_STATE_IND_DONE";             break; \
        case PNDV_EV_TO_PERI_CONNECT_IND                        : _pstr5 = "CONNECT_IND";                     break; \
        case PNDV_EV_TO_PERI_OWN_IND                            : _pstr5 = "OWN_IND";                         break; \
        case PNDV_EV_TO_PERI_OWN_IND_MORE_FOLLOWS               : _pstr5 = "OWN_IND_MORE_FOLLOWS";             break; \
        case PNDV_EV_TO_PERI_PRM_END_IND                        : _pstr5 = "PRM_END_IND";                     break; \
        case PNDV_EV_TO_PERI_PRM_END_IND_MORE_FOLLOWS           : _pstr5 = "PRM_END_IND_MORE_FOLLOWS";         break; \
        case PNDV_EV_TO_PERI_READY_FOR_INPUT_UPDATE             : _pstr5 = "READY_FOR_INPUT_UPDATE";         break; \
        case PNDV_EV_TO_PERI_AR_IN_DATA                         : _pstr5 = "AR_IN_DATA";                     break; \
        case PNDV_EV_TO_PERI_AR_DISCONNECT_IND                  : _pstr5 = "AR_DISCONNECT_IND";             break; \
        case PNDV_EV_TO_PERI_AR_ABORT_REQ_DONE                  : _pstr5 = "AR_ABORT_REQ_DONE";             break; \
        case PNDV_EV_TO_PERI_DS_RW                              : _pstr5 = "DS_RW";                         break; \
        case PNDV_EV_TO_PERI_PULL_AL_QUIT                       : _pstr5 = "PULL_AL_QUIT";                     break; \
        case PNDV_EV_TO_PERI_PULL_AL_MORE_FOLLOWS_QUIT          : _pstr5 = "PULL_AL_MORE_FOLLOWS_QUIT";     break; \
        case PNDV_EV_TO_PERI_PLUG_AL_QUIT                       : _pstr5 = "PLUG_AL_QUIT";                     break; \
        case PNDV_EV_TO_PERI_PLUG_AL_MORE_FOLLOWS_QUIT          : _pstr5 = "PLUG_AL_MORE_FOLLOWS_QUIT";     break; \
        case PNDV_EV_TO_PERI_SM_IN_DATA                         : _pstr5 = "SM_IN_DATA";                     break; \
        case PNDV_EV_TO_PERI_SM_IN_DATA_MORE_FOLLOWS            : _pstr5 = "SM_IN_DATA_MORE_FOLLOWS";         break; \
        case PNDV_EV_TO_PERI_CHANNEL_DIAG_QUIT                  : _pstr5 = "CHANNEL_DIAG_QUIT";             break; \
        case PNDV_EV_TO_PERI_GENERIC_DIAG_QUIT                  : _pstr5 = "GENERIC_DIAG_QUIT";             break; \
        case PNDV_EV_TO_PERI_PRAL_QUIT                          : _pstr5 = "PRAL_QUIT";                     break; \
        case PNDV_EV_TO_PERI_ROS_AL_QUIT                        : _pstr5 = "ROS_AL_QUIT";                     break; \
        case PNDV_EV_TO_PERI_UPAL_QUIT                          : _pstr5 = "UPAL_QUIT";                     break; \
        case PNDV_EV_TO_PERI_EXT_CHANNEL_DIAG_QUIT              : _pstr5 = "EXT_CHANNEL_DIAG_QUIT";         break; \
        case PNDV_EV_TO_PERI_URAL_QUIT                          : _pstr5 = "URAL_QUIT";                        break; \
        case PNDV_EV_TO_PERI_STAL_QUIT                          : _pstr5 = "STAL_QUIT";                        break; \
        case PNDV_EV_TO_PERI_PS_LOST_QUIT                       : _pstr5 = "PS_LOST_QUIT";                     break; \
        case PNDV_EV_TO_PERI_PS_RETURN_QUIT                     : _pstr5 = "PS_RETURN_QUIT";                 break; \
        case PNDV_EV_TO_PERI_ONESHOT_GC                         : _pstr5 = "ONESHOT_GC";                     break; \
        case PNDV_EV_TO_PERI_SR_EDGE_IND_DONE                   : _pstr5 = "SR_EDGE_IND_DONE";                 break; \
        case PNDV_EV_TO_PNDV_PNDV_START                         : _pstr5 = "PNDV_START";                     break; \
        case PNDV_EV_TO_PNDV_PNDV_STOP                          : _pstr5 = "PNDV_STOP";                     break; \
        case PNDV_EV_TO_PNDV_CM_DV_AKT_IND_DONE                 : _pstr5 = "CM_DV_AKT_IND_DONE";             break; \
        case PNDV_EV_TO_PNDV_PERI_STATE_IND                     : _pstr5 = "PERI_STATE_IND";                 break; \
        case PNDV_EV_TO_PNDV_CONNECT_IND_DONE                   : _pstr5 = "CONNECT_IND_DONE";                 break; \
        case PNDV_EV_TO_PNDV_OWN_IND_DONE                       : _pstr5 = "OWN_IND_DONE";                     break; \
        case PNDV_EV_TO_PNDV_OWN_IND_DONE_MORE_FOLLOWS          : _pstr5 = "OWN_IND_DONE_MORE_FOLLOWS";     break; \
        case PNDV_EV_TO_PNDV_PRM_END_IND_DONE                   : _pstr5 = "PRM_END_IND_DONE";                 break; \
        case PNDV_EV_TO_PNDV_PRM_END_IND_DONE_MORE_FOLLOWS      : _pstr5 = "PRM_END_IND_DONE_MORE_FOLLOWS"; break; \
        case PNDV_EV_TO_PNDV_READY_FOR_INPUT_UPDATE_DONE        : _pstr5 = "READY_FOR_INPUT_UPDATE_DONE";     break; \
        case PNDV_EV_TO_PNDV_AR_DISCONNECT_IND_DONE             : _pstr5 = "AR_DISCONNECT_IND_DONE";         break; \
        case PNDV_EV_TO_PNDV_AR_ABORT_REQ                       : _pstr5 = "AR_ABORT_REQ";                     break; \
        case PNDV_EV_TO_PNDV_DS_RW_DONE                         : _pstr5 = "DS_RW_DONE";                    break; \
        case PNDV_EV_TO_PNDV_PULL_AL                            : _pstr5 = "PULL_AL";                         break; \
        case PNDV_EV_TO_PNDV_PULL_AL_MORE_FOLLOWS               : _pstr5 = "PULL_AL_MORE_FOLLOWS";             break; \
        case PNDV_EV_TO_PNDV_PLUG_AL                            : _pstr5 = "PLUG_AL";                         break; \
        case PNDV_EV_TO_PNDV_PLUG_AL_MORE_FOLLOWS               : _pstr5 = "PLUG_AL_MORE_FOLLOWS";             break; \
        case PNDV_EV_TO_PNDV_SM_IN_DATA_DONE                    : _pstr5 = "SM_IN_DATA_DONE";                 break; \
        case PNDV_EV_TO_PNDV_SM_IN_DATA_DONE_MORE_FOLLOWS       : _pstr5 = "SM_IN_DATA_DONE_MORE_FOLLOWS";     break; \
        case PNDV_EV_TO_PNDV_CHANNEL_DIAG                       : _pstr5 = "CHANNEL_DIAG";                     break; \
        case PNDV_EV_TO_PNDV_CHANNEL_DIAG_RMV_ALL               : _pstr5 = "CHANNEL_DIAG_RMV_ALL";             break; \
        case PNDV_EV_TO_PNDV_GENERIC_DIAG                       : _pstr5 = "GENERIC_DIAG";                     break; \
        case PNDV_EV_TO_PNDV_PRAL                               : _pstr5 = "PRAL";                             break; \
        case PNDV_EV_TO_PNDV_ROS_AL                             : _pstr5 = "ROS_AL";                         break; \
        case PNDV_EV_TO_PNDV_UPAL                               : _pstr5 = "UPAL";                             break; \
        case PNDV_EV_TO_PNDV_EXT_CHANNEL_DIAG                   : _pstr5 = "EXT_CHANNEL_DIAG";                 break; \
        case PNDV_EV_TO_PNDV_URAL                               : _pstr5 = "URAL";                             break; \
        case PNDV_EV_TO_PNDV_STAL                               : _pstr5 = "STAL";                             break; \
        case PNDV_EV_TO_PNDV_PS_LOST                            : _pstr5 = "PS_LOST";                         break; \
        case PNDV_EV_TO_PNDV_PS_RETURN                          : _pstr5 = "PS_RETURN";                     break; \
        case PNDV_EV_TO_PNDV_SR_RDHT_TIMEOUT                    : _pstr5 = "SR_RDHT_TIMEOUT";                 break; \
        case PNDV_EV_TO_PNDV_SR_EDGE_IND                        : _pstr5 = "SR_EDGE_IND";                     break; \
        case PNDV_EV_TO_PNDV_NOT_USED                           : _pstr5 = "NOT USED";                         break; \
        default                                                 : _pstr5 = "UNDEFINED";                      break; \
    }                        \
    *_p_deb_str = _pstr5;     \
}
#endif /* PNDV_TRACE_MSG */


/**
 * @brief service state of interface
 *
 */
typedef enum
{
    PNDV_IFACE_SERVICE_NEW         = 1, //!< set by initiator to indicate a new job
    PNDV_IFACE_SERVICE_PROCCESSING    , //!< set by receiver to indicate processing of a job
    PNDV_IFACE_SERVICE_IDLE             //!< set by initiator to indicate finishing of job
} PNDV_IFACE_SERVICE;

/* set-actual comparison */
/*------------------------------------*/

typedef enum
{
    TRC_SUBLABEL_LIST_OPEN("TRC_PNDV_CMP_RES")
    PNDV_CMP_RES_OK                = 1, TRC_EXPLAIN("CMP_RES: OK")
    PNDV_CMP_RES_OK_CHANGE         = 2, TRC_EXPLAIN("CMP_RES: OK CHANGE")
    PNDV_CMP_RES_OK_SUBSTITUTED    = 3, TRC_EXPLAIN("CMP_RES: OK SUBSTITUDE")
    PNDV_CMP_RES_NO_MODULE         = 4, TRC_EXPLAIN("CMP_RES: NO MODULE")
    PNDV_CMP_RES_ERROR             = 128, TRC_EXPLAIN("CMP_RES: ERROR")
    TRC_SUBLABEL_LIST_CLOSE("TRC_PNDV_CMP_RES")
    PNDV_CMP_RES_NOT_USED          = 255
} PNDV_CMP_RES;

/* peri state */
/*------------------------------------*/

#define    PNDV_PERI_STATE_NOT_OK         (PNIO_UINT8)0
#define    PNDV_PERI_STATE_OK             (PNIO_UINT8)1

/* ioxs values */
/*------------------------------------*/

#define PNDV_IOXS_GOOD                  0x80
#define PNDV_IOXS_BAD_BY_DEVICE         0x40
#define PNDV_IOXS_BAD_BY_CONTROLLER     0x60
#define PNDV_IOXS_UNDETERMINED          0xFF

/* alarms */
/*------------------------------------*/

#define PNDV_AL_FREE       0
#define PNDV_AL_SENT       1

#define PNDV_AL_SPEC_KEINE_ANGABE       0
#define PNDV_AL_SPEC_KOMMEND            1
#define PNDV_AL_SPEC_GEHEND             2
#define PNDV_AL_SPEC_GEHEND_GESTOERT    3

typedef enum PNDV_AL_USI_E
{
    PNDV_AL_USI_UPDATEALARM                 = 0x0050,
    PNDV_AL_USI_PROCESSALARM                = 0,    /* !!! ??? nothing found in white paper */
    PNDV_AL_USI_RETURNOFSUBMODULALARM       = 0,    /* !!! ??? nothing found in white paper */
    PNDV_AL_USI_GENERIC_DIAG                = 1,    /*  not actual an own USI, but for a consistant interface ... */
    PNDV_AL_USI_CHANNELDIAG                 = 0x8000,
    PNDV_AL_USI_EXTCHANNELDIAG              = 0x8002,
    PNDV_AL_USI_UPLOAD_RETRIVAL             = 0x8200,
    PNDV_AL_USI_IPARAMETER                  = 0x8201

} PNDV_AL_USI_T;

/*------------------------------------*/

typedef struct
{
    CM_RQB_TYPE     rqb;
    CM_EVENT_TYPE   event_type;
    PNIO_UINT8      record_data[CM_RECORD_OFFSET + PNDV_MAX_RECORD_DATA_LEN];

} PNDV_RQB_DS;

typedef PNDV_RQB_DS * PNDV_RQB_DS_PTR;

/*------------------------------------*/

typedef struct
{
    CM_RQB_TYPE     rqb;
    CM_EVENT_TYPE   event_type;
    PNIO_UINT8      header[CM_RECORD_OFFSET];
    PNIO_UINT8      data[PNDV_MAX_RECORD_DS1_LEN];

} PNDV_RQB_DS1;

typedef PNDV_RQB_DS1 * PNDV_RQB_DS1_PTR;

/*------------------------------------*/

typedef struct PNDV_CHANNEL_DIAG_S
{
    PNIO_UINT8   alm_kommend;
    PNIO_UINT8   kennung;
    PNIO_UINT16  kanal;
    PNIO_UINT16  properties;
    PNIO_UINT16  fehler;
#ifdef PNDV_CFG_USE_USERTAG
    PNIO_UINT32  diag_tag;
#endif
} PNDV_CHANNEL_DIAG_T;

/*------------------------------------*/

typedef struct PNDV_CHANNEL_XDIAG_S
{
    PNIO_UINT8   alm_kommend;
    PNIO_UINT8   kennung;
    PNIO_UINT16  kanal;
    PNIO_UINT16  properties;
    PNIO_UINT16  fehler;
    PNIO_UINT16  ext_fehler;
    PNIO_UINT32  ext_wert;
#ifdef PNDV_CFG_USE_USERTAG
    PNIO_UINT32  diag_tag;
#endif
} PNDV_CHANNEL_XDIAG_T;

/* ---------------------------------------------------------------------------
// ChannelProperties, see PNIO-Spec
// Bit  0 -  7: ChannelProperties.Type
// Bit  8     : ChannelProperties.Accumulative
// Bit  9     : ChannelProperties.MaintenanceRequired
// Bit 10     : ChannelProperties.MaintenanceDemanded
// Bit 11 - 12: ChannelProperties.Specifier
// Bit 13 - 15: ChannelProperties.Direction
//
// ChannelProperties.Specifier
// 0x00 reserved
// 0x01 error appears
// 0x02 error disappears and error free
// 0x03 error disappears but other errors remain
//---------------------------------------------------------------------------*/

#define PNDV_SV_DIAG_CHANNEL_PROPERTIES_MAKE(type, spec, dir)               \
   ((LSA_UINT16)                                                            \
    ( (((type)  & 0xFF) <<  0) /* mask 0x00FF */                            \
    | (((spec)  & 0x03) << 11) /* mask 0x1800 */                            \
    | (((dir)   & 0x07) << 13) /* mask 0xE000 */                            \
    ))


///interface data used for issuing a generic diag
typedef struct PNDV_GENERIC_DIAG_S
{
    PNIO_UINT16          interface_index;
    PNIO_UINT16          slot;
    PNIO_UINT16          subslot;
    PNIO_UINT32          api;
    PNIO_UINT8           alm_kommend;
    PNIO_UINT16          usi;
    PNIO_UINT32          diag_tag; /* the diag tag must be provided by the PNPB */
    PNIO_UINT16          length;
    PNIO_UINT8*          data;
    // PNIO_UINT16          properties; // &&&2do  for DK4.0
    // PNIO_UINT16          channel;    // &&&2do  for DK4.0
    PNDV_IFACE_SERVICE  state;

} PNDV_GENERIC_DIAG_T;


/*------------------------------------*/

typedef struct PNDV_PRAL_INFO_S
{
    PNIO_UINT8  data[PNDV_AL_PRAL_INFO_MAX_LEN];
    PNIO_UINT16 data_length;
    PNIO_UINT16 usi;
} PNDV_PRAL_INFO_T;

typedef PNDV_PRAL_INFO_T * PNDV_SENDMSG_PRALINFO_PTR;

/*------------------------------------*/

typedef struct PNDV_UPAL_INFO_S
{
    PNIO_UINT8 data[PNDV_AL_UPAL_INFO_LEN];
} PNDV_UPAL_INFO_T;

typedef PNDV_UPAL_INFO_T * PNDV_SENDMSG_UPALINFO_PTR;

/*------------------------------------*/

typedef struct PNDV_URAL_INFO_S
{
    PNIO_UINT8     data[PNDV_AL_URAL_INFO_LEN];
    PNIO_UINT16    usi;
} PNDV_URAL_INFO_T;

typedef PNDV_URAL_INFO_T * PNDV_SENDMSG_URALINFO_PTR;

/*------------------------------------*/

typedef struct PNDV_STAL_INFO_S
{
    PNIO_UINT8     data[PNDV_AL_STAL_INFO_LEN];
    PNIO_UINT16    ar_idx;
    PNIO_UINT16    usi;
} PNDV_STAL_INFO_T;

typedef PNDV_STAL_INFO_T * PNDV_SENDMSG_STALINFO_PTR;

/*------------------------------------*/


typedef struct PNDV_REAL_CFG_ELEMENT_S
{
    CM_SV_SUBMODULE_ADD_ELEMENT_TYPE  sub_descr;
    PNDV_IFACE_SERVICE                state;

}   PNDV_REAL_CFG_ELEMENT_T;

typedef struct PNDV_REAL_DIAL_DATA_S
{
    PNDV_CHANNEL_DIAG_T         chn_diag[PNDV_MAX_CHN_DIAG_PER_SLOT_AND_MOMENT];
    PNIO_UINT32                 anz_chn_diag;

    PNDV_IFACE_SERVICE          state;
} PNDV_REAL_DIAL_DATA_T;


typedef struct PNDV_REAL_XDIAL_DATA_S
{
    PNDV_CHANNEL_XDIAG_T        ext_diag[PNDV_MAX_EXT_DIAG_PER_SLOT_AND_MOMENT];
    PNIO_UINT32                 anz_chn_diag;

    PNDV_IFACE_SERVICE          state;
} PNDV_REAL_XDIAL_DATA_T;

typedef struct PNDV_REAL_UPAL_DATA_S
{
    PNDV_UPAL_INFO_T            info;
    PNDV_IFACE_SERVICE          state;
} PNDV_REAL_UPAL_DATA_T;

typedef struct PNDV_REAL_URAL_DATA_S
{
    PNDV_URAL_INFO_T            info;
    PNDV_IFACE_SERVICE          state;
} PNDV_REAL_URAL_DATA_T;

typedef struct PNDV_REAL_STAL_DATA_S
{
    PNDV_STAL_INFO_T            info;
    PNDV_IFACE_SERVICE          state;
} PNDV_REAL_STAL_DATA_T;

typedef struct PNDV_REAL_GEN_DIAL_DATA_S
{
    PNDV_GENERIC_DIAG_T         info;
    PNDV_IFACE_SERVICE          state;
} PNDV_REAL_GEN_DIAL_DATA_T;

typedef struct PNDV_REAL_PRAL_DATA_S
{
    PNDV_PRAL_INFO_T            info;
    PNDV_IFACE_SERVICE          state;
#ifdef PNDV_CFG_USE_USERTAG
    PNIO_UINT32                 diag_tag;
#endif
} PNDV_REAL_PRAL_DATA_T;

typedef struct PNDV_REAL_ROS_S
{
    PNDV_IFACE_SERVICE          state;
} PNDV_REAL_ROS_T;

typedef struct PNDV_REAL_FLAGS_S
{
    PNIO_UINT32                  fdata;   //!< Bit0 = [0=parameter not configurable|1=parameter configurabele]
                                         //!< Bit1 =
}PNDV_REAL_FLAGS_T;


typedef struct PNDV_REAL_CFG_S
{
    PNDV_REAL_CFG_ELEMENT_T             elem;
    PNDV_REAL_DIAL_DATA_T               dial;
    PNDV_REAL_XDIAL_DATA_T              xdial;
    PNDV_REAL_UPAL_DATA_T               upal;
    PNDV_REAL_URAL_DATA_T               ural;
    PNDV_REAL_STAL_DATA_T               stal;
    PNDV_REAL_PRAL_DATA_T               pral;
    PNDV_REAL_ROS_T                     ros;
    PNDV_REAL_FLAGS_T                   flags;
} PNDV_REAL_CFG_T;

/*------------------------------------*/

/*------------------------------------------------------------------------------
// circularly linked list macros
//---------------------------------------------------------------------------*/

struct pndv_list_entry_tag {
    struct pndv_list_entry_tag  * Flink; /* forward link */
    struct pndv_list_entry_tag  * Blink; /* backward link */
};

typedef struct pndv_list_entry_tag  PNDV_LIST_ENTRY_TYPE;

typedef struct pndv_list_entry_tag   *PNDV_LIST_ENTRY_PTR_TYPE;


//!@ingroup grPERI
//@{
typedef struct PNDV_SET_CFG_SUBMODULE_S
{
    PNIO_BOOL                         ownership_passed;  //!< TRUE if this element got an ownership once. FALSE otherwise.
                                                         //   Can be used to detect, if subslot is projected.
    CM_SV_AR_OWNERSHIP_ELEMENT_TYPE   own_info;

    PNDV_CMP_RES                      cmp_result;

    PNDV_IFACE_SERVICE                own_state;
    PNDV_IFACE_SERVICE                prm_end_state;

}   PNDV_SET_CFG_ELEMENT_T;


#define PNDV_SET_CFG_FLAGS_OWN              0x00000001
#define PNDV_SET_CFG_FLAGS_IOXS_OVERWRITE   0x00000002
#define PNDV_SET_CFG_FLAGS_IOXS_GOOD        0x00000004
#define PNDV_SET_CFG_FLAGS_DO_DEF_PARA      0x00000008   //!< legacy not used anymore
#define PNDV_SET_CFG_FLAGS_SET_ARP          0x00000010   //!
#define PNDV_SET_CFG_FLAGS_MRP_OFF          0x00000020



typedef struct PNDV_SET_CFG_S
{
    PNDV_SET_CFG_ELEMENT_T          elem;

    PNIO_UINT32                     flags; //!< Bit0 = [0=not owner|1=owner]
                                           //!< ***** option handling *****
                                           //!< Bit1 = [0=no ioxs overwrite|1=ioxs overwrite]
                                           //!< Bit2 = if (Bit1 = 1)[0=ioxs bad|1=ioxs good]
                                           //!< ***** default para *****
                                           //!< Bit3 = [0=no def para|1=do def para]
                                           //!< ***** ready pending *****
                                           //!< Bit4 = [0=ready|1=pending]
                                           //!< **** set_mrp off ***
                                           //!< Bit5 = [0=MRP_ON|1=MRP_OFF] only evaluated for PNDV_IM_SLOT_NO


}   PNDV_SET_CFG_T;

/*------------------------------------*/

typedef struct PNDV_IFACE_CMD_ENTRY_S
{
    PNIO_UINT8       cmd;    /**< use commands acording @ref PNDV_EVENTS_E */
    PNIO_UINT8       add_1;
    PNIO_UINT16      add_2;
    // PNIO_VOID*      pCmdProp;   // &&&2do user defined handle, here: pointer  &&&2do  for DK4.0
}PNDV_IFACE_CMD_ENTRY_T;

typedef struct PNDV_IFACE_CMD_S
{
    /* events from peripheral bus to profinet stack */
    PNDV_IFACE_CMD_ENTRY_T      peri_to_pndv[PNDV_IFACE_CMD_CNT];

    PNIO_UINT32                 read_ptr_peri_to_pndv;               
    PNIO_UINT32                 write_ptr_peri_to_pndv;

    /* events from profinet stack to peripheral bus */
    PNDV_IFACE_CMD_ENTRY_T      pndv_to_peri[PNDV_IFACE_CMD_CNT];

    PNIO_UINT32                 read_ptr_pndv_to_peri;
    PNIO_UINT32                 write_ptr_pndv_to_peri;
} PNDV_IFACE_CMD_T;

/*------------------------------------*/

/* Begin: PNDV_IFACE_STRUCT */
typedef struct PNDV_IFACE_STRUCT_S
{

    PNDV_IFACE_CMD_T                cmd;        /**< command interface */

    PNDV_REAL_CFG_T                 real_cfg[PNDV_MAX_SV_ENTITY]; /**< information regarding the available submodule configuration */

    PNDV_SET_CFG_T                  set_cfg[PNDV_CM_AR_NO][PNDV_CM_SV_SUBSLOT_COUNT];  /**< information regarding the requested submodule configuration
                                                                                            array size is derived from the maximum number of projectable subslots per AR. */

    PNDV_GENERIC_DIAG_T             generic_diag_data[ PNDV_MAX_GENERIC_DIAG_NUMBER ]; /**< information regarding diagnosis of submodules */

    struct
    {
        CM_UPPER_RQB_PTR_TYPE       ptr;
        CM_UPPER_RQB_PTR_TYPE       ptr_host;
        PNDV_IFACE_SERVICE          state;

        PNDV_RQB_DS                 dfc_ds_struc_peri;
        PNDV_RQB_DS                 pndv_rqb_ds_res[PNDV_RECORD_MAX_RES];
        PNIO_BOOL                   suppress_error_if_para_ds;

    } ds_rw; //!< record interface

    struct
    {
        PNDV_IFACE_SERVICE          lost_state[PNDV_MAX_SV_ENTITY+1];
        PNDV_IFACE_SERVICE          return_state[PNDV_MAX_SV_ENTITY+1];
    } ps; 

    struct
    {
        PNIO_UINT32                  previous_rt_class;      /* needed for internal decisions */
        PNIO_UINT32                  current_rt_class;       /* tells rt class for slave processor */
        PNIO_UINT32                  sr_rdht_msec;           /* redundancy data hold time, in milliseconds */
        PNIO_UINT32                  sr_properties;          /* see CM_SR_PROP_INP_VALID_ON_BACKUP_AR, etc. */
        PNIO_UINT16                  sr_ar_set_nr;           /* number of the ar set a sysred ar belongs to */
        PNIO_UINT32                  sr_firstAR;
        CM_UUID_TYPE                 cmi_obj_uuid;           /* CMInitiatorObjectUUID to identify the connection (not the ar) */
        PNIO_UINT32                  host_ip;                /* network byte order, ip-address of IOC */
        PNIO_UINT8*                  host_name;              /* zero-terminated string */
        PNIO_BOOL                    red_int_info_used;      /**< if true, red_int_info is already filled up */
        PNIO_UINT16                  ar_session_key;         /* holds the AR session key, can be used by upper components for key->AR index mapping*/
        PNIO_UINT16                  input_cr_io_data_length;/**< sum of SubmoduleDataLength of all submodules within this CR */
        PNIO_UINT16                  input_cr_ioxs_length;   /**< sum of IOPS/IOCS of all submodules contained in this CR. DiscardIOXS not considered! */
        PNIO_UINT16                  output_cr_io_data_length;
        PNIO_UINT16                  output_cr_ioxs_length;
        PNIO_UINT8                   ar_fsu_enable;          /* fsu enabled if true */
        PNIO_UINT8                   ar_fsu_uuid[16];        /* onyl valid if ar_fsu_enable == true */
#ifdef PNDV_CFG_ENABLE_RS_INFO_BLOCK_SUPPORT
        PNIO_UINT32                  has_RSInfoBlock;        /* AR has RS info block */
#endif
        struct
        {
            PNIO_UINT32   RxEndNs;
            PNIO_UINT32   RxLocalTransferEndNs;
            PNIO_UINT32   TxEndNs;
            PNIO_UINT32   TxLocalTransferStartNs;
            PNIO_UINT32   TxLocalTransferEndNs;
        }red_int_info;/**< information filled up for rtc3 ar with RIR_IND (only newer PNIO base), PNDV_CFG_USE_RED_INT_INFO needs to be defined */

        PNIO_UINT16                  send_clock;
        PNIO_UINT16                  reduction_ratio[PNDV_MAX_IOCR_PER_AR];
    } ar[PNDV_CM_AR_NO];  /**< information regarding application relations (AR) */

    struct 
    {
        PNIO_UINT32 ip_address;
        PNIO_UINT32 netmask;
        PNIO_UINT32 gateway;
        PNIO_UINT8  mk_remanent;
    }ip_suite; /**< information regarding ip suite of device */

    struct 
    {
        const PNIO_VOID* pStationName; 
        PNIO_UINT16  StationNameLen;
    }name_of_station; /**< information regarding name of station of device */

} PNDV_IFACE_STRUCT;

typedef PNDV_IFACE_STRUCT * PNDV_IFACE_STRUCT_PTR;
//@}
/* End: PNDV_IFACE_STRUCT */


/*****************************************************************************/
/*****************************************************************************/
/*******************************                 *****************************/
/*******************************      SYSTEM     *****************************/
/*******************************                 *****************************/
/*****************************************************************************/
/*****************************************************************************/


/*****************************************************************************/
/* system service */
/*  -> pndv_open( parameter_ptr) */
/*****************************************************************************/

typedef struct PNDV_PD_CFG_S
{
    LSA_UINT16  interface_subslot_nr;
    LSA_UINT16  port_count_used;

    LSA_UINT32  im_mod_ident;

    struct
    {
        LSA_UINT32  submod_id;
        LSA_UINT8   im_0_bits;
    } port[PNDV_CFG_MAX_PORT_CNT+1];

} PNDV_PD_CFG_S_T;

typedef struct
{
    struct  /* open parameter cm interface */
    {
        LSA_SYS_PATH_TYPE       sys_path_cm[PNDV_MAX_CM_CHANNEL];        /* index see  PNDV_OPEN_CHANNEL_TYPE */
        LSA_UINT32              *annotation_ptr;
        LSA_UINT32              *oem_data_ptr;
        LSA_UINT16              device_id;
    } cm;

    struct /* open parameter ip2pn interface */
    {
        LSA_SYS_PATH_TYPE       sys_path_ip2pn[PNDV_MAX_IP2PN_CHANNEL];
    } ip2pn;

    PNDV_PD_CFG_S_T             pd;

    struct  /* open parameter system embedding */
    {
        PNDV_RQB_DS_PTR         rqb_ds_res_ptr[PNDV_RECORD_MAX_RES];  /* PNDV_RECORD_MAX_RES DS ressources incl. netto data */

    } sys;

    struct  /* open parameter to handle station parameter via apma operation */
    {
        PNDV_RQB_DS1_PTR        prm_ptr;

    } rema_station;

} PNDV_STRUC_PARAMETER;

typedef PNDV_STRUC_PARAMETER * PNDV_SYS_PARAMETER_PTR;


/*****************************************************************************/
/* ZS Req */
/*****************************************************************************/

typedef enum
{
    PNDV_AL_NIX_REQ  = 0,
    PNDV_AL_PULL_REQ,
    PNDV_AL_BREAK_REQ,
    PNDV_AL_BREAK_REQ_NO_AL,
    PNDV_AL_PLUG_REQ,
    PNDV_AL_RECURRENCE_REQ,
    PNDV_AL_RECURRENCE_REQ_NO_AL,
    PNDV_AL_RECURRENCE_BUT_PULL_REQ,
    PNDV_AL_CHK_RSP_REQ,

    PNDV_AL_PLUG_REQ_PNDV,
    PNDV_AL_PULL_REQ_PNDV,

}   PNDV_AL_PP_REQ;

typedef enum
{
    PNDV_DEBUG_OFF            = 0,    /* logging off */
    PNDV_DEBUG_ON_ALL                 /* logging on */

} PNDV_DEBUG_MODE_TYPE;

/*****************************************************************************/
/* system service  */
/*  -> pndv_version( version_len, version_ptr) */
/*****************************************************************************/

typedef LSA_VERSION_TYPE * PNDV_SYS_VERSION_PTR;

#define PNDV_LSA_COMPONENT_ID                   PNIO_PACKID_PNDV
#define PNDV_LSA_PREFIX                         "   -PNDV "
#define PNDV_KIND                               /* &K */ 'V'  /* K& */
                                                /* preliminary: 'R': release       */
                                                /*              'C': correction    */
                                                /*              'S': spezial       */
                                                /*              'T': test          */
                                                /*              'B': labor         */
                                                /* prereleased: 'P': pilot         */
                                                /* released:    'V': version       */
                                                /*              'K': correction    */
                                                /*              'D': demonstration */
#define PNDV_VERSION                            /* &V */ 3    /* V& */ /* [1 - 99] */
#define PNDV_DISTRIBUTION                       /* &D */ 71   /* D& */ /* [0 - 99] */
#define PNDV_FIX                                /* &F */ 07   /* F& */ /* [0 - 99] */
#define PNDV_HOTFIX                             /* &H */ 0    /* H& */ /* [0]      */
#define PNDV_PROJECT_NUMBER                     /* &P */ 0    /* P& */ /* [0 - 99] */
                                                             /* At LSA always 0!  */
#define PNDV_INCREMENT                          /* &I */ 0    /* I& */ /* [1 - 99] */
#define PNDV_INTEGRATION_COUNTER                /* &C */ 0    /* C& */ /* [1 - 99] */
#define PNDV_GEN_COUNTER                        /* &G */ 0    /* G& */ /* [1]      */


/*****************************************************************************/
/* system service  */
/*  -> pndv_debug_get_info( debug_info_ptr) */
/*****************************************************************************/

typedef struct
{
    PNIO_UINT8*  trace_data_ptr;
    PNIO_UINT32  trace_data_len;

    PNIO_UINT8*  local_data_ptr;
    PNIO_UINT32  local_data_len;

}   PNDV_STRUC_DEBUG_INFO;

typedef PNDV_STRUC_DEBUG_INFO * PNDV_SYS_DEBUG_INFO_PTR;


/*****************************************************************************/
/* system interface functions */
/*****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif


/* pndv.c */
//!@ingroup grSYSTEM
//!@{
PNIO_VOID        pndv_init                            (PNDV_IFACE_STRUCT_PTR  iface_ptr);
PNIO_VOID        pndv_open                            (PNDV_SYS_PARAMETER_PTR parameter_ptr);
PNIO_VOID        pndv_perform_services                (PNIO_VOID);
PNIO_UINT16      pndv_version                         (PNIO_UINT16 version_len, PNDV_SYS_VERSION_PTR version_ptr);
PNIO_VOID        pndv_debug_extern                    (PNIO_UINT16 detail_1, PNIO_UINT16 detail_2);
PNIO_VOID        pndv_debug_get_info                  (PNDV_SYS_DEBUG_INFO_PTR debug_info);
PNIO_VOID        pndv_connect_ind_trigger_timeout     (PNIO_UINT16 timer_id, PNDV_USER_ID_TYPE user_id);

PNIO_VOID        pndv_request                         (PNDV_RQB_PTR_TYPE rqb_ptr);
//! @}

/* pndv_cm.c */

PNIO_VOID        pndv_cm_callback                     (CM_UPPER_RQB_PTR_TYPE rqb_ptr);


/* pndv_ds.c */

PNIO_VOID        pndv_dfc_cbf_pndv_ds                 (CM_UPPER_RQB_PTR_TYPE ds_ptr, PNIO_UINT8 con_id );
PNIO_VOID        pndv_dfc_cbf_cm                      (CM_UPPER_RQB_PTR_TYPE rqb_ptr, PNIO_UINT8 con_id );
PNIO_VOID        pndv_read_write_record               (PNIO_VOID* req_ptr);
PNIO_UINT32      pndv_ds_check_arfsu_data_adjust      (PNIO_UINT8* record_ptr, PNIO_UINT8* ret_fsu_enabled, PNIO_UINT8* ret_fsuuuid_ptr);


/* pndv_peri.c */
//!@ingroup grSYSTEM
//!@{
PNIO_VOID        pndv_peri_read_write_record          (PNIO_VOID* req_ptr);
//!@}
PNIO_UINT8       pndv_ar_get_num_connected_ars        (PNIO_VOID);

PNIO_VOID        pndv_peri_init_entity_admin(PNIO_VOID);
PNIO_VOID        pndv_peri_reset_entity_admin_by_ar_idx(PNIO_UINT32 ar_idx);
PNIO_UINT32      pndv_peri_get_entity_index(PNIO_UINT16 slot, PNIO_UINT16 subslot, PNIO_UINT32 ar_idx);
#ifdef PNDV_CFG_PERI_QUEUE_DS_REQ
PNIO_UINT32      pndv_peri_do_rqb_queue(CM_UPPER_RQB_PTR_TYPE rqb_ptr);
PNIO_VOID        pndv_peri_rqb_queue_get_next(CM_UPPER_RQB_PTR_TYPE *rqb_ptr_ptr);
#endif

/* pndv_cfg.c*/


PNIO_VOID        pndv_maint_led_control               (PNIO_UINT32 sf_led_mode);
PNIO_VOID        pndv_check_maint_info                (PNIO_VOID);

#ifndef PNDV_CM_OPEN_CHANNEL_LOWER
PNIO_VOID        PNDV_CM_OPEN_CHANNEL_LOWER(CM_UPPER_RQB_PTR_TYPE _rqb);
#endif

PNIO_UINT32 pndv_in_debug_control(PNDV_DEBUG_MODE_TYPE debug_control);


#ifdef __cplusplus
}
#endif


/*****************************************************************************/


#ifdef __cplusplus
extern "C"
{
#endif




#ifdef __cplusplus
}
#endif


/*****************************************************************************/
/* reinclude-protection */
/*****************************************************************************/

#else
    #pragma message ("The header PNDV_COM.H is included twice or more !")
#endif


/*****************************************************************************/
/*  end of file.                                                             */
/*****************************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
