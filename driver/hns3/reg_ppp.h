
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_ppp
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_PPP_H__
#define __REG_PPP_H__

typedef union {
    struct {
        unsigned int fifo_uf_int_en : 1; /* [0] */
        unsigned int fifo_of_int_en : 1; /* [1] */
        unsigned int rsv : 30;           /* [31:2] */
    } bits;

    unsigned int value;
} u_ppp_part_int_enable;

typedef union {
    struct {
        unsigned int rsv0 : 12;                     /* [11:0] */
        unsigned int fd_tcam_ecc_mem_sb_int_en : 1; /* [12] */
        unsigned int rsv1 : 19;                     /* [31:13] */
    } bits;

    unsigned int value;
} u_ppp_mem_sbit_ecc_enable;

typedef union {
    struct {
        unsigned int rsv0 : 12;                     /* [11:0] */
        unsigned int fd_tcam_ecc_mem_mb_int_en : 1; /* [12] */
        unsigned int rsv1 : 19;                     /* [31:13] */
    } bits;

    unsigned int value;
} u_ppp_mem_mbit_ecc_enable;

typedef union {
    struct {
        unsigned int fifo_uf_rint_sts : 1; /* [0] */
        unsigned int fifo_of_rint_sts : 1; /* [1] */
        unsigned int rsv : 30;             /* [31:2] */
    } bits;

    unsigned int value;
} u_ppp_part_int_status;

typedef union {
    struct {
        unsigned int rsv0 : 12;                       /* [11:0] */
        unsigned int fd_tcam_ecc_mem_sb_rint_sts : 1; /* [12] */
        unsigned int rsv1 : 19;                       /* [31:13] */
    } bits;

    unsigned int value;
} u_ppp_mem_sbit_ecc_int_status;

typedef union {
    struct {
        unsigned int rsv0 : 12;                       /* [11:0] */
        unsigned int fd_tcam_ecc_mem_mb_rint_sts : 1; /* [12] */
        unsigned int rsv1 : 19;                       /* [31:13] */
    } bits;

    unsigned int value;
} u_ppp_mem_mbit_ecc_int_status;

typedef union {
    struct {
        unsigned int fifo0_uf_int_set : 1; /* [0] */
        unsigned int fifo1_uf_int_set : 1; /* [1] */
        unsigned int rsv0 : 14;            /* [15:2] */
        unsigned int fifo0_of_int_set : 1; /* [16] */
        unsigned int fifo1_of_int_set : 1; /* [17] */
        unsigned int rsv1 : 14;            /* [31:18] */
    } bits;

    unsigned int value;
} u_ppp_part_int_set;

typedef union {
    struct {
        unsigned int rsv0 : 12;                      /* [11:0] */
        unsigned int fd_tcam_ecc_mem_sb_err_req : 1; /* [12] */
        unsigned int rsv1 : 19;                      /* [31:13] */
    } bits;

    unsigned int value;
} u_ppp_mem_sbit_ecc_req;

typedef union {
    struct {
        unsigned int rsv0 : 12;                      /* [11:0] */
        unsigned int fd_tcam_ecc_mem_mb_err_req : 1; /* [12] */
        unsigned int rsv1 : 19;                      /* [31:13] */
    } bits;

    unsigned int value;
} u_ppp_mem_mbit_ecc_req;

typedef union {
    struct {
        unsigned int fifo_uf_int_src : 1; /* [0] */
        unsigned int fifo_of_int_src : 1; /* [1] */
        unsigned int rsv : 30;            /* [31:2] */
    } bits;

    unsigned int value;
} u_ppp_part_int_source;

typedef union {
    struct {
        unsigned int rsv0 : 12;                      /* [11:0] */
        unsigned int fd_tcam_ecc_mem_sb_int_src : 1; /* [12] */
        unsigned int rsv1 : 19;                      /* [31:13] */
    } bits;

    unsigned int value;
} u_ppp_mem_sbit_ecc_int_source;

typedef union {
    struct {
        unsigned int rsv0 : 12;                      /* [11:0] */
        unsigned int fd_tcam_ecc_mem_mb_int_src : 1; /* [12] */
        unsigned int rsv1 : 19;                      /* [31:13] */
    } bits;

    unsigned int value;
} u_ppp_mem_mbit_ecc_int_source;

typedef union {
    struct {
        unsigned int fifo_uf_bm : 2; /* [1:0] */
        unsigned int rsv : 30;       /* [31:2] */
    } bits;

    unsigned int value;
} u_ppp_fifo_uf_int_info;

typedef union {
    struct {
        unsigned int fifo_of_bm : 2; /* [1:0] */
        unsigned int rsv : 30;       /* [31:2] */
    } bits;

    unsigned int value;
} u_ppp_fifo_of_int_info;

