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
/*  F i l e               &F: ecos_ertec_bsp_spi_flash.h                :F&  */
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
/*
 * ecos_ertec_bsp_spi_flash.h
 *
 *  Created on: 15. 6. 2016
 *      Author: z003dbfm
 */

#ifndef SYSADAPT1_CFG_ECOS_ERTEC_BSP_SPI_FLASH_H_
#define SYSADAPT1_CFG_ECOS_ERTEC_BSP_SPI_FLASH_H_

/*adesto AT45DB081E SPI Flash memory commands*/
#define FLASH_AT_CMD_READ_STATUS			0xD7
#define FLASH_AT_CMD_READ					0x1B /*0xE8*/
#define FLASH_AT_CMD_BUFFER_TO_FLASH
#define FLASH_AT_CMD_READ_MODIFY_WRITE		0x58

/*Returned values of status check*/
#define FLASH_AT_STAT_RDY					0x80

/*winbond flash commands*/
#define FLASH_WB_CMD_READ					0x03
#define FLASH_WB_CMD_READ_STATUS			0x05
#define FLASH_WB_CMD_PROGRAM				0x02
#define FLASH_WB_CMD_WRITE_ENABLE			0x06
#define FLASH_WB_CMD_ERASE_4K				0x20

/*Retunred values of status check*/
#define FLASH_WB_STAT_RDY					0x01


/*flash identification*/
#define FLASH_ADESTO							0x10
#define FLASH_ADESTO_ID1						0x1F
#define FLASH_ADESTO_ID2						0x25
#define FLASH_ADESTO_ID3						0x00

#define FLASH_WINBOND						0x20
#define FLASH_WINBOND_ID1					0xEF
#define FLASH_WINBOND_ID2					0x40
#define FLASH_WINBOND_ID3					0x17

/*page size*/
#define FLASH_PAGE_SIZE_ADESTO				528
#define FLASH_PAGE_SIZE_WINBOND				256
#define FLASH_SECTOR_SIZE_WINBOND			4096

#define SPI_FLASH_ERR_OK					0x00


typedef struct DEV_STATUS
{
	unsigned char first;
	unsigned char second;
}DEV_STATUS;



/********************************public functions*******************************/
int 			spi_flash_read						( unsigned int flash_strt_addr, unsigned char * receive_addr, signed int data_size, unsigned int * pError );
int 			spi_flash_program					( unsigned int flash_strt_addr, unsigned char * transmit_addr, signed int data_size, unsigned int * pError );
void 			spi_flash_init						( void );
int				spi_flash_erase						( unsigned int flash_strt_addr, signed int data_size, unsigned int * pError );
int				spi_flash_chip_erase				( void );
int				spi_flash_erase_verify				( unsigned int flash_strt_addr, signed int data_size );
int 			spi_flash_verify					(unsigned char * pFlash, unsigned char * pBuf, int BufSize);


#endif /* SYSADAPT1_CFG_ECOS_ERTEC_BSP_SPI_FLASH_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
