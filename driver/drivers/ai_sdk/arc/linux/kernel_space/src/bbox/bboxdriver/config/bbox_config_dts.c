/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-08-13
 */

#include "bbox_config_dts.h"

#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/securec.h>
#include "bbox_config.h"
#include "bbox_product.h"
#include "bbox_platform.h"
#include "common/bbox_common.h"


STATIC struct bbox_config_data g_bbox_config_dts;

// check of_address_to_resource return variables
STATIC bool bbox_config_check_addr_valid(struct resource *res)
{
    u64 size;
    if (res == NULL) {
        return false;
    }
    if ((res->start == 0) || (res->start >= BBOX_UINT_INVALID)) {
        BB_PRINT_ERR("invalid resource start\n");
        return false;
    }
    size = resource_size(res);
    if ((size == 0) || (size >= BBOX_UINT_INVALID) || ((res->start + size) > BBOX_UINT_INVALID)) {
        BB_PRINT_ERR("invalid resource size\n");
        return false;
    }
    return true;
}

/*
 * @brief       : get basic rdr configuration data,
 *                including rdr reserved memory, reserved area, dump log timeout and so on.
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_config_init_rdr_basic(void)
{
    s32 ret, i;
    u32 value = 0;
    struct device_node *np = NULL;
    struct resource res = {0};

    np = of_find_compatible_node(NULL, NULL, "hisilicon,rdr");
    BB_CHECK_PTR(np, return BBOX_FAILURE, "[%s][%4d], find rdr memory node fail!\n", __func__, __LINE__);

    ret = of_address_to_resource(np, 0, &res);
    if ((ret != 0) || (!bbox_config_check_addr_valid(&res))) {
        BB_PRINT_ERR("failed to translate bbox addr, resource size : 0x%llx, with ret: %d\n",
                     resource_size(&res), ret);
        return BBOX_FAILURE;
    }

    g_bbox_config_dts.bbox_data.addr = res.start;
    g_bbox_config_dts.bbox_data.length = resource_size(&res);

    ret = of_property_read_u32(np, "wait-dumplog-timeout", &value);
    if ((ret != 0) || (value > DUMP_LOG_MAX_TIME)) {
        BB_PRINT_ERR("failed to get wait-dumplog-timeout: %u!\n", value);
        return BBOX_FAILURE;
    }
    g_bbox_config_dts.wait_timeout = value;

    ret = of_property_read_u32(np, "rdr_area_num", &value);
    if ((ret != 0) || (value > BBOX_AREA_MAXIMUM)) {
        BB_PRINT_ERR("find rdr_area_num[%u] in dts error, ret[%d]!\n", value, ret);
        return BBOX_FAILURE;
    }
    g_bbox_config_dts.reserved_area.area_num = value;

    for (i = 0; i < (s32)value; i++) {
        ret = of_property_read_string_index(np, "rdr_area_names", i, &g_bbox_config_dts.reserved_area.area_name[i]);
        if (ret == 0) {
            ret = of_property_read_u32_index(np, "rdr_area_sizes", (u32)i,
                &g_bbox_config_dts.reserved_area.area_size[i]);
        }
        if (ret != 0) {
            BB_PRINT_ERR("cannot find rdr_area in dts!\n");
            return BBOX_FAILURE;
        }
    }
    return BBOX_SUCCESS;
}

/*
 * @brief       : get basic lpm3 configuration.
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_config_init_ddr_mntn(void)
{
    s32 ret;
    struct device_node *np = NULL;
    struct resource res = {0};

    np = of_find_compatible_node(NULL, NULL, "hisilicon,rdr-lpm3-sram");
    if (np == NULL) {
        BB_PRINT_ERR("[%s], find lpm sram node fail!\n", __func__);
        return BBOX_FAILURE;
    }

    ret = of_address_to_resource(np, 0, &res);
    if ((ret != 0) || (!bbox_config_check_addr_valid(&res)) || (resource_size(&res) > LPM_SRAM_MAX_SIZE)) {
        BB_PRINT_ERR("failed to get lpm sram addr, resource size : 0x%llx, ret : %d\n",
                     resource_size(&res), ret);
        return BBOX_FAILURE;
    }

    g_bbox_config_dts.ddr_mntn.addr = res.start;
    g_bbox_config_dts.ddr_mntn.length = resource_size(&res);

    return BBOX_SUCCESS;
}

static inline bool bbox_config_check_of_addr(struct resource *res)
{
    return ((res->start == 0) || (resource_size(res) == 0)) ? false : true;
}

/*
 * @brief       : get regs configuration data.
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_config_init_regs(void)
{
    s32 ret, i;
    u32 num = 0;
    struct device_node *np = NULL;
    struct resource res = {0};

    np = of_find_compatible_node(NULL, NULL, "hisilicon,rdr_ap_adapter");
    if (np == NULL) {
        BB_PRINT_ERR("cannot find rdr_ap_adapter node in dts!\n");
        return BBOX_FAILURE;
    }

    ret = of_property_read_u32(np, "reg-dump-regions", &num);
    if ((ret != 0) || (num == 0) || (num > REGS_DUMP_MAX_NUM)) {
        BB_PRINT_ERR("failed to get reg-dump-regions in dts: %u!\n", num);
        return BBOX_FAILURE;
    }
    g_bbox_config_dts.ap_regs_info.reg_num = num;
    for (i = 0; i < (s32)num; i++) {
        ret = of_address_to_resource(np, i, &res);
        if ((ret != 0) || (bbox_config_check_of_addr(&res) == false) || (strlen(res.name) > REG_NAME_LEN)) {
            BB_PRINT_ERR("failed to get %s, resource size : 0x%llx!\n", res.name, resource_size(&res));
            return BBOX_FAILURE;
        }
        g_bbox_config_dts.ap_regs_info.reg_name[i] = res.name;
        g_bbox_config_dts.ap_regs_info.addr[i] = res.start;
        g_bbox_config_dts.ap_regs_info.size[i] = resource_size(&res);
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : get spmi controller channel configuration data.
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_config_init_spmi_ctrl(void)
{
    s32 ret;
    const struct device_node *np = NULL;

    if (bbox_check_feature(FEATURE_SPMI) == false) {
        return BBOX_SUCCESS;
    }

    np = of_find_compatible_node(NULL, NULL, "hisilicon,spmi-controller");
    if (np == NULL) {
        BB_PRINT_ERR("not found device node 'hisilicon,spmi-controller'!\n");
        return BBOX_FAILURE;
    }

    ret = of_property_read_u32(np, "spmi-channel", &g_bbox_config_dts.spmi_channel);
    if (ret != 0) {
        BB_PRINT_ERR("can not get spmi-channel.\n");
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}

/*
 * @brief       : get pmu base add configuration data.
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_config_init_reboot_reason(void)
{
    s32 ret;
    struct device_node *np = NULL;
    struct resource res = {0};

    np = of_find_compatible_node(NULL, NULL, "hisilicon,spmi");
    if (np == NULL) {
        BB_PRINT_ERR("Not found device node 'hisilicon,spmi'!\n");
        return BBOX_FAILURE;
    }

    ret = of_address_to_resource(np, 0, &res);
    if ((ret != 0) || (!bbox_config_check_addr_valid(&res))) {
        BB_PRINT_ERR("Fail to translate pmu_addr, resource size :0x%llx, with ret: %d.\n",
                     resource_size(&res), ret);
        return BBOX_FAILURE;
    }

    g_bbox_config_dts.pmu_reset_reg.addr = res.start;
    g_bbox_config_dts.pmu_reset_reg.length = resource_size(&res);

    return BBOX_SUCCESS;
}

/*
 * @brief       : get hdr dtb config
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_config_init_hdr(void)
{
    s32 ret;
    struct device_node *np = NULL;
    struct resource res = {0};

    np = of_find_compatible_node(NULL, NULL, "hisilicon,hdr");
    if (np == NULL) {
        return BBOX_ENODEV;
    }

    ret = of_address_to_resource(np, 0, &res);
    if ((ret != BBOX_SUCCESS) || (!bbox_config_check_addr_valid(&res))) {
        BB_PRINT_ERR("get hdr addr failed with ret : %d\n", ret);
        return BBOX_FAILURE;
    }

    g_bbox_config_dts.hdr.addr = res.start;
    g_bbox_config_dts.hdr.length = resource_size(&res);
    if ((g_bbox_config_dts.hdr.addr > BBOX_RESERVED_ADDR_MAX) ||
        (g_bbox_config_dts.hdr.length > BBOX_RESERVED_SIZE_MAX)) {
        BB_PRINT_ERR("hdr addr out of range.\n");
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

STATIC bool bbox_check_feature_cdr(void)
{
    return bbox_check_feature(FEATURE_BOOTCHECK_CDR);
}

/*
 * @brief       : get cdr dtb config
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_config_init_cdr(void)
{
    s32 ret;
    struct device_node *np = NULL;
    struct resource res = {0};

    if (bbox_check_feature_cdr() == false) {
        return BBOX_SUCCESS;
    }

    np = of_find_compatible_node(NULL, NULL, "hisilicon,chip_dfx");
    if (np == NULL) {
        BB_PRINT_ERR("No find chip dfx node!\n");
        return BBOX_FAILURE;
    }

    ret = of_address_to_resource(np, 0, &res);
    if ((ret != BBOX_SUCCESS) || (!bbox_config_check_addr_valid(&res))) {
        BB_PRINT_ERR("get chip dfx addr failed with ret : %d\n", ret);
        return BBOX_FAILURE;
    }

    g_bbox_config_dts.cdr.addr = res.start;
    g_bbox_config_dts.cdr.length = resource_size(&res);
    if ((g_bbox_config_dts.cdr.addr == 0) ||
        (g_bbox_config_dts.cdr.length == 0) ||
        (g_bbox_config_dts.cdr.length > CDR_MAX_SIZE)) {
        BB_PRINT_ERR("chip dfx addr out of range.\n");
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : get boot param dtb config
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_config_init_boot_param(void)
{
    s32 ret;
    struct device_node *np = NULL;
    struct resource res = {0};

    np = of_find_compatible_node(NULL, NULL, "hisilicon,bbox_boot_param");
    if (np == NULL) {
        return BBOX_ENODEV;
    }

    ret = of_address_to_resource(np, 0, &res);
    if ((ret != BBOX_SUCCESS) || (!bbox_config_check_addr_valid(&res))) {
        BB_PRINT_ERR("get boot param addr failed with ret : %d\n", ret);
        return BBOX_FAILURE;
    }

    g_bbox_config_dts.boot_param.addr = res.start;
    g_bbox_config_dts.boot_param.length = resource_size(&res);
    return BBOX_SUCCESS;
}

#if (defined BBOX_SOC_PLATFORM_MDC  && !defined BBOX_UT)
STATIC struct bbox_config_bootstatus g_bbox_config_dts_bootstatus[BBOX_BOOT_STATUS_BLOCK_MAX_NUM - 1];

/*
 * @brief       : get secure boot dtb config
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_config_init_bootstatus_bynp(struct device_node *np)
{
    s32 ret;
    u32 i, num, value, stage;
    static u32 indx;

    ret = of_property_read_u32(np, "stage", &stage);
    if (ret != 0 || stage != 0) {
        BB_PRINT_ERR("Boot status stage is invalid.(stage=%u, ret=%d)\n", stage, ret);
        return BBOX_EINVAL;
    }
    g_bbox_config_dts_bootstatus[indx].stage_id = stage;

    ret = of_property_read_u32(np, "block_id", &value);
    if ((ret != 0) || (value > BBOX_BLOCK_MAX_NUM)) {
        BB_PRINT_ERR("Boot status block_id is invalid.(block_id=%u, ret=%d)\n", value, ret);
        return BBOX_EINVAL;
    }
    g_bbox_config_dts_bootstatus[indx].block_id = value;

    ret = of_property_read_u32(np, "file_num", &num);
    if ((ret != 0) || (num == 0) || (num > BBOX_BOOT_STATUS_FILE_MAX_NUM)) {
        BB_PRINT_ERR("File_num is invalid.(file_num=%u, ret=%d)\n", num, ret);
        return BBOX_EINVAL;
    }
    g_bbox_config_dts_bootstatus[indx].file_num = num;
    for (i = 0; i < num; i++) {
        char name[32] = {0};
        const char *file_name = name;
        ret = of_property_read_string_index(np, "file_names", i, &file_name);
        if (ret != 0) {
            BB_PRINT_ERR("Cannot find file_names in dts.(ret=%d)\n", ret);
            return BBOX_FAILURE;
        }
        BB_PRINT_INFO("Boot status file name info.(block_id=%u, file_name=%s, ret=%d)\n", value, file_name, ret);
        ret = of_property_read_u32_index(np, "file_id", i,
            &g_bbox_config_dts_bootstatus[indx].file_list[i].file_id);
        if (ret != 0) {
            BB_PRINT_ERR("Cannot find file_id in dts.(ret=%d)\n", ret);
            return BBOX_FAILURE;
        }
        ret = of_property_read_u32_index(np, "file_value", BBOX_BOOT_STATUS_VALUE_PAIR * i,
            &g_bbox_config_dts_bootstatus[indx].file_list[i].min_valid_value);
        if (ret != 0) {
            BB_PRINT_ERR("Cannot find file_value min value in dts.(ret=%d)\n", ret);
            return BBOX_FAILURE;
        }
        ret = of_property_read_u32_index(np, "file_value", (BBOX_BOOT_STATUS_VALUE_PAIR * i + 1),
            &g_bbox_config_dts_bootstatus[indx].file_list[i].max_valid_value);
        if (ret != 0) {
            BB_PRINT_ERR("Cannot find file_value max value in dts.(ret=%d)\n", ret);
            return BBOX_FAILURE;
        }
    }
    indx++;
    return BBOX_SUCCESS;
}

/*
 * @brief       : get secure boot dtb config
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_config_init_bootstatus_bycompatible(const char *compatible)
{
    struct device_node *np = NULL;

    np = of_find_compatible_node(NULL, NULL, compatible);
    if (np == NULL) {
        BB_PRINT_ERR("Cannot find secure_boot node in dts.\n");
        return BBOX_ENODEV;
    }
    return bbox_config_init_bootstatus_bynp(np);
}

/*
 * @brief       : get secure boot dtb config
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_config_init_bootstatus(void)
{
    s32 ret;

    ret = bbox_config_init_bootstatus_bycompatible("hisilicon,secure_boot1");
    if (ret != BBOX_SUCCESS) {
        return ret;
    }
    return bbox_config_init_bootstatus_bycompatible("hisilicon,secure_boot2");
}

STATIC struct bbox_config_rootfscms g_bbox_config_dts_rootfscms;

/*
 * @brief       : get rootfs cms dtb config
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_config_init_rootfscms(void)
{
    s32 ret;
    u32 num, value, stage, result;
    struct device_node *np = NULL;

    np = of_find_compatible_node(NULL, NULL, "hisilicon,rootfs_cms");
    if (np == NULL) {
        BB_PRINT_ERR("Cannot find rootfs_cms node in dts!\n");
        return BBOX_ENODEV;
    }

    ret = of_property_read_u32(np, "stage", &stage);
    if (ret != 0 || stage != 1) {
        BB_PRINT_ERR("Stage is invalid. (stage=%u, ret=%d)\n", stage, ret);
        return BBOX_EINVAL;
    }
    g_bbox_config_dts_rootfscms.stage_id = stage;

    ret = of_property_read_u32(np, "block_id", &value);
    if ((ret != 0) || (value > BBOX_BLOCK_MAX_NUM)) {
        BB_PRINT_ERR("Block_id is invalid.(block_id=%u, ret=%d)\n", value, ret);
        return BBOX_EINVAL;
    }
    g_bbox_config_dts_rootfscms.block_id = value;

    ret = of_property_read_u32(np, "result", &result);
    if (ret != 0 || result != 1) {
        BB_PRINT_ERR("result_id is invalid. (result=%u, ret=%d)\n", result, ret);
        return BBOX_EINVAL;
    }
    g_bbox_config_dts_rootfscms.result = result;

    ret = of_property_read_u32(np, "value_num", &num);
    if (ret != 0 || num != BBOX_BOOT_STATUS_VALUE_PAIR) {
        BB_PRINT_ERR("Value_num is invalid.(ret=%u, num=%u)\n", ret, num);
        return BBOX_EINVAL;
    }

    ret = of_property_read_u32_index(np, "value", 0, &g_bbox_config_dts_rootfscms.min_valid_value);
    if (ret != 0) {
        BB_PRINT_ERR("Cannot find rootfs cms min value in dts.(ret=%d)\n", ret);
        return BBOX_FAILURE;
    }
    ret = of_property_read_u32_index(np, "value", 1, &g_bbox_config_dts_rootfscms.max_valid_value);
    if (ret != 0) {
        BB_PRINT_ERR("Cannot find rootfs cms max value in dts.(ret=%d)\n", ret);
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

const struct bbox_config_bootstatus *bbox_get_config_bootstatus(u32 *num)
{
    *num = sizeof(g_bbox_config_dts_bootstatus) / sizeof(g_bbox_config_dts_bootstatus[0]);
    return g_bbox_config_dts_bootstatus;
}

struct bbox_config_rootfscms bbox_get_config_rtoofscms(void)
{
    return g_bbox_config_dts_rootfscms;
}

STATIC void bbox_cofing_print_bootstatus(void)
{
    u32 num = sizeof(g_bbox_config_dts_bootstatus) / sizeof(g_bbox_config_dts_bootstatus[0]), i, j;
    for (i = 0; i < num; i++) {
        if (g_bbox_config_dts_bootstatus[i].file_num == 0) {
            continue;
        }
        BB_PRINT_INFO("Boot status info.(block_id=%u, file_num=%u, stage_id=%u)\n",
                      g_bbox_config_dts_bootstatus[i].block_id,
                      g_bbox_config_dts_bootstatus[i].file_num,
                      g_bbox_config_dts_bootstatus[i].stage_id);
        for (j = 0; j < g_bbox_config_dts_bootstatus[i].file_num; j++) {
            BB_PRINT_INFO("File info.(file_id=%u, min=%u, max=%u)\n",
                          g_bbox_config_dts_bootstatus[i].file_list[j].file_id,
                          g_bbox_config_dts_bootstatus[i].file_list[j].min_valid_value,
                          g_bbox_config_dts_bootstatus[i].file_list[j].max_valid_value);
        }
    }
    BB_PRINT_INFO("Rootfs cms info.(block_id=%u, min=%u, max=%u, result=%u, stage_id=%u)\n",
                  g_bbox_config_dts_rootfscms.block_id,
                  g_bbox_config_dts_rootfscms.min_valid_value,
                  g_bbox_config_dts_rootfscms.max_valid_value,
                  g_bbox_config_dts_rootfscms.result,
                  g_bbox_config_dts_rootfscms.stage_id);
}
#endif

/*
 * @brief       : dts config init
 * @return      : <0 failure; =0 success
 */
