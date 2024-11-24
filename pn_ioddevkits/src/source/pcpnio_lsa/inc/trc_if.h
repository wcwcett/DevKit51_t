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
/*  F i l e               &F: trc_if.h                                  :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*                       Frame for file "trc_if.h".                          */
/*                       ===========================                         */
/*                                                                           */
/*  Include-File:                                                            */
/*                                                                           */
/*  This file is prepared for subsystems will be abel to trace               */
/*  Includes of ltrc and of the different systems, compilers or              */
/*  operating system.                                                        */
/*                                                                           */
/*  This file has to be overwritten during system integration, because       */
/*  some includes depend on the different system, compiler or                */
/*  operating system.                                                        */
/*                                                                           */
/*****************************************************************************/

#ifndef TRC_IF_H                        /* ----- reinclude-protection ----- */
#define TRC_IF_H


#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif


    // ******* configure trace, depending on settings above *******
    #if (PNIO_TRACE == PNIO_TRACE_DK_MEM)
        #include "pniousrd.h"
        #include "trace_dk.h"
        #include "trace_mem.h"
        #define TRACE_INIT(_bufsize) {TrcDkInit(); TrcInit(_bufsize);}
        #define TRACE_PRINT(_fmt, _argptr)   TrcPrintMsgStore(_fmt, _argptr)

    #elif (PNIO_TRACE == PNIO_TRACE_DK_MEMXT)
        #include "pniousrd.h"
        #include "trace_dk.h"
        #include "trace_memxt.h"
        #define TRACE_INIT(_bufsize) {TrcDkInit(); TrcInit(_bufsize);}
        #define TRACE_PRINT(_fmt, _argptr)   TrcPrintMsgStore(_fmt, _argptr)

    #elif   (PNIO_TRACE == PNIO_TRACE_DK_CONSOLE)
        #include "trace_dk.h"
        #include "trace_con.h"
        #define TRACE_INIT(_bufsize) {TrcDkInit();}
        #define TRACE_PRINT(_fmt, _argptr)   /* no memory print */

    #elif (PNIO_TRACE == PNIO_TRACE_DK_LSA)
        #include "pniousrd.h"
        #include "trace_dk.h"
        #include "glob_sys.h"
        #include "pnio_types.h"
        #include "trace_cfg.h"
        #include "trace_lsa_com.h"
        #include "trace_lsa_dat.h"
        #define TRACE_INIT(_bufsize) {                                           \
            trace_init(TRACE_INIT_DEFAULT);                                      \
            trace_open(&tracebuffer, sizeof(tracebuffer));                       \
            TrcDkInit();                                                         \
        }
        #define TRACE_PRINT(_fmt, _argptr)   /* no memory print */

    #elif (PNIO_TRACE == PNIO_TRACE_DK_UDP)
        #include "trace_dk.h"
        #define TRACE_INIT(_bufsize)

    #elif (PNIO_TRACE == PNIO_TRACE_DK_TCP)
        #include "trace_dk.h"
        #define TRACE_INIT(_bufsize)
        #define TRACE_PRINT(_fmt, _argptr)

    #elif (PNIO_TRACE == PNIO_TRACE_NONE)
        #define TRACE_INIT(_bufsize)
        #define TRACE_PRINT(_fmt, _argptr)

        #define LSA_TRACE_00(subsys, level, msg)
        #define LSA_TRACE_01(subsys, level, msg, para1)
        #define LSA_TRACE_02(subsys, level, msg, para1, para2)
        #define LSA_TRACE_03(subsys, level, msg, para1, para2, para3)
        #define LSA_TRACE_04(subsys, level, msg, para1, para2, para3, para4)
        #define LSA_TRACE_05(subsys, level, msg, para1, para2, para3, para4, para5)
        #define LSA_TRACE_06(subsys, level, msg, para1, para2, para3, para4, para5, para6)
        #define LSA_TRACE_07(subsys, level, msg, para1, para2, para3, para4, para5, para6, para7)
        #define LSA_TRACE_08(subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8)
        #define LSA_TRACE_09(subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9)
        #define LSA_TRACE_10(subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10)
        #define LSA_TRACE_11(subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11)
        #define LSA_TRACE_12(subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12)
        #define LSA_TRACE_13(subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13)
        #define LSA_TRACE_14(subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14)
        #define LSA_TRACE_15(subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14, para15)
        #define LSA_TRACE_16(subsys, level, msg, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14, para15, para16)

        #define LSA_TRACE_BYTE_ARRAY(subsys_, level_, msg_, ptr_, len_)

        #define LTRC_LEVEL_TYPE    PNIO_UINT
    #else
        #error ("invalid trace define")
    #endif



/*****************************************************************************/
/*  end of file TRC_IF.H                                                     */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif  /* of LTRC_IF_H */


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
