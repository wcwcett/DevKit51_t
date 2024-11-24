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
/*  F i l e               &F: pnpb_sub_real.c                           :F&  */
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
#include "version_dk.h"
#include "nv_data.h"

#define LTRC_ACT_MODUL_ID   211
#define PNPB_MODULE_ID      211


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
PNPB_REAL PnpbMod;

// *-----------------------------------------------------------------
// * static functions
// *-----------------------------------------------------------------


// *-----------------------------------------------------------------
// *  pnpb_sub_real_init
// *  initialize module data
// *
// *  input: none
// *
// *  output: none
// *
// *-----------------------------------------------------------------
PNIO_VOID pnpb_sub_real_init(PNIO_VOID)
{
	PNPB_MEMSET(&PnpbMod, 0, sizeof (PnpbMod));
}


// *-----------------------------------------------------------------
// *  pnpb_sub_real_getp
// *  gets the PNPB_REAL_SUB reference for a corresponding api, slot, subslot
// *
// *  input:    ApiNum      number of the API
// *            SlotNum     number of the slot
// *            SubslotNum  number of the subslot
// *  output:   return      pSubslot or NULL (not found)
// *
// *-----------------------------------------------------------------
PNPB_REAL_SUB*   pnpb_sub_real_getp (PNIO_UINT32 ApiNum,
									 PNIO_UINT32 SlotNum,
									 PNIO_UINT32 SubNum)
{
	PNIO_UINT32 i;

	// *----------------------------------------------------------
	// * check, if already plugged
	// *----------------------------------------------------------
	for (i = 0; i < IOD_CFG_MAX_NUMOF_SUBSLOTS; i++)
	{
		 if (  (PnpbMod.Sub[i].PlugState == PNPB_PLUGGED)
			 && (PnpbMod.Sub[i].SlotNum == SlotNum)
			 && (PnpbMod.Sub[i].SubNum  == SubNum)
			 && (PnpbMod.Sub[i].ApiNum  == ApiNum))
		 { // subslot found
			return (&PnpbMod.Sub[i]);
		 }
	}
	return (NULL);  // subslot not found
}


// *-----------------------------------------------------------------
// *  pnpb_sub_real_entity
// *  gets the list index in real_cfg for a corresponding api, slot, subslot
// *  Optimized search, when number of plugged slots is reached, the search
// *  is stopped (--> use not while a plug to PNDV is running, because it
// *  temporary increases/decreases numOfSubslots, when a plug fails in PNDV)
// *
// *  input:    ApiNum      number of the API
// *            SlotNum     number of the slot
// *            SubslotNum  number of the subslot
// *  output:   return      Index in PnpbMod.Sub[Index] or (-1) (not found)
// *
// *-----------------------------------------------------------------
PNIO_UINT32   pnpb_sub_real_entity  (PNIO_UINT32 ApiNum,
									 PNIO_UINT32 SlotNum,
									 PNIO_UINT32 SubNum,
									 LSA_UINT32  ModIdent)
{
	PNIO_UINT32 i;

	// *----------------------------------------------------------
	// * check, if already plugged
	// *----------------------------------------------------------
	for (i = 0; i < IOD_CFG_MAX_NUMOF_SUBSLOTS; i++)
	{
		if (PnpbMod.Sub[i].PlugState == PNPB_PLUGGED)
		{
			if (   (PnpbMod.Sub[i].ApiNum  == ApiNum)
				&& (PnpbMod.Sub[i].SlotNum == SlotNum))
			{ //slot found
			    if ((PnpbMod.Sub[i].SubNum  == SubNum)
			        || PnpbMod.Sub[i].ModIdent != ModIdent)
			    {//same subslot or ModId
			        return (i);
			    }
			}
		}
	}
	return (PNPB_INVALID_INDEX);  // subslot not found
}


