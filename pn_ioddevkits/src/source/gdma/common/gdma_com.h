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
/*  F i l e               &F: gdma_com.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  header file containing public definitions for the                        */
/*  software module handling the ERTEC200+ / Triton2 GDMA module             */
/*                                                                           */
/*****************************************************************************/


#ifndef GDMA_COM_H
#define GDMA_COM_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

/** @name return codes
 *  return values of GDMA functions
 *  @{*/
#define GDMA_OK             0
#define GDMA_ERR_MEM        -1
#define GDMA_ERR_PARAMETER  -2
#define GDMA_ERR_INTERNAL   -3
/** @}*/

/** Enum, defining the size of a single transfer element.
 *  @see gdma_add_job
 *  @see gdma_transfer_descr_t
 */
typedef enum
{
    GDMA_ESIZE_8BIT,
    GDMA_ESIZE_16BIT,
    GDMA_ESIZE_32BIT
}gdma_element_size_t;

typedef enum	//add suzhen 20230810
{
	GDMA_ADDRESS_MODE_INCREMENT,
	GDMA_ADDRESS_MODE_HOLD,
}gdma_address_mode_t;

typedef enum
{
    GDMA_BURST_MODE_SINGLE,
    GDMA_BURST_MODE_INCR4,
    GDMA_BURST_MODE_INCR8,
    GDMA_BURST_MODE_INCR16,
}gdma_burst_mode_t;

#ifdef GDMA_CFG_HW_BASE_ERTEC200P
    /** Enum, defining all the different GDMA trigger types on the Ertec200p hardware (software / hardware)
     *  @see gdma_add_job
     *  @see gdma_job_descriptor_t
     */
    typedef enum
    {
        GDMA_TRIGGER_HW_PNPLL_OUT11,    /**< HW_JOB_START_0 */
        GDMA_TRIGGER_HW_PNPLL_OUT12,
        GDMA_TRIGGER_HW_PNPLL_OUT13,
        GDMA_TRIGGER_HW_PNPLL_OUT14,
        GDMA_TRIGGER_HW_PNPLL_OUT15,
        GDMA_TRIGGER_HW_PNPLL_OUT16,    /**< HW_JOB_START_5 */
        GDMA_TRIGGER_HW_PNPLL_OUT17,
        GDMA_TRIGGER_HW_PNPLL_OUT18,
        GDMA_TRIGGER_HW_PNPLL_OUT19,
        GDMA_TRIGGER_HW_PNPLL_OUT20,
        GDMA_TRIGGER_HW_TIM_OUT0,      /**< Timer 0 in timer unit, HW_JOB_START_10 */
        GDMA_TRIGGER_HW_TIM_OUT1,      /**< Timer 1 in timer unit */
        GDMA_TRIGGER_HW_TIM_OUT2,      /**< Timer 2 in timer unit */
        GDMA_TRIGGER_HW_TIM_OUT3,      /**< Timer 3 in timer unit */
        GDMA_TRIGGER_HW_TIM_OUT4,      /**< Timer 4 in timer unit */
        GDMA_TRIGGER_HW_TIM_OUT5,      /**< Timer 5 in timer unit */
        GDMA_TRIGGER_HW_GPIO0_INT,     /**< GPIO0 input pin, HW_JOB_START 16 */
        GDMA_TRIGGER_HW_GPIO1_INT,     /**< GPIO1 input pin */
        GDMA_TRIGGER_HW_GPIO2_INT,     /**< GPIO2 input pin */
        GDMA_TRIGGER_HW_GPIO3_INT,     /**< GPIO3 input pin */
        GDMA_TRIGGER_SW                /**< software trigger (no hardware triggering possible). This type is expected to be at the end of the list */
    }gdma_trigger_src_t;
#endif

