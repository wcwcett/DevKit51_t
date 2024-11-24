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
/*  F i l e               &F: ecos_ertec_bsp_ospi_flash.c               :F&  */
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

#ifdef BOARD_TYPE_STEP_3

#include "compiler.h"
#include <pnioerrx.h>
#include <os.h>
#include <pniousrd.h>
#include "ertec200p_reg.h"
#include "flash_ospi_devices.h"
#include "flash_ospi.h"
#include "ecos_ertec_bsp_ospi_flash.h"



const JedecMapping_t jedec_flashtype[] = {
    { 0x1870ef, FLASH_OSPI_FLASH_W25Q128JVSJM, "Winbond", "W25Q128JVSJM" },
    { 0x1840ef, FLASH_OSPI_FLASH_W25Q128JVSJM, "Winbond", "W25Q128JVSIQ" },
    { 0x18ba20, FLASH_OSPI_FLASH_MT25QL128, "Micron", "MT25QL128" },
    { 0x1820c2, FLASH_OSPI_FLASH_MX25L12845_DDR, "Macronix", "MX25L12845" },
    { SYS_OSPI_ID_MASK, FLASH_OSPI_FLASH_NONE, NULL, NULL }             // end marker, must stay the last.
};

flash_ospi_info_t ospi_info;

/**
 *  @brief  Update GPIO config for ospi device 
 *
 *
 */
static void gpio_config(const void * data,
        PNIO_UINT32 portmode,   
        PNIO_UINT32 portmode_mask,
        PNIO_UINT32 ioctrl,     
        PNIO_UINT32 ioctrl_mask,
        PNIO_UINT32 mux63_32,   
        PNIO_UINT32 mux63_32_mask,
        PNIO_UINT32 pull47_32,  
        PNIO_UINT32 pull47_32_mask)
{
    PNIO_UINT32 tmp;
    tmp = (PNIO_UINT32)REG32(PN_SUB_GPIOX__GPIO_IOCTRL_1);
    tmp &= ~ioctrl_mask;
    tmp |= ioctrl;
    REG32(PN_SUB_GPIOX__GPIO_IOCTRL_1) = tmp;

    tmp = (PNIO_UINT32)REG32(PN_SUB_GPIOX__GPIO_PORT_MODE_1_L);
    tmp &= ~portmode_mask;
    tmp |= portmode;
    REG32(PN_SUB_GPIOX__GPIO_PORT_MODE_1_L) = tmp;

    tmp = (PNIO_UINT32)REG32(PN_SUB_PN_SCRB__PULL47_32GPIO);
    tmp &= ~pull47_32_mask;
    tmp |= pull47_32;
    REG32(PN_SUB_PN_SCRB__PULL47_32GPIO) = tmp;

    tmp = (PNIO_UINT32)REG32(PN_SUB_PN_SCRB2__MUX63_32GPIO);
    tmp &= ~mux63_32_mask;
    tmp |= mux63_32;
    REG32(PN_SUB_PN_SCRB2__MUX63_32GPIO) = tmp;
}

/**
 *  @brief  initialize ospi memory device 
 *
 *  @return Returns	0 if OK, 1 if error
 *
 *
 */
PNIO_UINT32 init_ospi()
{
    PNIO_UINT32 ospi_id = 0;
    PNIO_UINT32 flash_id = 0;
    PNIO_UINT32 ret_code = 0;

    flash_ospi_init(&ospi_info);

    struct flash_ospi_open_param open_params;


    open_params.gpio_config_func = gpio_config;
    open_params.gpio_config_data = (void *)SYS_OSPI_GPIO_CONFIG_DATA;
    open_params.wait_us_func = NULL; //no need to implement. only used in one place.
    open_params.wait_us_data = NULL; //no need to implement. only used in one place.

    flash_ospi_open(&ospi_info,&open_params,SYS_OSPI_RESET_REQUIRED);

    ospi_id = flash_ospi_read_idreg() & SYS_OSPI_ID_MASK;

    if (ospi_id == SYS_OSPI_ID_MASK)
    {
        //error in read. return error code
    }

    for (PNIO_UINT32 index = 0; jedec_flashtype[index].jedec_id != SYS_OSPI_ID_MASK; index++)
    {
        if (jedec_flashtype[index].jedec_id == ospi_id)
        {
            // found the device !
            flash_id = jedec_flashtype[index].flash_param_id;
            break;
        }
    }

    if(flash_id == 0)
    {
        return PNIO_NOT_OK;
    }

    ret_code = flash_ospi_setup(&ospi_info, flash_id);

    if(ret_code != PNIO_OK)
    {
        return PNIO_NOT_OK;
    }

    return PNIO_OK;
}

/**
 *  @brief delete data from ospi. It should be noted that this function deletes BLOCKS. 
 *         Thus, if any other data exists in that block, that will be deleted too.
 *
 *  @param[in]   sector_addr 	Sector Address (1,2,etc.) in SPi flash, which deletion will be start
 *  @param[in]   length      	Deletion length
 *  @return                     Returns	0 if OK, error code if error 
 *
 *
 */
