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
 * Create: 2022-8-15
 */
#ifndef TRS_RES_ID_DEF_H
#define TRS_RES_ID_DEF_H

#define TRS_HW_TYPE_TSCPU 0
#define TRS_HW_TYPE_STARS 1

enum trs_id_type {
    TRS_STREAM = 0,
    TRS_EVENT,
    TRS_NOTIFY,
    TRS_MODEL,
    TRS_CMO,
    TRS_HW_SQ = 5,
    TRS_HW_CQ,
    TRS_SW_SQ = 7,
    TRS_SW_CQ,
    TRS_CB_SQ,
    TRS_CB_CQ,
    TRS_LOGIC_CQ = 11,
    TRS_CDQ,
    TRS_MAX_ID_TYPE,
};

#define TRS_CORE_MAX_ID_TYPE (TRS_LOGIC_CQ + 1)
#endif

