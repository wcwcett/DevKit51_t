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
/*  F i l e               &F: pndv_cfg.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  configure pndv.                                                          */
/*                                                                           */
/*****************************************************************************/



/*****************************************************************************/
/* contents:

    COMMON

      basic tool chain characteristics

      component detail configuration
      device configuration
      peripherie configuration

    SYSTEM

      system interface macros

    PNDV

      code attributes
      data attributes

*/
/*****************************************************************************/
/* reinclude protection */


#ifndef PNDV_CFG_H
#define PNDV_CFG_H


/*****************************************************************************/
/*****************************************************************************/
/*******************************                ******************************/
/*******************************     COMMON     ******************************/
/*******************************                ******************************/
/*****************************************************************************/
/*****************************************************************************/

#if (IOD_INCLUDE_S2_REDUNDANCY == 1)
#define PNDV_CFG_ENABLE_S2
#define PNDV_S2_REDUNDANCY_EDGE_WHILE_DISCONNECTING_FIX
#endif

#if (IOD_INCLUDE_DR == 1)
#define PNDV_CFG_ENABLE_CIR
#endif

#if IOD_INCLUDE_AMR
#define PNDV_CFG_IS_ASSET_MANAGEMENT_SUPPORTED  PNIO_TRUE
#else
#define PNDV_CFG_IS_ASSET_MANAGEMENT_SUPPORTED  PNIO_FALSE
#endif

#define PNDV_NO_SPECIAL_TREATMENT_FOR_RW_RECORD_INDEX_1

/*****************************************************************************/
/* basic tool chain characteristics */
/*****************************************************************************/

#define PNDV_HOST_PTR_COMPARE_TYPE             LSA_HOST_PTR_COMPARE_TYPE

#define PNDV_PROCESSOR_DEFAULT_ALIGN                     4
#define PNDV_PROCESSOR_PTR_SIZE                          4

#define PNDV_MAX_IO_LEN                                 1440
#define PNDV_MAX_OF_AR                                  IOD_CFG_NUMOF_IO_AR
#define PNDV_MAX_IO_INPUT_LEN                           PNDV_MAX_IO_LEN  /**< sum of gross data bytes over all input CRs. see GSDML/MaxInputLength */
#define PNDV_MAX_IO_OUTPUT_LEN                          PNDV_MAX_IO_LEN  /**< sum of gross data bytes over all output CRs. see GSDML/MaxOutputLength*/
#ifdef IM_SUPPORT_PN_S2_REDUNDANCY
    /** sum of gross data bytes over all CRs. In case of S2, CR sizes belonging to one ARset are added up in CM's plausibility check as well! */
    /*#define PNDV_MAX_IO_IN_OUT_LEN                    (4*PNDV_MAX_IO_LEN)    */
#define PNDV_MAX_IO_IN_OUT_LEN                          ( (PNDV_MAX_IO_INPUT_LEN+PNDV_MAX_IO_OUTPUT_LEN) * PNDV_MAX_OF_AR )
#else
    /** sum of gross data bytes over all CRs. In non redundant case, it's the sum of input and output CR length */
    #define PNDV_MAX_IO_IN_OUT_LEN                      ( (PNDV_MAX_IO_INPUT_LEN+PNDV_MAX_IO_OUTPUT_LEN) * PNDV_MAX_OF_AR )
#endif
/*****************************************************************************/
/* component detail configuration */
/*****************************************************************************/
#define PNDV_AL_STAL_SOE_USI_MIN    0x00
#define PNDV_AL_STAL_SOE_USI_MAX    0xffff

#define PNDV_AL_STAL_RS_LOW_WATERMARK_USI   0x8300
#define PNDV_AL_STAL_RS_TIMEOUT_USI         0x8301
#define PNDV_AL_STAL_RS_OVERFLOW_USI        0x8302
#define PNDV_AL_STAL_RS_EVENT_USI           0x8303



#define PNDV_CFG_DEBUG_ENABLE
#define PNDV_CFG_DEBUG_ELEMENT_NUMBER                4000

#define PNDV_ASSERT(_cond)                             \
{                                                      \
    if (!(_cond))                                      \
    {                                                  \
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0); \
    }                                                  \
}

#if (PNIO_TRACE != PNIO_TRACE_NONE)
#define PNDV_TRACE_MSG(_subsys, _level, msg, para1, para2, para3)                                               \
    if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[TRACE_SUBSYS_PNDV_LOWER].level >= _level))         \
        DK_TRACE_03(LTRC_ACT_MODUL_ID , __LINE__, TRACE_SUBSYS_PNDV_LOWER, _level, msg, para1, para2, para3)

#ifdef PNDV_TRACE_MSG
    #define PNDV_TRACE_MSG_GET_STR(_level, _p_deb_str, _deb_code, _detail_1, _detail_2, _detail_3)              \
        if ((PNIO_TRACE_COMPILE_LEVEL >= _level) && (SubsysTrace[TRACE_SUBSYS_PNDV_LOWER].level >= _level))     \
            PNDV_TRACE_MSG_GET_DEBUG_STR(_p_deb_str, _deb_code, _detail_1, _detail_2, _detail_3)
#endif //PNDV_TRACE_MSG
#else //(PNIO_TRACE != PNIO_TRACE_NONE)
#define PNDV_TRACE_MSG(_subsys, _level, msg, para1, para2, para3)
#define PNDV_TRACE_MSG_GET_STR(_level, _p_deb_str, _deb_code, _detail_1, _detail_2, _detail_3)
#endif //(PNIO_TRACE != PNIO_TRACE_NONE)


    //#define PNDV_CFG_DEBUG_USE_IM_TRC
