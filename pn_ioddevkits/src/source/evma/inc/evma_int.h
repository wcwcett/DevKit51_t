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
/*  F i l e               &F: evma_int.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  software module handling the ERTEC200+ pnpll application timer 2 and its */
/*  comparators as well as the pnpll MUX                                     */
/*                                                                           */
/*****************************************************************************/

#ifndef EVMA_INT_H_
#define EVMA_INT_H_

/* hardware properties */
#ifdef EVMA_USE_PNIP

    /*lint --e{46} allow unsigned long bit fields */

    /* register definitions */
    typedef struct
    {
        LSA_UINT32 F_Code : 4;
        LSA_UINT32 reserved0 : 1;
        LSA_UINT32 ConfRequest : 1;
        LSA_UINT32 User_ID : 3;
        LSA_UINT32 CommandValue : 19;
        LSA_UINT32 reserved1 : 4;
    }EVMA_PNIP_SYNCCOMMAND_IF_CONTROL_REG;

    typedef union
    {
        struct
        {
            LSA_UINT32 PLL_CMPSelect : 5;
            LSA_UINT32 reserved0     : 3;
            LSA_UINT32 PLL_APISelect : 4;
            LSA_UINT32 reserved1     : 4;
            LSA_UINT32 PLL_Level     : 1;
            LSA_UINT32 PLL_Length    : 7;
            LSA_UINT32 reserved2     : 8;
        }bits;
        LSA_UINT32  reg32;
    }EVMA_PNIP_PLL_OUT_CONTROL_REG;

    #define EVMA_NUM_COMPARATORS   7
    #define EVMA_NUM_OUTPUTS       21

    /* interrupt numbers of the Ertec200p's ICU */
    #define EVMA_INT_PNPLL_OUT9    72
    #define EVMA_INT_PNPLL_OUT10   73
    #define EVMA_INT_PNPLL_OUT11   74
    #define EVMA_INT_PNPLL_OUT12   75
    #define EVMA_INT_PNPLL_OUT13   76
    #define EVMA_INT_PNPLL_OUT14   77

#endif

/* -------------------------------------------------------- */
/* instance descriptor specific stuff                       */
/* -------------------------------------------------------- */
#define EVMA_INST_DESC_MAGIC        0x5a7f3922

typedef enum evma_instance_state_e
{
    EVMA_STATE_IDLE,
    EVMA_STATE_IN_USE
}evma_instance_state_t;

#ifdef EVMA_USE_PNIP
    /** PNIP instance descriptor */
typedef struct evma_instance_s
{
    LSA_UINT32 magic_value;             /** magic value that is used to verify the validity of the instance structure */
    evma_instance_state_t state;
    evma_application_type_t app_type;   /** is set according to the evma_alloc_event() - input parameter */
    LSA_VOID (*pnpll_isr)(LSA_VOID);    /** callback function, that is called at PNPLL event */
    LSA_UINT8 output_index;             /** index of the PNPLL output signal, that was allocated for this instance. Example: Output for dma shall be allocated -> 11 is returned for PNPLL11 */
    LSA_UINT8 application_timer_index;  /** index of the used application timer (2 = application timer for users */
    LSA_UINT8 comparator_index;         /** index of the allocated comparator inside the application timer */
    LSA_UINT32 trigger_time_ns;         /** is set according to the evma_alloc_event() input parameter */
    LSA_UINT8 gpio_index;               /** specifies the GPIO index that is to be used if application type EVMA_APPLICATION_GPIO is requested. -> output_index is defined that way */
    evma_gpio_level_type_t gpio_level;  /** specifies whether the GPIO signal is low or high active. only used for application type EVMA_PNPLL_APPLICATION_GPIO */
}evma_instance_t;

#endif

/* -------------------------------------------------------- */
/* handling of lists                                        */
/* -------------------------------------------------------- */
struct  evma_list_cb
{
    struct evma_list_cb* next_blk_ptr;
    struct evma_list_cb* prev_blk_ptr;
};

#define EVMA_CAST_LIST_CB_PTR (struct evma_list_cb *)
#define EVMA_CAST_LIST_CB_PTR_PTR (struct evma_list_cb **)

    /* initialization of list */
#define evma_init_list__(list_ptr)                                                          \
{                                                                                           \
    (EVMA_CAST_LIST_CB_PTR (list_ptr)) -> next_blk_ptr =                                    \
    (EVMA_CAST_LIST_CB_PTR (list_ptr)) -> prev_blk_ptr = EVMA_CAST_LIST_CB_PTR (list_ptr);  \
}

    /* check if list is empty */
