/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_ssu
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_SSU_H__
#define __REG_SSU_H__

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_enable : 1;      /* [0] */
        unsigned int mem_ecc_mbit_int_enable : 1;      /* [1] */
        unsigned int fifo_rd_int_enable : 1;           /* [2] */
        unsigned int fifo_wr_int_enable : 1;           /* [3] */
        unsigned int host_pkt_sof_mismatch_enable : 1; /* [4] */
        unsigned int host_pkt_eof_mismatch_enable : 1; /* [5] */
        unsigned int host_pkt_key_mismatch_enable : 1; /* [6] */
        unsigned int mac_pkt_sof_mismatch_enable : 1;  /* [7] */
        unsigned int mac_pkt_eof_mismatch_enable : 1;  /* [8] */
        unsigned int mac_pkt_key_mismatch_enable : 1;  /* [9] */
        unsigned int rpu_credit_err_int_enable : 1;    /* [10] */
        unsigned int bmp_rsc_return_int_enable : 1;    /* [11] */
        unsigned int bmp_age_err_int_enable : 1;       /* [12] */
        unsigned int rocee_credit_err_int_enable : 1;  /* [13] */
        unsigned int qm_free_rsc_int_enable : 1;       /* [14] */
        unsigned int rsv_0 : 17;                       /* [31:15] */
    } bits;

    unsigned int value;
} u_ssu_int_enable;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_status : 1;      /* [0] */
        unsigned int mem_ecc_mbit_int_status : 1;      /* [1] */
        unsigned int fifo_rd_int_status : 1;           /* [2] */
        unsigned int fifo_wr_int_status : 1;           /* [3] */
        unsigned int host_pkt_sof_mismatch_status : 1; /* [4] */
        unsigned int host_pkt_eof_mismatch_status : 1; /* [5] */
        unsigned int host_pkt_key_mismatch_status : 1; /* [6] */
        unsigned int mac_pkt_sof_mismatch_status : 1;  /* [7] */
        unsigned int mac_pkt_eof_mismatch_status : 1;  /* [8] */
        unsigned int mac_pkt_key_mismatch_status : 1;  /* [9] */
        unsigned int rpu_credit_err_int_status : 1;    /* [10] */
        unsigned int bmp_rsc_return_int_status : 1;    /* [11] */
        unsigned int bmp_age_err_int_status : 1;       /* [12] */
        unsigned int rocee_credit_err_int_status : 1;  /* [13] */
        unsigned int qm_free_rsc_int_status : 1;       /* [14] */
        unsigned int rsv_1 : 17;                       /* [31:15] */
    } bits;

    unsigned int value;
} u_ssu_int_status;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_set : 1;      /* [0] */
        unsigned int mem_ecc_mbit_int_set : 1;      /* [1] */
        unsigned int fifo_rd_int_set : 1;           /* [2] */
        unsigned int fifo_wr_int_set : 1;           /* [3] */
        unsigned int host_pkt_sof_mismatch_set : 1; /* [4] */
        unsigned int host_pkt_eof_mismatch_set : 1; /* [5] */
        unsigned int host_pkt_key_mismatch_set : 1; /* [6] */
        unsigned int mac_pkt_sof_mismatch_set : 1;  /* [7] */
        unsigned int mac_pkt_eof_mismatch_set : 1;  /* [8] */
        unsigned int mac_pkt_key_mismatch_set : 1;  /* [9] */
        unsigned int rpu_credit_err_int_set : 1;    /* [10] */
        unsigned int bmp_rsc_return_int_set : 1;    /* [11] */
        unsigned int bmp_age_err_int_set : 1;       /* [12] */
        unsigned int rocee_credit_err_int_set : 1;  /* [13] */
        unsigned int qm_free_rsc_int_set : 1;       /* [14] */
        unsigned int rsv_2 : 17;                    /* [31:15] */
    } bits;

    unsigned int value;
} u_ssu_int_set;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int : 1;      /* [0] */
        unsigned int mem_ecc_mbit_int : 1;      /* [1] */
        unsigned int fifo_rd_int : 1;           /* [2] */
        unsigned int fifo_wr_int : 1;           /* [3] */
        unsigned int host_pkt_sof_mismatch : 1; /* [4] */
        unsigned int host_pkt_eof_mismatch : 1; /* [5] */
        unsigned int host_pkt_key_mismatch : 1; /* [6] */
        unsigned int mac_pkt_sof_mismatch : 1;  /* [7] */
        unsigned int mac_pkt_eof_mismatch : 1;  /* [8] */
        unsigned int mac_pkt_key_mismatch : 1;  /* [9] */
        unsigned int rpu_credit_err_int : 1;    /* [10] */
        unsigned int bmp_rsc_return_int : 1;    /* [11] */
        unsigned int bmp_age_err_int : 1;       /* [12] */
        unsigned int rocee_credit_err_int : 1;  /* [13] */
        unsigned int qm_free_rsc_int : 1;       /* [14] */
        unsigned int rsv_3 : 17;                /* [31:15] */
    } bits;

    unsigned int value;
} u_ssu_int_source;

