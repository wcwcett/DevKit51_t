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
/*  F i l e               &F: trace_lsa_dat.h                           :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  internal structures & data                                               */
/*                                                                           */
/*****************************************************************************/



/*****************************************************************************/
/* contents:

    - internal structures
    - internal data

*/
/*****************************************************************************/
/* reinclude protection */


#ifndef TRACE_DAT_H
#define TRACE_DAT_H


/*****************************************************************************/
/* trace version (8bit) */
#define TRACE_VERSION_INTERN                   14        /* v1.4 */

/* maximum amount of parameters */
#define    TRACE_MAX_PARA                      16

/* size of ping_buffer_header in bytes */
#define    TRACE_PBHEADER_SIZE                 20

/* trace_response() */
#define    TRACE_RESPONSE_CHECKSUM_CLEAR       0x0000
/* opcodes */
#define    TRACE_RESPONSE_OPCODE_SETTRACELVL   0x0001
#define    TRACE_RESPONSE_OPCODE_GETTRACE      0x0002
#define    TRACE_RESPONSE_OPCODE_ERRORMODE     0x0003
#define    TRACE_RESPONSE_OPCODE_STATUS        0x0004
#define    TRACE_RESPONSE_OPCODE_GOTODEFECT    0x0005    /* goto defect */

#define    TRACE_RESPONSE_OPCODE_WRONG_CKSUM   0xFF00


/** params **/
#define    TRACE_RESPONSE_PARAM_ERROR          0xFFFF

#define    TRACE_RESPONSE_PARAM_RESET_SET      0x00FF
#define    TRACE_RESPONSE_PARAM_TRACELVL_SET   0x01FF
#define    TRACE_RESPONSE_PARAM_ERRMODE_SET    0x03FF

/* get trace */
#define    TRACE_RESPONSE_PARAM_SEQ_OOR        0xEEEE
#define    TRACE_RESPONSE_PARAM_SEQ_FULL       0xEE00
#define    TRACE_RESPONSE_PARAM_SEQ_END        0xEEAA

/* errormode */
#define    TRACE_RESPONSE_PARAM_ERRMODE_FATAL      0x0000
#define    TRACE_RESPONSE_PARAM_ERRMODE_RESETSOFT  0x0001
#define    TRACE_RESPONSE_PARAM_ERRMODE_RESETHARD  0x0002

/* status */
#define    TRACE_RESPONSE_PARAM_STATUS_SET     0x00FF



/*****************************************************************************/
/* internal structures */

/* PINGbuffer header */
typedef struct
{
    TRACE_UINT16 opcode;
    TRACE_UINT16 param;
    TRACE_UINT16 buff_len;
    TRACE_UINT16 seq;
    union
    {
        TRACE_UINT32 raw_addr;
        struct
        {
            TRACE_UINT16 raw_addr_lo;
            TRACE_UINT16 raw_addr_hi;
        } struct_raw_addr;
    } union_raw_addr;

    union
    {
        TRACE_UINT32 raw_len;
        struct
        {
            TRACE_UINT16 raw_len_lo;
            TRACE_UINT16 raw_len_hi;
        } struct_raw_len;
    } union_raw_len;

    TRACE_UINT16 reserved;
    TRACE_UINT16 checksum;
} PING_BUFFER_HEADER;


/* trace buffer */
typedef struct
{
    TRC_TPH_TYPE tph;
    TRACE_UINT32 element[1];
} TRACE_BUFFER;

/* traceentry header */
typedef struct
{
    TRACE_UINT32   timer_high;        /* APMA_GET_MS_COUNT(); */
    TRACE_UINT16   subsystem;         /* subsystem */
    TRACE_UINT16   module;            /* module */
    TRACE_UINT16   line;              /* linenumber */
    TRACE_UINT8    tracelevel;        /* tracelevel */
    TRACE_UINT8    paramcnt;          /* parameters */
} TRACE_ENTRY_HEADER;                 /* 3 Entries (32) + (16+16) + (16+8+8) */

#define TRACE_ENTRY_HEADER_LENGTH 4   /* (32bit) TIMER_LOW + TRACE_ENTRY_HEADER */


/*****************************************************************************/
/* extern declaration */


#ifndef TRACE_EXTERN_ATTR

    #define TRACE_EXTERN_ATTR extern

#endif

/*****************************************************************************/
/* internal data */

/* trace config */
TRACE_EXTERN_ATTR    TRACE_CFG    trace_config;

/* save reset-mode for trace_open() */
TRACE_EXTERN_ATTR    TRACE_UINT16 trace_reset;

/* size of trace_buffer (32bit) */
TRACE_EXTERN_ATTR    TRACE_UINT32 trace_buffer_len;

/* tracebuffer */
TRACE_EXTERN_ATTR    TRACE_BUFFER *trace_buffer;

/* pndv tracebuffer */
TRACE_EXTERN_ATTR    PNIO_VOID*      trace_buffer_pndv;
TRACE_EXTERN_ATTR    LSA_UINT32 trace_buffer_pndv_size;

TRACE_EXTERN_ATTR    PNIO_UINT32 tracebuffer[TRACE_BUFFER_SIZE];



/*****************************************************************************/
/* reinclude-protection */


#else
    #pragma message ("The header TRACE_DAT.H is included twice or more !")
#endif


/*** end of file *************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
