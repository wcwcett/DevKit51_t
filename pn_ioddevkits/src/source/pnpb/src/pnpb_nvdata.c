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
/*  F i l e               &F: pnpb_nvdata.c                             :F&  */
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

// *** general includes ***
#include "compiler.h"
#include "pniousrd.h"

// *** platform dependent includes ***
#include "pndv_inc.h"
#include "pnpb.h"
#include "glob_sys.h"
#include "os.h"
#include "trc_if.h"
#include "clrpc_inc.h"
#include "nv_data.h"

extern PNDV_IFACE_STRUCT_PTR   pPnpbIf;

// *-----------------------------------------------------------------
// * defines
// *-----------------------------------------------------------------


#define LTRC_ACT_MODUL_ID   205
#define PNPB_MODULE_ID      205


// *-----------------------------------------------------------------
// * external functions
// *-----------------------------------------------------------------

// *-----------------------------------------------------------------
// * external variables
// *-----------------------------------------------------------------

// *=================================================================
// * public variables
// *=================================================================



// *-----------------------------------------------------------------
// * static variables
// *-----------------------------------------------------------------

struct
{
    PNIO_UINT32     IpAddr;
    PNIO_UINT32     SubnetMask;
    PNIO_UINT32     DefRouter;
    PNIO_UINT8*     pStationName;
    PNIO_UINT16     StationNameLen;
    PNIO_UINT8*     pStationType;
    PNIO_UINT32     StationTypeLen;
    PNIO_UINT16     VendorId;
    PNIO_UINT16     DeviceId;
    PNIO_VOID*      pRemaData;
    PNIO_ANNOTATION DevAnnotation;
    PNPB_SNMP_DATA  SnmpData;
    PNIO_SUB_LIST_ENTRY *PDEVSubList;
    PNIO_UINT32     PDEVSubListLen;
} PnpbNvData;


// *----------------------------------------------------------------*
// *  PnpbNvInit ()
// *----------------------------------------------------------------*
PNIO_VOID PnpbNvInit (PNIO_VOID)
{
    PNPB_MEMSET(&PnpbNvData, 0, sizeof (PnpbNvData));
}

// ************************************************************************
//                      SET functions
// ************************************************************************


// *----------------------------------------------------------------*
// *  PnpbNvSetStationName ()
// *----------------------------------------------------------------*
PNIO_VOID PnpbNvSetStationName (PNIO_INT8   *pStationName,     // unique station name
                                 PNIO_UINT16 StationNameLen)
{

    if (    (PnpbNvData.pStationName == NULL)
         || (PnpbNvData.StationNameLen < StationNameLen)
         || (StationNameLen == 0)
         || (pStationName   == 0)
         )
    {
        if (PnpbNvData.pStationName != NULL)
        {
            PNPB_FREE_MEM(PnpbNvData.pStationName);
        }
        PnpbNvData.pStationName   = 0;
        PnpbNvData.StationNameLen = 0;
        if (StationNameLen)
        {
            PNPB_ALLOC_MEM((PNIO_VOID**) &PnpbNvData.pStationName, 0, StationNameLen);
        }
    }
    PnpbNvData.StationNameLen = StationNameLen;
    if (StationNameLen)
    {
        PNPB_COPY_BYTE((PNIO_VOID*) PnpbNvData.pStationName, pStationName, StationNameLen);
    }
}

// *----------------------------------------------------------------*
// *  PnpbNvSaveStationName ()
// *----------------------------------------------------------------*
PNIO_VOID PnpbNvSaveStationName (PNIO_INT8   *pStationName,     // unique station name
                                 PNIO_UINT16 StationNameLen,
                                 PNIO_BOOL   Remanent)
{
    PNIO_UINT32 Status;
    Status = PNIO_cbf_save_station_name (pStationName, StationNameLen, (PNIO_UINT8) Remanent);
    if (Status == PNIO_OK)
    {
        PnpbNvSetStationName(pStationName, StationNameLen);
        PNPB_API_TRACE_02 (PNIO_LOG_NOTE, "save StationNameLen=%d RM=%d", StationNameLen, Remanent);
    }
    else
    {
        PNPB_API_TRACE_02 (PNIO_LOG_WARNING_HIGH, "save StationNameLen=%d RM=%d", StationNameLen, Remanent);
    }
}


