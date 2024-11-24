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
/*  F i l e               &F: xx_mem.c                                  :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  alloc/free for ERTEC used memory                                         */
/*                                                                           */
/*****************************************************************************/


#include "compiler.h"
#include "os.h"
#include "xx_mem.h"

// * ----------------------------------------
// * defines
// * ----------------------------------------

// *** include special debug features (normally undefined) ***
//#define XX_MEM_STATISTIC           1
//#define XX_MEM_VALIDATE     	     0
//#define XX_MEM_CHECK_HEADER_EXT    1

// *** memory alignment
#define XX_MEM_MIN_ALIGN        0x0f
#define XX_MEM_MIN_ALIGN_MASK   0x0fffffff0UL

#define XX_MEM_FREE_PATTERN     0xAE
#define XX_MEM_HEADER_PATTERN1  0x7755abcd  // allocated buffer
#define XX_MEM_HEADER_PATTERN2  0x6644abcd  // free buffer

// *-----------------------------------------
// * note: sizeof (XX_MEM_BUF_HEADER_T) must
// *       fit to the specified alignment.
// *       Otherwise include fillbytes.
// *       Sizeof header here is 8*4 = 32 byte
// *-----------------------------------------
typedef struct XX_MEM_BUF_HEADER_S   // 0x20
{
    XX_MEM_DWORD                 pattern;   // fix
    XX_MEM_DWORD                 alloc_index;
    XX_MEM_DWORD                 checksum;   // nur fix
    XX_MEM_DWORD                 buf_len;   // fix

    XX_MEM_DWORD                 free_len_to_next_buf;
    XX_MEM_DWORD                 offs_prev_buf;
    XX_MEM_DWORD                 offs_next_buf;
    XX_MEM_DWORD                 offs_own;
} XX_MEM_BUF_HEADER_T;



// *-----------------------------------------
// * note: sizeof (XX_MEM_DESCRIPTOR_S) must
// *       fit to the specified alignment.
// *       Otherwise include fillbytes.
// *       Sizeof header here is 12*4 = 48 byte
// *-----------------------------------------
typedef struct XX_MEM_DESCRIPTOR_S    //0x30
{
    XX_MEM_DWORD         dummy_start;             // visual debugging tag to find descriptor start
    XX_MEM_DWORD         count_allocated_buffer;  // number of allocated buffers
    XX_MEM_DWORD         used_mem;                // totally used memory

    XX_MEM_DWORD         checksum;

    XX_MEM_DWORD         do_preset_buffer;  // fix
    XX_MEM_DWORD         dummy_align1;
    XX_MEM_DWORD         preset_value;  // fix
    XX_MEM_DWORD         size_aligned_without_desc;    // fix

    XX_MEM_DWORD         offs_first_buf; // fix
    XX_MEM_DWORD         offs_act_buf;
    XX_MEM_DWORD         alloc_index;
    XX_MEM_DWORD         dummy_1_a16;
} XX_MEM_DESCRIPTOR_T;


#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
    #define  ENTER_XX_MEM       {  OsTakeSemB(SemBId);}
    #define  EXIT_XX_MEM        {  OsGiveSemB(SemBId);}
    #define  INIT_ENTER_EXIT    {  OsAllocSemB(&SemBId);}
#else
    #define  ENTER_XX_MEM       {  OsEnterX(OS_MUTEX_XX_MEM);}
    #define  EXIT_XX_MEM        {  OsExitX(OS_MUTEX_XX_MEM);}
    #define  INIT_ENTER_EXIT
#endif



#define M_NULL_H  0UL

#define XX_MEM_DESC_NULL_PTR  (XX_MEM_DESCRIPTOR_T *)0


#ifndef STATIC
 #define STATIC   static
#endif



STATIC XX_MEM_DWORD xx_mem_check_header(  XX_MEM_BUF_HEADER_T  *  h );

STATIC XX_MEM_DWORD xx_mem_check_desc( XX_MEM_DESCRIPTOR_T  *  pDesc );


STATIC XX_MEM_DWORD xx_mem_get_checksum_header(  XX_MEM_BUF_HEADER_T  *  h );
STATIC XX_MEM_DWORD xx_mem_check_header_during_alloc(  XX_MEM_BUF_HEADER_T     *  h );

STATIC XX_MEM_DWORD xx_mem_get_checksum_desc(  XX_MEM_DESCRIPTOR_T  *  pDesc);

#define XX_MEM_TEST_PATTERN  0x12345678UL

#define XX_MEM_DUMMY_START   0x44556677UL

#define XX_MEM_CHECKSUM_FREE   0xF000000FUL

#ifdef XX_MEM_VALIDATE
    STATIC XX_MEM_DWORD xx_mem_validate( XX_MEM_DESCRIPTOR_T   *  pDesc );
#endif




typedef struct XX_MEM_ERR_INFO_S
{
    XX_MEM_STRING     info[0x40];
} XX_MEM_ERR_INFO_T;


