/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* Description:
* Author: huawei
* Create: 2023-1-13
*/
#include <linux/io.h>
#include "tsdrv_fault_init.h"
#include "tsdrv_common_fault_platform.h"
#include "tsdrv_log.h"
#include "tsdrv_common.h"
#include "devdrv_user_common.h"
#include "tsdrv_firmware_load.h"
#include "devdrv_platform.h"
#include "tsdrv_fault_comm.h"
#include "soc_res.h"
#include "dfm_dev_register.h"
#include "dms_sensor_type.h"
#include "dms_sensor.h"
#include "drv_ras_common.h"
#include "tsdrv_ras.h"
#include "tsdrv_tscpu_fault.h"
#include "tsdrv_fault_platform.h"

#ifdef CFG_FEATURE_FAULT_FPDC
static struct ras_fault_converge_item fault_converge_table[] = {
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TSCPU, DMS_DEV_TYPE_TSCPU, RAS_SEC_GENERIC, RAS_CODE_TSCPU,
        "tscpu module err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_ERROR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_SRAM_MULTI_BIT_ECC_UER,
        "l2buf multi bit ecc err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_SRAM_SINGLE_BIT_ECC_CE,
        "l2buf single bit ecc err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_CFG_0,
        "l2buf invalid security attr", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_CFG_1,
        "l2buf invalid access request", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_CFG_2,
        "l2buf invalid read request", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_CFG_3,
        "l2buf invalid write request", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_CFG_4,
        "l2buf invalid ptl size access", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_CFG_5,
        "l2buf invalid ptl size access", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_CFG_6,
        "l2buf access addr out-of-bound", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_CFG_7,
        "l2buf invalid addr attr access", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_CFG_8,
        "l2buf atomic fp_overflo err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_CFG_9,
        "l2buf atomic fp_inputinf err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_CFG_10,
        "l2buf atomic fp_srcnan err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_CFG_11,
        "l2buf atomic fp_destnan err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_CFG_12,
        "l2buf atomic fp_bothnan err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_CFG_13,
        "l2buf atomic int_overflow err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, L2BUFF_MODULE_ID, RAS_SEC_GENERIC, RAS_L2BUF_CFG_14,
        "l2buf atomic notequal err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_CFG_ERR,
        "ts disp cfg error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_CFG_ERR,
        "aic disp cfg error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_CFG_ERR,
        "dsa disp cfg error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_WD,
        "ts disp wd input error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_WD,
        "aic disp wd input error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_WD,
        "dsa disp wd input error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_0,
        "ts disp rsp input error 0", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_0,
        "aic disp rsp input error 0", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_0,
        "dsa disp rsp input error 0", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_1,
        "ts disp rsp input error 1", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_1,
        "aic disp rsp input error 1", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_1,
        "dsa disp rsp input error 1", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_2,
        "ts disp rsp input error 2", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_2,
        "aic disp rsp input error 2", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_2,
        "dsa disp rsp input error 2", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_3,
        "ts disp rsp input error 3", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_3,
        "aic disp rsp input error 3", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_3,
        "dsa disp rsp input error 3", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_4,
        "ts disp rsp input error 4", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_4,
        "aic disp rsp input error 4", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_4,
        "dsa disp rsp input error 4", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_5,
        "ts disp rsp input error 5", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_5,
        "aic disp rsp input error 5", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_5,
        "dsa disp rsp input error 5", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_6,
        "ts disp rsp input error 6", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_6,
        "aic disp rsp input error 6", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_6,
        "dsa disp rsp input error 6", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_7,
        "ts disp rsp input error 7", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_7,
        "aic disp rsp input error 7", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_7,
        "dsa disp rsp input error 7", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_8,
        "ts disp rsp input error 8", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_8,
        "aic disp rsp input error 8", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_INPUT_ERR_RSP_8,
        "dsa disp rsp input error 8", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC,
        RAS_DISP_DATRAM_MULTI_BIT_ECC,
        "ts disp datram multi bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_DATRAM_MULTI_BIT_ECC,
        "aic disp datram multi bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_DATRAM_MULTI_BIT_ECC,
        "dsa disp datram multi bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC,
        RAS_DISP_CMDRAM_MULTI_BIT_ECC,
        "ts disp cmdram multi bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_CMDRAM_MULTI_BIT_ECC,
        "aic disp cmdram multi bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_CMDRAM_MULTI_BIT_ECC,
        "dsa disp cmdram multi bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC,
        RAS_DISP_NCDIR_MULTI_BIT_ECC,
        "ts disp ndram multi bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_NCDIR_MULTI_BIT_ECC,
        "aic disp ndram multi bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_NCDIR_MULTI_BIT_ECC,
        "dsa disp ndram multi bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC,
        RAS_DISP_DATRAM_SINGLE_BIT_ECC,
        "ts disp datram single bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_DATRAM_SINGLE_BIT_ECC,
        "aic disp datram single bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_DATRAM_SINGLE_BIT_ECC,
        "dsa disp datram single bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC,
        RAS_DISP_CMDRAM_SINGLE_BIT_ECC,
        "ts disp cmdram single bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_CMDRAM_SINGLE_BIT_ECC,
        "aic disp cmdram single bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_CMDRAM_SINGLE_BIT_ECC,
        "dsa disp cmdram single bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TS_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC,
        RAS_DISP_NCDIR_SINGLE_BIT_ECC,
        "ts disp ndram single bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_AIC_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_NCDIR_SINGLE_BIT_ECC,
        "aic disp ndram single bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_DSA_DISP, DISP_MODULE_ID, RAS_SEC_GENERIC, RAS_DISP_NCDIR_SINGLE_BIT_ECC,
        "dsa disp ndram single bit ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold),
};

