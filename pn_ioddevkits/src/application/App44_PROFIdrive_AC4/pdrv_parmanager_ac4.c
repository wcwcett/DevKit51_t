/*********************************************************************************************************************/
/**@file        pdrv_parmanager_ac4.c
 * @brief       PROFIdrive Parameter Manager
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
 *              Shorthand symbol: PAR (PARameter)<br>
 *              The Parameter Manager implements the PROFIdrive parameter access.
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
#include "pniousrd.h"   /* for definitions of ArType needed */

#include "pdrv_types_ac4.h"  /* PROFIdrive datatype definitions */
#include "pdrv_parmanager_ac4.h"
#include "pdrv_pardatabase_ac4.h"

#if (EXAMPL_DEV_CONFIG_VERSION == 44)

/*------------  extern  functions  ------------*/

/*------------  extern  data  ------------*/

/*------------  type definitions, constants, enums  ------------*/

#define PAR_NAMELEN (16U)   /**< name length with 16 valid characters without end of string character "\0" */
#define PARCXN_NR   (5U)    /**< number of possible parameter connections at the same time
                                 you have to guarantee at least one controller and one supervisor access */
#define IOCTRL_I    (0U)    /**< m_tParCxnDatas[] index for the IO Controller connection */
#define SPRVSR_I    (PARCXN_NR - 1U)    /**< m_tParCxnDatas[] index for the Supervisor connection */

/** @name Base mode parameter request
  * :  structure of parameter request as byte indizes (Endianess) see PDRV V4.2 table 24, 25
  * @{
*/
#define PAR_HEADERSIZE   (4U)   /**< header size of a PDRV parameter request/response */
#define PAR_I_RQREF      (0U)   /**< index of request reference in request/response header */
#define PAR_I_RQID       (1U)   /**< index of request ID in request/response header */
#define PAR_I_DOID       (2U)   /**< index of drive object ID in request header/response */
#define PAR_I_NRPAR      (3U)   /**< index of number of parameters in request/response header */
#define PAR_I_REQ_ATTR   (4U)   /**< index of attribute in request (1st parameter address) */
#define PAR_I_REQ_NRELE  (5U)   /**< index of number of elements (1st parameter address) */
#define PAR_I_REQ_PNUHI  (6U)   /**< index of PNU MSB in request (1st parameter address) */
#define PAR_I_REQ_PNULO  (7U)   /**< index of PNU LSB in request (1st parameter address) */
#define PAR_I_REQ_SUBHI  (8U)   /**< index of subindex MSB in request (1st parameter address) */
#define PAR_I_REQ_SUBLO  (9U)   /**< index of subindex LSB in request (1st parameter address) */
#define PAR_I_REQ_FMT   (10U)   /**< index of format in request (single parameter change request) */
#define PAR_I_REQ_NRVAL (11U)   /**< index of number of values in request (single parameter change request) */
#define PAR_I_REQ_VAL   (12U)   /**< first index of values in request (single parameter change request) */
#define PAR_I_RSP_FMT    (4U)   /**< index of format in response (1st parameter value) */
#define PAR_I_RSP_NRVAL  (5U)   /**< index of number of values in response (1st parameter value) */
#define PAR_I_RSP_VAL    (6U)   /**< first index of values in response (1st parameter value) */
/* @} */

/** @name Coding of description index
  * : see PDRV V4.2 table 17
  * @{
*/
#define PDRV_DESCI_COMPLETE          (0U)   /**< complete description */
#define PDRV_DESCI_IDENTIFIER        (1U)   /**< identifier */
#define PDRV_DESCI_NROFELEMENTS      (2U)   /**< number of array elements or lenght of string */
#define PDRV_DESCI_STDFACTOR         (3U)   /**< standardisation factor */
#define PDRV_DESCI_VARATTRIB         (4U)   /**< variable attribute */
#define PDRV_DESCI_RESERVED1         (5U)   /**< reserved */
#define PDRV_DESCI_NAME              (6U)   /**< name */
#define PDRV_DESCI_LOLIMIT           (7U)   /**< low limit */
#define PDRV_DESCI_HILIMIT           (8U)   /**< high limit */
#define PDRV_DESCI_RESERVED2         (9U)   /**< reserved */
#define PDRV_DESCI_IDEXTENSION      (10U)   /**< ID extension is reserved */
#define PDRV_DESCI_REFPAR           (11U)   /**< DO IO DATA reference parameter */
#define PDRV_DESCI_NORMALISATION    (12U)   /**< DO IO DATA normalisation */
/* @} */

/** @name parameter codings
  * : coding of the fields in parameter request/response of Base Mode Parameter Access see PDRV V4.2 table 31
  * @{
*/
#define PDRV_PARCD_REQUEST  (0x01U)   /**< request parameter */
#define PDRV_PARCD_CHANGE   (0x02U)   /**< change parameter */
#define PDRV_PARCD_UNSUP    (0x80U)   /**< service not supported */
#define PDRV_PARCD_NEGRSP   (0x80U)   /**< negative response mask */
/* @} */

/** @name PROFIdrive request attribute
  * : see PDRV V4.2 table 31 column "Attribute"
  * @{
*/
#define PDRV_ATTR_VALUE (0x10U)     /**< attribute Value */
#define PDRV_ATTR_DESC  (0x20U)     /**< attribute Description */
#define PDRV_ATTR_TEXT  (0x30U)     /**< attribute Text */
/* @} */

/** @name PROFIdrive formats
  * : see error value 1 PDRV V4.2 table 31 column "Format"
  * @{
*/
#define PDRV_FMT_ZERO   (0x40U)     /**< format Zero */
#define PDRV_FMT_BYTE   (0x41U)     /**< format Byte */
#define PDRV_FMT_WORD   (0x42U)     /**< format Word */
#define PDRV_FMT_DWORD  (0x43U)     /**< format Double Word */
#define PDRV_FMT_ERROR  (0x44U)     /**< format Error */
/* @} */

#define PDRV_SUBI_ILLEGAL   (0xFFFFU)   /**< illegal subindex see PDRV V4.2 table 31 */

/** states of the general state machine for the Parameter Manager Processing see PDRV V4.2 table 33 */
typedef enum
{
    DISCONNECTED,   /**< Connection disconnected */
    CONNECTED_IDLE, /**< Connection established, no process is pending */
    RQ_PROCESSING,  /**< request is being processed */
    RSP_AVAILABLE   /**< response is available */
} PDRV_ParStates;

/** internal representation of a parameter request */
typedef struct
{
    PDRV_UINT32 uBufLen;                    /**< buffer lenght of request */
    PDRV_UINT16 uArType;                    /**< AR type */
    PDRV_UINT16 uArNum;                     /**< AR number */
    PDRV_UINT8 uReq[PDRV_PAR_BLOCKSIZE];    /**< Request buffer [Big Endian] */
    PDRV_UINT8 uRsp[PDRV_PAR_BLOCKSIZE];    /**< Response buffer [Big Endian] */
    PDRV_UINT8 uRspLen;                     /**< length of parameter response */
    PDRV_ParStates eState;                  /**< parameter channel state */
} PDRV_ParCxnData;


/*------------  static  data  ------------*/

static PDRV_ParCxnData m_tParCxnDatas[PARCXN_NR];   /**< parameter connection data for every parameter channel */
static PDRV_UINT8 m_tParReqCnt;  /**< round robin counter for parameter request processing */

static PDRV_UINT8 uParSetError(PDRV_UINT16 p_uErrorValue1, PDRV_UINT16 p_uErrorValue2, PDRV_UINT8 *p_puRsp);
static PDRV_UINT8 uParWrite(const PDRV_PAR_OBJ *p_ptParObj, PDRV_ParCxnData *p_ptParCxnData);
static PDRV_UINT8 uParRead(const PDRV_PAR_OBJ *p_ptParObj, PDRV_ParCxnData *p_ptParCxnData);
static PDRV_UINT8 uParReadDesc(const PDRV_PAR_OBJ *p_ptParObj, PDRV_ParCxnData *p_ptParCxnData);
static PDRV_UINT8 uParReadText(const PDRV_PAR_OBJ *p_ptParObj, PDRV_ParCxnData *p_ptParCxnData);
static PDRV_UINT uParPdrvSizeOfFmt(PDRV_UINT8 p_uFmt);
static PDRV_VOID ParPdrvStrCpy(PDRV_UINT8 * p_uDest, const char * p_cSrc);

/*------------  public data  ------------*/

/** PROFIdrive parameter manager initialization
 *  @details
 *  @return     void
*/
PDRV_VOID PdrvPar_Init(PDRV_VOID)
{
    PDRV_UINT uI;

    for (uI = 0; uI < PARCXN_NR; uI++)
    {
        m_tParCxnDatas[uI].eState = DISCONNECTED;
    }
    m_tParReqCnt = 0U;
}


/** PROFIdrive parameter connection is established
 *  @details
 *  @return     PDRV_FALSE if no unused connection channel is available
*/
PDRV_BOOL bPdrvPar_EstablishCxn
    (PDRV_UINT16  p_uArType,    /**< [in] type of AR (see cm_ar_type_enum) */
     PDRV_UINT16  p_uArNum      /**< [in] AR number */
    )
{
    PDRV_BOOL bIsCxnOk = PDRV_FALSE;
    PDRV_UINT uCnxI = PARCXN_NR;        /* connection index */
    PDRV_UINT uI;

    if (   (p_uArType == PNIO_AR_TYPE_SINGLE)
        || (p_uArType == PNIO_AR_TYPE_SINGLE_RTC3)
        || (p_uArType == PNIO_AR_TYPE_SUPERVISOR)
       )
    {
        /* search for a free parameter channel */
        for (uI = 0; uI < PARCXN_NR; uI++)
        {
            if (   (   (m_tParCxnDatas[uI].uArType == p_uArType)    /* same connection? */
                    && (m_tParCxnDatas[uI].uArNum == p_uArNum)
                   )
                || (   (m_tParCxnDatas[uI].eState == DISCONNECTED)  /* free parameter channel */
                    && (   (   (uI != SPRVSR_I)                     /* possible channel for Controller connection? */
                            && (   (p_uArType == PNIO_AR_TYPE_SINGLE)
                                || (p_uArType == PNIO_AR_TYPE_SINGLE_RTC3)
                               )
                           )
                        || (   (uI != IOCTRL_I)                     /* possible channel for Supervisor connection? */
                            && (p_uArType == PNIO_AR_TYPE_SUPERVISOR)
                           )
                       )
                   )
               )
            {
                uCnxI = uI;
                m_tParCxnDatas[uCnxI].eState = DISCONNECTED;
                break;
            }
        }

        /* new connection? */
        if (uCnxI < PARCXN_NR)
        {
            /* reset datas */
            for (uI = 0U; uI < PDRV_PAR_BLOCKSIZE; uI++)
            {
                m_tParCxnDatas[uCnxI].uReq[uI] = 0U;
                m_tParCxnDatas[uCnxI].uRsp[uI] = 0U;
            }
            m_tParCxnDatas[uCnxI].uArType = p_uArType;
            m_tParCxnDatas[uCnxI].uArNum = p_uArNum;
            m_tParCxnDatas[uCnxI].eState = CONNECTED_IDLE;
            bIsCxnOk = PDRV_TRUE;
        }
    }
    return bIsCxnOk;
}

