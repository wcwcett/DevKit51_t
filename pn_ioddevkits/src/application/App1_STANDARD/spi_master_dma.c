#include "gdma_cfg.h"
#include "gdma_int.h"
#include "arm926.h"


//e
extern TYPED_RLSPI		RlSpiStru;


OS_DATA_FAST unsigned short SPI_Send_Buff[64];//unsigned char SPI_Send_Buff[128];
OS_DATA_FAST unsigned short SPI_Rec_Buff[64];//unsigned char SPI_Rec_Buff[128];

cyg_sem_t dma_lock;

void spiInit_dma2(void)
{
#if 0
	unsigned long src_addr,dest_addr,transfer_control;
	gdma_trans_recx_cnt_reg_t transfer_count;
	gdma_jobx_ctrl_reg_t jobx_ctrl_reg;
	gdma_main_ctrl_reg_t main_ctrl_reg;

	//Init SPI1 interface (GPIO 16,17,18,19 - Alternative A)
	REG32(U_GPIO__GPIO_IOCTRL_0) &= (~BIT_18);	//gpio18 output  mosi
	REG32(U_GPIO__GPIO_IOCTRL_0) &= (~BIT_16);  //gpio16 output  sck
	REG32(U_GPIO__GPIO_IOCTRL_0) |= (BIT_19);	//gpio19 input   miso
	REG32(U_GPIO__GPIO_IOCTRL_0) &= (~BIT_17);	//gpio17 output  cs

	REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= ~(BIT_1);	//bit1=0,
	REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= ~(BIT_5);
	REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= ~(BIT_7);
	REG32(U_GPIO__GPIO_PORT_MODE_0_H) |= (BIT_0);	//bit0=1,	//gpio16 	Alternative A
	REG32(U_GPIO__GPIO_PORT_MODE_0_H) |= (BIT_4);	//gpio18 	Alternative A
	REG32(U_GPIO__GPIO_PORT_MODE_0_H) |= (BIT_6);	//gpio19 	Alternative A
	REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= ~(BIT_2);
	REG32(U_GPIO__GPIO_PORT_MODE_0_H) &= ~(BIT_3);	//c ck=00,GPIO	//gpio17    gpio

	REG32(U_GPIO__GPIO_OUT_SET_0)  |=  1 << 17;		//c CS=1

    REG32(U1_SPI__SSPCR1) &= ( ~U1_SPI__SSPCR1__MS );	//as a master
    /*frame format motorola*/
    REG32(U1_SPI__SSPCR0) &= ( ~U1_SPI__SSPCR0__FRF );	//motorola
    REG32(U1_SPI__SSPCR0) &= ( ~U1_SPI__SSPCR0__SPO );	//C 0 	//OLD,1//REG32(U1_SPI__SSPCR0) |= ( U1_SPI__SSPCR0__SPO );
    REG32(U1_SPI__SSPCR0) |= ( U1_SPI__SSPCR0__SPH );	//1
    REG32(U1_SPI__SSPCR0) &= ( ~U1_SPI__SSPCR0__DSS );
    REG32(U1_SPI__SSPCR0) |= 0x0f;							//C 16bit	//old,8-bit data//REG32(U1_SPI__SSPCR0) |= 7;
    REG32(U1_SPI__SSPCR0)  &= ( ~U1_SPI__SSPCR0__SCR );		//SCR,clear
    REG32(U1_SPI__SSPCR0)  |= 19;							//clock rate //old, (125000000/(10000000*6) - 1) << 8;
//    printf("SCR:%d \r\n", (125000000/(10000000*6) - 1));
    REG32(U1_SPI__SSPCPSR)  = ( U1_SPI__SSPCPSR_RESET__VALUE );	//CPSR=0
    REG32(U1_SPI__SSPCPSR) |=  6 ;	//FCLKOUT=125MHz/(CPSDVR*(1+SCR))
    /*start SPI*/
    REG32(U1_SPI__SSPCR1) |= U1_SPI__SSPCR1__SSE;		//Synchronous serial port enable
    REG32(U1_SPI__SSPCR1) |= U1_SPI__SSPCR1__TIE;		//transmit fifo interrupt enable
    REG32(U1_SPI__SSPCR1) |= U1_SPI__SSPCR1__RIE;		//receive  fifo interrupt enable

	// GDMA Main Control Register page111
	main_ctrl_reg.bits.dma_en = 0;           // disable "global enable"
	main_ctrl_reg.bits.sw_reset = 0;         // disable reset signal
	main_ctrl_reg.bits.jc_reset = 0;
	main_ctrl_reg.bits.list_size = 1;        // total number of transfers in transfer list (0-255), 1 means 2
	main_ctrl_reg.bits.err_int_en = 0;       // disable error interrupt
	GDMA_MAIN_CTRL->reg =  main_ctrl_reg.reg;

	//rx
	jobx_ctrl_reg.bits.transfer_ptr = 0;
	jobx_ctrl_reg.bits.job_prio = 30;  		//DMA job priority
	jobx_ctrl_reg.bits.hw_select = 0; 		//HW Job Start Selector
	jobx_ctrl_reg.bits.job_reset = 0;
	jobx_ctrl_reg.bits.intr_en = 0;  		//disable interrupt
	jobx_ctrl_reg.bits.hw_flow_en = 1;
	jobx_ctrl_reg.bits.hw_job_start_en = 0;
	jobx_ctrl_reg.bits.job_en = 1;
	jobx_ctrl_reg.bits.sw_job_start = 0;
	GDMA_JOBX_CTRL(8)->reg = jobx_ctrl_reg.reg;

	src_addr = U1_SPI__SSPDR;
	dest_addr = (unsigned long)(&SPI_Rec_Buff);
	transfer_control = 0x00800000; 			//SOURCE_AMODE = hold address, DEST_AMODE = incr address, BURST_MODE = single mode   page131
	transfer_count.bits.tr_count = 63;		//63+1 transfer count   page131
	transfer_count.bits.esize = 0x00;		//Element size = 8 bit
	transfer_count.bits.en_dma_ack = 1;		//Enable DMA_ACK
	transfer_count.bits.last_tr = 1;		//Last Transfer
	*GDMA_LIST_RAM_START = src_addr;
	*(GDMA_LIST_RAM_START + 0x01) = dest_addr;
	*(GDMA_LIST_RAM_START + 0x02) = transfer_control;
	*(GDMA_LIST_RAM_START + 0x03) = transfer_count.reg;

	//tx
	jobx_ctrl_reg.bits.transfer_ptr = 1;
	jobx_ctrl_reg.bits.job_prio = 31;  		//DMA job priority
	jobx_ctrl_reg.bits.hw_select = 0; 		//HW Job Start Selector
	jobx_ctrl_reg.bits.job_reset = 0;
	jobx_ctrl_reg.bits.intr_en = 0;  		//disable interrupt
	jobx_ctrl_reg.bits.hw_flow_en = 1;
	jobx_ctrl_reg.bits.hw_job_start_en = 0;
	jobx_ctrl_reg.bits.job_en = 1;
	jobx_ctrl_reg.bits.sw_job_start = 0;
	GDMA_JOBX_CTRL(9)->reg = jobx_ctrl_reg.reg;

	src_addr = (unsigned long)(&SPI_Send_Buff);
	dest_addr = U1_SPI__SSPDR;
	transfer_control = 0x00200000; 			//SOURCE_AMODE = incr address, DEST_AMODE = hold address, BURST_MODE = single
	transfer_count.bits.tr_count = 63;		//63+1 transfer count
	transfer_count.bits.esize = 0x00;		//Element size = 8 bit
	transfer_count.bits.en_dma_ack = 1;		//Enable DMA_ACK
	transfer_count.bits.last_tr = 1;		//Last Transfer
	*(GDMA_LIST_RAM_START + 0x4) = src_addr;
	*(GDMA_LIST_RAM_START + 0x5) = dest_addr;
	*(GDMA_LIST_RAM_START + 0x6) = transfer_control;
	*(GDMA_LIST_RAM_START + 0x7) = transfer_count.reg;

	*GDMA_JC_EN = 0x0300;
	GDMA_MAIN_CTRL->bits.dma_en = 1;

	cyg_semaphore_init(&dma_lock,0);
#endif	
}