#ifndef PNDV_CFG_DEBUG_USE_IM_TRC
        #define PNDV_TRC_STRUC(_TRC_TE_STRUC,_TRC_ELEMENT_NUMBER)    TRC_STRUC(_TRC_TE_STRUC,_TRC_ELEMENT_NUMBER)/* no ";" here! */
        #define PNDV_TRC_TP_INIT(_TRC,_TRC_ELEMENT_NUMBER,_TRC_ID)   TRC_TP_INIT(_TRC,_TRC_ELEMENT_NUMBER,_TRC_ID)
        #define PNDV_TRC_TEH_FILL(_TRC)                              TRC_TEH_FILL(_TRC)
        #define PNDV_TRC_TE_ACCESS(_TRC)                             TRC_TE_ACCESS(_TRC)
#endif


/* @brief enable internal record sink
 *
 *  The internal record responder is used for special handling of read i/o
 *  records (0x8028, 0x8029) for all subslots of the device.
 *  This is used in interface modules (e.g. IM151-3, IM153-4 etc.).
 */
// #define PNDV_CFG_ENABLE_RECORD_RESPONDER


#define PNDV_GET_ENTITY_NR(_API, _SLOT, _SUBSLOT, _AR_IDX) pndv_peri_get_entity_index(_SLOT, _SUBSLOT, _AR_IDX)

/*****************************************************************************/
/* feature confirguration */
/*****************************************************************************/

/*
 * Must be defined if the device does not support any shared ar while in
 * isochronous  mode.
 */
//#define PNDV_CFG_ISOM_NO_SHARED
/*
 * Defines the record index (prm record) that configures the isochronous mode.
 * PNDV is listening for this index to detect the isochronous ar.
 */
//#define PNDV_ISOM_DS_NR_NORM        APMA_ISOM_DS_NR_NORM
/*
 * If defined red phase informations provided with the RIR are copied to the interface
 * see PNDV_IFACE_STRUCT.ar
 */
#ifdef FBPB_USE_RED_INT_INFO
    #define PNDV_CFG_USE_RED_INT_INFO
#endif

/* @brief Enable the use of ADD2/REMOVE2 opcodes instead of ADD/REMOVE
 *
 * If defined enables the usage of CM_OPC_SV_DIAG_ADD2/REMOVE2 instead of CM_OPC_SV_DIAG_ADD/REMOVE
 * for both diag types (channel and generic).
 */
//#define PNDV_CFG_USE_DIAG2

/* @brief Enable the use of ADD2/REMOVE2 opcodes instead of ADD/REMOVE only for channel diag
 *
 * If defined enables the usage of CM_OPC_SV_DIAG_ADD2/REMOVE2 instead of CM_OPC_SV_DIAG_ADD/REMOVE
 * only for channel diag. Generic diag will still use ADD/REMOVE to be able to use diag_tag to distinguish the
 * diags. Therefor it is not allowed to use diag_tag == 1.
 */
//#define PNDV_CFG_USE_MULTIDIAG

#define PNDV_CFG_USE_USERTAG
/*****************************************************************************/
/* device configuration */
/*****************************************************************************/

#define PNDV_RESET_REASON_NONE                          (PNIO_UINT32)0x00000000
#define PNDV_RESET_REASON_CODE_ERR_PRM_CONSISTENCY      (PNIO_UINT32)0x00000001
#define PNDV_RESET_REASON_CODE_RESTART_AFTER_FATAL      (PNIO_UINT32)0x00000002

#define PNDV_CM_DEVICE_NO           DEV_CFG_IM_DEVICE_NO
#define PNDV_CM_AR_NO               CM_CFG_MAX_SV_ARS
#define PNDV_CM_API_NO_DEFAULT      0
#define PNDV_CM_VENDOR_ID           DEV_CFG_IM_VENDOR_ID
#define PNDV_CM_DEVICE_INDEX        PNDV_CM_DEVICE_NO
#define PNDV_CM_DEVICE_INSTANZ      PNDV_CM_DEVICE_NO

#define PNDV_CFG_MAX_PORT_CNT       EDD_CFG_MAX_PORT_CNT

#define PNDV_IM_SUBMODUL_1_ID       0
#define PNDV_IM_HAS_GLOB_PARA       0 /* 0 IM has no global para, 1 IM has global para */

#define PNDV_MAX_ARS_RT             DEV_CFG_MAX_ARS_RT
#define PNDV_MAX_ARS_DA             DEV_CFG_MAX_ARS_DA
#define PNDV_MAX_AR_SET             1                   /**< max count of system redundancy ar_sets (ARs that belong to the same redundant projecting)
                                                             should be half of PNDV_CM_AR_NO or less */

/* IM Module ID                                     */
/* bit4 .. bit0 -> issue of IM                      */
/* bit7 .. bit5 -> component of IM - Standard/HF    */
/* bit31.. bit8 -> IM..                             */

#define PNDV_IM_MODUL_MASK                      0x000000FF
#define PNDV_IM_MODUL_COMP_MASK                 0x000000E0
#define PNDV_IM_MODUL_ISSUE_MASK                0x0000001F
#define PNDV_IM_MODUL_MIN_SOLL_MASK             0x000000FF