// *----------------------------------------------------------------------------*
// *  pnpb_sub_real_plug  ()
// *
// *  plug a submodule into a subslot.
// *
// *  input:    DevHndl          device handle
// *            Api              Application process identifier
// *            *pAddr           geographical address (slotnumber, subslotnumber)
// *            ModIdent         Module identifier    (see GSD file)
// *            SubIdent         Submodule identifier (see GSD file)
// *            InputDataLen     Submodule input data length
// *            OutputDataLen    Submodule output data length
// *            Im0Support       PNIO_TRUE: submodule has own I&M0 data, PNIO_FALSE: else
// *  output    return           PNIO_OK, PNIO_NOT_OK
// *
// *
// *----------------------------------------------------------------------------*
PNIO_UINT32 pnpb_sub_real_plug
			   (PNIO_UINT32		    DevHndl,
				PNIO_UINT32         ApiNum,
				PNIO_DEV_ADDR	    *pAddr,
				LSA_UINT32		    ModIdent,
				LSA_UINT32		    SubIdent,
                PNIO_UINT32         InputDataLen,
                PNIO_UINT32         OutputDataLen,
				PNIO_IM0_SUPP_ENUM  Im0Support,
				IM0_DATA            *pIm0Dat,
				PNIO_UINT8          IopsIniVal,     // initial value for iops-input, ONLY FOR SUBMOD WITHOUT IO DATA (e.g. PDEV)
				PNIO_UINT32         *pIndex,        // [out] pointer to index in PnpbMod
				PNIO_BOOL           MoreFollows)    // more plug requests follow...
{
	PNIO_UINT32   i;
	PNIO_BOOL     Status = PNIO_OK;
	LSA_UNUSED_ARG (DevHndl);

	#if (IOD_INCLUDE_IM0_4 == 0)      // only if IM is not handled inside the PN stack..
		LSA_UNUSED_ARG (pIm0Dat);
	#endif

	// *----------------------------------------------------------
	// * check, if already plugged
	// *----------------------------------------------------------
	if (pnpb_sub_real_entity (ApiNum, pAddr->Geo.Slot, pAddr->Geo.Subslot, ModIdent) != PNPB_INVALID_INDEX)
	{ // subslot is already occupied, error
		PnpbSetLastError (PNIO_ERR_IOD_SLOT_OCCUPIED);
		PNPB_API_TRACE_03(LSA_TRACE_LEVEL_ERROR,  "subslot Api%d Sl%d Sub%d already plugged",
						  ApiNum, pAddr->Geo.Slot, pAddr->Geo.Subslot );
		return (PNIO_NOT_OK);
	}

	// *----------------------------------------------------------
	// * find free entry
	// *----------------------------------------------------------
	if (pIndex)
	{
		*pIndex = PNPB_INVALID_INDEX;   // preset value
	}
	for (i = 0; i < IOD_CFG_MAX_NUMOF_SUBSLOTS; i++)
	{
		if (PnpbMod.Sub[i].PlugState == PNPB_NOT_PLUGGED)
		{
			// *----------------------------------------------------------
			// * add submodule to PERI interface
			// *----------------------------------------------------------
			PnpbMod.Sub[i].PlugState        = PNPB_PLUGGED;
			PnpbMod.Sub[i].PeriphRealCfgInd = PnpbRealCfgOccupy(i); // find free index in periph.real_cfg
			PnpbMod.Sub[i].ApiNum       = ApiNum;
			PnpbMod.Sub[i].SlotNum      = pAddr->Geo.Slot;
			PnpbMod.Sub[i].SubNum       = pAddr->Geo.Subslot;
			PnpbMod.Sub[i].ModIdent     = ModIdent;
			PnpbMod.Sub[i].SubIdent     = SubIdent;
			PnpbMod.Sub[i].SubmodState  = STOP;
			PnpbMod.Sub[i].InIops       = IopsIniVal;
			PnpbMod.Sub[i].Im0Support   = PNIO_IM0_NOTHING;           // preset: no IM0 support

            /* Inform user application about new module */
            PNIO_cbf_new_plug_ind(pAddr, InputDataLen, OutputDataLen);

			if (pIndex)
			{
				*pIndex = i;        // store index of PnpbMod.Sub[index]
			}

			#if IOD_INCLUDE_IM0_4
			if (pIm0Dat && Im0Support)
			{
				PnpbMod.Sub[i].Im0Support = Im0Support;
				pnpb_copy_Im0Data (&PnpbMod.Sub[i].Im0Dat,
								   pIm0Dat,
								   Im0Support);    // IM0 data
			}
			#endif

			// **** plug module into free index in periph.real_cfg ****
			Status = PnpbAlarmSubPlug ( PnpbMod.Sub[i].PeriphRealCfgInd, ApiNum, pAddr->Geo.Slot, pAddr->Geo.Subslot,
							  	  	    ModIdent, SubIdent, Im0Support, MoreFollows);

			if (Status != PNIO_OK)
			{ // **** remove submodule from PNPB, if it could not be plugged in PNDV
				PNPB_MEMSET(&PnpbMod.Sub[i], 0, sizeof (PnpbMod.Sub[0]));
			}

			// **** refresh plug-information for this submodule in expected configuration ***
			// *----------------------------------------------------------
			// * set submodule state in expConfig to "plugged"
			// *----------------------------------------------------------
            {
                PNIO_UINT32 ArInd;

                // *--------------------------------------------------------------
                // * check, if submodule is included in in expected configuration
                // *--------------------------------------------------------------
                for (ArInd = 0; ArInd < IOD_CFG_NUMOF_AR; ArInd++)
                {
                    // *----------------------------------------------------------
                    // * check if AR is valid
                    // *----------------------------------------------------------
                    if (PnpbExp[ArInd].ArValid == PNIO_FALSE)
                    {
                        continue;
                    }

                    // *----------------------------------------------------------
                    // * check, if already present
                    // *----------------------------------------------------------
                    for (i = 0; i < PnpbExp[ArInd].NumOfPluggedSub; i++)
                    {
                        if ((PnpbExp[ArInd].Sub[i].EntityState == ELEM_OCCUPIED)
                            && (PnpbExp[ArInd].Sub[i].ApiNum == ApiNum)
                            && (PnpbExp[ArInd].Sub[i].SlotNum == pAddr->Geo.Slot)
                            && (PnpbExp[ArInd].Sub[i].SubNum == pAddr->Geo.Subslot))
                        { // subslot found
                        	if(PnpbExp[ArInd].Sub[i].ModIdent == ModIdent)
                        	{
                        		PnpbExp[ArInd].Sub[i].IsWrongSubmod = PNIO_FALSE;
                        	}
                            PnpbExp[ArInd].Sub[i].isPlugged = PNIO_TRUE;
                        }
                    }
                }
            }

			return (Status);
		}
	}
	// subslot is already occupied, error
	PnpbSetLastError (PNIO_ERR_IOD_MAXNUM_SUBSLOTS);
	PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_ERROR,  "subslot already occupied");
	return (PNIO_NOT_OK);
}

