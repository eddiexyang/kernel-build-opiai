
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_pa
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_PA_H__
#define __REG_PA_H__

typedef union {
    struct {
        unsigned int cfg_et0_pa_en : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_et_ipv4_enable;

typedef union {
    struct {
        unsigned int cfg_et0_begin : 16; /* [15:0] */
        unsigned int cfg_et0_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_et_ipv4;

typedef union {
    struct {
        unsigned int cfg_et1_pa_en : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_et_ipv6_enable;

typedef union {
    struct {
        unsigned int cfg_et1_begin : 16; /* [15:0] */
        unsigned int cfg_et1_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_et_ipv6;

typedef union {
    struct {
        unsigned int cfg_arp_pa_en : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_et_arp_enable;

typedef union {
    struct {
        unsigned int cfg_et2_begin : 16; /* [15:0] */
        unsigned int cfg_et2_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_et_arp;

typedef union {
    struct {
        unsigned int cfg_rarp_pa_en : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_et_rarp_enable;

typedef union {
    struct {
        unsigned int cfg_et3_begin : 16; /* [15:0] */
        unsigned int cfg_et3_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_et_rarp;

typedef union {
    struct {
        unsigned int cfg_et4_pa_en : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_et_def4_enable;

typedef union {
    struct {
        unsigned int cfg_et4_begin : 16; /* [15:0] */
        unsigned int cfg_et4_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_et_def4;

typedef union {
    struct {
        unsigned int cfg_et5_pa_en : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_et_def5_enable;

typedef union {
    struct {
        unsigned int cfg_et5_begin : 16; /* [15:0] */
        unsigned int cfg_et5_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_et_def5;

typedef union {
    struct {
        unsigned int cfg_et6_pa_en : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_et_lldp_enable;

typedef union {
    struct {
        unsigned int cfg_et6_begin : 16; /* [15:0] */
        unsigned int cfg_et6_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_et_lldp;

typedef union {
    struct {
        unsigned int rsv : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_pa_et_bpdu_enable;

typedef union {
    struct {
        unsigned int cfg_et8_pa_en : 1;  /* [0] */
        unsigned int cfg_et8_udf_en : 1; /* [1] */
        unsigned int rsv : 30;           /* [31:2] */
    } bits;

    unsigned int value;
} u_pa_et_mac_pause_enable;

typedef union {
    struct {
        unsigned int cfg_et8_begin : 16; /* [15:0] */
        unsigned int cfg_et8_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_et_mac_pause;

typedef union {
    struct {
        unsigned int cfg_et9_pa_en : 1;  /* [0] */
        unsigned int cfg_et9_udf_en : 1; /* [1] */
        unsigned int rsv : 30;           /* [31:2] */
    } bits;

    unsigned int value;
} u_pa_et_pfc_pause_enable;

typedef union {
    struct {
        unsigned int cfg_et9_begin : 16; /* [15:0] */
        unsigned int cfg_et9_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_et_pfc_pause;

typedef union {
    struct {
        unsigned int cfg_et10_pa_en : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_et_mac_1588_enable;

typedef union {
    struct {
        unsigned int cfg_et10_begin : 16; /* [15:0] */
        unsigned int cfg_et10_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_et_mac_1588;

typedef union {
    struct {
        unsigned int cfg_et11_pa_en : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_et_8021x_enable;

typedef union {
    struct {
        unsigned int cfg_et11_begin : 16; /* [15:0] */
        unsigned int cfg_et11_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_et_8021x;

typedef union {
    struct {
        unsigned int cfg_et12_pa_en : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_et_cnm_enable;

typedef union {
    struct {
        unsigned int cfg_et12_begin : 16; /* [15:0] */
        unsigned int cfg_et12_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_et_cnm;

typedef union {
    struct {
        unsigned int cfg_et13_pa_en : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_et_y1731_enable;

typedef union {
    struct {
        unsigned int cfg_et13_begin : 16; /* [15:0] */
        unsigned int cfg_et13_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_et_y1731;

typedef union {
    struct {
        unsigned int cfg_et14_pa_en : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_et_8023ah_enable;

typedef union {
    struct {
        unsigned int cfg_et14_begin : 16; /* [15:0] */
        unsigned int cfg_et14_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_et_8023ah;

typedef union {
    struct {
        unsigned int cfg_ip0_pa_en : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_ip_udp_enable;

typedef union {
    struct {
        unsigned int cfg_ip0_begin : 8; /* [7:0] */
        unsigned int cfg_ip0_end : 8;   /* [15:8] */
        unsigned int rsv : 16;          /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_ip_udp;

typedef union {
    struct {
        unsigned int cfg_ip1_pa_en : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_ip_tcp_enable;

typedef union {
    struct {
        unsigned int cfg_ip1_begin : 8; /* [7:0] */
        unsigned int cfg_ip1_end : 8;   /* [15:8] */
        unsigned int rsv : 16;          /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_ip_tcp;

typedef union {
    struct {
        unsigned int cfg_ip2_pa_en : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_ip_gre_enable;

typedef union {
    struct {
        unsigned int cfg_ip2_begin : 8; /* [7:0] */
        unsigned int cfg_ip2_end : 8;   /* [15:8] */
        unsigned int rsv : 16;          /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_ip_gre;

typedef union {
    struct {
        unsigned int cfg_ip3_pa_en : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_ip_sctp_enable;

typedef union {
    struct {
        unsigned int cfg_ip3_begin : 8; /* [7:0] */
        unsigned int cfg_ip3_end : 8;   /* [15:8] */
        unsigned int rsv : 16;          /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_ip_sctp;

typedef union {
    struct {
        unsigned int cfg_ip4_pa_en : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_ip_igmp_enable;

typedef union {
    struct {
        unsigned int cfg_ip4_begin : 8; /* [7:0] */
        unsigned int cfg_ip4_end : 8;   /* [15:8] */
        unsigned int rsv : 16;          /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_ip_igmp;

typedef union {
    struct {
        unsigned int cfg_ip5_pa_en : 1;            /* [0] */
        unsigned int cfg_ip5_ipv4_icmp_udf_en : 1; /* [1] */
        unsigned int rsv : 30;                     /* [31:2] */
    } bits;

    unsigned int value;
} u_pa_ipv4_icmp_enable;

typedef union {
    struct {
        unsigned int cfg_ip5_begin : 8; /* [7:0] */
        unsigned int cfg_ip5_end : 8;   /* [15:8] */
        unsigned int rsv : 16;          /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_ipv4_icmp;

typedef union {
    struct {
        unsigned int cfg_ip6_pa_en : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_ip_esp_enable;

typedef union {
    struct {
        unsigned int cfg_ip6_begin : 8; /* [7:0] */
        unsigned int cfg_ip6_end : 8;   /* [15:8] */
        unsigned int rsv : 16;          /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_ip_esp;

typedef union {
    struct {
        unsigned int cfg_ip7_pa_en : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_ip_ah_enable;

typedef union {
    struct {
        unsigned int cfg_ip7_begin : 8; /* [7:0] */
        unsigned int cfg_ip7_end : 8;   /* [15:8] */
        unsigned int rsv : 16;          /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_ip_ah;

typedef union {
    struct {
        unsigned int cfg_ip8_pa_en : 1;            /* [0] */
        unsigned int cfg_ip8_ipv6_icmp_udf_en : 1; /* [1] */
        unsigned int rsv : 30;                     /* [31:2] */
    } bits;

    unsigned int value;
} u_pa_ipv6_icmp_enable;

typedef union {
    struct {
        unsigned int cfg_ip8_begin : 8; /* [7:0] */
        unsigned int cfg_ip8_end : 8;   /* [15:8] */
        unsigned int rsv : 16;          /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_ipv6_icmp;

typedef union {
    struct {
        unsigned int cfg_ip9_pa_en : 1; /* [0] */
        unsigned int rsv : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_ip_def9_enable;

typedef union {
    struct {
        unsigned int cfg_ip9_begin : 8; /* [7:0] */
        unsigned int cfg_ip9_end : 8;   /* [15:8] */
        unsigned int rsv : 16;          /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_ip_def9;

typedef union {
    struct {
        unsigned int cfg_ip10_pa_en : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_ip_def10_enable;

typedef union {
    struct {
        unsigned int cfg_ip10_begin : 8; /* [7:0] */
        unsigned int cfg_ip10_end : 8;   /* [15:8] */
        unsigned int rsv : 16;           /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_ip_def10;

typedef union {
    struct {
        unsigned int cfg_ip11_pa_en : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_ip_def11_enable;

typedef union {
    struct {
        unsigned int cfg_ip11_begin : 8; /* [7:0] */
        unsigned int cfg_ip11_end : 8;   /* [15:8] */
        unsigned int rsv : 16;           /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_ip_def11;

typedef union {
    struct {
        unsigned int cfg_ip12_pa_en : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_ip_def12_enable;

typedef union {
    struct {
        unsigned int cfg_ip12_begin : 8; /* [7:0] */
        unsigned int cfg_ip12_end : 8;   /* [15:8] */
        unsigned int rsv : 16;           /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_ip_def12;

typedef union {
    struct {
        unsigned int cfg_ip13_pa_en : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_ip_def13_enable;

typedef union {
    struct {
        unsigned int cfg_ip13_begin : 8; /* [7:0] */
        unsigned int cfg_ip13_end : 8;   /* [15:8] */
        unsigned int rsv : 16;           /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_ip_def13;

typedef union {
    struct {
        unsigned int cfg_ip14_pa_en : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_ip_def14_enable;

typedef union {
    struct {
        unsigned int cfg_ip14_begin : 8; /* [7:0] */
        unsigned int cfg_ip14_end : 8;   /* [15:8] */
        unsigned int rsv : 16;           /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_ip_def14;

typedef union {
    struct {
        unsigned int cfg_dport0_pa_en : 1;   /* [0] */
        unsigned int cfg_dport0_udp_en : 1;  /* [1] */
        unsigned int cfg_dport0_tcp_en : 1;  /* [2] */
        unsigned int cfg_dport0_sctp_en : 1; /* [3] */
        unsigned int rsv : 28;               /* [31:4] */
    } bits;

    unsigned int value;
} u_pa_dport_dhbt_enable;

typedef union {
    struct {
        unsigned int cfg_dport0_begin : 16; /* [15:0] */
        unsigned int cfg_dport0_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_dport_dhbt;

typedef union {
    struct {
        unsigned int cfg_dport1_pa_en : 1;   /* [0] */
        unsigned int cfg_dport1_udp_en : 1;  /* [1] */
        unsigned int cfg_dport1_tcp_en : 1;  /* [2] */
        unsigned int cfg_dport1_sctp_en : 1; /* [3] */
        unsigned int rsv : 28;               /* [31:4] */
    } bits;

    unsigned int value;
} u_pa_dport_ippm_enable;

typedef union {
    struct {
        unsigned int cfg_dport1_begin : 16; /* [15:0] */
        unsigned int cfg_dport1_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_dport_ippm;

typedef union {
    struct {
        unsigned int cfg_dport2_pa_en : 1;   /* [0] */
        unsigned int cfg_dport2_udp_en : 1;  /* [1] */
        unsigned int cfg_dport2_tcp_en : 1;  /* [2] */
        unsigned int cfg_dport2_sctp_en : 1; /* [3] */
        unsigned int rsv : 28;               /* [31:4] */
    } bits;

    unsigned int value;
} u_pa_dport_udp_1588_enable;

typedef union {
    struct {
        unsigned int cfg_dport2_begin : 16; /* [15:0] */
        unsigned int cfg_dport2_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_dport_udp_1588;

typedef union {
    struct {
        unsigned int cfg_dport3_pa_en : 1;   /* [0] */
        unsigned int cfg_dport3_udp_en : 1;  /* [1] */
        unsigned int cfg_dport3_tcp_en : 1;  /* [2] */
        unsigned int cfg_dport3_sctp_en : 1; /* [3] */
        unsigned int rsv : 28;               /* [31:4] */
    } bits;

    unsigned int value;
} u_pa_dport_gtpu_enable;

typedef union {
    struct {
        unsigned int cfg_dport3_begin : 16; /* [15:0] */
        unsigned int cfg_dport3_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_dport_gtpu;

typedef union {
    struct {
        unsigned int cfg_dport4_pa_en : 1;   /* [0] */
        unsigned int cfg_dport4_udp_en : 1;  /* [1] */
        unsigned int cfg_dport4_tcp_en : 1;  /* [2] */
        unsigned int cfg_dport4_sctp_en : 1; /* [3] */
        unsigned int rsv : 28;               /* [31:4] */
    } bits;

    unsigned int value;
} u_pa_dport_http_enable;

typedef union {
    struct {
        unsigned int cfg_dport4_begin : 16; /* [15:0] */
        unsigned int cfg_dport4_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_dport_http;

typedef union {
    struct {
        unsigned int cfg_dport5_pa_en : 1;   /* [0] */
        unsigned int cfg_dport5_udp_en : 1;  /* [1] */
        unsigned int cfg_dport5_tcp_en : 1;  /* [2] */
        unsigned int cfg_dport5_sctp_en : 1; /* [3] */
        unsigned int rsv : 28;               /* [31:4] */
    } bits;

    unsigned int value;
} u_pa_dport_https_enable;

typedef union {
    struct {
        unsigned int cfg_dport5_begin : 16; /* [15:0] */
        unsigned int cfg_dport5_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_dport_https;

typedef union {
    struct {
        unsigned int cfg_dport6_pa_en : 1;   /* [0] */
        unsigned int cfg_dport6_udp_en : 1;  /* [1] */
        unsigned int cfg_dport6_tcp_en : 1;  /* [2] */
        unsigned int cfg_dport6_sctp_en : 1; /* [3] */
        unsigned int rsv : 28;               /* [31:4] */
    } bits;

    unsigned int value;
} u_pa_dport_ftp_enable;

typedef union {
    struct {
        unsigned int cfg_dport6_begin : 16; /* [15:0] */
        unsigned int cfg_dport6_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_dport_ftp;

typedef union {
    struct {
        unsigned int cfg_dport7_pa_en : 1;   /* [0] */
        unsigned int cfg_dport7_udp_en : 1;  /* [1] */
        unsigned int cfg_dport7_tcp_en : 1;  /* [2] */
        unsigned int cfg_dport7_sctp_en : 1; /* [3] */
        unsigned int rsv : 28;               /* [31:4] */
    } bits;

    unsigned int value;
} u_pa_dport_bfd_enable;

typedef union {
    struct {
        unsigned int cfg_dport7_begin : 16; /* [15:0] */
        unsigned int cfg_dport7_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_dport_bfd;

typedef union {
    struct {
        unsigned int cfg_dport8_pa_en : 1;   /* [0] */
        unsigned int cfg_dport8_udp_en : 1;  /* [1] */
        unsigned int cfg_dport8_tcp_en : 1;  /* [2] */
        unsigned int cfg_dport8_sctp_en : 1; /* [3] */
        unsigned int rsv : 28;               /* [31:4] */
    } bits;

    unsigned int value;
} u_pa_dport_roce_enable;

typedef union {
    struct {
        unsigned int cfg_dport8_begin : 16; /* [15:0] */
        unsigned int cfg_dport8_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_dport_roce;

typedef union {
    struct {
        unsigned int cfg_dport9_pa_en : 1;   /* [0] */
        unsigned int cfg_dport9_udp_en : 1;  /* [1] */
        unsigned int cfg_dport9_tcp_en : 1;  /* [2] */
        unsigned int cfg_dport9_sctp_en : 1; /* [3] */
        unsigned int rsv : 28;               /* [31:4] */
    } bits;

    unsigned int value;
} u_pa_dport_def9_enable;

typedef union {
    struct {
        unsigned int cfg_dport9_begin : 16; /* [15:0] */
        unsigned int cfg_dport9_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_dport_def9;

typedef union {
    struct {
        unsigned int cfg_dport10_pa_en : 1;   /* [0] */
        unsigned int cfg_dport10_udp_en : 1;  /* [1] */
        unsigned int cfg_dport10_tcp_en : 1;  /* [2] */
        unsigned int cfg_dport10_sctp_en : 1; /* [3] */
        unsigned int rsv : 28;                /* [31:4] */
    } bits;

    unsigned int value;
} u_pa_dport_def10_enable;

typedef union {
    struct {
        unsigned int cfg_dport10_begin : 16; /* [15:0] */
        unsigned int cfg_dport10_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_dport_def10;

typedef union {
    struct {
        unsigned int cfg_dport11_pa_en : 1;   /* [0] */
        unsigned int cfg_dport11_udp_en : 1;  /* [1] */
        unsigned int cfg_dport11_tcp_en : 1;  /* [2] */
        unsigned int cfg_dport11_sctp_en : 1; /* [3] */
        unsigned int rsv : 28;                /* [31:4] */
    } bits;

    unsigned int value;
} u_pa_dport_def11_enable;

typedef union {
    struct {
        unsigned int cfg_dport11_begin : 16; /* [15:0] */
        unsigned int cfg_dport11_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_dport_def11;

typedef union {
    struct {
        unsigned int cfg_dport12_pa_en : 1;   /* [0] */
        unsigned int cfg_dport12_udp_en : 1;  /* [1] */
        unsigned int cfg_dport12_tcp_en : 1;  /* [2] */
        unsigned int cfg_dport12_sctp_en : 1; /* [3] */
        unsigned int rsv : 28;                /* [31:4] */
    } bits;

    unsigned int value;
} u_pa_dport_def12_enable;

typedef union {
    struct {
        unsigned int cfg_dport12_begin : 16; /* [15:0] */
        unsigned int cfg_dport12_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_dport_def12;

typedef union {
    struct {
        unsigned int cfg_dport13_pa_en : 1;   /* [0] */
        unsigned int cfg_dport13_udp_en : 1;  /* [1] */
        unsigned int cfg_dport13_tcp_en : 1;  /* [2] */
        unsigned int cfg_dport13_sctp_en : 1; /* [3] */
        unsigned int rsv : 28;                /* [31:4] */
    } bits;

    unsigned int value;
} u_pa_dport_def13_enable;

typedef union {
    struct {
        unsigned int cfg_dport13_begin : 16; /* [15:0] */
        unsigned int cfg_dport13_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_dport_def13;

typedef union {
    struct {
        unsigned int cfg_dport14_pa_en : 1;   /* [0] */
        unsigned int cfg_dport14_udp_en : 1;  /* [1] */
        unsigned int cfg_dport14_tcp_en : 1;  /* [2] */
        unsigned int cfg_dport14_sctp_en : 1; /* [3] */
        unsigned int rsv : 28;                /* [31:4] */
    } bits;

    unsigned int value;
} u_pa_dport_def14_enable;

typedef union {
    struct {
        unsigned int cfg_dport14_begin : 16; /* [15:0] */
        unsigned int cfg_dport14_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_dport_def14;

typedef union {
    struct {
        unsigned int cfg_et_bpdu_begin : 16; /* [15:0] */
        unsigned int cfg_et_bpdu_end : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_et_bpdu;

typedef union {
    struct {
        unsigned int cfg_bpdu_match_addr : 24; /* [23:0] */
        unsigned int cfg_bpdu_pa_type : 1;     /* [24] */
        unsigned int cfg_bpdu_pa_en : 1;       /* [25] */
        unsigned int rsv : 6;                  /* [31:26] */
    } bits;

    unsigned int value;
} u_pa_bpdu_addr_match;

typedef union {
    struct {
        unsigned int cfg_bpdu_match_addr_mask : 24; /* [23:0] */
        unsigned int rsv : 8;                       /* [31:24] */
    } bits;

    unsigned int value;
} u_pa_bpdu_addr_match_mask;

typedef union {
    struct {
        unsigned int cfg_l2_flex_oft : 15;   /* [14:0] */
        unsigned int cfg_l2_flex_oft_en : 1; /* [15] */
        unsigned int rsv : 16;               /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_l2_flex_offset;

typedef union {
    struct {
        unsigned int cfg_l3_flex_oft : 15;   /* [14:0] */
        unsigned int cfg_l3_flex_oft_en : 1; /* [15] */
        unsigned int rsv : 16;               /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_l3_flex_offset;

typedef union {
    struct {
        unsigned int cfg_l4_flex_oft : 15;   /* [14:0] */
        unsigned int cfg_l4_flex_oft_en : 1; /* [15] */
        unsigned int rsv : 16;               /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_l4_flex_offset;

typedef union {
    struct {
        unsigned int cfg_l2_udf_def_value : 1; /* [0] */
        unsigned int cfg_l3_udf_def_value : 1; /* [1] */
        unsigned int cfg_l4_udf_def_value : 1; /* [2] */
        unsigned int rsv : 29;                 /* [31:3] */
    } bits;

    unsigned int value;
} u_pa_udf_def_value;

typedef union {
    struct {
        unsigned int cfg_l5_analy_en : 1;      /* [0] */
        unsigned int cfg_l3_check_en : 1;      /* [1] */
        unsigned int cfg_tcp_check_en : 1;     /* [2] */
        unsigned int cfg_udp_check_en : 1;     /* [3] */
        unsigned int cfg_sctp_check_en : 1;    /* [4] */
        unsigned int cfg_l2_analy_en : 1;      /* [5] */
        unsigned int cfg_l3_analy_en : 1;      /* [6] */
        unsigned int cfg_ext_hdr_analy_en : 1; /* [7] */
        unsigned int cfg_l4_analy_en : 1;      /* [8] */
        unsigned int cfg_l3_err_rpt_en : 1;    /* [9] */
        unsigned int cfg_l4_err_rpt_en : 1;    /* [10] */
        unsigned int rsv : 21;                 /* [31:11] */
    } bits;

    unsigned int value;
} u_pa_analy_config;

typedef union {
    struct {
        unsigned int cfg_port_type : 3;       /* [2:0] */
        unsigned int cfg_non_std_pkt_len : 6; /* [8:3] */
        unsigned int rsv : 23;                /* [31:9] */
    } bits;

    unsigned int value;
} u_pa_port_type;

typedef union {
    struct {
        unsigned int cfg_pro_len : 16; /* [15:0] */
        unsigned int rsv : 16;         /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_pro_len;

typedef union {
    struct {
        unsigned int cfg_snap_xsnap_en : 1; /* [0] */
        unsigned int rsv : 31;              /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_xsnap_enable;

typedef union {
    struct {
        unsigned int cfg_crc_chk_en : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_crc_chk_enable;

typedef union {
    struct {
        unsigned int cfg_ptp_enable : 1;    /* [0] */
        unsigned int cfg_udp_enable : 2;    /* [2:1] */
        unsigned int cfg_ptp_type : 2;      /* [4:3] */
        unsigned int cfg_v2_mesg_type1 : 4; /* [8:5] */
        unsigned int cfg_v2_mesg_type0 : 4; /* [12:9] */
        unsigned int rsv : 19;              /* [31:13] */
    } bits;

    unsigned int value;
} u_pa_ptp_syn_ctrl;

typedef union {
    struct {
        unsigned int cfg_et_cn_tag : 16;      /* [15:0] */
        unsigned int cfg_et_cn_tag_pa_en : 1; /* [16] */
        unsigned int rsv : 15;                /* [31:17] */
    } bits;

    unsigned int value;
} u_pa_et_cn_tag;

typedef union {
    struct {
        unsigned int cfg_cnt_clr_ce : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_cnt_clr_ce;

typedef union {
    struct {
        unsigned int cfg_pa_auto_gating_enable : 1; /* [0] */
        unsigned int rsv : 31;                      /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_clk_gating_ctrl;

typedef union {
    struct {
        unsigned int cfg_vlan_tag0 : 16; /* [15:0] */
        unsigned int cfg_vlan_tag1 : 16; /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_vlan_tag0;

typedef union {
    struct {
        unsigned int cfg_vlan_tag2 : 16; /* [15:0] */
        unsigned int cfg_vlan_tag3 : 16; /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_vlan_tag1;

typedef union {
    struct {
        unsigned int cfg_vlan_sel : 1;     /* [0] */
        unsigned int cfg_fst_vlan_num : 2; /* [2:1] */
        unsigned int cfg_sec_vlan_num : 2; /* [4:3] */
        unsigned int rsv : 27;             /* [31:5] */
    } bits;

    unsigned int value;
} u_pa_vlan_select;

typedef union {
    struct {
        unsigned int cfg_pri0_map_tc : 3;        /* [2:0] */
        unsigned int cfg_pri1_map_tc : 3;        /* [5:3] */
        unsigned int cfg_pri2_map_tc : 3;        /* [8:6] */
        unsigned int cfg_pri3_map_tc : 3;        /* [11:9] */
        unsigned int cfg_pri4_map_tc : 3;        /* [14:12] */
        unsigned int cfg_pri5_map_tc : 3;        /* [17:15] */
        unsigned int cfg_pri6_map_tc : 3;        /* [20:18] */
        unsigned int cfg_pri7_map_tc : 3;        /* [23:21] */
        unsigned int cfg_no_pri_map_default : 3; /* [26:24] */
        unsigned int rsv : 5;                    /* [31:27] */
    } bits;

    unsigned int value;
} u_pa_pri_map_tc;

typedef union {
    struct {
        unsigned int cfg_mpls_exp0_map_tc : 3;    /* [2:0] */
        unsigned int cfg_mpls_exp1_map_tc : 3;    /* [5:3] */
        unsigned int cfg_mpls_exp2_map_tc : 3;    /* [8:6] */
        unsigned int cfg_mpls_exp3_map_tc : 3;    /* [11:9] */
        unsigned int cfg_mpls_exp4_map_tc : 3;    /* [14:12] */
        unsigned int cfg_mpls_exp5_map_tc : 3;    /* [17:15] */
        unsigned int cfg_mpls_exp6_map_tc : 3;    /* [20:18] */
        unsigned int cfg_mpls_exp7_map_tc : 3;    /* [23:21] */
        unsigned int cfg_no_mpls_map_default : 3; /* [26:24] */
        unsigned int cfg_mpls_sel : 3;            /* [29:27] */
        unsigned int rsv : 2;                     /* [31:30] */
    } bits;

    unsigned int value;
} u_pa_exp_map_tc;

typedef union {
    struct {
        unsigned int cfg_pa_dscp0_map_tc : 3;     /* [2:0] */
        unsigned int cfg_pa_dscp1_map_tc : 3;     /* [5:3] */
        unsigned int cfg_pa_dscp2_map_tc : 3;     /* [8:6] */
        unsigned int cfg_pa_dscp3_map_tc : 3;     /* [11:9] */
        unsigned int cfg_pa_dscp4_map_tc : 3;     /* [14:12] */
        unsigned int cfg_pa_dscp5_map_tc : 3;     /* [17:15] */
        unsigned int cfg_pa_dscp6_map_tc : 3;     /* [20:18] */
        unsigned int cfg_pa_dscp7_map_tc : 3;     /* [23:21] */
        unsigned int cfg_no_dscp_map_default : 6; /* [29:24] */
        unsigned int rsv : 2;                     /* [31:30] */
    } bits;

    unsigned int value;
} u_pa_dscp_map_tc_0;

typedef union {
    struct {
        unsigned int cfg_pa_dscp8_map_tc : 3;  /* [2:0] */
        unsigned int cfg_pa_dscp9_map_tc : 3;  /* [5:3] */
        unsigned int cfg_pa_dscp10_map_tc : 3; /* [8:6] */
        unsigned int cfg_pa_dscp11_map_tc : 3; /* [11:9] */
        unsigned int cfg_pa_dscp12_map_tc : 3; /* [14:12] */
        unsigned int cfg_pa_dscp13_map_tc : 3; /* [17:15] */
        unsigned int cfg_pa_dscp14_map_tc : 3; /* [20:18] */
        unsigned int cfg_pa_dscp15_map_tc : 3; /* [23:21] */
        unsigned int rsv : 8;                  /* [31:24] */
    } bits;

    unsigned int value;
} u_pa_dscp_map_tc_1;

typedef union {
    struct {
        unsigned int cfg_pa_dscp16_map_tc : 3; /* [2:0] */
        unsigned int cfg_pa_dscp17_map_tc : 3; /* [5:3] */
        unsigned int cfg_pa_dscp18_map_tc : 3; /* [8:6] */
        unsigned int cfg_pa_dscp19_map_tc : 3; /* [11:9] */
        unsigned int cfg_pa_dscp20_map_tc : 3; /* [14:12] */
        unsigned int cfg_pa_dscp21_map_tc : 3; /* [17:15] */
        unsigned int cfg_pa_dscp22_map_tc : 3; /* [20:18] */
        unsigned int cfg_pa_dscp23_map_tc : 3; /* [23:21] */
        unsigned int rsv : 8;                  /* [31:24] */
    } bits;

    unsigned int value;
} u_pa_dscp_map_tc_2;

typedef union {
    struct {
        unsigned int cfg_pa_dscp24_map_tc : 3; /* [2:0] */
        unsigned int cfg_pa_dscp25_map_tc : 3; /* [5:3] */
        unsigned int cfg_pa_dscp26_map_tc : 3; /* [8:6] */
        unsigned int cfg_pa_dscp27_map_tc : 3; /* [11:9] */
        unsigned int cfg_pa_dscp28_map_tc : 3; /* [14:12] */
        unsigned int cfg_pa_dscp29_map_tc : 3; /* [17:15] */
        unsigned int cfg_pa_dscp30_map_tc : 3; /* [20:18] */
        unsigned int cfg_pa_dscp31_map_tc : 3; /* [23:21] */
        unsigned int rsv : 8;                  /* [31:24] */
    } bits;

    unsigned int value;
} u_pa_dscp_map_tc_3;

typedef union {
    struct {
        unsigned int cfg_pa_dscp32_map_tc : 3; /* [2:0] */
        unsigned int cfg_pa_dscp33_map_tc : 3; /* [5:3] */
        unsigned int cfg_pa_dscp34_map_tc : 3; /* [8:6] */
        unsigned int cfg_pa_dscp35_map_tc : 3; /* [11:9] */
        unsigned int cfg_pa_dscp36_map_tc : 3; /* [14:12] */
        unsigned int cfg_pa_dscp37_map_tc : 3; /* [17:15] */
        unsigned int cfg_pa_dscp38_map_tc : 3; /* [20:18] */
        unsigned int cfg_pa_dscp39_map_tc : 3; /* [23:21] */
        unsigned int rsv : 8;                  /* [31:24] */
    } bits;

    unsigned int value;
} u_pa_dscp_map_tc_4;

typedef union {
    struct {
        unsigned int cfg_pa_dscp40_map_tc : 3; /* [2:0] */
        unsigned int cfg_pa_dscp41_map_tc : 3; /* [5:3] */
        unsigned int cfg_pa_dscp42_map_tc : 3; /* [8:6] */
        unsigned int cfg_pa_dscp43_map_tc : 3; /* [11:9] */
        unsigned int cfg_pa_dscp44_map_tc : 3; /* [14:12] */
        unsigned int cfg_pa_dscp45_map_tc : 3; /* [17:15] */
        unsigned int cfg_pa_dscp46_map_tc : 3; /* [20:18] */
        unsigned int cfg_pa_dscp47_map_tc : 3; /* [23:21] */
        unsigned int rsv : 8;                  /* [31:24] */
    } bits;

    unsigned int value;
} u_pa_dscp_map_tc_5;

typedef union {
    struct {
        unsigned int cfg_pa_dscp48_map_tc : 3; /* [2:0] */
        unsigned int cfg_pa_dscp49_map_tc : 3; /* [5:3] */
        unsigned int cfg_pa_dscp50_map_tc : 3; /* [8:6] */
        unsigned int cfg_pa_dscp51_map_tc : 3; /* [11:9] */
        unsigned int cfg_pa_dscp52_map_tc : 3; /* [14:12] */
        unsigned int cfg_pa_dscp53_map_tc : 3; /* [17:15] */
        unsigned int cfg_pa_dscp54_map_tc : 3; /* [20:18] */
        unsigned int cfg_pa_dscp55_map_tc : 3; /* [23:21] */
        unsigned int rsv : 8;                  /* [31:24] */
    } bits;

    unsigned int value;
} u_pa_dscp_map_tc_6;

typedef union {
    struct {
        unsigned int cfg_pa_dscp56_map_tc : 3; /* [2:0] */
        unsigned int cfg_pa_dscp57_map_tc : 3; /* [5:3] */
        unsigned int cfg_pa_dscp58_map_tc : 3; /* [8:6] */
        unsigned int cfg_pa_dscp59_map_tc : 3; /* [11:9] */
        unsigned int cfg_pa_dscp60_map_tc : 3; /* [14:12] */
        unsigned int cfg_pa_dscp61_map_tc : 3; /* [17:15] */
        unsigned int cfg_pa_dscp62_map_tc : 3; /* [20:18] */
        unsigned int cfg_pa_dscp63_map_tc : 3; /* [23:21] */
        unsigned int rsv : 8;                  /* [31:24] */
    } bits;

    unsigned int value;
} u_pa_dscp_map_tc_7;

typedef union {
    struct {
        unsigned int cfg_tc_vlan_sel : 1; /* [0] */
        unsigned int cfg_tc_map_sel : 2;  /* [2:1] */
        unsigned int rsv : 29;            /* [31:3] */
    } bits;

    unsigned int value;
} u_pa_tc_sel;

typedef union {
    struct {
        unsigned int cfg_rw_rsv0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_pa_rw_rsv0;

typedef union {
    struct {
        unsigned int cfg_rw_rsv1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_pa_rw_rsv1;

typedef union {
    struct {
        unsigned int cfg_rw_rsv2 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_pa_rw_rsv2;

typedef union {
    struct {
        unsigned int cfg_rw_rsv3 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_pa_rw_rsv3;

typedef union {
    struct {
        unsigned int cfg_l2_err_cfi_en : 1;           /* [0] */
        unsigned int cfg_l2_err_snap_en : 1;          /* [1] */
        unsigned int cfg_l2_err_drop_en : 1;          /* [2] */
        unsigned int cfg_l3_err_ipv4_cks_en : 1;      /* [3] */
        unsigned int cfg_l3_err_ipv6_ttl_en : 1;      /* [4] */
        unsigned int cfg_l3_err_ipv6_len_act_en : 1;  /* [5] */
        unsigned int cfg_l3_err_ipv6_len_sctp_en : 1; /* [6] */
        unsigned int cfg_l3_err_ipv6_len_udp_en : 1;  /* [7] */
        unsigned int cfg_l3_err_ipv6_len_tcp_en : 1;  /* [8] */
        unsigned int cfg_l3_err_ipv6_drop_en : 1;     /* [9] */
        unsigned int cfg_l3_err_ipv6_ver_en : 1;      /* [10] */
        unsigned int cfg_l3_err_ipv4_ttl_en : 1;      /* [11] */
        unsigned int cfg_l3_err_ipv4_len_act_en : 1;  /* [12] */
        unsigned int cfg_l3_err_ipv4_len_sctp_en : 1; /* [13] */
        unsigned int cfg_l3_err_ipv4_len_udp_en : 1;  /* [14] */
        unsigned int cfg_l3_err_ipv4_len_tcp_en : 1;  /* [15] */
        unsigned int cfg_l3_err_ipv4_len_en : 1;      /* [16] */
        unsigned int cfg_l3_err_ipv4_ihl_en : 1;      /* [17] */
        unsigned int cfg_l3_err_ipv4_drop_en : 1;     /* [18] */
        unsigned int cfg_l3_err_ipv4_ver_en : 1;      /* [19] */
        unsigned int rsv : 12;                        /* [31:20] */
    } bits;

    unsigned int value;
} u_pa_l23_pro_check_enable;

typedef union {
    struct {
        unsigned int cfg_l4_err_tcp_cks_en : 1;         /* [0] */
        unsigned int cfg_l4_err_ipv6_udp_cks_en : 1;    /* [1] */
        unsigned int cfg_l4_err_ipv4_udp_cks_en : 1;    /* [2] */
        unsigned int cfg_l4_err_tcp_less_ipv6_en : 1;   /* [3] */
        unsigned int cfg_l4_err_tcp_less_ipv4_en : 1;   /* [4] */
        unsigned int cfg_l4_err_udp_less_ipv6_en : 1;   /* [5] */
        unsigned int cfg_l4_err_udp_less_ipv4_en : 1;   /* [6] */
        unsigned int cfg_l4_err_sctp_len_12_en : 1;     /* [7] */
        unsigned int cfg_l4_err_tcp_len_pay_en : 1;     /* [8] */
        unsigned int cfg_l4_err_tcp_len_pro_len_en : 1; /* [9] */
        unsigned int cfg_l4_err_udp_len_pay_en : 1;     /* [10] */
        unsigned int cfg_l4_err_udp_len_pro_len_en : 1; /* [11] */
        unsigned int cfg_l4_err_sctp_cks_en : 1;        /* [12] */
        unsigned int rsv : 19;                          /* [31:13] */
    } bits;

    unsigned int value;
} U_PA_L4_PRO_CHECK_ENABLE;

typedef union {
    struct {
        unsigned int l2_err_cfi_his : 1;           /* [0] */
        unsigned int l2_err_snap_his : 1;          /* [1] */
        unsigned int l2_err_drop_his : 1;          /* [2] */
        unsigned int l3_err_ipv4_cks_his : 1;      /* [3] */
        unsigned int l3_err_ipv6_ttl_his : 1;      /* [4] */
        unsigned int l3_err_ipv6_len_act_his : 1;  /* [5] */
        unsigned int l3_err_ipv6_len_sctp_his : 1; /* [6] */
        unsigned int l3_err_ipv6_len_udp_his : 1;  /* [7] */
        unsigned int l3_err_ipv6_len_tcp_his : 1;  /* [8] */
        unsigned int l3_err_ipv6_drop_his : 1;     /* [9] */
        unsigned int l3_err_ipv6_ver_his : 1;      /* [10] */
        unsigned int l3_err_ipv4_ttl_his : 1;      /* [11] */
        unsigned int l3_err_ipv4_len_act_his : 1;  /* [12] */
        unsigned int l3_err_ipv4_len_sctp_his : 1; /* [13] */
        unsigned int l3_err_ipv4_len_udp_his : 1;  /* [14] */
        unsigned int l3_err_ipv4_len_tcp_his : 1;  /* [15] */
        unsigned int l3_err_ipv4_len_his : 1;      /* [16] */
        unsigned int l3_err_ipv4_ihl_his : 1;      /* [17] */
        unsigned int l3_err_ipv4_drop_his : 1;     /* [18] */
        unsigned int l3_err_ipv4_ver_his : 1;      /* [19] */
        unsigned int rsv : 12;                     /* [31:20] */
    } bits;

    unsigned int value;
} u_pa_l23_his_pro_err;

typedef union {
    struct {
        unsigned int l4_err_tcp_cks_his : 1;         /* [0] */
        unsigned int l4_err_ipv6_udp_cks_his : 1;    /* [1] */
        unsigned int l4_err_ipv4_udp_cks_his : 1;    /* [2] */
        unsigned int l4_err_tcp_less_ipv6_his : 1;   /* [3] */
        unsigned int l4_err_tcp_less_ipv4_his : 1;   /* [4] */
        unsigned int l4_err_udp_less_ipv6_his : 1;   /* [5] */
        unsigned int l4_err_udp_less_ipv4_his : 1;   /* [6] */
        unsigned int l4_err_sctp_len_12_his : 1;     /* [7] */
        unsigned int l4_err_tcp_len_pay_his : 1;     /* [8] */
        unsigned int l4_err_tcp_len_pro_len_his : 1; /* [9] */
        unsigned int l4_err_udp_len_pay_his : 1;     /* [10] */
        unsigned int l4_err_udp_len_pro_len_his : 1; /* [11] */
        unsigned int l4_err_sctp_cks_his : 1;        /* [12] */
        unsigned int rsv : 19;                       /* [31:13] */
    } bits;

    unsigned int value;
} u_pa_l4_his_pro_err;

typedef union {
    struct {
        unsigned int pa_l2e_pkt : 16; /* [15:0] */
        unsigned int rsv : 16;        /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_l2e_pkt;

typedef union {
    struct {
        unsigned int pa_l3e_pkt : 16; /* [15:0] */
        unsigned int rsv : 16;        /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_l3e_pkt;

typedef union {
    struct {
        unsigned int pa_l4e_pkt : 16; /* [15:0] */
        unsigned int rsv : 16;        /* [31:16] */
    } bits;

    unsigned int value;
} u_pa_l4e_pkt;

typedef union {
    struct {
        unsigned int cfg_error_port_dfx : 2; /* [1:0] */
        unsigned int rsv : 30;               /* [31:2] */
    } bits;

    unsigned int value;
} u_pa_dfx_port;

typedef union {
    struct {
        unsigned int pa_key_dfx_sts : 1; /* [0] */
        unsigned int rsv : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_pa_key_dfx_clr;

typedef union {
    struct {
        unsigned int cfg_key_dfx_sel : 6; /* [5:0] */
        unsigned int rsv : 26;            /* [31:6] */
    } bits;

    unsigned int value;
} u_pa_key_sel;

typedef union {
    struct {
        unsigned int cfg_key_value : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_pa_key_value;

#endif  // __REG_PA_H__