#define PNDV_MODUL_ID_MODUL_SPEC_BIT_MASK       0x80000000
#define PNDV_MODUL_ID_MODUL_KENNMASK            0x000000FF
#define PNDV_MODUL_ID_PRM_BYTE1_MASK            0x00FF0000
#define PNDV_MODUL_ID_PRM_BYTE1_SHIFT_POS       16


/*****************************************************************************/
/* peripherie configuration */
/*****************************************************************************/

/* Schalter bei Compactdevice Eco-PN / Motorstarter definieren */
// #define PNDV_CFG_ET200_COMPACT

/* Schalter definieren, wenn Peripherieslots als IuM-faehig eingetragen werden sollen */
#define PNDV_CFG_IM0_BITS_FILTERLISTE

#define PNDV_IM_SLOT_NO                         IOD_CFG_DAP_SLOT_NUMBER
#define PNDV_IM_DEFAULT_SUBSLOT_NR              IOD_CFG_DAP_SUBSLOT_NUMBER
/* expected configuration settings */       
#define PNDV_CM_SV_MAX_SLOT_NR                  IOD_CFG_MAX_SLOT_NUMBER/* 0x0 ... 0x7fff, highest value for slot number */
#define PNDV_CM_SV_MAX_SUBSLOT_NR               IOD_CFG_MAX_NUMOF_SUBSL_PER_SLOT/* 0x0 ... 0x7fff, highest value for subslot number */
#define PNDV_CM_SV_SUBSLOT_COUNT                IOD_CFG_MAX_NUMOF_SUBSLOTS/* maximum count of used subslots in an ar */

#define PNDV_CM_PD_MAX_REMA_BUFFER_SIZE         CM_PD_MAX_REMA_BUFFER_SIZE

#define PNDV_MAX_SV_ENTITY                      PNDV_CM_SV_SUBSLOT_COUNT
#define PNDV_MAX_SV_SUBSLOTS                    PNDV_CM_SV_SUBSLOT_COUNT

#define PNDV_AL_PRAL_INFO_MAX_LEN               IOD_CFG_MAX_PROCESS_ALARM_LEN /* to be verified */ /*HySo: not found in norm, set same as UPAL_LEN (becuase used example generates fatal error)*/
#define PNDV_AL_PRAL_INFO_LEN                   IOD_CFG_MAX_PROCESS_ALARM_LEN
#define PNDV_AL_UPAL_INFO_LEN                   64
#define PNDV_AL_STAL_INFO_LEN                   4 /* dummy value  */
#define PNDV_AL_URAL_INFO_LEN                   IOD_CFG_MAX_UPLOAED_RETRIEVAL_INFO_LEN  /* MAX(Upload&Retrival = 20, IParam = 24)  */

#define PNDV_MAX_CHN_DIAG_PER_SLOT_AND_MOMENT   130 /* 64 channels with 2 diagnoses each + 2 for the entire BG */
#define PNDV_MAX_EXT_DIAG_PER_SLOT_AND_MOMENT   130
#define PNDV_MAX_GENERIC_DIAG_NUMBER            1
#define PNDV_MAX_GENERIC_DIAG_DATA_LENGTH       IOD_CFG_MAX_GEN_DIAG_DATA_LEN

#define PNDV_IFACE_CMD_CNT                      (5 * (IOD_CFG_MAX_NUMOF_SUBSLOTS + 1) ) //256

#define PNDV_HANDLE_ARFSU_DATA_ADJUST           LSA_TRUE
#define PNDV_NO_DISCARD_IOXS_SUPPORT            LSA_FALSE

/*****************************************************************************/
/* Records */
#define PNDV_MAX_RD_IO_DATA_LEN         IOD_CFG_MAX_IO_NET_LENGTH_PER_SUBSLOT /* maximum size for netto IO-Data of rd-input -output-records */
#define PNDV_MAX_RECORD_DATA_LEN        IOD_CFG_MAX_RECORD_LENGTH

/* es wird von den Err-codes nur der Low-Teil genommen */
#define PNDV_EC1_REC_NON               (PNIO_UINT8)0
#define PNDV_EC1_REC_WRITE             (PNIO_UINT8)0xA1 /* Bus-Zugriffsfehler bei DS schreiben */
#define PNDV_EC1_REC_BUSY              (PNIO_UINT8)0xA7
#define PNDV_EC1_REC_NOT_SUPPORTED     (PNIO_UINT8)0xA9
#define PNDV_EC1_REC_INDEX_INVALID     (PNIO_UINT8)0xB0 /* Baugruppe kennt den Datensatz nicht oder Baugruppe nicht miniprotokollfaehig */
#define PNDV_EC1_REC_WRITE_LEN         (PNIO_UINT8)0xB1 /* die angegebene Datensatzlange ist falsch */
#define PNDV_EC1_REC_SLOT_INVALID      (PNIO_UINT8)0xB2 /* Den Steckplatz gibt es nicht bzw. das Modul steckt nicht */
#define PNDV_EC1_REC_STATE_CONFLICT    (PNIO_UINT8)0xB5
#define PNDV_EC1_REC_ACCESS_DENIED     (PNIO_UINT8)0xB6
#define PNDV_EC1_REC_INVALID_RANGE     (PNIO_UINT8)0xB7
#define PNDV_EC1_REC_INVALID_PARAMETER (PNIO_UINT8)0xB8
#define PNDV_EC1_REC_INVALID_TYPE      (PNIO_UINT8)0xB9
#define PNDV_EC1_REC_WRITE_BACKUP      (PNIO_UINT8)0xBA


