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
/*  F i l e               &F: Tcp_IF.h                                  :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Example : TCP INTERFACE                                                  */
/*            performs point to point TCP connection to a remote device      */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/

#ifndef _TCP_IF_H
#define _TCP_IF_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif
// *---------------------------------------------------------------------------*/
// *  TCP_IF.h
// *  
// *  
// *  
// *  
// *  
// *---------------------------------------------------------------------------*/
#define TCP_SOCK_ERROR   -1

int tcp_if_inits        (void);
unsigned int tcp_if_connectC    (int LocalSockId,  unsigned int   RemIpAddr,  unsigned int Port);
         int tcp_if_connectS    (int LocalSockId,  unsigned int   RemIpAddr,  unsigned int Port);
unsigned int tcp_if_send        (int RemoteSockId, unsigned char* pDat,       unsigned int DatLen);
unsigned int tcp_if_receive     (int RemoteSockId, unsigned char* pDat,       unsigned int MaxDatLen);
unsigned int tcp_if_disconnect  (int RemoteSockId);  
unsigned int tcp_if_close       (int LocalSockId);




#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif  // __cplusplus

#endif  // _T_UTILS_H


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
