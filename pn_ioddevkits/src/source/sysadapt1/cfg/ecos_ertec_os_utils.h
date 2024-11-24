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
/*  F i l e               &F: ecos_ertec_os_utils.h                     :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
#ifndef _ECOS_ERTEC_OS_UTILS_H
#define _ECOS_ERTEC_OS_UTILS_H

/* stdio.h library is used */
#define HAVE_STDIO_H

/* stdlib.h library is used */
#define HAVE_STDLIB_H

/* stdint.h library is used */
#define HAVE_STDINT_H

/* stddef.h library is used */
#define HAVE_STDDEF_H

/* stdarg.h library is used */
#define HAVE_STDARG_H

/* string.h library is used */
#define HAVE_STRING_H

/* fcntl.h library is used */
#define HAVE_FCNTL_H

/* sys/stat.h library is used */
#define HAVE_SYS_STAT_H

/* arpa/inet.h library is not used */
#undef HAVE_ARPA_INET_H

/* math.h library is used */
#define HAVE_MATH_H

/* pthread.h library is used */
#define HAVE_PTHREAD_H

/* semaphore.h library is used */
#define HAVE_SEMAPHORE_H

/* signal.h library is used */
#define HAVE_SIGNAL_H

/* inttypes.h library is used */
#define HAVE_INTTYPES_H

/* mqueues.h library is used */
#define HAVE_MQUEUES_H

/* errno.h library is used */
#define HAVE_ERRNO_H

#if PNIO_BIG_ENDIAN
#define htonl(l) (l)
#define ntohl(l) (l)
#define htons(s) (s)
#define ntohs(s) (s)
#else
#define htonl(l) (((l & 0x000000ff)<<24 ) | ((l & 0x0000ff00)<<8) | ((l & 0x00ff0000)>>8) | ((l & 0xff000000)>>24))
#define ntohl(l) (((l & 0x000000ff)<<24 ) | ((l & 0x0000ff00)<<8) | ((l & 0x00ff0000)>>8) | ((l & 0xff000000)>>24))
#define htons(s) (((s & 0x00FF)<<8 ) | ((s & 0xFF00)>>8));
#define ntohs(s) (((s & 0x00FF)<<8 ) | ((s & 0xFF00)>>8));
#endif

#define OS_TIMER_NS_VALUE   500000

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
