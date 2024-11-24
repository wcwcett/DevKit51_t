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
/*  F i l e               &F: pnpb_lib_mem_int.h                        :F&  */
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

#ifndef APP1_STANDART_PNPB_LIB_MEM_INT_H_
#define APP1_STANDART_PNPB_LIB_MEM_INT_H_



#define PNPB_XHIF_ACYCLIC_TELEGRAM_HEADER_SIZE          4   /* 2B ID + 2B prm_size */
#define PNPB_XHIF_ACYCLIC_DATA_MAX                      (PNPB_XHIF_NUMOF_BYTES_PER_ACYCLIC_TELEGRAM - PNPB_XHIF_ACYCLIC_TELEGRAM_HEADER_SIZE)

/* These data are based on addresses in Ertec, set by linkerscript, visible in *.map file */
#define PNPB_XHIF_SECTION_CYC_IN                        0x21000000
#define PNPB_XHIF_SECTION_CYC_OUT                       0x21020000
#define PNPB_XHIF_SECTION_ACY_IN                        0x21040000
#define PNPB_XHIF_SECTION_ACY_OUT                       0x21080000
#define PNPB_XHIF_SECTION_TRACES                        0x210C0000

/* Data for one subslot as will be exchanged by memory interface */
typedef volatile struct pnpb_xhif_subslot_data
{
    PNIO_UINT32 Lock;
    PNIO_UINT32 Slot;
    PNIO_UINT32 Subslot;
    PNIO_UINT32 IOxS;
    PNIO_UINT32 Data_size;
    PNIO_UINT8  Data[PNPB_XHIF_NUMOF_BYTES_PER_SUBSLOT];
}pnpb_xhif_subslot_data;


/* Each buffer have to have read and write pointer - cyclic buffer realization */
typedef volatile struct pnpb_xhif_buffer_services
{
    PNIO_UINT32 read_ptr;
    PNIO_UINT32 write_ptr;
    /* For alignment */
    PNIO_UINT32 reserved[2];
}pnpb_xhif_buffer_services;


/* Any acyclic telegram as will be exchanged by memory interface */
typedef volatile struct pnpb_xhif_acyclic
{
    PNIO_UINT32 lock;
    PNIO_UINT32 id;
    PNIO_UINT32 data_len;
    PNIO_UINT8  data[PNPB_XHIF_NUMOF_BYTES_PER_ACYCLIC_TELEGRAM];
}pnpb_xhif_acyclic;

/* Structure and shortcuts for cyclical input buffer access */
typedef volatile struct pnpb_xhif_cyclic_in
{
    pnpb_xhif_buffer_services  pnpb_xhif_cyclic_in_services;
    pnpb_xhif_subslot_data     pnpb_xhif_cyclic_in [PNPB_XHIF_SIZE_OF_BUFFER_FOR_CYCLIC];
}pnpb_xhif_cyclic_in;

typedef pnpb_xhif_cyclic_in * pnpb_xhif_cyclic_in_ptr;
pnpb_xhif_cyclic_in_ptr  pnpb_xhif_cyclic_in_p;

#define PNPB_XHIF_CYCLIC_IN_SERV      pnpb_xhif_cyclic_in_p->pnpb_xhif_cyclic_in_services
#define PNPB_XHIF_CYCLIC_IN_DATA      pnpb_xhif_cyclic_in_p->pnpb_xhif_cyclic_in

/* Structure and shortcuts for cyclical output buffer access */
typedef volatile struct pnpb_xhif_cyclic_out
{
    pnpb_xhif_buffer_services  pnpb_xhif_cyclic_out_services;
    pnpb_xhif_subslot_data     pnpb_xhif_cyclic_out[PNPB_XHIF_SIZE_OF_BUFFER_FOR_CYCLIC];
}pnpb_xhif_cyclic_out;

typedef pnpb_xhif_cyclic_out * pnpb_xhif_cyclic_out_ptr;
pnpb_xhif_cyclic_out_ptr  pnpb_xhif_cyclic_out_p;

#define PNPB_XHIF_CYCLIC_OUT_SERV      pnpb_xhif_cyclic_out_p->pnpb_xhif_cyclic_out_services
#define PNPB_XHIF_CYCLIC_OUT_DATA      pnpb_xhif_cyclic_out_p->pnpb_xhif_cyclic_out

