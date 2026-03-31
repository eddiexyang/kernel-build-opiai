/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_rpu
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_RPU_H__
#define __REG_RPU_H__

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_en : 1; /* [0] */
        unsigned int mem_ecc_mbit_int_en : 1; /* [1] */
        unsigned int fifo_rd_int_en : 1;      /* [2] */
        unsigned int fifo_wr_int_en : 1;      /* [3] */
        unsigned int fsm_hbeat_int_en : 1;    /* [4] */
        unsigned int wr_axi_err_int_en : 1;   /* [5] */
        unsigned int rsv : 26;                /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_pf_int_enable0;

typedef union {
    struct {
        unsigned int ts_tx_bak_int_en : 1; /* [0] */
        unsigned int rsv : 31;             /* [31:1] */
    } bits;

    unsigned int value;
} u_rpu_pf_int_enable2;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_sts : 1; /* [0] */
        unsigned int mem_ecc_mbit_int_sts : 1; /* [1] */
        unsigned int fifo_rd_int_sts : 1;      /* [2] */
        unsigned int fifo_wr_int_sts : 1;      /* [3] */
        unsigned int fsm_hbeat_int_sts : 1;    /* [4] */
        unsigned int wr_axi_err_int_sts : 1;   /* [5] */
        unsigned int rsv : 26;                 /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_pf_int_status0;

typedef union {
    struct {
        unsigned int ts_tx_bak_int_sts : 1; /* [0] */
        unsigned int rsv : 31;              /* [31:1] */
    } bits;
    unsigned int value;
} u_rpu_pf_int_status2;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_set : 1; /* [0] */
        unsigned int mem_ecc_mbit_int_set : 1; /* [1] */
        unsigned int fifo_rd_int_set : 1;      /* [2] */
        unsigned int fifo_wr_int_set : 1;      /* [3] */
        unsigned int fsm_hbeat_int_set : 1;    /* [4] */
        unsigned int wr_axi_err_int_set : 1;   /* [5] */
        unsigned int rsv : 26;                 /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_pf_int_set0;

typedef union {
    struct {
        unsigned int ts_tx_bak_int_set : 1; /* [0] */
        unsigned int rsv : 31;              /* [31:1] */
    } bits;

    unsigned int value;
} u_rpu_pf_int_set2;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_src : 1; /* [0] */
        unsigned int mem_ecc_mbit_int_src : 1; /* [1] */
        unsigned int fifo_rd_int_src : 1;      /* [2] */
        unsigned int fifo_wr_int_src : 1;      /* [3] */
        unsigned int fsm_hbeat_int_src : 1;    /* [4] */
        unsigned int wr_axi_err_int_src : 1;   /* [5] */
        unsigned int rsv : 26;                 /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_pf_int_source0;

typedef union {
    struct {
        unsigned int ts_tx_bak_int_src : 1; /* [0] */
        unsigned int rsv : 31;              /* [31:1] */
    } bits;

    unsigned int value;
} u_rpu_pf_int_source2;

typedef union {
    struct {
        unsigned int pre_buf_req_fifo_wr_int_sts : 1; /* [0] */
        unsigned int buf_order_fifo_wr_int_sts : 1;   /* [1] */
        unsigned int rsv : 30;                        /* [31:2] */
    } bits;

    unsigned int value;
} u_rpu_prep_fifo_wr_int;

typedef union {
    struct {
        unsigned int test_bd_data_fifo_wr_int_sts : 1; /* [0] */
        unsigned int test_bd_info_fifo_wr_int_sts : 1; /* [1] */
        unsigned int test_bd_ostd_fifo_wr_int_sts : 1; /* [2] */
        unsigned int stash_sch_fifo_wr_int_sts : 1;    /* [3] */
        unsigned int prep_out_fifo_wr_int_sts : 1;     /* [4] */
        unsigned int rsv : 27;                         /* [31:5] */
    } bits;

    unsigned int value;
} u_rpu_axi_fifo_wr_int0;

typedef union {
    struct {
        unsigned int prep_pkt_fifo_wr_int_sts : 1;  /* [0] */
        unsigned int prep_info_fifo_wr_int_sts : 1; /* [1] */
        unsigned int prep_key_fifo_wr_int_sts : 1;  /* [2] */
        unsigned int rsv : 29;                      /* [31:3] */
    } bits;

    unsigned int value;
} u_rpu_store_fifo_wr_int;

