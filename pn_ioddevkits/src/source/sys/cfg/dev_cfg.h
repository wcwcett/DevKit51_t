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
/*  F i l e               &F: dev_cfg.h                                 :F&  */
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

/**
 * @file    dev_cfg.h
 * @brief   brief description
 * @author  cn3dit09
 * @version
 * @date    03.04.2012
 *
 * long description
 *
 */

/*****************************************************************************/

#ifndef DEV_CFG_H_
#define DEV_CFG_H_

#define dev_cfg_min__(_a,_b)                        ( (_a) < (_b) ? (_a) : (_b) )
#define dev_cfg_max__(_a,_b)                        ( (_a) > (_b) ? (_a) : (_b) )

#define DEV_CFG_MAX_ARS_RTC1_RTC2  IOD_CFG_NUMOF_IO_AR
#define DEV_CFG_MAX_ARS_RTC3       1

#define DEV_CFG_MAX_ARS_RT         dev_cfg_max__(DEV_CFG_MAX_ARS_RTC1_RTC2, DEV_CFG_MAX_ARS_RTC3)
#define DEV_CFG_MAX_ARS_DA         IOD_CFG_NUMOF_DEV_ACCESS_AR


#define DEV_CFG_IM_SUBMODULE_NR_COMMON                    0x0001
#define DEV_CFG_IM_SUBMODULE_NR_INTERFACE                 0x8000
#define DEV_CFG_IM_SUBMODULE_NR_PORT_1                    0x8001
#define DEV_CFG_IM_SUBMODULE_NR_PORT_2                    0x8002


#define DEV_CFG_IM_DEVICE_TYPE            IOD_CFG_DEVICE_TYPE
#define DEV_CFG_IM_DEVICE_NO              0x01

#define DEV_CFG_IM_VENDOR_ID              PnpbGetVendorId()

#endif /* DEV_CFG_H_ */

/*** end of file *************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
