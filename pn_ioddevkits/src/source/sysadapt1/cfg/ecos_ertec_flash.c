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
/*  F i l e               &F: ecos_ertec_flash.c                        :F&  */
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


#define LTRC_ACT_MODUL_ID   111
#define IOD_MODULE_ID       111


#include "compiler.h"
#if (PNIOD_PLATFORM & PNIOD_PLATFORM_ECOS_EB200P)
    #include "compiler_stdlibs.h"
    #include "pniousrd.h"
    #include "os_flash.h"
    #include "os.h"
    #include "os_taskprio.h"
    #include "nv_data.h"
    #include "trc_if.h"
    #include "ertec_inc.h"
    #include "ertec_x00.h"
    #include <stdio.h>

    // *********** defines **********
    #define FLASH_INIT_VALUE            0xff        // value after erasing flash
    #define FLASH_SPI_TYPE              1
    #define FLASH_NAND_TYPE             2
    #define FLASH_OSPI_TYPE             3

    /* Which nv memory will be used in case of XHIF boot */
    #define FLASH_DEF_TYPE              FLASH_NAND_TYPE
//    #define FLASH_DEF_TYPE         FLASH_SPI_TYPE

    #define ENTER_FLASH_DATA            OsEnterX(OS_MUTEX_FLASH_DATA);
    #define EXIT_FLASH_DATA             OsExitX(OS_MUTEX_FLASH_DATA);

    #define NUM_FLASH_IMAGES            (2)

    #define FLASH_USERDATA_START        (U_EMC_MEM_ASYNC_CHIP_SELECT0__START + 0x00f00000)  // S29 WS 128P has 32k sectors at bottom & top, else 128k
    #define FLASH_USERDATA_SEC_OFFSET   (0x00080000) /* Sector offset */
    #define FLASH_USERDATA_SEC1         (FLASH_USERDATA_START + FLASH_USERDATA_SEC_OFFSET * 0) /* Sector 121 */
    #define FLASH_USERDATA_SEC2         (FLASH_USERDATA_START + FLASH_USERDATA_SEC_OFFSET * 1) /* Sector 122 */
    #define FLASH_USERDATA_SIZE         0x01000000  //16MB = 2x 8MB 16 bit flash

    #include <cyg/io/flash.h>

    // ******* static data ********
    static PNIO_UINT8*  flashramimage;
    static PNIO_UINT32  UsrNvBlockSize;
    static PNIO_UINT32  UsrNvImageSize;
    static PNIO_UINT32  FlashType;
    static PNIO_UINT32  TskId_Flash;
    static PNIO_UINT32  flashSemId;
    static PNIO_UINT32  flashRamSemId;

    PNIO_BOOL EnableOsFlashPrintf = PNIO_FALSE;

    // ******* structure with callback and argument passed to queue ********
    typedef struct
    {
        nv_function_handler callback;
        PNIO_VOID*          arg;
    } FLASH_CBF_STR;

    // *----------------------------------------------------------------*
    // *
    // *  OsFlashPrintf()
    // *----------------------------------------------------------------*
    // *  Log ecos flash access when enabled
    // *
    // *----------------------------------------------------------------*
    PNIO_VOID OsFlashPrintf(PNIO_CHAR* fmt, ...)
    {
	    if (EnableOsFlashPrintf)
	    {
		    va_list argptr = {0};
		    va_start(argptr, fmt);

		    PNIO_printf (fmt, argptr);
		    fflush(stdout);

		    va_end (argptr);
	    }
    }

    // *----------------------------------------------------------------*
    // *
    // *  Task_NvDataPost ()
    // *
    // *----------------------------------------------------------------*
    // *
    // *
    // *  Input:       ----
    // *
    // *  Output:      return               PNIO_OK, PNIO_NOT_OK
    // *
    // *  post thread for handling nv data write functions in a very
    // *  low thread context
    // *----------------------------------------------------------------*
    // *----------------------------------------------------------------*
    static PNIO_UINT32  Task_OsFlashPost (PNIO_VOID)
    {
        PNIO_VOID * p1     = LSA_NULL;     // pointer to Requestblock
        FLASH_CBF_STR * p2 = LSA_NULL;
        PNIO_UINT32 Status = PNIO_OK;
        int                 CygStat;
        cyg_flashaddr_t     flash_err_address = 0;
	    nv_function_handler flash_callback;
        // Wait until own TaskID has been saved by the father process
        OsWaitOnEnable();
        PNIO_BOOL flash_second_partition;

        while (1)
        {
            OsReadMessageBlocked(&p1, TskId_Flash);

            if (p1 == 0)
            {
                LSA_TRACE_00  (TRACE_SUBSYS_IOD_MEMORY, LSA_TRACE_LEVEL_FATAL, "p1 = 0 in Task_NvDataPost\n");
            }
            else
            {
                p2 = (FLASH_CBF_STR *) p1;

                if(p2->callback == 0)
                {
                    LSA_TRACE_00   (TRACE_SUBSYS_IOD_MEMORY,
                                    LSA_TRACE_LEVEL_FATAL,
                                    "OsFlash : callback error\n");
                    continue;
                }

                flash_callback = (nv_function_handler) p2->callback;
                nv_function_param* prm = (nv_function_param*) p2->arg;
                flash_second_partition = PNIO_TRUE;
                if (prm != NULL)
                {
                    if(prm->restore_first_section == PNIO_TRUE)
                    {
                        /* This case is valid only when the data from first section are corrupted
                         * and therefore the data from the second section are used.
                         * This flag is used to restore first section and avoid erase of second section
                         * before the flash is done */
                        flash_second_partition = PNIO_FALSE;
                    }
                }

                switch ( FlashType )
                {
#ifndef BOARD_TYPE_STEP_3
                    case FLASH_SPI_TYPE:
                    {
                        if (flash_second_partition == PNIO_TRUE)
                        {
                            /*
                              * Prepare second partition
                              */

                             /* Erase second partition */
                             OsTakeSemB(flashSemId);
                             spi_in_use = 1;
                             CygStat= FLASH_ERASE(SPI_NV_ADDRESS_SEC2,
                                                 UsrNvImageSize,
                                                 &flash_err_address);
                             if (CygStat != CYG_FLASH_ERR_OK)
                             {
                                 LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                                                 LSA_TRACE_LEVEL_FATAL,
                                                 "erase error %d\n",
                                                 CygStat);
                             }
                             spi_in_use = 0;
                             OsGiveSemB(flashSemId);

                             /* Program second partition */
                             OsTakeSemB(flashSemId);
                             spi_in_use = 1;
                             CygStat= FLASH_PROGRAM(SPI_NV_ADDRESS_SEC2,
                                                    flashramimage,
                                                    UsrNvImageSize,
                                                    &flash_err_address);
                             if (CygStat != CYG_FLASH_ERR_OK)
                             {
                                 LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                                                 LSA_TRACE_LEVEL_FATAL,
                                                 "program error %d\n",
                                                 CygStat);
                             }
                             spi_in_use = 0;
                             OsGiveSemB(flashSemId);
                        }

                        /*
                         * Prepare first partition
                         */

                        /* Erase first partition */
                        OsTakeSemB(flashSemId);
                        spi_in_use = 1;
                        CygStat= FLASH_ERASE(SPI_NV_ADDRESS_SEC1,
                                            UsrNvImageSize,
                                            &flash_err_address);
                        if (CygStat != CYG_FLASH_ERR_OK)
                        {
                            LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                                            LSA_TRACE_LEVEL_FATAL,
                                            "erase error %d\n",
                                            CygStat);
                        }
                        spi_in_use = 0;
                        OsGiveSemB(flashSemId);

                        /* Program first partition */
                        OsTakeSemB(flashSemId);
                        spi_in_use = 1;
                        CygStat= FLASH_PROGRAM(SPI_NV_ADDRESS_SEC1,
                                               flashramimage,
                                               UsrNvImageSize,
                                               &flash_err_address);
                        if (CygStat != CYG_FLASH_ERR_OK)
                        {
                            LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                                            LSA_TRACE_LEVEL_FATAL,
                                            "program error %d\n",
                                            CygStat);
                        }
                        spi_in_use = 0;
                        OsGiveSemB(flashSemId);

                        /* it is not necessary to erase second partition - keep it */

                        break;
                    }
                    case FLASH_NAND_TYPE:
                    {
                        if (flash_second_partition == PNIO_TRUE)
                        {
                            /*
                             * Prepare second partition
                             */

                            OsTakeSemB(flashSemId);
                            CygStat = cyg_flash_erase(FLASH_USERDATA_SEC2,
                                                    UsrNvImageSize,
                                                    &flash_err_address);
                            if (CygStat != CYG_FLASH_ERR_OK)
                            {
                                LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                                                LSA_TRACE_LEVEL_FATAL,
                                                "erase error %d\n",
                                                CygStat);
                            }
                            OsGiveSemB(flashSemId);

                            OsTakeSemB(flashSemId);
                            OsTakeSemB(flashRamSemId);

                            CygStat= cyg_flash_program (FLASH_USERDATA_SEC2,
                                                        flashramimage,
                                                        UsrNvImageSize,
                                                        &flash_err_address);

                            OsGiveSemB(flashRamSemId);
                            OsGiveSemB(flashSemId);

                            if (CygStat != CYG_FLASH_ERR_OK)
                            {
                                LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                                                LSA_TRACE_LEVEL_FATAL,
                                                "program error %d\n",
                                                CygStat);
                            }
                        }

                        /*
                         * Prepare first partition
                         */

                        OsTakeSemB(flashSemId);
                        CygStat = cyg_flash_erase(FLASH_USERDATA_SEC1,
                                                UsrNvImageSize,
                                                &flash_err_address);
                        if (CygStat != CYG_FLASH_ERR_OK)
                        {
                            LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                                            LSA_TRACE_LEVEL_FATAL,
                                            "erase error %d\n",
                                            CygStat);
                        }
                        OsGiveSemB(flashSemId);

                        OsTakeSemB(flashSemId);
                        OsTakeSemB(flashRamSemId);

                        CygStat= cyg_flash_program (FLASH_USERDATA_SEC1,
                                                    flashramimage,
                                                    UsrNvImageSize,
                                                    &flash_err_address);

                        OsGiveSemB(flashRamSemId);
                        OsGiveSemB(flashSemId);

                        if (CygStat != CYG_FLASH_ERR_OK)
                        {
                            LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                                            LSA_TRACE_LEVEL_FATAL,
                                            "program error %d\n",
                                            CygStat);
                        }

                        /* it is not necessary to erase second partition - keep it */

                        break;
                    }
