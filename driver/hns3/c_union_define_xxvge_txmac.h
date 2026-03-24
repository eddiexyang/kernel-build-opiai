/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: xxvge_txmac
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __C_UNION_DEFINE_XXVGE_TXMAC_H__
#define __C_UNION_DEFINE_XXVGE_TXMAC_H__

/* define the union u_xxvge_txmac_base_freq_h */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int base_freq_h : 8; /* [7:0] */
        unsigned int reserved : 24;   /* [31:8] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_txmac_base_freq_h;

/* define the union u_xxvge_txmac_enable */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int tx_enable : 1; /* [0] */
        unsigned int reserved : 31; /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_txmac_enable;

/* define the union u_xxvge_txmac_lf_rf_control */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int unidir_en : 1;                 /* [0] */
        unsigned int rf_tx_en : 1;                  /* [1] */
        unsigned int lf_rf_insert : 2;              /* [3:2] */
        unsigned int tx_eee : 1;                    /* [4] */
        unsigned int rsvd : 3;                      /* [7:5] */
        unsigned int fault_mib_stop : 1;            /* [8] */
        unsigned int fault_insert_after_rep_en : 1; /* [9] */
        unsigned int rsvd1 : 2;                     /* [11:10] */
        unsigned int app_unidir_en : 1;             /* [12] */
        unsigned int app_rf_tx_en : 1;              /* [13] */
        unsigned int reserved : 18;                 /* [31:14] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_txmac_lf_rf_control;

/* define the union u_xxvge_txmac_lane_1ui_dly */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int bit_delay : 23; /* [22:0] */
        unsigned int reserved : 9;   /* [31:23] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_txmac_lane_1ui_dly;

/* define the union u_xxvge_txmac_replicate */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int factor : 12;       /* [11:0] */
        unsigned int replicate_sel : 1; /* [12] */
        unsigned int reserved : 19;     /* [31:13] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_txmac_replicate;

#endif  // __C_UNION_DEFINE_XXVGE_TXMAC_H__