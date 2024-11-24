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
/*  F i l e               &F: pnpb_sub_exp.c                            :F&  */
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
#include "iodapi_event.h"

#define LTRC_ACT_MODUL_ID   210
#define PNPB_MODULE_ID      210

// *-----------------------------------------------------------------
// * defines
// *-----------------------------------------------------------------

// *-----------------------------------------------------------------
// * external functions
// *-----------------------------------------------------------------

// *-----------------------------------------------------------------
// * external variables
// *-----------------------------------------------------------------

// *-----------------------------------------------------------------
// * static variables
// *-----------------------------------------------------------------


// *-----------------------------------------------------------------
// * static functions
// *-----------------------------------------------------------------


// *-----------------------------------------------------------------
// *  pnpb_sub_exp_init
// *  initialize PnpbExp data
// *
// *  input: none
// *
// *  output: none
// *
// *-----------------------------------------------------------------
PNIO_VOID pnpb_sub_exp_init()
{
	PNPB_MEMSET (&PnpbExp[0], 0, sizeof (PnpbExp));
}


// *-----------------------------------------------------------------
// *  pnpb_sub_exp_oneAr_getp
// *  gets the PNPB_EXP_SUB reference for a corresponding AR, api,
// *  slot, subslot
// *
// *  input:    ArNum       number of the AR  (1...IOD_CFG_NUMOF_AR)
// *  input:    ApiNum      number of the API
// *            SlotNum     number of the slot
// *            SubNum      number of the subslot
// *  output:   return      pSubslot or NULL (not found)
// *
// *-----------------------------------------------------------------
PNPB_CODE_FAST PNIO_EXP_SUB*   pnpb_sub_exp_oneAr_getp (PNIO_UINT32 ArInd,
										 PNIO_UINT32 ApiNum,
										 PNIO_UINT32 SlotNum,
										 PNIO_UINT32 SubNum)
{
	PNIO_UINT32 i;

	// *----------------------------------------------------------
	// * check if AR is valid
	// *----------------------------------------------------------
	if (PnpbExp[ArInd].ArValid == PNIO_FALSE)
	{
		return (NULL);
	}

	// *----------------------------------------------------------
	// * check, if already plugged
	// *----------------------------------------------------------
	for (i = 0; i < PnpbExp[ArInd].NumOfPluggedSub; i++)
	{
		if (   (PnpbExp[ArInd].Sub[i].EntityState == ELEM_OCCUPIED)
			&& (PnpbExp[ArInd].Sub[i].ApiNum  == ApiNum)
			&& (PnpbExp[ArInd].Sub[i].SlotNum == SlotNum)
			&& (PnpbExp[ArInd].Sub[i].SubNum  == SubNum))
		{ // subslot found
			return (&PnpbExp[ArInd].Sub[i]);
		}
	}
	return (NULL);  // subslot not found
}


// *-----------------------------------------------------------------
// *  pnpb_sub_exp_allAr_getp
// *  gets the PNPB_REAL_SUB reference for a corresponding api, slot, subslot.
// *  searches in all valid AR's
// *
// *  input:    pArInd      [out] returns the appropriate AR index (0..N-1)
// *            ApiNum      [in]  number of the API
// *            SlotNum     [in]  number of the slot
// *            SubNum      [in]  number of the subslot
// *  output:   return      pSubslot or NULL (not found)
// *
// *-----------------------------------------------------------------
PNPB_CODE_FAST PNIO_EXP_SUB*   pnpb_sub_exp_allAr_getp
							(  PNIO_UINT32*   pArInd,  // [out]
							   PNIO_UINT32    ApiNum,  // [in]
							   PNIO_UINT32    SlotNum, // [in]
							   PNIO_UINT32    SubNum)  // [in]
{
	PNIO_UINT32 ArInd;
	PNIO_EXP_SUB* pSubExp = NULL;

	// *--------------------------------------------------------------
	// * check, if submodule is included in in expected configuration
	// *--------------------------------------------------------------
	for (ArInd = 0; ArInd < IOD_CFG_NUMOF_AR; ArInd++)
	{
		pSubExp = pnpb_sub_exp_oneAr_getp ( ArInd, ApiNum, SlotNum, SubNum);
		if (pSubExp)
		{ // ** subslot found **
			*pArInd = ArInd;
			return (pSubExp);
		}
	}
	return (NULL);  // subslot not found
}