static const XX_MEM_ERR_INFO_T    MD_xx_err[] = { {"Not Defined"},
                                                  {"OK"},
                                                  {"INVALID_POINTER_ALIGN"},
                                                  {"ERR_NULL_PTR"},
                                                  {"INVALID_FIRST_BUF"},
                                                  {"NO_FREE_MEMORY"},
                                                  {"SIZE_TOO_SMALL"},
                                                  {"DESC_CORRUPT"},
                                                  {"BUF_CORRUPT"},
                                                  {"MEM_READ_ONLY"},
                                                  {"MEM_LEN_NULL"},
                                                  {"REENTER"},
                                                  {"MEM_CORRUPT_PREV_OVERWRITE"},
                                                  {"MEM_CORRUPT"},
                                                  {"MEM_CORRUPT_PREV_POINTER"},
                                                  {"MEM_CORRUPT_NEXT_POINTER"},
                                                  {"BUF_ALREADY_FREED"},
                                                  {"INVALID_FIRST_BUF_CHECKSUM"},
                                                  {"XX_MEM_RET_INVALID_ALLOC_INDEX_0"}
                                                        };

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
static PNIO_UINT32 SemBId;
#endif

// *-----------------------------------------------------------*
// *  XX_MEM_get_error_info(  XX_MEM_DWORD        ret )
// *
// *
// *
// *-----------------------------------------------------------*
const XX_MEM_STRING* XX_MEM_CODE_ATTR  XX_MEM_get_error_info(XX_MEM_DWORD ret)
{
    XX_MEM_DWORD    last_index = (sizeof(MD_xx_err) / sizeof(XX_MEM_ERR_INFO_T)) - 1;

    if ( ret >= last_index  )
    {
    ret = last_index;
    }

    return MD_xx_err[ret].info;
}
/*---------------------- end [subroutine] ---------------------------------*/



// ***************************************************************************/
// * F u n c t i o n:         XX_MEM_init()
// *
// * D e s c r i p t i o n :  init function, must be called once for every
// *                          memory pool. A binary semaphore is allocated, to
// *                          synchronize multi-threaded  alloc/free access.
// *
// *
// * A r g u m e n t s:       Size                  memory pool size
// *                          mem_ini_value         init value (init once) for complet pool
// *                          *pBaseAdr             memory pool start address
// *                          do_preset_buffer      1: buffer is presetted at every alloc, 0: else
// *                          preset_value          preset value (for do_reset_buffer = 1only)
// *
// *
// * Return Value:            XX_MEM_RET_OK         init successful
// *                          others                error number, if init failed
// *
// ***************************************************************************/
XX_MEM_DWORD XX_MEM_CODE_ATTR XX_MEM_init(  XX_MEM_DWORD         Size,
                                            XX_MEM_BYTE          mem_ini_value,
                                            XX_MEM_BYTE          *pBaseAdr,
                                            XX_MEM_DWORD         do_preset_buffer,
                                            XX_MEM_BYTE          preset_value  )
{
    XX_MEM_DWORD                      align, offset_first_buf;
    volatile XX_MEM_DESCRIPTOR_T*     pDesc;
    XX_MEM_BUF_HEADER_T*              pFirstBuf;
    XX_MEM_DWORD                      ret = XX_MEM_RET_OK;

    if (Size < 0x100)
    {
        return (XX_MEM_RET_SIZE_TOO_SMALL);
    }

    if (pBaseAdr == (XX_MEM_BYTE *)0)
    {
        return (XX_MEM_RET_ERR_NULL_PTR);
    }

    INIT_ENTER_EXIT;
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
    EXIT_XX_MEM;   /* do not call this on ECOS !! means on STEC ??? */
#endif
    ENTER_XX_MEM;

    pDesc = (XX_MEM_DESCRIPTOR_T *)pBaseAdr;


    // * --------------------------------------------
    // *   test with testpattern, if memory is writable
    // * --------------------------------------------
    pDesc->dummy_start = XX_MEM_TEST_PATTERN;
    if (XX_MEM_TEST_PATTERN != pDesc->dummy_start)
    {
        EXIT_XX_MEM;
        return (XX_MEM_RET_MEM_READ_ONLY);
    }


    // * --------------------------------------------
    // *   preset the complete memory
    // * --------------------------------------------
    // OsMemSet(pBaseAdr, 0x55, (PNIO_UINT32)Size);


    // * --------------------------------------------
    // *   align first buffer to 16 byte and locate behind the descriptor
    // * --------------------------------------------
    align = (XX_MEM_DWORD)pBaseAdr & XX_MEM_MIN_ALIGN;
    if (align != 0)
    {
        align = (XX_MEM_MIN_ALIGN+1) - align;
    }

    offset_first_buf = align + sizeof(XX_MEM_DESCRIPTOR_T);    // first buffer set behind descriptor
    pFirstBuf = (XX_MEM_BUF_HEADER_T*)(pBaseAdr + offset_first_buf);

    pDesc->size_aligned_without_desc = Size - ( align + sizeof(XX_MEM_DESCRIPTOR_T) );

    // * --------------------------------------------
    // *   fill the memory descriptor, which is located
    // *   at the beginning of the memory
    // * --------------------------------------------
    pDesc->dummy_start            = XX_MEM_DUMMY_START;    // set tag for visual debugging
#ifdef XX_MEM_STATISTIC
    pDesc->count_allocated_buffer = 0UL;
    pDesc->used_mem               = 0UL;
#endif
    pDesc->do_preset_buffer       = do_preset_buffer;
    pDesc->preset_value           = preset_value;
    pDesc->offs_first_buf         = offset_first_buf;
    pDesc->offs_act_buf           = offset_first_buf;
    pDesc->dummy_align1           = 0;

    // * --------------------------------------------
    // *   count descriptor checksum
    // * --------------------------------------------
    pDesc->checksum = xx_mem_get_checksum_desc((XX_MEM_DESCRIPTOR_T *)pDesc);


    pDesc->dummy_1_a16       = XX_MEM_DUMMY_START;
    pDesc->alloc_index       = 0;
    // * --------------------------------------------
    // *   initialize buffer memory (except descriptor)
    // * --------------------------------------------
    //OsMemSet(pFirstBuf, (PNIO_INT)mem_ini_value, (PNIO_UINT32)pDesc->size_aligned_without_desc);

    // * --------------------------------------------
    // *   initialize first buffer
    // * --------------------------------------------
    pFirstBuf->buf_len              = 0;
    pFirstBuf->free_len_to_next_buf = pDesc->size_aligned_without_desc - sizeof(XX_MEM_BUF_HEADER_T);
    pFirstBuf->checksum             = XX_MEM_CHECKSUM_FREE;
    pFirstBuf->offs_prev_buf        = M_NULL_H;
    pFirstBuf->offs_next_buf        = M_NULL_H;
    pFirstBuf->alloc_index          = 0;
    pFirstBuf->offs_own             = offset_first_buf;

#ifdef XX_MEM_VALIDATE
    ret = xx_mem_validate((XX_MEM_DESCRIPTOR_T*)pDesc);
#endif
    EXIT_XX_MEM;
    return (ret);
}
/*---------------------- end [subroutine] ---------------------------------*/