typedef union {
    struct {
        unsigned int mem_ecc_req0 : 2;  /* [1:0] */
        unsigned int mem_ecc_req1 : 2;  /* [3:2] */
        unsigned int mem_ecc_req2 : 2;  /* [5:4] */
        unsigned int mem_ecc_req3 : 2;  /* [7:6] */
        unsigned int mem_ecc_req4 : 2;  /* [9:8] */
        unsigned int mem_ecc_req5 : 2;  /* [11:10] */
        unsigned int mem_ecc_req6 : 2;  /* [13:12] */
        unsigned int mem_ecc_req7 : 2;  /* [15:14] */
        unsigned int mem_ecc_req8 : 2;  /* [17:16] */
        unsigned int mem_ecc_req9 : 2;  /* [19:18] */
        unsigned int mem_ecc_req10 : 2; /* [21:20] */
        unsigned int mem_ecc_req11 : 2; /* [23:22] */
        unsigned int mem_ecc_req12 : 2; /* [25:24] */
        unsigned int mem_ecc_req13 : 2; /* [27:26] */
        unsigned int mem_ecc_req14 : 2; /* [29:28] */
        unsigned int mem_ecc_req15 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_mem_err_insert0;

typedef union {
    struct {
        unsigned int mem_ecc_req16 : 2; /* [1:0] */
        unsigned int mem_ecc_req17 : 2; /* [3:2] */
        unsigned int mem_ecc_req18 : 2; /* [5:4] */
        unsigned int mem_ecc_req19 : 2; /* [7:6] */
        unsigned int mem_ecc_req20 : 2; /* [9:8] */
        unsigned int mem_ecc_req21 : 2; /* [11:10] */
        unsigned int mem_ecc_req22 : 2; /* [13:12] */
        unsigned int mem_ecc_req23 : 2; /* [15:14] */
        unsigned int mem_ecc_req24 : 2; /* [17:16] */
        unsigned int mem_ecc_req25 : 2; /* [19:18] */
        unsigned int mem_ecc_req26 : 2; /* [21:20] */
        unsigned int mem_ecc_req27 : 2; /* [23:22] */
        unsigned int mem_ecc_req28 : 2; /* [25:24] */
        unsigned int mem_ecc_req29 : 2; /* [27:26] */
        unsigned int mem_ecc_req30 : 2; /* [29:28] */
        unsigned int mem_ecc_req31 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_mem_err_insert1;

typedef union {
    struct {
        unsigned int mem_ecc_req32 : 2; /* [1:0] */
        unsigned int mem_ecc_req33 : 2; /* [3:2] */
        unsigned int mem_ecc_req34 : 2; /* [5:4] */
        unsigned int mem_ecc_req35 : 2; /* [7:6] */
        unsigned int rsv_4 : 24;        /* [31:8] */
    } bits;

    unsigned int value;
} u_ssu_mem_err_insert2;

typedef union {
    struct {
        unsigned int ecc_sbit_err_addr : 16; /* [15:0] */
        unsigned int ecc_sbit_err_id : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_ssu_mem_sbit_err_info;

typedef union {
    struct {
        unsigned int ecc_mbit_err_addr : 16; /* [15:0] */
        unsigned int ecc_mbit_err_id : 16;   /* [31:16] */
    } bits;

    unsigned int value;
} u_ssu_mem_mbit_err_info;

typedef union {
    struct {
        unsigned int fifo_wr_int0 : 1;  /* [0] */
        unsigned int fifo_wr_int1 : 1;  /* [1] */
        unsigned int fifo_wr_int2 : 1;  /* [2] */
        unsigned int fifo_wr_int3 : 1;  /* [3] */
        unsigned int fifo_wr_int4 : 1;  /* [4] */
        unsigned int fifo_wr_int5 : 1;  /* [5] */
        unsigned int fifo_wr_int6 : 1;  /* [6] */
        unsigned int fifo_wr_int7 : 1;  /* [7] */
        unsigned int fifo_wr_int8 : 1;  /* [8] */
        unsigned int fifo_wr_int9 : 1;  /* [9] */
        unsigned int fifo_wr_int10 : 1; /* [10] */
        unsigned int fifo_wr_int11 : 1; /* [11] */
        unsigned int fifo_wr_int12 : 1; /* [12] */
        unsigned int fifo_wr_int13 : 1; /* [13] */
        unsigned int fifo_wr_int14 : 1; /* [14] */
        unsigned int fifo_wr_int15 : 1; /* [15] */
        unsigned int fifo_wr_int16 : 1; /* [16] */
        unsigned int fifo_wr_int17 : 1; /* [17] */
        unsigned int fifo_wr_int18 : 1; /* [18] */
        unsigned int fifo_wr_int19 : 1; /* [19] */
        unsigned int fifo_wr_int20 : 1; /* [20] */
        unsigned int fifo_wr_int21 : 1; /* [21] */
        unsigned int fifo_wr_int22 : 1; /* [22] */
        unsigned int fifo_wr_int23 : 1; /* [23] */
        unsigned int fifo_wr_int24 : 1; /* [24] */
        unsigned int fifo_wr_int25 : 1; /* [25] */
        unsigned int fifo_wr_int26 : 1; /* [26] */
        unsigned int fifo_wr_int27 : 1; /* [27] */
        unsigned int fifo_wr_int28 : 1; /* [28] */
        unsigned int fifo_wr_int29 : 1; /* [29] */
        unsigned int fifo_wr_int30 : 1; /* [30] */
        unsigned int fifo_wr_int31 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_fifo_wr_int0;

typedef union {
    struct {
        unsigned int fifo_wr_int32 : 1; /* [0] */
        unsigned int fifo_wr_int33 : 1; /* [1] */
        unsigned int fifo_wr_int34 : 1; /* [2] */
        unsigned int fifo_wr_int35 : 1; /* [3] */
        unsigned int fifo_wr_int36 : 1; /* [4] */
        unsigned int fifo_wr_int37 : 1; /* [5] */
        unsigned int fifo_wr_int38 : 1; /* [6] */
        unsigned int fifo_wr_int39 : 1; /* [7] */
        unsigned int fifo_wr_int40 : 1; /* [8] */
        unsigned int fifo_wr_int41 : 1; /* [9] */
        unsigned int fifo_wr_int42 : 1; /* [10] */
        unsigned int fifo_wr_int43 : 1; /* [11] */
        unsigned int fifo_wr_int44 : 1; /* [12] */
        unsigned int fifo_wr_int45 : 1; /* [13] */
        unsigned int fifo_wr_int46 : 1; /* [14] */
        unsigned int fifo_wr_int47 : 1; /* [15] */
        unsigned int fifo_wr_int48 : 1; /* [16] */
        unsigned int fifo_wr_int49 : 1; /* [17] */
        unsigned int fifo_wr_int50 : 1; /* [18] */
        unsigned int fifo_wr_int51 : 1; /* [19] */
        unsigned int fifo_wr_int52 : 1; /* [20] */
        unsigned int fifo_wr_int53 : 1; /* [21] */
        unsigned int fifo_wr_int54 : 1; /* [22] */
        unsigned int fifo_wr_int55 : 1; /* [23] */
        unsigned int fifo_wr_int56 : 1; /* [24] */
        unsigned int fifo_wr_int57 : 1; /* [25] */
        unsigned int fifo_wr_int58 : 1; /* [26] */
        unsigned int fifo_wr_int59 : 1; /* [27] */
        unsigned int fifo_wr_int60 : 1; /* [28] */
        unsigned int fifo_wr_int61 : 1; /* [29] */
        unsigned int fifo_wr_int62 : 1; /* [30] */
        unsigned int fifo_wr_int63 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_fifo_wr_int1;

typedef union {
    struct {
        unsigned int fifo_wr_int64 : 1; /* [0] */
        unsigned int fifo_wr_int65 : 1; /* [1] */
        unsigned int fifo_wr_int66 : 1; /* [2] */
        unsigned int fifo_wr_int67 : 1; /* [3] */
        unsigned int fifo_wr_int68 : 1; /* [4] */
        unsigned int fifo_wr_int69 : 1; /* [5] */
        unsigned int fifo_wr_int70 : 1; /* [6] */
        unsigned int fifo_wr_int71 : 1; /* [7] */
        unsigned int fifo_wr_int72 : 1; /* [8] */
        unsigned int fifo_wr_int73 : 1; /* [9] */
        unsigned int fifo_wr_int74 : 1; /* [10] */
        unsigned int fifo_wr_int75 : 1; /* [11] */
        unsigned int fifo_wr_int76 : 1; /* [12] */
        unsigned int fifo_wr_int77 : 1; /* [13] */
        unsigned int fifo_wr_int78 : 1; /* [14] */
        unsigned int fifo_wr_int79 : 1; /* [15] */
        unsigned int fifo_wr_int80 : 1; /* [16] */
        unsigned int fifo_wr_int81 : 1; /* [17] */
        unsigned int fifo_wr_int82 : 1; /* [18] */
        unsigned int fifo_wr_int83 : 1; /* [19] */
        unsigned int fifo_wr_int84 : 1; /* [20] */
        unsigned int fifo_wr_int85 : 1; /* [21] */
        unsigned int fifo_wr_int86 : 1; /* [22] */
        unsigned int fifo_wr_int87 : 1; /* [23] */
        unsigned int fifo_wr_int88 : 1; /* [24] */
        unsigned int fifo_wr_int89 : 1; /* [25] */
        unsigned int fifo_wr_int90 : 1; /* [26] */
        unsigned int fifo_wr_int91 : 1; /* [27] */
        unsigned int fifo_wr_int92 : 1; /* [28] */
        unsigned int fifo_wr_int93 : 1; /* [29] */
        unsigned int fifo_wr_int94 : 1; /* [30] */
        unsigned int fifo_wr_int95 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_fifo_wr_int2;

typedef union {
    struct {
        unsigned int fifo_wr_int96 : 1;  /* [0] */
        unsigned int fifo_wr_int97 : 1;  /* [1] */
        unsigned int fifo_wr_int98 : 1;  /* [2] */
        unsigned int fifo_wr_int99 : 1;  /* [3] */
        unsigned int fifo_wr_int100 : 1; /* [4] */
        unsigned int fifo_wr_int101 : 1; /* [5] */
        unsigned int fifo_wr_int102 : 1; /* [6] */
        unsigned int fifo_wr_int103 : 1; /* [7] */
        unsigned int fifo_wr_int104 : 1; /* [8] */
        unsigned int fifo_wr_int105 : 1; /* [9] */
        unsigned int fifo_wr_int106 : 1; /* [10] */
        unsigned int fifo_wr_int107 : 1; /* [11] */
        unsigned int fifo_wr_int108 : 1; /* [12] */
        unsigned int fifo_wr_int109 : 1; /* [13] */
        unsigned int fifo_wr_int110 : 1; /* [14] */
        unsigned int fifo_wr_int111 : 1; /* [15] */
        unsigned int fifo_wr_int112 : 1; /* [16] */
        unsigned int fifo_wr_int113 : 1; /* [17] */
        unsigned int fifo_wr_int114 : 1; /* [18] */
        unsigned int fifo_wr_int115 : 1; /* [19] */
        unsigned int fifo_wr_int116 : 1; /* [20] */
        unsigned int fifo_wr_int117 : 1; /* [21] */
        unsigned int fifo_wr_int118 : 1; /* [22] */
        unsigned int fifo_wr_int119 : 1; /* [23] */
        unsigned int fifo_wr_int120 : 1; /* [24] */
        unsigned int fifo_wr_int121 : 1; /* [25] */
        unsigned int fifo_wr_int122 : 1; /* [26] */
        unsigned int fifo_wr_int123 : 1; /* [27] */
        unsigned int fifo_wr_int124 : 1; /* [28] */
        unsigned int fifo_wr_int125 : 1; /* [29] */
        unsigned int fifo_wr_int126 : 1; /* [30] */
        unsigned int fifo_wr_int127 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_fifo_wr_int3;

typedef union {
    struct {
        unsigned int fifo_wr_int128 : 1; /* [0] */
        unsigned int fifo_wr_int129 : 1; /* [1] */
        unsigned int fifo_wr_int130 : 1; /* [2] */
        unsigned int fifo_wr_int131 : 1; /* [3] */
        unsigned int fifo_wr_int132 : 1; /* [4] */
        unsigned int fifo_wr_int133 : 1; /* [5] */
        unsigned int fifo_wr_int134 : 1; /* [6] */
        unsigned int fifo_wr_int135 : 1; /* [7] */
        unsigned int fifo_wr_int136 : 1; /* [8] */
        unsigned int fifo_wr_int137 : 1; /* [9] */
        unsigned int fifo_wr_int138 : 1; /* [10] */
        unsigned int fifo_wr_int139 : 1; /* [11] */
        unsigned int fifo_wr_int140 : 1; /* [12] */
        unsigned int fifo_wr_int141 : 1; /* [13] */
        unsigned int fifo_wr_int142 : 1; /* [14] */
        unsigned int fifo_wr_int143 : 1; /* [15] */
        unsigned int fifo_wr_int144 : 1; /* [16] */
        unsigned int fifo_wr_int145 : 1; /* [17] */
        unsigned int fifo_wr_int146 : 1; /* [18] */
        unsigned int fifo_wr_int147 : 1; /* [19] */
        unsigned int fifo_wr_int148 : 1; /* [20] */
        unsigned int fifo_wr_int149 : 1; /* [21] */
        unsigned int fifo_wr_int150 : 1; /* [22] */
        unsigned int fifo_wr_int151 : 1; /* [23] */
        unsigned int fifo_wr_int152 : 1; /* [24] */
        unsigned int fifo_wr_int153 : 1; /* [25] */
        unsigned int fifo_wr_int154 : 1; /* [26] */
        unsigned int fifo_wr_int155 : 1; /* [27] */
        unsigned int fifo_wr_int156 : 1; /* [28] */
        unsigned int fifo_wr_int157 : 1; /* [29] */
        unsigned int fifo_wr_int158 : 1; /* [30] */
        unsigned int fifo_wr_int159 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_fifo_wr_int4;

typedef union {
    struct {
        unsigned int fifo_wr_int160 : 1; /* [0] */
        unsigned int fifo_wr_int161 : 1; /* [1] */
        unsigned int fifo_wr_int162 : 1; /* [2] */
        unsigned int fifo_wr_int163 : 1; /* [3] */
        unsigned int fifo_wr_int164 : 1; /* [4] */
        unsigned int fifo_wr_int165 : 1; /* [5] */
        unsigned int fifo_wr_int166 : 1; /* [6] */
        unsigned int fifo_wr_int167 : 1; /* [7] */
        unsigned int fifo_wr_int168 : 1; /* [8] */
        unsigned int fifo_wr_int169 : 1; /* [9] */
        unsigned int fifo_wr_int170 : 1; /* [10] */
        unsigned int fifo_wr_int171 : 1; /* [11] */
        unsigned int fifo_wr_int172 : 1; /* [12] */
        unsigned int fifo_wr_int173 : 1; /* [13] */
        unsigned int fifo_wr_int174 : 1; /* [14] */
        unsigned int fifo_wr_int175 : 1; /* [15] */
        unsigned int fifo_wr_int176 : 1; /* [16] */
        unsigned int fifo_wr_int177 : 1; /* [17] */
        unsigned int fifo_wr_int178 : 1; /* [18] */
        unsigned int fifo_wr_int179 : 1; /* [19] */
        unsigned int fifo_wr_int180 : 1; /* [20] */
        unsigned int fifo_wr_int181 : 1; /* [21] */
        unsigned int fifo_wr_int182 : 1; /* [22] */
        unsigned int fifo_wr_int183 : 1; /* [23] */
        unsigned int fifo_wr_int184 : 1; /* [24] */
        unsigned int fifo_wr_int185 : 1; /* [25] */
        unsigned int fifo_wr_int186 : 1; /* [26] */
        unsigned int fifo_wr_int187 : 1; /* [27] */
        unsigned int fifo_wr_int188 : 1; /* [28] */
        unsigned int fifo_wr_int189 : 1; /* [29] */
        unsigned int fifo_wr_int190 : 1; /* [30] */
        unsigned int fifo_wr_int191 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_fifo_wr_int5;

typedef union {
    struct {
        unsigned int fifo_wr_int192 : 1; /* [0] */
        unsigned int fifo_wr_int193 : 1; /* [1] */
        unsigned int fifo_wr_int194 : 1; /* [2] */
        unsigned int fifo_wr_int195 : 1; /* [3] */
        unsigned int fifo_wr_int196 : 1; /* [4] */
        unsigned int fifo_wr_int197 : 1; /* [5] */
        unsigned int fifo_wr_int198 : 1; /* [6] */
        unsigned int fifo_wr_int199 : 1; /* [7] */
        unsigned int fifo_wr_int200 : 1; /* [8] */
        unsigned int fifo_wr_int201 : 1; /* [9] */
        unsigned int fifo_wr_int202 : 1; /* [10] */
        unsigned int fifo_wr_int203 : 1; /* [11] */
        unsigned int fifo_wr_int204 : 1; /* [12] */
        unsigned int fifo_wr_int205 : 1; /* [13] */
        unsigned int fifo_wr_int206 : 1; /* [14] */
        unsigned int fifo_wr_int207 : 1; /* [15] */
        unsigned int fifo_wr_int208 : 1; /* [16] */
        unsigned int fifo_wr_int209 : 1; /* [17] */
        unsigned int fifo_wr_int210 : 1; /* [18] */
        unsigned int fifo_wr_int211 : 1; /* [19] */
        unsigned int fifo_wr_int212 : 1; /* [20] */
        unsigned int fifo_wr_int213 : 1; /* [21] */
        unsigned int fifo_wr_int214 : 1; /* [22] */
        unsigned int fifo_wr_int215 : 1; /* [23] */
        unsigned int fifo_wr_int216 : 1; /* [24] */
        unsigned int fifo_wr_int217 : 1; /* [25] */
        unsigned int fifo_wr_int218 : 1; /* [26] */
        unsigned int fifo_wr_int219 : 1; /* [27] */
        unsigned int fifo_wr_int220 : 1; /* [28] */
        unsigned int fifo_wr_int221 : 1; /* [29] */
        unsigned int fifo_wr_int222 : 1; /* [30] */
        unsigned int fifo_wr_int223 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_fifo_wr_int6;

typedef union {
    struct {
        unsigned int fifo_wr_int224 : 1; /* [0] */
        unsigned int fifo_wr_int225 : 1; /* [1] */
        unsigned int fifo_wr_int226 : 1; /* [2] */
        unsigned int fifo_wr_int227 : 1; /* [3] */
        unsigned int fifo_wr_int228 : 1; /* [4] */
        unsigned int fifo_wr_int229 : 1; /* [5] */
        unsigned int fifo_wr_int230 : 1; /* [6] */
        unsigned int fifo_wr_int231 : 1; /* [7] */
        unsigned int fifo_wr_int232 : 1; /* [8] */
        unsigned int fifo_wr_int233 : 1; /* [9] */
        unsigned int fifo_wr_int234 : 1; /* [10] */
        unsigned int fifo_wr_int235 : 1; /* [11] */
        unsigned int fifo_wr_int236 : 1; /* [12] */
        unsigned int fifo_wr_int237 : 1; /* [13] */
        unsigned int fifo_wr_int238 : 1; /* [14] */
        unsigned int fifo_wr_int239 : 1; /* [15] */
        unsigned int fifo_wr_int240 : 1; /* [16] */
        unsigned int fifo_wr_int241 : 1; /* [17] */
        unsigned int fifo_wr_int242 : 1; /* [18] */
        unsigned int fifo_wr_int243 : 1; /* [19] */
        unsigned int fifo_wr_int244 : 1; /* [20] */
        unsigned int fifo_wr_int245 : 1; /* [21] */
        unsigned int fifo_wr_int246 : 1; /* [22] */
        unsigned int fifo_wr_int247 : 1; /* [23] */
        unsigned int fifo_wr_int248 : 1; /* [24] */
        unsigned int fifo_wr_int249 : 1; /* [25] */
        unsigned int fifo_wr_int250 : 1; /* [26] */
        unsigned int fifo_wr_int251 : 1; /* [27] */
        unsigned int fifo_wr_int252 : 1; /* [28] */
        unsigned int fifo_wr_int253 : 1; /* [29] */
        unsigned int fifo_wr_int254 : 1; /* [30] */
        unsigned int fifo_wr_int255 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_fifo_wr_int7;

typedef union {
    struct {
        unsigned int fifo_rd_int0 : 1;  /* [0] */
        unsigned int fifo_rd_int1 : 1;  /* [1] */
        unsigned int fifo_rd_int2 : 1;  /* [2] */
        unsigned int fifo_rd_int3 : 1;  /* [3] */
        unsigned int fifo_rd_int4 : 1;  /* [4] */
        unsigned int fifo_rd_int5 : 1;  /* [5] */
        unsigned int fifo_rd_int6 : 1;  /* [6] */
        unsigned int fifo_rd_int7 : 1;  /* [7] */
        unsigned int fifo_rd_int8 : 1;  /* [8] */
        unsigned int fifo_rd_int9 : 1;  /* [9] */
        unsigned int fifo_rd_int10 : 1; /* [10] */
        unsigned int fifo_rd_int11 : 1; /* [11] */
        unsigned int fifo_rd_int12 : 1; /* [12] */
        unsigned int fifo_rd_int13 : 1; /* [13] */
        unsigned int fifo_rd_int14 : 1; /* [14] */
        unsigned int fifo_rd_int15 : 1; /* [15] */
        unsigned int fifo_rd_int16 : 1; /* [16] */
        unsigned int fifo_rd_int17 : 1; /* [17] */
        unsigned int fifo_rd_int18 : 1; /* [18] */
        unsigned int fifo_rd_int19 : 1; /* [19] */
        unsigned int fifo_rd_int20 : 1; /* [20] */
        unsigned int fifo_rd_int21 : 1; /* [21] */
        unsigned int fifo_rd_int22 : 1; /* [22] */
        unsigned int fifo_rd_int23 : 1; /* [23] */
        unsigned int fifo_rd_int24 : 1; /* [24] */
        unsigned int fifo_rd_int25 : 1; /* [25] */
        unsigned int fifo_rd_int26 : 1; /* [26] */
        unsigned int fifo_rd_int27 : 1; /* [27] */
        unsigned int fifo_rd_int28 : 1; /* [28] */
        unsigned int fifo_rd_int29 : 1; /* [29] */
        unsigned int fifo_rd_int30 : 1; /* [30] */
        unsigned int fifo_rd_int31 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_fifo_rd_int0;

typedef union {
    struct {
        unsigned int fifo_rd_int32 : 1; /* [0] */
        unsigned int fifo_rd_int33 : 1; /* [1] */
        unsigned int fifo_rd_int34 : 1; /* [2] */
        unsigned int fifo_rd_int35 : 1; /* [3] */
        unsigned int fifo_rd_int36 : 1; /* [4] */
        unsigned int fifo_rd_int37 : 1; /* [5] */
        unsigned int fifo_rd_int38 : 1; /* [6] */
        unsigned int fifo_rd_int39 : 1; /* [7] */
        unsigned int fifo_rd_int40 : 1; /* [8] */
        unsigned int fifo_rd_int41 : 1; /* [9] */
        unsigned int fifo_rd_int42 : 1; /* [10] */
        unsigned int fifo_rd_int43 : 1; /* [11] */
        unsigned int fifo_rd_int44 : 1; /* [12] */
        unsigned int fifo_rd_int45 : 1; /* [13] */
        unsigned int fifo_rd_int46 : 1; /* [14] */
        unsigned int fifo_rd_int47 : 1; /* [15] */
        unsigned int fifo_rd_int48 : 1; /* [16] */
        unsigned int fifo_rd_int49 : 1; /* [17] */
        unsigned int fifo_rd_int50 : 1; /* [18] */
        unsigned int fifo_rd_int51 : 1; /* [19] */
        unsigned int fifo_rd_int52 : 1; /* [20] */
        unsigned int fifo_rd_int53 : 1; /* [21] */
        unsigned int fifo_rd_int54 : 1; /* [22] */
        unsigned int fifo_rd_int55 : 1; /* [23] */
        unsigned int fifo_rd_int56 : 1; /* [24] */
        unsigned int fifo_rd_int57 : 1; /* [25] */
        unsigned int fifo_rd_int58 : 1; /* [26] */
        unsigned int fifo_rd_int59 : 1; /* [27] */
        unsigned int fifo_rd_int60 : 1; /* [28] */
        unsigned int fifo_rd_int61 : 1; /* [29] */
        unsigned int fifo_rd_int62 : 1; /* [30] */
        unsigned int fifo_rd_int63 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_fifo_rd_int1;

typedef union {
    struct {
        unsigned int fifo_rd_int64 : 1; /* [0] */
        unsigned int fifo_rd_int65 : 1; /* [1] */
        unsigned int fifo_rd_int66 : 1; /* [2] */
        unsigned int fifo_rd_int67 : 1; /* [3] */
        unsigned int fifo_rd_int68 : 1; /* [4] */
        unsigned int fifo_rd_int69 : 1; /* [5] */
        unsigned int fifo_rd_int70 : 1; /* [6] */
        unsigned int fifo_rd_int71 : 1; /* [7] */
        unsigned int fifo_rd_int72 : 1; /* [8] */
        unsigned int fifo_rd_int73 : 1; /* [9] */
        unsigned int fifo_rd_int74 : 1; /* [10] */
        unsigned int fifo_rd_int75 : 1; /* [11] */
        unsigned int fifo_rd_int76 : 1; /* [12] */
        unsigned int fifo_rd_int77 : 1; /* [13] */
        unsigned int fifo_rd_int78 : 1; /* [14] */
        unsigned int fifo_rd_int79 : 1; /* [15] */
        unsigned int fifo_rd_int80 : 1; /* [16] */
        unsigned int fifo_rd_int81 : 1; /* [17] */
        unsigned int fifo_rd_int82 : 1; /* [18] */
        unsigned int fifo_rd_int83 : 1; /* [19] */
        unsigned int fifo_rd_int84 : 1; /* [20] */
        unsigned int fifo_rd_int85 : 1; /* [21] */
        unsigned int fifo_rd_int86 : 1; /* [22] */
        unsigned int fifo_rd_int87 : 1; /* [23] */
        unsigned int fifo_rd_int88 : 1; /* [24] */
        unsigned int fifo_rd_int89 : 1; /* [25] */
        unsigned int fifo_rd_int90 : 1; /* [26] */
        unsigned int fifo_rd_int91 : 1; /* [27] */
        unsigned int fifo_rd_int92 : 1; /* [28] */
        unsigned int fifo_rd_int93 : 1; /* [29] */
        unsigned int fifo_rd_int94 : 1; /* [30] */
        unsigned int fifo_rd_int95 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_fifo_rd_int2;

typedef union {
    struct {
        unsigned int fifo_rd_int96 : 1;  /* [0] */
        unsigned int fifo_rd_int97 : 1;  /* [1] */
        unsigned int fifo_rd_int98 : 1;  /* [2] */
        unsigned int fifo_rd_int99 : 1;  /* [3] */
        unsigned int fifo_rd_int100 : 1; /* [4] */
        unsigned int fifo_rd_int101 : 1; /* [5] */
        unsigned int fifo_rd_int102 : 1; /* [6] */
        unsigned int fifo_rd_int103 : 1; /* [7] */
        unsigned int fifo_rd_int104 : 1; /* [8] */
        unsigned int fifo_rd_int105 : 1; /* [9] */
        unsigned int fifo_rd_int106 : 1; /* [10] */
        unsigned int fifo_rd_int107 : 1; /* [11] */
        unsigned int fifo_rd_int108 : 1; /* [12] */
        unsigned int fifo_rd_int109 : 1; /* [13] */
        unsigned int fifo_rd_int110 : 1; /* [14] */
        unsigned int fifo_rd_int111 : 1; /* [15] */
        unsigned int fifo_rd_int112 : 1; /* [16] */
        unsigned int fifo_rd_int113 : 1; /* [17] */
        unsigned int fifo_rd_int114 : 1; /* [18] */
        unsigned int fifo_rd_int115 : 1; /* [19] */
        unsigned int fifo_rd_int116 : 1; /* [20] */
        unsigned int fifo_rd_int117 : 1; /* [21] */
        unsigned int fifo_rd_int118 : 1; /* [22] */
        unsigned int fifo_rd_int119 : 1; /* [23] */
        unsigned int fifo_rd_int120 : 1; /* [24] */
        unsigned int fifo_rd_int121 : 1; /* [25] */
        unsigned int fifo_rd_int122 : 1; /* [26] */
        unsigned int fifo_rd_int123 : 1; /* [27] */
        unsigned int fifo_rd_int124 : 1; /* [28] */
        unsigned int fifo_rd_int125 : 1; /* [29] */
        unsigned int fifo_rd_int126 : 1; /* [30] */
        unsigned int fifo_rd_int127 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_fifo_rd_int3;

typedef union {
    struct {
        unsigned int fifo_rd_int128 : 1; /* [0] */
        unsigned int fifo_rd_int129 : 1; /* [1] */
        unsigned int fifo_rd_int130 : 1; /* [2] */
        unsigned int fifo_rd_int131 : 1; /* [3] */
        unsigned int fifo_rd_int132 : 1; /* [4] */
        unsigned int fifo_rd_int133 : 1; /* [5] */
        unsigned int fifo_rd_int134 : 1; /* [6] */
        unsigned int fifo_rd_int135 : 1; /* [7] */
        unsigned int fifo_rd_int136 : 1; /* [8] */
        unsigned int fifo_rd_int137 : 1; /* [9] */
        unsigned int fifo_rd_int138 : 1; /* [10] */
        unsigned int fifo_rd_int139 : 1; /* [11] */
        unsigned int fifo_rd_int140 : 1; /* [12] */
        unsigned int fifo_rd_int141 : 1; /* [13] */
        unsigned int fifo_rd_int142 : 1; /* [14] */
        unsigned int fifo_rd_int143 : 1; /* [15] */
        unsigned int fifo_rd_int144 : 1; /* [16] */
        unsigned int fifo_rd_int145 : 1; /* [17] */
        unsigned int fifo_rd_int146 : 1; /* [18] */
        unsigned int fifo_rd_int147 : 1; /* [19] */
        unsigned int fifo_rd_int148 : 1; /* [20] */
        unsigned int fifo_rd_int149 : 1; /* [21] */
        unsigned int fifo_rd_int150 : 1; /* [22] */
        unsigned int fifo_rd_int151 : 1; /* [23] */
        unsigned int fifo_rd_int152 : 1; /* [24] */
        unsigned int fifo_rd_int153 : 1; /* [25] */
        unsigned int fifo_rd_int154 : 1; /* [26] */
        unsigned int fifo_rd_int155 : 1; /* [27] */
        unsigned int fifo_rd_int156 : 1; /* [28] */
        unsigned int fifo_rd_int157 : 1; /* [29] */
        unsigned int fifo_rd_int158 : 1; /* [30] */
        unsigned int fifo_rd_int159 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_fifo_rd_int4;

typedef union {
    struct {
        unsigned int fifo_rd_int160 : 1; /* [0] */
        unsigned int fifo_rd_int161 : 1; /* [1] */
        unsigned int fifo_rd_int162 : 1; /* [2] */
        unsigned int fifo_rd_int163 : 1; /* [3] */
        unsigned int fifo_rd_int164 : 1; /* [4] */
        unsigned int fifo_rd_int165 : 1; /* [5] */
        unsigned int fifo_rd_int166 : 1; /* [6] */
        unsigned int fifo_rd_int167 : 1; /* [7] */
        unsigned int fifo_rd_int168 : 1; /* [8] */
        unsigned int fifo_rd_int169 : 1; /* [9] */
        unsigned int fifo_rd_int170 : 1; /* [10] */
        unsigned int fifo_rd_int171 : 1; /* [11] */
        unsigned int fifo_rd_int172 : 1; /* [12] */
        unsigned int fifo_rd_int173 : 1; /* [13] */
        unsigned int fifo_rd_int174 : 1; /* [14] */
        unsigned int fifo_rd_int175 : 1; /* [15] */
        unsigned int fifo_rd_int176 : 1; /* [16] */
        unsigned int fifo_rd_int177 : 1; /* [17] */
        unsigned int fifo_rd_int178 : 1; /* [18] */
        unsigned int fifo_rd_int179 : 1; /* [19] */
        unsigned int fifo_rd_int180 : 1; /* [20] */
        unsigned int fifo_rd_int181 : 1; /* [21] */
        unsigned int fifo_rd_int182 : 1; /* [22] */
        unsigned int fifo_rd_int183 : 1; /* [23] */
        unsigned int fifo_rd_int184 : 1; /* [24] */
        unsigned int fifo_rd_int185 : 1; /* [25] */
        unsigned int fifo_rd_int186 : 1; /* [26] */
        unsigned int fifo_rd_int187 : 1; /* [27] */
        unsigned int fifo_rd_int188 : 1; /* [28] */
        unsigned int fifo_rd_int189 : 1; /* [29] */
        unsigned int fifo_rd_int190 : 1; /* [30] */
        unsigned int fifo_rd_int191 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_fifo_rd_int5;

typedef union {
    struct {
        unsigned int fifo_rd_int192 : 1; /* [0] */
        unsigned int fifo_rd_int193 : 1; /* [1] */
        unsigned int fifo_rd_int194 : 1; /* [2] */
        unsigned int fifo_rd_int195 : 1; /* [3] */
        unsigned int fifo_rd_int196 : 1; /* [4] */
        unsigned int fifo_rd_int197 : 1; /* [5] */
        unsigned int fifo_rd_int198 : 1; /* [6] */
        unsigned int fifo_rd_int199 : 1; /* [7] */
        unsigned int fifo_rd_int200 : 1; /* [8] */
        unsigned int fifo_rd_int201 : 1; /* [9] */
        unsigned int fifo_rd_int202 : 1; /* [10] */
        unsigned int fifo_rd_int203 : 1; /* [11] */
        unsigned int fifo_rd_int204 : 1; /* [12] */
        unsigned int fifo_rd_int205 : 1; /* [13] */
        unsigned int fifo_rd_int206 : 1; /* [14] */
        unsigned int fifo_rd_int207 : 1; /* [15] */
        unsigned int fifo_rd_int208 : 1; /* [16] */
        unsigned int fifo_rd_int209 : 1; /* [17] */
        unsigned int fifo_rd_int210 : 1; /* [18] */
        unsigned int fifo_rd_int211 : 1; /* [19] */
        unsigned int fifo_rd_int212 : 1; /* [20] */
        unsigned int fifo_rd_int213 : 1; /* [21] */
        unsigned int fifo_rd_int214 : 1; /* [22] */
        unsigned int fifo_rd_int215 : 1; /* [23] */
        unsigned int fifo_rd_int216 : 1; /* [24] */
        unsigned int fifo_rd_int217 : 1; /* [25] */
        unsigned int fifo_rd_int218 : 1; /* [26] */
        unsigned int fifo_rd_int219 : 1; /* [27] */
        unsigned int fifo_rd_int220 : 1; /* [28] */
        unsigned int fifo_rd_int221 : 1; /* [29] */
        unsigned int fifo_rd_int222 : 1; /* [30] */
        unsigned int fifo_rd_int223 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_fifo_rd_int6;

typedef union {
    struct {
        unsigned int fifo_rd_int224 : 1; /* [0] */
        unsigned int fifo_rd_int225 : 1; /* [1] */
        unsigned int fifo_rd_int226 : 1; /* [2] */
        unsigned int fifo_rd_int227 : 1; /* [3] */
        unsigned int fifo_rd_int228 : 1; /* [4] */
        unsigned int fifo_rd_int229 : 1; /* [5] */
        unsigned int fifo_rd_int230 : 1; /* [6] */
        unsigned int fifo_rd_int231 : 1; /* [7] */
        unsigned int fifo_rd_int232 : 1; /* [8] */
        unsigned int fifo_rd_int233 : 1; /* [9] */
        unsigned int fifo_rd_int234 : 1; /* [10] */
        unsigned int fifo_rd_int235 : 1; /* [11] */
        unsigned int fifo_rd_int236 : 1; /* [12] */
        unsigned int fifo_rd_int237 : 1; /* [13] */
        unsigned int fifo_rd_int238 : 1; /* [14] */
        unsigned int fifo_rd_int239 : 1; /* [15] */
        unsigned int fifo_rd_int240 : 1; /* [16] */
        unsigned int fifo_rd_int241 : 1; /* [17] */
        unsigned int fifo_rd_int242 : 1; /* [18] */
        unsigned int fifo_rd_int243 : 1; /* [19] */
        unsigned int fifo_rd_int244 : 1; /* [20] */
        unsigned int fifo_rd_int245 : 1; /* [21] */
        unsigned int fifo_rd_int246 : 1; /* [22] */
        unsigned int fifo_rd_int247 : 1; /* [23] */
        unsigned int fifo_rd_int248 : 1; /* [24] */
        unsigned int fifo_rd_int249 : 1; /* [25] */
        unsigned int fifo_rd_int250 : 1; /* [26] */
        unsigned int fifo_rd_int251 : 1; /* [27] */
        unsigned int fifo_rd_int252 : 1; /* [28] */
        unsigned int fifo_rd_int253 : 1; /* [29] */
        unsigned int fifo_rd_int254 : 1; /* [30] */
        unsigned int fifo_rd_int255 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_fifo_rd_int7;

typedef union {
    struct {
        unsigned int bmp_rsc_err_info : 16; /* [15:0] */
        unsigned int rsv_5 : 16;            /* [31:16] */
    } bits;

    unsigned int value;
} u_ssu_bmp_rsc_err_info;

typedef union {
    struct {
        unsigned int bmp_age_err_addr : 16; /* [15:0] */
        unsigned int bmp_age_err_id : 4;    /* [19:16] */
        unsigned int rsv_6 : 12;            /* [31:20] */
    } bits;

    unsigned int value;
} u_ssu_bmp_age_err_info;

typedef union {
    struct {
        unsigned int rpu_credit_err_info : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_credit_err_info;

typedef union {
    struct {
        unsigned int rocee_credit_err_info : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_rocee_credit_err_info;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_type : 2;      /* [1:0] */
        unsigned int mem_ecc_mbit_int_type : 2;      /* [3:2] */
        unsigned int fifo_rd_int_type : 2;           /* [5:4] */
        unsigned int fifo_wr_int_type : 2;           /* [7:6] */
        unsigned int host_pkt_sof_mismatch_type : 2; /* [9:8] */
        unsigned int host_pkt_eof_mismatch_type : 2; /* [11:10] */
        unsigned int host_pkt_key_mismatch_type : 2; /* [13:12] */
        unsigned int mac_pkt_sof_mismatch_type : 2;  /* [15:14] */
        unsigned int mac_pkt_eof_mismatch_type : 2;  /* [17:16] */
        unsigned int mac_pkt_key_mismatch_type : 2;  /* [19:18] */
        unsigned int rpu_credit_err_int_type : 2;    /* [21:20] */
        unsigned int bmp_rsc_return_int_type : 2;    /* [23:22] */
        unsigned int bmp_age_err_int_type : 2;       /* [25:24] */
        unsigned int rocee_credit_err_int_type : 2;  /* [27:26] */
        unsigned int qm_free_rsc_int_type : 2;       /* [29:28] */
        unsigned int rsv_7 : 2;                      /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_int_type;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_msix : 1;      /* [0] */
        unsigned int mem_ecc_mbit_int_msix : 1;      /* [1] */
        unsigned int fifo_rd_int_msix : 1;           /* [2] */
        unsigned int fifo_wr_int_msix : 1;           /* [3] */
        unsigned int host_pkt_sof_mismatch_msix : 1; /* [4] */
        unsigned int host_pkt_eof_mismatch_msix : 1; /* [5] */
        unsigned int host_pkt_key_mismatch_msix : 1; /* [6] */
        unsigned int mac_pkt_sof_mismatch_msix : 1;  /* [7] */
        unsigned int mac_pkt_eof_mismatch_msix : 1;  /* [8] */
        unsigned int mac_pkt_key_mismatch_msix : 1;  /* [9] */
        unsigned int rpu_credit_err_int_msix : 1;    /* [10] */
        unsigned int bmp_rsc_return_int_msix : 1;    /* [11] */
        unsigned int bmp_age_err_int_msix : 1;       /* [12] */
        unsigned int rsv_8 : 19;                     /* [31:13] */
    } bits;

    unsigned int value;
} u_ssu_int_msix;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_ce : 1;      /* [0] */
        unsigned int mem_ecc_mbit_int_ce : 1;      /* [1] */
        unsigned int fifo_rd_int_ce : 1;           /* [2] */
        unsigned int fifo_wr_int_ce : 1;           /* [3] */
        unsigned int host_pkt_sof_mismatch_ce : 1; /* [4] */
        unsigned int host_pkt_eof_mismatch_ce : 1; /* [5] */
        unsigned int host_pkt_key_mismatch_ce : 1; /* [6] */
        unsigned int mac_pkt_sof_mismatch_ce : 1;  /* [7] */
        unsigned int mac_pkt_eof_mismatch_ce : 1;  /* [8] */
        unsigned int mac_pkt_key_mismatch_ce : 1;  /* [9] */
        unsigned int rpu_credit_err_int_ce : 1;    /* [10] */
        unsigned int bmp_rsc_return_int_ce : 1;    /* [11] */
        unsigned int bmp_age_err_int_ce : 1;       /* [12] */
        unsigned int rocee_credit_err_int_ce : 1;  /* [13] */
        unsigned int qm_free_rsc_int_ce : 1;       /* [14] */
        unsigned int rsv_9 : 17;                   /* [31:15] */
    } bits;

    unsigned int value;
} u_ssu_int_ce;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_nfe : 1;      /* [0] */
        unsigned int mem_ecc_mbit_int_nfe : 1;      /* [1] */
        unsigned int fifo_rd_int_nfe : 1;           /* [2] */
        unsigned int fifo_wr_int_nfe : 1;           /* [3] */
        unsigned int host_pkt_sof_mismatch_nfe : 1; /* [4] */
        unsigned int host_pkt_eof_mismatch_nfe : 1; /* [5] */
        unsigned int host_pkt_key_mismatch_nfe : 1; /* [6] */
        unsigned int mac_pkt_sof_mismatch_nfe : 1;  /* [7] */
        unsigned int mac_pkt_eof_mismatch_nfe : 1;  /* [8] */
        unsigned int mac_pkt_key_mismatch_nfe : 1;  /* [9] */
        unsigned int rpu_credit_err_int_nfe : 1;    /* [10] */
        unsigned int bmp_rsc_return_int_nfe : 1;    /* [11] */
        unsigned int bmp_age_err_int_nfe : 1;       /* [12] */
        unsigned int rocee_credit_err_int_nfe : 1;  /* [13] */
        unsigned int qm_free_rsc_int_nfe : 1;       /* [14] */
        unsigned int rsv_10 : 17;                   /* [31:15] */
    } bits;

    unsigned int value;
} u_ssu_int_nfe;

typedef union {
    struct {
        unsigned int mem_ecc_sbit_int_fe : 1;      /* [0] */
        unsigned int mem_ecc_mbit_int_fe : 1;      /* [1] */
        unsigned int fifo_rd_int_fe : 1;           /* [2] */
        unsigned int fifo_wr_int_fe : 1;           /* [3] */
        unsigned int host_pkt_sof_mismatch_fe : 1; /* [4] */
        unsigned int host_pkt_eof_mismatch_fe : 1; /* [5] */
        unsigned int host_pkt_key_mismatch_fe : 1; /* [6] */
        unsigned int mac_pkt_sof_mismatch_fe : 1;  /* [7] */
        unsigned int mac_pkt_eof_mismatch_fe : 1;  /* [8] */
        unsigned int mac_pkt_key_mismatch_fe : 1;  /* [9] */
        unsigned int rpu_credit_err_int_fe : 1;    /* [10] */
        unsigned int bmp_rsc_return_int_fe : 1;    /* [11] */
        unsigned int bmp_age_err_int_fe : 1;       /* [12] */
        unsigned int rocee_credit_err_int_fe : 1;  /* [13] */
        unsigned int qm_free_rsc_int_fe : 1;       /* [14] */
        unsigned int rsv_11 : 17;                  /* [31:15] */
    } bits;

    unsigned int value;
} u_ssu_int_fe;

typedef union {
    struct {
        unsigned int mem_init_start : 1; /* [0] */
        unsigned int rsv_12 : 31;        /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_mem_init_start;

typedef union {
    struct {
        unsigned int mem_init_done : 1;   /* [0] */
        unsigned int rsv_13 : 3;          /* [3:1] */
        unsigned int mem_init_done0 : 1;  /* [4] */
        unsigned int mem_init_done1 : 1;  /* [5] */
        unsigned int mem_init_done2 : 1;  /* [6] */
        unsigned int mem_init_done3 : 1;  /* [7] */
        unsigned int mem_init_done4 : 1;  /* [8] */
        unsigned int mem_init_done5 : 1;  /* [9] */
        unsigned int mem_init_done6 : 1;  /* [10] */
        unsigned int mem_init_done7 : 1;  /* [11] */
        unsigned int mem_init_done8 : 1;  /* [12] */
        unsigned int mem_init_done9 : 1;  /* [13] */
        unsigned int mem_init_done10 : 1; /* [14] */
        unsigned int mem_init_done11 : 1; /* [15] */
        unsigned int mem_init_done12 : 1; /* [16] */
        unsigned int mem_init_done13 : 1; /* [17] */
        unsigned int mem_init_done14 : 1; /* [18] */
        unsigned int mem_init_done15 : 1; /* [19] */
        unsigned int mem_init_done16 : 1; /* [20] */
        unsigned int mem_init_done17 : 1; /* [21] */
        unsigned int mem_init_done18 : 1; /* [22] */
        unsigned int mem_init_done19 : 1; /* [23] */
        unsigned int mem_init_done20 : 1; /* [24] */
        unsigned int mem_init_done21 : 1; /* [25] */
        unsigned int mem_init_done22 : 1; /* [26] */
        unsigned int mem_init_done23 : 1; /* [27] */
        unsigned int mem_init_done24 : 1; /* [28] */
        unsigned int mem_init_done25 : 1; /* [29] */
        unsigned int mem_init_done26 : 1; /* [30] */
        unsigned int mem_init_done27 : 1; /* [31] */
    } bits;

    unsigned int value;
} u_ssu_mem_init_done;

typedef union {
    struct {
        unsigned int sp_ram_tmod : 16; /* [15:0] */
        unsigned int rsv_14 : 16;      /* [31:16] */
    } bits;

    unsigned int value;
} u_ssu_spmem_tmod_ctrl;

typedef union {
    struct {
        unsigned int tp_ram_tmod : 16; /* [15:0] */
        unsigned int rsv_15 : 16;      /* [31:16] */
    } bits;

    unsigned int value;
} u_ssu_tpmem_tmod_ctrl;

typedef union {
    struct {
        unsigned int mem_power_mode : 8; /* [7:0] */
        unsigned int rsv_16 : 24;        /* [31:8] */
    } bits;

    unsigned int value;
} u_ssu_mem_power_ctrl;

typedef union {
    struct {
        unsigned int mem_ecc_bypass : 1; /* [0] */
        unsigned int rsv_17 : 31;        /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_ecc_bypass;

typedef union {
    struct {
        unsigned int bmu_pool_pause_req : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_bmu_pool_pause_req;

typedef union {
    struct {
        unsigned int cnt_clr_ce : 1; /* [0] */
        unsigned int rsv_18 : 31;    /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_cnt_clr_ce;

typedef union {
    struct {
        unsigned int mem_sbit_ecc_thd : 8; /* [7:0] */
        unsigned int rsv_19 : 24;          /* [31:8] */
    } bits;

    unsigned int value;
} u_ssu_ecc_sbit_thd;

typedef union {
    struct {
        unsigned int cfg_lo_force_hi_pri : 1; /* [0] */
        unsigned int rsv_20 : 31;             /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_lo_force_hi_pri;

typedef union {
    struct {
        unsigned int cfg_rocee_type : 4;   /* [3:0] */
        unsigned int rsv_21 : 4;           /* [7:4] */
        unsigned int cfg_rocee_cks_up : 1; /* [8] */
        unsigned int rsv_22 : 3;           /* [11:9] */
        unsigned int cfg_rocee_enable : 1; /* [12] */
        unsigned int rsv_23 : 19;          /* [31:13] */
    } bits;

    unsigned int value;
} u_ssu_rocee_type_cfg;

typedef union {
    struct {
        unsigned int rpu_port_init_en : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_rpu_port_init_enable;

typedef union {
    struct {
        unsigned int rocee_port_init_en : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_rocee_port_init_enable;

typedef union {
    struct {
        unsigned int rpu_port_init_credit : 4;   /* [3:0] */
        unsigned int rsv_24 : 4;                 /* [7:4] */
        unsigned int rocee_port_init_credit : 5; /* [12:8] */
        unsigned int rsv_25 : 19;                /* [31:13] */
    } bits;

    unsigned int value;
} u_ssu_rpu_init_credit;

typedef union {
    struct {
        unsigned int oq_rx_port_sch_en : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_rx_oq_schedule_en;

typedef union {
    struct {
        unsigned int oq_tx_port_sch_en : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_tx_oq_schedule_en;

typedef union {
    struct {
        unsigned int oq_drop_port_sch_en : 1; /* [0] */
        unsigned int rsv_26 : 31;             /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_drop_oq_schedule_en;

typedef union {
    struct {
        unsigned int cfg_rx_l2err_drop : 1; /* [0] */
        unsigned int cfg_tx_l2err_drop : 1; /* [1] */
        unsigned int rsv_27 : 30;           /* [31:2] */
    } bits;

    unsigned int value;
} u_ssu_l2err_drop_en;

typedef union {
    struct {
        unsigned int in_port_sel : 7; /* [6:0] */
        unsigned int rsv_28 : 25;     /* [31:7] */
    } bits;

    unsigned int value;
} u_ssu_in_port_sel;

typedef union {
    struct {
        unsigned int ot_port_list_status_sel : 7; /* [6:0] */
        unsigned int rsv_29 : 25;                 /* [31:7] */
    } bits;

    unsigned int value;
} u_ssu_ot_port_status_sel;

typedef union {
    struct {
        unsigned int oq_list_status_sel : 10; /* [9:0] */
        unsigned int rsv_30 : 22;             /* [31:10] */
    } bits;

    unsigned int value;
} u_ssu_oq_status_sel;

typedef union {
    struct {
        unsigned int rx_oq_tc_map : 24; /* [23:0] */
        unsigned int rsv_31 : 8;        /* [31:24] */
    } bits;

    unsigned int value;
} u_ssu_rx_oq_tc_map;

typedef union {
    struct {
        unsigned int tx_oq_tc_map : 24; /* [23:0] */
        unsigned int rsv_32 : 8;        /* [31:24] */
    } bits;

    unsigned int value;
} u_ssu_tx_oq_tc_map;

typedef union {
    struct {
        unsigned int software_gen_rx_pause : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_software_gen_rx_pause;

typedef union {
    struct {
        unsigned int software_gen_tx_pause : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_software_gen_tx_pause;

typedef union {
    struct {
        unsigned int pkt_max_drop_en : 1; /* [0] */
        unsigned int rsv_33 : 31;         /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_pkt_max_drop_enable;

typedef union {
    struct {
        unsigned int pkt_max_cell_num : 8; /* [7:0] */
        unsigned int rsv_34 : 24;          /* [31:8] */
    } bits;

    unsigned int value;
} u_ssu_pkt_max_cell_num;

typedef union {
    struct {
        unsigned int delta_cell_cnt : 13; /* [12:0] */
        unsigned int rsv_35 : 3;          /* [15:13] */
        unsigned int left_cell_thd : 13;  /* [28:16] */
        unsigned int rsv_36 : 3;          /* [31:29] */
    } bits;

    unsigned int value;
} u_ssu_bank_balance;

typedef union {
    struct {
        unsigned int ot_drop_rev_pd_num : 14; /* [13:0] */
        unsigned int rsv_37 : 18;             /* [31:14] */
    } bits;

    unsigned int value;
} u_ssu_drop_rev_pd_thr;

typedef union {
    struct {
        unsigned int cfg_rd_mb_ost_num : 5; /* [4:0] */
        unsigned int rsv_38 : 27;           /* [31:5] */
    } bits;

    unsigned int value;
} u_ssu_ppp_mb_rd_ost_num;

typedef union {
    struct {
        unsigned int cfg_epla_inf_ind : 1; /* [0] */
        unsigned int rsv_39 : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_epla_inf_ind;

typedef union {
    struct {
        unsigned int pkt_padding_ind : 1; /* [0] */
        unsigned int rsv_40 : 31;         /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_pkt_padding_ind;

typedef union {
    struct {
        unsigned int tm_info_bypass : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_tx_tm_info_bypass;

typedef union {
    struct {
        unsigned int txsch_info_bypass : 1; /* [0] */
        unsigned int rsv_41 : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_tx_sch_info_bypass;

typedef union {
    struct {
        unsigned int dup_up_lenth_offset_host : 7; /* [6:0] */
        unsigned int rsv_42 : 9;                   /* [15:7] */
        unsigned int dup_up_lenth_offset_mac : 7;  /* [22:16] */
        unsigned int rsv_43 : 9;                   /* [31:23] */
    } bits;

    unsigned int value;
} u_ssu_dup_up_lenth_offset;

typedef union {
    struct {
        unsigned int ets_up_lenth_offset_host : 7; /* [6:0] */
        unsigned int rsv_44 : 9;                   /* [15:7] */
        unsigned int ets_up_lenth_offset_mac : 7;  /* [22:16] */
        unsigned int rsv_45 : 9;                   /* [31:23] */
    } bits;

    unsigned int value;
} u_ssu_ets_up_lenth_offset;

typedef union {
    struct {
        unsigned int mult_qcn_up_lenth_offset_host : 7; /* [6:0] */
        unsigned int rsv_46 : 9;                        /* [15:7] */
        unsigned int mult_qcn_up_lenth_offset_mac : 7;  /* [22:16] */
        unsigned int rsv_47 : 9;                        /* [31:23] */
    } bits;

    unsigned int value;
} u_ssu_mult_qcn_up_lenth_offset;

typedef union {
    struct {
        unsigned int sgl_qcn_up_lenth_offset_host : 7; /* [6:0] */
        unsigned int rsv_48 : 9;                       /* [15:7] */
        unsigned int sgl_qcn_up_lenth_offset_mac : 7;  /* [22:16] */
        unsigned int rsv_49 : 9;                       /* [31:23] */
    } bits;

    unsigned int value;
} u_ssu_sgl_qcn_up_lenth_offset;

typedef union {
    struct {
        unsigned int rsv_50 : 16;             /* [15:0] */
        unsigned int cfg_gating_sts_port : 7; /* [22:16] */
        unsigned int rsv_51 : 9;              /* [31:23] */
    } bits;

    unsigned int value;
} u_ssu_clk_gating_ctrl;

typedef union {
    struct {
        unsigned int cfg_gating_win : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_gating_window;

typedef union {
    struct {
        unsigned int gating_sts : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_gating_sts;

typedef union {
    struct {
        unsigned int cfg_auto_gating_enable0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_clk_gating_en0;

typedef union {
    struct {
        unsigned int cfg_auto_gating_enable1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_clk_gating_en1;

typedef union {
    struct {
        unsigned int cfg_auto_gating_enable2 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_clk_gating_en2;

typedef union {
    struct {
        unsigned int cfg_auto_gating_enable3 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_clk_gating_en3;

typedef union {
    struct {
        unsigned int auto_age_en : 1;              /* [0] */
        unsigned int rsv_52 : 3;                   /* [3:1] */
        unsigned int age_onetime : 1;              /* [4] */
        unsigned int rsv_53 : 3;                   /* [7:5] */
        unsigned int age_auto_rls_en : 1;          /* [8] */
        unsigned int rsv_54 : 3;                   /* [11:9] */
        unsigned int age_find_nxt_with_rls : 3;    /* [14:12] */
        unsigned int rsv_55 : 1;                   /* [15] */
        unsigned int age_find_nxt_without_rls : 3; /* [18:16] */
        unsigned int rsv_56 : 13;                  /* [31:19] */
    } bits;

    unsigned int value;
} u_ssu_bmp_ageing_ctrl;

typedef union {
    struct {
        unsigned int ageing_sample_time : 28; /* [27:0] */
        unsigned int rsv_57 : 4;              /* [31:28] */
    } bits;

    unsigned int value;
} u_ssu_bmp_ageing_sample_thd;

typedef union {
    struct {
        unsigned int ageing_int_thd : 8; /* [7:0] */
        unsigned int rsv_58 : 24;        /* [31:8] */
    } bits;

    unsigned int value;
} u_ssu_bmp_ageing_int_thd;

typedef union {
    struct {
        unsigned int age_rls_int_thd : 15; /* [14:0] */
        unsigned int rsv_59 : 17;          /* [31:15] */
    } bits;

    unsigned int value;
} u_ssu_bmp_ageing_rls_thd;

typedef union {
    struct {
        unsigned int ageing_interval_l : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_bmp_ageing_timeout_l;

typedef union {
    struct {
        unsigned int ageing_interval_h : 16; /* [15:0] */
        unsigned int rsv_60 : 16;            /* [31:16] */
    } bits;

    unsigned int value;
} u_ssu_bmp_ageing_timeout_h;

typedef union {
    struct {
        unsigned int age_timeout_addr_b0 : 12; /* [11:0] */
        unsigned int rsv_61 : 1;               /* [12] */
        unsigned int cur_ageing_st_b0 : 3;     /* [15:13] */
        unsigned int age_found_waiting_b0 : 1; /* [16] */
        unsigned int rsv_62 : 15;              /* [31:17] */
    } bits;

    unsigned int value;
} u_ssu_bmp_ageing_st_bank0;

typedef union {
    struct {
        unsigned int age_timeout_addr_b1 : 12; /* [11:0] */
        unsigned int rsv_63 : 1;               /* [12] */
        unsigned int cur_ageing_st_b1 : 3;     /* [15:13] */
        unsigned int age_found_waiting_b1 : 1; /* [16] */
        unsigned int rsv_64 : 15;              /* [31:17] */
    } bits;

    unsigned int value;
} u_ssu_bmp_ageing_st_bank1;

typedef union {
    struct {
        unsigned int age_timeout_addr_b2 : 12; /* [11:0] */
        unsigned int rsv_65 : 1;               /* [12] */
        unsigned int cur_ageing_st_b2 : 3;     /* [15:13] */
        unsigned int age_found_waiting_b2 : 1; /* [16] */
        unsigned int rsv_66 : 15;              /* [31:17] */
    } bits;

    unsigned int value;
} u_ssu_bmp_ageing_st_bank2;

typedef union {
    struct {
        unsigned int age_timeout_addr_b3 : 12; /* [11:0] */
        unsigned int rsv_67 : 1;               /* [12] */
        unsigned int cur_ageing_st_b3 : 3;     /* [15:13] */
        unsigned int age_found_waiting_b3 : 1; /* [16] */
        unsigned int rsv_68 : 15;              /* [31:17] */
    } bits;

    unsigned int value;
} u_ssu_bmp_ageing_st_bank3;

typedef union {
    struct {
        unsigned int ssu_ageing_rls_cnt_b0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_bmp_ageing_rls_cnt_bank0;

typedef union {
    struct {
        unsigned int ssu_ageing_rls_cnt_b1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_bmp_ageing_rls_cnt_bank1;

typedef union {
    struct {
        unsigned int ssu_ageing_rls_cnt_b2 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_bmp_ageing_rls_cnt_bank2;

typedef union {
    struct {
        unsigned int ssu_ageing_rls_cnt_b3 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_bmp_ageing_rls_cnt_bank3;

typedef union {
    struct {
        unsigned int run_mode : 1; /* [0] */
        unsigned int rsv_69 : 31;  /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_run_mode;

typedef union {
    struct {
        unsigned int cge_run_mode : 1; /* [0] */
        unsigned int rsv_70 : 31;      /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_cge_run_mode;

typedef union {
    struct {
        unsigned int cge_port_exd_mode : 1; /* [0] */
        unsigned int rsv_71 : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_cge_port_exd_mode;

typedef union {
    struct {
        unsigned int cge_rx_port_init : 1; /* [0] */
        unsigned int rsv_72 : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_cge_rx_port_init;

typedef union {
    struct {
        unsigned int grp0_port_bitmap : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_eg_group0_port_bitmap;

typedef union {
    struct {
        unsigned int grp1_port_bitmap : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_eg_group1_port_bitmap;

typedef union {
    struct {
        unsigned int grp2_port_bitmap : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_eg_group2_port_bitmap;

typedef union {
    struct {
        unsigned int perf_test_en : 4; /* [3:0] */
        unsigned int rsv_73 : 28;      /* [31:4] */
    } bits;

    unsigned int value;
} u_ssu_perf_test_sel;

typedef union {
    struct {
        unsigned int ig_host_perf_test_port : 6; /* [5:0] */
        unsigned int rsv_74 : 2;                 /* [7:6] */
        unsigned int ig_mac_perf_test_port : 6;  /* [13:8] */
        unsigned int rsv_75 : 2;                 /* [15:14] */
        unsigned int eg_host_perf_test_port : 6; /* [21:16] */
        unsigned int rsv_76 : 2;                 /* [23:22] */
        unsigned int eg_mac_perf_test_port : 6;  /* [29:24] */
        unsigned int rsv_77 : 2;                 /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_perf_test_port;

typedef union {
    struct {
        unsigned int ig_host_perf_test_status : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ig_host_perf_test_status;

typedef union {
    struct {
        unsigned int ig_mac_perf_test_status : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ig_mac_perf_test_status;

typedef union {
    struct {
        unsigned int eg_host_perf_test_status : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_eg_host_perf_test_status;

typedef union {
    struct {
        unsigned int eg_mac_perf_test_status : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_eg_mac_perf_test_status;

typedef union {
    struct {
        unsigned int ig_host_perf_test_pps_status : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ig_host_perf_test_pps_status;

typedef union {
    struct {
        unsigned int ig_mac_perf_test_pps_status : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ig_mac_perf_test_pps_status;

typedef union {
    struct {
        unsigned int eg_host_perf_test_pps_status : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_eg_host_perf_test_pps_status;

typedef union {
    struct {
        unsigned int eg_mac_perf_test_pps_status : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_eg_mac_perf_test_pps_status;

typedef union {
    struct {
        unsigned int cfg_port_round_num : 7; /* [6:0] */
        unsigned int rsv_78 : 25;            /* [31:7] */
    } bits;

    unsigned int value;
} u_ssu_eg_mac_sch_round_num;

typedef union {
    struct {
        unsigned int oq_dup_bp_thd : 15; /* [14:0] */
        unsigned int rsv_79 : 17;        /* [31:15] */
    } bits;

    unsigned int value;
} u_ssu_oq_dup_bp_thd;

typedef union {
    struct {
        unsigned int ot_port_drop_thd : 15; /* [14:0] */
        unsigned int rsv_80 : 17;           /* [31:15] */
    } bits;

    unsigned int value;
} u_ssu_oq_drop_thd;

typedef union {
    struct {
        unsigned int dup_copy_pause_lo_thd : 15; /* [14:0] */
        unsigned int rsv_81 : 1;                 /* [15] */
        unsigned int dup_copy_pause_hi_thd : 15; /* [30:16] */
        unsigned int rsv_82 : 1;                 /* [31] */
    } bits;

    unsigned int value;
} u_ssu_oq_dup_copy_thd;

typedef union {
    struct {
        unsigned int lo_pri_sgl_buf_size : 11; /* [10:0] */
        unsigned int rsv_83 : 21;              /* [31:11] */
    } bits;

    unsigned int value;
} u_ssu_lo_pri_sgl_buf_size;

typedef union {
    struct {
        unsigned int lo_pri_mult_buf_size : 11; /* [10:0] */
        unsigned int rsv_84 : 21;               /* [31:11] */
    } bits;

    unsigned int value;
} u_ssu_lo_pri_mult_buf_size;

typedef union {
    struct {
        unsigned int hi_pri_mult_buf_size : 11; /* [10:0] */
        unsigned int rsv_85 : 21;               /* [31:11] */
    } bits;

    unsigned int value;
} u_ssu_hi_pri_mult_buf_size;

typedef union {
    struct {
        unsigned int lo_pri_sgl_drop_hi_thd : 11; /* [10:0] */
        unsigned int rsv_86 : 5;                  /* [15:11] */
        unsigned int lo_pri_sgl_drop_lo_thd : 11; /* [26:16] */
        unsigned int rsv_87 : 5;                  /* [31:27] */
    } bits;

    unsigned int value;
} u_ssu_lo_pri_sgl_drop_thd;

typedef union {
    struct {
        unsigned int lo_pri_mult_drop_hi_thd : 11; /* [10:0] */
        unsigned int rsv_88 : 5;                   /* [15:11] */
        unsigned int lo_pri_mult_drop_lo_thd : 11; /* [26:16] */
        unsigned int rsv_89 : 5;                   /* [31:27] */
    } bits;

    unsigned int value;
} u_ssu_lo_pri_mult_drop_thd;

typedef union {
    struct {
        unsigned int hi_pri_mult_drop_hi_thd : 11; /* [10:0] */
        unsigned int rsv_90 : 5;                   /* [15:11] */
        unsigned int hi_pri_mult_drop_lo_thd : 11; /* [26:16] */
        unsigned int rsv_91 : 5;                   /* [31:27] */
    } bits;

    unsigned int value;
} u_ssu_hi_pri_mult_drop_thd;

typedef union {
    struct {
        unsigned int hi_pri_mult_weight : 8; /* [7:0] */
        unsigned int lo_pri_mult_weight : 8; /* [15:8] */
        unsigned int lo_pri_sgl_weight : 8;  /* [23:16] */
        unsigned int wrr_weight_offset : 4;  /* [27:24] */
        unsigned int rsv_92 : 4;             /* [31:28] */
    } bits;

    unsigned int value;
} u_ssu_dup_wrr_weight;

typedef union {
    struct {
        unsigned int dup_shap_bypass : 1; /* [0] */
        unsigned int rsv_93 : 31;         /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_dup_shaping_bypass;

typedef union {
    struct {
        unsigned int hi_pri_mul_shap_cfg : 26; /* [25:0] */
        unsigned int rsv_94 : 6;               /* [31:26] */
    } bits;

    unsigned int value;
} u_ssu_hi_pri_mul_shaping;

typedef union {
    struct {
        unsigned int lo_pri_mul_shap_cfg : 26; /* [25:0] */
        unsigned int rsv_95 : 6;               /* [31:26] */
    } bits;

    unsigned int value;
} u_ssu_lo_pri_mul_shaping;

typedef union {
    struct {
        unsigned int lo_pri_uni_shap_cfg : 26; /* [25:0] */
        unsigned int rsv_96 : 6;               /* [31:26] */
    } bits;

    unsigned int value;
} u_ssu_lo_pri_uni_shaping;

typedef union {
    struct {
        unsigned int cfg_car_bps_ir : 20; /* [19:0] */
        unsigned int rsv_97 : 12;         /* [31:20] */
    } bits;

    unsigned int value;
} u_ssu_car_cfg0;

typedef union {
    struct {
        unsigned int cfg_car_bps_bs : 20; /* [19:0] */
        unsigned int rsv_98 : 12;         /* [31:20] */
    } bits;

    unsigned int value;
} u_ssu_car_cfg1;

typedef union {
    struct {
        unsigned int cfg_car_en : 1;         /* [0] */
        unsigned int rsv_99 : 15;            /* [15:1] */
        unsigned int cfg_car_bps_cycle : 14; /* [29:16] */
        unsigned int rsv_100 : 2;            /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_car_cfg2;

typedef union {
    struct {
        unsigned int cfg_car_init : 1; /* [0] */
        unsigned int rsv_101 : 31;     /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_car_cfg3;

typedef union {
    struct {
        unsigned int ig_rx_pkt_bp_gap : 5; /* [4:0] */
        unsigned int rsv_102 : 27;         /* [31:5] */
    } bits;

    unsigned int value;
} u_ssu_ig_rx_pkt_bp_gap;

typedef union {
    struct {
        unsigned int ig_tx_pkt_bp_gap : 5; /* [4:0] */
        unsigned int rsv_103 : 27;         /* [31:5] */
    } bits;

    unsigned int value;
} u_ssu_ig_tx_pkt_bp_gap;

typedef union {
    struct {
        unsigned int tm_inf_fifo_gap : 8; /* [7:0] */
        unsigned int rsv_104 : 24;        /* [31:8] */
    } bits;

    unsigned int value;
} u_ssu_tm_inf_fifo_gap;

typedef union {
    struct {
        unsigned int ppp_rlt_fifo_gap : 2; /* [1:0] */
        unsigned int rsv_105 : 30;         /* [31:2] */
    } bits;

    unsigned int value;
} u_ssu_ppp_rlt_fifo_gap;

typedef union {
    struct {
        unsigned int pkt_id_fifo_gap : 5; /* [4:0] */
        unsigned int rsv_106 : 27;        /* [31:5] */
    } bits;

    unsigned int value;
} u_ssu_cmd_pkt_id_fifo_gap;

typedef union {
    struct {
        unsigned int store_info_fifo_gap : 2; /* [1:0] */
        unsigned int rsv_107 : 30;            /* [31:2] */
    } bits;

    unsigned int value;
} u_ssu_dup_store_info_fifo_gap;

typedef union {
    struct {
        unsigned int dup_mult_fifo_gap : 2; /* [1:0] */
        unsigned int rsv_108 : 30;          /* [31:2] */
    } bits;

    unsigned int value;
} u_ssu_dup_mult_fifo_gap;

typedef union {
    struct {
        unsigned int dup_sgl_fifo_gap : 2; /* [1:0] */
        unsigned int rsv_109 : 30;         /* [31:2] */
    } bits;

    unsigned int value;
} u_ssu_dup_sgl_fifo_gap;

typedef union {
    struct {
        unsigned int end_drop_fifo_gap : 7; /* [6:0] */
        unsigned int rsv_110 : 25;          /* [31:7] */
    } bits;

    unsigned int value;
} u_ssu_end_drop_fifo_gap;

typedef union {
    struct {
        unsigned int mb_rlt_fifo_gap : 4; /* [3:0] */
        unsigned int rsv_111 : 28;        /* [31:4] */
    } bits;

    unsigned int value;
} u_ssu_mb_rlt_fifo_gap;

typedef union {
    struct {
        unsigned int mb_info_fifo_gap : 4; /* [3:0] */
        unsigned int rsv_112 : 28;         /* [31:4] */
    } bits;

    unsigned int value;
} u_ssu_mb_info_fifo_gap;

typedef union {
    struct {
        unsigned int dup_drop_fifo_gap : 5; /* [4:0] */
        unsigned int rsv_113 : 27;          /* [31:5] */
    } bits;

    unsigned int value;
} u_ssu_dup_drop_fifo_gap;

typedef union {
    struct {
        unsigned int ig_pkt_port_bp_thd : 7; /* [6:0] */
        unsigned int rsv_114 : 25;           /* [31:7] */
    } bits;

    unsigned int value;
} u_ssu_ig_pkt_port_bp_thd;

typedef union {
    struct {
        unsigned int rx_cmd_aful_thd : 6; /* [5:0] */
        unsigned int rsv_115 : 26;        /* [31:6] */
    } bits;

    unsigned int value;
} u_ssu_rx_cmd_fifo_aful_thd;

typedef union {
    struct {
        unsigned int tx_cmd_aful_thd : 6; /* [5:0] */
        unsigned int rsv_116 : 26;        /* [31:6] */
    } bits;

    unsigned int value;
} u_ssu_tx_cmd_fifo_aful_thd;

typedef union {
    struct {
        unsigned int drop_cmd_aful_thd : 5; /* [4:0] */
        unsigned int rsv_117 : 27;          /* [31:5] */
    } bits;

    unsigned int value;
} u_ssu_drop_cmd_fifo_aful_thd;

typedef union {
    struct {
        unsigned int rsv_cfg0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_eco_cfg0;

typedef union {
    struct {
        unsigned int rsv_cfg1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_eco_cfg1;

typedef union {
    struct {
        unsigned int rsv_cfg2 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_eco_cfg2;

typedef union {
    struct {
        unsigned int rsv_cfg3 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_eco_cfg3;

typedef union {
    struct {
        unsigned int rsv_status0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_eco_status0;

typedef union {
    struct {
        unsigned int rsv_status1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_eco_status1;

typedef union {
    struct {
        unsigned int rsv_status2 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_eco_status2;

typedef union {
    struct {
        unsigned int rsv_status3 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_eco_status3;

typedef union {
    struct {
        unsigned int ig_rx_pkt_cnt : 48; /* [47:0] */
        unsigned int rsv_118 : 16;       /* [63:48] */
    } bits;

    unsigned int value;
} u_ssu_ig_rx_pkt_cnt;

typedef union {
    struct {
        unsigned int ig_tx_pkt_cnt : 48; /* [47:0] */
        unsigned int rsv_119 : 16;       /* [63:48] */
    } bits;

    unsigned int value;
} u_ssu_ig_tx_pkt_cnt;

typedef union {
    struct {
        unsigned int eg_rx_pkt_cnt : 48; /* [47:0] */
        unsigned int rsv_120 : 16;       /* [63:48] */
    } bits;

    unsigned int value;
} u_ssu_eg_rx_pkt_cnt;

typedef union {
    struct {
        unsigned int eg_tx_pkt_cnt : 48; /* [47:0] */
        unsigned int rsv_121 : 16;       /* [63:48] */
    } bits;

    unsigned int value;
} u_ssu_eg_tx_pkt_cnt;

typedef union {
    struct {
        unsigned int ig_rx_short_pkt_cnt : 16; /* [15:0] */
        unsigned int rsv_122 : 16;             /* [31:16] */
    } bits;

    unsigned int value;
} u_ssu_ig_rx_short_pkt_cnt;

typedef union {
    struct {
        unsigned int ig_tx_short_pkt_cnt : 16; /* [15:0] */
        unsigned int rsv_123 : 16;             /* [31:16] */
    } bits;

    unsigned int value;
} u_ssu_ig_tx_short_pkt_cnt;

typedef union {
    struct {
        unsigned int ig_rx_err_pkt_cnt : 16; /* [15:0] */
        unsigned int rsv_124 : 16;           /* [31:16] */
    } bits;

    unsigned int value;
} u_ssu_ig_rx_err_pkt_cnt;

typedef union {
    struct {
        unsigned int ig_tx_err_pkt_cnt : 16; /* [15:0] */
        unsigned int rsv_125 : 16;           /* [31:16] */
    } bits;

    unsigned int value;
} u_ssu_ig_tx_err_pkt_cnt;

typedef union {
    struct {
        unsigned int eg_rx_rocee_pkt_cnt : 48; /* [47:0] */
        unsigned int rsv_126 : 16;             /* [63:48] */
    } bits;

    unsigned int value;
} u_ssu_eg_rx_rocee_pkt_cnt;

typedef union {
    struct {
        unsigned int tm_byte_info_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_tm_byte_info_cnt;

typedef union {
    struct {
        unsigned int txsch_byte_info_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_txsch_byte_info_cnt;

typedef union {
    struct {
        unsigned int pkt_bank0_rsc_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_pkt_bank0_rsc_cnt;

typedef union {
    struct {
        unsigned int pkt_bank1_rsc_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_pkt_bank1_rsc_cnt;

typedef union {
    struct {
        unsigned int pkt_bank2_rsc_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_pkt_bank2_rsc_cnt;

typedef union {
    struct {
        unsigned int pkt_bank3_rsc_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_pkt_bank3_rsc_cnt;

typedef union {
    struct {
        unsigned int oq_pd_bank_rsc_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_oq_pd_rsc_cnt;

typedef union {
    struct {
        unsigned int pkt_mult_total_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ppp_mult_total_cnt;

typedef union {
    struct {
        unsigned int ppp_rx_key_req_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ppp_rx_key_req_cnt;

typedef union {
    struct {
        unsigned int ppp_tx_key_req_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ppp_tx_key_req_cnt;

typedef union {
    struct {
        unsigned int ppp_rx_key_rlt_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_ssu_rx_key_rlt_cnt;

typedef union {
    struct {
        unsigned int ppp_tx_key_rlt_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_ssu_tx_key_rlt_cnt;

typedef union {
    struct {
        unsigned int ppp_key_drop_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_ssu_key_drop_cnt;

typedef union {
    struct {
        unsigned int ppp_rlt_drop_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ppp_ssu_rlt_drop_cnt;

typedef union {
    struct {
        unsigned int rx_pkt_l2err_drop_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_rx_l2err_drop_cnt;

typedef union {
    struct {
        unsigned int tx_pkt_l2err_drop_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_tx_l2err_drop_cnt;

typedef union {
    struct {
        unsigned int rx_pkt_full_drop_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_rx_pkt_full_drop_cnt;

typedef union {
    struct {
        unsigned int tx_pkt_full_drop_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_tx_pkt_full_drop_cnt;

typedef union {
    struct {
        unsigned int rx_pkt_part_drop_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_rx_pkt_part_drop_cnt;

typedef union {
    struct {
        unsigned int tx_pkt_part_drop_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_tx_pkt_part_drop_cnt;

typedef union {
    struct {
        unsigned int pkt_hi_pri_mult_drop_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_hi_pri_mult_drop_cnt;

typedef union {
    struct {
        unsigned int pkt_lo_pri_mult_drop_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_lo_pri_mult_drop_cnt;

typedef union {
    struct {
        unsigned int pkt_hi_pri_sgl_drop_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_hi_pri_sgl_drop_cnt;

typedef union {
    struct {
        unsigned int pkt_lo_pri_sgl_drop_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_lo_pri_sgl_drop_cnt;

typedef union {
    struct {
        unsigned int pkt_car_drop_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_car_drop_cnt;

typedef union {
    struct {
        unsigned int pkt_mult_uncopy_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_mult_uncopy_cnt;

typedef union {
    struct {
        unsigned int sys_abnormal_int_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_sys_abnormal_drop_cnt;

typedef union {
    struct {
        unsigned int oq_list_ptr_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_oq_list_ptr_cnt;

typedef union {
    struct {
        unsigned int in_port_cell_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_in_port_cell_cnt;

typedef union {
    struct {
        unsigned int in_port_pd_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_in_port_pd_cnt;

typedef union {
    struct {
        unsigned int pkt_ot_total_pd_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ot_total_pd_cnt;

typedef union {
    struct {
        unsigned int pkt_ot_total_mult_pd_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ot_total_mult_pd_cnt;

typedef union {
    struct {
        unsigned int pkt_ot_port_mult_pd_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ot_port_mult_pd_cnt;

typedef union {
    struct {
        unsigned int pkt_ot_port_mult_cell_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ot_port_mult_cell_cnt;

typedef union {
    struct {
        unsigned int pkt_ot_port_pd_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ot_port_pd_cnt;

typedef union {
    struct {
        unsigned int pkt_ot_port_cell_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ot_port_cell_cnt;

typedef union {
    struct {
        unsigned int pkt_oq_pd_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_oq_pd_cnt;

typedef union {
    struct {
        unsigned int pkt_oq_cell_cnt : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_oq_cell_cnt;

typedef union {
    struct {
        unsigned int ssu_mem_sbit_err_cnt : 16; /* [15:0] */
        unsigned int rsv_127 : 16;              /* [31:16] */
    } bits;

    unsigned int value;
} u_ssu_mem_sbit_err_cnt;

typedef union {
    struct {
        unsigned int ssu_mem_mbit_err_cnt : 16; /* [15:0] */
        unsigned int rsv_128 : 16;              /* [31:16] */
    } bits;

    unsigned int value;
} u_ssu_mem_mbit_err_cnt;

typedef union {
    struct {
        unsigned int fifo_dfx0 : 2;  /* [1:0] */
        unsigned int fifo_dfx1 : 2;  /* [3:2] */
        unsigned int fifo_dfx2 : 2;  /* [5:4] */
        unsigned int fifo_dfx3 : 2;  /* [7:6] */
        unsigned int fifo_dfx4 : 2;  /* [9:8] */
        unsigned int fifo_dfx5 : 2;  /* [11:10] */
        unsigned int fifo_dfx6 : 2;  /* [13:12] */
        unsigned int fifo_dfx7 : 2;  /* [15:14] */
        unsigned int fifo_dfx8 : 2;  /* [17:16] */
        unsigned int fifo_dfx9 : 2;  /* [19:18] */
        unsigned int fifo_dfx10 : 2; /* [21:20] */
        unsigned int fifo_dfx11 : 2; /* [23:22] */
        unsigned int fifo_dfx12 : 2; /* [25:24] */
        unsigned int fifo_dfx13 : 2; /* [27:26] */
        unsigned int fifo_dfx14 : 2; /* [29:28] */
        unsigned int fifo_dfx15 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_fifo_dfx0;

typedef union {
    struct {
        unsigned int fifo_dfx16 : 2; /* [1:0] */
        unsigned int fifo_dfx17 : 2; /* [3:2] */
        unsigned int fifo_dfx18 : 2; /* [5:4] */
        unsigned int fifo_dfx19 : 2; /* [7:6] */
        unsigned int fifo_dfx20 : 2; /* [9:8] */
        unsigned int fifo_dfx21 : 2; /* [11:10] */
        unsigned int fifo_dfx22 : 2; /* [13:12] */
        unsigned int fifo_dfx23 : 2; /* [15:14] */
        unsigned int fifo_dfx24 : 2; /* [17:16] */
        unsigned int fifo_dfx25 : 2; /* [19:18] */
        unsigned int fifo_dfx26 : 2; /* [21:20] */
        unsigned int fifo_dfx27 : 2; /* [23:22] */
        unsigned int fifo_dfx28 : 2; /* [25:24] */
        unsigned int fifo_dfx29 : 2; /* [27:26] */
        unsigned int fifo_dfx30 : 2; /* [29:28] */
        unsigned int fifo_dfx31 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_fifo_dfx1;

typedef union {
    struct {
        unsigned int fifo_dfx32 : 2; /* [1:0] */
        unsigned int fifo_dfx33 : 2; /* [3:2] */
        unsigned int fifo_dfx34 : 2; /* [5:4] */
        unsigned int fifo_dfx35 : 2; /* [7:6] */
        unsigned int fifo_dfx36 : 2; /* [9:8] */
        unsigned int fifo_dfx37 : 2; /* [11:10] */
        unsigned int fifo_dfx38 : 2; /* [13:12] */
        unsigned int fifo_dfx39 : 2; /* [15:14] */
        unsigned int fifo_dfx40 : 2; /* [17:16] */
        unsigned int fifo_dfx41 : 2; /* [19:18] */
        unsigned int fifo_dfx42 : 2; /* [21:20] */
        unsigned int fifo_dfx43 : 2; /* [23:22] */
        unsigned int fifo_dfx44 : 2; /* [25:24] */
        unsigned int fifo_dfx45 : 2; /* [27:26] */
        unsigned int fifo_dfx46 : 2; /* [29:28] */
        unsigned int fifo_dfx47 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_fifo_dfx2;

typedef union {
    struct {
        unsigned int fifo_dfx48 : 2; /* [1:0] */
        unsigned int fifo_dfx49 : 2; /* [3:2] */
        unsigned int fifo_dfx50 : 2; /* [5:4] */
        unsigned int fifo_dfx51 : 2; /* [7:6] */
        unsigned int fifo_dfx52 : 2; /* [9:8] */
        unsigned int fifo_dfx53 : 2; /* [11:10] */
        unsigned int fifo_dfx54 : 2; /* [13:12] */
        unsigned int fifo_dfx55 : 2; /* [15:14] */
        unsigned int fifo_dfx56 : 2; /* [17:16] */
        unsigned int fifo_dfx57 : 2; /* [19:18] */
        unsigned int fifo_dfx58 : 2; /* [21:20] */
        unsigned int fifo_dfx59 : 2; /* [23:22] */
        unsigned int fifo_dfx60 : 2; /* [25:24] */
        unsigned int fifo_dfx61 : 2; /* [27:26] */
        unsigned int fifo_dfx62 : 2; /* [29:28] */
        unsigned int fifo_dfx63 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_fifo_dfx3;

typedef union {
    struct {
        unsigned int fifo_dfx64 : 2; /* [1:0] */
        unsigned int fifo_dfx65 : 2; /* [3:2] */
        unsigned int fifo_dfx66 : 2; /* [5:4] */
        unsigned int fifo_dfx67 : 2; /* [7:6] */
        unsigned int fifo_dfx68 : 2; /* [9:8] */
        unsigned int fifo_dfx69 : 2; /* [11:10] */
        unsigned int fifo_dfx70 : 2; /* [13:12] */
        unsigned int fifo_dfx71 : 2; /* [15:14] */
        unsigned int fifo_dfx72 : 2; /* [17:16] */
        unsigned int fifo_dfx73 : 2; /* [19:18] */
        unsigned int fifo_dfx74 : 2; /* [21:20] */
        unsigned int fifo_dfx75 : 2; /* [23:22] */
        unsigned int fifo_dfx76 : 2; /* [25:24] */
        unsigned int fifo_dfx77 : 2; /* [27:26] */
        unsigned int fifo_dfx78 : 2; /* [29:28] */
        unsigned int fifo_dfx79 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_fifo_dfx4;

typedef union {
    struct {
        unsigned int fifo_dfx80 : 2; /* [1:0] */
        unsigned int fifo_dfx81 : 2; /* [3:2] */
        unsigned int fifo_dfx82 : 2; /* [5:4] */
        unsigned int fifo_dfx83 : 2; /* [7:6] */
        unsigned int fifo_dfx84 : 2; /* [9:8] */
        unsigned int fifo_dfx85 : 2; /* [11:10] */
        unsigned int fifo_dfx86 : 2; /* [13:12] */
        unsigned int fifo_dfx87 : 2; /* [15:14] */
        unsigned int fifo_dfx88 : 2; /* [17:16] */
        unsigned int fifo_dfx89 : 2; /* [19:18] */
        unsigned int fifo_dfx90 : 2; /* [21:20] */
        unsigned int fifo_dfx91 : 2; /* [23:22] */
        unsigned int fifo_dfx92 : 2; /* [25:24] */
        unsigned int fifo_dfx93 : 2; /* [27:26] */
        unsigned int fifo_dfx94 : 2; /* [29:28] */
        unsigned int fifo_dfx95 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_fifo_dfx5;

typedef union {
    struct {
        unsigned int fifo_dfx96 : 2;  /* [1:0] */
        unsigned int fifo_dfx97 : 2;  /* [3:2] */
        unsigned int fifo_dfx98 : 2;  /* [5:4] */
        unsigned int fifo_dfx99 : 2;  /* [7:6] */
        unsigned int fifo_dfx100 : 2; /* [9:8] */
        unsigned int fifo_dfx101 : 2; /* [11:10] */
        unsigned int fifo_dfx102 : 2; /* [13:12] */
        unsigned int fifo_dfx103 : 2; /* [15:14] */
        unsigned int fifo_dfx104 : 2; /* [17:16] */
        unsigned int fifo_dfx105 : 2; /* [19:18] */
        unsigned int fifo_dfx106 : 2; /* [21:20] */
        unsigned int fifo_dfx107 : 2; /* [23:22] */
        unsigned int fifo_dfx108 : 2; /* [25:24] */
        unsigned int fifo_dfx109 : 2; /* [27:26] */
        unsigned int fifo_dfx110 : 2; /* [29:28] */
        unsigned int fifo_dfx111 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_fifo_dfx6;

typedef union {
    struct {
        unsigned int fifo_dfx112 : 2; /* [1:0] */
        unsigned int fifo_dfx113 : 2; /* [3:2] */
        unsigned int fifo_dfx114 : 2; /* [5:4] */
        unsigned int fifo_dfx115 : 2; /* [7:6] */
        unsigned int fifo_dfx116 : 2; /* [9:8] */
        unsigned int fifo_dfx117 : 2; /* [11:10] */
        unsigned int fifo_dfx118 : 2; /* [13:12] */
        unsigned int fifo_dfx119 : 2; /* [15:14] */
        unsigned int fifo_dfx120 : 2; /* [17:16] */
        unsigned int fifo_dfx121 : 2; /* [19:18] */
        unsigned int fifo_dfx122 : 2; /* [21:20] */
        unsigned int fifo_dfx123 : 2; /* [23:22] */
        unsigned int fifo_dfx124 : 2; /* [25:24] */
        unsigned int fifo_dfx125 : 2; /* [27:26] */
        unsigned int fifo_dfx126 : 2; /* [29:28] */
        unsigned int fifo_dfx127 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_fifo_dfx7;

typedef union {
    struct {
        unsigned int fifo_dfx128 : 2; /* [1:0] */
        unsigned int fifo_dfx129 : 2; /* [3:2] */
        unsigned int fifo_dfx130 : 2; /* [5:4] */
        unsigned int fifo_dfx131 : 2; /* [7:6] */
        unsigned int fifo_dfx132 : 2; /* [9:8] */
        unsigned int fifo_dfx133 : 2; /* [11:10] */
        unsigned int fifo_dfx134 : 2; /* [13:12] */
        unsigned int fifo_dfx135 : 2; /* [15:14] */
        unsigned int fifo_dfx136 : 2; /* [17:16] */
        unsigned int fifo_dfx137 : 2; /* [19:18] */
        unsigned int fifo_dfx138 : 2; /* [21:20] */
        unsigned int fifo_dfx139 : 2; /* [23:22] */
        unsigned int fifo_dfx140 : 2; /* [25:24] */
        unsigned int fifo_dfx141 : 2; /* [27:26] */
        unsigned int fifo_dfx142 : 2; /* [29:28] */
        unsigned int fifo_dfx143 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_fifo_dfx8;

typedef union {
    struct {
        unsigned int fifo_dfx144 : 2; /* [1:0] */
        unsigned int fifo_dfx145 : 2; /* [3:2] */
        unsigned int fifo_dfx146 : 2; /* [5:4] */
        unsigned int fifo_dfx147 : 2; /* [7:6] */
        unsigned int fifo_dfx148 : 2; /* [9:8] */
        unsigned int fifo_dfx149 : 2; /* [11:10] */
        unsigned int fifo_dfx150 : 2; /* [13:12] */
        unsigned int fifo_dfx151 : 2; /* [15:14] */
        unsigned int fifo_dfx152 : 2; /* [17:16] */
        unsigned int fifo_dfx153 : 2; /* [19:18] */
        unsigned int fifo_dfx154 : 2; /* [21:20] */
        unsigned int fifo_dfx155 : 2; /* [23:22] */
        unsigned int fifo_dfx156 : 2; /* [25:24] */
        unsigned int fifo_dfx157 : 2; /* [27:26] */
        unsigned int fifo_dfx158 : 2; /* [29:28] */
        unsigned int fifo_dfx159 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_fifo_dfx9;

typedef union {
    struct {
        unsigned int fifo_dfx160 : 2; /* [1:0] */
        unsigned int fifo_dfx161 : 2; /* [3:2] */
        unsigned int fifo_dfx162 : 2; /* [5:4] */
        unsigned int fifo_dfx163 : 2; /* [7:6] */
        unsigned int fifo_dfx164 : 2; /* [9:8] */
        unsigned int fifo_dfx165 : 2; /* [11:10] */
        unsigned int fifo_dfx166 : 2; /* [13:12] */
        unsigned int fifo_dfx167 : 2; /* [15:14] */
        unsigned int fifo_dfx168 : 2; /* [17:16] */
        unsigned int fifo_dfx169 : 2; /* [19:18] */
        unsigned int fifo_dfx170 : 2; /* [21:20] */
        unsigned int fifo_dfx171 : 2; /* [23:22] */
        unsigned int fifo_dfx172 : 2; /* [25:24] */
        unsigned int fifo_dfx173 : 2; /* [27:26] */
        unsigned int fifo_dfx174 : 2; /* [29:28] */
        unsigned int fifo_dfx175 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_fifo_dfx10;

typedef union {
    struct {
        unsigned int fifo_dfx176 : 2; /* [1:0] */
        unsigned int fifo_dfx177 : 2; /* [3:2] */
        unsigned int fifo_dfx178 : 2; /* [5:4] */
        unsigned int fifo_dfx179 : 2; /* [7:6] */
        unsigned int fifo_dfx180 : 2; /* [9:8] */
        unsigned int fifo_dfx181 : 2; /* [11:10] */
        unsigned int fifo_dfx182 : 2; /* [13:12] */
        unsigned int fifo_dfx183 : 2; /* [15:14] */
        unsigned int fifo_dfx184 : 2; /* [17:16] */
        unsigned int fifo_dfx185 : 2; /* [19:18] */
        unsigned int fifo_dfx186 : 2; /* [21:20] */
        unsigned int fifo_dfx187 : 2; /* [23:22] */
        unsigned int fifo_dfx188 : 2; /* [25:24] */
        unsigned int fifo_dfx189 : 2; /* [27:26] */
        unsigned int fifo_dfx190 : 2; /* [29:28] */
        unsigned int fifo_dfx191 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_fifo_dfx11;

typedef union {
    struct {
        unsigned int fifo_dfx192 : 2; /* [1:0] */
        unsigned int fifo_dfx193 : 2; /* [3:2] */
        unsigned int fifo_dfx194 : 2; /* [5:4] */
        unsigned int fifo_dfx195 : 2; /* [7:6] */
        unsigned int fifo_dfx196 : 2; /* [9:8] */
        unsigned int fifo_dfx197 : 2; /* [11:10] */
        unsigned int fifo_dfx198 : 2; /* [13:12] */
        unsigned int fifo_dfx199 : 2; /* [15:14] */
        unsigned int fifo_dfx200 : 2; /* [17:16] */
        unsigned int fifo_dfx201 : 2; /* [19:18] */
        unsigned int fifo_dfx202 : 2; /* [21:20] */
        unsigned int fifo_dfx203 : 2; /* [23:22] */
        unsigned int fifo_dfx204 : 2; /* [25:24] */
        unsigned int fifo_dfx205 : 2; /* [27:26] */
        unsigned int fifo_dfx206 : 2; /* [29:28] */
        unsigned int fifo_dfx207 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_fifo_dfx12;

typedef union {
    struct {
        unsigned int fifo_dfx208 : 2; /* [1:0] */
        unsigned int fifo_dfx209 : 2; /* [3:2] */
        unsigned int fifo_dfx210 : 2; /* [5:4] */
        unsigned int fifo_dfx211 : 2; /* [7:6] */
        unsigned int fifo_dfx212 : 2; /* [9:8] */
        unsigned int fifo_dfx213 : 2; /* [11:10] */
        unsigned int fifo_dfx214 : 2; /* [13:12] */
        unsigned int fifo_dfx215 : 2; /* [15:14] */
        unsigned int fifo_dfx216 : 2; /* [17:16] */
        unsigned int fifo_dfx217 : 2; /* [19:18] */
        unsigned int fifo_dfx218 : 2; /* [21:20] */
        unsigned int fifo_dfx219 : 2; /* [23:22] */
        unsigned int fifo_dfx220 : 2; /* [25:24] */
        unsigned int fifo_dfx221 : 2; /* [27:26] */
        unsigned int fifo_dfx222 : 2; /* [29:28] */
        unsigned int fifo_dfx223 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_fifo_dfx13;

typedef union {
    struct {
        unsigned int fifo_dfx224 : 2; /* [1:0] */
        unsigned int fifo_dfx225 : 2; /* [3:2] */
        unsigned int fifo_dfx226 : 2; /* [5:4] */
        unsigned int fifo_dfx227 : 2; /* [7:6] */
        unsigned int fifo_dfx228 : 2; /* [9:8] */
        unsigned int fifo_dfx229 : 2; /* [11:10] */
        unsigned int fifo_dfx230 : 2; /* [13:12] */
        unsigned int fifo_dfx231 : 2; /* [15:14] */
        unsigned int fifo_dfx232 : 2; /* [17:16] */
        unsigned int fifo_dfx233 : 2; /* [19:18] */
        unsigned int fifo_dfx234 : 2; /* [21:20] */
        unsigned int fifo_dfx235 : 2; /* [23:22] */
        unsigned int fifo_dfx236 : 2; /* [25:24] */
        unsigned int fifo_dfx237 : 2; /* [27:26] */
        unsigned int fifo_dfx238 : 2; /* [29:28] */
        unsigned int fifo_dfx239 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_fifo_dfx14;

typedef union {
    struct {
        unsigned int fifo_dfx240 : 2; /* [1:0] */
        unsigned int fifo_dfx241 : 2; /* [3:2] */
        unsigned int fifo_dfx242 : 2; /* [5:4] */
        unsigned int fifo_dfx243 : 2; /* [7:6] */
        unsigned int fifo_dfx244 : 2; /* [9:8] */
        unsigned int fifo_dfx245 : 2; /* [11:10] */
        unsigned int fifo_dfx246 : 2; /* [13:12] */
        unsigned int fifo_dfx247 : 2; /* [15:14] */
        unsigned int fifo_dfx248 : 2; /* [17:16] */
        unsigned int fifo_dfx249 : 2; /* [19:18] */
        unsigned int fifo_dfx250 : 2; /* [21:20] */
        unsigned int fifo_dfx251 : 2; /* [23:22] */
        unsigned int fifo_dfx252 : 2; /* [25:24] */
        unsigned int fifo_dfx253 : 2; /* [27:26] */
        unsigned int fifo_dfx254 : 2; /* [29:28] */
        unsigned int fifo_dfx255 : 2; /* [31:30] */
    } bits;

    unsigned int value;
} u_ssu_fifo_dfx15;

typedef union {
    struct {
        unsigned int inter_bp_status0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_bp_status0;

typedef union {
    struct {
        unsigned int inter_bp_status1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_bp_status1;

typedef union {
    struct {
        unsigned int inter_bp_status2 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_bp_status2;

typedef union {
    struct {
        unsigned int inter_bp_status3 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_bp_status3;

typedef union {
    struct {
        unsigned int inter_bp_status4 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_bp_status4;

typedef union {
    struct {
        unsigned int inter_bp_status5 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_bp_status5;

typedef union {
    struct {
        unsigned int inter_bp_status6 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_bp_status6;

typedef union {
    struct {
        unsigned int inter_bp_status7 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_bp_status7;

typedef union {
    struct {
        unsigned int bank_balance_statuas : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_bank_balance_status;

typedef union {
    struct {
        unsigned int dup_shap_bmp : 3; /* [2:0] */
        unsigned int rsv_129 : 29;     /* [31:3] */
    } bits;

    unsigned int value;
} u_ssu_dup_shap_bmp;

typedef union {
    struct {
        unsigned int ets_shap_bmp0 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ets_shap_bmp0;

typedef union {
    struct {
        unsigned int ets_shap_bmp1 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ets_shap_bmp1;

typedef union {
    struct {
        unsigned int ets_shap_bmp2 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ets_shap_bmp2;

typedef union {
    struct {
        unsigned int ets_shap_bmp3 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ets_shap_bmp3;

typedef union {
    struct {
        unsigned int ets_shap_bmp4 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ets_shap_bmp4;

typedef union {
    struct {
        unsigned int ets_shap_bmp5 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ets_shap_bmp5;

typedef union {
    struct {
        unsigned int ets_shap_bmp6 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ets_shap_bmp6;

typedef union {
    struct {
        unsigned int ets_shap_bmp7 : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_ets_shap_bmp7;

typedef union {
    struct {
        unsigned int qset_cfg : 9; /* [8:0] */
        unsigned int rsv_130 : 23; /* [31:9] */
    } bits;

    unsigned int value;
} u_qcn_tc_qset;

typedef union {
    struct {
        unsigned int rx_share_buf_size : 15; /* [14:0] */
        unsigned int rsv_131 : 17;           /* [31:15] */
    } bits;

    unsigned int value;
} u_ssu_rx_share_buffer_size;

typedef union {
    struct {
        unsigned int rx_share_lo_thd : 15; /* [14:0] */
        unsigned int rsv_132 : 1;          /* [15] */
        unsigned int rx_share_hi_thd : 15; /* [30:16] */
        unsigned int rsv_133 : 1;          /* [31] */
    } bits;

    unsigned int value;
} u_ssu_rx_share_thd;

typedef union {
    struct {
        unsigned int tx_share_buf_size : 15; /* [14:0] */
        unsigned int rsv_134 : 17;           /* [31:15] */
    } bits;

    unsigned int value;
} u_ssu_tx_share_buffer_size;

typedef union {
    struct {
        unsigned int tx_share_lo_thd : 15; /* [14:0] */
        unsigned int rsv_135 : 1;          /* [15] */
        unsigned int tx_share_hi_thd : 15; /* [30:16] */
        unsigned int rsv_136 : 1;          /* [31] */
    } bits;

    unsigned int value;
} u_ssu_tx_share_thd;

typedef union {
    struct {
        unsigned int ssu_pause_time_out_en : 1; /* [0] */
        unsigned int rsv_137 : 31;              /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_pause_time_out_en;

typedef union {
    struct {
        unsigned int ssu_pause_time_out : 22; /* [21:0] */
        unsigned int rsv_138 : 10;            /* [31:22] */
    } bits;

    unsigned int value;
} u_ssu_pause_time_out;

typedef union {
    struct {
        unsigned int bmu_pool_pause_en : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_bmu_pool_pause_en;

typedef union {
    struct {
        unsigned int cfg_rx_port_type : 1;       /* [0] */
        unsigned int rsv_139 : 3;                /* [3:1] */
        unsigned int cfg_rx_non_std_pkt_len : 6; /* [9:4] */
        unsigned int rsv_140 : 22;               /* [31:10] */
    } bits;

    unsigned int value;
} u_ssu_rx_port_type;

typedef union {
    struct {
        unsigned int cfg_tx_port_type : 1;       /* [0] */
        unsigned int rsv_141 : 3;                /* [3:1] */
        unsigned int cfg_tx_non_std_pkt_len : 6; /* [9:4] */
        unsigned int rsv_142 : 22;               /* [31:10] */
    } bits;

    unsigned int value;
} u_ssu_tx_port_type;

typedef union {
    struct {
        unsigned int rx_in_vlan_type : 16; /* [15:0] */
        unsigned int rx_ot_vlan_type : 16; /* [31:16] */
    } bits;

    unsigned int value;
} u_ssu_rx_vlan_type_cfg;

typedef union {
    struct {
        unsigned int tx_in_vlan_type : 16; /* [15:0] */
        unsigned int tx_ot_vlan_type : 16; /* [31:16] */
    } bits;

    unsigned int value;
} u_ssu_tx_vlan_type_cfg;

typedef union {
    struct {
        unsigned int port_encrypt_profile : 11; /* [10:0] */
        unsigned int rsv_143 : 5;               /* [15:11] */
        unsigned int port_encrypt_sc : 9;       /* [24:16] */
        unsigned int rsv_144 : 7;               /* [31:25] */
    } bits;

    unsigned int value;
} u_ssu_port_enctrpt_info;

typedef union {
    struct {
        unsigned int mac_tc_weight_cfg : 8; /* [7:0] */
        unsigned int rsv_145 : 24;          /* [31:8] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_tc_weight;

typedef union {
    struct {
        unsigned int mac_tc_sp_mode_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_tc_sp_mode;

typedef union {
    struct {
        unsigned int mac_tcg_weight_cfg : 8; /* [7:0] */
        unsigned int rsv_146 : 24;           /* [31:8] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_tcg_weight;

typedef union {
    struct {
        unsigned int mac_tcg_sp_mode_cfg : 4; /* [3:0] */
        unsigned int rsv_147 : 28;            /* [31:4] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_tcg_sp_mode;

typedef union {
    struct {
        unsigned int mac_tcg_shap_cfg : 26; /* [25:0] */
        unsigned int rsv_148 : 6;           /* [31:26] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_tcg_shaping;

typedef union {
    struct {
        unsigned int mac_tc_shap_cfg : 26; /* [25:0] */
        unsigned int rsv_149 : 6;          /* [31:26] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_tc_shaping;

typedef union {
    struct {
        unsigned int mac_weight_offset_cfg : 4; /* [3:0] */
        unsigned int rsv_150 : 28;              /* [31:4] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_weight_offset;

typedef union {
    struct {
        unsigned int mac_port_maping_cfg : 4; /* [3:0] */
        unsigned int rsv_151 : 28;            /* [31:4] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_port_maping;

typedef union {
    struct {
        unsigned int mac_tcg_maping_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_tcg_maping;

typedef union {
    struct {
        unsigned int mac_tc_maping_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_tc_maping;

typedef union {
    struct {
        unsigned int mac_port_shap_cfg : 26; /* [25:0] */
        unsigned int rsv_152 : 6;            /* [31:26] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_port_shaping;

typedef union {
    struct {
        unsigned int mac_queue_shap_cfg : 26; /* [25:0] */
        unsigned int rsv_153 : 6;             /* [31:26] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_queue_shaping;

typedef union {
    struct {
        unsigned int mac_pre_sub_offset_len_cfg : 15; /* [14:0] */
        unsigned int rsv_154 : 17;                    /* [31:15] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_pre_sub_offset_len_cfg;

typedef union {
    struct {
        unsigned int mac_queue_shap_bypass : 1; /* [0] */
        unsigned int mac_tc_shap_bypass : 1;    /* [1] */
        unsigned int mac_tcg_shap_bypass : 1;   /* [2] */
        unsigned int mac_port_shap_bypass : 1;  /* [3] */
        unsigned int rsv_155 : 28;              /* [31:4] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_shapping_bypass;

typedef union {
    struct {
        unsigned int mac_queue_link_tc_cfg : 5; /* [4:0] */
        unsigned int rsv_156 : 27;              /* [31:5] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_queue_link_tc_cfg;

typedef union {
    struct {
        unsigned int mac_queue_en_cfg : 1; /* [0] */
        unsigned int rsv_157 : 31;         /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_queue_en_cfg;

typedef union {
    struct {
        unsigned int mac_tc_sendslope_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_tc_sendslope;

typedef union {
    struct {
        unsigned int mac_tc_idleslope_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_tc_idleslope;

typedef union {
    struct {
        unsigned int mac_tc_hicredit_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_tc_hicredit;

typedef union {
    struct {
        unsigned int mac_tc_locredit_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_tc_locredit;

typedef union {
    struct {
        unsigned int mac_tc_cbs_en_cfg : 1; /* [0] */
        unsigned int rsv_158 : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_tc_cbs_en;

typedef union {
    struct {
        unsigned int mac_tc_speed_sel_cfg : 4; /* [3:0] */
        unsigned int rsv_159 : 28;             /* [31:4] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_tc_speed_sel;

typedef union {
    struct {
        unsigned int mac_queue_flush_req : 1; /* [0] */
        unsigned int rsv_160 : 31;            /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_queue_flush_req;

typedef union {
    struct {
        unsigned int mac_queue_flush_done : 1; /* [0] */
        unsigned int rsv_161 : 31;             /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_mac_ets_queue_flush_done;

typedef union {
    struct {
        unsigned int host_tc_weight_cfg : 8; /* [7:0] */
        unsigned int rsv_162 : 24;           /* [31:8] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_tc_weight;

typedef union {
    struct {
        unsigned int host_tc_sp_mode_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_tc_sp_mode;

typedef union {
    struct {
        unsigned int host_tcg_weight_cfg : 8; /* [7:0] */
        unsigned int rsv_163 : 24;            /* [31:8] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_tcg_weight;

typedef union {
    struct {
        unsigned int host_tcg_sp_mode_cfg : 4; /* [3:0] */
        unsigned int rsv_164 : 28;             /* [31:4] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_tcg_sp_mode;

typedef union {
    struct {
        unsigned int host_tcg_shap_cfg : 26; /* [25:0] */
        unsigned int rsv_165 : 6;            /* [31:26] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_tcg_shaping;

typedef union {
    struct {
        unsigned int host_tc_shap_cfg : 26; /* [25:0] */
        unsigned int rsv_166 : 6;           /* [31:26] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_tc_shaping;

typedef union {
    struct {
        unsigned int host_weight_offset_cfg : 4; /* [3:0] */
        unsigned int rsv_167 : 28;               /* [31:4] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_weight_offset;

typedef union {
    struct {
        unsigned int host_port_maping_cfg : 4; /* [3:0] */
        unsigned int rsv_168 : 28;             /* [31:4] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_port_maping;

typedef union {
    struct {
        unsigned int host_tcg_maping_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_tcg_maping;

typedef union {
    struct {
        unsigned int host_tc_maping_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_tc_maping;

typedef union {
    struct {
        unsigned int host_port_shap_cfg : 26; /* [25:0] */
        unsigned int rsv_169 : 6;             /* [31:26] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_port_shaping;

typedef union {
    struct {
        unsigned int host_queue_shap_cfg : 26; /* [25:0] */
        unsigned int rsv_170 : 6;              /* [31:26] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_queue_shaping;

typedef union {
    struct {
        unsigned int host_pre_sub_offset_len_cfg : 15; /* [14:0] */
        unsigned int rsv_171 : 17;                     /* [31:15] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_pre_sub_offset_len_cfg;

typedef union {
    struct {
        unsigned int host_queue_shap_bypass : 1; /* [0] */
        unsigned int host_tc_shap_bypass : 1;    /* [1] */
        unsigned int host_tcg_shap_bypass : 1;   /* [2] */
        unsigned int host_port_shap_bypass : 1;  /* [3] */
        unsigned int rsv_172 : 28;               /* [31:4] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_shapping_bypass;

typedef union {
    struct {
        unsigned int host_queue_link_tc_cfg : 5; /* [4:0] */
        unsigned int rsv_173 : 27;               /* [31:5] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_queue_link_tc_cfg;

typedef union {
    struct {
        unsigned int host_queue_en_cfg : 1; /* [0] */
        unsigned int rsv_174 : 31;          /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_queue_en_cfg;

typedef union {
    struct {
        unsigned int host_tc_sendslope_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_tc_sendslope;

typedef union {
    struct {
        unsigned int host_tc_idleslope_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_tc_idleslope;

typedef union {
    struct {
        unsigned int host_tc_hicredit_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_tc_hicredit;

typedef union {
    struct {
        unsigned int host_tc_locredit_cfg : 32; /* [31:0] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_tc_locredit;

typedef union {
    struct {
        unsigned int host_tc_cbs_en_cfg : 1; /* [0] */
        unsigned int rsv_175 : 31;           /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_tc_cbs_en;

typedef union {
    struct {
        unsigned int host_tc_speed_sel_cfg : 4; /* [3:0] */
        unsigned int rsv_176 : 28;              /* [31:4] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_tc_speed_sel;

typedef union {
    struct {
        unsigned int host_queue_flush_req : 1; /* [0] */
        unsigned int rsv_177 : 31;             /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_queue_flush_req;

typedef union {
    struct {
        unsigned int host_queue_flush_done : 1; /* [0] */
        unsigned int rsv_178 : 31;              /* [31:1] */
    } bits;

    unsigned int value;
} u_ssu_host_ets_queue_flush_done;

typedef union {
    struct {
        unsigned int tx_oq_drop_lo_thd : 15; /* [14:0] */
        unsigned int rsv_179 : 1;            /* [15] */
        unsigned int tx_oq_drop_hi_thd : 15; /* [30:16] */
        unsigned int rsv_180 : 1;            /* [31] */
    } bits;

    unsigned int value;
} u_ssu_tx_oq_drop_thd;

typedef union {
    struct {
        unsigned int rx_oq_drop_lo_thd : 15; /* [14:0] */
        unsigned int rsv_181 : 1;            /* [15] */
        unsigned int rx_oq_drop_hi_thd : 15; /* [30:16] */
        unsigned int rsv_182 : 1;            /* [31] */
    } bits;

    unsigned int value;
} u_ssu_rx_oq_drop_thd;

typedef union {
    struct {
        unsigned int tx_oq_cell_drop_lo_thd : 15; /* [14:0] */
        unsigned int rsv_183 : 1;                 /* [15] */
        unsigned int tx_oq_cell_drop_hi_thd : 15; /* [30:16] */
        unsigned int rsv_184 : 1;                 /* [31] */
    } bits;

    unsigned int value;
} u_ssu_tx_oq_drop_cell_thd;

typedef union {
    struct {
        unsigned int rx_oq_cell_drop_lo_thd : 15; /* [14:0] */
        unsigned int rsv_185 : 1;                 /* [15] */
        unsigned int rx_oq_cell_drop_hi_thd : 15; /* [30:16] */
        unsigned int rsv_186 : 1;                 /* [31] */
    } bits;

    unsigned int value;
} u_ssu_rx_oq_drop_cell_thd;

typedef union {
    struct {
        unsigned int tx_dup_ot_port_drop_lo_thd : 15; /* [14:0] */
        unsigned int rsv_187 : 1;                     /* [15] */
        unsigned int tx_dup_ot_port_drop_hi_thd : 15; /* [30:16] */
        unsigned int rsv_188 : 1;                     /* [31] */
    } bits;

    unsigned int value;
} u_ssu_tx_dup_ot_port_drop_thd;

typedef union {
    struct {
        unsigned int rx_dup_ot_port_drop_lo_thd : 15; /* [14:0] */
        unsigned int rsv_189 : 1;                     /* [15] */
        unsigned int rx_dup_ot_port_drop_hi_thd : 15; /* [30:16] */
        unsigned int rsv_190 : 1;                     /* [31] */
    } bits;

    unsigned int value;
} u_ssu_rx_dup_ot_port_drop_thd;

typedef union {
    struct {
        unsigned int tx_ot_port_drop_lo_thd : 15; /* [14:0] */
        unsigned int rsv_191 : 1;                 /* [15] */
        unsigned int tx_ot_port_drop_hi_thd : 15; /* [30:16] */
        unsigned int rsv_192 : 1;                 /* [31] */
    } bits;

    unsigned int value;
} u_ssu_tx_ot_port_drop_thd;

typedef union {
    struct {
        unsigned int rx_ot_port_drop_lo_thd : 15; /* [14:0] */
        unsigned int rsv_193 : 1;                 /* [15] */
        unsigned int rx_ot_port_drop_hi_thd : 15; /* [30:16] */
        unsigned int rsv_194 : 1;                 /* [31] */
    } bits;

    unsigned int value;
} u_ssu_rx_ot_port_drop_thd;

typedef union {
    struct {
        unsigned int tx_dup_ot_port_cell_drop_lo_thd : 15; /* [14:0] */
        unsigned int rsv_195 : 1;                          /* [15] */
        unsigned int tx_dup_ot_port_cell_drop_hi_thd : 15; /* [30:16] */
        unsigned int rsv_196 : 1;                          /* [31] */
    } bits;

    unsigned int value;
} u_ssu_tx_dup_ot_port_drop_cell_thd;

typedef union {
    struct {
        unsigned int rx_dup_ot_port_cell_drop_lo_thd : 15; /* [14:0] */
        unsigned int rsv_197 : 1;                          /* [15] */
        unsigned int rx_dup_ot_port_cell_drop_hi_thd : 15; /* [30:16] */
        unsigned int rsv_198 : 1;                          /* [31] */
    } bits;

    unsigned int value;
} u_ssu_rx_dup_ot_port_drop_cell_thd;

typedef union {
    struct {
        unsigned int tx_ot_port_cell_drop_lo_thd : 15; /* [14:0] */
        unsigned int rsv_199 : 1;                      /* [15] */
        unsigned int tx_ot_port_cell_drop_hi_thd : 15; /* [30:16] */
        unsigned int rsv_200 : 1;                      /* [31] */
    } bits;

    unsigned int value;
} u_ssu_tx_ot_port_cell_drop_thd;

typedef union {
    struct {
        unsigned int rx_ot_port_cell_drop_lo_thd : 15; /* [14:0] */
        unsigned int rsv_201 : 1;                      /* [15] */
        unsigned int rx_ot_port_cell_drop_hi_thd : 15; /* [30:16] */
        unsigned int rsv_202 : 1;                      /* [31] */
    } bits;

    unsigned int value;
} u_ssu_rx_ot_port_cell_drop_thd;

typedef union {
    struct {
        unsigned int rx_rev_pd_num0 : 4; /* [3:0] */
        unsigned int rx_rev_pd_num1 : 4; /* [7:4] */
        unsigned int rx_rev_pd_num2 : 4; /* [11:8] */
        unsigned int rx_rev_pd_num3 : 4; /* [15:12] */
        unsigned int rx_rev_pd_num4 : 4; /* [19:16] */
        unsigned int rx_rev_pd_num5 : 4; /* [23:20] */
        unsigned int rx_rev_pd_num6 : 4; /* [27:24] */
        unsigned int rx_rev_pd_num7 : 4; /* [31:28] */
    } bits;

    unsigned int value;
} u_ssu_rx_rev_pd_thd;

typedef union {
    struct {
        unsigned int tx_rev_pd_num0 : 4; /* [3:0] */
        unsigned int tx_rev_pd_num1 : 4; /* [7:4] */
        unsigned int tx_rev_pd_num2 : 4; /* [11:8] */
        unsigned int tx_rev_pd_num3 : 4; /* [15:12] */
        unsigned int tx_rev_pd_num4 : 4; /* [19:16] */
        unsigned int tx_rev_pd_num5 : 4; /* [23:20] */
        unsigned int tx_rev_pd_num6 : 4; /* [27:24] */
        unsigned int tx_rev_pd_num7 : 4; /* [31:28] */
    } bits;

    unsigned int value;
} u_ssu_tx_rev_pd_thd;

#endif  // __REG_SSU_H__
