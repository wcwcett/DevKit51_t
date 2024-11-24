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
/*  F i l e               &F: pnpb_lib_main.c                           :F&  */
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
* @file     pnpb_lib_main.c
* @brief    PNPB library for XHIF - main interface
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
#include <pthread.h>    /*multithreading library*/
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>

#include <sched.h>

#include "pnpb_gpio_lib.h"
/*disable Ertec reset and load of */
#define NO_LOAD     0
/*if debug option enabled, FW must be loaded from debugger. */
#define DEBUGGER_TIMEOUT 60

static PNIO_UINT32 devmem;
static PNIO_VOID* p_xhif_data;
static PNIO_VOID* p_xhif_set_page;
static PNIO_VOID* p_gpmc_regs;

typedef struct _sig_ucontext {
 unsigned long     uc_flags;
 struct ucontext   *uc_link;
 stack_t           uc_stack;
 struct sigcontext uc_mcontext;
 sigset_t          uc_sigmask;
} sig_ucontext_t;

volatile int pnpb_keep_running = PNPB_THREAD_RUNNING;
volatile int acyc_gpio_running = PNPB_THREAD_TERMINATE;
volatile int acyc_service_running = PNPB_THREAD_TERMINATE;
volatile int cyc_gpio_running = PNPB_THREAD_TERMINATE;
volatile int cyc_service_running = PNPB_THREAD_TERMINATE;
volatile bool sig_captured = false;
volatile bool ertec_running = false;

void sig_int_capture(int sig_num, siginfo_t * info, void * ucontext)
{
    /* Handle signal only once! */
    if(sig_captured == false)
    {
        sig_captured = true;
        printf("\nSignal interrupt captured! Start handling!\n");

        /* Deinit library */
        pnpb_lib_deinit();

        /* Reset ERTEC */
        pnpb_reset_Ertec();

        printf("GPIO library deinit...\n");
        bbb_gpio_deinit();

        printf("GPMC registers unmapping...\n");
        munmap(p_gpmc_regs, PNPB_GPMC_SETTINGS_REGS_SIZE);
        munmap(p_xhif_data, PNPB_XHIF_SIZE);
        munmap(p_xhif_set_page, PNPB_XHIF_SET_REG_SIZE);
        close(devmem);

        printf("Terminating main thread...\n");
        printf("Exit application...\n");
        exit(0);
    }
    else
    {
        printf("\nSignal interrupt captured! Handling already in progress!\n");
    }
}

void seg_fault_capture(int sig_num, siginfo_t * info, void * ucontext)
{

    void *             caller_address;
    sig_ucontext_t *   uc;

    uc = (sig_ucontext_t *)ucontext;

    caller_address = (void *) uc->uc_mcontext.arm_pc;

    fprintf(stderr, "SIG_SEGV : address is %p from %p\n",
            info->si_addr,
            (void *)caller_address);

    /* Fatal error */
    PNPB_LIB_FATAL
}

/**
 *  @brief Fatal Error of PNPB library
 *
 *  @param[in]      void
 *
 *  Provide debug data, if possible
 *  Reset Ertec
 *  End program
 *
 */
PNIO_VOID pnpb_lib_in_fatal()
{
    fprintf(stderr, "\n\n\n-----------------------------------------------------------\n");
    fprintf(stderr, "  DEVICE IN FATAL ERROR\n");
    fprintf(stderr, "-----------------------------------------------------------\n\n\n");

    /* Catch data from Ertec before reset! */
    fprintf(stderr, "Acyc  IN: W:%03d R: %03d \n", PNPB_XHIF_ACYC_IN_SERV.write_ptr,
            PNPB_XHIF_ACYC_IN_SERV.read_ptr);
    fprintf(stderr, "Acyc OUT: W:%03d R: %03d \n", PNPB_XHIF_ACYC_OUT_SERV.write_ptr,
            PNPB_XHIF_ACYC_OUT_SERV.read_ptr);
    fprintf(stderr, "Cycl  IN: W:%03d R: %03d - to stack\n", PNPB_XHIF_CYCLIC_IN_SERV.write_ptr,
            PNPB_XHIF_CYCLIC_IN_SERV.read_ptr);
    fprintf(stderr, "Cycl OUT: W:%03d R: %03d- from stack\n", PNPB_XHIF_CYCLIC_OUT_SERV.write_ptr,
            PNPB_XHIF_CYCLIC_OUT_SERV.read_ptr);

    /* Reset Ertec - this will stop all PN services */
    pnpb_reset_Ertec();
    /* Exit the program */
	exit(EXIT_FAILURE);
}   /* pnpb_lib_in_fatal */

