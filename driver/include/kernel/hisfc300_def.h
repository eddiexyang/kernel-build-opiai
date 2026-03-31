/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
 *
 * Compatibility shim for legacy hisfc300 users. The original external header
 * is not bundled in the target tree, but restored modules still depend on its
 * partition-info shape and flash access declarations.
 */
#ifndef HISFC300_DEF_H
#define HISFC300_DEF_H

#include <linux/types.h>

#define HISFC_PART_NUM_MAX 128
#define HISFC_MTD_NAME_MAX_LEN 64

struct hisfc_mtd_info_st {
    unsigned char mtdName[HISFC_MTD_NAME_MAX_LEN];
};

/*
 * Legacy flash users expect the Reserve_1 partition name from the vendor
 * hisfc300 header. The same value is present in the source snapshot's
 * upgrade adapt headers, so restore it here for source-equivalent users.
 */
#define PART_NAME_RESERVE_1 "Reserve_1"

int hisi_sfc_ctl_flash_read(const unsigned char *part_name, unsigned int offset, size_t len, size_t *ret_len,
    unsigned char *buf);
int hisi_sfc_ctl_flash_write(const unsigned char *part_name, unsigned int offset, size_t len, size_t *ret_len,
    const unsigned char *buf);
int hisi_sfc_ctl_flash_erase(const unsigned char *part_name, unsigned int offset, size_t len);
int hisi_sfc_ctl_flash_read_mtd_info(unsigned int read_count, unsigned int *total_count,
    struct hisfc_mtd_info_st *mtd_part);

#endif
