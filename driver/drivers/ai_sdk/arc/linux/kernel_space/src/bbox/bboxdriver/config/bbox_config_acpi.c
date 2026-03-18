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

#include "bbox_config_acpi.h"
#include <linux/acpi.h>
#include <linux/securec.h>
#include "bbox_config.h"
#include "bbox_product.h"
#include "bbox_platform.h"
#include "common/bbox_print.h"
#include "common/bbox_common.h"

#define ACPI_ARRAY_MAX_CNT      128
#define ACPI_ARRAY_MIN_CNT      2
#define ACPI_BBOX_DEVICE_HID    "BBOX0000"
#define ACPI_RECURSION_DEPTH    10
#define ACPI_PATH_NAME          "_DSD"
#define ACPI_U32_MAX            0xFFFFFFFFULL

STATIC struct bbox_config_data g_bbox_config_acpi;
STATIC const union acpi_object *g_bbox_config_acpi_obj = NULL;

/*
 * @brief           : get acpi device cb, call back function for acpi_get_devices
 * @param [in]      : acpi_handle handle            acpi handler
 * @param [in]      : u32 depth                     depth, unused
 * @param [inout]   : void *context                 unused
 * @param [inout]   : void **ret                    unused
 * @return          : AE_OK success; AE_ERROR failure
 */
STATIC acpi_status bbox_config_acpi_device_cb(acpi_handle handle, u32 depth, void *context, void **ret)
{
    acpi_status status;
    union acpi_object *obj = NULL;
    struct acpi_buffer buf = {ACPI_ALLOCATE_BUFFER, NULL};

    BB_CHECK_PTR(handle, return AE_ERROR, "handle is NULL.\n");
    UNUSED(depth);      // input param unused
    UNUSED(ret);        // input param unused
    UNUSED(context);    // input param unused
    status = acpi_evaluate_object(handle, ACPI_PATH_NAME, NULL, &buf);
    if (ACPI_FAILURE(status)) {
        BB_PRINT_ERR("get dsd object failed\n");
        return AE_ERROR;
    }

    obj = (union acpi_object *)buf.pointer;
    if (obj->type != ACPI_TYPE_PACKAGE) {
        BB_PRINT_ERR("dsd object get error type[%u].\n", obj->type);
        kfree(buf.pointer);
        buf.pointer = NULL;
        return AE_ERROR;
    }

    g_bbox_config_acpi_obj = obj;
    return AE_OK;
}

/*
 * @brief       : get acpi handler by calling acpi_get_devices
 * @return      : =0 success; =-1 failure
 */
