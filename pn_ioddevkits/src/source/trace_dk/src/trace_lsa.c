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
/*  F i l e               &F: trace_lsa.c                               :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  trace                                                                    */
/*                                                                           */
/*****************************************************************************/



/*****************************************************************************/
/* contents:

    - trace_init
    - TRACE_ADD_HEADER
    - trace_add_entryNN

*/

/* include hierarchy */

#include "compiler.h"
#if (PNIO_TRACE == PNIO_TRACE_DK_LSA)

#include "usriod_cfg.h"
#include "version_dk.h"
#include "trace_dk.h"

#include "trace_lsa_inc.h"
#include "trace_lsa_com.h"
#include "trace_lsa_dat.h"

#include "pndv_inc.h"		/* CM includes are needed as well */

/* module ID */
#define TRACE_MODULE_ID    10
#define LTRC_ACT_MODUL_ID  10


//#define TRACE_SUBSYS_NUM 	MAXNUM_SUBSYSTEMS
//#define TRACE_SUBSYS_TRACE	TRACE_SUBSYS_DUMMY

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**********                                                         **********/
/**********               SYSTEM INTERFACE FUNCTIONS                **********/
/**********                                                         **********/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


/*****************************************************************************/

/* initialization */
PNIO_VOID trace_init(TRACE_UINT16 reset)
{
    PNIO_INT c=0;
    trace_config.cfg_checksum         = 0;
    trace_config.cfg_len              = sizeof(TRACE_CFG);
    trace_config.reserved             = 0xAFFE;
    /* FW-Version */
    trace_config.software_revision[0] =  TRACE_FW_VERSION_0;
    trace_config.software_revision[1] =  TRACE_FW_VERSION_1;
    trace_config.software_revision[2] =  TRACE_FW_VERSION_2;
    trace_config.software_revision[3] =  TRACE_FW_VERSION_3;
    /* trace-version */
    trace_config.trace_version        =  TRACE_VERSION_INTERN;
    /* uptime */
    trace_config.uptime               = 0;
    /* LSA subsystems */
    trace_config.lsa_subsys_num       = TRACE_SUBSYS_NUM;

    /* LSA subsystem-array */
    for (c=TRACE_SUBSYS_TRACE; c<TRACE_SUBSYS_NUM; ++c)
    {
        trace_config.maxtracelevel[c] = TRACE_NO_TRACE; /* no trace! */
    }
    /* fatal error */
    trace_config.fatalerror_mode      = TRACE_DEFAULTCONFIG_ERRORMODE;
    /* save for trace_open */
    trace_reset                       = reset;
}

