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
/*  F i l e               &F: pnpb_io.c                                 :F&  */
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
#include "iod_cfg.h"
#if(1 == IOD_USED_WITH_XHIF_HOST)
#include "PnUsr_xhif.h"
#endif
#define LTRC_ACT_MODUL_ID   204
#define PNPB_MODULE_ID      204


extern PNPB_REAL PnpbMod;
// *-----------------------------------------------------------------
// * defines
// *-----------------------------------------------------------------
#if IOD_INCLUDE_REC8028_8029
	#define READ_IO_POLLING_TIME_MS        10      // polling time in msec

	typedef enum
	{
		IO_READ_STATE_IDLE = 0,
		IO_READ_STATE_EXEC,
		IO_READ_STATE_FINISHED
	} IO_READ_STATE;

	typedef struct
	{
		PNIO_UINT32              SrcIoDatOffs;   // offset of io data in IOCR source buffer
		PNIO_UINT32              SrcIopsOffs;    // offset of iops in IOCR source buffer
		PNIO_UINT32              SrcIocsOffs;    // offset of iocs in IOCR source buffer
		PNIO_UINT32              IoDatLen;       // length of the io data in expected configuration
		PNIO_UINT8*              pDstIoDat;      // IO-data destination buffer pointer
		PNIO_UINT8*              pDstIops;       // IOPS destination buffer pointer (length = 1 byte)
		PNIO_UINT8*              pDstIocs;       // IOCS destination buffer pointer (length = 1 byte)
		volatile IO_READ_STATE   ExecProvRq;     // execute request (PNIO_TRUE, PNIO_FALSE) in ProvLock()
		volatile IO_READ_STATE   ExecConsRq;     // execute request (PNIO_TRUE, PNIO_FALSE) in ConsLock()
	}  REC_IO_COPY_REQ;
#endif

#define   CLEAR_PADDING_BYTES_IN_IO     0       // 1: padding bytes in cyclic IO telegram is set to 0 first, 0: else


// *-----------------------------------------------------------------
// * external variables
// *-----------------------------------------------------------------
#ifdef PNPB_SYSTEM_REDUNDANCY
	extern PNPB_INSTANCE            Pnpb;
#endif
// *-----------------------------------------------------------------
// * static variables
// *-----------------------------------------------------------------
#if IOD_INCLUDE_REC8028_8029
	static REC_IO_COPY_REQ  Rec8028_Req[IOD_CFG_NUMOF_AR];       // read input data by record index 0x8028
	static REC_IO_COPY_REQ  Rec8029_Req[IOD_CFG_NUMOF_AR];       // read output data by record index 0x8029
#endif
// *-----------------------------------------------------------------
// * static functions
// *-----------------------------------------------------------------

PNPB_CODE_FAST PNIO_UINT32 PnpbConsLock(PNIO_UINT8** ppDatOut, PNIO_UINT32** ppApduStat, PNIO_UINT16 ArInd)
{
	LSA_UINT16 Status;
	*ppDatOut = NULL;

    // *** check if AR is alive and paramEnd has been processed, else return ***
	if (PnpbExp[ArInd].FirstParamEndValid == PNIO_FALSE)
    {
        return (PNIO_ERR_NOT_ACCESSIBLE);
    }

	Status = iom_consumer_lock( (PNIO_VOID**)ppDatOut,
								(iom_apdu_status_t**)ppApduStat,
								 ArInd);
	switch (Status)
	 {
		case    IOM_OK_NEW_BUFFER:
				Status = PNIO_OK;
				break;
		case    IOM_OK_OLD_BUFFER:
				Status = PNIO_OK;
				break;
		case    IOM_INVALID_BUFFER:
				PNPB_SYS_TRACE_02(LSA_TRACE_LEVEL_UNEXP,  "error %d in ConsLock(AR=%d)\n",
								  Status, ArInd);
				Status = PNIO_NOT_OK;
				break;
		case    IOM_BUFFER_LOCKED:
		default:
				PNPB_SYS_TRACE_02(LSA_TRACE_LEVEL_ERROR,  "error %d in ConsLock(AR=%d)\n",
						Status, ArInd);
				Status = PNIO_NOT_OK;
				break;
	 }
	 #if IOD_INCLUDE_REC8028_8029
		 if (Rec8029_Req[ArInd].ExecConsRq  == IO_READ_STATE_EXEC)
		 {
			 if (Status == PNIO_OK)
			 {
				 PNPB_COPY_BYTE(Rec8029_Req[ArInd].pDstIoDat,
							    *ppDatOut + Rec8029_Req[ArInd].SrcIoDatOffs,
							    Rec8029_Req[ArInd].IoDatLen);
				 *(Rec8029_Req[ArInd].pDstIops) = *(*ppDatOut + Rec8029_Req[ArInd].SrcIopsOffs);
			 }
			 Rec8029_Req[ArInd].ExecConsRq = IO_READ_STATE_FINISHED;
		 }

		 if (Rec8028_Req[ArInd].ExecConsRq  == IO_READ_STATE_EXEC)
		 {
			 if (Status == PNIO_OK)
			 {
				 *(Rec8028_Req[ArInd].pDstIocs) = *(*ppDatOut + Rec8028_Req[ArInd].SrcIocsOffs);
			 }
			 Rec8028_Req[ArInd].ExecConsRq = IO_READ_STATE_FINISHED;
		 }
	#endif

	 return (Status);
}


PNPB_CODE_FAST PNIO_UINT32 PnpbConsUnlock(PNIO_UINT16 ArInd)
{
	LSA_UINT16 Status;
	Status = iom_consumer_unlock (ArInd);

	switch (Status)
	{
		case IOM_OK:           // OK
			return (PNIO_OK);
		case IOM_INVALID_BUFFER:     // error, iocr is not initialized
			PNPB_SYS_TRACE_02(LSA_TRACE_LEVEL_UNEXP,  "error %d in ConsUnlock(AR=%d)\n",
					Status, ArInd);
			break;
		case IOM_BUFFER_UNLOCKED:   // error, buffer was not locked
		default:
			PNPB_SYS_TRACE_02(LSA_TRACE_LEVEL_ERROR,  "error %d in ConsUnlock(AR=%d)\n",
							  Status, ArInd);
	}
	return (PNIO_NOT_OK);
}


PNPB_CODE_FAST PNIO_UINT32 PnpbProvLock(PNIO_UINT8** ppDatIn, PNIO_UINT16 ArInd, PNIO_UINT16 dataOffset, PNIO_UINT16 dataLength)
{
    *ppDatIn = NULL;

    if (PnpbExp[ArInd].Rdy4InpUpdateValid == PNIO_FALSE)
    {
        return (PNIO_ERR_NOT_ACCESSIBLE);
    }

    iom_provider_lock((LSA_VOID **)ppDatIn, ArInd);
    if (*ppDatIn)
    {
        return (PNIO_OK);
    }
    else
    {
        PNPB_SYS_TRACE_01(LSA_TRACE_LEVEL_UNEXP, "error in ProvLock(AR=%d)\n", ArInd);
        return (PNIO_NOT_OK);
    }
}


PNIO_UINT32 PnpbProvUnlock(PNIO_UINT16 ArInd)
{
	PNIO_UINT16 Status = iom_provider_unlock (ArInd);

	switch (Status)
	{
	   case    IOM_OK_NEW_BUFFER:
	   case    IOM_OK_OLD_BUFFER:
			   return (PNIO_OK);
	   case    IOM_INVALID_BUFFER:
			   PNPB_SYS_TRACE_02(LSA_TRACE_LEVEL_UNEXP,  "error %d in ConsLock(AR=%d)\n",
								 Status, ArInd);
			   break;
	   case    IOM_BUFFER_LOCKED:
	   default:
			   PNPB_SYS_TRACE_02(LSA_TRACE_LEVEL_ERROR,  "error %d in ConsLock(AR=%d)\n",
								 Status, ArInd);
			   break;
	}
	return (PNIO_NOT_OK);
}


PNIO_UINT32 PnpbCopyRecReadIoData(PNIO_UINT8* pDatIn, PNIO_UINT16 ArInd, PNIO_UINT16 dataOffset, PNIO_UINT16 dataLength)
{
    // NOTE: must be called before provider unlock, to read valid input data from provider buffers
    if (pDatIn)
    {
#if IOD_INCLUDE_REC8028_8029
        if (Rec8028_Req[ArInd].ExecProvRq  == IO_READ_STATE_EXEC)
        {
            PNPB_COPY_BYTE(Rec8028_Req[ArInd].pDstIoDat,
                           pDatIn + Rec8028_Req[ArInd].SrcIoDatOffs,
                           Rec8028_Req[ArInd].IoDatLen);
            *(Rec8028_Req[ArInd].pDstIops) = *(pDatIn + Rec8028_Req[ArInd].SrcIopsOffs);
            Rec8028_Req[ArInd].ExecProvRq  = IO_READ_STATE_FINISHED;
        }
        if (Rec8029_Req[ArInd].ExecProvRq == IO_READ_STATE_EXEC)
        {
            *(Rec8029_Req[ArInd].pDstIocs) = *(pDatIn + Rec8029_Req[ArInd].SrcIocsOffs);
            Rec8029_Req[ArInd].ExecProvRq = IO_READ_STATE_FINISHED;
        }
#endif
        return (PNIO_OK);
    }
    else
    {
        PNPB_SYS_TRACE_01(LSA_TRACE_LEVEL_UNEXP, "error %d in PnpbCopyRecReadIoData(AR=%d)\n", ArInd);
        return (PNIO_NOT_OK);
    }
}

// *------------------------------------------------------------------------
// *		pnpb_get_last_apdu_status()
// *
// *		reads the last remote apdu status for the specified AR
// *------------------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32 pnpb_get_last_apdu_status  (PNIO_UINT32 ArNum)
{
	if (ArNum)
	{
		return (PnpbExp[ArNum-1].LastApduStat);
	}
	else
	{
		PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_ERROR,  "error: invalid ArNum = 0\n");
		return (0);
	}
}


