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
/*  F i l e               &F: pnpb_lib_int.c                            :F&  */
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
* @file     pnpb_lib_int.c
* @brief    PNPB library for XHIF - internal functions
*
* XHIF functionality allows user to use ERTEC Devkit only for PN-stack functionalities
* and to realize user functionality on other device. The other device have to upload
* firmware to ERTEC Devkit as a binary and then communicate data.. This can be realized
* via XHIF memory interface.
* This file is such implementation developed for BeagleBone Black driven by TI Sitara
* processor.
*/

#include "pnpb_lib.h"
#include "pnpb_lib_int.h"
#include "pnpb_lib_mem_int.h"
#include <poll.h>
#include <time.h>
#include <sched.h>
#include "pnpb_gpio_lib.h"

#define PNPB_LIB_SET_VERBOSE			0	/* 1 = enable printouts for debugging */

/* Semaphore handles for both cyclic and acyclic data */
sem_t PnioAcSem;
sem_t PnioAcSemBack;
sem_t PnioCycSem;
sem_t PnioCycSemBack;

/**
 *  @brief Printout of Devkit version to console
 *
 *  @param[in]      void
 *
 *  @return                             Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Always succeeds
 *
 */
PNIO_UINT32 PrintDevkitVersion (void)
{
    PNIO_UINT32  Status = 0;


    printf("PNIO DEVKIT appl.-example no.%d, %c %d.%d.%d.%d, %s - %s\n",
               EXAMPL_DEV_CONFIG_VERSION, DeviceFwVersion.VerPrefix,
               DeviceFwVersion.VerHh, DeviceFwVersion.VerH, DeviceFwVersion.VerL, DeviceFwVersion.VerLl,
               __DATE__, __TIME__);
	printf("DAP %d\n", MODULE_ID_DAP);
    return (Status);
}   /* PrintDevkitVersion */


/**
 *  @brief Reads one character from keyboard (console)
 *
 *  @param[in]      void
 *
 *  @return                             One character from console
 *
 *  Blocking
 *
 */
PNIO_UINT8  OsGetChar (void)
{
    PNIO_INT8 Char = 0;

    while ((Char == 0x0a) || (Char == 0x00))   // hide carriage return and NUL
    {
        Char = getchar();
        if ( Char == EOF )
            printf("Getchar error caused by %d\n", errno);
    }
    return ( ( PNIO_UINT8 ) Char );
}   /* OsGetChar */

/**
 * @brief get formated number from input
 *
 * @param[in]   pText       PNIO_INT8*     text which should be sent to stdout
 * @param[in]   InputBase   PNIO_UINT32    format input for output number 16 : hexadecimal, 10 : decimal
 *
 * @return  PNIO_UINT32 - returned hexadecimal / decimal number
 *
 */
PNIO_UINT32 OsKeyScan32 (PNIO_INT8* pText, PNIO_UINT32 InputBase)
{
   PNIO_UINT32 val32 = 0;

   if (pText)
   {
       printf ("%s", pText);
   }

   if (InputBase == 16)
   {
       if ( scanf ("%x", (int *) &val32) == EOF )
           printf ("scanf error\n");
   }
   else if (InputBase == 10)
   {
       if ( scanf ("%d", (int *) &val32) == EOF )
           printf ("scanf error\n");
   }
   else
   {
       printf("no valid InputBase\n");
   }

   return (val32);
}

/**
 * @brief get formated string from input
 *
 * @param[in]   pText       PNIO_INT8*     text which should be sent to stdout
 * @param[in]   *pStr       PNIO_UINT32*   pointer to input buffer
 *
 * @return  PNIO_UINT32 - length of read string
 *
 */
PNIO_UINT32 OsKeyScanString( PNIO_INT8* pText, PNIO_UINT8* pStr, PNIO_UINT32 MaxLen )
{
    PNIO_INT32 len;
    PNIO_UINT32 NewChar;
    if( pText )
    {
        printf("%s", pText );
    }
    for( len = 0; len < MaxLen; )
    {
        NewChar = getchar(  );
        if( 0x0a == NewChar )   /* enter */
        {
            /* ignore first enter */
            if( 0 == len )
            {
                /* do nothing, not even increment len */
            }
            else
            {   /* some chars, then enter = valid input ended by enter */
                pStr[ len ] = 0x00; /* \0 */
                break;
            }
        }
        else
        {
            pStr[ len ] = NewChar;
            len++;
        }
    }
    if( len == MaxLen )
    {
        printf( "ERROR: Input sequence too long\n" );
        return 0;
    }
    return len; /* do not count last \0 character */
}

/**
 * @brief Get time for UUID generation
 *
 * @return PNIO_UINT64 - time in 100us
 */
PNIO_UINT64 OsGetUuidTime (void)
{
    PNIO_UINT64 time_uuid;
    struct timespec t;

    // RFC constant
    static const PNIO_UINT64 num100nsec1582    = 0x01b21dd213814000;

    // get time
    if ( clock_gettime (CLOCK_REALTIME, &t) == -1 )
    {
        printf("Error in clock_gettime :%d ()\n", errno );
    }

    // convert to 100us format
    time_uuid = t.tv_sec * 10000000;
    time_uuid = time_uuid + (t.tv_nsec / 100);
    time_uuid = time_uuid + num100nsec1582;

    // return
    return time_uuid;
}

/**
 *  @brief Scan user input for IP address
 *
 *  @param[in]      *ip_suite    Pointer where to store IP address
 *  @return         void
 *
 */
PNIO_VOID pnpb_scan_ip_address(PNIO_UINT8* ip_suite)
{
    PNIO_UINT8 Val;  /* decimal imput */
    PNIO_UINT8 i;

    printf( "Current IP address: %03d.%03d.%03d.%03d\n",
                  *( ip_suite + 0 ), *( ip_suite + 1 ), *( ip_suite + 2 ), *( ip_suite + 3 ) );
    printf( "Mask:               %03d.%03d.%03d.%03d\n",
                  *( ip_suite + 4 ), *( ip_suite + 5 ), *( ip_suite + 6 ), *( ip_suite + 7 ) );
    printf( "Default gateway:    %03d.%03d.%03d.%03d\n",
                  *( ip_suite + 8 ), *( ip_suite + 9 ), *( ip_suite + 10 ), *( ip_suite + 11 ) );

    printf ( "Modify 4 bytes of IP address, decadic input\n" );
    for( i = 0; i < 4; i++ )
    {
        printf( "%03d - ", *( ip_suite + i + 0 ) );
        Val = OsKeyScan32( NULL, 10 );
        if( 255 < Val )
        {
            printf( "Inputed value too big, truncated to unsigned 8bit/n");
        }
        *( ip_suite + i + 0 ) = ( PNIO_UINT8 ) Val;
    }

    printf ( "Modify 4 bytes of IP mask, decadic input\n" );
    for( i = 0; i < 4; i++ )
    {
        printf( "%03d - ", *( ip_suite + i + 4 ) );
        Val = OsKeyScan32( NULL, 10 );
        if( 255 < Val )
        {
            printf( "Inputed value too big, truncated to unsigned 8bit/n");
        }
        *( ip_suite + i + 4 ) = ( PNIO_UINT8 ) Val;
    }

    printf ( "Modify 4 bytes of default gateway, decadic input\n" );
    for( i = 0; i < 4; i++ )
    {
        printf( "%03d - ", *( ip_suite + i + 8 ) );
        Val = OsKeyScan32( NULL, 10 );
        if( 255 < Val )
        {
            printf( "Inputed value too big, truncated to unsigned 8bit/n");
        }
        *( ip_suite + i + 8 ) = ( PNIO_UINT8 ) Val;
    }
}

/**
 *  @brief Scan user input for MAC address
 *
 *  @param[in]      *mac_addr    Pointer where to store MAC address
 *  @return         void
 *
 */
PNIO_VOID pnpb_scan_mac_address(PNIO_UINT8* mac_addr)
{
    PNIO_UINT32 i;
    PNIO_UINT32 Val;  // hexadecimal input

    printf("Current Ethernet address is: %02x:%02x:%02x:%02x:%02x:%02x\n",
                      *(mac_addr+0), *(mac_addr+1), *(mac_addr+2),
                      *(mac_addr+3), *(mac_addr+4), *(mac_addr+5));

    for (i = 0; i < 6; i++)
    {
        printf((PNIO_INT8*) "%02x - ", *(mac_addr + i));
        Val = OsKeyScan32(NULL, 16);
        *(mac_addr + i) = (PNIO_UINT8) Val;
    }
}

