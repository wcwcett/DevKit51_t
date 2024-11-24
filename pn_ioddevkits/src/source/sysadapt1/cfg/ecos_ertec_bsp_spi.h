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
/*  F i l e               &F: ecos_ertec_bsp_spi.h                      :F&  */
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
 * ecos_ertec_bsp_spi.h
 *
 *  Created on: 15. 6. 2016
 *      Author: z003dbfm
 */

#ifndef SYSADAPT1_CFG_ECOS_ERTEC_BSP_SPI_H_
#define SYSADAPT1_CFG_ECOS_ERTEC_BSP_SPI_H_

#include <cyg/hal/ertec200p_reg.h>

/*
 * Set USED_SPI_MODULE to 1 if the SPI1 shall be used
 * instead of the SPI2
 *
 * Important note:
 * If SPI1 is used, it is necessary to change GPIO settings
 * in the MainAppl, because by default the SPI2 GPIOs are
 * configured to SPI alternative function.
 *
 * Following code has to be used:
 *
 * if( MODE_SPI_0x03 == ( REG32( U_SCRB__BOOT_REG ) & 0xf ) )
 * {
 *      REG32(U_GPIO__GPIO_IOCTRL_0) &= (~(1 << 21));
 *      REG32(U_GPIO__GPIO_IOCTRL_0) &= (~(1 << 18));
 *      REG32(U_GPIO__GPIO_IOCTRL_0) &= (~(1 << 16));
 *      REG32(U_GPIO__GPIO_IOCTRL_0) |= (1 << 19);
 *
 *      REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= (~(1 << 1));
 *      REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= (~(1 << 5));
 *      REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= (~(1 << 7));
 *      REG32(U_GPIO__GPIO_PORT_MODE_0_H) |= (1 << 0);
 *      REG32(U_GPIO__GPIO_PORT_MODE_0_H) |= (1 << 4);
 *      REG32(U_GPIO__GPIO_PORT_MODE_0_H) |= (1 << 6);
 * }
 */
#define USED_SPI_MODULE             2

/* SPI 2 interrupts */
#define SY_INT_SPI2_TFE             30
#define SY_INT_SPI2_RNE             31
/* SPI 1 interrupts */
#define SY_INT_SPI1_TFE             28
#define SY_INT_SPI1_RNE             29

#if (USED_SPI_MODULE == 1)
/* Interrupt vectors number */
#define SY_INT_SPI_TFE              SY_INT_SPI1_TFE
#define SY_INT_SPI_RNE              SY_INT_SPI1_RNE

/* Chip select macros */
#define SPI_CS_HIGH 			    REG32(U_GPIO__GPIO_OUT_SET_0) = (0xFFFFFFFF & 1) << 21
#define SPI_CS_LOW				    REG32(U_GPIO__GPIO_OUT_CLEAR_0) = (0xFFFFFFFF & 1) << 21

/* Control registers */
#define USED_SPI__SSPSR             U1_SPI__SSPSR
#define USED_SPI__SSPDR             U1_SPI__SSPDR
#define USED_SPI__SSPCR1            U1_SPI__SSPCR1
#define USED_SPI__SSPCR0            U1_SPI__SSPCR0
#define USED_SPI__SSPCPSR           U1_SPI__SSPCPSR
#else
/* Interrupt vectors number */
#define SY_INT_SPI_TFE              SY_INT_SPI2_TFE
#define SY_INT_SPI_RNE              SY_INT_SPI2_RNE

/* Chip select macros */
#define SPI_CS_HIGH 			    REG32(U_GPIO__GPIO_OUT_SET_0) = (0xFFFFFFFF & 1) << 31
#define SPI_CS_LOW				    REG32(U_GPIO__GPIO_OUT_CLEAR_0) = (0xFFFFFFFF & 1) << 31

/* Control registers */
#define USED_SPI__SSPSR             U2_SPI__SSPSR
#define USED_SPI__SSPDR             U2_SPI__SSPDR
#define USED_SPI__SSPCR1            U2_SPI__SSPCR1
#define USED_SPI__SSPCR0            U2_SPI__SSPCR0
#define USED_SPI__SSPCPSR           U2_SPI__SSPCPSR
#endif

#define RXbuff_CAPACITY     1024


typedef enum
{
	SPI_DATA_4BIT = 0x03,
	SPI_DATA_5BIT = 0x04,
	SPI_DATA_6BIT = 0x05,
	SPI_DATA_7BIT = 0x06,
	SPI_DATA_8BIT = 0x07,
	SPI_DATA_9BIT = 0x08,
	SPI_DATA_10BIT = 0x09,
	SPI_DATA_11BIT = 0x0A,
	SPI_DATA_12BIT = 0x0B,
	SPI_DATA_13BIT = 0x0C,
	SPI_DATA_14BIT = 0x0D,
	SPI_DATA_15BIT = 0x0E,
	SPI_DATA_16BIT = 0x0F
}SPI_DATA_LEN;


typedef struct RXbuff
{
	volatile unsigned char buff[RXbuff_CAPACITY];
	volatile unsigned int read_ptr;
	volatile unsigned int write_ptr;
	volatile unsigned int no_records;	/*number of records in buffer*/
}RXbuff;
volatile RXbuff spi_rec_buff;





/********************************public functions*******************************/
void 					spi_init			( void );
void 					spi_send_byte		(unsigned char i_data);
unsigned char 			spi_read_byte		( void );
unsigned char 			spi_data_to_read	( void );
void 					spi_rxb_flush		( void );

#endif /* SYSADAPT1_CFG_ECOS_ERTEC_BSP_SPI_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
