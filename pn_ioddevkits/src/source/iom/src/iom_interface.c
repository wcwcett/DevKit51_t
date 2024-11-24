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
/*  F i l e               &F: iom_interface.c                           :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  This file contains the user interface for the IOM manager. Function calls*/
/*  descend into "PERIF module driver" in case of the Ertec200p,             */
/*                                                                           */
/*****************************************************************************/

#include "iom_inc.h"

/**
 * @name common functions for PERIF
 * @{ */

/**
 * @brief module initialization function.
 *      Call this function near the initialization of the EDD.
 *
 * @param init_data     structure containing PERIF specific configuration parameters.
 *                      See iom_perif_init for details.
 */
LSA_VOID iom_init()
{
#if defined IOM_CFG_PNIP
    iom_perif_init();
#endif
}

/**
 * @brief This function allocates memory for one IOCR when an AR is set up.
 *
 * @param   ar_idx      index of the AR to be set up. PERIF : [0 - (IOM_CFG_PERIF_NUM_AR-1) (7)]
 * @param   ar_set_nr
 * @param   session_key
 * @param   iocr_ptr    This structure defines the size of the buffer to be allocated and the IOCR type (input / output).
 *                      The address of the allocated buffer is returned via this structure pointer.
 */
LSA_BOOL iom_allocate_iocr_memory(LSA_UINT32 ar_idx, LSA_UINT16 ar_set_nr, LSA_UINT16 session_key, CM_UPPER_SV_AR_CONNECT_IOCR_PTR_TYPE iocr_ptr)
{
#if defined IOM_CFG_PNIP
    return iom_perif_allocate_iocr_memory(ar_idx, ar_set_nr, session_key, iocr_ptr);
#endif
}

/**
 * @brief   Frees the IOCR memory for one specific AR, that has been previously allocated by iom_allocate_iocr_memory.
 *          The memory of both CRs belonging to the AR is freed. This function may not free the memory synchronously
 *          because buffers might be locked at calling time. The macro IOM_IOCR_MEMORY_FREED() is called, when
 *          deallocation is completed.
 * @param   ar_idx      index of the AR to be deallocated
 */
LSA_VOID iom_free_iocr_memory(LSA_UINT32 ar_idx)
{
#if defined IOM_CFG_PNIP
    iom_perif_free_iocr_memory(ar_idx);
#endif
}

#ifdef IOM_CFG_LOCATE_INSTRUCTIONS_TO_TCM
    #if defined (TOOL_CHAIN_GREENHILLS_ARM)
        #pragma ghs section text=IOM_ITCM_SECTION_NAME
        #define IOM_ATTRIBUTE_ITCM
    #elif defined (TOOL_CHAIN_GNU_PPC)
        #define IOM_ATTRIBUTE_ITCM __attribute__ ((section (IOM_ITCM_SECTION_NAME)))
    #else
        #define IOM_ATTRIBUTE_ITCM __attribute__ ((section (IOM_ITCM_SECTION_NAME)))
    #endif
#else
    #define IOM_ATTRIBUTE_ITCM
#endif


/**
 * @brief   This function retrieves the latest consumer buffer (communication direction : controller -> device / output CR).
 *          In case of the Perif submodule, data consistency is always guaranteed;
 * @param   buffer          the address of the latest consumer buffer is returned by this parameter
 * @param   apdu_status_ptr returns a pointer to the APDU status data. Enter LSA_NULL, if you don't need it.
 * @param   ar_idx          specifies the AR, whose output CR buffer shall be acquired
 * @return  IOM_INVALID_BUFFER if the output CR is not initialized,
 *          IOM_BUFFER_LOCKED if the CR memory is already locked,
 *          IOM_OK_NEW_BUFFER or IOM_OK_OLD_BUFFER if locking was successful
 */
IOM_ATTRIBUTE_ITCM LSA_UINT16 iom_consumer_lock(LSA_VOID** buffer, iom_apdu_status_t** apdu_status_ptr, LSA_UINT32 ar_idx)
{
#if defined IOM_CFG_PNIP
    return iom_perif_consumer_lock(buffer, apdu_status_ptr, ar_idx);
#endif
}


/**
 * @brief   This function revokes the reservation of the latest consumer buffer (communication direction : controller -> device / output CR).
 *          It has to be called after the users usage of the consumer buffer
 * @param   ar_idx      specifies the AR, whose output CR buffer shall be unlocked
 * @return  IOM_INVALID_BUFFER if the output CR is not initialized, IOM_BUFFER_UNLOCKED if the buffer
 *          isn't in the locked state, else IOM_PERIF_OK. Here you get no destinction between old and new buffers.
 */
