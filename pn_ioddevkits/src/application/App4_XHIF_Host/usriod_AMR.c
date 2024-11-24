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
/*  F i l e               &F: usriod_AMR.c                              :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  AMR asset management record handling                                     */
/*                                                                           */
/*                                                                           */
/*  THIS MODULE HAS TO BE MODIFIED BY THE PNIO USER                          */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/

#include "iod_cfg.h"

#if IOD_INCLUDE_AMR
#include "usriod_AMR.h"
#include <arpa/inet.h>

#define     IOD_INCLUDE_AMR_EXAMPLE_DATA        1

AssetManagementData *data;

PNIO_VOID ASSET_PUT16_HTON( PNIO_VOID * base, PNIO_UINT32 offset, PNIO_UINT16 val );
PNIO_VOID ASSET_PUT32_HTON( PNIO_VOID * base, PNIO_UINT32 offset, PNIO_UINT32 val );
PNIO_VOID ASSET_PUT8_HTON( PNIO_VOID * base, PNIO_UINT32 offset, PNIO_UINT8 val );

#if IOD_INCLUDE_AMR_EXAMPLE_DATA
PNIO_VOID AMR_AddExampleAMRBlocks ( PNIO_VOID );
#endif

/**
 * @brief Intialization of AMR Read record capability
 */
PNIO_VOID   AMR_Init ( PNIO_VOID )
{
    data = (AssetManagementData *) calloc(sizeof (AMRBlock), 1);

    if ( !data )
    {
        printf ("ERROR: AMR data is not initialized => zero Len !\n");
    }

#if IOD_INCLUDE_AMR_EXAMPLE_DATA
    AMR_AddExampleAMRBlocks();
#endif
}

#if IOD_INCLUDE_AMR_EXAMPLE_DATA
/**
 * @brief Generation of Example AMR blocks
 *
 */
PNIO_VOID AMR_AddExampleAMRBlocks ( PNIO_VOID )
{
    AMRBlock_ptr block1 = NULL;
    AMRBlock_ptr block2 = NULL;
    PNIO_UINT16 blockAMLocation1[] = { 0x0001, 0x0001, 0x0001, 0x0001 };
    PNIO_UINT16 blockAMLocation2[] = { 0x0002, 0x0001, 0x0002, 0x0001 };
    const PNIO_INT8 ExampleAnnotation[] = "Insert Description here";
    const PNIO_INT8 ExampleOrderID[] = "xxx-yyyy-zzzz";
    const PNIO_INT8 ExampleSerialNumber[] = "123456789012";

    block1 = AMRData_CreateAMRObject(Asset_FullInformation);
    AMRObject_SetIMUniqueIdentifier(block1,
                                    AMRObject_GenerateIMUniqueIdentifier(1, IOD_CFG_VENDOR_ID, IOD_CFG_DEVICE_ID ),
                                    sizeof (AMR_UUID) );
    AMRObject_SetAMLocation(block1, Asset_LocationSlotNumber, (PNIO_VOID *) blockAMLocation1, 4 );
    AMRObject_IMAnnotation(block1, (PNIO_INT8 *) ExampleAnnotation, sizeof (ExampleAnnotation) - 1);
    AMRObject_SetIMOrderID(block1, (PNIO_INT8 *) ExampleOrderID, sizeof(ExampleOrderID) - 1);
    AMRObject_SetIMSerialNumber(block1, (PNIO_INT8 *) ExampleSerialNumber, sizeof(ExampleSerialNumber) -1 );
    AMRObject_SetAMDeviceIdentification(block1, 0,0,0,0);
    AMRObject_SetAMTypeIdentification(block1, 0 );
    AMRObject_SetIMSoftwareRevision(block1, 'V', 0, 0, 0);
    AMRObject_SetIMHardwareRevision(block1, 0);

    block2 = AMRData_CreateAMRObject(Asset_FullInformation);
    AMRObject_SetIMUniqueIdentifier(block2,
                                    AMRObject_GenerateIMUniqueIdentifier(1, IOD_CFG_VENDOR_ID, IOD_CFG_DEVICE_ID ),
                                    sizeof (AMR_UUID) );
    AMRObject_SetAMLocation(block2, Asset_LocationSlotNumber, (PNIO_VOID *) blockAMLocation2, 4 );
    AMRObject_IMAnnotation(block2, (PNIO_INT8 *) ExampleAnnotation, sizeof (ExampleAnnotation) -1 );
    AMRObject_SetIMOrderID(block2, (PNIO_INT8 *) ExampleOrderID, sizeof(ExampleOrderID) - 1);
    AMRObject_SetIMSerialNumber(block2, (PNIO_INT8 *) ExampleSerialNumber, sizeof(ExampleSerialNumber) -1);
    AMRObject_SetAMDeviceIdentification(block2, 0,0,0,0);
    AMRObject_SetAMTypeIdentification(block2, 0 );
    AMRObject_SetIMSoftwareRevision(block2, 'V', 0, 0, 0);
    AMRObject_SetIMHardwareRevision(block2, 0);

    AMRData_AddAssetManagementBlock(block1);
    AMRData_AddAssetManagementBlock(block2);
}
#endif

