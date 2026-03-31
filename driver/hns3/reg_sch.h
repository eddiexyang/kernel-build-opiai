/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_sch
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_SCH_H__
#define __REG_SCH_H__

typedef union {
    struct {
        unsigned int msc_rw_rsv0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_tm_rw_rsv0;

typedef union {
    struct {
        unsigned int msc_rw_rsv1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_tm_rw_rsv1;

typedef union {
    struct {
        unsigned int msc_rw_rsv2 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_tm_rw_rsv2;

typedef union {
    struct {
        unsigned int msc_rw_rsv3 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_tm_rw_rsv3;

typedef union {
    struct {
        unsigned int sch_cfg_mem_init_start : 1;   /* [0] */
        unsigned int rsv_0 : 1;                    /* [1] */
        unsigned int rsv_1 : 1;                    /* [2] */
        unsigned int rsv_2 : 1;                    /* [3] */
        unsigned int sch_logic_mem_init_start : 1; /* [4] */
        unsigned int rsv_3 : 1;                    /* [5] */
        unsigned int rsv_4 : 1;                    /* [6] */
        unsigned int rsv_5 : 25;                   /* [31:7] */
    } bits;

    unsigned int value;
} u_tm_mem_init_start;

typedef union {
    struct {
        unsigned int sch_cfg_mem_init_done : 1;   /* [0] */
        unsigned int rsv0 : 1;                    /* [1] */
        unsigned int rsv1 : 1;                    /* [2] */
        unsigned int tm_cfg_mem_init_done : 1;    /* [3] */
        unsigned int sch_logic_mem_init_done : 1; /* [4] */
        unsigned int rsv2 : 1;                    /* [5] */
        unsigned int rsv3 : 1;                    /* [6] */
        unsigned int tm_logic_mem_init_done : 1;  /* [7] */
        unsigned int rsv4 : 24;                   /* [31:8] */
    } bits;

    unsigned int value;
} u_tm_mem_init_done;

typedef union {
    struct {
        unsigned int mem_timing_ctrl_sp : 7; /* [6:0] */
        unsigned int rsv0 : 1;               /* [7] */
        unsigned int mem_timing_ctrl_tp : 8; /* [15:8] */
        unsigned int rsv1 : 16;              /* [31:16] */
    } bits;

    unsigned int value;
} u_tm_mem_timing_ctrl;

typedef union {
    struct {
        unsigned int sch_mem_ecc_bypass : 1; /* [0] */
        unsigned int rsv : 31;               /* [31:1] */
    } bits;

    unsigned int value;
} u_tm_mem_ecc_bypass;

typedef union {
    struct {
        unsigned int quanta_value : 7; /* [6:0] */
        unsigned int rsv : 25;         /* [31:7] */
    } bits;

    unsigned int value;
} u_sch_quanta_value;

typedef union {
    struct {
        unsigned int port_shap_bypass : 1; /* [0] */
        unsigned int pg_cshap_bypass : 1;  /* [1] */
        unsigned int pg_pshap_bypass : 1;  /* [2] */
        unsigned int pri_cshap_bypass : 1; /* [3] */
        unsigned int pri_pshap_bypass : 1; /* [4] */
        unsigned int rsv : 27;             /* [31:5] */
    } bits;

    unsigned int value;
} u_sch_shap_bypass;

typedef union {
    struct {
        unsigned int vf_flush_qset_id : 9;  /* [8:0] */
        unsigned int rsv0 : 3;              /* [11:9] */
        unsigned int vf_flush_qset_vld : 1; /* [12] */
        unsigned int rsv1 : 19;             /* [31:13] */
    } bits;

    unsigned int value;
} u_tm_vf_flush_qs_id;

typedef union {
    struct {
        unsigned int tm_vf_flush_done : 1; /* [0] */
        unsigned int rsv : 31;             /* [31:1] */
    } bits;

    unsigned int value;
} u_tm_vf_flush_done;

typedef union {
    struct {
        unsigned int rsv0 : 1;              /* [0] */
        unsigned int sch_vf_flush_done : 1; /* [1] */
        unsigned int rsv1 : 30;             /* [31:2] */
    } bits;

    unsigned int value;
} u_tm_vf_flush_dfx;

typedef union {
    struct {
        unsigned int pri_loopback_len_en : 1;  /* [0] */
        unsigned int pg_loopback_len_en : 1;   /* [1] */
        unsigned int port_loopback_len_en : 1; /* [2] */
        unsigned int pri_coffset_len_en : 1;   /* [3] */
        unsigned int pg_coffset_len_en : 1;    /* [4] */
        unsigned int rsv : 27;                 /* [31:5] */
    } bits;

    unsigned int value;
} u_sch_node_offset_len_ctrl;

typedef union {
    struct {
        unsigned int sch_fifo_int_type : 2; /* [1:0] */
        unsigned int rsv : 30;              /* [31:2] */
    } bits;

    unsigned int value;
} u_sch_fifo_int_type;

typedef union {
    struct {
        unsigned int sch_ecc_sbit_int_type : 2; /* [1:0] */
        unsigned int rsv : 30;                  /* [31:2] */
    } bits;

    unsigned int value;
} u_sch_ecc_sbit_int_type;

typedef union {
    struct {
        unsigned int sch_ecc_mbit_int_type : 2; /* [1:0] */
        unsigned int rsv : 30;                  /* [31:2] */
    } bits;

    unsigned int value;
} u_sch_ecc_mbit_int_type;

typedef union {
    struct {
        unsigned int sch_ecc_intmsk : 2;               /* [1:0] */
        unsigned int port_shap_sub_fifo_intmsk : 2;    /* [3:2] */
        unsigned int pg_pshap_sub_fifo_intmsk : 2;     /* [5:4] */
        unsigned int pg_cshap_sub_fifo_intmsk : 2;     /* [7:6] */
        unsigned int pri_pshap_sub_fifo_intmsk : 2;    /* [9:8] */
        unsigned int pri_cshap_sub_fifo_intmsk : 2;    /* [11:10] */
        unsigned int port_shap_offset_fifo_intmsk : 2; /* [13:12] */
        unsigned int pg_pshap_offset_fifo_intmsk : 2;  /* [15:14] */
        unsigned int pg_cshap_offset_fifo_intmsk : 2;  /* [17:16] */
        unsigned int pri_pshap_offset_fifo_intmsk : 2; /* [19:18] */
        unsigned int pri_cshap_offset_fifo_intmsk : 2; /* [21:20] */
        unsigned int rsv : 2;                          /* [23:22] */
        unsigned int nq_vf_flush_fifo_intmsk : 2;      /* [25:24] */
        unsigned int qset_shap_offset_fifo_intmsk : 2; /* [27:26] */
        unsigned int qset_shap_sub_fifo_intmsk : 2;    /* [29:28] */
        unsigned int ssu_byte_fifo_intmsk : 2;         /* [31:30] */
    } bits;

    unsigned int value;
} u_sch_intmsk;

typedef union {
    struct {
        unsigned int sch_ecc_rint : 2;               /* [1:0] */
        unsigned int port_shap_sub_fifo_rint : 2;    /* [3:2] */
        unsigned int pg_pshap_sub_fifo_rint : 2;     /* [5:4] */
        unsigned int pg_cshap_sub_fifo_rint : 2;     /* [7:6] */
        unsigned int pri_pshap_sub_fifo_rint : 2;    /* [9:8] */
        unsigned int pri_cshap_sub_fifo_rint : 2;    /* [11:10] */
        unsigned int port_shap_offset_fifo_rint : 2; /* [13:12] */
        unsigned int pg_pshap_offset_fifo_rint : 2;  /* [15:14] */
        unsigned int pg_cshap_offset_fifo_rint : 2;  /* [17:16] */
        unsigned int pri_pshap_offset_fifo_rint : 2; /* [19:18] */
        unsigned int pri_cshap_offset_fifo_rint : 2; /* [21:20] */
        unsigned int rsv : 2;                        /* [23:22] */
        unsigned int nq_vf_flush_fifo_rint : 2;      /* [25:24] */
        unsigned int qset_shap_offset_fifo_rint : 2; /* [27:26] */
        unsigned int qset_shap_sub_fifo_rint : 2;    /* [29:28] */
        unsigned int ssu_byte_fifo_rint : 2;         /* [31:30] */
    } bits;

    unsigned int value;
} u_sch_rint;

typedef union {
    struct {
        unsigned int sch_ecc_rint_msix : 2;               /* [1:0] */
        unsigned int port_shap_sub_fifo_rint_msix : 2;    /* [3:2] */
        unsigned int pg_pshap_sub_fifo_rint_msix : 2;     /* [5:4] */
        unsigned int pg_cshap_sub_fifo_rint_msix : 2;     /* [7:6] */
        unsigned int pri_pshap_sub_fifo_rint_msix : 2;    /* [9:8] */
        unsigned int pri_cshap_sub_fifo_rint_msix : 2;    /* [11:10] */
        unsigned int port_shap_offset_fifo_rint_msix : 2; /* [13:12] */
        unsigned int pg_pshap_offset_fifo_rint_msix : 2;  /* [15:14] */
        unsigned int pg_cshap_offset_fifo_rint_msix : 2;  /* [17:16] */
        unsigned int pri_pshap_offset_fifo_rint_msix : 2; /* [19:18] */
        unsigned int pri_cshap_offset_fifo_rint_msix : 2; /* [21:20] */
        unsigned int rsv : 2;                             /* [23:22] */
        unsigned int nq_vf_flush_fifo_rint_msix : 2;      /* [25:24] */
        unsigned int qset_shap_offset_fifo_rint_msix : 2; /* [27:26] */
        unsigned int qset_shap_sub_fifo_rint_msix : 2;    /* [29:28] */
        unsigned int ssu_byte_fifo_rint_msix : 2;         /* [31:30] */
    } bits;

    unsigned int value;
} u_sch_rint_msix;

typedef union {
    struct {
        unsigned int sch_ecc_rint_ce : 2;               /* [1:0] */
        unsigned int port_shap_sub_fifo_rint_ce : 2;    /* [3:2] */
        unsigned int pg_pshap_sub_fifo_rint_ce : 2;     /* [5:4] */
        unsigned int pg_cshap_sub_fifo_rint_ce : 2;     /* [7:6] */
        unsigned int pri_pshap_sub_fifo_rint_ce : 2;    /* [9:8] */
        unsigned int pri_cshap_sub_fifo_rint_ce : 2;    /* [11:10] */
        unsigned int port_shap_offset_fifo_rint_ce : 2; /* [13:12] */
        unsigned int pg_pshap_offset_fifo_rint_ce : 2;  /* [15:14] */
        unsigned int pg_cshap_offset_fifo_rint_ce : 2;  /* [17:16] */
        unsigned int pri_pshap_offset_fifo_rint_ce : 2; /* [19:18] */
        unsigned int pri_cshap_offset_fifo_rint_ce : 2; /* [21:20] */
        unsigned int rsv : 2;                           /* [23:22] */
        unsigned int nq_vf_flush_fifo_rint_ce : 2;      /* [25:24] */
        unsigned int qset_shap_offset_fifo_rint_ce : 2; /* [27:26] */
        unsigned int qset_shap_sub_fifo_rint_ce : 2;    /* [29:28] */
        unsigned int ssu_byte_fifo_rint_ce : 2;         /* [31:30] */
    } bits;

    unsigned int value;
} u_sch_rint_ce;

typedef union {
    struct {
        unsigned int sch_ecc_rint_nfe : 2;               /* [1:0] */
        unsigned int port_shap_sub_fifo_rint_nfe : 2;    /* [3:2] */
        unsigned int pg_pshap_sub_fifo_rint_nfe : 2;     /* [5:4] */
        unsigned int pg_cshap_sub_fifo_rint_nfe : 2;     /* [7:6] */
        unsigned int pri_pshap_sub_fifo_rint_nfe : 2;    /* [9:8] */
        unsigned int pri_cshap_sub_fifo_rint_nfe : 2;    /* [11:10] */
        unsigned int port_shap_offset_fifo_rint_nfe : 2; /* [13:12] */
        unsigned int pg_pshap_offset_fifo_rint_nfe : 2;  /* [15:14] */
        unsigned int pg_cshap_offset_fifo_rint_nfe : 2;  /* [17:16] */
        unsigned int pri_pshap_offset_fifo_rint_nfe : 2; /* [19:18] */
        unsigned int pri_cshap_offset_fifo_rint_nfe : 2; /* [21:20] */
        unsigned int rsv : 2;                            /* [23:22] */
        unsigned int nq_vf_flush_fifo_rint_nfe : 2;      /* [25:24] */
        unsigned int qset_shap_offset_fifo_rint_nfe : 2; /* [27:26] */
        unsigned int qset_shap_sub_fifo_rint_nfe : 2;    /* [29:28] */
        unsigned int ssu_byte_fifo_rint_nfe : 2;         /* [31:30] */
    } bits;

    unsigned int value;
} u_sch_rint_nfe;

typedef union {
    struct {
        unsigned int sch_ecc_rint_fe : 2;               /* [1:0] */
        unsigned int port_shap_sub_fifo_rint_fe : 2;    /* [3:2] */
        unsigned int pg_pshap_sub_fifo_rint_fe : 2;     /* [5:4] */
        unsigned int pg_cshap_sub_fifo_rint_fe : 2;     /* [7:6] */
        unsigned int pri_pshap_sub_fifo_rint_fe : 2;    /* [9:8] */
        unsigned int pri_cshap_sub_fifo_rint_fe : 2;    /* [11:10] */
        unsigned int port_shap_offset_fifo_rint_fe : 2; /* [13:12] */
        unsigned int pg_pshap_offset_fifo_rint_fe : 2;  /* [15:14] */
        unsigned int pg_cshap_offset_fifo_rint_fe : 2;  /* [17:16] */
        unsigned int pri_pshap_offset_fifo_rint_fe : 2; /* [19:18] */
        unsigned int pri_cshap_offset_fifo_rint_fe : 2; /* [21:20] */
        unsigned int rsv : 2;                           /* [23:22] */
        unsigned int nq_vf_flush_fifo_rint_fe : 2;      /* [25:24] */
        unsigned int qset_shap_offset_fifo_rint_fe : 2; /* [27:26] */
        unsigned int qset_shap_sub_fifo_rint_fe : 2;    /* [29:28] */
        unsigned int ssu_byte_fifo_rint_fe : 2;         /* [31:30] */
    } bits;

    unsigned int value;
} u_sch_rint_fe;

typedef union {
    struct {
        unsigned int sch_ram_mbit_ecc_info : 24; /* [23:0] */
        unsigned int rsv : 8;                    /* [31:24] */
    } bits;

    unsigned int value;
} u_sch_ram_mbit_ecc_info;

typedef union {
    struct {
        unsigned int sch_ram_sbit_ecc_info : 24; /* [23:0] */
        unsigned int rsv : 8;                    /* [31:24] */
    } bits;

    unsigned int value;
} u_sch_ram_sbit_ecc_info;

typedef union {
    struct {
        unsigned int sch_ram_err_req0 : 1;  /* [0] */
        unsigned int sch_ram_err_req1 : 1;  /* [1] */
        unsigned int sch_ram_err_req2 : 1;  /* [2] */
        unsigned int sch_ram_err_req3 : 1;  /* [3] */
        unsigned int sch_ram_err_req4 : 1;  /* [4] */
        unsigned int sch_ram_err_req5 : 1;  /* [5] */
        unsigned int sch_ram_err_req6 : 1;  /* [6] */
        unsigned int sch_ram_err_req7 : 1;  /* [7] */
        unsigned int sch_ram_err_req8 : 1;  /* [8] */
        unsigned int sch_ram_err_req9 : 1;  /* [9] */
        unsigned int sch_ram_err_req10 : 1; /* [10] */
        unsigned int sch_ram_err_req11 : 1; /* [11] */
        unsigned int sch_ram_err_req12 : 1; /* [12] */
        unsigned int sch_ram_err_req13 : 1; /* [13] */
        unsigned int sch_ram_err_req14 : 1; /* [14] */
        unsigned int sch_ram_err_req15 : 1; /* [15] */
        unsigned int sch_ram_err_req16 : 1; /* [16] */
        unsigned int sch_ram_err_req17 : 1; /* [17] */
        unsigned int sch_ram_err_req18 : 1; /* [18] */
        unsigned int sch_ram_err_req19 : 1; /* [19] */
        unsigned int sch_ram_err_req20 : 1; /* [20] */
        unsigned int sch_ram_err_req21 : 1; /* [21] */
        unsigned int sch_ram_err_req22 : 1; /* [22] */
        unsigned int sch_ram_err_req23 : 1; /* [23] */
        unsigned int sch_ram_err_req24 : 1; /* [24] */
        unsigned int sch_ram_err_req25 : 1; /* [25] */
        unsigned int sch_ram_err_req26 : 1; /* [26] */
        unsigned int sch_ram_err_req27 : 1; /* [27] */
        unsigned int sch_ram_err_req28 : 1; /* [28] */
        unsigned int sch_ram_err_req29 : 1; /* [29] */
        unsigned int rsv : 2;               /* [31:30] */
    } bits;

    unsigned int value;
} u_sch_ram_err_req0;

typedef union {
    struct {
        unsigned int rsv0 : 1;              /* [0] */
        unsigned int rsv1 : 1;              /* [1] */
        unsigned int rsv2 : 1;              /* [2] */
        unsigned int rsv3 : 1;              /* [3] */
        unsigned int rsv4 : 1;              /* [4] */
        unsigned int rsv5 : 1;              /* [5] */
        unsigned int rsv6 : 1;              /* [6] */
        unsigned int rsv7 : 1;              /* [7] */
        unsigned int sch_ram_err_req38 : 1; /* [8] */
        unsigned int sch_ram_err_req39 : 1; /* [9] */
        unsigned int sch_ram_err_req40 : 1; /* [10] */
        unsigned int sch_ram_err_req41 : 1; /* [11] */
        unsigned int sch_ram_err_req42 : 1; /* [12] */
        unsigned int sch_ram_err_req43 : 1; /* [13] */
        unsigned int sch_ram_err_req44 : 1; /* [14] */
        unsigned int sch_ram_err_req45 : 1; /* [15] */
        unsigned int sch_ram_err_req46 : 1; /* [16] */
        unsigned int sch_ram_err_req47 : 1; /* [17] */
        unsigned int sch_ram_err_req48 : 1; /* [18] */
        unsigned int sch_ram_err_req49 : 1; /* [19] */
        unsigned int rsv8 : 12;             /* [31:20] */
    } bits;

    unsigned int value;
} u_sch_ram_err_req1;

typedef union {
    struct {
        unsigned int sch_ram_err_req50 : 1; /* [0] */
        unsigned int sch_ram_err_req51 : 1; /* [1] */
        unsigned int sch_ram_err_req52 : 1; /* [2] */
        unsigned int sch_ram_err_req53 : 1; /* [3] */
        unsigned int sch_ram_err_req54 : 1; /* [4] */
        unsigned int sch_ram_err_req55 : 1; /* [5] */
        unsigned int sch_ram_err_req56 : 1; /* [6] */
        unsigned int sch_ram_err_req57 : 1; /* [7] */
        unsigned int sch_ram_err_req58 : 1; /* [8] */
        unsigned int sch_ram_err_req59 : 1; /* [9] */
        unsigned int sch_ram_err_req60 : 1; /* [10] */
        unsigned int sch_ram_err_req61 : 1; /* [11] */
        unsigned int sch_ram_err_req62 : 1; /* [12] */
        unsigned int sch_ram_err_req63 : 1; /* [13] */
        unsigned int sch_ram_err_req64 : 1; /* [14] */
        unsigned int sch_ram_err_req65 : 1; /* [15] */
        unsigned int sch_ram_err_req66 : 1; /* [16] */
        unsigned int sch_ram_err_req67 : 1; /* [17] */
        unsigned int sch_ram_err_req68 : 1; /* [18] */
        unsigned int sch_ram_err_req69 : 1; /* [19] */
        unsigned int sch_ram_err_req70 : 1; /* [20] */
        unsigned int sch_ram_err_req71 : 1; /* [21] */
        unsigned int sch_ram_err_req72 : 1; /* [22] */
        unsigned int sch_ram_err_req73 : 1; /* [23] */
        unsigned int sch_ram_err_req74 : 1; /* [24] */
        unsigned int sch_ram_err_req75 : 1; /* [25] */
        unsigned int rsv : 6;               /* [31:26] */
    } bits;

    unsigned int value;
} u_sch_ram_err_req2;

typedef union {
    struct {
        unsigned int pri_pshap_sub_fifo_dfx : 2;    /* [1:0] */
        unsigned int pri_cshap_sub_fifo_dfx : 2;    /* [3:2] */
        unsigned int pg_pshap_sub_fifo_dfx : 2;     /* [5:4] */
        unsigned int pg_cshap_sub_fifo_dfx : 2;     /* [7:6] */
        unsigned int port_shap_sub_fifo_dfx : 2;    /* [9:8] */
        unsigned int pri_pshap_offset_fifo_dfx : 2; /* [11:10] */
        unsigned int pri_cshap_offset_fifo_dfx : 2; /* [13:12] */
        unsigned int pg_pshap_offset_fifo_dfx : 2;  /* [15:14] */
        unsigned int pg_cshap_offset_fifo_dfx : 2;  /* [17:16] */
        unsigned int port_shap_offset_fifo_dfx : 2; /* [19:18] */
        unsigned int rsv0 : 2;                      /* [21:20] */
        unsigned int nq_vf_flush_fifo_dfx : 2;      /* [23:22] */
        unsigned int qset_shap_sub_fifo_dfx : 2;    /* [25:24] */
        unsigned int qset_shap_offset_fifo_dfx : 2; /* [27:26] */
        unsigned int ssu_byte_fifo_dfx : 2;         /* [29:28] */
        unsigned int rsv1 : 2;                      /* [31:30] */
    } bits;

    unsigned int value;
} u_sch_fifo_dfx;

typedef union {
    struct {
        unsigned int sch_qs_dfx_id : 9; /* [8:0] */
        unsigned int rsv : 23;          /* [31:9] */
    } bits;

    unsigned int value;
} u_sch_qset_dfx_id;

typedef union {
    struct {
        unsigned int sch_qs_dfx_sts : 4; /* [3:0] */
        unsigned int rsv : 28;           /* [31:4] */
    } bits;

    unsigned int value;
} u_sch_qset_dfx_sts;

typedef union {
    struct {
        unsigned int sch_pri_dfx_id : 8; /* [7:0] */
        unsigned int rsv : 24;           /* [31:8] */
    } bits;

    unsigned int value;
} u_sch_pri_dfx_id;

typedef union {
    struct {
        unsigned int sch_pri_dfx_sts : 3; /* [2:0] */
        unsigned int rsv : 29;            /* [31:3] */
    } bits;

    unsigned int value;
} u_sch_pri_dfx_sts;

typedef union {
    struct {
        unsigned int sch_pg_dfx_id : 5; /* [4:0] */
        unsigned int rsv : 27;          /* [31:5] */
    } bits;

    unsigned int value;
} u_sch_pg_dfx_id;

typedef union {
    struct {
        unsigned int sch_pg_dfx_sts : 3; /* [2:0] */
        unsigned int rsv : 29;           /* [31:3] */
    } bits;

    unsigned int value;
} u_sch_pg_dfx_sts;

typedef union {
    struct {
        unsigned int sch_port_dfx_id : 5; /* [4:0] */
        unsigned int rsv : 27;            /* [31:5] */
    } bits;

    unsigned int value;
} u_sch_port_dfx_id;

typedef union {
    struct {
        unsigned int sch_port_dfx_sts : 2; /* [1:0] */
        unsigned int rsv : 30;             /* [31:2] */
    } bits;

    unsigned int value;
} u_sch_port_dfx_sts;

typedef union {
    struct {
        unsigned int sch_pri_bp : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_sch_pri_bp;

typedef union {
    struct {
        unsigned int nq_dfx_num : 9; /* [8:0] */
        unsigned int rsv : 23;       /* [31:9] */
    } bits;

    unsigned int value;
} u_sch_nq_dfx_cfg;

typedef union {
    struct {
        unsigned int sch_nq_num_cnt : 20; /* [19:0] */
        unsigned int rsv : 12;            /* [31:20] */
    } bits;

    unsigned int value;
} u_sch_nq_num_cnt;

typedef union {
    struct {
        unsigned int rsv0 : 10; /* [9:0] */
        unsigned int rsv1 : 22; /* [31:10] */
    } bits;

    unsigned int value;
} u_sch_rq_dfx_cfg;

typedef union {
    struct {
        unsigned int rsv0 : 20; /* [19:0] */
        unsigned int rsv1 : 12; /* [31:20] */
    } bits;

    unsigned int value;
} u_sch_rq_num_cnt;

typedef union {
    struct {
        unsigned int sch_ssu_fifo_afull_th : 10; /* [9:0] */
        unsigned int rsv0 : 6;                   /* [15:10] */
        unsigned int rsv1 : 5;                   /* [20:16] */
        unsigned int rsv2 : 11;                  /* [31:21] */
    } bits;

    unsigned int value;
} u_sch_fifo_afull_cfg;

typedef union {
    struct {
        unsigned int shaper_offset_fifo_afull_gap : 3; /* [2:0] */
        unsigned int rsv : 29;                         /* [31:3] */
    } bits;

    unsigned int value;
} u_offset_fifo_afull_cfg;

typedef union {
    struct {
        unsigned int rsv0 : 10; /* [9:0] */
        unsigned int rsv1 : 22; /* [31:10] */
    } bits;

    unsigned int value;
} u_sch_rcb_byteinfo_index;

typedef union {
    struct {
        unsigned int rsv0 : 18; /* [17:0] */
        unsigned int rsv1 : 6;  /* [23:18] */
        unsigned int rsv2 : 1;  /* [24] */
        unsigned int rsv3 : 7;  /* [31:25] */
    } bits;

    unsigned int value;
} u_sch_rcb_index_info;

typedef union {
    struct {
        unsigned int sch_port_mode_old : 1; /* [0] */
        unsigned int rsv : 31;              /* [31:1] */
    } bits;

    unsigned int value;
} u_sch_port_mode;

typedef union {
    struct {
        unsigned int pnet_id : 5; /* [4:0] */
        unsigned int rsv : 27;    /* [31:5] */
    } bits;

    unsigned int value;
} u_sch_pf_link_pnet;

typedef union {
    struct {
        unsigned int sch_nic_sel_port : 5; /* [4:0] */
        unsigned int rsv : 27;             /* [31:5] */
    } bits;

    unsigned int value;
} u_sch_nic_sel_port;

typedef union {
    struct {
        unsigned int sch_nic_num_port : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_sch_nic_num_port;

typedef union {
    struct {
        unsigned int sch_port_mode : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_ppe_sch_mode;

typedef union {
    struct {
        unsigned int port_offset_len : 8; /* [7:0] */
        unsigned int rsv : 24;            /* [31:8] */
    } bits;

    unsigned int value;
} u_sch_port_offset_len;

typedef union {
    struct {
        unsigned int port_weight : 8; /* [7:0] */
        unsigned int rsv : 24;        /* [31:8] */
    } bits;

    unsigned int value;
} u_sch_port_weight;

typedef union {
    struct {
        unsigned int port_shaping_cfg : 26; /* [25:0] */
        unsigned int rsv : 6;               /* [31:26] */
    } bits;

    unsigned int value;
} u_sch_port_shaping;

typedef union {
    struct {
        unsigned int pg_sch_mode : 1; /* [0] */
        unsigned int rsv : 31;        /* [31:1] */
    } bits;

    unsigned int value;
} u_sch_pg_mode;

typedef union {
    struct {
        unsigned int pg_weight : 8; /* [7:0] */
        unsigned int rsv : 24;      /* [31:8] */
    } bits;

    unsigned int value;
} u_sch_pg_weight;

typedef union {
    struct {
        unsigned int pg_cshaping_cfg : 26; /* [25:0] */
        unsigned int rsv : 6;              /* [31:26] */
    } bits;

    unsigned int value;
} u_sch_pg_cshaping;

typedef union {
    struct {
        unsigned int pg_pshaping_cfg : 26; /* [25:0] */
        unsigned int rsv : 6;              /* [31:26] */
    } bits;

    unsigned int value;
} u_sch_pg_pshaping;

typedef union {
    struct {
        unsigned int pg_to_pri_bmp : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_sch_pg_to_pri_maping;

typedef union {
    struct {
        unsigned int qs_sch_mode : 1; /* [0] */
        unsigned int rsv : 31;        /* [31:1] */
    } bits;

    unsigned int value;
} u_sch_qs_mode;

typedef union {
    struct {
        unsigned int qs_id : 9;       /* [8:0] */
        unsigned int qs_link_vld : 1; /* [9] */
        unsigned int rsv : 22;        /* [31:10] */
    } bits;

    unsigned int value;
} u_sch_nq_to_qs_link;

typedef union {
    struct {
        unsigned int pri_id : 8;          /* [7:0] */
        unsigned int qs_pri_link_vld : 1; /* [8] */
        unsigned int rsv : 23;            /* [31:9] */
    } bits;

    unsigned int value;
} u_sch_qs_to_pri_link;

typedef union {
    struct {
        unsigned int qs_weight : 8; /* [7:0] */
        unsigned int rsv : 24;      /* [31:8] */
    } bits;

    unsigned int value;
} u_sch_qs_weight;

typedef union {
    struct {
        unsigned int pri_weight : 8; /* [7:0] */
        unsigned int rsv : 24;       /* [31:8] */
    } bits;

    unsigned int value;
} u_sch_pri_weight;

typedef union {
    struct {
        unsigned int pri_cshaping_cfg : 26; /* [25:0] */
        unsigned int rsv : 6;               /* [31:26] */
    } bits;

    unsigned int value;
} u_sch_pri_cshaping;

typedef union {
    struct {
        unsigned int pri_pshaping_cfg : 26; /* [25:0] */
        unsigned int rsv : 6;               /* [31:26] */
    } bits;

    unsigned int value;
} u_sch_pri_pshaping;

typedef union {
    struct {
        unsigned int pri_sch_mode : 1; /* [0] */
        unsigned int rsv : 31;         /* [31:1] */
    } bits;

    unsigned int value;
} u_sch_pri_mode;

typedef union {
    struct {
        unsigned int qset_maping_vld : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_sch_bp_to_qset_maping;

typedef union {
    struct {
        unsigned int qs_cfg_bp : 1; /* [0] */
        unsigned int rsv : 31;      /* [31:1] */
    } bits;

    unsigned int value;
} u_sch_qs_cfg_bp;

typedef union {
    struct {
        unsigned int qset_ir_cfg : 26; /* [25:0] */
        unsigned int rsv : 6;          /* [31:26] */
    } bits;

    unsigned int value;
} u_tm_qset_shaping_ir;

typedef union {
    struct {
        unsigned int qset_loopback_len_en : 1; /* [0] */
        unsigned int rsv : 31;                 /* [31:1] */
    } bits;

    unsigned int value;
} u_qset_shap_lpbk_cfg;

typedef union {
    struct {
        unsigned int qset_shap_bypass : 1; /* [0] */
        unsigned int rsv : 31;             /* [31:1] */
    } bits;

    unsigned int value;
} u_qset_shap_bypass;

typedef union {
    struct {
        unsigned int qset_shap_pass_index : 9; /* [8:0] */
        unsigned int rsv : 23;                 /* [31:9] */
    } bits;

    unsigned int value;
} u_qcn_shap_index;

typedef union {
    struct {
        unsigned int qset_index_shaping_pass : 1; /* [0] */
        unsigned int rsv : 31;                    /* [31:1] */
    } bits;

    unsigned int value;
} u_qcn_index_shap_pass;

typedef union {
    struct {
        unsigned int tm_offset_info_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_tm_offset_info_cnt;

#endif  // __REG_SCH_H__