#ifdef PNDV_CFG_ENABLE_RECORD_RESPONDER
#define PNDV_PARA_RECORD_INDEX_FIX      0
#define PNDV_RECORD_ARFSU_DATA_ADJUST   0
#define PNDV_RECORD_INDEX_RD_INP        0
#define PNDV_RECORD_INDEX_RD_OUTP       0
#endif


/* PNDV_PERI_RECORD_INDEX_RD_INP, PNDV_PERI_RECORD_INDEX_RD_OUTP must be different from PNDV_RECORD_INDEX_RD_INP, PNDV_RECORD_INDEX_RD_OUTP
   because PNDV creates a local record for PERI */
#define PNDV_PERI_RECORD_INDEX_RD_INP   0
#define PNDV_PERI_RECORD_INDEX_RD_OUTP  0

/* opaque data */
#define PNDV_MAX_IOCR_PER_AR            2                               /* max count of iocr's */


/*****************************************************************************/
/*****************************************************************************/
/*******************************                ******************************/
/*******************************     SYSTEM     ******************************/
/*******************************                ******************************/
/*****************************************************************************/
/*****************************************************************************/


/*****************************************************************************/
/* system interface macros */

#define PNDV_NTOHS(_VAL)    OsNtohs(_VAL)
#define PNDV_NTOHL(_VAL)    OsNtohl(_VAL)
#define PNDV_HTONS(_VAL)    OsHtons(_VAL)
#define PNDV_HTONL(_VAL)    OsHtonl(_VAL)

#define PNDV_OPEN_DONE(_RESULT)                                               \
{                                                                             \
    PNIO_UINT32 result = (_RESULT);                                           \
                                                                              \
    if(result != PNDV_OK)                                                     \
    {                                                                         \
        PNDV_ASSERT(0);                                                       \
    }                                                                         \
}                                                                             \


#define PNDV_STOP_DONE(_RESULT)                                               \
{                                                                             \
    PNDV_IFACE_CMD_ENTRY_T stop_done_event;                                   \
                                                                              \
    stop_done_event.cmd = PNDV_EV_TO_PERI_PNDV_STOP_DONE;                     \
                                                                              \
    pndv_in_write_debug_buffer_all__(PNDV_DC_STOP_DONE, _RESULT);             \
                                                                              \
    pndv_in_peri_write_coupl_event( stop_done_event);                         \
}                                                                             \

#define PNDV_TRIGGER_INTERFACE_RESPONDER()  tskma_task_app_send_pnpb_trigger()

#define PNDV_REQUEST_LOCAL(_rqb)                                              \
{                                                                             \
    LSA_RQB_SET_REQ_FCT_PTR(_rqb, pndv_request);                              \
    OsSendMessage(TSKMA_TASK_ID_APPLICATION, /* id of the  task (references the task message queue) */ \
                  _rqb,                      /* parameter 2 (request block)*/ \
                  OS_MBX_PRIO_NORM);         /* message priority */           \
}

#define PNDV_RQB_APPL_REQUEST(_rqb_ptr, _method_ptr)                          \
{                                                                             \
    LSA_RQB_SET_REQ_FCT_PTR(_rqb_ptr, _method_ptr);                           \
    OsSendMessage(TSKMA_TASK_ID_APPLICATION, /* id of the  task (references the task message queue) */ \
                  _rqb_ptr,                  /* parameter 2 (request block)*/ \
                  OS_MBX_PRIO_NORM);         /* message priority */           \
}

/*******************************************************************************/
/* interface between pndv and ertec200p i/o data handling                      */
/*******************************************************************************/

/* returning data offset for input or output cr memory / iocr pointer and the ar-index is given */
#define PNDV_ALLOCATE_IOCR_MEMORY(_ar_idx, _ar_set_nr, _session_id, _iocr_ptr)            iom_allocate_iocr_memory((_ar_idx), (_ar_set_nr), (_session_id), (_iocr_ptr))

#define PNDV_COMPLETE_IOCR_INFORMATION(_ar_idx, _ar_set_nr, _session_id, _iocr_ptr) (_iocr_ptr==_iocr_ptr) /*TRUE*/

/* release input and output cr memory of the given ar -> offline indication */
#define PNDV_FREE_IOCR_MEMORY(_ar_idx)                                                    iom_free_iocr_memory(_ar_idx)


/*****************************************************************************/
/* fatal error handling */
extern void PNIO_FatalError(LSA_FATAL_ERROR_TYPE* pLsaErr);
#define PNDV_FATAL_ERROR(_ERROR_DETAIL_PTR)  PNIO_FatalError(_ERROR_DETAIL_PTR)

#define PNDV_IS_SUPPORTED_OLD_PROJECTING(_soll_id) (_soll_id!=_soll_id) /* (LSA_FALSE) or ( PNDV_IS_MODUL_ID_LT_V7( (_soll_id) ) )*/

/**
 *  @brief brief_description
 *
 *  This macro is called during pndv startup and it should return the startup condition
 *  whether it is a normal startup or a startup after reset etc.
 *  See also PNDV_RESET_REASON_xxx defines
 *
 */
#define PNDV_GET_RESET_REASON_CODE()   PNDV_RESET_REASON_NONE

#define PNDV_SPRINTF                   PNIO_sprintf

/*****************************************************************************/
/* copy data */

