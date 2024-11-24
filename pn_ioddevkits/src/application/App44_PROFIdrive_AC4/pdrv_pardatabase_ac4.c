/*********************************************************************************************************************/
/**@file        pdrv_pardatabase_ac4.c
 * @brief       PROFIdrive Parameter Database
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
 *              The Parameter Database implements the parameter objects.
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
#include "pdrv_parmanager_ac4.h"
#include "pdrv_diagnostics_ac4.h"
#include "pdrv_pardatabase_ac4.h"
#include "pdrv_statemachine_ac4.h"
#include "pdrv_application_ac4.h"

#if (EXAMPL_DEV_CONFIG_VERSION == 44)

/*------------  type definitions, constants, enums  ------------*/

/*------------  extern  functions  ------------*/
extern PDRV_UINT16 uPdrvUsr_GetTelegramNo(PDRV_VOID);

/** declaration of all text functions */
#define PDRV_PARAMETER(Pnu, Identifier, NrOfElements, VarAttrib, RefPar, Normalisation, StdFactor, LoLimit, HiLimit, Name, TextFunc, ReadFunc, WriteFunc) \
        extern char * TextFunc (PDRV_PAR_OBJ const *p_ptParObj, PDRV_UINT16 p_uSubindex);

#include "pdrv_parameter_ac4.inc"

#undef PDRV_PARAMETER

/** declaration of all read functions */
#define PDRV_PARAMETER(Pnu, Identifier, NrOfElements, VarAttrib, RefPar, Normalisation, StdFactor, LoLimit, HiLimit, Name, TextFunc, ReadFunc, WriteFunc) \
        extern PDRV_UINT32 ReadFunc (PDRV_PAR_OBJ const *p_ptParObj, PDRV_UINT16 p_uSubindex, PDRV_UINT16 p_uNrOfElements, PDRV_ParValues * p_ptValues);

#include "pdrv_parameter_ac4.inc"

#undef PDRV_PARAMETER

/** declaration of all read functions */
#define PDRV_PARAMETER(Pnu, Identifier, NrOfElements, VarAttrib, RefPar, Normalisation, StdFactor, LoLimit, HiLimit, Name, TextFunc, ReadFunc, WriteFunc) \
        extern PDRV_UINT32 WriteFunc (PDRV_PAR_OBJ const *p_ptParObj, PDRV_UINT16 p_uSubindex, PDRV_UINT16 p_uNrOfElements, PDRV_ParValues * p_ptValues);

#include "pdrv_parameter_ac4.inc"

#undef PDRV_PARAMETER

/*------------  extern  data  ------------*/


/*------------  type definitions, constants, enums  ------------*/

/** complete list of all parameters for implementation of parameters PNU00980 to PNU00989 */
static const PDRV_O2 m_tParList[] =
{
#define PDRV_PARAMETER(Pnu, Identifier, NrOfElements, VarAttrib, RefPar, Normalisation, StdFactor, LoLimit, HiLimit, Name, TextFunc, ReadFunc, WriteFunc) \
                       Pnu,
#include "pdrv_parameter_ac4.inc"
#undef PDRV_PARAMETER
};

#define PDRV_NOOFPARAMETERS (sizeof(m_tParList)/sizeof(m_tParList[0]))  /**< number of all parameters */

/** table with parameter objects */
static const PDRV_PAR_OBJ m_tParObjDatas[] =
{
#define PDRV_NULL_T PDRV_NULL   /**< redefinition to NULL pointer */
#define PDRV_NULL_R PDRV_NULL   /**< redefinition to NULL pointer */
#define PDRV_NULL_W PDRV_NULL   /**< redefinition to NULL pointer */
#define PDRV_PARAMETER(Pnu, Identifier, NrOfElements, VarAttrib, RefPar, Normalisation, StdFactor, LoLimit, HiLimit, Name, TextFunc, ReadFunc, WriteFunc) \
       {.uPnu = Pnu, \
        .uIdentifier = Identifier, \
        .uNrOfElements = NrOfElements, \
        .uVarAttrib = VarAttrib, \
        .uRefPar = RefPar, \
        .uNormalisation = Normalisation, \
        .fStdFactor = StdFactor, \
        .uLoLimit = LoLimit, \
        .uHiLimit = HiLimit, \
        .puName = Name, \
        .pfnText = TextFunc, \
        .pfnRead = ReadFunc, \
        .pfnWrite = WriteFunc},

#include "pdrv_parameter_ac4.inc"

#undef PDRV_PARAMETER
#undef PDRV_NULL_W
#undef PDRV_NULL_R
#undef PDRV_NULL_T
};

/** PROFIdrive search for parameter number and get pointer of found parameter object
 *  @details
 *  @return     pointer with parameter object, PDRV_NULL if not found
*/
const PDRV_PAR_OBJ * ptPdrvPar_GetParObj
    (PDRV_UINT16 p_uPnu     /**< [in] search this parameter number */
    )
{
    PDRV_PAR_OBJ const *ptParObj = PDRV_NULL;
    PDRV_UINT uI;

    for(uI = 0U; uI < PDRV_NOOFPARAMETERS; uI++)
    {
        /* parameter found? */
        if (m_tParList[uI] == p_uPnu)
        {
            ptParObj = &m_tParObjDatas[uI];
            break;
        }
    }

    return ptParObj;
}


