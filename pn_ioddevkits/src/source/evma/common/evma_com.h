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
/*  F i l e               &F: evma_com.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  component header for the EVMA software module                            */
/*                                                                           */
/*****************************************************************************/

#ifndef EVMA_COM_H_
#define EVMA_COM_H_

typedef LSA_VERSION_TYPE* evma_version_info_ptr_t;

#ifdef EVMA_USE_PNIP
/**
 * By passing one of the underneath application types, you tell the function
 * evma_alloc_event(), which purpose the event shall have -> an appropriate
 * MUX output is chosen by the EVMA library.
 */
typedef enum evma_application_type_e
{
    EVMA_APPLICATION_ARM_ICU,         /**< creates an event, thats purpose is to trigger an ISR */
    EVMA_APPLICATION_GDMA,            /**< creates an event, thats purpose is to trigger a GDMA job */
    EVMA_APPLICATION_ARM_ICU_GDMA,    /**< creates an event, that might trigger a GDMA job and an ISR (ISR must be set) */
    EVMA_APPLICATION_GPIO,            /**< creates an event, that is forwarded to a GPIO pin */
    EVMA_APPLICATION_SSI,
    EVMA_PERIF_TI_IN,                 /**< not possible due to EDDP's reservation of MUX output 0 */
    EVMA_PERIF_TO_IN
}evma_application_type_t;

/** the enum defines the polarity of the signal, that is output at a GPIO pin */
typedef enum evma_gpio_level_type_e
{
    EVMA_GPIO_LOW_ACTIVE = 0,
    EVMA_GPIO_HIGH_ACTIVE = 1
}evma_gpio_level_type_t;
#endif /* end of EVMA_USE_PNIP */

typedef PNIO_VOID*   evma_handle_t;

/** parameter structure that is used to parameterize an event
 *  @see evma_alloc_event
 *  @see evma_alloc_event_transfer_end
 */
typedef struct evma_alloc_params_s
{
    /** application_type specifies the purpose of the event, that is to be created. You have to
     *  fill the corresponding specific parameter structure in the following "spec_params-union"
     *  in order to parameterize the event */
    evma_application_type_t application_type;

    union alloc_specific_params_u
    {
        /** icu_params : must be provided for application type EVMA_APPLICATION_ARM_ICU */
        struct
        {
            LSA_VOID    (*event_cbf)(LSA_VOID);            /**< input parameter : pointer to the ISR */
        }icu_params;

        /** gpio_params : must be provided for application type EVMA_APPLICATION_GPIO. The GPIO registers
         *              are NOT MANAGED by EVMA! */
        struct
        {
            LSA_UINT8               gpio_index;            /**< input parameter : specifies the GPIO index.
                                                                      Ertec200p : 1-8 , 0 is managed by EDDP
                                                                      Ertec200  : 25-30 */
#ifdef EVMA_USE_PNIP
            evma_gpio_level_type_t  level_type;            /**< input parameter : set level_type to EVMA_GPIO_HIGH_ACTIVE or EVMA_GPIO_LOW_ACTIVE */
#endif
        }gpio_params;

#ifdef EVMA_USE_PNIP
        /** gdma_params (Ertec200p only) : must be provided for application type EVMA_APPLICATION_GDMA */
        struct
        {
            LSA_UINT8   mux_output;                        /**< output parameter : MUX output that was actually allocated.
                                                                    this value is needed when it comes to initializing the
                                                                    corresponding GDMA job, that is to be triggered by the allocated event.
                                                                    example: 11 is returned for PNPLL11 */
        }gdma_params;

        /** icu_gdma_params (Ertec200p only) : must be provided for application type EVMA_APPLICATION_ARM_ICU_GDMA */
        struct
        {
            LSA_VOID    (*event_cbf)(LSA_VOID);            /**< input parameter : pointer to the ISR */
            LSA_UINT8   mux_output;                        /**< output parameter : see the above description */
        }icu_gdma_params;
#endif

        /* PNIP: ssi / ti / to - application types don't need additional parameters */
    }spec_params;
}evma_alloc_params_t;



/* function declarations */
#ifdef __cplusplus
extern "C"
{
#endif

LSA_UINT16      evma_version                 (LSA_UINT16 version_len, evma_version_info_ptr_t version_ptr);

LSA_VOID        evma_init                    (PNIO_VOID);
evma_handle_t   evma_alloc_event             (evma_alloc_params_t* alloc_params, LSA_UINT32 trigger_time_ns);
evma_handle_t   evma_alloc_event_transfer_end(evma_alloc_params_t* alloc_params);
LSA_UINT8       evma_free_event              (evma_handle_t pnpll_instance);

#ifdef __cplusplus
}
#endif

#ifdef EVMA_USE_PNIP

/** reads the cycle length of the application timer (32 bit value [ns]) */
#define EVMA_GET_CYCLE_LENGTH()             EVMA_READ_PNIP_APPLTIMER2_LENGTH()
/** reads the current time of the application timer (32 bit value, [ns]) */
#define EVMA_GET_CYCLE_TIME()               EVMA_READ_PNIP_APPLTIMER2_TIME()
/** reads the transfer end time, that is set by the EDDP. The EDDP has to be
 *  parameterized accordingly on startup (-> management of application timer 2, comparator 0) */
#define EVMA_PNPLL_GET_TRANSFER_END_TIME()  EVMA_READ_PNIP_APPLTIMER2_COMP_TIME(0)

/** defines the number of interrupt vectors that are managed/used by EVMA */
#define EVMA_NUM_ISR_VECTORS                6
/** defines the index of the first interrupt vector that is used by EVMA (identical to EVMA_INT_PNPLL_OUT9) */
#define EVMA_FIRST_ISR_VECTOR_INDEX         72

#endif

#endif /* EVMA_COM_H_ */
 
/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