/**
 * @brief Get Size of AMR data
 *
 * @return PNIO_UINT16  size of AMR data record in B
 *
 * */
PNIO_UINT16 AMRData_GetSizeAssetRecord ( PNIO_VOID )
{
    PNIO_UINT16             recordSize;
    AMRBlock_ptr            block = NULL;
    PNIO_UINT16             size;
    PNIO_UINT16             iblock;

    recordSize = 8;

    for ( block = data->block, iblock = 0; iblock < data->numOfBlocks ; iblock ++, block = block->next_block )
    {
        switch ( block->type )
        {
            case Asset_FullInformation:
                size = ASSET_FULL_INFORMATION_SIZE;
                break;
            case Asset_HardwareInformation:
                size = ASSET_HARDWARE_INFORMATION_SIZE;
                break;
            case Asset_FirmwareInformation:
                size = ASSET_FIRMWARE_INFORMATION_SIZE;
                break;
            default:
                return 0;
        }

        recordSize += ( size + 4 );
    }

    return recordSize;
}

/**
 * @brief Generate response on read request
 *
 * @param[out]      pBuffer     pointer of memory array where record will be stored
 * @param[in,out]   pBufLen     pointer of memory array length
 *
 * @return  PNIO_UINT32         PNIO_OK
 *                              PNIO_NOT_OK
 */