/* open */
PNIO_VOID trace_open(PNIO_VOID*trc_buffer, TRACE_UINT32 trc_buffer_len)
{
    PNIO_INT c=0;
    TRACE_CFG temp;
	PNDV_STRUC_DEBUG_INFO pndv_debug_info;


    trace_buffer = (TRACE_BUFFER*)trc_buffer;
    trace_buffer_len = trc_buffer_len - sizeof(TRC_TPH_TYPE);
    trace_buffer_len = trace_buffer_len/4;

	pndv_debug_get_info (&pndv_debug_info);
    trace_buffer_pndv      = (PNIO_VOID*) (pndv_debug_info.trace_data_ptr);
    trace_buffer_pndv_size =  pndv_debug_info.trace_data_len;

    switch (trace_reset)                                        /* value from trace_init() */
    {
        case TRACE_INIT_RESTART_AFTER_FATAL_ERR:                /* no init due to reboot after fatal error */
        {
            for (c=TRACE_SUBSYS_TRACE; c<TRACE_SUBSYS_NUM; ++c)
            {
                trace_config.maxtracelevel[c] = TRACE_NO_TRACE;
            }
            trace_config.fatalerror_mode = TRACE_ERRORMODE_RESET_SOFT;
            break; /* case TRACE_INIT_RESTART_AFTER_FATAL_ERR */
        }
        case TRACE_INIT_DEFAULT:
        /* no break */
        default:                                                /* init tracebuffer */
        {
            for (c=TRACE_SUBSYS_TRACE; c<TRACE_SUBSYS_NUM; ++c)
            {
                trace_config.maxtracelevel[c] = TRACE_DEFAULTCONFIG_TRACELEVEL;
            }
            trace_config.fatalerror_mode = TRACE_DEFAULTCONFIG_ERRORMODE;     /* fatalerror */

            TRACE_MEMSET(trc_buffer, 0x00, trc_buffer_len);                         /* initialize buffer */

            /* do not use TRC_TP_INIT() */
            trace_buffer->tph.tph_size             = sizeof( trace_buffer->tph);
            trace_buffer->tph.tph_version          = TRC_TPH_VERSION;
            trace_buffer->tph.byte_ordering        = TRC_BYTE_ORDERING;
            trace_buffer->tph.trace_id             = TRC_ID_TRACE;

            trace_buffer->tph.entry_size           = TRACE_ENTRY_HEADER_LENGTH;     /* just the header! */
            trace_buffer->tph.teh_size             = 4; /*  size of timestamp_low | old: sizeof( TRC_TEH_TYPE.); */
            trace_buffer->tph.teh_version          = TRC_TEH_LSASUBSYS2;            /* set TEH_version to LSA */
            trace_buffer->tph.timestamp_size       = 4; /*  size of timestamp_low | old: sizeof( TRC_TEH_TYPE);*/

            trace_buffer->tph.trc_syntax_version   = TRC_SYNTAX_VERSION;
            trace_buffer->tph.processor_id         = TRC_PROCESSOR_ID;
            trace_buffer->tph.process_id           = TRC_PROCESS_ID;
            trace_buffer->tph.alignment            = TRC_ALIGNMENT;

            if (TRC_TEH_LSASUBSYS2 == trace_buffer->tph.teh_version)
            {
            	/* max element number cannot be calculated, as TRC_TEH_LSASUBSYS2 uses variable number of parameters */
                trace_buffer->tph.debug_element_number = 0xFFFC;
            }
            else
            {
                if (trace_buffer_len > 65535)
                {
                    trace_buffer_len = 65536-4;
                }
                trace_buffer->tph.debug_element_number = (PNIO_UINT16) trace_buffer_len;   /* length of trace buffer (32bit) */
            }
            trace_buffer->tph.buffer_index         = 0;

            trace_buffer->tph.timestamp_resolution = TRC_TIMESTAMP_RESOLUTION;
            trace_buffer->tph.max_timestamp        = TRC_MAX_TIMESTAMP;
            trace_buffer->tph.last_counter         = 0;


            TRACE_READ_CFG(&temp, sizeof(TRACE_CFG));

            if (temp.software_revision[0] == TRACE_FW_VERSION_0
            &&  temp.software_revision[1] == TRACE_FW_VERSION_1
            &&  temp.software_revision[2] == TRACE_FW_VERSION_2
            &&  temp.software_revision[3] == TRACE_FW_VERSION_3)
            {
                for(c=0; c<TRACE_SUBSYS_NUM; c++)
                {
                    trace_config.maxtracelevel[c] = temp.maxtracelevel[c];
                }
                trace_config.fatalerror_mode = temp.fatalerror_mode;
            }
            else /* wrong software revision or no config */
            {
                /* write default trace config to FS */
                TRACE_WRITE_CFG(&trace_config, sizeof(trace_config) );
            }

            break; /* case default */
        }
    } /* switch (reset) */
}

PNIO_VOID trace_change_tracelevel   (TRACE_UINT16 new_level)
{
    PNIO_INT c;

    for (c=TRACE_SUBSYS_TRACE; c<TRACE_SUBSYS_NUM; ++c)
    {
        trace_config.maxtracelevel[c] = new_level;
    }
}

