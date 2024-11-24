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
/*  F i l e               &F: iom_perif.h                               :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  header file containing definitions internally used by iom_perif.c        */
/*                                                                           */
/*****************************************************************************/
#ifndef APMA_PERIF_H_
#define APMA_PERIF_H_



#ifdef IOM_CFG_PNIP

/*lint --e{46} allow unsigned long bit fields */

/* ----------------------------- function declarations, local to PERIF --------------------------------- */

static LSA_VOID   iom_perif_in_fatal_error( LSA_UINT8 error_module, LSA_UINT16 error_line, LSA_UINT32 error_code);

/* -------------------------------- hardware register definitions -------------------------------------- */

#define IOM_PERIF_IP_VERSION            ((volatile iom_perif_ip_version_reg_t *)        U_PERIF_APB__IP_VERSION)
#define IOM_PERIF_ADDRESS_MODE          ((volatile iom_perif_address_mode_reg_t *)      U_PERIF_APB__ADDRESS_MODE)

#define IOM_PERIF_CR_ADDRESS_N__SET_FIELD(x, _FIELD_NAME, _VALUE)          if(iom_data.use_consistency_control_hw)     \
                ((volatile iom_perif_cr_address_reg_t *) (U_PERIF_APB__CR_ADDRESS_1 + (x-1) * sizeof(iom_perif_cr_address_reg_t)))->perif_v2cc._FIELD_NAME = _VALUE;     /* x : 1 ... 27 */\
                                                                            else                                        \
                ((volatile iom_perif_cr_address_reg_t *) (U_PERIF_APB__CR_ADDRESS_1 + (x-1) * sizeof(iom_perif_cr_address_reg_t)))->perif_v12._FIELD_NAME = _VALUE;       /* x : 1 ... 27 */

#define IOM_PERIF_CR_STATE_N(x)         ((volatile iom_perif_cr_state_reg_t *)         (U_PERIF_APB__CR_STATE_1 + (x-1) * sizeof(iom_perif_cr_state_reg_t)))    /* x : 1 ... 27 */
#define IOM_PERIF_COMMAND_IF_CONTROL    ((volatile iom_perif_command_if_control_reg_t*) U_PERIF_AHB__COMMAND_IF_CONTROL_APP)

#define IOM_PERIF_COMMAND_IF_CONTROL__SET_FIELD(_ADDR, _FIELD_NAME, _VALUE)         if(iom_data.use_consistency_control_hw)             \
                ((volatile iom_perif_command_if_control_reg_t *)      (_ADDR))->perif_v2cc._FIELD_NAME = _VALUE;     /* x : 1 ... 27 */ \
                                                                                    else                                                \
                ((volatile iom_perif_command_if_control_reg_t *)      (_ADDR))->perif_v12._FIELD_NAME = _VALUE;       /* x : 1 ... 27 */

#define IOM_PERIF_GUARD_CONTROL_N(x)    ((volatile iom_perif_guard_control_reg_t *)    (U_PERIF_APB__GUARD_CONTROL_1 + (x-1) * sizeof(iom_perif_guard_control_reg_t)))  /* x : 1 ... 27 */

#define IOM_PERIF_HOST_EVENT_LOW        ((volatile LSA_UINT32*)                 U_PERIF_AHB__HOST_EVENT_LOW)
#define IOM_PERIF_PN_IRQ_MASK_LOW       ((volatile LSA_UINT32*)                 U_PERIF_AHB__PN_IRQMASK_LOW)
#define IOM_PERIF_PN_IRQ_MASK_HIGH      ((volatile LSA_UINT32*)                 U_PERIF_AHB__PN_IRQMASK_HIGH)
#define IOM_PERIF_PN_IRQACK_LOW         ((volatile LSA_UINT32*)                 U_PERIF_AHB__PN_IRQACK_LOW)
#define IOM_PERIF_PN_IRQACK_HIGH        ((volatile LSA_UINT32*)                 U_PERIF_AHB__PN_IRQACK_HIGH)
#define IOM_PERIF_PN_IRQ_LOW            ((volatile LSA_UINT32*)                 U_PERIF_AHB__PN_IRQ_LOW)
#define IOM_PERIF_PN_IRQ_HIGH           ((volatile LSA_UINT32*)                 U_PERIF_AHB__PN_IRQ_HIGH)
#define IOM_PERIF_PN_EOI                ((volatile iom_perif_pn_eoi_reg_t *)    U_PERIF_AHB__PN_EOI)
#define IOM_PERIF_BURST_CONFIG          ((volatile iom_perif_burst_config_reg_t*) U_PERIF_APB__BURST_CONFIG)

