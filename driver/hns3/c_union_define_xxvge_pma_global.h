/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: xxvge_pma_global
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __C_UNION_DEFINE_XXVGE_PMA_GLOBAL_H__
#define __C_UNION_DEFINE_XXVGE_PMA_GLOBAL_H__

/* define the union u_xxvge_pma_calendar_table */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int content0 : 3;  /* [2:0] */
        unsigned int reserved : 1;  /* [3] */
        unsigned int content1 : 3;  /* [6:4] */
        unsigned int reserved1 : 1; /* [7] */
        unsigned int content2 : 3;  /* [10:8] */
        unsigned int reserved2 : 1; /* [11] */
        unsigned int content3 : 3;  /* [14:12] */
        unsigned int reserved3 : 1; /* [15] */
        unsigned int content4 : 3;  /* [18:16] */
        unsigned int reserved4 : 1; /* [19] */
        unsigned int content5 : 3;  /* [22:20] */
        unsigned int reserved5 : 1; /* [23] */
        unsigned int content6 : 3;  /* [26:24] */
        unsigned int reserved6 : 1; /* [27] */
        unsigned int content7 : 3;  /* [30:28] */
        unsigned int reserved7 : 1; /* [31] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_pma_calendar_table;

#endif  // __C_UNION_DEFINE_XXVGE_PMA_GLOBAL_H__