// *----------------------------------------------------------------*
// *  PnpbNvSetStationType ()
// *----------------------------------------------------------------*
PNIO_VOID PnpbNvSetStationType (PNIO_INT8   *pStationType,     // unique station name
                                PNIO_UINT32 StationTypeLen)
{
    if ((PnpbNvData.pStationType == NULL) || (PnpbNvData.StationTypeLen < StationTypeLen))
    {
        if (PnpbNvData.pStationType != NULL)
            PNPB_FREE_MEM(PnpbNvData.pStationType);
        PNPB_ALLOC_MEM((PNIO_VOID**) &PnpbNvData.pStationType, 0, StationTypeLen);
        PnpbNvData.StationTypeLen = StationTypeLen;
    }
    PNPB_COPY_BYTE((PNIO_VOID*) PnpbNvData.pStationType, pStationType, StationTypeLen);
}

// *----------------------------------------------------------------*
// *  PnpbNvSetIpSuite ()
// *----------------------------------------------------------------*
PNIO_VOID   PnpbNvSetIpSuite  (PNIO_UINT32 IpAddr,            // IP Suite: IP Addr
                               PNIO_UINT32 SubnetMask,        // IP Suite: Subnet Mask
                               PNIO_UINT32 DefRouterAddr)     // IP Suite: default router address
{
    PnpbNvData.IpAddr     = IpAddr;
    PnpbNvData.SubnetMask = SubnetMask;
    PnpbNvData.DefRouter  = DefRouterAddr;
}

PNIO_VOID Pnpb_set_new_address(PNIO_UINT32 param_to_change)
{
    PNIO_UINT8 cmd = 0;
    if (PNPB_CTRL_IP_SUITE == param_to_change)
    {
        pPnpbIf->ip_suite.ip_address  = PnpbNvData.IpAddr; 
        pPnpbIf->ip_suite.netmask     = PnpbNvData.SubnetMask; 
        pPnpbIf->ip_suite.gateway     = PnpbNvData.DefRouter; 
        pPnpbIf->ip_suite.mk_remanent = PNIO_TRUE;

        cmd = PNDV_EV_TO_PNDV_IP_SUITE;
    }
    else if (PNPB_CTRL_STATION_NAME == param_to_change)
    {
        pPnpbIf->name_of_station.pStationName   = PnpbNvData.pStationName; 
        pPnpbIf->name_of_station.StationNameLen = PnpbNvData.StationNameLen;

        cmd = PNDV_EV_TO_PNDV_NOS;
    }
    else
    {
        PNPB_API_TRACE_01(PNIO_LOG_ERROR_FATAL,
                          "This function support only change of ip_suite and station_name",
                          param_to_change);
        return; 
    }
    // no need to set add_1 and add_2 because no index required. 
    pnpb_write_event_to_pndv(cmd, 0, 0, (PNIO_VOID*)NULL);
    PNPB_TRIGGER_PNDV ();
}

/**
 *  @brief there is new ip data
 *
 *
 *  Reporting new ip to user, so it may be used for upper level (usrapp) functionality
 *
 *
 */
PNIO_UINT32 Pnpb_report_new_ip_data(PNIO_UINT32 NewIpAddr,
                                    PNIO_UINT32 SubnetMask,
                                    PNIO_UINT32 DefRouterAddr)
{
    PNIO_UINT32 IpAddrBigEnd;
    PNIO_UINT32 SubnetMaskBigEnd;
    PNIO_UINT32 DefRouterAddrBigEnd;

    IpAddrBigEnd = PNPB_SWITCH_TO_BIG_END(NewIpAddr);
    SubnetMaskBigEnd = PNPB_SWITCH_TO_BIG_END(SubnetMask);
    DefRouterAddrBigEnd = PNPB_SWITCH_TO_BIG_END(DefRouterAddr);

    return PNIO_cbf_report_new_ip_addr(IpAddrBigEnd, SubnetMaskBigEnd, DefRouterAddrBigEnd);
}