/**
 *  @brief Function will poll acyclic semaphore gpio
 *
 */
PNIO_VOID* poll_acyc_sem_gpio(PNIO_VOID * i)
{
    PNIO_UINT32 val;
    PNIO_UINT32 last_val;
    const struct timespec delay = {0, 100000L};
    struct timespec rem;
    pthread_t this_thread = pthread_self();
    struct sched_param th_param;

    /* Configure this thread to have low priority */
    th_param.sched_priority = sched_get_priority_min (SCHED_RR);
    if (pthread_setschedparam(this_thread, SCHED_RR, &th_param) != 0)
    {
        printf("Thread priority (poll_acyc_sem_gpio - low prio) configuration failed!");
        PNPB_LIB_FATAL
    }
	//lint --e{578} Declaration of symbol 'Symbol' hides symbol 'Symbol' (Location)
	last_val = bbb_gpio_get(GPIO_BANK_3, GPIO_NUM_19);

    /* Signalize that this thread is running */
    acyc_gpio_running = PNPB_THREAD_RUNNING;

    /* Send trigger to Ertec, that we are ready to capture */
    printf("Signalizing Ertec that we are ready...\n");
    bbb_gpio_set(GPIO_BANK_0, GPIO_NUM_14, 1);
    /* Sleep for predefined delay */
    nanosleep(&delay, &rem);
    bbb_gpio_set(GPIO_BANK_0, GPIO_NUM_14, 0);

    /* Run till the termination flag occurs */
    while (pnpb_keep_running == PNPB_THREAD_RUNNING)
    {
        /* Wait for event */
        do
        {
            val = bbb_gpio_get(GPIO_BANK_3, GPIO_NUM_19);
            if(last_val != val)
            {
                break;
            }
            else
            {
                /* Sleep for predefined delay */
                nanosleep(&delay, &rem);
            }
        } while (pnpb_keep_running == PNPB_THREAD_RUNNING);

        /* Check whether the loop was terminated by
         * signal or GPIO change and take action */
        if(pnpb_keep_running == PNPB_THREAD_RUNNING)
        {
            last_val = val;

            sem_post(&PnioAcSem);
            sem_wait(&PnioAcSemBack);
        }
    }

    acyc_gpio_running = PNPB_THREAD_TERMINATE;
    /* Post semaphore to allow pnpb_acyclical_capture_thread
     * to terminate */
    sem_post(&PnioAcSem);

    return NULL;
}

/**
 *  @brief Main function of thread for capturing incomming acyclical telegrams of GPMC - XHIF memory interface
 *
 *  @param[in]      *i                  Dummy param
 *  @return         void
 *
 *  Functions hang - waits for change on HW pin. For this is used blocking function poll.
 *
 */
PNIO_VOID * pnpb_acyclical_capture_thread(PNIO_VOID * i)
{
    pthread_t gpio_sem_handler;
    pthread_t this_thread = pthread_self();
    struct sched_param th_param;

    /* Configure this thread to have low priority */
    th_param.sched_priority = sched_get_priority_min (SCHED_RR);
    if (pthread_setschedparam(this_thread, SCHED_RR, &th_param) != 0)
    {
        printf("Thread priority (poll_acyc_sem_gpio - low prio) configuration failed!");
        PNPB_LIB_FATAL
    }

    /* Init semaphore before use */
    sem_init(&PnioAcSem, 0, 0);
    sem_init(&PnioAcSemBack, 0, 0);

    /* Signalize that this thread is running */
    acyc_service_running = PNPB_THREAD_RUNNING;

    /* Start thread for acyclic capture */
    pthread_create(&gpio_sem_handler, NULL, poll_acyc_sem_gpio, NULL);

    /* Run till the termination flag occurs */
    while (pnpb_keep_running == PNPB_THREAD_RUNNING)
    {
        sem_wait(&PnioAcSem);

        /* Check whether the termination flag occurs
         * during semaphore wait */
        if(pnpb_keep_running == PNPB_THREAD_RUNNING)
        {
            pnpb_xhif_acyc_read();

            /* Signalize to ERTEC that we received the trigger */
            bbb_gpio_set(GPIO_BANK_3, GPIO_NUM_21, 1);

            bbb_gpio_set(GPIO_BANK_3, GPIO_NUM_21, 0);

            sem_post(&PnioAcSemBack);
        }
    }

    /* Destroy semaphores */
    sem_destroy(&PnioAcSem);
    sem_destroy(&PnioAcSemBack);

    /* Signalize that this thread was terminated */
    acyc_service_running = PNPB_THREAD_TERMINATE;

    return NULL;
}   /* pnpb_acyclical_capture_thread */


/**
 *  @brief Function will poll cyclic semaphore gpio
 *
 */
PNIO_VOID* poll_cyc_sem_gpio(PNIO_VOID * i)
{
    PNIO_UINT32 val;
    PNIO_UINT32 last_val;
    const struct timespec delay = {0, 20000L};
    struct timespec rem;
    pthread_t this_thread = pthread_self();
    struct sched_param th_param;

    /* Configure this thread to have high priority */
    th_param.sched_priority = sched_get_priority_max (SCHED_RR);
    if (pthread_setschedparam(this_thread, SCHED_RR, &th_param) != 0)
    {
        printf("Thread priority (poll_cyc_sem_gpio - high prio) configuration failed!");
        PNPB_LIB_FATAL
    }

	//lint --e{578} Declaration of symbol 'Symbol' hides symbol 'Symbol' (Location)
	last_val = bbb_gpio_get(GPIO_BANK_3, GPIO_NUM_17);

    /* Signalize that this thread is running */
    cyc_gpio_running = PNPB_THREAD_RUNNING;

    /* Run till the termination flag occurs */
    while (pnpb_keep_running == PNPB_THREAD_RUNNING)
    {
        /* Wait for event */
        do
        {
            val = bbb_gpio_get(GPIO_BANK_3, GPIO_NUM_17);

            if(last_val != val)
            {
                break;
            }
            else
            {
                /* Sleep for predefined delay */
                nanosleep(&delay, &rem);
            }
        } while (pnpb_keep_running == PNPB_THREAD_RUNNING);

        /* Check whether the loop was terminated by
         * signal or GPIO change and take action */
        if(pnpb_keep_running == PNPB_THREAD_RUNNING)
        {
            last_val = val;

            sem_post(&PnioCycSem);
            sem_wait(&PnioCycSemBack);
        }
    }

    cyc_gpio_running = PNPB_THREAD_TERMINATE;
    /* Post semaphore to allow pnpb_cyclical_capture_thread
     * to terminate */
    sem_post(&PnioCycSem);

    return NULL;
}

/**
 *  @brief Main function of thread for handling cyclical communication via GPMC - XHIF memory interface
 *
 *  @param[in]      *i                  Dummy param
 *  @return         void
 *
 *  Functions hang - waits for change on HW pin. For this is used blocking function poll.
 *
 */
PNIO_VOID * pnpb_cyclical_execute_thread(PNIO_VOID * i)
{
    PNIO_UINT32 rv;
    pthread_t gpio_sem_handler;
    pthread_t this_thread = pthread_self();
    struct sched_param th_param;

    /* Configure this thread to have high priority */
    th_param.sched_priority = sched_get_priority_max (SCHED_RR);
    if (pthread_setschedparam(this_thread, SCHED_RR, &th_param) != 0)
    {
        printf("Thread priority (pnpb_cyclical_execute_thread - high prio) configuration failed!");
        PNPB_LIB_FATAL
    }

    /* Init semaphore before use */
    sem_init(&PnioCycSem, 0, 0);
    sem_init(&PnioCycSemBack, 0, 0);

    /* Signalize that this thread is running */
    cyc_service_running = PNPB_THREAD_RUNNING;

    /* Start thread for acyclic capture */
    pthread_create(&gpio_sem_handler, NULL, poll_cyc_sem_gpio, NULL);

    while (pnpb_keep_running == PNPB_THREAD_RUNNING)
    {
        sem_wait(&PnioCycSem);

        /* Check whether the termination flag occurs
         * during semaphore wait */
        if(pnpb_keep_running == PNPB_THREAD_RUNNING)
        {
            PnUsr_cbf_IoDatXch();
            sem_post(&PnioCycSemBack);
        }
    }

    /* Destroy semaphores */
    sem_destroy(&PnioCycSem);
    sem_destroy(&PnioCycSemBack);

    /* Signalize that this thread was terminated */
    cyc_service_running = PNPB_THREAD_TERMINATE;

    return NULL;
}   /* pnpb_cyclical_execute_thread */

