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
/*  F i l e               &F: ecos_ertec_bspadapt.c                     :F&  */
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
* @file     ecos_ertec_bspadapt.c
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

#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
    #include "cm_inc.h"
    #include "compiler_stdlibs.h"
    #include "os.h"
    #include "bspadapt.h"
    #include <cyg/kernel/kapi.h>
    #include "nv_data.h"
    #include "nv_data.h"
    #include "ertec_inc.h"
    #include "ertec_irq.h"

    #include "ertec_x00.h"
    #include <cyg/hal/hal_io.h>
    #include "os_taskprio.h"


    #define LTRC_ACT_MODUL_ID   110
    #define IOD_MODULE_ID       110

    #define SET_THREAD_PRIO_TIMER_THRESHOLD         4


    #define BSP_NUM_ISR_VECTORS (SY_INT_COMMTX926 + 1)  // see ertec_irq.h  definitions SY_INT_N_xxxxx

    PNIO_UINT32 toggleTimer = 0;


    /**
     *  @brief Configure general purpose port direction
     *
     *  @param[in]   port 		port to set direction for
     *  @param[in]   direction 	ERTECX00_GPIO_IN or ERTECX00_GPIO_OUT
     *  @return      			OK, always
     *
     *  This routine configures the general purpose 'port' direction to 'direction'
     *
     */
    PNIO_BOOL ertecx00GpioDirectionSet(PNIO_UINT32             port,       /* port to set direction for */
                                       ERTECX00_GPIO_DIRECTION direction)  /* ERTECX00_GPIO_IN or ERTECX00_GPIO_OUT */
    {
        if (port > 31)
        {
            return (PNIO_NOT_OK);
        }

        // *  Setting to Output:
        if (ERTECX00_GPIO_IN == direction)
        {
            *((volatile LSA_UINT32*) U_GPIO__GPIO_IOCTRL_0) |=   (1UL << port); // set to input
        }
        else
        {
            *((volatile LSA_UINT32*) U_GPIO__GPIO_IOCTRL_0) &=  ~(1UL << port); // set to output
        }
        return (PNIO_OK);
    }


    /**
     *  @brief Set general purpose port function
     *
     *  @param[in]   port 		Port to set function for
     *  @param[in]   function 	ERTECX00_GPIO_FUNC_N (N=0,1,2,3)
     *  @return      			OK, or ERROR if port is out-of-range
     *
     *  This routine sets the general purpose 'port' function to 'function'
     *
     * 'function' can have one of four different values of type
     * ERTECX00_GPIO_FUNCTION:
     *
     * value                | | meaning
     * ---------------------|-|-----------------
     * ERTECX00_GPIO_FUNC_0 | | GPIO function 0
     * ERTECX00_GPIO_FUNC_1 | | GPIO function 1
     * ERTECX00_GPIO_FUNC_2 | | GPIO function 2
     * ERTECX00_GPIO_FUNC_3 | | GPIO function 3
     *
     * The function definitions possible for the different ports are board specific
     * and are therefore defined in the board manual(s).
     */
    PNIO_BOOL ertecx00GpioFunctionSet(PNIO_UINT32            port,       /* port to set function for */
                                      ERTECX00_GPIO_FUNCTION function)   /* ERTECX00_GPIO_FUNC_N (N=0,1,2,3) */
    {
        PNIO_UINT32 RegAddr;
        PNIO_UINT32 GpioOfs;

        if (port < 16)
        {
            RegAddr = U_GPIO__GPIO_PORT_MODE_0_L;
            GpioOfs = port;
        }
        else if (port < 31)
        {
            RegAddr = U_GPIO__GPIO_PORT_MODE_0_H;
            GpioOfs = port - 16;
        }
        else
        {
            return (PNIO_NOT_OK);
        }

        if (function > 3)
        {
            return (PNIO_NOT_OK);
        }

        // ****alternate mode A:
        *((volatile LSA_UINT32*) RegAddr) = (*((volatile LSA_UINT32*) RegAddr) & ~(3UL << (2*GpioOfs))) | (function << (2*GpioOfs));

        return (PNIO_OK);
    }



    /**
     *  @brief Read value from specific general purpose port
     *
     *  @param[in]   port       Port to read input from
     *  @return                 Signal level on port (0 (FALSE, low) or 1 (TRUE, high))
     *
     *
     */
    PNIO_BOOL ertecx00GpioIn(PNIO_UINT32 port /* port to read input from */)
    {
        volatile PNIO_UINT32 gpioInVal = 0L;
        PNIO_UINT32 gpioPortPin;

        /* set port bit number */
        /* read gpio input register */

        gpioPortPin = 1 << port;
        gpioInVal = REG32(U_GPIO__GPIO_IN_0);

        /* return port bit value */
        return ((0 == (gpioInVal & gpioPortPin)) ? false : true);
    }



    /**
     *  @brief Write value to specific general purpose port
     *
     *  @param[in]   port 		Port to write output value to
     *  @param[out]  output 	Output value (0 or 1)
     *  @return      			GPIO output register value
     *
     *  This routine writes the 'output' value to specified general purpose output
     *  'port' and returns the new gpio output register value
     *
     */
    PNIO_UINT32 ertecx00GpioOut(PNIO_UINT32 port,     /* port to write output value to */
                                PNIO_BOOL   output )  /* output value (0 or 1) */
    {
        if (output)
        {
            // PNIO_printf (" ON\n");
            REG32(U_GPIO__GPIO_OUT_SET_0) = (1 << port);
        }
        else
        {
            // PNIO_printf (" OFF\n");
            REG32(U_GPIO__GPIO_OUT_CLEAR_0) = (1 << port);
        }

        return (0);  // read back not supported for E200P
    }



    /**
     *  @brief Initialize the ERTECX00 GPIO
     *
     *  @return      			PNIO_VOID
     *
     *  Note: This routine does nothing. The input/output setup of the specific lines
     *  has to be done by the device driver using that GPIO
     *
     */
    PNIO_VOID ertecX00GpioInit(PNIO_VOID)
    {
        return;
    }


    /**
     *  @brief Sets GPIO (0..31) to GPIO or alternate function 1..3
     *
     *  @param[in]   Gpio 		GPIO number (0..31)
     *  @param[in]   Function 	GPIO (0) or alternate function (1..3)
     *  @param[in]   InOut	 	1: in, 2: out
     *  @return      			PNIO_OK, PNIO_NOT_OK (invalid params)
     *
     *  It is not checked, if the alternate function is available!!
     *
     */
    PNIO_UINT32 Bsp_SetGpioMode (PNIO_UINT32     Gpio,
                                 PNIO_UINT32     Function,
                                 PNIO_GPIO_DIR   InOut)
    {
		PNIO_UINT32 RegAddr;
		PNIO_UINT32 GpioOfs;

		if (Gpio < 16)
		{
			RegAddr = U_GPIO__GPIO_PORT_MODE_0_L;
			GpioOfs = Gpio;
		}
		else if (Gpio < 31)
		{
			RegAddr = U_GPIO__GPIO_PORT_MODE_0_H;
			GpioOfs = Gpio - 16;
		}
		else
		{
			return (PNIO_NOT_OK);
		}

		if (Function > 3)
		{
			return (PNIO_NOT_OK);
		}

		// ****alternate mode A:
		*((volatile LSA_UINT32*) RegAddr) = (*((volatile LSA_UINT32*) RegAddr) & ~(3UL << (2*GpioOfs))) | (Function << (2*GpioOfs));

		// *  put on exit:
		if (InOut == GPIO_DIR_IN)
		{
			*((volatile LSA_UINT32*) U_GPIO__GPIO_IOCTRL_0) |=   (1UL << Gpio); // set to input
		}
		else
		{
			*((volatile LSA_UINT32*) U_GPIO__GPIO_IOCTRL_0) &=  ~(1UL << Gpio); // set to output
		}
		return (PNIO_OK);
    }



    /**
     *  @brief Read GPIOin 0 to 31
     *
     *  @return      			32 bit Input Values
     *
     *  Note: To set the specified GPIOs to input mode, is in the
     *  responsibility of the user. It is not checked in this function
     *
     */
    PNIO_UINT32 Bsp_ReadGPIOin_0_to_31  (PNIO_VOID)
    {
        // ****alternate mode A:
         return (*((volatile LSA_UINT32*) U_GPIO__GPIO_IN_0));
    }



    /**
     *  @brief Sets all GPIO's 0...31, where the Mask bit is set
     *
     *  @param[in]   OutMsk 	Mask bits for GPIO 0..31.
     *  @return      			PNIO_VOID
     *
     *  All GPIO's, where the mask bit is not set, keep unchanged
     *
     */
    PNIO_VOID Bsp_SetGPIOout_0_to_31  (PNIO_UINT32 OutMsk)
    {
         REG32(U_GPIO__GPIO_OUT_SET_0) = OutMsk;
    }



    /**
     *  @brief Clears all GPIO's 0...31, where the Mask bit is set
     *
     *  @param[in]   OutMsk 	Mask bits for GPIO 0..31.
     *  @return      			PNIO_VOID
     *
     *  All GPIO's, where the mask bit is not set, keep unchanged
     *
     */
    PNIO_VOID Bsp_ClearGPIOout_0_to_31  (PNIO_UINT32 OutMsk)
    {
         REG32(U_GPIO__GPIO_OUT_CLEAR_0) = OutMsk;
    }




    /**
     *  @brief Init function, called during system startup
     *
     *  @return      			PNIO_VOID
     *
     *
     */
    PNIO_UINT32 Bsp_Init  (PNIO_VOID)
    {
        BspLed_Open();

		#if ( IOD_INCLUDE_POF == 1 )
        Bsp_PofInit();
		#endif

        //FLASH_INIT;

        return (PNIO_OK);       // nothing to do here
    }



    /**
     *  @brief Reads the device mac address
     *
     *  @param[out]  pDevMacAddr    Destination pointer to the device mac address
     *  @return                     PNIO_OK
     *
     *  Mac address is read from non-volatile data
     *
     */
    PNIO_UINT32 Bsp_GetMacAddr  (PNIO_UINT8* pDevMacAddr)
    {
        PNIO_UINT8* pBootLine;
        PNIO_UINT32 bootlinelength;

        if (PNIO_OK == Bsp_nv_data_restore(PNIO_NVDATA_MAC_ADDR, (PNIO_VOID**)&pBootLine, &bootlinelength))
        {
            OsMemCpy(pDevMacAddr, pBootLine, 6);
            Bsp_nv_data_memfree(pBootLine);
            return (PNIO_OK);
        }

        if (PNIO_OK == Bsp_nv_data_restore(PNIO_NVDATA_PRIVATE_DATA, (PNIO_VOID**)&pBootLine, &bootlinelength))
        {
            OsMemCpy(pDevMacAddr,pBootLine + 0x103,      // offset 0x103 for MAC Address (compatible to history...)
                     6);
        }else
        {
            pBootLine = NULL;
        }

        // no valid BootLine or invalid MAC (since Individual/Group Bit for Broadcast is set)
        if ((0 == pBootLine) || (0x00 != (pDevMacAddr[0] & 0x01)))
        {
            pDevMacAddr[0] = 0x08;
            pDevMacAddr[1] = 0x00;
            pDevMacAddr[2] = 0x06;
            pDevMacAddr[3] = 0x02;
            pDevMacAddr[4] = 0x01;
            pDevMacAddr[5] = 0x10;
        }

        Bsp_nv_data_memfree(pBootLine);
        //sysEnetAddrGet(0, pDevMacAddr);

        return (PNIO_OK);
    }



    /**
     *  @brief Reads the specified port mac address
     *
     *  @param[out]  pPortMacAddr 	Destination pointer to the port address
     *  @param[in]   PortNum 		Port number, must be 1... max. portnum
     *  @return      				PNIO_OK
     *
     *  In this simple example implementation the port mac address is build by
     *  incrementing the device mac address
     *
     */
    PNIO_UINT32 Bsp_GetPortMacAddr  (PNIO_UINT8* pPortMacAddr, PNIO_UINT32 PortNum)
    {
	    PNIO_VAR16 setVal;
	    PNIO_UINT8* port_mac_addr = (PNIO_UINT8*)pPortMacAddr;


        // **** get device MAC address ***
        Bsp_GetMacAddr (pPortMacAddr);

        // **** add the port number to the last 2 bytes of the mac address
        setVal.u8[0] = *(port_mac_addr+4);
        setVal.u8[1] = *(port_mac_addr+5);

        // ***** swap to machine endian format and add port-number ***
        setVal.u16 = OsNtohs (setVal.u16);
	    setVal.u16 += (PortNum);

        // ***** swap back to network endian format ***
        setVal.u16 = OsHtons (setVal.u16);

        // **** modify last 2 bytes of mac address
	    *(port_mac_addr+4) = (setVal.u8[0]);
	    *(port_mac_addr+5) = (setVal.u8[1]);

        return (PNIO_OK);
    }


    /* ------------------------------------------------------------------- */
    /* interrupt handling for the EVMA (event manager) module (application */
    /* timer management)                                                   */
    /* ------------------------------------------------------------------- */