struct ras_fault_converge_item *tsdrv_get_fault_converge_table(void)
{
    return fault_converge_table;
}

int tsdrv_get_fault_converge_table_size(void)
{
    return (int)ARRAY_SIZE(fault_converge_table);
}
#endif

#define SENSOR_TABLE_OBJ(_devid) \
    { \
        /* STARS */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "stars", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_HWTS, \
            SENSOR_PRIV_DATA(_devid, 0, DMS_DEV_TYPE_HWTS_S_TS, DEV_NODE_HWTS_STARS, SENSOR_NODE_HWTS_STARS), \
            0xFFF, 0xFBF), \
        /* TS doorbell */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "ts", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_TS, \
            SENSOR_PRIV_DATA(_devid, 0, DMS_DEV_TYPE_TS, DEV_NODE_TS, SENSOR_NODE_TS), 0xFFF, 0xFBF), \
        /* AIC */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "aic", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_AIC, \
            SENSOR_PRIV_DATA(_devid, 0, DMS_DEV_TYPE_AIC, DEV_NODE_AIC, SENSOR_NODE_AIC), 0xFFF, 0xFBF), \
        /* SDMAM */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "sdma", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_SDMA, \
            SENSOR_PRIV_DATA(_devid, 0, DMS_DEV_TYPE_SDMA, DEV_NODE_SDMAM, SENSOR_NODE_SDMAM), 0xFFF, 0xFBF), \
        /* DSA */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "dsa", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_DSA, \
            SENSOR_PRIV_DATA(_devid, 0, DMS_DEV_TYPE_DSA, DEV_NODE_DSA, SENSOR_NODE_DSA), 0xFFF, 0xFBF), \
        /* mcu heartbeat */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_HEARTBEAT, "mcu_heartbeat", DMS_DISCRETE_SENSOR_CLASS, \
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL, \
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, TSDRV_FAULT_EVENT_SCAN_TSFW, \
            SENSOR_PRIV_DATA(_devid, 0, DMS_DEV_TYPE_TSFW, DEV_NODE_TSFW, SENSOR_NODE_TSFW), 0xFFF, 0xFBF), \
        /* Invalid sensor obj */ \
        GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_MAX_CUSTOM, "invalid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), \
    }

