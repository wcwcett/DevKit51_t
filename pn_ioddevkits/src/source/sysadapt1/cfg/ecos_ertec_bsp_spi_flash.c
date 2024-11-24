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
/*  F i l e               &F: ecos_ertec_bsp_spi_flash.c                :F&  */
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

/**
* @file     ecos_ertec_bsp_spi_flash.c
* @brief    functions for SPI flash usage
*
* Only for Adesto and Winbond flashes
* Covers identification, initialization, read, write and erase of both flashes
* Needs functions from ecos_ertec_bsp_spi.c to work
*
*/

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version    What                                              */
/*---------------------------------------------------------------------------*/

#include "compiler.h"

#if ( PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
#include <ecos_ertec_bsp_spi_flash.h>
#include <ecos_ertec_bsp_spi.h>
#include <pnioerrx.h>
#include <os.h>
#include <pniousrd.h>


/********************************internal function prototypes*******************************/
/*******************************************************************************************/

static int		flash_identify					( void );

static void 	flash_wait_ready_adesto			( void );
static int 		flash_read_status_adesto			( unsigned char * byte_1, unsigned char * byte_2 );
static void		flash_read_dev_info_adesto		( unsigned char * receive_addr );
static int 		flash_program_one_page_adesto	( unsigned int flash_strt_addr, unsigned char * transmit_addr,	unsigned int data_size,	unsigned int * pError );
static int 		flash_read_adesto				( unsigned int flash_strt_addr, unsigned char * receive_addr, unsigned int data_size, unsigned int * pError );
static int		flash_program_adesto				( unsigned int flash_strt_addr, unsigned char * transmit_addr, signed int data_size, unsigned int * pError );
static void 	flash_erase_adesto				(unsigned int flash_strt_addr);

static void 	flash_wait_ready_winbond		( void );
static int 		flash_read_status_winbond		( unsigned char * byte_1, unsigned char * byte_2 );
static void 	flash_read_dev_info_winbond		( unsigned char * receive_addr );
static int 		flash_program_one_page_winbond	( unsigned int flash_strt_addr, unsigned char * transmit_addr, signed int data_size, unsigned int * pError );
static int		flash_read_winbond				( unsigned int flash_strt_addr, unsigned char * receive_addr, unsigned int data_size, unsigned int * pError );
static int		flash_program_winbond			( unsigned int flash_strt_addr, unsigned char * transmit_addr, signed int data_size, unsigned int * pError );
static int 		flash_read_erase_write_winbond	( unsigned int flash_strt_addr, unsigned char * transmit_addr, signed int data_size, unsigned int * pError );
int 			flash_erase_winbond				( unsigned int flash_strt_addr, unsigned int * pError );
static void 	flash_write_enable_winbond		( void );

/********************************glabal variables*******************************************/
/*******************************************************************************************/

unsigned char flash_type;

/*******************************************************************************************/



/**
 *  @brief Init SPI and decide flash type
 *
 *  @return      			Void
 *
 *
 */
void spi_flash_init( void )
{
	spi_init(  );
	flash_type = flash_identify(  );
}	/*spi_flash_init(  )*/



/**
 *  @brief Identifies flash type
 *
 *  @return      			Returns	flash ID or 0 if unknown/fail
 *
 *
 */
static int flash_identify( void )
{
	static unsigned char buff[ 9 ];
	flash_read_dev_info_adesto( &buff[ 0 ] );
	if( (FLASH_ADESTO_ID1 == buff[ 1 ] ) )
	{
		return FLASH_ADESTO;
	}
	flash_read_dev_info_winbond( &buff[ 0 ] );
	if( (FLASH_WINBOND_ID1 == buff[ 1 ] ) )
	{
        flash_write_enable_winbond();
		return FLASH_WINBOND;
	}
	return 0;
}	/*flash_identify(  )*/



/**
 *  @brief Decide which flash write function to call, call it
 *
 *  @param[in]   flash_strt_addr 	Address in SPi flash, where write will begin
 *  @param[in]   * transmit_addr 	Pointer to data, which will be written
 *  @param[in]   data_size 			Amount of data to write
 *  @param[out]  * pError 			Pointer for error return
 *  @return      					Returns	0 if OK, 1 if error
 *
 *
 */