// *----------------------------------------------------------------*
// *  PnpbNvSaveIpSuite ()
// *----------------------------------------------------------------*
PNIO_VOID   PnpbNvSaveIpSuite (PNIO_UINT32 IpAddr,            // IP Suite: IP Addr
                               PNIO_UINT32 SubnetMask,        // IP Suite: Subnet Mask
                               PNIO_UINT32 DefRouterAddr,     // IP Suite: default router address
                               PNIO_BOOL   Remanent)          // LSA_TRUE: save remanent
{
    PNIO_UINT32 TrcLevel = PNIO_LOG_NOTE;
    PNIO_UINT32 Status;
    Status = PNIO_cbf_save_ip_addr(IpAddr, SubnetMask, DefRouterAddr, (PNIO_UINT8) Remanent);
    if (Status == PNIO_OK)
    {
        PnpbNvSetIpSuite (IpAddr, SubnetMask, DefRouterAddr);
    }
    else
    {
        TrcLevel = PNIO_LOG_WARNING_HIGH;
    }
    PNPB_API_TRACE_04 (TrcLevel, "save IP=0x%x SM==0x%x DR==0x%x RE=%d", IpAddr, SubnetMask, DefRouterAddr, Remanent);
}

// *----------------------------------------------------------------*
// *  PnpbNvSetVendorAndDeviceId ()
// *----------------------------------------------------------------*
PNIO_VOID PnpbNvSetVendorAndDeviceId (PNIO_UINT16 VendorId, PNIO_UINT16 DeviceId)
{
    PnpbNvData.VendorId = VendorId;
    PnpbNvData.DeviceId = DeviceId;
}


// *----------------------------------------------------------------*
// *  PnpbNvSetSnmpData()
// *----------------------------------------------------------------*
//lint -e{832, 578} Parameter 'Symbol' not explicitly declared, int assumed
PNIO_VOID PnpbNvSetSnmpData (PNIO_SNMP_LLDP *pSnmpData)
{
    PNIO_UINT32 Status = PNIO_OK;

    PNPB_MEMSET(&PnpbNvData.SnmpData, 0, sizeof (PnpbNvData.SnmpData));

    if (pSnmpData->SysNameLen <= PNPB_SYSNAME_MAXLEN)
    {
        PnpbNvData.SnmpData.SysNameLen = (PNIO_UINT16)pSnmpData->SysNameLen;
        PNPB_COPY_BYTE(&PnpbNvData.SnmpData.SysName[0],  pSnmpData->pSysName, pSnmpData->SysNameLen) ;
    }
    else
    {
        Status = PNIO_NOT_OK;
    }

    if (pSnmpData->SysContactLen <= PNPB_SYSCONT_MAXLEN)
    {
        PnpbNvData.SnmpData.SysContLen = (PNIO_UINT16)pSnmpData->SysContactLen;
        PNPB_COPY_BYTE(&PnpbNvData.SnmpData.SysCont[0],  pSnmpData->pSysContact, pSnmpData->SysContactLen) ;
    }
    else
    {
        Status = PNIO_NOT_OK;
    }

    if (pSnmpData->SysLocLen <= PNPB_SYSLOC_MAXLEN)
    {
        PnpbNvData.SnmpData.SysLocLen = (PNIO_UINT16)pSnmpData->SysLocLen;
        PNPB_COPY_BYTE(&PnpbNvData.SnmpData.SysLoc[0],   pSnmpData->pSysLoc, pSnmpData->SysLocLen) ;
    }
    else
    {
        Status = PNIO_NOT_OK;
    }

    if (pSnmpData->SysDescrLen <= PNPB_SYSDESCR_MAXLEN)
    {
        PnpbNvData.SnmpData.SysDescrLen = (PNIO_UINT16)pSnmpData->SysDescrLen;
        PNPB_COPY_BYTE(&PnpbNvData.SnmpData.SysDescr[0], pSnmpData->pSysDescr, pSnmpData->SysDescrLen) ;
    }
    else
    {
        Status = PNIO_NOT_OK;
    }

    if (Status == PNIO_OK)
    {
        PNPB_API_TRACE_04 (PNIO_LOG_NOTE, "set SNMP lName=%d lCont=%d lLoc=%d lDesc=%d",
                           pSnmpData->SysNameLen, pSnmpData->SysContactLen, pSnmpData->SysLocLen, pSnmpData->SysDescrLen);
    }
    else
    {
        PNPB_API_TRACE_04 (PNIO_LOG_ERROR, "Invalid SNMP lName=%d lCont=%d lLoc=%d lDesc=%d",
                           pSnmpData->SysNameLen, pSnmpData->SysContactLen, pSnmpData->SysLocLen, pSnmpData->SysDescrLen);
    }

}

