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
/*  F i l e               &F: xx_mem.h                                  :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  header file for xx_mem.c                                                 */
/*                                                                           */
/*****************************************************************************/

#ifdef __cplusplus
    extern "C" {
#endif

#ifndef XX_MEM_INC_H
#define XX_MEM_INC_H

typedef unsigned long   XX_MEM_DWORD;
typedef unsigned char   XX_MEM_BYTE;
typedef char            XX_MEM_STRING;

#define XX_MEM_NULL_PTR     (PNIO_VOID *)0


#define XX_MEM_RET_OK                         (XX_MEM_DWORD)0x01
#define XX_MEM_RET_INVALID_POINTER_ALIGN      (XX_MEM_DWORD)0x02
#define XX_MEM_RET_ERR_NULL_PTR               (XX_MEM_DWORD)0x03
#define XX_MEM_RET_INVALID_FIRST_BUF          (XX_MEM_DWORD)0x04
#define XX_MEM_RET_NO_FREE_MEMORY             (XX_MEM_DWORD)0x05
#define XX_MEM_RET_SIZE_TOO_SMALL             (XX_MEM_DWORD)0x06   // > 100
#define XX_MEM_RET_DESC_CORRUPT               (XX_MEM_DWORD)0x07
#define XX_MEM_RET_BUF_CORRUPT                (XX_MEM_DWORD)0x08
#define XX_MEM_RET_MEM_READ_ONLY              (XX_MEM_DWORD)0x09
#define XX_MEM_RET_MEM_LEN_NULL               (XX_MEM_DWORD)0x0a
#define XX_MEM_RET_REENTER                    (XX_MEM_DWORD)0x0b
#define XX_MEM_RET_MEM_CORRUPT_PREV_OVERWRITE (XX_MEM_DWORD)0x0c
#define XX_MEM_RET_MEM_CORRUPT                (XX_MEM_DWORD)0x0d
#define XX_MEM_RET_MEM_CORRUPT_PREV_POINTER   (XX_MEM_DWORD)0x0e
#define XX_MEM_RET_MEM_CORRUPT_NEXT_POINTER   (XX_MEM_DWORD)0x0f
#define XX_MEM_RET_BUF_ALREADY_FREED          (XX_MEM_DWORD)0x10
#define XX_MEM_RET_INVALID_FIRST_BUF_CHECKSUM (XX_MEM_DWORD)0x11
//#define XX_MEM_RET_INVALID_ALLOC_INDEX_0      (XX_MEM_DWORD)0x12




#define XX_MEM_CODE_ATTR





#define XX_MEM_DO_PRESET       1UL
#define XX_MEM_DO_NOT_PRESET   0UL

XX_MEM_DWORD XX_MEM_CODE_ATTR XX_MEM_init(  XX_MEM_DWORD         Size,
                                            XX_MEM_BYTE          mem_ini_value,
                                            XX_MEM_BYTE          *pBaseAdr,
                                            XX_MEM_DWORD         do_preset_buffer,
                                            XX_MEM_BYTE          preset_value  );


XX_MEM_DWORD  XX_MEM_CODE_ATTR  XX_MEM_alloc( PNIO_VOID               **p,
                                              XX_MEM_DWORD       len,
                                              XX_MEM_BYTE        *pBaseAdr );




XX_MEM_DWORD XX_MEM_CODE_ATTR  XX_MEM_free( PNIO_VOID       *p );
LSA_UINT32 xx_mem_validateAll              (PNIO_VOID* pMemP);


typedef struct XX_MEM_BUF_S
{
 PNIO_VOID             *p; // IN

 XX_MEM_DWORD           Size;
 XX_MEM_DWORD           AllocIndex;

} XX_MEM_BUF_T;

XX_MEM_DWORD XX_MEM_CODE_ATTR  XX_MEM_check( XX_MEM_BUF_T   *pBuf);


const XX_MEM_STRING * XX_MEM_CODE_ATTR XX_MEM_get_error_info(  XX_MEM_DWORD ret );


typedef struct XX_MEM_INFO_S
{
 XX_MEM_BYTE            *pBaseAdr; // IN

 XX_MEM_DWORD           free_mem;
 XX_MEM_DWORD           used_mem;
 XX_MEM_DWORD           count_allocated_buffer;
 XX_MEM_DWORD           max_free_buffer_size;

 PNIO_VOID              *pFirstBuf;
 XX_MEM_DWORD           SizeFirstBuf;
 XX_MEM_DWORD           IndexFirstBuf;

} XX_MEM_INFO_T;


XX_MEM_DWORD XX_MEM_CODE_ATTR  XX_MEM_info( XX_MEM_INFO_T *info);
XX_MEM_DWORD XX_MEM_CODE_ATTR  XX_MEM_infoPrint( XX_MEM_INFO_T  *info);


#endif  /* of XX_MEM_INC_H */


#ifdef __cplusplus
}
#endif


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
