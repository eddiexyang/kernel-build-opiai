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
 * Create: 2022-12-14
 */

#ifndef __VIRTMNGDEV_ELEMENT_H__
#define __VIRTMNGDEV_ELEMENT_H__

typedef struct vmngd_element {
    unsigned int attr;              /* including if use bitmap, create, update attribute */
    unsigned int num_per_bit;       /* when use bitmap, the number of each bit */
    unsigned int bitnum;            /* bitmap vailed bit num */
    unsigned int num;               /* resource number */
    unsigned long bitmap;           /* resource bitmap */
} vmngd_element_t;

int vmngd_get_element_bitnum(vmngd_element_t *self);
void vmngd_element_set(vmngd_element_t *self, const unsigned long bitmap);
void vmngd_element_copy(vmngd_element_t *self, const vmngd_element_t *input);
void vmngd_element_add(vmngd_element_t *self, const vmngd_element_t *input);
void vmngd_element_sub(vmngd_element_t *self, const vmngd_element_t *input);
void vmngd_element_clear(vmngd_element_t *self);
int vmngd_element_alloc(vmngd_element_t *self, const vmngd_element_t *from, unsigned long long num);

#endif