// *----------------------------------------------------------------------------*
// *  pnpb_sub_real_plug  ()
// *
// *----------------------------------------------------------------------------*
//lint -e{832, 578} Parameter 'Symbol' not explicitly declared, int assumed
PNIO_UINT32   pnpb_sub_real_plug_list
						 (PNIO_SUB_LIST_ENTRY   *pIoSubList,           // plugged submodules, including PDEV
						  PNIO_UINT32           NumOfSubListEntries,   // number of entries in pIoSubList
						  PNIO_IM0_LIST_ENTRY   *pIm0List,             // list of IM0 data sets
						  PNIO_UINT32           NumOfIm0ListEntries,   // number of entries in pIm0List
						  PNIO_UINT32*          pStatusList)           // list of return-Stati[NumOfSublistEntries]
{
	PNIO_UINT32   Status = PNIO_OK;
	PNIO_DEV_ADDR Addr;     // location (module/submodule)
	PNIO_UINT32   i, j;
	PNIO_UINT32   NumOfDeviceProxies = 0;       // must be 1 at least...
	PNIO_UINT32*  pIndexList;

	if (pStatusList == NULL)
	{
		PNPB_API_TRACE_00 (LSA_TRACE_LEVEL_FATAL, "parameter pStatusList must be <> 0\n");
		return (PNIO_NOT_OK);
	}

    // **** alloc memory for indexList and fill with values "INVALID_INDEX" ****
    PNPB_ALLOC_MEM((PNIO_VOID**) &pIndexList,                     // address pointer
                   0,                                             // preset value for the memory
                   NumOfSubListEntries * sizeof (PNIO_UINT32));   // number of bytes to alloc

	// **** preset values ****
	for (i = 0; i < NumOfSubListEntries; i++)
	{
		*(pIndexList  + i)  = PNPB_INVALID_INDEX;
		*(pStatusList + i)  = PNIO_OK;
	}

	// *=================================================================
	// *  plug all modules and submodules
	// *=================================================================
	Addr.Type = PNIO_ADDR_GEO;// must be PNIO_ADDR_GEO

	for (i = 0; i < NumOfSubListEntries; i++)
	{
        PNIO_IM0_SUPP_ENUM Im0Support;
        IM0_DATA           *pIm0 = NULL;

		Addr.Geo.Slot       = (pIoSubList+i)->Slot;
		Addr.Geo.Subslot    = (pIoSubList+i)->Subslot;

		// *----------------------------------------------------
		// *  Search for IM0 data in list of IM0-data
		// *----------------------------------------------------
		Im0Support = PNIO_IM0_NOTHING;           // preset: no IM0 support
        if (pIm0List)
        {
            for (j = 0; j < NumOfIm0ListEntries; j++)
            {
                if (((pIoSubList + i)->Api == (pIm0List + j)->Api)
                    && ((pIoSubList + i)->Slot == (pIm0List + j)->Slot)
                    && ((pIoSubList + i)->Subslot == (pIm0List + j)->Subslot)
                    )
                { // subslot found in Im0List
                    Im0Support = (pIoSubList + i)->Im0Support; // take value only if submodule is present in IM0 list
                    pIm0 = &((pIm0List + j)->Im0Dat);
                    break;                                   // entry found, end of loop
                }
            }
        }

		// *----------------------------------------------------
		// *  minimum the DAP Submodule must support IM0 data.
		// *----------------------------------------------------
		if (pIm0 && (Im0Support != PNIO_IM0_NOTHING))
		{
            if (Im0Support & PNIO_IM0_DEVICE)
			{   // is device proxy
				pnpb_save_proxy(&Addr);
				NumOfDeviceProxies++;       // increment counter of device proxies (must be 1 at least)
			}
		}

        if (Status == PNIO_OK)
		{
			PNIO_BOOL MoreFollows = (i != (NumOfSubListEntries - 1));

			Status = pnpb_sub_real_plug (PNIO_SINGLE_DEVICE_HNDL,
									(pIoSubList+i)->Api,
									&Addr,                              // location (slot, subslot)
									(pIoSubList+i)->ModId,              // we use the default number = 1..
									(pIoSubList+i)->SubId,              // we use the default number = 1..
                                    (pIoSubList + i)->InDatLen,         // Submodule input data length
                                    (pIoSubList + i)->OutDatLen,        // Submodule output data length
                                    Im0Support,                         // Submodule supports IM0
									pIm0,                               // pointer to im0 data set
									PNIO_S_GOOD,                        // initial iops value, only for submodules without io data
									pIndexList+i,                       // return error value, accessible after MoreFollows = PNIO_FALSE
									MoreFollows);                       // more submodules will be plugged now..

			// *** check for and save early errors (e.g. subslot already occupied) ***
			if (Status != PNIO_OK)
			{
				*(pStatusList+i) = Status;
			}
		}
	}   // end for (i = 0; i < NumOfSubListEntries, ...

	// *----------------------------------------------------
	// *  check, if exactly one subslot is set to
	// *  IM0-proxy for the device
	// *----------------------------------------------------
	if (NumOfDeviceProxies != 1)
	{
		Status = PNIO_NOT_OK;
		PNPB_API_TRACE_01 (LSA_TRACE_LEVEL_ERROR, "Number of device proxies (expected = 1) = %d\n", NumOfDeviceProxies);
	}

	// *----------------------------------------------------
	// *  set status to bad, if one ore more of the
	// *  entries in the status list is bad
	// *----------------------------------------------------
	for (i = 0; i < NumOfSubListEntries; i++)
	{
		if (*(pStatusList+i) == PNIO_OK)  // no "early" error found for this subslot
		{
			PNIO_UINT32 PnpbInd;
			PNIO_UINT32 TmpStat;

			// *** read response from PNPB (OK/NOT_OK) *****
			PnpbInd = *(pIndexList+i);
			if (PnpbInd != PNPB_INVALID_INDEX)
			{
				TmpStat = PnpbAlarmSubPlugGetState (PnpbMod.Sub[PnpbInd].PeriphRealCfgInd);

				if (TmpStat == PNIO_OK)
				{
					// *** increment number of (correctly) plugged submodules ***
					PnpbMod.NumOfPluggedSub++;;
					PnpbMod.Sub[PnpbInd].DataDirection = PNIO_SUB_PROP_NO_DATA;
					if(0 != (pIoSubList+i)->InDatLen)
					{
						PnpbMod.Sub[PnpbInd].DataDirection |= PNIO_SUB_PROP_IN;
					}
					if(0 != (pIoSubList+i)->OutDatLen)
					{
						PnpbMod.Sub[PnpbInd].DataDirection |= PNIO_SUB_PROP_OUT;
					}
				}
				else
				{
					PNPB_MEMSET(&PnpbMod.Sub[PnpbInd], 0, sizeof (PnpbMod.Sub[0]));
					PNPB_API_TRACE_03 (  LSA_TRACE_LEVEL_ERROR, "Api%d Sl%d Sub%d not plugged\n",
										 (pIoSubList+i)->Api,
										 (pIoSubList+i)->Slot,
										 (pIoSubList+i)->Subslot);

					// *** set specific and overall-status to NOT_OK ***
					*(pStatusList+i) = PNIO_NOT_OK;
					Status = PNIO_NOT_OK;
				}
			}
			else
			{
				// *** set specific and overall-status to NOT_OK ***
				*(pStatusList+i) = PNIO_NOT_OK;
				Status = PNIO_NOT_OK;
			}
		}
	}
	PNPB_FREE_MEM((PNIO_VOID*) pIndexList);
	return (Status);
}


