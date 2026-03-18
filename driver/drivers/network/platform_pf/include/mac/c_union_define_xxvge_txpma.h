/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: xxvge_txpma
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __C_UNION_DEFINE_XXVGE_TXPMA_H__
#define __C_UNION_DEFINE_XXVGE_TXPMA_H__

/* define the union u_xxvge_txpma_control */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int tx_bit_reverse : 1;                          /* [0] */
        unsigned int tx_afifo_bypass_en : 1;                      /* [1] */
        unsigned int txpma_tx_fifo_read_level : 6;                /* [7:2] */
        unsigned int txpma_pulse_w : 2;                           /* [9:8] */
        unsigned int para_data_width : 3;                         /* [12:10] */
        unsigned int tx_os_mode : 2;                              /* [14:13] */
        unsigned int txpma_waterline_clr : 1;                     /* [15] */
        unsigned int sds_pulse_gap : 7;                           /* [22:16] */
        unsigned int sds_gap_base : 2;                            /* [24:23] */
        unsigned int normal_1588_mode_sel : 1;                    /* [25] */
        unsigned int afifo_bypass_base : 4;                       /* [29:26] */
        unsigned int tx_half_duplex_ram_bypass_en : 1;            /* [30] */
        unsigned int two_freq_divider_posedge_clk_indication : 1; /* [31] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_txpma_control;
#endif  // __C_UNION_DEFINE_XXVGE_TXPMA_H__