IOM_ATTRIBUTE_ITCM LSA_UINT16 iom_consumer_unlock(LSA_UINT32 ar_idx)
{
#if defined IOM_CFG_PNIP
    return iom_perif_consumer_unlock(ar_idx);
#endif
}

/**
 * @brief   This function reserves the current provider buffer for the application (communication direction : device -> controller).
 *          It has to be called before the user writes to the input data buffer.
 * @param   buffer  returns the address of the current user buffer
 * @param   ar_idx  specifies the AR, whose input CR memory shall be locked
 */
IOM_ATTRIBUTE_ITCM LSA_VOID iom_provider_lock(LSA_VOID **buffer, LSA_UINT32 ar_idx)
{
#if defined IOM_CFG_PNIP
    iom_perif_provider_lock(buffer, ar_idx);
#endif
}


/**
 * @brief   This function revokes the reservation of the input buffer and hands it over to the hardware, managed by the EDD.
 *          It has to be called after the user has written to the input data buffer.
 *          @see iom_perif_provider_unlock
 * @param   ar_idx  specifies the AR, whose input CR memory shall be unlocked / passed to hardware
 * @return  IOM_INVALID_BUFFER if the output CR is not initialized, IOM_BUFFER_UNLOCKED if the buffer
 *          isn't in the locked state. IOM_OK_OLD_BUFFER, if the last provider buffer wasn't accepted by the
 *          EDD as next buffer (relevant only for EDDI, Perif can always update buffers and returns IOM_OK_NEW_BUFFER!).
 *          IOM_OK_NEW_BUFFER if the last provider buffer was accepted.
 */
IOM_ATTRIBUTE_ITCM LSA_UINT16 iom_provider_unlock(LSA_UINT32 ar_idx)
{
#if defined IOM_CFG_PNIP
    return iom_perif_provider_unlock(ar_idx);
#endif
}


#ifdef IOM_CFG_LOCATE_INSTRUCTIONS_TO_TCM
    #if defined (TOOL_CHAIN_GREENHILLS_ARM)
        #pragma ghs section text=IOM_DEFAULT_TEXT_SECTION_NAME
    #elif defined (TOOL_CHAIN_GNU_PPC)
        /* nothing needs to be done here */
    #else
        /* nothing needs to be done here */
    #endif
#endif

/**
 * @brief   This function returnes the buffer sizes (without APDU status) of the ICR and OCR, belonging to the AR <ar_idx>.
 *          These buffer sizes don't include padding bytes and don't correspond to internal buffer sizes!
 *          -> PERIF: performs internal 8 byte rounding
 * @param   ar_idx                  specifies the AR, whose buffer sizes are requested
 * @param   input_cr_buffer_size    returns the size of the input CRs buffer
 * @param   output_cr_buffer_size   returns the size of the output CRs buffer
 * @param   input_cr_buffer_size_allocated   returns the internally allocated size of the input CRs buffer
 * @param   output_cr_buffer_size_allocated  returns the internally allocated size of the output CRs buffer
 * @return  LSA_TRUE if no error occured, LSA_FALSE if the sizes couldn't be determined
 */
LSA_BOOL iom_get_iocr_buffer_sizes(LSA_UINT32 ar_idx, LSA_UINT32 *input_cr_buffer_size,           LSA_UINT32 *output_cr_buffer_size,
                                                      LSA_UINT32 *input_cr_buffer_size_allocated, LSA_UINT32 *output_cr_buffer_size_allocated)
{
#if defined IOM_CFG_PNIP
    return iom_perif_get_iocr_buffer_sizes(ar_idx, input_cr_buffer_size, output_cr_buffer_size, input_cr_buffer_size_allocated, output_cr_buffer_size_allocated);
#endif
}


/**
 *LSA function for retreiving the version info of the IOM component
 */
LSA_UINT16 iom_version(LSA_UINT16 version_len, iom_version_info_ptr_t version_ptr)
{
    if (sizeof(LSA_VERSION_TYPE) <= version_len)
    {
        LSA_UINT32 i;
        LSA_UINT8  tmp_prefix[LSA_PREFIX_SIZE] = IOM_LSA_PREFIX;

        version_ptr->lsa_component_id = IOM_LSA_COMPONENT_ID;

        for (i = 0; LSA_PREFIX_SIZE > i; i++)
        {
            version_ptr->lsa_prefix[i] = tmp_prefix[i];
        }

        version_ptr->kind                = IOM_KIND;
        version_ptr->version             = IOM_VERSION;
        version_ptr->distribution        = IOM_DISTRIBUTION;
        version_ptr->fix                 = IOM_FIX;
        version_ptr->hotfix              = IOM_HOTFIX;
        version_ptr->project_number      = IOM_PROJECT_NUMBER;
        version_ptr->increment           = IOM_INCREMENT;
        version_ptr->integration_counter = IOM_INTEGRATION_COUNTER;
        version_ptr->gen_counter         = IOM_GEN_COUNTER;

        return (0);
    }
    else
    {
        return (version_len);
    }
}