/** PROFIdrive parameter connection is disconnected
 *  @details
 *  @return     PDRV_FALSE no established connection found
*/
PDRV_BOOL bPdrvPar_DisconnCxn
    (PDRV_UINT16 p_uArNum      /**< [in] AR number */
    )
{
    PDRV_BOOL bIsCxnOk = PDRV_FALSE;
    PDRV_UINT uI;

    /* search for correct parameter channel */
    for (uI = 0; uI < PARCXN_NR; uI++)
    {
        if (m_tParCxnDatas[uI].uArNum == p_uArNum)
        {
            m_tParCxnDatas[uI].eState = DISCONNECTED;
            bIsCxnOk = PDRV_TRUE;
        }
    }

    return bIsCxnOk;
}


/** PROFIdrive parameter connection write request processing
 *  @details    see PDRV V4.2 table 33
 *  @return     Error_Code_1 see PDRV V4.2 table 169
*/
PDRV_UINT8 uPdrvPar_WriteReqCxn
    (PDRV_UINT16 p_uArNum,          /**< [in] AR number */
     PDRV_UINT32 *p_puBufLen,       /**< [in, out] in: length to write, out: length, written by user */
     PDRV_UINT8  *p_puBuffer        /**< [in] buffer pointer */
    )
{
    PDRV_BOOL uErrCode1 = PDRV_EC1_OK;
    PDRV_UINT uCnxI = PARCXN_NR;        /* connection index */
    PDRV_UINT uI;
    PDRV_UINT32 uBufLen = *p_puBufLen;


    /* search for requested parameter channel */
    for (uI = 0; uI < PARCXN_NR; uI++)
    {
        if (m_tParCxnDatas[uI].uArNum == p_uArNum)
        {
            uCnxI = uI;
            break;
        }
    }

    if (   (uCnxI >= PARCXN_NR)                             /* requested parameter channel not found? */
        || (m_tParCxnDatas[uCnxI].eState == DISCONNECTED)   /* wrong states of parameter processing? */
        || (m_tParCxnDatas[uCnxI].eState == RQ_PROCESSING)
       )
    {
        uErrCode1 = PDRV_EC1_ACC_STATE_CONFLICT;
    }
    else if (   (uBufLen > PDRV_PAR_BLOCKSIZE)
             || (uBufLen < PAR_HEADERSIZE)
            )
    {
        uErrCode1 = PDRV_EC1_ACC_WRITE_LEN;
    }
    else
    {   /* ok, copy datas into internal channel buffer */
        for (uI = 0; uI < uBufLen; uI++)
        {
            m_tParCxnDatas[uCnxI].uReq[uI] = *(p_puBuffer + uI);
        }
        for (; uI < PDRV_PAR_BLOCKSIZE; uI++)
        {
            m_tParCxnDatas[uCnxI].uReq[uI] = 0U;
        }
        m_tParCxnDatas[uCnxI].uBufLen = uBufLen;
        m_tParCxnDatas[uCnxI].eState = RQ_PROCESSING;
    }

    return uErrCode1;
}

/** PROFIdrive parameter connection read request processing
 *  @details    see PDRV V4.2 table 33
 *  @return     Error_Code_1 see PDRV V4.2
*/
PDRV_UINT8 uPdrvPar_ReadReqCxn
    (PDRV_UINT16 p_uArNum,          /**< [in] AR number */
     PDRV_UINT32 *p_puBufLen,       /**< [in, out] in: length to write, out: length, written by user */
     PDRV_UINT8  *p_puBuffer        /**< [in] buffer pointer */
    )
{
    PDRV_BOOL uErrCode1 = PDRV_EC1_OK;
    PDRV_UINT uCnxI = PARCXN_NR;        /* connection index */
    PDRV_UINT uI;
    PDRV_UINT32 uBufLen = *p_puBufLen;

    /* search for requested parameter channel */
    for (uI = 0; uI < PARCXN_NR; uI++)
    {
        if (m_tParCxnDatas[uI].uArNum == p_uArNum)
        {
            uCnxI = uI;
            break;
        }
    }

    if (   (uCnxI >= PARCXN_NR)                             /* requested parameter channel not found? */
        || (m_tParCxnDatas[uCnxI].eState == DISCONNECTED)   /* wrong states of parameter processing? */
        || (m_tParCxnDatas[uCnxI].eState == CONNECTED_IDLE)
        || (m_tParCxnDatas[uCnxI].eState == RQ_PROCESSING)
       )
    {
        uErrCode1 = PDRV_EC1_ACC_STATE_CONFLICT;
    }
    else if (uBufLen < m_tParCxnDatas[uCnxI].uRspLen)
    {
        uErrCode1 = PDRV_EC1_ACC_WRITE_LEN;
    }
    else
    {   /* ok, write datas from internal channel buffer into response buffer  */
        for (uI = 0U; uI < m_tParCxnDatas[uCnxI].uRspLen; uI++)
        {
            *(p_puBuffer + uI) = m_tParCxnDatas[uCnxI].uRsp[uI] ;
        }
        for (; uI < uBufLen; uI++)
        {
            *(p_puBuffer + uI) = 0U;
        }
        *p_puBufLen = m_tParCxnDatas[uCnxI].uRspLen;
        m_tParCxnDatas[uCnxI].eState = CONNECTED_IDLE;
    }

    return uErrCode1;
}

