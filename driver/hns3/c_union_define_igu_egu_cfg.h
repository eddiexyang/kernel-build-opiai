/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: igu_egu_cfg
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __C_UNION_DEFINE_IGU_EGU_CFG_H__
#define __C_UNION_DEFINE_IGU_EGU_CFG_H__

/* define the union u_igu_egu_tnl_int_enable */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rx_buf_overflow_int_en : 1;       /* [0] */
        unsigned int rx_stp_fifo_overflow_int_en : 1;  /* [1] */
        unsigned int rx_stp_fifo_underflow_int_en : 1; /* [2] */
        unsigned int tx_buf_overflow_int_en : 1;       /* [3] */
        unsigned int tx_buf_underun_int_en : 1;        /* [4] */
        unsigned int rx_buf_ecc_sbit_err_int_en : 1;   /* [5] */
        unsigned int rx_buf_ecc_multi_err_int_en : 1;  /* [6] */
        unsigned int tx_buf_ecc_sbit_err_int_en : 1;   /* [7] */
        unsigned int tx_buf_ecc_multi_err_int_en : 1;  /* [8] */
        unsigned int rsv_0 : 23;                       /* [31:9] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_egu_tnl_int_enable;

/* define the union u_igu_egu_tnl_int_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rx_buf_overflow_int_sts : 1;       /* [0] */
        unsigned int rx_stp_fifo_overflow_int_sts : 1;  /* [1] */
        unsigned int rx_stp_fifo_underflow_int_sts : 1; /* [2] */
        unsigned int tx_buf_overflow_int_sts : 1;       /* [3] */
        unsigned int tx_buf_underun_int_sts : 1;        /* [4] */
        unsigned int rx_buf_ecc_sbit_err_int_sts : 1;   /* [5] */
        unsigned int rx_buf_ecc_multi_err_int_sts : 1;  /* [6] */
        unsigned int tx_buf_ecc_sbit_err_int_sts : 1;   /* [7] */
        unsigned int tx_buf_ecc_multi_err_int_sts : 1;  /* [8] */
        unsigned int rsv_1 : 23;                        /* [31:9] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_egu_tnl_int_status;

/* define the union u_igu_egu_tnl_int_set */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rx_buf_overflow_set : 1;       /* [0] */
        unsigned int rx_stp_fifo_overflow_set : 1;  /* [1] */
        unsigned int rx_stp_fifo_underflow_set : 1; /* [2] */
        unsigned int tx_buf_overflow_set : 1;       /* [3] */
        unsigned int tx_buf_underun_set : 1;        /* [4] */
        unsigned int rx_buf_ecc_sbit_err_set : 1;   /* [5] */
        unsigned int rx_buf_ecc_multi_err_set : 1;  /* [6] */
        unsigned int tx_buf_ecc_sbit_err_set : 1;   /* [7] */
        unsigned int tx_buf_ecc_multi_err_set : 1;  /* [8] */
        unsigned int rsv_2 : 23;                    /* [31:9] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_egu_tnl_int_set;

/* define the union u_igu_egu_tnl_int_source */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rx_buf_overflow_int_src : 1;       /* [0] */
        unsigned int rx_stp_fifo_overflow_int_src : 1;  /* [1] */
        unsigned int rx_stp_fifo_underflow_int_src : 1; /* [2] */
        unsigned int tx_buf_overflow_int_src : 1;       /* [3] */
        unsigned int tx_buf_underun_int_src : 1;        /* [4] */
        unsigned int rx_buf_ecc_sbit_err_int_src : 1;   /* [5] */
        unsigned int rx_buf_ecc_multi_err_int_src : 1;  /* [6] */
        unsigned int tx_buf_ecc_sbit_err_int_src : 1;   /* [7] */
        unsigned int tx_buf_ecc_multi_err_int_src : 1;  /* [8] */
        unsigned int rsv_3 : 23;                        /* [31:9] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_egu_tnl_int_source;

/* define the union u_igu_egu_tnl_int_rpt_type */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rx_buf_overflow_int_rpt_type : 2;       /* [1:0] */
        unsigned int rx_stp_fifo_overflow_int_rpt_type : 2;  /* [3:2] */
        unsigned int rx_stp_fifo_underflow_int_rpt_type : 2; /* [5:4] */
        unsigned int tx_buf_overflow_int_rpt_type : 2;       /* [7:6] */
        unsigned int tx_buf_underun_int_rpt_type : 2;        /* [9:8] */
        unsigned int rx_buf_ecc_sbit_err_int_rpt_type : 2;   /* [11:10] */
        unsigned int rx_buf_ecc_multi_err_int_rpt_type : 2;  /* [13:12] */
        unsigned int tx_buf_ecc_sbit_err_int_rpt_type : 2;   /* [15:14] */
        unsigned int tx_buf_ecc_multi_err_int_rpt_type : 2;  /* [17:16] */
        unsigned int rsv_4 : 14;                             /* [31:18] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_egu_tnl_int_rpt_type;

/* define the union u_igu_egu_tnl_int_ce_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rx_buf_overflow_int_sts_ce : 1;       /* [0] */
        unsigned int rx_stp_fifo_overflow_int_sts_ce : 1;  /* [1] */
        unsigned int rx_stp_fifo_underflow_int_sts_ce : 1; /* [2] */
        unsigned int tx_buf_overflow_int_sts_ce : 1;       /* [3] */
        unsigned int tx_buf_underun_int_sts_ce : 1;        /* [4] */
        unsigned int rx_buf_ecc_sbit_err_int_sts_ce : 1;   /* [5] */
        unsigned int rx_buf_ecc_multi_err_int_sts_ce : 1;  /* [6] */
        unsigned int tx_buf_ecc_sbit_err_int_sts_ce : 1;   /* [7] */
        unsigned int tx_buf_ecc_multi_err_int_sts_ce : 1;  /* [8] */
        unsigned int rsv_5 : 23;                           /* [31:9] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_egu_tnl_int_ce_status;

/* define the union u_igu_egu_tnl_int_nfe_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rx_buf_overflow_int_sts_nfe : 1;       /* [0] */
        unsigned int rx_stp_fifo_overflow_int_sts_nfe : 1;  /* [1] */
        unsigned int rx_stp_fifo_underflow_int_sts_nfe : 1; /* [2] */
        unsigned int tx_buf_overflow_int_sts_nfe : 1;       /* [3] */
        unsigned int tx_buf_underun_int_sts_nfe : 1;        /* [4] */
        unsigned int rx_buf_ecc_sbit_err_int_sts_nfe : 1;   /* [5] */
        unsigned int rx_buf_ecc_multi_err_int_sts_nfe : 1;  /* [6] */
        unsigned int tx_buf_ecc_sbit_err_int_sts_nfe : 1;   /* [7] */
        unsigned int tx_buf_ecc_multi_err_int_sts_nfe : 1;  /* [8] */
        unsigned int rsv_6 : 23;                            /* [31:9] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_egu_tnl_int_nfe_status;

/* define the union u_igu_egu_tnl_int_fe_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rx_buf_overflow_int_sts_fe : 1;       /* [0] */
        unsigned int rx_stp_fifo_overflow_int_sts_fe : 1;  /* [1] */
        unsigned int rx_stp_fifo_underflow_int_sts_fe : 1; /* [2] */
        unsigned int tx_buf_overflow_int_sts_fe : 1;       /* [3] */
        unsigned int tx_buf_underun_int_sts_fe : 1;        /* [4] */
        unsigned int rx_buf_ecc_sbit_err_int_sts_fe : 1;   /* [5] */
        unsigned int rx_buf_ecc_multi_err_int_sts_fe : 1;  /* [6] */
        unsigned int tx_buf_ecc_sbit_err_int_sts_fe : 1;   /* [7] */
        unsigned int tx_buf_ecc_multi_err_int_sts_fe : 1;  /* [8] */
        unsigned int rsv_7 : 23;                           /* [31:9] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_egu_tnl_int_fe_status;

/* define the union u_igu_mac_en_cfg */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_mac_tx_en : 1;   /* [0] */
        unsigned int mac_tx_en_inner : 1; /* [1] */
        unsigned int cfg_mac_rx_en : 1;   /* [2] */
        unsigned int mac_rx_en_inner : 1; /* [3] */
        unsigned int rsv_8 : 28;          /* [31:4] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_mac_en_cfg;

/* define the union u_igu_mac_mode_cfg */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_mac_rate : 4;       /* [3:0] */
        unsigned int mac_rate_igu_inner : 4; /* [7:4] */
        unsigned int mac_rate_egu_inner : 4; /* [11:8] */
        unsigned int rsv_9 : 20;             /* [31:12] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_mac_mode_cfg;

/* define the union u_igu_drop_en */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_rx_pkt_under60_drop_en : 1; /* [0] */
        unsigned int cfg_rx_pause_drop_en : 1;       /* [1] */
        unsigned int cfg_rx_pfc_drop_en : 1;         /* [2] */
        unsigned int cfg_rx_long_pkt_err_en : 1;     /* [3] */
        unsigned int rsv_10 : 28;                    /* [31:4] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_drop_en;

/* define the union u_igu_egu_loop */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_rx2tx_loop : 1; /* [0] */
        unsigned int cfg_tx2rx_loop : 1; /* [1] */
        unsigned int rsv_11 : 30;        /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_egu_loop;

