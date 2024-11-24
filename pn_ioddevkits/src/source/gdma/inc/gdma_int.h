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
/*  F i l e               &F: gdma_int.h                                :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  module internal header file for the ERTEC200+ GDMA module                */
/*                                                                           */
/*****************************************************************************/

#ifndef GDMA_H_
#define GDMA_H_

/*lint --e{46} allow unsigned long bit fields */

#define GDMA_MAX_JOBS                                   32
#define GDMA_MAX_TRANSFERS                              256
#define GDMA_STACK_SIZE                                 512


// ------------------------------------------
// DMA registers
// ------------------------------------------
#define GDMA_REG_ADDR                                    ((volatile LSA_UINT32*) (GDMA_CFG_BASE_ADDRESS))
#define GDMA_LIST_ADDR                                   ((volatile LSA_UINT32*) (GDMA_CFG_BASE_ADDRESS + 0x04))
#define GDMA_MAIN_CTRL                                   ((volatile gdma_main_ctrl_reg_t*) (GDMA_CFG_BASE_ADDRESS + 0x08))
#define GDMA_JC_EN                                       ((volatile LSA_UINT32*) (GDMA_CFG_BASE_ADDRESS + 0x0C))
#define GDMA_JOBX_CTRL(x)                                ((volatile gdma_jobx_ctrl_reg_t*) (GDMA_CFG_BASE_ADDRESS + 0x10 + (x) * sizeof(gdma_jobx_ctrl_reg_t)))              /* x : 0 ... 31 */
#define GDMA_JOBX_CTRL_VAR_BASE_ADDR(reg_base_addr, x)   ((volatile gdma_jobx_ctrl_reg_t*) (reg_base_addr + (x) * sizeof(gdma_jobx_ctrl_reg_t)))/* x : 0 ... 31 */
#define GDMA_ACTUAL_STATUS                               ((volatile gdma_actual_status_reg_t*) (GDMA_CFG_BASE_ADDRESS + 0x98))
#define GDMA_FINISHED_JOBS                               ((volatile LSA_UINT32*) (GDMA_CFG_BASE_ADDRESS + 0x94))
#define GDMA_IRQ_STATUS                                  ((volatile LSA_UINT32*) (GDMA_CFG_BASE_ADDRESS + 0x9C))
#define GDMA_LIST_RAM_START                              ((volatile LSA_UINT32*) (GDMA_CFG_BASE_ADDRESS + 0xB0))
#define GDMA_LIST_RAM_END                                ((volatile LSA_UINT32*) (GDMA_CFG_BASE_ADDRESS + 0x10AF))
#define GDMA_TRANS_RECX_SRC(list_start_addr, entry_idx)  ((volatile LSA_UINT32*) (list_start_addr + (entry_idx) * 0x10))
#define GDMA_TRANS_RECX_DST(list_start_addr, entry_idx)  ((volatile LSA_UINT32*) (list_start_addr + (entry_idx) * 0x10 + 0x04))
#define GDMA_TRANS_RECX_CTRL(list_start_addr, entry_idx) ((volatile LSA_UINT32*) (list_start_addr + (entry_idx) * 0x10 + 0x08))
#define GDMA_TRANS_RECX_CNT(list_start_addr, entry_idx)  ((volatile gdma_trans_recx_cnt_reg_t*) (list_start_addr + (entry_idx) * 0x10 + 0x0C))
#define GDMA_JOB_STACK_RAM_START                         ((volatile LSA_UINT32*) (GDMA_CFG_BASE_ADDRESS + 0x10B0))
#define GDMA_JOB_STACK_RAM_END                           ((volatile LSA_UINT32*) (GDMA_CFG_BASE_ADDRESS + 0x12AF))

typedef union
{
    LSA_UINT32 reg;
    struct
    {
        LSA_UINT32 dma_en : 1;
        LSA_UINT32 sw_reset : 1;
        LSA_UINT32 reserved0 : 13;
        LSA_UINT32 jc_reset : 1;
        LSA_UINT32 err_int_en : 1;
        LSA_UINT32 reserved1 : 7;
        LSA_UINT32 list_size : 8;
    } bits;
}gdma_main_ctrl_reg_t;