/** PROFIdrive parameter request processing
 *  @details    it is called cyclically
 *  @return     PDRV_VOID
*/
PDRV_VOID PdrvPar_ProcessReq (PDRV_VOID)
{
    PDRV_UINT uI;
    PDRV_UINT uCnxI;        /* connection index */
    PDRV_UINT8 uRspLen = PAR_HEADERSIZE;     /* response length at least header size */
    PDRV_ParCxnData *ptParCxnData;

    /* search for next channel which has a processing request; round robin for all channels */
    uI = 0;
    uCnxI = m_tParReqCnt;
    do
    {
        uCnxI++;
        if (uCnxI >= PARCXN_NR)
        {
            uCnxI = 0U;
        }
        uI++;
    } while (   (uI < PARCXN_NR)
             && (m_tParCxnDatas[uCnxI].eState != RQ_PROCESSING));
    m_tParReqCnt = uCnxI;
    ptParCxnData = &m_tParCxnDatas[uCnxI];

    /* channel with "request processing" found? */
    if (ptParCxnData->eState == RQ_PROCESSING)
    {
        PDRV_UINT16 uSubindex =  (ptParCxnData->uReq[PAR_I_REQ_SUBHI] << 8U)
                                + ptParCxnData->uReq[PAR_I_REQ_SUBLO];

        ptParCxnData->uRsp[PAR_I_RQREF] = ptParCxnData->uReq[PAR_I_RQREF];  /* mirror Request Reference */
        ptParCxnData->uRsp[PAR_I_RQID]  = ptParCxnData->uReq[PAR_I_RQID];   /* default response ID is request ID */
        ptParCxnData->uRsp[PAR_I_DOID]  = ptParCxnData->uReq[PAR_I_DOID];   /* mirror DO-ID */
        ptParCxnData->uRsp[PAR_I_NRPAR] = ptParCxnData->uReq[PAR_I_NRPAR];  /* number of parameter */

        /* Request ID illegal, reserved or manufacturer specific? */
        if (   (ptParCxnData->uReq[PAR_I_RQID] != PDRV_PARCD_REQUEST) /* NO Request ID parameter request (read)? */
            && (ptParCxnData->uReq[PAR_I_RQID] != PDRV_PARCD_CHANGE)  /* NO Request ID parameter change (write)? */
           )
        {   /* illegal, reserved or manufacturer specific */
            ptParCxnData->uRsp[PAR_I_RQID] = PDRV_PARCD_UNSUP;       /* response ID: service not supported */
            uRspLen += uParSetError(PDRV_EV1_NOT_SUPPORTED, 0U, &ptParCxnData->uRsp[0]);
        }    /* Invalid DO? */
        else if (   (ptParCxnData->uReq[PAR_I_DOID] != 0U)    /* drive unit representative */
                 && (ptParCxnData->uReq[PAR_I_DOID] != 1U)    /* only DO=1 is valid, maybe changed by vendor */
                )
        { /* no valid DO */
            uRspLen += uParSetError(PDRV_EV1_AXIS, 0U, &ptParCxnData->uRsp[0]);
        }    /* more than 1 parameter request? (multi parameter access is not implemented yet) */
        else if (ptParCxnData->uReq[PAR_I_NRPAR] > 1U)
        { /* no valid number of parameters */
            uRspLen += uParSetError(PDRV_EV1_NO_MULTI_PAR, 0U, &ptParCxnData->uRsp[0]);
        }    /* No parameter? */
        else if (ptParCxnData->uReq[PAR_I_NRPAR] == 0U)
        { /* nothing to do */
        }    /* No supported attribute, number of elements not in range, parameter number "0"? */
        else if (   (   (ptParCxnData->uReq[PAR_I_REQ_ATTR] != PDRV_ATTR_VALUE)
                     && (ptParCxnData->uReq[PAR_I_REQ_ATTR] != PDRV_ATTR_DESC)
                     && (ptParCxnData->uReq[PAR_I_REQ_ATTR] != PDRV_ATTR_TEXT)
                    )
                 || (ptParCxnData->uReq[PAR_I_REQ_NRELE] > (PDRV_PAR_BLOCKSIZE - PAR_I_RSP_VAL))
                 || (   (ptParCxnData->uReq[PAR_I_REQ_PNUHI] == 0U)
                     && (ptParCxnData->uReq[PAR_I_REQ_PNULO] == 0U)
                    )
                )
        {
            uRspLen += uParSetError(PDRV_EV1_PAR_ADDR_IMPERMISS, 0U, &ptParCxnData->uRsp[0]);
        }    /* change of description? */
        else if (   (ptParCxnData->uReq[PAR_I_RQID] == PDRV_PARCD_CHANGE)
                 && (ptParCxnData->uReq[PAR_I_REQ_ATTR] == PDRV_ATTR_DESC)
                )
        { /* description change is not allowed */
            uRspLen += uParSetError(PDRV_EV1_DESC_CHANGE, uSubindex, &ptParCxnData->uRsp[0]);
        }
        else
        { /* all is fine until here */
            const PDRV_PAR_OBJ * ptParObj;
            PDRV_UINT16 uPnu;

            uPnu = ptParCxnData->uReq[PAR_I_REQ_PNULO] + ((PDRV_UINT16)ptParCxnData->uReq[PAR_I_REQ_PNUHI] << 8U);
            ptParObj = ptPdrvPar_GetParObj(uPnu);
            /* parameter number not found? */
            if (ptParObj == PDRV_NULL)
            { /* parameter number was not found */
                uRspLen += uParSetError(PDRV_EV1_PNU, 0U, &ptParCxnData->uRsp[0]);
            }    /* change of text? */
            else if (   (ptParCxnData->uReq[PAR_I_RQID] == PDRV_PARCD_CHANGE)
                     && (ptParCxnData->uReq[PAR_I_REQ_ATTR] == PDRV_ATTR_TEXT)
                    )
            { /* text change is not allowed */
                PDRV_UINT16 uEv1 = (ptParObj->uIdentifier & PDRV_PARID_ADDTEXT) ? PDRV_EV1_PAR_TXT_CHANGE : PDRV_EV1_NO_TEXT_ARRAY;

                uRspLen += uParSetError(uEv1, uSubindex, &ptParCxnData->uRsp[0]);
            }    /* parameter database error? */
            else if (   (ptParObj->pfnRead == PDRV_NULL)
                     || (   (ptParObj->pfnWrite == PDRV_NULL)
                         && ((ptParObj->uIdentifier & PDRV_PARID_READONLY) != PDRV_PARID_READONLY)
                        )
                    )
            {
                uRspLen += uParSetError(PDRV_EV1_NO_OP_PRIO, 0U, &ptParCxnData->uRsp[0]);   /* TODO: manufacturer specific error*/
            }    /* change parameter request for a read only parameter? */
            else if (   (ptParCxnData->uReq[PAR_I_RQID] == PDRV_PARCD_CHANGE)
                     && ((ptParObj->uIdentifier & PDRV_PARID_READONLY) == PDRV_PARID_READONLY)
                    )
            {
                uRspLen += uParSetError(PDRV_EV1_VALUE_CHANG, uSubindex, &ptParCxnData->uRsp[0]);
            }
            else
            { /* parameter was found and no other error */
                if (ptParCxnData->uReq[PAR_I_RQID] == PDRV_PARCD_CHANGE)  /* Request ID parameter change (write)? */
                { /* change response */
                    uRspLen += uParWrite(ptParObj, ptParCxnData);
                }
                else
                { /* read response */
                    if (ptParCxnData->uReq[PAR_I_REQ_ATTR] == PDRV_ATTR_VALUE)
                    { /* read values */
                        uRspLen += uParRead(ptParObj, ptParCxnData);
                    }
                    else if (ptParCxnData->uReq[PAR_I_REQ_ATTR] == PDRV_ATTR_DESC)
                    { /* read description */
                        uRspLen += uParReadDesc(ptParObj, ptParCxnData);
                    }
                    else if (ptParCxnData->uReq[PAR_I_REQ_ATTR] == PDRV_ATTR_TEXT)
                    { /* read text */
                        uRspLen += uParReadText(ptParObj, ptParCxnData);
                    }
                    else
                    {
                        uRspLen += uParSetError(PDRV_EV1_NOT_SUPPORTED, 0U, &ptParCxnData->uRsp[0]);
                    }
                }
            }
        }

        ptParCxnData->uRspLen = uRspLen;
        ptParCxnData->eState = RSP_AVAILABLE;
    }
}

/** PROFIdrive parameter error with or without subindex as error value 2
 *  @details    modifies response buffer for error response with error value1 and subindex (error value depended)
 *  @return     additional response length
*/
static PDRV_UINT8 uParSetError
    (PDRV_UINT16 p_uErrorValue1,    /**< [in] error value 1 */
     PDRV_UINT16 p_uErrorValue2,    /**< [in] error value 2 = faulty subindex */
     PDRV_UINT8 *p_puRsp            /**< [out] response buffer */
    )
{
    PDRV_UINT8 uRspLen;

    p_puRsp[PAR_I_RQID]        |= PDRV_PARCD_NEGRSP;        /* error response ID */
    p_puRsp[PAR_I_RSP_FMT]      = PDRV_FMT_ERROR;           /* format = error */
    p_puRsp[PAR_I_RSP_NRVAL]    = 1U;                       /* number of values = 1 */
    p_puRsp[PAR_I_RSP_VAL + 0U] = p_uErrorValue1 >> 8U;     /* MSB error value 1 */
    p_puRsp[PAR_I_RSP_VAL + 1U] = p_uErrorValue1 & 0xFFU;   /* LSB error value 1 */
    uRspLen = 4U;

    if (   (p_uErrorValue1 == PDRV_EV1_VALUE_CHANG)      /* parameter value cannot be changed */
        || (p_uErrorValue1 == PDRV_EV1_VALUE_LIMIT)      /* low or high limit exceeded */
        || (p_uErrorValue1 == PDRV_EV1_SUBINDEX)         /* faulty subindex */
        || (p_uErrorValue1 == PDRV_EV1_RESET_ONLY)       /* setting not permitted (may only be reset) */
        || (p_uErrorValue1 == PDRV_EV1_DESC_CHANGE)      /* description element cannot be changed */
        || (p_uErrorValue1 == PDRV_EV1_VALUE_IMPERMISS)  /* value impermissible */
       )
    {
        p_puRsp[PAR_I_RSP_NRVAL]    = 2U;                       /* number of values = 2 */
        p_puRsp[PAR_I_RSP_VAL + 2U] = p_uErrorValue2 >> 8U;     /* MSB error value 2 */
        p_puRsp[PAR_I_RSP_VAL + 3U] = p_uErrorValue2 & 0xFFU;   /* LSB error value 2 */
        uRspLen = 6U;
    }

    return uRspLen;
}