#define IOM_PERIF_ACTIVATE_NEW_DATA_INT(x)      *IOM_PERIF_PN_IRQ_MASK_LOW &= ~(1UL << (16 + (x)))    /* x: 0 ... 8 */
#define IOM_PERIF_DEACTIVATE_NEW_DATA_INT(x)    *IOM_PERIF_PN_IRQ_MASK_LOW |=  (1UL << (16 + (x)))
#define IOM_PERIF_ACKNOWLEDGE_NEW_DATA_INT(x)   *IOM_PERIF_PN_IRQACK_LOW   =   (1UL << (16 + (x)))    /* x: 0 ... 8 */

#define IOM_PERIF_MAX_PAGE_INDEX         16
#define IOM_PERIF_PNIP_BASE_ADDR        (EDD_COMMON_MEM_PTR_TYPE)U_PNIP__BASE

#define IOM_PERIF_MEMORY_PAGE_SIZE          256
#define IOM_PERIF_MEMORY_MAX_OFFSET_IN_PAGE 32
#define IOM_PERIF_8_BYTE_ALINGMENT          0xfffffff8
#define IOM_PERIF_MAX_ALLOC_SIZE            1792

typedef struct
{
    LSA_UINT32 debug_version : 8;
    LSA_UINT32 version       : 8;
    LSA_UINT32 configuration : 16;
}iom_perif_ip_version_reg_t;

/** CR_ADDRESS_MODE (paged / direct) register structure */
typedef struct
{
    LSA_UINT32 address_mode : 1;
    LSA_UINT32 reserved : 31;
}iom_perif_address_mode_reg_t;

/** CR_ADDRESS register structure */
typedef union
{
    struct
    {
        LSA_UINT32 reserved0 : 2;
        LSA_UINT32 cr_start_address : 10;
        LSA_UINT32 reserved1 : 4;
        LSA_UINT32 cr_end_address : 10;
        LSA_UINT32 reserved : 1;
        LSA_UINT32 zero_data : 1;
        LSA_UINT32 new_data_int : 4;
    }perif_v12;

    struct
    {
        LSA_UINT32 reserved0 : 2;
        LSA_UINT32 cr_start_address : 11;
        LSA_UINT32 reserved1 : 3;
        LSA_UINT32 cr_end_address : 11;
        LSA_UINT32 zero_data : 1;
        LSA_UINT32 new_data_int : 4;
    }perif_v2cc;

}iom_perif_cr_address_reg_t;

/** CR_STATE register structure */
typedef union
{
    struct
    {
        LSA_UINT32 cr_state_d : 2;
        LSA_UINT32 reserved0  : 2;     /* Ertec200p Step 1: reserved */
        LSA_UINT32 cr_state_f1 : 2;
        LSA_UINT32 cr_state_f2 : 2;
        LSA_UINT32 cr_state_n1 : 2;
        LSA_UINT32 cr_state_n2 : 2;
        LSA_UINT32 cr_state_u : 2;
        LSA_UINT32 reserved1   : 1;     /* Ertec200p Step 1: reserved */
        LSA_UINT32 three_buffer_mode : 1;
        LSA_UINT32 mapping : 16;
    }perif_v1;

    struct
    {
        LSA_UINT32 cr_state_d : 2;
        LSA_UINT32 cr_state_r2 : 2;     /* Ertec200p Step 2: "cr_state_r2" */
        LSA_UINT32 cr_state_f1 : 2;
        LSA_UINT32 cr_state_f2 : 2;
        LSA_UINT32 cr_state_n1 : 2;
        LSA_UINT32 cr_state_n2 : 2;
        LSA_UINT32 cr_state_u : 2;
        LSA_UINT32 redundance_mode : 1; /* Ertec200p Step 2: redundance_mode (set to 1 (EDDP spec)) */
        LSA_UINT32 three_buffer_mode : 1;
        LSA_UINT32 mapping : 16;
    }perif_v2;
}iom_perif_cr_state_reg_t;

enum {
    PERIF_BUFFER_0   = 0,
    PERIF_BUFFER_1   = 1,
    PERIF_BUFFER_2   = 2,
    PERIF_BUFFER_NIL = 3,               /* Ertec200p Step 1: no buffer in this state */
    PERIF_BUFFER_3   = 3                /* Ertec200p Step 2: buffer 3 in this state (4 buffer system) */
};

/** command interface register structure */
typedef union
{
    struct
    {
        LSA_UINT32 cr_number : 5;
        LSA_UINT32 conf_request : 1;
        LSA_UINT32 f_code : 4;
        LSA_UINT32 reserved0 : 3;
        LSA_UINT32 user_id : 3;
        LSA_UINT32 reserved1 : 16;
    }perif_v12;

    struct
    {
        LSA_UINT32 reserved : 5;
        LSA_UINT32 conf_request : 1;
        LSA_UINT32 f_code : 2;
        LSA_UINT32 reserved0 : 5;
        LSA_UINT32 user_id : 3;
        LSA_UINT32 cr_number : 6;
        LSA_UINT32 reserved1 : 10;
    }perif_v2cc;
}iom_perif_command_if_control_reg_t;