// *------------------------------------------------------------------------
// *		pnpb_initiate_data_read()
// *
// *		initiates reading the RT and/or IRT output data from stack and writing them  to
// *        the physical output
// *------------------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32 pnpb_initiate_data_read  (PNIO_UINT32   DevHndl)
{
	PNIO_UINT32 *pTmpApduStat;
	PNIO_UINT16 ArInd;
	PNIO_UINT32 i;
	PNIO_UINT8* pData = NULL;

	OS_INSTRUMENT_USER_START(0xAA, 1234);

	LSA_UNUSED_ARG (DevHndl);


	PNPB_ENTER(OS_MUTEX_PNPB_IO);
	for (ArInd = 0; ArInd < IOD_CFG_NUMOF_AR; ArInd++)
	{
		if (PnpbExp[ArInd].FirstParamEndValid)
		{
			// **** get pointer to IOCR buffer and APDU status (including cycle counter and status bits ***
			PnpbConsLock(&pData, &pTmpApduStat, (ArInd));

			if (pData == NULL)
			{
				PNPB_EXIT(OS_MUTEX_PNPB_IO);
				OS_INSTRUMENT_USER_STOP(0xAA, 5678);
				return (PNIO_NOT_OK);
			}
			PNPB_COPY_BYTE(&PnpbExp[ArInd].LastApduStat, pTmpApduStat, 4);

#if(1 == IOD_USED_WITH_XHIF_HOST)
			/* Report apdu to XHIF HOST */
			PnUsr_xhif_cyclical_write_apdu(ArInd+1, PnpbExp[ArInd].LastApduStat);
#endif

			for (i = 0; i < PnpbExp[ArInd].NumOfPluggedSub; i++)
			{
				PNIO_EXP_SUB*  pExpSub = &(PnpbExp[ArInd].Sub[i]);
				PNIO_DEV_ADDR  Addr;
				Addr.Geo.Slot = pExpSub->SlotNum;
				Addr.Geo.Subslot = pExpSub->SubNum;
				if (pExpSub->OwnSessionKey)
				{ // AR is submodule owner
					if (pExpSub->IoProp & PNIO_SUB_PROP_OUT)
					{
						if (pExpSub->isPlugged)
						{
#if(1 == IOD_USED_WITH_XHIF_HOST)
							pExpSub->Out.iocs_val = PnUsr_xhif_IO_data_read(
									PNIO_SINGLE_DEVICE_HNDL, &Addr, pExpSub->Out.data_length,
									pData + pExpSub->Out.data_offset, *(pData + pExpSub->Out.iops_offset));
#else
							pExpSub->Out.iocs_val = PNIO_cbf_data_read
										(	PNIO_SINGLE_DEVICE_HNDL,
											&Addr,							        // location (slot, subslot)
											pExpSub->Out.data_length,			    // number of output bytes
											pData + pExpSub->Out.data_offset,       // data buffer pointer
											*(pData + pExpSub->Out.iops_offset));	// remote IOPS from IOC						    // iops (from the io controller)

#endif
						}
						else
						{
							pExpSub->In.iocs_val = PNIO_S_BAD;
						}
					}
					if (pExpSub->IoProp & PNIO_SUB_PROP_IN)
					{
#if(1 == IOD_USED_WITH_XHIF_HOST)
                        if (pExpSub->isPlugged)
                        {
                            /* Send only IOxS to UserApp */
                            PnUsr_xhif_IO_data_read(PNIO_SINGLE_DEVICE_HNDL, &Addr, 0,
                                NULL, *(pData + pExpSub->In.iocs_offset));
                        }
#endif

					    /* save remote consumer state */
						pExpSub->In.iocs_val = *(pData + pExpSub->In.iocs_offset);
					}
				}
				else
				{ // AR is not submodule owner
					if (pExpSub->IoProp & PNIO_SUB_PROP_IN)
					{ // save remote consumer state
						pExpSub->In.iocs_val = PNIO_S_BAD;
					}
				}
			}
			// **** free pointer to IOCR buffer ***
			PnpbConsUnlock(ArInd);
		}
	}

	PNPB_ENTER(OS_MUTEX_PNPB_PULL);
	/* The submodule was pulled and we have to update user, that data are no more valid */
	if(PNIO_TRUE == PullPending.ReadPending)
	{
		PNIO_DEV_ADDR  Addr;
		Addr.Geo.Slot = PullPending.Slot;
		Addr.Geo.Subslot = PullPending.Subslot;
		/* Module not ready to be updated -> PNIO_S_BAD */
        PNIO_cbf_data_read_IOxS_only(PNIO_SINGLE_DEVICE_HNDL, &Addr, PNIO_S_BAD);

		/* Done */
		PullPending.ReadPending = PNIO_FALSE;
	}
	PNPB_EXIT(OS_MUTEX_PNPB_PULL);

	PNPB_EXIT(OS_MUTEX_PNPB_IO);
	OS_INSTRUMENT_USER_STOP(0xAA, 5678);
	return (PNIO_OK);
}


// *------------------------------------------------------------------------
// *		pnpb_initiate_data_write_ext()
// *
// *		initiates reading the RT and/or IRT output data from stack and writing them  to
// *        the physical output
// *------------------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32 pnpb_initiate_data_write (PNIO_UINT32   DevHndl)
{
	PNIO_UINT8  *pData = NULL;
	PNIO_UINT16 ArInd;
	PNIO_UINT32 i;

	OS_INSTRUMENT_USER_START(0xBB, 5678);
	LSA_UNUSED_ARG (DevHndl);

	PNPB_ENTER(OS_MUTEX_PNPB_IO);
	for (ArInd = 0; ArInd < IOD_CFG_NUMOF_AR; ArInd++)
	{

		if (PnpbExp[ArInd].Rdy4InpUpdateValid)
		{
			// **** get pointer to IOCR buffer ***
			PnpbProvLock(&pData, ArInd, 0, 0xFFFF);
			if (NULL == pData)
			{
				PNPB_EXIT(OS_MUTEX_PNPB_IO);
				OS_INSTRUMENT_USER_STOP(0xBB, 5678);
				return (PNIO_NOT_OK);
			}

#if CLEAR_PADDING_BYTES_IN_IO
			// **** clear IOCR buffer first ***
			if (PnpbExp[ArInd].IocrLen < 44)
			    PNPB_MEMSET(pData + PnpbExp[ArInd].IocrLen, 0, 44 - PnpbExp[ArInd].IocrLen);
#endif

			// **** write submodule IO data into IOCR buffer ***
			for (i = 0; i < PnpbExp[ArInd].NumOfPluggedSub; i++)
			{
				PNIO_EXP_SUB*  pExpSub = &(PnpbExp[ArInd].Sub[i]);
				PNIO_DEV_ADDR  Addr;
				Addr.Geo.Slot = pExpSub->SlotNum;
				Addr.Geo.Subslot = pExpSub->SubNum;

				// *** provide io data and iops from input submodule ***
				if (pExpSub->OwnSessionKey && pExpSub->isPlugged)
				{
					if (pExpSub->IoProp == PNIO_SUB_PROP_NO_DATA)
					{   // ** submod has no IO data **
						// *** update (input-)provider state in input frame ***
						if ((!pExpSub->IsWrongSubmod) && pExpSub->ParamEndValid)
							*(pData + pExpSub->In.iops_offset) = pExpSub->In.iops_val;
						else
							*(pData + pExpSub->In.iops_offset) = PNIO_S_BAD;
					}
					else
					{
						if (pExpSub->IoProp & PNIO_SUB_PROP_IN)
						{  // ** submod has input data **
						   // *** update (input-)provider state in input frame ***
							if ((!pExpSub->IsWrongSubmod) && pExpSub->ParamEndValid)
							{
#if(1 == IOD_USED_WITH_XHIF_HOST)
                                pExpSub->In.iops_val = PnUsr_xhif_IO_data_write(
                                        PNIO_SINGLE_DEVICE_HNDL, &Addr, pExpSub->In.data_length,
							            pData + pExpSub->In.data_offset, pExpSub->In.iocs_val);
                                *(pData + pExpSub->In.iops_offset) = pExpSub->In.iops_val;
#else
								pExpSub->In.iops_val = PNIO_cbf_data_write
											(	PNIO_SINGLE_DEVICE_HNDL,
												&Addr,							         // location (slot, subslot)
												pExpSub->In.data_length,			     // number of output bytes
												pData + pExpSub->In.data_offset,         // data buffer pointer
												pExpSub->In.iocs_val);	                 // last remote consumer state from IOC	// iops (from the io controller)
								 *(pData + pExpSub->In.iops_offset) = pExpSub->In.iops_val;
#endif
							}
							else
							{
								*(pData + pExpSub->In.iops_offset) = PNIO_S_BAD;
							}
						}
						if (pExpSub->IoProp & PNIO_SUB_PROP_OUT)
						{  // ** submod has out put data **
							if ((!pExpSub->IsWrongSubmod) && pExpSub->ParamEndValid)
								*(pData + pExpSub->Out.iocs_offset) = pExpSub->Out.iocs_val;
							else
								*(pData + pExpSub->Out.iocs_offset) = PNIO_S_BAD;
						}
					}
				}
				else // submod not owned or not plugged
				{
					if (pExpSub->IoProp == PNIO_SUB_PROP_NO_DATA)
					{  // ** submod has no data **
						*(pData + pExpSub->In.iops_offset) = PNIO_S_BAD;
					}
					else
					{
						if (pExpSub->IoProp & PNIO_SUB_PROP_IN)
						{  // ** submod has input data **
							PNPB_MEMSET((PNIO_VOID*)(pData + pExpSub->In.data_offset), 0x00, pExpSub->In.data_length);
							*(pData + pExpSub->In.iops_offset) = PNIO_S_BAD;
						}
						if (pExpSub->IoProp & PNIO_SUB_PROP_OUT)
						{  // ** submod has out put data **
							*(pData + pExpSub->Out.iocs_offset) = PNIO_S_BAD;
						}
					}
				}
			}

			// **** free pointer to IOCR buffer ***
			PnpbCopyRecReadIoData(pData, ArInd, 0, 0xFFFF);
			PnpbProvUnlock(ArInd);

			PnpbExp[ArInd].IoUpdatePending = PNIO_FALSE;
		}
	}

	PNPB_ENTER(OS_MUTEX_PNPB_PULL);
	/* The submodule was pulled and we have to update user, that data are no more valid */
	if(PNIO_TRUE == PullPending.WritePending)
	{
		PNIO_DEV_ADDR  Addr;
		Addr.Geo.Slot = PullPending.Slot;
		Addr.Geo.Subslot = PullPending.Subslot;
		/* Module not ready to be updated -> PNIO_S_BAD */
        PNIO_cbf_data_write_IOxS_only(PNIO_SINGLE_DEVICE_HNDL, &Addr, PNIO_S_BAD);

		/* Done */
		PullPending.WritePending = PNIO_FALSE;
	}
	PNPB_EXIT(OS_MUTEX_PNPB_PULL);

	PNPB_EXIT(OS_MUTEX_PNPB_IO);
	OS_INSTRUMENT_USER_STOP(0xBB, 5678);
	return (PNIO_OK);
}



