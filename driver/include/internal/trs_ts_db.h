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
 * Create: 2022-8-15
 */
#ifndef TRS_TS_DB_H
#define TRS_TS_DB_H

#include <linux/types.h>
#include <linux/compiler.h>

#include "trs_pub_def.h"
#include "trs_chip_def_comm.h"

int trs_ts_db_init(struct trs_id_inst *inst, int type, u32 start, u32 end);
void trs_ts_db_uninit(struct trs_id_inst *inst, int type);
int trs_ts_db_cfg(struct trs_id_inst *inst, phys_addr_t paddr, size_t size, u32 stride);
void trs_ts_db_decfg(struct trs_id_inst *inst);

int trs_ring_ts_db(struct trs_id_inst *inst, int type, u32 offset, u32 val);
int trs_get_ts_db_val(struct trs_id_inst *inst, int type, u32 offset, u32 *val);
int trs_get_ts_db_paddr(struct trs_id_inst *inst, int type, u32 offset, u64 *paddr);

#endif /* TRS_TS_DB_H */
