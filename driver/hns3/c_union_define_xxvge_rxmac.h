/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: xxvge_rxmac
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __C_UNION_DEFINE_XXVGE_RXMAC_H__
#define __C_UNION_DEFINE_XXVGE_RXMAC_H__

/* define the union u_xxvge_rxmac_enable */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rx_enable : 1; /* [0] */
        unsigned int reserved : 31; /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_rxmac_enable;

/* define the union u_xxvge_rxmac_lane_1ui_dly */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int bit_delay : 23; /* [22:0] */
        unsigned int reserved : 9;   /* [31:23] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_rxmac_lane_1ui_dly;

/* define the union u_xxvge_rxmac_replicate */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int factor : 12;       /* [11:0] */
        unsigned int replicate_sel : 1; /* [12] */
        unsigned int reserved : 19;     /* [31:13] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_rxmac_replicate;

/* define the union u_xxvge_rxmac_control */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int link_interrupt_en : 1;       /* [0] */
        unsigned int fault_det_sel : 1; /* [1] */
        unsigned int preempt_smd_v_r_bypass_mode : 1; /* [2] */
        unsigned int rsvd1 : 1; /* [3] */
        unsigned int min_ipg_clr : 1; /* [4] */
        unsigned int ipg_chk_en : 1; /* [5] */
        unsigned int rx_preemp_en : 1; /* [6] */
        unsigned int rx_preemp_keeps_en : 1; /* [7] */
        unsigned int rx_lpi_en : 1; /* [8] */
        unsigned int rx_adapt_select_en : 1; /* [9] */
        unsigned int rx_pad_strip_en : 1; /* [10] */
        unsigned int rsvd2 : 1; /* [11] */
        unsigned int rx_rgmii_crs_ctr : 1; /* [12] */
        unsigned int rgmii_linkup_ctr : 1; /* [13] */
        unsigned int rsvd3 : 2; /* [15:14] */
        unsigned int rx_fcs_en : 1; /* [16] */
        unsigned int rx_fcs_strip_en : 1; /* [17] */
        unsigned int rsvd4 : 1; /* [18] */
        unsigned int rx_preamble_trans_en : 1; /* [19] */
        unsigned int rx_under_min_err_en : 1; /* [20] */
        unsigned int rx_over_size_err_en : 1; /* [21] */
        unsigned int rsvd5 : 1; /* [22] */
        unsigned int rx_otn_orderset_en : 1; /* [23] */
        unsigned int rx_1588_en : 1; /* [24] */
        unsigned int rx_1731_en : 1; /* [25] */
        unsigned int rsvd6 : 1; /* [26] */
        unsigned int rx_preamble_err_chk_en : 1; /* [27] */
        unsigned int rsvd7 : 1; /* [28] */
        unsigned int rx_preamble_match_mode : 1; /* [29] */
        unsigned int rx_sfd_match_mode : 1; /* [30] */
        unsigned int reserved : 1;     /* [31] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_rxmac_control;

typedef union {
    /* define the struct bits */
    struct {
        unsigned int lpi_thd : 8;       /* [7:0] */
        unsigned int seq_cnt : 3;       /* [10:8] */
        unsigned int reserved1 : 1;       /* [11] */
        unsigned int rx_ipg : 5;       /* [16:12] */
        unsigned int rx_err_ctr : 1; /* [17] */
        unsigned int reserved : 14;     /* [31:18] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_rxmac_control1;

#endif  // __C_UNION_DEFINE_XXVGE_RXMAC_H__
