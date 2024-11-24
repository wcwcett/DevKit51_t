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
/*  F i l e               &F: ecos_ertec_bsp_spi.c                      :F&  */
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
* @file     ecos_ertec_bsp_spi.c
* @brief    functions for SPI usage
*
* covers SPI init, direct write and read using interrupts and buffer
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
#include <cyg/hal/arm926.h>
#include <cyg/kernel/kapi.h>
#include <ecos_ertec_bsp_spi.h>



/********************************internal function prototypes*******************************/
/*******************************************************************************************/
static void 			spi_open			( void );

static void 			spi_rxb_init		( void );
static unsigned char 	spi_rxb_read		( void );
static void 			spi_rxb_write		( unsigned char data );

/*******************************************************************************************/

/**
 *  @brief SPI receive interrupt isr
 *
 *  @return      			Void
 *
 *  Interrupt functionality, called after SPI data received
 *  running when interrupt pending -> as fast as possible
 *
 */
cyg_uint32
spi_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_interrupt_acknowledge(SY_INT_SPI_RNE);
    
    return CYG_ISR_CALL_DSR;
}


/**
 *  @brief SPI receive interrupt dsr
 *
 *  @return      			Void
 *
 *  interrupt functionality, called after SPI data received interrupt done
 *  actual interrupt reaction in here
 *  write all received data to rxb ( program ) buffer
 *
 */
void
spi_dsr(cyg_vector_t vector,
    cyg_ucount32 count,
    cyg_addrword_t data)
{
    /*read from SPI incomming buffer until it signalizes empty*/
    while ((REG32(USED_SPI__SSPSR)) & U2_SPI__SSPSR__RNE)
    {
        spi_rxb_write(REG32(USED_SPI__SSPDR));
    }
}


/**
 *  @brief Init SPI receive interrupt
 *
 *  @return      			Void
 *
 *  This function..
 *
 */
static void spi_open( void )
{
	static cyg_interrupt intH;
	static cyg_handle_t hIntH;

	cyg_interrupt_create(
            SY_INT_SPI_RNE, 		/*vector*/
			0, 						/*priority*/
			0,						/*data ptr*/
			( cyg_ISR_t * )spi_isr,	/* *isr*/
			( cyg_DSR_t * )spi_dsr,	/* *dsr*/
			&hIntH,					/* *return handle*/
			&intH);
	/* *interrupt*/
	cyg_interrupt_attach( hIntH );
	cyg_interrupt_unmask(SY_INT_SPI_RNE);
}	/*spi_open(  )*/



/**
 *  @brief Data from SPI to rxb buffer
 *
 *  @param[in]   data 		One byte received on SPI
 *  @return      			Void
 *
 *  Called by interrupt dsr after data was received on SPI
 *
 */
static void spi_rxb_write( unsigned char data )
{
	cyg_interrupt_disable(  );
	/*overflow - forget oldest record*/
	if ( RXbuff_CAPACITY  <= spi_rec_buff.no_records )
	{
		/*buffer full - do nothing - trash new data*/
	}
	else
	{
		if ( RXbuff_CAPACITY -1 > spi_rec_buff.write_ptr )
		{
			spi_rec_buff.write_ptr++;
		}
		else
		{
			spi_rec_buff.write_ptr = 0;
		}
		spi_rec_buff.no_records++;
		spi_rec_buff.buff[ spi_rec_buff.write_ptr ] = data;
	}
	cyg_interrupt_enable(  );
}	/*spi_rxb_write(  )*/



/**
 *  @brief Read and delete oldest record from rxb buffer
 *
 *  @return      			Returns	oldest byte from rxb buffer
 *
 *
 */
static unsigned char spi_rxb_read( void )
{
	unsigned char data;

	cyg_interrupt_disable(  );
	if ( RXbuff_CAPACITY - 1 > spi_rec_buff.read_ptr )
	{
		spi_rec_buff.read_ptr++;
	}
	else
	{
		spi_rec_buff.read_ptr = 0;
	}
	spi_rec_buff.no_records--;

	data = spi_rec_buff.buff[ spi_rec_buff.read_ptr ];

	cyg_interrupt_enable(  );

	return data;
}	/*spi_rxb_read(  )*/



/**
 *  @brief Init variables of rxb buffer (cyclic FIFO)
 *
 *  @return      			Void
 *
 *
 */
static void spi_rxb_init( void )
{
	spi_rec_buff.no_records = 0;
	spi_rec_buff.read_ptr = 0;
	spi_rec_buff.write_ptr = 0;
}	/*spi_rxb_init(  )*/



/**
 *  @brief Clean data from SPI and rxb buffer
 *
 *  @return      			Void
 *
 *  SPI always perform read and write together.. sometimes I read dummy data
 *
 */
void spi_rxb_flush( void )
{
	/*wait until all transfers are finished*/
	unsigned char tmp;
	do
	{
		tmp = REG32(USED_SPI__SSPSR);
	}
	while(	( ( tmp ) & U2_SPI__SSPSR__BSY ) || ( ( ~ tmp ) & U2_SPI__SSPSR__TNF )  );

	spi_rec_buff.no_records = 0;
	spi_rec_buff.read_ptr = 0;
	spi_rec_buff.write_ptr = 0;
}	/*spi_rxb_flush(  )*/



/**
 *  @brief Put one byte to SPI
 *
 *  @param[out]  i_data 	One byte to be send
 *  @return      			Void
 *
 *  Check if SPI idle before, wait if not
 *   => this is blocking function!
 *
 */