/* Structure and shortcuts for acyclic input buffer access */
typedef volatile struct pnpb_xhif_acyc_in
{
    pnpb_xhif_buffer_services   pnpb_xhif_acyc_in_services;
    pnpb_xhif_acyclic           pnpb_xhif_acyc_in [PNPB_XHIF_SIZE_OF_BUFFER_FOR_ACYC];
}pnpb_xhif_acyc_in;

typedef pnpb_xhif_acyc_in * pnpb_xhif_acyc_in_ptr;
pnpb_xhif_acyc_in_ptr  pnpb_xhif_acyc_in_p;

#define PNPB_XHIF_ACYC_IN_SERV      pnpb_xhif_acyc_in_p->pnpb_xhif_acyc_in_services
#define PNPB_XHIF_ACYC_IN_DATA      pnpb_xhif_acyc_in_p->pnpb_xhif_acyc_in

/* Structure and shortcuts for acyclic output buffer access */
typedef volatile struct pnpb_xhif_acyc_out
{
    pnpb_xhif_buffer_services   pnpb_xhif_acyc_out_services;
    pnpb_xhif_acyclic           pnpb_xhif_acyc_out[PNPB_XHIF_SIZE_OF_BUFFER_FOR_ACYC];
}pnpb_xhif_acyc_out;

typedef pnpb_xhif_acyc_out * pnpb_xhif_acyc_out_ptr;
pnpb_xhif_acyc_out_ptr pnpb_xhif_acyc_out_p;

#define PNPB_XHIF_ACYC_OUT_SERV      pnpb_xhif_acyc_out_p->pnpb_xhif_acyc_out_services
#define PNPB_XHIF_ACYC_OUT_DATA      pnpb_xhif_acyc_out_p->pnpb_xhif_acyc_out

/* Base address of traces */
volatile PNIO_UINT8* pnpb_xhif_traces_p;

/* Management of receiving acyclical message, which is carried by several telegrams of memory interface */
typedef struct pnpb_xhif_acyclic_recieve_continuous
{
    PNIO_UINT32 more_follows;
    PNIO_INT32  data_remains;
    PNIO_UINT8  *p_start_data;
    PNIO_UINT8  *p_write_data;
}pnpb_xhif_acyclic_recieve_continuous;
pnpb_xhif_acyclic_recieve_continuous pnpb_xhif_acyclic_recieve_cont_manage;


typedef struct pnpb_xhif_function_calls
{
    PNPB_XHIF_ACYC_TELEGRAMS   id;
    PNIO_VOID                   (*function_call) (PNIO_UINT8*);
}pnpb_xhif_function_calls;
pnpb_xhif_function_calls pnpb_xhif_acyclic_functions[PNPB_XHIF_ACYC_NUM_OF_TELEGRAMS];

PNIO_UINT32 pnpb_xhif_IO_buffer_init(PNIO_SUB_LIST_ENTRY* pIoSubList,
                                    PNIO_UINT32          NumOfSubListEntries);
PNIO_VOID pnpb_xhif_memory_interface_start(PNIO_VOID* p_xhif_set_page, PNIO_VOID* p_xhif_data);
PNIO_UINT32 pnpb_xhif_send_all_IO_data();
PNIO_UINT32 pnpb_xhif_cyclical_read();
PNIO_VOID pnpb_xhif_prepare_function_calls();


PNIO_UINT32 pnpb_xhif_cyclical_write(PNIO_UINT32 submodule_no,
                                    PNIO_UINT32 slot_no,
                                    PNIO_UINT32 subslot_no,
                                    PNIO_IOXS Iocs,
                                    PNIO_UINT32 DataLen,
                                    PNIO_UINT8 *pData);

PNIO_VOID pnpb_xhif_acyc_write(PNPB_XHIF_ACYC_TELEGRAMS id, PNIO_UINT16 prm_len, PNIO_UINT8 *p_prm);
PNIO_UINT32 pnpb_xhif_acyc_read();


PNIO_VOID pnpb_xhif_no_telegram(PNIO_UINT8* params);
PNIO_VOID pnpb_xhif_device_ready(PNIO_UINT8* params);

PNIO_VOID pnpb_xhif_wait( PNIO_VOID );

#endif /* APP1_STANDART_PNPB_LIB_MEM_INT_H_ */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