s32 bbox_config_init(void)
{
    s32 ret;

    ret = memset_s(&g_bbox_config_dts, sizeof(g_bbox_config_dts), 0, sizeof(g_bbox_config_dts));
    BB_CHECK_RET(ret != EOK, "[%s:%d] memset_s dts config error.\n", __func__, __LINE__);

    ret = bbox_config_init_rdr_basic();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("[%s]: config common early init failed!\n", __func__);
        return ret;
    }

    ret = bbox_config_init_ddr_mntn();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("[%s]: failed to get ddr mntn!\n", __func__);
        return ret;
    }

    ret = bbox_config_init_regs();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("[%s]: failed to get os regs!\n", __func__);
        return ret;
    }

    ret = bbox_config_init_reboot_reason();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("[%s]: failed to get reboot reason regs address!\n", __func__);
        return ret;
    }

    ret = bbox_config_init_spmi_ctrl();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("[%s]: failed to get spmi ctrl!\n", __func__);
        return ret;
    }

    ret = bbox_config_init_hdr();
    if ((ret != BBOX_SUCCESS) && (ret != BBOX_ENODEV)) {
        BB_PRINT_ERR("[%s]: failed to get hdr!\n", __func__);
        return ret;
    }

    ret = bbox_config_init_cdr();
    if ((ret != BBOX_SUCCESS) && (ret != BBOX_ENODEV)) {
        BB_PRINT_ERR("[%s]: failed to get cdr!\n", __func__);
        return ret;
    }

    ret = bbox_config_init_boot_param();
    if ((ret != BBOX_SUCCESS) && (ret != BBOX_ENODEV)) {
        BB_PRINT_ERR("[%s]: failed to get boot param!\n", __func__);
        return ret;
    }
#if (defined BBOX_SOC_PLATFORM_MDC && !defined BBOX_UT)
    ret = bbox_config_init_bootstatus();
    if ((ret != BBOX_SUCCESS) && (ret != BBOX_ENODEV)) {
        BB_PRINT_ERR("Failed to get boot status info. (func_name=%s)\n", __func__);
        return ret;
    }

    ret = bbox_config_init_rootfscms();
    if ((ret != BBOX_SUCCESS) && (ret != BBOX_ENODEV)) {
        BB_PRINT_ERR("Failed to get rootfs cms info. (func_name=%s)\n", __func__);
        return ret;
    }

    bbox_cofing_print_bootstatus();
#endif
    return BBOX_SUCCESS;
}

/*
 * @brief       : get dts config
 * @return      : config point
 */
const struct bbox_config_data *bbox_get_config(void)
{
    return &g_bbox_config_dts;
}