#define PNDV_COPY_BYTE(_DEST_PTR,_SRC_PTR,_LEN)                               \
{                                                                             \
    OsMemCpy((PNIO_VOID*)(_DEST_PTR), (PNIO_VOID*)(_SRC_PTR), _LEN);          \
}

#define PNDV_MEMSET(_DST_PTR,_VAL,_LEN)                                       \
{                                                                             \
    OsMemSet((PNIO_VOID*)(_DST_PTR), _VAL, _LEN);                             \
}

/*****************************************************************************/
/* memory comparison */

#define PNDV_CMP_BYTE(_DST_PTR, _SRC_PTR, _SIZE) OsMemCmp(_DST_PTR, _SRC_PTR, _SIZE)

/*****************************************************************************/
/* memory allocation */

#define PNDV_ALLOC_MEM(_local_mem_ptr_ptr, _length)                                    \
{                                                                                      \
    pndv_data.alloc_mem_counter++;                                                     \
    OsAllocX((PNIO_VOID**)_local_mem_ptr_ptr, 0, (LSA_UINT32)_length, MEMPOOL_DEFAULT);\
}

#define PNDV_ALLOC_RQB(_local_mem_ptr_ptr, _length)                                    \
{                                                                                      \
    pndv_data.alloc_rqb_counter++;                                                     \
    OsAllocX((PNIO_VOID**)_local_mem_ptr_ptr, 0, (LSA_UINT32)_length, MEMPOOL_DEFAULT);\
}

#define PNDV_FREE_MEM(_ret_val_ptr, _local_mem_ptr)                                    \
{                                                                                      \
    pndv_data.alloc_mem_counter--;                                                     \
    *(_ret_val_ptr) = (LSA_UINT16)OsFreeX(_local_mem_ptr, MEMPOOL_DEFAULT);            \
}

#define PNDV_FREE_RQB(_ret_val_ptr, _local_mem_ptr)                                    \
{                                                                                      \
    pndv_data.alloc_rqb_counter--;                                                     \
    *(_ret_val_ptr) = (LSA_UINT16)OsFreeX(_local_mem_ptr, MEMPOOL_DEFAULT);            \
}
/*****************************************************************************/
/* timer allocation */

#define PNDV_USER_ID_TYPE LSA_UINT16

#define PNDV_ALLOC_TIMER(_RET_VAL_PTR, _TIMER_ID_PTR, _TIMER_TYPE, _TIME_BASE)                                            \
{                                                                                                                         \
    *(_RET_VAL_PTR) = (LSA_UINT16)OsAllocTimer(_TIMER_ID_PTR, _TIMER_TYPE, _TIME_BASE, pndv_connect_ind_trigger_timeout); \
}

#define PNDV_START_TIMER(_RET_VAL_PTR, _TIMER_ID, _USER_ID, _TIME)                     \
{                                                                                      \
    *(_RET_VAL_PTR) = (LSA_UINT16)OsStartTimer(_TIMER_ID, _USER_ID.uvar32, _TIME);     \
}

#define PNDV_STOP_TIMER(_RET_VAL_PTR, _TIMER_ID)                                       \
{                                                                                      \
    *(_RET_VAL_PTR) = (LSA_UINT16)OsStopTimer(_TIMER_ID);                              \
}

#define PNDV_FREE_TIMER(_RET_VAL_PTR, _TIMER_ID)                                       \
{                                                                                      \
    *(_RET_VAL_PTR) = (LSA_UINT16)OsFreeTimer(_TIMER_ID);                              \
}

/*******************************************************************************/
/* Led and pin control                                                         */
/*******************************************************************************/

/* must be defined empty if no FSU measurement */
#define PNDV_CFG_MEASUREMENT_LINK_UP()
#define PNDV_CFG_MEASUREMENT_APPL_READY()


/**** new led implementation ***/

/*
 * Network cable disconnected.
 *
 */
#define PNDV_LED_CTRL_BUS_FAULT()\
{\
    Bsp_EbSetLed (PNIO_LED_ERROR, 1); \
}

/*
 * Network cable connected.
 *
 */
#define PNDV_LED_CTRL_BUS_OK()\
{\
    Bsp_EbSetLed (PNIO_LED_ERROR, 0); \
}

/*
 * Data-AR goes online.
 *
 */
#define PNDV_LED_CTRL_DATA_AR_IN_DATA()\
{\
    Bsp_EbSetLed (PNIO_LED_RUN, 1);  \
}

/*
 * Data-AR goes offline.
 *
 */
#define PNDV_LED_CTRL_DATA_AR_OFFLINE()\
{\
    Bsp_EbSetLed (PNIO_LED_RUN, 0); \
}

/*
 * IRT AR is in sync.
 *
 */
#define PNDV_LED_CTRL_IN_SYNC()\
{\
    Bsp_EbSetLed (PNIO_LED_SYNC, 1);  \
}

/*
 * IRT AR is out of sync.
 *
 */
#define PNDV_LED_CTRL_OUT_OFF_SYNC()\
{\
    Bsp_EbSetLed (PNIO_LED_SYNC, 0);  \
}

/*
 * @brief indicate existing maint events
 *
 * Maintenance event existing.
 *
 */
#define PNDV_LED_CTRL_MAINT_ON()\
{\
    Bsp_EbSetLed (PNIO_LED_MAINT, 1); \
}

/*
 * @brif indicate non existing main events
 *
 * No maintenance events existing.
 *
 */
