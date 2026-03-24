/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: xxvge_common
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __C_UNION_DEFINE_XXVGE_COMMON_H__
#define __C_UNION_DEFINE_XXVGE_COMMON_H__

/* define the union u_xxvge_port_mode */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int tx_mode : 3;    /* [2:0] */
        unsigned int tx_rmii_en : 1; /* [3] */
        unsigned int rx_mode : 3;    /* [6:4] */
        unsigned int rx_rmii_en : 1; /* [7] */
        unsigned int reserved : 24;  /* [31:8] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_port_mode;

/* define the union u_xxvge_port_speed */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int tx_speed : 3;   /* [2:0] */
        unsigned int reserved : 1;   /* [3] */
        unsigned int rx_speed : 3;   /* [6:4] */
        unsigned int reserved1 : 1;  /* [7] */
        unsigned int sel : 1;        /* [8] */
        unsigned int reserved2 : 23; /* [31:9] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_port_speed;

/* define the union u_xxvge_int_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int ierr_u : 1;             /* [0] */
        unsigned int ierr_c : 1;             /* [1] */
        unsigned int unf : 1;                /* [2] */
        unsigned int ovf : 1;                /* [3] */
        unsigned int link_up : 1;            /* [4] */
        unsigned int link_down : 1;          /* [5] */
        unsigned int lf : 1;                 /* [6] */
        unsigned int rf : 1;                 /* [7] */
        unsigned int link_interrupt : 1;     /* [8] */
        unsigned int wis : 1;                /* [9] */
        unsigned int an_restart : 1;         /* [10] */
        unsigned int reserved : 2;           /* [12:11] */
        unsigned int preemp_verify_done : 1; /* [13] */
        unsigned int an_rf : 1;              /* [14] */
        unsigned int an_np_up : 1;           /* [15] */
        unsigned int an_local_rf : 1;        /* [16] */
        unsigned int return_ts_done : 1;     /* [17] */
        unsigned int reserved1 : 14;         /* [31:18] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_int_status;

/* define the union u_xxvge_int_enable */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int ierr_u : 1;             /* [0] */
        unsigned int ierr_c : 1;             /* [1] */
        unsigned int unf : 1;                /* [2] */
        unsigned int ovf : 1;                /* [3] */
        unsigned int link_up : 1;            /* [4] */
        unsigned int link_down : 1;          /* [5] */
        unsigned int lf : 1;                 /* [6] */
        unsigned int rf : 1;                 /* [7] */
        unsigned int link_interrupt : 1;     /* [8] */
        unsigned int wis : 1;                /* [9] */
        unsigned int an_restart : 1;         /* [10] */
        unsigned int reserved : 2;           /* [12:11] */
        unsigned int preemp_verify_done : 1; /* [13] */
        unsigned int an_rf : 1;              /* [14] */
        unsigned int an_np_up : 1;           /* [15] */
        unsigned int an_local_rf : 1;        /* [16] */
        unsigned int return_ts_done : 1;     /* [17] */
        unsigned int reserved1 : 14;         /* [31:18] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_int_enable;

/* define the union u_xxvge_link_status */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int link_up : 1;       /* [0] */
        unsigned int reserved : 27;     /* [27:1] */
        unsigned int phy_an_duplex : 1; /* [28] */
        unsigned int phy_an_speed : 3;  /* [31:29] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_link_status;

/* define the union u_xxvge_link_control */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int mode : 2;               /* [1:0] */
        unsigned int rsvd : 2;               /* [3:2] */
        unsigned int lf_sel : 1;             /* [4] */
        unsigned int rf_sel : 1;             /* [5] */
        unsigned int link_interrupt_sel : 1; /* [6] */
        unsigned int rsvd1 : 1;              /* [7] */
        unsigned int link_rf : 1;            /* [8] */
        unsigned int link_phy : 1;           /* [9] */
        unsigned int link_duplex : 1;        /* [10] */
        unsigned int link_sync : 1;          /* [11] */
        unsigned int link_ctrl : 1;          /* [12] */
        unsigned int reserved : 19;          /* [31:13] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_link_control;

/* define the union u_xxvge_mib_control */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int mib_clr : 1;             /* [0] */
        unsigned int mib_car : 1;             /* [1] */
        unsigned int mib_fcs_mode : 1;        /* [2] */
        unsigned int mib_ecc_err_en : 1;      /* [3] */
        unsigned int mib_ecc_err_bit : 1;     /* [4] */
        unsigned int mib_unsaturate_mode : 1; /* [5] */
        unsigned int mib_align_mode : 1;      /* [6] */
        unsigned int rsvd : 1;                /* [7] */
        unsigned int mib_addr_ecc_chk_en : 8; /* [15:8] */
        unsigned int reserved : 16;           /* [31:16] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_mib_control;

/* define the union u_xxvge_mac_max_pkt_size */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int max_size : 14;    /* [13:0] */
        unsigned int reserved : 18;    /* [31:14] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_mac_max_pkt_size;

/* define the union u_xxvge_mac_pause_pfc_ctrl */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int tx_pause_en : 1;       /* [0] */
        unsigned int rx_pause_en : 1;       /* [1] */
        unsigned int tx_pfc_en : 1;         /* [2] */
        unsigned int rx_pfc_en : 1;         /* [3] */
        unsigned int reserved : 28;         /* [31:4] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_mac_pause_pfc_ctrl;

/* define the union u_xxvge_an_cfg_sel */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int speed_sel : 1;               /* [0] */
        unsigned int rsvd : 3;                    /* [3:1] */
        unsigned int pause_sel : 1;               /* [4] */
        unsigned int pfc_sel : 1;                 /* [5] */
        unsigned int rsvd1 : 2;                   /* [7:6] */
        unsigned int duplex_sel : 1;              /* [8] */
        unsigned int rsvd2 : 3;                   /* [11:9] */
        unsigned int sgmii_speed_sel : 1;         /* [12] */
        unsigned int rsvd3 : 3;                   /* [15:13] */
        unsigned int phy_link_en : 1;             /* [16] */
        unsigned int tx_msg_sel : 2;              /* [18:17] */
        unsigned int force_full_duplex_en : 1;    /* [19] */
        unsigned int rx_msg_sel : 1;              /* [20] */
        unsigned int tx_an_speed_map : 1;         /* [21] */
        unsigned int mpcs_speed_map : 1;          /* [22] */
        unsigned int rsvd4 : 9;                   /* [31:23] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_an_cfg_sel;

/* define the union u_xxvge_pcs_an_ctrl */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int restart_an : 1;               /* [0] */
        unsigned int autoneg_en : 1;               /* [1] */
        unsigned int an_unidir_en : 1;             /* [2] */
        unsigned int np_en : 1;                    /* [3] */
        unsigned int overtime_en : 1;              /* [4] */
        unsigned int ack_mode : 1;                 /* [5] */
        unsigned int ack : 1;                      /* [6] */
        unsigned int speed_byd : 1;                /* [7] */
        unsigned int rsvd : 16;                    /* [23:8] */
        unsigned int rxmac_an : 1;                 /* [24] */
        unsigned int sync_restart_en : 1;          /* [25] */
        unsigned int an_force_disable : 1;         /* [26] */
        unsigned int pause_resolve_pri_en : 1;     /* [27] */
        unsigned int duplex_resolve_pri_en : 1;    /* [28] */
        unsigned int idle_match_thd : 3;           /* [31:29] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_pcs_an_ctrl;

/* define the union u_xxvge_pcs_an_base_page */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int rsvd : 10;                     /* [9:0] */
        unsigned int port_speed : 2;                /* [11:10] */
        unsigned int duplex_mode : 1;               /* [12] */
        unsigned int rsvd1 : 1;                     /* [13] */
        unsigned int rsvd_for_ack : 1;              /* [14] */
        unsigned int link_status : 1;               /* [15] */
        unsigned int rsvd2 : 16;                    /* [31:16] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_pcs_an_base_page;

/* define the union u_xxvge_pcs_an_link_time */
typedef union {
    /* define the struct bits */
    struct {
        unsigned int link_time : 24;              /* [23:0] */
        unsigned int rsvd2 : 8;                   /* [31:24] */
    } bits;

    /* define an unsigned member */
    unsigned int value;
} u_xxvge_pcs_an_link_time;

#endif  // __C_UNION_DEFINE_XXVGE_COMMON_H__