int spi_flash_program(
		unsigned int flash_strt_addr,
		unsigned char * transmit_addr,
		signed int data_size,
		unsigned int * pError )
{
	if( FLASH_WINBOND == flash_type )
	{
		flash_read_erase_write_winbond( flash_strt_addr, transmit_addr, data_size, pError );
		return 0;
	}
	else if( FLASH_ADESTO == flash_type )
	{
		flash_program_adesto( ( flash_strt_addr & 0x0fffffff ), transmit_addr, data_size, pError );
		return 0;
	}
	else
	{
	    PNIO_printf ("Wrong type of flash!");
	}
	return 1;
}	/*spi_flash_program(  )*/




/**
 *  @brief Decide which flash read function to call, call it
 *
 *  @param[in]   flash_strt_addr 	Address in SPi flash, where read will begin
 *  @param[out]  * receive_addr 	Pointer to buffer, where read data will be placed
 *  @param[in]   data_size 			Amount of data to read
 *  @param[out]  * pError 			Pointer for error return
 *  @return      					Returns	0 if Error, return value of read function if OK
 *
 *
 */
int spi_flash_read(
		unsigned int flash_strt_addr,
		unsigned char * receive_addr,
		signed int data_size,
		unsigned int * pError )
{
	if( FLASH_WINBOND == flash_type )
	{
		return flash_read_winbond( flash_strt_addr, receive_addr, data_size, pError );
	}
	else if( FLASH_ADESTO == flash_type )
	{
		return flash_read_adesto( ( flash_strt_addr & 0x0fffffff ), receive_addr, data_size, pError );
	}
	return 0;
}	/*spi_flash_read(  )*/



/**
 *  @brief Dummy function for erase
 *
 *  @param[in]   flash_strt_addr 	Dummy
 *  @param[in]   data_size 			Dummy
 *  @param[out]  * pError 			Pointer for error return
 *  @return      					Returns	0
 *
 *	Dummy function for compatibility with fw update process
 *	Write functions cares about erasing, no need to solve it
 *
 */
int	spi_flash_erase	( unsigned int flash_strt_addr, signed int data_size, unsigned int * pError )
{
	return 0;
}	/*spi_flash_erase(  )*/



/**
 *  @brief Erase whole SPI flash
 *
 *  @return      			Returns	0
 *
 *  These erase functions are for 8MB Winbond or 4MB Adesto only
 *
 */
int	spi_flash_chip_erase( void )
{
	unsigned int i;
	unsigned int Error;

	if( FLASH_WINBOND == flash_type )
	{
		for ( i = 0; i < 0x800; i++ )
		{
			flash_erase_winbond( ( i * 0x1000 ), &Error );
		}
	}
	else if ( FLASH_ADESTO == flash_type )
	{
		for ( i = 0; i < 8064; i++ )
		{
			flash_erase_adesto( ( i * 528 ) );
		}
	}

	return 0;
}	/*spi_flash_chip_erase(  )*/



/**
 *  @brief Dummy function for erase
 *
 *  @param[in]   flash_strt_addr 	Dummy
 *  @param[in]   data_size 			Dummy
 *  @return      					Returns	0
 *
 *	Dummy function for compatibility with fw update process
 *	Write functions cares about erasing, no need to solve it
 *
 */
int	spi_flash_erase_verify( unsigned int flash_strt_addr, signed int data_size )
{
	return 0;
}	/*spi_flash_erase_verify(  )*/



/**
 *  @brief Compares data in buffer with data in SPI flash
 *
 *  @param[in]   * pFlash 			Start of data in flash
 *  @param[in]   * pBuf 			Pointer to Buffer with data
 *  @param[in]   data_size 			Amount of data to compare
 *  @return      					Returns	PNIO_OK or PNIO_NOT_OK
 *
 *	Reads data from flash, compare two buffers
 *
 */
