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
/*  F i l e               &F: pndv_list.h                               :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*  internal arrangements                                                    */
/*                                                                           */
/*****************************************************************************/
#ifndef PNDV_LIST_H
#define PNDV_LIST_H

#ifdef __cplusplus                       /* If C++ - compiler: Use C linkage */
extern "C"
{
#endif


/*****************************************************************************/
/* contents:

    - internal state machine defines
    - internal error sub-module defines
    - comparing operations
    - dummy operations

*/
/*****************************************************************************/
/* reinclude protection */


/*----------------------------------------------------------------------------*/

#define PNDV_LIST_INITIALIZE(ListHead_) {\
    (ListHead_)->Flink = (ListHead_)->Blink = (ListHead_);\
    }

#define PNDV_LIST_IS_EMPTY(ListHead_) (\
    LSA_HOST_PTR_ARE_EQUAL((ListHead_)->Flink, (ListHead_))\
    )

#define PNDV_LIST_REMOVE_ENTRY(Entry_) {\
    PNDV_LIST_ENTRY_PTR_TYPE EX_Entry_;\
    EX_Entry_ = (Entry_);\
    EX_Entry_->Blink->Flink = EX_Entry_->Flink;\
    EX_Entry_->Flink->Blink = EX_Entry_->Blink;\
    }

#define PNDV_LIST_REMOVE_HEAD(ListHead_, Entry_, Type_) {\
    (Entry_) = (Type_)((ListHead_)->Flink);\
    PNDV_LIST_REMOVE_ENTRY((ListHead_)->Flink);\
    }

#define PNDV_LIST_INSERT_HEAD(ListHead_, Entry_) {\
    (Entry_)->Flink = (ListHead_)->Flink;\
    (Entry_)->Blink = (ListHead_);\
    (ListHead_)->Flink->Blink = (Entry_);\
    (ListHead_)->Flink = (Entry_);\
    }

#define PNDV_LIST_INSERT_TAIL(ListHead_, Entry_) {\
    (Entry_)->Flink = (ListHead_);\
    (Entry_)->Blink = (ListHead_)->Blink;\
    (ListHead_)->Blink->Flink = (Entry_);\
    (ListHead_)->Blink = (Entry_);\
    }

#define PNDV_LIST_INSERT_BEFORE(Existing_, Entry_) {\
    PNDV_LIST_INSERT_TAIL (Existing_, Entry_);\
    }

#define PNDV_LIST_FIRST(ListHead_, Type_) (\
    (Type_)(PNDV_LIST_IS_EMPTY(ListHead_) ? LSA_NULL : (ListHead_)->Flink)\
    )

#define PNDV_LIST_NEXT(ListHead_, Entry_, Type_) (\
    (Type_)(LSA_HOST_PTR_ARE_EQUAL((Entry_)->Flink, (ListHead_)) ? LSA_NULL : (Entry_)->Flink)\
    )

#define PNDV_LIST_PREV_OR_HEAD(Entry_, Type_) (\
    (Type_)((Entry_)->Blink)\
    )

#define PNDV_LIST_LENGHT(ListHead_, LenPtr_) {\
    PNDV_LIST_ENTRY_PTR_TYPE el = PNDV_LIST_FIRST (ListHead_, PNDV_LIST_ENTRY_PTR_TYPE);\
    *(LenPtr_) = 0;\
    while (! LSA_HOST_PTR_ARE_EQUAL(el,LSA_NULL)) {\
        *(LenPtr_) += 1;\
        el = PNDV_LIST_NEXT (ListHead_, el, PNDV_LIST_ENTRY_PTR_TYPE);\
    }}

#define PNDV_LIST_APPEND(ListHead_, ListHeadToAppend_) {\
    if (! PNDV_LIST_IS_EMPTY (ListHeadToAppend_)) {\
        (ListHead_)->Blink->Flink = (ListHeadToAppend_)->Flink;\
        (ListHeadToAppend_)->Flink->Blink = (ListHead_)->Blink;\
        (ListHead_)->Blink = (ListHeadToAppend_)->Blink;\
        (ListHead_)->Blink->Flink = (ListHead_);\
        PNDV_LIST_INITIALIZE (ListHeadToAppend_);\
    }}


#define PNDV_LIST_LAST(ListHead_, Type_) (\
    (Type_)(PNDV_LIST_IS_EMPTY(ListHead_) ? LSA_NULL : (ListHead_)->Blink)\
    )

/*----------------------------------------------------------------------------*/

/* usage: for (PndvListForeach(...)) { ... } -- variant 1 */
#define PNDV_LIST_FOREACH(Var_, ListHead_, Type_) \
        (Var_) = PNDV_LIST_FIRST (ListHead_, Type_); \
        ! LSA_HOST_PTR_ARE_EQUAL((Var_), LSA_NULL); \
        (Var_) = PNDV_LIST_NEXT (ListHead_, (PNDV_LIST_ENTRY_PTR_TYPE)(Var_), Type_)

/* usage: for( PndvListEach(...) ) { ... } -- variant 2 */
#define PNDV_LIST_EACH(Var_, ListHead_, Type_) \
    (Var_) = PNDV_LIST_FIRST(ListHead_, Type_); \
    ! LSA_HOST_PTR_ARE_EQUAL((Var_), LSA_NULL); \
    (Var_) = PNDV_LIST_NEXT(ListHead_, (PNDV_LIST_ENTRY_PTR_TYPE)(Var_), Type_)

/*----------------------------------------------------------------------------*/

typedef  LSA_BOOL  /* return (ListEntry->field < NewEntry->field); */
pndv_list_cmp_function_type (
    PNDV_LIST_ENTRY_PTR_TYPE  ListEntry,
    PNDV_LIST_ENTRY_PTR_TYPE  NewEntry
    );
/* naming convention: pndv_list_cmp__<struct>__<field> */


/*----------------------------------------------------------------------------*/

#ifdef __cplusplus  /* If C++ - compiler: End of C linkage */
}
#endif

#endif  /* of PNDV_LIST_H */

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