/**
 *  @brief Reset Ertec Devkit
 *
 *  @param[out]     *gpio_ac_in_fd       Returns new pointer to file
 *
 *  @return         void
 *
 *  !!! Not to be used when GPMC - XHIF is active - BBB will freeze
 *  Toggles Ertec reset signal - Reset len 5 ms
 *
 */
PNIO_VOID pnpb_reset_Ertec()
{
    PNIO_UINT32 rv;
    PNIO_INT32 gpio_fd, len;
    PNIO_UINT8 tmp[4];
    struct timespec resetlen, rem;
    resetlen.tv_sec = 0;

    /* GPIO66 - force Ertec reset */
    rv = bbb_gpio_open(GPIO_BANK_2, GPIO_NUM_2, bbb_gpio_output);
    if(PNPB_OK != rv)
    {
        printf("Cannot reset ERTEC (GPIO library error)!\n");
        return;
    }

    /* Set default value */
    bbb_gpio_set(GPIO_BANK_2, GPIO_NUM_2, 0);
    /* Wait a little */
    resetlen.tv_nsec = 5000000;
    nanosleep(&resetlen, &rem);
    /* Toggle pin to reset ERTEC */
    bbb_gpio_set(GPIO_BANK_2, GPIO_NUM_2, 1);
    /* Wait a little */
    resetlen.tv_nsec = 5000000;
    nanosleep(&resetlen, &rem);
    bbb_gpio_set(GPIO_BANK_2, GPIO_NUM_2, 0);

     printf("Ertec reset done\n");
    /* Wait 5ms after reset - problem will ocur also if GPMC activated too soon - before primary boot of Ertec */
    resetlen.tv_nsec = 5000000;
    nanosleep(&resetlen, &rem);
}   /* pnpb_reset_Ertec */


/**
 *  @brief Putting values to registers of GPMC
 *
 *  @param[in]      *p_gpmc     Pointer to GPMC registers mounted by mmap
 *  @return         void
 *
 *  Takes settings prepared in header file and puts them into registers
 *  for two GPMC banks
 *  Example of switching off GPMC bank under #if 0
 *
 */