int spi_flash_verify( unsigned char * pFlash, unsigned char * pBuf, int BufSize )
{
	unsigned char * tBuf;
	int i, status;
	unsigned int  err_addr;

	// ***** alloc memory for test data ******
	if ( OsAllocF ( ( void** )&tBuf, BufSize ) != PNIO_OK )
	{
		PNIO_printf ( "ERROR OsAlloc\n" );
	}

	status = spi_flash_read( ( unsigned int )pFlash, tBuf, BufSize, &err_addr );
	if ( status == SPI_FLASH_ERR_OK )
	{
		PNIO_printf( "\nERROR flash_read at addr %x\n", err_addr );
		return ( PNIO_NOT_OK );
	}

	for ( i = 0; i < BufSize; i++ )
	{
		if ( *( tBuf + i ) != *( pBuf + i ) )
		{
			PNIO_printf( "\nERROR flash program at offset 0x%x: value = 0x%x (0x%x) \n",
						 i, *( tBuf + i ), *( pBuf + i ) );
			OsFree( tBuf );
			return ( PNIO_NOT_OK );
		}
	}
	OsFree( tBuf );
	return PNIO_OK;
}	/*spi_flash_verify(  )*/



/**
 *  @brief Software write enable
 *
 *  @return      			Void
 *
 *  After some types of operations, the Winbond flash automatically goes to write disable state
 *  It is recommended to call this function before any write or erase request
 *  Internal, implemented in all Winbond SPI flash functions, which needs it
 *
 */
static void flash_write_enable_winbond( void )
{
	SPI_CS_LOW;
	spi_send_byte( FLASH_WB_CMD_WRITE_ENABLE );
	spi_rxb_flush(  );
	SPI_CS_HIGH;
}	/*flash_write_enble_winbond(  )*/



/**
 *  @brief Reads Winbond device ID
 *
 *  @param[out]  * receive_addr 	Buffer to return read data
 *  @return      					Void
 *
 *	Calling function should check read data
 *
 */
static void flash_read_dev_info_winbond( unsigned char * receive_addr )
{
	int i;
	SPI_CS_LOW;
	spi_send_byte( 0x9F );
	spi_send_byte( 0x00 );
	spi_send_byte( 0x00 );
	spi_send_byte( 0x00 );
	spi_send_byte( 0x00 );
	SPI_CS_HIGH;
	for( i = 0; i < 5; i++ )
	{
		*receive_addr = spi_read_byte(  );
		receive_addr++;
	}
}	/*flash_read_dev_info_winbond(  )*/



/**
 *  @brief Write data to flash, Simulate RAM write
 *
 *  @param[in]   flash_strt_addr 	Address in SPi flash, where write will begin
 *  @param[in]   * transmit_addr 	Pointer to data, which will be written
 *  @param[in]   data_size 			Amount of data to write
 *  @param[out]  * pError 			Pointer for error return
 *  @return      					Returns	amount of written data
 *
 *  Simulates RAM write: Erase before write not needed and does not damage rest of data in sector
 *  For this, this function performs read and erase, then modify read data and write them
 *  Divides data to sectors ( 4096B ) as it process one sector at a time
 *  Start or end in the middle of sector is not a problem
 *
 */