/**
 *  @brief Entry function of PNPB library
 *
 *  @param[in]   	void
 *  @return      				Success[PNPB_OK, PNPB_NOT_OK]
 *
 *  Prepares GPMC - XHIF interface.
 *  Loads firmware to Ertec.
 *  Prepares memory interface.
 *  Calls user functionality.
 *
 */
PNIO_UINT32 main(PNIO_VOID)
{
	PNIO_UINT32 tmp_addr;
	PNIO_UINT32 tmp;
	PNIO_UINT32 test;
    PNIO_UINT32 rv;

    /* Signal handler */
    struct sigaction seg_fault_act;
    struct sigaction sig_int_act;

    /* For scheduler settings */
    struct sched_param param;
    param.sched_priority = sched_get_priority_max (SCHED_RR);

    /* Set the scheduling policy and parameters of a thread */
    if ( sched_setscheduler(0, SCHED_RR, &param) == -1 )
    {
        printf("Scheduler configuration failed!");
        PNPB_LIB_FATAL
    }

    /* Install signal handlers */
    seg_fault_act.sa_sigaction = seg_fault_capture;
    sigemptyset (&seg_fault_act.sa_mask);
    seg_fault_act.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction(SIGSEGV, &seg_fault_act, (struct sigaction *)NULL);

    sig_int_act.sa_sigaction = sig_int_capture;
    sigemptyset (&sig_int_act.sa_mask);
    sig_int_act.sa_flags = 0;
    sigaction(SIGINT, &sig_int_act, (struct sigaction *)NULL);

    /* Init random number generator */
    srand(time(NULL));

	printf("\n\n");

	/* Check if XHIF was activated in iod_cfg.h*/
	if(1 != IOD_USED_WITH_XHIF_HOST)
	{
	    printf("Error: XHIF not activated in Application/App_common/iod_cfg.h");
	    PNPB_LIB_FATAL
	}

	/* Open mem */
	devmem = open("/dev/mem", O_RDWR | O_SYNC);
	if(-1 == devmem)
	{
		printf( "Cannot open /dev/mem\n error: %s \n", strerror(errno));
		PNPB_LIB_FATAL
	}

	/* Map GPIO registers */
	if(bbb_gpio_init(devmem) == PNPB_NOT_OK)
	{
        printf( "GPIO register initialization failed\n");
        PNPB_LIB_FATAL
	}

	/* Map GPMC registers - cannot be accessed directly (Segmentation fault) */
	p_gpmc_regs = mmap(PNPB_XHIF_STRT, PNPB_GPMC_SETTINGS_REGS_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, devmem, PNPB_GPMC_SETTINGS_REGS_OFFSET);
	if((PNIO_VOID*) -1 == p_gpmc_regs)
	{
		printf("Cannot mmap\n error: %s \n", strerror(errno));
		PNPB_LIB_FATAL
	}
	/* Now, GPMC registers can be set */
	pnpb_set_gpmc(p_gpmc_regs);

	/* Open xhif for data - default bank 6 */
	p_xhif_data = mmap(PNPB_XHIF_STRT, PNPB_XHIF_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, devmem, PNPB_XHIF_DATA_OFFSET);
	if((PNIO_VOID*) -1 == p_xhif_data)
	{
		/* Cleanup if failure */
		printf("Cannot mmap\n error: %s \n", strerror(errno));
		munmap(p_gpmc_regs, PNPB_GPMC_SETTINGS_REGS_SIZE);
		PNPB_LIB_FATAL
	}
	/* Open xhif for page settings */
	p_xhif_set_page = mmap(PNPB_XHIF_STRT, PNPB_XHIF_SET_REG_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, devmem, PNPB_XHIF_SET_REG_OFFSET);
	if((PNIO_VOID*) -1 == p_xhif_set_page)
	{
		/* Cleanup if failure */
		printf("Cannot mmap\n error: %s \n", strerror(errno));
		munmap(p_gpmc_regs, PNPB_GPMC_SETTINGS_REGS_SIZE);
		munmap(p_xhif_data, PNPB_XHIF_SIZE);
		PNPB_LIB_FATAL
	}

	/* Open GPIOs */

    /* SEM0 - ertec ready flag */
    rv = bbb_gpio_open(GPIO_BANK_0, GPIO_NUM_15, bbb_gpio_input);
    if(PNPB_OK != rv)
    {
        printf("Cannot open SEM0 (GPIO library error)!\n");
        PNPB_LIB_FATAL
    }

	/* Open SEM1 - command to Ertec */
    if(PNPB_OK != bbb_gpio_open(GPIO_BANK_0, GPIO_NUM_14, bbb_gpio_output))
    {
        printf("Cannot open SEM1 (GPIO library error)!\n");
        PNPB_LIB_FATAL
    }

    /* SEM3 - acyclic receival*/
    rv = bbb_gpio_open(GPIO_BANK_3, GPIO_NUM_19, bbb_gpio_input);
    if(PNPB_OK != rv)
    {
        printf("Cannot open SEM3 (GPIO library error)!\n");
        PNPB_LIB_FATAL
    }

    /* IRT SYNC 0 - cyclic receival*/
    rv = bbb_gpio_open(GPIO_BANK_3, GPIO_NUM_17, bbb_gpio_input);
    if(PNPB_OK != rv)
    {
        printf("Cannot open IRT SYNC 0 (GPIO library error)!\n");
        PNPB_LIB_FATAL
    }

    /* SEM2 - confirmation of acyclic receival */
    rv = bbb_gpio_open(GPIO_BANK_3, GPIO_NUM_21, bbb_gpio_output);
    if(PNPB_OK != rv)
    {
        printf("Cannot open SEM2 (GPIO library error)!\n");
        PNPB_LIB_FATAL
    }

    /* Init semaphore before use - load and start of firmware in Ertec will trigger it */
    sem_init(&PnioDeviceReadySemId, 0, 0);
    sem_init(&TracesSemId, 0, 0);
    sem_post(&TracesSemId);

	/* Prepare mutex to protect GPMC interface access */
	if (pthread_mutex_init(&gpmc_access_mutex, NULL) != 0)
	{
        printf("Mutex init failed \n");
        PNPB_LIB_FATAL
	}

    /* Init library */
    pnpb_lib_init();

    PnioLogDest = 1;    /* Activate application print to console */
	MainAppl(); /* Application example. Never return from here */

	/* Never return - No code after here, only error, cleanup */
	/* Cleanup */
	bbb_gpio_deinit();
	munmap(p_gpmc_regs, PNPB_GPMC_SETTINGS_REGS_SIZE);
	munmap(p_xhif_data, PNPB_XHIF_SIZE);
	munmap(p_xhif_set_page, PNPB_XHIF_SET_REG_SIZE);
	close(devmem);
	printf("\nError:Program end - returned from usriod_main.c functionality\n\n");
	PNPB_LIB_FATAL
}

