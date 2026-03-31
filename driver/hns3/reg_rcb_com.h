/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_rcb_com
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_RCB_COM_H__
#define __REG_RCB_COM_H__

typedef union {
    struct {
        unsigned int rcb_bd_endian : 2; /* [1:0] */
        unsigned int rsv : 30;          /* [31:2] */
    } bits;

    unsigned int value;
} u_rcb_cfg_endian;

typedef union {
    struct {
        unsigned int rcb_init_en : 1; /* [0] */
        unsigned int rsv : 31;        /* [31:1] */
    } bits;

    unsigned int value;
} u_rcb_init_en;

typedef union {
    struct {
        unsigned int rcb_cfg_init_flag : 1; /* [0] */
        unsigned int rsv : 31;              /* [31:1] */
    } bits;

    unsigned int value;
} u_rcb_cfg_init_flag;

typedef union {
    struct {
        unsigned int rcb_axuser_fa : 1;           /* [0] */
        unsigned int rcb_axuser_fna : 1;          /* [1] */
        unsigned int rcb_axuser_cleaninv : 1;     /* [2] */
        unsigned int rcb_axuser_fp : 1;           /* [3] */
        unsigned int rcb_axuser_bd_pkt_type : 4;  /* [7:4] */
        unsigned int rcb_axuser_pkt_pkt_type : 4; /* [11:8] */
        unsigned int rsv : 20;                    /* [31:12] */
    } bits;

    unsigned int value;
} u_rcb_user_cfg0;

typedef union {
    struct {
        unsigned int rcb_axuser_snpattr : 1;     /* [0] */
        unsigned int rcb_axuser_th : 1;          /* [1] */
        unsigned int rcb_axuser_th_dict : 2;     /* [3:2] */
        unsigned int rcb_axuser_bd_part_id : 8;  /* [11:4] */
        unsigned int rcb_axuser_bd_pmg : 2;      /* [13:12] */
        unsigned int rcb_axuser_pkt_part_id : 8; /* [21:14] */
        unsigned int rcb_axuser_pkt_pmg : 2;     /* [23:22] */
        unsigned int rsv : 8;                    /* [31:24] */
    } bits;

    unsigned int value;
} u_rcb_user_cfg1;

typedef union {
    struct {
        unsigned int stash_pkt_wl : 16; /* [15:0] */
        unsigned int rsv : 16;          /* [31:16] */
    } bits;

    unsigned int value;
} u_stash_pkt_wl;

typedef union {
    struct {
        unsigned int fa_pkt_wl : 16; /* [15:0] */
        unsigned int rsv : 16;       /* [31:16] */
    } bits;

    unsigned int value;
} u_fa_pkt_wl;

typedef union {
    struct {
        unsigned int user_fa_sel : 1;    /* [0] */
        unsigned int user_stash_sel : 1; /* [1] */
        unsigned int user_lock_sel : 1;  /* [2] */
        unsigned int rsv : 29;           /* [31:3] */
    } bits;

    unsigned int value;
} u_user_sel;

typedef union {
    struct {
        unsigned int cfg_rcb_arqos : 4; /* [3:0] */
        unsigned int cfg_rcb_awqos : 4; /* [7:4] */
        unsigned int rsv : 24;          /* [31:8] */
    } bits;

    unsigned int value;
} u_rcb_axqos_cfg;

typedef union {
    struct {
        unsigned int cfg_rcb_arcache : 4; /* [3:0] */
        unsigned int cfg_rcb_awcache : 4; /* [7:4] */
        unsigned int rsv : 24;            /* [31:8] */
    } bits;

    unsigned int value;
} u_rcb_axcache_cfg;

typedef union {
    struct {
        unsigned int cfg_rcb_rx_gating_en : 1;  /* [0] */
        unsigned int cfg_rcb_tx_gating_en : 1;  /* [1] */
        unsigned int cfg_rcb_slv_gating_en : 1; /* [2] */
        unsigned int cfg_txsch_gating_en : 1;   /* [3] */
        unsigned int rsv : 28;                  /* [31:4] */
    } bits;

    unsigned int u32;
} u_rcb_clk_gating_ctrl;