PNIO_VOID pnpb_set_gpmc(PNIO_VOID* p_gpmc)
{
	PNIO_UINT32 tmp_reg;
#if(1 == PNPB_LIB_SET_VERBOSE)
	printf("\n");
	printf("Setting registers of GPMC:\n");
#endif	/* PNPB_SET_GPMC_VERBOSE */


#if 0
	/* In case of any previously used GPMC bank have to be switched off */
	/* Example on bank 6 */
	tmp_reg  = 0x00;
	tmp_reg |= (0x00 <<  8); /* MASKADDRESS */
	tmp_reg |= (0x00 <<  6); /* CSVALID */
	tmp_reg |= (0x00 <<  0); /* BASEADDRESS */

	WRITE32((p_gpmc + PNPB_GPMC_CONFIG_REG7_6), tmp_reg);
#if(1 == PNPB_SET_GPMC_VERBOSE)
	printf("Switching off bank %d: 0x%08x written to 0x%08x : 0x%08x\n",
			PNPB_GPMC_CONFIG_BANK_6, tmp_reg, p_gpmc + PNPB_GPMC_CONFIG_REG7_6, READ32(p_gpmc + PNPB_GPMC_CONFIG_REG7_6));
#endif	/* PNPB_SET_GPMC_VERBOSE */
#endif	/* 0 */

	/* Prepare xhif config general reg */
	tmp_reg = 0x00;
	tmp_reg |= (PNPB_GPMC_REG_VAL_WAIT1PINPOLARITY          <<  9);
    tmp_reg |= (PNPB_GPMC_REG_VAL_WAIT0PINPOLARITY          <<  8);
    tmp_reg |= (PNPB_GPMC_REG_VAL_WRITEPROTECT              <<  4);
    tmp_reg |= (PNPB_GPMC_REG_VAL_LIMITEDADDRESS            <<  1);
    tmp_reg |= (PNPB_GPMC_REG_VAL_NANDFORCEPOSTEDWRITE      <<  0);
    /* And write it to register; General -> Applies for all banks */
    WRITE32((p_gpmc + PNPB_GPMC_CONFIG_REG), tmp_reg);


	/* Prepare xhif config reg 1 */
	tmp_reg  = 0x00;
	tmp_reg |= (PNPB_GPMC_REG_VAL_WRAPBURST                 << 31);
	tmp_reg |= (PNPB_GPMC_REG_VAL_READMULTIPLE              << 30);
	tmp_reg |= (PNPB_GPMC_REG_VAL_READTYPE                  << 29);
	tmp_reg |= (PNPB_GPMC_REG_VAL_WRITEMULTIPLE             << 28);
	tmp_reg |= (PNPB_GPMC_REG_VAL_WRITETYPE                 << 27);
	tmp_reg |= (PNPB_GPMC_REG_VAL_CLKACTIVATIONTIME         << 25);
	tmp_reg |= (PNPB_GPMC_REG_VAL_ATTACHEDDEVICEPAGELENGTH  << 23);
	tmp_reg |= (PNPB_GPMC_REG_VAL_WAITREADMONITORING        << 22);
	tmp_reg |= (PNPB_GPMC_REG_VAL_WAITWRITEMONITORING       << 21);
	tmp_reg |= (PNPB_GPMC_REG_VAL_WAITONMONITORINGTIME      << 18);
	tmp_reg |= (PNPB_GPMC_REG_VAL_WAITPINSELECT             << 16);
	tmp_reg |= (PNPB_GPMC_REG_VAL_DEVICESIZE                << 12);
	tmp_reg |= (PNPB_GPMC_REG_VAL_DEVICETYPE                << 10);
	tmp_reg |= (PNPB_GPMC_REG_VAL_MUXADDATA                 <<  8);
	tmp_reg |= (PNPB_GPMC_REG_VAL_TIMEPARAGRANULARITY       <<  4);
	tmp_reg |= (PNPB_GPMC_REG_VAL_GPMCFCLKDIVIDER           <<  0);

	/* We are using two banks, same settings for both */
	WRITE32((p_gpmc + PNPB_GPMC_CONFIG_REG1), tmp_reg);
	WRITE32((p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG1), tmp_reg);

#if(1 == PNPB_LIB_SET_VERBOSE)
	printf("Config reg 1 of bank %d: 0x%08x written to 0x%08x : 0x%08x\n",
			PNPB_GPMC_CONFIG_BANK, tmp_reg, p_gpmc + PNPB_GPMC_CONFIG_REG1, READ32(p_gpmc + PNPB_GPMC_CONFIG_REG1));
	printf("Config reg 1 of bank %d: 0x%08x written to 0x%08x : 0x%08x\n",
			PNPB_GPMC_PAGE_CONFIG_BANK, tmp_reg, p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG1, READ32(p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG1));
#endif	/* PNPB_SET_GPMC_VERBOSE */

	/* Prepare xhif config reg 2 */
	tmp_reg  = 0x00;
	tmp_reg |= (PNPB_GPMC_REG_VAL_CSWROFFTIME   << 16);
	tmp_reg |= (PNPB_GPMC_REG_VAL_CSRDOFFTIME   <<  8);
	tmp_reg |= (PNPB_GPMC_REG_VAL_CSEXTRADELAY  <<  7);
	tmp_reg |= (PNPB_GPMC_REG_VAL_CSONTIME      <<  0);

	/* We are using two banks, same settings for both */
	WRITE32((p_gpmc + PNPB_GPMC_CONFIG_REG2), tmp_reg);
	WRITE32((p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG2), tmp_reg);

#if(1 == PNPB_LIB_SET_VERBOSE)
	printf("Config reg 2 of bank %d: 0x%08x written to 0x%08x : 0x%08x\n",
			PNPB_GPMC_CONFIG_BANK, tmp_reg, p_gpmc + PNPB_GPMC_CONFIG_REG2, READ32(p_gpmc + PNPB_GPMC_CONFIG_REG2));
	printf("Config reg 2 of bank %d: 0x%08x written to 0x%08x : 0x%08x\n",
			PNPB_GPMC_PAGE_CONFIG_BANK, tmp_reg, p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG2, READ32(p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG2));
#endif	/* PNPB_SET_GPMC_VERBOSE */

	/* Prepare xhif config reg 3 */
	tmp_reg = 0x00;
	tmp_reg |= (PNPB_GPMC_REG_VAL_ADVAADMUXWROFFTIME    << 28);
	tmp_reg |= (PNPB_GPMC_REG_VAL_ADVAADMUXRDOFFTIME    << 24);
	tmp_reg |= (PNPB_GPMC_REG_VAL_ADVWROFFTIME          << 16);
	tmp_reg |= (PNPB_GPMC_REG_VAL_ADVRDOFFTIME          <<  8);
	tmp_reg |= (PNPB_GPMC_REG_VAL_ADVEXTRADELAY         <<  7);
	tmp_reg |= (PNPB_GPMC_REG_VAL_ADVAADMUXONTIME       <<  4);
	tmp_reg |= (PNPB_GPMC_REG_VAL_ADVONTIME             <<  0);

	/* We are using two banks, same settings for both */
	WRITE32((p_gpmc + PNPB_GPMC_CONFIG_REG3), tmp_reg);
	WRITE32((p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG3), tmp_reg);

#if(1 == PNPB_LIB_SET_VERBOSE)
	printf("Config reg 3 of bank %d: 0x%08x written to 0x%08x : 0x%08x\n",
			PNPB_GPMC_CONFIG_BANK, tmp_reg, p_gpmc + PNPB_GPMC_CONFIG_REG3, READ32(p_gpmc + PNPB_GPMC_CONFIG_REG3));
	printf("Config reg 3 of bank %d: 0x%08x written to 0x%08x : 0x%08x\n",
			PNPB_GPMC_PAGE_CONFIG_BANK, tmp_reg, p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG3, READ32(p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG3));
#endif	/* PNPB_SET_GPMC_VERBOSE */

	/* Prepare xhif config reg 4 */
	tmp_reg = 0x00;
	tmp_reg |= (PNPB_GPMC_REG_VAL_WEOFFTIME         << 24);
	tmp_reg |= (PNPB_GPMC_REG_VAL_WEEXTRADELAY      << 23);
	tmp_reg |= (PNPB_GPMC_REG_VAL_WEONTIME          << 16);
	tmp_reg |= (PNPB_GPMC_REG_VAL_OEAADMUXOFFTIME   << 13);
	tmp_reg |= (PNPB_GPMC_REG_VAL_OEOFFTIME         <<  8);
	tmp_reg |= (PNPB_GPMC_REG_VAL_OEEXTRADELAY      <<  7);
	tmp_reg |= (PNPB_GPMC_REG_VAL_OEAADMUXONTIME    <<  4);
	tmp_reg |= (PNPB_GPMC_REG_VAL_OEONTIME          <<  0);

	/* We are using two banks, same settings for both */
	WRITE32((p_gpmc + PNPB_GPMC_CONFIG_REG4), tmp_reg);
	WRITE32((p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG4), tmp_reg);

#if(1 == PNPB_LIB_SET_VERBOSE)
	printf("Config reg 4 of bank %d: 0x%08x written to 0x%08x : 0x%08x\n",
			PNPB_GPMC_CONFIG_BANK, tmp_reg, p_gpmc + PNPB_GPMC_CONFIG_REG4, READ32(p_gpmc + PNPB_GPMC_CONFIG_REG4));
	printf("Config reg 4 of bank %d: 0x%08x written to 0x%08x : 0x%08x\n",
			PNPB_GPMC_PAGE_CONFIG_BANK, tmp_reg, p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG4, READ32(p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG4));
#endif	/* PNPB_SET_GPMC_VERBOSE */

	/* Prepare xhif config reg 5 */
	tmp_reg  = 0x00;
	tmp_reg |= (PNPB_GPMC_REG_VAL_RDACCESSTIME  << 16);
	tmp_reg |= (PNPB_GPMC_REG_VAL_WRCYCLETIME   <<  8);
	tmp_reg |= (PNPB_GPMC_REG_VAL_RDCYCLETIME   <<  0);

	/* We are using two banks, same settings for both */
	WRITE32((p_gpmc + PNPB_GPMC_CONFIG_REG5), tmp_reg);
	WRITE32((p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG5), tmp_reg);

#if(1 == PNPB_LIB_SET_VERBOSE)
	printf("Config reg 5 of bank %d: 0x%08x written to 0x%08x : 0x%08x\n",
			PNPB_GPMC_CONFIG_BANK, tmp_reg, p_gpmc + PNPB_GPMC_CONFIG_REG5, READ32(p_gpmc + PNPB_GPMC_CONFIG_REG5));
	printf("Config reg 5 of bank %d: 0x%08x written to 0x%08x : 0x%08x\n",
			PNPB_GPMC_PAGE_CONFIG_BANK, tmp_reg, p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG5, READ32(p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG5));
#endif	/* PNPB_SET_GPMC_VERBOSE */

	/* Prepare xhif config reg 6 */
	tmp_reg  = 0x00;
	tmp_reg |= (PNPB_GPMC_REG_VAL_WRACCESSTIME          << 24);
	tmp_reg |= (PNPB_GPMC_REG_VAL_WRDATAONADMUXBUS      << 16);
	tmp_reg |= (PNPB_GPMC_REG_VAL_CYCLE2CYCLEDELAY      <<  8);
	tmp_reg |= (PNPB_GPMC_REG_VAL_CYCLETOCYCLESAMECSEN  <<  7);
	tmp_reg |= (PNPB_GPMC_REG_VAL_CYCLE2CYCLEDIFFCSEN   <<  6);
	tmp_reg |= (PNPB_GPMC_REG_VAL_BUSTURNAROUND         <<  0);

	/* We are using two banks, same settings for both */
	WRITE32((p_gpmc + PNPB_GPMC_CONFIG_REG6), tmp_reg);
	WRITE32((p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG6), tmp_reg);

#if(1 == PNPB_LIB_SET_VERBOSE)
	printf("Config reg 6 of bank %d: 0x%08x written to 0x%08x : 0x%08x\n",
			PNPB_GPMC_CONFIG_BANK, tmp_reg, p_gpmc + PNPB_GPMC_CONFIG_REG6, READ32(p_gpmc + PNPB_GPMC_CONFIG_REG6));
	printf("Config reg 6 of bank %d: 0x%08x written to 0x%08x : 0x%08x\n",
			PNPB_GPMC_PAGE_CONFIG_BANK, tmp_reg, p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG6, READ32(p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG6));
#endif	/* PNPB_SET_GPMC_VERBOSE */

	/* Prepare xhif config reg 7 */
	tmp_reg = 0x00;
	tmp_reg |= (PNPB_GPMC_REG_VAL_BANK6_MASKADDRESS <<  8);
	tmp_reg |= (PNPB_GPMC_REG_VAL_BANK6_CSVALID     <<  6);
	tmp_reg |= (PNPB_GPMC_REG_VAL_BANK6_BASEADDRESS <<  0);

	WRITE32((p_gpmc + PNPB_GPMC_CONFIG_REG7), tmp_reg);
#if(1 == PNPB_LIB_SET_VERBOSE)
	printf("Config reg 7 of bank %d: 0x%08x written to 0x%08x : 0x%08x\n",
			PNPB_GPMC_CONFIG_BANK, tmp_reg, p_gpmc + PNPB_GPMC_CONFIG_REG7, READ32(p_gpmc + PNPB_GPMC_CONFIG_REG7));
#endif	/* PNPB_SET_GPMC_VERBOSE */
	/* This time, there is different settings for bank 0 */
	tmp_reg = 0x00;
	tmp_reg |= (PNPB_GPMC_REG_VAL_BANK0_MASKADDRESS <<  8);
	tmp_reg |= (PNPB_GPMC_REG_VAL_BANK0_CSVALID     <<  6);
	tmp_reg |= (PNPB_GPMC_REG_VAL_BANK0_BASEADDRESS <<  0);

	WRITE32( ( p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG7 ), tmp_reg );
#if(1 == PNPB_LIB_SET_VERBOSE)
	printf("Config reg 6 of bank %d: 0x%08x written to 0x%08x : 0x%08x\n",
			PNPB_GPMC_PAGE_CONFIG_BANK, tmp_reg, p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG7, READ32(p_gpmc + PNPB_GPMC_PAGE_CONFIG_REG7));
	printf("\n");
#endif	/* PNPB_SET_GPMC_VERBOSE */
}	/* pnpb_set_gpmc */