/*
add traceentry-header to tracebuffer
*/
#define TRACE_ADD_HEADER(subsystem, module, line, tracelevel, count, trace_buffer_counter, trace_entry_header)        \
{                                                                                                                     \
    if ( trace_buffer_counter >= trace_buffer_len - (TRACE_ENTRY_HEADER_LENGTH + TRACE_MAX_PARA ) )                   \
    {                                                                                                                 \
        trace_buffer_counter = 0;                                                                                     \
        /* write overflow-entry */                                                                                    \
        trace_buffer->element[trace_buffer_counter++] = (TRACE_UINT32)(TRACE_TIMER_LOW);                              \
        trace_entry_header = (TRACE_ENTRY_HEADER*)&trace_buffer->element[trace_buffer_counter];                       \
        trace_entry_header->timer_high = (TRACE_UINT32)(TRACE_TIMER_HIGH);                                            \
        trace_entry_header->subsystem = 0xEEE0;                                                                       \
        trace_entry_header->module = 0xEEE0;                                                                          \
        trace_entry_header->line = 0xEEE0;                                                                            \
        trace_entry_header->tracelevel = 0x0;                                                                         \
        trace_entry_header->paramcnt = 1;                                                                             \
        trace_buffer_counter += 3;                                                                                    \
        trace_buffer->element[trace_buffer_counter++] = TRACE_TIMER_HIGH;                                             \
    }                                                                                                                 \
    trace_buffer->element[trace_buffer_counter++] = (TRACE_UINT32)(TRACE_TIMER_LOW);                                  \
    trace_entry_header = (TRACE_ENTRY_HEADER*)&trace_buffer->element[trace_buffer_counter];                           \
    trace_entry_header->timer_high = (TRACE_UINT32)(TRACE_TIMER_HIGH);                                                \
    trace_entry_header->subsystem = subsystem;                                                                        \
    trace_entry_header->module = module;                                                                              \
    trace_entry_header->line = line;                                                                                  \
    trace_entry_header->tracelevel = tracelevel;                                                                      \
    trace_entry_header->paramcnt = count;                                                                             \
    trace_buffer_counter += 3;                                                                                        \
}