#else
                    case FLASH_OSPI_TYPE:
                    {
                        LSA_UNUSED_ARG(flash_err_address)
                        if (flash_second_partition == PNIO_TRUE)
                        {
                            /*
                              * Prepare second partition
                              */

                             /* Erase second partition */
                             OsTakeSemB(flashSemId);
                             CygStat= FLASH_ERASE(OSPI_NV_ADDRESS_SEC2,
                                                 UsrNvImageSize,
                                                 &flash_err_address);
                             if (CygStat != PNIO_OK)
                             {
                                 LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                                                 LSA_TRACE_LEVEL_FATAL,
                                                 "erase error %d\n",
                                                 CygStat);
                             }
                             OsGiveSemB(flashSemId);

                             /* Program second partition */
                             OsTakeSemB(flashSemId);
                             CygStat= FLASH_PROGRAM(OSPI_NV_ADDRESS_SEC2,
                                                    flashramimage,
                                                    UsrNvImageSize,
                                                    &flash_err_address);
                             if (CygStat != PNIO_OK)
                             {
                                 LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                                                 LSA_TRACE_LEVEL_FATAL,
                                                 "program error %d\n",
                                                 CygStat);
                             }
                             OsGiveSemB(flashSemId);
                        }

                        /*
                         * Prepare first partition
                         */

                        /* Erase first partition */
                        OsTakeSemB(flashSemId);
                        CygStat= FLASH_ERASE(OSPI_NV_ADDRESS_SEC1,
                                            UsrNvImageSize,
                                            &flash_err_address);
                        if (CygStat != PNIO_OK)
                        {
                            LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                                            LSA_TRACE_LEVEL_FATAL,
                                            "erase error %d\n",
                                            CygStat);
                        }
                        OsGiveSemB(flashSemId);

                        /* Program first partition */
                        OsTakeSemB(flashSemId);
                        CygStat= FLASH_PROGRAM(OSPI_NV_ADDRESS_SEC1,
                                               flashramimage,
                                               UsrNvImageSize,
                                               &flash_err_address);
                        if (CygStat != PNIO_OK)
                        {
                            LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                                            LSA_TRACE_LEVEL_FATAL,
                                            "program error %d\n",
                                            CygStat);
                        }
                        OsGiveSemB(flashSemId);

                        /* it is not necessary to erase second partition - keep it */

                        break;
                    }
