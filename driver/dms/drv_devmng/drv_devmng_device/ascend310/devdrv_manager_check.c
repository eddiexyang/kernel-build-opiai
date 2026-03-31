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


#include <linux/crypto.h>
#include <linux/fs.h>
#include <linux/scatterlist.h>
#include <linux/securec.h>
#include <linux/securectype.h>

#include "devdrv_manager.h"
#include "devdrv_manager_check.h"

STATIC struct devdrv_ts_bin_info dev_ts_bin_info = { 0 };
#define BUFF_LEN 512

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
#include <crypto/hash.h>

STATIC void devdrv_calculate_check_func(const u8 *buf, u32 len, u8 *check)
{
    struct crypto_shash *sha256 = NULL;
    char str[128];
    int i;
    int tlen;

    sha256 = crypto_alloc_shash("sha256", 0, 0);
    if (IS_ERR(sha256)) {
        devdrv_drv_err("crypto_alloc_tfm failed.\n");
        return;
    }

    do {
        SHASH_DESC_ON_STACK(shash, sha256);
        shash->tfm = sha256;

        (void)crypto_shash_init(shash);
        (void)crypto_shash_update(shash, buf, len);
        (void)crypto_shash_final(shash, check);
    } while (0);

    crypto_free_shash(sha256);
    for (i = 0; i < DEVDRV_TS_BIN_CHEKC_LEN; i++) {
        tlen = snprintf_s(&str[(long)i * 2], sizeof(str) / 2, 3, "%02x", check[i]);
        if (tlen < 0) {
            devdrv_drv_err("devdrv_calculate_check_func:%02x snprintf_s fail\n", check[i]);
            return;
        }
    }
    str[i * 2] = '\0'; //lint !e679

    devdrv_drv_info("generate sha256 result: %s\n", str);
}
#else
STATIC void devdrv_calculate_check_func(const u8 *buf, u32 len, u8 *check)
{
    struct scatterlist sg;
    struct hash_desc desc;
    char str[128];
    int i;
    int ret;
    int tlen;

    ret = memset_s(check, sizeof(struct devdrv_check_sum), 0, DEVDRV_TS_BIN_CHEKC_LEN);
    if (ret != 0) {
        devdrv_drv_err("memset_s error, ret = %d\n", ret);
        return;
    }

    desc.flags = 0;
    desc.tfm = crypto_alloc_hash("sha256", 0, 0);
    if (IS_ERR(desc.tfm)) {
        devdrv_drv_err("crypto_alloc_hash failed.\n");
        return;
    }

    sg_init_one(&sg, buf, len);
    (void)crypto_hash_init(&desc);
    (void)crypto_hash_update(&desc, &sg, len);
    (void)crypto_hash_final(&desc, check);
    crypto_free_hash(desc.tfm);

    for (i = 0; i < DEVDRV_TS_BIN_CHEKC_LEN; i++) {
        tlen = snprintf_s(&str[(long)i * 2], sizeof(str) / 2, 3, "%02x", check[i]);
        if (tlen < 0) {
            devdrv_drv_err("devdrv_calculate_check_func:%02x snprintf_s fail\n", check[i]);
            return;
        }
    }

    str[i * 2] = '\0'; //lint !e679
    devdrv_drv_info("generate sha256 result: %s\n", str);
}
#endif

STATIC void devdrv_calculate_check(const u8 *buf, u32 len, struct devdrv_check_sum *out)
{
    int ret;
    ret = memset_s(out, sizeof(struct devdrv_check_sum), 0, sizeof(struct devdrv_check_sum));
    if (ret != 0) {
        devdrv_drv_err("memset_s error, ret = %d\n", ret);
        return;
    }

    devdrv_calculate_check_func(buf, len, (u8 *)out);
}

STATIC void devdrv_get_check(u8 *buf, struct devdrv_check_sum *out)
{
    char str[128];
    int i;
    int tlen;

    for (i = 0; i < DEVDRV_TS_BIN_CHEKC_LEN; i++) {
        out->check[i] = *(u8 *)buf;
        buf += sizeof(u8);
        tlen = snprintf_s(&str[(long)i * 2], sizeof(str) / 2, 3, "%02x", out->check[i]);
        if (tlen < 0) {
            devdrv_drv_err("devdrv_get_check:%02x snprintf_s fail\n", out->check[i]);
            return;
        }
    }

    str[i * 2] = '\0'; //lint !e679
    devdrv_drv_info("get sha256 from file: %s\n", str);
}