#define PNDV_LED_CTRL_MAINT_OFF()\
{\
    Bsp_EbSetLed (PNIO_LED_MAINT, 0); \
}

/*
 * System error existing.
 */
#define PNDV_LED_CTRL_SYS_ERROR()\
{\
    Bsp_EbSetLed (PNIO_LED_ERROR, 1); \
}

/*
 * No system error existing.
 */
#define PNDV_LED_CTRL_NO_SYS_ERROR()\
{\
    Bsp_EbSetLed (PNIO_LED_ERROR, 0); \
}


/* Macro for dependency of the Maint-LED on the SF-LED      */
/* if there is no dependency, the macro can remain empty    */
/* ecoPN -> Maint-LED and SF-LED under one light guide      */
#define PNDV_MAINT_LED_CONTROL(_SF_LED_MODE)
#define PNDV_RECORD_INDEX_MULTIPLE_PDEV    CM_PD_RECORD_INDEX_MultiplePDev
#define PNDV_RECORD_INDEX_NAME_OF_STATION  CM_PD_RECORD_INDEX_NameOfStationRecord

#define PNDV_GET_PD_DS(_DATA_PTR_PTR, _DATA_LENGTH_PTR)                     \
{                                                                           \
    PnpbNvRestoreRemaMem(_DATA_LENGTH_PTR, (PNIO_UINT8**)_DATA_PTR_PTR);    \
}

#define PNDV_GET_SNMP_DATA(_NAME_PTR_PTR, _NAME_LENGTH_PTR, _CONTACT_PTR_PTR, _CONTACT_LENGTH_PTR, _LOCATION_PTR_PTR, _LOCATION_LENGTH_PTR)\
{                                                                           \
    PnpbNvpSnmpDatas((PNIO_UINT8**)_NAME_PTR_PTR, _NAME_LENGTH_PTR,         \
                     (PNIO_UINT8**)_CONTACT_PTR_PTR, _CONTACT_LENGTH_PTR,   \
                     (PNIO_UINT8**)_LOCATION_PTR_PTR, _LOCATION_LENGTH_PTR);\
}
#define PNDV_GET_IP_DATA(_DATA_PTR_PTR, _DATA_LENGTH_PTR)                   \
{                                                                           \
    PnpbNvGetpIpSuit((PNIO_UINT8**)(_DATA_PTR_PTR), _DATA_LENGTH_PTR);      \
}

#define PNDV_VALIDATE_INITIAL_IP(_DATA_PTR)  validate_ip_suite((PNIO_UINT8*)(_DATA_PTR))

#define PNDV_GET_NOS_DATA(_DATA_PTR_PTR, _DATA_LENGTH_PTR)                  \
{                                                                           \
    PnpbNvGetpStationName((PNIO_UINT8**)(_DATA_PTR_PTR), _DATA_LENGTH_PTR); \
}

#define PNDV_GET_DEVICE_TYPE(_PTR, _LEN)                                    \
{                                                                           \
    PnpbNvGetDeviceType(_PTR, _LEN);                                        \
}

#define PNDV_GET_ORDER_ID(_PTR, _LEN)                                       \
{                                                                           \
    PnpbNvGetOrderId(_PTR, _LEN);                                           \
}

#define PNDV_GET_SERIAL_NUMBER(_PTR, _LEN)                                  \
{                                                                           \
    PnpbNvGetSerialNumber(_PTR, _LEN);                                      \
}

#define PNDV_GET_HW_REVISION(_HW_REVISION)                                  \
{                                                                           \
    _HW_REVISION = PnpbNvGetHwRevision();                                   \
}

#define PNDV_GET_SW_REVISION(_PTR_REVPREF, _PTR_FNCENHC, _PTR_BUGFIX, _PTR_INTCHG)                    \
{                                                                                                     \
    PnpbNvGetSwRevision(_PTR_REVPREF, _PTR_FNCENHC, _PTR_BUGFIX, _PTR_INTCHG);                        \
}

#define PNDV_IM_GET_IP_SUITE(_RET_PTR, _IP_PTR, _SBN_PTR, _GTW_PTR)                                   \
{                                                                                                     \
    PnpbNvGetIpSuite((PNIO_UINT32*)_IP_PTR, (PNIO_UINT32*)_SBN_PTR, (PNIO_UINT32*)_GTW_PTR);          \
    *(_RET_PTR) = LSA_TRUE;                                                                           \
}

#define PNDV_GET_SNMP_SYS_DESCR(_SYS_DESCR_PTR_PTR, _SYS_DESCR_LEN_PTR)                               \
{                                                                                                     \
    *_SYS_DESCR_LEN_PTR = PnpbNvGetpSnmpData()->SysDescrLen;                                          \
    *_SYS_DESCR_PTR_PTR = PnpbNvGetpSnmpData()->SysDescr;                                             \
}

#define PNDV_GET_IFDESCR_INTERFACE(_IF_DESC_PTR_PTR, _IF_DESC_LEN_PTR)                                \
{                                                                                                     \
    *_IF_DESC_PTR_PTR = (LSA_UINT8*)IOD_CFG_SNMP_INTERFACE_NAME;                                      \
    *_IF_DESC_LEN_PTR = OsStrLen(IOD_CFG_SNMP_INTERFACE_NAME);                                        \
}

