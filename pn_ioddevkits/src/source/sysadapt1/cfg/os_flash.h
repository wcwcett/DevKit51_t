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
/*  F i l e               &F: os_flash.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  flash read/write abstraction layere                                      */
/*                                                                           */
/*****************************************************************************/
#ifndef _OS_FLASH_H
#define _OS_FLASH_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif


    #define FLASH_ERRNUM_OK        0
    #define FLASH_ERRNUM_NOT_OK    1


    PNIO_VOID OsFlashPrintf(PNIO_CHAR* fmt, ...);


    PNIO_UINT32 OsFlashInit   (PNIO_UINT32 FlashSize,
                               PNIO_UINT32 ImageSize);


    PNIO_UINT32 OsFlashErase  (PNIO_UINT32  FlashPageNum,
                               PNIO_UINT32  DataSize,
                               PNIO_UINT32* pError);

    PNIO_UINT32 OsFlashRead   (PNIO_UINT32  FlashPageNum,
                               PNIO_UINT8*  pMemDst,
                               PNIO_UINT32  DataSize,
                               PNIO_UINT32* pError);

    PNIO_UINT32 OsFlashProgram(PNIO_VOID (*callback)( PNIO_VOID*, PNIO_UINT32 ),
                               PNIO_VOID*   arg,
                               PNIO_UINT8*  pMemSrc8,
                               PNIO_UINT32  DataSize,
                               PNIO_UINT32* pError);

    PNIO_UINT32 flash_verify  (PNIO_CHAR* pFlash,
                               PNIO_CHAR* pBuf,
                               PNIO_INT   BufSize);
    PNIO_UINT32 flash_erase_verify(PNIO_CHAR* pFlash,
                                   PNIO_INT   BufSize );



#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
