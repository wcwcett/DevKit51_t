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
/*  F i l e               &F: pnpb_com.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  export information.                                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* reinclude protection */
#ifndef PNPB_COM_H
#define PNPB_COM_H


#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

#define PNPB_INVALID_INDEX32     0xffffffff
#define PNPB_INVALID_INDEX       PNPB_INVALID_INDEX32

/*****************************************************************************/
/*****************************************************************************/
/*******************************                 *****************************/
/*******************************     COMMON      *****************************/
/*******************************                 *****************************/
/*****************************************************************************/
/*****************************************************************************/

// *---------------------------------------------------
// *  structure of IN data (see PNIO specification):
// *
// *   REC_IO_INPUT_STRUCT
// *---------------------------------------------------
//lint -e(578) Declaration of symbol 'Symbol' hides symbol 'Symbol' (Location)
typedef ATTR_PNIO_PACKED_PRE struct
{
   REC_IO_BLOCKHDR      BlockHeader;                // BlockHeader
   PNIO_UINT8           LengthIOCS;                 // LengthIOCS
   PNIO_UINT8           IOCS;                       // iocs
   PNIO_UINT8           LengthIOPS;                 // LengthIOPS
   PNIO_UINT8           IOPS;                       // iops
   PNIO_UINT16          LengthData;                 // LengthData
}  ATTR_PNIO_PACKED REC_IO_INPUT_STRUCT; // more details see pnio spec


// *---------------------------------------------------
// *  structure of OUT data (see PNIO specification):
// *
// *   REC_IO_OUTPUT_STRUCT         (with BlockHeader type = 0x0016 = output)
// *   iocs                         (iocs value)
// *   OutputData                   (normal output data)
// *   iops                         (iops value)
// *   REC_IO_SUBSTVAL_STRUCT       (with BlockHeader type = 0x0014 = substituted)
// *   iocs                         (iocs value)
// *   SubstituteData               (substitute output data)
// *   Substitute data valid        (coded as iops and iocs)
// *---------------------------------------------------
typedef ATTR_PNIO_PACKED_PRE struct
{
   REC_IO_BLOCKHDR      BlockHeader;                // BlockHeader
   PNIO_UINT16          SubstActiveFlag;            // SubstituteActiveFlag:  0=operation, 1=substitute
   PNIO_UINT8           LengthIOCS;                 // LengthIOCS
   PNIO_UINT8           LengthIOPS;                 // LengthIOPS
   PNIO_UINT16          LengthData;                 // LengthData
}  ATTR_PNIO_PACKED REC_IO_OUTPUT_STRUCT; // more details see pnio spec

typedef ATTR_PNIO_PACKED_PRE struct
{
   REC_IO_BLOCKHDR      BlockHeader;                // BlockHeader
   PNIO_UINT16          SubstMode;                  // SubstitutionMode: 0=ZERO or inactive, 1:LastValue, 2:Replacement value
}  ATTR_PNIO_PACKED REC_IO_SUBSTVAL_STRUCT;// more details see pnio spec



// *------------------------------------------
// * return codes
// *------------------------------------------
#define PNPB_OK                         ((PNIO_UINT32)0x01)

// *------------------------------------------
// * global pnpb settings
// *------------------------------------------
#define PNPB_MAX_STATION_TYPE_LEN       255

/* PNPB system redundancy addition for ErtecDevKit43 and later*/
#if(IOD_INCLUDE_S2_REDUNDANCY == 1)
#define PNPB_SYSTEM_REDUNDANCY
#endif

#ifdef PNPB_SYSTEM_REDUNDANCY
#define PNPB_MAX_AR                     IOD_CFG_NUMOF_AR
#define PNPB_MAX_S2_AR_SETS             (PNPB_MAX_AR/2)
#define REDUNDANT_PRIMARY_STATUS_BIT    0x00010000


typedef enum
{
    PNPB_IO_DATA_STATE_P_B_BACKUP             = 0,
    PNPB_IO_DATA_STATE_P_B_PRIMARY            = 1,
    PNPB_IO_DATA_STATE_P_B_BACKUP_PROCEEDING  = 2,
    PNPB_IO_DATA_STATE_P_B_PRIMARY_PROCEEDING = 3,
    PNPB_IO_DATA_STATE_P_B_UNDETERMINED       = 0xFF
} PNPB_IO_DATA_STATE_P_B;

typedef enum
{
    PNPB_S2_STATE_NO_AR_OR_ALL_BACKUP        = 0,
    PNPB_S2_STATE_PRIMARY                    = 1,
    PNPB_S2_STATE_HOLD_O_DATA                = 2
} PNPB_S2_STATE;


PNIO_AR_TYPE pnpb_ArType[ PNPB_MAX_AR + 1 ];

typedef enum
{
    PNPB_S2_AR_PRESENT,
    PNPB_NO_S2_AR
}PNPB_S2_PRESENT;
PNPB_S2_PRESENT pnpb_s2_present;