PNIO_UINT32   AMRData_GenerateAssetRecord ( PNIO_UINT8 *pBuffer, PNIO_UINT32 *pBufLen )
{
    PNIO_UINT32             Status = PNIO_OK;
    PNIO_UINT16             iblock;
    PNIO_UINT32             pos = 0, i;
    PNIO_UINT16             size;
    PNIO_UINT16             recordSize;
    PNIO_UINT8              *dst = pBuffer;
    AMRBlock_ptr            block = NULL;

    ASSET_PUT16_HTON(dst, 0, Asset_ManagementData);
    ASSET_PUT16_HTON(dst, 4, 0x0100);
    ASSET_PUT16_HTON(dst, 6, data->numOfBlocks);
    pos += 8;
    recordSize = 8;

    for ( block = data->block, iblock = 0; iblock < data->numOfBlocks ; iblock ++, block = block->next_block )
    {
        // block type
        ASSET_PUT16_HTON(dst, pos+0, block->type);

        switch ( block->type )
        {
            case Asset_FullInformation:
                size = ASSET_FULL_INFORMATION_SIZE;
                break;
            case Asset_HardwareInformation:
                size = ASSET_HARDWARE_INFORMATION_SIZE;
                break;
            case Asset_FirmwareInformation:
                size = ASSET_FIRMWARE_INFORMATION_SIZE;
                break;
            default:
                return PNIO_NOT_OK;
        }

        recordSize += ( size + 4 );

        // block size
        ASSET_PUT16_HTON(dst, pos+2, size);

        // block version
        ASSET_PUT16_HTON(dst, pos+4, 0x0100);

        // padding
        ASSET_PUT16_HTON(dst, pos+6, 0x0000);

        // IM UniqueIdentifier part 1
        ASSET_PUT32_HTON(dst, pos+8, block->IMUniqueIdentifier.UUID_part_1);

        // IM UniqueIdentifier part 2
        ASSET_PUT16_HTON(dst, pos+12, block->IMUniqueIdentifier.UUID_part_2);

        // IM UniqueIdentifier part 3
        ASSET_PUT16_HTON(dst, pos+14, block->IMUniqueIdentifier.UUID_part_3);

        // IM UniqueIdentifier part 4
        ASSET_PUT16_HTON(dst, pos+16, block->IMUniqueIdentifier.UUID_part_4);

        // calc offset
        pos += 18;

        //IM_UniqueIdentifier part 5
        for ( i = 0; i < 6; i++ )
        {
            ASSET_PUT8_HTON(dst, pos+i, block->IMUniqueIdentifier.UUID_part_5_array[i]);
        }
        // calc offset
        pos += 6;

        //AM Location part 5
        for ( i = 0; i < 16; i++ )
        {
            ASSET_PUT8_HTON(dst, pos+i, block->AMLocation[i]);
        }
        // calc offset
        pos += 16;

        // IM Annotation
        for ( i = 0; i < 64; i++ )
        {
            ASSET_PUT8_HTON(dst, pos+i, block->IMAnnotation[i]);
        }
        // calc offset
        pos += 64;

        // IM OrderID
        for ( i = 0; i < 64; i++ )
        {
            ASSET_PUT8_HTON(dst, pos+i, block->IMOrderID[i]);
        }
        // calc offset
        pos += 64;

        switch ( block->type )
        {
            case Asset_FullInformation:
                // AM Software Revision
                for ( i = 0; i < 64; i++ )
                {
                    ASSET_PUT8_HTON(dst, pos+i, block->AMInfo.fullInformation.firmwareInformation.AMSoftwareRevision[i]);
                }
                // AM Hardware Revision
                for ( i = 0; i < 64; i++ )
                {
                    ASSET_PUT8_HTON(dst, pos+64+i, block->AMInfo.fullInformation.hardwareInformation.AMHardwareRevision[i]);
                }
                // calc offset
                pos += 128;
                break;
            case Asset_HardwareInformation:
                // AM Hardware Revision
                for ( i = 0; i < 64; i++ )
                {
                    ASSET_PUT8_HTON(dst, pos+i, block->AMInfo.hardwareInformation.AMHardwareRevision[i]);
                }
                // calc offset
                pos += 64;
                break;
            case Asset_FirmwareInformation:
                // AM Software Revision
                for ( i = 0; i < 64; i++ )
                {
                    ASSET_PUT8_HTON(dst, pos+i, block->AMInfo.firmwareInformation.AMSoftwareRevision[i]);
                }
                // calc offset
                pos += 64;
                break;
            default:
                return PNIO_NOT_OK;
        }

        // IM Serial Number
        for ( i = 0; i < 16; i++ )
        {
            ASSET_PUT8_HTON(dst, pos+i, block->IMSerialNumber[i]);
        }
        // calc offset
        pos += 16;

        switch ( block->type )
        {
            case Asset_FullInformation:
                ASSET_PUT8_HTON(dst, pos+0, block->AMInfo.fullInformation.firmwareInformation.IMSoftwareRevision.SWRevisionPrefix);
                ASSET_PUT8_HTON(dst, pos+1, block->AMInfo.fullInformation.firmwareInformation.IMSoftwareRevision.IM_SWRevision_Functional_Enhancement);
                ASSET_PUT8_HTON(dst, pos+2, block->AMInfo.fullInformation.firmwareInformation.IMSoftwareRevision.IM_SWRevision_Bug_Fix);
                ASSET_PUT8_HTON(dst, pos+3, block->AMInfo.fullInformation.firmwareInformation.IMSoftwareRevision.IM_SWRevision_Internal_Change);
                pos += 4;
                break;
            case Asset_FirmwareInformation:
                ASSET_PUT8_HTON(dst, pos+0, block->AMInfo.firmwareInformation.IMSoftwareRevision.SWRevisionPrefix);
                ASSET_PUT8_HTON(dst, pos+1, block->AMInfo.firmwareInformation.IMSoftwareRevision.IM_SWRevision_Functional_Enhancement);
                ASSET_PUT8_HTON(dst, pos+2, block->AMInfo.firmwareInformation.IMSoftwareRevision.IM_SWRevision_Bug_Fix);
                ASSET_PUT8_HTON(dst, pos+3, block->AMInfo.firmwareInformation.IMSoftwareRevision.IM_SWRevision_Internal_Change);
                pos += 4;
                break;
            case Asset_HardwareInformation:
                // empty
                break;
            default:
                return PNIO_NOT_OK;
        }

        // AMDeviceIdent Organization
        ASSET_PUT16_HTON(dst, pos, block->AMDeviceIdent.Organization);

        // AMDeviceIdent VendorID
        ASSET_PUT16_HTON(dst, pos+2, block->AMDeviceIdent.VendorID);

        // AMDeviceIdent DeviceID
        ASSET_PUT16_HTON(dst, pos+4, block->AMDeviceIdent.DeviceID);

        // AMDeviceIdent DeviceSubID
        ASSET_PUT16_HTON(dst, pos+6, block->AMDeviceIdent.DeviceSubID);
        pos += 8;

        // AM Type Identification
        ASSET_PUT16_HTON(dst, pos, block->AMTypeIdentification);
        pos += 2;

        switch ( block->type )
        {
            case Asset_FullInformation:
                ASSET_PUT16_HTON(dst, pos, block->AMInfo.fullInformation.hardwareInformation.IMHardwareRevision);
                pos += 2;
                break;
            case Asset_HardwareInformation:
                ASSET_PUT16_HTON(dst, pos, block->AMInfo.hardwareInformation.IMHardwareRevision);
                pos += 2;
                break;
            case Asset_FirmwareInformation:
                break;
            default:
                return PNIO_NOT_OK;
        }
    }

    ASSET_PUT16_HTON(dst, 2, recordSize - 4 );

    if( recordSize < *pBufLen )
        *pBufLen = recordSize;

    return Status;
}