// act immer wo zuletzt deallokiert wurde
// Verkettung: letzter Puffer  zeigt auf nichts

// ***************************************************************************/
// * F u n c t i o n:         XX_MEM_alloc ()
// *
// * D e s c r i p t i o n :  allocates a new memory block
// *
// *
// * A r g u m e n t s:       **p      [in]  ppointer to memory block
// *                                   [out] pointer to memory block
// *                          len      size of the block to allocate
// *                          pBaseAdr base address of the memory pool (pool descriptor)
// *
// * Return Value:            XX_MEM_OK or error number
// *
// *
// ***************************************************************************/
XX_MEM_DWORD  XX_MEM_CODE_ATTR  XX_MEM_alloc( PNIO_VOID                       **p,
                                              XX_MEM_DWORD               len,
                                              XX_MEM_BYTE                *pBaseAdr )
{
    XX_MEM_DWORD           ret, buf_size, buf_size_incl_header;
    XX_MEM_BUF_HEADER_T   *h, *act;
    XX_MEM_DESCRIPTOR_T   *  pDesc = (XX_MEM_DESCRIPTOR_T  *)pBaseAdr;
    XX_MEM_BYTE                  *p_own;

    XX_MEM_DWORD  LoopResetCnt = 1;

    *p = XX_MEM_NULL_PTR;

    ENTER_XX_MEM;

    ret = xx_mem_check_desc(pDesc);
    if ( ret != XX_MEM_RET_OK )
    {
        EXIT_XX_MEM;
        return(ret);
    }
    if ( len == 0 )
    {
        EXIT_XX_MEM;
        return(XX_MEM_RET_MEM_LEN_NULL);
    }

    #ifdef XX_MEM_VALIDATE
    ret = xx_mem_validate(pDesc);
    if ( ret != XX_MEM_RET_OK )
    {
        EXIT_XX_MEM;
        return(ret);
    }
    #endif // XX_MEM_VALIDATE

    buf_size             = (len + XX_MEM_MIN_ALIGN) & XX_MEM_MIN_ALIGN_MASK;

    // *** build blockpointer from pooldescriptor and block-offset in pool ***
    h = (XX_MEM_BUF_HEADER_T  *)((XX_MEM_BYTE *)pDesc + pDesc->offs_act_buf);

    if ( (h->buf_len == 0) && (h->free_len_to_next_buf >= buf_size) )
    {
        // *-------------------------------------------------------------------------------------------
        // * special case: pool empty, first buffer is allocated
        // * if first buffer has buflen = 0 it is free (pool is empty). But the first buffer must
        // * never be deleted !!
        // *-------------------------------------------------------------------------------------------
        if ( h->checksum != XX_MEM_CHECKSUM_FREE )
        {
            EXIT_XX_MEM;
            return(XX_MEM_RET_INVALID_FIRST_BUF_CHECKSUM);
        }

        // *** plausibility check: first pointer has no backward link ****
        if ( h->offs_prev_buf != M_NULL_H )
        {
            EXIT_XX_MEM;
            return(XX_MEM_RET_INVALID_FIRST_BUF);
        }

        // **** set new values for buf-size and free-len to next buffer ***
        h->buf_len              = buf_size;
        h->free_len_to_next_buf = h->free_len_to_next_buf - h->buf_len;

        // *** increment and store alloc-index ***
        pDesc->alloc_index++;
        h->alloc_index = pDesc->alloc_index;

        // *** set blockheader-properties  offset and checksum ***
        p_own        = (XX_MEM_BYTE *)(h + 1);               // find start address of user memory in this block
        h->offs_own = (XX_MEM_DWORD)h - (XX_MEM_DWORD)pDesc;// find offset of this header in mempool
        h->checksum  = xx_mem_get_checksum_header(h);

        h->pattern = XX_MEM_HEADER_PATTERN1;
        // *** check header during alloc ***
        ret = xx_mem_check_header_during_alloc(h);
        if ( ret != XX_MEM_RET_OK )
        {
            EXIT_XX_MEM;
            return(ret);
        }

        // *** return the user memory pointer ***
        *p = p_own;

        //h->id = XX_MEM_ID;

        // **** fill memory with preset value, if required ***
        if (pDesc->do_preset_buffer == XX_MEM_DO_PRESET )
        {
            OsMemSet(*p, (PNIO_INT)pDesc->preset_value, (PNIO_UINT32)h->buf_len);
        }

        // *** set some statistic values
        #ifdef XX_MEM_STATISTIC
        pDesc->count_allocated_buffer++;
        pDesc->used_mem += buf_size;
        #endif // XX_MEM_STATISTIC

        //PNIO_printf ("xxMemAlloc-?? (p=0x%x, len=%d)\n", *p, len);
        EXIT_XX_MEM;
        return(XX_MEM_RET_OK);
    }


    // *-------------------------------------------------------------------
    // * all other cases: pool is not empty, one ore more blocks are
    // * allocated
    // *-------------------------------------------------------------------
    buf_size_incl_header = buf_size + sizeof(XX_MEM_BUF_HEADER_T); // find buffer size incl. header

    if ( h->free_len_to_next_buf < buf_size_incl_header )          // check if gap is large enough-->else
    {
        //  start search from act_buf
        //  h = pDesc->first_buf;
        h = (XX_MEM_BUF_HEADER_T  *)(pBaseAdr + pDesc->offs_act_buf);

        for (;;)
        {
            ret = xx_mem_check_header_during_alloc(h);
            if ( ret != XX_MEM_RET_OK )
            {
                EXIT_XX_MEM;
                return(ret);
            }

            if ( h->free_len_to_next_buf >= buf_size_incl_header)
            {
                // ok, valid buffer found
                break;
            }


            if ( h->offs_next_buf == M_NULL_H ) // end of pool reached ?
            {
                if (LoopResetCnt)
                { // research from start of pool, but only once
                    LoopResetCnt--;
                    pDesc->offs_act_buf = pDesc->offs_first_buf;
                    h = (XX_MEM_BUF_HEADER_T  *)(pBaseAdr + pDesc->offs_act_buf);
                }
                else
                { // researching the whole pool was not successful --> error: no free memory
                    EXIT_XX_MEM;
                    return(XX_MEM_RET_NO_FREE_MEMORY);
                }
            }

            h = (XX_MEM_BUF_HEADER_T  *)(pBaseAdr + h->offs_next_buf); // goto next pool entry

        }  // for

    }

    // * --------------------------------
    // * new buffer found --> initialize
    // * --------------------------------

    // **** get start address of the current (free) block ***
    act = (XX_MEM_BUF_HEADER_T *)(  (XX_MEM_BYTE *)(h + 1) + h->buf_len);

    // *** set buffer size and free-len to next buffer ***
    act->buf_len              = buf_size;
    act->free_len_to_next_buf = h->free_len_to_next_buf - buf_size_incl_header;

    // *** set forward and backward linkpointer ***
    act->offs_prev_buf = h->offs_own;
    act->offs_next_buf = h->offs_next_buf;
    act->offs_own      = (XX_MEM_DWORD)act - (XX_MEM_DWORD)pBaseAdr;
    act->pattern       = XX_MEM_HEADER_PATTERN1;

    // refresh forward link and free-len to next  in the previous block
    h->free_len_to_next_buf = 0;
    h->offs_next_buf        = act->offs_own;

    // set forward linkpointer in current block
    if ( act->offs_next_buf != M_NULL_H )
    {
        XX_MEM_BUF_HEADER_T   *  hTmp = (XX_MEM_BUF_HEADER_T   *)(pBaseAdr + act->offs_next_buf);
        hTmp->offs_prev_buf = act->offs_own;
    }


    // refresh current block offset and alloc index in pool descriptor ***
    pDesc->offs_act_buf = act->offs_own;
    pDesc->alloc_index++;
    act->alloc_index = pDesc->alloc_index;

    // *** get user memory pointer of current block ***
    p_own          = (XX_MEM_BYTE *)(act + 1);
    *p = p_own;

    // *** set pool-offset value and checksum in current block ***
    act->offs_own = (XX_MEM_DWORD)act - (XX_MEM_DWORD)pDesc;
    act->checksum  = xx_mem_get_checksum_header(act);


    // **** fill memory with preset value, if required ***
    if (pDesc->do_preset_buffer == XX_MEM_DO_PRESET )
    {
        OsMemSet(*p, (PNIO_INT)pDesc->preset_value, (PNIO_UINT32)act->buf_len);
    }

    // *** set some statistic values
    #ifdef XX_MEM_STATISTIC
    pDesc->count_allocated_buffer++;
    pDesc->used_mem += buf_size;
    #endif // XX_MEM_STATISTIC

    #ifdef  XX_MEM_CHECK_HEADER_EXT
        ret = xx_mem_check_header(h);
        if ( ret != XX_MEM_RET_OK )
        {
            EXIT_XX_MEM;
            return(ret);
        }
    #endif

    // PNIO_printf ("xxMemAlloc (p=0x%x, len=%d)\n", *p, len);
    EXIT_XX_MEM;

    return(XX_MEM_RET_OK);
}
/*---------------------- end [subroutine] ---------------------------------*/