// *----------------------------------------------------------------------------*
// *  pnpb_sub_exp_add ()
// *
// *  adds a submodule to the expected configuration.
// *
// *  input:    ArInd            AR index (0...N-1)
// *            EntityIndSetCfg  index of subslot in perif interface
// *            pElem            submodule properties (incl. Ownership-Indication)
// *            flags            flags, ownership-flag is evaluated only
// *            **ppSubExp       address of pointer to expected submodule object
// *            moreFollows      more pnpb_sub_exp_add() calls will follow for this AR
// *
// *  output    return           PNIO_OK, PNIO_NOT_OK
// *                             *ppSubExp
// *
// *----------------------------------------------------------------------------*
PNIO_UINT32 pnpb_sub_exp_add(PNIO_UINT32 ArInd,                // [in]
                             PNIO_UINT32 EntityIndSetCfg,      // [in]
                             PNDV_SET_CFG_ELEMENT_T*  pElem,   // [in]
                             PNIO_UINT32 Flags,                // [in]
                             PNIO_EXP_SUB** ppSubExp,          // [in,out]
                             PNIO_BOOL  MoreFollows)           // [in]
{
	PNIO_UINT32     i;
	PNIO_UINT16     ArNum      = (PNIO_UINT16) (ArInd + 1);
	PNIO_UINT32     SubIndex   = PNPB_INVALID_INDEX;
	PNPB_REAL_SUB*  pSubReal   = NULL;
	PNIO_EXP_SUB*   pSubExp    = NULL;

	CM_SV_AR_OWNERSHIP_ELEMENT_TYPE* pOwn = &(pElem->own_info);

	PNPB_ASSERT(ArNum != 0);
	LSA_UNUSED_ARG (MoreFollows);

	PNPB_API_TRACE_04(LSA_TRACE_LEVEL_NOTE,  "Add-Exp Api%d AR%d Sl%d Sub%d",
					  ArNum,
					  pOwn->api,
					  pOwn->slot_nr,
					  pOwn->subslot_nr);

	PNPB_ENTER(OS_MUTEX_PNPB);
	PNPB_ENTER(OS_MUTEX_PNPB_IO);

	// *------------------------------------------------------------------
	// * find index. If subslot already in expConfig, take that
	// * index (overwrite old entry). If subslot not in expConfig,
	// * take the first free index (new entry)
	// *------------------------------------------------------------------
	for (i = 0; i < IOD_CFG_MAX_NUMOF_SUBSLOTS; i++)
	{
		pSubExp = &(PnpbExp[ArInd].Sub[i]);
		if (   (pSubExp->ApiNum  == pOwn->api)
			&& (pSubExp->SlotNum == pOwn->slot_nr)
			&& (pSubExp->SubNum  == pOwn->subslot_nr))
		{
			SubIndex = i;
			PnpbExp[ArInd].NumOfPluggedSub--; // decrement temporarily (because it is incremented later again )
			break;              // finish loop, submodule found in expConfig (plugging after application ready)
		}
		else
		{ // not found
			if ((pSubExp->EntityState == ELEM_FREE) && (SubIndex == PNPB_INVALID_INDEX))
			{
				SubIndex = i;   // take the first free index for later use
			}
			if ((i >= PnpbExp[ArInd].NumOfPluggedSub)&& (SubIndex != PNPB_INVALID_INDEX))
			{
				break;   // finish loop, submodule not yet plugged,  but free index available
			}
		}
	}

	// *----------------------------------------------------------
	// * no free slot and not in expConfig--> fatal error
	// *----------------------------------------------------------
	if (SubIndex == PNPB_INVALID_INDEX)
	{
		// subslot is already occupied, error
		 PnpbSetLastError (PNIO_ERR_IOD_MAXNUM_SUBSLOTS);
		 PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_ERROR,  "subslot already occupied");
		 PNPB_EXIT(OS_MUTEX_PNPB_IO);
		 PNPB_EXIT(OS_MUTEX_PNPB);
		 return (PNIO_NOT_OK);
	}

	// *----------------------------------------------------------
	// * fill entry
	// *----------------------------------------------------------
	pSubExp = &(PnpbExp[ArInd].Sub[SubIndex]);
	pSubExp->EntityState = ELEM_OCCUPIED;

	// *----------------------------------------------------------
	// * add submodule to PERI interface
	// *----------------------------------------------------------
	// *** submodule related properties ***
	pSubExp->ApiNum        = pOwn->api;
	pSubExp->ModIdent      = pOwn->mod_ident;
	pSubExp->ModProp       = pOwn->mod_properties;
	pSubExp->SlotNum       = pOwn->slot_nr;
	pSubExp->SubNum        = pOwn->subslot_nr;
	pSubExp->SubIdent      = pOwn->sub_ident;
	pSubExp->SubProp       = pOwn->sub_properties;
	pSubExp->EntityIndSetCfg = EntityIndSetCfg;

	// *** input data related  properties ***
	pSubExp->In.data_length   = pOwn->descr[0].data_length;
	pSubExp->In.data_offset   = pOwn->descr[0].data_offset;
	pSubExp->In.iocs_offset   = pOwn->descr[0].iocs_offset;
	pSubExp->In.iops_offset   = pOwn->descr[0].data_offset + pOwn->descr[0].data_length;

	// *** output data related  properties ***
	pSubExp->Out.data_offset   = pOwn->descr[1].data_offset;
	pSubExp->Out.data_length   = pOwn->descr[1].data_length;
	pSubExp->Out.iocs_offset   = pOwn->descr[1].iocs_offset;
	pSubExp->Out.iops_offset   = pOwn->descr[1].data_offset + pOwn->descr[1].data_length;
	pSubExp->IsWrongSubmod     = pOwn->is_wrong;


    if (Flags & PNDV_SET_CFG_FLAGS_OWN)
    {  // *** AR is not owner of the submodule ***
        pSubExp->OwnSessionKey = pOwn->owner_session_key;
    }
    else
    {
        pSubExp->OwnSessionKey = 0;
    }


	// *** evaluate, if subslot is occupied (submodule is plugged) ***
	pSubReal = pnpb_sub_real_getp (pOwn->api, pOwn->slot_nr, pOwn->subslot_nr);

	if (pSubReal)
	{
		pSubExp->isPlugged   = PNIO_TRUE;
		pSubExp->In.iops_val = pSubReal->InIops; // take iops for input data from real config
	}
	else
	{
		pSubExp->isPlugged      = PNIO_FALSE;
		pSubExp->In.iops_val    = PNIO_S_BAD;
		// PNDV is currently only checking plugged submodules for correct module/submodule ident ...
		pSubExp->IsWrongSubmod = PNIO_TRUE;
	}

	// *** set iops, iocs preset values  ***
	pSubExp->In.iocs_val   = PNIO_S_BAD;  // preset value is BAD
	pSubExp->Out.iops_val  = PNIO_S_BAD;  // preset value is BAD
	pSubExp->Out.iocs_val  = PNIO_S_BAD;  // preset value is BAD

	// *** consider submodule in input- update list yes/no ***
	pSubExp->IoProp = PNIO_SUB_PROP_NO_DATA;  // preset value

	if (pSubExp->In.data_length)
	{
			pSubExp->IoProp = PNIO_SUB_PROP_IN;
	}
	if (pSubExp->Out.data_length)
	{
			pSubExp->IoProp |= PNIO_SUB_PROP_OUT;
	}

	// *** increment number of plugged submodules ***
	PnpbExp[ArInd].NumOfPluggedSub++;

	// *** evaluate IOCR len as maximum of all IOPS/IOCS offset values ***
	if (    (pSubExp->In.data_length)
		 || ((pSubExp->In.data_length == 0) && (pSubExp->Out.data_length == 0)))     // input submodule or no io data submodule
	{
		if (pSubExp->In.iops_offset >= PnpbExp[ArInd].IocrLen)
			PnpbExp[ArInd].IocrLen = pSubExp->In.iops_offset + 1;
	}
	if (pSubExp->Out.data_length)   // output module
	{
		if (pSubExp->Out.iocs_offset >= PnpbExp[ArInd].IocrLen)
			PnpbExp[ArInd].IocrLen = pSubExp->Out.iocs_offset + 1;
	}

	// **** if last subslot, notify the application and set AR to valid***
	if (MoreFollows == PNIO_FALSE)
	{
		// * ---------------------------------------------------------
		// *   set state ArValid, i.e. list of expected submodules in
		// *   ownership indication has been evaluated and can be used.
		// * ---------------------------------------------------------
		PnpbExp[ArInd].ArValid = PNIO_TRUE;

		// * ---------------------------------------------------------
		// *   last ownership-element, now call PNIO_cbf_ar_ownership_ind()
		// * ---------------------------------------------------------
		PNIO_cbf_ar_ownership_ind (PNIO_SINGLE_DEVICE_HNDL,
								   ArInd + 1,               // AR number
								   &(PnpbExp[ArInd]));      // ownership properties of one submodule
	}

	// * ------------------------------------------------------------
	// *  free semaphores
	// * ------------------------------------------------------------
	PNPB_EXIT(OS_MUTEX_PNPB_IO);
	PNPB_EXIT(OS_MUTEX_PNPB);

	// **** return current element pointer to caller for later use ***
	*ppSubExp = pSubExp;
	return (PNIO_OK);
 }