/**
 * @brief handler of AMR read request
 *
 * @param[in]       pAddr           location of request ( slot , subslot )
 * @param[in, out]  pBufLen         length to be read
 * @param[in]       pBuffer         buffer pointer
 * @param[out]      pPnioState      4 byte PNIO status
 *
 * @return          PNIO_UINT32     PNIO_OK
 *                                  PNIO_NOT_OK
 */
PNIO_UINT32  AMR_ResponseHandler
    (
      PNIO_DEV_ADDR         *pAddr,
      PNIO_UINT32           *pBufLen,
      PNIO_UINT8            *pBuffer,
      PNIO_ERR_STAT         *pPnioState
    )
{
    PNIO_UINT32 Status = PNIO_OK;
    PNIO_UINT16 Size;

    if ( ! data )
    {
        return PNIO_NOT_OK;
    }

    Size = AMRData_GetSizeAssetRecord();

    if ( Size > *pBufLen)
    {
        pPnioState->ErrCode   = 0xde;
        pPnioState->ErrDecode = 0x80;
        pPnioState->ErrCode1  = 0xb8;
        Status = PNIO_NOT_OK;
        printf("ERR! Size > *pBufLen\n");
    }
    else
    {
        printf("AMRData_GenerateAssetRecord\n");
        Status = AMRData_GenerateAssetRecord( pBuffer, pBufLen );
    }

    return Status;
}

/**
 * @brief Adition AMR block to AMR data
 *
 * @param[in]   block           block to be added
 *
 * @return      PNIO_VOID
 */
PNIO_VOID   AMRData_AddAssetManagementBlock ( AMRBlock_ptr block )
{
    AMRBlock_ptr next;

    if ( !data )
    {
        printf ("ERROR: AMR_CreateAssetManagementObject() => zero Len !\n");
        return;
    }

    if ( !block )
    {
        printf ("ERROR: AMR_CreateAssetManagementObject() => zero Len !\n");
        return;
    }

    if ( data ->numOfBlocks == 0 )
    {
        data->block = block;
    }
    else
    {
        next = data->block;
        while ( next->next_block != NULL)
        {
            next = next->next_block;
        }

        next->next_block = block;
    }

    data->numOfBlocks += 1;
}

/**
 * @brief Creation of AMR Object
 *
 * @param[in] type      Asset_FullInformation
 *                      Asset_HardwareInformation
 *                      Asset_FirmwareInformation
 *
 * @return    ptr of created object
 */
AMRBlock_ptr AMRData_CreateAMRObject ( AssetType type )
{
    AMRBlock_ptr block = NULL;

    switch ( type )
    {
        case Asset_FullInformation:
        case Asset_HardwareInformation:
        case Asset_FirmwareInformation:
            break;
        default:
            printf ( "ERROR ( AMR ) : Not enough memory for creation of asset management data\n");
            return block;
    }

    block = (AMRBlock_ptr) calloc(sizeof (AMRBlock), 1);

    if ( ! block )
    {
        printf ( "ERROR ( AMR ) : Not enough memory for creation of asset management data\n");
        return NULL;
    }

    block->type = type;

    return block;
}

/**
 * @brief Example of generating of UUID data in version 1 according to the RFC
 *
 * @param[in]   instance            instance of application
 * @param[in]   vendor_id           vendor ID of manufacturer
 * @param[in]   device_id           device ID of device
 *
 * @return      PNIO_UINT8 *
 */
PNIO_UINT8 * AMRObject_GenerateIMUniqueIdentifier( PNIO_UINT32 instance, PNIO_UINT8 vendor_id, PNIO_UINT8 device_id )
{
    PNIO_UINT64 time_uuid;
    PNIO_UINT16 clock_seq;
    AMR_UUID    *uuid;

    uuid = (AMR_UUID*) calloc(sizeof (AMR_UUID), 1);

    //get 100ns time from OS
    time_uuid = OsGetUuidTime();

    // clock seq
    clock_seq = ( PNIO_UINT16 ) rand();

    // setup UUID version 1
    uuid->UUID_part_1 = time_uuid & 0xffffffff;
    uuid->UUID_part_2 = ( time_uuid >> 32 ) & 0xffff;
    uuid->UUID_part_3 = (1 << 12) | ( ( time_uuid >> 48 ) & 0xfff );
    uuid->UUID_part_4 = ( ( (clock_seq >> 8) & 0x3f ) | ( clock_seq & 0xff ) );

    uuid->UUID_part_5_array[0] = vendor_id;
    uuid->UUID_part_5_array[1] = device_id;
    uuid->UUID_part_5_array[2] = ( instance >> 24 ) & 0xFF;
    uuid->UUID_part_5_array[3] = ( instance >> 16 ) & 0xFF;
    uuid->UUID_part_5_array[4] = ( instance >> 8 ) & 0xFF;
    uuid->UUID_part_5_array[5] = ( instance >> 0 ) & 0xFF;

    return (PNIO_UINT8 *) uuid;
}