// *----------------------------------------------------------------*
// *  PnpbNvGetpSnmpData()
// *----------------------------------------------------------------*
PNPB_SNMP_DATA* PnpbNvGetpSnmpData (PNIO_VOID)
{
    return  (&PnpbNvData.SnmpData);
}

PNIO_VOID PnpbNvpSnmpDatas ( PNIO_UINT8** ppName, PNIO_UINT32* pNameLen, PNIO_UINT8** ppContact, PNIO_UINT32* pContactLen, PNIO_UINT8** ppLocation, PNIO_UINT32* pLocationLen)
{
    if (PnpbNvData.SnmpData.SysNameLen)
    {
        *ppName = (PNIO_UINT8*)&PnpbNvData.SnmpData.SysName;
        *pNameLen = PnpbNvData.SnmpData.SysNameLen;
    }
    if (PnpbNvData.SnmpData.SysLocLen)
    {
        *ppLocation = (PNIO_UINT8*)&PnpbNvData.SnmpData.SysLoc;
        *pLocationLen = PnpbNvData.SnmpData.SysLocLen;
    }
    if (PnpbNvData.SnmpData.SysContLen)
    {
        *ppContact = (PNIO_UINT8*)&PnpbNvData.SnmpData.SysCont;
        *pContactLen = PnpbNvData.SnmpData.SysContLen;
    }
}

// *----------------------------------------------------------------*
// *  PnpbNvSetAnnotation()
// *----------------------------------------------------------------*
PNIO_VOID PnpbNvSetAnnotation        (PNIO_ANNOTATION *pDevAnnotation)
{
    PNPB_COPY_BYTE(&PnpbNvData.DevAnnotation, pDevAnnotation, sizeof (PNIO_ANNOTATION));
}

// *----------------------------------------------------------------*
// *  PnpbNvSetSubmodId()
// *----------------------------------------------------------------*
//lint -e{832, 578} Parameter 'Symbol' not explicitly declared, int assumed
PNIO_VOID PnpbNvSetSubmodId(PNIO_SUB_LIST_ENTRY   *pPDEVSubList,    // plugged PDEV
                            PNIO_UINT32           pPDEVSubListLen)  // number of entries in PDEV list
{
    PnpbNvData.PDEVSubList = pPDEVSubList;
    PnpbNvData.PDEVSubListLen = pPDEVSubListLen;
}

// ************************************************************************
//                      GET functions
// ************************************************************************

