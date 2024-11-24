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
/*  F i l e               &F: nv_data.c                                 :F&  */
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
#include "compiler_stdlibs.h"
#include "pniousrd.h"
#include "iodapi_rema.h"
#include "os.h"
#include "os_taskprio.h"
#include "nv_data.h"
#include "pnio_trace.h"
#include "os_flash.h"
#include "PnUsr_xhif.h"

// *********** defines **********

#define LTRC_ACT_MODUL_ID           124
#define IOD_MODULE_ID               124

#define NV_DATA_INVALIUD_MAC_DEBUG


// *********** some default settings *******
#define DEFAULT_DEVICENAME          ""
#define DEVICE_IP_ADDRESS           0x0
#define DEVICE_SUBNET_MASK          0x0
#define DEVICE_DEF_ROUTER           0x0


#define DEFAULT_MAC_ADDR            {0x08, 0x00, 0x06, 0x02, 0x01, 0x10}
#define DEFAULT_SNMP_SYSNAME        ""
#define DEFAULT_SNMP_SYSCONT        ""
#define DEFAULT_SNMP_SYSLOC         ""


// ***** NV data object types ***
#define NV_TYP_MAC                  0x0001
#define NV_TYP_NAME                 0x0002
#define NV_TYP_IPS                  0x0004
#define NV_TYP_IM                   0x0008
#define NV_TYP_SNMP                 0x0010
#define NV_TYP_REMA                 0x0020
#define NV_TYP_PRIV                 0x0040
#define NV_TYP_ARFSU                0x0080

#define NVDATA_GROUP_DCP_COMM       (NV_TYP_NAME | NV_TYP_IPS | NV_TYP_SNMP | NV_TYP_REMA | NV_TYP_ARFSU)
#define NVDATA_GROUP_DCP_ALL        (NVDATA_GROUP_DCP_COMM | NV_TYP_IM)
#define NVDATA_GROUP_INITIAL_ALL    0xffff


// *********** data structures **********
#pragma pack(4)
	typedef struct
	{
		PNIO_UINT32         CheckSum;                           // checksum over all, must be first element
		PNIO_UINT32         Version;                            // version of the data
		PNIO_UINT8          MacAddr[MAC_ADDR_LEN];              // mac address
		NV_IP_SUITE         IpSuite;                            // ip suite (ip-addr, subnet mask, default router)
		IM1_DATA            IM1 [IOD_CFG_MAX_NUMOF_SUBSLOTS];   // I&M1  data
		IM2_DATA            IM2 [IOD_CFG_MAX_NUMOF_SUBSLOTS];   // I&M2  data
		IM3_DATA            IM3 [IOD_CFG_MAX_NUMOF_SUBSLOTS];   // I&M3  data
		IM4_DATA            IM4 [IOD_CFG_MAX_NUMOF_SUBSLOTS];   // I&M4  data
		PNIO_UINT16         SnmpSysNameLen;                     // 0xffff: invalid, else length value
		PNIO_UINT8          SnmpSysName[SNMP_SYSNAME_MAXLEN];
		PNIO_UINT16         SnmpSysContLen;                     // 0xffff: invalid, else length value
		PNIO_UINT8          SnmpSysCont[SNMP_SYSCONT_MAXLEN];
		PNIO_UINT16         SnmpSysLocLen;                      // 0xffff: invalid, else length value
		PNIO_UINT8          SnmpSysLoc[SNMP_SYSLOC_MAXLEN];
		PNIO_UINT16         DevNameLen;                         // 0xffff: invalid, else length value
		PNIO_UINT8          DevName[DEVICE_NAME_MAXLEN];
		PNIO_UINT16         RemaDataLen;                        // 0xffff: invalid, else length value
		PNIO_UINT8          RemaData[REMA_DATA_MAXLEN];
		PNIO_UINT16         PrivDataLen;                        // 0xffff: invalid, else length value
		PNIO_UINT8          PrivData[PRIV_DATA_MAXLEN];
		PNIO_UINT8			ARFSU[ARFSU_LEN];			//0x0000: empty

	} NV_DATA_STRUCT;
