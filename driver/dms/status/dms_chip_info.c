/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/

#ifndef AOS_LLVM_BUILD
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "securec.h"
#include "dms_define.h"
#include "dms_cmd_def.h"
#include "devdrv_user_common.h"
#include "hvtsdrv_tsagent.h"
#include "devdrv_common.h"
#include "devdrv_manager_container.h"
#include "devdrv_pcie.h"
#include "dms_chip_info.h"

#ifdef CFG_SOC_PLATFORM_MDC_V51
#include "devdrv_manager_comm.h"
#include "devmng_dms_adapt.h"
#endif

typedef enum {
    CORE_NUM_A_INDEX = 1,
    CORE_NUM_B_INDEX,
    CORE_NUM_C_INDEX,
    CORE_NUM_LEVEL_MAX
} AICORE_NUM_LEVEL;

typedef enum {
    FREQ_LITE_INDEX = 1,
    FREQ_PRO_INDEX,
    FREQ_PRE_INDEX,
    CORE_FREQ_LEVEL_MAX,
} AICORE_FREQ_LEVEL;

/* add for ai computing power */
typedef enum {
    AI_COMPUTING_LEVEL1 = 0,
    AI_COMPUTING_LEVEL2,
    AI_COMPUTING_LEVEL3,
    AI_COMPUTING_LEVEL4,
} AI_COMPUTING_LEVEL;

#define MINIV2_PRO_CORE_NUM 10
#define MINIV2_CORE_NUM 8

#define DEV_AICORE_FREQ_LEVEL_CLOUD 1
#define DEV_AICORE_FREQ_LEVEL_MINIV2 2

/* add for partial good */
#define CLOUD_FULL_GOOD_CORE_NUM 32
#define AICORE_FREQ_LEVEL_PRO 1100
#define AICORE_FREQ_LEVEL_PRE 1200

#define AICORE_LEVEL_LIMIT(NumLevel, FreqLevel) (((NumLevel) > 0) &&    \
    ((NumLevel) < CORE_NUM_LEVEL_MAX) && ((FreqLevel) > 0) &&          \
    ((FreqLevel) < CORE_FREQ_LEVEL_MAX))

#ifdef CFG_SOC_PLATFORM_MDC_V51
#define BIST_VALID    1
#define BIST_INVALID  0
#define BIST_MAGIC  0x42495354U   // "BIST"
#define BIST_MAGIC_CLEAR  0x4E4F4E45U   // "NONE"
#define POWERON_BIST_RESULT_SRAM_ADDR  0xC6F36200
#define POWERON_BIST_RESULT_SRAM_SIZE  (sizeof(BIOS_BIST_INFO))
#define BIST_RESULT_SRAM_ADDR  0xC6F36400
#define BIST_RESULT_SRAM_SIZE  0x100

#define BIST_SIL_HW_ERR_SHIFT  16
#define BIST_LP_HW_ERR_SHIFT   24
#define BIST_HW_ERR_CODE_MASK  0xFF
#define MIN_BIST_VEC_TIME 1U        // 单位ms
#define MAX_BIST_VEC_TIME 1000U     // 单位ms
#define MIN_BIST_MODE_SECONDES 1    // 单位s
#define MAX_BIST_MODE_SECONDES 255  // 单位s
#endif

STATIC char *g_aicore_freq_level[] = {"NULL", "", "Pro", "Premium"};
STATIC char *g_aicore_num_level[] = {"NULL", "A", "B", "C"};

#ifndef CFG_SOC_PLATFORM_MDC_V51
STATIC char *g_ai_computing_level[] = {"1", "2", "3", "4"};
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC volatile BIST_MODE_STRU g_bist_mode = {0, 0, 0};
STATIC volatile BIST_RESULT_STRU g_dms_bist_rslt = {0, {{0}, {0}}, {0}, {0}, {0}, {0}};
STATIC volatile BIST_VECTOR_INFO g_bist_vector_info = {0, 0, 0, {0}};
#endif

typedef struct chip_value_name_map {
    unsigned int chip_value;
    char *chip_name;
} chip_value_name_map_t;

#define MINI_CHIP_NAME_VALUE 6416

/* chip value 6416/6528/6481 means miniv1/cloud/miniv2 */
STATIC chip_value_name_map_t g_chip_value_name_map[] = {
    { 6416, "310" },
    { 6528, "910" },
#ifdef CFG_SOC_PLATFORM_MDC_V51
    { 6481, "610" },
#else
    { 6481, "310P" },
#endif
    { 6417, "310B" },
    { 6529, "910B" },
};

typedef struct dms_chip_aicore_info {
    unsigned int chip_ver;
    unsigned int chip_name;
    unsigned int num;
    unsigned int num_level;
    unsigned int freq;
    unsigned int freq_level;
} dms_chip_aicore_info_t;

STATIC int dms_get_chip_aicore_info(unsigned int dev_id, unsigned int vfid, bool is_container_split,
    dms_chip_aicore_info_t *chip_aicore_info)
{
    int ret;
    struct devdrv_info *dev_info = NULL;
    struct dms_dev_ctrl_block *dev_cb = NULL;
    struct devdrv_platform_data *pdata = NULL;

    dev_cb = dms_get_dev_cb(dev_id);
    if (dev_cb == NULL) {
        dms_err("Get device ctrl block failed. (dev_id=%u)\n", dev_id);
        return -ENODEV ;
    }

    if (dev_cb->dev_info == NULL) {
        dms_err("Device ctrl dev_info is null. (dev_id=%u)\n", dev_id);
        return -ENODEV ;
    }

    dev_info = (struct devdrv_info *)dev_cb->dev_info;
    pdata = dev_info->pdata;
    chip_aicore_info->freq_level = pdata->ai_core_freq_level;
    chip_aicore_info->num_level = pdata->ai_core_num_level;
    chip_aicore_info->freq = dev_info->aicore_freq;
    chip_aicore_info->chip_name = dev_info->chip_name;
    chip_aicore_info->chip_ver = dev_info->chip_version;
    if (is_container_split == true) {
        ret = hvdevmng_get_aicore_num(dev_id, vfid, &chip_aicore_info->num);
        if (ret != 0) {
            dms_err("Get container aicore num failed. (dev_id=%u; vfid=%u; ret=%d)", dev_id, vfid, ret);
            return ret;
        }
    } else {
        chip_aicore_info->num = dev_info->ai_core_num;
    }

    dms_debug("(Freq level=%u; num_level=%u; num=%u; freq=%u; chip_name=0x%u; chip_ver=%u)\n",
        chip_aicore_info->freq_level, chip_aicore_info->num_level, chip_aicore_info->num, chip_aicore_info->freq,
        chip_aicore_info->chip_name, chip_aicore_info->chip_ver);
    return 0;
}

