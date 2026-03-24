/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: xxvge_global
 * Create: 2021-12-28
 */

#ifndef __C_UNION_DEFINE_XXVGE_GLOBAL_H__
#define __C_UNION_DEFINE_XXVGE_GLOBAL_H__

/* define the union u_xxvge_work_mode */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int tx_mode : 5;         /* [4:0] */
        unsigned int reserved : 3;        /* [7:5] */
        unsigned int rx_mode : 5;         /* [12:8] */
        unsigned int reserved1 : 2;       /* [14:13] */
        unsigned int da_pad_strip_en : 1; /* [15] */
        unsigned int tx_qsgmii_en0 : 1;   /* [16] */
        unsigned int rx_qsgmii_en0 : 1;   /* [17] */
        unsigned int tx_qsgmii_en4 : 1;   /* [18] */
        unsigned int rx_qsgmii_en4 : 1;   /* [19] */
        unsigned int tx_sgmii_4p_en0 : 1; /* [20] */
        unsigned int rx_sgmii_4p_en0 : 1; /* [21] */
        unsigned int tx_sgmii_4p_en4 : 1; /* [22] */
        unsigned int rx_sgmii_4p_en4 : 1; /* [23] */
        unsigned int tx_rxaui_en : 1;     /* [24] */
        unsigned int rx_rxaui_en : 1;     /* [25] */
        unsigned int tx_wis_en : 1;       /* [26] */
        unsigned int rx_wis_en : 1;       /* [27] */
        unsigned int tx_brfec_en : 1;     /* [28] */
        unsigned int rx_brfec_en : 1;     /* [29] */
        unsigned int tx_rsfec_en : 1;     /* [30] */
        unsigned int rx_rsfec_en : 1;     /* [31] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_work_mode;

/* define the union u_xxvge_work_freq_h */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int work_freq_h : 9; /* [8:0] */
        unsigned int reserved : 23;   /* [31:9] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_work_freq_h;

/* define the union u_xxvge_tx_calen_depth */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int depth : 4;     /* [3:0] */
        unsigned int opcode : 8;    /* [11:4] */
        unsigned int reserved : 20; /* [31:12] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_tx_calen_depth;

/* define the union u_xxvge_tx_calendar_table */
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
} u_xxvge_tx_calendar_table;

/* define the union u_xxvge_ptp_clk_freq_sel */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int freq : 1;      /* [0] */
        unsigned int reserved : 31; /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_ptp_clk_freq_sel;

#endif  // __C_UNION_DEFINE_XXVGE_GLOBAL_H__