typedef struct PNPB_DATA
{
    struct
    {
        struct
        {
            PNIO_UINT32             rdht[PNPB_MAX_S2_AR_SETS];
            PNIO_UINT32             rdht_running_timer_val[PNPB_MAX_S2_AR_SETS];
            PNPB_IO_DATA_STATE_P_B  primary_backup_state[ PNPB_MAX_AR + 1 ];        /* holds the current state of the primary bit for each ar                 */
            PNIO_UINT16             current_io_controller_idx[PNPB_MAX_S2_AR_SETS]; /* the ar index of the latest controller that went from backup to primary */
            PNPB_S2_STATE           state[PNPB_MAX_S2_AR_SETS];                     /* the state machine due to figure #30 of the draft spec                  */
        }s2;
    }io;
    struct
    {
        PNIO_UINT8  rqb_in_use;
        PNIO_UINT16 ident;
    }timer[PNPB_MAX_S2_AR_SETS];

}PNPB_DATA;

PNPB_DATA pnpb_data;

PNIO_EXP    PnpbExp[IOD_CFG_NUMOF_AR];
PNIO_UINT32 Pnpb_ar_sr_set[IOD_CFG_NUMOF_AR];

/*new macros*/
#define PNPB_IN_SET_DATA_STATE(_ar_no, _value, _mask)                                                                                               \
{                                                                                                                                                   \
    PNPB_SYS_TRACE_03(LSA_TRACE_LEVEL_NOTE, "set_data_state [arno:%x arstate:%x ar_set:%x]", _ar_no, Pnpb.ArState[_ar_no], Pnpb_ar_sr_set[_ar_no]); \
    iom_provider_set_data_state(_ar_no, _value, _mask);                                                                                             \
}

#define PNPB_IN_SET_PRIMARY_ARID(_ar_no)    iom_set_session_key_primary_arid(_ar_no)

#define pnpb_ar_in_data__(_idx, _ArState)   ( ((_idx) < PNPB_MAX_AR) && ((_ArState[(_idx)] == PNPB_AR_INDATA_IND) || (_ArState[(_idx)] == PNPB_AR_SM_INDATA_IND) ) )
#define pnpb_ar_prm_ok__(_idx, _ArState)    ( ((_idx) < PNPB_MAX_AR) && ((_ArState[(_idx)] >= PNPB_AR_PARAM_END_FINISHED) ) )
#define pnpb_max__(_a,_b)                   ( (_a) > (_b) ? (_a) : (_b) )

#define PNPB_AR_OTHER(_ar)                  pnpb_second_ar_of_arset( _ar )
#define PNPB_AR_SET(_ar, _psr)                                                                              \
{                                                                                                           \
    *_psr = Pnpb_ar_sr_set[ _ar ];                                                                          \
    if( 0 == *_psr )                                                                                        \
    {                                                                                                       \
        /*0 is for non redundant - this should not happen*/                                                 \
        PNPB_SYS_TRACE_01( LSA_TRACE_LEVEL_FATAL, "Trying to obtain sr_set of non-redundant AR %x", _ar );  \
    }                                                                                                       \
}

/* function prototypes */
PNIO_UINT32      pnpb_initiate_s2_data_read    (PNIO_UINT32 DevHndl);
PNIO_UINT32      pnpb_initiate_s2_data_write   (PNIO_UINT32   DevHndl);
PNIO_VOID        pnpb_io_s2_ar_set_trigger_cnf (PNIO_UINT32 ar_idx, PNIO_UINT8 edge);
PNIO_VOID        pnpb_io_s2_ar_set_trigger_req (PNIO_UINT32 ar_idx);
PNIO_UINT8       pnpb_io_s2_ar_release         (PNIO_UINT32 ar_idx);
PNIO_VOID        pnpb_rdht_timeout             (PNIO_UINT32 ArSet);
PNIO_VOID        pnpb_trigger_rdht_timeout     (LSA_UINT16 timer_id, LSA_USER_ID_TYPE user_id);
PNIO_UINT32      pnpb_second_ar_of_arset       (PNIO_UINT32 first_ar);
#else
PNIO_EXP    PnpbExp[IOD_CFG_NUMOF_AR];
#endif /* end of PNPB_SYSTEM_REDUNDANCY */

PNDV_IFACE_STRUCT_PTR    pPnpbIf;

// *------------------------------------------
// *open parameter type of the PNPB component
// *------------------------------------------
typedef struct pnpb_open_parameter_s
{
    PNIO_UINT32 redundancy;
    PNDV_IFACE_STRUCT_PTR pndvInterface;
    PNIO_VOID (*done_cbf)(PNIO_VOID);

} pnpb_open_parameter_t;

typedef pnpb_open_parameter_t* pnpb_sys_parameter_ptr_t;

// *------------------------------------------
// * exported function prototypes
// *------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif

PNIO_VOID            pnpb_init                  (PNIO_VOID);
PNIO_VOID            pnpb_open                  (pnpb_sys_parameter_ptr_t parameter_ptr);
PNIO_VOID            pnpb_close                 (PNIO_VOID);
PNIO_VOID            pnpb_activate              (PNIO_VOID);
PNIO_VOID            pnpb_deactivate            (PNIO_VOID);
PNIO_VOID            pnbp_StartTaskPost         (PNIO_VOID);
PNIO_VOID            pnpb_perform_services      (PNIO_VOID);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif


/*****************************************************************************/
/* reinclude-protection */
#endif


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
