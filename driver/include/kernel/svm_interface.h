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

#ifndef __SVM_INTERFACE_H__
#define __SVM_INTERFACE_H__

#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/mm.h>

int devmm_get_pages_list(struct mm_struct *mm, u64 va, u64 num, struct page **pages);
#endif

struct devmm_set_convert_len_para {
    unsigned long long total_convert_len;
};

struct devmm_get_convert_len_para {
    unsigned long long total_convert_len;
};

int halMemGet(unsigned long long addr, unsigned long long size);
int halMemPut(unsigned long long addr, unsigned long long size);

#endif