#endif
                    default:
                    {
                            LSA_TRACE_00   (TRACE_SUBSYS_IOD_MEMORY,
                                            LSA_TRACE_LEVEL_FATAL,
                                            "OsFlash : wrong type of NV memory\n");
                            break;
                    }
                }

                if (Status != PNIO_OK)
                {
                    LSA_TRACE_00   (TRACE_SUBSYS_IOD_MEMORY,
                                    LSA_TRACE_LEVEL_FATAL,
                                    "OsFlash : wrong type of NV memory\n");
                }
                else
                {
                    flash_callback(p2->arg, UsrNvImageSize);
                }
            }
        }

        return (PNIO_OK); /*lint !e527 Unreachable code */
    }

    // *----------------------------------------------------------------*
    // *
    // *  OsFlashInit()
    // *----------------------------------------------------------------*
    // *  must be called once before using the other flash functions
    // *
    // *----------------------------------------------------------------*
    PNIO_UINT32 OsFlashInit(PNIO_UINT32 FlashAllocSize, PNIO_UINT32 ImageSize)
    {
        PNIO_UINT32         Status	= PNIO_OK;
		PNIO_UINT32         boot_reg;
        int                 CygStat = CYG_FLASH_ERR_INVALID;

        if (FlashAllocSize > FLASH_USERDATA_SIZE)
        {
            LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                            LSA_TRACE_LEVEL_FATAL,
                            "wrong UsD%d\n",
                            FlashAllocSize);
            return (PNIO_NOT_OK);
        }

        /* We need space at least for two images */
        if (FlashAllocSize < (ImageSize * 2))
        {
            LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                            LSA_TRACE_LEVEL_FATAL,
                            "wrong UsD%d\n",
                            FlashAllocSize);
            return (PNIO_NOT_OK);
        }

        OsAllocSemB (&flashSemId);
        OsAllocSemB (&flashRamSemId);

        OsGiveSemB(flashSemId);
        OsGiveSemB(flashRamSemId);

        OsAlloc ((PNIO_VOID**)&flashramimage, 0xFF, ImageSize );

        if (flashramimage == LSA_NULL)
        {
            LSA_TRACE_00   (TRACE_SUBSYS_IOD_MEMORY,
                            LSA_TRACE_LEVEL_FATAL,
                            "OsFlash : problem with heap\n");
            return (PNIO_NOT_OK);
        }

        UsrNvImageSize = ImageSize;
        UsrNvBlockSize = FlashAllocSize;

        boot_reg = REG32(U_SCRB__BOOT_REG);
        boot_reg &= 0xf;

        /* Determine which nv memory is used */
