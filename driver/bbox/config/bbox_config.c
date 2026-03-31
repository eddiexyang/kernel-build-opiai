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

#include "bbox_config.h"
#include "common/bbox_print.h"

/*
 * @brief       : get rdr paddr and size
 * @param [out] : u64 *paddr    reserved paddr
 * @param [out] : u64 *size     reserved paddr size
 * @return      : <0 failure; ==0 success
 */
s32 bbox_config_get_rdr(u64 *paddr, u64 *size)
{
    const struct bbox_config_data *config = bbox_get_config();
    if (config != NULL) {
        if (paddr != NULL) {
            *paddr = config->bbox_data.addr;
        }
        if (size != NULL) {
            *size = config->bbox_data.length;
        }
        return BBOX_SUCCESS;
    }

    return BBOX_FAILURE;
}

/*
 * @brief       : get hdr paddr and size
 * @param [out] : u64 *paddr    reserved paddr
 * @param [out] : u64 *size     reserved paddr size
 * @return      : <0 failure; ==0 success
 */
s32 bbox_config_get_hdr(u64 *paddr, u64 *size)
{
    const struct bbox_config_data *config = NULL;

    BB_CHECK_PTR(paddr, return BBOX_FAILURE, "invalid parameter. paddr: NULL.\n");
    BB_CHECK_PTR(size, return BBOX_FAILURE, "invalid parameter. size: NULL.\n");
    config = bbox_get_config();
    if (config != NULL) {
        *paddr = config->hdr.addr;
        *size = config->hdr.length;
        return BBOX_SUCCESS;
    }

    return BBOX_FAILURE;
}

/*
 * @brief       : get cdr paddr and size
 * @param [out] : u64 *paddr    reserved paddr
 * @param [out] : u64 *size     reserved paddr size
 * @return      : <0 failure; ==0 success
 */
s32 bbox_config_get_cdr(u64 *paddr, u64 *size)
{
    const struct bbox_config_data *config = NULL;

    BB_CHECK_PTR(paddr, return BBOX_FAILURE, "invalid parameter. paddr: NULL.\n");
    BB_CHECK_PTR(size, return BBOX_FAILURE, "invalid parameter. size: NULL.\n");
    config = bbox_get_config();
    if (config != NULL) {
        *paddr = config->cdr.addr;
        *size = config->cdr.length;
        return BBOX_SUCCESS;
    }

    return BBOX_FAILURE;
}

/*
 * @brief       : get spmi channel
 * @return      : spmi channel num
 */
u32 bbox_config_get_spmi_channel(void)
{
    const struct bbox_config_data *config = bbox_get_config();

    if (config != NULL) {
        return config->spmi_channel;
    }

    return 0;   // default channel is 0
}

/*
 * @brief       : get spmi channel
 * @return      : spmi channel num
 */
u32 bbox_config_get_dumplog_timeout(void)
{
    const struct bbox_config_data *config = bbox_get_config();
    if (config != NULL) {
        return config->wait_timeout;
    }
    return 0;
}

/*
 * @brief       : get boot param paddr and size
 * @param [out] : u64 *paddr    reserved paddr
 * @param [out] : u64 *size     reserved paddr size
 * @return      : <0 failure; ==0 success
 */
s32 bbox_config_get_boot_param(u64 *paddr, u64 *size)
{
    const struct bbox_config_data *config = NULL;

    BB_CHECK_PTR(paddr, return BBOX_FAILURE, "invalid parameter. paddr: NULL.\n");
    BB_CHECK_PTR(size, return BBOX_FAILURE, "invalid parameter. size: NULL.\n");
    config = bbox_get_config();
    if (config != NULL) {
        *paddr = config->boot_param.addr;
        *size = config->boot_param.length;
        return BBOX_SUCCESS;
    }

    return BBOX_FAILURE;
}