//void dma_spi_tx_rx_data(unsigned char *tx_data,unsigned char *rx_data,unsigned short len)
void dma_spi_tx_rx_data2(void)
{
#if 0
	gdma_trans_recx_cnt_reg_t transfer_count;
	unsigned long  i;
	
	unsigned char *tx_data;
	unsigned char *rx_data;
	unsigned short len;
	
	tx_data=RlSpiStru.SPI_Tx_buf;
	rx_data=RlSpiStru.SPI_Rx_buf;
	len=16;
	

	*GDMA_JC_EN = 0x0000;
	GDMA_MAIN_CTRL->bits.dma_en = 0;

	OsMemCpy(SPI_Send_Buff,tx_data,len);//e 8bit	//nnn ttt

	//rx
	transfer_count.bits.tr_count = len - 1;
	transfer_count.bits.esize = 0x00;
	transfer_count.bits.en_dma_ack = 1;
	transfer_count.bits.last_tr = 1;
	*(GDMA_LIST_RAM_START + 0x03) = transfer_count.reg;

	//tx
	transfer_count.bits.tr_count = len - 1;
	transfer_count.bits.esize = 0x00;
	transfer_count.bits.en_dma_ack = 1;
	transfer_count.bits.last_tr = 1;
	*(GDMA_LIST_RAM_START + 0x7) = transfer_count.reg;

	*GDMA_JC_EN = 0x0300;
	GDMA_MAIN_CTRL->bits.dma_en = 1;

	REG32(U_GPIO__GPIO_OUT_CLEAR_0) |= 1 << 17;

    REG32(U1_SPI__SSPIIR_SSPICR) |= U1_SPI__SSPIIR_SSPICR__RIS;
    REG32(U1_SPI__SSPCR1) |= U1_SPI__SSPCR1__RIE;
    REG32(U1_SPI__SSPCR1) |= U1_SPI__SSPCR1__TIE;
	REG32(U1_SPI__SSPCR1) |= U1_SPI__SSPCR1__SSE;

	for(i = 0;i < 2;i++);
	GDMA_JOBX_CTRL(8)->bits.sw_job_start = 1; // GDMA receives data from SPI1's Receive FIFO

	for(i = 0;i < 2;i++);
	GDMA_JOBX_CTRL(9)->bits.sw_job_start = 1; // GDMA sends data to SPI1's Transmit FIFO

	while((REG32(U_GDMA__GDMA_FINISHED_JOBS) & 0x0200) == 0); // Block until Job9 finishes
	while((REG32(U_GDMA__GDMA_FINISHED_JOBS) & 0x0100) == 0); // Block until Job8 finishes
	REG32(U_GDMA__GDMA_FINISHED_JOBS) = 0xffffffff;

	//get DMA receive data
	OsMemCpy(rx_data,SPI_Rec_Buff,len);

	REG32(U_GPIO__GPIO_OUT_SET_0) |= 1 << 17;

	REG32(U1_SPI__SSPCR1) &= ~U1_SPI__SSPCR1__RIE;
	REG32(U1_SPI__SSPCR1) &= ~U1_SPI__SSPCR1__TIE;
	REG32(U1_SPI__SSPIIR_SSPICR) &= ~U1_SPI__SSPIIR_SSPICR__TIS;
	REG32(U1_SPI__SSPIIR_SSPICR) &= ~U1_SPI__SSPIIR_SSPICR__RIS;
	REG32(U1_SPI__SSPCR1) &= ~U1_SPI__SSPCR1__SSE;
#endif	
}



////////////////////////////
