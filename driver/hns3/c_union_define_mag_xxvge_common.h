/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: mag_xxvge_common
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __C_UNION_DEFINE_MAG_XXVGE_COMMON_H__
#define __C_UNION_DEFINE_MAG_XXVGE_COMMON_H__

/* define the union u_afifo_tnl_int_enable */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int egu_lge_afifo_ovf_int_en : 1; /* [0] */
        unsigned int tx_ptp_fifo_err_int_en : 1;   /* [1] */
        unsigned int rsv_0 : 30;                   /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_afifo_tnl_int_enable;

/* define the union u_afifo_tnl_int_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int egu_lge_afifo_ovf_int_sts : 1; /* [0] */
        unsigned int tx_ptp_fifo_err_int_sts : 1;   /* [1] */
        unsigned int rsv_1 : 30;                    /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_afifo_tnl_int_status;

/* define the union u_afifo_tnl_int_set */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int egu_lge_afifo_ovf_int_set : 1; /* [0] */
        unsigned int tx_ptp_fifo_err_int_set : 1;   /* [1] */
        unsigned int rsv_2 : 30;                    /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_afifo_tnl_int_set;

/* define the union u_afifo_tnl_int_source */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int egu_lge_afifo_ovf_int_src : 1; /* [0] */
        unsigned int tx_ptp_fifo_err_int_src : 1;   /* [1] */
        unsigned int rsv_3 : 30;                    /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_afifo_tnl_int_source;

/* define the union u_afifo_tnl_int_rpt_type */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int egu_lge_afifo_ovf_int_rpt_type : 2; /* [1:0] */
        unsigned int tx_ptp_fifo_err_int_rpt_type : 2;   /* [3:2] */
        unsigned int rsv_4 : 28;                         /* [31:4] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_afifo_tnl_int_rpt_type;

/* define the union u_afifo_tnl_int_ce_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int egu_lge_afifo_ovf_int_sts_ce : 1; /* [0] */
        unsigned int tx_ptp_fifo_err_int_sts_ce : 1;   /* [1] */
        unsigned int rsv_5 : 30;                       /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_afifo_tnl_int_ce_status;

/* define the union u_afifo_tnl_int_nfe_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int egu_lge_afifo_ovf_int_sts_nfe : 1; /* [0] */
        unsigned int tx_ptp_fifo_err_int_sts_nfe : 1;   /* [1] */
        unsigned int rsv_6 : 30;                        /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_afifo_tnl_int_nfe_status;

/* define the union u_afifo_tnl_int_fe_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int egu_lge_afifo_ovf_int_sts_fe : 1; /* [0] */
        unsigned int tx_ptp_fifo_err_int_sts_fe : 1;   /* [1] */
        unsigned int rsv_7 : 30;                       /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_afifo_tnl_int_fe_status;

/* define the union u_mac_cfgif_rstart */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_xxvge_cfgif_rstart : 1; /* [0] */
        unsigned int cfg_lge_cfgif_rstart : 1;   /* [1] */
        unsigned int rsv_8 : 30;                 /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_cfgif_rstart;

/* define the union u_mac_rd_clear */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_xxvge_rd_clr : 1; /* [0] */
        unsigned int cfg_lge_rd_clr : 1;   /* [1] */
        unsigned int rsv_9 : 30;           /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_rd_clear;

/* define the union u_mac_clk_gate_enable */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_regs_xxvge_clk_gate_en : 1; /* [0] */
        unsigned int cfg_regs_lge_clk_gate_en : 1;   /* [1] */
        unsigned int rsv_10 : 30;                    /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_clk_gate_enable;

/* define the union u_mac_async_fifo_ae_threshold */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_xxvge_async_fifo_ae_thd : 4; /* [3:0] */
        unsigned int rsv_11 : 28;                     /* [31:4] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_async_fifo_ae_threshold;

/* define the union u_mac_reg_timeout_threshold */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int cfg_xxvge_reg_acs_thd : 16; /* [15:0] */
        unsigned int rsv_12 : 16;                /* [31:16] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_reg_timeout_threshold;

/* define the union u_lge_igu_afifo_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int lge_igu_afifo_sts : 2; /* [1:0] */
        unsigned int rsv_13 : 30;           /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_lge_igu_afifo_status;

/* define the union u_lge_egu_afifo_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int egu_lge_afifo_sts : 2; /* [1:0] */
        unsigned int rsv_14 : 30;           /* [31:2] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_lge_egu_afifo_status;

/* define the union u_mac_reg_acs_timeout_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int xxvge_reg_acs_timeout : 1; /* [0] */
        unsigned int rsv_15 : 31;               /* [31:1] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_mac_reg_acs_timeout_status;

#endif  // __C_UNION_DEFINE_MAG_XXVGE_COMMON_H__