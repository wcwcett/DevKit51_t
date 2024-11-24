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

#include "pnpb_lib.h"
#include "pnpb_lib_acyc.h"
#include "usriod_im_func.h"

// *********** some default settings *******
#define DEFAULT_DEVICENAME      ""
#define DEVICE_IP_ADDRESS       0x0
#define DEVICE_SUBNET_MASK      0x0
#define DEVICE_DEF_ROUTER       0x0

#define DEFAULT_MAC_ADDR        {0x08, 0x00, 0x06, 0x02, 0x01, 0x10}

#define DEFAULT_SNMP_SYSNAME    ""
#define DEFAULT_SNMP_SYSCONT    ""
#define DEFAULT_SNMP_SYSLOC     ""


// ***** NV data object types ***
#define NV_TYP_MAC              0x0001
#define NV_TYP_NAME             0x0002
#define NV_TYP_IPS              0x0004
#define NV_TYP_IM               0x0008
#define NV_TYP_SNMP             0x0010
#define NV_TYP_REMA             0x0020
#define NV_TYP_PRIV             0x0040
#define NV_TYP_ARFSU            0x0080


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
typedef PNIO_VOID (*nv_function_handler)(PNIO_UINT32, PNIO_UINT32, PNIO_UINT32);


// ******* static data ********
static const PNIO_UINT8     DefaultMacAddr[MAC_ADDR_LEN] = DEFAULT_MAC_ADDR;

static       NV_DATA_STRUCT NvData;
static       PNIO_BOOL useFlashMem = PNIO_FALSE;

/* Semaphore handles */
sem_t SemErtecNVMem;
static PNIO_UINT32 LastStat;

#if (USE_ERTEC_NV_MEM == 0)
#define NV_DATA_FILE_NAME    "nv_data.dat"

PNIO_UINT32 SaveDataToFlashMem(PNIO_VOID);
static PNIO_UINT32 RestoreDataFromFlashMem(PNIO_VOID);
#endif

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
static unsigned long device_compute_checksum(unsigned char* pMem, int size)
{
   unsigned long cksum = 0;
   int i;

   /* Sum all the bytes together */
   for (i = 0; i < size; i++)
   {
      cksum += *(pMem + i);
   }

   /* Do a little shuffling */
   cksum = (cksum >> 16) + (cksum & 0xffff);
   cksum += (cksum >> 16);

   /* Return the bitwise complement of the resulting mishmash */
   return(unsigned long)(~cksum);
}

PNIO_VOID Bsp_nv_data_flash_done ( PNIO_UINT32 Status, PNIO_UINT32 DatLen, PNIO_UINT32 nvDataType )
{
    printf ("***Bsp_nv_data_store: Completed ***\n" );

    if(Status != PNIO_OK)
    {
        printf ("***Bsp_nv_data_store: Error returned from ERTEC ***\n" );
    }
}

PNIO_VOID Bsp_im_data_flash_done ( PNIO_UINT32 Status, PNIO_UINT32 DatLen, PNIO_UINT32 nvDataType )
{
    PNIO_UINT32 IMidx;

    printf ("***Bsp_im_data_store: Completed ***\n" );

    if(Status != PNIO_OK)
    {
        printf ("***Bsp_im_data_store: Error returned from ERTEC ***\n" );
    }

    switch (nvDataType)
    {
        case PNIO_NVDATA_IM1:
            IMidx = 1;
            break;

        case PNIO_NVDATA_IM2:
            IMidx = 2;
            break;

        case PNIO_NVDATA_IM3:
            IMidx = 3;
            break;

        case PNIO_NVDATA_IM4:
            IMidx = 4;
            break;

        default:
            return;
    }

    ImX_write_Handler_done (IMidx, Status, DatLen);
}

