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

#ifndef TRS_CMD_H
#define TRS_CMD_H

#include <linux/ioctl.h>
#include <linux/types.h>

#include "ascend_hal_define.h"
#include "trs_res_id_def.h"

#define TRS_CTRL_USE_SQE_NUM 2

enum trs_uio_addr_type {
    TRS_UIO_SHR_INFO,
    TRS_UIO_HEAD,
    TRS_UIO_TAIL,
    TRS_UIO_DB,
    TRS_UIO_HEAD_REG,
    TRS_UIO_TAIL_REG,
    TRS_UIO_ADDR_MAX,
};

static inline bool trs_is_ro_addr(int addr_type)
{
    return ((addr_type == TRS_UIO_HEAD_REG) || (addr_type == TRS_UIO_TAIL_REG));
}

struct trs_uio_info {
    unsigned long sq_que_addr; /* input/output */
    unsigned long sq_ctrl_addr[TRS_UIO_ADDR_MAX]; /* input/output */
    uint8_t soft_que_flag; /* output: 1 use soft que, 2 use hw que */
    uint8_t uio_flag; /* output: 0 not use uio, 1 use uio */
};

struct trs_alloc_para { /* point to struct halSqCqInputInfo res, size <= 24 bytes */
    struct trs_uio_info *uio_info;
};

static inline struct trs_alloc_para *get_alloc_para_addr(struct halSqCqInputInfo *in)
{
    return (struct trs_alloc_para *)&in->res[1]; /* 1 for sq_addr 8 byte align */
}

struct trs_sq_shr_info {
    uint32_t cqe_status;
    uint8_t rsv[1020]; /* 1020 for reduce cache miss, kernel write cqe status, user write stat */
    unsigned long long send_full;
    unsigned long long send_ok;
};

struct trs_res_id_para {
    unsigned int tsid;
    int res_type;
    unsigned int id;
    unsigned int para;
    int prop;
    uint32_t value[2]; /* for id addr */
};

struct trs_ssid_query_para {
    int ssid;
};

struct trs_hw_info_query_para {
    int hw_type;
    int tsnum;
};

#define TRS_RES_ID_ALLOC            _IOWR('X', 0, struct trs_res_id_para)
#define TRS_RES_ID_FREE             _IOW('X', 1, struct trs_res_id_para)
#define TRS_RES_ID_ENABLE           _IOW('X', 2, struct trs_res_id_para)
#define TRS_RES_ID_DISABLE          _IOW('X', 3, struct trs_res_id_para)
#define TRS_RES_ID_NUM_QUERY        _IOWR('X', 4, struct trs_res_id_para)
#define TRS_RES_ID_MAX_QUERY        _IOWR('X', 5, struct trs_res_id_para)
#define TRS_RES_ID_USED_NUM_QUERY   _IOWR('X', 6, struct trs_res_id_para)
#define TRS_RES_ID_AVAIL_NUM_QUERY  _IOWR('X', 7, struct trs_res_id_para)
#define TRS_RES_ID_REG_OFFSET_QUERY _IOWR('X', 8, struct trs_res_id_para)
#define TRS_RES_ID_REG_SIZE_QUERY   _IOWR('X', 9, struct trs_res_id_para)
#define TRS_RES_ID_CFG              _IOW('X', 10, struct trs_res_id_para)

#define TRS_SSID_QUERY              _IOWR('X', 11, struct trs_ssid_query_para)
#define TRS_HW_INFO_QUERY           _IOWR('X', 12, struct trs_hw_info_query_para)

#define TRS_SQCQ_ALLOC              _IOWR('X', 15, struct halSqCqInputInfo)
#define TRS_SQCQ_FREE               _IOW('X', 16, struct halSqCqFreeInfo)
#define TRS_SQCQ_CONFIG             _IOW('X', 17, struct halSqCqConfigInfo)
#define TRS_SQCQ_QUERY              _IOWR('X', 18, struct halSqCqQueryInfo)
#define TRS_SQCQ_SEND               _IOW('X', 19, struct halTaskSendInfo)
#define TRS_SQCQ_RECV               _IOWR('X', 20, struct halReportRecvInfo)

#define TRS_MAX_CMD        24

#endif