/**
 *  @brief Set offset address and size of XHIF page
 *
 *  @param[in]      *p_xhif_set_page    Pointer to GPMC bank for page switch, mounted by mmap
 *  @param[in]      page                Number of page to set [0 - 7]
 *  @param[in]      addr                Address offset of page to set
 *  @return                             Result of parameters validity check[PNPB_TRUE, PNPB_FALSE]
 *
 *  Checks p_xhif_set_page validity, if addr is from address range reachable via XHIF and
 *  if number of page is from available 8 pages
 *  Addresses are set according to the page number
 *  Page size is always maximal, size is defined in header file
 *
 */
PNIO_UINT32 pnpb_set_page(PNIO_VOID* p_xhif_set_page, PNIO_UINT32 page, PNIO_UINT32 addr)
{
	/* Check for valid pointer to /dev/mem */
	if(PNPB_NULL == p_xhif_set_page)
	{
		printf("Error: invalid pointer to /dev/mem\n");
		return PNPB_NOT_OK;
	}
	/* Check for address validity - mapable memory areas */
	if(!(   ((0x00000000 <= addr) && ((0x04040000 - PNPB_XHIF_PAGE_SIZE) >= addr)) ||
	        ((0x08000000 <= addr) && ((0x08040000 - PNPB_XHIF_PAGE_SIZE) >= addr)) ||
	        ((0x10600000 <= addr) && ((0x10C00000 - PNPB_XHIF_PAGE_SIZE) >= addr)) ||
	        ((0x10D00000 <= addr) && ((0x10E00000 - PNPB_XHIF_PAGE_SIZE) >= addr)) ||
	        ((0x11000000 <= addr) && ((0x11100000 - PNPB_XHIF_PAGE_SIZE) >= addr)) ||
	        ((0x20000000 <= addr) && ((0x40600000 - PNPB_XHIF_PAGE_SIZE) >= addr))))
	{
		printf("Error: trying to set invalid address 0x%08x\n", addr);
		return PNPB_NOT_OK;
	}

	/* Page -> ertec register for XHIF settings address */
	switch(page)
	{
		case 0:
		{
			WRITE32(p_xhif_set_page + 0x04, addr);
			WRITE32(p_xhif_set_page + 0x00, PNPB_XHIF_PAGE_SIZE);
			break;
		}
		case 1:
		{
			WRITE32(p_xhif_set_page + 0x44, addr);
			WRITE32(p_xhif_set_page + 0x40, PNPB_XHIF_PAGE_SIZE);
			break;
		}
		case 2:
		{
			WRITE32(p_xhif_set_page + 0x24, addr);
			WRITE32(p_xhif_set_page + 0x20, PNPB_XHIF_PAGE_SIZE);
			break;
		}
		case 3:
		{
			WRITE32(p_xhif_set_page + 0x64, addr);
			WRITE32(p_xhif_set_page + 0x60, PNPB_XHIF_PAGE_SIZE);
			break;
		}
		case 4:
		{
			WRITE32(p_xhif_set_page + 0x14, addr);
			WRITE32(p_xhif_set_page + 0x10, PNPB_XHIF_PAGE_SIZE);
			break;
		}
		case 5:
		{
			WRITE32(p_xhif_set_page + 0x54, addr);
			WRITE32(p_xhif_set_page + 0x50, PNPB_XHIF_PAGE_SIZE);
			break;
		}
		case 6:
		{
			WRITE32(p_xhif_set_page + 0x34, addr);
			WRITE32(p_xhif_set_page + 0x30, PNPB_XHIF_PAGE_SIZE);
			break;
		}
		case 7:
		{
			WRITE32(p_xhif_set_page + 0x74, addr);
			WRITE32(p_xhif_set_page + 0x70, PNPB_XHIF_PAGE_SIZE);
			break;
		}
		default:
		{
			printf("Error: trying to set invalid page\n");
			return PNPB_NOT_OK;
		}
	}
	return PNPB_OK;
}	/* pnpb_set_page */


/**
 *  @brief Set offset address and size of group of XHIF pages
 *
 *  @param[in]      *p_xhif_set_page    Pointer to GPMC bank for page switch, mounted by mmap
 *  @param[in]      page_strt           Number of first page [0 - 7]
 *  @param[in]      pages               Number of pages to set
 *  @param[in]      strt_addr           Address offset of first page to set
 *  @return                             Result of parameters validity check[PNPB_TRUE, PNPB_FALSE]
 *
 *  Checks p_xhif_set_page validity, if addr is from address range reachable via XHIF and
 *  if number of page is from available 8 pages
 *  Addresses are set according to the page number
 *  Page size is always maximal, size is defined in header file
 *
 */
PNIO_UINT32 pnpb_set_multiple_pages(PNIO_VOID* p_xhif_set_page, PNIO_UINT32 page_strt, PNIO_UINT32 pages, PNIO_UINT32 strt_addr)
{
	PNIO_UINT32 i, status, address;

	/* Input values validity check */
	if(((page_strt + pages - 1) > 7) || (pages < 1))
	{
		printf("Error: invalid number of pages\n");
		return PNPB_NOT_OK;
	}

	/* Set all pages iteratively */
	for(i = 0; i < pages; i++)
	{
		address = strt_addr + (i * PNPB_XHIF_PAGE_SIZE);
		status = pnpb_set_page(p_xhif_set_page, i + page_strt, address);
		if(PNPB_NOT_OK == status)
		{
			return PNPB_NOT_OK;
		}
	}
	return PNPB_OK;
}	/* pnpb_set_multiple_pages */


/**
 *  @brief Printout whole block of registers for XHIF page settings
 *
 *  @param[in]      *p_xhif_set_page    pointer to GPMC bank for page switch, mounted by mmap
 *  @return         void
 *
 *  Printout of set offset and size for all eight pages of XHIF
 *
 */
PNIO_VOID pnpb_print_xhif_page_reg(PNIO_VOID* p_xhif_set_page)
{
	PNIO_UINT32 i, tmp;
	printf("printing xhif registers on 0x%08x", p_xhif_set_page);
	for(i = 0; i < 0x20; i++)
    {
        /* 4 register values on line, address of first one on the line */
        if(0 == i % 4)
        {
            printf("\n 0x%08x: ", 4 * i);
        }

        tmp = READ32(p_xhif_set_page + (4 * i));
        printf("%08x  ", tmp);
    }
    printf("\n");
}	/* pnpb_print_xhif_page_reg */


/**
 *  @brief Write to one ertec register
 *
 *  @param[in]      *p_xhif_data        Pointer to GPMC bank for xhif data exchange, mounted by mmap
 *  @param[in]      ertec_addr          Address of register in Ertec memory space
 *  @param[in]      value               Value to be written to register
 *  @return         void
 *
 *  Address of register can be either complete address in Ertec APB registers memory space or
 *  only offset from start of APB addresses.
 *
 */
PNIO_VOID pnpb_write_ertec_register(PNIO_VOID* p_xhif_data, PNIO_UINT32 ertec_addr, PNIO_UINT32 value)
{
    /* Mask out - only offset against apb register bank start*/
    ertec_addr &= 0x0001FFFF;
    /* Write via GPMC - XHIF */
    WRITE32((p_xhif_data + PNPB_XHIF_OFFSET_PAGE_7 + ertec_addr), value);
}   /* pnpb_write_ertec_register */


/**
 *  @brief Write to one ertec register
 *
 *  @param[in]      *p_xhif_data        Pointer to GPMC bank for xhif data exchange, mounted by mmap
 *  @param[in]      ertec_addr          Address of register in Ertec memory space
 *  @param[out]     *p_value            Value read from register will be placed to this address
 *  @return         void
 *
 *  Address of register can be either complete address in Ertec APB registers memory space or
 *  only offset from start of APB addresses.
 *
 */
PNIO_VOID pnpb_read_ertec_register(PNIO_VOID* p_xhif_data, PNIO_UINT32 ertec_addr, PNIO_UINT32 *p_value)
{
    /* Mask out - only offset against apb register bank start*/
    ertec_addr &= 0x0001FFFF;
    /* Write via GPMC - XHIF */
    *p_value = READ32(p_xhif_data + PNPB_XHIF_OFFSET_PAGE_7 + ertec_addr);
    printf("Read %x from %x - %x\n", *p_value, ertec_addr, p_xhif_data + PNPB_XHIF_OFFSET_PAGE_7 + ertec_addr);
}   /* pnpb_read_ertec_register */