PNIO_UINT32 ospi_erase(PNIO_UINT32 start_addr, PNIO_UINT32 length)
{
    PNIO_UINT32 start_block = 0; 
    PNIO_UINT32 try = 0;
    PNIO_UINT32 erase_nblocks = 0; 

    start_block = start_addr >> 16; 

    erase_nblocks = (length >> 16) + 1 + start_block;

    for(; start_block < erase_nblocks; start_block++)
    {
    	while(ospi_erase_block(start_block) != PNIO_OK)
    	{
    		try++;
    		//after 5 try, return
    		if(try >= 5)
    		{
    			return PNIO_NOT_OK;
    		}
    	}

    }

    return PNIO_OK;
}

/**
 *  @brief delete data sector from ospi. This function blocks until erase is done.
 *
 *  @param[in]   sector_addr 	Sector Address (1,2,etc.) in SPi flash, which will be deleted
 *  @return                     Returns	0 if OK, error code if error 
 *
 *
 */
PNIO_UINT32 ospi_erase_sector(PNIO_UINT32 sector_addr)
{
    PNIO_UINT32 ret1 = 1;

    ret1 = flash_ospi_erase_sector(&ospi_info, (sector_addr<<16));
    
    if (ret1 == 0)
    {
        while ((flash_ospi_get_erase_status(&ospi_info)) > 0) {
            //wait until job finishes
        }
    }

    return ret1;
}

/**
 *  @brief delete data block from ospi. This function blocks until erase is done.
 *
 *  @param[in]   block_addr 	Block Address (1,2,etc.) in SPi flash, which will be deleted 
 *  @return                     Returns	0 if OK, error code if error 
 *
 *
 */
PNIO_UINT32 ospi_erase_block(PNIO_UINT32 block_addr)
{
    PNIO_UINT32 ret1; 

    ret1 =  flash_ospi_erase_block(&ospi_info, block_addr << 16);

    if (ret1 == 0)
    {
        while (flash_ospi_get_erase_status(&ospi_info) != 0) {
            //wait until job finishes
        }
    }
    else
    {
        //problem about erasing block. return. 
        return PNIO_NOT_OK;
    }

    return PNIO_OK;
}

/**
 *  @brief write data to ospi without RSC
 *
 *  @param[in]   mem_addr 	Address in SPi flash, where write will begin
 *  @param[in]   src  	    Pointer to data, which will be written
 *  @param[in]   length     Amount of data to write
 *  @return                 Returns	0 if OK, error code if error 
 *
 *
 */
PNIO_UINT32 ospi_write_data(PNIO_UINT32 mem_addr, PNIO_VOID_PTR_TYPE src, PNIO_UINT32 length)
{
    PNIO_UINT32 ret; 
    ret = flash_ospi_write(&ospi_info, mem_addr, src, length);
    if(ret == 0)
    {
        return PNIO_OK;
    }
    
    return PNIO_NOT_OK;
}

/**
 *  @brief write data to ospi with RSC
 *
 *  @param[in]   mem_addr 	Address in SPi flash, where write will begin
 *  @param[in]   src  	    Pointer to data, which will be written
 *  @param[in]   length     Amount of data to write
 *  @return                 Returns	0 if OK, error code if error 
 *
 *
 */
PNIO_UINT32 ospi_write_data_rsc(PNIO_UINT32 mem_addr, PNIO_VOID_PTR_TYPE src, PNIO_UINT32 length)
{
    PNIO_UINT32 ret = 0; 
    PNIO_UINT32 total_write = 0; 
    PNIO_UINT32 leftover = 0; 
    PNIO_UINT32 current_mem_in_ospi = 0; 
    PNIO_UINT32 aligned_write = 0;

    //calculate aligned bytes pages
    total_write = length / SYS_OSPI_RSC_ENCODED_SIZE; 
    //calculate total aligned byte
    aligned_write = total_write*SYS_OSPI_RSC_ENCODED_SIZE;
    //calculate exact memory in ospi 
    current_mem_in_ospi = aligned_write + SYS_OSPI_RSC_SIZE*total_write;
    //calculate leftover (not aligned) memory bytes
    leftover = length - aligned_write;

    //first, write aligned bytes 
    ret = flash_ospi_write_rsc(&ospi_info, mem_addr, src, SYS_OSPI_RSC_ENCODED_SIZE, total_write);
    if(ret != 0)
    {
        return PNIO_NOT_OK;
    }
    if(leftover > 0)
    {
        //write leftover bytes
        ret = flash_ospi_write_rsc(&ospi_info, (mem_addr + current_mem_in_ospi),(PNIO_VOID_PTR_TYPE)((PNIO_UINT8*)src +aligned_write),
                                  leftover, 1);
        if(ret != 0)
        {
            return PNIO_NOT_OK;
        }
    }
    return PNIO_OK;
}