/* define the union u_igu_max_frame_len */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_max_frame_len : 14; /* [13:0] */
        unsigned int rsv_12 : 18;            /* [31:14] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_max_frame_len;

/* define the union u_igu_min_frame_len */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_min_frame_len : 14; /* [13:0] */
        unsigned int rsv_13 : 18;            /* [31:14] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_min_frame_len;

/* define the union u_igu_rx_fifo_threshold */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_rx_buf_af_thd : 6; /* [5:0] */
        unsigned int rsv_14 : 26;           /* [31:6] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_rx_fifo_threshold;

/* define the union u_egu_tx_fifo_threshold */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_tx_buf_af_thd : 6; /* [5:0] */
        unsigned int cfg_tx_buf_ae_thd : 6; /* [11:6] */
        unsigned int rsv_15 : 20;           /* [31:12] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_egu_tx_fifo_threshold;

/* define the union u_egu_mac_app_loop_ae_threshold */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_tx_buf_loop_af_thd : 6;   /* [5:0] */
        unsigned int cfg_loop_egu_fifo_ae_thd : 6; /* [11:6] */
        unsigned int rsv_16 : 20;                  /* [31:12] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_egu_mac_app_loop_ae_threshold;

/* define the union u_egu_send_1588_opc_en */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_test_1588_opcode_add_en : 1; /* [0] */
        unsigned int rsv_17 : 31;                     /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_egu_send_1588_opc_en;