/**
 *  @brief Write to one ertec register, only bits allowed by mask
 *
 *  @param[in]      *p_xhif_data        Pointer to GPMC bank for xhif data exchange, mounted by mmap
 *  @param[in]      ertec_addr          Address of register in Ertec memory space
 *  @param[in]      value               Value to be written to register *
 *  @param[in]      mask                Set bits to be written, others will be left as before
 *  @return         void
 *
 *  Address of register can be either complete address in Ertec APB registers memory space or
 *  only offset from start of APB addresses.
 *
 */
PNIO_VOID pnpb_write_ertec_register_masked(PNIO_VOID* p_xhif_data, PNIO_UINT32 ertec_addr, PNIO_UINT32 value, PNIO_UINT32 mask)
{
    PNIO_UINT32 tmp_value;
    /* Read original value */
    pnpb_read_ertec_register(p_xhif_data, ertec_addr, &tmp_value);
    /* Mask */
    tmp_value |= (mask & value);
    tmp_value &= ((~mask) | value);
    /* Write */
    pnpb_write_ertec_register(p_xhif_data, ertec_addr, tmp_value);
}   /* pnpb_write_ertec_register_masked */


/**
 *  @brief Ertec registers settings necessary before end of primary bootloader
 *
 *  @param[in]      *p_xhif_set_page    Pointer to GPMC bank for page switch, mounted by mmap
 *  @param[in]      *p_xhif_data        Pointer to GPMC bank for xhif data exchange, mounted by mmap
 *  @return         void
 *
 *  For settings in of both register areas - EMC settings regs and APB peripheries regs
 *
 */
PNIO_VOID pnpb_set_ertec_registers(PNIO_VOID* p_xhif_set_page, PNIO_VOID* p_xhif_data)
{
	/* Switch XHIF page 7 to register bank for EMC */
	pnpb_set_multiple_pages(p_xhif_set_page, 7, 1, PNPB_ERTEC_EMC_REGISTERS_OFFSET);
	/* Switch EMC settings - access to registers was set to page 7 -> offsett address */
	pnpb_write_ertec_register(p_xhif_data, PNPB_ERTEC_REG_SDRAM_CONFIG, PNPB_ERTEC_SDRAM_CONFIG_VAL);
	WRITE32((p_xhif_data + PNPB_XHIF_OFFSET_PAGE_7 + PNPB_ERTEC_REG_SDRAM_CONFIG), PNPB_ERTEC_SDRAM_CONFIG_VAL);
	/* Switch XHIF page 7 to register bank back for APB peripheries */
	pnpb_set_multiple_pages(p_xhif_set_page, 7, 1, PNPB_ERTEC_APB_REGISTERS_OFFSET);
}	/* pnpb_set_ertec_registers */


/**
 *  @brief Gives size of firmware data to be loaded to Ertec SDRAM and ITCM
 *
 *  @param[in]      *source             Pointer to file with firmware for Ertec
 *  @param[out]     *p_sdram_size       Size of SDRAM section
 *  @param[out]     *p_itcm_size        Size of ITCM section
 *  @return                             Check of validity of sizes[PNPB_TRUE, PNPB_FALSE]
 *
 *  Obtains size of binary, reads size of ITCM section, counts size of SDRAM section
 *
 */
PNIO_UINT32 pnpb_obtain_binary_size(FILE* source, PNIO_UINT32* p_sdram_size, PNIO_UINT32* p_itcm_size)
{
	PNIO_UINT32 binary_size, sdram_size, itcm_size;

	itcm_size = 0;	/* read fills only 2B -> clean before use is a must! */
	/* Obtain size of file */
    if(NULL != source)
    {
		fseek(source, 0, SEEK_END);
		binary_size = ftell(source);
    }
    else
    {
#if (1 == PNPB_LIB_SET_VERBOSE)
        printf("Error: File does not exist!\n\n");
#endif
        return PNPB_NOT_OK;
    }

	/* 2B at position (size of file - 2) = size of DTCM section */
	fseek(source, (binary_size - 2), SEEK_SET);
	fread( &itcm_size, 2, 1, source );
	/* start of code is offseted by 0x44 - compensate in first block size */
	itcm_size -= 0x44;
	sdram_size = binary_size - itcm_size;
	/* Check ITCM section size - max 0x40000 */
	if(itcm_size > 0x10000)
	{
		printf("Error: Wrong ITCM section size in binary file!\n\n");
		PNPB_LIB_FATAL
	}
	/* Check align of all blocks (4B align) */
	if((0 != (itcm_size % 4)) || (0 != (sdram_size % 4)))
	{
		printf("Error: One of the blocks not aligned to 4B!\n"
				"ITCM 0x%xB SDRAM 0x%xB\n\n",
				itcm_size, sdram_size);
		PNPB_LIB_FATAL
	}
	*p_sdram_size = sdram_size;
	*p_itcm_size = itcm_size;
	return PNPB_OK;
}	/* pnpb_obtain_binary_size */


/**
 *  @brief Copies one page of firmware from binary to Ertec SDRAM
 *
 *  @param[in]      *p_xhif_set_page    Pointer to GPMC bank for page switch, mounted by mmap
 *  @param[in]      *p_xhif_data        Pointer to GPMC bank for xhif data exchange, mounted by mmap
 *  @param[in]      *source             Pointer to file with firmware for Ertec
 *  @param[in]      page                Number of firmware page to be processed
 *  @param[in]      page_size           Data amount to be processed
 *  @param[in]      page_start          Initial offset on the given page
 *  @return                             Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Iteratively copies part of firmware to Ertec SDRAM
 *
 */
PNIO_UINT32 pnpb_copy_one_page_of_firmware( PNIO_VOID* p_xhif_set_page,
                                            PNIO_VOID* p_xhif_data,
                                            FILE* source,
                                            PNIO_UINT32 page,
                                            PNIO_UINT32 page_size,
                                            PNIO_UINT32 page_start)
{
	PNIO_UINT32 data_word;
	/* Set page */
	if(PNPB_NOT_OK ==
			pnpb_set_multiple_pages(p_xhif_set_page, 0, 7, (0x20000000 + (page * PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE))))
	{
		return PNPB_NOT_OK;
	}

#if (1 == PNPB_LIB_SET_VERBOSE)
	printf("Page %d: Copy from 0x%x to 0x%x \n", page,
			0x20000000 + (page * PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE) + page_start,
			0x20000000 + (page * PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE) + page_size);
#endif
    if(NULL != source)
    {
		/* Copy word by word */
		for(; page_start < page_size; page_start += 4)
		{
			fread(&data_word, 4, 1, source);
			WRITE32((p_xhif_data + page_start), data_word);
		}
#if (1 == PNPB_LIB_SET_VERBOSE)
		printf("    0x%x bytes successfully copied to 0x%x\n",
		page_start, 0x20000000 + (page * PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE));
#endif
    }
    else
    {
#if (1 == PNPB_LIB_SET_VERBOSE)
        printf("Error: File does not exist!\n\n");
#endif
        return PNPB_NOT_OK;
    }
	return PNPB_OK;
}	/* pnpb_copy_one_page_of_firmware */


/**
 *  @brief Copies firmware from binary to Ertec ITCM
 *
 *  @param[in]   	*p_xhif_set_page 	Pointer to GPMC bank for page switch, mounted by mmap
 *  @param[in]   	*p_xhif_data	 	Pointer to GPMC bank for xhif data exchange, mounted by mmap
 *  @param[in]   	*source	 			Pointer to file with firmware for Ertec
 *  @param[in]   	page_size	 		Data amount to be processed
 *  @return      						Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Iteratively copies part of firmware to Ertec ITCM
 *
 */
PNIO_UINT32 pnpb_copy_firmware_to_itcm( PNIO_VOID* p_xhif_set_page,
                                        PNIO_VOID* p_xhif_data,
                                        FILE* source,
                                        PNIO_UINT32 page_size)
{
	PNIO_UINT32 data_word;
	PNIO_UINT32 i = 0;

	/* Jump in binary to start of file - ITCM section */
	if(NULL != source)
	{
		fseek(source, 0, SEEK_SET);
	}
	else
	{
#if (1 == PNPB_LIB_SET_VERBOSE)
		printf("Error: File does not exist!\n\n");
#endif
		return PNPB_NOT_OK;
	}

	/* Set page */
	if(PNPB_NOT_OK == pnpb_set_multiple_pages(p_xhif_set_page, 0, 2, 0x08000000))
	{
		return PNPB_NOT_OK;
	}

	/* Copy word by word */
#if (1 == PNPB_LIB_SET_VERBOSE)
	printf("ITCM: Copy from 0x%x to 0x%x \n", 0x08030044 + i, 0x08030044 + page_size);
#endif
	for(; i < page_size; i += 4)
	{
		fread(&data_word, 4, 1, source);
		WRITE32((p_xhif_data + 0x00030044 + i), data_word);
	}
#if (1 == PNPB_LIB_SET_VERBOSE)
	printf("    0x%x bytes successfully copied to 0x%x\n", i, 0x08030044);
#endif
	return PNPB_OK;
}	/* pnpb_copy_firmware_to_itcm */