/*write specified data without damaging data in rest of sector*/
static int flash_read_erase_write_winbond(
		unsigned int flash_strt_addr,
		unsigned char * transmit_addr,
		signed int data_size,
		unsigned int * pError )
{
	unsigned char *buffer;
	volatile int sector, sector_begin, addr_in_sector, size, program_sum;
	int i;
	unsigned char * src;
	unsigned char * dst;

	if ( OsAllocF ( ( void** )&buffer, 4096 ) != PNIO_OK )
	{
		PNIO_printf ( "ERROR OsAlloc\n" );
	}

	program_sum = 0;

	while ( 0 < data_size )
	{
		/*identify sector*/
		sector = flash_strt_addr / FLASH_SECTOR_SIZE_WINBOND;
		sector_begin = sector * FLASH_SECTOR_SIZE_WINBOND;
		addr_in_sector = flash_strt_addr % FLASH_SECTOR_SIZE_WINBOND;
		/*copy whole sector? -> read and modify approach not needed*/
		if( ( 0 == addr_in_sector ) && ( data_size >= FLASH_SECTOR_SIZE_WINBOND ) )
		{
			/*erase sector*/
			flash_erase_winbond( sector_begin, pError );
			/*upload new sector*/
			size = flash_program_winbond( sector_begin, transmit_addr, FLASH_SECTOR_SIZE_WINBOND, pError );
		}
		else
		{
			/*download sector*/
			flash_read_winbond( sector_begin, &buffer[ 0 ], FLASH_SECTOR_SIZE_WINBOND, pError );
			/*erase sector*/
			flash_erase_winbond( sector_begin, pError );
			/*modify downloaded sector */
				/*count bytes to write*/
			if( ( 0 == addr_in_sector ) || ( data_size < ( FLASH_SECTOR_SIZE_WINBOND - addr_in_sector ) ) )
			{	/*last write or ( first and also last write ) */
				size = data_size;
			}
			else
			{	/*first write, but there will be next*/
				size = FLASH_SECTOR_SIZE_WINBOND - addr_in_sector;
			}
			/*memcpy to buffer - risk of alignment problems, so done manually byte by byte*/
			dst =  &buffer[ addr_in_sector ];
			src = transmit_addr;
			for ( i = 0; i < size; i++ )
			{
				*dst = *src;
				src++;
				dst++;
			}
			/*upload modified sector*/
			flash_program_winbond( sector_begin, &buffer[ 0 ], FLASH_SECTOR_SIZE_WINBOND, pError );
		}
		/*actualise counters and pointers*/
		program_sum += size;
		transmit_addr += size;
		flash_strt_addr += size;
		data_size -= size;
	}

	OsFree( buffer );
	return program_sum;
}	/*flash_read_erase_write_winbond(  )*/



/**
 *  @brief Write up to one page ( 256B ) of data to flash
 *
 *  @param[in]   flash_strt_addr 	Address in SPi flash, where write will begin
 *  @param[in]   * transmit_addr 	Pointer to data, which will be written
 *  @param[in]   data_size 			Amount of data to write
 *  @param[out]  * pError 			Pointer for error return
 *  @return      					Returns	amount of written data
 *
 *  Flash write: flash have to be erased before
 *  Cannot go over edge of page ( start at one page and end on other )
 *
 */
static int flash_program_one_page_winbond(
		unsigned int flash_strt_addr,
		unsigned char * transmit_addr,
		signed int data_size,
		unsigned int * pError )
{
	int i;
	unsigned char reading_done = 0;

	flash_wait_ready_winbond(  );
	flash_write_enable_winbond(  );

	SPI_CS_LOW;
	spi_send_byte( FLASH_WB_CMD_PROGRAM );		/*write command*/
	spi_send_byte( ( unsigned char )( flash_strt_addr>>16 ) );
	spi_send_byte( ( unsigned char )( flash_strt_addr>>8 ) );
	spi_send_byte( ( unsigned char )( flash_strt_addr ) );

	i = 0;
	/*writing data to flash*/
	while( 0 == reading_done )
	{
		i++;
		spi_send_byte( * transmit_addr );
		transmit_addr++;
		if( i >= data_size )
		{
			reading_done = 1;
		}
	}
	spi_rxb_flush(  );
	SPI_CS_HIGH;

	return ( i );
}	/*flash_program_one_page_winbond(  )*/



/**
 *  @brief Divide data into pages, call write
 *
 *  @param[in]   flash_strt_addr 	Address in SPi flash, where write will begin
 *  @param[in]   * transmit_addr 	Pointer to data, which will be written
 *  @param[in]   data_size 			Amount of data to write
 *  @param[out]  * pError 			Pointer for error return
 *  @return      					Returns	amount of written data
 *
 *  Finds beginning of page to write
 *  Any data length
 *  Flash area to write have to be previously erased
 *
 */