#ifdef PNPB_SYSTEM_REDUNDANCY
extern PNPB_INSTANCE            Pnpb;

// *------------------------------------------------------------------------
// *		pnpb_initiate_s2_data_read()
// *
// *		initiates reading the RT and/or IRT output data from stack and writing them  to
// *        the physical output
// *------------------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32 pnpb_initiate_s2_data_read  (PNIO_UINT32   DevHndl)
{
	PNIO_UINT32 *pTmpApduStat;
	PNIO_UINT16 ArInd, ArInd_other;
	PNIO_UINT32 i;
	PNIO_UINT8* pData = NULL;

	/*Empty macro*/
	OS_INSTRUMENT_USER_START(0xAA, 1234);
	/*To prevent warning*/
	LSA_UNUSED_ARG (DevHndl);


	PNPB_ENTER(OS_MUTEX_PNPB_IO);

	/*cycle through all slots for application relations*/
	for (ArInd = 0; ArInd < IOD_CFG_NUMOF_AR; ArInd++)
	{
		/*skip this ar slot if there is not opened ar inside*/
		/*or if the first parameterization of AR was not performed*/
		if((pnpb_ar_prm_ok__(ArInd, Pnpb.ArState)) && (PnpbExp[ArInd].FirstParamEndValid)) 
		{
			/* pTmpApduStat - get pointer to IOCR buffer and APDU status (including cycle counter and status bits */
			/* pData - get pointer to data received on PN*/
			PnpbConsLock(&pData, &pTmpApduStat, (ArInd));


			/*even in case of zero data, returned pointer cannot be null -> fatal error */
			if (pData == NULL)
			{
				PNPB_EXIT(OS_MUTEX_PNPB_IO);
				OS_INSTRUMENT_USER_STOP(0xAA, 5678);
				return (PNIO_NOT_OK);
			}

			/*copy Data status(1B), Transfer status(1B) and cycle counter(2B), save it to global var*/
			PNPB_COPY_BYTE(&PnpbExp[ArInd].LastApduStat, pTmpApduStat, 4);

#if(1 == IOD_USED_WITH_XHIF_HOST)
            /* Report apdu to XHIF HOST */
            PnUsr_xhif_cyclical_write_apdu(ArInd+1, PnpbExp[ArInd].LastApduStat);
#endif
            if( PNIO_AR_TYPE_SINGLE_SYSRED == PnpbExp[ArInd].ArType )
			{
				/*retrieve ar number of second of primary/backup pair*/
				ArInd_other = (PNIO_UINT16)PNPB_AR_OTHER((PNIO_UINT32)ArInd);
				/*retrieve number of S2 AR set */
				PNIO_UINT32 ArSet;
				PNPB_AR_SET(ArInd, &ArSet);

			    /*wait 300ns*/
			    volatile PNIO_INT j;
			    #define PNPB_APDU_CHECK_SLEEP_TIME 300 /*[ns]*/
			    #define PNPB_ONE_TICK_TIME 8 /*[ns]*/
			    #define PNPB_TICKS_PER_LOOP_ITERATION 10 /*[ticks]*/
			    #define PNPB_APDU_CHECK_NEEDED_ITERATIONS_TO_WAIT ( PNPB_APDU_CHECK_SLEEP_TIME / ( PNPB_ONE_TICK_TIME * PNPB_TICKS_PER_LOOP_ITERATION ) )

			    for(j = 0; j < PNPB_APDU_CHECK_NEEDED_ITERATIONS_TO_WAIT; j++ )
			    {

			    }
			    /*check if apdu status differs to old apdu status - chance to invalid data*/
			    if(0 != ( PNPB_CMPR_BYTE( &PnpbExp[ArInd].LastApduStat, pTmpApduStat, 4 ) ) )
			    {
				    /*not ok, have to take new apdu and data also*/
				    PnpbConsUnlock(ArInd);	/*Have to unlock - I have ended with this ARs data*/
				    PnpbConsLock(&pData, &pTmpApduStat, (ArInd));
				    PNPB_COPY_BYTE(&PnpbExp[ArInd].LastApduStat, pTmpApduStat, 4);
    #if(1 == IOD_USED_WITH_XHIF_HOST)
				    /* Report apdu to XHIF HOST */
				    PnUsr_xhif_cyclical_write_apdu(ArInd+1, PnpbExp[ArInd].LastApduStat);
    #endif
				    if(0 != ( PNPB_CMPR_BYTE( &PnpbExp[ArInd].LastApduStat, pTmpApduStat, 4 ) ) )
				    {
					    PNPB_SYS_TRACE_02(LSA_TRACE_LEVEL_FATAL, "Repeatedly changing APDU when buffer is locked", *pTmpApduStat, PnpbExp[ArInd].LastApduStat);
				    }
			    }


			    /*-------compare status from Apdu and current primary AR -> edge detection -----------------------------------------------*/
			    /*switch ar to primary*/
			    /*recieved status = primary && status in device = backup*/
			    if((*pTmpApduStat & REDUNDANT_PRIMARY_STATUS_BIT) && (PNPB_IO_DATA_STATE_P_B_BACKUP == pnpb_data.io.s2.primary_backup_state[ArInd]))
			    {
				    /* we are proceeding - do not interrupt*/
				    /*used to signalize edge to pndv*/
                    pnpb_data.io.s2.primary_backup_state[ArInd] = PNPB_IO_DATA_STATE_P_B_PRIMARY_PROCEEDING;

                    /* hold the ar index from the ar that went from backup to primary */
					    pnpb_data.io.s2.current_io_controller_idx[ArSet] = ArInd;

                    if /* if we are in state PNPB_S2_STATE_HOLD_O_DATA the rdht timer must have been started in the past */
					      (pnpb_data.io.s2.state[ArSet] == PNPB_S2_STATE_HOLD_O_DATA)
                    {
                        PNIO_UINT8 retval;
                        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_CHAT, "pnpb_initiate_s2_data_read rdht stopped");
                        retval = (PNIO_UINT8)PNPB_STOP_TIMER(pnpb_data.timer[ArSet].ident);

                        switch(retval)
                        {
                            case LSA_RET_OK:
                            case LSA_RET_OK_TIMER_NOT_RUNNING:
                            {
                                retval = (PNIO_UINT8)PNPB_RESET_TIMER(pnpb_data.timer[ArSet].ident);
                                break;
                            }
                            default:
                            {
                                PnpbSetLastError (PNPB_ERR_TIMER_BAD_RETURN_VAL);
                                PNPB_SYS_TRACE_01(LSA_TRACE_LEVEL_ERROR, "bad return value from PNPB_STOP_TIMER", retval);
                                PNPB_EXIT(OS_MUTEX_PNPB_IO);
                                return (PNIO_NOT_OK);
                            }
                        }

                    }
                    /*inform iom about new primary ar*/
                    PNPB_IN_SET_PRIMARY_ARID(ArInd);

                    /* backup->primary flank: do the synchroneous function first -> set DATA_STATE bit to #1 */
                    PNPB_SYS_TRACE_02(LSA_TRACE_LEVEL_CHAT, "pnpb_initiate_s2_data_read, 1", ArInd, Pnpb.ArState);
                    PNPB_IN_SET_DATA_STATE(ArInd, EDD_CSRT_DSTAT_BIT_STATE, EDD_CSRT_DSTAT_BIT_STATE);

                    if(    (pnpb_data.io.s2.primary_backup_state[ArInd_other] == PNPB_IO_DATA_STATE_P_B_BACKUP)
                        || (pnpb_data.io.s2.primary_backup_state[ArInd_other] == PNPB_IO_DATA_STATE_P_B_BACKUP_PROCEEDING) )
                    {
                        /* redundancy ok -> set bit to #0 */
                	    PNPB_SYS_TRACE_02(LSA_TRACE_LEVEL_CHAT, "pnpb_initiate_s2_data_read, 2", ArInd, Pnpb.ArState);
                        PNPB_IN_SET_DATA_STATE(ArInd, 0, EDD_CSRT_DSTAT_BIT_REDUNDANCY);

                        if( pnpb_ar_in_data__(ArInd_other, Pnpb.ArState) )
                        {
                            /* redundancy ok -> set bit to #0 */
                    	    PNPB_SYS_TRACE_02(LSA_TRACE_LEVEL_CHAT, "pnpb_initiate_s2_data_read, 3", ArInd, Pnpb.ArState);
                    	    if( PNPB_MAX_AR > ArInd_other )
                    	    {
                    		    PNPB_IN_SET_DATA_STATE( ArInd_other, 0, EDD_CSRT_DSTAT_BIT_REDUNDANCY );
                    	    }
                    	    else
                    	    {
                    		    PNPB_SYS_TRACE_00( LSA_TRACE_LEVEL_WARN, "pnpb set data state with invalid AR, 1");
                    	    }
                        }
                    }
                    else
                    {
                        /* primary fault -> set bit to #1 */
                        PNPB_SYS_TRACE_02(LSA_TRACE_LEVEL_CHAT, "pnpb_initiate_s2_data_read, 10", ArInd, Pnpb.ArState);
                        PNPB_IN_SET_DATA_STATE(ArInd_other, EDD_CSRT_DSTAT_BIT_REDUNDANCY, EDD_CSRT_DSTAT_BIT_REDUNDANCY);
                        if( pnpb_ar_in_data__(ArInd_other, Pnpb.ArState) )
                        {
                    	    PNPB_SYS_TRACE_02(LSA_TRACE_LEVEL_CHAT, "pnpb_initiate_s2_data_read, 4", ArInd, Pnpb.ArState);
                    	    if( PNPB_MAX_AR > ArInd_other )
                    	    {
                    		    PNPB_IN_SET_DATA_STATE( ArInd_other, EDD_CSRT_DSTAT_BIT_REDUNDANCY, EDD_CSRT_DSTAT_BIT_REDUNDANCY );
                    	    }
                    	    else
                    	    {
                    		    PNPB_SYS_TRACE_00( LSA_TRACE_LEVEL_WARN, "pnpb set data state with invalid AR, 2");
                    	    }
                        }
                    }


                    /* indicate edge change to pndv */
                    pnpb_io_s2_ar_set_trigger_req(ArInd);

                    /* every rising edge of the primary bit leads to the state PNPB_S2_STATE_PRIMARY */
					    pnpb_data.io.s2.state[ArSet] = PNPB_S2_STATE_PRIMARY;
			    /*move this end of if to correct place;*/
			    }

			    /*primary to backup switch edge*/
                else if( !(*pTmpApduStat & REDUNDANT_PRIMARY_STATUS_BIT) && (PNPB_IO_DATA_STATE_P_B_PRIMARY == pnpb_data.io.s2.primary_backup_state[ArInd]) )
                {
            	    /* we are proceeding - do not interrupt*/
				    /*used to signalize edge to pndv*/
                    pnpb_data.io.s2.primary_backup_state[ArInd] = PNPB_IO_DATA_STATE_P_B_BACKUP_PROCEEDING;

					    if(    (pnpb_data.io.s2.state[ArSet] == PNPB_S2_STATE_PRIMARY)
						    && (pnpb_data.io.s2.current_io_controller_idx[ArSet] == ArInd)               /*  from the primary controller */
                      )
                    {
                        PNIO_UINT8 ret_val;

                        LSA_USER_ID_TYPE    user_id;

                        /*switching in progress -. hold data*/
						    pnpb_data.io.s2.state[ArSet] = PNPB_S2_STATE_HOLD_O_DATA;

                        ret_val = LSA_RET_OK;

                        user_id.uvar32 = 0;


                        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_CHAT, "pnpb_initiate_s2_data_read rdht started");
                        /*prepare value of rdht into timer*/
						    pnpb_data.io.s2.rdht_running_timer_val[ArSet] = pnpb_data.io.s2.rdht[ArSet];
                        /* start rdht timer */
                        ret_val = (PNIO_UINT8)(PNPB_START_TIMER(pnpb_data.timer[ArSet].ident, user_id.uvar32, (PNIO_UINT16)pnpb_data.io.s2.rdht[ArSet]));
                        if( LSA_RET_ERR_PARAM == ret_val)
                        {
						     PnpbSetLastError (PNPB_ERR_TIMER_BAD_RETURN_VAL);
						     PNPB_SYS_TRACE_01(LSA_TRACE_LEVEL_ERROR, "bad return value from PNPB_START_TIMER", ret_val);
						     PNPB_EXIT(OS_MUTEX_PNPB_IO);
						     return (PNIO_NOT_OK);
                        }

                        /* as a primary-backup flank occured from now there is no controller until the next backup primary edge */
						    pnpb_data.io.s2.current_io_controller_idx[ArSet] = 0xFF;
                        PNPB_IN_SET_PRIMARY_ARID(0xFF);


                        /* redundancy not ok -> set bit to "1" */
                        /*Data state redundancy bit set to"1" = None primary AR of given ar set*/
                        PNPB_SYS_TRACE_02(LSA_TRACE_LEVEL_CHAT, "pnpb_initiate_s2_data_read, 6", ArInd, Pnpb.ArState);
                        PNPB_IN_SET_DATA_STATE(ArInd, (EDD_CSRT_DSTAT_BIT_STATE | EDD_CSRT_DSTAT_BIT_REDUNDANCY), (EDD_CSRT_DSTAT_BIT_STATE | EDD_CSRT_DSTAT_BIT_REDUNDANCY));

                        if( pnpb_ar_in_data__(ArInd_other, Pnpb.ArState) )
                        {
                            /* redundancy not ok -> set bit to "1" */
                    	    PNPB_SYS_TRACE_02(LSA_TRACE_LEVEL_CHAT, "pnpb_initiate_s2_data_read, 7", ArInd, Pnpb.ArState);
                    	    if( PNPB_MAX_AR > ArInd_other )
                    	    {
                    		    PNPB_IN_SET_DATA_STATE( ArInd_other, EDD_CSRT_DSTAT_BIT_REDUNDANCY, EDD_CSRT_DSTAT_BIT_REDUNDANCY );
                    	    }
                    	    else
                    	    {
                    		    PNPB_SYS_TRACE_00( LSA_TRACE_LEVEL_WARN, "pnpb set data state with invalid AR, 3");
                    	    }
                        }
                    }
                    else if((pnpb_data.io.s2.state[ArSet] == PNPB_S2_STATE_PRIMARY)                     /*If there is a primary AR in AR pair, current ArInd is switching from primary to backup*/
                            && (pnpb_data.io.s2.current_io_controller_idx[ArSet] == ArInd_other))       /* and ArInd_other is the last one switched from backup to primary -> switch current AR to backup*/
                    {
                        PNPB_IN_SET_DATA_STATE(ArInd, 0, (EDD_CSRT_DSTAT_BIT_STATE | EDD_CSRT_DSTAT_BIT_REDUNDANCY));
                    }

                    /* indicate edge change to pndv */
                    pnpb_io_s2_ar_set_trigger_req(ArInd);
                }


			    /*process outputs only from primary ar*/
                if((pnpb_data.io.s2.state[ArSet] == PNPB_S2_STATE_PRIMARY)						        /*at least one ar valid*/
					    && (pnpb_data.io.s2.current_io_controller_idx[ArSet] == ArInd)                  /*from the primary controller*/
			      )
			    {

				    for (i = 0; i < PnpbExp[ArInd].NumOfPluggedSub; i++)
				    {
					    PNIO_EXP_SUB*  pExpSub = &(PnpbExp[ArInd].Sub[i]);
					    PNIO_DEV_ADDR  Addr;
					    Addr.Geo.Slot = pExpSub->SlotNum;
					    Addr.Geo.Subslot = pExpSub->SubNum;

					    if (pExpSub->OwnSessionKey)
					    { // AR is submodule owner
						    if (pExpSub->IoProp & PNIO_SUB_PROP_OUT)
						    {
							    if (pExpSub->isPlugged)
							    {
    #if(1 == IOD_USED_WITH_XHIF_HOST)
							    pExpSub->Out.iocs_val = PnUsr_xhif_IO_data_read(
									    PNIO_SINGLE_DEVICE_HNDL, &Addr, pExpSub->Out.data_length,
									    pData + pExpSub->Out.data_offset, *(pData + pExpSub->Out.iops_offset));
    #else
								    pExpSub->Out.iocs_val = PNIO_cbf_data_read
											    (	PNIO_SINGLE_DEVICE_HNDL,
												    &Addr,							        // location (slot, subslot)
												    pExpSub->Out.data_length,			    // number of output bytes
												    pData + pExpSub->Out.data_offset,       // data buffer pointer
												    *(pData + pExpSub->Out.iops_offset));	// remote IOPS from IOC
    #endif
							    }
							    else
							    {
								    pExpSub->In.iocs_val = PNIO_S_BAD;
							    }
						    }
						    if (pExpSub->IoProp & PNIO_SUB_PROP_IN)
						    {
    #if(1 == IOD_USED_WITH_XHIF_HOST)
                                if (pExpSub->isPlugged)
                                {
                                    /* Send only IOxS to UserApp */
                                    PnUsr_xhif_IO_data_read(PNIO_SINGLE_DEVICE_HNDL, &Addr, 0,
                                        NULL, *(pData + pExpSub->In.iocs_offset));
                                }
    #endif
                            // save remote consumer state
							    pExpSub->In.iocs_val = *(pData + pExpSub->In.iocs_offset);
						    }
					    }
					    else
					    { // AR is not submodule owner
						    if (pExpSub->IoProp & PNIO_SUB_PROP_IN)
						    { // save remote consumer state
							    pExpSub->In.iocs_val = PNIO_S_BAD;
						    }
					    }
				    }
			    }	/*close if primary*/
			    /*for backup have to forward IOxS from primary*/
				else if ((pnpb_data.io.s2.current_io_controller_idx[ArSet] == ArInd_other               /* from the primary controller */
					    && PnpbExp[ArInd].Rdy4InpUpdateValid	  	  	  	  	  	  	  ))			/*backup AR active*/
			    {
                    PNIO_INT k;

				    for (i = 0; i < PnpbExp[ArInd].NumOfPluggedSub; i++)
				    {
                        PNIO_EXP_SUB*  pExpSub = &(PnpbExp[ArInd].Sub[i]);
                        if(ArInd_other < IOD_CFG_NUMOF_AR)
                        {
                            for (k = 0; k < PnpbExp[ArInd_other].NumOfPluggedSub; k++)
                            {
                                if ((pExpSub->ApiNum == PnpbExp[ArInd_other].Sub[k].ApiNum)
                                    && (pExpSub->SlotNum == PnpbExp[ArInd_other].Sub[k].SlotNum)
                                    && (pExpSub->SubNum == PnpbExp[ArInd_other].Sub[k].SubNum))
                                {
                                    if (pExpSub->IoProp & PNIO_SUB_PROP_OUT)
                                    {
                                        pExpSub->Out.iocs_val = PnpbExp[ArInd_other].Sub[k].Out.iocs_val;

                                        //H Bridge prm detection
                                        if( (PNIO_TRUE == PnpbExp[ArInd].FirstParamEndValid)
                                        	&&	(PNIO_FALSE == pExpSub->ParamEndValid)
                                        	&&	(PNIO_TRUE == PnpbExp[ArInd_other].Sub[k].ParamEndValid))
                                        {
                                        	if(*(pData + pExpSub->Out.iops_offset) == PNIO_S_GOOD)
                                        	{
                                        		pExpSub->ParamEndValid = PNIO_TRUE;
                                        	}
                                        }
                                    }
                                    if (pExpSub->IoProp & PNIO_SUB_PROP_IN)
                                    {
                                        pExpSub->In.iocs_val = PnpbExp[ArInd_other].Sub[k].In.iocs_val;

                                        //H Bridge prm detection
                                        if( (PNIO_TRUE == PnpbExp[ArInd].FirstParamEndValid)
                                        	&&	(PNIO_FALSE == pExpSub->ParamEndValid)
                                        	&&	(PNIO_TRUE == PnpbExp[ArInd_other].Sub[k].ParamEndValid))
                                        {
                                        	if(*(pData + pExpSub->In.iocs_offset) == PNIO_S_GOOD)
                                        	{
                                        		pExpSub->ParamEndValid = PNIO_TRUE;
                                        	}
                                        }
                                    }
								}
                            }
                        }
					}
				}
			}
			else /*non redundant ar in redundant system*/
			{
				for (i = 0; i < PnpbExp[ArInd].NumOfPluggedSub; i++)
				{
					PNIO_EXP_SUB*  pExpSub = &(PnpbExp[ArInd].Sub[i]);
					PNIO_DEV_ADDR  Addr;
					Addr.Geo.Slot = pExpSub->SlotNum;
					Addr.Geo.Subslot = pExpSub->SubNum;

					if (pExpSub->OwnSessionKey)
					{ // AR is submodule owner
						if (pExpSub->IoProp & PNIO_SUB_PROP_OUT)
						{
							if (pExpSub->isPlugged)
							{
#if(1 == IOD_USED_WITH_XHIF_HOST)
								pExpSub->Out.iocs_val = PnUsr_xhif_IO_data_read(
										PNIO_SINGLE_DEVICE_HNDL, &Addr, pExpSub->Out.data_length,
										pData + pExpSub->Out.data_offset, *(pData + pExpSub->Out.iops_offset));
#else
								pExpSub->Out.iocs_val = PNIO_cbf_data_read
											(	PNIO_SINGLE_DEVICE_HNDL,
												&Addr,							        // location (slot, subslot)
												pExpSub->Out.data_length,			    // number of output bytes
												pData + pExpSub->Out.data_offset,       // data buffer pointer
												*(pData + pExpSub->Out.iops_offset));	// remote IOPS from IOC
#endif
							}
							else
							{
								pExpSub->In.iocs_val = PNIO_S_BAD;
							}
						}
						if (pExpSub->IoProp & PNIO_SUB_PROP_IN)
						{
#if(1 == IOD_USED_WITH_XHIF_HOST)
                            if (pExpSub->isPlugged)
                            {
                                /* Send only IOxS to UserApp */
                                PnUsr_xhif_IO_data_read(PNIO_SINGLE_DEVICE_HNDL, &Addr, 0,
                                    NULL, *(pData + pExpSub->In.iocs_offset));
                            }
#endif
                        // save remote consumer state
							pExpSub->In.iocs_val = *(pData + pExpSub->In.iocs_offset);
						}
					}
					else
					{ // AR is not submodule owner
						if (pExpSub->IoProp & PNIO_SUB_PROP_IN)
						{ // save remote consumer state
							pExpSub->In.iocs_val = PNIO_S_BAD;
						}
					}
				}
			}


                // **** free pointer to IOCR buffer ***
			PNPB_SYS_TRACE_01(LSA_TRACE_LEVEL_CHAT, "pnpb_initiate_s2_data_read", ArInd);
                PnpbConsUnlock(ArInd);
		}  /*the code skips to here - prm not valid*/
	}	/*end of for through all of ars*/


	PNPB_ENTER(OS_MUTEX_PNPB_PULL);
	/* The submodule was pulled and we have to update user, that data are no more valid */
	if(PNIO_TRUE == PullPending.ReadPending)
	{
        PNIO_UINT32 PullDoneAllAR = PNIO_TRUE;
        PNIO_UINT16 ArIndTmp;

        for (ArIndTmp = 0; ArIndTmp < IOD_CFG_NUMOF_AR; ArIndTmp++)
        {
            if ((PnpbExp[ArIndTmp].Rdy4InpUpdateValid))
            {
                /* Check whether the module is pulled in stack - otherwise wait for next cycle to avoid ICS changing twice! */
                for (i = 0; i < PnpbExp[ArIndTmp].NumOfPluggedSub; i++)
                {
                    PNIO_EXP_SUB*  pExpSub = &(PnpbExp[ArIndTmp].Sub[i]);

                    /* non redundant ar in system with S2 */
                    if ((pExpSub->SlotNum == PullPending.Slot)
                        && (pExpSub->SubNum == PullPending.Subslot))
                    {
                        /* Module found! */
                        if (pExpSub->OwnSessionKey && pExpSub->isPlugged)
                        {
                            /* Module still plugged! Wait for next cycle! */
                            PullDoneAllAR = PNIO_FALSE;
                        }
                        continue;
                    }
                    else
                    {
                        /* Different module! */
                        continue;
                    }
                }
            }
        }

        if (PullDoneAllAR == PNIO_TRUE)
        {
		    PNIO_DEV_ADDR  Addr;
		    Addr.Geo.Slot = PullPending.Slot;
		    Addr.Geo.Subslot = PullPending.Subslot;
		    /* Module not ready to be updated -> PNIO_S_BAD */
#if(1 == IOD_USED_WITH_XHIF_HOST)
            /* Send only IOxS to UserApp */
            PnUsr_xhif_IO_data_read(PNIO_SINGLE_DEVICE_HNDL, &Addr, 0,
                    NULL, PNIO_S_BAD);
#else
		    PNIO_cbf_data_read_IOxS_only(PNIO_SINGLE_DEVICE_HNDL, &Addr, PNIO_S_BAD);
#endif
		    /* Done */
		    PullPending.ReadPending = PNIO_FALSE;
        }
	}
	PNPB_EXIT(OS_MUTEX_PNPB_PULL);

	PNPB_EXIT(OS_MUTEX_PNPB_IO);
	OS_INSTRUMENT_USER_STOP(0xAA, 5678);
	return (PNIO_OK);
}