void spi_send_byte ( unsigned char i_data )
{
	unsigned char tmp;
	/*wait for SPI to be idle*/
	do
	{
		tmp = REG32(USED_SPI__SSPSR);
	}
	while(	( ( tmp ) & U2_SPI__SSPSR__BSY ) || ( ( ~ tmp ) & U2_SPI__SSPSR__TNF )  );
	REG32(USED_SPI__SSPDR) = i_data;
}	/*spi_send_byte(  )*/



/**
 *  @brief Take one byte from rxb buffer or wait for it
 *
 *  @return      			Returns	one byte from rxb buffer
 *
 *  Takes oldest byte from rxb buffer
 *  Data in rxb are put inside by interrupt
 *  This function may be called before recieve interrupt. In such case,
 *  the rxb buffer might be empty.. This function will wait until data are present
 *   => this is blocking function
 *
 */
unsigned char spi_read_byte( void )
{
	volatile unsigned int tmp;
	do
	{
		tmp = spi_rec_buff.no_records;
	}
	while( 0 == tmp );
	return spi_rxb_read(  );
}	/*spi_read_byte(  )*/



/**
 *  @brief User can check amount of data in rxb buffer
 *
 *  @return      			Returns	how many bytes is in rxb buffer
 *
 *  Amount of data from SPI, which was not processed yet
 *
 */
unsigned char spi_data_to_read( void )
{
	return spi_rec_buff.no_records;
}	/*spi_data_to_read(  )*/



/**
 *  @brief Init of SPI registers
 *
 *  @return      			Void
 *
 *  Have to be done before any SPI communication attempt
 *
 */
void spi_init( void )
{
	spi_rxb_init(  );

#if (USED_SPI_MODULE == 1)
    /* GPIOs for SPI1 direction setup: GPIO18 is MISO -> input; others output */
    REG32(U_GPIO__GPIO_IOCTRL_0) &= (~BIT_21);
    REG32(U_GPIO__GPIO_IOCTRL_0) &= (~BIT_18);
    REG32(U_GPIO__GPIO_IOCTRL_0) &= (~BIT_16);
    REG32(U_GPIO__GPIO_IOCTRL_0) |= (BIT_19);

    /* Port mode 0b01 for bits 0, 4, 6 means alternative mode of gpio 16 - 18 */
    REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= ~(BIT_1);
    REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= ~(BIT_5);
    REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= ~(BIT_7);
    REG32(U_GPIO__GPIO_PORT_MODE_0_H) |= (BIT_0);
    REG32(U_GPIO__GPIO_PORT_MODE_0_H) |= (BIT_4);
    REG32(U_GPIO__GPIO_PORT_MODE_0_H) |= (BIT_6);
#else
    /* GPIOs for SPI2 direction setup: GPIO27 is MISO -> input; others output */
    REG32(U_GPIO__GPIO_IOCTRL_0) &= (~BIT_31);
    REG32(U_GPIO__GPIO_IOCTRL_0) &= (~BIT_24);
    REG32(U_GPIO__GPIO_IOCTRL_0) &= (~BIT_26);
    REG32(U_GPIO__GPIO_IOCTRL_0) |= (BIT_27);

    /* Port mode 0b01 for bits16, 18, 20, 22 means alternative mode of gpio 24 - 26 */
    REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= ~(BIT_17);
    REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= ~(BIT_21);
    REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= ~(BIT_23);
    REG32(U_GPIO__GPIO_PORT_MODE_0_H) |= (BIT_16);
    REG32(U_GPIO__GPIO_PORT_MODE_0_H) |= (BIT_20);
    REG32(U_GPIO__GPIO_PORT_MODE_0_H) |= (BIT_22);
#endif

	/*SPI master*/
	REG32(USED_SPI__SSPCR1) &= ( ~U2_SPI__SSPCR1__MS );
	/*frame format motorola*/
	REG32(USED_SPI__SSPCR0) &= ( ~U2_SPI__SSPCR0__FRF );
	/*SPO and SPH = 0: SCLKOUT polarity and phase*/
	REG32(USED_SPI__SSPCR0) &= ( ~U2_SPI__SSPCR0__SPO );
	REG32(USED_SPI__SSPCR0) &= ( ~U2_SPI__SSPCR0__SPH );
	/*SPI data length*/
	REG32(USED_SPI__SSPCR0) &= ( ~U2_SPI__SSPCR0__DSS );
	REG32(USED_SPI__SSPCR0) |= ( SPI_DATA_8BIT );	/*len 8bit*/
	/*SPI baudrate*/
	/*baud = SCLK / ( CPSDVR * ( 1 + SCR ) ; CPSDVR < 2:254 >, only even values; SCR < 0:255 >*/
	/*nearest to 25 MHz is CPSDVR = 6, SCR = 0 -> 125/6*/
	REG32(USED_SPI__SSPCR0)  &= ( ~U2_SPI__SSPCR0__SCR );
	REG32(USED_SPI__SSPCR0)  |= ( 0x00 << 8 );
	REG32(USED_SPI__SSPCPSR)  = ( U2_SPI__SSPCPSR_RESET__VALUE );
	REG32(USED_SPI__SSPCPSR) |= ( 0x06 );	//0xFE for slow, otherwise 0x06
	/*start SPI*/
	REG32(USED_SPI__SSPCR1) |= U2_SPI__SSPCR1__SSE;
	/*chip select - not selected*/
	SPI_CS_HIGH;
	/*set and enable SPI receive interrupt*/
	spi_open(  );
}	/*spi_init(  )*/

#endif	/* #if ( PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P) */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