/**
 * @brief Validy and Set to AssetBlock UUID data
 *
 * @param[out]  block       block where UUID will be stored
 * @param[in]   uuid        uuid which will be added to the block
 * @param[in]   size        sizeof uuid ( must be sizeof(AMR_UUID) )
 */
PNIO_VOID AMRObject_SetIMUniqueIdentifier(AMRBlock_ptr block, PNIO_UINT8 * uuid, PNIO_UINT32 size )
{
    PNIO_UINT8* tmp = uuid;

    if( !block )
    {
        printf ("ERROR: AMR_CreateAssetManagementObject() => zero Len !\n");
        return;
    }

    if ( !tmp )
    {
        printf("ERROR: AMR_CreateAssetManagementObject() => zero Len !\n");
        return;
    }

    if (size != sizeof(AMR_UUID) )
    {
        printf("ERROR: AMR_CreateAssetManagementObject() => zero Len !\n");
        return;
    }

    if ( (*tmp == 0) && ( memcmp(tmp, tmp + 1, size - 1) == 0 ) )
    {
        printf ( "ERROR ( AMR ) : zero value for UUID is reserved \n");
        return;
    }

    memcpy (& block->IMUniqueIdentifier.UUID_part_1, tmp, sizeof(block->IMUniqueIdentifier.UUID_part_1));
    tmp += sizeof(block->IMUniqueIdentifier.UUID_part_1);

    memcpy (& block->IMUniqueIdentifier.UUID_part_2, tmp, sizeof(block->IMUniqueIdentifier.UUID_part_2));
    tmp += sizeof(block->IMUniqueIdentifier.UUID_part_2);

    memcpy (& block->IMUniqueIdentifier.UUID_part_3, tmp, sizeof(block->IMUniqueIdentifier.UUID_part_3));
    tmp += sizeof(block->IMUniqueIdentifier.UUID_part_3);

    memcpy (& block->IMUniqueIdentifier.UUID_part_4, tmp, sizeof(block->IMUniqueIdentifier.UUID_part_4));
    tmp += sizeof(block->IMUniqueIdentifier.UUID_part_4);

    memcpy (& block->IMUniqueIdentifier.UUID_part_5_array[0], tmp, 6 * sizeof ( PNIO_UINT8) );

    return;
}

/**
 * @brief  Validy and Set Am Location in AMR block
 *
 * @param[out]  block       block where will be added
 * @param[in]   type        type of AM location Asset_LocationLevelTree, Asset_LocationSlotNumber
 * @param[in]   nodes       AM location data
 * @param[in]   num         number of AM location data
 */
PNIO_VOID AMRObject_SetAMLocation(AMRBlock_ptr block, LocationType type, PNIO_VOID *nodes, PNIO_UINT16 num )
{
    PNIO_UINT16 *dst = NULL;
    PNIO_UINT16 *src = NULL;
    int src_off = 0, dst_off = 0;
    int numBits = 0, shift = 0, mask = 0;

    if ( !block )
    {
        printf ("ERROR: (AMR) - NULL output asset management block for AM Location \n");
    }

    if ( !nodes )
    {
        printf ("ERROR: (AMR) - NULL input data for AM Location\n");
    }

    if ( ( type != Asset_LocationLevelTree ) && (type != Asset_LocationSlotNumber) )
    {
        printf ("ERROR: (AMR) - wrong type of AM Location \n");
    }

    if ( ( type == Asset_LocationLevelTree) && ( num > 12 ) )
    {
        printf ("ERROR: (AMR) - max number of nodes is 12 \n");
    }
    if ( ( type == Asset_LocationSlotNumber) && ( num != 4 ) )
    {
        printf ("ERROR: (AMR) - BeginSlotNumber BeginSubslotNumber EndSlotNumber EndSubslotNumber must be sent -> 4 \n");
    }

    block->AMLocation[15] = type;
    src = (PNIO_UINT16 *) nodes;

    switch ( type )
    {
        case Asset_LocationLevelTree:
            dst = (PNIO_UINT16 *) &block->AMLocation[13];

            while (src_off < num)
            {
                // find dst index
                dst_off = ( (src_off * 10) / 16 );
                shift = ( (src_off * 10) % 16 );
                numBits = ( (16 - shift) < 10) ? (16 - shift) : 10;
                mask = ( 1 << numBits ) - 1;

                if(dst_off <= 13)
                {
                    dst[13 - dst_off] |= (( src[src_off] & mask ) << shift );
                }
                if (numBits < 10)
                {
                    mask = ( 1 << ( 10 - numBits ) ) - 1;
                    if(dst_off < 13)
                    {
                        dst[13 - (dst_off+1)] |= ((src[src_off] & mask) >> shift );
                    }
                }

                src_off++;
            }
            break;
        case Asset_LocationSlotNumber:
            block->AMLocation[14] = 0; // Reserved 1

            dst = (PNIO_UINT16 *) &block->AMLocation[12];

            for (src_off = 0; src_off < num ; src_off ++ )
            {
                *(dst - src_off ) = htons( src[src_off] );
            }

            for (src_off = 0; src_off < 6; src_off ++ )
            {
                block->AMLocation[src_off] = 0;
            }
            break;
        default:
            // not possible to reach due to check of param
            break;
    }
}

