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
/*  F i l e               &F: pnpb_sub_real.h                           :F&  */
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
#ifndef PNPB_SUB_REAL_H
#define PNPB_SUB_REAL_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif


typedef enum
{
    PNPB_NOT_PLUGGED  = 0,
    PNPB_PLUGGED      = 1
} REAL_PLUG_STATE;

typedef enum
{
    STOP              = 0,  //  submodule is in stop state and has invalid data
    RUN               = 1   //  submodule is in run state and has valid data
}  REAL_SUBMOD_STATE;

typedef struct
{
    LSA_UINT32            ApiNum;               // PNIO subslot number (0...0xffff)
    LSA_UINT32            SlotNum;              // PNIO subslot number (0...0xffff)
    LSA_UINT32            SubNum;               // subslot handle = table index in SubslotArray
    LSA_UINT32            ModIdent;             // submodule ident number (from GSDML file)
    LSA_UINT32            SubIdent;             // submodule ident number (from GSDML file)
    LSA_UINT32            DataDirection;        // NoData, In, Out, InOut
    LSA_UINT32            PeriphRealCfgInd;     // entity index in periph interface-real_cfg
    REAL_PLUG_STATE       PlugState;            // submodule is plugged/not plugged
    REAL_SUBMOD_STATE     SubmodState;          // state of the submodule (RUN/STOP)
#if IOD_INCLUDE_IM0_4
    PNIO_IM0_SUPP_ENUM    Im0Support;           // defines if IM0 is supported for subslot/slot/device or not
    IM0_DATA              Im0Dat;               // IM0 data
#endif
    PNIO_UINT8            InIops;               // iops of input data  (if submodule has input data)
} PNPB_REAL_SUB;

// *-------------------------------------------------
// *    typedef for PNPB instance data
// *-------------------------------------------------
typedef struct PULL_PENDING
{
    PNIO_UINT32 WritePending;
    PNIO_UINT32 ReadPending;
    PNIO_UINT32 Slot;
    PNIO_UINT32 Subslot;
    PNIO_UINT32 DataDir;
}PULL_PENDING;
PULL_PENDING    PullPending;

// *-------------------------------------------------
// *    typedef for PNPB instance data
// *-------------------------------------------------
typedef struct _PNPB_REAL
{
    // *** device identification
    PNIO_UINT32     NumOfPluggedSub;                    // current number of pluggedsubslots
    PNPB_REAL_SUB   Sub[IOD_CFG_MAX_NUMOF_SUBSLOTS];    // list of subslots
} PNPB_REAL;


// *-------------------------------------------------
// *    PNPB internal functions
// *-------------------------------------------------
PNIO_VOID       pnpb_sub_real_init(PNIO_VOID);

PNPB_REAL_SUB*  pnpb_sub_real_getp
                    (PNIO_UINT32 ApiNum,
                     PNIO_UINT32 SlotNum,
                     PNIO_UINT32 SubNum);

PNIO_UINT32     pnpb_sub_real_entity
                     (PNIO_UINT32 ApiNum,
                      PNIO_UINT32 SlotNum,
                      PNIO_UINT32 SubNum,
                      LSA_UINT32  ModIdent);

PNIO_UINT32     pnpb_device_ar_abort
                     (PNIO_UINT32 ArNum);

// *-------------------------------------------------
// *    public functions
// *-------------------------------------------------
PNIO_UINT32     pnpb_sub_real_plug
                        (PNIO_UINT32         DevHndl,       // [in] must be PNIO_SINGLE_DEVICE_HNDL
                         PNIO_UINT32         ApiNum,        // [in] API number
                         PNIO_DEV_ADDR       *pAddr,        // [in] slot/subslot number
                         LSA_UINT32          ModIdent,      // [in] module ident number
                         LSA_UINT32          SubIdent,      // [in] submodule ident number
                         PNIO_UINT32         InputDataLen,  // [in] submodule input data length
                         PNIO_UINT32         OutputDataLen, // [in] submodule output data length               
                         PNIO_IM0_SUPP_ENUM  Im0Support,    // [in] subslot has IM0 data for subslot/slot/device/nothing
                         IM0_DATA            *pIm0Dat,      // [in] pointer to IM0 data set (if subslot has own IM0 data)
                         PNIO_UINT8          IopsIniVal,    // [in] initial value for iops-input, ONLY FOR SUBMOD WITHOUT IO DATA (e.g. PDEV)
                         PNIO_UINT32         *pIndex,       // [in, out] pointer to index in PnpbMod, includes indizes in PnpbMod.Sub[] after return
                         PNIO_BOOL           MoreFollows);  // [in] more plug requests follow...

PNIO_UINT32     pnpb_sub_real_plug_list
                        (PNIO_SUB_LIST_ENTRY   *pIoSubList,           // plugged submodules, including PDEV
                         PNIO_UINT32           NumOfSubListEntries,   // number of entries in pIoSubList
                         PNIO_IM0_LIST_ENTRY   *pIm0List,             // list of IM0 data sets
                         PNIO_UINT32           NumOfIm0ListEntries,   // number of entries in pIm0List
                         PNIO_UINT32*          pStatusList);          // list of return-Stati[NumOfSublistEntries]

PNIO_UINT32     pnpb_sub_real_pull
                   (PNIO_UINT32         DevHndl,
                    PNIO_UINT32         ApiNum,
                    PNIO_DEV_ADDR*      pAddr);



#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif



#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
