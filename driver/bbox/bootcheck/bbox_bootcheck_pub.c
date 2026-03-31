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

#include "bbox_bootcheck_pub.h"

#include <linux/securec.h>

#include "bbox_proxy.h"
#include "common/bbox_print.h"
#include "rdr/rdr_module_core.h"

/*
 * @brief       : get coreid by etype form bootcheck exception table
 * @param [in]  : u32 etype      exception type
 * @return      : coreid, default is BBOX_CORE_MAX
 */
u8 bbox_bootcheck_get_coreid(u32 etype)
{
    s32 i, ret;
    struct bbox_bootcheck_exception_table table;

    ret = memset_s(&table, sizeof(table), 0, sizeof(table));
    BB_CHECK_SECUREC(BB_CHECK_MEMSET, ret);

    ret = bbox_bootcheck_get_einfo(&table);
    if (ret != BBOX_SUCCESS) {
        return BBOX_CORE_MAX;
    }

    for (i = 0; i < (s32)table.size; i++) {
        if (table.list[i].etype == etype) {
            return table.list[i].coreid;
        }
    }

    return BBOX_CORE_MAX;
}

/*
 * @brief       : get excepid by etype form bootcheck exception table
 * @param [in]  : u8 etype      exception type
 * @return      : excepid, default is 0
 */
u32 bbox_bootcheck_get_excepid(u8 etype)
{
    s32 i, ret;
    struct bbox_bootcheck_exception_table table;

    ret = memset_s(&table, sizeof(table), 0, sizeof(table));
    BB_CHECK_SECUREC(BB_CHECK_MEMSET, ret);

    ret = bbox_bootcheck_get_einfo(&table);
    if (ret != BBOX_SUCCESS) {
        return 0;
    }

    for (i = 0; i < (s32)table.size; i++) {
        if (table.list[i].etype == etype) {
            return table.list[i].excepid;
        }
    }

    return 0;
}

/*
 * @brief       : get proxy module info
 * @param [in]  : struct bbox_bootcheck_proxy_info      proxy info
 * @param [out] : struct bbox_bootcheck_proxy_info      proxy info
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_bootcheck_get_proxy_info(struct bbox_bootcheck_proxy_info *info)
{
    s32 i;
    const struct bbox_proxy_module_ctrl *ctrl = NULL;

    BB_CHECK_PTR(info, return BBOX_FAILURE, "invalid param, info is NULL.\n");
    BB_CHECK_PTR((struct bbox_proxy_module_ctrl *)(uintptr_t)info->buffer,
        return BBOX_FAILURE, "invalid param, buffer is NULL.\n");
    ctrl = (struct bbox_proxy_module_ctrl *)(uintptr_t)info->buffer;
    if (ctrl->magic != BBOX_PROXY_MAGIC) {
        BB_PRINT_ERR("bootcheck proxy module data is unavailable.\n");
        return BBOX_FAILURE;
    }

    for (i = 0; i < (s32)ctrl->config.e_block_num; i++) {
        if (ctrl->config.block_info[i].ctrl_flag == (u16)CHECK_NONE) {
            info->offset = ctrl->config.block_info[i].info_offset;
            info->len = ctrl->config.block_info[i].info_block_len;
            break;
        }

        if (((ctrl->config.block_info[i].ctrl_flag & (u16)CHECK_STARTUP_EXCEPID) != 0) ||
            ((ctrl->config.block_info[i].ctrl_flag & (u16)CHECK_RUNTIME_EXCEPID) != 0)) {
            if (ctrl->block[i].e_main_excepid != info->excepid) {
                continue;
            }
        }

        info->offset = ctrl->config.block_info[i].info_offset;
        info->len = ctrl->config.block_info[i].info_block_len;
        if (((ctrl->config.block_info[i].ctrl_flag & (u16)CHECK_STARTUP_TMSTMP) != 0) ||
            ((ctrl->config.block_info[i].ctrl_flag & (u16)CHECK_RUNTIME_TMSTMP) != 0)) {
            info->tm.tv_sec = ctrl->block[i].e_clock.tv_sec;
            info->tm.tv_nsec = ctrl->block[i].e_clock.tv_usec * KILO;
            bbox_update_time_seq(&info->tm);
        }

        break;
    }
    return BBOX_SUCCESS;
}

/*
 * @brief       : send proxy module data
 * @param [in]  : struct rdr_exception_msg_info *info       sned msg info
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_bootcheck_get_proxy_module(const struct rdr_exception_msg_info *info)
{
    s32 ret;
    u64 buffer = 0;
    struct bbox_bootcheck_proxy_info proxy = {0};

    BB_CHECK_PTR(info, return BBOX_FAILURE, "invalid param, info is NULL.\n");
    ret = rdr_module_get_vaddr(info->devid, info->coreid, &buffer);
    if ((ret <= 0) || (buffer == 0)) {
        BB_PRINT_ERR("bootcheck get module[%hhu] vaddr failed with %d.\n", info->coreid, ret);
        return BBOX_FAILURE;
    }

    proxy.buffer_size = (u32)ret;
    proxy.excepid = info->excepid;
    proxy.tm.tv_sec = info->tm.tv_sec;
    proxy.tm.tv_nsec = info->tm.tv_nsec;
    proxy.buffer = buffer;
    ret = bbox_bootcheck_get_proxy_info(&proxy);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("bootcheck get module[%hhu] porxy info failed.\n", info->coreid);
        rdr_module_free_vaddr(buffer);
        return BBOX_FAILURE;
    }

    if ((proxy.offset >= proxy.buffer_size) || ((proxy.buffer_size - proxy.offset) < proxy.len)) {
        BB_PRINT_ERR("bootcheck get module[%hhu] porxy info invalid, data[%u@%u] go beyond the bounds[%u].\n",
            info->coreid, proxy.offset, proxy.len, proxy.buffer_size);
        rdr_module_free_vaddr(buffer);
        return BBOX_FAILURE;
    }

    ret = bbox_submit_module_message(info, (char *)(uintptr_t)(buffer + proxy.offset), proxy.len);
    BB_CHECK_RET(ret != BBOX_SUCCESS, "bootcheck send module[%hhu] data failed.\n", info->coreid);
    rdr_module_free_vaddr(buffer);
    return ret;
}

