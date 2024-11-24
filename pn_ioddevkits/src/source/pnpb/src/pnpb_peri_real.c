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
/*  F i l e               &F: pnpb_peri_real.c                          :F&  */
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


#include "compiler.h"
#include "pniousrd.h"
#include "trc_if.h"
#include "pndv_inc.h"
#include "pnpb.h"

#define LTRC_ACT_MODUL_ID   207
#define PNPB_MODULE_ID      207


// *-----------------------------------------------------------------
// * defines
// *-----------------------------------------------------------------


// *-----------------------------------------------------------------
// * external functions
// *-----------------------------------------------------------------

// *-----------------------------------------------------------------
// * external variables
// *-----------------------------------------------------------------
extern PNDV_IFACE_STRUCT_PTR   pPnpbIf;
extern PNPB_INSTANCE           Pnpb;


// *=================================================================
// * static functions
// *=================================================================
static PNIO_UINT8  RealPlugList[PNDV_MAX_SV_ENTITY];   // PERI real config plug list

// *=================================================================
// * public functions
// *=================================================================
PNIO_VOID PnpbRealCfgInit (PNIO_VOID)
{
    PNPB_MEMSET(&RealPlugList[0], 0, sizeof (RealPlugList));
}

// *-----------------------------------------------------------------
// *  PnpbRealCfgOccupy()
// *
// *  occupies a new free index in real cfg list
// *-----------------------------------------------------------------
// *  input:   PrefInd        // preferred index or PNPB_INVALID_INDEX --> force 1:1 mapping
// *
// *  output:  EntityIndex
// *-----------------------------------------------------------------
PNIO_UINT32 PnpbRealCfgOccupy  (PNIO_UINT32 PrefInd)
{
    PNIO_UINT32 Ind = PNPB_INVALID_INDEX;   // invalid value

    if (PrefInd != PNPB_INVALID_INDEX)
    {
        if (RealPlugList[PrefInd] == PNIO_FALSE)
        { // free index found, occupy
            RealPlugList[PrefInd] = PNIO_TRUE;
            return (PrefInd);
        }
        else
        {
            PNPB_SYS_TRACE_01(LSA_TRACE_LEVEL_FATAL,  "occupy error, PrefIndex=%d\n", PrefInd);
        }
    }

    for (Ind = 0; Ind < PNDV_MAX_SV_ENTITY; Ind++)
    {
        if (RealPlugList[Ind] == PNIO_FALSE)
        { // free index found, occupy
            RealPlugList[Ind] = PNIO_TRUE;
            return (Ind);
        }
    }

    PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_FATAL,  "occupy error in real cfg list of PERI");
    return (Ind);
}

// *-----------------------------------------------------------------
// *  PnpbRealCfgFree()
// *
// *  frees entry in real cfg list
// *-----------------------------------------------------------------
// *  input:   EntityIndex      index in real cfg list
// *
// *  output:  ---
// *-----------------------------------------------------------------
PNIO_VOID   PnpbRealCfgFree  (PNIO_UINT32 Ind)
{
    PNPB_MEMSET(&pPnpbIf->real_cfg[Ind], 0, sizeof (pPnpbIf->real_cfg[0]));
    pPnpbIf->real_cfg[Ind].elem.state = PNDV_IFACE_SERVICE_IDLE;  // restore Idle state for replugging
    RealPlugList[Ind] = PNIO_FALSE;
}

// *-----------------------------------------------------------------
// *  PnpbRealCfgGetApiSlotSubslot()
// *
// *  returns api, slot- and subslot number for a specified entity index
// *  in the real_cfg of PERIF
// *-----------------------------------------------------------------
// *  input:   entity_index     index in real_cfg list
// *
// *  output:  *pApi            API number
// *           *pSlot           slot number
// *           *pSub            subslot number
// *           return           PNIO_OK, PNIO_NOT_OK
// *-----------------------------------------------------------------
PNIO_UINT32 PnpbRealCfgGetApiSlotSubslot   (PNIO_UINT32 EntityInd,
                                            PNIO_UINT32*pApi,
                                            PNIO_UINT32 *pSlot,
                                            PNIO_UINT32 *pSub)
{
    PNDV_REAL_CFG_T* iface_real_cfg_ptr;

    if (RealPlugList[EntityInd] == PNIO_FALSE)
    { // slot not occupied, error
        PNPB_SYS_TRACE_01(LSA_TRACE_LEVEL_UNEXP,  "Entity %d not valid in PERIF\n", EntityInd);
        return (PNIO_NOT_OK);
    }
    iface_real_cfg_ptr = &pPnpbIf->real_cfg[EntityInd];
    *pSlot = iface_real_cfg_ptr->elem.sub_descr.slot_nr;
    *pSub  = iface_real_cfg_ptr->elem.sub_descr.subslot_nr;
    *pApi  = iface_real_cfg_ptr->elem.sub_descr.api;

    return (PNIO_OK);
}