static int flash_program_winbond(
		unsigned int flash_strt_addr,
		unsigned char * transmit_addr,
		signed int data_size,
		unsigned int * pError )
{
	unsigned int tmp, wrote = 0, to_write;
	/*phase 1 - finish incomplete page*/
	tmp = flash_strt_addr % FLASH_PAGE_SIZE_WINBOND;
	to_write = FLASH_PAGE_SIZE_WINBOND - tmp;
	if( 0 != ( tmp ) )
	{
		wrote = flash_program_one_page_winbond( flash_strt_addr, transmit_addr, ( to_write ), pError );
		flash_strt_addr += ( to_write );
		data_size -= ( to_write );
		transmit_addr += ( to_write );
	}
	/*phase 2 write all remaining pages*/
	while( data_size > 0 )
	{
		/*complete or incomplete page data size*/
		if( data_size > FLASH_PAGE_SIZE_WINBOND )
		{
			to_write = FLASH_PAGE_SIZE_WINBOND;
		}
		else
		{
			to_write = data_size;
		}
		wrote += flash_program_one_page_winbond( flash_strt_addr, transmit_addr, ( to_write ), pError );
		flash_strt_addr += ( to_write );
		data_size -= ( to_write );
		//lint -e{662} possible creation of out-of-bounds pointer ('Integer' beyond end of data) by operator 'String'
		transmit_addr += ( to_write );
	}
	return wrote;
}	/*flash_program_winbond(  )*/



/**
 *  @brief Reads data from SPI flash to provided buffer
 *
 *  @param[in]   flash_strt_addr 	Address in SPi flash, where read will begin
 *  @param[out]  * receive_addr 	Pointer to buffer, where read data will be placed
 *  @param[in]   data_size 			Amount of data to read
 *  @param[out]  * pError 			Pointer for error return
 *  @return      					Returns	amount of read data
 *
 *
 */
static int flash_read_winbond(
		unsigned int flash_strt_addr,
		unsigned char * receive_addr,
		unsigned int data_size,
		unsigned int * pError )
{
	static int to_read, read_bytes;
	int i;
	unsigned char reading_done = 0;
	read_bytes = 0;
	flash_wait_ready_winbond();

	SPI_CS_LOW;
	spi_send_byte( FLASH_WB_CMD_READ );		/*read command*/
	spi_send_byte( ( unsigned char )( flash_strt_addr>>16 ) );
	spi_send_byte( ( unsigned char )( flash_strt_addr>>8 ) );
	spi_send_byte( ( unsigned char )( flash_strt_addr ) );

	spi_rxb_flush();

	i = 0;
	/*reading data from flash*/
	while ( 4 != reading_done )
	{
		/*reading works with chunks of data of max size 512B - half of the buffer*/
		if ( data_size > 512 )
		{
			to_read = 512;
		}
		else
		{
			to_read = data_size;
		}
		i = 0;
		reading_done = 0;
		while( 0 == reading_done )
		{
			i++;
			spi_send_byte( 0x00 );
			if( i >= to_read )
			{
				reading_done = 1;
			}
		}

		/*interrupts will put received data to buffer, just take them*/
		i = 0;
		while( 1 == reading_done )
		{
			i++;
			* receive_addr = spi_read_byte(  );
			receive_addr++;
			if( i >= to_read )
			{
				reading_done = 2;
			}
		}
		data_size -= ( i );
		read_bytes += ( i );
		if ( data_size <= 0 )
		{
			reading_done = 4;
		}
	}

	SPI_CS_HIGH;
	return read_bytes;
}	/*flash_read_winbond(  )*/



/**
 *  @brief Asks flash for its status
 *
 *  @param[out]  byte_1 	Return pointer for first byte of status
 *  @param[out]  byte_2 	Return pointer for second byte of status
 *  @return      			Returns	1
 *
 *
 */
static int flash_read_status_winbond( unsigned char * byte_1, unsigned char * byte_2 )
{
	SPI_CS_LOW;
	spi_send_byte( FLASH_WB_CMD_READ_STATUS );/*command: read status*/
	spi_send_byte( 0x00 );/*dummy*/
	spi_send_byte( 0x00 );/*send dummy to read incoming data*/
	spi_send_byte( 0x00 );/*send dummy to read incoming data*/
	SPI_CS_HIGH;

	spi_read_byte(  );/*trash first received byte - nothing interesting there*/
	* byte_1 = spi_read_byte(  );
	* byte_2 = spi_read_byte(  );
	spi_read_byte(  );
	return 1;
}	/*flash_read_status_winbond(  )*/