#define PNDV_GET_IFDESCR_PORT(_IF_DESC_PTR_PTR, _IF_DESC_LEN_PTR, _PORT_ID)                           \
{                                                                                                     \
    switch(_PORT_ID)                                                                                  \
    {                                                                                                 \
        case 1:                                                                                       \
        {                                                                                             \
            *_IF_DESC_PTR_PTR = (LSA_UINT8*)IOD_CFG_SNMP_PORT1_NAME;                                  \
            *_IF_DESC_LEN_PTR = OsStrLen(IOD_CFG_SNMP_PORT1_NAME);                                    \
            break;                                                                                    \
        }                                                                                             \
        case 2:                                                                                       \
        {                                                                                             \
            *_IF_DESC_PTR_PTR = (LSA_UINT8*)IOD_CFG_SNMP_PORT2_NAME;                                  \
            *_IF_DESC_LEN_PTR = OsStrLen(IOD_CFG_SNMP_PORT2_NAME);                                    \
            break;                                                                                    \
        }                                                                                             \
        default:                                                                                      \
        {                                                                                             \
            pndv_in_fatal_error(PNDV_MODULE, __LINE__, _PORT_ID);                                     \
            break;                                                                                    \
        }                                                                                             \
    }                                                                                                 \
}

#define PNDV_CFG_REMA_WRITE_IP_DATA(_REMA_DATA_PTR, _REMA_DATA_LEN)                                   \
{                                                                                                     \
    PNDV_IP2PN_IPSUITE_INFO_T ipsuite;                                                                \
    if (get_ipsuite_data_from_rema_record(_REMA_DATA_PTR, _REMA_DATA_LEN, &ipsuite) == PNIO_OK)       \
    {                                                                                                 \
        PnpbNvSaveIpSuite(*((PNIO_UINT32*)&ipsuite.ip_address),                                       \
                          *((PNIO_UINT32*)&ipsuite.netmask),                                          \
                          *((PNIO_UINT32*)&ipsuite.gateway),                                          \
                          PNIO_TRUE);                                                                 \
    }                                                                                                 \
    else                                                                                              \
    {                                                                                                 \
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);                                                \
    }                                                                                                 \
}

#define PNDV_CFG_REMA_DELETE_IP_DATA()                                                                \
{                                                                                                     \
    PnpbNvSaveIpSuite (0, 0, 0, LSA_FALSE);                                                           \
}

#define PNDV_CFG_REPORT_NEW_IP_DATA(IPADDR, NETMASK, GATEWAY)                                         \
{                                                                                                     \
    PNIO_UINT32 ip_address[3];                                                                        \
    ip_address[0] = PNDV_HTONL((PNIO_UINT32)IPADDR);                                                  \
    ip_address[1] = PNDV_HTONL((PNIO_UINT32)NETMASK);                                                 \
    ip_address[2] = PNDV_HTONL((PNIO_UINT32)GATEWAY);                                                 \
    Pnpb_report_new_ip_data(*((PNIO_UINT32*)(ip_address)),                                            \
                            *((PNIO_UINT32*)(ip_address + 1)),                                        \
                            *((PNIO_UINT32*)(ip_address + 2)));                                       \
}

#define PNDV_CFG_REMA_WRITE_SNMP_DATA(_REMA_DATA_PTR, _REMA_DATA_LEN)                                    \
{                                                                                                        \
    PNDV_IP2PN_SNMP_DATA_TYPE_T snmp_data = {0};                                                         \
    if (get_snmp_data_from_rema_record(_REMA_DATA_PTR, _REMA_DATA_LEN, &snmp_data) == PNIO_OK)           \
    {                                                                                                    \
        PnpbNvStoreSnmpData((PNIO_UINT32)snmp_data.SysNameLen,     (PNIO_UINT8*)snmp_data.pSysName,      \
                            (PNIO_UINT32)snmp_data.SysContactLen,  (PNIO_UINT8*)snmp_data.pSysContact,   \
                            (PNIO_UINT32)snmp_data.SysLocationLen, (PNIO_UINT8*)snmp_data.pSysLocation); \
    }                                                                                                    \
    else                                                                                                 \
    {                                                                                                    \
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);                                                   \
    }                                                                                                    \
}

#define PNDV_CFG_REMA_DELETE_SNMP_DATA()                                                                 \
{                                                                                                        \
    PnpbNvStoreSnmpData(0, LSA_NULL, 0, LSA_NULL, 0, LSA_NULL);                                          \
}

#define PNDV_CFG_REMA_WRITE_PDEV_DATA(_REMA_DATA_PTR, _REMA_DATA_LEN)                                 \
{                                                                                                     \
    PnpbNvStoreRemaMem(_REMA_DATA_LEN, _REMA_DATA_PTR);                                               \
}

#define PNDV_CFG_REMA_DELETE_PDEV_DATA()                                                              \
{                                                                                                     \
    PnpbNvStoreRemaMem(0, (PNIO_UINT8*)0);                                                            \
}

#define PNDV_CFG_REMA_WRITE_NOS_DATA(_REMA_DATA_PTR, _REMA_DATA_LEN)                                  \
{                                                                                                     \
    PNIO_UINT8* nos_data_ptr = 0;                                                                     \
    PNIO_UINT16 nos_data_len = 0;                                                                     \
    if (get_nos_data_from_rema_record(_REMA_DATA_PTR, _REMA_DATA_LEN,                                 \
                                      &nos_data_ptr,  &nos_data_len) == PNIO_OK)                      \
    {                                                                                                 \
        PnpbNvSaveStationName((PNIO_INT8*)nos_data_ptr,                                               \
                              (PNIO_UINT32)nos_data_len,                                              \
                              ((nos_data_len == 0) ? PNIO_FALSE : PNIO_TRUE));                        \
    }                                                                                                 \
    else                                                                                              \
    {                                                                                                 \
        pndv_in_fatal_error(PNDV_MODULE, __LINE__, 0);                                                \
    }                                                                                                 \
}