#ifdef GDMA_CFG_HW_BASE_TRITON2
    typedef enum
    {
        GDMA_TRIGGER_HW_ISO_COMP0,      /**< HW_JOB_START_0 */
        GDMA_TRIGGER_HW_ISO_COMP1,
        GDMA_TRIGGER_HW_ISO_COMP2,
        GDMA_TRIGGER_HW_TIM_OUT0,       /**< Timer 0 in timer unit, HW_JOB_START_3 */
        GDMA_TRIGGER_HW_TIM_OUT1,       /**< Timer 1 in timer unit */
        GDMA_TRIGGER_HW_TIM_OUT2,       /**< Timer 2 in timer unit */
        GDMA_TRIGGER_HW_GPIO0_INT,      /**< GPIO input pin, HW_JOB_START_6 */
        GDMA_TRIGGER_HW_GPIO1_INT,      /**< GPIO input pin */
        GDMA_TRIGGER_HW_GPIO2_INT,      /**< GPIO input pin */
        GDMA_TRIGGER_HW_GPIO3_INT,      /**< GPIO input pin */
        GDMA_TRIGGER_SW                 /**< software trigger (no hardware triggering possible). This is expected to be at the end of the list */
    }gdma_trigger_src_t;
#endif


/** use this macro to initialize a structure of type gdma_transfer_descr_t.
 *  @see gdma_add_job
 *  @see gdma_transfer_descr_t
 */
#define GDMA_SET_TRANSFER_DESCR(trans_mem_ptr, trans_no, source_addr, destination_addr, num_elements, element_size, burst_mode_param)\
{                                                                                                                                    \
    ((gdma_transfer_descr_t*) trans_mem_ptr)[trans_no].magic_number   = GDMA_TRANSFER_DESCR_MAGIC;                                   \
    ((gdma_transfer_descr_t*) trans_mem_ptr)[trans_no].src_addr       = (LSA_UINT32*) (source_addr);                                 \
    ((gdma_transfer_descr_t*) trans_mem_ptr)[trans_no].dst_addr       = (LSA_UINT32*) (destination_addr);                            \
    ((gdma_transfer_descr_t*) trans_mem_ptr)[trans_no].transfer_count = num_elements;                                                \
    ((gdma_transfer_descr_t*) trans_mem_ptr)[trans_no].elem_size      = element_size;                                                \
    ((gdma_transfer_descr_t*) trans_mem_ptr)[trans_no].burst_mode     = burst_mode_param;                                            \
}

#define GDMA_TRANSFER_DESCR_MAGIC 0x5a5a5a5a    /**< magic number that has to be set in a transfer descriptor to ensure the validity of the descriptor*/
/** This structure defines one specific transfer that is part of a job.
 *  An array of gdma_transfer_descr_t structs is referenced by the job descriptor structure.
 *  @see gdma_add_job
 *  @see gdma_job_descriptor_t
 */
typedef struct
{
    LSA_UINT32          magic_number;       /**< prevents errors at parameter passing (num_transfers / transfers - fields of struct GDMA_JOB_DESCRIPTOR) */
    LSA_UINT32          *src_addr;          /**< source address for DMA transfer */
    LSA_UINT32          *dst_addr;          /**< destination address for DMA transfer */
    LSA_UINT16          transfer_count;     /**< number of elements to be transferred */
    gdma_element_size_t    elem_size;       /**< size of one element -> see enum (GDMA_ESIZE_8BIT for example) */
    gdma_burst_mode_t      burst_mode;
    gdma_address_mode_t source_address_mode; //add suzhen 20230810
    gdma_address_mode_t destination_address_mode; //add suzhen 20230810
}gdma_transfer_descr_t;

#ifdef GDMA_CFG_HW_BASE_ERTEC200P
    /** Enum that defines the hardware flow control type to be used for a job that is to be created.
     *  This enum is specific to the Ertec200p hardware.
     *  @see gdma_add_job
     *  @see gdma_job_descriptor_t
     */
    typedef enum gdma_hw_flow_src_e
    {
        GDMA_UART1_TX_HALF_FULL,
        GDMA_UART1_RX_NOT_EMPTY,
        GDMA_UART2_TX_HALF_FULL,
        GDMA_UART2_RX_NOT_EMPTY,
        GDMA_UART3_TX_HALF_FULL,
        GDMA_UART3_RX_NOT_EMPTY,
        GDMA_UART4_TX_HALF_FULL,
        GDMA_UART4_RX_NOT_EMPTY,
        GDMA_SPI1_SSPRXDMA,                 /**< rx fifo not empty */
        GDMA_SPI1_SSPTRINTR,                /**< tx fifo not half full */
        GDMA_SPI1_SSPTX_DELAYED_REQUEST,    /**< spi1 transmit delayed - elapsing of timer 4 sets this bit */
        GDMA_SPI2_SSPRXDMA,
        GDMA_SPI2_SSPTRINTR,
        GDMA_SPI2_SSPTX_DELAYED_REQUEST,
        GDMA_REQ_OUT,
        GDMA_HW_FLOW_OFF                    /**< HW flow disabled. This type is expected to be at the end of the list */
    }gdma_hw_flow_src_t;
