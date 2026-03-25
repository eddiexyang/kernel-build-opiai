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
 * Create: 2023-3-1
 */
#include <linux/io.h>

#include "soc_res.h"
#include "stars_event_table_ns_c_union_define.h"
#include "trs_stars_comm.h"

static volatile StarsEventTableNsRegsType *stars_event_table_ns_all_reg[TRS_DEV_MAX_NUM][TRS_TS_MAX_NUM];
spinlock_t event_table_all_reg_lock[TRS_DEV_MAX_NUM][TRS_TS_MAX_NUM];

int trs_init_event_tbl_ns_base_addr(struct trs_id_inst *inst)
{
	struct soc_reg_base_info io_base;
	struct res_inst_info res_inst;
	void *base_va;
	int ret;

	soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
	ret = soc_resmng_get_reg_base(&res_inst, "TS_STARS_EVENT_TBL_NS_REG", &io_base);
	if (ret != 0) {
		trs_err("Failed to get event status reg base. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
		return ret;
	}

	base_va = ioremap(io_base.io_base, io_base.io_base_size);
	if (base_va == NULL) {
		trs_err("Failed to ioremap. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
		return -ENOMEM;
	}

	spin_lock_init(&event_table_all_reg_lock[inst->devid][inst->tsid]);
	stars_event_table_ns_all_reg[inst->devid][inst->tsid] = base_va;

	return 0;
}

void trs_uninit_event_tbl_ns_base_addr(struct trs_id_inst *inst)
{
	void *base_va = stars_event_table_ns_all_reg[inst->devid][inst->tsid];

	spin_lock(&event_table_all_reg_lock[inst->devid][inst->tsid]);
	if (stars_event_table_ns_all_reg[inst->devid][inst->tsid] != NULL) {
		stars_event_table_ns_all_reg[inst->devid][inst->tsid] = NULL;
	}
	spin_unlock(&event_table_all_reg_lock[inst->devid][inst->tsid]);

	iounmap(base_va);
}

static StarsEventTable *trs_get_event_table(struct trs_id_inst *inst, u32 id)
{
	u32 group_id = id / STARS_TABLE_EVENT_NUM;
	u32 offset = id % STARS_TABLE_EVENT_NUM;
	StarsEventGroupTableInfo *group_info = NULL;
	StarsEventTableNsRegsType *tbl_info = stars_event_table_ns_all_reg[inst->devid][inst->tsid];

	if (tbl_info == NULL) {
		return NULL;
	}

	group_info = (StarsEventGroupTableInfo *)&(tbl_info->StarsEventGroupTable[group_id]);
	return &(group_info->starsEventTable[offset]);
}

int trs_stars_get_event_tbl_flag(struct trs_id_inst *inst, u32 id)
{
	int tbl_flag = 0;

	spin_lock(&event_table_all_reg_lock[inst->devid][inst->tsid]);
	if (trs_get_event_table(inst, id) != NULL) {
		tbl_flag = trs_get_event_table(inst, id)->bits.eventTableFlag;
	}
	spin_unlock(&event_table_all_reg_lock[inst->devid][inst->tsid]);

	return tbl_flag;
}

void trs_stars_set_event_tbl_flag(struct trs_id_inst *inst, u32 id, u32 val)
{
	spin_lock(&event_table_all_reg_lock[inst->devid][inst->tsid]);
	if (trs_get_event_table(inst, id) != NULL) {
		trs_get_event_table(inst, id)->bits.eventTableFlag = val;
	}
	spin_unlock(&event_table_all_reg_lock[inst->devid][inst->tsid]);
}