STATIC int devdrv_is_segment_invalid(void)
{
    u32 segment_end;
    u32 i;

    if (dev_ts_bin_info.segment_num >= DEVDRV_TS_BIN_MAX_SEGMENT_NUM) {
        devdrv_drv_err("segment num:%d is out of range.\n", dev_ts_bin_info.segment_num);
        return 1;
    }

    for (i = 0; i < dev_ts_bin_info.segment_num; i++) {
        segment_end = dev_ts_bin_info.segment[i].offset + dev_ts_bin_info.segment[i].len;
        if (segment_end > dev_ts_bin_info.fw_data_len) {
            devdrv_drv_err("segment %d is invalid, offset: %d, len: %d, while firmware len: %d.\n", i,
                dev_ts_bin_info.segment[i].offset, dev_ts_bin_info.segment[i].len, dev_ts_bin_info.fw_data_len);
            return 1;
        }
    }

    return 0;
}

STATIC void devdrv_get_segment_info(u8 *buf, u32 segment_num, int buff_len)
{
    u32 i;

    for (i = 0; i < segment_num; i++) {
        devdrv_drv_info("segment id: %d: .\n", i);
        dev_ts_bin_info.segment[i].offset = *(u32 *)buf;
        buf += sizeof(u32);
        dev_ts_bin_info.segment[i].len = *(u32 *)buf;
        buf += sizeof(u32);
        devdrv_get_check(buf, &dev_ts_bin_info.segment[i].segment_check);
        buf += sizeof(struct devdrv_check_sum);

        devdrv_drv_info("segment len: %d.\n", dev_ts_bin_info.segment[i].len);
    }
}

STATIC int devdrv_check_not_equal(struct devdrv_check_sum *left, struct devdrv_check_sum *right)
{
    int i;

    for (i = 0; i < DEVDRV_TS_BIN_CHEKC_LEN; i++) {
        if (left->check[i] != right->check[i])
            return 1;
    }

    return 0;
}

STATIC int devdrv_check_segments(u8 *ts_bin, u32 segment_num)
{
    struct devdrv_check_sum check_sum;
    u8 *segment = NULL;
    u32 i;

    for (i = 0; i < segment_num; i++) {
        segment = ts_bin + dev_ts_bin_info.segment[i].offset;
        devdrv_drv_info("segment id: %d: .\n", i);
        devdrv_calculate_check(segment, dev_ts_bin_info.segment[i].len, &check_sum);
        if (devdrv_check_not_equal(&check_sum, &dev_ts_bin_info.segment[i].segment_check)) {
            devdrv_drv_err("TS firmware code segments is invalid, index: %d.\n", i);
            return -1;
        }
    }

    return 0;
}

STATIC int devdrv_is_ts_bin_segment_num_invalid(u32 segment_num)
{
    return (segment_num > DEVDRV_TS_BIN_MAX_SEGMENT_NUM);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)