static struct dms_sensor_object_cfg g_sensor_obj_table[TSDRV_MAX_DAVINCI_NUM][DMS_MAX_NODE_SENSOR_COUNT] = {
    SENSOR_TABLE_OBJ(0x0ULL), /* dev0 */
    SENSOR_TABLE_OBJ(0x1ULL), /* dev1 */
    SENSOR_TABLE_OBJ(0x2ULL), /* dev2 */
    SENSOR_TABLE_OBJ(0x3ULL), /* dev3 */
    SENSOR_TABLE_OBJ(0x4ULL), /* dev4 */
    SENSOR_TABLE_OBJ(0x5ULL), /* dev5 */
    SENSOR_TABLE_OBJ(0x6ULL), /* dev6 */
    SENSOR_TABLE_OBJ(0x7ULL), /* dev7 */
};

struct dms_sensor_object_cfg *tsdrv_get_sensor_obj_table(u32 devid)
{
    return g_sensor_obj_table[devid];
}

#ifdef CFG_FEATURE_FAULT_FPDC
void tsdrv_fill_tscpu_sensor_status(struct ras_error *error)
{
    error->section_type = RAS_SEC_GENERIC;
    error->ras_code = RAS_CODE_TSCPU;
    error->sensor_status = RAS_ERROR_TYPE_ERROR;
}

bool fault_is_enable_unmask_ras_irp(u32 devid)
{
    return true;
}

#define RAS_REG_SIZE 0x10000

#define INT_TYP0_ENA   0x08 /* TYP0 is CE */
#define INT_TYP0_ENA_H 0x0C
#define INT_TYP1_ENA   0x10 /* TYP1 is Non-fatal */
#define INT_TYP1_ENA_H 0x14
#define RAS_IQR_REG_SIZE 32

#define L2BUFF_RAS0_L_MASK 0x3FF      /* RAS0: L2BUFF0 ~ L2BUFF9 */
#define L2BUFF_RAS0_H_MASK 0xFC00     /* RAS0: L2BUFF10 ~ L2BUFF15 */
#define L2BUFF_RAS2_L_MASK 0xFFFF0000 /* RAS2: L2BUFF16 ~ L2BUFF31 */

static u32 l2buff_ras_irq_masks[L2BUFF_NUM] = {
    22, 23, 24, 25, 26, 27, 28, 29, /* RAS0 */
    30, 31, 32, 33, 34, 35, 36, 37, /* RAS0 */
    16, 17, 18, 19, 20, 21, 22, 23, /* RAS2 */
    24, 25, 26, 27, 28, 29, 30, 31  /* RAS2 */
};

static u32 fault_get_l2buff_irq_mask(u32 node_id)
{
    u32 mask = (l2buff_ras_irq_masks[node_id] < RAS_IQR_REG_SIZE) ?
        l2buff_ras_irq_masks[node_id] : l2buff_ras_irq_masks[node_id] - RAS_IQR_REG_SIZE;

    return (1U << mask);
}

static void fault_unmask_l2buff_irq(u64 ras0_addr, u64 ras2_addr, u32 devid, u32 node_id)
{
    u32 mask = fault_get_l2buff_irq_mask(node_id);

    if ((1ULL << node_id) & L2BUFF_RAS0_L_MASK) {
        (void)ras_int_unmask_reg_write(ras0_addr + INT_TYP0_ENA, mask, mask);
        (void)ras_int_unmask_reg_write(ras0_addr + INT_TYP1_ENA, mask, mask);
    }
    if ((1ULL << node_id) & L2BUFF_RAS0_H_MASK) {
        (void)ras_int_unmask_reg_write(ras0_addr + INT_TYP0_ENA_H, mask, mask);
        (void)ras_int_unmask_reg_write(ras0_addr + INT_TYP1_ENA_H, mask, mask);
    }
    if ((1ULL << node_id) & L2BUFF_RAS2_L_MASK) {
        (void)ras_int_unmask_reg_write(ras2_addr + INT_TYP0_ENA, mask, mask);
        (void)ras_int_unmask_reg_write(ras2_addr + INT_TYP1_ENA, mask, mask);
    }
}