// *------------------------------------------------------------------------
// *		pnpb_initiate_s2_data_write()
// *
// *		initiates writing the RT and/or IRT input data to stack. This data are read
// *        from the physical output
// *		version for system redundant communication
// *------------------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32 pnpb_initiate_s2_data_write (PNIO_UINT32   DevHndl)
{
	PNIO_UINT8  *pData = NULL;
	PNIO_UINT16 ArInd;	
	PNIO_UINT32 i;

	OS_INSTRUMENT_USER_START(0xBB, 5678);
	LSA_UNUSED_ARG (DevHndl);


	PNPB_ENTER(OS_MUTEX_PNPB_IO);
	for (ArInd = 0; ArInd < IOD_CFG_NUMOF_AR; ArInd++)
	{
		if ( (PnpbExp[ArInd].Rdy4InpUpdateValid) )
		{
			// **** get pointer to IOCR buffer ***
			PnpbProvLock(&pData, ArInd, 0, 0xFFFF);
			if (NULL == pData)
			{
				PNPB_EXIT(OS_MUTEX_PNPB_IO);
				OS_INSTRUMENT_USER_STOP(0xBB, 5678);
				return (PNIO_NOT_OK);
			}

#if CLEAR_PADDING_BYTES_IN_IO
				// **** clear IOCR buffer first ***
			if (PnpbExp[ArInd].IocrLen < 44)
				PNPB_MEMSET(pData + PnpbExp[ArInd].IocrLen, 0, 44 - PnpbExp[ArInd].IocrLen);
#endif

			// **** write submodule IO data into IOCR buffer ***
			for (i = 0; i < PnpbExp[ArInd].NumOfPluggedSub; i++)
			{
				PNIO_EXP_SUB*  pExpSub = &(PnpbExp[ArInd].Sub[i]);
				PNIO_DEV_ADDR  Addr;
				Addr.Geo.Slot = pExpSub->SlotNum;
				Addr.Geo.Subslot = pExpSub->SubNum;

				if( PNIO_AR_TYPE_SINGLE_SYSRED == PnpbExp[ArInd].ArType )
				{
					/*retrieve number of S2 AR set */
					PNIO_UINT32 ArSet;
					PNPB_AR_SET(ArInd, &ArSet);
					// *** provide io data and iops from input submodule ***
					if (pExpSub->OwnSessionKey && pExpSub->isPlugged)
					{

						if (pExpSub->IoProp & PNIO_SUB_PROP_IN)
						{  // ** submod has input data ?? **
						   // *** update (input-)provider state in input frame ***
								if ( (!pExpSub->IsWrongSubmod) && (pExpSub->ParamEndValid) && (0xff != pnpb_data.io.s2.current_io_controller_idx[ArSet]) )
							{
#if(1 == IOD_USED_WITH_XHIF_HOST)
                                pExpSub->In.iops_val = PnUsr_xhif_IO_data_write(
                                        PNIO_SINGLE_DEVICE_HNDL, &Addr, pExpSub->In.data_length,
                                        pData + pExpSub->In.data_offset, pExpSub->In.iocs_val);
                                *(pData + pExpSub->In.iops_offset) = pExpSub->In.iops_val;
#else
								pExpSub->In.iops_val = PNIO_cbf_data_write
											(	PNIO_SINGLE_DEVICE_HNDL,
												&Addr,							         // location (slot, subslot)
												pExpSub->In.data_length,			     // number of output bytes
												pData + pExpSub->In.data_offset,         // data buffer pointer
												pExpSub->In.iocs_val);	                 // last remote consumer state from IOC	// iops (from the io controller)
								 *(pData + pExpSub->In.iops_offset) = pExpSub->In.iops_val;

#endif
							}
							else
							{
								*(pData + pExpSub->In.iops_offset) = PNIO_S_BAD;
							}
						}
						else
						{ // ** submod has no IO data  **
							if (pExpSub->IoProp == PNIO_SUB_PROP_NO_DATA)
							{
								// *** update (input-)provider state in input frame ***
								if ((!pExpSub->IsWrongSubmod) && pExpSub->ParamEndValid)
									*(pData + pExpSub->In.iops_offset) = pExpSub->In.iops_val;
								else
									*(pData + pExpSub->In.iops_offset) = PNIO_S_BAD;
							}
						}

						// *** if subslot has output data, update (output-)consumer state in input frame ***
						if (pExpSub->IoProp & PNIO_SUB_PROP_OUT)
						{  // ** submod has output data ?? **
							if ((!pExpSub->IsWrongSubmod) && pExpSub->ParamEndValid)
								*(pData + pExpSub->Out.iocs_offset) = pExpSub->Out.iocs_val;
							else
								*(pData + pExpSub->Out.iocs_offset) = PNIO_S_BAD;
						}
					}
					else	/*not owned by this ar or not plugged*/
					{
						if (pExpSub->IoProp & PNIO_SUB_PROP_OUT)
						{  // ** submod has out put data ?? **
							*(pData + pExpSub->Out.iocs_offset) = PNIO_S_BAD;
						}
						if ((pExpSub->IoProp & PNIO_SUB_PROP_IN) || (pExpSub->IoProp == PNIO_SUB_PROP_NO_DATA))
						{  // ** submod has input data ?? **
							*(pData + pExpSub->In.iops_offset) = PNIO_S_BAD;
						}
						#define PNPB_INVALIDATE_INPUT_DATA_FOR_PULLED_MODULE
						#ifdef PNPB_INVALIDATE_INPUT_DATA_FOR_PULLED_MODULE
						if ((pExpSub->IoProp & PNIO_SUB_PROP_IN) && (!pExpSub->isPlugged))
						{
							PNPB_MEMSET((PNIO_VOID*)(pData + pExpSub->In.data_offset), 0x00, pExpSub->In.data_length);
						}
						#endif
					}
				}
				else /*non redundant ar in system with S2*/
				{
					// *** provide io data and iops from input submodule ***
					if (pExpSub->OwnSessionKey && pExpSub->isPlugged && ( PNIO_TRUE == PnpbExp[ ArInd ].FirstParamEndValid ) )
					{
						if (pExpSub->IoProp == PNIO_SUB_PROP_NO_DATA)
						{   // ** submod has no IO data **
							// *** update (input-)provider state in input frame ***
							if ((!pExpSub->IsWrongSubmod) && pExpSub->ParamEndValid)
								*(pData + pExpSub->In.iops_offset) = pExpSub->In.iops_val;
							else
								*(pData + pExpSub->In.iops_offset) = PNIO_S_BAD;
						}
						else
						{
							if (pExpSub->IoProp & PNIO_SUB_PROP_IN)
							{  // ** submod has input data **
							   // *** update (input-)provider state in input frame ***
								if ((!pExpSub->IsWrongSubmod) && pExpSub->ParamEndValid)
								{
#if(1 == IOD_USED_WITH_XHIF_HOST)
                                pExpSub->In.iops_val = PnUsr_xhif_IO_data_write(
                                        PNIO_SINGLE_DEVICE_HNDL, &Addr, pExpSub->In.data_length,
                                        pData + pExpSub->In.data_offset, pExpSub->In.iocs_val);
                                *(pData + pExpSub->In.iops_offset) = pExpSub->In.iops_val;
#else
									pExpSub->In.iops_val = PNIO_cbf_data_write
												(	PNIO_SINGLE_DEVICE_HNDL,
													&Addr,							         // location (slot, subslot)
													pExpSub->In.data_length,			     // number of output bytes
													pData + pExpSub->In.data_offset,         // data buffer pointer
													pExpSub->In.iocs_val);	                 // last remote consumer state from IOC	// iops (from the io controller)
									 *(pData + pExpSub->In.iops_offset) = pExpSub->In.iops_val;

#endif
								}
								else
								{
									*(pData + pExpSub->In.iops_offset) = PNIO_S_BAD;
								}
							}
							if (pExpSub->IoProp & PNIO_SUB_PROP_OUT)
							{  // ** submod has out put data **
								if ((!pExpSub->IsWrongSubmod) && pExpSub->ParamEndValid)
									*(pData + pExpSub->Out.iocs_offset) = pExpSub->Out.iocs_val;
								else
									*(pData + pExpSub->Out.iocs_offset) = PNIO_S_BAD;
							}
						}
					}
					else // submod not owned or not plugged
					{
						if (pExpSub->IoProp == PNIO_SUB_PROP_NO_DATA)
						{  // ** submod has no data **
							*(pData + pExpSub->In.iops_offset) = PNIO_S_BAD;
						}
						else
						{
							if (pExpSub->IoProp & PNIO_SUB_PROP_IN)
							{  // ** submod has input data **
								PNPB_MEMSET((PNIO_VOID*)(pData + pExpSub->In.data_offset), 0x00, pExpSub->In.data_length);
								*(pData + pExpSub->In.iops_offset) = PNIO_S_BAD;
							}
							if (pExpSub->IoProp & PNIO_SUB_PROP_OUT)
							{  // ** submod has out put data **
								*(pData + pExpSub->Out.iocs_offset) = PNIO_S_BAD;
							}
						}
					}
				}
			}

			// **** free pointer to IOCR buffer ***
			PnpbCopyRecReadIoData(pData, ArInd, 0, 0xFFFF);
			PNPB_SYS_TRACE_01(LSA_TRACE_LEVEL_CHAT, "pnpb_initiate_s2_data_write", ArInd);
			PnpbProvUnlock(ArInd);

			PnpbExp[ArInd].IoUpdatePending = PNIO_FALSE;
		}
	}

	PNPB_ENTER(OS_MUTEX_PNPB_PULL);
	/* The submodule was pulled and we have to update user, that data are no more valid */
	if(PNIO_TRUE == PullPending.WritePending)
	{
        PNIO_UINT32 PullDoneAllAR = PNIO_TRUE;
        PNIO_UINT16 ArIndTmp;

        for (ArIndTmp = 0; ArIndTmp < IOD_CFG_NUMOF_AR; ArIndTmp++)
        {
            if ((PnpbExp[ArIndTmp].Rdy4InpUpdateValid))
            {
                /* Check whether the module is pulled in stack - otherwise wait for next cycle to avoid ICS changing twice! */
                for (i = 0; i < PnpbExp[ArIndTmp].NumOfPluggedSub; i++)
                {
                    PNIO_EXP_SUB*  pExpSub = &(PnpbExp[ArIndTmp].Sub[i]);

                    /* non redundant ar in system with S2 */
                    if ((pExpSub->SlotNum == PullPending.Slot)
                        && (pExpSub->SubNum == PullPending.Subslot))
                    {
                        /* Module found! */
                        if (pExpSub->OwnSessionKey && pExpSub->isPlugged)
                        {
                            /* Module still plugged! Wait for next cycle! */
                            PullDoneAllAR = PNIO_FALSE;
                        }
                        continue;
                    }
                    else
                    {
                        /* Different module! */
                        continue;
                    }
                }
            }
        }

        if (PullDoneAllAR == PNIO_TRUE)
        {
            /* Pull registered in stack! Check ICS! */
            PNIO_DEV_ADDR  Addr;
            Addr.Geo.Slot = PullPending.Slot;
            Addr.Geo.Subslot = PullPending.Subslot;
            /* Module not ready to be updated -> PNIO_S_BAD */
#if(1 == IOD_USED_WITH_XHIF_HOST)
    /* Send only IOxS to UserApp */
            PnUsr_xhif_IO_data_write(PNIO_SINGLE_DEVICE_HNDL, &Addr, 0,
                NULL, PNIO_S_BAD);
#else
            PNIO_cbf_data_write_IOxS_only(PNIO_SINGLE_DEVICE_HNDL, &Addr, PNIO_S_BAD);
#endif
            /* Done */
            PullPending.WritePending = PNIO_FALSE;
        }
	}
	PNPB_EXIT(OS_MUTEX_PNPB_PULL);

	PNPB_EXIT(OS_MUTEX_PNPB_IO);
	OS_INSTRUMENT_USER_STOP(0xBB, 5678);
	return (PNIO_OK);
}