/** PROFIdrive parameter change request
 *  @details    modifies response buffer
 *  @return     additional response length
*/
static PDRV_UINT8 uParWrite
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object in database */
     PDRV_ParCxnData *p_ptParCxnData    /**< [in, out] pointer to connection data */
    )
{
    PDRV_UINT8 uRspLen = 0;
    PDRV_UINT16 uSubindex =  (p_ptParCxnData->uReq[PAR_I_REQ_SUBHI] << 8U)
                            + p_ptParCxnData->uReq[PAR_I_REQ_SUBLO];
    PDRV_UINT8 uNrOfVal = p_ptParCxnData->uReq[PAR_I_REQ_NRVAL];
    PDRV_UINT8 ParObjFmt = p_ptParObj->uIdentifier & 0x00FFU;
    PDRV_UINT uSizeOfFmt = uParPdrvSizeOfFmt(ParObjFmt);
    PDRV_BL bIsResetOnly = (p_ptParObj->uIdentifier & PDRV_PARID_RESETONLY) == PDRV_PARID_RESETONLY ? PDRV_TRUE : PDRV_FALSE;
    PDRV_BL bIsArray = PDRV_FALSE;

    /* Parameter Object Formats VisibleString, OctetString, UnicodeString will be handled like an array. */
    if (   ((p_ptParObj->uIdentifier & PDRV_PARID_ARRAY) == PDRV_PARID_ARRAY)
        || (ParObjFmt == PDRV_PARID_VS)
        || (ParObjFmt == PDRV_PARID_OS)
        || (ParObjFmt == PDRV_PARID_US)
       )
    {
        bIsArray = PDRV_TRUE;
    }

    /* index incorrect in case of non array parameter? */
    if (   (bIsArray != PDRV_TRUE)
        && (   (uSubindex != 0U)
            || (uNrOfVal != 1U)
           )
       )
    { /* array access in case of non array parameter */
        uRspLen += uParSetError(PDRV_EV1_NO_ARRAY, 0U, &p_ptParCxnData->uRsp[0]);
    }    /* Illegal subindex? */
    else if (uSubindex == PDRV_SUBI_ILLEGAL)
    {
        uRspLen += uParSetError(PDRV_EV1_PAR_ADDR_IMPERMISS, 0U, &p_ptParCxnData->uRsp[0]);
    }    /* index out of range in case of an array parameter? */
    else if (   (bIsArray == PDRV_TRUE)
             && (   (uSubindex > (p_ptParObj->uNrOfElements - 1U))
                 || ((uSubindex + uNrOfVal) > p_ptParObj->uNrOfElements)
                )
            )
    { /* faulty subindex */
        uRspLen += uParSetError(PDRV_EV1_SUBINDEX, 0U, &p_ptParCxnData->uRsp[0]);
    }    /* number of values incorrect? */
    else if (   (   (p_ptParCxnData->uReq[PAR_I_REQ_NRELE] != 0U)        /* no single value request */
                 || (uNrOfVal != 1U)
                )
             && (   (p_ptParCxnData->uReq[PAR_I_REQ_NRELE] != uNrOfVal)
                 || ((uSizeOfFmt * uNrOfVal) > (PDRV_PAR_BLOCKSIZE - PAR_I_REQ_VAL))
                )
            )
    { /* incorrect index and/or number of values */
        uRspLen += uParSetError(PDRV_EV1_NR_VAL_INCONSISTNT, 0U, &p_ptParCxnData->uRsp[0]);
    }    /* request inconsistent with buffer length? */
    else if (p_ptParCxnData->uBufLen != (((uSizeOfFmt * uNrOfVal + 1U) & 0xFE) + PAR_I_REQ_VAL))
    { /* inconsistent with buffer length */
        uRspLen += uParSetError(PDRV_EV1_NR_VAL_INCONSISTNT, uSubindex, &p_ptParCxnData->uRsp[0]);
    }    /* parameter type incorrect? */
    else if (   (p_ptParCxnData->uReq[PAR_I_REQ_FMT] != ParObjFmt)         /* request differs from parameter data */
             && (   (p_ptParCxnData->uReq[PAR_I_REQ_FMT] != PDRV_FMT_BYTE) /* AND NO correct data size for byte */
                 || (uSizeOfFmt != 1U)
                )
             && (   (p_ptParCxnData->uReq[PAR_I_REQ_FMT] != PDRV_FMT_WORD) /* AND NO correct data size for word */
                 || (uSizeOfFmt != 2U)
                )
             && (   (p_ptParCxnData->uReq[PAR_I_REQ_FMT] != PDRV_FMT_DWORD) /* AND NO correct data size for dword */
                 || (uSizeOfFmt != 4U)
                )
            )
    { /* incorrect datatype */
        uRspLen += uParSetError(PDRV_EV1_DATATYPE, 0U, &p_ptParCxnData->uRsp[0]);
    }
    else
    {
        PDRV_UINT uJ = PAR_I_REQ_VAL;
        PDRV_UINT16 uErrorVal1 = 0U;
        PDRV_UINT16 uErrorVal2 = 0U;
        PDRV_UINT uI;
        PDRV_ParValues tValues;    /* parameter values with internal endianess */

        switch (ParObjFmt)
        {
        case PDRV_PARID_BL: /* Boolean */
        {
            for (uI = 0U; uI < uNrOfVal; uI++)
            {
                PDRV_BL bVal = p_ptParCxnData->uReq[uJ];

                /* limits exceeded? */
                if (bVal > PDRV_TRUE)
                {
                    uErrorVal1 = PDRV_EV1_VALUE_LIMIT;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }    /* non zero value but reset only attribute? */
                else if (   (bIsResetOnly == PDRV_TRUE)
                         && (bVal != PDRV_FALSE)
                        )
                {
                    uErrorVal1 = PDRV_EV1_RESET_ONLY;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }
                else
                {
                    tValues.bl[uI] = bVal;
                    uJ++;
                }
            }
            break;
        }
        case PDRV_PARID_O1: /* Unsigned8 */
        {
            PDRV_UINT uLoL = p_ptParObj->uLoLimit;
            PDRV_UINT uHiL = p_ptParObj->uHiLimit;

            for (uI = 0U; uI < uNrOfVal; uI++)
            {
                PDRV_UINT uVal = p_ptParCxnData->uReq[uJ];

                /* limits exceeded? */
                if (   (uVal < uLoL)
                    || (uVal > uHiL)
                   )
                {
                    uErrorVal1 = PDRV_EV1_VALUE_LIMIT;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }    /* non zero value but reset only attribute? */
                else if (   (bIsResetOnly == PDRV_TRUE)
                         && (uVal != 0U)
                        )
                {
                    uErrorVal1 = PDRV_EV1_RESET_ONLY;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }
                else
                {
                    tValues.o1[uI] = (PDRV_UINT8)uVal;
                    uJ++;
                }
            }
            break;
        }
        case PDRV_PARID_VS: /* VisibleString */
        case PDRV_PARID_OS: /* OctetString */
        {
            for (uI = 0U; uI < uNrOfVal; uI++)
            {
                tValues.os[uI] =  p_ptParCxnData->uReq[uJ];
                uJ++;
            }
            break;
        }
        case PDRV_PARID_I1: /* Signed8 */
        {
            PDRV_INT nLoL = (PDRV_INT)p_ptParObj->uLoLimit;
            PDRV_INT nHiL = (PDRV_INT)p_ptParObj->uHiLimit;

            for (uI = 0U; uI < uNrOfVal; uI++)
            {
                PDRV_UINT uVal = p_ptParCxnData->uReq[uJ];
                PDRV_INT  nVal = (uVal > 127U) ? -(PDRV_INT)((~uVal + 1) & 0x0FFU) : (PDRV_INT)uVal;

                /* limits exceeded? */
                if (   (nVal < nLoL)
                    || (nVal > nHiL)
                   )
                {
                    uErrorVal1 = PDRV_EV1_VALUE_LIMIT;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }    /* non zero value but reset only attribute? */
                else if (   (bIsResetOnly == PDRV_TRUE)
                         && (nVal != 0)
                        )
                {
                    uErrorVal1 = PDRV_EV1_RESET_ONLY;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }
                else
                {
                    tValues.i1[uI] = (PDRV_INT8)nVal;
                    uJ++;
                }
            }
            break;
        }
        case PDRV_PARID_O2: /* Unsigned16 */
        case PDRV_PARID_V2: /* V2 Bit sequence (16 bit) */
        case PDRV_PARID_L2: /* L2 Nibble (16 bit) */
        case PDRV_PARID_R2: /* R2 Reciprocal time constant (16 bit: 1 ... 16384) */
        case PDRV_PARID_T2: /* T2 Time constant (16 bit: 0 ... 32767, values >32767 are identical with zero) */
        case PDRV_PARID_D2: /* D2 Time constant (16 bit: 0 ... 32767) */
        {
            PDRV_UINT uLoL = p_ptParObj->uLoLimit;
            PDRV_UINT uHiL = p_ptParObj->uHiLimit;

            for (uI = 0U; uI < uNrOfVal; uI++)
            {
                PDRV_UINT uVal =  ((PDRV_UINT)p_ptParCxnData->uReq[uJ + 0U] << 8U)
                                +  (PDRV_UINT)p_ptParCxnData->uReq[uJ + 1U];

                /* limits exceeded? */
                if (   (uVal < uLoL)
                    || (uVal > uHiL)
                   )
                {
                    uErrorVal1 = PDRV_EV1_VALUE_LIMIT;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }    /* non zero value but reset only attribute? */
                else if (   (bIsResetOnly == PDRV_TRUE)
                         && (uVal != 0U)
                        )
                {
                    uErrorVal1 = PDRV_EV1_RESET_ONLY;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }
                else
                {
                    tValues.o2[uI] = (PDRV_UINT16)uVal;
                    uJ += 2U;
                }
            }
            break;
        }
        case PDRV_PARID_US: /* UnicodeString */
        {
            for (uI = 0U; uI < uNrOfVal; uI++)
            {
                PDRV_UINT16 uVal = ((PDRV_UINT16)p_ptParCxnData->uReq[uJ] << 8U) + p_ptParCxnData->uReq[uJ + 1U];

                tValues.o2[uI] = uVal;
                uJ += 2U;
            }
            break;
        }
        case PDRV_PARID_I2: /* Signed16 */
        case PDRV_PARID_N2: /* N2 Normalised value (16 bit with 0x4000 == 100%) */
        case PDRV_PARID_X2: /* X2 Normalised value (16 bit with variable normalisation) */
        case PDRV_PARID_E2: /* E2 Fixed point value (16 bit with 1 == 2^-7) */
        {
            PDRV_INT16 nLoL = (PDRV_INT16)p_ptParObj->uLoLimit;
            PDRV_INT16 nHiL = (PDRV_INT16)p_ptParObj->uHiLimit;

            for (uI = 0U; uI < uNrOfVal; uI++)
            {
                PDRV_INT16 nVal = (((PDRV_UINT16)p_ptParCxnData->uReq[uJ + 0U] << 8U)
                                 +  (PDRV_UINT16)p_ptParCxnData->uReq[uJ + 1U]);

                /* limits exceeded? */
                if (   (nVal < nLoL)
                    || (nVal > nHiL)
                   )
                {
                    uErrorVal1 = PDRV_EV1_VALUE_LIMIT;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }    /* non zero value but reset only attribute? */
                else if (   (bIsResetOnly == PDRV_TRUE)
                         && (nVal != 0)
                        )
                {
                    uErrorVal1 = PDRV_EV1_RESET_ONLY;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }
                else
                {
                    tValues.i2[uI] = nVal;
                    uJ += 2U;
                }
            }
            break;
        }
        case PDRV_PARID_O4: /* Unsigned32 */
        case PDRV_PARID_TOD: /* TimeOfDay without date indication  */
        case PDRV_PARID_TD: /* TimeDifference without date indication */
        case PDRV_PARID_T4: /* T4 Time constant (32 bit) */
        {
            PDRV_UINT32 uLoL = p_ptParObj->uLoLimit;
            PDRV_UINT32 uHiL = p_ptParObj->uHiLimit;

            for (uI = 0U; uI < uNrOfVal; uI++)
            {
                PDRV_UINT32 uVal =   ((PDRV_UINT32)p_ptParCxnData->uReq[uJ + 0U] << 24U)
                                   + ((PDRV_UINT32)p_ptParCxnData->uReq[uJ + 1U] << 16U)
                                   + ((PDRV_UINT32)p_ptParCxnData->uReq[uJ + 2U] <<  8U)
                                   +  (PDRV_UINT32)p_ptParCxnData->uReq[uJ + 3U];

                /* limits exceeded? */
                if (   (uVal < uLoL)
                    || (uVal > uHiL)
                   )
                {
                    uErrorVal1 = PDRV_EV1_VALUE_LIMIT;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }    /* non zero value but reset only attribute? */
                else if (   (bIsResetOnly == PDRV_TRUE)
                         && (uVal != 0U)
                        )
                {
                    uErrorVal1 = PDRV_EV1_RESET_ONLY;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }
                else
                {
                    tValues.o4[uI] = uVal;
                    uJ += 4U;
                }
            }
            break;
        }
        case PDRV_PARID_I4: /* Signed32 */
        case PDRV_PARID_N4: /* N4 Normalised value (32 bit with 0x40000000 == 100%) */
        case PDRV_PARID_X4: /* X4 Normalised value (32 bit with variable normalisation) */
        case PDRV_PARID_C4: /* C4 Fixed point value (32 bit with 1 == 0.0001) */
        {
            PDRV_INT32 nLoL = (PDRV_INT32)p_ptParObj->uLoLimit;
            PDRV_INT32 nHiL = (PDRV_INT32)p_ptParObj->uHiLimit;

            for (uI = 0U; uI < uNrOfVal; uI++)
            {
                PDRV_INT32 nVal =  (((PDRV_UINT32)p_ptParCxnData->uReq[uJ + 0U] << 24U)
                                  + ((PDRV_UINT32)p_ptParCxnData->uReq[uJ + 1U] << 16U)
                                  + ((PDRV_UINT32)p_ptParCxnData->uReq[uJ + 2U] <<  8U)
                                  +  (PDRV_UINT32)p_ptParCxnData->uReq[uJ + 3U]);

                /* limits exceeded? */
                if (   (nVal < nLoL)
                    || (nVal > nHiL)
                   )
                {
                    uErrorVal1 = PDRV_EV1_VALUE_LIMIT;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }    /* non zero value but reset only attribute? */
                else if (   (bIsResetOnly == PDRV_TRUE)
                         && (nVal != 0)
                        )
                {
                    uErrorVal1 = PDRV_EV1_RESET_ONLY;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }
                else
                {
                    tValues.i4[uI] = nVal;
                    uJ += 4U;
                }
            }
            break;
        }
        case PDRV_PARID_F4: /* FloatingPoint32 single precision */
        {
            union
            {
                PDRV_UINT32 o4;
                PDRV_F4     f4;
            } tVal; /* TODO: union is not a portable solution */

            PDRV_F4 fLoL;
            PDRV_F4 fHiL;

            tVal.f4 = 0.f;
            tVal.o4 = p_ptParObj->uLoLimit;
            fLoL = tVal.f4;
            tVal.o4 = p_ptParObj->uHiLimit;
            fHiL = tVal.f4;

            for (uI = 0U; uI < uNrOfVal; uI++)
            {
                tVal.o4 =   ((PDRV_UINT32)p_ptParCxnData->uReq[uJ + 0U] << 24U)
                          + ((PDRV_UINT32)p_ptParCxnData->uReq[uJ + 1U] << 16U)
                          + ((PDRV_UINT32)p_ptParCxnData->uReq[uJ + 2U] <<  8U)
                          +  (PDRV_UINT32)p_ptParCxnData->uReq[uJ + 3U];

                /* limits exceeded? */
                if (   (tVal.f4 < fLoL)
                    || (tVal.f4 > fHiL)
                   )
                {
                    uErrorVal1 = PDRV_EV1_VALUE_LIMIT;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }    /* non zero value but reset only attribute? */
                else if (   (bIsResetOnly == PDRV_TRUE)
                         && (tVal.f4 != FLOAT_0)
                        )
                {
                    uErrorVal1 = PDRV_EV1_RESET_ONLY;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }
                else
                {
                    tValues.f4[uI] = tVal.f4;
                    uJ += 4U;
                }
            }
            break;
        }
        case PDRV_PARID_F8: /* FloatingPoint64 double precision */
        {
            union
            {
                PDRV_UINT64 o8;
                PDRV_F8     f8;
            } tVal; /* TODO: union is not a portable solution */

            tVal.f8 = 0.f;
            for (uI = 0U; uI < uNrOfVal; uI++)
            {
                tVal.o8 =   ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 0U] << 56U)
                          + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 1U] << 48U)
                          + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 2U] << 40U)
                          + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 3U] << 32U)
                          + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 4U] << 24U)
                          + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 5U] << 16U)
                          + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 6U] <<  8U)
                          +  (PDRV_UINT64)p_ptParCxnData->uReq[uJ + 7U];

                /* non zero value but reset only attribute? */
                if (   (bIsResetOnly == PDRV_TRUE)
                    && (tVal.f8 != FLOAT_0)
                   )
                {
                    uErrorVal1 = PDRV_EV1_RESET_ONLY;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }
                else
                {
                    tValues.f8[uI] = tVal.f8;
                    uJ += 8U;
                }
            }
            break;
        }
        case PDRV_PARID_TODW: /* TimeOfDay (with date indication) */
        case PDRV_PARID_TDW: /* TimeDifference with date indication */
        {
            PDRV_TODW tVal;

            for (uI = 0U; uI < uNrOfVal; uI++)
            {

                tVal.ms =  ((PDRV_UINT32)p_ptParCxnData->uReq[uJ + 0U] << 24U)
                         + ((PDRV_UINT32)p_ptParCxnData->uReq[uJ + 1U] << 16U)
                         + ((PDRV_UINT32)p_ptParCxnData->uReq[uJ + 2U] <<  8U)
                         +  (PDRV_UINT32)p_ptParCxnData->uReq[uJ + 3U];
                tVal.d  =  ((PDRV_UINT16)p_ptParCxnData->uReq[uJ + 4U] << 8U)
                         +  (PDRV_UINT16)p_ptParCxnData->uReq[uJ + 5U];

                /* non zero value but reset only attribute? */
                if (   (bIsResetOnly == PDRV_TRUE)
                    && (   (tVal.ms != 0UL)
                        || (tVal.d  != 0U)
                       )
                   )
                {
                    uErrorVal1 = PDRV_EV1_RESET_ONLY;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }
                else
                {
                    tValues.todw[uI] = tVal;
                    uJ += 6U;
                }
            }
            break;
        }
        case PDRV_PARID_DATE: /* BinaryDate */
        {
            PDRV_DATE tVal;

            for (uI = 0U; uI < uNrOfVal; uI++)
            {

                tVal.ms  = ((PDRV_UINT16)p_ptParCxnData->uReq[uJ + 0U] <<  8U)
                          + (PDRV_UINT16)p_ptParCxnData->uReq[uJ + 1U];
                tVal.min =   (PDRV_UINT8)p_ptParCxnData->uReq[uJ + 2U];
                tVal.h   =   (PDRV_UINT8)p_ptParCxnData->uReq[uJ + 3U];
                tVal.d   =   (PDRV_UINT8)p_ptParCxnData->uReq[uJ + 4U];
                tVal.m   =   (PDRV_UINT8)p_ptParCxnData->uReq[uJ + 5U];
                tVal.y   =   (PDRV_UINT8)p_ptParCxnData->uReq[uJ + 6U];
                tVal.alignment = 0U;

                /* non zero value but reset only attribute? */
                if (   (bIsResetOnly == PDRV_TRUE)
                    && (   (tVal.ms  != 0U)
                        || (tVal.min != 0U)
                        || (tVal.h   != 0U)
                        || (tVal.d   != 0x21U)  /* Monday 01.01.1951 */
                        || (tVal.m   != 1U)     /* January */
                        || (tVal.y   != 51U)    /* 1951 */
                       )
                   )
                {
                    uErrorVal1 = PDRV_EV1_RESET_ONLY;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }
                else
                {
                    tValues.date[uI] = tVal;
                    uJ += 8U;
                }
            }
            break;
        }
        case PDRV_PARID_O8: /* Unsigned64 */
        {
            for (uI = 0U; uI < uNrOfVal; uI++)
            {
                PDRV_UINT64 uVal =   ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 0U] << 56U)
                                   + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 1U] << 48U)
                                   + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 2U] << 40U)
                                   + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 3U] << 32U)
                                   + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 4U] << 24U)
                                   + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 5U] << 16U)
                                   + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 6U] <<  8U)
                                   +  (PDRV_UINT64)p_ptParCxnData->uReq[uJ + 7U];

                /* non zero value but reset only attribute? */
                if (   (bIsResetOnly == PDRV_TRUE)
                    && (uVal != 0U)
                   )
                {
                    uErrorVal1 = PDRV_EV1_RESET_ONLY;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }
                else
                {
                    tValues.o8[uI] = uVal;
                    uJ += 8U;
                }
            }
            break;
        }
        case PDRV_PARID_I8: /* Signed64 */
        {
            for (uI = 0U; uI < uNrOfVal; uI++)
            {
                PDRV_INT64 nVal =  (((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 0U] << 56U)
                                  + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 1U] << 48U)
                                  + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 2U] << 40U)
                                  + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 3U] << 32U)
                                  + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 4U] << 24U)
                                  + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 5U] << 16U)
                                  + ((PDRV_UINT64)p_ptParCxnData->uReq[uJ + 6U] <<  8U)
                                  +  (PDRV_UINT64)p_ptParCxnData->uReq[uJ + 7U]);

                /* non zero value but reset only attribute? */
                if (   (bIsResetOnly == PDRV_TRUE)
                    && (nVal != 0U)
                   )
                {
                    uErrorVal1 = PDRV_EV1_RESET_ONLY;
                    uErrorVal2 = uI;
                    uNrOfVal = 0U;
                }
                else
                {
                    tValues.i8[uI] = nVal;
                    uJ += 8U;
                }
            }
            break;
        }
        default:
        {
            uErrorVal1 = PDRV_EV1_FORMAT_ILLEGAL;
            uErrorVal2 = 0U;
            break;
        }
        }

        if (uNrOfVal != 0U)
        {
            PDRV_UINT32 uErrCode = p_ptParObj->pfnWrite(p_ptParObj, uSubindex, uNrOfVal, &tValues);

            uErrorVal1 = uErrCode & 0xFFFFU;
            uErrorVal2 = uErrCode >> 16U;

        }
        if (uErrorVal1 != PDRV_EV1_NOERROR)
        {
            uRspLen += uParSetError(uErrorVal1, uErrorVal2, &p_ptParCxnData->uRsp[0]);
        }
    }

    return uRspLen;
}