#ifdef BOARD_TYPE_STEP_3

        FlashType = FLASH_OSPI_TYPE;
#else
        if(MODE_SPI_0x03 == boot_reg)
        {
            FlashType = FLASH_SPI_TYPE;
        }
        else if(MODE_XHIF == boot_reg)
        {
            /* It is necessary to manually select memory type
             * in case of XHIF boot mode */
            FlashType = FLASH_DEF_TYPE;
        }
        else
        {
            FlashType = FLASH_NAND_TYPE;
        }
#endif

        switch ( FlashType )
        {
#ifndef BOARD_TYPE_STEP_3
            case FLASH_SPI_TYPE:
            {
                FLASH_INIT;
                CygStat = CYG_FLASH_ERR_OK;
                break;
            }
            case FLASH_NAND_TYPE:
            {
                CygStat = cyg_flash_init( (cyg_flash_printf*) OsFlashPrintf);

                if (CygStat != CYG_FLASH_ERR_OK)
                {
                    LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                                    LSA_TRACE_LEVEL_FATAL,
                                    "flash_init error %d\n",
                                    CygStat);
                    UsrNvBlockSize = 0;
                    return (PNIO_NOT_OK);
                }

                break;
            }
#else
            case FLASH_OSPI_TYPE:
            {
                CygStat = FLASH_INIT;
                if (CygStat != PNIO_OK)
                {
                    LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                                    LSA_TRACE_LEVEL_FATAL,
                                    "flash_init error %d\n",
                                    CygStat);
                    UsrNvBlockSize = 0;
                    return (PNIO_NOT_OK);
                }
                break;
            }