#pragma pack()
	// ******* static data ********
	static const PNIO_UINT8     DefaultMacAddr[MAC_ADDR_LEN] = DEFAULT_MAC_ADDR;

	static       NV_DATA_STRUCT NvData;
	static       PNIO_BOOL useErtecNVMem = PNIO_FALSE;

	 /*------------------------------------------------------------------------------
	 *
	 * Function   : device_compute_checksum
	 *
	 * Description: Computes a checksum.
	 *
	 * Parameters : pMem - memory block of data to compute the checksum of
	 *              size - sieze of memory block in bytes
	 *
	 * Returns    : Checksum
	 *
	 *----------------------------------------------------------------------------*/
	static PNIO_UINT32 device_compute_checksum(PNIO_CHAR* pMem, PNIO_INT size)
	{
	   PNIO_UINT32 cksum = 0;
	   PNIO_INT i;

	   /* Sum all the bytes together */
	   for (i = 0; i < size; i++)
	   {
		  cksum += *(pMem + i);
	   }

	   /* Do a little shuffling */
	   cksum = (cksum >> 16) + (cksum & 0xffff);
	   cksum += (cksum >> 16);

	   /* Return the bitwise complement of the resulting mishmash */
	   return(PNIO_UINT32)(~cksum);
	}

	PNIO_VOID Bsp_nv_data_flash_done ( PNIO_VOID* arg, PNIO_UINT32 DatLen )
	{
	    PNIO_printf ("***Bsp_nv_data_store: Completed ***\n" );

	    nv_function_param* prm = (nv_function_param*) arg;

	    /* If parameter is not null, perform desired action */
	    //lint --e{548} doesn't need any else block.
	    if (prm != LSA_NULL)
	    {
#if(1 == IOD_USED_WITH_XHIF_HOST)
	    	if(prm->perform_host_callback == PNIO_TRUE)
	    	{
	            if(prm->im_callback == PNIO_TRUE)
	            {
	                /* Send callback to host */
	                /* Host is responsible for PNDV trigger */
	                PNIOext_cbf_im_data_flash_done(PNIO_OK, DatLen, prm->nv_data_type);
	            }
	            else
	            {
                    /* Send callback to host */
                    /* Host is responsible for PNDV trigger */
                    PNIOext_cbf_nv_data_flash_done(PNIO_OK, DatLen, prm->nv_data_type);
	            }
	    	}
	    	else
	    	{
	            if(prm->trigger_pndv == PNIO_TRUE)
	            {
					PNIO_ERR_STAT PnioStat;
					OsMemSet(&PnioStat, 0, sizeof(PNIO_ERR_STAT));

	                /* Trigger PNDV */
	                PNIO_trigger_pndv_ds_rw_done(&PnioStat, DatLen);
	            }
	    	}
#else
            if(prm->trigger_pndv == PNIO_TRUE)
			{
				PNIO_ERR_STAT PnioStat;
				OsMemSet(&PnioStat, 0, sizeof(PNIO_ERR_STAT));

                /* Trigger PNDV */
                PNIO_trigger_pndv_ds_rw_done(&PnioStat, DatLen);
            }
#endif

	        OsFree(prm);
	    }

	}

	PNIO_VOID Bsp_nv_data_factory_reset_flash_done ( PNIO_VOID* arg, PNIO_UINT32 DatLen )
	{
	    PNIO_printf ("***Bsp_nv_data_store: Completed ***\n" );

#if(0 == IOD_USED_WITH_XHIF_HOST)
	    OsReboot();
#else
	    /* Send callback to host */
	    PNIOext_cbf_nv_data_factory_reset_flash_done(PNIO_OK, DatLen);
#endif
	    return;
	}

    // *----------------------------------------------------------------*
    // *
    // *  SendToNvDataTask ()
    // *
    // *----------------------------------------------------------------*
    // *  sends a "store to flash" request to the low prior flash handler
    // *  task   (TskId_NvData)
    // *  Input:       ----
    // *
    // *  Output:      return               PNIO_OK, PNIO_NOT_OK
    // *
    // *----------------------------------------------------------------*
    static PNIO_UINT32 SendToNvDataTask (nv_function_handler handler, PNIO_VOID* arg)
    {
        PNIO_UINT32 Status = PNIO_OK;
        PNIO_UINT32 Error  = 0;


        ENTER_NV_DATA
        Status = OsFlashProgram(handler,
                                arg,
                                (PNIO_UINT8*) &NvData.CheckSum,          // source pointer
                                sizeof ( NV_DATA_STRUCT ),               // data length in bytes
                                &Error);                                 // return error value
        EXIT_NV_DATA

        if (Status != PNIO_OK)
        {
            PNIO_printf("OsFlashProgram(%s) error %d, ptr=0x%x, DatLen=%d: \n","REMA_SECTOR", Error, &NvData, sizeof ( NV_DATA_STRUCT ));
            return (PNIO_NOT_OK);
        }

        return (Status);
    }
	// *----------------------------------------------------------------*
	// *
	// *  NvDataSetDefaultValues (...)
	// *
	// *----------------------------------------------------------------*
	// *  sets all nvdata entries to default values (in ram mirror image)
	// *
	// *
	// *  Input:       pNvDat               ptr to struct NV_DATA_STRUCT
	// *  Output:      return               PNIO_OK
	// *
	// *----------------------------------------------------------------*
	static PNIO_VOID NvDataSetDefaultValues (PNIO_UINT32 NvDataTypes)
	{

		NV_DATA_STRUCT*   pNvDat = &NvData;

		PNIO_printf ("NvData : Set Default Values\n");

		// *** set default NV Data version ***
		pNvDat->Version = NV_DATA_VERSION;

		// *** set default MAC address  ***
        if (NvDataTypes & NV_TYP_MAC)
        {
		    OsMemCpy (  (PNIO_VOID*) &pNvDat->MacAddr[0],
					    (PNIO_VOID*) &DefaultMacAddr[0],
					    MAC_ADDR_LEN); // copy mac addr in mirror image
        }

		// *** set default IPSUITE ***
        if (NvDataTypes & NV_TYP_IPS)
        {
		    pNvDat->IpSuite.IpAddr      = DEVICE_IP_ADDRESS;
		    pNvDat->IpSuite.SubnetMask  = DEVICE_SUBNET_MASK;
		    pNvDat->IpSuite.DefRouter   = DEVICE_DEF_ROUTER;
        }

		// *** set default SNMP MIB2 data  ***
        if (NvDataTypes & NV_TYP_SNMP)
        {
		    pNvDat->SnmpSysNameLen = 0;
		    OsMemSet (&pNvDat->SnmpSysName[0], 0xFF, SNMP_SYSNAME_MAXLEN);
		    pNvDat->SnmpSysContLen = 0;
		    OsMemSet (&pNvDat->SnmpSysCont[0], 0xFF, SNMP_SYSCONT_MAXLEN);
		    pNvDat->SnmpSysLocLen  = 0;
		    OsMemSet (&pNvDat->SnmpSysLoc [0], 0xFF, SNMP_SYSLOC_MAXLEN);
        }

		// *** set default device name ***
        if (NvDataTypes & NV_TYP_NAME)
        {
		    pNvDat->DevNameLen = 0;
		    OsMemSet (&pNvDat->DevName[0], 0xFF, DEVICE_NAME_MAXLEN);
        }

		// *** set default private data ***
        if (NvDataTypes & NV_TYP_PRIV)
        {
		    pNvDat->PrivDataLen = 0;
		    OsMemSet(&pNvDat->PrivData[0], 0xFF, PRIV_DATA_MAXLEN);
        }

		// *** set default rema data ***
        if (NvDataTypes & NV_TYP_REMA)
        {
		    pNvDat->RemaDataLen = 0;
		    OsMemSet(&pNvDat->RemaData[0], 0xFF, REMA_DATA_MAXLEN);
        }

		// *** set default for IM1...IM4 ***
        if (NvDataTypes & NV_TYP_IM)
        {
        	LSA_UINT16 i;
        	for (i = 0; i< IOD_CFG_MAX_NUMOF_SUBSLOTS; i++){
        		//lint --e{866} Unusual use of '+' in argument to sizeof
        		OsMemSet (&pNvDat->IM1[i], ' ', sizeof (pNvDat->IM1[i])); // default value is ' '
        		OsMemSet (&pNvDat->IM2[i], ' ', sizeof (pNvDat->IM2[i])); // default value is ' '
				OsMemSet (&pNvDat->IM3[i], ' ', sizeof (pNvDat->IM3[i])); // default value is ' '
				OsMemSet (&pNvDat->IM4[i], 0,   sizeof (pNvDat->IM4[i])); // default value is 00        		
        	}
        }
        if (NvDataTypes & NV_TYP_ARFSU)
        {
        	OsMemSet ( &pNvDat->ARFSU[0], 0, sizeof( pNvDat->ARFSU ) );
        }
		// ******* compute checksum and store in mirror image ***
        pNvDat->CheckSum = device_compute_checksum
								((PNIO_CHAR*) &pNvDat->Version,
								 sizeof (NV_DATA_STRUCT) - sizeof (pNvDat->CheckSum));
#if(1 == IOD_USED_WITH_XHIF_HOST)
        PNIOext_cbf_nv_data_set_default(NvDataTypes);
#endif

	}

	// *----------------------------------------------------------------*
	// *
	// *  Bsp_nv_data_init ()
	// *
	// *----------------------------------------------------------------*
	// *
	// *
	// *  Input:       ----
	// *
	// *  Output:      return               PNIO_OK, PNIO_NOT_OK
	// *
	// *  startup function, must be called first
	// *----------------------------------------------------------------*
	// *----------------------------------------------------------------*
	PNIO_UINT32  Bsp_nv_data_init (PNIO_VOID* nvDataInit, PNIO_UINT32 nvDataLen)
	{
        PNIO_UINT32 CheckSum;
		PNIO_UINT32 Status = PNIO_OK;
        PNIO_UINT32 Error  = 0;
		NV_DATA_STRUCT* pNvDat = &NvData;

        // **** reset NV Data Structure ****
        OsMemSet (pNvDat, 0xFF, sizeof (NV_DATA_STRUCT));

#if(1 == IOD_USED_WITH_XHIF_HOST)
        /* If there no initial NV data, init Flash memory and load them */
        if(NULL == nvDataInit)
        {
#endif
		useErtecNVMem = PNIO_TRUE;

        /* Initialization non volatile memory */
        Status = OsFlashInit(NV_USERDATA_TOT_LEN, sizeof (NV_DATA_STRUCT));
		
		if(Status == PNIO_NOT_OK)
		{
			PNIO_printf("ERROR: An Error occured in OsFlashInit() function.\n ");
			return(Status);
		}
		// **** read all nv-data from the flash (Partition 1) ****

		Status = OsFlashRead(NV_USERDATA_SEC1,
							 (PNIO_UINT8*) &pNvDat->CheckSum,
							  sizeof (NV_DATA_STRUCT),
							  &Error);

		if(Status == PNIO_NOT_OK)
		{
			PNIO_printf("ERROR: An Error occured while reading all nv-data from the flash (Partition 1) in OsFlashRead() function.\n ");
			return(Status);
		}

        // **** compute new checksum ***
		CheckSum = device_compute_checksum
                        ((PNIO_CHAR*) &pNvDat->Version,
                        sizeof (NV_DATA_STRUCT) - sizeof (pNvDat->CheckSum));

		// **** if NVDATA version has changed then read from second partition ***
		if ((pNvDat->Version != NV_DATA_VERSION) || ( CheckSum != pNvDat->CheckSum ) )
		{
		    PNIO_printf ("Data from first section corrupted!\n");

            PNIO_printf ("Version : %x %x \n", pNvDat->Version, NV_DATA_VERSION);
            PNIO_printf ("Checksum : %x %x \n", pNvDat->CheckSum, CheckSum);

	        // **** read all nv-data from the flash (Partition 2) ****

	        Status = OsFlashRead(NV_USERDATA_SEC2,
							 (PNIO_UINT8*) &pNvDat->CheckSum,
							  sizeof (NV_DATA_STRUCT),
							  &Error);
			
			if(Status == PNIO_NOT_OK)
			{
				PNIO_printf("ERROR: An Error occured while reading all nv-data from the flash (Partition 2) in OsFlashRead() function.\n ");
				return(Status);
			}

			// **** compute new checksum ***
			CheckSum = device_compute_checksum
							((PNIO_CHAR*) &pNvDat->Version,
							sizeof (NV_DATA_STRUCT) - sizeof (pNvDat->CheckSum));

			// **** if NVDATA version has changed then restore default value ***
			if ((pNvDat->Version != NV_DATA_VERSION) || ( CheckSum != pNvDat->CheckSum ) )
			{
				PNIO_printf ("Data from second section corrupted, restoring default!\n");

				PNIO_printf ("Version : %x %x \n", pNvDat->Version, NV_DATA_VERSION);
				PNIO_printf ("Checksum : %x %x \n", pNvDat->CheckSum, CheckSum);
				NvDataSetDefaultValues (NVDATA_GROUP_INITIAL_ALL);

				SendToNvDataTask(&Bsp_nv_data_flash_done, (PNIO_VOID*) NULL);
			}
			else
			{
                nv_function_param* prm;
                OsAlloc((PNIO_VOID**) &prm, 0, sizeof(nv_function_param));

                prm->restore_first_section = PNIO_TRUE;

                PNIO_printf ("Using data from second section!\n");

                SendToNvDataTask(&Bsp_nv_data_flash_done, (PNIO_VOID*) prm);

                PNIO_printf ("Restoring first section!\n");
            }
		}
		else
		{
		    PNIO_printf ("Using data from first section!\n");
		}

#if(1 == IOD_USED_WITH_XHIF_HOST)
             /* Send data to host */
             PNIOext_cbf_nv_data_sync((PNIO_UINT8*) &pNvDat->CheckSum, sizeof (NV_DATA_STRUCT), 0);
        }
        else
        {
            useErtecNVMem = PNIO_FALSE;

            if(nvDataLen == sizeof (NV_DATA_STRUCT))
            {
                /* There are initial data - just copy them, don't use Flash memory */
                PNIO_printf ("Using data from host memory!\n");

                ENTER_NV_DATA
                /* Save data */
                OsMemCpy((PNIO_UINT8*) &pNvDat->CheckSum, (PNIO_UINT8*) nvDataInit, sizeof (NV_DATA_STRUCT));
                EXIT_NV_DATA

                /* Send confirmation to host */
				PNIOext_cbf_nv_data_sync((PNIO_UINT8*) &pNvDat->CheckSum, sizeof (NV_DATA_STRUCT), 0);
            }
            else
            {
                /* Send error to host */
                PNIOext_cbf_nv_data_sync((PNIO_UINT8*) NULL, 0, 1); /* errOccured = 1 */
                PNIO_printf ("Data from host memory corrupted (wrong length)!\n");
            }
        }
#endif

		return (Status);
	}



	// *----------------------------------------------------------------*
	// *
	// *  Bsp_nv_data_clear (...)
	// *
	// *----------------------------------------------------------------*
	// *
	// *
	// *  Input:       ----
	// *
	// *  Output:      return               PNIO_OK, PNIO_NOT_OK
	// *
	// *  sets all nvdata entries to factory preset values
	// *----------------------------------------------------------------*
	//lint -e{832, 578} Parameter 'Symbol' not explicitly declared, int assumed
	PNIO_UINT32  Bsp_nv_data_clear (PNIO_RTF_OPTION RtfOption)
	{
        PNIO_UINT32 Status = PNIO_OK;
        PNIO_UINT32 NvDataTypes = 0;
        nv_function_handler func;

        PNIO_printf ("***Bsp_nv_data_clear***\n");


        switch (RtfOption)
        {
            case     PNIO_RTF_RES_ALL:
                     PNIO_printf("##RESET all data to default values\n");
                     NvDataTypes = NVDATA_GROUP_DCP_ALL;
                     func = &Bsp_nv_data_factory_reset_flash_done;
                     break;

            case     PNIO_RTF_RES_COMM_PAR:
                     PNIO_printf("##RESET communication parameter\n");
                     NvDataTypes = NVDATA_GROUP_DCP_COMM;
                     func = &Bsp_nv_data_flash_done;
                     break;

                     /*reset nvdata after fw update via TCP*/
            case	PNIO_RTF_RES_FWUP:
            	    PNIO_printf("##RESET PNIO_RTF_RES_FWUP\n");
					NvDataTypes = ( NV_TYP_IPS | NV_TYP_SNMP | NV_TYP_PRIV | NV_TYP_REMA | NV_TYP_IM | NV_TYP_ARFSU );
					func = &Bsp_nv_data_flash_done;
					break;

            case    PNIO_RTF_RES_APPL_PAR:
                    PNIO_printf("##RESET application parameter\n");
                    NvDataTypes = 0x0000;
                    func = &Bsp_nv_data_flash_done;
                    break;

            case    PNIO_RTF_RES_ENG_PAR:
                    PNIO_printf("##RESET engineering parameter\n");
                    NvDataTypes = 0x0000;
                    func = &Bsp_nv_data_flash_done;
                    break;

            default: PNIO_printf(" unknown suboption at ResetToFactory service RtfOption=%x \n",RtfOption);
                     return (PNIO_NOT_OK);
        }

		// ***** set default values
		ENTER_NV_DATA
		NvDataSetDefaultValues (NvDataTypes);
		EXIT_NV_DATA

        /* If Flash memory is used, call flash thread
         * otherwise call callback, that data were stored successfully to NvData struct */
        if(useErtecNVMem == PNIO_TRUE)
        {
            nv_function_param* prm;
            OsAlloc((PNIO_VOID**) &prm, 0, sizeof(nv_function_param));

            prm->perform_host_callback = PNIO_TRUE;
            prm->im_callback = PNIO_FALSE;
            prm->trigger_pndv = PNIO_FALSE;
            prm->nv_data_type = NvDataTypes;

            SendToNvDataTask(func, (PNIO_VOID*) prm);
        }
        else
        {

            nv_function_param* prm;
            OsAlloc((PNIO_VOID**) &prm, 0, sizeof(nv_function_param));

            prm->perform_host_callback = PNIO_TRUE;
            prm->im_callback = PNIO_FALSE;
            prm->trigger_pndv = PNIO_FALSE;
            prm->nv_data_type = NvDataTypes;

            func((PNIO_VOID*) prm, 0);
        }

		return (Status);
	}

	/**
	* @brief Store I&M data
	*
	* @param[in]         NvDataType      		data type: devicename, ip-suite, pdev-records,...
	* @param[in]         *pMem        			record data
	* @param[in]         MemSize          		size of the shadow memory
	* @param[in]		 PeriphRealCfgInd		entity index in periph interface-real_cfg
	* @param[in]		 TriggerPNDV			PNIO_TRUE == trigger PNDV during callback
	*
	* @return            PNIO_OK             everything is valid
	*                    PNIO_NOT_OK         invalid state
	*
	* 	It stores the specified data in non volatile memory. Because
	*  	flash storing needs a lot of time, the function is executed
	*  	in another task context with low priority.
	*
	*/

	PNIO_UINT32  Bsp_im_data_store_prm
	(
		PNIO_NVDATA_TYPE    NvDataType,       // type of data (device name, IP suite, PDEV records,...)
		PNIO_VOID*          pMem,             // pointer to data source
		PNIO_UINT32         MemSize,          // size of memory to store
		LSA_UINT32			PeriphRealCfgInd, // entity index in periph interface-real_cfg
		PNIO_UINT32         TriggerPNDV       // size of memory to store
	)
	{

		NV_DATA_STRUCT* pNvDat = &NvData;
		PNIO_UINT32 Status = PNIO_OK;
		PNIO_UINT8* pDat = LSA_NULL;
		PNIO_UINT16 MaxMemSize;
		PNIO_UINT32 update = 0;

		PNIO_printf("***Bsp_im_data_store:NvDataType=%x***\n", NvDataType);

		// ******* plausibility check
		if ((pMem == (PNIO_VOID*)0) && (MemSize != 0))
		{
			PNIO_printf("Bsp_nv_data_store:PMem = NULL");
			return (PNIO_NOT_OK);
		}


		switch (NvDataType)
		{

		case PNIO_NVDATA_IM1:
			pDat = (PNIO_UINT8*)&pNvDat->IM1[PeriphRealCfgInd];
			MaxMemSize = sizeof(IM1_DATA);
			break;

		case PNIO_NVDATA_IM2:
			pDat = (PNIO_UINT8*)&pNvDat->IM2[PeriphRealCfgInd];
			MaxMemSize = sizeof(IM2_DATA);
			break;

		case PNIO_NVDATA_IM3:
			pDat = (PNIO_UINT8*)&pNvDat->IM3[PeriphRealCfgInd];
			MaxMemSize = sizeof(IM3_DATA);
			break;

		case PNIO_NVDATA_IM4:
			pDat = (PNIO_UINT8*)&pNvDat->IM4[PeriphRealCfgInd];
			MaxMemSize = sizeof(IM4_DATA);
			break;


		default:
			LSA_TRACE_00(TRACE_SUBSYS_IOD_MEMORY, LSA_TRACE_LEVEL_FATAL, "Bsp_nv_data_store:invalid NV-DATA type\n");
			return (PNIO_NOT_OK);
		}


		// **** check for maximum length ****
		if (MemSize > MaxMemSize)
		{
			PNIO_printf("error NV Data write: wrong data size\n");
			return (PNIO_NOT_OK);
		}


		// **** copy new data into mirror image
		if (MemSize)
		{
			if (OsMemCmp(pDat, pMem, MemSize))
			{
				ENTER_NV_DATA
					OsMemCpy(pDat, (PNIO_VOID*)pMem, MemSize);
				EXIT_NV_DATA
					update = 1;
			}
			else
			{
				update = 0;
			}
		}
		else
		{
			ENTER_NV_DATA
				OsMemSet(pDat, 0xFF, MaxMemSize);
			EXIT_NV_DATA
				update = 1;
		}



		if (update)
		{
			// **** compute new checksum ***
			pNvDat->CheckSum = device_compute_checksum
			((PNIO_CHAR*)&pNvDat->Version,
				sizeof(NV_DATA_STRUCT) - sizeof(pNvDat->CheckSum));

			/* If Flash memory is used, call flash thread
			 * otherwise call callback, that data were stored successfully to NvData struct */
			if (useErtecNVMem == PNIO_TRUE)
			{
				nv_function_param* prm;
				OsAlloc((PNIO_VOID**)&prm, 0, sizeof(nv_function_param));

				/* Callback from IM store, trigger PNDV in callback */
#if (IOD_INCLUDE_IM0_4 == 0)  // IM0..4 not handled inside PN stack--> do it here
				prm->perform_host_callback = PNIO_TRUE;
#else
					/* nv data handled in ERTEC - no need for callback */
				prm->perform_host_callback = PNIO_FALSE;
#endif
				prm->im_callback = PNIO_TRUE;
				prm->trigger_pndv = (PNIO_BOOL)TriggerPNDV;
				prm->nv_data_type = NvDataType;

				SendToNvDataTask(&Bsp_nv_data_flash_done, (PNIO_VOID*)prm);
			}
#if(1 == IOD_USED_WITH_XHIF_HOST)
			else
			{
#if (IOD_INCLUDE_IM0_4 == 0)  // IM0..4 not handled inside PN stack--> do it here
				nv_function_param* prm;
				OsAlloc((PNIO_VOID**)&prm, 0, sizeof(nv_function_param));

				/* Callback from IM store, trigger PNDV in callback */
				prm->im_callback = PNIO_TRUE;
				prm->trigger_pndv = PNIO_TRUE;
				prm->nv_data_type = NvDataType;

				Bsp_nv_data_flash_done((PNIO_VOID*)prm, MemSize);
#else
				/* nv data handled in BBB */
				PNIOext_cbf_im_data_store(NvDataType, pMem, MemSize, PeriphRealCfgInd, TriggerPNDV);
#endif
			}
#endif  /* #if(1 == IOD_USED_WITH_XHIF_HOST) */
		}
		else
		{
			nv_function_param* prm;
			OsAlloc((PNIO_VOID**)&prm, 0, sizeof(nv_function_param));

			/* Callback from IM store, trigger PNDV in callback */
#if (IOD_INCLUDE_IM0_4 == 0)  // IM0..4 not handled inside PN stack--> do it here
			prm->perform_host_callback = PNIO_TRUE;
#else
				/* nv data handled in ERTEC - no need for callback */
			prm->perform_host_callback = PNIO_FALSE;
#endif
			prm->im_callback = PNIO_TRUE;
			prm->trigger_pndv = (PNIO_BOOL)TriggerPNDV;
			prm->nv_data_type = NvDataType;

			/* No need to update data - send response OK */
			Bsp_nv_data_flash_done((PNIO_VOID*)prm, 0);
		}

		return (Status);
	}

	/**
	* @brief Store I&M data
	*
	* @param[in]         NvDataType      		data type: devicename, ip-suite, pdev-records,...
	* @param[in]         *pMem        			record data
	* @param[in]         MemSize          		size of the shadow memory
	* @param[in]		 PeriphRealCfgInd		entity index in periph interface-real_cfg
	*
	* @return            PNIO_OK             everything is valid
	*                    PNIO_NOT_OK         invalid state
	*
	* 	It stores the specified data in non volatile memory. Because
	*  	flash storing needs a lot of time, the function is executed
	*  	in another task context with low priority.
	*
	*/

	PNIO_UINT32  Bsp_im_data_store
	(
		PNIO_NVDATA_TYPE    NvDataType,     // type of data (device name, IP suite, PDEV records,...)
		PNIO_VOID*          pMem,           // pointer to data source
		PNIO_UINT32         MemSize,         // size of memory to store
		LSA_UINT32			PeriphRealCfgInd // entity index in periph interface-real_cfg
	)
	{
		return  Bsp_im_data_store_prm (NvDataType, pMem, MemSize, PeriphRealCfgInd, PNIO_TRUE);
	}

	/**
	* @brief Store I&M data
	*
	* @param[in]         NvDataType      		data type: devicename, ip-suite, pdev-records,...
	* @param[in]         *pMem        			record data
	* @param[in]         MemSize          		size of the shadow memory
	* @param[in]		 PeriphRealCfgInd		entity index in periph interface-real_cfg
	*
	* @return            PNIO_OK             everything is valid
	*                    PNIO_NOT_OK         invalid state
	*
	* 	It stores the specified data in non volatile memory. Because
	*  	flash storing needs a lot of time, the function is executed
	*  	in another task context with low priority.
	*
	*/

	PNIO_UINT32  Bsp_im_data_store_no_pndv_trigger
	(
		PNIO_NVDATA_TYPE    NvDataType,     // type of data (device name, IP suite, PDEV records,...)
		PNIO_VOID*          pMem,           // pointer to data source
		PNIO_UINT32         MemSize,         // size of memory to store
		LSA_UINT32			PeriphRealCfgInd // entity index in periph interface-real_cfg
	)
	{
		return  Bsp_im_data_store_prm(NvDataType, pMem, MemSize, PeriphRealCfgInd, PNIO_FALSE);
	}

	// *----------------------------------------------------------------*
	// *
	// *  Bsp_nv_data_store (...)
	// *
	// *----------------------------------------------------------------*
	// *  stores the specified data in non volatile memory. Because
	// *  flash storing needs a lot of time, the function is executed
	// *  in another task context with low priority.
	// *
	// *  Input:       PNIO_NV_DATA_TYPE    NvDataType       // datat type: devicename (zero terminated), ip-suite, pdev-records,...
	// *               PNIO_UINT8*          pMem             // record data
	// *               PNIO_UINT32,         MemSize         // size of the shadow memory
	// *
	// *  Output:      return               PNIO_OK, PNIO_NOT_OK
	// *
	// *
	// *----------------------------------------------------------------*
	PNIO_UINT32  Bsp_nv_data_store
	(
		PNIO_NVDATA_TYPE    NvDataType,     // type of data (device name, IP suite, PDEV records,...)
		PNIO_VOID*          pMem,           // pointer to data source
		PNIO_UINT32         MemSize         // size of memory to store
	)
	{

		NV_DATA_STRUCT* pNvDat = &NvData;
        PNIO_UINT32 Status = PNIO_OK;
		PNIO_UINT8* pDat = LSA_NULL;
		PNIO_UINT16 MaxMemSize;
		PNIO_UINT32 update = 0;
        PNIO_UINT16 prevSize = 0xffff;
		PNIO_BOOL call_callback = PNIO_OK;
	    PNIO_BOOL force_update = PNIO_NOT_OK;

		// ******* plausibility check
		if ((pMem == (PNIO_VOID*)0) && (MemSize != 0))
		{
			PNIO_printf ("Bsp_nv_data_store:PMem = NULL");
			return (PNIO_NOT_OK);
		}


		switch (NvDataType)
		{
			case PNIO_NVDATA_MAC_ADDR:
				pDat = (PNIO_UINT8* )&pNvDat->MacAddr[0];
				MaxMemSize = MAC_ADDR_LEN;
				break;

			case PNIO_NVDATA_DEVICENAME:
				pDat = (PNIO_UINT8* )&pNvDat->DevName[0];
				MaxMemSize = DEVICE_NAME_MAXLEN;
				if (MemSize <= MaxMemSize)
				{
	            	if(pNvDat->DevNameLen != MemSize)
	            	{
	            		/* If device name length does not previous - force update
	            		 * no matter what it contains */
	            		force_update = PNIO_OK;
	            	}
					pNvDat->DevNameLen = (PNIO_UINT16) MemSize;
				}
				break;

			case PNIO_NVDATA_IPSUITE:
				pDat = (PNIO_UINT8* )&pNvDat->IpSuite;
				MaxMemSize = sizeof (NV_IP_SUITE);
				break;

			case PNIO_NVDATA_SNMP_SYSNAME:
				pDat = (PNIO_UINT8* )&pNvDat->SnmpSysName[0];
				MaxMemSize = SNMP_SYSNAME_MAXLEN;
                if (MemSize == 0)
                {   // ****** erase object --> set default value ***
                    prevSize = pNvDat->SnmpSysNameLen;
		            pNvDat->SnmpSysNameLen = sizeof (DEFAULT_SNMP_SYSNAME) - 1;       // zero terminator of the string is not counted...
		            OsMemCpy (&pNvDat->SnmpSysName[0], DEFAULT_SNMP_SYSNAME, sizeof (DEFAULT_SNMP_SYSNAME));
                }
				else if (MemSize <= MaxMemSize)
                {   // ****** store object --> set new length value ***
                    prevSize = pNvDat->SnmpSysNameLen;
					pNvDat->SnmpSysNameLen = (PNIO_UINT16) MemSize;
				}
				break;

			case PNIO_NVDATA_SNMP_SYSCONT:
				pDat = (PNIO_UINT8* )&pNvDat->SnmpSysCont[0];
				MaxMemSize = SNMP_SYSCONT_MAXLEN;
				if (MemSize == 0)
                {   // ****** erase object --> set default value ***
                    prevSize = pNvDat->SnmpSysContLen;
		            pNvDat->SnmpSysContLen = sizeof (DEFAULT_SNMP_SYSCONT) - 1;       // zero terminator of the string is not counted...
		            OsMemCpy (&pNvDat->SnmpSysCont[0], DEFAULT_SNMP_SYSCONT, sizeof (DEFAULT_SNMP_SYSCONT));
                }
				else if (MemSize <= MaxMemSize)
                {   // ****** store object --> set new length value ***
                    prevSize = pNvDat->SnmpSysContLen;
					pNvDat->SnmpSysContLen = (PNIO_UINT16) MemSize;
				}
				break;

			case PNIO_NVDATA_SNMP_SYSLOC:
				pDat = (PNIO_UINT8* )&pNvDat->SnmpSysLoc[0];
				MaxMemSize = SNMP_SYSLOC_MAXLEN;
				if (MemSize == 0)
                {   // ****** erase object --> set default value ***
                    prevSize = pNvDat->SnmpSysLocLen;
		            pNvDat->SnmpSysLocLen  = sizeof (DEFAULT_SNMP_SYSLOC) - 1;       // zero terminator of the string is not counted...
		            OsMemCpy (&pNvDat->SnmpSysLoc[0], DEFAULT_SNMP_SYSLOC, sizeof (DEFAULT_SNMP_SYSLOC));
                }
				else if (MemSize <= MaxMemSize)
                {   // ****** store object --> set new length value ***
                    prevSize = pNvDat->SnmpSysLocLen;
					pNvDat->SnmpSysLocLen = (PNIO_UINT16) MemSize;
				}
				break;

			case PNIO_NVDATA_PDEV_RECORD:
				pDat = (PNIO_UINT8* )&pNvDat->RemaData[0];
				MaxMemSize = REMA_DATA_MAXLEN;
				if (MemSize <= MaxMemSize)
				{
					pNvDat->RemaDataLen = (PNIO_UINT16) MemSize;
				}
				call_callback = PNIO_NOT_OK;
				break;

			case PNIO_NVDATA_PRIVATE_DATA:
				pDat = (PNIO_UINT8* )&pNvDat->PrivData[0];
				MaxMemSize = PRIV_DATA_MAXLEN;
				if (MemSize <= MaxMemSize)
				{
					pNvDat->PrivDataLen = (PNIO_UINT16) MemSize;
				}
				break;

			case PNIO_NVDATA_ARFSU:
				pDat = (PNIO_UINT8* )&pNvDat->ARFSU[0];
				MaxMemSize = ARFSU_LEN;
				break;

			default:
				LSA_TRACE_00  (TRACE_SUBSYS_IOD_MEMORY, LSA_TRACE_LEVEL_FATAL, "Bsp_nv_data_store:invalid NV-DATA type\n");
				return (PNIO_NOT_OK);
		}


		// **** check for maximum length ****
		if (MemSize > MaxMemSize)
		{
			PNIO_printf ("error NV Data write: wrong data size\n");
			return (PNIO_NOT_OK);
		}


	    // **** copy new data into mirror image


		if (MemSize)
	    {
		    if ( OsMemCmp (pDat, pMem, MemSize) )
	        {
		        ENTER_NV_DATA
	            OsMemCpy (pDat, (PNIO_VOID*)pMem, MemSize);
		        EXIT_NV_DATA
	            update = 1;
	        }
		    else if(force_update == PNIO_OK)
		    {
		        ENTER_NV_DATA
	            OsMemCpy (pDat, (PNIO_VOID*)pMem, MemSize);
		        EXIT_NV_DATA
			    update = 1;
		    }
	        else
	        {
	            update = 0;
	        }
	    }
	    else
	    {
            if (prevSize != 0)
            {
                ENTER_NV_DATA
                OsMemSet ( pDat, 0xFF, MaxMemSize);
                EXIT_NV_DATA
                update = 1;
            }
        }


		if ( update )
	    {
		    // **** compute new checksum ***
	        pNvDat->CheckSum = device_compute_checksum
	                                ((PNIO_CHAR*) &pNvDat->Version,
	                                sizeof (NV_DATA_STRUCT) - sizeof (pNvDat->CheckSum));

	        PNIO_printf ("***Bsp_nv_data_store:NvDataType=%x, Size=%d, CheckSum: %x ***\n",NvDataType, MemSize, pNvDat->CheckSum );

            /* If Flash memory is used, call flash thread
             * otherwise call callback, that data were stored successfully to NvData struct */
            if(useErtecNVMem == PNIO_TRUE)
            {
                nv_function_param* prm;
                OsAlloc((PNIO_VOID**) &prm, 0, sizeof(nv_function_param));

                prm->perform_host_callback = PNIO_TRUE;
                prm->im_callback = PNIO_FALSE;
                prm->trigger_pndv = PNIO_FALSE;
                prm->nv_data_type = NvDataType;

                SendToNvDataTask(&Bsp_nv_data_flash_done, (PNIO_VOID*) prm);
            }
            else
            {
                if(call_callback == PNIO_OK)
                {
				
#if(1 == IOD_USED_WITH_XHIF_HOST)
                    // consider if other values may be synced also like SNMP at runtime ?? 
                    if  ((NvDataType == PNIO_NVDATA_SNMP_SYSNAME)||
                     	 (NvDataType == PNIO_NVDATA_SNMP_SYSCONT)||
                         (NvDataType == PNIO_NVDATA_SNMP_SYSLOC))
                    {
                     	PNIOext_cbf_nv_data_store(NvDataType, pMem, MemSize);
                    }
                    else
#endif
                    {
                	    nv_function_param* prm;
                        OsAlloc((PNIO_VOID**) &prm, 0, sizeof(nv_function_param));

                        prm->perform_host_callback = PNIO_TRUE;
                        prm->im_callback = PNIO_FALSE;
                        prm->trigger_pndv = PNIO_FALSE;
                        prm->nv_data_type = NvDataType;

                        /* Do not call callback in case of IP/MAC address and device name store
                         * Callback is performed by host functions */
                        Bsp_nv_data_flash_done((PNIO_VOID*) prm, MemSize);
                    }
                }
            }
		}
		else
		{
		    PNIO_printf ("***Bsp_nv_data_store:NvDataType=%x contained in NV memory ***\n",NvDataType );
		}

		return (Status);
	}


	/**
	* @brief Bsp_im_data_restore
	*
	* @param[in]         NvDataType      		data type: devicename, ip-suite, pdev-records,...
	* @param[in]         *pMemSize        		ptr to size of memory: in: buffer size, out: copied data size
	* @param[in]         **ppMem           		ptr to ptr to record data
	* @param[in]		 PeriphRealCfgInd		entity index in periph interface-real_cfg
	*
	* @return            PNIO_OK             everything is valid
	*                    PNIO_NOT_OK         invalid state
	*
	* It restores the im data from non volatile memory.If no
	*  valid data are available, default values are set.
	*
	*/


	PNIO_UINT32  Bsp_im_data_restore
				(
					PNIO_NVDATA_TYPE    NvDataType,     // type of data (device name, IP suite, PDEV records,...)
					PNIO_VOID**         ppMem,          // destination pointer
					PNIO_UINT32*        pMemSize,        // size of memory to restore
					LSA_UINT32			PeriphRealCfgInd // entity index in periph interface-real_cfg
				)
		{

			NV_DATA_STRUCT* pNvDat = &NvData;         // ram mirror image of NVDATA
			PNIO_UINT8*  pMem8;                       // allocated user memory, transferred to user
			PNIO_UINT8*  pDat;                        // source pointer for requested data in pNvDat
			PNIO_UINT16  MemSize;                     // size of data


			PNIO_printf ("***Bsp_im_data_restore:NvDataType=%x***\n",NvDataType);

			switch (NvDataType)
			{

				case PNIO_NVDATA_IM1:
					 pDat = (PNIO_UINT8* )&pNvDat->IM1[PeriphRealCfgInd];
					 MemSize = sizeof (IM1_DATA);
					 break;

				case PNIO_NVDATA_IM2:
					 pDat = (PNIO_UINT8* )&pNvDat->IM2[PeriphRealCfgInd];
					 MemSize = sizeof (IM2_DATA);
					 break;

				case PNIO_NVDATA_IM3:
					 pDat = (PNIO_UINT8* )&pNvDat->IM3[PeriphRealCfgInd];
					 MemSize = sizeof (IM3_DATA);
					 break;

				case PNIO_NVDATA_IM4:
					 pDat = (PNIO_UINT8* )&pNvDat->IM4[PeriphRealCfgInd];
					 MemSize = sizeof (IM4_DATA);
					 break;

				default:
					 LSA_TRACE_00  (TRACE_SUBSYS_IOD_MEMORY, LSA_TRACE_LEVEL_FATAL, "Bsp_nv_data_store:invalid NV-DATA type\n");
					 return (PNIO_NOT_OK);
			}

			// *** alloc user mem and copy specified entry from temp mem
			*pMemSize = MemSize;
			if (MemSize)
			{
				OsAlloc ((PNIO_VOID**)&pMem8, 0xff, MemSize);
				ENTER_NV_DATA
				OsMemCpy (pMem8, pDat, MemSize);
				*ppMem = pMem8;
				EXIT_NV_DATA
			}
			else
			{
				*ppMem = 0;
			}

			// pMem8 will be handled by the function Bsp_nv_data_memfree
			PNIO_printf ("*** done Bsp_im_data_restore  ***\n");
			return (PNIO_OK);
		}


	// *----------------------------------------------------------------*
	// *
	// *  Bsp_nv_data_restore (...)
	// *
	// *----------------------------------------------------------------*
	// *  restores the specified data from non volatile memory. If no
	// *  valid data are available, default values are set.
	// *  Bsp_nv_data_restore appends  a zero terminator to the devicename,
	// *  because profinet device name may not have that.
	// *
	// *  Input:       PNIO_NV_DATA_TYPE    NvDataType       // datat type: devicename, ip-suite, pdev-records,...
	// *               PNIO_UINT32*         pMemSize         // ptr to size of memory: in: buffer size, out: copied data size
	// *               PNIO_UINT8**         ppMem            // ptr to ptr to record data
	// *
	// *  Output:      return               PNIO_OK, PNIO_NOT_OK
	// *
	// *
	// *----------------------------------------------------------------*
	PNIO_UINT32  Bsp_nv_data_restore
			(
				PNIO_NVDATA_TYPE    NvDataType,     // type of data (device name, IP suite, PDEV records,...)
				PNIO_VOID**         ppMem,          // destination pointer
				PNIO_UINT32*        pMemSize        // size of memory to restore
			)
	{

		NV_DATA_STRUCT* pNvDat = &NvData;         // ram mirror image of NVDATA
		PNIO_UINT8*  pMem8;                       // allocated user memory, transferred to user
		PNIO_UINT8*  pDat;                        // source pointer for requested data in pNvDat
		PNIO_UINT16  MemSize;                     // size of data


		PNIO_printf ("***Bsp_nv_data_restore:NvDataType=%x***\n",NvDataType);



		switch (NvDataType)
		{
			case PNIO_NVDATA_MAC_ADDR:
				 pDat = (PNIO_UINT8* )&pNvDat->MacAddr[0];
				 MemSize = MAC_ADDR_LEN;
				 break;

			case PNIO_NVDATA_DEVICENAME:
				 pDat = (PNIO_UINT8* )&pNvDat->DevName[0];
				 MemSize = pNvDat->DevNameLen;
				 break;

			case PNIO_NVDATA_IPSUITE:
				 pDat = (PNIO_UINT8* )&pNvDat->IpSuite;
				 MemSize = sizeof (NV_IP_SUITE);
				 break;

			case PNIO_NVDATA_SNMP_SYSNAME:
				 pDat = (PNIO_UINT8* )&pNvDat->SnmpSysName[0];
				 MemSize = pNvDat->SnmpSysNameLen;
				 break;

			case PNIO_NVDATA_SNMP_SYSCONT:
				 pDat = (PNIO_UINT8* )&pNvDat->SnmpSysCont[0];
				 MemSize = pNvDat->SnmpSysContLen;
				 break;

			case PNIO_NVDATA_SNMP_SYSLOC:
				 pDat = (PNIO_UINT8* )&pNvDat->SnmpSysLoc[0];
				 MemSize = pNvDat->SnmpSysLocLen;
				 break;

			case PNIO_NVDATA_PDEV_RECORD:
				 pDat = (PNIO_UINT8* )&pNvDat->RemaData[0];
				 MemSize = pNvDat->RemaDataLen;
				 break;

			case PNIO_NVDATA_PRIVATE_DATA:
				 pDat = (PNIO_UINT8* )&pNvDat->PrivData[0];
				 MemSize = pNvDat->PrivDataLen;
				break;

			case PNIO_NVDATA_ARFSU:
				pDat = (PNIO_UINT8* )&pNvDat->ARFSU[0];
				MemSize = ARFSU_LEN;
				break;

			default:
				 LSA_TRACE_00  (TRACE_SUBSYS_IOD_MEMORY, LSA_TRACE_LEVEL_FATAL, "Bsp_nv_data_store:invalid NV-DATA type\n");
				 return (PNIO_NOT_OK);
		}

		// *** alloc user mem and copy specified entry from temp mem
		*pMemSize = MemSize;
		if (MemSize)
		{
			OsAlloc ((PNIO_VOID**)&pMem8, 0xff, MemSize);
			ENTER_NV_DATA
			OsMemCpy (pMem8, pDat, MemSize);
			*ppMem = pMem8;
			EXIT_NV_DATA
		}
		else
		{
			*ppMem = 0;
		}
		// pMem8 will be handled by the function Bsp_nv_data_memfree
		return (PNIO_OK);
	}

	// *----------------------------------------------------------------*
	// *
	// *  Bsp_nv_data_memfree (...)
	// *
	// *----------------------------------------------------------------*
	// *  stores the specified data in non volatile memory.
	// *
	// *  Input:       PNIO_VOID*           pMem             // pointer to alocated memory
	// *
	// *  Output:      return               PNIO_OK, PNIO_NOT_OK
	// *
	// *
	// *----------------------------------------------------------------*
	PNIO_UINT32  Bsp_nv_data_memfree
			(
				PNIO_VOID*          pMem           // pointer to data source
			)
    {
		if(pMem)
		{
			return OsFree (pMem);
		}
		return (PNIO_OK);
    }





/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