/* RAS0: AIC_DISP0~AIC_DISP2, AIC_DISP6~AIC_DISP8, AIC_DISP12~AIC_DISP14, AIC_DISP18~AIC_DISP20 */
#define AIC_DISP_RAS0_H_MASK 0x1C71C7
/* RAS1: AIC_DISP24 */
#define AIC_DISP_RAS1_L_MASK 0x1000000
/* RAS3: AIC_DISP3~AIC_DISP5, AIC_DISP9~AIC_DISP11, AIC_DISP15~AIC_DISP17, AIC_DISP21~AIC_DISP23 */
#define AIC_DISP_RAS3_L_MASK 0xE38E38

static u32 aic_disp_ras_irq_masks[AIC_DISP_NUM] = {
    47, 48, 49, /* RAS0 */
    2, 3, 4,    /* RAS3 */
    50, 51, 52, /* RAS0 */
    5, 6, 7,    /* RAS3 */
    53, 54, 55, /* RAS0 */
    8, 9, 10,   /* RAS3 */
    56, 57, 58, /* RAS0 */
    11, 12, 13, /* RAS3 */
    0           /* RAS1 */
};

static u32 fault_get_aic_disp_irq_mask(u32 node_id)
{
    u32 mask = (aic_disp_ras_irq_masks[node_id] < RAS_IQR_REG_SIZE) ?
        aic_disp_ras_irq_masks[node_id] : aic_disp_ras_irq_masks[node_id] - RAS_IQR_REG_SIZE;

    return (1U << mask);
}

void fault_unmask_aic_disp_irq(u64 ras0_addr, u64 ras2_addr, u32 devid, u32 node_id)
{
    u32 mask = fault_get_aic_disp_irq_mask(node_id);

    if ((1ULL << node_id) & AIC_DISP_RAS0_H_MASK) {
        (void)ras_int_unmask_reg_write(ras0_addr + INT_TYP0_ENA_H, mask, mask);
        (void)ras_int_unmask_reg_write(ras0_addr + INT_TYP1_ENA_H, mask, mask);
    }
    if ((1ULL << node_id) & AIC_DISP_RAS1_L_MASK) {
        (void)ras_int_unmask_reg_write(ras0_addr + RAS_REG_SIZE + INT_TYP0_ENA, mask, mask);
        (void)ras_int_unmask_reg_write(ras0_addr + RAS_REG_SIZE + INT_TYP1_ENA, mask, mask);
    }
    if ((1ULL << node_id) & AIC_DISP_RAS3_L_MASK) {
        (void)ras_int_unmask_reg_write(ras2_addr + RAS_REG_SIZE + INT_TYP0_ENA, mask, mask);
        (void)ras_int_unmask_reg_write(ras2_addr + RAS_REG_SIZE + INT_TYP1_ENA, mask, mask);
    }
}

#define STARS_DISP_RAS_IRQ_MASK 0x400  /* bit 42 */
void fault_unmask_stars_disp_irq(u64 ras0_addr, u64 ras2_addr, u32 devid, u32 node_id)
{
    (void)ras_int_unmask_reg_write(ras0_addr + INT_TYP0_ENA_H, STARS_DISP_RAS_IRQ_MASK, STARS_DISP_RAS_IRQ_MASK);
    (void)ras_int_unmask_reg_write(ras0_addr + INT_TYP1_ENA_H, STARS_DISP_RAS_IRQ_MASK, STARS_DISP_RAS_IRQ_MASK);
}

#define DSA_DISP_RAS_IRQ_MASK 0x100000 /* bit 52 */
void fault_unmask_dsa_disp_irq(u64 ras0_addr, u64 ras2_addr, u32 devid, u32 node_id)
{
    (void)ras_int_unmask_reg_write(ras2_addr + INT_TYP0_ENA_H, DSA_DISP_RAS_IRQ_MASK, DSA_DISP_RAS_IRQ_MASK);
    (void)ras_int_unmask_reg_write(ras2_addr + INT_TYP1_ENA_H, DSA_DISP_RAS_IRQ_MASK, DSA_DISP_RAS_IRQ_MASK);
}

