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

#include <securec.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/hashtable.h>
#include <linux/mutex.h>

#include "davinci_api.h"
#include "dp_proc_mng_cmd.h"
#include "dp_proc_mng_log.h"
#include "dp_proc_mng_proc_info.h"

/*
 * dp_proc_mng module init
 */
STATIC struct dp_proc_mng_info *dp_proc_mng = NULL;

struct dp_proc_mng_info *dp_proc_get_manager_info(void)
{
    return dp_proc_mng;
}

int dp_proc_mng_info_init(void)
{
    dp_proc_mng = kzalloc(sizeof(struct dp_proc_mng_info), GFP_KERNEL);
    if (dp_proc_mng == NULL) {
        dp_proc_mng_drv_err("kzalloc return NULL, failed to alloc mem for manager struct.\n");
        return -ENOMEM;
    }

    spin_lock_init(&dp_proc_mng->proc_hash_table_lock);
    INIT_LIST_HEAD(&dp_proc_mng->hostpid_list_header);
    hash_init(dp_proc_mng->proc_hash_table);
    mutex_init(&dp_proc_mng->dp_proc_mng_sign_list_lock);

    return 0;
}

void dp_proc_mng_info_unint(void)
{
    kfree(dp_proc_mng);
    dp_proc_mng = NULL;
}

int dp_proc_mng_davinci_module_init(const struct file_operations *ops)
{
    int ret;

    ret = drv_davinci_register_sub_module(DAVINCI_DP_PROC_MNG_SUB_MODULE_NAME, ops);
    if (ret != 0) {
        dp_proc_mng_drv_err("Register sub module failed. (ret=%d)\n", ret);
        return -ENODEV;
    }
    return 0;
}

void dp_proc_mng_davinci_module_uninit(void)
{
    int ret;

    ret = drv_ascend_unregister_sub_module(DAVINCI_DP_PROC_MNG_SUB_MODULE_NAME);
    if (ret != 0) {
        dp_proc_mng_drv_err("Unregister sub module failed. (ret=%d)\n", ret);
        return;
    }
    return;
}

STATIC int dp_proc_mng_create_random_sign(char *random_sign, u32 len)
{
    char random[RANDOM_SIZE] = {0};
    int offset = 0;
    int ret;
    int i;

    for (i = 0; i < RANDOM_SIZE; i++) {
        ret = snprintf_s(random_sign + offset, len - offset, len - 1 - offset, "%02x", (u8)random[i]);
        if (ret < 0) {
            dp_proc_mng_drv_err("snprintf failed, ret(%d).\n", ret);
            return -EINVAL;
        }
        offset += ret;
    }
    random_sign[len - 1] = '\0';

    return 0;
}

STATIC int dp_proc_get_process_sign(struct dp_proc_mng_info *d_info, char *sign, u32 len, u32 docker_id)
{
    struct dp_proc_mng_sign *d_sign = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    int ret;

    if (list_empty_careful(&d_info->hostpid_list_header) == 0) {
#ifndef EMU_ST
        list_for_each_safe(pos, n, &d_info->hostpid_list_header) {
            d_sign = list_entry(pos, struct dp_proc_mng_sign, list);
            if (d_sign->hostpid == current->tgid) {
                ret = strcpy_s(sign, len, d_sign->sign);
                if (ret != 0) {
                    dp_proc_mng_drv_err("Copy hostpid sign failed. (docker_id=%u; hostpid=%d; ret=%d)\n",
                        docker_id, d_sign->hostpid, ret);
                    return -EINVAL;
                }

                return 0;
            }
        }
#endif
    }

    if (d_info->dp_proc_mng_sign_count[docker_id] >= DP_PROC_MNG_MAX_SIGN_NUM) {
        dp_proc_mng_drv_err("List add failed. (docker_id=%u).\n", docker_id);
        return -EINVAL;
    }

    d_sign = kzalloc(sizeof(struct dp_proc_mng_sign), GFP_KERNEL | GFP_ACCOUNT);
    if (d_sign == NULL) {
        dp_proc_mng_drv_err("kzalloc failed. (docker_id=%u)\n", docker_id);
        return -ENOMEM;
    }
    d_sign->hostpid = current->tgid;
    d_sign->docker_id = docker_id;

    ret = dp_proc_mng_create_random_sign(d_sign->sign, PROCESS_SIGN_LENGTH);
    if (ret != 0) {
        dp_proc_mng_drv_err("Get sign failed. (ret=%d; docker_id=%u)\n", ret, docker_id);
        kfree(d_sign);
        d_sign = NULL;
        return ret;
    }
    ret = strcpy_s(sign, len, d_sign->sign);
    if (ret != 0) {
        dp_proc_mng_drv_err("strcpy_s failed. (ret=%d; docker_id=%u)\n", ret, docker_id);
        kfree(d_sign);
        d_sign = NULL;
        return -EINVAL;
    }

    list_add(&d_sign->list, &d_info->hostpid_list_header);
    d_info->dp_proc_mng_sign_count[d_sign->docker_id]++;
    dp_proc_mng_drv_info("Process sign list add success. (docker_id=%u; hostpid=%d)\n", docker_id, d_sign->hostpid);
    return 0;
}

STATIC int dp_proc_mng_get_process_sign(struct file *file, struct dp_proc_mng_ioctl_arg *arg)
{
    struct dp_proc_mng_info *d_info = dp_proc_get_manager_info();
    u32 docker_id = MAX_DOCKER_NUM;
    int ret;

    if (d_info == NULL) {
        dp_proc_mng_drv_err("d_info is null.\n");
        return -EINVAL;
    }

    /*
     * check current environment :
     * non-container : docker_id is set to MAX_DOCKER_NUM;
     * container : get docker_id , docker_id is 0 ~ MAX_DOCKER_NUM - 1.
     */
    ret = devdrv_manager_container_is_in_container();
    if (ret != 0) {
        ret = devdrv_manager_container_get_docker_id(&docker_id);
        if (ret != 0) {
            dp_proc_mng_drv_err("container get docker_id failed, ret(%d).\n", ret);
            return -EINVAL;
        }
    }

    mutex_lock(&d_info->dp_proc_mng_sign_list_lock);
    ret = dp_proc_get_process_sign(d_info, arg->data.get_process_sign_para.sign, PROCESS_SIGN_LENGTH, docker_id);
    if (ret != 0) {
        mutex_unlock(&d_info->dp_proc_mng_sign_list_lock);
        dp_proc_mng_drv_err("get process_sign failed, ret(%d).\n", ret);
        return ret;
    }
    mutex_unlock(&d_info->dp_proc_mng_sign_list_lock);

    arg->data.get_process_sign_para.tgid = current->tgid;

    return 0;
}

int (* const dp_proc_mng_ioctl_handlers[DP_PROC_MNG_CMD_MAX_CMD])(struct file *file,
    struct dp_proc_mng_ioctl_arg *arg) = {
        [_IOC_NR(DP_PROC_MNG_PROCESS_STATUS_QUERY)] = NULL,
        [_IOC_NR(DP_PROC_MNG_GET_PROCESS_SIGN)] = dp_proc_mng_get_process_sign,
};

int dp_proc_mng_create_work(void)
{
    return 0;
}

void dp_proc_mng_destroy_work(void)
{
}