/*edge signalled from pndv -> change primary_backup_state*/
/*this is second event in row - finishes switchover in pnpb*/
PNIO_VOID pnpb_io_s2_ar_set_trigger_cnf(PNIO_UINT32 ar_idx, PNIO_UINT8 edge)
{
	PNPB_SYS_TRACE_02(LSA_TRACE_LEVEL_CHAT, "pnpb_io_s2_ar_set_trigger_cnf", ar_idx, edge);
	/*ar is going down - don't change state*/
	if(0xff == ar_idx)
	{
		return;
	}

	if(0 == edge)
	{
	    if ( Pnpb.ArState[ar_idx] != PNPB_AR_OFFLINE )
	    {
	        /*set outgoing Data status byte to say this AR is backup*/
	        PNPB_IN_SET_DATA_STATE(ar_idx, 0, EDD_CSRT_DSTAT_BIT_STATE);
	    }

		pnpb_data.io.s2.primary_backup_state[ar_idx] = PNPB_IO_DATA_STATE_P_B_BACKUP;
	}
	else
	{
		pnpb_data.io.s2.primary_backup_state[ar_idx] = PNPB_IO_DATA_STATE_P_B_PRIMARY;
	}
}

/*signal to pndv about edge - redundancy switchover*/
PNIO_VOID pnpb_io_s2_ar_set_trigger_req(PNIO_UINT32 ar_idx)
{
    if(pnpb_data.io.s2.primary_backup_state[ar_idx] == PNPB_IO_DATA_STATE_P_B_PRIMARY_PROCEEDING)
    {
        pnpb_write_event_to_pndv(PNDV_EV_TO_PNDV_SR_EDGE_IND, (PNIO_UINT8)ar_idx, 1, NULL);
        PNPB_TRIGGER_PNDV ();

    }
    else if(pnpb_data.io.s2.primary_backup_state[ar_idx] == PNPB_IO_DATA_STATE_P_B_BACKUP_PROCEEDING)
    {
        pnpb_write_event_to_pndv(PNDV_EV_TO_PNDV_SR_EDGE_IND, (PNIO_UINT8)ar_idx, 0, NULL);
        PNPB_TRIGGER_PNDV ();
    }
}