// ***************************************************************************/
// * F u n c t i o n:         XX_MEM_free
// *
// * D e s c r i p t i o n :  frees an prior allocated memory block
// *
// *
// * A r g u m e n t s:       *p    pointer to memory block
// *
// * Return Value:            XX_MEM_OK  or error number
// *
// *
// ***************************************************************************/
XX_MEM_DWORD XX_MEM_CODE_ATTR XX_MEM_free(PNIO_VOID *p)
{
    XX_MEM_BUF_HEADER_T   *h, *next_buf, *prev_buf;
    XX_MEM_DWORD          ret;
    XX_MEM_DESCRIPTOR_T   *pDesc;

    // PNIO_printf ("xxMemFree (p=0x%x\n", p);

    if ( p == XX_MEM_NULL_PTR )
    {
        return(XX_MEM_RET_ERR_NULL_PTR);
    }

    // ** set start pointer of this block descriptor **
    h = (XX_MEM_BUF_HEADER_T *)p - 1;


    ENTER_XX_MEM;

    ret = xx_mem_check_header(h);
    if ( ret != XX_MEM_RET_OK )
    {
        EXIT_XX_MEM;
        return(ret);
    }


    // **** find start address of this pool ***
    pDesc = (XX_MEM_DESCRIPTOR_T   *)((XX_MEM_DWORD)h - h->offs_own);

    #ifdef XX_MEM_VALIDATE
        ret = xx_mem_validate(pDesc);
        if ( ret != XX_MEM_RET_OK )
        {
            EXIT_XX_MEM;
            return(ret);
        }
    #endif

    h->checksum    = XX_MEM_CHECKSUM_FREE;


    #ifdef XX_MEM_STATISTIC
        pDesc->count_allocated_buffer--;
        pDesc->used_mem -= h->buf_len;
    #endif


    OsMemSet(p, XX_MEM_FREE_PATTERN, h->buf_len );


    if ( h->offs_prev_buf == M_NULL_H )
    {
        // link first buffer -> header is still initialized
        h->free_len_to_next_buf = h->free_len_to_next_buf + h->buf_len;
        h->buf_len              = 0;

        pDesc->offs_act_buf       = h->offs_own;
        #ifdef XX_MEM_CHECK_HEADER_EXT
            ret = xx_mem_check_header(h);
            if (( ret != XX_MEM_RET_OK )&& (ret != XX_MEM_RET_BUF_ALREADY_FREED))
            {
                EXIT_XX_MEM;
                return(ret);
            }
        #endif
        EXIT_XX_MEM;
        return(XX_MEM_RET_OK);
    }

    // **** get previous buffer ***
    prev_buf = (XX_MEM_BUF_HEADER_T   *)((XX_MEM_BYTE *)pDesc + h->offs_prev_buf);

    // backward link pointer to forerunner
    pDesc->offs_act_buf  =  prev_buf->offs_own;

    // remove buffer from linked list -> delete header
    prev_buf->offs_next_buf     = h->offs_next_buf;
    prev_buf->free_len_to_next_buf +=
    h->buf_len + sizeof(XX_MEM_BUF_HEADER_T) + h->free_len_to_next_buf;


    if ( h->offs_next_buf == M_NULL_H )
    {
        #ifdef XX_MEM_CHECK_HEADER_EXT
            ret = xx_mem_check_header(h);
            if (( ret != XX_MEM_RET_OK )&& (ret != XX_MEM_RET_BUF_ALREADY_FREED))
            {
               EXIT_XX_MEM;
                return(ret);
            }
        #endif
        // this was the last buffer
        EXIT_XX_MEM;
        return(XX_MEM_RET_OK);
    }
    next_buf = (XX_MEM_BUF_HEADER_T   *)((XX_MEM_BYTE *)pDesc + h->offs_next_buf);

    // buffer was inside the chain (not first or last one)
    next_buf->offs_prev_buf = prev_buf->offs_own;

    #ifdef XX_MEM_CHECK_HEADER_EXT
        ret = xx_mem_check_header(h);
        if (( ret != XX_MEM_RET_OK )&& (ret != XX_MEM_RET_BUF_ALREADY_FREED))
        {
            EXIT_XX_MEM;
            return(ret);
        }
    #endif

    h->pattern = XX_MEM_HEADER_PATTERN2;
    EXIT_XX_MEM;
    return(XX_MEM_RET_OK);
}
/*---------------------- end [subroutine] ---------------------------------*/