static int dms_set_chip_name_by_aicore_num_and_freq(devdrv_query_chip_info_t *chip_info,
    dms_chip_aicore_info_t *chip_aicore_info)
{
    int ret;
    unsigned char *chip_name = chip_info->info.name;

    if (AICORE_LEVEL_LIMIT(chip_aicore_info->num_level, chip_aicore_info->freq_level)) {
        ret = strcat_s((char *)chip_name, MAX_CHIP_NAME, g_aicore_freq_level[chip_aicore_info->freq_level]);
        if (ret < 0) {
            dms_err("Strcat chip name by aicore freq level failed. (ret=%d)\n", ret);
            return ret;
        }

        ret = strcat_s((char *)chip_name, MAX_CHIP_NAME, g_aicore_num_level[chip_aicore_info->num_level]);
        if (ret < 0) {
            dms_err("Strcat chip name by aicore num level failed. (ret=%d)\n", ret);
            return ret;
        }
    } else {
        if (chip_aicore_info->freq < AICORE_FREQ_LEVEL_PRO) {
            ret = strcat_s((char *)chip_name, MAX_CHIP_NAME, g_aicore_freq_level[FREQ_LITE_INDEX]);
        } else if (chip_aicore_info->freq > AICORE_FREQ_LEVEL_PRE) {
            ret = strcat_s((char *)chip_name, MAX_CHIP_NAME, g_aicore_freq_level[FREQ_PRE_INDEX]);
        } else {
            ret = strcat_s((char *)chip_name, MAX_CHIP_NAME, g_aicore_freq_level[FREQ_PRO_INDEX]);
        }

        if (ret < 0) {
            dms_err("Strcat chip name by aicore freq fail. (dev_id=%u; frequency=%u; ret=%d)\n",
                chip_info->dev_id, chip_aicore_info->freq, ret);
            return ret;
        }

        if (chip_aicore_info->num < CLOUD_FULL_GOOD_CORE_NUM) {
            ret = strcat_s((char *)chip_name, MAX_CHIP_NAME, g_aicore_num_level[CORE_NUM_B_INDEX]);
        } else {
            ret = strcat_s((char *)chip_name, MAX_CHIP_NAME, g_aicore_num_level[CORE_NUM_A_INDEX]);
        }

        if (ret < 0) {
            dms_err("Strcat chip name by aicore num fail. (dev_id=%u; num=%u; ret=%d)\n",
                chip_info->dev_id, chip_aicore_info->num, ret);
            return ret;
        }
    }

    return 0;
}

STATIC int dms_set_chip_name_only_by_aicore_num(devdrv_query_chip_info_t *chip_info,
    dms_chip_aicore_info_t *chip_aicore_info)
{
    int ret;
    unsigned int ai_core_num = chip_aicore_info->num;
    unsigned char *chip_name = chip_info->info.name;

    if (ai_core_num == MINIV2_PRO_CORE_NUM) {
#ifdef CFG_SOC_PLATFORM_MDC_V51
        /* "1, 2" is array index, not magic num */
        ret = strcat_s((char *)chip_name, MAX_CHIP_NAME, g_aicore_freq_level[2]);
#else
        ret = strcat_s((char *)chip_name, MAX_CHIP_NAME, g_ai_computing_level[AI_COMPUTING_LEVEL1]);
#endif
        if (ret < 0) {
            dms_err("Strcat by aicore freq failed. (dev_id=%u; aicore_num=%u; ret=%d)\n",
                chip_info->dev_id, ai_core_num, ret);
            return ret;
        }
    } else if (ai_core_num == MINIV2_CORE_NUM) {
#ifdef CFG_SOC_PLATFORM_MDC_V51
        ret = strcat_s((char *)chip_name, MAX_CHIP_NAME, g_aicore_freq_level[1]);
#else
        ret = strcat_s((char *)chip_name, MAX_CHIP_NAME, g_ai_computing_level[AI_COMPUTING_LEVEL3]);
#endif
        if (ret < 0) {
            dms_err("Strcat by aicore freq failed. (dev_id=%u; aicore_num=%u; ret=%d)\n",
                chip_info->dev_id, ai_core_num, ret);
            return ret;
        }
    } else {
#ifdef CFG_FEATURE_BIN_DOWN
        dms_event("aicore_num = %d, please check whether mode BinDown is enabled.\n", ai_core_num);
        ret = sprintf_s((char *)chip_name, MAX_CHIP_NAME, "%s", "");
        if (ret < 0) {
            dms_err("Strcat by aicore freq failed. (dev_id=%u; aicore_num=%u; ret=%d)\n",
                chip_info->dev_id, ai_core_num, ret);
            return ret;
        }
#else
        dms_err("Aicore number is invalid. (dev_id=%u; aicore_num=%u)\n", chip_info->dev_id, ai_core_num);
        return DRV_ERROR_PARA_ERROR;
#endif
    }

    return 0;
}

#if defined (CFG_FEATURE_PG) && (!defined (CFG_SOC_PLATFORM_MDC_V11))
static int dms_set_chip_name_by_soc_version(u32 dev_id, u8 *chip_name)
{
    int ret;
    struct devdrv_info *dev_info = NULL;

    dev_info = devdrv_manager_get_devdrv_info(dev_id);
    if (dev_info == NULL) {
        dms_err("dev_info is NULL. (dev_id=%u)\n", dev_id);
        return DRV_ERROR_INVALID_VALUE;
    }

    if (strlen(dev_info->pg_info.spePgInfo.socVersion) <= strlen("Ascend")) {
        dms_err("soc version is too short. (ver=\"%s\")\n", dev_info->pg_info.spePgInfo.socVersion);
        return DRV_ERROR_INVALID_VALUE;
    }
    /* the soc_version read from hsm is like Ascend910xx, Ascend is not wanted */
    ret = strcpy_s(chip_name, MAX_CHIP_NAME, (u8 *)dev_info->pg_info.spePgInfo.socVersion + strlen("Ascend"));
    if (ret) {
        dms_err("Call strcpy_s failed. (dev_id=%u)\n", dev_id);
        return DRV_ERROR_INVALID_VALUE;
    }

    return 0;
}
#endif