/*will be called in case of rdht timer overflow*/
PNIO_VOID pnpb_rdht_timeout(PNIO_UINT32 ArSet)
{
	PNIO_UINT32 i,Found = 0;
	PNIO_UINT8  ArInd = 0;

	PNPB_ENTER(OS_MUTEX_PNPB_IO);

    pnpb_data.timer[ArSet].rqb_in_use = PNIO_FALSE;

    pnpb_data.io.s2.state[ArSet] = PNPB_S2_STATE_NO_AR_OR_ALL_BACKUP;

    pnpb_data.io.s2.current_io_controller_idx[ArSet] = 0xFF;

    PNPB_IN_SET_PRIMARY_ARID(0xFF);

    PNPB_SYS_TRACE_01(LSA_TRACE_LEVEL_CHAT,  "rdht_timeout_overflow for ArSet : %d", ArSet);

    for ( i = 0; i < IOD_CFG_NUMOF_AR ; i++)
    {
        if (( Pnpb.ArState[i] != PNPB_AR_OFFLINE ) && ( PNIO_AR_TYPE_SINGLE_SYSRED == PnpbExp[i].ArType ) )
        {
            if ( Pnpb_ar_sr_set[i] == ArSet )
            {
                if ( ( Pnpb.ArState[i] == PNPB_AR_INDATA_IND) || (Pnpb.ArState[i] == PNPB_AR_SM_INDATA_IND) )
                {
                    PNPB_IN_SET_DATA_STATE( i, EDD_CSRT_DSTAT_BIT_REDUNDANCY, EDD_CSRT_DSTAT_BIT_REDUNDANCY );

                }
                //Pnpb_ar_sr_set[i] = 0;
                ArInd = (PNIO_UINT8)i;
                Found = 1;
            }
        }
    }

    if ( Found )
    {
        pnpb_write_event_to_pndv(PNDV_EV_TO_PNDV_SR_RDHT_TIMEOUT, (PNIO_UINT8 ) ArInd, 0, NULL);
        PNPB_TRIGGER_PNDV ();
    }

    PNPB_EXIT(OS_MUTEX_PNPB_IO);
}

