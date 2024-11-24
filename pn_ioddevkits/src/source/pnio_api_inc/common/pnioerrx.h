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
/*  F i l e               &F: pnioerrx.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  error codes for IO-Base user interface                                   */
/*                                                                           */
/*****************************************************************************/
#ifndef _PNIO_ERRX_H              /* reinclude-protection */
#define _PNIO_ERRX_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

#define PNIO_APP_ASSERT(_cond,_text)                                                    \
{                                                                                       \
    if (!(_cond)) { LSA_TRACE_00(TRACE_SUBSYS_IOD_API, PNIO_LOG_ERROR_FATAL, _text); }  \
}

// *-----------------------------------------------
// * IOD API error Numbers
// *-----------------------------------------------
#define PNIO_NOT_OK             0
#define PNIO_OK                 1

// *-------- define error numbers for PNIO_get_last_error -----------
#define PNIO_ERR_RANGE_BEGIN    0x1000                          // begin of range
#define PNIO_ERR_RANGE_CM       PNIO_ERR_RANGE_BEGIN            // begin of range of error values of CM
#define PNIO_ERR_RANGE_IOD      PNIO_ERR_RANGE_BEGIN + 0x0100   // begin of range of error values of IOD
#define PNIO_ERR_RANGE_END      PNIO_ERR_RANGE_BEGIN + 0x0fff   // end of range

typedef enum
{
    PNIO_ERR_INIT_VALUE = 0,                    // initialization value
    // ****** Errors, created by CM *****
    PNIO_ERR_RESOURCE = PNIO_ERR_RANGE_CM,      // reserved range of numbers, starting at 0x1000
    PNIO_ERR_SYS_PATH,                          // error in system adaptation
    PNIO_ERR_SEQUENCE,                          // sequence error
    PNIO_ERR_PARAM,                             // parameter error

    PNIO_ERR_LOWER_LAYER,                       // an error occurred in the lower layer
    PNIO_ERR_ALLOC,                             // could not allocate upper memory
    PNIO_ERR_NOT_ACCESSIBLE,                    // could not lock buffer
    PNIO_ERR_ABORT,                             // operation was aborted
    PNIO_ERR_SESSION,                           // request belongs to an old session
    PNIO_ERR_TIMEOUT,                           // RPC failed due to a timeout
    PNIO_ERR_COMM,                              // RPC failed due to a comm-error
    PNIO_ERR_BUSY,                              // RPC peer signalled "busy" (should try again later)
    PNIO_ERR_LOWER_NARE,                        // an error occurred in the lower layer NARE */
    PNIO_ERR_LOWER_RPC,                         // an error occurred in the lower layer CLRPC */
    PNIO_ERR_RESERVED_CD,                       //
    PNIO_ERR_OWNED,                             // interface-submodule cannot be removed because it is owned by an AR */
    PNIO_ERR_FORMAT,                            // wrong format of alarm data */
    PNIO_ERR_PRM_INDEX,                         // unknown index (PrmWrite and PrmRead) */
    PNIO_ERR_PRM_PORTID,                        // port-id does not match with index (PrmWrite and PrmRead) */
    PNIO_ERR_PRM_DATA,                          // data-length too short (PrmRead) or data-length not consistent with block-structure (PrmWrite) */
    PNIO_ERR_PRM_BLOCK,                         // wrong block-type/version or something wrong with the block-data (PrmWrite) */
    PNIO_ERR_PRM_CONSISTENCY,                   // the parameterization is not consistent (PrmEnd) */
    PNIO_ERR_PRM_OTHER,                         // internally used */

    PNIO_ERR_NOT_YET,

    // ****** Errors, created by IOD *****
    PNIO_ERR_IOD_NO_MODULE = PNIO_ERR_RANGE_IOD,// pulling a module from an empty slot
    PNIO_ERR_IOD_NO_SUBMODULE,                  // pulling a submodule from an empty subslot
    PNIO_ERR_IOD_SLOT_OCCUPIED,                 // plugging a module into an occupied slot
    PNIO_ERR_IOD_SUBSLOT_OCCUPIED,              // plugging a submodule into an occupied subslot
    PNIO_ERR_IOD_INVALID_PARAM,                 // invalid parameter
    PNIO_ERR_IOD_NO_AR,                         // no application relation available
    PNIO_ERR_IOD_AR_RUNNING,                    // function only allowed if no AR is running
    PNIO_ERR_IOD_INVALID_API,                   // invalid API number
    PNIO_ERR_IOD_INVALID_SLOT,                  // invalid Slot number
    PNIO_ERR_IOD_INVALID_SUBSLOT,               // invalid Subslot number
    PNIO_ERR_IOD_NO_REC_RQ,                     // no record rd/wr request pending for asynchronous response
    PNIO_ERR_IOD_MAXNUM_API,                    // maximum numbers of APIs have been reached
    PNIO_ERR_IOD_MAXNUM_SLOTS,                  // maximum numbers of Slots exceeded
    PNIO_ERR_IOD_MAXNUM_SUBSLOTS,               // maximum numbers of Subslots exceeded
    PNIO_ERR_IOD_INVALID_DEV_HNDL,              // invalid device handle
    PNIO_ERR_IOD_INVALID_AR_HNDL,               // invalid AR handle
    PNIO_ERR_IOD_INVALID_API_HNDL,              // invalid API handle
    PNIO_ERR_IOD_INVALID_SLOT_HNDL,             // invalid slot handle
    PNIO_ERR_IOD_INVALID_SUBSLOT_HNDL,          // invalid subslot handle
    PNIO_ERR_IOD_INVALID_LOCKING,               // locking mechanism not allowed in IRT TOP
    PNIO_ERR_IOD_INVALID_BUFLEN,                // invalid buffer length
    PNIO_ERR_IOD_INVALID_DEVNAME,               // invalid device name (station name)
    PNIO_ERR_IOD_INVALID_DEVTYPE,               // invalid device type (station type)
    PNIO_ERR_IOD_NO_RESSOURCES,                 // a ressource limit has been reached
    PNIO_ERR_IOD_ALARM_SEQUENCE,                // alarm sequence error (new alarm before previous confirmation)
    PNPB_ERR_TIMER_BAD_RETURN_VAL,              // bad retval from timer handling

    PNIO_ERR_SYS,                               // system error

    /* parameter errors */
    PNIO_ERR_PRM_CALLBACK                       // * parameter cbf is illegal

} PNIO_ERR_ENUM;

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif  /* end of file _PNIO_ERRX_H */


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