/**
 *  @brief Wait if flash is busy
 *
 *  @return      			Void
 *
 *	Blocking, cyclically check flash status until it is not ready
 *
 */
static void flash_wait_ready_winbond( void )
{
	static DEV_STATUS status;
	do
	{
		flash_read_status_winbond( &status.first, &status.second );
	}
	while( ( status.first & FLASH_WB_STAT_RDY ) != 0 );
}	/*flash_wait_ready_winbond(  )*/



/**
 *  @brief Erase one 4K sector
 *
 *  @param[in]   flash_strt_addr 	Adress in flash, sector around it will be erased
 *  @param[out]  * pError 			Pointer for error return
 *  @return      					Returns	0
 *
 *	Start address might be anywhere in sector
 *	Always erase one whole sector
 *
 */
int flash_erase_winbond(
		unsigned int flash_strt_addr,
		unsigned int * pError )
{
	flash_wait_ready_winbond(  );
	flash_write_enable_winbond(  );

	SPI_CS_LOW;
	spi_send_byte( FLASH_WB_CMD_ERASE_4K );		/*read command*/
	spi_send_byte( ( unsigned char )( flash_strt_addr>>16 ) );
	spi_send_byte( ( unsigned char )( flash_strt_addr>>8 ) );
	spi_send_byte( ( unsigned char )( flash_strt_addr ) );

	spi_rxb_flush(  );
	SPI_CS_HIGH;
	return 0;

}	/*flash_erase_winbond(  )*/



/**
 *  @brief Erase one page ( 528B )
 *
 *  @param[in]   flash_strt_addr 	Adress in flash, page around it will be erased
 *  @param[out]  * pError 			Pointer for error return
 *  @return      					Void
 *
 *	Start address might be anywhere in page
 *	Always erase one whole page
 *	Unusual page size: 528 Bytes
 *
 */
static void flash_erase_adesto( unsigned int flash_strt_addr )
{
	unsigned int flash_page;
	flash_page = ( unsigned int )( flash_strt_addr / FLASH_PAGE_SIZE_ADESTO );

	flash_wait_ready_adesto(  );

	SPI_CS_LOW;
	spi_send_byte( 0x81 );spi_send_byte( ( unsigned char )( ( flash_page >> 6 ) & 0x7f ) );
	spi_send_byte( ( unsigned char )( flash_page << 2 ) );
	spi_send_byte( 0x00 );	//dummy

	spi_rxb_flush(  );
	SPI_CS_HIGH;
}	/*flash_erase_adesto(  )*/



/**
 *  @brief Reads Adesto device ID
 *
 *  @param[out]  * receive_addr 	Buffer to return read data
 *  @return      					Void
 *
 *	Calling function should check read data
 *
 */
/*read device ID*/
static void flash_read_dev_info_adesto( unsigned char * receive_addr )
{
	int i;
	SPI_CS_LOW;
	spi_send_byte( 0x9F );
	spi_send_byte( 0x90 );
	spi_send_byte( 0x92 );
	spi_send_byte( 0x94 );
	spi_send_byte( 0x00 );
	SPI_CS_HIGH;
	for( i = 0; i < 5; i++ )
	{
		*receive_addr = spi_read_byte(  );
		receive_addr++;
	}
}	/*flash_read_dev_info_adesto(  )*/



/**
 *  @brief Write up to one page ( 528B ) of data to flash
 *
 *  @param[in]   flash_strt_addr 	Address in SPi flash, where write will begin
 *  @param[in]   * transmit_addr 	Pointer to data, which will be written
 *  @param[in]   data_size 			Amount of data to write
 *  @param[out]  * pError 			Pointer for error return
 *  @return      					Returns	amount of written data
 *
 *  The page DO NOT have to be previously erased
 *  Cannot go over edge of page ( start at one page and end on other )
 *  Do not damage unaffected data in page (behaves as RAM write)
 *	Unusual page size: 528 Bytes
 *
 */