#define PNDV_CFG_REMA_DELETE_NOS_DATA()                                                               \
{                                                                                                     \
    PnpbNvSaveStationName((PNIO_INT8*)0, 0, PNIO_FALSE);                                              \
}

#define PNDV_RESET_TO_FACTORY_IND()           PnpbNvResetToFactorySettings(PNIO_RTF_RES_ALL)
#define PNDV_RESET_COMMUNICATION_IND()        PnpbNvResetToFactorySettings(PNIO_RTF_RES_COMM_PAR)

/*****************************************************************************/
/* system interface parameters */


/*****************************************************************************/
/*****************************************************************************/
/*******************************                ******************************/
/*******************************       CM       ******************************/
/*******************************                ******************************/
/*****************************************************************************/
/*****************************************************************************/


/*****************************************************************************/
/* cm interface macros */

#define PNDV_OPEN_CHANNEL_LOWER(_rqb, _comp_id)     \
{                                                   \
    PSI_RQB_SET_COMP_ID(_rqb, _comp_id);            \
    psi_ld_open_channel((PSI_RQB_PTR_TYPE)_rqb);    \
}

#define PNDV_CLOSE_CHANNEL_LOWER(_rqb, _comp_id)    \
{                                                   \
    PSI_RQB_SET_COMP_ID(_rqb, _comp_id);            \
    psi_ld_close_channel((PSI_RQB_PTR_TYPE)_rqb);   \
}

#define PNDV_REQUEST(_rqb, _comp_id)                \
{                                                   \
    PSI_RQB_SET_COMP_ID(_rqb, _comp_id);            \
    psi_ld_request((PSI_RQB_PTR_TYPE)_rqb);         \
}


/*****************************************************************************/
/*****************************************************************************/
/*********************************             *******************************/
/*********************************     RQB     *******************************/
/*********************************             *******************************/
/*****************************************************************************/
/*****************************************************************************/

/* RQB handling */

#define PNDV_RQB_HEADER   LSA_RQB_HEADER(PNDV_RQB_PTR_TYPE)
#define PNDV_RQB_TRAILER  LSA_RQB_TRAILER

#define PNDV_RQB_SET_NEXT_RQB(_rb, _value)                  LSA_RQB_SET_NEXT_RQB_PTR(_rb, _value)
#define PNDV_RQB_SET_PREV_RQB(_rb, _value)                  LSA_RQB_SET_PREV_RQB_PTR(_rb, _value)
#define PNDV_RQB_SET_OPCODE(_rb, _value)                    LSA_RQB_SET_OPCODE(_rb, _value)
#define PNDV_RQB_SET_HANDLE(_rb, _value)                    LSA_RQB_SET_HANDLE(_rb, _value)
#define PNDV_RQB_SET_USER_ID_UVAR16_ARRAY_LOW(_rb, _value)  LSA_RQB_SET_USER_ID_UVAR16_ARRAY_LOW(_rb, _value)
#define PNDV_RQB_SET_USER_ID_UVAR16_ARRAY_HIGH(_rb, _value) LSA_RQB_SET_USER_ID_UVAR16_ARRAY_HIGH(_rb, _value)

#define PNDV_RQB_GET_NEXT_RQB(_rb)                          LSA_RQB_GET_NEXT_RQB_PTR(_rb)
#define PNDV_RQB_GET_PREV_RQB(_rb)                          LSA_RQB_GET_PREV_RQB_PTR(_rb)
#define PNDV_RQB_GET_OPCODE(_rb)                            LSA_RQB_GET_OPCODE(_rb)
#define PNDV_RQB_GET_HANDLE(_rb)                            LSA_RQB_GET_HANDLE(_rb)
#define PNDV_RQB_GET_USER_ID_UVAR16_ARRAY_LOW(_rb)          LSA_RQB_GET_USER_ID_UVAR16_ARRAY_LOW(_rb)
#define PNDV_RQB_GET_USER_ID_UVAR16_ARRAY_HIGH(_rb)         LSA_RQB_GET_USER_ID_UVAR16_ARRAY_HIGH(_rb)

#define PNDV_RQB_SET_RESPONSE(_rb, _value)                  LSA_RQB_SET_RESPONSE(_rb, _value)
#define PNDV_RQB_GET_RESPONSE(_rb)                          LSA_RQB_GET_RESPONSE(_rb)

#define PNDV_SYSPATH_SET_HD(sys_path, val)                  PSI_SYSPATH_SET_HD(sys_path, val)
#define PNDV_SYSPATH_SET_IF(sys_path, val)                  PSI_SYSPATH_SET_IF(sys_path, val)
#define PNDV_SYSPATH_SET_PATH(sys_path, val)                PSI_SYSPATH_SET_PATH(sys_path, val)



/*****************************************************************************/
/*****************************************************************************/
/*******************************                 *****************************/
/*******************************       END       *****************************/
/*******************************                 *****************************/
/*****************************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* reinclude-protection */


#else
    #pragma message ("The header PNDV_CFG.H is included twice or more !")
#endif


/*****************************************************************************/
/*  end of file.                                                             */
/*****************************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
