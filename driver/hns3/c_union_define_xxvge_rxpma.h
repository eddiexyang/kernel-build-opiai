/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: xxvge_rxpma
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __C_UNION_DEFINE_XXVGE_RXPMA_H__
#define __C_UNION_DEFINE_XXVGE_RXPMA_H__

/* define the union u_xxvge_rxpma_control */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int ovf_init_en : 1;           /* [0] */
        unsigned int rx_bit_reverse : 1;        /* [1] */
        unsigned int gb_i_width_sel : 1;        /* [2] */
        unsigned int rgmii_sel : 1;             /* [3] */
        unsigned int fifo_alemp_th : 4;         /* [7:4] */
        unsigned int fifo_alful_th : 4;         /* [11:8] */
        unsigned int para_data_width : 3;       /* [14:12] */
        unsigned int rx_fifo_wr_flick_en : 1;   /* [15] */
        unsigned int reserved : 1;              /* [16] */
        unsigned int rx_fifo_waterline_clr : 1; /* [17] */
        unsigned int reserved1 : 1;             /* [18] */
        unsigned int rx_buf_abn_recover_en : 1; /* [19] */
        unsigned int rx_clk_los_det_en : 1;     /* [20] */
        unsigned int reserved2 : 3;             /* [23:21] */
        unsigned int rx_buf_abn_thd : 8;        /* [31:24] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_rxpma_control;

/* define the union u_xxvge_rxpma_enable */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int enable : 2;    /* [1:0] */
        unsigned int reserved : 30; /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_rxpma_enable;

#endif  // __C_UNION_DEFINE_XXVGE_RXPMA_H__