static int flash_program_one_page_adesto(
		unsigned int flash_strt_addr,
		unsigned char * transmit_addr,
		unsigned int data_size,
		unsigned int * pError )
{
	unsigned char tmp;
	unsigned int flash_addr, flash_page;
	int i;
	unsigned char reading_done = 0;


	flash_page = ( unsigned int )( flash_strt_addr / FLASH_PAGE_SIZE_ADESTO );
	flash_addr = ( unsigned int )( flash_strt_addr % FLASH_PAGE_SIZE_ADESTO );

	flash_wait_ready_adesto(  );

	SPI_CS_LOW;
	spi_send_byte( FLASH_AT_CMD_READ_MODIFY_WRITE );	/*read-modify-write command*/

	/*3 bytes address:
		 * 1 dummy bits
		 * 13 bits page address
		 * 10 bits address*/
	spi_send_byte( ( unsigned char )( ( flash_page >> 6 ) & 0x7f ) );
	tmp = ( unsigned char )( flash_page << 2 );
	tmp |= ( unsigned char )( ( flash_addr >> 8 ) & 0x03 );
	spi_send_byte( tmp );
	spi_send_byte( ( unsigned char )( flash_addr ) );

	/*trash 4 bytes - responses to command*/
	spi_rxb_flush(  );
	i = 0;
	/*writing data to flash*/
	while( 0 == reading_done )
	{
		i++;
		spi_send_byte( * transmit_addr );
		transmit_addr++;
		if( i >= data_size )
		{
			reading_done = 1;
		}
	}
	/*trash deta from spi during write*/
	spi_rxb_flush(  );
	SPI_CS_HIGH;
	/*done - end reading command*/

	return ( i );
}	/*flash_program_one_page_adesto(  )*/



/**
 *  @brief Divide data into pages, call write
 *
 *  @param[in]   flash_strt_addr 	Address in SPi flash, where write will begin
 *  @param[in]   * transmit_addr 	Pointer to data, which will be written
 *  @param[in]   data_size 			Amount of data to write
 *  @param[out]  * pError 			Pointer for error return
 *  @return      					Returns	amount of written data
 *
 *  Finds beginning of page to write
 *  Any data length
 *  Flash area to write DO NOT have to be previously erased
 *  Do not damage rest of the sector or page
 *
 */
static int flash_program_adesto(
		unsigned int flash_strt_addr,
		unsigned char * transmit_addr,
		signed int data_size,
		unsigned int * pError )
{
	unsigned int tmp, wrote = 0, to_write;
	/*phase 1 - finish incomplete page*/
	tmp = flash_strt_addr % FLASH_PAGE_SIZE_ADESTO;
	to_write = FLASH_PAGE_SIZE_ADESTO - tmp;
	if( 0 != ( tmp ) )
	{
		wrote = flash_program_one_page_adesto( flash_strt_addr, transmit_addr, ( to_write ), pError );
		flash_strt_addr += ( to_write );
		data_size -= ( to_write );
		transmit_addr += ( to_write );
	}

	while( data_size > 0 )
	{
		if( data_size > FLASH_PAGE_SIZE_ADESTO )
		{
			to_write = FLASH_PAGE_SIZE_ADESTO;
		}
		else
		{
			to_write = data_size;
		}
		wrote += flash_program_one_page_adesto( flash_strt_addr, transmit_addr, ( to_write ), pError );
		flash_strt_addr += ( to_write );
		data_size -= ( to_write );
		//lint -e{662} possible creation of out-of-bounds pointer ('Integer' beyond end of data) by operator 'String'
		transmit_addr += ( to_write );
	}
	return wrote;
}	/*flash_program_adesto(  )*/