/**
 * @brief Validy and Set of IM Annotation
 *
 * @param[out]  block         block where will be added
 * @param[in]   InputData     input data
 * @param[in]   length        length of data
 */
PNIO_VOID AMRObject_IMAnnotation(AMRBlock_ptr block, PNIO_INT8 *InputData, PNIO_UINT32 length )
{
    PNIO_UINT32 i;

    if ( !block )
    {
        printf ("ERROR: (AMR) - asset management block is NULL \n");
    }

    if ( !InputData )
    {
        printf ("ERROR: (AMR) - data is NULL \n");
    }

    if (length > 64 )
    {
        printf ("ERROR: (AMR) - length has to be shorter then 64B \n");
    }

    for ( i = 0; i < length ; i ++)
    {
        if ( ( InputData[i] >= 0x20 ) && ( InputData[i] <= 0x7E ) )
        {
            block->IMAnnotation[i] = InputData[i];
        }
        else
        {
            printf ("ERROR: (AMR) - character is not visible string \n");
            return;
        }
    }

    // If the text is shorter than the defined string length, the gap shall be filled with blanks.
    if ( length < 64 )
    {
        for ( i = length ; i < 64 ; i++ )
        {
            block->IMAnnotation[i] = ' ';
        }
    }
}

/**
 * @brief Validy and Set of IM Order ID
 *
 * @param[out]  block         block where will be added
 * @param[in]   InputData     input data
 * @param[in]   length        length of data
 */
PNIO_VOID AMRObject_SetIMOrderID(AMRBlock_ptr block, PNIO_INT8 *InputData, PNIO_UINT32 length )
{
    PNIO_UINT32 i;

    if ( !block )
    {
        printf ("ERROR: (AMR) - asset management block is NULL \n");
    }

    if ( !InputData )
    {
        printf ("ERROR: (AMR) - data is NULL \n");
    }

    if (length > 64 )
    {
        printf ("ERROR: (AMR) - length has to be shorter then 64B \n");
    }

    for ( i = 0; i < length ; i ++)
    {
        if ( ( InputData[i] >= 0x20 ) && ( InputData[i] <= 0x7E ) )
        {
            block->IMOrderID[i] = InputData[i];
        }
        else
        {
            printf ("ERROR: (AMR) - character is not visible string \n");
            return;
        }
    }

    // If the text is shorter than the defined string length, the gap shall be filled with blanks.
    if ( length < 64 )
    {
        for ( i = length ; i < 64 ; i++ )
        {
            block->IMOrderID[i] = ' ';
        }
    }
}

/**
 * @brief Validy and Set of IM Serial number
 *
 * @param[out]  block        block where will be added
 * @param[in]   InputData    Input data
 * @param[in]   length       length of data
 */
PNIO_VOID AMRObject_SetIMSerialNumber(AMRBlock_ptr block, PNIO_INT8 *InputData, PNIO_UINT32 length )
{
    PNIO_UINT32 i;

    if ( !block )
    {
        printf ("ERROR: (AMR) - asset management block is NULL \n");
    }

    if ( !InputData )
    {
        printf ("ERROR: (AMR) - data is NULL \n");
    }

    if ( length > 16 )
    {
        printf ("ERROR: (AMR) - length has to be shorter then 16B \n");
    }

    for ( i = 0; i < length ; i ++)
    {
        if ( ( InputData[i] >= 0x20 ) && ( InputData[i] <= 0x7E ) )
        {
            block->IMSerialNumber[i] = InputData[i];
        }
        else
        {
            printf ("ERROR: (AMR) - character is not visible string \n");
            return;
        }
    }

    // If the text is shorter than the defined string length, the gap shall be filled with blanks.
    if ( length < 16 )
    {
        for ( i = length ; i < 16 ; i++ )
        {
            block->IMSerialNumber[i] = ' ';
        }
    }
}

/**
 * @brief       Validy and set of AM device identification
 *
 * @param[out]  block           block where will be added
 * @param[in]   deviceID        id of device
 * @param[in]   VendorID        id of vendor
 * @param[in]   Organization    organization
 * @param[in]   DeviceSubID     id of devicesub
 */