PNIO_VOID Bsp_nv_data_factory_reset_flash_done ( PNIO_UINT32 Status, PNIO_UINT32 DatLen, PNIO_UINT32 nvDataType )
{
    printf ("*** Bsp_nv_data_reset:factory_reset_flash_done, nvDataType= %d ***\n" ,nvDataType);

    if(Status != PNIO_OK)
    {
        printf ("***Bsp_nv_data_reset: Error returned from ERTEC ***\n" );
    }
    else
    {
        printf ("***Bsp_nv_data_reset: Reboot to apply changes ***\n" );
    }
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
PNIO_VOID NvDataSetDefaultValues (PNIO_UINT32 NvDataTypes)
{
    NV_DATA_STRUCT*   pNvDat = &NvData;

    printf ("NvData : Set Default Values\n");

    // *** set default NV Data version ***
    pNvDat->Version = NV_DATA_VERSION;

    // *** set default MAC address  ***
    if (NvDataTypes & NV_TYP_MAC)
    {
        memcpy (  (PNIO_VOID*) &pNvDat->MacAddr[0],
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
        memset (&pNvDat->SnmpSysName[0], 0xFF, SNMP_SYSNAME_MAXLEN);
        pNvDat->SnmpSysContLen = 0;
        memset (&pNvDat->SnmpSysCont[0], 0xFF, SNMP_SYSCONT_MAXLEN);
        pNvDat->SnmpSysLocLen  = 0;
        memset (&pNvDat->SnmpSysLoc [0], 0xFF, SNMP_SYSLOC_MAXLEN);
    }

    // *** set default device name ***
    if (NvDataTypes & NV_TYP_NAME)
    {
        pNvDat->DevNameLen = 0;
        memset (&pNvDat->DevName[0], 0xFF, DEVICE_NAME_MAXLEN);
    }

    // *** set default private data ***
    if (NvDataTypes & NV_TYP_PRIV)
    {
        pNvDat->PrivDataLen = 0;
        memset(&pNvDat->PrivData[0], 0xFF, PRIV_DATA_MAXLEN);
    }

    // *** set default rema data ***
    if (NvDataTypes & NV_TYP_REMA)
    {
        pNvDat->RemaDataLen = 0;        
        memset(&pNvDat->RemaData[0], 0xFF, REMA_DATA_MAXLEN);
    }

    // *** set default for IM1...IM4 ***
    if (NvDataTypes & NV_TYP_IM)
    {
        PNIO_UINT16 i;
        for (i = 0; i< IOD_CFG_MAX_NUMOF_SUBSLOTS; i++){
            //lint --e{866} Unusual use of '+' in argument to sizeof
            memset (&pNvDat->IM1[i], ' ', sizeof (pNvDat->IM1[i])); // default value is ' '
            memset (&pNvDat->IM2[i], ' ', sizeof (pNvDat->IM2[i])); // default value is ' '
            memset (&pNvDat->IM3[i], ' ', sizeof (pNvDat->IM3[i])); // default value is ' '
            memset (&pNvDat->IM4[i], 0,   sizeof (pNvDat->IM4[i])); // default value is 00
        }


    }
    if (NvDataTypes & NV_TYP_ARFSU)
    {
        memset ( &pNvDat->ARFSU[0], 0, sizeof( pNvDat->ARFSU ) );
    }

    // ******* compute checksum and store in mirror image ***
    pNvDat->CheckSum = device_compute_checksum
                            ((unsigned char*) &pNvDat->Version,
                             sizeof (NV_DATA_STRUCT) - sizeof (PNIO_UINT32));
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
PNIO_UINT32  Bsp_nv_data_init (PNIO_VOID)
{
    PNIO_UINT32 CheckSum;
    PNIO_UINT32 Status = PNIO_OK;
    PNIO_UINT32 Error  = 0;
    NV_DATA_STRUCT* pNvDat = &NvData;
    PNIO_UINT32 MacAddrLen;
    PNIO_UINT32 IpSuiteLen;
    PNIO_UINT32 DevNameLen;
    PNIO_UINT8* tmpIpSuite;
    PNIO_UINT8* tmpMacAddr;
    PNIO_UINT8* tmpDevName;

    /* Init semaphore before use - load and start of firmware in Ertec will trigger it */
    sem_init(&SemErtecNVMem, 0, 0);

    // **** reset NV Data Structure ****
    memset (pNvDat, 0xFF, sizeof (NV_DATA_STRUCT));

#if (USE_ERTEC_NV_MEM == 1)
    /* Call ERTEC callback to init ERTEC nv memory */
    PNIOext_nv_data_init(NULL, 0);
#else
    /* Load nv data from file */
    Status = RestoreDataFromFlashMem();
    if(Status != PNIO_OK)
    {
        /* Restore defaults */
        printf ("NV data loading error! Restoring defaults!\n");
        NvDataSetDefaultValues (NVDATA_GROUP_INITIAL_ALL);

        Status = SaveDataToFlashMem();
        if(Status != PNIO_OK)
        {
            printf ("NV data saving error!\n");
        }
    }

    // **** compute new checksum ***
    CheckSum = device_compute_checksum
                     ((unsigned char*) &pNvDat->Version,
                     sizeof (NV_DATA_STRUCT) - sizeof (PNIO_UINT32));

    // **** if NVDATA version has changed then restore default value ***
    if ((pNvDat->Version != NV_DATA_VERSION) || ( CheckSum != pNvDat->CheckSum ) )
    {
        /* Restore defaults */
        printf ("NV data loading error! Restoring defaults!\n");
        NvDataSetDefaultValues (NVDATA_GROUP_INITIAL_ALL);

        Status = SaveDataToFlashMem();
        if(Status != PNIO_OK)
        {
            printf ("NV data saving error!\n");
        }
    }

    /* Call ERTEC callback to transfer nv memory */
    PNIOext_nv_data_init((PNIO_UINT8*) &pNvDat->CheckSum, sizeof (NV_DATA_STRUCT));
#endif

    /* Wait for semaphore in sync function */
    sem_wait(&SemErtecNVMem);

    return (LastStat);
}

// *----------------------------------------------------------------*
// *
// *  Bsp_nv_data_sync ()
// *
// *----------------------------------------------------------------*
// *
// *
// *  Input:       ----
// *
// *  Output:      return               PNIO_OK, PNIO_NOT_OK
// *
// *  save data downloaded from Ertec
// *----------------------------------------------------------------*
// *----------------------------------------------------------------*
PNIO_UINT32  Bsp_nv_data_sync (PNIO_VOID* nvData, PNIO_UINT32 nvSize, PNIO_UINT32 errOccured)
{
    if(errOccured == 1)
    {
        fprintf(stderr, "Bsp_nv_data_sync: error reported from ERTEC!\n");
        PNPB_LIB_FATAL
    }

    /* If nvSize == 0, NvData were previously filled by init function
     * and therefore data are valid */
    if(nvSize != 0)
    {
        memcpy ( (PNIO_VOID*) &NvData.CheckSum,
                            nvData,
                            nvSize);
    }

    /* Give semaphore to waiting request */
    sem_post(&SemErtecNVMem);

    return (PNIO_OK);
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
PNIO_UINT32  Bsp_nv_data_clear (PNIO_RTF_OPTION RtfOption)
{
    PNIO_UINT32 Status = PNIO_OK;
    PNIO_UINT32 NvDataTypes = 0;
    nv_function_handler func;

    printf ("***Bsp_nv_data_clear***\n");

#if (USE_ERTEC_NV_MEM == 1)
    /* Call ERTEC callback */
    PNIOext_nv_data_clear(RtfOption);

#else
    switch (RtfOption)
    {
        case     PNIO_RTF_RES_ALL:
                 printf ("##RESET all data to default values\n");
                 NvDataTypes = NVDATA_GROUP_DCP_ALL;
                 func = &Bsp_nv_data_factory_reset_flash_done;
                 break;

        case     PNIO_RTF_RES_COMM_PAR:
                 printf ("##RESET communication parameter\n");
                 NvDataTypes = NVDATA_GROUP_DCP_COMM;
                 func = &Bsp_nv_data_flash_done;
                 break;

                 /*reset nvdata after fw update via TCP*/
        case    PNIO_RTF_RES_FWUP:
                printf ("##RESET PNIO_RTF_RES_FWUP\n");
                NvDataTypes = ( NV_TYP_IPS | NV_TYP_SNMP | NV_TYP_PRIV | NV_TYP_REMA | NV_TYP_IM | NV_TYP_ARFSU );
                func = &Bsp_nv_data_flash_done;
                break;

        case    PNIO_RTF_RES_APPL_PAR:
                printf ("##RESET application parameter\n");
                NvDataTypes = 0x0000;
                func = &Bsp_nv_data_flash_done;
                break;
				
        case    PNIO_RTF_RES_ENG_PAR:
                printf ("##RESET engineering parameter\n");
                NvDataTypes = 0x0000;
                func = &Bsp_nv_data_flash_done;
                break;

        default: printf("unknown suboption at ResetToFactory service RtfOption=%x \n",RtfOption);
                 return (PNIO_NOT_OK);
    }

    // ***** set default values
    NvDataSetDefaultValues (NvDataTypes);

    // ***** Save data locally
    Status = SaveDataToFlashMem();

    // ***** Call callback
    func(Status, 0, 0);

#endif

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
        PNIO_NVDATA_TYPE    NvDataType,      // type of data (device name, IP suite, PDEV records,...)
        void*               pMem,            // pointer to data source
        PNIO_UINT32         MemSize,         // size of memory to store
        PNIO_UINT32			PeriphRealCfgInd // entity index in periph interface-real_cfg
    )
    {
        NV_DATA_STRUCT* pNvDat = &NvData;
        PNIO_UINT32 Status = PNIO_OK;
        PNIO_UINT8* pDat = NULL;
        PNIO_UINT16 MaxMemSize;
        PNIO_UINT32 update = 0;
        PNIO_NVDATA_TYPE NvDataTypeTmp = NvDataType & 0xFF;
        PNIO_UINT32 triggerPNDV = ( NvDataType >> 24 ) & 0x1;

        printf ("***Bsp_im_data_store:NvDataType=%x***\n", NvDataTypeTmp);

        // ******* plausibility check
        if ((pMem == (void*)0) && (MemSize != 0))
        {
            printf ("Bsp_im_data_store:PMem = NULL\n");
            return (PNIO_NOT_OK);
        }

        switch (NvDataTypeTmp)
        {

            case PNIO_NVDATA_IM1:
                pDat = (PNIO_UINT8* )&pNvDat->IM1[PeriphRealCfgInd];
                MaxMemSize = sizeof (IM1_DATA);
                break;

            case PNIO_NVDATA_IM2:
                pDat = (PNIO_UINT8* )&pNvDat->IM2[PeriphRealCfgInd];
                MaxMemSize = sizeof (IM2_DATA);
                break;

            case PNIO_NVDATA_IM3:
                pDat = (PNIO_UINT8* )&pNvDat->IM3[PeriphRealCfgInd];
                MaxMemSize = sizeof (IM3_DATA);
                break;

            case PNIO_NVDATA_IM4:
                pDat = (PNIO_UINT8* )&pNvDat->IM4[PeriphRealCfgInd];
                MaxMemSize = sizeof (IM4_DATA);
                break;


            default:
                printf  ("Bsp_im_data_store:invalid NV-DATA type\n");
                return (PNIO_NOT_OK);
        }


        // **** check for maximum length ****
        if (MemSize > MaxMemSize)
        {
            printf ("error NV Data write: wrong data size\n");
            return (PNIO_NOT_OK);
        }


        // **** copy new data into mirror image
        if (MemSize)
        {
            if( (pMem != NULL) && (pDat != NULL) )
            {
                if ( memcmp (pDat, pMem, MemSize) )
                {
                    memcpy (pDat, (void*)pMem, MemSize);
                    update = 1;
                }
                else
                {
                    update = 0;
                }
            }
            else
            {
                printf ("Bsp_im_data_store: NULL Pointer\n");
                return (PNIO_NOT_OK);
            }
        }
        else
        {
            memset ( pDat, 0xFF, MaxMemSize);
            update = 1;
        }



        if ( update )
        {
            // **** compute new checksum ***
            pNvDat->CheckSum = device_compute_checksum
                                ((unsigned char*) &pNvDat->Version,
                                sizeof (NV_DATA_STRUCT) - sizeof (PNIO_UINT32));

#if (USE_ERTEC_NV_MEM == 1)
            /* Call ERTEC callback */
            PNIOext_im_data_store (NvDataTypeTmp, PeriphRealCfgInd, MemSize, pMem);

#else
            // ***** Save data locally
            Status = SaveDataToFlashMem();

            if (triggerPNDV)
            {
                // ***** Call callback
                Bsp_im_data_flash_done(Status, MemSize, NvDataTypeTmp);
            }
            else 
            {
                printf("***Bsp_im_data_store:NvDataType=%x***\n", NvDataTypeTmp);
            }
#endif
        }
        else
        {
            printf ("***Bsp_im_data_store:NvDataType=%x contained in NV memory ***\n", NvDataTypeTmp);
            
            if (triggerPNDV)
            {
                // ***** Call callback
                Bsp_im_data_flash_done(PNIO_OK, 0, NvDataTypeTmp);
            }
        }

        return (Status);
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
    void*               pMem,           // pointer to data source
    PNIO_UINT32         MemSize         // size of memory to store
)
{
    NV_DATA_STRUCT* pNvDat = &NvData;
    PNIO_UINT32 Status = PNIO_OK;
    PNIO_UINT8* pDat = NULL;
    PNIO_UINT16 MaxMemSize;
    PNIO_UINT32 update = 0;
    PNIO_BOOL call_callback = PNIO_OK;
    PNIO_BOOL force_update = PNIO_NOT_OK;

    // ******* plausibility check
    if ((pMem == (void*)0) && (MemSize != 0))
    {
        printf ("Bsp_nv_data_store:PMem = NULL");
        return (PNIO_NOT_OK);
    }

    switch (NvDataType)
    {
        case PNIO_NVDATA_MAC_ADDR:
            pDat = (PNIO_UINT8* )&pNvDat->MacAddr[0];
            MaxMemSize = MAC_ADDR_LEN;
            call_callback = PNIO_NOT_OK;
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
            call_callback = PNIO_NOT_OK;
            break;

        case PNIO_NVDATA_IPSUITE:
            pDat = (PNIO_UINT8* )&pNvDat->IpSuite;
            MaxMemSize = sizeof (NV_IP_SUITE);
            call_callback = PNIO_NOT_OK;
            break;

        case PNIO_NVDATA_SNMP_SYSNAME:
            pDat = (PNIO_UINT8* )&pNvDat->SnmpSysName[0];
            MaxMemSize = SNMP_SYSNAME_MAXLEN;
            if (MemSize == 0)
            {   // ****** erase object --> set default value ***
                pNvDat->SnmpSysNameLen = sizeof (DEFAULT_SNMP_SYSNAME) - 1;       // zero terminator of the string is not counted...
                memcpy (&pNvDat->SnmpSysName[0], DEFAULT_SNMP_SYSNAME, sizeof (DEFAULT_SNMP_SYSNAME));
            }
            else if (MemSize <= MaxMemSize)
            {   // ****** store object --> set new length value ***
                pNvDat->SnmpSysNameLen = (PNIO_UINT16) MemSize;
            }
            break;

        case PNIO_NVDATA_SNMP_SYSCONT:
            pDat = (PNIO_UINT8* )&pNvDat->SnmpSysCont[0];
            MaxMemSize = SNMP_SYSCONT_MAXLEN;
            if (MemSize == 0)
            {   // ****** erase object --> set default value ***
                pNvDat->SnmpSysContLen = sizeof (DEFAULT_SNMP_SYSCONT) - 1;       // zero terminator of the string is not counted...
                memcpy (&pNvDat->SnmpSysCont[0], DEFAULT_SNMP_SYSCONT, sizeof (DEFAULT_SNMP_SYSCONT));
            }
            else if (MemSize <= MaxMemSize)
            {   // ****** store object --> set new length value ***
                pNvDat->SnmpSysContLen = (PNIO_UINT16) MemSize;
            }
            break;

        case PNIO_NVDATA_SNMP_SYSLOC:
            pDat = (PNIO_UINT8* )&pNvDat->SnmpSysLoc[0];
            MaxMemSize = SNMP_SYSLOC_MAXLEN;
            if (MemSize == 0)
            {   // ****** erase object --> set default value ***
                pNvDat->SnmpSysLocLen  = sizeof (DEFAULT_SNMP_SYSLOC) - 1;       // zero terminator of the string is not counted...
                memcpy (&pNvDat->SnmpSysLoc[0], DEFAULT_SNMP_SYSLOC, sizeof (DEFAULT_SNMP_SYSLOC));
            }
            else if (MemSize <= MaxMemSize)
            {   // ****** store object --> set new length value ***
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
            printf  ("Bsp_nv_data_store:invalid NV-DATA type\n");
            return (PNIO_NOT_OK);
    }


    // **** check for maximum length ****
    if (MemSize > MaxMemSize)
    {
        printf ("error NV Data write: wrong data size\n");
        return (PNIO_NOT_OK);
    }


    // **** copy new data into mirror image


    if (MemSize)
    {
        if( (pDat != NULL) && (pMem != NULL) )
        {
            if ( memcmp (pDat, pMem, MemSize) )
            {
                memcpy (pDat, (void*)pMem, MemSize);
                update = 1;
            }
            else if(force_update == PNIO_OK)
            {
                memcpy (pDat, (void*)pMem, MemSize);
                update = 1;
            }
            else
            {
                update = 0;
            }
        }
        else
        {
            printf ("Bsp_nv_data_store: NULL Pointer\n");
            return (PNIO_NOT_OK);
        }
    }
    else
    {
        memset ( pDat, 0xFF, MaxMemSize);
        update = 1;
    }


    if ( update )
    {
        // **** compute new checksum ***
        pNvDat->CheckSum = device_compute_checksum
                                ((unsigned char*) &pNvDat->Version,
                                sizeof (NV_DATA_STRUCT) - sizeof (PNIO_UINT32));

        printf ("***Bsp_nv_data_store:NvDataType=%x, Size=%d, CheckSum: %x ***\n",NvDataType, MemSize, pNvDat->CheckSum );

#if (USE_ERTEC_NV_MEM == 1)
        /* Call ERTEC callback */
        PNIOext_nv_data_store (NvDataType, MemSize, pMem);

#else
        // ***** Save data locally
        Status = SaveDataToFlashMem();

        // ***** Call callback
        if(call_callback == PNIO_OK)
        {
            /* Do not call callback in case of IP/MAC address and device name store
             * Callback is performed by ERTEC functions */
            Bsp_nv_data_flash_done(Status, MemSize, NvDataType);
        }
#endif
    }
    else
    {
        printf ("***Bsp_nv_data_store:NvDataType=%x contained in NV memory ***\n",NvDataType );
        // ***** Call callback
        Bsp_nv_data_flash_done(PNIO_OK, 0, NvDataType);
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
                PNIO_UINT32			PeriphRealCfgInd // entity index in periph interface-real_cfg
            )
    {

        NV_DATA_STRUCT* pNvDat = &NvData;         // ram mirror image of NVDATA
        PNIO_UINT8*  pMem8;                       // allocated user memory, transferred to user
        PNIO_UINT8*  pDat;                        // source pointer for requested data in pNvDat
        PNIO_UINT16  MemSize;                     // size of data


        printf ("***Bsp_im_data_restore:NvDataType=%x***\n",NvDataType);

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
                 printf  ("Bsp_im_data_restore:invalid NV-DATA type\n");
                 return (PNIO_NOT_OK);
        }

        // *** alloc user mem and copy specified entry from temp mem
        *pMemSize = MemSize;
        if (MemSize)
        {
            pMem8 = (PNIO_UINT8*) calloc (MemSize, 1);
            if( (pMem8 != NULL) && (pDat != NULL) )
            {
                memcpy (pMem8, pDat, MemSize);
                *ppMem = pMem8;
            }
            else
            {
                printf ("Bsp_im_data_restore: NULL Pointer");
                return (PNIO_NOT_OK);
            }
        }
        else
        {
            *ppMem = 0;
        }

        // pMem8 will be handled by the function Bsp_nv_data_memfree
        printf ("*** done Bsp_im_data_restore  ***\n");
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


    printf ("***Bsp_nv_data_restore:NvDataType=%x***\n",NvDataType);



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
             printf  ("Bsp_nv_data_restore:invalid NV-DATA type\n");
             return (PNIO_NOT_OK);
    }

    // *** alloc user mem and copy specified entry from temp mem
    *pMemSize = MemSize;
    if (MemSize)
    {
        pMem8 = (PNIO_UINT8*) calloc (MemSize, 1);
        if( (pMem8 != NULL) && (pDat != NULL) )
        {
            memcpy (pMem8, pDat, MemSize);
            *ppMem = pMem8;
        }
        else
        {
            printf ("Bsp_nv_data_restore: NULL Pointer");
            return (PNIO_NOT_OK);
        }
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
            PNIO_VOID* pMem           // pointer to data source
        )
{
    if(pMem)
    {
        free (pMem);
    }
    return (PNIO_OK);
}

#if (USE_ERTEC_NV_MEM == 0)

// *----------------------------------------------------------------*
// *
// *  SaveDataToFlashMem (...)
// *
// *----------------------------------------------------------------*
// *  stores the data in non volatile memory.
// *
// *  Input:       PNIO_VOID
// *
// *  Output:      return               PNIO_OK, PNIO_NOT_OK
// *
// *
// *----------------------------------------------------------------*
PNIO_UINT32 SaveDataToFlashMem(PNIO_VOID)
{
    FILE* nv_data_file;
    size_t write_len;

    /* w or wb: Truncate to zero length or create file for writing. */
    nv_data_file = fopen(NV_DATA_FILE_NAME, "w");
    if (nv_data_file == NULL)
    {
        printf("Failed to create file with NV data!\n");
        return PNIO_NOT_OK;
    }

    /* Copy data to file */
    write_len = fwrite(&NvData, 1, sizeof(NV_DATA_STRUCT), nv_data_file);
    /* write_len equals the number of bytes transferred only when size of item is 1 */
    if(write_len != sizeof(NV_DATA_STRUCT))
    {
        printf("Wrong number of written bytes of NV data (was %dB, expected %dB)!\n", write_len, sizeof(NV_DATA_STRUCT));
        return PNIO_NOT_OK;
    }

    fflush(nv_data_file);
    fclose(nv_data_file);

    printf("NV data has been saved to SD-Card!\n");

    return PNIO_OK;
}

// *----------------------------------------------------------------*
// *
// *  RestoreDataFromFlashMem (...)
// *
// *----------------------------------------------------------------*
// *  restores the data from non volatile memory.
// *
// *  Input:       PNIO_VOID
// *
// *  Output:      return               PNIO_OK, PNIO_NOT_OK
// *
// *
// *----------------------------------------------------------------*
static PNIO_UINT32 RestoreDataFromFlashMem(PNIO_VOID)
{
    FILE* nv_data_file;
    size_t read_len;

    /* r: Opens a file for reading. The file must exist. */
    nv_data_file = fopen(NV_DATA_FILE_NAME, "r");
    if (nv_data_file == NULL)
    {
        printf("Failed to open file with NV data!\n");
        return PNIO_NOT_OK;
    }

    /* Seek to the beginning of the file */
    fseek(nv_data_file, 0, SEEK_SET);

    /* Read data from file */
    read_len = fread(&NvData, 1, sizeof(NV_DATA_STRUCT), nv_data_file);
    /* read_len equals the number of bytes transferred only when size of item is 1 */
    if(read_len != sizeof(NV_DATA_STRUCT))
    {
        printf("Wrong number of read bytes of NV data (was %dB, expected %dB)!\n", read_len, sizeof(NV_DATA_STRUCT));
        return PNIO_NOT_OK;
    }

    fclose(nv_data_file);

    return PNIO_OK;
}
#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