PNIO_VOID* PnpbNvGetAnnotation (PNIO_VOID)
{
    static PNIO_UINT8  ClrpcAnnotation [CLRPC_MAX_ANNOTATION_SIZE];

    // *--------------------------------------------------
    // *  build annotation string
    // *--------------------------------------------------
    // *** check string-length of deviceType and OrderId string **
    PNPB_MEMSET(&ClrpcAnnotation[0],  ' ', sizeof (ClrpcAnnotation));

    // RPCAnnotation ::=
    //  DeviceType, Blank, OrderID, Blank, HWRevision, Blank,
    //  SWRevisionPrefix, SWRevision, EndTerm
    PNIO_sprintf (&ClrpcAnnotation[0],
         CLRPC_PNIO_ANNOTATION_FORMAT,
         &(PnpbNvData.DevAnnotation.DeviceType[0]),  //DeviceType
         &(PnpbNvData.DevAnnotation.OrderId[0]),     //OrderID
         PnpbNvData.DevAnnotation.HwRevision,          //HWRevision
         PnpbNvData.DevAnnotation.SwRevisionPrefix,    //SWRevisionPrefix
         PnpbNvData.DevAnnotation.SwRevision1,
         PnpbNvData.DevAnnotation.SwRevision2,
         PnpbNvData.DevAnnotation.SwRevision3
        );
    return (&ClrpcAnnotation[0]);
}

PNIO_UINT32  PnpbNvGetStationType  (PNIO_UINT8* pStationType, PNIO_UINT32 BufSize)
{
    if (BufSize < PnpbNvData.StationTypeLen)
    {
        PNPB_API_TRACE_02 (LSA_TRACE_LEVEL_FATAL, "TypeLen(%d) > BufSize(%d)", PnpbNvData.StationTypeLen, BufSize);
        return (0);
    }
    PNPB_MEMSET(pStationType, 0, BufSize);      // create a zero terminated string
    PNPB_COPY_BYTE(pStationType, PnpbNvData.pStationType, PnpbNvData.StationTypeLen);
    return (PnpbNvData.StationTypeLen);
}

// *----------------------------------------------------------------*
// *  PnpbNvGetpStationName ()
// *----------------------------------------------------------------*
PNIO_UINT32 PnpbNvGetStationName ( PNIO_UINT8* pStationName, PNIO_UINT32 BufSize)
{
    if (BufSize < PnpbNvData.StationNameLen)
    {
        PNPB_API_TRACE_02 (LSA_TRACE_LEVEL_FATAL, "NameLen(%d) > BufSize(%d)", PnpbNvData.StationNameLen, BufSize);
        return (0);
    }
    PNPB_COPY_BYTE(pStationName, PnpbNvData.pStationName, PnpbNvData.StationNameLen);
    return (PnpbNvData.StationNameLen);
}


PNIO_VOID PnpbNvGetpStationName ( PNIO_UINT8** ppStationName, PNIO_UINT16* pStationNameLen)
{
    *ppStationName = PnpbNvData.pStationName;
    *pStationNameLen = PnpbNvData.StationNameLen;
}

// *----------------------------------------------------------------*
// *  PnpbNvGetDeviceType ()
// *----------------------------------------------------------------*
PNIO_UINT32 PnpbNvGetDeviceType(PNIO_UINT8* pDeviceType, PNIO_UINT32 BufSize)
{
    if (MAX_DEVICE_TYPE_LENGTH > BufSize)
    {
        PNPB_API_TRACE_02(LSA_TRACE_LEVEL_FATAL, "DeviceTypeLen(%d) > BufSize(%d)", MAX_DEVICE_TYPE_LENGTH, BufSize);
        return (0);
    }
    PNPB_COPY_BYTE(pDeviceType, &PnpbNvData.DevAnnotation.DeviceType[0], MAX_DEVICE_TYPE_LENGTH);

    return (MAX_DEVICE_TYPE_LENGTH);
}

// *----------------------------------------------------------------*
// *  PnpbNvGetOrderId ()
// *----------------------------------------------------------------*
PNIO_UINT32 PnpbNvGetOrderId ( PNIO_UINT8* pOrderId, PNIO_UINT32 BufSize)
{
    if (MAX_ORDER_ID_LENGTH > BufSize)
    {
        PNPB_API_TRACE_02 (LSA_TRACE_LEVEL_FATAL, "OrderIdLen(%d) > BufSize(%d)", MAX_ORDER_ID_LENGTH, BufSize);
        return (0);
    }
    PNPB_COPY_BYTE(pOrderId, &PnpbNvData.DevAnnotation.OrderId[0], MAX_ORDER_ID_LENGTH);

    return (MAX_ORDER_ID_LENGTH);
}

