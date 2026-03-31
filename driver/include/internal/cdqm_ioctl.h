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
* Create: 2022-7-9
*/

#ifndef CDQM_IOCTL_H
#define CDQM_IOCTL_H
#include <asm/ioctl.h>

#include "drv_type.h"

#ifdef EMU_ST
#define THREAD__  __thread
#else
#define THREAD__
#endif

#define CDQ_INSTANCE(devid, tsid, qid) (((devid) << 24) | ((tsid) << 16) | (qid))

#define CDQ_NAME_LEN 64

#define CDQM_ID_MAGIC 'C'
#define CDQM_COMMAND          _IOWR(CDQM_ID_MAGIC, 1, struct cdqm_ioctl_arg)

#define CDQM_MAX_CMD        15

#define DAVINCI_INTF_MODULE_CDQM "CDQM"

struct tsdrv_cdqm_create_para {
    char name[CDQ_NAME_LEN];
    u32 tsid;
    u32 batch_num;
    u32 batch_size;
    u64 mem_addr;
    u32 ssid;
    u32 qid; /* output */
};

struct tsdrv_cdqm_destroy_para {
    u32 tsid;
    u32 qid;
};

struct tsdrv_cdqm_query_para {
    u32 host_pid;
    char name[CDQ_NAME_LEN];
    u32 tsid; /* output */
    u32 qid; /* output */
};

struct tsdrv_cdqm_alloc_batch_para {
    u32 tsid;
    u32 qid;
    u32 timeout;
    u32 batch_id; /* output */
};

struct tsdrv_cdqm_get_ready_batch_para {
    u32 tsid;
    u32 qid;
    u32 batch_size; /* output */
    u64 batch_addr; /* output */
};

struct tsdrv_cdqm_free_batch_para {
    u32 tsid;
    u32 qid;
    u32 batch_addr;
};

struct tsdrv_cdqm_batch_abnormal {
    u32 tsid;
    u32 qid;
};

typedef enum {
    CDQM_CREATE_CDQ = 1,
    CDQM_DESTROY_CDQ,
    CDQM_QUERY_CDQ,
    CDQM_ALLOC_BATCH,
    CDQM_GET_READY_BATCH,
    CDQM_FREE_BATCH,
    CDQM_BATCH_ABNORMAL,
    CDQM_OPT_TYPE_MAX
} tsdrv_cdqm_opt;

struct tsdrv_cdqm_args {
    tsdrv_cdqm_opt type;
    union {
        struct tsdrv_cdqm_create_para create_para;
        struct tsdrv_cdqm_alloc_batch_para alloc_para;
        struct tsdrv_cdqm_destroy_para destroy_para;
        struct tsdrv_cdqm_query_para query_para;
        struct tsdrv_cdqm_get_ready_batch_para rdy_para;
        struct tsdrv_cdqm_free_batch_para free_para;
        struct tsdrv_cdqm_batch_abnormal timeout_para;
    };
};

struct cdqm_ioctl_arg {
    u32 tsid;

    struct tsdrv_cdqm_args cdqm_para;
};

#endif