/** address guard register structure */
typedef struct
{
    LSA_UINT32 guard_address : 12;
    LSA_UINT32 reserved : 18;
    LSA_UINT32 guard_type : 1;
    LSA_UINT32 guard_valid : 1;
}iom_perif_guard_control_reg_t;

/** time based interrupt clearing register */
typedef struct
{
    LSA_UINT32 wait_time : 18;
    LSA_UINT32 reserved0 : 14;
}iom_perif_pn_eoi_reg_t;

typedef struct
{
    LSA_UINT32 burst_mode_com_ahb : 2;
    LSA_UINT32 reserved1 : 6;
    LSA_UINT32 burst_mode_appl_ahb : 2;
    LSA_UINT32 reserved2 : 22;
}iom_perif_burst_config_reg_t;

/* ----------------------------- data structs internally used by PERIF ---------------------------- */

/**
 * This structure describes one memory page of the PERIF. It consists of a memory map
 * that shows which bytes are in use and which are still available (1 bit of the map
 * corresponds to 8 bytes in PERIF memory).
 * For each page, the fields "bytes_free_from_start"/"bytes_free_from_end" tell how much
 * free space is available at the beginning/end of the page respectively.
 */
typedef struct iom_perif_page_mem_desc_s
{
    LSA_UINT16  bytes_free_from_start;
    LSA_UINT16  bytes_free_from_end;
    LSA_UINT16  bytes_free;
    LSA_UINT32  memory_map[512/8/32];
}iom_perif_page_mem_desc_t;

/**
 * structure describing the state and memory properties of a CR.
 */
#define IOM_PERIF_MAX_NUM_PAGES     7       /**< maximum number of pages, one CR can contain */

typedef struct iom_perif_cr_data_s
{
    LSA_UINT32  *start_address;             /**< start address of the CR buffer in PERIF-memory */
    LSA_UINT32  buffer_size_no_padding;     /**< size of the CR buffer (without rounding) */
    LSA_UINT32  buffer_size;                /**< size of the CR buffer in PERIF memory (8 byte granularity) */
    LSA_UINT32  *apdu_status_address;       /**< for the consumer, this pointer references the place within PNIP, where the APDU-status can be found */
    LSA_UINT8   cr_index;                   /**< index of the CR (1-27), corresponds to CR-register index of PERIF hardware */
    LSA_INT16   dma_job;                    /**< specifies the DMA job that serves this CR (user data copy + address guard -> see iocr_alloc*/
#ifdef IOM_CFG_DMA_BUF_EXCHANGE
    LSA_INT16   dma_job_buf_exchange;       /**< specifies the DMA job, that's used to trigger a buffer exchange without address guard */
#endif
    LSA_UINT8   num_pages;                  /**< refers to the used_pages-structures and tells how many pages are in use by this CR */
    struct
    {
        LSA_UINT8   page_number;
        LSA_UINT16  page_offset_low;        /**< specifies the index of the first used word IN the allocated section */
        LSA_UINT16  page_offset_high;       /**< specifies the index of the first free word AFTER allocated section */
    }used_pages[IOM_PERIF_MAX_NUM_PAGES];   /**< field that describes all pages in use by this CR (page number (0-15), memory range within page (0-255)) */

    LSA_UINT8   cons_last_user_buffer_idx;  /**< used for consumer CRs: index of the lastly used user buffer -> used to examine if a buffer refresh took place between two buffer requests */
    volatile LSA_BOOL    locked;            /**< flag that specifies the lock state; changed by lock() / unlock()-API functions */
}iom_perif_cr_data_t;

/**
 * structure describing the components of an AR
 */
typedef struct iom_perif_ar_data_s
{
    iom_perif_cr_data_t *output_cr;          /**< pointer to output-CR belonging to this AR */
    iom_perif_cr_data_t *input_cr;           /**< pointer to input-CR belonging to this AR */
    LSA_VOID (*new_data_cbf)(LSA_UINT8);     /**< pointer to callback that is executed when new data arrives for the output CR */
    LSA_BOOL free_request_issued;            /**< flag that signals the deallocation of all AR resources (during the next unlock()-call) */
    LSA_UINT16   prov_session_key;
    LSA_UINT16   prov_id;
    LSA_BOOL     is_sysred_provider;
}iom_perif_ar_data_t;

/* ----------------------- internal definitions ------------------------- */

/** module internal enum defining the type of CR that shall be dealt with */
typedef enum iom_perif_iocr_e {IOM_PERIF_INPUT_CR, IOM_PERIF_OUTPUT_CR, IOM_PERIF_ALL_CRS} iom_perif_iocr_t;

#endif  // #ifdef IOM_CFG_PNIP

#endif /* APMA_PERIF_H_ */
 
/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