typedef union {
    struct {
        unsigned int cfg_gating_win : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rcb_gating_window;

typedef union {
    struct {
        unsigned int clk_rcb_tx_gating_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rcb_tx_gating_cnt;

typedef union {
    struct {
        unsigned int clk_rcb_rx_gating_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rcb_rx_gating_cnt;

typedef union {
    struct {
        unsigned int clk_rcb_slv_gating_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rcb_slv_gating_cnt;

typedef union {
    struct {
        unsigned int clk_rcb_rx_gating_vld : 1;  /* [0] */
        unsigned int clk_rcb_slv_gating_vld : 1; /* [1] */
        unsigned int rsv : 30;                   /* [31:2] */
    } bits;

    unsigned int value;
} u_rcb_gating_sts;

typedef union {
    struct {
        unsigned int rcb_axuser_ssv : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_rcb_user_ssv;

typedef union {
    struct {
        unsigned int rcb_axuser_strmid : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_rcb_user_strmid;

typedef union {
    struct {
        unsigned int tso_mss_min : 14; /* [13:0] */
        unsigned int rsv : 18;         /* [31:14] */
    } bits;

    unsigned int value;
} u_tso_mss_min;

typedef union {
    struct {
        unsigned int tso_mss_max : 14; /* [13:0] */
        unsigned int rsv : 18;         /* [31:14] */
    } bits;

    unsigned int value;
} u_tso_mss_max;

typedef union {
    struct {
        unsigned int ppu_rx_drop_cfg0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppu_rx_drop_cfg0;

typedef union {
    struct {
        unsigned int ppu_to_be_rst : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_ppu_to_be_rst;

typedef union {
    struct {
        unsigned int ppu_could_be_rst : 1; /* [0] */
        unsigned int rsv : 31;             /* [31:1] */
    } bits;

    unsigned int value;
} u_ppu_could_be_rst;

typedef union {
    struct {
        unsigned int ppu_fun_rst_flag : 1; /* [0] */
        unsigned int rsv : 31;             /* [31:1] */
    } bits;

    unsigned int value;
} u_ppu_fun_rst_flag;

typedef union {
    struct {
        unsigned int sp_ram_tmod_rcb : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_sc_mem_ctrl_sp;

typedef union {
    struct {
        unsigned int tp_ram_tmod_rcb : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_sc_mem_ctrl_tp;

typedef union {
    struct {
        unsigned int mem_power_mode_ppu : 8; /* [7:0] */
        unsigned int rsv : 24;               /* [31:8] */
    } bits;

    unsigned int value;
} u_sc_mem_ctrl_power;

typedef union {
    struct {
        unsigned int tcam_tsel : 2; /* [1:0] */
        unsigned int tcam_tset : 2; /* [3:2] */
        unsigned int rsv : 28;      /* [31:4] */
    } bits;

    unsigned int value;
} u_tcam_mem_ctrl;

typedef union {
    struct {
        unsigned int tm_bypass_en : 1; /* [0] */
        unsigned int rsv : 31;         /* [31:1] */
    } bits;

    unsigned int value;
} u_tm_bypass_en;

typedef union {
    struct {
        unsigned int ecc_bypass_en : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_ecc_bypass_en;

typedef union {
    struct {
        unsigned int tcam_ecc_bypass_en : 1; /* [0] */
        unsigned int rsv : 31;               /* [31:1] */
    } bits;

    unsigned int value;
} u_tcam_ecc_bypass_en;

typedef union {
    struct {
        unsigned int cfg_cnt_clr_ce : 1; /* [0] */
        unsigned int snap_en : 1;        /* [1] */
        unsigned int rsv : 30;           /* [31:2] */
    } bits;

    unsigned int value;
} u_cnt_clr_ce;

typedef union {
    struct {
        unsigned int tm_comp_qid : 10; /* [9:0] */
        unsigned int rsv : 22;         /* [31:10] */
    } bits;

    unsigned int value;
} u_tm_comp_qid;

typedef union {
    struct {
        unsigned int tcam_ecc_gen_data : 6; /* [5:0] */
        unsigned int rsv : 26;              /* [31:6] */
    } bits;

    unsigned int value;
} u_rcb_tqp_map_ecc_data_gen;

typedef union {
    struct {
        unsigned int tcam_ecc_chk_data : 6; /* [5:0] */
        unsigned int rsv : 26;              /* [31:6] */
    } bits;

    unsigned int value;
} u_rcb_tqp_map_ecc_data_chk0;

typedef union {
    struct {
        unsigned int tcam_ecc_serr : 1; /* [0] */
        unsigned int tcam_ecc_merr : 1; /* [1] */
        unsigned int rsv : 30;          /* [31:2] */
    } bits;

    unsigned int value;
} u_rcb_tqp_map_ecc_data_chk1;

typedef union {
    struct {
        unsigned int tx_rst_curr_st : 8; /* [7:0] */
        unsigned int rsv0 : 8;           /* [15:8] */
        unsigned int rx_rst_curr_st : 5; /* [20:16] */
        unsigned int rsv1 : 11;          /* [31:21] */
    } bits;

    unsigned int value;
} u_fsm_dfx_st0;

typedef union {
    struct {
        unsigned int rd_fbd_curr_st : 9; /* [8:0] */
        unsigned int rsv : 23;           /* [31:9] */
    } bits;

    unsigned int value;
} u_fsm_dfx_st1;

typedef union {
    struct {
        unsigned int mac0_tpu_curr_st : 3; /* [2:0] */
        unsigned int rsv0 : 1;             /* [3] */
        unsigned int mac1_tpu_curr_st : 3; /* [6:4] */
        unsigned int rsv1 : 1;             /* [7] */
        unsigned int mac2_tpu_curr_st : 3; /* [10:8] */
        unsigned int rsv2 : 1;             /* [11] */
        unsigned int mac3_tpu_curr_st : 3; /* [14:12] */
        unsigned int rsv3 : 1;             /* [15] */
        unsigned int mac4_tpu_curr_st : 3; /* [18:16] */
        unsigned int rsv4 : 1;             /* [19] */
        unsigned int mac5_tpu_curr_st : 3; /* [22:20] */
        unsigned int rsv5 : 1;             /* [23] */
        unsigned int mac6_tpu_curr_st : 3; /* [26:24] */
        unsigned int rsv6 : 1;             /* [27] */
        unsigned int mac7_tpu_curr_st : 3; /* [30:28] */
        unsigned int rsv7 : 1;             /* [31] */
    } bits;

    unsigned int value;
} u_fsm_dfx_st2;

typedef union {
    struct {
        unsigned int ets_fifo_dfx : 18; /* [17:0] */
        unsigned int rsv : 14;          /* [31:18] */
    } bits;

    unsigned int value;
} u_fifo_dfx_st0;

typedef union {
    struct {
        unsigned int rsv : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_fifo_dfx_st1;

typedef union {
    struct {
        unsigned int rsv : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_fifo_dfx_st2;

typedef union {
    struct {
        unsigned int rsv : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_fifo_dfx_st3;

typedef union {
    struct {
        unsigned int rcb_rx_idle_st : 1; /* [0] */
        unsigned int rcb_tx_idle_st : 1; /* [1] */
        unsigned int rsv : 30;           /* [31:2] */
    } bits;

    unsigned int value;
} u_rcb_inner_st;

typedef union {
    struct {
        unsigned int rcb_tx_fbd_fifo_empty : 1; /* [0] */
        unsigned int rsv : 31;                  /* [31:1] */
    } bits;

    unsigned int value;
} u_fifo_dfx_st4;

typedef union {
    struct {
        unsigned int rcb_tx_fbd_fifo_full : 1; /* [0] */
        unsigned int rsv : 31;                 /* [31:1] */
    } bits;

    unsigned int value;
} u_fifo_dfx_st5;

typedef union {
    struct {
        unsigned int q_credit_vld : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_q_credit_vld;

typedef union {
    struct {
        unsigned int rcb_ecc_merr_int_type : 2;     /* [1:0] */
        unsigned int rcb_ecc_serr_int_type : 2;     /* [3:2] */
        unsigned int rd_bus_err_int_type : 2;       /* [5:4] */
        unsigned int wr_bus_err_int_type : 2;       /* [7:6] */
        unsigned int reg_search_miss_int_type : 2;  /* [9:8] */
        unsigned int rx_q_search_miss_int_type : 2; /* [11:10] */
        unsigned int rsv : 20;                      /* [31:12] */
    } bits;

    unsigned int value;
} u_mpf_abnormal_int_type;

typedef union {
    struct {
        unsigned int rcb_ecc_merr_int_en : 1;     /* [0] */
        unsigned int rcb_ecc_serr_int_en : 1;     /* [1] */
        unsigned int rd_bus_err_int_en : 1;       /* [2] */
        unsigned int wr_bus_err_int_en : 1;       /* [3] */
        unsigned int reg_search_miss_int_en : 1;  /* [4] */
        unsigned int rx_q_search_miss_int_en : 1; /* [5] */
        unsigned int rsv : 26;                    /* [31:6] */
    } bits;

    unsigned int value;
} u_mpf_abnormal_int_en;

typedef union {
    struct {
        unsigned int rcb_ecc_merr_int_st : 1;     /* [0] */
        unsigned int rcb_ecc_serr_int_st : 1;     /* [1] */
        unsigned int rd_bus_err_int_st : 1;       /* [2] */
        unsigned int wr_bus_err_int_st : 1;       /* [3] */
        unsigned int reg_search_miss_int_st : 1;  /* [4] */
        unsigned int rx_q_search_miss_int_st : 1; /* [5] */
        unsigned int rsv : 26;                    /* [31:6] */
    } bits;

    unsigned int value;
} u_mpf_abnormal_int_st;

typedef union {
    struct {
        unsigned int rcb_ecc_merr_int_src : 1;     /* [0] */
        unsigned int rcb_ecc_serr_int_src : 1;     /* [1] */
        unsigned int rd_bus_err_int_src : 1;       /* [2] */
        unsigned int wr_bus_err_int_src : 1;       /* [3] */
        unsigned int reg_search_miss_int_src : 1;  /* [4] */
        unsigned int rx_q_search_miss_int_src : 1; /* [5] */
        unsigned int rsv : 26;                     /* [31:6] */
    } bits;

    unsigned int value;
} u_mpf_abnormal_int_src;

typedef union {
    struct {
        unsigned int rcb_ecc_merr_int_set : 1;     /* [0] */
        unsigned int rcb_ecc_serr_int_set : 1;     /* [1] */
        unsigned int rd_bus_err_int_set : 1;       /* [2] */
        unsigned int wr_bus_err_int_set : 1;       /* [3] */
        unsigned int reg_search_miss_int_set : 1;  /* [4] */
        unsigned int rx_q_search_miss_int_set : 1; /* [5] */
        unsigned int rsv : 26;                     /* [31:6] */
    } bits;

    unsigned int value;
} u_mpf_abnormal_int_set;

typedef union {
    struct {
        unsigned int rcb_mem_merr_int_ce_src : 1;     /* [0] */
        unsigned int rcb_mem_serr_int_ce_src : 1;     /* [1] */
        unsigned int rd_bus_err_int_ce_src : 1;       /* [2] */
        unsigned int wr_bus_err_int_ce_src : 1;       /* [3] */
        unsigned int reg_search_miss_int_ce_src : 1;  /* [4] */
        unsigned int rx_q_search_miss_int_ce_src : 1; /* [5] */
        unsigned int rsv : 26;                        /* [31:6] */
    } bits;

    unsigned int value;
} u_mpf_abnormal_int_ce;

typedef union {
    struct {
        unsigned int rcb_mem_merr_int_nfe_src : 1;     /* [0] */
        unsigned int rcb_mem_serr_int_nfe_src : 1;     /* [1] */
        unsigned int rd_bus_err_int_nfe_src : 1;       /* [2] */
        unsigned int wr_bus_err_int_nfe_src : 1;       /* [3] */
        unsigned int reg_search_miss_int_nfe_src : 1;  /* [4] */
        unsigned int rx_q_search_miss_int_nfe_src : 1; /* [5] */
        unsigned int rsv : 26;                         /* [31:6] */
    } bits;

    unsigned int value;
} u_mpf_abnormal_int_nfe;

typedef union {
    struct {
        unsigned int rcb_mem_merr_int_fe_src : 1;     /* [0] */
        unsigned int rcb_mem_serr_int_fe_src : 1;     /* [1] */
        unsigned int rd_bus_err_int_fe_src : 1;       /* [2] */
        unsigned int wr_bus_err_int_fe_src : 1;       /* [3] */
        unsigned int reg_search_miss_int_fe_src : 1;  /* [4] */
        unsigned int rx_q_search_miss_int_fe_src : 1; /* [5] */
        unsigned int rsv : 26;                        /* [31:6] */
    } bits;

    unsigned int value;
} u_mpf_abnormal_int_fe;

typedef union {
    struct {
        unsigned int rcb_mem_ecc_serr_cnt : 16; /* [15:0] */
        unsigned int rsv : 16;                  /* [31:16] */
    } bits;

    unsigned int value;
} u_rcb_mem_serr_cnt;

typedef union {
    struct {
        unsigned int rcb_mem_ecc_merr_cnt : 16; /* [15:0] */
        unsigned int rsv : 16;                  /* [31:16] */
    } bits;

    unsigned int value;
} u_rcb_mem_merr_cnt;

typedef union {
    struct {
        unsigned int rcb_mem_ecc_serr_addr : 16; /* [15:0] */
        unsigned int rcb_mem_ecc_serr_id : 8;    /* [23:16] */
        unsigned int rsv : 8;                    /* [31:24] */
    } bits;

    unsigned int value;
} u_rcb_serr_addr;

typedef union {
    struct {
        unsigned int rcb_mem_ecc_merr_addr : 16; /* [15:0] */
        unsigned int rcb_mem_ecc_merr_id : 8;    /* [23:16] */
        unsigned int rsv : 8;                    /* [31:24] */
    } bits;

    unsigned int value;
} u_rcb_merr_addr;

typedef union {
    struct {
        unsigned int cfg_sbit_ecc_int_thd : 8; /* [7:0] */
        unsigned int rsv : 24;                 /* [31:8] */
    } bits;

    unsigned int value;
} u_rcb_mem_sbit_ecc_thd;

typedef union {
    struct {
        unsigned int reg_search_miss_data : 12; /* [11:0] */
        unsigned int rsv : 20;                  /* [31:12] */
    } bits;

    unsigned int value;
} u_reg_search_miss;

typedef union {
    struct {
        unsigned int rx_q_search_miss_data : 12; /* [11:0] */
        unsigned int rsv : 20;                   /* [31:12] */
    } bits;

    unsigned int value;
} u_rx_q_search_miss;

typedef union {
    struct {
        unsigned int ppu_rw_rsv0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppu_rw_rsv0;

typedef union {
    struct {
        unsigned int ppu_rw_rsv1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppu_rw_rsv1;

typedef union {
    struct {
        unsigned int ppu_rw_rsv2 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppu_rw_rsv2;

typedef union {
    struct {
        unsigned int ppu_rw_rsv3 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppu_rw_rsv3;

typedef union {
    struct {
        unsigned int tso_info_mem_ecc_req : 2;   /* [1:0] */
        unsigned int rsv0 : 6;                   /* [7:2] */
        unsigned int axi_rd_fbd_mem_ecc_req : 2; /* [9:8] */
        unsigned int rsv1 : 6;                   /* [15:10] */
        unsigned int rcb_fbd_mem_ecc_req : 2;    /* [17:16] */
        unsigned int rsv2 : 6;                   /* [23:18] */
        unsigned int txsch_mem_ecc_req : 2;      /* [25:24] */
        unsigned int rsv3 : 6;                   /* [31:26] */
    } bits;

    unsigned int value;
} u_rcb_mem_err_insert;

typedef union {
    struct {
        unsigned int tc_sp_mode_cfg : 16; /* [15:0] */
        unsigned int rsv : 16;            /* [31:16] */
    } bits;

    unsigned int value;
} u_ets_tc_sp_mode;

typedef union {
    struct {
        unsigned int mac_queue_shap_bypass : 1; /* [0] */
        unsigned int mac_tc_shap_bypass : 1;    /* [1] */
        unsigned int mac_tcg_shap_bypass : 1;   /* [2] */
        unsigned int mac_port_shap_bypass : 1;  /* [3] */
        unsigned int rsv : 28;                  /* [31:4] */
    } bits;

    unsigned int value;
} u_ets_shapping_bypass;

typedef union {
    struct {
        unsigned int tcg_sp_mode_cfg : 4; /* [3:0] */
        unsigned int rsv : 28;            /* [31:4] */
    } bits;

    unsigned int value;
} u_ets_tcg_sp_mode;

typedef union {
    struct {
        unsigned int weight_offset_cfg : 4; /* [3:0] */
        unsigned int rsv : 28;              /* [31:4] */
    } bits;

    unsigned int value;
} u_ets_weight_offset;

typedef union {
    struct {
        unsigned int tcg_weight_cfg : 8; /* [7:0] */
        unsigned int rsv : 24;           /* [31:8] */
    } bits;

    unsigned int value;
} u_ets_tcg_weight;

typedef union {
    struct {
        unsigned int tc_weight_cfg : 8; /* [7:0] */
        unsigned int rsv : 24;          /* [31:8] */
    } bits;

    unsigned int value;
} u_ets_tc_weight;

typedef union {
    struct {
        unsigned int tcg_shap_cfg : 26; /* [25:0] */
        unsigned int rsv : 6;           /* [31:26] */
    } bits;

    unsigned int value;
} u_ets_tcg_shaping;

typedef union {
    struct {
        unsigned int tc_shap_cfg : 26; /* [25:0] */
        unsigned int rsv : 6;          /* [31:26] */
    } bits;

    unsigned int value;
} u_ets_tc_shaping;

typedef union {
    struct {
        unsigned int port_maping_cfg : 4; /* [3:0] */
        unsigned int rsv : 28;            /* [31:4] */
    } bits;

    unsigned int value;
} u_ets_port_maping;

typedef union {
    struct {
        unsigned int tcg_maping_cfg : 16; /* [15:0] */
        unsigned int rsv : 16;            /* [31:16] */
    } bits;

    unsigned int value;
} u_ets_tcg_maping;

typedef union {
    struct {
        unsigned int tc_maping_cfg : 16; /* [15:0] */
        unsigned int rsv : 16;           /* [31:16] */
    } bits;

    unsigned int value;
} u_ets_tc_maping;

typedef union {
    struct {
        unsigned int port_shap_cfg : 26; /* [25:0] */
        unsigned int rsv : 6;            /* [31:26] */
    } bits;

    unsigned int value;
} u_ets_port_shaping;

typedef union {
    struct {
        unsigned int queue_shap_cfg : 26; /* [25:0] */
        unsigned int rsv : 6;             /* [31:26] */
    } bits;

    unsigned int value;
} u_ets_queue_shaping;

typedef union {
    struct {
        unsigned int pre_sub_offset_len_cfg : 15; /* [14:0] */
        unsigned int rsv : 17;                    /* [31:15] */
    } bits;

    unsigned int value;
} u_ets_pre_sub_offset_len_cfg;

typedef union {
    struct {
        unsigned int queue_link_tc_cfg : 4; /* [3:0] */
        unsigned int rsv : 28;              /* [31:4] */
    } bits;

    unsigned int value;
} u_ets_queue_link_tc;

typedef union {
    struct {
        unsigned int queue_en_cfg : 1; /* [0] */
        unsigned int rsv : 31;         /* [31:1] */
    } bits;

    unsigned int value;
} u_ets_queue_en_cfg;

typedef union {
    struct {
        unsigned int tc_send_slope_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ets_tc_send_slope;

typedef union {
    struct {
        unsigned int tc_idle_slope_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ets_tc_idle_slope;

typedef union {
    struct {
        unsigned int tc_high_credit_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ets_tc_high_credit;

typedef union {
    struct {
        unsigned int tc_low_credit_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ets_tc_low_credit;

typedef union {
    struct {
        unsigned int tc_cbs_en_cfg : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_ets_tc_cbs_en;

typedef union {
    struct {
        unsigned int tc_speed_sel_cfg : 4; /* [3:0] */
        unsigned int rsv : 28;             /* [31:4] */
    } bits;

    unsigned int value;
} u_ets_tc_speed_sel;

typedef union {
    struct {
        unsigned int queue_flush_req : 1; /* [0] */
        unsigned int rsv : 31;            /* [31:1] */
    } bits;

    unsigned int value;
} u_ets_queue_flush_req;

typedef union {
    struct {
        unsigned int queue_flush_done : 1; /* [0] */
        unsigned int rsv : 31;             /* [31:1] */
    } bits;

    unsigned int value;
} u_ets_queue_flush_done;

typedef union {
    struct {
        unsigned int tm_byte_info_afull_th : 10; /* [9:0] */
        unsigned int tm_byte_info_bp_en : 1;     /* [10] */
        unsigned int tm_offset_fifo_gap : 3;     /* [13:11] */
        unsigned int rsv : 18;                   /* [31:14] */
    } bits;

    unsigned int value;
} u_ets_byte_info_cfg;

typedef union {
    struct {
        unsigned int queue_offset_len : 10; /* [9:0] */
        unsigned int rsv : 22;              /* [31:10] */
    } bits;

    unsigned int value;
} u_ets_queue_offset_len;

typedef union {
    struct {
        unsigned int tc_link_tcg_cfg : 2; /* [1:0] */
        unsigned int rsv : 30;            /* [31:2] */
    } bits;

    unsigned int value;
} u_ets_tc_link_tcg;

typedef union {
    struct {
        unsigned int tcg_link_port_cfg : 2; /* [1:0] */
        unsigned int rsv : 30;              /* [31:2] */
    } bits;

    unsigned int value;
} u_ets_tcg_link_port;

typedef union {
    struct {
        unsigned int tx_int_idx : 8;    /* [7:0] */
        unsigned int tx_int_gl_idx : 2; /* [9:8] */
        unsigned int rsv0 : 6;          /* [15:10] */
        unsigned int tx_next_qid : 11;  /* [26:16] */
        unsigned int tx_next_qtype : 1; /* [27] */
        unsigned int rsv1 : 4;          /* [31:28] */
    } bits;

    unsigned int value;
} u_rcb_tx_q_intctl;

typedef union {
    struct {
        unsigned int rx_int_idx : 8;    /* [7:0] */
        unsigned int rx_int_gl_idx : 2; /* [9:8] */
        unsigned int rsv0 : 6;          /* [15:10] */
        unsigned int rx_next_qid : 11;  /* [26:16] */
        unsigned int rx_next_qtype : 1; /* [27] */
        unsigned int rsv1 : 4;          /* [31:28] */
    } bits;

    unsigned int value;
} u_rcb_rx_q_intctl;

typedef union {
    struct {
        unsigned int tqp_vfid_bit_inv : 4; /* [3:0] */
        unsigned int tqp_pfid_bit_inv : 5; /* [8:4] */
        unsigned int tqp_vid_bit_inv : 9;  /* [17:9] */
        unsigned int tqp_item_valid_x : 1; /* [18] */
        unsigned int rsv : 13;             /* [31:19] */
    } bits;

    unsigned int value;
} u_rcb_tqp_map_x;

typedef union {
    struct {
        unsigned int tqp_vfid : 4;         /* [3:0] */
        unsigned int tqp_pfid : 5;         /* [8:4] */
        unsigned int tqp_vid : 9;          /* [17:9] */
        unsigned int tqp_item_valid_y : 1; /* [18] */
        unsigned int rsv : 13;             /* [31:19] */
    } bits;

    unsigned int value;
} u_rcb_tqp_map_y;

typedef union {
    struct {
        unsigned int rcb_ring_to_be_rst : 1; /* [0] */
        unsigned int rsv : 31;               /* [31:1] */
    } bits;

    unsigned int value;
} u_rcb_ring_to_be_rst;

typedef union {
    struct {
        unsigned int rcb_ring_could_be_rst : 1; /* [0] */
        unsigned int rsv : 31;                  /* [31:1] */
    } bits;

    unsigned int value;
} u_rcb_ring_could_be_rst;

typedef union {
    struct {
        unsigned int cfg_qid_map_port_num : 2; /* [1:0] */
        unsigned int rsv : 30;                 /* [31:2] */
    } bits;

    unsigned int value;
} u_rcb_ring_port_map;

typedef union {
    struct {
        unsigned int over_8bd_no_fe_int_en : 1;      /* [0] */
        unsigned int tso_mss_cmp_min_err_int_en : 1; /* [1] */
        unsigned int tso_mss_cmp_max_err_int_en : 1; /* [2] */
        unsigned int tx_rd_fbd_poison_int_en : 1;    /* [3] */
        unsigned int rx_rd_ebd_poison_int_en : 1;    /* [4] */
        unsigned int rsv : 27;                       /* [31:5] */
    } bits;

    unsigned int value;
} u_pf_abnormal_int_en;

typedef union {
    struct {
        unsigned int over_8bd_no_fe_int_st : 1;      /* [0] */
        unsigned int tso_mss_cmp_min_err_int_st : 1; /* [1] */
        unsigned int tso_mss_cmp_max_err_int_st : 1; /* [2] */
        unsigned int tx_rd_fbd_poison_int_st : 1;    /* [3] */
        unsigned int rx_rd_ebd_poison_int_st : 1;    /* [4] */
        unsigned int rsv : 27;                       /* [31:5] */
    } bits;

    unsigned int value;
} u_pf_abnormal_int_st;

typedef union {
    struct {
        unsigned int over_8bd_no_fe_int_src : 1;      /* [0] */
        unsigned int tso_mss_cmp_min_err_int_src : 1; /* [1] */
        unsigned int tso_mss_cmp_max_err_int_src : 1; /* [2] */
        unsigned int tx_rd_fbd_poison_int_src : 1;    /* [3] */
        unsigned int rx_rd_ebd_poison_int_src : 1;    /* [4] */
        unsigned int rsv : 27;                        /* [31:5] */
    } bits;

    unsigned int value;
} u_pf_abnormal_int_src;

typedef union {
    struct {
        unsigned int over_8bd_no_fe_int_set : 1;      /* [0] */
        unsigned int tso_mss_cmp_min_err_int_set : 1; /* [1] */
        unsigned int tso_mss_cmp_max_err_int_set : 1; /* [2] */
        unsigned int tx_rd_fbd_poison_int_set : 1;    /* [3] */
        unsigned int rx_rd_ebd_poison_int_set : 1;    /* [4] */
        unsigned int rsv : 27;                        /* [31:5] */
    } bits;

    unsigned int value;
} u_pf_abnormal_int_set;

typedef union {
    struct {
        unsigned int over_8bd_no_fe_int_ce_src : 1;      /* [0] */
        unsigned int tso_mss_cmp_min_err_int_ce_src : 1; /* [1] */
        unsigned int tso_mss_cmp_max_err_int_ce_src : 1; /* [2] */
        unsigned int tx_rd_fbd_poison_int_ce_src : 1;    /* [3] */
        unsigned int rx_rd_ebd_poison_int_ce_src : 1;    /* [4] */
        unsigned int rsv : 27;                           /* [31:5] */
    } bits;

    unsigned int value;
} u_pf_abnormal_int_ce;

typedef union {
    struct {
        unsigned int over_8bd_no_fe_int_nfe_src : 1;      /* [0] */
        unsigned int tso_mss_cmp_min_err_int_nfe_src : 1; /* [1] */
        unsigned int tso_mss_cmp_max_err_int_nfe_src : 1; /* [2] */
        unsigned int tx_rd_fbd_poison_int_nfe_src : 1;    /* [3] */
        unsigned int rx_rd_ebd_poison_int_nfe_src : 1;    /* [4] */
        unsigned int rsv : 27;                            /* [31:5] */
    } bits;

    unsigned int value;
} u_pf_abnormal_int_nfe;

typedef union {
    struct {
        unsigned int over_8bd_no_fe_int_fe_src : 1;      /* [0] */
        unsigned int tso_mss_cmp_min_err_int_fe_src : 1; /* [1] */
        unsigned int tso_mss_cmp_max_err_int_fe_src : 1; /* [2] */
        unsigned int tx_rd_fbd_poison_int_fe_src : 1;    /* [3] */
        unsigned int rx_rd_ebd_poison_int_fe_src : 1;    /* [4] */
        unsigned int rsv : 27;                           /* [31:5] */
    } bits;

    unsigned int value;
} u_pf_abnormal_int_fe;

typedef union {
    struct {
        unsigned int over_8bd_no_fe_int_type : 2;      /* [1:0] */
        unsigned int tso_mss_cmp_min_err_int_type : 2; /* [3:2] */
        unsigned int tso_mss_cmp_max_err_int_type : 2; /* [5:4] */
        unsigned int tx_rd_fbd_poison_int_type : 2;    /* [7:6] */
        unsigned int rx_rd_ebd_poison_int_type : 2;    /* [9:8] */
        unsigned int rsv : 22;                         /* [31:10] */
    } bits;

    unsigned int value;
} u_pf_abnormal_int_type;

typedef union {
    struct {
        unsigned int over_8bd_no_fe_qid : 4; /* [3:0] */
        unsigned int rsv : 28;               /* [31:4] */
    } bits;

    unsigned int value;
} u_pf_abnormal_int_dfx0;

typedef union {
    struct {
        unsigned int tso_mss_cmp_min_err_qid : 4; /* [3:0] */
        unsigned int rsv : 28;                    /* [31:4] */
    } bits;

    unsigned int value;
} u_pf_abnormal_int_dfx1;

typedef union {
    struct {
        unsigned int tso_mss_cmp_max_err_qid : 4; /* [3:0] */
        unsigned int rsv : 28;                    /* [31:4] */
    } bits;

    unsigned int value;
} u_pf_abnormal_int_dfx2;

typedef union {
    struct {
        unsigned int tx_rd_fbd_poison_qid : 4; /* [3:0] */
        unsigned int rsv : 28;                 /* [31:4] */
    } bits;

    unsigned int value;
} u_pf_abnormal_int_dfx3;

typedef union {
    struct {
        unsigned int rx_rd_ebd_poison_qid : 4; /* [3:0] */
        unsigned int rsv : 28;                 /* [31:4] */
    } bits;

    unsigned int value;
} u_pf_abnormal_int_dfx4;

#endif  // __REG_RCB_COM_H__