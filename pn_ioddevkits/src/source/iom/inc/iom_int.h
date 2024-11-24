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
/*  F i l e               &F: iom_int.h                                 :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/* internal header for the IO Manager software module (PERIF)                */
/*                                                                           */
/*****************************************************************************/

#ifndef IOM_INT_H_
#define IOM_INT_H_

/*
 * PERIF function declarations
 */
LSA_VOID   iom_perif_init(LSA_VOID);
LSA_BOOL   iom_perif_allocate_iocr_memory(LSA_UINT32 ar_idx, LSA_UINT16 ar_set_nr, LSA_UINT16 session_key, CM_UPPER_SV_AR_CONNECT_IOCR_PTR_TYPE iocr_ptr);
LSA_VOID   iom_perif_free_iocr_memory(LSA_UINT32 ar_idx);
LSA_UINT16 iom_perif_consumer_lock(LSA_VOID **buffer, iom_apdu_status_t **apdu_status_ptr, LSA_UINT32 ar_idx);
LSA_UINT16 iom_perif_consumer_unlock(LSA_UINT32 ar_idx);
LSA_VOID   iom_perif_provider_lock(LSA_VOID **buffer, LSA_UINT32 ar_idx);
LSA_UINT16 iom_perif_provider_unlock(LSA_UINT32 ar_idx);
LSA_BOOL   iom_perif_get_iocr_buffer_sizes(LSA_UINT32 ar_idx, LSA_UINT32 *input_cr_buffer_size,           LSA_UINT32 *output_cr_buffer_size,
                                                              LSA_UINT32 *input_cr_buffer_size_allocated, LSA_UINT32 *output_cr_buffer_size_allocated);
#ifdef IOM_CFG_USE_GDMA
LSA_UINT16 iom_perif_install_dma_transfer(LSA_UINT32 ar_idx, LSA_UINT16 iocr_type, iom_dma_request_t *dma_request, LSA_INT16 *gdma_handle);
#endif
LSA_UINT16 iom_perif_install_consumer_data_ready_isr(LSA_UINT32 ar_idx, LSA_VOID (*new_data_cbf)(LSA_UINT8));
LSA_UINT32 iom_perif_provider_set_data_state(LSA_UINT32 ar_idx, LSA_UINT8 value, LSA_UINT8 mask);
LSA_VOID   iom_perif_set_session_key_primary_arid  (LSA_UINT32 ar_idx);
LSA_UINT16 iom_perif_get_session_key_primary_arid  (LSA_UINT16 ARSetID);

/*
 * access macros 
 */
#define IOM_GET_ALLOC_LEN_FROM_IOCR_PTR(_IOCR_PTR)                  _IOCR_PTR->iobuf.alloc_len
#define IOM_GET_APDU_OFFSET_FROM_IOCR_PTR(_IOCR_PTR)                _IOCR_PTR->iobuf.edd_apdu_status_offset
#define IOM_GET_DATA_STATUS_OFFSET_FROM_IOCR_PTR(_IOCR_PTR)         _IOCR_PTR->iobuf.edd_data_status_offset
#define IOM_GET_IS_PACKFRAME_FROM_IOCR_PTR(_IOCR_PTR)               _IOCR_PTR->iobuf.is_packframe
#define IOM_GET_DATA_OFFSET_IN_IOCR_PTR(_IOCR_PTR)                  _IOCR_PTR->iobuf.edd_data_offset
#define IOM_SET_DATA_OFFSET_IN_IOCR_PTR(_IOCR_PTR, _DATA_OFFSET)    _IOCR_PTR->iobuf.edd_data_offset = _DATA_OFFSET
#define IOM_SET_CR_NUMBER_IN_IOCR_PTR(_IOCR_PTR, _CR_INDEX)         _IOCR_PTR->iobuf.edd_cr_number   = _CR_INDEX
#define IOM_SET_FORWARD_MODE_IN_IOCR_PTR(_IOCR_PTR, _FWD_MODE)      _IOCR_PTR->iobuf.edd_forward_mode = _FWD_MODE
#define IOM_SET_CT_OUTDATASTART(_IOCR_PTR, _OUTDATA_START)          _IOCR_PTR->iobuf.edd_ct_outdatastart = _OUTDATA_START
#define IOM_SET_EDD_BUFFER_PROPERTIES_IN_IOCR_PTR(_IOCR_PTR, _PROP) _IOCR_PTR->iobuf.edd_buffer_properties = _PROP


/*
 * common definitions
 */
#define IOM_EXIT_RETURN(x)      IOM_EXIT();     \
                                return (x)
#define IOM_EXIT_RETURN_IO(x)   IOM_EXIT_IO();  \
                                return (x)

/**
 * component version constants
 */
#define IOM_LSA_PREFIX                         "   -IOM  "
#define IOM_KIND                               /* &K */ 'V'  /* K&                */
                                               /* preliminary: 'R': release       */
                                               /*              'C': correction    */
                                               /*              'S': spezial       */
                                               /*              'T': test          */
                                               /*              'B': labor         */
                                               /* prereleased: 'P': pilot         */
                                               /* released:    'V': version       */
                                               /*              'K': correction    */
                                               /*              'D': demonstration */
#define IOM_VERSION                            /* &V */ 0    /* V& */ /* [1 - 99] */
#define IOM_DISTRIBUTION                       /* &D */ 3    /* D& */ /* [0 - 99] */
#define IOM_FIX                                /* &F */ 11   /* F& */ /* [0 - 99] */
#define IOM_HOTFIX                             /* &H */ 0    /* H& */ /* [0]      */
#define IOM_PROJECT_NUMBER                     /* &P */ 0    /* P& */ /* [0 - 99] */
                                                             /* At LSA always 0!  */
#define IOM_INCREMENT                          /* &I */ 0    /* I& */ /* [1 - 99] */
#define IOM_INTEGRATION_COUNTER                /* &C */ 0    /* C& */ /* [1 - 99] */
#define IOM_GEN_COUNTER                        /* &G */ 0    /* G& */ /* [1]      */

/*
 * error handling
 */
#ifdef IOM_CFG_FATAL_ERROR_ON_FAILURE
    #ifdef IOM_LOCAL_ERR_MODULE
        #if IOM_LOCAL_ERR_MODULE == IOM_MODULE_PERIF
            #define IOM_ASSERT(condition, error_code)   if(!(condition))                                                    \
                                                        {                                                                   \
                                                            iom_perif_in_fatal_error(IOM_LOCAL_ERR_MODULE, (LSA_UINT16) __LINE__, (LSA_UINT32) error_code);     \
                                                            /*NOTREACHED*/    \
                                                        }
        #endif
    #endif
#else
    #define IOM_ASSERT(condition, error_code)   if(!(condition))                                                    \
                                                {                                                                   \
                                                   IOM_EXIT_RETURN(error_code);                                     \
                                                }
#endif
#endif /* IOM_INT_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
