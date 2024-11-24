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
/*  F i l e               &F: iom_com.h                                 :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  component header for the IO Manager software module (PERIF)              */
/*                                                                           */
/*****************************************************************************/


#ifndef IOM_COM_H_
#define IOM_COM_H_

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif

#if defined IOM_CFG_PNIP
    /*
     * PERIF specific declarations
     */

    /** initialization parameters
     * @see iom_init
     * @see iom_perif_init
     * */
    typedef struct
    {
        EDD_COMMON_MEM_PTR_TYPE pnip_base_address;
        EDD_COMMON_MEM_PTR_TYPE gsharedmem_address;

    } iom_init_struct_t;

#ifdef IOM_CFG_USE_GDMA
    /**
     * The function iom_perif_install_dma_transfer() expects this structure to install / initialize
     * a GDMA job in order to copy PERIF data to/from an external memory.
     *
     * @see iom_perif_install_dma_transfer
     */
    typedef union
    {
        /** output CR specific descriptor. data direction : controller -> device (IM) */
        struct
        {
            LSA_UINT32*         dest_addr;             /**< address of the destination buffer, to which the CR data will be copied */
            gdma_trigger_src_t  trigger;                /**< trigger source of the GDMA job to be created */
            LSA_UINT8           priority;               /**< priority of the GDMA job */
            LSA_VOID (*job_done_callback)(LSA_INT16 job_num, LSA_UINT32 cbf_arg);    /**< callback pointer : callback is executed after the GDMA job has finished */
            LSA_UINT32          cbf_arg;                /**< additional argument, that shall be passed to the callback */
        }out_cr;

        /** input CR specific descriptor. data direction : device (IM) -> controller */
        struct
        {
            LSA_UINT32*         src_addr;              /**< address of the source buffer, from which the CR data will be copied */
            gdma_trigger_src_t  trigger;
            LSA_UINT8           priority;
            LSA_VOID (*job_done_callback)(LSA_INT16 job_num, LSA_UINT32 cbf_arg);
            LSA_UINT32          cbf_arg;
        }in_cr;
    }iom_dma_request_t;
#endif /* end of IOM_CFG_USE_GDMA*/

#else
    #error "config error : define IOM_CFG_PNIP"
#endif /* end of IOM_CFG_PNIP */

typedef struct iom_apdu_status_s
{
    LSA_UINT16 cycle_counter;
    LSA_UINT8  data_status;
    LSA_UINT8  transfer_status;
} iom_apdu_status_t;

typedef LSA_VERSION_TYPE* iom_version_info_ptr_t;

/*
 * function declarations for PERIF
 */
LSA_VOID   iom_init(LSA_VOID);

LSA_BOOL   iom_allocate_iocr_memory(LSA_UINT32 ar_idx, LSA_UINT16 ar_set_nr, LSA_UINT16 session_key, CM_UPPER_SV_AR_CONNECT_IOCR_PTR_TYPE iocr_ptr);

LSA_VOID   iom_free_iocr_memory(LSA_UINT32 ar_idx);

LSA_UINT16 iom_consumer_lock(LSA_VOID** buffer, iom_apdu_status_t** apdu_status_ptr, LSA_UINT32 ar_idx);

LSA_UINT16 iom_consumer_unlock(LSA_UINT32 ar_idx);

LSA_VOID   iom_provider_lock(LSA_VOID** buffer, LSA_UINT32 ar_idx);

LSA_UINT16 iom_provider_unlock(LSA_UINT32 ar_idx);

LSA_BOOL   iom_get_iocr_buffer_sizes(LSA_UINT32 ar_idx, LSA_UINT32 *input_cr_buffer_size,           LSA_UINT32 *output_cr_buffer_size,
                                                        LSA_UINT32 *input_cr_buffer_size_allocated, LSA_UINT32 *output_cr_buffer_size_allocated);

#ifdef IOM_SYSTEM_REDUNDANCY_SUPPORT
LSA_UINT32 iom_provider_set_data_state     (LSA_UINT32 ar_idx, LSA_UINT8 value, LSA_UINT8 mask);
LSA_VOID   iom_set_session_key_primary_arid(LSA_UINT32 ar_idx);
LSA_UINT16 iom_provider_get_primary_arid   (LSA_UINT16 ARSetID);
#endif

LSA_UINT16 iom_version(LSA_UINT16 version_len, iom_version_info_ptr_t version_ptr);

#if defined IOM_CFG_PNIP

#ifdef IOM_CFG_USE_GDMA
LSA_UINT16 iom_install_dma_transfer(LSA_UINT32 ar_idx, LSA_UINT16 iocr_type, iom_dma_request_t *dma_request, LSA_INT16 *gdma_handle);
#endif
LSA_UINT16 iom_install_consumer_data_ready_isr(LSA_UINT32 ar_idx, LSA_VOID (*new_data_cbf)(LSA_UINT8));

#endif

/** @name return codes
 *  return values of IO manager functions
 *  @{*/
#define IOM_OK               0          /**< function call returned without error */
#define IOM_OK_NEW_BUFFER    0          /**< specific to lock() calls : -> new data buffer is returned */
#define IOM_OK_OLD_BUFFER    1          /**< specific to lock() calls : -> a data buffer that has been returned before is given back */
#define IOM_BUFFER_UNLOCKED  0xFFFD     /**< the buffer to be unlocked is already unlocked */
#define IOM_BUFFER_LOCKED    0xFFFE     /**< the buffer to be locked is already locked */
#define IOM_INVALID_BUFFER   0xFFFF     /**< the buffer that has been addressed doesn't exist */
#define IOM_PARAMETER_ERROR  0xFF00     /**< a function parameter is wrong */
#define IOM_INTERNAL_ERROR   0xFF01
/** @}*/

#ifdef __cplusplus
}
#endif

#endif /* IOM_COM_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