/*
trace_add_entry_NN functions
*/
PNIO_VOID trace_add_entry_00(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel)
{
    TRACE_ENTRY_HEADER * trace_entry_header;                                /* pointer to traceentry header */
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )        /* trace this entry ? */
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 0, trace_buffer->tph.buffer_index, trace_entry_header);
    TRACE_EXIT();
    }
}
PNIO_VOID trace_add_entry_01(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel,
        TRACE_UINT32 para1)
{
    TRACE_ENTRY_HEADER * trace_entry_header;
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 1, trace_buffer->tph.buffer_index, trace_entry_header);
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para1;
    TRACE_EXIT();
    }
}
PNIO_VOID trace_add_entry_02(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel,
        TRACE_UINT32 para1, TRACE_UINT32 para2)
{
    TRACE_ENTRY_HEADER * trace_entry_header;
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 2, trace_buffer->tph.buffer_index, trace_entry_header);
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para1;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para2;
    TRACE_EXIT();
    }
}
PNIO_VOID trace_add_entry_03(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel,
        TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3)
{
    TRACE_ENTRY_HEADER * trace_entry_header;
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 3, trace_buffer->tph.buffer_index, trace_entry_header);
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para1;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para2;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para3;
    TRACE_EXIT();
    }
}
PNIO_VOID trace_add_entry_04(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel,
        TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4)
{
    TRACE_ENTRY_HEADER * trace_entry_header;
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 4, trace_buffer->tph.buffer_index, trace_entry_header);
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para1;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para2;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para3;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para4;
    TRACE_EXIT();
    }
}
PNIO_VOID trace_add_entry_05(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel,
        TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5)
{
    TRACE_ENTRY_HEADER * trace_entry_header;
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 5, trace_buffer->tph.buffer_index, trace_entry_header);
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para1;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para2;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para3;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para4;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para5;
    TRACE_EXIT();
    }
}
PNIO_VOID trace_add_entry_06(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel,
        TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6)
{
    TRACE_ENTRY_HEADER * trace_entry_header;
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 6, trace_buffer->tph.buffer_index, trace_entry_header);
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para1;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para2;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para3;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para4;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para5;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para6;
    TRACE_EXIT();
    }
}
PNIO_VOID trace_add_entry_07(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel,
        TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7)
{
    TRACE_ENTRY_HEADER * trace_entry_header;
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 7, trace_buffer->tph.buffer_index, trace_entry_header);
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para1;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para2;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para3;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para4;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para5;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para6;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para7;
    TRACE_EXIT();
    }
}
PNIO_VOID trace_add_entry_08(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel,
        TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8)
{
    TRACE_ENTRY_HEADER * trace_entry_header;
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 8, trace_buffer->tph.buffer_index, trace_entry_header);
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para1;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para2;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para3;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para4;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para5;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para6;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para7;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para8;
    TRACE_EXIT();
    }
}
PNIO_VOID trace_add_entry_09(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel,
        TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8, TRACE_UINT32 para9)
{
    TRACE_ENTRY_HEADER * trace_entry_header;
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 9, trace_buffer->tph.buffer_index, trace_entry_header);
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para1;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para2;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para3;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para4;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para5;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para6;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para7;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para8;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para9;
    TRACE_EXIT();
    }
}
PNIO_VOID trace_add_entry_10(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel,
        TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8, TRACE_UINT32 para9, TRACE_UINT32 para10)
{
    TRACE_ENTRY_HEADER * trace_entry_header;
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 10, trace_buffer->tph.buffer_index, trace_entry_header);
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para1;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para2;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para3;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para4;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para5;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para6;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para7;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para8;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para9;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para10;
    TRACE_EXIT();
    }
}
PNIO_VOID trace_add_entry_11(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel,
        TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8, TRACE_UINT32 para9, TRACE_UINT32 para10, TRACE_UINT32 para11)
{
    TRACE_ENTRY_HEADER * trace_entry_header;
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 11, trace_buffer->tph.buffer_index, trace_entry_header);
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para1;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para2;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para3;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para4;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para5;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para6;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para7;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para8;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para9;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para10;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para11;
    TRACE_EXIT();
    }
}
PNIO_VOID trace_add_entry_12(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel,
        TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8, TRACE_UINT32 para9, TRACE_UINT32 para10, TRACE_UINT32 para11, TRACE_UINT32 para12)
{
    TRACE_ENTRY_HEADER * trace_entry_header;
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 12, trace_buffer->tph.buffer_index, trace_entry_header);
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para1;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para2;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para3;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para4;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para5;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para6;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para7;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para8;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para9;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para10;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para11;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para12;
    TRACE_EXIT();
    }
}
PNIO_VOID trace_add_entry_13(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel,
        TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8, TRACE_UINT32 para9, TRACE_UINT32 para10, TRACE_UINT32 para11, TRACE_UINT32 para12, TRACE_UINT32 para13)
{
    TRACE_ENTRY_HEADER * trace_entry_header;
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 13, trace_buffer->tph.buffer_index, trace_entry_header);
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para1;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para2;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para3;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para4;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para5;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para6;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para7;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para8;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para9;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para10;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para11;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para12;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para13;
    TRACE_EXIT();
    }
}
PNIO_VOID trace_add_entry_14(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel,
        TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8, TRACE_UINT32 para9, TRACE_UINT32 para10, TRACE_UINT32 para11, TRACE_UINT32 para12, TRACE_UINT32 para13, TRACE_UINT32 para14)
{
    TRACE_ENTRY_HEADER * trace_entry_header;
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 14, trace_buffer->tph.buffer_index, trace_entry_header);
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para1;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para2;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para3;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para4;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para5;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para6;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para7;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para8;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para9;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para10;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para11;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para12;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para13;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para14;
    TRACE_EXIT();
    }
}
PNIO_VOID trace_add_entry_15(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel,
        TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8, TRACE_UINT32 para9, TRACE_UINT32 para10, TRACE_UINT32 para11, TRACE_UINT32 para12, TRACE_UINT32 para13, TRACE_UINT32 para14, TRACE_UINT32 para15)
{
    TRACE_ENTRY_HEADER * trace_entry_header;
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 15, trace_buffer->tph.buffer_index, trace_entry_header);
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para1;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para2;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para3;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para4;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para5;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para6;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para7;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para8;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para9;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para10;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para11;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para12;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para13;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para14;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para15;
    TRACE_EXIT();
    }
}
PNIO_VOID trace_add_entry_16(TRACE_UINT16 subsystem, TRACE_UINT16 module, TRACE_UINT16 line, TRACE_UINT8 tracelevel,
        TRACE_UINT32 para1, TRACE_UINT32 para2, TRACE_UINT32 para3, TRACE_UINT32 para4, TRACE_UINT32 para5, TRACE_UINT32 para6, TRACE_UINT32 para7, TRACE_UINT32 para8, TRACE_UINT32 para9, TRACE_UINT32 para10, TRACE_UINT32 para11, TRACE_UINT32 para12, TRACE_UINT32 para13, TRACE_UINT32 para14, TRACE_UINT32 para15, TRACE_UINT32 para16)
{
    TRACE_ENTRY_HEADER * trace_entry_header;
    if ( tracelevel <= trace_config.maxtracelevel[subsystem] )
    {
    TRACE_ENTER();
        TRACE_ADD_HEADER(subsystem, module, line, tracelevel, 16, trace_buffer->tph.buffer_index, trace_entry_header);
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para1;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para2;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para3;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para4;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para5;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para6;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para7;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para8;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para9;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para10;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para11;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para12;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para13;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para14;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para15;
        trace_buffer->element[trace_buffer->tph.buffer_index++] = para16;
    TRACE_EXIT();
    }
}