#define   USER_CONTEXT_ISR       0   // 1: call user callback in ISR, else in DSR

OS_BSS_FAST  static cyg_handle_t     bsp_evma_isr_handles [BSP_NUM_ISR_VECTORS];
OS_BSS_FAST  static cyg_interrupt    bsp_evma_isr_memory  [BSP_NUM_ISR_VECTORS];
OS_DATA_FAST static volatile int     bsp_evma_int_context = 0;



    /**
     *  @brief Interrupt isr routine
     *
     *  @param[in]   vector 	Interrupt vector
     *  @param[in]   data	 	Pointer or word of interrupt data
     *  @return      			CYG_ISR_CALL_DSR | CYG_ISR_HANDLED
     *
     *  Data should carry pointer to user interrupt functionality
     *  The function from data will be called in dsr
     *
     */
    OS_CODE_FAST static cyg_uint32 Bsp_EVMA_ISR(cyg_vector_t vector, cyg_addrword_t data)
    {
#if (USER_CONTEXT_ISR)
        PNIO_VOID (*evma_isr)(PNIO_VOID) = (PNIO_VOID (*)(PNIO_VOID))(data);
        cyg_interrupt_disable();
        evma_isr ();
        cyg_interrupt_enable();
        cyg_interrupt_acknowledge(vector);
        return (CYG_ISR_HANDLED);
#else
        cyg_interrupt_disable();
        bsp_evma_int_context++;
        cyg_interrupt_enable();
        cyg_interrupt_acknowledge(vector);
        return ((cyg_uint32)CYG_ISR_HANDLED | (cyg_uint32)CYG_ISR_CALL_DSR);
#endif
    }