#define TSCPU0_RAS_IRQ_MASK 0x2000 /* bit 45 */
#define TSCPU1_RAS_IRQ_MASK 0x4000 /* bit 46 */
void fault_unmask_tscpu_irq(u64 ras0_addr, u64 ras2_addr, u32 devid, u32 node_id)
{
    if (node_id == 0) {
        (void)ras_int_unmask_reg_write(ras0_addr + INT_TYP0_ENA_H, TSCPU0_RAS_IRQ_MASK, TSCPU0_RAS_IRQ_MASK);
        (void)ras_int_unmask_reg_write(ras0_addr + INT_TYP1_ENA_H, TSCPU0_RAS_IRQ_MASK, TSCPU0_RAS_IRQ_MASK);
    }
    if (node_id == 1) {
        (void)ras_int_unmask_reg_write(ras0_addr + INT_TYP0_ENA_H, TSCPU1_RAS_IRQ_MASK, TSCPU1_RAS_IRQ_MASK);
        (void)ras_int_unmask_reg_write(ras0_addr + INT_TYP1_ENA_H, TSCPU1_RAS_IRQ_MASK, TSCPU1_RAS_IRQ_MASK);
    }
}

void fault_unmask_ras_irq(struct work_struct *work)
{
    struct soc_reg_base_info ras0_base;
    struct soc_reg_base_info ras2_base;
    struct res_inst_info res_inst;
    struct dfm_node *node = container_of(work, struct dfm_node, unmask_irq_work.work);
    u32 devid = node->devid;
    int node_type = node->node_type;
    u32 node_id = node->node_id;
    int ret;

    soc_resmng_inst_pack(&res_inst, devid, TS_SUBSYS, 0);
    ret = soc_resmng_get_reg_base(&res_inst, "RAS0_REG", &ras0_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to get ras0 reg base. (ret=%d; devid=%u)\n", ret, devid);
        return;
    }

    soc_resmng_inst_pack(&res_inst, devid, TS_SUBSYS, 0);
    ret = soc_resmng_get_reg_base(&res_inst, "RAS2_REG", &ras2_base);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to get ras2 reg base. (ret=%d; devid=%u)\n", ret, devid);
        return;
    }

    TSDRV_PRINT_DEBUG("(devid=%u; node_type=0x%x; node_id=%u)\n", devid, node_type, node_id);

    if (node_type == DMS_DEV_TYPE_L2BUF) {
        fault_unmask_l2buff_irq(ras0_base.io_base, ras2_base.io_base, devid, node_id);
    }

    if (node_type == DMS_DEV_TYPE_AIC_DISP) {
        fault_unmask_aic_disp_irq(ras0_base.io_base, ras2_base.io_base, devid, node_id);
    }

    if (node_type == DMS_DEV_TYPE_TS_DISP) {
        fault_unmask_stars_disp_irq(ras0_base.io_base, ras2_base.io_base, devid, node_id);
    }

    if (node_type == DMS_DEV_TYPE_DSA_DISP) {
        fault_unmask_dsa_disp_irq(ras0_base.io_base, ras2_base.io_base, devid, node_id);
    }

    if (node_type == DMS_DEV_TYPE_TSCPU) {
        fault_unmask_tscpu_irq(ras0_base.io_base, ras2_base.io_base, devid, node_id);
        TSDRV_PRINT_DEBUG("Unmask tscpu irq success. (devid=%u; node_type=0x%x; node_id=%u)\n",
            devid, node_type, node_id);
    }
}

static struct dfm_struct l2buff_dms[TSDRV_MAX_DAVINCI_NUM];
static struct dfm_struct *tsdrv_get_l2buff_dms(u32 devid)
{
    return &l2buff_dms[devid];
}

static struct dms_node g_l2buff_dms_nodes[TSDRV_MAX_DAVINCI_NUM][L2BUFF_NUM];
static struct dms_node *tsdrv_get_l2buff_dms_nodes(u32 devid, u32 node_id)
{
    return &g_l2buff_dms_nodes[devid][node_id];
}