/**
 *  @brief Reboot library
 *
 *  @param[in]      void
 *
 */
PNIO_VOID pnpb_lib_reboot()
{
    /* Wait for previous trace command to end */
    sem_wait(&TracesSemId);

	pnpb_lib_deinit();
	pnpb_lib_init();

    /* Allow trace commands */
    sem_post(&TracesSemId);
}

/**
 *  @brief Terminate all threads of library
 *
 *  @param[in]      void
 */
PNIO_VOID pnpb_lib_deinit()
{
    const struct timespec delay = {0, 1000000L};
    struct timespec rem;
	
	//lint -e{578} Declaration of symbol 'NumOfAr' hides symbol 'NumOfAr'
    if((ertec_running == true) && (NumOfAr > 0))
    {
        /* Cancel AR */
        printf("\nCancelling AR...\n");
        pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_DEVICE_STOP, 0, (void*)0);
        while(NumOfAr > 0)
        {
            /* Sleep for predefined delay - empty loop of process with high prio would block CPU! */
            nanosleep(&delay, &rem);
        }
    }

    /* Terminate all threads! */
    pnpb_keep_running = PNPB_THREAD_TERMINATE;

    /* Wait for threads */
	//lint -e{578} Declaration of symbol 'acyc_gpio_running' hides symbol 'acyc_gpio_running' 
	while(acyc_gpio_running == PNPB_THREAD_RUNNING)
    {
        /* Sleep for predefined delay - empty loop of process with high prio would block CPU! */
        nanosleep(&delay, &rem);
    }
    printf("Acyclic GPIO capture thread terminating...\n");
    while(acyc_service_running == PNPB_THREAD_RUNNING)
    {
        /* Sleep for predefined delay - empty loop of process with high prio would block CPU! */
        nanosleep(&delay, &rem);
    }
    printf("Acyclic service thread terminating...\n");
    while(cyc_gpio_running == PNPB_THREAD_RUNNING)
    {
        /* Sleep for predefined delay - empty loop of process with high prio would block CPU! */
        nanosleep(&delay, &rem);
    }
    printf("Cyclic GPIO capture thread terminating...\n");
    while(cyc_service_running == PNPB_THREAD_RUNNING)
    {
        /* Sleep for predefined delay - empty loop of process with high prio would block CPU! */
        nanosleep(&delay, &rem);
    }
    printf("Cyclic service thread terminating...\n");

    ertec_running = false;
}