PNIO_VOID AMRObject_SetAMDeviceIdentification ( AMRBlock_ptr block, PNIO_UINT16 DeviceID, PNIO_UINT16 VendorID, OrganizationType Organization, PNIO_UINT16 DeviceSubID )
{
    if ( !block )
    {
        printf ("ERROR: (AMR) - asset management block is NULL \n");
    }

    switch ( Organization )
    {
        case Asset_IEC_61158_6_10:
            if (DeviceSubID != 0)
            {
                printf ("ERROR: (AMR) - wrong device sub ID \n");
                return;
            }
            break;
        case Asset_IEC_61131_9:
        case Asset_IEC_61158_Type_3:
        case Asset_IEC_61158_Type_6:
        case Asset_IEC_62026_2:
        case Asset_IEC_61158_Type_9:
        case Asset_IEC_61158_Type_2:
        case Asset_IEC_61158_Type_1:
        case Asset_EN_50325_4:
        case Asset_IEC_61158_Type_8:
            break;
        default:
            printf ("ERROR: (AMR) - wrong organization ID \n");
            return;
    }

    block->AMDeviceIdent.DeviceID = DeviceID;
    block->AMDeviceIdent.DeviceSubID = DeviceSubID;
    block->AMDeviceIdent.Organization = Organization;
    block->AMDeviceIdent.VendorID = VendorID;
}

/**
 * @brief manufacturer assigned type identification
 *
 * @param[out] block                AssetManagementBlock for setting of data
 * @param[in]  AMTypeIdentification 0x0000 - Unspecified
 *                                  0x0001 - Standard Controller ( for example Programmable Logic Controller )
 *                                  0x0002 - PC-based Station
 *                                  0x0003 - IO-Module or IO-Submodule
 *                                  0x0004 - Communication Module or Communication Submodule
 *                                  0x0005 - Interface Module or Interface Submodule
 *                                  0x0006 - Active Network Infrastructure Component
 *                                  0x0007 - Media attachment unit ( for example an adapter to connect different Ethernet MAU types )
 *                                  0x0100 – 0x7FFF - Shall be defined by the manufacturer of the reporting entity
 *                                  other - Reserved
 *
 */
PNIO_VOID AMRObject_SetAMTypeIdentification ( AMRBlock_ptr block, PNIO_UINT16 AMTypeIdentification )
{
    if ( !block )
    {
        printf ("ERROR: (AMR) - asset management block is NULL \n");
        return;
    }

    if ( AMTypeIdentification >= 0x7FFF )
    {
        printf ("ERROR: (AMR) - AMTypeIdentification in reserved range \n");
        return;
    }

    block->AMTypeIdentification = AMTypeIdentification;
}

/**
 * @brief Validy and Set of AM Software Revision
 *
 * @param[out]  block         block where will be added
 * @param[in]   InputData     input data
 * @param[in]   length        length of data
 */
PNIO_VOID AMRObject_SetAMSoftwareRevision( AMRBlock_ptr block, PNIO_UINT8 *InputData, PNIO_UINT32 size )
{
    PNIO_UINT32 i;
    AM_FirmwareInformation *dst = NULL;

    if ( !block )
    {
        printf ("ERROR: (AMR) - asset management block is NULL \n");
        return;
    }

    switch ( block->type )
    {
        case Asset_FullInformation:
            dst = &block->AMInfo.fullInformation.firmwareInformation;
            break;
        case Asset_FirmwareInformation:
            dst = &block->AMInfo.firmwareInformation;
            break;
        case Asset_HardwareInformation:
        default:
            printf ("ERROR: (AMR) - wrong type of Asset Management Data \n");
            return;
    }

    if (dst->IMSoftwareRevision.SWRevisionPrefix != 0)
    {
        printf ("ERROR: (AMR) - IM SoftwareRevision is used \n");
        return;
    }

    for ( i = 0; i < size ; i ++)
    {
        if ( ( InputData[i] >= 0x20 ) && ( InputData[i] <= 0x7E ) )
        {
            dst->AMSoftwareRevision[i] = InputData[i];
        }
        else
        {
            printf ("ERROR: (AMR) - character is not visible string \n");
            return;
        }
    }

    // If the text is shorter than the defined string length, the gap shall be filled with blanks.
    if ( size < 64 )
    {
        for ( i = size ; i < 64 ; i++ )
        {
            dst->AMSoftwareRevision[i] = ' ';
        }
    }
}

/**
 * @brief Validy and Set of AM hardware Revision
 *
 * @param[out]  block         block where will be added
 * @param[in]   InputData     input data
 * @param[in]   length        length of data
 */