static struct dms_sensor_object_cfg g_l2buff_sensor_cfg[] = {
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "l2buff", DMS_DISCRETE_SENSOR_CLASS,
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL,
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, tsdrv_ras_event_scan,
        SENSOR_PRIV_DATA(0, 0, 0, 0, DMS_SEN_TYPE_RAS_SENSOR), 0xFFFF, 0xFFBF),
};
static struct dms_sensor_object_cfg *tsdrv_get_l2buff_sensor_cfg(u32 *sensor_num)
{
    *sensor_num = sizeof(g_l2buff_sensor_cfg) / sizeof(g_l2buff_sensor_cfg[0]);
    return g_l2buff_sensor_cfg;
}

static struct dfm_struct aic_disp_dms[TSDRV_MAX_DAVINCI_NUM];
static struct dfm_struct *tsdrv_get_aic_disp_dms(u32 devid)
{
    return &aic_disp_dms[devid];
}

static struct dms_node g_aic_disp_dms_nodes[TSDRV_MAX_DAVINCI_NUM][AIC_DISP_NUM];
static struct dms_node *tsdrv_get_aic_disp_dms_nodes(u32 devid, u32 node_id)
{
    return &g_aic_disp_dms_nodes[devid][node_id];
}

static struct dms_sensor_object_cfg g_aic_disp_sensor_cfg[] = {
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "aic_disp", DMS_DISCRETE_SENSOR_CLASS,
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL,
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, tsdrv_ras_event_scan,
        SENSOR_PRIV_DATA(0, 0, 0, 0, DMS_SEN_TYPE_RAS_SENSOR), 0xFFFF, 0xFFBF),
};
static struct dms_sensor_object_cfg *tsdrv_get_aic_disp_sensor_cfg(u32 *sensor_num)
{
    *sensor_num = sizeof(g_aic_disp_sensor_cfg) / sizeof(g_aic_disp_sensor_cfg[0]);
    return g_aic_disp_sensor_cfg;
}

static struct dfm_struct ts_disp_dms[TSDRV_MAX_DAVINCI_NUM];
static struct dfm_struct *tsdrv_get_ts_disp_dms(u32 devid)
{
    return &ts_disp_dms[devid];
}

static struct dms_node g_ts_disp_dms_nodes[TSDRV_MAX_DAVINCI_NUM][TS_DISP_NUM];
static struct dms_node *tsdrv_get_ts_disp_dms_nodes(u32 devid, u32 node_id)
{
    return &g_ts_disp_dms_nodes[devid][node_id];
}

static struct dms_sensor_object_cfg g_ts_disp_sensor_cfg[] = {
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "ts_disp", DMS_DISCRETE_SENSOR_CLASS,
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL,
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, tsdrv_ras_event_scan,
        SENSOR_PRIV_DATA(0, 0, 0, 0, DMS_SEN_TYPE_RAS_SENSOR), 0xFFFF, 0xFFBF),
};
static struct dms_sensor_object_cfg *tsdrv_get_ts_disp_sensor_cfg(u32 *sensor_num)
{
    *sensor_num = sizeof(g_ts_disp_sensor_cfg) / sizeof(g_ts_disp_sensor_cfg[0]);
    return g_ts_disp_sensor_cfg;
}

static struct dfm_struct dsa_disp_dms[TSDRV_MAX_DAVINCI_NUM];
static struct dfm_struct *tsdrv_get_dsa_disp_dms(u32 devid)
{
    return &dsa_disp_dms[devid];
}

static struct dms_node g_dsa_disp_dms_nodes[TSDRV_MAX_DAVINCI_NUM][DSA_DISP_NUM];
static struct dms_node *tsdrv_get_dsa_disp_dms_nodes(u32 devid, u32 node_id)
{
    return &g_dsa_disp_dms_nodes[devid][node_id];
}