/**
 *  @brief read data from ospi without RSC
 *
 *  @param[in]   mem_addr 	Address in SPi flash, where read will begin
 *  @param[in]   dest  	    Pointer to data, which will be destination
 *  @param[in]   length     Amount of data to read
 *  @return                 Returns	0 if OK, error code if error 
 *
 *
 */
PNIO_UINT32 ospi_read_data(PNIO_UINT32 mem_addr, PNIO_VOID_PTR_TYPE dest, PNIO_UINT32 length)
{
    PNIO_UINT32 ret; 
    ret = flash_ospi_read(&ospi_info, mem_addr, dest, length);
    if(ret == 0)
    {
        return PNIO_OK;
    }
    
    return PNIO_NOT_OK;
}

/**
 *  @brief read data from ospi with RSC
 *
 *  @param[in]   mem_addr 	Address in SPi flash, where read will begin
 *  @param[in]   dest  	    Pointer to data, which will be destination
 *  @param[in]   length     Amount of data to read
 *  @return                 Returns	0 if OK, error code if error 
 *
 *
 */
PNIO_UINT32 ospi_read_data_rsc(PNIO_UINT32 mem_addr, PNIO_VOID_PTR_TYPE dest, PNIO_UINT32 length)
{

    PNIO_UINT32 ret = 0; 
    PNIO_UINT32 leftover = 0; 
    PNIO_UINT32 aligned_write = 0;
    PNIO_UINT32 total_write = 0; 
    PNIO_UINT32 current_mem_in_ospi = 0; 

    //calculate aligned bytes pages
    total_write = length / SYS_OSPI_RSC_ENCODED_SIZE; 
    //calculate total aligned byte
    aligned_write = total_write*SYS_OSPI_RSC_ENCODED_SIZE;
    //calculate exact memory in ospi 
    current_mem_in_ospi = aligned_write + SYS_OSPI_RSC_SIZE*total_write;
    //calculate leftover (not aligned) memory bytes
    leftover = length - aligned_write;

    ret = flash_ospi_read_rsc(&ospi_info, mem_addr, dest, SYS_OSPI_RSC_ENCODED_SIZE, total_write);
    if(ret != 0)
    {
        return PNIO_NOT_OK;
    }
    if(leftover > 0)
    {
        ret = flash_ospi_read_rsc(&ospi_info, (mem_addr + current_mem_in_ospi),(PNIO_VOID_PTR_TYPE)((PNIO_UINT8*)dest + aligned_write),
                                  leftover, 1);
        if(ret != 0)
        {
            return PNIO_NOT_OK;
        }
    }
    return PNIO_OK;
}

/**
 *  @brief verify memory has been erased
 *
 *  @param[in]   mem_addr 	Address in SPi flash, where verify will begin
 *  @param[in]   length     Deleted data length
 *  @return                 Returns	PNIO_OK if OK, PNIO_NOT_OK if error 
 *
 *
 */
PNIO_UINT32 ospi_erase_verify(PNIO_UINT32 mem_addr, PNIO_UINT32 length)
{
    PNIO_UINT32 i;
    PNIO_UINT8 * buff = NULL; 
    
    OsAllocF((void**)&buff, length);

    ospi_read_data(mem_addr, buff, length);

    for ( i = 0; i < length; i++ )
    {
    	if ( *( buff + i ) != 0xff )
    	{
            OsFree(buff);
    		return PNIO_NOT_OK;
    	}
    }

    OsFree(buff);
    return PNIO_OK;
}

/**
 *  @brief verify memory and buffer identical
 *
 *  @param[in]   mem_addr 	Address in SPi flash, where verify will begin
 *  @param[in]   src        Buffer which will be compared
 *  @param[in]   length     Deleted data length
 *  @return                 Returns	PNIO_OK if OK, PNIO_NOT_OK if error 
 *
 *
 */
PNIO_UINT32 ospi_verify_write_data(PNIO_UINT32 mem_addr, PNIO_VOID_PTR_TYPE src, PNIO_UINT32 length)
{
    PNIO_UINT32 i;
    PNIO_UINT8 * buff = NULL; 
    PNIO_UINT8 * pSrc = (PNIO_UINT8 *)src;
    
    OsAllocF((void**)&buff, length);
    ospi_read_data(mem_addr, buff, length);

        for ( i = 0; i < length; i++ )
        {
        	if ( *(  pSrc + i ) != *( buff + i ) )
        	{
        		PNIO_ConsolePrintf( "\nERROR flash program at offset 0x%x: value = 0x%x (0x%x) \n",
        					 i, *( pSrc+i ), * (buff+i ) );
                OsFree(buff);
        		return PNIO_NOT_OK;
        	}
        }
        OsFree(buff);
        return PNIO_OK;
}

#endif //BOARD_TYPE_STEP_3

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
