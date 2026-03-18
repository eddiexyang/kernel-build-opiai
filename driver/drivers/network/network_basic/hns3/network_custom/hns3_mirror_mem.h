/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Create: 2023-1-19
 */
#ifndef _HNS3_MIRROR_MEM_H
#define _HNS3_MIRROR_MEM_H

#ifdef MIRROR_MEMORY_SUPPORT
#define MAX_CPU_NUM 8
#define MAX_NODE_NUM 64
#define MAX_NETWORK_CTRL_MEM_PAGE_CNT 16384

typedef int (*dbl_get_ctrl_nid_fun)(u32 devid, int nids[], int num);
typedef int (*dbl_get_ai_nid_fun)(u32 devid, int nids[], int num);
#endif

#endif