typedef union {
    struct {
        unsigned int ppp_ram_1b_err_addr : 16; /* [15:0] */
        unsigned int rsv : 16;                 /* [31:16] */
    } bits;

    unsigned int value;
} u_ppp_mem_sbit_err_info;

typedef union {
    struct {
        unsigned int ppp_ram_mb_err_addr : 16; /* [15:0] */
        unsigned int rsv : 16;                 /* [31:16] */
    } bits;

    unsigned int value;
} u_ppp_mem_mbit_err_info;

typedef union {
    struct {
        unsigned int ppp_mem_mb_int_type : 2; /* [1:0] */
        unsigned int ppp_mem_sb_int_type : 2; /* [3:2] */
        unsigned int ppp_fifo_int_type : 2;   /* [5:4] */
        unsigned int rsv : 26;                /* [31:6] */
    } bits;

    unsigned int value;
} u_ppp_int_type;

typedef union {
    struct {
        unsigned int fifo_uf_rint_ce : 1; /* [0] */
        unsigned int fifo_of_rint_ce : 1; /* [1] */
        unsigned int rsv : 30;            /* [31:2] */
    } bits;

    unsigned int value;
} u_ppp_part_rint_ce;

typedef union {
    struct {
        unsigned int rsv0 : 12;                      /* [11:0] */
        unsigned int fd_tcam_ecc_mem_sb_rint_ce : 1; /* [12] */
        unsigned int rsv1 : 19;                      /* [31:13] */
    } bits;

    unsigned int value;
} u_ppp_mem_sbit_ecc_rint_ce;

typedef union {
    struct {
        unsigned int rsv0 : 12;                      /* [11:0] */
        unsigned int fd_tcam_ecc_mem_mb_rint_ce : 1; /* [12] */
        unsigned int rsv1 : 19;                      /* [31:13] */
    } bits;

    unsigned int value;
} u_ppp_mem_mbit_ecc_rint_ce;

typedef union {
    struct {
        unsigned int fifo_uf_rint_nfe : 1; /* [0] */
        unsigned int fifo_of_rint_nfe : 1; /* [1] */
        unsigned int rsv : 30;             /* [31:2] */
    } bits;

    unsigned int value;
} u_ppp_part_rint_nfe;

typedef union {
    struct {
        unsigned int rsv0 : 12;                       /* [11:0] */
        unsigned int fd_tcam_ecc_mem_sb_rint_nfe : 1; /* [12] */
        unsigned int rsv1 : 19;                       /* [31:13] */
    } bits;

    unsigned int value;
} u_ppp_mem_sbit_ecc_rint_nfe;

typedef union {
    struct {
        unsigned int rsv0 : 12;                       /* [11:0] */
        unsigned int fd_tcam_ecc_mem_mb_rint_nfe : 1; /* [12] */
        unsigned int rsv1 : 19;                       /* [31:13] */
    } bits;

    unsigned int value;
} u_ppp_mem_mbit_ecc_rint_nfe;

typedef union {
    struct {
        unsigned int fifo_uf_rint_fe : 1; /* [0] */
        unsigned int fifo_of_rint_fe : 1; /* [1] */
        unsigned int rsv : 30;            /* [31:2] */
    } bits;

    unsigned int value;
} u_ppp_part_rint_fe;

typedef union {
    struct {
        unsigned int rsv0 : 12;                      /* [11:0] */
        unsigned int fd_tcam_ecc_mem_sb_rint_fe : 1; /* [12] */
        unsigned int rsv1 : 19;                      /* [31:13] */
    } bits;

    unsigned int value;
} u_ppp_mem_sbit_ecc_rint_fe;

typedef union {
    struct {
        unsigned int rsv0 : 12;                      /* [11:0] */
        unsigned int fd_tcam_ecc_mem_mb_rint_fe : 1; /* [12] */
        unsigned int rsv1 : 19;                      /* [31:13] */
    } bits;

    unsigned int value;
} u_ppp_mem_mbit_ecc_rint_fe;

typedef union {
    struct {
        unsigned int cfg_sbit_ecc_int_thd : 8; /* [7:0] */
        unsigned int rsv : 24;                 /* [31:8] */
    } bits;

    unsigned int value;
} u_ppp_mem_sbit_ecc_thd;

typedef union {
    struct {
        unsigned int ppp_mem_sbit_err_cnt : 16; /* [15:0] */
        unsigned int rsv : 16;                  /* [31:16] */
    } bits;

    unsigned int value;
} u_ppp_mem_sbit_err_cnt;