typedef union
{
    LSA_UINT32 reg;
    struct
    {
        LSA_UINT32 sw_job_start : 1;
        LSA_UINT32 job_en : 1;
        LSA_UINT32 hw_job_start_en : 1;
        LSA_UINT32 hw_flow_en : 1;
        LSA_UINT32 intr_en : 1;
        LSA_UINT32 job_reset : 1;
        LSA_UINT32 reserved0 : 2;
        LSA_UINT32 hw_select : 6;
        LSA_UINT32 reserved1 : 2;
        LSA_UINT32 job_prio : 5;
        LSA_UINT32 reserved2 : 3;
        LSA_UINT32 transfer_ptr : 8;
    } bits;
}gdma_jobx_ctrl_reg_t;

typedef union
{
    LSA_UINT32 reg;
    struct
    {
        LSA_UINT16 tr_count;
        LSA_UINT32 reserved0 : 6;
        LSA_UINT32 esize : 2;
        LSA_UINT32 reserved1 : 6;
        LSA_UINT32 en_dma_ack : 1;
        LSA_UINT32 last_tr : 1;
    } bits;
}gdma_trans_recx_cnt_reg_t;

typedef union
{
    LSA_UINT32 reg;
    struct
    {
        LSA_UINT32 act_job : 5;
        LSA_UINT32 act_job_val : 1;
        LSA_UINT32 reserved : 26;
    } bits;
}gdma_actual_status_reg_t;

/*
 * macros to handle reentrance behaviour. Prevent a second call to GDMA_CFG_ENTER() that may occur if functions are used nestedly.
 */
#define GDMA_ENTER()    {                                   \
                        if(gdma_data.gdma_semaphore == 0)   \
                        {                                   \
                            GDMA_CFG_ENTER();               \
                        }                                   \
                        gdma_data.gdma_semaphore++;         \
                        }
#define GDMA_EXIT()     {                                   \
                        gdma_data.gdma_semaphore--;         \
                        if(gdma_data.gdma_semaphore == 0)   \
                        {                                   \
                            GDMA_CFG_EXIT();                \
                        }                                   \
                        }
#define GDMA_EXIT_RETURN(x)     GDMA_EXIT(); return(x)

/*
 * error handling
 */
#ifdef GDMA_CFG_FATAL_ERROR_ON_FAILURE
    #define GDMA_ASSERT(condition, error_code)  if(!(condition))                                                    \
                                                {                                                                   \
                                                   gdma_in_fatal_error(GDMA_MODULE_GDMA, __LINE__, (LSA_UINT16) error_code);     \
                                                }
#else
    #define GDMA_ASSERT(condition, error_code)  if(!(condition))                                                    \
                                                {                                                                   \
                                                   GDMA_EXIT_RETURN(error_code);                                    \
                                                }
#endif

/**
 * component version constants
 */
#define GDMA_LSA_PREFIX                         "   -GDMA "
#define GDMA_KIND                               /* &K */ 'V'  /* K&                */
                                                /* preliminary: 'R': release       */
                                                /*              'C': correction    */
                                                /*              'S': spezial       */
                                                /*              'T': test          */
                                                /*              'B': labor         */
                                                /* prereleased: 'P': pilot         */
                                                /* released:    'V': version       */
                                                /*              'K': correction    */
                                                /*              'D': demonstration */
#define GDMA_VERSION                            /* &V */ 0    /* V& */ /* [1 - 99] */
#define GDMA_DISTRIBUTION                       /* &D */ 1    /* D& */ /* [0 - 99] */
#define GDMA_FIX                                /* &F */ 4    /* F& */ /* [0 - 99] */
#define GDMA_HOTFIX                             /* &H */ 0    /* H& */ /* [0]      */
#define GDMA_PROJECT_NUMBER                     /* &P */ 0    /* P& */ /* [0 - 99] */
                                                              /* At LSA always 0!  */
#define GDMA_INCREMENT                          /* &I */ 0    /* I& */ /* [1 - 99] */
#define GDMA_INTEGRATION_COUNTER                /* &C */ 0    /* C& */ /* [1 - 99] */
#define GDMA_GEN_COUNTER                        /* &G */ 0    /* G& */ /* [1]      */

#endif /* GDMA_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