// *-----------------------------------------------------------------
// *  PnpbRealCfgGetEntityInd()
// *
// *  searches for a subslot in real cfg list
// *-----------------------------------------------------------------
// *  input:   Api              api number
// *           Slot             slot number
// *           Sub              subslot number
// *
// *  output:  return           EntityIndex, if entry found
// *                            PNPB_INVALID_INDEX, if entry not found
// *-----------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32 PnpbRealCfgGetEntityInd (PNIO_UINT32  Api,     // api number
                                     PNIO_UINT32  Slot,    // slot number
                                     PNIO_UINT32  Sub)     // subslot number
{
    PNIO_UINT32 Ind = PNPB_INVALID_INDEX;   // invalid value
    PNDV_REAL_CFG_T* iface_real_cfg_ptr;

    for (Ind = 0; Ind < PNDV_MAX_SV_ENTITY; Ind++)
    {
        if (RealPlugList[Ind] == PNIO_TRUE)
        { // check occupied entry
            iface_real_cfg_ptr = &pPnpbIf->real_cfg[Ind];
            if (   (iface_real_cfg_ptr->elem.sub_descr.api        == Api)
                 &&(iface_real_cfg_ptr->elem.sub_descr.slot_nr    == Slot)
                 &&(iface_real_cfg_ptr->elem.sub_descr.subslot_nr == Sub))
            {
                return (Ind);
            }
        }
    }
    PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_UNEXP,  "entry not found in real cfg list of PERI");
    return (PNPB_INVALID_INDEX);
}

PNIO_UINT32 PnpbAlarmSubPlugGetState (PNIO_UINT32 PeriIndex)
{
   if (pPnpbIf->real_cfg[PeriIndex].elem.sub_descr.response == 0)
       return (PNIO_OK);
   else
       return (PNIO_NOT_OK);
}


PNIO_BOOL PnpbAlarmSubPlug (PNIO_UINT32          Ind,            // entity index in real_cfg
                            PNIO_UINT32          api,            // api number
                            PNIO_UINT32          slot_nr,        // slot number
                            PNIO_UINT32          subslot_nr,     // subslot number
                            PNIO_UINT32          modul_ident,    // module ID
                            PNIO_UINT32          submodul_ident, // submodule ID
                            PNIO_IM0_SUPP_ENUM   Im0Support,     // IM0 filter bits
                            PNIO_BOOL            more_follows)   // more follows PNIO_TRUE/PNIO_FALSE
{
    PNDV_REAL_CFG_T* iface_real_cfg_ptr;
    PNIO_UINT8       cmd = 0;
    PNIO_BOOL        Status = PNIO_OK;

    iface_real_cfg_ptr = &pPnpbIf->real_cfg[Ind];

    if (iface_real_cfg_ptr->elem.state == PNDV_IFACE_SERVICE_IDLE)
    {
        iface_real_cfg_ptr->elem.state = PNDV_IFACE_SERVICE_NEW;


        iface_real_cfg_ptr->elem.sub_descr.api = api;
        iface_real_cfg_ptr->elem.sub_descr.slot_nr = (PNIO_UINT16)slot_nr;
        iface_real_cfg_ptr->elem.sub_descr.subslot_nr = (PNIO_UINT16)subslot_nr;
        iface_real_cfg_ptr->elem.sub_descr.mod_ident = modul_ident;
        iface_real_cfg_ptr->elem.sub_descr.sub_ident = submodul_ident;
        iface_real_cfg_ptr->elem.sub_descr.im0_bits = Im0Support;
        iface_real_cfg_ptr->elem.sub_descr.response = 0;

        iface_real_cfg_ptr->dial.state = PNDV_IFACE_SERVICE_IDLE;
        iface_real_cfg_ptr->xdial.state = PNDV_IFACE_SERVICE_IDLE;
        iface_real_cfg_ptr->pral.state = PNDV_IFACE_SERVICE_IDLE;
        iface_real_cfg_ptr->stal.state = PNDV_IFACE_SERVICE_IDLE;
        iface_real_cfg_ptr->upal.state = PNDV_IFACE_SERVICE_IDLE;
        iface_real_cfg_ptr->ural.state = PNDV_IFACE_SERVICE_IDLE;
        iface_real_cfg_ptr->ros.state = PNDV_IFACE_SERVICE_IDLE;

        if (PNPB_ENABLE_PROFIENERGY(slot_nr, subslot_nr, modul_ident, submodul_ident))
        {
            iface_real_cfg_ptr->elem.sub_descr.im0_bits |= 0x80; /* enable ProfiEnergy with CM_SV_IM0_BITS_PROFIENERGY */
        }

#if PNPB_ENABLE_IM5
        if ( slot_nr == 0 ) 
        {
        	iface_real_cfg_ptr->elem.sub_descr.im0_bits |= CM_SV_IM0_BITS_IM5; /* enable device representative I&M5 for device */
        }
        else
        {
            if (Im0Support != PNIO_IM0_SUBMODULE)
            {
                iface_real_cfg_ptr->elem.sub_descr.im0_bits |= CM_SV_IM0_BITS_IM5; /* enable device representative I&M5 for slot without IM0 data */
            }
        }
#endif // PNPB_ENABLE_IM5

        /*** pnpb_cfg_cmp_soll_ist wrote the decision into rema_slot_options whether the slot should be services or not */
        if(more_follows == PNIO_FALSE)
        {
            cmd = PNDV_EV_TO_PNDV_PLUG_AL;
            PNPB_EXIT(OS_MUTEX_PNPB);                // free mutex temporarily during wainting on semaphore SEM_PNPB_PLUG, to avoid deadlock
            PnpbReqSync  (cmd,                       // command
                          0,                         // add value 1
                          (PNIO_UINT16)Ind,          // add value 2
                          Pnpb.SemId[SEM_PNPB_PLUG],
                          NULL);                     // not used yet
            PNPB_ENTER(OS_MUTEX_PNPB);               // reoccupy mutex after semaphore SEM_PNPB_PLUG has been set

            // *** check return value from PNPB ***
            if (iface_real_cfg_ptr->elem.sub_descr.response != 0)
                Status = PNIO_NOT_OK;
        }
        else
        {
            cmd = PNDV_EV_TO_PNDV_PLUG_AL_MORE_FOLLOWS;
            pnpb_write_event_to_pndv
                          (cmd,                     // command
                           0,                       // add value 1
                           (PNIO_UINT16)Ind,        // add value 2
                           NULL  );                 // not used yet
         }
    }
    else
    {
        Status = PNIO_NOT_OK;
        PNPB_SYS_TRACE_01(LSA_TRACE_LEVEL_ERROR,  "error elemState = %d\n", iface_real_cfg_ptr->elem.state);
    }

    return (Status);
}