/** PROFIdrive parameter read request
 *  @details    modifies response buffer
 *  @return     additional response length
*/
static PDRV_UINT8 uParRead
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object in database*/
     PDRV_ParCxnData *p_ptParCxnData    /**< [in, out] pointer to connection data */
    )
{
    PDRV_ParValues tValues;    /* parameter values with internal endianess */
    PDRV_UINT8 uRspLen = 0U;
    PDRV_UINT16 uNrOfElements = p_ptParCxnData->uReq[PAR_I_REQ_NRELE];
    PDRV_UINT16 uSubindex =  (p_ptParCxnData->uReq[PAR_I_REQ_SUBHI] << 8U)
                            + p_ptParCxnData->uReq[PAR_I_REQ_SUBLO];
    PDRV_UINT8 ParObjFmt = p_ptParObj->uIdentifier & 0x00FFU;
    PDRV_UINT uSizeOfFmt = uParPdrvSizeOfFmt(ParObjFmt);
    PDRV_BL bIsArray = PDRV_FALSE;
    PDRV_UINT16 uErrorVal1 = 0U;
    PDRV_UINT16 uErrorVal2 = 0U;

    /* another special treatment for OctetString (see ProfilTester) */
    if (   (ParObjFmt == PDRV_PARID_OS)
        && (uNrOfElements == 0U)
        && (uSubindex == 0U)
       )
    {
        uNrOfElements = p_ptParObj->uNrOfElements;
        if (uNrOfElements > (PDRV_PAR_BLOCKSIZE - PAR_I_RSP_VAL))
        {
            uNrOfElements = PDRV_PAR_BLOCKSIZE - PAR_I_RSP_VAL;
        }
    }

    /* single parameter value request see PDRV V4.2 table 34 */
    if (uNrOfElements == 0U)
    {
        uSubindex = 0U;
        uNrOfElements = 1U;
    }

    /* Parameter Object Formats VisibleString, OctetString and UnicodeString will be handled as an array. */
    if (   ((p_ptParObj->uIdentifier & PDRV_PARID_ARRAY) == PDRV_PARID_ARRAY)
        || (ParObjFmt == PDRV_PARID_VS)
        || (ParObjFmt == PDRV_PARID_OS)
        || (ParObjFmt == PDRV_PARID_US)
       )
    {
        bIsArray = PDRV_TRUE;
    }

     /* Illegal subindex? */
    if (uSubindex == PDRV_SUBI_ILLEGAL)
    {
        uRspLen += uParSetError(PDRV_EV1_PAR_ADDR_IMPERMISS, 0U, &p_ptParCxnData->uRsp[0]);
    }    /* faulty subindex? */
    else if (   (bIsArray == PDRV_TRUE)
             && (   (uSubindex > (p_ptParObj->uNrOfElements - 1U))
                 || ((uSubindex + uNrOfElements) > p_ptParObj->uNrOfElements)
                )
            )
    {
        uRspLen += uParSetError(PDRV_EV1_SUBINDEX, uSubindex + uNrOfElements, &p_ptParCxnData->uRsp[0]);
    }    /* incorrect subindex or incorrect number of elements if non array parameter? */
    else if (   (bIsArray != PDRV_TRUE)
             && (   (uSubindex > 0U)
                 || (uNrOfElements > 1U)
                )
            )
    {
        uRspLen += uParSetError(PDRV_EV1_NO_ARRAY, 0U, &p_ptParCxnData->uRsp[0]);
    }    /* response too long? */
    else if ((uSizeOfFmt * uNrOfElements) > (PDRV_PAR_BLOCKSIZE - PAR_I_RSP_VAL))
    {
        uRspLen += uParSetError(PDRV_EV1_RESP_TOO_LONG, 0U, &p_ptParCxnData->uRsp[0]);
    }
    else
    {
        PDRV_UINT32 uErrCode = p_ptParObj->pfnRead(p_ptParObj, uSubindex, uNrOfElements, &tValues);
        uErrorVal1 = uErrCode & 0xFFFFU;
        uErrorVal2 = uErrCode >> 16U;

        /* read function with error value 1? */
        if (uErrorVal1 != PDRV_EV1_NOERROR)
        {
            uRspLen += uParSetError(uErrorVal1, uErrorVal2, &p_ptParCxnData->uRsp[0]);
        }
        else
        {
            PDRV_UINT uI;
            PDRV_UINT uJ = PAR_I_RSP_VAL;

            p_ptParCxnData->uRsp[PAR_I_RSP_FMT] = ParObjFmt;
            p_ptParCxnData->uRsp[PAR_I_RSP_NRVAL] = uNrOfElements;
            switch (ParObjFmt)
            {
            case PDRV_PARID_BL: /* Boolean */
            case PDRV_PARID_I1: /* Signed8 */
            case PDRV_PARID_O1: /* Unsigned8 */
            case PDRV_PARID_VS: /* VisibleString */
            case PDRV_PARID_OS: /* OctetString */
            {
                for (uI = 0U; uI < uNrOfElements; uI++)
                {
                    p_ptParCxnData->uRsp[uJ++] = tValues.o1[uI];
                }
                /* odd number of bytes? */
                if ((uJ & 1U) == 1)
                { /* PDRV uses words of 16 bits therefore an even number is required */
                    p_ptParCxnData->uRsp[uJ++] = 0U;
                }
                uRspLen = uJ;
                break;
            }
            case PDRV_PARID_I2: /* Signed16 */
            case PDRV_PARID_O2: /* Unsigned16 */
            case PDRV_PARID_US: /* UnicodeString */
            case PDRV_PARID_N2: /* N2 Normalised value (16 bit with 0x4000 == 100%) */
            case PDRV_PARID_V2: /* V2 Bit sequence (16 bit) */
            case PDRV_PARID_L2: /* L2 Nibble (16 bit) */
            case PDRV_PARID_R2: /* R2 Reciprocal time constant (16 bit: 1 ... 16384) */
            case PDRV_PARID_T2: /* T2 Time constant (16 bit: 0 ... 32767, values >32767 are identical with zero) */
            case PDRV_PARID_D2: /* D2 Time constant (16 bit: 0 ... 32767) */
            case PDRV_PARID_E2: /* E2 Fixed point value (16 bit with 1 == 2^-7) */
            case PDRV_PARID_X2: /* X2 Normalised value (16 bit with variable normalisation) */
            {
                for (uI = 0U; uI < uNrOfElements; uI++)
                {
                    p_ptParCxnData->uRsp[uJ++] = tValues.o2[uI] >> 8U;
                    p_ptParCxnData->uRsp[uJ++] = tValues.o2[uI] & 0xFFU;
                }
                uRspLen = uJ;
                break;
            }
            case PDRV_PARID_I4: /* Signed32 */
            case PDRV_PARID_O4: /* Unsigned32 */
            case PDRV_PARID_N4: /* N4 Normalised value (32 bit with 0x40000000 == 100%) */
            case PDRV_PARID_T4: /* T4 Time constant (32 bit) */
            case PDRV_PARID_C4: /* C4 Fixed point value (32 bit with 1 == 0.0001) */
            case PDRV_PARID_X4: /* X4 Normalised value (32 bit with variable normalisation) */
            case PDRV_PARID_F4: /* Floating Point 32 */
            case PDRV_PARID_TOD: /* TimeOfDay without date indication  */
            case PDRV_PARID_TD: /* TimeDifference without date indication */
            {
                for (uI = 0U; uI < uNrOfElements; uI++)
                {
                    p_ptParCxnData->uRsp[uJ++] = (tValues.o4[uI] >> 24U) & 0xFFU;
                    p_ptParCxnData->uRsp[uJ++] = (tValues.o4[uI] >> 16U) & 0xFFU;
                    p_ptParCxnData->uRsp[uJ++] = (tValues.o4[uI] >>  8U) & 0xFFU;
                    p_ptParCxnData->uRsp[uJ++] =  tValues.o4[uI] & 0xFFU;
                }
                uRspLen = uJ;
                break;
            }
            case PDRV_PARID_I8: /* Signed64 */
            case PDRV_PARID_O8: /* Unsigned64 */
            case PDRV_PARID_F8: /* Floating Point 64 (double) */
            {
                for (uI = 0U; uI < uNrOfElements; uI++)
                {
                    p_ptParCxnData->uRsp[uJ++] = (tValues.o8[uI] >> 54U) & 0xFFU;
                    p_ptParCxnData->uRsp[uJ++] = (tValues.o8[uI] >> 48U) & 0xFFU;
                    p_ptParCxnData->uRsp[uJ++] = (tValues.o8[uI] >> 40U) & 0xFFU;
                    p_ptParCxnData->uRsp[uJ++] = (tValues.o8[uI] >> 32U) & 0xFFU;
                    p_ptParCxnData->uRsp[uJ++] = (tValues.o8[uI] >> 24U) & 0xFFU;
                    p_ptParCxnData->uRsp[uJ++] = (tValues.o8[uI] >> 16U) & 0xFFU;
                    p_ptParCxnData->uRsp[uJ++] = (tValues.o8[uI] >>  8U) & 0xFFU;
                    p_ptParCxnData->uRsp[uJ++] =  tValues.o8[uI] & 0xFFU;
                }
                uRspLen = uJ;
                break;
            }
            case PDRV_PARID_TODW: /* TimeOfDay (with date indication) TimeOfDay */
            case PDRV_PARID_TDW: /* TimeDifference with date indication */
            {
                for (uI = 0U; uI < uNrOfElements; uI++)
                {
                    p_ptParCxnData->uRsp[uJ++] = (tValues.todw[uI].ms >> 24U) & 0xFFU;
                    p_ptParCxnData->uRsp[uJ++] = (tValues.todw[uI].ms >> 16U) & 0xFFU;
                    p_ptParCxnData->uRsp[uJ++] = (tValues.todw[uI].ms >>  8U) & 0xFFU;
                    p_ptParCxnData->uRsp[uJ++] =  tValues.todw[uI].ms & 0xFFU;
                    p_ptParCxnData->uRsp[uJ++] =  tValues.todw[uI].d >>  8U;
                    p_ptParCxnData->uRsp[uJ++] =  tValues.todw[uI].d & 0xFFU;
                }
                uRspLen = uJ;
                break;
            }
            case PDRV_PARID_DATE: /* BinaryDate */
            {
                for (uI = 0U; uI < uNrOfElements; uI++)
                {
                    p_ptParCxnData->uRsp[uJ++] = tValues.date[uI].ms >> 8U;
                    p_ptParCxnData->uRsp[uJ++] = tValues.date[uI].ms & 0xFFU;
                    p_ptParCxnData->uRsp[uJ++] = tValues.date[uI].min;
                    p_ptParCxnData->uRsp[uJ++] = tValues.date[uI].h;
                    p_ptParCxnData->uRsp[uJ++] = tValues.date[uI].d;
                    p_ptParCxnData->uRsp[uJ++] = tValues.date[uI].m;
                    p_ptParCxnData->uRsp[uJ++] = tValues.date[uI].y;
                    p_ptParCxnData->uRsp[uJ++] = 0U; /* alignment byte */
                }
                uRspLen = uJ;
                break;
            }
            default:
            {
                uRspLen = PAR_I_RSP_FMT;
                break;
            }
            }
            uRspLen -= PAR_I_RSP_FMT;
        }
    }

    return uRspLen;
}


