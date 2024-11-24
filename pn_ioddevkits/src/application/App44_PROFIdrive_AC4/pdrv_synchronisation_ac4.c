/*********************************************************************************************************************/
/**@file        pdrv_synchronisation_ac4.c
 * @brief       PROFIdrive synchronisation mechanisms
 *
 * @author      Siemens AG
 * @copyright   Copyright (C) Siemens AG 2017. All rights reserved.
 *              This program is protected by German copyright law and international treaties.
 *              The use of this software including but not limited to its Source Code is subject to restrictions as
 *              agreed in the license agreement between you and Siemens.
 *              Copying or distribution is not allowed unless expressly permitted according to your license agreement
 *              with Siemens.
 *
 * @version     V1.0
 *
 * @details     Project: PROFIdrive Application Example<br>
 *              Shorthand symbol: Syn (SYNchronisation)<br>
 *              The file implements the PROFIdrive synchronisation mechanism.
 *
 * @internal
 * @note        Doxygen comments are used for automatic documentation generation.
 *              For further information about Doxygen please look at http://www.stack.nl/~dimitri/doxygen/index.html
 *              IDE Eclipse's folding feature is used too. see https://www.eclipse.org
 * @endinternal
*/
/*********************************************************************************************************************/

/*------------  header includes  ------------*/
#include "iod_cfg.h"
#include "usriod_cfg.h" /* for definition of EXAMPL_DEV_CONFIG_VERSION needed */

#include "pdrv_types_ac4.h"  /* PROFIdrive datatype definitions */
#include "pdrv_synchronisation_ac4.h"
#include "pdrv_setpointchannel_ac4.h"
#include "pdrv_statemachine_ac4.h"
#include "pdrv_application_ac4.h"
#include "pdrv_diagnostics_ac4.h"
#include "pdrv_parmanager_ac4.h"
#include "pdrv_pardatabase_ac4.h"

#if (EXAMPL_DEV_CONFIG_VERSION == 44)

/*------------  extern  functions  ------------*/
extern PDRV_BOOL bPdrvUsr_IsIsoActiv(PDRV_VOID);
extern PDRV_BOOL bPdrvUsr_IsIsoReq(PDRV_VOID);
extern PDRV_BOOL bPdrvUsr_IsSyncedPLL(PDRV_VOID);
extern PDRV_UINT16 uPdrvUsr_GetIsoCacf(PDRV_VOID);

/*------------  extern  data  ------------*/

/*------------  type definitions, constants, enums  ------------*/
#define LS_BIT_MSK  (0xF000U)   /**< sign of life bit mask */
#define LS_MIN      (0x1000U)   /**< sign of life minimal value */
#define LS_MAX      (0xF000U)   /**< sign of life maximal value */
#define LS_STEP     (0x1000U)   /**< sign of life step */

#define LS_FAIL_STEPUP (10UL)   /**< step up value if a sign of life error occurs */

/*------------  static  data  ------------*/
static PDRV_UINT16 m_uPDRVNoCLSFaults = 5U; /**< tolerated number of controller life sign failures, see PNU00925 */

static PDRV_SYNCSTATES_TYPE m_eStateSignOfLife = SYNCSTATE_IDLE; /**< state of controller sign of life synchronisation */
static PDRV_UINT m_uCACF;                   /**< controller application cycle factor */
static PDRV_UINT m_uCyclCnt;                /**< cycle counter for the position inside the controller application cycle */
static PDRV_UINT32 m_uFailCnt;              /**< failure counter of controller sign of life errors */
static PDRV_UINT16 m_uSignOfLife = 0U;      /**< last controllers sign of life */

/*------------  public data  ------------*/

/*------------  functions  ------------*/
static PDRV_UINT16 uCalcSignOfLife(PDRV_UINT16 p_uSignOfLife);

/** calc a new sign of life
 *  @details    see PDRV V4.2 chapter 6.3.12
 *  @return     new sign of life
*/
static PDRV_UINT16 uCalcSignOfLife(PDRV_UINT16 p_uSignOfLife)
{
    return ((p_uSignOfLife & LS_BIT_MSK) == LS_MAX) ? LS_MIN : p_uSignOfLife + LS_STEP;
}

/** State of sign-of-life synchronisation
 *  @details
 *  @return     state of sign-of-life synchronisation
*/
PDRV_SYNCSTATES_TYPE ePdrvSyn_GetSignOfLifeState(PDRV_VOID)
{
    return m_eStateSignOfLife;
}