// *----------------------------------------------------------------------------*
// *  pnpb_sub_real_pull  ()
// *
// *  pulls a submodule from a subslot. If all submodules of the module are pulled,
// *  the module is pulled automatically.
// *
// *  input:    DevHndl          device handle
// *            Api              Application process identifier
// *            *pAddr           geographical address (slotnumber, subslotnumber)
// *  output    return           PNIO_OK, PNIO_NOT_OK
// *
// *
// *----------------------------------------------------------------------------*
PNIO_UINT32 pnpb_sub_real_pull(	PNIO_UINT32   DevHndl,
								PNIO_UINT32   ApiNum,
								PNIO_DEV_ADDR *pAddr)
{
	PNIO_UINT32 Status = PNIO_NOT_OK;   // preselection
	PNIO_UINT32 i;
	PNIO_UINT32 Direction;
	PNIO_UINT32 PeriphRealCfgInd; 
#if IOD_INCLUDE_IM0_4
    PNIO_IM0_SUPP_ENUM Im0Support;
#endif
	LSA_UNUSED_ARG (DevHndl);

	// *----------------------------------------------------------
	// * find subslot
	// *----------------------------------------------------------
	for (i = 0; i < IOD_CFG_MAX_NUMOF_SUBSLOTS; i++)
	{
		if (   (PnpbMod.Sub[i].ApiNum  == ApiNum)
			&& (PnpbMod.Sub[i].SlotNum == pAddr->Geo.Slot)
			&& (PnpbMod.Sub[i].SubNum  == pAddr->Geo.Subslot))
		{ // *** subslot found ***

			if (PnpbMod.Sub[i].PlugState != PNPB_PLUGGED)
			{
				PnpbSetLastError (PNIO_ERR_IOD_NO_SUBMODULE);
				PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_ERROR,  "subslot not plugged");
				return (PNIO_NOT_OK);
			}

            /* Inform user application about module removing */
            PNIO_cbf_new_pull_ind(pAddr);

			/* Remember direction and idx for a while longer */
			Direction = PnpbMod.Sub[i].DataDirection;
			PeriphRealCfgInd = PnpbMod.Sub[i].PeriphRealCfgInd;
#if IOD_INCLUDE_IM0_4
            Im0Support = PnpbMod.Sub[i].Im0Support;
#endif

			// ** set all submodule properties to 0  **
			PNPB_MEMSET(&PnpbMod.Sub[i], 0, sizeof (PNPB_REAL_SUB));

			// *----------------------------------------------------------
			// * remove submodule from PERI interface
			// *----------------------------------------------------------
			Status = PnpbAlarmSubPull  (ApiNum, pAddr->Geo.Slot, pAddr->Geo.Subslot, PNIO_FALSE);   // more follows....

			PnpbMod.NumOfPluggedSub--;              // decrement number of plugged submodules

			/* Signalize to cyclical task, that there is new pull */
			PNPB_ENTER(OS_MUTEX_PNPB_PULL);
			/* Params for last IOxS update*/
			PullPending.DataDir = Direction;
			PullPending.Slot = pAddr->Geo.Slot;
			PullPending.Subslot = pAddr->Geo.Subslot;
            if (Direction & PNIO_SUB_PROP_IN)
            {
                PullPending.WritePending = PNIO_TRUE;
            }
            if (Direction & PNIO_SUB_PROP_OUT)
            {
                PullPending.ReadPending = PNIO_TRUE;
            }
			PNPB_EXIT(OS_MUTEX_PNPB_PULL);

			// *----------------------------------------------------------
			// * set submodule state in expConfig to "not plugged in realConfig
			// *----------------------------------------------------------
			{
                PNIO_UINT32 ArInd;

                // *--------------------------------------------------------------
                // * check, if submodule is included in in expected configuration
                // *--------------------------------------------------------------
                for (ArInd = 0; ArInd < IOD_CFG_NUMOF_AR; ArInd++)
                {
                    // *----------------------------------------------------------
                    // * check if AR is valid
                    // *----------------------------------------------------------
                    if (PnpbExp[ArInd].ArValid == PNIO_FALSE)
                    {
                        continue;
                    }

                    // *----------------------------------------------------------
                    // * check, if already plugged
                    // *----------------------------------------------------------
                    for (i = 0; i < PnpbExp[ArInd].NumOfPluggedSub; i++)
                    {
                        if ((PnpbExp[ArInd].Sub[i].EntityState == ELEM_OCCUPIED)
                            && (PnpbExp[ArInd].Sub[i].ApiNum == ApiNum)
                            && (PnpbExp[ArInd].Sub[i].SlotNum == pAddr->Geo.Slot)
                            && (PnpbExp[ArInd].Sub[i].SubNum == pAddr->Geo.Subslot))
                        { // subslot found
                            PnpbExp[ArInd].Sub[i].isPlugged = PNIO_FALSE;
                            PnpbExp[ArInd].Sub[i].In.iops_val = PNIO_S_BAD;
                            PnpbExp[ArInd].Sub[i].IsWrongSubmod = PNIO_TRUE;
                        }
                    }
                }
			}

#if IOD_INCLUDE_IM0_4
            if (Im0Support != PNIO_IM0_NOTHING)
            {
                PNIO_UINT8 *IM1_clr;
                PNIO_UINT8 *IM2_clr;
                PNIO_UINT8 *IM3_clr;
                PNIO_UINT8 *IM4_clr;

                // ***** alloc empty memory (and fill it with space) *****
                // ***** which will be stored to remanent memory *****
                PNPB_ALLOC_MEM((PNIO_VOID**) &IM1_clr, 0x20, sizeof(IM1_DATA));
                PNPB_ALLOC_MEM((PNIO_VOID**) &IM2_clr, 0x20, sizeof(IM2_DATA));
                PNPB_ALLOC_MEM((PNIO_VOID**) &IM3_clr, 0x20, sizeof(IM3_DATA));
                PNPB_ALLOC_MEM((PNIO_VOID**) &IM4_clr, 0x00, sizeof(IM4_DATA));

                // ***** delete I&M data stored in remanent memory *****
                Status = Bsp_im_data_store_no_pndv_trigger(
                    PNIO_NVDATA_IM1,	// nv data type: device name
                    IM1_clr,			// source pointer to the devicename
                    sizeof(IM1_DATA),	// length of IM data
                    PeriphRealCfgInd);
                if (Status == PNIO_OK)
                {
                    PNIO_printf("##DELETE IM1 Data, Slot=%d Subslot=%d Len=%d\n",
                        pAddr->Geo.Slot, pAddr->Geo.Subslot, sizeof(IM1_DATA));
                }

                Status = Bsp_im_data_store_no_pndv_trigger(
                    PNIO_NVDATA_IM2,	// nv data type: device name
                    IM2_clr,			// source pointer to the devicename
                    sizeof(IM2_DATA),	// length of IM data
                    PeriphRealCfgInd);
                if (Status == PNIO_OK)
                {
                    PNIO_printf("##DELETE IM2 Data, Slot=%d Subslot=%d Len=%d\n",
                        pAddr->Geo.Slot, pAddr->Geo.Subslot, sizeof(IM2_DATA));
                }

                Status = Bsp_im_data_store_no_pndv_trigger(
                    PNIO_NVDATA_IM3,	// nv data type: device name
                    IM3_clr,			// source pointer to the devicename
                    sizeof(IM3_DATA),	// length of IM data
                    PeriphRealCfgInd);
                if (Status == PNIO_OK)
                {
                    PNIO_printf("##DELETE IM3 Data, Slot=%d Subslot=%d Len=%d\n",
                        pAddr->Geo.Slot, pAddr->Geo.Subslot, sizeof(IM3_DATA));
                }

                Status = Bsp_im_data_store_no_pndv_trigger(
                    PNIO_NVDATA_IM4,	// nv data type: device name
                    IM4_clr,			// source pointer to the devicename
                    sizeof(IM4_DATA),	// length of IM data
                    PeriphRealCfgInd);
                if (Status == PNIO_OK)
                {
                    PNIO_printf("##DELETE IM4 Data, Slot=%d Subslot=%d Len=%d\n",
                        pAddr->Geo.Slot, pAddr->Geo.Subslot, sizeof(IM4_DATA));
                }

                PNPB_FREE_MEM(IM1_clr);
                PNPB_FREE_MEM(IM2_clr);
                PNPB_FREE_MEM(IM3_clr);
                PNPB_FREE_MEM(IM4_clr);
            }
#endif

			return (Status);
		}
	}

	PnpbSetLastError (PNIO_ERR_IOD_NO_SUBMODULE);
	PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_ERROR,  "subslot not found");

	return (PNIO_NOT_OK);
}


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