/** PROFIdrive parameter description read request
 *  @details    realize telegram sequence 10 of parameter access, see PDRV 4.2 tables 63 .. 65
 *  @return     additional response length
*/
static PDRV_UINT8 uParReadDesc
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object in database*/
     PDRV_ParCxnData *p_ptParCxnData    /**< [in, out] pointer to connection data */
    )
{
    PDRV_UINT8 uRspLen = 0;
    PDRV_UINT16 uSubindex =  (p_ptParCxnData->uReq[PAR_I_REQ_SUBHI] << 8U)
                            + p_ptParCxnData->uReq[PAR_I_REQ_SUBLO];

    /* number of elements wrong? */
    if (   (uSubindex != PDRV_DESCI_COMPLETE)
        && (p_ptParCxnData->uReq[PAR_I_REQ_NRELE] != 1U)
       )
    { /* number of elements is wrong */
        uRspLen += uParSetError(PDRV_EV1_PAR_ADDR_IMPERMISS, 0U, &p_ptParCxnData->uRsp[0]);
    }    /* Illegal subindex? */
    else if (uSubindex == PDRV_SUBI_ILLEGAL)
    {
        uRspLen += uParSetError(PDRV_EV1_PAR_ADDR_IMPERMISS, 0U, &p_ptParCxnData->uRsp[0]);
    }
    else
    { /* all is fine, see PDRV V4.2 table 17 */
        switch (uSubindex)
        {
            case PDRV_DESCI_COMPLETE:       /* complete description */
            {
                 union
                {
                    PDRV_UINT32 o4;
                    PDRV_F4     f4;
                } tStdFactor; /* TODO: union is not a portable solution */

                tStdFactor.f4 = p_ptParObj->fStdFactor;

                p_ptParCxnData->uRsp[PAR_I_RSP_FMT]      = PDRV_PARID_OS;                           /* format = octet string */
                p_ptParCxnData->uRsp[PAR_I_RSP_NRVAL]    = 46U;                                     /* number of values = 46 */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 0U] = p_ptParObj->uIdentifier >> 8U;           /* MSB identifier */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 1U] = p_ptParObj->uIdentifier & 0xFFU;         /* LSB identifier */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 2U] = p_ptParObj->uNrOfElements >> 8U;         /* MSB number of elements */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 3U] = p_ptParObj->uNrOfElements & 0xFFU;       /* LSB number of elements */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 4U] = (tStdFactor.o4 >> 24U) & 0xFFU;          /* MSB standardisation factor */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 5U] = (tStdFactor.o4 >> 16U) & 0xFFU;          /* standardisation factor */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 6U] = (tStdFactor.o4 >>  8U) & 0xFFU;          /* standardisation factor */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 7U] =  tStdFactor.o4         & 0xFFU;          /* LSB standardisation factor */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 8U] = p_ptParObj->uVarAttrib >> 8U;            /* MSB variable attribute */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 9U] = p_ptParObj->uVarAttrib & 0xFFU;          /* LSB vairable attribute */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +10U] = 0U;                                      /* reserved */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +11U] = 0U;                                      /* reserved */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +12U] = 0U;                                      /* reserved */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +13U] = 0U;                                      /* reserved */
                ParPdrvStrCpy(&p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 14U], p_ptParObj->puName);      /* 16 characters with parameter name */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +30U] = p_ptParObj->uLoLimit >> 24U;             /* MSB low limit */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +31U] = (p_ptParObj->uLoLimit >> 16U) & 0xFFU;   /* low limit */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +32U] = (p_ptParObj->uLoLimit >>  8U) & 0xFFU;   /* low limit */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +33U] = p_ptParObj->uLoLimit & 0xFFU;            /* LSB low limit */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +34U] = p_ptParObj->uHiLimit >> 24U;             /* MSB high limit */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +35U] = (p_ptParObj->uHiLimit >> 16U) & 0xFFU;   /* high limit */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +36U] = (p_ptParObj->uHiLimit >>  8U) & 0xFFU;   /* high limit */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +37U] = p_ptParObj->uHiLimit & 0xFFU;            /* LSB high limit */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +38U] = 0U;                                      /* reserved */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +39U] = 0U;                                      /* reserved */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +40U] = 0U;                                      /* ID extension is reserved */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +41U] = 0U;                                      /* ID extension is reserved */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +42U] = p_ptParObj->uRefPar >> 8U;               /* MSB reference parameter */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +43U] = p_ptParObj->uRefPar & 0xFFU;             /* LSB reference parameter */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +44U] = p_ptParObj->uNormalisation >> 8U;        /* MSB normalisation */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL +45U] = p_ptParObj->uNormalisation & 0xFFU;      /* LSB normalisation */
                uRspLen = 48U;
                break;
            }
            case PDRV_DESCI_IDENTIFIER:     /* identifier */
            {
                p_ptParCxnData->uRsp[PAR_I_RSP_FMT]      = PDRV_PARID_V2;                   /* format = V2 */
                p_ptParCxnData->uRsp[PAR_I_RSP_NRVAL]    = 1U;                              /* number of values = 1 */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 0U] = p_ptParObj->uIdentifier >> 8U;   /* MSB identifier */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 1U] = p_ptParObj->uIdentifier & 0xFFU; /* LSB identifier */
                uRspLen = 4U;
                break;
            }
            case PDRV_DESCI_NROFELEMENTS:   /* number of array elements or lenght of string */
            {
                p_ptParCxnData->uRsp[PAR_I_RSP_FMT]      = PDRV_PARID_O2;                       /* format = Unsigned16 */
                p_ptParCxnData->uRsp[PAR_I_RSP_NRVAL]    = 1U;                                  /* number of values = 1 */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 0U] = p_ptParObj->uNrOfElements >> 8U;     /* MSB number of elements */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 1U] = p_ptParObj->uNrOfElements & 0xFFU;   /* LSB number of elements */
                uRspLen = 4U;
                break;
            }
            case PDRV_DESCI_STDFACTOR:      /* standardisation factor */
            {
                union
                {
                    PDRV_UINT32 o4;
                    PDRV_F4     f4;
                } tStdFactor; /* TODO: union is not a portable solution */

                tStdFactor.f4 = p_ptParObj->fStdFactor;

                p_ptParCxnData->uRsp[PAR_I_RSP_FMT]      = PDRV_PARID_F4;   /* format = floating point */
                p_ptParCxnData->uRsp[PAR_I_RSP_NRVAL]    = 1U;              /* number of values = 1 */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 0U] = (tStdFactor.o4 >> 24U) & 0xFFU; /* MSB standardisation factor */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 1U] = (tStdFactor.o4 >> 16U) & 0xFFU; /* standardisation factor */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 2U] = (tStdFactor.o4 >>  8U) & 0xFFU; /* standardisation factor */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 3U] =  tStdFactor.o4         & 0xFFU; /* LSB standardisation factor */
                uRspLen = 6U;
                break;
            }
            case PDRV_DESCI_VARATTRIB:      /* variable attribute */
            {
                p_ptParCxnData->uRsp[PAR_I_RSP_FMT]      = PDRV_PARID_OS;                   /* format = octet string */
                p_ptParCxnData->uRsp[PAR_I_RSP_NRVAL]    = 2U;                              /* number of values = 2 */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 0U] = p_ptParObj->uVarAttrib >> 8U;    /* MSB variable attribute */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 1U] = p_ptParObj->uVarAttrib & 0xFFU;  /* LSB vairable attribute */
                uRspLen = 4U;
                break;
            }
            case PDRV_DESCI_RESERVED1:
            {
                p_ptParCxnData->uRsp[PAR_I_RSP_FMT]      = PDRV_PARID_OS;   /* format = octet string */
                p_ptParCxnData->uRsp[PAR_I_RSP_NRVAL]    = 4U;              /* number of values = 4 */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 0U] = 0U;
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 1U] = 0U;
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 2U] = 0U;
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 3U] = 0U;
                uRspLen = 4U;
                break;
            }
            case PDRV_DESCI_NAME:           /* name */
            {
                p_ptParCxnData->uRsp[PAR_I_RSP_FMT]      = PDRV_PARID_VS;  /* format = visible string */
                p_ptParCxnData->uRsp[PAR_I_RSP_NRVAL]    = PAR_NAMELEN;    /* number of values = PAR_NAMELEN */
                ParPdrvStrCpy(&p_ptParCxnData->uRsp[PAR_I_RSP_VAL], p_ptParObj->puName);
                uRspLen = PAR_NAMELEN + 2U;
                break;
            }
            case PDRV_DESCI_LOLIMIT:        /* low limit */
            {
                p_ptParCxnData->uRsp[PAR_I_RSP_FMT]      = PDRV_PARID_OS;                           /* format = octet string */
                p_ptParCxnData->uRsp[PAR_I_RSP_NRVAL]    = 4U;                                      /* number of values = 4 */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 0U] = p_ptParObj->uLoLimit >> 24U;             /* MSB low limit */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 1U] = (p_ptParObj->uLoLimit >> 16U) & 0xFFU;   /* low limit */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 2U] = (p_ptParObj->uLoLimit >>  8U) & 0xFFU;   /* low limit */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 3U] = p_ptParObj->uLoLimit & 0xFFU;            /* LSB low limit */
                uRspLen = 6U;
                break;
            }
            case PDRV_DESCI_HILIMIT:        /* high limit */
            {
                p_ptParCxnData->uRsp[PAR_I_RSP_FMT]      = PDRV_PARID_OS;                           /* format = octet string */
                p_ptParCxnData->uRsp[PAR_I_RSP_NRVAL]    = 4U;                                      /* number of values = 4 */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 0U] = p_ptParObj->uHiLimit >> 24U;             /* MSB high limit */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 1U] = (p_ptParObj->uHiLimit >> 16U) & 0xFFU;   /* high limit */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 2U] = (p_ptParObj->uHiLimit >>  8U) & 0xFFU;   /* high limit */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 3U] = p_ptParObj->uHiLimit & 0xFFU;            /* LSB high limit */
                uRspLen = 6U;
                break;
            }
            case PDRV_DESCI_RESERVED2:
            {
                p_ptParCxnData->uRsp[PAR_I_RSP_FMT]      = PDRV_PARID_OS;   /* format = octet string */
                p_ptParCxnData->uRsp[PAR_I_RSP_NRVAL]    = 2U;              /* number of values = 2 */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 0U] = 0U;              /* MSB variable attribute */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 1U] = 0U;              /* LSB vairable attribute */
                uRspLen = 4U;
                break;
            }
            case PDRV_DESCI_IDEXTENSION:
            {
                p_ptParCxnData->uRsp[PAR_I_RSP_FMT]      = PDRV_PARID_V2;   /* format = V2 Bit sequence (16 bit) */
                p_ptParCxnData->uRsp[PAR_I_RSP_NRVAL]    = 1U;              /* number of values = 1 */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 0U] = 0U;
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 1U] = 0U;
                uRspLen = 4U;
                break;
            }
            case PDRV_DESCI_REFPAR:         /* DO IO DATA reference parameter */
            {
                p_ptParCxnData->uRsp[PAR_I_RSP_FMT]      = PDRV_PARID_O2;               /* format = Unsigned 16 */
                p_ptParCxnData->uRsp[PAR_I_RSP_NRVAL]    = 1U;                          /* number of values = 1 */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 0U] = p_ptParObj->uRefPar >> 8U;   /* MSB reference parameter */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 1U] = p_ptParObj->uRefPar & 0xFFU; /* LSB reference parameter */
                uRspLen = 4U;
                break;
            }
            case PDRV_DESCI_NORMALISATION:  /* DO IO DATA normalisation */
            {
                p_ptParCxnData->uRsp[PAR_I_RSP_FMT]      = PDRV_PARID_V2;                       /* format = V2 */
                p_ptParCxnData->uRsp[PAR_I_RSP_NRVAL]    = 1U;                                  /* number of values = 1 */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 0U] = p_ptParObj->uNormalisation >> 8U;    /* MSB normalisation */
                p_ptParCxnData->uRsp[PAR_I_RSP_VAL + 1U] = p_ptParObj->uNormalisation & 0xFFU;  /* LSB normalisation */
                uRspLen = 4U;
                break;
            }
            default:
            { /* fault unknown subindex */
                uRspLen += uParSetError(PDRV_EV1_NO_DESC, 0U, &p_ptParCxnData->uRsp[0]);
                break;
            }
        }
    }

    return uRspLen;
}

