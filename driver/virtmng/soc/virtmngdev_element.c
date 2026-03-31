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

#include <linux/bitmap.h>
#include <linux/errno.h>
#include "virtmngdev_element.h"

int vmngd_get_element_bitnum(vmngd_element_t *self)
{
    return bitmap_weight(&self->bitmap, self->bitnum);
}

void vmngd_element_set(vmngd_element_t *self, const unsigned long bitmap)
{
    self->bitmap = bitmap;
    self->num = bitmap_weight(&self->bitmap, self->bitnum) * self->num_per_bit;
}

void vmngd_element_copy(vmngd_element_t *self, const vmngd_element_t *input)
{
    self->bitmap = input->bitmap;
    self->attr = input->attr;
    self->num_per_bit = input->num_per_bit;
    self->bitnum = input->bitnum;
    self->num = bitmap_weight(&self->bitmap, self->bitnum) * self->num_per_bit;
}

void vmngd_element_add(vmngd_element_t *self, const vmngd_element_t *input)
{
    self->bitmap |= input->bitmap;
    self->num = bitmap_weight(&self->bitmap, self->bitnum) * self->num_per_bit;
}

void vmngd_element_sub(vmngd_element_t *self, const vmngd_element_t *input)
{
    self->bitmap &= (~input->bitmap);
    self->num = bitmap_weight(&self->bitmap, self->bitnum) * self->num_per_bit;
}

void vmngd_element_clear(vmngd_element_t *self)
{
    self->bitmap = 0;
    self->num = 0;
}

int vmngd_element_alloc(vmngd_element_t *self, const vmngd_element_t *from, unsigned long long num)
{
    int idx = -1;
    int nbits;
    int i;

    if (from->num_per_bit == 0) {
        return -EINVAL;
    }
    if (num % from->num_per_bit != 0) {
        return -EINVAL;
    }
    nbits = num / from->num_per_bit;
    if (bitmap_weight(&from->bitmap, from->bitnum) < nbits) {
        return -EINVAL;
    }

    for (i = 0; i < nbits; ++i) {
        idx = find_next_bit(&from->bitmap, from->bitnum, idx + 1);
        bitmap_set(&self->bitmap, idx, 1);
    }
    self->attr = from->attr;
    self->num_per_bit = from->num_per_bit;
    self->bitnum = from->bitnum;
    self->num = bitmap_weight(&self->bitmap, self->bitnum) * self->num_per_bit;
    return 0;
}

