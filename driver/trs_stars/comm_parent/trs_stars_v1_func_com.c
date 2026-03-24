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
 * Create: 2023-1-11
 */
#include "trs_stars_v1_func_com.h"
#include "stars_event_tbl_ns.h"
int trs_stars_func_event_id_reset(struct trs_id_inst *inst, u32 id)
{
    trs_stars_set_event_tbl_flag(inst, id, 0);
    trs_debug("Reset event id. (devid=%u; event_id=%u)\n", inst->devid, id);
    return 0;
}

int trs_stars_func_event_id_check_and_reset(struct trs_id_inst *inst, u32 id)
{
    int status;

    status = trs_stars_get_event_tbl_flag(inst, id);
    if (status != 0) { /* 0: id status is idle */
        trs_warn("Id status abnormal. (devid=%u; tsid=%u; id=%u; status=%u)\n",
            inst->devid, inst->tsid, id, status);
        return trs_stars_func_event_id_reset(inst, id);
    }

    return 0;
}