typedef union {
    struct {
        unsigned int ppp_mem_mbit_err_cnt : 16; /* [15:0] */
        unsigned int rsv : 16;                  /* [31:16] */
    } bits;

    unsigned int value;
} u_ppp_mem_mbit_err_cnt;

typedef union {
    struct {
        unsigned int ppp_mem_init_start : 1; /* [0] */
        unsigned int rsv : 31;               /* [31:1] */
    } bits;

    unsigned int value;
} u_ppp_mem_init_ctrl;

typedef union {
    struct {
        unsigned int ppp_mem_init_done : 1; /* [0] */
        unsigned int rsv : 31;              /* [31:1] */
    } bits;

    unsigned int value;
} u_ppp_mem_init_status;

typedef union {
    struct {
        unsigned int ppp_sp_ram_tmod : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_spmem_ctrl_cfg;

typedef union {
    struct {
        unsigned int ppp_tp_ram_tmod : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_tpmem_ctrl_cfg;

typedef union {
    struct {
        unsigned int ppp_mem_power_mode : 4; /* [3:0] */
        unsigned int rsv : 28;               /* [31:4] */
    } bits;

    unsigned int value;
} u_ppp_mem_power_ctrl_cfg;

typedef union {
    struct {
        unsigned int ppp_tcam_cell_tselr : 3; /* [2:0] */
        unsigned int ppp_tcam_cell_tselw : 2; /* [4:3] */
        unsigned int ppp_tcam_tmod_tm : 1;    /* [5] */
        unsigned int ppp_tcam_tmod_twdt : 1;  /* [6] */
        unsigned int ppp_tcam_tmod_trdt : 1;  /* [7] */
        unsigned int ppp_tcam_tmod_tra : 2;   /* [9:8] */
        unsigned int ppp_tcam_tmod_kp : 2;    /* [11:10] */
        unsigned int rsv : 20;                /* [31:12] */
    } bits;

    unsigned int value;
} u_ppp_tcam_cfg;

typedef union {
    struct {
        unsigned int cfg_ring_mode : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_ppp_mode_cfg;

typedef union {
    struct {
        unsigned int cnt_clr_ce : 1; /* [0] */
        unsigned int rsv : 31;       /* [31:1] */
    } bits;

    unsigned int value;
} u_ppp_cnt_clr_ce;

typedef union {
    struct {
        unsigned int cfg_ppp_auto_gate_en : 1; /* [0] */
        unsigned int rsv0 : 4;                 /* [4:1] */
        unsigned int cfg_fd_tcam_gate_en : 1;  /* [5] */
        unsigned int cfg_fd_lgc_gate_en : 1;   /* [6] */
        unsigned int rsv1 : 9;                 /* [15:7] */
        unsigned int cfg_ppp_gating_th : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_ppp_auto_gate_en;

typedef union {
    struct {
        unsigned int cfg_ppp_gating_win : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_gating_window;

typedef union {
    struct {
        unsigned int ppp_gating_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_gating_sts;

typedef union {
    struct {
        unsigned int cfg_req_rx_aful_thrd : 4; /* [3:0] */
        unsigned int rsv0 : 4;                 /* [7:4] */
        unsigned int cfg_req_tx_aful_thrd : 4; /* [11:8] */
        unsigned int rsv1 : 20;                /* [31:12] */
    } bits;

    unsigned int value;
} u_ppp_req_fifo_cfg;

typedef union {
    struct {
        unsigned int cfg_req_rx_weight : 4; /* [3:0] */
        unsigned int rsv0 : 4;              /* [7:4] */
        unsigned int cfg_req_tx_weight : 4; /* [11:8] */
        unsigned int rsv1 : 20;             /* [31:12] */
    } bits;

    unsigned int value;
} u_ppp_req_wrr_weight;

typedef union {
    struct {
        unsigned int rsv0 : 8;                      /* [7:0] */
        unsigned int cfg_sc_pkt_vlan_handle_en : 1; /* [8] */
        unsigned int rsv1 : 23;                     /* [31:9] */
    } bits;

    unsigned int value;
} u_ppp_glb_misc_cfg;

typedef union {
    struct {
        unsigned int cfg_tuple_data_driven : 1; /* [0] */
        unsigned int rsv : 31;                  /* [31:1] */
    } bits;

    unsigned int value;
} u_ppp_tuple_data_driven;

typedef union {
    struct {
        unsigned int cfg_fd_bs_period : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fd_tcam_bs_period_cfg;

typedef union {
    struct {
        unsigned int cfg_fd_bs_en : 1;           /* [0] */
        unsigned int rsv : 15;                   /* [15:1] */
        unsigned int cfg_fd_bs_step_period : 16; /* [31:16] */
    } bits;

    unsigned int value;
} u_ppp_fd_tcam_bs_cfg;

typedef union {
    struct {
        unsigned int cfg_fd_bit_mode : 2; /* [1:0] */
        unsigned int rsv : 30;            /* [31:2] */
    } bits;

    unsigned int value;
} u_ppp_tcam_mode;

typedef union {
    struct {
        unsigned int cfg_rss_tc_size : 3;   /* [2:0] */
        unsigned int rsv0 : 1;              /* [3] */
        unsigned int cfg_rss_tc_offset : 4; /* [7:4] */
        unsigned int rsv1 : 7;              /* [14:8] */
        unsigned int cfg_rss_tc_valid : 1;  /* [15] */
        unsigned int rsv2 : 16;             /* [31:16] */
    } bits;

    unsigned int value;
} u_ppp_rss_tc_mode_data;

typedef union {
    struct {
        unsigned int cfg_rss_tc_mode_cmd : 1;    /* [0] */
        unsigned int rsv0 : 2;                   /* [2:1] */
        unsigned int cfg_rss_tc_mode_status : 1; /* [3] */
        unsigned int cfg_rss_tc_mode_cmd_en : 1; /* [4] */
        unsigned int rsv1 : 11;                  /* [15:5] */
        unsigned int cfg_rss_tc_mode_tc_id : 3;  /* [18:16] */
        unsigned int rsv2 : 5;                   /* [23:19] */
        unsigned int cfg_rss_tc_mode_fun_id : 2; /* [25:24] */
        unsigned int rsv3 : 6;                   /* [31:26] */
    } bits;

    unsigned int value;
} u_ppp_rss_tc_mode_cfg;

typedef union {
    struct {
        unsigned int cfg_pkt_tc_ovrd_data0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_pkt_tc_ovrd_data0;

typedef union {
    struct {
        unsigned int cfg_pkt_tc_ovrd_data1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_pkt_tc_ovrd_data1;

typedef union {
    struct {
        unsigned int cfg_idt_block_offset_data : 9; /* [8:0] */
        unsigned int rsv : 23;                      /* [31:9] */
    } bits;

    unsigned int value;
} u_ppp_rss_idt_alloc_data;

typedef union {
    struct {
        unsigned int cfg_pkt_tc_ovrd_cmd : 1;    /* [0] */
        unsigned int rsv0 : 2;                   /* [2:1] */
        unsigned int cfg_pkt_tc_ovrd_status : 1; /* [3] */
        unsigned int cfg_pkt_tc_ovrd_cmd_en : 1; /* [4] */
        unsigned int rsv1 : 19;                  /* [23:5] */
        unsigned int cfg_pkt_tc_ovrd_fun_id : 2; /* [25:24] */
        unsigned int rsv2 : 6;                   /* [31:26] */
    } bits;

    unsigned int value;
} u_ppp_pkt_tc_ovrd_cfg;

typedef union {
    struct {
        unsigned int cfg_rss_idt_qid : 4;     /* [3:0] */
        unsigned int rsv0 : 12;               /* [15:4] */
        unsigned int cfg_rss_idt_qid_vld : 1; /* [16] */
        unsigned int rsv1 : 15;               /* [31:17] */
    } bits;

    unsigned int value;
} u_ppp_rss_idt_data;

typedef union {
    struct {
        unsigned int cfg_rss_idt_cmd : 1;    /* [0] */
        unsigned int rsv0 : 2;               /* [2:1] */
        unsigned int cfg_rss_idt_status : 1; /* [3] */
        unsigned int cfg_rss_idt_cmd_en : 1; /* [4] */
        unsigned int rsv1 : 11;              /* [15:5] */
        unsigned int cfg_rss_idt_offset : 4; /* [19:16] */
        unsigned int rsv2 : 12;              /* [31:20] */
    } bits;

    unsigned int value;
} u_ppp_rss_idt_cfg;

typedef union {
    struct {
        unsigned int cfg_rss_non_ip_en : 2; /* [1:0] */
        unsigned int rsv : 30;              /* [31:2] */
    } bits;

    unsigned int value;
} u_ppp_rss_type_nip;

typedef union {
    struct {
        unsigned int cfg_rss_tcpipv4_en : 4;  /* [3:0] */
        unsigned int cfg_rss_udpipv4_en : 4;  /* [7:4] */
        unsigned int cfg_rss_sctpipv4_en : 5; /* [12:8] */
        unsigned int cfg_rss_ipv4_en : 4;     /* [16:13] */
        unsigned int rsv : 15;                /* [31:17] */
    } bits;

    unsigned int value;
} u_ppp_rss_type_ipv4;

typedef union {
    struct {
        unsigned int cfg_rss_tcpipv6_en : 4;  /* [3:0] */
        unsigned int cfg_rss_udpipv6_en : 4;  /* [7:4] */
        unsigned int cfg_rss_sctpipv6_en : 5; /* [12:8] */
        unsigned int cfg_rss_ipv6_en : 4;     /* [16:13] */
        unsigned int rsv : 15;                /* [31:17] */
    } bits;

    unsigned int value;
} u_ppp_rss_type_ipv6;

typedef union {
    struct {
        unsigned int cfg_rss_hash_key0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_rss_hash_key0;

typedef union {
    struct {
        unsigned int cfg_rss_hash_key1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_rss_hash_key1;

typedef union {
    struct {
        unsigned int cfg_rss_hash_key2 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_rss_hash_key2;

typedef union {
    struct {
        unsigned int cfg_rss_hash_key3 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_rss_hash_key3;

typedef union {
    struct {
        unsigned int cfg_rss_hash_key4 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_rss_hash_key4;

typedef union {
    struct {
        unsigned int cfg_rss_hash_key5 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_rss_hash_key5;

typedef union {
    struct {
        unsigned int cfg_rss_hash_key6 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_rss_hash_key6;

typedef union {
    struct {
        unsigned int cfg_rss_hash_key7 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_rss_hash_key7;

typedef union {
    struct {
        unsigned int cfg_rss_hash_key8 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_rss_hash_key8;

typedef union {
    struct {
        unsigned int cfg_rss_hash_key9 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_rss_hash_key9;

typedef union {
    struct {
        unsigned int cfg_rss_hash_algo : 2;       /* [1:0] */
        unsigned int cfg_gro_en : 1;              /* [2] */
        unsigned int cfg_rss_xor_hash_fd_mth : 1; /* [3] */
        unsigned int cfg_rss_xor_lsb_en : 1;      /* [4] */
        unsigned int cfg_rss_sym_sel : 1;         /* [5] */
        unsigned int rsv : 26;                    /* [31:6] */
    } bits;

    unsigned int value;
} u_ppp_rss_hash_algo;

typedef union {
    struct {
        unsigned int cfg_rss_hash_cmd : 1;    /* [0] */
        unsigned int rsv0 : 2;                /* [2:1] */
        unsigned int cfg_rss_hash_status : 1; /* [3] */
        unsigned int cfg_rss_hash_cmd_en : 1; /* [4] */
        unsigned int rsv1 : 11;               /* [15:5] */
        unsigned int cfg_rss_hash_fun_id : 2; /* [17:16] */
        unsigned int rsv2 : 14;               /* [31:18] */
    } bits;

    unsigned int value;
} u_ppp_rss_hash_cfg;

typedef union {
    struct {
        unsigned int cfg_fd_key_templ_data0 : 4; /* [3:0] */
        unsigned int rsv : 28;                   /* [31:4] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_templ_data0;

typedef union {
    struct {
        unsigned int rsv : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_templ_data1;

typedef union {
    struct {
        unsigned int cfg_fd_key_templ_data2 : 8; /* [7:0] */
        unsigned int rsv : 24;                   /* [31:8] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_templ_data2;

typedef union {
    struct {
        unsigned int cfg_fd_key_templ_num0 : 3; /* [2:0] */
        unsigned int rsv : 29;                  /* [31:3] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_templ_data3;

typedef union {
    struct {
        unsigned int cfg_fd_key_templ_cmd : 1;    /* [0] */
        unsigned int rsv0 : 2;                    /* [2:1] */
        unsigned int cfg_fd_key_templ_status : 1; /* [3] */
        unsigned int cfg_fd_key_templ_cmd_en : 1; /* [4] */
        unsigned int rsv1 : 11;                   /* [15:5] */
        unsigned int cfg_fd_key_templ_addr : 3;   /* [18:16] */
        unsigned int rsv2 : 4;                    /* [22:19] */
        unsigned int rsv3 : 9;                    /* [31:23] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_templ_cfg;

typedef union {
    struct {
        unsigned int cfg_fd_key_sel_tdata0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_sel_tdata0;

typedef union {
    struct {
        unsigned int cfg_fd_key_sel_tdata1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_sel_tdata1;

typedef union {
    struct {
        unsigned int cfg_fd_key_sel_mdata : 10; /* [9:0] */
        unsigned int rsv : 22;                  /* [31:10] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_sel_mdata;

typedef union {
    struct {
        unsigned int cfg_fd_key_sel_cmd : 1;    /* [0] */
        unsigned int rsv0 : 2;                  /* [2:1] */
        unsigned int cfg_fd_key_sel_status : 1; /* [3] */
        unsigned int cfg_fd_key_sel_cmd_en : 1; /* [4] */
        unsigned int rsv1 : 11;                 /* [15:5] */
        unsigned int cfg_fd_key_sel_addr : 3;   /* [18:16] */
        unsigned int rsv2 : 13;                 /* [31:19] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_sel_cfg;

typedef union {
    struct {
        unsigned int cfg_fd_key_data0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_data0;

typedef union {
    struct {
        unsigned int cfg_fd_key_data1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_data1;

typedef union {
    struct {
        unsigned int cfg_fd_key_data2 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_data2;

typedef union {
    struct {
        unsigned int cfg_fd_key_data3 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_data3;

typedef union {
    struct {
        unsigned int cfg_fd_key_data4 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_data4;

typedef union {
    struct {
        unsigned int cfg_fd_key_data5 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_data5;

typedef union {
    struct {
        unsigned int cfg_fd_key_data6 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_data6;

typedef union {
    struct {
        unsigned int cfg_fd_key_data7 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_data7;

typedef union {
    struct {
        unsigned int cfg_fd_key_data8 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_data8;

typedef union {
    struct {
        unsigned int cfg_fd_key_data9 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_data9;

typedef union {
    struct {
        unsigned int cfg_fd_key_data10 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_data10;

typedef union {
    struct {
        unsigned int cfg_fd_key_data11 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_data11;

typedef union {
    struct {
        unsigned int cfg_fd_key_data_msb : 16; /* [15:0] */
        unsigned int cfg_fd_key_x_vld : 1;     /* [16] */
        unsigned int rsv : 15;                 /* [31:17] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_data_msb;

typedef union {
    struct {
        unsigned int cfg_fd_key_cmd : 2;    /* [1:0] */
        unsigned int rsv0 : 1;              /* [2] */
        unsigned int cfg_fd_key_status : 1; /* [3] */
        unsigned int cfg_fd_key_cmd_en : 1; /* [4] */
        unsigned int rsv1 : 1;              /* [5] */
        unsigned int cfg_fd_key_x : 1;      /* [6] */
        unsigned int rsv2 : 1;              /* [7] */
        unsigned int cfg_fd_key_ad_sel : 1; /* [8] */
        unsigned int rsv3 : 7;              /* [15:9] */
        unsigned int cfg_fd_key_addr : 8;   /* [23:16] */
        unsigned int rsv4 : 8;              /* [31:24] */
    } bits;

    unsigned int value;
} u_ppp_fd_key_cfg;

typedef union {
    struct {
        unsigned int fd_hit_idx : 8;  /* [7:0] */
        unsigned int rsv0 : 8;        /* [15:8] */
        unsigned int fd_hit_flag : 1; /* [16] */
        unsigned int rsv1 : 15;       /* [31:17] */
    } bits;

    unsigned int value;
} u_ppp_fd_status;

typedef union {
    struct {
        unsigned int cfg_ing_vlan_data0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_ing_vlan_data0;

typedef union {
    struct {
        unsigned int cfg_ing_vlan_data1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_ing_vlan_data1;

typedef union {
    struct {
        unsigned int cfg_ing_vlan_cmd : 1;    /* [0] */
        unsigned int rsv0 : 2;                /* [2:1] */
        unsigned int cfg_ing_vlan_status : 1; /* [3] */
        unsigned int cfg_ing_vlan_cmd_en : 1; /* [4] */
        unsigned int rsv1 : 11;               /* [15:5] */
        unsigned int cfg_ing_vlan_addr : 2;   /* [17:16] */
        unsigned int rsv2 : 14;               /* [31:18] */
    } bits;

    unsigned int value;
} u_ppp_ing_vlan_cfg;

typedef union {
    struct {
        unsigned int cfg_tp2nupm : 24; /* [23:0] */
        unsigned int rsv : 8;          /* [31:24] */
    } bits;

    unsigned int value;
} u_ppp_up_mapping_data;

typedef union {
    struct {
        unsigned int cfg_upmap_cmd : 1;    /* [0] */
        unsigned int rsv0 : 2;             /* [2:1] */
        unsigned int cfg_upmap_status : 1; /* [3] */
        unsigned int cfg_upmap_cmd_en : 1; /* [4] */
        unsigned int rsv1 : 11;            /* [15:5] */
        unsigned int cfg_upmap_addr : 2;   /* [17:16] */
        unsigned int rsv2 : 14;            /* [31:18] */
    } bits;

    unsigned int value;
} u_ppp_up_mapping_cfg;

typedef union {
    struct {
        unsigned int cfg_ti2oupm : 24; /* [23:0] */
        unsigned int rsv : 8;          /* [31:24] */
    } bits;

    unsigned int value;
} u_ppp_up_i2o_mapping_data;

typedef union {
    struct {
        unsigned int cfg_pps_ir : 20; /* [19:0] */
        unsigned int rsv : 12;        /* [31:20] */
    } bits;

    unsigned int value;
} u_ppp_pps_car_cfg0;

typedef union {
    struct {
        unsigned int cfg_pps_bs : 20; /* [19:0] */
        unsigned int rsv : 12;        /* [31:20] */
    } bits;

    unsigned int value;
} u_ppp_pps_car_cfg1;

typedef union {
    struct {
        unsigned int cfg_pps_tk_ini : 1; /* [0] */
        unsigned int cfg_pps_car_en : 1; /* [1] */
        unsigned int rsv0 : 6;           /* [7:2] */
        unsigned int cfg_pps_cycle : 14; /* [21:8] */
        unsigned int rsv1 : 10;          /* [31:22] */
    } bits;

    unsigned int value;
} u_ppp_pps_car_cfg2;

typedef union {
    struct {
        unsigned int cfg_egr_vlan_rsv : 1;              /* [0] */
        unsigned int cfg_egr_vlan_strip_en : 2;         /* [2:1] */
        unsigned int cfg_egr_vlan_strip_sel : 2;        /* [4:3] */
        unsigned int cfg_egr_vlan_strip_discard_en : 2; /* [6:5] */
        unsigned int cfg_egr_vlan_prionly : 2;          /* [8:7] */
        unsigned int cfg_egr_vlan_lkup_strip_en : 2;    /* [10:9] */
        unsigned int rsv : 21;                          /* [31:11] */
    } bits;

    unsigned int value;
} u_ppp_egr_port_attr;

typedef union {
    struct {
        unsigned int cfg_egr_port_attr_cmd : 1;    /* [0] */
        unsigned int rsv0 : 2;                     /* [2:1] */
        unsigned int cfg_egr_port_attr_status : 1; /* [3] */
        unsigned int cfg_egr_port_attr_cmd_en : 1; /* [4] */
        unsigned int rsv1 : 11;                    /* [15:5] */
        unsigned int cfg_egr_port_attr_addr : 2;   /* [17:16] */
        unsigned int rsv2 : 14;                    /* [31:18] */
    } bits;

    unsigned int value;
} u_ppp_egr_port_attr_cfg;

typedef union {
    struct {
        unsigned int rsv0 : 4;                 /* [3:0] */
        unsigned int cfg_mport_fw_lkup_en : 1; /* [4] */
        unsigned int cfg_mport_fw_key_sel : 1; /* [5] */
        unsigned int rsv1 : 26;                /* [31:6] */
    } bits;

    unsigned int value;
} u_ppp_mport_misc_cfg;

typedef union {
    struct {
        unsigned int cfg_tag_v2p_vlan_sel : 2; /* [1:0] */
        unsigned int cfg_tag_i2o_vlan_sel : 2; /* [3:2] */
        unsigned int rsv : 28;                 /* [31:4] */
    } bits;

    unsigned int value;
} u_ppp_up_map_cfg;

typedef union {
    struct {
        unsigned int rsv0 : 4;                 /* [3:0] */
        unsigned int cfg_cport_fw_lkup_en : 1; /* [4] */
        unsigned int cfg_cport_fw_key_sel : 1; /* [5] */
        unsigned int rsv1 : 26;                /* [31:6] */
    } bits;

    unsigned int value;
} u_ppp_tnl_misc_cfg;

typedef union {
    struct {
        unsigned int ppp_rw_eco0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_eco0;

typedef union {
    struct {
        unsigned int ppp_rw_eco1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_eco1;

typedef union {
    struct {
        unsigned int ppp_rw_eco2 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_eco2;

typedef union {
    struct {
        unsigned int ppp_rw_eco3 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_eco3;

typedef union {
    struct {
        u64 ppp_ing_rx_pkt_cnt : 48; /* [47:0] */
        u64 rsv : 16;                /* [63:48] */
    } bits;

    u64 value;
} u_ppp_ing_rx_pkt_cnt;

typedef union {
    struct {
        u64 ppp_ing_tx_pkt_cnt : 48; /* [47:0] */
        u64 rsv : 16;                /* [63:48] */
    } bits;

    u64 value;
} u_ppp_ing_tx_pkt_cnt;

typedef union {
    struct {
        unsigned int ppp_ing_vlan_handle_drop_pkt_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_lsw_ing_vlan_handle_drop_pkt_cnt;

typedef union {
    struct {
        unsigned int cfg_ppp_fd_cnt_status : 1; /* [0] */
        unsigned int cfg_ppp_fd_cnt_cmd_en : 1; /* [1] */
        unsigned int rsv0 : 2;                  /* [3:2] */
        unsigned int ppp_fd_cnt_idx : 5;        /* [8:4] */
        unsigned int rsv1 : 23;                 /* [31:9] */
    } bits;

    unsigned int value;
} u_ppp_fd_cnt_cfg;

typedef union {
    struct {
        u64 ppp_fd_cnt : 48; /* [47:0] */
        u64 rsv : 16;
    } bits;

    u64 value;
} u_ppp_fd_cnt;

typedef union {
    struct {
        unsigned int ppp_car_drop_cnt : 16; /* [15:0] */
        unsigned int rsvd : 16;             /* [31:16] */
    } bits;

    unsigned int value;
} u_ppp_drop_cnt;

typedef union {
    struct {
        u64 ppp_fw_fd_bypass_pkt_cnt : 48; /* [47:0] */
        u64 rsv : 16;                      /* [63:48] */
    } bits;

    u64 value;
} u_ppp_fw_fd_bypass_pkt_cnt;

typedef union {
    struct {
        u64 ppp_fw_fd_stg_1_hit_pkt_cnt : 48; /* [47:0] */
        u64 rsv : 16;                         /* [63:48] */
    } bits;

    u64 value;
} u_ppp_fw_fd_stg_1_hit_pkt_cnt;

typedef union {
    struct {
        u64 ppp_fw_fd_stg_1_miss_pkt_cnt : 48; /* [47:0] */
        u64 rsv : 16;                          /* [63:48] */
    } bits;

    u64 value;
} u_ppp_fw_fd_stg_1_miss_pkt_cnt;

typedef union {
    struct {
        unsigned int ppp_fw_fd_stg_1_hit_drop_pkt_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fw_fd_stg_1_hit_drop_pkt_cnt;

typedef union {
    struct {
        unsigned int ppp_ing_rx_fifo_cnt : 7; /* [6:0] */
        unsigned int ppp_ing_tx_fifo_cnt : 7; /* [13:7] */
        unsigned int rsv : 18;                /* [31:14] */
    } bits;

    unsigned int value;
} u_ppp_ing_fifo_status;

typedef union {
    struct {
        unsigned int cfg_ppp_rlt_rdata_sel : 3; /* [2:0] */
        unsigned int rsv : 29;                  /* [31:3] */
    } bits;

    unsigned int value;
} u_ppp_rlt_status_cfg;

typedef union {
    struct {
        unsigned int ppp_rlt_data : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_rlt_status;

typedef union {
    struct {
        unsigned int cfg_ppp_fw_fd_port_sel : 2;   /* [1:0] */
        unsigned int rsv0 : 3;                     /* [4:2] */
        unsigned int cfg_ppp_fw_fd_dir_sel : 1;    /* [5] */
        unsigned int cfg_ppp_fw_fd_num_sel : 4;    /* [9:6] */
        unsigned int cfg_ppp_fw_fd_key_sel : 1;    /* [10] */
        unsigned int cfg_ppp_fw_fd_tuple_sel : 1;  /* [11] */
        unsigned int rsv1 : 1;                     /* [12] */
        unsigned int cfg_ppp_fw_fd_cap_en : 1;     /* [13] */
        unsigned int cfg_ppp_fw_fd_cap_status : 1; /* [14] */
        unsigned int rsv2 : 17;                    /* [31:15] */
    } bits;

    unsigned int value;
} u_ppp_fw_fd_status_cfg;

typedef union {
    struct {
        unsigned int ppp_fw_fd_key_status : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_fw_fd_status;

typedef union {
    struct {
        unsigned int ppp_fw_fd_stg_1_hit_idx : 8;  /* [7:0] */
        unsigned int rsv0 : 5;                     /* [12:8] */
        unsigned int ppp_fw_fd_stg_1_hit_flag : 1; /* [13] */
        unsigned int rsv1 : 18;                    /* [31:14] */
    } bits;

    unsigned int value;
} u_ppp_fw_fd_hit_status;

typedef union {
    struct {
        unsigned int cfg_fw_tnl_sel : 4; /* [3:0] */
        unsigned int rsv_150 : 28;       /* [31:4] */
    } bits;

    unsigned int value;
} u_ppp_fw_tnl_cnt_cfg;

typedef union {
    struct {
        u64 ppp_fw_tnl_pkt_cnt : 48; /* [47:0] */
        u64 rsv : 16;                /* [63:48] */
    } bits;

    u64 value;
} u_ppp_fw_tnl_pkt_cnt;

#endif  // __REG_PPP_H__