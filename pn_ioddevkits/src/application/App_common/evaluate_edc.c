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
/*  F i l e               &F: evaluate_edc.c                            :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  Detect and correct bit errors during memory access                       */
/*  (1-bit error, correctable; 2-bit error, detectable).                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  H i s t o r y :                                                          */
/*                                                                           */
/*  Date        Version        Who  What                                     */
/*                                                                           */
/*****************************************************************************/

#ifdef BOARD_TYPE_STEP_3

#include "compiler.h"
#include "pniousrd.h"
#include "os.h"
#include "iodapi_event.h"
#include "evaluate_edc.h"

extern PNIO_UINT32 __itcm_flash_strt;

typedef struct
{
	PNIO_UINT32 edc_1bit_error_count;
	PNIO_UINT32 edc_1bit_error_source;
	PNIO_UINT32 edc_fatal_error_source;
} EDC_DATA;

extern EDC_DATA edc_data;

PNIO_BOOL irreparable_ecc_problem_present(PNIO_UINT32 *ecc_add_value)
{
    PNIO_UINT32 edc_event = *((volatile PNIO_UINT32*) PN_SUB_PN_SCRB__EDC_EVENT);

    edc_event &= (  PN_SUB_PN_SCRB__EDC_EVENT__I_TCM926_2B
                    | PN_SUB_PN_SCRB__EDC_EVENT__D_TCM926_2B
                    | PN_SUB_PN_SCRB__EDC_EVENT__GDMA_2B
                    | PN_SUB_PN_SCRB__EDC_EVENT__PN_2B
                    | PN_SUB_PN_SCRB__EDC_EVENT__PERIF_2B
                    | PN_SUB_PN_SCRB__EDC_EVENT__D_CACHE_PAR
                    | PN_SUB_PN_SCRB__EDC_EVENT__D_TAG_PAR
                    | PN_SUB_PN_SCRB__EDC_EVENT__I_CACHE_PAR
                    | PN_SUB_PN_SCRB__EDC_EVENT__I_TAG_PAR
                    | PN_SUB_PN_SCRB__EDC_EVENT__I_TCM966_2B
                    | PN_SUB_PN_SCRB__EDC_EVENT__D_TCM966_2B
                    | PN_SUB_PN_SCRB__EDC_EVENT__SPI_PFU_2B
                    | PN_SUB_PN_SCRB__EDC_EVENT__SRAM_2B
                    | PN_SUB_PN_SCRB__EDC_EVENT__I_TCM926_2B_RFSH
                    | PN_SUB_PN_SCRB__EDC_EVENT__D_TCM926_2B_RFSH
                    | PN_SUB_PN_SCRB__EDC_EVENT__PN_2B_RFSH
                    | PN_SUB_PN_SCRB__EDC_EVENT__I_TCM966_2B_RFSH
                    | PN_SUB_PN_SCRB__EDC_EVENT__D_TCM966_2B_RFSH
                    | PN_SUB_PN_SCRB__EDC_EVENT__SRAM_2B_RFSH

                   );

    if(edc_event & (PN_SUB_PN_SCRB__EDC_EVENT__I_CACHE_PAR | PN_SUB_PN_SCRB__EDC_EVENT__I_TAG_PAR))
    {
        /* Instruction cache is corrupted -> throw all content away and reload from SDRAM if needed */
        __asm("STMDB sp!, { r0 }");
        __asm("MOV r0, #0");
        __asm("MCR p15, 0, r0, c7, c5, 0");           // invalidate I-Cache
        __asm("LDMIA sp!, { r0 }");
    }
    if(edc_event & PN_SUB_PN_SCRB__EDC_EVENT__I_TCM926_2B)
    {
        /* ITCM corrupted -> Disable TCM region*/
    	PNIO_UINT32 value_x ;
        value_x = 0;               // reset value
        asm("mcr p15, 0, %0, c9, c1, 1"::"r"(value_x));  // write r0 to data TCM Region Register
    }


    if(ecc_add_value)
    {
        *ecc_add_value = edc_event;
    }

    return edc_event != 0;
}

static PNIO_BOOL evaluate_edc_refresh_pallas(PNIO_UINT32 *erroneous_mem_addr, PNIO_UINT32 *add_info, PNIO_BOOL *was_writeback_possible)
{
    PNIO_BOOL new_error_detected = PNIO_FALSE;
    PNIO_UINT32 edc_status      = *((volatile PNIO_UINT32*) PN_SUB_PN_SCRB2__EDC_STAT_REG);

    if(edc_status & (  PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_I_TCM926
                               | PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_D_TCM926
                               | PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_PNIP
                               | PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_I_TCM966
                               | PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_D_TCM966
                               | PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_SRAM
        ))
    {
        new_error_detected      = PNIO_TRUE;

        *add_info               = edc_status;
        *was_writeback_possible = PNIO_TRUE;
        ++edc_data.edc_1bit_error_count;

        /* if a cyclic refresh finds a 1B-error, then !NO! 1B-error will be marked inside the EDC_EVENT-register! */
        if(edc_status & PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_I_TCM926)
            *erroneous_mem_addr = __itcm_flash_strt;                                 // address of ITCM from linker file
        else if(edc_status & PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_D_TCM926)
            *erroneous_mem_addr = __itcm_flash_strt;                                 // address of DTCM from linker file
        else if(edc_status & PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_PNIP)
            *erroneous_mem_addr = 0x10600000;
        else if(edc_status & PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_I_TCM966)
            *erroneous_mem_addr = 0x04000000;
        else if(edc_status & PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_D_TCM966)
            *erroneous_mem_addr = 0x04000000;
        else if(edc_status & PN_SUB_PN_SCRB2__EDC_STAT_REG__EDC_CORR_SRAM)
            *erroneous_mem_addr = PN_SUB_AHB_SRAM__start;
        /* inline-EDC: not handled here. See hama_pallas_mem_evaluate_edc_errors() */
        else
            *erroneous_mem_addr = 0xFFFFFFFF;

        // acknowledge correction event
        *((volatile PNIO_UINT32*) PN_SUB_PN_SCRB2__EDC_STAT_REG) = edc_status;
    }

    return new_error_detected;
}