#if (USER_CONTEXT_ISR == 0)
    /**
     *  @brief Interrupt dsr routine
     *
     *  @param[in]   vector 	Interrupt vector
     *  @param[in]   count	 	Pending interrupts counter
     *  @param[in]   data	 	Pointer or word of interrupt data
     *  @return      			PNIO_VOID
     *
     *  Data should carry pointer to user interrupt functionality
     *  The function from data will be called here
     *
     */
    OS_CODE_FAST static PNIO_VOID Bsp_EVMA_DSR (cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
    {
        /* note: count should be always 1 */
        PNIO_VOID (*evma_dsr)(PNIO_VOID) = (PNIO_VOID (*)(PNIO_VOID))(data);

        evma_dsr();
        cyg_interrupt_disable();
        bsp_evma_int_context-= count;
        if (bsp_evma_int_context < 0)
            bsp_evma_int_context = 0;
        cyg_interrupt_enable();
    }
#endif



        /**
         *  @brief Register interrupt isr routine to interrupt handler
         *
         *  @param[in]   vector 	Interrupt vector
         *  @param[in]   *isr	 	Pointer to the isr
         *  @return      			PNIO_VOID
         *
         *
         */
    PNIO_VOID Bsp_EVMA_register_ISR(PNIO_UINT32 vector, PNIO_VOID (*isr)(PNIO_VOID))
    {
        if (isr)
        {
            PNIO_INT32 handle_index = vector;  // - EVMA_FIRST_ISR_VECTOR_INDEX;

            if(handle_index >= 0 && handle_index < BSP_NUM_ISR_VECTORS)
            {
#if (USER_CONTEXT_ISR)
                cyg_interrupt_create(vector, 0, (cyg_addrword_t) isr, Bsp_EVMA_ISR, NULL, &bsp_evma_isr_handles[handle_index], &bsp_evma_isr_memory[handle_index]);
#else
                cyg_interrupt_create(vector, 0, (cyg_addrword_t) isr, Bsp_EVMA_ISR, Bsp_EVMA_DSR, &bsp_evma_isr_handles[handle_index], &bsp_evma_isr_memory[handle_index]);
#endif
                cyg_interrupt_attach(bsp_evma_isr_handles[handle_index]);
                cyg_interrupt_unmask(vector);
            }
        }
        else /* unregister ISR when called with NULL pointer */
        {
            Bsp_EVMA_unregister_ISR(vector);
        }
    }



    /**
     *  @brief Remove isr, mask its interrupt
     *
     *  @param[in]   vector 	Interrupt vector
     *  @return      			PNIO_VOID
     *
     *  Basically remove interrupt
     *
     */
    PNIO_VOID Bsp_EVMA_unregister_ISR(PNIO_UINT32 vector)
    {
        PNIO_INT32 handle_index = vector;  //  - EVMA_FIRST_ISR_VECTOR_INDEX;

        if(handle_index >= 0 && handle_index < BSP_NUM_ISR_VECTORS)
        {
            cyg_interrupt_mask(vector);
            cyg_interrupt_detach(bsp_evma_isr_handles[handle_index]);
            cyg_interrupt_delete(bsp_evma_isr_handles[handle_index]);
        }
    }


    volatile int ECOS_intContext = 0;



    /**
     *  @brief Interrupt isr routine
     *
     *  @param[in]   vector 	Interrupt vector
     *  @param[in]   data	 	Pointer or word of interrupt data
     *  @return      			CYG_ISR_CALL_DSR | CYG_ISR_HANDLED
     *
     *  Data should carry pointer to user interrupt functionality
     *  The function from data will be called in dsr
     *
     */
    OS_CODE_FAST static cyg_uint32 Bsp_ISR(cyg_vector_t vector, cyg_addrword_t data)
    {
    	cyg_interrupt_disable();
    	++ECOS_intContext;
    	cyg_interrupt_enable();
    	cyg_interrupt_acknowledge(vector);
    	return ((cyg_uint32)CYG_ISR_CALL_DSR | (cyg_uint32)CYG_ISR_HANDLED);
    }



    /**
     *  @brief Interrupt dsr routine
     *
     *  @param[in]   vector 	Interrupt vector
     *  @param[in]   count	 	Pending interrupts counter
     *  @param[in]   data	 	Pointer or word of interrupt data
     *  @return      			PNIO_VOID
     *
     *  Data should carry pointer to user interrupt functionality
     *  The function from data will be called here
     *
     */
    OS_CODE_FAST static PNIO_VOID Bsp_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
    {
    	/* note: count should be always 1 */
    	PNIO_VOID (*DSRCall)(PNIO_VOID) = (PNIO_VOID (*)(PNIO_VOID))(data);
    	DSRCall();
    	cyg_interrupt_disable();
    	ECOS_intContext-= count;
    	cyg_interrupt_enable();
    	if (ECOS_intContext < 0)
    		ECOS_intContext = 0;
    }



    /**
     *  @brief Assigns interrupts to interrupt handler
     *
     *  @param[in]   pErtecSwiIntH 	High priority switch interrupts
     *  @param[in]   pErtecSwiIntL 	Low priority switch interrupts
     *  @return      				PNIO_OK
     *
     *
     */
    PNIO_UINT32 Bsp_ErtecSwiIntConnect(PNIO_CBF_ERTEC_SWI_INT_H pErtecSwiIntH,
                                       PNIO_CBF_ERTEC_SWI_INT_L pErtecSwiIntL)
    {
        static cyg_interrupt intH;
        static cyg_interrupt intL;
        static cyg_handle_t hIntH;
        static cyg_handle_t hIntL;

        cyg_interrupt_create(SY_INT_PN_IRQ2_2,0,(cyg_addrword_t)pErtecSwiIntH,Bsp_ISR,Bsp_DSR,&hIntH,&intH);
        cyg_interrupt_create(SY_INT_PN_IRQ2_0,0,(cyg_addrword_t)pErtecSwiIntL,Bsp_ISR,Bsp_DSR,&hIntL,&intL);
        cyg_interrupt_attach(hIntL);
        cyg_interrupt_attach(hIntH);
        cyg_interrupt_unmask(SY_INT_PN_IRQ2_2);
        cyg_interrupt_unmask(SY_INT_PN_IRQ2_0);

        return (PNIO_OK);
    }



    /**
     *  @brief Assigns error interrupts to interrupt handlers
     *
     *  @param[in]   pErtecParityErrInt 	Parity error interrupt
     *  @param[in]   pErtecAccessErrInt 	Access error interrupt
     *  @return      						PNIO_OK
     *
     *
     */
    PNIO_UINT32 Bsp_ErtecErrIntConnect(PNIO_CBF_ERTEC_PARITY_ERR_INT   pErtecParityErrInt,
                                       PNIO_CBF_ERTEC_ACCESS_ERR_INT   pErtecAccessErrInt)
    {
        static cyg_interrupt intParityErr;
        static cyg_interrupt intAhbAccessErr;
        static cyg_interrupt intApbAccessErr;
        static cyg_interrupt intEmcAccessErr;

        static cyg_handle_t hIntParityErr;
        static cyg_handle_t hIntAhbAccessErr;
        static cyg_handle_t hIntApbAccessErr;
        static cyg_handle_t hIntEmcAccessErr;

        cyg_interrupt_create(SY_INT_EDC_EVENT,         0, (cyg_addrword_t)pErtecParityErrInt, Bsp_ISR, Bsp_DSR, &hIntParityErr,    &intParityErr);
        cyg_interrupt_create(SY_INT_AHB_ADDRESS_ERROR, 0, (cyg_addrword_t)pErtecAccessErrInt, Bsp_ISR, Bsp_DSR, &hIntAhbAccessErr, &intAhbAccessErr);
        cyg_interrupt_create(SY_INT_APB_ADDRESS_ERROR, 0, (cyg_addrword_t)pErtecAccessErrInt, Bsp_ISR, Bsp_DSR, &hIntApbAccessErr, &intApbAccessErr);
        cyg_interrupt_create(SY_INT_EMC_ADDRESS_ERROR, 0, (cyg_addrword_t)pErtecAccessErrInt, Bsp_ISR, Bsp_DSR, &hIntEmcAccessErr, &intEmcAccessErr);

        cyg_interrupt_attach(hIntParityErr);
        cyg_interrupt_attach(hIntAhbAccessErr);
        cyg_interrupt_attach(hIntApbAccessErr);
        cyg_interrupt_attach(hIntEmcAccessErr);

        cyg_interrupt_unmask(SY_INT_EDC_EVENT);
        cyg_interrupt_unmask(SY_INT_AHB_ADDRESS_ERROR);
        cyg_interrupt_unmask(SY_INT_APB_ADDRESS_ERROR);
        cyg_interrupt_unmask(SY_INT_EMC_ADDRESS_ERROR);

        return (PNIO_OK);
    }


#if (IOD_INCLUDE_POF == 1)
    /**
     *  @brief Init GPIO for optical fiber communication
     *
     *  @return                          PNIO_VOID
     *
     *
     */
    PNIO_VOID Bsp_PofInit (PNIO_VOID)
    {
        Bsp_SetGpioMode (GPIO_POF1_SCL, 3, GPIO_DIR_OUT);
        Bsp_ClearGPIOout_0_to_31 (1<<GPIO_POF1_SCL);
        Bsp_SetGpioMode (GPIO_POF1_SDA, 3, GPIO_DIR_OUT);
        Bsp_ClearGPIOout_0_to_31 (1<<GPIO_POF1_SDA);

        Bsp_SetGpioMode (GPIO_POF2_SCL, 3, GPIO_DIR_OUT);
        Bsp_ClearGPIOout_0_to_31 (1<<GPIO_POF2_SCL);
        Bsp_SetGpioMode (GPIO_POF2_SDA, 3, GPIO_DIR_OUT);
        Bsp_ClearGPIOout_0_to_31 (1<<GPIO_POF2_SDA);
    }
#endif

#define BSP_WD_TIME	10		/*multiple of 100ms*/



    /**
     *  @brief Prepare watchdog timer, set time before reset and reset method
     *
     *  @param[in]   wd_time            Time multiplied by wd_granity
     *  @param[in]   wd_granity         Time base [BSP_WD_100MS, BSP_WD_10MS, BSP_WD_1MS, BSP_WD_100US]
     *
     *  @return                         PNIO_VOID
     *
     *
     */
    PNIO_VOID Bsp_hw_watchdog_init(PNIO_UINT32 wd_time, BSP_WD_GRANITY wd_granity)
    {
        /*all wd settings registers are write protect*/
        /*to write them, there have to be password 0x9876 in upper 4 bytes of register*/
        PNIO_UINT32 load_low, load_high, timer_base = 0xBEBC2;

        switch ( wd_granity )
        {
            case BSP_WD_100MS:
            {
                timer_base = 0xBEBC2;
                break;
            }
            case BSP_WD_10MS:
            {
                timer_base = 0x1312D;
                break;
            }
            case BSP_WD_1MS:
            {
                timer_base = 0x1E84;    //truncated
                break;
            }
            case BSP_WD_100US:
            {
                timer_base = 0x30D;     //truncated
                break;
            }
        }

        /*WD 1 have 36 bit counter on 125MHz clock*/
        /*100 ms = 0xBEBC20*/
        /*RELD1 high and low gives together 32 bits for 36 bit value - ignore lowest 4 bits*/
        /*==> 100 ms = 0xBEBC2 written to register*/
        load_high = timer_base * wd_time;
        load_low = load_high & 0x0000ffff;
        load_low |= 0x98760000;
        load_high = load_high >> 16;
        load_high &= 0x0000ffff;
        load_high |= 0x98760000;

        REG32(U_WDOG__RELD1_LOW) =  load_low;       // The watchdog is raised to 251 ms
        REG32(U_WDOG__RELD1_HIGH) = load_high;
        REG32(U_WDOG__WD_CTRL_STATUS) = 0x98760004; //password + reload

        REG32(U_SCRB__ASYN_RES_CTRL_REG) |= 0x01;   // unlock in SCRB
    }



    /**
     *  @brief Deinitialize WDT, turn off reset by WDT
     *
     *
     *  @return                         PNIO_VOID
     *
     *
     */
    PNIO_VOID Bsp_hw_watchdog_deinit( PNIO_VOID )
    {
        REG32(U_WDOG__WD_CTRL_STATUS) = 0x98760000;  // password
        REG32(U_SCRB__ASYN_RES_CTRL_REG) &= ( ~( 0x01 ) );  // turn off wd reset
        REG32(U_WDOG__RELD1_LOW) =  0;
        REG32(U_WDOG__RELD1_HIGH) = 0;
    }



    /**
     *  @brief Start WDT. Have to be initialiyed by user before
     *
     *
     *  @return                         PNIO_VOID
     *
     *
     */
    PNIO_VOID Bsp_hw_watchdog_start( PNIO_VOID )
    {
        REG32(U_WDOG__WD_CTRL_STATUS) = 0x98760006;  // password + trigger + WD1RUN
    }



    /**
     *  @brief Stop (pause) WDT
     *
     *
     *  @return                         PNIO_VOID
     *
     *    Don't forget to trigger before restart
     *
     */
    PNIO_VOID Bsp_hw_watchdog_stop( PNIO_VOID )
    {
        REG32(U_WDOG__WD_CTRL_STATUS) = 0x98760000;  // password
    }



    /**
     *  @brief Trigger WDT - reset wd timer
     *
     *
     *  @return                         PNIO_VOID
     *
     *
     */
    PNIO_VOID Bsp_hw_watchdog_trigger( PNIO_VOID )
    {
        REG32(U_WDOG__WD_CTRL_STATUS) |= 0x98760004;  // password + trigger + WD1RUN
    }


    /**
     *  @brief Register Interrupt routine for acyc. receive GPIO
     *
     *  @param          int_vector      Number of interrupt vector
     *  @param          gpio_isr        Function called as isr(have to contain acknowledge of interrupt)
     *  @param          gpio_dsr        Function called as dsr
     *
     *  @return         PNIO_VOID
     *
     *  Settings of interrupt handling
     *
     */
    PNIO_VOID Bsp_GPIO_set_int_for_acyc(PNIO_UINT32 int_vector, PNIO_VOID* gpio_isr, PNIO_VOID* gpio_dsr)
    {

        static cyg_interrupt intH;
        static cyg_handle_t hIntH;
        //lint -e{611} Suspicious cast
        cyg_interrupt_create(
                int_vector,        /*vector*/
                0,                      /*priority*/
                0,                      /*data ptr*/
                ( cyg_ISR_t * )gpio_isr, /* *isr*/
                ( cyg_DSR_t * )gpio_dsr, /* *dsr*/
                &hIntH,                 /* *return handle*/
                &intH);
        /* *interrupt*/
        cyg_interrupt_attach( hIntH );
        cyg_interrupt_unmask( int_vector );
    }   /* Bsp_GPIO_set_int_for_acyc */


    /**
     *  @brief Register Interrupt routine for acyc. receive GPIO
     *
     *  @param          int_vector      Number of interrupt vector
     *  @param          gpio_isr        Function called as isr(have to contain acknowledge of interrupt)
     *  @param          gpio_dsr        Function called as dsr
     *
     *  @return         PNIO_VOID
     *
     *  Settings of interrupt handling
     *
     */
    PNIO_VOID Bsp_GPIO_set_int_for_acyc_confirm(PNIO_UINT32 int_vector, PNIO_VOID* gpio_isr, PNIO_VOID* gpio_dsr)
    {

        static cyg_interrupt intH;
        static cyg_handle_t hIntH;
        //lint -e{611} Suspicious cast
        cyg_interrupt_create(
                int_vector,        /*vector*/
                0,                      /*priority*/
                0,                      /*data ptr*/
                ( cyg_ISR_t * )gpio_isr, /* *isr*/
                ( cyg_DSR_t * )gpio_dsr, /* *dsr*/
                &hIntH,                 /* *return handle*/
                &intH);
        /* *interrupt*/
        cyg_interrupt_attach( hIntH );
        cyg_interrupt_unmask( int_vector );
    }   /* Bsp_GPIO_set_int_for_acyc_confirm */


    /**
     *  @brief Interrupt acknowledge for isr routine
     *
     *  @param          int_vector      Number of interrupt vector
     *
     *  @return         PNIO_VOID
     *
     *  Acknowledge interrupt
     *
     */
    PNIO_VOID Bsp_GPIO_isr(PNIO_UINT32 int_vector)
    {
        cyg_interrupt_acknowledge(int_vector);
    }   /* PnUsr_xhif_acyc_isr */


#endif


/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