PNIO_UINT32 PnpbAlarmSubPull (PNIO_UINT32   Api,      // api number
                              PNIO_UINT32   Slot,     // slot number
                              PNIO_UINT32   Sub,      // subslot number
                              PNIO_BOOL     more_follows) // more follows PNIO_TRUE/PNIO_FALSE
{
    PNIO_UINT8       cmd;
    PNIO_UINT32      Ind;            // entity index in real_cfg
    PNIO_BOOL        Status = PNIO_OK;

    Ind = PnpbRealCfgGetEntityInd   (Api,     // api number
                                     Slot,    // slot number
                                     Sub);

    if (Ind == PNPB_INVALID_INDEX)
    {
        return (PNIO_FALSE);
    }

    if ( pPnpbIf->real_cfg[Ind].elem.state == PNDV_IFACE_SERVICE_IDLE )
    {
        pPnpbIf->real_cfg[Ind].elem.state = PNDV_IFACE_SERVICE_NEW;
        /*** pnpb_cfg_cmp_soll_ist wrote the decision into rema_slot_options whether the slot should be services or not */
        if(more_follows == PNIO_FALSE)
        {
            cmd = PNDV_EV_TO_PNDV_PULL_AL;
        }
        else
        {
            cmd = PNDV_EV_TO_PNDV_PULL_AL_MORE_FOLLOWS;
        }

        // *** trigger PNDV to serve this request ***
        {
            PNPB_EXIT(OS_MUTEX_PNPB);                  // free mutex temporarily during wainting on semaphore SEM_PNPB_PLUG, to avoid deadlock
            PnpbReqSync (cmd,                          // command
                         0,                            // add value 1
                         (PNIO_UINT16)Ind,             // add value 2
                         Pnpb.SemId[SEM_PNPB_PLUG],
                         NULL);                        // not used here
            PNPB_ENTER(OS_MUTEX_PNPB);                 // reoccupy mutex after semaphore SEM_PNPB_PLUG has been set
        }
    }
    else
    {
        Status = PNIO_NOT_OK;
        PNPB_SYS_TRACE_01(LSA_TRACE_LEVEL_ERROR,  "error elemState = %d\n", pPnpbIf->real_cfg[Ind].elem.state);
    }

    return (Status);
}


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
