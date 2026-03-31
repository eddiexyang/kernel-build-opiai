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
 * Create: 2023-2-1
 */

#ifndef TRS_MIA_INFO_TS_H
#define TRS_MIA_INFO_TS_H

#include <linux/types.h>

#include "virtmng_interface.h"

int trs_mia_notice_ts(struct trs_id_inst *ts_inst, u32 vfid, struct vmng_vdev_ctrl *cfg, u32 op);

#endif /* TRS_MIA_INFO_TS_H */

