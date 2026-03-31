/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Create: 2023-01-07
 */

#include "virtmngdev_res_common.h"

int vmngd_alloc_aicpu_for_vf(vmngd_element_t *vf_aicpu, vmngd_element_t *remain, const u32 alloc_aicpu_num);
void vmngd_release_aicpu(vmngd_element_t *vf_aicpu, vmngd_element_t *remain);
void vmngd_resource_get_aicpu_info(u32 dev_id, vmngd_element_t *aicpu);