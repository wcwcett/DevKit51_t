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
/*  F i l e               &F: sys_pck1.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Packing Makros                                                           */
/*                                                                           */
/*****************************************************************************/
#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

/*
 * no re-include protection!!!
 */


/*---------------------------------------------------------------------------*/
/*      1.  UNDEF PACKED DEFINES                                             */
/*---------------------------------------------------------------------------*/

#if defined (SYS_BYTE_PACKED)
	#error "why is it defined?"
#endif

#if defined (SYS_WORD_PACKED)
	#error "why is it defined?"
#endif

#if defined (SYS_PRAGMA_PACK)
	#error "why is it defined?"
#endif

/*---------------------------------------------------------------------------*/
/*      2.  DEFINE BYTE-PACKED                                               */
/*---------------------------------------------------------------------------*/
#define SYS_BYTE_PACKED       ATTR_PNIO_PACKED
#define SYS_BYTE_PACKED_PRE   ATTR_PNIO_PACKED_PRE



/*----------------------------------------------------------------------*/
/*      3.  SET BYTE-ALIGNMENT                                          */
/*----------------------------------------------------------------------*/

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_WIN_EB200P)
	#pragma pack(1)

#elif (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
    #pragma pack (1)

#else
    #error "tool-chain not defined"
#endif

/*---------------------------------------------------------------------------*/
/*      4.  SANITY CHECK                                                     */
/*---------------------------------------------------------------------------*/

#if defined (SYS_BYTE_PACKED) && defined (SYS_WORD_PACKED)
	#error "both not possible! see pragmas"
#endif

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