/** PROFIdrive parameter text read request
 *  @details    copy one text string to the response buffer, see PDRV 4.2 chapter 6.2.1.4 Text
 *  @return     additional response length
*/
static PDRV_UINT8 uParReadText
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object in database */
     PDRV_ParCxnData *p_ptParCxnData    /**< [in, out] pointer to connection data */
    )
{
    PDRV_UINT8 uRspLen = 0;
    PDRV_UINT16 uSubindex =  (p_ptParCxnData->uReq[PAR_I_REQ_SUBHI] << 8U)
                            + p_ptParCxnData->uReq[PAR_I_REQ_SUBLO];
    PDRV_UINT8 uPdrvDataType = p_ptParObj->uIdentifier & 0x0FFU;
    PDRV_BL bIsArray = (p_ptParObj->uIdentifier & PDRV_PARID_ARRAY) == PDRV_PARID_ARRAY ? PDRV_TRUE : PDRV_FALSE;

    /* text not available? */
    if (   (p_ptParObj->pfnText == PDRV_NULL)
        || ((p_ptParObj->uIdentifier & PDRV_PARID_ADDTEXT) != PDRV_PARID_ADDTEXT)
        || (   (bIsArray != PDRV_TRUE)             /* single parameter */
            && (uPdrvDataType != PDRV_PARID_V2)    /* data type bit field 16 */
            && (uPdrvDataType != PDRV_PARID_BL)    /* data type boolean */
            && (uPdrvDataType != PDRV_PARID_O1)    /* data types unsigned */
            && (uPdrvDataType != PDRV_PARID_O2)
            && (uPdrvDataType != PDRV_PARID_O4)
           )
       )
    { /* no text available */
       uRspLen += uParSetError(PDRV_EV1_NO_TEXT_ARRAY, 0U, &p_ptParCxnData->uRsp[0]);
    }    /* number of elements wrong? */
    else if (p_ptParCxnData->uReq[PAR_I_REQ_NRELE] != 1U)
    { /* number of elements is wrong */
        uRspLen += uParSetError(PDRV_EV1_PAR_ADDR_IMPERMISS, 0U, &p_ptParCxnData->uRsp[0]);
    }    /* Illegal subindex? */
    else if (uSubindex == PDRV_SUBI_ILLEGAL)
    {
        uRspLen += uParSetError(PDRV_EV1_PAR_ADDR_IMPERMISS, 0U, &p_ptParCxnData->uRsp[0]);
    }    /* incorrect subindex? */
    else if (   (   (bIsArray == PDRV_TRUE)                         /* array parameter */
                 && (uSubindex >= p_ptParObj->uNrOfElements)        /* exceeds array size */
                )
             || (   (bIsArray != PDRV_TRUE)                         /* single parameter */
                 && (   (   (uPdrvDataType == PDRV_PARID_V2)        /* data type bit field 16 */
                         && (uSubindex >= 32U)                      /* 32 indices allowed */
                        )
                     || (   (uPdrvDataType == PDRV_PARID_BL)        /* data type boolean */
                         && (uSubindex >= 2U)                       /* 2 indices */
                        )
                     || (   (   (uPdrvDataType == PDRV_PARID_O1)    /* data type unsigned */
                             || (uPdrvDataType == PDRV_PARID_O2)
                             || (uPdrvDataType == PDRV_PARID_O4)
                            )
                         && (   (uSubindex <  p_ptParObj->uLoLimit) /* low limit */
                             || (uSubindex >= p_ptParObj->uHiLimit) /* high limit */
                            )
                        )
                    )
                )
            )
    { /* subindex is incorrect */
        uRspLen += uParSetError(PDRV_EV1_SUBINDEX, uSubindex, &p_ptParCxnData->uRsp[0]);
    }
    else
    { /* all is fine */
        p_ptParCxnData->uRsp[PAR_I_RSP_FMT]   = PDRV_PARID_VS;
        p_ptParCxnData->uRsp[PAR_I_RSP_NRVAL] = PAR_NAMELEN;    /* number of values = PAR_NAMELEN */
        ParPdrvStrCpy(&p_ptParCxnData->uRsp[PAR_I_RSP_VAL], p_ptParObj->pfnText(p_ptParObj, uSubindex));
        uRspLen = PAR_NAMELEN + 2U;
    }

    return uRspLen;
}