PNIO_BOOL evaluate_edc_errors()
{
    PNIO_UINT32 *erroneous_mem_addr = 0;
    PNIO_BOOL *was_writeback_possible = 0;
    PNIO_UINT32 *add_info = 0;

    while(1)
    {

        OsWait_ms(500);
        PNIO_UINT32 edc_event = *((volatile PNIO_UINT32*) PN_SUB_PN_SCRB__EDC_EVENT);
        PNIO_BOOL new_error_detected = PNIO_FALSE;
        if(irreparable_ecc_problem_present(NULL))
        {
            /* EDC error that can't be corrected -> fatal error */
            new_error_detected = PNIO_TRUE;
            edc_data.edc_fatal_error_source = edc_event;
            PNIO_ConsolePrintf("== ERROR : irreparable ecc problem present==\n");
            PNIO_Fatal(  );
        }
        else if(edc_event & (  PN_SUB_PN_SCRB__EDC_EVENT__I_TCM926_1B
                                | PN_SUB_PN_SCRB__EDC_EVENT__D_TCM926_1B
                                | PN_SUB_PN_SCRB__EDC_EVENT__GDMA_1B
                                | PN_SUB_PN_SCRB__EDC_EVENT__PN_1B
                                | PN_SUB_PN_SCRB__EDC_EVENT__PERIF_1B
                                | PN_SUB_PN_SCRB__EDC_EVENT__I_TCM966_1B
                                | PN_SUB_PN_SCRB__EDC_EVENT__D_TCM966_1B
                                | PN_SUB_PN_SCRB__EDC_EVENT__SPI_PFU_1B
                                | PN_SUB_PN_SCRB__EDC_EVENT__SRAM_1B
                           ))
        {

        /* one bit error that has been corrected by HW (no writeback) -> reset flag and continue running */

            PNIO_UINT32 is_repairable_by_refresh_on_pallas =  PN_SUB_PN_SCRB__EDC_EVENT__I_TCM926_1B
                                                               | PN_SUB_PN_SCRB__EDC_EVENT__D_TCM926_1B
                                                               | PN_SUB_PN_SCRB__EDC_EVENT__PN_1B
                                                               | PN_SUB_PN_SCRB__EDC_EVENT__I_TCM966_1B
                                                               | PN_SUB_PN_SCRB__EDC_EVENT__D_TCM966_1B
                                                               | PN_SUB_PN_SCRB__EDC_EVENT__SRAM_1B;


           if(!(edc_event & is_repairable_by_refresh_on_pallas))

           {
        	   /* Code section : EDC error that can be corrected -> if new error detected then inform user. */
                new_error_detected      = PNIO_TRUE;
                *add_info               = edc_event;
                *was_writeback_possible = PNIO_FALSE;
                 ++edc_data.edc_1bit_error_count;

                if(edc_event & PN_SUB_PN_SCRB__EDC_EVENT__I_TCM926_1B){
                     *erroneous_mem_addr = __itcm_flash_strt;   // address of ITCM from linker file
                }
                else if(edc_event & PN_SUB_PN_SCRB__EDC_EVENT__D_TCM926_1B){
                    *erroneous_mem_addr = __itcm_flash_strt;   // address of DTCM from linker file
                }
                else if(edc_event & PN_SUB_PN_SCRB__EDC_EVENT__GDMA_1B)
                   *erroneous_mem_addr = 0x10A00000;
                else if(edc_event & PN_SUB_PN_SCRB__EDC_EVENT__PN_1B)
                   *erroneous_mem_addr = 0x10600000;
                else if(edc_event & PN_SUB_PN_SCRB__EDC_EVENT__PERIF_1B)
                    *erroneous_mem_addr = 0x40000000;
                else if(edc_event & PN_SUB_PN_SCRB__EDC_EVENT__I_TCM966_1B)
                    *erroneous_mem_addr = 0x04000000;
                else if(edc_event & PN_SUB_PN_SCRB__EDC_EVENT__D_TCM966_1B)
                    *erroneous_mem_addr = 0x04000000;
                else if(edc_event & PN_SUB_PN_SCRB__EDC_EVENT__SPI_PFU_1B)
                    *erroneous_mem_addr = PN_SUB_OSPI_SPFU_PREFETCH_RAM__start;
                else if(edc_event & PN_SUB_PN_SCRB__EDC_EVENT__SRAM_1B)
                    *erroneous_mem_addr = PN_SUB_AHB_SRAM__start;
                else{
                    *erroneous_mem_addr = 0xFFFFFFFF;
                 }

                edc_data.edc_1bit_error_source = edc_event;

           }

           *((volatile PNIO_UINT32*) PN_SUB_PN_SCRB__EDC_EVENT) = 0;

        }

        else
        {
            /* EDC-Refresher on Ertec-side */
            new_error_detected = evaluate_edc_refresh_pallas(erroneous_mem_addr, add_info, was_writeback_possible);
        }


        if(new_error_detected == PNIO_TRUE)
        {
        	PNIO_ConsolePrintf("new_error_detected : %d\t edc_fatal_error_source = %d\t  edc_1bit_error_count = %d\n",
                     new_error_detected, edc_data.edc_fatal_error_source, edc_data.edc_1bit_error_count );
        }

    	}
    }
#endif
/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/