/**
 *  @brief Reads data from SPI flash to provided buffer
 *
 *  @param[in]   flash_strt_addr 	Address in SPi flash, where read will begin
 *  @param[out]  * receive_addr 	Pointer to buffer, where read data will be placed
 *  @param[in]   data_size 			Amount of data to read
 *  @param[out]  * pError 			Pointer for error return
 *  @return      					Returns	amount of read data
 *
 *
 */
/*reads requested amount of data from flash to provided buffer*/
static int flash_read_adesto(
		unsigned int flash_strt_addr,
		unsigned char * receive_addr,
		unsigned int data_size,
		unsigned int * pError )
{
	unsigned char tmp;
	static int to_read, read_bytes;
	unsigned int flash_page, flash_addr;
	int i;
	unsigned char reading_done = 0;

	read_bytes = 0;

	flash_page = ( unsigned int )( flash_strt_addr / FLASH_PAGE_SIZE_ADESTO );
	flash_addr = ( unsigned int )( flash_strt_addr % FLASH_PAGE_SIZE_ADESTO );

	flash_wait_ready_adesto(  );

	SPI_CS_LOW;
	spi_send_byte( FLASH_AT_CMD_READ );		/*read command*/

	/*3 bytes address:
		 * 1 dummy bits
		 * 13 bits page address
		 * 10 bits address*/
	spi_send_byte( ( unsigned char )( ( flash_page >> 6 ) & 0x7f ) );
	tmp = ( unsigned char )( flash_page << 2 );
	tmp |= ( unsigned char )( ( flash_addr >> 8 ) & 0x03 );
	spi_send_byte( tmp );
	spi_send_byte( ( unsigned char )( flash_addr ) );
	spi_send_byte( 0x00 );  /*2 dummy bytes*/
	spi_send_byte( 0x00 );

	spi_rxb_flush(  );

	i = 0;
	/*reading data from flash*/
	while ( 4 != reading_done )
	{
		if ( data_size > 512 )
		{
			to_read = 512;
		}
		else
		{
			to_read = data_size;
		}
		i = 0;
		reading_done = 0;
		while( 0 == reading_done )
		{
			i++;
			spi_send_byte( 0x00 );
			if( i >= to_read )
			{
				reading_done = 1;
			}
		}

		/*interrupts will put received data to buffer, just take them*/
		i = 0;
		while( 1 == reading_done )
		{
			i++;
			* receive_addr = spi_read_byte(  );
			receive_addr++;
			if( i >= to_read )
			{
				reading_done = 2;
			}
		}

		data_size -= ( i );
		read_bytes += ( i );
		if ( data_size <= 0 )
		{
			reading_done = 4;
		}
	}

	/*done - end reading command*/
	SPI_CS_HIGH;
	return read_bytes;
}	/*flash_read_adesto(  )*/



/**
 *  @brief Asks flash for its status
 *
 *  @param[out]  byte_1 	Return pointer for first byte of status
 *  @param[out]  byte_2 	Return pointer for second byte of status
 *  @return      			Returns	1
 *
 *
 */
static int flash_read_status_adesto( unsigned char * byte_1, unsigned char * byte_2 )
{
	SPI_CS_LOW;
	spi_send_byte( FLASH_AT_CMD_READ_STATUS );/*command: read status*/
	spi_send_byte( 0x00 );/*dummy*/
	spi_send_byte( 0x00 );/*send dummy to read incoming data*/
	spi_send_byte( 0x00 );/*send dummy to read incoming data*/
	SPI_CS_HIGH;

	spi_read_byte(  );/*trash first received byte - nothing interesting there*/
	* byte_1 = spi_read_byte(  );
	* byte_2 = spi_read_byte(  );
	spi_read_byte(  );
	return 1;
}	/*flash_read_status_adesto(  )*/



/**
 *  @brief Wait if flash is busy
 *
 *  @return      			Void
 *
 *	Blocking, cyclically check flash status until it is not ready
 *
 */
static void flash_wait_ready_adesto( void )
{
	static DEV_STATUS status;
	do
	{
		flash_read_status_adesto( &status.first, &status.second );
	}
	while( ( status.first & FLASH_AT_STAT_RDY ) == 0 );
}

#endif	/* #if ( PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P) */


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