// *----------------------------------------------------------------------------*
// *  pnpb_remove_all_sub  ()
// *
// *  removes all submodules of one AR from the expected configuration.
// *
// *  input:    ArNum            AR number (1...N)
// *  output    return           PNIO_OK, PNIO_NOT_OK
// *
// *----------------------------------------------------------------------------*
PNIO_BOOL pnpb_remove_all_sub  (PNIO_UINT32 ArInd)
{
	PNIO_UINT32 ArNum = ArInd + 1;

	PNPB_ASSERT(ArNum != 0);


	PNPB_API_TRACE_01(LSA_TRACE_LEVEL_NOTE,  "pnpb_sub_exp_add AR%d",
					  ArNum);

	PNPB_ENTER(OS_MUTEX_PNPB);
	PNPB_ENTER(OS_MUTEX_PNPB_IO);

    PNPB_MEMSET (&PnpbExp[ArInd], 0, sizeof (PNIO_EXP) );

	PnpbExp[ArInd].FirstParamEndValid   = PNIO_FALSE;		// reset ParamEnd flag
#ifdef PNPB_SYSTEM_REDUNDANCY
	if( PNIO_AR_TYPE_SINGLE_SYSRED == pnpb_ArType[ ArInd ] )
	{
	    /*do not reset in data flag if other ar is in data*/
		if(PNIO_FALSE == PnpbExp[PNPB_AR_OTHER(ArInd)].Rdy4InpUpdateValid)
		{
			PnpbExp[ArInd].Rdy4InpUpdateValid   = PNIO_FALSE;    	// reset InData flag
		}
		else if(PNIO_TRUE == PnpbExp[PNPB_AR_OTHER(ArInd)].Rdy4InpUpdateValid)
	    {
	        PnpbExp[ArInd].Rdy4InpUpdateValid = PNIO_TRUE;
	    }
	}

    PnpbExp[ArInd].ArType = pnpb_ArType[ ArInd ];
#else
	PnpbExp[ArInd].Rdy4InpUpdateValid   = PNIO_FALSE;    	// reset InData flag
#endif
	PnpbExp[ArInd].ArValid          = PNIO_FALSE;    	// AR is valid  (ownership indication has been processed or is under work)


	PNPB_EXIT(OS_MUTEX_PNPB_IO);
	PNPB_EXIT(OS_MUTEX_PNPB);

	return (PNIO_TRUE);
}


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
