/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_fun_common
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_FUN_COMMON_H__
#define __REG_FUN_COMMON_H__

typedef union {
    struct {
        unsigned int tqp_int_en : 1; /* [0] */
        unsigned int rsv : 31;       /* [31:1] */
    } bits;

    unsigned int value;
} u_tqp_int_ctrl;

typedef union {
    struct {
        unsigned int tqp_int_gl0 : 12; /* [11:0] */
        unsigned int rsv : 19;         /* [30:12] */
        unsigned int gap_cfg_gl0 : 1;  /* [31] */
    } bits;

    unsigned int value;
} u_tqp_int_gl0;

typedef union {
    struct {
        unsigned int tqp_int_gl1 : 12; /* [11:0] */
        unsigned int rsv : 19;         /* [30:12] */
        unsigned int gap_cfg_gl1 : 1;  /* [31] */
    } bits;

    unsigned int value;
} u_tqp_int_gl1;

typedef union {
    struct {
        unsigned int tqp_int_gl2 : 12; /* [11:0] */
        unsigned int rsv : 19;         /* [30:12] */
        unsigned int gap_cfg_gl2 : 1;  /* [31] */
    } bits;

    unsigned int value;
} u_tqp_int_gl2;

typedef union {
    struct {
        unsigned int tqp_int_rl : 6;    /* [5:0] */
        unsigned int tqp_int_rl_en : 1; /* [6] */
        unsigned int rsv : 25;          /* [31:7] */
    } bits;

    unsigned int value;
} u_tqp_int_rl;

typedef union {
    struct {
        unsigned int tqp_int_tx_ql : 10; /* [9:0] */
        unsigned int rsv : 22;           /* [31:10] */
    } bits;

    unsigned int value;
} u_tqp_int_ql_tx;

typedef union {
    struct {
        unsigned int tqp_int_rx_ql : 10; /* [9:0] */
        unsigned int rsv : 22;           /* [31:10] */
    } bits;

    unsigned int value;
} u_tqp_int_ql_rx;

typedef union {
    struct {
        unsigned int tqp_int_eq_gl0 : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_tqp_int_eq_en_gl0;

typedef union {
    struct {
        unsigned int tqp_int_eq_gl1 : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_tqp_int_eq_en_gl1;

typedef union {
    struct {
        unsigned int tqp_int_eq_gl2 : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_tqp_int_eq_en_gl2;

typedef union {
    struct {
        unsigned int vector0_1588_int_en : 1; /* [0] */
        unsigned int rsv : 31;                /* [31:1] */
    } bits;

    unsigned int value;
} u_vector0_port_other_int_ctrl;

typedef union {
    struct {
        unsigned int vector0_1588_int_src : 1; /* [0] */
        unsigned int rsv : 31;                 /* [31:1] */
    } bits;

    unsigned int value;
} u_vector0_port_other_int_src;

typedef union {
    struct {
        unsigned int vector0_1588_int_sts : 1; /* [0] */
        unsigned int rsv : 31;                 /* [31:1] */
    } bits;

    unsigned int value;
} u_vector0_port_other_int_sts;

typedef union {
    struct {
        unsigned int nic_ras_intr_fe : 8;  /* [7:0] */
        unsigned int nic_ras_intr_nfe : 8; /* [15:8] */
        unsigned int nic_ras_intr_ce : 8;  /* [23:16] */
        unsigned int rsv : 8;              /* [31:24] */
    } bits;

    unsigned int value;
} u_ras_port_other_int_sts;

#endif  // __REG_FUN_COMMON_H__