/** PROFIdrive string copy with PAR_NAMELEN
 *  @details    copy 16 characters from C string to PDRV string (completed with blanks)
 *  @return     PDRV_VOID
*/
static PDRV_VOID ParPdrvStrCpy
    (PDRV_UINT8 * p_uDest,      /**< [out] pointer to destination as C-type string */
     const char * p_cSrc        /**< [in] pointer to source with size of 16 bytes PROFIdrive string */
    )
{
    PDRV_UINT uI;

    for (uI = 0U; (uI < PAR_NAMELEN) && (*p_cSrc != '\0'); uI++)
    {
        *p_uDest++ = *p_cSrc++;
    }
    for (; uI < PAR_NAMELEN; uI++)
    {
        *p_uDest++ = ' ';
    }
}

/** returns byte size of PDRV format
 *  @details
 *  @return     returns byte size of PDRV format
*/
static PDRV_UINT uParPdrvSizeOfFmt
    (PDRV_UINT8 p_uFmt  /**< [in] PROFIdrive format PDRV_PARID_... */
    )
{
    PDRV_UINT uSize = 0U;

    switch (p_uFmt)
    {
        case PDRV_PARID_BL: /* Boolean */
        case PDRV_PARID_I1: /* Signed8 */
        case PDRV_PARID_O1: /* Unsigned8 */
        case PDRV_PARID_VS: /* VisibleString */
        case PDRV_PARID_OS: /* OctetString */
        {
            uSize = 1U;
            break;
        }
        case PDRV_PARID_I2: /* Signed16 */
        case PDRV_PARID_O2: /* Unsigned16 */
        case PDRV_PARID_US: /* UnicodeString */
        case PDRV_PARID_N2: /* N2 Normalised value (16 bit with 0x4000 == 100%) */
        case PDRV_PARID_V2: /* V2 Bit sequence (16 bit) */
        case PDRV_PARID_L2: /* L2 Nibble (16 bit) */
        case PDRV_PARID_R2: /* R2 Reciprocal time constant (16 bit: 1 ... 16384) */
        case PDRV_PARID_T2: /* T2 Time constant (16 bit: 0 ... 32767, values >32767 are identical with zero) */
        case PDRV_PARID_D2: /* D2 Time constant (16 bit: 0 ... 32767) */
        case PDRV_PARID_E2: /* E2 Fixed point value (16 bit with 1 == 2^-7) */
        case PDRV_PARID_X2: /* X2 Normalised value (16 bit with variable normalisation) */
        {
            uSize = 2U;
            break;
        }
        case PDRV_PARID_I4: /* Signed32 */
        case PDRV_PARID_O4: /* Unsigned32 */
        case PDRV_PARID_F4: /* FloatingPoint32 single precision */
        case PDRV_PARID_N4: /* N4 Normalised value (32 bit with 0x40000000 == 100%) */
        case PDRV_PARID_T4: /* T4 Time constant (32 bit) */
        case PDRV_PARID_C4: /* C4 Fixed point value (32 bit with 1 == 0.0001) */
        case PDRV_PARID_X4: /* X4 Normalised value (32 bit with variable normalisation) */
        case PDRV_PARID_TD: /* TimeDifference without date indication */
        case PDRV_PARID_TOD: /* TimeOfDay without date indication  */
        {
            uSize = 4U;
            break;
        }
        case PDRV_PARID_TODW: /* TimeOfDay (with date indication) */
        case PDRV_PARID_TDW:  /* TimeDifference with date indication */
        {
            uSize = 6U;
            break;
        }
        case PDRV_PARID_F8:     /* FloatingPoint64 double precision */
        case PDRV_PARID_I8:     /* Signed64 */
        case PDRV_PARID_O8:     /* Unsigned64 */
        case PDRV_PARID_DATE:   /* BinaryDate: 7 data bytes + 1 alignment byte */
        {
            uSize = 8U;
            break;
        }
        default:
        {
            uSize = 0U;
            break;
        }
    }

    return uSize;
}


#endif