/*------------  parameter manager assigned text functions, read functions, write functions  ------------*/

/** PROFIdrive read function for parameter PNU00964 "drive unit identification"
 *  @details
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00964
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    static const PDRV_O2 uPnu00964[6] =
    {
      [0U] = PDRV_ID_MANUFACTURER,
      [1U] = PDRV_ID_DUTYPE,
      [2U] = PDRV_ID_FWVERSION,
      [3U] = PDRV_ID_FWDATE_Y,
      [4U] = PDRV_ID_FWDATA_DM,
      [5U] = 1          /* number of Drive Objects: 1 */
    };
    PDRV_UINT uI;

    for (uI = 0; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->o2[uI] = uPnu00964[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;
}


/** PROFIdrive read function for parameter PNU00965 "profile identification number"
 *  @details    none
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00965
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    static const PDRV_O1 uPnu00965[2] =
    {
      [0U] = 3,     /* Profile number */
      [1U] = 42     /* PROFIdrive profile version V4.2 */
    };
    PDRV_UINT uI;

    for (uI = 0; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->o1[uI] = uPnu00965[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;
}


/** PROFIdrive read function for parameter PNU00974 "Base Mode Parameter Access service identification"
 *  @details    none
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00974
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    static const PDRV_O2 uPnu00974[3] =
    {
       [0U] = PDRV_PAR_BLOCKSIZE,  /* maximum block length = default block length */
       [1U] = 1,  /* maximum number of parameter requests per multiparameter request = 1 (no multiparameter requests) */
       [2U] = 0,  /* maximum latency per request = 0 means no specification */
    };
    PDRV_UINT uI;

    for (uI = 0; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->o2[uI] = uPnu00974[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;
}


/** PROFIdrive read function for parameter PNU00975 "drive object identification"
 *  @details    none
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00975
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_O2 uPnu00975[8] =
    {
       [0U] = PDRV_ID_MANUFACTURER,
       [1U] = PDRV_ID_DOTYPE,
       [2U] = PDRV_ID_FWVERSION,
       [3U] = PDRV_ID_FWDATE_Y,
       [4U] = PDRV_ID_FWDATA_DM,
       [5U] = 1,         /* PROFIdrive DO type class = 1 */
       [6U] = 8,         /* PROFIdrive DO sub class = AC4 */
       [7U] = 1          /* Drive ObjectID = 1 */
    };
    PDRV_UINT uI;

    if(uPdrvUsr_GetTelegramNo()==PDRV_STDTLG1 || uPdrvUsr_GetTelegramNo()==PDRV_STDTLG2)	//AC1
    {
    	uPnu00975[6] = 1;
    }
    else if(uPdrvUsr_GetTelegramNo()==PDRV_STDTLG7 || uPdrvUsr_GetTelegramNo()==PDRV_STDTLG9)	//AC3
    {
    	uPnu00975[6] = 4;
    }
    else if(uPdrvUsr_GetTelegramNo()==PDRV_STDTLG3 || uPdrvUsr_GetTelegramNo()==PDRV_STDTLG4 || uPdrvUsr_GetTelegramNo()==PDRV_STDTLG5 || uPdrvUsr_GetTelegramNo()==PDRV_STDTLG6)	//AC4
    {
    	uPnu00975[6] = 8;
    }

    for (uI = 0; uI < p_uNrOfElements; uI++)
    {
        p_ptValues->o2[uI] = uPnu00975[p_uSubindex + uI];
    }
    return PDRV_EV1_NOERROR;
}


/** PROFIdrive read function for parameter PNU00980 "Number list of defined parameter"
 *  @details    none
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu00980
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    PDRV_UINT uJ = p_uSubindex;
    PDRV_UINT uI;

    for (uI = 0U; uI < p_uNrOfElements; uI++)
    {
        /* Don't list the parameter PNU980..PNU989. see PDRV V4.2 table 144 */
        while (    (980U <= m_tParList[uJ])
                && (m_tParList[uJ] <= 989U))
        {
            uJ++;
        }
        p_ptValues->o2[uI] = (uJ < PDRV_NOOFPARAMETERS) ? m_tParList[uJ] : 0U;
        uJ++;
    }
    return PDRV_EV1_NOERROR;
}

/** PROFIdrive read function for parameter PNU60000 "Velocity reference value"
 *  @details    none
 *  @return     PROFIdrive error number in Base mode parameter responses (starts with PDRV_EV1_...)
*/
PDRV_UINT32 uPdrv_RfPnu60000
    (const PDRV_PAR_OBJ *p_ptParObj,    /**< [in] pointer to the parameter object datas */
     PDRV_UINT16 p_uSubindex,           /**< [in] first subindex to read/write */
     PDRV_UINT16 p_uNrOfElements,       /**< [in] number of elements to read */
     PDRV_ParValues * p_ptValues        /**< [in, out] buffer pointer for write/read values */
    )
{
    p_ptValues->f4[0U] = FLOAT_3000;    /* AC1 example supports 3000 rmin only */
    return PDRV_EV1_NOERROR;
}

#endif