// ***************************************************************************/
// * F u n c t i o n:         XX_MEM_info ()
// *
// * D e s c r i p t i o n :  reads out some pool information
// *
// *
// * A r g u m e n t s:       XX_MEM_INFO_T  *info     pointer to the info block
// *
// * Return Value:            XX_MEM_RET_OK or error number
// *
// *
// ***************************************************************************/
XX_MEM_DWORD XX_MEM_CODE_ATTR  XX_MEM_info( XX_MEM_INFO_T  *info)
{
    XX_MEM_DESCRIPTOR_T   *pDesc = (XX_MEM_DESCRIPTOR_T  *)info->pBaseAdr;
    XX_MEM_DWORD                  ret;
    XX_MEM_BUF_HEADER_T   *h;

    ENTER_XX_MEM;

    info->max_free_buffer_size   = 0;
    #ifdef XX_MEM_STATISTIC
        info->count_allocated_buffer = 0;
        info->used_mem               = 0;
        info->free_mem               = 0;
    #endif

    info->pFirstBuf              = (PNIO_VOID *)0;

    ret = xx_mem_check_desc(pDesc);
    if ( ret != XX_MEM_RET_OK )
    {
        EXIT_XX_MEM;
        return(ret);
    }

    h = (XX_MEM_BUF_HEADER_T *)(info->pBaseAdr + pDesc->offs_first_buf);

    for ( ;;)
    {


        if ( (h->buf_len > 0) && (h->alloc_index >= info->IndexFirstBuf) )
        {
            #ifdef XX_MEM_STATISTIC
                info->count_allocated_buffer++;    // the first is always there, but can be empty
            #endif
            if ( info->pFirstBuf == (PNIO_VOID *)0  )
            {
                info->pFirstBuf     = (PNIO_VOID *)(h + 1);
                info->IndexFirstBuf = h->alloc_index;
                info->SizeFirstBuf  = h->buf_len;
            }
        }

        #ifdef XX_MEM_STATISTIC
        info->free_mem += h->free_len_to_next_buf;
        info->used_mem += h->buf_len;
        #endif // XX_MEM_STATISTIC

        if ( h->free_len_to_next_buf > info->max_free_buffer_size )
        {
            info->max_free_buffer_size = h->free_len_to_next_buf;
        }



        if ( h->offs_next_buf == M_NULL_H )
        {
            break;
        }

        h = (XX_MEM_BUF_HEADER_T *)(info->pBaseAdr + h->offs_next_buf);

        ret = xx_mem_check_header(h);
        if ( ret != XX_MEM_RET_OK )
        {
            EXIT_XX_MEM;
            return(ret);
        }

    }  // for


    EXIT_XX_MEM;
    return(XX_MEM_RET_OK);
}
/*---------------------- end [subroutine] ---------------------------------*/