/**
 *  @brief Checks one page of firmware from binary against data in Ertec SDRAM
 *
 *  @param[in]      *p_xhif_set_page    Pointer to GPMC bank for page switch, mounted by mmap
 *  @param[in]      *p_xhif_data        Pointer to GPMC bank for xhif data exchange, mounted by mmap
 *  @param[in]      *source             Pointer to file with firmware for Ertec
 *  @param[in]      page                Number of firmware page to be processed
 *  @param[in]      page_size           Data amount to be processed
 *  @param[in]      page_start          Initial offset on the given page
 *  @return                             Number of diferences
 *
 *  Iterative check. Have to be run in primary bootloader of Ertec, otherwise, the data might
 *  be changed by Ertec runtime and check might fail.
 *
 */
PNIO_UINT32 pnpb_check_one_page_of_firmware(PNIO_VOID* p_xhif_set_page,
                                            PNIO_VOID* p_xhif_data,
                                            FILE* source,
                                            PNIO_UINT32 page,
                                            PNIO_UINT32 page_size,
                                            PNIO_UINT32 page_start)
{
	PNIO_UINT32 data_word, check_word;
	PNIO_UINT32 check = 0;	/* Number of differences */
	/* Set page */
	if(PNPB_NOT_OK ==
			pnpb_set_multiple_pages(p_xhif_set_page, 0, 7, (0x20000000 + (page * PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE))))
	{
		return PNPB_NOT_OK;
	}

#if (1 == PNPB_LIB_SET_VERBOSE)
	printf("Page %d: Check from 0x%x to 0x%x \n", page,
			0x20000000 + (page * PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE) + page_start,
			0x20000000 + (page * PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE) + page_size);
#endif
	/* Check word by word */
	for(; page_start < page_size; page_start += 4)
	{
		fread(&data_word, 4, 1, source);
		check_word = READ32(p_xhif_data + page_start);
		/* Compare data from binary against data read from Ertec */
		if(data_word != check_word)
		{
			check ++;
#if (1 == PNPB_LIB_SET_VERBOSE)
			printf("Error at %x: %08x but expected %08x\n",
					0x20000000 + (page * PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE) + page_start,
					check_word, data_word);
#endif
		}
	}
#if (1 == PNPB_LIB_SET_VERBOSE)
	printf("    0x%x bytes successfully checked at 0x%x\n",
			page_start, 0x20000000 + (page * PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE));
#endif
	return check;
}	/* pnpb_check_one_page_of_firmware */


/**
 *  @brief Checks one page of firmware from binary against data in Ertec ITCM
 *
 *  @param[in]      *p_xhif_set_page    Pointer to GPMC bank for page switch, mounted by mmap
 *  @param[in]      *p_xhif_data        Pointer to GPMC bank for xhif data exchange, mounted by mmap
 *  @param[in]      *source             Pointer to file with firmware for Ertec
 *  @param[in]      page_size           Data amount to be processed
 *  @return                             Number of diferences
 *
 *  Iterative check. Have to be run in primary bootloader of Ertec, otherwise, the ITCM is not accessible,
 *  0x00 is read and check will fail.
 *
 */
PNIO_UINT32 pnpb_check_firmware_in_itcm(PNIO_VOID* p_xhif_set_page,
                                        PNIO_VOID* p_xhif_data,
                                        FILE* source,
                                        PNIO_UINT32 page_size)
{
	PNIO_UINT32 data_word, check_word;
	PNIO_UINT32 i = 0;
	PNIO_UINT32 check = 0;

	/* Jump in binary to start of file - ITCM section */
	fseek(source, 0, SEEK_SET);

	/* Set page */
	if(PNPB_NOT_OK == pnpb_set_multiple_pages(p_xhif_set_page, 0, 2, 0x08000000))
	{
		return PNPB_NOT_OK;
	}

#if (1 == PNPB_LIB_SET_VERBOSE)
	printf("ITCM: Check from 0x%x to 0x%x \n", 0x08030044 + i, 0x08030044 + page_size);
#endif
	/* Check word by word */
	for(; i < page_size; i += 4)
	{
		fread(&data_word, 4, 1, source);
		check_word = READ32(p_xhif_data + 0x00030044 + i);
		/* Compare data from binary against data read from Ertec */
		if(data_word != check_word)
		{
			check ++;
#if (1 == PNPB_LIB_SET_VERBOSE)
			printf("Error at %08x: %08x but expected %08x\n", 0x08030044 + i,
					check_word, data_word);
#endif
		}
	}
#if (1 == PNPB_LIB_SET_VERBOSE)
	printf("    0x%x bytes successfully checked at 0x%x\n", i, 0x08030044);
#endif
	return check;
}	/* pnpb_check_firmware_in_itcm */


/**
 *  @brief Checks whole firmware from binary against data in Ertec SDRAM
 *
 *  @param[in]      *p_xhif_set_page    Pointer to GPMC bank for page switch, mounted by mmap
 *  @param[in]      *p_xhif_data        Pointer to GPMC bank for xhif data exchange, mounted by mmap
 *  @param[in]      *source             Pointer to file with firmware for Ertec
 *  @param[in]      sum_pages           Number of firmware pages
 *  @param[in]      itcm_size           Size of ITCM part of firmware
 *  @param[in]      binary_size         Size of SDRAM part of firmware
 *  @return                             Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  GPMC - XHIF firmware transfer check. Have to be run in primary bootloader of Ertec,
 *  otherwise the data might be changed by Ertec runtime and check might fail.
 *
 */
PNIO_UINT32 pnpb_check_loaded_firmware( PNIO_VOID* p_xhif_set_page,
                                        PNIO_VOID* p_xhif_data,
                                        FILE* source,
                                        PNIO_UINT32 sum_pages,
                                        PNIO_UINT32 itcm_size,
                                        PNIO_UINT32 binary_size)
{
	PNIO_UINT32 page, page_start, page_size, check;
	page = 0;
	if(NULL == source)
	{
#if (1 == PNPB_LIB_SET_VERBOSE)
		printf("Error: File does not exist!\n\n");
#endif
		return PNPB_NOT_OK;
	}
	/* Check firmware in SDRAM */
	while(page < sum_pages)
	{
		/* Prepare start address of page */
		if(0 == page)
		{
			/* First page */
			page_start = itcm_size + 0x44;
			fseek(source, (itcm_size), SEEK_SET);
		}
		else
		{
			page_start = 0;
		}
		/* Prepare size to be copied */
		if((0 != (binary_size % PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE)) &&
				(page == (sum_pages - 1)))
		{
			/* Last page size */
			page_size = binary_size + itcm_size + 44 - (page * PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE);
		}
		else
		{
			page_size = PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE;
		}

		/* Set page */
		pnpb_set_multiple_pages(p_xhif_set_page, 0, 7, (0x20000000 + (page * PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE)));
		/* Check one page */
		check =	pnpb_check_one_page_of_firmware(p_xhif_set_page, p_xhif_data, source, page, page_size, page_start);

		/* Evaluate checked page */
		if(PNPB_OK != check)
		{
			printf("Error: %d bytes of firmware copied wrong to SRAM\n\n", check);
			PNPB_LIB_FATAL
		}

		page++;
	}

	/* Check ITCM */
	check = pnpb_check_firmware_in_itcm(p_xhif_set_page, p_xhif_data, source, itcm_size);
	/* Evaluate ITCM check*/
	if(PNPB_OK != check)
	{
		printf("Error: %d bytes of firmware copied wrong to ITCM\n\n", check);
		PNPB_LIB_FATAL
	}
	return PNPB_OK;
}	/* pnpb_check_loaded_firmware */

/**
 *  @brief For an external host processor to recognize whether the
 *  XHIF interface is ready, the following procedure must be observed
 *
 *  @param[in]      *p_xhif_set_page    Pointer to GPMC bank for page switch, mounted by mmap
 *  @return                             Success[PNPB_OK, PNPB_NOT_OK]
 *
 */
