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
/*  F i l e               &F: flash_ospi_read.c                         :F&  */
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
#ifdef BOARD_TYPE_STEP_3

#include "lsa_cfg.h"
#include "pnio_types.h"
#include "os.h"

#include "flash_ospi.h"
#include "arm926.h"
#include "ertec200p_reg.h"

#define RSC_BYTES 16


/**
 * @brief read data from flash
 *
 * @param info		pointer to FLASH info structure
 * @param mem_addr	byte address in flash memory
 * @param dest		virtual destination address
 * @param len		number of bytes to read
 *
 * @retval 0		OK
 * @retval PNIO_FLASH_ADDR_ERR	mem_addr outside PFU address region
 * @retval PNIO_FLASH_SIZE_ERR,	mem_addr+len outside PFU address region
 * @retval PNIO_FLASH_BUSY_ERR	erase in progress
 */
PNIO_UINT32 flash_ospi_read(const struct flash_ospi_info * info, PNIO_UINT32 mem_addr, PNIO_VOID * dest,
		size_t len)
{
	size_t pfu_size;

	/* fail if erase is in progress */
	if (info->erase_in_progress) {
		PNIO_ConsolePrintf("ERROR: erase in progress.\n");
		return PNIO_FLASH_BUSY_ERR;
	}

	/* check addr/len in SPI-PFU window */
	pfu_size = U_OCT_SPI_ACCESS__end - U_OCT_SPI_ACCESS__start + 1;
	if (mem_addr >= pfu_size) {
		PNIO_ConsolePrintf("ERROR: mem_addr outside PFU address region.\n");
		return PNIO_FLASH_ADDR_ERR;
	} else if (mem_addr + len > pfu_size) {
		PNIO_ConsolePrintf("ERROR: mem_addr+len outside PFU address region.\n");
		return PNIO_FLASH_SIZE_ERR;
	}

	/* already in read mode */
	OsMemCpy(dest, (PNIO_VOID *) ((PNIO_UINT32)U_OCT_SPI_ACCESS__start + mem_addr), len);

	return 0;
}

/**
 * @brief read data from flash and decode RSC
 *
 * @param info		pointer to FLASH info structure
 * @param mem_addr	byte address in flash memory
 * @param dest		virtual destination address
 * @param rsc_page_size	size of RSC page (max. 238)
 * @param npages	number of pages to read
 *
 * @retval 0		OK
 * @retval PNIO_FLASH_ADDR_ERR	mem_addr outside PFU address region
 * @retval PNIO_FLASH_SIZE_ERR,	mem_addr + read size outside PFU address region
 * @retval PNIO_FLASH_PARAM_ERR 	illegal value for rsc_page_size
 * @retval PNIO_FLASH_BUSY_ERR	erase in progress
 * @retval others >0	maximum number of RSC errors in any page
 *
 * @note When reading multiple pages, they are read from the flash without gap, i.e.
 *       with rsc_page_size=236, the start address of the second page is 236+16=252 bytes
 *       after the start address of the first page. (16 bytes checksum)
 *
 * @note For highest performance, mem_addr should be aligned to 4 bytes, and
 *       rsc_page_size should be a multiple of 4.
 */
PNIO_UINT32 flash_ospi_read_rsc(flash_ospi_info_t * info, PNIO_UINT32 mem_addr, PNIO_VOID * dest,
		PNIO_UINT32 rsc_page_size, size_t npages)
{
	size_t pfu_size;
	PNIO_UINT32 max_errors;
	PNIO_UINT32 rsc_srcmode;

	/* fail if erase is in progress */
	if (info->erase_in_progress) {
		PNIO_ConsolePrintf("ERROR: erase in progress.\n");
		return PNIO_FLASH_BUSY_ERR;
	}

	/* check addr/len in SPI-PFU window */
	pfu_size = U_OCT_SPI_ACCESS__end - U_OCT_SPI_ACCESS__start + 1;
	if (mem_addr >= pfu_size) {
		PNIO_ConsolePrintf("ERROR: mem_addr outside PFU address region.\n");
		return PNIO_FLASH_ADDR_ERR;
	} else if (mem_addr + npages * (rsc_page_size + RSC_BYTES) > pfu_size) {
		PNIO_ConsolePrintf("ERROR: mem_addr + read size outside PFU address region.\n");
		return PNIO_FLASH_SIZE_ERR;
	}

	if (rsc_page_size == 0 || rsc_page_size > 238) {
		PNIO_ConsolePrintf(" ERROR: illegal value for rsc_page_size.\n");
		return PNIO_FLASH_PARAM_ERR ;
	}

	/* just in case a previous job is still running */
	if (REG32(U_RS_CONTROLLER1__DEC_Status)
			& (PNIO_UINT32)U_RS_CONTROLLER1__DEC_Status__run) {
		PNIO_ConsolePrintf("ERROR: previous job is still running.\n");
		return PNIO_FLASH_BUSY_ERR;
	}

	/* setup RS controller */
	if ((mem_addr & 0x3) == 0 && (rsc_page_size & 0x3) == 0) {
		rsc_srcmode = 0x8; // word access
	} else if ((mem_addr & 0x1) == 0 && (rsc_page_size & 0x1) == 0) {
		rsc_srcmode = 0x4; // halfword access
	} else {
		rsc_srcmode = 0x0; // byte access
	}
	(rsc_srcmode |= (PNIO_UINT32)U_RS_CONTROLLER1__DEC_SRCmode__incr_addr);
	REG32(U_RS_CONTROLLER1__DEC_SRCmode) = rsc_srcmode;
	REG32(U_RS_CONTROLLER1__DEC_Length) = rsc_page_size;
	REG32(U_RS_CONTROLLER1__DEC_SRCaddr) = ((PNIO_UINT32)U_OCT_SPI_ACCESS__start + mem_addr);

	/* clear error status */
	REG32(U_RS_CONTROLLER1__DEC_Status) =
		U_RS_CONTROLLER1__DEC_Status__correction
		| U_RS_CONTROLLER1__DEC_Status__corr_count;

	while (npages--) {
		/* start decoding */
		REG32(U_RS_CONTROLLER1__DEC_Status)
			= U_RS_CONTROLLER1__DEC_Status__run;

		/* wait until decoder is finished */
		while (REG32(U_RS_CONTROLLER1__DEC_Status)
				& (PNIO_UINT32)U_RS_CONTROLLER1__DEC_Status__run)
			;

		/* copy to destination */
		OsMemCpy(dest, (PNIO_VOID*) U_RS_CONTROLLER1_RSC_RAM__start, rsc_page_size);
		dest = (PNIO_UINT8*) dest + rsc_page_size;
	}

	/* read decoder status */
	max_errors = (REG32(U_RS_CONTROLLER1__DEC_Status) >> 4) & 0xf;

	return max_errors;
}

#define U_RS_CONTROLLERn__DEC_SRCaddr(n) ((n) == 0 ? U_RS_CONTROLLER1__DEC_SRCaddr : U_RS_CONTROLLER2__DEC_SRCaddr)
#define U_RS_CONTROLLERn__DEC_Status(n) ((n) == 0 ? U_RS_CONTROLLER1__DEC_Status : U_RS_CONTROLLER2__DEC_Status)


#endif // BOARD_TYPE_STEP_3

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/