// ***************************************************************************/
// * F u n c t i o n:         XX_MEM_infoPrint ()
// *
// * D e s c r i p t i o n :  reads out some pool information
// *
// *
// * A r g u m e n t s:       XX_MEM_INFO_T  *info     pointer to the info block
// *
// * Return Value:            XX_MEM_RET_OK or error number
// *
// *
// ***************************************************************************/
XX_MEM_DWORD XX_MEM_CODE_ATTR  XX_MEM_infoPrint( XX_MEM_INFO_T  *info)
{
    XX_MEM_DWORD   ret;
    
    ret = XX_MEM_info(info);

    if (ret != XX_MEM_RET_OK)
    {
        return(ret);
    }
    
    return(XX_MEM_RET_OK);
}
/*---------------------- end [subroutine] ---------------------------------*/




// ***************************************************************************/
// * F u n c t i o n:
// *
// * D e s c r i p t i o n :
// *
// *
// * A r g u m e n t s:
// *
// * Return Value:
// *
// *
// ***************************************************************************/
XX_MEM_DWORD XX_MEM_CODE_ATTR  XX_MEM_check(XX_MEM_BUF_T *pBuf)
{
    XX_MEM_BUF_HEADER_T   *h;
    XX_MEM_DWORD                ret;

    if ( pBuf->p == XX_MEM_NULL_PTR )
    {
        return(XX_MEM_RET_ERR_NULL_PTR);
    }

    h = (XX_MEM_BUF_HEADER_T *)(pBuf->p) - 1;

    ret = xx_mem_check_header(h);

    pBuf->AllocIndex = h->alloc_index;
    pBuf->Size       = h->buf_len;

    return(ret);
}
/*---------------------- end [subroutine] ---------------------------------*/