PNIO_UINT32 pnpb_wait_for_ertec(PNIO_VOID* p_xhif_set_page, PNIO_VOID* p_xhif_data)
{
    PNIO_UINT32 val;
    struct timespec delay, rem;
    delay.tv_sec = 0;
    delay.tv_nsec = 50000;

    printf("Waiting for XHIF interface to become ready...\n");

    /*
     * Host read XHIF range register bank 0
     * Value == 0x0010_0000 -> continue
     */

    do
    {
        /* Wait a little */
        nanosleep(&delay, &rem);
        val = READ32(p_xhif_set_page + 0x00);
    } while(val != 0x00100000);

    /*
     * Host read XHIF-R offset register bank 0
     * Value == 0x0800_0000 -> continue
     */

    do
    {
        /* Wait a little */
        nanosleep(&delay, &rem);
        val = READ32(p_xhif_set_page + 0x04);
    } while(val != 0x08000000);

    printf("Waiting for D-TCM memory to become ready...\n");

    /* Set page of XHIF */
    if(PNPB_NOT_OK == pnpb_set_multiple_pages(p_xhif_set_page, 5, 2, 0x08000000))
    {
        return PNPB_NOT_OK;
    }

    /*
     * Host read D-TCM+0x40 (page 5)
     * Value == 0x0000_0000 -> continue
     */

    do
    {
        /* Wait a little */
        nanosleep(&delay, &rem);
        val = READ32(p_xhif_data + PNPB_XHIF_OFFSET_PAGE_5 + 0x00030040);
    } while(val != 0);

    /*
     * Host write/read D-TCM+0x44 (page 5)
     * Value == Exp_value -> continue
     */

    do
    {
        WRITE32((p_xhif_data + PNPB_XHIF_OFFSET_PAGE_5 + 0x00030044), 0xDEADBEEF);
        /* Wait a little */
        nanosleep(&delay, &rem);
        val = READ32(p_xhif_data + PNPB_XHIF_OFFSET_PAGE_5 + 0x00030044);
    } while(val != 0xDEADBEEF);

    return PNPB_OK;
}

/**
 *  @brief Clean acyclic messaging queues
 *
 *  @param[in]      *p_xhif_set_page    Pointer to GPMC bank for page switch, mounted by mmap
 *  @param[in]      *p_xhif_data        Pointer to GPMC bank for xhif data exchange, mounted by mmap
 *  @return                             Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Queues placed in Ertec, but have to be cleaned from BBB before fwup
 *
 */
PNIO_UINT32 pnpb_cleanup_ertec(PNIO_VOID* p_xhif_set_page, PNIO_VOID* p_xhif_data)
{
    PNIO_VOID *tmp_acyc_in_p;
    PNIO_VOID *tmp_acyc_out_p;
    PNIO_UINT32 page_size = 2 * PNPB_XHIF_PAGE_SIZE; /* Acyclic queues occupies two pages each */

    /* Set pages of XHIF */
    pnpb_set_multiple_pages(p_xhif_set_page, 0, 7, PNPB_XHIF_SECTION_CYC_IN);

    /* Pointers to acyclic queues */
    tmp_acyc_in_p = (PNIO_VOID*)(p_xhif_data + PNPB_XHIF_SECTION_ACY_IN - PNPB_XHIF_SECTION_CYC_IN);
    tmp_acyc_out_p = (PNIO_VOID*)(p_xhif_data + PNPB_XHIF_SECTION_ACY_OUT - PNPB_XHIF_SECTION_CYC_IN);

    /* Set to 0 */
    memset(tmp_acyc_in_p, 0, page_size);
    memset(tmp_acyc_out_p, 0, page_size);

    return PNPB_OK;
}   /* pnpb_cleanup_ertec */


/**
 *  @brief Copy firmware to Ertec, verify.
 *
 *  @param[in]      *p_xhif_set_page    Pointer to GPMC bank for page switch, mounted by mmap
 *  @param[in]      *p_xhif_data        Pointer to GPMC bank for xhif data exchange, mounted by mmap
 *  @return                             Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Copies all firmware from binary file to Ertec. Verifies copied data.
 *
 */
PNIO_UINT32 pnpb_copy_firmware(PNIO_VOID* p_xhif_set_page, PNIO_VOID* p_xhif_data)
{
	FILE* source;
	PNIO_UINT32 data_word, page_start, binary_size, itcm_size, sum_pages, page_size;
	PNIO_UINT32 page = 0;
	PNIO_UINT32 check = 0;

	/* Open binary with fw */
	source = fopen(PNPB_BINARY_FOR_ERTEC, "r");
	if(PNPB_NULL == source)
	{
		printf("Error: Failed to open file with binary for Ertec!\n\n");
		PNPB_LIB_FATAL
	}
	if(PNPB_NOT_OK == pnpb_obtain_binary_size(source, &binary_size, &itcm_size))
	{
		return PNPB_NOT_OK;
	}
#if (1 == PNPB_LIB_SET_VERBOSE)
	printf("Ready to copy from binary to Ertec: \n"
			"  ITCM : 0x%x bytes \n  SDRAM: 0x%x bytes \n",
			itcm_size, binary_size);
	printf(" ITCM: %x - %x  SDRAM %x - %x\n",
			0x08000044, (0x08000044 + itcm_size),
			itcm_size + 0x20000044, itcm_size + 0x20000044 + binary_size);
#endif
	/* Firmware can not be copied at once due to available range of XHIF addresses - split it to pages */
	sum_pages = (binary_size + itcm_size + 44) / PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE;
	if(0 != ((binary_size + itcm_size + 44) % PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE))
	{
		sum_pages += 1;
	}
	if(PNPB_NULL == source)
	{
		printf("Error: Failed to open file with binary for Ertec!\n\n");
		PNPB_LIB_FATAL
	}
	else
	{
		/*Copy to SDRAM without TCM*/
		while(page < sum_pages)
		{
			/* Decide addresses */
			if(0 == page)
			{
				/* First page is not whole - part went to ITCM */
				/* It is futile to copy those bytes again -> skip */
				page_start = itcm_size + 0x44;
				/* Jump in binary behind TCM section */
				fseek(source, (itcm_size), SEEK_SET);
			}
			else
			{
				page_start = 0;
			}

			if((0 != (binary_size % PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE)) &&
				(page == (sum_pages - 1)))
			{
				/* There is one more page, but it is not whole */
				page_size = binary_size  + itcm_size + 44 - (page * PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE);
			}
			else
			{
				page_size = PNPB_BINARY_COPY_BYTES_PER_XHIF_PAGE;
			}

			if(PNPB_NOT_OK ==
				pnpb_copy_one_page_of_firmware(p_xhif_set_page, p_xhif_data, source, page, page_size, page_start))
			{
				return PNPB_NOT_OK;
			}

			page++;
		}
	}

	/* Copy to TCM */
	if(PNPB_NOT_OK == pnpb_copy_firmware_to_itcm(p_xhif_set_page, p_xhif_data, source, itcm_size))
	{
		return PNPB_NOT_OK;
	}

	/* Check loaded data */
	if(PNPB_NOT_OK ==
			pnpb_check_loaded_firmware(p_xhif_set_page,p_xhif_data, source, sum_pages, itcm_size, binary_size))
	{
		return PNPB_NOT_OK;
	}
	if(PNPB_NULL == source)
	{
		printf("Error: Failed to open file with binary for Ertec!\n\n");
		PNPB_LIB_FATAL
	}
	else
	{
		/* Cleanup */
		fclose(source);
		return PNPB_OK;
	}
}	/* pnpb_copy_firmware */

/**
 *  @brief Continue from primary bootloader
 *
 *  @param[in]      *p_xhif_set_page    Pointer to GPMC bank for page switch, mounted by mmap
 *  @param[in]      *p_xhif_data        Pointer to GPMC bank for xhif data exchange, mounted by mmap
 *  @return                             Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Writes keyword to enable Ertec to continue from primary bootloader to secondary.
 *
 */
PNIO_UINT32 pnpb_start_firmware(PNIO_VOID* p_xhif_set_page, PNIO_VOID* p_xhif_data)
{
	PNIO_UINT32 data_word;

	/* Activate firmware in Ertec */
	/* The primary bootloader is in wait loop until this*/
	if(PNPB_NOT_OK == pnpb_set_multiple_pages(p_xhif_set_page, 0, 2, 0x08000000))
	{
		return PNPB_NOT_OK;
	}
	data_word = PNPB_ERTEC_FIRMWARE_LOADED_CODE;
	WRITE32((p_xhif_data + 0x00030040), data_word);

	return PNPB_OK;
}	/* pnpb_start_firmware */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