STATIC s32 bbox_config_get_bbox_acpi_handle(void)
{
    acpi_status status;

    status = acpi_get_devices(ACPI_BBOX_DEVICE_HID, bbox_config_acpi_device_cb, NULL, NULL);
    if (ACPI_FAILURE(status) || g_bbox_config_acpi_obj == NULL) {
        BB_PRINT_ERR("No device in acpi, status: %u.\n", status);
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : recurse to get the property object by property name
 * @param [in]  : const union acpi_object *obj       acpi property object
 * @param [in]  : const char *propname               property name
 * @param [in]  : u32 depth                          recursions number
 * @return      : !NULL success; NULL failure
 */
STATIC const union acpi_object *bbox_config_find_property_obj(const union acpi_object *obj,
                                                              const char *propname,
                                                              u32 depth)
{
    s32 i;
    s32 num;
    const union acpi_object *header_obj = NULL;
    const union acpi_object *ret_obj = NULL;
    const union acpi_object *sub_obj = NULL;

    BB_CHECK_PTR(obj, return NULL, "invalid input. object is NULL.\n");
    BB_CHECK_PTR(propname, return NULL, "invalid input. propname is NULL.\n");
    BB_CHECK_EXP_ACT(depth == 0, return NULL, "invalid input. depth[%u].\n", depth);

    depth--;
    if (obj->type != ACPI_TYPE_PACKAGE) {
        BB_PRINT_ERR("acpi object pkg type error, type[%u]\n", obj->type);
        return NULL;
    }
    num = (s32)obj->package.count;
    if (num > ACPI_ARRAY_MAX_CNT || num < ACPI_ARRAY_MIN_CNT) {
        BB_PRINT_ERR("acpi object pkg element num error, num[%d]\n", num);
        return NULL;
    }
    // search first package element, if the the property name matches, return the second element
    header_obj = &obj->package.elements[0];
    if ((header_obj != NULL) &&
        (header_obj->type == ACPI_TYPE_STRING) &&
        (strcmp(header_obj->string.pointer, propname) == BBOX_EOK)) {
        ret_obj = &obj->package.elements[1];
        return ret_obj;
    }

    for (i = 0; i < num; i++) {
        sub_obj = &obj->package.elements[i];
        if ((sub_obj != NULL) && (sub_obj->type == ACPI_TYPE_PACKAGE)) {
            // recurse to get in sub obj
            ret_obj = bbox_config_find_property_obj(sub_obj, propname, depth);
            if (ret_obj != NULL) {
                return ret_obj;
            }
        }
    }

    return NULL;
}

/*
 * @brief           : get the int property value by property name
 * @param [in]      : const char *propname                  property name
 * @param [out]     : u64 *val                              return property value
 * @return          : <0 failure; =0 success
 */
STATIC s32 bbox_config_property_read_u64(const char *propname, u64 *val)
{
    const union acpi_object *curr_obj = NULL;

    BB_CHECK_PTR(val, return BBOX_FAILURE, "invalid input. val is NULL.\n");
    BB_CHECK_PTR(propname, return BBOX_FAILURE, "invalid input. propname is NULL.\n");

    curr_obj = bbox_config_find_property_obj(g_bbox_config_acpi_obj, propname, ACPI_RECURSION_DEPTH);
    if (curr_obj == NULL) {
        BB_PRINT_INFO("no find propname[%s] in acpi.\n", propname);
        return BBOX_FAILURE;
    }

    if (curr_obj->type != ACPI_TYPE_INTEGER) {
        BB_PRINT_ERR("%s: acpi object error, type[%u].\n", __func__, curr_obj->type);
        return BBOX_FAILURE;
    }

    *val = curr_obj->integer.value;
    return BBOX_SUCCESS;
}

/*
 * @brief           : get the u64 array property value by property name
 * @param [in]      : const char *propname                  property name
 * @param [out]     : u64 *val                              return property value
 * @param [in]      : u32 num                               the number of property val
 * @return          : <0 failure; =0 success
 */
STATIC s32 bbox_config_property_read_u64_array(const char *propname, u64 *val, u32 num)
{
    s32 i;
    const union acpi_object *curr_obj = NULL;
    const union acpi_object *sub_obj = NULL;

    BB_CHECK_PTR(val, return BBOX_FAILURE, "invalid input. val is NULL.\n");
    BB_CHECK_PTR(propname, return BBOX_FAILURE, "invalid input. propname is NULL.\n");
    BB_CHECK_EXP_ACT(num == 0 || num > ACPI_ARRAY_MAX_CNT,
        return BBOX_FAILURE, "invalid input. num[%u].\n", num);

    curr_obj = bbox_config_find_property_obj(g_bbox_config_acpi_obj, propname, ACPI_RECURSION_DEPTH);
    if (curr_obj == NULL) {
        BB_PRINT_INFO("no find propname[%s] in acpi\n", propname);
        return BBOX_FAILURE;
    }

    if (curr_obj->type != ACPI_TYPE_PACKAGE) {
        BB_PRINT_ERR("%s: acpi object error, type[%u]\n", __func__, curr_obj->type);
        return BBOX_FAILURE;
    }

    for (i = 0; i < (s32)curr_obj->package.count && i < (s32)num; i++) {
        sub_obj = &curr_obj->package.elements[i];
        if (sub_obj->type == ACPI_TYPE_INTEGER) {
            val[i] = sub_obj->integer.value;
        }
    }
    return BBOX_SUCCESS;
}

/*
 * @brief           : get the u32 property value by property name
 * @param [in]      : const char *propname                  property name
 * @param [out]     : u32 *val                              return property value
 * @return          : <0 failure; =0 success
 */
STATIC s32 bbox_config_property_read_u32(const char *propname, u32 *val)
{
    s32 ret;
    u64 tmp = 0;

    BB_CHECK_PTR(val, return BBOX_FAILURE, "invalid input. val is NULL.\n");
    BB_CHECK_PTR(propname, return BBOX_FAILURE, "invalid input. propname is NULL.\n");

    ret = bbox_config_property_read_u64(propname, &tmp);
    if (ret != BBOX_SUCCESS || tmp > ACPI_U32_MAX) {
        BB_PRINT_ERR("Read u32 error, propname[%s] value[%llu].\n", propname, tmp);
        return BBOX_FAILURE;
    }
    *val = (u32)tmp;
    return ret;
}

/*
 * @brief           : get the u32 array property value by property name
 * @param [in]      : const char *propname                  property name
 * @param [out]     : u32 *val                              return property value
 * @param [in]      : u32 num                               the number of property
 * @return          : <0 failure; =0 success
 */
STATIC s32 bbox_config_property_read_u32_array(const char *propname, u32 *val, u32 num)
{
    s32 i;
    s32 ret;
    u64 *tmp = NULL;

    BB_CHECK_PTR(val, return BBOX_FAILURE, "invalid input. val is NULL.\n");
    BB_CHECK_PTR(propname, return BBOX_FAILURE, "invalid input. propname is NULL.\n");
    BB_CHECK_EXP_ACT(num == 0 || num > ACPI_ARRAY_MAX_CNT, return BBOX_FAILURE, "Invalid input. num[%u].\n", num);

    tmp = (u64 *)bbox_vmalloc(sizeof(u64) * num);
    if (tmp == NULL) {
        BB_PRINT_ERR("Bbox vmalloc u32 array tmp buffer failed.\n");
        return BBOX_FAILURE;
    }

    ret = bbox_config_property_read_u64_array(propname, tmp, num);
    if (ret != BBOX_SUCCESS) {
        BBOX_VFREE(tmp);
        return ret;
    }

    for (i = 0; i < (s32)num; i++) {
        if (tmp[i] > ACPI_U32_MAX) {
            BB_PRINT_ERR("Read u32 array error, id[%d] propname[%s] value[%llu] will be truncated.\n",
                         i, propname, tmp[i]);
            BBOX_VFREE(tmp);
            return BBOX_FAILURE;
        }
        val[i] = (u32)tmp[i];
    }
    BBOX_VFREE(tmp);
    return BBOX_SUCCESS;
}

/*
 * @brief           : get array number by property name
 * @param [in]      : const char *propname            property name
 * @return          : <0 failure; >=0 success
 */
STATIC s32 bbox_config_property_read_arrary_num(const char *propname)
{
    const union acpi_object *curr_obj = NULL;

    BB_CHECK_PTR(propname, return BBOX_FAILURE, "invalid input. propname is NULL.\n");

    curr_obj = bbox_config_find_property_obj(g_bbox_config_acpi_obj, propname, ACPI_RECURSION_DEPTH);
    if (curr_obj == NULL) {
        BB_PRINT_ERR("%s: cannot find propname[%s] in current obj\n", __func__, propname);
        return BBOX_FAILURE;
    }

    if (curr_obj->type != ACPI_TYPE_PACKAGE) {
        BB_PRINT_ERR("%s: acpi pkg object error, type[%u]\n", __func__, curr_obj->type);
        return BBOX_FAILURE;
    }

    return (s32)curr_obj->package.count;
}

/*
 * @brief       : get the string array property value by property name
 * @param [in]  : const char *propname                  property name
 * @param [out] : const char **val                      return property value
 * @param [in]  : u32 num                               the number of property
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_config_property_read_string_array(const char *propname, const char **val, u32 num)
{
    s32 i;
    const union acpi_object *curr_obj = NULL;
    const union acpi_object *sub_obj = NULL;

    BB_CHECK_PTR(val, return BBOX_FAILURE, "invalid input. val is NULL.\n");
    BB_CHECK_PTR(propname, return BBOX_FAILURE, "invalid input. propname is NULL.\n");
    BB_CHECK_EXP_ACT(num == 0 || num > ACPI_ARRAY_MAX_CNT, return BBOX_FAILURE, "Invalid input. num[%u].\n", num);

    curr_obj = bbox_config_find_property_obj(g_bbox_config_acpi_obj, propname, ACPI_RECURSION_DEPTH);
    if (curr_obj == NULL) {
        BB_PRINT_ERR("%s: cannot find propname[%s] in obj\n", __func__, propname);
        return BBOX_FAILURE;
    }

    if (curr_obj->type != ACPI_TYPE_PACKAGE) {
        BB_PRINT_ERR("%s: acpi object error, type[%u]\n", __func__, curr_obj->type);
        return BBOX_FAILURE;
    }

    for (i = 0; i < (s32)curr_obj->package.count && i < (s32)num; i++) {
        sub_obj = &curr_obj->package.elements[i];
        if (sub_obj->type == ACPI_TYPE_STRING) {
            val[i] = sub_obj->string.pointer;
        }
    }

    return BBOX_SUCCESS;
}

STATIC bool bbox_config_check_addr_valid(u64 addr, u64 size)
{
    if ((addr == 0) || (size == 0) ||
        (addr >= BBOX_UINT_INVALID) || (size >= BBOX_UINT_INVALID) ||
        (addr + size > BBOX_UINT_INVALID)) {
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
    s32 ret;
    s32 num;

    ret = bbox_config_property_read_u64("rdr_base_addr", &g_bbox_config_acpi.bbox_data.addr);
    if (ret == BBOX_SUCCESS) {
        ret = bbox_config_property_read_u64("rdr_base_len", &g_bbox_config_acpi.bbox_data.length);
    }

    if ((ret != BBOX_SUCCESS) ||
        (bbox_config_check_addr_valid(g_bbox_config_acpi.bbox_data.addr,
            g_bbox_config_acpi.bbox_data.length) == false)) {
        BB_PRINT_ERR("Get bbox reserved addr failed with %d.\n", ret);
        return BBOX_FAILURE;
    }

    ret = bbox_config_property_read_u32("wait-dumplog-timeout", &g_bbox_config_acpi.wait_timeout);
    if (ret != BBOX_SUCCESS || (g_bbox_config_acpi.wait_timeout > DUMP_LOG_MAX_TIME)) {
        BB_PRINT_ERR("Find wait dumplog timeout[%u] failed with %d!\n",
                     g_bbox_config_acpi.wait_timeout, ret);
        return BBOX_FAILURE;
    }

    num = bbox_config_property_read_arrary_num("rdr_area_names");
    if (num <= 0 || num > (s32)BBOX_AREA_MAXIMUM) {
        BB_PRINT_ERR("[%s], invaild core num[%d]!\n", __func__, num);
        return BBOX_FAILURE;
    }
    g_bbox_config_acpi.reserved_area.area_num = (u32)num;

    ret = bbox_config_property_read_string_array("rdr_area_names",
                                                 g_bbox_config_acpi.reserved_area.area_name, (u32)num);
    if (ret == BBOX_SUCCESS) {
        ret = bbox_config_property_read_u32_array("rdr_area_sizes",
                                                  g_bbox_config_acpi.reserved_area.area_size, (u32)num);
    }
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("[%s], find rdr area info failed with %d!\n", __func__, ret);
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : get basic lpfw configuration.
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_config_init_ddr_mntn(void)
{
    s32 ret;
    u64 addr = 0;
    u64 size = 0;

    ret = bbox_config_property_read_u64("sram_base_addr", &addr);
    if (ret == BBOX_SUCCESS) {
        ret = bbox_config_property_read_u64("sram_base_len", &size);
    }
    if ((ret != BBOX_SUCCESS) ||
        (size > LPFW_SRAM_MAX_SIZE) ||
        (bbox_config_check_addr_valid(addr, size) == false)) {
        BB_PRINT_ERR("read sram info failed with %d!\n", ret);
        return BBOX_FAILURE;
    }

    g_bbox_config_acpi.ddr_mntn.addr = addr;
    g_bbox_config_acpi.ddr_mntn.length = size;
    return BBOX_SUCCESS;
}

/*
 * @brief       : get regs configuration data.
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_config_init_regs(void)
{
    s32 i;
    s32 ret;
    s32 num;

    num = bbox_config_property_read_arrary_num("reg_name");
    if (num <= 0 || num > (s32)REGS_DUMP_MAX_NUM) {
        BB_PRINT_ERR("read reg name num error. num[%d]\n", num);
        return BBOX_FAILURE;
    }

    g_bbox_config_acpi.ap_regs_info.reg_num = (u32)num;
    ret = bbox_config_property_read_string_array("reg_name", g_bbox_config_acpi.ap_regs_info.reg_name, (u32)num);
    if (ret == BBOX_SUCCESS) {
        ret = bbox_config_property_read_u64_array("reg_addr", g_bbox_config_acpi.ap_regs_info.addr, (u32)num);
        if (ret == BBOX_SUCCESS) {
            ret = bbox_config_property_read_u64_array("reg_size", g_bbox_config_acpi.ap_regs_info.size, (u32)num);
        }
    }

    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("Get os regs info failed with %d!\n", ret);
        return BBOX_FAILURE;
    }

    for (i = 0; i < num; i++) {
        if ((strlen(g_bbox_config_acpi.ap_regs_info.reg_name[i]) > REG_NAME_LEN) ||
            (!bbox_config_check_addr_valid(g_bbox_config_acpi.ap_regs_info.addr[i],
                g_bbox_config_acpi.ap_regs_info.size[i]))) {
            BB_PRINT_ERR("Get regs config failed, idx[%d] reg name:%s!\n",
                         i, g_bbox_config_acpi.ap_regs_info.reg_name[i]);
            return BBOX_FAILURE;
        }
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

    ret = bbox_config_property_read_u64("pmu_base_addr", &g_bbox_config_acpi.pmu_reset_reg.addr);
    if (ret != BBOX_SUCCESS || g_bbox_config_acpi.pmu_reset_reg.addr == 0) {
        BB_PRINT_ERR("Get reboot reason regs addr failed with %d!\n", ret);
        return BBOX_FAILURE;
    }

    ret = bbox_config_property_read_u64("pmu_base_len", &g_bbox_config_acpi.pmu_reset_reg.length);
    if (ret != BBOX_SUCCESS || g_bbox_config_acpi.pmu_reset_reg.length == 0) {
        BB_PRINT_ERR("Get reboot reason regs len[0x%llx] failed with %d!\n",
                     g_bbox_config_acpi.pmu_reset_reg.length, ret);
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : get basic hdr configuration.
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_config_init_hdr(void)
{
    s32 ret;
    u64 addr = 0;
    u64 size = 0;

    ret = bbox_config_property_read_u64("hdr_base_addr", &addr);
    if (ret == BBOX_SUCCESS) {
        ret = bbox_config_property_read_u64("hdr_base_len", &size);
    }

    if (ret != BBOX_SUCCESS) {
        return BBOX_ENODEV;
    }
    if ((size > HDR_MAX_SIZE) ||
        (bbox_config_check_addr_valid(addr, size) == false)) {
        BB_PRINT_ERR("Read hdr info failed with %d!\n", ret);
        return BBOX_FAILURE;
    }

    g_bbox_config_acpi.hdr.addr = addr;
    g_bbox_config_acpi.hdr.length = size;
    return BBOX_SUCCESS;
}

STATIC bool bbox_check_feature_cdr(void)
{
    return bbox_check_feature(FEATURE_BOOTCHECK_CDR);
}

/*
 * @brief       : get basic cdr configuration.
 * @return      : <0 failure; =0 success
 */
STATIC s32 bbox_config_init_cdr(void)
{
    s32 ret;
    u64 addr = 0;
    u64 size = 0;

    if (bbox_check_feature_cdr() == false) {
        return BBOX_SUCCESS;
    }

    ret = bbox_config_property_read_u64("chip_dfx_addr", &addr);
    if (ret == BBOX_SUCCESS) {
        ret = bbox_config_property_read_u64("chip_dfx_len", &size);
    }

    if (ret != BBOX_SUCCESS) {
        return BBOX_ENODEV;
    }

    if ((size > CDR_MAX_SIZE) ||
        (bbox_config_check_addr_valid(addr, size) == false)) {
        BB_PRINT_ERR("read chip info failed with %d!\n", ret);
        return BBOX_FAILURE;
    }

    g_bbox_config_acpi.cdr.addr = addr;
    g_bbox_config_acpi.cdr.length = size;
    return BBOX_SUCCESS;
}

/*
 * @brief       : read configuration from acpi.
 * @return      : <0 failure; =0 success
 */
s32 bbox_config_init(void)
{
    s32 ret;

    BB_PRINT_INFO("bbox config init.\n");
    ret = memset_s(&g_bbox_config_acpi, sizeof(g_bbox_config_acpi), 0, sizeof(g_bbox_config_acpi));
    BB_CHECK_RET(ret != EOK, "[%s:%d] memset_s acpi config error.\n", __func__, __LINE__);

    g_bbox_config_acpi.spmi_channel = 0;

    // get acpi config data handle
    ret = bbox_config_get_bbox_acpi_handle();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("Init bbox acpi handle failed with %d.\n", ret);
        return ret;
    }

    // get rdr basic config
    ret = bbox_config_init_rdr_basic();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("Init rdr basic config failed with %d.\n", ret);
        return ret;
    }

    // get the addr config to save lpfw sram
    ret = bbox_config_init_ddr_mntn();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("Init ddr mntn config failed with %d.\n", ret);
        return ret;
    }

    // get the addr config of ap regs
    ret = bbox_config_init_regs();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("Init regs config failed with %d.", ret);
        return ret;
    }

    // get the addr config of pmu
    ret = bbox_config_init_reboot_reason();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("Init reboot reason regs config failed with %d.", ret);
        return ret;
    }

    ret = bbox_config_init_hdr();
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS && ret != BBOX_ENODEV, return ret, "init hdr config failed with %d.", ret);

    ret = bbox_config_init_cdr();
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS && ret != BBOX_ENODEV, return ret, "init cdr config failed with %d.", ret);

    return BBOX_SUCCESS;
}

/*
 * @brief       : get acpi config data
 * @return      : config data point
 */
const struct bbox_config_data *bbox_get_config(void)
{
    return &g_bbox_config_acpi;
}