/** Get fail counter of sign-of-life synchronisation
 *  @details
 *  @return     fail counter of sign-of-life synchronisation
*/
PDRV_UINT32 uPdrvSyn_GetFailCnt(PDRV_VOID)
{
    return m_uFailCnt;
}

/** Get the tolerated number of controller life sign failures
 *  @details    see PDRV V4.2 chapter 6.4.1.1 "Life sign monitoring"
 *  @return     value of parameter PNU00925
*/
PDRV_UINT16 uPdrvSyn_GetNoCLSFaults(PDRV_VOID)
{
    return m_uPDRVNoCLSFaults;
}

/** Checks the Sign of Life of STW2 and calcs the Sign of Life for ZSW2
 *  @details    none
 *  @return     none
*/
PDRV_VOID PdrvSyn_CheckSignOfLife(PDRV_VOID)
{
    PDRV_UINT16 uSignOfLife = uPdrvSpc_GetStw2() & LS_BIT_MSK;     /* get received controller sign of life */
    PDRV_SYNCSTATES_TYPE eStateSignOfLife = m_eStateSignOfLife;
    PDRV_BOOL bIsSyncedPLL = bPdrvUsr_IsSyncedPLL();

    /* No PLL synchronisation? */
    if (   (   (!bIsSyncedPLL)
            && (eStateSignOfLife < SYNCSTATE_OPERATION)
           )
        || (bPdrvUsr_IsIsoActiv() == PDRV_FALSE)
        || (bPdrvUsr_IsIsoReq() == PDRV_FALSE)
       )
    {  /* PLL is not synchronous. A new synchronisation is needed */
        eStateSignOfLife = SYNCSTATE_IDLE;
    }
    else
    {  /* PLL is synchronous. */
        switch (eStateSignOfLife)
        {
           case SYNCSTATE_IDLE:      /* waiting for change of controller sign of life */
           {
               /* Change of controller sign of life? */
               if (   (uSignOfLife != 0U)
                   && (uSignOfLife != m_uSignOfLife)
                  )
               {
                   m_uCyclCnt = 0U;
                   m_uCACF = 0U;
                   m_uFailCnt = 0UL;
                   eStateSignOfLife = SYNCSTATE_CSYNCING00;
               }
               break;
           }
           case SYNCSTATE_CSYNCING00:  /* controller application cycle */
           {
               m_uCyclCnt++;     /* increment cycle counter */

               /* 2nd change of controller sign of life? */
               if (uSignOfLife != m_uSignOfLife)
               {
                   /* Is received sign of life equal with expected sign of life
                    * AND counted cycles equal with expected CACF? */
                   if (   (uSignOfLife == uCalcSignOfLife(m_uSignOfLife))
                       && (m_uCyclCnt == uPdrvUsr_GetIsoCacf())
                      )
                   {
                       m_uCACF = m_uCyclCnt;
                       m_uCyclCnt = 0U;
                       eStateSignOfLife = SYNCSTATE_CSYNCING01;
                   }
                   else
                   {   /* wrong sign of life */
                       eStateSignOfLife = SYNCSTATE_IDLE;
                   }
               }
               else
               {
                   /* too many cycle counts? */
                   if (m_uCyclCnt > PDRV_ISO_CACF_MAX)
                   {
                       eStateSignOfLife = SYNCSTATE_IDLE;
                   }
               }
               break;
           }
           case SYNCSTATE_CSYNCING01:      /* 16 valid controller signs of life */
           case SYNCSTATE_CSYNCING02:
           case SYNCSTATE_CSYNCING03:
           case SYNCSTATE_CSYNCING04:
           case SYNCSTATE_CSYNCING05:
           case SYNCSTATE_CSYNCING06:
           case SYNCSTATE_CSYNCING07:
           case SYNCSTATE_CSYNCING08:
           case SYNCSTATE_CSYNCING09:
           case SYNCSTATE_CSYNCING10:
           case SYNCSTATE_CSYNCING11:
           case SYNCSTATE_CSYNCING12:
           case SYNCSTATE_CSYNCING13:
           case SYNCSTATE_CSYNCING14:
           case SYNCSTATE_CSYNCING15:
           case SYNCSTATE_DSYNCING00:      /* 16 valid drive object signs of life */
           case SYNCSTATE_DSYNCING01:
           case SYNCSTATE_DSYNCING02:
           case SYNCSTATE_DSYNCING03:
           case SYNCSTATE_DSYNCING04:
           case SYNCSTATE_DSYNCING05:
           case SYNCSTATE_DSYNCING06:
           case SYNCSTATE_DSYNCING07:
           case SYNCSTATE_DSYNCING08:
           case SYNCSTATE_DSYNCING09:
           case SYNCSTATE_DSYNCING10:
           case SYNCSTATE_DSYNCING11:
           case SYNCSTATE_DSYNCING12:
           case SYNCSTATE_DSYNCING13:
           case SYNCSTATE_DSYNCING14:
           case SYNCSTATE_DSYNCING15:
           {
               PDRV_UINT16 uExpectedSignOfLife = m_uSignOfLife;

               /* Is there a new controller application cycle? */
               if (++m_uCyclCnt == m_uCACF)
               {   /* calc new expected controller sign of life */
                   uExpectedSignOfLife = uCalcSignOfLife(uExpectedSignOfLife);
                   m_uCyclCnt = 0;   /* reset cycle counter */
               }

               /* Is received sign of life equal with expected sign of life? */
               if (uSignOfLife == uExpectedSignOfLife)
               {
                   /* Is there a new controller application cycle OR syncing drive object sign of life? */
                   if (   (m_uCyclCnt == 0)
                       || (eStateSignOfLife > SYNCSTATE_CSYNCING15)
                      )
                   {
                       eStateSignOfLife++;
                   }
               }
               else
               {
                   eStateSignOfLife = SYNCSTATE_IDLE;
               }
               break;
           }
           case SYNCSTATE_OPERATION:   /* Synchronised operation */
           {
               PDRV_UINT16 uExpectedSignOfLife = m_uSignOfLife;   /* expected controller sign of life */

               /* Is there a new controller application cycle? */
               if (++m_uCyclCnt == m_uCACF)
               {   /* calc new expected controller sign of life */
                   uExpectedSignOfLife = uCalcSignOfLife(uExpectedSignOfLife);
                   m_uCyclCnt = 0;   /* reset cycle counter */
               }

               /* Is received sign of life equal with expected sign of life? */
               if (   (uSignOfLife == uExpectedSignOfLife)
                   && bIsSyncedPLL
                  )
               {   /* Controller sign of life is synchronised. */
                   if (m_uFailCnt > 0UL)
                   {   /* decrement fail counter */
                       m_uFailCnt--;
                   }
               }
               else
               {
                   m_uFailCnt += LS_FAIL_STEPUP;    /* increment fail counter */

                   /* Too many failures? */
                   if (   (m_uPDRVNoCLSFaults != PDRV_SYNC_LS_OFF)
                       && (m_uFailCnt > (LS_FAIL_STEPUP * m_uPDRVNoCLSFaults))
                      )
                   {   /* no synchronisation */
                       eStateSignOfLife = SYNCSTATE_IDLE;
                       PdrvDiag_SetFaultMsg(FAULT_SIGNOFLIFE, 0U);
                   }
                   else
                   {   /* still in synchronisation */
                       uSignOfLife = uExpectedSignOfLife;
                   }
               }
               break;
           }
           default:
               eStateSignOfLife = SYNCSTATE_IDLE;
               break;
        }
    }

    m_eStateSignOfLife = eStateSignOfLife;      /* new synchronisation state */
    m_uSignOfLife = uSignOfLife;                /* new controller sign of life */

    /* Handling of drive object sign of life */
    uSignOfLife = 0U;                           /* reset drive object sign of life */

    /* Is controller sign of life synchronised? */
    if (eStateSignOfLife > SYNCSTATE_CSYNCING15)
    {  /* calc the next sign of life */
        uSignOfLife = uPdrvSpc_GetZsw2() & LS_BIT_MSK;
        uSignOfLife = uCalcSignOfLife(uSignOfLife);
    }
    PdrvSpc_SetZsw2(uSignOfLife, LS_BIT_MSK);   /* new drive object sign of life */
}

/** PROFIdrive read function for parameter PNU00925 "max. C-LS faults"
 *  @details    none
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00925
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->o2[0U] = m_uPDRVNoCLSFaults;
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive write function for parameter PNU00925 "max. C-LS faults"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_WfPnu00925
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to write/read */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    m_uPDRVNoCLSFaults = p_ptValues->o2[0];
    return PDRV_EV1_NOERROR;
}


#endif