ssize_t read_file(struct file *fp, void *firmware_dst_addr, loff_t fsize, loff_t pos, int buf_len)
{
    ssize_t ret = 0;
    if (fp != NULL && firmware_dst_addr != NULL)
        ret = kernel_read(fp, firmware_dst_addr, fsize, &pos);
    return ret;
}
#else
ssize_t read_file(struct file *fp, void *firmware_dst_addr, loff_t fsize, loff_t pos, int buf_len)
{
    mm_segment_t old_fs;
    ssize_t ret = 0;
    if (fp != NULL && firmware_dst_addr != NULL) {
        old_fs = get_fs();
        set_fs((mm_segment_t)KERNEL_DS); /*lint !e501 */
        ret = vfs_read(fp, firmware_dst_addr, fsize, &pos);
        set_fs(old_fs);
    }
    return ret;
}
#endif
STATIC int devdrv_judge_bin_validity_early_input_check(struct file *fp, loff_t *fsize, loff_t *pos, u32 *ts_check_file)
{
    if (fp == NULL || fsize == NULL || pos == NULL || ts_check_file == NULL) {
        devdrv_drv_err("input error %pK %pK %pK %pK.\n", fp, fsize, pos, ts_check_file);
        return -1;
    }
    return 0;
}
int devdrv_judge_bin_validity_early(struct file *fp, loff_t *fsize, loff_t *pos, u32 *ts_check_file)
{
    struct devdrv_check_sum check;
    u32 segment_num;
    int ret;

    loff_t read_pos;
    ssize_t read_num;
    u32 fw_len_allign;
    u32 fw_len;
    u8 buf[BUFF_LEN] = {0};

    read_pos = 0;
    ret = devdrv_judge_bin_validity_early_input_check(fp, fsize, pos, ts_check_file);
    if (ret != 0) {
        return -1;
    }

    read_num = read_file(fp, buf, sizeof(u32) * 2, read_pos, BUFF_LEN);
    if (read_num < (ssize_t)(sizeof(u32) * 2)) {
        devdrv_drv_err("vfs_read failed.\n");
        return -1;
    }

    if (buf[0] != 'T' || buf[1] != 'S' || buf[2] != 'F' || buf[3] != 'W') {
        devdrv_drv_info("firmware is not a 'TSFW'.\n");
        ts_check_file = 0;
        dev_ts_bin_info.ts_check_file = 0;
        return 0;
    }

    devdrv_drv_info("firmware is a TS firmware.\n");
    fw_len = *(u32 *)(buf + sizeof(u32));

    if (fw_len == 0 || fw_len >= (*fsize - sizeof(u32) * 2)) {
        devdrv_drv_err("TS firmware length is invalid: %d.\n", fw_len);
        return -1;
    }

    devdrv_drv_info("TS firmware length: %d.\n", fw_len);

    dev_ts_bin_info.ts_check_file = 1;
    *ts_check_file = 1;
    *fsize = fw_len;
    *pos = 8;

    fw_len_allign = 4 - (fw_len % 4);
    fw_len_allign = (fw_len % 4 > 0) ? (fw_len + fw_len_allign) : fw_len;

    devdrv_drv_info("fw_len_allign length: %d.\n", fw_len_allign);

    /* get segment info */
    read_pos = 8 + fw_len_allign;

    read_num = read_file(fp, buf, sizeof(struct devdrv_check_sum) + sizeof(u32) * 2, read_pos, BUFF_LEN);
    if (read_num < (int)(sizeof(struct devdrv_check_sum) + sizeof(u32) * 2)) {
        devdrv_drv_err("vfs_read failed.\n");
        return -1;
    }

    segment_num = *(u32 *)buf;
    if (devdrv_is_ts_bin_segment_num_invalid(segment_num)) {
        devdrv_drv_err("TS firmware is not valid: too much segments: %d.\n", segment_num);
        return -1;
    }

    devdrv_drv_info("segment number: %d.\n", segment_num);

    devdrv_get_check((buf + 2 * sizeof(u32)), &check);

    read_pos = 8 + fw_len_allign + 2 * sizeof(u32) + sizeof(struct devdrv_check_sum);

    read_num =
        read_file(fp, buf, (long)(unsigned)sizeof(struct devdrv_ts_bin_segment) * segment_num, read_pos, BUFF_LEN);
    if (read_num < (int)(sizeof(struct devdrv_ts_bin_segment) * segment_num)) {
        devdrv_drv_err("vfs_read failed.\n");
        return -1;
    }

    ret = memcpy_s(&dev_ts_bin_info.fw_data_check, sizeof(struct devdrv_check_sum), &check,
        sizeof(struct devdrv_check_sum));
    if (ret != 0) {
        devdrv_drv_err("memcpy_s error, ret = %d\n", ret);
        return -EINVAL;
    }

    dev_ts_bin_info.fw_data_len = fw_len;
    dev_ts_bin_info.segment_num = segment_num;
    devdrv_get_segment_info(buf, segment_num, BUFF_LEN);

    if (devdrv_is_segment_invalid()) {
        devdrv_drv_err("devdrv_is_segment_invalid return error.\n");
        return -1;
    }

    return 0;
}

int devdrv_judge_bin_validity(u32 type, u32 ts_check_file, void *ts_bin, u32 check_fw_data)
{
    struct devdrv_check_sum check_sum_origin;

    if (type != DEVDRV_FW_TYPE_TS) {
        devdrv_drv_info("it's not a TS file.\n");
        return 0;
    }

    if (!ts_check_file) {
        devdrv_drv_info("it's not a TS check file.\n");
        return 0;
    }

    if (ts_bin == NULL) {
        devdrv_drv_err("invalid handle.\n");
        return -1;
    }

    if (devdrv_check_segments((u8 *)ts_bin, dev_ts_bin_info.segment_num)) {
        devdrv_drv_err("TS firmware is not valid: check code segments failed.\n");
        return -1;
    }

    if (!check_fw_data)
        return 0;

    devdrv_calculate_check((u8 *)ts_bin, dev_ts_bin_info.fw_data_len, &check_sum_origin);
    if (devdrv_check_not_equal(&check_sum_origin, &dev_ts_bin_info.fw_data_check)) {
        devdrv_drv_err("TS firmware is not valid: invalid fw data check sum.\n");
        return -1;
    }
    return 0;
}

int devdrv_is_ts_check_file(void)
{
    return dev_ts_bin_info.ts_check_file;
}