/* define the union u_egu_ptp_return_type */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_ptp_return_type : 1; /* [0] */
        unsigned int rsv_18 : 31;             /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_egu_ptp_return_type;

/* define the union u_igu_pause_pkt_dmac_h_cfg */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_igu_pause_pkt_dmac_h : 16; /* [15:0] */
        unsigned int rsv_19 : 16;                   /* [31:16] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_pause_pkt_dmac_h_cfg;

/* define the union u_igu_pause_pkt_dmac_l_cfg */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_igu_pause_pkt_dmac_l : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_pause_pkt_dmac_l_cfg;

/* define the union u_egu_dyingasp_en */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_dyinggasp_en : 1; /* [0] */
        unsigned int rsv_20 : 31;          /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_egu_dyingasp_en;

/* define the union u_egu_dyinggasp_pkt_cfg0 */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_dyinggasp_sa_high : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_egu_dyinggasp_pkt_cfg0;

/* define the union u_egu_dyinggasp_pkt_cfg1 */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_dyinggasp_sa_low : 16; /* [15:0] */
        unsigned int rsv_21 : 16;               /* [31:16] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_egu_dyinggasp_pkt_cfg1;

/* define the union u_egu_dyinggasp_pkt_cfg2 */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_dyinggasp_flag : 16; /* [15:0] */
        unsigned int cfg_dyinggasp_code : 8;  /* [23:16] */
        unsigned int cfg_dyinggasp_cnt : 8;   /* [31:24] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_egu_dyinggasp_pkt_cfg2;

/* define the union u_egu_tx_underun_threshold */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_egu_tx_underrun_int_thd : 16; /* [15:0] */
        unsigned int cfg_cge_eof_rx_delay : 5;         /* [20:16] */
        unsigned int cfg_lge_eof_rx_delay : 4;         /* [24:21] */
        unsigned int rsv_22 : 7;                       /* [31:25] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_egu_tx_underun_threshold;

/* define the union u_igu_rx_pkt_inner_gap */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_rx_pkt_inner_gap : 16; /* [15:0] */
        unsigned int rsv_23 : 16;               /* [31:16] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_rx_pkt_inner_gap;

/* define the union u_igu_cnt_cyc_clr_ctrl */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_cnt_clr_ce : 1; /* [0] */
        unsigned int rsv_24 : 31;        /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_cnt_cyc_clr_ctrl;

/* define the union u_igu_level1_wrr_weight */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_level1_wrr_weight0 : 8; /* [7:0] */
        unsigned int cfg_level1_wrr_weight1 : 8; /* [15:8] */
        unsigned int cfg_level1_wrr_weight2 : 8; /* [23:16] */
        unsigned int cfg_level1_wrr_weight3 : 8; /* [31:24] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_level1_wrr_weight;

/* define the union u_igu_auto_gate_en */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_auto_gate_en : 1; /* [0] */
        unsigned int rsv_25 : 31;          /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_auto_gate_en;