// ***************************************************************************/
// * F u n c t i o n:          xx_mem_get_checksum_header ()
// *
// * D e s c r i p t i o n :   build checksum of the blockheader
// *                           simple checksum here, take only the offs_own + 1
// *
// * A r g u m e n t s:
// *
// * Return Value:
// *
// *
// ***************************************************************************/
STATIC XX_MEM_DWORD xx_mem_get_checksum_header(  XX_MEM_BUF_HEADER_T *h )
{
    XX_MEM_DWORD    checksum;

    checksum = (XX_MEM_DWORD)(h->offs_own);

    checksum++;

    return(checksum);
}
/*---------------------- end [subroutine] ---------------------------------*/


// ***************************************************************************/
// * F u n c t i o n:          xx_mem_get_checksum_desc
// *
// * D e s c r i p t i o n :   builds a checksum for the descriptor about the
// *                           elements
// *                            - offs_first_buf
// *                            - size_aligned_without_desc
// *                            - do_preset_buffer
// *                            - preset_value
// *
// * A r g u m e n t s:         pDesc       descriptor address
// *
// * Return Value:              checksum    checksum value
// *
// *
// ***************************************************************************/
STATIC XX_MEM_DWORD xx_mem_get_checksum_desc(  XX_MEM_DESCRIPTOR_T   *  pDesc )
{
    XX_MEM_DWORD    checksum;

    checksum = (XX_MEM_DWORD)(pDesc->offs_first_buf) +
                pDesc->size_aligned_without_desc +
                pDesc->do_preset_buffer + pDesc->preset_value;

    checksum++;

    return(checksum);
}
/*---------------------- end [subroutine] ---------------------------------*/


// ***************************************************************************/
// * F u n c t i o n:         xx_mem_check_header ()
// *
// * D e s c r i p t i o n :  test the checksum, alignment and link-pointer
// *                          of a buffer header for validity
// *
// * A r g u m e n t s:       *h       buffer pointer header
// *
// * Return Value:            XX_MEM_RET_OK     header is ok
// *                          error-number      header not ok,
// *
// ***************************************************************************/
STATIC XX_MEM_DWORD xx_mem_check_header(  XX_MEM_BUF_HEADER_T     *  h )
{
    XX_MEM_DWORD                  checksum;
     XX_MEM_BUF_HEADER_T     *prev_h, *next_h, *hTmp;
     XX_MEM_BYTE                   * pBaseAdr;


    // *** check alignment of the buffer start ***
    if ( (XX_MEM_DWORD)h & XX_MEM_MIN_ALIGN )
    {
        return(XX_MEM_RET_INVALID_POINTER_ALIGN);
    }

    // *** test checksum ***
    checksum = xx_mem_get_checksum_header(h);

    if (h->pattern !=  XX_MEM_HEADER_PATTERN1)
        return (XX_MEM_RET_BUF_CORRUPT);

    if ( checksum != h->checksum )
    {
        if ( h->checksum == XX_MEM_CHECKSUM_FREE )
        {
            return(XX_MEM_RET_BUF_ALREADY_FREED);
        }

        return(XX_MEM_RET_BUF_CORRUPT);
    }

    pBaseAdr = (XX_MEM_BYTE *)((XX_MEM_DWORD)h - h->offs_own);

    // *------------------------------------------------------
    // * check the backward- and forward-linkpointer
    // *------------------------------------------------------
    if ( h->offs_prev_buf )
    {
        prev_h = (XX_MEM_BUF_HEADER_T  *)(pBaseAdr + h->offs_prev_buf);
        if ( prev_h->offs_next_buf != h->offs_own )
        {
            return(XX_MEM_RET_MEM_CORRUPT_PREV_POINTER);
        }
    }


    if ( h->offs_next_buf  )
    {
        next_h = (XX_MEM_BUF_HEADER_T  *)(pBaseAdr + h->offs_next_buf);
        hTmp   = (XX_MEM_BUF_HEADER_T  *)(pBaseAdr + next_h->offs_prev_buf);

        if ( hTmp->offs_own != h->offs_own )
        {
            return(XX_MEM_RET_MEM_CORRUPT_NEXT_POINTER);
        }
    }


    return(XX_MEM_RET_OK);
}
/*---------------------- end [subroutine] ---------------------------------*/