#ifdef IOM_SYSTEM_REDUNDANCY_SUPPORT
LSA_UINT32 iom_provider_set_data_state(LSA_UINT32 ar_idx, LSA_UINT8 value, LSA_UINT8 mask)
{
#if defined IOM_CFG_PNIP
    return iom_perif_provider_set_data_state(ar_idx, value, mask);
#endif
}

/**
 * @brief This function sets the currently primary AR.
 *
 * The primary AR is set by the application after a backup->primary edge was detected in the APDU status.
 * IOM herewith gets informed about the newly primary AR and later gets asked by CM about the primary AR's session key
 * ( see iom_provider_get_primary_arid() ).
 *
 * @param ar_idx    index of the new primary AR
 */
LSA_VOID iom_set_session_key_primary_arid(LSA_UINT32 ar_idx)
{
#if defined IOM_CFG_PNIP
    iom_perif_set_session_key_primary_arid(ar_idx);
#endif
}

/**
 *  @brief This function tells the session key of the primary AR of an AR-set.
 *
  * The primary AR is set by the application after a backup->primary edge was detected in the APDU status.
  * IOM gets informed about the change in primary AR by iom_perif_set_session_key_primary_arid().
  * This function is used by CM to retrieve the primary session key of the redundant AR-Set.
  *
  * @param ArSetID  specifies the AR-set for the request.
  */
LSA_UINT16 iom_provider_get_primary_arid(LSA_UINT16 ARSetID)
{
#if defined IOM_CFG_PNIP
    return iom_perif_get_session_key_primary_arid(ARSetID);
#endif
}

#endif

/** @} */

#if defined IOM_CFG_PNIP
    /** @name PERIF specific functions
     *  @{
     */
    #ifdef IOM_CFG_USE_GDMA
    /**
     * @brief   This function installs a DMA request as described by the parameter <dma_request> for the CR defined
     *          by <ar_idx> and <iocr_type> (only on PERIF hardware).
     *          Therefore the GDMA reusable software module is used. The CR buffer is completely copied as one block
     *          to / from Perif with a granularity of 4 bytes. The job is always created in GDMA job pool 0 (first job pool
     *          - see GDMA module for details).
     *          The DMA transfer gets uninstalled as soon as the PERIF AR is deallocated. Don't remove the job on your own.
     *          Don't forget to activate the DMA transfer after it has been installed.
     *
     * @param   ar_idx      [in] specifies the AR, to which the corresponding CR belongs
     * @param   iocr_type   [in] specifies the CR within the AR [CM_IOCR_TYPE_INPUT / CM_IOCR_TYPE_OUTPUT / ...]
     * @param   dma_request [in] determines the buffer addresses of the DMA copy job and further parameters
     * @param   gdma_handle [out] returns the handle of the created DMA job. It can be used to start software triggered jobs. If gdma_handle == LSA_NULL, no handle is returned.
     *
     * @return  IOM_OK if everything went well, IOM_INTERNAL_ERROR or IOM_PARAMETER_ERROR if an error occured
     */
    LSA_UINT16 iom_install_dma_transfer(LSA_UINT32 ar_idx, LSA_UINT16 iocr_type, iom_dma_request_t *dma_request, LSA_INT16 *gdma_handle)
    {
        return iom_perif_install_dma_transfer(ar_idx, iocr_type, dma_request, gdma_handle);
    }
    #endif

    /**
     * @brief   This function enables the "new data ready" interrupt for an output CR (only on PERIF hardware).
     *          The interrupt may occur before the transfer end event occurs.
     *
     * @param ar_idx        [in] zero-based index of the AR whose ouput CR shall signal new data via an interrupt
     * @param new_data_cbf  [in] pointer to a callback function, that's called when new data is available.
     *                           The AR index is passed to this callback in case of an interrupt.
     * @return              IOM_OK if no error occured, IOM_PARAMETER_ERROR or IOM_INVALID_BUFFER otherwise.
     */
    LSA_UINT16 iom_install_consumer_data_ready_isr(LSA_UINT32 ar_idx, LSA_VOID (*new_data_cbf)(LSA_UINT8))
    {
        return iom_perif_install_consumer_data_ready_isr(ar_idx, new_data_cbf);
    }

    /** @} */

#endif /* end of IOM_CFG_PNIP */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