LSA_UINT16 trace_version( LSA_UINT16 version_len, TRACE_SYS_VERSION_PTR version_ptr)
{
    if ( sizeof( LSA_VERSION_TYPE) <= version_len )
    {
        PNIO_UINT32 i;
        PNIO_UINT8  tmp_prefix[] = TRACE_LSA_PREFIX;

        version_ptr->lsa_component_id = TRACE_LSA_COMPONENT_ID;

        for ( i = 0; LSA_PREFIX_SIZE > i; i++ )
        {
            version_ptr->lsa_prefix[i] = tmp_prefix[i];
        }

        version_ptr->kind                = TRACE_KIND;
        version_ptr->version             = TRACE_VERSION;
        version_ptr->distribution        = TRACE_DISTRIBUTION;
        version_ptr->fix                 = TRACE_FIX;
        version_ptr->hotfix              = TRACE_HOTFIX;
        version_ptr->project_number      = TRACE_PROJECT_NUMBER;
        version_ptr->increment           = TRACE_INCREMENT;
        version_ptr->integration_counter = TRACE_INTEGRATION_COUNTER;
        version_ptr->gen_counter         = TRACE_GEN_COUNTER;

        return( 0);
    }
    else
    {
        return( version_len);
    }
}

#define TRACE_BUFFER_COUNT    2

// *----------------------------------------------------------------*
// *   TrcGetBufMax    (PNIO_VOID)
// *----------------------------------------------------------------*
// *   returns the number of tracebuffers.
// *
// *  input :  --
// *  out   :  number of trace buffers
// *----------------------------------------------------------------*
TRACE_UINT32    TrcGetBufMax    (PNIO_VOID)
{
    return (TRACE_BUFFER_COUNT);
}


// *----------------------------------------------------------------*
// *   TrcGetBufPtr    (PNIO_VOID)
// *----------------------------------------------------------------*
// *   returns the start address of the Tracebuffer.
// *
// *  input :  --
// *  out   :  pTrcBbuf          pointer to the trace buffer
// *----------------------------------------------------------------*
TRACE_UINT8*    TrcGetBufPtr    (TRACE_UINT32 TraceIndex)
{
	switch (TraceIndex)
	{
		case 0 : return ((PNIO_VOID*) tracebuffer); // LSA trace
		case 1 : return ((PNIO_VOID*) trace_buffer_pndv);
		default: return NULL;
	}
}

// *----------------------------------------------------------------*
// *   TrcGetBufSize    (PNIO_VOID)
// *----------------------------------------------------------------*
// *   returns the length of the tracebuffer.
// *
// *  input :  --
// *  out   :  pTrcBbuf          pointer to the trace buffer
// *----------------------------------------------------------------*
TRACE_UINT32    TrcGetBufSize    (TRACE_UINT32 TraceIndex)
{
	switch (TraceIndex)
	{
		case 0 : return (sizeof(tracebuffer));	// LSA trace
		case 1 : return (trace_buffer_pndv_size);
		default: return 0;
	}
}


// *----------------------------------------------------------------*
// *   TrcGetFillState    (PNIO_VOID)
// *----------------------------------------------------------------*
// *   returns the length of the tracebuffer.
// *
// *  input :  --
// *  out   :  pTrcBbuf          pointer to the trace buffer
// *----------------------------------------------------------------*
TRACE_UINT32    TrcGetFillState   (TRACE_UINT32 TraceIndex)
{
	switch (TraceIndex)
	{
		case 0 : return ( ((TRACE_UINT32)&trace_buffer->element[trace_buffer->tph.buffer_index+1])-((TRACE_UINT32)&trace_buffer) );
		case 1 : return (trace_buffer_pndv_size);  // fill state is not available
		default: return 0;
	}
}


PNIO_VOID SaveTraceBuffer (PNIO_VOID)
{
	PNIO_printf ("SaveTraceBuffer not supported.\n");
}

PNIO_VOID RestoreTraceBuffer (PNIO_VOID)
{
	PNIO_printf ("RestoreTraceBuffer not supported.\n");
}

#endif /* PNIO_TRACE == PNIO_TRACE_DK_LSA */

/*** end of file *************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