// ***************************************************************************/
// * F u n c t i o n:          xx_mem_check_header_during_alloc
// *
// * D e s c r i p t i o n :
// *
// *
// * A r g u m e n t s:
// *
// * Return Value:
// *
// *
// ***************************************************************************/
STATIC XX_MEM_DWORD xx_mem_check_header_during_alloc(  XX_MEM_BUF_HEADER_T     *  h )
{
    XX_MEM_DWORD                   checksum;
    XX_MEM_BUF_HEADER_T     *prev_h, *next_h;
    XX_MEM_BYTE             *pBaseAdr;


    if (h->pattern !=  XX_MEM_HEADER_PATTERN1)
        return (XX_MEM_RET_BUF_CORRUPT);

    // **** build checksum of blockheader again...
    checksum = xx_mem_get_checksum_header(h);

    // *** .. and compare with the already stored value ****
    if ( h->checksum != XX_MEM_CHECKSUM_FREE )  /* XX_MEM_CHECKSUM_FREE may happen at first buffer */
    {
        if ( checksum != h->checksum )
        {
            return(XX_MEM_RET_MEM_CORRUPT_PREV_OVERWRITE);
        }
    }

    // *** find the pool descriptor base address ***
    pBaseAdr = (XX_MEM_BYTE *)((XX_MEM_DWORD)h - h->offs_own);


    // *** find the header pointer of the previous and next block ***
    prev_h = (XX_MEM_BUF_HEADER_T  *)(pBaseAdr + h->offs_prev_buf);
    next_h = (XX_MEM_BUF_HEADER_T  *)(pBaseAdr + h->offs_next_buf);


    // *** plausibility: check if the previous block points to this block ***
    if ( h->offs_prev_buf && (prev_h->offs_next_buf != h->offs_own) )
    {
        return(XX_MEM_RET_MEM_CORRUPT_PREV_POINTER);
    }


    // *** plausibility: check if the next block points back to this block ***
    if ( h->offs_next_buf && (next_h->offs_prev_buf != h->offs_own) )
    {
        return(XX_MEM_RET_MEM_CORRUPT_NEXT_POINTER);
    }

    return(XX_MEM_RET_OK);
}
/*---------------------- end [subroutine] ---------------------------------*/



// ***************************************************************************/
// * F u n c t i o n:         xx_mem_check_desc ()
// *
// * D e s c r i p t i o n :  validity check of the pool descriptor
// *
// *
// * A r g u m e n t s:       *pDescr       pool descriptor pointer
// *
// * Return Value:            XX_MEM_RET_OK or error number
// *
// *
// ***************************************************************************/
STATIC XX_MEM_DWORD xx_mem_check_desc( XX_MEM_DESCRIPTOR_T   *  pDesc )
{
    XX_MEM_DWORD   checksum;

    if ( pDesc == XX_MEM_DESC_NULL_PTR )
    {
        return(XX_MEM_RET_ERR_NULL_PTR);
    }

    checksum = xx_mem_get_checksum_desc(pDesc);

    if ( checksum != pDesc->checksum )
    {
        return(XX_MEM_RET_DESC_CORRUPT);
    }

    return(XX_MEM_RET_OK);
}

/*---------------------- end [subroutine] ---------------------------------*/

#ifdef XX_MEM_VALIDATE
    // ***************************************************************************/
    // * F u n c t i o n:         xx_mem_validate

    // *
    // * D e s c r i p t i o n :  checks all block memory headers for validity
    // *
    // *
    // * A r g u m e n t s:       pDesc         pointer to buffer descriptor
    // *
    // * Return Value:            XX_MEM_OK     all block header ok
    // *                          else          error number
    // *
    // ***************************************************************************/
    #define  XX_MEM_MAX_BUFFER 0x00004000UL
    STATIC XX_MEM_DWORD xx_mem_validate( XX_MEM_DESCRIPTOR_T   *  pDesc )
    {
        XX_MEM_BUF_HEADER_T   *h;
        XX_MEM_DWORD                ret, i;

        // *** set header pointer to the first buffer ***
        h = (XX_MEM_BUF_HEADER_T  *)((XX_MEM_BYTE *)pDesc + pDesc->offs_first_buf);

        for ( i = 0; i <  XX_MEM_MAX_BUFFER; i++)
        {
            if ( h->offs_next_buf == M_NULL_H )
            {
                break;  //lint !e960
            }

            h = (XX_MEM_BUF_HEADER_T  *)((XX_MEM_BYTE *)pDesc + h->offs_next_buf);

            // don't check first buffer
            ret = xx_mem_check_header(h);
            if ( ret != XX_MEM_RET_OK )
            {
                return(ret);
            }
        }

        return(XX_MEM_RET_OK);
    }


    LSA_UINT32 xx_mem_validateAll (PNIO_VOID* pMemP)
    {
        PNIO_UINT32 Status;
        XX_MEM_DESCRIPTOR_T* pDscr = (XX_MEM_DESCRIPTOR_T*) pMemP;

        Status = xx_mem_validate (pDscr);


        return (Status);
    }

#endif

/*---------------------- end [subroutine] ---------------------------------*/



/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
