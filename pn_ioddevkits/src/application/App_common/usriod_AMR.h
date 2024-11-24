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
/*  F i l e               &F: usriod_AMR.h                              :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  header file to usriod_AMR.c.                                             */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/

#ifndef _USR_IOD_AMR_H
#define _USR_IOD_AMR_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

/* Format of UUID: XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX */
typedef struct
{
    PNIO_UINT32              UUID_part_1;
    PNIO_UINT16              UUID_part_2;
    PNIO_UINT16              UUID_part_3;
    PNIO_UINT16              UUID_part_4;
    PNIO_UINT8               UUID_part_5_array[6];
} AMR_UUID;

typedef enum
{
    Asset_IEC_61158_6_10       = 0x0000,
    Asset_IEC_61131_9          = 0x0100,
    Asset_IEC_61158_Type_3     = 0x0101,
    Asset_IEC_61158_Type_6     = 0x0102,
    Asset_IEC_62026_2          = 0x0103,
    Asset_IEC_61158_Type_9     = 0x0104,
    Asset_IEC_61158_Type_2     = 0x0105,
    Asset_IEC_61158_Type_1     = 0x0106,
    Asset_EN_50325_4           = 0x0107,
    Asset_IEC_61158_Type_8     = 0x0108,
} OrganizationType;

typedef struct
{
    PNIO_UINT16                 DeviceSubID;
    PNIO_UINT16                 DeviceID;
    PNIO_UINT16                 VendorID;
    OrganizationType            Organization;
} AMR_AM_Device_Ident;

typedef struct
{
    PNIO_UINT8                  SWRevisionPrefix;
    PNIO_UINT8                  IM_SWRevision_Functional_Enhancement;
    PNIO_UINT8                  IM_SWRevision_Bug_Fix;
    PNIO_UINT8                  IM_SWRevision_Internal_Change;
} AMR_IMSoftwareRevision;

typedef struct
{
    PNIO_UINT8                  AMHardwareRevision[64];
    PNIO_UINT16                 IMHardwareRevision;
} AM_HardwareInformation;

typedef struct
{
    PNIO_UINT8                  AMSoftwareRevision[64];
    AMR_IMSoftwareRevision      IMSoftwareRevision;
} AM_FirmwareInformation;

typedef struct
{
    AM_HardwareInformation      hardwareInformation;
    AM_FirmwareInformation      firmwareInformation;
} AM_FullInformation;

typedef union
{
    AM_FullInformation          fullInformation;
    AM_HardwareInformation      hardwareInformation;
    AM_FirmwareInformation      firmwareInformation;
} AM_Type;

typedef enum
{
    Asset_ManagementData        = 0x0035,
    Asset_FullInformation       = 0x0036,
    Asset_HardwareInformation   = 0x0037,
    Asset_FirmwareInformation   = 0x0038
} AssetType;

typedef enum
{
    Asset_LocationLevelTree    = 0x01,
    Asset_LocationSlotNumber   = 0x02
} LocationType;

struct _AssetManagementBlock
{
    struct _AssetManagementBlock   *next_block;

    AMR_UUID                        IMUniqueIdentifier;
    PNIO_CHAR                       AMLocation[16];
    PNIO_CHAR                       IMAnnotation[64];
    PNIO_CHAR                       IMOrderID[64];
    PNIO_CHAR                       IMSerialNumber[16];
    AMR_AM_Device_Ident             AMDeviceIdent;
    PNIO_UINT16                     AMTypeIdentification;

    AssetType                       type;
    AM_Type                         AMInfo;
};

typedef struct _AssetManagementBlock * AMRBlock_ptr;
typedef struct _AssetManagementBlock   AMRBlock;

typedef struct
{
    PNIO_UINT16                 numOfBlocks;
    AMRBlock_ptr                block;
} AssetManagementData;

#define                 ASSET_FULL_INFORMATION_SIZE         0x0144
#define                 ASSET_HARDWARE_INFORMATION_SIZE     0x0100
#define                 ASSET_FIRMWARE_INFORMATION_SIZE     0x0100

PNIO_VOID               AMR_Init ( PNIO_VOID );
PNIO_UINT32             AMR_ResponseHandler ( PNIO_DEV_ADDR *pAddr, PNIO_UINT32 *pBufLen, PNIO_UINT8 *pBuffer, PNIO_ERR_STAT *pPnioState );

PNIO_VOID               AMRData_AddAssetManagementBlock ( AMRBlock_ptr block );
PNIO_UINT16             AMRData_GetSizeAssetRecord ( PNIO_VOID );
AMRBlock_ptr            AMRData_CreateAMRObject ( AssetType type );

PNIO_UINT8 *            AMRObject_GenerateIMUniqueIdentifier( PNIO_UINT32 instance, PNIO_UINT8 vendor_id, PNIO_UINT8 device_id );
PNIO_VOID               AMRObject_SetIMUniqueIdentifier(AMRBlock_ptr block, PNIO_UINT8 * uuid, PNIO_UINT32 size );
PNIO_VOID               AMRObject_SetAMLocation(AMRBlock_ptr block, LocationType type, PNIO_VOID *nodes, PNIO_UINT16 num );
PNIO_VOID               AMRObject_IMAnnotation(AMRBlock_ptr block, const PNIO_CHAR *InputData, PNIO_UINT32 length );
PNIO_VOID               AMRObject_SetIMOrderID(AMRBlock_ptr block, const PNIO_CHAR *InputData, PNIO_UINT32 length );
PNIO_VOID               AMRObject_SetIMSerialNumber(AMRBlock_ptr block, const PNIO_CHAR *InputData, PNIO_UINT32 length );
PNIO_VOID               AMRObject_SetAMDeviceIdentification ( AMRBlock_ptr block, PNIO_UINT16 DeviceID, PNIO_UINT16 VendorID, OrganizationType Organization, PNIO_UINT16 DeviceSubID );
PNIO_VOID               AMRObject_SetAMTypeIdentification ( AMRBlock_ptr block, PNIO_UINT16 AMTypeIdentification );
PNIO_VOID               AMRObject_SetAMSoftwareRevision( AMRBlock_ptr block, PNIO_UINT8 *InputData, PNIO_UINT32 size );
PNIO_VOID               AMRObject_SetAMHardwareRevision( AMRBlock_ptr block, PNIO_UINT8 *InputData, PNIO_UINT32 size );
PNIO_VOID               AMRObject_SetIMSoftwareRevision( AMRBlock_ptr block, PNIO_UINT8 prefix, PNIO_UINT8 revision, PNIO_UINT8 bugfix, PNIO_UINT8 internal );
PNIO_VOID               AMRObject_SetIMHardwareRevision( AMRBlock_ptr block, PNIO_UINT16 ImHardwareRevision );

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