PNIO_VOID AMRObject_SetAMHardwareRevision( AMRBlock_ptr block, PNIO_UINT8 *InputData, PNIO_UINT32 size )
{
    PNIO_UINT32 i;
    AM_HardwareInformation *dst = NULL;

    if ( !block )
    {
        printf ("ERROR: (AMR) - asset management block is NULL \n");
        return;
    }

    switch ( block->type )
    {
        case Asset_FullInformation:
            dst = &block->AMInfo.fullInformation.hardwareInformation;
            break;
        case Asset_HardwareInformation:
            dst = &block->AMInfo.hardwareInformation;
            break;
        case Asset_FirmwareInformation:
        default:
            printf ("ERROR: (AMR) - wrong type of Asset Management Data \n");
            return;
    }

    if (dst->IMHardwareRevision != 0)
    {
        printf ("ERROR: (AMR) - IM HardwareRevision is used \n");
        return;
    }

    for ( i = 0; i < size ; i ++)
    {
        if ( ( InputData[i] >= 0x20 ) && ( InputData[i] <= 0x7E ) )
        {
            dst->AMHardwareRevision[i] = InputData[i];
        }
        else
        {
            printf ("ERROR: (AMR) - character is not visible string \n");
            return;
        }
    }

    // If the text is shorter than the defined string length, the gap shall be filled with blanks.
    if ( size < 64 )
    {
        for ( i = size ; i < 64 ; i++ )
        {
            dst->AMHardwareRevision[i] = ' ';
        }
    }
}

/**
 * @brief Validy and Set of IM software revision
 *
 * @param[out]  block      block where will be added
 * @param[in]   prefix     prefix of SW release
 * @param[in]   revision   revision of SW release
 * @param[in]   bugfix     bugfix of SW release
 * @param[in]   internal   internal change of SW release
 */
PNIO_VOID AMRObject_SetIMSoftwareRevision( AMRBlock_ptr block, PNIO_UINT8 prefix, PNIO_UINT8 revision, PNIO_UINT8 bugfix, PNIO_UINT8 internal )
{
    PNIO_UINT32 i;
    AM_FirmwareInformation *dst = NULL;

    if ( !block )
    {
        printf ("ERROR: (AMR) - asset management block is NULL \n");
        return;
    }

    switch ( block->type )
    {
        case Asset_FullInformation:
            dst = &block->AMInfo.fullInformation.firmwareInformation;
            break;
        case Asset_FirmwareInformation:
            dst = &block->AMInfo.firmwareInformation;
            break;
        case Asset_HardwareInformation:
        default:
            printf ("ERROR: (AMR) - wrong type of Asset Management Data \n");
            return;
    }

    if ( ( prefix != 'V') && (prefix != 'R') &&
         ( prefix != 'P') && (prefix != 'U') &&
         ( prefix != 'T')
       )
    {
        printf ("ERROR: (AMR) - SW prefix is wrong \n");
        return;
    }

    dst->IMSoftwareRevision.SWRevisionPrefix = prefix;
    dst->IMSoftwareRevision.IM_SWRevision_Functional_Enhancement = revision;
    dst->IMSoftwareRevision.IM_SWRevision_Bug_Fix = bugfix;
    dst->IMSoftwareRevision.IM_SWRevision_Internal_Change = internal;

    for ( i = 0; i < 64 ; i ++)
    {
        dst->AMSoftwareRevision[i] = ' ';
    }
}

/**
 * @brief   Validy and Set IM Hardware revision
 *
 * @param[out]  block                   block where will be added
 * @param[in]   ImHardwareRevision      im hw revision
 */
PNIO_VOID AMRObject_SetIMHardwareRevision( AMRBlock_ptr block, PNIO_UINT16 ImHardwareRevision )
{
    PNIO_UINT32 i;
    AM_HardwareInformation *dst = NULL;

    if ( !block )
    {
        printf ("ERROR: (AMR) - asset management block is NULL \n");
        return;
    }

    switch ( block->type )
    {
        case Asset_FullInformation:
            dst = &block->AMInfo.fullInformation.hardwareInformation;
            break;
        case Asset_HardwareInformation:
            dst = &block->AMInfo.hardwareInformation;
            break;
        case Asset_FirmwareInformation:
        default:
            printf ("ERROR: (AMR) - wrong type of Asset Management Data \n");
            return;
    }

    dst->IMHardwareRevision = ImHardwareRevision;

    for ( i = 0; i < 64 ; i ++)
    {
        dst->AMHardwareRevision[i] = ' ';
    }
}

PNIO_VOID ASSET_PUT8_HTON(PNIO_VOID * base, PNIO_UINT32 offset, PNIO_UINT8 val)
{
    PNIO_UINT8 *ptr = base;
    ptr += offset;

    memcpy ((void*)ptr, (void*) & val, sizeof(val));
}

PNIO_VOID ASSET_PUT16_HTON( PNIO_VOID * base, PNIO_UINT32 offset, PNIO_UINT16 val )
{
    PNIO_UINT8 *ptr = base;
    ptr += offset;

    val = htons (val);

    memcpy ((void*)ptr, (void*) & val, sizeof(val));
}

PNIO_VOID   ASSET_PUT32_HTON(PNIO_VOID * base, PNIO_UINT32 offset, PNIO_UINT32 val)
{
    PNIO_UINT8 *ptr = base;
    ptr += offset;

    val = htonl (val);

    memcpy ((void*)ptr, (void*) & val, sizeof(val));
}
#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