/* define the union u_igu_gating_window */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_gating_win : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_gating_window;

/* define the union u_igu_gating_sts */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_gating_cnt : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_gating_sts;

/* define the union u_igu_eco_rw_rsv0 */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_eco_rw_rsv0 : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_eco_rw_rsv0;

/* define the union u_igu_eco_rw_rsv1 */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_eco_rw_rsv1 : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_eco_rw_rsv1;

/* define the union u_igu_eco_rw_rsv2 */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_eco_rw_rsv2 : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_eco_rw_rsv2;

/* define the union u_igu_eco_rw_rsv3 */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_eco_rw_rsv3 : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_eco_rw_rsv3;

/* define the union u_igu_buf_ecc_err_addr */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rx_buf_ecc_sbit_err_addr : 6;  /* [5:0] */
        unsigned int rx_buf_ecc_multi_err_addr : 6; /* [11:6] */
        unsigned int rsv_26 : 20;                   /* [31:12] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_buf_ecc_err_addr;

/* define the union u_egu_buf_ecc_err_addr */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int tx_buf_ecc_sbit_err_addr : 6;  /* [5:0] */
        unsigned int tx_buf_ecc_multi_err_addr : 6; /* [11:6] */
        unsigned int rsv_27 : 20;                   /* [31:12] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_egu_buf_ecc_err_addr;

/* define the union u_tx_1588_return_type_error */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int return_type_err_1588 : 1; /* [0] */
        unsigned int rsv_28 : 31;              /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_tx_1588_return_type_error;

/* define the union u_igu_rx_err_pkt */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int igu_rx_err_pkt : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_rx_err_pkt;

/* define the union u_igu_rx_no_sof_pkt */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int igu_rx_no_sof_pkt_cnt : 16; /* [15:0] */
        unsigned int rsv_29 : 16;                /* [31:16] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_rx_no_sof_pkt;

/* define the union u_igu_rx_ctrl_drop_pkt */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int igu_rx_ctrl_drop_cnt : 16; /* [15:0] */
        unsigned int rsv_30 : 16;               /* [31:16] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_rx_ctrl_drop_pkt;

/* define the union u_igu_rx_oversize_pkt */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int igu_rx_oversize_pkt : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_rx_oversize_pkt;

/* define the union u_igu_rx_undersize_pkt */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int igu_rx_undersize_pkt : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_rx_undersize_pkt;

/* define the union u_igu_rx_no_eof_pkt */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int igu_rx_no_eof_pkt_cnt : 16; /* [15:0] */
        unsigned int rsv_31 : 16;                /* [31:16] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_rx_no_eof_pkt;

/* define the union u_egu_tx_err_pkt */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int tx_tnl_pkt_err : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_egu_tx_err_pkt;

/* define the union u_egu_tx_short_drop_pkt */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int egu_tx_short_drop_pkt : 16; /* [15:0] */
        unsigned int rsv_32 : 16;                /* [31:16] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_egu_tx_short_drop_pkt;

/* define the union u_igu_rx_out_all_pkt */
typedef union {
    /* define the struct bits */
    struct {
        unsigned long igu_rx_out_all_pkt : 64; /* [63:0] */
    } bits;

    /* define an unsigned member */
    unsigned long value;
} u_igu_rx_out_all_pkt;

/* define the union u_egu_tx_out_all_pkt */
typedef union {
    /* define the struct bits */
    struct {
        unsigned long egu_tx_out_all_pkt : 64; /* [63:0] */
    } bits;

    /* define an unsigned member */
    unsigned long value;
} u_egu_tx_out_all_pkt;

/* define the union u_igu_rx_stp_fifo_cnt */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int igu_rx_stp_fifo_cnt : 6; /* [5:0] */
        unsigned int rsv_33 : 26;             /* [31:6] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_rx_stp_fifo_cnt;

/* define the union u_egu_tx_1588_pkt */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int tx_1588_pkt : 32; /* [31:0] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_egu_tx_1588_pkt;

/* define the union u_egu_tx_dyinggasp_cnt */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int egu_tx_dyinggasp_cnt : 8; /* [7:0] */
        unsigned int rsv_34 : 24;              /* [31:8] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_egu_tx_dyinggasp_cnt;

/* define the union u_igu_egu_fifo_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rx_buf_sta_dfx : 9; /* [8:0] */
        unsigned int tx_buf_sta_dfx : 9; /* [17:9] */
        unsigned int rsv_35 : 14;        /* [31:18] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_igu_egu_fifo_status;

#endif  // __C_UNION_DEFINE_IGU_EGU_CFG_H__