#endif
#ifdef GDMA_CFG_HW_BASE_TRITON2
    typedef enum gdma_hw_flow_src_e
    {
        GDMA_SPI_SSPRXDMA,                  /**< rx fifo not empty */
        GDMA_SPI_SSPTFE,                    /**< tx fifo empty */
        GDMA_HW_FLOW_OFF                    /**< HW flow disabled */
    }gdma_hw_flow_src_t;
#endif

/** Structure that is handed over to gdma_add_job in order to define
 *  the specifics of the DMA job to be created.
 */
typedef struct
{
    LSA_UINT16             num_transfers; /**< size of the array / structure field "transfers" */
    gdma_transfer_descr_t* transfers;     /**< pointer to array of GDMA_TRANSFER_DESCR-structs describing all transfers of a job */
    gdma_trigger_src_t     trigger;       /**< trigger source of DMA-job : hardware or software possible. See enum */
    LSA_UINT8              priority;      /**< priority of job : 0 - 31 */
    gdma_hw_flow_src_t     hw_flow_src;   /**< hw_dma_req signal to use */
    LSA_VOID (*job_done_callback)(LSA_INT16, LSA_UINT32);   /**< job done callback (IRQ) for this job. If set to LSA_NULL -> interrupt disabled
                                                              The parameters are: 1. job handle, 2. additional user defined callback argument (see field cbf_arg below) */
    LSA_UINT32             cbf_arg;       /**< user defined additional argument, that is passed to the job_done_callback. */
}gdma_job_descriptor_t;

/**
 * Use this structure as argument to the gdma_add_transfers_to_job() function. This enables the user
 * to add transfers to an existing job (allocated with gdma_add_job() before). The transfers are
 * appended (-> this works only if no other job reserved the transfer list memory there already)
 */
typedef struct
{
    LSA_UINT16             num_transfers; /**< size of the array / structure field "transfers" */
    gdma_transfer_descr_t* transfers;     /**< pointer to array of GDMA_TRANSFER_DESCR-structs, describing all additional transfers of a job */
}gdma_add_transfers_descriptor_t;

typedef LSA_VERSION_TYPE  * gdma_version_info_ptr_t;

/*
 *  public functions
 */
LSA_VOID  gdma_init(LSA_VOID);
LSA_BOOL  gdma_is_job_allocated(LSA_INT16 job_num);
LSA_INT16 gdma_add_job(LSA_UINT8 job_pool_index, gdma_job_descriptor_t* job_desc);
LSA_INT16 gdma_add_transfers_to_job(LSA_INT16 job_num, gdma_add_transfers_descriptor_t *add_transfers_desc);
LSA_INT16 gdma_remove_job(LSA_INT16 job_num);
LSA_INT16 gdma_trigger_job(LSA_INT16 job_num);
LSA_VOID  gdma_disable_all_jobs(LSA_VOID);
LSA_INT16 gdma_enable_job(LSA_INT16 job_num);
LSA_INT16 gdma_disable_job(LSA_INT16 job_num);
LSA_INT16 gdma_switch_to_job_pool(LSA_UINT8 job_pool_index);

LSA_UINT16 gdma_version(LSA_UINT16 version_len, gdma_version_info_ptr_t version_ptr);

/*
 * useful macros
 */
#define GDMA_IS_VALID_JOB(x)          gdma_is_job_allocated(x)                 /**<    returns TRUE if the value of x represents a valid job index
                                                                                    (-> the job was successfully allocated by gdma_add_job()) */

#define GDMA_IS_VALID_JOB_IDX(x)      ((0 <= (x)) && ((x) < 32 * GDMA_CFG_NUM_JOB_LISTS))   /**< returns TRUE, if the job index is in valid bounds */

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
}
#endif

#endif      //GDMA_COM_H

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