/**
 *  @brief Reset Ertec and load firmware. Initialize all threads of library.
 *
 *  @param[in]      void
 */
PNIO_VOID pnpb_lib_init()
{
    PNIO_UINT32 val;

    pthread_t acyclic_handler;
    pthread_t cyclical_handler;

    struct sched_param param;
    param.sched_priority = sched_get_priority_min (SCHED_RR);

	/* Thread handlers */
    pthread_t this_thread = pthread_self();

    const struct timespec delay = {0, 100000L};
    struct timespec rem;

    /* Terminate all threads! */
    pnpb_keep_running = PNPB_THREAD_RUNNING;

    /* Set default values */
    //lint -e{832, 578} Parameter 'Symbol' not explicitly declared, int assumed
	bbb_gpio_set(GPIO_BANK_3, GPIO_NUM_21, 0);
    bbb_gpio_set(GPIO_BANK_0, GPIO_NUM_14, 0);

#if(NO_LOAD == 0)
    /* Toggle reset signal of Ertec board */
    pnpb_reset_Ertec();
#endif

	PnpbDeviceStartupState = PNPB_USR_START_IDLE;   /*State of Ertec boot - boot not performed*/

#if(NO_LOAD == 0)
	/* Detection of ERTEC 200P readiness (for XHIF boot) by the external host */
    pnpb_wait_for_ertec(p_xhif_set_page, p_xhif_data);
    printf("XHIF interface is ready to be used!\n");
#endif

	/* Prepare Ertec to recieve firmware */
	pnpb_set_ertec_registers(p_xhif_set_page, p_xhif_data);

    /* From now on, all fatal errors should be handled by fatal error function */

#if(NO_LOAD == 0)
    /* Cleanup queue for acyclic messaging from Ertec */
    pnpb_cleanup_ertec(p_xhif_set_page, p_xhif_data);
	/* Copy firmware to Ertec */
#if(DEBUG_XHIF == 1)
    printf("Start loading binary via debugger. If take too long, please update DEBUGGER_TIMEOUT.\n");
    int timeout = DEBUGGER_TIMEOUT; 
    while(timeout>0){
        printf("\r  ");
        printf("\r%d",timeout);
        fflush(stdout);
        sleep(1);
        timeout--;
    }
    printf("\n"); 
#else
    pnpb_copy_firmware(p_xhif_set_page, p_xhif_data);
#endif
    /* Start firmware in Ertec */
    pnpb_start_firmware(p_xhif_set_page, p_xhif_data);
#endif

	/* Prepare runtime memory interface */
	pnpb_xhif_memory_interface_start(p_xhif_set_page, p_xhif_data);
	pnpb_xhif_prepare_function_calls();

	/* Wait for ertec to Enter MainAppl */
	printf("Waiting for ertec to Enter MainAppl...\n");
    do
    {
        val = bbb_gpio_get(GPIO_BANK_0, GPIO_NUM_15);
		/* Sleep for predefined delay */
		nanosleep(&delay, &rem);
    } while (val != 0);

	/* Start thread for acyclic capture */
    pthread_create(&acyclic_handler, NULL, pnpb_acyclical_capture_thread, NULL);
	/* Wait for Ertec to become ready */
	sem_wait(&PnioDeviceReadySemId);
	if(PNPB_USR_START_BOOT_OK != PnpbDeviceStartupState)
	{
	    printf("Error: Invalid state of Ertec boot-up\n");
	    PNPB_LIB_FATAL
	}
	else
	{
	    ertec_running = true;
	}

	/* Start thread for cyclic capture */
	pthread_create(&cyclical_handler, NULL, pnpb_cyclical_execute_thread, NULL);

	/* Configure this thread to have low priority */
	param.sched_priority = sched_get_priority_min (SCHED_RR);
    if (pthread_setschedparam(this_thread, SCHED_RR, &param) != 0)
    {
        printf("Thread priority (MainAppl - low prio) configuration failed!");
        PNPB_LIB_FATAL
    }
}

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
