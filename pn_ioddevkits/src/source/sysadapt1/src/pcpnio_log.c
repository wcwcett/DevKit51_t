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
/*  F i l e               &F: pcpnio_log.c                              :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  pniod logging functions                                                  */
/*                                                                           */
/*****************************************************************************/

#define LTRC_ACT_MODUL_ID   128
#define IOD_MODULE_ID       128

#include "compiler.h"
#include "pniousrd.h"
#include "lsa_usr.h"


// *---------- defines --------------
#ifndef LSA_UNUSED_ARG
    #define LSA_UNUSED_ARG(arg_)    {arg_ = arg_;}
#endif


// *----------static data --------------





// *----------------------------------------------------------------*/
// * PNIO_set_log_level
// *
// *----------------------------------------------------------------*/
PNIO_UINT32 PNIO_set_log_level
                    (PNIO_UINT32  NewLogLevel, 
                     PNIO_UINT32  PackId)
{
    PNIO_printf("function PNIO_set_log_level %d %d not supported\n",
                 NewLogLevel, PackId);
    return (PNIO_OK);
}

// *----------------------------------------------------------------*/
// * DecodePacketId
// * Returns PNIO Pack ID equivalents of LSA COMP IDs
// *----------------------------------------------------------------*/
PNIO_UINT32 DecodePackId (PNIO_UINT32 LsaPackId)
{
    LSA_UINT32 ApiPackId;

    switch (LsaPackId)
    {
        case LSA_COMP_ID_ACP:       ApiPackId = PNIO_PACKID_ACP;       break;
        case LSA_COMP_ID_CLRPC:     ApiPackId = PNIO_PACKID_CLRPC;     break;
        case LSA_COMP_ID_CM:        ApiPackId = PNIO_PACKID_CM;        break;
        case LSA_COMP_ID_EDD:       ApiPackId = PNIO_PACKID_EDDP;      break;
        case LSA_COMP_ID_EDDP:      ApiPackId = PNIO_PACKID_EDDP;      break;
        case LSA_COMP_ID_GSY:       ApiPackId = PNIO_PACKID_GSY;       break;
        case LSA_COMP_ID_IP2PN:     ApiPackId = PNIO_PACKID_IP2PN;     break;
        case LSA_COMP_ID_LLDP:      ApiPackId = PNIO_PACKID_LLDP;      break;
        case LSA_COMP_ID_MRP:       ApiPackId = PNIO_PACKID_MRP;       break;
        case LSA_COMP_ID_SOCK:      ApiPackId = PNIO_PACKID_SOCK;      break;
        case LSA_COMP_ID_SNMPX:     ApiPackId = PNIO_PACKID_SNMPX;     break;
        case LSA_COMP_ID_POF:       ApiPackId = PNIO_PACKID_POF;       break;
        case LSA_COMP_ID_PSI:       ApiPackId = PNIO_PACKID_PSI;       break;

        default:                    ApiPackId = PNIO_PACKID_OTHERS;
        break;
    }

    return (ApiPackId);
}


// *----------------------------------------------------------------*/
// * log output to file and console, if enabled
// *
// *----------------------------------------------------------------*/
PNIO_VOID pcpnio_log (LSA_UINT32 ErrLevel,
                      LSA_UINT32 PackId, 
                      LSA_UINT32 ModId, 
                      LSA_UINT32 LineNum) 
{
    LSA_UINT32 ApiPackId;

    ApiPackId = DecodePackId (PackId);

    PNIO_Log (PNIO_SINGLE_DEVICE_HNDL,
              ErrLevel,   // error level (fatal, error, important, normal, ..)
              ApiPackId,  // LSA packet ID
              ModId,      // module ID 
              LineNum);   // line number in source code
}


// *----------------------------------------------------------------*/
// * log output to file and console, if enabled
// *
// *----------------------------------------------------------------*/
PNIO_VOID pcpnio_lsa_log (LSA_UINT32 MsgType,
                          LSA_UINT32 PackId, 
                          LSA_UINT32 ModId, 
                          LSA_UINT32 LineNum,
                          LSA_UINT32 Length,
                          LSA_FATAL_ERROR_TYPE* pErr) 
{
    LSA_UINT32 ApiPackId;

    LSA_UNUSED_ARG (pErr);
    LSA_UNUSED_ARG (Length);

    ApiPackId = DecodePackId (PackId);

    PNIO_TrcPrintf ("LSA-ERROR, Comp %d ModId %d line %d, ErrCod0=0x%x ErrCod1=0x%x ErrCod2=0x%x ErrCod3=0x%x Len=%d pDat=0x%x\n", 
                    pErr->lsa_component_id,
                    pErr->module_id,
                    pErr->line,
                    pErr->error_code[0],
                    pErr->error_code[1],
                    pErr->error_code[2],
                    pErr->error_code[3],
                    pErr->error_data_length,
                    pErr->error_data_ptr
                    );

    PNIO_Log (PNIO_SINGLE_DEVICE_HNDL,
              MsgType,    // message type (error, important, normal, ..)
              ApiPackId,  // LSA packet ID
              ModId,      // module ID 
              LineNum);   // line number in source code
}

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