typedef union {
    struct {
        unsigned int axi_data_fifo_wr_int_sts : 1; /* [0] */
        unsigned int axi_info_fifo_wr_int_sts : 1; /* [1] */
        unsigned int poe_cmd_fifo_wr_int_sts : 1;  /* [2] */
        unsigned int poe_data_fifo_wr_int_sts : 1; /* [3] */
        unsigned int poe_ostd_fifo_wr_int_sts : 1; /* [4] */
        unsigned int ddr_ostd_fifo_wr_int_sts : 1; /* [5] */
        unsigned int rsv : 26;                     /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_axi_fifo_wr_int1;

typedef union {
    struct {
        unsigned int pre_buf_req_fifo_rd_int_sts : 1; /* [0] */
        unsigned int buf_order_fifo_rd_int_sts : 1;   /* [1] */
        unsigned int rsv : 30;                        /* [31:2] */
    } bits;

    unsigned int value;
} u_rpu_prep_fifo_rd_int;

typedef union {
    struct {
        unsigned int test_bd_data_fifo_rd_int_sts : 1; /* [0] */
        unsigned int test_bd_info_fifo_rd_int_sts : 1; /* [1] */
        unsigned int test_bd_ostd_fifo_rd_int_sts : 1; /* [2] */
        unsigned int stash_sch_fifo_rd_int_sts : 1;    /* [3] */
        unsigned int prep_out_fifo_rd_int_sts : 1;     /* [4] */
        unsigned int rsv : 27;                         /* [31:5] */
    } bits;

    unsigned int value;
} u_rpu_axi_fifo_rd_int0;

typedef union {
    struct {
        unsigned int prep_pkt_fifo_rd_int_sts : 1;  /* [0] */
        unsigned int prep_info_fifo_rd_int_sts : 1; /* [1] */
        unsigned int prep_key_fifo_rd_int_sts : 1;  /* [2] */
        unsigned int rsv : 29;                      /* [31:3] */
    } bits;

    unsigned int value;
} u_rpu_store_fifo_rd_int;

typedef union {
    struct {
        unsigned int axi_data_fifo_rd_int_sts : 1; /* [0] */
        unsigned int axi_info_fifo_rd_int_sts : 1; /* [1] */
        unsigned int poe_cmd_fifo_rd_int_sts : 1;  /* [2] */
        unsigned int poe_data_fifo_rd_int_sts : 1; /* [3] */
        unsigned int poe_ostd_fifo_rd_int_sts : 1; /* [4] */
        unsigned int ddr_ostd_fifo_rd_int_sts : 1; /* [5] */
        unsigned int rsv : 26;                     /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_axi_fifo_rd_int1;

typedef union {
    struct {
        unsigned int mem_sbit_ecc_mem_addr : 12; /* [11:0] */
        unsigned int mem_sbit_ecc_mem_id : 8;    /* [19:12] */
        unsigned int rsv : 12;                   /* [31:20] */
    } bits;

    unsigned int value;
} u_rpu_mem_err_info0;

typedef union {
    struct {
        unsigned int mem_mbit_ecc_mem_addr : 12; /* [11:0] */
        unsigned int mem_mbit_ecc_mem_id : 8;    /* [19:12] */
        unsigned int rsv : 12;                   /* [31:20] */
    } bits;

    unsigned int value;
} u_rpu_mem_err_info1;

typedef union {
    struct {
        unsigned int wr_axi_int_info : 16; /* [15:0] */
        unsigned int rsv : 16;             /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_wr_axi_err_info;

typedef union {
    struct {
        unsigned int fsm_hbeat_int_info : 1; /* [0] */
        unsigned int rsv : 31;               /* [31:1] */
    } bits;

    unsigned int value;
} u_rpu_tnl_int_info;

typedef struct {
    u64 tx_1588_int_info_low;  /* [63:0] */
    u32 tx_1588_int_info_high; /* [95:64] */
} st_tx_1588_int_info;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_type : 2; /* [1:0] */
        unsigned int mem_ecc_mbit_int_type : 2; /* [3:2] */
        unsigned int fifo_rd_int_type : 2;      /* [5:4] */
        unsigned int fifo_wr_int_type : 2;      /* [7:6] */
        unsigned int fsm_hbeat_int_type : 2;    /* [9:8] */
        unsigned int wr_axi_err_int_type : 2;   /* [11:10] */
        unsigned int ts_tx_bak_int_type : 2;    /* [13:12] */
        unsigned int rsv : 18;                  /* [31:14] */
    } bits;

    unsigned int value;
} u_rpu_pf_int_type0;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_ce : 1; /* [0] */
        unsigned int mem_ecc_mbit_int_ce : 1; /* [1] */
        unsigned int fifo_rd_int_ce : 1;      /* [2] */
        unsigned int fifo_wr_int_ce : 1;      /* [3] */
        unsigned int fsm_hbeat_int_ce : 1;    /* [4] */
        unsigned int wr_axi_err_int_ce : 1;   /* [5] */
        unsigned int rsv : 26;                /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_pf_int_ce0;

typedef union {
    struct {
        unsigned int ts_tx_bak_int_ce : 1; /* [0] */
        unsigned int rsv : 31;             /* [31:1] */
    } bits;

    unsigned int value;
} u_rpu_pf_int_ce2;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_nfe : 1; /* [0] */
        unsigned int mem_ecc_mbit_int_nfe : 1; /* [1] */
        unsigned int fifo_rd_int_nfe : 1;      /* [2] */
        unsigned int fifo_wr_int_nfe : 1;      /* [3] */
        unsigned int fsm_hbeat_int_nfe : 1;    /* [4] */
        unsigned int wr_axi_err_int_nfe : 1;   /* [5] */
        unsigned int rsv : 26;                 /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_pf_int_nfe0;

typedef union {
    struct {
        unsigned int ts_tx_bak_int_nfe : 1; /* [0] */
        unsigned int rsv : 31;              /* [31:1] */
    } bits;

    unsigned int value;
} u_rpu_pf_int_nfe2;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_fe : 1; /* [0] */
        unsigned int mem_ecc_mbit_int_fe : 1; /* [1] */
        unsigned int fifo_rd_int_fe : 1;      /* [2] */
        unsigned int fifo_wr_int_fe : 1;      /* [3] */
        unsigned int fsm_hbeat_int_fe : 1;    /* [4] */
        unsigned int wr_axi_err_int_fe : 1;   /* [5] */
        unsigned int rsv : 26;                /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_pf_int_fe0;

typedef union {
    struct {
        unsigned int ts_tx_bak_int_fe : 1; /* [0] */
        unsigned int rsv : 31;             /* [31:1] */
    } bits;

    unsigned int value;
} u_rpu_pf_int_fe2;

typedef union {
    struct {
        unsigned int cfg_sbit_ecc_int_thd : 8; /* [7:0] */
        unsigned int rsv : 24;                 /* [31:8] */
    } bits;

    unsigned int value;
} u_rpu_mem_sbit_ecc_thd;

typedef union {
    struct {
        unsigned int rpu_mem_sbit_err_cnt : 16; /* [15:0] */
        unsigned int rsv : 16;                  /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_mem_sbit_err_cnt;

typedef union {
    struct {
        unsigned int rpu_mem_mbit_err_cnt : 16; /* [15:0] */
        unsigned int rsv : 16;                  /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_mem_mbit_err_cnt;

typedef union {
    struct {
        unsigned int mem_init_start : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_rpu_mem_init_start;

typedef union {
    struct {
        unsigned int mem_init_done : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_rpu_mem_init_done;

typedef union {
    struct {
        unsigned int cfg_sp_mem_timing_ctrl : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_mem_ctrl_sp;

typedef union {
    struct {
        unsigned int cfg_tp_mem_timing_ctrl : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_mem_ctrl_tp;

typedef union {
    struct {
        unsigned int cfg_mem_power_ctrl : 8; /* [7:0] */
        unsigned int rsv : 24;               /* [31:8] */
    } bits;

    unsigned int value;
} u_rpu_mem_ctrl_power;

typedef union {
    struct {
        unsigned int cfg_mem_ecc_bypass : 1; /* [0] */
        unsigned int rsv : 31;               /* [31:1] */
    } bits;

    unsigned int value;
} u_rpu_mem_ecc_bypass;

typedef union {
    struct {
        unsigned int cfg_ecc_sbit_int_thd : 8; /* [7:0] */
        unsigned int rsv : 24;                 /* [31:8] */
    } bits;

    unsigned int value;
} u_rpu_mem_ecc_sbit_thd;

typedef union {
    struct {
        unsigned int cfg_tcam_mbist_test : 3;  /* [2:0] */
        unsigned int rsv0 : 1;                 /* [3] */
        unsigned int cfg_tcam_mbist_t_sel : 8; /* [11:4] */
        unsigned int rsv1 : 4;                 /* [15:12] */
        unsigned int cfg_tcam_mbist_en : 1;    /* [16] */
        unsigned int rsv2 : 15;                /* [31:17] */
    } bits;

    unsigned int value;
} u_rpu_tcam_mbist_cfg;

typedef union {
    struct {
        unsigned int tcam_mbist_done : 1;    /* [0] */
        unsigned int tcam_mbist_err_mem : 1; /* [1] */
        unsigned int tcam_mbist_err_cmp : 1; /* [2] */
        unsigned int rsv : 29;               /* [31:3] */
    } bits;

    unsigned int value;
} u_rpu_tcam_mbist_test;

typedef union {
    struct {
        unsigned int axi_data_mem_m0_ecc_req : 2; /* [1:0] */
        unsigned int poe_data_mem_m0_ecc_req : 2; /* [3:2] */
        unsigned int axi_data_mem_m1_ecc_req : 2; /* [5:4] */
        unsigned int poe_data_mem_m1_ecc_req : 2; /* [7:6] */
        unsigned int test_data_mem_ecc_req : 2;   /* [9:8] */
        unsigned int rsv : 22;                    /* [31:10] */
    } bits;

    unsigned int value;
} u_rpu_mem_err_insert_0;

typedef union {
    struct {
        unsigned int prep_info_mem_ecc_req0 : 2;  /* [1:0] */
        unsigned int prep_info_mem_ecc_req1 : 2;  /* [3:2] */
        unsigned int prep_info_mem_ecc_req2 : 2;  /* [5:4] */
        unsigned int prep_info_mem_ecc_req3 : 2;  /* [7:6] */
        unsigned int prep_info_mem_ecc_req4 : 2;  /* [9:8] */
        unsigned int prep_info_mem_ecc_req5 : 2;  /* [11:10] */
        unsigned int prep_info_mem_ecc_req6 : 2;  /* [13:12] */
        unsigned int prep_info_mem_ecc_req7 : 2;  /* [15:14] */
        unsigned int prep_info_mem_ecc_req8 : 2;  /* [17:16] */
        unsigned int prep_info_mem_ecc_req9 : 2;  /* [19:18] */
        unsigned int prep_info_mem_ecc_req10 : 2; /* [21:20] */
        unsigned int prep_info_mem_ecc_req11 : 2; /* [23:22] */
        unsigned int prep_info_mem_ecc_req12 : 2; /* [25:24] */
        unsigned int prep_info_mem_ecc_req13 : 2; /* [27:26] */
        unsigned int prep_info_mem_ecc_req14 : 2; /* [29:28] */
        unsigned int prep_info_mem_ecc_req15 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_rpu_mem_err_insert_1;

typedef union {
    struct {
        unsigned int prep_info_mem_ecc_req16 : 2; /* [1:0] */
        unsigned int prep_info_mem_ecc_req17 : 2; /* [3:2] */
        unsigned int prep_info_mem_ecc_req18 : 2; /* [5:4] */
        unsigned int prep_info_mem_ecc_req19 : 2; /* [7:6] */
        unsigned int prep_info_mem_ecc_req20 : 2; /* [9:8] */
        unsigned int prep_info_mem_ecc_req21 : 2; /* [11:10] */
        unsigned int prep_info_mem_ecc_req22 : 2; /* [13:12] */
        unsigned int prep_info_mem_ecc_req23 : 2; /* [15:14] */
        unsigned int prep_info_mem_ecc_req24 : 2; /* [17:16] */
        unsigned int prep_info_mem_ecc_req25 : 2; /* [19:18] */
        unsigned int prep_info_mem_ecc_req26 : 2; /* [21:20] */
        unsigned int prep_info_mem_ecc_req27 : 2; /* [23:22] */
        unsigned int prep_info_mem_ecc_req28 : 2; /* [25:24] */
        unsigned int prep_info_mem_ecc_req29 : 2; /* [27:26] */
        unsigned int prep_info_mem_ecc_req30 : 2; /* [29:28] */
        unsigned int prep_info_mem_ecc_req31 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_rpu_mem_err_insert_2;

typedef union {
    struct {
        unsigned int prep_pkt_mem_ecc_req0 : 2;  /* [1:0] */
        unsigned int prep_pkt_mem_ecc_req1 : 2;  /* [3:2] */
        unsigned int prep_pkt_mem_ecc_req2 : 2;  /* [5:4] */
        unsigned int prep_pkt_mem_ecc_req3 : 2;  /* [7:6] */
        unsigned int prep_pkt_mem_ecc_req4 : 2;  /* [9:8] */
        unsigned int prep_pkt_mem_ecc_req5 : 2;  /* [11:10] */
        unsigned int prep_pkt_mem_ecc_req6 : 2;  /* [13:12] */
        unsigned int prep_pkt_mem_ecc_req7 : 2;  /* [15:14] */
        unsigned int prep_pkt_mem_ecc_req8 : 2;  /* [17:16] */
        unsigned int prep_pkt_mem_ecc_req9 : 2;  /* [19:18] */
        unsigned int prep_pkt_mem_ecc_req10 : 2; /* [21:20] */
        unsigned int prep_pkt_mem_ecc_req11 : 2; /* [23:22] */
        unsigned int prep_pkt_mem_ecc_req12 : 2; /* [25:24] */
        unsigned int prep_pkt_mem_ecc_req13 : 2; /* [27:26] */
        unsigned int prep_pkt_mem_ecc_req14 : 2; /* [29:28] */
        unsigned int prep_pkt_mem_ecc_req15 : 2; /* [31:30] */
    } bits;
    unsigned int value;
} u_rpu_mem_err_insert_3;

typedef union {
    struct {
        unsigned int prep_pkt_mem_ecc_req16 : 2; /* [1:0] */
        unsigned int prep_pkt_mem_ecc_req17 : 2; /* [3:2] */
        unsigned int prep_pkt_mem_ecc_req18 : 2; /* [5:4] */
        unsigned int prep_pkt_mem_ecc_req19 : 2; /* [7:6] */
        unsigned int prep_pkt_mem_ecc_req20 : 2; /* [9:8] */
        unsigned int prep_pkt_mem_ecc_req21 : 2; /* [11:10] */
        unsigned int prep_pkt_mem_ecc_req22 : 2; /* [13:12] */
        unsigned int prep_pkt_mem_ecc_req23 : 2; /* [15:14] */
        unsigned int prep_pkt_mem_ecc_req24 : 2; /* [17:16] */
        unsigned int prep_pkt_mem_ecc_req25 : 2; /* [19:18] */
        unsigned int prep_pkt_mem_ecc_req26 : 2; /* [21:20] */
        unsigned int prep_pkt_mem_ecc_req27 : 2; /* [23:22] */
        unsigned int prep_pkt_mem_ecc_req28 : 2; /* [25:24] */
        unsigned int prep_pkt_mem_ecc_req29 : 2; /* [27:26] */
        unsigned int prep_pkt_mem_ecc_req30 : 2; /* [29:28] */
        unsigned int prep_pkt_mem_ecc_req31 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_rpu_mem_err_insert_4;

typedef union {
    struct {
        unsigned int prep_key_mem_ecc_req0 : 2;  /* [1:0] */
        unsigned int prep_key_mem_ecc_req1 : 2;  /* [3:2] */
        unsigned int prep_key_mem_ecc_req2 : 2;  /* [5:4] */
        unsigned int prep_key_mem_ecc_req3 : 2;  /* [7:6] */
        unsigned int prep_key_mem_ecc_req4 : 2;  /* [9:8] */
        unsigned int prep_key_mem_ecc_req5 : 2;  /* [11:10] */
        unsigned int prep_key_mem_ecc_req6 : 2;  /* [13:12] */
        unsigned int prep_key_mem_ecc_req7 : 2;  /* [15:14] */
        unsigned int prep_key_mem_ecc_req8 : 2;  /* [17:16] */
        unsigned int prep_key_mem_ecc_req9 : 2;  /* [19:18] */
        unsigned int prep_key_mem_ecc_req10 : 2; /* [21:20] */
        unsigned int prep_key_mem_ecc_req11 : 2; /* [23:22] */
        unsigned int prep_key_mem_ecc_req12 : 2; /* [25:24] */
        unsigned int prep_key_mem_ecc_req13 : 2; /* [27:26] */
        unsigned int prep_key_mem_ecc_req14 : 2; /* [29:28] */
        unsigned int prep_key_mem_ecc_req15 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_rpu_mem_err_insert_5;

typedef union {
    struct {
        unsigned int prep_key_mem_ecc_req16 : 2; /* [1:0] */
        unsigned int prep_key_mem_ecc_req17 : 2; /* [3:2] */
        unsigned int prep_key_mem_ecc_req18 : 2; /* [5:4] */
        unsigned int prep_key_mem_ecc_req19 : 2; /* [7:6] */
        unsigned int prep_key_mem_ecc_req20 : 2; /* [9:8] */
        unsigned int prep_key_mem_ecc_req21 : 2; /* [11:10] */
        unsigned int prep_key_mem_ecc_req22 : 2; /* [13:12] */
        unsigned int prep_key_mem_ecc_req23 : 2; /* [15:14] */
        unsigned int prep_key_mem_ecc_req24 : 2; /* [17:16] */
        unsigned int prep_key_mem_ecc_req25 : 2; /* [19:18] */
        unsigned int prep_key_mem_ecc_req26 : 2; /* [21:20] */
        unsigned int prep_key_mem_ecc_req27 : 2; /* [23:22] */
        unsigned int prep_key_mem_ecc_req28 : 2; /* [25:24] */
        unsigned int prep_key_mem_ecc_req29 : 2; /* [27:26] */
        unsigned int prep_key_mem_ecc_req30 : 2; /* [29:28] */
        unsigned int prep_key_mem_ecc_req31 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_rpu_mem_err_insert_6;

typedef union {
    struct {
        unsigned int gro_ctx_mem_ecc_req : 2; /* [1:0] */
        unsigned int gro_buf_mem_ecc_req : 2; /* [3:2] */
        unsigned int rsv : 28;                /* [31:4] */
    } bits;

    unsigned int value;
} u_rpu_mem_err_insert_7;

typedef union {
    struct {
        unsigned int cfg_fsm_timeout_thd : 24; /* [23:0] */
        unsigned int rsv : 8;                  /* [31:24] */
    } bits;

    unsigned int value;
} u_rpu_fsm_heat_timeout;

typedef union {
    struct {
        unsigned int cfg_buf_req_repeat_num : 16; /* [15:0] */
        unsigned int rsv : 16;                    /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_buf_empty_cfg;

typedef union {
    struct {
        unsigned int cfg_cnt_clr_ce : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_rpu_cnt_cyc_clr_ctrl;

typedef union {
    struct {
        unsigned int cfg_single_tnl_faster_en : 1; /* [0] */
        unsigned int rsv : 31;                     /* [31:1] */
    } bits;

    unsigned int value;
} u_rpu_single_tnl_faster_en;

typedef union {
    struct {
        unsigned int rpu_ns_lock : 16; /* [15:0] */
        unsigned int rsv : 16;         /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_ns_ctrl_reg;

typedef union {
    struct {
        unsigned int cfg_ok_pulse : 1; /* [0] */
        unsigned int rsv : 31;         /* [31:1] */
    } bits;

    unsigned int value;
} u_rpu_cfg_ok_reg;

typedef union {
    struct {
        unsigned int cfg_buf_req_drop_mode : 1;  /* [0] */
        unsigned int cfg_descpt_endian_ctrl : 2; /* [2:1] */
        unsigned int rsv0 : 1;                   /* [3] */
        unsigned int cfg_buf_req_interval : 16;  /* [19:4] */
        unsigned int rsv1 : 12;                  /* [31:20] */
    } bits;

    unsigned int value;
} u_rpu_tnl_ctrl;

typedef union {
    struct {
        unsigned int cfg_non_jsf_tag23_16 : 8; /* [7:0] */
        unsigned int rsv : 24;                 /* [31:8] */
    } bits;

    unsigned int value;
} u_rpu_non_jsf_tag_cfg;

typedef union {
    struct {
        unsigned int cfg_inf_gating_en : 1;       /* [0] */
        unsigned int cfg_ctrl_gating_en : 1;      /* [1] */
        unsigned int cfg_axi_gating_en : 1;       /* [2] */
        unsigned int cfg_rw_reg_gating_en : 1;    /* [3] */
        unsigned int cfg_store_gating_en : 1;     /* [4] */
        unsigned int cfg_pa_gating_en : 1;        /* [5] */
        unsigned int cfg_test_port_gating_en : 1; /* [6] */
        unsigned int cfg_rpu_gating_en : 1;       /* [7] */
        unsigned int rsv : 24;                    /* [31:8] */
    } bits;

    unsigned int value;
} u_rpu_clk_gating_ctrl;

typedef union {
    struct {
        unsigned int cfg_gating_win : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_gating_window;

typedef union {
    struct {
        unsigned int clk_store_gating_vld : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_store_gating_sts;

typedef union {
    struct {
        unsigned int clk_pa_gating_vld : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_pa_gating_sts;

typedef union {
    struct {
        unsigned int clk_inf_gating_vld : 1;       /* [0] */
        unsigned int clk_ctrl_gating_vld : 1;      /* [1] */
        unsigned int clk_axi_dbw0_gating_vld : 1;  /* [2] */
        unsigned int clk_axi_dbw1_gating_vld : 1;  /* [3] */
        unsigned int clk_rw_reg_gating_vld : 1;    /* [4] */
        unsigned int clk_test_port_gating_vld : 1; /* [5] */
        unsigned int clk_rpu_gating_vld : 1;       /* [6] */
        unsigned int rsv : 25;                     /* [31:7] */
    } bits;

    unsigned int value;
} u_rpu_other_gating_sts;

typedef union {
    struct {
        unsigned int clk_rw_cfg_gating_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_rw_cfg_gating_cnt;

typedef union {
    struct {
        unsigned int clk_axi_dbw1_gating_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_dbw1_gating_cnt;

typedef union {
    struct {
        unsigned int clk_axi_dbw0_gating_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_dbw0_gating_cnt;

typedef union {
    struct {
        unsigned int clk_ctrl_gating_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_ctrl_gating_cnt;

typedef union {
    struct {
        unsigned int clk_inf_gating_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_inf_gating_cnt;

typedef union {
    struct {
        unsigned int clk_test_gating_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_test_gating_cnt;

typedef union {
    struct {
        unsigned int clk_rpu_gating_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_gating_cnt;

typedef union {
    struct {
        unsigned int cfg_pre_buf_req_fifo_thd : 4; /* [3:0] */
        unsigned int cfg_buf_order_fifo_thd : 3;   /* [6:4] */
        unsigned int cfg_prep_out_fifo_thd : 2;    /* [8:7] */
        unsigned int rsv : 23;                     /* [31:9] */
    } bits;

    unsigned int value;
} u_rpu_prep_fifo_thd;

typedef union {
    struct {
        unsigned int cfg_axi_info_fifo_thd : 5; /* [4:0] */
        unsigned int rsv0 : 11;                 /* [15:5] */
        unsigned int cfg_poe_data_fifo_thd : 6; /* [21:16] */
        unsigned int cfg_poe_cmd_fifo_thd : 6;  /* [27:22] */
        unsigned int rsv1 : 4;                  /* [31:28] */
    } bits;

    unsigned int value;
} u_rpu_wr_axi_fifo_thd0;

typedef union {
    struct {
        unsigned int cfg_ddr_ostd_fifo_thd : 7; /* [6:0] */
        unsigned int rsv0 : 1;                  /* [7] */
        unsigned int cfg_poe_ostd_fifo_thd : 6; /* [13:8] */
        unsigned int rsv1 : 18;                 /* [31:14] */
    } bits;

    unsigned int value;
} u_rpu_wr_axi_fifo_thd1;

typedef union {
    struct {
        unsigned int cfg_fna_choose_fa_invert : 1; /* [0] */
        unsigned int rsv : 31;                     /* [31:1] */
    } bits;

    unsigned int value;
} u_rpu_ddr_user_fna_sel;

typedef union {
    struct {
        unsigned int cfg_ddr_awcache : 4; /* [3:0] */
        unsigned int cfg_hac_awcache : 4; /* [7:4] */
        unsigned int cfg_ddr_arcache : 4; /* [11:8] */
        unsigned int cfg_hac_arcache : 4; /* [15:12] */
        unsigned int rsv : 16;            /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_axcachec_cfg;

typedef union {
    struct {
        unsigned int cfg_ddr_awqos : 4; /* [3:0] */
        unsigned int cfg_hac_awqos : 4; /* [7:4] */
        unsigned int cfg_ddr_arqos : 4; /* [11:8] */
        unsigned int cfg_hac_arqos : 4; /* [15:12] */
        unsigned int rsv : 16;          /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_axqos_cfg;

typedef union {
    struct {
        unsigned int cfg_ddr_w_fa : 1;       /* [0] */
        unsigned int cfg_ddr_w_fna : 1;      /* [1] */
        unsigned int cfg_ddr_w_rinvld : 1;   /* [2] */
        unsigned int cfg_ddr_w_fp : 1;       /* [3] */
        unsigned int cfg_ddr_w_bd_type : 4;  /* [7:4] */
        unsigned int cfg_ddr_w_ply_type : 4; /* [11:8] */
        unsigned int cfg_ddr_w_snpattr : 1;  /* [12] */
        unsigned int cfg_ddr_w_pmg : 2;      /* [14:13] */
        unsigned int cfg_ddr_w_seq_type : 2; /* [16:15] */
        unsigned int cfg_ddr_w_partid : 8;   /* [24:17] */
        unsigned int rsv : 7;                /* [31:25] */
    } bits;

    unsigned int value;
} u_rpu_ddr_user_property;

typedef union {
    struct {
        unsigned int cfg_user_sset : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_rpu_user_sset;

typedef union {
    struct {
        unsigned int cfg_user_streamid : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_user_streamid;

typedef union {
    struct {
        unsigned int cfg_ddr_w_cmd_type : 3;  /* [2:0] */
        unsigned int cfg_acc_w_cmd_type : 3;  /* [5:3] */
        unsigned int cfg_test_w_cmd_type : 3; /* [8:6] */
        unsigned int cfg_ts_w_cmd_type : 3;   /* [11:9] */
        unsigned int rsv : 20;                /* [31:12] */
    } bits;

    unsigned int value;
} u_rpu_user_cmd_type;

typedef union {
    struct {
        unsigned int cfg_ring_bd_merge_age_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_bd_merge_age_cnt;

typedef union {
    struct {
        unsigned int cfg_rx_bd_num_wl : 6; /* [5:0] */
        unsigned int rsv : 26;             /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_bd_num_wl;

typedef union {
    struct {
        unsigned int cfg_pf_eco_cfg0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_pf_eco_cfg0;

typedef union {
    struct {
        unsigned int cfg_pf_eco_cfg1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_pf_eco_cfg1;

typedef union {
    struct {
        unsigned int cfg_pf_eco_cfg2 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_pf_eco_cfg2;

typedef union {
    struct {
        unsigned int pre_buf_req_fifo_cnt : 5; /* [4:0] */
        unsigned int rsv0 : 15;                /* [19:5] */
        unsigned int rsv1 : 5;                 /* [24:20] */
        unsigned int buf_order_fifo_cnt : 7;   /* [31:25] */
    } bits;

    unsigned int value;
} u_rpu_prep_fifo_cnt0;

typedef union {
    struct {
        unsigned int prep_pkt_fifo_cnt : 6;  /* [5:0] */
        unsigned int rsv0 : 2;               /* [7:6] */
        unsigned int prep_info_fifo_cnt : 5; /* [12:8] */
        unsigned int rsv1 : 3;               /* [15:13] */
        unsigned int rsv2 : 16;              /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_prep_fifo_cnt1;

typedef union {
    struct {
        unsigned int axi_data_fifo_cnt : 7; /* [6:0] */
        unsigned int rsv0 : 1;              /* [7] */
        unsigned int axi_info_fifo_cnt : 6; /* [13:8] */
        unsigned int rsv1 : 2;              /* [15:14] */
        unsigned int ddr_ostd_fifo_cnt : 8; /* [23:16] */
        unsigned int poe_ostd_fifo_cnt : 7; /* [30:24] */
        unsigned int rsv2 : 1;              /* [31] */
    } bits;

    unsigned int value;
} u_rpu_wr_axi_fifo_cnt0;

typedef union {
    struct {
        unsigned int poe_cmd_fifo_cnt : 7;  /* [6:0] */
        unsigned int rsv0 : 1;              /* [7] */
        unsigned int poe_data_fifo_cnt : 7; /* [14:8] */
        unsigned int rsv1 : 1;              /* [15] */
        unsigned int rsv2 : 16;             /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_wr_axi_fifo_cnt1;

typedef union {
    struct {
        unsigned int prep_out_fifo_cnt : 3;     /* [2:0] */
        unsigned int rsv0 : 13;                 /* [15:3] */
        unsigned int test_bd_data_fifo_cnt : 4; /* [19:16] */
        unsigned int test_bd_info_fifo_cnt : 4; /* [23:20] */
        unsigned int test_bd_ostd_fifo_cnt : 5; /* [28:24] */
        unsigned int rsv1 : 3;                  /* [31:29] */
    } bits;

    unsigned int value;
} u_rpu_wr_axi_fifo_cnt2;

typedef union {
    struct {
        unsigned int mf_data_fifo_cnt : 8; /* [7:0] */
        unsigned int mf_info_fifo_cnt : 8; /* [15:8] */
        unsigned int rsv : 16;             /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_mf_fifo_cnt0;

typedef union {
    struct {
        unsigned int mf_aw_ddr_fifo_cnt : 8; /* [7:0] */
        unsigned int mf_ostd_fifo_cnt : 8;   /* [15:8] */
        unsigned int rsv : 16;               /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_mf_fifo_cnt1;

typedef union {
    struct {
        unsigned int prep_pkt_fifo_sts : 2;  /* [1:0] */
        unsigned int prep_info_fifo_sts : 2; /* [3:2] */
        unsigned int prep_key_fifo_sts : 2;  /* [5:4] */
        unsigned int rsv : 26;               /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_prep_fifo_status0;

typedef union {
    struct {
        unsigned int pre_buf_req_fifo_sts : 2; /* [1:0] */
        unsigned int buf_order_fifo_sts : 2;   /* [3:2] */
        unsigned int rsv : 28;                 /* [31:4] */
    } bits;

    unsigned int value;
} u_rpu_prep_fifo_status1;

typedef union {
    struct {
        unsigned int axi_data_fifo_sts : 2; /* [1:0] */
        unsigned int axi_info_fifo_sts : 2; /* [3:2] */
        unsigned int poe_cmd_fifo_sts : 2;  /* [5:4] */
        unsigned int poe_data_fifo_sts : 2; /* [7:6] */
        unsigned int poe_ostd_fifo_sts : 2; /* [9:8] */
        unsigned int ddr_ostd_fifo_sts : 2; /* [11:10] */
        unsigned int rsv : 20;              /* [31:12] */
    } bits;

    unsigned int value;
} u_rpu_wr_axi_fifo_status0;

typedef union {
    struct {
        unsigned int test_bd_data_fifo_sts : 2; /* [1:0] */
        unsigned int test_bd_info_fifo_sts : 2; /* [3:2] */
        unsigned int test_bd_ostd_fifo_sts : 2; /* [5:4] */
        unsigned int stash_sch_fifo_sts : 2;    /* [7:6] */
        unsigned int prep_out_fifo_sts : 2;     /* [9:8] */
        unsigned int rsv : 22;                  /* [31:10] */
    } bits;

    unsigned int value;
} u_rpu_wr_axi_fifo_status1;

typedef union {
    struct {
        unsigned int mf_data_fifo_sts : 2; /* [1:0] */
        unsigned int mf_info_fifo_sts : 2; /* [3:2] */
        unsigned int mf_aw_fifo_sts : 2;   /* [5:4] */
        unsigned int mf_ostd_fifo_sts : 2; /* [7:6] */
        unsigned int rsv : 24;             /* [31:8] */
    } bits;

    unsigned int value;
} u_rpu_mf_fifo_status;

typedef union {
    struct {
        unsigned int tx_1588_msg_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_tx_1588_cnt;

typedef union {
    struct {
        unsigned int wr_dbw0_axi_bp_cnt : 16; /* [15:0] */
        unsigned int rsv : 16;                /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_wr_axi_dbw0_bp_cnt;

typedef union {
    struct {
        unsigned int wr_dbw1_axi_bp_cnt : 16; /* [15:0] */
        unsigned int rsv : 16;                /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_wr_axi_dbw1_bp_cnt;

typedef union {
    struct {
        unsigned int mf_input_pkt_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_mf_input_cnt;

typedef union {
    struct {
        unsigned int mf_payload_err_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_mf_payload_err_cnt;

typedef union {
    struct {
        unsigned int mf_err_drop_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_mf_err_drop_cnt;

typedef union {
    struct {
        unsigned int qid_miss_drop_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_q_miss_drop_cnt;

typedef union {
    struct {
        unsigned int all_input_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_all_input_cnt;

typedef union {
    struct {
        unsigned int all_dbw0_poe_pd_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_all_dbw0_poe_pd_cnt;

typedef union {
    struct {
        unsigned int all_dbw0_poe_pkt_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_all_dbw0_poe_pkt_cnt;

typedef union {
    struct {
        unsigned int all_dbw1_poe_pd_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_all_dbw1_poe_pd_cnt;

typedef union {
    struct {
        unsigned int all_dbw1_poe_pkt_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_all_dbw1_poe_pkt_cnt;

typedef union {
    struct {
        unsigned int send_rcb_fbd_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_send_rcb_fbd_cnt;

typedef union {
    struct {
        unsigned int send_rcb_fe_bd_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_send_rcb_fe_cnt;

typedef union {
    struct {
        unsigned int send_rcb_pkt_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_send_rcb_pkt_cnt;

typedef union {
    struct {
        unsigned int mf_out_pkt_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_mf_out_pkt_cnt;

typedef union {
    struct {
        unsigned int test_timeout_int_cnt : 16; /* [15:0] */
        unsigned int rsv : 16;                  /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_test_timeout_int_cnt;

typedef union {
    struct {
        unsigned int test_port_pkt_wr_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_test_port_pkt_cnt;

typedef union {
    struct {
        unsigned int test_vld_int_cnt : 16; /* [15:0] */
        unsigned int rsv : 16;              /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_test_vld_int_cnt;

typedef union {
    struct {
        unsigned int all_single_pkt_has_recv : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_interface_status0;

typedef union {
    struct {
        unsigned int rpu_mam_buf_req : 1; /* [0] */
        unsigned int rsv : 31;            /* [31:1] */
    } bits;

    unsigned int value;
} u_rpu_interface_status1;

typedef union {
    struct {
        unsigned int safe_flush_ready : 1;      /* [0] */
        unsigned int safe_flush_en_sts : 1;     /* [1] */
        unsigned int dbw0_wr_ddr_idle : 1;      /* [2] */
        unsigned int dbw1_wr_ddr_idle : 1;      /* [3] */
        unsigned int up_send_poe_idle : 1;      /* [4] */
        unsigned int down_send_poe_idle : 1;    /* [5] */
        unsigned int test_wr_axi_idle : 1;      /* [6] */
        unsigned int ctrl_prep_curr_bp_sts : 1; /* [7] */
        unsigned int wr_axi0_curr_bp_sts : 1;   /* [8] */
        unsigned int wr_axi1_curr_bp_sts : 1;   /* [9] */
        unsigned int rpu_idle_sts : 1;          /* [10] */
        unsigned int rcb_bd_merge_full : 1;     /* [11] */
        unsigned int rcb_bd_merge_empty : 1;    /* [12] */
        unsigned int rsv : 19;                  /* [31:13] */
    } bits;

    unsigned int value;
} u_rpu_inner_status0;

typedef union {
    struct {
        unsigned int recb_flow_sts_0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_recb_flow_sts0;

typedef union {
    struct {
        unsigned int recb_flow_sts_1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_recb_flow_sts1;

typedef union {
    struct {
        unsigned int recb_flow_sts_2 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_recb_flow_sts2;

typedef union {
    struct {
        unsigned int recb_flow_sts_3 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_recb_flow_sts3;

typedef union {
    struct {
        unsigned int prep_list_cpt_sts : 8; /* [7:0] */
        unsigned int rsv : 24;              /* [31:8] */
    } bits;

    unsigned int value;
} u_rpu_prep_list_cpt_sts;

typedef union {
    struct {
        unsigned int prep_list_use_sts : 8; /* [7:0] */
        unsigned int rsv : 24;              /* [31:8] */
    } bits;

    unsigned int value;
} u_rpu_prep_list_use_sts;

typedef union {
    struct {
        unsigned int buf_req_curr_state : 9; /* [8:0] */
        unsigned int rsv : 23;               /* [31:9] */
    } bits;

    unsigned int value;
} u_rpu_fsm_status;

typedef union {
    struct {
        unsigned int perf_dbw0_poe_pkt_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_perf_dbw0_poe_pkt_cnt;

typedef union {
    struct {
        unsigned int perf_dbw1_poe_pkt_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_perf_dbw1_poe_pkt_cnt;

typedef union {
    struct {
        u64 cfg_bus_dly_port : 4; /* [3:0] */
        u64 rsv : 28;             /* [31:4] */
    } bits;

    u64 value;
} u_rpu_perf_bus_delay_cfg;

typedef union {
    struct {
        u64 perf_bus_dly_cnt0 : 48; /* [47:0] */
        u64 rsv : 16;               /* [63:48] */
    } bits;

    u64 value;
} u_rpu_perf_bus_delay_cnt0;

typedef union {
    struct {
        u64 perf_bus_dly_cnt1 : 48; /* [47:0] */
        u64 rsv : 16;               /* [63:48] */
    } bits;

    u64 value;
} u_rpu_perf_bus_delay_cnt1;

typedef union {
    struct {
        u64 perf_bus_dly_cnt2 : 48; /* [47:0] */
        u64 rsv : 16;               /* [63:48] */
    } bits;

    u64 value;
} u_rpu_perf_bus_delay_cnt2;

typedef union {
    struct {
        u64 perf_bus_dly_cnt3 : 48; /* [47:0] */
        u64 rsv : 16;               /* [63:48] */
    } bits;

    u64 value;
} u_rpu_perf_bus_delay_cnt3;

typedef union {
    struct {
        u64 perf_bus_dly_cnt4 : 48; /* [47:0] */
        u64 rsv : 16;               /* [63:48] */
    } bits;

    u64 value;
} u_rpu_perf_bus_delay_cnt4;

typedef union {
    struct {
        u64 perf_bus_dly_cnt5 : 48; /* [47:0] */
        u64 rsv : 16;               /* [63:48] */
    } bits;

    u64 value;
} u_rpu_perf_bus_delay_cnt5;

typedef union {
    struct {
        u64 perf_bus_dly_cnt6 : 48; /* [47:0] */
        u64 rsv : 16;               /* [63:48] */
    } bits;

    u64 value;
} u_rpu_perf_bus_delay_cnt6;

typedef union {
    struct {
        u64 perf_bus_dly_cnt7 : 48; /* [47:0] */
        u64 rsv : 16;               /* [63:48] */
    } bits;

    u64 value;
} u_rpu_perf_bus_delay_cnt7;

typedef union {
    struct {
        unsigned int ring_safe_flush_en : 1;    /* [0] */
        unsigned int ring_safe_flush_ready : 1; /* [1] */
        unsigned int rsv : 30;                  /* [31:2] */
    } bits;

    unsigned int value;
} u_rpu_ring_flush_sts;

typedef union {
    struct {
        unsigned int tnl_undone_pkt_snap_cnt : 16; /* [15:0] */
        unsigned int tnl_ring_flush_ready : 1;     /* [16] */
        unsigned int rsv : 15;                     /* [31:17] */
    } bits;

    unsigned int value;
} u_rpu_ring_flush_snap_cnt;

typedef union {
    struct {
        unsigned int buf_aban_int_en : 1;    /* [0] */
        unsigned int buf_empty_int_en : 1;   /* [1] */
        unsigned int buf_len_err_int_en : 1; /* [2] */
        unsigned int fw_tnl_cfg_int_en : 1;  /* [3] */
        unsigned int mf_debug_int_en : 1;    /* [4] */
        unsigned int y1731_cfg_int_en : 1;   /* [5] */
        unsigned int rsv : 26;               /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_mode_int_enable;

typedef union {
    struct {
        unsigned int buf_aban_int_sts : 1;    /* [0] */
        unsigned int buf_empty_int_sts : 1;   /* [1] */
        unsigned int buf_len_err_int_sts : 1; /* [2] */
        unsigned int fw_tnl_cfg_int_sts : 1;  /* [3] */
        unsigned int mf_debug_int_sts : 1;    /* [4] */
        unsigned int y1731_cfg_int_sts : 1;   /* [5] */
        unsigned int rsv : 26;                /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_mode_int_status;

typedef union {
    struct {
        unsigned int buf_aban_int_set : 1;    /* [0] */
        unsigned int buf_empty_int_set : 1;   /* [1] */
        unsigned int buf_len_err_int_set : 1; /* [2] */
        unsigned int fw_tnl_cfg_int_set : 1;  /* [3] */
        unsigned int mf_debug_int_set : 1;    /* [4] */
        unsigned int y1731_cfg_int_set : 1;   /* [5] */
        unsigned int rsv : 26;                /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_mode_int_set;

typedef union {
    struct {
        unsigned int buf_aban_int_src : 1;    /* [0] */
        unsigned int buf_empty_int_src : 1;   /* [1] */
        unsigned int buf_len_err_int_src : 1; /* [2] */
        unsigned int fw_tnl_cfg_int_src : 1;  /* [3] */
        unsigned int mf_debug_int_src : 1;    /* [4] */
        unsigned int y1731_cfg_int_src : 1;   /* [5] */
        unsigned int rsv : 26;                /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_mode_int_source;

typedef union {
    struct {
        unsigned int buf_req_int_info : 15; /* [14:0] */
        unsigned int rsv : 17;              /* [31:15] */
    } bits;

    unsigned int value;
} u_rpu_buf_req_int_info;

typedef union {
    struct {
        unsigned int buf_aban_int_type : 2;    /* [1:0] */
        unsigned int buf_empty_int_type : 2;   /* [3:2] */
        unsigned int buf_len_err_int_type : 2; /* [5:4] */
        unsigned int fw_tnl_cfg_int_type : 2;  /* [7:6] */
        unsigned int mf_debug_int_type : 2;    /* [9:8] */
        unsigned int y1731_cfg_int_type : 2;   /* [11:10] */
        unsigned int rsv : 20;                 /* [31:12] */
    } bits;

    unsigned int value;
} u_rpu_mode_int_type;

typedef union {
    struct {
        unsigned int buf_aban_int_ce : 1;    /* [0] */
        unsigned int buf_empty_int_ce : 1;   /* [1] */
        unsigned int buf_len_err_int_ce : 1; /* [2] */
        unsigned int fw_tnl_cfg_int_ce : 1;  /* [3] */
        unsigned int mf_debug_int_ce : 1;    /* [4] */
        unsigned int y1731_cfg_int_ce : 1;   /* [5] */
        unsigned int rsv : 26;               /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_mode_int_ce;

typedef union {
    struct {
        unsigned int buf_aban_int_nfe : 1;    /* [0] */
        unsigned int buf_empty_int_nfe : 1;   /* [1] */
        unsigned int buf_len_err_int_nfe : 1; /* [2] */
        unsigned int cfg_fw_tnl_int_nfe : 1;  /* [3] */
        unsigned int mf_debug_int_nfe : 1;    /* [4] */
        unsigned int cfg_y1731_int_nfe : 1;   /* [5] */
        unsigned int rsv : 26;                /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_mode_int_nfe;

typedef union {
    struct {
        unsigned int buf_aban_int_fe : 1;    /* [0] */
        unsigned int buf_empty_int_fe : 1;   /* [1] */
        unsigned int buf_len_err_int_fe : 1; /* [2] */
        unsigned int fw_tnl_cfg_int_fe : 1;  /* [3] */
        unsigned int mf_debug_int_fe : 1;    /* [4] */
        unsigned int y1731_cfg_int_fe : 1;   /* [5] */
        unsigned int rsv : 26;               /* [31:6] */
    } bits;

    unsigned int value;
} u_rpu_mode_int_fe;

typedef union {
    struct {
        unsigned int cfg_vf_eco_cfg0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_vf_eco_cfg0;

typedef union {
    struct {
        unsigned int cfg_vf_eco_cfg1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_vf_eco_cfg1;

typedef union {
    struct {
        unsigned int cfg_vf_eco_cfg2 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_vf_eco_cfg2;

typedef union {
    struct {
        unsigned int qos_pkt_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_qos_pkt_cnt;

typedef union {
    struct {
        unsigned int recv_all_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_all_pkt_cnt;

typedef union {
    struct {
        unsigned int recv_ok_pkt_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_ok_pkt_cnt;

typedef union {
    struct {
        unsigned int mst0_send_poe_pd_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_mst0_send_poe_pd_cnt;

typedef union {
    struct {
        unsigned int mst0_send_poe_pkt_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_mst0_send_poe_pkt_cnt;

typedef union {
    struct {
        unsigned int mst1_send_poe_pd_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_mst1_send_poe_pd_cnt;

typedef union {
    struct {
        unsigned int mst1_send_poe_pkt_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_mst1_send_poe_pkt_cnt;

typedef union {
    struct {
        unsigned int empty_buf_cnt : 16; /* [15:0] */
        unsigned int rsv : 16;           /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_empty_int_cnt;

typedef union {
    struct {
        unsigned int buf_drop_int_cnt : 16; /* [15:0] */
        unsigned int rsv : 16;              /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_buf_drop_int_cnt;

typedef union {
    struct {
        unsigned int buf_disable_int_cnt : 16; /* [15:0] */
        unsigned int rsv : 16;                 /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_buf_disable_int_cnt;

typedef union {
    struct {
        unsigned int buf_len_zero_cnt : 16; /* [15:0] */
        unsigned int rsv : 16;              /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_buf_len_zero_int_cnt;

typedef union {
    struct {
        unsigned int empty_drop_pkt_cnt : 16; /* [15:0] */
        unsigned int rsv : 16;                /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_empty_drop_pkt_cnt;

typedef union {
    struct {
        unsigned int recv_err_pkt_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rpu_recv_err_pkt_cnt;

typedef union {
    struct {
        unsigned int buf_len_err_cnt : 16; /* [15:0] */
        unsigned int rsv : 16;             /* [31:16] */
    } bits;

    unsigned int value;
} u_rpu_buf_len_err_int_cnt;

#endif  // __REG_RPU_H__
