/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-11-15
*/
#include <linux/fs.h>
#include <linux/mmzone.h>
#include <linux/version.h>

#include "securec.h"
#include "dms_cmd_def.h"
#include "urd_acc_ctrl.h"
#include "dms_mem_info.h"
#include "dms_mem_common.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
ssize_t mem_read_file(struct file *fp, char *dst_addr, loff_t fsize, loff_t *pos)
{
    return kernel_read(fp, dst_addr, fsize, pos);
}
#else
ssize_t mem_read_file(struct file *fp, char *dst_addr, loff_t fsize, loff_t *pos)
{
    mm_segment_t old_fs;
    ssize_t ret;

    old_fs = get_fs();
    /*lint -emacro(501,KERNEL_DS)*/
    set_fs((mm_segment_t)KERNEL_DS); /*lint!e501*/
    ret = vfs_read(fp, dst_addr, fsize, pos);
    set_fs(old_fs);
    return ret;
}
#endif

char *mem_read_line(struct file *fp, char *buf, unsigned int buf_len)
{
    long ret;
    unsigned int i = 0;

    ret = mem_read_file(fp, buf, buf_len - 1, &fp->f_pos);
    if (ret <= 0) {
        dms_err("File filestring not right. (buf_len=%u)\n", buf_len);
        return NULL;
    }

    while (buf[i++] != '\n') {
        if (i >= ret) {
            break;
        }
    }

    if (i < ret) {
        fp->f_pos += i - ret;
    }

    if (i < buf_len) {
        buf[i] = '\0';
    }

    return buf;
}

STATIC int mem_info_read_line(struct file *fp, u64 *read_value)
{
    char mem_buf[MEMINFO_PATH_LEN] = {0};
    char tmp[MEMINFO_PATH_LEN] = {0};
    u64 hbm_size;
    int ret;

    if (mem_read_line(fp, mem_buf, MEMINFO_PATH_LEN) == NULL) {
        dms_err("Read line failed.\n");
        return -EINVAL;
    }

    ret = sscanf_s(mem_buf, "%s %s %s %llu %s",
        tmp, sizeof(tmp), tmp, sizeof(tmp), tmp, sizeof(tmp), &hbm_size, tmp, sizeof(tmp));
    if (ret != MEMINFO_READ_INFO_NUM) {
        dms_err("File sscanf not right. (file=%s, ret=%d)\n", mem_buf, ret);
        return -EINVAL;
    }
    *read_value = hbm_size;
    return 0;
}

STATIC int mem_info_read_hugepage_line(struct file *fp, u32 *read_value)
{
    char mem_buf[MEMINFO_PATH_LEN] = {0};
    char tmp[MEMINFO_PATH_LEN] = {0};
    u32 hugepage_value;
    int ret;

    if (mem_read_line(fp, mem_buf, MEMINFO_PATH_LEN) == NULL) {
        dms_err("read line failed.\n");
        return -EINVAL;
    }

    ret = sscanf_s(mem_buf, "%s %s %s %u",
        tmp, sizeof(tmp), tmp, sizeof(tmp), tmp, sizeof(tmp), &hugepage_value);
    if (ret != MEMINFO_HUGEPAGE_READ_INFO_NUM) {
        dms_err("file %s sscanf not right, ret=%d.\n", mem_buf, ret);
        return -EINVAL;
    }
    *read_value = hugepage_value;
    return 0;
}

int mem_get_info_single(u32 node_index, u64 *total_size, u64 *free_size)
{
    char mem_buf[MEMINFO_PATH_LEN] = {0};
    char node_name[MEMINFO_PATH_LEN] = {0};
    struct file *fp = NULL;
    u32 hugepage_free_num = 0;
    int cnt;
    int ret;

    ret = sprintf_s(node_name, MEMINFO_PATH_LEN, "/sys/devices/system/node/node%u/meminfo", node_index);
    if (ret < 0) {
        dms_err("snprintf_s failed. (ret=%d; node=%u)\n", ret, node_index);
        return -EINVAL;
    }

    fp = filp_open(node_name, O_RDONLY, 0);
    if (IS_ERR_OR_NULL((void const *)fp)) {
        dms_err("File open failed. (file=%s; errno=%ld)\n", node_name, PTR_ERR((void const *)fp));
        return -EINVAL;
    }

    /* read first row line */
    ret = mem_info_read_line(fp, total_size);
    if (ret != 0) {
        filp_close(fp, NULL);
        return ret;
    }

    /* read second row line */
    ret = mem_info_read_line(fp, free_size);
    if (ret != 0) {
        filp_close(fp, NULL);
        return ret;
    }

    /* skip off the unused line */
    for (cnt = 0; cnt < UNUSED_MEMINFO_LINE_NUM; cnt++) {
        if (mem_read_line(fp, mem_buf, MEMINFO_PATH_LEN) == NULL) {
            filp_close(fp, NULL);
            return -EINVAL;
        }
    }

    /* read hugepage free row line */
    ret = mem_info_read_hugepage_line(fp, &hugepage_free_num);
    if (ret != 0) {
        filp_close(fp, NULL);
        return ret;
    }

    *free_size = *free_size + hugepage_free_num * HUGE_PAGE_UNIT_SIZE;

    filp_close(fp, NULL);
    return 0;
}