// *----------------------------------------------------------------*
// *  PnpbNvGetSerialNumber ()
// *----------------------------------------------------------------*
PNIO_UINT32 PnpbNvGetSerialNumber ( PNIO_UINT8* pSerialNumber, PNIO_UINT32 BufSize)
{
    if (sizeof(PnpbNvData.DevAnnotation.SerialNumber) > BufSize)
    {
        PNPB_API_TRACE_02 (LSA_TRACE_LEVEL_FATAL, "SerialNumberLen(%d) > BufSize(%d)", sizeof(PnpbNvData.DevAnnotation.SerialNumber), BufSize);
        return (0);
    }
    PNPB_COPY_BYTE(pSerialNumber, &PnpbNvData.DevAnnotation.SerialNumber[0], sizeof(PnpbNvData.DevAnnotation.SerialNumber));
    return (sizeof(PnpbNvData.DevAnnotation.SerialNumber));
}


// *----------------------------------------------------------------*
// *  PnpbNvGetHwRevision ()
// *----------------------------------------------------------------*
PNIO_UINT32 PnpbNvGetHwRevision (PNIO_VOID)
{
    PNIO_UINT32 HwRevision = PnpbNvData.DevAnnotation.HwRevision;
    if (HwRevision > 99999)
    {
        return (99999);
    }
    return (HwRevision);
}

// *----------------------------------------------------------------*
// *  PnpbNvGetSwRevision ()
// *----------------------------------------------------------------*
PNIO_VOID PnpbNvGetSwRevision(LSA_UINT8* pRevPref, LSA_UINT8* pFncEnhc, LSA_UINT8* pBugFix, LSA_UINT8* pIntChg)
{
    *pRevPref = (LSA_UINT8)PnpbNvData.DevAnnotation.SwRevisionPrefix;
    *pFncEnhc = (LSA_UINT8)PnpbNvData.DevAnnotation.SwRevision1;
    *pBugFix  = (LSA_UINT8)PnpbNvData.DevAnnotation.SwRevision2;
    *pIntChg  = (LSA_UINT8)PnpbNvData.DevAnnotation.SwRevision3;
}

// *----------------------------------------------------------------*
// *  PnpbNvGetVendorId ()
// *----------------------------------------------------------------*
PNIO_UINT16 PnpbGetVendorId (PNIO_VOID)
{
    return (PnpbNvData.VendorId);
}

// *----------------------------------------------------------------*
// *  PnpbNvGetDeviceId ()
// *----------------------------------------------------------------*
PNIO_UINT16 PnpbGetDeviceId (PNIO_VOID)
{
    return (PnpbNvData.DeviceId);
}

// *----------------------------------------------------------------*
// *  PnpbGetModId ()
// *----------------------------------------------------------------*
PNIO_UINT32 PnpbGetModId(PNIO_UINT32 SubmodIdx)
{
    return PnpbNvData.PDEVSubList[SubmodIdx].ModId;
}

// *----------------------------------------------------------------*
// *  PnpbGetSubmodId ()
// *----------------------------------------------------------------*
PNIO_UINT32 PnpbGetSubmodId(PNIO_UINT32 SubmodIdx)
{
    return PnpbNvData.PDEVSubList[SubmodIdx].SubId;
}

// *----------------------------------------------------------------*
// *  PnpbGetSubmodCnt ()
// *----------------------------------------------------------------*
PNIO_UINT32 PnpbGetSubmodCnt(PNIO_VOID)
{
    return PnpbNvData.PDEVSubListLen;
}

// *----------------------------------------------------------------*
// *  PnpbNvGetIpSuite ()
// *----------------------------------------------------------------*
PNIO_VOID   PnpbNvGetIpSuite  (PNIO_UINT32* pIpAddr,            // IP Suite: IP Addr
                               PNIO_UINT32* pSubnetMask,        // IP Suite: Subnet Mask
                               PNIO_UINT32* pDefRouterAddr)     // IP Suite: default router address
{
    // attention: destination pointer may not be 4 byte aligned -> use byte copy
    PNPB_COPY_BYTE(pIpAddr,        &PnpbNvData.IpAddr,     sizeof (PnpbNvData.IpAddr));
    PNPB_COPY_BYTE(pSubnetMask,    &PnpbNvData.SubnetMask, sizeof (PnpbNvData.SubnetMask));
    PNPB_COPY_BYTE(pDefRouterAddr, &PnpbNvData.DefRouter,  sizeof (PnpbNvData.DefRouter));

}

