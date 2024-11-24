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
/*  F i l e               &F: trace_lsa_com.h                           :F&  */
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
/* contents:

        -

*/
/*****************************************************************************/
/* reinclude protection */


#ifndef TRACE_COM_H
#define TRACE_COM_H


/*****************************************************************************/
/*****************************************************************************/
/*******************************                 *****************************/
/*******************************     COMMON      *****************************/
/*******************************                 *****************************/
/*****************************************************************************/
/*****************************************************************************/

typedef LSA_VERSION_TYPE * TRACE_SYS_VERSION_PTR;

#define TRACE_LSA_COMPONENT_ID                   0x0000
#define TRACE_LSA_PREFIX                         "   -TRAC "
#define TRACE_KIND                               /* &K */ 'V'  /* K& */
                                                /* preliminary: 'R': release       */
                                                /*              'C': correction    */
                                                /*              'S': spezial       */
                                                /*              'T': test          */
                                                /*              'B': labor         */
                                                /* prereleased: 'P': pilot         */
                                                /* released:    'V': version       */
                                                /*              'K': correction    */
                                                /*              'D': demonstration */
#define TRACE_VERSION                            /* &V */ 1    /* V& */ /* [1 - 99] */
#define TRACE_DISTRIBUTION                       /* &D */ 1   /* D& */ /* [0 - 99] */
#define TRACE_FIX                                /* &F */ 2    /* F& */ /* [0 - 99] */
#define TRACE_HOTFIX                             /* &H */ 0    /* H& */ /* [0]      */
#define TRACE_PROJECT_NUMBER                     /* &P */ 0    /* P& */ /* [0 - 99] */
                                                             /* At LSA always 0!  */
#define TRACE_INCREMENT                          /* &I */ 0    /* I& */ /* [1 - 99] */
#define TRACE_INTEGRATION_COUNTER                /* &C */ 0    /* C& */ /* [1 - 99] */
#define TRACE_GEN_COUNTER                        /* &G */ 0    /* G& */ /* [1]      */

/*****************************************************************************/
/*****************************************************************************/
/*******************************                 *****************************/
/*******************************       TRACE     *****************************/
/*******************************                 *****************************/
/*****************************************************************************/
/*****************************************************************************/

/* errormodes */
#define    TRACE_ERRORMODE_FATAL               0
#define    TRACE_ERRORMODE_RESET_SOFT          1
#define    TRACE_ERRORMODE_RESET_HARD          2

/* trace init */
#define    TRACE_INIT_DEFAULT                  0
#define    TRACE_INIT_RESTART_AFTER_FATAL_ERR  1


#ifdef __cplusplus
extern "C"
{
#endif

PNIO_VOID          trace_init                (TRACE_UINT16 reset);
PNIO_VOID          trace_open                (PNIO_VOID* trc_buffer, TRACE_UINT32 trc_buffer_len);
PNIO_VOID          trace_change_tracelevel   (TRACE_UINT16 new_level);
TRACE_UINT16       trace_version             (TRACE_UINT16 version_len, TRACE_SYS_VERSION_PTR version_ptr);

TRACE_UINT32       TrcGetBufMax    (PNIO_VOID);
TRACE_UINT8*       TrcGetBufPtr    (TRACE_UINT32 TraceIndex);
TRACE_UINT32       TrcGetBufSize   (TRACE_UINT32 TraceIndex);
TRACE_UINT32       TrcGetFillState (TRACE_UINT32 TraceIndex);
TRACE_UINT32       TrcStoreBuf     (PNIO_VOID);

PNIO_VOID          SaveTraceBuffer    (PNIO_VOID);
PNIO_VOID          RestoreTraceBuffer (PNIO_VOID);

PNIO_VOID          trace_add_entry_00   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel);
PNIO_VOID          trace_add_entry_01   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel, TRACE_UINT32 para1);
PNIO_VOID          trace_add_entry_02   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel, TRACE_UINT32 para1, TRACE_UINT32 para2);
PNIO_VOID          trace_add_entry_03   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel, TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3);
PNIO_VOID          trace_add_entry_04   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel, TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4);
PNIO_VOID          trace_add_entry_05   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel, TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5);
PNIO_VOID          trace_add_entry_06   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel, TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6);
PNIO_VOID          trace_add_entry_07   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel, TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7);
PNIO_VOID          trace_add_entry_08   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel, TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8);
PNIO_VOID          trace_add_entry_09   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel, TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8, TRACE_UINT32 para9);
PNIO_VOID          trace_add_entry_10   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel, TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8, TRACE_UINT32 para9, TRACE_UINT32 para10);
PNIO_VOID          trace_add_entry_11   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel, TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8, TRACE_UINT32 para9, TRACE_UINT32 para10, TRACE_UINT32 para11);
PNIO_VOID          trace_add_entry_12   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel, TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8, TRACE_UINT32 para9, TRACE_UINT32 para10, TRACE_UINT32 para11, TRACE_UINT32 para12);
PNIO_VOID          trace_add_entry_13   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel, TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8, TRACE_UINT32 para9, TRACE_UINT32 para10, TRACE_UINT32 para11, TRACE_UINT32 para12, TRACE_UINT32 para13);
PNIO_VOID          trace_add_entry_14   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel, TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8, TRACE_UINT32 para9, TRACE_UINT32 para10, TRACE_UINT32 para11, TRACE_UINT32 para12, TRACE_UINT32 para13, TRACE_UINT32 para14);
PNIO_VOID          trace_add_entry_15   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel, TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8, TRACE_UINT32 para9, TRACE_UINT32 para10, TRACE_UINT32 para11, TRACE_UINT32 para12, TRACE_UINT32 para13, TRACE_UINT32 para14, TRACE_UINT32 para15);
PNIO_VOID          trace_add_entry_16   (TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel, TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8, TRACE_UINT32 para9, TRACE_UINT32 para10, TRACE_UINT32 para11, TRACE_UINT32 para12, TRACE_UINT32 para13, TRACE_UINT32 para14, TRACE_UINT32 para15, TRACE_UINT32 para16);


PNIO_VOID          trace_response       (PNIO_VOID* buff, PNIO_UINT buf_length);

TRACE_UINT16       trace_get_fatalerror_mode (PNIO_VOID);

typedef enum TRACE_SOURCE_E
{
	TRACE_SOURCE_FLASH,
	TRACE_SOURCE_RAM
} TRACE_SOURCE;

/* trace config/status */
typedef struct trace_cfg_tag
{
    TRACE_UINT16   cfg_checksum;            /* config checksum */
    TRACE_UINT16   cfg_len;                 /* config length */
    TRACE_UINT16   trace_version;           /* trace-version */
    TRACE_UINT16   reserved;                /* reserved (alignment) */
    TRACE_UINT8    software_revision[4];    /* apma_dat.h: FW-Ausgabestand Vx.y.z    */
    TRACE_UINT32   uptime;                  /* APMA_GET_MS_COUNT(); */
    TRACE_UINT16   fatalerror_mode;         /* fatal error? */
    TRACE_UINT16   lsa_subsys_num;          /* # lsa-tracelevel */
    TRACE_UINT16   maxtracelevel[TRACE_SUBSYS_NUM];  /* tracelevel-array */
    TRACE_SOURCE   source;
} TRACE_CFG;


#ifdef __cplusplus
}
#endif


/*****************************************************************************/
/* reinclude-protection */


#else
    #pragma message ("The header TRACE_COM.H is included twice or more !")
#endif


/*** end of file *************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
