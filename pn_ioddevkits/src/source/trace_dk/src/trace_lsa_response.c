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
/*  F i l e               &F: trace_lsa_response.c                      :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  trace_response                                                           */
/*                                                                           */
/*****************************************************************************/



/*****************************************************************************/
/* contents:

    - trace_response
*/
/*****************************************************************************/
/* 2do:

*/
/*****************************************************************************/
/* include hierarchy */

#include "compiler.h"
#include "usriod_cfg.h"
#include "lsa_cfg.h"
#include "pnio_types.h"

#if (PNIO_TRACE == PNIO_TRACE_DK_LSA)
#include "trace_dk.h"
#include "trace_lsa_inc.h"
#include "trace_lsa_com.h"
#include "trace_lsa_dat.h"

/* module ID */
#define TRACE_MODULE_ID    142
#define LTRC_ACT_MODUL_ID  142


    /*****************************************************************************/
    /*****************************************************************************/
    /*****************************************************************************/
    /**********                                                         **********/
    /**********               SYSTEM INTERFACE FUNCTIONS                **********/
    /**********                                                         **********/
    /*****************************************************************************/
    /*****************************************************************************/
    /*****************************************************************************/

    TRACE_CKSUM_EXTERN

    /* evaluate ping_buffer_header in buff and fill the data-area with the requested info */
    PNIO_VOID trace_response(PNIO_VOID* buff, PNIO_UINT buf_length)
    {
        TRACE_UINT32 a=0, b=0;                          /* lower/upper limit - ping_buffer */
        TRACE_UINT32 max_buffer=0;                      /* amount of 32bit-values */
        TRACE_UINT32 buff_len=0;                        /* length of buffer */

        PNIO_VOID * tb_seq=0;                           /* sequence test */

        TRACE_UINT16 check_cksum=0, calc_cksum=0;       /* save received checksum; calculate new checksum */
        TRACE_UINT16 trace_opcode_save=0;               /* save opcode if checksum-check fails */

        TRACE_UINT32 trace_raw_addr=0;                  /* RAW-address */
        TRACE_UINT32 trace_raw_len=0;                   /* RAW-length */

        /* autoread-table */
        TRACE_AUTOREAD_ENTRY * trace_autoread_table_ptr=0;
        TRACE_AUTOREAD_ENTRY trace_autoread_table_end;

        /* pingbuffer header */
        PING_BUFFER_HEADER   *ping_buffer_header;

        ping_buffer_header = buff;

        check_cksum = ping_buffer_header->checksum;
        ping_buffer_header->checksum = TRACE_RESPONSE_CHECKSUM_CLEAR;                  /* zero cksum before re-calculating */
        calc_cksum = TRACE_CKSUM(buff, TRACE_PBHEADER_SIZE);
        ping_buffer_header->checksum = check_cksum;                                    /* restore old cksum */

        if (calc_cksum != check_cksum)
        {
            trace_opcode_save = ping_buffer_header->opcode;                            /* save opcode */
            ping_buffer_header->opcode = TRACE_RESPONSE_OPCODE_WRONG_CKSUM;            /* wrong checksum; most likely due to normal ping request! */
        }
        else
        {
            buff_len = ping_buffer_header->buff_len;
            max_buffer = buff_len-TRACE_PBHEADER_SIZE;
            max_buffer = max_buffer/4;
            max_buffer = max_buffer*4;
        }

        switch (ping_buffer_header->opcode)
        {
            case TRACE_RESPONSE_OPCODE_SETTRACELVL:                          /* set new tracelevel */
            {
                /* copy new tracelevels to trace_maxtracelevel */
                TRACE_MEMCPY(&trace_config.maxtracelevel, (PNIO_VOID*)((TRACE_UINT32)buff + TRACE_PBHEADER_SIZE), sizeof(trace_config.maxtracelevel));

                /* write trace config to FS */
                TRACE_WRITE_CFG(&trace_config, sizeof(trace_config) );

                ping_buffer_header->param = TRACE_RESPONSE_PARAM_TRACELVL_SET;

                ping_buffer_header->checksum = TRACE_RESPONSE_CHECKSUM_CLEAR;
                ping_buffer_header->checksum = TRACE_CKSUM(buff, TRACE_PBHEADER_SIZE);

                break; /* case TRACE_RESPONSE_OPCODE_SETTRACELVL */
            }


            case TRACE_RESPONSE_OPCODE_GETTRACE:                             /* get tracebuffer */
            {
                switch (ping_buffer_header->param)
                {
                    default:
                    {
                        trace_raw_addr = ping_buffer_header->union_raw_addr.struct_raw_addr.raw_addr_hi << 16 | ping_buffer_header->union_raw_addr.struct_raw_addr.raw_addr_lo;
                        trace_raw_len  = ping_buffer_header->union_raw_len.struct_raw_len.raw_len_hi    << 16 | ping_buffer_header->union_raw_len.struct_raw_len.raw_len_lo;

                        a = ping_buffer_header->seq*max_buffer;                 /* start */
                        b = a + (max_buffer-1);                                 /* end */
                        if ( a > trace_raw_len )
                        {
                            ping_buffer_header->param = TRACE_RESPONSE_PARAM_SEQ_OOR;        /* sequence out of range */
                        }
                        else
                        {
                            if ( b > trace_raw_len )        /* sequence not full */
                            {
                                b = trace_raw_len-1;          /* limit to maximum; 0-based */
                                ping_buffer_header->param = TRACE_RESPONSE_PARAM_SEQ_END;    /* buffer shortened */
                            }
                            else
                            {
                                ping_buffer_header->param = TRACE_RESPONSE_PARAM_SEQ_FULL;   /* used entire buffer */
                            }
                            tb_seq = (PNIO_VOID *)trace_raw_addr;
                            TRACE_MEMCPY((PNIO_VOID*)((TRACE_UINT32)buff+TRACE_PBHEADER_SIZE), (PNIO_VOID*)((TRACE_UINT32)tb_seq+a), (b-a+1));       /* copy data to ping buffer */
                        }
                        break;
                    }
                } /* ping_buffer_header->param */
                if ((b-a+1) > 65535)
                {
            	    ping_buffer_header->buff_len = 65536-4;                         /* set used buffer-length */
                }
                else
                {
            	    ping_buffer_header->buff_len = (TRACE_UINT16) (b-a+1); 			/* set used buffer-length */
                }
                ping_buffer_header->checksum = TRACE_RESPONSE_CHECKSUM_CLEAR;
                ping_buffer_header->checksum = TRACE_CKSUM(buff, TRACE_PBHEADER_SIZE);

                break; /* TRACE_RESPONSE_OPCODE_GETTRACE */
            }

        case TRACE_RESPONSE_OPCODE_ERRORMODE:
            switch (ping_buffer_header->param)
            {
            case TRACE_RESPONSE_PARAM_ERRMODE_FATAL:
                trace_config.fatalerror_mode = TRACE_ERRORMODE_FATAL;
            break;
            case TRACE_RESPONSE_PARAM_ERRMODE_RESETSOFT:
                trace_config.fatalerror_mode = TRACE_ERRORMODE_RESET_SOFT;
            break;
            case TRACE_RESPONSE_PARAM_ERRMODE_RESETHARD:
                trace_config.fatalerror_mode = TRACE_ERRORMODE_RESET_HARD;
            break;
            default:
                ping_buffer_header->param = TRACE_RESPONSE_PARAM_ERROR;                /* param unknown - error */
            break;
            } /* ping_buffer_header->param */

            if (ping_buffer_header->param != TRACE_RESPONSE_PARAM_ERROR)
            {
                ping_buffer_header->param = TRACE_RESPONSE_PARAM_ERRMODE_SET;
                /* write trace config to FS */
                TRACE_WRITE_CFG(&trace_config, sizeof(trace_config) );
            }

            ping_buffer_header->checksum = TRACE_RESPONSE_CHECKSUM_CLEAR;
            ping_buffer_header->checksum = TRACE_CKSUM(buff, TRACE_PBHEADER_SIZE);
        break; /* TRACE_RESPONSE_OPCODE_ERRORMODE */

        case TRACE_RESPONSE_OPCODE_STATUS:                          /* status */
            /*  Uptime */
            trace_config.uptime = TRACE_UPTIME;

            trace_config.cfg_checksum = 0;
            trace_config.cfg_checksum = TRACE_CKSUM(&trace_config, sizeof(trace_config));

            TRACE_MEMCPY((PNIO_VOID*)((TRACE_UINT32)buff+TRACE_PBHEADER_SIZE), (PNIO_VOID*)&trace_config, sizeof(trace_config));

            trace_raw_len = ping_buffer_header->union_raw_len.struct_raw_len.raw_len_hi << 16 | ping_buffer_header->union_raw_len.struct_raw_len.raw_len_lo;

            /* autoread table */
            a = TRACE_PBHEADER_SIZE+sizeof(trace_config);             /* start ar-t */
    #ifdef TRACE_GET_AUTOREAD_TABLE
            TRACE_GET_AUTOREAD_TABLE( trace_autoread_table_ptr);     /* get autoread-table */
            while ( trace_autoread_table_ptr->mem_len )              /* read while mem_len != NIL */
            {
                if ( buff_len >= ( a + ( 2* sizeof(TRACE_AUTOREAD_ENTRY)) ) )
                {
                    /* maximum response size not reached,
                     * it must stay place for the next entry AND end-entry (trace_autoread_table_end) */
                    tb_seq = trace_autoread_table_ptr;
                    TRACE_MEMCPY(buff+a, tb_seq, sizeof(TRACE_AUTOREAD_ENTRY));
                    trace_autoread_table_ptr++;                          /* next entry */
                    a = a + sizeof(TRACE_AUTOREAD_ENTRY);
                }
                else
                {
                    break;
                }
            }
    #endif
            TRACE_MEMSET( &trace_autoread_table_end, 0, sizeof(TRACE_AUTOREAD_ENTRY) );         /* end of autoread-table */
            TRACE_MEMCPY( (PNIO_VOID*)((TRACE_UINT32)buff+a), &trace_autoread_table_end, sizeof(TRACE_AUTOREAD_ENTRY) );    /* add to buffer */
            a = a + sizeof(TRACE_AUTOREAD_ENTRY);
            /* autoread table */

            ping_buffer_header->param = TRACE_RESPONSE_PARAM_STATUS_SET;


            ping_buffer_header->checksum = TRACE_RESPONSE_CHECKSUM_CLEAR;
            ping_buffer_header->checksum = TRACE_CKSUM(buff, TRACE_PBHEADER_SIZE);
        break; /* TRACE_RESPONSE_OPCODE_STATUS */


        case TRACE_RESPONSE_OPCODE_GOTODEFECT:                           /* goto defect */
            TRACE_GOTO_FATAL_ERROR(1);
        break; /* TRACE_RESPONSE_OPCODE_GOTODEFECT */


        case TRACE_RESPONSE_OPCODE_WRONG_CKSUM:                          /* wrong cksum */
            ping_buffer_header->opcode = trace_opcode_save;              /* restore opcode */
            /* do nothing - do not interfere with normal pings! */
        break; /* TRACE_RESPONSE_OPCODE_WRONG_CKSUM */


        default:                                                         /* opcode unknown - error */
            TRACE_OPCODE_HANDLER_EXTERN(ping_buffer_header->opcode, ping_buffer_header->param);
            ping_buffer_header->checksum = TRACE_RESPONSE_CHECKSUM_CLEAR;
            ping_buffer_header->checksum = TRACE_CKSUM(buff, TRACE_PBHEADER_SIZE);
        break; /* default */
        }
        return;
    }

    TRACE_UINT16 trace_get_fatalerror_mode()
    {
        TRACE_UINT16 retval=0;
        switch (trace_config.fatalerror_mode)
        {
        case TRACE_ERRORMODE_RESET_SOFT:
            retval = TRACE_ERRORMODE_RESET_SOFT;
        break;
        case TRACE_ERRORMODE_RESET_HARD:
            retval = TRACE_ERRORMODE_RESET_SOFT;
        break;
        default:
            retval = 0;
        break;
        }
        return retval;
    }

#else  /* PNIO_TRACE != PNIO_TRACE_DK_LSA */

    void trace_response(void* buff, unsigned int buf_length)
    {
    }

#endif



/*** end of file *************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