#endif
            default:
                LSA_TRACE_00   (TRACE_SUBSYS_IOD_MEMORY,
                                LSA_TRACE_LEVEL_FATAL,
                                "OsFlash : wrong type of NV memory\n");
                break;
        }

        Status = OsCreateThread((PNIO_VOID(*)(PNIO_VOID))Task_OsFlashPost, 0, (PNIO_UINT8*)"Pnio_NvData", TASK_PRIO_NV_DATA, OS_TASK_DEFAULT_STACKSIZE, &TskId_Flash);
        PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: OsCreateThread\n");
        Status = OsCreateMsgQueue(TskId_Flash);   // install the task message queue
        PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: OsCreateMsgQueue\n");
        Status = OsStartThread(TskId_Flash);      // start, after the task message queue has been installed
        PNIO_APP_ASSERT(Status == PNIO_OK, "ERROR: OsStartThread\n");

        PNIO_printf("flash_init: Status = 0x%x, Type = 0x%x\n", Status, FlashType);

        return (Status);
    }

    // *----------------------------------------------------------------*
    // *
    // *  OsFlashErase
    // *----------------------------------------------------------------*
    // *  erases the affected flash sectors
    // *
    // *  Input:       FlashPageNum         image number (1 or 2)
    // *               DataSize             Size of data to erase
    // *               pError               pointer to error value
    // *
    // *  Output:      return               PNIO_OK, PNIO_NOT_OK
    // *
    // *----------------------------------------------------------------*
    PNIO_UINT32 OsFlashErase(PNIO_UINT32  FlashPageNum,
                             PNIO_UINT32 DataSize,
                             PNIO_UINT32* pError)
    {
        int             CygStat = CYG_FLASH_ERR_INVALID;
        cyg_flashaddr_t flash_err_address = 0;

        /* Double limit due to data mirroring on flash */
        if (FlashPageNum > NUM_FLASH_IMAGES)
        {
        	PNIO_printf("wrong size error Page(%d)+Len(%d) > Usd(%d)\n",
        	        FlashPageNum, DataSize, UsrNvBlockSize);
            LSA_TRACE_03   (TRACE_SUBSYS_IOD_MEMORY,
                            LSA_TRACE_LEVEL_FATAL,
                            "error Page(%d)+Len(%d) > Usd(%d)\n",
                            FlashPageNum, DataSize, UsrNvBlockSize);
            return (PNIO_NOT_OK);
        }

        *pError = FLASH_ERRNUM_OK;

        switch ( FlashType )
        {
#ifndef BOARD_TYPE_STEP_3
            case FLASH_SPI_TYPE:
            {
                OsTakeSemB(flashSemId);
                spi_in_use = 1;
		    	CygStat= FLASH_ERASE(SPI_NV_ADDRESS + FlashPageNum * SPI_NV_ADDRESS_SEC_OFFSET,
		    	         UsrNvImageSize,
						 &flash_err_address);
		    	spi_in_use = 0;
		    	OsGiveSemB(flashSemId);
		    	break;
		    }
            case FLASH_NAND_TYPE:
		    {
		        OsTakeSemB(flashSemId);
				CygStat = cyg_flash_erase(FLASH_USERDATA_START + FlashPageNum * FLASH_USERDATA_SEC_OFFSET,
				                         UsrNvImageSize,
										 &flash_err_address);
				OsGiveSemB(flashSemId);
				break;
		    }
#else
            case FLASH_OSPI_TYPE:
            {
                OsTakeSemB(flashSemId);
                CygStat= FLASH_ERASE(OSPI_NV_ADDRESS + FlashPageNum * OSPI_NV_ADDRESS_SEC_OFFSET,
		    	         UsrNvImageSize,
						 &flash_err_address);
                OsGiveSemB(flashSemId);
                if (CygStat != PNIO_OK )
                {
                    LSA_TRACE_02   (TRACE_SUBSYS_IOD_MEMORY,
                                    LSA_TRACE_LEVEL_FATAL,
                                    "flash_erase error %d at addr 0x%x\n",
                                    CygStat, flash_err_address);
                    return (PNIO_NOT_OK);
                }
            }
#endif
            default:
            {
                LSA_TRACE_00   (TRACE_SUBSYS_IOD_MEMORY,
                                LSA_TRACE_LEVEL_FATAL,
                                "OsFlash : wrong type of NV memory\n");
                break;
            }
        }

        if (CygStat != CYG_FLASH_ERR_OK )
        {
            LSA_TRACE_02   (TRACE_SUBSYS_IOD_MEMORY,
                                LSA_TRACE_LEVEL_FATAL,
                                "flash_erase error %d at addr 0x%x\n",
                                CygStat, flash_err_address);
        }

        return (PNIO_OK);
    }



    // *----------------------------------------------------------------*
    // *
    // *  OsFlashProgram
    // *----------------------------------------------------------------*
    // *  programs specified data to flash
    // *
    // *  Input:       callback             callback function pointer
    // *               arg                  pointer to callback argument
    // *               pMemSrc8             pointer to source memory
    // *               DataSize             Size of data to program
    // *               pError               pointer to error value
    // *
    // *  Output:      return               PNIO_OK, PNIO_NOT_OK
    // *
    // *----------------------------------------------------------------*
    PNIO_UINT32 OsFlashProgram( PNIO_VOID (*callback)( PNIO_VOID*, PNIO_UINT32 ),
                               PNIO_VOID*   arg,
                               PNIO_UINT8*  pMemSrc8,
                               PNIO_UINT32  DataSize,
                               PNIO_UINT32* pError)
    {
        PNIO_UINT32      Status = PNIO_OK;
        FLASH_CBF_STR*   queueItem;

        if ( ( callback == LSA_NULL ) || ( pMemSrc8 == LSA_NULL ) )
        {
            LSA_TRACE_00  (TRACE_SUBSYS_IOD_MEMORY, LSA_TRACE_LEVEL_ERROR, "NV Data Store failed \n");
            return (PNIO_NOT_OK);
        }

        if ( DataSize == 0)
        {
            LSA_TRACE_00  (TRACE_SUBSYS_IOD_MEMORY, LSA_TRACE_LEVEL_ERROR, "NV Data Store failed \n");
            return (PNIO_NOT_OK);
        }

        /* Update image */
        OsTakeSemB(flashRamSemId);

        OsMemCpy (flashramimage, pMemSrc8, DataSize);

        OsGiveSemB(flashRamSemId);

        /* Create queue item to be passed to flash thread */
        OsAlloc ((PNIO_VOID**)&queueItem, 0x0, sizeof(FLASH_CBF_STR) );
        queueItem->arg = arg;
        queueItem->callback = callback;

        Status = OsSendMessage  (TskId_Flash,                   // destination task id
                                 (PNIO_VOID*) queueItem,        // pointer to source
                                 OS_MBX_PRIO_NORM);             // message priority

        if (Status == PNIO_NOT_OK)
        {
            LSA_TRACE_00  (TRACE_SUBSYS_IOD_MEMORY, LSA_TRACE_LEVEL_ERROR, "NV Data Store failed because of error OsSendMessage (mailbox full ?)\n");
            return (PNIO_NOT_OK);
        }

        return (PNIO_OK);
     }

    // *----------------------------------------------------------------*
    // *
    // *  OsFlashRead
    // *----------------------------------------------------------------*
    // *  read specified part of data from flash  ram-cache.
    // *
    // *  Input:       FlashPageNum         image number (1 or 2)
    // *               pMemDst              pointer to destination memory
    // *               DataSize             Size of data to read
    // *               pError               pointer to error value
    // *
    // *  Output:      return               PNIO_OK, PNIO_NOT_OK
    // *
    // *----------------------------------------------------------------*
    PNIO_UINT32 OsFlashRead   (PNIO_UINT32  FlashPageNum,
                               PNIO_UINT8*  pMemDst8,
                               PNIO_UINT32  DataSize,
                               PNIO_UINT32* pError)
    {
        int             CygStat = CYG_FLASH_ERR_INVALID;
        cyg_flashaddr_t flash_err_address = 0;

        /* Double limit due to data mirroring on flash */
        if (FlashPageNum > NUM_FLASH_IMAGES)
        {
            LSA_TRACE_03   (TRACE_SUBSYS_IOD_MEMORY,
                            LSA_TRACE_LEVEL_FATAL,
                            "OsFlashRead error Pafe%d Dat%d UsD%d\n",
                            FlashPageNum,
                            DataSize,
                            UsrNvImageSize);
            return (PNIO_NOT_OK);
        }

        switch (FlashType)
        {
#ifndef BOARD_TYPE_STEP_3
            case FLASH_SPI_TYPE:
            {
                OsTakeSemB(flashSemId);
                spi_in_use = 1;
                CygStat= FLASH_READ(SPI_NV_ADDRESS + FlashPageNum * SPI_NV_ADDRESS_SEC_OFFSET,
                                pMemDst8,
                                DataSize,
                                &flash_err_address);
                spi_in_use = 0;
                OsGiveSemB(flashSemId);

                if (CygStat == 0 )	/* FLASH_READ returns number of read bytes */
                {
                    LSA_TRACE_01   (TRACE_SUBSYS_IOD_MEMORY,
                                    LSA_TRACE_LEVEL_FATAL,
                                    "flash_read error at addr 0x%x\n",
                                    flash_err_address);
                    return (PNIO_NOT_OK);
                }
                break;
            }
            case FLASH_NAND_TYPE:
            {
                OsTakeSemB(flashSemId);
                CygStat= cyg_flash_read(FLASH_USERDATA_START + FlashPageNum * FLASH_USERDATA_SEC_OFFSET,
                                        pMemDst8,
                                        DataSize,
                                        &flash_err_address);
                OsGiveSemB(flashSemId);

                if (CygStat != CYG_FLASH_ERR_OK )
                {
                    LSA_TRACE_02   (TRACE_SUBSYS_IOD_MEMORY,
                                    LSA_TRACE_LEVEL_FATAL,
                                    "flash_read error %d at addr 0x%x\n",
                                    CygStat, flash_err_address);
                    return (PNIO_NOT_OK);
                }
                break;
            }
#else
            case FLASH_OSPI_TYPE:
            {
                OsTakeSemB(flashSemId);
                CygStat= FLASH_READ(OSPI_NV_ADDRESS + FlashPageNum * OSPI_NV_ADDRESS_SEC_OFFSET,
                                pMemDst8,
                                DataSize,
                                &flash_err_address);
                OsGiveSemB(flashSemId);

                if (CygStat != PNIO_OK )
                {
                    LSA_TRACE_02   (TRACE_SUBSYS_IOD_MEMORY,
                                    LSA_TRACE_LEVEL_FATAL,
                                    "flash_read error %d at addr 0x%x\n",
                                    CygStat, flash_err_address);
                    return (PNIO_NOT_OK);
                }
                
            }
            default:
            {
                LSA_TRACE_00   (TRACE_SUBSYS_IOD_MEMORY,
                                LSA_TRACE_LEVEL_FATAL,
                                "OsFlash : wrong type of NV memory\n");
                break;
            }
#endif
        }

        *pError = FLASH_ERRNUM_OK;

        LSA_TRACE_03   (TRACE_SUBSYS_IOD_MEMORY,
                        LSA_TRACE_LEVEL_NOTE_HIGH,
                        "OsFlashRead (%d, 0x%x, %d) done, OK\n",
                        FlashPageNum, pMemDst8, DataSize);
        return (PNIO_OK);
    }

    PNIO_UINT32 flash_verify	( PNIO_CHAR * pFlash, PNIO_CHAR * pBuf, int BufSize )
    {
    	int i;

    	for ( i = 0; i < BufSize; i++ )
		{
			if ( *( pFlash + i ) != *( pBuf+i ) )
			{
				PNIO_ConsolePrintf( "\nERROR flash program at offset 0x%x: value = 0x%x (0x%x) \n",
							 i, *( pFlash+i ), * (pBuf+i ) );
				return ( PNIO_NOT_OK );
			}
		}
    	return PNIO_OK;
    }


    PNIO_UINT32 flash_erase_verify	( PNIO_CHAR * pFlash, int BufSize )
    {
    	int i;

    	for ( i = 0; i < BufSize; i++ )
		{
			if ( *( pFlash + i ) != 0xff )
			{
				PNIO_ConsolePrintf( "ERROR flash is not erased at offset 0x%x\n", i );
				return ( PNIO_NOT_OK );
			}
		}
    	return PNIO_OK;
    }

#endif

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