static struct dms_sensor_object_cfg g_dsa_disp_sensor_cfg[] = {
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "dsa_disp", DMS_DISCRETE_SENSOR_CLASS,
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL,
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, tsdrv_ras_event_scan,
        SENSOR_PRIV_DATA(0, 0, 0, 0, DMS_SEN_TYPE_RAS_SENSOR), 0xFFFF, 0xFFBF),
};
static struct dms_sensor_object_cfg *tsdrv_get_dsa_disp_sensor_cfg(u32 *sensor_num)
{
    *sensor_num = sizeof(g_dsa_disp_sensor_cfg) / sizeof(g_dsa_disp_sensor_cfg[0]);
    return g_dsa_disp_sensor_cfg;
}

struct dfm_struct *tsdrv_get_dfm_by_node_type(u32 devid, int node_type)
{
    switch (node_type) {
        case DMS_DEV_TYPE_L2BUF:
            return tsdrv_get_l2buff_dms(devid);
        case DMS_DEV_TYPE_AIC_DISP:
            return tsdrv_get_aic_disp_dms(devid);
        case DMS_DEV_TYPE_TS_DISP:
            return tsdrv_get_ts_disp_dms(devid);
        case DMS_DEV_TYPE_DSA_DISP:
            return tsdrv_get_dsa_disp_dms(devid);
        case DMS_DEV_TYPE_TSCPU:
            return tsdrv_get_tscpu_dms(devid);
        default:
            TSDRV_PRINT_ERR("Para error. (devid=%u; node_type=%d)\n", devid, node_type);
            return NULL;
    }
}

struct dms_node *tsdrv_get_dms_node(u32 devid, int node_type, u32 node_id)
{
    switch (node_type) {
        case DMS_DEV_TYPE_L2BUF:
            return tsdrv_get_l2buff_dms_nodes(devid, node_id);
        case DMS_DEV_TYPE_AIC_DISP:
            return tsdrv_get_aic_disp_dms_nodes(devid, node_id);
        case DMS_DEV_TYPE_TS_DISP:
            return tsdrv_get_ts_disp_dms_nodes(devid, node_id);
        case DMS_DEV_TYPE_DSA_DISP:
            return tsdrv_get_dsa_disp_dms_nodes(devid, node_id);
        case DMS_DEV_TYPE_TSCPU:
            return tsdrv_get_tscpu_dms_nodes(devid, node_id);
        default:
            return NULL;
    }
}

struct dms_sensor_object_cfg *tsdrv_get_sensor_cfg(int node_type, u32 *sensor_num)
{
    switch (node_type) {
        case DMS_DEV_TYPE_L2BUF:
            return tsdrv_get_l2buff_sensor_cfg(sensor_num);
        case DMS_DEV_TYPE_AIC_DISP:
            return tsdrv_get_aic_disp_sensor_cfg(sensor_num);
        case DMS_DEV_TYPE_TS_DISP:
            return tsdrv_get_ts_disp_sensor_cfg(sensor_num);
        case DMS_DEV_TYPE_DSA_DISP:
            return tsdrv_get_dsa_disp_sensor_cfg(sensor_num);
        case DMS_DEV_TYPE_TSCPU:
            return tsdrv_get_tscpu_sensor_cfg(sensor_num);
        default:
            *sensor_num = 0;
            return NULL;
    }
}

static struct tsdrv_ras_node_info ras_nodes_info[] = {
    {DMS_DEV_TYPE_L2BUF, DMS_DEV_TYPE_L2BUF, L2BUFF_NUM},
    {DMS_DEV_TYPE_AIC, DMS_DEV_TYPE_AIC_DISP, AIC_DISP_NUM},
    {DMS_DEV_TYPE_TS, DMS_DEV_TYPE_TS_DISP, TS_DISP_NUM},
    {DMS_DEV_TYPE_DSA, DMS_DEV_TYPE_DSA_DISP, DSA_DISP_NUM},
    {DMS_DEV_TYPE_TSCPU, DMS_DEV_TYPE_TSCPU, TSCPU_NUM}
};

void tsdrv_get_ras_node_types(struct tsdrv_ras_node_info **ras_nodes, int *num)
{
    *ras_nodes = ras_nodes_info;
    *num = ARRAY_SIZE(ras_nodes_info);
}

int fault_remap_l2buf(u32 devid)
{
    return 0;
}

void fault_unmap_l2buf(u32 devid)
{
}
#endif