PNIO_UINT32		pnpb_second_ar_of_arset( PNIO_UINT32 first_ar )
{
	PNIO_UINT32 i;
	PNIO_UINT32 sr_ar_set = Pnpb_ar_sr_set[ first_ar ];
	if( 0 == sr_ar_set )
    {
		/* 0 is for non redundant / there is no connected AR */
		return 0xff;
    }
	/*check all ARs for seeked sr_ar_set*/
	for( i = 0; i < IOD_CFG_NUMOF_AR; i++ )
	{
		if( sr_ar_set == Pnpb_ar_sr_set[ i ] )
		{
			/*found first_ar -> continue with search*/
			if( i == first_ar )
			{
				/*nothing to do here*/
			}
			/*found seeked ar -> return*/
			else
			{
				return i;
			}
		}
	}
	/*not found => only one ar of set connected*/
	/*returned value is used as array index - added special default value - not fatal*/
	return IOD_CFG_NUMOF_AR; /*highest as (virtual) never used ar - always returns as not connected, backup*/
}

/*this should be fast -> only set flag*/
/*functionality done in pnpb_rdht_timeout, which is called from elsewhere based on this flag*/
PNIO_VOID pnpb_trigger_rdht_timeout(LSA_UINT16 timer_id, LSA_USER_ID_TYPE user_id)
{
	/*find ArSet from timer_id - go through all ids*/
	PNIO_UINT32 ArSet;
	for(ArSet = 0; ArSet < PNPB_MAX_S2_AR_SETS; ArSet++)
	{
		if(timer_id == pnpb_data.timer[ArSet].ident)
		{
			break;
		}
	}
    if(ArSet < PNPB_MAX_S2_AR_SETS)
    {
        /*set pnpb_rdht_timeout() to be called before s2 data read*/
        pnpb_data.timer[ArSet].rqb_in_use = PNIO_TRUE;
    }
}

/*additional functionality for ar release in case of redundancy release*/
PNIO_UINT8 pnpb_io_s2_ar_release(PNIO_UINT32 ar_idx)
{
    PNIO_INT i, haveSysRed = 0;
    PNIO_UINT8 retval;

    PNIO_UINT32 ArSet;

    PNPB_AR_SET(ar_idx, &ArSet);
    PNIO_UINT32 idx_other_ar = PNPB_AR_OTHER(ar_idx);

	PNPB_SYS_TRACE_01(LSA_TRACE_LEVEL_CHAT, "pnpb_s2_ar_release on ar", ar_idx);
    pnpb_data.io.s2.primary_backup_state[ar_idx] = PNPB_IO_DATA_STATE_P_B_BACKUP;

    /*reset info about ArType*/
    PnpbExp[ ar_idx ].ArType = PNIO_AR_TYPE_SINGLE;
    /* is there any other s2 ar remaining? */
    for( i = 0; i < IOD_CFG_NUMOF_AR; i++ )
    {
    	if( PNIO_AR_TYPE_SINGLE_SYSRED == PnpbExp[ i ].ArType )
    	{
    		haveSysRed = 1;
    	}
    }
    if( 0 == haveSysRed )
    {
    	pnpb_ArType[ ar_idx ] = PNIO_AR_TYPE_SINGLE; /* last sysred ar removed - system now processes data as default non s2 */
    }

    /* the other ar exists - start rdht timer */
    if(    (pnpb_data.io.s2.state[ArSet] == PNPB_S2_STATE_PRIMARY)
        && (pnpb_data.io.s2.current_io_controller_idx[ArSet] == ar_idx)               /*  ar in parameter is primary  */
      )
    {
        PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_CHAT, "switch to other ar");

        /* as a primary-backup flank occured from now there is no controller until the next backup primary edge */
        pnpb_data.io.s2.current_io_controller_idx[ArSet]    = 0xFF;
        PNPB_IN_SET_PRIMARY_ARID(0xFF);

        /* we must assign the primary bit too in order to take over the new session key */
        PNPB_IN_SET_DATA_STATE(ar_idx, EDD_CSRT_DSTAT_BIT_STATE | EDD_CSRT_DSTAT_BIT_REDUNDANCY, EDD_CSRT_DSTAT_BIT_STATE | EDD_CSRT_DSTAT_BIT_REDUNDANCY);


        /*other AR exists*/
        if(Pnpb.ArState[idx_other_ar] != PNPB_AR_OFFLINE)
        {
        	if( PNPB_MAX_AR > idx_other_ar )
        	{
        		PNPB_IN_SET_DATA_STATE( idx_other_ar, EDD_CSRT_DSTAT_BIT_REDUNDANCY, EDD_CSRT_DSTAT_BIT_REDUNDANCY );
        	}
        	else
        	{
        		PNPB_SYS_TRACE_01( LSA_TRACE_LEVEL_FATAL, "pnpb set data state with invalid AR %x, 5", idx_other_ar);
        	}

            PNIO_UINT8 ret_val;

            LSA_USER_ID_TYPE    user_id;

            pnpb_data.io.s2.state[ArSet] = PNPB_S2_STATE_HOLD_O_DATA;

            ret_val = LSA_RET_OK;

            user_id.uvar32 = 0;

            /*prepare value of rdht into timer*/
            pnpb_data.io.s2.rdht_running_timer_val[ArSet] = pnpb_data.io.s2.rdht[ArSet];
            /* start rdht timer */
            ret_val = (PNIO_UINT8)(PNPB_START_TIMER(pnpb_data.timer[ArSet].ident, user_id.uvar32, (PNIO_UINT16)pnpb_data.io.s2.rdht[ArSet]));
            if( LSA_RET_ERR_PARAM == ret_val)
            {
                PnpbSetLastError (PNPB_ERR_TIMER_BAD_RETURN_VAL);
                PNPB_SYS_TRACE_01(LSA_TRACE_LEVEL_ERROR, "bad return value from PNPB_START_TIMER", ret_val);
            }
            if(idx_other_ar < PNPB_MAX_AR)
            {
                /* The case of two primary and releasing the second (active) one */
                if(PNPB_IO_DATA_STATE_P_B_PRIMARY == pnpb_data.io.s2.primary_backup_state[idx_other_ar])
                {
                    /* Invalid (first) primary have to be switched to backup */
                    PNPB_IN_SET_DATA_STATE( idx_other_ar, 0, EDD_CSRT_DSTAT_BIT_STATE );
                    pnpb_data.io.s2.primary_backup_state[idx_other_ar] = PNPB_IO_DATA_STATE_P_B_BACKUP;
                }
            }
        }
    }
    if(idx_other_ar <= IOD_CFG_NUMOF_AR)
    {
        if((IOD_CFG_NUMOF_AR == idx_other_ar) || (Pnpb.ArState[idx_other_ar] == PNPB_AR_OFFLINE))
        {
            /* the one and only ar has gone  */
            PNPB_SYS_TRACE_00(LSA_TRACE_LEVEL_CHAT, "last ar released");
            /* Stop RDHT as there is no-one to receive the alarm */
            /* If there will be RDHT alarm called, it will made mess for new AR establishing */
            retval = (PNIO_UINT8)PNPB_STOP_TIMER(pnpb_data.timer[ArSet].ident);
            switch(retval)
            {
                case LSA_RET_OK:
                case LSA_RET_OK_TIMER_NOT_RUNNING:
                {
                    retval = (PNIO_UINT8)PNPB_RESET_TIMER(pnpb_data.timer[ArSet].ident);
                    break;
                }
                default:
                {
                    PnpbSetLastError (PNPB_ERR_TIMER_BAD_RETURN_VAL);
                    PNPB_SYS_TRACE_01(LSA_TRACE_LEVEL_ERROR, "bad return value from PNPB_STOP_TIMER in AR release", retval);
                    PNPB_EXIT(OS_MUTEX_PNPB_IO);
                    return (PNIO_NOT_OK);
                }
            }
            /* Also clear flag, for the case we already have triggerred alarm */
            pnpb_data.timer[ArSet].rqb_in_use = PNIO_FALSE;
        }
    }
    /* Indicate that this AR is offline */
    Pnpb.ArState[ar_idx] = PNPB_AR_OFFLINE;
    // clean up of arset of disconnected AR
    Pnpb_ar_sr_set[ ar_idx ] = 0;

    return 0;
}