#define evma_list_empty__(list_ptr)                                                         \
                                                                                            \
    (EVMA_CAST_LIST_CB_PTR (list_ptr)) -> next_blk_ptr == EVMA_CAST_LIST_CB_PTR (list_ptr)  \

    /* attach entry to the end of the list */
#define evma_put_blk_to_list__(list_ptr, blk_ptr)                                                           \
{                                                                                                           \
    /* blk.next = &list */                                                                                  \
    (EVMA_CAST_LIST_CB_PTR (blk_ptr)) -> next_blk_ptr = EVMA_CAST_LIST_CB_PTR (list_ptr);                   \
                                                                                                            \
    /* list.prev -> next = blk */                                                                           \
    (EVMA_CAST_LIST_CB_PTR (list_ptr)) -> prev_blk_ptr -> next_blk_ptr = EVMA_CAST_LIST_CB_PTR (blk_ptr);   \
                                                                                                            \
    /* blk.prev = list.prev */                                                                              \
    (EVMA_CAST_LIST_CB_PTR (blk_ptr)) -> prev_blk_ptr = (EVMA_CAST_LIST_CB_PTR (list_ptr)) -> prev_blk_ptr; \
                                                                                                            \
    /* list.prev = blk */                                                                                   \
    (EVMA_CAST_LIST_CB_PTR (list_ptr)) -> prev_blk_ptr = EVMA_CAST_LIST_CB_PTR (blk_ptr);                   \
}

    /* remove entry from anywhere in the list */
#define evma_remove_blk__(blk_ptr)                                                                                          \
{                                                                                                                           \
    /* blk.prev -> next = blk.next */                                                                                       \
    (EVMA_CAST_LIST_CB_PTR (blk_ptr)) -> prev_blk_ptr -> next_blk_ptr = (EVMA_CAST_LIST_CB_PTR (blk_ptr)) -> next_blk_ptr;  \
                                                                                                                            \
    /* blk.next -> prev = blk.prev */                                                                                       \
    (EVMA_CAST_LIST_CB_PTR (blk_ptr)) -> next_blk_ptr -> prev_blk_ptr = (EVMA_CAST_LIST_CB_PTR (blk_ptr)) -> prev_blk_ptr;  \
}

    /* get first entry of the list */
#define evma_get_blk_from_list__(list_ptr, adr_blk_ptr)                                                     \
{                                                                                                           \
    /* blk = list.next */                                                                                   \
    *(EVMA_CAST_LIST_CB_PTR_PTR (adr_blk_ptr)) = (EVMA_CAST_LIST_CB_PTR (list_ptr)) -> next_blk_ptr;        \
}



/*
 * component version constants
 */
#define EVMA_LSA_PREFIX                         "   -EVMA "
#define EVMA_KIND                               /* &K */ 'V'  /* K&                */
                                                /* preliminary: 'R': release       */
                                                /*              'C': correction    */
                                                /*              'S': spezial       */
                                                /*              'T': test          */
                                                /*              'B': labor         */
                                                /* prereleased: 'P': pilot         */
                                                /* released:    'V': version       */
                                                /*              'K': correction    */
                                                /*              'D': demonstration */
#define EVMA_VERSION                            /* &V */ 1    /* V& */ /* [1 - 99] */
#define EVMA_DISTRIBUTION                       /* &D */ 0    /* D& */ /* [0 - 99] */
#define EVMA_FIX                                /* &F */ 3    /* F& */ /* [0 - 99] */
#define EVMA_HOTFIX                             /* &H */ 0    /* H& */ /* [0]      */
#define EVMA_PROJECT_NUMBER                     /* &P */ 0    /* P& */ /* [0 - 99] */
                                                              /* At LSA always 0!  */
#define EVMA_INCREMENT                          /* &I */ 0    /* I& */ /* [1 - 99] */
#define EVMA_INTEGRATION_COUNTER                /* &C */ 0    /* C& */ /* [1 - 99] */
#define EVMA_GEN_COUNTER                        /* &G */ 0    /* G& */ /* [1]      */

/*
 * error handling
 */

#ifdef EVMA_CFG_FATAL_ERROR_ON_FAILURE
    #define EVMA_ASSERT(condition, error_code)  if(!(condition))                                                    \
                                                {                                                                   \
                                                    evma_in_fatal_error(EVMA_LOCAL_ERR_MODULE, __LINE__, (LSA_UINT16) error_code);     \
                                                }
#else
    #define EVMA_ASSERT(condition, error_code)  if(!(condition))                                                    \
                                                {                                                                   \
                                                   return(error_code);                                              \
                                                }
#endif


PNIO_VOID evma_in_fatal_error( LSA_UINT8 error_module, LSA_UINT16 error_line, LSA_UINT16 error_code);


#endif /* EVMA_INT_H_ */
 
/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