PNIO_VOID PnpbNvGetpIpSuit ( PNIO_UINT8** ppIP, PNIO_UINT32* pIPLen)
{
    *ppIP = (PNIO_UINT8*)&PnpbNvData.IpAddr;
    *pIPLen = 0xc;
}
//lint -e{832, 578} Parameter 'Symbol' not explicitly declared, int assumed
PNIO_VOID   PnpbNvResetToFactorySettings (PNIO_RTF_OPTION RtfOption)
{
    PNIO_cbf_reset_factory_settings(PNIO_SINGLE_DEVICE_HNDL,
                                    RtfOption);          // suboption "reset all"
}



// *----------------------------------------------------------------*
// *  PnpbNvRestoreRemaMem (...)
// *----------------------------------------------------------------*
PNIO_UINT32  PnpbNvRestoreRemaMem
(
            PNIO_UINT32*        pMemSize,       // REMA data size
            PNIO_UINT8**        ppMem           // pointer to pointer to rema data
)
{
    PNIO_UINT32 Status;

    static PNIO_INT ExecCounter = 0;

    ExecCounter++;
    if (ExecCounter != 1)
    {
        PNPB_API_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "PnpbNvRestoreRemaMem only call once\n");
    }

    // first delete old data block, if present
    if (PnpbNvData.pRemaData)
    {
        PNIO_cbf_free_rema_mem ((PNIO_UINT8*)PnpbNvData.pRemaData);
        PnpbNvData.pRemaData = NULL;
    }

    Status = PNIO_cbf_restore_rema_mem (pMemSize, ppMem); // PNIO_cbf_restore_rema_mem allocates new memory
    if (Status == PNIO_OK)
    {
        PnpbNvData.pRemaData = *ppMem;
    }

    return (Status);
}


// *----------------------------------------------------------------*
// *  PnpbNvStoreRemaMem (...)
// *----------------------------------------------------------------*
PNIO_UINT32  PnpbNvStoreRemaMem
        (
            PNIO_UINT32         MemSize,        // REMA Opcode
            PNIO_UINT8*         pMem            // request information
        )
{
    return (PNIO_cbf_store_rema_mem (MemSize, pMem));
}


// *----------------------------------------------------------------*
// *  PnpbNvStoreRemaMem(...)
// *----------------------------------------------------------------*
PNIO_UINT32  PnpbNvStoreSnmpData
        (
            PNIO_UINT16         SysNameLen,
            PNIO_UINT8*         pSysName,
            PNIO_UINT16         SysContLen,
            PNIO_UINT8*         pSysCont,
            PNIO_UINT16         SysLocLen,
            PNIO_UINT8*         pSysLoc
        )
{
    PNIO_UINT32 Status;

    Status =  Bsp_nv_data_store (PNIO_NVDATA_SNMP_SYSNAME,              // nv data type: snmp sysName
                                 pSysName,                              // source pointer to the object
                                 SysNameLen);                           // length of the object

    if (Status == PNIO_OK)
    {
        Status =  Bsp_nv_data_store (PNIO_NVDATA_SNMP_SYSCONT,          // nv data type: snmp sysContact
                                     pSysCont,                          // source pointer to the object
                                     SysContLen);                       // length of the object
    }
    if (Status == PNIO_OK)
    {
        Status =  Bsp_nv_data_store (PNIO_NVDATA_SNMP_SYSLOC,           // nv data type: snmp sysLocation
                                     pSysLoc,                           // source pointer to the object
                                     SysLocLen);                        // length of the object
    }
    return (Status);
}


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