#endif	/* PNPB_SYSTEM_REDUNDANCY */


// *------------------------------------------------------------------------
// *		pnpb_set_iops()
// *
// *		changes the IOPS value of the PDEV interface or port
// *		data access.
// *
// *		input:  Subslot     subslot number of PDEV (0x8000, 0x8001,..)
// *		        Iops        PNIO_S_GOOD, PNIO_S_BAD
// *
// *
// *------------------------------------------------------------------------
PNPB_CODE_FAST PNIO_UINT32 pnpb_set_iops   (PNIO_UINT32 Api,
						   PNIO_UINT32 SlotNum,
						   PNIO_UINT32 SubNum,
						   PNIO_UINT8  Iops)
{
	PNIO_UINT32     ArInd;   // 0..N-1

	// *** set iops in expected configuration ***
	{
		PNIO_EXP_SUB* pSubExp = pnpb_sub_exp_allAr_getp  (&ArInd, Api,SlotNum, SubNum);
		if (pSubExp->IoProp !=  PNIO_SUB_PROP_NO_DATA)
		{
			PNPB_API_TRACE_04 (LSA_TRACE_LEVEL_ERROR, "ERR pnpb_set_iops (Api%d Sl%d Ss%d=%d\n",
							   Api, SlotNum, SubNum, Iops);
			PnpbSetLastError (PNIO_ERR_IOD_INVALID_SUBSLOT);
			return (PNIO_NOT_OK);
		}
		pSubExp->In.iops_val = Iops;
	}

	// *** set iops in real configuration ***
	{
		PNPB_REAL_SUB* pSubReal = pnpb_sub_real_getp  (Api,SlotNum, SubNum);
		pSubReal->InIops = Iops;
	}

	PNPB_API_TRACE_04 (LSA_TRACE_LEVEL_NOTE, "New IOPS (Api%d Sl%d Ss%d=%d\n",
					   Api, SlotNum, SubNum, Iops);
	return (PNIO_OK);
}


#if IOD_INCLUDE_REC8028_8029

	PNIO_UINT32 pnpb_io_init()
	{
		PNPB_MEMSET(&Rec8028_Req[0], 0, sizeof (Rec8028_Req));
		PNPB_MEMSET(&Rec8029_Req[0], 0, sizeof (Rec8029_Req));
#ifdef PNPB_SYSTEM_REDUNDANCY
		PNIO_INT i;
		for(i = 0; i < PNPB_MAX_S2_AR_SETS; i++)
		{
			pnpb_data.io.s2.rdht[i] = 0;
		}

#endif
		return (PNIO_OK);
	}

	// *------------------------------------------------------------------------
	// *        pnpb_get_input_data()
	// *
	// *        initiates reading the RT and/or IRT input data for one subslot and
	// *        wait until data have been provided.  Reading input data and IOPS is
	// *        done in function PnpbProvLock, reading IOCS is done in
	// *        function PnpbConsLock, both functions in context
	// *        of the Task CycleIo (application task).
	// *        Note: PnpbProvLock is called inside PNIO_initiate_data_write or
	// *              PNIO_dbai_buf_lock.
	// *              PnpbConsLock is called inside PNIO_initiate_data_read or
	// *              PNIO_dbai_buf_lock.
	// *
	// *        input:  ArInd       AR-index (0...N-1)
	// *                pExSub      pointer to properties of expected submodule
	// *                pPioDatBuf  destination pointer for the IO data
	// *                pIopsBuf    destination pointer for the local IOPS value
	// *                pIocsBuf    destination pointer for the remote IOCS value
	// *
	// *------------------------------------------------------------------------
	PNIO_UINT32 pnpb_get_input_data (PNIO_UINT32    ArInd,
									 PNIO_EXP_SUB*  pExpSub,
									 PNIO_UINT8*    pIoDatBuf,
									 PNIO_UINT8*    pIopsBuf,
									 PNIO_UINT8*    pIocsBuf)
	{
		// record index 0x8029  = read output values
		Rec8028_Req[ArInd].SrcIoDatOffs  = pExpSub->In.data_offset;
		Rec8028_Req[ArInd].SrcIopsOffs   = pExpSub->In.iops_offset;
		Rec8028_Req[ArInd].SrcIocsOffs   = pExpSub->In.iocs_offset;
		Rec8028_Req[ArInd].pDstIoDat     = pIoDatBuf;
		Rec8028_Req[ArInd].pDstIops      = pIopsBuf;
		Rec8028_Req[ArInd].pDstIocs      = pIocsBuf;
		Rec8028_Req[ArInd].IoDatLen      = pExpSub->In.data_length;
		Rec8028_Req[ArInd].ExecProvRq    = IO_READ_STATE_EXEC;
		Rec8028_Req[ArInd].ExecConsRq    = IO_READ_STATE_EXEC;

		// *** wait until request has been served by Io-Task (Buffer-Lock-function)
		while ((Rec8028_Req[ArInd].ExecProvRq == IO_READ_STATE_EXEC) || (Rec8028_Req[ArInd].ExecConsRq == IO_READ_STATE_EXEC))
		{
			PNPB_WAIT_MS(READ_IO_POLLING_TIME_MS); // we poll here to avoid semaphore handling in time critical cycleIO-task
		}

		return (PNIO_OK);
	}


	// *------------------------------------------------------------------------
	// *        pnpb_get_output_data()
	// *
	// *        initiates reading the RT and/or IRT output data for one subslot and
	// *        wait until data have been provided.  Reading output data and IOPS is
	// *        done in function PnpbConsLock, reading IOCS is done in
	// *        function PnpbProvLock, both functions in context
	// *        of the Task CycleIo (application task).
	// *
	// *        Note: PnpbProvLock is called inside PNIO_initiate_data_write or
	// *              PNIO_dbai_buf_lock.
	// *              PnpbConsLock is called inside PNIO_initiate_data_read or
	// *              PNIO_dbai_buf_lock.
	// *        input:  ArInd       AR-index (0...N-1)
	// *                pExSub      pointer to properties of expected submodule
	// *                pPioDatBuf  destination pointer for the IO data
	// *                pIopsBuf    destination pointer for the remote IOPS value
	// *                pIocsBuf    destination pointer for the local IOCS value
	// *
	// *------------------------------------------------------------------------
	PNIO_UINT32 pnpb_get_output_data (PNIO_UINT32    ArInd,
									  PNIO_EXP_SUB*  pExpSub,
									  PNIO_UINT8*    pIoDatBuf,
									  PNIO_UINT8*    pIopsBuf,
									  PNIO_UINT8*    pIocsBuf)
	{
		// record index 0x8029  = read output values
		Rec8029_Req[ArInd].SrcIoDatOffs  = pExpSub->Out.data_offset;
		Rec8029_Req[ArInd].SrcIopsOffs   = pExpSub->Out.iops_offset;
		Rec8029_Req[ArInd].SrcIocsOffs   = pExpSub->Out.iocs_offset;
		Rec8029_Req[ArInd].pDstIoDat     = pIoDatBuf;
		Rec8029_Req[ArInd].pDstIops      = pIopsBuf;
		Rec8029_Req[ArInd].pDstIocs      = pIocsBuf;
		Rec8029_Req[ArInd].IoDatLen      = pExpSub->Out.data_length;
		Rec8029_Req[ArInd].ExecConsRq    = IO_READ_STATE_EXEC;
		Rec8029_Req[ArInd].ExecProvRq    = IO_READ_STATE_EXEC;

		// *** wait until request has been served by Io-Task (Buffer-Lock-function)
		while ((Rec8029_Req[ArInd].ExecConsRq == IO_READ_STATE_EXEC) || (Rec8029_Req[ArInd].ExecProvRq == IO_READ_STATE_EXEC))
		{
			PNPB_WAIT_MS(READ_IO_POLLING_TIME_MS); // we poll here to avoid semaphore handling in time critical cycleIO-task
		}

		return (PNIO_OK);
	}

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