#ifdef CFG_CHIP_INFO_FROM_DEVINFO
STATIC int dms_set_chip_template_name_from_devinfo(u32 dev_id, u32 vfid, devdrv_query_chip_info_t *chip_info)
{
    int ret;
    struct devdrv_info *dev_info = NULL;

    if (devdrv_manager_is_pf_device(dev_id) && (vfid == 0)) {
        return 0;
    }

    dev_info = devdrv_manager_get_devdrv_info(dev_id);
    if (dev_info == NULL) {
        dms_err("dev_info is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    ret = strcat_s((char *)chip_info->info.name, MAX_CHIP_NAME, dev_info->template_name);
    if (ret != 0) {
        dms_err("Set template name failed. (dev_id=%u; vfid=%u; ret=%d)\n", dev_id, vfid, ret);
        return -EINVAL;
    }

    return 0;
}
#define dms_set_chip_template_name dms_set_chip_template_name_from_devinfo
#elif defined(CFG_CHIP_INFO_FROM_VMNG)
STATIC int dms_set_chip_template_name_from_vmng(u32 dev_id, u32 vfid, devdrv_query_chip_info_t *chip_info)
{
    int ret;
    struct vmng_soc_resource_enquire info;

    if (devdrv_manager_is_pf_device(dev_id) && (vfid == 0)) {
        return 0;
    }

#ifdef CFG_HOST_ENV
    ret = vmngh_enquire_soc_resource(dev_id, vfid, &info);
#else
    ret = vmngd_enquire_soc_resource(dev_id, vfid, &info);
#endif
    if (ret != 0) {
        dms_err("Enquire soc resource failed. (dev_id=%u; vfid=%u; ret=%d)\n", dev_id, vfid, ret);
        return ret;
    }

    ret = strcat_s((char *)chip_info->info.name, MAX_CHIP_NAME, info.each.name);
    if (ret != 0) {
        dms_err("Set template name failed. (dev_id=%u; vfid=%u; ret=%d)\n", dev_id, vfid, ret);
        return -EINVAL;
    }

    return 0;
}
#define dms_set_chip_template_name dms_set_chip_template_name_from_vmng
#else
STATIC int dms_set_chip_template_name(u32 dev_id, u32 vfid, devdrv_query_chip_info_t *chip_info)
{
    return 0;
}
#endif

STATIC int dms_set_chip_type(u32 dev_id, devdrv_query_chip_info_t *chip_info, unsigned int virt_id)
{
    int ret = 0;

#if defined (CFG_FEATURE_PG) && defined (CFG_SOC_PLATFORM_MDC_V11)
    struct devdrv_info *dev_info = NULL;
    dev_info = devdrv_manager_get_devdrv_info(dev_id);
    if (dev_info == NULL) {
        dms_err("dev_info is NULL. (dev_id=%u)\n", dev_id);
        return DRV_ERROR_INVALID_VALUE;
    }
    /* The AS31XM1X information is obtained from the sys_base_config.xml file in the ts_platform table. */
    ret = strcpy_s((char *)chip_info->info.type, MAX_CHIP_NAME, dev_info->pg_info.spePgInfo.socVersion);
    if (ret != 0) {
        dms_err("Copy as31xm1 chip type failed. (dev_id=%u; ret=%d)\n", chip_info->dev_id, ret);
        return ret;
    }
#else
    /* Initialize the chip_type to Ascend. */
    ret = strcpy_s((char *)chip_info->info.type, MAX_CHIP_NAME, "Ascend");
    if (ret != 0) {
        dms_err("Copy chip type failed. (virt_id=%u; dev_id=%u; ret=%d)\n", virt_id, chip_info->dev_id, ret);
        return ret;
    }
#endif
    return 0;
}

static int dms_set_chip_name(devdrv_query_chip_info_t *chip_info, bool is_container_split,
    dms_chip_aicore_info_t *chip_aicore_info, unsigned int virt_id)
{
    int ret = 0;
    unsigned int i;

    ret = sprintf_s((char *)chip_info->info.name, MAX_CHIP_NAME, "%s", "unknown");
    if (ret < 0) {
        dms_err("Set initial chip name failed. (virt_id=%u; dev_id=%u; ret=%d)\n", virt_id, chip_info->dev_id, ret);
        return ret;
    }

#if defined (CFG_FEATURE_PG) && (!defined (CFG_SOC_PLATFORM_MDC_V11))
    ret = dms_set_chip_name_by_soc_version(chip_info->dev_id, (u8 *)chip_info->info.name);
    if (ret) {
        dms_err("Set chip name by soc version failed. (dev_id=%u)\n", chip_info->dev_id);
        return ret;
    }
    return 0;
#elif defined (CFG_FEATURE_PG) && (defined (CFG_SOC_PLATFORM_MDC_V11))
    ret = strcpy_s((char *)chip_info->info.name, MAX_CHIP_NAME, g_aicore_freq_level[1]);
    if (ret != 0) {
        dms_err("Set AS31XM1 name err. (dev_id=%u  ret=%d)\n", chip_info->dev_id, ret);
        return ret;
    }
    return 0;
#endif

    for (i = 0; i < sizeof(g_chip_value_name_map) / sizeof(chip_value_name_map_t); ++i) {
        if (chip_aicore_info->chip_name != g_chip_value_name_map[i].chip_value) {
            continue;
        }

        if (is_container_split == true) {
#ifndef CFG_FEATURE_VFG
            ret = sprintf_s((char *)chip_info->info.name, MAX_CHIP_NAME, "%svir%02u",
                g_chip_value_name_map[i].chip_name, chip_aicore_info->num);
#else
            ret = sprintf_s((char *)chip_info->info.name, MAX_CHIP_NAME, "%s",
                g_chip_value_name_map[i].chip_name);
#endif
            if (ret < 0) {
                dms_err("Translate chip name failed. (dev_id=%u; ret=%d)\n", chip_info->dev_id, ret);
                return ret;
            }

            return 0;
        }

        ret = sprintf_s((char *)chip_info->info.name, MAX_CHIP_NAME, "%s", g_chip_value_name_map[i].chip_name);
        if (ret < 0) {
            dms_err("Translate chip name failed. (ret=%d)\n", ret);
            return ret;
        }

        if (chip_aicore_info->chip_name == g_chip_value_name_map[DEV_AICORE_FREQ_LEVEL_CLOUD].chip_value) {
            ret = dms_set_chip_name_by_aicore_num_and_freq(chip_info, chip_aicore_info);
            if (ret != 0) {
                dms_err("Set chip name by aicore number and freq fail. (dev_id=%u; ret=%d)\n", chip_info->dev_id, ret);
                return ret;
            }
        }

        if (chip_aicore_info->chip_name == g_chip_value_name_map[DEV_AICORE_FREQ_LEVEL_MINIV2].chip_value) {
            ret = dms_set_chip_name_only_by_aicore_num(chip_info, chip_aicore_info);
            if (ret != 0) {
                dms_err("Set chip name only by aicore number fail. (dev_id=%u; ret=%d)\n", chip_info->dev_id, ret);
                return ret;
            }
        }

        break;
    }

    return 0;
}

STATIC int dms_update_chip_info(devdrv_query_chip_info_t *chip_info, dms_chip_aicore_info_t *chip_aicore_info)
{
    int ret = DRV_ERROR_NONE;
    int tmp;

#if defined(CFG_SOC_PLATFORM_CLOUD) || defined(CFG_SOC_PLATFORM_MINIV2)
    tmp = sprintf_s((char *)chip_info->info.version, MAX_CHIP_NAME, "V%01x", chip_aicore_info->chip_ver);
#else
    /* mini chip info register, chip version is 0-11 bit, 3bytes, so need V%03x */
    tmp = sprintf_s((char *)chip_info->info.version, MAX_CHIP_NAME, "V%03x", chip_aicore_info->chip_ver);
#endif
    if (tmp < 0) {
        dms_err("get version err. (dev=%u chip_ver=%u ret=%d)\n", chip_info->dev_id, chip_aicore_info->chip_ver, tmp);
        return DRV_ERROR_INNER_ERR;
    }

#ifdef CFG_SOC_PLATFORM_MDC_V51
    switch (chip_aicore_info->chip_ver) {
        case CHIP_TYPE_ASCEND_V1:
            break;
        case CHIP_TYPE_ASCEND_V2:
            ret = strcpy_s((char *)chip_info->info.type, MAX_CHIP_NAME, "BS9SX1A");
            if (ret != 0) {
                dms_err("get type 'BS9SX1A' err. (dev_id=%u chip_ver=%u ret=%d)\n",
                    chip_info->dev_id, chip_aicore_info->chip_ver, ret);
                break;
            }

            ret = strcpy_s((char *)chip_info->info.name, MAX_CHIP_NAME, "A");
            if (ret != 0) {
                dms_err("get chip name 'A' err. (dev_id=%u chip_ver=%u ret=%d)\n",
                    chip_info->dev_id, chip_aicore_info->chip_ver, ret);
            }

            break;
        default:
            ret = DRV_ERROR_INVALID_VALUE;
            dms_err("chip_ver is not support! (dev_id=%u chip_ver=%u ret=%d)\n",
                chip_info->dev_id, chip_aicore_info->chip_ver, ret);
            return ret;
    }

    dms_info("dms_update_chip_info suc. (dev_id=%u chip_ver=%u)\n", chip_info->dev_id, chip_aicore_info->chip_ver);
#endif

    return ret;
}

static int dms_get_chip_info(unsigned int virt_id, devdrv_query_chip_info_t *chip_info)
{
    int ret;
    u32 vfid = 0;
    bool is_in_container;
    bool is_container_split = false;
    dms_chip_aicore_info_t chip_aicore_info = {0};

    ret = dms_trans_and_check_id(virt_id, &chip_info->dev_id, &vfid);
    if (ret != 0) {
        dms_err("Transfer device id failed. (virt_id=%u; ret=%d)", virt_id, ret);
        return ret;
    }

    is_in_container = devdrv_manager_container_is_in_container();
    if ((is_in_container == true) && (vfid > 0)) {
        is_container_split = true;
    }

    ret = dms_get_chip_aicore_info(chip_info->dev_id, vfid, is_container_split, &chip_aicore_info);
    if (ret != 0) {
        dms_err("Get chip version register info fail. (virt_id=%u; dev_id=%u; ret=%d)\n",
            virt_id, chip_info->dev_id, ret);
        return ret;
    }

    ret = dms_set_chip_type(chip_info->dev_id, chip_info, virt_id);
    if (ret != 0) {
        dms_err("Copy chip type failed. (virt_id=%u; dev_id=%u; ret=%d)\n", virt_id, chip_info->dev_id, ret);
        return ret;
    }

    ret = dms_set_chip_name(chip_info, is_container_split, &chip_aicore_info, virt_id);
    if (ret != 0) {
        dms_err("Set chip name failed. (virt_id=%u; dev_id=%u; ret=%d)\n", virt_id, chip_info->dev_id, ret);
        return ret;
    }

    ret = dms_set_chip_template_name(chip_info->dev_id, vfid, chip_info);
    if (ret != 0) {
        dms_err("Set chip template name failed. (virt_id=%u; dev_id=%u; ret=%d)\n", virt_id, chip_info->dev_id, ret);
        return ret;
    }

    ret = dms_update_chip_info(chip_info, &chip_aicore_info);
    if (ret != 0) {
        dms_err("update chip info failed. (virt_id=%u; dev_id=%u; ret=%d)\n", virt_id, chip_info->dev_id, ret);
        return ret;
    }

    return 0;
}

int dms_feature_get_chip_info(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    int ret;
    unsigned int virt_id;
    devdrv_query_chip_info_t chip_info = {0};

    if ((in == NULL) || (in_len != sizeof(unsigned int))) {
        dms_err("Input data is NULL or input data length is wrong. (in_len=%u; correct_in_len=%lu)\n",
            in_len, sizeof(unsigned int));
        return -EINVAL;
    }

    if ((out == NULL) || (out_len != sizeof(devdrv_query_chip_info_t))) {
        dms_err("Output data is NULL or output data length is wrong. (out_len=%u; correct_out_len=%lu)\n",
            out_len, sizeof(devdrv_query_chip_info_t));
        return -EINVAL;
    }

    virt_id = *(unsigned int *)in;
    ret = dms_get_chip_info(virt_id, &chip_info);
    if (ret != 0) {
        dms_err("Get chip info failed. (virt_id=%u; ret=%d)\n", virt_id, ret);
        return ret;
    }

    *(devdrv_query_chip_info_t *)out = chip_info;
    return 0;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
int dms_set_bist_mode(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    int ret;
    int chip_type = 0;
    BIST_MODE_STRU bist_mode;
    struct dms_set_bist_info_in *bist_info = NULL;
    BIST_VECTOR_INFO *bist_vector_info = dms_get_bist_vector_info_addr();

    if ((in == NULL) || (in_len != sizeof(struct dms_set_bist_info_in))) {
        dms_err("Input data is NULL or input data length is wrong. (in_len=%u; correct_in_len=%lu)\n",
            in_len, sizeof(struct dms_set_bist_info_in));
        return -EINVAL;
    }

    if (bist_vector_info->valid != BIST_VALID) {
        dms_err("Bist vector info is invalid, please check DTS file.\n");
        return -ENODATA;
    }

    bist_info = (struct dms_set_bist_info_in *)in;
    if (bist_info->buff_size < sizeof(BIST_MODE_STRU)) {
        dms_err("Input buff size is too short, buff_size=%u.\n", bist_info->buff_size);
        return -EINVAL;
    }

    ret = devdrv_manager_get_chip_type(&chip_type);
    if (ret != EOK) {
        dms_err("get chip_type failed, ret = %d.\n", ret);
        return ret;
    }

    if ((chip_type != CHIP_TYPE_ASCEND_V2) && (chip_type != CHIP_TYPE_ASCEND_V51_LITE)) {
        dms_err("chip %d can't support BIST mode cfg.\n", chip_type);
        return -EOPNOTSUPP;
    }

    ret = copy_from_user_safe((void *)&bist_mode, bist_info->buff, sizeof(BIST_MODE_STRU));
    if (ret != EOK) {
        dms_err("copy bist mode info from user failed, ret=%d.\n", ret);
        return ret;
    }

    /* seconds有效范围[1,255], bist_flag有效范围[0,1], start_vector_id有效范围[0,vector_cnt) */
    if ((bist_mode.seconds > MAX_BIST_MODE_SECONDES) || (bist_mode.seconds < MIN_BIST_MODE_SECONDES) ||
        (bist_mode.bist_flag >= MAX_BIST_FLAG) || (bist_mode.start_vector_id >= bist_vector_info->vector_cnt)) {
        dms_err("bist mode para is invalid. (seconds=%u; bist_flag=%u; start_vector_id=%u)\n",
            bist_mode.seconds,
            bist_mode.bist_flag,
            bist_mode.start_vector_id);
        return -EINVAL;
    }

    g_bist_mode.seconds = bist_mode.seconds;
    g_bist_mode.bist_flag = bist_mode.bist_flag;
    g_bist_mode.start_vector_id = bist_mode.start_vector_id;

    dms_info("Set bist mode success. (seconds=%u; bist_flag=%u; start_vector_id=%u)\n",
        g_bist_mode.seconds,
        g_bist_mode.bist_flag,
        g_bist_mode.start_vector_id);
    return EOK;
}

BIST_MODE_STRU *dms_get_bist_mode_addr(void)
{
    return (BIST_MODE_STRU *)&g_bist_mode;
}

void dms_clear_bist_mode(void)
{
    BIST_MODE_STRU *bist_mode = dms_get_bist_mode_addr();
    bist_mode->seconds = 0;
    bist_mode->bist_flag = 0;
    bist_mode->start_vector_id = 0;
    dms_info("Clear BIST mode para success.\n");
    return;
}

int dms_get_bist_mode(BIST_MODE_STRU *bist_mode)
{
    if (bist_mode == NULL) {
        dms_err("Input data is NULL\n");
        return -EINVAL;
    }

    bist_mode->seconds = g_bist_mode.seconds;
    bist_mode->bist_flag = g_bist_mode.bist_flag;
    bist_mode->start_vector_id = g_bist_mode.start_vector_id;

    return EOK;
}

void dms_bist_rslt_dump(BIST_RESULT_STRU *bist_result)
{
    dms_info("This is bist magic: 0x%x(expect 0x%x).\n", bist_result->magic, BIST_MAGIC);
    dms_info("Power on bist result: magic=0x%x(expect 0x%x), version=%u, len=%u, check_sum=%u, "
             "error_code=0x%x, vector_tested_cnt=%u, vector_sum=%u, vector_failed_id=%u, vector_failed_cnt=%u.\n",
             bist_result->pwr_on_bist.info_hdr.magic, POWERON_BIST_MAGIC, bist_result->pwr_on_bist.info_hdr.version,
             bist_result->pwr_on_bist.info_hdr.len, bist_result->pwr_on_bist.info_hdr.check_sum,
             bist_result->pwr_on_bist.bist_info.error_code, bist_result->pwr_on_bist.bist_info.vector_tested_cnt,
             bist_result->pwr_on_bist.bist_info.vector_sum, bist_result->pwr_on_bist.bist_info.vector_failed_id,
             bist_result->pwr_on_bist.bist_info.vector_failed_cnt);
    dms_info("Sils mem bist result: total_num=%hhu, image_num=%hhu, next_image_id=%hhu, next_vector_id=%hhu, "
             "tested_vector_num=%hhu, failed_vector_num=%hhu, first_failed_id=%hhu, dot_code=0x%x, error_code=0x%x.\n",
             bist_result->sil_mbist_rslt.total_num, bist_result->sil_mbist_rslt.image_num,
             bist_result->sil_mbist_rslt.next_image_id, bist_result->sil_mbist_rslt.next_vector_id,
             bist_result->sil_mbist_rslt.tested_vector_num, bist_result->sil_mbist_rslt.failed_vector_num,
             bist_result->sil_mbist_rslt.first_failed_id, bist_result->sil_mbist_rslt.dot_code,
             bist_result->sil_mbist_rslt.error_code);
    dms_info("Lp mem bist result: total_num=%hhu, next_vector_id=%hhu, next_seg_id=%hhu, "
             "tested_vector_num=%hhu, failed_vector_num=%hhu, first_failed_id=%hhu, error_code=0x%x.\n",
             bist_result->lp_mbist_rslt.total_num, bist_result->lp_mbist_rslt.next_vector_id,
             bist_result->lp_mbist_rslt.next_seg_id, bist_result->lp_mbist_rslt.tested_vector_num,
             bist_result->lp_mbist_rslt.failed_vector_num, bist_result->lp_mbist_rslt.first_failed_id,
             bist_result->lp_mbist_rslt.error_code);
    dms_info("Logic bist result: total_num=%hhu, next_vector_id=%hhu, next_seg_id=%hhu, tested_vector_num=%hhu, "
             "failed_vector_num=%hhu, first_failed_id=%hhu, error_code=0x%x.\n",
             bist_result->lbist_rslt.total_num, bist_result->lbist_rslt.next_vector_id,
             bist_result->lbist_rslt.next_seg_id, bist_result->lbist_rslt.tested_vector_num,
             bist_result->lbist_rslt.failed_vector_num, bist_result->lbist_rslt.first_failed_id,
             bist_result->lbist_rslt.error_code);
    dms_info("Bist unified info: failed_vector_id=%u, tested_vector_cnt=%u.\n",
             bist_result->unified_info.failed_vector_id, bist_result->unified_info.tested_vector_cnt);

    return;
}

int dms_get_bist_rslt_form_sram(void)
{
    void *poweron_bist_addr = NULL;
    void *bist_addr = NULL;

    poweron_bist_addr = ioremap(POWERON_BIST_RESULT_SRAM_ADDR, POWERON_BIST_RESULT_SRAM_SIZE);
    if (poweron_bist_addr == NULL) {
        dms_err("ioremap power on bist sram area failed.\n");
        return -EINVAL;
    }

    bist_addr = ioremap(BIST_RESULT_SRAM_ADDR, BIST_RESULT_SRAM_SIZE);
    if (bist_addr == NULL) {
        iounmap(poweron_bist_addr);
        dms_err("ioremap mem bist sram area failed.\n");
        return -EINVAL;
    }

    g_dms_bist_rslt.magic = BIST_MAGIC;

    memcpy_fromio((void *)&g_dms_bist_rslt.pwr_on_bist, poweron_bist_addr, sizeof(BIOS_BIST_INFO));
    memcpy_fromio((void *)&g_dms_bist_rslt.sil_mbist_rslt, bist_addr, sizeof(struct dms_sil_bist_rslt_info));
    memcpy_fromio((void *)&g_dms_bist_rslt.lp_mbist_rslt, bist_addr + sizeof(struct dms_sil_bist_rslt_info),
                  sizeof(struct dms_lp_bist_rslt_info));
    memcpy_fromio((void *)&g_dms_bist_rslt.lbist_rslt,
                  bist_addr + sizeof(struct dms_sil_bist_rslt_info) + sizeof(struct dms_lp_bist_rslt_info),
                  sizeof(struct dms_lp_bist_rslt_info));
    memcpy_fromio((void *)&g_dms_bist_rslt.unified_info,
                  bist_addr + sizeof(struct dms_sil_bist_rslt_info) + sizeof(struct dms_lp_bist_rslt_info)
                  + sizeof(struct dms_lp_bist_rslt_info), sizeof(struct dms_bist_unified_info));
    dms_bist_rslt_dump((BIST_RESULT_STRU *)&g_dms_bist_rslt);

    iounmap(poweron_bist_addr);
    iounmap(bist_addr);
    return EOK;
}

BIST_VECTOR_INFO *dms_get_bist_vector_info_addr(void)
{
    return (BIST_VECTOR_INFO *)&g_bist_vector_info;
}

BIST_RESULT_STRU *dms_get_bist_rslt_addr(void)
{
    return (BIST_RESULT_STRU *)&g_dms_bist_rslt;
}

u32 dms_sil_mbist_hw_err_parse(u32 value)
{
    return ((((u32)value) >> BIST_SIL_HW_ERR_SHIFT) & BIST_HW_ERR_CODE_MASK);
}

u32 dms_lp_bist_hw_err_parse(u32 value)
{
    return ((((u32)value) >> BIST_LP_HW_ERR_SHIFT) & BIST_HW_ERR_CODE_MASK);
}

void dms_clear_bist_rslt_magic(void)
{
    BIST_RESULT_STRU *rslt = NULL;

    rslt = dms_get_bist_rslt_addr();
    rslt->magic = BIST_MAGIC_CLEAR;

    dms_event("Clear BIST magic success.\n");
    return;
}

int dms_check_poweron_bist_rslt(BIOS_BIST_INFO *bist_rslt)
{
    u32 index;
    u32 check_sum = 0;
    u8 *check_array = (u8 *)&(bist_rslt->bist_info);

    if (bist_rslt->info_hdr.magic != POWERON_BIST_MAGIC) {
        dms_err("Power on BIST magic 0x%x is invalid, expect 0x%x.\n", bist_rslt->info_hdr.magic, POWERON_BIST_MAGIC);
        return -EINVAL;
    }

    for (index = 0; index < sizeof(PWRON_BIST_RLST); index++) {
        check_sum = check_sum + (u32)check_array[index];
    }

    if (check_sum != bist_rslt->info_hdr.check_sum) {
        dms_err("Power on BIST checksum is invalid. (check_sum=%u; bios_check_sum=%u)\n",
            check_sum, bist_rslt->info_hdr.check_sum);
        return -EINVAL;
    }

    return EOK;
}

u32 dms_parse_pwron_bist_rslt(BIST_RESULT_STRU *rslt, DMS_BIST_RESULT_STRU *out)
{
    u32 power_on_bist_errcode = 0;
    BIOS_BIST_INFO *power_on_bist = &(rslt->pwr_on_bist);

    if (dms_check_poweron_bist_rslt(power_on_bist) != EOK) {
        out->bist_info[POWER_ON_BIST].bist_result = DEV_ERROR_BIST_SW_ERR;
        return POWERON_BIST_SW_ERR;
    }

    power_on_bist_errcode = rslt->pwr_on_bist.bist_info.error_code;
    if (power_on_bist_errcode == 0) {
        out->bist_info[POWER_ON_BIST].bist_result = 0;
    } else if (power_on_bist_errcode == POWERON_BIST_HW_ERR) {
        out->bist_info[POWER_ON_BIST].bist_result = DEV_ERROR_BIST_HW_ERR;
    } else {
        out->bist_info[POWER_ON_BIST].bist_result = DEV_ERROR_BIST_SW_ERR;
    }

    out->bist_info[POWER_ON_BIST].error_code = rslt->pwr_on_bist.bist_info.error_code;
    out->bist_info[POWER_ON_BIST].vector_tested_cnt = rslt->pwr_on_bist.bist_info.vector_tested_cnt;
    out->bist_info[POWER_ON_BIST].vector_sum = rslt->pwr_on_bist.bist_info.vector_sum;
    out->bist_info[POWER_ON_BIST].vector_failed_id = rslt->pwr_on_bist.bist_info.vector_failed_id;
    out->bist_info[POWER_ON_BIST].vector_failed_cnt = rslt->pwr_on_bist.bist_info.vector_failed_cnt;

    return power_on_bist_errcode;
}

void dms_parse_mbist_bist_rslt(BIST_RESULT_STRU *rslt, DMS_BIST_RESULT_STRU *out)
{
    u32 sils_errcode;

    sils_errcode = rslt->lp_mbist_rslt.error_code;
    out->bist_info[MEM_BIST].error_code = (sils_errcode != 0) ? sils_errcode : rslt->sil_mbist_rslt.error_code;
    out->bist_info[MEM_BIST].vector_failed_cnt =
        (sils_errcode != 0) ? rslt->lp_mbist_rslt.failed_vector_num : rslt->sil_mbist_rslt.failed_vector_num;
    out->bist_info[MEM_BIST].vector_failed_id =
        (sils_errcode != 0) ? rslt->lp_mbist_rslt.first_failed_id : rslt->sil_mbist_rslt.first_failed_id;
    out->bist_info[MEM_BIST].vector_sum = rslt->lp_mbist_rslt.total_num + rslt->sil_mbist_rslt.total_num;
    out->bist_info[MEM_BIST].vector_tested_cnt =
        rslt->lp_mbist_rslt.tested_vector_num + rslt->sil_mbist_rslt.tested_vector_num;

    if (out->bist_info[MEM_BIST].error_code == 0) {
        out->bist_info[MEM_BIST].bist_result = 0;
    } else if (dms_sil_mbist_hw_err_parse(rslt->sil_mbist_rslt.error_code) == MBIST_HW_ERR_CODE ||
               dms_lp_bist_hw_err_parse(rslt->lp_mbist_rslt.error_code) == MBIST_HW_ERR_CODE) {
        out->bist_info[MEM_BIST].bist_result = DEV_ERROR_BIST_HW_ERR;
    } else {
        out->bist_info[MEM_BIST].bist_result = DEV_ERROR_BIST_SW_ERR;
    }
    return;
}

void dms_parse_lbist_bist_rslt(BIST_RESULT_STRU *rslt, DMS_BIST_RESULT_STRU *out)
{
    out->bist_info[LOGIC_BIST].error_code = rslt->lbist_rslt.error_code;
    out->bist_info[LOGIC_BIST].vector_failed_cnt = rslt->lbist_rslt.failed_vector_num;
    out->bist_info[LOGIC_BIST].vector_failed_id = rslt->lbist_rslt.first_failed_id;
    out->bist_info[LOGIC_BIST].vector_sum = rslt->lbist_rslt.total_num;
    out->bist_info[LOGIC_BIST].vector_tested_cnt = rslt->lbist_rslt.tested_vector_num;

    if (out->bist_info[LOGIC_BIST].error_code == 0) {
        out->bist_info[LOGIC_BIST].bist_result = 0;
    } else if (dms_lp_bist_hw_err_parse(out->bist_info[LOGIC_BIST].error_code) == LBIST_HW_ERR_CODE) {
        out->bist_info[LOGIC_BIST].bist_result = DEV_ERROR_BIST_HW_ERR;
    } else {
        out->bist_info[LOGIC_BIST].bist_result = DEV_ERROR_BIST_SW_ERR;
    }
    return;
}

void dms_parse_bist_unified_info(BIST_RESULT_STRU *rslt, DMS_BIST_RESULT_STRU *out)
{
    out->failed_vector_id = rslt->unified_info.failed_vector_id;
    out->tested_vector_cnt = rslt->unified_info.tested_vector_cnt;
    return;
}

int dms_parse_bist_result(DMS_BIST_RESULT_STRU *buff)
{
    u32 bist_result;
    u32 pwron_bist_errcode;
    BIST_RESULT_STRU *rslt = NULL;

    rslt = dms_get_bist_rslt_addr();
    if ((rslt->magic != BIST_MAGIC) && (dms_get_bist_rslt_form_sram() != EOK)) {
        return -EINVAL;
    }

    pwron_bist_errcode = dms_parse_pwron_bist_rslt(rslt, buff);
    dms_parse_mbist_bist_rslt(rslt, buff);
    dms_parse_lbist_bist_rslt(rslt, buff);
    dms_parse_bist_unified_info(rslt, buff);

    bist_result = pwron_bist_errcode | rslt->sil_mbist_rslt.error_code | rslt->lp_mbist_rslt.error_code |
                  rslt->lbist_rslt.error_code;

    if (pwron_bist_errcode == POWERON_BIST_HW_ERR ||
        (dms_sil_mbist_hw_err_parse(rslt->sil_mbist_rslt.error_code) == MBIST_HW_ERR_CODE) ||
        (dms_lp_bist_hw_err_parse(rslt->lp_mbist_rslt.error_code) == MBIST_HW_ERR_CODE) ||
        (dms_lp_bist_hw_err_parse(rslt->lbist_rslt.error_code) == LBIST_HW_ERR_CODE)) {
        buff->bist_result = DEV_ERROR_BIST_HW_ERR;
    } else if (bist_result != EOK) {
        buff->bist_result = DEV_ERROR_BIST_SW_ERR;
    } else {
        buff->bist_result = DRV_ERROR_NONE;
    }

    return EOK;
}

STATIC int dms_feature_para_check(char *in, u32 in_len, u32 correct_in_len,
    char *out, u32 out_len, u32 correct_out_len)
{
    if ((in == NULL) || (in_len != correct_in_len)) {
        dms_err("Input data is NULL or in_len is wrong. (in_len=%u; correct_in_len=%u)\n",
            in_len, correct_in_len);
        return -EINVAL;
    }

    if ((out == NULL) || (out_len != correct_out_len)) {
        dms_err("Output data is NULL or out_len is wrong. (out_len=%u; correct_out_len=%u)\n",
            out_len, correct_out_len);
        return -EINVAL;
    }

    return EOK;
}

int dms_get_bist_result(struct dms_get_bist_info_in* in, struct dms_get_bist_info_out* out)
{
    int ret;
    DMS_BIST_RESULT_STRU bist_rslt = { 0 };

    if ((in->size < sizeof(DMS_BIST_RESULT_STRU)) || in->buff == NULL) {
        dms_err("Buff length is not enough or buff is NULL. (buff_len=%u; correct_buff_len=%lu)\n",
                in->size, sizeof(DMS_BIST_RESULT_STRU));
        return -EINVAL;
    }

    ret = dms_parse_bist_result(&bist_rslt);
    if (ret != EOK) {
        dms_err("Parse bist result failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = copy_to_user(in->buff, (void*)&bist_rslt, sizeof(DMS_BIST_RESULT_STRU));
    if (ret != EOK) {
        dms_err("Copy bist result to user failed. (ret=%d)\n", ret);
        return ret;
    }

    out->size = sizeof(DMS_BIST_RESULT_STRU);
    return EOK;
}

STATIC int dms_get_bist_vector_cnt(struct dms_get_bist_info_in* in, struct dms_get_bist_info_out* out)
{
    int ret;
    u32 bist_vector_cnt;
    BIST_VECTOR_INFO *bist_vector_info = dms_get_bist_vector_info_addr();

    if ((in->size < sizeof(u32)) || in->buff == NULL) {
        dms_err("Buff length is not enough or buff is NULL. (buff_len=%u; correct_buff_len=%lu)\n",
                in->size, sizeof(u32));
        return -EINVAL;
    }

    if (bist_vector_info->valid != BIST_VALID) {
        dms_err("Bist vector info is invalid.\n");
        return -ENODATA;
    }
    bist_vector_cnt = bist_vector_info->vector_cnt;

    ret = copy_to_user(in->buff, (void*)&bist_vector_cnt, sizeof(u32));
    if (ret != EOK) {
        dms_err("Copy bist vector cnt to user failed. (ret=%d)\n", ret);
        return ret;
    }

    out->size = sizeof(u32);
    return EOK;
}

STATIC int dms_get_bist_vector_time(struct dms_get_bist_info_in* in, struct dms_get_bist_info_out* out)
{
    int ret;
    u32 bist_vector_cnt;
    u32 *bist_vector_time;
    BIST_VECTOR_INFO *bist_vector_info = dms_get_bist_vector_info_addr();

    if (bist_vector_info->valid != BIST_VALID) {
        dms_err("Bist vector info is invalid.\n");
        return -ENODATA;
    }
    bist_vector_cnt = bist_vector_info->vector_cnt;
    bist_vector_time = bist_vector_info->vector_time;

    if ((in->size < sizeof(u32) * bist_vector_cnt) || (in->buff == NULL)) {
        dms_err("Buff length is not enough or buff is NULL. (buff_len=%u; correct_buff_len=%lu)\n",
            in->size, sizeof(u32) * bist_vector_cnt);
        return -EINVAL;
    }

    ret = copy_to_user(in->buff, (void*)bist_vector_time, sizeof(u32) * bist_vector_cnt);
    if (ret != EOK) {
        dms_err("Copy bist vector time to user failed. (ret=%d)\n", ret);
        return ret;
    }

    out->size = sizeof(u32) * bist_vector_cnt;
    return EOK;
}

int dms_get_bist_info(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    int ret;
    u32 sub_cmd;
    int chip_type = 0;
    struct dms_get_bist_info_in *bist_info_in;
    struct dms_get_bist_info_out *bist_info_out;

    (void)feature;
    ret = dms_feature_para_check(in, in_len, sizeof(struct dms_get_bist_info_in),
                                out, out_len, sizeof(struct dms_get_bist_info_out));
    if (ret != EOK) {
        dms_err("Dms feature para check failed.\n");
        return ret;
    }

    ret = devdrv_manager_get_chip_type(&chip_type);
    if (ret != EOK) {
        dms_err("Get chip_type failed, ret = %d.\n", ret);
        return ret;
    }

    if ((chip_type != CHIP_TYPE_ASCEND_V2) && (chip_type != CHIP_TYPE_ASCEND_V51_LITE)) {
        dms_err("Chip %d can't support BIST.\n", chip_type);
        return -EOPNOTSUPP;
    }

    bist_info_in = (struct dms_get_bist_info_in *)in;
    bist_info_out = (struct dms_get_bist_info_out *)out;
    sub_cmd = bist_info_in->sub_cmd;

    switch (sub_cmd) {
        case DMS_BIST_CMD_GET_RSLT:
            ret = dms_get_bist_result(bist_info_in, bist_info_out);
            break;
        case DMS_BIST_CMD_GET_VEC_CNT:
            ret = dms_get_bist_vector_cnt(bist_info_in, bist_info_out);
            break;
        case DMS_BIST_CMD_GET_VEC_TIME:
            ret = dms_get_bist_vector_time(bist_info_in, bist_info_out);
            break;
        default:
            dms_err("Bist info sub_cmd is not support. (sub_cmd=%u)\n", sub_cmd);
            return -EOPNOTSUPP;
    }
    if (ret != EOK) {
        dms_err("Dms get bist info failed. (ret=%d, sub_cmd=%u)\n", ret, sub_cmd);
        return ret;
    }

    return EOK;
}

STATIC int bist_vector_info_para_check(BIST_VECTOR_INFO *info)
{
    u32 index;

    if (info->lp_bist_vec_offset >= info->vector_cnt) {
        dms_err("Lp_bist_vec_offset[%u] is larger than vector_cnt[%u], please check DTS file.\n",
            info->lp_bist_vec_offset,
            info->vector_cnt);
        return -EINVAL;
    }

    for (index = 0; index < info->vector_cnt; index++) {
        if (info->vector_time[index] < MIN_BIST_VEC_TIME || info->vector_time[index] > MAX_BIST_VEC_TIME) {
            dms_err("Vector_time is out of range, please check DTS file. (vector_time[%u] = %u)\n",
                index, info->vector_time[index]);
            return -EINVAL;
        }
    }

    return EOK;
}

STATIC void dms_print_bist_vector_info(void)
{
    BIST_VECTOR_INFO *bist_vector_info = dms_get_bist_vector_info_addr();
    dms_info("Get bist vector info from DTS success. (valid=%u, vector_cnt=%u, lp_bist_vec_offset=%u)\n",
        bist_vector_info->valid,
        bist_vector_info->vector_cnt,
        bist_vector_info->lp_bist_vec_offset);
    return;
}

STATIC int dms_get_bist_vector_info_from_dts(void)
{
    int ret;
    struct device_node *np = NULL;
    BIST_VECTOR_INFO temp_info = {0};
    BIST_VECTOR_INFO *bist_vector_info = dms_get_bist_vector_info_addr();

    np = of_find_compatible_node(NULL, NULL, "hisi,bist");
    if (np == NULL) {
        dms_warn("Can't find valid hisi,bist node.\n");
        return -ENODEV;
    }

    ret = of_property_read_u32(np, "lp_bist_vec_offset", &temp_info.lp_bist_vec_offset);
    if (ret != 0) {
        dms_err("Function of_property_read_u32 failed. (ret=%d))\n", ret);
        return ret;
    }

    /* 读取32位数组，成功返回数组元素个数，元素个数限定在[0,MAX_BIST_VEC_CNT](超出范围返回-EOVERFLOW) */
    ret = of_property_read_variable_u32_array(np, "bist_vec_time", temp_info.vector_time, 0, MAX_BIST_VEC_CNT);
    if (ret < 0) {
        dms_err("Function of_property_read_variable_u32_array failed. (ret=%d)\n", ret);
        return ret;
    }
    temp_info.vector_cnt = ret;

    ret = bist_vector_info_para_check(&temp_info);
    if (ret != 0) {
        dms_err("Bist_vector_info_para_check failed.\n");
        return ret;
    }

    /* 操作全局变量前将标记置INVALID,操作完成后置VALID */
    bist_vector_info->valid = BIST_INVALID;
    ret = memcpy_s((void *)bist_vector_info, sizeof(BIST_VECTOR_INFO), (void *)&temp_info, sizeof(BIST_VECTOR_INFO));
    if (ret != 0) {
        dms_err("Memcpy_s failed. (ret=%d)\n", ret);
        return ret;
    }
    bist_vector_info->valid = BIST_VALID;

    dms_print_bist_vector_info();

    return EOK;
}

int dms_bist_init(void)
{
    int ret;

    ret = dms_get_bist_vector_info_from_dts();
    if (ret == EOK) {
        dms_info("Dms init bist info success.\n");
    } else if (ret == -ENODEV) {
        dms_warn("Bist is not supported on this board type.\n");
    } else {
        dms_err("Dms get bist vector info from DTS failed. (ret=%d)\n", ret);
    }

    return EOK;
}

void dms_bist_uninit(void)
{
    return;
}

#endif
#ifdef CFG_FEATURE_REBOOT_REASON
STATIC void dms_parse_reboot_reason(struct dms_reboot_reason *reboot_reason)
{
    void __iomem *vaddr = NULL;
    unsigned int i, type;
#define DMS_RESET_REG_NUM  3
    unsigned int reason_val[DMS_RESET_REG_NUM] = {0};
    unsigned int reason_sram_addr[DMS_RESET_REG_NUM] = {
        DMS_RESET_REASON_L3SRAM_ADDR_BASE + RST_REASON_HW_L3SRAM_OFFSET,
        DMS_RESET_REASON_L3SRAM_ADDR_BASE + RST_REASON_HIS_L3SRAM_OFFSET1,
        DMS_RESET_REASON_L3SRAM_ADDR_BASE + RST_REASON_HIS_L3SRAM_OFFSET2 };

    struct dms_reason_priority reason_priority[] = {
#ifdef CFG_SOC_PLATFORM_MDC_V51
        { HW_RST_REASON,       RST_REASON_COLD,           RST_REASON_COLD_RST_BIT,     RST_REASON_COLD_RST_BIT },
        { SW_HIS_RST_REASON_1, RST_REASON_DSMI_CMD,       RST_REASON_BIT_DSMI_CMD,     RST_REASON_BIT_DSMI_CMD },
        { SW_HIS_RST_REASON_1, RST_REASON_REBOOT_CMD,     RST_REASON_BIT_REBOOT_CMD,   RST_REASON_BIT_REBOOT_CMD },
        { SW_HIS_RST_REASON_1, RST_REASON_PANIC,          RST_REASON_BIT_PANIC,        RST_REASON_BIT_PANIC },
        { SW_HIS_RST_REASON_2, RST_REASON_UTIMATE_WDG,    RST_REASON_BIT_UTIMATE_WDG,  RST_REASON_BIT_UTIMATE_WDG },
        { SW_HIS_RST_REASON_1, RST_REASON_BIOS_EXCEPTION, RST_REASON_BIT_BIOS,         RST_REASON_BIT_BIOS },
        { SW_HIS_RST_REASON_2, RST_REASON_SUSPEND_FAIL,   RST_REASON_BIT_SUSPEND_FAIL, RST_REASON_BIT_SUSPEND_FAIL },
        { SW_HIS_RST_REASON_2, RST_REASON_RESUME_FAIL,    RST_REASON_BIT_RESUME_FAIL,  RST_REASON_BIT_RESUME_FAIL },
        { HW_RST_REASON,       RST_REASON_WDG,            RST_DDR_R_WDG_BIT,           RST_DDR_R_WDG_BIT },
        { HW_RST_REASON,       RST_REASON_WDG,            RST_DDR_L_WDG_BIT,           RST_DDR_L_WDG_BIT },
        { HW_RST_REASON,       RST_REASON_WDG,            RST_LP_WDG_BIT,              RST_LP_WDG_BIT },
        { HW_RST_REASON,       RST_REASON_WDG,            RST_SILS_WDG_BIT,            RST_SILS_WDG_BIT },
        { HW_RST_REASON,       RST_REASON_EXTERNAL_PIN,   RST_REASON_PERST_N_BIT,      RST_REASON_PERST_N_BIT },
        { HW_RST_REASON,       RST_REASON_EXTERNAL_PIN,   RST_REASON_RSTIN_N_BIT,      RST_REASON_RSTIN_N_BIT },
        { HW_RST_REASON,       RST_REASON_PCIE_RST,       RST_REASON_PCIE_BIT,         RST_REASON_PCIE_BIT },
        { HW_RST_REASON,       RST_REASON_SOFT_RST,       RST_REASON_SOFT_RST_BIT,     RST_REASON_SOFT_RST_BIT}
#else
        { HW_RST_REASON,       RST_REASON_COLD,           HW_RST_REASON_MASK_V11,      HW_RST_REASON_COLD },
        { SW_HIS_RST_REASON_1, RST_REASON_DSMI_CMD,       RST_REASON_BIT_DSMI_CMD,     RST_REASON_BIT_DSMI_CMD },
        { SW_HIS_RST_REASON_1, RST_REASON_REBOOT_CMD,     RST_REASON_BIT_REBOOT_CMD,   RST_REASON_BIT_REBOOT_CMD },
        { SW_HIS_RST_REASON_1, RST_REASON_PANIC,          RST_REASON_BIT_PANIC,        RST_REASON_BIT_PANIC },
        { SW_HIS_RST_REASON_2, RST_REASON_UTIMATE_WDG,    RST_REASON_BIT_UTIMATE_WDG,  RST_REASON_BIT_UTIMATE_WDG },
        { SW_HIS_RST_REASON_1, RST_REASON_BIOS_EXCEPTION, RST_REASON_BIT_BIOS,         RST_REASON_BIT_BIOS },
        { SW_HIS_RST_REASON_2, RST_REASON_SUSPEND_FAIL,   RST_REASON_BIT_SUSPEND_FAIL, RST_REASON_BIT_SUSPEND_FAIL },
        { SW_HIS_RST_REASON_2, RST_REASON_RESUME_FAIL,    RST_REASON_BIT_RESUME_FAIL,  RST_REASON_BIT_RESUME_FAIL },
        { HW_RST_REASON,       RST_REASON_WDG,            HW_RST_REASON_MASK_V11,      HW_RST_REASON_LP_WDOG },
        { HW_RST_REASON,       RST_REASON_WDG,            HW_RST_REASON_MASK_V11,      HW_RST_REASON_DDR_WDOG },
        { HW_RST_REASON,       RST_REASON_EXTERNAL_PIN,   HW_RST_REASON_MASK_V11,      HW_RST_REASON_RSTN_IN },
        { HW_RST_REASON,       RST_REASON_EXTERNAL_PIN,   HW_RST_REASON_MASK_V11,      HW_RST_REASON_PCIE_PAD },
        { HW_RST_REASON,       RST_REASON_PCIE_RST,       HW_RST_REASON_MASK_V11,      HW_RST_REASON_PCIE_HOT },
        { HW_RST_REASON,       RST_REASON_SOFT_RST,       HW_RST_REASON_MASK_V11,      HW_RST_REASON_SOFT }
#endif
};

    reboot_reason->reason = RST_REASON_UNKNOWN;
    reboot_reason->sub_reason = 0;
    for (i = 0; i < DMS_RESET_REG_NUM; i++) {
        vaddr = ioremap(reason_sram_addr[i], DMS_RESET_REASON_SIZE);
        if (vaddr == NULL) {
            devdrv_drv_err("ioremap reason_sram_addr[%u] fail.\n", i);
            return;
        }
        reason_val[i] = readl_relaxed(vaddr);
        iounmap(vaddr);
        vaddr = NULL;
        devdrv_drv_info("reason_val[%u] = 0x%x.\n", i, reason_val[i]);
    }

    for (i = 0; i < sizeof(reason_priority) / sizeof(struct dms_reason_priority); i++) {
        type = reason_priority[i].type;
        if ((reason_val[type] & reason_priority[i].mask) == reason_priority[i].val) {
            reboot_reason->reason = reason_priority[i].reason;
            return;
        }
    }
    return;
}

int dms_get_reboot_reason(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    if ((in == NULL) || (in_len != sizeof(unsigned int))) {
        dms_err("Input data is NULL or input data length is wrong. (in_len=%u; correct_in_len=%lu)\n",
            in_len, sizeof(unsigned int));
        return -EINVAL;
    }

    if ((out == NULL) || (out_len != sizeof(struct dms_reboot_reason))) {
        dms_err("Output data is NULL or output data length is wrong. (out_len=%u; correct_out_len=%lu)\n",
            out_len, sizeof(struct dms_reboot_reason));
        return -EINVAL;
    }

    dms_parse_reboot_reason((struct dms_reboot_reason*)out);
    return 0;
}

#endif
#endif
